#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/rename_table.h>
#include <takatori/statement/rename_index.h>
#include <takatori/statement/rename_column.h>
#include <takatori/statement/drop_partition.h>

#include <takatori/type/date.h>
#include <takatori/value/date.h>

#include <yugawara/binding/extract.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>

#include <mizugaki/ast/statement/alter_table_statement.h>
#include <mizugaki/ast/statement/alter_index_statement.h>
#include <mizugaki/ast/statement/rename_table_action.h>
#include <mizugaki/ast/statement/rename_index_action.h>
#include <mizugaki/ast/statement/rename_column_action.h>
#include <mizugaki/ast/statement/drop_partition_action.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/host_parameter_reference.h>
#include <mizugaki/ast/literal/datetime.h>
#include <mizugaki/ast/literal/special.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_alter_test : public test_parent {
protected:
    void invalid(ast::statement::statement const& stmt) {
        trelation::graph_type graph {};
        auto r = analyze_statement(context(), stmt);
        EXPECT_TRUE(std::holds_alternative<erroneous_result_type>(r)) << diagnostics();
        EXPECT_GT(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::statement::statement const& stmt) {
        invalid(stmt);
        EXPECT_TRUE(find_error(code)) << code << " <=> " << diagnostics();
    }

    // partition key: (division_id, event_date), primary key: (division_id, event_date, event_serial)
    static constexpr std::string_view partitioned_table_name = "d2";

    std::shared_ptr<::yugawara::storage::table> install_partitioned_table() {
        auto table = storages_->add_table(::yugawara::storage::table {
                partitioned_table_name,
                {
                        { "division_id", ttype::int4 {} },
                        { "event_date", ttype::date {} },
                        { "event_serial", ttype::int4 {} },
                },
        });
        ::yugawara::storage::index index {
                table,
                partitioned_table_name,
                {
                        table->columns()[0],
                        table->columns()[1],
                        table->columns()[2],
                },
                {},
                {
                        ::yugawara::storage::index_feature::find,
                        ::yugawara::storage::index_feature::scan,
                        ::yugawara::storage::index_feature::primary,
                        ::yugawara::storage::index_feature::unique,
                },
        };
        index.partition_key_columns().emplace_back(table->columns()[0]);
        index.partition_key_columns().emplace_back(table->columns()[1]);
        storages_->add_index(std::move(index));
        return table;
    }

    static ast::statement::alter_table_statement drop_partition(
            std::initializer_list<ast::statement::partition_criteria> target,
            std::string_view table_name = partitioned_table_name) {
        return ast::statement::alter_table_statement {
                id(table_name),
                ast::statement::drop_partition_action { target },
                false,
        };
    }

    static ast::scalar::comparison_predicate compare(
            ast::scalar::expression&& left,
            ast::scalar::comparison_operator operator_kind,
            ast::scalar::expression&& right) {
        return ast::scalar::comparison_predicate {
                std::move(left),
                operator_kind,
                std::move(right),
        };
    }

    static ast::scalar::literal_expression date(char const* token) {
        return literal(ast::literal::datetime {
                ast::literal::kind::date,
                token,
        });
    }

    tstatement::drop_partition const& extract_drop_partition(analyze_statement_result_type const& r) {
        auto alternative = std::get_if<statement_result_type>(&r);
        if (!alternative) {
            throw std::domain_error(diagnostics());
        }
        return downcast<tstatement::drop_partition>(**alternative);
    }
};

TEST_F(analyze_statement_alter_test, alter_table_rename_table_simple) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_table_action {
                    id("replacement")
            },
            false,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::rename_table);
    auto&& stmt = downcast<tstatement::rename_table>(**alternative);

    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract(stmt.target());
    EXPECT_EQ(target, *table);

    EXPECT_EQ("replacement", stmt.replacement());
}

TEST_F(analyze_statement_alter_test, alter_table_rename_table_not_found) {
    invalid(sql_analyzer_code::table_not_found, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_table_action {
                    id("replacement")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_table_not_found_if_exists) {
    auto r = analyze_statement(context(), ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_table_action {
                    id("replacement")
            },
            true,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_alter_test, alter_table_rename_table_conflict) {
    install_table("testing");
    install_table("conflict");
    invalid(sql_analyzer_code::table_already_exists, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_table_action {
                    id("conflict")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_table_itself) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::table_already_exists, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_table_action {
                    id("testing")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_index_simple) {
    auto table = install_table("testing");
    auto index = storages_->add_index(::yugawara::storage::index {
            table,
            "idx",
            {
                    {
                            table->columns()[1],
                            ::yugawara::storage::index::key::direction_type::ascendant,
                    },
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::unique,
            },
    });
    auto r = analyze_statement(context(), ast::statement::alter_index_statement {
            id("idx"),
            ast::statement::rename_index_action {
                    id("replacement")
            },
            false,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::rename_index);
    auto&& stmt = downcast<tstatement::rename_index>(**alternative);

    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract<::yugawara::storage::index>(stmt.target());
    EXPECT_EQ(target, *index);

    EXPECT_EQ("replacement", stmt.replacement());
}

TEST_F(analyze_statement_alter_test, alter_index_rename_index_not_found) {
    invalid(sql_analyzer_code::index_not_found, ast::statement::alter_index_statement {
            id("testing"),
            ast::statement::rename_index_action {
                    id("replacement")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_index_rename_index_not_found_if_exists) {
    auto r = analyze_statement(context(), ast::statement::alter_index_statement {
            id("testing"),
            ast::statement::rename_index_action {
                    id("replacement")
            },
            true,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_alter_test, alter_index_rename_index_conflict) {
    auto table = install_table("testing");
    storages_->add_index(::yugawara::storage::index {
            table,
            "idx",
            {
                    {
                            table->columns()[1],
                            ::yugawara::storage::index::key::direction_type::ascendant,
                    },
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::unique,
            },
    });
    storages_->add_index(::yugawara::storage::index {
            table,
            "conflict",
            {
                    {
                            table->columns()[2],
                            ::yugawara::storage::index::key::direction_type::ascendant,
                    },
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::unique,
            },
    });
    invalid(sql_analyzer_code::index_already_exists, ast::statement::alter_index_statement {
            id("idx"),
            ast::statement::rename_index_action {
                    id("conflict")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_index_rename_index_itself) {
    auto table = install_table("testing");
    storages_->add_index(::yugawara::storage::index {
            table,
            "idx",
            {
                    {
                            table->columns()[1],
                            ::yugawara::storage::index::key::direction_type::ascendant,
                    },
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::unique,
            },
    });
    invalid(sql_analyzer_code::index_already_exists, ast::statement::alter_index_statement {
            id("idx"),
            ast::statement::rename_index_action {
                    id("idx")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_index_rename_index_primary_index) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::alter_index_statement {
            id("testing"),
            ast::statement::rename_index_action {
                    id("replacement")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_simple) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("v"),
                    id("replacement"),
                    false,
            },
            false,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::rename_column);
    auto&& stmt = downcast<tstatement::rename_column>(**alternative);

    EXPECT_EQ(extract(stmt.table()), *table);

    auto&& target = extract<::yugawara::storage::column>(stmt.target());
    EXPECT_EQ(target, table->columns()[1]);

    EXPECT_EQ("replacement", stmt.replacement());
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_table_not_found) {
    invalid(sql_analyzer_code::table_not_found, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("v"),
                    id("replacement"),
                    false,
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_table_not_found_if_exists) {
    auto r = analyze_statement(context(), ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("v"),
                    id("replacement"),
                    false,
            },
            true,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_column_not_found) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::column_not_found, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("missing"),
                    id("replacement"),
                    false,
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_column_not_found_if_exists) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("missing"),
                    id("replacement"),
                    true,
            },
            false,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_conflict) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::column_already_exists, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("v"),
                    id("w"), // conflict
                    false,
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_itself) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::column_already_exists, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("v"),
                    id("v"),
                    false,
            },
            false,
    });
}



using op = ast::scalar::comparison_operator;
using top = ::takatori::scalar::comparison_operator;

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_simple) {
    auto table = install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(number("1"))),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    auto&& index = extract<::yugawara::storage::index>(stmt.index());
    EXPECT_EQ(&index.table(), table.get());
    EXPECT_TRUE(index.features().contains(::yugawara::storage::index_feature::primary));

    ASSERT_EQ(stmt.target().size(), 1);
    auto&& elements = stmt.target()[0].elements();
    ASSERT_EQ(elements.size(), 1);
    EXPECT_EQ(extract<::yugawara::storage::column>(elements[0].column()), table->columns()[0]);
    EXPECT_EQ(elements[0].column_name(), "division_id");
    ASSERT_EQ(elements[0].predicates().size(), 1);
    EXPECT_EQ(elements[0].predicates()[0].first, top::equal);
    EXPECT_EQ(*elements[0].predicates()[0].second, tvalue::int8 { 1 });
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_multiple_columns) {
    auto table = install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(number("1"))),
                    compare(vref(id("event_date")), op::equals, date("'2000-01-01'")),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    ASSERT_EQ(stmt.target().size(), 1);
    auto&& elements = stmt.target()[0].elements();
    ASSERT_EQ(elements.size(), 2);
    EXPECT_EQ(elements[0].column_name(), "division_id");
    EXPECT_EQ(elements[1].column_name(), "event_date");
    EXPECT_EQ(extract<::yugawara::storage::column>(elements[1].column()), table->columns()[1]);
    ASSERT_EQ(elements[1].predicates().size(), 1);
    EXPECT_EQ(elements[1].predicates()[0].first, top::equal);
    EXPECT_EQ(*elements[1].predicates()[0].second, (tvalue::date { 2000, 1, 1 }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_elements_in_key_order) {
    install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("event_date")), op::equals, date("'2000-01-01'")),
                    compare(vref(id("division_id")), op::equals, literal(number("1"))),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    auto&& elements = stmt.target()[0].elements();
    ASSERT_EQ(elements.size(), 2);
    EXPECT_EQ(elements[0].column_name(), "division_id");
    EXPECT_EQ(elements[1].column_name(), "event_date");
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_prefix) {
    install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(number("1"))),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    auto&& elements = stmt.target()[0].elements();
    ASSERT_EQ(elements.size(), 1);
    EXPECT_EQ(elements[0].column_name(), "division_id");
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_range_on_last_column) {
    install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(number("1"))),
                    compare(vref(id("event_date")), op::greater_than_or_equals, date("'2000-01-01'")),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    auto&& elements = stmt.target()[0].elements();
    ASSERT_EQ(elements.size(), 2);
    ASSERT_EQ(elements[1].predicates().size(), 1);
    EXPECT_EQ(elements[1].predicates()[0].first, top::greater_equal);
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_range_on_middle_column) {
    install_partitioned_table();
    invalid(sql_analyzer_code::invalid_partition_key, drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::greater_than_or_equals, literal(number("1"))),
                    compare(vref(id("event_date")), op::greater_than_or_equals, date("'2000-01-01'")),
            } },
    }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_missing_first_column) {
    install_partitioned_table();
    invalid(sql_analyzer_code::invalid_partition_key, drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("event_date")), op::equals, date("'2000-01-01'")),
            } },
    }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_lower_and_upper) {
    install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::greater_than_or_equals, literal(number("1"))),
                    compare(vref(id("division_id")), op::less_than, literal(number("5"))),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    auto&& elements = stmt.target()[0].elements();
    ASSERT_EQ(elements.size(), 1);
    auto&& predicates = elements[0].predicates();
    ASSERT_EQ(predicates.size(), 2);
    EXPECT_EQ(predicates[0].first, top::greater_equal);
    EXPECT_EQ(*predicates[0].second, tvalue::int8 { 1 });
    EXPECT_EQ(predicates[1].first, top::less);
    EXPECT_EQ(*predicates[1].second, tvalue::int8 { 5 });
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_upper_and_lower) {
    install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::less_than_or_equals, literal(number("5"))),
                    compare(vref(id("division_id")), op::greater_than, literal(number("1"))),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    auto&& predicates = stmt.target()[0].elements()[0].predicates();
    ASSERT_EQ(predicates.size(), 2);
    EXPECT_EQ(predicates[0].first, top::greater);
    EXPECT_EQ(*predicates[0].second, tvalue::int8 { 1 });
    EXPECT_EQ(predicates[1].first, top::less_equal);
    EXPECT_EQ(*predicates[1].second, tvalue::int8 { 5 });
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_multiple_equalities) {
    install_partitioned_table();
    invalid(sql_analyzer_code::unsupported_feature, drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(number("1"))),
                    compare(vref(id("division_id")), op::equals, literal(number("2"))),
            } },
    }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_multiple_equalities_on_middle_column) {
    install_partitioned_table();
    invalid(sql_analyzer_code::unsupported_feature, drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(number("1"))),
                    compare(vref(id("division_id")), op::equals, literal(number("2"))),
                    compare(vref(id("event_date")), op::equals, date("'2000-01-01'")),
            } },
    }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_empty_criteria) {
    install_partitioned_table();
    invalid(sql_analyzer_code::malformed_syntax, drop_partition({
            ast::statement::partition_criteria {},
    }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_multiple_criteria) {
    install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(number("1"))),
            } },
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(number("2"))),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    ASSERT_EQ(stmt.target().size(), 2);
    EXPECT_EQ(*stmt.target()[0].elements()[0].predicates()[0].second, tvalue::int8 { 1 });
    EXPECT_EQ(*stmt.target()[1].elements()[0].predicates()[0].second, tvalue::int8 { 2 });
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_literal_first_equals) {
    install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(literal(number("1")), op::equals, vref(id("division_id"))),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    auto&& predicates = stmt.target()[0].elements()[0].predicates();
    ASSERT_EQ(predicates.size(), 1);
    EXPECT_EQ(predicates[0].first, top::equal);
    EXPECT_EQ(*predicates[0].second, tvalue::int8 { 1 });
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_literal_first_flip) {
    install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(literal(number("1")), op::less_than_or_equals, vref(id("division_id"))),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    auto&& predicates = stmt.target()[0].elements()[0].predicates();
    ASSERT_EQ(predicates.size(), 1);
    EXPECT_EQ(predicates[0].first, top::greater_equal);
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_not_equals) {
    install_partitioned_table();
    invalid(sql_analyzer_code::unsupported_feature, drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::not_equals, literal(number("1"))),
            } },
    }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_placeholder) {
    install_partitioned_table();
    host_parameters_.add({ "p", ttype::int4 {} });
    invalid(sql_analyzer_code::unsupported_feature, drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, ast::scalar::host_parameter_reference { id(":p") }),
            } },
    }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_null) {
    options_.allow_context_independent_null() = false;
    install_partitioned_table();
    invalid(sql_analyzer_code::missing_context_of_null, drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(ast::literal::null {})),
            } },
    }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_null_allow_independent) {
    options_.allow_context_independent_null() = true;
    install_partitioned_table();
    invalid(sql_analyzer_code::unsupported_feature, drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(ast::literal::null {})),
            } },
    }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_signed_number) {
    install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(number("1", ast::literal::sign::minus))),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    EXPECT_EQ(*stmt.target()[0].elements()[0].predicates()[0].second, tvalue::int8 { -1 });
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_value_not_converted) {
    install_partitioned_table();
    auto r = analyze_statement(context(), drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("division_id")), op::equals, literal(number("1"))),
                    compare(vref(id("event_date")), op::equals, literal(string("'2000-01-01'"))),
            } },
    }));
    auto&& stmt = extract_drop_partition(r);
    expect_no_error();

    // NOTE: the value is not converted to the column type here, the type is checked in yugawara
    EXPECT_EQ(*stmt.target()[0].elements()[1].predicates()[0].second, tvalue::character { "2000-01-01" });
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_table_not_partitioned) {
    install_table("testing");
    invalid(sql_analyzer_code::index_not_partitioned, drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("k")), op::equals, literal(number("1"))),
            } },
    }, "testing"));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_column_not_partition_key) {
    install_partitioned_table();
    invalid(sql_analyzer_code::column_not_found, drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("event_serial")), op::equals, literal(number("1"))),
            } },
    }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_column_not_found) {
    install_partitioned_table();
    invalid(sql_analyzer_code::column_not_found, drop_partition({
            ast::statement::partition_criteria { {
                    compare(vref(id("nothing")), op::equals, literal(number("1"))),
            } },
    }));
}

TEST_F(analyze_statement_alter_test, alter_table_drop_partition_table_not_found_if_exists) {
    auto r = analyze_statement(context(), ast::statement::alter_table_statement {
            id("nothing"),
            ast::statement::drop_partition_action { {
                    ast::statement::partition_criteria { {
                            compare(vref(id("a")), op::equals, literal(number("1"))),
                    } },
            } },
            true,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();
    EXPECT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

} // namespace mizugaki::analyzer::details
