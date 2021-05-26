#pragma once


#include "node_region.h"

namespace mizugaki::ast {

/**
 * @brief an abstract super interface of abstract syntax tree (AST) element.
 */
class element {
public:
    /// @brief the region type of element.
    using region_type = node_region;

    /**
     * @brief returns the region of this element.
     * @return the element region
     */
    [[nodiscard]] constexpr region_type& region() noexcept {
        return region_;
    }

    /// @copydoc region()
    [[nodiscard]] constexpr region_type const& region() const noexcept {
        return region_;
    }

protected:
    /**
     * @brief creates a new instance.
     */
    constexpr element() = default;

    /**
     * @brief creates a new instance.
     * @param region the region of this element
     */
    explicit constexpr element(region_type region) noexcept :
        region_ { region }
    {}
    
    ~element() = default;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    element(element const& other) noexcept = default;

    /**
     * @brief assigns into this.
     * @param other the copy source
     * @return this
     */
    element& operator=(element const& other) noexcept = default;

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    element(element&& other) noexcept = default;

    /**
     * @brief assigns into this.
     * @param other the move source
     * @return this
     */
    element& operator=(element&& other) noexcept = default;

private:
    region_type region_ {};
};

} // namespace mizugaki::ast
