#pragma once

#include <memory>
#include <vector>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/sort_element.h>
#include <mizugaki/ast/common/rvalue_list.h>

#include <mizugaki/ast/scalar/expression.h>

#include "constraint.h"
#include "storage_parameter.h"

namespace mizugaki::ast::statement {

/**
 * @brief key based constraints.
 * @details key will be omitted if this constraint is for columns.
 * @note `11.6 <table constraint definition>`
 */
class key_constraint final : public constraint {

    using super = constraint;

public:
    /// @brief the constraint kind type.
    using constraint_kind_type = common::regioned<node_kind_type>;

    /// @brief the available node kind of this.
    static constexpr constraint_kind_set tags{
            node_kind_type::unique,
            node_kind_type::primary_key,
    };

    /**
     * @brief creates a new instance.
     * @param constraint_kind the constraint kind
     * @param key the target key, or empty for column constraints
     * @param values the index values
     * @param parameters the storage parameters
     * @param region the node region
     */
    explicit key_constraint(
            constraint_kind_type constraint_kind,
            std::vector<common::sort_element> key,
            std::vector<std::unique_ptr<scalar::expression>> values = {},
            std::vector<storage_parameter> parameters = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param constraint_kind the constraint kind
     * @param key the target key, or empty for column constraints
     * @param values the index values
     * @param parameters the storage parameters
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit key_constraint(
            constraint_kind_type constraint_kind,
            std::initializer_list<common::sort_element> key = {},
            common::rvalue_list<scalar::expression> values = {},
            std::initializer_list<storage_parameter> parameters = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit key_constraint(::takatori::util::clone_tag_t, key_constraint const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit key_constraint(::takatori::util::clone_tag_t, key_constraint&& other);

    [[nodiscard]] key_constraint* clone() const& override;

    [[nodiscard]] key_constraint* clone()&& override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the constraint kind.
     * @return the constraint kind
     */
    [[nodiscard]] constraint_kind_type& constraint_kind() noexcept;

    /// @copydoc constraint_kind()
    [[nodiscard]] constraint_kind_type const& constraint_kind() const noexcept;

    /**
     * @brief return the index key.
     * @return the index key
     * @return empty if this constraint is for columns.
     */
    [[nodiscard]] std::vector<common::sort_element>& key() noexcept;

    /// @copydoc key()
    [[nodiscard]] std::vector<common::sort_element> const& key() const noexcept;

    /**
     * @brief returns the index values.
     * @return returns the index values
     * @return empty if they are not defined
     */
    [[nodiscard]] std::vector<std::unique_ptr<scalar::expression>>& values() noexcept;
    
    /// @copydoc values()
    [[nodiscard]] std::vector<std::unique_ptr<scalar::expression>> const& values() const noexcept;

    /**
     * @brief returns the storage parameters.
     * @return the storage parameters
     */
    [[nodiscard]] std::vector<storage_parameter>& parameters() noexcept;
    
    /// @copydoc parameters()
    [[nodiscard]] std::vector<storage_parameter> const& parameters() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(key_constraint const& a, key_constraint const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(key_constraint const& a, key_constraint const& b) noexcept;

protected:
    [[nodiscard]] bool equals(constraint const& other) const noexcept override;

    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    constraint_kind_type constraint_kind_;
    std::vector<common::sort_element> key_ {};
    std::vector<std::unique_ptr<scalar::expression>> values_ {};
    std::vector<storage_parameter> parameters_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, key_constraint const& value);

} // namespace mizugaki::ast::statement
