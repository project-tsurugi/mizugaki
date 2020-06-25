#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::parser::details {

enum class sequence_option_field_kind {
    type,
    initial_value,
    increment_value,
    min_value,
    max_value,
    owner,
    cycle,
};

using sequence_option_field_kind_set = ::takatori::util::enum_set<
        sequence_option_field_kind,
        sequence_option_field_kind::type,
        sequence_option_field_kind::cycle>;

inline constexpr std::string_view to_string_view(sequence_option_field_kind value) noexcept {
    using kind = sequence_option_field_kind;
    using namespace std::string_view_literals;
    switch (value) {
        case kind::type: return "type"sv;
        case kind::initial_value: return "initial_value"sv;
        case kind::increment_value: return "increment_value"sv;
        case kind::min_value: return "min_value"sv;
        case kind::max_value: return "max_value"sv;
        case kind::owner: return "owner"sv;
        case kind::cycle: return "cycle"sv;
    }
    std::abort();
}

} // namespace mizugaki::parser::details
