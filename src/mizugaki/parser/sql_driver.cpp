#include <mizugaki/parser/sql_driver.h>

#include <cctype>
#include <cerrno>
#include <cstdlib>

#include <takatori/util/assertion.h>
#include <takatori/util/downcast.h>

#include <mizugaki/ast/name/qualified.h>
#include <mizugaki/ast/type/user_defined.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/builtin_function_invocation.h>

namespace mizugaki::parser {

using ::takatori::util::unsafe_downcast;
using ::takatori::util::maybe_shared_ptr;

using document_type = sql_driver::document_type;

sql_driver::sql_driver(maybe_shared_ptr<document_type const> document) noexcept :
    document_ { std::move(document) }
{}

maybe_shared_ptr<document_type const> const& sql_driver::document() const noexcept {
    return document_;
}

sql_driver::result_type& sql_driver::result() noexcept {
    return result_;
}

sql_driver::result_type const& sql_driver::result() const noexcept {
    return result_;
}

void sql_driver::success(std::vector<node_ptr<ast::statement::statement>> statements) {
    // note: removes the trailing empty statement, it always indicates blank without trailing semicolon
    if (!statements.empty()) {
        auto&& last = statements.back();
        if (last->node_kind() == ast::statement::kind::empty_statement) {
            statements.pop_back();
        }
    }
    result_.value() = node<ast::compilation_unit>(
            std::move(statements),
            std::move(comments_),
            document_);
}

void sql_driver::error(sql_driver::location_type location, sql_parser_result::message_type message) {
    result_ = sql_parser_diagnostic {
            std::move(message),
            document_,
            location,
    };
}

void sql_driver::add_comment(location_type location) {
    comments_.emplace_back(location);
}

std::vector<sql_driver::location_type>& sql_driver::comments() noexcept {
    return comments_;
}

std::vector<sql_driver::location_type> const& sql_driver::comments() const noexcept {
    return comments_;
}

sql_driver::node_ptr<ast::scalar::expression> sql_driver::try_merge_identifier_chain(
        node_ptr<ast::scalar::expression>& qualifier,
        node_ptr<ast::name::simple>& identifier) {
    if (auto name = try_build_identifier_chain(qualifier, identifier)) {
        auto r = name->region();
        return node<ast::scalar::variable_reference>(std::move(name), r);
    }
    if (qualifier->node_kind() == ast::scalar::builtin_function_invocation::tag) {
        auto&& v = unsafe_downcast<ast::scalar::builtin_function_invocation>(*qualifier);
        if (v.function() == ast::scalar::builtin_function_kind::next_value_for && v.arguments().size() == 1) {
            if (auto replaced = try_merge_identifier_chain(v.arguments()[0], identifier)) {
                v.region() = v.region() | replaced->region();
                v.arguments()[0] = std::move(replaced);
                return std::move(qualifier);
            }
        }
    }
    return {};
}

sql_driver::node_ptr<ast::name::name> sql_driver::try_build_identifier_chain(
        node_ptr<ast::scalar::expression>& qualifier,
        node_ptr<ast::name::simple>& identifier) {
    if (qualifier->node_kind() == ast::scalar::variable_reference::tag) {
        auto&& v = unsafe_downcast<ast::scalar::variable_reference>(*qualifier);
        auto r = qualifier->region() | identifier->region();
        return node<ast::name::qualified>(std::move(v.name()), std::move(identifier), r);
    }
    return {};
}

sql_driver::node_ptr<ast::type::type> sql_driver::try_build_type(node_ptr<ast::scalar::expression>& expr) {
    if (expr->node_kind() == ast::scalar::variable_reference::tag) {
        auto&& v = unsafe_downcast<ast::scalar::variable_reference>(*expr);
        return node<ast::type::user_defined>(std::move(v.name()), v.region());
    }
    return {};
}

std::size_t sql_driver::to_size(ast::common::chars const& str) { // NOLINT: non-static for calling conv
    errno = 0;
    auto result = std::strtoull(str.data(), nullptr, 10);
    if (errno == ERANGE) {
        // FIXME: error
        return 0;
    }
    return result;
}

ast::common::chars sql_driver::parse_regular_identifier(ast::common::chars str) { // NOLINT
    return str;
}

ast::common::chars sql_driver::parse_delimited_identifier(ast::common::chars const& str) {
    constexpr char delimiter = '"';
    BOOST_ASSERT(str.size() >= 3); // NOLINT
    BOOST_ASSERT(str.front() == delimiter); // NOLINT
    BOOST_ASSERT(str.back() == delimiter); // NOLINT
    ast::common::chars result {};
    result.reserve(str.size() - 2);
    bool saw_escape = false;
    for (std::size_t i = 1, n = str.size() - 1; i < n; ++i) {
        char c = str[i];
        if (saw_escape) {
            result.push_back(c);
            saw_escape = false;
        } else if (c == delimiter) {
            saw_escape = true;
        } else {
            result.push_back(c);
        }
    }
    BOOST_ASSERT(!saw_escape); // NOLINT
    return result;
}

sql_driver::node_ptr<ast::name::simple>
sql_driver::to_regular_identifier(ast::common::chars str, location_type location) {
    auto identifier = parse_regular_identifier(std::move(str));
    return node<ast::name::simple>(
            std::move(identifier),
            ast::name::simple::identifier_kind_type::regular,
            location);
}

sql_driver::node_ptr<ast::name::simple>
sql_driver::to_delimited_identifier(ast::common::chars const& str, location_type location) {
    auto identifier = parse_delimited_identifier(str);
    return node<ast::name::simple>(
            std::move(identifier),
            ast::name::simple::identifier_kind_type::delimited,
            location);
}

} // namespace mizugaki::parser

