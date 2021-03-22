#include <mizugaki/translator/shakujo_translator_options.h>

#include <yugawara/storage/configurable_provider.h>
#include <yugawara/variable/configurable_provider.h>
#include <yugawara/function/configurable_provider.h>
#include <yugawara/aggregate/configurable_provider.h>

namespace mizugaki::translator {

shakujo_translator_options::shakujo_translator_options(
        std::shared_ptr<::yugawara::storage::provider const> storage_provider,
        std::shared_ptr<::yugawara::variable::provider const> variable_provider,
        std::shared_ptr<::yugawara::function::provider const> function_provider,
        std::shared_ptr<::yugawara::aggregate::provider const> aggregate_function_provider,
        std::shared_ptr<::yugawara::variable::provider const> host_variable_provider,
        ::takatori::util::object_creator creator) :
    creator_ { creator },
    storage_provider_ { std::move(storage_provider) },
    variable_provider_ { std::move(variable_provider) },
    function_provider_ { std::move(function_provider) },
    aggregate_function_provider_ { std::move(aggregate_function_provider) },
    host_variable_provider_ { std::move(host_variable_provider) }
{}

::yugawara::storage::provider const& shakujo_translator_options::storage_provider() const noexcept {
    if (storage_provider_) {
        return *storage_provider_;
    }
    static ::yugawara::storage::configurable_provider const empty;
    return empty;
}

::yugawara::variable::provider const& shakujo_translator_options::variable_provider() const noexcept {
    if (variable_provider_) {
        return *variable_provider_;
    }
    static ::yugawara::variable::configurable_provider const empty;
    return empty;
}

::yugawara::function::provider const& shakujo_translator_options::function_provider() const noexcept {
    if (function_provider_) {
        return *function_provider_;
    }
    static ::yugawara::function::configurable_provider const empty;
    return empty;
}

::yugawara::aggregate::provider const& shakujo_translator_options::aggregate_function_provider() const noexcept {
    if (aggregate_function_provider_) {
        return *aggregate_function_provider_;
    }
    static ::yugawara::aggregate::configurable_provider const empty;
    return empty;
}

::yugawara::variable::provider const& shakujo_translator_options::host_variable_provider() const noexcept {
    if (host_variable_provider_) {
        return *host_variable_provider_;
    }
    static ::yugawara::variable::configurable_provider const empty;
    return empty;
}

::takatori::util::object_creator shakujo_translator_options::get_object_creator() const noexcept {
    return creator_;
}

} // namespace mizugaki::translator
