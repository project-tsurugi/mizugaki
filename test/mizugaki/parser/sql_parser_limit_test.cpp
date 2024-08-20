#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <string_view>

#include <mizugaki/ast/statement/schema_definition.h>
#include <mizugaki/ast/statement/table_definition.h>
#include <mizugaki/ast/statement/column_definition.h>

#include <mizugaki/ast/statement/insert_statement.h>
#include <mizugaki/ast/statement/update_statement.h>
#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/with_expression.h>
#include <mizugaki/ast/query/table_value_constructor.h>

#include <mizugaki/ast/scalar/value_constructor.h>
#include <mizugaki/ast/scalar/cast_expression.h>
#include <mizugaki/ast/scalar/case_expression.h>
#include <mizugaki/ast/scalar/literal_expression.h>

#include <mizugaki/ast/literal/string.h>

#include <mizugaki/ast/type/row.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

using element_kind = sql_parser_element_kind;

class sql_parser_limit_test : public ::testing::Test {
protected:
    void expect_exceed(sql_parser_result const& result) {
        if (result.has_value()) {
            ADD_FAILURE() << *result.value();
            return;
        }
        if (!result.has_diagnostic()) {
            ADD_FAILURE() << "no diagnostics";
            return;
        }
        auto d = result.diagnostic();
        EXPECT_EQ(d.code(), sql_parser_code::exceed_number_of_elements);
    }
};

TEST_F(sql_parser_limit_test, statement_ok) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::statement] = 2;

    std::string content { R"(SELECT 1; SELECT 2)" };
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto cu = std::move(r.value());
    EXPECT_EQ(cu->statements().size(), 2);
}

TEST_F(sql_parser_limit_test, statement_exceed) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::statement] = 2;

    std::string content { R"(SELECT 1; SELECT 2; SELECT 3)" };
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, schema_element_ok) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::schema_element_definition] = 2;

    std::string content { R"(CREATE SCHEMA s CREATE SEQUENCE s1 CREATE SEQUENCE s2;)" };
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::schema_definition>(*r.value()->statements()[0]);
    ASSERT_EQ(s.elements().size(), 2);
}

TEST_F(sql_parser_limit_test, schema_element_exceed) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::schema_element_definition] = 2;

    std::string content { R"(CREATE SCHEMA s CREATE SEQUENCE s1 CREATE SEQUENCE s2 CREATE SEQUENCE s3;)" };
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, table_element_definition_ok) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::table_element_definition] = 2;

    std::string content { R"(CREATE TABLE t (c1 int, c2 int))" };
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::table_definition>(*r.value()->statements()[0]);
    ASSERT_EQ(s.elements().size(), 2);
}

TEST_F(sql_parser_limit_test, table_element_definition_exceed) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::table_element_definition] = 2;

    std::string content { R"(CREATE TABLE t (c1 int, c2 int, c3 int))" };
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, column_constraint_definition_ok) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::column_constraint_definition] = 2;

    std::string content { R"(CREATE TABLE t (c1 int NULL DEFAULT 1))" };
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::table_definition>(*r.value()->statements()[0]);
    auto&& c = downcast<ast::statement::column_definition>(*s.elements()[0]);
    ASSERT_EQ(c.constraints().size(), 2);
}

TEST_F(sql_parser_limit_test, column_constraint_definition_exceed) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::column_constraint_definition] = 2;

    std::string content { R"(CREATE TABLE t (c1 int PRIMARY KEY NOT NULL DEFAULT 1))" };
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, column_reference_ok) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::column_reference] = 2;

    std::string content { R"(INSERT INTO t (c1, c2) VALUES (1, 2))" };
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::insert_statement>(*r.value()->statements()[0]);
    ASSERT_EQ(s.columns().size(), 2);
}

TEST_F(sql_parser_limit_test, column_reference_exceed) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::column_reference] = 2;

    std::string content { R"(INSERT INTO t (c1, c2, c3) VALUES (1, 2, 3))" };
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, storage_parameter_ok) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::storage_parameter] = 2;

    std::string content { R"(CREATE TABLE t (c1 int) WITH (p1, p2))" };
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::table_definition>(*r.value()->statements()[0]);
    ASSERT_EQ(s.parameters().size(), 2);
}

TEST_F(sql_parser_limit_test, storage_parameter_exceed) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::storage_parameter] = 2;

    std::string content { R"(CREATE TABLE t (c1 int) WITH (p1, p2, p3))" };
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, set_clause_ok) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::set_clause] = 2;

    std::string content { R"(UPDATE t SET c1=1, c2=2)" };
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::update_statement>(*r.value()->statements()[0]);
    ASSERT_EQ(s.elements().size(), 2);
}

TEST_F(sql_parser_limit_test, set_clause_exceed) {
    sql_parser parser {};
    parser.options().element_limits()[element_kind::set_clause] = 2;

    std::string content { R"(UPDATE t SET c1=1, c2=2, c3=3)" };
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, with_element_ok) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::with_element] = 2;

    std::string content{R"(WITH q1 AS (SELECT 1), q2 AS (SELECT 2) SELECT 0)"};
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::select_statement>(*r.value()->statements()[0]);
    auto&& q = downcast<ast::query::with_expression>(*s.expression());
    ASSERT_EQ(q.elements().size(), 2);
}

TEST_F(sql_parser_limit_test, with_element_exceed) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::with_element] = 2;

    std::string content{R"(WITH q1 AS (SELECT 1), q2 AS (SELECT 2), q3 AS (SELECT 3) SELECT 0)"};
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, select_element_ok) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::select_element] = 2;

    std::string content{R"(SELECT c1, c2)"};
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::select_statement>(*r.value()->statements()[0]);
    auto&& q = downcast<ast::query::query>(*s.expression());
    ASSERT_EQ(q.elements().size(), 2);
}

TEST_F(sql_parser_limit_test, select_element_exceed) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::select_element] = 2;

    std::string content{R"(SELECT c1, c2, c3)"};
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, table_reference_ok) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::table_reference] = 2;

    std::string content{R"(SELECT * FROM t1, t2)"};
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::select_statement>(*r.value()->statements()[0]);
    auto&& q = downcast<ast::query::query>(*s.expression());
    ASSERT_EQ(q.from().size(), 2);
}

TEST_F(sql_parser_limit_test, table_reference_exceed) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::table_reference] = 2;

    std::string content{R"(SELECT * FROM t1, t2, t3)"};
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, grouping_element_ok) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::grouping_element] = 2;

    std::string content{R"(SELECT * FROM t1 GROUP BY c1, c2)"};
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::select_statement>(*r.value()->statements()[0]);
    auto&& q = downcast<ast::query::query>(*s.expression());
    ASSERT_EQ(q.group_by()->elements().size(), 2);
}

TEST_F(sql_parser_limit_test, grouping_element_exceed) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::grouping_element] = 2;

    std::string content{R"(SELECT * FROM t1 GROUP BY c1, c2, c3)"};
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, ordering_element_ok) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::ordering_element] = 2;

    std::string content{R"(SELECT * FROM t1 ORDER BY c1, c2)"};
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::select_statement>(*r.value()->statements()[0]);
    auto&& q = downcast<ast::query::query>(*s.expression());
    ASSERT_EQ(q.order_by().size(), 2);
}

TEST_F(sql_parser_limit_test, ordering_element_exceed) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::ordering_element] = 2;

    std::string content{R"(SELECT * FROM t1 ORDER BY c1, c2, c3)"};
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, field_definition_ok) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::field_definition] = 2;

    std::string content{R"(VALUES (x::ROW(c1 int, c2 int)))"};
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::select_statement>(*r.value()->statements()[0]);
    auto&& q = downcast<ast::query::table_value_constructor>(*s.expression());
    auto&& e = downcast<ast::scalar::value_constructor>(*q.elements()[0]);
    auto&& c = downcast<ast::scalar::cast_expression>(*e.elements()[0]);
    auto&& t = downcast<ast::type::row>(*c.type());

    ASSERT_EQ(t.elements().size(), 2);
}

TEST_F(sql_parser_limit_test, field_definition_exceed) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::field_definition] = 2;

    std::string content{R"(VALUES (x::ROW(c1 int, c2 int, c3 int)))"};
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, row_expression_ok) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::row_expression] = 2;

    std::string content{R"(VALUES (1), (2))"};
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::select_statement>(*r.value()->statements()[0]);
    auto&& q = downcast<ast::query::table_value_constructor>(*s.expression());

    ASSERT_EQ(q.elements().size(), 2);
}

TEST_F(sql_parser_limit_test, row_expression_exceed) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::row_expression] = 2;

    std::string content{R"(VALUES (1), (2), (3))"};
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, scalar_expression_ok) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::scalar_expression] = 2;

    std::string content{R"(VALUES (1, 2))"};
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::select_statement>(*r.value()->statements()[0]);
    auto&& q = downcast<ast::query::table_value_constructor>(*s.expression());
    auto&& e = downcast<ast::scalar::value_constructor>(*q.elements()[0]);

    ASSERT_EQ(e.elements().size(), 2);
}

TEST_F(sql_parser_limit_test, scalar_expression_exceed) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::scalar_expression] = 2;

    std::string content{R"(VALUES (1, 2, 3))"};
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, when_clause_ok) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::when_clause] = 2;

    std::string content{R"(VALUES (CASE WHEN c1 THEN d1 WHEN c2 THEN d2 END))"};
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::select_statement>(*r.value()->statements()[0]);
    auto&& q = downcast<ast::query::table_value_constructor>(*s.expression());
    auto&& x = downcast<ast::scalar::value_constructor>(*q.elements()[0]);
    auto&& e = downcast<ast::scalar::case_expression>(*x.elements()[0]);

    ASSERT_EQ(e.when_clauses().size(), 2);
}

TEST_F(sql_parser_limit_test, when_clause_exceed) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::when_clause] = 2;

    std::string content{R"(VALUES (CASE WHEN c1 THEN d1 WHEN c2 THEN d2 WHEN c3 THEN d3 END))"};
    auto r = parser("-", content);
    expect_exceed(r);
}

TEST_F(sql_parser_limit_test, string_literal_concatenation_ok) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::string_literal_concatenation] = 2;

    std::string content{R"(VALUES ('c0' 'c1' 'c2'))"};
    auto r = parser("-", content);
    ASSERT_TRUE(r.has_value());

    auto&& s = downcast<ast::statement::select_statement>(*r.value()->statements()[0]);
    auto&& q = downcast<ast::query::table_value_constructor>(*s.expression());
    auto&& x = downcast<ast::scalar::value_constructor>(*q.elements()[0]);
    auto&& e = downcast<ast::scalar::literal_expression>(*x.elements()[0]);
    auto&& l = downcast<ast::literal::string>(*e.value());

    ASSERT_EQ(l.concatenations().size(), 2);
}

TEST_F(sql_parser_limit_test, string_literal_concatenation_exceed) {
    sql_parser parser{};
    parser.options().element_limits()[element_kind::string_literal_concatenation] = 2;

    std::string content{R"(VALUES ('c0' 'c1' 'c2' 'c3'))"};
    auto r = parser("-", content);
    expect_exceed(r);
}

} // namespace mizugaki::parser
