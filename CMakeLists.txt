cmake_minimum_required(VERSION 3.16)

project(mizugaki-project
    VERSION 1.0.0
    DESCRIPTION "Mizugaki - A SQL compiler front end"
    LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

option(BUILD_TESTS "build test programs" ON)
option(BUILD_DOCUMENTS "build documents" ON)
option(BUILD_EXAMPLES "build examples" ON)
option(INSTALL_EXAMPLES "install examples" OFF)
option(BUILD_SHARED_LIBS "build shared libraries instead of static" ON)
option(BUILD_STRICT "build with option strictly determine of success" ON)

option(FORCE_INSTALL_RPATH "automatically add library directory of custom prefixes to INSTALL_RPATH" OFF)

if (FORCE_INSTALL_RPATH)
    message(DEPRECATION "FORCE_INSTALL_RPATH is obsoleted")
endif (FORCE_INSTALL_RPATH)

option(ENABLE_COVERAGE "enable coverage on debug build" OFF)

include(GNUInstallDirs)

find_package(takatori REQUIRED)
find_package(yugawara REQUIRED)
find_package(mpdecpp REQUIRED)

find_package(tsl-hopscotch-map 2.2 REQUIRED)
find_package(FLEX REQUIRED)
find_package(BISON 3.6 REQUIRED)
find_package(gflags REQUIRED)
find_package(Threads REQUIRED)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.65
    COMPONENTS container
    REQUIRED)

find_package(Doxygen
    OPTIONAL_COMPONENTS dot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (BUILD_STRICT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-sanitize=alignment")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-sanitize-recover=address")

if (ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
endif (ENABLE_COVERAGE)

set(export_name "mizugaki")
set(package_name "mizugaki")

include(CMakePackageConfigHelpers)

enable_testing()

add_library(mizugaki-api INTERFACE)

target_include_directories(mizugaki-api
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${export_name}>
)

add_subdirectory(src)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
if (BUILD_DOCUMENTS)
    add_subdirectory(doxygen)
endif()
if (BUILD_EXAMPLES OR INSTALL_EXAMPLES)
    add_subdirectory(examples)
endif()
add_subdirectory(third_party)

install(
    TARGETS
        mizugaki-api
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

install(
    DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${export_name}"
    COMPONENT Development
    FILES_MATCHING PATTERN "*.h"
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${package_name}-config.cmake
    @ONLY
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${package_name}-config-version.cmake"
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${package_name}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${package_name}-config-version.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/${package_name}
)

install(
    EXPORT ${package_name}
    FILE ${package_name}-targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${package_name}
    EXPORT_LINK_INTERFACE_LIBRARIES
)
