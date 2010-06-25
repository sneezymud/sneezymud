//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "spec_mobs.cc" - Special procedures for mobiles.
//
///////////////////////////////////////////////////////////////////////////

#if 0
    CMD_GENERIC_PULSE, CMD_MOB_ALIGN_PULSE, CMD_MOB_COMBAT:
      two formal TBeings are identical
      if it dies, do not delete and just return DELETE_THIS
      if you kill mob->fight() in CMD_MOB_COMBAT, remember to return TRUE

    CMD_MOB_MOVED_INTO_ROOM:
      returns TRUE and mob is kicked out of room
      ch = mover, myself = mob with proc, obj = old room cast to TObj *

    CMD_MOB_VIOLENCE_PEACEFUL:
      first TBeing = violent mob
      second TBeing = mob checking
      TObj : target of violence casted to (TOBj *)
      return DELETE_VICT to kill first TBeing

    CMD_GENERIC_INIT:
      called during load process while it is still parsing the file

    CMD_GENERIC_CREATED:
      called at the end of the read_mobile process

    Generic Commands:
      return DELETE_VICT if first TBeing gone
      return DELETE_THIS if second TBeing gone
#endif

#include <errno.h>


#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "low.h"
#include "monster.h"
#include "configuration.h"
#include "account.h"
#include "combat.h"
#include "disease.h"
#include "person.h"
#include "disc_aegis.h"
#include "materials.h"
#include "disc_wrath.h"
#include "disc_alchemy.h"
#include "paths.h"
#include "database.h"
#include "obj_symbol.h"
#include "obj_window.h"
#include "obj_general_weapon.h"
#include "obj_base_clothing.h"
#include "obj_bow.h"
#include "obj_trap.h"
#include "obj_table.h"
#include "obj_drinkcon.h"
#include "corporation.h"
#include "shopowned.h"
#include "pathfinder.h"
#include "shop.h"
#include "obj_base_corpse.h"
#include "obj_player_corpse.h"
#include "obj_tool.h"
#include "obj_plant.h"
#include "obj_note.h"
#include "obj_commodity.h"
#include "obj_component.h"
#include "obj_food.h"
#include "spec_mobs.h"
#include "weather.h"

const int GET_MOB_SPE_INDEX(int d)
{
  return (((d > NUM_MOB_SPECIALS) || (d < 0)) ? 0 : d);
}



// returns DELETE_THIS for this
// returns DELETE_VICT if the special kills t
// returns DELETE_ITEM for t2
int TMonster::checkSpec(TBeing *t, cmdTypeT cmd, const char *arg, TThing *t2)
{
  int rc;

  //  if (cmd == CMD_GENERIC_PULSE && spec == SPEC_BOUNTY_HUNTER)
  //    vlogf(LOG_DASH, format("Bounty Hunter spec %d on %s called with CMD_GENERIC_PULSE (checkSpec)") %  spec % getName());

  if(inRoom() == Room::NOCTURNAL_STORAGE)
    return FALSE;
  // if we move them to hell, it's probably cause there is a problem with
  // the proc, so skip it.  Realize, this may let them leak memory since
  // the destroy message is not called...
  if (inRoom() == Room::HELL && spec != SPEC_TORMENTOR)
    return FALSE;

  // we will use a static cast on t2 as we don't always pass a true
  // TThing as the extra pointer.  e.g.
  // CMD_MOB_KILLED_NEARBY: vict passed as t2
  if (spec) {
    rc = (mob_specials[GET_MOB_SPE_INDEX(spec)].proc)
               (t, cmd, arg, this, static_cast<TObj *>(t2));
    return rc;
  }
  return FALSE;
}

walkPathT TMonster::walk_path(const path_struct *p, int &pos)
{
  if (p[(pos + 1)].direction == -1){
    return WALK_PATH_END;
  } else if (p[pos].cur_room != in_room){
    // we're off the path
    dirTypeT dir;

    // check surrounding rooms
    for (dir=MIN_DIR; dir < MAX_DIR;dir++) {
      if (canGo(dir) && 
	  roomp->dir_option[dir]->to_room ==
	  p[pos].cur_room){
	goDirection(dir);
	return WALK_PATH_MOVED;
      }
    }
    
    // check the entire path
    pos = -1;
    do {
      pos += 1;
      if (p[pos].cur_room == in_room)
	return WALK_PATH_MOVED;
    } while (p[pos].cur_room != -1);
    return WALK_PATH_LOST;
  } else if (getPosition() < POSITION_STANDING) {
    doStand();
  } else {
    if(goDirection(p[pos + 1].direction)){
      pos++;
    }
  }

  return WALK_PATH_MOVED;
}



bool TMonster::isPolice() const
{
  int num = mobVnum();

  return (!isPc() && ((spec == SPEC_CITYGUARD) || 
                      (num == Mob::BOUNCER) || (num == Mob::Mob::BOUNCER2) ||
                      (num == Mob::Mob::BOUNCER_HEAD)));
}
  
int TMonster::npcSteal(TPerson *victim)
{
  sstring buf;
  TThing *t=NULL;

  int tmp = getLevel(THIEF_LEVEL_IND);

  if ((spec == SPEC_TRAINER_THIEF) || (spec == SPEC_GM_THIEF))
    return FALSE;

  if (victim->inGrimhaven() && victim->GetMaxLevel() <= 5)
    return FALSE;

  if (victim->isImmortal() || fight() ||
      victim->fight() || !victim->getMoney() ||
      !canSee(victim))
    return FALSE;

  // check if a guard is around
  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (tbt && tbt->isPolice())
      return FALSE;
  }

  // see if victim is cagey
  // this use to be a check for urban hometerrain, but urban is +per
  double factor = victim->plotStat(STAT_CURRENT, STAT_PER, -3.0, 3.0, 0.0);
  if (victim->isLucky(levelLuckModifier(victim->GetMaxLevel() + factor)))
    return FALSE;

  if (doesKnowSkill(SKILL_STEAL))
    if (getSkillValue(SKILL_STEAL) < tmp)
      setSkillValue(SKILL_STEAL, tmp);

  return doSteal(format("talens %s") % fname(victim->name), victim);
}

// myself has the proc, ch just killed o
int factionFaery(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
{
  TBeing *mas;
  TBeing *vict;
  char buf[160];

  if (!myself->awake())
    return FALSE;

  mas = myself->master;

  if (cmd == CMD_GENERIC_PULSE) {
    if (!mas)
      return DELETE_THIS;
    return FALSE;
  }

  if (!mas)
    return FALSE;

  if (cmd == CMD_MOB_KILLED_NEARBY) {
    // cast down, and then back up since o is really a being
    TThing *ttt = o;
    vict = dynamic_cast<TBeing *>(ttt);
    if (!vict)
      return FALSE;

    // only talk if my master was one killing
    if (ch != mas)
      return FALSE;
    if (vict->isPc())
      return FALSE;

    if (ch->isSameFaction(vict)) {
      sprintf(buf, "%s You shouldn't kill %s; %s is the same faction as you.",
            fname(ch->name).c_str(), vict->getName(), vict->hssh());
      myself->doWhisper(buf);
    } else {
#if FACTIONS_IN_USE
      if (ch->getPercX(vict->getFaction()) >= 100.0) {
        sprintf(buf, "%s You don't need to kill any more followers of %s's faction.",
              fname(ch->name).c_str(), vict->getName());
        myself->doWhisper(buf);
      }
#endif
    }
    return FALSE;

  } else if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    one_argument(arg, buf, cElements(buf));

    if (strcmp(buf, "help"))
      return FALSE;
  }
  
  return FALSE;
}

int rumorMonger(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *o)
{
  char caFilebuf[256], buf[256];
  int type, room, numrumors = 0, num;
  int amt;
  FILE *fp;

  if (cmd != CMD_GENERIC_PULSE && 
      cmd != CMD_BUY && 
      cmd != CMD_LIST && 
      cmd != CMD_MOB_GIVEN_COINS)
    return FALSE;

  // make it manageble
  if (cmd == CMD_GENERIC_PULSE && ::number(0,5))
    return FALSE;

  // skip med mobs
  if (myself->mobVnum() <= 0) 
    return FALSE;
  if (!myself->awake()) 
    return FALSE;

  sprintf(caFilebuf, "mobdata/rumors/%d", myself->mobVnum());
  if ((fp = fopen(caFilebuf, "r")) == NULL) {
    vlogf(LOG_LOW, format("Missing rumor file (%s) (%d)") %  caFilebuf % errno);
    return FALSE;
  }

  do {
    fgets(buf, 255, fp);
  } while (*buf == '#');

  if (sscanf(buf, "%d %d\n", &type, &room) != 2) {
    vlogf(LOG_LOW, format("Bad rumor format line 1 (%s) %s") %  caFilebuf);
    fclose(fp);
    return FALSE;
  }
    
  if (!type) {
    vlogf(LOG_LOW, format("Bad rumor type (%s) %s") %  caFilebuf % buf);
    fclose(fp);
    return FALSE;
  }

  if (type == -1) {
    if (cmd != CMD_GENERIC_PULSE) {
      fclose(fp);
      return FALSE;
    }
    if (room != -1 && room != myself->inRoom()) {
      fclose(fp);
      return FALSE;
    }
    // count all rumors
    numrumors = 0;
    while (fgets(buf, 255, fp)) {
      if (*buf != '#')
        numrumors++;
    }

    if (!numrumors) {
      vlogf(LOG_LOW, format("No rumors (%s)") %  caFilebuf);
      fclose(fp);
      return FALSE;
    }

    // select random rumor
    num = ::number(1, numrumors);

    // skip to that rumor
    rewind(fp);
    // skip over type
    do {
      fgets(buf, 255, fp);
    } while (*buf == '#');

    numrumors = 0;
    while (numrumors < num) {
      fgets(buf, 255, fp);
      if (*buf != '#')
        numrumors++;
    }
    // strip off newline
    if (strlen(buf) >= 1)
      buf[strlen(buf) - 1] = '\0';

    myself->doSay(buf);
    fclose(fp);
    return TRUE;
  } else if (type > 0) {
    if (room != -1 && room != myself->inRoom()) {
      fclose(fp);
      return FALSE;
    }
    if (cmd == CMD_BUY) {
      if (ch->getMoney() >= type) {
        ch->giveMoney(myself, type, GOLD_SHOP);
      } else {
        ch->sendTo("You don't have that much money.\n\r");
        fclose(fp);
        return TRUE;
      }
    } else if (cmd == CMD_MOB_GIVEN_COINS) {
      if ((amt = (int) o) < type) {
        myself->doSay("Those that are GENEROUS are often rewarded.");
        fclose(fp);
        return TRUE;
      }
    } else if (cmd == CMD_LIST) {
      // the next (2nd) line contains the sstring we should say
      do {
        if (!fgets(buf, 255, fp)) {
          vlogf(LOG_LOW, format("Missing sstring for list (%s)") %  caFilebuf);
          fclose(fp);
          return TRUE;
        }
      } while (*buf == '#');
      if (strlen(buf) >= 1)
        buf[strlen(buf) - 1] = '\0';

      myself->doTell(fname(ch->name), buf);
      fclose(fp);
      return TRUE;
    }

    // don't count the "list" line
    do {
      if (!fgets(buf, 255, fp)) {
        vlogf(LOG_LOW, format("Missing sstring for list (%s)") %  caFilebuf);
        fclose(fp);
        return TRUE;
      }
    } while (*buf == '#');

    // count all rumors
    numrumors = 0;
    while (fgets(buf, 255, fp)) {
      if (*buf != '#')
        numrumors++;
    }

    if (!numrumors) {
      vlogf(LOG_LOW, format("No rumors (%s)") %  caFilebuf);
      fclose(fp);
      return FALSE;
    }

    // select random rumor
    num = ::number(1, numrumors);

    // skip to that rumor
    rewind(fp);

    do {
      fgets(buf, 255, fp); // skip over type
    } while (*buf == '#');

    do {
      fgets(buf, 255, fp); // skip over list sstring
    } while (*buf == '#');

    numrumors = 0;
    while (numrumors < num) {
      fgets(buf, 255, fp);
      if (*buf != '#')
        numrumors++;
    }
    // strip off newline
    if (strlen(buf) >= 1)
      buf[strlen(buf) - 1] = '\0';

    myself->doTell(fname(ch->name), buf);
    fclose(fp);
    return TRUE;
  }
  fclose(fp);
  return FALSE;
}

int newbieEquipper(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  int request = 0, duration = 0, r_num = 0, found = 0;
  char buf[256];
  TSymbol *best = NULL;
//  TObj *best = NULL; // 
  TObj *obj = NULL;
  TThing *i = NULL;
  TThing *j = NULL;
  int books = 0;
  affectedData af;

  if (me->mobVnum() <= 0)
    return FALSE;

  if (cmd != CMD_SAY && cmd != CMD_SAY2 && cmd != CMD_WHISPER && cmd != CMD_ASK) 
    return FALSE;

  if (!ch->canSpeak())
    return FALSE;

  if (!ch->desc)
    return FALSE;

  if (ch->polyed)
    return FALSE;

  if (ch->isImmortal()) {
    me->doSay("I'm for newbie's use, not you dumb god's enjoyment!");
    return FALSE;
  }

  if (!me->awake())
    return FALSE;
  
  one_argument(arg, buf, cElements(buf));

  if (!strcmp(buf, "newbie")) 
    books = 1;
  else if (!strcmp(buf, "guide")) 
    books = 2;
  

  if (books) {
    if (cmd == CMD_SAY2)
      cmd = CMD_SAY;

    if (cmd == CMD_SAY)
      ch->doSay(arg);
    else if (cmd == CMD_WHISPER)
      ch->doWhisper(arg);
    else
      ch->doAsk(arg);

    if (books == 1) {
      if ((r_num = real_object(1458)) >= 0) {
        obj = read_object(r_num, REAL);
        *ch += *obj;    // newbie book 
       } else {
         vlogf(LOG_BUG, "Problem in NewbieEquipper, newbie");
         return TRUE;
       }
       act("$n smiles and hands $p to $N.", TRUE, me, obj, ch,TO_NOTVICT);
       act("$n smiles and hands $p to you.", TRUE, me, obj, ch,TO_VICT);
       return TRUE;
    } else if (books == 2) {
      if ((r_num = real_object(1459)) >= 0) {
        obj = read_object(r_num, REAL);
        *ch += *obj;    // conversion book 
      } else {
        vlogf(LOG_BUG, "Problem in NewbieEquipper, guide");
        return TRUE;
      }
      act("$n smiles and hands $p to $N.", TRUE, me, obj, ch,TO_NOTVICT);
      act("$n smiles and hands $p to you.", TRUE, me, obj, ch,TO_VICT);
      return TRUE;
    }
    vlogf(LOG_BUG, "Something wierd in newbieEquipper, books funcition");
    return TRUE;
  }

  if (!strcmp(buf, "equipment")) {
    request = 1;
  } else if (!strcmp(buf, "symbol")) {
    if (!ch->hasClass(CLASS_CLERIC) && !ch->hasClass(CLASS_DEIKHAN)) {
      if (cmd == CMD_SAY)
        ch->doSay(arg);
      else if (cmd == CMD_WHISPER)
        ch->doWhisper(arg);
      else
        ch->doAsk(arg);
      me->doTell(ch->getName(), "You are not a cleric or deikhan. I am not that charitable.");
      return TRUE;
    } else {
      request = 2;
    }
  } else if (!strcmp(buf, "weapon")) 
    request = 3;
  
#if 0
  } else if (!strcmp(buf, "spellbag")) {
    if (!ch->hasClass(CLASS-MAGE) && !ch->hasClass(CLASS_RANGER)) {
      if (cmd == CMD_SAY)
        ch->doSay(arg);
      else if (cmd == CMD_WHISPER)
        ch->doWhisper(arg); 
      else
        ch->doAsk(arg);
      sprintf(tmp_buf, "%s You are not a mage or ranger.  I am not that charitable.", ch->getName());
      me->doTell(tmp_buf);
      return TRUE;
    } else 
      request = 4;
#endif

  if (!request)
    return FALSE;
  
  if (ch->GetMaxLevel() >= 10) {
    me->doTell(ch->getName(), "I can no longer help you!");
    return TRUE;
  }

  if (cmd == CMD_SAY2)
    cmd = CMD_SAY;

  if (cmd == CMD_SAY)
    ch->doSay(arg);
  else if (cmd == CMD_WHISPER)
    ch->doWhisper(arg);
  else
    ch->doAsk(arg);

  switch (request) {
    case 1:
#if 0
      if (ch->getStuff()) {
        sprintf(tmp_buf, "%s You have some equipment already. I only equip the totally destitute.", ch->getName());
        me->doTell(tmp_buf);
        return TRUE;
      }
#endif
      if (ch->affectedBySpell(AFFECT_NEWBIE)) {
        found = 1;
      } else {
        act("$n rummages through a few bins and selects some items.", TRUE, me, NULL, NULL, TO_ROOM);
        act("$n smiles and hands $N some equipment.", TRUE, me, NULL, ch,TO_NOTVICT);
        act("$n smiles and hands you some equipment.", TRUE, me, NULL, ch,TO_VICT);
        ch->doNewbieEqLoad(RACE_NORACE, 0, true);
        me->doTell(ch->getName(), "May these items serve you well.  Be more careful next time!");
        found = 2;
      }
      break;
    case 2:
      if (!ch->hasClass(CLASS_CLERIC) &&  !ch->hasClass(CLASS_DEIKHAN)) {
        me->doTell(ch->getName(), "You are not a cleric or deikhan. I only give symbols to those who will use them.");
        return TRUE;
      }
      if (ch->affectedBySpell(AFFECT_NEWBIE)) {
        found = 1;
        break;
      } else {
        best = NULL;
        for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end() && (i=*it);++it) {
          i->findSym(&best);

          for(StuffIter it=i->stuff.begin();it!=i->stuff.end() && (j=*it);++it) {
            j->findSym(&best); 
          }
        }
        if (best) {
          me->doTell(ch->getName(), "You already have a symbol.  I only help the the totally destitute.");
          return TRUE;
        }
        act("$n rummages through a bin and selects an item.", TRUE, me, NULL, NULL, TO_ROOM);
        if ((r_num = real_object(500)) >= 0) {
          obj = read_object(r_num, REAL);
          act("$n smiles and hands $p to $N.", TRUE, me, obj, ch,TO_NOTVICT);
          act("$n smiles and hands $p to you.", TRUE, me, obj, ch,TO_VICT);
          delete obj;
          obj = NULL;
        } else {
          act("$n smiles and hands a holy symbol to $N.", TRUE, me, NULL, ch,TO_NOTVICT);
          act("$n smiles and hands a holy symbol to you.", TRUE, me, NULL, ch,TO_VICT);
        }
        personalize_object(NULL, ch, 500, -1);
        me->doTell(ch->getName(), "May it serve you well. Be more careful next time!");
        found = 2;
        if (ch->hasClass(CLASS_CLERIC))
          duration = max(1, (ch->GetMaxLevel() / 3)) * 48 * UPDATES_PER_MUDHOUR;
      }
      break;
    case 3:
      if (ch->hasClass(CLASS_MONK)) {
        me->doTell(ch->getName(), "You are a monk.  I only help those who directly need the help.");
        return TRUE;
      }
      for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end() && (i=*it);++it) {
        if (dynamic_cast<TGenWeapon *>(i)) {
          me->doTell(ch->getName(), "You already have a weapon.  I only help the the totally destitute.");
          return TRUE;
        }
      }
      if (ch->affectedBySpell(AFFECT_NEWBIE)) {
        found = 1;
      } else {
        act("$n rummages through a bin and selects an item.", TRUE, me, NULL, NULL, TO_ROOM);
        if (ch->hasClass(CLASS_CLERIC)) {
          if ((r_num = real_object(324)) >= 0) {
            obj = read_object(r_num, REAL);
            *ch += *obj;   //  newbie staff 
          } else {
            vlogf(LOG_BUG, "Problem in NewbieEquipper, staff");
            return TRUE;
          }
        } else {
          if ((r_num = real_object(Obj::WEAPON_T_DAGGER)) >= 0) {
            obj = read_object(r_num, REAL);
            *ch += *obj;    // newbie dagger 
          } else {
            vlogf(LOG_BUG, "Problem in NewbieEquipper, dagger");
            return TRUE;
          }
        }
        act("$n smiles and hands $p to $N.", TRUE, me, obj, ch,TO_NOTVICT);
        act("$n smiles and hands $p to you.", TRUE, me, obj, ch,TO_VICT);
        me->doTell(ch->getName(), "May it serve you well. Be more careful next time!");
        found = 2;
        duration = max(1, (ch->GetMaxLevel() / 3)) * 48 * UPDATES_PER_MUDHOUR;
      }
      break;
    default:
      return TRUE;
      break;
  }
  if (found == 2) {
    if (!ch->affectedBySpell(AFFECT_NEWBIE)) {
      af.type = AFFECT_NEWBIE;
      af.level = 0;
      // roughly 4 hours
      if (duration)
        af.duration = duration; 
      else
        af.duration = 4 * UPDATES_PER_MUDHOUR;
      ch->affectTo(&af);
    }
    vlogf(LOG_MISC,format("%s was given newbie gear by %s case %d") %  ch->getName() % me->getName() % request);
    if (me->desc) {
      vlogf(LOG_MISC, format("Switched god used newbieEquip  %s by %s") %  ch->getName()  % me->getName());
    }
  } else if (found == 1) {
    me->doTell(ch->getName(), "You just used my service.  Come back later and only if you haven't gotten other help.");
    return TRUE;
  } else {
    vlogf(LOG_BUG, format("Somehow something got through equipNewbie %s by %s") %  ch->getName() % me->getName());
  }
  return TRUE;
}

// really, this should be using doMove or moveOne
void move_thing_forshop(TThing *what, int to)
{
  TThing *tied = what->tied_to;
  if (tied)
  {
    --(*tied);
    thing_to_room(tied, to);
  }

  --(*what);
  thing_to_room(what, to);

  what->tied_to = tied;
  if (tied)
    tied->tied_to = what;
}

// shared function to kick horses and junk from your shop
int kick_mobs_from_shop(TMonster *myself, TBeing *ch, int from_room)
{
  if (dynamic_cast<TBeing *>(ch->riding)) {
    sstring buf;
    buf = format("Hey, get that damn %s out of my shop!") % fname(ch->riding->name);
    myself->doSay(buf);

    if (!dynamic_cast<TMonster *>(ch)) {
      act("You throw $N out.", FALSE, myself, 0, ch, TO_CHAR);
      act("$n throws you out of $s shop.", FALSE, myself, 0, ch, TO_VICT);
      act("$n throws $N out of $s shop.", FALSE, myself, 0, ch, TO_NOTVICT);

      move_thing_forshop(ch->riding, from_room);
      move_thing_forshop(ch, from_room);

    } else {
      // Just kick out the mount, not the mobile. -Lapsos
      TThing *tMount = ch->riding;

      act("You throw $N out.", FALSE, myself, 0, ch, TO_CHAR);
      act("$n throws your mount out of $s shop.", FALSE, myself, 0, ch, TO_VICT);
      act("$n throws $N out of $s shop.", FALSE, myself, 0, ch->riding, TO_NOTVICT);

      ch->dismount(POSITION_STANDING);
      move_thing_forshop(tMount, from_room);
    }

    return TRUE;
  } else if (dynamic_cast<TBeing *>(ch->rider)) {
    if (!dynamic_cast<TMonster *>(ch->rider)) {
      move_thing_forshop(ch, from_room);
      move_thing_forshop(ch->rider, from_room);
    } else {
      // Just kick out the mount, not the mobile. -Lapsos
      ch->rider->dismount(POSITION_STANDING);
      move_thing_forshop(ch, from_room);
    }

    return TRUE;
  }

  return FALSE;
}


int librarian(TBeing *ch, cmdTypeT cmd, const char * arg, TMonster *myself, TObj *)
{
  affectedData aff;

  if (!ch->isPc()) {
    return FALSE;
  }

  if (cmd != CMD_SAY && cmd != CMD_SHOUT)
    return FALSE;
  if (!myself->awake())
    return FALSE;
  if (!myself->canSee(ch))
    return FALSE;

  if (cmd == CMD_SAY || cmd == CMD_SHOUT) {
    if (!ch->canSpeak())
      return FALSE;
    if (!ch->isImmortal()) {
      act("$n starts to say something!", TRUE, ch, 0, 0, TO_ROOM);
      act("$N interrupts you before you can say anything!", TRUE, ch, 0, myself, TO_CHAR);
    } else {
      if (cmd == CMD_SAY)
        ch->doSay(arg);
      else
        ch->doShout(arg);
    }
    myself->doSay("None of *that* in my library!");
    act("$n glares at $N and starts to whisper a mantra in a low voice!", FALSE, myself, 0, ch, TO_NOTVICT);
    act("$n glares at you and starts to whisper a mantra in a low voice!", FALSE, myself, 0, ch, TO_VICT);
    act("An orange beam of light streaks from $n's hands encasing $N's vocal cords.", TRUE, myself, 0, ch, TO_NOTVICT, ANSI_ORANGE);
    act("An orange beam of light streaks from $n's hands encasing your vocal cords.", TRUE, myself, 0, ch, TO_VICT, ANSI_ORANGE);

    if (ch->isImmortal()) {
      act("The foolish mortal actually tried to silence $N!", FALSE, myself, 0, ch, TO_NOTVICT);
      act("The foolish mortal actually tried to silence you!", FALSE, myself, 0, ch, TO_VICT);
    } else { 
      act("$n smiles with satisfaction!", FALSE, myself, 0, ch, TO_NOTVICT);
      act("$n smiles with satisfaction!", TRUE, myself, 0, ch, TO_VICT);
      aff.type = SPELL_SILENCE;
      aff.level = 0;
      aff.duration =  12 * UPDATES_PER_MUDHOUR;
      aff.modifier = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = AFF_SILENT;
      ch->affectTo(&aff);
    }
    return TRUE;
  }
  return FALSE;
}


static int moveHorseman(TMonster *myself)
{
  // we get here if horseman has problem finding path to it's random target
  // we eliminate random target (choosing new one)
  // and randomly teleport too in case we are just in totally bad spot
  // to begin with

  myself->opinion.random = NULL;
  TThing * t = myself->riding;
  if (t) {
    // trash the horse, it causes problems
    myself->dismount(POSITION_STANDING);
    delete t;
    t = NULL;
  }

  myself->genericTeleport(SILENT_NO);   // bounce to new locale

  return FALSE;
}

// this function will cause a mob to attempt to hunt down a given critter
// once it gets there, it will look for a new mob to hunt and track them down
// looks at both mobs and PC's
int TMonster::randomHunt()
{
  TBeing *hunted;
  int rc;
  
  // this opinion.target stuff works because the horsemen are UtilMobProcs
  // and the mobAI stuff doesn't apply to them

  // try and find a target if none
  if (!(hunted = opinion.random)) {
    for (hunted = character_list;hunted;hunted = hunted->next) {
      if (!::number(0,350)) {
        opinion.random = hunted;
        break;
      }
    }
    return FALSE;
  } else {
    int room = hunted->in_room;
    dirTypeT dir;
    TPathFinder path;

    if (in_room != room) {
      dir = path.findPath(in_room, findRoom(room));

      if (!exitDir(dir) || !real_roomp(exitDir(dir)->to_room) || dir<MIN_DIR){
        // unable to find a path 
	if(spec==SPEC_HORSE_FAMINE ||
	   spec==SPEC_HORSE_WAR ||
	   spec==SPEC_HORSE_DEATH ||
	   spec==SPEC_HORSE_PESTILENCE)
	  moveHorseman(this);
	else
	  opinion.random = NULL;

        return FALSE;
      }
      // This if statement prevents mounted from being stuck
      if (dir < MAX_DIR &&
	  real_roomp(exitDir(dir)->to_room)->isRoomFlag(ROOM_INDOORS) &&
	  (spec==SPEC_HORSE_FAMINE ||
	   spec==SPEC_HORSE_WAR ||
	   spec==SPEC_HORSE_DEATH ||
	   spec==SPEC_HORSE_PESTILENCE)){
	moveHorseman(this);
      } else {
        rc = goDirection(dir);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
      }
    } else {
      // we've found our wander target
      doAction(fname(hunted->name),CMD_GRIN);
      opinion.random = NULL;
    }
  }
  return TRUE;
}

int TMonster::findMyHorse()
{
  TBeing *horse, *horse_temp;
  int horse_num;
  dirTypeT dir;
  int rc;
  TPathFinder path;

  // this is here to prevent the endless horse create/fall to death scenario

  if(roomp && roomp->isFallSector())
    return FALSE;

  switch (spec) {
    case SPEC_HORSE_PESTILENCE:
      horse_num = Mob::APOC_PESTHORSE;
      break;
    case SPEC_HORSE_WAR:
      horse_num = Mob::Mob::APOC_WARHORSE;
      break;
    case SPEC_HORSE_FAMINE:
      horse_num = Mob::Mob::APOC_FAMINEHORSE;
      break;
    case SPEC_HORSE_DEATH:
      horse_num = Mob::Mob::APOC_DEATHHORSE;
      break;
    default:
      return FALSE;
  }
  for (horse = character_list;horse;horse = horse_temp) {
    horse_temp = horse->next;
    if (horse->mobVnum() == horse_num)
      break;
  }
  if (!horse) {
    doSay("Hey, time to get a new horse.");
    act("$n makes a strange, backwards gesture.",FALSE, this, 0, 0, TO_ROOM);
    horse = read_mobile(horse_num, VIRTUAL);
    *roomp += *horse;
    act("$n pops into being.", FALSE, horse, 0, 0, TO_ROOM);
    return TRUE;
  }
  if (!sameRoom(*horse)) {

    if((dir=path.findPath(in_room, findRoom(horse->in_room))) < 0){
      // unable to find a path 
      if (horse->in_room >= 0) {
        doSay("Bloody stupid horse!");
        act("$n has left into the void.",0, this, 0, 0, TO_ROOM);
        --(*this);
        *horse->roomp += *this;
        act("$n steps out of the ether.", 0, this, 0, 0, TO_ROOM);
        doAction(fname(horse->name),CMD_SCOLD);
      }
      return TRUE;
    }
    rc = goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  } else {
    rc = doMount("",CMD_MOUNT, horse);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete horse;
      horse = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  }
}


int insulter(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *vict = NULL;
  TThing *t=NULL;
  if ((cmd != CMD_GENERIC_PULSE) || !myself->awake())
    return FALSE;
  if (::number(0,3))
    return FALSE;

  if (myself->fight())
    vict = myself->fight();
  else {
    for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end() && (t=*it);++it) {
      vict = dynamic_cast<TBeing *>(t);
      if (!vict)
        continue;
      if (t == myself)
        continue;
      if (::number(0,1))
        break;
    }
  }

  if (!vict)
    return FALSE;
  myself->setAnger(70);
  myself->setMalice(0);  
  myself->aiInsultDoer(vict);

  return TRUE;
}


int siren(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int rc;
  TBeing *vict = NULL;
  roomDirData *exitp, *back;
  dirTypeT door;
  TRoom *rp, *rp2;
  TThing *t=NULL;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (!(rp = myself->roomp))
    return FALSE;
  for (door = MIN_DIR;door < MAX_DIR; door++) {
    if (!(exitp = rp->dir_option[door]))
      continue;
    if (!(rp2 = real_roomp(exitp->to_room)))
      continue;
    if (!(back = rp2->dir_option[rev_dir[door]]))
      continue;
    if (rp != real_roomp(back->to_room))
      continue;

    for(StuffIter it=rp2->stuff.begin();it!=rp2->stuff.end();){
      t=*(it++);
      vict = dynamic_cast<TBeing *>(t);
      if (!vict)
        continue;
      if (!vict->isPc() && !vict->desc)
        continue;
      if (!vict->awake())
        continue;
      if (vict->isImmortal() ||
          (vict->getSex() != SEX_MALE) ||
          vict->isImmune(IMMUNE_CHARM, WEAR_BODY)) {
        vict->sendTo("You hear a siren song in the distance but are able to resist its allure...\n\r");
        continue;
      }
      if (vict->isAffected(AFF_CHARM)) {
        vict->sendTo("You hear a siren song in the distance but your existing charm is too strong for you to be drawn.\n\r");
        continue;
      }
      vict->sendTo("You hear a siren song and feel compelled to follow its allure...\n\r");
      rc = vict->moveOne(rev_dir[door]);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete vict;
        vict = NULL;
      }
    }  
  }

  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end() && (t=*it);++it) {
    vict = dynamic_cast<TBeing *>(t);
    if (!vict)
      continue;
    if (vict == myself)
      continue;
    if (isname("siren", vict->name))
      continue;
    if (vict->isImmortal()) {
      act("The silly siren tried to charm you.",
          FALSE, vict, 0, myself, TO_CHAR);
      continue;
    }
    if (!vict->awake())
      continue;
    if (!vict->isPc() && !vict->desc)
      continue;
    if (vict->isAffected(AFF_CHARM)) {
      if (vict->master == myself) {
        continue;
      } else {
      act("$N tried to charm you even though you are already charmed.",
          FALSE, vict, 0, myself, TO_CHAR);
      continue;
      }
    }
    if (vict->isImmune(IMMUNE_CHARM, WEAR_BODY) ||
        (vict->getSex() != SEX_MALE)) {
      act("You are just barely able to resist being charmed by $N's song.",
          FALSE, vict, 0, myself, TO_CHAR);
      continue;
    }
    act("$N charms you with $S beautiful voice.",
        FALSE, vict, 0, myself, TO_CHAR);
    act("$N charms $n with $S beautiful voice.",
        FALSE, vict, 0, myself, TO_ROOM);

    if (vict->master)
      vict->stopFollower(TRUE);
    myself->addFollower(vict);

    affectedData aff;
    int level = myself->GetMaxLevel();

    aff.type = SPELL_ENSORCER;
    aff.level = level;
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    aff.duration  =  level/5 * PULSE_COMBAT;

    vict->affectTo(&aff);
  }
  return TRUE;
}

static int rob_blind(TBeing *ch, TBeing *vict)
{
  // make all checks prohibiting stealing before coming in here
  TThing *t;
  sstring name, buf;
 
  if (ch->fight() || vict->fight())
    return FALSE;
  if ((ch->getRace() == RACE_HOBBIT) && (ch->getSkillValue(SKILL_STEAL) < 80))
    ch->setSkillValue(SKILL_STEAL, 80);

  for(StuffIter it=vict->stuff.begin();it!=vict->stuff.end();){
    t=*(it++);
    if (::number(0,4) || !ch->canSee(t))
      continue;
    TObj *tmp_obj = dynamic_cast<TObj *>(t);
    if (tmp_obj && tmp_obj->isMonogrammed())
      continue;
    name=fname(t->name);
    buf=format("%s %s") % name % fname(vict->name);
    if (ch->getRace() == RACE_HOBBIT) 
      act("$n says, \"Hey $N, I'm just going to borrow your $o for a bit.\"",
	  TRUE, ch, t, vict, TO_ROOM);
    
    return ch->doSteal(buf, vict);
  }
  return FALSE;
}

int thief(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *)
{
  TBeing *cons = NULL;
  int rc;

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake())
    return FALSE;

  if (ch->getPosition() != POSITION_STANDING)
    return FALSE;

  if (ch->fight())
    return FALSE;

  TThing *t1;
  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end();){
    t1=*(it++);
    cons = dynamic_cast<TBeing *>(t1);
    if (!cons)
      continue;
    if ((cons == ch) || cons->fight() || !ch->canSee(cons))
      continue;
    if ((cons->isImmortal()) || (::number(0,25)))
      continue;
    rc = rob_blind(ch, cons);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete cons;
      cons = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;
    return TRUE;
  }
  return FALSE;
}

int Summoner(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *)
{
  Descriptor *d;
  TBeing *targ = 0;
  charList *i;
  int rc = 0;

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake())
    return FALSE;

  if (ch->checkSoundproof())
    return FALSE;

  if (ch->fight())
    return FALSE;

  if (ch->getHit() > ((ch->hitLimit() * 3) / 4)) {
    if (ch->hatefield & HATE_CHAR) {
      if (ch->hates.clist) {
        for (i = ch->hates.clist; i; i = i->next) {
          for (d = descriptor_list; d; d = d->next) {
            if ((d->connected == CON_PLYNG) && (d->character) && (!strcmp(d->character->getName(), i->name))) {
              targ = d->character;
              break;
            }
          }
        }
      }
    }
    if (targ) {
      act("$n utters the words 'Your ass is mine!'.", 1, ch, 0, 0, TO_ROOM);
      rc = ch->doDiscipline(SPELL_SUMMON, fname(targ->name).c_str());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      if (targ->sameRoom(*ch)) {
        rc = ch->hit(targ);
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          delete targ;
          targ = NULL;
        } else if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;  // delete ch
        }
      }
      return FALSE;
    } else
      return FALSE;
  } else
    return FALSE;
}

int replicant(TBeing *ch, cmdTypeT cmd, const char *, TMonster *, TObj *)
{
  TBeing *mob;

  if ((cmd != CMD_GENERIC_PULSE))
    return FALSE;

  if (ch->getHit() < ch->hitLimit()) {
    act("Drops of $n's blood hit the $g, and spring up into another one!", TRUE, ch, 0, 0, TO_ROOM);
    if (ch->number >= 0) {
      mob = read_mobile(ch->number, REAL);
      *ch->roomp += *mob;
      act("Two undamaged opponents face you now.", TRUE, ch, 0, 0, TO_ROOM);
      ch->setHit(ch->hitLimit());
    } else
      vlogf(LOG_PROC, "spec_mobs: replicant created from MEDit mob failed.");
  }
  return FALSE;
}


int tormentor(TBeing *ch, cmdTypeT cmd, const char *, TMonster *, TObj *)
{
  if (cmd == CMD_GENERIC_PULSE)
    return FALSE;

  if (!ch->isPc())
    return FALSE;

  if (ch->isImmortal())
    return FALSE;

  return TRUE;
}



int StatTeller(TBeing *pch, cmdTypeT cmd, const char *, TMonster *, TObj *)
{
  int choice;
  char buf[200];
  TBeing *ch;

  const int STAT_PRICE = 10000;

  if ((cmd != CMD_GENERIC_PULSE)) {
    ch = pch;
    if (cmd == CMD_BUY) {
      if (ch->getMoney() < STAT_PRICE) {
        ch->sendTo("You do not have the money to pay me.\n\r");
        return (TRUE);
      } else {
        ch->addToMoney(-STAT_PRICE, GOLD_HOSPITAL);
      }

      choice = number(0, 2);
      switch (choice) {
        case 0:
          sprintf(buf, "STR: %d, WIS: %d, DEX: %d\n\r",
                  ch->getStat(STAT_CURRENT,STAT_STR),
                  ch->getStat(STAT_CURRENT,STAT_WIS),
                  ch->getStat(STAT_CURRENT,STAT_DEX));
          ch->sendTo(buf);
          break;
        case 1:
          sprintf(buf, "INT: %d, DEX:  %d, CON: %d \n\r",
                  ch->getStat(STAT_CURRENT,STAT_INT),
                  ch->getStat(STAT_CURRENT,STAT_DEX),
                  ch->getStat(STAT_CURRENT,STAT_CON));
          ch->sendTo(buf);
          break;
        case 2:
          sprintf(buf, "CON: %d, INT: %d , WIS: %d \n\r",
                  ch->getStat(STAT_CURRENT,STAT_CON),
                  ch->getStat(STAT_CURRENT,STAT_INT),
                  ch->getStat(STAT_CURRENT,STAT_WIS));
          ch->sendTo(buf);
          break;
        default:
          ch->sendTo("We are experiencing Technical difficulties\n\r");
          return TRUE;
      }

    } else {
      return FALSE;
    }
  }
  return FALSE;
}

void TBeing::throwChar(TBeing *v, dirTypeT dir, bool also, silentTypeT silent, bool forceStand)
{
  TRoom *rp;
  int oldr;
  char buf[256];

  rp = v->roomp;
  if (rp && rp->dir_option[dir] &&
      rp->dir_option[dir]->to_room && 
      !IS_SET(rp->dir_option[dir]->condition, EX_CLOSED) &&
      (rp->dir_option[dir]->to_room != Room::NOWHERE)) {
    if (v->fight() && !silent) {
      sendTo("Not while fighting!\n\r");
      return;
    }

    if (forceStand && v->isFlying()) {
      act("You bat $N out of the air.",
	  false, this, 0, v, TO_CHAR);
      act("$n bats you out of the air.",
	  false, this, 0, v, TO_VICT);
      act("$n bats $N out of the air..",
	  false, this, 0, v, TO_NOTVICT);
      v->setPosition(POSITION_STANDING);
    }

    if (forceStand && v->riding) {
      act("You knock $N off $S mount.",
          false, this, 0, v, TO_CHAR);
      act("$n knocks you off your mount.",
          false, this, 0, v, TO_VICT);
      act("$n knock $N off $S mount.",
          false, this, 0, v, TO_NOTVICT);
      v->fallOffMount(v->riding, POSITION_SITTING);
    }


    if (forceStand && v->getPosition() < POSITION_STANDING) {
      act("You drag $N to $S feet.",
                false, this, 0, v, TO_CHAR);
      act("$n drags you to your feet.",
                false, this, 0, v, TO_VICT);
      act("$n drags $N to $S feet.",
                false, this, 0, v, TO_NOTVICT);
      v->setPosition(POSITION_STANDING);
    }

    if (v->getPosition() != POSITION_STANDING && !silent) {
      sendTo("You can't throw someone who's not standing.\n\r");
      return;
    }
    if(!silent){
      sendTo(COLOR_MOBS, format("You push %s %s out of the room.\n\r") % v->getName() % dirs[dir]);
      v->sendTo(COLOR_MOBS, format("%s pushes you %s out of the room.\n\r") % sstring(getName()).cap() % dirs[dir]);
      sprintf(buf, "$N is pushed %s out of the room by $n.", dirs[dir]);
      act(buf, TRUE, this, 0, v, TO_NOTVICT);
    }
    oldr = v->in_room;
    --(*v);
    if (also) {
      --(*this);
      thing_to_room(this, real_roomp(oldr)->dir_option[dir]->to_room);
    }
    thing_to_room(v, real_roomp(oldr)->dir_option[dir]->to_room);

    if(!silent){
      act("$n is pushed into the room.", TRUE, v, 0, 0, TO_ROOM);
    }
    v->doLook("", CMD_LOOK);
    if (also)
      doLook("", CMD_LOOK);
    v->addToWait(combatRound(1));
  } else {
    if(!silent){
      act("You slam $N into the wall!", FALSE, this, 0, v, TO_CHAR);
      act("$n slams you into the wall!", FALSE, this, 0, v, TO_VICT);
      act("$N is slammed into the wall by $n!", TRUE, this, 0, v, TO_NOTVICT);
    }
  }
}

void TBeing::throwChar(TBeing *v, int to_room, bool also, silentTypeT silent, bool forceStand)
{
  TRoom *rp;
  int oldr;
  char buf[256];

  rp = v->roomp;
  if (rp && to_room != Room::NOWHERE) {
    if (v->fight() && !silent) {
      sendTo("Not while fighting!\n\r");
      return;
    }

    if (forceStand && v->isFlying()) {
      act("You bat $N out of the air.",
	  false, this, 0, v, TO_CHAR);
      act("$n bats you out of the air.",
	  false, this, 0, v, TO_VICT);
      act("$n bats $N out of the air..",
	  false, this, 0, v, TO_NOTVICT);
      v->setPosition(POSITION_STANDING);
    }

    if (forceStand && v->riding) {
      act("You knock $N off $S mount.",
          false, this, 0, v, TO_CHAR);
      act("$n knocks you off your mount.",
          false, this, 0, v, TO_VICT);
      act("$n knock $N off $S mount.",
          false, this, 0, v, TO_NOTVICT);
      v->fallOffMount(v->riding, POSITION_SITTING);
    }


    if (forceStand && v->getPosition() < POSITION_STANDING) {
      act("You drag $N to $S feet.",
                false, this, 0, v, TO_CHAR);
      act("$n drags you to your feet.",
                false, this, 0, v, TO_VICT);
      act("$n drags $N to $S feet.",
                false, this, 0, v, TO_NOTVICT);
      v->setPosition(POSITION_STANDING);
    }

    if (v->getPosition() != POSITION_STANDING && !silent) {
      sendTo("You can't throw someone who's not standing.\n\r");
      return;
    }
    if(!silent){
      sendTo(COLOR_MOBS, format("You push %s out of the room.\n\r") % v->getName());
      v->sendTo(COLOR_MOBS, format("%s pushes you out of the room.\n\r") % sstring(getName()).cap());
      sprintf(buf, "$N is pushed out of the room by $n.");
      act(buf, TRUE, this, 0, v, TO_NOTVICT);
    }
    oldr = v->in_room;
    --(*v);
    if (also) {
      --(*this);
      thing_to_room(this, to_room);
    }
    thing_to_room(v, to_room);

    if(!silent){
      act("$n is pushed into the room.", TRUE, v, 0, 0, TO_ROOM);
    }
    v->doLook("", CMD_LOOK);
    if (also)
      doLook("", CMD_LOOK);
    v->addToWait(combatRound(1));
  } else {
    if(!silent){
      act("You slam $N into the wall!", FALSE, this, 0, v, TO_CHAR);
      act("$n slams you into the wall!", FALSE, this, 0, v, TO_VICT);
      act("$N is slammed into the wall by $n!", TRUE, this, 0, v, TO_NOTVICT);
    }
  }
}


// Bulge's proc
int payToll(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  dirTypeT dir;

  if (cmd != CMD_NORTH &&
      cmd != CMD_EAST &&
      cmd != CMD_SOUTH &&
      cmd != CMD_WEST &&
      cmd != CMD_UP &&
      cmd != CMD_DOWN &&
      cmd != CMD_NE &&
      cmd != CMD_NW &&
      cmd != CMD_SE &&
      cmd != CMD_SW &&
      cmd != CMD_FLEE &&
      cmd != CMD_DRAG &&
      cmd != CMD_OPEN)
    return FALSE;

  if (cmd >= CMD_NORTH && cmd <= CMD_SW) {
    dir = getDirFromCmd(cmd);
  } else if (cmd == CMD_FLEE) {
    if (myself == ch)
      return FALSE;
    act("$n blocks $N from fleeing!", false, myself, 0, ch, TO_NOTVICT);
    act("$n blocks you from fleeing!", false, myself, 0, ch, TO_VICT);
    myself->doSay("You can't escape that easily!");

    myself->roomp->playsound(SOUND_NONE_PASS, SOUND_TYPE_NOISE);

    return TRUE;
  } else if (cmd == CMD_DRAG) {
    // well, we aren't checking room, but oh well
    if(!myself->canSee(ch) || myself==ch || ch->isAnimal() || 
       !myself->awake()) {
      return FALSE;
    }

    myself->doSay("HEY!  This is a no dragging zone!");
    return TRUE;
  } else
    dir=dirTypeT(convertTo<int>(arg));

  switch(myself->inRoom()){
    case 1024:
      if(!myself->canSee(ch) || myself==ch || ch->isAnimal() || 
         !myself->awake() || myself->fight() || (cmd == CMD_OPEN)) {
	return FALSE;
      } else if(rev_dir[dir]==ch->specials.last_direction){
	act("$n growls but lets you return the way you came.",
	    FALSE, myself, 0, ch, TO_VICT);
	act("$n growls but lets $N return the way $e came.",
	    FALSE, myself, 0, ch, TO_NOTVICT);
	ch->remQuestBit(TOG_PAID_TOLL);
	return FALSE;
      } else if(ch->hasQuestBit(TOG_PAID_TOLL) || 
                dynamic_cast<TMonster *>(ch)){
	act("$n smiles happily and lets you pass.",
	    FALSE, myself, 0, ch, TO_VICT);
	act("$n smiles happily and lets $N pass.",
	    FALSE, myself, 0, ch, TO_NOTVICT);
	ch->remQuestBit(TOG_PAID_TOLL);
	return FALSE;
      } else {
	act("$n pushes you back roughly and demands the toll!",
	    FALSE, myself, 0, ch, TO_VICT);
	act("$n pushes $N back roughly and demands the toll!",
	    FALSE, myself, 0, ch, TO_NOTVICT);
        myself->roomp->playsound(SOUND_NONE_PASS, SOUND_TYPE_NOISE);
	return TRUE;
      }
      break;
    case 36060:
      if (!myself->canSee(ch) || (myself == ch) || ch->isAnimal() || !myself->awake() || myself->fight()) {
        return FALSE;
      } else if (((cmd == CMD_OPEN) && arg && is_abbrev(arg, "gate")) || (cmd == CMD_NORTH)) {
        TRoom  * tRoom   = real_roomp(36060);
        TObj   * tObj    = NULL;

	for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end();++it)
          if ((tObj = dynamic_cast<TObj *>(*it)) && (tObj->objVnum() == 36060))
            return FALSE;

        if (ch->master && ch->sameRoom(*(ch->master)))
          for(StuffIter it=ch->master->stuff.begin();it!=ch->master->stuff.end();++it)
            if ((tObj = dynamic_cast<TObj *>(*it)) && (tObj->objVnum() == 36060))
              return FALSE;

        if (ch->isPc())
          myself->doTell(ch->getName(), "Hey! Your not authorized to be here!");

        act("$n shoves you south out of the room!", FALSE, myself, 0, ch, TO_VICT);
        act("$n shoves $N south out of the room!", FALSE, myself, 0, ch, TO_NOTVICT);

        if (tRoom) {
          --(*ch);
          (*tRoom) += (*ch);

	  myself->doAction("", CMD_WHISTLE);
	  ch->doLook("", CMD_LOOK);
        } else
          vlogf(LOG_PROC, "Error loading room 36060 for forced creature movement.  (toll taker proc)");

        return TRUE;
      } else
        return FALSE;
      break;
    case 22713:
      if((rev_dir[dir]==ch->specials.last_direction) || (cmd == CMD_OPEN)){
	return FALSE;
      } else if(ch->hasQuestBit(TOG_PAID_TOLL) || 
                dynamic_cast<TMonster *>(ch)){
	act("$n disperses the forcefield and lets you through..",
	    FALSE, myself, 0, ch, TO_VICT);
	act("$n disperses the forcefield and lets $N through...",
	    FALSE, myself, 0, ch, TO_NOTVICT);
	ch->remQuestBit(TOG_LOGRUS_INITIATION);
	return FALSE;
      } else {
	act("A mysterious force prevents you from going through.",
	    FALSE, myself, 0, ch, TO_VICT);
	act("A mysterious force prevents $N from going through.",
	    FALSE, myself, 0, ch, TO_NOTVICT);
	return TRUE;
      }
      break;
    case 22724:
    case 22728:
    case 22736:
    case 22738:
    case 22748:
      if(((!myself->canSee(ch) || myself==ch)) || ch->isAnimal() ||
         myself->fight() || (cmd == CMD_OPEN)){
	return FALSE;
      } else if(rev_dir[dir]==ch->specials.last_direction){
	return FALSE;
      } else if((ch->hasQuestBit(TOG_PAID_TOLL) || 
                dynamic_cast<TMonster *>(ch))){
	switch(myself->mobVnum()){
	  case 22737:
	    act("$n turns away from you as you pass.",
		FALSE, myself, 0, ch, TO_VICT);
	    act("$n turns away as $N passes",
		FALSE, myself, 0, ch, TO_NOTVICT);
	    break;
	  case 22721:
	    act("$n blesses you as you pass by.",
		FALSE, myself, 0, ch, TO_VICT);
	    act("$n blesses $N as $E passes by.",
		FALSE, myself, 0, ch, TO_NOTVICT);
	    break;
	}
	ch->remQuestBit(TOG_PAID_TOLL);
	return FALSE;
      } else {
	switch(myself->mobVnum()){
  	  case 22724:
	    act("$n pulls you back into the fray.",
		FALSE, myself, 0, ch, TO_VICT);
	    act("$n pulls $N back into the fray.",
		FALSE, myself, 0, ch, TO_NOTVICT);
	    break;
   	  case 22728:
	    act("$n drags you back.",
		FALSE, myself, 0, ch, TO_VICT);
	    act("$n drags $N back.",
		FALSE, myself, 0, ch, TO_NOTVICT);
	    break;
	  case 22736:
	    act("$n shoves you back with a snarl.",
		FALSE, myself, 0, ch, TO_VICT);
	    act("$n shoves $N back with a snarl.",
		FALSE, myself, 0, ch, TO_NOTVICT);
	    break;
	  case 22737:
	    act("$n pulls you back with one of his tentacles.",
		FALSE, myself, 0, ch, TO_VICT);
	    act("$n pulls $N back with one of his tentacles.",
		FALSE, myself, 0, ch, TO_NOTVICT);
	    break;
	  case 22721:
	    act("$n bars your way past.",
		FALSE, myself, 0, ch, TO_VICT);
	    act("$n bars $N's way past.",
		FALSE, myself, 0, ch, TO_NOTVICT);
	    break;
	}
	return TRUE;
      }
      
      break;
  }
  return FALSE;
}

int ThrowerMob(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *vict;

  if (cmd == CMD_GENERIC_PULSE) {
    if (myself->awake() && myself->fight()) {
      vict = myself->fight();
      switch (myself->in_room) {
        case 13912:
          myself->throwChar(vict, DIR_EAST, FALSE, SILENT_NO, false);  // throw chars to the east 
          return (FALSE);
          break;
        default:
          return (FALSE);
      }
    }
  } else {
    switch (myself->in_room) {
      case 13912:{
          if (cmd == CMD_NORTH) {
            myself->sendTo("The Troll blocks your way.\n\r");
            return (TRUE);
          }
          break;
        }
      default:
        return (FALSE);
    }
  }
  return (FALSE);
}

// Swallower special 
int Tyrannosaurus_swallower(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int rc = 0;
  TBeing *targ;

  if ((cmd != CMD_GENERIC_PULSE) && cmd != CMD_STEAL)
    return (FALSE);

  if (cmd == CMD_STEAL) {
    ch->sendTo("You're much too afraid to steal anything!\n\r");
    return (TRUE);
  }
  if (::number(0,5))
    return FALSE;

  if (ch->getWait() > 0)
    return FALSE;
  if (ch->getPosition() < POSITION_STANDING)
    return FALSE;

  if (ch->awake()) {
    if ((targ = ch->findAnAttacker()) && !targ->isImmortal()) {
      act("$n opens $s gaping mouth.", TRUE, ch, 0, 0, TO_ROOM);

      ch->addToWait(combatRound(5));

      if (targ->isLucky(levelLuckModifier(myself->GetMaxLevel()))) {
        act("Thank the deities!  You are able to leap aside at the last moment.",
           TRUE, targ, 0, 0, TO_CHAR);
        act("Fate has saved you from a nasty death.",
           TRUE, targ, 0, 0, TO_CHAR);
        act("$n leaps away from $N's maw.",
           TRUE, targ, 0, ch, TO_ROOM);
        return TRUE;
      }
      if (targ->getHeight() <= (ch->getHeight()*3)) {
        if (!::number(0, 20)) {
          act("In a single gulp, $N is swallowed whole!",
                     TRUE, ch, 0, targ, TO_ROOM);
          targ->sendTo("In a single gulp, you are swallowed whole!\n\r");
          targ->sendTo("The horror!  The horror!\n\r");
          ch->sendTo("MMM.  yum!\n\r");

          ch->roomp->playsound(SOUND_CHEWED_UP, SOUND_TYPE_NOISE);

          if (targ->getMyRace()->hasTalent(TALENT_FROGSLIME_SKIN) && ::number(0,1))
          {
            act("Your skin-poison defenses activate in the panic!", TRUE, targ, 0, ch, TO_CHAR);
            act("$N starts to choke on you and spits you out!", TRUE, targ, 0, ch, TO_CHAR);
            act("Oh wait!  That creature's skin tastes horrible!", TRUE, targ, 0, ch, TO_VICT);
            act("You spit out the little blighter.", TRUE, targ, 0, ch, TO_VICT);
            act("$N looks sick.", TRUE, targ, 0, ch, TO_ROOM);
            act("$N spits out $n in disgust.", TRUE, targ, 0, ch, TO_ROOM);
            return TRUE;
          }

          vlogf(LOG_PROC, format("%s killed by being swallowed at %s (%d)") % 
              targ->getName() % targ->roomp->getName() % targ->inRoom());
          rc = targ->die(DAMAGE_EATTEN);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete targ;
            targ = NULL;
          }
          // there is no defense from this rawKill, so we should be able to
          // assume that first item in room is this player's corpse.
          // ie, corpse == rp->getStuff()

          // just a bit of a safety check
          TBaseCorpse *co = NULL;
          if ((co = dynamic_cast<TBaseCorpse *>(ch->roomp->stuff.front()))) {
            TThing *t;
	    for(StuffIter it=co->stuff.begin();it!=co->stuff.end();){
	      t=*(it++);
              --(*t);
              *ch += *t; 
            }
            delete co;        // remove the corpse 
            co = NULL;
          }
        } else { // if here we just mangle them REAL bad.
          act("$n brings $s maw down hard upon $N",
              TRUE, ch, 0, targ, TO_ROOM);
          rc = ch->reconcileDamage(targ, (ch->GetMaxLevel() * 3), DAMAGE_EATTEN);

          if (rc == -1) {
            targ->sendTo("Unfortunatly you were unable to withstand the pain...\n\r");
            ch->sendTo("Perhaps you can dine on them later.\n\r");

            delete targ;
            targ = NULL;
          } else {
            targ->sendTo("You are really messed up after that one.\n\r");
            ch->sendTo("You almost got them that time!\n\r");
          }
        }

        return TRUE;
      } else {
        act("$n tries to swallow $N, but $N is a little too big.", TRUE, ch, 0, targ, TO_ROOM);
        act("You try to swallow $N, but $N is a little too big.", TRUE, ch, 0, targ, TO_CHAR);
        act("$n tries to swallow you, but you're a little too big.", TRUE, ch, 0, targ, TO_VICT);
      }
    }
  }
  return FALSE;
}

static int frost_giant_stuff(TMonster *ch)
{
  TThing *t;
  TBeing *vict = NULL, *v2;
  int rc;
  followData *f, *n;
  bool shouted = FALSE;

  if ((v2 = ch->fight())) {
    for (f = ch->followers; f; f = n) {
      n = f->next;
      if ((vict = f->follower) && vict->inGroup(*ch) && !vict->fight()) {
        TMonster *tmons = dynamic_cast<TMonster *>(vict);
        if (!tmons)
          continue;
        if (!shouted) {
          act("$n screams, \"Destroy these puny scum!\"",
               FALSE, ch, 0, 0, TO_ROOM);
          shouted = TRUE;
        }
        rc = tmons->takeFirstHit(*v2);
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          delete v2;
          v2 = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tmons;
          tmons = NULL;
        }
      }
    }
    return shouted;
  }
  // attack anyone in room
  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end();){
    t=*(it++);
    vict = dynamic_cast<TBeing *>(t);
    if (!vict)
      continue;
    if ((vict == ch) || (vict == ch->riding) ||
        (vict->inGroup(*ch)) || vict->isImmortal())
      continue;
    if (!ch->canSee(vict) || !ch->awake())
      continue;
    ch->doAction("", CMD_GROWL);
    rc = ch->takeFirstHit(*vict);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete vict;
      vict = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    return TRUE;
  }
  return FALSE;
}

int frostGiant(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int res = 0, i;
  int rc;
  dirTypeT door;
  TBeing *mob;
  TPathFinder path;

  class hunt_struct {
    public:
      int cur_pos;
      bool completed;
  };
  hunt_struct *job;
 
  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<hunt_struct *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;
 
  if (::number(0,2))
    return FALSE;

  if (!myself->awake() || myself->fight())
    return FALSE;
 
  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new hunt_struct())) {
     perror("failed new of frost_giant.");
     exit(0);
    }
    job = static_cast<hunt_struct *>( myself->act_ptr);
    job->cur_pos = 0;
    job->completed = false;

    act("$n climbs down out of the mountains, roaring a challenge.",
        TRUE, myself, 0, 0, TO_ROOM);

    // make sure he can get through the gate
    myself->getDiscipline(getDisciplineNumber(SKILL_DOORBASH, 0))->setLearnedness(100);
    myself->setSkillValue(SKILL_DOORBASH, 95);
    res = ::number(2,5);
  
    act("A band of ice goblins comes to the aid of this mighty giant.",
        TRUE, myself, 0, 0, TO_ROOM);
    
    SET_BIT(myself->specials.affectedBy, AFF_GROUP);
    for (i= 0; i < res; i++) {
      if (!(mob = read_mobile(10221, VIRTUAL))) {
        vlogf(LOG_PROC, "Bad load of ice goblin.");
        continue;
      }
      *myself->roomp += *mob;
      myself->addFollower(mob);
      SET_BIT(mob->specials.affectedBy, AFF_GROUP);
    }
  }
  if (!(job = static_cast<hunt_struct *>( myself->act_ptr))) {
    vlogf(LOG_PROC, "Unable to allocate memory for frostGiant!  This is bad!");
    return TRUE;
  }

  if (myself->fight()) {
    rc = frost_giant_stuff(myself);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    else if (rc)
      return TRUE;
  }

  if (job->completed) {
    path.setThruDoors(true);

    if((door=path.findPath(myself->inRoom(), findPolice())) > DIR_NONE){
      rc = myself->goDirection(door);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      else if (rc) {
        rc = frost_giant_stuff(myself);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
      }
      return TRUE;
    }
  } else if (frosty_path_pos[(job->cur_pos + 1)].direction == -1) {
    // end of path
    job->completed = true;
    return TRUE;
  } else if (frosty_path_pos[job->cur_pos].cur_room != myself->in_room) {
    // not in correct room
    // check surrounding rooms, I probably fled
    for (door = MIN_DIR; door < MAX_DIR; door++) {
      if (myself->canGo(door)) {
        if (myself->roomp->dir_option[door]->to_room ==
                frosty_path_pos[job->cur_pos].cur_room) {
          rc = myself->goDirection(door);
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
          else if (rc) {
            rc = frost_giant_stuff(myself);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              return DELETE_THIS;
            }
          }

          return TRUE;
        }
      }
    }

    // trace along entire route and see if I can correct
    job->cur_pos = -1;
    do {
      job->cur_pos += 1;
      if (frosty_path_pos[job->cur_pos].cur_room == myself->in_room)
        return TRUE;
    } while (frosty_path_pos[job->cur_pos].cur_room != -1);
 
    if (myself->riding)
      myself->dismount(POSITION_STANDING);

#if 0
    return DELETE_THIS;  // if lost, destroy
#else
    // if lost, just go into completed routine
    job->completed = true;
    return FALSE;
#endif

  } else if (myself->getPosition() < POSITION_STANDING) {
    myself->doStand();
  } else {
    rc = myself->goDirection(frosty_path_pos[job->cur_pos + 1].direction);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    } else if (rc) {
      rc = frost_giant_stuff(myself);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      }
      job->cur_pos += 1;
    }
    // if go_dir == 0, then it either can't go that way or is opening a door
  }
  return 0;
}

// look in present room for a lamp, refill if necessary
static void lamp_stuff(TMonster *ch)
{
  TThing *lamp;
  lamp = searchLinkedListVis(ch, "lamppost", ch->roomp->stuff);
  if (lamp) {
    lamp->lampLightStuff(ch);
  }
}

int lamp_lighter(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int rc;

  class hunt_struct {
    public:
    byte cur_pos;
    byte cur_path;
    byte town;   // 0 = gh, 1 = brightmoon, 2 = logrus, 3-4 = amber 
                 // 6-9 obsidian keep
  };
  hunt_struct *job;
 
  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<hunt_struct *>( myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }

  if (::number(0,2))
    return FALSE;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;
 
  if (!myself->awake() || myself->fight())
    return FALSE;
 
  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new hunt_struct())) {
     perror("failed new of lamp_lighter.");
     exit(0);
    }
    job = static_cast<hunt_struct *>(myself->act_ptr);
    job->cur_pos = 0;
    if (myself->in_room == Room::CS) {
      job->town = 0;
      job->cur_path = ::number(MIN_GRIM_PATHS, MAX_GRIM_PATHS);
    } else if (myself->in_room == 1303) {
      job->town = 1;
      job->cur_path = ::number(MIN_BM_PATHS, MAX_BM_PATHS);
    } else if (myself->in_room == 3701) {
      job->town = 2;
      job->cur_path = ::number(MIN_LOGRUS_PATHS, MAX_LOGRUS_PATHS);
    } else if (myself->in_room == 7115) {
      job->town = 6;
      job->cur_path = KEEP_PATH_1;
    } else if (myself->in_room == 7271) {
      job->town = 7;
      job->cur_path = ::number(MIN_KEEP_PATH_2, MAX_KEEP_PATH_2);
    } else if (myself->in_room == 7061) {
      job->town = 8;
      job->cur_path = ::number(MIN_KEEP_PATH_3, MAX_KEEP_PATH_3); 
    } else if (myself->in_room == 7276) {
      job->town = 9;
      job->cur_path = ::number(MIN_KEEP_PATH_4, MAX_KEEP_PATH_4);
    } else if (myself->in_room == 8747) {
      job->town = 3;
      job->cur_path = ::number(MIN_AMBER_PATH_1, MAX_AMBER_PATH_1);
    } else if (myself->in_room == 8891) {
      job->town = 4;
      job->cur_path = AMBER_PATH_2; 
    } else {
      vlogf(LOG_PROC, "Bogus room load of lampboy");
      job->town = 0;
      job->cur_path = 0;
    }
  }
  if (!(job = static_cast<hunt_struct *>(myself->act_ptr))) {
    vlogf(LOG_PROC, "Unable to allocate memory for lamp_lighter!  This is bad!");
    return TRUE;
  }
  if (lamp_path_pos[job->cur_path][(job->cur_pos + 1)].direction == -1) {
    // end of path
    job->cur_pos = 0;

    if (myself->in_room == Room::CS) {
      job->cur_path = ::number(MIN_GRIM_PATHS, MAX_GRIM_PATHS);
    } else if (myself->in_room == 1303) {
      job->cur_path = ::number(MIN_BM_PATHS, MAX_BM_PATHS);
    } else if (myself->in_room == 3701) {
      job->cur_path = ::number(MIN_LOGRUS_PATHS, MAX_LOGRUS_PATHS);
#if 1
    } else if (myself->in_room == 8747) {
      job->cur_path = ::number(MIN_AMBER_PATH_1, MAX_AMBER_PATH_1);
    } else if (myself->in_room == 8891) {
      job->cur_path = AMBER_PATH_2;
#else
// old amber
    } else if (myself->in_room == 2783) {
      job->cur_path = AMBER_PATH_1;
    } else if (myself->in_room == 2768) {
      job->cur_path = AMBER_PATH_2;
    } else if (myself->in_room == 2776) {
      job->cur_path = AMBER_PATH_3;
#endif
    } else if (myself->in_room == 7115) {
      job->cur_path = KEEP_PATH_1;
    } else if (myself->in_room == 7271) {
      job->cur_path = ::number(MIN_KEEP_PATH_2, MAX_KEEP_PATH_2);
    } else if (myself->in_room == 7061) {
      job->cur_path = ::number(MIN_KEEP_PATH_3, MAX_KEEP_PATH_3);
    } else if (myself->in_room == 7276) {
      job->cur_path = ::number(MIN_KEEP_PATH_4, MAX_KEEP_PATH_4);
    }

    return TRUE;
  } else if (lamp_path_pos[job->cur_path][job->cur_pos].cur_room != myself->in_room) {
    // not in correct room
    // check surrounding rooms, I probably fled
    dirTypeT door;
    for (door = MIN_DIR; door < MAX_DIR; door++) {
      if (myself->canGo(door)) {
        if (myself->roomp->dir_option[door]->to_room ==
                lamp_path_pos[job->cur_path][job->cur_pos].cur_room) {
          rc = myself->goDirection(door);
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;

          return TRUE;
        }
      }
    }

    // trace along entire route and see if I can correct
    // vlogf(LOG_PROC, format("Lampboy got lost ip: path: %d, pos: %d, room: %d, should: %d") %  job->cur_path % job->cur_pos % myself->in_room % lamp_path_pos[job->cur_path][job->cur_pos].cur_room);
    job->cur_pos = -1;
    do {
      job->cur_pos += 1;
      if (lamp_path_pos[job->cur_path][job->cur_pos].cur_room == myself->in_room)
        return TRUE;
    } while (lamp_path_pos[job->cur_path][job->cur_pos].cur_room != -1);
 
    act("$n seems to have gotten a little bit lost.",0, myself, 0, 0, TO_ROOM);
    act("$n goes to ask directions.", 0, myself, 0, 0, TO_ROOM);
    //vlogf(LOG_PROC, format("Lampboy got lost: path: %d, pos: %d") %  job->cur_path % myself->in_room);
    if (myself->riding)
      myself->dismount(POSITION_STANDING);
    --(*myself);
    thing_to_room(myself, lamp_path_pos[job->cur_path][0].cur_room);
    act("$n has arrived.", 0, myself, 0, 0, TO_ROOM);
    lamp_stuff(myself);
    return TRUE;
  } else if (myself->getPosition() < POSITION_STANDING) {
    myself->doStand();
  } else {
    rc = myself->goDirection(lamp_path_pos[job->cur_path][job->cur_pos + 1].direction);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    } else if (rc) {
      lamp_stuff(myself);
      job->cur_pos += 1;
    }
    // if go_dir == 0, then it either can't go that way or is opening a door
  }
  return 0;
}

class caravan_struct {
  public:
    short int cur_pos;
    byte cur_path; // 0 = gh->bm, 1 = bm->gh
    factionTypeT dest_fact;
    int wealth;
    TObj *caravan;

    caravan_struct() :
      cur_pos(0),
      cur_path(0),
      dest_fact(FACT_UNDEFINED),
      wealth(0),
      caravan(NULL)
    {
    }
    ~caravan_struct()
    {
    }
};

// returns DELETE_THIS (car)
static int caravan_stuff(TBeing *car, caravan_struct *job, dirTypeT)
{
  // randomly make attackers appear along the way
  // attacker types to choose from
  // large thief  (700)    L16   cost 256
  const int cost_thief_large = 256;
  // road robber  (651)    L13   cost 169
  const int cost_robber = 169;
  // road thief   (650)    L9    cost 81
  const int cost_thief = 81;
  // ideally, we want 5 attackers

  const int cost_best = cost_thief_large;

  // base the strength of the attackers on how much wealth is being transported
  int wealth_max = job->wealth / 3;

  // this chance is pretty arbitrary, and ought to be low since it is
  // checked per step
  int chance;

  // for obvious reasons, lets not have robbers attack *IN* the cities
  if (car->roomp->isCitySector())
    chance = 0;
  else
    chance = 10;  // 1%

  // if caravan is extremely wealthy, raise the chance
  // basically, this says if there is a ton of wealth on the caravan (more
  // then the cost for our best set of attackers), raise the frequency of
  // attacks
  if (wealth_max > cost_best * 5)
    chance = chance * wealth_max / (cost_best * 5);

  if (::number(0,999) >= chance)
    return FALSE;

  int wealth_used = 0;
  bool loaded = false; 

  int num_loaded;
  for (num_loaded = 0; num_loaded < 5; num_loaded++) {
    int wealth_left = wealth_max - wealth_used;
    int wealth_per = wealth_left / (5 - num_loaded);

    TMonster * mon;
    if (wealth_per > cost_thief_large) {
      mon = read_mobile(Mob::Mob::ROAD_THIEF_LARGE, VIRTUAL);
    } else if (wealth_per > cost_robber) {
      mon = read_mobile(Mob::ROAD_ROBBER, VIRTUAL);
    } else if (wealth_per > cost_thief) {
      mon = read_mobile(Mob::ROAD_THIEF, VIRTUAL);
    } else {
      continue;
    }

    *car->roomp += *mon;

    // theoretically, the cost calculated here should be same as above
    double grl = mon->getRealLevel();
    wealth_used += (int) (grl * grl);

    if (!loaded) {
      act("Suddenly, everything goes strangely quiet!",
           false, car, 0, 0, TO_ROOM);
      act("Look Out!!!   AMBUSH!!!!",
           false, car, 0, 0, TO_ROOM);
      loaded = true;
    }

    int rc = mon->takeFirstHit(*car);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete mon;
      mon = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_THIS;
  }
  return FALSE;
}

int caravan(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int rc, i;
  bool ok;
  factionTypeT faction;
  int amount;
  char buf[256];
  TObj *obj;

  caravan_struct *job;
 
  if (cmd == CMD_GENERIC_DESTROYED) {
    if (!(job = (caravan_struct *) myself->act_ptr))
      return FALSE;
    obj = job->caravan;

    if (obj)
      obj->checkSpec(NULL, CMD_OBJ_WAGON_UNINIT, "", NULL);

    delete (caravan_struct *) myself->act_ptr;
    myself->act_ptr = NULL;

    return FALSE;
  }

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;
 
  if (::number(0,2) && !timeTill)
    return FALSE;

  if (!myself->awake() || myself->fight())
    return FALSE;
 
  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new caravan_struct())) {
      perror("failed new of caravan.");
      exit(0);
    }
    job = (caravan_struct *) myself->act_ptr;
    job->cur_pos = 0;
    ok = FALSE;
    if (myself->in_room == CAR_GH_HOME) {
      myself->setFaction(FACT_NONE);
      faction = myself->getFaction();
      REMOVE_BIT(FactionInfo[faction].caravan_flags, 
            CARAVAN_CUR_DEST_GH | CARAVAN_CUR_DEST_BM |
            CARAVAN_CUR_DEST_LOG | CARAVAN_CUR_DEST_AMBER);
      for (i = 0; i < 10 && !ok; i++) {
        switch (::number(1,3)) {
          case 1:
            if (IS_SET(FactionInfo[faction].caravan_flags, CARAVAN_DEST_BM)) {
              SET_BIT(FactionInfo[faction].caravan_flags, CARAVAN_CUR_DEST_BM);
              job->cur_path = CAR_GH_BM;
              job->dest_fact = FACT_BROTHERHOOD;
              ok = TRUE;
            }
            break;
          case 2:
            if (IS_SET(FactionInfo[faction].caravan_flags, CARAVAN_DEST_LOG)) {
              SET_BIT(FactionInfo[faction].caravan_flags, CARAVAN_CUR_DEST_LOG);
              job->cur_path = CAR_GH_LOG;
              job->dest_fact = FACT_CULT;
              ok = TRUE;
            }
            break;
          case 3:
            if (IS_SET(FactionInfo[faction].caravan_flags, CARAVAN_DEST_AMBER)) {
              SET_BIT(FactionInfo[faction].caravan_flags, CARAVAN_CUR_DEST_AMBER);
              job->cur_path = CAR_GH_AMB;
              job->dest_fact = FACT_SNAKE;
              ok = TRUE;
            }
            break;
          default:
            break;
        }
      }
      if (!ok)
        return DELETE_THIS;
    } else if (myself->in_room == CAR_BM_HOME) {
      myself->setFaction(FACT_BROTHERHOOD);
      faction = myself->getFaction();
      REMOVE_BIT(FactionInfo[faction].caravan_flags, 
            CARAVAN_CUR_DEST_GH | CARAVAN_CUR_DEST_BM |
            CARAVAN_CUR_DEST_LOG | CARAVAN_CUR_DEST_AMBER);
      for (i = 0; i < 10 && !ok; i++) {
        switch (::number(1,2)) {
          case 1:
            if (IS_SET(FactionInfo[faction].caravan_flags, CARAVAN_DEST_GH)) {
              SET_BIT(FactionInfo[faction].caravan_flags, CARAVAN_CUR_DEST_GH);
              job->cur_path = CAR_BM_GH;
              job->dest_fact = FACT_NONE;
              ok = TRUE;
            }
            break;
          case 2:
            if (IS_SET(FactionInfo[faction].caravan_flags, CARAVAN_DEST_AMBER)) {
              SET_BIT(FactionInfo[faction].caravan_flags, CARAVAN_CUR_DEST_AMBER);
              job->cur_path = CAR_BM_AMB;
              job->dest_fact = FACT_SNAKE;
              ok = TRUE;
            }
            break;
          default:
            break;
        }
      }
      if (!ok)
        return DELETE_THIS;
    } else if (myself->in_room == CAR_LOG_HOME) {
      myself->setFaction(FACT_CULT);
      faction = myself->getFaction();
      REMOVE_BIT(FactionInfo[faction].caravan_flags, 
            CARAVAN_CUR_DEST_GH | CARAVAN_CUR_DEST_BM |
            CARAVAN_CUR_DEST_LOG | CARAVAN_CUR_DEST_AMBER);
      for (i = 0; i < 10 && !ok; i++) {
        switch (::number(1,2)) {
          case 1:
            if (IS_SET(FactionInfo[faction].caravan_flags, CARAVAN_DEST_GH)) {
              SET_BIT(FactionInfo[faction].caravan_flags, CARAVAN_CUR_DEST_GH);
              job->cur_path = CAR_LOG_GH;
              job->dest_fact = FACT_NONE;
              ok = TRUE;
            }
            break;
          case 2:
            if (IS_SET(FactionInfo[faction].caravan_flags, CARAVAN_DEST_AMBER)) {
              SET_BIT(FactionInfo[faction].caravan_flags, CARAVAN_CUR_DEST_AMBER);
              job->cur_path = CAR_LOG_AMB;
              job->dest_fact = FACT_SNAKE;
              ok = TRUE;
            }
            break;

          default:
            break;
        }
      }
      if (!ok)
        return DELETE_THIS;
    } else if (myself->in_room == CAR_AMBER_HOME) {
      myself->setFaction(FACT_SNAKE);
      faction = myself->getFaction();
      REMOVE_BIT(FactionInfo[faction].caravan_flags, 
            CARAVAN_CUR_DEST_GH | CARAVAN_CUR_DEST_BM |
            CARAVAN_CUR_DEST_LOG | CARAVAN_CUR_DEST_AMBER);
      for (i = 0; i < 10 && !ok; i++) {
        switch (::number(1,3)) {
          case 1:
            if (IS_SET(FactionInfo[faction].caravan_flags, CARAVAN_DEST_GH)) {
              SET_BIT(FactionInfo[faction].caravan_flags, CARAVAN_CUR_DEST_GH);
              job->cur_path = CAR_AMB_GH;
              job->dest_fact = FACT_NONE;
              ok = TRUE;
            }
            break;
          case 2:
            if (IS_SET(FactionInfo[faction].caravan_flags, CARAVAN_DEST_BM)) {
              SET_BIT(FactionInfo[faction].caravan_flags, CARAVAN_CUR_DEST_BM);
              job->cur_path = CAR_AMB_BM;
              job->dest_fact = FACT_BROTHERHOOD;
              ok = TRUE;
            }
            break;
          case 3:
            if (IS_SET(FactionInfo[faction].caravan_flags, CARAVAN_DEST_LOG)) {
              SET_BIT(FactionInfo[faction].caravan_flags, CARAVAN_CUR_DEST_LOG);
              job->cur_path = CAR_AMB_LOG;
              job->dest_fact = FACT_CULT;
              ok = TRUE;
            }
            break;
          default:
            break;
        }
      }
      if (!ok)
        return DELETE_THIS;
    } else {
      vlogf(LOG_PROC, format("Bogus room load of caravan (%d)") %  myself->in_room);
      return DELETE_THIS;
    }
    FactionInfo[faction].caravan_attempts++;

    amount = min(FactionInfo[faction].caravan_value,
                  FactionInfo[faction].getMoney());
    job->wealth = amount;
    FactionInfo[faction].addToMoney(-amount);

    // construct caravan
    if (!(obj = read_object(Obj::CARAVAN, VIRTUAL))) {
      vlogf(LOG_PROC, "Problem with caravan load (1)");
      return TRUE;
    }
    *myself->roomp += *obj;

    obj->checkSpec(myself, CMD_OBJ_WAGON_INIT, "", NULL);

    job->caravan = obj;

    if (faction != FACT_NONE) {
      sprintf(buf, "A caravan has formed bound for %s.", 
             CaravanDestination(-faction - 1));
      sendToFaction(faction, myself, buf);
    }
    save_factions();

    // we've allocated job, drop through and take 1st step
  }
  if (!(job = (caravan_struct *) myself->act_ptr)) {
    vlogf(LOG_PROC, "Unable to allocate memory for caravan!  This is bad!");
    return TRUE;
  }

  faction = myself->getFaction();

  if (caravan_path_pos[job->cur_path][(job->cur_pos + 1)].direction == -1) {
    // end of path
    act("The caravan has arrived successfully!", 0, myself, 0, 0, TO_ROOM);
    FactionInfo[faction].caravan_successes++;
    FactionInfo[faction].addToMoney((200 * job->wealth / 100));
    FactionInfo[job->dest_fact].addToMoney((100 * job->wealth / 100));
    
    if (faction != FACT_NONE) {
      sprintf(buf, "A caravan has arrived successfully in %s.", CaravanDestination(-faction - 1));
      sendToFaction(faction, myself, buf);
    }
    save_factions();

    // make caravan go bye-bye
    delete job->caravan;
    job->caravan = NULL;

    return DELETE_THIS;
  } else if (caravan_path_pos[job->cur_path][job->cur_pos].cur_room != myself->in_room) {
    // not in correct room
    // check surrounding rooms, I probably fled
    dirTypeT door;
    for (door = MIN_DIR; door < MAX_DIR; door++) {
      if (myself->canGo(door)) {
        if (myself->roomp->dir_option[door]->to_room ==
                caravan_path_pos[job->cur_path][job->cur_pos].cur_room) {
          rc = myself->goDirection(door);
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;

          return TRUE;
        }
      }
    }

    // trace along entire route and see if I can correct
#if 1
    vlogf(LOG_PROC, format("Caravan got lost ip: path: %d, pos: %d, room: %d, should: %d") %  job->cur_path % job->cur_pos % myself->in_room % caravan_path_pos[job->cur_path][job->cur_pos].cur_room);
#endif
    job->cur_pos = -1;
    do {
      job->cur_pos += 1;
      if (caravan_path_pos[job->cur_path][job->cur_pos].cur_room == myself->in_room)
        return TRUE;
    } while (caravan_path_pos[job->cur_path][job->cur_pos].cur_room != -1);
 
    act("$n seems to have gotten a little bit lost.",0, myself, 0, 0, TO_ROOM);
    act("$n goes to ask directions.", 0, myself, 0, 0, TO_ROOM);
#if 1
    vlogf(LOG_PROC, format("Caravan got lost: path: %d, pos: %d") %  job->cur_path % myself->in_room);
#endif
    if (myself->riding)
      myself->dismount(POSITION_STANDING);
    --(*myself);
    thing_to_room(myself, caravan_path_pos[job->cur_path][0].cur_room);
    act("$n has arrived.", 0, myself, 0, 0, TO_ROOM);
    rc = caravan_stuff(myself, job, DIR_NONE);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    return TRUE;

  } else if (myself->getPosition() < POSITION_STANDING) {
    myself->doStand();
    return TRUE;
  } else {
    rc = myself->goDirection(caravan_path_pos[job->cur_path][job->cur_pos + 1].direction);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    } else if (rc) {
      rc = caravan_stuff(myself, job, caravan_path_pos[job->cur_path][job->cur_pos + 1].direction);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      job->cur_pos += 1;

      if (timeTill && (job->cur_pos % 5)) {
        // if we are in shutdown mode, move 5 steps at a time
        return caravan(NULL, cmd, NULL, myself, NULL);
      }
    }
    // if go_dir == 0, then it either can't go that way or is opening a door
    return TRUE;
  }
  return 0;
}


// ch calls a guard to come after vict
void CallForGuard(TBeing *ch, TBeing *vict, int lev)
{
  TBeing *m = NULL;
  TMonster *tmons = NULL;
  int i = 0;

  if (!vict) {
    vlogf(LOG_PROC, "No victim in CallForGuard");
    return;
  }

  for (m = character_list, i = 0; m && i < lev; m = m->next) {
    if (m->isPc() || m == ch || m == vict)
      continue;
    tmons = dynamic_cast<TMonster *>(m);
    if (!tmons) 
      continue;
    if (tmons->spec != SPEC_CITYGUARD)
      continue;
    // get only critters in my zone
    // treat grimhaven as all one zone
    if (tmons->roomp->getZoneNum() != vict->roomp->getZoneNum() &&
        !(tmons->inGrimhaven() && vict->inGrimhaven()))
      continue;
    if (tmons->fight() || !::number(0,4))
      continue;

    if (!IS_SET(tmons->specials.act, ACT_HUNTING)) {
      tmons->setHunting(vict);
      i++;
      tmons->act_ptr = ch;
    }
    else if (tmons->act_ptr == ch)
      i++;
  }
}

int dagger_thrower(TBeing *pch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TObj *dagger;
  TBeing *tmp_ch, *ch, *temp;
  int range;
  dirTypeT dir;
  sstring buf;

  if ((cmd != CMD_GENERIC_PULSE) || !pch->awake())
    return (FALSE);
  ch = pch;

  for (tmp_ch = character_list; tmp_ch; tmp_ch = temp) {
    temp = tmp_ch->next;
    can_see_linear(me, tmp_ch, &range, &dir);
    if (dir != DIR_NONE) {
      if (tmp_ch->isPc()) {
        act("$n screams 'All intruders must die!!!'", FALSE, me, 0, tmp_ch, TO_ROOM);
        act("$n utters the words 'ssa skcik siht'", FALSE, me, 0, 0, TO_ROOM);

#if 1
// builder port uses stripped down database which was causing problems
// hence this setup instead.
        int robj = real_object(Obj::GENERIC_DAGGER);
        if (robj < 0 || robj >= (signed int) obj_index.size()) {
          vlogf(LOG_BUG, format("dagger_thrower(): No object (%d) in database!") % 
                Obj::GENERIC_DAGGER);
          return FALSE;
        }
    
        if (!(dagger = read_object(robj, REAL))) {
          vlogf(LOG_BUG, "Couldn't make a dagger for dagger_thrower()!");
          return FALSE;
        }
#else
        dagger = read_object(Obj::GENERIC_DAGGER, VIRTUAL);
#endif

        if (!me->equipment[HOLD_RIGHT])
          me->equipChar(dagger, HOLD_RIGHT);
        else {
          vlogf(LOG_BUG, format("Dagger_thrower problem: equipped right hand.  %s at %d") %  me->getName() % me->inRoom());
          delete dagger;
          return FALSE;
        }

        buf=format("%s %s %d") % fname(dagger->name) % tmp_ch->name % 5;
        me->doThrow(buf);
        return TRUE;
      }
    }
  }
  return TRUE;
}

int horse(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TBeing *vict;
  int rc;

  if (cmd == CMD_GENERIC_PULSE){
    if (!::number(0,500) && gamePort == Config::Port::PROD) {
      me->setCond(POOP, 24);
      me->doPoop();
    }
  }

  if ((cmd != CMD_MOB_COMBAT) || !me->awake())
    return FALSE;

  if (!(vict = me->fight()))
    return FALSE;

  spellNumT skill = me->getSkillNum(SKILL_KICK);
  if (!me->doesKnowSkill(skill)) {
    me->setSkillValue(skill,me->GetMaxLevel()*2);
  }
  rc = me->doKick("", vict);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete vict;
    vict = NULL;
  }

  return TRUE;
}


int beggar(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *o)
{
  long n;

  if (cmd == CMD_MOB_GIVEN_ITEM)
    me->doSay("Thanks!  I'll pawn that right away for beer money.");
  else if (cmd == CMD_MOB_GIVEN_COINS) {
    n = (long) o;
    if (n < 50)
      me->doSay("Damn.  Don't break the bank, you cheap bastard.");
    else if (n < 250)
      me->doSay("Good... money for booze.");
    else if (n < 1000) {
      me->doSay("Wow.  Thanks.");
      act("$n thanks you.", TRUE, me, NULL, ch, TO_VICT);
      act("$n thanks $N.", TRUE, me, NULL, ch, TO_ROOM);
    } else if (n < 10000) {
      act("$n staggers a bit, utterly amazed.", TRUE, me, NULL, NULL, TO_ROOM);
      me->doSay("I can buy a year's supply of ale now!  Thank you!");
    } else if (n < 100000) {
      act("$n shakes uncontrollably.", TRUE, me, NULL, NULL, TO_ROOM);
      act("$n tries to say something, but is speechless.", TRUE, me, NULL, NULL, TO_ROOM);
    } else {
      act("$n passes out in amazement, $s body hitting the $g with a loud *thud*!", TRUE, me, NULL, NULL, TO_ROOM);
      me->setPosition(POSITION_SLEEPING);
    }
  }
  return FALSE;
}

int petPriceL(int level)
{
  int price;
  if (level <= 5) {                 // 175
    price = (((level)*15) + 100);
  } else if (level <= 12)  {        // 3600
    price = (level*level*25);
  } else if (level <= 17) {         // 10115
    price = (level*level*35);
  } else if (level <= 21) {         // 19845
    price = (level*level*45);
  } else if (level <= 28) {
    price = (level*level*60);
  } else  {
    price = (level*level*90);
  }

  return price;
}

int TMonster::petPrice() const
{
  int price;
  price=petPriceL(GetMaxLevel());

  // they make great tanks
  if (canFly())
    price *= 3;

  return price;
}



static TWindow * getFirstWindowInRoom(TMonster *myself)
{
  TThing *t=NULL;
  TWindow *tw = NULL;
  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end() && (t=*it);++it) {
    tw = dynamic_cast<TWindow *>(t);
    if (tw)
      return tw;
  }
  return NULL;
}

int petVeterinarian(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  TDatabase db(DB_SNEEZY);
  int vnum, num;
  sstring short_desc;
  char new_name[80], buf[80];
  char tmpbuf2[160];
  sstring tmpbuf;


  if(cmd != CMD_LIST &&
     cmd != CMD_BUY)
    return FALSE;

  db.query("select vnum, name, exp, level from pet where player_id=%i order by name", ch->getPlayerID());
    

  if(cmd == CMD_LIST){
    for(int i=1;db.fetchRow();++i){
      if(i==1){
	me->doTell(ch->getName(), "Yeah, your pet dragged himself in here, half dead, and I fixed him up.");
	me->doTell(ch->getName(), "You'll have to pay the bill if you want the cuddly little thing back.");
      }
      
      vnum=convertTo<int>(db["vnum"]);
      short_desc=mob_index[real_mobile(vnum)].short_desc;
      int level=convertTo<int>(db["level"]);

      if(!db["name"].empty()){
	if(short_desc.word(0) != "a" &&
	   short_desc.word(0) != "an")
	  short_desc=format("\"%s\", the %s") % db["name"].cap() % short_desc;
	else
	  short_desc=format("\"%s\", %s") % db["name"].cap() % short_desc;
	me->doTell(ch->getName(), format("%i) %s - %i talens") %
		   i % short_desc % petPriceL(level));
      } else {
	me->doTell(ch->getName(), format("%i) %s - %i talens") %
		   i % short_desc % petPriceL(level));
      }
    }

    return TRUE;
  } else if(cmd == CMD_BUY){
    num=convertTo<int>(arg);

    for(int i=1;db.fetchRow();++i){
      if(i == num)
	break;
    }

    vnum=convertTo<int>(db["vnum"]);
    int level=convertTo<int>(db["level"]);
    affectedData *aff = NULL, *an = NULL;
    char *owner=NULL;

    // search the world for the pet
    for(TBeing *t=character_list;t;t=t->next){
      if(t->mobVnum() == vnum && t->isPet(PETTYPE_PET)){
	// get the owner name
	for (an = t->affected; an; an = an->next) {
	  if (an->type == AFFECT_PET) {
	    aff = an;
	    break;
	  }
	}

	if(aff){
	  owner=(char *)aff->be;
	  
	  if(!strcmp(owner, ch->getName())){
	    // get the pet name
	    sstring short_desc=t->name;
	    sstring name="";
	    if((t->specials.act & ACT_STRINGS_CHANGED)){
	      for(int i=0;!short_desc.word(i).empty();++i){
		name=short_desc.word(i);
	      }
	    }
	    
	    if(name == db["name"]){
	      me->doTell(ch->getName(), "Hmm my mistake, I thought that guy wandered in here but looks like it wandered out again...");
	      return TRUE;
	    }
	  }
	}
      }
    }

    int price=petPriceL(level);

    if(ch->getMoney() < price){
      me->doTell(ch->getName(), "You can't afford it!");
      return TRUE;
    }


    TMonster *pet;    
    if (!(pet = read_mobile(vnum, VIRTUAL))) {
      vlogf(LOG_PROC, "Whoa!  No pet in pet_keeper");
      return TRUE;
    }

    if(!db["name"].empty()){
      pet->swapToStrung();
      
      //  Remake the pet's name.  
      strcpy(new_name, db["name"].c_str());
      tmpbuf = format("%s %s") % pet->name % new_name;
      delete [] pet->name;
      pet->name = mud_str_dup(tmpbuf);
      
      // remake the short desc
      //      sprintf(tmpbuf2, stripColorCodes(pet->getName()).c_str());
      sprintf(tmpbuf2, pet->getName());
      one_argument(tmpbuf2, buf, cElements(buf));
      if (!strcmp(buf, "a") || !strcmp(buf, "an"))
	tmpbuf=format("\"%s\", the %s") % sstring(new_name).cap() %
	  one_argument(tmpbuf2, buf, cElements(buf));
      else
	tmpbuf = format("\"%s\", %s") % sstring(new_name).cap() % pet->getName();
      
      delete [] pet->shortDescr;
      pet->shortDescr = mud_str_dup(tmpbuf);
      
      // remake the long desc
      tmpbuf += " is here.\n\r";
      delete [] pet->player.longDescr;
      pet->player.longDescr = mud_str_dup(tmpbuf);
    }

    pet->balanceMakeNPCLikePC();

    // raiseLevel() aborts if there isn't enough exp, so...
    pet->setExp(getExpClassLevel(pet->bestClass(),level));

    for(int i=pet->GetMaxLevel();i<level;++i)
      pet->raiseLevel(pet->bestClass());
    
    // now set the actual exp
    pet->setExp(convertTo<float>(db["exp"]));
    SET_BIT(pet->specials.affectedBy, AFF_CHARM);

    ch->giveMoney(me, price, GOLD_SHOP_PET);

    *ch->roomp += *pet;
    ch->addFollower(pet);

    affectedData aff2;

    aff2.type = AFFECT_PET;
    aff2.level = 0;
    aff2.duration  = PERMANENT_DURATION;
    aff2.location = APPLY_NONE;
    aff2.modifier = 0;   // to be used for elemental skill level
    aff2.bitvector = 0;

    char * tmp = mud_str_dup(ch->name);
    aff2.be = (TThing *) tmp;

    pet->affectTo(&aff2, -1);
  }

  return FALSE;
}


int pet_keeper(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  char buf[256];
  TRoom *rp;
  int price, shop_nr=find_shop_nr(me->number);
  affectedData aff;

  if (cmd >= MAX_CMD_LIST)
    return FALSE;

  switch (me->mobVnum()) {
    case Mob::PETGUY_GH:
      rp = real_roomp(Room::PETS_GH);
      break;
    case Mob::PETGUY_BM:
      rp = real_roomp(Room::PETS_BM);
      break;
    case Mob::PETGUY_AMB:
      rp = real_roomp(Room::PETS_AMB);
      break;
    case Mob::PETGUY_LOG:
      rp = real_roomp(Room::PETS_LOG);
      break;
    default:
      vlogf(LOG_PROC, "Bogus mob in petguy proc");
      return FALSE;
  }
  if (!rp) {
    vlogf(LOG_PROC, "Pet keeper spec_proc called with no pet room!");
    return FALSE;
  }
  if (cmd == CMD_WHISPER){
    return shopWhisper(ch, me, shop_nr, arg);
  } else if (cmd == CMD_LIST) {
    TWindow *tw = getFirstWindowInRoom(me);

    sstring tellBuf = "Look through the ";
    tellBuf += tw ? fname(tw->name) : "window";
    tellBuf += " to see the pets!";
    me->doTell(fname(ch->name), tellBuf);

    me->doTell(ch->getName(), "If you see something you'd like, VALUE <mob> and I'll tell you the price.");
    return TRUE;
  } else if (cmd == CMD_BUY) {
    arg = one_argument(arg, buf, cElements(buf));
    price = 0;
    
    TBeing *tbt = get_char_room(buf, rp->number);
    TMonster *pet = dynamic_cast<TMonster *>(tbt);
    if (!pet) {
      TWindow *tw = getFirstWindowInRoom(me);

      sstring tellBuf = "Look through the ";
      tellBuf += tw ? fname(tw->name) : "window";
      tellBuf += " again, there is no such pet!";
      me->doTell(fname(ch->name), tellBuf);

      return TRUE;
    }

    if (pet->desc || pet->isPc() || pet->number < 0) {
      me->doTell(fname(ch->name), format("%s is not for sale.") % pet->getName());
      return TRUE;
    }
    int petLevel = pet->GetMaxLevel();
    int pcLevel = ch->GetMaxLevel();

    if (ch->isImmortal()) {
    } else if (!ch->hasClass(CLASS_RANGER)) {
      if ((4 * petLevel) > (3 * pcLevel)) {
        me->doTell(fname(ch->name), "I think I would be negligent if I sold you so powerful a pet.");
        return TRUE;
      }
    } else {
      if (petLevel > pcLevel) {
        me->doTell(fname(ch->name), "I think I would be negligent if I sold you so powerful a pet.");
        return TRUE;
      }
    }
    if (ch->tooManyFollowers(pet, FOL_PET)) {
      me->doTell(fname(ch->name), "With your charisma, it would be animal abuse for me to sell you this pet.");
      return TRUE;
    }

    price = (int)((float) pet->petPrice() * 
		  shop_index[shop_nr].getProfitBuy(NULL, ch));

    if (ch->isPc() && ((ch->getMoney()) < price) && !ch->isImmortal()) {
      me->doTell(ch->name, "You don't have enough money for that pet!");
      return TRUE;
    }
    if (!(pet = read_mobile(pet->number, REAL))) {
      vlogf(LOG_PROC, "Whoa!  No pet in pet_keeper");
      return TRUE;
    }
    if (!ch->isImmortal()){
      ch->giveMoney(me, price, GOLD_SHOP_PET);
      me->saveItems(shop_nr);
      shoplog(shop_nr, ch, me, pet->getName(), price, "buying");
    }
    pet->setExp(0);
    SET_BIT(pet->specials.affectedBy, AFF_CHARM);

    *ch->roomp += *pet;
    ch->addFollower(pet);

//   PET CHANGES
    pet->balanceMakeNPCLikePC();

    sprintf(buf, "May you enjoy your pet, %s", ch->name);
    me->doSay(buf);

    if (ch->desc) {
      ch->desc->career.pets_bought++;
      ch->desc->career.pet_levels_bought += pet->GetMaxLevel();
    }

    aff.type = AFFECT_PET;
    aff.level = 0;
    aff.duration  = PERMANENT_DURATION;
    aff.location = APPLY_NONE;
    aff.modifier = 0;   // to be used for elemental skill level
    aff.bitvector = 0;

    // we will cheat here, and steal the "TThing * be" variable, to store
    // info about our owner.  Obviously, our master might die, so saving
    // a pointer to him would be a bad idea.  Change the data type and
    // store the name instead
    // obviously, since we allocate memory here, we also need some special
    // handling for this situation in the affectedData functions
    char * tmp = mud_str_dup(ch->name);
    aff.be = (TThing *) tmp;

    pet->affectTo(&aff, -1);
    return TRUE;
  } else if (cmd == CMD_VALUE) {
    arg = one_argument(arg, buf, cElements(buf));
    price = 0;
    TBeing *tbt = get_char_room(buf, rp->number);
    TMonster *pet = dynamic_cast<TMonster *>(tbt);

    if (!pet) {
      TWindow *tw = getFirstWindowInRoom(me);

      sstring tellBuf = "Look through the ";
      tellBuf += tw ? fname(tw->name) : "window";
      tellBuf += " again, there is no such pet!";
      me->doTell(fname(ch->name), tellBuf);
      return TRUE;
    }
    int petLevel = pet->GetMaxLevel();
    int pcLevel = ch->GetMaxLevel();

    price = (int)((float) pet->petPrice() * 
		  shop_index[shop_nr].getProfitBuy(NULL, ch));

    me->doTell(ch->name, format("A pet %s will cost %d to purchase.") % fname(pet->name) % price);
    //    me->doTell(ch->name, format("and %d to rent.") % (pet->petPrice() / 4));
    if (ch->isImmortal()) {
    } else if (!ch->hasClass(CLASS_RANGER)) {
      if ((4 * petLevel) > (3 * pcLevel)) {
        me->doTell(fname(ch->name), "I think I would be negligent if I sold you so powerful a pet.");
      }
    } else {
      if (petLevel > pcLevel) {
        me->doTell(fname(ch->name), "I think I would be negligent if I sold you so powerful a pet.");
      }
    }
    if (ch->tooManyFollowers(pet, FOL_PET)) {
      me->doTell(fname(ch->name), "With your charisma, it would be animal abuse for me to sell you this pet.");
    }
    return TRUE;
  }
  return FALSE;
}


int stable_man(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  if (cmd >= MAX_CMD_LIST)
    return FALSE;

  if (cmd == CMD_LIST) {
    TWindow *tw = getFirstWindowInRoom(me);

    sstring tellBuf = "Look through the ";
    tellBuf += tw ? fname(tw->name) : "window";
    tellBuf += " to see the mounts!";
    me->doTell(fname(ch->name), tellBuf);

    return TRUE;
  }
  return FALSE;
}

static int attunePrice(const TSymbol *obj, TBeing *ch, unsigned int shop_nr)
{
  double cost;

  if (!obj->getSymbolMaxStrength())
    cost=max(1, obj->obj_flags.cost / 50);
  else {
    float fract = (float) obj->getSymbolCurStrength() / obj->getSymbolMaxStrength();
    cost=max(1, (int) (obj->obj_flags.cost * fract));
  }

  cost *= shop_index[shop_nr].getProfitBuy(obj, ch);

  return (int) cost;
}

void attune_struct::clearAttuneData()
{
  pc = NULL;
  sym = NULL;
  hasJob = false;
  faction = FACT_UNDEFINED;
  cost = 0;
  wait = 0;
}

static void attuneStructSanityCheck(attune_struct *job)
{
  // this is mostly to validate job->pc and job->sym pointers
  // if what those point at got deleted, no notification is sent to
  // attuner.
  // storing pointers to the actual obj/char is probably not a good
  // idea, we should store name of obj and name of char and just
  // look them up each time instead.
  // not sure of efficiency of this...

  if (job->pc) {
    TBeing *tch;
    for (tch = character_list; tch && tch != job->pc; tch = tch->next);
    if (!tch) {
      // chances are, what job->pc points at is deleted memory, so do NOT
      // reference it
      vlogf(LOG_PROC, "Attuner lost person attuning for.");
      job->clearAttuneData();
    }
  }
  if (job->sym) {
    TObjIter iter;
    for(iter=object_list.begin();
	iter!=object_list.end() && (*iter) != job->sym;++iter);

    if (iter==object_list.end()) {
      // chances are, what job->sym points at is deleted memory, so do NOT
      // reference it
      vlogf(LOG_PROC, "Attuner lost symbol being attuned.");
      job->clearAttuneData();
    }
  }
}

void TThing::attunerValue(TBeing *ch, TMonster *me)
{
  me->doTell(ch->getName(), "I can only attune symbols.");
}

void TSymbol::attunerValue(TBeing *ch, TMonster *me)
{
  int cost;

  if (getSymbolFaction() != FACT_UNDEFINED) {
    me->doTell(ch->getName(), format("%s has already been attuned!") % getName());
    return;
  }
  cost = attunePrice(this, ch, find_shop_nr(me->number));

  me->doTell(ch->getName(), format("I will tithe you %d talens to attune your %s.") % cost % getName());
}

void TThing::attunerGiven(TBeing *ch, TMonster *me)
{
  sstring buf;

  me->doTell(ch->getName(), "I can only attune symbols!");
  buf=format("%s %s") % add_bars(name) % fname(ch->name);
  me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT);
}

void TSymbol::attunerGiven(TBeing *ch, TMonster *me)
{
  int cost;
  char buf[256];
  attune_struct *job;

  if (getSymbolFaction() != FACT_UNDEFINED) {
    me->doTell(ch->getName(), "That symbol has already been attuned!");
    strcpy(buf, name);
    strcpy(buf, add_bars(buf).c_str());
    sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
    me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
    return;
  }
  cost = attunePrice(this, ch, find_shop_nr(me->number));

  if (ch->getMoney() < cost) {
    me->doTell(ch->getName(), "I only attune for a reasonable tithe. I am sorry, I do not make exceptions!");
    strcpy(buf, name);
    strcpy(buf, add_bars(buf).c_str());
    sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
    me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT);
    return;
  }
  // Now we have a symbol to be attuned
  job = (struct attune_struct *) me->act_ptr;

  // sanity check
  attuneStructSanityCheck(job);

  if (!job->wait) {
#if 0
    // blech, this is just needless wait time (boring! boring! boring!)
    // sort of nice idea, but lets not waste person's time.
    job->wait = getSymbolMaxStrength();
    job->wait /= 7;
    job->wait += 1;   // gotta exit with at least 1
#else
    job->wait = 3;
#endif

    sprintf(buf, "Thanks for the donation.  I'll take your %d talen%s tithe in advance!", cost, (cost > 1) ? "s" : "");
    me->doSay(buf);
    ch->giveMoney(me, cost, GOLD_SHOP_SYMBOL);
    shoplog(find_shop_nr(me->number), ch, me, getName(), 
	    cost, "attuning");
    me->saveChar(Room::AUTO_RENT);
    ch->saveChar(Room::AUTO_RENT);


    job->cost = cost;
    job->hasJob = TRUE;
    job->pc = ch;
    job->sym = this;
    vlogf(LOG_SILENT, format("%s gave %s to be attuned.") %  ch->getName() % getName());
    --(*this);
    *me += *this;
//    setSymbolCurStrength(getSymbolMaxStrength());
    job->sym->setSymbolFaction(ch->getFaction());
    me->doRest("");
    act("$n puts a drop of holy water on $p then raises it towards the sky.", TRUE, me, this, NULL, TO_ROOM);
    act("$n carefully places $p on $s lap and begins to pray over it.", TRUE, me, this, NULL, TO_ROOM);
    return;
  } else {
    if (ch == job->pc)
      sprintf(buf, "Sorry, %s, but you'll have to wait while I attune your other symbol.", ch->getName());
    else 
      sprintf(buf, "Sorry, %s, but you'll have to wait while I attune %s's symbol.", ch->getName(), job->pc->getName());

    me->doSay(buf);
    strcpy(buf, name);
    strcpy(buf, add_bars(buf).c_str());
    sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
    me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT);
    return;
  }
  return;
}
int attuner(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  char buf[256];
  TThing *t=NULL;
  TBeing *final_pers;
  TThing *ttt;
  TBeing *tbt = NULL;
  dirTypeT dir = DIR_NONE;
  roomDirData *exitp;
  int rc, found = FALSE;

  attune_struct *job;

  if(cmd == CMD_WHISPER){
    return shopWhisper(ch, me, find_shop_nr(me->number), arg);    
  } else if (cmd == CMD_GENERIC_DESTROYED) {
    delete (attune_struct *) me->act_ptr;
    me->act_ptr = NULL;
    return FALSE;
  } else if (cmd == CMD_GENERIC_INIT) {
    return FALSE;
  } else if (cmd == CMD_GENERIC_CREATED) {
    if (!me->hasClass(CLASS_CLERIC) && !me->hasClass(CLASS_DEIKHAN)) {
      vlogf(LOG_LOW, format("Attuner %s is not a deikhan or cleric.") %  me->getName());
    }
    if (!(me->act_ptr = new attune_struct())) {
      perror("failed new of attuner.");
      exit(0);
    }
    return FALSE;
  }

  if (!(job = (attune_struct *) me->act_ptr)) {
    vlogf(LOG_PROC,format("ATTUNER PROC ERROR/MobPulse: terminating (hopefully) cmd=%d") %  cmd);
    return FALSE;
  }

  // sanity check
  attuneStructSanityCheck(job);

  if (!job->hasJob) {
    if (job->sym || job->pc || job->wait || 
                    job->cost || (job->faction > FACT_UNDEFINED)) {
      if (job->pc && job->pc->name)
        vlogf(LOG_PROC, format("Attuner (%s) seems to have a bad job structure (case 1) see %s.") %  me->getName() % job->pc->getName());
      else
        vlogf(LOG_PROC, format("Attuner (%s) seems to have a bad job structure (case 1A).") %  me->getName());
      job->clearAttuneData();
      return TRUE;
    }
  }
  if (job->hasJob && (!job->pc || !job->sym ||
                  (job->sym && !*job->sym->name) ||
                  (job->pc && !*job->pc->name))) {
    if (job->pc && *job->pc->name) {
      vlogf(LOG_PROC, format("Attuner (%s) seems to have a bad job structure (case 2) see %s.") %  me->getName() % job->pc->getName());
    } else {
      vlogf(LOG_PROC, format("Attuner (%s) seems to have a bad job structure (case 2A).") %  me->getName());
    }
    job->clearAttuneData();
    me->doStand();
    return FALSE;
  }

  if (!(cmd == CMD_MOB_GIVEN_ITEM) && !(cmd == CMD_VALUE)) {
    if (!job->hasJob) {
      if (!me->fight())
        me->doStand();
      return FALSE;
    }
  } 

  switch (cmd) {
    case CMD_MOB_VIOLENCE_PEACEFUL:
      if ((me->getPosition() == POSITION_STANDING) || (me->getPosition() == POSITION_SITTING)) {
        act("$n gets to $s feet and turns towards the hostile parties.", TRUE, me, NULL, NULL, TO_ROOM);
      }
      // cast o down to tthing, and back up to make it a being.
      ttt = o;
      tbt = dynamic_cast<TBeing *>(ttt);
      me->doSay("I am sorry, this is a peaceful spot. You will have to take it elsewhere.");
      for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
        if (exit_ok(exitp = me->exitDir(dir), NULL)) {
          act("$n escorts you out of the vicinity.",
                 FALSE, me, 0, ch, TO_VICT);
          act("$n escorts $N from $s presence.",
                 FALSE, me, 0, ch, TO_NOTVICT);
          me->throwChar(ch, dir, FALSE, SILENT_NO, true);
          act("$n escorts you from $s presence.",
                 FALSE, me, 0, tbt, TO_VICT);
          act("$n escorts $N from $s presence.",
                 FALSE, me, 0, tbt, TO_NOTVICT);
          me->throwChar(tbt, dir, FALSE, SILENT_NO, true);
          return TRUE;
        }
      }
      me->doRest("");
      act("$n breathes deeply then continues $e attuning.", TRUE, me, NULL, NULL, TO_ROOM);
      return TRUE;
    case CMD_GENERIC_PULSE:
      for(StuffIter it=me->stuff.begin();it!=me->stuff.end() && (t=*it);++it) {
        if (t != job->sym)
          continue;
        else {
          found = TRUE;
          break;
        }
      }

      if (!found) {
        me->doSay("Ack, I lost the symbol somehow! Tell a god immediately!");
        vlogf(LOG_PROC, format("Attuner (%s) seems to have lost %s's %s.") %  me->getName() % job->pc->getName() % job->sym->getName());
        job->clearAttuneData();
        me->doStand();
        return FALSE;
      }
      if ((me->getPosition() > POSITION_SITTING) && !me->fight()) {
        me->doRest("");
      }
      if (job->wait > 0) {
        job->wait--;
        if (!job->wait) {
          sprintf(buf, "My prayers are done. Here is your symbol %s!", job->pc->getName());
          me->doSay(buf);

          if (!job->pc->desc || !(me->roomp == job->pc->roomp)) {
            me->doSay("Hmm, I seem to have lost the person I was attuning for.");
            rc = me->doDrop(t->name, NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              job->clearAttuneData();
              me->doStand();
              return DELETE_THIS;
            }
            job->clearAttuneData();
            me->doStand();
            return FALSE;
          }
          final_pers = job->pc;
          if ((final_pers->getCarriedVolume() + t->getTotalVolume())  > final_pers->carryVolumeLimit()) {
             me->doSay("You can't carry this symbol! I'll leave it on the ground for you!");
            rc = me->doDrop(t->name, NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              job->clearAttuneData();
              me->doStand();
              return DELETE_THIS;
            }
            job->clearAttuneData();
            me->doStand();
            return FALSE;
          }
          // final-weight > free final carry weight
          if (compareWeights(t->getTotalWeight(TRUE),
               (final_pers->carryWeightLimit() - final_pers->getCarriedWeight())) == -1) {
            me->doSay("You can't carry this symbol! I'll leave it on the ground for you!");
            rc = me->doDrop(t->name, NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              job->clearAttuneData();
              me->doStand();
              return DELETE_THIS;
            }
            job->clearAttuneData();
            me->doStand();
            return FALSE;
          }
          strcpy(buf, job->sym->name);
          strcpy(buf, add_bars(buf).c_str());
          sprintf(buf + strlen(buf), " %s", fname(job->pc->name).c_str());
          if (me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT) == DELETE_THIS) {
            job->clearAttuneData();
            me->doStand();
            return DELETE_THIS;
          }
          job->clearAttuneData();
          me->doStand();
          return FALSE;
        } else {
          if (!::number(0,2)) {
            act("You can feel $n's prayer in your soul as you watch $m sanctify $p", TRUE, me, t, job->pc, TO_VICT);
           act("You utter a prayer from your soul as you sanctify $p", TRUE, me, t, job->pc,TO_CHAR);
            act("$n sits with $s head bowed concentrating on $p", TRUE, me, t, job->pc, TO_NOTVICT); 
          }
          return FALSE;
        }
      }
      break;
    case CMD_NORTH:
    case CMD_SOUTH:
    case CMD_WEST:
    case CMD_EAST:
    case CMD_UP:
    case CMD_DOWN:
    case CMD_NE:
    case CMD_SW:
    case CMD_SE:
    case CMD_NW:
      if (job->pc == ch) {
        sprintf(buf, "%s! Don't leave until I finish attuning %s!", ch->getName(), job->sym->getName());
        me->doSay(buf);
        return TRUE;
      } else if (job->pc->master) {
        if ((ch == job->pc->master) && (me->roomp == job->pc->roomp) && job->pc->desc && (me->roomp == ch->roomp)) {
          sprintf(buf, "%s! You can't leave, I am still sanctifying %s's %s!", ch->getName(), job->pc->getName(), job->sym->getName());
          me->doSay(buf);
          return TRUE;
        }
        return FALSE;
      }
      break;
    case CMD_VALUE:
      if (!ch->hasClass(CLASS_CLERIC) && !ch->hasClass(CLASS_DEIKHAN)) {
        me->doTell(ch->getName(), "You are not a cleric or Deikhan.  I can not help you.");
        return TRUE;
      }
      for(; *arg && isspace(*arg);arg++);

      if (!(t = searchLinkedListVis(ch, arg, ch->stuff))) {
        me->doTell(ch->getName(), "You don't have that symbol.");
        return TRUE;
      }
      t->attunerValue(ch, me);
      return TRUE;
    case CMD_MOB_GIVEN_ITEM:
      if (!(t = o)) {
        me->doTell(ch->getName(), "You don't have that item!");
        return TRUE;
      }
      if (!ch->hasClass(CLASS_CLERIC) && !ch->hasClass(CLASS_DEIKHAN)) {
        act("You do not accept $N's offer to give you $p.", TRUE, me, t, ch, TO_CHAR);
        act("$n rejects your attempt to give $s $p.", TRUE, me, t, ch, TO_VICT);
        act("$n refuses to accept $p from $N.", TRUE, me, t, ch, TO_NOTVICT); 
        me->doTell(ch->getName(), "You are not a cleric or Deikhan.  I can not help you.");
        strcpy(buf, t->name);
        strcpy(buf, add_bars(buf).c_str());
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      if ((ch != me) && (ch->getPosition() != POSITION_RESTING) &&
            (ch->getPosition() != POSITION_SITTING)) {
        act("You do not accept $N's offer to give you $p.", TRUE, me, t, ch, TO_CHAR);
        act("$n rejects your attempt to give $s $p.", TRUE, me, t, ch, TO_VICT);
        act("$n refuses to accept $p from $N.", TRUE, me, t, ch, TO_NOTVICT);
        me->doTell(ch->getName(), "I can not help you unless you take a position of rest.");
        strcpy(buf, t->name);
        strcpy(buf, add_bars(buf).c_str());
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      me->logItem(t, CMD_EAST);  // log the receipt of the item
      t->attunerGiven(ch, me);
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}

int TThing::sharpenerValueMe(const TBeing *ch, TMonster *me) const
{
  me->doTell(ch->getName(), "I can only value weapons.");
  return TRUE;
}

int TThing::sharpenerGiveMe(TBeing *ch, TMonster *me)
{
  char buf[256];

  me->doTell(ch->getName(), "I can only sharpen weapons!");
  strcpy(buf, name);
  strcpy(buf, add_bars(buf).c_str());
  sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
  me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
  return TRUE;
}

int sharpener(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  char buf[256];
  TObj *weap;
  TThing *final;
  TThing *valued;
  TBeing *final_pers;
  dirTypeT dir;
  roomDirData *exitp;
  int rc;
  TThing *ttt;
  TBeing *tbt;

  sharp_struct *job;

  switch (cmd) {
    case CMD_WHISPER:
      return shopWhisper(ch, me, find_shop_nr(me->number), arg);
    case CMD_GENERIC_DESTROYED:
      delete (sharp_struct *) me->act_ptr;
      me->act_ptr = NULL;
      return FALSE;
    case CMD_GENERIC_CREATED:
      if (!(me->act_ptr = new sharp_struct())) {
        perror("failed new of sharpener.");
        exit(0);
      }
      return FALSE;
    case CMD_MOB_MOVED_INTO_ROOM:

      return kick_mobs_from_shop(me, ch, (int)o);

    case CMD_MOB_VIOLENCE_PEACEFUL:
      ttt = o;
      tbt = dynamic_cast<TBeing *>(ttt);
      me->doSay("Hey!  Take it outside.");
      for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
        if (exit_ok(exitp = me->exitDir(dir), NULL)) {
          act("$n throws you from $s shop.",
                 FALSE, me, 0, ch, TO_VICT);
          act("$n throws $N from $s shop.",
                 FALSE, me, 0, ch, TO_NOTVICT);
          me->throwChar(ch, dir, FALSE, SILENT_NO, true);
          act("$n throws you from $s shop.",
                 FALSE, me, 0, tbt, TO_VICT);
          act("$n throws $N from $s shop.",
                 FALSE, me, 0, tbt, TO_NOTVICT);
          me->throwChar(tbt, dir, FALSE, SILENT_NO, true);
          return TRUE;
        }
      }
      return TRUE;
    case CMD_GENERIC_PULSE:
      job = static_cast<sharp_struct *>(me->act_ptr);
      if (!job) {
        vlogf(LOG_PROC,"SHARPENER PROC ERROR: terminating (hopefully)");
        return FALSE;
      }
      if (!job->char_name || !job->obj_name)
        return FALSE;

      if (job->wait > 0) {
        job->wait--;
        if (!job->wait) {
          sprintf(buf, "Here ya go %s! Good as new.", job->char_name);
          me->doSay(buf);

          if (!(final = searchLinkedList(job->obj_name, me->stuff))) {
            me->doSay("Ack, I lost the weapon somehow! Tell a god immediately!");
            return FALSE;
          } 
          if (!(final_pers = get_char_room(job->char_name, me->in_room))) {
            me->doSay("Hmm, I seem to have lost the person I was sharpening for.");
            rc = me->doDrop(job->obj_name, NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
            return FALSE;
          } 
          if (!final_pers->desc) {
            me->doSay("Hmm, I seem to have lost the person I was sharpening for.");
            rc = me->doDrop(job->obj_name, NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
            return FALSE;
          }
          sprintf(buf, "%s %s", job->obj_name, job->char_name);
          if (me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT) == DELETE_THIS)
            return DELETE_THIS;
          delete [] job->char_name;
          job->char_name = NULL;
          delete [] job->obj_name;
          job->obj_name = NULL;
          job->wait = 0;
          job->cost = 0;
          return FALSE;
        } else {
          if (!(final = searchLinkedList(job->obj_name, me->stuff))) {
            me->doSay("Ack, I lost the weapon somehow! Tell a god immediately!");
            return FALSE;
          } 

          if (!job->isBlunt)
            sprintf(buf, "$n whets $p.");
          else
            sprintf(buf, "$n smoothes $p.");

          act(buf, FALSE, me, final, NULL, TO_ROOM);
          return FALSE;
        }
      }
      break;
    case CMD_NORTH:
    case CMD_SOUTH:
    case CMD_WEST:
    case CMD_EAST:
    case CMD_UP:
    case CMD_DOWN:
    case CMD_NE:
    case CMD_SW:
    case CMD_SE:
    case CMD_NW:
      if (!(job = (sharp_struct *) me->act_ptr)) {
        return FALSE;
      }
      if (!job->char_name) {
        return FALSE;
      }
      if (!strcmp(job->char_name, ch->getName())) {
        sprintf(buf, "%s! Don't leave until I finish with this %s!", ch->getName(), job->obj_name);
        me->doSay(buf);
        return TRUE;
      } else {
        followData *f, *n;
        TBeing *vict;

        for (f = ch->followers; f; f = n) {
          n = f->next;
          if ((vict = f->follower)) {
            if (!strcmp(job->char_name, vict->getName())) {
              sprintf(buf, "%s! You can't leave! %s has a %s being sharpened!",
                ch->getName(), vict->getName(), job->obj_name);
              me->doSay(buf);
              return TRUE;
            }
          }
        }
        return FALSE;
      }
      break;
    case CMD_VALUE:
      for(; *arg && isspace(*arg);arg++);

      if (!(valued = searchLinkedListVis(ch, arg, ch->stuff))) {
        me->doTell(ch->getName(), "You don't have that item.");
        return TRUE;
      }
      return valued->sharpenerValueMe(ch, me);
    case CMD_MOB_GIVEN_ITEM:
      if (!(weap = o)) {
        me->doTell(ch->getName(), "You don't have that item!");
        return TRUE;
      }
      me->logItem(weap, CMD_EAST);  // log the receipt of the item
      return weap->sharpenerGiveMe(ch, me);
    default:
      return FALSE;
  }
  return FALSE;
}

int cold_giver(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  affectedData af;

  if ((cmd != CMD_GENERIC_PULSE) || number(0, 50))
    return FALSE;

  act("$n coughs uncontrollably.", FALSE, me, NULL, NULL, TO_ROOM);
  me->sendTo("You cough uncontrollably.\n\r");
  af.type = AFFECT_DISEASE;
  af.level = 0;
  af.duration = 100;
  af.modifier = DISEASE_COLD;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  spread_affect(me, 250, 0, 0, &af);

  return FALSE;
}

int frostbiter(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  affectedData af;

  if ((cmd != CMD_GENERIC_PULSE) || number(0, 50))
    return FALSE;

  act("A frigid breeze blows past, chilling you to the bone.", 
         FALSE, me, NULL, NULL, TO_ROOM);
  af.type = AFFECT_DISEASE;
  af.level = 0;
  af.duration = 200;
  af.modifier = DISEASE_FROSTBITE;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  spread_affect(me, 500, 0, 0, &af);

  return FALSE;
}

int leper(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  affectedData af;

  if ((cmd != CMD_GENERIC_PULSE) || number(0, 50))
    return FALSE;

  af.type = AFFECT_DISEASE;
  af.level = 0;
  af.duration = PERMANENT_DURATION;
  af.modifier = DISEASE_LEPROSY;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  spread_affect(me, 50, 0, 0, &af);

  return FALSE;
}

int flu_giver(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  affectedData af;

  if ((cmd != CMD_GENERIC_PULSE) || number(0, 50))
    return FALSE;

  act("$n pukes in multiple colors.", FALSE, me, NULL, NULL, TO_ROOM);
  me->sendTo("You puke.\n\r");
  me->dropPool(10, LIQ_VOMIT);
  af.type = AFFECT_DISEASE;
  af.level = 0;
  af.duration = 200;
  af.modifier = DISEASE_FLU;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  spread_affect(me, 100, 0, 0, &af);

  return FALSE;
}


int bogus_mob_proc(TBeing *, cmdTypeT, const char *, TMonster *me, TObj *)
{
  if (me)
    vlogf(LOG_PROC, format("WARNING:  %s is running around with a bogus spec_proc #%d") % 
       me->name % me->spec);
  else
    vlogf(LOG_PROC, "WARNING: indeterminate mob has bogus spec_proc");
  return FALSE;
}


int specificDisc(TBeing *, cmdTypeT, const char *, TMonster *, TObj *)
{
  // this proc sets discipline learnedness, nothing else
  return FALSE;
}

int death(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int rc;

  if ((cmd != CMD_GENERIC_PULSE) || !me)
    return FALSE;
  if (!me->riding) {
    rc = me->findMyHorse();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  } else {
    rc = me->randomHunt();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  // this "kills" folks but doesn't cause exp loss
  TThing *t1;
  for(StuffIter it=me->roomp->stuff.begin();it!=me->roomp->stuff.end();){
    t1=*(it++);
    TBeing * t = dynamic_cast<TBeing *>(t1);
    if (!t)
      continue;
    if (t == me)
      continue;
    if (!t->isPc() && UtilMobProc(t))
      continue;
    if (t->rider)
      continue;
    if (t->isImmortal())
      continue;
    if (number(0,3))  // randomize slain a bit 
      continue;
    // protect newbies 
    if (t->GetMaxLevel() < 8 && me->inGrimhaven())
      continue;
    act("$n's skeletal gaze falls upon $N.",TRUE,me,0,t,TO_NOTVICT);
    if (!t->isPc() || !::number(0,5)) {
      act("$N is dead!  R.I.P.",TRUE,me,0,t,TO_NOTVICT);
      act("$n's skeletal gaze falls upon you!",TRUE,me,0,t,TO_VICT);
      t->rawKill(DAMAGE_NORMAL, me);
      delete t;
      t = NULL;
    } else {
      // let PCs save since "death" causes some complaints
      t->setHit(t->getHit()/2);
    }
    break;
  }
  return FALSE;
}

int war(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TBeing *t = NULL, *next_tar = NULL;
  TBeing *vict;
  int rc;

  if ((cmd != CMD_GENERIC_PULSE) || !me)
    return FALSE;
  if (!me->riding) {
    rc = me->findMyHorse();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;  // delete me
  } else {
    rc = me->randomHunt();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  
  if ((vict = me->fight())) {
    // calls forth great warriors to protect him 
    for (t = character_list;t;t = next_tar) {
      next_tar = t->next;
      if (t->mobVnum() == Mob::Mob::APOC_WARRIOR) {
        if (!t->sameRoom(*me)) {
          --(*t);
          *me->roomp += *t;
          act("$n summons $s minions from across the ether!",0, me, 0, 0, TO_ROOM);
          if (!t->master)
            me->addFollower(t);
          if (t->reconcileDamage(vict,0,DAMAGE_NORMAL) == -1) {
            delete vict;
            vict = NULL;
          }
          return FALSE;
        }
      }
    }
    if (mob_index[real_mobile(Mob::Mob::APOC_WARRIOR)].getNumber() > 5)
      return FALSE;

    act("$n calls forth a great warrior to bring war upon The World!",0, me, 0, 0, TO_ROOM, ANSI_RED);
    t = read_mobile(Mob::Mob::APOC_WARRIOR,VIRTUAL);
    *me->roomp += *t;
    me->addFollower(t);
    t->reconcileDamage(me->fight(),0,DAMAGE_NORMAL);
  }
  // start people in the room fighting each other, randomly 
  // keep War and his horse out of it though 
  if (me->checkPeaceful(""))
    return FALSE;
  TThing *t1, *t2=NULL, *t3=NULL;
  for(StuffIter it=me->roomp->stuff.begin();it!=me->roomp->stuff.end();){
    t1=*(it++);
    t = dynamic_cast<TBeing *>(t1);
    if (!t)
      continue;
    if (t == me)
      continue;
    if (t->rider)
      continue;  
    if (!t->isPc() && UtilMobProc(t))
      continue;
    if (t->fight())
      continue;
    if (t->isImmortal())
      continue;
    // protect newbies 
    if (t->GetMaxLevel() < 8 && me->inGrimhaven())
      continue;
    for (next_tar = NULL;t2;t2 = t3, next_tar = NULL) {
      t2=*(it++);
      next_tar = dynamic_cast<TBeing *>(t2);
      if (!next_tar)
        continue;
      if (next_tar == me)
        continue;
      if (next_tar->isImmortal())
        continue;
      if (next_tar->rider)
        continue;
      if (UtilMobProc(next_tar))
        continue;
      // This is cruel and unusual punishment for low level PCs or
      // players of 'weak stature', such as mages and thieves.  Lets
      // Not make there lives a living hell please.
      if (next_tar->isPc() || next_tar->orig)
        continue;

      // we have a valid target as next
      break;
    }
    if (!next_tar)
      continue;
    t->sendTo("Your blood boils and you feel compelled to fight!\n\r");
    t->reconcileDamage(next_tar,0,DAMAGE_NORMAL);
  }

  return FALSE;
}

void TBeing::findFood()
{
  TThing *t, *t3;

  for(StuffIter it=stuff.begin();it!=stuff.end();){
    t=*(it++);

    for(StuffIter it=t->stuff.begin();it!=t->stuff.end();){
      t3=*(it++);
      t3->nukeFood();
      // t3 may be invalid here
    }

    t->nukeFood();
    // t may be invalid here
  }
}

int famine(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TBeing *t;
  TThing *t1;
  int rc;
  affectedData aff;

  if ((cmd != CMD_GENERIC_PULSE) || !me)
    return FALSE;
  if (!me->riding) {
    rc = me->findMyHorse();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;  // delete me
  } else {
    rc = me->randomHunt();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  if ((t = me->fight())) {
   // drain movement from victim
    act("$n grins as $e drains energy from $N to feed $s own.",TRUE,me,0,t,TO_NOTVICT);
    act("$n grins as $e drains your energy to feed $s own!",TRUE,me,0,t,TO_VICT);
    int num = min(t->getMove(),dice(3,5));
    t->addToMove(-num);
    t->addToHit(2 * num);
    return TRUE;
  }

  for(StuffIter it=me->roomp->stuff.begin();it!=me->roomp->stuff.end();){
    t1=*(it++);
    t = dynamic_cast<TBeing *>(t1);
    if (!t)
      continue;
    if (t->isImmortal())
      continue;
    if (t == me)
      continue;
    if (!t->isPc() && UtilMobProc(t))
      continue;
    if (t->rider)
      continue;
    if (!t->isHumanoid())
       continue;
    switch (number(0, 1)) {
      // put the scurvy on, or fall through & starve them
      case 0:
        // i don't think scurvy is a really a pathogenic disease, but whatever
        if (!t->hasDisease(DISEASE_SCURVY)
            && !t->isImmune(IMMUNE_DISEASE, WEAR_BODY)
            && t->isHumanoid()) {
          aff.type = AFFECT_DISEASE;
          aff.level = 0;
          aff.duration = ::number(500, 1000);
          aff.location = APPLY_NONE;
          aff.bitvector = 0;
          aff.modifier2 = t->GetMaxLevel();
          aff.modifier = DISEASE_SCURVY;
          aff.duration *= (100 - t->getImmunity(IMMUNE_DISEASE));
          aff.duration /= 100;
          if (aff.modifier && aff.duration > 0) {
            act("$N's presence drains your body of something essential.", TRUE, me, 0, t, TO_VICT);
            act("You sap the good parts from $n's blood!", TRUE, me, 0, t, TO_CHAR);
            t->affectTo(&aff);
						disease_start(t, &aff);
            break;
          }
        }
      case 1:
        act("Your throat tightens as $N causes a great thirst!", TRUE, t, 0, me, TO_CHAR);
        act("$N causes you to slowly starve!!!!", TRUE, t, 0, me, TO_CHAR);
        t->setCond(THIRST, 0);
        t->setCond(FULL, 0);
        t->findFood();
        if (t->reconcileDamage(t,number(5,8),DAMAGE_STARVATION) == -1) {
          delete t;
          t = NULL;
        }
        break;
      default:
        // doesn't fall through
        break;
    }
  }
  return FALSE;
}

int pestilence(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  affectedData aff;
  TBeing *t;
  int num;
  int rc;

  if ((cmd != CMD_GENERIC_PULSE) || !me)
    return FALSE;
  if (!me->riding) {
    rc = me->findMyHorse();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  } else {
    rc = me->randomHunt();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  if ((t = me->fight()) 
      && !number(0, 3)
      && !t->isImmune(IMMUNE_DISEASE, WEAR_BODY)
      && t->isHumanoid()) {
    // plagues them
    aff.type = AFFECT_DISEASE;
    aff.level = 0;
    aff.duration = 500;
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    aff.modifier2 = me->GetMaxLevel();
    num = number(1,100);
    if (num <= 30 && !t->hasDisease(DISEASE_COLD))
      aff.modifier = DISEASE_COLD;
    else if (num <= 50 && !t->hasDisease(DISEASE_DYSENTERY))
      aff.modifier = DISEASE_DYSENTERY;
    else if (num <= 65 && !t->hasDisease(DISEASE_FLU))
      aff.modifier = DISEASE_FLU;
    else if (num <= 80 && !t->hasDisease(DISEASE_PNEUMONIA))
      aff.modifier = DISEASE_PNEUMONIA;
    else if (num <= 90 && !t->hasDisease(DISEASE_LEPROSY))
      aff.modifier = DISEASE_LEPROSY;
    else if (!t->hasDisease(DISEASE_PLAGUE))
      aff.modifier = DISEASE_PLAGUE;
    aff.duration *= (100 - t->getImmunity(IMMUNE_DISEASE));
    aff.duration /= 100;
    if (aff.modifier && aff.duration > 0) {
      t->affectTo(&aff);
      disease_start(t, &aff);
    }
  }

  // casts disease on everybody
  TThing *t1;
  for(StuffIter it=me->roomp->stuff.begin();it!=me->roomp->stuff.end();){
    t1=*(it++);
    if (number(0,14))
      continue;
    t = dynamic_cast<TBeing *>(t1);
    if (!t)
      continue;
    if (t->isImmortal())
      continue;
    if (t->isImmune(IMMUNE_DISEASE, WEAR_BODY))
      continue;
    if (t == me)
      continue;
    if (!t->isPc() && UtilMobProc(t))
      continue;
    if (t->rider)
      continue;
    if (!t->isHumanoid())
      continue;
    // protect newbies 
    if (t->GetMaxLevel() < 8 && me->inGrimhaven())
      continue;
    aff.type = AFFECT_DISEASE;
    aff.level = 0;
    aff.duration = 500;
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    aff.modifier2 = t->GetMaxLevel();
    num = number(1,100);
    if (num <= 30 && !t->hasDisease(DISEASE_COLD))
      aff.modifier = DISEASE_COLD;
    else if (num <= 50 && !t->hasDisease(DISEASE_DYSENTERY))
      aff.modifier = DISEASE_DYSENTERY;
    else if (num <= 70 && !t->hasDisease(DISEASE_FLU))
      aff.modifier = DISEASE_FLU;
    else if (num <= 85 && !t->hasDisease(DISEASE_PNEUMONIA))
      aff.modifier = DISEASE_PNEUMONIA;
    else if (num <= 95 && !t->hasDisease(DISEASE_LEPROSY))
      aff.modifier = DISEASE_LEPROSY;
    else if (!t->hasDisease(DISEASE_PLAGUE))
      aff.modifier = DISEASE_PLAGUE;
    aff.duration *= (100 - t->getImmunity(IMMUNE_DISEASE));
    aff.duration /= 100;
		if (aff.modifier && aff.duration > 0) {
      act("$N gazes upon you with lidless white eyes.", TRUE, me, 0, t, TO_VICT);
      act("You gaze upon $n.", TRUE, me, 0, t, TO_CHAR);
      act("$n gasps and exhales a plume of dank vapors.", TRUE, me, 0, 0, TO_ROOM);
      t->affectTo(&aff);
      disease_start(t, &aff);
    }
  }
  return FALSE;
}

static int engraveCost(TObj *obj, TBeing *ch, unsigned int shop_nr)
{
  double cost;

  cost = obj->obj_flags.cost;

  cost *= max(1,obj->obj_flags.cost/10000);

  if(shop_nr)
    cost *= shop_index[shop_nr].getProfitBuy(obj, ch);

  return (int) cost;
}

int engraver(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  char buf[256];
  TObj *item;
  TBeing *final_pers;
  int cost;
  dirTypeT dir = DIR_NONE;
  roomDirData *exitp;
  int rc;
  TBeing *tbt = NULL;
  TThing *ttt;

  class reg_struct {
    public:
    int wait;
    int cost;
    char *char_name;
    char *obj_name;
    reg_struct() :
      wait(0),
      cost(0),
      char_name(NULL),
      obj_name(NULL)
    {
    }
    ~reg_struct() {
      delete [] char_name;
      char_name = NULL;
      delete [] obj_name;
      obj_name = NULL;
    }
  };
  static reg_struct *job;

  switch (cmd) {
    case CMD_WHISPER:
      return shopWhisper(ch, me, find_shop_nr(me->number), arg);
    case CMD_GENERIC_DESTROYED:
      delete (reg_struct *) me->act_ptr;
      me->act_ptr = NULL;
      return FALSE;
    case CMD_GENERIC_CREATED:
      if (!(me->act_ptr = new reg_struct())) {
        perror("failed new of engraver.");
        exit(0);
      }
      return FALSE;
    case CMD_MOB_MOVED_INTO_ROOM:

        return kick_mobs_from_shop(me, ch, (int)o);

    case CMD_MOB_VIOLENCE_PEACEFUL:
      ttt = o;
      tbt = dynamic_cast<TBeing *>(ttt);
      me->doSay("Hey!  Take it outside.");
      for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
        if (exit_ok(exitp = me->exitDir(dir), NULL)) {
          act("$n throws you from $s shop.",
                 FALSE, me, 0, ch, TO_VICT);
          act("$n throws $N from $s shop.",
                 FALSE, me, 0, ch, TO_NOTVICT);
          me->throwChar(ch, dir, FALSE, SILENT_NO, true);
          act("$n throws you from $s shop.",
                 FALSE, me, 0, tbt, TO_VICT);
          act("$n throws $N from $s shop.",
                 FALSE, me, 0, tbt, TO_NOTVICT);
          me->throwChar(tbt, dir, FALSE, SILENT_NO, true);
          return TRUE;
        }
      }
      return TRUE;
    case CMD_GENERIC_PULSE:
      if (!(job = (reg_struct *) me->act_ptr)) {
        vlogf(LOG_PROC,"ENGRAVER PROC ERROR: terminating (hopefully)");
        return FALSE;   
      }
      if (!job->char_name || !job->obj_name)
        return FALSE;

      if (job->wait > 0) {
        job->wait--;
        if (!job->wait) {
          sprintf(buf, "That should do it %s!", job->char_name);
          me->doSay(buf);
          TThing *ts = NULL;
          TObj *final = NULL;
          if (!(ts = searchLinkedList(job->obj_name, me->stuff)) ||
              !(final = dynamic_cast<TObj *>(ts))) {
            me->doSay("Ack, I lost the item somehow! Tell a god immediately!  ");
            vlogf(LOG_PROC,format("engraver lost his engraving item (%s)") % final->name);
            return FALSE;
          }
          final->swapToStrung();

          //  Remake the obj name.  
          sprintf(buf, "%s %s", final->name, job->char_name);
          delete [] final->name;
          final->name = mud_str_dup(buf);

          sprintf(buf, "This is the personalized object of %s", job->char_name);
          delete [] final->action_description;
          final->action_description = mud_str_dup(buf);

          if (!(final_pers = get_char_room(job->char_name, me->in_room))) {
            me->doSay("Hmm, I seem to have lost the person I was engraving for.");
            rc = me->doDrop(job->obj_name, NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
            return FALSE;
          }
          if ((final_pers->getCarriedVolume() + final->getTotalVolume())  > final_pers->carryVolumeLimit()) {
            me->doSay("You can't carry it! I'll just drop it here for you!");
            rc = me->doDrop(job->obj_name, NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
            delete [] job->char_name;
            job->char_name = NULL;
            delete [] job->obj_name;
            job->obj_name = NULL;
            job->cost = 0;
            return FALSE;
          }
          // final-weight > free final carry weight
          if (compareWeights(final->getTotalWeight(TRUE),
                (final_pers->carryWeightLimit() - final_pers->getCarriedWeight())) == -1) {
            me->doSay("You can't carry it! I'll just drop it here for you!");
            rc = me->doDrop(job->obj_name, NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
            delete [] job->char_name;
            job->char_name = NULL;
            delete [] job->obj_name;
            job->obj_name = NULL;

            job->cost = 0;
            return FALSE;
          }
          sprintf(buf, "%s %s", job->obj_name, job->char_name);
          if (me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT) == DELETE_THIS)
            return DELETE_THIS;
          delete [] job->char_name;
          job->char_name = NULL;
          delete [] job->obj_name;
          job->obj_name = NULL;

          job->cost = 0;
          return FALSE;
        } else {
          sprintf(buf, "$n etches part of %s's %s.", job->char_name, job->obj_name);
          act(buf, FALSE, me, NULL, ch, TO_ROOM);
          return FALSE;
        }
      }
      break;
    case CMD_NORTH:
    case CMD_SOUTH:
    case CMD_WEST:
    case CMD_EAST:
    case CMD_UP:
    case CMD_DOWN:
    case CMD_NE:
    case CMD_SW:
    case CMD_SE:
    case CMD_NW:
      if (!(job = (reg_struct *) me->act_ptr)) {
        return FALSE;
      }
      if (!job->char_name) {
        return FALSE;
      }
      if (!strcmp(job->char_name, ch->getName())) {
        sprintf(buf, "%s! Don't leave until I finish with this %s!", ch->getName(), job->obj_name);
        me->doSay(buf);
        return TRUE;
      } else
        return FALSE;
      break;
    case CMD_VALUE: {
      for(; *arg && isspace(*arg);arg++);
      TThing *ts = NULL;
      TObj *valued = NULL;
      if (!(ts = searchLinkedListVis(ch, arg, ch->stuff)) ||
          !(valued = dynamic_cast<TObj *>(ts))) {
        me->doTell(ch->getName(), "You don't have that item.");
        return TRUE;
      }

      if (valued->engraveMe(ch, me, false))
        return TRUE;

      if (valued->obj_flags.cost <=  500) {
        me->doTell(ch->getName(), "This item is too cheap to be engraved.");
        return TRUE;
      }
      if (valued->action_description) {
        me->doTell(ch->getName(), "This item has already been engraved!");
        return TRUE;
      }
      if (obj_index[valued->getItemIndex()].max_exist <= 10) {
        me->doTell(ch->getName(), "I refuse to engrave such an artifact of beauty!");
        return TRUE;
      }
      if (valued->obj_flags.decay_time >= 0) {
        me->doTell(ch->getName(), "Sorry, but this item won't last long enough to bother with an engraving!");
        return TRUE;
      }

      cost = engraveCost(valued, ch, find_shop_nr(me->number));

      me->doTell(ch->getName(), format("It will cost %d talens to engrave your %s.") % cost % fname(valued->name));
      return TRUE;
      }
    case CMD_MOB_GIVEN_ITEM:
      // prohibit polys and charms from engraving 
      if (dynamic_cast<TMonster *>(ch)) {
        me->doTell(fname(ch->name), "I don't engrave for beasts.");
        return TRUE;
      }
      if (!(item = o)) {
        me->doTell(ch->getName(), "You don't have that item!");
        return TRUE;
      }
      me->logItem(item, CMD_EAST);  // log the receipt of the item

      if (item->engraveMe(ch, me, true))
        return TRUE;

      if (item->obj_flags.cost <= 500) {
        me->doTell(ch->getName(), "That can't be engraved!");
        strcpy(buf, item->name);
        strcpy(buf, add_bars(buf).c_str());
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      if (item->action_description) {
        me->doTell(ch->getName(), "Sorry, but this item has already been engraved!");
        strcpy(buf, item->name);
        strcpy(buf, add_bars(buf).c_str());
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      if (obj_index[item->getItemIndex()].max_exist <= 10) {
        me->doTell(ch->getName(), "This artifact is too powerful to be engraved!");
        strcpy(buf, item->name);
        strcpy(buf, add_bars(buf).c_str());
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      if (item->obj_flags.decay_time >= 0) {
        me->doTell(ch->getName(), "This won't be around long enough to bother engraving it!");
        strcpy(buf, item->name);
        strcpy(buf, add_bars(buf).c_str());
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }

      cost = engraveCost(item, ch, find_shop_nr(me->number));

      if (ch->getMoney() < cost) {
        me->doTell(ch->getName(), "I have to make a living! If you don't have the money, I don't do the work!");
        strcpy(buf, item->name);
        strcpy(buf, add_bars(buf).c_str());
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      job = (reg_struct *) me->act_ptr;
      if (!job->wait || !job->char_name) {
        job->wait = max(1, (int) (item->obj_flags.max_struct_points)/7);
        sprintf(buf, "Thanks for your business, I'll take your %d talens payment in advance!", cost);
        me->doSay(buf);

	TShopOwned tso(find_shop_nr(me->number), me, ch);
	tso.doBuyTransaction(cost, format("engraving %s") % item->getName(), 
			     TX_BUYING_SERVICE);

        job->cost = cost;
        job->char_name = new char[strlen(ch->getName()) + 1];
        strcpy(job->char_name, ch->getName());
        job->obj_name = new char[fname(item->name).length() + 1];
        strcpy(job->obj_name, fname(item->name).c_str());
        --(*item);
        *me += *item; 

	me->saveChar(Room::AUTO_RENT);
	ch->saveChar(Room::AUTO_RENT);

        return TRUE;
      } else {
        sprintf(buf, "Sorry, %s, but you'll have to wait while I engrave %s's item.", ch->getName(), job->char_name);
        me->doSay(buf);
        strcpy(buf, item->name);
        strcpy(buf, add_bars(buf).c_str());
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      return FALSE;
    default:
      return FALSE;
  }
  return FALSE;
}

int fireMaster(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  char buf[256];

  if (cmd != CMD_ENTER)
    return FALSE;

  one_argument(arg, buf, cElements(buf));
  if (!isname(buf, obj_index[real_object(Obj::FLAMING_PORTAL)].name))
    return FALSE;

  if (ch != me) {
    me->doSay("Oh no you don't!");
    me->doSay("You're not going anywhere until you get past me!");
    return TRUE;
  }
  return FALSE;
}

int TicketGuy(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  char obj_name[MAX_INPUT_LENGTH];
  const int TICKET_PRICE = 1000;

  if (!ch || !me) {
    vlogf(LOG_PROC,"NULL ch's in TicketGuy");
    return FALSE;
  }
  if (cmd != CMD_BUY)
    return FALSE;

  if (ch->getPosition() > POSITION_STANDING) {
    me->doTell(fname(ch->name), "I won't sell you a ticket unless you stand on your own feet.");
    return TRUE;
  }

  arg = one_argument(arg, obj_name, cElements(obj_name));

  if (!*obj_name || strcmp(obj_name,"ticket")) {
    me->doTell(fname(ch->name), "Buy what?!?");
    return TRUE;
  }
  // prohibit polys and charms from engraving 
  if (dynamic_cast<TMonster *>(ch)) {
    me->doTell(fname(ch->name), "I don't sell tickets to beasts.");
    return TRUE;
  }
  if (ch->getMoney() < TICKET_PRICE) {
    me->doTell(fname(ch->name), format("Tickets cost %d talens.") % TICKET_PRICE);
    return TRUE;
  }
  ch->addToMoney(-TICKET_PRICE, GOLD_HOSPITAL);
  ch->sendTo("You buy a ticket.\n\r");
  ch->sendTo("The mage makes a strange gesture before you.\n\r");
  ch->sendTo("      *BLICK*\n\r");
  ch->sendTo("Suddenly you find yourself in another plane of existence.\n\r");
  act("$n purchases a ticket and $N transports $m into another plane.",FALSE,ch,0,me,TO_ROOM);
  --(*ch);
  thing_to_room(ch,Room::TICKET_DESTINATION);
  ch->doLook("", CMD_LOOK);
  act("$n blicks into the room.",TRUE,ch,0,0,TO_ROOM);
  return TRUE;
}


int hobbitEmissary(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  class hunt_struct {
    public:
    int cur_pos;
    int cur_path;
    char *hunted_victim;

    hunt_struct() {
      hunted_victim = NULL;
    }
    ~hunt_struct() {
      delete [] hunted_victim;
    }
  };
  hunt_struct *job;

  TBeing *targ;

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<hunt_struct *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }

  if (cmd != CMD_GENERIC_PULSE) 
    return FALSE;

  if (::number(0,2))
    return FALSE;

  if (!myself->awake() || myself->fight())
    return FALSE;

  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new hunt_struct())) {
      perror("failed new of hobbit emissary.");
      exit(0);
    }
    job = static_cast<hunt_struct *>(myself->act_ptr);
    job->hunted_victim = NULL;
    job->cur_pos = 0;
    job->cur_path = 0;
  }

  if (!(job = static_cast<hunt_struct *>(myself->act_ptr))) {
    vlogf(LOG_PROC, "Unable to allocate memory for hobbit emissary!  This is bad!");
    return TRUE;
  }

  if(job->hunted_victim == NULL){
    delete [] job->hunted_victim;
    job->hunted_victim = mud_str_dup("ambassador hobbit Grimhaven");
  }

  targ = get_char_room(job->hunted_victim, EXACT_YES);

  // found target
  if (targ) {
    if (!strcmp(job->hunted_victim, "ambassador hobbit Grimhaven")) {
      myself->doSay(format("Good %s, your excellency.") % describeTime());
      targ->doSay(format("Good %s.") % describeTime());
      myself->doSay("I have a message from his lordship.");
      myself->doWhisper(format("%s sweet nothings") % fname(targ->name));
      targ->doSay("Hmm, that is useful news.  Relay this message back for me.");
      targ->doWhisper(format("%s sweet nothings") % fname(myself->name));
      targ->doSay("Hurry off with that and report back soonest.");
      act("$n salutes the ambassador.",0, myself, 0, 0, TO_ROOM);
      
      delete [] job->hunted_victim;
      job->hunted_victim = mud_str_dup("king Grimhaven");
      job->cur_path = 1;
      job->cur_pos = 0;
    } else if (!strcmp(job->hunted_victim, "king Grimhaven")) {
      myself->doSay(format("Good %s, your lordship.") % describeTime());
      targ->doSay(format("Good %s.") % describeTime());
      myself->doSay("I have a message from his excellency.");
      myself->doWhisper(format("%s sweet nothings") % fname(targ->name));
      targ->doSay("Hmm, that is useful news.  Relay this message back for me.");
      targ->doWhisper(format("%s sweet nothings") % fname(myself->name));
      targ->doSay("Hurry off with that and tell me his response.");
      act("$n salutes the King.",0, myself, 0, 0, TO_ROOM);
      
      delete [] job->hunted_victim;
      job->hunted_victim = mud_str_dup("ambassador hobbit Grimhaven");
      job->cur_path = 0;
      job->cur_pos = 0;
    } else {
      vlogf(LOG_PROC,format("Error: hobbit emissary hunted undefined target. (%s)") % 
	    job->hunted_victim);
      delete [] job->hunted_victim;
      job->hunted_victim = NULL;
    }
    return TRUE;
  }

  switch(myself->walk_path(hobbit_path_pos[job->cur_path], job->cur_pos)){
    case WALK_PATH_END:
      if (job->cur_path == 0) {
	job->cur_path = 1;
	job->cur_pos = 0;
	delete [] job->hunted_victim;
	job->hunted_victim = mud_str_dup("ambassador hobbit Grimhaven");
      } else {
	job->cur_path = 0;
	job->cur_pos = 0;
	delete [] job->hunted_victim;
	job->hunted_victim = mud_str_dup("king Grimhaven");
      }
      return TRUE;
      break;
    case WALK_PATH_LOST:
      act("$n seems to have gotten a little bit lost.",
	  0, myself, 0, 0, TO_ROOM);
      act("$n goes to ask directions.", 
	  0, myself, 0, 0, TO_ROOM);
      if (myself->riding)
	myself->dismount(POSITION_STANDING);
      --(*myself);
      thing_to_room(myself, hobbit_path_pos[job->cur_path][0].cur_room);
      act("$n has arrived.", 0, myself, 0, 0, TO_ROOM);
      return TRUE;
      break;
    case WALK_PATH_MOVED:
      break;
  }

  return 0;
}

int banshee(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (::number(0,4))
    return FALSE;

  act("You unleash a hideous screech.", FALSE, myself, 0, 0, TO_CHAR);
  act("$n unleashes a hideous screech.", FALSE, myself, 0, 0, TO_ROOM);

  TThing *t=NULL;
  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end() && (t=*it);++it) {
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (!tbt)
      continue;
    if (tbt == myself)
      continue;
    if (tbt->isImmortal())
      continue;
    TObj *muff = dynamic_cast<TObj *>(tbt->equipment[WEAR_HEAD]);
    if (muff && muff->objVnum() == Obj::EARMUFF)
      continue;

    act("The scream rocks you to your core and you feel more aged.", FALSE, tbt, 0, 0, TO_CHAR);
    tbt->age_mod += 1;
  }

  return FALSE;
}

int corpseMuncher(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TThing *t;
  TObj *obj = NULL;
  int found=0, msg;
  const char *munch[]={
      "$n stops eating, looks up at you and burps loudly, then resumes feasting on $p.",
      "<r>Blood<1> spatters about as $n bites deeply into $p.",
      "$n utters a low growl as $e rips a chunk of flesh off $p.",
      "$n stops to spit out a piece of <W>bone<1>, then resumes eating $p."};


  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake() || ch->fight())
    return FALSE;

  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end();){
    t=*(it++);
    if (!(obj = dynamic_cast<TObj *>(t)) ||
        !myself->canSee(obj) || !dynamic_cast<TBaseCorpse *>(obj) ||
	obj->obj_flags.decay_time<=0)
      continue;

    found=1;

    if(!myself->isAffected(AFF_MUNCHING_CORPSE)){
      act("$n kneels down and begins munching on $p.", 
             TRUE, myself, obj, 0, TO_ROOM);
      myself->dropPool(9, LIQ_BLOOD);   
      SET_BIT(myself->specials.affectedBy, AFF_MUNCHING_CORPSE);
      SET_BIT(myself->specials.act, ACT_SENTINEL);
    } else {
      if((msg=::number(0,10))<4){
	act(munch[msg], TRUE, myself, obj, 0, TO_ROOM);
	myself->dropPool(3, LIQ_BLOOD);

        // don't decay player corpses
        TPCorpse *tmpcorpse = dynamic_cast<TPCorpse *>(obj);
        if (!tmpcorpse)
          obj->decayMe();
      }
    }

    break;
  }
  
  if(!found && myself->isAffected(AFF_MUNCHING_CORPSE)){
    act("$n swallows down the last few pieces of flesh and looks around for another corpse to eat.",
      TRUE, myself, 0, 0, TO_ROOM);
    REMOVE_BIT(myself->specials.affectedBy, AFF_MUNCHING_CORPSE);
    REMOVE_BIT(myself->specials.act, ACT_SENTINEL);
  }

  return FALSE;
}

int fishTracker(TBeing *ch, cmdTypeT cmd, const char *argument, TMonster *myself, TObj *o)
{
  sstring buf, arg=argument;
  //TThing *t;
  TDatabase db(DB_SNEEZY);

  if(!ch || !ch->awake() || ch->fight())
    return FALSE;

  
  switch(cmd){
    case CMD_MOB_GIVEN_ITEM:
      if(!o || !isname("caughtfish", o->name)){
        return FALSE;
      }
      
      // update total weight caught for player
      db.query("update fishkeeper set weight=weight+%f where name='%s'", o->getWeight(), ch->name);
      if (db.rowCount() == 0) {
        // probably no row for user (first fish!) so try an insert instead
        db.query("insert into fishkeeper values ('%s', %f)", ch->name, o->getWeight());
      }

      // check for record
      db.query("update fishlargest set name = '%s', weight = %f where vnum = %i and weight < %f", 
          ch->getName(), o->getWeight(), o->objVnum(), o->getWeight());

      if (db.rowCount() > 0) {
        myself->doSay(format("Oh my, you've broken the record for %s!") % o->shortDescr);
        buf=format("This the largest I've seen, weighing in at %i!  Very nice! (%i talens)") 
            % (int)o->getWeight() % (int)(o->getWeight()*100);
        myself->doSay(buf);
        ch->addToMoney((int)(o->getWeight()*100), GOLD_COMM);	
      } else {
        buf=format("Ok, I tallied your fish, weighing in at %i.  Nice one! (%i talens)") %
            (int)o->getWeight() % (int)(o->getWeight()*2);
        myself->doSay(buf);
        ch->addToMoney((int)(o->getWeight()*2), GOLD_COMM);
      }

      /*
      -- 08/04/2004 -- Was causing crashes and corruption, changed to a standard 'return DELETE_ITEM' to fix. -Lapsos

      // heh why'd I do this instead of returning DELETE_ ??
      // beats me, I ain't gonna touch it now though
      for(StuffIter it=myself->stuff.begin();it!=myself->stuff.end() && (t=*it);++it){
	if(isname("caughtfish", t->name)){
	  delete t;
	  break;
	}
      }
      */
      // ch already gets saved from the give command
      // ch->doSave(SILENT_YES);

      return DELETE_ITEM;

      break;

    case CMD_ASK:
    case CMD_WHISPER:
      arg = one_argument(arg, buf);
      
      if(!isname(buf, myself->name))
        return FALSE;

      arg = one_argument(arg, buf);

      if(buf=="records"){
        db.query("select f1.name, o1.short_desc as type, f1.weight from fishlargest f1 join obj o1 on f1.vnum = o1.vnum where f1.weight > 0 order by f1.weight desc");

      	while(db.fetchRow()){
      	  buf=format("%s caught %s weighing in at %i.") 
      	      % db["name"] % db["type"] % (int)(convertTo<float>(db["weight"]));
      	  myself->doSay(buf);
      	}

      } else if (buf == "score") {
        int iTotCount = 0, iPerCount = 0;

        db.query("select f1.name, o1.short_desc as type, f1.weight from fishlargest f1 join obj o1 on f1.vnum = o1.vnum order by f1.weight desc");

        while (db.fetchRow()) {
          if (db["name"] == ch->getName()) {
            buf = format("You caught %s weighing in at %i.") % db["type"] % (int)(convertTo<float>(db["weight"]));
            myself->doSay(buf);
            iPerCount++;
          }

          iTotCount++;
        }

        if (!iPerCount)
          myself->doSay("You do not hold any records at this time, go out there and land the big one that got away!");
        else {
          buf = format("Out of %d records you hold %d.") % iTotCount % iPerCount;
          myself->doSay(buf);
        }

      } else {
        sstring weight="0";
        bool topten=false;
        if(buf=="topten"){
          db.query("select o.name, o.weight, count(l.name) as count from fishkeeper o left join fishlargest l on o.name=l.name group by o.name, o.weight order by weight desc limit 10");
          topten=true;
        } else {
          db.query("select o.name, o.weight, count(l.name) as count from fishkeeper o left join fishlargest l on o.name=l.name where o.name='%s' group by o.name, o.weight order by weight desc limit 10", buf.c_str());
        }
	
        while(db.fetchRow()){
          if(topten){
            weight=talenDisplay((int)(convertTo<float>(db["weight"])));
          } else {
            weight=format("%i") % (int)(convertTo<float>(db["weight"]));
          }

          buf=format("%s has %s pounds of fish and %i records.") 
              % db["name"] % weight % convertTo<int>(db["count"]);
          myself->doSay(buf);
        }      
      }

      break;
    default:
      return FALSE;
  }

  return TRUE;
}


int grimhavenHooker(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int found=0;
  dirTypeT dir=DIR_NONE;
  const int nhomes=4, homes[nhomes]={340, 339, 338, 337};
  TThing *t=NULL;
  TMonster *tmons=NULL;
  sstring hookername, johnname, tmp;
  TPathFinder path;

   enum hookerStateT {
    STATE_NONE,
    STATE_OFFER,      // 1
    STATE_REJECT1,    // 2
    STATE_ASKPRICE,   // 2
    STATE_TELLPRICE,  // 3
    STATE_REJECT2,    // 4
    STATE_ACCEPT,
    STATE_WALKING,    // 4
    STATE_WAITFORCLEAR
  };

  class hunt_struct {
    public:
      int cur_pos;
      int cur_path;
      hookerStateT state;
      TMonster *john;
  } *job;

  if (cmd == CMD_GENERIC_DESTROYED) {
    if(myself->act_ptr){
      delete static_cast<hunt_struct *>(myself->act_ptr);
      myself->act_ptr = NULL;
    }
    return FALSE;
  } else  if ((cmd != CMD_GENERIC_PULSE) ||
	      !myself->awake() || myself->fight())
    return FALSE;

  if (!myself->act_ptr) {
    if(::number(0,10))
      return FALSE;

    // find a john
    for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end() && (t=*it);++it){
      if((tmons=dynamic_cast<TMonster *>(t))){
	if(tmons!=myself && tmons->isHumanoid() && !tmons->isShopkeeper() &&
      !IS_SET(tmons->specials.act, ACT_SENTINEL) &&
	   (tmons->getSex()!=SEX_NEUTER && tmons->getSex()!=myself->getSex())){
	  found=1;
	  break;
	}
      }
    }
    
    // didn't find one
    if(!found)
      return FALSE;

    if (!(myself->act_ptr = new hunt_struct())) {
      vlogf(LOG_PROC, "failed memory allocation in mob proc grimhavenHooker.");
      return FALSE;
    }
    job = static_cast<hunt_struct *>(myself->act_ptr);
    job->cur_pos = 0;
    job->cur_path=::number(0, nhomes-1);
    job->john=tmons;
    job->state=STATE_OFFER;
  } else {
    if(::number(0,4))
      return FALSE;
  }
  
  if (!(job = static_cast<hunt_struct *>(myself->act_ptr))) {
    vlogf(LOG_PROC, "grimhavenHooker: error, static_cast");
    return FALSE;
  }
  
  if(job->state==STATE_NONE){
    //    vlogf(LOG_PROC, "STATE_NONE, deleting act_ptr");
    delete static_cast<hunt_struct *>(myself->act_ptr);
    myself->act_ptr=NULL;
    return FALSE;
  }


  // Make sure our john is still alive and in the same room
  found=0;
  if(job->john){
    if(!myself->sameRoom(*job->john)){
      myself->doAction("", CMD_FROWN);
      job->john=NULL;
      job->state=STATE_NONE;
      return TRUE;	
    }
  } else {
    job->state=STATE_NONE;
    return FALSE;
  }
    	  
  hookername=myself->name;
  hookername=add_bars(hookername);
  johnname=job->john->name;
  johnname=add_bars(johnname);

  switch(job->state){
    case STATE_NONE:
      break;
    case STATE_OFFER:
      myself->doAction(johnname, CMD_PEER);
      switch(::number(0,3)){
	case 0: 
	  myself->doSay("You want to party, honey?"); 
	  myself->doAction("", CMD_LICK);
	  break;
	case 1:
	  myself->doSay("How about some action baby?"); 
	  myself->doAction(johnname, CMD_NUZZLE);
	  break;
	case 2:
	  if(job->john->getSex() == SEX_MALE)
	    myself->doSay("Is that a training dagger in your pants, or do you need some hot lovin'?"); 
	  else 
	    myself->doSay("Did you just come from a sushi bar, or do you need some hot lovin'?");
	  myself->doAction(johnname, CMD_GRIN);
	  break;
	case 3:
	  myself->doAction("", CMD_WIGGLE);
	  myself->doSay("Me so horny, me love you long time!");
	  myself->doAction("", CMD_LICK);
	  break;
      }

      if(::number(0,2))
	job->state=STATE_REJECT1;
      else
	job->state=STATE_ASKPRICE;
      break;
    case STATE_REJECT1:
      switch(::number(0,4)){
	case 0:
	  job->john->doAction("", CMD_SNICKER);
	  job->john->doSay("Yeah, right.  As if.");
	  break;
	case 1:
	  job->john->doAction("", CMD_SNEER);
	  job->john->doSay("Get the hell away from me you skank ghetto whore!");
	  break;
	case 2:
	  job->john->doAction("", CMD_BLUSH);
	  job->john->doSay("Um, no thanks.");
	  break;
	case 3:
	  job->john->doSay("Not me sexy, why pay when I get it for free in the pasture?");
	  job->john->doAction("", CMD_CHUCKLE);
	  break;
	case 4:
	  job->john->doSay("Wait, is that my wife I see down the street?!?");
	  job->john->doSay("I'm out of here!");
	  job->john->doFlee("");
	  break;
      }
      job->john=NULL;
      job->state=STATE_NONE;
      break;
    case STATE_ASKPRICE:
      switch(::number(0,4)){
	case 0:
	  job->john->doSay("Yeah baby, I want some real rough love.");
	  job->john->doAction(hookername, CMD_TAUNT);
	  job->john->doSay("What's it gonna cost me to tie that little bad ass up?");
	  job->john->doAction(hookername, CMD_GRIN);
	  break;
	case 1:
	  job->john->doScan("");
	  job->john->doSay("Well... Ok, but it has to be secret.  How much?");
	  break;
	case 2:
	  job->john->doAction("", CMD_MOAN);
	  job->john->doSay("Oh god, I want you so bad, I'll pay anything you want!");
	  job->john->doAction(johnname, CMD_GROPE); 
	  break;
	case 3:
	  job->john->doSay("I've had a rough day, I could certainly use a little winding down.");
	  job->john->doSay("Let's get it on!");
	  job->john->doAction("", CMD_CLAP);
	  job->john->doAction("", CMD_GIGGLE);	  
	  break;
	case 4:
	  tmp = format("%s Do you swallow?") % hookername;
	  job->john->doWhisper(tmp);
	  myself->doEmote("looks startled and almost chokes.");
	  myself->doAction(johnname, CMD_SLAP);
	  job->john->doAction(hookername, CMD_GRIN);
	  myself->doAction("", CMD_GIGGLE);
	  break;
      }
      job->state=STATE_TELLPRICE;
      break;
    case STATE_TELLPRICE:
      switch(::number(0,2)){
	case 0:
	  myself->doSay("It will cost you...");
	  myself->doEmote("grins evilly revealing pointed teeth and her eyes turn <r>red<1>!");
	  myself->doSay("...YOUR SOUL!");
	  myself->doEmote("suddenly returns to normal.");
	  myself->doAction("", CMD_GIGGLE);
	  myself->doSay("Just joking.");
	  myself->doAction(johnname, CMD_BECKON);
	  break;
	case 1:
	  myself->doAction(johnname, CMD_PONDER);
	  myself->doSay("Maybe we can work out a payment plan or something.");
	  break;
	case 2:
	  myself->doSay("Don't worry sailor, I'm sure you can afford it.");
	  myself->doAction(johnname, CMD_PET);
	  break;
	case 3:
	  myself->doSay("For the service I provide, the cost is quite reasonable, I assure you.");
	  myself->doAction(johnname, CMD_MASSAGE);
	  break;
      }
      
      if(job && job->john && IS_SET(job->john->specials.act, ACT_SENTINEL)){
	job->state=STATE_REJECT2;
      } else {
	if(::number(0,1))
	  job->state=STATE_REJECT2;
	else
	  job->state=STATE_ACCEPT;
      }
      break;
    case STATE_REJECT2:
      switch(::number(0,3)){
	case 0:
	  job->john->doSay("Hell, for that price I could just get married.");
	  job->john->doAction("", CMD_CACKLE);
	  break;
	case 1:
	  job->john->doAction("", CMD_SMIRK);
	  job->john->doSay("The pet store has cheaper prices.");
	  job->john->doSay("Er, I mean, um.");
	  job->john->doEmote("turns away quickly.");
	  break;
	case 2:
	  job->john->doSay("How about I mail you a check?  No?  Damn.");
	  job->john->doAction("", CMD_SULK);
	  break;
	case 3:
	  job->john->doSay("Wait, is that my wife I see down the street?!?");
	  job->john->doSay("I'm out of here!");
	  job->john->doFlee("");
	  break;
      }
      job->john=NULL;
      job->state=STATE_NONE;
      break;
    case STATE_ACCEPT:
      switch(::number(0,3)){
	case 0:
	  job->john->doSay("Excellent.  You take Grimhaven Express right?");
	  break;
	case 1:
	  job->john->doSay("Hm, that's almost a week's pay.");
	  job->john->doAction("head", CMD_SCRATCH);
	  job->john->doSay("Deal!");
	  break;
	case 2:
	  job->john->doAction("", CMD_DROOL);
	  job->john->doSay("Me Tarzan, you cheap whore.");
	  break;
	case 3:
	  job->john->doSay("Alright, but you better not have crabs or anything.");
	  break;
      }
      if(!IS_SET(job->john->specials.act, ACT_SENTINEL))
	job->john->doFollow(hookername.c_str());
      job->state=STATE_WALKING;
      break;
    case STATE_WALKING:
      //      if(::number(0,2))
      //	break;

      if(myself->in_room != homes[job->cur_path]){
	switch((dir=path.findPath(myself->in_room, 
				  findRoom(homes[job->cur_path])))){
          case 0: case 1: case 2: case 3: case 4: 
          case 5: case 6: case 7: case 8: case 9:
	    myself->goDirection(dir);
	    break;
          case -1: // lost
  	  default: // portal
	    myself->doSay("Damn, I think I'm lost.");
	    delete job->john;
	    if(myself->act_ptr){
	      delete static_cast<hunt_struct *>(myself->act_ptr);
	      myself->act_ptr = NULL;
	    }
	    break;
        }
// returns -1 indicating a problem or can't find a path
// returns 0-9 indicating a direction to travel
// returns 10+ indicating a portal to enter (10 = first in room, 11 = 2nd,...)

      } else {
	// if arrived at destination
	job->state=STATE_WAITFORCLEAR;
	myself->doSay("Let's get to work.");
	myself->doAction("", CMD_GRIN);
      }
      break;
    case STATE_WAITFORCLEAR:
      found=0;
      for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end() && (t=*it);++it){
	if(dynamic_cast<TPerson *>(t)){
	  switch(::number(0,1)){
	    case 0:
	      myself->doSay("Um, a little privacy please.");
	      myself->doAction("", CMD_COUGH);
	      break;
	    case 1:
	      job->john->doEmote("tries to hide his face from you.");
	      break;
	  }
	  found=1;
	}
      }
      if(!found){
	delete job->john;
	if(myself->act_ptr){
	  delete static_cast<hunt_struct *>(myself->act_ptr);
	  myself->act_ptr = NULL;
	  return DELETE_THIS;
	}
      }
      break;
  }
  
  return FALSE;
}


// this proc is kind of ugly, but it works
int bankGuard(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
{
  Descriptor *i;
  int zone_nr=real_roomp(31750)->getZoneNum(), v=0;
  TBeing *victims[10], *vict;
  int saferooms[20]={31750, 31751, 31756, 31757, 31758, 31759, 31764, 31788,
  31789,31790,31791,31792,31793,31794,31795,31796,31797,31798,31799,31760};  

  // only on pulse and only if we're not already hunting someone
  if(cmd != CMD_GENERIC_PULSE || IS_SET(myself->specials.act, ACT_HUNTING))
    return FALSE;

  for (i = descriptor_list; i && v<=9; i = i->next){
    if (!i->connected && i->character && i->character->roomp &&
	i->character->roomp->getZoneNum() == zone_nr &&
	i->character->in_room != saferooms[0] &&
	i->character->in_room != saferooms[1] &&
	i->character->in_room != saferooms[2] &&
	i->character->in_room != saferooms[3] &&
	i->character->in_room != saferooms[4] &&
	i->character->in_room != saferooms[5] &&
	i->character->in_room != saferooms[6] &&
	i->character->in_room != saferooms[7] &&
	i->character->in_room != saferooms[8] &&
	i->character->in_room != saferooms[9] &&
	i->character->in_room != saferooms[10] &&
	i->character->in_room != saferooms[11] &&
	i->character->in_room != saferooms[12] &&
	i->character->in_room != saferooms[13] &&
	i->character->in_room != saferooms[14] &&
	i->character->in_room != saferooms[15] &&
	i->character->in_room != saferooms[16] &&
	i->character->in_room != saferooms[17] &&
	i->character->in_room != saferooms[18] &&
	i->character->in_room != saferooms[19]){
      victims[v]=i->character;
      ++v;
    }
  }

  if(!v)
    return FALSE;

  vict=victims[::number(0,v-1)];
  vlogf(LOG_PEEL, format("bank guard hunting %s") %  vict->getName());
  myself->setHunting(vict);
  myself->addHated(vict);
  
  // set my followers to hate them too
  for (followData *f = myself->followers; f; f = f->next) {
    if(myself->inGroup(*f->follower)){
      f->follower->addHated(vict);
    }
  }

  return TRUE;
}

// for the jungle canyon zone - dash 7/25/01
int scaredKid(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
{
  TThing *t = NULL;
  TBeing *tb = NULL;
  bool found = false;

  if (cmd != CMD_GENERIC_QUICK_PULSE || myself->fight())
    return FALSE;
  
  // look for people to run from
  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end();){
    t=*(it++);
    if(!(tb=dynamic_cast<TBeing *>(t)))
      continue;
    if(tb->isPc() && myself->canSee(tb))
      found = true;
  }
  if (!found)
    return FALSE;

  // ok, its a pulse, i'm not fighting, and there's a big scary PC that i can see
  // RUN HOME!!!

  int r = myself->inRoom();
  switch (r) {
    case 27472:
      act("$n begins to cry as $e flees through the jungle.", FALSE, myself, 0, 0, TO_ROOM);
      myself->doMove(CMD_EAST);
      break;
    case 27473:
      act("$n sobs softly as $e flees through the jungle.", FALSE, myself, 0, 0, TO_ROOM);
      myself->doMove(CMD_EAST);
      break;
    case 27474:
      act("$n sobs softly as $e fiddles with one of the trees.", FALSE, myself, 0, 0, TO_ROOM);
      myself->doOpen("canopy");
      act("$n grabs a vine and leaps up through the hole in the canopy.", FALSE, myself, 0, 0, TO_ROOM);
      myself->doMove(CMD_UP);
      myself->doClose("canopy");
      break;
    case 27476:
      act("$n charges through the undergrowth, glancing over $s shoulder.", FALSE, myself, 0, 0, TO_ROOM);
      myself->doMove(CMD_NW);
      break;
    case 27477:
      act("$n charges through the undergrowth, glancing over $s shoulder.", FALSE, myself, 0, 0, TO_ROOM);
      myself->doMove(CMD_WEST);
      break;
    case 27480:
      act("$n runs toward the small wooden bridge to the northwest.", FALSE, myself, 0, 0, TO_ROOM);
      myself->doMove(CMD_NW);
      break;
    case 27481:
      act("$n charges through the undergrowth, glancing over $s shoulder.", FALSE, myself, 0, 0, TO_ROOM);
      myself->doMove(CMD_WEST);
      break;
    case 27483:
      act("$n squeaks with fright!", FALSE, myself, 0, 0, TO_ROOM);
      act("$n jumps to $s feet and dives into the undergrowth.", FALSE, myself, 0, 0, TO_ROOM);
      myself->doMove(CMD_NORTH);
      break;
    case 27488:
      act("$n screams with rage!", FALSE, myself, 0, tb, TO_ROOM);
      myself->doMove(CMD_EAST);
      myself->doMove(CMD_SOUTH);
      myself->doMove(CMD_SOUTH);
      myself->doMove(CMD_EAST);
      break;
    default:
      return FALSE;
      break;
  }

  return TRUE;
}



static int divCost(TObj *obj, TBeing *ch, unsigned int shop_nr)
{
  double cost=obj->obj_flags.cost;

  cost *= shop_index[shop_nr].getProfitBuy(obj, ch);

  if(cost<0)
    cost=0;

  return (int) cost;
}

int divman(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  char buf[256]; //,buf2[256];
  TObj *item;
  int cost;
  dirTypeT dir = DIR_NONE;
  roomDirData *exitp;
  TBeing *tbt = NULL;
  TThing *ttt;
  sstring str;
  TShopOwned *tso;

  class div_struct {
    public:
    int wait;
    int cost;
    char *char_name;
    char *obj_name;
    div_struct() :
      wait(0),
      cost(0),
      char_name(NULL),
      obj_name(NULL)
    {
    }
    ~div_struct() {
      delete [] char_name;
      char_name = NULL;
      delete [] obj_name;
      obj_name = NULL;
    }
  };
  static div_struct *job;

  switch (cmd) {
    case CMD_WHISPER:
      return shopWhisper(ch, me, find_shop_nr(me->number), arg);
    case CMD_GENERIC_DESTROYED:
      delete (div_struct *) me->act_ptr;
      me->act_ptr = NULL;
      return FALSE;
    case CMD_GENERIC_CREATED:
      if (!(me->act_ptr = new div_struct())) {
        perror("failed new of engraver.");
        exit(0);
      }
      return FALSE;
    case CMD_MOB_MOVED_INTO_ROOM:

      return kick_mobs_from_shop(me, ch, (int)o);

    case CMD_MOB_VIOLENCE_PEACEFUL:
      ttt = o;
      tbt = dynamic_cast<TBeing *>(ttt);
      me->doSay("Hey!  Take it outside.");
      for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
        if (exit_ok(exitp = me->exitDir(dir), NULL)) {
          act("$n throws you from $s shop.",
                 FALSE, me, 0, ch, TO_VICT);
          act("$n throws $N from $s shop.",
                 FALSE, me, 0, ch, TO_NOTVICT);
          me->throwChar(ch, dir, FALSE, SILENT_NO, true);
          act("$n throws you from $s shop.",
                 FALSE, me, 0, tbt, TO_VICT);
          act("$n throws $N from $s shop.",
                 FALSE, me, 0, tbt, TO_NOTVICT);
          me->throwChar(tbt, dir, FALSE, SILENT_NO, true);
          return TRUE;
        }
      }
      return TRUE;
    case CMD_GENERIC_PULSE:
      break;
    case CMD_NORTH:
    case CMD_SOUTH:
    case CMD_WEST:
    case CMD_EAST:
    case CMD_UP:
    case CMD_DOWN:
    case CMD_NE:
    case CMD_SW:
    case CMD_SE:
    case CMD_NW:
      if (!(job = (div_struct *) me->act_ptr)) {
        return FALSE;
      }
      if (!job->char_name) {
        return FALSE;
      }
      if (!strcmp(job->char_name, ch->getName())) {
        sprintf(buf, "%s! Don't leave until I finish with this %s!", ch->getName(), job->obj_name);
        me->doSay(buf);
        return TRUE;
      } else
        return FALSE;
      break;
    case CMD_VALUE: {
      for(; *arg && isspace(*arg);arg++);
      TThing *ts = NULL;
      TObj *valued = NULL;
      if (!(ts = searchLinkedListVis(ch, arg, ch->stuff)) ||
          !(valued = dynamic_cast<TObj *>(ts))) {
        me->doTell(ch->getName(), "You don't have that item.");
        return TRUE;
      }
      if (valued->obj_flags.decay_time >= 0) {
        me->doTell(ch->getName(), "Sorry, but this item won't last long and isn't worth the money spent.");
        return TRUE;
      }

      cost = divCost(valued, ch, find_shop_nr(me->number));

      me->doTell(ch->getName(), format("It will cost %d talens to identify your %s.") % cost % fname(valued->name));
      return TRUE;
      }
    case CMD_MOB_GIVEN_ITEM:
      if (!(item = o)) {
        me->doTell(ch->getName(), "You don't have that item!");
        return TRUE;
      }
      // prohibit polys and charms from engraving 
      if (dynamic_cast<TMonster *>(ch)) {
        me->doTell(fname(ch->name), "I don't identify for beasts.");
        me->doGive(ch, item,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      me->logItem(item, CMD_EAST);  // log the receipt of the item
      cost = divCost(item, ch, find_shop_nr(me->number));
      if (ch->getMoney() < cost) {
        me->doTell(ch->getName(), "I have to make a living! If you don't have the money, I don't do the work!");
        me->doGive(ch,item,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      sprintf(buf, "Thanks for your business, I'll take your %d talens payment in advance!", cost);
      me->doSay(buf);

      tso=new TShopOwned(find_shop_nr(me->number), 
			 dynamic_cast<TMonster *>(me), ch);
      tso->doBuyTransaction(cost, "divination", TX_BUYING_SERVICE);
      delete tso;

      ch->sendTo(COLOR_BASIC, format("%s concentrates deeply on %s.\n\r") % me->getName() % item->getName());
      ch->sendTo(format("%s conjures up a cloud of smoke.\n\rInside the cloud of smoke you see...\n\r") % me->getName());
      ch->statObjForDivman(item);
      sprintf(buf, "Thank you, %s, for your business! Please come again!", ch->getName());
      me->doSay(buf);
      me->doGive(ch,item,GIVE_FLAG_IGN_DEX_TEXT);


      me->saveChar(Room::AUTO_RENT);
      ch->saveChar(Room::AUTO_RENT);

      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}
int gardener(TBeing *, cmdTypeT cmd, const char *, TMonster *mob, TObj *)
{
  TTool *tool=NULL;

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  // if they're already planting or something, don't do anything
  // and if we're not planting, remove the sentinel bit that we set
  if (mob->task)
    return FALSE;
  else
    REMOVE_BIT(mob->specials.act, ACT_SENTINEL);

  if(::number(0,100))
    return FALSE;

  for(StuffIter it=mob->stuff.begin();it!=mob->stuff.end();++it){
    if((tool=dynamic_cast<TTool *>(*it)) &&
       tool->getToolType() == TOOL_SEED){
      break;
    }
    tool=NULL;
  }
  if(!tool)
    return FALSE;
  int count=0;
  for(StuffIter it=mob->roomp->stuff.begin();it!=mob->roomp->stuff.end();++it){
    if (dynamic_cast<TPlant *>(*it))
      ++count;
  }
  if (count >= 8) {
    REMOVE_BIT(mob->specials.act, ACT_SENTINEL);
    mob->doAction("", CMD_GRUMBLE);
    return FALSE;
  }

  
  // don't wander around while planting
  SET_BIT(mob->specials.act, ACT_SENTINEL);

  mob->doAction("", CMD_WHISTLE);
  mob->doPlant(add_bars(tool->name));
  return TRUE;
}


int bmarcher(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *)
{

  if (cmd != CMD_GENERIC_QUICK_PULSE || ::number(0,3)) 
    return FALSE;
  
  int arrownum = 0, bownum = 0;

  switch(ch->mobVnum()) {
    case 18576:
      bownum = 18576;
      arrownum = 18578;
      break;
    case 18577:
      bownum = 18577;
      arrownum = 18579;
      break;
  }

  TBow *bow = NULL;
  TObj *arrow = NULL;
  bool nobow = FALSE;
  int rm = 0, new_rm = 0;
  TThing *t=NULL;
  char temp[256];
  const char *directions[][2] =
  {
    {"north", "south"},
    {"east", "west"},
    {"south", "north"},
    {"west", "east"},
    {"up", "below"},
    {"down", "above"},
    {"northeast", "southwest"},
    {"northwest", "southeast"},
    {"southeast", "northwest"},
    {"southwest", "northeast"}
  };



  if(!(bow = dynamic_cast<TBow *>(ch->equipment[HOLD_RIGHT])))
    nobow = TRUE;
  else if (obj_index[bow->getItemIndex()].virt != bownum) {
    delete bow;
    nobow = TRUE;
  }

  if (nobow) {
    //    vlogf(LOG_DASH, "archer loading a new bow");
    if (!(bow = dynamic_cast<TBow *>(read_object(bownum, VIRTUAL)))) {
      vlogf(LOG_PROC, format("Archer couldn't load his bow %d.  DASH!!!") %  bownum);
      return TRUE;
    }
    strcpy(temp, bow->name);
    strcpy(temp, add_bars(temp).c_str());
    ch->doJunk(temp, NULL); // just in case its loaded, no point making tons
    act("You quickly unpack $p.", FALSE, ch, bow, 0, TO_CHAR);
    act("$n quickly unpacks $p.", FALSE, ch, bow, 0, TO_ROOM);
    ch->equipChar(bow, ch->getPrimaryHold(), SILENT_YES);
    return TRUE;
  }


  if (bow->stuff.empty()){
    //    vlogf(LOG_DASH, "archer loading an arrow");

    if (!(arrow = read_object(arrownum, VIRTUAL))) {
      vlogf(LOG_PROC, format("Archer couldn't load his arrow %d.  DASH!!!") %  arrownum);
      return TRUE;
    }
    *bow += *arrow;
    act("You quickly load $N into $p.", FALSE, ch, bow, arrow, TO_CHAR);
    act("$n quickly loads $N into $p.", FALSE, ch, bow, arrow, TO_ROOM);
    return TRUE;
  }

  if (bow->isBowFlag(BOW_STRING_BROKE)) {
    //    vlogf(LOG_DASH, "archer fixing a bowsstring");

    
    act("You quickly restring $p.", FALSE, ch, bow, 0, TO_CHAR);
    act("$n quickly restrings $p.", FALSE, ch, bow, 0, TO_ROOM);

    bow->remBowFlags(BOW_STRING_BROKE);
    return TRUE;
  }

  char buf[256], buf2[256];
  int count, numsimilar;
  int which;
  int Hi = 0, Hf = 0; //hp initial, hp final
  dirTypeT i;
  TBeing *tbt = NULL;
  TBeing *tbt2 = NULL;

  for (i = MIN_DIR; i <= (MAX_DIR - 1); i++) {
    rm = ch->in_room;
    if (clearpath(rm, i)) {
      
      new_rm = real_roomp(rm)->dir_option[i]->to_room;
      if (new_rm == rm || (real_roomp(rm)->isRoomFlag(ROOM_PEACEFUL)))
	continue;
      else
	rm = new_rm;
      count = 0;

      

      for(StuffIter it=real_roomp(rm)->stuff.begin();it!=real_roomp(rm)->stuff.end() && (t=*it);++it) {
	tbt = dynamic_cast<TBeing *>(t);
	if (!tbt)
	  continue;
	if (!tbt->isCult())
	  continue;
	if (!ch->canSee(tbt))
          continue;

	//ok we have a mob and he's a logrite - tally him the pick a random one.
	count++;
      }
      
      if (count == 0)
	return TRUE;
      

      which = ::number(1,count);// which target to pick on the next pass
      count = 0;
      for(StuffIter it=real_roomp(rm)->stuff.begin();it!=real_roomp(rm)->stuff.end() && (t=*it);++it) {
        tbt = dynamic_cast<TBeing *>(t);
        if (!tbt)
          continue;
        if (!tbt->isCult())
          continue;
	if (!ch->canSee(tbt))
	  continue;
        //ok we have a mob and he's a logrite - is he the one we want?.
        count++;
	if (count == which)
	  break;
      }
	
      // ok now we have to do a bit of trickery - there could be multiple identical mobs
      // in the same room, ie, 3 orc guards, and even if we pick the third one our code
      // will only aim at the first one.  So we have to figure out which we're aiming at
      // before we can lob one off and hope to hit him.

      strcpy(temp, tbt->name);
      count =0;
      numsimilar = 0;
      for(StuffIter it=real_roomp(rm)->stuff.begin();it!=real_roomp(rm)->stuff.end() && (t=*it);++it) {
        tbt2 = dynamic_cast<TBeing *>(t);
        if (!tbt2)
          continue;
        if (!ch->canSee(tbt2))
          continue;
	if (tbt->isCult())
	  count++;
	if (!strcmp(temp, tbt2->name)) // this mob has the same name as our target, so count him
	  numsimilar++;
	if (count == which)
          break;
      }

      numsimilar = max(numsimilar, 1);// sometimes we get 0 instead of 1 if there is only one in the room

      strcpy(temp, add_bars(temp).c_str());


      Hi = tbt->getHit();

      if(ch->in_room == Room::TROLLEY) {
	
	sprintf(buf, "From inside the trolley, $N aims a $o at you.");
	act(buf,FALSE, tbt, bow, ch, TO_CHAR);
	sprintf(buf, "From inside the trolley, $N aims a $o at $n.");
	act(buf,FALSE, tbt, bow, ch, TO_ROOM);
	sprintf(buf, "You aim a $o out the trolley at $N.");
	act(buf,FALSE, ch, bow, tbt, TO_CHAR);
	sprintf(buf, "$n aims a $o out the trolley at $N.");
	act(buf,FALSE, ch, bow, tbt, TO_ROOM);
      } else {
	
	sprintf(buf, "From up on the %s tower, $N aims a $o down at you.", directions[i][1]);
	act(buf,FALSE, tbt, bow, ch, TO_CHAR);
	sprintf(buf, "From up on the %s tower, $N aims a $o down at $n.", directions[i][1]);
	act(buf,FALSE, tbt, bow, ch, TO_ROOM);
	sprintf(buf, "You aim a $o down through the %s arrow slit at $N.", directions[i][0]);
	act(buf,FALSE, ch, bow, tbt, TO_CHAR);
	sprintf(buf, "$n aims a $o down through the %s arrow slit at $N.", directions[i][0]);
	act(buf,FALSE, ch, bow, tbt, TO_ROOM);
      }

      sprintf(buf, "%s %d.%s 1", directions[i][0], numsimilar, temp);
      //      vlogf(LOG_DASH, format("Brightmoon Defense: %s shooting at %s (%d.%s)") %  ch->getName() % tbt->getName() % numsimilar % temp);

      strcpy(temp, tbt->getName());

      ch->doShoot(buf);
      //      vlogf(LOG_DASH, "archer shooting at a target");
      Hf = tbt->getHit();

#if 1
      //      vlogf(LOG_DASH, format("archer debug: %d->%d, temp/name: (%s)/(%s), tbt?: %s") % 
      //    Hi % Hf % temp % (tbt->getName() ? tbt->getName() : "(NULL)") % (tbt ? "exists" : "(NULL)"));
#endif
      if (!tbt->getName()) {
        switch(::number(1,7)) {
          case 1:
            ch->doShout("Woo woo!  I got a confirmed kill, yeah!");
            break;
          case 2:
            ch->doShout("How's that for Brightmoon hospitality, you Logrite bastards!");
            break;
          case 3:
            ch->doShout("And another one gone, another one gone, another one bites the dust, yeah!");
            break;
          case 4:
            ch->doShout("Who's yo daddy!");
            break;
          case 5:
            sprintf(buf, "Cleanup on aisle 9 - we got %s splattered all over the place!", temp);
            ch->doShout(buf);
            break;
          case 6:
            sprintf(buf, "I hope you brought your recall scrolls with you, %s, because... oops, too late.", temp);
            ch->doShout(buf);
            break;
          case 7:
            sprintf(buf, "Good luck getting your corpse, %s - you'll need it!", temp);
            ch->doShout(buf);
            break;
        }
        return TRUE;
      } else if (Hi > Hf) {
	// we hit them, so lets shout some catcalls down
	switch (::number(1,9)) {
	  case 1:
	    sprintf(buf2, "How 'bout DEM apples!");
	    break;
	  case 2:
	    sprintf(buf2, "Why don't you go on a diet you fat bastard, I couldn't miss you if I tried!");
	    break;
	  case 3:
	    sprintf(buf2, "U-G-L-Y, you ain't got no ALIBI, you UGLY!");
	    break;
	  case 4:
	    sprintf(buf2, "Hey, I didn't know Logrus was enlisting pincushions!");
	    break;
	  case 5:
	    sprintf(buf2, "Why don't you go home, you dirty whores!");
	    break;
	  case 6:
	    sprintf(buf2, "You like it this way?  UNGH!  You like that??  UNGH!  UNGH!  Take that!  UNGH!");
	    break;
	  case 7:
	    sprintf(buf2, "Eat some of that!");
	    break;
	  case 8:
	    sprintf(buf2, "Straight from my heart to yours!");
	    break;
	  case 9:
	    sprintf(buf2, "Wa-hey, bitch!");
	    break;
	}
	
	
	sprintf(buf, "<c>$N yells,<1> \"%s\"", buf2);
	act(buf,FALSE, tbt, bow, ch, TO_CHAR);
	sprintf(buf, "<c>$N yells,<1> \"%s\"", buf2);
	act(buf,FALSE, tbt, bow, ch, TO_ROOM);
	sprintf(buf, "<c>You yell,<1> \"%s\"", buf2);
	act(buf,FALSE, ch, bow, tbt, TO_CHAR);
	sprintf(buf, "<c>$n yells,<1> \"%s\"", buf2);
	act(buf,FALSE, ch, bow, tbt, TO_ROOM);
      }
      return TRUE;
     
    }
  }
  return TRUE;
}

// this uses some hardcoded values, so don't reuse without tweaking
// for general use
int aggroFollower(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  bool fighting=false;
  TThing *t=NULL;
  TMonster *tmons;
  int rc;
  sstring argument=arg;

  if(cmd != CMD_GENERIC_PULSE &&
     !((cmd == CMD_SAY || cmd == CMD_SAY2) && 
       ((argument.word(0).lower()=="pirates,") ||
	(argument.word(0).lower()=="pirate,")) && ch &&
       (ch->desc->account->name == "trav" ||
	ch->desc->account->name == "scout" ||
	ch->desc->account->name == "laren" ||
	ch->desc->account->name == "ekeron")))
    return FALSE;
  
  if(cmd==CMD_GENERIC_PULSE){
    // if fighting return false
    if(myself->fight()){
      fighting=true;
    } else {
      for (followData *f = myself->followers; f; f = f->next){
	if(myself->inGroup(*f->follower) && 
	   (f->follower->fight() || f->follower->spelltask)){
	  fighting=true;
	}
      }
    }
    
    if(fighting)
      return FALSE;
    
    // if mob in room that isn't in my group
    //   if within my level range (my level + 10 or lower?)
    //     attack
    for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end() && (t=*it);++it){
      if((tmons=dynamic_cast<TMonster *>(t)) &&
	 !tmons->inGroup(*myself) && !tmons->isImmortal()){

	// don't attack mobs from my friendly zones
	if((tmons->mobVnum() >= 19000 && tmons->mobVnum() <= 19024) ||
	   (tmons->mobVnum() >= 26850 && tmons->mobVnum() <= 26899))
	  continue;

	rc = myself->takeFirstHit(*tmons);
	if (IS_SET_DELETE(rc, DELETE_VICT)) {
	  delete tmons;
	  tmons = NULL;
	}
	if (IS_SET_DELETE(rc, DELETE_THIS)) 
	  return DELETE_VICT;
      }
    }
    return TRUE;
  } else {
    if(cmd == CMD_SAY || cmd == CMD_SAY2){
      if(argument.word(1) == "follow"){
	myself->doAction(add_bars(ch->getName()), CMD_SALUTE);
	myself->doFollow(add_bars(ch->getName()).c_str());
      } else if(argument.word(1) == "stay"){
	myself->doAction(add_bars(ch->getName()), CMD_SALUTE);
	myself->doFollow("self");
      }
    }
    return FALSE;
  }

  return FALSE;
}


int adventurer(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  bool fighting=false;
  TThing *t=NULL;
  TMonster *tmons;
  int rc;

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  // if fighting return false
  if(myself->fight()){
    fighting=true;
  } else {
    for (followData *f = myself->followers; f; f = f->next){
      if(myself->inGroup(*f->follower) && 
	 (f->follower->fight() || f->follower->spelltask)){
	fighting=true;
      }
    }
  }

  if(fighting)
    return FALSE;

  // if mob in room that isn't in my group
  //   if within my level range (my level + 10 or lower?)
  //     attack
  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end() && (t=*it);++it){
    if((tmons=dynamic_cast<TMonster *>(t)) &&
       !tmons->inGroup(*myself)){
      rc = myself->takeFirstHit(*tmons);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
	delete tmons;
	tmons = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS)) 
	return DELETE_VICT;
    }
  }



  // else
  //   wander randomly
  myself->wanderAround();


  return TRUE;
}

int barmaid(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{

  TThing *t=NULL, *t2=NULL;
  TBaseCup *glass;
  TTable *table;


  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  // if fighting return false
  if(myself->fight() || myself->getWait())
    return FALSE;


  if ((myself->getCarriedWeight()/myself->carryWeightLimit()) * 100.0 > 25 || 
      (myself->getCarriedVolume()/myself->carryVolumeLimit()) * 100.0 > 25) {
    act("$n carries the empty glasses behind the counter and washes them.",FALSE, myself, 0, 0, TO_ROOM);

    for(StuffIter it=myself->stuff.begin();it!=myself->stuff.end();){
      t=*(it++);
      if((glass=dynamic_cast<TBaseCup *>(t))){
	--(*glass);
        delete glass;
        glass = NULL;
        myself->addToWait(2);
      }
    }

    myself->addToWait(50);
    return TRUE;
  }
  
  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end() && (t=*it);++it){
    if((table=dynamic_cast<TTable *>(t))){
      
      for(t2=table->rider;t2;t2=t2->nextRider){
	if((glass=dynamic_cast<TBaseCup *>(t2))){
	  //	  vlogf(LOG_DASH, format("Barmaid: found  %s with %d units left.") %  glass->getName() % glass->getDrinkUnits());
	  if (glass->getDrinkUnits() <= 0) {
	    //	    vlogf(LOG_DASH, format("Barmaid: found empty %s on %s.") %  glass->getName() % table->getName());

	    glass->dismount(POSITION_DEAD);
	    //	    --(*glass);
	    *myself += *glass;
	    act("$n gets $p from $P.",FALSE, myself, glass, table, TO_ROOM);
	    act("$n carefully balances the empty glass on $s tray.",FALSE, myself, glass, 0, TO_ROOM);
	    act("You get $p from $P.",FALSE, myself, glass, table, TO_CHAR);
	    act("You carefully balance the empty glass on your tray.",FALSE, myself, glass, 0, TO_CHAR);
	    myself->addToWait(20);
	    return TRUE;
	  }
	}
      }
    } else if ((glass=dynamic_cast<TBaseCup *>(t))){
      //      vlogf(LOG_DASH, format("Barmaid: found  %s with %d units left.") %  glass->getName() % glass->getDrinkUnits());
      if(glass->getDrinkUnits() <= 0) {
	//	vlogf(LOG_DASH, format("Barmaid: found empty  %s.") %  glass->getName());
	--(*glass);
	*myself += *glass;
	act("$n gets $p.",FALSE, myself, glass, 0, TO_ROOM);
	act("$n carefully balances the empty glass on $s tray.",FALSE, myself, glass, 0, TO_ROOM);
	act("You get $p.",FALSE, myself, glass, 0, TO_CHAR);
        act("You carefully balance the empty glass on your tray.",FALSE, myself, glass, 0, TO_CHAR);


	myself->addToWait(20);
	return TRUE;
      }
    }
  }
  
  if (!::number(0,19)) {
    act("$n flirts with some of the patrons." , FALSE, myself, myself, myself, TO_ROOM);
    myself->addToWait(20);
  }

  return TRUE;
}


bool okForCommodMaker(TObj *o, sstring &ret)
{
    if(material_nums[o->getMaterial()].price <= 0){
      ret=format("%s: That isn't a valuable - I can't convert that.") % o->getName();
      return false;
    }
    
    if(dynamic_cast<TCommodity *>(o)){
      ret=format("%s: That's already a commodity.") % o->getName();
      return false;
    }
    
    if(!o->isRentable()){
      ret=format("%s: That isn't rentable so I can't convert it.") % o->getName();
      return false;
    }
    
    if(o->isMonogrammed()){
      ret=format("%s: Sorry, I don't convert monogrammed items.") % o->getName();
      return false;
    }

    if(dynamic_cast<TComponent *>(o)){
      ret=format("%s: Sorry, I cannot convert magical components.") % o->getName();
      return false;
    }

    TBaseCup *tbc;
    if((tbc=dynamic_cast<TBaseCup *>(o)) && tbc->getDrinkUnits()){
      ret=format("%s: Sorry, I can't convert liquid containers unless they are empty.") % o->getName();
      return false;
    }

    TObj *obj;
    for(StuffIter it=o->stuff.begin();it!=o->stuff.end();++it){
      if(!(obj=dynamic_cast<TObj *>(*it)))
	      continue;

      if(material_nums[obj->getMaterial()].price <= 0){
        ret=format("%s: That isn't a valuable - I can't convert that.") % obj->getName();
        return false;
      }
      
      if(dynamic_cast<TCommodity *>(obj)){
        ret=format("%s: That's already a commodity.") % obj->getName();
        return false;
      }
      
      if(!obj->isRentable()){
        ret=format("%s: That isn't rentable so I can't convert it.") % obj->getName();
        return false;
      }
      
      if(obj->isMonogrammed()){
        ret=format("%s: Sorry, I don't convert monogrammed items.") % o->getName();
        return false;
      }

      if(dynamic_cast<TComponent *>(obj)){
        ret=format("%s: Sorry, I cannot convert magical components.") % obj->getName();
        return false;
      }
      
      TBaseCup *tbc;
      if((tbc=dynamic_cast<TBaseCup *>(obj)) && tbc->getDrinkUnits()){
        ret=format("%s: Sorry, I can't convert liquid containers unless they are empty.") % obj->getName();
        return false;
      }
    }
    
    return true;
}


std::map <ubyte,int> commodMakerValue(TObj *o, float &value)
{
  std::map <ubyte,int> mat_list;
  const float wastage=0.90;
  value=0;
  int amt=0;
  TObj *obj;

  amt=(int)(o->getWeight() * 10.0 * wastage);
  mat_list[o->getMaterial()]+=amt;
  value += max((float)1.0, amt * (float)material_nums[o->getMaterial()].price);


  for(StuffIter it=o->stuff.begin();it!=o->stuff.end();++it){
    if(!(obj=dynamic_cast<TObj *>(*it)))
      continue;

    amt=(int)(obj->getWeight() * 10.0 * wastage);
    mat_list[obj->getMaterial()]+=amt;
    value += max((float)1.0, amt * (float)material_nums[obj->getMaterial()].price);
  }

  return mat_list;
}

int commodMaker(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  float value;
  sstring buf;
  TObj *commod=NULL;
  int shop_nr=find_shop_nr(me->number);
  TThing *ts = NULL;
  std::map <ubyte,int> mat_list;
  std::map <ubyte,int>::iterator iter;

  if(cmd == CMD_WHISPER)
    return shopWhisper(ch, me, shop_nr, arg);

  if(cmd != CMD_VALUE && cmd != CMD_MOB_GIVEN_ITEM)
    return FALSE;

  if(cmd == CMD_VALUE){
    if (!(ts = searchLinkedListVis(ch, arg, ch->stuff)) ||
	!(o = dynamic_cast<TObj *>(ts))) {
      me->doTell(ch->getName(), "You don't have that item.");
      return TRUE;
    }

    if(!okForCommodMaker(o, buf)){
      me->doTell(ch->getName(), buf);
      return TRUE;
    }

    mat_list=commodMakerValue(o, value);

    me->doTell(ch->getName(), "I can turn that into:");
    for(iter=mat_list.begin();iter!=mat_list.end();++iter){
       me->doTell(ch->getName(), format("%i units of %s.") %
		 (*iter).second % material_nums[(*iter).first].mat_name);
    }
    me->doTell(ch->getName(), format("My fee for this is %i talens.") %
	       (int)(shop_index[shop_nr].getProfitBuy(o, ch) * value));

    return TRUE;
  } else if(cmd == CMD_MOB_GIVEN_ITEM){
    if(material_nums[o->getMaterial()].price <= 0){
      me->doTell(ch->getName(), "That isn't a valuable commodity - I can't convert that.");
      me->doGive(ch,o, GIVE_FLAG_DROP_ON_FAIL);
      return TRUE;
    }

    if(!okForCommodMaker(o,buf)){
      me->doTell(ch->getName(), buf);
      me->doGive(ch,o, GIVE_FLAG_DROP_ON_FAIL);
      return TRUE;
    }
    
    mat_list=commodMakerValue(o, value);
    value = (int)(shop_index[shop_nr].getProfitBuy(o, ch) * value);

    if(ch->getMoney() < (int)value){
      me->doTell(ch->getName(), "You can't afford it!");
      me->doGive(ch,o, GIVE_FLAG_DROP_ON_FAIL);
      return TRUE;
    }


    for(iter=mat_list.begin();iter!=mat_list.end();++iter){
      TShopOwned tso(shop_nr, me, ch);
      tso.doBuyTransaction((int)value, format("deconstructing %s (%s)") % 
			   o->getName() % 
			   material_nums[(*iter).first].mat_name,
			   TX_BUYING_SERVICE);
      
      commod = read_object(Obj::GENERIC_COMMODITY, VIRTUAL);
      
      commod->setWeight((*iter).second/10.0);
      commod->setMaterial((*iter).first);
      
      me->doTell(ch->getName(), "Alright, here you go!");
      
      *me += *commod;
      me->doGive(ch,commod, GIVE_FLAG_DROP_ON_FAIL);
    }
  
    delete o;
  }

  return FALSE;
}


int konastisGuard(TBeing *ch, cmdTypeT cmd, const char *argument, TMonster *me, TObj *o)
{
  sstring arg=argument, buf;
  TDrinkCon *tdc;
  TObj *obj;

  if(cmd!=CMD_SAY && cmd!=CMD_SAY2 && cmd!=CMD_MOB_GIVEN_ITEM)
    return false;

  if((cmd == CMD_SAY || cmd==CMD_SAY2) && arg.lower() == "hello"){
    me->doSay("Bring me some beer and maybe I'll give you the key!");
    me->doAction("", CMD_LICK);
    return true;
  }

  if(cmd == CMD_MOB_GIVEN_ITEM && o && (tdc=dynamic_cast<TDrinkCon *>(o))){
    buf = format("takes a sip from the %s.") % tdc->getName();

    switch(tdc->getDrinkType()){
      case LIQ_BEER:
	me->doEmote(buf);
	obj=read_object(18600, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("I could really use an ale to go with this beer!");
	break;
      case LIQ_ALE:
	me->doEmote(buf);
	obj=read_object(18601, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("This ale is delicious, but I'd like a dark ale!");
	break;
      case LIQ_DARKALE:
	me->doEmote(buf);
	obj=read_object(18602, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("I love ales and beers, but I think I'd like to try some white wine.");
	break;
      case LIQ_WINE:
	me->doEmote(buf);
	obj=read_object(18603, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("Ahh good, but now I'd care to sample some red wine.");
	break;
      case LIQ_RED_WINE:
	me->doEmote(buf);
	obj=read_object(18604, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("Wine is great for taste, but I'd like to move on to something harder now, say, whiskey!");
	break;
      case LIQ_WHISKY:
	me->doEmote(buf);
	obj=read_object(18605, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("That hits the spot.  Let's kick it up a notch with some firebreather!");
	break;
      case LIQ_FIREBRT:
	me->doEmote(buf);
	obj=read_object(18606, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("This stuff tastes foul.  Now vodka grain alcohol, there's a drink that goes down easy.");
	break;
      case LIQ_VODKA:
	me->doEmote(buf);
	obj=read_object(18607, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("I need to cleanse my palate with some mead.");
	break;
      case LIQ_MEAD:
	me->doEmote(buf);
	obj=read_object(18608, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("Ok, let's get back to the hard stuff.  How about some nice rum?");
	break;
      case LIQ_RUM:
	me->doEmote(buf);
	obj=read_object(18609, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("Ahh, reminds me of my bucaneering days.  We used to loot brandy from rich manors.");
	break;
      case LIQ_BRANDY:
	me->doEmote(buf);
	obj=read_object(18610, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("Whew, this is quite a drinking binge.  Time to celebrate with some champagne!");
	break;
      case LIQ_CHAMPAGNE:
	me->doEmote(buf);
	obj=read_object(18611, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("Not bad, not bad, but I need some coffee now to stay awake.");
	break;
      case LIQ_COFFEE:
	me->doEmote(buf);
	obj=read_object(18612, VIRTUAL);
	*me += *obj;
	me->doGive(ch,obj,GIVE_FLAG_IGN_DEX_TEXT);
	me->doSay("Ok I've had enough to drink for now.  Come by some other time!");
	break;
      default:
	return false;
    }
    return DELETE_ITEM;
  }
  
  return false;
}


// riddling tree proc
int riddlingTree(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *tree, TObj *)
{
  
  if (cmd != CMD_SAY && cmd != CMD_SAY2 && cmd != CMD_TELL && cmd != CMD_OPEN)
    return FALSE;
    
  sstring sarg = arg;
  if ( (cmd == CMD_TELL && sarg.find(" ") > 0 && 
         !isname(sarg.substr(0,(int) sarg.find(" ")), tree->name)) ||
       ((cmd == CMD_SAY || cmd == CMD_SAY2) 
          && sarg.lower().find("clue") == sstring::npos ))
  {
    return FALSE;
  }
  
// make him unresponsive if somehow he's in the wrong room
  if (tree->inRoom() != 24700)
    return FALSE;
    
  if (cmd == CMD_OPEN && sarg.lower().find("vine") != sstring::npos) 
  {
    act("$n blocks your way.", TRUE, tree, NULL, ch, TO_VICT);
    act("$N moves towards some vines, but $n blocks the way.", 
      TRUE, tree, NULL, ch, TO_NOTVICT);
    act("<c>$n says,<z> \"Just where do you think you're going?!?\"", 
      TRUE, tree, NULL, ch, TO_ROOM);
    return TRUE;
  } else if (cmd == CMD_OPEN) 
    return FALSE;
  
  int whichRiddle;
  std::vector <sstring> riddles;
  std::vector <sstring> answers;
  static time_t firstGuessTime;
  static short int chancesLeft;
  int elapsedTime = 0;

  // reset the proc after 10 minutes
  if (firstGuessTime) elapsedTime = (time(0) - firstGuessTime) / 60;
  if (elapsedTime >= 10) firstGuessTime = 0;
  if (!firstGuessTime) chancesLeft = 3;
  
  riddles.push_back("A great tree that spreads its roots where dryads step around the gentle dancing of nymphs.");
  answers.push_back("oak");

  riddles.push_back("An odd-looking tree, but quite friendly.  Always had problems with nasty werewolves scraping off its bark.");
  answers.push_back("scraggly");

  riddles.push_back("A great tree, but hard set-upon by the large number of travelers that now issue from that vile fledgling city to the south of it.");
  answers.push_back("spruce");

  riddles.push_back("Among the oldest of trees and very wise, it sits in a grove sometimes frequented by shedu seeking knowledge.");
  answers.push_back("willow");

  riddles.push_back("Alas, last I saw this great tree was sitting in the midst of slaughtered animals.  I suppose it should come as no surprise that the nasty mess-making humans had stolen the animals from their own kin.");
  answers.push_back("ash");

  riddles.push_back("I always loved to watch the griffons and winged lions sweep about the skies near this tree.  The gnats were a little annoying, though.");
  answers.push_back("redwood");
  
  riddles.push_back("This tree was well-tended by elves from a nearby village."); 
  answers.push_back("elm"); 
  
  riddles.push_back("A settlement of short hairy-footed folk had recently been established near this tree last I visited.");
  answers.push_back("apple");

  riddles.push_back("Passing travellers had a bad habit of writing their initials in this one.  Felons, all.");
  answers.push_back("hickory");

  riddles.push_back("A most wonderful valley, with many elk, black bears, and bluejays and a curious saltwater lake was the home of this tree.");
  answers.push_back("pine");

  riddles.push_back("A dark forest well-protected by its bears was this one's home.");
  answers.push_back("oak");

  riddles.push_back("Home to many birds and monkeys, this fair tree grows where lions and zebras roam.");
  answers.push_back("acacia");
  
  // which riddle are we using this boot
  whichRiddle = Uptime % riddles.size();
  vlogf(LOG_MAROR, format("riddle = %s") % riddles[whichRiddle]);
  vlogf(LOG_MAROR, format("answer = %s") %  answers[whichRiddle]);

  if (cmd == CMD_TELL && sarg.lower().find(answers[whichRiddle]) != 
      sstring::npos) {
// reset guessing
    firstGuessTime = 0;
    tree->doAction("", CMD_SMILE);
    act("<c>$n says,<z> \"Oh, thank you - you have brought a ray of sunshine into an old tree's day.\"", 
      TRUE, tree, NULL, ch, TO_ROOM);
    act("$n hums a little tune.", TRUE, tree, NULL, ch, TO_ROOM);
    tree->openUniqueDoor(DIR_NORTH, DOOR_UNIQUE_DEF,
      "",
      "",
      "The tree reaches out gnarled hands and pushes some vines to the north aside and away.",
      "The tree reaches out gnarled hands and pushes some vines to the north aside and away.",
      "Some vines to the south have been pushed aside.",
      "",
      "",
      ""
    );
  }
  else if ( chancesLeft > 0 && (cmd == CMD_TELL || cmd == CMD_SAY 
        || cmd == CMD_SAY2) && 
      sarg.lower().find("clue") != sstring::npos) 
  {
    act("<c>$n says,<z> \"A clue, hmm, a clue... ah, hmm, yes, if you answer this I'll know that my friend you have found:\"", 
      TRUE, tree, NULL, ch, TO_ROOM);
    sstring sayRiddle;
    sayRiddle = format("<c>$n says,<z> \"%s\"") % riddles[whichRiddle];
    act(sayRiddle, TRUE, tree, NULL, ch, TO_ROOM);
    sstring askForClue;
    askForClue = format("<c>$n says,<z> \"You'll have to <g>tell<z> me the answer if you hope to pass.  I'll give you %d %s to guess.\"") %
      chancesLeft % ((chancesLeft != 1) ? "chances" : "chance");
    act(askForClue, TRUE, tree, NULL, ch, TO_ROOM);
  }
 // control response to other tells to the tree 
  else 
/*  if ((cmd == CMD_SAY && chancesLeft == 0 ) || (cmd == CMD_TELL &&*/
/*      sarg.lower().find(answers[whichRiddle]) == sstring::npos))*/
  {
    if (chancesLeft == 3) 
      firstGuessTime = time(0);
    chancesLeft--;
    act("$n shrugs helplessly.", TRUE, tree, NULL, ch, TO_ROOM);
    if (chancesLeft > 0)
    {
      act("<c>$n says<z>, \"A shame, a shame... I do hope you're wrong due to ignorance, and not because my friend no longer stands.\"", 
        TRUE, tree, NULL, ch, TO_ROOM);
      tree->doAction("",CMD_SNICKER); 
      sstring stateChancesLeft;
      stateChancesLeft = format("<c>$n says,<z> \"There's a good chance of that, at least.  You have %d %s left, so you'd best stop fooling around.\"") %
        chancesLeft % ((chancesLeft != 1) ? "chances" : "chance");
      act(stateChancesLeft, TRUE, tree, NULL, ch, TO_ROOM);
    } else {
      act("<c>$n says,<z> \"I gave plenty of chances.  I still am not sure whether to mourn my friend or pity your kind its ignorance, but I'm certainly not letting anyone in.\"",
        TRUE, tree, NULL, ch, TO_ROOM);
    }
  }
  return TRUE;
    
}

int shopWhisper(TBeing *ch, TMonster *myself, int shop_nr, const char *arg)
{
  char buf[256];
  
  arg = one_argument(arg, buf, cElements(buf));
  if(!isname(buf, myself->name))
    return FALSE;
  
  arg = one_argument(arg, buf, cElements(buf));
  
  TShopOwned tso(shop_nr, myself, ch);
  
  if(!strcmp(buf, "info")){ /////////////////////////////////////////
    tso.showInfo();
  } else if(!strcmp(buf, "statements")){
    tso.giveStatements(arg);
  } else if(!strcmp(buf, "dividend")){
    tso.setDividend(arg);
  } else if(!strcmp(buf, "reserve")){
    tso.setReserve(arg);
  } else if(!strcmp(buf, "setrates")){ //////////////////////////////////
    tso.setRates(arg);
  } else if(!strcmp(buf, "buy")){ /////////////////////////////////
    tso.buyShop(arg);
  } else if(!strcmp(buf, "sell")){ //////////////////////////////////
    tso.sellShop();
  } else if(!strcmp(buf, "give")){ /////////////////////////////
    tso.giveMoney(arg);
  } else if(!strcmp(buf, "access")){ ////////////////////////////
    tso.setAccess(arg);
  } else if(!strcmp(buf, "logs")){ /////////////////////////////////////////
    tso.doLogs(arg);
  } else if(!strcmp(buf, "string")){
    tso.setString(arg);
  } else {
    myself->doTell(ch->getName(), "Read 'help shop owner' for assistance.");
  }

  return TRUE;
}

int mimic(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *mimic, TObj *)
{
  if (cmd != CMD_SAY && cmd != CMD_SAY2 && cmd != CMD_SHOUT && cmd != CMD_WHISPER)
    return FALSE;

  sstring sarg = arg;
  sstring target, buf;
  one_argument(sarg, target);

  if (isname(target, mimic->name) || isname(ch->name, mimic->name) ) {
      return FALSE;
  }
  
  bool snicker = FALSE;
  if ( sarg.lower().find("i'm") != sstring::npos ||
        sarg.lower().find("i am") != sstring::npos)
  {
    snicker = TRUE;
  }
  
  if (cmd == CMD_WHISPER) {
    ch->doWhisper(arg);
    buf = format("%s psssst") % target; 
    if (snicker) {
      mimic->doAction("", CMD_SNICKER);
    } else
      mimic->doWhisper(buf.c_str());
    return TRUE;
  }

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    ch->doSay(arg);
    if (snicker) {
      mimic->doAction("", CMD_SNICKER);
    } else
      mimic->doSay(arg);
    return TRUE;
  }

  if (cmd == CMD_SHOUT) {
    ch->doShout(arg);
    if (snicker) {
      mimic->doAction("", CMD_SNICKER);
    } else
      mimic->doShout(arg);
    return TRUE;
  }

  return FALSE;

}  


int chicken(TBeing *, cmdTypeT cmd, const char *, TMonster *chicken, TObj *)
{

  if(cmd != CMD_GENERIC_PULSE || !chicken || !chicken->roomp)
    return FALSE;

  if(::number(0,4999))
    return FALSE;

  TObj *egg=read_object(2376, VIRTUAL);

  if(!egg){
    vlogf(LOG_BUG, "problem loading chicken egg in chicken spec proc");
    return FALSE;
  }

  *chicken->roomp += *egg;
  
  act("$n lays an egg.", TRUE, chicken, NULL, NULL, TO_ROOM);

  return FALSE;
}

int shippingOfficial(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  sstring argument=arg;
  TObj *o;

  if(cmd != CMD_SAY && cmd != CMD_SAY2)
    return FALSE;

  if(argument!="I need a receipt")
    return FALSE;

  if(!ch || !myself)
    return FALSE;

  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end();++it){
    if((o=dynamic_cast<TObj *>(*it))){
      if(o->objVnum() == 90){
	myself->doEmote("looks the crate over carefully.");
	myself->doSay("Ok, this looks good.");
	myself->doEmote("signals a worker, who drags the crate into the warehouse.");

	delete o;

	o=read_object(91, VIRTUAL);

	if(!o){
	  vlogf(LOG_BUG, "couldn't load receipt in shippingOfficial()");
	  myself->doSay("Hmm, I seem to be out of receipts, sorry.");
	  return TRUE;
	}

	*myself += *o;

	sstring giveBuf = format("%s %s") % 
	  add_bars(o->name) % add_bars(ch->name);
	myself->doGive(giveBuf, GIVE_FLAG_IGN_DEX_TEXT);

	return TRUE;
      }
    }
  }
  return FALSE;
}


int cat(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  TBeing *tb;
  TMonster *tm;
  std::vector <TBeing *> roompeople;
  std::vector <TObj *> roomobj;
  TObj *o;
  affectedData af;

  if(!me || !me->roomp || (me->getPosition() <= POSITION_SLEEPING) ||
     (cmd!=CMD_GENERIC_PULSE && cmd!=CMD_PET))
    return FALSE;

  if(cmd==CMD_PET){
    for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end();++it){
      if (((tb = get_char_room_vis(ch, arg, NULL, EXACT_YES)) ||
	   (tb = get_char_room_vis(ch, arg))) && tb==me){
	ch->doAction(arg, CMD_PET);
	switch(::number(0,4)){
	  case 0:
	    me->doAction(add_bars(ch->name), CMD_PURR);
	    break;
	  case 1:
	    me->doAction(add_bars(ch->name), CMD_DROOL);
	    break;
	  case 2:
	    me->doAction(add_bars(ch->name), CMD_LICK);
	    break;
	  case 3:
	    me->doAction(add_bars(ch->name), CMD_STRETCH);
	    break;	    
	  case 4:
	    me->doAction(add_bars(ch->name), CMD_NUZZLE);
	    break;	    
	}	    
	return TRUE;
      }
    }

    return FALSE;
  }

  if(cmd==CMD_GENERIC_PULSE){
    if(::number(0,49))
      return FALSE;
    
    // LOOK FOR MICE
    for(StuffIter it=me->roomp->stuff.begin();it!=me->roomp->stuff.end();++it){
      if((tm=dynamic_cast<TMonster *>(*it)) && tm->getRace()==RACE_RODENT &&
	 tm->getRealLevel() < me->getRealLevel()){
	me->doAction(add_bars(tm->name), CMD_GROWL);
	return tm->takeFirstHit(*me);
      }
    }

    // no mice, do some other lower priority stuff.
    switch(::number(0,10)){
      case 0:
	// clean
	act("$n begins cleaning $mself.", TRUE, me, NULL, NULL, TO_ROOM);
	break;
      case 1:
	// stare at something
	if((me->roomp->isIndoorSector() || 
	    me->roomp->isRoomFlag(ROOM_INDOORS))){
	  act("$n stares intently at a spot on the $g.",
	      TRUE, me, NULL, NULL, TO_ROOM);
	} else {
	  act("$n stares intently as a bird flies overhead.",
	      TRUE, me, NULL, NULL, TO_ROOM);
	}
	break;
      case 2:
	// sprint out of room
	me->doFlee("");
	break;
      case 3:
	// rub against leg if player (if humanoid?)
	for(StuffIter it=me->roomp->stuff.begin();it!=me->roomp->stuff.end();++it){
	  if((tb=dynamic_cast<TBeing *>(*it)) && tb->isHumanoid()){
	    roompeople.push_back(tb);
	  }
	}
	
	if(roompeople.size()>0){
	  tb=roompeople[::number(0,roompeople.size()-1)];
	  act("$n rubs against $N's leg.",
	      TRUE, me, NULL, tb, TO_NOTVICT);
	  act("$n rubs against your leg.",
	      TRUE, me, NULL, tb, TO_VICT);
	}
	break;
      case 4:
	// sleep
	af.type = AFFECT_DUMMY;
	af.level = 1;
	af.duration = 15;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_SLEEP;
	me->affectJoin(NULL, &af, AVG_DUR_NO, AVG_EFF_NO);
	me->doSleep("");
	break;
      case 5: 
	me->doAction("", CMD_STRETCH);
	break;
      case 6:
	for(StuffIter it=me->roomp->stuff.begin();it!=me->roomp->stuff.end();++it){
	  if((o=dynamic_cast<TObj *>(*it))){
	    roomobj.push_back(o);
	  }
	}
	
	if(roomobj.size()>0){
	  o=roomobj[::number(0,roomobj.size()-1)];
	  act("$n sharpens $s claws on $p.",
	      TRUE, me, o, NULL, TO_ROOM);
	}
	
	break;
      case 7: case 8: case 9: case 10:
	// play with other cats
        for(StuffIter it=me->roomp->stuff.begin();it!=me->roomp->stuff.end();++it){
          if((tb=dynamic_cast<TBeing *>(*it)) && 
	     (tb->getRace() == RACE_FELINE) &&
	     tb!=me){
            roompeople.push_back(tb);
          }
        }

        if(roompeople.size()>0){
          tb=roompeople[::number(0,roompeople.size()-1)];

	  switch(::number(0,3)){
	    case 0:
	      me->doAction(add_bars(tb->name), CMD_TACKLE);
	      break;
	    case 1:
	      me->doAction(add_bars(tb->name), CMD_NUZZLE);
	      break;
	    case 2:
	      me->doAction(add_bars(tb->name), CMD_LICK);
	      break;
	    case 3:
	      me->doAction(add_bars(tb->name), CMD_BITE);
	      break;
	  }
	}
	break;
    }
  }
    
  return TRUE;
}

int beeDeath(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *) {
  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  if (!ch->fight())
    return FALSE;

  TBeing *vict;
  vict = ch->fight();
  
  if (!vict->isAffected(AFF_ENGAGER) && number(0,10) > 7) {
    act("$n darts directly toward your head!",TRUE,ch,NULL,vict,TO_VICT,NULL);  
    act("$n misses and smacks into the ground.",TRUE,ch,NULL,vict,TO_VICT,NULL);
    act("$n flies directly at $N's head, misses, and smacks into the ground.",TRUE,ch,NULL,vict,TO_NOTVICT,NULL);
    act("A bee is dead! R.I.P.",TRUE,ch,NULL,vict,TO_ROOM,NULL);
    act("A bee is dead! R.I.P.",TRUE,ch,NULL,vict,TO_CHAR,NULL);  
    myself->makeCorpse(DAMAGE_NORMAL); // generic type for phony corpse 
  }
  return TRUE;
}

int brickCollector(TBeing *ch, cmdTypeT cmd, const char *argument, TMonster *myself, TObj *o) {
  sstring buf, arg=argument;
  TDatabase db(DB_SNEEZY);

  if(!ch || !ch->awake() || ch->fight())
    return FALSE;

  
  switch(cmd){
    case CMD_MOB_GIVEN_ITEM:
      if(!o || !isname("foundbrick", o->name)){
	return FALSE;
      }

      db.query("select 1 from brickquest where name='%s'", ch->name);
      if(!db.fetchRow()){
	db.query("insert into brickquest values (1, '%s')", 
		 ch->name);
      } else {
	db.query("update brickquest set numbricks=numbricks+1 where name='%s'", ch->name);
      }
      db.query("select name, numbricks from brickquest where name='%s'", ch->name);
      while (db.fetchRow()) {
	buf = format("Thanks %s! That makes your total %i bricks. I will update the scores.") % db["name"] % 
convertTo<int>(db["numbricks"]);
	myself->doSay(buf);
	buf = format("Gauge has won the last brick quest on 7-1-2006. Yay!");
	myself->doSay(buf);
        // vlogf(LOG_JESUS, format("%s turned in another brick for a total of %i") % ch->name % convertTo<int>(db["numbricks"]));
      }
      ch->doSave(SILENT_YES);
      return DELETE_ITEM;
      break;
    default:
      return FALSE;
  }

  return TRUE;
}

int caretaker(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  int vnums[3]={33775, 33775, 33777};
  TObj *o;
  bool found=false;

  if(cmd!=CMD_GENERIC_PULSE || !me || !me->roomp)
    return FALSE;

  // pick stuff up
  for(StuffIter it=me->roomp->stuff.begin();it!=me->roomp->stuff.end();++it){
    if((o=dynamic_cast<TObj *>(*it))){
      for(int i=0;i<3;++i){
	if(o->objVnum() == vnums[i]){
	  if(me->doGet(add_bars(o->name).c_str()))
	    found=true;
	}
      }
    }
  }
  
  // drop stuff
  if(!found){
    for(StuffIter it=me->stuff.begin();it!=me->stuff.end();++it){
      if((o=dynamic_cast<TObj *>(*it))){
	for(int i=0;i<3;++i){
	  if(o->objVnum() == vnums[i]){
	    if(me->doDrop("", o))
	      found=true;
	  }
	}
      }
    }
  }

  if(found)
    me->wanderAround();

  return TRUE;
}


int cannonLoader(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  if(!((cmd == CMD_SAY || cmd == CMD_SAY2) && ch &&
       (ch->desc->account->name == "trav" ||
	ch->desc->account->name == "scout" ||
	ch->desc->account->name == "laren" ||
	ch->desc->account->name == "ekeron")))
    return FALSE;
  
  sstring argument=arg;

  if(argument.lower() != "load the cannons!")
    return FALSE;

  me->doGload("cannon-iron-dragon");
  
  return FALSE;
}

int idCardProvider(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  int amt;

  if(cmd != CMD_MOB_GIVEN_COINS)
    return FALSE;

  amt=(int)o;

  if(amt < 50){
    me->doSay("Uh... thanks for the tip.  If you want an ID card it'll be 50 talens.");
    return FALSE;
  }

  struct time_info_data birth_data;
  GameTime::mudTimePassed(ch->player.time->birth, GameTime::getBeginningOfTime(), &birth_data);
  birth_data.year += GameTime::getYearAdjust();
  birth_data.year -= ch->getBaseAge();
  
  int day = birth_data.day + 1;        // day in [1..35] 
  
  sstring dob=format("%s %s, %d") % 
    month_name[birth_data.month] % 
    numberAsString(day) % 
    birth_data.year;

  sstring buf="";

  buf += format("+-------------------------------------+\n\r");
  buf += format("|                                     |\n\r");
  buf += format("|        Kingdom of Grimhaven         |\n\r");
  buf += format("|            Official ID              |\n\r");
  buf += format("|                                     |\n\r");
  buf += format("|  ID Number     :  %-18i|\n\r") % ch->getPlayerID();
  buf += format("|  Name          :  %-18s|\n\r") % ch->getName();
  buf += format("|  DOB           :  %-18s|\n\r") % dob;
  buf += format("|  Profession    :  %-18s|\n\r") % ch->getProfName();
  buf += format("|                                     |\n\r");
  buf += format("+-------------------------------------+\n\r");

  TNote *card = createNote(mud_str_dup(buf));
  delete [] card->name;
  card->name = mud_str_dup("card paper id");
  delete [] card->shortDescr;
  card->shortDescr = mud_str_dup("<W>an ID card<1>"); 
  delete [] card->getDescr();
  card->setDescr(mud_str_dup("<W>An official Kingdom of Grimhaven ID card lies here.<1>"));

  *me += *card;
  me->doSay("Alright, here you are!");
  me->doGive(format("%s %s") % add_bars(card->name) % add_bars(ch->name),
	     GIVE_FLAG_IGN_DEX_TEXT);
  return FALSE;
}

int rationFactory(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  TDatabase db(DB_SNEEZY);
  int factory_shop=251;
  TShopOwned tso(factory_shop, ch);
  TFood *food;
  int maxsupply=36*1000;

  // the cost of the foodstuff for rations and provs is 1.85 and 2.5
  // respectively.  i precalculated this on current prices (cheating).
  // since the factory will produce one of each production pulse,
  // then we can average these prices to get the actual supply cost.
  float base_price=2.175; 

  if(cmd != CMD_MOB_GIVEN_ITEM)
    return FALSE;

  db.query("select supplyamt from factorysupplies where shop_nr=%i and supplyname='meat'", factory_shop);
  
  if(!db.fetchRow()){
    vlogf(LOG_BUG, "ration factory missing db entry");
    return FALSE;
  }
	
  if(convertTo<int>(db["supplyamt"]) >= maxsupply){
    me->doSay("I have enough meat right now.");
    me->doDrop("", o);
    return FALSE;
  }

  if(!o || !o->objVnum()==Obj::GENERIC_STEAK || 
     !(food=dynamic_cast<TFood *>(o))){
    me->doSay("What the hell is this?!  That's not going into a ration.");
    me->doDrop("", o);
    return FALSE;
  }

  float price=base_price * shop_index[factory_shop].getProfitSell(o, ch);
  price *= food->getFoodFill();

  me->doSay("Now that's a nice cut of steak!  Here you go.");
  me->doEmote(format("hands you %i talens.") % (int)price);
  
  tso.doSellTransaction((int)price, "meat",
			TX_SELLING,  food->getFoodFill());

  db.query("update factorysupplies set supplyamt=supplyamt+%i where shop_nr=%i and supplyname='meat'", food->getFoodFill(), factory_shop);

  return TRUE;
}



// janitors and trash collectors etc, in spec_mobs_janitors.cc
extern int janitor(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int prisonJanitor(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int amberJanitor(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int brightmoonJanitor(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int garbageConvoy(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int fruitScavenger(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);

// vehicle related procs, mostly in spec_mobs_vehicle.cc
extern int fishingBoatCaptain(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int casinoElevatorOperator(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int casinoElevatorGuard(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int trolleyBoatCaptain(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int shipCaptain(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);


// combat procs, these are (mostly) in spec_mobs_combat.cc
extern int vampire(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int arch_vampire(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int rock_worm(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int rust_monster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int paralyzeBite(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int paralyzeBreath(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int Fireballer(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int Teleporter(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int MSwarmer(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int IceStormer(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int Edrain(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int LBolter(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int Disser(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int Witherer(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int Paralyzer(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int AcidBlaster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int kingUrik(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int ram(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int kraken(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int ascallion(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int electricEel(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int poisonHit(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int belimus(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int paralyzeGaze(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int poisonBite(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int targetDummy(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);

// misc external procs
extern int commodTrader(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int loanManager(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int auctioneer(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int propertyClerk(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int signMaker(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int paladinPatrol(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int leperHunter(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int taxman(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int banker(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int centralBanker(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int loanShark(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int statSurgeon(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int fireman(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int flaskPeddler(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int limbDispo(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int corporateAssistant(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int postman(TBeing *, cmdTypeT , const char *, TMonster *, TObj *);
extern int holdemPlayer(TBeing *, cmdTypeT cmd, const char *, TMonster *, TObj *);
extern int cityguard(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *);
extern int tudy(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *);
extern int moneyTrain(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *);
extern int guildRegistrar(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int grimhavenPosse(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int coroner(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int lotteryRedeemer(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int GenericGuildMaster(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *);

// Fields: display_under_medit, name_of_special, name_of_function_to_call
TMobSpecs mob_specials[NUM_MOB_SPECIALS + 1] =
{
  {TRUE, "BOGUS", bogus_mob_proc},        // 0 
  {TRUE, "energy drainer", vampire},
  {TRUE, "arch vampire", arch_vampire},
  {FALSE, "dragon breath", DragonBreath},
  {TRUE, "thief", thief},
  {TRUE, "janitor", janitor},        // 5 
  {FALSE, "tormentor", tormentor},
  {FALSE, "Trainer: air", CDGenericTrainer},
  {TRUE, "chicken", chicken},
  {TRUE, "thrower mob", ThrowerMob},
  {TRUE, "swallower", Tyrannosaurus_swallower},                // 10 
  {TRUE, "bounty hunter", bounty_hunter},
  {FALSE, "alignment god", alignment_deity},
  {TRUE, "engraver", engraver},
  {FALSE, "cityguard", cityguard},
  {FALSE, "dagger thrower", dagger_thrower},        // 15 
  {TRUE, "mount", horse},
  {TRUE, "beggar", beggar},
  {FALSE, "pet keeper", pet_keeper},
  {FALSE, "stable man", stable_man},
  {FALSE, "famine", famine},        // 20 
  {FALSE, "war", war},
  {TRUE, "cold giver", cold_giver},
  {TRUE, "flu giver", flu_giver},
  {FALSE, "craps man", craps_table_man},
  {FALSE, "shop keeper", shop_keeper},        // 25 
  {TRUE, "postmaster", postmaster},
  {TRUE, "receptionist", receptionist},
  {FALSE, "rock worm", rock_worm},        
  {FALSE, "mage guildmaster", GenericGuildMaster},
  {FALSE, "deikhan guildmaster", GenericGuildMaster},        // 30 
  {FALSE, "monk guildmaster", GenericGuildMaster},
  {FALSE, "warrior guildmaster", GenericGuildMaster},
  {FALSE, "thief guildmaster", GenericGuildMaster},
  {FALSE, "cleric guildmaster", GenericGuildMaster},
  {FALSE, "ranger guildmaster", GenericGuildMaster},        // 35 
  {FALSE, "hobbit emissary", hobbitEmissary},
  {FALSE, "Trainer: alchemy", CDGenericTrainer},
  {FALSE, "Trainer: earth", CDGenericTrainer},
  {TRUE, "repairman", repairman},
  {TRUE, "sharpener", sharpener},        // 40 
  {TRUE, "rust monster", rust_monster},
  {TRUE, "paralyze bite", paralyzeBite},
  {FALSE, "caravan leader", caravan},
  {FALSE, "death", death},
  {FALSE, "pestilence", pestilence},        // 45 
  {FALSE, "firemaster", fireMaster},
  {TRUE, "paralyze breath", paralyzeBreath},
  {FALSE, "doctor", doctor},
  {FALSE, "Trainer: fire", CDGenericTrainer},
  {TRUE, "frostbiter", frostbiter},       // 50 
  {FALSE, "ticket-guy", TicketGuy},
  {FALSE, "fireballer",Fireballer},
  {FALSE,"teleporter",Teleporter},
  {FALSE,"meteorswarm",MSwarmer},
  {FALSE,"icestormer",IceStormer},     // 55 
  {FALSE,"energy drainer2",Edrain},
  {FALSE,"lightning bolter",LBolter},
  {FALSE,"disintigrater",Disser},
  {FALSE,"witherer",Witherer},
  {FALSE,"paralyzer2",Paralyzer},      // 60 
  {FALSE,"acid blaster",AcidBlaster},
  {FALSE,"king Urik",kingUrik},
  {FALSE,"Trainer: sorcery", CDGenericTrainer},
  {FALSE,"Trainer: spirit", CDGenericTrainer},
  {FALSE,"Trainer: water", CDGenericTrainer},   // 65 
  {FALSE,"Trainer: wrath", CDGenericTrainer},
  {FALSE,"Trainer: afflictions", CDGenericTrainer},
  {FALSE,"Trainer: cures", CDGenericTrainer},
  {FALSE,"Trainer: hand of god", CDGenericTrainer},
  {FALSE,"Trainer: ranger", CDGenericTrainer},    // 70 
  {FALSE,"replicant", replicant},
  {FALSE,"Trainer: looting", CDGenericTrainer},
  {FALSE,"Trainer: murder", CDGenericTrainer},
  {FALSE,"Trainer: hand-to-hand", CDGenericTrainer},
  {FALSE,"Trainer: adventuring", CDGenericTrainer},    // 75 
  {FALSE,"Trainer: warrior", CDGenericTrainer},
  {TRUE, "Banshee", banshee},        
  {FALSE,"Meeting Organizer", meeting_organizer},        
  {FALSE,"toll taker", payToll},
  {FALSE,"Trainer: wizardry", CDGenericTrainer},   // 80 
  {FALSE,"Trainer: faith", CDGenericTrainer},
  {FALSE,"Trainer: slash", CDGenericTrainer},
  {FALSE,"Trainer: blunt", CDGenericTrainer},
  {FALSE,"Trainer: pierce", CDGenericTrainer},
  {FALSE,"Trainer: ranged", CDGenericTrainer},    // 85 
  {TRUE,"Tunneler/Earthquake", tunnelerEarthquake},
  {FALSE,"Trainer: deikhan", CDGenericTrainer},
  {TRUE, "Ram", ram},
  {TRUE, "Insulter", insulter},
  {TRUE, "Kraken", kraken},      // 90 
  {FALSE, "ascallion", ascallion},
  {FALSE, "electric eel", electricEel},
  {FALSE, "poison hit", poisonHit},
  {FALSE, "Belimus", belimus},
  {FALSE, "Siren", siren},      // 95 
  {TRUE, "Lamp-Lighter", lamp_lighter},
  {TRUE, "Leper", leper},
  {TRUE, "Air Magi", specificDisc},
  {TRUE, "Water Magi", specificDisc},
  {TRUE, "Earth Magi", specificDisc},   // 100 
  {TRUE, "Fire Magi", specificDisc},
  {TRUE, "Sorcerer", specificDisc},
  {FALSE, "Faction-Faery", factionFaery},
  {FALSE,"Trainer: brawling", CDGenericTrainer},
  {FALSE,"Trainer: mage/thief", CDGenericTrainer},   // 105 
  {FALSE,"Trainer: meditation (monk)", CDGenericTrainer},
  {FALSE,"Trainer: survival", CDGenericTrainer},
  {FALSE,"Trainer: armadillo", CDGenericTrainer},
  {FALSE,"Trainer: animal", CDGenericTrainer},
  {FALSE,"Trainer: aegis", CDGenericTrainer},    // 110 
  {FALSE,"Trainer: shaman rites", CDGenericTrainer},
  {FALSE,"frost giant warrior", frostGiant},
  {FALSE,"librarian", librarian},
  {FALSE,"rumormonger", rumorMonger},
  {FALSE,"Trainer: mage", CDGenericTrainer},    // 115 
  {FALSE,"Trainer: monk", CDGenericTrainer},   
  {FALSE,"Trainer: cleric", CDGenericTrainer},
  {FALSE,"Trainer: thief", CDGenericTrainer},
  {FALSE,"Trainer: plants", CDGenericTrainer},
  {FALSE,"Trainer: soldiering", CDGenericTrainer},       // 120 
  {FALSE,"Trainer: blacksmithing", CDGenericTrainer}, 
  {FALSE,"Trainer: deikhan fight", CDGenericTrainer},
  {FALSE,"Trainer: mounted", CDGenericTrainer},
  {FALSE,"Trainer: deikhan aegis", CDGenericTrainer},
  {FALSE,"Trainer: deikhan cures", CDGenericTrainer},    // 125 
  {FALSE,"Trainer: deikhan wrath", CDGenericTrainer},
  {FALSE,"Trainer: leverage", CDGenericTrainer},
  {FALSE,"Trainer: mindbody", CDGenericTrainer},
  {FALSE, "grimhaven posse", grimhavenPosse},
  {FALSE, "grimhaven hooker", grimhavenHooker},       // 130 
  {FALSE,"Trainer: focused attacks", CDGenericTrainer},
  {TRUE,"Corpse Muncher", corpseMuncher},
  {FALSE,"Trainer: barehand", CDGenericTrainer},
  {FALSE,"Trainer: thief fight", CDGenericTrainer},
  {FALSE,"Trainer: poisons", CDGenericTrainer},    // 135 
  {FALSE,"Trainer: frog", CDGenericTrainer},
  {FALSE,"Trainer: shaman alchemy", CDGenericTrainer},
  {FALSE,"Trainer: skunk", CDGenericTrainer},
  {FALSE,"Trainer: spider", CDGenericTrainer},
  {FALSE,"Trainer: control", CDGenericTrainer},       // 140 
  {FALSE,"Trainer: ritualism", CDGenericTrainer},
  {FALSE,"paladin patrol", paladinPatrol},
  {FALSE,"shaman guildmaster", GenericGuildMaster},
  {FALSE,"Trainer: combat", CDGenericTrainer},
  {FALSE,"Trainer: stealth", CDGenericTrainer},     // 145 
  {FALSE,"Trainer: traps", CDGenericTrainer},       
  {FALSE,"Newbie Equipper", newbieEquipper},
  {FALSE,"Trainer: lore", CDGenericTrainer},
  {FALSE,"Trainer: theology", CDGenericTrainer},   
  {FALSE,"attuner", attuner},                      // 150 
  {TRUE,"paralyze gaze", paralyzeGaze},
  {TRUE,"Doppleganger/Mimic", doppleganger},
  {TRUE,"Tusker/Goring", tuskGoring},
  {FALSE,"Fish Tracker", fishTracker},
  {FALSE, "Bank Guard", bankGuard},               // 155
  {FALSE, "Fishing Boat Captain", fishingBoatCaptain},
  {FALSE, "Coroner", coroner},
  {FALSE, "Guild Registrar", guildRegistrar},
  {FALSE, "Trainer: defense", CDGenericTrainer},
  {FALSE, "Scared Kid", scaredKid},               // 160
  {TRUE, "Corporate Asistant", corporateAssistant},     
  {FALSE,"Trainer: psionics", CDGenericTrainer},
  {TRUE, "Divination Man", divman},
  {FALSE,"Trainer: healing abilities", CDGenericTrainer},
  {FALSE, "Gardener", gardener},                 // 165
  {FALSE, "Brightmoon Archer", bmarcher},
  {FALSE, "Money Train", moneyTrain},
  {FALSE, "Adventurer", adventurer},
  {FALSE, "Trainer: iron body", CDGenericTrainer},
  {FALSE, "Tudy", tudy},                           // 170
  {TRUE, "Barmaid", barmaid},
  {FALSE, "tattoo artist", tattooArtist},
  {FALSE, "casino elevator operator", casinoElevatorOperator},
  {FALSE, "casino elevator guard", casinoElevatorGuard},
  {FALSE, "commodity maker", commodMaker}, // 175
  {FALSE, "lottery redeemer", lotteryRedeemer},
  {FALSE, "konastis's guard", konastisGuard},
  {FALSE, "hold'em player", holdemPlayer},
  {FALSE, "postman", postman},
  {TRUE, "poison bite", poisonBite}, // 180
  {FALSE, "riddling tree", riddlingTree},
  {FALSE, "fireman", fireman},
  {TRUE, "mimic", mimic},
  {TRUE, "archer", archer},
  {FALSE, "peddler", flaskPeddler}, // 185
  {FALSE, "limb disposer", limbDispo},
  {FALSE, "stat surgeon", statSurgeon},
  {FALSE, "shipping official", shippingOfficial},
  {FALSE, "loan shark", loanShark},
  {FALSE, "trolley driver", trolleyBoatCaptain}, // 190
  {FALSE, "property clerk", propertyClerk},
  {FALSE, "banker", banker},
  {FALSE, "prison janitor", prisonJanitor},
  {TRUE, "cat", cat},
  {FALSE, "taxman", taxman}, // 195
  {FALSE,"Trainer: advanced adventuring", CDGenericTrainer},
  {FALSE, "amber janitor", amberJanitor},
  {FALSE, "brightmoon janitor", brightmoonJanitor},
  {FALSE, "garbage convoy", garbageConvoy},
  {FALSE, "signmaker", signMaker}, // 200
  {TRUE, "butler", receptionist},
  {FALSE, "leper hunter", leperHunter},
  {FALSE, "auctioneer", auctioneer},
  {FALSE, "loan manager", loanManager},
  {TRUE, "bee death", beeDeath}, // 205
  {FALSE, "hero faerie", heroFaerie},
  {FALSE,"Brick Collector", brickCollector},
  {FALSE, "caretaker", caretaker},
  {FALSE, "ship captain", shipCaptain},
  {FALSE, "aggro follower", aggroFollower}, // 210
  {FALSE, "central banker", centralBanker},
  {FALSE, "cannon loader", cannonLoader},
  {FALSE, "id card provider", idCardProvider},
  {TRUE, "Target dummy", targetDummy},
  {TRUE, "fruit scavenger", fruitScavenger}, // 215
  {FALSE, "commodity trader", commodTrader},
  {FALSE, "ration factory", rationFactory},
  {FALSE, "pet veterinarian", petVeterinarian},
// replace non-zero, bogus_mob_procs above before adding
};


