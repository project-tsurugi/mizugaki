#include "relation_info.h"

#include <numeric>

#include <takatori/util/downcast.h>
#include <takatori/util/exception.h>
#include <takatori/util/fail.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

namespace mizugaki::translator::details {

namespace descriptor = ::takatori::descriptor;

using ::takatori::util::fail;
using ::takatori::util::optional_ptr;
using ::takatori::util::string_builder;
using ::takatori::util::throw_exception;
using ::takatori::util::unsafe_downcast;

using ::shakujo::model::name::Name;
using ::shakujo::model::name::SimpleName;
using ::shakujo::model::name::QualifiedName;

relation_info::relation_info()
    : entity_(for_projection {})
{}

relation_info::relation_info(yugawara::storage::index const& index)
    : entity_(for_scan {
            std::addressof(index),
            {},
            build_column_map(index),
            decltype(for_scan::variable_map_) {},
    })
{}

relation_info::relation_info(
        ::yugawara::storage::index const& index,
        Name const& name,
        ::takatori::util::object_creator creator)
    : entity_(for_scan {
            std::addressof(index),
            std::addressof(name),
            build_column_map(index),
            build_variable_map(index, creator),
    })
{}

relation_info::relation_info(relation_info&& left, relation_info&& right)
    : entity_(for_join { build_relations(std::move(left), std::move(right) )})
{}

decltype(relation_info::for_scan::column_map_)
relation_info::build_column_map(yugawara::storage::index const& index) {
    decltype(for_scan::column_map_) result;
    auto&& cols = index.table().columns();
    result.reserve(index.table().columns().size());
    for (auto&& col : cols) {
        result.emplace(col.simple_name(), std::addressof(col));
    }
    return result;
}

static auto column_name(::yugawara::storage::column const& column) {
#if defined(NDEBUG)
    (void) column;
    return std::string_view {};
#else
    using ::takatori::util::string_builder;
    return string_builder {}
            << column.owner().simple_name()
            << "::"
            << column.simple_name()
            << string_builder::to_string;
#endif
}

decltype(relation_info::for_scan::variable_map_)
relation_info::build_variable_map(yugawara::storage::index const& index, ::takatori::util::object_creator creator) {
    decltype(for_scan::variable_map_) result;
    ::yugawara::binding::factory factory { creator };
    auto&& cols = index.table().columns();
    result.reserve(index.table().columns().size());
    for (auto&& col : cols) {
        result.emplace(std::addressof(col), factory.stream_variable(column_name(col)));
    }
    return result;
}

std::vector<relation_info> relation_info::build_relations(relation_info&& left, relation_info&& right) {
    std::vector<relation_info> result;
    if (left.entity_.index() == for_join::index) {
        auto&& rs = std::get<for_join>(left.entity_).relations_;
        result = std::move(rs);
    } else {
        result.emplace_back(std::move(left));
    }
    if (right.entity_.index() == for_join::index) {
        auto&& rs = std::get<for_join>(right.entity_).relations_;
        result.reserve(result.size() + rs.size());
        for (auto&& r : rs) {
            result.emplace_back(std::move(r));
        }
    } else {
        result.emplace_back(std::move(right));
    }
    return result;
}

optional_ptr<::yugawara::storage::index const> relation_info::table_index() const {
    if (auto const* entity = std::get_if<for_scan>(&entity_); entity != nullptr) {
        return optional_ptr { entity->table_index_ };
    }
    return {};
}

optional_ptr<::yugawara::storage::column const> relation_info::find_table_column(shakujo::model::name::SimpleName const& name) const {
    if (auto const* entity = std::get_if<for_scan>(&entity_); entity != nullptr) {
        auto&& map = entity->column_map_;
        if (auto it = map.find(name.token()); it != map.end()) {
            return optional_ptr { it->second };
        }
    }
    return {};
}

optional_ptr<descriptor::variable const> relation_info::find_variable(Name const& name) const {
    if (entity_.index() == for_join::index) {
        return find_variable_for_join(name);
    }
    if (name.kind() == SimpleName::tag) {
        return find_variable0(unsafe_downcast<SimpleName>(name));
    }
    if (name.kind() == QualifiedName::tag) {
        return find_variable0(unsafe_downcast<QualifiedName>(name));
    }
    fail();
}

optional_ptr<descriptor::variable const> relation_info::find_variable0(SimpleName const& name) const {
    switch (entity_.index()) {
        case for_projection::index: {
            auto&& entity = std::get<for_projection>(entity_);
            auto&& map = entity.variable_map_;
            if (auto it = map.find(name.token());
                    it != map.end()
                    && it.value() != for_projection::ambiguous) {
                // FIXME: report conflict
                return entity.variable_list_[it.value()].first;
            }
            return {};
        }
        case for_scan::index: {
            auto&& entity = std::get<for_scan>(entity_);
            auto&& cs = entity.column_map_;
            if (auto cit = cs.find(name.token());
                    cit != cs.end()
                    && cit.value() != for_scan::ambiguous) {
                // FIXME: report conflict
                auto&& vs = entity.variable_map_;
                if (auto vit = vs.find(cit.value()); vit != vs.end()) {
                    return optional_ptr { vit.value() };
                }
            }
            return {};
        }
        default:
            break;
    }
    fail(to_string_view(name.kind()));
}

optional_ptr<descriptor::variable const> relation_info::find_variable0(QualifiedName const& name) const {
    if (match_namespace(*name.qualifier())) {
        return find_variable0(*name.name());
    }
    return {};
}

optional_ptr<::takatori::descriptor::variable const> relation_info::find_variable(yugawara::storage::column const& column) const {
    switch (entity_.index()) {
        case for_scan::index: {
            auto&& entity = std::get<for_scan>(entity_);
            auto&& map = entity.variable_map_;
            if (auto it = map.find(std::addressof(column)); it != map.end()) {
                return it.value();
            }
            return {};
        }
        case for_join::index:
            return find_variable_for_join(column);
        case for_projection::index:
            return {};
    }
    fail();
}

static SimpleName const& last(Name const& name) {
    if (name.kind() == SimpleName::tag) {
        return unsafe_downcast<SimpleName>(name);
    }
    if (name.kind() == QualifiedName::tag) {
        return *unsafe_downcast<QualifiedName>(name).name();
    }
    fail();
}

static optional_ptr<Name const> qualifier(Name const& name) {
    if (name.kind() == SimpleName::tag) {
        return {};
    }
    if (name.kind() == QualifiedName::tag) {
        return optional_ptr { unsafe_downcast<QualifiedName>(name).qualifier() };
    }
    fail();
}

static bool is_suffix_of(Name const& a, Name const& b) {
    if (last(a) == last(b)) {
        auto qa = qualifier(a);
        auto qb = qualifier(b);
        if (!qa) {
            return true;
        }
        if (!qb) {
            return false;
        }
        return is_suffix_of(*qa, *qb);
    }
    return false;
}

bool relation_info::match_namespace(Name const& name) const {
    switch (entity_.index()) {
        case for_projection::index: {
            auto&& entity = std::get<for_projection>(entity_);
            if (entity.name_ == nullptr) {
                return false;
            }
            return is_suffix_of(name, *entity.name_);
        }
        case for_scan::index: {
            auto&& entity = std::get<for_scan>(entity_);
            if (entity.name_ == nullptr) {
                return false;
            }
            return is_suffix_of(name, *entity.name_);
        }
        default:
            break;
    }
    fail(to_string_view(name.kind()));
}

void relation_info::enumerate_variables(consumer_type const& consumer) const {
    switch (entity_.index()) {
        case for_projection::index: {
            auto&& entity = std::get<for_projection>(entity_);
            for (auto&& [v, n] : entity.variable_list_) {
                consumer(v, n);
            }
            return;
        }
        case for_scan::index: {
            auto&& entity = std::get<for_scan>(entity_);
            auto&& vs = entity.variable_map_;
            for (auto&& c : entity.table_index_->table().columns()) {
                if (auto it = vs.find(std::addressof(c)); it != vs.end()) {
                    consumer(it.value(), c.simple_name());
                } else {
                    throw_exception(std::logic_error(string_builder {}
                            << "inconsistent table column map: "
                            << c
                            << string_builder::to_string));
                }
            }
            return;
        }
        case for_join::index: {
            auto&& entity = std::get<for_join>(entity_);
            for (auto&& r : entity.relations_) {
                r.enumerate_variables(consumer);
            }
            return;
        }
    }
    fail();
}

std::size_t relation_info::count_variables() const {
    switch (entity_.index()) {
        case for_projection::index: {
            auto&& entity = std::get<for_projection>(entity_);
            return entity.variable_list_.size();
        }
        case for_scan::index: {
            auto&& entity = std::get<for_scan>(entity_);
            return entity.table_index_->table().columns().size();
        }
        case for_join::index: {
            auto&& entity = std::get<for_join>(entity_);
            return std::accumulate(
                    entity.relations_.begin(),
                    entity.relations_.end(),
                    std::size_t { 0 },
                    [](std::size_t count, relation_info const& b) {
                        return count + b.count_variables();
                    });
        }
    }
    fail();
}

template<class T>
optional_ptr<::takatori::descriptor::variable const> relation_info::find_variable_for_join(T const& key) const {
    auto&& entity = std::get<for_join>(entity_);
    optional_ptr<descriptor::variable const> result {};
    for (auto&& r : entity.relations_) {
        auto candidate = r.find_variable(key);
        if (candidate) {
            if (result) {
                // FIXME: report conflict
                return {};
            }
            result = std::move(candidate);
        }
    }
    return result;
}

optional_ptr<::shakujo::model::name::Name const> relation_info::relation_name() const {
    switch (entity_.index()) {
        case for_projection::index: {
            auto&& entity = std::get<for_projection>(entity_);
            return optional_ptr { entity.name_ };
        }
        case for_scan::index: {
            auto&& entity = std::get<for_scan>(entity_);
            return optional_ptr { entity.name_ };
        }
        case for_join::index: {
            return {};
        }
    }
    fail();
}

void relation_info::rename_relation(::shakujo::model::name::SimpleName const& name) {
    switch (entity_.index()) {
        case for_projection::index: {
            auto&& entity = std::get<for_projection>(entity_);
            entity.name_ = std::addressof(name);
            return;
        }
        case for_scan::index: {
            auto&& entity = std::get<for_scan>(entity_);
            entity.name_ = std::addressof(name);
            return;
        }
        case for_join::index: {
            throw_exception(std::domain_error("must not be a join scope"));
        }
    }
    fail();
}

void relation_info::rename_columns(shakujo::model::util::NodeList<SimpleName> const& names) {
    switch (entity_.index()) {
        case for_projection::index: {
            auto&& entity = std::get<for_projection>(entity_);
            if (names.size() != entity.variable_list_.size()) {
                throw_exception(std::domain_error("inconsistent column number"));
            }
            entity.variable_map_.clear();
            for (std::size_t i = 0, n = entity.variable_list_.size(); i < n; ++i) {
                entity.variable_list_[i].second = names[i]->token();
                auto [it, success] = entity.variable_map_.emplace(names[i]->token(), i);
                if (!success) {
                    it.value() = static_cast<std::size_t>(-1);
                }
            }
            return;
        }
        case for_scan::index: {
            auto&& entity = std::get<for_scan>(entity_);
            auto&& columns = entity.table_index_->table().columns();
            if (names.size() != columns.size()) {
                throw_exception(std::domain_error("inconsistent column number"));
            }
            entity.column_map_.clear();
            for (std::size_t i = 0, n = columns.size(); i < n; ++i) {
                auto [it, success] = entity.column_map_.emplace(names[i]->token(), std::addressof(columns[i]));
                if (!success) {
                    it.value() = nullptr;
                }
            }
            return;
        }
        case for_join::index: {
            throw_exception(std::domain_error("must not be a join scope"));
        }
    }
    fail();

}

void relation_info::add_column(descriptor::variable variable, std::string name) {
    if (entity_.index() != for_projection::index) {
        throw_exception(std::domain_error("must be a projection scope"));
    }
    auto&& entity = std::get<for_projection>(entity_);
    if (name.empty()) {
        entity.variable_list_.emplace_back(std::move(variable), std::string {});
    } else {
        auto index = entity.variable_list_.size();
        entity.variable_list_.emplace_back(std::move(variable), name);
        auto [it, success] = entity.variable_map_.emplace(std::move(name), index);
        if (!success) {
            it.value() = static_cast<std::size_t>(-1);
        }
    }
}

} // namespace mizugaki::translator::details
