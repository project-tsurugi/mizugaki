#pragma once

#include <vector>

#include <takatori/util/clonable.h>
#include <takatori/util/object_creator.h>

namespace mizugaki::ast::common {

/**
 * @brief a copyable wrapper.
 * @tparam T the element type
 */
template<class T>
class clone_wrapper;

/**
 * @brief a copyable wrapper for clonable unique_ptr.
 * @tparam E the element type
 */
template<class E>
class clone_wrapper<::takatori::util::unique_object_ptr<E>> {
public:
    /// @brief the element type.
    using element_type = E;

    /// @brief the smart pointer type.
    using target_type = ::takatori::util::unique_object_ptr<E>;

    /**
     * @brief creates a new empty object.
     */
    clone_wrapper() = default;

    /**
     * @brief creates a new object.
     * @param element the source element
     */
    constexpr clone_wrapper(target_type&& element) noexcept : // NOLINT
            element_ { std::move(element) }
    {}

    ~clone_wrapper() = default;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    clone_wrapper(clone_wrapper const& other) :
        element_ { other.clone() }
    {}

    /**
     * @brief assigns into this.
     * @param other the copy source
     * @return this
     */
    clone_wrapper& operator=(clone_wrapper const& other) {
        element_ = other.clone();
        return *this;
    }

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    clone_wrapper(clone_wrapper&& other) noexcept = default;

    /**
     * @brief assigns into this.
     * @param other the move source
     * @return this
     */
    clone_wrapper& operator=(clone_wrapper&& other) noexcept = default;

    /**
     * @brief returns the holding element.
     * @return the holding element
     */
    [[nodiscard]] target_type& element() noexcept {
        return element_;
    }

    /// @copydoc element()
    [[nodiscard]] target_type const& element() const noexcept {
        return element_;
    }

    /// @copydoc element()
    [[nodiscard]] target_type& operator*() noexcept {
        return element();
    }

    /// @copydoc element()
    [[nodiscard]] target_type const& operator*() const noexcept {
        return element();
    }

private:
    target_type element_ {};

    [[nodiscard]] target_type clone() const noexcept {
        return ::takatori::util::clone_unique(
                element_,
                ::takatori::util::get_object_creator_from_deleter(element_.get_deleter()));
    }
};

/**
 * @brief a copyable wrapper for clonable vector of unique_ptr.
 * @tparam E the element type
 */
template<class E>
class clone_wrapper<
        std::vector<
                ::takatori::util::unique_object_ptr<E>,
                ::takatori::util::object_allocator<::takatori::util::unique_object_ptr<E>>>> {
public:
    /// @brief the element type.
    using element_type = E;

    /// @brief the smart pointer type.
    using target_type = std::vector<
            ::takatori::util::unique_object_ptr<E>,
            ::takatori::util::object_allocator<::takatori::util::unique_object_ptr<E>>>;

    /**
     * @brief creates a new empty object.
     */
    clone_wrapper() = default;

    /**
     * @brief creates a new object.
     * @param element the source element
     */
    constexpr clone_wrapper(target_type&& element) noexcept : // NOLINT
        element_ { std::move(element) }
    {}

    ~clone_wrapper() = default;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    clone_wrapper(clone_wrapper const& other) :
        element_ { other.clone() }
    {}

    /**
     * @brief assigns into this.
     * @param other the copy source
     * @return this
     */
    clone_wrapper& operator=(clone_wrapper const& other) {
        element_ = other.clone();
    }

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    clone_wrapper(clone_wrapper&& other) noexcept = default;

    /**
     * @brief assigns into this.
     * @param other the move source
     * @return this
     */
    clone_wrapper& operator=(clone_wrapper&& other) noexcept {
        // NOTE: force propagate allocator
        std::destroy_at(&element_);
        new (&element_) decltype(element_)(std::move(other.element_));
        return *this;
    }

    /**
     * @brief returns the holding element.
     * @return the holding element
     */
    [[nodiscard]] target_type& element() noexcept {
        return element_;
    }

    /// @copydoc element()
    [[nodiscard]] target_type const& element() const noexcept {
        return element_;
    }

    /// @copydoc element()
    [[nodiscard]] target_type& operator*() noexcept {
        return element();
    }

    /// @copydoc element()
    [[nodiscard]] target_type const& operator*() const noexcept {
        return element();
    }

private:
    target_type element_ {};

    [[nodiscard]] target_type clone() const {
        target_type results { element_.get_allocator() };
        results.reserve(element_.size());
        for (auto&& e : element_) {
            results.emplace_back(::takatori::util::clone_unique(
                    e,
                    ::takatori::util::get_object_creator_from_deleter(e.get_deleter())));
        }
        return results;
    }
};

} // namespace mizugaki::ast::common