#include <mizugaki/analyzer/details/analyze_query_expression.h>

#include <numeric>
#include <vector>

#include <tsl/hopscotch_set.h>

#include <takatori/value/primitive.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/variable_reference.h>
#include <takatori/scalar/binary.h>
#include <takatori/scalar/compare.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/values.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>

#include <takatori/relation/intermediate/join.h>
#include <takatori/relation/intermediate/distinct.h>
#include <takatori/relation/intermediate/limit.h>
#include <takatori/relation/intermediate/union.h>

#include <takatori/util/downcast.h>
#include <takatori/util/fail.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

#include <mizugaki/ast/scalar/value_constructor.h>
#include <mizugaki/ast/scalar/variable_reference.h>

#include <mizugaki/ast/query/dispatch.h>
#include <mizugaki/ast/table/dispatch.h>

#include <mizugaki/analyzer/details/analyze_name.h>
#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include "set_function_processor.h"

namespace mizugaki::analyzer::details {

namespace tdescriptor = ::takatori::descriptor;
namespace tvalue = ::takatori::value;
namespace trelation = ::takatori::relation;
namespace tscalar = ::takatori::scalar;

using output_port = ::takatori::relation::expression::output_port_type;
using result_type = analyze_query_expression_result;

using ::takatori::util::debug_string_builder;
using ::takatori::util::optional_ptr;
using ::takatori::util::reference_vector;
using ::takatori::util::string_builder;
using ::takatori::util::unsafe_downcast;

template<class T>
using node_vector = std::vector<std::unique_ptr<T>>;

namespace {

class engine {
public:
    engine(
            analyzer_context& context,
            trelation::graph_type& graph) noexcept :
        context_ { context },
        graph_ { graph }
    {}

    [[nodiscard]] result_type process(
            ast::query::expression const& expression,
            optional_ptr<query_scope const> parent,
            row_value_context const& value_context) {
        return dispatch(expression, std::move(parent), value_context);
    }

    [[nodiscard]] result_type operator()(
            ast::query::expression const& expr,
            optional_ptr<query_scope const> const&,
            row_value_context const&) {
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported query expression: "
                        << expr.node_kind()
                        << string_builder::to_string,
                expr.region());
        return {};
    }

    [[nodiscard]] result_type operator()( // NOLINT(*-function-cognitive-complexity)
            ast::query::query const& expr,
            optional_ptr<query_scope const> parent,
            row_value_context const&) {
        query_scope scope { parent };
        optional_ptr<output_port> output {};

        // FROM ...
        auto from = process_from_clause(expr.from(), scope);
        if (!from) {
            return {};
        }
        output = std::move(from);

        // WHERE ...
        if (expr.where()) {
            auto predicate = analyze_scalar_expression(context_, *expr.where(), scope);
            if (!predicate) {
                return {};
            }
            if (predicate.saw_aggregate()) {
                context_.report(
                        sql_analyzer_code::unsupported_feature,
                        "aggregated function in WHERE clause is not yet supported",
                        expr.where()->region());
                return {};
            }
            auto&& filter = graph_.emplace<trelation::filter>(
                    predicate.release());
            if (!context_.resolve(filter)) {
                return {};
            }
            filter.region() = context_.convert(expr.where()->region());
            filter.input().connect_to(*output);
            output = filter.output();
        }

        // GROUP BY ...
        set_function_processor set_functions { context_, graph_ };
        optional_ptr<output_port> set_function_entry { output };
        if (auto&& group_by = expr.group_by()) {
            set_functions.activate();
            for (auto&& element : group_by->elements()) {
                auto r = dispatch(*element, set_functions, scope);
                if (!r) {
                    return {};
                }
            }
        }

        // HAVING ...
        if (expr.having()) {
            set_functions.activate();
            auto predicate = analyze_scalar_expression(context_, *expr.having(), scope);
            if (!predicate) {
                return {};
            }
            auto&& filter = graph_.insert(context_.create<trelation::filter>(
                    expr.having()->region(),
                    predicate.release()));
            if (!set_functions.process(filter.ownership_condition())) {
                return {};
            }
            if (!context_.resolve(filter)) {
                return {};
            }
            filter.input().connect_to(*output);
            output = filter.output();
        }

        // SELECT ...
        relation_info info;
        optional_ptr<trelation::project> select_columns {};
        {
            if (expr.elements().size() > context_.options()->max_select_elements()) {
                context_.report(
                        sql_analyzer_code::exceed_number_of_elements,
                        string_builder {}
                                << "too many select elements: "
                                << "must be less than or equal to " << context_.options()->max_select_elements()
                                << string_builder::to_string,
                        expr.region());
                return {};
            }
            auto&& project = graph_.emplace<trelation::project>(std::vector<trelation::project::column> {});
            project.columns().reserve(std::min(expr.elements().size(), std::size_t { 8 }));
            if (!expr.elements().empty()) {
                project.region() = context_.convert(
                        expr.elements().front()->region() | expr.elements().back()->region());
            }
            for (auto&& e : expr.elements()) {
                auto select = dispatch(*e, scope, project, info);
                if (!select.success) {
                    return {};
                }
                if (select.saw_aggregate) {
                    set_functions.activate();
                }
            }
            if (!context_.resolve(project)) {
                return {};
            }
            // NOTE: we never omit `SELECT *`, then later option may reduce it
            project.input().connect_to(*output);
            output = project.output();
            select_columns = project;

            // FIXME: impl expose projections for downstream operators
            // scope.add(info);
        }

        // SELECT DISTINCT
        if (auto&& quantifier = expr.quantifier(); quantifier == ast::query::set_quantifier::distinct) {
            auto keys = create_vector<tdescriptor::variable>(info.columns().size());
            for (auto&& column : info.columns()) {
                keys.emplace_back(column.variable());
            }
            auto&& distinct = graph_.emplace<trelation::intermediate::distinct>(std::move(keys));
            distinct.region() = context_.convert(quantifier->region());
            if (!context_.resolve(distinct)) {
                return {};
            }
            distinct.input().connect_to(*output);
            output = distinct.output();
        }

        optional_ptr<trelation::project> sort_columns {};
        optional_ptr<trelation::intermediate::limit> sort_limit {};
        if (auto&& specs = expr.order_by(); !specs.empty()) {
            auto sort_keys = create_vector<trelation::intermediate::limit::sort_key>(specs.size());
            auto&& project = graph_.emplace<trelation::project>(std::vector<trelation::project::column> {});
            project.columns().reserve(specs.size());
            for (auto&& spec : specs) {
                if (spec.collation()) {
                    context_.report(
                            sql_analyzer_code::unsupported_feature,
                            "collation in ORDER BY is not yet supported",
                            spec.region());
                    return {};
                }
                if (spec.null_location()) {
                    context_.report(
                            sql_analyzer_code::unsupported_feature,
                            "NULLS FIRST/LAST in ORDER BY is not yet supported",
                            spec.region());
                }
                auto result = analyze_scalar_expression(context_, *spec.key(), scope);
                if (!result) {
                    return {};
                }
                if (result.saw_aggregate()) {
                    set_functions.activate();
                }
                if (result.value().kind() == tscalar::immediate::tag) {
                    context_.report(
                            sql_analyzer_code::unsupported_feature,
                            "plain literal is not allowed in ORDER BY clause",
                            spec.key()->region());
                    return {};
                }
                // FIXME: debug string for order key column
                auto variable = factory_.stream_variable();
                variable.region() = result.value().region();
                auto direction = convert(spec.direction());
                if (!direction) {
                    context_.clear_expression_resolution(result.value());
                    return {};
                }
                project.columns().emplace_back(variable, result.release());
                sort_keys.emplace_back(std::move(variable), *direction);
            }
            if (!context_.resolve(project)) {
                return {};
            }
            auto&& limit = graph_.insert(context_.create<trelation::intermediate::limit>(
                    specs.front().region() | specs.back().region(),
                    std::nullopt,
                    std::vector<tdescriptor::variable> {},
                    std::move(sort_keys)));
            if (!context_.resolve(limit)) {
                return {};
            }

            project.input().connect_to(*output);
            limit.input().connect_to(project.output());
            output = limit.output();
            sort_columns = project;
            sort_limit = limit;
        }

        // LIMIT
        if (expr.limit()) {
            auto nrows_expr = analyze_scalar_expression(context_, *expr.limit(), scope);
            if (!nrows_expr) {
                return {};
            }
            auto nrows = extract_size(nrows_expr.release());
            if (!nrows) {
                return {};
            }
            // NOTE: we never check aggregated view, because the operand must be a constant
            if (sort_limit) {
                // NOTE: combine previous ORDER BY operation to represent a "top-k" operator
                sort_limit->count(nrows);
                // FIXME: update region
            } else {
                auto&& limit = graph_.insert(context_.create<trelation::intermediate::limit>(
                        expr.limit()->region(),
                        nrows,
                        create_vector<tdescriptor::variable>(),
                        create_vector<trelation::intermediate::limit::sort_key>()));
                if (!context_.resolve(limit)) {
                    return {};
                }
                limit.input().connect_to(*output);
                output = limit.output();
            }
        }
        if (set_functions.active()) {
            if (auto project = select_columns) {
                for (auto&& column : project->columns()) {
                    if (!set_functions.process(column.ownership_value())) {
                        return {};
                    }
                    set_functions.add_aggregated(column.variable());
                }
            }
            if (auto project = sort_columns) {
                for (auto&& column : project->columns()) {
                    if (!set_functions.process(column.ownership_value())) {
                        return {};
                    }
                    set_functions.add_aggregated(column.variable());
                }
            }
            auto aggregate_output = set_functions.install(*set_function_entry);
            if (!aggregate_output) {
                return {};
            }
            if (auto project = select_columns) {
                if (!context_.resolve(*project)) {
                    return {};
                }
            }
            if (auto project = sort_columns) {
                if (!context_.resolve(*project)) {
                    return {};
                }
            }

            if (!aggregate_output->opposite()) {
                // it seems the aggregate output is tail - consider it as the query output
                output = aggregate_output;
            }
        }
        return { *output, std::move(info) };
    }

    [[nodiscard]] std::optional<tdescriptor::variable> extract_variable(std::unique_ptr<tscalar::expression> expr) {
        context_.clear_expression_resolution(*expr);
        if (expr->kind() == tscalar::variable_reference::tag) {
            return std::move(unsafe_downcast<tscalar::variable_reference>(*expr).variable());
        }
        return {};
    }

    [[nodiscard]] std::optional<trelation::sort_direction> convert(
            std::optional<ast::common::regioned<ast::common::ordering_specification>> const& source) {
        using from = ast::common::ordering_specification;
        using to = trelation::sort_direction;
        if (!source) {
            return to::ascendant;
        }
        switch (**source) {
            case from::asc:
                return to::ascendant;
            case from::desc:
                return to::descendant;
        }
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported ordering specification: "
                        << *source
                        << string_builder::to_string,
                source.value().region());
        return {};
    }

    [[nodiscard]] result_type operator()(
            ast::query::table_reference const& expr,
            optional_ptr<query_scope const> const&,
            row_value_context const&) {
        auto r = analyze_relation_name(context_, *expr.name());
        if (!r) {
            return {};
        }
        if (r->kind() != ::yugawara::storage::table::tag) {
            // FIXME: impl for views
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    string_builder {}
                            << "unsupported relation kind: "
                            << r->kind()
                            << string_builder::to_string,
                    expr.region());
            return {};
        }
        auto&& table = unsafe_downcast<::yugawara::storage::table>(*r);
        auto info = build_relation_info(context_, table);
        auto index = info.primary_index();
        if (!index) {
            context_.report(
                    sql_analyzer_code::primary_index_not_found,
                    string_builder {}
                            << "missing primary index for table: " << table.simple_name()
                            << string_builder::to_string,
                    expr.region());
            return {};
        }

        auto columns = create_vector<trelation::scan::column>(info.columns().size());
        for (auto&& column : info.columns()) {
            columns.emplace_back(factory_(*column.declaration()), column.variable());
        }

        auto&& op = graph_.emplace<trelation::scan>(
                factory_(*index),
                std::move(columns),
                trelation::scan::endpoint {},
                trelation::scan::endpoint {},
                std::nullopt);
        op.region() = context_.convert(expr.region());
        if (!context_.resolve(op)) {
            return {};
        }

        return { op.output(), std::move(info) };
    }

    [[nodiscard]] result_type operator()(
            ast::query::table_value_constructor const& expr,
            optional_ptr<query_scope const> const& parent,
            row_value_context const& val) {
        if (expr.elements().size() > context_.options()->max_table_value_constructor_rows()) {
            context_.report(
                    sql_analyzer_code::exceed_number_of_elements,
                    string_builder {}
                            << "too many table value constructor elements: "
                            << "must be less than or equal to "
                            << context_.options()->max_table_value_constructor_rows()
                            << string_builder::to_string,
                    expr.region());
            return {};
        }

        // compute row values only which is a row value constructor
        auto rows = create_vector<trelation::values::row>(expr.elements().size());
        for (auto&& row_value : expr.elements()) {
            if (auto row_ctor = as_row_value_constructor(*row_value)) {
                if (row_ctor->elements().size() > context_.options()->max_row_value_constructor_columns()) {
                    context_.report(
                            sql_analyzer_code::exceed_number_of_elements,
                            string_builder {}
                                    << "too many row value constructor elements: "
                                    << "must be less than or equal to "
                                    << context_.options()->max_row_value_constructor_columns()
                                    << string_builder::to_string,
                            row_ctor->region());
                    return {};
                }
                auto row = create_ref_vector<tscalar::expression>(row_ctor->elements().size());
                std::size_t index = 0;
                for (auto&& elem_expr : row_ctor->elements()) {
                    auto elem = analyze_scalar_expression(
                            context_,
                            *elem_expr,
                            query_scope { parent },
                            get_column_value(val, index));
                    if (!elem) {
                        return {};
                    }
                    // FIXME: can appear agg func if this is in a sub-query
                    row.push_back(elem.release());
                    ++index;
                }
                rows.emplace_back(std::move(row));
            } else {
                // FIXME: merge another `values` op and then extract rows by `project`
                context_.report(
                        sql_analyzer_code::unsupported_feature,
                        string_builder {}
                                << "table value constructor element: "
                                << row_value->node_kind()
                                << string_builder::to_string,
                        expr.region());
                return {};
            }
        }

        // compute max number of columns
        auto ncols = std::accumulate(
                rows.begin(),
                rows.end(),
                std::size_t { 0 },
                [](std::size_t c, auto&& r) {
                    return std::max(c, r.elements().size());
                });
        auto columns = create_vector<trelation::values::column>(ncols);
        for (std::size_t i = 0; i < ncols; ++i) {
            auto variable = factory_.stream_variable(
                    debug_string_builder {}
                            << "values_" << i
                            << debug_string_builder::to_string);
            columns.emplace_back(std::move(variable));
        }

        auto&& op = graph_.emplace<trelation::values>(
                std::move(columns),
                std::move(rows));
        op.region() = context_.convert(expr.region());
        if (!context_.resolve(op)) {
            return {};
        }

        relation_info info {};
        info.reserve(ncols, false);
        for (auto&& column : op.columns()) {
            info.add({
                    std::nullopt,
                    column,
                    "", // no name
            });
        }

        return { op.output(), std::move(info) };
    }

    [[nodiscard]] result_type operator()(
            ast::query::binary_expression const& expr,
            optional_ptr<query_scope const> const& parent,
            row_value_context const& val) {
        using kind = ast::query::binary_operator;
        switch (*expr.operator_kind()) {
            case kind::union_:
                return process_union(expr, parent, val);

            case kind::except:
            case kind::intersect:
            case kind::outer_union:
            default:
                break;
        }
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                    << "unsupported binary operator: "
                    << expr.operator_kind()
                    << string_builder::to_string,
                expr.region());
        return {};
    }

    [[nodiscard]] result_type process_union(
            ast::query::binary_expression const& expr,
            optional_ptr<query_scope const> const& parent,
            row_value_context const& val) {
        if (expr.corresponding()) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "CORRESPONDING clause is not yet supported",
                    expr.region());
            return {};
        }
        auto left = process(*expr.left(), parent, val);
        if (!left) {
            return {};
        }
        auto right = process(*expr.right(), parent, val);
        if (!right) {
            return {};
        }

        auto&& left_info = left.relation();
        auto&& right_info = right.relation();

        auto left_columns = extract_exported_columns(left_info.columns());
        auto right_columns = extract_exported_columns(right_info.columns());
        if (left_columns.size() != right_columns.size()) {
            context_.report(
                    sql_analyzer_code::inconsistent_columns,
                    "column count mismatch",
                    expr.operator_kind().region());
            return {};
        }

        trelation::set_quantifier quantifier { trelation::set_quantifier::distinct };
        if (expr.quantifier() == ast::query::set_quantifier::all) {
            quantifier = trelation::set_quantifier::all;
        }

        relation_info output_info {};
        output_info.reserve(left_info.columns().size());

        std::vector<trelation::intermediate::union_::mapping> mappings {};
        mappings.reserve(left_info.columns().size());
        for (std::size_t index = 0, size = left_columns.size(); index < size; ++index) {
            auto&& left_column = *left_columns[index];
            auto&& right_column = *right_columns[index];
            auto output_column = factory_.stream_variable(
                    // FIXME: debug name
            );
            mappings.emplace_back(left_column.variable(), right_column.variable(), output_column);
            output_info.add({
                    {},
                    std::move(output_column),
                    left_column.identifier(), // NOTE: pick the column identifier from the left-most query
            });
        }
        auto&& result = graph_.insert(context_.create<trelation::intermediate::union_>(
                expr.operator_kind().region(),
                quantifier,
                std::move(mappings)));
        result.left() << left.output();
        result.right() << right.output();
        if (!context_.resolve(result)) {
            return {};
        }
        return { result.output(), std::move(output_info) };
    }

    [[nodiscard]] std::vector<column_info const*> extract_exported_columns(
            ::takatori::util::sequence_view<column_info> columns) {
        std::vector<column_info const*> results {};
        results.reserve(columns.size());
        for (auto&& c : columns) {
            if (c.exported()) {
                results.emplace_back(std::addressof(c));
            }
        }
        return results;
    }

    // FIXME: impl with_expression

    // table expressions
    [[nodiscard]] optional_ptr<output_port> operator()(
            ast::table::table_reference const& expr,
            query_scope& scope) {
        // FIXME: refactor - code dup with query::table_reference
        // FIXME: treat is_only()
        auto r = analyze_relation_name(context_, *expr.name());
        if (!r) {
            return {};
        }
        if (r->kind() != ::yugawara::storage::table::tag) {
            // FIXME: impl for views
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    string_builder {}
                            << "unsupported relation kind: "
                            << r->kind()
                            << string_builder::to_string,
                    expr.region());
            return {};
        }
        auto&& table = unsafe_downcast<::yugawara::storage::table>(*r);
        auto info = build_relation_info(context_, table);
        auto index = info.primary_index();
        if (!index) {
            context_.report(
                    sql_analyzer_code::primary_index_not_found,
                    string_builder {}
                            << "missing primary index for table: " << table.simple_name()
                            << string_builder::to_string,
                    expr.region());
            return {};
        }

        auto columns = create_vector<trelation::scan::column>(info.columns().size());
        for (auto&& column : info.columns()) {
            columns.emplace_back(factory_(*column.declaration()), column.variable());
        }

        auto&& op = graph_.emplace<trelation::scan>(
                factory_(*index),
                std::move(columns),
                trelation::scan::endpoint {},
                trelation::scan::endpoint {},
                std::nullopt);
        op.region() = context_.convert(expr.region());
        if (!context_.resolve(op)) {
            return {};
        }

        if (auto corr = expr.correlation()) {
            if (!rebuild_relation_info(info, *corr)) {
                return {};
            }
        }
        scope.add(std::move(info));

        return op.output();
    }

    [[nodiscard]] optional_ptr<output_port> operator()(
            ast::table::unnest const& expr,
            query_scope&) {
        // FIXME: impl
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported table expression: "
                        << expr.node_kind()
                        << string_builder::to_string,
                expr.region());
        return {};
    }

    [[nodiscard]] optional_ptr<output_port> operator()(
            ast::table::join const& expr,
            query_scope& scope) {
        using from = ast::table::join_type;
        using to = trelation::join_kind;
        switch (*expr.operator_kind()) {
            case from::cross: return process_join(to::inner, false, expr, scope);
            case from::inner: return process_join(to::inner, false, expr, scope);
            case from::left_outer: return process_join(to::left_outer, false, expr, scope);
            case from::right_outer: return process_join(to::left_outer, false, expr, scope, true);
            case from::full_outer: return process_join(to::full_outer, false, expr, scope);
            case from::natural_inner: return process_join(to::inner, true, expr, scope);
            case from::natural_left_outer: return process_join(to::left_outer, true, expr, scope);
            case from::natural_right_outer: return process_join(to::left_outer, true, expr, scope, true);
            case from::natural_full_outer: return process_join(to::full_outer, true, expr, scope);
            case from::union_: return process_union_join(expr, scope);
        }
        ::takatori::util::fail();
    }

    [[nodiscard]] optional_ptr<output_port> operator()(
            ast::table::subquery const& expr,
            query_scope& scope) {
        // FIXME: treat is_lateral
        if (expr.is_lateral()) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "LATERAL is not yet supported",
                    expr.region());
            return {};
        }
        query_scope next {};
        auto result = dispatch(*expr.expression(), next, {});
        if (!result) {
            return {};
        }
        auto&& info = result.relation();
        if (!rebuild_relation_info(info, expr.correlation())) {
            return {};
        }
        scope.add(std::move(info));
        return result.output();
    }

    // select_element

    struct select_element_result {
        bool success { false };
        bool saw_aggregate { false };
    };

    [[nodiscard]] std::optional<std::string> maybe_name(std::unique_ptr<ast::name::simple> const& n) noexcept {
        if (n) {
            return normalize_identifier(context_, *n);
        }
        return std::nullopt;
    }

    [[nodiscard]] select_element_result operator()(
            ast::query::select_column const& elem,
            query_scope const& scope,
            trelation::project& target,
            relation_info& info) {
        auto result = analyze_scalar_expression(context_, *elem.value(), scope);
        if (!result) {
            return {};
        }
        std::string name;
        if (auto n = maybe_name(elem.name())) {
            name = std::move(*n);
        } else if (name.empty() && elem.value()->node_kind() == ast::scalar::variable_reference::tag) {
            auto&& var = unsafe_downcast<ast::scalar::variable_reference>(*elem.value());
            name = normalize_identifier(context_, var.name()->last_name());
        }

        auto column = factory_.stream_variable(maybe_name(elem.name()).value_or(""));
        column.region() = result.value().region();
        target.columns().emplace_back(column, result.release());
        info.add({
                {},
                std::move(column),
                std::move(name),
                true,
        });
        return { true, result.saw_aggregate() };
    }

    [[nodiscard]] select_element_result operator()(
            ast::query::select_asterisk const& elem,
            query_scope const& scope,
            trelation::project&,
            relation_info& info) {
        if (auto&& qualifier = elem.qualifier()) {
            if (qualifier->node_kind() != ast::scalar::variable_reference::tag) {
                context_.report(
                        sql_analyzer_code::unsupported_feature,
                        "qualifier must be a plain name",
                        qualifier->region());
                return {};
            }
            auto&& name = unsafe_downcast<ast::scalar::variable_reference>(*qualifier).name();
            auto found = analyze_relation_info_name(context_, *name, scope);
            if (!found) {
                return {}; // error already reported
            }
            for (auto&& column : found->columns()) {
                if (!column.exported()) {
                    continue;
                }
                info.add(column);
            }
            return { true };
        }
        for (auto&& relation : scope.references()) {
            for (auto&& column : relation.columns()) {
                if (!column.exported()) {
                    continue;
                }
                info.add(column);
            }
        }
        return { true };
    }

    // grouping_element

    [[nodiscard]] bool operator()(
            ast::query::grouping_element const& elem,
            set_function_processor&,
            query_scope const&) {
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported grouping element: "
                        << elem.node_kind()
                        << string_builder::to_string,
                elem.region());
        return false;
    }

    [[nodiscard]] bool operator()(
            ast::query::grouping_column const& elem,
            set_function_processor& processor,
            query_scope const& scope) {
        auto r = analyze_scalar_expression(context_, *elem.column(), scope);
        if (!r) {
            return {};
        }
        if (r.saw_aggregate()) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "set functions are not allowed in GROUP BY clause",
                    elem.region());
            return false;
        }
        auto variable = extract_variable(r.release());
        if (!variable) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "plain variable is required in GROUP BY clause",
                    elem.region());
            return false;
        }
        processor.add_group_key(*variable);
        return true;
    }

    // join specification

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::table::join_condition const& elem,
            query_scope const& scope,
            query_scope::position_type) {
        auto r = analyze_scalar_expression(context_, *elem.expression(), scope);
        if (!r) {
            return {};
        }
        if (r.saw_aggregate()) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "aggregated function in JOIN ON clause is not yet supported",
                    elem.region());
            return {};
        }
        return r.release();
    }

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()( // NOLINT(*-function-cognitive-complexity)
            ast::table::join_columns const& elem,
            query_scope& scope,
            query_scope::position_type pivot) {
        std::unique_ptr<tscalar::expression> result {};

        for (auto&& name : elem.columns()) {
            auto&& id = normalize_identifier(context_, *name);
            auto&& rels = scope.references();
            optional_ptr<tdescriptor::variable const> left {};
            for (query_scope::position_type i = 0, n = pivot; i < n; ++i) {
                auto&& rel = rels[i];
                for (auto&& column = rel.find(id); column; column = rel.next(*column)) {
                    // FIXME: check existing
                    if (!column->exported()) {
                        continue;
                    }
                    if (left) {
                        context_.report(
                                sql_analyzer_code::column_ambiguous,
                                string_builder {}
                                        << "column \'" << id << "\" in join left hand side is ambiguous"
                                        << string_builder::to_string,
                                name->region());
                        return {};
                    }
                    left = column->variable();
                }
            }
            if (!left) {
                context_.report(
                        sql_analyzer_code::column_not_found,
                        string_builder {}
                                << "column \'" << id << "\" is not found in join left hand side"
                                << string_builder::to_string,
                        name->region());
                return {};
            }
            optional_ptr<tdescriptor::variable const> right {};
            for (query_scope::position_type i = pivot, n = rels.size(); i < n; ++i) {
                auto&& rel = rels[i];
                for (auto&& column = rel.find(id); column; column = rel.next(*column)) {
                    // FIXME: check existing
                    if (!column->exported()) {
                        continue;
                    }
                    if (right) {
                        context_.report(
                                sql_analyzer_code::column_ambiguous,
                                string_builder {}
                                        << "column \'" << id << "\" in join right hand side is ambiguous"
                                        << string_builder::to_string,
                                name->region());
                        return {};
                    }
                    right = column->variable();

                    // NOTE: hide column in RHS
                    column->exported() = false;
                }
            }
            if (!right) {
                context_.report(
                        sql_analyzer_code::column_not_found,
                        string_builder {}
                                << "column \'" << id << "\" is not found in join left hand side"
                                << string_builder::to_string,
                        name->region());
                return {};
            }

            auto term = context_.create<tscalar::compare>(
                    name->region(),
                    tscalar::comparison_operator::equal,
                    context_.create<tscalar::variable_reference>(
                            name->region(),
                            *left),
                    context_.create<tscalar::variable_reference>(
                            name->region(),
                            *right));
            if (!result) {
                result = std::move(term);
            } else {
                result = context_.create<tscalar::binary>(
                        elem.columns()[0]->region() | name->region(),
                        tscalar::binary_operator::conditional_and,
                        std::move(result),
                        std::move(term));
            }
        }

        if (!result) {
            context_.report(
                    sql_analyzer_code::malformed_syntax,
                    "join columns must not be empty",
                    elem.region());
            return {};
        }
        if (!context_.resolve(*result)) {
            return {};
        }
        return result;
    }

private:
    analyzer_context& context_;
    ::takatori::relation::graph_type& graph_;

    ::yugawara::binding::factory factory_ {};

    [[nodiscard]] result_type dispatch(
            ast::query::expression const& expr,
            optional_ptr<query_scope const> parent,
            row_value_context const& val) {
        return ast::query::dispatch(*this, expr, std::move(parent), val);
    }

    [[nodiscard]] optional_ptr<output_port> dispatch(
            ast::table::expression const& expr,
            query_scope& scope) {
        return ast::table::dispatch(*this, expr, scope);
    }

    [[nodiscard]] select_element_result dispatch(
            ast::query::select_element const& elem,
            query_scope const& scope,
            trelation::project& project,
            relation_info& relation) {
        return ast::query::dispatch(*this, elem, scope, project, relation);
    }

    [[nodiscard]] bool dispatch(
            ast::query::grouping_element const& elem,
            set_function_processor& processor,
            query_scope const& scope) {
        return ast::query::dispatch(*this, elem, processor, scope);
    }

    template<class T>
    [[nodiscard]] std::vector<T> create_vector(std::size_t capacity = 0) {
        std::vector<T> result {};
        if (capacity > 0) {
            result.reserve(capacity);
        }
        return result;
    }

    template<class T>
    [[nodiscard]] reference_vector<T> create_ref_vector(std::size_t capacity = 0) {
        reference_vector<T> result {};
        if (capacity > 0) {
            result.reserve(capacity);
        }
        return result;
    }

    [[nodiscard]] bool rebuild_relation_info(relation_info& info, ast::table::correlation_clause const& correlation) {
        if (correlation.column_names().size() > info.columns().size()) {
            context_.report(
                    sql_analyzer_code::inconsistent_columns,
                    string_builder {}
                            << "too many correlation columns: " << correlation
                            << string_builder::to_string,
                    correlation.region());
            return {};
        }
        info.identifier() = normalize_identifier(context_, *correlation.correlation_name());
        if (!correlation.column_names().empty()) {
            auto&& rel = info.columns();
            auto&& cor = correlation.column_names();
            for (std::size_t index = 0, n = cor.size(); index < n; ++index) {
                rel[index].identifier() = normalize_identifier(context_, *cor[index]);
            }
            // project relation if correlation columns are reduced
            info.erase(cor.size(), rel.size());
            info.rebuild();
        }
        return true;
    }

    [[nodiscard]] std::optional<std::size_t> extract_size(std::unique_ptr<tscalar::expression> expr) {
        context_.clear_expression_resolution(*expr);
        if (expr->kind() == tscalar::immediate::tag) {
            auto&& immediate = unsafe_downcast<tscalar::immediate>(*expr);
            auto value = extract_int_value(immediate.value());
            if (!value || value < 0) {
                context_.report(
                        sql_analyzer_code::invalid_unsigned_integer,
                        string_builder {}
                                << "not a valid unsigned integer: " << immediate.value()
                                << string_builder::to_string,
                        expr->region());
                return {};
            }
            return static_cast<std::size_t>(*value);
        }
        context_.report(
                sql_analyzer_code::invalid_unsigned_integer,
                string_builder {}
                        << "must be a unsigned integer: "
                        << expr->kind()
                        << string_builder::to_string,
                expr->region());
        return {};
    }

    [[nodiscard]] static std::optional<std::int64_t> extract_int_value(tvalue::data const& value) {
        switch (value.kind()) {
            case tvalue::int4::tag: return unsafe_downcast<tvalue::int4>(value).get();
            case tvalue::int8::tag: return unsafe_downcast<tvalue::int8>(value).get();
            default: return {};
        }
    }

    [[nodiscard]] optional_ptr<output_port> process_from_clause(
            node_vector<ast::table::expression> const& elements,
            query_scope& scope) {
        if (elements.empty()) {
            // NOTE: postgres extension: w/o from clause
            // we'll treat it as VALUES()
            auto rows = create_vector<trelation::values::row>(1);
            rows.emplace_back(create_ref_vector<tscalar::expression>());
            auto&& op = graph_.emplace<trelation::values>(
                    create_vector<trelation::values::column>(),
                    std::move(rows));
            if (!context_.resolve(op)) {
                return {};
            }
            scope.add({});
            return op.output();
        }

        optional_ptr<output_port> output {};
        for (auto&& elem : elements) {
            auto next = dispatch(*elem, scope);
            if (!next) {
                return {};
            }
            if (!output) {
                output = std::move(next);
            } else {
                // cross join
                auto&& join = graph_.emplace<trelation::intermediate::join>(trelation::join_kind::inner);
                if (!context_.resolve(join)) {
                    return {};
                }
                join.left().connect_to(*output);
                join.right().connect_to(*next);
                output = join.output();
            }
        }
        return output;
    }

    [[nodiscard]] static optional_ptr<ast::scalar::value_constructor const> as_row_value_constructor(
            ast::scalar::expression const& expr) {
        if (expr.node_kind() == ast::scalar::value_constructor::tag) {
            auto&& row = unsafe_downcast<ast::scalar::value_constructor>(expr);
            if (row.operator_kind() == ast::scalar::value_constructor_kind::row) {
                return row;
            }
        }
        return {};
    }

    [[nodiscard]] static scalar_value_context const& get_column_value(row_value_context const& row, std::size_t index) {
        if (row && index < row.elements().size()) {
            return row.elements()[index];
        }
        static scalar_value_context const empty {};
        return empty;
    }

    [[nodiscard]] optional_ptr<output_port> process_join(
            trelation::join_kind join_type,
            bool natural,
            ast::table::join const& expr,
            query_scope& scope,
            bool transpose = false) {
        auto left = dispatch(*expr.left(), scope);
        if (!left) {
            return {};
        }
        auto pivot = scope.create_pivot();
        auto right = dispatch(*expr.right(), scope);
        if (!right) {
            return {};
        }
        if (transpose) {
            std::swap(left, right);
        }

        std::unique_ptr<tscalar::expression> condition {};
        if (natural) {
            if (expr.specification()) {
                context_.report(
                        sql_analyzer_code::invalid_unsigned_integer,
                        "natural join must not have any more join specifications",
                        expr.specification()->region());
                return {};
            }
            condition = collect_natural_join_columns(expr, scope, pivot);
            if (!condition) {
                return {};
            }
        } else if (expr.specification()) {
            condition = ast::table::dispatch(*this, *expr.specification(), scope, pivot);
            if (!condition) {
                return {};
            }
        }

        auto&& op = graph_.emplace<trelation::intermediate::join>(
                join_type,
                std::move(condition));
        op.region() = context_.convert(expr.region());
        if (!context_.resolve(op)) {
            return {};
        }
        op.left().connect_to(*left);
        op.right().connect_to(*right);
        return op.output();
    }

    [[nodiscard]] std::unique_ptr<tscalar::expression> collect_natural_join_columns(
            ast::table::join const& expr,
            query_scope& scope,
            query_scope::position_type pivot) {
        std::unique_ptr<tscalar::expression> result {};
        ::tsl::hopscotch_set<std::string_view> saw {};

        // FIXME: impl
        (void) scope;
        (void) pivot;
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported natural join: " << expr
                        << string_builder::to_string,
                expr.region());
        return {};
    }

    [[nodiscard]] optional_ptr<output_port> process_union_join(
            ast::table::join const& expr,
            query_scope& scope) {
        auto left = dispatch(*expr.left(), scope);
        if (!left) {
            return {};
        }
        auto pivot = scope.create_pivot();
        auto right = dispatch(*expr.right(), scope);
        if (!right) {
            return {};
        }

        auto mappings = create_vector<trelation::intermediate::union_::mapping>();
        for (query_scope::position_type i = 0, n = pivot; i < n; ++i) {
            for (auto&& relation : scope.references()) {
                for (auto&& column : relation.columns()) {
                    mappings.emplace_back(
                            column.variable(),
                            std::nullopt,
                            factory_.stream_variable());
                }
            }
        }
        for (query_scope::position_type i = pivot, n = scope.references().size(); i < n; ++i) {
            for (auto&& relation : scope.references()) {
                for (auto&& column : relation.columns()) {
                    mappings.emplace_back(
                            std::nullopt,
                            column.variable(),
                            factory_.stream_variable());
                }
            }
        }

        auto&& op = graph_.emplace<trelation::intermediate::union_>(
                trelation::set_quantifier::all,
                std::move(mappings));
        op.region() = context_.convert(expr.region());
        if (!context_.resolve(op)) {
            return {};
        }

        op.left().connect_to(*left);
        op.right().connect_to(*right);
        return op.output();
    }
};

} // namespace

analyze_query_expression_result::analyze_query_expression_result(
        output_port& output,
        relation_info relation) noexcept :
    output_ { output },
    relation_ { std::move(relation) }
{}

bool analyze_query_expression_result::has_value() const noexcept {
    return output_.has_value();
}

analyze_query_expression_result::operator bool() const noexcept {
    return has_value();
}

output_port& analyze_query_expression_result::output() {
    return *output_;
}

relation_info& analyze_query_expression_result::relation() noexcept {
    return relation_;
}

analyze_query_expression_result analyze_query_expression(
        analyzer_context& context,
        trelation::graph_type& graph,
        ast::query::expression const& expression,
        optional_ptr<query_scope const> parent,
        row_value_context const& value_context) {
    engine e { context, graph };
    return e.process(expression, std::move(parent), value_context);
}

[[nodiscard]] relation_info build_relation_info(
        analyzer_context& context,
        ::yugawara::storage::table const& table,
        bool escape_columns,
        bool include_system_columns) {
    relation_info result { table, "" };
    result.reserve(table.columns().size(), true);
    ::yugawara::binding::factory f;
    for (auto&& column : table.columns()) {
        bool system = column.features().contains(::yugawara::storage::column_feature::synthesized);
        bool hidden = column.features().contains(::yugawara::storage::column_feature::hidden);
        // keep the system columns
        if (system && !include_system_columns) {
            continue;
        }
        auto variable = escape_columns ?
                f.stream_variable(
                        debug_string_builder {} << column.simple_name() << debug_string_builder::to_string
                ) :
                f.table_column(column);
        context.resolve_as(variable, column);
        result.add(column_info {
                column,
                std::move(variable),
                column_info::name_type { column.simple_name() },
                !hidden && !system,
        });
    }
    return result;
}

} // namespace mizugaki::analyzer::details
