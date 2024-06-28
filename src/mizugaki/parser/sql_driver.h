#pragma once

#include <cstddef>


#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/type/type.h>
#include <mizugaki/ast/scalar/expression.h>
#include <mizugaki/ast/statement/statement.h>

#include <mizugaki/parser/sql_parser_result.h>

namespace mizugaki::parser {

class sql_driver {
public:
    using document_type = ::takatori::document::document;
    using result_type = sql_parser_result;

    using location_type = ast::node_region;

    template<class T>
    using node_ptr = std::unique_ptr<T>;

    explicit sql_driver(::takatori::util::maybe_shared_ptr<document_type const> document) noexcept;

    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const> const& document() const noexcept;

    [[nodiscard]] result_type& result() noexcept;

    [[nodiscard]] result_type const& result() const noexcept;

    void success(std::vector<node_ptr<ast::statement::statement>> statements);

    void error(location_type location, result_type::message_type message);

    void add_comment(location_type location);

    [[nodiscard]] std::vector<location_type>& comments() noexcept;

    [[nodiscard]] std::vector<location_type> const& comments() const noexcept;

    template<class T, class... Args>
    [[nodiscard]] node_ptr<T> node(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<class T>
    [[nodiscard]] std::vector<node_ptr<T>> node_vector(std::size_t capacity = 0) {
        std::vector<node_ptr<T>> result {};
        result.reserve(capacity);
        return result;
    }

    template<class T, class... Args>
    [[nodiscard]] std::vector<node_ptr<T>> to_node_vector(Args&&... args) {
        auto result = node_vector<T>(sizeof...(args));
        (..., result.emplace_back(std::forward<Args>(args)));
        return result;
    }

    template<class T>
    [[nodiscard]] std::vector<T> element_vector(std::size_t capacity = 0) {
        std::vector<T> result {};
        result.reserve(capacity);
        return result;
    }

    [[nodiscard]] std::size_t to_size(ast::common::chars const& str);

    [[nodiscard]] ast::common::chars parse_regular_identifier(ast::common::chars str);

    [[nodiscard]] ast::common::chars parse_delimited_identifier(ast::common::chars const& str);

    [[nodiscard]] node_ptr<ast::name::simple> to_regular_identifier(ast::common::chars str, location_type location);

    [[nodiscard]] node_ptr<ast::name::simple> to_delimited_identifier(
            ast::common::chars const& str,
            location_type location);

    [[nodiscard]] node_ptr<ast::scalar::expression> try_merge_identifier_chain(
            node_ptr<ast::scalar::expression>& qualifier,
            node_ptr<ast::name::simple>& identifier);

    [[nodiscard]] node_ptr<ast::name::name> try_build_identifier_chain(
            node_ptr<ast::scalar::expression>& qualifier,
            node_ptr<ast::name::simple>& identifier);

    [[nodiscard]] node_ptr<ast::type::type> try_build_type(node_ptr<ast::scalar::expression>& expr);

private:
    ::takatori::util::maybe_shared_ptr<document_type const> document_;
    std::vector<location_type> comments_;
    result_type result_ {};
};

} // namespace sandbox
