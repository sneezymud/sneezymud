//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// random.h - Random number generation utilities
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <algorithm>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

// Returns a reference to the global Mersenne Twister RNG.
// Uses Meyer's singleton pattern (construct on first use) to avoid
// static initialization order fiasco.
std::mt19937& getRng();

// Shuffle a container using the global RNG.
// Encapsulates RNG access so callers don't need to know about getRng().
template <typename Container>
void shuffleContainer(Container& container) {
  std::shuffle(container.begin(), container.end(), getRng());
}

// Helper to retrieve by value a random entry from a vector.
// Receives a copy of a std::vector<T>, shuffles it in place using the shared
// RNG, then returns first entry by value.
// Returns std::nullopt if the vector is empty.
template <typename T>
std::optional<T> getRandomEntryByVal(std::vector<T> v) {
  if (v.empty()) {
    return std::nullopt;
  }
  shuffleContainer(v);
  return v.front();
}

// Helper to retrieve by reference a random entry from a vector.
// Takes an instance of std::vector<T>, shuffles in place using the shared RNG,
// then returns reference to first entry. Mutates the original vector.
// Should be more efficient when mutability of original isn't an issue.
// Throws std::invalid_argument if the vector is empty.
template <typename T>
T& getRandomEntryByRef(std::vector<T>& v) {
  if (v.empty()) {
    throw std::invalid_argument("getRandomEntryByRef called with empty vector");
  }
  shuffleContainer(v);
  return v.front();
}
