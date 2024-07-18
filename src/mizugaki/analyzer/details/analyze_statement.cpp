#include <mizugaki/analyzer/details/analyze_statement.h>

#include <vector>

#include <tsl/hopscotch_set.h>

#include <takatori/value/unknown.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/variable_reference.h>
#include <takatori/scalar/cast.h>
#include <takatori/scalar/function_call.h>

#include <takatori/relation/emit.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>
#include <takatori/relation/scan.h>
#include <takatori/relation/values.h>
#include <takatori/relation/write.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/create_table.h>
#include <takatori/statement/create_index.h>
#include <takatori/statement/drop_table.h>
#include <takatori/statement/drop_index.h>
#include <takatori/statement/write.h>

#include <takatori/util/string_builder.h>

#include <yugawara/binding/extract.h>
#include <yugawara/binding/factory.h>
#include <yugawara/binding/table_column_info.h>

#include <mizugaki/ast/statement/dispatch.h>

#include <mizugaki/analyzer/details/analyze_name.h>
#include <mizugaki/analyzer/details/analyze_query_expression.h>
#include <mizugaki/analyzer/details/analyze_scalar_expression.h>
#include <mizugaki/analyzer/details/analyze_type.h>

#include "name_print_support.h"

namespace mizugaki::analyzer::details {

namespace tvalue = ::takatori::value;
namespace tstatement = ::takatori::statement;
namespace trelation = ::takatori::relation;
namespace tscalar = ::takatori::scalar;

using ::takatori::util::optional_ptr;
using ::takatori::util::string_builder;
using ::takatori::util::unsafe_downcast;

using diagnostic_type = analyzer_context::diagnostic_type;
using diagnostic_code = diagnostic_type::code_type;

using result_type = analyze_statement_result_type;

namespace {

class engine {
public:
    explicit engine(analyzer_context& context) noexcept :
            context_ { context }
    {}

    [[nodiscard]] result_type process(ast::statement::statement const& statement) {
        auto r = dispatch(statement);
        return r;
    }

    [[nodiscard]] result_type operator()(ast::statement::statement const& stmt) {
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported statement: "
                        << stmt.node_kind()
                        << string_builder::to_string,
                stmt.region());
        return {};
    }

    [[nodiscard]] result_type operator()(ast::statement::select_statement const& stmt) {
        if (!stmt.targets().empty()) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "target statement is not supported yet",
                    stmt.targets().at(0).region());
            return {};
        }
        auto graph = std::make_unique<::takatori::relation::graph_type>();
        if (!stmt.expression()) {
            context_.report(
                    sql_analyzer_code::malformed_syntax,
                    "SELECT statement must have a query expression",
                    stmt.targets().at(0).region());
            return {};
        }

        auto result = analyze_query_expression(
                context_,
                *graph,
                *stmt.expression(),
                {},
                {});

        if (!result) {
            return {}; // erroneous
        }

        auto&& relation = result.relation();
        auto columns = create_vector<trelation::emit::column>(relation.columns().size());
        for (auto&& rc : relation.columns()) {
            if (rc.exported()) {
                if (auto& id = rc.identifier()) {
                    columns.emplace_back(rc.variable(), *id);
                } else {
                    columns.emplace_back(rc.variable());
                }
            }
        }
        auto&& op = graph->insert(context_.create<trelation::emit>(
                stmt.expression()->region(),
                std::move(columns)));
        op.input().connect_to(result.output());

        return graph;
    }

    [[nodiscard]] result_type operator()(ast::statement::insert_statement const& stmt) { // NOLINT(*-function-cognitive-complexity)
        // <table-expression> -> write[insert]

        auto info_opt = extract_table(*stmt.table_name());
        if (!info_opt) {
            return {}; // error
        }
        auto info = std::move(*info_opt);
        auto index = info.primary_index();

        auto graph = std::make_unique<::takatori::relation::graph_type>();

        auto destination_columns = create_vector<::yugawara::storage::column const*>();
        auto columns_context = create_vector<scalar_value_context>();

        // FIXME: treat default values
        if (stmt.columns().empty()) {
            destination_columns.reserve(info.columns().size());
            columns_context.reserve(info.columns().size());
            for (auto&& column : info.columns()) {
                if (column.exported()) {
                    destination_columns.push_back(column.declaration().get());
                    columns_context.emplace_back(column.declaration()->shared_type());
                }
            }
        } else {
            ::tsl::hopscotch_set<::yugawara::storage::column const*> saw_columns { stmt.columns().size() };
            destination_columns.reserve(stmt.columns().size());
            columns_context.reserve(stmt.columns().size());
            for (auto&& column_name : stmt.columns()) {
                auto identifier = normalize_identifier(context_, *column_name);
                auto column = info.find(identifier);
                if (!column || !column->declaration() || !column->exported()) {
                    // may not ambiguous
                    context_.report(
                            sql_analyzer_code::column_not_found,
                            string_builder {}
                                    << "column is not found: "
                                    << info.primary_index()->table().simple_name()
                                    << "."
                                    << print_support { *column_name }
                                    << string_builder::to_string,
                            column_name->region());
                    return {};
                }
                auto column_ptr = column->declaration().get();
                if (std::find(
                        saw_columns.begin(), saw_columns.end(),
                        column_ptr) != saw_columns.end()) {
                    context_.report(
                            sql_analyzer_code::column_already_exists,
                            string_builder {}
                                    << "duplicate column in insert statement: "
                                    << info.primary_index()->table().simple_name()
                                    << "."
                                    << print_support { *column_name }
                                    << string_builder::to_string,
                            column_name->region());
                    return {};
                }
                saw_columns.insert(column_ptr);
                destination_columns.push_back(column_ptr);
                columns_context.emplace_back(column->declaration()->shared_type());
            }
        }

        if (!stmt.expression()) {
            // FIXME: impl treat "DEFAULT VALUES" clause
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "DEFAULT VALUES clause is yet not supported",
                    stmt.region());
            return {};
        }
        auto source = analyze_query_expression(
                context_,
                *graph,
                *stmt.expression(),
                {},
                row_value_context { std::move(columns_context) });
        if (!source) {
            return {}; // error
        }

        auto&& source_columns = source.relation().columns();
        if (source_columns.size() != destination_columns.size()) {
            context_.report(
                    sql_analyzer_code::inconsistent_columns,
                    string_builder {}
                            << "column count mismatch in insert statement: "
                            << "source-columns="
                            << source_columns.size()
                            << ", "
                            << "destination-columns="
                            << destination_columns.size()
                            << string_builder::to_string,
                    stmt.expression()->region());
            return {};
        }

        auto write_operator = compute_write_type(stmt);
        if (!write_operator) {
            return {};
        }

        if (context_.options()->prefer_write_statement() &&
                source.output().owner().kind() == trelation::expression_kind::values) {
            // FIXME: check no sub-queries in values
            // prefer write statement instead of execute statement with values - write flow.
            auto write_columns = create_vector<tstatement::write::column>(destination_columns.size());
            for (auto column : destination_columns) {
                write_columns.emplace_back(factory_(*column));
            }

            auto&& values = unsafe_downcast<trelation::values>(source.output().owner());
            auto write_tuples = create_vector<tstatement::write::tuple>(values.rows().size());
            for (auto&& row : values.rows()) {
                write_tuples.emplace_back(row.elements().release_elements());
            }
            return context_.create<tstatement::write>(
                    stmt.region(),
                    *write_operator,
                    factory_(index),
                    std::move(write_columns),
                    std::move(write_tuples));
        }

        auto write_tuples = create_vector<trelation::write::column>(destination_columns.size());
        for (std::size_t idx = 0, size = destination_columns.size(); idx < size; ++idx) {
            auto dest = destination_columns[idx];
            auto& src = source_columns[idx];
            write_tuples.emplace_back(src.variable(), factory_(*dest));
        }

        auto key_positions = create_vector<std::size_t>(index->keys().size());
        for (auto&& key : index->keys()) {
            auto iter = std::find(
                    destination_columns.begin(), destination_columns.end(),
                    std::addressof(key.column()));
            if (iter == destination_columns.end()) {
                // NOTE: key columns may not present in insertion columns for default values or implicit columns
                continue;
            }
            auto position = static_cast<std::size_t>(std::distance(destination_columns.begin(), iter));
            key_positions.push_back(position);
        }

        auto write_keys = create_vector<trelation::write::key>(key_positions.size());
        auto write_values = create_vector<trelation::write::key>(destination_columns.size() - key_positions.size());
        for (std::size_t idx = 0, size = destination_columns.size(); idx < size; ++idx) {
            // NOTE: use linear search to keep key ordering
            auto is_key = std::find(key_positions.begin(), key_positions.end(), idx) != key_positions.end();
            if (is_key) {
                write_keys.emplace_back(std::move(write_tuples[idx]));
            } else {
                write_values.emplace_back(std::move(write_tuples[idx]));
            }
        }

        auto&& op_write = graph->insert(context_.create<trelation::write>(
                stmt.table_name()->region(),
                *write_operator,
                factory_(index),
                std::move(write_keys),
                std::move(write_values)));
        op_write.input().connect_to(source.output());

        return graph;
    }

    [[nodiscard]] std::optional<trelation::write_kind> compute_write_type(
            ast::statement::insert_statement const& stmt) {
        using from = ast::statement::insert_statement_option;
        using to = trelation::write_kind;
        auto result = to::insert;
        bool saw = false;
        for (auto&& option : stmt.options()) {
            switch (*option) {
                case from::or_ignore:
                    if (saw) {
                        context_.report(
                                sql_analyzer_code::malformed_syntax,
                                "multiple insert options are not supported",
                                option.region());
                    }
                    saw = true;
                    result = to::insert_skip;
                    break;
                case from::or_replace:
                    if (saw) {
                        context_.report(
                                sql_analyzer_code::malformed_syntax,
                                "multiple insert options are not supported",
                                option.region());
                    }
                    saw = true;
                    result = to::insert_overwrite;
                    break;
                default:
                    context_.report(
                            sql_analyzer_code::unsupported_feature,
                            string_builder {}
                                    << "unsupported insert option: "
                                    << *option
                                    << string_builder::to_string,
                            option.region());
                    return {};
            }
        }

        return result;
    }

    [[nodiscard]] result_type operator()(ast::statement::update_statement const& stmt) {
        // scan (-> filter) -> project -> write[update]
        if (stmt.elements().empty()) {
            context_.report(
                    sql_analyzer_code::malformed_syntax,
                    "empty UPDATE SET is not supported",
                    stmt.region());
            return {};
        }

        auto info_opt = extract_table(*stmt.table_name());
        if (!info_opt) {
            return {}; // error
        }
        query_scope scope {};
        auto&& info = scope.add(std::move(*info_opt));
        auto index = info.primary_index();

        auto graph = std::make_unique<::takatori::relation::graph_type>();
        auto&& op_scan = graph->insert(build_table_scan(info, stmt.table_name()->region()));

        auto* upstream = std::addressof(op_scan.output());

        if (stmt.where()) {
            auto predicate = build_filter(*stmt.where(), scope);
            if (!predicate) {
                return {}; // error
            }
            auto&& op_filter = graph->insert(std::move(predicate));
            op_filter.input().connect_to(*upstream);
            upstream = std::addressof(op_filter.output());
        }

        auto write_keys = build_write_keys(info, stmt.table_name()->region());

        ::tsl::hopscotch_set<::yugawara::storage::column const*> saw_columns { stmt.elements().size() };
        auto projections = create_vector<trelation::project::column>(stmt.elements().size());
        auto write_columns = create_vector<trelation::write::column>(stmt.elements().size());
        for (auto&& element : stmt.elements()) {
            if (element.target()->node_kind() != ast::name::kind::simple) {
                // may not ambiguous
                context_.report(
                        sql_analyzer_code::unsupported_feature,
                        "qualified UPDATE SET target is not supported",
                        element.target()->region());
                return {};
            }

            auto identifier = normalize_identifier(context_, element.target()->last_name());
            auto column = info.find(identifier);
            if (!column || !column->declaration()) {
                context_.report(
                        sql_analyzer_code::column_not_found,
                        string_builder {}
                                << "UPDATE SET target column is not found: "
                                << info.primary_index()->table().simple_name()
                                << "."
                                << print_support { *element.target() }
                                << string_builder::to_string,
                        element.target()->region());
                return {};
            }
            auto column_ptr = column->declaration().get();
            if (std::find(
                    saw_columns.begin(), saw_columns.end(),
                    column_ptr) != saw_columns.end()) {
                context_.report(
                        sql_analyzer_code::column_already_exists,
                        string_builder {}
                                << "duplicate column in update statement: "
                                << info.primary_index()->table().simple_name()
                                << "."
                                << print_support { *element.target() }
                                << string_builder::to_string,
                        element.target()->region());
                return {};
            }
            saw_columns.insert(column_ptr);
            auto variable = factory_.stream_variable(identifier);
            auto value = analyze_scalar_expression(
                    context_,
                    *element.value(),
                    scope,
                    scalar_value_context { column->declaration()->shared_type() });
            if (!value) {
                return {};
            }

            projections.emplace_back(variable, value.release());

            write_columns.emplace_back(std::move(variable), factory_(*column->declaration()));
        }

        auto&& op_project = graph->insert(context_.create<trelation::project>(
                stmt.table_name()->region(),
                std::move(projections)));
        op_project.input().connect_to(*upstream);
        upstream = std::addressof(op_project.output());

        auto&& op_write = graph->insert(context_.create<trelation::write>(
                stmt.table_name()->region(),
                trelation::write_kind::update,
                factory_(index),
                std::move(write_keys),
                std::move(write_columns)));
        op_write.input().connect_to(*upstream);

        return graph;
    }

    [[nodiscard]] result_type operator()(ast::statement::delete_statement const& stmt) {
        // scan (-> filter) -> write[delete]

        auto info_opt = extract_table(*stmt.table_name());
        if (!info_opt) {
            return {}; // error
        }
        query_scope scope {};
        auto&& info = scope.add(std::move(*info_opt));
        auto index = info.primary_index();

        auto graph = std::make_unique<::takatori::relation::graph_type>();
        auto&& op_scan = graph->insert(build_table_scan(info, stmt.table_name()->region()));

        auto* upstream = std::addressof(op_scan.output());

        if (stmt.where()) {
            auto predicate = build_filter(*stmt.where(), scope);
            if (!predicate) {
                return {}; // error
            }
            auto&& op_filter = graph->insert(std::move(predicate));
            op_filter.input().connect_to(*upstream);
            upstream = std::addressof(op_filter.output());
        }

        auto write_keys = build_write_keys(info, stmt.table_name()->region());

        auto&& op_write = graph->insert(context_.create<trelation::write>(
                stmt.table_name()->region(),
                trelation::write_kind::delete_,
                factory_(index),
                std::move(write_keys),
                create_vector<trelation::write::key>()));
        op_write.input().connect_to(*upstream);

        return graph;
    }

    [[nodiscard]] std::optional<relation_info> extract_table(ast::name::name const& table_name) {
        auto relation = analyze_relation_name(context_, table_name);
        if (!relation) {
            return {}; // relation not found
        }
        if (relation->kind() != ::yugawara::storage::table::tag) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    string_builder {}
                            << "unsupported relation type \""
                            << relation->kind()
                            << "\": "
                            << print_support { table_name }
                            << string_builder::to_string,
                    table_name.region());
            return {};
        }
        auto&& table = unsafe_downcast<::yugawara::storage::table>(*relation);
        auto info = build_relation_info(context_, table, true, true);
        if (!info.primary_index()) {
            context_.report(
                    sql_analyzer_code::primary_index_not_found,
                    string_builder {}
                            << "missing primary index for table: " << table.simple_name()
                            << string_builder::to_string,
                    table_name.region());
            return {};
        }
        return info;
    }

    [[nodiscard]] std::unique_ptr<trelation::scan> build_table_scan(
            relation_info const& info,
            ast::node_region region) {
        std::vector<trelation::scan::column> scan_columns {};
        if (!info.columns().empty()) {
            scan_columns.reserve(info.columns().size());
        }
        for (auto&& column : info.columns()) {
            scan_columns.emplace_back(factory_(*column.declaration()), column.variable());
        }
        auto result = context_.create<trelation::scan>(
                region,
                factory_(*info.primary_index()),
                std::move(scan_columns),
                trelation::scan::endpoint {},
                trelation::scan::endpoint {},
                std::nullopt);
        return result;
    }

    [[nodiscard]] std::unique_ptr<trelation::filter> build_filter(
            ast::scalar::expression const& expr,
            query_scope const& scope) {
        auto predicate = analyze_scalar_expression(
                context_,
                expr,
                scope,
                {});
        if (!predicate) {
            return {}; // error
        }
        if (predicate.saw_aggregate()) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "aggregated function in WHERE clause is not yet supported",
                    expr.region());
            return {};
        }
        auto result = context_.create<trelation::filter>(
                expr.region(),
                predicate.release());
        return result;
    }

    [[nodiscard]] std::vector<trelation::write::column> build_write_keys(
            relation_info const& info,
            ast::node_region region) {
        auto&& index = info.primary_index();
        auto write_keys = create_vector<trelation::write::key>(index->keys().size());
        for (auto&& k : index->keys()) {
            auto column = info.find(k.column());
            if (!column) { // may not occur
                context_.report(
                        sql_analyzer_code::column_not_found,
                        string_builder {}
                                << "missing column for primary key: " << k.column().simple_name()
                                << string_builder::to_string,
                        region);
                return {};
            }
            write_keys.emplace_back(column->variable(), factory_(k.column()));
        }
        if (write_keys.empty()) {
            context_.report(
                    sql_analyzer_code::unknown,
                    string_builder {}
                            << "primary index has no keys: " << index->table().simple_name()
                            << string_builder::to_string,
                    region);
            return {};
        }
        return write_keys;
    }

    [[nodiscard]] result_type operator()(ast::statement::table_definition const& stmt) { // NOLINT(*-function-cognitive-complexity)
        // check features
        if (!stmt.parameters().empty()) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "table parameters are not supported yet",
                    stmt.parameters().at(0).name()->region());
            return {};
        }

        // check existing table
        if (auto existing = analyze_table_name(context_, *stmt.name(), false)) {
            if (find(stmt.options(), ast::statement::table_definition_option::if_not_exists)) {
                return context_.create<tstatement::empty>(stmt.region());
            }
            context_.report(
                    sql_analyzer_code::table_already_exists,
                    string_builder {}
                            << "table is already defined: "
                            << print_support { *stmt.name() }
                            << string_builder::to_string,
                    stmt.name()->region());
            return {};
        }

        if (stmt.name()->node_kind() != ast::name::kind::simple) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "qualified table table_name is not supported",
                    stmt.name()->region());
            return {};
        }

        auto table_name = normalize_identifier(context_, stmt.name()->last_name());

        // extract columns
        ::takatori::util::reference_vector<::yugawara::storage::column> table_columns {};
        ::tsl::hopscotch_set<std::string_view> saw_columns {};
        table_columns.reserve(stmt.elements().size());
        saw_columns.reserve(stmt.elements().size());
        optional_ptr<::yugawara::storage::column const> primary_key_column {};
        for (auto&& element : stmt.elements()) {
            if (element->node_kind() != ast::statement::table_element_kind::column_definition) {
                continue;
            }
            auto&& column = unsafe_downcast<ast::statement::column_definition>(*element);
            auto column_name = normalize_identifier(context_, *column.name());
            auto column_type = analyze_type(context_, *column.type());
            if (!column_type) {
                return {}; // error
            }
            if (std::find(
                    saw_columns.begin(), saw_columns.end(),
                    column_name) != saw_columns.end()) {
                context_.report(
                        sql_analyzer_code::column_already_exists,
                        string_builder {}
                                << "duplicate column in table definition: "
                                << table_name
                                << "."
                                << column_name
                                << string_builder::to_string,
                        column.name()->region());
                return {};
            }
            auto prototype = std::make_unique<::yugawara::storage::column>(
                    std::move(column_name),
                    std::move(column_type),
                    ::yugawara::variable::criteria {},
                    ::yugawara::storage::column_value {},
                    ::yugawara::storage::column_feature_set {});
            saw_columns.insert(prototype->simple_name());
            bool saw_nullity = false;
            for (auto&& constraint_def : column.constraints()) {
                // NOTE: restricted combinations of column constraints
                // - PRIMARY KEY + PRIMARY KEY
                // - PRIMARY KEY + NULL
                // - NULL + NULL
                // - NULL + NOT NULL
                // - NOT NULL + NOT NULL
                // - DEFAULT + DEFAULT
                auto&& constraint = *constraint_def.body();
                using kind = ast::statement::constraint_kind;
                switch (constraint.node_kind()) {
                    case kind::primary_key:
                        // capture primary indices
                        if (primary_key_column) {
                            context_.report(
                                    sql_analyzer_code::primary_index_already_exists,
                                    "multiple primary keys are not supported",
                                    constraint.region());
                            return {};
                        }
                        if (saw_nullity && prototype->criteria().nullity() == ::yugawara::variable::nullable) {
                            context_.report(
                                    sql_analyzer_code::invalid_constraint,
                                    "primary key must not be NULL-able",
                                    constraint.region());
                            return {};
                        }
                        prototype->criteria().nullity(~::yugawara::variable::nullable);
                        primary_key_column.reset(prototype.get());
                        break;
                    case kind::null:
                        if (saw_nullity) {
                            context_.report(
                                    sql_analyzer_code::invalid_constraint,
                                    "multiple NULL-ity constraints are not supported",
                                    constraint.region());
                            return {};
                        }
                        if (prototype->criteria().nullity() == ~::yugawara::variable::nullable) {
                            context_.report(
                                    sql_analyzer_code::invalid_constraint,
                                    "primary key must not be NULL-able",
                                    constraint.region());
                            return {};
                        }
                        saw_nullity = true;
                        prototype->criteria().nullity(::yugawara::variable::nullable);
                        break;
                    case kind::not_null:
                        if (saw_nullity) {
                            context_.report(
                                    sql_analyzer_code::invalid_constraint,
                                    "multiple NULL-ity constraints are not supported",
                                    constraint.region());
                            return {};
                        }
                        saw_nullity = true;
                        prototype->criteria().nullity(~::yugawara::variable::nullable);
                        break;
                    case kind::default_clause:
                        if (prototype->default_value()) {
                            context_.report(
                                    sql_analyzer_code::invalid_constraint,
                                    "multiple default values are not supported",
                                    constraint.region());
                            return {};
                        }
                        {
                            auto result = process_column_value(
                                    unsafe_downcast<ast::statement::expression_constraint>(constraint),
                                    *prototype);
                            if (!result) {
                                return {};
                            }
                            prototype->default_value() = std::move(result.value());
                        }
                        break;
                    default:
                        // FIXME: impl more column constraints
                        context_.report(
                                sql_analyzer_code::unsupported_feature,
                                string_builder {}
                                        << "unsupported column constraint_def: "
                                        << constraint.node_kind()
                                        << string_builder::to_string,
                                constraint.region());
                        return {};
                }
            }
            table_columns.push_back(std::move(prototype));
        }
        if (table_columns.empty()) {
            context_.report(
                    sql_analyzer_code::malformed_syntax,
                    "table definition must have at least one column",
                    stmt.region());
            return {};
        }
        saw_columns.clear();

        auto declaration = std::make_shared<::yugawara::storage::table>(
                std::nullopt,
                std::move(table_name),
                std::move(table_columns));

        std::shared_ptr<::yugawara::storage::index> primary_index {};
        if (primary_key_column) {
            auto index_keys = create_vector<::yugawara::storage::index::key>(1);
            index_keys.emplace_back(*primary_key_column);
            primary_index = std::make_shared<::yugawara::storage::index>(
                    std::nullopt,
                    declaration,
                    std::string {}, // empty name for primary indices
                    std::move(index_keys),
                    std::vector<::yugawara::storage::index::column_ref>(),
                    ::yugawara::storage::index_feature_set {
                            ::yugawara::storage::index_feature::primary,
                    });
        }

        for (auto&& element : stmt.elements()) {
            if (element->node_kind() != ast::statement::table_element_kind::constraint_definition) {
                continue;
            }
            auto&& constraint_def = unsafe_downcast<ast::statement::table_constraint_definition>(*element);
            auto&& constraint = *constraint_def.body();
            using kind = ast::statement::constraint_kind;
            switch (constraint.node_kind()) {
                case kind::primary_key:
                    if (primary_index) {
                        context_.report(
                                sql_analyzer_code::primary_index_already_exists,
                                "multiple primary keys are not supported",
                                constraint.region());
                        return {};
                    }
                    {
                        auto result = process_primary_key(
                                unsafe_downcast<ast::statement::key_constraint>(constraint),
                                declaration);
                        if (!result) {
                            return {};
                        }
                        primary_index = std::move(result);
                        primary_index->features().insert(::yugawara::storage::index_feature::primary);

                        // primary key columns are implicitly not nullable
                        for (auto&& key : primary_index->keys()) {
                            if (key.column().criteria().nullity() == ::yugawara::variable::nullable) {
                                auto&& columns = declaration->columns();
                                // FIXME: warn if not found
                                if (auto iter = std::find(columns.begin(), columns.end(), key.column()); iter != columns.end()) {
                                    iter->criteria().nullity(~::yugawara::variable::nullable);
                                }
                            }
                        }
                    }
                    break;

                default:
                    // FIXME: impl more column constraints
                    context_.report(
                            sql_analyzer_code::unsupported_feature,
                            string_builder {}
                                    << "unsupported column constraint_def: "
                                    << constraint.node_kind()
                                    << string_builder::to_string,
                            constraint.region());
                    return {};
            }
        }
        if (!primary_index) {
            primary_index = std::make_shared<::yugawara::storage::index>(
                    std::nullopt,
                    declaration,
                    std::string {}, // empty name for primary indices
                    std::vector<::yugawara::storage::index::key>(),
                    std::vector<::yugawara::storage::index::column_ref>(),
                    ::yugawara::storage::index_feature_set {
                            ::yugawara::storage::index_feature::primary,
                    });
        }
        // fill implicit column default values
        for (auto&& column : declaration->columns()) {
            if (column.criteria().nullity() == ::yugawara::variable::nullable && !column.default_value()) {
                column.default_value() = { context_.values().get(tvalue::unknown {}) };
            }
        }

        auto result = context_.create<tstatement::create_table>(
                stmt.region(),
                factory_(context_.options()->default_schema_shared()),
                factory_(std::move(declaration)),
                factory_(std::move(primary_index)));

        return result;
    }

    [[nodiscard]] std::optional<::yugawara::storage::column_value> process_column_value(
            ast::statement::expression_constraint const& constraint,
            ::yugawara::storage::column const& prototype) {
        auto result = analyze_scalar_expression(
                context_,
                *constraint.expression(),
                {},
                { prototype.shared_type() });
        if (!result) {
            return {}; // error
        }
        auto value = extract_column_value(constraint, result.release());
        return value;
    }

    [[nodiscard]] std::optional<::yugawara::storage::column_value> extract_column_value(
            ast::statement::expression_constraint const& constraint,
            std::unique_ptr<tscalar::expression> expr) {
        context_.clear_expression_resolution(*expr);
        using kind = tscalar::expression_kind;
        switch (expr->kind()) {
            case kind::immediate:
            {
                auto&& value = unsafe_downcast<tscalar::immediate>(*expr);
                return ::yugawara::storage::column_value { context_.values().get(value.value()) };
            }
            case kind::function_call:
            {
                auto&& value = unsafe_downcast<tscalar::function_call>(*expr);
                if (!value.arguments().empty()) {
                    context_.report(
                            sql_analyzer_code::unsupported_feature,
                            "function call in default value must not have any parameters",
                            constraint.expression()->region());
                    return {};
                }
                auto&& function = ::yugawara::binding::extract_shared(value.function());
                return ::yugawara::storage::column_value { std::move(function) };
            }
            case kind::cast:
            {
                auto&& e = unsafe_downcast<tscalar::cast>(*expr);
                // FIXME: impl casting
                return extract_column_value(constraint, e.release_operand());
            }
            default:
                context_.report(
                        sql_analyzer_code::unsupported_feature,
                        "default value must be a literal",
                        constraint.expression()->region());
                return {};
        }
    }

    [[nodiscard]] std::shared_ptr<::yugawara::storage::index> process_primary_key(
            ast::statement::key_constraint const& constraint,
            std::shared_ptr<::yugawara::storage::table const> table) {
        // check features
        if (!constraint.values().empty()) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "index values are not supported yet",
                    constraint.values().at(0)->region());
            return {};
        }
        if (!constraint.parameters().empty()) {
            context_.report(
                    sql_analyzer_code::malformed_syntax,
                    "index parameters are not supported yet",
                    constraint.parameters().at(0).name()->region());
        }

        query_scope scope {};
        scope.add(build_relation_info(context_, *table, false));

        auto index_keys = build_index_keys(constraint.key(), scope, constraint.region());
        if (index_keys.empty()) {
            return {};
        }

        auto declaration = std::make_shared<::yugawara::storage::index>(
                std::nullopt,
                std::move(table),
                std::string {},
                std::move(index_keys),
                std::vector<::yugawara::storage::index::column_ref>(),
                ::yugawara::storage::index_feature_set {});

        return declaration;
    }

    [[nodiscard]] std::vector<::yugawara::storage::index::key> build_index_keys(
            std::vector<ast::common::sort_element> const& source_keys,
            query_scope const& scope,
            ast::node_region region) {
        ::tsl::hopscotch_set<::yugawara::storage::column const*> saw_columns { source_keys.size() };
        auto index_keys = create_vector<::yugawara::storage::index::key>(source_keys.size());
        for (auto&& key : source_keys) {
            if (auto&& p = key.null_location()) {
                // FIXME: impl NULLS FIRST/LAST for indices
                context_.report(
                        sql_analyzer_code::unsupported_feature,
                        "index key element with NULLS location is not supported",
                        p->region());
                return {};
            }

            auto result = analyze_scalar_expression(
                    context_,
                    *key.key(),
                    scope,
                    {});
            if (!result) {
                return {};
            }
            auto expr = result.release();
            context_.clear_expression_resolution(*expr);
            if (expr->kind() != tscalar::expression_kind::variable_reference) {
                context_.report(
                        sql_analyzer_code::unsupported_feature,
                        "index key element must be a table column",
                        key.key()->region());
                return {};
            }
            auto&& variable = unsafe_downcast<tscalar::variable_reference>(*expr).variable();
            auto binding = ::yugawara::binding::extract_if<::yugawara::storage::column>(variable);
            if (!binding) {
                context_.report(
                        sql_analyzer_code::unsupported_feature,
                        "index key element must be a table column",
                        key.key()->region());
                return {};
            }
            if (std::find(
                    saw_columns.begin(), saw_columns.end(),
                    binding.get()) != saw_columns.end()) {
                context_.report(
                        sql_analyzer_code::column_already_exists,
                        string_builder {}
                                << "duplicate column in index definition: "
                                << binding->simple_name()
                                << string_builder::to_string,
                        key.key()->region());
                return {};
            }
            saw_columns.insert(binding.get());
            index_keys.emplace_back(*binding, to_sort_direction(key.direction()));
        }
        if (index_keys.empty()) {
            context_.report(
                    sql_analyzer_code::malformed_syntax,
                    "index with empty key is not supported",
                    region);
            return {};
        }
        return index_keys;
    }

    [[nodiscard]] result_type operator()(ast::statement::index_definition const& stmt) {
        // check features
        if (!stmt.values().empty()) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "index values are not supported yet",
                    stmt.values().at(0)->region());
            return {};
        }
        if (stmt.predicate()) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "index predicate is not supported yet",
                    stmt.predicate()->region());
            return {};
        }
        if (auto opt = find(stmt.options(), ast::statement::index_definition_option::unique)) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "unique index is not supported yet",
                    opt->region());
            return {};
        }
        if (!stmt.parameters().empty()) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "index parameters are not supported yet",
                    stmt.parameters().at(0).name()->region());
        }

        // check existing
        if (stmt.name()) {
            if (auto existing = analyze_index_name(context_, *stmt.name(), false)) {
                if (find(stmt.options(), ast::statement::index_definition_option::if_not_exists)) {
                    return context_.create<tstatement::empty>(stmt.region());
                }
                context_.report(
                        sql_analyzer_code::index_already_exists,
                        string_builder {}
                                << "index is already defined: "
                                << print_support { *stmt.name() }
                                << string_builder::to_string,
                        stmt.name()->region());
                return {};
            }
        }

        auto table_result = analyze_table_name(context_, *stmt.table_name());
        if (!table_result) {
            return {}; // error already reported
        }

        auto&& table_ptr = table_result->second;

        ast::node_region region;
        std::string name {};
        if (stmt.name()) {
            // FIXME: impl qualified index name
            if (stmt.name()->node_kind() != ast::name::kind::simple) {
                context_.report(
                        sql_analyzer_code::unsupported_feature,
                        "qualified index name is not supported",
                        stmt.name()->region());
                return {};
            }
            name = normalize_identifier(context_, stmt.name()->last_name());
            region = stmt.name()->region();
        }

        query_scope scope {};
        scope.add(build_relation_info(context_, *table_ptr, false));

        auto index_keys = build_index_keys(stmt.keys(), scope, region);
        if (index_keys.empty()) {
            return {};
        }

        auto declaration = std::make_shared<::yugawara::storage::index>(
                std::nullopt,
                table_result->second,
                std::move(name),
                std::move(index_keys),
                std::vector<::yugawara::storage::index::column_ref>(),
                ::yugawara::storage::index_feature_set  {});

        auto result = context_.create<tstatement::create_index>(
                stmt.region(),
                factory_.schema(context_.options()->default_schema_shared()),
                factory_(std::move(declaration)));

        return result;
    }

    [[nodiscard]] trelation::sort_direction to_sort_direction(
            std::optional<ast::common::regioned<ast::common::ordering_specification>> const& direction) {
        if (direction && *direction == ast::common::ordering_specification::desc) {
            return trelation::sort_direction::descendant;
        }
        return trelation::sort_direction::ascendant;
    }

    template<class T>
    [[nodiscard]] static std::optional<ast::common::regioned<T>> find(
            std::vector<ast::common::regioned<T>> const& container,
            T const& value) {
        auto iter = std::find(
                container.begin(), container.end(),
                value);
        if (iter != container.end()) {
            return *iter;
        }
        return {};
    }

// FIXME: impl

//    [[nodiscard]] result_type operator()(ast::statement::view_definition const& stmt) {
//        (void) stmt;
//        return {};
//    }
//
//    [[nodiscard]] result_type operator()(ast::statement::sequence_definition const& stmt) {
//        (void) stmt;
//        return {};
//    }
//
//    [[nodiscard]] result_type operator()(ast::statement::schema_definition const& stmt) {
//        (void) stmt;
//        return {};
//    }

    [[nodiscard]] result_type operator()(ast::statement::drop_statement const& stmt) {
        auto cascade = find(stmt.options(), ast::statement::drop_statement_option::cascade);
        auto restrict = find(stmt.options(), ast::statement::drop_statement_option::restrict);
        if (cascade && restrict) {
            context_.report(
                    sql_analyzer_code::malformed_syntax,
                    "DROP statement must not declare both CASCADE and RESTRICT",
                    cascade->region());
            return {};
        }

        using tag_t = ast::statement::kind;
        switch (stmt.node_kind()) {
            case tag_t::drop_table_statement:
                return process_drop_table(stmt);
            case tag_t::drop_index_statement:
                return process_drop_index(stmt);
            default:
                break;
        }
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported statement: "
                        << stmt.node_kind()
                        << string_builder::to_string,
                stmt.region());
        return {};
    }

    [[nodiscard]] result_type process_drop_table(ast::statement::drop_statement const& stmt) {
        if (auto opt = find(stmt.options(), ast::statement::drop_statement_option::cascade)) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "CASCADE option in DROP TABLE statement is not supported yet",
                    opt->region());
            return {};
        }
        // NOTE: restrict option is default behavior

        auto optional = find(stmt.options(), ast::statement::drop_statement_option::if_exists).has_value();

        auto result = analyze_table_name(context_, *stmt.name(), !optional);
        if (!result) {
            if (optional) {
                // NOTE: replace as no-op instead, to achieve "IF EXISTS"
                return context_.create<tstatement::empty>(stmt.region());
            }
            // NOTE: error already reported
            return {};
        }
        auto schema = std::move(result->first);
        auto target = std::move(result->second);
        return context_.create<tstatement::drop_table>(
                stmt.region(),
                factory_.schema(std::move(schema).ownership()),
                factory_.storage(std::move(target)));
    }

    [[nodiscard]] result_type process_drop_index(ast::statement::drop_statement const& stmt) {
        if (auto opt = find(stmt.options(), ast::statement::drop_statement_option::cascade)) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "CASCADE option in DROP TABLE statement is not supported yet",
                    opt->region());
            return {};
        }
        // NOTE: restrict option is default behavior

        auto optional = find(stmt.options(), ast::statement::drop_statement_option::if_exists).has_value();

        auto result = analyze_index_name(context_, *stmt.name(), !optional);
        if (!result) {
            if (optional) {
                // NOTE: replace as no-op instead, to achieve "IF EXISTS"
                return context_.create<tstatement::empty>(stmt.region());
            }
            // NOTE: error already reported
            return {};
        }
        auto schema = std::move(result->first);
        auto target = std::move(result->second);
        return context_.create<tstatement::drop_index>(
                stmt.region(),
                factory_.schema(std::move(schema).ownership()),
                factory_.index(std::move(target)));
    }

    [[nodiscard]] result_type operator()(ast::statement::empty_statement const& stmt) {
        return context_.create<tstatement::empty>(stmt.region());
    }

private:
    analyzer_context& context_; // NOLINT(*-avoid-const-or-ref-data-members)
    ::yugawara::binding::factory factory_;

    [[nodiscard]] result_type dispatch(ast::statement::statement const& statement) {
        return ast::statement::dispatch(*this, statement);
    }

    template<class T>
    [[nodiscard]] std::vector<T> create_vector(std::size_t capacity = 0) {
        std::vector<T> result {};
        if (capacity > 0) {
            result.reserve(capacity);
        }
        return result;
    }
};

} // namespace

analyze_statement_result_type analyze_statement(
        analyzer_context& context,
        ast::statement::statement const& statement) {
    engine e { context };
    auto result = e.process(statement);
    return result;
}

} // namespace mizugaki::analyzer::details
