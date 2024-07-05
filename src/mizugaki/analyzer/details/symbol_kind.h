#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::analyzer::details {

enum class symbol_kind {
    column_variable,
    field_variable,
    schema_variable,
    relation_info,
    relation_decl,
    schema_decl,
    catalog_decl,
    table_decl,
    index_decl,
};

/// @brief set of symbol kind.
using symbol_kind_set = ::takatori::util::enum_set<
        symbol_kind,
        symbol_kind::column_variable,
        symbol_kind::index_decl>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(symbol_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = symbol_kind;
    switch (value) {
        case kind::column_variable: return "column_variable"sv;
        case kind::field_variable: return "field_variable"sv;
        case kind::schema_variable: return "schema_variable"sv;
        case kind::relation_info: return "relation"sv;
        case kind::relation_decl: return "relation"sv;
        case kind::schema_decl: return "schema"sv;
        case kind::catalog_decl: return "catalog"sv;
        case kind::table_decl: return "table"sv;
        case kind::index_decl: return "index"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, symbol_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::analyzer::details
