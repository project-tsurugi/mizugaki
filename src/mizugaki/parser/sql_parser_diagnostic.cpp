#include <mizugaki/parser/sql_parser_diagnostic.h>

namespace mizugaki::parser {

using ::takatori::util::maybe_shared_ptr;

sql_parser_diagnostic::sql_parser_diagnostic(
        code_type code,
        message_type message,
        maybe_shared_ptr<document_type const> document,
        region_type region) noexcept:
    code_ { code },
    message_(std::move(message)),
    document_ { std::move(document) },
    region_ { region }
{}

bool sql_parser_diagnostic::is_valid() const noexcept {
    return code_ != code_type::unknown || !message_.empty();
}

sql_parser_diagnostic::operator bool() const noexcept {
    return is_valid();
}

sql_parser_diagnostic::code_type& sql_parser_diagnostic::code() noexcept {
    return code_;
}

sql_parser_diagnostic::code_type const& sql_parser_diagnostic::code() const noexcept {
    return code_;
}

sql_parser_diagnostic::message_type& sql_parser_diagnostic::message() noexcept {
    return message_;
}

sql_parser_diagnostic::message_type const& sql_parser_diagnostic::message() const noexcept {
    return message_;
}

maybe_shared_ptr<sql_parser_diagnostic::document_type const>& sql_parser_diagnostic::document() noexcept {
    return document_;
}

maybe_shared_ptr<sql_parser_diagnostic::document_type const> const& sql_parser_diagnostic::document() const noexcept {
    return document_;
}

sql_parser_diagnostic::region_type& sql_parser_diagnostic::region() noexcept {
    return region_;
}

sql_parser_diagnostic::region_type const& sql_parser_diagnostic::region() const noexcept {
    return region_;
}

std::string_view sql_parser_diagnostic::contents() const {
    if (region_.size() == 0) {
        return {};
    }
    return document_->contents(region_.first(), region_.size());
}

} // namespace mizugaki::parser
