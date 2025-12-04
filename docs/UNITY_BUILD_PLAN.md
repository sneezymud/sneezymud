# Unity Build Implementation Plan

This document captures the analysis and recommendations for enabling CMake unity builds in the SneezyMUD codebase. Unity builds can significantly speed up clean builds (CI, Docker) but require fixing several codebase issues first.

**Status:** Future work (not yet started)
**Last Updated:** 2025-12
**Estimated Impact:** 30-50% faster clean builds

---

## Executive Summary

Unity builds combine multiple source files into single translation units, reducing compiler invocations and redundant header parsing. However, they expose latent bugs that are hidden when files compile separately. The SneezyMUD codebase has several issues that must be fixed before unity builds can be enabled.

---

## Issues Discovered

### 1. Static Initialization Order Fiasco (Critical) - ✅ FIXED

**Symptom:** Program hangs at startup in `std::shuffle` within `CardDeck::CardDeck()`

**Root Cause:** Global game objects are constructed before `main()`, and their constructors use a global RNG that may not be initialized yet.

**Solution Applied:** Implemented construct-on-first-use idiom (Meyer's singleton) in
`sys/random.h` and `sys/random.cc`. The `getRng()` function now returns a reference to
a function-local static `std::mt19937`, ensuring it's initialized on first use regardless
of static initialization order.

---

### 2. Missing Include Guards (Clang)

**Symptom:** `error: redefinition of 'DrawPokerGame'`

**Files Affected:**
- `game/game_drawpoker.h`
- `game/game_crazyeights.h`

**Fix:** Add proper `#pragma once` or include guards to these headers.

---

### 3. Duplicate Definitions (Clang)

**Symptom:** `error: redefinition of 'CART_VNUM'`

**Files Affected:**
- `spec/spec_mobs_limbDispo.cc:9`

**Root Cause:** `const` variables at namespace scope have internal linkage in C++, but if the same header is included in multiple files in a unity batch, or if a `.cc` file defines it and gets combined, you get duplicates.

**Fix:** Use `inline constexpr` for constants in headers, or move to anonymous namespace in `.cc` files.

---

### 4. Ambiguous Symbol References (Clang)

**Symptom:** `error: reference to 'byte' is ambiguous`

**Files Affected:**
- `disc/disc_cleric_cures.cc:1256`
- `disc/disc_cleric_wrath.cc:131`

**Root Cause:** The codebase defines a `byte` type that conflicts with `std::byte` (C++17). When files are combined in unity builds, both are visible.

**Fix:**
1. Rename the custom `byte` type to something like `ubyte` or `sneezy_byte`
2. Or fully qualify usages: `::byte` vs `std::byte`

---

### 5. Redefinition of Default Arguments

**Symptom:** `error: redefinition of default argument` in `range.h:10`

**Root Cause:** Default arguments appearing in multiple declarations when headers are combined.

**Fix:** Ensure default arguments only appear once (typically in the declaration, not definition).

---

### 6. Ambiguous Function Overloads

**Symptom:** `error: call to 'get_char_vis_direction' is ambiguous`

**Files Affected:**
- `misc/range.cc:163`
- `misc/range.cc:1295`

**Root Cause:** Multiple overloads become visible in unity builds that weren't visible before.

**Fix:** Use explicit qualification or rename functions to avoid ambiguity.

---

### 7. Duplicate Static/Const Variables

**Symptom:** `error: redefinition of 'MaxShapeShiftType'`

**Files Affected:**
- `misc/other.cc:1034`

**Fix:** Use `inline constexpr` or move to anonymous namespace.

---

## Implementation Plan

### Phase 1: Fix Critical Issues (Required)

1. ~~**Fix static initialization order** - Implement construct-on-first-use for `rng`~~ ✅ Done
2. **Add missing include guards** - Audit all headers in `game/`
3. **Fix duplicate definitions** - Convert `const` to `inline constexpr`

### Phase 2: Fix Clang-Specific Issues

4. **Resolve `byte` ambiguity** - Rename custom `byte` type
5. **Fix default argument redefinitions** - Audit `range.h`
6. **Resolve function overload ambiguity** - Fix `get_char_vis_direction`

### Phase 3: Enable Unity Builds

7. **Enable for release-gcc first** - GCC is more tolerant
8. **Test thoroughly** - Run verify_boot.sh, functional tests
9. **Enable for release-clang** - After all issues fixed

---

## Testing Strategy

1. **Before enabling unity builds:**
   ```bash
   # Baseline build time
   rm -rf build/release-gcc
   time cmake --preset release-gcc && time cmake --build --preset release-gcc
   ```

2. **Enable unity builds incrementally:**
   ```cmake
   # In CMakePresets.json, add to release presets:
   "SNEEZY_UNITY_BUILD": "ON"
   ```

3. **Verify functionality:**
   ```bash
   ./scripts/verify_boot.sh
   func-test/run_test.sh
   ```

---

## CMake Configuration (For Future Use)

When ready, add to `CMakeLists.txt`:
```cmake
option(SNEEZY_UNITY_BUILD "Enable unity build for faster compilation" OFF)
```

And in `code/code/CMakeLists.txt`:
```cmake
if(SNEEZY_UNITY_BUILD)
    set_target_properties(sneezy_lib PROPERTIES
        UNITY_BUILD ON
        UNITY_BUILD_BATCH_SIZE 50
    )
endif()
```

---

## Expected Benefits

| Scenario | Current | With Unity | Improvement |
|----------|---------|------------|-------------|
| CI clean build | ~3 min | ~2 min | ~30% |
| Docker build | ~5 min | ~3 min | ~40% |
| Local clean build | ~2 min | ~1.5 min | ~25% |

*Note: Incremental builds are slower with unity, so this is only for release/CI builds.*

---

## References

- [CMake Unity Builds](https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD.html)
- [Static Initialization Order Fiasco](https://en.cppreference.com/w/cpp/language/siof)
- [Include What You Use](https://include-what-you-use.org/)
