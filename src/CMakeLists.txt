set(FLEX_FLAGS_DEBUG "-d")
set(FLEX_FLAGS_RELEASE "-Cfe")
set(FLEX_FLAGS_RELWITHDEBINFO "-Cfe")
set(FLEX_FLAGS_MINSIZEREL "-Ce")

set(BISON_FLAGS_DEBUG "-Wall -Werror")
set(BISON_FLAGS_RELEASE "-Wall -Werror")
set(BISON_FLAGS_RELWITHDEBINFO "-Wall -Werror")
set(BISON_FLAGS_MINSIZEREL "-Wall -Werror")

string(TOUPPER ${CMAKE_BUILD_TYPE} UPPER_CMAKE_BUILD_TYPE)
set(FLEX_FLAGS "${FLEX_FLAGS_${UPPER_CMAKE_BUILD_TYPE}}")
set(BISON_FLAGS "${BISON_FLAGS_${UPPER_CMAKE_BUILD_TYPE}}")

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/mizugaki/parser)

FLEX_TARGET(sql_scanner
    mizugaki/parser/sql_scanner.ll
    ${CMAKE_CURRENT_BINARY_DIR}/mizugaki/parser/sql_scanner_generated.cpp
    COMPILE_FLAGS "${FLEX_FLAGS}"
)

BISON_TARGET(sql_parser
    mizugaki/parser/sql_parser.yy
    ${CMAKE_CURRENT_BINARY_DIR}/mizugaki/parser/sql_parser_generated.cpp
    COMPILE_FLAGS "${BISON_FLAGS}"
)

ADD_FLEX_BISON_DEPENDENCY(
    sql_scanner
    sql_parser
)

add_library(mizugaki

    # AST models
    mizugaki/ast/node_region.cpp
    mizugaki/ast/compilation_unit.cpp

    mizugaki/ast/common/sort_element.cpp
    mizugaki/ast/common/target_element.cpp

    mizugaki/ast/name/simple.cpp
    mizugaki/ast/name/qualified.cpp

    mizugaki/ast/literal/boolean.cpp
    mizugaki/ast/literal/numeric.cpp
    mizugaki/ast/literal/string.cpp
    mizugaki/ast/literal/datetime.cpp
    mizugaki/ast/literal/interval.cpp

    mizugaki/ast/type/simple.cpp
    mizugaki/ast/type/character_string.cpp
    mizugaki/ast/type/bit_string.cpp
    mizugaki/ast/type/decimal.cpp
    mizugaki/ast/type/binary_numeric.cpp
    mizugaki/ast/type/datetime.cpp
    mizugaki/ast/type/interval.cpp

    mizugaki/ast/scalar/literal_expression.cpp
    mizugaki/ast/scalar/variable_reference.cpp
    mizugaki/ast/scalar/field_reference.cpp
    mizugaki/ast/scalar/case_expression.cpp
    mizugaki/ast/scalar/case_when_clause.cpp
    mizugaki/ast/scalar/cast_expression.cpp
    mizugaki/ast/scalar/unary_expression.cpp
    mizugaki/ast/scalar/binary_expression.cpp
    mizugaki/ast/scalar/value_constructor.cpp
    mizugaki/ast/scalar/comparison_predicate.cpp
    mizugaki/ast/scalar/between_predicate.cpp
    mizugaki/ast/scalar/in_predicate.cpp
    mizugaki/ast/scalar/pattern_match_predicate.cpp
    mizugaki/ast/scalar/function_invocation.cpp
    mizugaki/ast/scalar/set_function_invocation.cpp
    mizugaki/ast/scalar/builtin_function_invocation.cpp
    mizugaki/ast/scalar/builtin_set_function_invocation.cpp
    mizugaki/ast/scalar/new_invocation.cpp
    mizugaki/ast/scalar/method_invocation.cpp
    mizugaki/ast/scalar/static_method_invocation.cpp
    mizugaki/ast/scalar/subquery.cpp

    mizugaki/ast/table/table_reference.cpp
    mizugaki/ast/table/join.cpp
    mizugaki/ast/table/subquery.cpp
    mizugaki/ast/table/correlation_clause.cpp

    mizugaki/ast/query/query.cpp
    mizugaki/ast/query/table_reference.cpp
    mizugaki/ast/query/table_value_constructor.cpp
    mizugaki/ast/query/binary_expression.cpp
    mizugaki/ast/query/with_expression.cpp
    mizugaki/ast/query/select_column.cpp
    mizugaki/ast/query/select_asterisk.cpp
    mizugaki/ast/query/select_all_fields.cpp
    mizugaki/ast/query/group_by_clause.cpp
    mizugaki/ast/query/grouping_column.cpp
    mizugaki/ast/query/corresponding_clause.cpp
    mizugaki/ast/query/with_element.cpp

    mizugaki/ast/statement/select_statement.cpp
    mizugaki/ast/statement/insert_statement.cpp
    mizugaki/ast/statement/update_statement.cpp
    mizugaki/ast/statement/delete_statement.cpp
    mizugaki/ast/statement/empty_statement.cpp
    mizugaki/ast/statement/set_element.cpp

    # SQL parser
    mizugaki/parser/sql_parser.cpp
    mizugaki/parser/sql_parser_result.cpp
    mizugaki/parser/sql_scanner.cpp
    mizugaki/parser/sql_driver.cpp
    ${FLEX_sql_scanner_OUTPUTS}
    ${BISON_sql_parser_OUTPUTS}

    # common tools
    mizugaki/placeholder_map.cpp
    mizugaki/placeholder_entry.cpp

    # shakujo IR translator
    mizugaki/translator/shakujo_translator.cpp
    mizugaki/translator/shakujo_translator_options.cpp
    mizugaki/translator/shakujo_translator_result.cpp
    mizugaki/translator/shakujo_translator_impl.cpp
    mizugaki/translator/details/relation_info.cpp
    mizugaki/translator/details/variable_scope.cpp
    mizugaki/translator/details/type_info_translator.cpp
    mizugaki/translator/details/value_info_translator.cpp
    mizugaki/translator/details/type_translator.cpp
    mizugaki/translator/details/scalar_expression_translator.cpp
    mizugaki/translator/details/relation_expression_translator.cpp
    mizugaki/translator/details/aggregate_analyzer.cpp
    mizugaki/translator/details/dml_statement_translator.cpp
)

target_include_directories(mizugaki
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${FLEX_INCLUDE_DIRS}
)

target_link_libraries(mizugaki
    PUBLIC mizugaki-api
    PUBLIC takatori
    PUBLIC yugawara
    PUBLIC shakujo-model # FIXME:transitive
    PRIVATE tsl::hopscotch_map
)

set_target_properties(mizugaki
    PROPERTIES
        SOVERSION ${PROJECT_VERSION}
)

# Add INSTALL_RPATH from CMAKE_INSTALL_PREFIX and CMAKE_PREFIX_PATH
# The default behavior of CMake omits RUNPATH if it is already in CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES.
if (FORCE_INSTALL_RPATH)
    get_target_property(rpath mizugaki INSTALL_RPATH)

    # add ${CMAKE_INSTALL_PREFIX}/lib if it is not in system link directories
    get_filename_component(p "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" ABSOLUTE)
    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${p}" is_system)
    if (is_system STREQUAL "-1")
        list(APPEND rpath "${p}")
    endif()

    # add each ${CMAKE_PREFIX_PATH}/lib
    foreach (p IN LISTS CMAKE_PREFIX_PATH)
        get_filename_component(p "${p}/${CMAKE_INSTALL_LIBDIR}" ABSOLUTE)
        list(APPEND rpath "${p}")
    endforeach()

    if (rpath)
        set_target_properties(mizugaki
            PROPERTIES
            INSTALL_RPATH "${rpath}"
        )
    endif()

    # add other than */lib paths
    set_target_properties(mizugaki
        PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH ON
    )
endif (FORCE_INSTALL_RPATH)

install(
    TARGETS
        mizugaki
    EXPORT
        ${export_name}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Runtime
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${export_name}
        COMPONENT Development
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Runtime
)

# for tests
add_library(mizugaki-impl INTERFACE)

target_include_directories(mizugaki-impl
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
    INTERFACE ${FLEX_INCLUDE_DIRS}
)

target_link_libraries(mizugaki-impl
    INTERFACE mizugaki
    INTERFACE tsl::hopscotch_map
)
