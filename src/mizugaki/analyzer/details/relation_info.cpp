#include <mizugaki/analyzer/details/relation_info.h>

#include <boost/dynamic_bitset.hpp>

#include <takatori/util/downcast.h>
#include <takatori/util/fail.h>
#include <takatori/util/print_support.h>
#include <takatori/util/optional_print_support.h>

#include <yugawara/storage/provider.h>

namespace mizugaki::analyzer::details {

using ::takatori::util::optional_ptr;
using ::takatori::util::sequence_view;
using ::takatori::util::unsafe_downcast;

namespace storage = ::yugawara::storage;

relation_info::relation_info(
        optional_ptr<storage::relation const> declaration,
        std::string identifier) :
    declaration_ { std::move(declaration) },
    identifier_ { std::move(identifier) }
{}

optional_ptr<storage::relation const>& relation_info::declaration() noexcept {
    return declaration_;
}

optional_ptr<storage::relation const> relation_info::declaration() const noexcept {
    return declaration_;
}

std::shared_ptr<storage::index const> relation_info::primary_index() const {
    if (!declaration_) {
        return {};
    }
    if (declaration_->kind() != storage::relation_kind::table) {
        return {};
    }
    auto provider = declaration_->owner();
    if (!provider) {
        return {};
    }
    auto&& table = unsafe_downcast<storage::table>(*declaration_);
    return provider->find_primary_index(table);
}

std::string& relation_info::identifier() noexcept {
    return identifier_;
}

std::string const& relation_info::identifier() const noexcept {
    return identifier_;
}

sequence_view<column_info> relation_info::columns() noexcept {
    return columns_;
}

sequence_view<column_info const> relation_info::columns() const noexcept {
    return columns_;
}

void relation_info::reserve(std::size_t count, bool also_declaration_map) {
    columns_.reserve(count);
    name_map_.reserve(count);
    if (also_declaration_map) {
        declaration_map_.reserve(count);
    }
}

column_info& relation_info::add(column_info info) {
    info.next().reset();
    auto position = columns_.size();
    columns_.emplace_back(std::move(info));
    return build_internal(position);
}

void relation_info::erase(position_type first, position_type last) {
    if (first >= last) {
        return;
    }
    using difference_type = std::iterator_traits<decltype(columns_)::iterator>::difference_type;
    columns_.erase(
            columns_.begin() + static_cast<difference_type>(first),
            columns_.begin() + static_cast<difference_type>(last));
}

find_element_result<column_info> relation_info::find(std::string_view identifier) {
    if (auto pos = find_internal(identifier)) {
        if (pos == ambiguous) {
            return find_element_result<column_info>::ambiguous;
        }
        return columns_[*pos];
    }
    return {};
}

find_element_result<column_info const> relation_info::find(std::string_view identifier) const {
    if (auto pos = find_internal(identifier)) {
        if (pos == ambiguous) {
            return find_element_result<column_info const>::ambiguous;
        }
        return columns_[*pos];
    }
    return {};
}

find_element_result<column_info> relation_info::find(yugawara::storage::column const& column) {
    if (auto pos = find_internal(column)) {
        if (pos == ambiguous) {
            return find_element_result<column_info>::ambiguous;
        }
        return columns_[*pos];
    }
    return {};
}

find_element_result<column_info const> relation_info::find(yugawara::storage::column const& column) const {
    if (auto pos = find_internal(column)) {
        if (pos == ambiguous) {
            return find_element_result<column_info const>::ambiguous;
        }
        return columns_[*pos];
    }
    return {};
}

void relation_info::rebuild() {
    name_map_.clear();
    declaration_map_.clear();
    for (auto&& column : columns_) {
        column.next().reset();
    }
    for (std::size_t i = 0, n = columns_.size(); i < n; ++i) {
        build_internal(i);
    }
}

optional_ptr<column_info> relation_info::next(column_info const& column) {
    if (auto&& pos = column.next()) {
        return columns_[*pos];
    }
    return {};
}

optional_ptr<column_info const> relation_info::next(column_info const& column) const {
    if (auto&& pos = column.next()) {
        return columns_[*pos];
    }
    return {};
}

std::optional<relation_info::position_type> relation_info::find_internal(std::string_view identifier) const {
    if (auto it = name_map_.find(identifier); it != name_map_.end()) {
        return it.value();
    }
    return {};
}

std::optional<relation_info::position_type> relation_info::find_internal(yugawara::storage::column const& column) const {
    if (auto it = declaration_map_.find(std::addressof(column)); it != declaration_map_.end()) {
        return it.value();
    }
    return {};
}

column_info& relation_info::build_internal(position_type position) {
    auto&& target = columns_[position];
    if (auto&& id = target.identifier()) {
        if (auto [it, success] = name_map_.emplace(id.value(), position); !success) {
            auto pos = it.value();
            ::boost::dynamic_bitset<> saw { columns_.size() };
            while (true) {
                if (pos == ambiguous) {
                    break;
                }
                // FIXME: avoid cyclic reference
                auto&& current = columns_[pos];
                if (saw.test(pos)) {
                    current.next() = ambiguous;
                    break;
                }
                saw.set(pos);
                if (auto&& next = current.next()) {
                    pos = *next;
                } else {
                    next = position;
                    break;
                }
            }
        }
    }
    if (auto decl = target.declaration()) {
        if (auto [it, success] = declaration_map_.emplace(target.declaration().get(), position); !success) {
            it.value() = ambiguous;
        }
    }
    return target;

}

std::ostream& operator<<(std::ostream& out, relation_info const& value) {
    using ::takatori::util::print_support;
    return out << "column_info("
               << "declaration=" << print_support { value.declaration() } << ", "
               << "identifier=" << value.identifier() << ", "
               << "columns=" << value.columns() << ")";
}

} // namespace mizugaki::analyzer::details
