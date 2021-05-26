#pragma once

#include <shakujo/model/expression/Expression.h>

#include <takatori/scalar/expression.h>

#include "variable_scope.h"
#include "../shakujo_translator_impl.h"

namespace mizugaki::translator::details {

class scalar_expression_translator {
public:
    using translator_type = shakujo_translator::impl;
    using result_type = std::unique_ptr<::takatori::scalar::expression>;

    explicit scalar_expression_translator(translator_type& translator) noexcept;

    [[nodiscard]] result_type process(
            ::shakujo::model::expression::Expression const& node,
            variable_scope const& scope);

    [[nodiscard]] bool saw_aggregate() const noexcept;

private:
    translator_type& translator_;
    bool saw_aggregate_ { false };
};

} // namespace mizugaki::translator::details
