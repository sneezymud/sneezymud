# cmake/CompilerOptions.cmake
# Compiler warning flags, debug options, ccache, PCH, and IWYU support

# ccache support - auto-enabled if found (speeds up clean rebuilds and branch switching)
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
endif()

# Precompiled headers (PCH) - stable STL/Boost headers only
# These rarely change and are included by most source files
# PCH works alongside ccache (they cache different things)
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

# Include-What-You-Use (IWYU) - optional analysis tool for header cleanup
# Usage: cmake --preset dev-gcc -DSNEEZY_USE_IWYU=ON
option(SNEEZY_USE_IWYU "Run include-what-you-use during build (for header analysis)" OFF)
if(SNEEZY_USE_IWYU)
    find_program(IWYU_PROGRAM include-what-you-use)
    if(IWYU_PROGRAM)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PROGRAM}")
        message(STATUS "IWYU enabled: ${IWYU_PROGRAM}")
    else()
        message(WARNING "IWYU requested but include-what-you-use not found")
    endif()
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
        -Wc++20-compat-pedantic
        -Wc++20-extensions
        -Wctad-maybe-unsupported
        -Wformat-non-iso
        -Wformat-pedantic
        #-Wimplicit-fallthrough
        -Wno-inconsistent-missing-override
        -Wrange-loop-construct
    >
)

# Debug-specific compile options (for better debugging and sanitizer stack traces)
target_compile_options(sneezy_compiler_options INTERFACE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:
        -ggdb3
        -fno-common
        -fno-optimize-sibling-calls
        -fno-omit-frame-pointer
    >
)

# Clang needs explicit flag for full debug info on libstdc++ types
target_compile_options(sneezy_compiler_options INTERFACE
    $<$<AND:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,$<CXX_COMPILER_ID:Clang>>:
        -fno-limit-debug-info
    >
)

# Debug symbols for linker
target_link_options(sneezy_compiler_options INTERFACE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:
        -ggdb3
    >
)
