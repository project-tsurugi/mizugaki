#pragma once

#include <cstddef>

#include <takatori/util/object_creator.h>

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
    using node_ptr = ::takatori::util::unique_object_ptr<T>;

    explicit sql_driver(
            ::takatori::util::maybe_shared_ptr<document_type const> document,
            ::takatori::util::object_creator creator) noexcept;

    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const> const& document() const noexcept;

    [[nodiscard]] ::takatori::util::object_creator get_object_creator() const noexcept;

    [[nodiscard]] result_type& result() noexcept;

    [[nodiscard]] result_type const& result() const noexcept;

    void success(ast::common::vector<node_ptr<ast::statement::statement>> statements);

    void error(location_type location, result_type::message_type message);

    void add_comment(location_type location);

    [[nodiscard]] ast::common::vector<location_type>& comments() noexcept;

    [[nodiscard]] ast::common::vector<location_type> const& comments() const noexcept;

    template<class T, class... Args>
    [[nodiscard]] node_ptr<T> node(Args&&... args) {
        return creator_.create_unique<T>(std::forward<Args>(args)...);
    }

    template<class T>
    [[nodiscard]] ast::common::vector<node_ptr<T>> node_vector(std::size_t capacity = 0) {
        ast::common::vector<node_ptr<T>> result { creator_.allocator() };
        result.reserve(capacity);
        return result;
    }

    template<class T, class... Args>
    [[nodiscard]] ast::common::vector<node_ptr<T>> to_node_vector(Args&&... args) {
        auto result = node_vector<T>(sizeof...(args));
        (..., result.emplace_back(std::forward<Args>(args)));
        return result;
    }

    template<class T>
    [[nodiscard]] ast::common::vector<T> element_vector(std::size_t capacity = 0) {
        ast::common::vector<T> result { creator_.allocator() };
        result.reserve(capacity);
        return result;
    }

    [[nodiscard]] std::size_t to_size(ast::common::chars const& str);

    [[nodiscard]] ast::common::chars parse_delimited_identifier(ast::common::chars const& str);

    [[nodiscard]] node_ptr<ast::name::name> try_build_identifier_chain(
            node_ptr<ast::scalar::expression>& qualifier,
            node_ptr<ast::name::simple>& identifier);

    [[nodiscard]] node_ptr<ast::type::type> try_build_type(node_ptr<ast::scalar::expression>& expr);

private:
    ::takatori::util::maybe_shared_ptr<document_type const> document_;
    ::takatori::util::object_creator creator_;
    ast::common::vector<location_type> comments_;
    result_type result_ {};
};

} // namespace sandbox