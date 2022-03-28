#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <takatori/util/optional_ptr.h>

#include <yugawara/schema/declaration.h>

#include <yugawara/storage/provider.h>
#include <yugawara/variable/provider.h>
#include <yugawara/function/provider.h>
#include <yugawara/aggregate/provider.h>

namespace mizugaki::translator {

/**
 * @brief options of shakujo_translator.
 */
class shakujo_translator_options {
public:
    /**
     * @brief creates a new instance.
     * @param schema the target schema
     * @param host_variable_provider the host variable declaration provider
     * @note if the provider is empty, it will provide nothing
     */
    shakujo_translator_options( // NOLINT
            ::std::shared_ptr<::yugawara::schema::declaration> schema,
            std::shared_ptr<::yugawara::variable::provider const> host_variable_provider = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param storage_provider the storage element provider
     * @param variable_provider the external variable declaration provider
     * @param function_provider the function declaration provider
     * @param aggregate_function_provider the aggregate function declaration provider
     * @param host_variable_provider the host variable declaration provider
     * @note if each provider is empty, it will provide nothing
     */
    shakujo_translator_options(
            std::shared_ptr<::yugawara::storage::provider> storage_provider,
            std::shared_ptr<::yugawara::variable::provider> variable_provider,
            std::shared_ptr<::yugawara::function::provider> function_provider,
            std::shared_ptr<::yugawara::aggregate::provider> aggregate_function_provider,
            std::shared_ptr<::yugawara::variable::provider const> host_variable_provider = {});

    /**
     * @brief returns the target schema.
     * @return the target schema
     */
    [[nodiscard]] ::yugawara::schema::declaration const& schema() const noexcept;

    /**
     * @brief returns the target schema.
     * @return the target schema as shared pointer
     */
    [[nodiscard]] std::shared_ptr<::yugawara::schema::declaration> shared_schema() const noexcept;

    /**
     * @brief returns the storage element provider.
     * @return the storage element provider
     */
    [[nodiscard]] ::yugawara::storage::provider const& storage_provider() const noexcept;

    /**
     * @brief returns the external variable declaration provider.
     * @return the variable provider
     */
    [[nodiscard]] ::yugawara::variable::provider const& variable_provider() const noexcept;

    /**
     * @brief returns the function declaration provider.
     * @return the function provider
     */
    [[nodiscard]] ::yugawara::function::provider const& function_provider() const noexcept;

    /**
     * @brief returns the aggregate function declaration provider.
     * @return the aggregate function provider
     */
    [[nodiscard]] ::yugawara::aggregate::provider const& aggregate_function_provider() const noexcept;

    /**
     * @brief returns the host variable declaration provider.
     * @return the host variable provider
     */
    [[nodiscard]] ::yugawara::variable::provider const& host_variable_provider() const noexcept;

private:
    std::shared_ptr<::yugawara::schema::declaration> schema_;
    std::shared_ptr<::yugawara::variable::provider const> host_variable_provider_;
};

} // namespace mizugaki::translator
