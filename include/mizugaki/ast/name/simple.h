#pragma once

#include <mizugaki/ast/common/chars.h>

#include "name.h"

namespace mizugaki::ast::name {

/**
 * @brief represents a simple name.
 */
class simple final : public name {

    using super = name;

public:
    /// @brief the identifier type.
    using identifier_type = common::chars;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::simple;

    /**
     * @brief creates a new instance.
     * @param identifier the name identifier
     * @param region the node region
     */
    explicit simple(identifier_type identifier, region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit simple(simple const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit simple(simple&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] simple* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] simple* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns this.
     * @return this
     */
    [[nodiscard]] simple& last() noexcept override;

    /// @copydoc last()
    [[nodiscard]] simple const& last() const noexcept override;

    /**
     * @brief always returns empty.
     * @return empty
     */
    [[nodiscard]] ::takatori::util::optional_ptr<name> optional_qualifier() noexcept override;

    /// @copydoc optional_qualifier()
    [[nodiscard]] ::takatori::util::optional_ptr<name const> optional_qualifier() const noexcept override;

    /**
     * @brief returns the identifier of this name.
     * @return the identifier
     */
    [[nodiscard]] identifier_type& identifier() noexcept;

    /// @copydoc identifier()
    [[nodiscard]] identifier_type const& identifier() const noexcept;

private:
    identifier_type identifier_;
};

} // namespace mizugaki::ast::name
