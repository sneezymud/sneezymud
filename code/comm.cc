//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//   "comm.cc" - All functions and routines related to the central game
//               loop
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "systemtask.h"
#include "socket.h"

#include <csignal>
#include <cstdarg>

extern "C" {
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/param.h>
#include <arpa/inet.h>
}

const int PACKET_BUFFER_SIZE = 40960;
const int MAX_HOSTNAME =   256;
const int MAX_SOCKET_WRITE =  40960;

const int PROMPT_DONT_SEND = -1;

SystemTask *systask;

// local globals 

int noSpecials = 0;		// Suppress ass. of special routines 
time_t Uptime;			// time that the game has been up 

char hostlist[MAX_BAN_HOSTS][40];	// list of sites to ban
char hostLogList[MAX_BAN_HOSTS][40];
int numberhosts;
int numberLogHosts;
int gamePort;

extern void save_all();
extern int run_the_game();


// Init sockets, run game, and cleanup sockets 
int run_the_game()
{
  vlogf(LOG_MISC, "Signal trapping.");
  signalSetup();

  vlogf(LOG_MISC, "Opening mother connection.");
  gSocket = new TSocket(gamePort);
  gSocket->initSocket();

  bootDb();

  vlogf(LOG_MISC, "Entering game loop.");

  systask = new SystemTask();
  gSocket->gameLoop();
  gSocket->closeAllSockets();

  vlogf(LOG_MISC, "Normal termination of game.");
  delete gSocket;

  return FALSE;
}

void zoneData::nukeMobs()
{
  TThing *t, *t2;
  TBeing *mob, *mob2;
  wearSlotT i;
  
  if (!nuke_inactive_mobs)
    return;

  for (mob = character_list; mob; mob = mob2) {
    mob2 = mob->next;
    if (mob->isPc())
      continue;
    if (mob->specials.zone != zone_nr)
      continue;

    // preserve charms, mounts, etc
    if (mob->master && mob->master->isPc())
      continue;
    if ((mob->spec == SPEC_BOUNTY_HUNTER) || dynamic_cast<TMonster *>(mob)->hatefield)
      continue;
    if (mob->isRideable())
      continue;

    // nuke the mob and equipment
    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      if (mob->equipment[i]) {
        TThing *t_obj = mob->unequip(i);
        delete t_obj;
        t_obj = NULL;
      }
    }
    for (t = mob->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      delete t;
    }
    delete mob;
    mob = NULL;
  }
}

void TBeing::sendTo(colorTypeT lev, const sstring &msg) const
{
  if (!desc)
    return;
  if (desc->connected == CON_WRITING)
    return;

  sstring messageBuffer = colorString(this, desc, msg, NULL, lev, FALSE);
  desc->output.putInQ(messageBuffer);
}

void TRoom::sendTo(colorTypeT lev, const sstring &text) const
{
  TThing *i;

  for (i = getStuff(); i; i = i->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(i);
    if (tbt && tbt->desc && !tbt->desc->connected) {
      if ((lev == COLOR_NEVER) || (lev == COLOR_NONE)) {
      } else {
        sstring messageBuffer = colorString(tbt, i->desc, text, NULL, lev, TRUE);
        tbt->desc->output.putInQ(messageBuffer);
      }
    }
  }
}

void TThing::sendTo(colorTypeT, const sstring &msg) const
{
}


void TBeing::sendTo(const sstring &msg) const
{
  if (msg.empty() || !desc)
    return;

  if (desc->connected == CON_WRITING)
    return;

  desc->output.putInQ(msg);
}

void save_all()
{
  descriptor_list->saveAll();
}

void sendToOutdoor(colorTypeT lev, const sstring &text, const sstring &text_tropic)
{
  Descriptor *i;
  TBeing *ch;

  if (!text.empty()) {
    for (i = descriptor_list; i; i = i->next) {
      if (!i->connected && (ch = i->character)) {
        if (ch->outside() && ch->awake() && ch->roomp  &&
              !(ch->isPlayerAction(PLR_MAILING | PLR_BUGGING))) {
          if ((lev == COLOR_NEVER) || (lev == COLOR_NONE)) {
            if (ch->roomp->isTropicalSector()) {
              i->output.putInQ(text_tropic);
            } else {
              i->output.putInQ(text);
            }
          } else {
            sstring buf;
            if (ch->roomp->isTropicalSector()) {
              buf = colorString(ch, i, text_tropic, NULL, lev, FALSE);
            } else {
              buf = colorString(ch, i, text, NULL, lev, FALSE);
            }
            i->output.putInQ(buf);
          }
        }
      }
    }
  }
}


void sendToExcept(char *text, TBeing *ch)
{
  Descriptor *i;

  if (text)
    for (i = descriptor_list; i; i = i->next)
      if (ch->desc != i && !i->connected)
        i->output.putInQ(text);
}

void sendToRoom(colorTypeT color, const char *text, int room)
{
  TThing *i;

  if (!real_roomp(room)) {
    vlogf(LOG_MISC, "BOGUS room %d in sendToRoom", room);
    return;
  }
  if (text) {
    for (i = real_roomp(room)->getStuff(); i; i = i->nextThing) {
      TBeing *tbt = dynamic_cast<TBeing *>(i);
      if (tbt && tbt->desc && !tbt->desc->connected && tbt->awake()) {
        sstring buf = colorString(tbt, tbt->desc, text, NULL, color, FALSE);
        tbt->desc->output.putInQ(buf);
      }
    }
  }
}

void sendToRoom(const char *text, int room)
{
  TThing *i;

  if (!real_roomp(room)) {
    vlogf(LOG_MISC, "BOGUS room %d in sendToRoom", room);
    return;
  }
  if (text) {
    for (i = real_roomp(room)->getStuff(); i; i = i->nextThing) {
      TBeing *tbt = dynamic_cast<TBeing *>(i);
      if (!tbt)
        continue;
      if (tbt->desc && !tbt->desc->connected && tbt->awake())
        tbt->desc->output.putInQ(text);
    }
  }
}

void sendrpf(colorTypeT color, TRoom *rp, const char *msg, ...)
{
  char messageBuffer[MAX_STRING_LENGTH];
  va_list ap;

  if (rp && msg) {
    va_start(ap, msg);
    vsprintf(messageBuffer, msg, ap);
    va_end(ap);

    sendrpf(0, color, rp, messageBuffer);
  }
}

void sendrpf(int tslevel, colorTypeT color, TRoom *rp, const char *msg,...)
{
  char messageBuffer[MAX_STRING_LENGTH];
  va_list ap;
  TThing *i;

  if (rp && msg) {
    va_start(ap, msg);
    vsprintf(messageBuffer, msg, ap);
    va_end(ap);
    for (i = rp->getStuff(); i; i = i->nextThing) {
      TBeing *tbt = dynamic_cast<TBeing *>(i);
      if (tbt && tbt->desc && !tbt->desc->connected && tbt->awake() &&
          tbt->GetMaxLevel() > tslevel)
        tbt->desc->output.putInQ(colorString(tbt, tbt->desc, messageBuffer, NULL, color, TRUE));
    }
  }
}

void sendrpf(TRoom *rp, const char *msg, ...)
{
  char messageBuffer[MAX_STRING_LENGTH];
  va_list ap;

  if (rp && msg) {
    va_start(ap, msg);
    vsprintf(messageBuffer, msg, ap);
    va_end(ap);

    sendrpf(0, rp, messageBuffer);
  }
}

void sendrpf(int tslevel, TRoom *rp, const char *msg,...)
{
  char messageBuffer[MAX_STRING_LENGTH];
  va_list ap;
  TThing *i;
  
  if (rp && msg) {
    va_start(ap, msg);
    vsprintf(messageBuffer, msg, ap);
    va_end(ap);
    for (i = rp->getStuff(); i; i = i->nextThing) {
      TBeing *tbt = dynamic_cast<TBeing *>(i);
      if (tbt && tbt->desc && !tbt->desc->connected && tbt->awake() &&
          tbt->GetMaxLevel() > tslevel)
        tbt->desc->output.putInQ(colorString(tbt, tbt->desc, messageBuffer, NULL, COLOR_NONE, TRUE));

    }
  }
}


void sendrp_exceptf(TRoom *rp, TBeing *ch, const char *msg,...)
{
  char messageBuffer[MAX_STRING_LENGTH];
  va_list ap;
  TThing *i;

  if (rp && msg) {
    va_start(ap, msg);
    vsprintf(messageBuffer, msg, ap);
    va_end(ap);
    for (i = rp->getStuff(); i; i = i->nextThing) {
      TBeing *tbt = dynamic_cast<TBeing *>(i);
      if (tbt && tbt->desc && !tbt->desc->connected && (tbt != ch) && tbt->awake())
        tbt->desc->output.putInQ(messageBuffer);
    }
  }
}

void TRoom::sendTo(const sstring &text) const
{
  TThing *i;

  for (i = getStuff(); i; i = i->nextThing) {
    if (i->desc && !i->desc->connected)
      i->desc->output.putInQ(text);
  }
}

void TThing::sendTo(const sstring &) const
{
}

void colorAct(colorTypeT colorLevel, const sstring &str, bool hide, const TThing *t1, const TThing *obj, const TThing *t3, actToParmT type, const char *color, int tslevel)
{
  bool colorize = 0;
  const TThing *to;
  int which = 0;
  char buf[256];


  if(str.empty())
    return;

  if (!t1) {
    vlogf(LOG_MISC, "There is no char in coloract TOCHAR.");
    vlogf(LOG_MISC, "%s", str.c_str());
    return;
  }

  if (!t1->roomp) 
    return;

  if (!t3) {
    if (type == TO_VICT) {
      vlogf(LOG_MISC, "There is no victim in coloract TOVICT %s is char.", t1->getName());
      vlogf(LOG_MISC, "%s", str.c_str());
      return;
    } else if (type == TO_NOTVICT) {
      type = TO_ROOM;
      vlogf(LOG_MISC, "There is no victim in coloract TONOTVICT %s is char.", t1->getName());
      vlogf(LOG_MISC, "%s", str.c_str());
    }
  }

  if (type == TO_VICT) {
    to = t3;
    which = TRUE;
  } else if (type == TO_CHAR) {
    to = t1;
    which = TRUE;
  } else {
    which = FALSE;
    to = t1->roomp->getStuff();
  }

  if (!to->desc && ((type == TO_VICT) || (type == TO_CHAR))) 
    return;

  if (which) {
    switch (colorLevel) {
      case COLOR_ALWAYS:
      case COLOR_BASIC:   //allows for basic ansi
        colorize = TRUE;
        break;
      case COLOR_NONE:
      case COLOR_NEVER:
        colorize = FALSE;
        break;
      case COLOR_COMM:
        if (!(IS_SET(to->desc->plr_color, PLR_COLOR_COMM))) {
           colorize = FALSE;
        } else {
          colorize = TRUE;
        }
        break;
      case COLOR_OBJECTS:
        if (!(IS_SET(to->desc->plr_color, PLR_COLOR_OBJECTS))) 
           colorize = FALSE;
        else 
          colorize = TRUE;
        
        break;
      case COLOR_MOBS:
        if (!(IS_SET(to->desc->plr_color, PLR_COLOR_MOBS))) {
           colorize = FALSE;
        } else {
          colorize = TRUE;
        }
        break;
      case COLOR_ROOMS:
        if (!(IS_SET(to->desc->plr_color, PLR_COLOR_ROOMS))) {
           colorize = FALSE;
        } else {
          colorize = TRUE;
        }
        break;
      case COLOR_ROOM_NAME:
        if (!(IS_SET(to->desc->plr_color, PLR_COLOR_ROOM_NAME))) {
           colorize = FALSE;
        } else {
          colorize = TRUE;
        }
        break;
      case COLOR_SHOUTS:
        if (!(IS_SET(to->desc->plr_color, PLR_COLOR_SHOUTS))) {
           colorize = FALSE;
        } else {
          colorize = TRUE;
        }
        break;
      case COLOR_SPELLS:
        if (!(IS_SET(to->desc->plr_color, PLR_COLOR_SPELLS))) {
           colorize = FALSE;
        } else {
          colorize = TRUE;
        }
        break;
      case COLOR_LOGS:
        if (!(IS_SET(to->desc->plr_color, PLR_COLOR_LOGS))) {
           colorize = FALSE;
        } else {
          colorize = TRUE;
        }
        break;
      default:
        vlogf(LOG_MISC, "colorAct with a default COLOR setting");
        colorize = TRUE;
        break;
    }
    if (colorize) {
      sprintf(buf, "%s", str.c_str());
    } else {
      sprintf(buf, "%s", colorString(dynamic_cast<const TBeing *>(to), to->desc, str.c_str(), NULL, COLOR_NONE, TRUE).c_str());

    }
    act(buf, hide, t1, obj, t3, type, color);
  } else {
// TO_ROOM
// Doesnt work well if there are substitutes but if none its ok
    for (; to; to = to->nextThing) {
      const TBeing *tbto = dynamic_cast<const TBeing *>(to);
      if (!tbto || !tbto->desc || tbto->GetMaxLevel() <= tslevel) 
        continue;
    
      if (tbto == t1) 
        continue;
     
      if (tbto == t3 && type == TO_NOTVICT) 
        continue;
    
      colorAct(colorLevel, str, hide, t1, obj, tbto, TO_VICT, color, tslevel);
    }
  }
}

void act(const sstring &str, bool hide, const TThing *t1, const TThing *obj, const TThing *t3, actToParmT type, const char *color, int tslevel)
{
  register const char *strp;
  register char *point;
  register const char *i = NULL;
  const TThing *ttto;
  char buf[MAX_STRING_LENGTH];
  char namebuf[MAX_NAME_LENGTH];
  char lastColor[3];
  const char *codes = NULL;
  const char *codes2 = NULL;
  int hasLast = FALSE;
  int x = 0;
  personTypeT per;
  const TObj *tobj = NULL;
  sstring catstr;

  if(str.empty())
    return;

  if (!t1) {
    vlogf(LOG_MISC, "There is no char in act() TOCHAR.");
    vlogf(LOG_MISC, "%s", str.c_str());
    return;
  }
  if (!t1->roomp) 
    return;

  if (!t3) {
    if (type == TO_VICT) {
      vlogf(LOG_MISC, "There is no victim in act() TOVICT %s is char.", t1->getName());
      vlogf(LOG_MISC, "%s", str.c_str());
      return;
    } else if (type == TO_NOTVICT) {
      type = TO_ROOM;
      vlogf(LOG_MISC, "There is no victim in act() TONOTVICT %s is char.", t1->getName());
      vlogf(LOG_MISC, "%s", str.c_str());
    }
  }
  if (type == TO_VICT) 
    ttto = t3;
  else if (type == TO_CHAR) 
    ttto = t1;
  else {
    if (!t1->roomp)
      return;

    ttto = t1->roomp->getStuff();
  }
  memset(&buf, '\0', sizeof(buf));
  memset(lastColor, '\0', sizeof(lastColor));
  for (; ttto; ttto = ttto->nextThing) {
    const TBeing *to = dynamic_cast<const TBeing *>(ttto);
    if (to && to->desc && to->GetMaxLevel() > tslevel &&
          ((to != t1) || (type == TO_CHAR)) &&
          ((to != t3 || (t1 == t3 && type == TO_CHAR)) ||
               (type == TO_VICT) || (type == TO_ROOM)) &&
        (to->canSee(t1) || !hide) &&
	to->awake() && (to->desc->connected < MAX_CON_STATUS) && 
        !(to->isPlayerAction(PLR_MAILING | PLR_BUGGING))) {
      for (strp = str.c_str(), point = buf;;) {
        x = x + 1;
        codes = strp;
        if ((*codes == '<') && (*(++codes) != '<')) {
          codes2 = codes;
          if (*(++codes) == '>') {
              lastColor[0] = '<';
              lastColor[1] = *codes2;
              lastColor[2] = '>';

//            sprintf(lastColor, "<%s>", *codes2);
//            lastColor = codes2;
            hasLast = TRUE;
          }
        }
	if (*strp == '$') {
          const TBeing * tbtt;
	  switch (*(++strp)) {
	    case 'n':
              tbtt = dynamic_cast<const TBeing *>(t1);
              i = tbtt ? to->pers(t1) : to->objs(t1);
              if (x == 1 || (x == 4 && *lastColor)) {
                strcpy(namebuf, i);
                cap(namebuf);
                i = namebuf;
              }
              i = colorString(to, to->desc, i, NULL, tbtt ? COLOR_MOBS : COLOR_OBJECTS, FALSE).c_str();
	      break;
	    case 'P':
	    case 'N':
              if (!t3) {
                forceCrash("Bad act P or N. '%s'", str.c_str());
                return;
              }
              tbtt = dynamic_cast<const TBeing *>(t3);
              i = tbtt ? to->pers(t3) : to->objs(t3);
              if (x == 1 || (x == 4 && *lastColor)) {
                strcpy(namebuf, i);
                cap(namebuf);
                i = namebuf;
              }
              if ((type == TO_CHAR) && (t1 == t3)) {
                if (!strncmp(strp+1,"'s ",3)) {
                  i = "your";
                  strp += 2;
                } else if (!strncmp(strp+1," is",3)) {
                  i = "you are";
                  strp += 3;
                } else {
                  // first word = "You", otherwise "Yourself"
                  if (strlen(buf))
                    i = "yourself";
                  else
                    i = "you";
                }
              } else if ((type == TO_NOTVICT) && (t1 == t3)) {
                if (!strncmp(strp+1,"'s ",3)) {
                  i = t1->hshr();
                  strp += 2;
                } else if (strp != (str.c_str() + 1)) {
                  // "himself" if it isn't the first word in the sstring
                  char tmp_buffer[20];
                  sprintf(tmp_buffer, "%sself", t1->hmhr());
                  i = tmp_buffer;
                }
              }
              i = colorString(to, to->desc, i, NULL, tbtt ? COLOR_MOBS : COLOR_OBJECTS, FALSE).c_str();
              
	      break;
	    case 'g':
              i = t1->roomp->describeGround().c_str();
              break;
	    case 'G':
              if (!t3) {
                forceCrash("Bad act G. '%s'", str.c_str());
                return;
              }
              i = t3->roomp->describeGround().c_str();
              break;
	    case 'd': 
              per = ((to == t1) ? FIRST_PERSON : (!strlen(buf) ? THIRD_PERSON : SECOND_PERSON));
              i = t1->yourDeity(your_deity_val, per, (per == THIRD_PERSON) ? to : NULL).c_str();
              break;
            
	    case 'D':
              if (!t3) {
                forceCrash("Bad act D. '%s'", str.c_str());
                return;
              }
              i = t3->yourDeity(your_deity_val, ((to == t3) ? FIRST_PERSON : (strlen(buf) == 0 ? THIRD_PERSON : SECOND_PERSON))).c_str();
              break;
            case 'q':
              // is/are based on plurality of $o, $p
              if (!obj) {
                forceCrash("Bad act q. '%s'", str.c_str());
                return;
              }
              tobj = dynamic_cast<const TObj *>(obj);
              if (tobj)
                i = tobj->isPluralItem() ? "are" : "is";
              else
                i = "is";
              break;
            case 'Q':
              // a verb modifier so can do "$o look$Q happy" for plurality
              if (!obj) {
                forceCrash("Bad act Q. '%s'", str.c_str());
                return;
              }
              tobj = dynamic_cast<const TObj *>(obj);
              if (tobj)
                i = tobj->isPluralItem() ? "" : "s";
              else
                i = "s";
              break;
            case 'r':
              // is/are based on plurality of $n
              if (!t1) {
                forceCrash("Bad act r. '%s'", str.c_str());
                return;
              }
              tobj = dynamic_cast<const TObj *>(t1);
              if (tobj)
                i = tobj->isPluralItem() ? "are" : "is";
              else
                i = "is";
              break;
            case 'R':
              // a verb modifier so can do "$n look$Q happy" for plurality
              if (!t1) {
                forceCrash("Bad act R. '%s'", str.c_str());
                return;
              }
              tobj = dynamic_cast<const TObj *>(t1);
              if (tobj)
                i = tobj->isPluralItem() ? "" : "s";
              else
                i = "s";
              break;
	    case 'm':
              if (to->canSee(t1))
                i = t1->hmhr();
              else
                i = "someone";
	      break;
	    case 'M':
              if (!t3) {
                forceCrash("Bad act M. '%s'", str.c_str());
                return;
              }
              if ((type == TO_CHAR) && (t1 == t3)) 
                i = "yourself";
              else if (to->canSee(t3))
                i = t3->hmhr();
              else
                i = "someone";
	      break;
	    case 's':
              if (to->canSee(t1))
                i = t1->hshr();
              else
                i = "their";
	      break;
	    case 'S':
              if (!t3) {
                forceCrash("Bad act S. '%s'", str.c_str());
                return;
              }
              if (to->canSee(t3))
                i = t3->hshr();
              else
                i = "their";
	      break;
	    case 'e':
              if (to->canSee(t1))
                i = t1->hssh();
              else
                i = "it";
	      break;
	    case 'E':
              if (!t3) {
                forceCrash("Bad act E. '%s'", str.c_str());
                return;
              }
              if (to->canSee(t3))
                i = t3->hssh();
              else
                i = "it";
	      break;
	    case 'o':
              if (!obj) {
                forceCrash("Bad act o. '%s'", str.c_str());
                return;
              }
	      i = dynamic_cast<const TBeing *>(obj) ? to->persfname(obj).c_str() : to->objn(obj).c_str();
	      break;
	    case 'O':
              if (!t3) {
                forceCrash("Bad act O. '%s'", str.c_str());
                return;
              }
	      i = dynamic_cast<const TBeing *>(t3) ? to->persfname(t3).c_str() : to->objn(t3).c_str();
	      break;
	    case 'p':
              if (!obj) {
                forceCrash("Bad act p. '%s'", str.c_str());
                return;
              }
              tbtt = dynamic_cast<const TBeing *>(obj);
              i = tbtt ? to->pers(obj) : to->objs(obj);
              if (x == 1 || (x == 4 && *lastColor)) {
                strcpy(namebuf, i);
                cap(namebuf);
                i = namebuf;
              }
              i = colorString(to, to->desc, i, NULL, tbtt ? COLOR_MOBS : COLOR_OBJECTS, FALSE).c_str();
	      break;
	    case 'a':
              if (!obj) {
                forceCrash("Bad act a. '%s'", str.c_str());
                return;
              }
	      i = obj->sana();
	      break;
	    case 'A':
              if (!t3) {
                forceCrash("Bad act A. '%s'", str.c_str());
                return;
              }
	      i = t3->sana();
	      break;
	    case 'T':
              if (!t3) {
                forceCrash("Bad act T. '%s'", str.c_str());
                return;
              }
	      i = (const char *) t3;
	      break;
	    case 'F':
              if (!t3) {
                forceCrash("Bad act F. '%s'", str.c_str());
                return;
              }
	      i = fname((const char *) t3).c_str();
	      break;
	    case '$':
	      i = "$";
	      break;
	    default:
              i = "$";
	      break;
          }
          // color in the replacement sstring may reset existing color
          // to get around this, lets tack on any existing color
          if (color) {
            catstr = i;
            catstr += to->ansi_color(color);
            i = catstr.c_str();
          }
	  while ((*point = *(i++)) != 0)
	    ++point;
	  ++strp;

        } else if (!(*(point++) = *(strp++)))
	  break;
      }

      // we used to put the \n\r pad on here, but this causes the optional
      // color codes to be left dangling on the next line, causing some
      // problems for the client
      *(--point) = '\0';

      if (!((to->GetMaxLevel() > MAX_MORT) && 
          (IS_SET(to->desc->plr_color, PLR_COLOR_CODES)))) {
        sprintf(buf, "%s", colorString(to, to->desc, buf, NULL, COLOR_BASIC, FALSE).c_str());
      }

      if (!color) {
        to->desc->output.putInQ(cap(buf));
      } else {
        sstring str = to->ansi_color(color);
        if (str.empty())
          to->desc->output.putInQ(cap(buf));
        else {
          to->desc->output.putInQ(str);
          to->desc->output.putInQ(cap(buf));
          to->desc->output.putInQ(to->norm());
        } 
      }
      to->desc->output.putInQ("\n\r");
    }
    if ((type == TO_VICT) || (type == TO_CHAR))
      return;
  }
}

void Descriptor::updateScreenVt100(unsigned int update)
{
  char buf[4096];
  TBeing *f, *ch;

  if (!(ch = character))
    return;

  if (!ch->vt100()) {
    vlogf(LOG_MISC, "%s in updateScreenVt100 and not vt (%d)",ch->getName(),update);
    return;
  }

  if (!ch->desc || !IS_SET(ch->desc->prompt_d.type, PROMPT_VTANSI_BAR))
    return;

  writeToQ(VT_CURSAVE);
  sprintf(buf, VT_CURSPOS, ch->getScreen() - 3, 1);

  if (update & CHANGED_HP) {
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 2, 13);
    sprintf(buf + strlen(buf), "%-5d", ch->getHit());
  }
  if (ch->hasClass(CLASS_DEIKHAN) || ch->hasClass(CLASS_CLERIC)) {
    if (update & CHANGED_PIETY) {
      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 2, 40);
      sprintf(buf + strlen(buf), "%.1f", ch->getPiety());
    }
  } else if (ch->hasClass(CLASS_SHAMAN)) {
    if (update & CHANGED_LIFEFORCE) {
      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 2, 40);
      sprintf(buf + strlen(buf), "%-4d", ch->getLifeforce());
    }
  } else {
    if (update & CHANGED_MANA) {
      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 2, 40);
      sprintf(buf + strlen(buf), "%-4d", ch->getMana());
    }
  }
  if (update & CHANGED_MOVE) {
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 2, 67);
    sprintf(buf + strlen(buf), "%-4d", ch->getMove());
  }
  if (ch->isImmortal()) {
    if (update & CHANGED_ROOM) {
      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 1, 13);
      sprintf(buf + strlen(buf), "%-6d", ch->roomp->number);
    }
  } else {
#if FACTIONS_IN_USE
    if (update & CHANGED_PERC) {
      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 1, 13);
      sprintf(buf + strlen(buf), "%3.4f", ch->getPerc());
    }
#endif
  }
  if (update & CHANGED_GOLD) {
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 1, 40);
    sprintf(buf + strlen(buf), "%-8d", ch->getMoney());
  }
  if (update & CHANGED_EXP) {
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 1, 64);
    sprintf(buf + strlen(buf), "%s", ch->displayExp().c_str());
  }
  if ((f = ch->fight()) != NULL) {
    if (f->sameRoom(*ch)) {
      int maxh = max(1, (int) f->hitLimit());
      int ratio = min(10, max(0, ((f->getHit() * 9) / maxh)));
      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen(), 3);
      sprintf(buf + strlen(buf), "%s<%s=%s>%s", ch->purple(), fname(f->name).c_str(),
                                            prompt_mesg[ratio], ch->norm());
      last.fighting = TRUE;
    }
  } else {
    if (last.fighting) {
      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen(), 3);
      sprintf(buf + strlen(buf), "                          ");
      last.fighting = FALSE;
    }
  }
  if (IS_SET(update, CHANGED_MUD)) {
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen(), 35);
    sprintf(buf + strlen(buf), "%s",
         hmtAsString(hourminTime()).c_str());
  }
  if (IS_SET(update, CHANGED_TIME)) {
    time_t t1;
    struct tm *tptr;
    if ((t1 = time(0)) != -1) {
      tptr = localtime(&t1);

      // adjust time for users local site 
      tptr->tm_hour += account->time_adjust;
      if (tptr->tm_hour < 0) {
	tptr->tm_hour += 24;
      } else if (tptr->tm_hour > 23) 
        tptr->tm_hour -= 24;

      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen(), 62);
      sprintf(buf + strlen(buf), "%2d:%02d %2s",
        (!(tptr->tm_hour%12) ? 12 : tptr->tm_hour%12), 
        tptr->tm_min,
        (tptr->tm_hour >= 12) ? "PM" : "AM");
    }
  }
  writeToQ(buf);
  writeToQ(VT_NORMALT);
  writeToQ(VT_CURREST);
  
  // Make sure it doesn't send a new '>' when automatically updated - Russ
  if ((update == CHANGED_TIME) || (update == CHANGED_MUD))
    prompt_mode = PROMPT_DONT_SEND;
}

void Descriptor::updateScreenAnsi(unsigned int update)
{
  char buf[MAX_STRING_LENGTH];
  int missing_hit, missing_mana, missing_moves;
  int current_hit, current_mana, current_moves;
  int i;
  TBeing *f, *ch;

  if (!(ch = character))
    return;

  if (!ch->ansi()) {
    vlogf(LOG_MISC, "%s in updateScreenAnsi and not ansi (%d)",ch->getName(), update);
    return;
  }

  if (!ch->desc || !IS_SET(ch->desc->prompt_d.type, PROMPT_VTANSI_BAR))
    return;

  current_hit = (int) (10 * ((double) ch->getHit() / (double) ch->hitLimit()));
  if (ch->hasClass(CLASS_DEIKHAN) || ch->hasClass(CLASS_CLERIC)) 
    current_mana = (int) (10 * ch->getPiety() / 100.0);
  else if (ch->hasClass(CLASS_SHAMAN)) 
    current_mana = (int) (10 * ((double) ch->getLifeforce()));
  else
    current_mana = (int) (10 * ((double) ch->getMana() / (double) ch->manaLimit()));
  current_moves = (int) (10 * ((double) ch->getMove() / (double) ch->moveLimit()));
  current_hit = max(0, min(10, current_hit));
  if (ch->hasClass(CLASS_DEIKHAN) || ch->hasClass(CLASS_CLERIC)) 
    current_mana = (int) max(0., min(10., (double) current_mana));
  else if (ch->hasClass(CLASS_SHAMAN)) 
    current_mana = max(0, min(10, current_mana));
  else
    current_mana = max(0, min(10, current_mana));
  current_moves = max(0, min(10, current_moves));
  missing_hit = 10 - current_hit;
  missing_mana = 10 - current_mana;
  missing_moves = 10 - current_moves;

  writeToQ(VT_CURSAVE);
  sprintf(buf, VT_CURSPOS, ch->getScreen() - 2, 1);

  if (IS_SET(update, CHANGED_HP)) {
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 2, 6);
    sprintf(buf + strlen(buf), "%s%-5d ", current_hit > 2 ? VT_BOLDTEX : ANSI_RED, ch->getHit());
    sprintf(buf + strlen(buf), ANSI_BLUE);
    for (i = 1; i <= current_hit; i++)
      sprintf(buf + strlen(buf), ANSI_BAR3);
    sprintf(buf + strlen(buf), ANSI_CYAN);
    for (i = 1; i <= missing_hit; i++)
      sprintf(buf + strlen(buf), ANSI_BAR3);
  }
  if (IS_SET(update, CHANGED_MANA) || IS_SET(update, CHANGED_PIETY) || IS_SET(update, CHANGED_LIFEFORCE)) {
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 2, 35);
    if (ch->hasClass(CLASS_DEIKHAN) || ch->hasClass(CLASS_CLERIC)) 
      sprintf(buf + strlen(buf), "%s%-5.1f ", current_mana ? VT_BOLDTEX : ANSI_RED, ch->getPiety());
    else if (ch->hasClass(CLASS_SHAMAN)) 
      sprintf(buf + strlen(buf), "%s%-5d ", current_mana ? VT_BOLDTEX : ANSI_RED, ch->getLifeforce());
    else
      sprintf(buf + strlen(buf), "%s%-5d ", current_mana ? VT_BOLDTEX : ANSI_RED, ch->getMana());
    sprintf(buf + strlen(buf), ANSI_BLUE);
    for (i = 1; i <= current_mana; i++)
      sprintf(buf + strlen(buf), ANSI_BAR3);
    sprintf(buf + strlen(buf), ANSI_CYAN);
    for (i = 1; i <= missing_mana; i++)
      sprintf(buf + strlen(buf), ANSI_BAR3);
  }
  if (IS_SET(update, CHANGED_MOVE)) {
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 2, 62);
    sprintf(buf + strlen(buf), "%s%-5d ", current_moves ? VT_BOLDTEX : ANSI_RED, ch->getMove());
    sprintf(buf + strlen(buf), ANSI_BLUE);
    for (i = 1; i <= current_moves; i++)
      sprintf(buf + strlen(buf), ANSI_BAR3);
    sprintf(buf + strlen(buf), ANSI_CYAN);
    for (i = 1; i <= missing_moves; i++)
      sprintf(buf + strlen(buf), ANSI_BAR3);
  }
  if (ch->isImmortal()) {
    if (IS_SET(update, CHANGED_ROOM)) {
      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 1, 6);
      sprintf(buf + strlen(buf), "%s%-6d", ANSI_GREEN, ch->roomp->number);
    }
  } else {
#if FACTIONS_IN_USE
    if (IS_SET(update, CHANGED_PERC)) {
      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 1, 6);
      sprintf(buf + strlen(buf), "%s%3.4f", ANSI_GREEN, ch->getPerc());
    }
#endif
  }
  if (IS_SET(update, CHANGED_GOLD)) {
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 1, 34);
    sprintf(buf + strlen(buf), "%s%-8d", ANSI_GREEN, ch->getMoney());
  }
  if (IS_SET(update, CHANGED_EXP)) {
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen() - 1, 59);
    sprintf(buf + strlen(buf), "%s%s", ANSI_GREEN, ch->displayExp().c_str());
  }
  if ((f = ch->fight()) != NULL) {
    if (f->sameRoom(*ch)) {
      int ratio = min(10, max(0, ((f->getHit() * 9) / f->hitLimit())));
      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen(), 3);
      sprintf(buf + strlen(buf), "%s<%s=%s>%s", ch->purple(), fname(f->name).c_str(),
                                            prompt_mesg[ratio], ch->norm());
      for (i = strlen(f->name) + strlen(prompt_mesg[ratio]); i < 20; i++)
        strcat(buf, " ");
      last.fighting = TRUE;
    }
  } else {
    if (last.fighting) {
      sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen(), 3);
      sprintf(buf + strlen(buf), "                          ");
      last.fighting = FALSE;
    }
  }
  if (IS_SET(update, CHANGED_MUD)) {
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen(), 30);
    sprintf(buf + strlen(buf), "%s",
         hmtAsString(hourminTime()).c_str());
  }
  time_t t1;
  struct tm *tptr;
  if ((t1 = time((time_t *) 0)) != -1) {
    tptr = localtime(&t1);

    // adjust time for users local site 
    if (account)
      tptr->tm_hour += account->time_adjust;
    if (tptr->tm_hour < 0) 
      tptr->tm_hour += 24;
    else if (tptr->tm_hour > 23) 
      tptr->tm_hour -= 24;
    
    sprintf(buf + strlen(buf), VT_CURSPOS, ch->getScreen(), 57);
    sprintf(buf + strlen(buf), "%2d:%02d %2s",
        (!(tptr->tm_hour%12) ? 12 : tptr->tm_hour%12), 
        tptr->tm_min,
        (tptr->tm_hour >= 12) ? "PM" : "AM");
  }
  writeToQ(buf);
  writeToQ(VT_NORMALT);
  writeToQ(VT_CURREST);

  // Make sure it doesn't send a new '>' when automatically updated - Russ
  if ((update == CHANGED_TIME) || (update == CHANGED_MUD))
    prompt_mode = PROMPT_DONT_SEND;
}
