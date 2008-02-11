#include "stdsneezy.h"
#include "sstring.h"
#include "format.h"


// puts commas every 3rd char, for formatting number strings
const sstring sstring::comify() const
{
  sstring tString=*this;
  int  strCount, charIndex = 0;

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

  for (; (*this)[charIndex]; charIndex++)
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
    if ((*this)[loc] == '\n' && (*this)[loc-1] != '\r' &&
      (loc+1) <= len && (*this)[loc+1] != '\r') {
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
    
    switch ((*this)[i]) {
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
  return false;
} 

const sstring sstring::replaceString(sstring find, sstring replace) const
{
  sstring str = *this;
  str.inlineReplaceString(find, replace);
  return str;
}

const sstring & sstring::operator=(fmt &a)
{
  this->assign(a);
  return *this;
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
    if (data)
      data[iFound] = substr(iPosLast, iPos);
    iFound++;
    iPosLast = ++iPos;
  }
  if (data)
    data[iFound] = substr(iPosLast, length());
  return iFound + 1;
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

