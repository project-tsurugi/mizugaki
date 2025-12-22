set(test_target mizugaki-test)

add_executable(${test_target}
    main.cpp
)

target_include_directories(${test_target}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${test_target}
    PUBLIC gtest
    PRIVATE mizugaki-impl
)

function (add_test_executable source_file)
    get_filename_component(test_name "${source_file}" NAME_WE)
    target_sources(${test_target}
        PRIVATE ${source_file}
    )
    add_test(
        NAME ${test_name}
        COMMAND ${test_target} --gtest_filter=${test_name}.* --gtest_output=xml:${test_name}_gtest_result.xml
    )
endfunction (add_test_executable)

function (add_analyzer_test_executable source_file)
    add_test_executable(${source_file})
    target_sources(${test_target}
            PRIVATE mizugaki/analyzer/details/test_parent.cpp
    )
endfunction (add_analyzer_test_executable)

# AST
add_test_executable(mizugaki/ast/node_region_test.cpp)
add_test_executable(mizugaki/ast/literal_dispatch_test.cpp)
add_test_executable(mizugaki/ast/name_dispatch_test.cpp)
add_test_executable(mizugaki/ast/type_dispatch_test.cpp)
add_test_executable(mizugaki/ast/scalar_dispatch_test.cpp)
add_test_executable(mizugaki/ast/table_dispatch_test.cpp)
add_test_executable(mizugaki/ast/query_dispatch_test.cpp)
add_test_executable(mizugaki/ast/statement_dispatch_test.cpp)

# SQL parser
add_test_executable(mizugaki/parser/sql_driver_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_element_kind_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_statement_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_query_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_table_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_predicate_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_scalar_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_function_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_literal_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_type_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_limit_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_misc_test.cpp)
add_test_executable(mizugaki/parser/sql_parser_error_test.cpp)
add_test_executable(mizugaki/parser/sql_tree_validator_test.cpp)

# SQL analyzer
add_test_executable(mizugaki/analyzer/sql_analyzer_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_literal_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_name_primary_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_name_qualified_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_type_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_scalar_expression_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_scalar_expression_predicate_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_scalar_expression_case_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_scalar_expression_function_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_scalar_expression_set_function_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_query_expression_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_query_expression_select_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_query_expression_join_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_query_expression_aggregate_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_query_expression_subquery_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_query_expression_binary_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_query_expression_apply_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_statement_dml_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_statement_select_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_statement_insert_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_statement_update_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_statement_delete_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_statement_ddl_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_statement_table_definition_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_statement_index_definition_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_statement_identity_column_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/analyze_description_test.cpp)
add_analyzer_test_executable(mizugaki/analyzer/details/set_function_processor_test.cpp)
