#pragma once

#include <memory>
#include <variant>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/type/type.h>
#include <mizugaki/ast/scalar/expression.h>

#include "sequence_option_field_kind.h"

namespace mizugaki::parser::details {

struct sequence_option_field {
    using kind_type = sequence_option_field_kind;
    using value_type = std::variant<
            std::monostate,
            std::unique_ptr<ast::type::type>,
            std::unique_ptr<ast::scalar::expression>,
            std::unique_ptr<ast::name::name>,
            ast::common::regioned<bool>>;

    kind_type kind {};
    value_type value {};
    ast::node_region region {};
};

} // namespace mizugaki::parser::details
