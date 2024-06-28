#pragma once

#include <optional>
#include <vector>

#include <tsl/hopscotch_set.h>

#include <takatori/descriptor/variable.h>

#include <takatori/relation/graph.h>
#include <takatori/relation/project.h>
#include <takatori/relation/intermediate/aggregate.h>

#include <takatori/util/optional_ptr.h>
#include <takatori/util/ownership_reference.h>

#include <yugawara/extension/scalar/aggregate_function_call.h>

#include "column_info.h"
#include "analyzer_context.h"

namespace mizugaki::analyzer::details {

class set_function_processor {
public:
    set_function_processor(
            analyzer_context& context,
            ::takatori::relation::graph_type& graph);

    [[nodiscard]] bool active() const noexcept;

    void activate() noexcept;

    void add_group_key(::takatori::descriptor::variable column);

    void add_aggregated(::takatori::descriptor::variable column);

    [[nodiscard]] bool is_grouping(::takatori::descriptor::variable const& column) const;

    [[nodiscard]] bool is_aggregated(::takatori::descriptor::variable const& column) const;

    [[nodiscard]] bool process(::takatori::util::ownership_reference<::takatori::scalar::expression> expression);

    ::takatori::relation::expression::output_port_type& install(::takatori::relation::expression::output_port_type& port);

    void consume(::takatori::util::ownership_reference<::takatori::scalar::expression> expression);

private:
    analyzer_context& context_;
    ::takatori::relation::graph_type& graph_;

    bool active_ { false };
    ::tsl::hopscotch_set<::takatori::descriptor::variable> grouping_columns_ {};
    ::tsl::hopscotch_set<::takatori::descriptor::variable> aggregated_columns_ {};

    ::takatori::util::optional_ptr<::takatori::relation::project> arguments_ {};
    ::takatori::util::optional_ptr<::takatori::relation::intermediate::aggregate> aggregations_ {};

    [[nodiscard]] ::takatori::relation::project& arguments_store();

    [[nodiscard]] ::takatori::relation::intermediate::aggregate& aggregations_store();

    [[nodiscard]] ::takatori::descriptor::variable replace_argument(
            ::takatori::util::ownership_reference<::takatori::scalar::expression> expression);

    [[nodiscard]] std::optional<::takatori::descriptor::variable> find_aggregation(
            ::takatori::descriptor::aggregate_function const& function,
            std::vector<::takatori::descriptor::variable> const& arguments);
};

} // namespace mizugaki::analyzer::details
