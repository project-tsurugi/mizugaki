#include <mizugaki/analyzer/details/analyze_name.h>

#include <functional>
#include <variant>

#include <takatori/descriptor/variable.h>

#include <takatori/scalar/variable_reference.h>

#include <takatori/util/assertion.h>
#include <takatori/util/downcast.h>
#include <takatori/util/optional_ptr.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

#include <yugawara/schema/declaration.h>
#include <yugawara/schema/catalog.h>

#include <yugawara/storage/table.h>

#include <mizugaki/analyzer/details/find_result.h>
#include <mizugaki/analyzer/details/name_printer.h>
#include <mizugaki/analyzer/details/relation_info.h>

namespace mizugaki::analyzer::details {

namespace tscalar = ::takatori::scalar;

namespace descriptor = ::takatori::descriptor;
using schema = ::yugawara::schema::declaration;
using catalog = ::yugawara::schema::catalog;
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
        std::reference_wrapper<schema const>,
        std::reference_wrapper<catalog const>>;

class engine {
public:
    engine(analyzer_context& context, query_scope const& scope) noexcept :
        context_ { context },
        scope_ { scope }
    {}

    [[nodiscard]] std::unique_ptr<tscalar::expression> find_variable(ast::name::name const& name) {
        auto alternatives = find_variable0(name, 0);
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
                        << "'" << name_printer << name << "' is not found"
                        << string_builder::to_string,
                name.region());
        return {};
    }

    [[nodiscard]] optional_ptr<relation_decl const> find_relation(ast::name::name const& name) {
        auto alternatives = find_relation0(name, 0);
        if (saw_error(alternatives)) {
            return {};
        }
        if (auto v = unwrap(std::get_if<std::reference_wrapper<relation_decl const>>(&alternatives))) {
            return v;
        }
        context_.report(sql_analyzer_code::table_not_found,
                string_builder {}
                        << "'" << name_printer << name << "' is not found"
                        << string_builder::to_string,
                name.region());
        return {};
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

    [[nodiscard]] symbol_info find_variable0(ast::name::name const& name, std::size_t depth) {
        if (auto q = name.optional_qualifier()) {
            symbol_info qualifier = find_variable0(*q, depth + 1);
            return find_variable_qualified(std::move(qualifier), unsafe_downcast<ast::name::qualified>(name), depth);
        }
        return find_variable_primary(unsafe_downcast<ast::name::simple>(name), depth);
    }

    [[nodiscard]] symbol_info find_variable_primary(ast::name::simple const& name, std::size_t depth) {
        // 1-1. find column ref
        if (auto r = find_variable_in_query(scope_, name)) {
            if (auto&& v = r.found()) {
                return *v;
            }
        } else {
            return saw_error; // propagate error
        }

        // 1-2. find relation ref (depth >= 1)
        if (depth >= 1) {
            if (auto r = find_relation_ref_in_query(scope_, name)) {
                if (auto&& v = r.found()) {
                    return *v;
                }
            } else {
                return saw_error; // propagate error
            }
        }

        // FIXME: 2. find column also from main query
        // FIXME: 3. find parameter variables
        // FIXME: 4. find session variables

        // 5. find schema elements
        for (auto&& s : context_.options()->schema_search_path().elements()) {
            // 5-1. find schema variables
            if (auto r = find_variable_in_schema(*s, name, depth); !not_found(r)) {
                return r;
            }
        }

        // 6. find schema (depth >= 1)
        if (depth >= 1) {
            if (auto r = find_schema_in_catalog(context_.options()->catalog(), name)) {
                if (auto&& v = r.found()) {
                    return { *v };
                }
            } else {
                return saw_error;
            }
        }

        // 7. find catalog (depth >= 2)
        if (depth >= 2) {
            if (auto r = find_catalog(name)) {
                if (auto&& v = r.found()) {
                    return { *v };
                }
            } else {
                return saw_error;
            }
        }

        context_.report(sql_analyzer_code::symbol_not_found,
                string_builder {}
                        << "symbol '" << name.identifier() << "' is not found"
                        << string_builder::to_string,
                name.region());
        return saw_error;
    }

    [[nodiscard]] symbol_info find_variable_qualified(symbol_info qualifier, ast::name::qualified const& name, std::size_t depth) {
        if (saw_error(qualifier) || not_found(qualifier)) {
            return qualifier;
        }
        if (auto v = optional_ptr { std::get_if<descriptor::variable>(&qualifier) }) {
            // FIXME: impl field reference
            (void) v;
            context_.report(sql_analyzer_code::unsupported_feature,
                    "the field reference is not supported",
                    name.region());
            return saw_error;
        }

        if (auto v = unwrap(std::get_if<std::reference_wrapper<relation_info const>>(&qualifier))) {
            if (auto r = find_variable_in_relation(*v, name)) {
                if (auto&& q = r.found()) {
                    return *q;
                }
                context_.report(sql_analyzer_code::column_not_found,
                        string_builder {}
                                << "column '" << name_printer << *name.last() << "' is not found"
                                << "in relation '" << name_printer << *name.qualifier() << "'"
                                << string_builder::to_string,
                        name.region());
            }
            return saw_error;
        }

        if (auto v = unwrap(std::get_if<std::reference_wrapper<schema const>>(&qualifier))) {
            if (auto r = find_variable_in_schema(*v, name, depth); !not_found(r)) {
                return r;
            }
            context_.report(sql_analyzer_code::variable_not_found,
                    string_builder {}
                            << "variable '" << name_printer << *name.last() << "' is not found"
                            << "in schema '" << name_printer << *name.qualifier() << "'"
                            << string_builder::to_string,
                    name.region());
            return saw_error;
        }

        if (auto v = unwrap(std::get_if<std::reference_wrapper<catalog const>>(&qualifier))) {
            if (auto s = find_schema_in_catalog(*v, name)) {
                if (auto&& q = s.found()) {
                    return { *q };
                }
                context_.report(sql_analyzer_code::schema_not_found,
                        string_builder {}
                                << "schema '" << name_printer << *name.last() << "' is not found"
                                << "in catalog '" << name_printer << *name.qualifier() << "'"
                                << string_builder::to_string,
                        name.region());
            }
            return saw_error;
        }
        ::takatori::util::fail();
    }

    [[nodiscard]] symbol_info find_relation0(ast::name::name const& name, std::size_t depth) {
        if (auto q = name.optional_qualifier()) {
            symbol_info qualifier = find_relation0(*q, depth + 1);
            return find_relation_qualified(std::move(qualifier), unsafe_downcast<ast::name::qualified>(name), depth);
        }
        return find_relation_primary(unsafe_downcast<ast::name::simple>(name), depth);
    }

    [[nodiscard]] symbol_info find_relation_primary(ast::name::simple const& name, std::size_t depth) {
        // FIXME: 1. query name

        // 2. find schema elements (depth = 0)
        if (depth == 0) {
            for (auto&& s : context_.options()->schema_search_path().elements()) {
                if (auto r = find_relation_decl_in_schema(*s, name)) {
                    if (r.found()) {
                        return *r.found();
                    }
                } else {
                    return saw_error;
                }
            }
        }

        // 3. find schema (depth = 1)
        if (depth == 1) {
            if (auto r = find_schema_in_catalog(context_.options()->catalog(), name)) {
                if (auto&& v = r.found()) {
                    return { *v };
                }
            } else {
                return saw_error;
            }
        }

        // 4. find catalog (depth = 2)
        if (depth == 2) {
            if (auto r = find_catalog(name)) {
                if (auto&& v = r.found()) {
                    return { *v };
                }
            } else {
                return saw_error;
            }
        }

        context_.report(sql_analyzer_code::symbol_not_found,
                string_builder {}
                        << "symbol '" << name.identifier() << "' is not found"
                        << string_builder::to_string,
                name.region());
        return saw_error;
    }

    [[nodiscard]] symbol_info find_relation_qualified(symbol_info qualifier, ast::name::qualified const& name, std::size_t depth) {
        if (saw_error(qualifier) || not_found(qualifier)) {
            return qualifier;
        }
        // NOTE: must not come relation_info, because it only appears in depth = 0.

        if (auto v = unwrap(std::get_if<std::reference_wrapper<schema const>>(&qualifier))) {
            BOOST_ASSERT(depth == 0); // NOLINT
            if (auto r = find_relation_decl_in_schema(*v, name)) {
                if (auto&& q = r.found()) {
                    return { *q };
                }
                context_.report(sql_analyzer_code::table_not_found,
                        string_builder {}
                                << "relation '" << name_printer << *name.last() << "' is not found"
                                << "in schema '" << name_printer << *name.qualifier() << "'"
                                << string_builder::to_string,
                        name.region());
            }
            return saw_error;
        }

        if (auto v = unwrap(std::get_if<std::reference_wrapper<catalog const>>(&qualifier))) {
            BOOST_ASSERT(depth == 1); // NOLINT
            if (auto s = find_schema_in_catalog(*v, name)) {
                if (auto&& q = s.found()) {
                    return { *q };
                }
                context_.report(sql_analyzer_code::schema_not_found,
                        string_builder {}
                                << "schema '" << name_printer << *name.last() << "' is not found"
                                << "in catalog '" << name_printer << *name.qualifier() << "'"
                                << string_builder::to_string,
                        name.region());
            }
            return saw_error;
        }
        ::takatori::util::fail();
    }

    [[nodiscard]] find_result<descriptor::variable> find_variable_in_query(
            query_scope const& scope,
            ast::name::name const& name) {
        std::optional<descriptor::variable> found {};
        for (auto&& r : scope.references()) {
            for (auto v = r.find(name.last_identifier()); v; v = r.next(*v)) {
                // NOTE: treats only exported
                if (!v->exported()) {
                    continue;
                }
                if (found) {
                    context_.report(sql_analyzer_code::variable_ambiguous,
                            string_builder {}
                                    << "column '" << name_printer << name << "' is ambiguous"
                                    << string_builder::to_string,
                            name.region());
                    return find_result<descriptor::variable>::error;
                }
                found = context_.bless(v->variable(), name.region());
            }
        }
        return { std::move(found) };
    }

    [[nodiscard]] find_result<descriptor::variable> find_variable_in_relation(
            relation_info const& parent,
            ast::name::name const& name) {
        std::optional<descriptor::variable> found {};
        for (auto v = parent.find(name.last_identifier()); v; v = parent.next(*v)) {
            // NOTE: treats even if not exported
            if (found) {
                context_.report(sql_analyzer_code::variable_ambiguous,
                        string_builder {}
                                << "column '" << name_printer << name << "' is ambiguous"
                                << string_builder::to_string,
                        name.region());
                return find_result<descriptor::variable>::error;
            }
            found = context_.bless(v->variable(), name.region());
        }
        return { std::move(found) };
    }

    [[nodiscard]] symbol_info find_variable_in_schema(
            schema const& parent,
            ast::name::name const& name,
            std::size_t depth) {
        if (auto v = parent.variable_provider().find(name.last_identifier())) {
            return { context_.bless(to_descriptor(std::move(v)), name.region()) };
        }
        if (depth >= 1) {
            if (auto t = parent.storage_provider().find_table(name.last_identifier())) {
                for (optional_ptr<query_scope const> s = scope_; s; s = s->parent()) {
                    if (auto info = s->find(*t)) {
                        return { *info };
                    }
                }
            }
        }
        return not_found;
    }

    [[nodiscard]] static find_result<relation_info const&> find_relation_ref_in_query(query_scope const& parent, ast::name::name const& name) {
        return parent.find(name.last_identifier());
    }

    [[nodiscard]] static find_result<relation_decl const&> find_relation_decl_in_schema(schema const& parent, ast::name::name const& name) {
        if (auto t = parent.storage_provider().find_table(name.last_identifier())) {
            return { *t };
        }
        return {};
    }

    find_result<catalog const&> find_catalog(ast::name::simple const& name) {
        auto&& c = context_.options()->catalog();
        if (!c.name().empty() && name.identifier() == c.name()) {
            return { c };
        }
        return {};
    }

    static find_result<schema const&> find_schema_in_catalog(catalog const& parent, ast::name::name const& name) {
        if (auto s = parent.schema_provider().find(name.last_identifier())) {
            return { *s };
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

optional_ptr<relation_decl const> analyze_relation_name(
        analyzer_context& context,
        ast::name::name const& name) {
    query_scope empty_scope;
    engine e { context, empty_scope };
    return e.find_relation(name);
}

} // namespace mizugaki::analyzer::details
