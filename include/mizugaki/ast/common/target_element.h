#pragma once

#include <optional>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/name/simple.h>

namespace mizugaki::ast::common {

/**
 * @brief specifies output target.
 * @note `6.3 <value specification> and <target specification>` - `<target specification>`
 */
class target_element : public element {
public:
    /**
     * @brief creates a new instance.
     * @param target the target name
     * @param indicator the optional indicator name
     * @param region the element region
     */
    explicit target_element(
            std::unique_ptr<name::name> target,
            std::unique_ptr<name::simple> indicator = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit target_element(::takatori::util::clone_tag_t, target_element const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit target_element(::takatori::util::clone_tag_t, target_element&& other);

    /**
     * @brief returns the target name.
     * @return the target name
     */
    [[nodiscard]] std::unique_ptr<name::name>& target() noexcept;

    /// @copydoc target()
    [[nodiscard]] std::unique_ptr<name::name> const& target() const noexcept;

    /**
     * @brief returns the optional indicator target.
     * @return the indicator target
     * @return empty if it is not declared
     */
    [[nodiscard]] std::unique_ptr<name::simple>& indicator() noexcept;

    /// @copydoc indicator()
    [[nodiscard]] std::unique_ptr<name::simple> const& indicator() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(target_element const& a, target_element const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(target_element const& a, target_element const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            target_element const& value);

private:
    clone_wrapper<std::unique_ptr<name::name>> target_;
    clone_wrapper<std::unique_ptr<name::simple>> indicator_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, target_element const& value);

} // namespace mizugaki::ast::common
