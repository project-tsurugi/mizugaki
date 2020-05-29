#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/scalar/expression.h>
#include <mizugaki/ast/query/expression.h>

#include "expression.h"
#include "join_type.h"

namespace mizugaki::ast::table {

/**
 * @brief processes join operation.
 * @note `7.7 <joined table>`
 */
class join final : public expression {

    using super = expression;

public:
    /// @brief the join type.
    using operator_kind_type = join_type;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::join;

    /**
     * @brief creates a new instance.
     * @param operator_kind the join type
     * @param left the left operand
     * @param right the right operand
     * @param condition the join condition (`ON`)
     * @param named_columns the join columns (`USING`)
     * @param region the node region
     * @note `condition` and `named_columns` are exclusive, and they can be present only if
     *      `operator_kind` is ony of `inner`, `left_outer`, `right_outer`, or `full_outer`.
     */
    explicit join(
            operator_kind_type operator_kind,
            ::takatori::util::unique_object_ptr<table::expression> left,
            ::takatori::util::unique_object_ptr<table::expression> right,
            ::takatori::util::unique_object_ptr<scalar::expression> condition = {},
            common::vector<::takatori::util::unique_object_ptr<name::simple>> named_columns = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit join(join const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit join(join&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] join* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] join* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the join type.
     * @return the join type
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the left operand.
     * @return the left operand
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<table::expression>& left() noexcept;

    /// @copydoc left()
    [[nodiscard]] ::takatori::util::unique_object_ptr<table::expression> const& left() const noexcept;

    /**
     * @brief returns the right operand.
     * @return the right operand
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<table::expression>& right() noexcept;

    /// @copydoc right()
    [[nodiscard]] ::takatori::util::unique_object_ptr<table::expression> const& right() const noexcept;

    /**
     * @brief returns the join condition (`ON`).
     * @return the join condition expression
     * @return empty if it is not declared
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& condition() noexcept;

    /// @copydoc condition()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& condition() const noexcept;

    /**
     * @brief returns the join columns (`USING`).
     * @return the join columns
     * @return empty if they are not declared
     */
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>>& named_columns() noexcept;

    /// @copydoc named_columns()
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>> const& named_columns() const noexcept;

private:
    operator_kind_type operator_kind_;
    ::takatori::util::unique_object_ptr<table::expression> left_;
    ::takatori::util::unique_object_ptr<table::expression> right_;
    ::takatori::util::unique_object_ptr<scalar::expression> condition_;
    common::vector<::takatori::util::unique_object_ptr<name::simple>> named_columns_;
};

} // namespace mizugaki::ast::table
