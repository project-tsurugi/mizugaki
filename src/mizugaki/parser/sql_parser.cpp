#include <mizugaki/parser/sql_parser.h>

#include <sstream>

#include <takatori/document/basic_document.h>

#include <mizugaki/parser/sql_parser_generated.hpp>
#include <mizugaki/parser/sql_driver.h>
#include <mizugaki/parser/sql_scanner.h>

namespace mizugaki::parser {

using ::takatori::document::basic_document;

sql_parser& sql_parser::set_debug(int level) noexcept {
    debug_ = level;
    return *this;
}

sql_parser::result_type sql_parser::operator()(std::string location, std::string contents) const {
    std::istringstream input { contents };
    sql_scanner scanner { input };

    auto document = std::make_shared<basic_document>(std::move(location), std::move(contents));
    sql_driver driver { std::move(document) };

    sql_parser_generated parser { scanner, driver };

#if YYDEBUG
    parser.set_debug_level(static_cast<sql_parser_generated::debug_level_type>(debug_));
    parser.set_debug_stream(std::cout);
#endif // YYDEBUG

    parser.parse();
    return std::move(driver.result());
}

} // namespace mizugaki::parser

