#include <mizugaki/analyzer/sql_analyzer_impl.h>

#include <mizugaki/analyzer/details/analyze_statement.h>

namespace mizugaki::analyzer {

using impl = sql_analyzer::impl;

using ::takatori::util::optional_ptr;
using ::takatori::util::sequence_view;

impl::result_type impl::process(
        options_type const& options,
        ast::statement::statement const& statement,
        optional_ptr<::takatori::document::document const> source,
        sequence_view<ast::node_region const> comments,
        placeholder_map const& placeholders,
        ::takatori::util::optional_ptr<::yugawara::variable::provider const> host_parameters) {
    using namespace details;
    auto finalizer = context_.initialize(options, source, comments, placeholders, host_parameters);
    auto result = analyze_statement(context_, statement);
    if (std::holds_alternative<erroneous_result_type>(result)) {
        return std::move(context_.diagnostics());
    }
    if (std::holds_alternative<execution_plan_result_type>(result)) {
        return std::move(std::get<execution_plan_result_type>(result));
    }
    return std::move(std::get<statement_result_type>(result));
}

} // namespace mizugaki::analyzer
