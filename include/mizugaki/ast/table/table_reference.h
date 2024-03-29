#pragma once

#include <optional>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/name/name.h>

#include "expression.h"
#include "correlation_clause.h"

namespace mizugaki::ast::table {

/**
 * @brief refers a table by its name.
 * @note `7.6 <table reference>` - `<table or query name>`
 */
class table_reference final : public expression {

    using super = expression;

public:
    /// @brief the correlation declaration type.
    using correlation_type = correlation_clause;

    /// @brief truth type with element region information.
    using bool_type = common::regioned<bool>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::table_reference;

    /**
     * @brief creates a new instance.
     * @param is_only whether or not `ONLY` is specified
     * @param name the table or query name
     * @param correlation the correlation declaration
     * @param region the node region
     */
    explicit table_reference(
            bool_type is_only,
            std::unique_ptr<name::name> name,
            std::optional<correlation_type> correlation = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the table or query name
     * @param correlation the correlation declaration
     * @param is_only whether or not `ONLY` is specified
     * @param region the node region
     * @attention this may take copy of elements
     */
    explicit table_reference(
            name::name&& name,
            std::optional<correlation_type> correlation = {},
            bool_type is_only = false,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit table_reference(::takatori::util::clone_tag_t, table_reference const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit table_reference(::takatori::util::clone_tag_t, table_reference&& other);

    [[nodiscard]] table_reference* clone() const& override;
    [[nodiscard]] table_reference* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the table name.
     * @return the table name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the correlation declaration.
     * @return the correlation declaration
     * @return empty if there is no such the declaration
     */
    [[nodiscard]] std::optional<correlation_type>& correlation() noexcept;

    /// @copydoc correlation()
    [[nodiscard]] std::optional<correlation_type> const& correlation() const noexcept;

    /**
     * @brief returns whether or not `ONLY` is specified.
     * @return true if `ONLY` is specified
     * @return false
     */
    [[nodiscard]] bool_type& is_only() noexcept;

    /// @copydoc is_only()
    [[nodiscard]] bool_type const& is_only() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(table_reference const& a, table_reference const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(table_reference const& a, table_reference const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    bool_type is_only_;
    std::unique_ptr<name::name> name_;
    std::optional<correlation_type> correlation_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, table_reference const& value);

} // namespace mizugaki::ast::table
