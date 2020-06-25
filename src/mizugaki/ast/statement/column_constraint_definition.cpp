#include <mizugaki/ast/statement/column_constraint_definition.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

column_constraint_definition::column_constraint_definition(
        std::unique_ptr<name::name> name,
        std::unique_ptr<constraint> body,
        region_type region) noexcept :
    element { region },
    name_ { std::move(name) },
    body_ { std::move(body) }
{}

column_constraint_definition::column_constraint_definition(
        constraint&& body,
        rvalue_ptr<name::name> name,
        region_type region) :
    column_constraint_definition {
            clone_unique(name),
            clone_unique(std::move(body)),
            region,
    }
{}

column_constraint_definition::column_constraint_definition(::takatori::util::clone_tag_t, column_constraint_definition const& other) :
    column_constraint_definition {
            clone_unique(*other.name_),
            clone_unique(*other.body_),
            other.region(),
    }
{}

column_constraint_definition::column_constraint_definition(::takatori::util::clone_tag_t, column_constraint_definition&& other) :
    column_constraint_definition {
            std::move(*other.name_),
            std::move(*other.body_),
            other.region(),
    }
{}

std::unique_ptr<name::name>& column_constraint_definition::name() noexcept {
    return *name_;
}

std::unique_ptr<name::name> const& column_constraint_definition::name() const noexcept {
    return *name_;
}

std::unique_ptr<constraint>& column_constraint_definition::body() noexcept {
    return *body_;
}

std::unique_ptr<constraint> const& column_constraint_definition::body() const noexcept {
    return *body_;
}

bool operator==(column_constraint_definition const& a, column_constraint_definition const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(*a.name_, *b.name_)
            && eq(*a.body_, *b.body_);
}

bool operator!=(column_constraint_definition const& a, column_constraint_definition const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, column_constraint_definition const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "name"sv, *value.name_);
    property(acceptor, "body"sv, *value.body_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, column_constraint_definition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
