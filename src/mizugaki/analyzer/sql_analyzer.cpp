#include <mizugaki/analyzer/sql_analyzer.h>

#include <mizugaki/analyzer/sql_analyzer_impl.h>

namespace mizugaki::analyzer {

using ::takatori::util::optional_ptr;
using ::takatori::util::sequence_view;

sql_analyzer::sql_analyzer()
    : impl_(std::make_unique<impl>())
{}

sql_analyzer::~sql_analyzer() = default;

sql_analyzer::sql_analyzer(sql_analyzer&& other) noexcept = default;

sql_analyzer& sql_analyzer::operator=(sql_analyzer&& other) noexcept = default;

sql_analyzer::result_type sql_analyzer::operator()(
        options_type const& options,
        ast::statement::statement const& statement,
        ast::compilation_unit const& source,
        placeholder_map const& placeholders,
        ::takatori::util::optional_ptr<::yugawara::variable::provider const> host_parameters) {
    return operator()(
            options,
            statement,
            optional_ptr { source.document().get() },
            sequence_view { source.comments() },
            placeholders,
            host_parameters);
}

sql_analyzer::result_type sql_analyzer::operator()(
        options_type const& options,
        ast::statement::statement const& statement,
        optional_ptr<::takatori::document::document const> source,
        sequence_view<ast::node_region const> comments,
        placeholder_map const& placeholders,
        ::takatori::util::optional_ptr<::yugawara::variable::provider const> host_parameters) {
    return impl_->process(
            options,
            statement,
            std::move(source),
            comments,
            placeholders,
            host_parameters);
}

} // namespace mizugaki::analyzer
