#include <mizugaki/ast/statement/drop_statement.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

drop_statement::drop_statement(
        statement_kind_type statement_kind,
        std::unique_ptr<name::name> name,
        std::vector<option_type> options,
        region_type region) :
    super { region },
    statement_kind_ { statement_kind },
    name_ { std::move(name) },
    options_ {std::move(options) }
{
    utils::validate_kind(tags, *statement_kind);
}

drop_statement::drop_statement(
        statement_kind_type statement_kind,
        name::name&& name,
        std::initializer_list<option_type> options,
        region_type region) :
    drop_statement {
            statement_kind,
            clone_unique(std::move(name)),
            options,
            region,
    }
{}

drop_statement::drop_statement(::takatori::util::clone_tag_t, drop_statement const& other) :
    drop_statement {
            other.statement_kind_,
            clone_unique(other.name_),
            other.options_,
            other.region(),
    }
{}

drop_statement::drop_statement(::takatori::util::clone_tag_t, drop_statement&& other) :
        drop_statement {
                other.statement_kind_,
                std::move(other.name_),
                std::move(other.options_),
                other.region(),
        }
{}

drop_statement* drop_statement::clone() const& {
    return new drop_statement(::takatori::util::clone_tag, *this); // NOLINT
}

drop_statement* drop_statement::clone() && {
    return new drop_statement(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type drop_statement::node_kind() const noexcept {
    return *statement_kind_;
}

drop_statement::statement_kind_type& drop_statement::statement_kind() noexcept {
    return statement_kind_;
}

drop_statement::statement_kind_type const& drop_statement::statement_kind() const noexcept {
    return statement_kind_;
}

std::unique_ptr<name::name>& drop_statement::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& drop_statement::name() const noexcept {
    return name_;
}

std::vector<drop_statement::option_type>& drop_statement::options() noexcept {
    return options_;
}

std::vector<drop_statement::option_type> const& drop_statement::options() const noexcept {
    return options_;
}

bool operator==(drop_statement const& a, drop_statement const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.statement_kind_, b.statement_kind_)
        && eq(a.name_, b.name_)
        && eq(a.options_, b.options_);
}

bool operator!=(drop_statement const& a, drop_statement const& b) noexcept {
    return !(a == b);
}

bool drop_statement::equals(statement const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<drop_statement>(other);
}

void drop_statement::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    property(acceptor, "options"sv, options_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, drop_statement const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
