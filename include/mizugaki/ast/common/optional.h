#pragma once

#include <optional>

#include <takatori/util/object_creator.h>
#include <takatori/util/clonable.h>

namespace mizugaki::ast::common {

/**
 * @brief take a clone of the optional value.
 * @tparam T the element type
 * @param element the target value
 * @param creator the object creator
 * @return the created clone
 */
template<class T>
[[nodiscard]] std::optional<T> clone_optional(
        std::optional<T> const& element,
        ::takatori::util::object_creator creator) {
    if (!element) {
        return {};
    }
    return std::optional<T> { std::in_place, *element, creator };
}

/// @copydoc clone_optional()
template<class T>
[[nodiscard]] std::optional<T> clone_optional(
        std::optional<T>&& element,
        ::takatori::util::object_creator creator) {
    if (!element) {
        return {};
    }
    return std::optional<T> { std::in_place, std::move(*element), creator };
}

} // namespace mizugaki::ast::common
