#include <mizugaki/ast/scalar/trim_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::rvalue_ptr;
using ::takatori::util::unique_object_ptr;

trim_expression::trim_expression(
        std::optional<specification_type> specification,
        operand_type character,
        operand_type source,
        region_type region) noexcept :
    super { region },
    specification_ { std::move(specification) },
    character_ { std::move(character) },
    source_ { std::move(source) }
{}

trim_expression::trim_expression(
        std::optional<specification_type> specification,
        rvalue_ptr<expression> character,
        expression&& source,
        region_type region) :
    trim_expression {
            std::move(specification),
            clone_unique(character),
            clone_unique(std::move(source)),
            region,
    }
{}

trim_expression::trim_expression(trim_expression const& other, object_creator creator) :
    trim_expression {
            other.specification_,
            clone_unique(other.character_, creator),
            clone_unique(other.source_, creator),
            other.region(),
    }
{}

trim_expression::trim_expression(trim_expression&& other, object_creator creator) :
    trim_expression {
            std::move(other.specification_),
            clone_unique(std::move(other.character_), creator),
            clone_unique(std::move(other.source_), creator),
            other.region(),
    }
{}

trim_expression* trim_expression::clone(object_creator creator) const& {
    return creator.create_object<trim_expression>(*this, creator);
}

trim_expression* trim_expression::clone(object_creator creator) && {
    return creator.create_object<trim_expression>(std::move(*this), creator);
}

expression::node_kind_type trim_expression::node_kind() const noexcept {
    return tag;
}

std::optional<trim_expression::specification_type>& trim_expression::specification() noexcept {
    return specification_;
}

std::optional<trim_expression::specification_type> const& trim_expression::specification() const noexcept {
    return specification_;
}

expression::operand_type& trim_expression::character() noexcept {
    return character_;
}

expression::operand_type const& trim_expression::character() const noexcept {
    return character_;
}

expression::operand_type& trim_expression::source() noexcept {
    return source_;
}

expression::operand_type const& trim_expression::source() const noexcept {
    return source_;
}

bool operator==(trim_expression const& a, trim_expression const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.specification_, b.specification_)
            && eq(a.character_, b.character_)
            && eq(a.source_, b.source_);
}

bool operator!=(trim_expression const& a, trim_expression const& b) noexcept {
    return !(a == b);
}

bool trim_expression::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void trim_expression::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "character"sv, character_);
    property(acceptor, "specification"sv, specification_);
    property(acceptor, "source"sv, source_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, trim_expression const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
