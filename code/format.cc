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
    sstring oBuf=x;
    oBuf=oBuf.replaceString("&", "&#38;");
    oBuf=oBuf.replaceString("<", "&#60;");
    oBuf=oBuf.replaceString(">", "&#62;");

    // styles
    oBuf=oBuf.replaceString(VT_BOLDTEX, "<font style=\"bold\" />");
    oBuf=oBuf.replaceString(ANSI_UNDER, "<font style=\"under\" />");
    oBuf=oBuf.replaceString(VT_INVERTT, "<font style=\"invert\" />");
    oBuf=oBuf.replaceString(ANSI_FLASH, "<font style=\"flash\" />");

    // colors
    oBuf=oBuf.replaceString(ANSI_WHITE, "<font color=\"white\" />");
    oBuf=oBuf.replaceString(ANSI_BLACK, "<font color=\"black\" />");
    oBuf=oBuf.replaceString(ANSI_RED, "<font color=\"red\" />");
    oBuf=oBuf.replaceString(ANSI_NORMAL, "<font color=\"norm\" />");
    oBuf=oBuf.replaceString(ANSI_BLUE, "<font color=\"blue\" />");
    oBuf=oBuf.replaceString(ANSI_CYAN, "<font color=\"cyan\" />");
    oBuf=oBuf.replaceString(ANSI_GREEN, "<font color=\"green\" />");
    oBuf=oBuf.replaceString(ANSI_ORANGE, "<font color=\"orange\" />");
    oBuf=oBuf.replaceString(ANSI_PURPLE, "<font color=\"purple\" />");

    // colors with styles
    oBuf=oBuf.replaceString(ANSI_RED_BOLD, 
			    "<font style=\"bold\" color=\"red\" />");
    oBuf=oBuf.replaceString(ANSI_GREEN_BOLD, 
			    "<font style=\"bold\" color=\"green\" />");
    oBuf=oBuf.replaceString(ANSI_ORANGE_BOLD, 
			    "<font style=\"bold\" color=\"orange\" />");
    oBuf=oBuf.replaceString(ANSI_YELLOW_BOLD, 
			    "<font style=\"bold\" color=\"yellow\" />");
    oBuf=oBuf.replaceString(ANSI_BLUE_BOLD, 
			    "<font style=\"bold\" color=\"blue\" />");
    oBuf=oBuf.replaceString(ANSI_PURPLE_BOLD, 
			    "<font style=\"bold\" color=\"purple\" />");
    oBuf=oBuf.replaceString(ANSI_CYAN_BOLD, 
			    "<font style=\"bold\" color=\"cyan\" />");
    oBuf=oBuf.replaceString(ANSI_WHITE_BOLD, 
			    "<font style=\"bold\" color=\"white\" />");

    // colors with background
    oBuf=oBuf.replaceString(ANSI_BK_ON_BK, 
			    "<font bgcolor=\"black\" color=\"black\" />");
    oBuf=oBuf.replaceString(ANSI_BK_ON_WH, 
			    "<font bgcolor=\"white\" color=\"black\" />");
    oBuf=oBuf.replaceString(ANSI_WH_ON_BL, 
			    "<font bgcolor=\"blue\" color=\"white\" />");
    oBuf=oBuf.replaceString(ANSI_WH_ON_CY, 
			    "<font bgcolor=\"cyan\" color=\"white\" />");
    oBuf=oBuf.replaceString(ANSI_WH_ON_GR, 
			    "<font bgcolor=\"green\" color=\"white\" />");
    oBuf=oBuf.replaceString(ANSI_WH_ON_OR, 
			    "<font bgcolor=\"orange\" color=\"white\" />");
    oBuf=oBuf.replaceString(ANSI_WH_ON_PR, 
			    "<font bgcolor=\"purple\" color=\"white\" />");
    oBuf=oBuf.replaceString(ANSI_WH_ON_RD, 
			    "<font bgcolor=\"red\" color=\"white\" />");

    snprintf(buf, MY_MAX_STRING_LENGTH, fmtq.c_str(), oBuf.c_str());
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

