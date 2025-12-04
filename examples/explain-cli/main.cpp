#include <gflags/gflags.h>

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#include <takatori/type/int.h>
#include <takatori/type/decimal.h>
#include <takatori/type/character.h>

#include <takatori/serializer/json_printer.h>
#include <takatori/serializer/object_acceptor.h>

#include <takatori/util/optional_ptr.h>

#include <yugawara/compiler.h>
#include <yugawara/compiler_options.h>
#include <yugawara/compiler_result.h>
#include <yugawara/compiled_info.h>

#include <yugawara/variable/configurable_provider.h>
#include <yugawara/variable/declaration.h>

#include <mizugaki/parser/sql_parser.h>
#include <mizugaki/parser/sql_parser_options.h>
#include <mizugaki/parser/sql_parser_result.h>

#include <mizugaki/ast/statement/statement.h>

#include <mizugaki/analyzer/sql_analyzer.h>

#include "ddl_interpreter.h"
#include "options.h"

namespace mizugaki::examples::explain_cli {

static parser::sql_parser_result parse(std::string path, std::string source, parser::sql_parser_options options) {
    parser::sql_parser engine { std::move(options) };
    auto result = engine(std::move(path), std::move(source));
    return result;
}

static std::shared_ptr<::yugawara::variable::configurable_provider> parse_variables(std::string const& sequence) {
    auto result = std::make_shared<::yugawara::variable::configurable_provider>();

    // parse name:type pairs separated by ','
    std::size_t start = 0;
    while (start < sequence.size()) {
        auto end = sequence.find(',', start);
        if (end == std::string::npos) {
            end = sequence.size();
        }
        // separate name and type by ':'
        auto pair = sequence.substr(start, end - start);
        auto equal_pos = pair.find(':');
        if (equal_pos != std::string::npos) {
            auto key = pair.substr(0, equal_pos);
            auto value = pair.substr(equal_pos + 1);
            for (auto& c : value) {
                if ('A' <= c && c <= 'Z') {
                    c = static_cast<char>(c - 'A' + 'a');
                }
            }
            if (value == "int4" || value == "int" || value == "integer") {
                result->add({ key, ::takatori::type::int4 {} });
            } else if (value == "int8" || value == "bigint" || value == "bigingeger") {
                result->add({ key, ::takatori::type::int8 {} });
            } else if (value == "decimal") {
                result->add({ key, ::takatori::type::decimal {} });
            } else if (value == "varchar") {
                result->add({ key, ::takatori::type::character { ::takatori::type::varying, {} } });
            } else {
                throw std::runtime_error("unsupported placeholder type: " + value);
            }
        } else {
            throw std::runtime_error("invalid placeholder definition: " + pair);
        }
        start = end + 1;
    }
    return result;
}

static analyzer::sql_analyzer_result analyze(
        ast::statement::statement const& statement,
        ast::compilation_unit const& source,
        analyzer::sql_analyzer_options const& options,
        ::yugawara::variable::provider const& variables) {
    analyzer::sql_analyzer engine {};
    auto result = engine(options, statement, source, {}, variables);
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
DEFINE_string(placeholders, "", "placeholder definitions (name:type, separated by ,)"); // NOLINT

int main(int argc, char* argv[]) noexcept(false) {
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

    auto variables = parse_variables(FLAGS_placeholders);

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

    auto schema = create_default_schema("public");

    auto compiler_opts = compiler_options();
    for (auto&& statement : compilation_unit->statements()) {
        if (FLAGS_echo) {
            auto&& region = statement->region();
            std::cout << "> "
                    << compilation_unit->document()->contents(region.begin, region.size())
                    << '\n';
        }

        auto analyzer_opts = analyzer_options(schema);
        auto analyzer_result = analyze(*statement, *compilation_unit, analyzer_opts, *variables);

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

        interpret(compiler_result.statement());
    }
    return 0;
}
