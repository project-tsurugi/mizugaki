#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>

#include <takatori/scalar/immediate.h>

#include <yugawara/binding/factory.h>

#include <yugawara/variable/configurable_provider.h>

#include <mizugaki/ast/literal/numeric.h>

#include <mizugaki/ast/scalar/host_parameter_reference.h>

#include "../utils.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_scalar_expression_test : public ::testing::Test {
public:
    analyzer_context& context() {
        if (!context_finalizer_) {
            context_finalizer_ = context_.initialize(options_, {}, {}, placeholders_, host_variables_);
        }
        return context_;
    }

    ::takatori::descriptor::variable vd(std::string_view name = "id") {
        return ::yugawara::binding::factory {}.stream_variable(name);
    }

protected:
    sql_analyzer_options options_ { {}, {}, {} }; // NOLINT
    placeholder_map placeholders_ {};
    ::yugawara::variable::configurable_provider host_variables_ {};

private:
    analyzer_context context_;
    std::optional<::takatori::util::finalizer> context_finalizer_;
};

TEST_F(analyze_scalar_expression_test, literal_expression) {
    auto r = analyze_scalar_expression(
            context(),
            literal(number("1")),
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, immediate(1));
}

TEST_F(analyze_scalar_expression_test, variable_reference) {
    auto c0 = vdesc();
    query_scope scope {};
    auto&& rinfo = scope.add({});
    rinfo.add({ {}, c0, "x" });

    auto r = analyze_scalar_expression(
            context(),
            vref(id("x")),
            scope,
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(c0));
}

TEST_F(analyze_scalar_expression_test, host_parameter_reference_value) {
    auto c0 = vdesc();
    placeholders_.add(":x", { ttype::int4 {}, tvalue::int4 { 1 } });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::host_parameter_reference { id(":x") },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, immediate(1));
}

TEST_F(analyze_scalar_expression_test, host_parameter_reference_variable) {
    auto decl = host_variables_.add({ ":x", ttype::int4 {} });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::host_parameter_reference { id(":x") },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(::yugawara::binding::factory {}(decl)));
}

} // namespace mizugaki::analyzer::details
