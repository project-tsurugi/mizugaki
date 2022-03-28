#pragma once

#include <shakujo/model/statement/Statement.h>

#include <mizugaki/translator/shakujo_translator_result.h>

#include "../shakujo_translator_impl.h"

namespace mizugaki::translator::details {

class ddl_statement_translator {
public:
    using translator_type = shakujo_translator::impl;
    using result_type = shakujo_translator_result;

    explicit ddl_statement_translator(translator_type& translator) noexcept;

    [[nodiscard]] static bool is_supported(::shakujo::model::statement::Statement const& node);

    [[nodiscard]] result_type process(::shakujo::model::statement::Statement const& node);

private:
    translator_type& translator_;
};

} // namespace mizugaki::translator::details
