#pragma once

#include <takatori/util/clone_tag.h>

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
            std::vector<field_definition> elements,
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
     */
    explicit row(::takatori::util::clone_tag_t, row const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit row(::takatori::util::clone_tag_t, row&& other);

    [[nodiscard]] row* clone() const& override;
    [[nodiscard]] row* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the field definitions.
     * @return the field definitions
     */
    [[nodiscard]] std::vector<field_definition>& elements() noexcept;

    /// @copydoc elements()
    [[nodiscard]] std::vector<field_definition> const& elements() const noexcept;

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
    std::vector<field_definition> elements_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, row const& value);

} // namespace mizugaki::ast::type
