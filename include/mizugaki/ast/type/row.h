#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>

#include "type.h"
#include "field_definition.h"

namespace mizugaki::ast::type {

/**
 * @brief represents a row type.
 */
class row final : public type {

    using super = type;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::row;

    /**
     * @brief creates a new instance.
     * @param elements the field definitions
     * @param region the node region
     */
    explicit row(
            common::vector<field_definition> elements,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param elements the field definitions
     * @param region the node region
     * @attention this will take a copy of argument
     */
    row(
            std::initializer_list<field_definition> elements,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit row(row const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit row(row&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] row* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] row* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the field definitions.
     * @return the field definitions
     */
    [[nodiscard]] common::vector<field_definition>& elements() noexcept;

    /// @copydoc elements()
    [[nodiscard]] common::vector<field_definition> const& elements() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(row const& a, row const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(row const& a, row const& b) noexcept;

protected:
    [[nodiscard]] bool equals(type const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    common::vector<field_definition> elements_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, row const& value);

} // namespace mizugaki::ast::type
