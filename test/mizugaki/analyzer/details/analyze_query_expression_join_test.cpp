#include <mizugaki/analyzer/details/analyze_query_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>
#include <takatori/type/decimal.h>

#include <takatori/scalar/binary.h>
#include <takatori/scalar/cast.h>
#include <takatori/scalar/coalesce.h>
#include <takatori/scalar/compare.h>
#include <takatori/scalar/variable_reference.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>
#include <takatori/relation/intermediate/aggregate.h>
#include <takatori/relation/intermediate/distinct.h>
#include <takatori/relation/intermediate/join.h>

#include <yugawara/binding/extract.h>

#include <yugawara/type/conversion.h>

#include <yugawara/extension/scalar/exists.h>

#include <mizugaki/ast/name/qualified.h>

#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/builtin_set_function_invocation.h>
#include <mizugaki/ast/scalar/table_predicate.h>

#include <mizugaki/ast/table/table_reference.h>
#include <mizugaki/ast/table/join.h>
#include <mizugaki/ast/table/join_condition.h>
#include <mizugaki/ast/table/join_columns.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/select_column.h>
#include <mizugaki/ast/query/select_asterisk.h>
#include <mizugaki/ast/query/grouping_column.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_query_expression_join_test : public test_parent {
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

    using column_ref = ::yugawara::storage::column const*;

    /**
     * @brief returns the table column which the given variable refers to.
     * @details this follows the project columns which only forward a variable.
     */
    static column_ref source_column(trelation::graph_type const& graph, ::takatori::descriptor::variable variable) {
        for (bool changed = true; changed;) {
            changed = false;
            for (auto&& node : graph) {
                if (auto project = downcast<trelation::project>(&node)) {
                    for (auto&& column : project->columns()) {
                        if (column.variable() != variable) {
                            continue;
                        }
                        if (auto ref = downcast<tscalar::variable_reference>(&column.value())) {
                            variable = ref->variable();
                            changed = true;
                        }
                    }
                }
            }
        }
        for (auto&& node : graph) {
            if (auto scan = downcast<trelation::scan>(&node)) {
                for (auto&& column : scan->columns()) {
                    if (column.destination() == variable) {
                        return std::addressof(extract<::yugawara::storage::column>(column.source()));
                    }
                }
            }
        }
        return nullptr;
    }

    /**
     * @brief returns the variable which the scan operator in the graph assigns to the given table column.
     */
    static std::optional<::takatori::descriptor::variable> scan_variable(
            trelation::graph_type const& graph,
            ::yugawara::storage::column const& target) {
        for (auto&& node : graph) {
            if (auto scan = downcast<trelation::scan>(&node)) {
                for (auto&& column : scan->columns()) {
                    if (std::addressof(extract<::yugawara::storage::column>(column.source())) == std::addressof(target)) {
                        return column.destination();
                    }
                }
            }
        }
        return {};
    }

    /**
     * @brief tests whether the graph has a join operator with the given condition.
     */
    static bool has_join_condition(trelation::graph_type const& graph, tscalar::expression const& expected) {
        for (auto&& node : graph) {
            if (auto join = downcast<trelation::intermediate::join>(&node)) {
                if (join->condition() && *join->condition() == expected) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * @brief tests that each column of the relation refers to the given table column, in order.
     */
    template<class Relation>
    static void expect_source_columns(
            trelation::graph_type const& graph,
            Relation const& relation,
            std::vector<column_ref> const& expected) {
        auto columns = relation.columns();
        ASSERT_EQ(columns.size(), expected.size());
        for (std::size_t i = 0; i < expected.size(); ++i) {
            EXPECT_EQ(source_column(graph, columns[i].variable()), expected[i]) << "column #" << i;
        }
    }

    static std::vector<column_ref> all_columns(::yugawara::storage::table const& table) {
        std::vector<column_ref> results {};
        for (auto&& column : table.columns()) {
            results.emplace_back(std::addressof(column));
        }
        return results;
    }

    static std::vector<column_ref> concat(std::initializer_list<std::vector<column_ref>> lists) {
        std::vector<column_ref> results {};
        for (auto&& list : lists) {
            results.insert(results.end(), list.begin(), list.end());
        }
        return results;
    }

    static ast::table::join_condition on_equals(std::string_view left, std::string_view right) {
        return ast::table::join_condition {
                ast::scalar::comparison_predicate {
                        vref(id(left, "k")),
                        ast::scalar::comparison_operator::equals,
                        vref(id(right, "k")),
                },
        };
    }

    static tscalar::compare equals(::takatori::descriptor::variable left, ::takatori::descriptor::variable right) {
        return tscalar::compare {
                tscalar::comparison_operator::equal,
                vref(std::move(left)),
                vref(std::move(right)),
        };
    }

    /*
     * SELECT a.k, b.k, a.*, b.*
     * FROM a <type> JOIN b ON a.k = b.k
     */
    void check_qualified_references_on(ast::table::join_type type) {
        auto table_a = install_table("a");
        auto table_b = install_table("b");
        trelation::graph_type graph {};
        auto r = analyze_query_expression(
                context(),
                graph,
                ast::query::query {
                        {
                                ast::query::select_column { vref(id("a", "k")) },
                                ast::query::select_column { vref(id("b", "k")) },
                                ast::query::select_asterisk { vref(id("a")) },
                                ast::query::select_asterisk { vref(id("b")) },
                        },
                        {
                                ast::table::join {
                                        ast::table::table_reference { id("a") },
                                        type,
                                        ast::table::table_reference { id("b") },
                                        on_equals("a", "b"),
                                },
                        },
                },
                {},
                {});
        ASSERT_TRUE(r) << diagnostics();
        expect_no_error();

        expect_source_columns(graph, r.relation(), concat({
                { &table_a->columns()[0], &table_b->columns()[0] },
                all_columns(*table_a),
                all_columns(*table_b),
        }));
    }

    /*
     * SELECT a.k, a.*
     * FROM a <type> JOIN b USING (k)
     */
    void check_qualified_references_using_left(ast::table::join_type type) {
        auto table_a = install_table("a");
        auto table_b = install_table("b");
        trelation::graph_type graph {};
        auto r = analyze_query_expression(
                context(),
                graph,
                ast::query::query {
                        {
                                ast::query::select_column { vref(id("a", "k")) },
                                ast::query::select_asterisk { vref(id("a")) },
                        },
                        {
                                ast::table::join {
                                        ast::table::table_reference { id("a") },
                                        type,
                                        ast::table::table_reference { id("b") },
                                        ast::table::join_columns {
                                                id("k"),
                                        },
                                },
                        },
                },
                {},
                {});
        ASSERT_TRUE(r) << diagnostics();
        expect_no_error();

        expect_source_columns(graph, r.relation(), concat({
                { &table_a->columns()[0] },
                all_columns(*table_a),
        }));
    }
    /*
     * SELECT b.k, b.*
     * FROM a <type> JOIN b USING (k)
     */
    void check_qualified_references_using_right(ast::table::join_type type) {
        auto table_a = install_table("a");
        auto table_b = install_table("b");
        trelation::graph_type graph {};
        auto r = analyze_query_expression(
                context(),
                graph,
                ast::query::query {
                        {
                                ast::query::select_column { vref(id("b", "k")) },
                                ast::query::select_asterisk { vref(id("b")) },
                        },
                        {
                                ast::table::join {
                                        ast::table::table_reference { id("a") },
                                        type,
                                        ast::table::table_reference { id("b") },
                                        ast::table::join_columns {
                                                id("k"),
                                        },
                                },
                        },
                },
                {},
                {});
        ASSERT_TRUE(r) << diagnostics();
        expect_no_error();

        expect_source_columns(graph, r.relation(), concat({
                { &table_b->columns()[0] },
                all_columns(*table_b),
        }));
    }

    /**
     * @brief installs a table like install_table(), but its first column "k" has the given type.
     */
    std::shared_ptr<::yugawara::storage::table> install_keyed_table(std::string_view name, ttype::data&& key_type) {
        auto table = storages_->add_table(::yugawara::storage::table {
                name,
                {
                        { "k", std::move(key_type) },
                        { "v", ttype::character { ttype::varying }, ::yugawara::variable::nullable },
                        { "w", ttype::character { ttype::varying }, ::yugawara::variable::nullable },
                        { "x", ttype::character { ttype::varying }, ::yugawara::variable::nullable },
                },
        });
        storages_->add_index(::yugawara::storage::index {
                table,
                name,
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
        return table;
    }

    /**
     * @brief follows the project columns which only forward a variable.
     */
    static ::takatori::descriptor::variable forwarded(trelation::graph_type const& graph, ::takatori::descriptor::variable variable) {
        for (bool changed = true; changed;) {
            changed = false;
            for (auto&& node : graph) {
                if (auto project = downcast<trelation::project>(&node)) {
                    for (auto&& column : project->columns()) {
                        if (column.variable() != variable) {
                            continue;
                        }
                        if (auto ref = downcast<tscalar::variable_reference>(&column.value())) {
                            variable = ref->variable();
                            changed = true;
                        }
                    }
                }
            }
        }
        return variable;
    }

    /**
     * @brief returns the expression which computes the given variable in a project operator, or nullptr if there is not.
     * @details this follows the project columns which only forward a variable.
     */
    static tscalar::expression const* defining_value(trelation::graph_type const& graph, ::takatori::descriptor::variable const& variable) {
        auto target = forwarded(graph, variable);
        for (auto&& node : graph) {
            if (auto project = downcast<trelation::project>(&node)) {
                for (auto&& column : project->columns()) {
                    if (column.variable() == target) {
                        return std::addressof(column.value());
                    }
                }
            }
        }
        return nullptr;
    }

    /**
     * @brief analyzes "SELECT k FROM <left> <type> JOIN <right> USING (k)", and returns the variable of "k".
     */
    std::optional<::takatori::descriptor::variable> analyze_common_column(
            trelation::graph_type& graph,
            std::string_view left,
            ast::table::join_type type,
            std::string_view right) {
        auto r = analyze_query_expression(
                context(),
                graph,
                ast::query::query {
                        {
                                ast::query::select_column { vref(id("k")) },
                        },
                        {
                                ast::table::join {
                                        ast::table::table_reference { id(left) },
                                        type,
                                        ast::table::table_reference { id(right) },
                                        ast::table::join_columns {
                                                id("k"),
                                        },
                                },
                        },
                },
                {},
                {});
        if (!r) {
            ADD_FAILURE() << diagnostics();
            return {};
        }
        expect_no_error();
        auto columns = r.relation().columns();
        if (columns.size() != 1) {
            ADD_FAILURE() << columns.size();
            return {};
        }
        return columns[0].variable();
    }

    /*
     * SELECT k
     * FROM a <type> JOIN b USING (k)
     * -- a.k and b.k have the same type.
     */
    void check_common_column_same_type(ast::table::join_type type) {
        auto table_a = install_table("a");
        auto table_b = install_table("b");
        trelation::graph_type graph {};
        auto k = analyze_common_column(graph, "a", type, "b");
        ASSERT_TRUE(k);
        auto a_k = scan_variable(graph, table_a->columns()[0]);
        auto b_k = scan_variable(graph, table_b->columns()[0]);
        ASSERT_TRUE(a_k);
        ASSERT_TRUE(b_k);
        switch (type) {
            case ast::table::join_type::inner:
            case ast::table::join_type::left_outer:
                EXPECT_EQ(forwarded(graph, *k), *a_k);
                break;
            case ast::table::join_type::right_outer:
                EXPECT_EQ(forwarded(graph, *k), *b_k);
                break;
            case ast::table::join_type::full_outer: {
                auto value = defining_value(graph, *k);
                ASSERT_TRUE(value);
                EXPECT_EQ(*value, (tscalar::coalesce { { vref(*a_k), vref(*b_k) } }));
                break;
            }
            default:
                FAIL();
        }
    }

    /*
     * SELECT k
     * FROM l <type> JOIN r USING (k)
     * -- the common column is "CAST(<base>.k AS <unified type>)".
     */
    void check_common_column_cast(
            ast::table::join_type type,
            ::yugawara::storage::table const& left,
            ::yugawara::storage::table const& right,
            ::yugawara::storage::table const& base) {
        trelation::graph_type graph {};
        auto k = analyze_common_column(graph, left.simple_name(), type, right.simple_name());
        ASSERT_TRUE(k);
        auto base_k = scan_variable(graph, base.columns()[0]);
        ASSERT_TRUE(base_k);
        auto unified = ::yugawara::type::unifying_conversion(left.columns()[0].type(), right.columns()[0].type());
        ASSERT_TRUE(unified);

        auto value = defining_value(graph, *k);
        ASSERT_TRUE(value);
        auto cast = downcast<tscalar::cast>(value);
        ASSERT_TRUE(cast) << *value;
        EXPECT_EQ(cast->type(), *unified);
        EXPECT_EQ(cast->operand(), vref(*base_k));
    }

};

TEST_F(analyze_query_expression_join_test, multiple_from) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::table_reference { id("a") },
                            ast::table::table_reference { id("b") },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::inner);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_FALSE(join.condition());

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_cross) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::cross,
                                    ast::table::table_reference { id("b") },
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::inner);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_FALSE(join.condition());

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_inner) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::inner,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_condition {
                                            ast::scalar::comparison_predicate {
                                                    vref(ast::name::qualified(id("a"), id("k"))),
                                                    ast::scalar::comparison_operator::equals,
                                                    vref(ast::name::qualified(id("b"), id("k"))),
                                            }
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.condition(), (tscalar::compare(
            tscalar::comparison_operator::equal,
            vref(left_columns[0].destination()),
            vref(right_columns[0].destination()))));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_left) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::left_outer,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_condition {
                                            ast::scalar::comparison_predicate {
                                                    vref(ast::name::qualified(id("a"), id("k"))),
                                                    ast::scalar::comparison_operator::equals,
                                                    vref(ast::name::qualified(id("b"), id("k"))),
                                            }
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::left_outer);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.condition(), (tscalar::compare(
            tscalar::comparison_operator::equal,
            vref(left_columns[0].destination()),
            vref(right_columns[0].destination()))));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_right) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::right_outer,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_condition {
                                            ast::scalar::comparison_predicate {
                                                    vref(ast::name::qualified(id("a"), id("k"))),
                                                    ast::scalar::comparison_operator::equals,
                                                    vref(ast::name::qualified(id("b"), id("k"))),
                                            }
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_b);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_a);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::left_outer);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.condition(), (tscalar::compare(
            tscalar::comparison_operator::equal,
            vref(right_columns[0].destination()),
            vref(left_columns[0].destination()))));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(right_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(left_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_full) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::full_outer,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_condition {
                                            ast::scalar::comparison_predicate {
                                                    vref(ast::name::qualified(id("a"), id("k"))),
                                                    ast::scalar::comparison_operator::equals,
                                                    vref(ast::name::qualified(id("b"), id("k"))),
                                            }
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::full_outer);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.condition(), (tscalar::compare(
            tscalar::comparison_operator::equal,
            vref(left_columns[0].destination()),
            vref(right_columns[0].destination()))));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, self_join) {
    auto table = install_table("t");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk {},
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference {
                                            id("t"),
                                            id("t1"),
                                    },
                                    ast::table::join_type::inner,
                                    ast::table::table_reference {
                                            id("t"),
                                            id("t2"),
                                    },
                                    ast::table::join_condition{
                                            ast::scalar::comparison_predicate {
                                                    vref(ast::name::qualified(id("t1"), id("k"))),
                                                    ast::scalar::comparison_operator::equals,
                                                    vref(ast::name::qualified(id("t2"), id("k"))),
                                            }
                                    },
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 8);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::inner);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.condition(), (tscalar::compare {
            tscalar::comparison_operator::equal,
            vref(left_columns[0].destination()),
            vref(right_columns[0].destination()),
    }));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 0);

    EXPECT_EQ(relation_columns[0].variable(), left_columns[0].destination());
    EXPECT_EQ(relation_columns[1].variable(), left_columns[1].destination());
    EXPECT_EQ(relation_columns[2].variable(), left_columns[2].destination());
    EXPECT_EQ(relation_columns[3].variable(), left_columns[3].destination());
    EXPECT_EQ(relation_columns[4].variable(), right_columns[0].destination());
    EXPECT_EQ(relation_columns[5].variable(), right_columns[1].destination());
    EXPECT_EQ(relation_columns[6].variable(), right_columns[2].destination());
    EXPECT_EQ(relation_columns[7].variable(), right_columns[3].destination());
}

TEST_F(analyze_query_expression_join_test, join_columns) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::inner,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_columns {
                                            id("k")
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.condition(), (tscalar::compare(
            tscalar::comparison_operator::equal,
            vref(left_columns[0].destination()),
            vref(right_columns[0].destination()))));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_columns_multiple) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("k"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("k"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::inner,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_columns {
                                            id("v"),
                                            id("w"),
                                            id("x"),
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.condition(), (tscalar::binary {
            tscalar::binary_operator::conditional_and,
            tscalar::binary {
                    tscalar::binary_operator::conditional_and,
                    tscalar::compare {
                            tscalar::comparison_operator::equal,
                            vref(left_columns[1].destination()), // a.v
                            vref(right_columns[1].destination()), // b.v
                    },
                    tscalar::compare {
                            tscalar::comparison_operator::equal,
                            vref(left_columns[2].destination()), // a.w
                            vref(right_columns[2].destination()), // b/w
                    },
            },
            tscalar::compare {
                    tscalar::comparison_operator::equal,
                    vref(left_columns[3].destination()), // a.x
                    vref(right_columns[3].destination()), // b.x
            },
    }));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    EXPECT_EQ(project_columns[0].value(), vref(left_columns[0].destination())); // a.k
    EXPECT_EQ(project_columns[1].value(), vref(right_columns[0].destination())); // b.k

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_condition_invalid_expression) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    invalid(ast::query::query {
            {
                    ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                    ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
            },
            {
                    ast::table::join {
                            ast::table::table_reference { id("a") },
                            ast::table::join_type::inner,
                            ast::table::table_reference { id("b") },
                            ast::table::join_condition {
                                    ast::scalar::comparison_predicate {
                                            vref(id("k")),
                                            ast::scalar::comparison_operator::equals,
                                            vref(id("k")),
                                    }
                            }
                    }
            },
    });
}

TEST_F(analyze_query_expression_join_test, join_condition_invalid_aggregated) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    auto count = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "count",
            ttype::int8 {},
            {},
            true,
    });
    invalid(sql_analyzer_code::unsupported_feature, ast::query::query {
            {
                    ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                    ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
            },
            {
                    ast::table::join {
                            ast::table::table_reference { id("a") },
                            ast::table::join_type::inner,
                            ast::table::table_reference { id("b") },
                            ast::table::join_condition {
                                    ast::scalar::comparison_predicate {
                                            ast::scalar::builtin_set_function_invocation {
                                                    ast::scalar::builtin_set_function_kind::count,
                                                    {},
                                                    {},
                                            },
                                            ast::scalar::comparison_operator::equals,
                                            ast::scalar::binary_expression {
                                                    vref(ast::name::qualified(id("a"), id("k"))),
                                                    ast::scalar::binary_operator::plus,
                                                    vref(ast::name::qualified(id("b"), id("k"))),
                                            }
                                    }
                            }
                    }
            },
    });
}

TEST_F(analyze_query_expression_join_test, join_columns_missing) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    invalid(sql_analyzer_code::column_not_found, ast::query::query {
            {
                    ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                    ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
            },
            {
                    ast::table::join {
                            ast::table::table_reference { id("a") },
                            ast::table::join_type::inner,
                            ast::table::table_reference { id("b") },
                            ast::table::join_columns {
                                    id("MISSING")
                            }
                    }
            },
    });
}

TEST_F(analyze_query_expression_join_test, qualified_asterisk_left) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk { vref(id("a")) },
                    },
                    {
                            ast::table::table_reference { id("a") },
                            ast::table::table_reference { id("b") },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 4);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 0);

    EXPECT_EQ(relation_columns[0].variable(), left_columns[0].destination());
    EXPECT_EQ(relation_columns[1].variable(), left_columns[1].destination());
    EXPECT_EQ(relation_columns[2].variable(), left_columns[2].destination());
    EXPECT_EQ(relation_columns[3].variable(), left_columns[3].destination());
}

TEST_F(analyze_query_expression_join_test, qualified_asterisk_right) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk { vref(id("public", "b")) },
                    },
                    {
                            ast::table::table_reference { id("a") },
                            ast::table::table_reference { id("b") },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 4);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 0);

    EXPECT_EQ(relation_columns[0].variable(), right_columns[0].destination());
    EXPECT_EQ(relation_columns[1].variable(), right_columns[1].destination());
    EXPECT_EQ(relation_columns[2].variable(), right_columns[2].destination());
    EXPECT_EQ(relation_columns[3].variable(), right_columns[3].destination());
}

TEST_F(analyze_query_expression_join_test, join_condition_left_nested) {
    /*
     * SELECT a.v, b.w, c.x
     * FROM a JOIN b ON a.k = b.k JOIN c ON a.k = c.k
     * -- the second ON can refer to the columns of a, b, and c.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    auto table_c = install_table("c");
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("a", "v")) },
                            ast::query::select_column { vref(id("b", "w")) },
                            ast::query::select_column { vref(id("c", "x")) },
                    },
                    {
                            ast::table::join {
                                    ast::table::join {
                                            ast::table::table_reference { id("a") },
                                            ast::table::join_type::inner,
                                            ast::table::table_reference { id("b") },
                                            on_equals("a", "b"),
                                    },
                                    ast::table::join_type::inner,
                                    ast::table::table_reference { id("c") },
                                    on_equals("a", "c"),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    expect_source_columns(graph, r.relation(), {
            &table_a->columns()[1],
            &table_b->columns()[2],
            &table_c->columns()[3],
    });

    auto a_k = scan_variable(graph, table_a->columns()[0]);
    auto b_k = scan_variable(graph, table_b->columns()[0]);
    auto c_k = scan_variable(graph, table_c->columns()[0]);
    ASSERT_TRUE(a_k);
    ASSERT_TRUE(b_k);
    ASSERT_TRUE(c_k);
    EXPECT_TRUE(has_join_condition(graph, equals(*a_k, *b_k)));
    EXPECT_TRUE(has_join_condition(graph, equals(*a_k, *c_k)));
}

TEST_F(analyze_query_expression_join_test, join_condition_right_nested) {
    /*
     * SELECT a.v, b.w, c.x
     * FROM a JOIN (b JOIN c ON b.k = c.k) ON a.k = c.k
     * -- the inner ON can refer to the columns of b and c,
     * -- and the outer ON can refer to the columns of a, b, and c.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    auto table_c = install_table("c");
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("a", "v")) },
                            ast::query::select_column { vref(id("b", "w")) },
                            ast::query::select_column { vref(id("c", "x")) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::inner,
                                    ast::table::join {
                                            ast::table::table_reference { id("b") },
                                            ast::table::join_type::inner,
                                            ast::table::table_reference { id("c") },
                                            on_equals("b", "c"),
                                    },
                                    on_equals("a", "c"),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    expect_source_columns(graph, r.relation(), {
            &table_a->columns()[1],
            &table_b->columns()[2],
            &table_c->columns()[3],
    });

    auto a_k = scan_variable(graph, table_a->columns()[0]);
    auto b_k = scan_variable(graph, table_b->columns()[0]);
    auto c_k = scan_variable(graph, table_c->columns()[0]);
    ASSERT_TRUE(a_k);
    ASSERT_TRUE(b_k);
    ASSERT_TRUE(c_k);
    EXPECT_TRUE(has_join_condition(graph, equals(*b_k, *c_k)));
    EXPECT_TRUE(has_join_condition(graph, equals(*a_k, *c_k)));
}

TEST_F(analyze_query_expression_join_test, join_condition_outer_reference) {
    /*
     * SELECT a.v
     * FROM a
     * WHERE EXISTS (
     *   SELECT b.w
     *   FROM b JOIN c ON b.k = c.k AND c.k = a.k
     * )
     * -- ON can refer to the columns of the outer query.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    auto table_c = install_table("c");
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("a", "v")) },
                    },
                    {
                            ast::table::table_reference { id("a") },
                    },
                    ast::scalar::table_predicate {
                            ast::scalar::table_operator::exists,
                            ast::query::query {
                                    {
                                            ast::query::select_column { vref(id("b", "w")) },
                                    },
                                    {
                                            ast::table::join {
                                                    ast::table::table_reference { id("b") },
                                                    ast::table::join_type::inner,
                                                    ast::table::table_reference { id("c") },
                                                    ast::table::join_condition {
                                                            ast::scalar::binary_expression {
                                                                    ast::scalar::comparison_predicate {
                                                                            vref(id("b", "k")),
                                                                            ast::scalar::comparison_operator::equals,
                                                                            vref(id("c", "k")),
                                                                    },
                                                                    ast::scalar::binary_operator::and_,
                                                                    ast::scalar::comparison_predicate {
                                                                            vref(id("c", "k")),
                                                                            ast::scalar::comparison_operator::equals,
                                                                            vref(id("a", "k")),
                                                                    },
                                                            },
                                                    },
                                            },
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    ::yugawara::extension::scalar::exists const* exists = nullptr;
    for (auto&& node : graph) {
        if (auto filter = downcast<trelation::filter>(&node)) {
            exists = downcast<::yugawara::extension::scalar::exists>(&filter->condition());
        }
    }
    ASSERT_TRUE(exists);

    auto a_k = scan_variable(graph, table_a->columns()[0]);
    ASSERT_TRUE(a_k);
    auto&& parameters = exists->parameters();
    ASSERT_EQ(parameters.size(), 1);
    EXPECT_EQ(parameters[0].source(), *a_k);

    auto&& subgraph = exists->query_graph();
    auto b_k = scan_variable(subgraph, table_b->columns()[0]);
    auto c_k = scan_variable(subgraph, table_c->columns()[0]);
    ASSERT_TRUE(b_k);
    ASSERT_TRUE(c_k);
    EXPECT_TRUE(has_join_condition(subgraph, tscalar::binary {
            tscalar::binary_operator::conditional_and,
            equals(*b_k, *c_k),
            equals(*c_k, parameters[0].destination()),
    }));
}

TEST_F(analyze_query_expression_join_test, asterisk_nested) {
    /*
     * SELECT *
     * FROM a JOIN (b JOIN c ON b.k = c.k) ON a.k = b.k
     * -- columns of a, b, and c, in order.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    auto table_c = install_table("c");
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk {},
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::inner,
                                    ast::table::join {
                                            ast::table::table_reference { id("b") },
                                            ast::table::join_type::inner,
                                            ast::table::table_reference { id("c") },
                                            on_equals("b", "c"),
                                    },
                                    on_equals("a", "b"),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    expect_source_columns(graph, r.relation(), concat({
            all_columns(*table_a),
            all_columns(*table_b),
            all_columns(*table_c),
    }));

    auto relation_columns = r.relation().columns();
    ASSERT_EQ(relation_columns.size(), 12);
    for (std::size_t i = 0; i < relation_columns.size(); ++i) {
        EXPECT_EQ(relation_columns[i].identifier(), table_a->columns()[i % 4].simple_name()) << "column #" << i;
    }
}

TEST_F(analyze_query_expression_join_test, qualified_references_inner) {
    check_qualified_references_on(ast::table::join_type::inner);
}

TEST_F(analyze_query_expression_join_test, qualified_references_left) {
    check_qualified_references_on(ast::table::join_type::left_outer);
}

TEST_F(analyze_query_expression_join_test, qualified_references_right) {
    check_qualified_references_on(ast::table::join_type::right_outer);
}

TEST_F(analyze_query_expression_join_test, qualified_references_full) {
    check_qualified_references_on(ast::table::join_type::full_outer);
}

TEST_F(analyze_query_expression_join_test, qualified_references_join_columns_left_inner) {
    check_qualified_references_using_left(ast::table::join_type::inner);
}

TEST_F(analyze_query_expression_join_test, qualified_references_join_columns_left_left) {
    check_qualified_references_using_left(ast::table::join_type::left_outer);
}

TEST_F(analyze_query_expression_join_test, qualified_references_join_columns_left_right) {
    check_qualified_references_using_left(ast::table::join_type::right_outer);
}

TEST_F(analyze_query_expression_join_test, qualified_references_join_columns_left_full) {
    check_qualified_references_using_left(ast::table::join_type::full_outer);
}

TEST_F(analyze_query_expression_join_test, join_condition_invalid_outside_operands) {
    /*
     * SELECT a.v
     * FROM a JOIN (b JOIN c ON a.k = c.k) ON a.k = b.k
     * -- the inner ON cannot refer to the columns of a.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    auto table_c = install_table("c");
    invalid(sql_analyzer_code::symbol_not_found, ast::query::query {
            {
                    ast::query::select_column { vref(id("a", "v")) },
            },
            {
                    ast::table::join {
                            ast::table::table_reference { id("a") },
                            ast::table::join_type::inner,
                            ast::table::join {
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_type::inner,
                                    ast::table::table_reference { id("c") },
                                    on_equals("a", "c"),
                            },
                            on_equals("a", "b"),
                    },
            },
    });
}

TEST_F(analyze_query_expression_join_test, join_condition_invalid_preceding_table) {
    /*
     * SELECT a.v
     * FROM a, b JOIN c ON a.k = c.k
     * -- ON cannot refer to the columns of the preceding table reference in the FROM clause.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    auto table_c = install_table("c");
    invalid(sql_analyzer_code::symbol_not_found, ast::query::query {
            {
                    ast::query::select_column { vref(id("a", "v")) },
            },
            {
                    ast::table::table_reference { id("a") },
                    ast::table::join {
                            ast::table::table_reference { id("b") },
                            ast::table::join_type::inner,
                            ast::table::table_reference { id("c") },
                            on_equals("a", "c"),
                    },
            },
    });
}

TEST_F(analyze_query_expression_join_test, join_columns_nested) {
    /*
     * SELECT a.v, b.w, c.x
     * FROM a JOIN (b JOIN c USING (k)) ON a.k = b.k
     * -- USING looks for the columns only in its operands, so that "k" in a does not conflict.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    auto table_c = install_table("c");
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("a", "v")) },
                            ast::query::select_column { vref(id("b", "w")) },
                            ast::query::select_column { vref(id("c", "x")) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::inner,
                                    ast::table::join {
                                            ast::table::table_reference { id("b") },
                                            ast::table::join_type::inner,
                                            ast::table::table_reference { id("c") },
                                            ast::table::join_columns {
                                                    id("k"),
                                            },
                                    },
                                    on_equals("a", "b"),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    expect_source_columns(graph, r.relation(), {
            &table_a->columns()[1],
            &table_b->columns()[2],
            &table_c->columns()[3],
    });

    auto a_k = scan_variable(graph, table_a->columns()[0]);
    auto b_k = scan_variable(graph, table_b->columns()[0]);
    auto c_k = scan_variable(graph, table_c->columns()[0]);
    ASSERT_TRUE(a_k);
    ASSERT_TRUE(b_k);
    ASSERT_TRUE(c_k);
    EXPECT_TRUE(has_join_condition(graph, equals(*b_k, *c_k)));
    EXPECT_TRUE(has_join_condition(graph, equals(*a_k, *b_k)));
}

TEST_F(analyze_query_expression_join_test, qualified_references_join_columns_right_inner) {
    check_qualified_references_using_right(ast::table::join_type::inner);
}

TEST_F(analyze_query_expression_join_test, qualified_references_join_columns_right_left) {
    check_qualified_references_using_right(ast::table::join_type::left_outer);
}

TEST_F(analyze_query_expression_join_test, qualified_references_join_columns_right_right) {
    check_qualified_references_using_right(ast::table::join_type::right_outer);
}

TEST_F(analyze_query_expression_join_test, qualified_references_join_columns_right_full) {
    check_qualified_references_using_right(ast::table::join_type::full_outer);
}

TEST_F(analyze_query_expression_join_test, join_columns_common_inner) {
    check_common_column_same_type(ast::table::join_type::inner);
}

TEST_F(analyze_query_expression_join_test, join_columns_common_left) {
    check_common_column_same_type(ast::table::join_type::left_outer);
}

TEST_F(analyze_query_expression_join_test, join_columns_common_right) {
    check_common_column_same_type(ast::table::join_type::right_outer);
}

TEST_F(analyze_query_expression_join_test, join_columns_common_full) {
    check_common_column_same_type(ast::table::join_type::full_outer);
}

TEST_F(analyze_query_expression_join_test, join_columns_common_cast_inner) {
    auto table_a = install_keyed_table("a", ttype::int4 {});
    auto table_b = install_keyed_table("b", ttype::int8 {});
    check_common_column_cast(ast::table::join_type::inner, *table_a, *table_b, *table_a);
}

TEST_F(analyze_query_expression_join_test, join_columns_common_cast_left) {
    auto table_a = install_keyed_table("a", ttype::int4 {});
    auto table_b = install_keyed_table("b", ttype::int8 {});
    check_common_column_cast(ast::table::join_type::left_outer, *table_a, *table_b, *table_a);
}

TEST_F(analyze_query_expression_join_test, join_columns_common_cast_right) {
    auto table_a = install_keyed_table("a", ttype::int8 {});
    auto table_b = install_keyed_table("b", ttype::int4 {});
    check_common_column_cast(ast::table::join_type::right_outer, *table_a, *table_b, *table_b);
}

TEST_F(analyze_query_expression_join_test, join_columns_common_cast_decimal) {
    /*
     * -- a.k: DECIMAL(5, 2), b.k: DECIMAL(6, 1)
     * SELECT k FROM a JOIN b USING (k)
     * -- the types are different even if they are both DECIMAL, so that a.k is converted.
     */
    auto table_a = install_keyed_table("a", ttype::decimal { 5, 2 });
    auto table_b = install_keyed_table("b", ttype::decimal { 6, 1 });
    check_common_column_cast(ast::table::join_type::inner, *table_a, *table_b, *table_a);
}

TEST_F(analyze_query_expression_join_test, join_columns_common_same_type_right) {
    /*
     * -- a.k: INT, b.k: BIGINT
     * SELECT k FROM a RIGHT JOIN b USING (k)
     * -- b.k already has the unified type, so that it is used as is.
     */
    auto table_a = install_keyed_table("a", ttype::int4 {});
    auto table_b = install_keyed_table("b", ttype::int8 {});
    trelation::graph_type graph {};
    auto k = analyze_common_column(graph, "a", ast::table::join_type::right_outer, "b");
    ASSERT_TRUE(k);
    auto b_k = scan_variable(graph, table_b->columns()[0]);
    ASSERT_TRUE(b_k);
    EXPECT_EQ(forwarded(graph, *k), *b_k);
}

TEST_F(analyze_query_expression_join_test, join_columns_common_in_where) {
    /*
     * SELECT a.v
     * FROM a FULL JOIN b USING (k)
     * WHERE k = k
     * -- "k" in WHERE clause refers to the common column.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("a", "v")) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::full_outer,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_columns {
                                            id("k"),
                                    },
                            },
                    },
                    ast::scalar::comparison_predicate {
                            vref(id("k")),
                            ast::scalar::comparison_operator::equals,
                            vref(id("k")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto a_k = scan_variable(graph, table_a->columns()[0]);
    auto b_k = scan_variable(graph, table_b->columns()[0]);
    ASSERT_TRUE(a_k);
    ASSERT_TRUE(b_k);

    trelation::filter const* filter = nullptr;
    for (auto&& node : graph) {
        if (auto p = downcast<trelation::filter>(&node)) {
            filter = p;
        }
    }
    ASSERT_TRUE(filter);
    auto compare = downcast<tscalar::compare>(&filter->condition());
    ASSERT_TRUE(compare);
    auto left = downcast<tscalar::variable_reference>(&compare->left());
    ASSERT_TRUE(left);
    auto value = defining_value(graph, left->variable());
    ASSERT_TRUE(value);
    EXPECT_EQ(*value, (tscalar::coalesce { { vref(*a_k), vref(*b_k) } }));
}

TEST_F(analyze_query_expression_join_test, join_columns_common_in_group_by) {
    /*
     * SELECT k
     * FROM a FULL JOIN b USING (k)
     * GROUP BY k
     * -- "k" in GROUP BY clause refers to the common column.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("k")) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::full_outer,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_columns {
                                            id("k"),
                                    },
                            },
                    },
                    {},
                    ast::query::group_by_clause {
                            ast::query::grouping_column { id("k") },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto a_k = scan_variable(graph, table_a->columns()[0]);
    auto b_k = scan_variable(graph, table_b->columns()[0]);
    ASSERT_TRUE(a_k);
    ASSERT_TRUE(b_k);

    std::vector<::takatori::descriptor::variable> keys {};
    for (auto&& node : graph) {
        if (auto p = downcast<trelation::intermediate::aggregate>(&node)) {
            keys.assign(p->group_keys().begin(), p->group_keys().end());
        }
        if (auto p = downcast<trelation::intermediate::distinct>(&node)) {
            keys.assign(p->group_keys().begin(), p->group_keys().end());
        }
    }
    ASSERT_EQ(keys.size(), 1);
    auto value = defining_value(graph, keys[0]);
    ASSERT_TRUE(value);
    EXPECT_EQ(*value, (tscalar::coalesce { { vref(*a_k), vref(*b_k) } }));
}

TEST_F(analyze_query_expression_join_test, join_columns_asterisk) {
    /*
     * SELECT *
     * FROM a FULL JOIN b USING (k)
     * -- the common column, the rest of a, and then the rest of b.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk {},
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::full_outer,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_columns {
                                            id("k"),
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto relation_columns = r.relation().columns();
    ASSERT_EQ(relation_columns.size(), 7);
    std::vector<std::string_view> names { "k", "v", "w", "x", "v", "w", "x" };
    for (std::size_t i = 0; i < names.size(); ++i) {
        EXPECT_EQ(relation_columns[i].identifier(), names[i]) << "column #" << i;
    }

    auto a_k = scan_variable(graph, table_a->columns()[0]);
    auto b_k = scan_variable(graph, table_b->columns()[0]);
    ASSERT_TRUE(a_k);
    ASSERT_TRUE(b_k);
    auto value = defining_value(graph, relation_columns[0].variable());
    ASSERT_TRUE(value);
    EXPECT_EQ(*value, (tscalar::coalesce { { vref(*a_k), vref(*b_k) } }));

    for (std::size_t i = 1; i < 4; ++i) {
        EXPECT_EQ(source_column(graph, relation_columns[i].variable()), &table_a->columns()[i]) << "column #" << i;
        EXPECT_EQ(source_column(graph, relation_columns[i + 3].variable()), &table_b->columns()[i]) << "column #" << (i + 3);
    }
}

TEST_F(analyze_query_expression_join_test, join_columns_asterisk_multiple) {
    /*
     * SELECT *
     * FROM a JOIN b USING (w, k)
     * -- the common columns in the order of USING, the rest of a, and then the rest of b.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk {},
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::inner,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_columns {
                                            id("w"),
                                            id("k"),
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    expect_source_columns(graph, r.relation(), {
            &table_a->columns()[2], // w
            &table_a->columns()[0], // k
            &table_a->columns()[1], // a.v
            &table_a->columns()[3], // a.x
            &table_b->columns()[1], // b.v
            &table_b->columns()[3], // b.x
    });
}

TEST_F(analyze_query_expression_join_test, join_columns_chain) {
    /*
     * SELECT k
     * FROM a FULL JOIN b USING (k) FULL JOIN c USING (k)
     * -- the second USING refers to the common column of the first join.
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    auto table_c = install_table("c");
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("k")) },
                    },
                    {
                            ast::table::join {
                                    ast::table::join {
                                            ast::table::table_reference { id("a") },
                                            ast::table::join_type::full_outer,
                                            ast::table::table_reference { id("b") },
                                            ast::table::join_columns {
                                                    id("k"),
                                            },
                                    },
                                    ast::table::join_type::full_outer,
                                    ast::table::table_reference { id("c") },
                                    ast::table::join_columns {
                                            id("k"),
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto a_k = scan_variable(graph, table_a->columns()[0]);
    auto b_k = scan_variable(graph, table_b->columns()[0]);
    auto c_k = scan_variable(graph, table_c->columns()[0]);
    ASSERT_TRUE(a_k);
    ASSERT_TRUE(b_k);
    ASSERT_TRUE(c_k);

    auto relation_columns = r.relation().columns();
    ASSERT_EQ(relation_columns.size(), 1);
    auto outer = defining_value(graph, relation_columns[0].variable());
    ASSERT_TRUE(outer);
    auto outer_coalesce = downcast<tscalar::coalesce>(outer);
    ASSERT_TRUE(outer_coalesce) << *outer;
    ASSERT_EQ(outer_coalesce->alternatives().size(), 2);
    EXPECT_EQ(outer_coalesce->alternatives()[1], vref(*c_k));

    auto inner_ref = downcast<tscalar::variable_reference>(&outer_coalesce->alternatives()[0]);
    ASSERT_TRUE(inner_ref);
    auto inner = defining_value(graph, inner_ref->variable());
    ASSERT_TRUE(inner);
    EXPECT_EQ(*inner, (tscalar::coalesce { { vref(*a_k), vref(*b_k) } }));

    EXPECT_TRUE(has_join_condition(graph, equals(inner_ref->variable(), *c_k)));
}

TEST_F(analyze_query_expression_join_test, join_columns_invalid_type) {
    /*
     * -- a.k: BIGINT, b.k: VARCHAR
     * SELECT * FROM a JOIN b USING (k)
     */
    auto table_a = install_keyed_table("a", ttype::int8 {});
    auto table_b = install_keyed_table("b", ttype::character { ttype::varying });
    invalid(sql_analyzer_code::inconsistent_type, ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::join {
                            ast::table::table_reference { id("a") },
                            ast::table::join_type::inner,
                            ast::table::table_reference { id("b") },
                            ast::table::join_columns {
                                    id("k"),
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_join_test, join_columns_invalid_duplicate) {
    /*
     * SELECT * FROM a JOIN b USING (k, k)
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    invalid(sql_analyzer_code::column_already_exists, ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::join {
                            ast::table::table_reference { id("a") },
                            ast::table::join_type::inner,
                            ast::table::table_reference { id("b") },
                            ast::table::join_columns {
                                    id("k"),
                                    id("k"),
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_join_test, join_columns_invalid_hidden) {
    /*
     * -- a.k is hidden
     * SELECT * FROM a JOIN b USING (k)
     */
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    table_a->columns()[0].features() += ::yugawara::storage::column_feature_set {
            ::yugawara::storage::column_feature::hidden,
    };
    invalid(sql_analyzer_code::column_not_found, ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::join {
                            ast::table::table_reference { id("a") },
                            ast::table::join_type::inner,
                            ast::table::table_reference { id("b") },
                            ast::table::join_columns {
                                    id("k"),
                            },
                    },
            },
    });
}

} // namespace mizugaki::analyzer::details
