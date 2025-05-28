#include <mizugaki/parser/sql_driver.h>

#include <cctype>
#include <cerrno>
#include <cstdlib>

#include <algorithm>
#include <iterator>
#include <string_view>

#include <takatori/util/assertion.h>
#include <takatori/util/downcast.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/name/qualified.h>
#include <mizugaki/ast/type/user_defined.h>
#include <mizugaki/ast/literal/numeric.h>
#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/unary_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/builtin_function_invocation.h>

namespace mizugaki::parser {

using ::takatori::util::unsafe_downcast;
using ::takatori::util::maybe_shared_ptr;

using document_type = sql_driver::document_type;

namespace {

using std::string_view_literals::operator""sv; // NOLINT(misc-unused-using-decls)

constexpr std::string_view prefix_description_comment = "/**"sv;

constexpr std::string_view suffix_description_comment = "*/"sv;

} // namespace

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

void sql_driver::error(
        diagnostic_code_type code,
        location_type location,
        result_type::message_type message) {
    result_ = sql_parser_diagnostic {
            code,
            std::move(message),
            document_,
            location,
    };
}

void sql_driver::add_comment(location_type location) {
    if (!saw_comments_) {
        auto separator = last_comment_separator_ ? last_comment_separator_.begin : 0;
        if (comment_separators_.empty() || comment_separators_.back() < separator) {
            comment_separators_.emplace_back(separator);
        }
        saw_comments_ = true;
    }
    comments_.emplace_back(location);
}

void sql_driver::add_comment_separator(location_type location) {
    if (saw_comments_) {
        // NOTE: comment_separators_ are not empty if saw_comments_ is true
        auto separator = location.begin;
        if (comment_separators_.back() < separator) {
            comment_separators_.emplace_back(separator);
        }
        saw_comments_ = false;
    }
    last_comment_separator_ = location;
}

void sql_driver::add_placeholder_mark(location_type token) {
    if (!token) {
        return;
    }
    placeholder_marks_.emplace_back(token.begin);
}

std::size_t sql_driver::find_placeholder_mark(location_type token) const {
    if (!token) {
        return 0;
    }
    auto key = token.begin;
    auto iter = std::lower_bound(
            placeholder_marks_.begin(),
            placeholder_marks_.end(),
            key);
    if (iter == placeholder_marks_.end() || *iter != key) {
        return 0; // placeholder is not found
    }
    // compute the placeholder position (1-origin)
    return static_cast<std::size_t>(std::distance(placeholder_marks_.begin(), iter)) + 1;
}

std::size_t& sql_driver::max_expected_candidates() noexcept {
    return max_expected_candidates_;
}

::takatori::util::optional_ptr<sql_parser_element_map<std::size_t> const>& sql_driver::element_limits() noexcept {
    return element_limits_;
}

std::vector<sql_driver::location_type>& sql_driver::comments() noexcept {
    return comments_;
}

std::vector<sql_driver::location_type> const& sql_driver::comments() const noexcept {
    return comments_;
}

std::vector<sql_driver::location_type> sql_driver::leading_comments(location_type token) const {
    if (!token) {
        return {};
    }
    auto key = token.begin;
    auto iter = std::lower_bound(
            comment_separators_.begin(),
            comment_separators_.end(),
            key);
    if (iter == comment_separators_.end() || *iter != key) {
        return {}; // separator not found
    }
    if (iter == comment_separators_.begin()) {
        return {}; // no leading comments
    }
    auto prev = std::prev(iter);
    return comments_in_range(*prev, *iter);
}

std::vector<sql_driver::location_type> sql_driver::following_comments(location_type token) const {
    if (!token) {
        return {};
    }
    auto key = token.begin;
    auto iter = std::lower_bound(
            comment_separators_.begin(),
            comment_separators_.end(),
            key);
    if (iter == comment_separators_.end() || *iter != key) {
        return {}; // separator not found
    }
    auto next = std::next(iter);
    if (next == comment_separators_.end()) {
        if (!document_) {
            return {};
        }
        return comments_in_range(*iter, document_->size());
    }
    return comments_in_range(*iter, *next);
}

bool& sql_driver::enable_description_comments() noexcept {
    return enable_description_comments_;
}

sql_driver::location_type sql_driver::leading_description_comment(location_type token) const {
    if (!enable_description_comments_) {
        return {};
    }
    auto comments = leading_comments(token);
    if (comments.empty()) {
        return {};
    }
    if (!document_) {
        return {};
    }
    auto&& doc = *document_;
    auto iter = std::find_if(
            comments.rbegin(),
            comments.rend(),
            [&doc](location_type comment) {
                if (comment.size() < prefix_description_comment.size() + suffix_description_comment.size()) {
                    return false;
                }
                auto prefix = doc.contents(
                        comment.begin,
                        prefix_description_comment.size());
                if (prefix != prefix_description_comment) {
                    return false;
                }
                auto suffix = doc.contents(
                        comment.end - suffix_description_comment.size(),
                        suffix_description_comment.size());
                if (suffix != suffix_description_comment) {
                    return false; // NOLINT
                }
                return true;
            });
    if (iter == comments.rend()) {
        return {};
    }
    return *iter;
}

std::vector<sql_driver::location_type> sql_driver::comments_in_range(
        location_type::position_type from,
        location_type::position_type to) const {
    if (from > to) {
        return {};
    }
    auto iter_lower = std::lower_bound(
            comments_.begin(),
            comments_.end(),
            location_type { from, from },
            [](location_type a, location_type b) {
                return a.begin < b.begin;
            });
    if (iter_lower == comments_.end()) {
        return {};
    }
    auto iter_upper = std::upper_bound(
            comments_.begin(),
            comments_.end(),
            location_type { to, to },
            [](location_type a, location_type b) {;
                return a.begin < b.begin;
            });
    if (iter_upper <= iter_lower) {
        return {};
    }

    return { iter_lower, iter_upper };
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

std::string_view sql_driver::image(location_type location) {
    if (location.size() == 0) {
        return {};
    }
    return document_->contents(location.first(), location.size());
}

bool sql_driver::check_regular_identifier(const ast::common::chars& str) {
    if (str.size() >= 2) {
        return str[0] != '_' || str[1] != '_';
    }
    return true;
}

bool sql_driver::check_delimited_identifier(const ast::common::chars &str) {
    if (str.size() >= 3) {
        return str[1] != '_' || str[2] != '_';
    }
    return true;
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

sql_driver::node_ptr<ast::scalar::expression>
sql_driver::try_fold_literal(node_ptr<ast::scalar::expression> expression) {
    if (expression->node_kind() != ast::scalar::unary_expression::tag) {
        return expression;
    }

    auto&& unary = unsafe_downcast<ast::scalar::unary_expression>(*expression);
    if (unary.operator_kind() != ast::scalar::unary_operator::plus &&
            unary.operator_kind() != ast::scalar::unary_operator::minus) {
        return expression;
    }
    if (unary.operand()->node_kind() != ast::scalar::literal_expression::tag) {
        return expression;
    }

    auto&& literal = unsafe_downcast<ast::scalar::literal_expression>(*unary.operand()).value();
    if (literal->node_kind() != ast::literal::kind::exact_numeric &&
            literal->node_kind() != ast::literal::kind::approximate_numeric) {
        return expression;
    }

    // only set if the original literal does not have a sign
    auto&& value = unsafe_downcast<ast::literal::numeric>(*literal);
    if (value.sign().has_value()) {
        return expression;
    }

    if (unary.operator_kind() == ast::scalar::unary_operator::plus) {
        value.sign() = { ast::literal::sign::plus, unary.operator_kind().region() };
    } else {
        value.sign() = { ast::literal::sign::minus, unary.operator_kind().region() };
    }
    unary.operand()->region() = expression->region();

    return std::move(unary.operand());
}

bool sql_driver::validate_count(location_type location, std::size_t size, sql_driver::element_kind kind) {
    if (!element_limits_) {
        return true;
    }
    auto limit = (*element_limits_)[kind];
    if (limit != 0 && limit < size) {
        using ::takatori::util::string_builder;
        error(sql_parser_code::exceed_number_of_elements, location,
                string_builder {}
                    << "SQL parser has reached the max number of elements: "
                    << kind
                    << " must be less than or equal to " << limit
                    << string_builder::to_string);
        return false;
    }
    return true;
}

} // namespace mizugaki::parser

