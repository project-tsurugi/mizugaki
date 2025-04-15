#pragma once

#include <memory>
#include <optional>

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/type/type.h>
#include <mizugaki/ast/scalar/expression.h>

#include "statement.h"
#include "sequence_definition_option.h"
#include "storage_parameter.h"

namespace mizugaki::ast::statement {

/**
 * @brief defines a sequence.
 * @see `[SQL-2003] 11.62 <sequence generator definition>`
 */
class sequence_definition final : public statement {

    using super = statement;

public:
    /// @brief the kind of this element.
    static constexpr node_kind_type tag = node_kind_type::sequence_definition;

    /// @brief option type with element region information.
    using option_type = common::regioned<sequence_definition_option>;

    /**
     * @brief creates a new instance.
     * @param name the sequence name
     * @param type the value type, or empty if it is not defined
     * @param initial_value the initial value, or empty if it is not defined
     * @param increment_value the increment value, or empty if it is not defined
     * @param min_value the minimum value, or empty if it is not defined
     * @param max_value the maximum value, or empty if it is not defined
     * @param owner the owner column name
     * @param options the declared options
     * @param description the region of description comment
     * @param region the node region
     */
    explicit sequence_definition(
            std::unique_ptr<name::name> name,
            std::unique_ptr<type::type> type = {},
            std::unique_ptr<scalar::expression> initial_value = {},
            std::unique_ptr<scalar::expression> increment_value = {},
            std::unique_ptr<scalar::expression> min_value = {},
            std::unique_ptr<scalar::expression> max_value = {},
            std::unique_ptr<name::name> owner = {},
            std::vector<option_type> options = {},
            region_type description = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the sequence name
     * @param type the value type, or empty if it is not defined
     * @param initial_value the initial value, or empty if it is not defined
     * @param increment_value the increment value, or empty if it is not defined
     * @param min_value the minimum value, or empty if it is not defined
     * @param max_value the maximum value, or empty if it is not defined
     * @param owner the owner column name
     * @param options the declared options
     * @param description the region of description comment
     * @param region the node region
     */
    explicit sequence_definition(
            name::name&& name,
            ::takatori::util::rvalue_ptr<type::type> type = {},
            ::takatori::util::rvalue_ptr<scalar::expression> initial_value = {},
            ::takatori::util::rvalue_ptr<scalar::expression> increment_value = {},
            ::takatori::util::rvalue_ptr<scalar::expression> min_value = {},
            ::takatori::util::rvalue_ptr<scalar::expression> max_value = {},
            ::takatori::util::rvalue_ptr<name::name> owner = {},
            std::initializer_list<option_type> options = {},
            region_type description = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit sequence_definition(::takatori::util::clone_tag_t, sequence_definition const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit sequence_definition(::takatori::util::clone_tag_t, sequence_definition&& other);

    [[nodiscard]] sequence_definition* clone() const& override;
    [[nodiscard]] sequence_definition* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the table name.
     * @return the table name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the value type.
     * @return the value type
     * @return empty if it is not defined
     */
    [[nodiscard]] std::unique_ptr<type::type>& type() noexcept;

    /// @copydoc type()
    [[nodiscard]] std::unique_ptr<type::type> const& type() const noexcept;

    /**
     * @brief returns the initial value.
     * @return the initial value
     * @return empty if it is not defined
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& initial_value() noexcept;

    /// @copydoc initial_value()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& initial_value() const noexcept;

    /**
     * @brief returns the increment value.
     * @return the increment value
     * @return empty if it is not defined
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& increment_value() noexcept;

    /// @copydoc increment_value()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& increment_value() const noexcept;

    /**
     * @brief returns the minimum value.
     * @return the minimum value
     * @return empty if it is not defined
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& min_value() noexcept;

    /// @copydoc min_value()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& min_value() const noexcept;

    /**
     * @brief returns the maximum value.
     * @return the maximum value
     * @return empty if it is not defined
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& max_value() noexcept;

    /// @copydoc max_value()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& max_value() const noexcept;

    /**
     * @brief returns list of declared options.
     * @return declared options
     */
    [[nodiscard]] std::vector<option_type>& options() noexcept;

    /// @copydoc options()
    [[nodiscard]] std::vector<option_type> const& options() const noexcept;

    /**
     * @brief returns the region of description comment for this definition.
     * @return the description comment region
     * @return the empty region if not specified
     */
    [[nodiscard]] region_type& description() noexcept;

    /// @copydoc description()
    [[nodiscard]] region_type const& description() const noexcept;

    /**
     * @brief returns the owner column name.
     * @return owner column name
     * @return empty if it is not defined
     */
    [[nodiscard]] std::unique_ptr<name::name>& owner() noexcept;

    /// @copydoc owner()
    [[nodiscard]] std::unique_ptr<name::name> const& owner() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(sequence_definition const& a, sequence_definition const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(sequence_definition const& a, sequence_definition const& b) noexcept;

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_ {};
    std::unique_ptr<type::type> type_ {};
    std::unique_ptr<scalar::expression> initial_value_ {};
    std::unique_ptr<scalar::expression> increment_value_ {};
    std::unique_ptr<scalar::expression> min_value_ {};
    std::unique_ptr<scalar::expression> max_value_ {};
    std::unique_ptr<name::name> owner_ {};
    std::vector<option_type> options_ {};
    region_type description_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, sequence_definition const& value);

} // namespace mizugaki::ast::statement
