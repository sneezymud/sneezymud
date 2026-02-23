include(GoogleTest)

# Auto-discover test files — no need to update this file when adding new tests.
# After adding or removing *_test.cc files, re-run cmake configuration.
file(GLOB UNIT_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/*_test.cc")

add_executable(sneezy_tests ${UNIT_TEST_SOURCES})

target_link_libraries(sneezy_tests
    PRIVATE
        sneezy_lib
        GTest::gtest_main
        sneezy::compiler_options
        sneezy::sanitizers
)

# WORKING_DIRECTORY set to lib/ for tests that load race data files.
gtest_discover_tests(sneezy_tests
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
)
