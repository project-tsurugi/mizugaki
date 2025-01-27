#include "example_prototype_processor.h"

#include <yugawara/storage/index_feature.h>

namespace mizugaki::examples::explain_cli {

using feature_type = ::yugawara::storage::index_feature;

bool example_prototype_processor::ensure(
        ::yugawara::schema::declaration const& location,
        ::yugawara::storage::table& table_prototype,
        ::yugawara::storage::index& primary_index_prototype,
        diagnostic_consumer_type const& diagnostic_consumer) {
    (void) location;
    (void) table_prototype;
    (void) diagnostic_consumer;
    primary_index_prototype.features() = {
        feature_type::primary,
        feature_type::find,
        feature_type::scan,
        feature_type::unique,
        feature_type::unique_constraint,
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
