#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/name/name.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief a pseudo expression that tests whether or not current cursor position.
 * @note `14.6 <delete statement: positioned>`
 * @note `14.9 <update statement: positioned>`
 */
class current_of_cursor final : public expression {

    using super = expression;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::current_of_cursor;

    /**
     * @brief creates a new instance.
     * @param name the cursor name
     * @param region the node region
     */
    explicit current_of_cursor(
            std::unique_ptr<name::name> name,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the variable name
     * @param region the node region
     * @attention this will take copy of argument
     */
    explicit current_of_cursor(
            name::name&& name,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit current_of_cursor(::takatori::util::clone_tag_t, current_of_cursor const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit current_of_cursor(::takatori::util::clone_tag_t, current_of_cursor&& other);

    [[nodiscard]] current_of_cursor* clone() const& override;
    [[nodiscard]] current_of_cursor* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the cursor name.
     * @return the cursor name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(current_of_cursor const& a, current_of_cursor const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(current_of_cursor const& a, current_of_cursor const& b) noexcept;
    
protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, current_of_cursor const& value);

} // namespace mizugaki::ast::scalar
