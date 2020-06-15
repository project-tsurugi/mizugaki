#pragma once

#include "statement.h"

namespace mizugaki::ast::statement {

/**
 * @brief statement that does nothing.
 */
class empty_statement final : public statement {

    using super = statement;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::empty_statement;

    /**
     * @brief creates a new instance.
     * @param region the node region
     */
    explicit empty_statement(
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit empty_statement(empty_statement const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit empty_statement(empty_statement&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] empty_statement* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] empty_statement* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;
};

} // namespace mizugaki::ast::statement
