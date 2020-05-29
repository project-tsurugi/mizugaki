#pragma once

#include <vector>

#include <takatori/util/object_creator.h>
#include <takatori/util/clonable.h>

namespace mizugaki::ast::common {

/**
 * @brief element vector type.
 * @tparam T the element type
 */
template<class T>
using vector = std::vector<T, ::takatori::util::object_allocator<T>>;

/**
 * @brief take a clone of th vector.
 * @tparam T the element type
 * @param elements the target vector
 * @param creator the object creator
 * @return the created clone
 */
template<class T>
[[nodiscard]] vector<::takatori::util::unique_object_ptr<T>> clone_vector(
        vector<::takatori::util::unique_object_ptr<T>> const& elements,
        ::takatori::util::object_creator creator) {
    vector<::takatori::util::unique_object_ptr<T>> results { creator.allocator() };
    results.reserve(elements.size());
    for (auto&& e : elements) {
        results.emplace_back(::takatori::util::clone_unique(e, creator));
    }
    return results;
}

/// @copydoc clone_vector()
template<class T>
[[nodiscard]] vector<::takatori::util::unique_object_ptr<T>> clone_vector(
        vector<::takatori::util::unique_object_ptr<T>>&& elements,
        ::takatori::util::object_creator creator) {
    if (creator.is_compatible(elements.get_allocator())) {
        for (auto&& e : elements) {
            e = ::takatori::util::clone_unique(std::move(e), creator);
        }
        return std::move(elements);
    }
    vector<::takatori::util::unique_object_ptr<T>> results { creator.allocator() };
    results.reserve(elements.size());
    for (auto&& e : elements) {
        results.emplace_back(::takatori::util::clone_unique(std::move(e), creator));
    }
    return results;
}

/// @copydoc clone_vector()
template<class T>
[[nodiscard]] std::enable_if_t<
        std::is_constructible_v<T, T const&, ::takatori::util::object_creator>,
        vector<T>>
clone_vector(vector<T> const& elements, ::takatori::util::object_creator creator) {
    vector<T> results { creator.allocator() };
    results.reserve(elements.size());
    for (auto&& e : elements) {
        results.emplace_back(e, creator);
    }
    return results;
}

/// @copydoc clone_vector()
template<class T>
[[nodiscard]] std::enable_if_t<
        std::is_constructible_v<T, T&&, ::takatori::util::object_creator>,
        vector<T>>
clone_vector(vector<T>&& elements, ::takatori::util::object_creator creator) {
    vector<T> results { creator.allocator() };
    results.reserve(elements.size());
    for (auto&& e : elements) {
        results.emplace_back(std::move(e), creator);
    }
    return results;
}

} // namespace mizugaki::ast::common
