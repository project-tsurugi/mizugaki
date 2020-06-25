#pragma once

#include <memory>
#include <optional>

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/query/expression.h>

#include "statement.h"
#include "view_definition_option.h"
#include "storage_parameter.h"

namespace mizugaki::ast::statement {

/**
 * @brief defines a view.
 * @note `11.21 <view definition>`
 */
class view_definition final : public statement {

    using super = statement;

public:
    /// @brief the kind of this element.
    static constexpr node_kind_type tag = node_kind_type::view_definition;

    /// @brief option type with element region information.
    using option_type = common::regioned<view_definition_option>;

    /**
     * @brief creates a new instance.
     * @param name the view name
     * @param columns the view column names
     * @param query  the binding query
     * @param options declared options
     * @param parameters the storage parameters
     * @param region the node region
     */
    explicit view_definition(
            std::unique_ptr<name::name> name,
            std::vector<std::unique_ptr<name::simple>> columns,
            std::unique_ptr<query::expression> query,
            std::vector<option_type> options = {},
            std::vector<storage_parameter> parameters = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the view name
     * @param columns the view column names
     * @param query  the binding query
     * @param options declared options
     * @param parameters the storage parameters
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit view_definition(
            name::name&& name,
            common::rvalue_list<name::simple> columns,
            query::expression&& query,
            std::initializer_list<option_type> options = {},
            std::initializer_list<storage_parameter> parameters = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit view_definition(::takatori::util::clone_tag_t, view_definition const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit view_definition(::takatori::util::clone_tag_t, view_definition&& other);

    [[nodiscard]] view_definition* clone() const& override;
    [[nodiscard]] view_definition* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the view name.
     * @return the view name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the view column names.
     * @return the view column names
     */
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>>& columns() noexcept;

    /// @copydoc columns()
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>> const& columns() const noexcept;

    /**
     * @brief returns the binding query expression.
     * @return the binding query expression
     */
    [[nodiscard]] std::unique_ptr<query::expression>& query() noexcept;

    /// @copydoc query()
    [[nodiscard]] std::unique_ptr<query::expression> const& query() const noexcept;

    /**
     * @brief returns list of declared options.
     * @return declared options
     */
    [[nodiscard]] std::vector<option_type>& options() noexcept;

    /// @copydoc options()
    [[nodiscard]] std::vector<option_type> const& options() const noexcept;

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
    friend bool operator==(view_definition const& a, view_definition const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(view_definition const& a, view_definition const& b) noexcept;

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_ {};
    std::vector<std::unique_ptr<name::simple>> columns_ {};
    std::unique_ptr<query::expression> query_ {};
    std::vector<option_type> options_ {};
    std::vector<storage_parameter> parameters_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, view_definition const& value);

} // namespace mizugaki::ast::statement
