#include <stdexcept>
#include "stdsneezy.h"
#include "sstring.h"
#include "format.h"


// puts commas every 3rd char, for formatting number strings
const sstring sstring::comify() const
{
  sstring tString=*this;
  unsigned int  strCount, charIndex = 0;

  tString=fmt("%.0f") % convertTo<float>(*this);
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

// converts A-Z to lower case a-z
const sstring sstring::lower() const
{
  sstring::size_type iter;
  sstring s=*this;

  do {
    iter = s.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if (iter != sstring::npos)
      s[iter]=tolower(s[iter]);
  } while (iter != sstring::npos);

  return s;
}

// converts a-z to upper case A-Z
const sstring sstring::upper() const
{
  sstring::size_type iter;
  sstring s=*this;

  do {
    iter = s.find_first_of("abcdefghijklmnopqrstuvwxyz");
    if (iter != sstring::npos)
      s[iter]=toupper(s[iter]);
  } while (iter != sstring::npos);

  return s;
}

// capitalizes first letter, skipping color codes
const sstring sstring::cap() const
{
  int counter = 0;
  sstring s=*this;

  if(s.length()>0 && s[0] != '<'){
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

// splits the string up by whitespace and returns the i'th "word"
const sstring sstring::word(int i) const
{
  unsigned int copy_begin=0, copy_end=0;
  sstring whitespace=" \f\n\r\t\v"; // taken from isspace() man page
  
  while(1){
    // find first non-whitespace past our last working point
    copy_begin=find_first_not_of(whitespace, copy_end);
    
    // if nothing found, no more words, return
    if(copy_begin == sstring::npos)
      return "";
    
    // find our first whitespace past last non-whitespace
    copy_end=find_first_of(whitespace, copy_begin);
    
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
  for(unsigned int i=0;i<size();++i){
    if (isdigit((*this)[i]))
      return true;
  }

  return false;
}


// returns true if string has only digits in it
const bool sstring::isNumber() const
{
  for(unsigned int i=0;i<size();++i){
    if (!isdigit((*this)[i]))
      return false;
  }

  return true;
}

const bool sstring::isWord() const
{
  for(unsigned int i=0;i<size();++i){
    if (!isalpha((*this)[i]))
      return false;
  }
  return true;
}

const bool sstring::startsVowel() const
{
  for(unsigned int i=0;i<size();++i){
    if(isspace((*this)[i]))
      continue;
    return isvowel((*this)[i]);
  }
  return false;
} 

const sstring sstring::replaceString(sstring find, sstring replace) const
{
  sstring str = *this;
  str.inlineReplaceString(find, replace);
  return str;
}


const char *sstring::c_str() const
{
  // we say greater than here, because a string might have nulls in it, which
  // cause strlen to come up short. we're only interested if std::string::c_str
  // gives us a too-long string.
  if(strlen(string::c_str()) > length())
    throw std::runtime_error(fmt("corruption in sstring::c_str"));

  return string::c_str();
}

const sstring & sstring::operator=(fmt &a)
{
  this->assign(a);
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

// removes leading and trailing whitespace
const sstring sstring::trim() const
{
  unsigned int iStart, iEnd;
  sstring whitespace = " \f\n\r\t\v"; // same as word whitespace

  iStart = find_first_not_of(whitespace);
  iEnd = find_last_not_of(whitespace);

  if (iStart == sstring::npos && iEnd == sstring::npos)
    return *this;
  if (iStart == sstring::npos)
    iStart = 0;
  if (iEnd == sstring::npos)
    iEnd = length();
  else
    iEnd++;

  return substr(iStart, iEnd-iStart);
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

// splits a string into an array of strings, given a delimiter
// pass NULL for data to get the amount of split strings
// like this:
//   int c = s.split(';', NULL);
//   sstring *commands = new sstring[c];
//   s.split(';', commands);
//   delete[] commands;
int sstring::split(const char delimit, sstring *data) const
{
  int iFound = 0;
  size_t iPos = 0, iPosLast = 0;

  while(sstring::npos != (iPos = find(delimit, iPos)))
  {
    size_t len = iPos-iPosLast;
    if (len)
    {
      if (data)
        data[iFound] = substr(iPosLast, len);
      iFound++;
    }
    iPosLast = ++iPos;
  }
  if (iPosLast < length())
  {
    if (data)
      data[iFound] = substr(iPosLast, length());
    iFound++;
  }

  return iFound;
}

// given a sentence, try to match to the same case structure
const sstring sstring::matchCase(const sstring match) const
{
  string out = *this;
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

// finds toFind and returns its offset as long as its inbetween start and end
size_t sstring::findBetween(const sstring start, const sstring toFind, const sstring end) const
{
  sstring data = upper();
  size_t iStart = 0;
  while(1)
  {
    iStart = data.find(start.upper().c_str(), iStart);
    if (iStart == sstring::npos)
      return sstring::npos;
    size_t iEnd = data.find(end.upper().c_str(), iStart);
    if (iEnd == sstring::npos)
      return sstring::npos;
    size_t iFind = data.find(toFind.upper().c_str(), iStart);
    if (iFind != sstring::npos && iFind < iEnd)
      return iFind;
    iStart = iEnd + end.length();
  }
  return sstring::npos;
}

// returns the number of times the substring appears in this string
int sstring::countSubstr(const sstring sub) const
{
  int c = 0;
  for(size_t pos = find(sub); pos != sstring::npos; pos = find(sub, pos+1))
    c++;
  return c;
}

// removes characters which are now allowable in ascii
void sstring::ascify()
{
  size_t remove = sstring::npos;
  for(size_t i = 0; i < length(); i++)
  {
    int c = (*this)[i];
    bool nonAscii = (c < 0 || c > 127);

    if (remove == sstring::npos && nonAscii)
      remove = i;
    else if (remove != sstring::npos && !nonAscii)
    {
      replace(remove, i-remove, "", 0);
      i = remove;
      remove = sstring::npos;
    }
  }
  if (remove != sstring::npos)
  {
    replace(remove, length()-remove, "", 0);
  }
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
void sstring::convertStringColor(const sstring replacement)
{
  // we use <tmpi> to represent a dummy placeholder which we convert to
  // <z> at the end
  sstring repl = "<tmpi>";
  repl += replacement;
 
  while (find("<z>") != sstring::npos)  
    replace(find("<z>"), 3, repl);

  while (find("<Z>") != sstring::npos)  
    replace(find("<Z>"), 3, repl);

  while (find("<1>") != sstring::npos)  
    replace(find("<1>"), 3, repl);

  while (find("<tmpi>") != sstring::npos)  
    replace(find("<tmpi>"), 6, "<z>");
}


bool isvowel(const char c)
{
  switch (c) {
    case 'A':
    case 'E':
    case 'I':
    case 'O':
    case 'U':
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
      return true;
    default:
      return false;
    }
}

