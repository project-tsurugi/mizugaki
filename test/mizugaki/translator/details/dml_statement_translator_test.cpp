#include <mizugaki/translator/details/dml_statement_translator.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/value/primitive.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/variable_reference.h>
#include <takatori/scalar/binary.h>
#include <takatori/scalar/compare.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/emit.h>
#include <takatori/relation/write.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>
#include <takatori/relation/intermediate/join.h>
#include <takatori/relation/intermediate/distinct.h>
#include <takatori/relation/intermediate/limit.h>
#include <takatori/relation/intermediate/aggregate.h>

#include <takatori/statement/write.h>

#include <yugawara/binding/factory.h>
#include <yugawara/storage/configurable_provider.h>

#include <shakujo/common/core/type/Int.h>
#include <shakujo/common/core/value/Int.h>
#include <shakujo/model/IRFactory.h>

#include <mizugaki/translator/testing/utils.h>

namespace mizugaki::translator::details {

using namespace ::mizugaki::translator::testing;
using code = shakujo_translator_diagnostic_code;
using result_kind = dml_statement_translator::result_type::kind_type;

namespace type = ::takatori::type;
namespace value = ::takatori::value;
namespace scalar = ::takatori::scalar;
namespace relation = ::takatori::relation;
namespace statement = ::takatori::statement;

namespace tinfo = ::shakujo::common::core::type;

class dml_statement_translator_test : public ::testing::Test {
public:
    std::shared_ptr<::yugawara::storage::configurable_provider> storages
            = std::make_shared<::yugawara::storage::configurable_provider>();

    std::shared_ptr<::yugawara::storage::table> t0 = storages->add_table("T0", {
            "T0",
            {
                    { "C0", type::int4() },
                    { "C1", type::int4() },
                    { "C2", type::int4() },
            },
    });
    std::shared_ptr<::yugawara::storage::table> t1 = storages->add_table("T1", {
            "T1",
            {
                    { "C0", type::int4() },
                    { "C1", type::int4() },
                    { "C2", type::int4() },
            },
    });

    std::shared_ptr<::yugawara::storage::index> i0 = storages->add_index("I0", {
            t0,
            "I0",
            {
                    t0->columns()[0],
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::unique,
                    ::yugawara::storage::index_feature::primary,
            },
    });
    std::shared_ptr<::yugawara::storage::index> i1 = storages->add_index("I1", {
            t1,
            "I1",
            {
                    t1->columns()[0],
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::unique,
                    ::yugawara::storage::index_feature::primary,
            },
    });

    shakujo_translator::impl entry { new_translator_impl() };
    shakujo_translator_options options { new_options(storages) };
    dml_statement_translator engine { entry.initialize(options) };

    ::shakujo::model::IRFactory ir;
    ::yugawara::binding::factory bindings { options.get_object_creator() };
};

TEST_F(dml_statement_translator_test, emit) {
    auto s = ir.EmitStatement(ir.ScanExpression(ir.Name("T0")));
    auto r = engine.process(*s);
    auto&& graph = r.element<result_kind::execution_plan>();
    auto&& emit = last<relation::emit>(graph);
    auto&& scan = next<relation::scan>(emit.input());

    ASSERT_EQ(scan.columns().size(), 3);
    ASSERT_EQ(emit.columns().size(), 3);

    EXPECT_EQ(emit.columns()[0].source(), scan.columns()[0].destination());
    EXPECT_EQ(emit.columns()[1].source(), scan.columns()[1].destination());
    EXPECT_EQ(emit.columns()[2].source(), scan.columns()[2].destination());
    EXPECT_EQ(emit.columns()[0].name(), "C0");
    EXPECT_EQ(emit.columns()[1].name(), "C1");
    EXPECT_EQ(emit.columns()[2].name(), "C2");
}

TEST_F(dml_statement_translator_test, emit_project) {
    auto s = ir.EmitStatement(ir.ProjectionExpression(
            ir.ScanExpression(ir.Name("T0")),
            {
                    ir.ProjectionExpressionColumn(
                            ir.VariableReference(ir.Name("C1")),
                            ir.SimpleName("p0")),
            }));
    auto r = engine.process(*s);
    auto&& graph = r.element<result_kind::execution_plan>();
    auto&& emit = last<relation::emit>(graph);
    auto&& scan = next<relation::scan>(emit.input());

    ASSERT_EQ(scan.columns().size(), 3);
    ASSERT_EQ(emit.columns().size(), 1);

    EXPECT_EQ(emit.columns()[0].source(), scan.columns()[1].destination());
    EXPECT_EQ(emit.columns()[0].name(), "p0");
}

TEST_F(dml_statement_translator_test, insert_relations) {
    auto s = ir.InsertRelationStatement(
            ir.Name("T0"),
            {},
            ir.ScanExpression(ir.Name("T1")));
    auto r = engine.process(*s);
    auto&& graph = r.element<result_kind::execution_plan>();
    auto&& write = last<relation::write>(graph);
    auto&& scan = next<relation::scan>(write.input());

    EXPECT_EQ(write.operator_kind(), relation::write_kind::insert);

    ASSERT_EQ(scan.columns().size(), 3);
    ASSERT_EQ(write.columns().size(), 3);
    EXPECT_EQ(write.columns()[0].source(), scan.columns()[0].destination());
    EXPECT_EQ(write.columns()[1].source(), scan.columns()[1].destination());
    EXPECT_EQ(write.columns()[2].source(), scan.columns()[2].destination());
    EXPECT_EQ(write.columns()[0].destination(), bindings(t0->columns()[0]));
    EXPECT_EQ(write.columns()[1].destination(), bindings(t0->columns()[1]));
    EXPECT_EQ(write.columns()[2].destination(), bindings(t0->columns()[2]));

    ASSERT_EQ(write.keys().size(), 1);
    EXPECT_EQ(write.keys()[0].source(), scan.columns()[0].destination());
    EXPECT_EQ(write.keys()[0].destination(), bindings(t0->columns()[0]));
}

TEST_F(dml_statement_translator_test, insert_relations_columns) {
    auto s = ir.InsertRelationStatement(
            ir.Name("T0"),
            {
                    ir.Name("C2"),
                    ir.Name("C1"),
                    ir.Name("C0"),
            },
            ir.ScanExpression(ir.Name("T1")));
    auto r = engine.process(*s);
    auto&& graph = r.element<result_kind::execution_plan>();
    auto&& write = last<relation::write>(graph);
    auto&& scan = next<relation::scan>(write.input());

    EXPECT_EQ(write.operator_kind(), relation::write_kind::insert);

    ASSERT_EQ(scan.columns().size(), 3);
    ASSERT_EQ(write.columns().size(), 3);
    EXPECT_EQ(write.columns()[0].source(), scan.columns()[0].destination());
    EXPECT_EQ(write.columns()[1].source(), scan.columns()[1].destination());
    EXPECT_EQ(write.columns()[2].source(), scan.columns()[2].destination());
    EXPECT_EQ(write.columns()[0].destination(), bindings(t0->columns()[2]));
    EXPECT_EQ(write.columns()[1].destination(), bindings(t0->columns()[1]));
    EXPECT_EQ(write.columns()[2].destination(), bindings(t0->columns()[0]));

    ASSERT_EQ(write.keys().size(), 1);
    EXPECT_EQ(write.keys()[0].source(), scan.columns()[2].destination());
    EXPECT_EQ(write.keys()[0].destination(), bindings(t0->columns()[0]));
}

TEST_F(dml_statement_translator_test, insert_relations_inconsistent) {
    auto s = ir.InsertRelationStatement(
            ir.Name("T0"),
            {
                    ir.Name("C0"),
            },
            ir.ScanExpression(ir.Name("T1")));
    auto r = engine.process(*s);
    EXPECT_FALSE(r);
    EXPECT_TRUE(occurred(code::inconsistent_columns, entry.diagnostics()));
}

TEST_F(dml_statement_translator_test, update) {
    // UPDATE T0 SET C1 = C1 + 1
    auto s = ir.UpdateStatement(
            ir.ScanExpression(ir.Name("T0")),
            ir.Name("T0"),
            {},
            {
                    ir.UpdateStatementColumn(
                            ir.Name("C1"),
                            ir.BinaryOperator(
                                    ::shakujo::model::expression::BinaryOperator::Kind::ADD,
                                    ir.VariableReference(ir.Name("C1")),
                                    ir.Literal(tinfo::Int(32), 1))),
            });
    auto r = engine.process(*s);
    auto&& graph = r.element<result_kind::execution_plan>();
    auto&& write = last<relation::write>(graph);
    auto&& prj = next<relation::project>(write.input());
    auto&& scan = next<relation::scan>(prj.input());

    EXPECT_EQ(write.operator_kind(), relation::write_kind::update);

    ASSERT_EQ(scan.columns().size(), 3);
    ASSERT_EQ(prj.columns().size(), 1);
    EXPECT_EQ(prj.columns()[0].value(), scalar::binary(
            scalar::binary_operator::add,
            scalar::variable_reference(scan.columns()[1].destination()),
            scalar::immediate(value::int4(1), type::int4())));

    ASSERT_EQ(write.columns().size(), 1);
    EXPECT_EQ(write.columns()[0].source(), prj.columns()[0].variable());
    EXPECT_EQ(write.columns()[0].destination(), bindings(t0->columns()[1]));

    ASSERT_EQ(write.keys().size(), 1);
    EXPECT_EQ(write.keys()[0].source(), scan.columns()[0].destination());
    EXPECT_EQ(write.keys()[0].destination(), bindings(t0->columns()[0]));
}

TEST_F(dml_statement_translator_test, update_direct) {
    // UPDATE T0 SET C1 = C2, C2 = C1
    auto s = ir.UpdateStatement(
            ir.ScanExpression(ir.Name("T0")),
            ir.Name("T0"),
            {},
            {
                    ir.UpdateStatementColumn(
                            ir.Name("C1"),
                            ir.VariableReference(ir.Name("C2"))),
                    ir.UpdateStatementColumn(
                            ir.Name("C2"),
                            ir.VariableReference(ir.Name("C1"))),
            });
    auto r = engine.process(*s);
    auto&& graph = r.element<result_kind::execution_plan>();
    auto&& write = last<relation::write>(graph);
    auto&& scan = next<relation::scan>(write.input());

    EXPECT_EQ(write.operator_kind(), relation::write_kind::update);

    ASSERT_EQ(scan.columns().size(), 3);
    ASSERT_EQ(write.columns().size(), 2);
    EXPECT_EQ(write.columns()[0].source(), scan.columns()[2].destination());
    EXPECT_EQ(write.columns()[1].source(), scan.columns()[1].destination());
    EXPECT_EQ(write.columns()[0].destination(), bindings(t0->columns()[1]));
    EXPECT_EQ(write.columns()[1].destination(), bindings(t0->columns()[2]));

    ASSERT_EQ(write.keys().size(), 1);
    EXPECT_EQ(write.keys()[0].source(), scan.columns()[0].destination());
    EXPECT_EQ(write.keys()[0].destination(), bindings(t0->columns()[0]));
}

TEST_F(dml_statement_translator_test, update_conditional) {
    // UPDATE T0 SET C1 = C2, C2 = C1 WHERE C1 != C2
    auto s = ir.UpdateStatement(
            ir.SelectionExpression(
                    ir.ScanExpression(ir.Name("T0")),
                    ir.BinaryOperator(
                            ::shakujo::model::expression::BinaryOperator::Kind::NOT_EQUAL,
                            ir.VariableReference(ir.Name("C1")),
                            ir.VariableReference(ir.Name("C2")))),
            ir.Name("T0"),
            {},
            {
                    ir.UpdateStatementColumn(
                            ir.Name("C1"),
                            ir.VariableReference(ir.Name("C2"))),
                    ir.UpdateStatementColumn(
                            ir.Name("C2"),
                            ir.VariableReference(ir.Name("C1"))),
            });
    auto r = engine.process(*s);
    auto&& graph = r.element<result_kind::execution_plan>();
    auto&& write = last<relation::write>(graph);
    auto&& filter = next<relation::filter>(write.input());
    auto&& scan = next<relation::scan>(filter.input());

    EXPECT_EQ(write.operator_kind(), relation::write_kind::update);

    ASSERT_EQ(scan.columns().size(), 3);

    EXPECT_EQ(filter.condition(), scalar::compare(
            scalar::comparison_operator::not_equal,
            scalar::variable_reference(scan.columns()[1].destination()),
            scalar::variable_reference(scan.columns()[2].destination())));
}

TEST_F(dml_statement_translator_test, update_inconsistent_table) {
    auto s = ir.UpdateStatement(
            ir.ScanExpression(ir.Name("T0")),
            ir.Name("T1"),
            {},
            {
                    ir.UpdateStatementColumn(
                            ir.Name("C1"),
                            ir.VariableReference(ir.Name("C2"))),
            });
    auto r = engine.process(*s);
    EXPECT_FALSE(r);
    EXPECT_TRUE(occurred(code::inconsistent_table, entry.diagnostics()));
}

TEST_F(dml_statement_translator_test, delete) {
    auto s = ir.DeleteStatement(
            ir.ScanExpression(ir.Name("T0")),
            ir.Name("T0"));
    auto r = engine.process(*s);
    auto&& graph = r.element<result_kind::execution_plan>();
    auto&& write = last<relation::write>(graph);
    auto&& scan = next<relation::scan>(write.input());

    EXPECT_EQ(write.operator_kind(), relation::write_kind::delete_);

    ASSERT_EQ(scan.columns().size(), 3);
    ASSERT_EQ(write.columns().size(), 0);
    ASSERT_EQ(write.keys().size(), 1);
    EXPECT_EQ(write.keys()[0].source(), scan.columns()[0].destination());
    EXPECT_EQ(write.keys()[0].destination(), bindings(t0->columns()[0]));
}

TEST_F(dml_statement_translator_test, delete_conditional) {
    auto s = ir.DeleteStatement(
            ir.SelectionExpression(
                    ir.ScanExpression(ir.Name("T0")),
                    ir.BinaryOperator(
                            ::shakujo::model::expression::BinaryOperator::Kind::NOT_EQUAL,
                            ir.VariableReference(ir.Name("C1")),
                            ir.VariableReference(ir.Name("C2")))),
            ir.Name("T0"));
    auto r = engine.process(*s);
    auto&& graph = r.element<result_kind::execution_plan>();
    auto&& write = last<relation::write>(graph);
    auto&& filter = next<relation::filter>(write.input());
    auto&& scan = next<relation::scan>(filter.input());

    EXPECT_EQ(write.operator_kind(), relation::write_kind::delete_);

    ASSERT_EQ(scan.columns().size(), 3);

    EXPECT_EQ(filter.condition(), scalar::compare(
            scalar::comparison_operator::not_equal,
            scalar::variable_reference(scan.columns()[1].destination()),
            scalar::variable_reference(scan.columns()[2].destination())));
}

TEST_F(dml_statement_translator_test, insert_values) {
    auto s = ir.InsertValuesStatement(
            ir.Name("T0"),
            {
                    ir.InsertValuesStatementColumn(ir.Name("C0"), ir.Literal(tinfo::Int(32), 0)),
                    ir.InsertValuesStatementColumn(ir.Name("C1"), ir.Literal(tinfo::Int(32), 1)),
                    ir.InsertValuesStatementColumn(ir.Name("C2"), ir.Literal(tinfo::Int(32), 2)),
            });
    auto r = engine.process(*s);
    auto&& write = downcast<statement::write>(r.element<result_kind::statement>());

    EXPECT_EQ(write.operator_kind(), relation::write_kind::insert);

    ASSERT_EQ(write.columns().size(), 3);
    EXPECT_EQ(write.columns()[0], bindings(t0->columns()[0]));
    EXPECT_EQ(write.columns()[1], bindings(t0->columns()[1]));
    EXPECT_EQ(write.columns()[2], bindings(t0->columns()[2]));

    ASSERT_EQ(write.tuples().size(), 1);
    auto&& es = write.tuples()[0].elements();
    ASSERT_EQ(es.size(), 3);
    EXPECT_EQ(es[0], scalar::immediate(value::int4(0), type::int4()));
    EXPECT_EQ(es[1], scalar::immediate(value::int4(1), type::int4()));
    EXPECT_EQ(es[2], scalar::immediate(value::int4(2), type::int4()));
}

} // namespace mizugaki::translator::details
