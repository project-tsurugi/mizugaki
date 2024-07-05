#include <mizugaki/analyzer/details/analyze_name.h>

#include <gtest/gtest.h>

#include <yugawara/binding/factory.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_name_qualified_test : public test_parent {
protected:
    template<class T>
    void invalid(T const& result, sql_analyzer_code code) {
        EXPECT_FALSE(result);
        EXPECT_TRUE(find_error(code)) << code << " <=> " << diagnostics();
        clear_error();
    }

    void validate(std::unique_ptr<tscalar::expression> const& result, ::takatori::descriptor::variable expect) {
        if (!result) {
            ADD_FAILURE() << "expect is empty: " << diagnostics();
        } else {
            EXPECT_EQ(*result, vref(std::move(expect))) << diagnostics();
            expect_no_error();
        }
    }

    void validate(
            std::unique_ptr<tscalar::expression> const& result,
            std::shared_ptr<::yugawara::variable::declaration const> const& expect) {
        if (!result) {
            ADD_FAILURE() << "expect is empty: " << diagnostics();
        } else {
            ::yugawara::binding::factory f {};
            EXPECT_EQ(*result, vref(f(expect))) << diagnostics();
            expect_no_error();
        }
    }

    template<class T1, class T2>
    void validate(::takatori::util::optional_ptr<T1> result, find_element_result<T2> expect) {
        if (!expect.is_found()) {
            ADD_FAILURE() << "expect is " << expect.kind();
        } else {
            EXPECT_EQ(result.get(), std::addressof(expect.value())) << diagnostics();
            expect_no_error();
        }
    }

    template<class T1, class T2>
    void validate(::takatori::util::optional_ptr<T1> result, std::shared_ptr<T2> const& expect) {
        if (!expect) {
            ADD_FAILURE() << "expect is empty";
            return;
        }
        EXPECT_EQ(result.get(), expect.get()) << diagnostics();
        expect_no_error();
    }

    template<class T1, class T2>
    void validate(std::shared_ptr<T1> const& result, std::shared_ptr<T2> const& expect) {
        if (!expect) {
            ADD_FAILURE() << "expect is empty";
            return;
        }
        if (!result) {
            ADD_FAILURE() << diagnostics();
            return;
        }
        EXPECT_EQ(*result, *expect) << diagnostics();
        expect_no_error();
    }

    template<class T>
    void validate(
            std::optional<schema_element<T>> result,
            ::yugawara::schema::declaration const& expect_schema,
            std::shared_ptr<T> const& expect_table) {
        if (!result) {
            ADD_FAILURE() << diagnostics();
            return;
        }
        EXPECT_EQ(*result->first, expect_schema);
        EXPECT_EQ(*result->second, *expect_table);
        expect_no_error();
    }
};

TEST_F(analyze_name_qualified_test, column_variable_in_relation_info) {
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({ {}, "t0", });
    relation.add(column_info { {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("t0", "c0"),
            scope);
    validate(r, v);
}

TEST_F(analyze_name_qualified_test, column_variable_in_relation_info_multiple) {
    auto v0 = vdesc("v0");
    auto v1 = vdesc("v1");
    auto v2 = vdesc("v2");

    query_scope scope;
    auto&& relation = scope.add({ {}, "t0", });
    relation.add(column_info { {}, v0, "c0", });
    relation.add(column_info { {}, v1, "c1", });
    relation.add(column_info { {}, v2, "c2", });

    auto r = analyze_variable_name(
            context(),
            id("t0", "c1"),
            scope);
    validate(r, v1);
}

TEST_F(analyze_name_qualified_test, column_variable_in_relation_info_missing) {
    auto v0 = vdesc("v0");
    auto v1 = vdesc("v1");
    auto v2 = vdesc("v2");

    query_scope scope;
    auto&& relation = scope.add({ {}, "t0", });
    relation.add(column_info { {}, v0, "c0", });
    relation.add(column_info { {}, v1, "c1", });
    relation.add(column_info { {}, v2, "c2", });

    auto r = analyze_variable_name(
            context(),
            id("t0", "MISSING"),
            scope);
    invalid(r, diagnostic_code::column_not_found);
}

TEST_F(analyze_name_qualified_test, column_variable_in_relation_info_ambiguous) {
    auto v0 = vdesc("v0");
    auto v1 = vdesc("v1");
    auto v2 = vdesc("v2");

    query_scope scope;
    auto&& relation = scope.add({ {}, "t0", });
    relation.add(column_info { {}, v0, "c0", });
    relation.add(column_info { {}, v1, "c1", });
    relation.add(column_info { {}, v2, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("t0", "c0"),
            scope);
    invalid(r, diagnostic_code::column_ambiguous);
}

TEST_F(analyze_name_qualified_test, column_variable_in_relation_info_not_exported) {
    auto v0 = vdesc("v0");
    auto v1 = vdesc("v1");
    auto v2 = vdesc("v2");

    query_scope scope;
    auto&& relation = scope.add({ {}, "t0", });
    relation.add(column_info { {}, v0, "c0", });
    relation.add(column_info { {}, v1, "c1", false, });
    relation.add(column_info { {}, v2, "c2", });

    auto r = analyze_variable_name(
            context(),
            id("t0", "c1"),
            scope);
    invalid(r, diagnostic_code::column_not_found);
}

TEST_F(analyze_name_qualified_test, column_variable_in_relation_info_ambiguous_not_exported) {
    auto v0 = vdesc("v0");
    auto v1 = vdesc("v1");
    auto v2 = vdesc("v2");

    query_scope scope;
    auto&& relation = scope.add({ {}, "t0", });
    relation.add(column_info { {}, v0, "c0", false, });
    relation.add(column_info { {}, v1, "c1", });
    relation.add(column_info { {}, v2, "c0", true, });

    auto r = analyze_variable_name(
            context(),
            id("t0", "c0"),
            scope);
    validate(r, v2);
}

TEST_F(analyze_name_qualified_test, column_variable_in_relation_info_missing_qualifier) {
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({ {}, "r0", });
    relation.add(column_info { {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("MISSING", "c0"),
            scope);
    invalid(r, diagnostic_code::symbol_not_found);
}

TEST_F(analyze_name_qualified_test, column_variable_in_relation_info_ambiguous_qualifier) {
    auto v = vdesc();

    query_scope scope;
    auto&& r0 = scope.add({ {}, "r0", });
    r0.add(column_info { {}, v, "c0", });
    auto&& r1 = scope.add({ {}, "r0", });
    r1.add(column_info { {}, v, "c1", });

    auto r = analyze_variable_name(
            context(),
            id("r0", "c0"),
            scope);
    invalid(r, diagnostic_code::relation_ambiguous);
}

TEST_F(analyze_name_qualified_test, schema_variable_in_schema_decl) {
    auto sv0 = std::make_shared<::yugawara::variable::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, {}, sv0, });
    auto x0 = sv0->add({ "x0", ttype::int8 {} });

    auto r = analyze_variable_name(
            context(),
            id("s0", "x0"),
            {});
    validate(r, x0);
}

TEST_F(analyze_name_qualified_test, schema_variable_in_schema_decl_multiple_variables) {
    auto sv0 = std::make_shared<::yugawara::variable::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, {}, sv0, });
    auto x0 = sv0->add({ "x0", ttype::int8 {} });
    auto x1 = sv0->add({ "x1", ttype::int8 {} });
    auto x2 = sv0->add({ "x2", ttype::int8 {} });

    auto r = analyze_variable_name(
            context(),
            id("s0", "x1"),
            {});
    validate(r, x1);
}

TEST_F(analyze_name_qualified_test, schema_variable_in_schema_decl_missing) {
    auto sv0 = std::make_shared<::yugawara::variable::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, {}, sv0, });
    auto x0 = sv0->add({ "x0", ttype::int8 {} });
    auto x1 = sv0->add({ "x1", ttype::int8 {} });
    auto x2 = sv0->add({ "x2", ttype::int8 {} });

    auto r = analyze_variable_name(
            context(),
            id("s0", "MISSING"),
            {});
    invalid(r, sql_analyzer_code::variable_not_found);
}

TEST_F(analyze_name_qualified_test, relation_info_in_schema_decl) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });

    query_scope scope;
    scope.add({ *t0 });

    auto r = analyze_relation_info_name(
            context(),
            id("s0", "t0"),
            scope);
    validate(r, scope.find("t0"));
}

TEST_F(analyze_name_qualified_test, relation_info_in_schema_decl_multiple) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });
    auto t1 = ss0->add_table({ "t1", {} });
    auto t2 = ss0->add_table({ "t2", {} });

    query_scope scope;
    scope.add({ *t0 });
    scope.add({ *t1 });
    scope.add({ *t2 });

    auto r = analyze_relation_info_name(
            context(),
            id("s0", "t1"),
            scope);
    validate(r, scope.find("t1"));
}

TEST_F(analyze_name_qualified_test, relation_info_in_schema_decl_not_in_scope) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });
    auto t1 = ss0->add_table({ "t1", {} });
    auto t2 = ss0->add_table({ "t2", {} });

    auto r = analyze_relation_info_name(
            context(),
            id("s0", "t1"),
            {});
    invalid(r, sql_analyzer_code::table_not_found);
}

TEST_F(analyze_name_qualified_test, relation_info_in_schema_decl_not_in_schema) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto t0 = ss0->add_table({ "t0", {} });
    auto t1 = ss0->add_table({ "t1", {} });
    auto t2 = ss0->add_table({ "t2", {} });

    auto s0 = schemas_->add({ "s0", {}, });

    query_scope scope;
    scope.add({ *t0 });
    scope.add({ *t1 });
    scope.add({ *t2 });

    auto r = analyze_relation_info_name(
            context(),
            id("s0", "t1"),
            scope);
    invalid(r, sql_analyzer_code::table_not_found);
}

TEST_F(analyze_name_qualified_test, relation_decl_in_schema_decl) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });

    auto r = analyze_relation_name(
            context(),
            id("s0", "T0"));
    validate(r, t0);
}

TEST_F(analyze_name_qualified_test, relation_decl_in_schema_decl_multiple) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });
    auto t1 = ss0->add_table({ "t1", {} });
    auto t2 = ss0->add_table({ "t2", {} });

    auto r = analyze_relation_name(
            context(),
            id("s0", "t1"));
    validate(r, t1);
}

TEST_F(analyze_name_qualified_test, relation_decl_in_schema_decl_missing) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });
    auto t1 = ss0->add_table({ "t1", {} });
    auto t2 = ss0->add_table({ "t2", {} });

    auto r = analyze_relation_name(
            context(),
            id("s0", "MISSING"));
    invalid(r, sql_analyzer_code::table_not_found);
}

TEST_F(analyze_name_qualified_test, table_decl_in_schema_decl) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });

    auto r = analyze_table_name(
            context(),
            id("s0", "T0"));
    validate(r, *s0, t0);
}

TEST_F(analyze_name_qualified_test, table_decl_in_schema_decl_multiple) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });
    auto t1 = ss0->add_table({ "t1", {} });
    auto t2 = ss0->add_table({ "t2", {} });

    auto r = analyze_table_name(
            context(),
            id("s0", "t1"));
    validate(r, *s0, t1);
}

TEST_F(analyze_name_qualified_test, table_decl_in_schema_decl_missing) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });
    auto t1 = ss0->add_table({ "t1", {} });
    auto t2 = ss0->add_table({ "t2", {} });

    auto r = analyze_table_name(
            context(),
            id("s0", "MISSING"));
    invalid(r, sql_analyzer_code::table_not_found);
}

TEST_F(analyze_name_qualified_test, index_decl_in_schema_decl) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });
    auto i0 = ss0->add_index({ t0, "i0" });

    auto r = analyze_index_name(
            context(),
            id("s0", "I0"));
    validate(r, *s0, i0);
}

TEST_F(analyze_name_qualified_test, index_decl_in_schema_decl_multiple) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });
    auto i0 = ss0->add_index({ t0, "i0" });
    auto i1 = ss0->add_index({ t0, "i1" });
    auto i2 = ss0->add_index({ t0, "i2" });

    auto r = analyze_index_name(
            context(),
            id("s0", "i1"));
    validate(r, *s0, i1);
}

TEST_F(analyze_name_qualified_test, index_decl_in_schema_decl_missing) {
    auto ss0 = std::make_shared<::yugawara::storage::configurable_provider>();
    auto s0 = schemas_->add({ "s0", {}, ss0, });
    auto t0 = ss0->add_table({ "t0", {} });
    auto i0 = ss0->add_index({ t0, "i0" });
    auto i1 = ss0->add_index({ t0, "i1" });
    auto i2 = ss0->add_index({ t0, "i2" });

    auto r = analyze_index_name(
            context(),
            id("s0", "MISSING"));
    invalid(r, sql_analyzer_code::index_not_found);
}

TEST_F(analyze_name_qualified_test, schema_in_catalog) {
    auto s0 = schemas_->add({ "s0" });

    auto r = analyze_schema_name(
            context(),
            id(catalog_->name(), "s0"));
    validate(r, s0);
}

TEST_F(analyze_name_qualified_test, schema_in_catalog_multiple) {
    auto s0 = schemas_->add({ "s0" });
    auto s1 = schemas_->add({ "s1" });
    auto s2 = schemas_->add({ "s2" });

    auto r = analyze_schema_name(
            context(),
            id(catalog_->name(), "s1"));
    validate(r, s1);
}

TEST_F(analyze_name_qualified_test, schema_in_catalog_missing) {
    auto s0 = schemas_->add({ "s0" });
    auto s1 = schemas_->add({ "s1" });
    auto s2 = schemas_->add({ "s2" });

    auto r = analyze_schema_name(
            context(),
            id(catalog_->name(), "MISSING"));
    invalid(r, sql_analyzer_code::schema_not_found);
}

TEST_F(analyze_name_qualified_test, schema_in_catalog_missing_catalog) {
    auto s0 = schemas_->add({ "s0" });
    auto s1 = schemas_->add({ "s1" });
    auto s2 = schemas_->add({ "s2" });

    auto r = analyze_schema_name(
            context(),
            id("MISSING", "s0"));
    invalid(r, sql_analyzer_code::catalog_not_found);
}

} // namespace mizugaki::analyzer::details
