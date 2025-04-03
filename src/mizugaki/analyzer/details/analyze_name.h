#pragma once

#include <utility>
#include <optional>

#include <takatori/scalar/expression.h>

#include <takatori/util/maybe_shared_ptr.h>

#include <yugawara/aggregate/declaration.h>
#include <yugawara/function/declaration.h>
#include <yugawara/schema/declaration.h>
#include <yugawara/storage/index.h>
#include <yugawara/storage/relation.h>
#include <yugawara/storage/table.h>

#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/analyzer/details/analyzer_context.h>
#include <mizugaki/analyzer/details/query_scope.h>

namespace mizugaki::analyzer::details {

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

[[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::relation const> analyze_relation_name(
        analyzer_context& context,
        ast::name::name const& name);

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
        ast::name::simple const& name);

} // namespace mizugaki::analyzer::details
