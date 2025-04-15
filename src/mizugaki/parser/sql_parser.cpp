#include <mizugaki/parser/sql_parser.h>

#include <sstream>

#include <takatori/document/basic_document.h>

#include <mizugaki/parser/sql_parser_generated.hpp>
#include <mizugaki/parser/sql_driver.h>
#include <mizugaki/parser/sql_scanner.h>

#include "sql_tree_validator.h"

namespace mizugaki::parser {

using ::takatori::document::basic_document;

sql_parser::sql_parser(sql_parser_options options) noexcept :
    options_ { std::move(options) }
{}

sql_parser_options &sql_parser::options() noexcept {
    return options_;
}

sql_parser_options const &sql_parser::options() const noexcept {
    return options_;
}

sql_parser::result_type sql_parser::operator()(std::string location, std::string contents) const {
    auto document = std::make_shared<basic_document>(std::move(location), std::move(contents));
    return operator()(std::move(document));
}

sql_parser::result_type sql_parser::operator()(takatori::util::maybe_shared_ptr<document_type const> document) const {
    std::istringstream input {
            std::string {
                    document->contents(0, document->size()),
            },
    };
    sql_scanner scanner { input };

    sql_driver driver { std::move(document) };
    driver.max_expected_candidates() = options_.max_expected_candidates();
    driver.element_limits() = options_.element_limits();
    driver.enable_description_comments() = options_.enable_description_comments();

    sql_parser_generated parser { scanner, driver };

#if YYDEBUG
    parser.set_debug_level(static_cast<sql_parser_generated::debug_level_type>(options_.debug()));
    parser.set_debug_stream(std::cout);
#endif // YYDEBUG

    parser.parse();
    if (driver.result().has_value()) {
        sql_tree_validator checker {
                options_.tree_node_limit(),
                options_.tree_depth_limit(),
        };
        if (auto diagnostic = checker(*driver.result().value())) {
            return std::move(*diagnostic);
        }
        driver.result().max_tree_depth() = checker.last_max_depth();
        driver.result().tree_node_count() = checker.last_node_count();
    }

    return std::move(driver.result());
}

} // namespace mizugaki::parser
