#include <mizugaki/analyzer/details/scalar_value_context.h>

#include <takatori/util/clonable.h>
#include <takatori/util/print_support.h>

namespace mizugaki::analyzer::details {

using ::takatori::util::clone_shared;

scalar_value_context::scalar_value_context(
        std::shared_ptr<::takatori::type::data const> type,
        std::shared_ptr<::takatori::value::data const> default_value) noexcept :
    type_ { std::move(type) },
    default_value_ { std::move(default_value) }
{}

scalar_value_context::scalar_value_context(
        ::takatori::type::data&& type,
        ::takatori::util::rvalue_ptr<::takatori::value::data> default_value) noexcept :
    scalar_value_context {
            clone_shared(std::move(type)),
            clone_shared(default_value),
    }
{}

std::shared_ptr<::takatori::type::data const>& scalar_value_context::type() noexcept {
    return type_;
}

std::shared_ptr<::takatori::type::data const> const& scalar_value_context::type() const noexcept {
    return type_;
}

std::shared_ptr<::takatori::value::data const>& scalar_value_context::default_value() noexcept {
    return default_value_;
}

std::shared_ptr<::takatori::value::data const> const& scalar_value_context::default_value() const noexcept {
    return default_value_;
}

std::ostream& operator<<(std::ostream& out, scalar_value_context const& value) {
    using ::takatori::util::print_support;
    return out << "scalar_value_context("
               << "type=" << print_support { value.type() } << ", "
               << "value=" << print_support { value.default_value() } << ")";
}

} // namespace mizugaki::analyzer::details
