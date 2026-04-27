include_guard()

# cmake/Dependencies.cmake
# External library detection and configuration

include(FetchContent)

# Boost (required)
# Version matches Ubuntu 24.04 LTS (production Docker image)
# Components: program_options, regex, filesystem, system
# Static linking eliminates the Boost shared library dependency at runtime,
# which simplifies the Docker image and prevents CI cache-hit failures where the
# cached binary can't find Boost shared libraries that were never installed.
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.83 REQUIRED COMPONENTS
    program_options
    regex
    filesystem
    system
)

# MariaDB via pkg-config (required)
# pkg_search_module tries multiple names - Some OSes use libmariadb instead of mariadb
find_package(PkgConfig REQUIRED)
pkg_search_module(MARIADB REQUIRED IMPORTED_TARGET mariadb libmariadb)

# cURL (required for Discord webhooks)
find_package(CURL REQUIRED)

# crypt library (required for password hashing)
find_library(CRYPT_LIBRARY crypt REQUIRED)

# nlohmann/json (header-only, fetched via FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(nlohmann_json)


# Google Test (fetched on demand, only when testing is enabled)
if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Print dependency summary
message(STATUS "Dependencies:")
message(STATUS "  Boost: ${Boost_VERSION}")
message(STATUS "  MariaDB: ${MARIADB_VERSION}")
message(STATUS "  cURL: ${CURL_VERSION_STRING}")
message(STATUS "  crypt: ${CRYPT_LIBRARY}")
