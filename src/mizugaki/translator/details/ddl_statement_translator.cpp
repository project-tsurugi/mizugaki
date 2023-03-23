#include "ddl_statement_translator.h"

#include <takatori/scalar/immediate.h>

#include <takatori/statement/create_table.h>
#include <takatori/statement/create_index.h>
#include <takatori/statement/drop_table.h>
#include <takatori/statement/drop_index.h>
#include <takatori/statement/empty.h>

#include <takatori/util/assertion.h>
#include <takatori/util/downcast.h>
#include <takatori/util/fail.h>
#include <takatori/util/optional_ptr.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

#include <mizugaki/translator/util/statement_dispatch.h>

#include "scalar_expression_translator.h"
#include "type_translator.h"

namespace mizugaki::translator::details {

using translator_type = shakujo_translator::impl;
using result_type = shakujo_translator_result;
using code_type = shakujo_translator_code;

using ::takatori::util::fail;
using ::takatori::util::optional_ptr;
using ::takatori::util::string_builder;
using ::takatori::util::unsafe_downcast;

namespace ddl = ::shakujo::model::statement::ddl;

namespace {

class engine {
public:
    explicit engine(translator_type& translator) noexcept :
        translator_{translator}
    {}

    [[noreturn]] result_type operator()(::shakujo::model::statement::Statement const& node) const {
        fail(string_builder {}
                << "must be a DDL statement: "
                << node.kind()
                << string_builder::to_string);
    }

    result_type operator()(ddl::CreateTableStatement const& node) {
        auto schema = find_schema_from_element_name(node.table());
        if (!schema) return {};

        auto existing = translator_.find_table(*node.table());
        if (existing) {
            auto&& attrs = node.attributes();
            if (attrs.find(ddl::CreateTableStatement::Attribute::IF_NOT_EXISTS) != attrs.end()) {
                // target table is already present; we return empty statement for "create table if not exists"
                auto result = create<::takatori::statement::empty>();
                result->region() = translator_.region(node.region());
                return { std::move(result) };
            }
            report(code_type::duplicate_table, *last_name(node.table()), string_builder {}
                    << "table `" << *node.table() << "' is already defined");
            return {};
        }

        ::takatori::util::reference_vector<::yugawara::storage::column> columns {};
        columns.reserve(node.columns().size());
        optional_ptr<::yugawara::storage::column> primary_key_column {};
        for (auto const* source_column : node.columns()) {
            using ColumnAttr = ddl::CreateTableStatement::Column::Attribute;
            auto&& attrs = source_column->attributes();

            type_translator type_translator { translator_ };
            auto type = type_translator.process(*source_column->type());
            if (!type) return {};

            ::yugawara::variable::criteria criteria {
                    ::yugawara::variable::nullity { attrs.find(ColumnAttr::NOT_NULL) == attrs.end() }
            };

            ::yugawara::storage::column_value default_value {};
            if (auto const* source_value = source_column->value()) {
                scalar_expression_translator exp_translator { translator_ };
                auto result = exp_translator.process(*source_value, { translator_.options(), {} });
                if (!result) return {};

                if (result->kind() != ::takatori::scalar::immediate::tag) {
                    report(shakujo_translator_code::invalid_default_value, *source_value,
                            string_builder {} << "column default value must be a literal");
                    return {};
                }
                default_value = unsafe_downcast<::takatori::scalar::immediate>(*result).shared_value();
            }

            auto&& target = columns.emplace_back(
                    source_column->name()->token(),
                    std::move(type),
                    std::move(criteria),
                    std::move(default_value),
                    ::yugawara::storage::column::feature_set_type {});

            if (attrs.find(ColumnAttr::PRIMARY_KEY) != attrs.end()) {
                if (primary_key_column) {
                    report(shakujo_translator_code::invalid_default_value, *source_column->name(),
                            string_builder {} << "primary key definition must be upto one");
                    return {};
                }
                primary_key_column = target;
            }
        }

        auto new_table = std::make_shared<::yugawara::storage::table>(
                std::nullopt,
                last_name(node.table())->token(),
                std::move(columns));


        std::vector<::yugawara::storage::index::key> primary_keys {};
        if (!node.primary_keys().empty()) {
            if (primary_key_column) {
                report(shakujo_translator_code::invalid_default_value, *node.primary_keys().front()->name(),
                        string_builder {} << "primary key definition must be upto one");
                return {};
            }
            primary_keys.reserve(node.primary_keys().size());
            auto&& table_columns = new_table->columns();
            for (auto const* source_key : node.primary_keys()) {
                auto&& column_name = source_key->name()->token();
                auto found = std::find_if(
                        table_columns.begin(),
                        table_columns.end(),
                        [&](auto&& column) { return column.simple_name() == column_name; });
                if (found == table_columns.end()) {
                    report(shakujo_translator_code::column_not_found, *source_key->name(),
                            string_builder {}
                                    << "primary key column \"" << column_name << "\" is not found");
                    return {};
                }
                auto direction = convert_direction(source_key->direction());
                primary_keys.emplace_back(*found, direction);
            }
        } else if (primary_key_column) {
            primary_keys.reserve(1);
            primary_keys.emplace_back(*primary_key_column);
        }
        auto new_primary_index = std::make_shared<::yugawara::storage::index>(
                std::nullopt,
                new_table,
                std::string {},
                std::move(primary_keys),
                std::vector<::yugawara::storage::index::column_ref> {},
                ::yugawara::storage::index_feature_set {
                    ::yugawara::storage::index_feature::primary,
                });

        using ::takatori::statement::create_table;
        auto result = create<create_table>(
                build(std::move(schema), qualifier(node.table())),
                build(std::move(new_table), last_name(node.table())),
                build(std::move(new_primary_index), node.table()));
        result->region() = translator_.region(node.region());

        return { std::move(result) };
    }

    result_type operator()(ddl::CreateIndexStatement const& node) {
        auto schema = find_schema_from_element_name(node.table());
        if (!schema) return {};

        auto table = translator_.find_table(*node.table());
        if (!table) {
            report(code_type::table_not_found, *last_name(node.table()), string_builder {}
                    << "table `" << *node.table() << "' is not found");
            return {};
        }

        std::string name {};
        if (node.index() != nullptr) {
            auto existing = translator_.find_index(*node.index());
            auto&& attrs = node.attributes();
            if (existing) {
                if (attrs.find(ddl::CreateIndexStatement::Attribute::IF_NOT_EXISTS) != attrs.end()) {
                    // target index is already present; we return empty statement for "create index if not exists"
                    auto result = create<::takatori::statement::empty>();
                    result->region() = translator_.region(node.region());
                    return { std::move(result) };
                }
                report(code_type::duplicate_index, *last_name(node.index()), string_builder {}
                        << "index `" << *node.index() << "' is already defined");
                return {};
            }
            name = last_name(node.index())->token();
        }

        std::vector<::yugawara::storage::index::key> index_keys {};
        index_keys.reserve(node.columns().size());
        auto&& table_columns = table->table().columns();
        for (auto const* source_key : node.columns()) {
            auto&& column_name = source_key->name()->token();
            auto found = std::find_if(
                    table_columns.begin(),
                    table_columns.end(),
                    [&](auto&& column) { return column.simple_name() == column_name; });
            if (found == table_columns.end()) {
                report(shakujo_translator_code::column_not_found, *source_key->name(),
                        string_builder {}
                                << "index key column \"" << column_name << "\" is not found in \""
                                << table->table().simple_name() << "\"");
                return {};
            }
            auto direction = convert_direction(source_key->direction());
            index_keys.emplace_back(*found, direction);
        }
        auto new_index = std::make_shared<::yugawara::storage::index>(
                std::nullopt,
                table->shared_table(),
                std::move(name),
                std::move(index_keys),
                std::vector<::yugawara::storage::index::column_ref> {},
                ::yugawara::storage::index_feature_set {});

        using ::takatori::statement::create_index;
        auto result = create<create_index>(
                build(std::move(schema), qualifier(node.table())),
                build(std::move(new_index), std::addressof(node)));
        result->region() = translator_.region(node.region());

        return { std::move(result) };
    }

    result_type operator()(ddl::DropTableStatement const& node) {
        auto schema = find_schema_from_element_name(node.table());
        if (!schema) return {};

        auto index = translator_.find_table(*node.table());
        if(!index) {
            if (node.attributes().find(ddl::DropTableStatement::Attribute::IF_EXISTS) != node.attributes().end()) {
                // target table is absent; we return empty statement for "drop if exists"
                auto result = create<::takatori::statement::empty>();
                result->region() = translator_.region(node.region());
                return { std::move(result) };
            }
            report(code_type::table_not_found, *last_name(node.table()), string_builder {}
                    << "table `" << *node.table() << "' is not found");
            return {};
        }
        if (!index) return {};

        using ::takatori::statement::drop_table;
        auto result = create<drop_table>(
                build(std::move(schema), last_name(node.table())),
                build(index->shared_table(), qualifier(node.table())));
        result->region() = translator_.region(node.region());

        return { std::move(result) };
    }

    result_type operator()(ddl::DropIndexStatement const& node) {
        auto schema = find_schema_from_element_name(node.index());
        if (!schema) return {};

        auto index = translator_.find_index(*node.index());
        if(!index) {
            if (node.attributes().find(ddl::DropIndexStatement::Attribute::IF_EXISTS) != node.attributes().end()) {
                // target index is absent; we return empty statement for "drop if exists"
                auto result = create<::takatori::statement::empty>();
                result->region() = translator_.region(node.region());
                return { std::move(result) };
            }
            report(code_type::index_not_found, *last_name(node.index()), string_builder {}
                    << "index `" << *node.index() << "' is not found");
            return {};
        }
        if (!index) return {};

        using ::takatori::statement::drop_index;
        auto result = create<drop_index>(
                build(std::move(schema), last_name(node.index())),
                build(std::move(index), qualifier(node.index())));
        result->region() = translator_.region(node.region());

        return { std::move(result) };
    }

private:
    translator_type& translator_;

    result_type report(code_type code, ::shakujo::model::Node const& node, string_builder&& builder) {
        translator_.diagnostics().emplace_back(
                code,
                builder << string_builder::to_string,
                translator_.region(node.region()));
        return {};
    }

    template<class T>
    [[nodiscard]] std::vector<T> new_vector(std::size_t capacity = 0) const {
        std::vector<T> result {};
        result.reserve(capacity);
        return result;
    }

    template<class T, class... Args>
    [[nodiscard]] std::unique_ptr<T> create(Args&&... args) const {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<class T>
    [[nodiscard]] auto build(T&& source, ::shakujo::model::Node const* node = {})
            -> decltype(std::declval<::yugawara::binding::factory>()(std::declval<T&>())) {
        auto r = factory()(std::forward<T>(source));
        if (node != nullptr) {
            r.region() = translator_.region(node->region());
        }
        return r;
    }

    [[nodiscard]] ::yugawara::binding::factory factory() const noexcept {
        return {};
    }


    [[nodiscard]] ::shakujo::model::name::Name const* qualifier(::shakujo::model::name::Name const* name) {
        using k = ::shakujo::model::name::NameKind;
        switch (name->kind()) {
            case k::SIMPLE_NAME:
                return nullptr;
            case k::QUALIFIED_NAME:
                return unsafe_downcast<::shakujo::model::name::QualifiedName>(name)->qualifier();
        }
        std::abort();
    }

    [[nodiscard]] ::shakujo::model::name::SimpleName const* last_name(::shakujo::model::name::Name const* name) {
        using k = ::shakujo::model::name::NameKind;
        switch (name->kind()) {
            case k::SIMPLE_NAME:
                return unsafe_downcast<::shakujo::model::name::SimpleName>(name);
            case k::QUALIFIED_NAME:
                return unsafe_downcast<::shakujo::model::name::QualifiedName>(name)->name();
        }
        std::abort();
    }

    [[nodiscard]] std::shared_ptr<::yugawara::schema::declaration const> find_schema_from_element_name(
            ::shakujo::model::name::Name const*) {
        // FIXME: singleton schema
        return translator_.options().shared_schema();
    }

    [[nodiscard]] std::shared_ptr<::yugawara::storage::index const> find_table(::shakujo::model::name::Name const* name) {
        auto index = translator_.find_table(*name);
        if (!index) {
            report(code_type::table_not_found, *name, string_builder {}
                    << "table `" << *name << "' is not found");
            return {};
        }
        return index;
    }

    template<class T>
    [[nodiscard]] static ::yugawara::storage::index::key::direction_type convert_direction(T direction) noexcept {
        using k = T;
        switch (direction) {
            case k::DONT_CARE:
            case k::ASCENDANT:
                return ::yugawara::storage::index::key::direction_type::ascendant;
            case k::DESCENDANT:
                return ::yugawara::storage::index::key::direction_type::descendant;
        }
        std::abort();
    }
};

} // namespace

ddl_statement_translator::ddl_statement_translator(translator_type& translator) noexcept :
    translator_ { translator }
{}

result_type ddl_statement_translator::process(::shakujo::model::statement::Statement const& node) {
    engine e { translator_ };
    return util::dispatch(e, node);
}

bool ddl_statement_translator::is_supported(shakujo::model::statement::Statement const& node) {
    using k = shakujo::model::statement::StatementKind;
    switch (node.kind()) {
        case k::CREATE_TABLE_STATEMENT:
        case k::CREATE_INDEX_STATEMENT:
        case k::DROP_TABLE_STATEMENT:
        case k::DROP_INDEX_STATEMENT:
            return true;
        default:
            return false;
    }
    std::abort();
}

} // namespace mizugaki::translator::details
