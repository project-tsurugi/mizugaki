#include "location_query.h"

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

example_symbol_location_query::example_symbol_location_query(std::vector<remote_table_entry> const& remote_tables) {
    locations_.emplace(local_location_id, ::yugawara::remote::location_info { local_location_id, "local" });
    available_location_ids_.emplace(local_location_id);
    for (auto&& entry : remote_tables) {
        locations_.emplace(entry.location_id, ::yugawara::remote::location_info { entry.location_id, entry.location_name });
        available_location_ids_.emplace(entry.location_id);
        if (auto [iter, success] = remote_tables_.emplace(entry.table_name, entry.location_id); !success) {
            throw std::runtime_error("duplicate remote table: " + entry.table_name);
        }
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

location_id_set_type example_symbol_location_query::find(::yugawara::variable::declaration const&) const {
    return { local_location_id };
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

std::shared_ptr<example_symbol_location_query> make_symbol_location_query(std::string_view specification) {
    return std::make_shared<example_symbol_location_query>(parse_remote_tables(specification));
}

} // namespace mizugaki::examples::explain_cli
