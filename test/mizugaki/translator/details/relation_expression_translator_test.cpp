#include <mizugaki/translator/details/relation_expression_translator.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/value/primitive.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/variable_reference.h>
#include <takatori/scalar/binary.h>
#include <takatori/scalar/compare.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>
#include <takatori/relation/intermediate/join.h>
#include <takatori/relation/intermediate/distinct.h>
#include <takatori/relation/intermediate/limit.h>
#include <takatori/relation/intermediate/aggregate.h>

#include <yugawara/binding/factory.h>
#include <yugawara/storage/configurable_provider.h>
#include <yugawara/function/configurable_provider.h>
#include <yugawara/aggregate/configurable_provider.h>

#include <shakujo/common/core/type/Int.h>
#include <shakujo/common/core/value/Int.h>
#include <shakujo/model/IRFactory.h>

#include <mizugaki/translator/testing/utils.h>

namespace mizugaki::translator::details {

using namespace ::mizugaki::translator::testing;
using code = shakujo_translator_code;

namespace type = ::takatori::type;
namespace value = ::takatori::value;
namespace scalar = ::takatori::scalar;
namespace relation = ::takatori::relation;

namespace tinfo = ::shakujo::common::core::type;

class relation_expression_translator_test : public ::testing::Test {
public:
    std::shared_ptr<::yugawara::storage::configurable_provider> storages
            = std::make_shared<::yugawara::storage::configurable_provider>();
    std::shared_ptr<::yugawara::function::configurable_provider> functions
            = std::make_shared<::yugawara::function::configurable_provider>();
    std::shared_ptr<::yugawara::aggregate::configurable_provider> aggregates
            = std::make_shared<::yugawara::aggregate::configurable_provider>();

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
                    t1->columns()[1],
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
    shakujo_translator_options options { new_options(storages, {}, functions, aggregates) };
    relation_expression_translator engine { entry.initialize(options) };

    ::shakujo::model::IRFactory ir;
    ::yugawara::binding::factory bindings { options.get_object_creator() };

    ::takatori::relation::graph_type graph;
    relation_info rinfo;
};

TEST_F(relation_expression_translator_test, scan) {
    auto s = ir.ScanExpression(ir.Name("T0"));
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    ASSERT_TRUE(r->owner().kind() == relation::scan::tag);

    auto&& op = downcast<relation::scan>(r->owner());
    EXPECT_EQ(op.source(), bindings(*i0));

    ASSERT_EQ(op.columns().size(), 3);
    EXPECT_EQ(op.columns()[0].source(), bindings(t0->columns()[0]));
    EXPECT_EQ(op.columns()[1].source(), bindings(t0->columns()[1]));
    EXPECT_EQ(op.columns()[2].source(), bindings(t0->columns()[2]));

    auto cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 3);
    EXPECT_EQ(op.columns()[0].destination(), cs[0]);
    EXPECT_EQ(op.columns()[1].destination(), cs[1]);
    EXPECT_EQ(op.columns()[2].destination(), cs[2]);

    EXPECT_FALSE(op.lower());
    EXPECT_FALSE(op.upper());
    EXPECT_FALSE(op.limit());

    EXPECT_EQ(rinfo.relation_name(), *ir.Name("T0"));
}

TEST_F(relation_expression_translator_test, scan_missing) {
    auto s = ir.ScanExpression(ir.Name("MISSING"));
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::table_not_found, entry.diagnostics()));
}

TEST_F(relation_expression_translator_test, rename) {
    auto s = ir.RenameExpression(
            ir.ScanExpression(ir.Name("T0")),
            ir.Name("R"));
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    ASSERT_TRUE(r->owner().kind() == relation::scan::tag);

    EXPECT_EQ(rinfo.relation_name(), *ir.Name("R"));
    EXPECT_EQ(rinfo.find_table_column(*ir.Name("C0")), t0->columns()[0]);
    EXPECT_EQ(rinfo.find_table_column(*ir.Name("C1")), t0->columns()[1]);
    EXPECT_EQ(rinfo.find_table_column(*ir.Name("C2")), t0->columns()[2]);
}

TEST_F(relation_expression_translator_test, rename_columns) {
    auto s = ir.RenameExpression(
            ir.ScanExpression(ir.Name("T0")),
            ir.Name("R"),
            {
                    ir.Name("S0"),
                    ir.Name("S1"),
                    ir.Name("S2"),
            });
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    ASSERT_TRUE(r->owner().kind() == relation::scan::tag);

    EXPECT_EQ(rinfo.relation_name(), *ir.Name("R"));
    EXPECT_EQ(rinfo.find_table_column(*ir.Name("S0")), t0->columns()[0]);
    EXPECT_EQ(rinfo.find_table_column(*ir.Name("S1")), t0->columns()[1]);
    EXPECT_EQ(rinfo.find_table_column(*ir.Name("S2")), t0->columns()[2]);
}

TEST_F(relation_expression_translator_test, filter) {
    auto s = ir.SelectionExpression(
            ir.ScanExpression(ir.Name("T0")),
            ir.Literal(tinfo::Bool(), true));
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    ASSERT_TRUE(r->owner().kind() == relation::filter::tag);

    auto&& op = downcast<relation::filter>(r->owner());
    EXPECT_EQ(op.input().opposite()->owner().kind(), relation::scan::tag);

    EXPECT_EQ(op.condition(), scalar::immediate(value::boolean(true), type::boolean()));

    EXPECT_EQ(rinfo.relation_name(), *ir.Name("T0"));
    EXPECT_EQ(rinfo.table_index(), *i0);
}

TEST_F(relation_expression_translator_test, project) {
    auto s = ir.ProjectionExpression(
            ir.ScanExpression(ir.Name("T0")),
            {
                    ir.ProjectionExpressionColumn(ir.VariableReference(ir.Name("C1"))),
            });
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    ASSERT_TRUE(r->owner().kind() == relation::scan::tag);

    auto&& op = downcast<relation::scan>(r->owner());
    ASSERT_EQ(op.columns().size(), 3);

    EXPECT_EQ(rinfo.relation_name(), nullptr);

    auto&& cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 1);
    EXPECT_EQ(cs[0], op.columns()[1].destination());
    EXPECT_EQ(column_name(rinfo, cs[0]), "C1");
}

TEST_F(relation_expression_translator_test, project_alias) {
    auto s = ir.ProjectionExpression(
            ir.ScanExpression(ir.Name("T0")),
            {
                    ir.ProjectionExpressionColumn(
                            ir.VariableReference(ir.Name("C1")),
                            ir.Name("A")),
            });
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);

    auto&& cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 1);
    EXPECT_EQ(column_name(rinfo, cs[0]), "A");
}

TEST_F(relation_expression_translator_test, project_complex) {
    auto s = ir.ProjectionExpression(
            ir.ScanExpression(ir.Name("T0")),
            {
                    ir.ProjectionExpressionColumn(
                            ir.VariableReference(ir.Name("C0"))),
                    ir.ProjectionExpressionColumn(
                            ir.BinaryOperator(
                                    ::shakujo::model::expression::BinaryOperator::Kind::ADD,
                                    ir.VariableReference(ir.Name("C1")),
                                    ir.VariableReference(ir.Name("C2")))),
                    ir.ProjectionExpressionColumn(
                            ir.Literal(tinfo::Int(32), 1),
                            ir.Name("v")),
            });
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);

    auto&& op = downcast<relation::project>(r->owner());
    auto&& scan = next<relation::scan>(op.input());
    ASSERT_EQ(scan.columns().size(), 3);

    ASSERT_EQ(op.columns().size(), 2);
    EXPECT_EQ(op.columns()[0].value(), scalar::binary(
            scalar::binary_operator::add,
            scalar::variable_reference(scan.columns()[1].destination()),
            scalar::variable_reference(scan.columns()[2].destination())));
    EXPECT_EQ(op.columns()[1].value(), scalar::immediate(value::int4(1), type::int4()));

    EXPECT_EQ(rinfo.relation_name(), nullptr);

    auto&& cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 3);
    EXPECT_EQ(cs[0], scan.columns()[0].destination());
    EXPECT_EQ(cs[1], op.columns()[0].variable());
    EXPECT_EQ(cs[2], op.columns()[1].variable());
    EXPECT_EQ(column_name(rinfo, cs[0]), "C0");
    EXPECT_EQ(column_name(rinfo, cs[1]), "");
    EXPECT_EQ(column_name(rinfo, cs[2]), "v");
}

TEST_F(relation_expression_translator_test, DISABLED_project_chain) { // FIXME: impl
    auto s = ir.ProjectionExpression(
            ir.ScanExpression(ir.Name("T0")),
            {
                    ir.ProjectionExpressionColumn(
                            ir.VariableReference(ir.Name("C1")),
                            ir.Name("P0")),
                    ir.ProjectionExpressionColumn(
                            ir.BinaryOperator(
                                    ::shakujo::model::expression::BinaryOperator::Kind::ADD,
                                    ir.VariableReference(ir.Name("P0")),
                                    ir.Literal(tinfo::Int(32), 1)),
                            ir.Name("P1")),
                    ir.ProjectionExpressionColumn(
                            ir.VariableReference(ir.Name("P1")),
                            ir.Name("P2")),
            });
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);

    auto&& op = downcast<relation::project>(r->owner());
    auto&& scan = next<relation::scan>(op.input());
    ASSERT_EQ(scan.columns().size(), 3);

    ASSERT_EQ(op.columns().size(), 1);
    EXPECT_EQ(op.columns()[0].value(), scalar::binary(
            scalar::binary_operator::add,
            scalar::variable_reference(scan.columns()[1].destination()),
            scalar::immediate(value::int4(1), type::int4())));

    auto&& cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 3);
    EXPECT_EQ(cs[0], scan.columns()[1].destination());
    EXPECT_EQ(cs[1], op.columns()[0].variable());
    EXPECT_EQ(cs[2], op.columns()[0].variable());
    EXPECT_EQ(column_name(rinfo, cs[0]), "P0");
    EXPECT_EQ(column_name(rinfo, cs[1]), "P1");
    EXPECT_EQ(column_name(rinfo, cs[2]), "P2");
}

TEST_F(relation_expression_translator_test, project_aggregate) {
    auto f = aggregates->add({
            20'001,
            "f",
            type::int4 {},
            {
                    type::int4 {},
            },

    });
    auto s = ir.ProjectionExpression(
            ir.ScanExpression(ir.Name("T0")),
            {
                    ir.ProjectionExpressionColumn(
                            ir.FunctionCall(
                                    ir.Name("f"),
                                    {
                                            ir.VariableReference(ir.Name("C1")),
                                    })),
            });
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    auto&& aggregate = downcast<relation::intermediate::aggregate>(r->owner());
    auto&& scan = next<relation::scan>(aggregate.input());
    ASSERT_EQ(scan.columns().size(), 3);
    ASSERT_EQ(aggregate.columns().size(), 1);
    {
        auto&& c = aggregate.columns()[0];
        EXPECT_EQ(c.function(), bindings(f));
        ASSERT_EQ(c.arguments().size(), 1);
        EXPECT_EQ(c.arguments()[0], scan.columns()[1].destination());
    }

    ASSERT_EQ(aggregate.group_keys().size(), 0);

    auto&& cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 1);
    EXPECT_EQ(cs[0], aggregate.columns()[0].destination());
    EXPECT_EQ(column_name(rinfo, cs[0]), "");
}

TEST_F(relation_expression_translator_test, project_aggregate_after_project) {
    auto f = aggregates->add({
            20'001,
            "f",
            type::int4 {},
            {
                    type::int4 {},
            },

    });
    auto s = ir.ProjectionExpression(
            ir.ScanExpression(ir.Name("T0")),
            {
                    // f(C0 + C1)
                    ir.ProjectionExpressionColumn(
                            ir.FunctionCall(
                                    ir.Name("f"),
                                    {
                                            ir.BinaryOperator(
                                                    ::shakujo::model::expression::BinaryOperator::Kind::ADD,
                                                    ir.VariableReference(ir.Name("C0")),
                                                    ir.VariableReference(ir.Name("C1"))),
                                    })),
            });
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    auto&& aggregate = downcast<relation::intermediate::aggregate>(r->owner());
    auto&& projection = next<relation::project>(aggregate.input());
    auto&& scan = next<relation::scan>(projection.input());
    ASSERT_EQ(scan.columns().size(), 3);
    ASSERT_EQ(projection.columns().size(), 1);
    EXPECT_EQ(projection.columns()[0].value(), scalar::binary(
            scalar::binary_operator::add,
            scalar::variable_reference(scan.columns()[0].destination()),
            scalar::variable_reference(scan.columns()[1].destination())));

    ASSERT_EQ(aggregate.columns().size(), 1);
    {
        auto&& c = aggregate.columns()[0];
        EXPECT_EQ(c.function(), bindings(f));
        ASSERT_EQ(c.arguments().size(), 1);
        EXPECT_EQ(c.arguments()[0], projection.columns()[0].variable());
    }

    ASSERT_EQ(aggregate.group_keys().size(), 0);

    auto&& cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 1);
    EXPECT_EQ(cs[0], aggregate.columns()[0].destination());
    EXPECT_EQ(column_name(rinfo, cs[0]), "");
}

TEST_F(relation_expression_translator_test, project_aggregate_and_project) {
    auto f1 = aggregates->add({
            20'001,
            "f1",
            type::int4 {},
            {
                    type::int4 {},
            },

    });
    auto f2 = aggregates->add({
            20'001,
            "f2",
            type::int4 {},
            {
                    type::int4 {},
            },

    });
    auto s = ir.ProjectionExpression(
            ir.ScanExpression(ir.Name("T0")),
            {
                    // f(c0) + f(c1)
                    ir.ProjectionExpressionColumn(
                            ir.BinaryOperator(
                                    ::shakujo::model::expression::BinaryOperator::Kind::ADD,
                                    ir.FunctionCall(
                                            ir.Name("f1"),
                                            {
                                                    ir.VariableReference(ir.Name("C0")),
                                            }),
                                    ir.FunctionCall(
                                            ir.Name("f2"),
                                            {
                                                    ir.VariableReference(ir.Name("C1")),
                                            })
                                    )),
            });
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    auto&& projection = downcast<relation::project>(r->owner());
    auto&& aggregate = next<relation::intermediate::aggregate>(projection.input());
    auto&& scan = next<relation::scan>(aggregate.input());
    ASSERT_EQ(scan.columns().size(), 3);
    ASSERT_EQ(aggregate.columns().size(), 2);
    {
        auto&& c = aggregate.columns()[0];
        EXPECT_EQ(c.function(), bindings(f1));
        ASSERT_EQ(c.arguments().size(), 1);
        EXPECT_EQ(c.arguments()[0], scan.columns()[0].destination());
    }
    {
        auto&& c = aggregate.columns()[1];
        EXPECT_EQ(c.function(), bindings(f2));
        ASSERT_EQ(c.arguments().size(), 1);
        EXPECT_EQ(c.arguments()[0], scan.columns()[1].destination());
    }

    ASSERT_EQ(projection.columns().size(), 1);
    EXPECT_EQ(projection.columns()[0].value(), scalar::binary(
            scalar::binary_operator::add,
            scalar::variable_reference(aggregate.columns()[0].destination()),
            scalar::variable_reference(aggregate.columns()[1].destination())));

    auto&& cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 1);
    EXPECT_EQ(cs[0], projection.columns()[0].variable());
    EXPECT_EQ(column_name(rinfo, cs[0]), "");
}

TEST_F(relation_expression_translator_test, project_aggregate_group) {
    auto f = aggregates->add({
            20'001,
            "f",
            type::int4 {},
            {
                    type::int4 {},
            },

    });
    auto s = ir.ProjectionExpression(
            ir.GroupExpression(
                    ir.ScanExpression(ir.Name("T0")),
                    {
                            ir.VariableReference(ir.Name("C0")),
                    }
            ),
            {
                    ir.ProjectionExpressionColumn(
                            ir.FunctionCall(
                                    ir.Name("f"),
                                    {
                                            ir.VariableReference(ir.Name("C1")),
                                    })),
            }
    );
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    ASSERT_TRUE(r->owner().kind() == relation::intermediate::aggregate::tag);

    auto&& aggregate = downcast<relation::intermediate::aggregate>(r->owner());
    auto&& scan = next<relation::scan>(aggregate.input());
    ASSERT_EQ(scan.columns().size(), 3);
    ASSERT_EQ(aggregate.columns().size(), 1);
    {
        auto&& c = aggregate.columns()[0];
        EXPECT_EQ(c.function(), bindings(f));
        ASSERT_EQ(c.arguments().size(), 1);
        EXPECT_EQ(c.arguments()[0], scan.columns()[1].destination());
    }

    ASSERT_EQ(aggregate.group_keys().size(), 1);
    EXPECT_EQ(aggregate.group_keys()[0], scan.columns()[0].destination());

    auto&& cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 1);
    EXPECT_EQ(cs[0], aggregate.columns()[0].destination());
    EXPECT_EQ(column_name(rinfo, cs[0]), "");
}

TEST_F(relation_expression_translator_test, project_aggregate_group_with_key) {
    auto f = aggregates->add({
            20'001,
            "f",
            type::int4 {},
            {
                    type::int4 {},
            },

    });
    auto s = ir.ProjectionExpression(
            ir.GroupExpression(
                    ir.ScanExpression(ir.Name("T0")),
                    {
                            ir.VariableReference(ir.Name("C0")),
                    }
            ),
            {
                    ir.ProjectionExpressionColumn(
                            ir.VariableReference(ir.Name("C0"))),
                    ir.ProjectionExpressionColumn(
                            ir.FunctionCall(
                                    ir.Name("f"),
                                    {
                                            ir.VariableReference(ir.Name("C1")),
                                    })),
            }
    );
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    ASSERT_TRUE(r->owner().kind() == relation::intermediate::aggregate::tag);

    auto&& aggregate = downcast<relation::intermediate::aggregate>(r->owner());
    auto&& scan = next<relation::scan>(aggregate.input());
    ASSERT_EQ(scan.columns().size(), 3);

    auto&& cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 2);
    EXPECT_EQ(cs[0], scan.columns()[0].destination());
    EXPECT_EQ(column_name(rinfo, cs[0]), "C0");
    EXPECT_EQ(cs[1], aggregate.columns()[0].destination());
    EXPECT_EQ(column_name(rinfo, cs[1]), "");
}

TEST_F(relation_expression_translator_test, project_aggregate_group_key_only) {
    auto f = aggregates->add({
            20'001,
            "f",
            type::int4 {},
            {
                    type::int4 {},
            },

    });
    auto s = ir.ProjectionExpression(
            ir.GroupExpression(
                    ir.ScanExpression(ir.Name("T0")),
                    {
                            ir.VariableReference(ir.Name("C0")),
                            ir.VariableReference(ir.Name("C1")),
                    }
            ),
            {
                    ir.ProjectionExpressionColumn(
                            ir.VariableReference(ir.Name("C0"))),
                    ir.ProjectionExpressionColumn(
                            ir.VariableReference(ir.Name("C1"))),
            }
    );
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    ASSERT_TRUE(r->owner().kind() == relation::intermediate::aggregate::tag);

    auto&& aggregate = downcast<relation::intermediate::aggregate>(r->owner());
    auto&& scan = next<relation::scan>(aggregate.input());
    ASSERT_EQ(scan.columns().size(), 3);

    auto&& cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 2);
    EXPECT_EQ(cs[0], scan.columns()[0].destination());
    EXPECT_EQ(column_name(rinfo, cs[0]), "C0");
    EXPECT_EQ(cs[1], scan.columns()[1].destination());
    EXPECT_EQ(column_name(rinfo, cs[1]), "C1");
}

TEST_F(relation_expression_translator_test, project_aggregate_bare_group_member) {
    auto s = ir.ProjectionExpression(
            ir.GroupExpression(
                    ir.ScanExpression(ir.Name("T0")),
                    {
                            ir.VariableReference(ir.Name("C0")),
                    }
            ),
            {
                    ir.ProjectionExpressionColumn(
                            ir.VariableReference(ir.Name("C1"))),
            }
    );
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::invalid_aggregation_column, entry.diagnostics()));
}

TEST_F(relation_expression_translator_test, project_aggregate_deep_bare_group_member) {
    auto s = ir.ProjectionExpression(
            ir.GroupExpression(
                    ir.ScanExpression(ir.Name("T0")),
                    {
                            ir.VariableReference(ir.Name("C0")),
                    }
            ),
            {
                    ir.ProjectionExpressionColumn(
                            ir.BinaryOperator(
                                    ::shakujo::model::expression::BinaryOperator::Kind::ADD,
                                    ir.VariableReference(ir.Name("C0")),
                                    ir.VariableReference(ir.Name("C1")))),
            }
    );
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::invalid_aggregation_column, entry.diagnostics()));
}

TEST_F(relation_expression_translator_test, project_aggregate_group_complex_key) {
    auto f = aggregates->add({
            20'001,
            "f",
            type::int4 {},
            {
                    type::int4 {},
            },

    });
    auto s = ir.ProjectionExpression(
            ir.GroupExpression(
                    ir.ScanExpression(ir.Name("T0")),
                    {
                            ir.BinaryOperator(
                                    ::shakujo::model::expression::BinaryOperator::Kind::ADD,
                                    ir.VariableReference(ir.Name("C0")),
                                    ir.VariableReference(ir.Name("C1"))),
                    }
            ),
            {
                    ir.ProjectionExpressionColumn(
                            ir.FunctionCall(
                                    ir.Name("f"),
                                    {
                                            ir.VariableReference(ir.Name("C1")),
                                    })),
            }
    );
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::unsupported_relational_operator, entry.diagnostics()));
}

TEST_F(relation_expression_translator_test, join) {
    auto s = ir.JoinExpression(
            ::shakujo::model::expression::relation::JoinExpression::Kind::CROSS,
            ir.ScanExpression(ir.Name("T0")),
            ir.ScanExpression(ir.Name("T1")));
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    auto&& op = downcast<relation::intermediate::join>(r->owner());
    auto&& left = next<relation::scan>(op.left());
    auto&& right = next<relation::scan>(op.right());

    EXPECT_EQ(left.source(), bindings(*i0));
    EXPECT_EQ(right.source(), bindings(*i1));

    ASSERT_EQ(op.operator_kind(), relation::join_kind::inner);
    ASSERT_EQ(op.condition(), nullptr);

    EXPECT_EQ(rinfo.relation_name(), nullptr);
    EXPECT_EQ(rinfo.count_variables(), 6);
    EXPECT_EQ(rinfo.find_variable(*ir.Name("T0", "C0")), left.columns()[0].destination());
    EXPECT_EQ(rinfo.find_variable(*ir.Name("T0", "C1")), left.columns()[1].destination());
    EXPECT_EQ(rinfo.find_variable(*ir.Name("T0", "C2")), left.columns()[2].destination());
    EXPECT_EQ(rinfo.find_variable(*ir.Name("T1", "C0")), right.columns()[0].destination());
    EXPECT_EQ(rinfo.find_variable(*ir.Name("T1", "C1")), right.columns()[1].destination());
    EXPECT_EQ(rinfo.find_variable(*ir.Name("T1", "C2")), right.columns()[2].destination());

    auto&& cs = get_columns(entry, rinfo);
    ASSERT_EQ(cs.size(), 6);
    EXPECT_EQ(cs[0], left.columns()[0].destination());
    EXPECT_EQ(cs[1], left.columns()[1].destination());
    EXPECT_EQ(cs[2], left.columns()[2].destination());
    EXPECT_EQ(cs[3], right.columns()[0].destination());
    EXPECT_EQ(cs[4], right.columns()[1].destination());
    EXPECT_EQ(cs[5], right.columns()[2].destination());
}

TEST_F(relation_expression_translator_test, join_left_outer) {
    auto s = ir.JoinExpression(
            ::shakujo::model::expression::relation::JoinExpression::Kind::LEFT_OUTER,
            ir.ScanExpression(ir.Name("T0")),
            ir.ScanExpression(ir.Name("T1")),
            ir.BinaryOperator(
                    ::shakujo::model::expression::BinaryOperator::Kind::EQUAL,
                    ir.VariableReference(ir.Name("T0", "C0")),
                    ir.VariableReference(ir.Name("T1", "C0"))));
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    auto&& op = downcast<relation::intermediate::join>(r->owner());
    auto&& left = next<relation::scan>(op.left());
    auto&& right = next<relation::scan>(op.right());

    ASSERT_EQ(op.operator_kind(), relation::join_kind::left_outer);
    ASSERT_EQ(op.condition(), scalar::compare(
            scalar::comparison_operator::equal,
            scalar::variable_reference(left.columns()[0].destination()),
            scalar::variable_reference(right.columns()[0].destination())));
}

TEST_F(relation_expression_translator_test, join_right_outer) {
    auto s = ir.JoinExpression(
            ::shakujo::model::expression::relation::JoinExpression::Kind::RIGHT_OUTER,
            ir.ScanExpression(ir.Name("T0")),
            ir.ScanExpression(ir.Name("T1")),
            ir.BinaryOperator(
                    ::shakujo::model::expression::BinaryOperator::Kind::EQUAL,
                    ir.VariableReference(ir.Name("T0", "C0")),
                    ir.VariableReference(ir.Name("T1", "C0"))));
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    auto&& op = downcast<relation::intermediate::join>(r->owner());
    auto&& left = next<relation::scan>(op.right());
    auto&& right = next<relation::scan>(op.left());

    EXPECT_EQ(left.source(), bindings(*i0));
    EXPECT_EQ(right.source(), bindings(*i1));

    ASSERT_EQ(op.operator_kind(), relation::join_kind::left_outer);
    ASSERT_EQ(op.condition(), scalar::compare(
            scalar::comparison_operator::equal,
            scalar::variable_reference(left.columns()[0].destination()),
            scalar::variable_reference(right.columns()[0].destination())));
}

TEST_F(relation_expression_translator_test, join_unsupported) {
    auto s = ir.JoinExpression(
            ::shakujo::model::expression::relation::JoinExpression::Kind::NATURAL_INNER,
            ir.ScanExpression(ir.Name("T0")),
            ir.ScanExpression(ir.Name("T1")));
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::unsupported_relational_operator, entry.diagnostics()));
}

TEST_F(relation_expression_translator_test, distinct) {
    auto s = ir.DistinctExpression(ir.ScanExpression(ir.Name("T0")));
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    auto&& op = downcast<relation::intermediate::distinct>(r->owner());
    auto&& scan = next<relation::scan>(op.input());

    ASSERT_EQ(op.group_keys().size(), 3);
    EXPECT_EQ(op.group_keys()[0], scan.columns()[0].destination());
    EXPECT_EQ(op.group_keys()[1], scan.columns()[1].destination());
    EXPECT_EQ(op.group_keys()[2], scan.columns()[2].destination());
}

TEST_F(relation_expression_translator_test, limit_order) {
    auto s = ir.OrderExpression(
            ir.ScanExpression(ir.Name("T0")),
            {
                    ir.OrderExpressionElement(
                            ir.VariableReference(ir.Name("C1")),
                            ::shakujo::model::expression::relation::OrderExpression::Direction::ASCENDANT),
                    ir.OrderExpressionElement(
                            ir.VariableReference(ir.Name("C2")),
                            ::shakujo::model::expression::relation::OrderExpression::Direction::DESCENDANT),
            });
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    auto&& op = downcast<relation::intermediate::limit>(r->owner());
    auto&& scan = next<relation::scan>(op.input());

    EXPECT_EQ(op.count(), std::nullopt);
    ASSERT_EQ(op.group_keys().size(), 0);
    ASSERT_EQ(op.sort_keys().size(), 2);
    EXPECT_EQ(op.sort_keys()[0].variable(), scan.columns()[1].destination());
    EXPECT_EQ(op.sort_keys()[0].direction(), relation::sort_direction::ascendant);
    EXPECT_EQ(op.sort_keys()[1].variable(), scan.columns()[2].destination());
    EXPECT_EQ(op.sort_keys()[1].direction(), relation::sort_direction::descendant);
}

TEST_F(relation_expression_translator_test, limit_unsupported) {
    auto s = ir.OrderExpression(
            ir.ScanExpression(ir.Name("T0")),
            {
                    ir.OrderExpressionElement(ir.BinaryOperator(
                            ::shakujo::model::expression::BinaryOperator::Kind::ADD,
                            ir.VariableReference(ir.Name("C1")),
                            ir.VariableReference(ir.Name("C2")))),
            });
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_FALSE(r);
    EXPECT_TRUE(occurred(code::unsupported_relational_operator, entry.diagnostics()));
}

TEST_F(relation_expression_translator_test, limit_count) {
    auto s = ir.LimitExpression(
            ir.ScanExpression(ir.Name("T0")),
            5);
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    auto&& op = downcast<relation::intermediate::limit>(r->owner());

    EXPECT_EQ(op.count(), 5);
    ASSERT_EQ(op.group_keys().size(), 0);
    ASSERT_EQ(op.sort_keys().size(), 0);
}

TEST_F(relation_expression_translator_test, limit_top) {
    auto s = ir.LimitExpression(
            ir.OrderExpression(
                    ir.ScanExpression(ir.Name("T0")),
                    {
                            ir.OrderExpressionElement(ir.VariableReference(ir.Name("C1"))),
                    }),
            5);
    auto r = engine.process(*s, graph, rinfo);
    ASSERT_TRUE(r);
    auto&& op = downcast<relation::intermediate::limit>(r->owner());
    auto&& scan = next<relation::scan>(op.input());

    EXPECT_EQ(op.count(), 5);
    ASSERT_EQ(op.group_keys().size(), 0);
    ASSERT_EQ(op.sort_keys().size(), 1);
    EXPECT_EQ(op.sort_keys()[0].variable(), scan.columns()[1].destination());
    EXPECT_EQ(op.sort_keys()[0].direction(), relation::sort_direction::ascendant);
}

} // namespace mizugaki::translator::details
