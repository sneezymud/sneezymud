# code/code/CMakeLists.txt
# Main library and executable definitions for SneezyMUD

# =============================================================================
# Core Library: sneezy_lib
# Contains all game logic from the 8 core directories
# =============================================================================

# Source directories (ordered by dependencies)
set(LIBSNEEZY_DIRS sys misc cmd disc game obj spec task)

# Collect sources from each directory
set(LIBSNEEZY_SOURCES "")
set(LIBSNEEZY_INCLUDE_DIRS "")

foreach(dir ${LIBSNEEZY_DIRS})
    file(GLOB dir_sources "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cc")
    list(APPEND LIBSNEEZY_SOURCES ${dir_sources})
    list(APPEND LIBSNEEZY_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
endforeach()

# Create the core static library
add_library(sneezy_lib STATIC ${LIBSNEEZY_SOURCES})
add_library(sneezy::lib ALIAS sneezy_lib)

# Precompiled headers for faster compilation
# SNEEZY_PCH_HEADERS is defined in cmake/CompilerOptions.cmake
if(SNEEZY_PCH_HEADERS)
    target_precompile_headers(sneezy_lib PRIVATE ${SNEEZY_PCH_HEADERS})
    message(STATUS "PCH enabled for sneezy_lib")
endif()

# Include directories
target_include_directories(sneezy_lib
    PUBLIC
        ${LIBSNEEZY_INCLUDE_DIRS}
)

# Link dependencies
target_link_libraries(sneezy_lib
    PUBLIC
        Boost::program_options
        Boost::regex
        Boost::filesystem
        Boost::system
        PkgConfig::MARIADB
        CURL::libcurl
        ${CRYPT_LIBRARY}
        nlohmann_json::nlohmann_json
    PRIVATE
        sneezy::compiler_options
        sneezy::sanitizers
)

# =============================================================================
# Main Executable: sneezy
# =============================================================================

add_executable(sneezy main.cc)

target_link_libraries(sneezy
    PRIVATE
        sneezy::lib
        sneezy::compiler_options
        sneezy::sanitizers
)

# Symlink the binary to code/sneezy for convenience (dev builds only)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET sneezy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            "$<TARGET_FILE:sneezy>"
            "${CMAKE_SOURCE_DIR}/code/sneezy"
        COMMENT "Symlinked code/sneezy -> $<TARGET_FILE:sneezy>"
    )
endif()

# =============================================================================
# Optional: LOW Tools (deprecated admin utilities)
# =============================================================================

if(SNEEZY_BUILD_LOWTOOLS)
    message(STATUS "Building LOW tools (deprecated)")

    # Shared lowtools code
    add_library(lowtools_common OBJECT low/lowtools.cc)
    target_link_libraries(lowtools_common
        PRIVATE
            sneezy::lib
            sneezy::compiler_options
            sneezy::sanitizers
    )

    # List of LOW tools
    set(LOWTOOL_NAMES
        amap
        sqladdmobs
        sqladdobjs
        sqladdresp
        sqladdwld
        sqledimmobj
        sqledmob
        sqledobj
        sqledresp
        sqledshop
        sqledwld
    )

    foreach(tool ${LOWTOOL_NAMES})
        add_executable(${tool} low/${tool}.cc $<TARGET_OBJECTS:lowtools_common>)
        target_link_libraries(${tool}
            PRIVATE
                sneezy::lib
                sneezy::compiler_options
                sneezy::sanitizers
        )
    endforeach()

    # Install to lowtools directory
    install(TARGETS ${LOWTOOL_NAMES}
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/code/lowtools
    )
endif()

# =============================================================================
# Optional: CGI Utilities (deprecated)
# =============================================================================

if(SNEEZY_BUILD_CGI)
    message(STATUS "Building CGI utilities (deprecated)")

    # Session handling object
    add_library(cgi_session OBJECT cgi/session.cgi.cc)
    target_link_libraries(cgi_session
        PRIVATE
            sneezy::lib
            sneezy::compiler_options
            sneezy::sanitizers
    )

    # mudmail.cgi
    add_executable(mudmail.cgi cgi/mudmail.cgi.cc $<TARGET_OBJECTS:cgi_session>)
    target_link_libraries(mudmail.cgi
        PRIVATE
            sneezy::lib
            PkgConfig::CGICC
            OpenSSL::Crypto
            sneezy::compiler_options
            sneezy::sanitizers
    )

    install(TARGETS mudmail.cgi
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/code/cgi
    )
endif()

# =============================================================================
# Optional: Shell Utilities (deprecated)
# =============================================================================

if(SNEEZY_BUILD_SHELL)
    message(STATUS "Building shell utilities (deprecated)")

    add_executable(econ shell/econ.cc)
    target_link_libraries(econ
        PRIVATE
            sneezy::lib
            sneezy::compiler_options
            sneezy::sanitizers
    )

    install(TARGETS econ
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/code/shell
    )
endif()

# =============================================================================
# Print source file count
# =============================================================================

list(LENGTH LIBSNEEZY_SOURCES SOURCE_COUNT)
message(STATUS "Core library: ${SOURCE_COUNT} source files from ${LIBSNEEZY_DIRS}")
