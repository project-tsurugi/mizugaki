#include <mizugaki/translator/shakujo_translator_options.h>

#include <yugawara/variable/null_provider.h>

namespace mizugaki::translator {

shakujo_translator_options::shakujo_translator_options(
        ::std::shared_ptr<::yugawara::schema::declaration> schema,
        std::shared_ptr<::yugawara::variable::provider const> host_variable_provider) noexcept :
    schema_ { std::move(schema) },
    host_variable_provider_ { std::move(host_variable_provider) }
{}

shakujo_translator_options::shakujo_translator_options(
        std::shared_ptr<::yugawara::storage::provider> storage_provider,
        std::shared_ptr<::yugawara::variable::provider> variable_provider,
        std::shared_ptr<::yugawara::function::provider> function_provider,
        std::shared_ptr<::yugawara::aggregate::provider> aggregate_function_provider,
        std::shared_ptr<::yugawara::variable::provider const> host_variable_provider) :
    schema_ { std::make_shared<::yugawara::schema::declaration>(
            std::string_view {},
            std::nullopt,
            std::move(storage_provider),
            std::move(variable_provider),
            std::move(function_provider),
            std::move(aggregate_function_provider),
            std::shared_ptr<::yugawara::type::provider> {}) },
    host_variable_provider_ { std::move(host_variable_provider) }
{}

::yugawara::schema::declaration const& shakujo_translator_options::schema() const noexcept {
    return *schema_;
}

std::shared_ptr<::yugawara::schema::declaration> shakujo_translator_options::shared_schema() const noexcept {
    return schema_;
}

::yugawara::storage::provider const& shakujo_translator_options::storage_provider() const noexcept {
    return schema_->storage_provider();
}

::yugawara::variable::provider const& shakujo_translator_options::variable_provider() const noexcept {
    return schema_->variable_provider();
}

::yugawara::function::provider const& shakujo_translator_options::function_provider() const noexcept {
    return schema_->function_provider();
}

::yugawara::aggregate::provider const& shakujo_translator_options::aggregate_function_provider() const noexcept {
    return schema_->set_function_provider();
}

::yugawara::variable::provider const& shakujo_translator_options::host_variable_provider() const noexcept {
    if (host_variable_provider_) {
        return *host_variable_provider_;
    }
    static ::yugawara::variable::null_provider const empty;
    return empty;
}

} // namespace mizugaki::translator
