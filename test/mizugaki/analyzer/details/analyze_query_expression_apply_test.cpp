#include <mizugaki/analyzer/details/analyze_query_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>
#include <takatori/type/decimal.h>
#include <takatori/type/table.h>

#include <takatori/value/decimal.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/apply.h>
#include <takatori/relation/project.h>

#include <yugawara/binding/extract.h>

#include <yugawara/function/declaration.h>

#include <mizugaki/ast/table/apply.h>
#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/select_column.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_query_expression_apply_test : public test_parent {
protected:
    void invalid(ast::query::expression const& expression) {
        trelation::graph_type graph {};
        auto r = analyze_query_expression(
                context(),
                graph,
                expression,
                {},
                {});
        EXPECT_FALSE(r);
        EXPECT_NE(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::query::expression const& expression) {
        invalid(expression);
        EXPECT_TRUE(find_error(code)) << diagnostics();
    }
};

TEST_F(analyze_query_expression_apply_test, simple) {
    /*
     * -- t: table(k:int8, v:varchar, w:varchar, x:varchar)
     * -- f: () -> table(c0:int4)
     * SELECT t.k, x.c0
     * FROM t
     * APPLY f() x
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                {
                    { "c0", ttype::int4 {} },
                },
            },
            {},
            { ::yugawara::function::function_feature::table_valued_function },
    });
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("t", "k")) },
                            ast::query::select_column { vref(id("x", "c0")) },
                    },
                    {
                            ast::table::apply {
                                    ast::table::table_reference {
                                        id("t"),
                                    },
                                    id("f"),
                                    {},
                                    {
                                            id("x"),
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    /*
     * scan t: (t.k, t.v, t.w, t.x)
     * apply f()
     * project t.k, x.c0
     */
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);
    auto&& r0 = relation_columns[0];
    auto&& r1 = relation_columns[1];

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& apply = *find_prev<trelation::apply>(project);
    auto&& scan = *find_prev<trelation::scan>(apply);

    ASSERT_EQ(scan.columns().size(), 4);
    auto t_k = scan.columns()[0].destination();

    EXPECT_EQ(apply.operator_kind(), trelation::apply_kind::cross);
    EXPECT_EQ(extract(apply.function()), *function);
    ASSERT_EQ(apply.arguments().size(), 0);
    ASSERT_EQ(apply.columns().size(), 1);

    EXPECT_EQ(apply.columns()[0].position(), 0);
    auto x_c0 = apply.columns()[0].variable();

    ASSERT_EQ(project.columns().size(), 2);
    EXPECT_EQ(project.columns()[0].value(), vref(t_k));
    EXPECT_EQ(project.columns()[0].variable(), r0.variable());
    EXPECT_EQ(project.columns()[1].value(), vref(x_c0));
    EXPECT_EQ(project.columns()[1].variable(), r1.variable());
}

TEST_F(analyze_query_expression_apply_test, cross) {
    /*
     * -- t: table(k:int8, v:varchar, w:varchar, x:varchar)
     * -- f: () -> table(c0:int4)
     * SELECT t.k, x.c0
     * FROM t
     * CROSS APPLY f() x
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                {
                    { "c0", ttype::int4 {} },
                },
            },
            {},
            { ::yugawara::function::function_feature::table_valued_function },
    });
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("t", "k")) },
                            ast::query::select_column { vref(id("x", "c0")) },
                    },
                    {
                            ast::table::apply {
                                    ast::table::table_reference {
                                        id("t"),
                                    },
                                    id("f"),
                                    {},
                                    {
                                            id("x"),
                                    },
                                    ast::table::apply_type::cross,
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    /*
     * scan t: (t.k, t.v, t.w, t.x)
     * apply[cross] f()
     * project t.k, x.c0
     */
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& apply = *find_prev<trelation::apply>(project);

    EXPECT_EQ(apply.operator_kind(), trelation::apply_kind::cross);
}

TEST_F(analyze_query_expression_apply_test, outer) {
    /*
     * -- t: table(k:int8, v:varchar, w:varchar, x:varchar)
     * -- f: () -> table(c0:int4)
     * SELECT t.k, x.c0
     * FROM t
     * OUTER APPLY f() x
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                {
                    { "c0", ttype::int4 {} },
                },
            },
            {},
            { ::yugawara::function::function_feature::table_valued_function },
    });
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("t", "k")) },
                            ast::query::select_column { vref(id("x", "c0")) },
                    },
                    {
                            ast::table::apply {
                                    ast::table::table_reference {
                                        id("t"),
                                    },
                                    id("f"),
                                    {},
                                    {
                                            id("x"),
                                    },
                                    ast::table::apply_type::outer,
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    /*
     * scan t: (t.k, t.v, t.w, t.x)
     * apply[cross] f()
     * project t.k, x.c0
     */
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& apply = *find_prev<trelation::apply>(project);

    EXPECT_EQ(apply.operator_kind(), trelation::apply_kind::outer);
}

TEST_F(analyze_query_expression_apply_test, arguments) {
    /*
     * -- t: table(k:int4, v:varchar, ...)
     * -- f: (int8, varchar, decimal) -> table(c0:int4)
     * SELECT x.c0
     * FROM t
     * APPLY f(t.k, t.v, 3.14) x
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                {
                    { "c0", ttype::int4 {} },
                },
            },
            {
                ttype::int8 {},
                ttype::character { ttype::varying, {} },
                ttype::decimal { {}, {} },
            },
            { ::yugawara::function::function_feature::table_valued_function },
    });
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("x", "c0")) },
                    },
                    {
                            ast::table::apply {
                                    ast::table::table_reference {
                                        id("t"),
                                    },
                                    id("f"),
                                    {
                                            vref(id("t", "k")),
                                            vref(id("t", "v")),
                                            literal(number("3.14")),
                                    },
                                    {
                                            id("x"),
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    /*
     * scan t: (t.k, t.v, t.w, t.x)
     * apply f(t.k, t.v, 3.14)
     * project x.c0
     */
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& apply = *find_prev<trelation::apply>(project);
    auto&& scan = *find_prev<trelation::scan>(apply);

    ASSERT_EQ(scan.columns().size(), 4);
    auto t_k = scan.columns()[0].destination();
    auto t_v = scan.columns()[1].destination();

    ASSERT_EQ(apply.arguments().size(), 3);
    EXPECT_EQ(apply.arguments()[0], vref(t_k));
    EXPECT_EQ(apply.arguments()[1], vref(t_v));
    EXPECT_EQ(apply.arguments()[2], tscalar::immediate(tvalue::decimal {{ 314, -2 }}, ttype::decimal { {}, 2 }));
}

TEST_F(analyze_query_expression_apply_test, multiple_columns) {
    /*
     * -- t: table(k:int8, v:varchar, w:varchar, x:varchar)
     * -- f: () -> table(c0:int4, c1:int4, c2:int4)
     * SELECT t.k, x.c0, x.c1, x.c2
     * FROM t
     * APPLY f() x
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                {
                    { "c0", ttype::int4 {} },
                    { "c1", ttype::int4 {} },
                    { "c2", ttype::int4 {} },
                },
            },
            {},
            { ::yugawara::function::function_feature::table_valued_function },
    });
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("t", "k")) },
                            ast::query::select_column { vref(id("x", "c0")) },
                            ast::query::select_column { vref(id("x", "c1")) },
                            ast::query::select_column { vref(id("x", "c2")) },
                    },
                    {
                            ast::table::apply {
                                    ast::table::table_reference {
                                        id("t"),
                                    },
                                    id("f"),
                                    {},
                                    {
                                            id("x"),
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    /*
     * scan t: (t.k, t.v, t.w, t.x)
     * apply f()
     * project t.k, x.c0, x.c1, x.c2
     */
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 4);
    auto&& r0 = relation_columns[0];
    auto&& r1 = relation_columns[1];
    auto&& r2 = relation_columns[2];
    auto&& r3 = relation_columns[3];

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& apply = *find_prev<trelation::apply>(project);
    auto&& scan = *find_prev<trelation::scan>(apply);

    ASSERT_EQ(scan.columns().size(), 4);
    auto t_k = scan.columns()[0].destination();

    ASSERT_EQ(apply.columns().size(), 3);
    EXPECT_EQ(apply.columns()[0].position(), 0);
    EXPECT_EQ(apply.columns()[1].position(), 1);
    EXPECT_EQ(apply.columns()[2].position(), 2);
    auto x_c0 = apply.columns()[0].variable();
    auto x_c1 = apply.columns()[1].variable();
    auto x_c2 = apply.columns()[2].variable();

    ASSERT_EQ(project.columns().size(), 4);
    EXPECT_EQ(project.columns()[0].value(), vref(t_k));
    EXPECT_EQ(project.columns()[0].variable(), r0.variable());
    EXPECT_EQ(project.columns()[1].value(), vref(x_c0));
    EXPECT_EQ(project.columns()[1].variable(), r1.variable());
    EXPECT_EQ(project.columns()[2].value(), vref(x_c1));
    EXPECT_EQ(project.columns()[2].variable(), r2.variable());
    EXPECT_EQ(project.columns()[3].value(), vref(x_c2));
    EXPECT_EQ(project.columns()[3].variable(), r3.variable());
}

TEST_F(analyze_query_expression_apply_test, column_case) {
    /*
     * -- t: table(k:int8, v:varchar, w:varchar, x:varchar)
     * -- f: () -> table(Col)
     * SELECT x.COL
     * FROM t
     * APPLY f() x
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                {
                    { "Col", ttype::int4 {} },
                },
            },
            {},
            { ::yugawara::function::function_feature::table_valued_function },
    });
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("x", "COL")) },
                    },
                    {
                            ast::table::apply {
                                    ast::table::table_reference {
                                        id("t"),
                                    },
                                    id("f"),
                                    {},
                                    {
                                            id("x"),
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    /*
     * scan t: (t.k, t.v, t.w, t.x)
     * apply f()
     * project x.COL
     */
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    ASSERT_EQ(relation.columns().size(), 1);
    auto&& r0 = relation.columns()[0];

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& apply = *find_prev<trelation::apply>(project);
    auto&& scan = *find_prev<trelation::scan>(apply);
    (void) scan;

    ASSERT_EQ(apply.columns().size(), 1);
    EXPECT_EQ(apply.columns()[0].position(), 0);
    auto x_col = apply.columns()[0].variable();

    ASSERT_EQ(project.columns().size(), 1);
    EXPECT_EQ(project.columns()[0].value(), vref(x_col));
    EXPECT_EQ(project.columns()[0].variable(), r0.variable());
}

TEST_F(analyze_query_expression_apply_test, correlation_columns) {
    /*
     * -- t: table(k:int8, v:varchar, w:varchar, x:varchar)
     * -- f: () -> table(c0:int4)
     * SELECT t.k, x.c0
     * FROM t
     * APPLY f() x(v0)
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                {
                    { "c0", ttype::int4 {} },
                },
            },
            {},
            { ::yugawara::function::function_feature::table_valued_function },
    });
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("t", "k")) },
                            ast::query::select_column { vref(id("x", "v0")) },
                    },
                    {
                            ast::table::apply {
                                    ast::table::table_reference {
                                        id("t"),
                                    },
                                    id("f"),
                                    {},
                                    {
                                            id("x"),
                                            {
                                                    id("v0"),
                                            },
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    /*
     * scan t: (t.k, t.v, t.w, t.x)
     * apply f()
     * project t.k, x.v0
     */
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);
    auto&& r0 = relation_columns[0];
    auto&& r1 = relation_columns[1];

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& apply = *find_prev<trelation::apply>(project);
    auto&& scan = *find_prev<trelation::scan>(apply);

    ASSERT_EQ(scan.columns().size(), 4);
    auto t_k = scan.columns()[0].destination();

    EXPECT_EQ(apply.operator_kind(), trelation::apply_kind::cross);
    EXPECT_EQ(extract(apply.function()), *function);
    ASSERT_EQ(apply.arguments().size(), 0);
    ASSERT_EQ(apply.columns().size(), 1);

    EXPECT_EQ(apply.columns()[0].position(), 0);
    auto x_v0 = apply.columns()[0].variable();

    ASSERT_EQ(project.columns().size(), 2);
    EXPECT_EQ(project.columns()[0].value(), vref(t_k));
    EXPECT_EQ(project.columns()[0].variable(), r0.variable());
    EXPECT_EQ(project.columns()[1].value(), vref(x_v0));
    EXPECT_EQ(project.columns()[1].variable(), r1.variable());
}

TEST_F(analyze_query_expression_apply_test, correlation_column_case) {
    /*
     * -- t: table(k:int8, v:varchar, w:varchar, x:varchar)
     * -- f: () -> table(c0:int4)
     * SELECT x.COL
     * FROM t
     * APPLY f() x(Col)
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                {
                    { "c0", ttype::int4 {} },
                },
            },
            {},
            { ::yugawara::function::function_feature::table_valued_function },
    });
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("x", "COL")) },
                    },
                    {
                            ast::table::apply {
                                    ast::table::table_reference {
                                        id("t"),
                                    },
                                    id("f"),
                                    {},
                                    {
                                            id("x"),
                                            {
                                                    id("COL"),
                                            },
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    /*
     * scan t: (t.k, t.v, t.w, t.x)
     * apply f()
     * project x.COL
     */
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    ASSERT_EQ(relation.columns().size(), 1);
    auto&& r0 = relation.columns()[0];

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& apply = *find_prev<trelation::apply>(project);
    auto&& scan = *find_prev<trelation::scan>(apply);
    (void) scan;

    ASSERT_EQ(apply.columns().size(), 1);
    EXPECT_EQ(apply.columns()[0].position(), 0);
    auto x_col = apply.columns()[0].variable();

    ASSERT_EQ(project.columns().size(), 1);
    EXPECT_EQ(project.columns()[0].value(), vref(x_col));
    EXPECT_EQ(project.columns()[0].variable(), r0.variable());
}

TEST_F(analyze_query_expression_apply_test, invalid_argument) {
    /*
     * -- t: table(k:int8, ...)
     * -- f: (int8) -> table(c0:int4)
     * SELECT x.c0
     * FROM t
     * APPLY f(INVALID) x
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                    {
                            { "c0", ttype::int4 {} },
                    },
            },
            {
                    ttype::int8 {},
            },
            { ::yugawara::function::function_feature::table_valued_function },
    });
    invalid(ast::query::query {
            {
                    ast::query::select_column { vref(id("x", "c0")) },
            },
            {
                    ast::table::apply {
                            ast::table::table_reference {
                                id("t"),
                            },
                            id("f"),
                            {
                                    vref(id("INVALID"))
                            },
                            {
                                    id("x"),
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_apply_test, invalid_function_name) {
    /*
     * -- t: table(k:int8, ...)
     * -- f: (int8) -> table(c0:int4)
     * SELECT x.c0
     * FROM t
     * APPLY INVALID(t.k) x
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                    {
                            { "c0", ttype::int4 {} },
                    },
            },
            {
                    ttype::int8 {},
            },
            { ::yugawara::function::function_feature::table_valued_function },
    });
    invalid(sql_analyzer_code::function_not_found, ast::query::query {
            {
                    ast::query::select_column { vref(id("x", "c0")) },
            },
            {
                    ast::table::apply {
                            ast::table::table_reference {
                                id("t"),
                            },
                            id("INVALID"),
                            {
                                    vref(id("k"))
                            },
                            {
                                    id("x"),
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_apply_test, invalid_argument_type) {
    /*
     * -- t: table(k:int8, ...)
     * -- f: (int8) -> table(c0:int4)
     * SELECT x.c0
     * FROM t
     * APPLY f(t.x) x -- apply varchar to int8
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                    {
                            { "c0", ttype::int4 {} },
                    },
            },
            {
                    ttype::int8 {},
            },
            { ::yugawara::function::function_feature::table_valued_function },
    });
    invalid(sql_analyzer_code::function_not_found, ast::query::query {
            {
                    ast::query::select_column { vref(id("x", "c0")) },
            },
            {
                    ast::table::apply {
                            ast::table::table_reference {
                                id("t"),
                            },
                            id("f"),
                            {
                                    vref(id("x"))
                            },
                            {
                                    id("x"),
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_apply_test, ambiguous_function_overload) {
    /*
     * -- t: table(k:int8, ...)
     * -- f: (int8) -> table(c0:int4) (duplicated)
     * SELECT x.c0
     * FROM t
     * APPLY f(t.k) x -- ambiguous
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                    {
                            { "c0", ttype::int4 {} },
                    },
            },
            {
                    ttype::int8 {},
            },
            { ::yugawara::function::function_feature::table_valued_function },
    });
    auto function_duplicate = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 2,
            "f",
            ttype::table {
                    {
                            { "c0", ttype::int4 {} },
                    },
            },
            {
                    ttype::int8 {},
            },
            { ::yugawara::function::function_feature::table_valued_function },
    });
    invalid(sql_analyzer_code::function_ambiguous, ast::query::query {
            {
                    ast::query::select_column { vref(id("x", "c0")) },
            },
            {
                    ast::table::apply {
                            ast::table::table_reference {
                                id("t"),
                            },
                            id("f"),
                            {
                                    vref(id("t", "k"))
                            },
                            {
                                    id("x"),
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_apply_test, invalid_function_type) {
    /*
     * -- t: table(k:int8, ...)
     * -- f: (int8) -> table(c0:int4) (not a table-valued function)
     * SELECT x.c0
     * FROM t
     * APPLY f(t.k) x -- not found
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                    {
                            { "c0", ttype::int4 {} },
                    },
            },
            {
                    ttype::int8 {},
            },
            { ::yugawara::function::function_feature::scalar_function },
    });
    invalid(sql_analyzer_code::function_not_found, ast::query::query {
            {
                    ast::query::select_column { vref(id("x", "c0")) },
            },
            {
                    ast::table::apply {
                            ast::table::table_reference {
                                id("t"),
                            },
                            id("f"),
                            {
                                    vref(id("t", "k"))
                            },
                            {
                                    id("x"),
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_apply_test, invalid_function_return_type) {
    /*
     * -- t: table(k:int8, ...)
     * -- f: (int8) -> int4 (not a table return type)
     * SELECT x.c0
     * FROM t
     * APPLY f(t.k) x
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::int4 {},
            {
                    ttype::int8 {},
            },
            { ::yugawara::function::function_feature::table_valued_function },
    });
    invalid(sql_analyzer_code::inconsistent_type, ast::query::query {
            {
                    ast::query::select_column { vref(id("x", "c0")) },
            },
            {
                    ast::table::apply {
                            ast::table::table_reference {
                                id("t"),
                            },
                            id("f"),
                            {
                                    vref(id("t", "k"))
                            },
                            {
                                    id("x"),
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_apply_test, invalid_empty_table_type) {
    /*
     * -- t: table(k:int8, ...)
     * -- f: (int8) -> table() -- empty table type
     * SELECT x.c0
     * FROM t
     * APPLY f(t.k) x
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                    {}, // empty columns
            },
            {
                    ttype::int8 {},
            },
            { ::yugawara::function::function_feature::table_valued_function },
    });
    invalid(sql_analyzer_code::inconsistent_type, ast::query::query {
            {
                    ast::query::select_column { vref(id("x", "c0")) },
            },
            {
                    ast::table::apply {
                            ast::table::table_reference {
                                id("t"),
                            },
                            id("f"),
                            {
                                    vref(id("t", "k"))
                            },
                            {
                                    id("x"),
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_apply_test, invalid_correlation_columns) {
    /*
     * -- t: table(k:int8, ...)
     * -- f: (int8) -> table(c0:int4)
     * SELECT x.c0
     * FROM t
     * APPLY f(t.k) x(r0, r1, r2) -- too many correlation columns
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                    {
                            { "c0", ttype::int4 {} },
                    },
            },
            {
                    ttype::int8 {},
            },
            { ::yugawara::function::function_feature::table_valued_function },
    });
    invalid(sql_analyzer_code::inconsistent_columns, ast::query::query {
            {
                    ast::query::select_column { vref(id("x", "c0")) },
            },
            {
                    ast::table::apply {
                            ast::table::table_reference {
                                id("t"),
                            },
                            id("f"),
                            {
                                    vref(id("t", "k"))
                            },
                            {
                                    id("x"),
                                    {
                                            id("r0"),
                                            id("r1"),
                                            id("r2"),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_apply_test, invalid_correlation_columns_conflict) {
    /*
     * -- t: table(k:int8, ...)
     * -- f: (int8) -> table(c0:int4, c1:int4, c2:int4)
     * SELECT x.c0
     * FROM t
     * APPLY f(t.k) x(r0, r1, r0) -- conflict correlation column names
     */
    auto table = install_table("t");
    auto function = functions_->add({
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "f",
            ttype::table {
                    {
                            { "c0", ttype::int4 {} },
                            { "c1", ttype::int4 {} },
                            { "c2", ttype::int4 {} },
                    },
            },
            {
                    ttype::int8 {},
            },
            { ::yugawara::function::function_feature::table_valued_function },
    });
    invalid(sql_analyzer_code::column_already_exists, ast::query::query {
            {
                    ast::query::select_column { vref(id("x", "c0")) },
            },
            {
                    ast::table::apply {
                            ast::table::table_reference {
                                id("t"),
                            },
                            id("f"),
                            {
                                    vref(id("t", "k"))
                            },
                            {
                                    id("x"),
                                    {
                                            id("r0"),
                                            id("r1"),
                                            id("R0"),
                                    },
                            },
                    },
            },
    });
}

} // namespace mizugaki::analyzer::details
