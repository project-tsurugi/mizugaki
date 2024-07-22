#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/create_table.h>

#include <yugawara/binding/factory.h>
#include <yugawara/binding/extract.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/sequence.h>
#include <yugawara/storage/column.h>

#include <mizugaki/ast/type/simple.h>
#include <mizugaki/ast/type/decimal.h>

#include <mizugaki/ast/scalar/unary_expression.h>

#include <mizugaki/ast/statement/table_definition.h>
#include <mizugaki/ast/statement/column_definition.h>

#include <mizugaki/ast/statement/expression_constraint.h>
#include <mizugaki/ast/statement/identity_constraint.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_identity_column_test : public test_parent {
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

TEST_F(analyze_statement_identity_column_test, always) {
    options_.default_sequence_cycle() = true;
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    EXPECT_TRUE(table_columns[0].features().contains(::yugawara::storage::column_feature::read_only));

    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_EQ(seq.definition_id(), std::nullopt);
    EXPECT_EQ(seq.simple_name(), "");
    EXPECT_EQ(seq.initial_value(), 1);
    EXPECT_EQ(seq.increment_value(), 1);
    EXPECT_EQ(seq.min_value(), 1);
    EXPECT_EQ(seq.max_value(), std::numeric_limits<std::int64_t>::max());
    EXPECT_TRUE(seq.cycle());
}

TEST_F(analyze_statement_identity_column_test, by_default) {
    options_.default_sequence_cycle() = true;
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::by_default,
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    EXPECT_FALSE(table_columns[0].features().contains(::yugawara::storage::column_feature::read_only));

    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_EQ(seq.definition_id(), std::nullopt);
    EXPECT_EQ(seq.simple_name(), "");
    EXPECT_EQ(seq.initial_value(), 1);
    EXPECT_EQ(seq.increment_value(), 1);
    EXPECT_EQ(seq.min_value(), 1);
    EXPECT_EQ(seq.max_value(), std::numeric_limits<std::int64_t>::max());
    EXPECT_TRUE(seq.cycle());
}

TEST_F(analyze_statement_identity_column_test, bigint_positive) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_EQ(seq.increment_value(), +1);
    EXPECT_EQ(seq.initial_value(), +1);
    EXPECT_EQ(seq.min_value(), +1);
    EXPECT_EQ(seq.max_value(), std::numeric_limits<std::int64_t>::max());
}

TEST_F(analyze_statement_identity_column_test, bigint_negative) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
                                            literal(number("1", ast::literal::sign::minus)),
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_EQ(seq.increment_value(), -1);
    EXPECT_EQ(seq.initial_value(), -1);
    EXPECT_EQ(seq.max_value(), -1);
    EXPECT_EQ(seq.min_value(), std::numeric_limits<std::int64_t>::min());
}

TEST_F(analyze_statement_identity_column_test, int_positive) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_EQ(seq.increment_value(), +1);
    EXPECT_EQ(seq.initial_value(), +1);
    EXPECT_EQ(seq.min_value(), +1);
    EXPECT_EQ(seq.max_value(), std::numeric_limits<std::int32_t>::max());
}

TEST_F(analyze_statement_identity_column_test, int_negative) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
                                            literal(number("1", ast::literal::sign::minus)),
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_EQ(seq.increment_value(), -1);
    EXPECT_EQ(seq.initial_value(), -1);
    EXPECT_EQ(seq.max_value(), -1);
    EXPECT_EQ(seq.min_value(), std::numeric_limits<std::int32_t>::min());
}

TEST_F(analyze_statement_identity_column_test, start_with) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            literal(number("10")),
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_EQ(seq.initial_value(), 10);
}

TEST_F(analyze_statement_identity_column_test, increment_by_positive) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
                                            literal(number("10")),
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_EQ(seq.increment_value(), 10);
    EXPECT_EQ(seq.max_value(), std::numeric_limits<std::int64_t>::max());
    EXPECT_EQ(seq.min_value(), 1);
}

TEST_F(analyze_statement_identity_column_test, increment_by_negative) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
                                            literal(number("10", ast::literal::sign::minus)),
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_EQ(seq.increment_value(), -10);
    EXPECT_EQ(seq.max_value(), -1);
    EXPECT_EQ(seq.min_value(), std::numeric_limits<std::int64_t>::min());
}

TEST_F(analyze_statement_identity_column_test, minvalue) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
                                            literal(number("1", ast::literal::sign::plus)),
                                            literal(number("9999", ast::literal::sign::minus)),
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_EQ(seq.initial_value(), -9999);
    EXPECT_EQ(seq.min_value(), -9999);
}

TEST_F(analyze_statement_identity_column_test, maxvalue) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
                                            literal(number("1", ast::literal::sign::minus)),
                                            {},
                                            literal(number("9999", ast::literal::sign::plus)),
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_EQ(seq.initial_value(), +9999);
    EXPECT_EQ(seq.max_value(), +9999);
}

TEST_F(analyze_statement_identity_column_test, cycle) {
    options_.default_sequence_cycle() = false;
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            std::unique_ptr<ast::scalar::expression> {},
                                            {},
                                            {},
                                            {},
                                            true,
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_TRUE(seq.cycle());
}

TEST_F(analyze_statement_identity_column_test, nocycle) {
    options_.default_sequence_cycle() = true;
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            std::unique_ptr<ast::scalar::expression> {},
                                            {},
                                            {},
                                            {},
                                            false,
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_FALSE(seq.cycle());
}

TEST_F(analyze_statement_identity_column_test, cycle_default) {
    options_.default_sequence_cycle() = false;
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
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
    auto&& table = extract(stmt.definition());
    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& column_value = table_columns[0].default_value();
    ASSERT_EQ(column_value.kind(), ::yugawara::storage::column_value_kind::sequence);
    auto&& seq = *column_value.element<::yugawara::storage::column_value_kind::sequence>();

    EXPECT_FALSE(seq.cycle());
}

TEST_F(analyze_statement_identity_column_test, invalid_boundary_start_min) {
    invalid(sql_analyzer_code::invalid_sequence_value, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            literal(number("0", ast::literal::sign::plus)),
                                            {},
                                            literal(number("1", ast::literal::sign::plus)),
                                            literal(number("999", ast::literal::sign::plus)),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_identity_column_test, invalid_boundary_start_max) {
    invalid(sql_analyzer_code::invalid_sequence_value, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            literal(number("1000", ast::literal::sign::plus)),
                                            {},
                                            literal(number("1", ast::literal::sign::plus)),
                                            literal(number("999", ast::literal::sign::plus)),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_identity_column_test, invalid_overflow_min_int8) {
    invalid(sql_analyzer_code::invalid_sequence_value, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
                                            {},
                                            literal(number("9223372036854775809", ast::literal::sign::minus)),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_identity_column_test, invalid_overflow_max_int8) {
    invalid(sql_analyzer_code::invalid_sequence_value, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
                                            {},
                                            {},
                                            literal(number("9223372036854775808", ast::literal::sign::plus)),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_identity_column_test, invalid_overflow_min_int4) {
    invalid(sql_analyzer_code::invalid_sequence_value, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
                                            {},
                                            literal(number("2147483649", ast::literal::sign::minus)),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_identity_column_test, invalid_overflow_max_int4) {
    invalid(sql_analyzer_code::invalid_sequence_value, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
                                            {},
                                            {},
                                            literal(number("2147483648", ast::literal::sign::plus)),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_identity_column_test, invalid_increment) {
    invalid(sql_analyzer_code::invalid_sequence_value, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            {},
                                            literal(number("0", ast::literal::sign::plus)),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_identity_column_test, duplicate_identity) {
    invalid(sql_analyzer_code::invalid_constraint, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                    },
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::by_default,
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_identity_column_test, conflict_identity) {
    invalid(sql_analyzer_code::invalid_constraint, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::big_integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                    },
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            literal(number("1")),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_identity_column_test, unsupported_column_type) {
    invalid(sql_analyzer_code::inconsistent_type, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::decimal {
                                    ast::type::kind::decimal,
                                    10,
                                    0,
                            },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_statement_identity_column_test, invalid_decimal_part) {
    invalid(sql_analyzer_code::invalid_sequence_value, ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::identity_constraint {
                                            ast::statement::identity_generation_type::always,
                                            literal(number("3.14")),
                                    },
                            },
                    },
            },
    });
}

} // namespace mizugaki::analyzer::details
