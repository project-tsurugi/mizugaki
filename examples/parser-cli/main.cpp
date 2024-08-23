#include <gflags/gflags.h>

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#include <mizugaki/parser/sql_parser.h>

namespace mizugaki::examples::parser_cli {

bool run(std::string_view source, std::size_t repeat, bool quiet, bool stats, parser::sql_parser engine) {
    for (std::size_t round = 0; round < repeat; ++round) {
        auto result = engine("-", std::string { source });
        if (auto&& error = result.diagnostic()) {
            std::cerr << error.message() << "; "
                      << "occurred at " << error.region().first()
                      << ", token: `" << error.document()->contents(error.region().first(), error.region().size()) << "`" << std::endl;
            return false;
        }
        if (!quiet && round == 0) {
            std::cout << *result.value() << std::endl;
            if (stats) {
                std::cout << "AST nodes: " << result.tree_node_count() << std::endl;
                std::cout << "AST depth: " << result.max_tree_depth() << std::endl;
            }
        }
    }
    return true;
}

} // namespace mizugaki::examples::parser_cli

using namespace mizugaki::examples::parser_cli;

DEFINE_bool(quiet, false, "only show diagnostic erroneous"); // NOLINT
DEFINE_int32(debug, 0, "parser debug level"); // NOLINT
DEFINE_uint32(repeat, 1, "repeat parse operation"); // NOLINT
DEFINE_string(file, "", "input file path"); // NOLINT
DEFINE_string(text, "", "input text"); // NOLINT
DEFINE_bool(stats, false, "show AST statistics"); // NOLINT
DEFINE_uint64(node_limit, 10'000, "AST node limit"); // NOLINT
DEFINE_uint64(depth_limit, 5'000, "AST depth limit"); // NOLINT

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("mizugaki SQL parser CLI");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if ((FLAGS_text.empty() && FLAGS_file.empty()) || (!FLAGS_text.empty() && !FLAGS_file.empty())) {
        gflags::ShowUsageWithFlags(argv[0]); // NOLINT
        return 1;
    }

    std::string source;
    if (!FLAGS_text.empty()) {
        source = FLAGS_text;
    } else {
        std::ifstream ifs { FLAGS_file };
        if (ifs.fail()) {
            std::cerr << "failed to open file: " << FLAGS_file << std::endl;
            return 2;
        }
        source.assign(
                std::istreambuf_iterator<char> { ifs },
                std::istreambuf_iterator<char> {});
        ifs.close();
    }
    ::mizugaki::parser::sql_parser engine {};
    engine.options().debug() = FLAGS_debug;
    engine.options().tree_node_limit() = FLAGS_node_limit;
    engine.options().tree_depth_limit() = FLAGS_depth_limit;
    if (run(source, FLAGS_repeat, FLAGS_quiet, FLAGS_stats, std::move(engine))) {
        return 0;
    }
    return 1;
}
