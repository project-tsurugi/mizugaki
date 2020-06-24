# Add INSTALL_RPATH from CMAKE_INSTALL_PREFIX and CMAKE_PREFIX_PATH
# The default behavior of CMake omits RUNPATH if it is already in CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES.
function (force_install_rpath target)
    get_target_property(rpath ${target} INSTALL_RPATH)

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
        set_target_properties(${target}
            PROPERTIES
            INSTALL_RPATH "${rpath}"
            )
    endif()

    # add other than */lib paths
    set_target_properties(${target}
        PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH ON
        )
endfunction(force_install_rpath target)