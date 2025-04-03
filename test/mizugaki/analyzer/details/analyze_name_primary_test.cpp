#include <mizugaki/analyzer/details/analyze_name.h>

#include <gtest/gtest.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_name_primary_test : public test_parent {
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

    template<class T1, class T2>
    void validate(std::vector<std::shared_ptr<T1>> const& result, std::shared_ptr<T2> const& expect) {
        auto iter = std::find(result.begin(), result.end(), expect);
        if (iter == result.end()) {
            ADD_FAILURE() << "expected: " << *expect << "\n" << diagnostics();
            return;
        }
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

TEST_F(analyze_name_primary_test, column_variable) {
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("c0"),
            scope);
    validate(r, v);
}

TEST_F(analyze_name_primary_test, column_variable_missing_variable) {
    query_scope scope;
    auto r = analyze_variable_name(
            context(),
            id("c0"),
            scope);
    invalid(r, diagnostic_code::variable_not_found);
}

TEST_F(analyze_name_primary_test, column_variable_multiple_variables) {
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
    validate(r, v1);
}

TEST_F(analyze_name_primary_test, column_variable_primary_relations) {
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
    validate(r, v1);
}

TEST_F(analyze_name_primary_test, column_variable_ambiguous_in_relation) {
    auto v0 = vdesc();
    auto v1 = vdesc();
    auto v2 = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v0, "c0", });
    relation.add({ {}, v1, "c1", });
    relation.add({ {}, v2, "c0", });

    auto r0 = analyze_variable_name(
            context(),
            id("c0"),
            scope);
    invalid(r0, diagnostic_code::column_ambiguous);

    auto r1 = analyze_variable_name(
            context(),
            id("c1"),
            scope);
    validate(r1, v1);
}

TEST_F(analyze_name_primary_test, column_variable_ambiguous_over_relation) {
    auto v0 = vdesc();
    auto v1 = vdesc();
    auto v2 = vdesc();

    query_scope scope;
    scope.reserve(3);
    auto&& q0 = scope.add({ {}, "r0" });
    auto&& q1 = scope.add({ {}, "r1" });
    auto&& q2 = scope.add({ {}, "r2" });
    q0.add({ {}, v0, "c0", });
    q1.add({ {}, v1, "c1", });
    q2.add({ {}, v2, "c0", });

    auto r0 = analyze_variable_name(
            context(),
            id("c0"),
            scope);
    invalid(r0, diagnostic_code::column_ambiguous);

    auto r1 = analyze_variable_name(
            context(),
            id("c1"),
            scope);
    validate(r1, v1);
}

TEST_F(analyze_name_primary_test, column_variable_ambiguous_but_unique_exported) {
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
    validate(r, v1);
}

TEST_F(analyze_name_primary_test, column_variable_not_exported) {
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v, "c0", false, });

    auto r = analyze_variable_name(
            context(),
            id("c0"),
            scope);
    invalid(r, diagnostic_code::variable_not_found);
}

TEST_F(analyze_name_primary_test, schema_variable) {
    auto sv0 = std::make_shared<::yugawara::variable::configurable_provider>();
    ::yugawara::schema::declaration s0 { "s0", {}, {}, sv0, };
    search_path_->elements().emplace_back(&s0);

    auto x0 = sv0->add({ "x0", ttype::int8 {} });

    auto r = analyze_variable_name(
            context(),
            id("x0"),
            {});
    validate(r, x0);
}

TEST_F(analyze_name_primary_test, schema_variable_multiple_variables) {
    auto sv0 = std::make_shared<::yugawara::variable::configurable_provider>();
    ::yugawara::schema::declaration s0 { "s0", {}, {}, sv0, };
    search_path_->elements().emplace_back(&s0);

    auto x0 = sv0->add({ "x0", ttype::int8 {} });
    auto x1 = sv0->add({ "x1", ttype::int8 {} });
    auto x2 = sv0->add({ "x2", ttype::int8 {} });

    auto r = analyze_variable_name(
            context(),
            id("x1"),
            {});
    validate(r, x1);
}

TEST_F(analyze_name_primary_test, schema_variable_multiple_schemas) {
    auto sv0 = std::make_shared<::yugawara::variable::configurable_provider>();
    auto sv1 = std::make_shared<::yugawara::variable::configurable_provider>();
    auto sv2 = std::make_shared<::yugawara::variable::configurable_provider>();

    ::yugawara::schema::declaration s0 { "s0", {}, {}, sv0, };
    ::yugawara::schema::declaration s1 { "s1", {}, {}, sv1, };
    ::yugawara::schema::declaration s2 { "s2", {}, {}, sv2, };

    search_path_->elements().emplace_back(&s0);
    search_path_->elements().emplace_back(&s1);
    search_path_->elements().emplace_back(&s2);

    auto x0 = sv0->add({ "x0", ttype::int8 {} });
    auto x1 = sv1->add({ "x1", ttype::int8 {} });
    auto x2 = sv2->add({ "x2", ttype::int8 {} });

    auto r = analyze_variable_name(
            context(),
            id("x1"),
            {});
    validate(r, x1);
}

TEST_F(analyze_name_primary_test, schema_variable_multiple_schemas_override) {
    auto sv0 = std::make_shared<::yugawara::variable::configurable_provider>();
    auto sv1 = std::make_shared<::yugawara::variable::configurable_provider>();
    auto sv2 = std::make_shared<::yugawara::variable::configurable_provider>();

    ::yugawara::schema::declaration s0 { "s0", {}, {}, sv0, };
    ::yugawara::schema::declaration s1 { "s1", {}, {}, sv1, };
    ::yugawara::schema::declaration s2 { "s2", {}, {}, sv2, };

    search_path_->elements().emplace_back(&s0);
    search_path_->elements().emplace_back(&s1);
    search_path_->elements().emplace_back(&s2);

    auto x0 = sv0->add({ "x0", ttype::int8 {} });
    auto x1 = sv1->add({ "x1", ttype::int8 {} });
    auto x2 = sv2->add({ "x1", ttype::int8 {} });

    auto r = analyze_variable_name(
            context(),
            id("x1"),
            {});
    validate(r, x1);
}

TEST_F(analyze_name_primary_test, relation_info_relation) {
    auto t = storages_->add_table({ "t", {} });

    query_scope scope;
    scope.add({ *t });

    auto r = analyze_relation_info_name(
            context(),
            id("t"),
            scope);
    validate(r, scope.find("t"));
}

TEST_F(analyze_name_primary_test, relation_info_correlation) {
    auto v = vdesc();

    query_scope scope;
    scope.add({ {}, "r" });

    auto r = analyze_relation_info_name(
            context(),
            id("r"),
            scope);
    validate(r, scope.find("r"));
}

TEST_F(analyze_name_primary_test, relation_info_multiple) {
    auto v = vdesc();

    query_scope scope;
    scope.reserve(3);
    scope.add({ {}, "r0" });
    scope.add({ {}, "r1" });
    scope.add({ {}, "r2" });

    auto r = analyze_relation_info_name(
            context(),
            id("r1"),
            scope);
    validate(r, scope.find("r1"));
}

TEST_F(analyze_name_primary_test, relation_info_missing) {
    auto v = vdesc();

    query_scope scope;
    scope.add({ {}, "r0" });
    scope.add({ {}, "r1" });
    scope.add({ {}, "r2" });

    auto r = analyze_relation_info_name(
            context(),
            id("MISSING"),
            scope);
    invalid(r, diagnostic_code::table_not_found);
}

TEST_F(analyze_name_primary_test, relation_info_ambiguous) {
    auto v = vdesc();

    query_scope scope;
    scope.add({ {}, "r" });
    scope.add({ {}, "r" });

    auto r = analyze_relation_info_name(
            context(),
            id("r"),
            scope);
    invalid(r, diagnostic_code::relation_ambiguous);
}

TEST_F(analyze_name_primary_test, relation_decl) {
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
    validate(r, r0);
}

TEST_F(analyze_name_primary_test, relation_decl_missing) {
    options_ = { {}, {}, {} };

    auto r = analyze_relation_name(
            context(),
            id("r0"));
    invalid(r, diagnostic_code::table_not_found);
}

TEST_F(analyze_name_primary_test, function_decl) {
    auto functions = std::make_shared<::yugawara::function::configurable_provider>();
    ::yugawara::schema::declaration s {
            "s",
            {},
            {},
            {},
            functions,
    };
    auto f0 = functions->add({
            ::yugawara::function::declaration::minimum_user_function_id + 1,
            "f",
            ttype::int8 {},
            {},
    });
    search_path_->elements().emplace_back(&s);

    auto r = analyze_function_name(
            context(),
            id("f"),
            0);
    EXPECT_EQ(r.size(), 1);
    validate(r, f0);

    auto as = analyze_aggregation_name(
            context(),
            id("f"),
            0);
    EXPECT_EQ(as.size(), 0);
}

TEST_F(analyze_name_primary_test, function_decl_multiple) {
    auto functions = std::make_shared<::yugawara::function::configurable_provider>();
    ::yugawara::schema::declaration s {
        "s",
        {},
        {},
        {},
        functions,
    };
    auto f0 = functions->add({
            ::yugawara::function::declaration::minimum_user_function_id + 1,
            "f",
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
    });
    auto f1 = functions->add({
            ::yugawara::function::declaration::minimum_user_function_id + 2,
            "f",
            ttype::int8 {},
            {
                    ttype::character { ttype::varying, {} },
            },
    });
    auto f2 = functions->add({
            ::yugawara::function::declaration::minimum_user_function_id + 3,
            "f",
            ttype::int8 {},
            {
                    ttype::octet { ttype::varying, {} },
            },
    });
    search_path_->elements().emplace_back(&s);

    auto r = analyze_function_name(
            context(),
            id("f"),
            1);
    EXPECT_EQ(r.size(), 3);
    validate(r, f0);
    validate(r, f1);
    validate(r, f2);
}

TEST_F(analyze_name_primary_test, function_decl_mismatch_name) {
    auto functions = std::make_shared<::yugawara::function::configurable_provider>();
    ::yugawara::schema::declaration s {
            "s",
            {},
            {},
            {},
            functions,
    };
    auto f0 = functions->add({
            ::yugawara::function::declaration::minimum_user_function_id + 1,
            "f",
            ttype::int8 {},
            {},
    });
    search_path_->elements().emplace_back(&s);

    auto r = analyze_function_name(
            context(),
            id("g"),
            0);
    EXPECT_EQ(r.size(), 0);
}

TEST_F(analyze_name_primary_test, function_decl_mismatch_argument_count) {
    auto functions = std::make_shared<::yugawara::function::configurable_provider>();
    ::yugawara::schema::declaration s {
            "s",
            {},
            {},
            {},
            functions,
    };
    auto f0 = functions->add({
            ::yugawara::function::declaration::minimum_user_function_id + 1,
            "f",
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
    });
    search_path_->elements().emplace_back(&s);

    auto r = analyze_function_name(
            context(),
            id("f"),
            0);
    EXPECT_EQ(r.size(), 0);
}

TEST_F(analyze_name_primary_test, aggregation_decl) {
    auto functions = std::make_shared<::yugawara::aggregate::configurable_provider>();
    ::yugawara::schema::declaration s {
            "s",
            {},
            {},
            {},
            {},
            functions,
    };
    auto f0 = functions->add({
            ::yugawara::aggregate::declaration::minimum_user_function_id + 1,
            "f",
            ttype::int8 {},
            {},
    });
    search_path_->elements().emplace_back(&s);

    auto r = analyze_aggregation_name(
            context(),
            id("f"),
            0);
    EXPECT_EQ(r.size(), 1);
    validate(r, f0);

    auto fs = analyze_function_name(
            context(),
            id("f"),
            0);
    EXPECT_EQ(fs.size(), 0);
}

TEST_F(analyze_name_primary_test, aggregation_decl_multiple) {
    auto functions = std::make_shared<::yugawara::aggregate::configurable_provider>();
    ::yugawara::schema::declaration s {
            "s",
            {},
            {},
            {},
            {},
            functions,
    };
    auto f0 = functions->add({
            ::yugawara::aggregate::declaration::minimum_user_function_id + 1,
            "f",
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
    });
    auto f1 = functions->add({
            ::yugawara::aggregate::declaration::minimum_user_function_id + 2,
            "f",
            ttype::int8 {},
            {
                    ttype::character { ttype::varying, {} },
            },
    });
    auto f2 = functions->add({
            ::yugawara::aggregate::declaration::minimum_user_function_id + 3,
            "f",
            ttype::int8 {},
            {
                    ttype::octet { ttype::varying, {} },
            },
    });
    search_path_->elements().emplace_back(&s);

    auto r = analyze_aggregation_name(
            context(),
            id("f"),
            1);
    EXPECT_EQ(r.size(), 3);
    validate(r, f0);
    validate(r, f1);
    validate(r, f2);
}

TEST_F(analyze_name_primary_test, aggregation_decl_mismatch_name) {
    auto functions = std::make_shared<::yugawara::aggregate::configurable_provider>();
    ::yugawara::schema::declaration s {
            "s",
            {},
            {},
            {},
            {},
            functions,
    };
    auto f0 = functions->add({
            ::yugawara::aggregate::declaration::minimum_user_function_id + 1,
            "f",
            ttype::int8 {},
            {},
    });
    search_path_->elements().emplace_back(&s);

    auto r = analyze_aggregation_name(
            context(),
            id("g"),
            0);
    EXPECT_EQ(r.size(), 0);
}

TEST_F(analyze_name_primary_test, aggregation_decl_mismatch_argument_count) {
    auto functions = std::make_shared<::yugawara::aggregate::configurable_provider>();
    ::yugawara::schema::declaration s {
            "s",
            {},
            {},
            {},
            {},
            functions,
    };
    auto f0 = functions->add({
            ::yugawara::aggregate::declaration::minimum_user_function_id + 1,
            "f",
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
    });
    search_path_->elements().emplace_back(&s);

    auto r = analyze_aggregation_name(
            context(),
            id("f"),
            0);
    EXPECT_EQ(r.size(), 0);
}

TEST_F(analyze_name_primary_test, table_decl) {
    auto storage = std::make_shared<::yugawara::storage::configurable_provider>();
    ::yugawara::schema::declaration s {
            "s",
            {},
            storage,
    };
    auto t0 = storage->add_table({ "t0", {} });
    search_path_->elements().emplace_back(&s);

    auto r = analyze_table_name(
            context(),
            id("T0"));
    validate(r, s, t0);
}

TEST_F(analyze_name_primary_test, table_decl_missing_mandatory) {
    auto r = analyze_table_name(
            context(),
            id("t0"),
            true);
    invalid(r, sql_analyzer_code::table_not_found);
}

TEST_F(analyze_name_primary_test, table_decl_missing_optional) {
    auto r = analyze_table_name(
            context(),
            id("t0"),
            false);
    EXPECT_FALSE(r);
    expect_no_error();
}

TEST_F(analyze_name_primary_test, index_decl) {
    auto storage = std::make_shared<::yugawara::storage::configurable_provider>();
    ::yugawara::schema::declaration s {
            "s",
            {},
            storage,
    };
    search_path_->elements().emplace_back(&s);

    auto t0 = storage->add_table({ "t0", {} });
    auto i0 = storage->add_index({ t0, "i0" });

    auto r = analyze_index_name(
            context(),
            id("I0"));
    validate(r, s, i0);
}

TEST_F(analyze_name_primary_test, index_decl_missing_mandatory) {
    auto r = analyze_index_name(
            context(),
            id("i0"),
            true);
    invalid(r, sql_analyzer_code::index_not_found);
}

TEST_F(analyze_name_primary_test, index_decl_missing_optional) {
    auto r = analyze_index_name(
            context(),
            id("i0"),
            false);
    EXPECT_FALSE(r);
    expect_no_error();
}

TEST_F(analyze_name_primary_test, schema_decl) {
    auto s0 = schemas_->add({ "s0" });
    auto r = analyze_schema_name(
            context(),
            id("S0"));
    validate(r, s0);
}

TEST_F(analyze_name_primary_test, schema_decl_missing_mandatory) {
    auto r = analyze_schema_name(
            context(),
            id("s0"),
            true);
    invalid(r, sql_analyzer_code::schema_not_found);
}

TEST_F(analyze_name_primary_test, schema_decl_missing_optional) {
    auto r = analyze_schema_name(
            context(),
            id("s0"),
            false);
    EXPECT_FALSE(r);
    expect_no_error();
}

TEST_F(analyze_name_primary_test, regular_to_lowercase_enabled) {
    options_.lowercase_regular_identifiers() = true;
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("C0"),
            scope);
    validate(r, v);
}

TEST_F(analyze_name_primary_test, regular_to_lowercase_disabled) {
    options_.lowercase_regular_identifiers() = false;
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            id("C0"),
            scope);
    invalid(r, diagnostic_code::variable_not_found);
}

TEST_F(analyze_name_primary_test, delimited_to_lowercase) {
    options_.lowercase_regular_identifiers() = true;
    auto v = vdesc();

    query_scope scope;
    auto&& relation = scope.add({});
    relation.add({ {}, v, "c0", });

    auto r = analyze_variable_name(
            context(),
            ast::name::simple {
                    "C0",
                    ast::name::identifier_kind::delimited,
            },
            scope);
    invalid(r, diagnostic_code::variable_not_found);
}

} // namespace mizugaki::analyzer::details
