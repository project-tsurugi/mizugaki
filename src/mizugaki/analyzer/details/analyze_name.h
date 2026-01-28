#pragma once

#include <functional>
#include <utility>
#include <optional>
#include <variant>

#include <takatori/scalar/expression.h>

#include <takatori/util/maybe_shared_ptr.h>

#include <yugawara/aggregate/declaration.h>
#include <yugawara/function/declaration.h>
#include <yugawara/schema/declaration.h>
#include <yugawara/storage/index.h>
#include <yugawara/storage/relation.h>
#include <yugawara/storage/table.h>

#include <mizugaki/ast/name/name.h>

#include "analyzer_context.h"
#include "query_info.h"
#include "query_scope.h"

namespace mizugaki::analyzer::details {

enum class name_kind {
    not_specified,
    variable,
    function,
    relation,
    schema,
    catalog,
};

template<class T>
using schema_element = std::pair<
        ::takatori::util::maybe_shared_ptr<::yugawara::schema::declaration const>,
        std::shared_ptr<T const>>;

[[nodiscard]] std::unique_ptr<::takatori::scalar::expression> analyze_variable_name(
        analyzer_context& context,
        ast::name::name const& name,
        query_scope const& scope); // FIXME: search for parent scope

[[nodiscard]] ::takatori::util::optional_ptr<relation_info const> analyze_relation_info_name(
        analyzer_context& context,
        ast::name::name const& name,
        query_scope const& scope);

/// @brief the result of analyze_relation_name()
using declared_relation = std::variant<
        std::monostate, // error
        std::reference_wrapper<::yugawara::storage::table const>, // found table
        std::shared_ptr<query_info const>>; // found named subquery (CTE)

/**
 * @brief analyzes a declared relation name and returns its declaration.
 * @details This function looks up the relation declaration information.
 *    This can return the following types of relations:
 *    - tables - returns as yugawara::storage::table
 *    - CTEs - returns as query_info
 * @param context The analyzer context.
 * @param name The AST representation of the relation name.
 * @param scope the query scope to search within (for CTEs)
 * @return An optional pointer to the relation declaration if found; otherwise, an empty optional.
 */
[[nodiscard]] declared_relation analyze_relation_name(
        analyzer_context& context,
        ast::name::name const& name,
        query_scope const& scope = {});

[[nodiscard]] std::vector<std::shared_ptr<::yugawara::function::declaration const>> analyze_function_name(
        analyzer_context& context,
        ast::name::name const& name,
        std::size_t argument_count);

[[nodiscard]] std::vector<std::shared_ptr<::yugawara::aggregate::declaration const>> analyze_aggregation_name(
        analyzer_context& context,
        ast::name::name const& name,
        std::size_t argument_count);

[[nodiscard]] std::optional<schema_element<::yugawara::storage::table>> analyze_table_name(
        analyzer_context& context,
        ast::name::name const& name,
        bool mandatory = true);

[[nodiscard]] std::optional<schema_element<::yugawara::storage::index>> analyze_index_name(
        analyzer_context& context,
        ast::name::name const& name,
        bool mandatory = true);

[[nodiscard]] std::shared_ptr<::yugawara::schema::declaration const> analyze_schema_name(
        analyzer_context& context,
        ast::name::name const& name,
        bool mandatory = true);

[[nodiscard]] std::string normalize_identifier(
        analyzer_context& context,
        ast::name::simple const& name,
        name_kind kind);

} // namespace mizugaki::analyzer::details
