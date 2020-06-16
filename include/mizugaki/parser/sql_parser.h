#pragma once

#include <string>

#include <takatori/util/object_creator.h>

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
     * @brief creates a new instance.
     * @param creator the object creator for building syntactic models
     */
    explicit sql_parser(::takatori::util::object_creator creator = {}) noexcept;

    /**
     * @brief parses the contents.
     * @param location the content location
     * @param contents the target contents
     * @return the parsed result
     */
    [[nodiscard]] result_type operator()(std::string location, std::string contents);

private:
    ::takatori::util::object_creator creator_;
};

} // namespace mizugaki::parser
