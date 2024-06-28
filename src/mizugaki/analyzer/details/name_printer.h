#pragma once

#include <ostream>

#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/name/qualified.h>

namespace mizugaki::analyzer::details {

class name_printer_t {
public:
    explicit name_printer_t() = default;

    void print(std::ostream& out, ast::name::name const& value) {
        if (auto q = value.optional_qualifier()) {
            print(out, *q);
            out << ".";
        }
        out << value.last_identifier();
    }

    class waiting_for_name {
    public:
        explicit constexpr waiting_for_name(std::ostream& out) noexcept : out_ { out } {}
        std::ostream& operator<<(ast::name::name const& value) {
            name_printer_t {}.print(out_, value);
            return out_;
        }
    private:
        std::ostream& out_;
    };

    friend waiting_for_name operator<<(std::ostream& out, name_printer_t) noexcept {
        return waiting_for_name { out };
    }
};

constexpr name_printer_t name_printer {};

} // namespace mizugaki::analyzer::details
