#include <mizugaki/analyzer/details/query_scope.h>

#include <algorithm>

#include <takatori/util/fail.h>
#include <takatori/util/print_support.h>
#include <takatori/util/optional_print_support.h>

#include <yugawara/binding/factory.h>

namespace mizugaki::analyzer::details {

namespace descriptor = ::takatori::descriptor;

using ::takatori::util::optional_ptr;
using ::takatori::util::sequence_view;

query_scope::query_scope(
        optional_ptr<query_scope> parent) :
    parent_ { std::move(parent) }
{}

optional_ptr<query_scope>& query_scope::parent() noexcept {
    return parent_;
}

optional_ptr<query_scope const> query_scope::parent() const noexcept {
    return parent_;
}

sequence_view<relation_info> query_scope::references() noexcept {
    return relations_;
}

sequence_view<relation_info const> query_scope::references() const noexcept {
    return relations_;
}

void query_scope::reserve(std::size_t size) {
    relations_.reserve(size);
    name_map_.reserve(size);
    reference_map_.reserve(size);
}

relation_info& query_scope::add(relation_info info) {
    auto position = relations_.size();
    auto&& added = relations_.emplace_back(std::move(info));

    if (auto&& id = added.identifier(); !id.empty()) {
        if (auto [it, success] = name_map_.try_emplace(id, position); !success) {
            it.value() = ambiguous;
        }
    } else if (auto ref = added.declaration()) {
        auto&& name = ref->simple_name();
        if (auto [it, success] = name_map_.try_emplace(std::string { name }, position); !success) {
            it.value() = ambiguous;
        }
        if (auto [it, success] = reference_map_.try_emplace(ref.get(), position); !success) {
            it.value() = ambiguous;
        }
    }
    return added;
}

query_scope::result_type query_scope::find(std::string_view identifier) {
    if (auto pos = find_internal(identifier)) {
        if (*pos == ambiguous) {
            return result_type::ambiguous;
        }
        return relations_[*pos];
    }
    return {};
}

query_scope::const_result_type query_scope::find(std::string_view identifier) const {
    if (auto pos = find_internal(identifier)) {
        if (*pos == ambiguous) {
            return const_result_type::ambiguous;
        }
        return relations_[*pos];
    }
    return {};
}

query_scope::result_type query_scope::find(::yugawara::storage::relation const& relation) {
    if (auto pos = find_internal(relation)) {
        if (*pos == ambiguous) {
            return result_type::ambiguous;
        }
        return relations_[*pos];
    }
    return {};
}

query_scope::const_result_type query_scope::find(::yugawara::storage::relation const& relation) const {
    if (auto pos = find_internal(relation)) {
        if (*pos == ambiguous) {
            return const_result_type::ambiguous;
        }
        return relations_[*pos];
    }
    return {};
}

std::optional<query_scope::position_type> query_scope::find_internal(std::string_view identifier) const {
    auto&& map = name_map_;
    if (auto it = map.find(identifier); it != map.end()) {
        return it.value();
    }
    return {};
}

std::optional<query_scope::position_type> query_scope::find_internal(::yugawara::storage::relation const& relation) const {
    auto&& map = reference_map_;
    if (auto it = map.find(std::addressof(relation)); it != map.end()) {
        return it.value();
    }
    return {};
}

query_scope::position_type query_scope::create_pivot() const noexcept {
    return relations_.size();
}

bool query_scope::add(std::string name, std::shared_ptr<query_info const> query) { // NOLINT(performance-unnecessary-value-param)
    auto [it, added] = query_map_.try_emplace(std::move(name), std::move(query));
    (void) it;
    return added;
}

std::shared_ptr<query_info const> query_scope::find_query(std::string_view name) const {
    if (auto it = query_map_.find(name); it != query_map_.end())  {
        return it.value();
    }
    return {};
}

bool& query_scope::capture_parameters() noexcept {
    return capture_parameters_;
}

bool query_scope::capture_parameters() const noexcept {
    return capture_parameters_;
}

std::vector<std::tuple<descriptor::variable, descriptor::variable>>& query_scope::list_parameters() {
    return parameters_;
}

std::vector<std::tuple<descriptor::variable, descriptor::variable>> const& query_scope::list_parameters() const {
    return parameters_;
}

descriptor::variable& query_scope::add_parameter(descriptor::variable const& free_variable) {
    auto iter = std::find_if(
            parameters_.begin(),
            parameters_.end(),
            [&](auto const& pair) {
                return free_variable == std::get<0>(pair);
            });
    if (iter != parameters_.end()) {
        return std::get<1>(*iter);
    }
    ::yugawara::binding::factory factory {};
    auto parameter = factory.frame_variable(free_variable);
    auto&& entry = parameters_.emplace_back(free_variable, std::move(parameter));
    return std::get<1>(entry);
}

std::ostream& operator<<(std::ostream& out, query_scope const& value) {
    using ::takatori::util::print_support;
    return out << "query_scope("
               << "this=" << &value << ", "
               << "parent=" << value.parent().get() << ", "
               << "references=" << print_support { value.references() } << ")";
}

} // namespace mizugaki::analyzer::details
