#pragma once

#include <takatori/util/clone_tag.h>

#include "statement.h"

namespace mizugaki::ast::statement {

/**
 * @brief statement that does nothing.
 */
class empty_statement final : public statement {

    using super = statement;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::empty_statement;

    /**
     * @brief creates a new instance.
     * @param region the node region
     */
    explicit empty_statement(
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit empty_statement(::takatori::util::clone_tag_t, empty_statement const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit empty_statement(::takatori::util::clone_tag_t, empty_statement&& other);

    [[nodiscard]] empty_statement* clone() const& override;
    [[nodiscard]] empty_statement* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(empty_statement const& a, empty_statement const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(empty_statement const& a, empty_statement const& b) noexcept;
    
protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, empty_statement const& value);

} // namespace mizugaki::ast::statement
