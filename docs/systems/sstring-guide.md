---
title: sstring Class Guide
description: SneezyMUD's string wrapper extending std::string with NULL-safety, word parsing, case manipulation, and Boost.Format integration
keywords: [sstring, string handling, NULL-safe, convertTo, word parsing, boost format]
category: Understanding Systems
related: [parse-system.md, command-system.md, database-system.md]
last_updated: 2026-02-01
created_by_model: opus
source_files: [code/code/sys/sstring.h, code/code/sys/sstring.cc, code/code/misc/parse.h]
---

# sstring Class Guide

## Overview

The sstring class wraps std::string to address a fundamental problem in SneezyMUD's codebase: the pervasive use of C-style strings that may be NULL. Where std::string would crash on NULL input, sstring creates an empty string, allowing legacy code to work safely without exhaustive NULL checks at every call site.

Beyond NULL safety, sstring provides domain-specific operations that appear throughout MUD command parsing. Players type commands like "give 100 gold to guard", and sstring offers word extraction, numeric conversion, and case manipulation methods that make parsing these inputs straightforward. The class integrates with both Boost.Format and C++20's std::format for message generation.

Understanding sstring's silent failure modes is critical. The convertTo template and several validation methods fail silently in ways that can mask bugs. Empty strings pass isNumber() validation. Missing database columns become zero when converted. These behaviors are intentional compatibility choices, but they require explicit validation to avoid subtle corruption.

## Patterns

### Validating Before Conversion

Always validate strings before numeric conversion. The convertTo template returns zero for empty strings, non-numeric input, and whitespace-only strings without any error indication.

Check both emptiness and numeric format before converting. Use the pattern `!arg.empty() && arg.isNumber()` before calling convertTo. Never assume that a zero result means the user typed zero.

When parsing command arguments, validate word count before extracting specific indices. The word() method returns an empty string for out-of-range indices, which convertTo silently converts to zero.

### Handling Database Results

Treat database columns defensively. Missing or misspelled column names return empty strings that convertTo silently converts to zero. Log unexpected zeros during development, or validate that required columns exist.

For critical database values, consider checking if the column actually exists before conversion rather than trusting that zero is a valid result.

### NULL Safety Boundaries

Use sstring at NULL boundaries where C strings enter the system. Database results, C library functions, and optional parameters may return NULL. Let sstring's constructor handle the conversion rather than scattering NULL checks throughout the code.

When passing strings to code that requires non-NULL pointers, use c_str() only after confirming the string is non-empty if the receiving code cannot handle empty strings.

### Word Parsing for Commands

Extract command arguments using word() with zero-based indices. The first word at index 0 is typically the command name, with arguments following at indices 1, 2, and so on.

Use dropWord() or dropWords(n) when passing remaining arguments to subcommand handlers. This preserves the original string structure while removing already-processed portions.

Use words() when you need to iterate over all arguments or check the total argument count before processing.

### Case-Aware Capitalization

Use cap() rather than manual capitalization when the string may contain color codes. The method skips over color code sequences of the form `<X>` before finding and capitalizing the first actual letter.

Use upper() and lower() for full case conversion. These methods transform the entire string regardless of color codes.

## Reference

### Construction and Assignment

| Source | Behavior |
|--------|----------|
| `nullptr` / NULL | Empty string |
| `const char*` | Contents copied |
| `std::string` | Contents copied |
| `std::string_view` | Contents copied |
| `boost::format` | Formatted result |

### Word Parsing Methods

| Method | Returns | Notes |
|--------|---------|-------|
| `word(n)` | nth word (0-indexed) | Empty string if out of range |
| `words()` | `std::vector<sstring>` | All whitespace-delimited words |
| `dropWord()` | String minus first word | Preserves remaining spacing |
| `dropWords(n)` | String minus first n words | Preserves remaining spacing |
| `lastWord()` | Final word | Empty string if empty |
| `dropLastWord()` | String minus final word | Preserves leading spacing |

### Case Manipulation

| Method | Behavior |
|--------|----------|
| `upper()` | Entire string uppercase |
| `lower()` | Entire string lowercase |
| `cap()` | First letter uppercase, skips color codes |
| `uncap()` | First letter lowercase, skips color codes |

### String Utilities

| Method | Behavior |
|--------|----------|
| `trim()` | Remove leading and trailing whitespace |
| `trimLeft()` | Remove leading whitespace |
| `trimRight()` | Remove trailing whitespace |
| `comify()` | Insert commas in number strings |
| `replaceString(find, replace)` | Return copy with all occurrences replaced |
| `inlineReplaceString(find, replace)` | In-place replacement |
| `lengthNoColor()` | Length excluding color codes |

### Validation Methods

| Method | Returns true when |
|--------|-------------------|
| `isNumber()` | All digits only (caveat: empty returns true) |
| `hasDigit()` | Contains at least one digit |
| `isWord()` | All alphabetic characters |
| `startsVowel()` | Starts with vowel (skips whitespace) |

### convertTo Template Support

| Type | Conversion | Behavior |
|------|------------|----------|
| `int` | `strtol()` base 10 | Leading digits parsed, returns 0 on failure |
| `unsigned int` | `strtoll()` base 10 | May truncate, returns 0 on failure |
| `float` | `strtof()` | Returns 0 on failure |
| `double` | `strtof()` | Uses float precision |
| Other | `std::istringstream` | Returns 0 on extraction failure |

### Operator Differences from std::string

| Operation | sstring behavior | std::string behavior |
|-----------|------------------|----------------------|
| `[index]` | Throws `std::out_of_range` | Undefined behavior |
| Construct from NULL | Empty string | Undefined behavior |

## Implementation

### NULL-Safe Constructor

The sstring class inherits from std::string and overrides the const char* constructor to check for NULL before passing to the parent. When the input pointer is NULL, it passes an empty string literal instead. This single check prevents crashes throughout the codebase where legacy C functions may return NULL.

The class declaration lives in sstring.h with method implementations in sstring.cc. The inheritance relationship means sstring is implicitly convertible to std::string and can be passed anywhere a std::string is expected.

### Bounds-Checked Indexing

The operator[] overloads call std::string::at() rather than std::string::operator[]. This provides bounds checking at the cost of throwing std::out_of_range on invalid access. The design choice prevents silent memory corruption but requires callers to check length before indexing or catch exceptions.

### Word Parsing

Word extraction splits on whitespace to support command argument parsing. The word() method performs a linear scan, counting whitespace-delimited segments until reaching the requested index. It returns an empty sstring if the index exceeds the word count.

The words() method splits the entire string into a vector, enabling iteration and count checks. The dropWord() family reconstructs the string without leading words, preserving the original spacing between remaining words.

### Case Manipulation and Color Codes

The cap() and uncap() methods implement color code awareness by scanning past sequences matching the pattern `<X>` where X is any single character. This pattern represents SneezyMUD's color escape sequences. The methods find the first non-color-code character and apply the case transformation there.

The upper() and lower() methods transform the entire string without special handling, meaning color codes become uppercase or lowercase along with the text.

### convertTo Template

The convertTo template function lives in parse.h, not in the sstring class itself. It accepts an sstring by const reference and returns the requested numeric type.

For int, unsigned int, float, and double, the function uses the C library strtol/strtof functions. These functions skip leading whitespace and parse as many digits as possible, stopping at the first non-numeric character. This means "123abc" parses as 123, while "abc123" parses as 0.

For other types, the function uses std::istringstream and stream extraction. If extraction fails, it assigns zero to the result variable.

The silent-failure-to-zero behavior maintains compatibility with legacy code that used atoi(), which also returns zero on failure. Changing this behavior would require auditing thousands of call sites.

### Boost.Format Integration

The global format() function wraps boost::format with configurable exception handling. The Config::ThrowFormatExceptions() setting determines whether format errors throw or fail silently. The sstring constructor accepting boost::format extracts the formatted string and stores it.

### std::format Compatibility

A std::formatter specialization for sstring delegates to the std::string formatter by casting the sstring to its base class. This enables sstring values in std::format calls without explicit conversion.

## Troubleshooting

### Zero Where Non-Zero Expected

**Symptom:** Numeric value is zero when the user provided input.

**Cause:** convertTo returns zero for empty strings, non-numeric input, or when word() returns an empty string for an out-of-range index.

**Diagnostic:** Log the raw string value before conversion. Check if the string is empty or contains unexpected content.

**Fix:** Add validation before conversion: check emptiness, verify word count matches expected arguments, and use isNumber() for integer inputs. Note that isNumber() returns true for empty strings.

### Index Out of Bounds Exception

**Symptom:** std::out_of_range exception during string indexing.

**Cause:** sstring operator[] uses at() which throws on bounds violation, unlike std::string operator[] which has undefined behavior.

**Diagnostic:** Check the stack trace for the indexing operation and verify the index value against string length.

**Fix:** Check length() or empty() before indexing. Consider using word() or front()/back() methods that handle empty strings gracefully.

### Capitalization Incorrect with Colors

**Symptom:** Color code character gets capitalized instead of first visible letter.

**Cause:** Manual capitalization using operator[] or toupper() without accounting for color code sequences.

**Diagnostic:** Check if the string starts with `<` indicating a color code.

**Fix:** Use cap() which skips color code sequences before finding the first letter.

### Partial Number Parsing

**Symptom:** Input like "100gold" parses as 100 instead of being rejected.

**Cause:** strtol and strtof parse leading digits and stop at non-numeric characters without error.

**Diagnostic:** Log both the input string and converted result.

**Fix:** Use isNumber() to verify the entire string is numeric before conversion. Note this also requires an empty() check since isNumber() returns true for empty strings.

### Empty String Passes isNumber

**Symptom:** Empty input treated as valid number with value zero.

**Cause:** isNumber() returns true for empty strings (vacuous truth - all zero characters are digits).

**Diagnostic:** Check if validation includes an empty() check.

**Fix:** Always combine checks: `!arg.empty() && arg.isNumber()`.

### Database Column Typo Silent Zero

**Symptom:** Database value is always zero despite valid data in the table.

**Cause:** Column name misspelled in code. Missing columns return empty string, which convertTo converts to zero.

**Diagnostic:** Print all column names from the query result. Check spelling of column access.

**Fix:** Verify column names match the database schema. Consider logging when convertTo produces zero from a database value during development.
