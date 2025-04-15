#pragma once

#include <memory>
#include <optional>

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/common/sort_element.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/scalar/expression.h>

#include "statement.h"
#include "index_definition_option.h"
#include "storage_parameter.h"

namespace mizugaki::ast::statement {

/**
 * @brief defines an index.
 */
class index_definition final : public statement {

    using super = statement;

public:
    /// @brief the kind of this element.
    static constexpr node_kind_type tag = node_kind_type::index_definition;

    /// @brief option type with element region information.
    using option_type = common::regioned<index_definition_option>;

    /**
     * @brief creates a new instance.
     * @param name the index name
     * @param table_name the target table name
     * @param keys the index keys
     * @param values the index values
     * @param predicate the index predicate for partial indices
     * @param options the declared options
     * @param parameters the storage parameters
     * @param description the region of description comment
     * @param region the node region
     */
    explicit index_definition(
            std::unique_ptr<name::name> name,
            std::unique_ptr<name::name> table_name,
            std::vector<common::sort_element> keys,
            std::vector<std::unique_ptr<scalar::expression>> values = {},
            std::unique_ptr<scalar::expression> predicate = {},
            std::vector<option_type> options = {},
            std::vector<storage_parameter> parameters = {},
            region_type description = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the index name
     * @param table_name the target table name
     * @param keys the index keys
     * @param values the index values
     * @param predicate the index predicate for partial indices
     * @param options the declared options
     * @param parameters the storage parameters
     * @param description the region of description comment
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit index_definition(
            ::takatori::util::rvalue_ptr<name::name> name,
            name::name&& table_name,
            std::initializer_list<common::sort_element> keys,
            common::rvalue_list<scalar::expression> values = {},
            ::takatori::util::rvalue_ptr<scalar::expression> predicate = {},
            std::initializer_list<option_type> options = {},
            std::initializer_list<storage_parameter> parameters = {},
            region_type description = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit index_definition(::takatori::util::clone_tag_t, index_definition const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit index_definition(::takatori::util::clone_tag_t, index_definition&& other);

    [[nodiscard]] index_definition* clone() const& override;
    [[nodiscard]] index_definition* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the index name.
     * @return the index name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;


    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the target table name.
     * @return the target table name
     */
    [[nodiscard]] std::unique_ptr<name::name>& table_name() noexcept;
    
    /// @copydoc table_name()
    [[nodiscard]] std::unique_ptr<name::name> const& table_name() const noexcept;

    /**
     * @brief returns the index keys.
     * @return the index keys
     */
    [[nodiscard]] std::vector<common::sort_element>& keys() noexcept;
    
    /// @copydoc keys()
    [[nodiscard]] std::vector<common::sort_element> const& keys() const noexcept;

    /**
     * @brief returns the index values.
     * @return the index values
     */
    [[nodiscard]] std::vector<std::unique_ptr<scalar::expression>>& values() noexcept;
    
    /// @copydoc values()
    [[nodiscard]] std::vector<std::unique_ptr<scalar::expression>> const& values() const noexcept;

    /**
     * @brief returns the index predicate for partial indices.
     * @return the index predicate for partial indices
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& predicate() noexcept;
    
    /// @copydoc predicate()
    [[nodiscard]] std::unique_ptr<scalar::expression>const& predicate() const noexcept;

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
     * @brief returns the region of description comment for this definition.
     * @return the description comment region
     * @return the empty region if not specified
     */
    [[nodiscard]] region_type& description() noexcept;

    /// @copydoc description()
    [[nodiscard]] region_type const& description() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(index_definition const& a, index_definition const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(index_definition const& a, index_definition const& b) noexcept;

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_ {};
    std::unique_ptr<name::name> table_name_ {};
    std::vector<common::sort_element> keys_ {};
    std::vector<std::unique_ptr<scalar::expression>> values_ {};
    std::unique_ptr<scalar::expression> predicate_ {};
    std::vector<option_type> options_ {};
    std::vector<storage_parameter> parameters_ {};
    region_type description_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, index_definition const& value);

} // namespace mizugaki::ast::statement
