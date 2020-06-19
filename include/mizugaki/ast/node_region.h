#pragma once

#include <algorithm>
#include <functional>
#include <ostream>

#include <takatori/serializer/object_acceptor.h>
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

    /// @brief represents an invalid position.
    static constexpr position_type npos = static_cast<position_type>(-1);

    /// @brief the beginning position (inclusive, 0-origin).
    position_type begin { npos }; // NOLINT(misc-non-private-member-variables-in-classes): for parser generator's convention

    /// @brief the ending position (exclusive, 0-origin).
    position_type end { npos }; // NOLINT(misc-non-private-member-variables-in-classes): for parser generator's convention

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

    /**
     * @brief returns the sub region.
     * @details the sub region must be included into the original region.
     * @param offset offset from the beginning position
     * @param length the sub region length
     * @return the sub region
     */
    [[nodiscard]] constexpr node_region operator()(position_type offset, position_type length = npos) noexcept {
        auto nbegin = begin + offset;
        auto nend = (length == npos) ? end : nbegin + length;
        return {
                std::min({ nbegin, nend, end }),
                std::min({ nend, end }),
        };
    }

    /**
     * @brief returns the beginning position of this region.
     * @return the beginning position (inclusive)
     */
    [[nodiscard]] constexpr position_type first() const noexcept {
        return begin;
    }

    /**
     * @brief returns the ending position of this region.
     * @return the ending position (exclusive)
     */
    [[nodiscard]] constexpr position_type last() const noexcept {
        return end;
    }

    /**
     * @brief returns the size of this region.
     * @return the number of position in this region
     */
    [[nodiscard]] constexpr position_type size() const noexcept {
        return end - begin;
    }
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

/**
 * @brief dumps structure information of the given value into the target acceptor.
 * @param acceptor the target acceptor
 * @param value the target value
 * @return the output
 */
::takatori::serializer::object_acceptor& operator<<(
        ::takatori::serializer::object_acceptor& acceptor,
        node_region value);

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
