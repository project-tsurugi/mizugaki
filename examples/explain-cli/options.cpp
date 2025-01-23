#include "options.h"

#include <memory>
#include <optional>

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>
#include <takatori/type/decimal.h>

#include <takatori/value/primitive.h>
#include <takatori/value/character.h>

#include <yugawara/runtime_feature.h>

#include <yugawara/aggregate/configurable_provider.h>

#include <yugawara/function/configurable_provider.h>

#include <yugawara/schema/catalog.h>
#include <yugawara/schema/declaration.h>
#include <yugawara/schema/configurable_provider.h>
#include <yugawara/schema/search_path.h>

#include <yugawara/storage/configurable_provider.h>
#include <yugawara/storage/table.h>
#include <yugawara/storage/column.h>
#include <yugawara/storage/index.h>

namespace mizugaki::examples::explain_cli {

[[nodiscard]] static std::shared_ptr<::yugawara::storage::provider> storage_provider();

[[nodiscard]] static std::shared_ptr<::yugawara::function::provider> function_provider();

[[nodiscard]] static std::shared_ptr<::yugawara::aggregate::provider> set_function_provider();

parser::sql_parser_options parser_options() {
    parser::sql_parser_options options {};
    return options;
}

analyzer::sql_analyzer_options analyzer_options() {
    auto schema_provider = std::make_shared<::yugawara::schema::configurable_provider>();
    auto schema = schema_provider->add({
            "public",
            std::nullopt,
            storage_provider(),
            {},
            function_provider(),
            set_function_provider(),
    });
    auto catalog = std::make_shared<::yugawara::schema::catalog>(
            "tsurugi",
            std::nullopt,
            std::move(schema_provider));

    auto search_path = std::make_shared<::yugawara::schema::search_path>(
            ::yugawara::schema::search_path::vector_type { schema });

    analyzer::sql_analyzer_options options {
            std::move(catalog),
            std::move(search_path),
            std::move(schema),
    };
    return options;
}

::yugawara::compiler_options compiler_options() {
    ::yugawara::runtime_feature_set features {};
    features.insert(::yugawara::runtime_feature::broadcast_exchange);
    features.insert(::yugawara::runtime_feature::aggregate_exchange);
    features.insert(::yugawara::runtime_feature::index_join);
    features.insert(::yugawara::runtime_feature::index_join_scan);
    features.insert(::yugawara::runtime_feature::broadcast_join_scan);
    features.insert(::yugawara::runtime_feature::always_inline_scalar_local_variables);
    ::yugawara::compiler_options options { features };
    return options;
}

std::shared_ptr<::yugawara::storage::provider> storage_provider() {
    auto provider = std::make_shared<::yugawara::storage::configurable_provider>();
    // CREATE TABLE ksv (...);
    auto ksv = provider->add_table({
            "ksv",
            {
                    {
                            // k BIGINT PRIMARY KEY
                            "k",
                            ::takatori::type::int8 {},
                            ~::yugawara::variable::nullable,
                            {},
                    },
                    {
                            // s DECIMAL(18, 2) DEFAULT NULL
                            "s",
                            ::takatori::type::decimal { 18, 2 },
                            ::yugawara::variable::nullable,
                            ::takatori::value::unknown {},
                    },
                    {
                            // v VARCHAR(*) DEFAULT NULL
                            "v",
                            ::takatori::type::character { ::takatori::type::varying, {} },
                            ::yugawara::variable::nullable,
                            ::takatori::value::unknown {},
                    },
            },
    });
    // primary key of ksv
    auto ksv_p = provider->add_index({
            ksv,
            ".ksv",
            { ksv->columns()[0] },
            {},
            {
                    ::yugawara::storage::index_feature::primary,
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::unique,
                    ::yugawara::storage::index_feature::unique_constraint,
            },
    });
    // CREATE INDEX ON ksv (s)
    auto ksv_s = provider->add_index({
            ksv,
            "ksv_s",
            { ksv->columns()[1] },
            {},
            {
                    ::yugawara::storage::index_feature::primary,
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
            },
    });
    return provider;
}

std::shared_ptr<::yugawara::function::provider> function_provider() {
    auto provider = std::make_shared<::yugawara::function::configurable_provider>();
    return provider;
}

std::shared_ptr<::yugawara::aggregate::provider> set_function_provider() {
    auto provider = std::make_shared<::yugawara::aggregate::configurable_provider>();
    return provider;
}

} // namespace mizugaki::examples::explain_cli
