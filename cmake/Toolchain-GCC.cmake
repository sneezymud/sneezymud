include_guard()

# cmake/Toolchain-GCC.cmake
# GCC-specific toolchain settings: LTO support with bfd linker
#
# WHY SEPARATE GCC/CLANG TOOLCHAINS?
# ----------------------------------
# Each compiler has an optimal toolchain configuration:
#   - GCC: bfd linker for full LTO support (linker plugin API)
#   - Clang: lld linker, ThinLTO for faster link times, additional sanitizer features
#
# WHY BFD LINKER FOR GCC?
# -----------------------
# GCC LTO requires full linker plugin support. The bfd linker (GNU ld) provides this.
# gold is deprecated as of Feb 2025 (removed from binutils 2.44).
# mold has limited LTO support (doesn't implement linker plugin API)
#
# WHY STATIC LIBRARIES (NOT SHARED)?
# ----------------------------------
# We use static libraries instead of shared libraries:
#   1. Zero configuration complexity - shared libs require RPATH, symbol visibility, etc.
#   2. LTO compatibility - static linking enables LTO
#   3. Single instance deployment - no memory sharing benefit with one production server
#   4. Extensive global state - ~100+ extern declarations would need refactoring

# Only apply if using GCC
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    return()
endif()

message(STATUS "Configuring GCC toolchain")

# LTO configuration for GCC
if(SNEEZY_ENABLE_LTO)
    # GCC LTO requires gcc-ar and gcc-ranlib
    find_program(GCC_AR gcc-ar)
    find_program(GCC_RANLIB gcc-ranlib)

    if(GCC_AR AND GCC_RANLIB)
        set(CMAKE_AR "${GCC_AR}" CACHE FILEPATH "Archiver" FORCE)
        set(CMAKE_RANLIB "${GCC_RANLIB}" CACHE FILEPATH "Ranlib" FORCE)

        # Use -flto=auto for parallel LTO (uses all available cores)
        target_compile_options(sneezy_toolchain INTERFACE -flto=auto)
        target_link_options(sneezy_toolchain INTERFACE -flto=auto -fuse-linker-plugin)
        message(STATUS "GCC LTO enabled with bfd linker")
    else()
        message(WARNING "LTO requested but gcc-ar/gcc-ranlib not found. LTO disabled.")
        set(SNEEZY_ENABLE_LTO OFF CACHE BOOL "" FORCE)
    endif()
endif()

# Static linking: check for undefined symbols at link time
target_link_options(sneezy_toolchain INTERFACE -Wl,-z,defs)
