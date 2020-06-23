#include <mizugaki/ast/compilation_unit.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast {

using ::takatori::util::maybe_shared_ptr;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

compilation_unit::compilation_unit(
        common::vector<unique_object_ptr<statement::statement>> statements,
        common::vector<region_type> comments,
        maybe_shared_ptr<document_type const> document) noexcept :
    statements_ { std::move(statements) },
    comments_ { std::move(comments) },
    document_ { std::move(document) }
{}

compilation_unit::compilation_unit(
        common::rvalue_list<statement::statement> statements,
        std::initializer_list<region_type> comments) :
    compilation_unit {
            common::to_vector(statements),
            decltype(comments_) { comments },
    }
{}

compilation_unit::compilation_unit(compilation_unit const& other, object_creator creator) :
    compilation_unit {
            clone_vector(other.statements_, creator),
            { other.comments_, creator.allocator() },
            other.document_,
    }
{}

compilation_unit::compilation_unit(compilation_unit&& other, object_creator creator) :
    compilation_unit {
            clone_vector(std::move(other.statements_), creator),
            { std::move(other.comments_), creator.allocator() },
            other.document_,
    }
{}

common::vector<unique_object_ptr<statement::statement>>& compilation_unit::statements() noexcept {
    return statements_;
}

common::vector<unique_object_ptr<statement::statement>> const& compilation_unit::statements() const noexcept {
    return statements_;
}

common::vector<compilation_unit::region_type>& compilation_unit::comments() noexcept {
    return comments_;
}

common::vector<compilation_unit::region_type> const& compilation_unit::comments() const noexcept {
    return comments_;
}

maybe_shared_ptr<compilation_unit::document_type const>& compilation_unit::document() noexcept {
    return document_;
}

maybe_shared_ptr<compilation_unit::document_type const> const& compilation_unit::document() const noexcept {
    return document_;
}

bool operator==(compilation_unit const& a, compilation_unit const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.statements_, b.statements_);
}

bool operator!=(compilation_unit const& a, compilation_unit const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, compilation_unit const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "statements"sv, value.statements_);
    property(acceptor, "comments"sv, value.comments_);
    if (value.document_) {
        property(acceptor, "document"sv, value.document_->location());
    }
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, compilation_unit const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast
