#include <mizugaki/analyzer/sql_analyzer.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/type/table.h>

#include <takatori/relation/emit.h>

#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/table/apply.h>
#include <mizugaki/ast/table/subquery.h>
#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/query/select_asterisk.h>

#include <mizugaki/ast/statement/insert_statement.h>
#include <mizugaki/ast/statement/select_statement.h>

#include "details/test_parent.h"

namespace mizugaki::analyzer {

using namespace testing;

class sql_analyzer_test : public details::test_parent {
};

TEST_F(sql_analyzer_test, insert_charcter_into_int_column) {
    install_table("t");
    sql_analyzer analyzer;
    auto result = analyzer(
            options_,
            ast::statement::insert_statement {
                    id("t"),
                    {
                            id("k"),
                    },
                    ast::query::table_value_constructor {
                            ast::scalar::value_constructor {
                                    literal(string("'p'"))
                            },
                    },
            }
    );
    ASSERT_TRUE(result) << diagnostics();
}

TEST_F(sql_analyzer_test, table_subquery_self_join) {
    install_table("t1");
    install_table("t2");

    sql_analyzer analyzer;
    auto result = analyzer(
            options_,
            // SELECT * FROM (SELECT * FROM t1, t1) t1, (SELECT * FROM t2) t2;
            ast::statement::select_statement {
                    ast::query::query {
                            {
                                    ast::query::select_asterisk {},
                            },
                            {
                                    ast::table::subquery {
                                            ast::query::query {
                                                    {
                                                            ast::query::select_asterisk {},
                                                    },
                                                    {
                                                            ast::table::table_reference { id("t1") },
                                                            ast::table::table_reference { id("t1") },
                                                    },
                                            },
                                            id("t1")
                                    },
                                    ast::table::subquery {
                                            ast::query::query {
                                                    {
                                                            ast::query::select_asterisk {},
                                                    },
                                                    {
                                                            ast::table::table_reference { id("t2") },
                                                    },
                                            },
                                            id("t2")
                                    },
                            },
                    }
            }
    );
    ASSERT_TRUE(result) << diagnostics();
    auto graph = result.release<sql_analyzer_result_kind::execution_plan>();

    auto&& emit = *find_last<trelation::emit>(*graph);
    ASSERT_EQ(emit.columns().size(), 12);
}

TEST_F(sql_analyzer_test, table_subquery_self_join_2) {
    install_table("t1");
    install_table("t2");

    sql_analyzer analyzer;
    auto result = analyzer(
            options_,
            // SELECT t1.*, t1.* FROM (SELECT * FROM t1, t1) t1, (SELECT * FROM t2) t2;
            ast::statement::select_statement {
                    ast::query::query {
                            {
                                    ast::query::select_asterisk { vref(id("t1")) },
                                    ast::query::select_asterisk { vref(id("t1")) },
                            },
                            {
                                    ast::table::subquery {
                                            ast::query::query {
                                                    {
                                                            ast::query::select_asterisk {},
                                                    },
                                                    {
                                                            ast::table::table_reference { id("t1") },
                                                            ast::table::table_reference { id("t1") },
                                                    },
                                            },
                                            id("t1")
                                    },
                                    ast::table::subquery {
                                            ast::query::query {
                                                    {
                                                            ast::query::select_asterisk {},
                                                    },
                                                    {
                                                            ast::table::table_reference { id("t2") },
                                                    },
                                            },
                                            id("t2")
                                    },
                            },
                    }
            }
    );
    ASSERT_TRUE(result) << diagnostics();
    auto graph = result.release<sql_analyzer_result_kind::execution_plan>();

    auto&& emit = *find_last<trelation::emit>(*graph);
    ASSERT_EQ(emit.columns().size(), 16);
}

TEST_F(sql_analyzer_test, table_subquery_self_join_3) {
    install_table("t1");
    sql_analyzer analyzer;
    auto result = analyzer(
            options_,
            // SELECT * FROM (SELECT *, * FROM t1, t1) t
            ast::statement::select_statement {
                    ast::query::query {
                            {
                                    ast::query::select_asterisk {},
                            },
                            {
                                    ast::table::subquery {
                                            ast::query::query {
                                                    {
                                                            ast::query::select_asterisk {},
                                                            ast::query::select_asterisk {},
                                                    },
                                                    {
                                                            ast::table::table_reference { id("t1") },
                                                            ast::table::table_reference { id("t1") },
                                                    },
                                            },
                                            id("t")
                                    },
                            },
                    }
            }
    );
    ASSERT_TRUE(result) << diagnostics();
    auto graph = result.release<sql_analyzer_result_kind::execution_plan>();

    auto&& emit = *find_last<trelation::emit>(*graph);
    ASSERT_EQ(emit.columns().size(), 16);
}

TEST_F(sql_analyzer_test, apply_walkthrough) {
    /*
     * -- t: table(k:int4, v:varchar, ...)
     * -- f: (int8) -> table(c0:int4, c1:varchar)
     * SELECT *
     * FROM t
     * APPLY f(t.k) x
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                {
                    { "c0", ttype::int4 {} },
                    { "c1", ttype::character { ttype::varying, {} } },
                },
            },
            {
                ttype::int8 {},
            },
            { ::yugawara::function::function_feature::table_valued_function },
    });
    sql_analyzer analyzer;
    auto result = analyzer(
            options_,
            // SELECT * FROM t1 APPLY f(t1.k) x;
            ast::statement::select_statement {
                ast::query::query {
                        {
                                ast::query::select_asterisk {},
                        },
                        {
                                ast::table::apply {
                                        ast::table::table_reference {
                                            id("t"),
                                        },
                                        id("f"),
                                        {
                                                vref(id("t", "k")),
                                        },
                                        {
                                                id("x"),
                                        },
                                },
                        },
                },
            }
    );
    ASSERT_TRUE(result) << diagnostics();
    auto graph = result.release<sql_analyzer_result_kind::execution_plan>();

    auto&& emit = *find_last<trelation::emit>(*graph);
    ASSERT_EQ(emit.columns().size(), 6); // t.k, t.v, t.w, t.x, x.c0, x.c1
}

} // namespace mizugaki::analyzer
