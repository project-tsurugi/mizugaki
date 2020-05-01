#include <mizugaki/translator/shakujo_translator_result.h>

namespace mizugaki::translator {

shakujo_translator_result::shakujo_translator_result(std::vector<shakujo_translator_diagnostic> element) noexcept
    : entity_(std::in_place_index<static_cast<std::size_t>(kind_type::diagnostics)>, std::move(element))
{}

shakujo_translator_result::shakujo_translator_result(std::unique_ptr<::takatori::relation::graph_type> element) noexcept
    : entity_(std::in_place_index<static_cast<std::size_t>(kind_type::execution_plan)>, std::move(element))
{}

shakujo_translator_result::shakujo_translator_result(std::unique_ptr<::takatori::statement::statement> element) noexcept
    : entity_(std::in_place_index<static_cast<std::size_t>(kind_type::statement)>, std::move(element))
{}

std::ostream& operator<<(std::ostream& out, shakujo_translator_result const& value) {
    return out << value.kind();
}

} // namespace mizugaki::translator
