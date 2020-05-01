#pragma once

#include <functional>
#include <string_view>
#include <variant>

#include <tsl/hopscotch_map.h>

#include <takatori/descriptor/variable.h>

#include <takatori/util/object_creator.h>
#include <takatori/util/optional_ptr.h>

#include <yugawara/storage/index.h>
#include <yugawara/storage/column.h>

#include <shakujo/model/name/Name.h>
#include <shakujo/model/name/SimpleName.h>
#include <shakujo/model/name/QualifiedName.h>

#include <shakujo/model/util/NodeList.h>

namespace mizugaki::translator::details {

class relation_info {
public:
    using consumer_type = std::function<void(::takatori::descriptor::variable const& variable, std::string_view name)>;

    /**
     * @brief creates a new instance for projection.
     */
    relation_info();

    /**
     * @brief creates a new instance for scan, without building stream variables.
     * @param index the source relation
     */
    explicit relation_info(::yugawara::storage::index const& index);

    /**
     * @brief creates a new instance for scan.
     * @param index the source relation
     * @param name the relation name
     * @param creator the object creator
     */
    explicit relation_info(
            ::yugawara::storage::index const& index,
            ::shakujo::model::name::Name const& name,
            ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance for join.
     * @param left left relation
     * @param right right relation
     */
    explicit relation_info(relation_info&& left, relation_info&& right);

    [[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::index const> table_index() const;

    [[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::column const> find_table_column(::shakujo::model::name::SimpleName const& name) const;

    [[nodiscard]] ::takatori::util::optional_ptr<::takatori::descriptor::variable const> find_variable(::shakujo::model::name::Name const& name) const;

    [[nodiscard]] ::takatori::util::optional_ptr<::takatori::descriptor::variable const> find_variable(::yugawara::storage::column const& column) const;

    void enumerate_variables(consumer_type const& consumer) const;

    [[nodiscard]] std::size_t count_variables() const;

    [[nodiscard]] ::takatori::util::optional_ptr<::shakujo::model::name::Name const> relation_name() const;

    void rename_relation(::shakujo::model::name::SimpleName const& name);

    void rename_columns(::shakujo::model::util::NodeList<::shakujo::model::name::SimpleName> const& names);

    void add_column(::takatori::descriptor::variable variable, std::string name);

private:
    struct for_projection {
        static constexpr std::size_t index = 0;
        static constexpr std::size_t ambiguous = static_cast<std::size_t>(-1);
        ::shakujo::model::name::SimpleName const* name_;
        std::vector<std::pair<::takatori::descriptor::variable, std::string>> variable_list_;
        ::tsl::hopscotch_map<
                std::string,
                decltype(variable_list_)::size_type,
                std::hash<std::string_view>,
                std::equal_to<>> variable_map_;
    };
    struct for_scan {
        static constexpr std::size_t index = for_projection::index + 1;
        static constexpr std::nullptr_t ambiguous = nullptr;
        ::yugawara::storage::index const* table_index_;
        ::shakujo::model::name::Name const* name_;
        ::tsl::hopscotch_map<
                std::string_view,
                ::yugawara::storage::column const*,
                std::hash<std::string_view>,
                std::equal_to<>> column_map_;
        ::tsl::hopscotch_map<::yugawara::storage::column const*, ::takatori::descriptor::variable> variable_map_;
    };
    struct for_join {
        static constexpr std::size_t index = for_scan::index + 1;
        std::vector<relation_info> relations_;
    };
    std::variant<for_projection, for_scan, for_join> entity_;

    [[nodiscard]] static decltype(for_scan::column_map_) build_column_map(
            ::yugawara::storage::index const& index);
    [[nodiscard]] static decltype(for_scan::variable_map_) build_variable_map(
            ::yugawara::storage::index const& index,
            ::takatori::util::object_creator creator);
    [[nodiscard]] static std::vector<relation_info> build_relations(relation_info&& left, relation_info&& right);

    template<class T>
    [[nodiscard]] ::takatori::util::optional_ptr<::takatori::descriptor::variable const> find_variable_for_join(T const& key) const;

    [[nodiscard]] ::takatori::util::optional_ptr<::takatori::descriptor::variable const> find_variable0(::shakujo::model::name::SimpleName const& name) const;
    [[nodiscard]] ::takatori::util::optional_ptr<::takatori::descriptor::variable const> find_variable0(::shakujo::model::name::QualifiedName const& name) const;
    [[nodiscard]] bool match_namespace(::shakujo::model::name::Name const& name) const;
};

} // namespace mizugaki::translator::details
