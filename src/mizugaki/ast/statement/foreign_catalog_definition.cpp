#include <mizugaki/ast/statement/foreign_catalog_definition.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

using common::clone_vector;

foreign_catalog_definition::foreign_catalog_definition(
        std::unique_ptr<name::name> name,
        std::unique_ptr<name::name> server_name,
        std::vector<option_type> options,
        region_type region) noexcept :
    super { region },
    name_ { std::move(name) },
    server_name_ { std::move(server_name) },
    options_ { std::move(options) }
{}

foreign_catalog_definition::foreign_catalog_definition(
        ::takatori::util::rvalue_ptr<name::name> name,
        ::takatori::util::rvalue_ptr<name::name> server_name,
        std::initializer_list<option_type> options,
        region_type region) :
    foreign_catalog_definition {
            clone_unique(name),
            clone_unique(server_name),
            options,
            region,
    }
{}

foreign_catalog_definition::foreign_catalog_definition(::takatori::util::clone_tag_t, foreign_catalog_definition const& other) :
    foreign_catalog_definition {
            clone_unique(other.name_),
            clone_unique(other.server_name_),
            clone_vector(other.options_),
            other.region(),
    }
{}

foreign_catalog_definition::foreign_catalog_definition(::takatori::util::clone_tag_t, foreign_catalog_definition&& other) :
    foreign_catalog_definition {
            std::move(other.name_),
            std::move(other.server_name_),
            std::move(other.options_),
            other.region(),
    }
{}

foreign_catalog_definition* foreign_catalog_definition::clone() const& {
    return new foreign_catalog_definition(::takatori::util::clone_tag, *this); // NOLINT
}

foreign_catalog_definition* foreign_catalog_definition::clone() && {
    return new foreign_catalog_definition(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type foreign_catalog_definition::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& foreign_catalog_definition::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& foreign_catalog_definition::name() const noexcept {
    return name_;
}

std::unique_ptr<name::name>& foreign_catalog_definition::server_name() noexcept {
    return server_name_;
}

std::unique_ptr<name::name> const& foreign_catalog_definition::server_name() const noexcept {
    return server_name_;
}

std::vector<foreign_catalog_definition::option_type>& foreign_catalog_definition::options() noexcept {
    return options_;
}

std::vector<foreign_catalog_definition::option_type> const& foreign_catalog_definition::options() const noexcept {
    return options_;
}

bool operator==(foreign_catalog_definition const& a, foreign_catalog_definition const& b) noexcept {
    return eq(a.name_, b.name_)
        && eq(a.server_name_, b.server_name_)
        && eq(a.options_, b.options_);
}

bool operator!=(foreign_catalog_definition const& a, foreign_catalog_definition const& b) noexcept {
    return !(a == b);
}

bool foreign_catalog_definition::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void foreign_catalog_definition::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    property(acceptor, "server_name"sv, server_name_);
    property(acceptor, "options"sv, options_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, foreign_catalog_definition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
