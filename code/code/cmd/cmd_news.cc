//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "cmd_news.cc" - The news command and shared news/wiznews utilities
//
//    News and wiznews entries are individual files in lib/txt/news.d/ and
//    lib/txt/wiznews.d/ named with the format YYYY-MM-DD-slug (e.g.
//    2026-02-15-help-files).  Files are sorted reverse-lexicographically
//    so newest entries appear first.
//
//////////////////////////////////////////////////////////////////////////

#include "cmd_news.h"

#include "being.h"
#include "extern.h"
#include "statistics.h"

#include <algorithm>
#include <dirent.h>
#include <memory>

namespace {

bool parseFilename(
  const std::string& name, int& year, int& month, int& day) {
  if (name.size() < 10)
    return false;
  if (name[4] != '-' || name[7] != '-')
    return false;

  // Validate digit positions: YYYY-MM-DD
  for (int i : {0, 1, 2, 3, 5, 6, 8, 9}) {
    if (!isdigit(static_cast<unsigned char>(name[i])))
      return false;
  }

  year = std::stoi(name.substr(0, 4));
  month = std::stoi(name.substr(5, 2));
  day = std::stoi(name.substr(8, 2));

  return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

// Formats a news entry with the date prefix on the first line and all
// continuation lines indented to align with the content start.
//
// Example output:
//   02-15-26: Help files have been expanded and improved across the board.
//             Many skills, spells, prayers, and rituals now have detailed
//             help entries where previously there were only brief
//             placeholders.
sstring formatNewsBlock(std::string_view prefix, std::string_view content) {
  sstring result(prefix);
  std::string indent(prefix.size(), ' ');

  bool firstLine = true;
  size_t pos = 0;
  while (pos <= content.size()) {
    auto nl = content.find('\n', pos);
    auto lineEnd = (nl == std::string_view::npos) ? content.size() : nl;
    auto line = content.substr(pos, lineEnd - pos);

    if (firstLine) {
      firstLine = false;
    } else {
      result += "\n";
      if (!line.empty())
        result += indent;
    }
    result += std::string(line);

    if (nl == std::string_view::npos)
      break;
    pos = nl + 1;
  }

  return result;
}

}  // namespace

sstring formatDatePrefix(int year, int month, int day) {
  return (format("%02d-%02d-%02d: ") % month % day % (year % 100)).str();
}

std::vector<NewsEntry> scanNewsFilenames(const char* dir) {
  std::vector<NewsEntry> entries;

  auto d = std::unique_ptr<DIR, decltype(&closedir)>(opendir(dir), closedir);
  if (!d)
    return entries;

  while (auto* dp = readdir(d.get())) {
    if (dp->d_name[0] == '.')
      continue;

    NewsEntry entry;
    entry.filename = dp->d_name;

    if (!parseFilename(entry.filename, entry.year, entry.month, entry.day))
      continue;

    entries.push_back(std::move(entry));
  }

  // Sort newest first (reverse lexicographic by filename)
  std::sort(entries.begin(), entries.end(),
    [](const NewsEntry& a, const NewsEntry& b) {
      return a.filename > b.filename;
    });

  return entries;
}

void loadEntryContent(const char* dir, NewsEntry& entry) {
  sstring filepath = (format("%s/%s") % dir % entry.filename).str();
  if (!file_to_sstring(filepath.c_str(), entry.content, CONCAT_NO))
    return;

  // file_to_sstring appends \r after each \n; strip them so formatNewsBlock
  // can indent continuation lines without embedded \r clobbering the indent.
  entry.content.erase(
    std::remove(entry.content.begin(), entry.content.end(), '\r'),
    entry.content.end());
}

std::vector<NewsEntry> readNewsEntries(const char* dir) {
  auto entries = scanNewsFilenames(dir);
  for (auto& entry : entries)
    loadEntryContent(dir, entry);
  return entries;
}

sstring assembleNewsEntries(const char* dir, std::string_view searchTerm) {
  auto entries = readNewsEntries(dir);
  sstring result;

  auto searchLower = sstring(std::string(searchTerm)).lower();

  for (const auto& entry : entries) {
    sstring prefix = formatDatePrefix(entry.year, entry.month, entry.day);
    sstring block = formatNewsBlock(prefix, entry.content);

    if (!searchTerm.empty()) {
      if (block.lower().find(searchLower) == sstring::npos)
        continue;
    }

    result += block;
    if (!result.empty() && result.back() != '\n')
      result += "\n";
    result += "\n";
  }

  return result;
}

sstring latestNewsDateString(const char* dir) {
  auto entries = scanNewsFilenames(dir);
  if (entries.empty())
    return "Unknown";

  // scanNewsFilenames returns newest-first
  const auto& e = entries.front();
  return (format("%04d-%02d-%02d") % e.year % e.month % e.day).str();
}

void TBeing::doNews(const char* argument) {
  if (!desc)
    return;

  news_used_num++;

  char arg[MAX_INPUT_LENGTH];
  one_argument(argument, arg, cElements(arg));

  sstring str = assembleNewsEntries(File::NEWS_DIR, arg);

  if (str.empty())
    str = "No news today.\n\r";

  desc->page_string(str.toCRLF());
}
