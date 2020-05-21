function (add_test_executable source_file)
    get_filename_component(test_name "${source_file}" NAME_WE)
    add_executable(${test_name}
        ${source_file}
    )
    target_include_directories(${test_name}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(${test_name}
        PRIVATE mizugaki-impl
        PRIVATE mizugaki-test-main
    )
    add_test(
        NAME ${test_name}
        COMMAND ${test_name} --gtest_output=xml:${test_name}_gtest_result.xml
    )
endfunction (add_test_executable)

add_library(mizugaki-test-main STATIC
    main.cpp
)

target_link_libraries(mizugaki-test-main
    PUBLIC gtest
)

add_test_executable(mizugaki/translator/shakujo_translator_test.cpp)
add_test_executable(mizugaki/translator/details/type_info_translator_test.cpp)
add_test_executable(mizugaki/translator/details/value_info_translator_test.cpp)
add_test_executable(mizugaki/translator/details/type_translator_test.cpp)
add_test_executable(mizugaki/translator/details/scalar_expression_translator_test.cpp)
add_test_executable(mizugaki/translator/details/relation_expression_translator_test.cpp)
add_test_executable(mizugaki/translator/details/dml_statement_translator_test.cpp)