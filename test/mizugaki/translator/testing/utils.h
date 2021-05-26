#pragma once

#include <memory>

#include <takatori/descriptor/variable.h>

#include <takatori/util/downcast.h>
#include <takatori/util/fail.h>
#include <takatori/util/optional_ptr.h>
#include <takatori/util/string_builder.h>

#include <yugawara/storage/provider.h>

#include <mizugaki/translator/shakujo_translator_impl.h>

#include <mizugaki/translator/details/relation_info.h>

namespace mizugaki::translator::testing {

namespace descriptor = ::takatori::descriptor;
using ::takatori::util::downcast;
using ::takatori::util::fail;
using ::takatori::util::optional_ptr;
using ::takatori::util::string_builder;

inline shakujo_translator::impl new_translator_impl() {
    shakujo_translator::impl result;
    return result;
}

inline shakujo_translator_options new_options(
        std::shared_ptr<::yugawara::storage::provider const> storages = {},
        std::shared_ptr<::yugawara::variable::provider const> variables = {},
        std::shared_ptr<::yugawara::function::provider const> functions = {},
        std::shared_ptr<::yugawara::aggregate::provider const> aggregate_functions = {},
        std::shared_ptr<::yugawara::variable::provider const> host_variables = {}) {
    return {
            std::move(storages),
            std::move(variables),
            std::move(functions),
            std::move(aggregate_functions),
            std::move(host_variables),
    };
}

inline optional_ptr<::yugawara::diagnostic<shakujo_translator_code> const> occurred(
        shakujo_translator_code code,
        ::takatori::util::sequence_view<::yugawara::diagnostic<shakujo_translator_code> const> diagnostics) {
    for (auto&& d : diagnostics) {
        if (d.code() == code) {
            return d;
        }
    }
    return {};
}

template<class T, class Port>
inline T& next(Port& port) {
    if (!port.opposite()) {
        throw std::domain_error("not connected");
    }
    auto&& r = port.opposite()->owner();
    if (r.kind() != T::tag) {
        throw std::domain_error(string_builder {}
                << r.kind()
                << " <=> "
                << T::tag
                << string_builder::to_string);
    }
    return downcast<T>(r);
}

template<class T>
inline T& last(::takatori::relation::graph_type& graph) {
    for (auto&& e : graph) {
        if (e.output_ports().empty()) {
            return downcast<T>(e);
        }
    }
    fail();
}

inline void validate(std::vector<descriptor::variable> const& variables, shakujo_translator::impl const& trans) {
    auto&& a = trans.expression_analyzer();
    for (auto&& v : variables) {
        auto&& rv = a.variables().find(v);
        if (!rv) {
            fail(string_builder {} << rv << string_builder::to_string);
        }
        if (auto&& e = rv.element_if<::yugawara::analyzer::variable_resolution_kind::scalar_expression>()) {
            auto&& re = a.expressions().find(*e);
            if (!re) {
                fail(string_builder {} << re << string_builder::to_string);
            }
        }
    }
}

inline std::vector<descriptor::variable> get_columns(
        shakujo_translator::impl const& trans,
        translator::details::relation_info const& info) {
    std::vector<descriptor::variable> results;
    info.enumerate_variables([&](descriptor::variable const& v, std::string_view) {
        results.emplace_back(v);
    });
    validate(results, trans);
    return results;
}

inline std::string column_name(translator::details::relation_info const& info, descriptor::variable const& variable) {
    std::string name;
    info.enumerate_variables([&](descriptor::variable const& v, std::string_view n) {
        if (v == variable) {
            name = std::string { n };
        }
    });
    return name;
}

} // namespace mizugaki::translator::testing
