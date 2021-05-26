#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/scalar/expression.h>

#include "select_element.h"

namespace mizugaki::ast::query {

/**
 * @brief table column enumerations in select list.
 * @note `7.11 <query specification>` - `<asterisk>`
 * @note `7.11 <query specification>` - `<qualified asterisk>`
 * @note `7.11 <query specification>` - `<all fields reference>`
 */
class select_asterisk : public select_element {

    using super = select_element;

public:
    /// @brief the kind of this element.
    static constexpr select_element_kind tag = select_element_kind::asterisk;

    /**
     * @brief creates a new instance.
     * @param qualifier the optional qualifier
     * @param region the element region
     */
    explicit select_asterisk(
            std::unique_ptr<scalar::expression> qualifier = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param qualifier the optional qualifier
     * @param region the element region
     * @attention this may take copy of elements
     */
    explicit select_asterisk(
            scalar::expression&& qualifier,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit select_asterisk(::takatori::util::clone_tag_t, select_asterisk const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit select_asterisk(::takatori::util::clone_tag_t, select_asterisk&& other);

    [[nodiscard]] select_asterisk* clone() const& override;
    [[nodiscard]] select_asterisk* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the qualifier.
     * @return the qualifier name
     * @return empty there is no qualifier
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& qualifier() noexcept;

    /// @copydoc qualifier()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& qualifier() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(select_asterisk const& a, select_asterisk const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(select_asterisk const& a, select_asterisk const& b) noexcept;

protected:
    [[nodiscard]] bool equals(select_element const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<scalar::expression> qualifier_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, select_asterisk const& value);

} // namespace mizugaki::ast::query
