//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "colorstring.cc" - the colorstring function
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "colorstring.h"


sstring stripColorCodes(const sstring &s)
{
  sstring buf = "";
  unsigned int len;

  len = s.length();
  
  for(unsigned int i=0;i<len;++i){
    if(s[i] == '<') {
      i+=2;
      continue;
    }
    
    buf += s[i];
  }

  return buf;
}


bool hasColorStrings(const TBeing *mob, const sstring &arg, int field)
{
  sstring s;

  if(arg.empty())
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
  for(unsigned int i=0;i<s.size()-1;++i){
    if ((s[i] == '<') && (s[i+2] == '>')) {
      switch (s[i+1]) {
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
          return true;
        case 'h':
        case 'H':
        case 'n':
        case 'N':
        default:
          break;
      }
    }
  }
  return FALSE;
}

// takes the sstring given by arg, replaces any <m> or <M> in it with
// ting's name.  Colorizes as appropriate for me/ch.  Undoes any color
// changes that were made by insertion of ting's name sstring also.
sstring addNameToBuf(const TBeing *me, const Descriptor *ch, const TThing *ting, const sstring &arg, colorTypeT lev) 
{
  unsigned int s;
  unsigned int len;
  sstring buf;
  char tmp[80];
  bool y = FALSE;
  int x = 0;

  if (!ch)
    return arg;

  len = arg.length();
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
            strcpy(tmp, sstring(tmp).cap().c_str());
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
          // if there is a color sstring, it will pick it up after <m>
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

sstring nameColorString(TBeing *me, Descriptor *ch, const sstring &arg, int *flag, colorTypeT, bool noret)
{
  unsigned int len, s;
  sstring buf;
          
  len = arg.length();

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
            buf += sstring(me->getName()).cap();
	    if(me->isPkChar())
	      buf+=" (PK)";

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

const sstring colorString(const TBeing *me, const Descriptor *ch, const sstring &arg, int *flag, colorTypeT lev, bool end, bool noret)
{
// (me = who to, ch is the desc, arg = arg, flag = ?, int lev = desired color
//  level, end = whether to send terminator at end of sstring..false if in
//  middle of senstence (color a mob or something).. used in act() 
//  noret is overloaded to add a \n\r to the end of the buf, it defaults
//  to no so if you dont pass anything, it will not return -Cos
  int len, s;
  sstring buf;
  bool colorize = TRUE;
  bool addNorm = FALSE;

  if (arg.empty())
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
      vlogf(LOG_BUG,"Colorsstring with a default COLOR setting");
      colorize = TRUE;
      break;
  }
  len = arg.length();

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
              buf += sstring(me->getName()).cap();
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
