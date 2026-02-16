include_guard()

# cmake/Sanitizers.cmake
# AddressSanitizer and UndefinedBehaviorSanitizer configuration
#
# WHY SANITIZERS IN PRODUCTION?
# -----------------------------
# SneezyMUD runs ASan and UBSan even in production builds. This is intentional:
#   1. Small, trusted playerbase - ~2x memory/CPU overhead is acceptable
#   2. Untested code often goes live - limited developer time means inadequate testing
#   3. Sanitizers catch bugs immediately - memory corruption, undefined behavior, overflows
#   4. Better than crashes - sanitizers provide clear stack traces vs mysterious segfaults
#   5. Volunteer development - inexperienced contributors benefit from runtime checks
#
# Recommended runtime environment variables for enhanced error reporting:
#   export ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1
#   export UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1   # halt_on_error=0 for production
#
# Production runs UBSan with halt_on_error=0, so undefined behavior is logged but
# doesn't crash the server. This allows catching bugs without disrupting players.
#
# WHY NO SECURITY HARDENING FLAGS?
# --------------------------------
# We don't use traditional hardening (_FORTIFY_SOURCE, stack protector, PIE):
#   1. _FORTIFY_SOURCE conflicts with ASan - Google's sanitizer wiki documents incompatibility
#   2. Stack protector is redundant - ASan catches all stack overflows and more
#   3. PIE adds ~3% overhead - for security-only benefit we don't need
#   4. Security is not a priority - small userbase, low-value target, trusted players
#   5. Sanitizers are superior - catch more bugs than all hardening flags combined

add_library(sneezy_sanitizers INTERFACE)
add_library(sneezy::sanitizers ALIAS sneezy_sanitizers)

# AddressSanitizer (ASan)
# Catches: buffer overflows, use-after-free, use-after-return, memory leaks
if(SNEEZY_ENABLE_ASAN)
    message(STATUS "AddressSanitizer enabled")

    target_compile_options(sneezy_sanitizers INTERFACE
        -fsanitize=address
        -fsanitize-address-use-after-scope
    )

    target_link_options(sneezy_sanitizers INTERFACE
        -fsanitize=address
        -fsanitize-address-use-after-scope
    )

    # Clang-only: enhanced use-after-return detection
    target_compile_options(sneezy_sanitizers INTERFACE
        $<$<CXX_COMPILER_ID:Clang>:-fsanitize-address-use-after-return=always>
    )
    target_link_options(sneezy_sanitizers INTERFACE
        $<$<CXX_COMPILER_ID:Clang>:-fsanitize-address-use-after-return=always>
    )

    # Ensure good stack traces even in optimized builds
    target_compile_options(sneezy_sanitizers INTERFACE
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
    )
endif()

# UndefinedBehaviorSanitizer (UBSan)
# Catches: signed overflow, null pointer dereference, type confusion, etc.
if(SNEEZY_ENABLE_UBSAN)
    message(STATUS "UndefinedBehaviorSanitizer enabled")

    target_compile_options(sneezy_sanitizers INTERFACE
        -fsanitize=undefined
        -fsanitize=float-divide-by-zero
    )

    target_link_options(sneezy_sanitizers INTERFACE
        -fsanitize=undefined
        -fsanitize=float-divide-by-zero
    )

    # Clang-only: additional UBSan checks
    # - signed-integer-overflow: arithmetic overflow (most valuable check)
    # - nullability: null pointer violations
    # - local-bounds: out-of-bounds array access
    #
    # NOTE: We avoid -fsanitize=integer group because it includes checks that
    # trigger false positives in third-party libraries:
    # - unsigned-integer-overflow: libstdc++ std::string::_S_compare() uses intentional wrap
    # - implicit-integer-sign-change: Boost.Regex char trait handling (unsigned char → char)
    target_compile_options(sneezy_sanitizers INTERFACE
        $<$<CXX_COMPILER_ID:Clang>:
            -fsanitize=signed-integer-overflow,nullability,local-bounds
        >
    )
    target_link_options(sneezy_sanitizers INTERFACE
        $<$<CXX_COMPILER_ID:Clang>:
            -fsanitize=signed-integer-overflow,nullability,local-bounds
        >
    )
endif()
