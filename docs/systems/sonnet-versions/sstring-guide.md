---
title: sstring Class Guide
category: critical
keywords: [sstring, NULL-safe, convertTo, word parsing, boost format, string utilities, silent failure, validation]
related: [parse-system.md, command-system.md, database-system.md]
primary_symbols:
  functions: [convertTo, word, words, dropWord, upper, lower, cap, trim, comify, replaceString, isNumber, hasDigit, isWord, startsVowel, lengthNoColor, format]
  classes: [sstring]
  files: [code/code/sys/sstring.h, code/code/sys/sstring.cc, code/code/misc/parse.h]
---

## Overview

The sstring class is SneezyMUD's string wrapper that extends std::string with NULL-safety, convenience methods, and Boost.Format integration. Understanding its silent failure modes is critical for avoiding data corruption bugs.

sstring inherits from std::string and adds NULL-safe construction that converts nullptr to empty string instead of crashing, word parsing methods for command argument splitting, case manipulation with color code awareness, string utilities for formatting and validation, and direct assignment from boost::format objects.

The most important behavioral difference from std::string is the NULL-safe constructor. When constructing from a const char pointer, sstring checks for nullptr and substitutes an empty string, preventing undefined behavior. This is essential when receiving strings from database queries, C library functions, or optional parameters where NULL is a valid return value.

The second critical difference is operator[] behavior. While std::string has undefined behavior on out-of-bounds access, sstring uses std::string::at() which throws std::out_of_range exceptions. This trades silent corruption for explicit failure but requires length checks before indexing.

The third major consideration is convertTo template function behavior. This function silently returns zero on invalid input, matching atoi semantics for backward compatibility but potentially masking bugs when parsing user input or database columns.

## Patterns

### Safe Numeric Conversion

Always validate before converting strings to numbers. The convertTo template returns zero for empty strings, non-numeric input, and missing database columns without error indication.

Check that the string is non-empty and contains only expected characters before calling convertTo. Use isNumber for integer validation but beware it returns true for empty strings. Combine empty check with isNumber to ensure valid input.

When parsing command arguments, extract the word first and validate it before conversion. Never chain word extraction and convertTo without intermediate validation. Invalid word indices return empty strings which convert to zero.

For database columns, be aware that missing column names return empty sstring which converts to zero. Always verify column spelling and consider logging unexpected zero values during development.

### Command Argument Parsing

Use word method to extract positional arguments from command strings. Zero-indexed word extraction returns empty string for out-of-range indices. Check word count with words method size before extracting by index.

Use dropWord to remove the command verb and iterate through remaining arguments. Use dropWords with count to skip multiple leading words. These methods return new strings and do not modify the original.

Combine word extraction with validation. Extract to temporary sstring, validate with isNumber or other checks, then convert. This prevents silent zero conversion from masking invalid user input.

For variable argument commands, use words to split into vector and iterate. This handles arbitrary argument counts without manual index management.

### Case Manipulation With Color Codes

The cap and uncap methods skip color codes when finding the first letter to capitalize. Color codes follow the pattern of less-than, single character, greater-than. These methods correctly handle colored strings that begin with formatting directives.

Use upper and lower for full-string case conversion. These operate on all characters including those within color codes, which is appropriate since color code letters are case-insensitive in the MUD's color system.

When capitalizing sentences for display, prefer cap over manual character manipulation to preserve color formatting integrity.

### Whitespace Trimming

Use trim to remove both leading and trailing whitespace before parsing. This prevents empty word results from user input with excess spacing. Trim before word splitting to normalize input.

Use trimLeft when processing line-oriented input where trailing whitespace may be significant. Use trimRight when appending formatted strings to avoid double-spacing.

Trim does not affect internal whitespace and preserves word structure. Apply trim before isNumber validation to handle user input with surrounding spaces.

### Format String Safety

Assign boost::format results directly to sstring without calling str method. The sstring constructor accepting boost::format reference handles extraction automatically.

Use the global format function which wraps boost::format with configurable exception handling based on Config::ThrowFormatExceptions. In production mode exceptions are suppressed for format string mismatches, while development mode enables strict validation.

For C++20 code, sstring works with std::format through custom formatter specialization. This delegates to std::string formatter after casting.

### Database Result Handling

Database operator[] returns sstring for column access. Missing columns return empty strings without error. Always verify column names and consider defensive checks for expected non-zero values.

NULL database values are returned as empty strings. Use empty check to distinguish between NULL and zero-length string storage if the distinction matters for application logic.

Combine database retrieval with convertTo carefully. The pattern of fetching numeric columns as sstring then converting can silently produce zero for missing columns, NULL values, and malformed data.

### Migration From C Strings

Replace const char* parameters with std::string_view for modern code while maintaining NULL-safety at system boundaries. Use sstring for storage when interfacing with database queries and legacy functions that may return NULL.

When receiving char* from C libraries, construct sstring to safely handle NULL then convert to std::string_view for function parameters if needed. This preserves NULL-safety while enabling modern string handling patterns.

Avoid passing sstring.c_str() to std::string constructors. Use sstring directly since it inherits from std::string and implicit conversion applies.

## Reference

### Construction

sstring default constructor creates empty string. Constructor from const char* accepts nullptr and converts to empty string. Constructor from std::string copies content. Constructor from boost::format reference extracts formatted result. Constructor from std::string_view creates owned copy.

### Word Parsing

word method takes zero-based index and returns nth whitespace-delimited word or empty string if index exceeds word count. words method returns std::vector of all whitespace-delimited words. dropWord returns string with first word removed. dropWords takes count and removes that many leading words. lastWord returns final word. dropLastWord returns string with final word removed.

### Case Manipulation

upper converts all characters to uppercase. lower converts all characters to lowercase. cap capitalizes first letter after skipping color codes. uncap lowercases first letter after skipping color codes.

### String Utilities

trim removes leading and trailing whitespace. trimLeft removes leading whitespace only. trimRight removes trailing whitespace only. comify inserts commas into numeric strings for display formatting and handles negative sign. replaceString takes find and replace arguments and returns copy with all occurrences replaced. inlineReplaceString modifies string in place. lengthNoColor returns character count excluding color codes.

### Validation

isNumber returns true if string contains only digit characters but also returns true for empty string. hasDigit returns true if string contains at least one digit character. isWord returns true if string contains only alphabetic characters. startsVowel returns true if first non-whitespace character is vowel.

### Conversion

convertTo template function converts sstring to numeric type T. Specialized for int using strtol, float using strtof, double using strtof with float precision, unsigned int using strtoll. Falls back to std::istringstream for other types. Returns zero on conversion failure without error indication.

### Indexing

operator[] takes unsigned int and uses std::string::at internally. Throws std::out_of_range on bounds violation instead of undefined behavior. Const and non-const overloads available.

## Implementation

The sstring class inherits publicly from std::string and adds no data members. All added methods are convenience wrappers that operate on inherited std::string storage.

NULL-safe construction is implemented by ternary operator in constructor initializer list checking pointer for nullptr before passing to std::string base constructor. This prevents std::string from receiving NULL pointer.

Word parsing uses std::istringstream for splitting. The word method constructs stream from string content and extracts words by stream operator until reaching requested index. Returns empty string if stream exhausted before index reached.

Case manipulation methods use std::transform with ::toupper and ::tolower functions from C locale library. The cap and uncap methods scan for first non-color-code character position then modify only that character. Color code detection checks for less-than followed by any character followed by greater-than pattern.

Trimming uses std::find_if with predicate checking std::isspace. Finds first and last non-space character positions then extracts substring between them. Handles all-whitespace strings by returning empty string.

The comify method works by building result string from right to left, inserting comma every three digits. Handles negative sign by detecting leading minus and processing separately. Final reverse operation produces correct left-to-right order.

String replacement methods use std::string::find in loop to locate all occurrences. Each found position triggers std::string::replace to substitute new text. Loop continues until find returns npos.

The lengthNoColor method iterates character-by-character tracking whether inside color code based on less-than and greater-than delimiters. Increments count only for characters outside color codes.

Validation methods use std::all_of or std::any_of algorithms with character classification predicates. The isNumber check uses std::isdigit but returns true for empty string due to vacuous truth of all_of on empty range.

The boost::format constructor calls str method on format object to extract formatted result and passes to std::string base constructor. The global format function creates boost::format with exception handling policy from Config::ThrowFormatExceptions configuration value.

std::format compatibility comes from template specialization of std::formatter for sstring type. The specialization inherits from std::formatter for std::string and delegates format method after casting sstring reference to const std::string reference.

The convertTo template uses typeid to dispatch to appropriate conversion function. For int, float, double, and unsigned int specializations call C library strtol and strtof functions. The else branch uses std::istringstream with stream extraction operator and checks failbit to detect invalid input. On failure, sets result to zero before returning.

## Troubleshooting

### Silent Zero Conversion

Symptom: Numeric conversions unexpectedly produce zero values. User provides valid-looking input but system treats as zero.

Cause: convertTo returns zero for non-numeric strings, empty strings, and out-of-range word indices. No error indication distinguishes invalid input from legitimate zero.

Solution: Always validate strings before conversion. Check empty first since isNumber returns true for empty strings. Use isNumber to validate integer format. Consider logging unexpected zero values during development to catch conversion failures early.

For command parsing, verify word count matches expected argument count before extracting by index. Out-of-range word calls return empty string which converts to zero.

For database columns, verify spelling of column names. Missing columns return empty sstring. Consider defensive checks that unexpected zero values indicate data problems.

### Empty String Validation Failure

Symptom: Validation using isNumber alone accepts empty input and produces zero value.

Cause: isNumber returns true for empty string due to vacuous truth. All characters in empty string satisfy digit predicate.

Solution: Combine empty check with isNumber validation. Use pattern of checking not empty and isNumber together. Never rely on isNumber alone for user input validation.

### Out of Bounds Index Exception

Symptom: Program terminates with std::out_of_range exception when accessing string characters.

Cause: sstring operator[] uses std::string::at which throws on bounds violation. Differs from std::string operator[] undefined behavior.

Solution: Always check length or use empty test before indexing. Prefer range-based algorithms over manual indexing when possible. If indexing required, validate index less than length before access.

### Partial Numeric Parse

Symptom: Strings like "123abc" convert to 123 instead of rejecting as invalid.

Cause: strtol and strtof functions parse leading numeric portion and ignore trailing non-numeric characters. convertTo uses these functions for int and float conversions.

Solution: Use isNumber validation before convertTo to ensure entire string is numeric. isNumber checks all characters are digits and rejects partial numeric strings. Alternative is validate returned value makes sense in context.

### Word Index Off By One

Symptom: Extracting arguments by word index produces wrong argument or empty string.

Cause: word uses zero-based indexing. First word is word(0) not word(1). Off-by-one errors produce empty string for out-of-range access.

Solution: Remember zero-based indexing. word(0) is command verb, word(1) is first argument. Test boundary cases with single-word input to verify indexing.

### Database Column Typo

Symptom: Database columns read as zero when they contain non-zero values.

Cause: Missing or misspelled column name returns empty sstring from operator[]. convertTo produces zero for empty string.

Solution: Verify column names match schema exactly. Consider SELECT with explicit column list rather than SELECT star to catch missing columns at query time. Log unexpected zero values to detect column name errors.

### Boost Format Mismatch

Symptom: Format string has different placeholder count than arguments provided but no error reported.

Cause: Config::ThrowFormatExceptions controls whether boost::format throws on argument mismatch. Production mode suppresses exceptions.

Solution: Enable ThrowFormatExceptions during development to catch format string errors. Test format strings with various argument counts. Consider moving to std::format which has compile-time validation for literal format strings.

### Color Code Length Calculation

Symptom: Display formatting assumes wrong string length when string contains color codes.

Cause: length method counts color code characters while display rendering does not show them. Width calculations for padding or alignment produce wrong results.

Solution: Use lengthNoColor instead of length when calculating display width. Color codes like less-than-r-greater-than occupy three characters in storage but zero display width. Subtract lengthNoColor from length to determine color code overhead.

### Case Conversion With Color

Symptom: cap or uncap does not affect expected character position.

Cause: These methods skip color codes when finding first letter. Colored string has different first letter position than uncolored string.

Solution: Understand cap and uncap are color-code aware. First letter means first non-color-code character. Verify color codes use less-than-character-greater-than pattern recognized by parser.

### Whitespace In Validation

Symptom: isNumber rejects numeric strings that should be valid.

Cause: isNumber requires all characters to be digits. Leading or trailing whitespace fails validation.

Solution: Apply trim before isNumber check. strtol accepts leading whitespace but isNumber does not. Trim normalizes input for consistent validation.
