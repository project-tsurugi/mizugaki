#include <gflags/gflags.h>

#include <iostream>
#include <string>
#include <string_view>

#include <mizugaki/parser/sql_parser.h>

namespace mizugaki::examples::parser_cli {

bool run(std::string_view source, std::size_t repeat, bool quiet, int debug) {
    parser::sql_parser parser;
    parser.set_debug(debug);
    for (std::size_t round = 0; round < repeat; ++round) {
        auto result = parser("-", std::string { source });
        if (auto&& error = result.diagnostic()) {
            std::cerr << error.message() << "; "
                      << "occurred at " << error.region().first()
                      << ", token: `" << error.document()->contents(error.region().first(), error.region().size()) << "`" << std::endl;
            return false;
        }
        if (!quiet && round == 0) {
            std::cout << *result.value() << std::endl;
        }
    }
    return true;
}

} // namespace mizugaki::examples::parser_cli

using namespace mizugaki::examples::parser_cli;

DEFINE_bool(quiet, false, "only show diagnostic erroneous"); // NOLINT
DEFINE_int32(debug, 0, "parser debug level"); // NOLINT
DEFINE_uint32(repeat, 1, "repeat parse operation"); // NOLINT

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("mizugaki SQL parser CLI");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    if (argc != 2) {
        gflags::ShowUsageWithFlags(argv[0]); // NOLINT
        return -1;
    }

    std::string_view source { argv[1] }; // NOLINT
    if (run(source, FLAGS_repeat, FLAGS_quiet, FLAGS_debug)) {
        return 0;
    }
    return 1;
}
