#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

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
            ::takatori::util::unique_object_ptr<name::name> target,
            ::takatori::util::unique_object_ptr<name::simple> indicator = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit target_element(target_element const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit target_element(target_element&& other, ::takatori::util::object_creator creator);

    /**
     * @brief returns the target name.
     * @return the target name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& target() noexcept;

    /// @copydoc target()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& target() const noexcept;

    /**
     * @brief returns the optional indicator target.
     * @return the indicator target
     * @return empty if it is not declared
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple>& indicator() noexcept;

    /// @copydoc indicator()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple> const& indicator() const noexcept;

private:
    clone_wrapper<::takatori::util::unique_object_ptr<name::name>> target_;
    clone_wrapper<::takatori::util::unique_object_ptr<name::simple>> indicator_;
};

} // namespace mizugaki::ast::common
