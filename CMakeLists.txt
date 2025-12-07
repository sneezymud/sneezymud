cmake_minimum_required(VERSION 3.21)

project(SneezyMUD
    VERSION 1.0.0
    DESCRIPTION "Classic text-based MUD server"
    LANGUAGES CXX
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed. Please create a build directory:\n"
        "  cmake --preset dev-gcc\n"
        "Or manually:\n"
        "  mkdir build && cd build && cmake .."
    )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13")
        message(FATAL_ERROR
            "GCC ${CMAKE_CXX_COMPILER_VERSION} does not fully support C++20.\n"
            "Please upgrade to GCC 13 or later."
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17")
        message(FATAL_ERROR
            "Clang ${CMAKE_CXX_COMPILER_VERSION} does not fully support C++20.\n"
            "Please upgrade to Clang 17 or later."
        )
    endif()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type specified, defaulting to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

option(SNEEZY_ENABLE_ASAN "Enable AddressSanitizer (recommended for production)" ON)
option(SNEEZY_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer (dev builds)" OFF)
option(SNEEZY_ENABLE_LTO "Enable Link-Time Optimization (10-20% performance gain)" OFF)
option(SNEEZY_BUILD_LOWTOOLS "Build LOW admin tools (deprecated)" OFF)
option(SNEEZY_BUILD_CGI "Build CGI utilities (deprecated)" OFF)
option(SNEEZY_BUILD_SHELL "Build shell utilities (deprecated)" OFF)

message(STATUS "")
message(STATUS "SneezyMUD Build Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  ASan: ${SNEEZY_ENABLE_ASAN}")
message(STATUS "  UBSan: ${SNEEZY_ENABLE_UBSAN}")
message(STATUS "  LTO: ${SNEEZY_ENABLE_LTO}")
message(STATUS "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CompilerOptions)
include(Toolchain-GCC)
include(Toolchain-Clang)
include(Sanitizers)
include(Dependencies)
include(VersionInfo)
include(ClangTidy)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    message(STATUS "Checking git submodules...")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_SUBMOD_RESULT
    )
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(WARNING "git submodule update failed. JSON library may not be available.")
    endif()
endif()

if(CMAKE_EXPORT_COMPILE_COMMANDS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    file(RELATIVE_PATH COMPILE_COMMANDS_REL
        "${CMAKE_SOURCE_DIR}"
        "${CMAKE_BINARY_DIR}/compile_commands.json"
    )
    file(CREATE_LINK
        "${COMPILE_COMMANDS_REL}"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
        SYMBOLIC
    )
    message(STATUS "Symlinked compile_commands.json -> ${COMPILE_COMMANDS_REL}")
endif()

add_subdirectory(code)
