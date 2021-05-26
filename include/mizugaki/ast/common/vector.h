#pragma once

#include <vector>

#include <takatori/util/clonable.h>
#include <takatori/util/clone_tag.h>

namespace mizugaki::ast::common {

/**
 * @brief take a clone of th vector.
 * @tparam T the element type
 * @param elements the target vector
 * @return the created clone
 */
template<class T>
[[nodiscard]] std::vector<std::unique_ptr<T>> clone_vector(std::vector<std::unique_ptr<T>> const& elements) {
    std::vector<std::unique_ptr<T>> results {};
    results.reserve(elements.size());
    for (auto&& e : elements) {
        results.emplace_back(::takatori::util::clone_unique(e));
    }
    return results;
}

/// @copydoc clone_vector()
template<class T>
[[nodiscard]] std::vector<std::unique_ptr<T>> clone_vector(std::vector<std::unique_ptr<T>>&& elements) {
        for (auto&& e : elements) {
            e = ::takatori::util::clone_unique(std::move(e));
        }
        return std::move(elements);
}

/// @copydoc clone_vector()
template<class T>
[[nodiscard]] std::enable_if_t<
        ::takatori::util::is_clone_constructible_v<T>,
        std::vector<T>>
clone_vector(std::vector<T> const& elements) {
    std::vector<T> results {};
    results.reserve(elements.size());
    for (auto&& e : elements) {
        results.emplace_back(::takatori::util::clone_tag, e);
    }
    return results;
}

/// @copydoc clone_vector()
template<class T>
[[nodiscard]] std::enable_if_t<
        ::takatori::util::is_clone_constructible_v<T>,
        std::vector<T>>
clone_vector(std::vector<T>&& elements) {
    std::vector<T> results {};
    results.reserve(elements.size());
    for (auto&& e : elements) {
        results.emplace_back(::takatori::util::clone_tag, std::move(e));
    }
    return results;
}

} // namespace mizugaki::ast::common
