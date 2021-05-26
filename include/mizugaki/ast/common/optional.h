#pragma once

#include <optional>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::common {

/**
 * @brief take a clone of the optional value.
 * @tparam T the element type
 * @param element the target value
 * @return the created clone
 */
template<class T>
[[nodiscard]] std::optional<T> clone_optional(std::optional<T> const& element) {
    if (!element) {
        return std::nullopt;
    }
    return std::optional<T> { std::in_place, *element };
}

/// @copydoc clone_optional()
template<class T>
[[nodiscard]] std::optional<T> clone_optional(std::optional<T>&& element) {
    if (!element) {
        return std::nullopt;
    }
    return std::optional<T> { std::in_place, std::move(*element) };
}

} // namespace mizugaki::ast::common
