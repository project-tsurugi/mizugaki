#pragma once

#include <shakujo/model/expression/Expression.h>

#include <takatori/relation/expression.h>
#include <takatori/relation/graph.h>
#include <takatori/util/optional_ptr.h>

#include "relation_info.h"
#include "../shakujo_translator_impl.h"
#include "../shakujo_translator_context_impl.h"

namespace mizugaki::translator::details {

class relation_expression_translator {
public:
    using translator_type = shakujo_translator::impl;
    using result_type = ::takatori::util::optional_ptr<::takatori::relation::expression::output_port_type>;

    explicit relation_expression_translator(translator_type& translator) noexcept;

    [[nodiscard]] result_type process(
            ::shakujo::model::expression::Expression const& node,
            ::takatori::relation::graph_type& graph,
            relation_info& relation);

private:
    translator_type& translator_;
};

} // namespace mizugaki::translator::details
