---
title: sstring Class Guide
description: The sstring class is SneezyMUD's string wrapper that extends std::string with NULL-safety, convenience methods, and integration with Boost.Format for safer string handling.
keywords:
  - sstring
  - string handling
  - NULL-safe construction
  - convertTo
  - word parsing
  - boost format
  - string utilities
  - isNumber
  - trim
  - upper
  - lower
  - cap
  - replaceString
category: Understanding Systems
related:
  - parse-system.md
  - command-system.md
  - database-system.md
last_updated: 2026-01-29
source_files:
  - code/code/sys/sstring.h
  - code/code/sys/sstring.cc
  - code/code/misc/parse.h
---

# sstring Class Guide

The `sstring` class is SneezyMUD's string wrapper that extends `std::string` with NULL-safety, convenience methods, and integration with Boost.Format. Understanding its behavior is critical for avoiding silent data corruption bugs.

**Files:** `code/code/sys/sstring.h`, `code/code/sys/sstring.cc`

## Overview

```cpp
class sstring : public std::string {
  public:
    sstring() : std::string() {}
    sstring(const char* str) : std::string(str ? str : "") {}
    sstring(const std::string& str) : std::string(str) {}
    sstring(boost::format& a) : std::string(a.str()) {}
    sstring(std::string_view sv) : std::string(sv) {}
    // ...
};
```

`sstring` inherits from `std::string` and adds:
- **NULL-safe construction** - passing `nullptr` creates an empty string instead of crashing
- **Word parsing** - `word()`, `words()`, `dropWord()` for command argument parsing
- **Case manipulation** - `upper()`, `lower()`, `cap()` with color code awareness
- **String utilities** - `trim()`, `comify()`, `replaceString()`, etc.
- **Boost.Format integration** - direct assignment from `boost::format` objects

## Critical: NULL-Safe Construction

The most important feature of `sstring` is its NULL-safe constructor:

```cpp
sstring(const char* str) : std::string(str ? str : "") {}
```

**This prevents crashes when constructing from potentially-NULL C strings:**

```cpp
// SAFE: sstring handles NULL gracefully
const char* maybeNull = someFunction();  // Might return NULL
sstring s = maybeNull;  // Creates empty string if NULL

// CRASH: std::string does NOT handle NULL
std::string s2 = maybeNull;  // Undefined behavior if NULL!
```

This is especially important when receiving strings from:
- Database queries (`db["column"]` returns empty string on missing column)
- C library functions
- Optional parameters

## operator[] Behavior

**WARNING:** `sstring::operator[]` uses `std::string::at()`, which throws `std::out_of_range` on bounds violation:

```cpp
char& sstring::operator[](unsigned int i) { return this->at(i); }
const char& sstring::operator[](unsigned int i) const { return this->at(i); }
```

This differs from `std::string::operator[]`, which has undefined behavior on out-of-bounds access. The `sstring` version is safer but **will throw exceptions** rather than silently returning garbage.

```cpp
sstring s = "hello";
char c = s[10];  // THROWS std::out_of_range, not silent corruption
```

**Best practice:** Always check `length()` or `empty()` before indexing.

## convertTo<T>() Template

The `convertTo<T>()` template function (defined in `parse.h`) converts strings to numeric types with **silent failure to zero**.

```cpp
template <class T>
T convertTo(const sstring& s) {
  T x;
  if (typeid(x) == typeid(int)) {
    return (T)strtol(s.c_str(), NULL, 10);
  } else if (typeid(x) == typeid(float)) {
    return (T)strtof(s.c_str(), NULL);
  } else if (typeid(x) == typeid(double)) {
    return (T)strtof(s.c_str(), NULL);
  } else if (typeid(x) == typeid(unsigned int)) {
    return (T)strtoll(s.c_str(), NULL, 10);
  } else {
    std::istringstream is(s);
    if (!(is >> x))
      x = 0;  // Silent failure!
    return x;
  }
}
```

### Supported Types

| Type | Conversion Function | Notes |
|------|---------------------|-------|
| `int` | `strtol()` | Base 10 |
| `unsigned int` | `strtoll()` | Base 10, may truncate |
| `float` | `strtof()` | |
| `double` | `strtof()` | Uses float precision! |
| Other | `std::istringstream` | Falls back to stream extraction |

### Silent Failure Modes

**CRITICAL:** `convertTo<T>()` silently returns 0 on invalid input. This is intentional for backward compatibility with `atoi()` behavior but can mask bugs.

```cpp
// All of these silently return 0:
convertTo<int>("")           // Empty string -> 0
convertTo<int>("abc")        // Non-numeric -> 0
convertTo<int>("  ")         // Whitespace -> 0
convertTo<int>("12abc")      // Partial parse -> 12 (NOT 0!)

// This is valid:
convertTo<int>("42")         // Returns 42
convertTo<int>("-5")         // Returns -5
convertTo<int>("  123  ")    // Returns 123 (strtol skips leading whitespace)
```

### Common Gotcha: word() + convertTo

A frequent pattern is parsing command arguments:

```cpp
sstring arg = "give 100 gold";
int amount = convertTo<int>(arg.word(1));  // Returns 100

// But if the user types incorrectly:
sstring arg2 = "give gold";
int amount2 = convertTo<int>(arg2.word(1));  // Returns 0 (gold -> 0)
```

**Best practice:** Validate input before conversion:

```cpp
sstring amountStr = arg.word(1);
if (!amountStr.isNumber()) {
    ch->sendTo("You must specify a number.\n\r");
    return;
}
int amount = convertTo<int>(amountStr);
```

### Database Results

Database columns return `sstring` via `operator[]`. Missing columns return empty strings, which `convertTo` converts to 0:

```cpp
TDatabase db(DB_SNEEZY);
db.query("select vnum, price from obj where vnum=%i", vnum);
if (db.fetchRow()) {
    int price = convertTo<int>(db["price"]);       // Works
    int missing = convertTo<int>(db["nonexistent"]); // Returns 0, no error!
}
```

## Word Parsing Methods

### word(int index)

Returns the nth whitespace-delimited word (0-indexed). Returns empty string if index out of range.

```cpp
sstring cmd = "give sword to guard";
cmd.word(0);  // "give"
cmd.word(1);  // "sword"
cmd.word(2);  // "to"
cmd.word(3);  // "guard"
cmd.word(4);  // "" (empty, out of range)
```

### words()

Returns a `std::vector<sstring>` of all whitespace-delimited words.

```cpp
sstring cmd = "cast fireball goblin";
std::vector<sstring> parts = cmd.words();
// parts[0] = "cast", parts[1] = "fireball", parts[2] = "goblin"
```

### dropWord() / dropWords(n)

Returns string with first word (or first n words) removed.

```cpp
sstring cmd = "give 100 gold to guard";
cmd.dropWord();      // "100 gold to guard"
cmd.dropWords(2);    // "gold to guard"
```

### lastWord() / dropLastWord()

```cpp
sstring cmd = "give sword to guard";
cmd.lastWord();      // "guard"
cmd.dropLastWord();  // "give sword to"
```

## Case Manipulation

### upper() / lower()

Convert entire string to uppercase/lowercase.

```cpp
sstring s = "Hello World";
s.upper();  // "HELLO WORLD"
s.lower();  // "hello world"
```

### cap() / uncap()

Capitalize/uncapitalize the first letter, **skipping color codes**.

```cpp
sstring s = "hello";
s.cap();    // "Hello"

sstring colored = "<r>hello";
colored.cap();  // "<r>Hello" (skips the color code!)
```

Color codes follow the pattern `<X>` where X is a single character. The methods skip over these when finding the first letter.

## String Utilities

### trim() / trimLeft() / trimRight()

Remove leading/trailing whitespace.

```cpp
sstring s = "  hello world  ";
s.trim();       // "hello world"
s.trimLeft();   // "hello world  "
s.trimRight();  // "  hello world"
```

### comify()

Add commas to number strings for display.

```cpp
sstring s = "1234567";
s.comify();  // "1,234,567"

sstring neg = "-1234567";
neg.comify();  // "-1,234,567" (handles negative sign)
```

### replaceString(find, replace)

Returns a copy with all occurrences replaced.

```cpp
sstring s = "hello world world";
s.replaceString("world", "there");  // "hello there there"
```

For in-place replacement, use `inlineReplaceString()`.

### lengthNoColor()

Returns string length excluding color codes.

```cpp
sstring s = "<r>Hello<z> World";
s.length();         // 17 (includes color codes)
s.lengthNoColor();  // 11 (just "Hello World")
```

## Validation Methods

### isNumber()

Returns true if string contains only digits.

```cpp
sstring("123").isNumber();   // true
sstring("12.3").isNumber();  // false (has decimal)
sstring("-5").isNumber();    // false (has minus)
sstring("").isNumber();      // true (vacuously true!)
```

**Gotcha:** Empty string returns true! Always check `!s.empty() && s.isNumber()`.

### hasDigit()

Returns true if string contains at least one digit.

```cpp
sstring("abc123").hasDigit();  // true
sstring("abc").hasDigit();     // false
```

### isWord()

Returns true if string contains only alphabetic characters.

```cpp
sstring("hello").isWord();  // true
sstring("hello1").isWord(); // false
```

### startsVowel()

Returns true if string starts with a vowel (skipping whitespace).

```cpp
sstring("apple").startsVowel();   // true
sstring("  apple").startsVowel(); // true (skips space)
sstring("banana").startsVowel();  // false
```

## Boost.Format Integration

`sstring` can be assigned directly from `boost::format`:

```cpp
sstring msg = format("Player %s has %i gold") % name % gold;
```

The global `format()` function (defined in `sstring.cc`) wraps `boost::format` with configurable exception handling:

```cpp
boost::format format(const std::string& f_string) {
  boost::format fmter(f_string);
  if (Config::ThrowFormatExceptions()) {
    fmter.exceptions(boost::io::all_error_bits);
  } else {
    fmter.exceptions(boost::io::no_error_bits);  // Silent failures
  }
  return fmter;
}
```

## std::format Compatibility

A custom formatter enables `sstring` with C++20 `std::format`:

```cpp
template <>
struct std::formatter<sstring> : std::formatter<std::string> {
    auto format(const sstring& s, std::format_context& ctx) const {
      return std::formatter<std::string>::format(
        static_cast<const std::string&>(s), ctx);
    }
};
```

This allows:

```cpp
sstring name = "Gandalf";
std::string msg = std::format("Hello, {}!", name);
```

## Common Gotchas and Silent Failures

### 1. Empty String Validation

```cpp
// BAD: isNumber() returns true for empty string
if (arg.isNumber()) {
    int val = convertTo<int>(arg);  // val = 0, but was input valid?
}

// GOOD: Check for empty first
if (!arg.empty() && arg.isNumber()) {
    int val = convertTo<int>(arg);
}
```

### 2. convertTo on Non-Numeric Strings

```cpp
// BAD: Silent failure to 0
sstring arg = "sword";
int amount = convertTo<int>(arg);  // amount = 0, no warning

// GOOD: Validate first
if (!arg.isNumber()) {
    ch->sendTo("Invalid number.\n\r");
    return;
}
```

### 3. word() Index Out of Range

```cpp
// BAD: word() returns empty string, convertTo returns 0
int target = convertTo<int>(arg.word(5));  // Silently 0 if < 6 words

// GOOD: Check word count
if (arg.words().size() < 6) {
    ch->sendTo("Not enough arguments.\n\r");
    return;
}
```

### 4. Database Missing Columns

```cpp
// BAD: Missing column returns "", convertTo returns 0
int price = convertTo<int>(db["pricee"]);  // Typo! Silent 0

// GOOD: Log unexpected zeros or validate column names
```

### 5. Partial Numeric Parsing

```cpp
// SURPRISING: strtol parses leading digits
convertTo<int>("123abc");  // Returns 123, not 0!
convertTo<int>("abc123");  // Returns 0 (no leading digits)
```

## Migration to std::string

When modernizing code, consider these sstring-specific behaviors:

| sstring Feature | std::string Equivalent | Migration Notes |
|-----------------|------------------------|-----------------|
| NULL-safe ctor | Manual check needed | `str ? str : ""` |
| `word(n)` | Manual parsing | Use `std::istringstream` or ranges |
| `cap()` | Manual | Color-code aware; consider `toupper(s[0])` |
| `upper()`/`lower()` | `std::transform` + `::toupper`/`::tolower` | |
| `trim()` | `std::erase` or ranges | |
| `comify()` | `std::format` with locale | |
| `convertTo<T>()` | `std::stoi`, `std::stof`, etc. | These throw on error! |

**Key difference:** `std::stoi()` and friends **throw exceptions** on parse failure, while `convertTo<T>()` silently returns 0. When migrating, add try/catch or pre-validation.

## Source Files Reference

| File | Contents |
|------|----------|
| `code/code/sys/sstring.h` | Class declaration, inline methods |
| `code/code/sys/sstring.cc` | Method implementations |
| `code/code/misc/parse.h` | `convertTo<T>()` template |
