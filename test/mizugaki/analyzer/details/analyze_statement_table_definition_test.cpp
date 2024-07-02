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
using ::yugawara::binding::extract_if;

class analyze_statement_table_definition_test : public test_parent {};

TEST_F(analyze_statement_table_definition_test, table_definition_simple) {
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
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

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

    auto&& key = extract_if<::yugawara::storage::index>(stmt.primary_key());
    ASSERT_TRUE(key);
    EXPECT_EQ(key->simple_name(), "");
    ASSERT_EQ(key->keys().size(), 0);
    EXPECT_EQ(key->features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

TEST_F(analyze_statement_table_definition_test, table_definition_column_multiple) {
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
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

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

TEST_F(analyze_statement_table_definition_test, table_definition_column_primary_key) {
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
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

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

    auto&& key = extract_if<::yugawara::storage::index>(stmt.primary_key());
    ASSERT_TRUE(key);
    EXPECT_EQ(key->simple_name(), "");
    auto&& key_columns = key->keys();
    ASSERT_EQ(key_columns.size(), 1);
    {
        auto&& column = key_columns[0];
        EXPECT_EQ(&column.column(), &table_columns[0]);
        EXPECT_EQ(column.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    EXPECT_EQ(key->features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

TEST_F(analyze_statement_table_definition_test, table_definition_column_null) {
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
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

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

TEST_F(analyze_statement_table_definition_test, table_definition_column_not_null) {
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
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

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

TEST_F(analyze_statement_table_definition_test, table_definition_column_default) {
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
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

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

TEST_F(analyze_statement_table_definition_test, table_definition_table_primary_key) {
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
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

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

    auto&& key = extract_if<::yugawara::storage::index>(stmt.primary_key());
    ASSERT_TRUE(key);
    EXPECT_EQ(key->simple_name(), "");
    auto&& key_columns = key->keys();
    ASSERT_EQ(key_columns.size(), 1);
    {
        auto&& column = key_columns[0];
        EXPECT_EQ(&column.column(), &table_columns[0]);
        EXPECT_EQ(column.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    EXPECT_EQ(key->features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

} // namespace mizugaki::analyzer::details
