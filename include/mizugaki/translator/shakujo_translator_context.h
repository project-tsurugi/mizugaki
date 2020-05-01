#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <takatori/util/object_creator.h>
#include <takatori/util/optional_ptr.h>

#include <yugawara/storage/provider.h>
#include <yugawara/variable/provider.h>
#include <yugawara/function/provider.h>
#include <yugawara/aggregate/provider.h>

namespace mizugaki::translator {

/**
 * @brief contextual information of shakujo_translator.
 */
class shakujo_translator_context {
public:
    /// @private
    class impl;

    /**
     * @brief creates a new instance.
     */
    shakujo_translator_context();

    /**
     * @brief creates a new instance.
     * @param storages the storage element provider
     * @param variables the external variable declaration provider
     * @param functions the function declaration provider
     * @param aggregate_functions the aggregate function declaration provider
     * @param creator the object creator to build IR elements
     * @note if each provider is empty, it will provide nothing
     */
    explicit shakujo_translator_context(
            std::shared_ptr<::yugawara::storage::provider const> storages,
            std::shared_ptr<::yugawara::variable::provider const> variables,
            std::shared_ptr<::yugawara::function::provider const> functions,
            std::shared_ptr<::yugawara::aggregate::provider const> aggregate_functions,
            ::takatori::util::object_creator creator = {});

    ~shakujo_translator_context();

    shakujo_translator_context(shakujo_translator_context const& other) = delete;
    shakujo_translator_context& operator=(shakujo_translator_context const& other) = delete;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    shakujo_translator_context(shakujo_translator_context&& other) noexcept;

    /**
     * @brief assigns the given object into this.
     * @param other the copy source
     */
    shakujo_translator_context& operator=(shakujo_translator_context&& other) noexcept;

    /**
     * @brief returns the storage element provider.
     * @return the storage element provider
     */
    [[nodiscard]] ::yugawara::storage::provider const& storages() const noexcept;

    /**
     * @brief returns the external variable declaration provider.
     * @return the variable provider
     */
    [[nodiscard]] ::yugawara::variable::provider const& variables() const noexcept;

    /**
     * @brief returns the function declaration provider.
     * @return the function provider
     */
    [[nodiscard]] ::yugawara::function::provider const& functions() const noexcept;

    /**
     * @brief returns the aggregate function declaration provider.
     * @return the aggregate function provider
     */
    [[nodiscard]] ::yugawara::aggregate::provider const& aggregate_functions() const noexcept;

    /**
     * @brief returns the object creator for building IR elements.
     * @return the object creator
     */
    [[nodiscard]] ::takatori::util::object_creator get_object_creator() const;

private:
    std::unique_ptr<impl> impl_;
    friend impl;
};

} // namespace mizugaki::translator
