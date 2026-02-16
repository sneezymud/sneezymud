include(GoogleTest)

# Auto-discover test files — no need to update this file when adding new tests.
file(GLOB UNIT_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/unit/*_test.cc")

add_executable(sneezy_tests ${UNIT_TEST_SOURCES})

target_link_libraries(sneezy_tests
    PRIVATE
        sneezy::lib
        GTest::gtest_main
)

# WORKING_DIRECTORY set to lib/ for tests that load race data files.
# PRE_TEST defers discovery to ctest invocation, so sanitizer/env issues at
# discovery time don't break the build itself.
gtest_discover_tests(sneezy_tests
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
    DISCOVERY_MODE PRE_TEST
)
