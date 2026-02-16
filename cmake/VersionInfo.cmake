include_guard()

# cmake/VersionInfo.cmake
# Extract git version information and write to lib/version.txt

find_package(Git QUIET)

if(GIT_FOUND)
    # Get the current git commit hash
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE GIT_HASH_RESULT
    )

    # Check if working directory is dirty
    execute_process(
        COMMAND ${GIT_EXECUTABLE} status --short
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DIRTY_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if(GIT_DIRTY_OUTPUT)
        set(GIT_DIRTY "+")
    else()
        set(GIT_DIRTY "")
    endif()

    # Get the commit date
    execute_process(
        COMMAND ${GIT_EXECUTABLE} show -s --format=%cd --date=format:%Y-%m-%d HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if(GIT_HASH_RESULT EQUAL 0)
        message(STATUS "Git version: ${GIT_HASH}${GIT_DIRTY} (${GIT_DATE})")
    else()
        set(GIT_HASH "unknown")
        set(GIT_DIRTY "")
        set(GIT_DATE "unknown")
        message(WARNING "Could not determine git version")
    endif()
else()
    set(GIT_HASH "unknown")
    set(GIT_DIRTY "")
    set(GIT_DATE "unknown")
    message(WARNING "Git not found, version info unavailable")
endif()

# Write version file
# This matches the format expected by the existing code
file(WRITE ${CMAKE_SOURCE_DIR}/lib/version.txt
    "${GIT_HASH}${GIT_DIRTY}\n${GIT_DATE}"
)
