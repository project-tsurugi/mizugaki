#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/target_element.h>
#include <mizugaki/ast/query/expression.h>

#include "statement.h"

namespace mizugaki::ast::statement {

using common::target_element;

/**
 * @brief processes a query expression.
 */
class select_statement final : public statement {

    using super = statement;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::select_statement;

    /**
     * @brief creates a new instance.
     * @param expression the query expression
     * @param targets the optional target outputs
     * @param region the node region
     */
    explicit select_statement(
            ::takatori::util::unique_object_ptr<query::expression> expression,
            common::vector<target_element> targets,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit select_statement(select_statement const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit select_statement(select_statement&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] select_statement* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] select_statement* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the query expression.
     * @return the query expression
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression>& expression() noexcept;

    /// @brief expression()
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression> const& expression() const noexcept;

    /**
     * @brief returns the optional output targets.
     * @return the output targets to store the result of expression()
     * @return empty if the output targets are not declared
     * @note `14.5 <select statement: single row>`
     */
    [[nodiscard]] common::vector<target_element>& targets() noexcept;

    /// @copydoc targets()
    [[nodiscard]] common::vector<target_element> const& targets() const noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(select_statement const& a, select_statement const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(select_statement const& a, select_statement const& b) noexcept;

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;

private:
    ::takatori::util::unique_object_ptr<query::expression> expression_;
    common::vector<target_element> targets_;
};

} // namespace mizugaki::ast::statement
