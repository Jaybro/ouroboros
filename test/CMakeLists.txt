include(GoogleTest)

set(TEST_TARGET_NAME ${PROJECT_NAME}_test)
add_executable(${TEST_TARGET_NAME})
set_default_target_properties(${TEST_TARGET_NAME})
set_target_properties(${TEST_TARGET_NAME}
    PROPERTIES

    # The library should be compliant with the C++17 standard.
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
include_directories(${CMAKE_CURRENT_LIST_DIR})

set(TEST_TARGET_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/cyclic_deque_test.cpp
)

target_sources(${TEST_TARGET_NAME} PRIVATE ${TEST_TARGET_SOURCES})
target_link_libraries(${TEST_TARGET_NAME}
    ${PROJECT_NAME}
    GTest::GTest
    GTest::Main
)

gtest_add_tests(
    TARGET ${TEST_TARGET_NAME}
    TEST_LIST ${TEST_TARGET_NAME}_list
)
