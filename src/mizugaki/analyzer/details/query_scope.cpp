#include <mizugaki/analyzer/details/query_scope.h>

#include <takatori/util/fail.h>
#include <takatori/util/print_support.h>
#include <takatori/util/optional_print_support.h>

namespace mizugaki::analyzer::details {

using ::takatori::util::optional_ptr;
using ::takatori::util::sequence_view;

query_scope::query_scope(
        optional_ptr<query_scope const> parent) :
    parent_ { std::move(parent) }
{}

optional_ptr<query_scope const>& query_scope::parent() noexcept {
    return parent_;
}

optional_ptr<query_scope const> query_scope::parent() const noexcept {
    return parent_;
}

sequence_view<relation_info> query_scope::references() noexcept {
    return references_;
}

sequence_view<relation_info const> query_scope::references() const noexcept {
    return references_;
}

void query_scope::reserve(std::size_t count) {
    references_.reserve(count);
    reference_name_map_.reserve(count);
}

relation_info& query_scope::add(relation_info info) {
    auto position = references_.size();
    auto&& added = references_.emplace_back(std::move(info));
    if (auto&& id = added.identifier(); !id.empty()) {
        if (auto [it, success] = reference_name_map_.emplace(id, position); !success) {
            // FIXME: duplicate correlation
            (void) it;
            ::takatori::util::fail(); // FIXME: fail
        }
    }
    return added;
}

optional_ptr<relation_info> query_scope::find(std::string_view identifier) {
    if (auto pos = find_internal(identifier)) {
        return references_[*pos];
    }
    return {};
}

optional_ptr<relation_info const> query_scope::find(std::string_view identifier) const {
    if (auto pos = find_internal(identifier)) {
        return references_[*pos];
    }
    return {};
}

optional_ptr<relation_info> query_scope::find(::yugawara::storage::relation const& relation) {
    if (auto pos = find_internal(relation)) {
        return references_[*pos];
    }
    return {};
}

optional_ptr<relation_info const> query_scope::find(::yugawara::storage::relation const& relation) const {
    if (auto pos = find_internal(relation)) {
        return references_[*pos];
    }
    return {};
}

std::optional<query_scope::position_type> query_scope::find_internal(std::string_view identifier) const {
    auto&& map = reference_name_map_;
    if (auto it = map.find(identifier); it != map.end()) {
        return it.value();
    }
    return {};
}

std::optional<query_scope::position_type> query_scope::find_internal(yugawara::storage::relation const& relation) const {
    auto&& list = references_;
    for (position_type i = 0, n = list.size(); i < n; ++i) {
        auto&& ref = list[i];
        if (ref.declaration().get() == std::addressof(relation)) {
            // FIXME: ambiguous
            return i;
        }
    }
    return {};
}

query_scope::position_type query_scope::create_pivot() const noexcept {
    return references_.size();
}

std::optional<query_scope::position_type> query_scope::position_of(relation_info const& relation) const noexcept {
    for (position_type position = 0, n = references_.size(); position < n; ++position) {
        if (std::addressof(relation) == std::addressof(references_[position])) {
            return position;
        }
    }
    return {};
}

std::ostream& operator<<(std::ostream& out, query_scope const& value) {
    using ::takatori::util::print_support;
    return out << "query_scope("
               << "this=" << &value << ", "
               << "parent=" << value.parent().get() << ", "
               << "references=" << print_support { value.references() } << ")";
}

} // namespace mizugaki::analyzer::details
