#pragma once

#include <memory>

#include <shakujo/common/core/Type.h>

#include <takatori/type/data.h>

#include "../shakujo_translator_impl.h"
#include "../shakujo_translator_context_impl.h"

namespace mizugaki::translator::details {

class type_info_translator {
public:
    using translator_type = shakujo_translator::impl;
    using result_type = std::shared_ptr<::takatori::type::data>;

    explicit type_info_translator(translator_type& translator) noexcept;

    [[nodiscard]] result_type process(
            ::shakujo::common::core::Type const& info,
            ::shakujo::common::core::DocumentRegion const& region);

private:
    translator_type& translator_;
};

} // namespace mizugaki::translator::details
