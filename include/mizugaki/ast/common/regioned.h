#pragma once

#include <ostream>
#include <functional>

#include <takatori/util/clone_tag.h>
#include <takatori/util/detect.h>

#include <mizugaki/ast/node_region.h>

namespace mizugaki::ast::common {

/**
 * @brief a value with region.
 * @tparam T the value type
 */
template<class T>
class regioned {
public:
    /// @brief the value type.
    using value_type = T;

    /// @brief the region type.
    using region_type = node_region;

    /**
     * @brief creates a new default instance.
     * @details This constructor is available only if the value type is default constructible.
     */
    constexpr regioned() = default;

    /**
     * @brief creates a new instance.
     * @param value the target value
     * @param region the region of the value
     */
    constexpr regioned( // NOLINT
            value_type value,
            region_type region = {}) noexcept(std::is_nothrow_move_constructible_v<value_type>)
        : value_ { std::move(value) }
        , region_ { region }
    {}

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit regioned(::takatori::util::clone_tag_t, regioned const& other) :
        value_ { other.value_ },
        region_ { other.region() }
    {}

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit regioned(::takatori::util::clone_tag_t, regioned&& other) :
        value_ { std::move(other.value_) },
        region_ { other.region() }
    {}

    /**
     * @brief returns the holding value.
     * @return the value
     */
    [[nodiscard]] constexpr value_type& value() noexcept {
        return value_;
    }

    /// @copydoc value()
    [[nodiscard]] constexpr value_type const& value() const noexcept {
        return value_;
    }

    /// @copydoc value()
    [[nodiscard]] constexpr value_type& operator*() & noexcept {
        return value();
    }

    /// @copydoc value()
    [[nodiscard]] constexpr value_type const& operator*() const & noexcept {
        return value();
    }

    /// @copydoc value()
    [[nodiscard]] constexpr value_type&& operator*() && noexcept {
        return std::move(value());
    }

    /// @copydoc value()
    [[nodiscard]] explicit constexpr operator value_type&() & noexcept {
        return value();
    }

    /// @copydoc value()
    [[nodiscard]] explicit constexpr operator value_type const&() const& noexcept {
        return value();
    }

    /// @copydoc value()
    [[nodiscard]] explicit constexpr operator value_type&&() && noexcept {
        return std::move(value());
    }

    /**
     * @brief returns the region of this value.
     * @return the region
     */
    [[nodiscard]] constexpr region_type& region() noexcept {
        return region_;
    }

    /// @copydoc region()
    [[nodiscard]] constexpr region_type const& region() const noexcept {
        return region_;
    }

private:
    value_type value_ {};
    region_type region_ {};
};

/**
 * @brief returns whether or not the two values are equivalent ignoring their regions.
 * @tparam T the first value type
 * @tparam U the second value type
 * @param a the first value
 * @param b the second value
 * @return true if the both are equivalent
 * @return false otherwise
 */
template<class T, class U>
[[nodiscard]] constexpr bool operator==(regioned<T> const& a, regioned<U> const& b) noexcept {
    return a.value() == b.value();
}

/**
 * @brief returns whether or not the two values are different ignoring their regions.
 * @tparam T the first value type
 * @tparam U the second value type
 * @param a the first value
 * @param b the second value
 * @return true if the both are different
 * @return false otherwise
 */
template<class T, class U>
[[nodiscard]] constexpr bool operator!=(regioned<T> const& a, regioned<U> const& b) noexcept {
    return !(a == b);
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
template<class T>
std::ostream& operator<<(std::ostream& out, regioned<T> const& value) {
    return out << value.value();
}

} // namespace mizugaki::ast::common

/**
 * @brief provides hash code of mizugaki::ast::regioned.
 * @tparam the value type
 */
template<class T>
struct std::hash<::mizugaki::ast::common::regioned<T>> {
    /**
     * @brief returns hash code of the given object.
     * @param object the target object
     * @return the computed hash code
     */
    [[nodiscard]] constexpr std::size_t operator()(::mizugaki::ast::common::regioned<T> const& object) const noexcept {
        return std::hash<T> {}(object.value());
    }
};
