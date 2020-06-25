#include <mizugaki/analyzer/details/value_context.h>

#include <takatori/util/print_support.h>
#include <takatori/util/vector_print_support.h>

namespace mizugaki::analyzer::details {

value_context::value_context(scalar_value_context element) noexcept :
    alternatives_ { std::in_place_type<scalar_value_context>, std::move(element) }
{}

value_context::value_context(row_value_context element) noexcept :
    alternatives_ { std::in_place_type<row_value_context>, std::move(element) }
{}

bool value_context::available() const noexcept {
    return kind() != kind_type::unspecified;
}

value_context::operator bool() const noexcept {
    return available();
}

value_context::kind_type value_context::kind() const noexcept {
    switch (alternatives_.index()) {
        case static_cast<std::size_t>(kind_type::scalar): return kind_type::scalar;
        case static_cast<std::size_t>(kind_type::row): return kind_type::row;
    }
    return kind_type::unspecified;
}

value_context::position_type value_context::size() const noexcept {
    switch (kind()) {
        case kind_type::unspecified: return 0;
        case kind_type::scalar: return 1;
        case kind_type::row: return get<kind_type::row>().elements().size();
    }
    std::abort();
}

scalar_value_context const& value_context::find(value_context::position_type position) const {
    switch (kind()) {
        case kind_type::unspecified:
            break;
        case kind_type::scalar: {
            auto&& v = get<kind_type::scalar>();
            if (position == 0) {
                return v;
            }
            break;
        }
        case kind_type::row: {
            auto&& v = get<kind_type::row>();
            if (position < v.elements().size()) {
                return v.elements()[position];
            }
            break;
        }
    }
    static scalar_value_context const empty;
    return empty;
}

template<> scalar_value_context& value_context::get<value_context_kind::scalar>() {
    return std::get<scalar_value_context>(alternatives_);
}

template<> row_value_context& value_context::get<value_context_kind::row>() {
    return std::get<row_value_context>(alternatives_);
}

template<> scalar_value_context const& value_context::get<value_context_kind::scalar>() const {
    return std::get<scalar_value_context>(alternatives_);
}

template<> row_value_context const& value_context::get<value_context_kind::row>() const {
    return std::get<row_value_context>(alternatives_);
}

std::ostream& operator<<(std::ostream& out, value_context const& value) {
    using k = value_context_kind;
    switch (value.kind()) {
        case k::unspecified: return out << k::unspecified;
        case k::scalar: return out << value.get<k::scalar>();
        case k::row: return out << value.get<k::row>();
    }
    std::abort();
}

} // namespace mizugaki::analyzer::details
