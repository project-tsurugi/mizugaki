#include <mizugaki/translator/details/scalar_expression_translator.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/type/decimal.h>
#include <takatori/type/character.h>

#include <takatori/value/primitive.h>
#include <takatori/value/decimal.h>
#include <takatori/value/character.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/variable_reference.h>
#include <takatori/scalar/unary.h>
#include <takatori/scalar/cast.h>
#include <takatori/scalar/binary.h>
#include <takatori/scalar/compare.h>
#include <takatori/scalar/match.h>
#include <takatori/scalar/function_call.h>

#include <yugawara/binding/factory.h>
#include <yugawara/function/configurable_provider.h>
#include <yugawara/aggregate/configurable_provider.h>
#include <yugawara/extension/scalar/aggregate_function_call.h>

#include <shakujo/common/core/type/Int.h>
#include <shakujo/common/core/value/Int.h>
#include <shakujo/model/IRFactory.h>

#include <mizugaki/placeholder_map.h>
#include <mizugaki/translator/testing/utils.h>

namespace mizugaki::translator::details {

using namespace ::mizugaki::translator::testing;
using code = shakujo_translator_diagnostic_code;

namespace type = ::takatori::type;
namespace value = ::takatori::value;
namespace scalar = ::takatori::scalar;

namespace tinfo = ::shakujo::common::core::type;
namespace vinfo = ::shakujo::common::core::value;

namespace extension = ::yugawara::extension;

using quantifier = ::yugawara::aggregate::set_quantifier;

class scalar_expression_translator_test : public ::testing::Test {
public:
    std::shared_ptr<::yugawara::function::configurable_provider> functions = std::make_shared<::yugawara::function::configurable_provider>();
    std::shared_ptr<::yugawara::aggregate::configurable_provider> aggregates = std::make_shared<::yugawara::aggregate::configurable_provider>();

    shakujo_translator::impl entry { new_translator_impl() };
    shakujo_translator_options options { new_options({}, {}, functions, aggregates) };
    placeholder_map placeholders;
    scalar_expression_translator engine { entry.initialize(options, {}, placeholders) };

    ::shakujo::model::IRFactory f;
    ::yugawara::binding::factory bindings { options.get_object_creator() };
};

TEST_F(scalar_expression_translator_test, literal) {
    auto s = f.Literal(tinfo::Int(32), vinfo::Int(100));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::immediate(value::int4(100), type::int4()));
}

TEST_F(scalar_expression_translator_test, variable_reference) {
    relation_info relation;
    auto v = bindings.stream_variable("x");
    relation.add_column(v, "x");

    auto s = f.VariableReference(f.Name("x"));
    auto r = engine.process(*s, { options, relation });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::variable_reference(v));
}

TEST_F(scalar_expression_translator_test, variable_reference_missing) {
    auto s = f.VariableReference(f.Name("x"));
    auto r = engine.process(*s, { options, {} });
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::variable_not_found, entry.diagnostics()));
}

TEST_F(scalar_expression_translator_test, unary_operator) {
    using Op = ::shakujo::model::expression::UnaryOperator::Kind;
    auto s = f.UnaryOperator(Op::IS_NULL, f.Literal(tinfo::Bool(), true));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::unary(
            scalar::unary_operator::is_null,
            scalar::immediate(value::boolean(true), type::boolean())));
}

TEST_F(scalar_expression_translator_test, unary_operator_complement) {
    using Op = ::shakujo::model::expression::UnaryOperator::Kind;
    auto s = f.UnaryOperator(Op::IS_NOT_NULL, f.Literal(tinfo::Bool(), true));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::unary(
            scalar::unary_operator::conditional_not,
            scalar::unary(
                    scalar::unary_operator::is_null,
                    scalar::immediate(value::boolean(true), type::boolean()))));
}

TEST_F(scalar_expression_translator_test, unary_operator_unsupported) {
    using Op = ::shakujo::model::expression::UnaryOperator::Kind;
    auto s = f.UnaryOperator(Op::CHECK_NULL, f.Literal(tinfo::Bool(), true));
    auto r = engine.process(*s, { options, {} });
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::unsupported_scalar_expression, entry.diagnostics()));
}

TEST_F(scalar_expression_translator_test, binary_operator) {
    using Op = ::shakujo::model::expression::BinaryOperator::Kind;
    auto s = f.BinaryOperator(
            Op::ADD,
            f.Literal(tinfo::Int(32), vinfo::Int(1)),
            f.Literal(tinfo::Int(32), vinfo::Int(2)));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::binary(
            scalar::binary_operator::add,
            scalar::immediate(value::int4(1), type::int4()),
            scalar::immediate(value::int4(2), type::int4())));
}

TEST_F(scalar_expression_translator_test, binary_operator_compare) {
    using Op = ::shakujo::model::expression::BinaryOperator::Kind;
    auto s = f.BinaryOperator(
            Op::EQUAL,
            f.Literal(tinfo::Int(32), vinfo::Int(1)),
            f.Literal(tinfo::Int(32), vinfo::Int(2)));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::compare(
            scalar::comparison_operator::equal,
            scalar::immediate(value::int4(1), type::int4()),
            scalar::immediate(value::int4(2), type::int4())));
}

TEST_F(scalar_expression_translator_test, binary_operator_match) {
    using Op = ::shakujo::model::expression::BinaryOperator::Kind;
    auto s = f.BinaryOperator(
            Op::LIKE,
            f.Literal(tinfo::String(), vinfo::String("ABC")),
            f.Literal(tinfo::String(), vinfo::String("A%")));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::match(
            scalar::match_operator::like,
            scalar::immediate(value::character("ABC"), type::character(type::varying)),
            scalar::immediate(value::character("A%"), type::character(type::varying)),
            scalar::immediate(value::character(""), type::character(0))));
}

TEST_F(scalar_expression_translator_test, binary_operator_match_complement) {
    using Op = ::shakujo::model::expression::BinaryOperator::Kind;
    auto s = f.BinaryOperator(
            Op::NOT_LIKE,
            f.Literal(tinfo::String(), vinfo::String("ABC")),
            f.Literal(tinfo::String(), vinfo::String("A%")));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::unary(
            scalar::unary_operator::conditional_not,
            scalar::match(
                    scalar::match_operator::like,
                    scalar::immediate(value::character("ABC"), type::character(type::varying)),
                    scalar::immediate(value::character("A%"), type::character(type::varying)),
                    scalar::immediate(value::character(""), type::character(0)))));
}

TEST_F(scalar_expression_translator_test, binary_operator_unsupported) {
    using Op = ::shakujo::model::expression::BinaryOperator::Kind;
    auto s = f.BinaryOperator(
            Op::RELATION_IN,
            f.Literal(tinfo::String(), vinfo::String("ABC")),
            f.Literal(tinfo::String(), vinfo::String("A%")));
    auto r = engine.process(*s, { options, {} });
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::unsupported_scalar_expression, entry.diagnostics()));
}

TEST_F(scalar_expression_translator_test, cast) {
    using Op = ::shakujo::model::expression::TypeOperator::Kind;
    auto s = f.TypeOperator(
            Op::CAST,
            f.Int32Type(),
            f.Literal(tinfo::Int(64), vinfo::Int(1)));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::cast(
            type::int4(),
            scalar::cast::loss_policy_type::ignore,
            scalar::immediate(value::int4(1), type::int8())));
}

TEST_F(scalar_expression_translator_test, cast_implicit) {
    auto s = f.ImplicitCast(
            tinfo::Int(32),
            f.Literal(tinfo::Int(64), vinfo::Int(1)));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::cast(
            type::int4(),
            scalar::cast::loss_policy_type::ignore,
            scalar::immediate(value::int4(1), type::int8())));
}

TEST_F(scalar_expression_translator_test, placeholder) {
    placeholders.add(":x", { value::int4(1), type::int8() });
    auto s = f.Placeholder(":x");
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::immediate(value::int4(1), type::int8()));
}

TEST_F(scalar_expression_translator_test, placeholder_missing) {
    auto s = f.Placeholder(":x");
    auto r = engine.process(*s, { options, {} });
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::variable_not_found, entry.diagnostics()));
}

TEST_F(scalar_expression_translator_test, function_call) {
    auto d = functions->add({
            20'001,
            "f",
            type::int4 {},
            {
                    type::int8 {},
            },
    });
    auto s = f.FunctionCall(
            f.Name("f"),
            f.Literal(tinfo::Int(64), vinfo::Int(1)));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::function_call(
            bindings(d),
            {
                    scalar::immediate(value::int4(1), type::int8()),
            }));
    EXPECT_FALSE(engine.saw_aggregate());
}

TEST_F(scalar_expression_translator_test, function_call_overload) {
    auto d = functions->add({
            20'001,
            "f",
            type::int4 {},
            {
                    type::int8 {},
            },
    });
    functions->add({
            20'002,
            "f",
            type::int4 {},
            {
                    type::boolean {},
            },
    });
    auto s = f.FunctionCall(
            f.Name("f"),
            f.Literal(tinfo::Int(64), vinfo::Int(1)));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::function_call(
            bindings(d),
            {
                    scalar::immediate(value::int4(1), type::int8()),
            }));
}

TEST_F(scalar_expression_translator_test, function_call_overload_candidates) {
    auto d = functions->add({
            20'001,
            "f",
            type::int4 {},
            {
                    type::int4 {},
            },
    });
    functions->add({
            20'002,
            "f",
            type::int8 {},
            {
                    type::int8 {},
            },
    });
    auto s = f.FunctionCall(
            f.Name("f"),
            f.Literal(tinfo::Int(64), vinfo::Int(1)));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, scalar::function_call(
            bindings(d),
            {
                    scalar::immediate(value::int4(1), type::int4()),
            }));
}

TEST_F(scalar_expression_translator_test, function_call_missing) {
    auto s = f.FunctionCall(
            f.Name("f"),
            f.Literal(tinfo::Int(64), vinfo::Int(1)));
    auto r = engine.process(*s, { options, {} });
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::function_not_found, entry.diagnostics()));
}

TEST_F(scalar_expression_translator_test, function_call_overload_umbiguous) {
    functions->add({
            20'001,
            "f",
            type::int4 {},
            {
                    type::decimal { 5, 2 },
            },
    });
    functions->add({
            20'002,
            "f",
            type::int8 {},
            {
                    type::int4 {},
            },
    });
    auto s = f.FunctionCall(
            f.Name("f"),
            f.Literal(tinfo::Int(8), vinfo::Int(1)));
    auto r = engine.process(*s, { options, {} });
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::function_ambiguous, entry.diagnostics()));
}

TEST_F(scalar_expression_translator_test, function_call_child_error) {
    auto s = f.FunctionCall(
            f.Name("f"),
            f.BinaryOperator(
                    ::shakujo::model::expression::BinaryOperator::Kind::ADD,
                    f.Literal(tinfo::Null(), vinfo::Null()),
                    f.Literal(tinfo::Null(), vinfo::Null())));
    auto r = engine.process(*s, { options, {} });
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::type_error, entry.diagnostics()));
}

TEST_F(scalar_expression_translator_test, aggregate_function_call_quantifier_absent) {
    auto d = aggregates->add({
            20'001,
            "f",
            quantifier::all,
            type::int4 {},
            {
                    type::int8 {},
            },
    });
    aggregates->add({
            20'002,
            "f",
            quantifier::distinct,
            type::int4 {},
            {
                    type::int8 {},
            },
    });
    auto s = f.FunctionCall(
            f.Name("f"),
            f.Literal(tinfo::Int(64), vinfo::Int(1)));
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, extension::scalar::aggregate_function_call(
            bindings(d),
            {
                    scalar::immediate(value::int4(1), type::int8()),
            }));
    EXPECT_TRUE(engine.saw_aggregate());
}

TEST_F(scalar_expression_translator_test, aggregate_function_call_quantifier_all) {
    auto d = aggregates->add({
            20'001,
            "f",
            quantifier::all,
            type::int4 {},
            {
                    type::int8 {},
            },
    });
    aggregates->add({
            20'002,
            "f",
            quantifier::distinct,
            type::int4 {},
            {
                    type::int8 {},
            },
    });
    auto s = f.FunctionCall(
            f.Name("f"),
            f.Literal(tinfo::Int(64), vinfo::Int(1)),
            ::shakujo::model::expression::FunctionCall::Quantifier::ALL);
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, extension::scalar::aggregate_function_call(
            bindings(d),
            {
                    scalar::immediate(value::int4(1), type::int8()),
            }));
    EXPECT_TRUE(engine.saw_aggregate());
}

TEST_F(scalar_expression_translator_test, aggregate_function_call_quantifier_distinct) {
    aggregates->add({
            20'001,
            "f",
            quantifier::all,
            type::int4 {},
            {
                    type::int8 {},
            },
    });
    auto d = aggregates->add({
            20'002,
            "f",
            quantifier::distinct,
            type::int4 {},
            {
                    type::int8 {},
            },
    });
    auto s = f.FunctionCall(
            f.Name("f"),
            f.Literal(tinfo::Int(64), vinfo::Int(1)),
            ::shakujo::model::expression::FunctionCall::Quantifier::DISTINCT);
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, extension::scalar::aggregate_function_call(
            bindings(d),
            {
                    scalar::immediate(value::int4(1), type::int8()),
            }));
    EXPECT_TRUE(engine.saw_aggregate());
}

TEST_F(scalar_expression_translator_test, aggregate_function_call_quantifier_asterisk) {
    auto d = aggregates->add({
            20'001,
            "f",
            quantifier::all,
            type::int4 {},
            {},
    });
    auto s = f.FunctionCall(
            f.Name("f"),
            {},
            ::shakujo::model::expression::FunctionCall::Quantifier::ASTERISK);
    auto r = engine.process(*s, { options, {} });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, extension::scalar::aggregate_function_call(
            bindings(d),
            {}));
    EXPECT_TRUE(engine.saw_aggregate());
}

TEST_F(scalar_expression_translator_test, aggregate_function_call_missing) {
    auto s = f.FunctionCall(
            f.Name("f"),
            f.Literal(tinfo::Int(64), vinfo::Int(1)),
            ::shakujo::model::expression::FunctionCall::Quantifier::ALL);
    auto r = engine.process(*s, { options, {} });
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::function_not_found, entry.diagnostics()));
}

TEST_F(scalar_expression_translator_test, aggregate_function_call_overload_umbiguous) {
    aggregates->add({
            20'001,
            "f",
            quantifier::all,
            type::int4 {},
            {
                    type::decimal { 5, 2 },
            },
    });
    aggregates->add({
            20'002,
            "f",
            quantifier::all,
            type::int8 {},
            {
                    type::int4 {},
            },
    });
    auto s = f.FunctionCall(
            f.Name("f"),
            f.Literal(tinfo::Int(64), vinfo::Int(1)),
            ::shakujo::model::expression::FunctionCall::Quantifier::ALL);
    auto r = engine.process(*s, { options, {} });
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::function_ambiguous, entry.diagnostics()));
}

TEST_F(scalar_expression_translator_test, aggregate_function_call_overload_conflict) {
    functions->add({
            20'001,
            "f",
            type::int4 {},
            {
                    type::int4 {},
            },
    });
    aggregates->add({
            20'002,
            "f",
            quantifier::all,
            type::int8 {},
            {
                    type::int4 {},
            },
    });
    auto s = f.FunctionCall(
            f.Name("f"),
            f.Literal(tinfo::Int(32), vinfo::Int(1)));
    auto r = engine.process(*s, { options, {} });
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::function_ambiguous, entry.diagnostics()));
}

} // namespace mizugaki::translator::details
