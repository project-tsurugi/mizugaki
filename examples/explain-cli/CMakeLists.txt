add_executable(explain-cli
    main.cpp
    options.cpp
    example_prototype_processor.cpp
    ddl_interpreter.cpp
)

target_include_directories(explain-cli
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(explain-cli
    PRIVATE mizugaki
    PRIVATE gflags::gflags
    PRIVATE Threads::Threads
)

set(output_name "mizugaki-explain-cli")

set_target_properties(explain-cli
    PROPERTIES
        OUTPUT_NAME
            ${output_name}
)

set_target_properties(explain-cli
    PROPERTIES
        INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
)

if (INSTALL_EXAMPLES)
    install(
        TARGETS
            explain-cli
        EXPORT
            ${export_name}
        RUNTIME
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime
    )
endif()

add_test(
    NAME explain-cli
    COMMAND ${output_name} "-text" "SELECT * FROM ksv;"
)

add_test(
    NAME explain-cli-features
    COMMAND ${output_name} "-features=none,+aggregate_exchange,-aggregate_exchange" "-text" "SELECT * FROM ksv;"
)

add_test(
    NAME explain-cli-features-unknown
    COMMAND ${output_name} "-features=+unknown_feature" "-text" "SELECT * FROM ksv;"
)
set_tests_properties(explain-cli-features-unknown
    PROPERTIES
        WILL_FAIL TRUE
)
