#include "example_prototype_processor.h"

#include <algorithm>
#include <string>
#include <vector>

#include <yugawara/storage/index_feature.h>

namespace mizugaki::examples::explain_cli {

using feature_type = ::yugawara::storage::index_feature;

// NOTE: the index definitions follow the ones in jogasaki, except it never generates the primary key column
bool example_prototype_processor::ensure(
        ::yugawara::schema::declaration const& location,
        ::yugawara::storage::table& table_prototype,
        ::yugawara::storage::index& primary_index_prototype,
        diagnostic_consumer_type const& diagnostic_consumer) {
    (void) location;
    (void) diagnostic_consumer;

    // put non-key columns as primary index values
    auto&& keys = primary_index_prototype.keys();
    std::vector<::yugawara::storage::index::column_ref> values {};
    for (auto&& column : table_prototype.columns()) {
        if (std::none_of(keys.begin(), keys.end(), [&](auto&& key) { return key.column() == column; })) {
            values.emplace_back(column);
        }
    }

    primary_index_prototype.simple_name(std::string { table_prototype.simple_name() });
    primary_index_prototype.values() = std::move(values);
    primary_index_prototype.features() = {
        feature_type::primary,
        feature_type::find,
        feature_type::scan,
        feature_type::unique,
    };
    return true;
}

bool example_prototype_processor::ensure(
        ::yugawara::schema::declaration const& location,
        ::yugawara::storage::index& secondary_index_prototype,
        diagnostic_consumer_type const& diagnostic_consumer) {
    (void) location;
    (void) diagnostic_consumer;
    secondary_index_prototype.features() = {
        feature_type::find,
        feature_type::scan,
    };
    return true;
}

} // namespace mizugaki::examples::explain_cli
