#pragma once

#include <takatori/util/clone_tag.h>

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
    static constexpr node_kind_type tag = node_kind_type::qualified;

    /**
     * @brief creates a new instance.
     * @param qualifier the name qualifier
     * @param last the last name
     * @param region the node region
     */
    explicit qualified(
            std::unique_ptr<name> qualifier,
            std::unique_ptr<simple> last,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param qualifier the name qualifier
     * @param last the last name
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit qualified(
            name&& qualifier,
            simple&& last,
            region_type region = {});

    /**
     * @brief creates a new instance from chain of names.
     * @details this is a DSL-style constructor designed for testing.
     * @tparam Args the chain types, must be simple
     * @param qualifier the name qualifier
     * @param next the next simple name
     * @param args the rest simple names, must be rvalue references
     * @attention this will take copy of arguments
     */
    template<class... Args>
    explicit qualified(name&& qualifier, simple&& next, Args&&... args) :
        qualified {
                qualified {
                        std::move(qualifier),
                        std::move(next),
                },
                std::forward<Args>(args)...,
        }
    {}

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit qualified(::takatori::util::clone_tag_t, qualified const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit qualified(::takatori::util::clone_tag_t, qualified&& other);

    [[nodiscard]] qualified* clone() const& override;
    [[nodiscard]] qualified* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the last name of this.
     * @return the last name
     */
    [[nodiscard]] simple const& last_name() const noexcept override;

    /// @copydoc qualifier()
    [[nodiscard]] ::takatori::util::optional_ptr<name const> optional_qualifier() const noexcept override;

    /**
     * @brief returns the qualifier of this name.
     * @return the qualifier
     */
    [[nodiscard]] std::unique_ptr<name>& qualifier() noexcept;

    /// @copydoc qualifier()
    [[nodiscard]] std::unique_ptr<name> const& qualifier() const noexcept;

    /**
     * @brief returns a container of the qualified identifier.
     * @return the qualified identifier
     */
    [[nodiscard]] std::unique_ptr<simple>& last() noexcept;

    /// @copydoc last()
    [[nodiscard]] std::unique_ptr<simple> const& last() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(qualified const& a, qualified const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(qualified const& a, qualified const& b) noexcept;

protected:
    [[nodiscard]] bool equals(name const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name> qualifier_;
    std::unique_ptr<simple> last_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, qualified const& value);

} // namespace mizugaki::ast::name
