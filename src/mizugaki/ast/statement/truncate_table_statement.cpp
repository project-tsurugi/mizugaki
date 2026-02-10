#include <mizugaki/ast/statement/truncate_table_statement.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::statement {

using identity_column_option_type = truncate_table_statement::identity_column_option_type;

using ::takatori::util::clone_unique;


truncate_table_statement::truncate_table_statement(
        std::unique_ptr<name::name> name,
        std::optional<identity_column_option_type> identity_column_option,
        region_type region) noexcept :
    super { region },
    name_ { std::move(name) },
    identity_column_option_ { identity_column_option }
{}

truncate_table_statement::truncate_table_statement(
        name::name&& name,
        std::optional<identity_column_option_type> identity_column_option,
        region_type region) noexcept :
    truncate_table_statement {
            clone_unique(std::move(name)),
            identity_column_option,
            region,
    }
{}

truncate_table_statement::truncate_table_statement(takatori::util::clone_tag_t, truncate_table_statement const& other) :
    truncate_table_statement {
            clone_unique(other.name_),
            other.identity_column_option_,
    }
{}

truncate_table_statement::truncate_table_statement(takatori::util::clone_tag_t, truncate_table_statement&& other) :
    truncate_table_statement {
            std::move(other.name_),
            other.identity_column_option_,
    }
{}

truncate_table_statement* truncate_table_statement::clone() const & {
    return new truncate_table_statement(::takatori::util::clone_tag, *this); // NOLINT
}

truncate_table_statement* truncate_table_statement::clone() && {
    return new truncate_table_statement(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type truncate_table_statement::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& truncate_table_statement::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& truncate_table_statement::name() const noexcept {
    return name_;
}

std::optional<identity_column_option_type>& truncate_table_statement::identity_column_option() noexcept {
    return identity_column_option_;
}

std::optional<identity_column_option_type> const& truncate_table_statement::identity_column_option() const noexcept {
    return identity_column_option_;
}

bool operator==(truncate_table_statement const& a, truncate_table_statement const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.name_, b.name_)
        && eq(a.identity_column_option_, b.identity_column_option_);
}

bool operator!=(truncate_table_statement const& a, truncate_table_statement const& b) noexcept {
    return !(a == b);
}

bool truncate_table_statement::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void truncate_table_statement::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    property(acceptor, "identity_column_option"sv, identity_column_option_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, truncate_table_statement const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
