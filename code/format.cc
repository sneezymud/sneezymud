#include "stdsneezy.h"
#include <mysql/mysql.h>

sstring fmt::doFormat(const sstring &fmt, const char *x)
{
  unsigned int MY_MAX_STRING_LENGTH=MAX_STRING_LENGTH * 2;
  char buf[MY_MAX_STRING_LENGTH];

  if(fmt[1] == 'q'){
    char to[MY_MAX_STRING_LENGTH];
    sstring fmtq=fmt;

    fmtq[1]='s';
    mysql_escape_string(to, x, strlen(x));

    snprintf(buf, MY_MAX_STRING_LENGTH, fmtq.c_str(), to);
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

