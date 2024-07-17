#include <mizugaki/analyzer/details/analyze_literal.h>

#include <limits>

#include <takatori/datetime/conversion.h>

#include <takatori/type/primitive.h>
#include <takatori/type/decimal.h>
#include <takatori/type/character.h>
#include <takatori/type/date.h>
#include <takatori/type/time_of_day.h>
#include <takatori/type/time_point.h>

#include <takatori/value/primitive.h>
#include <takatori/value/decimal.h>
#include <takatori/value/date.h>
#include <takatori/value/time_of_day.h>
#include <takatori/value/time_point.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/cast.h>

#include <takatori/util/string_builder.h>

#include <mizugaki/ast/literal/dispatch.h>
#include <takatori/value/character.h>

namespace mizugaki::analyzer::details {

namespace ttype = ::takatori::type;
namespace tvalue = ::takatori::value;
namespace tscalar = ::takatori::scalar;

using ::takatori::util::string_builder;

namespace {

class engine {
public:
    explicit engine(analyzer_context& context, scalar_value_context const& value_context) noexcept :
        context_ { context },
        value_context_ { value_context }
    {}

    [[nodiscard]] std::unique_ptr<tscalar::expression> process(ast::literal::literal const& literal) {
        auto result = ast::literal::dispatch(*this, literal);
        if (!result) {
            return {};
        }
        if (auto&& t = value_context_.type();
                context_.options()->cast_literals_in_context() &&
                t && *t != result->type()) {
            // NOTE: here, we only apply cast operation to constant values.
            // later optimization (if it available) will reduce this operation
            return context_.create<tscalar::cast>(
                    result->region(),
                    t,
                    tscalar::cast_loss_policy::error,
                    std::move(result));
        }
        return result;

    }

    std::unique_ptr<tscalar::immediate> operator()(ast::literal::literal const& value) {
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                    << "unsupported literal kind: "
                    << value.node_kind()
                    << string_builder::to_string,
                value.region());
        return {};
    }

    std::unique_ptr<tscalar::immediate> operator()(ast::literal::boolean const& value) {
        if (value.value() == ast::literal::boolean_kind::unknown) {
            return context_.create<tscalar::immediate>(
                    value.region(),
                    context_.values().get(tvalue::unknown {}),
                    context_.types().get(ttype::boolean {}));
        }
        return context_.create<tscalar::immediate>(
                value.region(),
                context_.values().get(tvalue::boolean {
                        value.value() == ast::literal::boolean_kind::true_,
                }),
                context_.types().get(ttype::boolean {}));
    }

    std::unique_ptr<tscalar::immediate> operator()(ast::literal::numeric const& value) {
        using ast::literal::kind;
        switch (value.node_kind()) {
            case kind::exact_numeric: return process_exact_numeric(value);
            case kind::approximate_numeric: return process_approximate_numeric(value);
            default:
                break;
        }
        // FIXME impl
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported numeric literal kind: "
                        << value.node_kind()
                        << string_builder::to_string,
                value.region());
        return {};
    }

    std::unique_ptr<tscalar::immediate> operator()(ast::literal::string const& value) {
        using ast::literal::kind;
        switch (value.node_kind()) {
            case kind::character_string: return process_character_string(value);
            default:
                break;
        }
        // FIXME impl
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                    << "unsupported character string literal kind: "
                    << value.node_kind()
                    << string_builder::to_string,
                value.region());
        return {};
    }

    std::unique_ptr<tscalar::immediate> operator()(ast::literal::datetime const& value) {
        auto&& quoted = *value.value();
        if (quoted.size() < 2 || quoted.front() != '\'' || quoted.back() != '\'') {
            context_.report(
                    sql_analyzer_code::malformed_quoted_string,
                    string_builder {}
                            << "invalid quoted string \"" << quoted << "\""
                            << " for " << value.node_kind() << " literal"
                            << string_builder::to_string,
                    value.value().region());
            return {};
        }

        using ast::literal::kind;
        switch (value.node_kind()) {
            case kind::date:
                return process_date(value);
            case kind::time:
                return process_time(value);
            case kind::time_with_time_zone:
                break; // NOTE: TIME WITH TIME ZONE is not supported yet
            case kind::timestamp:
                return process_timestamp(value, false);
            case kind::timestamp_with_time_zone:
                return process_timestamp(value, true);
            default:
                break;
        }
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported temporal literal kind: "
                        << value.node_kind()
                        << string_builder::to_string,
                value.region());
        return {};
    }


//    std::unique_ptr<tscalar::immediate> operator()(ast::literal::interval const& value) {
//        (void) value;
//        return {};
//    }

    std::unique_ptr<tscalar::immediate> operator()(ast::literal::null const& value) {
        if (auto t = value_context_.type()) {
            return context_.create<tscalar::immediate>(
                    value.region(),
                    context_.values().get(tvalue::unknown { tvalue::unknown_kind::null }),
                    std::move(t));
        }
        if (context_.options()->allow_context_independent_null()) {
            return context_.create<tscalar::immediate>(
                    value.region(),
                    context_.values().get(tvalue::unknown { tvalue::unknown_kind::null }),
                    context_.types().get(ttype::unknown {}));
        }
        context_.report(sql_analyzer_code::missing_context_of_null,
                "cannot use 'NULL' here",
                value.region());
        return {};
    }

//    std::unique_ptr<tscalar::immediate> operator()(ast::literal::empty const& value) {
//        (void) value;
//        return {};
//    }

    std::unique_ptr<tscalar::immediate> operator()(ast::literal::default_ const& value) {
        auto t = value_context_.type();
        auto v = value_context_.default_value();
        if (t && v) {
            return context_.create<tscalar::immediate>(
                    value.region(),
                    std::move(v),
                    std::move(t));
        }
        context_.report(sql_analyzer_code::missing_context_of_default_value,
                "cannot use 'DEFAULT' here",
                value.region());
        return {};
    }

private:
    analyzer_context& context_;
    scalar_value_context const& value_context_;

    template<class T>
    [[nodiscard]] static std::optional<T> soft_cast(std::int64_t value) noexcept {
        if (value >= std::numeric_limits<T>::min() && value <= std::numeric_limits<T>::max()) {
            return static_cast<T>(value);
        }
        return {};
    }

    [[nodiscard]] std::unique_ptr<tscalar::immediate> process_exact_numeric(ast::literal::numeric const& value) { // NOLINT(*-function-cognitive-complexity)
        // FIXME: move to yugawara
        auto max_precision = static_cast<mpd_ssize_t>(context_.options()->max_decimal_precision());
        ::decimal::Context context {
            max_precision,
            MPD_MAX_EMAX, // emax
            MPD_MIN_EMIN, // emin
            // FIXME: other decimal context
        };

        ::decimal::Decimal v { *value.unsigned_value(), context };
        if ((context.status() & ::decimal::DecRounded) != 0U ||
                v.exponent() > 0 ||
                v.exponent() < -max_precision) {
            context_.report(sql_analyzer_code::unsupported_decimal_value,
                    string_builder {}
                            << "invalid decimal value '" << value.unsigned_value() << "'"
                            << " (inexact value by rounding)"
                            << string_builder::to_string,
                    value.region());
            return {};
        }
        if (value.sign() == ast::literal::sign::minus) {
            v = v.minus(context);
        }
        if (!v.iszero() && !v.isnormal(context)) {
            context_.report(sql_analyzer_code::unsupported_decimal_value,
                    string_builder {}
                            << "invalid decimal value '" << value.unsigned_value() << "' "
                            << "(" << v.number_class(context) << ")"
                            << string_builder::to_string,
                    value.region());
        }
        ::takatori::decimal::triple triple { v };
        if (triple.exponent() == 0) {
            static constexpr auto positive_max = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
            static constexpr auto negative_max = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::min());

            std::optional<std::int64_t> integer {};
            if (triple.coefficient_high() == 0
                && ((triple.coefficient_low() <= positive_max)
                    || (triple.sign() < 0 && triple.coefficient_low() <= negative_max))) {
                integer = static_cast<std::int64_t>(triple.coefficient_low());
                if (triple.sign() < 0) {
                    integer = -*integer;
                }
            }
            if (integer) {
                if (context_.options()->prefer_small_integer_literals()) {
                    if (auto r = soft_cast<std::int8_t>(*integer)) {
                        return context_.create<tscalar::immediate>(
                                value.region(),
                                context_.values().get(tvalue::int4 { *r }),
                                context_.types().get(ttype::int1 {}));
                    }
                    if (auto r = soft_cast<std::int16_t>(*integer)) {
                        return context_.create<tscalar::immediate>(
                                value.region(),
                                context_.values().get(tvalue::int4 { *r }),
                                context_.types().get(ttype::int2 {}));
                    }
                    if (auto r = soft_cast<std::int32_t>(*integer)) {
                        return context_.create<tscalar::immediate>(
                                value.region(),
                                context_.values().get(tvalue::int4 { *r }),
                                context_.types().get(ttype::int4 {}));
                    }
                }
                return context_.create<tscalar::immediate>(
                        value.region(),
                        context_.values().get(tvalue::int8 { *integer }),
                        context_.types().get(ttype::int8 {}));
            }
        }
        auto scale = static_cast<std::size_t>(-v.exponent());
        std::optional<std::size_t> precision {};
        if (context_.options()->prefer_small_decimal_literals()) {
            auto digits = static_cast<std::size_t>(v.coeff().adjexp() + 1);
            precision = std::max(digits, scale);
        }
        return context_.create<tscalar::immediate>(
                value.region(),
                context_.values().get(tvalue::decimal { ::takatori::decimal::triple { v } }),
                context_.types().get(ttype::decimal {
                        precision,
                        scale,
                }));
    }

    [[nodiscard]] std::unique_ptr<tscalar::immediate> process_approximate_numeric(ast::literal::numeric const& value) {
        // FIXME: more accurate values for fp
        std::size_t index {};
        double result {};
        try {
            result = std::stod(*value.unsigned_value(), &index);
            if (value.sign() == ast::literal::sign::minus) {
                result = -result;
            }
            if (index != value.unsigned_value().value().size()) {
                context_.report(sql_analyzer_code::invalid_numeric_scale,
                        string_builder {}
                                << "invalid numeric value: "
                                << value.unsigned_value()
                                << string_builder::to_string,
                        value.region());
                return {};
            }
        } catch (std::invalid_argument& e) {
            context_.report(sql_analyzer_code::invalid_numeric_scale,
                    string_builder {}
                            << "invalid numeric value: "
                            << value.unsigned_value()
                            << string_builder::to_string,
                    value.region());
            return {};
        } catch (std::out_of_range& e) {
            context_.report(sql_analyzer_code::unsupported_approximate_number,
                    string_builder {}
                            << "out of range numeric value :"
                            << value.unsigned_value()
                            << string_builder::to_string,
                    value.region());
            return {};
        }
        return context_.create<tscalar::immediate>(
                value.region(),
                context_.values().get(tvalue::float8 { result }),
                context_.types().get(ttype::float8 {}));
    }

    [[nodiscard]] std::optional<std::size_t> count_characters(ast::literal::string::value_type const& string) {
        constexpr char quote = ast::literal::string::quote_character;
        auto&& q = *string;
        if (q.size() >= 2 && q.front() == quote && q.back() == quote) {
            std::size_t count = 0;
            bool malformed = false;
            for (std::size_t i = 1, n = q.size() - 1; i < n; ++i) {
                auto c = q[i];
                if (c == quote) {
                    ++i;
                    if (i >= n || q[i] != quote) {
                        malformed = true;
                        break;
                    }
                }
                ++count;
            }
            if (!malformed) {
                return count;
            }
        }
        context_.report(sql_analyzer_code::malformed_quoted_string,
                string_builder {}
                        << "invalid quoted string \"" << q << "\""
                        << string_builder::to_string,
                string.region());
        return {};
    }

    template<class T>
    static void append_quoted_string_unchecked(ast::common::chars const& from, T& destination) {
        constexpr char quote = ast::literal::string::quote_character;
        for (auto iter = from.begin() + 1, end = from.end() - 1; iter != end; ++iter) {
            auto c = *iter;
            destination.append(1, c);
            if (c == quote) {
                ++iter;
            }
        }
    }

    [[nodiscard]] std::unique_ptr<tscalar::immediate> process_character_string(ast::literal::string const& value) {
        std::size_t nchars = 0;
        if (auto n = count_characters(value.value())) {
            nchars += *n;
        } else {
            return {};
        }
        for (auto&& v : value.concatenations()) {
            if (auto n = count_characters(v)) {
                nchars += *n;
            } else {
                return {};
            }
        }

        tvalue::character::entity_type string {};
        string.reserve(nchars);

        append_quoted_string_unchecked(*value.value(), string);
        for (auto&& v : value.concatenations()) {
            append_quoted_string_unchecked(*v, string);
        }
        std::optional<std::size_t> size {};
        if (context_.options()->prefer_small_character_literals()) {
            size = nchars;
        }
        return context_.create<tscalar::immediate>(
                value.region(),
                context_.values().get(tvalue::character { std::move(string) }),
                context_.types().get(ttype::character {
                        ttype::varying,
                        size,
                }));
    }

    std::unique_ptr<tscalar::immediate> process_date(ast::literal::datetime const& value) {
        std::string_view contents { *value.value() };
        contents.remove_prefix(1);
        contents.remove_suffix(1);
        auto result = ::takatori::datetime::parse_date(contents);
        if (!result) {
            context_.report(
                    sql_analyzer_code::unsupported_string_value,
                    string_builder {}
                            << "invalid date literal: "
                            << contents
                            << " (" << result.error() << ")"
                            << string_builder::to_string,
                    value.value().region());
            return {};
        }
        return context_.create<tscalar::immediate>(
                value.region(),
                context_.values().get(tvalue::date { convert(result.value()) }),
                context_.types().get(ttype::date {}));
    }

    static ::takatori::datetime::date convert(::takatori::datetime::date_info const& info) {
        return ::takatori::datetime::date {
                static_cast<std::int32_t>(info.year),
                static_cast<std::uint32_t>(info.month),
                static_cast<std::uint32_t>(info.day),
        };
    }

    std::unique_ptr<tscalar::immediate> process_time(ast::literal::datetime const& value) {
        std::string_view contents { *value.value() };
        contents.remove_prefix(1);
        contents.remove_suffix(1);
        auto result = ::takatori::datetime::parse_time(contents);
        if (!result) {
            context_.report(
                    sql_analyzer_code::unsupported_string_value,
                    string_builder {}
                            << "invalid time literal: "
                            << contents
                            << " (" << result.error() << ")"
                            << string_builder::to_string,
                    value.value().region());
            return {};
        }
        return context_.create<tscalar::immediate>(
                value.region(),
                context_.values().get(tvalue::time_of_day { convert(result.value()) }),
                context_.types().get(ttype::time_of_day {}));
    }

    static ::takatori::datetime::time_of_day convert(::takatori::datetime::time_info const& info) {
        return ::takatori::datetime::time_of_day {
                static_cast<std::uint32_t>(info.hour),
                static_cast<std::uint32_t>(info.minute),
                static_cast<std::uint32_t>(info.second),
                info.subsecond,
        };
    }

    std::unique_ptr<tscalar::immediate> process_timestamp(ast::literal::datetime const& value, bool with_tz) {
        std::string_view contents { *value.value() };
        contents.remove_prefix(1);
        contents.remove_suffix(1);
        auto result = ::takatori::datetime::parse_datetime(contents);
        if (!result) {
            context_.report(
                    sql_analyzer_code::unsupported_string_value,
                    string_builder {}
                            << "invalid datetime literal: "
                            << contents
                            << " (" << result.error() << ")"
                            << string_builder::to_string,
                    value.value().region());
            return {};
        }
        if (!with_tz && result.value().offset) {
            context_.report(
                    sql_analyzer_code::unsupported_string_value,
                    string_builder {}
                            << "invalid datetime literal: "
                            << contents
                            << " (unexpected zone offset)"
                            << string_builder::to_string,
                    value.value().region());
            return {};
        }
        auto&& info = result.value();
        return context_.create<tscalar::immediate>(
                value.region(),
                context_.values().get(tvalue::time_point { convert(info) }),
                context_.types().get(ttype::time_point {}));
    }

    ::takatori::datetime::time_point convert(::takatori::datetime::datetime_info const& info) {
        ::takatori::datetime::time_point ts { convert(info.date), convert(info.time) };
        auto tz = convert(info.offset);
        ts -= tz;
        return ts;
    }

    sql_analyzer_options::zone_offset_type convert(std::optional<::takatori::datetime::zone_offset_info> const& info) {
        using duration_type = sql_analyzer_options::zone_offset_type;
        if (info) {
            auto&& v = info.value();
            duration_type tz {};
            tz += std::chrono::duration_cast<duration_type>(std::chrono::hours { v.hour });
            tz += std::chrono::duration_cast<duration_type>(std::chrono::minutes { v.minute });
            if (!v.plus) {
                tz = -tz;
            }
            return tz;
        }
        return context_.options()->system_zone_offset();
    }
};

} // namespace

std::unique_ptr<::takatori::scalar::expression> analyze_literal(
        analyzer_context& context,
        ast::literal::literal const& literal,
        scalar_value_context const& value_context) {
    engine e { context, value_context };
    return e.process(literal);
}

} // namespace mizugaki::analyzer::details
