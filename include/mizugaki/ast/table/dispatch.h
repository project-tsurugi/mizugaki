#pragma once

#include <takatori/util/exception.h>
#include <takatori/util/callback.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/table/expression.h>
#include <mizugaki/ast/table/table_reference.h>
#include <mizugaki/ast/table/unnest.h>
#include <mizugaki/ast/table/join.h>
#include <mizugaki/ast/table/subquery.h>

#include <mizugaki/ast/table/join_specification.h>
#include <mizugaki/ast/table/join_condition.h>
#include <mizugaki/ast/table/join_columns.h>

namespace mizugaki::ast::table {

/// @cond
namespace impl {

[[noreturn]] inline void unsupported(expression const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported table expression kind: "
            << object.node_kind()
            << string_builder::to_string));
}

[[noreturn]] inline void unsupported(join_specification const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported join specification kind: "
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
    if constexpr (std::is_base_of_v<expression, std::remove_const_t<std::remove_reference_t<E>>>) { // NOLINT
        switch (object.node_kind()) {
            case table_reference::tag: return polymorphic_callback<table_reference>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case unnest::tag: return polymorphic_callback<unnest>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case join::tag: return polymorphic_callback<join>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case subquery::tag: return polymorphic_callback<subquery>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        }
        impl::unsupported(object);
    } else if constexpr (std::is_base_of_v<join_specification, std::remove_const_t<std::remove_reference_t<E>>>) { // NOLINT
        switch (object.node_kind()) {
            case join_condition::tag: return polymorphic_callback<join_condition>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case join_columns::tag: return polymorphic_callback<join_columns>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        }
        impl::unsupported(object);
    } else {
        static_assert([]{ return false; });
        std::abort();
    }
}

} // namespace mizugaki::ast::table
