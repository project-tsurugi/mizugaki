#include <mizugaki/analyzer/details/analyze_statement.h>

#include <optional>
#include <stdexcept>

#include <gtest/gtest.h>

#include <takatori/util/clonable.h>
#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>
#include <takatori/type/decimal.h>
#include <takatori/type/character.h>
#include <takatori/type/octet.h>
#include <takatori/type/date.h>
#include <takatori/type/time_point.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/create_table.h>

#include <yugawara/binding/factory.h>
#include <yugawara/binding/extract.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>
#include <yugawara/storage/column.h>

#include <yugawara/function/declaration.h>

#include <mizugaki/ast/type/simple.h>
#include <mizugaki/ast/type/character_string.h>
#include <mizugaki/ast/type/user_defined.h>
#include <mizugaki/ast/type/decimal.h>
#include <mizugaki/ast/type/datetime.h>
#include <mizugaki/ast/type/octet_string.h>

#include <mizugaki/ast/scalar/unary_expression.h>
#include <mizugaki/ast/scalar/host_parameter_reference.h>
#include <mizugaki/ast/scalar/builtin_function_invocation.h>
#include <mizugaki/ast/scalar/function_invocation.h>
#include <mizugaki/ast/scalar/extract_expression.h>
#include <mizugaki/ast/scalar/binary_expression.h>

#include <mizugaki/ast/statement/table_definition.h>
#include <mizugaki/ast/statement/column_definition.h>
#include <mizugaki/ast/statement/table_constraint_definition.h>
#include <mizugaki/ast/statement/simple_constraint.h>
#include <mizugaki/ast/statement/key_constraint.h>
#include <mizugaki/ast/statement/expression_constraint.h>
#include <mizugaki/ast/statement/partition_definition.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_table_definition_test : public test_parent {
protected:
    void invalid(ast::statement::statement const& stmt) {
        trelation::graph_type graph {};
        auto r = analyze_statement(context(), stmt);
        EXPECT_TRUE(std::holds_alternative<erroneous_result_type>(r)) << diagnostics();
        EXPECT_GT(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::statement::statement const& stmt) {
        invalid(stmt);
        EXPECT_TRUE(find_error(code)) << diagnostics();
    }

    /**
     * @brief returns a table definition with columns c1, c2, and c3 (INT).
     * @param primary_key the primary key elements, or empty to omit the primary key
     * @param partition the partition key elements, or empty to omit the partition definition
     */
    static ast::statement::table_definition partitioned_table(
            std::initializer_list<ast::common::sort_element> primary_key,
            std::initializer_list<ast::statement::partition_key_element> partition) {
        std::vector<std::unique_ptr<ast::statement::table_element>> elements {};
        for (auto&& name : { "c1", "c2", "c3" }) {
            elements.emplace_back(std::make_unique<ast::statement::column_definition>(
                    id(name),
                    ast::type::simple { ast::type::kind::integer }));
        }
        if (primary_key.size() != 0) {
            elements.emplace_back(std::make_unique<ast::statement::table_constraint_definition>(
                    ast::statement::key_constraint {
                            ast::statement::constraint_kind::primary_key,
                            primary_key,
                    }));
        }
        std::optional<ast::statement::partition_definition> definition {};
        if (partition.size() != 0) {
            definition.emplace(partition);
        }
        return ast::statement::table_definition {
                ::takatori::util::clone_unique(id("testing")),
                std::move(elements),
                std::move(definition),
        };
    }

    /**
     * @brief returns a table definition with columns k (INT), v, and w (INT), whose primary key is (k, v).
     * @param value_type the type of column v
     * @param partition the partition key elements
     */
    static ast::statement::table_definition virtual_column_table(
            ast::type::type&& value_type,
            std::initializer_list<ast::statement::partition_key_element> partition) {
        std::vector<std::unique_ptr<ast::statement::table_element>> elements {};
        elements.emplace_back(std::make_unique<ast::statement::column_definition>(
                id("k"),
                ast::type::simple { ast::type::kind::integer }));
        elements.emplace_back(std::make_unique<ast::statement::column_definition>(
                id("v"),
                std::move(value_type)));
        elements.emplace_back(std::make_unique<ast::statement::column_definition>(
                id("w"),
                ast::type::simple { ast::type::kind::integer }));
        elements.emplace_back(std::make_unique<ast::statement::table_constraint_definition>(
                ast::statement::key_constraint {
                        ast::statement::constraint_kind::primary_key,
                        {
                                { id("k") },
                                { id("v") },
                        },
                }));
        return ast::statement::table_definition {
                ::takatori::util::clone_unique(id("testing")),
                std::move(elements),
                ast::statement::partition_definition { partition },
        };
    }

    /**
     * @brief returns a partition key element of the virtual column.
     */
    static ast::statement::partition_key_element virtual_column(
            ast::scalar::expression&& expression,
            std::string_view name = "x") {
        return ast::statement::partition_key_element {
                std::move(expression),
                id(name),
        };
    }

    void declare_function(std::string_view name, ttype::data&& return_type, ttype::data&& parameter) {
        functions_->add(::yugawara::function::declaration {
                next_function_id_++,
                name,
                std::move(return_type),
                { std::move(parameter) },
                { ::yugawara::function::function_feature::scalar_function },
        });
    }

    void declare_function(
            std::string_view name,
            ttype::data&& return_type,
            ttype::data&& parameter0,
            ttype::data&& parameter1) {
        functions_->add(::yugawara::function::declaration {
                next_function_id_++,
                name,
                std::move(return_type),
                { std::move(parameter0), std::move(parameter1) },
                { ::yugawara::function::function_feature::scalar_function },
        });
    }

    void declare_function(
            std::string_view name,
            ttype::data&& return_type,
            ttype::data&& parameter0,
            ttype::data&& parameter1,
            ttype::data&& parameter2) {
        functions_->add(::yugawara::function::declaration {
                next_function_id_++,
                name,
                std::move(return_type),
                { std::move(parameter0), std::move(parameter1), std::move(parameter2) },
                { ::yugawara::function::function_feature::scalar_function },
        });
    }

    ::yugawara::storage::partition_key_element const& virtual_key_of(analyze_statement_result_type const& r) {
        auto&& keys = primary_index_of(r).partition_key_columns();
        if (keys.size() != 2) {
            throw std::domain_error(diagnostics());
        }
        return keys[1];
    }

    std::size_t next_function_id_ { ::yugawara::function::declaration::minimum_builtin_function_id };

    ::yugawara::storage::index const& primary_index_of(analyze_statement_result_type const& r) {
        auto alternative = std::get_if<statement_result_type>(&r);
        if (!alternative) {
            throw std::domain_error("not a statement");
        }
        auto&& stmt = downcast<tstatement::create_table>(**alternative);
        return extract<::yugawara::storage::index>(stmt.primary_key());
    }
};

TEST_F(analyze_statement_table_definition_test, simple) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value { tvalue::unknown {} });
        EXPECT_EQ(column.description(), "");
    }

    auto&& key = extract<::yugawara::storage::index>(stmt.primary_key());
    EXPECT_EQ(key.simple_name(), "");
    ASSERT_EQ(key.keys().size(), 0);
    EXPECT_EQ(key.features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
    EXPECT_EQ(key.description(), "");

    EXPECT_EQ(table.description(), "");
}

TEST_F(analyze_statement_table_definition_test, column_multiple) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c0"),
                            ast::type::simple { ast::type::kind::small_integer },
                    },
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::column_definition {
                            id("c2"),
                            ast::type::simple { ast::type::kind::big_integer },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 3);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c0");
        EXPECT_EQ(column.type(), ttype::int2 {});
    }
    {
        auto&& column = table_columns[1];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
    }
    {
        auto&& column = table_columns[2];
        EXPECT_EQ(column.simple_name(), "c2");
        EXPECT_EQ(column.type(), ttype::int8 {});
    }
}

TEST_F(analyze_statement_table_definition_test, column_missing) {
    invalid(sql_analyzer_code::malformed_syntax, ast::statement::table_definition {
            id("testing"),
            {},
    });
}

TEST_F(analyze_statement_table_definition_test, column_primary_key) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ~::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value {});
    }

    auto&& key = extract<::yugawara::storage::index>(stmt.primary_key());
    EXPECT_EQ(key.simple_name(), "");
    auto&& key_columns = key.keys();
    ASSERT_EQ(key_columns.size(), 1);
    {
        auto&& column = key_columns[0];
        EXPECT_EQ(column.column(), table_columns[0]);
        EXPECT_EQ(column.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    EXPECT_EQ(key.features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

TEST_F(analyze_statement_table_definition_test, column_null) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::null,
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value { tvalue::unknown {} });
    }
}

TEST_F(analyze_statement_table_definition_test, column_not_null) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::not_null,
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ~::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value {});
    }
}

TEST_F(analyze_statement_table_definition_test, column_default) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            literal(number("1")),
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int8 {});
        EXPECT_EQ(column.criteria().nullity(), ::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value { tvalue::int8 { 1 } });
    }
}

TEST_F(analyze_statement_table_definition_test, column_default_function) {
    auto f = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "current_user",
            ttype::character { ttype::varying, {} },
            {},
    });
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::character_string {
                                    ast::type::kind::character_varying,
                            },
                            {
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            ast::scalar::builtin_function_invocation {
                                                    ast::scalar::builtin_function_kind::current_user,
                                                    {},
                                            },
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), (ttype::character { ttype::varying, {} }));
        EXPECT_EQ(column.criteria().nullity(), ::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value { f });
    }
}

TEST_F(analyze_statement_table_definition_test, column_default_function_invalid_arg) {
    auto f = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "length",
            ttype::int8 {},
            {
                    ttype::character { ttype::varying, {} },
            },
    });
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            ast::scalar::builtin_function_invocation {
                                                    ast::scalar::builtin_function_kind::length,
                                                    {
                                                            literal(string("'testing'"))
                                                    },
                                            },
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, table_primary_key) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    id("c1"),
                                            }
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ~::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value {});
    }

    auto&& key = extract<::yugawara::storage::index>(stmt.primary_key());
    EXPECT_EQ(key.simple_name(), "");
    auto&& key_columns = key.keys();
    ASSERT_EQ(key_columns.size(), 1);
    {
        auto&& column = key_columns[0];
        EXPECT_EQ(column.column(), table_columns[0]);
        EXPECT_EQ(column.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    EXPECT_EQ(key.features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

TEST_F(analyze_statement_table_definition_test, table_primary_key_multiple_columns) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::column_definition {
                            id("c2"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::column_definition {
                            id("c3"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    id("c1"),
                                            },
                                            {
                                                    id("c2"),
                                            },
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 3);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ~::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value {});
    }
    {
        auto&& column = table_columns[1];
        EXPECT_EQ(column.simple_name(), "c2");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ~::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value {});
    }
    {
        auto&& column = table_columns[2];
        EXPECT_EQ(column.simple_name(), "c3");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value { tvalue::unknown {} });
    }

    auto&& key = extract<::yugawara::storage::index>(stmt.primary_key());
    EXPECT_EQ(key.simple_name(), "");
    auto&& key_columns = key.keys();
    ASSERT_EQ(key_columns.size(), 2);
    {
        auto&& column = key_columns[0];
        EXPECT_EQ(column.column(), table_columns[0]);
        EXPECT_EQ(column.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    {
        auto&& column = key_columns[1];
        EXPECT_EQ(column.column(), table_columns[1]);
        EXPECT_EQ(column.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    EXPECT_EQ(key.features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

TEST_F(analyze_statement_table_definition_test, table_primary_key_missing_columns) {
    invalid(sql_analyzer_code::malformed_syntax, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {},
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, description) {
    auto desc_table = add_comment("/** table */");
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
            },
            {},
            {},
            {},
            desc_table,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.description(), "table");
}

TEST_F(analyze_statement_table_definition_test, column_description) {
    auto column_desc = add_comment("/** column */");
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {},
                            column_desc,
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    EXPECT_EQ(table_columns[0].description(), "column");
}

TEST_F(analyze_statement_table_definition_test, duplicate_target) {
    install_table("testing"); // conflict
    invalid(sql_analyzer_code::table_already_exists, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, duplicate_target_if_not_exists) {
    install_table("testing"); // conflict
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
            },
            {},
            {
                    ast::statement::table_definition_option::if_not_exists,
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_table_definition_test, column_invalid_type) {
    invalid(ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::user_defined { id("MISSING") },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_duplicated) {
    invalid(sql_analyzer_code::column_already_exists, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::column_definition {
                            id("c2"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::column_definition {
                            id("c3"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::column_definition {
                            id("c1"), // conflict
                            ast::type::simple { ast::type::kind::integer },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_primary_key_multiple) {
    invalid(sql_analyzer_code::primary_index_already_exists, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                            },
                    },
                    ast::statement::column_definition {
                            id("c2"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::column_definition {
                            id("c3"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_constraint_primary_key_include) {
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                            {},
                                            {
                                                    vref(id("c2")),
                                            },
                                    },
                            },
                    },
                    ast::statement::column_definition {
                            id("c2"),
                            ast::type::simple { ast::type::kind::integer },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_constraint_primary_key_parameter) {
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                            {},
                                            {},
                                            {
                                                    { id("testing_parameter"), literal(number("1")) },
                                            },
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_constraint_primary_key_vs_primary_key) {
    invalid(sql_analyzer_code::primary_index_already_exists, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_constraint_primary_key_vs_null) {
    invalid(sql_analyzer_code::invalid_constraint, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::null,
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_constraint_primary_key_vs_not_null) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::not_null,
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ~::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value {});
    }

    auto&& key = extract<::yugawara::storage::index>(stmt.primary_key());
    EXPECT_EQ(key.simple_name(), "");
    auto&& key_columns = key.keys();
    ASSERT_EQ(key_columns.size(), 1);
    EXPECT_EQ(key_columns[0].column(), table_columns[0]);

    EXPECT_EQ(key.features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

TEST_F(analyze_statement_table_definition_test, column_constraint_primary_key_vs_default) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            literal(number("1")),
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int8 {});
        EXPECT_EQ(column.criteria().nullity(), ~::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value { tvalue::int8 { 1 } });
    }

    auto&& key = extract<::yugawara::storage::index>(stmt.primary_key());
    EXPECT_EQ(key.simple_name(), "");
    auto&& key_columns = key.keys();
    ASSERT_EQ(key_columns.size(), 1);
    EXPECT_EQ(key_columns[0].column(), table_columns[0]);

    EXPECT_EQ(key.features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

TEST_F(analyze_statement_table_definition_test, column_constraint_null_vs_primary_key) {
    invalid(sql_analyzer_code::invalid_constraint, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::null,
                                    },
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_constraint_null_vs_null) {
    invalid(sql_analyzer_code::invalid_constraint, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::null,
                                    },
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::null,
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_constraint_null_vs_not_null) {
    invalid(sql_analyzer_code::invalid_constraint, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::null,
                                    },
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::not_null,
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_constraint_null_vs_default) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::null,
                                    },
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            literal(number("1")),
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int8 {});
        EXPECT_EQ(column.criteria().nullity(), ::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value { tvalue::int8 { 1 } });
    }
}

TEST_F(analyze_statement_table_definition_test, column_constraint_not_null_vs_primary_key) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::not_null,
                                    },
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ~::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value {});
    }

    auto&& key = extract<::yugawara::storage::index>(stmt.primary_key());
    EXPECT_EQ(key.simple_name(), "");
    auto&& key_columns = key.keys();
    ASSERT_EQ(key_columns.size(), 1);
    EXPECT_EQ(key_columns[0].column(), table_columns[0]);

    EXPECT_EQ(key.features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

TEST_F(analyze_statement_table_definition_test, column_constraint_not_null_vs_null) {
    invalid(sql_analyzer_code::invalid_constraint, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::not_null,
                                    },
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::null,
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_constraint_not_null_vs_not_null) {
    invalid(sql_analyzer_code::invalid_constraint, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::not_null,
                                    },
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::not_null,
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_constraint_not_null_vs_default) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::not_null,
                                    },
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            literal(number("1")),
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(extract(stmt.schema()), *default_schema_);

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int8 {});
        EXPECT_EQ(column.criteria().nullity(), ~::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value { tvalue::int8 { 1 } });
    }
}

TEST_F(analyze_statement_table_definition_test, column_constraint_default_vs_default) {
    invalid(sql_analyzer_code::invalid_constraint, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            literal(number("1")),
                                    },
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            literal(number("2")),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_default_invalid_expression) {
    invalid(ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            vref(id("MISSING")),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, column_default_not_literal) {
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            ast::scalar::unary_expression {
                                                    ast::scalar::unary_operator::minus,
                                                    literal(number("1")),
                                            }
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, table_primary_key_vs_table_primary_key) {
    invalid(sql_analyzer_code::primary_index_already_exists, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::column_definition {
                            id("c2"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    id("c1"),
                                            }
                                    },
                            },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    id("c2"),
                                            }
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, table_primary_key_vs_column_primary_key) {
    invalid(sql_analyzer_code::primary_index_already_exists, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                            },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    id("c1"),
                                            }
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, table_primary_key_column_invalid_expression) {
    invalid(ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    id("MISSING"),
                                            }
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, table_primary_key_column_collate) {
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    vref(id("c1")),
                                                    id("testing_collation"),
                                            }
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, table_primary_key_parameter) {
    invalid(ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    vref(id("c1")),
                                            }
                                    },
                                    {},
                                    {
                                            { id("testing_parameter"), literal(number("1")) },
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, constraint_name_distinct) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::column_constraint_definition {
                                            ast::statement::key_constraint {
                                                    ast::statement::constraint_kind::primary_key,
                                            },
                                            id("n1"),
                                    },
                            },
                    },
                    ast::statement::column_definition {
                            id("c2"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::column_constraint_definition {
                                            ast::statement::simple_constraint {
                                                    ast::statement::constraint_kind::not_null,
                                            },
                                            id("n2"),
                                    },
                            },
                    },
            },
    });
    ASSERT_TRUE(std::get_if<statement_result_type>(&r)) << diagnostics();
    expect_no_error();
}

TEST_F(analyze_statement_table_definition_test, constraint_name_duplicate_column) {
    invalid(sql_analyzer_code::constraint_already_exists, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::column_constraint_definition {
                                            ast::statement::key_constraint {
                                                    ast::statement::constraint_kind::primary_key,
                                            },
                                            id("n"),
                                    },
                            },
                    },
                    ast::statement::column_definition {
                            id("c2"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::column_constraint_definition {
                                            ast::statement::simple_constraint {
                                                    ast::statement::constraint_kind::not_null,
                                            },
                                            id("n"),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, constraint_name_duplicate_column_and_table) {
    invalid(sql_analyzer_code::constraint_already_exists, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::column_constraint_definition {
                                            ast::statement::simple_constraint {
                                                    ast::statement::constraint_kind::not_null,
                                            },
                                            id("n"),
                                    },
                            },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    vref(id("c1")),
                                            }
                                    },
                            },
                            id("n"),
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, table_primary_key_column_not_variable) {
    invalid(ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    literal(number("1")),
                                            }
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, table_primary_key_column_not_table_column) {
    auto decl = host_parameters_.add({ "x", ttype::int8 {} });
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    ast::scalar::host_parameter_reference {
                                                            id(":x"),
                                                    },
                                            }
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, table_primary_key_column_duplicated) {
    invalid(sql_analyzer_code::column_already_exists, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::column_definition {
                            id("c2"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::column_definition {
                            id("c3"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    id("c1"),
                                            },
                                            {
                                                    id("c2"),
                                            },
                                            {
                                                    id("c1"),
                                            },
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_table_definition_test, partition) {
    auto r = analyze_statement(context(), partitioned_table(
            { { id("c1") }, { id("c2") } },
            { { id("c1") } }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    auto&& index = primary_index_of(r);
    auto&& table_columns = index.table().columns();
    auto&& partition_keys = index.partition_key_columns();
    ASSERT_EQ(partition_keys.size(), 1);
    EXPECT_EQ(partition_keys[0].column(), table_columns[0]);
    EXPECT_EQ(partition_keys[0].column_name(), "c1");
    EXPECT_FALSE(partition_keys[0].prefix_extractor());
}

TEST_F(analyze_statement_table_definition_test, partition_multiple_columns) {
    auto r = analyze_statement(context(), partitioned_table(
            { { id("c1") }, { id("c2") }, { id("c3") } },
            { { id("c1") }, { id("c2") } }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    auto&& index = primary_index_of(r);
    auto&& table_columns = index.table().columns();
    auto&& partition_keys = index.partition_key_columns();
    ASSERT_EQ(partition_keys.size(), 2);
    EXPECT_EQ(partition_keys[0].column(), table_columns[0]);
    EXPECT_EQ(partition_keys[1].column(), table_columns[1]);
}

TEST_F(analyze_statement_table_definition_test, partition_primary_key_descendant) {
    auto r = analyze_statement(context(), partitioned_table(
            {
                    { id("c1"), ast::common::ordering_specification::desc },
                    { id("c2") },
            },
            { { id("c1") } }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    auto&& index = primary_index_of(r);
    auto&& partition_keys = index.partition_key_columns();
    ASSERT_EQ(partition_keys.size(), 1);
    EXPECT_EQ(partition_keys[0].column(), index.table().columns()[0]);
}

TEST_F(analyze_statement_table_definition_test, partition_absent) {
    auto r = analyze_statement(context(), partitioned_table(
            { { id("c1") } },
            {}));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    auto&& index = primary_index_of(r);
    EXPECT_TRUE(index.partition_key_columns().empty());
}

TEST_F(analyze_statement_table_definition_test, partition_not_prefix) {
    invalid(sql_analyzer_code::invalid_partition_key, partitioned_table(
            { { id("c1") }, { id("c2") } },
            { { id("c2") } }));
}

TEST_F(analyze_statement_table_definition_test, partition_without_primary_key) {
    // NOTE: tables without primary key must be rejected, even if the primary key prefix check implies it
    invalid(sql_analyzer_code::invalid_partition_key, partitioned_table(
            {},
            { { id("c1") } }));
}

TEST_F(analyze_statement_table_definition_test, partition_duplicate_columns) {
    // NOTE: duplicate columns must be rejected, even if the primary key prefix check implies it
    invalid(sql_analyzer_code::invalid_partition_key, partitioned_table(
            { { id("c1") }, { id("c2") } },
            { { id("c1") }, { id("c1") } }));
}

TEST_F(analyze_statement_table_definition_test, partition_empty_elements) {
    auto stmt = partitioned_table(
            { { id("c1") } },
            {});
    stmt.partition().emplace(std::vector<ast::statement::partition_key_element> {});
    invalid(sql_analyzer_code::malformed_syntax, stmt);
}

TEST_F(analyze_statement_table_definition_test, partition_column_with_name) {
    invalid(sql_analyzer_code::invalid_partition_key, partitioned_table(
            { { id("c1") } },
            { ast::statement::partition_key_element { vref(id("c1")), id("x") } }));
}

TEST_F(analyze_statement_table_definition_test, partition_column_not_found) {
    invalid(sql_analyzer_code::variable_not_found, partitioned_table(
            { { id("c1") }, { id("c2") } },
            { { id("x") } }));
}


TEST_F(analyze_statement_table_definition_test, partition_virtual_column_extract_timestamp) {
    declare_function("extract_year_to_month", ttype::date {}, ttype::time_point {});
    auto r = analyze_statement(context(), virtual_column_table(
            ast::type::datetime { ast::type::kind::timestamp },
            {
                    { id("k") },
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::year_to_month, vref(id("v")) }),
            }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    namespace p = ::yugawara::storage::partition;
    auto&& key = virtual_key_of(r);
    EXPECT_EQ(key.column(), primary_index_of(r).table().columns()[1]);
    EXPECT_EQ(key.column_name(), "x");
    EXPECT_EQ(key.prefix_extractor(), p::prefix_extractor { p::timestamp_truncate { p::timestamp_unit::month } });
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_extract_date) {
    declare_function("extract_year_to_day", ttype::date {}, ttype::date {});
    auto r = analyze_statement(context(), virtual_column_table(
            ast::type::simple { ast::type::kind::date },
            {
                    { id("k") },
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::year_to_day, vref(id("v")) }),
            }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    namespace p = ::yugawara::storage::partition;
    EXPECT_EQ(virtual_key_of(r).prefix_extractor(), p::prefix_extractor { p::date_truncate { p::date_unit::day } });
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_extract_date_date) {
    // NOTE: EXTRACT(DATE FROM ...) is a synonym of EXTRACT(YEAR TO DAY FROM ...)
    declare_function("date", ttype::date {}, ttype::date {});
    auto r = analyze_statement(context(), virtual_column_table(
            ast::type::simple { ast::type::kind::date },
            {
                    { id("k") },
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::date, vref(id("v")) }),
            }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    namespace p = ::yugawara::storage::partition;
    EXPECT_EQ(virtual_key_of(r).prefix_extractor(), p::prefix_extractor { p::date_truncate { p::date_unit::day } });
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_extract_timestamp_date) {
    declare_function("date", ttype::date {}, ttype::time_point {});
    auto r = analyze_statement(context(), virtual_column_table(
            ast::type::datetime { ast::type::kind::timestamp },
            {
                    { id("k") },
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::date, vref(id("v")) }),
            }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    namespace p = ::yugawara::storage::partition;
    EXPECT_EQ(virtual_key_of(r).prefix_extractor(), p::prefix_extractor { p::timestamp_truncate { p::timestamp_unit::day } });
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_extract_year) {
    declare_function("extract_year", ttype::int4 {}, ttype::time_point {});
    auto r = analyze_statement(context(), virtual_column_table(
            ast::type::datetime { ast::type::kind::timestamp },
            {
                    { id("k") },
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::year, vref(id("v")) }),
            }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    namespace p = ::yugawara::storage::partition;
    EXPECT_EQ(virtual_key_of(r).prefix_extractor(), p::prefix_extractor { p::timestamp_truncate { p::timestamp_unit::year } });
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_extract_date_hour) {
    // NOTE: reported as the plain SQL expression, before checking the virtual column
    declare_function("extract_year_to_hour", ttype::time_point {}, ttype::time_point {});
    invalid(sql_analyzer_code::function_not_found, virtual_column_table(
            ast::type::simple { ast::type::kind::date },
            {
                    { id("k") },
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::year_to_hour, vref(id("v")) }),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_extract_with_time_zone) {
    declare_function("extract_year_to_month", ttype::date {}, ttype::time_point { ttype::with_time_zone });
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::datetime { ast::type::kind::timestamp, true },
            {
                    { id("k") },
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::year_to_month, vref(id("v")) }),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_extract_unsupported_field) {
    declare_function("extract_month", ttype::int4 {}, ttype::time_point {});
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::datetime { ast::type::kind::timestamp },
            {
                    { id("k") },
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::month, vref(id("v")) }),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_substring_character) {
    declare_function("substring", ttype::character { ttype::varying }, ttype::character { ttype::varying }, ttype::int8 {}, ttype::int8 {});
    auto r = analyze_statement(context(), virtual_column_table(
            ast::type::character_string { ast::type::kind::character_varying, 20 },
            {
                    { id("k") },
                    virtual_column(ast::scalar::builtin_function_invocation {
                            ast::scalar::builtin_function_kind::substring,
                            { vref(id("v")), literal(number("1")), literal(number("3")) },
                    }),
            }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    namespace p = ::yugawara::storage::partition;
    EXPECT_EQ(virtual_key_of(r).prefix_extractor(), p::prefix_extractor { p::character_prefix { 3 } });
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_substring_binary) {
    declare_function("substring", ttype::octet { ttype::varying }, ttype::octet { ttype::varying }, ttype::int8 {}, ttype::int8 {});
    auto r = analyze_statement(context(), virtual_column_table(
            ast::type::octet_string { ast::type::kind::octet_varying, 20 },
            {
                    { id("k") },
                    virtual_column(ast::scalar::builtin_function_invocation {
                            ast::scalar::builtin_function_kind::substring,
                            { vref(id("v")), literal(number("1")), literal(number("3")) },
                    }),
            }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    namespace p = ::yugawara::storage::partition;
    EXPECT_EQ(virtual_key_of(r).prefix_extractor(), p::prefix_extractor { p::binary_prefix { 3 } });
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_substring_start) {
    declare_function("substring", ttype::character { ttype::varying }, ttype::character { ttype::varying }, ttype::int8 {}, ttype::int8 {});
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::character_string { ast::type::kind::character_varying, 20 },
            {
                    { id("k") },
                    virtual_column(ast::scalar::builtin_function_invocation {
                            ast::scalar::builtin_function_kind::substring,
                            { vref(id("v")), literal(number("2")), literal(number("3")) },
                    }),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_floor_division) {
    declare_function("floor_div", ttype::int8 {}, ttype::int8 {}, ttype::int8 {});
    auto r = analyze_statement(context(), virtual_column_table(
            ast::type::simple { ast::type::kind::big_integer },
            {
                    { id("k") },
                    virtual_column(ast::scalar::function_invocation {
                            id("floor_div"),
                            { vref(id("v")), literal(number("1000")) },
                    }),
            }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    namespace p = ::yugawara::storage::partition;
    EXPECT_EQ(virtual_key_of(r).prefix_extractor(), p::prefix_extractor { p::floor_division { 1000 } });
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_floor_division_function_name) {
    options_.floor_division_function_name() = "fdiv";
    declare_function("fdiv", ttype::int8 {}, ttype::int8 {}, ttype::int8 {});
    auto r = analyze_statement(context(), virtual_column_table(
            ast::type::simple { ast::type::kind::big_integer },
            {
                    { id("k") },
                    virtual_column(ast::scalar::function_invocation {
                            id("fdiv"),
                            { vref(id("v")), literal(number("10")) },
                    }),
            }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    namespace p = ::yugawara::storage::partition;
    EXPECT_EQ(virtual_key_of(r).prefix_extractor(), p::prefix_extractor { p::floor_division { 10 } });
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_floor_division_user_function) {
    next_function_id_ = ::yugawara::function::declaration::minimum_user_function_id;
    declare_function("floor_div", ttype::int8 {}, ttype::int8 {}, ttype::int8 {});
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::simple { ast::type::kind::big_integer },
            {
                    { id("k") },
                    virtual_column(ast::scalar::function_invocation {
                            id("floor_div"),
                            { vref(id("v")), literal(number("1000")) },
                    }),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_floor_division_divisor) {
    declare_function("floor_div", ttype::int8 {}, ttype::int8 {}, ttype::int8 {});
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::simple { ast::type::kind::big_integer },
            {
                    { id("k") },
                    virtual_column(ast::scalar::function_invocation {
                            id("floor_div"),
                            { vref(id("v")), literal(number("10", ast::literal::sign::minus)) },
                    }),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_floor_quotient) {
    // NOTE: FLOOR(c / d) is not in the allow list
    declare_function("floor", ttype::int8 {}, ttype::int8 {});
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::simple { ast::type::kind::big_integer },
            {
                    { id("k") },
                    virtual_column(ast::scalar::builtin_function_invocation {
                            ast::scalar::builtin_function_kind::floor,
                            {
                                    ast::scalar::binary_expression {
                                            vref(id("v")),
                                            ast::scalar::binary_operator::solidus,
                                            literal(number("1000")),
                                    },
                            },
                    }),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_floor_fractional) {
    declare_function("floor", ttype::decimal {}, ttype::decimal {}, ttype::int8 {});
    auto r = analyze_statement(context(), virtual_column_table(
            ast::type::decimal { ast::type::kind::decimal, 10, 4 },
            {
                    { id("k") },
                    virtual_column(ast::scalar::builtin_function_invocation {
                            ast::scalar::builtin_function_kind::floor,
                            { vref(id("v")), literal(number("2")) },
                    }),
            }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    namespace p = ::yugawara::storage::partition;
    EXPECT_EQ(virtual_key_of(r).prefix_extractor(), p::prefix_extractor { p::floor_fractional { 2 } });
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_floor_fractional_single_argument) {
    declare_function("floor", ttype::decimal {}, ttype::decimal {});
    auto r = analyze_statement(context(), virtual_column_table(
            ast::type::decimal { ast::type::kind::decimal, 10, 2 },
            {
                    { id("k") },
                    virtual_column(ast::scalar::builtin_function_invocation {
                            ast::scalar::builtin_function_kind::floor,
                            { vref(id("v")) },
                    }),
            }));
    ASSERT_TRUE(std::holds_alternative<statement_result_type>(r)) << diagnostics();
    expect_no_error();

    namespace p = ::yugawara::storage::partition;
    EXPECT_EQ(virtual_key_of(r).prefix_extractor(), p::prefix_extractor { p::floor_fractional { 0 } });
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_floor_fractional_integer) {
    // NOTE: the function is found, but the prefix extractor is not applicable to the integral types
    declare_function("floor", ttype::int4 {}, ttype::int4 {}, ttype::int8 {});
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::simple { ast::type::kind::integer },
            {
                    { id("k") },
                    virtual_column(ast::scalar::builtin_function_invocation {
                            ast::scalar::builtin_function_kind::floor,
                            { vref(id("v")), literal(number("2")) },
                    }),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_floor_fractional_integer_single_argument) {
    declare_function("floor", ttype::int4 {}, ttype::int4 {});
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::simple { ast::type::kind::integer },
            {
                    { id("k") },
                    virtual_column(ast::scalar::builtin_function_invocation {
                            ast::scalar::builtin_function_kind::floor,
                            { vref(id("v")) },
                    }),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_unsupported_function) {
    declare_function("upper", ttype::character { ttype::varying }, ttype::character { ttype::varying });
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::character_string { ast::type::kind::character_varying, 20 },
            {
                    { id("k") },
                    virtual_column(ast::scalar::builtin_function_invocation {
                            ast::scalar::builtin_function_kind::upper,
                            { vref(id("v")) },
                    }),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_not_last) {
    declare_function("extract_year", ttype::int4 {}, ttype::time_point {});
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::datetime { ast::type::kind::timestamp },
            {
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::year, vref(id("v")) }),
                    { id("k") },
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_source_not_primary_key) {
    declare_function("extract_year", ttype::int4 {}, ttype::time_point {});
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::datetime { ast::type::kind::timestamp },
            {
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::year, vref(id("v")) }),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_name_conflict) {
    declare_function("extract_year", ttype::int4 {}, ttype::time_point {});
    invalid(sql_analyzer_code::column_already_exists, virtual_column_table(
            ast::type::datetime { ast::type::kind::timestamp },
            {
                    { id("k") },
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::year, vref(id("v")) }, "w"),
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_without_name) {
    declare_function("extract_year", ttype::int4 {}, ttype::time_point {});
    invalid(sql_analyzer_code::invalid_partition_key, virtual_column_table(
            ast::type::datetime { ast::type::kind::timestamp },
            {
                    { id("k") },
                    ast::statement::partition_key_element {
                            ast::scalar::extract_expression { ast::scalar::extract_field_kind::year, vref(id("v")) },
                    },
            }));
}

TEST_F(analyze_statement_table_definition_test, partition_virtual_column_inconsistent_function_type) {
    // NOTE: the function declaration does not follow the typing rules of the prefix extractors
    declare_function("extract_year", ttype::int8 {}, ttype::time_point {});
    invalid(sql_analyzer_code::inconsistent_function_type, virtual_column_table(
            ast::type::datetime { ast::type::kind::timestamp },
            {
                    { id("k") },
                    virtual_column(ast::scalar::extract_expression { ast::scalar::extract_field_kind::year, vref(id("v")) }),
            }));
}

} // namespace mizugaki::analyzer::details
