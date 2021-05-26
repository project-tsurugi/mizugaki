#pragma once

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/common/regioned.h>

#include "expression.h"
#include "pattern_match_operator.h"

namespace mizugaki::ast::scalar {

/**
 * @brief processes a pattern-match comparison.
 * @note `8.5 <like predicate>`
 * @note `8.6 <similar predicate>`
 * @see pattern_match_operator
 */
class pattern_match_predicate final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<pattern_match_operator>;

    /// @brief truth type with element region information.
    using bool_type = common::regioned<bool>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::pattern_match_predicate;

    /**
     * @brief creates a new instance.
     * @param match_value the match value
     * @param is_not whether or not `NOT` is declared
     * @param operator_kind the pattern match operator
     * @param pattern the pattern string
     * @param escape the escape character
     * @param region the node region
     */
    explicit pattern_match_predicate(
            operand_type match_value,
            bool_type is_not,
            operator_kind_type operator_kind,
            operand_type pattern,
            operand_type escape = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param match_value the match value
     * @param operator_kind the pattern match operator
     * @param pattern the pattern string
     * @param escape the escape character
     * @param is_not whether or not `NOT` is declared
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit pattern_match_predicate(
            expression&& match_value,
            operator_kind_type operator_kind,
            expression&& pattern,
            ::takatori::util::rvalue_ptr<expression> escape = {},
            bool_type is_not = false,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit pattern_match_predicate(::takatori::util::clone_tag_t, pattern_match_predicate const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit pattern_match_predicate(::takatori::util::clone_tag_t, pattern_match_predicate&& other);

    [[nodiscard]] pattern_match_predicate* clone() const& override;
    [[nodiscard]] pattern_match_predicate* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the match value.
     * @return the match value
     */
    [[nodiscard]] operand_type& match_value() noexcept;

    /// @copydoc match_value()
    [[nodiscard]] operand_type const& match_value() const noexcept;

    /**
     * @brief returns the search pattern.
     * @return the search pattern
     */
    [[nodiscard]] operand_type& pattern() noexcept;

    /// @copydoc pattern()
    [[nodiscard]] operand_type const& pattern() const noexcept;

    /**
     * @brief returns the escape character.
     * @return the escape character
     */
    [[nodiscard]] operand_type& escape() noexcept;

    /// @copydoc escape()
    [[nodiscard]] operand_type const& escape() const noexcept;

    /**
     * @brief returns whether or not this declared as `NOT`.
     * @return true if `NOT` is declared
     * @return false otherwise
     */
    [[nodiscard]] bool_type& is_not() noexcept;

    /// @copydoc is_not()
    [[nodiscard]] bool_type const& is_not() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(pattern_match_predicate const& a, pattern_match_predicate const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(pattern_match_predicate const& a, pattern_match_predicate const& b) noexcept;
    
protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operand_type match_value_;
    bool_type is_not_;
    operator_kind_type operator_kind_;
    operand_type pattern_;
    operand_type escape_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, pattern_match_predicate const& value);

} // namespace mizugaki::ast::scalar
