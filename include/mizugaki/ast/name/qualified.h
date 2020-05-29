#pragma once

#include "name.h"
#include "simple.h"

namespace mizugaki::ast::name {

/**
 * @brief represents a qualified name.
 */
class qualified final : public name {

    using super = name;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::simple;

    /**
     * @brief creates a new instance.
     * @param qualifier the name qualifier
     * @param last the last name
     * @param region the node region
     */
    explicit qualified(
            ::takatori::util::unique_object_ptr<name> qualifier,
            ::takatori::util::unique_object_ptr<simple> last,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit qualified(qualified const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit qualified(qualified&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] qualified* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] qualified* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns a container of the qualified identifier.
     * @return the qualified identifier
     */
    [[nodiscard]] simple& last() noexcept override;

    /// @copydoc last()
    [[nodiscard]] simple const& last() const noexcept override;

    /// @copydoc last()
    [[nodiscard]] ::takatori::util::unique_object_ptr<simple>& mutable_last() noexcept;

    /**
     * @brief returns the qualifier of this name.
     * @return the qualifier
     */
    [[nodiscard]] name& qualifier() noexcept;

    /// @copydoc qualifier()
    [[nodiscard]] name const& qualifier() const noexcept;

    [[nodiscard]] ::takatori::util::optional_ptr<name> optional_qualifier() noexcept override;
    [[nodiscard]] ::takatori::util::optional_ptr<name const> optional_qualifier() const noexcept override;

    /**
     * @brief returns a container of the qualifier of this name.
     * @return the qualifier
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name>& mutable_qualifier() noexcept;

private:
    ::takatori::util::unique_object_ptr<name> qualifier_;
    ::takatori::util::unique_object_ptr<simple> last_;
};

} // namespace mizugaki::ast::name
