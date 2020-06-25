#include <mizugaki/analyzer/details/row_value_context.h>

#include <takatori/util/print_support.h>
#include <takatori/util/vector_print_support.h>

namespace mizugaki::analyzer::details {

row_value_context::row_value_context(
        std::vector<scalar_value_context> elements) noexcept :
    elements_ { std::move(elements) }
{}

row_value_context::row_value_context(std::initializer_list<scalar_value_context> elements) noexcept :
    row_value_context {
            decltype(elements_) { elements }
    }
{}

bool row_value_context::is_enabled() const noexcept {
    return true;
}

row_value_context::operator bool() const noexcept {
    return is_enabled();
}

std::vector<scalar_value_context>& row_value_context::elements() noexcept {
    return elements_;
}

std::vector<scalar_value_context> const& row_value_context::elements() const noexcept {
    return elements_;
}

std::ostream& operator<<(std::ostream& out, row_value_context const& value) {
    using ::takatori::util::print_support;
    return out << "row_value_context("
               << "enabled=" << print_support { value.is_enabled() } << ", "
               << "elements=" << print_support { value.elements() } << ")";
}

} // namespace mizugaki::analyzer::details
