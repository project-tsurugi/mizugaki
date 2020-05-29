#pragma once

#include <string>

#include <takatori/util/object_creator.h>

namespace mizugaki::ast::common {

/// @brief character string type.
using chars = std::basic_string<
        char,
        std::char_traits<char>,
        ::takatori::util::object_allocator<char>>;

} // namespace mizugaki::ast::common
