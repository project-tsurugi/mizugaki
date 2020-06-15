#include <mizugaki/parser/sql_driver.h>

namespace mizugaki::parser {

using ::takatori::util::maybe_shared_ptr;
using ::takatori::util::object_creator;

using document_type = sql_driver::document_type;

maybe_shared_ptr<document_type const> const& sql_driver::document() const noexcept {
    return document_;
}

object_creator sql_driver::get_object_creator() const noexcept {
    return creator_;
}

sql_driver::result_type& sql_driver::result() noexcept {
    return result_;
}

sql_driver::result_type const& sql_driver::result() const noexcept {
    return result_;
}

void sql_driver::success(ast::common::vector<node_ptr<ast::statement::statement>> statements) {
    result_.value() = node<ast::compilation_unit>(
            std::move(statements),
            document_);
}

void sql_driver::error(sql_driver::location_type location, sql_parser_result::message_type message) {
    result_ = {
            location,
            std::move(message),
    };
}

void sql_driver::add_comment(sql_driver::location_type location) {
    // FIXME: impl
    (void) location;
}

} // namespace mizugaki::parser

