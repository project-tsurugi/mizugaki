#pragma once

#include <memory>

#include <takatori/type/data.h>
#include <takatori/value/data.h>
#include <takatori/util/rvalue_ptr.h>

namespace mizugaki::analyzer::details {

class scalar_value_context {
public:
    constexpr scalar_value_context() = default;

    scalar_value_context( // NOLINT: allow implicit conversions
            std::shared_ptr<::takatori::type::data const> type,
            std::shared_ptr<::takatori::value::data const> default_value = {}) noexcept;

    scalar_value_context( // NOLINT: allow implicit conversions
            ::takatori::type::data&& type,
            ::takatori::util::rvalue_ptr<::takatori::value::data> default_value = {}) noexcept;

    [[nodiscard]] std::shared_ptr<::takatori::type::data const>& type() noexcept;
    [[nodiscard]] std::shared_ptr<::takatori::type::data const> const& type() const noexcept;
    [[nodiscard]] std::shared_ptr<::takatori::value::data const>& default_value() noexcept;
    [[nodiscard]] std::shared_ptr<::takatori::value::data const> const& default_value() const noexcept;

private:
    std::shared_ptr<::takatori::type::data const> type_ {};
    std::shared_ptr<::takatori::value::data const> default_value_ {};
};

std::ostream& operator<<(std::ostream& out, scalar_value_context const& value);

} // namespace mizugaki::analyzer::details
