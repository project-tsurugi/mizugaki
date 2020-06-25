#pragma once

#include <vector>

#include "scalar_value_context.h"

namespace mizugaki::analyzer::details {

class row_value_context {
public:
    using position_type = std::size_t;

    row_value_context() = default;
    explicit row_value_context(std::vector<scalar_value_context> elements) noexcept;
    row_value_context(std::initializer_list<scalar_value_context> elements) noexcept;
    [[nodiscard]] bool is_enabled() const noexcept;
    [[nodiscard]] explicit operator bool() const noexcept;
    [[nodiscard]] std::vector<scalar_value_context>& elements() noexcept;
    [[nodiscard]] std::vector<scalar_value_context> const& elements() const noexcept;

private:
    std::vector<scalar_value_context> elements_ {};
};

std::ostream& operator<<(std::ostream& out, row_value_context const& value);

} // namespace mizugaki::analyzer::details
