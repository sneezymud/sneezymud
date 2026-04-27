#pragma once

// Composable utility for writing minimal binary pfiles to disk.
// Use as a fallback when testing pfile-gated functions that can't
// be decomposed into testable sub-functions.
//
// RAII: constructor writes the file, destructor removes it.
// Requires a matching `player` row in the database (load_char
// also queries for talens).

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>

#include "charfile.h"
#include "db.h"
#include "low.h"
#include "race.h"

class PfileHelper {
  public:
    PfileHelper(const std::string& name, int level, race_t race) : name_(name) {
      namespace fs = std::filesystem;

      if (name.empty()) {
        throw std::invalid_argument("PfileHelper: name must not be empty");
      }

      charFile cf{};
      if (name.size() >= sizeof(cf.name)) {
        throw std::invalid_argument(
          "PfileHelper: name is too long for charFile");
      }
      std::strncpy(cf.name, name.c_str(), sizeof(cf.name) - 1);
      cf.level[0] = static_cast<byte>(level);
      cf.race = static_cast<ubyte>(race);
      cf.sex = SEX_MALE;
      cf.Class = CLASS_WARRIOR;
      cf.hit = 100;
      cf.maxHit = 100;
      cf.move = 100;
      cf.maxMove = 100;
      cf.mana = 100;
      cf.maxMana = 100;
      cf.hometown = Room::CS;

      // load_char uses LOWER(name[0]) for the directory and name.lower()
      // for the filename. Match that convention so pfiles are findable.
      auto lowerName = name;
      std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
        [](unsigned char c) { return std::tolower(c); });
      auto dir = fs::path(Path::MUTABLE_PLAYER) / std::string(1, lowerName[0]);
      fs::create_directories(dir);

      path_ = dir / lowerName;
      std::ofstream out(path_, std::ios::binary);
      if (!out) {
        throw std::runtime_error(
          "PfileHelper: failed to create " + path_.string());
      }
      out.write(reinterpret_cast<const char*>(&cf), sizeof(cf));
      if (!out) {
        throw std::runtime_error(
          "PfileHelper: failed to write " + path_.string());
      }
    }

    ~PfileHelper() noexcept { cleanupNoThrow(); }

    PfileHelper(const PfileHelper&) = delete;
    PfileHelper& operator=(const PfileHelper&) = delete;

    PfileHelper(PfileHelper&& other) noexcept :
      name_(std::move(other.name_)),
      path_(std::move(other.path_)) {
      other.path_.clear();
    }

    PfileHelper& operator=(PfileHelper&& other) noexcept {
      if (this != &other) {
        cleanupNoThrow();
        name_ = std::move(other.name_);
        path_ = std::move(other.path_);
        other.path_.clear();
      }
      return *this;
    }

    void cleanup() noexcept { cleanupNoThrow(); }

    [[nodiscard]] const std::string& name() const { return name_; }
    [[nodiscard]] const std::filesystem::path& path() const { return path_; }

  private:
    void cleanupNoThrow() noexcept {
      if (!path_.empty()) {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
        path_.clear();
      }
    }

    std::string name_;
    std::filesystem::path path_;
};
