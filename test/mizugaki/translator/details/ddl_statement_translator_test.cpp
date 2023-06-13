#include <mizugaki/translator/details/ddl_statement_translator.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>
#include <takatori/value/primitive.h>

#include <takatori/scalar/immediate.h>

#include <takatori/statement/create_table.h>
#include <takatori/statement/create_index.h>
#include <takatori/statement/drop_table.h>
#include <takatori/statement/drop_index.h>
#include <takatori/statement/empty.h>

#include <yugawara/binding/factory.h>
#include <yugawara/binding/extract.h>
#include <yugawara/storage/configurable_provider.h>

#include <shakujo/common/core/type/Int.h>
#include <shakujo/common/core/value/Int.h>
#include <shakujo/model/IRFactory.h>

#include <mizugaki/translator/testing/utils.h>

namespace mizugaki::translator::details {

using namespace ::mizugaki::translator::testing;
using code = shakujo_translator_code;
using result_kind = ddl_statement_translator::result_type::kind_type;

using ::yugawara::binding::extract_shared;

namespace type = ::takatori::type;
namespace value = ::takatori::value;
namespace scalar = ::takatori::scalar;
namespace statement = ::takatori::statement;

namespace tinfo = ::shakujo::common::core::type;
namespace ddl = ::shakujo::model::statement::ddl;

class ddl_statement_translator_test : public ::testing::Test {
public:
    std::shared_ptr<::yugawara::storage::configurable_provider> storages
            = std::make_shared<::yugawara::storage::configurable_provider>();

    std::shared_ptr<::yugawara::storage::table> t0 = storages->add_table({
            "T0",
            {
                    {"C0", type::int4()},
                    {"C1", type::int4()},
                    {"C2", type::int4()},
            },
    });
    std::shared_ptr<::yugawara::storage::table> t1 = storages->add_table({
            "T1",
            {
                    {"C0", type::int4()},
                    {"C1", type::int4()},
                    {"C2", type::int4()},
            },
    });

    std::shared_ptr<::yugawara::storage::index> i0 = storages->add_index({
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
    std::shared_ptr<::yugawara::storage::index> i1 = storages->add_index({
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
    std::shared_ptr<::yugawara::storage::index> i0s = storages->add_index({
            t0,
            "I0S",
            {
                    t0->columns()[1],
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
            },
    });

    shakujo_translator::impl entry { new_translator_impl() };
    shakujo_translator_options options { new_options(storages) };
    ddl_statement_translator engine {entry.initialize(options) };

    ::shakujo::model::IRFactory ir;
    ::yugawara::binding::factory bindings{};
};

TEST_F(ddl_statement_translator_test, create_table) {
    auto s = ir.CreateTableStatement(
            ir.Name("test_table"),
            {
                    ir.CreateTableStatementColumn(
                            ir.Name("C0"),
                            ir.Type(tinfo::Int { 32 })),
                    ir.CreateTableStatementColumn(
                            ir.Name("C1"),
                            ir.Type(tinfo::Float { 64 })),
                    ir.CreateTableStatementColumn(
                            ir.Name("C2"),
                            ir.Type(tinfo::Char { true, 16 })),
            },
            {},
            {});
    auto r = engine.process(*s);
    auto&& stmt = downcast<statement::create_table>(r.element<result_kind::statement>());

    auto schema = extract_shared(stmt.schema());
    EXPECT_EQ(schema.get(), options.shared_schema().get());


    auto table = extract_shared(stmt.definition());
    EXPECT_FALSE(table->definition_id());
    EXPECT_EQ(table->simple_name(), "test_table");
    {
        using ::yugawara::variable::nullable;

        auto&& cols = table->columns();
        ASSERT_EQ(cols.size(), 3);

        EXPECT_EQ(cols[0].simple_name(), "C0");
        EXPECT_EQ(cols[0].type(), type::int4 {});
        EXPECT_EQ(cols[0].criteria(), nullable);
        EXPECT_FALSE(cols[0].default_value());

        EXPECT_EQ(cols[1].simple_name(), "C1");
        EXPECT_EQ(cols[1].type(), type::float8 {});
        EXPECT_EQ(cols[1].criteria(), nullable);
        EXPECT_FALSE(cols[1].default_value());

        EXPECT_EQ(cols[2].simple_name(), "C2");
        EXPECT_EQ(cols[2].type(), (type::character { type::varying, 16 }));
        EXPECT_EQ(cols[2].criteria(), nullable);
        EXPECT_FALSE(cols[2].default_value());
    }

    auto index = extract_shared<::yugawara::storage::index>(stmt.primary_key());
    EXPECT_EQ(index->shared_table(), table);
    EXPECT_TRUE(index.ownership());
    EXPECT_FALSE(index->definition_id());
    {
        auto&& keys = index->keys();
        ASSERT_EQ(keys.size(), 0);
    }
}

TEST_F(ddl_statement_translator_test, create_table_conflict) {
    auto s = ir.CreateTableStatement(
            ir.Name(t0->simple_name()),
            {
                    ir.CreateTableStatementColumn(
                            ir.Name("C0"),
                            ir.Type(tinfo::Int { 32 })),
            },
            {},
            {});
    auto r = engine.process(*s);
    ASSERT_FALSE(r);
}

TEST_F(ddl_statement_translator_test, create_table_if_not_exists) {
    auto s = ir.CreateTableStatement(
            ir.Name(t0->simple_name()),
            {
                    ir.CreateTableStatementColumn(
                            ir.Name("C0"),
                            ir.Type(tinfo::Int { 32 })),
            },
            { ddl::CreateTableStatement::Attribute::IF_NOT_EXISTS },
            {});
    auto r = engine.process(*s);
    ASSERT_TRUE(r);
    EXPECT_EQ(r.element<result_kind::statement>().kind(), statement::empty::tag);
}

TEST_F(ddl_statement_translator_test, create_table_not_null) {
    auto s = ir.CreateTableStatement(
            ir.Name("test_table"),
            {
                    ir.CreateTableStatementColumn(
                            ir.Name("C0"),
                            ir.Type(tinfo::Int { 32 }),
                            {},
                            { ddl::CreateTableStatement::Column::Attribute::NOT_NULL }),
            },
            {},
            {});
    auto r = engine.process(*s);
    auto&& stmt = downcast<statement::create_table>(r.element<result_kind::statement>());
    auto table = extract_shared(stmt.definition());
    using ::yugawara::variable::nullable;

    auto&& cols = table->columns();
    ASSERT_EQ(cols.size(), 1);

    EXPECT_EQ(cols[0].criteria(), ~nullable);
}

TEST_F(ddl_statement_translator_test, create_table_default_value) {
    auto s = ir.CreateTableStatement(
            ir.Name("test_table"),
            {
                    ir.CreateTableStatementColumn(
                            ir.Name("C0"),
                            ir.Type(tinfo::Int { 32 }),
                            ir.Literal(tinfo::Int { 32 },100),
                            {}),
            },
            {},
            {});
    auto r = engine.process(*s);
    auto&& stmt = downcast<statement::create_table>(r.element<result_kind::statement>());
    auto table = extract_shared(stmt.definition());
    using ::yugawara::variable::nullable;

    auto&& cols = table->columns();
    ASSERT_EQ(cols.size(), 1);

    auto&& dv = cols[0].default_value();
    ASSERT_EQ(dv.kind(), ::yugawara::storage::column_value::kind_type::immediate);
    auto&& v = dv.element<::yugawara::storage::column_value::kind_type::immediate>();
    EXPECT_EQ(*v, value::int4 { 100 });
}

TEST_F(ddl_statement_translator_test, create_table_primary_key_column) {
    auto s = ir.CreateTableStatement(
            ir.Name("test_table"),
            {
                    ir.CreateTableStatementColumn(
                            ir.Name("C0"),
                            ir.Type(tinfo::Int { 32 }),
                            {},
                            { ddl::CreateTableStatement::Column::Attribute::PRIMARY_KEY }),
                    ir.CreateTableStatementColumn(
                            ir.Name("C1"),
                            ir.Type(tinfo::Float { 64 })),
                    ir.CreateTableStatementColumn(
                            ir.Name("C2"),
                            ir.Type(tinfo::Char { true, 16 })),
            },
            {},
            {});
    auto r = engine.process(*s);
    auto&& stmt = downcast<statement::create_table>(r.element<result_kind::statement>());

    auto table = extract_shared(stmt.definition());
    auto&& columns = table->columns();
    ASSERT_EQ(columns.size(), 3);

    auto index = extract_shared<::yugawara::storage::index>(stmt.primary_key());
    EXPECT_TRUE(index.ownership());
    EXPECT_FALSE(index->definition_id());
    {
        auto&& keys = index->keys();
        ASSERT_EQ(keys.size(), 1);
        EXPECT_EQ(keys[0].column(), columns[0]);
        EXPECT_EQ(keys[0].column().criteria().nullity(), ~::yugawara::variable::nullable);
    }
}

TEST_F(ddl_statement_translator_test, create_table_primary_key_column_conflict) {
    auto s = ir.CreateTableStatement(
            ir.Name("test_table"),
            {
                    ir.CreateTableStatementColumn(
                            ir.Name("C0"),
                            ir.Type(tinfo::Int { 32 }),
                            {},
                            { ddl::CreateTableStatement::Column::Attribute::PRIMARY_KEY }),
                    ir.CreateTableStatementColumn(
                            ir.Name("C1"),
                            ir.Type(tinfo::Float { 64 })),
                    ir.CreateTableStatementColumn(
                            ir.Name("C2"),
                            ir.Type(tinfo::Char { true, 16 }),
                            {},
                            { ddl::CreateTableStatement::Column::Attribute::PRIMARY_KEY }),
            },
            {},
            {});
    auto r = engine.process(*s);
    ASSERT_FALSE(r);
}

TEST_F(ddl_statement_translator_test, create_table_primary_key_constraint) {
    auto s = ir.CreateTableStatement(
            ir.Name("test_table"),
            {
                    ir.CreateTableStatementColumn(
                            ir.Name("C0"),
                            ir.Type(tinfo::Int { 32 }),
                            {},
                            {}),
                    ir.CreateTableStatementColumn(
                            ir.Name("C1"),
                            ir.Type(tinfo::Int { 64 })),
                    ir.CreateTableStatementColumn(
                            ir.Name("C2"),
                            ir.Type(tinfo::Char { true, 16 })),
            },
            {},
            {
                    ir.CreateTableStatementPrimaryKey(ir.Name("C0")),
                    ir.CreateTableStatementPrimaryKey(
                            ir.Name("C1"),
                            ddl::CreateTableStatement::PrimaryKey::Direction::DESCENDANT),
            });
    auto r = engine.process(*s);
    auto&& stmt = downcast<statement::create_table>(r.element<result_kind::statement>());

    auto table = extract_shared(stmt.definition());
    auto&& columns = table->columns();
    ASSERT_EQ(columns.size(), 3);

    auto index = extract_shared<::yugawara::storage::index>(stmt.primary_key());
    EXPECT_TRUE(index.ownership());
    EXPECT_FALSE(index->definition_id());
    {
        using ::yugawara::storage::sort_direction;

        auto&& keys = index->keys();
        ASSERT_EQ(keys.size(), 2);

        EXPECT_EQ(keys[0].column(), columns[0]);
        EXPECT_EQ(keys[0].direction(), sort_direction::ascendant);
        EXPECT_EQ(keys[0].column().criteria().nullity(), ~::yugawara::variable::nullable);

        EXPECT_EQ(keys[1].column(), columns[1]);
        EXPECT_EQ(keys[1].direction(), sort_direction::descendant);
        EXPECT_EQ(keys[1].column().criteria().nullity(), ~::yugawara::variable::nullable);
    }
}

TEST_F(ddl_statement_translator_test, create_table_primary_key_constraint_conflict) {
    auto s = ir.CreateTableStatement(
            ir.Name("test_table"),
            {
                    ir.CreateTableStatementColumn(
                            ir.Name("C0"),
                            ir.Type(tinfo::Int { 32 }),
                            {},
                            { ddl::CreateTableStatement::Column::Attribute::PRIMARY_KEY }),
                    ir.CreateTableStatementColumn(
                            ir.Name("C1"),
                            ir.Type(tinfo::Int { 64 })),
                    ir.CreateTableStatementColumn(
                            ir.Name("C2"),
                            ir.Type(tinfo::Char { true, 16 })),
            },
            {},
            {
                    ir.CreateTableStatementPrimaryKey(ir.Name("C0")),
                    ir.CreateTableStatementPrimaryKey(
                            ir.Name("C1"),
                            ddl::CreateTableStatement::PrimaryKey::Direction::DESCENDANT),
            });
    auto r = engine.process(*s);
    ASSERT_FALSE(r);
}


TEST_F(ddl_statement_translator_test, create_index) {
    auto s = ir.CreateIndexStatement(
            {},
            ir.Name("T1"),
            {
                    ir.CreateIndexStatementColumn(
                            ir.Name("C1"),
                            ddl::CreateIndexStatement::Column::Direction::DONT_CARE),
            },
            {});
    auto r = engine.process(*s);
    auto&& stmt = downcast<statement::create_index>(r.element<result_kind::statement>());

    auto schema = extract_shared(stmt.schema());
    EXPECT_EQ(schema.get(), options.shared_schema().get());

    auto index = extract_shared<::yugawara::storage::index>(stmt.definition());
    EXPECT_FALSE(index->definition_id());
    EXPECT_EQ(&index->table(), t1.get());
    EXPECT_EQ(index->simple_name(), "");
    {
        auto&& cols = index->keys();
        ASSERT_EQ(cols.size(), 1);

        EXPECT_EQ(cols[0].column(), t1->columns()[1]);
        EXPECT_EQ(cols[0].direction(), ::yugawara::storage::sort_direction::ascendant);
    }
}

TEST_F(ddl_statement_translator_test, create_index_columns) {
    auto s = ir.CreateIndexStatement(
            {},
            ir.Name("T0"),
            {
                    ir.CreateIndexStatementColumn(
                            ir.Name("C0"),
                            ddl::CreateIndexStatement::Column::Direction::DONT_CARE),
                    ir.CreateIndexStatementColumn(
                            ir.Name("C1"),
                            ddl::CreateIndexStatement::Column::Direction::DONT_CARE),
                    ir.CreateIndexStatementColumn(
                            ir.Name("C2"),
                            ddl::CreateIndexStatement::Column::Direction::DONT_CARE),
            },
            {});
    auto r = engine.process(*s);
    auto&& stmt = downcast<statement::create_index>(r.element<result_kind::statement>());

    auto index = extract_shared<::yugawara::storage::index>(stmt.definition());
    {
        auto&& cols = index->keys();
        ASSERT_EQ(cols.size(), 3);

        EXPECT_EQ(cols[0].column(), t0->columns()[0]);
        EXPECT_EQ(cols[0].direction(), ::yugawara::storage::sort_direction::ascendant);

        EXPECT_EQ(cols[1].column(), t0->columns()[1]);
        EXPECT_EQ(cols[1].direction(), ::yugawara::storage::sort_direction::ascendant);

        EXPECT_EQ(cols[2].column(), t0->columns()[2]);
        EXPECT_EQ(cols[2].direction(), ::yugawara::storage::sort_direction::ascendant);
    }
}

TEST_F(ddl_statement_translator_test, create_index_column_direction) {
    auto s = ir.CreateIndexStatement(
            {},
            ir.Name("T0"),
            {
                    ir.CreateIndexStatementColumn(
                            ir.Name("C2"),
                            ddl::CreateIndexStatement::Column::Direction::ASCENDANT),
                    ir.CreateIndexStatementColumn(
                            ir.Name("C1"),
                            ddl::CreateIndexStatement::Column::Direction::DESCENDANT),
            },
            {});
    auto r = engine.process(*s);
    auto&& stmt = downcast<statement::create_index>(r.element<result_kind::statement>());

    auto index = extract_shared<::yugawara::storage::index>(stmt.definition());
    {
        auto&& cols = index->keys();
        ASSERT_EQ(cols.size(), 2);

        EXPECT_EQ(cols[0].column(), t0->columns()[2]);
        EXPECT_EQ(cols[0].direction(), ::yugawara::storage::sort_direction::ascendant);

        EXPECT_EQ(cols[1].column(), t0->columns()[1]);
        EXPECT_EQ(cols[1].direction(), ::yugawara::storage::sort_direction::descendant);
    }
}

TEST_F(ddl_statement_translator_test, create_index_conflict) {
    auto s = ir.CreateIndexStatement(
            ir.Name(i0s->simple_name()),
            ir.Name("T1"),
            {
                    ir.CreateIndexStatementColumn(
                            ir.Name("C1"),
                            ddl::CreateIndexStatement::Column::Direction::DONT_CARE),
            },
            {});
    auto r = engine.process(*s);
    ASSERT_FALSE(r);
}

TEST_F(ddl_statement_translator_test, create_index_if_not_exists) {
    auto s = ir.CreateIndexStatement(
            ir.Name(i0s->simple_name()),
            ir.Name("T1"),
            {
                    ir.CreateIndexStatementColumn(
                            ir.Name("C1"),
                            ddl::CreateIndexStatement::Column::Direction::DONT_CARE),
            },
            { ddl::CreateIndexStatement::Attribute::IF_NOT_EXISTS });
    auto r = engine.process(*s);
    ASSERT_TRUE(r);
    EXPECT_EQ(r.element<result_kind::statement>().kind(), statement::empty::tag);
}

TEST_F(ddl_statement_translator_test, create_index_missing_column) {
    auto s = ir.CreateIndexStatement(
            ir.Name(i0s->simple_name()),
            ir.Name("T1"),
            {
                    ir.CreateIndexStatementColumn(
                            ir.Name("C_MISSING"),
                            ddl::CreateIndexStatement::Column::Direction::DONT_CARE),
            },
            {});
    auto r = engine.process(*s);
    ASSERT_FALSE(r);
}

TEST_F(ddl_statement_translator_test, create_index_conflict_column) {
    auto s = ir.CreateIndexStatement(
            ir.Name(i0s->simple_name()),
            ir.Name("T1"),
            {
                    ir.CreateIndexStatementColumn(
                            ir.Name("C1"),
                            ddl::CreateIndexStatement::Column::Direction::ASCENDANT),
                    ir.CreateIndexStatementColumn(
                            ir.Name("C2"),
                            ddl::CreateIndexStatement::Column::Direction::DESCENDANT),
            },
            {});
    auto r = engine.process(*s);
    ASSERT_FALSE(r);
}


TEST_F(ddl_statement_translator_test, drop_table) {
    auto s = ir.DropTableStatement(
            ir.Name(t0->simple_name()),
            {});
    auto r = engine.process(*s);
    auto&& stmt = downcast<statement::drop_table>(r.element<result_kind::statement>());

    auto schema = extract_shared(stmt.schema());
    EXPECT_EQ(schema.get(), options.shared_schema().get());

    auto table = extract_shared(stmt.target());
    EXPECT_EQ(table.get(), t0.get());
}

TEST_F(ddl_statement_translator_test, drop_table_missing) {
    auto s = ir.DropTableStatement(
            ir.Name("MISSING"),
            {});
    auto r = engine.process(*s);
    ASSERT_FALSE(r);
}

TEST_F(ddl_statement_translator_test, drop_table_if_exists) {
    auto s = ir.DropTableStatement(
            ir.Name("MISSING"),
            { ddl::DropTableStatement::Attribute::IF_EXISTS });
    auto r = engine.process(*s);
    ASSERT_TRUE(r);
    EXPECT_EQ(r.element<result_kind::statement>().kind(), statement::empty::tag);
}

TEST_F(ddl_statement_translator_test, drop_index) {
    auto s = ir.DropIndexStatement(
            ir.Name(i0s->simple_name()),
            {});
    auto r = engine.process(*s);
    auto&& stmt = downcast<statement::drop_index>(r.element<result_kind::statement>());

    auto schema = extract_shared(stmt.schema());
    EXPECT_EQ(schema.get(), options.shared_schema().get());

    auto index = extract_shared<::yugawara::storage::index>(stmt.target());
    EXPECT_EQ(index.get(), i0s.get());
}

TEST_F(ddl_statement_translator_test, drop_index_missing) {
    auto s = ir.DropIndexStatement(
            ir.Name("MISSING"),
            {});
    auto r = engine.process(*s);
    ASSERT_FALSE(r);
}

TEST_F(ddl_statement_translator_test, drop_index_if_exists) {
    auto s = ir.DropIndexStatement(
            ir.Name("MISSING"),
            { ddl::DropIndexStatement::Attribute::IF_EXISTS });
    auto r = engine.process(*s);
    ASSERT_TRUE(r);
    EXPECT_EQ(r.element<result_kind::statement>().kind(), statement::empty::tag);
}

} // namespace mizugaki::translator::details
