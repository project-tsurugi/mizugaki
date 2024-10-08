#pragma once

#include <takatori/util/downcast.h>
#include <takatori/util/fail.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/literal/numeric.h>
#include <mizugaki/ast/name/simple.h>

#include <mizugaki/parser/sql_parser.h>
#include <mizugaki/parser/sql_parser_result.h>

namespace mizugaki::parser::testing {

using namespace ast;

template<class T>
using node_ptr = std::unique_ptr<T>;

inline sql_parser default_parser() {
    sql_parser parser {};
    parser.options().tree_node_limit() = 100;
    parser.options().tree_depth_limit() = 10;
    return parser;
}

template<class T, class U>
inline T const& downcast(U const& u) {
    if (auto const* t = ::takatori::util::downcast<T>(&u)) {
        return *t;
    }
    ::takatori::util::fail();
}

inline std::string diagnostics(sql_parser_result const& result) {
    using ::takatori::util::string_builder;
    if (auto&& d = result.diagnostic()) {
        return string_builder {}
                << d.message()
                << " : "
                << d.document()->contents(0, d.region().first())
                << " >>> "
                << d.document()->contents(d.region().first(), d.region().size())
                << string_builder::to_string;
    }
    return {};
}

inline scalar::literal_expression int_literal(std::string_view str) {
    return scalar::literal_expression {
            literal::numeric {
                    literal::kind::exact_numeric,
                    std::nullopt,
                    common::chars { str },
            },
    };
}

inline scalar::variable_reference v(std::string_view str) {
    return scalar::variable_reference {
            name::simple { str },
    };
}

} // namespace mizugaki::parser::testing
