#include "variable_scope.h"

#include <takatori/util/downcast.h>

#include <yugawara/binding/factory.h>

#include <shakujo/model/name/SimpleName.h>

namespace mizugaki::translator::details {

using ::takatori::util::unsafe_downcast;

variable_scope::variable_scope(context_type const& context, relation_info const& relation) noexcept
    : context_(context)
    , relation_(relation)
{}

std::optional<::takatori::descriptor::variable> variable_scope::find(::shakujo::model::name::Name const& name) const {
    if (auto v = relation_.find_variable(name)) {
        return *v;
    }
    if (name.kind() == ::shakujo::model::name::SimpleName::tag) {
        auto&& n = unsafe_downcast<::shakujo::model::name::SimpleName>(name);
        if (auto v = context_.variables().find(n.token())) {
            ::yugawara::binding::factory f { context_.get_object_creator() };
            return f(std::move(v));
        }
    }
    return {};
}

} // namespace mizugaki::translator::details
