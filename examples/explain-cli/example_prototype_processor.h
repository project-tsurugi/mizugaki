#pragma once

#include <yugawara/storage/basic_prototype_processor.h>

namespace mizugaki::examples::explain_cli {

class example_prototype_processor : public ::yugawara::storage::basic_prototype_processor {
public:
    /**
     * @brief creates a new instance.
     * @param secondary_index_features the additional features of secondary indices
     */
    explicit example_prototype_processor(::yugawara::storage::index_feature_set secondary_index_features = {}) noexcept;

    bool ensure(
            ::yugawara::schema::declaration const& location,
            ::yugawara::storage::table& table_prototype,
            ::yugawara::storage::index& primary_index_prototype,
            diagnostic_consumer_type const& diagnostic_consumer) override;

    bool ensure(
            ::yugawara::schema::declaration const& location,
            ::yugawara::storage::index& secondary_index_prototype,
            diagnostic_consumer_type const& diagnostic_consumer) override;

private:
    ::yugawara::storage::index_feature_set secondary_index_features_;
};

} // namespace mizugaki::examples::explain_cli
