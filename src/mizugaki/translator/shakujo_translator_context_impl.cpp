#include "shakujo_translator_context_impl.h"

#include <takatori/util/downcast.h>

#include <yugawara/storage/configurable_provider.h>
#include <yugawara/variable/configurable_provider.h>
#include <yugawara/function/configurable_provider.h>
#include <yugawara/aggregate/configurable_provider.h>

#include <shakujo/model/name/SimpleName.h>

namespace mizugaki::translator {

using impl = shakujo_translator_context::impl;

using ::takatori::util::downcast;
using ::takatori::util::optional_ptr;

impl::impl(::takatori::util::object_creator creator) noexcept
    : creator_(creator)
{}

::yugawara::storage::provider const& impl::storages() const noexcept {
    if (storages_) {
        return *storages_;
    }
    static ::yugawara::storage::configurable_provider const empty;
    return empty;
}

::yugawara::variable::provider const& impl::variables() const noexcept {
    if (variables_) {
        return *variables_;
    }
    static ::yugawara::variable::configurable_provider const empty;
    return empty;
}

::yugawara::function::provider const& impl::functions() const noexcept {
    if (functions_) {
        return *functions_;
    }
    static ::yugawara::function::configurable_provider const empty;
    return empty;
}

::yugawara::aggregate::provider const& impl::aggregates() const noexcept {
    if (aggregate_functions_) {
        return *aggregate_functions_;
    }
    static ::yugawara::aggregate::configurable_provider const empty;
    return empty;
}

void impl::storages(std::shared_ptr<::yugawara::storage::provider const> provider) noexcept {
    storages_ = std::move(provider);
}

void impl::variables(std::shared_ptr<::yugawara::variable::provider const> provider) noexcept {
    variables_ = std::move(provider);
}

void impl::functions(std::shared_ptr<::yugawara::function::provider const> provider) noexcept {
    functions_ = std::move(provider);
}

void impl::aggregates(std::shared_ptr<::yugawara::aggregate::provider const> provider) noexcept {
    aggregate_functions_ = std::move(provider);
}

optional_ptr<::yugawara::storage::index const> impl::find_table(std::string_view name) const {
    if (auto table = storages().find_table(name)) {
        if (auto index = storages().find_primary_index(*table)) {
            return *index;
        }
    }
    return {};
}

optional_ptr<::yugawara::storage::index const> impl::find_table(::shakujo::model::name::Name const& name) const {
    if (auto const* n = downcast<::shakujo::model::name::SimpleName>(&name); n != nullptr) {
        return find_table(n->token());
    }
    // FIXME: qualified table name
    return {};
}

::takatori::util::object_creator impl::get_object_creator() const {
    return creator_;
}

impl& impl::extract(shakujo_translator_context& interface) noexcept {
    return *interface.impl_;
}

} // namespace mizugaki::translator
