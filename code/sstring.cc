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

const sstring & sstring::operator=(fmt &a)
{
  this->assign(a);
  return *this;
}



