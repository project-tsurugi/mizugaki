#pragma once

#include <memory>
#include <optional>
#include <type_traits>

#include <takatori/util/optional_ptr.h>

namespace mizugaki::analyzer::details {

template<class T>
class find_symbol_result {
public:
    static inline find_symbol_result const error = find_symbol_result(std::in_place); // NOLINT: constexpr requested but T may not be a literal type

    constexpr find_symbol_result( // NOLINT: implicit conversion
            std::optional<T> result = std::nullopt) noexcept(std::is_nothrow_move_constructible_v<std::optional<T>>) :
        found_ { std::move(result) }
    {}

    [[nodiscard]] constexpr bool is_error() const noexcept {
        return error_;
    }

    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return !is_error();
    }

    [[nodiscard]] constexpr std::optional<T>& found() noexcept {
        return found_;
    }

    [[nodiscard]] constexpr std::optional<T> const& found() const noexcept {
        return found_;
    }

private:
    explicit constexpr find_symbol_result(std::in_place_t) noexcept :
        error_ { true }
    {}

    std::optional<T> found_ { std::nullopt };
    bool error_ { false };
};

template<class T>
class find_symbol_result<T&> {
public:
    static inline find_symbol_result error = find_symbol_result(std::in_place); // NOLINT: constexpr requested but T may not be a literal type

    constexpr find_symbol_result( // NOLINT: implicit conversion
            ::takatori::util::optional_ptr<T> result = nullptr) :
        found_ { std::move(result) }
    {}

    [[nodiscard]] constexpr bool is_error() const noexcept {
        return error_;
    }

    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return !is_error();
    }

    [[nodiscard]] constexpr ::takatori::util::optional_ptr<T> found() noexcept {
        return found_;
    }

private:
    explicit constexpr find_symbol_result(std::in_place_t) noexcept :
        error_ { true }
    {}

    ::takatori::util::optional_ptr<T> found_ { nullptr };
    bool error_ { false };
};

template<class T>
class find_symbol_result<T*> {
public:
    static inline find_symbol_result error = find_symbol_result(std::in_place); // NOLINT: constexpr requested but T may not be a literal type

    constexpr find_symbol_result( // NOLINT: implicit conversion
            std::shared_ptr<T> result = nullptr) :
            found_ { std::move(result) }
    {}

    [[nodiscard]] constexpr bool is_error() const noexcept {
        return error_;
    }

    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return !is_error();
    }

    [[nodiscard]] constexpr std::shared_ptr<T> const& found() const & noexcept {
        return found_;
    }

    [[nodiscard]] constexpr std::shared_ptr<T> & found() & noexcept {
        return found_;
    }

    [[nodiscard]] constexpr std::shared_ptr<T> found() && noexcept {
        return std::move(found_);
    }

private:
    explicit constexpr find_symbol_result(std::in_place_t) noexcept :
            error_ { true }
    {}

    std::shared_ptr<T> found_ { nullptr };
    bool error_ { false };
};

} // namespace mizugaki::analyzer::details
