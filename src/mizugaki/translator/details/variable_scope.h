#pragma once

#include <optional>

#include <takatori/descriptor/variable.h>

#include <shakujo/model/name/Name.h>
#include <shakujo/model/name/SimpleName.h>

#include <mizugaki/translator/shakujo_translator_options.h>

#include "relation_info.h"

namespace mizugaki::translator::details {

class variable_scope {
public:
    using options_type = shakujo_translator_options;

    variable_scope(options_type const& options, relation_info const& relation) noexcept;

    [[nodiscard]] std::optional<::takatori::descriptor::variable> find(::shakujo::model::name::Name const& name) const;

private:
    options_type const& options_;
    relation_info const& relation_;
};

} // namespace mizugaki::translator::details
