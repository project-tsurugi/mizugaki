#include <mizugaki/translator/shakujo_translator_context.h>

#include "shakujo_translator_context_impl.h"

namespace mizugaki::translator {

shakujo_translator_context::shakujo_translator_context()
    : impl_(std::make_unique<impl>())
{}

shakujo_translator_context::shakujo_translator_context(
        std::shared_ptr<::yugawara::storage::provider const> storages,
        std::shared_ptr<::yugawara::variable::provider const> variables,
        std::shared_ptr<::yugawara::function::provider const> functions,
        std::shared_ptr<::yugawara::aggregate::provider const> aggregate_functions,
        ::takatori::util::object_creator creator)
    : impl_(std::make_unique<impl>(creator))
{
    impl_->storages(std::move(storages));
    impl_->variables(std::move(variables));
    impl_->functions(std::move(functions));
    impl_->aggregates(std::move(aggregate_functions));
}

shakujo_translator_context::~shakujo_translator_context() = default;

shakujo_translator_context::shakujo_translator_context(shakujo_translator_context&& other) noexcept = default;

shakujo_translator_context& shakujo_translator_context::operator=(shakujo_translator_context&& other) noexcept = default;

::yugawara::storage::provider const& shakujo_translator_context::storages() const noexcept {
    return impl_->storages();
}

::yugawara::variable::provider const& shakujo_translator_context::variables() const noexcept {
    return impl_->variables();
}

::yugawara::function::provider const& shakujo_translator_context::functions() const noexcept {
    return impl_->functions();
}

::yugawara::aggregate::provider const& shakujo_translator_context::aggregate_functions() const noexcept {
    return impl_->aggregates();
}

::takatori::util::object_creator shakujo_translator_context::get_object_creator() const {
    return impl_->get_object_creator();
}

} // namespace mizugaki::translator
