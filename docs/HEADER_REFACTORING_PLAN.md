# Header Refactoring Roadmap

This document captures the analysis and recommendations for cleaning up header dependencies in the SneezyMUD codebase. These changes would significantly improve compilation times but require careful refactoring.

**Status:** Future work (not yet started)
**Last Updated:** 2025-12
**Estimated Impact:** 40-60% faster clean builds

---

## Executive Summary

The codebase has accumulated header dependency issues over 40 years. The primary bottleneck is `being.h` (69KB), which is included by 289 files. Breaking up this monolithic header and optimizing include patterns could dramatically improve build times.

---

## Analysis Results

### File Sizes and Dependencies

| Header | Size | Lines | Files Including It |
|--------|------|-------|-------------------|
| `being.h` | 69KB | 2,075 | 289 files |
| `obj.h` | 24KB | 763 | ~200 files |
| `extern.h` | 21KB | ~600 | ~150 files |
| `thing.h` | 16KB | ~450 | ~300 files |
| `room.h` | 11KB | 267 | ~180 files |

### Directory Statistics

| Directory | Source Files | Header Files | Total Size |
|-----------|-------------|--------------|------------|
| misc/ | 99 .cc | many .h | 5.7MB |
| obj/ | 85 .cc | 86 .h | 1.3MB |
| spec/ | 59 .cc | few .h | 1.3MB |
| cmd/ | 56 .cc | few .h | 932KB |
| disc/ | 45 .cc | many .h | 1.5MB |
| task/ | 45 .cc | few .h | 420KB |
| sys/ | 29 .cc | many .h | 948KB |

### The being.h Problem

`being.h` directly includes 19 other headers:
- db.h, thing.h, comm.h, response.h, race.h
- immunity.h, garble.h, sound.h, wiz_powers.h
- cmd_message.h, task.h, toggle.h, tweaks.h
- spell2.h, disease.h, trap.h, stats.h
- obj.h, defs.h

This creates a massive transitive include tree. Any change to these headers triggers recompilation of 289 files.

### Circular Dependencies Identified

- `being.h` → `obj.h` → `being.h` (circular)
- `being.h` → `thing.h` → `limbs.h` → `faction.h`
- `room.h` → `obj.h` → `being.h` (transitive)

---

## Recommended Refactoring Steps

### Phase 1: Forward Declaration Audit (Low Risk)

**Goal:** Reduce includes by using forward declarations where possible.

**Steps:**
1. Run IWYU on the codebase:
   ```bash
   cmake --preset dev-gcc -DSNEEZY_USE_IWYU=ON
   cmake --build --preset dev-gcc 2>&1 | tee iwyu.log
   ```
2. Review IWYU suggestions for each file
3. Replace includes with forward declarations where safe
4. Test compilation after each batch of changes

**Expected Impact:** 10-15% improvement

### Phase 2: Create Forward Declaration Headers (Medium Risk)

**Goal:** Provide lightweight headers with just type declarations.

**Files to Create:**
- `being_fwd.h` - Forward declares TBeing and related types
- `obj_fwd.h` - Forward declares TObj and obj_index
- `room_fwd.h` - Forward declares TRoom

**Example `being_fwd.h`:**
```cpp
#pragma once

class TBeing;
class TMonster;
class TPerson;
class affectedData;
struct skillData;
// ... other forward declarations
```

**Expected Impact:** 15-20% improvement

### Phase 3: Split being.h (High Impact, Higher Risk)

**Goal:** Break the monolithic `being.h` into logical components.

**Proposed Split:**
```
being.h (69KB) →
├── being_core.h      (~15KB) - Core TBeing class definition
├── being_combat.h    (~20KB) - Combat methods and data
├── being_skills.h    (~15KB) - Skill/spell methods
├── being_movement.h  (~10KB) - Movement and position
└── being_io.h        (~10KB) - I/O and communication
```

**Approach:**
1. Identify logical groupings of methods/data
2. Create new headers with minimal includes
3. Use pimpl pattern to hide implementation details
4. Update includes incrementally (one directory at a time)

**Expected Impact:** 25-35% improvement

### Phase 4: Optimize extern.h (Medium Risk)

**Goal:** Split the kitchen-sink extern.h into focused modules.

**Proposed Split:**
```
extern.h (21KB) →
├── extern_globals.h  - Global variables
├── extern_being.h    - Being-related externs
├── extern_world.h    - World/room externs
└── extern_misc.h     - Miscellaneous
```

**Expected Impact:** 5-10% improvement

### Phase 5: Apply Pimpl Pattern (High Impact, High Risk)

**Goal:** Hide implementation details behind pimpl for major classes.

**Priority Classes:**
1. `TBeing` - Most impactful due to 289 dependents
2. `TObj` - Second most included
3. `TRoom` - Third most included

**Example Pattern:**
```cpp
// being.h (public interface)
class TBeing : public TThing {
public:
    // Public interface only
    void doSomething();
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

// being_impl.h (internal, not included by most files)
struct TBeing::Impl {
    // All the data members that were in being.h
    CSkill* skills[MAX_SKILLS];
    // ...
};
```

**Note:** `person.h` already uses this pattern with `TPersonPimpl` - use as reference.

**Expected Impact:** 20-30% improvement (but overlaps with Phase 3)

---

## Implementation Guidelines

### Testing Strategy

1. **Before starting:** Baseline build time measurement
   ```bash
   rm -rf build/dev-gcc
   time cmake --preset dev-gcc && time cmake --build --preset dev-gcc
   ```

2. **After each phase:** Measure improvement
   ```bash
   rm -rf build/dev-gcc
   time cmake --preset dev-gcc && time cmake --build --preset dev-gcc
   ```

3. **Regression testing:** Run `./scripts/verify_boot.sh` after each change

### Safety Rules

1. **Never break the build** - All changes must compile
2. **Test incrementally** - One directory at a time
3. **Preserve ABI** - Don't change class layouts without careful review
4. **Document changes** - Update comments when moving code

### Tools to Use

- **IWYU:** `cmake --preset dev-gcc -DSNEEZY_USE_IWYU=ON`
- **clang-tidy:** `cmake --build --preset dev-gcc --target clang-tidy`
- **Build profiling:** `./scripts/build-profile.sh dev-clang`

---

## Priority Order

1. **Quick wins (Phase 1-2):** Forward declarations, *_fwd.h headers
2. **High impact (Phase 3):** Split being.h
3. **Cleanup (Phase 4):** Split extern.h
4. **Advanced (Phase 5):** Pimpl pattern (optional, overlaps with Phase 3)

---

## Estimated Timeline

| Phase | Effort | Risk | Impact |
|-------|--------|------|--------|
| Phase 1: Forward declarations | 2-4 hours | Low | 10-15% |
| Phase 2: Forward declaration headers | 2-4 hours | Medium | 15-20% |
| Phase 3: Split being.h | 8-16 hours | High | 25-35% |
| Phase 4: Split extern.h | 2-4 hours | Medium | 5-10% |
| Phase 5: Pimpl pattern | 16+ hours | High | 20-30% |

**Total estimated improvement:** 40-60% faster clean builds

---

## References

- [Include What You Use](https://include-what-you-use.org/)
- [C++ Pimpl Pattern](https://en.cppreference.com/w/cpp/language/pimpl)
- [ClangBuildAnalyzer](https://github.com/aras-p/ClangBuildAnalyzer)
