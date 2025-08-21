#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/drop_table.h>
#include <takatori/statement/drop_index.h>
#include <takatori/statement/grant_table.h>
#include <takatori/statement/revoke_table.h>

#include <yugawara/binding/extract.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>

#include <mizugaki/ast/statement/drop_statement.h>
#include <mizugaki/ast/statement/grant_privilege_statement.h>
#include <mizugaki/ast/statement/revoke_privilege_statement.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_ddl_test : public test_parent {
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
};

TEST_F(analyze_statement_ddl_test, drop_table_simple) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("TESTING"),
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_table);

    auto&& stmt = downcast<tstatement::drop_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract(stmt.target());
    EXPECT_EQ(&target, table.get());
}

TEST_F(analyze_statement_ddl_test, drop_table_not_found) {
    invalid(sql_analyzer_code::table_not_found, ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("MISSING"),
    });
}

TEST_F(analyze_statement_ddl_test, drop_table_not_found_if_exists) {
    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("MISSING"),
            {
                    ast::statement::drop_statement_option::if_exists,
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_ddl_test, drop_table_restrict) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("testing"),
            {
                    ast::statement::drop_statement_option::restrict,
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_table);

    auto&& stmt = downcast<tstatement::drop_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract(stmt.target());
    EXPECT_EQ(&target, table.get());
}

TEST_F(analyze_statement_ddl_test, drop_table_cascade) {
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("MISSING"),
            {
                    ast::statement::drop_statement_option::cascade,
            },
    });
}

TEST_F(analyze_statement_ddl_test, drop_index_simple) {
    auto table = install_table("testing");
    auto index = storages_->add_index(::yugawara::storage::index {
            table,
            "testing_index",
            {
                    table->columns()[1],
            }
    });

    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("TESTING_INDEX"),
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_index);

    auto&& stmt = downcast<tstatement::drop_index>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract<::yugawara::storage::index>(stmt.target());
    EXPECT_EQ(&target, index.get());
}

TEST_F(analyze_statement_ddl_test, drop_index_not_found) {
    invalid(sql_analyzer_code::index_not_found, ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("MISSING"),
    });
}

TEST_F(analyze_statement_ddl_test, drop_index_not_found_if_exists) {
    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("testing_index"),
            {
                    ast::statement::drop_statement_option::if_exists,
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_ddl_test, drop_index_restrict) {
    auto table = install_table("testing");
    auto index = storages_->add_index(::yugawara::storage::index {
            table,
            "testing_index",
            {
                    table->columns()[1],
            }
    });

    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("testing_index"),
            {
                    ast::statement::drop_statement_option::restrict,
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_index);

    auto&& stmt = downcast<tstatement::drop_index>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract<::yugawara::storage::index>(stmt.target());
    EXPECT_EQ(&target, index.get());
}

TEST_F(analyze_statement_ddl_test, drop_index_cascade) {
    auto table = install_table("testing");
    auto index = storages_->add_index(::yugawara::storage::index {
            table,
            "testing_index",
            {
                    table->columns()[1],
            }
    });
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("testing_index"),
            {
                    ast::statement::drop_statement_option::cascade,
            },
    });
}

TEST_F(analyze_statement_ddl_test, drop_statement_restrict_vs_cascade) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::malformed_syntax, ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("testing"),
            {
                    ast::statement::drop_statement_option::restrict,
                    ast::statement::drop_statement_option::cascade,
            },
    });
}

TEST_F(analyze_statement_ddl_test, grant_table_simple) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::select },
            },
            {
                    id("testing"),
            },
            {
                    id("u"),
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::grant_table);

    auto&& stmt = downcast<tstatement::grant_table>(**alternative);
    auto&& elems = stmt.elements();
    ASSERT_EQ(elems.size(), 1);

    {
        auto&& e = elems[0];
        EXPECT_EQ(extract(e.table()), *table);

        auto&& ps = e.default_privileges();
        ASSERT_EQ(ps.size(), 0);

        auto&& as = e.authorization_entries();
        ASSERT_EQ(as.size(), 1);
        {
            auto&& a = as[0];
            EXPECT_EQ(a.authorization_identifier(), "u");

            auto&& aps = a.privileges();
            ASSERT_EQ(aps.size(), 1);
            EXPECT_EQ(aps[0].action_kind(), tstatement::table_action_kind::select);
        }
    }
}

TEST_F(analyze_statement_ddl_test, grant_table_table) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::select },
            },
            {
                    {
                            ast::statement::privilege_object_kind::table,
                            id("testing"),
                    },
            },
            {
                    id("u"),
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::grant_table);

    auto&& stmt = downcast<tstatement::grant_table>(**alternative);
    auto&& elems = stmt.elements();
    ASSERT_EQ(elems.size(), 1);
}

TEST_F(analyze_statement_ddl_test, grant_table_public) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::all_privileges },
            },
            {
                    id("testing"),
            },
            {
                    {},
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::grant_table);

    auto&& stmt = downcast<tstatement::grant_table>(**alternative);
    std::cout << stmt << std::endl;

    auto&& elems = stmt.elements();
    ASSERT_EQ(elems.size(), 1);

    {
        auto&& e = elems[0];
        EXPECT_EQ(extract(e.table()), *table);

        auto&& ps = e.default_privileges();
        ASSERT_EQ(ps.size(), 1);
        EXPECT_EQ(ps[0].action_kind(), tstatement::table_action_kind::control);

        auto&& as = e.authorization_entries();
        ASSERT_EQ(as.size(), 0);
    }
}

TEST_F(analyze_statement_ddl_test, grant_table_multiple_actions) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::insert },
                    { ast::statement::privilege_action_kind::update },
                    { ast::statement::privilege_action_kind::delete_ },
            },
            {
                    id("testing"),
            },
            {
                    id("u"),
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::grant_table);

    auto&& stmt = downcast<tstatement::grant_table>(**alternative);
    auto&& elems = stmt.elements();
    ASSERT_EQ(elems.size(), 1);

    {
        auto&& e = elems[0];
        EXPECT_EQ(extract(e.table()), *table);

        auto&& ps = e.default_privileges();
        ASSERT_EQ(ps.size(), 0);

        auto&& as = e.authorization_entries();
        ASSERT_EQ(as.size(), 1);
        {
            auto&& a = as[0];
            EXPECT_EQ(a.authorization_identifier(), "u");

            auto&& aps = a.privileges();
            ASSERT_EQ(aps.size(), 3);
            EXPECT_EQ(aps[0].action_kind(), tstatement::table_action_kind::insert);
            EXPECT_EQ(aps[1].action_kind(), tstatement::table_action_kind::update);
            EXPECT_EQ(aps[2].action_kind(), tstatement::table_action_kind::delete_);
        }
    }
}

TEST_F(analyze_statement_ddl_test, grant_table_multiple_tables) {
    auto t1 = install_table("t1");
    auto t2 = install_table("t2");
    auto t3 = install_table("t3");
    auto r = analyze_statement(context(), ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::select },
            },
            {
                    id("t1"),
                    id("t2"),
                    id("t3"),
            },
            {
                    id("u"),
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::grant_table);

    auto&& stmt = downcast<tstatement::grant_table>(**alternative);
    auto&& elems = stmt.elements();
    ASSERT_EQ(elems.size(), 3);

    {
        auto&& e = elems[0];
        EXPECT_EQ(extract(e.table()), *t1);

        auto&& ps = e.default_privileges();
        ASSERT_EQ(ps.size(), 0);

        auto&& as = e.authorization_entries();
        ASSERT_EQ(as.size(), 1);
    }
    {
        auto&& e = elems[1];
        EXPECT_EQ(extract(e.table()), *t2);

        auto&& ps = e.default_privileges();
        ASSERT_EQ(ps.size(), 0);

        auto&& as = e.authorization_entries();
        ASSERT_EQ(as.size(), 1);
    }
    {
        auto&& e = elems[2];
        EXPECT_EQ(extract(e.table()), *t3);

        auto&& ps = e.default_privileges();
        ASSERT_EQ(ps.size(), 0);

        auto&& as = e.authorization_entries();
        ASSERT_EQ(as.size(), 1);
    }
}

TEST_F(analyze_statement_ddl_test, grant_table_multiple_tables_deduplication) {
    auto t1 = install_table("t1");
    auto r = analyze_statement(context(), ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::select },
            },
            {
                    id("t1"),
                    id("t1"),
                    id("t1"),
            },
            {
                    id("u"),
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::grant_table);

    auto&& stmt = downcast<tstatement::grant_table>(**alternative);
    auto&& elems = stmt.elements();
    ASSERT_EQ(elems.size(), 1);

    {
        auto&& e = elems[0];
        EXPECT_EQ(extract(e.table()), *t1);

        auto&& ps = e.default_privileges();
        ASSERT_EQ(ps.size(), 0);

        auto&& as = e.authorization_entries();
        ASSERT_EQ(as.size(), 1);
    }
}

TEST_F(analyze_statement_ddl_test, grant_table_multiple_users) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::select },
            },
            {
                    id("testing"),
            },
            {
                    id("u"),
                    id("v"),
                    id("w"),
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::grant_table);

    auto&& stmt = downcast<tstatement::grant_table>(**alternative);
    auto&& elems = stmt.elements();
    ASSERT_EQ(elems.size(), 1);

    {
        auto&& e = elems[0];
        EXPECT_EQ(extract(e.table()), *table);

        auto&& ps = e.default_privileges();
        ASSERT_EQ(ps.size(), 0);

        auto&& as = e.authorization_entries();
        ASSERT_EQ(as.size(), 3);
        {
            auto&& a = as[0];
            EXPECT_EQ(a.authorization_identifier(), "u");

            auto&& aps = a.privileges();
            ASSERT_EQ(aps.size(), 1);
        }
        {
            auto&& a = as[1];
            EXPECT_EQ(a.authorization_identifier(), "v");

            auto&& aps = a.privileges();
            ASSERT_EQ(aps.size(), 1);
        }
        {
            auto&& a = as[2];
            EXPECT_EQ(a.authorization_identifier(), "w");

            auto&& aps = a.privileges();
            ASSERT_EQ(aps.size(), 1);
        }
    }
}

TEST_F(analyze_statement_ddl_test, grant_table_multiple_users_deduplication) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::select },
            },
            {
                    id("testing"),
            },
            {
                    id("u"),
                    id("u"),
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::grant_table);

    auto&& stmt = downcast<tstatement::grant_table>(**alternative);
    auto&& elems = stmt.elements();
    ASSERT_EQ(elems.size(), 1);

    {
        auto&& e = elems[0];
        EXPECT_EQ(extract(e.table()), *table);

        auto&& ps = e.default_privileges();
        ASSERT_EQ(ps.size(), 0);

        auto&& as = e.authorization_entries();
        ASSERT_EQ(as.size(), 1);
        {
            auto&& a = as[0];
            EXPECT_EQ(a.authorization_identifier(), "u");

            auto&& aps = a.privileges();
            ASSERT_EQ(aps.size(), 1);
        }
    }
}

TEST_F(analyze_statement_ddl_test, grant_empty_actions) {
    auto table = install_table("dummy");
    invalid(sql_analyzer_code::malformed_syntax, ast::statement::grant_privilege_statement {
            {},
            {
                    id("dummy"),
            },
            {
                    id("u"),
            },
    });
}

TEST_F(analyze_statement_ddl_test, grant_empty_objects) {
    invalid(sql_analyzer_code::malformed_syntax, ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::all_privileges },
            },
            {},
            {
                    id("u"),
            },
    });
}

TEST_F(analyze_statement_ddl_test, grant_empty_users) {
    auto table = install_table("dummy");
    invalid(sql_analyzer_code::malformed_syntax, ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::all_privileges },
            },
            {
                    id("dummy"),
            },
            {},
    });
}

TEST_F(analyze_statement_ddl_test, grant_table_not_found) {
    invalid(sql_analyzer_code::table_not_found, ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::select },
            },
            {
                    id("MISSING"),
            },
            {
                    id("u"),
            },
    });
}

TEST_F(analyze_statement_ddl_test, grant_mixed_objects) {
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::all_privileges },
            },
            {
                    {
                            ast::statement::privilege_object_kind::table,
                            id("t"),
                    },
                    {
                            ast::statement::privilege_object_kind::schema,
                            id("s"),
                    },
            },
            {
                    id("u"),
            },
    });
}

TEST_F(analyze_statement_ddl_test, grant_schema) {
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::grant_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::all_privileges },
            },
            {
                    {
                            ast::statement::privilege_object_kind::schema,
                            id("s"),
                    },
            },
            {
                    id("u"),
            },
    });
}

TEST_F(analyze_statement_ddl_test, revoke_table_simple) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::revoke_privilege_statement {
            {
                    { ast::statement::privilege_action_kind::select },
            },
            {
                    id("testing"),
            },
            {
                    id("u"),
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::revoke_table);

    auto&& stmt = downcast<tstatement::revoke_table>(**alternative);
    auto&& elems = stmt.elements();
    ASSERT_EQ(elems.size(), 1);

    {
        auto&& e = elems[0];
        EXPECT_EQ(extract(e.table()), *table);

        auto&& ps = e.default_privileges();
        ASSERT_EQ(ps.size(), 0);

        auto&& as = e.authorization_entries();
        ASSERT_EQ(as.size(), 1);
        {
            auto&& a = as[0];
            EXPECT_EQ(a.authorization_identifier(), "u");

            auto&& aps = a.privileges();
            ASSERT_EQ(aps.size(), 1);
            EXPECT_EQ(aps[0].action_kind(), tstatement::table_action_kind::select);
        }
    }
}

} // namespace mizugaki::analyzer::details
