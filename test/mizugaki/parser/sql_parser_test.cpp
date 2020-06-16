#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <takatori/util/string_builder.h>

#include <mizugaki/ast/compilation_unit.h>

#include <mizugaki/ast/statement/empty_statement.h>
#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/select_asterisk.h>
#include <mizugaki/ast/query/select_column.h>

#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/field_reference.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>

#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/literal/numeric.h>

namespace mizugaki::parser {

using namespace ast;

using ::takatori::util::object_creator;
using ::takatori::util::string_builder;

template<class T>
using node_ptr = ::takatori::util::unique_object_ptr<T>;

class sql_parser_test : public ::testing::Test {};

static statement::statement const& first(sql_parser::result_type const& result) {
    return *result.value()->statements().at(0);
}

static std::string diagnostics(sql_parser::result_type const& result) {
    if (auto&& d = result.diagnostic()) {
        return string_builder {}
                << d.message()
                << " : "
                << d.document()->contents(d.region().first(), d.region().size())
                << string_builder::to_string;
    }
    return {};
}

TEST_F(sql_parser_test, empty_document) {
    sql_parser parser;
    auto result = parser("-", "");
    ASSERT_TRUE(result);

    auto cu = std::move(result.value());
    EXPECT_EQ(*cu, (compilation_unit {}));
}

TEST_F(sql_parser_test, empty_statement) {
    sql_parser parser;
    auto result = parser("-", ";");
    ASSERT_TRUE(result);

    EXPECT_EQ(first(result), (statement::empty_statement {}));
}

TEST_F(sql_parser_test, select_asterisk) {
    sql_parser parser;
    auto result = parser("-", "SELECT * FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_asterisk {},
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T0" },
                            },
                    },
                    { // WHERE
                    }
            }
    }));
}

TEST_F(sql_parser_test, select_asterisk_qualified) {
    sql_parser parser;
    auto result = parser("-", "SELECT T0.* FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_asterisk {
                                    scalar::variable_reference {
                                            name::simple { "T0" },
                                    },
                            },
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T0" },
                            },
                    },
                    { // WHERE
                    }
            }
    }));
}

TEST_F(sql_parser_test, select_column) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_column {
                                    scalar::variable_reference {
                                            name::simple { "C0" },
                                    },
                            },
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T0" },
                            },
                    },
                    { // WHERE
                    }
            }
    }));
}

TEST_F(sql_parser_test, select_column_name) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 AS x0 FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_column {
                                    scalar::variable_reference {
                                            name::simple { "C0" },
                                    },
                                    name::simple { "x0" },
                            },
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T0" },
                            },
                    },
                    { // WHERE
                    }
            }
    }));
}

TEST_F(sql_parser_test, select_where) {
    sql_parser parser;
    parser.set_debug(1);
    auto result = parser("-", "SELECT * FROM T0 WHERE a < 0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_asterisk {},
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T0" },
                            },
                    },
                    { // WHERE
                            scalar::comparison_predicate {
                                    scalar::variable_reference {
                                            name::simple { "a" },
                                    },
                                    scalar::comparison_operator::less_than,
                                    scalar::literal_expression {
                                            literal::numeric {
                                                    literal::kind::exact_numeric,
                                                    std::nullopt,
                                                    { "0" },
                                            },
                                    },
                            },
                    }
            }
    }));
}

} // namespace mizugaki::parser
