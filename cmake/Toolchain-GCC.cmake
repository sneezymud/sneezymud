# cmake/Toolchain-GCC.cmake
# GCC-specific toolchain settings: fast linkers and LTO support
#
# WHY SEPARATE GCC/CLANG TOOLCHAINS?
# ----------------------------------
# Each compiler has an optimal toolchain configuration:
#   - GCC: mold/gold linker, gcc-ar/gcc-ranlib for LTO, classic LTO with auto parallelism
#   - Clang: lld linker, ThinLTO for faster link times, additional sanitizer features
#
# WHY STATIC LIBRARIES (NOT SHARED)?
# ----------------------------------
# We use static libraries with fast linkers instead of shared libraries:
#   1. Fast linkers provide similar link-time benefits - mold is 5-10x faster than GNU ld
#   2. Zero configuration complexity - shared libs require RPATH, symbol visibility, etc.
#   3. LTO compatibility - static linking enables LTO (10-20% performance gain)
#   4. Single instance deployment - no memory sharing benefit with one production server
#   5. Extensive global state - ~100+ extern declarations would need refactoring

# Only apply if using GCC
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    return()
endif()

message(STATUS "Configuring GCC toolchain")

# Try mold first (fastest), fall back to gold, then default ld
find_program(MOLD_LINKER mold)
find_program(GOLD_LINKER gold)

if(MOLD_LINKER)
    add_link_options(-fuse-ld=mold)
    message(STATUS "Using mold linker (5-10x faster than ld)")
elseif(GOLD_LINKER)
    add_link_options(-fuse-ld=gold)
    message(STATUS "Using gold linker (2-5x faster than ld)")
else()
    message(STATUS "Using default GNU ld linker (consider installing mold for faster builds)")
endif()

# LTO configuration for GCC
if(SNEEZY_ENABLE_LTO)
    # GCC LTO requires gcc-ar and gcc-ranlib
    find_program(GCC_AR gcc-ar)
    find_program(GCC_RANLIB gcc-ranlib)

    if(GCC_AR AND GCC_RANLIB)
        set(CMAKE_AR "${GCC_AR}" CACHE FILEPATH "Archiver" FORCE)
        set(CMAKE_RANLIB "${GCC_RANLIB}" CACHE FILEPATH "Ranlib" FORCE)

        # Use -flto=auto for parallel LTO (uses all available cores)
        add_compile_options(-flto=auto)
        add_link_options(-flto=auto -fuse-linker-plugin)
        message(STATUS "GCC LTO enabled with parallel compilation")
    else()
        message(WARNING "LTO requested but gcc-ar/gcc-ranlib not found. LTO disabled.")
        set(SNEEZY_ENABLE_LTO OFF CACHE BOOL "" FORCE)
    endif()
endif()

# GCC 10+ linker fix for crypt with ASan
# Without this, libcrypt may not be found at runtime
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "10")
    add_link_options(-Wl,--no-as-needed)
endif()

# Static linking: check for undefined symbols at link time
add_link_options(-Wl,-z,defs)
