#include <mizugaki/translator/shakujo_translator.h>

#include "shakujo_translator_impl.h"

namespace mizugaki::translator {

shakujo_translator::shakujo_translator()
    : impl_(std::make_unique<impl>())
{}

shakujo_translator::~shakujo_translator() = default;

shakujo_translator::shakujo_translator(shakujo_translator&& other) noexcept = default;

shakujo_translator& shakujo_translator::operator=(shakujo_translator&& other) noexcept = default;

shakujo_translator::result_type shakujo_translator::operator()(
        options_type& options,
        ::shakujo::model::statement::Statement const& statement,
        document_map const& documents,
        placeholder_map const& placeholders) {
    auto&& this_impl = impl::extract(*this);
    return this_impl(options, statement, documents, placeholders);
}

} // namespace mizugaki::translator
