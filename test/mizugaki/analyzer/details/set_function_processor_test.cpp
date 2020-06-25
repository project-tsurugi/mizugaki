#include <mizugaki/analyzer/details/set_function_processor.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/values.h>

#include <yugawara/binding/extract.h>

#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/query/table_reference.h>
#include <mizugaki/ast/query/table_value_constructor.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class set_function_processor_test : public test_parent {};

TEST_F(set_function_processor_test, count_asterisk) {

}

} // namespace mizugaki::analyzer::details
