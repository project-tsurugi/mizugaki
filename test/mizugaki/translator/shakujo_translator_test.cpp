#include <mizugaki/translator/shakujo_translator.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/value/primitive.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/variable_reference.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/emit.h>
#include <takatori/relation/write.h>

#include <takatori/statement/write.h>

#include <takatori/document/basic_document.h>

#include <yugawara/compiler.h>
#include <yugawara/binding/factory.h>
#include <yugawara/storage/configurable_provider.h>
#include <yugawara/function/configurable_provider.h>
#include <yugawara/aggregate/configurable_provider.h>

#include <shakujo/common/core/type/Int.h>
#include <shakujo/common/core/value/Int.h>
#include <shakujo/model/IRFactory.h>

#include <mizugaki/translator/testing/utils.h>

namespace mizugaki::translator {

using namespace ::mizugaki::translator::testing;
using code = shakujo_translator_code;
using result_kind = shakujo_translator::result_type::kind_type;

namespace type = ::takatori::type;
namespace value = ::takatori::value;
namespace scalar = ::takatori::scalar;
namespace relation = ::takatori::relation;
namespace statement = ::takatori::statement;

namespace tinfo = ::shakujo::common::core::type;

class shakujo_translator_test : public ::testing::Test {
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

    shakujo_translator translator;
    shakujo_translator_options options {
            storages,
            {},
            {},
            {},
    };
    placeholder_map placeholders;
    ::takatori::document::document_map documents;
    ::shakujo::model::IRFactory ir;
    ::yugawara::binding::factory bindings { options.get_object_creator() };
};

void compile(shakujo_translator_test& test, relation::graph_type&& graph) {
    ::yugawara::compiler_options options {
            test.storages,
    };
    ::yugawara::compiler compiler {};
    auto r = compiler(options, std::move(graph));
    if (!r) {
        throw std::runtime_error("fail");
    }
}

TEST_F(shakujo_translator_test, plan) {
    auto s = ir.EmitStatement(ir.ScanExpression(ir.Name("T0")));
    auto r = translator(options, *s, documents, placeholders);
    ASSERT_EQ(r.kind(), result_kind::execution_plan);

    auto ptr = r.release<result_kind::execution_plan>();
    auto&& graph = *ptr;
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

    compile(*this, std::move(graph));
}

TEST_F(shakujo_translator_test, statement) {
    auto s = ir.InsertValuesStatement(
            ir.Name("T0"),
            {
                    ir.InsertValuesStatementColumn(ir.Name("C0"), ir.Literal(tinfo::Int(32), 0)),
                    ir.InsertValuesStatementColumn(ir.Name("C1"), ir.Literal(tinfo::Int(32), 1)),
                    ir.InsertValuesStatementColumn(ir.Name("C2"), ir.Literal(tinfo::Int(32), 2)),
            });
    auto r = translator(options, *s, documents, placeholders);
    ASSERT_EQ(r.kind(), result_kind::statement);

    auto ptr = r.release<result_kind::statement>();
    auto&& write = downcast<statement::write>(*ptr);

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

TEST_F(shakujo_translator_test, diagnostics) {
    auto s = ir.EmitStatement(ir.ScanExpression(ir.Name("MISSING")));
    auto r = translator(options, *s, documents, placeholders);
    ASSERT_EQ(r.kind(), result_kind::diagnostics);

    auto v = r.release<result_kind::diagnostics>();
    EXPECT_TRUE(occurred(code::table_not_found, v));
}

TEST_F(shakujo_translator_test, document) {
    ::takatori::document::basic_document doc {
            "testing.sql",
            "?\nMISSING\n?"
    };
    documents.add(doc);

    auto name = ir.Name("MISSING");
    name->region(::shakujo::common::core::DocumentRegion {
            "testing.sql",
            { 2, 1 },
            { 2, 7 },
    });

    auto s = ir.EmitStatement(ir.ScanExpression(std::move(name)));
    auto r = translator(options, *s, documents, placeholders);
    ASSERT_EQ(r.kind(), result_kind::diagnostics);

    auto v = r.release<result_kind::diagnostics>();
    auto d = occurred(code::table_not_found, v);
    ASSERT_TRUE(d);
    EXPECT_EQ(d->location().contents(), "MISSING");
}

TEST_F(shakujo_translator_test, placeholder) {
    placeholders
            .add("p0", { type::int4(), value::int4(0) })
            .add("p1", { type::int4(), value::int4(1) })
            .add("p2", { type::int4(), value::int4(2) });

    auto s = ir.InsertValuesStatement(
            ir.Name("T0"),
            {
                    ir.InsertValuesStatementColumn(ir.Name("C0"), ir.Placeholder("p0")),
                    ir.InsertValuesStatementColumn(ir.Name("C1"), ir.Placeholder("p1")),
                    ir.InsertValuesStatementColumn(ir.Name("C2"), ir.Placeholder("p2")),
            });
    auto r = translator(options, *s, documents, placeholders);
    ASSERT_EQ(r.kind(), result_kind::statement);

    auto ptr = r.release<result_kind::statement>();
    auto&& write = downcast<statement::write>(*ptr);

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

TEST_F(shakujo_translator_test, compile_group_by_key_only) {
    auto s = ir.EmitStatement(
            ir.ProjectionExpression(
                    ir.GroupExpression(
                            ir.ScanExpression(ir.Name("T0")),
                            {
                                    ir.VariableReference(ir.Name("C0")),
                            }
                    ),
                    {
                            ir.ProjectionExpressionColumn(
                                    ir.VariableReference(ir.Name("C0"))),
                    }
            )
    );
    auto r = translator(options, *s, documents, placeholders);
    ASSERT_EQ(r.kind(), result_kind::execution_plan);

    auto ptr = r.release<result_kind::execution_plan>();
    auto&& graph = *ptr;
    compile(*this, std::move(graph));
}

} // namespace mizugaki::translator
