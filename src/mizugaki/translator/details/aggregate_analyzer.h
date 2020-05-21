#pragma once

#include <tsl/hopscotch_set.h>

#include <takatori/descriptor/variable.h>

#include <takatori/relation/project.h>
#include <takatori/relation/intermediate/aggregate.h>

#include <takatori/util/optional_ptr.h>

#include "relation_info.h"
#include "../shakujo_translator_impl.h"

namespace mizugaki::translator::details {

class aggregate_analyzer {
public:
    using translator_type = shakujo_translator::impl;

    explicit aggregate_analyzer(translator_type& translator);

    [[nodiscard]] bool process(
            ::takatori::relation::project& projection,
            ::takatori::relation::intermediate::aggregate& aggregation,
            relation_info const& source);

    [[nodiscard]] bool validate_group_member(relation_info const& output);

private:
    translator_type& translator_;
    ::tsl::hopscotch_set<::takatori::descriptor::variable> group_members_;
};

} // namespace mizugaki::translator::details
