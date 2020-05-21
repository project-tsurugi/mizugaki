#include "value_info_translator.h"

#include <numeric>

#include <takatori/value/primitive.h>
#include <takatori/value/character.h>

#include <takatori/util/string_builder.h>

#include <mizugaki/translator/util/value_info_dispatch.h>

namespace mizugaki::translator::details {

namespace value = ::takatori::value;

using translator_type = shakujo_translator::impl;
using result_type = std::shared_ptr<::takatori::value::data>;
using code_type = shakujo_translator_code;

using ::takatori::util::string_builder;

namespace {

class engine {
public:
    explicit engine(
            translator_type& translator,
            ::shakujo::common::core::DocumentRegion const& region) noexcept
        : translator_(translator)
        , region_(region)
    {}

    result_type operator()(::shakujo::common::core::Value const& info) {
        return report(code_type::unsupported_value, string_builder {}
                << info.kind());
    }

    result_type operator()(::shakujo::common::core::value::Bool const& info) {
        return translator_.values().get(value::boolean { *info });
    }

    result_type operator()(::shakujo::common::core::value::Int const& info) {
        if (auto v = convert_if_fit<value::int4>(info)) {
            return v;
        }
        if (auto v = convert_if_fit<value::int8>(info)) {
            return v;
        }
        return report(code_type::unsupported_value, string_builder {}
                << "invalid integer range: "
                << *info);
    }

    result_type operator()(::shakujo::common::core::value::Float const& info) {
        return translator_.values().get(value::float8 { *info });
    }

    result_type operator()(::shakujo::common::core::value::String const& info) {
        return translator_.values().get(value::character {
                *info,
                translator_.object_creator().allocator(),
        });
    }

    result_type operator()(::shakujo::common::core::value::Null const&) {
        return translator_.values().get(value::unknown { value::unknown_kind::null });
    }

private:
    translator_type& translator_;
    ::shakujo::common::core::DocumentRegion const& region_;

    result_type report(code_type code, string_builder&& builder) {
        translator_.diagnostics().emplace_back(
                code,
                builder << string_builder::to_string,
                translator_.region(region_));
        return {};
    }

    template<class T>
    result_type convert_if_fit(::shakujo::common::core::value::Int const& value) {
        using value_type = typename T::entity_type;
        using limits = std::numeric_limits<value_type>;
        auto v = *value;
        if (limits::lowest() <= v && v <= limits::max()) {
            return translator_.values().get(T { static_cast<value_type>(v) });
        }
        return {};
    }
};

} // namespace

value_info_translator::value_info_translator(translator_type& translator) noexcept
    : translator_(translator)
{}

result_type value_info_translator::process(
        ::shakujo::common::core::Value const& info,
        ::shakujo::common::core::DocumentRegion const& region) {
    engine e { translator_, region };
    return util::dispatch(e, info);
}

} // namespace mizugaki::translator::details
