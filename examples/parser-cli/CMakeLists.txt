add_executable(parser-cli
    main.cpp
)

target_include_directories(parser-cli
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(parser-cli
    PRIVATE mizugaki
    PRIVATE gflags::gflags
    PRIVATE Threads::Threads
)

set(output_name "mizugaki-parser-cli")

set_target_properties(parser-cli
    PROPERTIES
        OUTPUT_NAME
            ${output_name}
)

set_target_properties(parser-cli
    PROPERTIES
        INSTALL_RPATH "\$ORIGIN/../lib"
)

if (INSTALL_EXAMPLES)
    install(
        TARGETS
            parser-cli
        EXPORT
            ${export_name}
        RUNTIME
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime
    )
endif()

add_test(
    NAME parser-cli
    COMMAND ${output_name} "-text" "SELECT * FROM T0;"
)
