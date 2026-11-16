#include "location_query.h"

#include <takatori/relation/expression_kind.h>
#include <takatori/scalar/expression_kind.h>
#include <takatori/plan/step_kind.h>
#include <takatori/statement/statement_kind.h>

#include <stdexcept>
#include <utility>
#include <vector>

namespace mizugaki::examples::explain_cli {

using location_id_type = ::yugawara::remote::location_info::location_id_type;
using location_id_set_type = ::yugawara::remote::location_info::location_id_set_type;

namespace {

constexpr location_id_type local_location_id = ::yugawara::remote::location_info::local_location_id;

[[nodiscard]] std::vector<std::string_view> split(std::string_view sequence, char delimiter) {
    std::vector<std::string_view> results {};
    std::size_t start = 0;
    while (start <= sequence.size()) {
        auto end = sequence.find(delimiter, start);
        if (end == std::string_view::npos) {
            end = sequence.size();
        }
        results.emplace_back(sequence.substr(start, end - start));
        start = end + 1;
    }
    return results;
}

[[nodiscard]] location_id_type parse_location_id(std::string_view text) {
    std::size_t position {};
    location_id_type result {};
    try {
        result = std::stoull(std::string { text }, &position);
    } catch (std::exception const&) {
        throw std::runtime_error("invalid location ID: " + std::string { text });
    }
    if (position != text.size() || result == local_location_id) {
        throw std::runtime_error("invalid location ID: " + std::string { text });
    }
    return result;
}

} // namespace

example_symbol_location_query::example_symbol_location_query(
        std::vector<remote_table_entry> const& remote_tables,
        std::vector<placeholder_location_entry> const& placeholders) {
    locations_.emplace(local_location_id, ::yugawara::remote::location_info { local_location_id, "local" });
    available_location_ids_.emplace(local_location_id);
    for (auto&& entry : remote_tables) {
        locations_.emplace(entry.location_id, ::yugawara::remote::location_info { entry.location_id, entry.location_name });
        available_location_ids_.emplace(entry.location_id);
        if (auto [iter, success] = remote_tables_.emplace(entry.table_name, entry.location_id); !success) {
            throw std::runtime_error("duplicate remote table: " + entry.table_name);
        }
    }
    for (auto&& entry : placeholders) {
        location_id_set_type locations {};
        if (entry.location == "*") {
            locations = available_location_ids_;
        } else {
            location_id_type id {};
            std::size_t position {};
            try {
                id = std::stoull(entry.location, &position);
            } catch (std::exception const&) {
                position = 0;
            }
            if (entry.location.empty()
                    || position != entry.location.size()
                    || available_location_ids_.find(id) == available_location_ids_.end()) {
                throw std::runtime_error("invalid placeholder location: " + entry.name + ":" + entry.location);
            }
            locations.emplace(id);
        }
        placeholders_.insert_or_assign(entry.name, std::move(locations));
    }
}

::yugawara::remote::location_info const& example_symbol_location_query::of(location_id_type location_id) const {
    return locations_.at(location_id);
}

location_id_set_type const& example_symbol_location_query::available_location_ids() const {
    return available_location_ids_;
}

location_id_set_type example_symbol_location_query::find(::yugawara::storage::table const& symbol) const {
    if (auto iter = remote_tables_.find(symbol.simple_name()); iter != remote_tables_.end()) {
        return { iter->second };
    }
    return { local_location_id };
}

location_id_set_type example_symbol_location_query::find(::yugawara::function::declaration const&) const {
    return { local_location_id };
}

location_id_set_type example_symbol_location_query::find(::yugawara::aggregate::declaration const&) const {
    return { local_location_id };
}

location_id_set_type example_symbol_location_query::find(::yugawara::variable::declaration const& symbol) const {
    if (auto iter = placeholders_.find(symbol.name()); iter != placeholders_.end()) {
        return iter->second;
    }
    return available_location_ids_;
}

std::vector<remote_table_entry> parse_remote_tables(std::string_view specification) {
    std::vector<remote_table_entry> results {};
    if (specification.empty()) {
        return results;
    }
    for (auto entry : split(specification, ',')) {
        auto fields = split(entry, ':');
        if (fields.size() < 2 || fields.size() > 3 || fields[0].empty()) {
            throw std::runtime_error("invalid remote table specification: " + std::string { entry });
        }
        auto id = parse_location_id(fields[1]);
        std::string name = fields.size() == 3 ? std::string { fields[2] } : "remote" + std::to_string(id);
        results.push_back(remote_table_entry {
                std::string { fields[0] },
                id,
                std::move(name),
        });
    }
    return results;
}

std::shared_ptr<example_symbol_location_query> make_symbol_location_query(
        std::string_view specification,
        std::vector<placeholder_location_entry> const& placeholders) {
    return std::make_shared<example_symbol_location_query>(parse_remote_tables(specification), placeholders);
}

void apply_remote_kinds(::yugawara::remote::location_options& options, std::string_view name) {
    using options_type = ::yugawara::remote::location_options;
    if (name == "universe") {
        options.remote_relation_expression_kinds() = options_type::default_remote_relation_expression_kinds;
        options.remote_scalar_expression_kinds() = options_type::default_remote_scalar_expression_kinds;
        options.remote_step_kinds() = options_type::default_remote_step_kinds;
        options.remote_statement_kinds() = options_type::default_remote_statement_kinds;
        return;
    }
    if (name == "phase1") {
        using relation = ::takatori::relation::expression_kind;
        using scalar = ::takatori::scalar::expression_kind;
        using step = ::takatori::plan::step_kind;
        using statement = ::takatori::statement::statement_kind;
        options.remote_relation_expression_kinds() = {
                relation::scan,
                relation::find,
                relation::filter,
                relation::project,
                relation::offer,
        };
        options.remote_scalar_expression_kinds() = {
                scalar::immediate,
                scalar::variable_reference,
                scalar::unary,
                scalar::cast,
                scalar::binary,
                scalar::compare,
        };
        options.remote_step_kinds() = {
                step::process,
        };
        options.remote_statement_kinds() = {
                statement::execute,
        };
        return;
    }
    throw std::runtime_error("unknown remote kinds: " + std::string { name });
}

} // namespace mizugaki::examples::explain_cli
