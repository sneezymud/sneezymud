include_guard()

# cmake/CompilerOptions.cmake
# Compiler warning flags, debug options, ccache, PCH, and IWYU support

# ccache support - auto-enabled if found (speeds up clean rebuilds and branch switching)
# NO_CACHE ensures we always check for ccache, even when reusing a cached CMake configuration
# (otherwise a cached "not found" result prevents ccache from being used after installation)
find_program(CCACHE_PROGRAM ccache NO_CACHE)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
endif()

# Precompiled headers (PCH) - stable STL/Boost headers only
# These rarely change and are included by most source files
# Note: PCH and ccache are incompatible - PCH invalidates ccache on every clean build
# ccache provides better overall build performance, so PCH is disabled by default
option(SNEEZY_ENABLE_PCH "Enable precompiled headers (breaks ccache)" OFF)
if(SNEEZY_ENABLE_PCH)
    set(SNEEZY_PCH_HEADERS
        <algorithm>
        <cstdio>
        <cstdlib>
        <cstring>
        <ctime>
        <fstream>
        <functional>
        <list>
        <map>
        <memory>
        <optional>
        <queue>
        <set>
        <sstream>
        <string>
        <unordered_map>
        <vector>
        # Boost headers (commonly used throughout codebase)
        <boost/algorithm/string.hpp>
        <boost/regex.hpp>
    )
else()
    set(SNEEZY_PCH_HEADERS "")
    message(STATUS "PCH disabled")
endif()

# IWYU fix target - runs IWYU analysis and automatically applies fixes
# Usage: cmake --build build --target iwyu-fix
find_program(IWYU_TOOL iwyu_tool.py)
find_program(FIX_INCLUDES fix_includes.py)
if(IWYU_TOOL AND FIX_INCLUDES)
    # IWYU options:
    #   --cxx17ns: C++17 nested namespace syntax for forward declarations
    # Note: --no_comments removes line numbers needed by fix_includes.py, so only use for check
    set(IWYU_ARGS_BASE "-Xiwyu;--cxx17ns")

    # Project-specific mappings (if present)
    if(EXISTS "${CMAKE_SOURCE_DIR}/iwyu.imp")
        list(APPEND IWYU_ARGS_BASE "-Xiwyu;--mapping_file=${CMAKE_SOURCE_DIR}/iwyu.imp")
    endif()

    # Fix target needs line numbers in output (no --no_comments)
    string(REPLACE ";" " " IWYU_ARGS_FIX "${IWYU_ARGS_BASE}")

    # Check target can use cleaner output without line number comments
    set(IWYU_ARGS_CHECK "${IWYU_ARGS_BASE};-Xiwyu;--no_comments")
    string(REPLACE ";" " " IWYU_ARGS_CHECK "${IWYU_ARGS_CHECK}")

    add_custom_target(iwyu-fix
        COMMAND ${CMAKE_COMMAND} -E echo "Running IWYU analysis and applying fixes..."
        COMMAND sh -c "${IWYU_TOOL} -j 0 -p ${CMAKE_BINARY_DIR} -- ${IWYU_ARGS_FIX} 2>&1 | grep -v 'unicode-data.h.*warning.*no private include name' | ${FIX_INCLUDES}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running include-what-you-use and applying fixes"
        VERBATIM
    )

    add_custom_target(iwyu-check
        COMMAND ${CMAKE_COMMAND} -E echo "Running IWYU analysis (dry run)..."
        COMMAND sh -c "${IWYU_TOOL} -j 0 -p ${CMAKE_BINARY_DIR} -- ${IWYU_ARGS_CHECK} 2>&1 | grep -v 'unicode-data.h.*warning.*no private include name'"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running include-what-you-use (analysis only, no fixes applied)"
        VERBATIM
    )
    message(STATUS "IWYU targets available: iwyu-fix, iwyu-check")
endif()

# Create interface library for common compile options
add_library(sneezy_compiler_options INTERFACE)
add_library(sneezy::compiler_options ALIAS sneezy_compiler_options)

# Base compiler warnings (both GCC and Clang)
# Commented out or disabled warnings represent best practices that *should* be enabled, but currently produce too much noise.
# Each of them should be enabled (or removed) one at a time, with any resulting warnings fixed as they come up.
target_compile_options(sneezy_compiler_options INTERFACE
    -fdiagnostics-color=always
    -Wall
    -Wcast-align
    -Wcast-qual
    #-Wconversion
    -Wdisabled-optimization
    #-Wdouble-promotion
    -Wextra

    # ---
    # Enable -Wformat=2, remove -Wno-format, and fix all format string issues in single pass
    #-Wformat=2
    -Wno-format
    # ---

    -Wnon-virtual-dtor
    -Wnull-dereference
    -Woverloaded-virtual
    -Wpedantic
    -Wpointer-arith
    #-Wredundant-decls
    #-Wshadow
    -Wstring-compare
    #-Wsuggest-override
    -Wuninitialized
    -Wunknown-pragmas
    -Wwrite-strings
    # Disabled warnings (to be fixed later)
    # If -Wformat is enabled, -Wformat-nonliteral warns if the format string is not a string literal and so cannot be checked, unless the format function takes its format arguments as a va_list.
    -Wno-format-nonliteral
    -Wno-ignored-qualifiers
    -Wno-unused-parameter
    -Wno-unused-result
)

# GCC-specific warnings
target_compile_options(sneezy_compiler_options INTERFACE
    $<$<CXX_COMPILER_ID:GNU>:
        -funsafe-loop-optimizations
        -Wanalyzer-possible-null-dereference
        #-Wduplicated-branches
        -Wduplicated-cond
        -Wformat-overflow=2
        -Wformat-truncation=2

        # ---
        # Enable -Wimplicit-fallthrough=5, remove -Wno-implicit-fallthrough, and fix all fallthrough cases in single pass
        #-Wimplicit-fallthrough=5
        -Wno-implicit-fallthrough
        # ---

        -Wlogical-op
        -Wunsafe-loop-optimizations
        #-Wuseless-cast
    >
)

# Clang-specific warnings
target_compile_options(sneezy_compiler_options INTERFACE
    $<$<CXX_COMPILER_ID:Clang>:
        -ferror-limit=0
        -Wc++23-extensions
        -Wctad-maybe-unsupported
        -Wformat-non-iso
        -Wformat-pedantic
        #-Wimplicit-fallthrough
        -Wno-inconsistent-missing-override
        -Wrange-loop-construct
    >
)

# Clang warnings requiring minimum version (not available in older distro Clang packages)
target_compile_options(sneezy_compiler_options INTERFACE
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19>>:
        -Wc++23-compat
    >
)

# Debug-specific compile options (for better debugging and sanitizer stack traces)
# Use debugger-native formats: -ggdb3 for GCC/GDB, -glldb for Clang/LLDB
target_compile_options(sneezy_compiler_options INTERFACE
    $<$<CONFIG:Debug>:
        -O0
    >
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:
        -fno-common
    >
    $<$<AND:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,$<CXX_COMPILER_ID:GNU>>:
        -ggdb3
    >
    $<$<AND:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,$<CXX_COMPILER_ID:Clang>>:
        -glldb
        -fno-limit-debug-info
    >
)

# Debug symbols for linker
target_link_options(sneezy_compiler_options INTERFACE
    $<$<AND:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,$<CXX_COMPILER_ID:GNU>>:
        -ggdb3
    >
    $<$<AND:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,$<CXX_COMPILER_ID:Clang>>:
        -glldb
    >
)
