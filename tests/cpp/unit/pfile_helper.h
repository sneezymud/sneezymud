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
#include <string>

#include "charfile.h"
#include "db.h"
#include "low.h"
#include "race.h"

class PfileHelper {
  public:
    PfileHelper(const std::string& name, int level, race_t race) : name_(name) {
      namespace fs = std::filesystem;

      charFile cf{};
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
      out.write(reinterpret_cast<const char*>(&cf), sizeof(cf));
    }

    ~PfileHelper() { cleanup(); }

    PfileHelper(const PfileHelper&) = delete;
    PfileHelper& operator=(const PfileHelper&) = delete;

    PfileHelper(PfileHelper&& other) noexcept :
      name_(std::move(other.name_)),
      path_(std::move(other.path_)) {
      other.path_.clear();
    }

    PfileHelper& operator=(PfileHelper&& other) noexcept {
      if (this != &other) {
        cleanup();
        name_ = std::move(other.name_);
        path_ = std::move(other.path_);
        other.path_.clear();
      }
      return *this;
    }

    void cleanup() {
      if (!path_.empty()) {
        std::filesystem::remove(path_);
        path_.clear();
      }
    }

    [[nodiscard]] const std::string& name() const { return name_; }
    [[nodiscard]] const std::filesystem::path& path() const { return path_; }

  private:
    std::string name_;
    std::filesystem::path path_;
};
