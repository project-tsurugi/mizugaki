#pragma once

#include <takatori/scalar/expression.h>

#include <yugawara/storage/relation.h>

#include <mizugaki/ast/name/name.h>
#include <mizugaki/analyzer/details/analyzer_context.h>
#include <mizugaki/analyzer/details/query_scope.h>

namespace mizugaki::analyzer::details {

[[nodiscard]] std::unique_ptr<::takatori::scalar::expression> analyze_variable_name(
        analyzer_context& context,
        ast::name::name const& name,
        query_scope const& scope); // FIXME: search for parent scope

[[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::relation const> analyze_relation_name(
        analyzer_context& context,
        ast::name::name const& name);

} // namespace mizugaki::analyzer::details
