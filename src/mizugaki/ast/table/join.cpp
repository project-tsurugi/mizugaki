#include <mizugaki/ast/table/join.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

join::join(
        operator_kind_type operator_kind,
        unique_object_ptr<table::expression> left,
        unique_object_ptr<table::expression> right,
        unique_object_ptr<scalar::expression> condition,
        common::vector<unique_object_ptr<name::simple>> named_columns,
        element::region_type region) noexcept :
    super { region },
    operator_kind_ { operator_kind },
    left_ { std::move(left) },
    right_ { std::move(right) },
    condition_ { std::move(condition) },
    named_columns_ { std::move(named_columns) }
{}

join::join(join const& other, object_creator creator) :
    join {
            other.operator_kind_,
            clone_unique(other.left_, creator),
            clone_unique(other.right_, creator),
            clone_unique(other.condition_, creator),
            clone_vector(other.named_columns_, creator),
            other.region(),
    }
{}

join::join(join&& other, object_creator creator) :
    join {
            other.operator_kind_,
            clone_unique(std::move(other.left_), creator),
            clone_unique(std::move(other.right_), creator),
            clone_unique(std::move(other.condition_), creator),
            clone_vector(std::move(other.named_columns_), creator),
            other.region(),
    }
{}

join* join::clone(object_creator creator) const& {
    return creator.create_object<join>(*this, creator);
}

join* join::clone(object_creator creator) && {
    return creator.create_object<join>(std::move(*this), creator);
}

expression::node_kind_type join::node_kind() const noexcept {
    return tag;
}

join::operator_kind_type& join::operator_kind() noexcept {
    return operator_kind_;
}

join::operator_kind_type const& join::operator_kind() const noexcept {
    return operator_kind_;
}

unique_object_ptr<table::expression>& join::left() noexcept {
    return left_;
}

unique_object_ptr<table::expression> const& join::left() const noexcept {
    return left_;
}

unique_object_ptr<table::expression>& join::right() noexcept {
    return right_;
}

unique_object_ptr<table::expression> const& join::right() const noexcept {
    return right_;
}

unique_object_ptr<scalar::expression>& join::condition() noexcept {
    return condition_;
}

unique_object_ptr<scalar::expression> const& join::condition() const noexcept {
    return condition_;
}

common::vector<unique_object_ptr<name::simple>>& join::named_columns() noexcept {
    return named_columns_;
}

common::vector<unique_object_ptr<name::simple>> const& join::named_columns() const noexcept {
    return named_columns_;
}

} // namespace mizugaki::ast::table