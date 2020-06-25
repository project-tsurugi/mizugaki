#pragma once

#include <takatori/util/exception.h>
#include <takatori/util/callback.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/name/kind.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/name/qualified.h>

namespace mizugaki::ast::name {

/// @cond
namespace impl {

[[noreturn]] inline void unsupported(name const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported name kind: "
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
        case simple::tag: return polymorphic_callback<simple>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case qualified::tag: return polymorphic_callback<qualified>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
    }
    impl::unsupported(object);
}

} // namespace mizugaki::ast::name
