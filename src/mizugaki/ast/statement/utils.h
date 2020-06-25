#pragma once

#include <takatori/util/exception.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/statement/kind.h>

namespace mizugaki::ast::statement::utils {

template<class EnumSet>
inline void validate_kind(EnumSet expected, typename EnumSet::value_type actual) {
    if (!expected.contains(actual)) {
        using ::takatori::util::string_builder;
        using ::takatori::util::throw_exception;
        throw_exception(std::invalid_argument(string_builder {}
                << "unsupported node kind: " << actual << ", "
                << "must be one of " << expected
                << string_builder::to_string));
    }
}

} // namespace mizugaki::ast::statement::utils
