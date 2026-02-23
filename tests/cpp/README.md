# C++ Tests

Google Test-based tests for SneezyMUD, run via `make test`.

```
tests/cpp/
  unit/           Unit tests - no database or game server
  CMakeLists.txt  Auto-discovers *_test.cc files in unit/
  README.md
```

New `*_test.cc` files in `unit/` are discovered automatically at CMake configure time. After adding a new file, re-run cmake so it knows about it:

```bash
cmake --preset dev-clang    # or whichever preset you use (cmake --list-presets to see all)
```

## What makes a good unit test

**Tests behavior, not implementation.** Assert on observable outcomes, not internal mechanics. A test for `trim()` should verify that whitespace is removed, not that a specific index variable reaches a specific value.

**Protects against realistic regressions.** If the test breaks, it should mean something actually went wrong - not that someone renamed an internal variable or reformatted output. Ask: "what realistic code change would this catch?"

**Tests edge cases, not happy paths.** The happy path often exercises the standard library more than your code. `convertTo<int>("5")` returning 5 tells you nothing. `convertTo<int>("")` or `convertTo<int>("12abc")` tests real decisions in your code.

**Assertions must be meaningful.** "Doesn't crash" is not a test outcome in an ASan-enabled codebase. Neither is pushing X into a container and asserting you get X back - that tests the container, not your code. Every test must assert something specific about your code's behavior. If you need 100 lines of setup for 3 lines of assertions, the test belongs at a different layer or the code being tested needs refactoring.

**Tests must be independent.** Don't modify globals (`character_list`, `Races[]`) without restoring them, don't rely on execution order, and don't use `setAbortTestOnFail` or side effects from earlier tests. If a test only passes when run in a specific order, it's broken.

## Choosing the right test layer

### C++ unit tests (`tests/cpp/unit/`, `make test`)

Use when:

- The code being tested is **pure logic** - takes inputs, returns outputs, no side effects
- Setup requires **no database, no game server, no loaded characters**
- Examples: string utilities, parsers, math/formulas, data structure operations

Also use when:

- The code needs **game objects** (characters, rooms, components) but **not a database or running server**
- You need **direct access to return values** or internal state that functional tests can't observe
- Examples: `act()` message routing, `parseSpellNum()` spell ID resolution, `findComponent()` equipment/inventory search

For tests that need game objects, inherit from `GameFixture` (`unit/game_fixture.h`), which provides:

- `makeCharacter(name)` - creates TPerson with descriptor, socket, and registration in global lists. Returns `TestCharacter&` with `.ch` (the character), `.desc` (its descriptor), and `.drainOutput()` (flushes the descriptor's output queue and returns the concatenated text - this is how you observe what the character would have seen).
- `makeRoom(vnum)` - creates TRoom with ROOM_ALWAYS_LIT. Vnum must be < 50000.
- `placeInRoom(tc, room)` - places character in room via operator+=.
- `makeComponent(spell, charges)` - creates TComponent (ownership transfers to whatever it's placed into).
- `makeContainer()` - creates TChest (ownership transfers on placement).

The fixture handles a complex cleanup sequence: severing descriptor links before character deletion so `~TBeing` skips database-dependent code paths, registering characters in `character_list` for safe destructor traversal, and destroying characters before rooms so `~TPerson::dropItemsToRoom()` has a valid room target.

Do not use when:

- You need a database connection, disk files, or a running server
- You're testing how systems interact (commands, AI responses, game loops)

If you find yourself needing a running server, use the **functional suite** instead.

### Functional tests (`tests/functional/`, `make test-func`)

Use when:

- Testing requires a **running game server** and **database**
- You're testing **end-to-end behavior** through the game interface (commands, output formatting, game mechanics)
- The natural way to describe the test is "when a player does X, they should see Y"
- Examples: shop transactions, character creation, combat sequences, tell routing

## Running tests

```bash
make test                   # run all C++ unit tests
```

To run a single test or subset, use `ctest` with a name filter:

```bash
ctest --test-dir build/dev-clang -R ActTest --output-on-failure
```

Or run the test binary directly with Google Test's filter flag:

```bash
./build/dev-clang/tests/cpp/sneezy_tests --gtest_filter="ActTest.*"
```

The `--gtest_filter` flag supports wildcards: `*Convert*` matches all tests with "Convert" in the name, `SstringTest.Trim` matches a single test.

## Troubleshooting

**"No tests were found"** after adding a new file - You need to re-run cmake to pick up the new `*_test.cc` file. Run `cmake --preset dev-clang` (or your preset), then `make test`.

**Linker errors** - Unit tests link against `sneezy_lib`. If you're calling a function that isn't part of the library target, you'll get unresolved symbols. Check that the source file is included in `code/CMakeLists.txt`'s library sources.

**Sanitizer errors (ASan/UBSan)** - These appear as stack traces with `ERROR: AddressSanitizer` or `runtime error:` prefixes. They indicate real memory safety or undefined behavior bugs - don't suppress them. The stack trace points to the offending line. If the error is in test setup code rather than the code under test, the test fixture may need adjustment.

**Test passes alone but fails with other tests** - Global state pollution. Check that your test isn't modifying globals (`character_list`, `Races[]`, `room_db`) without restoring them. `GameFixture` handles this for characters and rooms, but other globals are your responsibility.
