#pragma once

#include "sstring.h"

#include <string>
#include <vector>

struct NewsEntry {
  std::string filename;  // e.g. "2026-02-15-help-files"
  int year = 0;
  int month = 0;
  int day = 0;
  sstring content;  // file body text
};

// Scans the directory for entry filenames, parses dates, and returns them
// sorted newest-first by filename.  Does NOT read file contents.
std::vector<NewsEntry> scanNewsFilenames(const char* dir);

// Loads file content into an entry whose filename and date are already set.
void loadEntryContent(const char* dir, NewsEntry& entry);

// Reads all entry files (filenames + content), sorted newest-first.
// Equivalent to scanNewsFilenames() followed by loadEntryContent() on each.
std::vector<NewsEntry> readNewsEntries(const char* dir);

// Assembles all entries from the given directory into a single display string.
// If searchTerm is non-empty, only entries whose content contains the term
// (case-insensitive) are included.
sstring assembleNewsEntries(const char* dir, std::string_view searchTerm = {});

// Returns the most recent entry's date as a displayable string (YYYY-MM-DD),
// or "Unknown" if no entries exist.  Used by assembleMotd().
sstring latestNewsDateString(const char* dir);

// Format date from filename components into "MM-DD-YY: " display format.
sstring formatDatePrefix(int year, int month, int day);
