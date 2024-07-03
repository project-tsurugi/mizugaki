#include <mizugaki/analyzer/sql_analyzer.h>

#include <gtest/gtest.h>

#include <yugawara/schema/catalog.h>

#include <yugawara/schema/declaration.h>
#include <yugawara/schema/configurable_provider.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>
#include <yugawara/storage/configurable_provider.h>

#include <yugawara/variable/declaration.h>
#include <yugawara/variable/configurable_provider.h>

#include <yugawara/aggregate/declaration.h>
#include <yugawara/aggregate/configurable_provider.h>

#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/query/table_value_constructor.h>

#include <mizugaki/ast/statement/insert_statement.h>

#include "utils.h"

namespace mizugaki::analyzer {

using namespace testing;

class sql_analyzer_test : public ::testing::Test {
protected:
    [[nodiscard]] std::string diagnostics(sql_analyzer_result const& result) {
        if (result.is_valid()) {
            return {};
        }
        ::takatori::util::string_builder buffer;
        for (auto&& e : result.element<sql_analyzer_result_kind::diagnostics>()) {
            buffer << e << "\n";
        }
        return buffer.str();
    }

    std::shared_ptr<::yugawara::storage::configurable_provider> storages_{
            std::make_shared<::yugawara::storage::configurable_provider>()
    };
    std::shared_ptr<::yugawara::aggregate::configurable_provider> set_functions_{
            std::make_shared<::yugawara::aggregate::configurable_provider>()
    };
    std::shared_ptr<::yugawara::schema::configurable_provider> schemas_{
            std::make_shared<::yugawara::schema::configurable_provider>()
    };
    std::shared_ptr<::yugawara::schema::catalog> catalog_{
            std::make_shared<::yugawara::schema::catalog>(
                    "tsurugi",
                    std::nullopt,
                    schemas_)
    };
    std::shared_ptr<::yugawara::schema::declaration> default_schema_ =
            std::make_shared<::yugawara::schema::declaration>(
                    "public",
                    std::nullopt,
                    storages_,
                    std::shared_ptr<::yugawara::variable::provider> {},
                    std::shared_ptr<::yugawara::function::provider> {},
                    set_functions_);
    std::shared_ptr<::yugawara::schema::search_path> search_path_ {
            std::make_shared<::yugawara::schema::search_path>(
                    ::yugawara::schema::search_path::vector_type { default_schema_ })
    };
    sql_analyzer_options options_ {
            catalog_,
            search_path_,
            default_schema_,
    };
    placeholder_map placeholders_ {};
    ::yugawara::variable::configurable_provider host_parameters_ {};
};

TEST_F(sql_analyzer_test, insert_charcter_into_int_column) {
    auto table = storages_->add_table(::yugawara::storage::table {
            "t",
            {
                    {
                            "c0",
                            ttype::int8 {},
                    },
                    {
                            "c1",
                            ttype::float8 {},
                    },
            },
    });
    storages_->add_index(::yugawara::storage::index {
            table,
            ".t",
            {
                    {
                            table->columns()[0],
                            ::yugawara::storage::index::key::direction_type::ascendant,
                    },
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::primary,
                    ::yugawara::storage::index_feature::unique,
            },
    });
    sql_analyzer analyzer;
    auto result = analyzer(
            options_,
            ast::statement::insert_statement {
                    id("t"),
                    {
                            id("c0")
                    },
                    ast::query::table_value_constructor {
                            ast::scalar::value_constructor {
                                    literal(string("'p'"))
                            },
                    },
            }
    );
    ASSERT_TRUE(result) << diagnostics(result);
}

} // namespace mizugaki::analyzer