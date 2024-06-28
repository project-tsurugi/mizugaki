#include <mizugaki/analyzer/details/column_info.h>

#include <takatori/util/print_support.h>
#include <takatori/util/optional_print_support.h>

namespace mizugaki::analyzer::details {

using ::takatori::util::optional_ptr;

namespace descriptor = ::takatori::descriptor;
namespace storage = ::yugawara::storage;

column_info::column_info(
        optional_ptr<storage::column const> declaration,
        descriptor::variable variable,
        std::optional<name_type> identifier,
        bool exported) noexcept :
    declaration_ { std::move(declaration) },
    variable_ { std::move(variable) },
    identifier_ { std::move(identifier) },
    exported_ { exported }
{}

optional_ptr<storage::column const>& column_info::declaration() noexcept {
    return declaration_;
}

optional_ptr<storage::column const> column_info::declaration() const noexcept {
    return declaration_;
}

descriptor::variable& column_info::variable() noexcept {
    return variable_;
}

descriptor::variable const& column_info::variable() const noexcept {
    return variable_;
}

std::optional<column_info::name_type>& column_info::identifier() noexcept {
    return identifier_;
}

std::optional<column_info::name_type> const& column_info::identifier() const noexcept {
    return identifier_;
}

bool& column_info::exported() noexcept {
    return exported_;
}

bool const& column_info::exported() const noexcept {
    return exported_;
}

std::optional<column_info::position_type>& column_info::next() noexcept {
    return next_;
}

std::optional<column_info::position_type> const& column_info::next() const noexcept {
    return next_;
}

std::ostream& operator<<(std::ostream& out, column_info const& value) {
    using ::takatori::util::print_support;
    return out << "column_info("
               << "declaration=" << print_support { value.declaration() } << ", "
               << "variable=" << value.variable() << ", "
               << "identifier=" << print_support { value.identifier() } << ", "
               << "exported=" << value.exported() << ", "
               << "next=" << print_support { value.next() } << ")";
}

} // namespace mizugaki::analyzer::details
