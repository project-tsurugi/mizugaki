#pragma once

#include <variant>

#include "value_context_kind.h"
#include "scalar_value_context.h"
#include "row_value_context.h"

namespace mizugaki::analyzer::details {

class value_context {
public:
    using kind_type = value_context_kind;
    using position_type = row_value_context::position_type;
    using alternatives_type = std::variant<std::monostate, scalar_value_context, row_value_context>;

    constexpr value_context() = default;
    value_context(scalar_value_context element) noexcept; // NOLINT: implicit conversion
    value_context(row_value_context element) noexcept; // NOLINT: implicit conversion
    [[nodiscard]] bool available() const noexcept;
    [[nodiscard]] explicit operator bool() const noexcept;

    [[nodiscard]] kind_type kind() const noexcept;

    template<kind_type Kind>
    [[nodiscard]] std::variant_alternative_t<static_cast<std::size_t>(Kind), alternatives_type>& get() = delete;

    template<kind_type Kind>
    [[nodiscard]] std::variant_alternative_t<static_cast<std::size_t>(Kind), alternatives_type> const& get() const = delete;

    [[nodiscard]] position_type size() const noexcept;
    [[nodiscard]] scalar_value_context const& find(position_type position) const;

private:
    alternatives_type alternatives_ {};
};

template<> scalar_value_context& value_context::get<value_context_kind::scalar>();
template<> row_value_context& value_context::get<value_context_kind::row>();
template<> scalar_value_context const& value_context::get<value_context_kind::scalar>() const;
template<> row_value_context const& value_context::get<value_context_kind::row>() const;

std::ostream& operator<<(std::ostream& out, value_context const& value);

} // namespace mizugaki::analyzer::details
