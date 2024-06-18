#include <mizugaki/analyzer/details/analyze_statement.h>

#include <takatori/util/string_builder.h>

namespace mizugaki::analyzer::details {

using ::takatori::util::string_builder;

using diagnostic_type = analyzer_context::diagnostic_type;
using diagnostic_code = diagnostic_type::code_type;

analyze_statement_result_type analyze_statement(
        analyzer_context& context,
        ast::statement::statement const& statement) {
    // FIXME: mock

    context.report(
            diagnostic_code::unsupported_feature,
            string_builder {}
                << "unknown statement: "
                << statement.node_kind()
                << string_builder::to_string,
            statement.region());
    return {};
}

} // namespace mizugaki::analyzer::details
