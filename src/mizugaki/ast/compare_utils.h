#pragma once

#include <memory>
#include <vector>

#include <takatori/util/downcast.h>

namespace mizugaki::ast {

using ::takatori::util::unsafe_downcast;

template<class T>
[[nodiscard]] bool eq(T const& a, T const& b) noexcept;

namespace impl {

template<class T, class = void>
struct eqer {
    constexpr bool operator()(T const& a, T const& b) const noexcept {
        return a == b;
    }
};

template<class E, class D>
struct eqer<std::unique_ptr<E, D>> {
    bool operator()(std::unique_ptr<E, D> const& a, std::unique_ptr<E, D> const& b) const noexcept {
        if (!a) {
            return !b;
        }
        if (!b) {
            return false;
        }
        return eq(*a, *b);
    }
};

template<class E, class A>
struct eqer<std::vector<E, A>> {
    bool operator()(std::vector<E, A> const& a, std::vector<E, A> const& b) const noexcept {
        if (a.size() != b.size()) {
            return false;
        }
        for (std::size_t i = 0, n = a.size(); i < n; ++i) {
            if (!eq(a[i], b[i])) {
                return false;
            }
        }
        return true;
    }
};

} // namespace impl

template<class T>
bool eq(T const& a, T const& b) noexcept {
    return std::addressof(a) == std::addressof(b) || impl::eqer<T> {}(a, b);
}

} // namespace mizugaki::ast
