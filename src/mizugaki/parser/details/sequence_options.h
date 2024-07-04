#pragma once

#include <memory>
#include <optional>

#include <takatori/util/fail.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/type/type.h>
#include <mizugaki/ast/scalar/expression.h>

#include <mizugaki/parser/sql_driver.h>

#include "sequence_option_field.h"

namespace mizugaki::parser::details {

struct sequence_options {
    std::unique_ptr<ast::type::type> type {};
    std::unique_ptr<ast::scalar::expression> initial_value {};
    std::unique_ptr<ast::scalar::expression> increment_value {};
    std::unique_ptr<ast::scalar::expression> min_value {};
    std::unique_ptr<ast::scalar::expression> max_value {};
    std::unique_ptr<ast::name::name> owner {};
    std::optional<ast::common::regioned<bool>> cycle {};

    sequence_option_field_kind_set saw {};
};

inline bool set(sql_driver& driver, sequence_options& options, sequence_option_field&& field) {
    if (options.saw.contains(field.kind)) {
        using ::takatori::util::string_builder;
        driver.error(
                sql_parser_code::syntax_error,
                field.region,
                string_builder {}
                    << "duplicate declaration of " << to_string_view(field.kind)
                    << string_builder::to_string);
        return false;
    }
    options.saw.insert(field.kind);
    using kind = sequence_option_field_kind;
    switch (field.kind) {
        case kind::type:
            options.type = std::get<std::unique_ptr<ast::type::type>>(std::move(field.value));
            return true;
        case kind::initial_value:
            options.initial_value = std::get<std::unique_ptr<ast::scalar::expression>>(std::move(field.value));
            return true;
        case kind::increment_value:
            options.increment_value = std::get<std::unique_ptr<ast::scalar::expression>>(std::move(field.value));
            return true;
        case kind::min_value:
            options.min_value = std::get<std::unique_ptr<ast::scalar::expression>>(std::move(field.value));
            return true;
        case kind::max_value:
            options.max_value = std::get<std::unique_ptr<ast::scalar::expression>>(std::move(field.value));
            return true;
        case kind::owner:
            options.owner = std::get<std::unique_ptr<ast::name::name>>(std::move(field.value));
            return true;
        case kind::cycle:
            options.cycle = std::get<ast::common::regioned<bool>>(field.value);
            return true;
    }
    ::takatori::util::fail();
}

} // namespace mizugaki::parser::details
