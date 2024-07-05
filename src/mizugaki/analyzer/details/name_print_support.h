#pragma once

#include <ostream>
#include <type_traits>

#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/name/qualified.h>

namespace mizugaki::analyzer::details {

template<class T, class = void>
class print_support {};

template<class T>
print_support(T const&) -> print_support<T>;

template<class T>
class print_support<
        T,
        std::enable_if_t<std::is_base_of_v<ast::name::name, T>>> {
public:
    explicit constexpr print_support(ast::name::name const& object) noexcept :
        value_ { object }
    {}

    void print(std::ostream& out) const {
        print0(out, value_);
    }

private:
    ast::name::name const& value_;

    static void print0(std::ostream& out, ast::name::name const& value) {
        if (auto q = value.optional_qualifier()) {
            print0(out, *q);
            out << ".";
        }
        out << value.last_identifier();
    }
};

template<class T>
inline std::ostream& operator<<(std::ostream& out, print_support<T> const& object) {
    object.print(out);
    return out;
}

} // namespace mizugaki::analyzer::details
