#include <mizugaki/analyzer/details/set_function_processor.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/scalar/unary.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/values.h>

#include <takatori/util/clonable.h>

#include <yugawara/binding/extract.h>
#include <yugawara/binding/factory.h>

#include <yugawara/extension/scalar/aggregate_function_call.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::takatori::util::clone_unique;
using ::takatori::util::ownership_reference;

using output_port_type = ::takatori::relation::expression::output_port_type;

using ::yugawara::binding::extract;
using ::yugawara::extension::scalar::aggregate_function_call;

class set_function_processor_test : public test_parent {
public:


protected:
    ::takatori::relation::graph_type graph_;
    ::yugawara::binding::factory factory_;

    std::shared_ptr<::yugawara::aggregate::declaration> count_asterisk = set_functions_->add(
            ::yugawara::aggregate::declaration {
                    ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
                    "count",
                    ttype::int8 {},
                    {},
                    true,
            });

    std::shared_ptr<::yugawara::aggregate::declaration> count_value = set_functions_->add(
            ::yugawara::aggregate::declaration {
                    ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
                    "count",
                    ttype::int8 {},
                    {
                            ttype::int8 {},
                    },
                    true,
            });
};

TEST_F(set_function_processor_test, simple) {
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());
    std::unique_ptr<tscalar::expression> expr = clone_unique(aggregate_function_call {
            factory_(count_asterisk),
    });

    bool result = processor.process(ownership_reference { expr });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    EXPECT_FALSE(output.opposite());

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(values);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 0);
    ASSERT_EQ(aggregate->columns().size(), 1);
    {
        auto&& column = aggregate->columns()[0];
        EXPECT_EQ(&extract(column.function()), count_asterisk.get());
        EXPECT_EQ(*expr, vref(column.destination()));
        ASSERT_EQ(column.arguments().size(), 0);
    }

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, function_arguments) {
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());
    std::unique_ptr<tscalar::expression> expr = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    immediate(1),
            }
    });

    bool result = processor.process(ownership_reference { expr });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    EXPECT_FALSE(output.opposite());

    auto&& arguments = find_next<trelation::project>(values);
    ASSERT_TRUE(arguments);
    ASSERT_EQ(arguments->columns().size(), 1);
    EXPECT_EQ(arguments->columns()[0].value(), immediate(1));

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*arguments);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 0);
    ASSERT_EQ(aggregate->columns().size(), 1);
    {
        auto&& column = aggregate->columns()[0];
        EXPECT_EQ(&extract(column.function()), count_value.get());
        EXPECT_EQ(*expr, vref(column.destination()));
        ASSERT_EQ(column.arguments().size(), 1);
        EXPECT_EQ(column.arguments()[0], arguments->columns()[0].variable());
    }

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, grouping) {
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());

    auto va = factory_.stream_variable("a");

    processor.add_group_key(va);
    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    EXPECT_FALSE(output.opposite());

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(values);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 1);
    EXPECT_EQ(aggregate->group_keys()[0], va);

    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, grouping_function) {
    set_function_processor processor { context(), graph_ };


    auto vk = factory_.stream_variable("k");
    auto vv = factory_.stream_variable("v");

    std::unique_ptr<tscalar::expression> expr = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    vref(vv),
            }
    });

    processor.add_group_key(vk);
    bool result = processor.process(ownership_reference { expr });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    EXPECT_FALSE(output.opposite());

    auto&& arguments = find_next<trelation::project>(values);
    ASSERT_TRUE(arguments);
    ASSERT_EQ(arguments->columns().size(), 1);
    EXPECT_EQ(arguments->columns()[0].value(), vref(vv));

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*arguments);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 1);
    EXPECT_EQ(aggregate->group_keys()[0], vk);

    ASSERT_EQ(aggregate->columns().size(), 1);
    {
        auto&& column = aggregate->columns()[0];
        EXPECT_EQ(&extract(column.function()), count_value.get());
        EXPECT_EQ(*expr, vref(column.destination()));
        ASSERT_EQ(column.arguments().size(), 1);
        EXPECT_EQ(column.arguments()[0], arguments->columns()[0].variable());
    }

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, top_grouping) {
    // SELECT g GROUP BY g
    set_function_processor processor { context(), graph_ };

    auto vg = factory_.stream_variable("vg");
    processor.add_group_key(vg);

    std::unique_ptr<tscalar::expression> expr = clone_unique(vref(vg));
    bool result = processor.process(ownership_reference { expr });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    EXPECT_FALSE(output.opposite());

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(values);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, top_aggregated) {
    // SELECT g GROUP BY g
    set_function_processor processor { context(), graph_ };

    auto vg = factory_.stream_variable("vg");
    processor.add_group_key(vg);

    auto va = factory_.stream_variable("va");
    processor.add_aggregated(va);

    std::unique_ptr<tscalar::expression> expr = clone_unique(vref(va));
    bool result = processor.process(ownership_reference { expr });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    EXPECT_FALSE(output.opposite());

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(values);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, top_plain_invalid) {
    // SELECT g GROUP BY g
    set_function_processor processor { context(), graph_ };

    auto vg = factory_.stream_variable("p");
    processor.add_group_key(vg);

    auto vp = factory_.stream_variable("p");

    std::unique_ptr<tscalar::expression> expr = clone_unique(vref(vp));
    bool result = processor.process(ownership_reference { expr });
    EXPECT_FALSE(result);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_aggregation_column));
}

TEST_F(set_function_processor_test, inner_plain) {
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());

    auto vp = factory_.stream_variable("p");

    std::unique_ptr<tscalar::expression> expr = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    vref(vp),
            }
    });

    bool result = processor.process(ownership_reference { expr });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    EXPECT_FALSE(output.opposite());

    auto&& arguments = find_next<trelation::project>(values);
    ASSERT_TRUE(arguments);
    ASSERT_EQ(arguments->columns().size(), 1);
    EXPECT_EQ(arguments->columns()[0].value(), vref(vp));

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*arguments);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 0);
    ASSERT_EQ(aggregate->columns().size(), 1);
}

TEST_F(set_function_processor_test, inner_grouping) {
    set_function_processor processor { context(), graph_ };


    auto vg = factory_.stream_variable("vg");

    std::unique_ptr<tscalar::expression> expr = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    vref(vg),
            }
    });

    processor.add_group_key(vg);
    bool result = processor.process(ownership_reference { expr });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    EXPECT_FALSE(output.opposite());

    auto&& arguments = find_next<trelation::project>(values);
    ASSERT_TRUE(arguments);
    ASSERT_EQ(arguments->columns().size(), 1);
    EXPECT_EQ(arguments->columns()[0].value(), vref(vg));

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*arguments);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 1);
    ASSERT_EQ(aggregate->columns().size(), 1);
}

TEST_F(set_function_processor_test, inner_aggregated_invalid) {
    set_function_processor processor { context(), graph_ };


    auto vg = factory_.stream_variable("vg");
    auto va = factory_.stream_variable("va");

    std::unique_ptr<tscalar::expression> expr = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    vref(va),
            }
    });

    processor.add_group_key(vg);
    processor.add_aggregated(va);
    bool result = processor.process(ownership_reference { expr });
    EXPECT_FALSE(result);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_aggregation_column));
}

TEST_F(set_function_processor_test, column_unary) {
    set_function_processor processor { context(), graph_ };

    std::unique_ptr<tscalar::expression> expr = clone_unique(tscalar::unary {
            tscalar::unary_operator::sign_inversion,
            aggregate_function_call {
                    factory_(count_value),
                    {
                            immediate(1),
                    }
            },
    });

    EXPECT_FALSE(processor.active());

    bool result = processor.process(ownership_reference { expr });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    EXPECT_FALSE(output.opposite());

    auto&& arguments = find_next<trelation::project>(values);
    ASSERT_TRUE(arguments);
    ASSERT_EQ(arguments->columns().size(), 1);
    EXPECT_EQ(arguments->columns()[0].value(), immediate(1));

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*arguments);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 0);
    ASSERT_EQ(aggregate->columns().size(), 1);
    {
        auto&& column = aggregate->columns()[0];
        EXPECT_EQ(&extract(column.function()), count_value.get());
        EXPECT_EQ(*expr, (tscalar::unary {
                tscalar::unary_operator::sign_inversion,
                vref(column.destination()),
        }));
        ASSERT_EQ(column.arguments().size(), 1);
        EXPECT_EQ(column.arguments()[0], arguments->columns()[0].variable());
    }

    EXPECT_FALSE(aggregate->output().opposite());
}

} // namespace mizugaki::analyzer::details
