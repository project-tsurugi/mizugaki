#pragma once

#include <string>
#include <string_view>

#include <takatori/document/document.h>
#include <takatori/util/maybe_shared_ptr.h>

#include <mizugaki/ast/node_region.h>

namespace mizugaki::parser {

/**
 * @brief diagnostic information of parsing SQL.
 */
class sql_parser_diagnostic {
public:
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
     * @param message the diagnostic message
     * @param document the source document
     * @param region the diagnostic region on the document
     */
    explicit sql_parser_diagnostic(
            message_type message,
            ::takatori::util::maybe_shared_ptr<document_type const> document = {},
            region_type region = {}) noexcept :
        message_(std::move(message)),
        document_ { std::move(document) },
        region_ { region }
    {}

    /**
     * @brief returns whether or not this diagnostic information is valid.
     * @return true if this is valid
     * @return false otherwise
     */
    [[nodiscard]] bool is_valid() const noexcept {
        return !message_.empty();
    }

    /// @copydoc is_valid()
    [[nodiscard]] explicit operator bool() const noexcept {
        return is_valid();
    }

    /**
     * @brief returns the diagnostic message.
     * @return the diagnostic message
     * @return empty if this is an empty diagnostic
     */
    [[nodiscard]] message_type& message() noexcept {
        return message_;
    }

    /// @copydoc message()
    [[nodiscard]] message_type const& message() const noexcept {
        return message_;
    }

    /**
     * @brief returns the source document.
     * @return the source document
     * @return empty if it is not sure
     */
    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const>& document() noexcept {
        return document_;
    }

    /// @copydoc document()
    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const> const& document() const noexcept {
        return document_;
    }

    /**
     * @brief returns the region of this diagnostic.
     * @return the diagnostic region
     */
    [[nodiscard]] constexpr region_type& region() noexcept {
        return region_;
    }

    /// @copydoc region()
    [[nodiscard]] constexpr region_type const& region() const noexcept {
        return region_;
    }

private:
    message_type message_ {};
    ::takatori::util::maybe_shared_ptr<document_type const> document_ {};
    region_type region_ {};
};

} // namespace mizugaki::parser
