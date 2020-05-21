#pragma once

#include <memory>

#include <shakujo/model/type/Type.h>

#include <takatori/type/data.h>

#include "../shakujo_translator_impl.h"

namespace mizugaki::translator::details {

class type_translator {
public:
    using translator_type = shakujo_translator::impl;
    using result_type = std::shared_ptr<::takatori::type::data>;

    explicit type_translator(translator_type& translator) noexcept;

    [[nodiscard]] result_type process(::shakujo::model::type::Type const& node);

private:
    translator_type& translator_;
};

} // namespace mizugaki::translator::details
