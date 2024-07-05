#pragma once

#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include <takatori/util/assertion.h>

namespace mizugaki::analyzer::details {

enum class find_element_result_kind {
    found,
    absent,
    ambiguous,
};

inline constexpr std::string_view to_string_view(find_element_result_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = find_element_result_kind;
    switch (value) {
        case kind::found: return "found"sv;
        case kind::absent: return "absent"sv;
        case kind::ambiguous: return "ambiguous"sv;
    }
    std::abort();
}

inline std::ostream& operator<<(std::ostream& out, find_element_result_kind value) {
    return out << to_string_view(value);
}

template<class T>
class find_element_result {
public:
    using kind_type = find_element_result_kind;

    static inline find_element_result const ambiguous { kind_type::ambiguous }; // NOLINT

    constexpr find_element_result() = default;

    constexpr find_element_result(std::nullptr_t) : // NOLINT(*-explicit-constructor, *-explicit-conversions)
        find_element_result {}
    {}

    constexpr find_element_result(T& value) : // NOLINT(*-explicit-constructor, *-explicit-conversions)
        kind_ { kind_type::found },
        value_ { std::addressof(value) }
    {}

    find_element_result(::takatori::util::optional_ptr<T> value) : // NOLINT(*-explicit-constructor, *-explicit-conversions)
            kind_ { value.has_value() ? kind_type::found : kind_type::absent },
            value_ { value.get() }
    {}

    [[nodiscard]] constexpr kind_type kind() const noexcept {
        return kind_;
    }

    [[nodiscard]] constexpr bool is_found() const noexcept {
        return kind_ == kind_type::found;
    }

    [[nodiscard]] constexpr bool is_absent() const noexcept {
        return kind_ == kind_type::absent;
    }

    [[nodiscard]] constexpr bool is_ambiguous() const noexcept {
        return kind_ == kind_type::ambiguous;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return is_found();
    }

    [[nodiscard]] T& value() const noexcept {
        BOOST_ASSERT(is_found()); // NOLINT
        return *value_;
    }

    [[nodiscard]] T& operator*() const noexcept {
        return value();
    }

    [[nodiscard]] T* operator->() const noexcept {
        BOOST_ASSERT(is_found()); // NOLINT
        return value_;
    }

private:
    kind_type kind_ { kind_type::absent };
    T* value_ { nullptr };

    constexpr explicit find_element_result(kind_type kind) :
        kind_ { kind }
    {}
};

} // namespace mizugaki::analyzer::details
