#pragma once

#include <optional>

#include <mizugaki/ast/common/chars.h>
#include <mizugaki/ast/common/regioned.h>

#include "literal.h"
#include "sign.h"

namespace mizugaki::ast::literal {

/**
 * @brief represents a datetime interval value.
 */
class interval final : public literal {

    using super = literal;

public:
    /// @brief the sign type.
    using sign_type = common::regioned<::mizugaki::ast::literal::sign>;

    /// @brief the value type.
    using value_type = common::regioned<common::chars>;

    // FIXME: impl using qualifier_type = common::interval_qualifier;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::interval;

    /**
     * @brief creates a new instance.
     * @param sign the numeric sign
     * @param value quoted string representation of the value
     * @param region the node region
     */
    explicit interval(
            std::optional<sign_type> sign,
            value_type value,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit interval(interval const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit interval(interval&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] interval* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] interval* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the numeric sign.
     * @return the numeric sign
     */
    [[nodiscard]] std::optional<sign_type>& sign() noexcept;

    /// @copydoc sign()
    [[nodiscard]] std::optional<sign_type> const& sign() const noexcept;

    /**
     * @brief returns the quoted string representation of the value.
     * @return the value string
     */
    [[nodiscard]] value_type& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] value_type const& value() const noexcept;

    // FIXME: impl std::optional<qualifier_type> start()
    // FIXME: impl std::optional<qualifier_type> end()

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(interval const& a, interval const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(interval const& a, interval const& b) noexcept;

protected:
    [[nodiscard]] bool equals(literal const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::optional<sign_type> sign_;
    value_type value_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, interval const& value);

} // namespace mizugaki::ast::literal
