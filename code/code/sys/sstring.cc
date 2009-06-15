#include <iostream>
#include <stdexcept>
#include <mysql/mysql.h>
#include <boost/regex.hpp>
#include "sstring.h"
#include "db.h"
#include "extern.h"
#include "ansi.h"
#include "parse.h"
#include "configuration.h"

const sstring sstring::escape(stringEscapeT escape_type) const
{
  sstring oBuf;
  unsigned int MY_MAX_STRING_LENGTH=MAX_STRING_LENGTH * 2;

  if(escape_type==SQL){
    char buf[MY_MAX_STRING_LENGTH];
    mysql_escape_string(buf, c_str(), strlen(c_str()));
    oBuf=(sstring)buf;
  } else if(escape_type==XML){
    boost::regex e("(^|[^<])<.>");
    boost::sregex_iterator m((*this).begin(), (*this).end(), e);
    boost::sregex_iterator last_m;
    boost::sregex_iterator end;
    char code;
    oBuf="";

    for(;m!=end;++m){
      oBuf.append((*m).prefix());
      oBuf.append((*m)[1]);
      
      if((*m)[0].str()[1]=='<')
	code=(*m)[0].str()[2];
      else
	code=(*m)[0].str()[1];


      switch(code){
	case 'h':
	  oBuf.append(MUD_NAME);
	  break;
	case 'H':
	  oBuf.append(MUD_NAME_VERS);
	  break;
	case 'R':
	  oBuf.append(ANSI_RED_BOLD);
	  break;
	case 'r':
	  oBuf.append(ANSI_NORMAL);
	  oBuf.append(ANSI_RED);
	  break;
	case 'G':
	  oBuf.append(ANSI_GREEN_BOLD);
	  break;
	case 'g':
	  oBuf.append(ANSI_NORMAL);
	  oBuf.append(ANSI_GREEN);
	  break;
	case 'y':
	  oBuf.append(ANSI_ORANGE_BOLD);
	  break;
	case 'Y':
	  oBuf.append(ANSI_ORANGE_BOLD);
	  break;
	case 'o':
	  oBuf.append(ANSI_NORMAL);
	  oBuf.append(ANSI_ORANGE);
	  break;
	case 'O':
	  oBuf.append(ANSI_NORMAL);
	  oBuf.append(ANSI_ORANGE);
	  break;
	case 'B':
	  oBuf.append(ANSI_BLUE_BOLD);
	  break;
	case 'b':
	  oBuf.append(ANSI_NORMAL);
	  oBuf.append(ANSI_BLUE);
	  break;
	case 'P':
	  oBuf.append(ANSI_PURPLE_BOLD);
	  break;
	case 'p':
	  oBuf.append(ANSI_NORMAL);
	  oBuf.append(ANSI_PURPLE);
	  break;
	case 'C':
	  oBuf.append(ANSI_CYAN_BOLD);
	  break;
	case 'c':
	  oBuf.append(ANSI_NORMAL);
	  oBuf.append(ANSI_CYAN);
	  break;
	case 'W':
	  oBuf.append(ANSI_WHITE_BOLD);
	  break;
	case 'w':
	  oBuf.append(ANSI_NORMAL);
	  oBuf.append(ANSI_WHITE);
	  break;
	case 'k':
	  oBuf.append(VT_BOLDTEX);
	  oBuf.append(ANSI_BLACK);
	  break;
	case 'K':
	  oBuf.append(ANSI_NORMAL);
	  oBuf.append(ANSI_BLACK);
	  break;
	case 'A':
	  oBuf.append(VT_BOLDTEX);
	  oBuf.append(ANSI_UNDER);
	  break;
	case 'a':
	  oBuf.append(ANSI_UNDER);
	  break;
	case 'D':
	  oBuf.append(VT_BOLDTEX);
	  break;
	case 'd':
	  oBuf.append(VT_BOLDTEX);
	  break;
	case 'F':
	  oBuf.append(ANSI_FLASH);
	  break;
	case 'f':
	  oBuf.append(ANSI_FLASH);
	  break;
	case 'i':
	  oBuf.append(VT_INVERTT);
	  break;
	case 'I':
	  oBuf.append(VT_INVERTT);
	  break;
	case 'e':
	  oBuf.append(ANSI_BK_ON_WH);
	  break;
	case 'E':
	  oBuf.append(ANSI_BK_ON_WH);
	  break;
	case 'j':
	  oBuf.append(ANSI_BK_ON_BK);
	  break;
	case 'J':
	  oBuf.append(ANSI_BK_ON_BK);
	  break;
	case 'l':
	  oBuf.append(ANSI_WH_ON_RD);
	  break;
	case 'L':
	  oBuf.append(ANSI_WH_ON_RD);
	  break;
	case 'q':
	  oBuf.append(ANSI_WH_ON_GR);
	  break;
	case 'Q':
	  oBuf.append(ANSI_WH_ON_GR);
	  break;
	case 't':
	  oBuf.append(ANSI_WH_ON_OR);
	  break;
	case 'T':
	  oBuf.append(ANSI_WH_ON_OR);
	  break;
	case 'u':
	  oBuf.append(ANSI_WH_ON_BL);
	  break;
	case 'U':
	  oBuf.append(ANSI_WH_ON_BL);
	  break;
	case 'v':
	  oBuf.append(ANSI_WH_ON_PR);
	  break;
	case 'V':
	  oBuf.append(ANSI_WH_ON_PR);
	  break;
	case 'x':
	  oBuf.append(ANSI_WH_ON_CY);
	  break;
	case 'X':
	  oBuf.append(ANSI_WH_ON_CY);
	  break;
	case 'z':
	  oBuf.append(ANSI_NORMAL);
	  break;
	case 'Z':
	  oBuf.append(ANSI_NORMAL);
	  break;
	case '1':
	  oBuf.append(ANSI_NORMAL);    
	  break;
      }      
      last_m=m;
    }
    if(last_m!=m)
      oBuf.append((*last_m).suffix());
    else
      oBuf=*this;

    oBuf.inlineReplaceString("<<", "<");
    
    // escape for xml
    oBuf.inlineReplaceString("&", "&#38;");
    oBuf.inlineReplaceString("<", "&#60;");
    oBuf.inlineReplaceString(">", "&#62;");

    // ansi font styles
    oBuf.inlineReplaceString(VT_BOLDTEX, "<font style=\"bold\" />");
    oBuf.inlineReplaceString(ANSI_UNDER, "<font style=\"under\" />");
    oBuf.inlineReplaceString(VT_INVERTT, "<font style=\"invert\" />");
    oBuf.inlineReplaceString(ANSI_FLASH, "<font style=\"flash\" />");
    // ansi font colors
    oBuf.inlineReplaceString(ANSI_WHITE, "<font color=\"white\" />");
    oBuf.inlineReplaceString(ANSI_BLACK, "<font color=\"black\" />");
    oBuf.inlineReplaceString(ANSI_RED, "<font color=\"red\" />");
    oBuf.inlineReplaceString(ANSI_NORMAL, "<font color=\"norm\" />");
    oBuf.inlineReplaceString(ANSI_BLUE, "<font color=\"blue\" />");
    oBuf.inlineReplaceString(ANSI_CYAN, "<font color=\"cyan\" />");
    oBuf.inlineReplaceString(ANSI_GREEN, "<font color=\"green\" />");
    oBuf.inlineReplaceString(ANSI_ORANGE, "<font color=\"orange\" />");
    oBuf.inlineReplaceString(ANSI_PURPLE, "<font color=\"purple\" />");

    // colors with styles
    oBuf.inlineReplaceString(ANSI_RED_BOLD, 
			    "<font style=\"bold\" color=\"red\" />");
    oBuf.inlineReplaceString(ANSI_GREEN_BOLD, 
			    "<font style=\"bold\" color=\"green\" />");
    oBuf.inlineReplaceString(ANSI_ORANGE_BOLD, 
			    "<font style=\"bold\" color=\"orange\" />");
    oBuf.inlineReplaceString(ANSI_YELLOW_BOLD, 
			    "<font style=\"bold\" color=\"yellow\" />");
    oBuf.inlineReplaceString(ANSI_BLUE_BOLD, 
			    "<font style=\"bold\" color=\"blue\" />");
    oBuf.inlineReplaceString(ANSI_PURPLE_BOLD, 
			    "<font style=\"bold\" color=\"purple\" />");
    oBuf.inlineReplaceString(ANSI_CYAN_BOLD, 
			    "<font style=\"bold\" color=\"cyan\" />");
    oBuf.inlineReplaceString(ANSI_WHITE_BOLD, 
			    "<font style=\"bold\" color=\"white\" />");

    // colors with background
    oBuf.inlineReplaceString(ANSI_BK_ON_BK, 
			    "<font bgcolor=\"black\" color=\"black\" />");
    oBuf.inlineReplaceString(ANSI_BK_ON_WH, 
			    "<font bgcolor=\"white\" color=\"black\" />");
    oBuf.inlineReplaceString(ANSI_WH_ON_BL, 
			    "<font bgcolor=\"blue\" color=\"white\" />");
    oBuf.inlineReplaceString(ANSI_WH_ON_CY, 
			    "<font bgcolor=\"cyan\" color=\"white\" />");
    oBuf.inlineReplaceString(ANSI_WH_ON_GR, 
			    "<font bgcolor=\"green\" color=\"white\" />");
    oBuf.inlineReplaceString(ANSI_WH_ON_OR, 
			    "<font bgcolor=\"orange\" color=\"white\" />");
    oBuf.inlineReplaceString(ANSI_WH_ON_PR, 
			    "<font bgcolor=\"purple\" color=\"white\" />");
    oBuf.inlineReplaceString(ANSI_WH_ON_RD, 
			    "<font bgcolor=\"red\" color=\"white\" />");
  }

  if(oBuf.length() == MY_MAX_STRING_LENGTH - 1){
    vlogf(LOG_BUG, "sstring::escape(): buffer reached MAX_STRING_LENGTH");

    // avoid formatting just to be safe
    vlogf(LOG_BUG, sstring("sstring::escape(): buffer=")+
	  oBuf.substr(70));
  }

  return oBuf;
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

// converts A-Z to lower case a-z
const sstring sstring::lower() const
{
  sstring s=*this;

  std::transform(s.begin(), s.end(), s.begin(), ::tolower);

  return s;
}


// converts a-z to upper case A-Z
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
const size_t sstring::lengthNoColor() const
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
  if(strlen(std::string::c_str()) > length())
    throw std::runtime_error(format("corruption in sstring::c_str").str());

  return std::string::c_str();
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


boost::format format(const std::string &f_string){
  boost::format fmter(f_string);
  if(Config::ThrowFormatExceptions()){
    fmter.exceptions(boost::io::all_error_bits);
  } else {
    fmter.exceptions(boost::io::no_error_bits);
  }
  return fmter;
}




