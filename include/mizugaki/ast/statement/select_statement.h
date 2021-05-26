#pragma once

#include <optional>

#include <takatori/util/clone_tag.h>

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
            std::unique_ptr<query::expression> expression,
            std::vector<target_element> targets = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param expression the query expression
     * @param targets the optional target outputs
     * @param region the node region
     * @attention this may take copy of elements
     */
    explicit select_statement(
            query::expression&& expression,
            std::initializer_list<target_element> targets = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit select_statement(::takatori::util::clone_tag_t, select_statement const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit select_statement(::takatori::util::clone_tag_t, select_statement&& other);

    [[nodiscard]] select_statement* clone() const& override;
    [[nodiscard]] select_statement* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the query expression.
     * @return the query expression
     */
    [[nodiscard]] std::unique_ptr<query::expression>& expression() noexcept;

    /// @brief expression()
    [[nodiscard]] std::unique_ptr<query::expression> const& expression() const noexcept;

    /**
     * @brief returns the optional output targets.
     * @return the output targets to store the result of expression()
     * @return empty if the output targets are not declared
     * @note `14.5 <select statement: single row>`
     */
    [[nodiscard]] std::vector<target_element>& targets() noexcept;

    /// @copydoc targets()
    [[nodiscard]] std::vector<target_element> const& targets() const noexcept;
    
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
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<query::expression> expression_;
    std::vector<target_element> targets_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, select_statement const& value);

} // namespace mizugaki::ast::statement
