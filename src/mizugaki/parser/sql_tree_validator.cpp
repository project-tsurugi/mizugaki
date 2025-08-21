#include "sql_tree_validator.h"

#include <algorithm>

#include <takatori/util/optional_ptr.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/statement/dispatch.h>
#include <mizugaki/ast/query/dispatch.h>
#include <mizugaki/ast/table/dispatch.h>
#include <mizugaki/ast/scalar/dispatch.h>
#include <mizugaki/ast/literal/dispatch.h>
#include <mizugaki/ast/type/dispatch.h>
#include <mizugaki/ast/name/dispatch.h>

#define ACCEPT(e) if (!accept((e), depth)) return false

namespace mizugaki::parser {

namespace {

class engine {
public:
    explicit engine(
            ::takatori::util::maybe_shared_ptr<::takatori::document::document const> document,
            std::size_t node_limit,
            std::size_t depth_limit) noexcept :
        document_ { std::move(document) },
        node_limit_ { node_limit },
        depth_limit_ { depth_limit }
    {}

    void clear() noexcept {
        result_ = {};
        node_count_ = 0;
        max_depth_ = 0;
    }

    [[nodiscard]] std::optional<sql_parser_diagnostic> process(
            ast::compilation_unit const& element,
            std::size_t depth) {
        clear();
        for (auto&& child : element.statements()) {
            if (!accept(child, depth)) {
                break;
            }
        }
        return std::move(result_);
    }

    [[nodiscard]] std::size_t node_count() const noexcept {
        return node_count_;
    }

    [[nodiscard]] std::size_t max_depth() const noexcept {
        return max_depth_;
    }

    template<class T>
    [[nodiscard]] bool check(T const& element, std::size_t depth) {
        ++node_count_;
        max_depth_ = std::max(depth, max_depth_);
        if (node_limit_ > 0 && node_count_ > node_limit_) {
            using ::takatori::util::string_builder;
            result_ = sql_parser_diagnostic {
                    sql_parser_code::exceed_number_of_elements,
                    string_builder {}
                            << "exceeds the max number of elements in the SQL syntax tree: "
                            << "count=" << node_count_
                            << ", kind=" << element.node_kind()
                            << string_builder::to_string,
                    document_,
                    element.region(),
            };
            return false;
        }
        if (depth_limit_ > 0 && depth > depth_limit_) {
            using ::takatori::util::string_builder;
            result_ = sql_parser_diagnostic {
                    sql_parser_code::exceed_number_of_elements,
                    string_builder {}
                        << "exceeds the max depth of the SQL syntax tree: "
                        << "depth=" << depth
                        << ", kind=" << element.node_kind()
                        << string_builder::to_string,
                    document_,
                    element.region(),
            };
            return false;
        }
        return true;
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::statement::statement> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::statement::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::statement::table_element> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::statement::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::statement::constraint> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::statement::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::query::expression> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::query::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::query::select_element> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::query::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::query::grouping_element> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::query::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::table::expression> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::table::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::table::join_specification> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::table::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::scalar::expression> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::scalar::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::literal::literal> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::literal::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::type::type> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::type::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::name::name> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return ast::name::dispatch(*this, *element, depth + 1);
    }

    [[nodiscard]] bool accept(std::unique_ptr<ast::name::simple> const& element, std::size_t depth) {
        if (!element) {
            return true;
        }
        if (!check(*element, depth + 1)) {
            return false;
        }
        return operator()(*element, depth + 1);
    }

    template<class E>
    [[nodiscard]] bool accept(std::vector<std::unique_ptr<E>> const& elements, std::size_t depth) {
        for (auto&& element: elements) { // NOLINT(*-use-anyofallof)
            ACCEPT(element);
        }
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::empty_statement const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::select_statement const& element, std::size_t depth) {
        ACCEPT(element.expression());
        for (auto&& target : element.targets()) { // NOLINT(*-use-anyofallof)
            ACCEPT(target.target());
            ACCEPT(target.indicator());
        }
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::insert_statement const& element, std::size_t depth) {
        ACCEPT(element.table_name());
        ACCEPT(element.columns());
        ACCEPT(element.expression());
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::update_statement const& element, std::size_t depth) {
        ACCEPT(element.table_name());
        for (auto&& clause : element.elements()) { // NOLINT(*-use-anyofallof)
            ACCEPT(clause.target());
            ACCEPT(clause.value());
        }
        ACCEPT(element.where()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::delete_statement const& element, std::size_t depth) {
        ACCEPT(element.table_name());
        ACCEPT(element.where());
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::table_definition const& element, std::size_t depth) {
        ACCEPT(element.name());
        ACCEPT(element.elements());
        for (auto&& parameter : element.parameters()) { // NOLINT(*-use-anyofallof)
            ACCEPT(parameter.name());
            ACCEPT(parameter.value());
        }
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::index_definition const& element, std::size_t depth) {
        ACCEPT(element.name());
        ACCEPT(element.table_name());
        for (auto&& key : element.keys()) { // NOLINT(*-use-anyofallof)
            ACCEPT(key.key());
            ACCEPT(key.collation());
        }
        ACCEPT(element.values());
        ACCEPT(element.predicate());
        for (auto&& parameter : element.parameters()) { // NOLINT(*-use-anyofallof)
            ACCEPT(parameter.name());
            ACCEPT(parameter.value());
        }
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::view_definition const& element, std::size_t depth) {
        ACCEPT(element.name());
        ACCEPT(element.columns());
        ACCEPT(element.query());
        for (auto&& parameter : element.parameters()) { // NOLINT(*-use-anyofallof)
            ACCEPT(parameter.name());
            ACCEPT(parameter.value());
        }
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::sequence_definition const& element, std::size_t depth) {
        ACCEPT(element.name());
        ACCEPT(element.type());
        ACCEPT(element.initial_value());
        ACCEPT(element.increment_value());
        ACCEPT(element.min_value());
        ACCEPT(element.max_value());
        ACCEPT(element.owner());
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::schema_definition const& element, std::size_t depth) {
        ACCEPT(element.name());
        ACCEPT(element.user_name());
        ACCEPT(element.elements());
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::drop_statement const& element, std::size_t depth) {
        ACCEPT(element.name()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::grant_privilege_statement const& element, std::size_t depth) {
        for (auto&& object : element.objects()) {
            ACCEPT(object.object_name());
        }
        for (auto&& user : element.users()) { // NOLINT(*-use-anyofallof)
            ACCEPT(user.authorization_identifier());
        }
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::revoke_privilege_statement const& element, std::size_t depth) {
        for (auto&& object : element.objects()) {
            ACCEPT(object.object_name());
        }
        for (auto&& user : element.users()) { // NOLINT(*-use-anyofallof)
            ACCEPT(user.authorization_identifier());
        }
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::column_definition const& element, std::size_t depth) {
        ACCEPT(element.name());
        ACCEPT(element.type());
        for (auto&& constraint : element.constraints()) { // NOLINT(*-use-anyofallof)
            ACCEPT(constraint.name());
            ACCEPT(constraint.body());
        }
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::table_constraint_definition const& element, std::size_t depth) {
        ACCEPT(element.name());
        ACCEPT(element.body());
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::simple_constraint const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::expression_constraint const& element, std::size_t depth) {
        ACCEPT(element.expression()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::key_constraint const& element, std::size_t depth) {
        for (auto&& sort : element.key()) { // NOLINT(*-use-anyofallof)
            ACCEPT(sort.key());
            ACCEPT(sort.collation());
        }
        ACCEPT(element.values());
        for (auto&& parameter : element.parameters()) { // NOLINT(*-use-anyofallof)
            ACCEPT(parameter.name());
            ACCEPT(parameter.value());
        }
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::referential_constraint const& element, std::size_t depth) {
        ACCEPT(element.columns());
        ACCEPT(element.target());
        ACCEPT(element.target_columns());
        return true;
    }

    [[nodiscard]] bool operator()(ast::statement::identity_constraint const& element, std::size_t depth) {
        ACCEPT(element.initial_value());
        ACCEPT(element.increment_value());
        ACCEPT(element.min_value());
        ACCEPT(element.max_value());
        return true;
    }

    [[nodiscard]] bool operator()(ast::query::query const& element, std::size_t depth) {
        ACCEPT(element.elements());
        ACCEPT(element.from());
        ACCEPT(element.where());
        if (auto&& clause = element.group_by()) {
            ACCEPT(clause->elements());
        }
        ACCEPT(element.having());
        for (auto&& clause : element.order_by()) {
            ACCEPT(clause.key());
            ACCEPT(clause.collation());
        }
        ACCEPT(element.limit()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::query::table_reference const& element, std::size_t depth) {
        ACCEPT(element.name()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::query::table_value_constructor const& element, std::size_t depth) {
        ACCEPT(element.elements()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::query::binary_expression const& element, std::size_t depth) {
        ACCEPT(element.left());
        if (auto&& clause = element.corresponding()) {
            ACCEPT(clause->column_names());
        }
        ACCEPT(element.right());
        return true;
    }

    [[nodiscard]] bool operator()(ast::query::with_expression const& element, std::size_t depth) {
        for (auto&& clause: element.elements()) { // NOLINT(*-use-anyofallof)
            ACCEPT(clause.name());
            ACCEPT(clause.column_names());
            ACCEPT(clause.expression());
        }
        ACCEPT(element.expression()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::query::select_column const& element, std::size_t depth) {
        ACCEPT(element.value());
        ACCEPT(element.name());
        return true;
    }

    [[nodiscard]] bool operator()(ast::query::select_asterisk const& element, std::size_t depth) {
        ACCEPT(element.qualifier()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::query::grouping_column const& element, std::size_t depth) {
        ACCEPT(element.column());
        ACCEPT(element.collation());
        return true;
    }

    [[nodiscard]] bool operator()(ast::table::table_reference const& element, std::size_t depth) {
        ACCEPT(element.name());
        if (auto&& clause = element.correlation()) {
            ACCEPT(clause->correlation_name());
            ACCEPT(clause->column_names());
        }
        return true;
    }

    [[nodiscard]] bool operator()(ast::table::unnest const& element, std::size_t depth) {
        ACCEPT(element.expression());
        ACCEPT(element.correlation().correlation_name());
        ACCEPT(element.correlation().column_names());
        return true;
    }

    [[nodiscard]] bool operator()(ast::table::join const& element, std::size_t depth) {
        ACCEPT(element.left());
        ACCEPT(element.right());
        ACCEPT(element.specification());
        return true;
    }

    [[nodiscard]] bool operator()(ast::table::subquery const& element, std::size_t depth) {
        ACCEPT(element.expression());
        ACCEPT(element.correlation().correlation_name());
        ACCEPT(element.correlation().column_names());
        return true;
    }

    [[nodiscard]] bool operator()(ast::table::join_condition const& element, std::size_t depth) {
        ACCEPT(element.expression()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::table::join_columns const& element, std::size_t depth) {
        ACCEPT(element.columns()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::literal_expression const& element, std::size_t depth) {
        ACCEPT(element.value()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::variable_reference const& element, std::size_t depth) {
        ACCEPT(element.name()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::host_parameter_reference const& element, std::size_t depth) {
        ACCEPT(element.name()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::field_reference const& element, std::size_t depth) {
        ACCEPT(element.value());
        ACCEPT(element.name());
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::case_expression const& element, std::size_t depth) {
        ACCEPT(element.operand());
        for (auto&& clause: element.when_clauses()) { // NOLINT(*-use-anyofallof)
            ACCEPT(clause.when());
            ACCEPT(clause.result());
        }
        ACCEPT(element.default_result()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::cast_expression const& element, std::size_t depth) {
        ACCEPT(element.operand());
        ACCEPT(element.type());
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::unary_expression const& element, std::size_t depth) {
        ACCEPT(element.operand()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::binary_expression const& element, std::size_t depth) {
        ACCEPT(element.left());
        ACCEPT(element.right());
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::extract_expression const& element, std::size_t depth) {
        ACCEPT(element.operand()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::trim_expression const& element, std::size_t depth) {
        ACCEPT(element.character());
        ACCEPT(element.source());
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::value_constructor const& element, std::size_t depth) {
        ACCEPT(element.elements()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::subquery const& element, std::size_t depth) {
        ACCEPT(element.expression()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::comparison_predicate const& element, std::size_t depth) {
        ACCEPT(element.left());
        ACCEPT(element.right());
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::quantified_comparison_predicate const& element, std::size_t depth) {
        ACCEPT(element.left());
        ACCEPT(element.right());
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::between_predicate const& element, std::size_t depth) {
        ACCEPT(element.target());
        ACCEPT(element.left());
        ACCEPT(element.right());
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::in_predicate const& element, std::size_t depth) {
        ACCEPT(element.left());
        ACCEPT(element.right());
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::pattern_match_predicate const& element, std::size_t depth) {
        ACCEPT(element.match_value());
        ACCEPT(element.pattern());
        ACCEPT(element.escape());
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::table_predicate const& element, std::size_t depth) {
        ACCEPT(element.operand()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::function_invocation const& element, std::size_t depth) {
        ACCEPT(element.arguments()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::builtin_function_invocation const& element, std::size_t depth) {
        ACCEPT(element.arguments()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::builtin_set_function_invocation const& element, std::size_t depth) {
        ACCEPT(element.arguments()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::new_invocation const& element, std::size_t depth) {
        ACCEPT(element.type());
        ACCEPT(element.arguments()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::method_invocation const& element, std::size_t depth) {
        ACCEPT(element.value());
        ACCEPT(element.name());
        ACCEPT(element.arguments()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::static_method_invocation const& element, std::size_t depth) {
        ACCEPT(element.type());
        ACCEPT(element.name());
        ACCEPT(element.arguments()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::current_of_cursor const& element, std::size_t depth) {
        ACCEPT(element.name()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::scalar::placeholder_reference const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::literal::boolean const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::literal::numeric const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::literal::string const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::literal::datetime const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::literal::interval const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::literal::null const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::literal::empty const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::literal::default_ const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::type::simple const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::type::character_string const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::type::bit_string const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::type::octet_string const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::type::decimal const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::type::binary_numeric const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::type::datetime const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::type::interval const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::type::row const& element, std::size_t depth) {
        for (auto&& field : element.elements()) { // NOLINT(*-use-anyofallof)
            ACCEPT(field.name());
            ACCEPT(field.type());
            ACCEPT(field.collation());
        }
        return true;
    }

    [[nodiscard]] bool operator()(ast::type::user_defined const& element, std::size_t depth) {
        ACCEPT(element.name()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::type::collection const& element, std::size_t depth) {
        ACCEPT(element.element()); // NOLINT(*-simplify-boolean-expr)
        return true;
    }

    [[nodiscard]] bool operator()(ast::name::simple const& element, std::size_t depth) {
        (void) element;
        (void) depth;
        return true;
    }

    [[nodiscard]] bool operator()(ast::name::qualified const& element, std::size_t depth) {
        ACCEPT(element.qualifier());
        ACCEPT(element.last());
        return true;
    }

private:
    ::takatori::util::maybe_shared_ptr<::takatori::document::document const> document_;
    std::size_t node_limit_;
    std::size_t depth_limit_;

    std::size_t node_count_ {};
    std::size_t max_depth_ {};
    std::optional<sql_parser_diagnostic> result_ {};
};

} // namespace

sql_tree_validator::sql_tree_validator(std::size_t node_limit, std::size_t depth_limit) noexcept :
    node_limit_ { node_limit },
    depth_limit_ { depth_limit }
{}

bool sql_tree_validator::enabled() const noexcept {
    return node_limit_ > 0 || depth_limit_ > 0;
}

std::optional<sql_parser_diagnostic> sql_tree_validator::operator()(ast::compilation_unit const& element) {
    if (!enabled()) {
        last_node_count_ = 0;
        last_max_depth_ = 0;
        return {};
    }
    engine e { element.document(), node_limit_, depth_limit_ };
    auto result = e.process(element, 0);
    last_node_count_ = e.node_count();
    last_max_depth_ = e.max_depth();
    return result;
}

std::size_t sql_tree_validator::last_node_count() const noexcept {
    return last_node_count_;
}

std::size_t sql_tree_validator::last_max_depth() const noexcept {
    return last_max_depth_;
}

} // namespace mizugaki::parser
