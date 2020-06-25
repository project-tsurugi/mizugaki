#include <mizugaki/analyzer/details/analyze_name.h>

#include <gtest/gtest.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_name_test : public test_parent {};

TEST_F(analyze_name_test, variable_simple) {
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("c0"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v));
}

TEST_F(analyze_name_test, variable_simple_missing_variable) {
    query_scope scope;
    auto r = analyze_variable_name(
            context(),
            id("c0"),
            scope);
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::symbol_not_found));
}

TEST_F(analyze_name_test, variable_simple_multiple_variables) {
    auto v0 = vdesc();
    auto v1 = vdesc();
    auto v2 = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v0, "c0", });
    relation.add({ {}, v1, "c1", });
    relation.add({ {}, v2, "c2", });

    auto r = analyze_variable_name(
            context(),
            id("c1"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v1));
}

TEST_F(analyze_name_test, variable_simple_multiple_relations) {
    auto v0 = vdesc();
    auto v1 = vdesc();
    auto v2 = vdesc();

    query_scope scope;
    scope.add({ {}, "r0" });
    scope.add({ {}, "r1" });
    scope.add({ {}, "r2" });
    auto&& r0 = *scope.find("r0");
    auto&& r1 = *scope.find("r1");
    auto&& r2 = *scope.find("r2");

    r0.add({ {}, v0, "c0", });
    r1.add({ {}, v1, "c1", });
    r2.add({ {}, v2, "c2", });

    auto r = analyze_variable_name(
            context(),
            id("c1"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v1));
}

TEST_F(analyze_name_test, variable_simple_ambiguous_in_relation) {
    auto v0 = vdesc();
    auto v1 = vdesc();
    auto v2 = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v0, "c0", });
    relation.add({ {}, v1, "c0", });
    relation.add({ {}, v2, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("c0"),
            scope);
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::variable_ambiguous));
}

TEST_F(analyze_name_test, variable_simple_ambiguous_over_relation) {
    auto v0 = vdesc();
    auto v1 = vdesc();
    auto v2 = vdesc();

    query_scope scope;
    scope.add({ {}, "r0" });
    scope.add({ {}, "r1" });
    scope.add({ {}, "r2" });
    auto&& r0 = *scope.find("r0");
    auto&& r1 = *scope.find("r1");
    auto&& r2 = *scope.find("r2");
    r0.add({ {}, v0, "c0", });
    r1.add({ {}, v1, "c0", });
    r2.add({ {}, v2, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("c0"),
            scope);
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::variable_ambiguous));
}

TEST_F(analyze_name_test, variable_simple_ambiguous_but_unique_exported) {
    auto v0 = vdesc();
    auto v1 = vdesc();
    auto v2 = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v0, "c0", false, });
    relation.add({ {}, v1, "c0", true, });
    relation.add({ {}, v2, "c0", false, });

    auto r = analyze_variable_name(
            context(),
            id("c0"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v1));
}

TEST_F(analyze_name_test, variable_simple_not_exported) {
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v, "c0", false, });

    auto r = analyze_variable_name(
            context(),
            id("c0"),
            scope);
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::symbol_not_found));
}

TEST_F(analyze_name_test, variable_relation) {
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({ {}, "t0", });
    relation.add(column_info { {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("t0", "c0"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v));
}

TEST_F(analyze_name_test, variable_relation_multiple_relation) {
    auto v0 = vdesc();
    auto v1 = vdesc();
    auto v2 = vdesc();

    query_scope scope;
    scope.add({ {}, "r0" });
    scope.add({ {}, "r1" });
    scope.add({ {}, "r2" });
    auto&& r0 = *scope.find("r0");
    auto&& r1 = *scope.find("r1");
    auto&& r2 = *scope.find("r2");
    r0.add({ {}, v0, "c0", });
    r1.add({ {}, v1, "c0", });
    r2.add({ {}, v2, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("r1", "c0"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v1));
}

TEST_F(analyze_name_test, variable_relation_multiple_variable) {
    auto v0 = vdesc();
    auto v1 = vdesc();
    auto v2 = vdesc();

    query_scope scope;
    auto&& relation = scope.add({ {}, "r0", });
    relation.add({ {}, v0, "c0", });
    relation.add({ {}, v1, "c1", });
    relation.add({ {}, v2, "c2", });

    auto r = analyze_variable_name(
            context(),
            id("r0", "c1"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v1));
}

TEST_F(analyze_name_test, variable_relation_not_exported) {
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({ {}, "t0", });
    relation.add(column_info { {}, v, "c0", false, });

    auto r = analyze_variable_name(
            context(),
            id("t0", "c0"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v));
}

TEST_F(analyze_name_test, variable_relation_missing_relation) {
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({ {}, "-", });
    relation.add(column_info { {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("t0", "c0"),
            scope);
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::symbol_not_found));
}

TEST_F(analyze_name_test, variable_relation_missing_variable) {
    query_scope scope;
    scope.add({ {}, "t0", });

    auto r = analyze_variable_name(
            context(),
            id("t0", "c0"),
            scope);
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::column_not_found));
}

TEST_F(analyze_name_test, variable_relation_ambiguous_not_exported) {
    auto v0 = vdesc();
    auto v1 = vdesc();

    query_scope scope;
    auto&& relation = scope.add({ {}, "r0", });
    relation.add({ {}, v0, "c0", true, });
    relation.add({ {}, v1, "c0", false, });

    auto r = analyze_variable_name(
            context(),
            id("r0", "c0"),
            scope);
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::variable_ambiguous));
}

TEST_F(analyze_name_test, variable_schemapath_relation) {
    auto t = storages_->add_table({ "t", {} });

    ::yugawara::schema::declaration schema {
            "s",
            {},
            storages_,
    };
    ::yugawara::schema::search_path path { share(schema) };

    options_ = { {}, share(path), {} };

    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({ *t, "r", });
    relation.add(column_info { {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("t", "c0"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v));
}

TEST_F(analyze_name_test, variable_schema_relation) {
    auto t = storages_->add_table({ "t", {} });
    auto schema = schemas_->add(::yugawara::schema::declaration {
            "s",
            {},
            storages_,
    });
    auto catalog = share(::yugawara::schema::catalog {
            "c",
            {},
            schemas_,
    });

    options_ = { catalog, {}, {} };

    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({ *t, "r", });
    relation.add(column_info { {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("s", "t", "c0"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v));
}

TEST_F(analyze_name_test, variable_schema_relation_not_referring) {
    auto t0 = storages_->add_table({ "t0", {} });
    auto t1 = storages_->add_table({ "t1", {} });
    auto schema = schemas_->add(::yugawara::schema::declaration {
            "s",
            {},
            storages_,
    });
    auto catalog = share(::yugawara::schema::catalog {
            "c",
            {},
            schemas_,
    });

    options_ = { catalog, {}, {} };

    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({ *t0, "r", });
    relation.add(column_info { {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("s", "t1", "c0"),
            scope);
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::variable_not_found));
}

TEST_F(analyze_name_test, variable_catalog_schema_relation) {
    auto t = storages_->add_table({ "t", {} });
    auto schema = schemas_->add(::yugawara::schema::declaration {
            "s",
            {},
            storages_,
    });
    auto catalog = share(::yugawara::schema::catalog {
            "c",
            {},
            schemas_,
    });

    options_ = { catalog, {}, {} };

    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({ *t, "r", });
    relation.add(column_info { {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("c", "s", "t", "c0"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v));
}

TEST_F(analyze_name_test, variable_catalog_schema_missing) {
    auto catalog = share(::yugawara::schema::catalog {
            "c",
            {},
            schemas_,
    });
    options_ = { catalog, {}, {} };
    auto r = analyze_variable_name(
            context(),
            id("c", "s", "t", "c0"),
            {});
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::schema_not_found));
}

TEST_F(analyze_name_test, relation_primary_schema_path) {
    auto r0 = storages_->add_table({ "r0", {} });
    auto schema = schemas_->add(::yugawara::schema::declaration {
            "s",
            {},
            storages_,
    });
    ::yugawara::schema::search_path path { schema };
    options_ = { {}, share(path), {} };

    auto r = analyze_relation_name(
            context(),
            id("r0"));
    ASSERT_TRUE(r);
    EXPECT_EQ(r.get(), r0.get());
}

TEST_F(analyze_name_test, relation_primary_missing) {
    options_ = { {}, {}, {} };

    auto r = analyze_relation_name(
            context(),
            id("r0"));
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::symbol_not_found));
}

TEST_F(analyze_name_test, relation_schema) {
    auto r0 = storages_->add_table({ "r0", {} });
    auto schema = schemas_->add(::yugawara::schema::declaration {
            "s",
            {},
            storages_,
    });
    ::yugawara::schema::catalog catalog { "c", {}, schemas_, };
    options_ = { share(catalog), {}, {} };

    auto r = analyze_relation_name(
            context(),
            id("s", "r0"));
    ASSERT_TRUE(r);
    EXPECT_EQ(r.get(), r0.get());
}

TEST_F(analyze_name_test, relation_schema_missing_relation) {
    auto schema = schemas_->add(::yugawara::schema::declaration {
            "s",
            {},
            storages_,
    });
    ::yugawara::schema::catalog catalog { "c", {}, schemas_, };
    options_ = { share(catalog), {}, {} };

    auto r = analyze_relation_name(
            context(),
            id("s", "r0"));
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::table_not_found));
}

TEST_F(analyze_name_test, relation_catalog_schema) {
    auto r0 = storages_->add_table({ "r0", {} });
    auto schema = schemas_->add(::yugawara::schema::declaration {
            "s",
            {},
            storages_,
    });
    ::yugawara::schema::catalog catalog { "c", {}, schemas_, };
    options_ = { share(catalog), {}, {} };

    auto r = analyze_relation_name(
            context(),
            id("c", "s", "r0"));
    ASSERT_TRUE(r);
    EXPECT_EQ(r.get(), r0.get());
}

TEST_F(analyze_name_test, relation_catalog_schema_missing_schema) {
    ::yugawara::schema::catalog catalog { "c", {}, schemas_, };
    options_ = { share(catalog), {}, {} };

    auto r = analyze_relation_name(
            context(),
            id("c", "s", "r0"));
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::schema_not_found));
}

TEST_F(analyze_name_test, regular_to_lowercase_enabled) {
    options_.lowercase_regular_identifiers() = true;
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("C0"),
            scope);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(v));
}

TEST_F(analyze_name_test, regular_to_lowercase_disabled) {
    options_.lowercase_regular_identifiers() = false;
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("C0"),
            scope);
    ASSERT_FALSE(r);
}

TEST_F(analyze_name_test, regular_to_lowercase_delimited) {
    options_.lowercase_regular_identifiers() = true;
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("C0", false),
            scope);
    ASSERT_FALSE(r);
}

} // namespace mizugaki::analyzer::details
