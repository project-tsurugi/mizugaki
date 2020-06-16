#include <mizugaki/ast/compilation_unit.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast {

using ::takatori::util::maybe_shared_ptr;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

compilation_unit::compilation_unit(
        common::vector<unique_object_ptr<statement::statement>> statements,
        maybe_shared_ptr<document_type const> document) noexcept :
    statements_ { std::move(statements) },
    document_ { std::move(document) }
{}

compilation_unit::compilation_unit(compilation_unit const& other, object_creator creator) :
    compilation_unit {
            clone_vector(other.statements_, creator),
            other.document_,
    }
{}

compilation_unit::compilation_unit(compilation_unit&& other, object_creator creator) :
    compilation_unit {
            clone_vector(std::move(other.statements_), creator),
            other.document_,
    }
{}

common::vector<unique_object_ptr<statement::statement>>& compilation_unit::statements() noexcept {
    return statements_;
}

common::vector<unique_object_ptr<statement::statement>> const& compilation_unit::statements() const noexcept {
    return statements_;
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

} // namespace mizugaki::ast
