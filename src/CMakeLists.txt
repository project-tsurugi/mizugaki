add_library(mizugaki

    # common tools
    mizugaki/document_map.cpp
    mizugaki/placeholder_map.cpp
    mizugaki/placeholder_entry.cpp

    # shakujo IR translator
    mizugaki/translator/shakujo_translator.cpp
    mizugaki/translator/shakujo_translator_context.cpp
    mizugaki/translator/shakujo_translator_result.cpp
    mizugaki/translator/shakujo_translator_diagnostic.cpp
    mizugaki/translator/shakujo_translator_impl.cpp
    mizugaki/translator/shakujo_translator_context_impl.cpp
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
    PRIVATE .
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
    INTERFACE .
)

target_link_libraries(mizugaki-impl
    INTERFACE mizugaki
    INTERFACE tsl::hopscotch_map
)
