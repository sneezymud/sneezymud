//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//     "actions.cc" - All social functions and routines
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_pool.h"

class socialMessg {
  public:
    bool hide;
    positionTypeT minPos;
  
    // No argument was supplied 
    const char *char_no_arg;
    const char *others_no_arg;
  
    // An argument was there, and a victim was found 
    const char *char_found;            
    const char *others_found;
    const char *vict_found;
  
    // An argument was there, but no victim was found 
    const char *not_found;
  
    // The victim turned out to be the character
    const char *char_auto;
    const char *others_auto;

    socialMessg() :
      hide(false),
      minPos(POSITION_DEAD),
      char_no_arg(NULL),
      others_no_arg(NULL),
      char_found(NULL),
      others_found(NULL),
      vict_found(NULL),
      not_found(NULL),
      char_auto(NULL),
      others_auto(NULL)
    {
    }
    socialMessg(const socialMessg &a) :
      hide(a.hide),
      minPos(a.minPos)
    {
      char_no_arg = mud_str_dup(a.char_no_arg);
      others_no_arg = mud_str_dup(a.others_no_arg);
      char_found = mud_str_dup(a.char_found);
      others_found = mud_str_dup(a.others_found);
      vict_found = mud_str_dup(a.vict_found);
      not_found = mud_str_dup(a.not_found);
      char_auto = mud_str_dup(a.char_auto);
      others_auto = mud_str_dup(a.others_auto);
    }
    ~socialMessg() {
      delete [] char_no_arg;
      delete [] others_no_arg;
      delete [] char_found;
      delete [] others_found;
      delete [] vict_found;
      delete [] not_found;
      delete [] char_auto;
      delete [] others_auto;
      char_no_arg = NULL;
      others_no_arg = NULL;
      char_found = NULL;
      others_found = NULL;
      vict_found = NULL;
      not_found = NULL;
      char_auto = NULL;
      others_auto = NULL;
    }
    socialMessg & operator = (const socialMessg &a) {
      if (this == &a) return *this;
      
      hide = a.hide;
      minPos = a.minPos;

      delete [] char_no_arg;
      delete [] others_no_arg;
      delete [] char_found;
      delete [] others_found;
      delete [] vict_found;
      delete [] not_found;
      delete [] char_auto;
      delete [] others_auto;

      char_no_arg = mud_str_dup(a.char_no_arg);
      others_no_arg = mud_str_dup(a.others_no_arg);
      char_found = mud_str_dup(a.char_found);
      others_found = mud_str_dup(a.others_found);
      vict_found = mud_str_dup(a.vict_found);
      not_found = mud_str_dup(a.not_found);
      char_auto = mud_str_dup(a.char_auto);
      others_auto = mud_str_dup(a.others_auto);

      return *this;
    }
};

char *fread_action(FILE *fl) return rslt(NULL)
{
  char buf[MAX_STRING_LENGTH];

  for (;;) {
    fgets(buf, MAX_STRING_LENGTH, fl);

    if (feof(fl)) {
      vlogf(LOG_FILE, "Fread_action - unexpected EOF.");
      exit(0);
    }

    if (*buf == '#')
      return NULL;
    else {
      *(buf + strlen(buf) - 1) = '\0';
      rslt = mud_str_dup(buf);
      return;
    }
  }
}

static map<int, socialMessg>soc_mess_list;

void bootSocialMessages(void)
{
  FILE *fl;
  int hide, min_pos;
  char buf[256];

  if (!(fl = fopen(SOCMESS_FILE, "r"))) {
    perror("bootSocialMessages");
    exit(0);
  }

  for (;;) {
    fscanf(fl, " %s ", buf);
    cmdTypeT tmp = searchForCommandNum(buf);

    if (tmp >= MAX_CMD_LIST)
      break;

    fscanf(fl, " %d ", &hide);
    fscanf(fl, " %d \n", &min_pos);

    socialMessg sm;
    sm.hide = hide;
    
    sm.minPos = mapFileToPos(min_pos);

    sm.char_no_arg = fread_action(fl);
    sm.others_no_arg = fread_action(fl);
    sm.char_found = fread_action(fl);

    if (!sm.char_found) {
      sm.others_found = NULL;
      sm.vict_found = NULL;
      sm.not_found = NULL;
      sm.char_auto = NULL;
      sm.others_auto = NULL;

      soc_mess_list[tmp] = sm;
      continue;
    }

    sm.others_found = fread_action(fl);
    sm.vict_found = fread_action(fl);
    sm.not_found = fread_action(fl);
    sm.char_auto = fread_action(fl);
    sm.others_auto = fread_action(fl);

    soc_mess_list[tmp] = sm;
  }
  fclose(fl);
}

// returns DELETE_THIS
int TBeing::doAction(const string & argument, cmdTypeT cmd) return rc(0)
{
  char buf[MAX_INPUT_LENGTH];
  TBeing *vict;
  TMonster *tmp = NULL;
  TThing *t, *t2, *tvict=NULL;

  if (fight() || riding) {
    switch(cmd) {
// disallow  while fighting, allow for riding
// lower torso actions (dance, wiggle) prevented
      case CMD_BOP:
      case CMD_APPLAUD:
      case CMD_BEG:
      case CMD_TIP:
      case CMD_MOAN:
      case CMD_RAISE:
      case CMD_PET:
      case CMD_COMFORT:
      case CMD_YODEL:
      case CMD_GREET:
      case CMD_BOW:
      case CMD_CLAP:
      case CMD_SNAP:
      case CMD_PAT:
      case CMD_HUG:
      case CMD_POINT:
      case CMD_WAVE:
      case CMD_STRETCH:
      case CMD_FLIPOFF:
      case CMD_TWIDDLE:
      case CMD_LOVE:
      case CMD_SULK:
      case CMD_COMB:
      case CMD_FLEX:
      case CMD_BECKON:
      case CMD_SALUTE:
        if (fight())  {
          sendTo("You cannot perform that action while fighting!\n\r");
          return FALSE;
        }
        break;
// allowed while riding or fighting
// this should be list of facial expressions only, no hands allowed
      case CMD_AVERT:
      case CMD_WINK:
      case CMD_BOGGLE:
      case CMD_RAZZ:
      case CMD_WINCE:
      case CMD_BRANDISH:
      case CMD_GRUNT:
      case CMD_SCOLD:
      case CMD_BELITTLE:
      case CMD_CHORTLE:
      case CMD_APOLOGIZE:
      case CMD_WHIMPER:
      case CMD_SNEER:
      case CMD_MOO:
      case CMD_WHINE:
      case CMD_ACCUSE:
      case CMD_SNORE:
      case CMD_STEAM:
      case CMD_PEER:
      case CMD_PURR:
      case CMD_BLEED:
      case CMD_DAYDREAM:
      case CMD_HICCUP:
      case CMD_NOD:
      case CMD_FUME:
      case CMD_PUKE:
      case CMD_WHISTLE:
      case CMD_CRINGE:
      case CMD_THINK:
      case CMD_THANK:
      case CMD_ARCH:
      case CMD_SHRUG:
      case CMD_SIGH:
      case CMD_TAUNT:
      case CMD_GIGGLE:
      case CMD_CHEER:
      case CMD_GRUMBLE:
      case CMD_SNICKER:
      case CMD_CACKLE:
      case CMD_GASP:
      case CMD_GRIN:
      case CMD_SHAKE:  // shake head no, too bad this is also shake hand
      case CMD_GRIMACE:
      case CMD_SING:
      case CMD_SMILE:
      case CMD_HOWL:
      case CMD_STARE:
      case CMD_SNARL:
      case CMD_WOO:
      case CMD_CHUCKLE:
      case CMD_GROWL:
      case CMD_LAUGH:
      case CMD_CURSE:
      case CMD_CRY:
      case CMD_POUT:
      case CMD_SPIT:
      case CMD_SMIRK:
      case CMD_FROWN:
      case CMD_BEAM:
      case CMD_PONDER:
      case CMD_GLARE:
      case CMD_SCREAM:
      case CMD_AGREE:
      case CMD_YAWN:
      case CMD_DISAGREE:
      case CMD_SNORT:
      case CMD_ROAR:
      case CMD_ROLL:
      case CMD_GROAN:
      case CMD_BLUSH:
      case CMD_SHIVER:
      case CMD_DROOL:
      case CMD_COUGH:
      case CMD_SNIFF:
      case CMD_BURP:
      case CMD_FART:
      case CMD_SNEEZE:
      case CMD_HUM:
      case CMD_BLINK:
        break;
      default:
        // prevented due to riding/fighting
        sendTo("You currently cannot perform that action!\n\r");
        return FALSE;
    }
  }

  map<int, socialMessg>::const_iterator CT;
  CT = soc_mess_list.find(cmd);

  if (CT == soc_mess_list.end()) {
    sendTo("That action is not supported.\n\r");
    return FALSE;
  }

  socialMessg action = CT->second;

  if (action.char_found)
    only_argument(argument.c_str(), buf);
  else
    *buf = '\0';

  if (roomp) {
    switch (cmd) {
      case CMD_YAWN:
        roomp->playsound(pickRandSound(SOUND_YAWN_1, SOUND_YAWN_4), SOUND_TYPE_SOCIAL);
        break;
      case CMD_GIGGLE:
        roomp->playsound(SOUND_GIGGLE, SOUND_TYPE_SOCIAL);
        break;
      case CMD_BURP:
        roomp->playsound(SOUND_BURP, SOUND_TYPE_SOCIAL);
        break;
      case CMD_CLAP:
        roomp->playsound(SOUND_CLAP, SOUND_TYPE_SOCIAL);
        break;
      case CMD_FART:
        roomp->playsound(SOUND_FART, SOUND_TYPE_SOCIAL);
        break;
      case CMD_SNEEZE:
        roomp->playsound(SOUND_SNEEZE, SOUND_TYPE_SOCIAL);
        break;
      case CMD_CACKLE:
        roomp->playsound(SOUND_CACKLE, SOUND_TYPE_SOCIAL);
        break;
      case CMD_SCREAM:
        roomp->playsound(SOUND_SCREAM, SOUND_TYPE_SOCIAL);
        break;
      case CMD_CHORTLE:
        roomp->playsound(SOUND_DM_LAUGH, SOUND_TYPE_SOCIAL);
        break;
      case CMD_DISAGREE:
        roomp->playsound(SOUND_DISAGREE, SOUND_TYPE_SOCIAL);
        break;
      case CMD_WOO:
        roomp->playsound(SOUND_YAHOO, SOUND_TYPE_SOCIAL);
        break;
      default:
        break;  
    }
  }

  if (!roomp)
    return FALSE;

  if (!*buf) {
    sendTo(action.char_no_arg);
    sendTo("\n\r");
    act(action.others_no_arg, action.hide, this, 0, 0, TO_ROOM);

    for (t = roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      tmp = dynamic_cast<TMonster *>(t);
      if (!tmp)
        continue;

      rc = tmp->aiSocialSwitch(this, NULL, cmd, TARGET_NONE);

      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tmp;
        tmp = NULL;
      }

      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
    }

    return FALSE;
  }

  if (!(vict = get_char_room_vis(this, buf, NULL, EXACT_YES))) {
    if (!(vict = get_char_room_vis(this, buf))) {
      if (!(tvict = get_obj_vis_accessible(this, buf))) {
	sendTo(action.not_found);
	sendTo("\n\r");
	return FALSE;
      }
    }
  }
	  

  if (vict && vict == this) {
    sendTo(action.char_auto);
    sendTo("\n\r");
    act(action.others_auto, action.hide, this, 0, 0, TO_ROOM);

    for (t = roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      tmp = dynamic_cast<TMonster *>(t);

      if (!tmp)
        continue;

      rc = tmp->aiSocialSwitch(this, NULL, cmd, TARGET_SELF);

      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tmp;
        tmp = NULL;
      }

      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
    }
  } else if(vict){
    if (vict->getPosition() < action.minPos)
      act("$N is not in a proper position for that.", FALSE, this, 0, vict, TO_CHAR);
    else {
      if (socialLimbBad(vict,cmd))
        return FALSE;

      act(action.char_found, 0, this, 0, vict, TO_CHAR);
      act(action.others_found, action.hide, this, 0, vict, TO_NOTVICT);
      act(action.vict_found, action.hide, this, 0, vict, TO_VICT);

      for (t = roomp->getStuff(); t; t = t2) {
        t2 = t->nextThing;
        tmp = dynamic_cast<TMonster *>(t);

        if (!tmp)
          continue;

        if (tmp == vict)
          rc = tmp->aiSocialSwitch(this, NULL, cmd, TARGET_MOB);
        else
          rc = tmp->aiSocialSwitch(this, vict, cmd, TARGET_OTHER);

        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tmp;
          tmp = NULL;
        }

        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_THIS;
      }
    }
  } else if(tvict){
    act(action.char_found, 0, this, 0, tvict, TO_CHAR);
    act(action.others_found, action.hide, this, 0, tvict, TO_NOTVICT);
  }    

  return FALSE;
}

void TBeing::doInsult(const char *argument)
{
  char buf[100];
  char arg[MAX_INPUT_LENGTH];
  TBeing *victim;

  only_argument(argument, arg);

  if (*arg) {
    if (!(victim = get_char_room_vis(this, arg))) 
      sendTo("Can't hear you!\n\r");
    else {
      if (victim != this) {
        sprintf(buf, "You insult %s.\n\r", victim->getName());
        sendTo(buf);

        switch (::number(0, 2)) {
          case 0:
            if (getSex() == SEX_MALE) {
              if (victim->getSex() == SEX_MALE)
                act("$n accuses you of fighting like a woman!", FALSE, this, 0, victim, TO_VICT);
              else
                act("$n says that women can't fight.", FALSE, this, 0, victim, TO_VICT);
            } else {        
              if (victim->getSex() == SEX_MALE)
                act("$n accuses you of having the smallest.... (brain?)",
                   FALSE, this, 0, victim, TO_VICT);
              else
                act("$n tells you that you'd lose a beauty contest against a troll.",
                   FALSE, this, 0, victim, TO_VICT);
            }
            break;
          case 1:
            act("$n calls your mother a bitch!", FALSE, this, 0, victim, TO_VICT);
            break;
          default:
            act("$n tells you to get lost!", FALSE, this, 0, victim, TO_VICT);
            break;
        }                
        act("$n insults $N.", TRUE, this, 0, victim, TO_NOTVICT);
      } else
        sendTo("You feel insulted.\n\r");
    }
  } else
    sendTo("Sure you don't want to insult everybody.\n\r");
}

void TBeing::doScratch(const char *argument)
{
  char arg[256];

  if (in_room < 0)
    return;

  only_argument(argument, arg);

  if (!strcasecmp(arg, "leg")) {
    act("$n vigorously scratches $s leg!", TRUE, this, 0, 0, TO_ROOM);
    act("You vigorously scratch your leg!", FALSE, this, 0, 0, TO_CHAR); 
  } else if (!strcasecmp(arg, "ass") || !strcasecmp(arg, "butt")) {
    act("$n vigorously scratches $s butt!", TRUE, this, 0, 0, TO_ROOM);
    act("You vigorously scratch your butt!", FALSE, this, 0, 0, TO_CHAR);
  } else if (!strcasecmp(arg, "crotch")) {
    act("$n vigorously scratches $s genital region!", TRUE, this, 0, 0, TO_ROOM);
    act("You vigorously scratch your genitals!", FALSE, this, 0, 0, TO_CHAR);
  } else if (!strcasecmp(arg, "head")) {
    act("$n vigorously scratches $s head!", TRUE, this, 0, 0, TO_ROOM);
    act("You vigorously scratch your head!", FALSE, this, 0, 0, TO_CHAR);
  } else {
    act("$n vigorously scratches $mself!", TRUE, this, 0, 0, TO_ROOM);
    act("You vigorously scratch yourself!", FALSE, this, 0, 0, TO_CHAR);
  }
}

void TObj::peeMe(const TBeing *)
{
}

void TPool::peeMe(const TBeing *ch)
{
  act("$n smiles happily as $e pisses into $p.", TRUE, ch, this, NULL, TO_ROOM);
  act("You smile happily as you piss into $p.",  TRUE, ch, this, NULL, TO_CHAR); 

  if (getDrinkType() == LIQ_WATER) {
    act("$e turns water to wine!", TRUE, ch, this, NULL, TO_ROOM);
    act("You turn water to wine!", TRUE, ch, this, NULL, TO_CHAR);
    setDrinkType(LIQ_RED_WINE);
  } else 
    fillMe(ch, LIQ_LEMONADE);
}

void TBeing::doPee(const char *argument)
{
  TThing *t;
  TObj *o;
  TBeing *tmp_char;
  char arg[200], arg2[200];
  liqTypeT liquid=MAX_DRINK_TYPES, i;

  if (powerCheck(POWER_PEE))
    return;

  if (in_room < 0)
    return;

  only_argument(argument, arg);

  for (i = MIN_DRINK_TYPES; i < MAX_DRINK_TYPES; i++) {
    if (is_abbrev(arg, stripColorCodes(DrinkInfo[i]->name).c_str())) {
      liquid = i;
      break;
    }
  }

  if (*arg && liquid == MAX_DRINK_TYPES) {
    if (!strncmp(arg, "in", 2) && isspace(arg[2])) {
      only_argument(argument + 3, arg2);

      if (arg2 && generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, this, &tmp_char, &o)) 
       	o->peeMe(this);	
    } else if ((t = searchLinkedListVis(this, arg, roomp->getStuff()))) 
      t->peeOnMe(this);
  } else {
    act("$n quietly relieves $mself.  You are not amused.", TRUE, this, NULL, NULL, TO_ROOM);
    sendTo("You relieve yourself as stealthfully as possible.\n\r");
    dropPool(::number(1,10), (liquid < 0 || liquid>=MAX_DRINK_TYPES) ? LIQ_LEMONADE : liquid);
  }
}

void TBeing::doPoint(const char *arg)
{
  TThing *t = NULL;
  TThing *hold = NULL;
  TObj *obj;
  TBeing *b;
  char buf[128] = "\0";
  char holdBuf[128] = "\0";

  if (!roomp)
    return;

  for (;isspace(*arg);arg++);

  if ((hold = equipment[getPrimaryHold()])) 
    strcpy(buf, fname(hold->name).c_str());
  else
    strcpy(buf, "finger");

  if (!*arg) {
    sprintf(holdBuf, "You point your %s around randomly.", buf);
    act(holdBuf, FALSE, this, NULL, this, TO_CHAR);
    sprintf(holdBuf, "$n points $s %s around randomly.", buf);
    act(holdBuf, FALSE, this, NULL, this, TO_ROOM);
    return;
  }

  // point in a direction
  dirTypeT dir = getDirFromChar(arg);
  if (dir != DIR_NONE) {
    sendTo("You point %s%s%s%s.\n\r", 
        buf ? "your " : "",
        buf ? buf : "",
        buf ? " " : "",
        dirs_to_blank[dir]);
    sprintf(holdBuf, "$n points %s%s%s%s.", 
        buf ? "$s " : "",
        buf ? buf : "",
        buf ? " " : "",
        dirs_to_blank[dir]);
    act(holdBuf, false, this, NULL, NULL, TO_ROOM);
    return;
  }

  // point at someone
  for (t = roomp->getStuff(); t; t = t->nextThing) {
    if (isname(arg, t->name)) {
      obj = dynamic_cast<TObj *>(t);
      if (obj) {
        sendTo(COLOR_OBJECTS,"You point your %s at %s.\n\r", buf, obj->getName());
        sprintf(holdBuf, "$n points $s %s at $o.", buf);
        act(holdBuf, FALSE, this, obj, NULL, TO_ROOM);
        return;
      } 
      b = dynamic_cast<TBeing *>(t);
      if (b) {
        if (b == this) {
          sendTo("You point at yourself.\n\r");
          act("$n points at $mself.", FALSE, this, NULL, NULL, TO_ROOM);
        } else {
	  sendTo(COLOR_OBJECTS, "You point at %s with your %s.\n\r", b->getName(), buf);
	  sprintf(holdBuf, "$n points at $N with $s %s.", buf);
	  act(holdBuf, FALSE, this, NULL, b, TO_NOTVICT);
	  sprintf(holdBuf, "$n points at you with $s %s.", buf);
	  act(holdBuf, FALSE, this, NULL, b, TO_VICT);
	}
        return;
      }
    }
  }
  // If we got here, the person pointed at something that wasnt in the room
  sendTo("Do you usually point at things that aren't there?\n\r");
}
