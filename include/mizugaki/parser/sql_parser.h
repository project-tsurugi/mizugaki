#pragma once

#include <string>

#include <takatori/document/document.h>

#include <takatori/util/maybe_shared_ptr.h>

#include "sql_parser_result.h"

namespace mizugaki::parser {

/**
 * @brief parses SQL text.
 */
class sql_parser {
public:
    /// @brief the source document type.
    using document_type = ::takatori::document::document;

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
     * @details This operation creates a new document object, and then the resulting compilation unit will hold
     *      the ownership of the created document object.
     * @param location the content location
     * @param contents the target contents
     * @return the parsed result
     */
    [[nodiscard]] result_type operator()(std::string location, std::string contents) const;

    /**
     * @brief parses the contents in the given document.
     * @param document the source document
     * @return the parsed result
     */
    [[nodiscard]] result_type operator()(::takatori::util::maybe_shared_ptr<document_type const> document) const;

private:
    int debug_ {};
};

} // namespace mizugaki::parser
