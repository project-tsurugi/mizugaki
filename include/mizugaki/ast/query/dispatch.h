#pragma once

#include <takatori/util/exception.h>
#include <takatori/util/callback.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/query/expression.h>
#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_reference.h>
#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/query/binary_expression.h>
#include <mizugaki/ast/query/with_expression.h>

#include <mizugaki/ast/query/grouping_element.h>
#include <mizugaki/ast/query/grouping_column.h>

#include <mizugaki/ast/query/select_element.h>
#include <mizugaki/ast/query/select_column.h>
#include <mizugaki/ast/query/select_asterisk.h>

namespace mizugaki::ast::query {

/// @cond
namespace impl {

[[noreturn]] inline void unsupported(expression const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported query expression kind: "
            << object.node_kind()
            << string_builder::to_string));
}

[[noreturn]] inline void unsupported(grouping_element const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported grouping element kind: "
            << object.node_kind()
            << string_builder::to_string));
}

[[noreturn]] inline void unsupported(select_element const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported select element kind: "
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
            case query::tag: return polymorphic_callback<query>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case table_reference::tag: return polymorphic_callback<table_reference>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case table_value_constructor::tag: return polymorphic_callback<table_value_constructor>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case binary_expression::tag: return polymorphic_callback<binary_expression>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case with_expression::tag: return polymorphic_callback<with_expression>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        }
        impl::unsupported(object);
    } else if constexpr (std::is_base_of_v<grouping_element, std::remove_const_t<std::remove_reference_t<E>>>) { // NOLINT
        switch (object.node_kind()) {
            case grouping_column::tag: return polymorphic_callback<grouping_column>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        }
        impl::unsupported(object);
    } else if constexpr (std::is_base_of_v<select_element, std::remove_const_t<std::remove_reference_t<E>>>) { // NOLINT
        switch (object.node_kind()) {
            case select_column::tag: return polymorphic_callback<select_column>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case select_asterisk::tag: return polymorphic_callback<select_asterisk>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        }
        impl::unsupported(object);
    } else {
        static_assert([]{ return false; });
        std::abort();
    }
}

} // namespace mizugaki::ast::query
