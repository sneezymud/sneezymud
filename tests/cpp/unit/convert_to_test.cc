// Edge-case tests for convertTo<T> (code/code/misc/parse.h).
//
// The legacy ConvertTo.h only tested happy paths. These tests exercise the
// real decision points: how the function handles bad input, partial parses,
// and boundary values.

#include <gtest/gtest.h>

#include <climits>
#include <cmath>

#include "sstring.h"
#include "parse.h"

// --- convertTo<int> ---
// Uses strtol(s.c_str(), NULL, 10) internally.

TEST(ConvertTo, IntEmptyString) { EXPECT_EQ(convertTo<int>(""), 0); }

TEST(ConvertTo, IntWhitespaceOnly) {
  // strtol skips leading whitespace, then finds no digits → returns 0
  EXPECT_EQ(convertTo<int>("   "), 0);
  EXPECT_EQ(convertTo<int>("\t\n"), 0);
}

TEST(ConvertTo, IntNonNumeric) {
  EXPECT_EQ(convertTo<int>("abc"), 0);
  EXPECT_EQ(convertTo<int>("hello world"), 0);
}

TEST(ConvertTo, IntMixedInput) {
  // strtol parses leading digits and stops at first non-digit
  EXPECT_EQ(convertTo<int>("12abc"), 12);
  EXPECT_EQ(convertTo<int>("100gold"), 100);
  EXPECT_EQ(convertTo<int>("-5xyz"), -5);
}

TEST(ConvertTo, IntLeadingWhitespace) {
  // strtol accepts leading whitespace
  EXPECT_EQ(convertTo<int>("  42"), 42);
  EXPECT_EQ(convertTo<int>("\t-7"), -7);
}

TEST(ConvertTo, IntNegative) {
  EXPECT_EQ(convertTo<int>("-1"), -1);
  EXPECT_EQ(convertTo<int>("-999"), -999);
}

TEST(ConvertTo, IntOverflow) {
  // strtol clamps to LONG_MAX on overflow. On 64-bit (where long > int),
  // LONG_MAX = 0x7FFFFFFFFFFFFFFF truncates to int -1. The exact value
  // is platform-dependent but should not be the "parse failed" value of 0.
  auto result = convertTo<int>("99999999999999999999");
  EXPECT_NE(result, 0);
}

TEST(ConvertTo, IntNegativeOverflow) {
  // strtol clamps to LONG_MIN on underflow. On 64-bit systems,
  // LONG_MIN = 0x8000000000000000 — the lower 32 bits are all zeros,
  // so casting to int produces 0, indistinguishable from a parse failure.
  auto result = convertTo<int>("-99999999999999999999");
  EXPECT_EQ(result, 0);
}

// --- convertTo<unsigned int> ---
// Uses strtoll(s.c_str(), NULL, 10) internally, cast to unsigned int.

TEST(ConvertTo, UnsignedIntBasic) {
  EXPECT_EQ(convertTo<unsigned int>("42"), 42u);
  EXPECT_EQ(convertTo<unsigned int>("0"), 0u);
}

TEST(ConvertTo, UnsignedIntEmptyString) {
  EXPECT_EQ(convertTo<unsigned int>(""), 0u);
}

TEST(ConvertTo, UnsignedIntNegativeInput) {
  // strtoll parses negative values; cast to unsigned wraps
  auto result = convertTo<unsigned int>("-1");
  EXPECT_EQ(result, static_cast<unsigned int>(-1));
}

// --- convertTo<float> ---
// Uses strtof(s.c_str(), NULL) internally.

TEST(ConvertTo, FloatEmptyString) {
  EXPECT_FLOAT_EQ(convertTo<float>(""), 0.0f);
}

TEST(ConvertTo, FloatNonNumeric) {
  EXPECT_FLOAT_EQ(convertTo<float>("abc"), 0.0f);
}

TEST(ConvertTo, FloatMixedInput) {
  EXPECT_FLOAT_EQ(convertTo<float>("3.14abc"), 3.14f);
}

TEST(ConvertTo, FloatNegative) {
  EXPECT_FLOAT_EQ(convertTo<float>("-2.5"), -2.5f);
}

TEST(ConvertTo, FloatScientificNotation) {
  // strtof handles scientific notation
  EXPECT_FLOAT_EQ(convertTo<float>("1.5e2"), 150.0f);
}

// --- convertTo<double> ---
// Implementation note: uses strtof (not strtod), so precision is limited
// to float range. This is a known quirk of the implementation.

TEST(ConvertTo, DoubleEmptyString) {
  EXPECT_DOUBLE_EQ(convertTo<double>(""), 0.0);
}

TEST(ConvertTo, DoubleBasic) {
  // Due to strtof usage, only float-precision accuracy is guaranteed
  EXPECT_NEAR(convertTo<double>("3.14"), 3.14, 0.001);
}

TEST(ConvertTo, DoubleLosesPrecision) {
  // This documents the strtof-instead-of-strtod bug: values that need
  // double precision get truncated to float precision.
  double result = convertTo<double>("1.23456789012345");
  float as_float = std::strtof("1.23456789012345", nullptr);
  EXPECT_DOUBLE_EQ(result, static_cast<double>(as_float));
}
