#pragma once

#include <memory>

#include <shakujo/common/core/Value.h>

#include <takatori/value/data.h>

#include "../shakujo_translator_impl.h"
#include "../shakujo_translator_context_impl.h"

namespace mizugaki::translator::details {

class value_info_translator {
public:
    using translator_type = shakujo_translator::impl;
    using result_type = std::shared_ptr<::takatori::value::data>;

    explicit value_info_translator(translator_type& translator) noexcept;

    [[nodiscard]] result_type process(
            ::shakujo::common::core::Value const& info,
            ::shakujo::common::core::DocumentRegion const& region);

private:
    translator_type& translator_;
};

} // namespace mizugaki::translator::details
