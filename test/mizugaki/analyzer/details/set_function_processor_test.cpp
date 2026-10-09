#include <mizugaki/analyzer/details/set_function_processor.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/scalar/unary.h>
#include <takatori/scalar/binary.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/values.h>

#include <takatori/util/clonable.h>

#include <yugawara/binding/extract.h>
#include <yugawara/binding/factory.h>

#include <yugawara/extension/scalar/aggregate_function_call.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

namespace tdescriptor = ::takatori::descriptor;

using ::takatori::util::clone_unique;
using ::takatori::util::ownership_reference;

using output_port_type = ::takatori::relation::expression::output_port_type;

using ::yugawara::binding::extract;
using ::yugawara::extension::scalar::aggregate_function_call;

class set_function_processor_test : public test_parent {
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

    std::shared_ptr<::yugawara::aggregate::declaration> sum_value = set_functions_->add(
            ::yugawara::aggregate::declaration {
                    ::yugawara::aggregate::declaration::minimum_builtin_function_id + 3,
                    "sum",
                    ttype::int8 {},
                    {
                            ttype::int8 {},
                    },
                    true,
            });
};

TEST_F(set_function_processor_test, simple) {
    // SELECT COUNT(*)
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
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

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
    // SELECT COUNT(1)
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
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

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

TEST_F(set_function_processor_test, function_arguments_twice) {
    // SELECT COUNT(1), COUNT(2)
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());
    std::unique_ptr<tscalar::expression> e0 = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    immediate(1),
            }
    });
    bool r0 = processor.process(ownership_reference { e0 });
    ASSERT_TRUE(r0);

    std::unique_ptr<tscalar::expression> e1 = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    immediate(2),
            }
    });
    bool r1 = processor.process(ownership_reference { e1 });
    ASSERT_TRUE(r1);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& arguments = find_next<trelation::project>(values);
    ASSERT_TRUE(arguments);
    ASSERT_EQ(arguments->columns().size(), 2);
    EXPECT_EQ(arguments->columns()[0].value(), immediate(1));
    EXPECT_EQ(arguments->columns()[1].value(), immediate(2));

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*arguments);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 0);
    ASSERT_EQ(aggregate->columns().size(), 2);
    {
        auto&& column = aggregate->columns()[0];
        EXPECT_EQ(&extract(column.function()), count_value.get());
        EXPECT_EQ(*e0, vref(column.destination()));
        ASSERT_EQ(column.arguments().size(), 1);
        EXPECT_EQ(column.arguments()[0], arguments->columns()[0].variable());
    }
    {
        auto&& column = aggregate->columns()[1];
        EXPECT_EQ(&extract(column.function()), count_value.get());
        EXPECT_EQ(*e1, vref(column.destination()));
        ASSERT_EQ(column.arguments().size(), 1);
        EXPECT_EQ(column.arguments()[0], arguments->columns()[1].variable());
    }

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, function_arguments_match) {
    // SELECT COUNT(1), SUM(1)
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());
    std::unique_ptr<tscalar::expression> e0 = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    immediate(1),
            }
    });
    bool r0 = processor.process(ownership_reference { e0 });
    ASSERT_TRUE(r0);

    std::unique_ptr<tscalar::expression> e1 = clone_unique(aggregate_function_call {
            factory_(sum_value),
            {
                    immediate(1),
            }
    });
    bool r1 = processor.process(ownership_reference { e1 });
    ASSERT_TRUE(r1);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& arguments = find_next<trelation::project>(values);
    ASSERT_TRUE(arguments);
    ASSERT_EQ(arguments->columns().size(), 1);
    EXPECT_EQ(arguments->columns()[0].value(), immediate(1));

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*arguments);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 0);
    ASSERT_EQ(aggregate->columns().size(), 2);
    {
        auto&& column = aggregate->columns()[0];
        EXPECT_EQ(&extract(column.function()), count_value.get());
        EXPECT_EQ(*e0, vref(column.destination()));
        ASSERT_EQ(column.arguments().size(), 1);
        EXPECT_EQ(column.arguments()[0], arguments->columns()[0].variable());
    }
    {
        auto&& column = aggregate->columns()[1];
        EXPECT_EQ(&extract(column.function()), sum_value.get());
        EXPECT_EQ(*e1, vref(column.destination()));
        ASSERT_EQ(column.arguments().size(), 1);
        EXPECT_EQ(column.arguments()[0], arguments->columns()[0].variable());
    }

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, function_duplicate) {
    // SELECT COUNT(1), COUNT(1)
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());
    std::unique_ptr<tscalar::expression> e0 = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    immediate(1),
            }
    });
    bool r0 = processor.process(ownership_reference { e0 });
    ASSERT_TRUE(r0);

    std::unique_ptr<tscalar::expression> e1 = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    immediate(1),
            }
    });
    bool r1 = processor.process(ownership_reference { e1 });
    ASSERT_TRUE(r1);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

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
        EXPECT_EQ(*e0, vref(column.destination()));
        EXPECT_EQ(*e1, vref(column.destination()));
        ASSERT_EQ(column.arguments().size(), 1);
        EXPECT_EQ(column.arguments()[0], arguments->columns()[0].variable());
    }

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, add_group_key_variable) {
    // GROUP BY g
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());

    auto vg = vd("g", ttype::int8 {});

    processor.add_group_key(vg);
    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(values);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 1);
    EXPECT_EQ(aggregate->group_keys()[0], vg);

    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, add_group_key_expression) {
    // GROUP BY g + 1
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());

    auto vg = vd("g", ttype::int8 {});
    tscalar::binary expr {
            tscalar::binary_operator::add,
            vref(vg),
            immediate(1),
    };

    processor.add_group_key(clone_unique(expr));
    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& project = find_next<trelation::project>(values);
    ASSERT_TRUE(project);
    ASSERT_EQ(project->columns().size(), 1);
    EXPECT_EQ(project->columns()[0].value(), expr);
    auto&& group_key = project->columns()[0].variable();

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*project);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 1);
    EXPECT_EQ(aggregate->group_keys()[0], group_key);

    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, add_group_key_variable_reference) {
    // GROUP BY g
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());

    auto vg = vd("g", ttype::int8 {});

    tscalar::variable_reference expr { vg };
    processor.add_group_key(clone_unique(expr));
    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(values);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 1);
    EXPECT_EQ(aggregate->group_keys()[0], vg);

    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, add_group_key_mix) {
    // GROUP BY g, g + 1
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());

    auto vg = vd("g", ttype::int8 {});
    processor.add_group_key(vg);
    EXPECT_TRUE(processor.active());

    tscalar::binary expr {
            tscalar::binary_operator::add,
            vref(vg),
            immediate(1),
    };
    processor.add_group_key(clone_unique(expr));
    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& project = find_next<trelation::project>(values);
    ASSERT_TRUE(project);
    ASSERT_EQ(project->columns().size(), 1);
    EXPECT_EQ(project->columns()[0].value(), expr);
    auto&& group_key = project->columns()[0].variable();

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*project);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 2);
    EXPECT_EQ(aggregate->group_keys()[0], vg);
    EXPECT_EQ(aggregate->group_keys()[1], group_key);

    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, process_function) {
    // SELECT COUNT(v) GROUP BY g
    set_function_processor processor { context(), graph_ };

    auto vg = vd("g", ttype::int8 {});
    auto vv = vd("v", ttype::int8 {});

    std::unique_ptr<tscalar::expression> expr = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    vref(vv),
            }
    });

    processor.add_group_key(vg);
    bool result = processor.process(ownership_reference { expr });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& arguments = find_next<trelation::project>(values);
    ASSERT_TRUE(arguments);
    ASSERT_EQ(arguments->columns().size(), 1);
    EXPECT_EQ(arguments->columns()[0].value(), vref(vv));

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*arguments);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 1);
    EXPECT_EQ(aggregate->group_keys()[0], vg);

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

TEST_F(set_function_processor_test, process_refer_group_key_variable_top) {
    // SELECT g GROUP BY g
    set_function_processor processor { context(), graph_ };

    auto vg = vd("g", ttype::int8 {});
    processor.add_group_key(vg);

    std::unique_ptr<tscalar::expression> element = clone_unique(vref(vg));
    bool result = processor.process(ownership_reference { element });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(values);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 1);
    EXPECT_EQ(aggregate->group_keys()[0], vg);
    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_EQ(*element, vref(aggregate->group_keys()[0]));

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, process_refer_group_key_expression_top) {
    // SELECT g + 1 GROUP BY g + 1
    set_function_processor processor { context(), graph_ };

    auto vg = vd("g", ttype::int8 {});
    tscalar::binary expr {
            tscalar::binary_operator::add,
            vref(vg),
            immediate(1),
    };
    processor.add_group_key(clone_unique(expr));

    std::unique_ptr<tscalar::expression> element = clone_unique(expr);
    bool result = processor.process(ownership_reference { element });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& project = find_next<trelation::project>(values);
    ASSERT_TRUE(project);
    ASSERT_EQ(project->columns().size(), 1);
    EXPECT_EQ(project->columns()[0].value(), expr);
    auto&& group_key = project->columns()[0].variable();

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*project);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 1);
    EXPECT_EQ(aggregate->group_keys()[0], group_key);
    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_EQ(*element, vref(aggregate->group_keys()[0]));

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, process_refer_group_key_expression_inner) {
    // SELECT g + 1 + 2 GROUP BY g + 1
    set_function_processor processor { context(), graph_ };

    auto vg = vd("g", ttype::int8 {});
    tscalar::binary expr {
            tscalar::binary_operator::add,
            vref(vg),
            immediate(1),
    };
    processor.add_group_key(clone_unique(expr));

    std::unique_ptr<tscalar::expression> element = clone_unique(tscalar::binary {
            tscalar::binary_operator::add,
            tscalar::binary {
                    tscalar::binary_operator::add,
                    vref(vg),
                    immediate(1),
            },
            immediate(2),
    });
    bool result = processor.process(ownership_reference { element });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& project = find_next<trelation::project>(values);
    ASSERT_TRUE(project);
    ASSERT_EQ(project->columns().size(), 1);
    EXPECT_EQ(project->columns()[0].value(), expr);
    auto&& group_key = project->columns()[0].variable();

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*project);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 1);
    EXPECT_EQ(aggregate->group_keys()[0], group_key);
    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_EQ(*element, (tscalar::binary {
            tscalar::binary_operator::add,
            vref(aggregate->group_keys()[0]),
            immediate(2),
    }));

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, process_refer_group_key_match_long) {
    // SELECT g + 1 + 2 + 3 GROUP BY g, g + 1, g + 1 + 2
    set_function_processor processor { context(), graph_ };

    auto vg = vd("g", ttype::int8 {});

    auto eg0 = vref(vg);
    processor.add_group_key(clone_unique(eg0));

    tscalar::binary eg1 {
            tscalar::binary_operator::add,
            vref(vg),
            immediate(1),
    };
    processor.add_group_key(clone_unique(eg1));

    tscalar::binary eg2 {
            tscalar::binary_operator::add,
            tscalar::binary {
                    tscalar::binary_operator::add,
                    vref(vg),
                    immediate(1),
            },
            immediate(2),
    };
    processor.add_group_key(clone_unique(eg2));

    std::unique_ptr<tscalar::expression> element = clone_unique(tscalar::binary {
            tscalar::binary_operator::add,
            std::move(*clone_unique(eg2)),
            immediate(3),
    });
    bool result = processor.process(ownership_reference { element });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& project = find_next<trelation::project>(values);
    ASSERT_TRUE(project);
    ASSERT_EQ(project->columns().size(), 2);
    // eg0 ('g') is not placed in the project: bare variable references are used as group keys as is
    EXPECT_EQ(project->columns()[0].value(), eg1);
    EXPECT_EQ(project->columns()[1].value(), eg2);

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*project);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 3);
    EXPECT_EQ(aggregate->group_keys()[0], vg);
    EXPECT_EQ(aggregate->group_keys()[1], project->columns()[0].variable());
    EXPECT_EQ(aggregate->group_keys()[2], project->columns()[1].variable());
    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_EQ(*element, (tscalar::binary {
            tscalar::binary_operator::add,
            vref(aggregate->group_keys()[2]),
            immediate(3),
    }));

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, process_refer_group_key_expression_invalid) {
    // SELECT g + 2 GROUP BY g + 1
    set_function_processor processor { context(), graph_ };

    auto vg = vd("g", ttype::int8 {});
    tscalar::binary expr {
            tscalar::binary_operator::add,
            vref(vg),
            immediate(1),
    };
    processor.add_group_key(clone_unique(expr));

    std::unique_ptr<tscalar::expression> element = clone_unique(tscalar::binary {
            tscalar::binary_operator::add,
            vref(vg),
            immediate(2),
    });
    bool result = processor.process(ownership_reference { element });
    ASSERT_FALSE(result);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_aggregation_column));
}

TEST_F(set_function_processor_test, process_refer_aggregated_top) {
    // SELECT a GROUP BY g -- a: aggregated value
    set_function_processor processor { context(), graph_ };

    auto vg = vd("g", ttype::int8 {});
    processor.add_group_key(vg);

    auto va = vd("a", ttype::int8 {});
    processor.add_aggregated(va);

    std::unique_ptr<tscalar::expression> expr = clone_unique(vref(va));
    bool result = processor.process(ownership_reference { expr });
    ASSERT_TRUE(result);

    EXPECT_TRUE(processor.active());

    trelation::values values { {}, {} };
    auto&& output = processor.install(values.output());
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(values);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->columns().size(), 0);

    EXPECT_FALSE(aggregate->output().opposite());
}

TEST_F(set_function_processor_test, process_refer_invalid_variable_top) {
    // SELECT p GROUP BY g -- invalid
    set_function_processor processor { context(), graph_ };

    auto vg = vd("g", ttype::int8 {});
    processor.add_group_key(vg);

    auto vp = vd("p", ttype::int8 {});

    std::unique_ptr<tscalar::expression> expr = clone_unique(vref(vp));
    bool result = processor.process(ownership_reference { expr });
    EXPECT_FALSE(result);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_aggregation_column));
}

TEST_F(set_function_processor_test, process_refer_plain_variable_in_function) {
    // SELECT COUNT(p)
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());

    auto vp = vd("p", ttype::int8 {});

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
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& arguments = find_next<trelation::project>(values);
    ASSERT_TRUE(arguments);
    ASSERT_EQ(arguments->columns().size(), 1);
    EXPECT_EQ(arguments->columns()[0].value(), vref(vp));

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*arguments);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 0);
    ASSERT_EQ(aggregate->columns().size(), 1);
}

TEST_F(set_function_processor_test, process_refer_group_key_in_function) {
    // SELECT COUNT(g) GROUP BY g
    set_function_processor processor { context(), graph_ };

    auto vg = vd("g", ttype::int8 {});

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
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

    auto&& arguments = find_next<trelation::project>(values);
    ASSERT_TRUE(arguments);
    ASSERT_EQ(arguments->columns().size(), 1);
    EXPECT_EQ(arguments->columns()[0].value(), vref(vg));

    auto&& aggregate = find_next<trelation::intermediate::aggregate>(*arguments);
    ASSERT_TRUE(aggregate);
    ASSERT_EQ(aggregate->group_keys().size(), 1);
    ASSERT_EQ(aggregate->columns().size(), 1);
}

TEST_F(set_function_processor_test, process_refer_aggregated_in_function) {
    // SELECT COUNT(a) GROUP BY g -- a: aggregated value, invalid
    set_function_processor processor { context(), graph_ };

    auto vg = vd("g", ttype::int8 {});
    auto va = vd("a", ttype::int8 {});

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

TEST_F(set_function_processor_test, process_refer_argument_on_top_invalid) {
    // SELECT COUNT(p + 1), p + 1 -- never replace second `p + 1` with function argument
    set_function_processor processor { context(), graph_ };

    EXPECT_FALSE(processor.active());

    auto vp = vd("p", ttype::int8 {});

    std::unique_ptr<tscalar::expression> e0 = clone_unique(aggregate_function_call {
            factory_(count_value),
            {
                    tscalar::binary {
                            tscalar::binary_operator::add,
                            vref(vp),
                            immediate(1),
                    }
            }
    });
    bool r0 = processor.process(ownership_reference { e0 });
    ASSERT_TRUE(r0);

    std::unique_ptr<tscalar::expression> e1 = clone_unique(tscalar::binary {
            tscalar::binary_operator::add,
            vref(vp),
            immediate(1),
    });
    bool r1 = processor.process(ownership_reference { e1 });
    ASSERT_FALSE(r1);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_aggregation_column));
}

TEST_F(set_function_processor_test, column_unary) {
    // SELECT -COUNT(1)
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
    ASSERT_TRUE(output);
    EXPECT_FALSE(output->opposite());

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
