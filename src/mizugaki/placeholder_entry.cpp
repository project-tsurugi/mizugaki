#include <mizugaki/placeholder_entry.h>

#include <takatori/scalar/immediate.h>

#include <takatori/util/clonable.h>

namespace mizugaki {

namespace type = ::takatori::type;
namespace value = ::takatori::value;
namespace scalar = ::takatori::scalar;

using ::takatori::util::clone_shared;

placeholder_entry::placeholder_entry(
        std::shared_ptr<value::data const> value,
        std::shared_ptr<type::data const> type) noexcept
    : value_(std::move(value))
    , type_(std::move(type))
{}

placeholder_entry::placeholder_entry(
        std::shared_ptr<type::data const> type,
        std::shared_ptr<value::data const> value) noexcept
    : value_(std::move(value))
    , type_(std::move(type))
{}

placeholder_entry::placeholder_entry(value::data&& value, type::data&& type)
    : placeholder_entry(
            clone_shared(std::move(value)),
            clone_shared(std::move(type)))
{}

placeholder_entry::placeholder_entry(type::data&& type, value::data&& value)
    : placeholder_entry(
        clone_shared(std::move(value)),
        clone_shared(std::move(type)))
{}

std::unique_ptr<scalar::expression> placeholder_entry::resolve() const {
    return std::make_unique<scalar::immediate>(value_, type_);
}

std::ostream& operator<<(std::ostream& out, placeholder_entry const& value) {
    return out << "placeholder("
               << "value=" << *value.value_ << ", "
               << "type=" << *value.type_ << ")";
}

} // namespace mizugaki
