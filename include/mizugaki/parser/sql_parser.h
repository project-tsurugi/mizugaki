#pragma once

#include <string>


#include "sql_parser_result.h"

namespace mizugaki::parser {

/**
 * @brief parses SQL text.
 */
class sql_parser {
public:
    /// @brief the result type.
    using result_type = sql_parser_result;

    /**
     * @brief sets the debug level.
     * @param level the debug level
     * @return this
     * @note this feature is only available for debug configurations
     */
    sql_parser& set_debug(int level) noexcept;

    /**
     * @brief parses the contents.
     * @param location the content location
     * @param contents the target contents
     * @return the parsed result
     */
    [[nodiscard]] result_type operator()(std::string location, std::string contents) const;

private:
    int debug_ {};
};

} // namespace mizugaki::parser
