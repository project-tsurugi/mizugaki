#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/scalar/expression.h>

#include "expression.h"

namespace mizugaki::ast::query {

/**
 * @brief constructs a table from row expressions (`VALUES`).
 * @note `7.3 <table value constructor>`
 */
class table_value_constructor final : public expression {

    using super = expression;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::table_value_constructor;

    /**
     * @brief creates a new instance.
     * @param elements the element **row** expressions
     * @param region the node region
     */
    explicit table_value_constructor(
            std::vector<std::unique_ptr<scalar::expression>> elements,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param elements the element **row** expressions
     * @param region the node region
     * @attention this will take copy of argument
     */
    table_value_constructor(
            common::rvalue_list<scalar::expression> elements,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit table_value_constructor(::takatori::util::clone_tag_t, table_value_constructor const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit table_value_constructor(::takatori::util::clone_tag_t, table_value_constructor&& other);

    [[nodiscard]] table_value_constructor* clone() const& override;
    [[nodiscard]] table_value_constructor* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the element **row** expressions.
     * @return the element expressions
     */
    [[nodiscard]] std::vector<std::unique_ptr<scalar::expression>>& elements() noexcept;

    /// @brief elements()
    [[nodiscard]] std::vector<std::unique_ptr<scalar::expression>> const& elements() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(table_value_constructor const& a, table_value_constructor const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(table_value_constructor const& a, table_value_constructor const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::vector<std::unique_ptr<scalar::expression>> elements_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, table_value_constructor const& value);

} // namespace mizugaki::ast::query
