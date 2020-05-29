#pragma once

#include <takatori/util/exception.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/literal/kind.h>

namespace mizugaki::ast::literal::utils {

inline void validate_kind(kind_set expected, kind actual) {
    if (!expected.contains(actual)) {
        using ::takatori::util::string_builder;
        using ::takatori::util::throw_exception;
        throw_exception(std::invalid_argument(string_builder {}
                << "unsupported node kind: " << actual << ", "
                << "must be one of " << expected
                << string_builder::to_string));
    }
}

} // namespace mizugaki::ast::literal::utils
