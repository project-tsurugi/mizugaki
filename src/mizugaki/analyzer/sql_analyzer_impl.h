#pragma once

#include <takatori/type/data.h>
#include <takatori/value/data.h>

#include <yugawara/util/object_repository.h>

#include <mizugaki/placeholder_map.h>
#include <mizugaki/analyzer/sql_analyzer.h>
#include <mizugaki/analyzer/details/analyzer_context.h>

namespace mizugaki::analyzer {

class sql_analyzer::impl {
public:
    using options_type = sql_analyzer::options_type;
    using result_type = sql_analyzer::result_type;
    using context_type = details::analyzer_context;

    [[nodiscard]] result_type process(
            options_type const& options,
            ast::statement::statement const& statement,
            ::takatori::util::optional_ptr<::takatori::document::document const> source,
            ::takatori::util::sequence_view<ast::node_region const> comments,
            placeholder_map const& placeholders,
            ::takatori::util::optional_ptr<::yugawara::variable::provider const> host_parameters);

private:
    context_type context_;
};

} // namespace mizugaki::analyzer
