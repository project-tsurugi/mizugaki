if(TARGET mpdecpp)
    return()
endif()

find_library(mpdecpp_LIBRARY_FILE NAMES mpdec++)
find_library(mpdec_LIBRARY_FILE NAMES mpdec)
find_path(mpdecpp_INCLUDE_DIR NAMES decimal.hh)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mpdecpp DEFAULT_MSG
    mpdecpp_LIBRARY_FILE
    mpdec_LIBRARY_FILE
    mpdecpp_INCLUDE_DIR)

if(mpdec_LIBRARY_FILE AND mpdecpp_LIBRARY_FILE AND mpdecpp_INCLUDE_DIR)
    set(mpdecpp_FOUND ON)
    add_library(mpdecpp SHARED IMPORTED)
    target_link_libraries(mpdecpp
        INTERFACE "${mpdec_LIBRARY_FILE}"
    )
    set_target_properties(mpdecpp PROPERTIES
        IMPORTED_LOCATION "${mpdecpp_LIBRARY_FILE}"
        INTERFACE_INCLUDE_DIRECTORIES "${mpdecpp_INCLUDE_DIR}")
else()
    set(mpdecpp_FOUND OFF)
endif()

unset(mpdecpp_LIBRARY_FILE CACHE)
unset(mpdec_LIBRARY_FILE CACHE)
unset(mpdecpp_INCLUDE_DIR CACHE)
