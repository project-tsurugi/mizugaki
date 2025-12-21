#pragma once

#include <optional>
#include <vector>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/scalar/expression.h>

#include "expression.h"
#include "apply_type.h"
#include "correlation_clause.h"

namespace mizugaki::ast::table {

/**
 * @brief invokes table-valued function and correlated-join with the left table.
 * @note This is a non-standard extension, for using table-valued functions.
 *     In standard SQL, this is represented as `JOIN LATERAL TABLE(<function-invocation>)`.
 */
class apply final : public expression {

    using super = expression;

public:
    /// @brief the apply type.
    using operator_kind_type = common::regioned<apply_type>;

    /// @brief the correlation declaration type.
    using correlation_type = correlation_clause;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::apply;

    /**
     * @brief creates a new instance.
     * @param operand the left table operand
     * @param operator_kind the apply type
     * @param name the table-valued function name
     * @param arguments the function arguments
     * @param correlation the correlation declaration
     * @param region the node region
     */
    explicit apply(
            std::unique_ptr<table::expression> operand,
            std::optional<operator_kind_type> operator_kind,
            std::unique_ptr<name::name> name,
            std::vector<std::unique_ptr<scalar::expression>> arguments,
            correlation_type correlation,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param operand the left table operand
     * @param name the table-valued function name
     * @param arguments the function arguments
     * @param correlation the correlation declaration
     * @param operator_kind the apply type
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit apply(
            table::expression&& operand,
            name::name&& name,
            common::rvalue_list<scalar::expression> arguments,
            correlation_type correlation,
            std::optional<operator_kind_type> operator_kind = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit apply(::takatori::util::clone_tag_t, apply const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit apply(::takatori::util::clone_tag_t, apply&& other);

    [[nodiscard]] apply* clone() const& override;
    [[nodiscard]] apply* clone() && override;
    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the left table operand.
     * @return the left table operand
     */
    [[nodiscard]] std::unique_ptr<table::expression>& operand() noexcept;

    /// @copydoc operand()
    [[nodiscard]] std::unique_ptr<table::expression> const& operand() const noexcept;

    /**
     * @brief returns the apply type.
     * @return the apply type
     * @return empty if there is no such the type
     */
    [[nodiscard]] std::optional<operator_kind_type>& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] std::optional<operator_kind_type> const& operator_kind() const noexcept;

    /**
     * @brief returns the function name.
     * @return the function name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the function argument list.
     * @return the function argument list
     */
    [[nodiscard]] std::vector<std::unique_ptr<scalar::expression>>& arguments() noexcept;

    /// @copydoc arguments()
    [[nodiscard]] std::vector<std::unique_ptr<scalar::expression>> const& arguments() const noexcept;

    /**
     * @brief returns the correlation declaration.
     * @return the correlation declaration
     * @return empty if there is no such the declaration
     */
    [[nodiscard]] correlation_type& correlation() noexcept;

    /// @copydoc correlation()
    [[nodiscard]] correlation_type const& correlation() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(apply const& a, apply const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(apply const& a, apply const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<table::expression> operand_;
    std::optional<operator_kind_type> operator_kind_;
    std::unique_ptr<name::name> name_;
    std::vector<std::unique_ptr<scalar::expression>> arguments_;
    correlation_type correlation_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, apply const& value);

} // namespace mizugaki::ast::table
