#include <mizugaki/translator/shakujo_translator_diagnostic.h>

#include <utility>

namespace mizugaki::translator {

shakujo_translator_diagnostic::shakujo_translator_diagnostic(
        code_type code,
        message_type message,
        location_type location) noexcept
    : code_(code)
    , message_(std::move(message))
    , location_(location)
{}

shakujo_translator_diagnostic::shakujo_translator_diagnostic(code_type code, location_type location) noexcept
    : code_(code)
    , location_(location)
{}

shakujo_translator_diagnostic::code_type shakujo_translator_diagnostic::code() const noexcept {
    return code_;
}

shakujo_translator_diagnostic::message_type const& shakujo_translator_diagnostic::message() const noexcept {
    return message_;
}

shakujo_translator_diagnostic::location_type const& shakujo_translator_diagnostic::location() const noexcept {
    return location_;
}

std::ostream& operator<<(std::ostream& out, shakujo_translator_diagnostic const& value) {
    return out << "diagnostic("
               << "code=" << value.code() << ", "
               << "message=" << value.message() << ", "
               << "location=" << value.location() << ")";
}

} // namespace mizugaki::translator
