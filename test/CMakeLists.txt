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


## AST
add_test_executable(mizugaki/ast/node_region_test.cpp)

## shakujo IR translator
add_test_executable(mizugaki/translator/shakujo_translator_test.cpp)
add_test_executable(mizugaki/translator/details/type_info_translator_test.cpp)
add_test_executable(mizugaki/translator/details/value_info_translator_test.cpp)
add_test_executable(mizugaki/translator/details/type_translator_test.cpp)
add_test_executable(mizugaki/translator/details/scalar_expression_translator_test.cpp)
add_test_executable(mizugaki/translator/details/relation_expression_translator_test.cpp)
add_test_executable(mizugaki/translator/details/dml_statement_translator_test.cpp)
