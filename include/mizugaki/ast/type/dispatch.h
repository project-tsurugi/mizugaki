#pragma once

#include <takatori/util/exception.h>
#include <takatori/util/callback.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/type/type.h>
#include <mizugaki/ast/type/simple.h>
#include <mizugaki/ast/type/character_string.h>
#include <mizugaki/ast/type/bit_string.h>
#include <mizugaki/ast/type/octet_string.h>
#include <mizugaki/ast/type/decimal.h>
#include <mizugaki/ast/type/binary_numeric.h>
#include <mizugaki/ast/type/datetime.h>
#include <mizugaki/ast/type/interval.h>
#include <mizugaki/ast/type/row.h>
#include <mizugaki/ast/type/user_defined.h>
#include <mizugaki/ast/type/collection.h>

namespace mizugaki::ast::type {

/// @cond
namespace impl {

[[noreturn]] inline void unsupported(type const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported type kind: "
            << object.node_kind()
            << string_builder::to_string));
}

} // namespace impl
/// @endcond

/**
 * @brief invoke callback function for individual subclasses of the element.
 * @attention You must declare all callback functions for individual subclasses,
 * or declare Callback::operator()(E&&, Args...) as "default" callback function.
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
        case kind::unknown: return polymorphic_callback<type_of_t<kind::unknown>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::character: return polymorphic_callback<type_of_t<kind::character>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::character_varying: return polymorphic_callback<type_of_t<kind::character_varying>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::character_large_object: return polymorphic_callback<type_of_t<kind::character_large_object>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::bit: return polymorphic_callback<type_of_t<kind::bit>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::bit_varying: return polymorphic_callback<type_of_t<kind::bit_varying>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::octet: return polymorphic_callback<type_of_t<kind::octet>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::octet_varying: return polymorphic_callback<type_of_t<kind::octet_varying>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::binary_large_object: return polymorphic_callback<type_of_t<kind::binary_large_object>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::numeric: return polymorphic_callback<type_of_t<kind::numeric>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::decimal: return polymorphic_callback<type_of_t<kind::decimal>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::tiny_integer: return polymorphic_callback<type_of_t<kind::tiny_integer>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::small_integer: return polymorphic_callback<type_of_t<kind::small_integer>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::integer: return polymorphic_callback<type_of_t<kind::integer>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::big_integer: return polymorphic_callback<type_of_t<kind::big_integer>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::float_: return polymorphic_callback<type_of_t<kind::float_>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::real: return polymorphic_callback<type_of_t<kind::real>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::double_precision: return polymorphic_callback<type_of_t<kind::double_precision>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::binary_integer: return polymorphic_callback<type_of_t<kind::binary_integer>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::binary_float: return polymorphic_callback<type_of_t<kind::binary_float>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::boolean: return polymorphic_callback<type_of_t<kind::boolean>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::date: return polymorphic_callback<type_of_t<kind::date>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::time: return polymorphic_callback<type_of_t<kind::time>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::timestamp: return polymorphic_callback<type_of_t<kind::timestamp>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::interval: return polymorphic_callback<type_of_t<kind::interval>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::row: return polymorphic_callback<type_of_t<kind::row>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::user_defined: return polymorphic_callback<type_of_t<kind::user_defined>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case kind::collection: return polymorphic_callback<type_of_t<kind::collection>>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
    }
    impl::unsupported(object);
}

} // namespace mizugaki::ast::type
