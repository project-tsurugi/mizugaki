#include <mizugaki/analyzer/details/analyze_statement.h>

namespace mizugaki::analyzer::details {

analyze_statement_result_type analyze_statement(
        analyzer_context& context,
        ast::statement::statement const& statement) {
    // FIXME: mock
    (void) context;
    (void) statement;
    return {};
}

} // namespace mizugaki::analyzer::details
