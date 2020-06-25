#pragma once

#include <takatori/util/exception.h>
#include <takatori/util/callback.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/scalar/expression.h>
#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/host_parameter_reference.h>
#include <mizugaki/ast/scalar/field_reference.h>
#include <mizugaki/ast/scalar/case_expression.h>
#include <mizugaki/ast/scalar/cast_expression.h>
#include <mizugaki/ast/scalar/unary_expression.h>
#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/extract_expression.h>
#include <mizugaki/ast/scalar/trim_expression.h>
#include <mizugaki/ast/scalar/value_constructor.h>
#include <mizugaki/ast/scalar/subquery.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/quantified_comparison_predicate.h>
#include <mizugaki/ast/scalar/between_predicate.h>
#include <mizugaki/ast/scalar/in_predicate.h>
#include <mizugaki/ast/scalar/pattern_match_predicate.h>
#include <mizugaki/ast/scalar/table_predicate.h>
#include <mizugaki/ast/scalar/function_invocation.h>
#include <mizugaki/ast/scalar/builtin_function_invocation.h>
#include <mizugaki/ast/scalar/builtin_set_function_invocation.h>
#include <mizugaki/ast/scalar/new_invocation.h>
#include <mizugaki/ast/scalar/method_invocation.h>
#include <mizugaki/ast/scalar/static_method_invocation.h>
#include <mizugaki/ast/scalar/current_of_cursor.h>

namespace mizugaki::ast::scalar {

/// @cond
namespace impl {

[[noreturn]] inline void unsupported(expression const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported scalar expression kind: "
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
        case literal_expression::tag: return polymorphic_callback<literal_expression>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case variable_reference::tag: return polymorphic_callback<variable_reference>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case host_parameter_reference::tag: return polymorphic_callback<host_parameter_reference>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case field_reference::tag: return polymorphic_callback<field_reference>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case case_expression::tag: return polymorphic_callback<case_expression>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case cast_expression::tag: return polymorphic_callback<cast_expression>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case unary_expression::tag: return polymorphic_callback<unary_expression>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case binary_expression::tag: return polymorphic_callback<binary_expression>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case extract_expression::tag: return polymorphic_callback<extract_expression>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case trim_expression::tag: return polymorphic_callback<trim_expression>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case value_constructor::tag: return polymorphic_callback<value_constructor>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case subquery::tag: return polymorphic_callback<subquery>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case comparison_predicate::tag: return polymorphic_callback<comparison_predicate>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case quantified_comparison_predicate::tag: return polymorphic_callback<quantified_comparison_predicate>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case between_predicate::tag: return polymorphic_callback<between_predicate>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case in_predicate::tag: return polymorphic_callback<in_predicate>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case pattern_match_predicate::tag: return polymorphic_callback<pattern_match_predicate>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case table_predicate::tag: return polymorphic_callback<table_predicate>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case function_invocation::tag: return polymorphic_callback<function_invocation>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case builtin_function_invocation::tag: return polymorphic_callback<builtin_function_invocation>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case builtin_set_function_invocation::tag: return polymorphic_callback<builtin_set_function_invocation>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case new_invocation::tag: return polymorphic_callback<new_invocation>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case method_invocation::tag: return polymorphic_callback<method_invocation>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case static_method_invocation::tag: return polymorphic_callback<static_method_invocation>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        case current_of_cursor::tag: return polymorphic_callback<current_of_cursor>(std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
    }
    impl::unsupported(object);
}

} // namespace mizugaki::ast::scalar
