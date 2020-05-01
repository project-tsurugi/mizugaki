#include <mizugaki/translator/shakujo_translator.h>

#include "shakujo_translator_impl.h"
#include "shakujo_translator_context_impl.h"

namespace mizugaki::translator {

shakujo_translator::shakujo_translator()
    : impl_(std::make_unique<impl>())
{}

shakujo_translator::~shakujo_translator() = default;

shakujo_translator::shakujo_translator(shakujo_translator&& other) noexcept = default;

shakujo_translator& shakujo_translator::operator=(shakujo_translator&& other) noexcept = default;

shakujo_translator::result_type shakujo_translator::operator()(
        context_type& context,
        ::shakujo::model::statement::Statement const& statement,
        document_map const& documents,
        placeholder_map const& placeholders) {
    auto&& this_impl = impl::extract(*this);
    auto&& context_impl = context_type::impl::extract(context);
    return this_impl(context_impl, statement, documents, placeholders);
}

} // namespace mizugaki::translator
