#pragma once

#include <optional>

#include <mizugaki/ast/common/chars.h>
#include <mizugaki/ast/common/regioned.h>

#include "literal.h"
#include "sign.h"

namespace mizugaki::ast::literal {

/**
 * @brief represents a numeric value.
 */
class numeric final : public literal {

    using super = literal;

public:
    /// @brief the value kind type.
    using value_kind_type = node_kind_type;

    /// @brief the sign type.
    using sign_type = common::regioned<::mizugaki::ast::literal::sign>;

    /// @brief the value type.
    using value_type = common::regioned<common::chars>;

    /// @brief the node kind of this.
    static constexpr kind_set tags {
            node_kind_type::exact_numeric,
            node_kind_type::approximate_numeric,
    };

    /**
     * @brief creates a new instance.
     * @param value_kind the value kind, must be one of `exact_numeric` or `approximate_numeric`
     * @param sign the sign
     * @param unsigned_value the unsigned numeric
     * @param region the node region
     * @throws std::invalid_argument if kind is invalid
     * @see tags
     */
    explicit numeric(
            value_kind_type value_kind,
            std::optional<sign_type> sign,
            value_type unsigned_value,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit numeric(numeric const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit numeric(numeric&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] numeric* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] numeric* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the value kind.
     * @return the value kind
     */
    [[nodiscard]] value_kind_type& value_kind() noexcept;

    /// @copydoc value_kind()
    [[nodiscard]] value_kind_type const& value_kind() const noexcept;

    /**
     * @brief returns the numeric sign.
     * @return the numeric sign
     */
    [[nodiscard]] std::optional<sign_type>& sign() noexcept;

    /// @copydoc sign()
    [[nodiscard]] std::optional<sign_type> const& sign() const noexcept;

    /**
     * @brief returns string representation of the unsigned value.
     * @return the unsigned value string
     */
    [[nodiscard]] value_type& unsigned_value() noexcept;

    /// @copydoc unsigned_value()
    [[nodiscard]] value_type const& unsigned_value() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(numeric const& a, numeric const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(numeric const& a, numeric const& b) noexcept;

protected:
    [[nodiscard]] bool equals(literal const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    value_kind_type value_kind_;
    std::optional<sign_type> sign_;
    value_type unsigned_value_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, numeric const& value);

} // namespace mizugaki::ast::literal
