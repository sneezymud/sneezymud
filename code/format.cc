#include "stdsneezy.h"
#include <mysql/mysql.h>
#include <boost/regex.hpp>

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

    // process mud color codes
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<h>"), 
		       ("$1"+(sstring)MUD_NAME));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<H>"), 
		       ("$1"+(sstring)MUD_NAME_VERS));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<R>"), 
		       ("$1"+(sstring)ANSI_RED_BOLD));

    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<r>"), 
		       ("$1"+(sstring)(ANSI_NORMAL)+(sstring)(ANSI_RED)));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<G>"), 
		       ("$1"+(sstring)ANSI_GREEN_BOLD));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<g>"), 
		       ("$1"+(string)(ANSI_NORMAL)+(sstring)(ANSI_GREEN)));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<y>"), 
		       ("$1"+(sstring)ANSI_ORANGE_BOLD));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<Y>"), 
		       ("$1"+(sstring)ANSI_ORANGE_BOLD));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<o>"), 
		       ("$1"+(sstring)(ANSI_NORMAL)+(sstring)(ANSI_ORANGE)));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<O>"), 
		       ("$1"+(sstring)(ANSI_NORMAL)+(sstring)(ANSI_ORANGE)));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<B>"), 
		       ("$1"+(sstring)ANSI_BLUE_BOLD));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<b>"), 
		       ("$1"+(sstring)(ANSI_NORMAL)+(sstring)(ANSI_BLUE)));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<P>"), 
		       ("$1"+(sstring)ANSI_PURPLE_BOLD));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<p>"), 
		       ("$1"+(sstring)(ANSI_NORMAL)+(sstring)(ANSI_PURPLE)));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<C>"), 
		       ("$1"+(sstring)ANSI_CYAN_BOLD));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<c>"), 
		       ("$1"+(sstring)(ANSI_NORMAL)+(sstring)(ANSI_CYAN)));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<W>"), 
		       ("$1"+(sstring)ANSI_WHITE_BOLD));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<w>"), 
		       ("$1"+(sstring)(ANSI_NORMAL)+(sstring)(ANSI_WHITE)));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<k>"), 
		       ("$1"+(sstring)(VT_BOLDTEX)+(sstring)(ANSI_BLACK)));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<K>"), 
		       ("$1"+(sstring)(ANSI_NORMAL)+(sstring)(ANSI_BLACK)));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<A>"), 
		       ("$1"+(sstring)(VT_BOLDTEX)+(sstring)(ANSI_UNDER)));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<a>"), 
		       ("$1"+(sstring)ANSI_UNDER));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<D>"), 
		       ("$1"+(sstring)VT_BOLDTEX));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<d>"), 
		       ("$1"+(sstring)VT_BOLDTEX));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<F>"), 
		       ("$1"+(sstring)ANSI_FLASH));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<f>"), 
		       ("$1"+(sstring)ANSI_FLASH));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<i>"), 
		       ("$1"+(sstring)VT_INVERTT));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<I>"), 
		       ("$1"+(sstring)VT_INVERTT));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<e>"), 
		       ("$1"+(sstring)ANSI_BK_ON_WH));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<E>"), 
		       ("$1"+(sstring)ANSI_BK_ON_WH));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<j>"), 
		       ("$1"+(sstring)ANSI_BK_ON_BK));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<J>"), 
		       ("$1"+(sstring)ANSI_BK_ON_BK));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<l>"), 
		       ("$1"+(sstring)ANSI_WH_ON_RD));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<L>"), 
		       ("$1"+(sstring)ANSI_WH_ON_RD));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<q>"), 
		       ("$1"+(sstring)ANSI_WH_ON_GR));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<Q>"), 
		       ("$1"+(sstring)ANSI_WH_ON_GR));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<t>"), 
		       ("$1"+(sstring)ANSI_WH_ON_OR));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<T>"), 
		       ("$1"+(sstring)ANSI_WH_ON_OR));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<u>"), 
		       ("$1"+(sstring)ANSI_WH_ON_BL));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<U>"), 
		       ("$1"+(sstring)ANSI_WH_ON_BL));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<v>"), 
		       ("$1"+(sstring)ANSI_WH_ON_PR));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<V>"), 
		       ("$1"+(sstring)ANSI_WH_ON_PR));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<x>"), 
		       ("$1"+(sstring)ANSI_WH_ON_CY));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<X>"), 
		       ("$1"+(sstring)ANSI_WH_ON_CY));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<z>"), 
		       ("$1"+(sstring)ANSI_NORMAL));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<Z>"), 
		       ("$1"+(sstring)ANSI_NORMAL));
    oBuf=regex_replace(oBuf, boost::regex("(^|[^<])<1>"), 
		       ("$1"+(sstring)ANSI_NORMAL));    

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

