//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __ANSI_H
#define __ANSI_H

enum colorTypeT {
     COLOR_NONE, //       = 0;
     COLOR_ALWAYS, //     = 1;
     COLOR_NEVER, //      = 2;
     COLOR_BASIC, //      = 3;
     COLOR_COMM, //       = 4;
     COLOR_OBJECTS, //    = 5;
     COLOR_MOBS, //       = 6;
     COLOR_ROOMS, //      = 7;
     COLOR_ROOM_NAME, //  = 8;
     COLOR_SHOUTS, //     = 9;
     COLOR_SPELLS, //     = 10;
     COLOR_LOGS, //       = 11;
     COLOR_CODES, //      = 12;

     MAX_COLOR //        = 13;
};

const unsigned int PLR_COLOR_BASIC      = (1<<0);
const unsigned int PLR_COLOR_OBJECTS    = (1<<1);
const unsigned int PLR_COLOR_MOBS       = (1<<2);
const unsigned int PLR_COLOR_ROOMS      = (1<<3);
const unsigned int PLR_COLOR_ROOM_NAME  = (1<<4);
const unsigned int PLR_COLOR_SPELLS     = (1<<5);
const unsigned int PLR_COLOR_COMM       = (1<<6);
const unsigned int PLR_COLOR_SHOUTS     = (1<<7);
const unsigned int PLR_COLOR_LOGS       = (1<<8);
const unsigned int PLR_COLOR_CODES      = (1<<9);
const unsigned int PLR_COLOR_ALL        = (1<<10);

const unsigned int MAX_PLR_COLOR        = 11;

const char * const ANSI_MENU_1    = "txt/ansi/login1.ans";
const char * const ANSI_MENU_2    = "txt/ansi/login2.ans";
const char * const ANSI_MENU_3    = "txt/ansi/login3.ans";
const char * const NORM_MENU_1    = "txt/vt/login1.vt";
const char * const NORM_MENU_2    = "txt/vt/login2.vt";
const char * const NORM_MENU_3    = "txt/vt/login3.vt";
const char * const ANSI_OPEN      = "txt/ansi/title.ans";
const char * const NORM_OPEN      = "txt/vt/title.vt";

enum colorSubT {
     COLOR_SUB_NONE,     //         = 0;
     COLOR_SUB_BLACK,     //        = 1;
     COLOR_SUB_RED,     //          = 2;
     COLOR_SUB_GREEN,     //        = 3;
     COLOR_SUB_ORANGE,     //       = 4;
     COLOR_SUB_BLUE,     //         = 5;
     COLOR_SUB_PURPLE,     //       = 6;
     COLOR_SUB_CYAN,     //         = 7;
     COLOR_SUB_WHITE,     //        = 8;
     COLOR_SUB_YELLOW,     //       = 9;
     COLOR_SUB_GRAY,     //         = 10;
     COLOR_SUB_BOLD,     //         = 11;
     COLOR_SUB_BOLD_RED,     //     = 12;
     COLOR_SUB_BOLD_GREEN,     //   = 13;
     COLOR_SUB_BOLD_BLUE,     //    = 14;
     COLOR_SUB_BOLD_PURPLE,     //  = 15;
     COLOR_SUB_BOLD_CYAN,     //    = 16;
     COLOR_SUB_BOLD_WHITE      //   = 17;
};

const unsigned int COLOR_BLACK         = (1<<0); 
const unsigned int COLOR_RED           = (1<<1);
const unsigned int COLOR_UNDER         = (1<<2);
const unsigned int COLOR_GREEN         = (1<<3);
const unsigned int COLOR_ORANGE        = (1<<4);
const unsigned int COLOR_BLUE          = (1<<5);
const unsigned int COLOR_PURPLE        = (1<<6);
const unsigned int COLOR_CYAN          = (1<<7);
const unsigned int COLOR_WHITE         = (1<<8);
const unsigned int COLOR_YELLOW        = (1<<9);
const unsigned int COLOR_GRAY          = (1<<10);
const unsigned int COLOR_BOLD          = (1<<11);
const unsigned int COLOR_BOLD_RED      = (1<<12);
const unsigned int COLOR_BOLD_GREEN    = (1<<13);
const unsigned int COLOR_BOLD_BLUE     = (1<<14);
const unsigned int COLOR_BOLD_PURPLE   = (1<<15);
const unsigned int COLOR_BOLD_CYAN     = (1<<16);
const unsigned int COLOR_BOLD_WHITE    = (1<<17);


const char * const ANSI_NORMAL           = "\033[0m";
const char * const ANSI_FLASH            = "\033[5m";
const char * const ANSI_CHECK            = "\033[6n";
const char * const ANSI_BLACK            = "\033[30m";
const char * const ANSI_GRAY             = "\033[1;30m";
const char * const ANSI_RED              = "\033[31m";
const char * const ANSI_RED_BOLD         = "\033[1;31m";
const char * const ANSI_UNDER            = "\033[4m";
const char * const ANSI_GREEN            = "\033[32m";
const char * const ANSI_GREEN_BOLD       = "\033[1;32m";
const char * const ANSI_ORANGE           = "\033[33m";
const char * const ANSI_ORANGE_BOLD      = "\033[1;33m";
const char * const ANSI_YELLOW           = ANSI_ORANGE;
const char * const ANSI_YELLOW_BOLD      = ANSI_ORANGE_BOLD;
const char * const ANSI_BLUE             = "\033[34m";
const char * const ANSI_BLUE_BOLD        = "\033[1;34m";
const char * const ANSI_PURPLE           = "\033[35m";
const char * const ANSI_PURPLE_BOLD      = "\033[1;35m";
const char * const ANSI_CYAN             = "\033[36m";
const char * const ANSI_CYAN_BOLD        = "\033[1;36m";
const char * const ANSI_WHITE            = "\033[37m";
const char * const ANSI_WHITE_BOLD       = "\033[1;37m";

const char * const ANSI_BK_ON_BK   = "\033[8m";             /* black on black text */
const char * const ANSI_BK_ON_WH   = "\033[7m";            /* black on white text */
const char * const ANSI_WH_ON_RD   = "\033[41m";            /* white on red text */
const char * const ANSI_WH_ON_GR   = "\033[42m";            /* white on green text */
const char * const ANSI_WH_ON_OR   = "\033[43m";            /* white on orange text */
const char * const ANSI_WH_ON_BL   = "\033[44m";            /* white on blue text */
const char * const ANSI_WH_ON_PR   = "\033[45m";            /* white on purple text */
const char * const ANSI_WH_ON_CY   = "\033[46m";            /* white on cyan text */

const char * const ANSI_BAR1     = "\260";
const char * const ANSI_BAR2     = "\261";
const char * const ANSI_BAR3     = "\262";
const char * const ANSI_BAR4     = "\333";

const char * const ANSI_BLUE_BAR = "\033[34m\333\033[0m";

const char * const VT_UPARROW    = "\033[A";
const char * const VT_DNARROW    = "\033[B";
const char * const VT_LTARROW    = "\033[D";
const char * const VT_RTARROW    = "\033[C";

const char * const VT_INITSEQ    = "\033[1;24r";          /* fixes up margins */
const char * const VT_CURSPOS    = "\033[%d;%dH";         /* respositions cursor */
const char * const VT_CURSRIG    = "\033[%dC";            /* moves cursor right */
const char * const VT_CURSLEF    = "\033[%dD";           /* moves cursor left */
const char * const VT_HOMECLR    = "\033[2J\033[0;0H";
const char * const VT_CTEOTCR    = "\033[K";
const char * const VT_CLENSEQ    = "\033[r\033[2J";       /* clears and resets screen */
const char * const VT_INVERTT    = "\033[0;1;7m";         /* inverted text */
const char * const VT_BOLDTEX    = "\033[0;1m";           /* bold text */
const char * const VT_NORMALT    = "\033[0m";             /* normal text */
const char * const VT_MARGSET    = "\033[%d;%dr";         /* sets margins */
const char * const VT_CURSAVE    = "\0337";               /* saves cursor position */
const char * const VT_CURREST    = "\0338";               /* restores cursor position */

/* Stuff for vt and ansi terminal prompts */

const unsigned int CHANGED_HP     = (1<<0);
const unsigned int CHANGED_MANA   = (1<<1);
const unsigned int CHANGED_MOVE   = (1<<2);
const unsigned int CHANGED_GOLD   = (1<<3);
const unsigned int CHANGED_EXP    = (1<<4);
const unsigned int CHANGED_OPP    = (1<<5);
const unsigned int CHANGED_ROOM   = (1<<6);
const unsigned int CHANGED_PERC   = (1<<7);
const unsigned int CHANGED_TIME   = (1<<8);
const unsigned int CHANGED_MUD    = (1<<9);
const unsigned int CHANGED_PIETY  = (1<<10);
const unsigned int CHANGED_COND   = (1<<11);
const unsigned int CHANGED_POS    = (1<<12);
const unsigned int CHANGED_LIFEFORCE    = (1<<13);

enum setColorFieldT {
  SET_COL_FIELD_HIT,
  SET_COL_FIELD_MANA,
  SET_COL_FIELD_MOVE,
  SET_COL_FIELD_TALEN,
  SET_COL_FIELD_XP,
  SET_COL_FIELD_OPP,
  SET_COL_FIELD_ROOM,
  SET_COL_FIELD_TANK,
  SET_COL_FIELD_TANK_OTHER,
  SET_COL_FIELD_PIETY,
  SET_COL_FIELD_LIFEFORCE,
  SET_COL_FIELD_TIME
};

enum setColorKolorT {
  SET_COL_KOL_OFF,
  SET_COL_KOL_BLUE,
  SET_COL_KOL_RED,
  SET_COL_KOL_GREEN,
  SET_COL_KOL_WHITE,
  SET_COL_KOL_PURPLE,
  SET_COL_KOL_CYAN,
  SET_COL_KOL_ORANGE,
  SET_COL_KOL_YELLOW,
  SET_COL_KOL_CHARCOAL,
  SET_COL_KOL_WH_ON_BL,
  SET_COL_KOL_INVERT,
  SET_COL_KOL_WH_ON_CY,
  SET_COL_KOL_WH_ON_RD,
  SET_COL_KOL_WH_ON_PR,
  SET_COL_KOL_WH_ON_GR,
  SET_COL_KOL_WH_ON_OR,
  SET_COL_KOL_BLINK,
  SET_COL_KOL_BOLDRED,
  SET_COL_KOL_BOLDGREEN,
  SET_COL_KOL_BOLDBLUE,
  SET_COL_KOL_BOLDPURPLE,
  SET_COL_KOL_BOLDCYAN
};

#endif
