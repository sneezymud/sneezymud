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

    // process mud color codes
    oBuf.inlineReplaceString("<h>", MUD_NAME);
    oBuf.inlineReplaceString("<H>", MUD_NAME_VERS);
    oBuf.inlineReplaceString("<R>", ANSI_RED_BOLD);

    oBuf.inlineReplaceString("<r>",
			     (sstring)(ANSI_NORMAL)+(sstring)(ANSI_RED));
    oBuf.inlineReplaceString("<G>", ANSI_GREEN_BOLD);
    oBuf.inlineReplaceString("<g>", 
			     (sstring)(ANSI_NORMAL)+(sstring)(ANSI_GREEN));
    oBuf.inlineReplaceString("<y>", ANSI_ORANGE_BOLD);
    oBuf.inlineReplaceString("<Y>", ANSI_ORANGE_BOLD);
    oBuf.inlineReplaceString("<o>", 
			     (sstring)(ANSI_NORMAL)+(sstring)(ANSI_ORANGE));
    oBuf.inlineReplaceString("<O>", 
			     (sstring)(ANSI_NORMAL)+(sstring)(ANSI_ORANGE));
    oBuf.inlineReplaceString("<B>", ANSI_BLUE_BOLD);
    oBuf.inlineReplaceString("<b>", 
			     (sstring)(ANSI_NORMAL)+(sstring)(ANSI_BLUE));
    oBuf.inlineReplaceString("<P>", ANSI_PURPLE_BOLD);
    oBuf.inlineReplaceString("<p>", 
			     (sstring)(ANSI_NORMAL)+(sstring)(ANSI_PURPLE));
    oBuf.inlineReplaceString("<C>", ANSI_CYAN_BOLD);
    oBuf.inlineReplaceString("<c>", 
			     (sstring)(ANSI_NORMAL)+(sstring)(ANSI_CYAN));
    oBuf.inlineReplaceString("<W>", ANSI_WHITE_BOLD);
    oBuf.inlineReplaceString("<w>", 
			     (sstring)(ANSI_NORMAL)+(sstring)(ANSI_WHITE));
    oBuf.inlineReplaceString("<k>", 
			     (sstring)(VT_BOLDTEX)+(sstring)(ANSI_BLACK));
    oBuf.inlineReplaceString("<K>", 
			     (sstring)(ANSI_NORMAL)+(sstring)(ANSI_BLACK));
    oBuf.inlineReplaceString("<A>", 
			     (sstring)(VT_BOLDTEX)+(sstring)(ANSI_UNDER));
    oBuf.inlineReplaceString("<a>", ANSI_UNDER);
    oBuf.inlineReplaceString("<D>", VT_BOLDTEX);
    oBuf.inlineReplaceString("<d>", VT_BOLDTEX);
    oBuf.inlineReplaceString("<F>", ANSI_FLASH);
    oBuf.inlineReplaceString("<f>", ANSI_FLASH);
    oBuf.inlineReplaceString("<i>", VT_INVERTT);
    oBuf.inlineReplaceString("<I>", VT_INVERTT);
    oBuf.inlineReplaceString("<e>", ANSI_BK_ON_WH);
    oBuf.inlineReplaceString("<E>", ANSI_BK_ON_WH);
    oBuf.inlineReplaceString("<j>", ANSI_BK_ON_BK);
    oBuf.inlineReplaceString("<J>", ANSI_BK_ON_BK);
    oBuf.inlineReplaceString("<l>", ANSI_WH_ON_RD);
    oBuf.inlineReplaceString("<L>", ANSI_WH_ON_RD);
    oBuf.inlineReplaceString("<q>", ANSI_WH_ON_GR);
    oBuf.inlineReplaceString("<Q>", ANSI_WH_ON_GR);
    oBuf.inlineReplaceString("<t>", ANSI_WH_ON_OR);
    oBuf.inlineReplaceString("<T>", ANSI_WH_ON_OR);
    oBuf.inlineReplaceString("<u>", ANSI_WH_ON_BL);
    oBuf.inlineReplaceString("<U>", ANSI_WH_ON_BL);
    oBuf.inlineReplaceString("<v>", ANSI_WH_ON_PR);
    oBuf.inlineReplaceString("<V>", ANSI_WH_ON_PR);
    oBuf.inlineReplaceString("<x>", ANSI_WH_ON_CY);
    oBuf.inlineReplaceString("<X>", ANSI_WH_ON_CY);
    oBuf.inlineReplaceString("<z>", ANSI_NORMAL);
    oBuf.inlineReplaceString("<Z>", ANSI_NORMAL);
    oBuf.inlineReplaceString("<1>", ANSI_NORMAL);    
    

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

