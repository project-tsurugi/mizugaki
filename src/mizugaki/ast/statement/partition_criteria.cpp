#include <mizugaki/ast/statement/partition_criteria.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using common::clone_vector;

partition_criteria::partition_criteria(
        std::vector<std::unique_ptr<predicate_type>> predicates,
        region_type region) noexcept :
    element { region },
    predicates_ { std::move(predicates) }
{}

partition_criteria::partition_criteria(
        common::rvalue_list<predicate_type> predicates,
        region_type region) :
    partition_criteria {
            common::to_vector(predicates),
            region,
    }
{}

partition_criteria::partition_criteria(::takatori::util::clone_tag_t, partition_criteria const& other) :
    partition_criteria {
            clone_vector(*other.predicates_),
            other.region(),
    }
{}

partition_criteria::partition_criteria(::takatori::util::clone_tag_t, partition_criteria&& other) :
    partition_criteria {
            clone_vector(std::move(*other.predicates_)),
            other.region(),
    }
{}

std::vector<std::unique_ptr<partition_criteria::predicate_type>>& partition_criteria::predicates() noexcept {
    return *predicates_;
}

std::vector<std::unique_ptr<partition_criteria::predicate_type>> const& partition_criteria::predicates() const noexcept {
    return *predicates_;
}

bool operator==(partition_criteria const& a, partition_criteria const& b) noexcept {
    return eq(*a.predicates_, *b.predicates_);
}

bool operator!=(partition_criteria const& a, partition_criteria const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(
        ::takatori::serializer::object_acceptor& acceptor,
        partition_criteria const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "predicates"sv, *value.predicates_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, partition_criteria const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
