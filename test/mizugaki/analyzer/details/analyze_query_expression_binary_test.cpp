#include <mizugaki/analyzer/details/analyze_query_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/values.h>
#include <takatori/relation/project.h>
#include <takatori/relation/intermediate/union.h>

#include <yugawara/binding/extract.h>

#include <mizugaki/ast/type/simple.h>

#include <mizugaki/ast/scalar/cast_expression.h>
#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/query/binary_expression.h>
#include <mizugaki/ast/query/select_column.h>
#include <mizugaki/ast/query/select_asterisk.h>

#include <mizugaki/ast/table/table_reference.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_query_expression_binary_test : public test_parent {
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
        clear_error();
    }
};

TEST_F(analyze_query_expression_binary_test, union_all) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::binary_expression {
                    ast::query::query {
                            {
                                    ast::query::select_asterisk {},
                            },
                            {
                                    ast::table::table_reference {
                                            id("a"),
                                    },
                            },
                    },
                    ast::query::binary_operator::union_,
                    ast::query::set_quantifier::all,
                    ast::query::query {
                            {
                                    ast::query::select_asterisk {},
                            },
                            {
                                    ast::table::table_reference {
                                            id("b"),
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 5);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_FALSE(relation.declaration());
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 4);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), "k");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[1];
        EXPECT_EQ(column.identifier(), "v");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[2];
        EXPECT_EQ(column.identifier(), "w");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[3];
        EXPECT_EQ(column.identifier(), "x");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }

    // (scan - project)x2 - union[all]
    auto&& union_ = downcast<trelation::intermediate::union_>(r.output().owner());
    auto&& left_project = *find_prev<trelation::project>(union_.left());
    auto&& left_scan = *find_prev<trelation::scan>(left_project);
    auto&& right_project = *find_prev<trelation::project>(union_.right());
    auto&& right_scan = *find_prev<trelation::scan>(right_project);

    ASSERT_EQ(left_project.columns().size(), 0);

    EXPECT_EQ(extract<::yugawara::storage::index>(left_scan.source()).table(), *table_a);
    auto&& left_columns = left_scan.columns();
    ASSERT_EQ(left_columns.size(), 4);

    ASSERT_EQ(right_project.columns().size(), 0);

    EXPECT_EQ(extract<::yugawara::storage::index>(right_scan.source()).table(), *table_b);
    auto&& right_columns = right_scan.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(union_.quantifier(), trelation::set_quantifier::all);
    auto&& union_columns = union_.mappings();
    ASSERT_EQ(union_columns.size(), 4);
    {
        auto&& column = union_columns[0];
        EXPECT_EQ(column.left(), left_columns[0].destination());
        EXPECT_EQ(column.right(), right_columns[0].destination());
    }
    {
        auto&& column = union_columns[1];
        EXPECT_EQ(column.left(), left_columns[1].destination());
        EXPECT_EQ(column.right(), right_columns[1].destination());
    }
    {
        auto&& column = union_columns[2];
        EXPECT_EQ(column.left(), left_columns[2].destination());
        EXPECT_EQ(column.right(), right_columns[2].destination());
    }
    {
        auto&& column = union_columns[3];
        EXPECT_EQ(column.left(), left_columns[3].destination());
        EXPECT_EQ(column.right(), right_columns[3].destination());
    }

    EXPECT_EQ(union_columns[0].destination(), relation_columns[0].variable());
    EXPECT_EQ(union_columns[1].destination(), relation_columns[1].variable());
    EXPECT_EQ(union_columns[2].destination(), relation_columns[2].variable());
    EXPECT_EQ(union_columns[3].destination(), relation_columns[3].variable());
}

TEST_F(analyze_query_expression_binary_test, union_distinct) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::binary_expression {
                    ast::query::query {
                            {
                                    ast::query::select_asterisk {},
                            },
                            {
                                    ast::table::table_reference {
                                            id("a"),
                                    },
                            },
                    },
                    ast::query::binary_operator::union_,
                    ast::query::set_quantifier::distinct,
                    ast::query::query {
                            {
                                    ast::query::select_asterisk {},
                            },
                            {
                                    ast::table::table_reference {
                                            id("b"),
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 5);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_FALSE(relation.declaration());
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 4);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), "k");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[1];
        EXPECT_EQ(column.identifier(), "v");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[2];
        EXPECT_EQ(column.identifier(), "w");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[3];
        EXPECT_EQ(column.identifier(), "x");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }

    // (scan - project)x2 - union[all]
    auto&& union_ = downcast<trelation::intermediate::union_>(r.output().owner());
    auto&& left_project = *find_prev<trelation::project>(union_.left());
    auto&& left_scan = *find_prev<trelation::scan>(left_project);
    auto&& right_project = *find_prev<trelation::project>(union_.right());
    auto&& right_scan = *find_prev<trelation::scan>(right_project);

    ASSERT_EQ(left_project.columns().size(), 0);

    EXPECT_EQ(extract<::yugawara::storage::index>(left_scan.source()).table(), *table_a);
    auto&& left_columns = left_scan.columns();
    ASSERT_EQ(left_columns.size(), 4);

    ASSERT_EQ(right_project.columns().size(), 0);

    EXPECT_EQ(extract<::yugawara::storage::index>(right_scan.source()).table(), *table_b);
    auto&& right_columns = right_scan.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(union_.quantifier(), trelation::set_quantifier::distinct);
    auto&& union_columns = union_.mappings();
    ASSERT_EQ(union_columns.size(), 4);
    {
        auto&& column = union_columns[0];
        EXPECT_EQ(column.left(), left_columns[0].destination());
        EXPECT_EQ(column.right(), right_columns[0].destination());
    }
    {
        auto&& column = union_columns[1];
        EXPECT_EQ(column.left(), left_columns[1].destination());
        EXPECT_EQ(column.right(), right_columns[1].destination());
    }
    {
        auto&& column = union_columns[2];
        EXPECT_EQ(column.left(), left_columns[2].destination());
        EXPECT_EQ(column.right(), right_columns[2].destination());
    }
    {
        auto&& column = union_columns[3];
        EXPECT_EQ(column.left(), left_columns[3].destination());
        EXPECT_EQ(column.right(), right_columns[3].destination());
    }

    EXPECT_EQ(union_columns[0].destination(), relation_columns[0].variable());
    EXPECT_EQ(union_columns[1].destination(), relation_columns[1].variable());
    EXPECT_EQ(union_columns[2].destination(), relation_columns[2].variable());
    EXPECT_EQ(union_columns[3].destination(), relation_columns[3].variable());
}

TEST_F(analyze_query_expression_binary_test, union_default) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::binary_expression {
                    ast::query::query {
                            {
                                    ast::query::select_asterisk {},
                            },
                            {
                                    ast::table::table_reference {
                                            id("a"),
                                    },
                            },
                    },
                    ast::query::binary_operator::union_,
                    {},
                    ast::query::query {
                            {
                                    ast::query::select_asterisk {},
                            },
                            {
                                    ast::table::table_reference {
                                            id("b"),
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 5);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_FALSE(relation.declaration());
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 4);

    // (scan - project)x2 - union[all]
    auto&& union_ = downcast<trelation::intermediate::union_>(r.output().owner());
    auto&& left_project = *find_prev<trelation::project>(union_.left());
    auto&& left_scan = *find_prev<trelation::scan>(left_project);
    auto&& right_project = *find_prev<trelation::project>(union_.right());
    auto&& right_scan = *find_prev<trelation::scan>(right_project);

    ASSERT_EQ(left_project.columns().size(), 0);

    EXPECT_EQ(extract<::yugawara::storage::index>(left_scan.source()).table(), *table_a);
    auto&& left_columns = left_scan.columns();
    ASSERT_EQ(left_columns.size(), 4);

    ASSERT_EQ(right_project.columns().size(), 0);

    EXPECT_EQ(extract<::yugawara::storage::index>(right_scan.source()).table(), *table_b);
    auto&& right_columns = right_scan.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(union_.quantifier(), trelation::set_quantifier::distinct);
    auto&& union_columns = union_.mappings();
    ASSERT_EQ(union_columns.size(), 4);

    EXPECT_EQ(union_columns[0].destination(), relation_columns[0].variable());
    EXPECT_EQ(union_columns[1].destination(), relation_columns[1].variable());
    EXPECT_EQ(union_columns[2].destination(), relation_columns[2].variable());
    EXPECT_EQ(union_columns[3].destination(), relation_columns[3].variable());
}

TEST_F(analyze_query_expression_binary_test, union_type_unification) {
    auto&& ctxt = context();
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            ctxt,
            graph,
            ast::query::binary_expression {
                    ast::query::table_value_constructor {
                            ast::scalar::value_constructor {
                                    ast::scalar::cast_expression {
                                            ast::scalar::cast_operator::cast,
                                            literal(number("100")),
                                            ast::type::simple {
                                                    ast::type::kind::big_integer,
                                            },
                                    },
                            },
                    },
                    ast::query::binary_operator::union_,
                    ast::query::set_quantifier::all,
                    ast::query::table_value_constructor {
                            ast::scalar::value_constructor {
                                    ast::scalar::cast_expression {
                                            ast::scalar::cast_operator::cast,
                                            literal(number("100")),
                                            ast::type::simple {
                                                    ast::type::kind::float_,
                                            },
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_FALSE(relation.declaration());
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), "");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }

    // (values)x2 - union[all]
    auto&& union_ = downcast<trelation::intermediate::union_>(r.output().owner());
    auto&& left_values = *find_prev<trelation::values>(union_.left());
    auto&& right_values = *find_prev<trelation::values>(union_.right());

    auto&& left_columns = left_values.columns();
    ASSERT_EQ(left_columns.size(), 1);

    auto&& right_columns = right_values.columns();
    ASSERT_EQ(right_columns.size(), 1);

    auto&& union_columns = union_.mappings();
    ASSERT_EQ(union_columns.size(), 1);

    auto info = ctxt.test_info();
    auto&& type = info.type_of(union_columns[0].destination());
    EXPECT_EQ(type, ttype::float8 {});
}

TEST_F(analyze_query_expression_binary_test, union_corresponding_unsupported) {
    invalid(sql_analyzer_code::unsupported_feature, ast::query::binary_expression {
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("100")),
                    },
            },
            ast::query::binary_operator::union_,
            ast::query::set_quantifier::all,
            {},
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("100")),
                    },
            },
    });
}

TEST_F(analyze_query_expression_binary_test, union_left_broken) {
    invalid(ast::query::binary_expression {
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            vref(id("MISSING")),
                    },
            },
            ast::query::binary_operator::union_,
            ast::query::set_quantifier::all,
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("100")),
                    },
            },
    });
}

TEST_F(analyze_query_expression_binary_test, union_right_broken) {
    invalid(ast::query::binary_expression {
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("100")),
                    },
            },
            ast::query::binary_operator::union_,
            ast::query::set_quantifier::all,
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            vref(id("MISSING")),
                    },
            },
    });
}

TEST_F(analyze_query_expression_binary_test, union_inconsistent_columns) {
    invalid(sql_analyzer_code::inconsistent_columns, ast::query::binary_expression {
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("100")),
                    },
            },
            ast::query::binary_operator::union_,
            ast::query::set_quantifier::all,
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("100")),
                            literal(number("200")),
                    },
            },
    });
}


TEST_F(analyze_query_expression_binary_test, union_inconsistent_type) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    invalid(sql_analyzer_code::inconsistent_type, ast::query::binary_expression {
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("k")) },
                    },
                    {
                            ast::table::table_reference {
                                    id("a"),
                            },
                    },
            },
            ast::query::binary_operator::union_,
            ast::query::set_quantifier::all,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("v")) },
                    },
                    {
                            ast::table::table_reference {
                                    id("b"),
                            },
                    },
            },
    });
}

} // namespace mizugaki::analyzer::details
