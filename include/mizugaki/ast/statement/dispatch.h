#pragma once

#include <takatori/util/exception.h>
#include <takatori/util/callback.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/statement/statement.h>
#include <mizugaki/ast/statement/select_statement.h>
#include <mizugaki/ast/statement/insert_statement.h>
#include <mizugaki/ast/statement/update_statement.h>
#include <mizugaki/ast/statement/delete_statement.h>
#include <mizugaki/ast/statement/table_definition.h>
#include <mizugaki/ast/statement/index_definition.h>
#include <mizugaki/ast/statement/view_definition.h>
#include <mizugaki/ast/statement/sequence_definition.h>
#include <mizugaki/ast/statement/schema_definition.h>
#include <mizugaki/ast/statement/drop_statement.h>
#include <mizugaki/ast/statement/truncate_table_statement.h>
#include <mizugaki/ast/statement/grant_privilege_statement.h>
#include <mizugaki/ast/statement/revoke_privilege_statement.h>
#include <mizugaki/ast/statement/empty_statement.h>

#include <mizugaki/ast/statement/table_element.h>
#include <mizugaki/ast/statement/column_definition.h>
#include <mizugaki/ast/statement/table_constraint_definition.h>

#include <mizugaki/ast/statement/constraint.h>
#include <mizugaki/ast/statement/simple_constraint.h>
#include <mizugaki/ast/statement/expression_constraint.h>
#include <mizugaki/ast/statement/key_constraint.h>
#include <mizugaki/ast/statement/referential_constraint.h>
#include <mizugaki/ast/statement/identity_constraint.h>

namespace mizugaki::ast::statement {

/// @cond
namespace impl {

[[noreturn]] inline void unsupported(statement const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported statement kind: "
            << object.node_kind()
            << string_builder::to_string));
}

[[noreturn]] inline void unsupported(table_element const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported table element kind: "
            << object.node_kind()
            << string_builder::to_string));
}

[[noreturn]] inline void unsupported(constraint const& object) {
    using ::takatori::util::string_builder;
    using ::takatori::util::throw_exception;
    throw_exception(std::invalid_argument(string_builder {}
            << "unsupported constraint kind: "
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
    if constexpr (std::is_base_of_v<statement, std::remove_const_t<std::remove_reference_t<E>>>) { // NOLINT
        switch (object.node_kind()) {
            case select_statement::tag:
                return polymorphic_callback<select_statement>(std::forward<Callback>(callback), std::forward<E>(object),
                        std::forward<Args>(args)...);
            case insert_statement::tag:
                return polymorphic_callback<insert_statement>(std::forward<Callback>(callback), std::forward<E>(object),
                        std::forward<Args>(args)...);
            case update_statement::tag:
                return polymorphic_callback<update_statement>(std::forward<Callback>(callback), std::forward<E>(object),
                        std::forward<Args>(args)...);
            case delete_statement::tag:
                return polymorphic_callback<delete_statement>(std::forward<Callback>(callback), std::forward<E>(object),
                        std::forward<Args>(args)...);

            case table_definition::tag:
                return polymorphic_callback<table_definition>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case view_definition::tag:
                return polymorphic_callback<view_definition>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case index_definition::tag:
                return polymorphic_callback<index_definition>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case sequence_definition::tag:
                return polymorphic_callback<sequence_definition>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case schema_definition::tag:
                return polymorphic_callback<schema_definition>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);

            case kind::drop_table_statement:
            case kind::drop_index_statement:
            case kind::drop_view_statement:
            case kind::drop_sequence_statement:
            case kind::drop_schema_statement:
                return polymorphic_callback<drop_statement>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);

            case kind::truncate_table_statement:
                return polymorphic_callback<truncate_table_statement>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);

            case kind::grant_privilege_statement:
                return polymorphic_callback<grant_privilege_statement>(std::forward<Callback>(callback),
                        std::forward<E>(object), std::forward<Args>(args)...);
            case kind::revoke_privilege_statement:
                return polymorphic_callback<revoke_privilege_statement>(std::forward<Callback>(callback),
                        std::forward<E>(object), std::forward<Args>(args)...);

            case empty_statement::tag:
                return polymorphic_callback<empty_statement>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        }
        impl::unsupported(object);
    } else if constexpr (std::is_base_of_v<table_element, std::remove_const_t<std::remove_reference_t<E>>>) { // NOLINT
        switch (object.node_kind()) {
            case column_definition::tag:
                return polymorphic_callback<column_definition>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case table_constraint_definition::tag:
                return polymorphic_callback<table_constraint_definition>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        }
        impl::unsupported(object);
    } else if constexpr (std::is_base_of_v<constraint, std::remove_const_t<std::remove_reference_t<E>>>) { // NOLINT
        switch (object.node_kind()) {
            case constraint_kind::check:
                return polymorphic_callback<expression_constraint>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case constraint_kind::unique:
                return polymorphic_callback<key_constraint>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case constraint_kind::primary_key:
                return polymorphic_callback<key_constraint>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case referential_constraint::tag:
                return polymorphic_callback<referential_constraint>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case constraint_kind::null:
                return polymorphic_callback<simple_constraint>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case constraint_kind::not_null:
                return polymorphic_callback<simple_constraint>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case constraint_kind::default_clause:
                return polymorphic_callback<expression_constraint>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case constraint_kind::generation_clause:
                return polymorphic_callback<expression_constraint>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
            case identity_constraint::tag:
                return polymorphic_callback<identity_constraint>(
                        std::forward<Callback>(callback), std::forward<E>(object), std::forward<Args>(args)...);
        }
        impl::unsupported(object);
    } else {
        static_assert([]{ return false; });
        std::abort();
    }
}

} // namespace mizugaki::ast::statement
