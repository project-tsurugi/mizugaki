#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <utility>

#include <takatori/descriptor/variable.h>
#include <takatori/util/optional_ptr.h>

#include <yugawara/storage/column.h>

namespace mizugaki::analyzer::details {

class column_info {
public:
    using position_type = std::size_t;
    using name_type = std::string;

    column_info(
            ::takatori::util::optional_ptr<::yugawara::storage::column const> declaration,
            ::takatori::descriptor::variable variable,
            std::optional<name_type> identifier,
            bool exported = true) noexcept;

    [[nodiscard]] ::takatori::descriptor::variable& variable() noexcept;
    [[nodiscard]] ::takatori::descriptor::variable const& variable() const noexcept;
    [[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::column const>& declaration() noexcept;
    [[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::column const> declaration() const noexcept;
    [[nodiscard]] std::optional<name_type>& identifier() noexcept;
    [[nodiscard]] std::optional<name_type> const& identifier() const noexcept;
    [[nodiscard]] bool& exported() noexcept;
    [[nodiscard]] bool const& exported() const noexcept;
    [[nodiscard]] std::optional<position_type>& next() noexcept;
    [[nodiscard]] std::optional<position_type> const& next() const noexcept;

private:
    ::takatori::util::optional_ptr<::yugawara::storage::column const> declaration_;
    ::takatori::descriptor::variable variable_;
    std::optional<name_type> identifier_;
    bool exported_;
    std::optional<position_type> next_ {};
};

std::ostream& operator<<(std::ostream& out, column_info const& value);

} // namespace mizugaki::analyzer::details
