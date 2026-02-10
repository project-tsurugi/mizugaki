#include <mizugaki/ast/statement/identity_column_restart_option.h>

namespace mizugaki::ast::statement {

std::ostream& operator<<(std::ostream& out, identity_column_restart_option value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
