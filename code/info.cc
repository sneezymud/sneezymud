//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: info.cc,v $
// Revision 1.4  1999/10/03 09:47:28  lapsos
// Added gag to failed sneak display.
//
// Revision 1.3  1999/10/02 06:37:44  lapsos
// Fixed up who -y to ignore immortals.
//
// Revision 1.2  1999/09/29 03:28:27  lapsos
// Added who -y flag.
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//                                                                      //
//    SneezyMUD++ - All rights reserved, SneezyMUD Coding Team      //
//                                                                      //
//    "info.cc" - All informative functions and routines                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

#include <algorithm>
#include <iostream.h>
#include <cmath>

#include "account.h"
#include "disease.h"
#include "games.h"
#include "combat.h"
#include "statistics.h"
#include "components.h"
#include "skillsort.h"

string describeDuration(const TBeing *ch, int dur)
{
  char buf[160];
  string ret;
  int weeks = 0, days = 0, hours = 0, mins = 0;
  int errnum = 0;

  if (dur == PERMANENT_DURATION) {
    ret = "permanent";
    return ret;
  }
  // random error
  if (!ch->isImmortal()) {
#if 0
    errnum = ch->plotStat(STAT_CURRENT, STAT_PER, 175, 15, 75); 
    errnum = ::number(-1 * errnum, 1 * errnum);
#else
    // bad to randomize it, just have them overestimate it
    errnum = ch->plotStat(STAT_CURRENT, STAT_PER, 400, 30, 175);
#endif
    dur *= 1000 + errnum;
    dur /= 1000;
    dur = max(1, dur);
  }
  // aff->dur decrements once per PULSE_COMBAT
  // 1 tick = 30 mud mins = 1 PULSE_TICKS

  hours = dur * PULSE_COMBAT/ PULSE_TICKS / 2;
  mins = dur * 30 * PULSE_COMBAT/ PULSE_TICKS % 60;

  if (hours >= 24) {
    days = hours/24;
    hours = hours % 24;
  }
  if (days >= 7) {
    weeks = days / 7;
    days = days % 7;
  }
  *buf = '\0';
  if (weeks)
    sprintf(buf + strlen(buf), "%d week%s, ", weeks, (weeks == 1 ? "" : "s"));
  if (days)
    sprintf(buf + strlen(buf), "%d day%s, ", days, (days == 1 ? "" : "s"));
  if (hours)
    sprintf(buf + strlen(buf), "%d hour%s, ", hours, (hours == 1 ? "" : "s"));
  if (mins)
    sprintf(buf + strlen(buf), "%d minute%s, ", mins, (mins== 1 ? "" : "s"));
    
  if (strlen(buf) > 0) {
    while (buf[strlen(buf) - 1] == ' ' || buf[strlen(buf) - 1] == ',')
      buf[strlen(buf) - 1] = '\0';
  }
  ret = buf;
  return ret;
}

void argument_split_2(const char *argument, char *first_arg, char *second_arg)
{
  int look_at, begin;
  begin = 0;

  for (; *(argument + begin) == ' '; begin++);

  for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

  *(first_arg + look_at) = '\0';
  begin += look_at;

  for (; *(argument + begin) == ' '; begin++);

  for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
    *(second_arg + look_at) = LOWER(*(argument + begin + look_at));

  *(second_arg + look_at) = '\0';
  begin += look_at;
}

bool hasColorStrings(const TBeing *mob, const char *arg, int field)
{
  const char *s = NULL;
  int found = 0;

  if (!arg || !*arg)
    return FALSE;

  mud_assert(field >=1 && field <= 2, "Bad args");

  switch (field) {
    case 1:
      s = mob->getName();
      break;
    case 2:
      s = arg;
      break;
  }
  for (; *s; s++) {

    if ((*s == '<') && ((*(s + 2)) == '>')) {
      switch (*(s + 1)) {
        case 'b':
        case 'c':
        case 'g':
        case 'k':
        case 'o':
        case 'p':
        case 'r':
        case 'w':
        case 'y':
        case 'B':
        case 'C':
        case 'G':
        case 'K':
        case 'O':
        case 'P':
        case 'R':
        case 'W':
        case 'Y':
        case 'a':
        case 'A':
        case 'd':
        case 'D':
        case 'f':
        case 'F':
        case 'i':
        case 'I':
        case 'e':
        case 'E':
        case 'j':
        case 'J':
        case 'l':
        case 'L':
        case 'q':
        case 'Q':
        case 't':
        case 'T':
        case 'u':
        case 'U':
        case 'v':
        case 'V':
        case 'x':
        case 'X':
        case 'z':
        case 'Z':
        case '1':
          found = 1;
          break;
        case 'h':
        case 'H':
        case 'n':
        case 'N':
        default:
          break;
      }
    }
  }
  if (found)
    return TRUE;
  else
    return FALSE;

}

// takes the string given by arg, replaces any <m> or <M> in it with
// ting's name.  Colorizes as appropriate for me/ch.  Undoes any color
// changes that were made by insertion of ting's name string also.
string addNameToBuf(const TBeing *me, const Descriptor *ch, const TThing *ting, const char *arg, colorTypeT lev) 
{
  unsigned int s;
  unsigned int len;
  string buf;
  char tmp[256];
  bool y = FALSE;
  int x = 0;

  if (!ch)
    return arg;

  len = strlen(arg);
  buf = "";

  for(s = 0;len > 2 && s<(len-2);s++) {
    if ((arg[s] == '<') && (arg[s + 2] == '>')) {
      // two sequential << chars treat this as desiring to write "<"
      // we already wrote the first, so just skip this char
      if (s > 0 && arg[s-1] == '<') 
        continue;
      
      switch (arg[(s+1)]) {
        case 'm':
        case 'M':
          strcpy(tmp, ting->getName());
          if ((s == 0) || (y && (s == 3))) {
            cap(tmp);
          }
          if (lev != COLOR_NONE) 
            buf += colorString(me, ch, tmp, NULL, lev, FALSE);
          else 
            buf += tmp;
          
          if (y) {
            // Adding back in last colorString
            buf += arg[x];
            buf += arg[x + 1];
            buf += arg[x + 2];
          }
          s += 2;
          break;
        case 'R':
        case 'r':
        case 'b':
        case 'B':
        case 'g':
        case 'G':
        case 'c':
        case 'C':
        case 'p':
        case 'P':
        case 'o':
        case 'y':
        case 'O':
        case 'Y':
        case 'w':
        case 'W':
        case 'k':
        case 'K':
        case 'a':
        case 'A':
        case 'd':
        case 'D':
        case 'f':
        case 'F':
        case 'i':
        case 'I':
        case 'e':
        case 'E':
        case 'j':
        case 'J':
        case 'l':
        case 'L':
        case 'q':
        case 'Q':
        case 't':
        case 'T':
        case 'u':
        case 'U':
        case 'v':
        case 'V':
        case 'x':
        case 'X':
        case 'z':
        case 'Z':
        case '1':
        case 'h':
        case 'H':
          // if there is a color string, it will pick it up after <m>
          y = TRUE;
          x = s; 
        // pass through
        default:
          buf += arg[s];
          break;
      }
    } else 
      buf += arg[s];
  }
  while(s < len) 
    buf += arg[s++];
  
  return buf;
}

string nameColorString(TBeing *me, Descriptor *ch, const char *arg, int *flag, colorTypeT, bool noret)
{
  unsigned int len, s;
  string buf;
  char tmp[256];

  if (!ch)
    return arg;
          
  len = strlen(arg);
  buf = "";

  for(s = 0;len > 2 && s < (len-2); s++) {
    if ((arg[s] == '<') && (arg[s + 2] == '>')) {
      if (s > 0 && arg[s-1] == '<') {
        // two sequential << chars treat this as desiring to write "<"
        // we already wrote the first, so just skip this char
        continue;
      }
      switch (arg[(s+1)]) {
        case 'n':
        case 'N':
          if (me) {
            strcpy(tmp, me->getName());
            buf += cap(tmp);
            if (flag)
              *flag = TRUE;
            s += 2;
          } else 
            buf += arg[s];
          
          break;
        default:
          buf += arg[s];
          break;
      }   
    } else 
      buf += arg[s];
  }
  while(s < len) 
    buf += arg[s++];
  
    // force a nice termination
  buf += "<z>";

  if (noret) 
    buf += "\n\r";

  return buf;
}

const string colorString(const TBeing *me, const Descriptor *ch, const char *arg, int *flag, colorTypeT lev, bool end, bool noret)
{
// (me = who to, ch is the desc, arg = arg, flag = ?, int lev = desired color
//  level, end = whether to send terminator at end of string..false if in
//  middle of senstence (color a mob or something).. used in act() 
//  noret is overloaded to add a \n\r to the end of the buf, it defaults
//  to no so if you dont pass anything, it will not return -Cos
  int len, s;
  string buf;
  char tmp[80];
  bool colorize = TRUE;
  bool addNorm = FALSE;

  if (!arg)
    return ("");

  if (!ch && lev != COLOR_NONE && lev != COLOR_NEVER)
    return arg;

  if (ch && IS_SET(ch->plr_color, PLR_COLOR_CODES)) {
    return arg;
  }
  switch (lev) {
    case COLOR_ALWAYS:
    case COLOR_BASIC:   //allows for basic ansi
      colorize = TRUE; 
      break;
    case COLOR_NONE:
    case COLOR_NEVER:
      colorize = FALSE;
      break;
    case COLOR_COMM:
      if (!(IS_SET(ch->plr_color, PLR_COLOR_COMM))) 
         colorize = FALSE;
      else 
        colorize = TRUE;
      
      break;
    case COLOR_OBJECTS:
      if (!(IS_SET(ch->plr_color, PLR_COLOR_OBJECTS))) 
         colorize = FALSE;
      else 
        colorize = TRUE; 
      
      break;
    case COLOR_MOBS:
      if (!(IS_SET(ch->plr_color, PLR_COLOR_MOBS))) 
         colorize = FALSE;
      else 
        colorize = TRUE;
      
      break;
    case COLOR_ROOMS:
      if (!(IS_SET(ch->plr_color, PLR_COLOR_ROOMS))) 
         colorize = FALSE;
      else { 
        addNorm = TRUE;
        colorize = TRUE;
      }
      break;
    case COLOR_ROOM_NAME:
      if (!(IS_SET(ch->plr_color, PLR_COLOR_ROOM_NAME))) 
         colorize = FALSE;
      else {
        addNorm = TRUE;
        colorize = TRUE;
      }
      break;
    case COLOR_SHOUTS:
      if (!(IS_SET(ch->plr_color, PLR_COLOR_SHOUTS))) 
         colorize = FALSE;
      else 
        colorize = TRUE;
      
      break;
    case COLOR_SPELLS:
      if (!(IS_SET(ch->plr_color, PLR_COLOR_SPELLS))) 
         colorize = FALSE;
      else 
        colorize = TRUE;
      
      break;
    case COLOR_LOGS:
      if (!(IS_SET(ch->plr_color, PLR_COLOR_LOGS))) 
         colorize = FALSE;
      else 
        colorize = TRUE;
      
      break;
    default:
      vlogf(5,"Colorstring with a default COLOR setting");
      colorize = TRUE;
      break;
  }
  len = strlen(arg);

  buf = "";
  if (colorize) {
    for(s = 0; s < (len - 2); s++){
      if ((arg[s] == '<') && (arg[s + 2] == '>')) {
        if (s > 0 && arg[s - 1] == '<') {
          // two sequential << chars treat this as desiring to write "<"
          // we already wrote the first, so just skip this char
          continue;
        }
        switch (arg[(s+1)]) {
          case 'h':
            buf += MUD_NAME;
            s += 2;
            break;
          case 'H':
            buf += MUD_NAME_VERS;
            s += 2;
            break;
          case 'R':
            buf += ch->redBold();
            s += 2;
            break;
          case 'r':
            if (addNorm) 
              buf += ch->norm();
            
            buf += ch->red();
            s += 2;
            break; 
          case 'G':
            buf += ch->greenBold();
            s += 2;
            break;
          case 'g':
            if (addNorm) 
              buf += ch->norm();
            
            buf += ch->green();
            s += 2;
            break;
          case 'y':
          case 'Y':
            // yellow 
            buf += ch->orangeBold();
            s += 2;
            break;
          case 'O':
          case 'o':
            if (addNorm) 
              buf += ch->norm();
            
            buf += ch->orange();
            s += 2;
            break;
          case 'B':
            buf += ch->blueBold();
            s += 2;
            break;
          case 'b':
            if (addNorm) 
              buf += ch->norm();
            
            buf += ch->blue();
            s += 2;
            break;
          case 'P':
            buf += ch->purpleBold();
            s += 2;
            break;
          case 'p':
            if (addNorm) 
              buf += ch->norm();
            
            buf += ch->purple();
            s += 2;
            break;
          case 'C':
            buf += ch->cyanBold();
            s += 2;
            break;
          case 'c':
            if (addNorm) 
              buf += ch->norm();
            
            buf += ch->cyan();
            s += 2;
            break;
          case 'W':
            buf += ch->whiteBold();
            s += 2;
            break;
          case 'w':
            if (addNorm) 
              buf += ch->norm();
            
            buf += ch->white();
            s += 2;
            break;
          case 'k':
            // NOTE this is the bold will be charcoal
            buf += ch->blackBold();
            s += 2;
            break;
          case 'K':
            if (addNorm) 
              buf += ch->norm();
            
            buf += ch->black();
            s += 2;
            break;
          case 'A':
            buf += ch->underBold();
            s += 2;
            break;
          case 'a':
            buf += ch->under();
            s += 2;
            break;
          case 'D':
          case 'd':
            buf += ch->bold();
            s += 2;
            break;
          case 'f':
          case 'F':
            if (me && me->isImmortal()) {
              buf += ch->flash();
              s += 2;
            } else 
              buf += arg[s];
            
            break;
          case 'i':
          case 'I':
            buf += ch->invert();
            s += 2;
            break;
          case 'e':
          case 'E':
            if (me && me->isImmortal()) {
              buf += ch->BlackOnWhite();
              s += 2;
            } else 
              buf += arg[s];
            
            break;
          case 'j':
          case 'J':
            if (me && me->isImmortal()) {
              buf += ch->BlackOnBlack();
              s += 2;
            } else 
              buf += arg[s];
            
            break;
          case 'l':
          case 'L':
            if (me && me->isImmortal()) {
              buf += ch->WhiteOnRed();
              s += 2;
            } else 
              buf += arg[s];
            
            break;
          case 'q':
          case 'Q':
            if (me && me->isImmortal()) {
              buf += ch->WhiteOnGreen();
              s += 2;
            } else 
              buf += arg[s];
            
            break;
          case 't':
          case 'T':
            if (me && me->isImmortal()) {
              buf += ch->WhiteOnOrange();
              s += 2;
            } else 
              buf += arg[s];
            
            break;
          case 'u':
          case 'U':
            if (me && me->isImmortal()) {
              buf += ch->WhiteOnBlue();
              s += 2;
            } else 
              buf += arg[s];
            
            break;
          case 'v':
          case 'V':
            if (me && me->isImmortal()) {
              buf += ch->WhiteOnPurple();
              s += 2;
            } else 
              buf += arg[s];
            
            break;
          case 'x':
          case 'X':
            if (me && me->isImmortal()) {
              buf += ch->WhiteOnCyan();
              s += 2;
            } else 
              buf += arg[s];
            
            break;
          case 'z':
          case 'Z':
          case '1':
            buf += ch->norm();
            s += 2;
            break;
          default:
            buf += arg[s];
            break;
        }
      } else 
        buf += arg[s];
    }   
    // copy the last 1 or 2 characters into buf.
    while(s < len) {
      // strip terminations, added again below if requested
      if (arg[s]) 
        buf += arg[s];
      
      s++;
    }
  } else {
    // colorize is off

    for (s = 0; s < (len-2); s++) {
      if ((arg[s] == '<') && (arg[s+2] == '>')) {
        if (s > 0 && arg[s-1] == '<') {
          // two sequential << chars treat this as desiring to write "<"
          // we already wrote the first, so just skip this char
          continue;
        }
        switch (arg[(s+1)]) {
          case 'n':
          case 'N':
            if (me) {
              strcpy(tmp, me->getName());
              buf += cap(tmp);
              if (flag)
                *flag = TRUE;
              s += 2;
            } else 
              buf += arg[s];
            
            break;
          case 'R':
          case 'r':
          case 'b':
          case 'B':
          case 'g':
          case 'G':
          case 'c':
          case 'C':
          case 'p':
          case 'P':
          case 'o':
          case 'y':
          case 'O':
          case 'Y':
          case 'w':
          case 'W':
          case 'k':
          case 'K':
          case 'a':
          case 'A':
          case 'd':
          case 'D':
          case 'f':
          case 'F':
          case 'i':
          case 'I':
          case 'e':
          case 'E':
          case 'j':
          case 'J':
          case 'l':
          case 'L':
          case 'q':
          case 'Q':
          case 't':
          case 'T':
          case 'u':
          case 'U':
          case 'v':
          case 'V':
          case 'x':
          case 'X':
          case 'z':
          case 'Z':
          case 'h':
          case 'H':
          case '1':
            s += 2;
            break;
          default:
            buf += arg[s];
            break;
        }
      } else 
        buf += arg[s];
    }
    // copy the last 1 or 2 characters into buf.
    while(s < len) {
      if (arg[s]) 
        buf += arg[s];
      
      s ++;
    }
  }


  // force a nice termination
  if (colorize) {
    if (end) 
      buf += ch->norm();
  }

  if (noret) 
    buf += "\n\r";

  return buf;
}

void TBeing::parseTitle(char *arg, Descriptor *)
{
  strcpy(arg, getName());
  return;
}

void TPerson::parseTitle(char *arg, Descriptor *user)
{
  int flag = FALSE;
  if (!title) {
    strcpy(arg, getName());
    return;
  }

  strcpy(arg, nameColorString(this, user, title, &flag, COLOR_BASIC, FALSE).c_str());
  if (!flag &&
      colorString(this, user, title, NULL, COLOR_NONE, TRUE).find(getNameNOC(this).c_str()) ==
      string::npos)
    strcpy(arg, getName());  // did not specify a <n>

  // explicitely terminate it since players are sloppy
  strcat(arg, "<1>");

  return;
}

void TBeing::doWhozone()
{
  Descriptor *d;
  TRoom *rp = NULL;
  char buf[256];
  TBeing *person = NULL;
  int count = 0;

  sendTo("Players:\n\r--------\n\r");
  for (d = descriptor_list; d; d = d->next) {
    if (!d->connected && canSee(d->character) &&
        (rp = real_roomp((person = (d->original ? d->original : d->character))->in_room)) &&
        (rp->getZone() == roomp->getZone())) {
      sprintf(buf, "%-25s - %s ", person->getName(), rp->name);
      if (GetMaxLevel() > MAX_MORT)
        sprintf(buf + strlen(buf), "[%d]", person->in_room);
      strcat(buf, "\n\r");
      sendTo(COLOR_BASIC, buf);
      count++;
    }
  }
  sendTo("\n\rTotal visible players: %d\n\r", count);
}

void Descriptor::menuWho() 
{
  TBeing *person;
  char buf[256];
  char buf2[256];
  char send[4096] = "\0";

  strcpy(send, "\n\r");

  for (person = character_list; person; person = person->next) {
    if (person->isPc() && person->polyed == POLY_TYPE_NONE) {
      if (dynamic_cast<TPerson *>(person) &&
          (person->getInvisLevel() < GOD_LEVEL1)) {
        person->parseTitle(buf, this);

        sprintf(buf2, "%s", colorString(person, this, buf, NULL, COLOR_BASIC, FALSE).c_str());
        strcat(buf2, "\n\r");
        strcat(send, buf2);
      }
    }
  }
  strcat(send, "\n\r");
  writeToQ(send);
  writeToQ("[Press return to continue]\n\r");
}

static const string getWizDescriptLev(const TBeing *ch)
{
  if (ch->hasWizPower(POWER_WIZARD))
    return "creator";
  else if (ch->hasWizPower(POWER_GOD))
    return "  god  ";
  else if (ch->hasWizPower(POWER_BUILDER))
    return "demigod";
  else
    return "BUG ME!";
}

static const string getWhoLevel(const TBeing *ch, TBeing *p)
{
  char tempbuf[256];
  char colorBuf[256] = "\0";

  if (p->hasWizPower(POWER_WIZARD))
    strcpy(colorBuf, ch->purple());
  else if (p->hasWizPower(POWER_GOD))
    strcpy(colorBuf, ch->red());
  else if (p->hasWizPower(POWER_BUILDER))
    strcpy(colorBuf, ch->cyan());

  if (p->msgVariables == MSG_IMM_TITLE &&
      !p->msgVariables(MSG_IMM_TITLE, (TThing *)NULL).empty())
    sprintf(tempbuf, "%sLevel:[%-14s%s][%s] %s",
            colorBuf, p->msgVariables(MSG_IMM_TITLE, (TThing *)NULL).c_str(),
            colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Brutius"))
    sprintf(tempbuf, "%sLevel:[ Grand Poobah ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Marsh"))
    sprintf(tempbuf, "%sLevel:[Grand Poobette][%s] %s",
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Batopr"))
    sprintf(tempbuf, "%sLevel:[  Mr. Fix-It  ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Cosmo"))
    sprintf(tempbuf, "%sLevel:[The Obfuscation Elminator][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Dolgan"))
    sprintf(tempbuf, "%sLevel:[Chief Surveyor][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Spawn"))
    sprintf(tempbuf, "%sLevel:[Shmack Talker ][%s] %s",
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Messiah"))
    sprintf(tempbuf, "%sLevel:[  The  Maker  ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Speef"))
    sprintf(tempbuf, "%sLevel:[ The Idea Man ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
#if 0
  else if (!strcmp(p->getName(), "Gringar"))
    sprintf(tempbuf, "%sLevel:[Faction Lackey][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
#endif
  else if (!strcmp(p->getName(), "Peel"))
    sprintf(tempbuf, "%sLevel:[The Freshmaker][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Mithros"))
    sprintf(tempbuf, "%sLevel:[Head Architect][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Lapsos"))
    sprintf(tempbuf, "%sLevel:[Jack of Trades][%s] %s",
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Omen"))
    sprintf(tempbuf, "%sLevel:[   Bad Omen   ][%s] %s",
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Sidartha"))
    sprintf(tempbuf, "%sLevel:[Lord of Worlds][%s] %s",
            colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (!strcmp(p->getName(), "Damescena"))
    sprintf(tempbuf, "%sLevel:[ All Business ][%s] %s",
            colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  //  else if (!strcmp(p->getName(), "Phenohol"))
  // sprintf(tempbuf, "%sLevel:[ He's Special ][%s] %s", 
  //        colorBuf, getWizDescriptLev(p).c_str(), ch->norm());

 
// generic cases
  else if (p->GetMaxLevel() == GOD_LEVEL10)
    sprintf(tempbuf, "%sLevel:[ Implementor  ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (p->GetMaxLevel() == GOD_LEVEL9)
    sprintf(tempbuf, "%sLevel:[ Grand Wizard ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (p->GetMaxLevel() == GOD_LEVEL8)
    sprintf(tempbuf, "%sLevel:[ Senior Lord  ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (p->GetMaxLevel() == GOD_LEVEL7)
    sprintf(tempbuf, "%sLevel:[ Junior Lord  ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (p->GetMaxLevel() == GOD_LEVEL6)
    sprintf(tempbuf, "%sLevel:[    Eternal   ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (p->GetMaxLevel() == GOD_LEVEL5) {
    if (p->getSex() == SEX_FEMALE)
      sprintf(tempbuf, "%sLevel:[   Goddess    ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
    else
      sprintf(tempbuf, "%sLevel:[     God      ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  } else if (p->GetMaxLevel() == GOD_LEVEL4) {
    if (p->getSex() == SEX_FEMALE)
      sprintf(tempbuf, "%sLevel:[ Demi-Goddess ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
    else
      sprintf(tempbuf, "%sLevel:[   Demi-God   ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  } else if (p->GetMaxLevel() == GOD_LEVEL3)
    sprintf(tempbuf, "%sLevel:[     Saint    ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  else if (p->GetMaxLevel() == GOD_LEVEL2) {
    if (p->getSex() == SEX_FEMALE)
      sprintf(tempbuf, "%sLevel:[    Heroine   ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
    else
      sprintf(tempbuf, "%sLevel:[     Hero     ][%s] %s", 
           colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  } else if (p->GetMaxLevel() == GOD_LEVEL1)
    sprintf(tempbuf, "%sLevel:[Area Designer ][%s] %s",
            colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
#if 1
  else {
    string tmpstring;
    if(p->isPlayerAction(PLR_ANONYMOUS) && !ch->isImmortal()){
      tmpstring = "Anonymous";
    } else {
      tmpstring = p->getProfAbbrevName();
      tmpstring += " Lev ";
      sprintf(tempbuf, "%d", p->GetMaxLevel());
      tmpstring += tempbuf;
    }

    while (tmpstring.length() < 13)
      tmpstring = " " + tmpstring + " ";
    if (tmpstring.length() < 14)
      tmpstring += " ";

    sprintf(tempbuf, "Level:[%s] ", tmpstring.c_str());
  }
#else
  else if (ch->isImmortal()) {
    string tmpstring = p->getProfAbbrevName();
    tmpstring += " Lev ";
    sprintf(tempbuf, "%d", p->GetMaxLevel());
    tmpstring += tempbuf;
    while (tmpstring.length() < 13)
      tmpstring = " " + tmpstring + " ";
    if (tmpstring.length() < 14)
      tmpstring += " ";

    sprintf(tempbuf, "Level:[%s] ", tmpstring.c_str());
  }
  else if (p->desc && p->desc->original)
    sprintf(tempbuf, "Level:[      %-2d      ] ", p->desc->original->GetMaxLevel());
  else      // a non immortal 
    sprintf(tempbuf, "Level:[      %-2d      ] ", p->GetMaxLevel());
#endif

  return tempbuf;
}

void TBeing::doWho(const char *argument)
{
#if 0
  // New Who Code to handle: who -ol j 20 40

  if (gamePort == BETA_GAMEPORT) {
    // 27 Who Flags upon (10-1-99):
    //  i=Idle          l=Levels        q=Quests
    //  h=Hit/Mana/Move z=Seeks-Group   p=Grouped
    //  d=Linkdead      g=Gods/Creators b=Builders/Gods/Creators
    //  o=Mortals       s=Stats         f=Faction
    //  1=Mage          2=Cleric        3=Warrior
    //  4=Thief         5=Deikhan       6=Monk
    //  7=Ranger        8=Shaman        e=Elf
    //  t=Hobbit        n=Gnome         u=Human
    //  r=Ogre          w=Dwarven       y=Not-Grouped
    const  char *tPerfMatches = "ilqhzpydgbosf12345678etnurw";
    char   tString[256],
           tBuffer[256]  = "\0",
           tOutput[1024] = "\0";
    int    tLow   =  0,
           tHigh  = 60,
           tCount =  0,
           tLinkD =  0;
    string tSb("");
    unsigned long int tBits = 0;
    TBeing *tBeing,
           *tFollower;

    while ((argument = one_argument(argument, tString))) {
      if (tString[0] == '-') {
        for (const char *tMarker = (tString + 1); *tMarker; tMarker++)
          if (strchr(tPerfMatches, *tMarker))
            tBits |= (1 << (strchr(tPerfMatches, *tMarker) - tPerfMatches));
          else {
            if (*tMarker != '?') {
              sprintf(tBuffer, "Unknown Option '%c':\n\r", *tMarker);
              tSb += tBuffer;
            }

            if (isImmortal()) {
              tSb += "[-] [i]idle [l]levels [q]quests [h]hit/mana/move\n\r";
              tSb += "[-] [z]seeks-group [p]groups [y]currently-not-grouped\n\r";
              tSb += "[-] [d]linkdead [g]God [b]Builders [o]Mort [s]stats [f]action\n\r";
              tSb += "[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Deikhan[6]Monk[7]Ranger[8]Shaman\n\r";
              tSb += "[-] [e]elf [t]hobbit [n]gnome [u]human [r]ogre [w]dwarven\n\r\n\r";
            } else {
              tSb += "[-] [q]quests [g]god [b]builder [o]mort [f]faction\n\r";
              tSb += "[-] [z]seeks-group [p]groups [y]currently-not-grouped\n\r";
              tSb += "[-] [e]elf [t]hobbit [n]gnome [u]human [r]ogre [w]dwarven\n\r\n\r";
#if 1
              tSb += "[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Deikhan[6]Monk[7]Ranger[8]Shaman\n\r";
#endif
            }

            if (desc)
              desc->page_string(tSb.c_str(), 0, TRUE);

            return;
          }
      } else if (is_number(tBuffer)) {
        if (!tLow)
          tLow = atoi(tBuffer);
        else
          tHigh = atoi(tBuffer);

	if (tLow <= 0 || tHigh > 60) {
          sendTo("Level numbers must be between 1 and 60.\n\r");
          return;
        }

        if (tHigh < tLow) {
          tCount = tLow;
          tLow   = tHigh;
          tHigh  = tCount;
        }
      } else
        strcpy(tBuffer, tString);
    }

    tSb += "Players: (Use who -? for online help)\n\r----------\n\r";
    tCount = tLinkD = 0;

    for (tBeing = character_list; tBeing; tBeing = tBeing->next)
      if (tBeing->isPc() && (tBeing->polyed == POLY_TYPE_NONE || isImmortal()) &&
          dynamic_cast<TPerson *>(tBeing) && canSeeWho(tBeing)) {
        if (tBeing->isLinkdead())
          tLinkD++;
        else
          tCount++;

        bool anonCheck == (isImmortal() || !tBeing->isPlayerAction(PLR_ANONYMOUS));

        if ((!(tBits & (1 <<  2)) || tBeing->inQuest()) &&
            (!(tBits & (1 <<  4)) || tBeing->isPlayerAction(PLR_SEEKSGROUP)) &&
            (!(tBits & (1 <<  5)) || (tBeing->isAffected(AFF_GROUP) && !tBeing->master && tBeing->followers)) &&
            (!(tBits & (1 <<  6)) || (!tBeing->isAffected(AFF_GROUP) && !tBeing->isImmortal())) &&
            (!(tBits & (1 <<  7)) || (tBeing->isLinkdead() && isImmortal())) && 
            (!(tBits & (1 <<  8)) || tBeing->hasWizPower(POWER_GOD)) &&
            (!(tBits & (1 <<  9)) || tBeing->hasWizPower(POWER_BUILDER)) &&
            (!(tBits & (1 << 10)) || !tBeing->hasWizPower(POEWR_BUILDER)) &&
            (!(tBits & (1 << 13)) || (anonCheck && tBeing->hasClass(CLASS_MAGE))) &&
            (!(tBits & (1 << 14)) || (anonCheck && tBeing->hasClass(CLASS_CLERIC))) &&
            (!(tBits & (1 << 15)) || (anonCheck && tBeing->hasClass(CLASS_WARRIOR))) &&
            (!(tBits & (1 << 16)) || (anonCheck && tBeing->hasClass(CLASS_THIEF))) &&
            (!(tBits & (1 << 17)) || (anonCheck && tBeing->hasClass(CLASS_DEIKHAN))) &&
            (!(tBits & (1 << 18)) || (anonCheck && tBeing->hasClass(CLASS_MONK))) &&
            (!(tBits & (1 << 19)) || (anonCheck && tBeing->hasClass(CLASS_RANGER))) &&
            (!(tBits & (1 << 20)) || (anonCheck && tBeing->hasClass(CLASS_SHAMAN))) &&
            (!(tBits & (1 << 21)) || tBeing->getRace() == RACE_ELVEN) &&
            (!(tBits & (1 << 22)) || tBeing->getRace() == RACE_GNOME) &&
            (!(tBits & (1 << 23)) || tBeing->getRace() == RACE_HUMAN) &&
            (!(tBits & (1 << 24)) || tBeing->getRace() == RACE_DWARF) &&
            (!(tBits & (1 << 25)) || tBeing->getRace() == RACE_OGRE) &&
            (!(tBits & (1 << 26)) || tBeing->getRace() == RACE_HOBBIT) &&
            (!*tBuffer || is_abbrev(tBuffer, tBeing->getName())) &&
            in_range(tBeing->GetMaxLevel(), tLow, tHigh)) {
          tOutput[0] = '\0';

          if ((tBits & (1 << 0)) && isImmortal()) {
            sprintf(tString, "Idle:[%-3d] ", tBeing->getTimer());
            strcat(tOutput, tString);
          }

          if ((tBits & (1 << 1)))
            strcat(tOutput, getWhoLevel(this, tBeing).c_str());


	  // Name goes here.

          if ((tBits & (1 << 12))) {
            if ((isImmortal() || getFaction() == tBeing->getFaction()) && !tBeing->isImmortal())
#if FACTIONS_IN_USE
              sprintf(tString, " [%s] %5.2f%%",
                      FactionInfo[tBeing->getFaction()].faction_name,
                      tBeing->getPerc());
#else
              sprintf(tString, " [%s]",
                      FactionInfo[tBeing->getFaction()].faction_name);
#endif

            strcat(tOutput, tString);
          }

          if (isImmortal()) {
            if (tBeing->polyed == POLY_TYPE_SWITCH)
              strcat(tOutput, " (switched)")
          }

          if ((tBits & (1 << 3)) && isImmortal()) {
            if (tBeing->hasClass(CLASS_CLERIC) || tBeing->hasClass(CLASS_DEIKHAN))
              sprintf(tString, "\n\r\tHit:[%-3d] Pty:[%-5.2f] Move:[%-3d] Talens:[%-8d] Bank:[%-8d]",
                      tBeing->getHit(), tBeing->getPiety(), tBeing->getMove(), tBeing->getMoney(), tBeing->getBank());
            else
              sprintf(tString, "\n\r\tHit:[%-3d] Mna:[%-3f] Move:[%-3d] Talens:[%-8d] Bank:[%-8d]",
                      tBeing->getHit(), tBeing->getMana(), tBeing->getMove(), tBeing->getMoney(), tBeing->getBank());

            strcat(tOutput, tString);
          }

          if ((tBits & (1 << 11)) && isImmortal()) {
            sprintf(tString, "\n\r\t[St:%-3d Br:%-3d Co:%-3d De:%-3d Ag:%-3d In:%-3d Wi:%-3d Fo:%-3d Pe:%-3d Ch:%-3d Ka:%-3d Sp:%-3d]",
                    tBeing->curStats.get(STAT_STR),
                    tBeing->curStats.get(STAT_BRA),
                    tBeing->curStats.get(STAT_CON),
                    tBeing->curStats.get(STAT_DEX),
                    tBeing->curStats.get(STAT_AGI),
                    tBeing->curStats.get(STAT_INT),
                    tBeing->curStats.get(STAT_WIS),
                    tBeing->curStats.get(STAT_FOC),
                    tBeing->curStats.get(STAT_PER),
                    tBeing->curStats.get(STAT_CHA),
                    tBeing->curStats.get(STAT_KAR),
                    tBeing->curStats.get(STAT_SPE));

            strcat(tOutput, tString);
          }
        }
      }
    /*
    char   tString[256],
           tBuffer[256]  = "\0",
           tOutput[1024] = "\0";
    string tSb("");
    TBeing *tBeing,
           *tFollower;

[-] [i]idle [l]levels [q]quests [h]hit/mana/move
[-] [z]seeks-group [p]groups [y]currently-not-grouped
[-] [d]linkdead [g]God [b]Builders [o]Mort [s]stats [f]action
[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Deikhan[6]Monk[7]Ranger[8]Shaman
[-] [e]elf [t]hobbit [n]gnome [u]human [r]ogre [w]dwarven
     */

    return;
  }
#endif

  TBeing *k, *p;
  char buf[1024] = "\0\0\0";
  int listed = 0, lcount, l;
  unsigned int count;
  char arg[1024], tempbuf[1024];
  string sb;
  int which1 = 0;
  int which2 = 0;

  for (; isspace(*argument); argument++);

  sb += "Players: (Add -? for online help)\n\r--------\n\r";
  lcount = count = 0;

  if (!*argument || 
       ((sscanf(argument, "%d %d", &which1, &which2) == 2) && 
          which1 > 0 && which2 > 0) ||
       ((sscanf(argument, "%d %d", &which1, &which2) == 1) && 
          which1 > 0  && (which2 = MAX_IMMORT))) {
    // plain old 'who' command 
    // who <level>      level2 assigned to 60
    // who <level> <level2>
    for (p = character_list; p; p = p->next) {
      if (p->isPc() && p->polyed == POLY_TYPE_NONE) {
        if (dynamic_cast<TPerson *>(p)) {
          if (canSeeWho(p) && (!*argument || ((!p->isPlayerAction(PLR_ANONYMOUS) || isImmortal()) && p->GetMaxLevel() >= which1 && p->GetMaxLevel() <= which2))){
            count++;

            p->parseTitle(buf, desc);
            if (!*argument) {
              if (p->isPlayerAction(PLR_SEEKSGROUP))
                sprintf(buf + strlen(buf), "   (Seeking Group)");

              if (p->isPlayerAction(PLR_NEWBIEHELP))
                sprintf(buf + strlen(buf), "   (Newbie-Helper)");

              strcat(buf, "\n\r");
            } else {
              sprintf(buf + strlen(buf), "   %s", getWhoLevel(this, p).c_str());
              if (p->isPlayerAction(PLR_SEEKSGROUP))
                sprintf(buf + strlen(buf), "   (Seeking Group)");

              if (p->isPlayerAction(PLR_NEWBIEHELP))
                sprintf(buf + strlen(buf), "   (Newbie-Helper)");

              sprintf(buf + strlen(buf), "\n\r");
            }
            char tmp[256];
            if (isImmortal() && p->isLinkdead()) {
              sprintf(tmp, "** %s", buf);
            } else {
              sprintf(tmp, "%s%s",
                  (p->polyed == POLY_TYPE_SWITCH ?  "(switched) " : ""), buf);
              sb += tmp;
            }
            //            sb += tmp;
          }
        } else if (isImmortal()) {
// only immortals will see this to provide them some concealment
          if (canSeeWho(p) && 
              (!*argument || 
                (p->GetMaxLevel() >= which1 && p->GetMaxLevel() <= which2)) &&
              IS_SET(p->specials.act, ACT_POLYSELF)) {
            count++;
            strcpy(tempbuf, pers(p));
            sprintf(buf, "%s (polymorphed magic user)\n\r", cap(tempbuf));
            sb += buf;
          } else if (canSeeWho(p) &&
                (!*argument || 
                (p->GetMaxLevel() >= which1 && p->GetMaxLevel() <= which2)) &&
                     IS_SET(p->specials.act, ACT_DISGUISED)) {
            count++;
            strcpy(tempbuf, pers(p));
            sprintf(buf, "%s (disguised thief)\n\r", cap(tempbuf));
            sb += buf;
          }
        }
      }
    }
    max_player_since_reboot = max(max_player_since_reboot, count);
    sprintf(buf, "\n\rTotal Players : [%d] Max since last reboot : [%d] Avg Players : [%.1f]\n\r", count, max_player_since_reboot, stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
    sb += buf;
    if (desc)
      desc->page_string(sb.c_str(), 0, TRUE);

    return;
  } else {
    argument = one_argument(argument, arg);
    if (*arg == '-') {
      if (strchr(arg, '?')) {
        if (isImmortal()) {
          sb += "[-] [i]idle [l]levels [q]quests [h]hit/mana/move\n\r";
          sb += "[-] [z]seeks-group [p]groups [y]currently-not-grouped\n\r";
          sb += "[-] [d]linkdead [g]God [b]Builders [o]Mort [s]stats [f]action\n\r";
          sb += "[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Deikhan[6]Monk[7]Ranger[8]Shaman\n\r";
          sb += "[-] [e]elf [t]hobbit [n]gnome [u]human [r]ogre [w]dwarven\n\r\n\r";
        } else {
          sb += "[-] [q]quests [g]god [b]builder [o]mort [f]faction\n\r";
          sb += "[-] [z]seeks-group [p]groups [y]currently-not-grouped\n\r";
          sb += "[-] [e]elf [t]hobbit [n]gnome [u]human [r]ogre [w]dwarven\n\r\n\r";
#if 1
          sb += "[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Deikhan[6]Monk[7]Ranger[8]Shaman\n\r";
#endif
        }
        if (desc)
          desc->page_string(sb.c_str(), 0, TRUE);
        return;
      }
      bool level, statsx, iPoints, quest, idle, align, group;
      for (p = character_list; p; p = p->next) {
        align = level = statsx = idle = iPoints = quest = group = FALSE;
        if (dynamic_cast<TPerson *>(p) && canSeeWho(p)) {
          count++;
          if (p->isLinkdead())
            lcount++;

          if ((canSeeWho(p) &&
              (!strchr(arg, 'g') || (p->GetMaxLevel() >= GOD_LEVEL1)) &&
              (!strchr(arg, 'b') || (p->GetMaxLevel() >= GOD_LEVEL1)) &&
              (!strchr(arg, 'q') || (p->inQuest())) &&
              (!strchr(arg, 'o') || (p->GetMaxLevel() <= MAX_MORT)) &&
              (!strchr(arg, 'z') || (p->isPlayerAction(PLR_SEEKSGROUP))) &&
              (!strchr(arg, 'p') || (p->isAffected(AFF_GROUP) && !p->master && p->followers)) &&
              (!strchr(arg, 'y') || (!p->isAffected(AFF_GROUP) && !p->isImmortal())) &&
              (!strchr(arg, '1') || (p->hasClass(CLASS_MAGIC_USER) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '2') || (p->hasClass(CLASS_CLERIC) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '3') || (p->hasClass(CLASS_WARRIOR) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '4') || (p->hasClass(CLASS_THIEF) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '5') || (p->hasClass(CLASS_DEIKHAN) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '6') || (p->hasClass(CLASS_MONK) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '7') || (p->hasClass(CLASS_RANGER) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '8') || (p->hasClass(CLASS_SHAMAN) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, 'd') || (p->isLinkdead() && isImmortal())) &&
              (!strchr(arg, 'e') || p->getRace() == RACE_ELVEN) &&
              (!strchr(arg, 'n') || p->getRace() == RACE_GNOME) &&
              (!strchr(arg, 'u') || p->getRace() == RACE_HUMAN) &&
              (!strchr(arg, 'w') || p->getRace() == RACE_DWARF) &&
              (!strchr(arg, 'r') || p->getRace() == RACE_OGRE) &&
              (!strchr(arg, 't') || p->getRace() == RACE_HOBBIT))) {
            if (p->isLinkdead() && isImmortal())
              sprintf(buf, "[%-12s] ", pers(p));
            else if (p->polyed == POLY_TYPE_SWITCH && isImmortal())
              sprintf(buf, "[%-12s] (switched) ", pers(p));
            else if (dynamic_cast<TMonster *>(p) &&
                     (p->specials.act & ACT_POLYSELF))
              sprintf(buf, "(%-14s) ", pers(p));
            else 
              sprintf(buf, "%-11s ", pers(p));
            listed++;
            for (l = 1; l <= (int) strlen(arg); l++) {
              switch (arg[l]) {
                case 'p':
                  // we trapped only group leaders above...
                  if (!group) {
                    TBeing *ch;
                    followData *f;
                    for (f = p->followers; f; f = f->next) {
                      ch = f->follower;
                      if (!ch->isPc())
                        continue;
                      if (!canSeeWho(ch))
                        continue;
                      if (ch->isLinkdead() && isImmortal())
                        sprintf(buf, "[%-12s] ", pers(ch));
                      else if (ch->polyed == POLY_TYPE_SWITCH && isImmortal())
                        sprintf(buf, "[%-12s] (switched) ", pers(ch));
                      else if (dynamic_cast<TMonster *>(ch) &&
                               (ch->specials.act & ACT_POLYSELF))
                        sprintf(buf + strlen(buf), "(%-14s) ", pers(ch));
                      else if (ch->isPlayerAction(PLR_ANONYMOUS) && !isImmortal())
                        sprintf(buf + strlen(buf), "%-11s (???) ", pers(ch));
                      else
                        sprintf(buf + strlen(buf), "%-11s (L%d) ", pers(ch), ch->GetMaxLevel());
                    }

                    group = true;
                  }
                  break;
                case 'i':
                  if (!idle) {
                    if (isImmortal())
                      sprintf(buf + strlen(buf), "Idle:[%-3d] ", p->getTimer());
                  }
                  idle = TRUE;
                  break;
                case 'l':
                case 'y':
                  if (!level) {
                    strcat(buf, getWhoLevel(this, p).c_str());
                    if (p->isPlayerAction(PLR_SEEKSGROUP))
                      sprintf(buf + strlen(buf), "   (Seeking Group)");

                    if (p->isPlayerAction(PLR_NEWBIEHELP))
                      sprintf(buf + strlen(buf), "   (Newbie-Helper)");
                  }
                  level = TRUE;
                  break;
                case 'g':
                case 'b':
                  // canSeeWho already separated out invisLevel > my own
                  // only a god can go invis, mortals technically have
                  // invisLevel if they are linkdead, ignore that though
                  if (p->getInvisLevel() > MAX_MORT)
                    sprintf(buf + strlen(buf), "  (invis %d)  ",
                        p->getInvisLevel());
                  break;
                case 'h':
                  if (!iPoints) {
                    if (isImmortal())
                      if (p->hasClass(CLASS_CLERIC)||p->hasClass(CLASS_DEIKHAN))
                        sprintf(buf + strlen(buf), "Hit:[%-3d] Pty:[%-.2f] Move:[%-3d], Talens:[%-8d], Bank:[%-8d]",
                              p->getHit(), p->getPiety(), p->getMove(), p->getMoney(), p->getBank());
                      else
                          sprintf(buf + strlen(buf), "Hit:[%-3d] Mana:[%-3d] Move:[%-3d], Talens:[%-8d], Bank:[%-8d]",
                              p->getHit(), p->getMana(), p->getMove(), p->getMoney(), p->getBank());
                  }
                  iPoints = TRUE;
                  break;
                case 'f':
                  if (!align) {
                    // show factions of everyone to immorts
                    // mortal version will show non-imms that are in same fact
                    if ((getFaction()==p->getFaction() &&
                         p->GetMaxLevel() <= MAX_MORT) || isImmortal())
#if FACTIONS_IN_USE
                      sprintf(buf + strlen(buf), "[%s] %5.2f%%", 
                        FactionInfo[p->getFaction()].faction_name,
                        p->getPerc());
#else
                      sprintf(buf + strlen(buf), "[%s]", 
                        FactionInfo[p->getFaction()].faction_name);
#endif
                  }
                  align = TRUE;
                  break;
                case 's':
                  if (!statsx) {
                    if (isImmortal())
                      sprintf(buf + strlen(buf), "\n\r\t[St:%-3d Br:%-3d Co:%-3d De:%-3d Ag:%-3d In:%-3d Wi:%-3d Fo:%-3d Pe:%-3d Ch:%-3d Ka:%-3d Sp:%-3d]",
                        p->curStats.get(STAT_STR),
                        p->curStats.get(STAT_BRA),
                        p->curStats.get(STAT_CON),
                        p->curStats.get(STAT_DEX),
                        p->curStats.get(STAT_AGI),
                        p->curStats.get(STAT_INT),
        		p->curStats.get(STAT_WIS),
			p->curStats.get(STAT_FOC),
			p->curStats.get(STAT_PER),
			p->curStats.get(STAT_CHA),
			p->curStats.get(STAT_KAR),
			p->curStats.get(STAT_SPE));
                  }
                  statsx = TRUE;
                  break;
                case 'q':
                  if (!quest) {
                    if (p->isPlayerAction(PLR_SOLOQUEST))
                      sprintf(buf + strlen(buf), " (%sSOLO QUEST%s)", red(), norm());
                    
                    if (p->isPlayerAction(PLR_GRPQUEST))
                      sprintf(buf + strlen(buf), " (%sGROUP QUEST%s)", blue(), norm());
                  }
                  quest = TRUE;
                  break;
                default:
                  break;
              }        // end of switch statement 
            }        // end of for-loop 
            strcat(buf, "\n\r");
            sb += buf;
          }        // end of 'should I skip this fool' if-statement 
        }        // end of !NPC(p) loop 
      }                // end of 'step through the character list loop 
    } else {
      // 'who playername' command 
      int c = 0;
      for (k = character_list; k; k = k->next) {
        if (!k->isPc() || !isname(arg, k->name) || !canSee(k)) 
          continue;
 
        c++;
        *buf = '\0';
        k->parseTitle(buf, desc);
        strcat(buf, "    ");
        strcat(buf, getWhoLevel(this, k).c_str());
        if (k->isPlayerAction(PLR_SEEKSGROUP))
          sprintf(buf + strlen(buf), "   (Seeking Group)");

        if (k->isLinkdead() && isImmortal())
          strcat(buf, "   (link-dead)");

        if (k->polyed == POLY_TYPE_SWITCH && isImmortal())
          strcat(buf, "   (switched)");

        if (k->isPlayerAction(PLR_NEWBIEHELP))
          sprintf(buf + strlen(buf), "   (Newbie-Helper)");

        strcat(buf, "\n\r");
        sb += buf;
      }
      if (!c)
        sb += "No one logged in with that name.\n\r";

      if (desc)
        desc->page_string(sb.c_str(), 0, TRUE);
      return;
    }
  }
  max_player_since_reboot = max(max_player_since_reboot, count);
  if (isImmortal()) {
    if (!listed)
      sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%)\n\rMax since Reboot [%d]  Avg Players : [%.1f]\n\r",
           count, lcount, ((double) lcount / (int) count) * 100, 
           max_player_since_reboot,
           stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
    else
      sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%)\n\rNumber Listed: %d  Max since Reboot [%d]  Avg Players : [%.1f]\n\r", 
           count, lcount, ((double) lcount / (int) count) * 100, listed,
           max_player_since_reboot,
           stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
  } else {
    sprintf(buf, "\n\rTotal Players : [%d] Max since last reboot : [%d] Avg Players : [%.1f]\n\r", count, max_player_since_reboot, stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
  }
  sb += buf;
  if (desc)
    desc->page_string(sb.c_str(), 0, TRUE);
  return;
}

static const string describe_part_wounds(const TBeing *ch, wearSlotT pos)
{
  int i, flags;
  int last = 0, count = 0;
  char buf[256];

  if (ch->isLimbFlags(pos, PART_MISSING))
    return ("missing.");

  *buf = '\0';

  for (i = 0; i < MAX_PARTS; i++) {
    flags = (1 << i);
    if (ch->isLimbFlags(pos, flags)) {
      last = i;
      count++;
    }
  }
  if (count == 1 && (ch->isLimbFlags(pos, PART_TRANSFORMED)))
    return "";
  for (i = 0; i < MAX_PARTS; i++) {
    flags = (1 << i);
    if (ch->isLimbFlags(pos, flags)) {
      if (count == 1)
        sprintf(buf + strlen(buf), "%s%s%s.", ch->red(), body_flags[i], ch->norm());
      else if (i != last)
        sprintf(buf + strlen(buf), "%s%s%s, ", ch->red(), body_flags[i], ch->norm());
      else
        sprintf(buf + strlen(buf), "and %s%s%s.", ch->red(), body_flags[i], ch->norm());
    }
  }
  if (*buf)
    return (buf);
  else
    return ("");
}

// rp is the room looking at
// can't use roomp since spying into other room possible
void TBeing::listExits(const TRoom *rp) const
{
  int num = 0, count = 0;
  dirTypeT door;
  roomDirData *exitdata;
  char buf[1024];

  const char *exDirs[] =
  {
    "N", "E", "S", "W", "U",
    "D", "NE", "NW", "SE", "SW"
  };

  *buf = '\0';

  if (desc && desc->client) 
    return;
  
  // Red if closed (imm only), Blue if an open exit has a type, purple if normal

  if (isPlayerAction(PLR_BRIEF)) {
    sendTo("[Exits: ");
    for (door = MIN_DIR; door < MAX_DIR; door++) {
      exitdata = rp->exitDir(door);
      if (exitdata && (exitdata->to_room != ROOM_NOWHERE)) {
        if (isImmortal()) {
          if (IS_SET(exitdata->condition, EX_CLOSED)) 
            sendTo(" %s%s%s", red(), exDirs[door], norm());
          else if (exitdata->door_type != DOOR_NONE) 
            sendTo(" %s%s%s", blue(), exDirs[door], norm());
          else 
            sendTo(" %s%s%s", purple(), exDirs[door], norm());
        } else if (canSeeThruDoor(exitdata)) {
          if (exitdata->door_type != DOOR_NONE && !IS_SET(exitdata->condition, EX_CLOSED)) 
            sendTo(" %s%s%s", blue(), exDirs[door], norm());
          else 
            sendTo(" %s%s%s", purple(), exDirs[door], norm());
        }
      }
    }
    sendTo("]\n\r");
    return;
  }

  // The following for loop is to figure out which room is the last
  // legal exit, so the word "and" can be put in front of it to make
  // the sentence sent to the player grammatically correct.        

  for (door = MIN_DIR; door < MAX_DIR; door++) {
    exitdata = rp->exitDir(door);
    if (exitdata) {
      if (exitdata->to_room != ROOM_NOWHERE &&
          (canSeeThruDoor(exitdata) ||
           isImmortal())) {
        num = door;
        count++;
      }
      if (IS_SET(exitdata->condition, EX_DESTROYED)) {
        if (!exitdata->keyword) {
          vlogf(LOW_ERROR,"Destroyed door with no name!  Room %d", in_room);
        } else if (door == 4) 
          sendTo("%sThe %s in the ceiling has been destroyed.%s\n\r",
              blue(), fname(exitdata->keyword).c_str(), norm());
        else if (door == 5)
          sendTo("%sThe %s in the %s has been destroyed.%s\n\r",
              blue(), fname(exitdata->keyword).c_str(), roomp->describeGround().c_str(), norm());
        else
          sendTo("%sThe %s %s has been destroyed.%s\n\r",
              blue(), fname(exitdata->keyword).c_str(), dirs_to_leading[door], norm());
      }
      if (IS_SET(exitdata->condition, EX_CAVED_IN)) {
        sendTo("%sA cave in blocks the way %s.%s\n\r",
            blue(), dirs[door], norm());
      }
      // chance to detect secret - bat
      // the || case is a chance at a false-positive   :)
      if ((IS_SET(exitdata->condition, EX_SECRET) &&
          IS_SET(exitdata->condition, EX_CLOSED)) ||
          (!::number(0,100) && !isPerceptive())) {
        int chance = max(0, (int) getSkillValue(SKILL_SEARCH));

        if (getRace() == RACE_ELVEN)
          chance += 25;
        if (getRace() == RACE_GNOME)
          chance += plotStat(STAT_CURRENT, STAT_PER, 3, 18, 13) +
                    GetMaxLevel()/2;
        if (getRace() == RACE_DWARF && rp->isIndoorSector())
          chance += GetMaxLevel()/2 + 10;

        if ((::number(1,1000) < chance) && !isImmortal())
          sendTo("%sYou suspect something out of the ordinary here.%s\n\r",
              blue(), norm());
      }
    }
  }

  for (door = MIN_DIR; door < MAX_DIR; door++) {
    exitdata = rp->exitDir(door);
    if (exitdata && (exitdata->to_room != ROOM_NOWHERE)) {
      if (isImmortal()) {
// Red if closed, Blue if an open exit has a type, purple if normal
        if (IS_SET(exitdata->condition, EX_CLOSED)) {
          if (count == 1)
            sprintf(buf + strlen(buf), "%s%s%s.\n\r", red(), dirs[door], norm());
          else if (door != num)
            sprintf(buf + strlen(buf), "%s%s%s, ", red(), dirs[door], norm());
          else
            sprintf(buf + strlen(buf), "and %s%s%s.\n\r", red(), dirs[door],norm());
        } else if (exitdata->door_type != DOOR_NONE) {
          if (count == 1)
            sprintf(buf + strlen(buf), "%s%s%s.\n\r", blue(), dirs[door], norm());
          else if (door != num)
            sprintf(buf + strlen(buf), "%s%s%s, ", blue(), dirs[door], norm());
          else
            sprintf(buf + strlen(buf), "and %s%s%s.\n\r", blue(), dirs[door], norm());
        } else {  
          if (count == 1)
            sprintf(buf + strlen(buf), "%s%s%s.\n\r", purple(), dirs[door], norm());
          else if (door != num)
            sprintf(buf + strlen(buf), "%s%s%s, ", purple(), dirs[door], norm());
          else
            sprintf(buf + strlen(buf), "and %s%s%s.\n\r", purple(), dirs[door], norm());
        }
      } else {
        if ((canSeeThruDoor(exitdata))) {
          TRoom *exitp = real_roomp(exitdata->to_room);
          if (exitp) {
            if (exitdata->door_type != DOOR_NONE &&
                !IS_SET(exitdata->condition, EX_CLOSED)) {
              sprintf(buf + strlen(buf), "%s%s%s%s%s",
                ((count != 1 && door == num) ? "and " : ""),
                (exitp->getSectorType() == SECT_FIRE ? redBold() :
                (exitp->isAirSector() ? cyanBold() :
                (exitp->isWaterSector() ? blueBold() :
                purpleBold()))),
                dirs[door],
                norm(),
                (count == 1 || door == num ? ".\n\r" : ", "));
            } else {
              sprintf(buf + strlen(buf), "%s%s%s%s%s",
                ((count != 1 && door == num) ? "and " : ""),
                (exitp->getSectorType() == SECT_FIRE ? red() :
                (exitp->isAirSector() ? cyan() :
                (exitp->isWaterSector() ? blue() :
                purple()))),
                dirs[door],
                norm(),
                (count == 1 || door == num ? ".\n\r" : ", "));
            }
          } else
            vlogf(LOW_ERROR, "Problem with door in room %d", inRoom());
        }
      }
    }
  }

  if (*buf) {
    if (count == 1) 
      sendTo("You see an exit %s", buf);
    else 
      sendTo("You can see exits to the %s", buf);
  } else
    sendTo("You see no obvious exits.\n\r");
}


void list_char_in_room(TThing *list, TBeing *ch)
{
  TThing *i, *cond_ptr[50];
  int k, cond_top;
  unsigned int cond_tot[50];
  bool found = FALSE;

  cond_top = 0;

  for (i = list; i; i = i->nextThing) {
    if (dynamic_cast<TBeing *>(i) && (ch != i) && (!i->rider) &&
        (ch->isAffected(AFF_SENSE_LIFE) || ch->isAffected(AFF_INFRAVISION) || (ch->canSee(i)))) {
      if ((cond_top < 50) && !i->riding) {
        found = FALSE;
        if (dynamic_cast<TMonster *>(i)) {
          for (k = 0; (k < cond_top && !found); k++) {
            if (cond_top > 0) {
              if (i->isSimilar(cond_ptr[k])) {
                cond_tot[k] += 1;
                found = TRUE;
              }
            }
          }
        }
        if (!found) {
          cond_ptr[cond_top] = i;
          cond_tot[cond_top] = 1;
          cond_top += 1;
        }
      } else
        ch->showTo(i, SHOW_MODE_DESC_PLUS);
    }
  }
  if (cond_top) {
    for (k = 0; k < cond_top; k++) {
      if (cond_tot[k] > 1)
        ch->showMultTo(cond_ptr[k], SHOW_MODE_DESC_PLUS, cond_tot[k]);
      else
        ch->showTo(cond_ptr[k], SHOW_MODE_DESC_PLUS);
    }
  }
}


void list_char_to_char(TBeing *list, TBeing *ch, int)
{
  TThing *i;

  for (i = list; i; i = i->nextThing) {
    if ((ch != i) && (ch->isAffected(AFF_SENSE_LIFE) || (ch->canSee(i))))
      ch->showTo(i, SHOW_MODE_DESC_PLUS);
  }
}

void TBaseCup::lookObj(TBeing *ch, int) const
{
  int temp;

  if (getMaxDrinkUnits()/128) {
    ch->sendTo(COLOR_OBJECTS, "%s has a capacity of %d gallon%s, %d fluid ounce%s.\n\r",
          good_cap(ch->pers(this)).c_str(),
          getMaxDrinkUnits()/128,
          (getMaxDrinkUnits()/128 == 1 ? "" : "s"),
          getMaxDrinkUnits()%128,
          (getMaxDrinkUnits()%128 == 1 ? "" : "s"));
  } else {
    ch->sendTo(COLOR_OBJECTS, "%s has a capacity of %d fluid ounce%s.\n\r",
          good_cap(ch->pers(this)).c_str(),
          getMaxDrinkUnits()%128,
          (getMaxDrinkUnits()%128 == 1 ? "" : "s"));
  }
  if (getDrinkUnits() <= 0 || !getMaxDrinkUnits())
    act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
  else {
    temp = ((getDrinkUnits() * 3) / getMaxDrinkUnits());
    ch->sendTo(COLOR_OBJECTS, "It's %sfull of a %s liquid.\n\r",
          fullness[temp], DrinkInfo[getDrinkType()]->color);
  }
}

void TObj::lookObj(TBeing *ch, int bits) const
{
  ch->sendTo("That is not a container.\n\r");
}

void TRealContainer::lookObj(TBeing *ch, int bits) const
{
  if (isClosed()) {
    ch->sendTo("It is closed.\n\r");
    return;
  }

  ch->sendTo(fname(name).c_str());
  switch (bits) {
    case FIND_OBJ_INV:
      ch->sendTo(" (carried) : ");
      break;
    case FIND_OBJ_ROOM:
      ch->sendTo(" (here) : ");
      break;
    case FIND_OBJ_EQUIP:
      ch->sendTo(" (used) : ");
      break;
  }
  if (carryVolumeLimit() && carryWeightLimit()) {
    // moneypouches are occasionally overfilled, so we will just force the
    // info to look right...
    ch->sendTo("%d%% full, %d%% loaded.\n\r",
      min(100, getCarriedVolume() * 100 / carryVolumeLimit()),
      min(100, (int) (getCarriedWeight() * 100.0 / carryWeightLimit())));
  } else {
    vlogf(8, "Problem in look in for object: (%s:%d), check vol/weight limit", getName(), objVnum());
  }
  list_in_heap(stuff, ch, 0, 100);

  // list_in_heap uses sequential sendTo's, so lets string it to them for
  // easier browsing
  ch->makeOutputPaged();
}

void TThing::lookAtObj(TBeing *ch, const char *, showModeT x) const
{
  ch->showTo(this, x);        // Show no-description 
  ch->describeObject(this);
}

void TBeing::doLook(const char *argument, cmdTypeT cmd, TThing *specific)
{
  char buffer[256], *tmp_desc, *tmp;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  int keyword_no, res, j, found, totalFound = 0, iNum = 0;
  unsigned int bits = 0;
  TThing *t = NULL, *t2 = NULL;
  TObj *o = NULL;
  TObj *o2 = NULL;
  TBeing *tmp_char = NULL;
  roomDirData *exitp;
  TRoom *rp;

  static const char *keywords[] = {
    "north",    // 0
    "east",
    "south",
    "west",
    "up",
    "down",     // 5
    "in",
    "at",
    "",                                // Look at '' case 
    "room",
    "ne",      // 10
    "nw",
    "se",
    "sw",
    "\n"
  };

  if (!desc || !roomp)
    return;

  if (gGin.check(this)) {
    if (gGin.look(this, argument))
      return;
  }
  if (checkHearts()) {
    if (gHearts.look(this, argument))
      return;
  }
  if (checkCrazyEights()) {
    if (gEights.look(this, argument))
      return;
  }
  if (checkDrawPoker()) {
    if (gPoker.look(this, argument))
      return;
  }
  if (getPosition() < POSITION_SLEEPING)
    sendTo("You can't see anything but stars!\n\r");
  else if (getPosition() == POSITION_SLEEPING)
    sendTo("You can't see anything -- you're sleeping!\n\r");
  else if (isAffected(AFF_BLIND) && !isImmortal() && !isAffected(AFF_TRUE_SIGHT))
    sendTo("You can't see a damn thing -- you're blinded!\n\r");
  else if (roomp->pitchBlackDark() && !isImmortal() &&
           (visionBonus <= 0) &&
           !(roomp->getRoomFlags() & ROOM_ALWAYS_LIT) &&
           !isAffected(AFF_TRUE_SIGHT)) {
    sendTo("It is very dark in here...\n\r");

    // this already handles stuff like infravision, and glowing mobs
    list_char_in_room(roomp->stuff, this);

    for (t = roomp->stuff; t; t = t->nextThing) {
      if (dynamic_cast<TObj *>(t) && canSee(t))   // glowing objects
        showTo(t, SHOW_MODE_DESC_PLUS);
    }
  } else {
    // we use only_arg so we pick up "at" from "look at the window"
    only_argument(argument, arg1);

    // then we start using one_argument so we will munch "the" in above example
    if (!strncmp(arg1, "at", 2) && isspace(arg1[2])) {
      strcpy(arg2, argument + 3);
      keyword_no = 7;
    } else if (!strncmp(arg1, "in", 2) && isspace(arg1[2])) {
      strcpy(arg2, argument + 3);
      keyword_no = 6;
    } else if (!strncmp(arg1, "on", 2) && isspace(arg1[2])) {
      strcpy(arg2, argument + 3);
      keyword_no = 6;
    } else
      keyword_no = search_block(arg1, keywords, FALSE);

    if ((keyword_no == -1) && *arg1) {
      keyword_no = 7;
      strcpy(arg2, argument);
    }
    found = FALSE;
    o = NULL;
    tmp_char = NULL;
    tmp_desc = NULL;

    switch (keyword_no) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 10:   // diagonals
      case 11:
      case 12:
      case 13:
        if (keyword_no >= 10)  // adjust cause of our whacky array
          keyword_no -= 4;

        if (!(exitp = exitDir(dirTypeT(keyword_no)))) {
          if (roomp && roomp->ex_description &&
              (tmp_desc = roomp->ex_description->findExtraDesc(dirs[keyword_no])))
            sendTo(tmp_desc);
          else
            sendTo("You see nothing special.\n\r");

          return;
        } else {
          sendTo("You look %swards.\n\r", dirs[keyword_no]);
          sprintf(buffer, "$n looks %swards.", dirs[keyword_no]);
          act(buffer, TRUE, this, 0, 0, TO_ROOM);

          if (canSeeThruDoor(exitp)) {
            if (exitp->description)
              sendTo(COLOR_ROOMS, exitp->description);
            else {
              if (exitp->to_room && (rp = real_roomp(exitp->to_room))) {
                if (IS_SET(desc->plr_color, PLR_COLOR_ROOM_NAME)) {
                  if (hasColorStrings(NULL, rp->getName(), 2)) {
                    sendTo(COLOR_ROOM_NAME, "You see %s<1>.\n\r",
                           dynColorRoom(rp, 1, TRUE).c_str());
                  } else {
                    sendTo(COLOR_ROOM_NAME, "You see %s%s%s.\n\r",
                           addColorRoom(rp, 1).c_str(), rp->name ,norm());
                  }
                } else {
                  sendTo(COLOR_BASIC, "You see %s%s%s.\n\r", purple(), 
                       rp->getNameNOC(this).c_str(), norm());
                }
              } else {
                sendTo("You see nothing special.\n\r");
                vlogf(9, "Bad room exit in room %d", in_room);
              }
            }

            if (keyword_no != DIR_UP && keyword_no != DIR_DOWN)
              if ((exitp->condition & EX_SLOPED_UP))
                sendTo("The way seems sloped up in that direction.\n\r");
              else if ((exitp->condition & EX_SLOPED_DOWN))
                sendTo("The way seems sloped down in that direction.\n\r");

            if (isAffected(AFF_SCRYING) || isImmortal()) {
              if (!(rp = real_roomp(exitp->to_room)))
                sendTo("You see swirling chaos.\n\r");
              else {
                if (!isPlayerAction(PLR_BRIEF))
                  sendRoomDesc(rp);

                listExits(rp);
                list_thing_in_room(rp->stuff, this);
              }
            }
          } else if (!(exitp->condition & EX_SECRET))
            sendTo("The %s is closed.\n\r", exitp->getName().c_str());
          else
            sendTo("You see nothing special.\n\r");
        }
        break;
      case 6:
        if (*arg2 || specific) {
          if (specific) {
            if (!dynamic_cast<TObj *> (specific)) {
              sendTo("Look in what?!\n\r");
            } else {
              TObj *tmpO = dynamic_cast<TObj *> (specific);
              if (tmpO->parent && (this == parent)) {
                bits = FIND_OBJ_INV;
              } else if (tmpO->equippedBy && (this == tmpO->equippedBy)) {
                bits = FIND_OBJ_EQUIP;
              } else if (tmpO->parent && (roomp == tmpO->parent)) {
                bits = FIND_OBJ_ROOM;
              }
              if ((bits == FIND_OBJ_ROOM) && riding && tmpO->parent) {
                sendTo("You can't look into items on the %s while mounted!\n\r",roomp->describeGround().c_str());
                return;
              } else {
                tmpO->lookObj(this, bits);
              }
            }
            return;
          }
          bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, this, &tmp_char, &o);
          if (bits) {
            if ((bits == FIND_OBJ_ROOM) && riding && o->parent) {
              // we want to allow them to look at item's on a table, but not
              // in a bag while on a horse.
              sendTo("You can't look into items on the %s while mounted!\n\r", roomp->describeGround().c_str());
              return;
            }

            o->lookObj(this, bits);
            
          } else        // wrong argument 
            sendTo("You do not see that item here.\n\r");
        } else                // no argument 
          sendTo("Look in what?!\n\r");

        break;
      case 7:{
          if (*arg2 || specific) {
            if (cmd == CMD_READ) {
#if 1 
              const char *tempArg = NULL;
              char tempArg2[256];
              tempArg = arg2;
              tempArg = one_argument(tempArg, tempArg2);
              if (is_abbrev(tempArg2, "chapter") ||
		  is_abbrev(tempArg2, "section")) {
                char tempArg3[256];
                if (tempArg)
                  tempArg = one_argument(tempArg, tempArg3);
                if (*tempArg || !atoi(tempArg3)) {
                   bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_EQUIP, this, &tmp_char, &o2);
                } else if (atoi(tempArg3)) {
                  TObj * tempObj = NULL;
                  if ((tempObj = dynamic_cast<TBook *> (heldInPrimHand()))) {
                    o2 = tempObj;
                    bits = FIND_OBJ_EQUIP;
                  } else if ((tempObj = dynamic_cast<TBook *> (heldInSecHand()))) {
                    o2 = tempObj;
                    bits = FIND_OBJ_EQUIP;
                  }
                  if (!bits) {
                    TThing * tempThing = NULL;
                    for (tempThing = stuff; tempThing; tempThing = tempThing->nextThing) {
                      if (!dynamic_cast<TBook *> (tempThing)) {
                        continue;
                      } else {
                        o2 = dynamic_cast<TBook *> (tempThing);
                        if (o2) {
                          bits = FIND_OBJ_INV;
                          break;
                        }
                      }
                    }
                  }
                  if (!bits)
                    bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_EQUIP, this , &tmp_char, &o2);
                } else {
                  bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_EQUIP, this , &tmp_char, &o2);
                }
              } else {
                bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_EQUIP,
                        this, &tmp_char, &o2);
              }
#else
              bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_EQUIP,
                        this, &tmp_char, &o2);
#endif
            } else {
              bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
                        FIND_OBJ_EQUIP | FIND_CHAR_ROOM, this, &tmp_char, &o2);
            }
            if (dynamic_cast<TBeing *> (specific)) {
              TBeing *tmpBeing = dynamic_cast<TBeing *> (specific);
              showTo(tmpBeing, SHOW_MODE_SHORT_PLUS);
              if (this != tmpBeing && !affectedBySpell(SKILL_SPY) &&
                                      !tmpBeing->isImmortal()) {
                act("$n looks at you.", TRUE, this, 0, tmpBeing, TO_VICT);
                act("$n looks at $N.", TRUE, this, 0, tmpBeing, TO_NOTVICT);
                if (!tmpBeing->isPc())
                  dynamic_cast<TMonster *>(tmpBeing)->aiLook(this);
              } else if (tmpBeing != this && !tmpBeing->isImmortal()) {
                // Thieves in the room will be able to detect spying looks.
                TBeing *bOther;
                for (t = roomp->stuff; t; t = t->nextThing)
                  if ((bOther = dynamic_cast<TBeing *>(t)) &&
                      (bOther->affectedBySpell(SKILL_SPY) ||
                       bOther->isAffected(AFF_SCRYING)) &&
                      bOther->GetMaxLevel() >= GetMaxLevel() &&
                      bOther != this) {
                    sprintf(arg1, "You detect $n looking at %s with spying eyes.",
                            (bOther == tmpBeing ? "you" : tmpBeing->getName()));
                    act(arg1, TRUE, this, 0, bOther, TO_VICT);
                    if (bOther == tmpBeing && !tmpBeing->isPc())
                      dynamic_cast<TMonster *>(tmpBeing)->aiLook(this);
                  }
              }
              return;
            } else if (dynamic_cast<TObj *> (specific)) {
              TObj *tmpObj = dynamic_cast<TObj *> (specific);
              if (!canSee(tmpObj)) {
                sendTo("Look at what?\n\r");
                return;
              }
              if (tmpObj->ex_description) {
                if ((tmp_desc = tmpObj->ex_description->findExtraDesc(tmp))) {
                  desc->page_string(tmp_desc, 0);
                  found = TRUE;
                  describeObject(tmpObj);
                  if (tmpObj->riding)
                    sendTo(COLOR_OBJECTS, "%s is on %s.", tmpObj->getName(), tmpObj->riding->getName());
                  showTo(tmpObj, SHOW_MODE_PLUS);
                  return;
                } else {
                  tmpObj->lookAtObj(this, tmp, SHOW_MODE_TYPE);
                  if (tmpObj->riding)
                    sendTo(COLOR_OBJECTS, "%s is on %s.", tmpObj->getName(), tmpObj->riding->getName());
                  return;
                }
              } else {
                tmpObj->lookAtObj(this, tmp, SHOW_MODE_TYPE);
                return;
              }
            }
            if (tmp_char) {
              showTo(tmp_char, SHOW_MODE_SHORT_PLUS);
              if (this != tmp_char && !affectedBySpell(SKILL_SPY) &&
                                   !tmp_char->isImmortal()) {
                act("$n looks at you.", TRUE, this, 0, tmp_char, TO_VICT);
                act("$n looks at $N.", TRUE, this, 0, tmp_char, TO_NOTVICT);
                if (!tmp_char->isPc())
                  dynamic_cast<TMonster *>(tmp_char)->aiLook(this);
              } else if (tmp_char != this && !tmp_char->isImmortal()) {
                // Thieves in the room will be able to detect spying looks.
                TBeing *bOther;
                for (t = roomp->stuff; t; t = t->nextThing)
                  if ((bOther = dynamic_cast<TBeing *>(t)) &&
                      (bOther->affectedBySpell(SKILL_SPY) ||
                       bOther->isAffected(AFF_SCRYING)) &&
                      bOther->GetMaxLevel() >= GetMaxLevel() &&
                      bOther != this) {
                    sprintf(arg1, "You detect $n looking at %s with spying eyes.",
                            (bOther == tmp_char ? "you" : tmp_char->getName()));
                    act(arg1, TRUE, this, 0, bOther, TO_VICT);
                    if (bOther == tmp_char && !tmp_char->isPc())
                      dynamic_cast<TMonster *>(tmp_char)->aiLook(this);
                  }
              }
              return;
            }
            char tmpname[MAX_INPUT_LENGTH];
            strcpy(tmpname, arg2);
            tmp = tmpname;
            iNum = get_number(&tmp);

            if (!strcmp(tmp, "_tele_") && !isImmortal()) {
              sendTo("Look at what?\n\r");
              return;
            }
            if (!found) {
              for (t = roomp->stuff; t && !found; t = t->nextThing) {
                if (!dynamic_cast<TBeing *>(t))
                  continue;
                if (!canSee(t))
                  continue;
                if (isname(tmp, t->name)) {
                  totalFound++;
                }
              }
            }
            // In inventory
            if (!found) {
              for (t = stuff; t && !found; t = t->nextThing) {
                if (!canSee(t))
                  continue;
                if (t->ex_description) {
                  if ((tmp_desc = t->ex_description->findExtraDesc(tmp))) {
                    totalFound++;
                    if (iNum != totalFound)
                      continue;
                    if (o2 == t) {
                      // look at XX where XX is the item's name and extradesc
                      desc->page_string(tmp_desc, 0);
                      found = TRUE;
                      describeObject(t);
                      o2 = dynamic_cast<TObj *>(t);  // for showTo(o2,6) later on
                    } else {
                      // look at XX where XX is some random desc on the obj
                        desc->page_string(tmp_desc, 0);
                        found = TRUE;
                        return;
                    }
                  }
                }
                if (isname(tmp, t->name)) {
                  totalFound++;
                  if (iNum == totalFound) {
                    t->lookAtObj(this, tmp, SHOW_MODE_TYPE);
                    return;
                  } else {
                    continue;
                  }
                }
              }
            }
            // Worn equipment second
            if (!found) {
              for (j = MIN_WEAR; j < MAX_WEAR && !found; j++) {
                t = equipment[j];
                if (t) {
                  TObj *tobj = dynamic_cast<TObj *>(t);
                  if (tobj->isPaired()) {
                    if (isRightHanded()) {
                      if ((j == WEAR_LEGS_L) || (j == HOLD_LEFT))
                        continue;
                    } else {
                      if ((j == WEAR_LEGS_L) || (j == HOLD_RIGHT))
                        continue;
                    }
                  }
                  if (!canSee(t))
                    continue;
                  if (t->ex_description) {
                    if ((tmp_desc = t->ex_description->findExtraDesc(tmp))) {
                      totalFound++;
                      if (iNum != totalFound)
                        continue;
                      if (o2 == t) {
                        // look at XX where XX is the item's name and extradesc
                        desc->page_string(tmp_desc, 0);
                        found = TRUE;
                        describeObject(t);
                        o2 = dynamic_cast<TObj *>(t);  // for showTo(o2,6) later on
                      } else {
                        // look at XX where XX is some random desc on the obj
                        desc->page_string(tmp_desc, 0);
                        found = TRUE;
                        return;
                      }
                    }
                  }
                  if (isname(tmp, t->name)) {
                    totalFound++;
                    if (iNum == totalFound) {
                      t->lookAtObj(this, tmp, SHOW_MODE_TYPE);
                      return;
                    } else {
                      continue;
                    }
                  }
                }
              }
            }
            // room objects
            if (!found) {
              for (t = roomp->stuff; t && !found; t = t->nextThing) {
                if (dynamic_cast<TBeing *>(t))
                  continue;
                if (!canSee(t))
                  continue;
                if (t->ex_description) {
                  if ((tmp_desc = t->ex_description->findExtraDesc(tmp))) {
                    totalFound++;
                    if (iNum != totalFound)
                      continue;
                    if (o2 == (TObj *) t) {
                      desc->page_string(tmp_desc, 0);
                      found = TRUE;
                      describeObject(t);
                      return;
                    } else {
                      // look at XX where XX is some random desc on the obj
                      desc->page_string(tmp_desc, 0);
                      found = TRUE;
                      return;
                    }
                  }
                }
                if (isname(tmp, t->name)) {
                  totalFound++;
                  if (iNum == totalFound) {
                    t->lookAtObj(this, tmp, SHOW_MODE_TYPE);
                    return;
                  } else {
                    continue;
                  }
                }
                if (dynamic_cast<TTable *>(t)) {
                  for (t2 = t->rider; t2; t2 = t2->nextRider) {
                    if (dynamic_cast<TBeing *>(t2))
                      continue;
                    if (!canSee(t2))
                      continue;
                    if (t2->ex_description &&
                        (tmp_desc = t2->ex_description->findExtraDesc(tmp))) {
                      totalFound++;
                      if (iNum != totalFound)
                        continue;
                      desc->page_string(tmp_desc, 0);
                      found = TRUE;
                      describeObject(t2);
                      sendTo(COLOR_OBJECTS, "%s is on %s.", t2->getName(), t->getName());
                      return;
                    }
                    if (isname(tmp, t2->name)) {
                      totalFound++;
                      if (iNum == totalFound) {
                        t2->lookAtObj(this, tmp, SHOW_MODE_TYPE);
                        sendTo(COLOR_OBJECTS, "%s is on %s.", t2->getName(), t->getName());
                        return;
                      } else {
                        continue;
                      }
                    }
                  }
                }  // table
              }
            }
            // room extras
            if (!found) {
              if ((tmp_desc = roomp->ex_description->findExtraDesc(tmp))) {
                totalFound++;
                if (totalFound == iNum) {
                  desc->page_string(tmp_desc, 0);
                  return;
                }
              }
            }
            if (!found) {
              if (bits)                 // If an object was found 
                o2->lookAtObj(this, tmp, SHOW_MODE_TYPE);
              else
                sendTo("You do not see that here.\n\r");
              return;
            }
            showTo(o2, SHOW_MODE_PLUS);
            return;
          }
          sendTo("Look at what?\n\r");
          return;
        }
        break;

       // look '' 
      case 8:
// purple if color basic, nothing if no color, varied color if color room name
        sendRoomName(roomp);
        if (!isPlayerAction(PLR_BRIEF)) 
          sendRoomDesc(roomp);

        describeWeather(in_room);
        listExits(roomp);

        if (dynamic_cast<TPerson *>(this)) {
          if (isPlayerAction(PLR_HUNTING)) {
            if (affectedBySpell(SKILL_TRACK)) {
              if (!(res = track(specials.hunting))) {
                specials.hunting = 0;
                hunt_dist = 0;
                remPlayerAction(PLR_HUNTING);
                affectFrom(SKILL_TRACK);
              }
            } else if (affectedBySpell(SKILL_SEEKWATER)) {
              if (!(res = track(NULL))) {
                hunt_dist = 0;
                remPlayerAction(PLR_HUNTING);
                affectFrom(SKILL_SEEKWATER);
              }
            } else {
              hunt_dist = 0;
              remPlayerAction(PLR_HUNTING);
            }
          }
        } else {
          if (specials.act & ACT_HUNTING) {
            if (affectedBySpell(SKILL_TRACK)) {
              if (!(res = track(specials.hunting))) {
                specials.hunting = 0;
                hunt_dist = 0;
                REMOVE_BIT(specials.act, ACT_HUNTING);
                affectFrom(SKILL_TRACK);
              }
            } else if (affectedBySpell(SKILL_SEEKWATER)) {
              if (!(res = track(NULL))) {
                hunt_dist = 0;
                REMOVE_BIT(specials.act, ACT_HUNTING);
                affectFrom(SKILL_SEEKWATER);
              }
            } else if (specials.hunting) {
              if (!(res = track(specials.hunting))) {
                specials.hunting = 0;
                hunt_dist = 0;
                REMOVE_BIT(specials.act, ACT_HUNTING);
              }
            } else {
              hunt_dist = 0;
              REMOVE_BIT(specials.act, ACT_HUNTING);
            }
          }
        }
        list_thing_in_room(roomp->stuff, this);
        break;
      case -1:
        // wrong arg     
        sendTo("Sorry, I didn't understand that!\n\r");
        break;
      case 9:{
// purple if color basic, nothing if no color, varied color if color room name
        sendRoomName(roomp);
        sendRoomDesc(roomp);
        describeWeather(in_room);
        listExits(roomp);

        if (dynamic_cast<TPerson *>(this)) {
          if (isPlayerAction(PLR_HUNTING)) {
            if (affectedBySpell(SKILL_TRACK)) {
              if (!(res = track(specials.hunting))) {
                specials.hunting = 0;
                hunt_dist = 0;
                remPlayerAction(PLR_HUNTING);
                affectFrom(SKILL_TRACK);
              }
            } else if (affectedBySpell(SKILL_SEEKWATER)) {
              if (!(res = track(NULL))) {
                hunt_dist = 0;
                remPlayerAction(PLR_HUNTING);
                affectFrom(SKILL_SEEKWATER);
              }
            } else {
              hunt_dist = 0;
              remPlayerAction(PLR_HUNTING);
            }
          }
        } else {
          if (IS_SET(specials.act, ACT_HUNTING)) {
            if (affectedBySpell(SKILL_TRACK)) {
              if (!(res = track(specials.hunting))) {
                specials.hunting = 0;
                hunt_dist = 0;
                REMOVE_BIT(specials.act, ACT_HUNTING);
                affectFrom(SKILL_TRACK);
              }
            } else if (affectedBySpell(SKILL_SEEKWATER)) {
              if (!(res = track(NULL))) {
                hunt_dist = 0;
                REMOVE_BIT(specials.act, ACT_HUNTING);
                affectFrom(SKILL_SEEKWATER);
              }
            } else if (specials.hunting) {
              if (!(res = track(specials.hunting))) {
                specials.hunting = 0;
                hunt_dist = 0;
                REMOVE_BIT(specials.act, ACT_HUNTING);
              }
            } else {
              hunt_dist = 0;
              REMOVE_BIT(specials.act, ACT_HUNTING);
            }
          }
        }
        list_thing_in_room(roomp->stuff, this);
      }
      break;
    }
  }
}

string TBeing::dynColorRoom(TRoom * rp, int title, bool) const
{
//  if (rp && title && full) {
//  }

  int len, letter;
  char argument[MAX_STRING_LENGTH];
  char buf2[10];
  char buf3[5];

  *buf2 = *buf3 = '\0';

  memset(argument, '\0', sizeof(argument));
  memset(buf2, '\0', sizeof(buf2));
  memset(buf3, '\0', sizeof(buf3));

  if (title == 1) {
    if (rp->getName()) {
      strcpy(argument, rp->getName());
      if (argument[0] == '<') {
          buf3[0] = argument[0];
          buf3[1] = argument[1];
          buf3[2] = argument[2];
          strcpy(buf2, buf3);
      } else {
        strcpy(buf2, addColorRoom(rp, 1).c_str());
      }
    } else {
      vlogf(5, "%s is in a room with no descr", getName());
      return "Bogus Name";
    }
  } else if (title == 2) {
    if (rp->getDescr()) {
      strcpy(argument, rp->getDescr());
      if (argument[0] == '<') {
#if 1
        buf2[0] = argument[0];
        buf2[1] = argument[1];
        buf2[2] = argument[2];
#else
        buf3[0] = argument[0];
        buf3[1] = argument[1];  
        buf3[2] = argument[2];  
        strcpy(buf2, buf3);
#endif
      } else {   
        strcpy(buf2, addColorRoom(rp, 2).c_str());
      }
    } else {
      vlogf(5, "%s is in a room with no descr", getName());
      return "Bogus Name";
    }
  } else {
    vlogf(5, "%s called a function with a bad dynColorRoom argument", getName());
    return "Something Bogus, tell a god";
  }
// Found had to initialize with this logic and too tired to figure out why

  string buf = "";
  if (buf2) {
    buf = buf2;
  }

  len = strlen(argument);
  for(letter=0; letter <= len; letter++) {
    if (letter < 2) {
      buf += argument[letter];
      continue;
    }
    if ((argument[letter] == '>') && (argument[letter - 2] == '<')) {
      switch (argument[(letter - 1)]) {
        case '1':
        case 'z':
        case 'Z':
          buf += argument[letter];
          if (buf2) {
            buf += buf2;
          }
          break;
        default:
          buf += argument[letter];
          break;
      }
    } else {
      buf += argument[letter];
    }
  }
  buf += "<1>";
  return buf;
}

// Peel
string TRoom::daynightColorRoom() const
{
  if(IS_SET(roomFlags, ROOM_INDOORS))
    return("<z>");

  switch (weather_info.sunlight) {
    case SUN_DAWN:
    case SUN_RISE:
      return("<w>");
      break;
    case SUN_LIGHT:
      return("<z>");
      break;
    case SUN_SET:
    case SUN_TWILIGHT:
      return("<w>");
      break;
    case SUN_DARK:
      return("<k>");
      break;
  }

  return("<z>");
}

const string TBeing::addColorRoom(TRoom * rp, int title) const
{
  char buf2[10];
  char buf3[10];

  *buf2 = *buf3 = '\0';

// Found had to initialize with this logic and too tired to figure out why
  strcpy(buf3, "<z>");

  sectorTypeT sector = rp->getSectorType();

  switch (sector) {
    case SECT_SUBARCTIC:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<p>");
      break;
    case SECT_ARCTIC_WASTE:
      strcpy(buf2, "<w>");
      strcpy(buf3, "<W>");
      break;
    case SECT_ARCTIC_CITY:
      strcpy(buf2, "<C>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_ARCTIC_ROAD:
      strcpy(buf2, "<W>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_TUNDRA:
      strcpy(buf2, "<o>");
      strcpy(buf3, "<p>");
      break;
    case SECT_ARCTIC_MOUNTAINS:
      strcpy(buf2, "<o>");
      strcpy(buf3, "<W>");
      break;
    case SECT_ARCTIC_FOREST:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<W>");
      break;
    case SECT_ARCTIC_MARSH:
      strcpy(buf2, "<B>");
      strcpy(buf3, "<p>");
      break;
    case SECT_ARCTIC_RIVER_SURFACE:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<c>");
      break;
    case SECT_ICEFLOW:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<W>");
      break;
    case SECT_COLD_BEACH:
      strcpy(buf2, "<p>");
      strcpy(buf3, "<P>");
      break;
    case SECT_SOLID_ICE:
      strcpy(buf2, "<c>");
      strcpy(buf3, "<C>");
      break;
    case SECT_ARCTIC_BUILDING:
      strcpy(buf2, "<p>");
      break;
    case SECT_ARCTIC_CAVE:
      strcpy(buf2, "<c>");
      strcpy(buf3, "<k>");
      break;
    case SECT_ARCTIC_ATMOSPHERE:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<C>");
      break;
    case SECT_ARCTIC_CLIMBING:
    case SECT_ARCTIC_FOREST_ROAD:
      strcpy(buf2, "<p>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_PLAINS:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TEMPERATE_CITY:
    case SECT_TEMPERATE_ROAD:
      strcpy(buf2, "<p>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_GRASSLANDS:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TEMPERATE_HILLS:
      strcpy(buf2, "<o>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TEMPERATE_MOUNTAINS:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<o>");
      break;
    case SECT_TEMPERATE_FOREST:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TEMPERATE_SWAMP:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<p>");
      break;
    case SECT_TEMPERATE_OCEAN:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<c>");
      break;
    case SECT_TEMPERATE_RIVER_SURFACE:
      strcpy(buf2, "<B>");
      strcpy(buf3, "<b>");
      break;
    case SECT_TEMPERATE_UNDERWATER:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<b>");
      break;
    case SECT_TEMPERATE_CAVE:
      strcpy(buf2, "<o>");
      strcpy(buf3, "<k>");
      break;
    case SECT_TEMPERATE_ATMOSPHERE:
      strcpy(buf2, "<G>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_TEMPERATE_CLIMBING:
      strcpy(buf2, "<G>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_TEMPERATE_FOREST_ROAD:
      strcpy(buf2, "<g>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_DESERT:
    case SECT_SAVANNAH:
      strcpy(buf2, "<y>");
      strcpy(buf3, "<o>");
      break;
    case SECT_VELDT:
      strcpy(buf2, "<g>");
      strcpy(buf3, "<o>");
      break;
    case SECT_TROPICAL_CITY:
      strcpy(buf2, "<G>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_TROPICAL_ROAD:
      strcpy(buf2, "<g>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_JUNGLE:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<g>");
      break;
    case SECT_RAINFOREST:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TROPICAL_HILLS:
      strcpy(buf2, "<R>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TROPICAL_MOUNTAINS:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<p>");
      break;
    case SECT_VOLCANO_LAVA:
      strcpy(buf2, "<y>");
      strcpy(buf3, "<R>");
      break;
    case SECT_TROPICAL_SWAMP:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TROPICAL_OCEAN:
      strcpy(buf2, "<b>");
      strcpy(buf3, "<c>");
      break;
    case SECT_TROPICAL_RIVER_SURFACE:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<B>");
      break;
    case SECT_TROPICAL_UNDERWATER:
      strcpy(buf2, "<B>");
      strcpy(buf3, "<b>");
      break;
    case SECT_TROPICAL_BEACH:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<y>");
      break;
    case SECT_TROPICAL_BUILDING:
      strcpy(buf2, "<p>");
      break;
    case SECT_TROPICAL_CAVE:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<k>");
      break;
    case SECT_TROPICAL_ATMOSPHERE:
      strcpy(buf2, "<P>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_TROPICAL_CLIMBING:
      strcpy(buf2, "<P>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_RAINFOREST_ROAD:
      strcpy(buf2, "<P>");
      strcpy(buf3, rp->daynightColorRoom().c_str());
      break;
    case SECT_ASTRAL_ETHREAL:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<c>");
      break;
    case SECT_SOLID_ROCK:
      strcpy(buf2, "<k>");
      strcpy(buf3, "<w>");
      break;
    case SECT_FIRE:
      strcpy(buf2, "<y>");
      strcpy(buf3, "<R>");
      break;
    case SECT_INSIDE_MOB:
      strcpy(buf2, "<R>");
      strcpy(buf3, "<r>");
      break;
    case SECT_FIRE_ATMOSPHERE:
      strcpy(buf2, "<y>");
      strcpy(buf3, "<R>");
      break;
    case SECT_TEMPERATE_BEACH:
    case SECT_TEMPERATE_BUILDING:
    case SECT_MAKE_FLY:
    case MAX_SECTOR_TYPES:
      strcpy(buf2, "<p>");
      break;
  }

  if (title == 1) {
    if (rp->getName()) {
      return buf2;
    } else {
      vlogf(5, "room without a name for dynamic coloring");
      return "";
    }
  } else if (title == 2) {
    if (rp->getDescr()) 
      return buf3;
    else {
      vlogf(5, "room without a descr for dynamic coloring, %s", roomp->getName());
      return "";
    }
  } else {
    vlogf(5, "addColorRoom without a correct title variable");
    return "";
  }
}

void TBeing::doRead(const char *argument)
{
  char buf[100];

  // This is just for now - To be changed later! 
  sprintf(buf, "at %s", argument);
  doLook(buf, CMD_READ);
}

void TBaseCup::examineObj(TBeing *ch) const
{
  int bits = FALSE;

  if (parent && (ch == parent)) {
    bits = FIND_OBJ_INV;
  } else if (equippedBy && (ch == equippedBy)) {
    bits = FIND_OBJ_EQUIP;
  } else if (parent && (ch->roomp == parent)) {
    bits = FIND_OBJ_ROOM;
  }

  ch->sendTo("When you look inside, you see:\n\r");
#if 1
  lookObj(ch, bits);
#else
  char buf[256];
  char buf2[256];
  sprintf(buf2, "%s", name);
  add_bars(buf2);
  sprintf(buf, "in %s", buf2);
  ch->doLook(buf, CMD_LOOK);
#endif
}

void TContainer::examineObj(TBeing *ch) const
{
  int bits = FALSE;

  if (parent && (ch == parent)) {
    bits = FIND_OBJ_INV;
  } else if (equippedBy && (ch == equippedBy)) {
    bits = FIND_OBJ_EQUIP;
  } else if (parent && (ch->roomp == parent)) {
    bits = FIND_OBJ_ROOM;
  }

  ch->sendTo("When you look inside, you see:\n\r");
#if 1
  lookObj(ch, bits);
#else
  char buf[256];
  char buf2[256];
  sprintf(buf2, "%s", name);
  add_bars(buf2);
  sprintf(buf, "in %s", buf2);
  ch->doLook(buf, CMD_LOOK);
#endif
}

void TBeing::doExamine(const char *argument, TThing * specific)
{
  char caName[100], buf[100];
  int bits;
  TBeing *tmp = NULL;
  TObj *o = NULL;

  if (specific) {
    doLook("", CMD_LOOK, specific);
    return;
  }

  one_argument(argument, caName);

  if (!*caName) {
    sendTo("Examine what?\n\r");
    return;
  }
  bits = generic_find(caName, FIND_ROOM_EXTRA | FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP | FIND_CHAR_ROOM, this, &tmp, &o);

  if (bits) {
    sprintf(buf, "at %s", argument);
    doLook(buf, CMD_LOOK);
  }
  if (o) 
    o->examineObj(this);
  
  if (!bits && !o)
    sendTo("Examine what?\n\r");
}

void TBeing::describeAffects(TBeing *ch)
{
  affectedData *aff, *af2;

  for (aff = ch->affected; aff; aff = af2) {
    af2 = aff->next;

    if (aff->type == AFFECT_DISEASE) {
      if (ch == this)
        sendTo("Disease: '%s'\n\r",
                DiseaseInfo[DISEASE_INDEX(aff->modifier)].name);
    } else if (aff->type == AFFECT_DUMMY) {
      if (this == ch)
        sendTo("Dummy Affect: \n\r");
      else
        sendTo("Affected : '%s'\t: Time Left : %s %s\n\r",
               "DUMMY",
               describeDuration(this, aff->duration).c_str(),
               (aff->canBeRenewed() ? "(Renewable)" : "(Not Yet Renewable)"));
    } else if (aff->type == AFFECT_FREE_DEATHS) {
      sendTo("Free deaths remaining: %d\n\r",
               aff->modifier);
    } else if (aff->type == AFFECT_PLAYERKILL) {
      sendTo("PLAYER KILLER!\n\r");
    } else if (aff->type == AFFECT_TEST_FIGHT_MOB) {
      sendTo("Test Fight Mob: %d\n\r",
               aff->modifier);
    } else if (aff->type == AFFECT_SKILL_ATTEMPT) {
      if (isImmortal()) {
        sendTo("Skill Attempt:(%d) '%s'\t: Time Left : %s\n\r", aff->modifier, (discArray[aff->modifier] ? discArray[aff->modifier]->name : "Unknown"), describeDuration(this, aff->duration).c_str());
      } else if (aff->modifier != getSkillNum(SKILL_SNEAK)) {
        sendTo("Skill Attempt: '%s'\t: Time Left : %s\n\r", (discArray[aff->modifier] ? discArray[aff->modifier]->name : "Unknown"), describeDuration(this, aff->duration).c_str());
      }
    } else if (aff->type == AFFECT_NEWBIE) {
      if (this == ch)
        sendTo("Donation Recipient: \n\r");
    } else if (aff->type == SKILL_TRACK || aff->type == SKILL_SEEKWATER) {
      sendTo("Tracking: %s\n\r", (aff->type == SKILL_TRACK ?
             ch->specials.hunting->getName() : "seeking water"));
    } else if (aff->type == AFFECT_COMBAT || aff->type == AFFECT_PET) {
      // no display
    } else if (aff->type == AFFECT_TRANSFORMED_ARMS) {
      if (ch == this)
        sendTo("Affected: Transformed Limb: falcon wings: approx. duration : %s\n\r",
               describeDuration(this, aff->duration).c_str());
      else
        sendTo("Affected: Transformed Limb: falcon wings: \n\r");
    } else if (aff->type == AFFECT_TRANSFORMED_HANDS) {
      if (ch == this)
        sendTo("Affected: Transformed Limb: bear claws: approx. duration : %s\n\r",
               describeDuration(this, aff->duration).c_str());
      else
        sendTo("Affected: Transformed Limb: bear claws \n\r");
    } else if (aff->type == AFFECT_TRANSFORMED_LEGS) {
      if (ch == this)
        sendTo("Affected: Transformed Limb: dolphin tail: approx. duration : %s\n\r",
               describeDuration(this, aff->duration).c_str());
      else
        sendTo("Affected: Transformed Limb: dolphin tail: \n\r");
    } else if (aff->type == AFFECT_TRANSFORMED_HEAD) {
      if (ch == this)
        sendTo("Affected: Transformed Limb: eagle's head: approx. duration : %s\n\r",
               describeDuration(this, aff->duration).c_str());
      else
        sendTo("Affected: Transformed Limb: eagle's head: \n\r");
    } else if (aff->type == AFFECT_TRANSFORMED_NECK) {
      if (ch == this)
        sendTo("Affected: Transformed Limb: fish gills: approx. duration : %s\n\r",
               describeDuration(this, aff->duration).c_str());
      else
        sendTo("Affected: Transformed Limb: fish gills: \n\r");
    } else if (aff->type == AFFECT_DRUNK) {
      if (ch == this)
        sendTo("Affected: Drunken Slumber: approx. duration : %s\n\r",
               describeDuration(this, aff->duration).c_str());
      else
        sendTo("Affected: Drunken Slumber: \n\r");
    } else if (aff->type == AFFECT_DRUG) {
      if (!aff->shouldGenerateText())
	continue;
      if (ch == this)
	sendTo("Affected: %s: approx. duration : %s\n\r",
	       drugTypes[aff->modifier2].name,
	       describeDuration(this, aff->duration).c_str());
      else
	sendTo("Affected: %s: \n\r", drugTypes[aff->modifier2].name);
    } else if (aff->type >= MIN_SPELL && aff->type < MAX_SKILL) {
      // some spells have 2 effects, skip over one of them
      if (!aff->shouldGenerateText())
        continue;
      else if (discArray[aff->type]) {
        if ((ch == this) && strcmp(discArray[aff->type]->name, "sneak")) {
          if (aff->renew == -1) {
            sendTo("Affected : '%s'\t: Approx. Duration : %s\n\r",
                 discArray[aff->type]->name,
                 describeDuration(this, aff->duration).c_str());

          } else {
            sendTo("Affected : '%s'\t: Time Left : %s %s\n\r",
               discArray[aff->type]->name,
               describeDuration(this, aff->duration).c_str(), 
               (aff->canBeRenewed() ? "(Renewable)" : "(Not Yet Renewable)"));
          }
        }
      } else {
        forceCrash("BOGUS AFFECT (%d) on %s.", aff->type, ch->getName());
        ch->affectRemove(aff);
      }
    } else {
      forceCrash("BOGUS AFFECT (%d) on %s.", aff->type, ch->getName());
      ch->affectRemove(aff);
    }
  }
}

void TBeing::describeLimbDamage(const TBeing *ch) const
{
  char buf[256], buf2[80];
  wearSlotT j;
  TThing *t;

  if (ch == this)
    strcpy(buf2,"your");
  else
    strcpy(buf2,ch->hshr());

  for (j = MIN_WEAR; j < MAX_WEAR; j++) {
    if (j == HOLD_RIGHT || j == HOLD_LEFT)
      continue;
    if (!ch->slotChance(j))
      continue;
    if (ch->isLimbFlags(j, ~PART_TRANSFORMED)) {
      const string str = describe_part_wounds(ch, j);
      if (!str.empty()) {
        sprintf(buf, "<y>%s %s %s %s<1>", cap(buf2), 
               ch->describeBodySlot(j).c_str(),
               ch->slotPlurality(j).c_str(), 
               str.c_str());
        act(buf, FALSE, this, NULL, NULL, TO_CHAR);
      }
    }
    if ((t = ch->getStuckIn(j))) {
      if (canSee(t)) {
        sprintf(buf, "<y>$p is sticking out of %s %s!<1>",
                uncap(buf2), ch->describeBodySlot(j).c_str());
        act(buf, FALSE, this, t, NULL, TO_CHAR);
      }
    }
  }
  if (ch->affected) {
    affectedData *aff;
    for (aff = ch->affected; aff; aff = aff->next) {
      if (aff->type == AFFECT_DISEASE) {
        if (!aff->level) {
          if (ch == this)
            sendTo(COLOR_BASIC, "<y>You have %s.<1>\n\r",
               DiseaseInfo[DISEASE_INDEX(aff->modifier)].name);
          else
            sendTo(COLOR_BASIC, "<y>It seems %s has %s.<1>\n\r",
                ch->hssh(), DiseaseInfo[DISEASE_INDEX(aff->modifier)].name);
        }
      }
    }
  }
}

void TBeing::doTime(const char *argument)
{
  char buf[100], arg[160];
  int weekday, day, tmp_num, tmp2;

  if (!desc) {
    sendTo("Silly mob, go home.\n\r");
    return;
  }

  one_argument(argument, arg);
  if (*arg) {
    if (!atoi(arg) && strcmp(arg, "0")) {
      sendTo("Present time differential is set to %d hours.\n\r", desc->account->time_adjust);
      sendTo("Syntax: time <difference>\n\r");
      return;
    }
    desc->account->time_adjust = atoi(arg);
    sendTo("Your new time difference between your site and %s's will be: %d hours.\n\r", MUD_NAME, desc->account->time_adjust);
    desc->saveAccount();
    return;
  }
  tmp_num = (time_info.hours / 2);
  sprintf(buf, "It is %d:%s %s, on ",
          (!(tmp_num % 12) ? 12 : (tmp_num % 12)),
          (!(time_info.hours % 2) ? "00" : "30"),
          ((time_info.hours >= 24) ? "PM" : "AM"));

  weekday = ((28 * time_info.month) + time_info.day + 1) % 7;        // 28 days in a month 

  strcat(buf, weekdays[weekday]);
  strcat(buf, "\n\r");
  sendTo(buf);

  day = time_info.day + 1;        // day in [1..35] 

  sendTo("The %s day of %s, Year %d P.S.\n\r", 
           numberAsString(day).c_str(),
           month_name[time_info.month], time_info.year);

  tmp2 = sunRise();
  tmp_num = tmp2 / 2;
  sprintf(buf, "The sun will rise today at:   %d:%s %s.\n\r",
          (!(tmp_num % 12) ? 12 : (tmp_num % 12)),
          (!(tmp2 % 2) ? "00" : "30"),
          ((tmp2 >= 24) ? "PM" : "AM"));
  sendTo(buf);

  tmp2 = sunSet();
  tmp_num = tmp2 / 2;
  sprintf(buf, "The sun will set today at:    %d:%s %s.\n\r",
          (!(tmp_num % 12) ? 12 : (tmp_num % 12)),
          (!(tmp2 % 2) ? "00" : "30"),
          ((tmp2 >= 24) ? "PM" : "AM"));
  sendTo(buf);

  tmp2 = moonRise();
  tmp_num = tmp2 / 2;
  sprintf(buf, "The moon will rise today at:  %d:%s %s    (%s).\n\r",
          (!(tmp_num % 12) ? 12 : (tmp_num % 12)),
          (!(tmp2 % 2) ? "00" : "30"),
          ((tmp2 >= 24) ? "PM" : "AM"), moonType());
  sendTo(buf);

  tmp2 = moonSet();
  tmp_num = tmp2 / 2;
  sprintf(buf, "The moon will set today at:   %d:%s %s.\n\r",
          (!(tmp_num % 12) ? 12 : (tmp_num % 12)),
          (!(tmp2 % 2) ? "00" : "30"),
          ((tmp2 >= 24) ? "PM" : "AM"));
  sendTo(buf);

  time_t ct;
  char *tmstr;
  if (desc->account)
    ct = time(0) + 3600 * desc->account->time_adjust;
  else
    ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sendTo("%sIn the real world, the time is:                     %s%s\n\r", 
        blue(), tmstr, norm());

  if (timeTill) {
    sendTo("%sThe game will be shutdown in %s.\n\r%s",
           red(),
           secsToString(timeTill - time(0)).c_str(),
           norm());
  }
}

void TBeing::doWeather(const char *arg)
{
  char buf[80];
  char buffer[256];
  changeWeatherT change = WEATHER_CHANGE_NONE;
 
  arg = one_argument(arg, buffer);

  if (!*buffer || !isImmortal()) {
    if (roomp->getWeather() == WEATHER_SNOWY)
      strcpy(buf,"It is snowing");
    else if (roomp->getWeather() == WEATHER_LIGHTNING)
      strcpy(buf,"The sky is lit by flashes of lightning as a heavy rain pours
down");
    else if (roomp->getWeather() == WEATHER_RAINY)
      strcpy(buf,"It is raining");
    else if (roomp->getWeather() == WEATHER_CLOUDY)
      strcpy(buf,"The sky is cloudy");
    else if (roomp->getWeather() == WEATHER_CLOUDLESS) {
      if (is_nighttime())
        strcpy(buf,"A <K>dark sky<1> stretches before you");
      else
        strcpy(buf,"A <b>clear blue sky<1> stretches before you");
//  } else if (roomp->getWeather() == WEATHER_WINDY) {
//    strcpy(buf,"The winds begin to pick up");
    } else if (roomp->getWeather() == WEATHER_NONE) {
      sendTo("You have no feeling about the weather at all.\n\r");
      describeRoomLight();
      return;
    } else {
      vlogf(5,"Error in getWeather for %s.",getName());
      return;
    }
    sendTo(COLOR_BASIC, "%s and %s.\n\r", buf,
        (weather_info.change >= 0 ? "you feel a relatively warm wind from the south" :
         "your foot tells you bad weather is due"));

    if (moonIsUp()) {
      sendTo("A %s moon hangs in the sky.\n\r", moonType());
    }
    describeRoomLight();
    return;
  } else {
    if (is_abbrev(buffer, "worse")) {
      weather_info.change = -10;
      sendTo("The air-pressure drops and the weather worsens.\n\r");
      weather_info.pressure += weather_info.change;

      AlterWeather(&change);
      if (weather_info.pressure >= 1040) {
        sendTo("The weather can't get any better.\n\r");
        weather_info.pressure = 1040;
      } else if (weather_info.pressure <= 960) {
        sendTo("The weather can't get any worse.\n\r");
        weather_info.pressure = 960;
      }
      return;
    } else if (is_abbrev(buffer, "better")) {
      weather_info.change = +10;
      sendTo("The air-pressure climbs and the weather improves.\n\r");
      weather_info.pressure += weather_info.change;

      AlterWeather(&change);
      if (weather_info.pressure >= 1040) {
        sendTo("The weather can't get any better.\n\r");
        weather_info.pressure = 1040;
      } else if (weather_info.pressure <= 960) {
        sendTo("The weather can't get any worse.\n\r");
        weather_info.pressure = 960;
      }
    } else if (is_abbrev(buffer, "month")) {
      arg = one_argument(arg, buffer);
      if (!*buffer) {
        sendTo("Syntax: weather month <num>\n\r");
        return;
      }
      int num = atoi(buffer);
      if (num <= 0 || num > 12) {
        sendTo("Syntax: weather month <num>\n\r");
        sendTo("<num> must be in range 1-12.\n\r");
        return;
      }
      time_info.month = num - 1 ;
      sendTo("You set the month to: %s\n\r", month_name[time_info.month]);
      return;
    } else if (is_abbrev(buffer, "moon")) {
      arg = one_argument(arg, buffer);
      if (!*buffer) {
        sendTo("Syntax: weather moon <num>\n\r");
        return;
      }
      int num = atoi(buffer);
      if (num <= 0 || num > 32) {
        sendTo("Syntax: weather moon <num>\n\r");
        sendTo("<num> must be in range 1-32.\n\r");
        return;
      }
      moontype = num;
      sendTo("The moon is now in stage %d (%s).\n\r", moontype, moonType());
      return;
    } else {
      sendTo("Syntax: weather <\"worse\" | \"better\" | \"month\" | \"moon\">\n\r");
      return;
    }
  }
}

wizPowerT wizPowerFromCmd(cmdTypeT cmd)
{
  switch (cmd) {
    case CMD_CHANGE:
      return POWER_CHANGE;
      break;
    case CMD_ECHO:
      return POWER_ECHO;
      break;
    case CMD_FORCE:
      return POWER_FORCE;
      break;
    case CMD_TRANSFER:
      return POWER_TRANSFER;
      break;
    case CMD_STAT:
      return POWER_STAT;
      break;
    case CMD_LOAD:
      return POWER_LOAD;
      break;
    case CMD_PURGE:
      return POWER_PURGE;
      break;
    case CMD_AT:
      return POWER_AT;
      break;
    case CMD_SNOOP:
      return POWER_SNOOP;
      break;
    case CMD_AS:
    case CMD_SWITCH:
      return POWER_SWITCH;
      break;
    case CMD_SNOWBALL:
      return POWER_SNOWBALL;
      break;
    case CMD_INFO:
      return POWER_INFO;
      break;
    case CMD_WHERE:
      return POWER_WHERE;
      break;
    case CMD_PEE:
      return POWER_PEE;
      break;
    case CMD_WIZNET:
      return POWER_WIZNET;
      break;
    case CMD_RESTORE:
      return POWER_RESTORE;
      break;
    case CMD_USERS:
      return POWER_USERS;
      break;
    case CMD_SYSTEM:
      return POWER_SYSTEM;
      break;
    case CMD_STEALTH:
      return POWER_STEALTH;
      break;
    case CMD_SET:
      return POWER_SET;
      break;
    case CMD_RSAVE:
      return POWER_RSAVE;
      break;
    case CMD_RLOAD:
      return POWER_RLOAD;
      break;
    case CMD_WIZLOCK:
      return POWER_WIZLOCK;
      break;
    case CMD_SHOW:
      return POWER_SHOW;
      break;
    case CMD_TOGGLE:
      return POWER_TOGGLE;
      break;
    case CMD_BREATH:
      return POWER_BREATHE;
      break;
    case CMD_LOG:
      return POWER_LOG;
      break;
    case CMD_WIPE:
      return POWER_WIPE;
      break;
    case CMD_CUTLINK:
      return POWER_CUTLINK;
      break;
    case CMD_CHECKLOG:
      return POWER_CHECKLOG;
      break;
    case CMD_LOGLIST:
      return POWER_LOGLIST;
      break;
    case CMD_DEATHCHECK:
      return POWER_DEATHCHECK;
      break;
    case CMD_REDIT:
      return POWER_REDIT;
      break;
    case CMD_OEDIT:
      return POWER_OEDIT;
      break;
    case CMD_MEDIT:
      return POWER_MEDIT;
      break;
    case CMD_ACCESS:
      return POWER_ACCESS;
      break;
    case CMD_REPLACE:
      return POWER_REPLACE;
      break;
    case CMD_GAMESTATS:
      return POWER_GAMESTATS;
      break;
    case CMD_HOSTLOG:
      return POWER_HOSTLOG;
      break;
    case CMD_TRACEROUTE:
      return POWER_TRACEROUTE;
      break;
    case CMD_LOW:
      return POWER_LOW;
      break;
    case CMD_RESIZE:
      return POWER_RESIZE;
      break;
    case CMD_HEAVEN:
      return POWER_HEAVEN;
      break;
    case CMD_ACCOUNT:
      return POWER_ACCOUNT;
      break;
    case CMD_CLIENTS:
      return POWER_CLIENTS;
      break;
    case CMD_FINDEMAIL:
      return POWER_FINDEMAIL;
      break;
    case CMD_COMMENT:
      return POWER_COMMENT;
      break;
    case CMD_EGOTRIP:
      return POWER_EGOTRIP;
      break;
    case CMD_LONGDESCR:
      return POWER_LONGDESC;
      break;
    case CMD_POWERS:
      return POWER_POWERS;
      break;
    case CMD_SEDIT:
      return POWER_SEDIT;
      break;
    case CMD_SHUTDOWN:
    case CMD_SHUTDOW:
      return POWER_SHUTDOWN;
      break;
    case CMD_RESET:
      return POWER_RESET;
      break;
    case CMD_SLAY:
      return POWER_SLAY;
      break;
    case CMD_TIMESHIFT:
      return POWER_TIMESHIFT;
      break;
    case CMD_CRIT:
      return POWER_CRIT;
      break;
    case CMD_RELEASE: // ???
    case CMD_CAPTURE: // ???
    case CMD_CREATE:  // Lapsos
    case CMD_TASKS:
    case CMD_TEST_FIGHT:
    case CMD_PEELPK:
    case CMD_TESTCODE:
    case CMD_BRUTTEST:
      return POWER_WIZARD;
      break;
    break;
    default:
      break;
  }

  return MAX_POWER_INDEX;
}

void TBeing::doWizhelp()
{
  char      buf[MAX_STRING_LENGTH];
  int       no;
  wizPowerT tPower;

  if (!isImmortal())
    return;

  sendTo("The following privileged commands are available:\n\r\n\r");

  *buf = '\0';

  if ((tPower = wizPowerFromCmd(CMD_AS)) == MAX_POWER_INDEX ||
      hasWizPower(tPower))
    strcpy(buf,"as        ");  // has to be at level 1 to be useful 

  unsigned int i;

  for (no = 2, i = 0; i < MAX_CMD_LIST; i++) {
    if (!commandArray[i])
      continue;

    if ((GetMaxLevel() >= commandArray[i]->minLevel) &&
        (commandArray[i]->minLevel > MAX_MORT) &&
        ((tPower = wizPowerFromCmd(cmdTypeT(i))) == MAX_POWER_INDEX ||
         hasWizPower(tPower))) {

      sprintf(buf + strlen(buf), "%-10s", commandArray[i]->name);
      if (!(no % 7))
        strcat(buf, "\n\r");
      no++;
    }
  }

  strcat(buf, "\n\r      Check out HELP GODS (or HELP BUILDERS) for an index of help files.\n\r");
  desc->page_string(buf, 0);
}

void TBeing::doUsers(const char *)
{
  sendTo("Dumb monsters can't use the users command!\n\r");
}

void TPerson::doUsers(const char *argument)
{
  char line[200], buf2[100], buf3[100], buf4[10];
  Descriptor *d;
  int count = 0;
  string sb;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  TBeing *k = NULL;

  if (powerCheck(POWER_USERS))
    return;

  const char USERS_HEADER[] = "\n\rName              Hostname                           Connected  Account Name\n\r--------------------------------------------------------------------------------\n\r";

  argument = two_arg(argument, arg1, arg2);

  *line = '\0';

  if (!*arg1 || !arg1) {
    sb += USERS_HEADER;

    for (d = descriptor_list; d; d = d->next) {
      if (d->character && d->character->name) {
        if (!d->connected && !canSeeWho(d->character))
          continue;

        if (d->original)
          sprintf(line, "%s%-16.16s%s: ",purple(), d->original->name, norm());
        else
          sprintf(line, "%s%-16.16s%s: ",purple(), d->character->name, norm());
      } else
        strcpy(line, "UNDEFINED       : ");

      // don't let newbie gods blab who imm's mortals are
      if (d->account && IS_SET(d->account->flags, ACCOUNT_IMMORTAL) && 
            !hasWizPower(POWER_VIEW_IMM_ACCOUNTS)) {
        sprintf(line + strlen(line), "*** Information Concealed ***\n\r");
      } else {
        sprintf(buf2, "[%s]", (d->host ? d->host : "????"));
        sprintf(buf3, "[%s]", ((d->connected < MAX_CON_STATUS && d->connected >= 0) ? connected_types[d->connected] : "Editing"));
        sprintf(buf4, "[%s]", (d->account && d->account->name) ? d->account->name : "UNDEFINED");
        sprintf(line + strlen(line), "%s%-34.34s%s %s%-10.10s%s %s%s%s\n\r", red(), buf2, norm(), green(), buf3, norm(), cyan(), buf4, norm());
      }
      sb += line;
      count++;
    }
    sprintf(buf2, "\n\rTotal Descriptors : %d\n\r", count);
    sb += buf2;
    if (desc)
      desc->page_string(sb.c_str(), 0, TRUE);
    return;
  } else if (is_abbrev(arg1, "site")) {
    if (!*arg2 || !arg2) {
      sendTo("Syntax : users site <sitename>\n\r");
      return;
    } else {
      sendTo("\n\rPlayers online from %s:\n\r\n\r", arg2);
      sendTo(USERS_HEADER);
      for (d = descriptor_list; d; d = d->next) {
        if (d->host && strcasestr(d->host, arg2)) {
          if (d->character && d->character->name) {
            if (!d->connected && !canSeeWho(d->character))
              continue;

            if (d->original)
              sprintf(line, "%-16.16s: ", d->original->name);
            else
              sprintf(line, "%-16.16s: ", d->character->name);
          } else
            strcpy(line, "UNDEFINED       : ");

          // don't let newbie gods blab who imm's mortals are
          if (d->account && IS_SET(d->account->flags, ACCOUNT_IMMORTAL) && 
                !hasWizPower(POWER_VIEW_IMM_ACCOUNTS)) {
            sprintf(line + strlen(line), "*** Information Concealed ***\n\r");
          } else {
            sprintf(buf2, "[%s]", (d->host ? d->host : "????"));
            sprintf(buf3, "[%s]", ((d->connected < MAX_CON_STATUS && d->connected >= 0) ? connected_types[d->connected] : "Editing"));
            sprintf(buf4, "[%s]", (d->account && d->account->name) ? d->account->name : "UNDEFINED");
            sprintf(line + strlen(line), "%-34.34s %-10.10s %s\n\r", buf2, buf3, buf4);
          }
          sendTo(line);
          count++;
        }
      }
    }
    if (!count) {
      sendTo("No players online from that site.\n\r");
      return;
    }
  } else if ((k = get_pc_world(this, arg1, EXACT_YES)) ||
             (k = get_pc_world(this, arg1, EXACT_NO))) {
    if (k->desc) {
      // don't let newbie gods blab who imm's mortals are
      if (k->desc->account && IS_SET(k->desc->account->flags, ACCOUNT_IMMORTAL) && 
            !hasWizPower(POWER_VIEW_IMM_ACCOUNTS)) {
        sendTo(COLOR_MOBS, "\n\r%-16.16s : *******Information Concealed*******\n\r", k->getName());
      } else {
        sprintf(buf2, "[%s]", (k->desc->host ? k->desc->host : "????"));
        sprintf(buf3, "[%s]", ((k->desc->connected < MAX_CON_STATUS && k->desc->connected >= 0) ? connected_types[k->desc->connected] : "Editing"));
        sprintf(buf4, "[%s]", (k->desc->account->name));
        sendTo(COLOR_MOBS, "\n\r%-16.16s : %-34.34s %-15.15s %-10.10s\n\r", k->getName(), buf2, buf3, buf4);
      }
      return;
    } else {
      sendTo("That person is linkdead. Users can give you no info on them.\n\r");
      return;
    }
  } else {
    sendTo("Syntax : users (no argument list all users)\n\r");
    sendTo("         users <playername>\n\r");
    sendTo("         users site <sitename>\n\r");
    sendTo("The sitename can be abbreviated just like when you wizlock.\n\r");
    return;
  }
}

void TBeing::doInventory(const char *argument)
{
  TBeing *victim;
  char arg[80];

  one_argument(argument, arg);

  if (!*argument || !isImmortal()) {
    if (isAffected(AFF_BLIND)) {
      sendTo("It's pretty hard to take inventory when you can't see.\n\r");
      return;
    }
    sendTo("You are carrying:\n\r");
    list_in_heap(stuff, this, 0, 100);

    if (GetMaxLevel() > 10) {
      sendTo("\n\r%d%% volume, %d%% weight.\n\r",
              getCarriedVolume() * 100 / carryVolumeLimit(),
              (int) (getCarriedWeight() * 100.0 / carryWeightLimit()));
    }
  } else {
    victim = get_char_vis_world(this, arg, NULL, EXACT_YES);
    if (!victim)
      victim = get_char_vis_world(this, arg, NULL, EXACT_NO);

    if (victim) {
      act("$N is carrying:", FALSE, this, NULL, victim, TO_CHAR);
      list_in_heap(victim->stuff, this, 1, 100);
    }
  }
}

void TBeing::doEquipment(const char *argument)
{
  wearSlotT j;
  int found;
  char capbuf[80], buf[80], trans[80];
  TThing *t;

  one_argument(argument, buf);
  if (is_abbrev(buf, "damaged") || is_abbrev(buf, "all.damaged")) {
    sendTo("The following equipment is damaged:\n\r");
    found = FALSE;
    for (j = MIN_WEAR; j < MAX_WEAR; j++) {
      TThing *tt = equipment[j];
      TObj *tobj = dynamic_cast<TObj *>(tt);
      if (tobj && tobj->getMaxStructPoints() != tobj->getStructPoints()) {
        if (!tobj->shouldntBeShown(j)) {
          sprintf(buf, "<%s>", describeEquipmentSlot(j).c_str());
            sendTo("%s%-25s%s", cyan(), buf, norm());
            if (canSee(tobj)) {
            showTo(tobj, SHOW_MODE_SHORT_PLUS);
            found = TRUE;
          } else {
            sendTo("Something.\n\r");
            found = TRUE;
          }
        }
      }
    }
  } else if (!*argument || !isImmortal()) {
    sendTo("You are using:\n\r");
    found = FALSE;
    for (j = MIN_WEAR; j < MAX_WEAR; j++) {
      if (equipment[j]) {
        if (!equipment[j]->shouldntBeShown(j)) {
          sprintf(buf, "<%s>", describeEquipmentSlot(j).c_str());
          sendTo("%s%-25s%s", cyan(), buf, norm());
          if (canSee(equipment[j])) {
            showTo(equipment[j], SHOW_MODE_SHORT_PLUS);
            found = TRUE;
          } else {
            sendTo("Something.\n\r");
            found = TRUE;
          }
        }
      }
    }
  } else {
    // allow immortals to get eq of players
    TBeing *victim = get_char_vis_world(this, argument, NULL, EXACT_YES);
    if (!victim)
      victim = get_char_vis_world(this, argument, NULL, EXACT_NO);

    if (victim) {
      act("$N is using.", FALSE, this, 0, victim, TO_CHAR);
      found = FALSE;
      for (j = MIN_WEAR; j < MAX_WEAR; j++) {
        if (victim->equipment[j]) {
          if (!victim->equipment[j]->shouldntBeShown(j)) {
            sprintf(buf, "<%s>", victim->describeEquipmentSlot(j).c_str());
            sendTo("%s%-25s%s", cyan(), buf, norm());
            if (canSee(victim->equipment[j])) {
              showTo(victim->equipment[j], SHOW_MODE_SHORT_PLUS);
              found = TRUE;
            } else {
              sendTo("Something.\n\r");
              found = TRUE;
            }
          }
        }
      }
    } else 
      sendTo("No such character exists.\n\r");

    return;
  }
  if (!found)
    sendTo("Nothing.\n\r");

  for (j = MIN_WEAR; j < MAX_WEAR; j++) {
    if (j == HOLD_RIGHT || j == HOLD_LEFT)
      continue;
    if (!slotChance(j))
      continue;
    if (isLimbFlags(j, PART_TRANSFORMED)){
      switch (j) {
        case WEAR_FINGER_R:
        case WEAR_FINGER_L:
          break;
        case WEAR_NECK:
          sprintf(trans, "<%s>", describeTransLimb(j).c_str());
          sendTo("%s%s%s\n\r", cyan(), trans, norm());
          break;
        case WEAR_BODY:
          break;
        case WEAR_HEAD:
          sprintf(trans, "<%s>", describeTransLimb(j).c_str());
          sendTo("%s%s%s\n\r", cyan(), trans, norm());
          break;
        case WEAR_LEGS_L:
          break;
        case WEAR_LEGS_R:
          sprintf(trans, "<%s>", describeTransLimb(j).c_str());
          sendTo("%s%s%s\n\r", cyan(), trans, norm());
          break;
        case WEAR_FOOT_R:
        case WEAR_FOOT_L:
          break;
        case WEAR_HAND_R:
          if (isLimbFlags(WEAR_ARM_R, PART_TRANSFORMED))
            break;
          sprintf(trans, "<%s>", describeTransLimb(j).c_str());
          sendTo("%s%s%s\n\r", cyan(), trans, norm());
          break;
        case WEAR_HAND_L:
          if (isLimbFlags(WEAR_ARM_L, PART_TRANSFORMED))
            break;
          sprintf(trans, "<%s>", describeTransLimb(j).c_str());
          sendTo("%s%s%s\n\r", cyan(), trans, norm());
          break;
        case WEAR_ARM_R:
          sprintf(trans, "<%s>", describeTransLimb(j).c_str());
          sendTo("%s%s%s\n\r", cyan(), trans, norm());
          break;
        case WEAR_ARM_L:
          sprintf(trans, "<%s>", describeTransLimb(j).c_str());
          sendTo("%s%s%s\n\r", cyan(), trans, norm());
          break;
        case WEAR_BACK :
        case WEAR_WAISTE:
        case WEAR_WRIST_R:
        case WEAR_WRIST_L:
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case WEAR_EX_FOOT_R:
        case WEAR_EX_FOOT_L:
          break;
        default:
          break;
      }
    }
    if ((t = getStuckIn(j))) {
      if (canSee(t)) {
        strcpy(capbuf, t->getName());
        sendTo(COLOR_OBJECTS, "%s is sticking out of your %s!\n\r", cap(capbuf), describeBodySlot(j).c_str());
      }
    }
  }
}


void TBeing::doCredits()
{
  if (desc)
    desc->start_page_file(CREDITS_FILE, "Credits file being revised!\n\r");
}


void TBeing::doNews()
{
  if (desc) {
    news_used_num++;
    desc->start_page_file(NEWS_FILE, "No news is good news!\n\r");
  }
}

void TBeing::doWizlist()
{
  if (desc) {
    FILE   *tFile;
    string  tStString("");

    if (!(tFile = fopen(WIZLIST_FILE, "r")))
      sendTo("Sorry, wizlist under construction!\n\r");
    else {
      wizlist_used_num++;

      file_to_string(WIZLIST_FILE, tStString, true);
      desc->page_string(tStString.c_str(), 0);
      fclose(tFile);
    }
  }
}

static int whichNumberMobile(const TThing *mob)
{
  TBeing *i;
  string name;
  int iNum;

  name = fname(mob->name);
  for (i = character_list, iNum = 0; i; i = i->next) {
    if (isname(name.c_str(), i->name) && i->in_room != ROOM_NOWHERE) {
      iNum++;
      if (i == mob)
        return iNum;
    }
  }
  return 0;
}

static const string numbered_person(const TBeing *ch, const TThing *person)
{
  char buf[256];

  if (dynamic_cast<const TMonster *>(person) && ch->isImmortal())
    sprintf(buf, "%d.%s", whichNumberMobile(person), fname(person->name).c_str());
  else
    strcpy(buf, ch->pers(person));

  return buf;
}

void do_where_thing(const TBeing *ch, const TThing *obj, bool recurse, string &sb)
{
  char buf[256];

  if (obj->in_room != ROOM_NOWHERE) {       // object in a room 
    sprintf(buf, "%-30s- ",
           obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%-35s [%d]\n\r",
           obj->roomp->getNameNOC(ch).c_str(), obj->in_room);
// object carried by monster
 } else if (dynamic_cast<TBeing *>(obj->parent) && obj->parent->roomp) {
    sprintf(buf, "%-30s- carried by %s -", obj->getNameNOC(ch).c_str(), 
               numbered_person(ch, obj->parent).c_str());
    sprintf(buf + strlen(buf), " %-20s [%d]\n\r",
               (obj->parent->roomp->name ? obj->parent->roomp->getNameNOC(ch).c_str() : "Room Unknown"), 
               obj->parent->in_room);
  } else if (dynamic_cast<TBeing *>(obj->parent) && obj->parent->riding && obj->parent->riding->roomp) {
    sprintf(buf, "%-30s- carried by %s - ", 
               obj->getNameNOC(ch).c_str(), 
               numbered_person(ch, obj->parent).c_str());
    sprintf(buf + strlen(buf), "riding %s - ", 
               obj->parent->riding->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r", 
               (obj->parent->riding->roomp->name ? obj->parent->riding->roomp->getNameNOC(ch).c_str() : "Room Unknown"),
               obj->parent->riding->in_room);
  } else if (dynamic_cast<TBeing *>(obj->parent) && obj->parent->riding) {
    sprintf(buf, "%-30s- carried by %s - ",
               obj->getNameNOC(ch).c_str(), numbered_person(ch, obj->parent).c_str());
    sprintf(buf + strlen(buf), "riding %s - (Room Unknown)\n\r",
               obj->parent->riding->getNameNOC(ch).c_str());
  } else if (dynamic_cast<TBeing *>(obj->parent)) {  // object carried by monster 
    sprintf(buf, "%-30s- carried by %s (Room Unknown)\n\r", obj->getNameNOC(ch).c_str(), 
               numbered_person(ch, obj->parent).c_str());
// object equipped by monster
  } else if (obj->equippedBy && obj->equippedBy->roomp) {
    sprintf(buf, "%-30s- equipped by %s - ", obj->getNameNOC(ch).c_str(), 
               numbered_person(ch, obj->equippedBy).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r", 
               (obj->equippedBy->roomp->name ? obj->equippedBy->roomp->getNameNOC(ch).c_str() : "Room Unknown"), 
               obj->equippedBy->in_room);
  } else if (obj->equippedBy) {       // object equipped by monster 
    sprintf(buf, "%-30s- equipped by %s (Room Unknown)\n\r", obj->getNameNOC(ch).c_str(), 
               numbered_person(ch, obj->equippedBy).c_str());
  } else if (obj->stuckIn && obj->stuckIn->roomp) {
    sprintf(buf, "%-20s- stuck in %s - ",
               obj->getNameNOC(ch).c_str(), 
               numbered_person(ch, obj->stuckIn).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r",
               (obj->stuckIn->roomp->name ? obj->stuckIn->roomp->getNameNOC(ch).c_str() : "Room Unknown"), 
               obj->stuckIn->in_room);
  } else if (obj->stuckIn) {
    sprintf(buf, "%-20s- stuck in %s - (Room Unknown)\n\r", obj->getNameNOC(ch).c_str(), 
               numbered_person(ch, obj->stuckIn).c_str());
// object in object
  } else if (obj->parent && obj->parent->parent) {
    sprintf(buf, "%-30s- ",
               obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "in %s - ",
               obj->parent->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "in %s - ",
               obj->parent->parent->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r", 
            ((obj->parent->parent->roomp && obj->parent->parent->roomp->name) ?
             obj->parent->parent->roomp->getNameNOC(ch).c_str() : "Room Unknown"),
               obj->parent->parent->in_room);
  } else if (obj->parent && obj->parent->equippedBy) {
    sprintf(buf, "%-30s- ",
               obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "in %s - ",
               obj->parent->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "equipped by %s - ",
               obj->parent->equippedBy->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r", 
    ((obj->parent->equippedBy->roomp && obj->parent->equippedBy->roomp->name) ?
       obj->parent->equippedBy->roomp->getNameNOC(ch).c_str() : "Room Unknown"),
       obj->parent->equippedBy->in_room);
  } else if (obj->parent && obj->parent->stuckIn) {
    sprintf(buf, "%-30s- ",
               obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "in %s - ",
               obj->parent->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "stuck in %s - ",
               obj->parent->stuckIn->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r", 
       ((obj->parent->stuckIn->roomp && obj->parent->stuckIn->roomp->name) ?
        obj->parent->stuckIn->roomp->getNameNOC(ch).c_str() : "Room Unknown"),
        obj->parent->stuckIn->in_room);
// object in object 
  } else if (obj->parent) {
    sprintf(buf, "%-30s- in ",
               obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s - ",
               obj->parent->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r",
               obj->parent->roomp ? obj->parent->roomp->getNameNOC(ch).c_str() : "(Room Unkown)",
               obj->parent->in_room);
  } else if (obj->riding) {
    sprintf(buf, "%-30s- on ",
               obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s - ",
               obj->riding->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r",
               obj->riding->roomp ? obj->riding->roomp->getNameNOC(ch).c_str() : "(Room Unkown)",
               obj->riding->in_room);
  } else {
    sprintf(buf, "%-30s- god doesn't even know where...\n\r", obj->getNameNOC(ch).c_str());
  }
  if (*buf)
    sb += buf;

  if (recurse) {
    if (obj->in_room != ROOM_NOWHERE)
      return;
    else if (dynamic_cast<TBeing *>(obj->parent))
      do_where_thing(ch, obj->parent, TRUE, sb);
    else if (obj->equippedBy)
      do_where_thing(ch, obj->equippedBy, TRUE, sb);
    else if (obj->stuckIn)
      do_where_thing(ch, obj->stuckIn, TRUE, sb);
    else if (obj->parent)
      do_where_thing(ch, obj->parent, TRUE, sb);
    else if (obj->riding)
      do_where_thing(ch, obj->riding, TRUE, sb);
  }
}

void TBeing::doWhere(const char *argument)
{
  char namebuf[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char *nameonly;
  register TBeing *i, *ch;
  register TObj *k;
  Descriptor *d;
  int iNum, count;
  string sb;
  bool dash = FALSE;
  bool gods = FALSE;

  if (powerCheck(POWER_WHERE))
    return;

  only_argument(argument, namebuf);

  if (!*namebuf || (dash = (*namebuf == '-'))) {
    if (GetMaxLevel() <= MAX_MORT) {
      sendTo("What are you looking for?\n\r");
      return;
    } else {
      if (dash) {
        if (!strchr(namebuf, 'g')) {
          sendTo("Syntax : where -g\n\r");
          return;
        }
        gods = TRUE;
      }
      sb += "Players:\n\r--------\n\r";

      for (d = descriptor_list; d; d = d->next) {
        if ((ch = d->character) && canSeeWho(ch) &&
            (ch->in_room != ROOM_NOWHERE) &&
            (!gods || ch->isImmortal())) {
          if (d->original)
            sprintf(buf, "%-20s - %s [%d] In body of %s\n\r",
                    d->original->getNameNOC(ch).c_str(), ch->roomp->name,
                    ch->in_room, ch->getNameNOC(ch).c_str());
          else
            sprintf(buf, "%-20s - %s [%d]\n\r", ch->getNameNOC(ch).c_str(),
                    ch->roomp ? ch->roomp->name : "Bad room", ch->in_room);

          sb += buf;
        }
      }
      if (desc)
        desc->page_string(sb.c_str(), 0, TRUE);
      return;
    }
  }
  if (isdigit(*namebuf)) {
    nameonly = namebuf;
    count = iNum = get_number(&nameonly);
  } else
    count = iNum = 0;

  *buf = '\0';

  unsigned int tot_found = 0;
  for (i = character_list; i; i = i->next) {
    if (!i->name) {
      vlogf(5, "Being without a name in character_list (doWhere) looking for %s", namebuf);
      continue;
    }
    if (isname(namebuf, i->name) && canSeeWho(i) && canSee(i)) {
      if ((i->in_room != ROOM_NOWHERE) && (isImmortal() || (i->roomp->getZone() == roomp->getZone()))) {
        if (!iNum || !(--count)) {
          if (!iNum) {
            sprintf(buf, "[%2d] ", ++count);
            sb += buf;
          }
          if (++tot_found > 500) {
            sb += "Too many creatures found.\n\r";
            break;
          }
          
          do_where_thing(this, i, TRUE, sb);
          *buf = 1;
          if (iNum != 0)
            break;
        }
        if (GetMaxLevel() <= MAX_MORT)
          break;
      }
    }
  }
  if (GetMaxLevel() > MAX_MORT) {
    for (k = object_list; k; k = k->next) {
      if (!k->name) {
        vlogf(5, "Item without a name in object_list (doWhere) looking for %s", namebuf);
        continue;
      }
      if (isname(namebuf, k->name) && canSee(k)) {
        if (!iNum || !(--count)) {
          if (!iNum) {
            sprintf(buf, "[%2d] ", ++count);
            sb += buf;
          }
          if (++tot_found > 500) {
            sb += "Too many objects found.\n\r";
            break;
          }
          do_where_thing(this, k, iNum != 0, sb);
          *buf = 1;
          if (iNum != 0)
            break;
        }
      }
    }
  }
  if (sb.empty())
    sendTo("Couldn't find any such thing.\n\r");
  else {
    if (desc)
      desc->page_string(sb.c_str(), 0, TRUE);
  }

}

extern void comify(char *);

void TBeing::doLevels(const char *argument)
{
#if 0
  int       tIndex,
            tValueA,
            tValueB,
            tValueC,
            tValueD,
            tDents[4];
  char      tString[256],
            tBuffer[256];
  string    tStString("");
  classIndT tClass     = MAGE_LEVEL_IND;
  bool      tRemaining = false;

  if (argument && *argument && is_abbrev(argument, "left"))
    tRemaining = true;

  for (tIndex = 1; tIndex <= (MAX_MORT / 4) + 1; tIndex++) {
    tValueA = (int) getExpClassLevel(tClass, (tDents[0] = (tIndex + 0 * (MAX_MORT / 4 + 1))));
    tValueB = (int) getExpClassLevel(tClass, (tDents[1] = (tIndex + 1 * (MAX_MORT / 4 + 1))));
    tValueC = (int) getExpClassLevel(tClass, (tDents[2] = (tIndex + 2 * (MAX_MORT / 4 + 1))));
    tValueD = (int) getExpClassLevel(tClass, (tDents[3] = (tIndex + 3 * (MAX_MORT / 4 + 1))));

    if (tRemaining) {
      tValueA = max(0, ((int) getExp() - tValueA));
      tValueB = max(0, ((int) getExp() - tValueB));
      tValueC = max(0, ((int) getExp() - tValueC));
      tValueD = max(0, ((int) getExp() - tValueD));
    }

    for (int tTemp = 0; tTemp < 4; tTemp++)
      if (tDents[tTemp] <= MAX_MORT) {
        sprintf(tBuffer, "%d", tValueA);
        comify(tBuffer);
        sprintf(tString, "%s[%2d]%s %s%13s%s",
                cyan(), tDents[tTemp], norm(),
                (tRemaining ? (tValueA ? orange() : green()) :
                 ((tDents[tTemp] > GetMaxLevel()) ? orange() : green())),
                tString, norm());
        tStString += tString;
      }
    tStString += "\n\r";
  }

  tStString += "\n\r";

  if (desc)
    desc->page_string(tStString.c_str(), 0, TRUE);
#else
  int i;
  classIndT Class;
// int RaceMax;
  string sb;
  char buf[256],
       tString[256];

  for (; isspace(*argument); argument++);

  if (!*argument) {
    if (isSingleClass()) {
      int num = CountBits(getClass()) - 1;
      if (num < MIN_CLASS_IND || num >= MAX_CLASSES) {
        return;
      }
      Class = classIndT(num);
    } else {
      sendTo("You must supply a class!\n\r");
      return;
    }
  } else if (is_abbrev(argument, "mage"))
    Class = MAGE_LEVEL_IND;
  else if (is_abbrev(argument, "monk"))
    Class = MONK_LEVEL_IND;
  else {
    switch (*argument) {
      case 'C':
      case 'c':
        Class = CLERIC_LEVEL_IND;
        break;
      case 'F':
      case 'f':
      case 'W':
      case 'w':
        Class = WARRIOR_LEVEL_IND;
        break;
      case 'T':
      case 't':
        Class = THIEF_LEVEL_IND;
        break;
      case 'R':
      case 'r':
        Class = RANGER_LEVEL_IND;
        break;
      case 'p':
      case 'P':
      case 'd':     // deikhan
      case 'D':
        Class = DEIKHAN_LEVEL_IND;
        break;
      case 's':     // shaman 
      case 'S':
        Class = SHAMAN_LEVEL_IND;
        break;

      default:
        sendTo("I don't recognize %s\n\r", argument);
        return;
        break;
    }
  }
  //RaceMax = RacialMax[race->getRace()][Class];

  sprintf(buf, "Experience needed for level in class %s:\n\r\n\r",
      classNames[Class].capName);
  sb += buf;

  ubyte cLvl = getLevel(Class);

  for (i = 1; i <= MAX_MORT/4 + 1; i++) {
    int j = i + 1*(MAX_MORT/4+1);
    int k = i + 2*(MAX_MORT/4+1);
    int m = i + 3*(MAX_MORT/4+1);

    if (i <= MAX_MORT) {
      sprintf(tString, "%d", (int) getExpClassLevel(Class, i));
      comify(tString);
      sprintf(buf, "%s[%2d]%s %s%13s%s%s", 
            cyan(), i, norm(),
            ((i > cLvl) ? orange() : green()), tString, norm(),
            " ");
      sb += buf;
    }
    if (j <= MAX_MORT) {
      sprintf(tString, "%d", (int) getExpClassLevel(Class, j));
      comify(tString);
      sprintf(buf, "%s[%2d]%s %s%13s%s%s",
            cyan(), j, norm(),
            ((j > cLvl) ? orange() : green()), tString, norm(),
              " ");
      sb += buf;
    }
    if (k <= MAX_MORT) {
      sprintf(tString, "%d", (int) getExpClassLevel(Class, k));
      comify(tString);
      sprintf(buf, "%s[%2d]%s %s%13s%s%s",
            cyan(), k, norm(),
            ((k > cLvl) ? orange() : green()), tString, norm(),
            " ");
      sb += buf;
    }
    if (m <= MAX_MORT) {
      sprintf(tString, "%d", (int) getExpClassLevel(Class, m));
      comify(tString);
      sprintf(buf, "%s[%2d]%s %s%13s%s%s",
            cyan(), m, norm(),
            ((m > cLvl) ? orange() : green()), tString, norm(),
            "\n\r");
      sb += buf;
    } else {
      sb += "\n\r";
    }
  }
  sb += "\n\r";
  if (desc)
    desc->page_string(sb.c_str(), 0, TRUE);
  return;
#endif
}


void TBeing::doConsider(const char *argument)
{
  TBeing *victim;
  char namebuf[256];
  int diff=0;

  only_argument(argument, namebuf);

  if (!(victim = get_char_room_vis(this, namebuf))) {
    if (!isImmortal() || !hasWizPower(POWER_IMM_EVAL)) {
      sendTo("Consider killing whom?\n\r");
      return;
    } else if (!(victim = get_char_vis_world(this, namebuf, NULL, EXACT_YES)) &&
               !(victim = get_char_vis_world(this, namebuf, NULL, EXACT_NO))) {
      sendTo("I'm afraid I was unable to find them.\n\r");
      return;
    } else if (!dynamic_cast<TPerson *>(victim)) {
      sendTo("I'm afraid you can only use this on mortals in this fashion.\n\r");
      return;
    }
  }
  if (!canSee(victim) && canSee(victim, INFRA_YES)) {
    strcpy(namebuf, "a blob");
  } else {
    strcpy(namebuf, victim->getName());
  }
  if (victim == this) {
    if (!isImmortal()) {
      sendTo("You consider your equipment...\n\r");
      int armor = 1000 - getArmor();
      sh_int suggest = suggestArmor();
      diff = (int) (suggest - armor);
      sendTo("Your equipment would seem %s for your class and level.\n\r",
             (diff >=  210 ? "laughably pathetic" :
             (diff >=  160 ? "horrid" :
             (diff >=   90 ? "bad" :
             (diff >=   50 ? "poor" :
             (diff >=   30 ? "weak" :
             (diff >=   10 ? "o.k." :
             (diff >     0 ? "near perfect" :
             (diff ==    0 ? "perfect" :
             (diff >= - 30 ? "good" :
             (diff >= - 40 ? "very good" :
             (diff >= - 80 ? "great" :
             (diff >= -150 ? "fantastic" :
             (diff >= -200 ? "superb" : "incredibly good"))))))))))))));
      return;
    } else {
      sendTo("You're funny...  You're a god, what do you need armor for??\n\r");
      return;
    }
  }
  if (victim->isPc() && victim->isImmortal()) {
    sendTo("You must sure have a big ego to contemplate fighting gods.\n\r");
    act("$N just considered fighting you.",TRUE,victim,0,this,TO_CHAR);
    return;
  } else if (dynamic_cast<TPerson *>(victim)) {
    if (isImmortal() && hasWizPower(POWER_IMM_EVAL)) {
      diff       = (int) (victim->getArmor());
      sh_int suggest = victim->suggestArmor();
      int prefArmorC = (1000 - suggest);

      sendTo("You consider %s's equipment...\n\r", victim->getName());
      sendTo("%s should have an AC of [%d] but has an AC of [%d]\n\r",
             good_cap(victim->getName()).c_str(),
             prefArmorC,
             diff);
      return;
    } else {
      sendTo("Would you like to borrow a cross and a shovel?\n\r");
      playsound(SOUND_DONT_KILL_ME, SOUND_TYPE_COMBAT);
      return;
    }
  }

  // everything should be a monster by this point
  TMonster *tmon = dynamic_cast<TMonster *>(victim);

  act("$n looks $N over.", TRUE, this, 0, tmon, TO_NOTVICT);
  act("$n looks you over.", TRUE, this, 0, tmon, TO_VICT);

#if 0
  diff = tmon->GetMaxLevel() - GetMaxLevel();
#else
  // let's use the present real lev so we look at spells and stuff
  diff = (int) (tmon->getRealLevel() + 0.5) - GetMaxLevel();
#endif
  if (diff <= -15)
    sendTo("Shall I tie both hands behind your back, or just one?\n\r");
  else if (diff <= -10)
    sendTo("Why bother???\n\r");
  else if (diff <= -6)
    sendTo("Don't strain yourself.\n\r");
  else if (diff <= -3)
    sendTo("Piece of cake.\n\r");
  else if (diff <= -2)
    sendTo("Odds are in your favor.\n\r");
  else if (diff <= -1)
    sendTo("You have a slight advantage.\n\r");
  else if (!diff)
    sendTo("A fair fight.\n\r");
  else if (diff <= 1)
    act("$E doesn't look that tough...", TRUE, this, 0, tmon, TO_CHAR);
  else if (diff <= 2)
    sendTo("Cross your fingers.\n\r");
  else if (diff <= 3)
    sendTo("Cross your fingers and hope they don't get broken.\n\r");
  else if (diff <= 6)
    sendTo("I hope you have a good plan!\n\r");
  else if (diff <= 10)
    sendTo("Bring friends.\n\r");
  else if (diff <= 15)
    sendTo("You and what army??\n\r");
  else if (diff <= 30)
    act("You'll win if $E never hits you.", TRUE, this, 0, tmon, TO_CHAR);
  else
    sendTo("There are better ways to suicide.\n\r");

  if (getDiscipline(DISC_ADVENTURING)) {
    int learn = 0;
    spellNumT sknum = TYPE_UNDEFINED;
    int roll = 0;

    if (tmon->isAnimal() && doesKnowSkill(SKILL_CONS_ANIMAL)) {
      sknum = SKILL_CONS_ANIMAL;
      roll = 1;
      learn = getSkillValue(SKILL_CONS_ANIMAL);
      sendTo(COLOR_MOBS, "Using your knowledge of animal lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isVeggie() && doesKnowSkill(SKILL_CONS_VEGGIE)) {
      sknum = SKILL_CONS_VEGGIE;
      roll = 1;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_VEGGIE));
      sendTo(COLOR_MOBS, "Using your knowledge of vegetable lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isDiabolic() && doesKnowSkill(SKILL_CONS_DEMON)) {
      sknum = SKILL_CONS_DEMON;
      roll = 1;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_DEMON));
      sendTo(COLOR_MOBS, "Using your knowledge of demon lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isReptile() && doesKnowSkill(SKILL_CONS_REPTILE)) {
      sknum = SKILL_CONS_REPTILE;
      roll = 2;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_REPTILE));
      sendTo(COLOR_MOBS, "Using your knowledge of reptile lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isUndead() && doesKnowSkill(SKILL_CONS_UNDEAD)) {
      sknum = SKILL_CONS_UNDEAD;
      roll = 2;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_UNDEAD));
      sendTo(COLOR_MOBS, "Using your knowledge of the undead, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isGiantish() && doesKnowSkill(SKILL_CONS_GIANT)) {
      sknum = SKILL_CONS_GIANT;
      roll = 1;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_GIANT));
      sendTo(COLOR_MOBS, "Using your knowledge of giant lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isPeople() && doesKnowSkill(SKILL_CONS_PEOPLE)) {
      sknum = SKILL_CONS_PEOPLE;
      roll = 2;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_PEOPLE));
      sendTo(COLOR_MOBS, "Using your knowledge of human and demi-human lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isOther() && doesKnowSkill(SKILL_CONS_OTHER)) {
      sknum = SKILL_CONS_OTHER;
      roll = 1;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_OTHER));
      sendTo(COLOR_MOBS, "Using your knowledge of monster lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (learn > MAX_SKILL_LEARNEDNESS)
      learn = MAX_SKILL_LEARNEDNESS;

    if (!learn)
      return;

    if ((GetMaxLevel() <= MAX_MORT) && !::number(0,roll)) {
      learnFromDoing(sknum, SILENT_NO, 0);
    }

    addToWait(combatRound(1));

    int num = GetApprox(tmon->hitLimit(), max(80,learn));
    double fnum = hitLimit() ? ((double) num / (double) hitLimit()) : 
            (num ? 10.0 : 0.0);
    if (learn > 5)
      sendTo("Est Max HP are: %s.\n\r", DescRatio(fnum));

    // mob's AC : expressed as a level (use mob scale for AC-lev)
    num = GetApprox(max(((1000 - tmon->getArmor()) - 400)/20, 0), 10*max(80,learn));
    // my AC : expressed as a level  (use PC scale for AC-lev)
    int num2 = (int) max(((1000 - getArmor()) - 500)/25, 0);
    // take the difference
    num -= num2;  // if positive, mob has better armor
    // normalize it
    fnum = ((double) num/get_doubling_level(GetMaxLevel()));
    // if same ACs, num = 0 and want it to be 1.0
    // if mob had AC of twice as good mob, num = 1, want it to be 2.0
    // if mob had AC of 4X as good mob, num = 2, want it to be 4.0
    // if mob had AC of twice as bad mob, num = -1, want it to be 0.5
    fnum = pow(2.0, fnum);

    if (learn > 20)
      sendTo("Est. armor class is : %s.\n\r", DescRatio(fnum));

    if (learn > 40)
      sendTo("Est. # of attacks: %s.\n\r", DescAttacks(GetApprox((int) tmon->getMult(), max(80,learn))));

    if (learn > 60) {
      num = GetApprox((int) tmon->baseDamage(), max(80,learn));
      fnum = (double) num;
      sendTo("Est. damage of attacks: %s.\n\r", DescDamage(fnum));
    }
  }
  immortalEvaluation(tmon);
}


int offset;

int compar(const void *i, const void *j) 
{ 
  return(strcmp((const char *) i + offset, (const char *) j + offset)); 
}

void TBeing::doWorld()
{
  time_t ct, ot, tt;
  char *tmstr, *otmstr;
  int i;

  ot = Uptime;
  if (desc && desc->account)
    tt = ot + 3600 * desc->account->time_adjust;
  else
    tt = ot;

  otmstr = asctime(localtime(&tt));
  *(otmstr + strlen(otmstr) - 1) = '\0';
  sendTo("%sStart time was:                      %s%s\n\r", 
        blue(),otmstr, norm());

  if (desc && desc->account)
    ct = time(0) + 3600 * desc->account->time_adjust;
  else
    ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sendTo("%sCurrent time is:                     %s%s\n\r", 
         blue(), tmstr, norm());

  time_t upt = ct - tt;
  sendTo("%sUptime is:                           %s%s\n\r", 
         blue(), secsToString(upt).c_str(), norm());

  sendTo("%sMachine Lag: Avg/Cur/High/Low        %d/%d/%d/%d%s\n\r",
	 blue(), 
	 (lag_info.count ? lag_info.total/lag_info.count : 0),
	 lag_info.current,
	 lag_info.high,
	 lag_info.low, norm());

  if(isImmortal()){
    for(i=0;i<10;++i){
      if(lag_info.lagcount[i])
	sendTo("%sLag %i:                              %d/%d%s\n\r",
	       blue(), i, lag_info.lagtime[i], lag_info.lagcount[i], norm());
    }
  }


  sendTo("Total number of rooms in world:               %d\n\r", 
        roomCount);
  sendTo("Total number of zones in world:               %d\n\r", 
        zone_table.size());
  sendTo("Total number of distinct objects in world:%s    %d%s\n\r",
        green(), obj_index.size(), norm());
  sendTo("Total number of objects in game:%s              %d%s\n\r",
        green(), objCount, norm());
  sendTo("Total number of registered accounts:%s          %d%s\n\r", 
         blue(), account_number, norm());
  sendTo("Total number of registered players:%s           %d%s\n\r", 
         blue(), player_count, norm());

  char timebuf[256];

  strcpy(timebuf, ctime(&stats.first_login));
  timebuf[strlen(timebuf) - 1] = '\0';
  strcat(timebuf, ":");
  sendTo("Logins since %-32.32s %s%ld  (%ld per day)%s\n\r",
     timebuf,  blue(), stats.logins, 
     (long) ((double) stats.logins * SECS_PER_REAL_DAY / (time(0) - stats.first_login)),
     norm());

  sendTo("Total number of distinct mobiles in world:%s    %d%s\n\r",
        red(), mob_index.size(), norm());

  if (GetMaxLevel() >= GOD_LEVEL1) {
    sendTo("%sDistinct Mobs by level:%s\n\r",
         blue(), norm());
    sendTo("%sL1-5  [%s%3d%s]  L6-10 [%s%3d%s]  L11-15[%s%3d%s]  L16-20[%s%3d%s]  L21-25 [%s%3d%s] L26-30  [%s%3d%s]%s\n\r", norm(),
        purple(), stats.mobs_1_5, norm(),
        purple(), stats.mobs_6_10, norm(),
        purple(), stats.mobs_11_15, norm(),
        purple(), stats.mobs_16_20, norm(),
        purple(), stats.mobs_21_25, norm(),
        purple(), stats.mobs_26_30, norm(),
        norm());
    sendTo("%sL31-40[%s%3d%s]  L41-50[%s%3d%s]  L51-60[%s%3d%s]  L61-70[%s%3d%s]  L71-100[%s%3d%s] L101-127[%s%3d%s]%s\n\r", norm(),
        purple(), stats.mobs_31_40, norm(),
        purple(), stats.mobs_41_50, norm(),
        purple(), stats.mobs_51_60, norm(),
        purple(), stats.mobs_61_70, norm(),
        purple(), stats.mobs_71_100, norm(),
        purple(), stats.mobs_101_127, norm(),
        norm());
  }
  sendTo("Total number of monsters in game:%s             %d%s\n\r",
        red(), mobCount, norm());
  sendTo("%sActual Mobs by level:%s\n\r",
        purple(), norm());
  sendTo("%sL1-5  [%s%4u%s]  L6-10 [%s%3u%s]  L11-15[%s%3u%s]  L16-20[%s%3u%s]  L21-25 [%s%3u%s] L26-30  [%s%3u%s]%s\n\r", norm(),
        purple(), stats.act_1_5, norm(),
        purple(), stats.act_6_10, norm(),
        purple(), stats.act_11_15, norm(),
        purple(), stats.act_16_20, norm(),
        purple(), stats.act_21_25, norm(),
        purple(), stats.act_26_30, norm(),
        norm());
  sendTo("%sL31-40[%s%4u%s]  L41-50[%s%3u%s]  L51-60[%s%3u%s]  L61-70[%s%3u%s]  L71-100[%s%3u%s] L101-127[%s%3u%s]%s\n\r", norm(),
        purple(), stats.act_31_40, norm(),
        purple(), stats.act_41_50, norm(),
        purple(), stats.act_51_60, norm(),
        purple(), stats.act_61_70, norm(),
        purple(), stats.act_71_100, norm(),
        purple(), stats.act_101_127, norm(),
        norm());
}

const char *DescRatio(double f)
{                                // theirs / yours 
  if (f >= 4.0)
    return ("Way better than yours");
  else if (f > 3.0)
    return ("More than three times yours");
  else if (f > 2.0)
    return ("More than twice yours");
  else if (f > 1.5)
    return ("More than half again greater than yours");
  else if (f > 1.4)
    return ("At least a third greater than yours");
  else if (f > 1.0)
    return ("About the same as yours");
  else if (f > .9)
    return ("A little worse than yours");
  else if (f > .6)
    return ("Worse than yours");
  else if (f > .4)
    return ("About half as good as yours");
  else if (f > .2)
    return ("Much worse than yours");
  else if (f > .1)
    return ("Inferior");
  else
    return ("Extremely inferior");
}

const char *DescDamage(double dam)
{
  if (dam < 1.0)
    return ("Minimal Damage");
  else if (dam <= 2.0)
    return ("Slight damage");
  else if (dam <= 4.0)
    return ("A bit of damage");
  else if (dam <= 10.0)
    return ("A decent amount of damage");
  else if (dam <= 15.0)
    return ("A lot of damage");
  else if (dam <= 25.0)
    return ("A whole lot of damage");
  else if (dam <= 35.0)
    return ("A very large amount");
  else
    return ("A TON of damage");
}

const char *DescAttacks(double a)
{
  if (a < 1.0)
    return ("Not many");
  else if (a < 2.0)
    return ("About average");
  else if (a < 3.0)
    return ("A few");
  else if (a < 5.0)
    return ("A lot");
  else if (a < 9.0)
    return ("Many");
  else
    return ("A whole bunch");
}

const char *DescMoves(double a)
{
  if (a < .1)
    return ("very tired");
  else if (a < .3)
    return ("tired");
  else if (a < .5)
    return ("slightly tired");
  else if (a < .7)
    return ("decently rested");
  else if (a < 1.0)
    return ("well rested");
  else
    return ("totally rested");
}

const char *ac_for_score(int a)
{
  if (a > 750)
    return ("scantily clothed");
  else if (a > 500)
    return ("heavily clothed");
  else if (a > 250)
    return ("slightly armored");
  else if (a > 0)
    return ("moderately armored");
  else if (a > -250)
    return ("armored rather heavily");
  else if (a > -500)
    return ("armored very heavily");
  else if (a > -1000)
    return ("armored extremely heavily");
  else
    return ("totally armored");
}

void TBeing::doClear(const char *argument)
{
  int i;

  if (!desc)
    return;

  if (!*argument) {
    sendTo("Clear which alias?\n\r");
    return;
  } else
    i = atoi(argument) - 1;

  if ((i > -1) && (i < 16)) {
    desc->alias[i].command[0] = '\0';
    desc->alias[i].word[0] = '\0';
    sendTo("Ok. Alias %d now clear.\n\r", i + 1);
    return;
  } else {
    sendTo("Syntax :clear <alias number>\n\r");
    return;
  }
}

void TBeing::doAlias(const char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char spaces[20];
  int i;
  int remOption = FALSE;

  if (!desc)
    return;

  if (desc->client) {
    sendTo("Use the client aliases. See client help file for #alias command and
options menu\n\r");
    return;
  }

  for (i = 0; i < 19; i++)
    spaces[i] = ' ';

  spaces[19] = '\0';
  if (!*argument) {
    sendTo("Your list of aliases.....\n\r");
    for (i = 0; i < 16; i++) {
      sendTo("%2d) %s%s %s %s\n\r", i + 1, desc->alias[i].word,
            spaces + strlen(desc->alias[i].word),
            (ansi() ? ANSI_BLUE_BAR : "|"),
            desc->alias[i].command);
    }
    return;
  }
  half_chop(argument, arg1, arg2);
  if ((!arg2) || (!*arg2)) {
    sendTo("You need a second argument.\n\r");
    return;
  }
  if (!strcmp(arg2, "clear")) {
    sendTo("You could get in a loop like that!\n\r");
    return;
  }
  if (!strcmp(arg1, "clear")) 
    remOption = TRUE;
  
  if (!strcmp(arg1, arg2)) {
    sendTo("You could get in a loop like that!\n\r");
    return;
  }
  if (strlen(arg1) > 11) {
    sendTo("First argument is too long. It must be shorter than 12 characters.\n\r");
    return;
  }
  if (strlen(arg2) > 28) {
    sendTo("Second argument is too long. It must be less than 30 characters.\n\r");
    return;
  }
  i = 0;
  if (remOption) {
    while ((i < 16)) {
      if (*desc->alias[i].word && !strcmp(arg2, desc->alias[i].word)) {
        sendTo("Clearing alias %s\n\r", arg2);
        return;
      }
      i++;
    }
    sendTo("You have no alias for %s.\n\r", arg2);
    return;
  }

  i = -1;
  do {
    i++;
  }

  while ((i < 16) && *desc->alias[i].word && strcmp(arg1, desc->alias[i].word));
  if ((i == 16)) {
    sendTo("You have no more space for aliases. You will have to clear an alias before adding another one.\n\r");
    return;
  }
  strcpy(desc->alias[i].word, arg1);
  strcpy(desc->alias[i].command, arg2);
  sendTo("Setting alias %s to %s\n\r", arg1, arg2);
}

string TObj::equip_condition(int amt) const
{
  double p;

  if (!getMaxStructPoints()) {
    string a("<C>brand new<1>");
    return a;
  } else if (amt == -1)
    p = ((double) getStructPoints()) / ((double) getMaxStructPoints());
  else
    p = ((double) amt) / ((double) getMaxStructPoints());

  if (p == 1) {
    string a("<C>brand new<1>");
    return a;
  } else if (p > .9) {
    string a("<c>like new<1>");
    return a;
  } else if (p > .8) {
    string a("<B>excellent<1>");
    return a;
  } else if (p > .7) {
    string a("<b>very good<1>");
    return a;
  } else if (p > .6) {
    string a("<P>good<1>");
    return a;
  } else if (p > .5) {
    string a("<p>fine<1>");
    return a;
  } else if (p > .4) {
    string a("<G>fair<1>");
    return a;
  } else if (p > .3) {
    string a("<g>poor<1>");
    return a;
  } else if (p > .2) {
    string a("<y>very poor<1>");
    return a;
  } else if (p > .1) {
    string a("<o>bad<1>");
    return a;
  } else if (p > .001) {
    string a("<R>very bad<1>");
    return a;
  } else {
    string a("<r>destroyed<1>");
    return a;
  }
}

void TBeing::doMotd(const char *argument)
{
  for (; isspace(*argument); argument++);

  if (!desc)
    return;

  if (!*argument || !argument) {
    sendTo("Today's message of the day is :\n\r\n\r");
    desc->sendMotd(GetMaxLevel() > MAX_MORT);
    return;
#if 0
sendTo("Feature disabled, bug Batopr.\n\r");
  } else if (is_abbrev(argument, "message") && (GetMaxLevel() >= GOD_LEVEL4)) {
    TThing *t_note = searchLinkedListVis(this, "note", stuff);
    TObj *note = dynamic_cast<TObj *>(t_note);
    if (note) {
      if (!note->action_description) {
        sendTo("Your note has no message for the new motd!\n\r");
        return;
      } else {
        strcpy(motd, note->action_description);
        return;
      }
    } else {
      sendTo("You need a note with what you want the message to be in you inventory.\n\r");
      return;
    }
#endif
  }
}

const char *LimbHealth(double a)
{
  if (a < .1)
    return ("<R>in extremely bad shape<Z>");
  else if (a < .3)
    return ("<o>badly beaten<z>");
  else if (a < .5)
    return ("<O>moderately wounded<z>");
  else if (a < .7)
    return ("<g>mildly wounded<z>");
  else if (a < .99)
    return ("<g>slightly wounded<z>");
  else if (a >= 1.00)
    return ("<c>in perfect condition<Z>");
  else
    return ("in near perfect condition");
}

const string TBeing::slotPlurality(int limb) const
{
  char buf[10];

  if ((race->getBodyType() == BODY_BIRD) &&
      (limb == WEAR_WAISTE)) {
    // tail feathers
    sprintf(buf, "are");
  } else if ((race->getBodyType() == BODY_TREE) &&
      ((limb == WEAR_ARM_R) ||
       (limb == WEAR_ARM_L))) {
    // branches
    sprintf(buf, "are");
  } else
    sprintf(buf, "is");

  return buf;
}

void TBeing::doLimbs(const string & argument)
{
  wearSlotT i;
  char buf[512], who[5];
  affectedData *aff;
  TThing *t;
  TBeing *v=NULL;

  *buf = '\0';


  if(!argument.empty()) {
    only_argument(argument.c_str(), buf);
    if (!(v = get_char_room_vis(this, buf))) {
      if (!(v = fight())) {
	sendTo("Check whose limbs?\n\r");
	return;
      }
    }
    if (!sameRoom(v)) {
      sendTo("That person doesn't seem to be around.\n\r");
      return;
    }
  }

  if(!v || v==this){
    v=this;
    strcpy(who, "Your");
    sendTo("You evaluate your limbs and their health.\n\r");
  } else {
    strncpy(who, v->hshr(), 5);
    cap(who);
    sendTo(COLOR_BASIC, "You evaluate %s's limbs and their health.\n\r", v->getName());
  }


  bool found = false;
  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (i == HOLD_RIGHT || i == HOLD_LEFT)
      continue;
    if (!v->slotChance(i))
      continue;
    double perc = (double) v->getCurLimbHealth(i) / (double) v->getMaxLimbHealth(i);

    if (v->isLimbFlags(i, PART_MISSING)) {
      sendTo(COLOR_BASIC, "<R>%s %s%s%s %s missing!<Z>\n\r", who, red(), v->describeBodySlot(i).c_str(), norm(), v->slotPlurality(i).c_str());
      found = TRUE;
      continue;
    } 
    if (perc < 1.00) {
      sendTo(COLOR_BASIC, "%s %s %s %s.\n\r", who, v->describeBodySlot(i).c_str(), v->slotPlurality(i).c_str(), LimbHealth(perc));
      found = TRUE;
    } 
    if (v->isLimbFlags(i, PART_USELESS)) {
      sendTo(COLOR_BASIC, "%s %s%s%s %s <O>useless<Z>!\n\r",who, red(),v->describeBodySlot(i).c_str(),norm(), v->slotPlurality(i).c_str());
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_PARALYZED)) {
      sendTo(COLOR_BASIC, "%s %s%s%s %s <O>paralyzed<Z>!\n\r",
         who,red(),v->describeBodySlot(i).c_str(),
         norm(), v->slotPlurality(i).c_str());
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_BLEEDING)) {
      sendTo(COLOR_BASIC, "%s %s%s%s %s <R>bleeding profusely<Z>!\n\r",
         who,red(),v->describeBodySlot(i).c_str(),
         norm(), v->slotPlurality(i).c_str());
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_INFECTED)) {
      sendTo(COLOR_BASIC, "%s %s%s%s %s infected with many germs!\n\r",
         who,red(),v->describeBodySlot(i).c_str(),
         norm(), v->slotPlurality(i).c_str());
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_BROKEN)) {
      sendTo(COLOR_BASIC, "%s %s%s%s %s broken!\n\r",
         who,red(),v->describeBodySlot(i).c_str(),
         norm(), v->slotPlurality(i).c_str());
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_LEPROSED)) {
      sendTo(COLOR_BASIC, "Leprosy has set into %s %s%s%s!\n\r",
          v->hshr(), red(),v->describeBodySlot(i).c_str(),norm());
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_TRANSFORMED)) {
      sendTo(COLOR_BASIC, "%s %s%s%s has been transformed!\n\r",
          who, red(),v->describeBodySlot2(i).c_str(),norm());
      found = TRUE;
    }
    if ((t = v->getStuckIn(i))) {
      if (canSee(t)) {
        strcpy(buf, t->getName());
        sendTo(COLOR_OBJECTS, "%s is sticking out of %s %s!\n\r",
        cap(buf), v->hshr(), v->describeBodySlot(i).c_str());
      }
    }
  }
   
  if(v==this)
    strcpy(who, "You");
  else {
    strncpy(who, v->hssh(), 5);
    cap(who);
  }
    
  if (v->affected) {
    for (aff = v->affected; aff; aff = aff->next) {
      if (aff->type == AFFECT_DISEASE) {
        if (!aff->level) {
          sendTo("%s %s %s.\n\r", who, 
	         (v==this)?"have":"has",
                 DiseaseInfo[DISEASE_INDEX(aff->modifier)].name);
          found = TRUE;
        }
      }
    }
  }
  if (!found)
    sendTo("All %s limbs are perfectly healthy!\n\r", (v==this)?"your":v->hshr());
}

void TBeing::genericEvaluateItem(const TThing *obj)
{
  obj->evaluateMe(this);
}

void TThing::evaluateMe(TBeing *ch) const
{
  int learn;

  learn = ch->getSkillValue(SKILL_EVALUATE);
  if (learn <= 0) {
    ch->sendTo("You are not sufficiently knowledgeable about evaluation.\n\r");
    return;
  }

  act("$p has little practical value in a fight.", false, ch, this, 0, TO_CHAR);
}

void TMagicItem::evaluateMe(TBeing *ch) const
{
  int learn;

  learn = ch->getSkillValue(SKILL_EVALUATE);
  if (learn <= 0) {
    ch->sendTo("You are not sufficiently knowledgeable about evaluation.\n\r");
    return;
  }

  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_EVALUATE, 10);

  // adjust for knowledge about magic stuff
  if (ch->hasClass(CLASS_RANGER)) {
    learn *= ch->getClassLevel(CLASS_RANGER);
    learn /= 200;
  } else {
    learn *= ch->getSkillValue(SPELL_IDENTIFY);
    learn /= 100;
  }
  if (learn <= 0) {
    ch->sendTo("You lack the knowledge to identify that item of magic.\n\r");
    return;
  }
  ch->sendTo(COLOR_OBJECTS, "You evaluate the magical powers of %s...\n\r\n\r",
         getName()); 

  ch->describeObject(this);

  if (learn > 10) 
    ch->describeMagicLevel(this, learn);
  
  if (learn > 15) {
    ch->describeMagicLearnedness(this, learn);
  }
  
  if (learn > 50) {
    ch->describeMagicSpell(this, learn);
  }
}

void TBeing::doEvaluate(const char *argument)
{
  char arg[160];
  TThing *obj;
  int count = 0,
      rNatureCount = 0;

  only_argument(argument, arg);
  if (!arg || !*arg) {
    sendTo("Evaluate what?\n\r");
    return;
  }
  if (is_abbrev(arg, "room")) {
    if (!roomp)
      return;

    if (isAffected(AFF_BLIND) && !isImmortal() && !isAffected(AFF_TRUE_SIGHT)) {
      sendTo("You can't see a damn thing -- you're blinded!\n\r");
      return;
    }

    describeRoomLight();

    if (!roomp) {
      sendTo("You have no idea where you are do you...\n\r");
      vlogf(7, "Player without room called evaluate room.  [%s]", getName());
    } else if (roomp->isCitySector())
      sendTo("You assume you are in a city by the way things look.\n\r");
    else if (roomp->isRoadSector())
      sendTo("It seems to be a well traveled road.\n\r");
    else if (roomp->isFlyingSector())
      sendTo("It seems as if you could just fly around here.\n\r");
    else if (roomp->isVertSector())
      sendTo("You seem to be scaling something.\n\r");
    else if (roomp->isUnderwaterSector())
      sendTo("\"Glub glub glub\" seem to best explain your current location.\n\r");
    else if (roomp->isSwampSector())
      sendTo("The swampy marsh gently rolls over you.\n\r");
    else if (roomp->isBeachSector())
      sendTo("A nice beach with a good view\n\r");
    else if (roomp->isHillSector())
      sendTo("A simple hill, nothing special.\n\r");
    else if (roomp->isMountainSector())
      sendTo("This appears to be a mountain.\n\r");
    else if (roomp->isForestSector())
      sendTo("Judging by the trees you bet your in a forest.\n\r");
    else if (roomp->isAirSector())
      sendTo("You are up in the air, are you sure you should be here?\n\r");
    else if (roomp->isOceanSector())
      sendTo("You are in the ocean, going on a sea cruise?\n\r");
    else if (roomp->isRiverSector()) {
      if (roomp->getRiverSpeed() >= 30)
        sendTo("The river flows swifly towards the distance.\n\r");
      else if (roomp->getRiverSpeed() >= 15)
        sendTo("The river flows steadily towards the distance.\n\r");
      else if (roomp->getRiverSpeed() > 0)
        sendTo("The river gently flows towards the distance.\n\r");
      else
        sendTo("The river seems to be standing still.\n\r");
    } else if (roomp->isArcticSector())
      sendTo("The arctic winds around you seem to tell it all.\n\r");
    else if (roomp->isTropicalSector())
      sendTo("The tropical world around you makes you hot and sweaty.\n\r");
    else if (roomp->isWildernessSector() || roomp->isNatureSector())
      sendTo("You would appear to be in area draped with nature and the natural wilds.\n\r");
    else
      sendTo("You are somewhere...But damned if you can figure it out.\n\r");

    if (!hasClass(CLASS_RANGER))
      return;

    for (int Runner = MIN_DIR; Runner < MAX_DIR; Runner++)
      if (Runner != DIR_UP && Runner != DIR_DOWN && roomp->dir_option[Runner] &&
          real_roomp((count = roomp->dir_option[Runner]->to_room)))
        if (real_roomp(count)->notRangerLandSector())
          rNatureCount--;
        else
          rNatureCount++;

    if (roomp->notRangerLandSector()) {
      if (rNatureCount > 6)
        sendTo("You are getting real close to nature, you can feel it.\n\r");
      else if (rNatureCount > 3)
        sendTo("You are not that far, just a little more.\n\r");
      else if (rNatureCount > 0)
        sendTo("You feel really out of touch from nature here...\n\r");
      else
        sendTo("You don't feel very much in touch with nature here.\n\r");
    } else {
      if (rNatureCount > 6)
        sendTo("Now this is nature, you feel right at home.\n\r");
      else if (rNatureCount > 3)
        sendTo("The soothing sounds of nature rush over your body.\n\r");
      else if (rNatureCount > 0)
        sendTo("Altho not that dense, it's still home.\n\r");
      else
        sendTo("It's nature, but it sure doesn't feel like nature...\n\r");
    }
  } else {
    wearSlotT j;
    if (!(obj = get_thing_in_equip(this, arg, equipment, &j, TRUE, &count))) {
      if (!(obj = searchLinkedListVis(this, arg, stuff, &count))) {
        sendTo("You do not seem to have the '%s'.\n\r", arg);
        return;
      }
    }
    genericEvaluateItem(obj);
  }
}

void TTool::describeCondition(const TBeing *) const
{
  // intentionally blank
}

void TObj::describeCondition(const TBeing *ch) const
{
  ch->sendTo(COLOR_OBJECTS, "It is in %s condition.\n\r",equip_condition(-1).c_str());
}

void TThing::describeContains(const TBeing *ch) const
{
  if (stuff)
    ch->sendTo(COLOR_OBJECTS, "%s seems to have something in it...\n\r", good_cap(getName()).c_str());
}

void TBaseCup::describeContains(const TBeing *ch) const
{
  if (getDrinkUnits())
    ch->sendTo(COLOR_OBJECTS, "%s seems to have something in it...\n\r", good_cap(getName()).c_str());
}

void TFood::describeCondition(const TBeing *ch) const
{
  ch->sendTo(COLOR_OBJECTS, "It is in %s condition.\n\r",equip_condition(-1).c_str());
}

void TFood::describeObjectSpecifics(const TBeing *ch) const
{
  if (isFoodFlag(FOOD_SPOILED))
    act("$p looks a bit spoiled.", FALSE, ch, this, 0, TO_CHAR); 
}

void TCorpse::describeObjectSpecifics(const TBeing *ch) const
{
  if (isCorpseFlag(CORPSE_NO_SKIN))
    act("$p doesn't appear to have any skin left on it.",
        FALSE, ch, this, 0, TO_CHAR);
  else if (isCorpseFlag(CORPSE_HALF_SKIN))
    act("$p appears to have been half skinned.",
        FALSE, ch, this, 0, TO_CHAR);
  if (isCorpseFlag(CORPSE_NO_DISSECT))
    act("$p appears to have been dissected already.",
        FALSE, ch, this, 0, TO_CHAR);
}

void TSymbol::describeObjectSpecifics(const TBeing *ch) const
{
  double diff;
  int attuneCode = 1;
  factionTypeT sym_faction = getSymbolFaction();
  
if (attuneCode) {
  switch (sym_faction) {
    case FACT_NONE:
      ch->sendTo(COLOR_OBJECTS, "You can tell that %s has been sanctified but it bears no insignia.\n\r", good_cap(getName()).c_str());
      break;
    case FACT_BROTHERHOOD:
      ch->sendTo(COLOR_OBJECTS, "%s has the sign of the Brotherhood of Galek stamped upon it.\n\r", good_cap(getName()).c_str());
      break;
    case FACT_CULT:
      ch->sendTo(COLOR_OBJECTS, "%s has been sanctified and bears the insignia of the Cult of the Logrus.\n\r", good_cap(getName()).c_str());
      break;
    case FACT_SNAKE:
      ch->sendTo(COLOR_OBJECTS, "%s has been sanctified and branded with the image of a snake.\n\r", good_cap(getName()).c_str());
      break;
    case MAX_FACTIONS:
    case FACT_UNDEFINED:
      ch->sendTo(COLOR_OBJECTS, "%s is inert and has not been sanctified for use.\n\r", good_cap(getName()).c_str());
  }
}

  if (getSymbolMaxStrength()) {
    if (getSymbolCurStrength() == getSymbolMaxStrength()) {
      ch->sendTo(COLOR_OBJECTS, "%s has all of its original strength, it has not been used.\n\r", good_cap(getName()).c_str());

    } else {
      diff = (double) ((double) getSymbolCurStrength() /
                       (double) getSymbolMaxStrength());
      ch->sendTo(COLOR_OBJECTS, "You can tell that %s has %s strength left.\n\r", getName(),
          ((diff < .20) ? "very little" : ((diff < .50) ? "some" :
          ((diff < .75) ? "a good bit of" : "most of its"))));
    }
  } else {
    ch->sendTo(COLOR_OBJECTS, "%s has none of its strength left.\n\r", good_cap(getName()).c_str());
  }
}

void TTool::describeObjectSpecifics(const TBeing *ch) const
{
  double diff;

  if (getToolMaxUses()) 
    diff = ((double) getToolUses()) / ((double) getToolMaxUses());
  else
    diff = 1.00;

  ch->sendTo(COLOR_OBJECTS,"It appears %s is %s.\n\r", getName(),
       ((diff <= 0.0) ? "totally gone" :
        ((diff >= 1.0) ? "brand new" :
        ((diff >= 0.8) ? "almost new" :
        ((diff >= 0.6) ? "like new" :
        ((diff >= 0.4) ? "half gone" :
        ((diff >= 0.2) ? "more than half gone" :
                  "almost gone")))))));
}

void TObj::describeMe(const TBeing *ch) const
{
  char buf[80], buf2[256];
  char capbuf[256];
  char name_buf[80];
  TThing *t2;

  strcpy(buf, material_nums[getMaterial()].mat_name);
  strcpy(buf2, ch->objs(this));
  ch->sendTo(COLOR_OBJECTS,"%s is %s made of %s.\n\r", cap(buf2),
                 ItemInfo[itemType()]->common_name, uncap(buf));

  if (ch->isImmortal() || canWear(ITEM_TAKE)) {

    if (ch->desc && IS_SET(ch->desc->account->flags, ACCOUNT_IMMORTAL)){
      if(obj_index[getItemIndex()].max_exist <= MIN_EXIST_IMMORTAL)
        ch->sendTo("It is limited for immortals and there is %i out of %i in existence.\n\r", 
                   obj_index[getItemIndex()].number,
                   obj_index[getItemIndex()].max_exist);
      else
        ch->sendTo("It is not limited for immortals.\n\r");
    }


    if (isRentable()) {
      int temp = max(0, rentCost());
  
      ch->sendTo("It has a rental cost of %d talen%s.\n\r",
          temp, (temp != 1 ? "s" : ""));
    } else 
      ch->sendTo("It can't be rented.\n\r");

    int volume = getVolume();
    volume = ((volume >= 100) ? volume/100 * 100 :
              ((volume >= 10) ? volume/10 * 10 : volume));

    // weight >= 1.0
    float wgt = getTotalWeight(TRUE);
    int volumeTmp, yards = 0;
    int feet = 0;
    int inches;
    char volumeBuf[256] = "\0";

    volumeTmp = volume;
    if (volumeTmp > CUBIC_INCHES_PER_YARD) {
      yards = volume/CUBIC_INCHES_PER_YARD;
      volumeTmp = volume % CUBIC_INCHES_PER_YARD;
      sprintf(volumeBuf, "%d cubic yard%s, ", yards, (yards == 1) ? "" : "s");
    }
    if (volumeTmp > CUBIC_INCHES_PER_FOOT) {
      feet = volumeTmp/CUBIC_INCHES_PER_FOOT;
      volumeTmp = volume % CUBIC_INCHES_PER_FOOT;
      sprintf(volumeBuf + strlen(volumeBuf), "%d cubic %s, ", feet, (yards == 1) ? "foot" : "feet");
    }
    if ((inches = volumeTmp))
      sprintf(volumeBuf + strlen(volumeBuf), "%d cubic inch%s", inches, (inches == 1) ? "" : "es");
    if (!volume) {
      // this only kicks in if no volume
      sprintf(volumeBuf + strlen(volumeBuf), "0 cubic inches");
    }

    if (compareWeights(wgt, 1.0) != 1) 
      ch->sendTo("It weighs about %d pound%s and occupies roughly %s.\n\r", 
               (int) wgt, ((((int) wgt) == 1) ? "" : "s"), volumeBuf);
    else 
      ch->sendTo("It weighs about %d drechel%s and occupies roughly %s.\n\r", 
               getDrechels(TRUE), ((getDrechels(TRUE) == 1) ? "" : "s"), volumeBuf);
  }
  describeCondition(ch);
  if (isObjStat(ITEM_GLOW))
    act("It is <o>glowing<1>.", FALSE, ch, 0, 0, TO_CHAR);
  if (isObjStat(ITEM_BURNING))
    act("It is <r>burning<1>.", FALSE, ch, 0, 0, TO_CHAR);
  if (isObjStat(ITEM_CHARRED))
    act("It is <k>charred<1>.", FALSE, ch, 0, 0, TO_CHAR);
  describeContains(ch);

  if (dynamic_cast<TBeing *>(rider)) {
    act("$p is being used by $N.", FALSE, ch, this, horseMaster(), TO_CHAR);
    for (t2 = rider->nextRider; t2; t2 = t2->nextRider) {
      if (t2 == horseMaster())
        continue;
      if (!dynamic_cast<TBeing *>(t2))
        continue;
      act("$p is also being used by $N.", FALSE, ch, this, t2, TO_CHAR);
    }
  }
  if (action_description) {
    strcpy(capbuf, action_description);
    if ((sscanf(capbuf, "This is the personalized object of %s.", name_buf)) == 1)
      sendTo("A monogram on it indicates it belongs to %s.\n\r", name_buf);
  }
  describeObjectSpecifics(ch);
}

void TBeing::describeObject(const TThing *t) const
{
  t->describeMe(this);
}

string TBeing::describeSharpness(const TThing *obj) const
{
  return obj->describeMySharp(this);
}

string TThing::describeMySharp(const TBeing *) const
{
  char buf[256];
  sprintf(buf, "%s is not a weapon", getName());
  return buf;
}

string TBeing::describePointiness(const TBaseWeapon *obj) const
{
  char buf[256];
  char sharpbuf[80];
  int maxsharp = obj->getMaxSharp();
  int sharp = obj->getCurSharp();
  double diff;

  if (!maxsharp)
    diff = (double) 0;
  else
    diff = (double) ((double) sharp / (double) maxsharp);
//  strcpy(capbuf, objs(obj));
  string capbuf = colorString(this,desc, objs(obj), NULL, COLOR_OBJECTS, TRUE);

  if (diff <= .02)
    strcpy(sharpbuf, "is totally blunt");
  else if (diff < .10)
    strcpy(sharpbuf, "is virtually blunt now");
  else if (diff < .30)
    strcpy(sharpbuf, "has very little of its point left");
  else if (diff < .50)
    strcpy(sharpbuf, "is starting to get blunt");
  else if (diff < .70)
    strcpy(sharpbuf, "has some pointedness");
  else if (diff < .90)
    strcpy(sharpbuf, "has a good point");
  else if (diff < 1.0)
    strcpy(sharpbuf, "is nice and pointy");
  else
    strcpy(sharpbuf, "is as pointed as it is going to get");

  sprintf(buf, "%s %s", good_uncap(capbuf).c_str(), sharpbuf);
  return buf;
}

string TBeing::describeBluntness(const TBaseWeapon *obj) const
{
  char buf[256];
  char sharpbuf[80];
  int maxsharp = obj->getMaxSharp();
  int sharp = obj->getCurSharp();
  double diff;
  if (!maxsharp)
    diff = (double) 0;
  else
    diff = (double) ((double) sharp / (double) maxsharp);
//  strcpy(capbuf, objs(obj));
  string capbuf = colorString(this,desc, objs(obj), NULL, COLOR_OBJECTS, TRUE);

  if (diff <= .02)
    strcpy(sharpbuf, "is totally jagged");
  else if (diff < .10)
    strcpy(sharpbuf, "is extremely jagged now");
  else if (diff < .30)
    strcpy(sharpbuf, "has become jagged");
  else if (diff < .50)
    strcpy(sharpbuf, "is fairly jagged");
  else if (diff < .70)
    strcpy(sharpbuf, "has a lot of chips and is starting to get jagged");
  else if (diff < .90)
    strcpy(sharpbuf, "has some chips");
  else if (diff < 1.0)
    strcpy(sharpbuf, "has a few chips");
  else
    strcpy(sharpbuf, "is completely blunt");

  sprintf(buf, "%s %s", good_uncap(capbuf).c_str(), sharpbuf);
  return buf;
}

void TBeing::describeMaxSharpness(const TBaseWeapon *obj, int learn) const
{
  if (obj->isBluntWeapon()) {
    describeMaxBluntness(obj, learn);
    return;
  } else if (obj->isPierceWeapon()) {
    describeMaxPointiness(obj, learn);
    return;
  }
  if (!hasClass(CLASS_THIEF) && !hasClass(CLASS_WARRIOR) && 
      !hasClass(CLASS_DEIKHAN) && !hasClass(CLASS_RANGER))
    learn /= 3;

  int maxsharp = GetApprox(obj->getMaxSharp(), learn);

  char capbuf[80], sharpbuf[80];
  strcpy(capbuf, objs(obj));

  if (maxsharp >= 99)
    strcpy(sharpbuf, "being unhumanly sharp");
  else if (maxsharp >= 90)
    strcpy(sharpbuf, "being razor sharp");
  else if (maxsharp >= 80)
    strcpy(sharpbuf, "approximating razor sharpness");
  else if (maxsharp >= 72)
    strcpy(sharpbuf, "being incredibly sharp");
  else if (maxsharp >= 64)
    strcpy(sharpbuf, "being very sharp");
  else if (maxsharp >= 56)
    strcpy(sharpbuf, "being sharp");
  else if (maxsharp >= 48)
    strcpy(sharpbuf, "being fairly sharp");
  else if (maxsharp >= 40)
    strcpy(sharpbuf, "having a sword-like edge");
  else if (maxsharp >= 30)
    strcpy(sharpbuf, "having a very sharp edge");
  else if (maxsharp >= 20)
    strcpy(sharpbuf, "having a knife-like edge");
  else if (maxsharp >= 10)
    strcpy(sharpbuf, "having a sharp edge");
  else
    strcpy(sharpbuf, "having a ragged edge");

  sendTo(COLOR_OBJECTS,"%s seems to be capable of %s.\n\r",
           cap(capbuf), sharpbuf);
}

void TBeing::describeMaxPointiness(const TBaseWeapon *obj, int learn) const
{
  char capbuf[80], sharpbuf[80];
  strcpy(capbuf, objs(obj));

  if (!hasClass(CLASS_THIEF) && !hasClass(CLASS_WARRIOR) && 
      !hasClass(CLASS_DEIKHAN) && !hasClass(CLASS_RANGER) &&
      !hasClass(CLASS_SHAMAN) && !hasClass(CLASS_MAGIC_USER))
    learn /= 3;

  int maxsharp = GetApprox(obj->getMaxSharp(), learn);

  if (maxsharp >= 99)
    strcpy(sharpbuf, "being unhumanly pointy");
  else if (maxsharp >= 90)
    strcpy(sharpbuf, "being awesomly pointy");
  else if (maxsharp >= 80)
    strcpy(sharpbuf, "having an amazing point");
  else if (maxsharp >= 72)
    strcpy(sharpbuf, "being incredibly pointy");
  else if (maxsharp >= 64)
    strcpy(sharpbuf, "being very pointy");
  else if (maxsharp >= 56)
    strcpy(sharpbuf, "being pointy");
  else if (maxsharp >= 48)
    strcpy(sharpbuf, "being fairly pointy");
  else if (maxsharp >= 40)
    strcpy(sharpbuf, "having a dagger-like point");
  else if (maxsharp >= 30)
    strcpy(sharpbuf, "having a nice point");
  else if (maxsharp >= 20)
    strcpy(sharpbuf, "having a spear-like point");
  else if (maxsharp >= 10)
    strcpy(sharpbuf, "having a point");
  else
    strcpy(sharpbuf, "having a dull point");

  sendTo(COLOR_OBJECTS,"%s seems to be capable of %s.\n\r",
           cap(capbuf), sharpbuf);
}

void TBeing::describeOtherFeatures(const TGenWeapon *obj, int learn) const
{
  char capbuf[80];
  strcpy(capbuf, objs(obj));

  if (!obj) {
    sendTo("Something went wrong, tell a god what you did.\n\r");
    return;
  }

  if (hasClass(CLASS_THIEF) || isImmortal()) {
    if (obj->canCudgel())
      sendTo(COLOR_OBJECTS, "%s seems small enough to be used for cudgeling.\n\r",
             cap(capbuf));
    if (obj->canStab())
      sendTo(COLOR_OBJECTS, "%s seems small enough to be used for stabbing.\n\r",
             cap(capbuf));
    if (obj->canBackstab())
      sendTo(COLOR_OBJECTS, "%s seems small enough to be used for backstabbing.\n\r",
             cap(capbuf));
  }
}

void TBeing::describeMaxBluntness(const TBaseWeapon *obj, int learn) const
{
  char capbuf[80], sharpbuf[80];
  strcpy(capbuf, objs(obj));

  if (!hasClass(CLASS_CLERIC) && !hasClass(CLASS_WARRIOR) && 
      !hasClass(CLASS_SHAMAN) && !hasClass(CLASS_DEIKHAN))
    learn /= 3;

  int maxsharp = GetApprox(obj->getMaxSharp(), learn);

  if (maxsharp >= 99)
    strcpy(sharpbuf, "being polished smooth");
  else if (maxsharp >= 90)
    strcpy(sharpbuf, "being awesomly blunt");
  else if (maxsharp >= 80)
    strcpy(sharpbuf, "being extremely blunt");
  else if (maxsharp >= 72)
    strcpy(sharpbuf, "being incredibly blunt");
  else if (maxsharp >= 64)
    strcpy(sharpbuf, "being very blunt");
  else if (maxsharp >= 56)
    strcpy(sharpbuf, "being blunt");
  else if (maxsharp >= 48)
    strcpy(sharpbuf, "being fairly blunt");
  else if (maxsharp >= 40)
    strcpy(sharpbuf, "having a hammer-like bluntness");
  else if (maxsharp >= 30)
    strcpy(sharpbuf, "being somewhat blunt");
  else if (maxsharp >= 20)
    strcpy(sharpbuf, "having a mace-like bluntness");
  else if (maxsharp >= 10)
    strcpy(sharpbuf, "having a ragged bluntness");
  else
    strcpy(sharpbuf, "having a sharp and ragged bluntness");

  sendTo(COLOR_OBJECTS,"%s seems to be capable of %s.\n\r",
           cap(capbuf), sharpbuf);
}

void TBeing::describeMaxStructure(const TObj *obj, int learn) const
{
  obj->descMaxStruct(this, learn);
}

void TBeing::describeWeaponDamage(const TBaseWeapon *obj, int learn) const
{
  if (!hasClass(CLASS_RANGER) && !hasClass(CLASS_WARRIOR) && 
      !hasClass(CLASS_DEIKHAN)) {
    learn /= 3;
  }

  double av_dam = obj->baseDamage();
  av_dam += (double) obj->itemDamroll();
  av_dam = GetApprox((int) av_dam, learn);

  sendTo(COLOR_OBJECTS,"It's capable of doing %s damage.\n\r",
          ((av_dam < 1) ? "exceptionally low" :
          ((av_dam < 2) ? "incredibly low" :
          ((av_dam < 3) ? "very low" :
          ((av_dam < 4) ? "low" :
          ((av_dam < 5) ? "fairly low" :
          ((av_dam < 6) ? "moderate" :
          ((av_dam < 7) ? "a good bit of" :
          ((av_dam < 8) ? "fairly good" :
          ((av_dam < 9) ? "good" :
          ((av_dam < 10) ? "very good" :
          ((av_dam < 11) ? "really good" :
          ((av_dam < 12) ? "exceptionally good" :
          ((av_dam < 13) ? "very nice" :
          ((av_dam < 14) ? "a sizable amount of" :
          ((av_dam < 15) ? "respectable" :
          ((av_dam < 16) ? "whopping" :
          ((av_dam < 17) ? "punishing" :
          ((av_dam < 18) ? "devestating" :
          ((av_dam < 19) ? "awesome" :
          ((av_dam < 20) ? "superb" :
                           "super-human")))))))))))))))))))));
}

void TBeing::describeArmor(const TBaseClothing *obj, int learn)
{
  if (!hasClass(CLASS_RANGER) && !hasClass(CLASS_WARRIOR) && 
      !hasClass(CLASS_DEIKHAN))
    learn /= 3;

  int armor = 0;    // works in reverse here.  armor > 0 is GOOD
  armor -= obj->itemAC();
  armor = GetApprox(armor, learn);

  sendTo(COLOR_OBJECTS,"In terms of armor quality, it ranks as %s.\n\r",
      ((armor < 0) ? "being more hurt then help" :
      ((armor < 2) ? "being virtually non-existant" :
      ((armor < 4) ? "being exceptionally low" :
      ((armor < 6) ? "being low" :
      ((armor < 8) ? "being fairly good" :
      ((armor < 10) ? "being somewhat good" :
      ((armor < 12) ? "being good" :
      ((armor < 14) ? "being really good" :
      ((armor < 16) ? "protecting you fairly well" :
      ((armor < 18) ? "protecting you well" :
      ((armor < 20) ? "protecting you very well" :
      ((armor < 22) ? "protecting you really well" :
      ((armor < 24) ? "protecting you exceptionally well" :
      ((armor < 26) ? "armoring you like a fortress" :
      ((armor < 28) ? "armoring you like a dragon" :
      ((armor < 30) ? "being virtually impenetrable" :
                       "being impenetrable")))))))))))))))));
}

void TBeing::describeImmunities(TBeing *vict, int learn)
{
  char buf[80];
  int x;
  for (immuneTypeT i = MIN_IMMUNE;i < MAX_IMMUNES; i++) {
    x = GetApprox(vict->getImmunity(i), learn);

    if (x == 0 || !*immunity_names[i])
      continue;
    if (x > 90 || x < -90)
      strcpy(buf, "extremely");
    else if (x > 70 || x < -70)
      strcpy(buf, "heavily");
    else if (x > 50 || x < -50)
      strcpy(buf, "majorly");
    else if (x > 30 || x < -30)
      strcpy(buf, "greatly");
    else if (x > 10 || x < -10)
      strcpy(buf, "somewhat");
    else
      strcpy(buf, "lightly");

    if (vict == this)
      sendTo(COLOR_MOBS, "You are %s %s to %s.\n\r",
         buf, (x > 0 ? "resistant" : "susceptible"),
         immunity_names[i]);
    else
      sendTo(COLOR_MOBS, "%s is %s %s to %s.\n\r",
         good_cap(pers(vict)).c_str(),
         buf, (x > 0 ? "resistant" : "susceptible"),
         immunity_names[i]);
  }
}

void TBeing::describeArrowDamage(const TArrow *obj, int learn)
{
  if (!hasClass(CLASS_RANGER))
    learn /= 3;

  double av_dam = obj->baseDamage();
  av_dam += (double) obj->itemDamroll();
  av_dam = GetApprox((int) av_dam, learn);

  sendTo(COLOR_OBJECTS, "It's capable of doing %s damage.\n\r",
          ((av_dam < 1) ? "exceptionally low" :
          ((av_dam < 2) ? "incredibly low" :
          ((av_dam < 3) ? "very low" :
          ((av_dam < 4) ? "low" :
          ((av_dam < 5) ? "fairly low" :
          ((av_dam < 6) ? "moderate" :
          ((av_dam < 7) ? "a good bit of" :
          ((av_dam < 8) ? "fairly good" :
          ((av_dam < 9) ? "good" :
          ((av_dam < 10) ? "very good" :
          ((av_dam < 11) ? "really good" :
          ((av_dam < 12) ? "exceptionally good" :
          ((av_dam < 13) ? "very nice" :
          ((av_dam < 14) ? "a sizable amount of" :
          ((av_dam < 15) ? "respectable" :
          ((av_dam < 16) ? "whopping" :
          ((av_dam < 17) ? "punishing" :
          ((av_dam < 18) ? "devestating" :
          ((av_dam < 19) ? "awesome" :
          ((av_dam < 20) ? "superb" :
                           "super-human")))))))))))))))))))));
}

void TBeing::describeArrowSharpness(const TArrow *obj, int learn)
{
  if (!hasClass(CLASS_RANGER))
    learn /= 3;

  int maxsharp = GetApprox(obj->getCurSharp(), learn);
 
  char capbuf[80], sharpbuf[80];
  strcpy(capbuf, objs(obj));
 
  if (maxsharp >= 99)
    strcpy(sharpbuf, "unhumanly sharp");
  else if (maxsharp >= 90)
    strcpy(sharpbuf, "razor sharp");
  else if (maxsharp >= 80)
    strcpy(sharpbuf, "almost razor sharp");
  else if (maxsharp >= 72)
    strcpy(sharpbuf, "incredibly sharp");
  else if (maxsharp >= 64)
    strcpy(sharpbuf, "very sharp");
  else if (maxsharp >= 56)
    strcpy(sharpbuf, "sharp");
  else if (maxsharp >= 48)
    strcpy(sharpbuf, "fairly sharp");
  else if (maxsharp >= 40)
    strcpy(sharpbuf, "kind of sharp");
  else if (maxsharp >= 30)
    strcpy(sharpbuf, "not really sharp");
  else if (maxsharp >= 20)
    strcpy(sharpbuf, "dull");
  else if (maxsharp >= 10)
    strcpy(sharpbuf, "very dull");
  else
    strcpy(sharpbuf, "extremely dull");
 
  sendTo(COLOR_OBJECTS, "%s has a tip that is %s.\n\r", cap(capbuf), sharpbuf);

}

void TBeing::describeNoise(const TObj *obj, int learn) const
{
  if (!dynamic_cast<const TBaseClothing *>(obj) &&
      !dynamic_cast<const TBaseWeapon *>(obj) && 
      !dynamic_cast<const TBow *>(obj))
    return;

  if (obj->canWear(ITEM_HOLD) || obj->canWear(ITEM_WEAR_FINGER))
    return;

  int iNoise = GetApprox(material_nums[obj->getMaterial()].noise, learn);

  char capbuf[160];
  strcpy(capbuf, objs(obj));

  sendTo(COLOR_OBJECTS, "%s is %s.\n\r", cap(capbuf),
          ((iNoise < -9) ? "beyond silent" :
          ((iNoise < -5) ? "extremely silent" :
          ((iNoise < -2) ? "very silent" :
          ((iNoise < 1) ? "silent" :
          ((iNoise < 3) ? "very quiet" :
          ((iNoise < 6) ? "quiet" :
          ((iNoise < 10) ? "pretty quiet" :
          ((iNoise < 14) ? "mostly quiet" :
          ((iNoise < 19) ? "slightly noisy" :
          ((iNoise < 25) ? "fairly noisy" :
          ((iNoise < 31) ? "noisy" :
          ((iNoise < 38) ? "very noisy" :
                           "loud")))))))))))));
}

void TBeing::describeRoomLight()
{
  int illum = roomp->getLight();

  sendTo("This area is %s.\n\r", 
          ((illum < -4) ? "super dark" :
          ((illum < 0) ? "pitch dark" :
          ((illum < 1) ? "dark" :
          ((illum < 3) ? "very dimly lit" :
          ((illum < 5) ? "dimly lit" :
          ((illum < 9) ? "barely lit" :
          ((illum < 13) ? "lit" :
          ((illum < 19) ? "brightly lit" :
          ((illum < 25) ? "very brightly lit" :
                           "bright as day"))))))))));
}

void TBeing::describeBowRange(const TBow *obj, int learn)
{
  if (!hasClass(CLASS_RANGER))
    learn /= 3;

  int range = GetApprox((int) obj->getMaxRange(), learn);

  char capbuf[160];
  strcpy(capbuf, objs(obj));

  sendTo(COLOR_OBJECTS, "%s can %s.\n\r", cap(capbuf),
          ((range < 1) ? "not shoot out of the immediate area" :
          ((range < 3) ? "barely shoot beyond arm's length" :
          ((range < 5) ? "shoot a short distance" :
          ((range < 8) ? "fire a reasonable distance" :
          ((range < 11) ? "shoot quite a ways" :
          ((range < 15) ? "shoot a long distance" :
                           "fire incredibly far")))))));
}

void TBeing::describeMagicLevel(const TMagicItem *obj, int learn) const
{
  if (!hasClass(CLASS_MAGIC_USER) && !hasClass(CLASS_CLERIC) &&
      !hasClass(CLASS_RANGER)  && !hasClass(CLASS_DEIKHAN))
    return;

  int level = GetApprox(obj->getMagicLevel(), learn);
  level = max(level,0);

  char capbuf[160];
  strcpy(capbuf, objs(obj));

  sendTo(COLOR_OBJECTS, "Spells from %s seem to be cast at %s level.\n\r", 
          uncap(capbuf),
          numberAsString(level).c_str());

}

const string numberAsString(int num)
{
  char buf[50];

  if (in_range(num%100, 11, 13))
    sprintf(buf, "%dth", num);
  else if ((num%10) == 1)
    sprintf(buf, "%dst", num);
  else if ((num%10) == 2)
    sprintf(buf, "%dnd", num);
  else if ((num%10) == 3)
    sprintf(buf, "%drd", num);
  else
    sprintf(buf, "%dth", num);

  return buf;
}

void TBeing::describeMagicLearnedness(const TMagicItem *obj, int learn) const
{
  if (!hasClass(CLASS_MAGIC_USER) && !hasClass(CLASS_CLERIC) &&
      !hasClass(CLASS_RANGER)  && !hasClass(CLASS_DEIKHAN))
    return;

  int level = GetApprox(obj->getMagicLearnedness(), learn);

  char capbuf[160];
  strcpy(capbuf, objs(obj));

  sendTo(COLOR_OBJECTS, "The learnedness of the spells in %s is: %s.\n\r", uncap(capbuf),
                how_good(level));
}

void TBeing::describeMagicSpell(const TMagicItem *obj, int learn)
{
  char capbuf[160];
  strcpy(capbuf, objs(obj));

  if (!hasClass(CLASS_MAGIC_USER) && !hasClass(CLASS_CLERIC) &&
      !hasClass(CLASS_RANGER)  && !hasClass(CLASS_DEIKHAN))
    return;

  int level = GetApprox(getSkillLevel(SKILL_EVALUATE), learn);

  if (obj->getMagicLevel() > level) {
    sendTo(COLOR_OBJECTS, "You can tell nothing about the spells %s produces.\n\r", uncap(capbuf));
    return;
  }

  obj->descMagicSpells(this);
}

void TWand::descMagicSpells(TBeing *ch) const
{
  discNumT das = DISC_NONE;
  spellNumT iSpell;
  char capbuf[160];
  strcpy(capbuf, ch->objs(this));

  if ((iSpell = getSpell()) >= MIN_SPELL && discArray[iSpell] &&
      ((das = getDisciplineNumber(iSpell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(iSpell))
      ch->sendTo(COLOR_OBJECTS, "%s produces: %s.\n\r", cap(capbuf), 
            discArray[iSpell]->name);
    else
      ch->sendTo(COLOR_OBJECTS, "%s produces: Something from the %s discipline.\n\r", cap(capbuf),  disc_names[das]);
  }

  return;
}

void TStaff::descMagicSpells(TBeing *ch) const
{
  discNumT das = DISC_NONE;
  spellNumT iSpell;
  char capbuf[160];
  strcpy(capbuf, ch->objs(this));

  if ((iSpell = getSpell()) >= MIN_SPELL && discArray[iSpell] &&
      ((das = getDisciplineNumber(iSpell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(iSpell))
      ch->sendTo(COLOR_OBJECTS, "%s produces: %s.\n\r", cap(capbuf), 
            discArray[iSpell]->name);
    else
      ch->sendTo(COLOR_OBJECTS, "%s produces: Something from the %s discipline.\n\r", cap(capbuf),  disc_names[das]);
  }

  return;
}

void TScroll::descMagicSpells(TBeing *ch) const
{
  discNumT das = DISC_NONE;
  spellNumT spell;
  char capbuf[160];
  strcpy(capbuf, ch->objs(this));

  spell = getSpell(0);
  if (spell > TYPE_UNDEFINED && discArray[spell] &&
      ((das = getDisciplineNumber(spell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(spell))
      ch->sendTo(COLOR_OBJECTS, "%s produces: %s.\n\r", cap(capbuf), 
            discArray[spell]->name);
    else
      ch->sendTo(COLOR_OBJECTS, "%s produces: Something from the %s discipline.\n\r", cap(capbuf),  disc_names[das]);
  }

  spell = getSpell(1);
  if (spell > TYPE_UNDEFINED && discArray[spell] &&
      ((das = getDisciplineNumber(spell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(spell))
      ch->sendTo(COLOR_OBJECTS, "%s produces: %s.\n\r", cap(capbuf), 
            discArray[spell]->name);
    else
       ch->sendTo(COLOR_OBJECTS, "%s produces: Something from the %s discipline.\n\r", cap(capbuf), disc_names[das]);
  }

  spell = getSpell(2);
  if (spell > TYPE_UNDEFINED && discArray[spell] &&
      ((das = getDisciplineNumber(spell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(spell))
      ch->sendTo(COLOR_OBJECTS, "%s produces: %s.\n\r", cap(capbuf), 
            discArray[spell]->name);
    else
       ch->sendTo(COLOR_OBJECTS, "%s produces: Something from the %s discipline.\n\r", cap(capbuf), disc_names[das]);
  }

  return;
}

void TPotion::descMagicSpells(TBeing *ch) const
{
  discNumT das = DISC_NONE;
  spellNumT spell;
  char capbuf[160];
  strcpy(capbuf, ch->objs(this));

  spell = getSpell(0);
  if (spell >= 0 && discArray[spell] &&
      ((das = getDisciplineNumber(spell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(spell))
      ch->sendTo(COLOR_OBJECTS, "%s produces: %s.\n\r", cap(capbuf), 
            discArray[spell]->name);
    else
      ch->sendTo(COLOR_OBJECTS, "%s produces: Something from the %s discipline.\n\r", cap(capbuf),  disc_names[das]);
  }

  spell = getSpell(1);
  if (spell >= 0 && discArray[spell] &&
      ((das = getDisciplineNumber(spell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(spell))
      ch->sendTo(COLOR_OBJECTS, "%s produces: %s.\n\r", cap(capbuf), 
            discArray[spell]->name);
    else
       ch->sendTo(COLOR_OBJECTS, "%s produces: Something from the %s discipline.\n\r", cap(capbuf), disc_names[das]);
  }

  spell = getSpell(2);
  if (spell >= 0 && discArray[spell] &&
      ((das = getDisciplineNumber(spell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(spell))
      ch->sendTo(COLOR_OBJECTS, "%s produces: %s.\n\r", cap(capbuf), 
            discArray[spell]->name);
    else
       ch->sendTo(COLOR_OBJECTS, "%s produces: Something from the %s discipline.\n\r", cap(capbuf), disc_names[das]);
  }

  return;
}

void TBeing::describeSymbolOunces(const TSymbol *obj, int learn) const
{
  if (obj->getSymbolFaction() != FACT_UNDEFINED) {
    act("$p is already attuned.", false, this, obj, 0, TO_CHAR);
    return;
  }

  if (!hasClass(CLASS_CLERIC) && !hasClass(CLASS_DEIKHAN))
    learn /= 3;

  // number of ounces needed, see attuning
  int amt = obj->obj_flags.cost / 100;
  amt = GetApprox(amt, learn);

  char capbuf[160];
  strcpy(capbuf, objs(obj));

  sendTo(COLOR_OBJECTS, "%s requires about %d ounce%s of holy water to attune.\n\r", cap(capbuf), amt, amt == 1 ? "" : "s");

  return;
}

void TBeing::describeComponentUseage(const TComponent *obj, int) const
{
  char capbuf[160];
  strcpy(capbuf, objs(obj));

  if (IS_SET(obj->getComponentType(), COMP_SPELL))
    sendTo(COLOR_OBJECTS, "%s is a component used in creating magic.\n\r", cap(capbuf));
  else if (IS_SET(obj->getComponentType(), COMP_POTION))
    sendTo(COLOR_OBJECTS, "%s is a component used to brew potions.\n\r", cap(capbuf));
  else if (IS_SET(obj->getComponentType(), COMP_SCRIBE))
    sendTo(COLOR_OBJECTS, "%s is a component used during scribing.\n\r", cap(capbuf));

  return;
}

void TBeing::describeComponentDecay(const TComponent *obj, int learn) const
{
  if (!hasClass(CLASS_MAGIC_USER) && !hasClass(CLASS_CLERIC) &&
      !hasClass(CLASS_RANGER)  && !hasClass(CLASS_DEIKHAN))
    learn /= 3;

  int level = GetApprox(obj->obj_flags.decay_time, learn);

  char capbuf[160];
  strcpy(capbuf, objs(obj));

  sendTo(COLOR_OBJECTS, "%s will last ", cap(capbuf));

  if (!obj->isComponentType(COMP_DECAY)) {
    sendTo("well into the future.\n\r");
    return;
  }

  if ((level <= -1) || (level >= 800))
    sendTo("well into the future.\n\r");
  else if (level < 50)
    sendTo("about a day.\n\r");
  else if (level < 100)
    sendTo("a few days *tops*.\n\r");
  else if (level < 200)
    sendTo("about a week.\n\r");
  else if (level < 400)
    sendTo("only a couple of weeks.\n\r");
  else if (level < 800)
    sendTo("around a month.\n\r");

  return;
}

void TBeing::describeComponentSpell(const TComponent *obj, int learn) const
{
  if (!hasClass(CLASS_MAGIC_USER) && !hasClass(CLASS_CLERIC) &&
      !hasClass(CLASS_RANGER)  && !hasClass(CLASS_DEIKHAN))
    learn /= 3;

//  int level = GetApprox(getSkillLevel(SKILL_EVALUATE), learn);

  char capbuf[160];
  strcpy(capbuf, objs(obj));

#if 0
  if (obj->getMagicLevel() > level) {
    sendTo(COLOR_OBJECTS, "You can tell nothing about the spell %s is used for.\n\r", uncap(capbuf));
    return;
  }
#endif

  int which = obj->getComponentSpell();

  if (which >= 0 && discArray[which])
    sendTo(COLOR_OBJECTS, "%s is used for: %s.\n\r", cap(capbuf),
          discArray[which]->name);

  return;
}

void TBeing::describeMaterial(const TThing *t)
{
  int mat = t->getMaterial();
  char mat_name[40];

  strcpy(mat_name, good_uncap(material_nums[mat].mat_name).c_str());

  if (dynamic_cast<const TBeing *>(t))
    sendTo(COLOR_MOBS, "%s has a skin type of %s.\n\r", good_cap(t->getName()).c_str(), mat_name);
  else
    sendTo(COLOR_OBJECTS, "%s is made of %s.\n\r", good_cap(t->getName()).c_str(), mat_name);

  describeMaterial(mat);
}

void TBeing::describeMaterial(int mat)
{
  //  int mat = t->getMaterial();
  char mat_name[40];

  strcpy(mat_name, good_uncap(material_nums[mat].mat_name).c_str());

  sendTo("%s is %d%% susceptible to slash attacks.\n\r",
     good_cap(mat_name).c_str(),
     material_nums[mat].cut_susc);
  sendTo("%s is %d%% susceptible to pierce attacks.\n\r",
     good_cap(mat_name).c_str(),
     material_nums[mat].pierced_susc);
  sendTo("%s is %d%% susceptible to blunt attacks.\n\r",
     good_cap(mat_name).c_str(),
     material_nums[mat].smash_susc);
  sendTo("%s is %d%% susceptible to flame attacks.\n\r",
     good_cap(mat_name).c_str(),
     material_nums[mat].burned_susc);
  sendTo("%s is %d%% susceptible to acid attacks.\n\r",
     good_cap(mat_name).c_str(),
     material_nums[mat].acid_susc);
  sendTo("%s is %d%% susceptible to water erosion, and suffers %d damage per erosion.\n\r",
     good_cap(mat_name).c_str(),
     material_nums[mat].water_susc%10 * 10,
     material_nums[mat].water_susc/10);
  sendTo("%s is %d%% susceptible to fall shock, and suffers %d damage per shock.\n\r",
     good_cap(mat_name).c_str(),
     material_nums[mat].fall_susc%10 *10,
     material_nums[mat].fall_susc/10);
  sendTo("%s has a hardness of %d units.\n\r",
     good_cap(mat_name).c_str(),
     material_nums[mat].hardness);
  sendTo("%s has a compaction ratio of %d:1.\n\r",
     good_cap(mat_name).c_str(),
     material_nums[mat].vol_mult);
  sendTo("%s is %sconsidered a conductive material.\n\r",
     good_cap(mat_name).c_str(),
     (material_nums[mat].conductivity ? "" : "not "));
}

void TBeing::sendRoomName(TRoom *rp) const
{
  unsigned int rFlags = rp->getRoomFlags();
  Descriptor *d = desc;
  char clientBuf[20];

  if (!d)
    return;

  sprintf(clientBuf, "\200%d|", CLIENT_ROOMNAME);

  if (IS_SET(desc->plr_color, PLR_COLOR_ROOM_NAME)) {
    if (hasColorStrings(this, rp->getName(), 2)) {
      sendTo(COLOR_ROOM_NAME,"%s%s%s%s%s%s\n\r", 
                d->client ? clientBuf : "",
                dynColorRoom(rp, 1, TRUE).c_str(),
                norm(), red(), 
                ((rFlags & ROOM_NO_HEAL) ? "     [NOHEAL]" :
                ((rFlags & ROOM_HOSPITAL) ? "     [HOSPITAL]" :
                ((rFlags & ROOM_ARENA) ? "     [ARENA]" :
                " "))), norm());
    } else {
      sendTo(COLOR_ROOM_NAME,"%s%s%s%s%s%s%s\n\r", 
                d->client ? clientBuf : "",  
                addColorRoom(rp, 1).c_str(),
                rp->getName(), norm(), red(),  
                ((rFlags & ROOM_NO_HEAL) ? "     [NOHEAL]" :
                ((rFlags & ROOM_HOSPITAL) ? "     [HOSPITAL]" :
                ((rFlags & ROOM_ARENA) ? "     [ARENA]" :
                " "))), norm());
    }
  } else {
    if (hasColorStrings(this, rp->getName(), 2)) {
      sendTo(COLOR_BASIC,"%s%s%s%s%s%s\n\r", 
              d->client ? clientBuf : "", purple(), 
              colorString(this, desc, rp->getName(), NULL, COLOR_NONE, FALSE).c_str(),
              red(),
              ((rFlags & ROOM_NO_HEAL) ? "     [NOHEAL]" :
              ((rFlags & ROOM_HOSPITAL) ? "     [HOSPITAL]" :
              ((rFlags & ROOM_HOSPITAL) ? "     [ARENA]" :
             " "))), norm());
    } else {
      sendTo(COLOR_BASIC,"%s%s%s%s%s%s\n\r", d->client ? clientBuf : "", 
             purple(),rp->getName(), red(),
             ((rFlags & ROOM_NO_HEAL) ? "     [NOHEAL]" :
             ((rFlags & ROOM_HOSPITAL) ? "     [HOSPITAL]" :
             ((rFlags & ROOM_HOSPITAL) ? "     [ARENA]" :
             " "))), norm());
    }
  }
  if (isImmortal() && (desc->prompt_d.type & PROMPT_BUILDER_ASSISTANT)) {
    sendTo("{ %s%s%s%s%s%s%s%s%s%s%s%s%s%s}\n\r",
           (rFlags == 0 ?
           "--none-- "       : ""),
           (!(rFlags & (ROOM_ALWAYS_LIT | ROOM_NO_MOB    | ROOM_INDOORS |
                        ROOM_PEACEFUL   | ROOM_NO_STEAL  | ROOM_NO_SUM  |
                        ROOM_NO_MAGIC   | ROOM_NO_PORTAL | ROOM_SILENCE |
                        ROOM_NO_ORDER   | ROOM_NO_FLEE   | ROOM_HAVE_TO_WALK)) &&
            (rFlags > 0)                 ? "--others-- "     : ""),
           ((rFlags & ROOM_ALWAYS_LIT)   ? "[Light] "        : ""),
           ((rFlags & ROOM_NO_MOB)       ? "[!Mob] "         : ""),
           ((rFlags & ROOM_INDOORS)      ? "[Indoors] "      : ""),
           ((rFlags & ROOM_PEACEFUL)     ? "[Peaceful] "     : ""),
           ((rFlags & ROOM_NO_STEAL)     ? "[!Steal] "       : ""),
           ((rFlags & ROOM_NO_SUM)       ? "[!Summon] "      : ""),
           ((rFlags & ROOM_NO_MAGIC)     ? "[!Magic] "       : ""),
           ((rFlags & ROOM_NO_PORTAL)    ? "[!Portal] "      : ""),
           ((rFlags & ROOM_SILENCE)      ? "[Silent] "       : ""),
           ((rFlags & ROOM_NO_ORDER)     ? "[!Order] "       : ""),
           ((rFlags & ROOM_NO_FLEE)      ? "[!Flee] "        : ""),
           ((rFlags & ROOM_HAVE_TO_WALK) ? "[Have-To-Walk] " : ""));
  }
}

void TBeing::sendRoomDesc(TRoom *rp) const
{
  if (hasColorStrings(this, rp->getDescr(), 2)) {
    sendTo(COLOR_ROOMS, "%s%s", dynColorRoom(rp, 2, TRUE).c_str(), norm());
  } else {
    sendTo(COLOR_ROOMS, "%s%s%s", addColorRoom(rp, 2).c_str(), rp->getDescr(), norm());
  }
}

void TBeing::describeTrapEffect(const TTrap *, int) const
{
  if (!hasClass(CLASS_THIEF))
    return;

  // this tells things like the triggers, why let them know these?
  return;
}

void TBeing::describeTrapLevel(const TTrap *obj, int learn) const
{
  if (!hasClass(CLASS_THIEF))
    return;

  int level = GetApprox(obj->getTrapLevel(), learn);
  level = max(level,0);

  sendTo(COLOR_OBJECTS, "%s seems to be a %s level trap.\n\r", 
       good_cap(objs(obj)).c_str(), numberAsString(level).c_str());
}

void TBeing::describeTrapCharges(const TTrap *obj, int learn) const
{
  if (!hasClass(CLASS_THIEF))
    return;

  int level = GetApprox(obj->getTrapCharges(), learn);
  level = max(level,0);

  sendTo(COLOR_OBJECTS, "%s seems to have %d charge%s left.\n\r", 
       good_cap(objs(obj)).c_str(), level, level == 1 ? "" : "s");
}

void TBeing::describeTrapDamType(const TTrap *obj, int) const
{
  if (!hasClass(CLASS_THIEF))
    return;

  sendTo(COLOR_OBJECTS, "You suspect %s is %s %s trap.\n\r", 
       good_uncap(objs(obj)).c_str(),
       startsVowel(trap_types[obj->getTrapDamType()]) ? "an" : "a",
       good_uncap(trap_types[obj->getTrapDamType()]).c_str());
}

void TBeing::doSpells(const char *argument)
{
  char buf[MAX_STRING_LENGTH * 2], buffer[MAX_STRING_LENGTH * 2];
  char learnbuf[64];
  spellNumT i;
  unsigned int j, l;
  Descriptor *d;
  CDiscipline *cd;
  char arg[200], arg2[200], arg3[200];
  int subtype=0, types[4], type=0, badtype=0, showall=0;
  discNumT das;
  TThing *primary=heldInPrimHand(), *secondary=heldInSecHand();
  TThing *belt=equipment[WEAR_WAISTE];
  TComponent *item=NULL;
  int totalcharges;
  wizardryLevelT wizlevel = getWizardryLevel();
  TThing *t1, *t2;
  TRealContainer *tContainer;

  struct {
    TThing *where;
    wizardryLevelT wizlevel;
  } search[] = {
      {primary  , WIZ_LEV_COMP_PRIM_OTHER_FREE},
      {secondary, WIZ_LEV_COMP_EITHER         },
      {stuff    , WIZ_LEV_COMP_INV            },
      {belt     , WIZ_LEV_COMP_BELT           }
  };

  if (!(d = desc))
    return;

  *buffer = '\0';

  if (!*argument)
    memset(types, 1, sizeof(int) * 4);      
  else {
    memset(types, 0, sizeof(int) * 4);

    three_arg(argument, arg, arg2, arg3);    

    if (*arg3 && is_abbrev(arg3, "all"))
      showall = 1;

    if (*arg2) {
      if (is_abbrev(arg2, "all"))
        showall = 1;
      else if (is_abbrev(arg2, "targeted"))
        subtype = 1;
      else if (is_abbrev(arg2, "nontargeted"))
        subtype = 2;
      else
        badtype = 1;
    }
    
    if (is_abbrev(arg, "offensive")) {
      if(!subtype || subtype == 1)
        types[0] = 1;
      if(!subtype || subtype == 2)
        types[1] = 1;
    } else if (is_abbrev(arg, "utility")) {
      if(!subtype || subtype == 1)
        types[2] = 1;
      if(!subtype || subtype == 2)
        types[3] = 1;      
    } else
      badtype = 1;
    
    if (badtype) {
      sendTo("You must specify a valid spell type.\n\r");
      sendTo("Syntax: spells <offensive|utility> <targeted|nontargeted> <all>.\n\r");
      return;
    }
  }

  vector<skillSorter>skillSortVec(0);

  for (i = MIN_SPELL; i < MAX_SKILL; i++) {
    if (hideThisSpell(i) || !discArray[i]->minMana)
      continue;

    skillSortVec.push_back(skillSorter(this, i));
  }
  
  // sort it into proper order
  sort(skillSortVec.begin(), skillSortVec.end(), skillSorter());

  for (type = 0; type < 4;++type) {
    if (!types[type])
      continue;

    if (*buffer)
      strcat(buffer, "\n\r");

    switch (type) {
      case 0:
        strcat(buffer, "Targeted offensive spells:\n\r");
        break;
      case 1:
        strcat(buffer, "Non-targeted offensive spells:\n\r");
        break;
      case 2:
        strcat(buffer, "Targeted utility spells:\n\r");
        break;
      case 3:
        strcat(buffer, "Non-targeted utility spells:\n\r");
        break;
    }

    for (j = 0; j < skillSortVec.size(); j++) {
      i = skillSortVec[j].theSkill;
      das = getDisciplineNumber(i, FALSE);
      if (das == DISC_NONE) {
        vlogf(5, "Bad disc for skill %d in doSpells", i);
        continue;
      }
      cd = getDiscipline(das);
      
      // getLearnedness is -99 for an unlearned skill, make it seem like a 0
      int tmp_var = ((!cd || cd->getLearnedness() <= 0) ? 0 : cd->getLearnedness());
      tmp_var = min((int) MAX_DISC_LEARNEDNESS, tmp_var);

      switch (type) {
        case 0: // single target offensive
          if (!(discArray[i]->targets & TAR_VIOLENT) ||
              (discArray[i]->targets & TAR_AREA))
            continue;
          break;
        case 1: // area offensive
          if (!(discArray[i]->targets & TAR_VIOLENT) ||
              !(discArray[i]->targets & TAR_AREA))
            continue;
          break;
        case 2: // targeted utility
          if ((discArray[i]->targets & TAR_VIOLENT) ||
              !(discArray[i]->targets & TAR_CHAR_ROOM))
            continue;
          break;
        case 3: // non-targeted utility
          if ((discArray[i]->targets &  TAR_VIOLENT) ||
              (discArray[i]->targets & TAR_CHAR_ROOM))
            continue;
          break;
      }

      // can't we say if !cd, continue here?
      if (cd && !cd->ok_for_class && getSkillValue(i) <= 0) 
        continue;

      totalcharges = 0;
      item = NULL;
      
      for (l = 0; l < 4; l++) {
        if (search[l].where && wizlevel >= search[l].wizlevel) {
          for (t1 = search[l].where; t1; t1 = t1->nextThing) {
            if (!(item = dynamic_cast<TComponent *>(t1)) &&
                (!(tContainer = dynamic_cast<TRealContainer *>(item)) ||
                 !tContainer->isClosed())) {
              for (t2 = t1->stuff; t2; t2 = t2->nextThing) {
                if ((item = dynamic_cast<TComponent *>(t2)) &&
                    item->getComponentSpell() == i && 
                    item->isComponentType(COMP_SPELL))
                  totalcharges += item->getComponentCharges();
              }
            } else if (item->getComponentSpell() == i && 
                       item->isComponentType(COMP_SPELL))
              totalcharges += item->getComponentCharges();
          }
        }
      }

      if ((getSkillValue(i) <= 0) &&
          (!tmp_var || (discArray[i]->start - tmp_var) > 0)) {
        if (!showall) 
          continue;

        sprintf(buf, "%s%-22.22s%s  (Learned: %s)", 
                cyan(), discArray[i]->name, norm(),
                skill_diff(discArray[i]->start - tmp_var));
      } else if (discArray[i]->toggle && 
                 !hasQuestBit(discArray[i]->toggle)) {
        if (!showall) 
          continue;

        sprintf(buf, "%s%-22.22s%s  (Learned: Quest)",
                cyan(), discArray[i]->name, norm());
      } else { 
        if (getMaxSkillValue(i) < MAX_SKILL_LEARNEDNESS) {
          if (discArray[i]->startLearnDo > 0) {
            sprintf(learnbuf, "%.9s/%.9s", how_good(getSkillValue(i)),
                    how_good(getMaxSkillValue(i))+1);
            sprintf(buf, "%s%-22.22s%s %-19.19s",
                    cyan(), discArray[i]->name, norm(), 
                    learnbuf);
          } else {
            sprintf(buf, "%s%-22.22s%s %-19.19s",
                    cyan(), discArray[i]->name, norm(), 
                    how_good(getSkillValue(i)));
          }
        } else {
          sprintf(buf, "%s%-22.22s%s %-19.19s",
                  cyan(), discArray[i]->name, norm(), 
                  how_good(getSkillValue(i)));
        }
        unsigned int comp;

        for (comp = 0; (comp < CompInfo.size()) &&
                       (i != CompInfo[comp].spell_num); comp++);

        if (comp != CompInfo.size() && CompInfo[comp].comp_num >= 0) {
          sprintf(buf + strlen(buf), "   [%3i] %s",  totalcharges, 
                  obj_index[real_object(CompInfo[comp].comp_num)].short_desc);
        }         
      }
        strcat(buf, "\n\r");
        
      if (strlen(buf) + strlen(buffer) > (MAX_STRING_LENGTH * 2) - 2)
        break;

      strcat(buffer, buf);
    } 
  }
  d->page_string(buffer, 0);
  return;
}

void TBeing::doPrayers(const char *argument)
{
  char buf[MAX_STRING_LENGTH * 2] = "\0";
  char buffer[MAX_STRING_LENGTH * 2] = "\0";
  char learnbuf[64];
  spellNumT i;
  unsigned int j, l;
  Descriptor *d;
  CDiscipline *cd;
  char arg[200], arg2[200], arg3[200];
  int subtype=0, types[4], type=0, badtype=0, showall=0;
  discNumT das;
  TThing *primary = heldInPrimHand(), *secondary = heldInSecHand();
  TThing *belt = equipment[WEAR_WAISTE];
  TComponent *item = NULL;
  int totalcharges;
  wizardryLevelT wizlevel = getWizardryLevel();
  TThing *t1, *t2;
  TRealContainer *tContainer;

  struct {
    TThing *where;
    wizardryLevelT wizlevel;
  } search[]={{primary, WIZ_LEV_COMP_PRIM_OTHER_FREE}, {secondary, WIZ_LEV_COMP_EITHER}, {stuff, WIZ_LEV_COMP_INV}, {belt, WIZ_LEV_COMP_BELT}};

  if (!(d = desc))
    return;

  if(!*argument)
    memset(types, 1, sizeof(int)*4);      
  else {
    memset(types, 0, sizeof(int)*4);

    three_arg(argument, arg, arg2, arg3);    

    if (*arg3 && is_abbrev(arg3, "all"))
      showall = 1;

    if (*arg2){
      if (is_abbrev(arg2, "all")) 
        showall=1;
        else if(is_abbrev(arg2, "targeted")) 
        subtype=1;
        else if(is_abbrev(arg2, "nontargeted")) 
        subtype=2;
        else badtype=1;
    }      
    if (is_abbrev(arg, "offensive")){
        if (!subtype || subtype==1) 
        types[0] = 1;

        if (!subtype || subtype==2) 
        types[1] = 1;
    } else if(is_abbrev(arg, "utility")) {
        if (!subtype || subtype==1) 
        types[2] = 1;
        
      if (!subtype || subtype==2) 
        types[3] = 1;      
    } else  
      badtype = 1;
      
    if (badtype) {
        sendTo("You must specify a valid spell type.\n\r");
        sendTo("Syntax: spells <offensive|utility> <targeted|nontargeted> <all>.\n\r");
        return;
    }
  }

  vector<skillSorter>skillSortVec(0);

  for (i = MIN_SPELL; i < MAX_SKILL; i++) {
    if (hideThisSpell(i) || !discArray[i]->minMana)
      continue;
    skillSortVec.push_back(skillSorter(this, i));
  }  
    
  sort(skillSortVec.begin(), skillSortVec.end(), skillSorter());

  for (type = 0;type < 4;++type) {
    if (!types[type])
      continue;

    if(*buffer)
        strcat(buffer, "\n\r");

    switch(type){
      case 0:
        strcat(buffer, "Targeted offensive spells:\n\r");
        break;
      case 1:
        strcat(buffer, "Non-targeted offensive spells:\n\r");
        break;
      case 2:
        strcat(buffer, "Targeted utility spells:\n\r");
        break;
      case 3:
        strcat(buffer, "Non-targeted utility spells:\n\r");
        break;
    }
    for (j = 0; j < skillSortVec.size(); j++) {
      i = skillSortVec[j].theSkill;
      das = getDisciplineNumber(i, FALSE);
      if (das == DISC_NONE) {
        vlogf(5, "Bad disc for skill %d in doPrayers", i);
          continue;
      }
      cd = getDiscipline(das);
        
      // getLearnedness is -99 for an unlearned skill, make it seem like a 0
      int tmp_var = ((!cd || cd->getLearnedness() <= 0) ? 0 : cd->getLearnedness());
      tmp_var = min((int) MAX_DISC_LEARNEDNESS, tmp_var);

      switch (type) {
        case 0: // single target offensive
          if(!(discArray[i]->targets & TAR_VIOLENT) ||
                (discArray[i]->targets & TAR_AREA))
                continue;
  
          break;
        case 1: // area offensive
          if(!(discArray[i]->targets & TAR_VIOLENT) ||
             !(discArray[i]->targets & TAR_AREA))
            continue;
  
          break;
         case 2: // targeted utility
          if((discArray[i]->targets & TAR_VIOLENT) ||
             !(discArray[i]->targets & TAR_CHAR_ROOM))
            continue;

          break;
        case 3: // non-targeted utility
          if((discArray[i]->targets &  TAR_VIOLENT) ||
             (discArray[i]->targets & TAR_CHAR_ROOM))
            continue;

          break;
      }
      // can't we say if !cd, continue here?
      if (cd && !cd->ok_for_class && getSkillValue(i) <= 0) 
        continue;

      totalcharges = 0;
      item = NULL;
        
      for (l = 0; l < 4; ++l){
        if (search[l].where && wizlevel >= search[l].wizlevel) {
          for (t1 = search[l].where; t1; t1 = t1->nextThing) {
            if(!(item=dynamic_cast<TComponent *>(t1)) &&
               (!(tContainer = dynamic_cast<TRealContainer *>(item)) ||
                !tContainer->isClosed())) {
              for (t2 = t1->stuff; t2; t2 = t2->nextThing) {
                if ((item=dynamic_cast<TComponent *>(t2)) &&
                  item->getComponentSpell() == i && 
                  item->isComponentType(COMP_SPELL))
                  totalcharges += item->getComponentCharges();
                }
              } else if(item->getComponentSpell() == i && 
                item->isComponentType(COMP_SPELL))
                
              totalcharges += item->getComponentCharges();
            }
          }
        }
        if ((getSkillValue(i) <= 0) &&
          (!tmp_var || (discArray[i]->start - tmp_var) > 0)) {
          
        if (!showall) 
          continue;

        sprintf(buf, "%s%-22.22s%s  (Learned: %s)",  cyan(), discArray[i]->name, norm(), skill_diff(discArray[i]->start - tmp_var));
      } else if (discArray[i]->toggle && !hasQuestBit(discArray[i]->toggle)) {
          if (!showall) 
          continue;

          sprintf(buf, "%s%-22.22s%s  (Learned: Quest)", cyan(), discArray[i]->name, norm());
      } else { 
        if (getMaxSkillValue(i) < MAX_SKILL_LEARNEDNESS) {
          if (discArray[i]->startLearnDo > 0) {
            sprintf(learnbuf, "%.9s/%.9s", how_good(getSkillValue(i)), how_good(getMaxSkillValue(i))+1);
            sprintf(buf, "%s%-22.22s%s %-19.19s", cyan(), discArray[i]->name, norm(), learnbuf);
          } else 
            sprintf(buf, "%s%-22.22s%s %-19.19s", cyan(), discArray[i]->name, norm(), how_good(getSkillValue(i)));   
        } else 
          sprintf(buf, "%s%-22.22s%s %-19.19s", cyan(), discArray[i]->name, norm(), how_good(getSkillValue(i)));
            
        unsigned int comp;

        for (comp = 0; (comp < CompInfo.size()) && (i != CompInfo[comp].spell_num);comp++);

        if (comp != CompInfo.size() && CompInfo[comp].comp_num >= 0) 
          sprintf(buf + strlen(buf), "   [%2i] %s",  totalcharges, obj_index[real_object(CompInfo[comp].comp_num)].short_desc); 
      }
      strcat(buf, "\n\r");
          
      if (strlen(buf) + strlen(buffer) > (MAX_STRING_LENGTH * 2) - 2)
        break;

      strcat(buffer, buf);
    } 
  }
  d->page_string(buffer, 0);
  return;
}
