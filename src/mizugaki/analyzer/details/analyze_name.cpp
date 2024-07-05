#include <mizugaki/analyzer/details/analyze_name.h>

#include <functional>
#include <variant>

#include <takatori/descriptor/variable.h>

#include <takatori/scalar/variable_reference.h>

#include <takatori/util/downcast.h>
#include <takatori/util/optional_ptr.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

#include <yugawara/schema/declaration.h>
#include <yugawara/schema/catalog.h>

#include <yugawara/storage/table.h>

#include <mizugaki/analyzer/details/relation_info.h>

#include "symbol_kind.h"
#include "find_symbol_result.h"
#include "name_print_support.h"

namespace mizugaki::analyzer::details {

namespace tscalar = ::takatori::scalar;

namespace descriptor = ::takatori::descriptor;
using schema_decl = ::yugawara::schema::declaration;
using catalog_decl = ::yugawara::schema::catalog;
using table_decl = ::yugawara::storage::table;
using index_decl = ::yugawara::storage::index;
using relation_decl = ::yugawara::storage::relation;

using ::takatori::util::optional_ptr;
using ::takatori::util::string_builder;
using ::takatori::util::unsafe_downcast;

namespace {

struct saw_error_t {
    template<class... Alternatives>
    [[nodiscard]] bool operator()(std::variant<Alternatives...> const& alternatives) const noexcept {
        return std::holds_alternative<saw_error_t>(alternatives);
    }
};

struct not_found_t {
    template<class... Alternatives>
    [[nodiscard]] bool operator()(std::variant<Alternatives...> const& alternatives) const noexcept {
        return std::holds_alternative<not_found_t>(alternatives);
    }
};

constexpr saw_error_t const saw_error {};
constexpr not_found_t const not_found {};

using symbol_info = std::variant<
        saw_error_t,
        not_found_t,
        descriptor::variable,
        std::reference_wrapper<relation_info const>,
        std::reference_wrapper<relation_decl const>,
        std::shared_ptr<schema_decl const>,
        std::reference_wrapper<catalog_decl const>,
        schema_element<table_decl>,
        schema_element<index_decl>>;

class engine {
public:
    engine(analyzer_context& context, query_scope const& scope) noexcept :
        context_ { context },
        scope_ { scope }
    {}

    [[nodiscard]] std::unique_ptr<tscalar::expression> find_variable(ast::name::name const& name) {
        auto alternatives = find_symbol(scope_, name, {
                symbol_kind::column_variable,
                symbol_kind::field_variable,
                symbol_kind::schema_variable,
        });
        if (saw_error(alternatives)) {
            return {};
        }
        if (auto v = optional_ptr { std::get_if<descriptor::variable>(&alternatives) }) {
            return context_.create<tscalar::variable_reference>(
                    name.region(),
                    std::move(*v));
        }
        context_.report(sql_analyzer_code::variable_not_found,
                string_builder {}
                        << "'" << print_support { name } << "' is not found"
                        << string_builder::to_string,
                name.region());
        return {};
    }

    [[nodiscard]] optional_ptr<relation_info const> find_relation_info(ast::name::name const& name) {
        auto alternatives = find_symbol(scope_, name, { symbol_kind::relation_info });
        if (saw_error(alternatives)) {
            return {};
        }
        if (auto v = unwrap(std::get_if<std::reference_wrapper<relation_info const>>(&alternatives))) {
            return v;
        }
        context_.report(sql_analyzer_code::symbol_not_found,
                string_builder {}
                        << "'" << print_support { name } << "' is not found"
                        << string_builder::to_string,
                name.region());
        return {};
    }

    [[nodiscard]] optional_ptr<relation_decl const> find_relation_decl(ast::name::name const& name) {
        auto alternatives = find_symbol(scope_, name, { symbol_kind::relation_decl });
        if (saw_error(alternatives)) {
            return {};
        }
        if (auto v = unwrap(std::get_if<std::reference_wrapper<relation_decl const>>(&alternatives))) {
            return v;
        }
        context_.report(sql_analyzer_code::table_not_found,
                string_builder {}
                        << "'" << print_support { name } << "' is not found"
                        << string_builder::to_string,
                name.region());
        return {};
    }

    [[nodiscard]] std::optional<schema_element<table_decl>> find_table_decl(ast::name::name const& name, bool mandatory) {
        auto alternatives = find_symbol(scope_, name, { symbol_kind::table_decl }, mandatory);
        if (saw_error(alternatives)) {
            return {};
        }
        if (auto v = std::get_if<schema_element<table_decl>>(&alternatives)) {
            return std::move(*v);
        }
        if (mandatory) {
            context_.report(sql_analyzer_code::table_not_found,
                    string_builder {}
                            << "'" << print_support { name } << "' is not found"
                            << string_builder::to_string,
                    name.region());
        }
        return {};
    }

    [[nodiscard]] std::optional<schema_element<index_decl>> find_index_decl(ast::name::name const& name, bool mandatory) {
        auto alternatives = find_symbol(scope_, name, { symbol_kind::index_decl }, mandatory);
        if (saw_error(alternatives)) {
            return {};
        }
        if (auto v = std::get_if<schema_element<index_decl>>(&alternatives)) {
            return std::move(*v);
        }
        if (mandatory) {
            context_.report(sql_analyzer_code::index_not_found,
                    string_builder {}
                            << "'" << print_support { name } << "' is not found"
                            << string_builder::to_string,
                    name.region());
        }
        return {};
    }

    [[nodiscard]] std::shared_ptr<schema_decl const> find_schema_decl(ast::name::name const& name, bool mandatory) {
        auto alternatives = find_symbol(scope_, name, { symbol_kind::schema_decl }, mandatory);
        if (saw_error(alternatives)) {
            return {};
        }
        if (auto v = std::get_if<std::shared_ptr<schema_decl const>>(&alternatives)) {
            return std::move(*v);
        }
        if (mandatory) {
            context_.report(sql_analyzer_code::schema_not_found,
                    string_builder {}
                            << "'" << print_support { name } << "' is not found"
                            << string_builder::to_string,
                    name.region());
        }
        return {};
    }

    [[nodiscard]] static ast::common::chars const& to_identifier(
            ast::name::simple const& name,
            bool lowercase_regular_identifiers,
            ast::common::chars& buffer,
            bool always_to_buffer = false) {
        auto&& id = name.identifier();
        if (name.identifier_kind() == ast::name::identifier_kind::regular && lowercase_regular_identifiers) {
            buffer.resize(id.size());
            for (std::size_t i = 0, n = id.size(); i < n; ++i) {
                buffer[i] = static_cast<char>(std::tolower(id[i])); // FIXME: using the default locale
            }
            return buffer;
        }
        if (always_to_buffer) {
            buffer.assign(id.begin(), id.end());
            return buffer;
        }
        return id;
    }

private:
    analyzer_context& context_;
    query_scope const& scope_;

    template<class T>
    [[nodiscard]] auto to_descriptor(T&& t) const {
        ::yugawara::binding::factory f {};
        return f(std::forward<T>(t));
    }

    template<class T>
    [[nodiscard]] static optional_ptr<T> unwrap(std::reference_wrapper<T> const* ptr) noexcept {
        if (ptr != nullptr) {
            return ptr->get();
        }
        return {};
    }

    [[nodiscard]] ast::common::chars const& to_identifier(ast::name::simple const& name, ast::common::chars& buffer) {
        return to_identifier(name, context_.options()->lowercase_regular_identifiers(), buffer);
    }

    [[nodiscard]] symbol_info find_symbol(
            query_scope const& scope,
            ast::name::name const& name,
            symbol_kind_set targets,
            bool report_not_found = true) {
        if (auto q = name.optional_qualifier()) {
            symbol_kind_set qualifier_targets {};
            for (auto k : targets) {
                qualifier_targets |= compute_qualifier_symbol_kind(k);
            }
            auto qualifier = find_symbol(scope, *q, qualifier_targets, report_not_found);
            if (saw_error(qualifier) || not_found(qualifier)) {
                return qualifier;
            }
            auto&& qualified_name = unsafe_downcast<ast::name::qualified>(name);
            return find_symbol_qualified(scope, std::move(qualifier), qualified_name, targets, report_not_found);
        }
        return find_symbol_primary(scope, unsafe_downcast<ast::name::simple>(name), targets, report_not_found);
    }

    [[nodiscard]] symbol_kind_set compute_qualifier_symbol_kind(symbol_kind target) {
        using kind = symbol_kind;
        switch (target) {
            case kind::column_variable:
                return { kind::relation_info };
            case kind::field_variable:
                return { kind::column_variable, kind::field_variable, kind::schema_variable };
            case kind::schema_variable:
                return { kind::schema_decl };
            case kind::relation_info:
                return { kind::schema_decl };
            case kind::relation_decl:
                return { kind::schema_decl };
            case kind::schema_decl:
                return { kind::catalog_decl };
            case kind::catalog_decl:
                return {};
            case kind::table_decl:
                return { kind::schema_decl };
            case kind::index_decl:
                return { kind::schema_decl };
        }
        context_.report(sql_analyzer_code::unknown,
                string_builder {}
                    << "unexpected symbol kind: "
                    << target
                    << string_builder::to_string,
                ast::node_region {});
        return {};
    }

    [[nodiscard]] symbol_info find_symbol_primary( // NOLINT(*-function-cognitive-complexity)
            query_scope const& scope,
            ast::name::simple const& name,
            symbol_kind_set targets,
            bool report_not_found = true) {
        // 1-1. find relation ref in query
        if (targets.contains(symbol_kind::relation_info)) {
            auto r = find_relation_info_in_query(scope, name);
            if (auto&& v = r.found()) {
                return *v;
            }
            if (r.is_error()) {
                return saw_error; // propagate error
            }
        }

        // 1-2. find column ref in query
        if (targets.contains(symbol_kind::column_variable)) {
            auto r = find_variable_in_query(scope, name);
            if (auto&& v = r.found()) {
                return *v;
            }
            if (r.is_error()) {
                return saw_error; // propagate error
            }
        }

        // FIXME: 2. find column also from main query
        // FIXME: 3. find parameter variables
        // FIXME: 4. find session variables

        // 5-1. find variables declared from schema search path
        if (targets.contains(symbol_kind::schema_variable)) {
            for (auto&& s : context_.options()->schema_search_path().elements()) {
                auto r = find_variable_in_schema(*s, name);
                if (!not_found(r)) {
                    return r;
                }
            }
        }

        // 5-2. find relation declared from schema search path
        if (targets.contains(symbol_kind::relation_decl)) {
            for (auto&& s : context_.options()->schema_search_path().elements()) {
                auto r = find_relation_decl_in_schema(*s, name);
                if (auto&& v = r.found()) {
                    return { *v };
                }
                if (r.is_error()) {
                    return saw_error;
                }
            }
        }

        // 5-3. find table declared from schema search path
        if (targets.contains(symbol_kind::table_decl)) {
            for (auto&& s : context_.options()->schema_search_path().elements()) {
                auto r = find_table_decl_in_schema(*s, name);
                if (auto&& v = r.found()) {
                    return std::make_pair(s, std::move(v));
                }
                if (r.is_error()) {
                    return saw_error;
                }
            }
        }

        // 5-4. find index declared from schema search path
        if (targets.contains(symbol_kind::index_decl)) {
            for (auto&& s : context_.options()->schema_search_path().elements()) {
                auto r = find_index_decl_in_schema(*s, name);
                if (auto&& v = r.found()) {
                    return std::make_pair(s, std::move(v));
                }
                if (r.is_error()) {
                    return saw_error;
                }
            }
        }

        // 6. find schema from the default catalog
        if (targets.contains(symbol_kind::schema_decl)) {
            auto r = find_schema_in_catalog(context_.options()->catalog(), name);
            if (auto v = r.found()) {
                return v;
            }
            if (r.is_error()) {
                return saw_error;
            }
        }

        // 7. find catalog in the system
        if (targets.contains(symbol_kind::catalog_decl)) {
            if (auto r = find_catalog_in_system(name)) {
                if (auto&& v = r.found()) {
                    return { *v };
                }
            } else {
                return saw_error;
            }
        }

        // not found
        if (report_not_found) {
            report_symbol_not_found(name, targets);
            return saw_error;
        }
        return not_found;
    }

    [[nodiscard]] symbol_info find_symbol_qualified( // NOLINT(*-function-cognitive-complexity)
            query_scope const& scope,
            symbol_info qualifier,
            ast::name::qualified const& qualified_name,
            symbol_kind_set targets,
            bool report_not_found = true) {
        if (saw_error(qualifier)) {
            return saw_error;
        }
        if (not_found(qualifier)) {
            return not_found;
        }

        // qualifier is variable
        if (auto q = optional_ptr { std::get_if<descriptor::variable>(&qualifier) }) {
            targets &= { symbol_kind::field_variable };
            if (targets.contains(symbol_kind::field_variable)) {
                // FIXME: impl field reference
                (void) q;
                context_.report(sql_analyzer_code::unsupported_feature,
                        "the field reference is not supported",
                        qualified_name.last_name().region());
                return saw_error;
            }
        }

        // qualifier is relation info
        if (auto q = unwrap(std::get_if<std::reference_wrapper<relation_info const>>(&qualifier))) {
            targets &= { symbol_kind::column_variable };
            if (targets.contains(symbol_kind::column_variable)) {
                auto r = find_column_in_relation(*q, qualified_name.last_name());
                if (auto&& v = r.found()) {
                    return { *v };
                }
                if (r.is_error()) {
                    return saw_error;
                }
            }
        }

        // qualifier is schema decl
        if (auto q = std::get_if<std::shared_ptr<schema_decl const>>(&qualifier)) {
            targets &= {
                    symbol_kind::schema_variable,
                    symbol_kind::relation_info,
                    symbol_kind::relation_decl,
                    symbol_kind::table_decl,
                    symbol_kind::index_decl,
            };
            if (targets.contains(symbol_kind::schema_variable)) {
                auto r = find_variable_in_schema(**q, qualified_name.last_name());
                if (!not_found(r)) {
                    return r;
                }
            }
            if (targets.contains(symbol_kind::relation_info)) {
                auto r = find_relation_info_in_schema(scope, **q, qualified_name.last_name());
                if (auto&& v = r.found()) {
                    return { *v };
                }
                if (r.is_error()) {
                    return saw_error;
                }
            }
            if (targets.contains(symbol_kind::relation_decl)) {
                auto r = find_relation_decl_in_schema(**q, qualified_name.last_name());
                if (auto&& v = r.found()) {
                    return { *v };
                }
                if (r.is_error()) {
                    return saw_error;
                }
            }
            if (targets.contains(symbol_kind::table_decl)) {
                auto r = find_table_decl_in_schema(**q, qualified_name.last_name());
                if (auto&& v = r.found()) {
                    return std::make_pair(std::move(*q), std::move(v));
                }
                if (r.is_error()) {
                    return saw_error;
                }
            }
            if (targets.contains(symbol_kind::index_decl)) {
                auto r = find_index_decl_in_schema(**q, qualified_name.last_name());
                if (auto&& v = r.found()) {
                    return std::make_pair(std::move(*q), std::move(v));
                }
                if (r.is_error()) {
                    return saw_error;
                }
            }
        }
        if (auto q = unwrap(std::get_if<std::reference_wrapper<catalog_decl const>>(&qualifier))) {
            targets &= { symbol_kind::schema_decl };
            if (targets.contains(symbol_kind::schema_decl)) {
                auto r = find_schema_in_catalog(*q, qualified_name.last_name());
                if (auto v = r.found()) {
                    return v;
                }
                if (r.is_error()) {
                    return saw_error;
                }
            }
        }

        // not found
        if (report_not_found) {
            report_symbol_not_found(qualified_name, targets);
            return saw_error;
        }
        return not_found;
    }

    void report_symbol_not_found(
            ast::name::name const& name,
            symbol_kind_set targets) {
        if (targets.size() != 1) {
            constexpr symbol_kind_set variables {
                    symbol_kind::column_variable,
                    symbol_kind::field_variable,
                    symbol_kind::schema_variable,
            };
            if ((targets - variables).empty()) {
                context_.report(sql_analyzer_code::variable_not_found,
                        string_builder {}
                                << "variable \"" << print_support { name } << "\" is not found"
                                << ", searching for " << targets
                                << string_builder::to_string,
                        name.last_name().region());
                return;
            }
            context_.report(sql_analyzer_code::symbol_not_found,
                    string_builder {}
                            << "symbol \"" << print_support { name } << "\" is not found"
                            << ", searching for " << targets
                            << string_builder::to_string,
                    name.last_name().region());
            return;
        }
        auto target = *targets.begin();
        switch (target) {
            case symbol_kind::column_variable:
                context_.report(sql_analyzer_code::column_not_found,
                        string_builder {}
                                << "column \"" << print_support { name } << "\" is not found"
                                << string_builder::to_string,
                        name.last_name().region());
                break;
            case symbol_kind::field_variable:
                context_.report(sql_analyzer_code::variable_not_found,
                        string_builder {}
                                << "variable \"" << print_support { name } << "\" is not found"
                                << string_builder::to_string,
                        name.last_name().region());
                break;
            case symbol_kind::schema_variable:
                context_.report(sql_analyzer_code::variable_not_found,
                        string_builder {}
                                << "variable \"" << print_support { name } << "\" is not found"
                                << string_builder::to_string,
                        name.last_name().region());
                break;
            case symbol_kind::schema_decl:
                context_.report(sql_analyzer_code::schema_not_found,
                        string_builder {}
                                << "schema \"" << print_support { name } << "\" is not found"
                                << string_builder::to_string,
                        name.last_name().region());
                break;
            case symbol_kind::catalog_decl:
                context_.report(sql_analyzer_code::catalog_not_found,
                        string_builder {}
                                << "catalog \"" << print_support { name } << "\" is not found"
                                << string_builder::to_string,
                        name.last_name().region());
                break;
            case symbol_kind::relation_info:
            case symbol_kind::relation_decl: // FIXME: for views
            case symbol_kind::table_decl:
                context_.report(sql_analyzer_code::table_not_found,
                        string_builder {}
                                << "table \"" << print_support { name } << "\" is not found"
                                << string_builder::to_string,
                        name.last_name().region());
                break;
            case symbol_kind::index_decl:
                context_.report(sql_analyzer_code::index_not_found,
                        string_builder {}
                                << "index \"" << print_support { name } << "\" is not found"
                                << string_builder::to_string,
                        name.last_name().region());
                break;
            default:
                context_.report(sql_analyzer_code::symbol_not_found,
                        string_builder {}
                                << "symbol \"" << print_support { name } << "\" is not found"
                                << ", searching for " << target
                                << string_builder::to_string,
                        name.last_name().region());
                break;
        }
    }

    [[nodiscard]] find_symbol_result<relation_info const&> find_relation_info_in_schema(
            query_scope const& scope,
            schema_decl const& schema,
            ast::name::simple const& name) {
        if (auto decl = find_relation_decl_in_schema(schema, name); decl.found()) {
            auto r = scope.find(*decl.found());
            if (r.is_found()) {
                return { *r };
            }
            if (r.is_ambiguous()) {
                report_relation_ambiguous(name);
                return find_symbol_result<relation_info const&>::error;
            }
            // not found
        } else if (decl.is_error()) {
            return find_symbol_result<relation_info const&>::error;
        }

        return {};
    }

    [[nodiscard]] find_symbol_result<descriptor::variable> find_variable_in_query(
            query_scope const& scope,
            ast::name::simple const& name) {
        ast::common::chars buffer;
        std::optional<descriptor::variable> found {};
        for (auto&& r : scope.references()) {
            for (auto v = r.find(to_identifier(name, buffer)); !v.is_absent(); v = r.next(*v)) {
                if (v.is_ambiguous()) {
                    report_column_ambiguous(name);
                    return find_symbol_result<descriptor::variable>::error;
                }
                if (!v->exported()) {
                    continue;
                }
                if (found) {
                    report_column_ambiguous(name);
                    return find_symbol_result<descriptor::variable>::error;
                }
                found = context_.bless(v->variable(), name.region());
            }
        }
        return { std::move(found) };
    }

    void report_column_ambiguous(ast::name::name const& name) {
        context_.report(sql_analyzer_code::column_ambiguous,
                string_builder {}
                        << "column '" << print_support { name } << "' is ambiguous"
                        << string_builder::to_string,
                name.region());
    }

    void report_relation_ambiguous(ast::name::name const& name) {
        context_.report(sql_analyzer_code::relation_ambiguous,
                string_builder {}
                        << "relation '" << print_support { name } << "' is ambiguous"
                        << string_builder::to_string,
                name.region());
    }

    [[nodiscard]] find_symbol_result<descriptor::variable> find_column_in_relation(
            relation_info const& parent,
            ast::name::name const& name) {
        ast::common::chars buffer;
        std::optional<descriptor::variable> found {};
        for (auto v = parent.find(to_identifier(name.last_name(), buffer));; v = parent.next(*v)) {
            if (v.is_ambiguous()) {
                report_column_ambiguous(name);
                return find_symbol_result<descriptor::variable>::error;
            }
            if (v.is_absent()) {
                break;
            }
            if (!v.value().exported()) {
                continue;
            }
            // NOTE: treats even if not exported
            if (found) {
                report_column_ambiguous(name);
                return find_symbol_result<descriptor::variable>::error;
            }
            found = context_.bless(v->variable(), name.region());
        }
        return { std::move(found) };
    }

    [[nodiscard]] symbol_info find_variable_in_schema(
            schema_decl const& schema,
            ast::name::simple const& name) {
        ast::common::chars buffer;
        auto id = to_identifier(name.last_name(), buffer);
        if (auto v = schema.variable_provider().find(id)) {
            return { context_.bless(to_descriptor(std::move(v)), name.region()) };
        }
        return not_found;
    }

    [[nodiscard]] find_symbol_result<relation_info const&> find_relation_info_in_query(
            query_scope const& query,
            ast::name::simple const& name) {
        ast::common::chars buffer;
        auto id = to_identifier(name.last_name(), buffer);
        auto r = query.find(id);
        if (r.is_ambiguous()) {
            report_relation_ambiguous(name);
            return find_symbol_result<relation_info const&>::error;
        }
        if (r) {
            return { *r };
        }
        return {};
    }

    [[nodiscard]] find_symbol_result<relation_decl const&> find_relation_decl_in_schema(schema_decl const& parent, ast::name::simple const& name) {
        ast::common::chars buffer;
        auto id = to_identifier(name, buffer);
        if (auto t = parent.storage_provider().find_relation(id)) {
            return { *t };
        }
        return {};
    }

    [[nodiscard]] find_symbol_result<table_decl const*> find_table_decl_in_schema(schema_decl const& parent, ast::name::simple const& name) {
        ast::common::chars buffer;
        auto id = to_identifier(name, buffer);
        if (auto t = parent.storage_provider().find_table(id)) {
            return { std::move(t) };
        }
        return {};
    }

    [[nodiscard]] find_symbol_result<index_decl const*> find_index_decl_in_schema(schema_decl const& parent, ast::name::simple const& name) {
        ast::common::chars buffer;
        auto id = to_identifier(name, buffer);
        if (auto t = parent.storage_provider().find_index(id)) {
            return { std::move(t) };
        }
        return {};
    }

    find_symbol_result<schema_decl const*> find_schema_in_catalog(catalog_decl const& parent, ast::name::simple const& name) {
        ast::common::chars buffer;
        auto id = to_identifier(name, buffer);
        if (auto s = parent.schema_provider().find(id)) {
            return { std::move(s) };
        }
        return {};
    }

    find_symbol_result<catalog_decl const&> find_catalog_in_system(ast::name::simple const& name) {
        if (name.node_kind() != ast::name::kind::simple) {
            return {};
        }
        ast::common::chars buffer;
        auto id = to_identifier(name, buffer);
        auto&& c = context_.options()->catalog();
        if (!c.name().empty() && id == c.name()) {
            return { c };
        }
        return {};
    }
};

} // namespace

std::unique_ptr<tscalar::expression> analyze_variable_name(
        analyzer_context& context,
        ast::name::name const& name,
        query_scope const& scope) {
    engine e { context, scope };
    return e.find_variable(name);
}

optional_ptr<relation_info const> analyze_relation_info_name(
        analyzer_context& context,
        ast::name::name const& name,
        query_scope const& scope) {
    engine e { context, scope };
    return e.find_relation_info(name);
}

optional_ptr<relation_decl const> analyze_relation_name(
        analyzer_context& context,
        ast::name::name const& name) {
    query_scope empty_scope;
    engine e { context, empty_scope };
    return e.find_relation_decl(name);
}

std::optional<schema_element<table_decl>> analyze_table_name(
        analyzer_context& context,
        ast::name::name const& name,
        bool mandatory) {
    query_scope empty_scope;
    engine e { context, empty_scope };
    return e.find_table_decl(name, mandatory);
}

std::optional<schema_element<index_decl>> analyze_index_name(
        analyzer_context& context,
        ast::name::name const& name,
        bool mandatory) {
    query_scope empty_scope;
    engine e { context, empty_scope };
    return e.find_index_decl(name, mandatory);
}

std::shared_ptr<schema_decl const> analyze_schema_name(
        analyzer_context& context,
        ast::name::name const& name,
        bool mandatory) {
    query_scope empty_scope;
    engine e { context, empty_scope };
    return e.find_schema_decl(name, mandatory);
}

std::string normalize_identifier(
        analyzer_context& context,
        ast::name::simple const& name) {
    std::string buffer;
    (void) engine::to_identifier(name, context.options()->lowercase_regular_identifiers(), buffer, true);
    return buffer;
}

} // namespace mizugaki::analyzer::details
