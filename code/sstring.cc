#include "stdsneezy.h"
#include "sstring.h"

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

const sstring sstring::lower() const
{
  sstring::size_type iter;
  sstring s=*this;

  do {
    iter = s.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if (iter != sstring::npos)
      s.replace(iter, 1, tolower(s[iter]));
  } while (iter != sstring::npos);

  return s;
}

const sstring sstring::upper() const
{
  sstring::size_type iter;
  sstring s=*this;

  do {
    iter = s.find_first_of("abcdefghijklmnopqrstuvwxyz");
    if (iter != sstring::npos)
      s.replace(iter, 1, toupper(s[iter]));
  } while (iter != sstring::npos);

  return s;
}

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

