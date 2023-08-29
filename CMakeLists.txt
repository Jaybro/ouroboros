cmake_minimum_required(VERSION 3.12)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/utils.cmake)

project(ouroboros
    LANGUAGES CXX
    VERSION 0.1.0
    DESCRIPTION "Ouroboros is a C++ header only library containing a cyclic deque."
    HOMEPAGE_URL "https://github.com/Jaybro/ouroboros")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(PROJECT_PACKAGE_NAME "Ouroboros")
add_subdirectory(src)

option(BUILD_EXAMPLES "Enable the creation of examples." ON)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

include(CTest)
find_package(GTest QUIET)

if(BUILD_TESTING)
    if(GTEST_FOUND)
        enable_testing()
        add_subdirectory(test)
        message(STATUS "GTest and Eigen found. Building unit tests.")
    else()
        message(STATUS "GTest and/or Eigen not found. Unit tests will not be build.")
    endif()
endif()
