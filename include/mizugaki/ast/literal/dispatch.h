#pragma once

#include <takatori/util/exception.h>
#include <takatori/util/callback.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/literal/kind.h>
#include <mizugaki/ast/literal/literal.h>
#include <mizugaki/ast/literal/boolean.h>
#include <mizugaki/ast/literal/numeric.h>
#include <mizugaki/ast/literal/string.h> // NOLINT
#include <mizugaki/ast/literal/datetime.h>
#include <mizugaki/ast/literal/interval.h>
#include <mizugaki/ast/literal/special.h>

namespace mizugaki::ast::literal {

/// @cond
namespace impl {

[[noreturn]] inline void unsupported(literal const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported literal kind: "
            << object.node_kind()
            << string_builder::to_string));
}

} // namespace impl
/// @endcond

/**
 * @brief invoke callback function for individual subclasses of the element.
 * @attention You must declare all callback functions for individual subclasses,
 * or declare Callback::operator()(literal&, Args...) as "default" callback function.
 * Each return type of callback function must be identical.
 * @tparam Callback the callback object type
 * @tparam E the element type
 * @tparam Args the callback argument types
 * @param callback the callback object
 * @param object the target element
 * @param args the callback arguments
 * @return the callback result
 */
template<
        class Callback,
        class E,
        class... Args>
inline auto dispatch(Callback&& callback, E&& object, Args&&... args) {
    using ::takatori::util::polymorphic_callback;
    switch (object.node_kind()) {
        case kind::boolean: return polymorphic_callback<type_of_t<kind::boolean>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::exact_numeric: return polymorphic_callback<type_of_t<kind::exact_numeric>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::approximate_numeric: return polymorphic_callback<type_of_t<kind::approximate_numeric>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::character_string: return polymorphic_callback<type_of_t<kind::character_string>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::bit_string: return polymorphic_callback<type_of_t<kind::bit_string>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::hex_string: return polymorphic_callback<type_of_t<kind::hex_string>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::date: return polymorphic_callback<type_of_t<kind::date>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::time: return polymorphic_callback<type_of_t<kind::time>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::timestamp: return polymorphic_callback<type_of_t<kind::timestamp>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::interval: return polymorphic_callback<type_of_t<kind::interval>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::null: return polymorphic_callback<type_of_t<kind::null>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::empty: return polymorphic_callback<type_of_t<kind::empty>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::default_: return polymorphic_callback<type_of_t<kind::default_>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
    }
    impl::unsupported(object);
}

} // namespace mizugaki::ast::literal
