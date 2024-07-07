#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/create_table.h>

#include <yugawara/binding/extract.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>
#include <yugawara/storage/column.h>

#include <mizugaki/ast/type/simple.h>
#include <mizugaki/ast/type/user_defined.h>

#include <mizugaki/ast/scalar/unary_expression.h>
#include <mizugaki/ast/scalar/host_parameter_reference.h>

#include <mizugaki/ast/statement/table_definition.h>
#include <mizugaki/ast/statement/column_definition.h>
#include <mizugaki/ast/statement/table_constraint_definition.h>
#include <mizugaki/ast/statement/simple_constraint.h>
#include <mizugaki/ast/statement/key_constraint.h>
#include <mizugaki/ast/statement/expression_constraint.h>

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
    }

    auto&& key = extract<::yugawara::storage::index>(stmt.primary_key());
    EXPECT_EQ(key.simple_name(), "");
    ASSERT_EQ(key.keys().size(), 0);
    EXPECT_EQ(key.features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
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

} // namespace mizugaki::analyzer::details
