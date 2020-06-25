#pragma once

#include <memory>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>

#include "constraint.h"

namespace mizugaki::ast::statement {

/**
 * @brief keyword only constraints.
 * @note `11.4 <column definition>` - `<column constraint>`
 */
class simple_constraint final : public constraint {

    using super = constraint;

public:
    /// @brief the constraint kind type.
    using constraint_kind_type = common::regioned<node_kind_type>;

    /// @brief the available node kind of this.
    static constexpr constraint_kind_set tags {
            node_kind_type::null,
            node_kind_type::not_null,
    };

    /**
     * @brief creates a new instance.
     * @param constraint_kind the constraint kind
     * @param region the node region
     */
    explicit simple_constraint(
            constraint_kind_type constraint_kind,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit simple_constraint(::takatori::util::clone_tag_t, simple_constraint const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit simple_constraint(::takatori::util::clone_tag_t, simple_constraint&& other);

    [[nodiscard]] simple_constraint* clone() const& override;
    [[nodiscard]] simple_constraint* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the constraint kind.
     * @return the constraint kind
     */
    [[nodiscard]] constraint_kind_type& constraint_kind() noexcept;

    /// @copydoc constraint_kind()
    [[nodiscard]] constraint_kind_type const& constraint_kind() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(simple_constraint const& a, simple_constraint const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(simple_constraint const& a, simple_constraint const& b) noexcept;

protected:
    [[nodiscard]] bool equals(constraint const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    constraint_kind_type constraint_kind_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, simple_constraint const& value);

} // namespace mizugaki::ast::statement
