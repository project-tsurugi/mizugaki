#include <gflags/gflags.h>

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#include <takatori/serializer/json_printer.h>
#include <takatori/serializer/object_acceptor.h>

#include <yugawara/compiler.h>
#include <yugawara/compiler_options.h>
#include <yugawara/compiler_result.h>
#include <yugawara/compiled_info.h>

#include <mizugaki/parser/sql_parser.h>
#include <mizugaki/parser/sql_parser_options.h>
#include <mizugaki/parser/sql_parser_result.h>

#include <mizugaki/ast/statement/statement.h>

#include <mizugaki/analyzer/sql_analyzer.h>

#include "options.h"

namespace mizugaki::examples::explain_cli {

static parser::sql_parser_result parse(std::string path, std::string source, parser::sql_parser_options options) {
    parser::sql_parser engine { std::move(options) };
    auto result = engine(std::move(path), std::move(source));
    return result;
}

static analyzer::sql_analyzer_result analyze(
        ast::statement::statement const& statement,
        ast::compilation_unit const& source,
        analyzer::sql_analyzer_options const& options) {
    analyzer::sql_analyzer engine {};
    auto result = engine(options, statement, source);
    return result;
}

static ::yugawara::compiler_result compile(
        std::unique_ptr<::takatori::statement::statement> statement,
        ::yugawara::compiler_options const& options) {
    ::yugawara::compiler engine {};
    auto result = engine(options, std::move(statement));
    return result;
}

static ::yugawara::compiler_result compile(
        std::unique_ptr<::takatori::relation::graph_type> graph,
        ::yugawara::compiler_options const& options) {
    ::yugawara::compiler engine {};
    auto result = engine(options, std::move(*graph));
    graph->clear();
    return result;
}

} // namespace mizugaki::examples::explain_cli

using namespace mizugaki::examples::explain_cli;

DEFINE_bool(echo, false, "prints the source SQL"); // NOLINT
DEFINE_bool(quiet, false, "only show diagnostic erroneous"); // NOLINT
DEFINE_string(file, "", "input file path"); // NOLINT
DEFINE_string(text, "", "input text"); // NOLINT

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("mizugaki SQL parser CLI");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if ((FLAGS_text.empty() && FLAGS_file.empty()) || (!FLAGS_text.empty() && !FLAGS_file.empty())) {
        gflags::ShowUsageWithFlags(argv[0]); // NOLINT
        return 1;
    }

    std::string path;
    std::string source;
    if (!FLAGS_text.empty()) {
        path = "<text>";
        source = FLAGS_text;
    } else {
        path = FLAGS_file;
        std::ifstream ifs { FLAGS_file };
        if (ifs.fail()) {
            std::cerr << "failed to open file: " << FLAGS_file << '\n';
            return 2;
        }
        source.assign(
                std::istreambuf_iterator { ifs },
                std::istreambuf_iterator<char> {});
        ifs.close();
    }

    // parse
    auto parser_opts = parser_options();
    auto parser_result = parse(std::move(path), std::move(source), std::move(parser_opts));
    if (!parser_result) {
        auto&& info = parser_result.diagnostic();
        std::cerr << "failed to parse SQL: " << info.code() << '\n'
                << "  message: " << info.message() << '\n'
                << "  at " << info.region() << '\n';
        return 1;
    }
    auto compilation_unit = std::move(parser_result.value());

    auto compiler_opts = compiler_options();
    for (auto&& statement : compilation_unit->statements()) {
        if (FLAGS_echo) {
            auto&& region = statement->region();
            std::cout << "> "
                    << compilation_unit->document()->contents(region.begin, region.end)
                    << '\n';
        }

        auto analyzer_opts = analyzer_options();
        auto analyzer_result = analyze(*statement, *compilation_unit, analyzer_opts);

        ::yugawara::compiler_result compiler_result {};
        switch (analyzer_result.kind()) {
            case ::mizugaki::analyzer::sql_analyzer_result_kind::statement: {
                auto stmt = analyzer_result.release<::mizugaki::analyzer::sql_analyzer_result_kind::statement>();
                compiler_result = compile(std::move(stmt), compiler_opts);
                break;
            }
            case ::mizugaki::analyzer::sql_analyzer_result_kind::execution_plan: {
                auto graph = analyzer_result.release<::mizugaki::analyzer::sql_analyzer_result_kind::execution_plan>();
                compiler_result = compile(std::move(graph), compiler_opts);
                break;
            }
            case ::mizugaki::analyzer::sql_analyzer_result_kind::diagnostics: {
                auto errors = analyzer_result.release<::mizugaki::analyzer::sql_analyzer_result_kind::diagnostics>();
                for (auto&& info : errors) {
                    std::cerr << "failed to analyze SQL: " << info.code() << '\n'
                            << "  message: " << info.message() << '\n'
                            << "  at " << info.location() << '\n';
                }
                return 2;
            }
            default:
                std::abort();
        }

        if (!compiler_result) {
            auto errors = compiler_result.diagnostics();
            for (auto&& info : errors) {
                std::cerr << "failed to compile SQL: " << info.code() << '\n'
                        << "  message: " << info.message() << '\n'
                        << "  at " << info.location() << '\n';
            }
            return 3;
        }

        if (!FLAGS_quiet) {
            ::takatori::serializer::json_printer printer { std::cout };
            auto&& scanner = compiler_result.info().object_scanner();
            auto&& compiled = compiler_result.statement();
            scanner(compiled, printer);
            std::cout << '\n' << std::flush;
        }
    }
    return 0;
}
