#include <iostream>
#include <stdexcept>
#include <boost/regex.hpp>

#include "sstring.h"
#include "db.h"
#include "extern.h"
#include "ansi.h"
#include "parse.h"
#include "configuration.h"
#include "database.h"


const char * WHITESPACE = " \f\n\r\t\v";

bool isvowel(const char c) { return strchr("AEIOUaeiou", c); }

boost::format format(const std::string &f_string)
{
  boost::format fmter(f_string);
  if(Config::ThrowFormatExceptions()){
    fmter.exceptions(boost::io::all_error_bits);
  } else {
    fmter.exceptions(boost::io::no_error_bits);
  }
  return fmter;
}


const sstring & sstring::operator+=(const char &a){
  std::string::operator+=(a);
  return *this;
}

const sstring & sstring::operator+=(const std::string &a){
  this->append(a);
  return *this;
}

const sstring & sstring::operator+=(const char *a)
{
  this->append(a);
  return *this;
}

const sstring & sstring::operator+=(const boost::format &a)
{
  this->append(a.str());
  return *this;
}

const sstring & sstring::operator=(const boost::format &a)
{
  this->assign(a.str());
  return *this;
}

const char & sstring::operator[](unsigned int i) const
{
  return this->at(i);
}

char & sstring::operator[](unsigned int i)
{
  return this->at(i);
}


// puts commas every 3rd char, for formatting number strings
const sstring sstring::comify() const
{
  sstring tString=*this;
  unsigned int  strCount, charIndex = 0;

  tString=format("%.0f") % convertTo<float>(*this);
  strCount = tString.length();
  tString="";

  for (; charIndex < strCount; charIndex++) {
    // put commas every 3rd char EXCEPT if next char is '-'
    // that is, want "123456" to become "123,456"
    // but don't want "-123" to become "-,123"
    if (!((strCount - charIndex) % 3) && charIndex != 0 &&
        !(charIndex == 1 && (*this)[0] == '-'))
      tString += ",";

    tString += (*this)[charIndex];
  }

  for (; charIndex != this->length(); charIndex++)
    tString += (*this)[charIndex];

  return tString;
}


// converts newlines in the string to CRLF if possible
// this is for preparation for sending out to a player
// for cross platform compatibility
const sstring sstring::toCRLF() const
{
  sstring dosstr = "";
  unsigned int len;

  len = (*this).length();
  for (unsigned int loc=0; loc < len; ++loc){
    dosstr += (*this)[loc];
    if (loc>0 && (*this)[loc] == '\n' && (*this)[loc-1] != '\r' &&
      (loc+1) < len && (*this)[loc+1] != '\r') {
      dosstr += '\r';
    }
  }
  return dosstr;
}


const sstring sstring::lower() const
{
  sstring s=*this;
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  return s;
}


const sstring sstring::upper() const
{
  sstring s=*this;
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  return s;
}


// capitalizes first letter, skipping color codes
const sstring sstring::cap() const
{
  int counter = 0;
  sstring s=*this;

  if (s.length() <= 0)
    return s;

  if(s[0] != '<'){
    s[0]=toupper(s[0]);
  } else {
    // Accounting for Items with color strings and % as first character
    for(sstring::size_type i=0;i<s.length();++i){
      if (s[i] == '<')
        counter = 0;
      else
        counter++;

      if (counter == 3) {
        s[i] = toupper(s[i]);
        return s;
      }
    }
  }

  return s;
}


// uncapitalizes first letter, skipping color codes
const sstring sstring::uncap() const
{
  int counter = 0;
  sstring s=*this;

  if (s.length() <= 0)
    return s;

  if (s[0] != '<') {
    s[0] = tolower(s[0]);
  } else {
    // Accounting for Items with color sstrings and % as first character
    for(sstring::size_type i=0;i<s.length();++i){
      if (s[i] == '<')
	      counter = 0;
      else
	      counter++;

      if (counter == 3) {
	      s[i] = tolower(s[i]);
        return s;
      }
    }
  }

  return s;
}


// returns length of the string, skipping color codes
size_t sstring::lengthNoColor() const
{
  const char *sz = c_str();
  size_t len = length();
  size_t cleanLen = 0;

  for (size_t iScan = 0; iScan < len; iScan++)
    if (sz[iScan] == '<' && (iScan+2) <= len && sz[(iScan+2)] == '>')
      iScan += 2;
    else
      cleanLen++;

  return cleanLen;
}


// splits the string up by whitespace and returns the i'th "word"
const sstring sstring::word(int i) const
{
  size_t copy_begin=0, copy_end=0;

  while(1){
    // find first non-whitespace past our last working point
    copy_begin=find_first_not_of(WHITESPACE, copy_end);

    // if nothing found, no more words, return
    if(copy_begin == sstring::npos)
      return "";

    // find our first whitespace past last non-whitespace
    copy_end=find_first_of(WHITESPACE, copy_begin);

    if(!i--){
      // if nothing found, we're on the last word, no trailing whitespace
      if(copy_end == sstring::npos)
        return substr(copy_begin);
      else
        return substr(copy_begin, copy_end-copy_begin);
    }
  }

  return "";
}


// returns true if string has a digit in it
const bool sstring::hasDigit() const
{
  return find("0123456789", 0);
}


// returns true if string has only digits in it
const bool sstring::isNumber() const
{
  return find_first_not_of("0123456789") == sstring::npos;
}


const bool sstring::startsVowel() const
{
  for (unsigned int i=0;i<size();++i){
    if (isspace(at(i)))
      continue;
    return isvowel(at(i));
  }
  return false;
}


const sstring sstring::replaceString(const sstring &find, const sstring &replace) const
{
  sstring str = *this;
  str.inlineReplaceString(find, replace);
  return str;
}


// converts each beginning char of each sentence to uppercase
const sstring sstring::capitalizeSentences() const
{
  sstring str = *this;
  size_t st = str.find_first_not_of(" ");
  while(st != sstring::npos)
  {
    str[st] = toupper(str[st]);
    st = str.find_first_of(".!?", st);
    if (st != sstring::npos)
      st = str.find_first_not_of(" ", st+1);
  }
  return str;
}


std::vector<sstring> sstring::split(const sstring &separator) const
{
  std::vector<sstring> out;
  size_t previous = 0, current = 0;
  while ((current=find(static_cast<const std::string&>(separator), previous)) != sstring::npos) {
    out.push_back(substr(previous, current));
    previous = current + separator.length();
  }
  if (previous < length())
    out.push_back(substr(previous, sstring::npos));
  return out;
}


// given a sentence, try to match to the same case structure
const sstring sstring::matchCase(const sstring &match) const
{
  std::string out = *this;
  int iOut = 0, iMatch = 0;

  while(iMatch < (int)match.length() && iOut < (int)out.length())
  {
    // skip to next word to match case on
    if (match[iMatch] == ' ')
    {
      while (iMatch < (int)match.length() && match[iMatch] == ' ') iMatch++;
      if (out[iOut] != ' ')
        while (iOut < (int)out.length() && out[iOut] != ' ') iOut++;
      while (iOut < (int)out.length() && out[iOut] == ' ') iOut++;
      continue;
    }

    // we're done with our word, skip to see next match
    if (out[iOut] == ' ')
    {
      while (iOut < (int)out.length() && out[iOut] == ' ') iOut++;
      if (out[iOut] != ' ')
        while (iMatch < (int)match.length() && match[iMatch] != ' ') iMatch++;
      while (iMatch < (int)match.length() && match[iMatch] == ' ') iMatch++;
      continue;
    }

    // match yon case
    if (isupper(match[iMatch]) && !isupper(out[iOut]))
      out[iOut] = toupper(out[iOut]);
    else if (islower(match[iMatch]) && !islower(out[iOut]))
      out[iOut] = tolower(out[iOut]);
    iMatch++;
    iOut++;
  }

  return out;
}


// many of the talk features colorize the says/tells/etc for easier viewing
// If I do "say this <r>color<z> is cool", I would expect to see color in
// red, and "this ", " is cool" be the 'normal' say color.
// unfortunately, turning off red (<z>) makes everything go back to
// normal, and we lose the 'normal' color.
// To get around this, we parse the say statement, and convert any <z>, <Z>,
// or <1> to a 'replacement' color sstring and then send it out.
// unfortunately, we also need to "unbold", so we need to send both the
// normal <z> as well as the replacement
void sstring::convertStringColor(const sstring &replacement)
{
  // <tmpi> is sentinel, replaced with <z> at the end
  sstring repl = "<XXXX>" + replacement;
  size_t pos = 0;
  for (const char* s: { "<z>", "<Z>", "<1>" })
    while ((pos=find(s)) != sstring::npos)
      replace(pos, 3, repl);
  while ((pos=find("<XXXX>")) != sstring::npos)
    replace(pos, 6, "<z>");
}


const sstring sstring::escape(const sstring &chars) const
{
  std::ostringstream out;
  for (auto c: *this) {
    if (chars.find(c) != sstring::npos)
      out << '\\';
    out << c;
  }
  return out.str();
}


const sstring sstring::escapeJson() const {
  return escape("\\\"/\b\f\n\r\t");
}


const sstring sstring::escapeSql() const {
  // list taken from mysql manual, added backtick myself --rmsr
  return escape("\x00\r\n\b\t\x1a\\\"'%_`");
}


const bool sstring::startswith(const sstring &s) const
{
  return !(this->compare(0, s.length(), s));
}


const bool sstring::endswith(const sstring &s) const
{
  return !(this->compare(length() - s.length(), sstring::npos, s));
}


const bool sstring::contains(const sstring &s) const
{
  return find(static_cast<const std::string &>(s)) != sstring::npos;
}


const sstring sstring::trim(const sstring &whitespace) const
{
  size_t start = find_first_not_of(whitespace);
  start = start == sstring::npos ? 0 : start;
  return substr(start, find_last_not_of(whitespace));
}


const sstring sstring::ltrim(const sstring &whitespace) const
{
  size_t start = find_last_not_of(whitespace);
  return start == sstring::npos ? "" : substr(start, sstring::npos);
}


const sstring sstring::rtrim(const sstring &whitespace) const {
  return substr(0, find_last_not_of(whitespace));
}


void sstring::asciify()
{
  erase(std::remove_if(begin(), end(), [](char c){return (c > 31 && c < 127);}), end());
}


void sstring::inlineReplaceString(const sstring &search_ss, const sstring &replace_ss)
{
  // sstring REAAAALY doesn't like casting back to std::string, lotsa hoop jumping --rmsr
  size_t start = 0;
  const std::string &search = static_cast<const std::string&>(search_ss);
  const std::string &replace = static_cast<const std::string&>(replace_ss);
  while ((start=find(search, start)) != sstring::npos) {
    std::string::replace(start, search.length(), replace);
    start += replace.length();
  }
}
