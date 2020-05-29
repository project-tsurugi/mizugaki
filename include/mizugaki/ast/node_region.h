#pragma once

#include <ostream>
#include <functional>

#include <takatori/util/detect.h>

#include <cstddef>

namespace mizugaki::ast {

/**
 * @brief represents a node region in the source document.
 * @details This only tracks the absolute byte position in the target document.
 */
struct node_region {

    /// @brief the position type.
    using position_type = std::size_t;

    /// @brief the beginning position (inclusive, 0-origin).
    position_type begin {}; // NOLINT(misc-non-private-member-variables-in-classes): for parser generator's convention

    /// @brief the ending position (exclusive, 0-origin).
    position_type end {}; // NOLINT(misc-non-private-member-variables-in-classes): for parser generator's convention

    /**
     * @brief creates a new instance with empty region.
     */
    constexpr node_region() = default;

    /**
     * @brief creates a new instance.
     * @param first the beginning position (inclusive, 0-origin)
     * @param last the ending position (exclusive, 0-origin)
     */
    constexpr node_region(position_type first, position_type last) noexcept :
        begin { first },
        end { last }
    {}
};

/**
 * @brief returns the union of the two regions.
 * @details The regions need not have intersections.
 * @param a the first region
 * @param b the second region
 * @return the union of the regions
 */
[[nodiscard]] node_region operator|(node_region a, node_region b) noexcept;

/**
 * @brief returns whether or not the two regions are equivalent.
 * @param a the first region
 * @param b the second region
 * @return true if the both are equivalent
 * @return false otherwise
 */
[[nodiscard]] bool operator==(node_region a, node_region b) noexcept;

/**
 * @brief returns whether or not the two regions are different.
 * @param a the first region
 * @param b the second region
 * @return true if the both are different
 * @return false otherwise
 */
[[nodiscard]] bool operator!=(node_region a, node_region b) noexcept;

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, node_region value);

/// @cond IMPL_DEFS
namespace impl {

template<class T>
using region_t = decltype(std::declval<T&>().region());

} // namespace impl
/// @endcond

/**
 * @brief whether or not the target type has `node_region& T::region()`.
 * @tparam T the target type
 */
template<class T>
using has_region = std::is_same<
        ::takatori::util::detect_t<impl::region_t, T>,
        std::add_lvalue_reference_t<node_region>>;

/// @copydoc has_region
template<class T>
inline constexpr bool has_region_v = has_region<T>::value;

/**
 * @brief sets a region into the given element.
 * @tparam T the node type
 * @param element the target element
 * @param region the element's region
 * @return the element
 */
template<class T>
inline constexpr std::enable_if_t<
        has_region_v<std::remove_reference_t<T>>,
        T&&>
operator<<(T&& element, node_region region) noexcept {
    element.region() = region;
    return std::forward<T>(element);
}

} // namespace mizugaki::ast

/**
 * @brief provides hash code of mizugaki::ast::node_region.
 */
template<>
struct std::hash<mizugaki::ast::node_region> {
    /**
     * @brief returns hash code of the given object.
     * @param object the target object
     * @return the computed hash code
     */
    [[nodiscard]] std::size_t operator()(mizugaki::ast::node_region object) const noexcept;
};
