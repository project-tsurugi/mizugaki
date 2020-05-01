#pragma once

#include <optional>

#include <takatori/descriptor/variable.h>

#include <shakujo/model/name/Name.h>
#include <shakujo/model/name/SimpleName.h>

#include "relation_info.h"
#include "../shakujo_translator_context_impl.h"

namespace mizugaki::translator::details {

class variable_scope {
public:
    using context_type = shakujo_translator_context::impl;

    variable_scope(context_type const& context, relation_info const& relation) noexcept;

    [[nodiscard]] std::optional<::takatori::descriptor::variable> find(::shakujo::model::name::Name const& name) const;

private:
    context_type const& context_;
    relation_info const& relation_;
};

} // namespace mizugaki::translator::details
