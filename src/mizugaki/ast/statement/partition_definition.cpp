#include <mizugaki/ast/statement/partition_definition.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using common::clone_vector;

partition_definition::partition_definition(
        std::vector<element_type> elements,
        region_type region) noexcept :
    element { region },
    elements_ { std::move(elements) }
{}

partition_definition::partition_definition(
        std::initializer_list<element_type> elements,
        region_type region) :
    partition_definition {
            std::vector<element_type> { elements },
            region,
    }
{}

partition_definition::partition_definition(::takatori::util::clone_tag_t, partition_definition const& other) :
    partition_definition {
            clone_vector(other.elements_),
            other.region(),
    }
{}

partition_definition::partition_definition(::takatori::util::clone_tag_t, partition_definition&& other) :
    partition_definition {
            clone_vector(std::move(other.elements_)),
            other.region(),
    }
{}

std::vector<partition_definition::element_type>& partition_definition::elements() noexcept {
    return elements_;
}

std::vector<partition_definition::element_type> const& partition_definition::elements() const noexcept {
    return elements_;
}

bool operator==(partition_definition const& a, partition_definition const& b) noexcept {
    return eq(a.elements_, b.elements_);
}

bool operator!=(partition_definition const& a, partition_definition const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(
        ::takatori::serializer::object_acceptor& acceptor,
        partition_definition const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "elements"sv, value.elements_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, partition_definition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
