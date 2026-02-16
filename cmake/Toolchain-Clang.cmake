include_guard()

# cmake/Toolchain-Clang.cmake
# Clang-specific toolchain settings: lld linker and ThinLTO support
#
# WHY SEPARATE GCC/CLANG TOOLCHAINS?
# ----------------------------------
# Each compiler has an optimal toolchain configuration:
#   - GCC: bfd linker, gcc-ar/gcc-ranlib for LTO, classic LTO with auto parallelism
#   - Clang: lld linker, ThinLTO for faster link times, additional sanitizer features
# See Toolchain-GCC.cmake for rationale on static libraries vs shared.

# Only apply if using Clang
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    return()
endif()

message(STATUS "Configuring Clang toolchain")

# Use lld for faster linking
find_program(LLD_LINKER ld.lld)

if(LLD_LINKER)
    target_link_options(sneezy_toolchain INTERFACE -fuse-ld=lld)
    message(STATUS "Using lld linker")
else()
    message(STATUS "lld not found, using default linker")
    message(STATUS "  Install lld and ensure 'ld.lld' is in PATH (e.g., sudo update-alternatives --install /usr/bin/ld.lld ld.lld /usr/bin/ld.lld-18 100)")
endif()

# ThinLTO configuration for Clang
# ThinLTO is faster than full LTO while providing most of the benefits
if(SNEEZY_ENABLE_LTO)
    target_compile_options(sneezy_toolchain INTERFACE -flto=thin)
    target_link_options(sneezy_toolchain INTERFACE -flto=thin)

    # Use LLVM ar/ranlib for LTO - system ar uses wrong plugin version
    find_program(LLVM_AR llvm-ar llvm-ar-20 llvm-ar-19 llvm-ar-18)
    find_program(LLVM_RANLIB llvm-ranlib llvm-ranlib-20 llvm-ranlib-19 llvm-ranlib-18)

    if(LLVM_AR AND LLVM_RANLIB)
        set(CMAKE_AR "${LLVM_AR}" CACHE FILEPATH "Archiver" FORCE)
        set(CMAKE_RANLIB "${LLVM_RANLIB}" CACHE FILEPATH "Ranlib" FORCE)
        message(STATUS "Clang ThinLTO enabled (using ${LLVM_AR})")
    else()
        message(WARNING "llvm-ar/llvm-ranlib not found - LTO may fail for static libraries")
        message(STATUS "Clang ThinLTO enabled")
    endif()
endif()

# Static linking: check for undefined symbols at link time
target_link_options(sneezy_toolchain INTERFACE -Wl,-z,defs)
