#include "options.h"

#include <memory>
#include <optional>

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>
#include <takatori/type/decimal.h>
#include <takatori/type/octet.h>
#include <takatori/type/lob.h>

#include <takatori/value/primitive.h>
#include <takatori/value/character.h>
#include <takatori/value/decimal.h>

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

#include "example_prototype_processor.h"

namespace mizugaki::examples::explain_cli {

[[nodiscard]] static std::shared_ptr<::yugawara::storage::provider> storage_provider();

[[nodiscard]] static std::shared_ptr<::yugawara::function::provider> function_provider();

[[nodiscard]] static std::shared_ptr<::yugawara::aggregate::provider> set_function_provider();

std::shared_ptr<::yugawara::schema::declaration> create_default_schema(std::string name) {
    auto schema = std::make_shared<::yugawara::schema::declaration>(
            std::nullopt,
            std::move(name),
            storage_provider(),
            std::shared_ptr<::yugawara::variable::provider> {},
            function_provider(),
            set_function_provider());
    return schema;
}

parser::sql_parser_options parser_options() {
    parser::sql_parser_options options {};
    return options;
}

analyzer::sql_analyzer_options analyzer_options(std::shared_ptr<::yugawara::schema::declaration> default_schema) {
    auto schema_provider = std::make_shared<::yugawara::schema::configurable_provider>();
    auto schema = schema_provider->add(std::move(default_schema));
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
    options.storage_processor(std::make_shared<example_prototype_processor>());
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
                            // s DECIMAL(18, 2) DEFAULT 0.00
                            "s",
                            ::takatori::type::decimal { 18, 2 },
                            ::yugawara::variable::nullable,
                            ::takatori::value::decimal {{ 0, -2 }},
                    },
                    {
                            // v VARCHAR(*) DEFAULT ''
                            "v",
                            ::takatori::type::character { ::takatori::type::varying, {} },
                            ::yugawara::variable::nullable,
                            ::takatori::value::character { "" },
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
    auto definition_id = ::yugawara::function::declaration::minimum_user_function_id;
    namespace tt = ::takatori::type;
    auto provider = std::make_shared<::yugawara::function::configurable_provider>();
    provider->add({ // OCTET_LENGTH(VARCHAR(*))
            ++definition_id,
            "octet_length",
            tt::int8 {},
            { tt::character { tt::varying, {} } },
    });
    provider->add({ // OCTET_LENGTH(VARBINARY(*))
            ++definition_id,
            "octet_length",
            tt::int8 {},
            { tt::octet { tt::varying, {} } },
    });
    provider->add({ // OCTET_LENGTH(CLOB)
            ++definition_id,
            "octet_length",
            tt::int8 {},
            { tt::clob {} },
    });
    provider->add({ // OCTET_LENGTH(BLOB)
            ++definition_id,
            "octet_length",
            tt::int8 {},
            { tt::blob {} },
    });
    return provider;
}

std::shared_ptr<::yugawara::aggregate::provider> set_function_provider() {
    auto definition_id = ::yugawara::aggregate::declaration::minimum_user_function_id;
    namespace tt = ::takatori::type;
    auto provider = std::make_shared<::yugawara::aggregate::configurable_provider>();
    provider->add({ // COUNT(*)
            ++definition_id,
            "count",
            tt::int8 {},
            {},
            true,
    });
    return provider;
}

} // namespace mizugaki::examples::explain_cli
