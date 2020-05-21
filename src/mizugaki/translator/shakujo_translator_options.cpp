#include <mizugaki/translator/shakujo_translator_options.h>

#include <yugawara/storage/configurable_provider.h>
#include <yugawara/variable/configurable_provider.h>
#include <yugawara/function/configurable_provider.h>
#include <yugawara/aggregate/configurable_provider.h>

namespace mizugaki::translator {

shakujo_translator_options::shakujo_translator_options(
        std::shared_ptr<::yugawara::storage::provider const> storages,
        std::shared_ptr<::yugawara::variable::provider const> variables,
        std::shared_ptr<::yugawara::function::provider const> functions,
        std::shared_ptr<::yugawara::aggregate::provider const> aggregate_functions,
        ::takatori::util::object_creator creator)
    : creator_(creator)
    , storages_(std::move(storages))
    , variables_(std::move(variables))
    , functions_(std::move(functions))
    , aggregate_functions_(std::move(aggregate_functions))
{}

::yugawara::storage::provider const& shakujo_translator_options::storages() const noexcept {
    if (storages_) {
        return *storages_;
    }
    static ::yugawara::storage::configurable_provider const empty;
    return empty;
}

::yugawara::variable::provider const& shakujo_translator_options::variables() const noexcept {
    if (variables_) {
        return *variables_;
    }
    static ::yugawara::variable::configurable_provider const empty;
    return empty;
}

::yugawara::function::provider const& shakujo_translator_options::functions() const noexcept {
    if (functions_) {
        return *functions_;
    }
    static ::yugawara::function::configurable_provider const empty;
    return empty;
}

::yugawara::aggregate::provider const& shakujo_translator_options::aggregate_functions() const noexcept {
    if (aggregate_functions_) {
        return *aggregate_functions_;
    }
    static ::yugawara::aggregate::configurable_provider const empty;
    return empty;
}

::takatori::util::object_creator shakujo_translator_options::get_object_creator() const noexcept {
    return creator_;
}

} // namespace mizugaki::translator
