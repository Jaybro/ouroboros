cmake_minimum_required(VERSION 3.12)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/utils.cmake)

project(ouroboros
    LANGUAGES CXX
    VERSION 0.1.0
    DESCRIPTION "Ouroboros is a C++ header only library containing a cyclic deque, aka a circular buffer, that provides a cyclic view of a contiguous sequence."
    HOMEPAGE_URL "https://github.com/Jaybro/ouroboros")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(PROJECT_PACKAGE_NAME "Ouroboros")
add_subdirectory(src)

option(BUILD_EXAMPLES "Enable the creation of examples." ON)
message(STATUS "BUILD_EXAMPLES: ${BUILD_EXAMPLES}")

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

find_package(GTest QUIET)

if(GTEST_FOUND)
    include(CTest)
    message(STATUS "BUILD_TESTING: ${BUILD_TESTING}")

    if(BUILD_TESTING)
        enable_testing()
        add_subdirectory(test)
    endif()
else()
    message(STATUS "GTest not found. Unit tests cannot be build.")
endif()

find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)
    set(DOC_TARGET_NAME ${PROJECT_NAME}_doc)

    doxygen_add_docs(
        ${DOC_TARGET_NAME}
        src)

    message(STATUS "To build the Doxygen documentation: cmake --build . --target ${DOC_TARGET_NAME}")
else()
    message(STATUS "Doxygen not found. Documentation cannot be build.")
endif()
