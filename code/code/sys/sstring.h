#pragma once

#include <string.h>
#include <boost/format.hpp>


extern const char * WHITESPACE;
extern bool isvowel(const char c);
extern boost::format format(const std::string &);

class sstring : public std::string {
public:
  // constructors
  sstring() : std::string(){}
  sstring(const char *str) : std::string(str?str:"") {}
  sstring(const std::string &str) : std::string(str) {}
  sstring(boost::format &a) : std::string(a.str()) {}

  const sstring & operator=(const boost::format &a);
  const sstring & operator+=(const boost::format &a);
  const sstring & operator+=(const char &a);
  const sstring & operator+=(const char *a);
  const sstring & operator+=(const std::string &a);

  char &operator[](unsigned int i);
  const char &operator[](unsigned int i) const;

  // these functions return modified copies of the string
  const sstring toCRLF() const;
  const sstring lower() const;
  const sstring upper() const;
  const sstring cap() const;
  const sstring uncap() const;
  const sstring word(int) const;
  const sstring comify() const;
  const sstring replaceString(const sstring &, const sstring &) const;
  const sstring capitalizeSentences() const;
  const sstring matchCase(const sstring &) const;
  const sstring escape(const sstring &) const;
  const sstring escapeSql() const;
  const sstring escapeJson() const;
  size_t lengthNoColor() const;

  const sstring trim(const sstring &t = WHITESPACE) const;
  const sstring ltrim(const sstring &t = WHITESPACE) const;
  const sstring rtrim(const sstring &t = WHITESPACE) const;

  const bool startswith(const sstring &) const;
  const bool endswith(const sstring &) const;
  const bool contains(const sstring &) const;

  // call this function with NULL data to get alloc size, then again with alloc'd array
  std::vector<sstring> split(const sstring &) const;

  // other functions
  const bool hasDigit() const;
  const bool isNumber() const;
  const bool isWord() const;
  const bool startsVowel() const;

  // mungers
  void asciify();
  void convertStringColor(const sstring &replacement);

  // in-place mungers
  void inlineReplaceString(const sstring &, const sstring &);
};


// used for easily defining buffer sizes
#ifndef cElements
#define cElements(x) (sizeof(x)/sizeof(x[0]))
#endif
