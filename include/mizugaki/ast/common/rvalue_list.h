#pragma once

#include <initializer_list>

#include <takatori/util/clonable.h>
#include <takatori/util/rvalue_reference_wrapper.h>

#include "vector.h"

namespace mizugaki::ast::common {

/**
 * @brief initializer list for non-copyable elements.
 * @tparam T the element type
 */
template<class T>
using rvalue_list = std::initializer_list<::takatori::util::rvalue_reference_wrapper<T>>;

/**
 * @brief converts rvalue_list to vector.
 * @tparam T the element type
 * @param elements the source elements
 * @return the created vector
 */
template<class T>
inline std::vector<std::unique_ptr<T>> to_vector(rvalue_list<T> elements) {
    std::vector<std::unique_ptr<T>> results {};
    results.reserve(elements.size());
    for (auto&& e : elements) {
        results.emplace_back(::takatori::util::clone_unique(e));
    }
    return results;
}

} // namespace mizugaki::ast::common
