#include "type_info_translator.h"

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>

#include <takatori/util/string_builder.h>

#include <mizugaki/translator/util/type_info_dispatch.h>

namespace mizugaki::translator::details {

namespace type = ::takatori::type;

using translator_type = shakujo_translator::impl;
using result_type = std::shared_ptr<::takatori::type::data>;
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

    result_type operator()(::shakujo::common::core::Type const& info) {
        return report(code_type::unsupported_type, string_builder {}
                << info.kind());
    }

    result_type operator()(::shakujo::common::core::type::Bool const&) {
        return translator_.types().get(type::boolean {});
    }

    result_type operator()(::shakujo::common::core::type::Int const& info) {
        switch (info.size()) {
            case 8: return translator_.types().get(type::int1 {});
            case 16: return translator_.types().get(type::int2 {});
            case 32: return translator_.types().get(type::int4 {});
            case 64: return translator_.types().get(type::int8 {});
            default:
                return report(code_type::unsupported_type, string_builder {}
                        << "unsupported integer width: "
                        << info.size());
        }
    }

    result_type operator()(::shakujo::common::core::type::Float const& info) {
        switch (info.size()) {
            case 32: return translator_.types().get(type::float4 {});
            case 64: return translator_.types().get(type::float8 {});
            default:
                return report(code_type::unsupported_type, string_builder {}
                        << "unsupported floating point number width: "
                        << info.size());
        }
    }

    result_type operator()(::shakujo::common::core::type::Char const& info) {
        return translator_.types().get(type::character {
                type::varying_t { info.varying() },
                info.size(),
        });
    }

    result_type operator()(::shakujo::common::core::type::String const&) {
        return translator_.types().get(type::character { type::varying });
    }

    result_type operator()(::shakujo::common::core::type::Null const&) {
        return translator_.types().get(type::unknown {});
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
};

} // namespace

type_info_translator::type_info_translator(translator_type& translator) noexcept
    : translator_(translator)
{}

result_type type_info_translator::process(
        ::shakujo::common::core::Type const& info,
        ::shakujo::common::core::DocumentRegion const& region) {
    engine e { translator_, region };
    return util::dispatch(e, info);
}

} // namespace mizugaki::translator::details
