set(LIBSNEEZY_DIRS sys misc cmd disc game obj spec task)

set(LIBSNEEZY_SOURCES "")
set(LIBSNEEZY_INCLUDE_DIRS "")

foreach(dir ${LIBSNEEZY_DIRS})
    file(GLOB dir_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cc")
    list(APPEND LIBSNEEZY_SOURCES ${dir_sources})
    list(APPEND LIBSNEEZY_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
endforeach()

add_library(sneezy_lib STATIC ${LIBSNEEZY_SOURCES})
add_library(sneezy::lib ALIAS sneezy_lib)

if(SNEEZY_PCH_HEADERS)
    target_precompile_headers(sneezy_lib PRIVATE ${SNEEZY_PCH_HEADERS})
    message(STATUS "PCH enabled for sneezy_lib")
endif()

target_include_directories(sneezy_lib
    PUBLIC
        ${LIBSNEEZY_INCLUDE_DIRS}
)

# GCC needs --no-as-needed for libcrypt when ASan is enabled, otherwise the
# linker drops it as "unused" and crypt() calls fail at runtime. Scope it to
# just this library using push-state/pop-state instead of the global flag.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CRYPT_LINK_LIBRARY
        -Wl,--push-state,--no-as-needed
        ${CRYPT_LIBRARY}
        -Wl,--pop-state
    )
else()
    set(CRYPT_LINK_LIBRARY ${CRYPT_LIBRARY})
endif()

target_link_libraries(sneezy_lib
    PUBLIC
        Boost::program_options
        Boost::regex
        Boost::filesystem
        Boost::system
        PkgConfig::MARIADB
        CURL::libcurl
        ${CRYPT_LINK_LIBRARY}
        nlohmann_json::nlohmann_json
        sneezy::compiler_options
        sneezy::toolchain
        sneezy::sanitizers
)

add_executable(sneezy main.cc)

target_link_libraries(sneezy
    PRIVATE
        sneezy::lib
)

add_custom_command(TARGET sneezy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:sneezy>"
        "${CMAKE_SOURCE_DIR}/code/sneezy"
    COMMENT "Copied sneezy binary to code/sneezy"
)




list(LENGTH LIBSNEEZY_SOURCES SOURCE_COUNT)
message(STATUS "Core library: ${SOURCE_COUNT} source files from ${LIBSNEEZY_DIRS}")
