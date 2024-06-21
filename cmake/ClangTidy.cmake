# cmake/ClangTidy.cmake
# Optional clang-tidy target for code quality analysis
#
# Usage: cmake --build --preset dev-gcc --target clang-tidy
#
# This runs clang-tidy on all source files without blocking the main build.
# Results are informational - fix issues incrementally as you work on files.

find_program(CLANG_TIDY_PROGRAM clang-tidy)
find_program(RUN_CLANG_TIDY_PROGRAM run-clang-tidy)

if(CLANG_TIDY_PROGRAM)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_PROGRAM}")

    if(RUN_CLANG_TIDY_PROGRAM)
        # Use run-clang-tidy for parallel execution
        add_custom_target(clang-tidy
            COMMAND ${RUN_CLANG_TIDY_PROGRAM}
                -p ${CMAKE_BINARY_DIR}
                -quiet
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running clang-tidy on all source files..."
            VERBATIM
        )
    else()
        # Fallback: run clang-tidy directly (slower, sequential)
        add_custom_target(clang-tidy
            COMMAND ${CLANG_TIDY_PROGRAM}
                -p ${CMAKE_BINARY_DIR}
                ${CMAKE_SOURCE_DIR}/code/code/*/*.cc
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running clang-tidy on all source files (sequential)..."
            VERBATIM
        )
    endif()

    # Also add a target for running on a single file
    # Usage: cmake --build --preset dev-gcc --target clang-tidy-file -- FILE=path/to/file.cc
    add_custom_target(clang-tidy-file
        COMMAND ${CLANG_TIDY_PROGRAM}
            -p ${CMAKE_BINARY_DIR}
            $ENV{FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on single file..."
        VERBATIM
    )
else()
    message(STATUS "clang-tidy not found - clang-tidy target disabled")
endif()
