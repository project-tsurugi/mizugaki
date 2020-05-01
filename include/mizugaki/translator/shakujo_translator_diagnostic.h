#pragma once

#include <iostream>
#include <string>

#include <takatori/document/region.h>

#include "shakujo_translator_diagnostic_code.h"

namespace mizugaki::translator {

/**
 * @brief represents a diagnostic information of translate operation.
 */
class shakujo_translator_diagnostic {
public:
    /// @brief the diagnostic code type.
    using code_type = shakujo_translator_diagnostic_code;
    /// @brief the diagnostic message type.
    using message_type = std::string;
    /// @brief the document location type.
    using location_type = ::takatori::document::region;

    /**
     * @brief creates a new empty instance.
     */
    shakujo_translator_diagnostic() noexcept = default;

    /**
     * @brief creates a new instance.
     * @param code the diagnostic code
     * @param message the optional diagnostic message
     * @param location the diagnostic location
     */
    shakujo_translator_diagnostic(code_type code, message_type message, location_type location) noexcept;

    /**
     * @brief creates a new instance.
     * @param code the diagnostic code
     * @param location the diagnostic location
     */
    shakujo_translator_diagnostic(code_type code, location_type location) noexcept;

    /**
     * @brief returns the diagnostic code.
     * @return the diagnostic code
     */
    [[nodiscard]] code_type code() const noexcept;

    /**
     * @brief returns the diagnostic message.
     * @return the diagnostic message
     */
    [[nodiscard]] message_type const& message() const noexcept;

    /**
     * @brief returns the diagnostic location.
     * @return the diagnostic location
     */
    [[nodiscard]] location_type const& location() const noexcept;

    /**
     * @brief appends string representation of the given value.
     * @param out the target output
     * @param value the target value
     * @return the output stream
     */
    friend std::ostream& operator<<(std::ostream& out, shakujo_translator_diagnostic const& value);

private:
    code_type code_ { code_type::unknown };
    message_type message_ {};
    location_type location_ {};
};

} // namespace mizugaki::translator
