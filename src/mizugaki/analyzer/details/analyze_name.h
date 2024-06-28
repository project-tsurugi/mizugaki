#pragma once

#include <utility>
#include <optional>

#include <takatori/scalar/expression.h>

#include <takatori/util/maybe_shared_ptr.h>

#include <yugawara/storage/relation.h>
#include <yugawara/schema/declaration.h>
#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>

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

[[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::relation const> analyze_relation_name(
        analyzer_context& context,
        ast::name::name const& name);

[[nodiscard]] std::optional<schema_element<::yugawara::storage::table>> analyze_table_name(
        analyzer_context& context,
        ast::name::name const& name,
        bool mandatory = true);

[[nodiscard]] std::optional<schema_element<::yugawara::storage::index>> analyze_index_name(
        analyzer_context& context,
        ast::name::name const& name,
        bool mandatory = true);

[[nodiscard]] std::string normalize_identifier(
        analyzer_context& context,
        ast::name::simple const& name);

} // namespace mizugaki::analyzer::details
