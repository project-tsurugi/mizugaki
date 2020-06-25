#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>

#include <takatori/scalar/immediate.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/values.h>
#include <takatori/relation/write.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/create_table.h>
#include <takatori/statement/create_index.h>
#include <takatori/statement/drop_table.h>
#include <takatori/statement/drop_index.h>
#include <takatori/statement/write.h>

#include <yugawara/binding/factory.h>
#include <yugawara/binding/extract.h>

#include <yugawara/schema/declaration.h>
#include <yugawara/schema/configurable_provider.h>
#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>
#include <yugawara/storage/column.h>
#include <yugawara/storage/configurable_provider.h>
#include <yugawara/variable/configurable_provider.h>

#include <mizugaki/ast/type/simple.h>

#include <mizugaki/ast/query/table_value_constructor.h>

#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/statement/empty_statement.h>
#include <mizugaki/ast/statement/insert_statement.h>
#include <mizugaki/ast/statement/table_definition.h>
#include <mizugaki/ast/statement/column_definition.h>
#include <mizugaki/ast/statement/index_definition.h>
#include <mizugaki/ast/statement/drop_statement.h>
#include <mizugaki/ast/statement/table_constraint_definition.h>
#include <mizugaki/ast/statement/simple_constraint.h>
#include <mizugaki/ast/statement/key_constraint.h>
#include <mizugaki/ast/statement/expression_constraint.h>

#include "../utils.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;
using ::yugawara::binding::extract_if;

class analyze_statement_test : public ::testing::Test {
public:
    analyzer_context& context() {
        if (!context_finalizer_) {
            context_finalizer_ = context_.initialize(options_);
        }
        return context_;
    }

    std::string diagnostics() {
        ::takatori::util::string_builder s {};
        for (auto&& d : context_.diagnostics()) {
            s << d << "\n";
        }
        return s << ::takatori::util::string_builder::to_string;
    }

    std::shared_ptr<::yugawara::storage::table const> install_table(std::string_view name) {
        auto table = storage_->add_table(::yugawara::storage::table {
                name,
                {
                        {
                                "k",
                                ttype::int4 {},
                        },
                        {
                                "v",
                                ttype::character { ttype::varying },
                                ::yugawara::variable::nullable,
                                tvalue::character { "V" },
                        },
                        {
                                "w",
                                ttype::character { ttype::varying },
                                ::yugawara::variable::nullable,
                                tvalue::character { "W" },
                        },
                        {
                                "x",
                                ttype::character { ttype::varying },
                                ::yugawara::variable::nullable,
                                tvalue::character { "X" },
                        },
                },
        });
        storage_->add_index(::yugawara::storage::index {
                table,
                "",
                {
                        {
                                table->columns()[0],
                                ::yugawara::storage::index::key::direction_type::ascendant,
                        },
                },
                {},
                {
                        ::yugawara::storage::index_feature::find,
                        ::yugawara::storage::index_feature::scan,
                        ::yugawara::storage::index_feature::primary,
                        ::yugawara::storage::index_feature::unique,
                },
        });
        return table;
    }

    ::takatori::descriptor::variable vd(std::string_view name = "id") {
        return ::yugawara::binding::factory {}.stream_variable(name);
    }

protected:
    std::shared_ptr<::yugawara::storage::configurable_provider> storage_ = std::make_shared<::yugawara::storage::configurable_provider>();
    std::shared_ptr<::yugawara::schema::declaration> schema_ = std::make_shared<::yugawara::schema::declaration>(
            "public",
            std::nullopt,
            storage_);
    std::shared_ptr<::yugawara::schema::catalog> catalog_ = std::make_shared<::yugawara::schema::catalog>(
            "tsurugi",
            std::nullopt,
            build_shared<::yugawara::schema::configurable_provider>([&](auto& v) { v.add(schema_); }));
    std::shared_ptr<::yugawara::schema::search_path> search_path_ = std::make_shared<::yugawara::schema::search_path>(
            ::yugawara::schema::search_path::vector_type { schema_ });

    sql_analyzer_options options_ {
        catalog_,
        search_path_,
        schema_,
    };

private:
    analyzer_context context_;
    std::optional<::takatori::util::finalizer> context_finalizer_;
};

TEST_F(analyze_statement_test, empty_statement) {
    auto r = analyze_statement(context(), ast::statement::empty_statement {});
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    auto&& stmt = *alternative;
    ASSERT_EQ(stmt->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_test, insert_simple) {
    options_.prefer_write_statement() = false;
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::insert_statement {
            id("testing"),
            {},
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(string("'a'")),
                            literal(string("'b'")),
                            literal(string("'c'")),
                    }
            },
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

    auto first = find_first<trelation::values>(graph);
    ASSERT_TRUE(first);
    ASSERT_EQ(first->columns().size(), 4);
    ASSERT_EQ(first->rows().size(), 1);
    ASSERT_EQ(first->rows()[0].elements().size(), 4);
    {
        auto&& row = first->rows()[0].elements();
        EXPECT_EQ(row[0], immediate(1));
        EXPECT_EQ(row[1], immediate("a"));
        EXPECT_EQ(row[2], immediate("b"));
        EXPECT_EQ(row[3], immediate("c"));
    }

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*first).get(), last.get());

    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::insert);
    EXPECT_EQ(&extract<::yugawara::storage::index>(last->destination()).table(), table.get());

    ASSERT_EQ(last->keys().size(), 1);
    {
        auto&& column = last->keys()[0];
        EXPECT_EQ(column.source(), first->columns()[0]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[0]);
    }

    ASSERT_EQ(last->columns().size(), 3);
    {
        auto&& column = last->columns()[0];
        EXPECT_EQ(column.source(), first->columns()[1]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[1]);
    }
    {
        auto&& column = last->columns()[1];
        EXPECT_EQ(column.source(), first->columns()[2]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[2]);
    }
    {
        auto&& column = last->columns()[2];
        EXPECT_EQ(column.source(), first->columns()[3]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[3]);
    }
}

TEST_F(analyze_statement_test, insert_partial) {
    options_.prefer_write_statement() = false;
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::insert_statement {
            id("testing"),
                    {
                            id("v"),
                            id("k"),
                    },
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(string("'a'")),
                            literal(number("1")),
                    }
            },
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

    auto first = find_first<trelation::values>(graph);
    ASSERT_TRUE(first);
    ASSERT_EQ(first->columns().size(), 2);
    ASSERT_EQ(first->rows().size(), 1);
    ASSERT_EQ(first->rows()[0].elements().size(), 2);
    {
        auto&& row = first->rows()[0].elements();
        EXPECT_EQ(row[0], immediate("a"));
        EXPECT_EQ(row[1], immediate(1));
    }

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*first).get(), last.get());

    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::insert);
    EXPECT_EQ(&extract<::yugawara::storage::index>(last->destination()).table(), table.get());

    ASSERT_EQ(last->keys().size(), 1);
    {
        auto&& column = last->keys()[0];
        EXPECT_EQ(column.source(), first->columns()[1]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[0]);
    }

    ASSERT_EQ(last->columns().size(), 1);
    {
        auto&& column = last->columns()[0];
        EXPECT_EQ(column.source(), first->columns()[0]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[1]);
    }
}

TEST_F(analyze_statement_test, insert_statement) {
    options_.prefer_write_statement() = true;
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::insert_statement {
            id("testing"),
            {},
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(string("'a'")),
                            literal(string("'b'")),
                            literal(string("'c'")),
                    }
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::write);
    auto&& stmt = downcast<tstatement::write>(**alternative);

    EXPECT_EQ(stmt.operator_kind(), tstatement::write_kind::insert);
    EXPECT_EQ(&extract<::yugawara::storage::index>(stmt.destination()).table(), table.get());

    auto&& columns = stmt.columns();
    ASSERT_EQ(columns.size(), 4);
    {
        EXPECT_EQ(&extract<::yugawara::storage::column>(columns[0]), &table->columns()[0]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(columns[1]), &table->columns()[1]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(columns[2]), &table->columns()[2]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(columns[3]), &table->columns()[3]);
    }

    auto&& tuples = stmt.tuples();
    ASSERT_EQ(tuples.size(), 1);
    {
        auto&& row = tuples[0];
        EXPECT_EQ(row.elements()[0], immediate(1));
        EXPECT_EQ(row.elements()[1], immediate("a"));
        EXPECT_EQ(row.elements()[2], immediate("b"));
        EXPECT_EQ(row.elements()[3], immediate("c"));
    }
}

TEST_F(analyze_statement_test, table_definition_simple) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), schema_.get());

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value { tvalue::unknown {} });
    }

    auto&& key = extract_if<::yugawara::storage::index>(stmt.primary_key());
    ASSERT_TRUE(key);
    EXPECT_EQ(key->simple_name(), "");
    ASSERT_EQ(key->keys().size(), 0);
    EXPECT_EQ(key->features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

TEST_F(analyze_statement_test, table_definition_column_multiple) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c0"),
                            ast::type::simple { ast::type::kind::small_integer },
                    },
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::column_definition {
                            id("c2"),
                            ast::type::simple { ast::type::kind::big_integer },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), schema_.get());

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 3);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c0");
        EXPECT_EQ(column.type(), ttype::int2 {});
    }
    {
        auto&& column = table_columns[1];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
    }
    {
        auto&& column = table_columns[2];
        EXPECT_EQ(column.simple_name(), "c2");
        EXPECT_EQ(column.type(), ttype::int8 {});
    }
}

TEST_F(analyze_statement_test, table_definition_column_primary_key) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::key_constraint {
                                            ast::statement::constraint_kind::primary_key,
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), schema_.get());

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& key = extract_if<::yugawara::storage::index>(stmt.primary_key());
    ASSERT_TRUE(key);
    EXPECT_EQ(key->simple_name(), "");
    auto&& key_columns = key->keys();
    ASSERT_EQ(key_columns.size(), 1);
    {
        auto&& column = key_columns[0];
        EXPECT_EQ(&column.column(), &table_columns[0]);
        EXPECT_EQ(column.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    EXPECT_EQ(key->features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

TEST_F(analyze_statement_test, table_definition_column_null) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::null,
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), schema_.get());

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value { tvalue::unknown {} });
    }
}

TEST_F(analyze_statement_test, table_definition_column_not_null) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::simple_constraint {
                                            ast::statement::constraint_kind::not_null,
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), schema_.get());

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ~::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value {});
    }
}

TEST_F(analyze_statement_test, table_definition_column_default) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                            {
                                    ast::statement::expression_constraint {
                                            ast::statement::constraint_kind::default_clause,
                                            literal(number("1")),
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), schema_.get());

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    {
        auto&& column = table_columns[0];
        EXPECT_EQ(column.simple_name(), "c1");
        EXPECT_EQ(column.type(), ttype::int4 {});
        EXPECT_EQ(column.criteria().nullity(), ::yugawara::variable::nullable);
        EXPECT_EQ(column.default_value(), ::yugawara::storage::column_value { tvalue::int4 { 1 } });
    }
}

TEST_F(analyze_statement_test, table_definition_table_primary_key) {
    auto r = analyze_statement(context(), ast::statement::table_definition {
            id("testing"),
            {
                    ast::statement::column_definition {
                            id("c1"),
                            ast::type::simple { ast::type::kind::integer },
                    },
                    ast::statement::table_constraint_definition {
                            ast::statement::key_constraint {
                                    ast::statement::constraint_kind::primary_key,
                                    {
                                            {
                                                    id("c1"),
                                            }
                                    },
                            },
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_table);

    auto&& stmt = downcast<tstatement::create_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), schema_.get());

    auto&& table = extract(stmt.definition());
    EXPECT_EQ(table.simple_name(), "testing");

    auto&& table_columns = table.columns();
    ASSERT_EQ(table_columns.size(), 1);
    auto&& key = extract_if<::yugawara::storage::index>(stmt.primary_key());
    ASSERT_TRUE(key);
    EXPECT_EQ(key->simple_name(), "");
    auto&& key_columns = key->keys();
    ASSERT_EQ(key_columns.size(), 1);
    {
        auto&& column = key_columns[0];
        EXPECT_EQ(&column.column(), &table_columns[0]);
        EXPECT_EQ(column.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    EXPECT_EQ(key->features(), (::yugawara::storage::index_feature_set {
            ::yugawara::storage::index_feature::primary,
    }));
}

TEST_F(analyze_statement_test, index_definition_simple) {
    auto table = install_table("testing_table");
    auto r = analyze_statement(context(), ast::statement::index_definition {
            id("testing"),
            id("testing_table"),
            {
                    {
                            id("v"),
                    }
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_index);

    auto&& stmt = downcast<tstatement::create_index>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), schema_.get());

    auto&& index = extract<::yugawara::storage::index>(stmt.definition());
    EXPECT_EQ(index.simple_name(), "testing");

    auto&& keys = index.keys();
    ASSERT_EQ(keys.size(), 1);
    {
        auto&& key = keys[0];
        EXPECT_EQ(&key.column(), &table->columns()[1]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }

    ASSERT_EQ(index.values().size(), 0);
    ASSERT_EQ(index.features(), ::yugawara::storage::index::feature_set_type {});
}

TEST_F(analyze_statement_test, index_definition_multiple) {
    auto table = install_table("testing_table");
    auto r = analyze_statement(context(), ast::statement::index_definition {
            id("testing"),
            id("testing_table"),
            {
                    {
                            id("v"),
                    },
                    {
                        id("w"),
                    },
                    {
                        id("x"),
                    }
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_index);

    auto&& stmt = downcast<tstatement::create_index>(**alternative);

    auto&& index = extract<::yugawara::storage::index>(stmt.definition());
    EXPECT_EQ(index.simple_name(), "testing");

    auto&& keys = index.keys();
    ASSERT_EQ(keys.size(), 3);
    {
        auto&& key = keys[0];
        EXPECT_EQ(&key.column(), &table->columns()[1]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    {
        auto&& key = keys[1];
        EXPECT_EQ(&key.column(), &table->columns()[2]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    {
        auto&& key = keys[2];
        EXPECT_EQ(&key.column(), &table->columns()[3]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
}

TEST_F(analyze_statement_test, index_definition_direction) {
    auto table = install_table("testing_table");
    auto r = analyze_statement(context(), ast::statement::index_definition {
            id("testing"),
            id("testing_table"),
            {
                    {
                            id("v"),
                            ast::common::ordering_specification::asc,
                    },
                    {
                            id("w"),
                            ast::common::ordering_specification::desc,
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_index);

    auto&& stmt = downcast<tstatement::create_index>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), schema_.get());

    auto&& index = extract<::yugawara::storage::index>(stmt.definition());
    EXPECT_EQ(index.simple_name(), "testing");

    auto&& keys = index.keys();
    ASSERT_EQ(keys.size(), 2);
    {
        auto&& key = keys[0];
        EXPECT_EQ(&key.column(), &table->columns()[1]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    {
        auto&& key = keys[1];
        EXPECT_EQ(&key.column(), &table->columns()[2]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::descendant);
    }
}

TEST_F(analyze_statement_test, drop_table_simple) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("testing"),
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_table);

    auto&& stmt = downcast<tstatement::drop_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), schema_.get());

    auto&& target = extract(stmt.target());
    EXPECT_EQ(&target, table.get());
}

TEST_F(analyze_statement_test, drop_index_simple) {
    auto table = install_table("testing");
    auto index = storage_->add_index(::yugawara::storage::index {
            table,
            "testing_index",
            {
                    table->columns()[1],
            }
    });

    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("testing_index"),
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_index);

    auto&& stmt = downcast<tstatement::drop_index>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), schema_.get());

    auto&& target = extract<::yugawara::storage::index>(stmt.target());
    EXPECT_EQ(&target, index.get());
}

} // namespace mizugaki::analyzer::details
