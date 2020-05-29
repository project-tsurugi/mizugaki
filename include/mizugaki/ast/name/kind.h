#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::ast::name {

class simple;
class qualified;

/**
 * @brief represents a kind of name.
 */
enum class kind {
    /**
     * @brief simple name.
     * @see simple
     */
    simple,

    /**
     * @brief qualified name.
     * @see qualified
     */
    qualified,
};

/**
 * @brief provides the implementation type of the type kind.
 * @tparam Kind the type kind
 */
template<kind Kind> struct type_of;

/// @copydoc type_of
template<kind Kind> using type_of_t = typename type_of<Kind>::type;

/// @brief provides implementation type of kind::simple.
template<> struct type_of<kind::simple> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::qualified.
template<> struct type_of<kind::qualified> : ::takatori::util::meta_type<qualified> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(kind value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case kind::simple: return "simple"sv;
        case kind::qualified: return "qualified"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::name
