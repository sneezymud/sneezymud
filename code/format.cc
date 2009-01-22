#include "stdsneezy.h"
#include <mysql/mysql.h>

sstring fmt::doFormat(const sstring &fmt, const char *x)
{
  unsigned int MY_MAX_STRING_LENGTH=MAX_STRING_LENGTH * 2;
  char buf[MY_MAX_STRING_LENGTH];
  char to[MY_MAX_STRING_LENGTH];
  sstring fmtq=fmt;

  if(fmt[1] == 'q'){
    fmtq[1]='s';
    mysql_escape_string(to, x, strlen(x));

    snprintf(buf, MY_MAX_STRING_LENGTH, fmtq.c_str(), to);
  } else if(fmt[1] == 'x'){
    fmtq[1]='s';
    sstring outputBuf=x;
    outputBuf=outputBuf.replaceString("&", "&#38;");
    outputBuf=outputBuf.replaceString("<", "&#60;");
    outputBuf=outputBuf.replaceString(">", "&#62;");

    snprintf(buf, MY_MAX_STRING_LENGTH, fmtq.c_str(), outputBuf.c_str());
  } else {
    snprintf(buf, MY_MAX_STRING_LENGTH, fmt.c_str(), x);
  }

  if(strlen(buf) == MY_MAX_STRING_LENGTH - 1){
    vlogf(LOG_BUG, "fmt::doFormat(): buffer reached MAX_STRING_LENGTH");

    // can't use fmt here of course
    vlogf(LOG_BUG, sstring("fmt::doFormat(): buffer=")+
	  sstring(buf).substr(70));
  }

  return (sstring) buf;
}

sstring fmt::doFormat(const sstring &fmt, const sstring &x)
{
  return doFormat(fmt, x.c_str());
}


sstring fmt::doFormat(const sstring &fmt, const string &x)
{
  return doFormat(fmt, x.c_str());
}

