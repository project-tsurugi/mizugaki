#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <yugawara/remote/location_info.h>
#include <yugawara/remote/symbol_location_query.h>

namespace mizugaki::examples::explain_cli {

/**
 * @brief a table which is defined only in a remote location.
 */
struct remote_table_entry {
    /// @brief the simple name of the table.
    std::string table_name;

    /// @brief the location ID where the table is defined (must not be the local location).
    ::yugawara::remote::location_info::location_id_type location_id;

    /// @brief the name of the location.
    std::string location_name;
};

/**
 * @brief a symbol location query for explain-cli.
 * @details The tables specified in the constructor are defined only in the given remote locations,
 *      and the other symbols are defined only in the local location.
 * @see make_symbol_location_query()
 */
class example_symbol_location_query : public ::yugawara::remote::symbol_location_query {
public:
    /**
     * @brief creates a new instance.
     * @param remote_tables the tables which are defined only in remote locations
     * @throws std::runtime_error if the same table name appears two or more times
     */
    explicit example_symbol_location_query(std::vector<remote_table_entry> const& remote_tables);

    [[nodiscard]] ::yugawara::remote::location_info const& of(location_id_type location_id) const override;

    [[nodiscard]] location_id_set_type const& available_location_ids() const override;

    [[nodiscard]] location_id_set_type find(::yugawara::storage::table const& symbol) const override;

    [[nodiscard]] location_id_set_type find(::yugawara::function::declaration const& symbol) const override;

    [[nodiscard]] location_id_set_type find(::yugawara::aggregate::declaration const& symbol) const override;

    [[nodiscard]] location_id_set_type find(::yugawara::variable::declaration const& symbol) const override;

private:
    std::map<location_id_type, ::yugawara::remote::location_info> locations_ {};
    location_id_set_type available_location_ids_ {};
    std::map<std::string, location_id_type, std::less<>> remote_tables_ {};
};

/**
 * @brief parses the remote table specification.
 * @param specification the remote table specification (`<table>:<location id>[:<location name>]`, separated by `,`)
 * @return the parsed entries, or empty if the specification is empty
 * @throws std::runtime_error if the specification is malformed
 */
[[nodiscard]] std::vector<remote_table_entry> parse_remote_tables(std::string_view specification);

/**
 * @brief creates a new symbol location query from the remote table specification.
 * @param specification the remote table specification (see parse_remote_tables())
 * @return the created symbol location query
 * @throws std::runtime_error if the specification is malformed
 */
[[nodiscard]] std::shared_ptr<example_symbol_location_query> make_symbol_location_query(std::string_view specification);

} // namespace mizugaki::examples::explain_cli
