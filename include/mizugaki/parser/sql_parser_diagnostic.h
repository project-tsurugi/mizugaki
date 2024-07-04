#pragma once

#include <string>
#include <string_view>

#include <takatori/document/document.h>
#include <takatori/util/maybe_shared_ptr.h>

#include <mizugaki/ast/node_region.h>

#include "sql_parser_code.h"

namespace mizugaki::parser {

/**
 * @brief diagnostic information of parsing SQL.
 */
class sql_parser_diagnostic {
public:
    /// @brief the diagnostic code type.
    using code_type = sql_parser_code;

    /// @brief the diagnostic message type.
    using message_type = std::string;

    /// @brief the region type of element.
    using region_type = ast::node_region;

    /// @brief the source document type.
    using document_type = ::takatori::document::document;

    /**
     * @brief creates a new empty instance.
     */
    sql_parser_diagnostic() = default;

    /**
     * @brief creates a new instance
     * @param code the diagnostic code
     * @param message the diagnostic message
     * @param document the source document
     * @param region the diagnostic region on the document
     */
    explicit sql_parser_diagnostic(
            code_type code,
            message_type message,
            ::takatori::util::maybe_shared_ptr<document_type const> document = {},
            region_type region = {}) noexcept;

    /**
     * @brief returns whether or not this diagnostic information is valid.
     * @return true if this is valid
     * @return false otherwise
     */
    [[nodiscard]] bool is_valid() const noexcept;

    /// @copydoc is_valid()
    [[nodiscard]] explicit operator bool() const noexcept;

    /**
     * @brief returns the diagnostic code.
     * @return the diagnostic code
     * @return sql_parser_code::unknown if this is an empty diagnostic
     */
    [[nodiscard]] code_type& code() noexcept;

    /// @copydoc code()
    [[nodiscard]] code_type const& code() const noexcept;

    /**
     * @brief returns the diagnostic message.
     * @return the diagnostic message
     * @return empty if this is an empty diagnostic
     */
    [[nodiscard]] message_type& message() noexcept;

    /// @copydoc message()
    [[nodiscard]] message_type const& message() const noexcept;

    /**
     * @brief returns the source document.
     * @return the source document
     * @return empty if it is not sure
     */
    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const>& document() noexcept;

    /// @copydoc document()
    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const> const& document() const noexcept;

    /**
     * @brief returns the region of this diagnostic.
     * @return the diagnostic region
     */
    [[nodiscard]] region_type& region() noexcept;

    /// @copydoc region()
    [[nodiscard]] region_type const& region() const noexcept;

    /**
     * @brief returns the contents of the diagnostic region.
     * @return the contents
     * @return empty string if the region is empty (may represent EOF)
     * @attention the returned contents will be disabled after document() was disposed
     */
    [[nodiscard]]  std::string_view contents() const;

private:
    code_type code_ { code_type::unknown };
    message_type message_ {};
    ::takatori::util::maybe_shared_ptr<document_type const> document_ {};
    region_type region_ {};
};

} // namespace mizugaki::parser
