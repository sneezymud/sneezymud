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

#include "stdsneezy.h"
#include "combat.h"
#include "disease.h"
#include "disc_sorcery.h"
#include "disc_afflictions.h"
#include "disc_aegis.h"
#include "disc_earth.h"
#include "disc_water.h"
#include "disc_wrath.h"
#include "disc_alchemy.h"
#include "obj_component.h"
#include "paths.h"
#include "database.h"
#include "obj_base_corpse.h"
#include "obj_player_corpse.h"
#include "obj_symbol.h"
#include "obj_window.h"
#include "obj_general_weapon.h"
#include "obj_pool.h"
#include "obj_base_clothing.h"
#include "obj_bow.h"
#include "obj_trap.h"
#include "obj_table.h"
#include "obj_drinkcon.h"

#include <fstream.h>

const int GET_MOB_SPE_INDEX(int d)
{
  return (((d > NUM_MOB_SPECIALS) || (d < 0)) ? 0 : d);
}

// used as a conditional to find_path
int is_target_room_p(int room, void *tgt_room)
{
  return room == (long) tgt_room;
}

// used as a conditional to find_path
int find_closest_obj_by_name(int room, void *arg)
{
  return named_object_on_ground(room, arg);
}

// used as a conditional to find_path
int find_closest_mob_by_name(int room, void *arg)
{
  return named_mob_on_ground(room, arg);
}

// used as a conditional to find_path
int find_closest_being_by_name(int room, void *arg)
{
  return named_being_on_ground(room, arg);
}

// used as a conditional to find_path
int find_closest_water(int room, void *)
{
  TRoom *rp;
  TThing *t;
  rp = real_roomp(room);

  if (rp->isRiverSector())
    return TRUE;

  for (t = rp->getStuff(); t; t = t->nextThing) {
    if (t->spec == SPEC_FOUNTAIN)
      return TRUE;
    if (t->waterSource())
      return TRUE;
  }
  return FALSE;
}

// used as a conditional to find_path
int find_closest_police(int room, void *)
{
  TRoom *rp;
  TThing *t;
  rp = real_roomp(room);

  for (t = rp->getStuff(); t; t = t->nextThing) {
    TBeing *ch = dynamic_cast<TBeing *>(t);
    if (!ch)
      continue;
    if (ch->isPc() && !ch->isImmortal())
      return TRUE;
    if (ch->isPolice())
      return TRUE;
  }
  return FALSE;
}

// could be used by find_path
int named_object_on_ground(int room, void *c_data)
{
  return (searchLinkedList((char *) c_data, real_roomp(room)->getStuff(), TYPEOBJ) != NULL);
}

// could be used by find_path
int named_mob_on_ground(int room, void *c_data)
{
  return (searchLinkedList((char *) c_data, real_roomp(room)->getStuff(), TYPEMOB) != NULL);
}

// could be used by find_path
int named_being_on_ground(int room, void *c_data)
{
  return (searchLinkedList((char *) c_data, real_roomp(room)->getStuff(), TYPEBEING) != NULL);
}

// returns DELETE_THIS for this
// returns DELETE_VICT if the special kills t
// returns DELETE_ITEM for t2
int TMonster::checkSpec(TBeing *t, cmdTypeT cmd, const char *arg, TThing *t2)
{
  int rc;

  //  if (cmd == CMD_GENERIC_PULSE && spec == SPEC_BOUNTY_HUNTER)
  //    vlogf(LOG_DASH, "Bounty Hunter spec %d on %s called with CMD_GENERIC_PULSE (checkSpec)", spec, getName());

  if(inRoom() == ROOM_NOCTURNAL_STORAGE)
    return FALSE;
  // if we move them to hell, it's probably cause there is a problem with
  // the proc, so skip it.  Realize, this may let them leak memory since
  // the destroy message is not called...
  if (inRoom() == ROOM_HELL && spec != SPEC_TORMENTOR)
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

bool TMonster::isPolice() const
{
  int num = mobVnum();

  return (!isPc() && ((spec == SPEC_CITYGUARD) || 
                      (num == MOB_BOUNCER) || (num == MOB_BOUNCER2) ||
                      (num == MOB_BOUNCER_HEAD)));
}
  
int TMonster::npcSteal(TPerson *victim)
{
  char buf[160];
  TThing *t;

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
  for (t = roomp->getStuff(); t; t = t->nextThing) {
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

  sprintf(buf,"talens %s",fname(victim->name).c_str());
  return doSteal(buf, victim);
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

  } else if (cmd == CMD_SAY) {
    one_argument(arg, buf);

    if (strcmp(buf, "help"))
      return FALSE;
  }
  
  return FALSE;
}

int rumorMonger(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *o)
{
  char caFilebuf[256], buf[256], buf2[512];;
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
    vlogf(LOG_LOW, "Missing rumor file (%s) (%d)", caFilebuf, errno);
    return FALSE;
  }

  do {
    fgets(buf, 255, fp);
  } while (*buf == '#');

  if (sscanf(buf, "%d %d\n", &type, &room) != 2) {
    vlogf(LOG_LOW, "Bad rumor format line 1 (%s) %s", caFilebuf);
    fclose(fp);
    return FALSE;
  }
    
  if (!type) {
    vlogf(LOG_LOW, "Bad rumor type (%s) %s", caFilebuf, buf);
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
      vlogf(LOG_LOW, "No rumors (%s)", caFilebuf);
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
        ch->addToMoney(-type, GOLD_SHOP);
        myself->addToMoney(type, GOLD_SHOP);
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
      // the next (2nd) line contains the string we should say
      do {
        if (!fgets(buf, 255, fp)) {
          vlogf(LOG_LOW, "Missing string for list (%s)", caFilebuf);
          fclose(fp);
          return TRUE;
        }
      } while (*buf == '#');
      if (strlen(buf) >= 1)
        buf[strlen(buf) - 1] = '\0';

      sprintf(buf2, "%s %s", fname(ch->name).c_str(), buf);
      myself->doTell(buf2);
      fclose(fp);
      return TRUE;
    }

    // don't count the "list" line
    do {
      if (!fgets(buf, 255, fp)) {
        vlogf(LOG_LOW, "Missing string for list (%s)", caFilebuf);
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
      vlogf(LOG_LOW, "No rumors (%s)", caFilebuf);
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
      fgets(buf, 255, fp); // skip over list string
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

    sprintf(buf2, "%s %s", fname(ch->name).c_str(), buf);
    myself->doTell(buf2);
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
  char tmp_buf[256];
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
  
  one_argument(arg, buf);

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
      sprintf(tmp_buf, "%s You are not a cleric or deikhan. I am not that charitable.", ch->getName());
      me->doTell(tmp_buf);
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
    sprintf(buf, "%s I can no longer help you!", ch->getName());
    me->doTell(buf);
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
        sprintf(tmp_buf, "%s May these items serve you well.  Be more careful next time!", ch->getName());
        me->doTell(tmp_buf);
        found = 2;
      }
      break;
    case 2:
      if (!ch->hasClass(CLASS_CLERIC) &&  !ch->hasClass(CLASS_DEIKHAN)) {
        sprintf(tmp_buf, "%s You are not a cleric or deikhan. I only give symbols to those who will use them.", ch->getName());
        me->doTell(tmp_buf);
        return TRUE;
      }
      if (ch->affectedBySpell(AFFECT_NEWBIE)) {
        found = 1;
        break;
      } else {
        best = NULL;
        for (i = ch->getStuff(); i && !best; i = i->nextThing) {
          i->findSym(&best);

          for (j = i->getStuff(); j && !best; j = j->nextThing) {
            j->findSym(&best); 
          }
        }
        if (best) {
          sprintf(tmp_buf, "%s You already have a symbol.  I only help the the totally destitute.", ch->getName()); 
          me->doTell(tmp_buf);
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
        sprintf(tmp_buf, "%s May it serve you well. Be more careful next time!", ch->getName());
        me->doTell(tmp_buf);
        found = 2;
        if (ch->hasClass(CLASS_CLERIC))
          duration = max(1, (ch->GetMaxLevel() / 3)) * 48 * UPDATES_PER_MUDHOUR;
      }
      break;
    case 3:
      if (ch->hasClass(CLASS_MONK)) {
        sprintf(tmp_buf, "%s You are a monk.  I only help those who directly need the help.", ch->getName());
        me->doTell(tmp_buf);
        return TRUE;
      }
      for (i = ch->getStuff(); i; i = i->nextThing) {
        if (dynamic_cast<TGenWeapon *>(i)) {
          sprintf(tmp_buf, "%s You already have a weapon.  I only help the the totally destitute.", ch->getName());
          me->doTell(tmp_buf);
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
          if ((r_num = real_object(WEAPON_T_DAGGER)) >= 0) {
            obj = read_object(r_num, REAL);
            *ch += *obj;    // newbie dagger 
          } else {
            vlogf(LOG_BUG, "Problem in NewbieEquipper, dagger");
            return TRUE;
          }
        }
        act("$n smiles and hands $p to $N.", TRUE, me, obj, ch,TO_NOTVICT);
        act("$n smiles and hands $p to you.", TRUE, me, obj, ch,TO_VICT);
        sprintf(tmp_buf, "%s May it serve you well. Be more careful next time!", ch->getName());
        me->doTell(tmp_buf);
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
    vlogf(LOG_MISC,"%s was given newbie gear by %s case %d", ch->getName(), me->getName(), request);
    if (me->desc) {
      vlogf(LOG_MISC, "Switched god used newbieEquip  %s by %s", ch->getName() , me->getName());
    }
  } else if (found == 1) {
    sprintf(tmp_buf, "%s You just used my service.  Come back later and only if you haven't gotten other help.", ch->getName());
    me->doTell(tmp_buf);
    return TRUE;
  } else {
    vlogf(LOG_BUG, "Somehow something got through equipNewbie %s by %s", ch->getName(), me->getName());
  }
  return TRUE;
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

int vampire(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  if (ch->spelltask)
    return FALSE;

  if (ch->fight() && ch->sameRoom(*ch->fight())) {
    act("$n touches $N!", TRUE, ch, 0, ch->fight(), TO_NOTVICT);
    act("$n touches you in an attempt to suck away your energy!",
        TRUE, ch, 0, ch->fight(), TO_VICT);

    if (!ch->doesKnowSkill(SPELL_ENERGY_DRAIN))
      ch->setSkillValue(SPELL_ENERGY_DRAIN,120);

    return energyDrain(ch,ch->fight());
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
    if (in_room != room) {
      dir = find_path(in_room, is_target_room_p, (void *) room, trackRange(), 0);
      if (dir < MIN_DIR) {
        // unable to find a path 
        moveHorseman(this);

        return FALSE;
      }
      // This if statement prevents mounted from being stuck
      if (dir < MAX_DIR &&
          real_roomp(exitDir(dir)->to_room)->isRoomFlag(ROOM_INDOORS)) {
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

  switch (spec) {
    case SPEC_HORSE_PESTILENCE:
      horse_num = APOC_PESTHORSE;
      break;
    case SPEC_HORSE_WAR:
      horse_num = APOC_WARHORSE;
      break;
    case SPEC_HORSE_FAMINE:
      horse_num = APOC_FAMINEHORSE;
      break;
    case SPEC_HORSE_DEATH:
      horse_num = APOC_DEATHHORSE;
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
    if ((dir = find_path(in_room, is_target_room_p, (void *) horse->in_room, 
              trackRange(), 0)) < 0) {
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

int kraken(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *opp;

  if ((cmd != CMD_MOB_COMBAT) || !ch->awake())
    return FALSE;

  if ((opp = ch->fight()) && opp->sameRoom(*ch)) {
    act("$n sprays $N with an inky black cloud!", 
           1, ch, 0, opp, TO_NOTVICT);
    act("$n sprays you with an inky black cloud!", 1, ch, 0, opp, TO_VICT);
    if (!opp->affectedBySpell(SPELL_BLINDNESS)) {
      opp->sendTo("You have been BLINDED!\n\r");

      opp->rawBlind(myself->GetMaxLevel(), 
                    (myself->GetMaxLevel()/20 + 1) * UPDATES_PER_MUDHOUR,
                    SAVE_YES);
    }
    return TRUE;
  }
  return FALSE;
}

int insulter(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *vict = NULL;
  TThing *t;
  if ((cmd != CMD_GENERIC_PULSE) || !myself->awake())
    return FALSE;
  if (::number(0,3))
    return FALSE;

  if (myself->fight())
    vict = myself->fight();
  else {
    for (t = myself->roomp->getStuff(); t; t = t->nextThing) {
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

int ascallion(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int i;
  TBeing *vict;
  TMonster *mob;

  if ((cmd != CMD_MOB_COMBAT) || !me->awake())
    return FALSE;
  if (!(vict = me->fight()))
    return FALSE;
  if (!vict->sameRoom(*me))
    return FALSE;
  if (::number(0,10))
    return FALSE;

  act("$n spews forth young to protect her!",0, me, 0, 0, TO_ROOM);
  for (i = 0; i < dice(2,3);i++) {
    if (!(mob = read_mobile(MOB_ASCALLION,VIRTUAL))) {
      vlogf(LOG_PROC, "Bad mob in ascallion spec_proc");
      return FALSE;
    }
    *me->roomp += *mob;
    me->addFollower(mob);
    mob->reconcileDamage(vict,0,DAMAGE_NORMAL);
    mob->addHated(vict);
  }
  return TRUE;
}

int electricEel(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *vict;

  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;
  if (!(vict = myself->fight()))
    return FALSE;
  if (!vict->sameRoom(*myself))
    return FALSE;
  if (::number(0,10))
    return FALSE;

  int dam = dice(3,6);
  act("$n emits a shock into the water!", 0, myself, 0, 0, TO_ROOM);
  vict->sendTo("You've been fried!\n\r");
  act("$n has been fried!", 0, vict, 0, 0, TO_ROOM);

  if (myself->reconcileDamage(vict,dam, DAMAGE_ELECTRIC) == -1) {
    delete vict;
    vict = NULL;
  }

  return TRUE;
}

int poisonHit(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *vict;

  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;
  if (!(vict = myself->fight()))
    return FALSE;
  if (!vict->sameRoom(*myself))
    return FALSE;
  if (vict->isImmune(IMMUNE_POISON, 20))
    return FALSE;
  if (vict->affectedBySpell(SPELL_POISON))
    return FALSE;

  if (::number(1,10) > 3)
    return FALSE;

  affectedData aff;
  aff.type = SPELL_POISON;
  aff.level = myself->GetMaxLevel();
  aff.duration = (aff.level) * UPDATES_PER_MUDHOUR;
  aff.modifier = -20;
  aff.location = APPLY_STR;
  aff.bitvector = AFF_POISON;

  act("$n wraps $s suckers around $N!", 0, myself, 0, vict, TO_NOTVICT);
  act("$n wraps $s suckers around YOU!", 0, myself, 0, vict, TO_VICT);

  vict->sendTo("You've been poisoned!\n\r");
  vict->affectTo(&aff);

  return TRUE;
}

// These are {MobVNum, ToRoom, ChanceOfSwallow, ChanceOfDeath}
// if ChanceOfDeath == -1 then they cannot die from this mob.
const int SWALLOWER_TO_ROOM_PROC[][4] = {
  {12402, 13480, 33, 40},
  {27912, 28499, 80, -1}
};

// Increment if you add to the above.
const int MAX_SWALLOWER_TO_ROOM = 2;

int belimus(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int rc,
      targetSwallower = -1;
  TBeing *vict;
  TBeing *tmp;

  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  if (!(vict = myself->fight()))
    return FALSE;

  if (!vict->sameRoom(*myself))
    return FALSE;

  for (int swallowerIndex = 0; swallowerIndex < MAX_SWALLOWER_TO_ROOM; swallowerIndex++)
    if (SWALLOWER_TO_ROOM_PROC[swallowerIndex][0] == myself->mobVnum()) {
      targetSwallower = swallowerIndex;
      break;
    }

  if (targetSwallower == -1 ||
      targetSwallower >= MAX_SWALLOWER_TO_ROOM) {
    vlogf(LOG_PROC, "Mobile in belimus() proc that isn't hard coded.  [%s] [%d]",
          myself->getName(), myself->mobVnum());
    return FALSE;
  }

  if (::number(0, 100) > SWALLOWER_TO_ROOM_PROC[targetSwallower][2])
    return FALSE;

  act("$n lunges at $N with $s maw gaping wide!",
       FALSE, myself, 0, vict, TO_NOTVICT);
  act("$n lunges at you with $s maw gaping wide!",
       FALSE, myself, 0, vict, TO_VICT);

  if (vict->isLucky(levelLuckModifier(30))) {
    vict->sendTo("Thank your deities, You leap aside at the last moment!\n\r");
    act("$n leaps aside at the last moment!", FALSE, vict, 0,0, TO_ROOM);
    return TRUE;
  }

  if (myself->fight())
    myself->stopFighting();
  if (vict->fight())
    vict->stopFighting();

  act("$n screams as $e is swallowed whole!", FALSE, vict, 0,0, TO_ROOM);

  tmp = dynamic_cast<TBeing *>(vict->riding);
  if (tmp) {
    vict->dismount(POSITION_STANDING);
    --(*tmp);
    thing_to_room(tmp, SWALLOWER_TO_ROOM_PROC[targetSwallower][1]);
    act("$n's mawed corpse arrives tumbling down $N's throat!",
        FALSE, tmp, 0, myself, TO_ROOM);
    vlogf(LOG_PROC, "%s killed by belimus-swallow[%s] at %s (%d)",
          tmp->getName(), myself->getName(),
          tmp->roomp->getName(), tmp->inRoom());

    rc = tmp->die(DAMAGE_EATTEN);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete tmp;
      tmp = NULL;
    }
  }
  tmp = dynamic_cast<TBeing *>(vict->rider);
  if (tmp) {
    tmp->dismount(POSITION_STANDING);
    --(*tmp);
    thing_to_room(tmp, SWALLOWER_TO_ROOM_PROC[targetSwallower][1]);
    act("$n's mawed corpse arrives tumbling down $N's throat!",
        FALSE, tmp, 0, myself, TO_ROOM);
    tmp->rawKill(DAMAGE_EATTEN, myself);
    delete tmp;
    tmp = NULL;
  }
    
  --(*vict);
  thing_to_room(vict, SWALLOWER_TO_ROOM_PROC[targetSwallower][1]);

  vict->sendTo("You have been swallowed whole!!!!\n\r");

  if ((SWALLOWER_TO_ROOM_PROC[targetSwallower][3] != -1) &&
      (::number(1, 100) < SWALLOWER_TO_ROOM_PROC[targetSwallower][3])) {
    vict->sendTo("%s chomps down upon you, biting you in two!!!!\n\r", myself->getName());
    act("$n's mawed corpse arrives tumbling down $N's throat!",
        FALSE, vict, 0, myself, TO_ROOM);
    vlogf(LOG_PROC, "%s killed by Belimus-swallow[%s] at %s (%d)",
          vict->getName(), myself->getName(),
          vict->roomp->getName(), vict->inRoom());
    rc = vict->die(DAMAGE_EATTEN);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete vict;
      vict = NULL;
    }
    return TRUE;  
  }
  act("$n arrives tumbling down $N's throat screaming!",
        FALSE, vict, 0, myself, TO_ROOM);
  vict->doLook("", CMD_LOOK);
  return TRUE;
}

int siren(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int rc;
  TBeing *vict = NULL;
  roomDirData *exitp, *back;
  dirTypeT door;
  TRoom *rp, *rp2;
  TThing *t, *t2;

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

    for (t = rp2->getStuff();t;t = t2) {
      t2 = t->nextThing;
      vict = dynamic_cast<TBeing *>(t);
      if (!vict)
        continue;
      if (!vict->isPc() && !vict->desc)
        continue;
      if (!vict->awake())
        continue;
      if (vict->isImmortal() ||
          (vict->getSex() != SEX_MALE) ||
          vict->isImmune(IMMUNE_CHARM, myself->GetMaxLevel())) {
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

  for (t = myself->roomp->getStuff(); t; t = t->nextThing) {
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
    if (vict->isImmune(IMMUNE_CHARM, myself->GetMaxLevel()) ||
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
    aff.duration  =  8 * level * UPDATES_PER_MUDHOUR;

    vict->affectTo(&aff);
  }
  return TRUE;
}

int ram(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  TBeing * vict = myself->fight();
  if (!vict)
    return FALSE;

  if (!vict->sameRoom(*myself))
    return FALSE;
  if (vict->riding)
    return FALSE;
  if (vict->getPosition() > POSITION_STANDING)
    return FALSE;
  if (::number(0,5))
    return FALSE;

  int damage = dice(5,6);

  act("$n lowers $s head and charges at you!", 
             FALSE, myself, 0, vict, TO_VICT);
  act("$n lowers $s head and charges at $N!",
             FALSE, myself, 0, vict, TO_NOTVICT);
  if (!vict->isLucky(levelLuckModifier(myself->GetMaxLevel()))) {
    act("$n slams $s head into your midriff.",
             FALSE, myself, 0, vict, TO_VICT);
    act("$n slams $s head into $N's midriff.",
             FALSE, myself, 0, vict, TO_NOTVICT);
    if (myself->reconcileDamage(vict, damage, DAMAGE_RAMMED) == -1) {
      delete vict;
      vict = NULL;
      return TRUE;
    }
    myself->cantHit += myself->loseRound(1);
    vict->cantHit += vict->loseRound(2);

    vict->setPosition(POSITION_SITTING);
  } else {
    act("You sidestep quickly, and $n thunders by.   TORO!",
             FALSE, myself, 0, vict, TO_VICT);
    act("$N sidesteps quickly, and $n thunders by $M.   TORO!",
             FALSE, myself, 0, vict, TO_NOTVICT);
  }
  return TRUE;
}

int paralyzeBreath(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *v;
  affectedData aff;
  affectedData aff2;
  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  if (!(v = myself->fight()) || !v->sameRoom(*myself)) 
    return FALSE;

  if (v->isAffected(AFF_PARALYSIS) || myself->checkForSkillAttempt(SPELL_PARALYZE)) {
    return FALSE;
  }

  act("$n spits out some noxious fumes at $N.", 
              TRUE, myself, NULL, v, TO_NOTVICT);
  act("$n spits out some noxious fumes at you!", TRUE, myself, NULL, v, TO_VICT);
  act("You spit some noxious fumes out at $N.", TRUE, myself, NULL, v, TO_CHAR);

  if (v->isImmune(IMMUNE_PARALYSIS, myself->GetMaxLevel())) {
    act("Your immunity saves you.", false, v, 0, 0, TO_CHAR);
    act("$n's immunity saves $m.", false, v, 0, 0, TO_ROOM);
    return FALSE;
  }

  if(!v->isImmortal()){
    aff.type = SPELL_PARALYZE;
    aff.level = myself->GetMaxLevel();
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_PARALYSIS;

    // each update is a combat round long...
    
    aff.duration = min(10, number(1, aff.level));

    aff.modifier = 0;
    
    v->affectTo(&aff);
    // this should keep paralyze proc mobs from paralyzing the same person right when he wakes up 10-20-00 -dash
    aff2.type = AFFECT_SKILL_ATTEMPT;
    aff2.level = myself->GetMaxLevel();
    aff2.location = APPLY_NONE;
    aff2.bitvector = 0;
    aff2.duration = aff.duration + (::number(1,3)); //one round should be enough, might as well randomize it a ittle though
    aff2.modifier = SPELL_PARALYZE;
    myself->affectTo(&aff2);
  } else {
    v->sendTo("Good thing you are immortal.\n\r");
  }    

  return TRUE;
}

int paralyzeBite(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *v;
  affectedData aff;
  affectedData aff2;
  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  if (!(v = myself->fight()) || !v->sameRoom(*myself))
    return FALSE;

  if (v->isAffected(AFF_PARALYSIS)|| myself->checkForSkillAttempt(SPELL_PARALYZE)) {
    return FALSE;
  }

  act("You bite $N!", 1, myself, 0, v, TO_CHAR);
  act("$n bites you!", 1, myself, 0, v, TO_VICT);
  act("$n bites $N!", 1, myself, 0, v, TO_NOTVICT);

  if (v->isImmune(IMMUNE_PARALYSIS, myself->GetMaxLevel())) {
    act("Your immunity saves you.", false, v, 0, 0, TO_CHAR);
    act("$n's immunity saves $m.", false, v, 0, 0, TO_ROOM);
    return FALSE;
  }

  if(!v->isImmortal()){
    aff.type = SPELL_PARALYZE;
    aff.level = myself->GetMaxLevel();
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_PARALYSIS;
    aff.duration = min(10, number(1, aff.level));
    aff.modifier = 0;
    
    v->affectTo(&aff);
    // this should keep paralyze proc mobs from paralyzing the same person right when he wakes up 10-20-00 -dash
    aff2.type = AFFECT_SKILL_ATTEMPT;
    aff2.level = myself->GetMaxLevel();
    aff2.location = APPLY_NONE;
    aff2.bitvector = 0;
    aff2.duration = aff.duration + (::number(1,3)); //one round should be enough, might as well randomize it a ittle though
    aff2.modifier = SPELL_PARALYZE;
    myself->affectTo(&aff2);
  } else {
    v->sendTo("Good thing you are immortal.\n\r");
  }  

  return TRUE;
}

// Arch does two drains at once!
int arch_vampire(TBeing *ch, cmdTypeT cmd, const char *, TMonster *, TObj *)
{
  if ((cmd != CMD_MOB_COMBAT) || !ch->awake())
    return FALSE;

  if (ch->spelltask)
    return FALSE;

  if (ch->fight() && ch->fight()->sameRoom(*ch)) {
    act("$n bites $N!", 1, ch, 0, ch->fight(), TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, ch->fight(), TO_VICT);
    if (!ch->doesKnowSkill(SPELL_ENERGY_DRAIN))
      ch->setSkillValue(SPELL_ENERGY_DRAIN,2*ch->GetMaxLevel());

    energyDrain(ch,ch->fight());

#if 0
// energyDrain goes through spelltask stuff, so can't double cast it anymore
    if (ch->fight() && ch->fight()->sameRoom(*ch)) {
      energyDrain(ch,ch->fight());
    }
#endif

    return TRUE;
  }
  return FALSE;
}

static int rob_blind(TBeing *ch, TBeing *vict)
{
  // make all checks prohibiting stealing before coming in here
  TThing *t, *t2;
  char name[80], buf[160];
 
  if (ch->fight() || vict->fight())
    return FALSE;
  if ((ch->getRace() == RACE_HOBBIT) && (ch->getSkillValue(SKILL_STEAL) < 80))
    ch->setSkillValue(SKILL_STEAL, 80);

  for (t=vict->getStuff();t;t= t2) {
    t2 = t->nextThing;
    if (::number(0,4) || !ch->canSee(t))
      continue;
    strcpy(name, fname(t->name).c_str());
    sprintf(buf, "%s %s", name, fname(vict->name).c_str());
    if (ch->getRace() == RACE_HOBBIT) 
      act("$n says, \"Hey $N, I'm just going to borrow your $o for a bit.\"", TRUE, ch, t, vict, TO_ROOM);
    
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

  TThing *t1, *t2;
  for (t1 = ch->roomp->getStuff();t1; t1 = t2) {
    t2 = t1->nextThing;
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

bool okForJanitor(TMonster *myself, TObj *obj)
{
  // only things that can be taken, and that are not pools
  if (!obj->canWear(ITEM_TAKE) && !dynamic_cast<TPool *>(obj))
    return false;
  if (!myself->canSee(obj) || (obj->in_room == ROOM_DONATION))
    return false;

  // Don't let them try and get corpses that are being skinned.
  TBaseCorpse *corpse = dynamic_cast<TBaseCorpse *>(obj);
  if (corpse && corpse->isCorpseFlag(CORPSE_PC_SKINNING))
    return false;
  // nor sacrificing
  if (corpse && corpse->isCorpseFlag(CORPSE_SACRIFICE))
    return false;
  if (corpse && corpse->isCorpseFlag(CORPSE_PC_BUTCHERING))
    return false;

  // Dont let them loot pcorpses with stuff in it
  TPCorpse *tmpcorpse = dynamic_cast<TPCorpse *>(obj);
  if (tmpcorpse && tmpcorpse->getStuff())
    return false;

  // the value check means corpse can be res'd
  // pc corpses can't be res'd, so can't be looted
  // also give pcs a moment to loot
  if (corpse && (corpse->getCorpseFlags() == 0) &&
      (corpse->obj_flags.decay_time <= MAX_NPC_CORPSE_TIME - 1)) {
    if (corpse->getStuff()) {
      TThing *t3, *t4;
      for (t3 = corpse->getStuff(); t3; t3 = t4) {
        t4 = t3->nextThing;
        TObj *obj2 = dynamic_cast<TObj *>(t3);
        if (!obj2)
          return false;
        if (!obj2->canWear(ITEM_TAKE))
          return false;
        if (!myself->canSee(corpse))
          return false;

        // keep this from happening for clutter-search
        if (myself->sameRoom(*corpse))
          get(myself, obj2, corpse, GETOBJOBJ, true);
      }
    }
    // if nothing in the corpse, let them get the corpse
  }
  if (corpse && corpse->getStuff())
    return false;

  return true;
}

// used as a conditional to find_path
static int find_closest_clutter(int room, void *myself)
{
  if (room == ROOM_DONATION)
    return FALSE;

  TRoom *rp = real_roomp(room);
  if (!rp->inGrimhaven())
    return FALSE;

  TThing *t;
  for (t = rp->getStuff(); t; t = t->nextThing) {
    TObj * obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    if (!okForJanitor((TMonster *) myself, obj))
      continue;
    return TRUE;
  }
  return FALSE;
}

static int findSomeClutter(TMonster *myself)
{
  dirTypeT dir;
  int rc;

  dir = find_path(myself->inRoom(), find_closest_clutter, (void *) myself, myself->trackRange(), 0);

  if (dir >= MIN_DIR) {
    rc = myself->goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  }
  // no clutter found
#if 0
  rc = myself->wanderAround();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  else if (rc)
    return TRUE;

  return FALSE;
#else
  // lots of them piling up with nothing to do
  return DELETE_THIS;
#endif
}

int janitor(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int rc;
  char buf[256];  

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake() || ch->fight())
    return FALSE;

  if (::number(0,3))
    return FALSE;

  for (t = myself->roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;

    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    if (myself->inRoom() == ROOM_DONATION)
      break;

    if (!okForJanitor(myself, obj))
      continue;

    if (dynamic_cast<TPool *>(obj)){
      sprintf(buf, "$n mops up $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);
      delete obj;
    } else if (dynamic_cast<TBaseCorpse *>(obj)) {
      sprintf(buf, "$n disposes of $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);

      myself->roomp->playsound(SOUND_BRING_DEAD, SOUND_TYPE_NOISE);

      TThing *t;
      while ((t = obj->getStuff())) {
        (*t)--;
        *myself += *t;
      }
      delete obj;
    } else if (!obj->isObjStat(ITEM_PROTOTYPE)) {
      act("$n picks up some trash.", FALSE, myself, 0, 0, TO_ROOM);
      --(*obj);
      *myself += *obj; 
      if(obj->objVnum() == OBJ_PILE_OFFAL)
	delete obj;
    }
    return TRUE;
  }

  // we only get here if there is nothing in my room worth picking up

  if (myself->mobVnum() == MOB_SWEEPER || myself->mobVnum() == MOB_SWEEPER2) {
    if (myself->getStuff()) {
      rc = myself->doDonate();
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      }
      return TRUE;
    } else {
      rc = findSomeClutter(myself);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      }
      return TRUE;
    }
  }

  return FALSE;
}

// for use by janitors to drop stuff in donation 
// returns DELETE_THIS
int TBeing::doDonate()
{
  int room = ROOM_DONATION;
  dirTypeT dir;
  int rc;

  if (in_room != room) {
    if ((dir = find_path(in_room, is_target_room_p, (void *) room, trackRange(), 0)) < 0) {
      // unable to find a path 
      if (room >= 0) {
        doSay("Time for my coffee break");
        act("$n has left into the void.",0, this, 0, 0, TO_ROOM);
        --(*this);
        thing_to_room(this, room);
        act("$n comes back to work.", 0, this, 0, 0, TO_ROOM);
      }
      return FALSE;
    }
    rc = goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  } else {
    rc = doDrop("all" , NULL);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    // chance to purge the room of objects
    if (roomp->roomIsEmpty(FALSE) && !::number(0,34)) {
      TThing *t, *t2;
      for (t = roomp->getStuff(); t; t = t2) {
        t2 = t->nextThing;
        if (!dynamic_cast<TObj *> (t)) 
          continue;
        dynamic_cast<TObj *> (t)->purgeMe(this);
        // t is possibly invalid here.
      }
    }
    return DELETE_THIS;
  }
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

int fighter(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *)
{
  int rc = 0;
  TBeing *vict;

  if ((cmd != CMD_MOB_COMBAT) || !ch->awake())
    return (FALSE);

  if (ch->getWait() > 0)
    return FALSE;

  if ((vict = ch->fight())) {
    if (ch->getPosition() >= POSITION_SITTING) {
      rc = ch->fighterMove(*vict);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        delete vict;
        vict = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      if (rc)
        return rc;
    } else {
      rc = ch->standUp();
      if (rc)
        return rc;
    }

    rc = ch->findABetterWeapon();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
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
  int or;
  char buf[256];

  rp = v->roomp;
  if (rp && rp->dir_option[dir] &&
      rp->dir_option[dir]->to_room && 
      !IS_SET(rp->dir_option[dir]->condition, EX_CLOSED) &&
      (rp->dir_option[dir]->to_room != ROOM_NOWHERE)) {
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
      sendTo(COLOR_MOBS, "You push %s %s out of the room.\n\r", v->getName(), dirs[dir]);
      v->sendTo(COLOR_MOBS, "%s pushes you %s out of the room.\n\r", good_cap(getName()).c_str(), dirs[dir]);
      sprintf(buf, "$N is pushed %s out of the room by $n.", dirs[dir]);
      act(buf, TRUE, this, 0, v, TO_NOTVICT);
    }
    or = v->in_room;
    --(*v);
    if (also) {
      --(*this);
      thing_to_room(this, real_roomp(or)->dir_option[dir]->to_room);
    }
    thing_to_room(v, real_roomp(or)->dir_option[dir]->to_room);

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
      cmd != CMD_DRAG)
    return FALSE;

  if (cmd >= CMD_NORTH && cmd <= CMD_SW) {
    dir = getDirFromCmd(cmd);
  } else if (cmd == CMD_DRAG) {
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
    dir=dirTypeT(atoi_safe(arg));

  switch(myself->inRoom()){
    case 1024:
      if(!myself->canSee(ch) || myself==ch || ch->isAnimal() || 
         !myself->awake() || myself->fight()) {
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
    case 22713:
      if(rev_dir[dir]==ch->specials.last_direction){
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
         myself->fight()){
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

          vlogf(LOG_PROC, "%s killed by being swallowed at %s (%d)",
              targ->getName(), targ->roomp->getName(), targ->inRoom());
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
          if ((co = dynamic_cast<TBaseCorpse *>(ch->roomp->getStuff()))) {
            TThing *t;
            for (t = co->getStuff(); t; t = co->getStuff()) {
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
  TThing *t, *t2;
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
  for (t = ch->roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
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
    if ((door = find_path(myself->inRoom(), find_closest_police, (void *) NULL, -2000, 0)) > DIR_NONE) {
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
  lamp = searchLinkedListVis(ch, "lamppost", ch->roomp->getStuff());
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
    if (myself->in_room == ROOM_CS) {
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

    if (myself->in_room == ROOM_CS) {
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
    // vlogf(LOG_PROC, "Lampboy got lost ip: path: %d, pos: %d, room: %d, should: %d", job->cur_path, job->cur_pos, myself->in_room, lamp_path_pos[job->cur_path][job->cur_pos].cur_room);
    job->cur_pos = -1;
    do {
      job->cur_pos += 1;
      if (lamp_path_pos[job->cur_path][job->cur_pos].cur_room == myself->in_room)
        return TRUE;
    } while (lamp_path_pos[job->cur_path][job->cur_pos].cur_room != -1);
 
    act("$n seems to have gotten a little bit lost.",0, myself, 0, 0, TO_ROOM);
    act("$n goes to ask directions.", 0, myself, 0, 0, TO_ROOM);
    //vlogf(LOG_PROC, "Lampboy got lost: path: %d, pos: %d", job->cur_path, myself->in_room);
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
      mon = read_mobile(MOB_ROAD_THIEF_LARGE, VIRTUAL);
    } else if (wealth_per > cost_robber) {
      mon = read_mobile(MOB_ROAD_ROBBER, VIRTUAL);
    } else if (wealth_per > cost_thief) {
      mon = read_mobile(MOB_ROAD_THIEF, VIRTUAL);
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
      vlogf(LOG_PROC, "Bogus room load of caravan (%d)", myself->in_room);
      return DELETE_THIS;
    }
    FactionInfo[faction].caravan_attempts++;

    amount = min((long) FactionInfo[faction].caravan_value,
                  FactionInfo[faction].faction_wealth);
    job->wealth = amount;
    FactionInfo[faction].faction_wealth -= amount;

    // construct caravan
    if (!(obj = read_object(OBJ_CARAVAN, VIRTUAL))) {
      vlogf(LOG_PROC, "Problem with caravan load (1)");
      return TRUE;
    }
    *myself->roomp += *obj;

    obj->checkSpec(myself, CMD_OBJ_WAGON_INIT, "", NULL);

    job->caravan = obj;

    if (faction != FACT_NONE) {
      sprintf(buf, "A caravan has formed bound for %s.", 
             CaravanDestination(-faction - 1));
      sendToFaction(faction, myself->getName(), buf);
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
    FactionInfo[faction].faction_wealth += (200 * job->wealth / 100);
    FactionInfo[job->dest_fact].faction_wealth += (100 * job->wealth / 100);
    
    if (faction != FACT_NONE) {
      sprintf(buf, "A caravan has arrived successfully in %s.", CaravanDestination(-faction - 1));
      sendToFaction(faction, myself->getName(), buf);
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
    vlogf(LOG_PROC, "Caravan got lost ip: path: %d, pos: %d, room: %d, should: %d", job->cur_path, job->cur_pos, myself->in_room, caravan_path_pos[job->cur_path][job->cur_pos].cur_room);
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
    vlogf(LOG_PROC, "Caravan got lost: path: %d, pos: %d", job->cur_path, myself->in_room);
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
      if (vict) {
        tmons->setHunting(vict);
        i++;
      }
    }
  }
}

int dagger_thrower(TBeing *pch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TObj *dagger;
  TBeing *tmp_ch, *ch, *temp;
  int range;
  dirTypeT dir;
  char buf[128];

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
        int robj = real_object(GENERIC_DAGGER);
        if (robj < 0 || robj >= (signed int) obj_index.size()) {
          vlogf(LOG_BUG, "dagger_thrower(): No object (%d) in database!",
                GENERIC_DAGGER);
          return FALSE;
        }
    
        if (!(dagger = read_object(robj, REAL))) {
          vlogf(LOG_BUG, "Couldn't make a dagger for dagger_thrower()!");
          return FALSE;
        }
#else
        dagger = read_object(GENERIC_DAGGER, VIRTUAL);
#endif

        if (!me->equipment[HOLD_RIGHT])
          me->equipChar(dagger, HOLD_RIGHT);
        else {
          vlogf(LOG_BUG, "Dagger_thrower problem: equipped right hand.  %s at %d", me->getName(), me->inRoom());
          delete dagger;
          return FALSE;
        }

        sprintf(buf, "%s %s %d", fname(dagger->name).c_str(), tmp_ch->name, 5);
        me->doThrow(buf);
        return TRUE;
      }
    }
  }
  return TRUE;
}

// XXX
int horse(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TBeing *vict;
  int rc;
  TObj *obj=NULL;

  if (cmd == CMD_GENERIC_PULSE){
    if (!::number(0,500) && me->roomp && gamePort == PROD_GAMEPORT) {
      obj = read_object(OBJ_PILE_OFFAL, VIRTUAL);

      if (obj) {
        *me->roomp += *obj;
        act("$n <o>defecates<z> on the $g.",
             TRUE, me, NULL, NULL, TO_ROOM);
      }
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

int TMonster::petPrice() const
{
  int price;
  if (GetMaxLevel() <= 5) {
    price = (((GetMaxLevel())*15) + 100);
  } else if (GetMaxLevel() <= 12)  {
    price = (GetMaxLevel()*GetMaxLevel()*25);
  } else if (GetMaxLevel() <= 17) {
    price = (GetMaxLevel()*GetMaxLevel()*35);
  } else if (GetMaxLevel() <= 21) {
    price = (GetMaxLevel()*GetMaxLevel()*45);
  } else if (GetMaxLevel() <= 28) {
    price = (GetMaxLevel()*GetMaxLevel()*60);
  } else  {
    price = (GetMaxLevel()*GetMaxLevel()*90);
  }
  // they make great tanks
  if (canFly())
    price *= 3;

  return price;
}

static TWindow * getFirstWindowInRoom(TMonster *myself)
{
  TThing *t;
  TWindow *tw = NULL;
  for (t = myself->roomp->getStuff(); t; t = t->nextThing) {
    tw = dynamic_cast<TWindow *>(t);
    if (tw)
      return tw;
  }
  return NULL;
}

int pet_keeper(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  char buf[256];
  TRoom *rp;
  int price;
  affectedData aff;

  if (cmd >= MAX_CMD_LIST)
    return FALSE;

  switch (me->mobVnum()) {
    case MOB_PETGUY_GH:
      rp = real_roomp(ROOM_PETS_GH);
      break;
    case MOB_PETGUY_BM:
      rp = real_roomp(ROOM_PETS_BM);
      break;
    case MOB_PETGUY_AMB:
      rp = real_roomp(ROOM_PETS_AMB);
      break;
    case MOB_PETGUY_LOG:
      rp = real_roomp(ROOM_PETS_LOG);
      break;
    default:
      vlogf(LOG_PROC, "Bogus mob in petguy proc");
      return FALSE;
  }
  if (!rp) {
    vlogf(LOG_PROC, "Pet keeper spec_proc called with no pet room!");
    return FALSE;
  }
  if (cmd == CMD_LIST) {
    TWindow *tw = getFirstWindowInRoom(me);

    string tellBuf = fname(ch->name);
    tellBuf += " Look through the ";
    tellBuf += tw ? fname(tw->name) : "window";
    tellBuf += " to see the pets!";
    me->doTell(tellBuf.c_str());

    sprintf(buf, "%s If you see something you'd like, VALUE <mob> and I'll tell you the price.", ch->getName());
    me->doTell(buf);
    return TRUE;
  } else if (cmd == CMD_BUY) {
    arg = one_argument(arg, buf);
    price = 0;
    
    TBeing *tbt = get_char_room(buf, rp->number);
    TMonster *pet = dynamic_cast<TMonster *>(tbt);
    if (!pet) {
      TWindow *tw = getFirstWindowInRoom(me);

      string tellBuf = fname(ch->name);
      tellBuf += " Look through the ";
      tellBuf += tw ? fname(tw->name) : "window";
      tellBuf += " again, there is no such pet!";
      me->doTell(tellBuf.c_str());

      return TRUE;
    }

    if (pet->desc || pet->isPc() || pet->number < 0) {
      sprintf(buf, "%s %s is not for sale.", fname(ch->name).c_str(), pet->getName());
      me->doTell(buf);
      return TRUE;
    }
    int petLevel = pet->GetMaxLevel();
    int pcLevel = ch->GetMaxLevel();

    if (ch->isImmortal()) {
    } else if (!ch->hasClass(CLASS_RANGER)) {
      if ((4 * petLevel) > (3 * pcLevel)) {
        sprintf(buf, "%s I think I would be negligent if I sold you so powerful
a pet.", fname(ch->name).c_str());
        me->doTell(buf);
        return TRUE;
      }
    } else {
      if (petLevel > pcLevel) {
        sprintf(buf, "%s I think I would be negligent if I sold you so powerful
a pet.", fname(ch->name).c_str());
        me->doTell(buf);
        return TRUE;
      }
    }
    if (ch->tooManyFollowers(pet, FOL_PET)) {
      sprintf(buf, "%s With your charisma, it would be animal abuse for me to sell you this pet.", fname(ch->name).c_str()); 
      me->doTell(buf);
      return TRUE;
    }

    price = pet->petPrice();

    if (ch->isPc() && ((ch->getMoney()) < price) && !ch->isImmortal()) {
      sprintf(buf, "%s You don't have enough money for that pet!", ch->name);
      me->doTell(buf);
      return TRUE;
    }
    if (!ch->isImmortal())
      ch->addToMoney(-(price), GOLD_SHOP_PET);

    if (!(pet = read_mobile(pet->number, REAL))) {
      vlogf(LOG_PROC, "Whoa!  No pet in pet_keeper");
      return TRUE;
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
    arg = one_argument(arg, buf);
    price = 0;
    TBeing *tbt = get_char_room(buf, rp->number);
    TMonster *pet = dynamic_cast<TMonster *>(tbt);

    if (!pet) {
      TWindow *tw = getFirstWindowInRoom(me);

      string tellBuf = fname(ch->name);
      tellBuf += " Look through the ";
      tellBuf += tw ? fname(tw->name) : "window";
      tellBuf += " again, there is no such pet!";
      me->doTell(tellBuf.c_str());
      return TRUE;
    }
    int petLevel = pet->GetMaxLevel();
    int pcLevel = ch->GetMaxLevel();

    price = pet->petPrice();
    sprintf(buf, "%s A pet %s will cost %d to purchase",
            ch->name, fname(pet->name).c_str(), price);
    me->doTell(buf);
    sprintf(buf, "%s and %d to rent.",
            ch->name, (pet->petPrice() / 4));
    me->doTell(buf);
    if (ch->isImmortal()) {
    } else if (!ch->hasClass(CLASS_RANGER)) {
      if ((4 * petLevel) > (3 * pcLevel)) {
        sprintf(buf, "%s I think I would be negligent if I sold you so powerful a pet.", fname(ch->name).c_str());
        me->doTell(buf);
      }
    } else {
      if (petLevel > pcLevel) {
        sprintf(buf, "%s I think I would be negligent if I sold you so powerful a pet.", fname(ch->name).c_str());
        me->doTell(buf);
      }
    }
    if (ch->tooManyFollowers(pet, FOL_PET)) {
      sprintf(buf, "%s With your charisma, it would be animal abuse for me to sell you this pet.", fname(ch->name).c_str());
      me->doTell(buf);
    }
    return TRUE;
  }
  return FALSE;
}


int stable_man(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  char buf[1024];

  if (cmd >= MAX_CMD_LIST)
    return FALSE;

  if (cmd == CMD_LIST) {
    TWindow *tw = getFirstWindowInRoom(me);

    sprintf(buf, "%s Look through %s to see the mounts!",
            fname(ch->name).c_str(), tw ? fname(tw->name).c_str() : "window");
    string tellBuf = fname(ch->name);
    tellBuf += " Look through the ";
    tellBuf += tw ? fname(tw->name) : "window";
    tellBuf += " to see the mounts!";
    me->doTell(tellBuf.c_str());

    return TRUE;
  }
  return FALSE;
}

static int attunePrice(const TSymbol *obj)
{
  if (!obj->getSymbolMaxStrength())
    return max(1, obj->obj_flags.cost / 50);
  else {
    float fract = (float) obj->getSymbolCurStrength() / obj->getSymbolMaxStrength();
    return max(1, (int) (obj->obj_flags.cost * fract));
  }
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
    TObj *tch;
    for (tch = object_list; tch && tch != job->sym; tch = tch->next);
    if (!tch) {
      // chances are, what job->sym points at is deleted memory, so do NOT
      // reference it
      vlogf(LOG_PROC, "Attuner lost symbol being attuned.");
      job->clearAttuneData();
    }
  }
}

void TThing::attunerValue(TBeing *ch, TMonster *me)
{
  char buf[256];

  sprintf(buf, "%s I can only attune symbols.", ch->getName());
  me->doTell(buf);
}

void TSymbol::attunerValue(TBeing *ch, TMonster *me)
{
  int cost;
  char buf[256];

  if (getSymbolFaction() != FACT_UNDEFINED) {
    sprintf(buf, "%s %s has already been attuned!", ch->getName(), getName());
    me->doTell(buf);
    return;
  }
  cost = attunePrice(this);

  sprintf(buf, "%s I will tithe you %d talens to attune your %s.",
         ch->getName(), cost, getName());
  me->doTell(buf);
}

void TThing::attunerGiven(TBeing *ch, TMonster *me)
{
  char buf[256];

  sprintf(buf, "%s, I can only attune symbols!", ch->getName());
  me->doTell(buf);
  strcpy(buf, name);
  add_bars(buf);
  sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
  me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT);
}

void TSymbol::attunerGiven(TBeing *ch, TMonster *me)
{
  int cost;
  char buf[256];
  attune_struct *job;

  if (getSymbolFaction() != FACT_UNDEFINED) {
    sprintf(buf, "%s, That symbol has already been attuned!", ch->getName());
    me->doTell(buf);
    strcpy(buf, name);
    add_bars(buf);
    sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
    me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
    return;
  }
  cost = attunePrice(this);

  if (ch->getMoney() < cost) {
    sprintf(buf, "%s, I only attune for a reasonable tithe. I am sorry, I do not make exceptions!", ch->getName());
    me->doTell(buf);
    strcpy(buf, name);
    add_bars(buf);
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
    ch->addToMoney(-cost, GOLD_SHOP_SYMBOL);

    job->cost = cost;
    job->hasJob = TRUE;
    job->pc = ch;
    job->sym = this;
    vlogf(LOG_SILENT, "%s gave %s to be attuned.", ch->getName(), getName());
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
    add_bars(buf);
    sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
    me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT);
    return;
  }
  return;
}
int attuner(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  char buf[256];
  TThing *t;
  TBeing *final_pers;
  TThing *ttt;
  TBeing *tbt = NULL;
  dirTypeT dir = DIR_NONE;
  roomDirData *exitp;
  int rc, found = FALSE;

  attune_struct *job;

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete (attune_struct *) me->act_ptr;
    me->act_ptr = NULL;
    return FALSE;
  } else if (cmd == CMD_GENERIC_INIT) {
    return FALSE;
  } else if (cmd == CMD_GENERIC_CREATED) {
    if (!me->hasClass(CLASS_CLERIC) && !me->hasClass(CLASS_DEIKHAN)) {
      vlogf(LOG_LOW, "Attuner %s is not a deikhan or cleric.", me->getName());
    }
    if (!(me->act_ptr = new attune_struct())) {
      perror("failed new of attuner.");
      exit(0);
    }
    return FALSE;
  }

  if (!(job = (attune_struct *) me->act_ptr)) {
    vlogf(LOG_PROC,"ATTUNER PROC ERROR/MobPulse: terminating (hopefully) cmd=%d", cmd);
    return FALSE;
  }

  // sanity check
  attuneStructSanityCheck(job);

  if (!job->hasJob) {
    if (job->sym || job->pc || job->wait || 
                    job->cost || (job->faction > FACT_UNDEFINED)) {
      if (job->pc && job->pc->name)
        vlogf(LOG_PROC, "Attuner (%s) seems to have a bad job structure (case 1) see %s.", me->getName(), job->pc->getName());
      else
        vlogf(LOG_PROC, "Attuner (%s) seems to have a bad job structure (case 1A).", me->getName());
      job->clearAttuneData();
      return TRUE;
    }
  }
  if (job->hasJob && (!job->pc || !job->sym ||
                  (job->sym && !*job->sym->name) ||
                  (job->pc && !*job->pc->name))) {
    if (job->pc && *job->pc->name) {
      vlogf(LOG_PROC, "Attuner (%s) seems to have a bad job structure (case 2) see %s.", me->getName(), job->pc->getName());
    } else {
      vlogf(LOG_PROC, "Attuner (%s) seems to have a bad job structure (case 2A).", me->getName());
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
      for (t = me->getStuff(); t; t = t->nextThing) {
        if (t != job->sym)
          continue;
        else {
          found = TRUE;
          break;
        }
      }

      if (!found) {
        me->doSay("Ack, I lost the symbol somehow! Tell a god immediately!");
        vlogf(LOG_PROC, "Attuner (%s) seems to have lost %s's %s.", me->getName(), job->pc->getName(), job->sym->getName());
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
            me->doSay("Hmm, I seem to have lost the person I was attuning for.
");
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
          add_bars(buf);
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
        sprintf(buf, "%s You are not a cleric or Deikhan.  I can not help you.", ch->getName());
        me->doTell(buf);
        return TRUE;
      }
      for(; *arg && isspace(*arg);arg++);

      if (!(t = searchLinkedListVis(ch, arg, ch->getStuff()))) {
        sprintf(buf, "%s You don't have that symbol.", ch->getName());
        me->doTell(buf);
        return TRUE;
      }
      t->attunerValue(ch, me);
      return TRUE;
    case CMD_MOB_GIVEN_ITEM:
      if (!(t = o)) {
        sprintf(buf, "%s, You don't have that item!", ch->getName());
        me->doTell(buf);
        return TRUE;
      }
      if (!ch->hasClass(CLASS_CLERIC) && !ch->hasClass(CLASS_DEIKHAN)) {
        act("You do not accept $N's offer to give you $p.", TRUE, me, t, ch, TO_CHAR);
        act("$n rejects your attempt to give $s $p.", TRUE, me, t, ch, TO_VICT);
        act("$n refuses to accept $p from $N.", TRUE, me, t, ch, TO_NOTVICT); 
        sprintf(buf, "%s You are not a cleric or Deikhan.  I can not help you.",
        ch->getName());
        me->doTell(buf);
        strcpy(buf, t->name);
        add_bars(buf);
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      if ((ch != me) && (ch->getPosition() != POSITION_RESTING) &&
            (ch->getPosition() != POSITION_SITTING)) {
        act("You do not accept $N's offer to give you $p.", TRUE, me, t, ch, TO_CHAR);
        act("$n rejects your attempt to give $s $p.", TRUE, me, t, ch, TO_VICT);
        act("$n refuses to accept $p from $N.", TRUE, me, t, ch, TO_NOTVICT);
        sprintf(buf, "%s I can not help you unless you take a position of rest.",
        ch->getName());
        me->doTell(buf);
        strcpy(buf, t->name);
        add_bars(buf);
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
  char buf[256];

  sprintf(buf, "%s I can only value weapons.", ch->getName());
  me->doTell(buf);
  return TRUE;
}

int TThing::sharpenerGiveMe(TBeing *ch, TMonster *me)
{
  char buf[256];

  sprintf(buf, "%s, I can only sharpen weapons!", ch->getName());
  me->doTell(buf);
  strcpy(buf, name);
  add_bars(buf);
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
      if (dynamic_cast<TBeing *>(ch->riding)) {
        sprintf(buf, "Hey, get that damn %s out of my shop!",
            fname(ch->riding->name).c_str());
        me->doSay(buf);
        act("You throw $N out.", FALSE, me, 0, ch, TO_CHAR);
        act("$n throws you out of $s shop.", FALSE, me, 0, ch, TO_VICT);
        act("$n throws $N out of $s shop.", FALSE, me, 0, ch, TO_NOTVICT);
        --(*ch->riding);
        thing_to_room(ch->riding, (int) o);
        --(*ch);
        thing_to_room(ch, (int) o);
        return TRUE;
      } else if (dynamic_cast<TBeing *>(ch->rider)) {
        --(*ch->rider);
        thing_to_room(ch->rider, (int) o);
        --(*ch);
        thing_to_room(ch, (int) o);
        return TRUE;
      }
      return FALSE;
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

          if (!(final = searchLinkedList(job->obj_name, me->getStuff()))) {
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
          if (!(final = searchLinkedList(job->obj_name, me->getStuff()))) {
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

      if (!(valued = searchLinkedListVis(ch, arg, ch->getStuff()))) {
        sprintf(buf, "%s You don't have that item.", ch->getName());
        me->doTell(buf);
        return TRUE;
      }
      return valued->sharpenerValueMe(ch, me);
    case CMD_MOB_GIVEN_ITEM:
      if (!(weap = o)) {
        sprintf(buf, "%s, You don't have that item!", ch->getName());
        me->doTell(buf);
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
    vlogf(LOG_PROC, "WARNING:  %s is running around with a bogus spec_proc #%d",
       me->name, me->spec);
  else
    vlogf(LOG_PROC, "WARNING: indeterminate mob has bogus spec_proc");
  return FALSE;
}

int rust_monster(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int i;
  TBeing *tmp_ch;
  TThing *t, *t2;

  if ((cmd != CMD_MOB_COMBAT))
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (!me->roomp || me->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

  for (t = me->roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    tmp_ch = dynamic_cast<TBeing *>(t);
    if (!tmp_ch)
      continue;
    if (tmp_ch->fight() == me) {
      for (i = MIN_WEAR; i < MAX_WEAR; i++) {
        TObj *eq = dynamic_cast<TObj *>(tmp_ch->equipment[i]);
        if (eq) {
          if (eq->canRust()) {
            if ((number(0, eq->getStructPoints()) < 10) &&
                 number(0,101) <= material_nums[eq->getMaterial()].acid_susc) {
              act("$n reaches out and touches your $o!", 
                         FALSE, me, eq, tmp_ch, TO_VICT);
              act("$n reaches out and touches $N's $o!", 
                         FALSE, me, eq, tmp_ch, TO_NOTVICT);
              act("$p is decayed by $N's rust!", FALSE, tmp_ch, eq, me, TO_ROOM);
              act("$p is decayed by $N's rust!", FALSE, tmp_ch, eq, me, TO_CHAR);
              eq->addToStructPoints(-1);
              if (eq->getStructPoints() <= 0) {
                eq->makeScraps();
                delete eq;
                eq = NULL;
              }

              return TRUE;
            }
          }
        }
      }
    }
  }
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
  TThing *t1, *t2;
  for (t1 = me->roomp->getStuff();t1; t1 = t2) {
    t2 = t1->nextThing;
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
      if (t->mobVnum() == APOC_WARRIOR) {
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
    if (mob_index[real_mobile(APOC_WARRIOR)].number > 5)
      return FALSE;

    act("$n calls forth a great warrior to bring war upon The World!",0, me, 0, 0, TO_ROOM, ANSI_RED);
    t = read_mobile(APOC_WARRIOR,VIRTUAL);
    *me->roomp += *t;
    me->addFollower(t);
    t->reconcileDamage(me->fight(),0,DAMAGE_NORMAL);
  }
  // start people in the room fighting each other, randomly 
  // keep War and his horse out of it though 
  if (me->checkPeaceful(""))
    return FALSE;
  TThing *t1, *t2, *t3;
  for (t1 = me->roomp->getStuff();t1; t1 = t2) {
    t2 = t1->nextThing;
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
      t3 = t2->nextThing;
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
  TThing *t, *t2, *t3, *t4;

  for (t = getStuff();t;t = t2) {
    t2 = t->nextThing;

    for (t3 = t->getStuff();t3;t3 = t4) {
      t4 = t3->nextThing;
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
  TThing *t1, *t2;
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

  if ((t = me->fight())) {
   // drain movement from victim
    act("$n grins as $e drains energy from $N to feed $s own.",TRUE,me,0,t,TO_NOTVICT);
    act("$n grins as $e drains your energy to feed $s own!",TRUE,me,0,t,TO_VICT);
    int num = min(t->getMove(),dice(3,5));
    t->addToMove(-num);
    t->addToHit(2 * num);
    return TRUE;
  }

  for (t1 = me->roomp->getStuff();t1; t1 = t2) {
    t2 = t1->nextThing;
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
    act("Your throat tightens as $N causes a great thirst!",TRUE,t,0,
        me, TO_CHAR);
    act("$N causes you to slowly starve!!!!",TRUE,t,0,me, TO_CHAR);
    t->setCond(THIRST, 0);
    t->setCond(FULL, 0);
    t->findFood();
    if (t->reconcileDamage(t,number(5,8),DAMAGE_STARVATION) == -1) {
      delete t;
      t = NULL;
    }
  }
  return FALSE;
}

int pestilence(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  affectedData aff;
  TBeing *t;
  int num = number(1,100);
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

  if ((t = me->fight())) {
    // plagues them
    aff.type = AFFECT_DISEASE;
    aff.level = 0;
    aff.duration = 500;
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    if (num <= 40)
      aff.modifier = DISEASE_COLD;
    else if (num <= 70)
      aff.modifier = DISEASE_FLU;
    else if (num <= 90)
      aff.modifier = DISEASE_LEPROSY;
    else
      aff.modifier = DISEASE_PLAGUE;
    if (!number(0,3))
      t->affectTo(&aff);
  }

  // casts disease on everybody
  TThing *t1, *t2;
  for (t1 = me->roomp->getStuff();t1; t1 = t2) {
    t2 = t1->nextThing;
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
    // protect newbies 
    if (t->GetMaxLevel() < 8 && me->inGrimhaven())
      continue;
    aff.type = AFFECT_DISEASE;
    aff.level = 0;
    aff.duration = 500;
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    if (num <= 60)
      aff.modifier = DISEASE_COLD;
    else if (num <= 85)
      aff.modifier = DISEASE_FLU;
    else if (num <= 95)
      aff.modifier = DISEASE_LEPROSY;
    else
      aff.modifier = DISEASE_PLAGUE;
    if (!number(0,9)) {
      act("$n gazes across the land.", TRUE,me,0,0,TO_ROOM);
      act("Vermin and disease follow in $s wake.", TRUE,me,0,0,TO_ROOM);
      t->affectTo(&aff);
    }
  }
  return FALSE;
}

static int engraveCost(TObj *obj)
{
  const float REGISTRATION_FEE = 1.5;
  int cost;

  cost = (int) (REGISTRATION_FEE * (obj->obj_flags.cost));

  cost *= max(1,obj->obj_flags.cost/10000);

  return cost;
}

int engraver(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  char buf[256],buf2[256];
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
      if (dynamic_cast<TBeing *>(ch->riding)) {
        sprintf(buf, "Hey, get that damn %s out of my shop!",
            fname(ch->riding->name).c_str());
        me->doSay(buf);
        act("You throw $N out.", FALSE, me, 0, ch, TO_CHAR);
        act("$n throws you out of $s shop.", FALSE, me, 0, ch, TO_VICT);
        act("$n throws $N out of $s shop.", FALSE, me, 0, ch, TO_NOTVICT);
        --(*ch->riding);
        thing_to_room(ch->riding, (int) o);
        --(*ch);
        thing_to_room(ch, (int) o);
        return TRUE;
      } else if (dynamic_cast<TBeing *>(ch->rider)) {
        --(*ch->rider);
        thing_to_room(ch->rider, (int) o);
        --(*ch);
        thing_to_room(ch, (int) o);
        return TRUE;
      }
      return FALSE;
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
          if (!(ts = searchLinkedList(job->obj_name, me->getStuff())) ||
              !(final = dynamic_cast<TObj *>(ts))) {
            me->doSay("Ack, I lost the item somehow! Tell a god immediately!  ");
            vlogf(LOG_PROC,"engraver lost his engraving item (%s)",final->name);
            return FALSE;
          }
          final->swapToStrung();

          //  Remake the obj name.  
          sprintf(buf, "%s %s", final->name, job->char_name);
          delete [] final->name;
          final->name = mud_str_dup(buf);

          //  Remake the short description.  
          strcpy(buf,final->shortDescr);
          TBaseClothing *tbc;
          if (final->canWear(ITEM_WEAR_BODY) || 
              ((tbc = dynamic_cast<TBaseClothing *>(final)) &&
               tbc->isShield()))
            sprintf(buf2, "%s with the coat of arms of %s",
                 buf, job->char_name);
          else
            sprintf(buf2, "%s bearing the insignia of %s",
                 buf, job->char_name);
          delete [] final->shortDescr;
          final->shortDescr = mud_str_dup(buf2);

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
      if (!(ts = searchLinkedListVis(ch, arg, ch->getStuff())) ||
          !(valued = dynamic_cast<TObj *>(ts))) {
        sprintf(buf, "%s You don't have that item.", ch->getName());
        me->doTell(buf);
        return TRUE;
      }

      if (valued->engraveMe(ch, me, false))
        return TRUE;

      if (valued->obj_flags.cost <=  500) {
        sprintf(buf, "%s This item is too cheap to be engraved.", ch->getName());
        me->doTell(buf);
        return TRUE;
      }
      if (valued->action_description) {
        sprintf(buf, "%s This item has already been engraved!", ch->getName());
        me->doTell(buf);
        return TRUE;
      }
      if (obj_index[valued->getItemIndex()].max_exist <= 10) {
        sprintf(buf, "%s I refuse to engrave such an artifact of beauty!", ch->getName());
        me->doTell(buf);
        return TRUE;
      }
      if (valued->obj_flags.decay_time >= 0) {
        sprintf(buf, "%s Sorry, but this item won't last long enough to bother with an engraving!", ch->getName());
        me->doTell(buf);
        return TRUE;
      }

      cost = engraveCost(valued);

      sprintf(buf, "%s It will cost %d talens to engrave your %s.", ch->getName(), cost, fname(valued->name).c_str());
      me->doTell(buf);
      return TRUE;
      }
    case CMD_MOB_GIVEN_ITEM:
      // prohibit polys and charms from engraving 
      if (dynamic_cast<TMonster *>(ch)) {
        sprintf(buf, "%s, I don't engrave for beasts.", fname(ch->name).c_str());
        me->doTell(buf);
        return TRUE;
      }
      if (!(item = o)) {
        sprintf(buf, "%s, You don't have that item!", ch->getName());
        me->doTell(buf);
        return TRUE;
      }
      me->logItem(item, CMD_EAST);  // log the receipt of the item

      if (item->engraveMe(ch, me, true))
        return TRUE;

      if (item->obj_flags.cost <= 500) {
        sprintf(buf, "%s, That can't be engraved!", ch->getName());
        me->doTell(buf);
        strcpy(buf, item->name);
        add_bars(buf);
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      if (item->action_description) {
        sprintf(buf, "%s, Sorry, but this item has already been engraved!", ch->getName());
        me->doTell(buf);
        strcpy(buf, item->name);
        add_bars(buf);
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      if (obj_index[item->getItemIndex()].max_exist <= 10) {
        sprintf(buf, "%s, This artifact is too powerful to be engraved!", ch->getName());
        me->doTell(buf);
        strcpy(buf, item->name);
        add_bars(buf);
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      if (item->obj_flags.decay_time >= 0) {
        sprintf(buf, "%s, This won't be around long enough to bother engraving it!", ch->getName());
        me->doTell(buf);
        strcpy(buf, item->name);
        add_bars(buf);
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }

      cost = engraveCost(item);

      if (ch->getMoney() < cost) {
        sprintf(buf, "%s, I have to make a living! If you don't have the money, I don't do the work!", ch->getName());
        me->doTell(buf);
        strcpy(buf, item->name);
        add_bars(buf);
        sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      job = (reg_struct *) me->act_ptr;
      if (!job->wait || !job->char_name) {
        job->wait = max(1, (int) (item->obj_flags.max_struct_points)/7);
        sprintf(buf, "Thanks for your business, I'll take your %d talens payment in advance!", cost);
        me->doSay(buf);
        ch->addToMoney(-cost, GOLD_HOSPITAL);
        job->cost = cost;
        job->char_name = new char[strlen(ch->getName()) + 1];
        strcpy(job->char_name, ch->getName());
        job->obj_name = new char[fname(item->name).length() + 1];
        strcpy(job->obj_name, fname(item->name).c_str());
        --(*item);
        *me += *item; 
        return TRUE;
      } else {
        sprintf(buf, "Sorry, %s, but you'll have to wait while I engrave %s's item.", ch->getName(), job->char_name);
        me->doSay(buf);
        strcpy(buf, item->name);
        add_bars(buf);
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

  one_argument(arg, buf);
  if (!isname(buf, obj_index[real_object(OBJ_FLAMING_PORTAL)].name))
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
  char buf[256];
  const int TICKET_PRICE = 1000;

  if (!ch || !me) {
    vlogf(LOG_PROC,"NULL ch's in TicketGuy");
    return FALSE;
  }
  if (cmd != CMD_BUY)
    return FALSE;

  if (ch->getPosition() > POSITION_STANDING) {
    sprintf(buf,"%s, I won't sell you a ticket unless you stand on your own feet.",fname(ch->name).c_str());
    me->doTell(buf);
    return TRUE;
  }

  arg = one_argument(arg, obj_name);

  if (!*obj_name || strcmp(obj_name,"ticket")) {
    sprintf(buf,"%s, Buy what?!?",fname(ch->name).c_str());
    me->doTell(buf);
    return TRUE;
  }
  // prohibit polys and charms from engraving 
  if (dynamic_cast<TMonster *>(ch)) {
    sprintf(buf, "%s, I don't sell tickets to beasts.", fname(ch->name).c_str());
    me->doTell(buf);
    return TRUE;
  }
  if (ch->getMoney() < TICKET_PRICE) {
    sprintf(buf,"%s, Tickets cost %d talens.",fname(ch->name).c_str(),TICKET_PRICE);
    me->doTell(buf);
    return TRUE;
  }
  ch->addToMoney(-TICKET_PRICE, GOLD_HOSPITAL);
  ch->sendTo("You buy a ticket.\n\r");
  ch->sendTo("The mage makes a strange gesture before you.\n\r");
  ch->sendTo("      *BLICK*\n\r");
  ch->sendTo("Suddenly you find yourself in another plane of existence.\n\r");
  act("$n purchases a ticket and $N transports $m into another plane.",FALSE,ch,0,me,TO_ROOM);
  --(*ch);
  thing_to_room(ch,ROOM_TICKET_DESTINATION);
  ch->doLook("", CMD_LOOK);
  act("$n blicks into the room.",TRUE,ch,0,0,TO_ROOM);
  return TRUE;
}

int Fireballer(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TRoom *rp;
  TBeing *tmp, *temp;
  int dam;

  if (!me || !ch || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  rp = me->roomp;
  if (rp && rp->isUnderwaterSector()) {
    me->sendTo("The water completely dissolves your fireball!\n\r");
    return FALSE;
  }
  act("$n calls forth a mighty Djinn to do $s bidding.",TRUE,me,0,0,TO_ROOM);
  act("You call forth a mighty Djinn to smite your foes.",TRUE,me,0,0,TO_CHAR);

  for (tmp = character_list; tmp; tmp = temp) {
    temp = tmp->next;
    if (me->sameRoom(*tmp) && (me != tmp) &&
        !tmp->isImmortal() ) {
      if (!me->inGroup(*tmp)) {
        dam = dice(me->GetMaxLevel(), 4);
        me->reconcileHurt(tmp, 0.02);
        act("The Djinn breathes a gust of flame at you!",TRUE,tmp,0,0,TO_CHAR);
        act("The Djinn bathes $n in a breath of flame.",TRUE,tmp,0,0,TO_ROOM);
        if (tmp->isLucky(levelLuckModifier(me->GetMaxLevel()))) {
          act("....You are able to dodge the majority of the flame.",TRUE,tmp,0,0,TO_CHAR);
          dam /= 2;
        }
        if (me->reconcileDamage(tmp, dam, SPELL_FIRE_BREATH) == -1) {
          delete tmp;
          tmp = NULL;
        }
      }
    } else if (tmp->isImmortal() && me->sameRoom(*tmp)) {
      act("The Djinn chokes on a hairball.",TRUE,tmp,0,0,TO_CHAR);
      act("$n causes the Djinn to choke on a hairball before it can breathe at $m.",TRUE,tmp,0,0,TO_ROOM);
    } else if ((me != tmp) && (tmp->in_room != ROOM_NOWHERE) && (rp->getZoneNum() == tmp->roomp->getZoneNum())) {
      tmp->sendTo("You hear a loud explosion and feel a gust of hot air.\n\r");
    }
  }

  return TRUE;
}

int Teleporter(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int rc;
  TBeing *vict;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!(vict = me->fight()))
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n summons a Djinn to do $s bidding.",TRUE,me,0,0,TO_ROOM);
  act("You summon forth a Djinn.",TRUE,me,0,0,TO_CHAR);
  
  act("The Djinn looks in your direction and snaps his fingers!",TRUE,vict,0,0,TO_CHAR);
  act("The Djinn looks in $n's direction and snaps his fingers.",TRUE,vict,0,0,TO_ROOM);

  rc = vict->genericTeleport(SILENT_NO);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete vict;
    vict = NULL;
  }

  return TRUE;
}

int MSwarmer(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TObj *stone;
  int ret, rc;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;
  if (!me->fight())
    return FALSE;
  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n pulls a small stone out of $s robes.",TRUE,me,0,0,TO_ROOM);
  act("You pull a small stone out of your robes.",TRUE,me,0,0,TO_CHAR);

  act("$e throws it to the $g and looks skyward hopefully.",TRUE,me,0,0,TO_ROOM);
  act("You toss the stone to the $g in hopes of bringing forth meteors.",TRUE,me,0,0,TO_CHAR);

  me->setSkillValue(SPELL_METEOR_SWARM, 100);

  if (number(1,5) > 2) {
    ret = castMeteorSwarm(me, me->fight());
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }
  } else {
    stone = read_object(COMP_SKY_ROCK, VIRTUAL);
    *me->roomp += *stone;
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  me->addSkillLag(SPELL_METEOR_SWARM, 0);
  return TRUE;
}

int IceStormer(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int ret, rc;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  if (!me->in_room || me->in_room == ROOM_NOWHERE)
    return FALSE;

  act("$n pulls a small cube of ice from out of a pouch.",TRUE,me,0,0,TO_ROOM);
  act("You pull a small cube of ice from out of a pouch.",TRUE,me,0,0,TO_CHAR);
  act("$e crushes it against $s belt and throws the flakes to the $g.",TRUE,me,0,0,TO_ROOM);
  act("You crush the flakes and throw them to the $g.",TRUE,me,0,0,TO_CHAR);

  me->setSkillValue(SPELL_ICE_STORM, 100);

  if (number(1,5) > 2) {
    ret = castIceStorm(me);
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }

  } else {
    *me->roomp += *read_object(COMP_ICEBERG_CORE, VIRTUAL);
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  me->addSkillLag(SPELL_ICE_STORM, 0);
  return TRUE;
}

int Edrain(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int ret, rc = FALSE;
  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n utters some mysterious words and holds forth $s hand.",TRUE,me,0,0,TO_ROOM);
  act("You utter some dark words of power and hold forth your hand.",TRUE,me,0,0,TO_CHAR);
  act("A black orb rises from $n's hand - it spins straight up...",TRUE,me,0,0,TO_ROOM);
  act("You create a black orb and see it rise before you...",TRUE,me,0,0,TO_CHAR);
  act("A scathing beam of light from the orb sears $n!!",TRUE,me->fight(),0,0,TO_ROOM);
  act("A scathing beam of light from the orb sears you!!!",TRUE,me->fight(),0,0,TO_CHAR);

  me->setSkillValue(SPELL_ENERGY_DRAIN, 100);

  if (number(1,5)>2) {
    ret = castEnergyDrain(me, me->fight());
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }
  } else {
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  me->addSkillLag(SPELL_ENERGY_DRAIN, 0);
  return TRUE;
}

int LBolter(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int ret, rc;
  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n utters some mysterious words and holds forth $s hand.",TRUE,me,0,0,TO_ROOM);
  act("You utter some dark words of power and hold forth your hand.",TRUE,me,0,0,TO_CHAR);
  act("A brilliant red orb crackling with power rises slowly from $s palm.",TRUE,me,0,0,TO_ROOM);
  act("A brilliant red orb crackling with power rises skyward.",TRUE,me,0,0,TO_CHAR);
  act("A mighty blast is unleashed from the orb aimed at $n.",TRUE,me->fight(),0,0,TO_ROOM);
  act("A mighty blast is unleashed from the orb at you!!!",TRUE,me->fight(),0,0,TO_CHAR);

  if (number(1,5)>2) {
    ret = castBlastOfFury(me, me->fight());
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }
  } else {
    *me->roomp += *read_object(COMP_BLOOD_SCORN_WOMAN,VIRTUAL);
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  me->addSkillLag(SPELL_BLAST_OF_FURY, 0);
  return TRUE;
}

int Disser(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int rc, ret;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n begins to chant a incantation...",TRUE,me,0,0,TO_ROOM);
  act("You begin to chant a incantation...",TRUE,me,0,0,TO_CHAR);
  me->doSay("Ashes to ashes, dust to dust... Ashes to ashes, dust to dust...");
  act("$n points at $N ominously.",TRUE,me,0,me->fight(),TO_NOTVICT);
  act("$n points at you ominously.",TRUE,me,0,me->fight(),TO_VICT);
  act("You point at $N.",TRUE,me,0,me->fight(),TO_CHAR);

  me->setSkillValue(SPELL_ATOMIZE, 100);

  if (number(1,5)>2) {
    ret = atomize(me, me->fight());
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }

  } else {
    *me->roomp += *read_object(COMP_DRAGON_BONE,VIRTUAL);
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  return TRUE;
}

int Witherer(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  TBeing *vict = me->fight();
  if (!vict)
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n begins to chant a mantra...",TRUE,me,0,0,TO_ROOM);
  act("You begin to chant a mantra...",TRUE,me,0,0,TO_CHAR);
  me->doSay("You put you left foot in... You take your left foot out...");
  act("$n points at $N ominously.",TRUE,me,0,vict,TO_NOTVICT);
  act("$n points at you ominously.",TRUE,me,0,vict,TO_VICT);
  act("You point at $N.",TRUE,me,0,vict,TO_CHAR);

  me->setSkillValue(SPELL_WITHER_LIMB, 100);

  if (number(1,5)>2) {
    int ret = witherLimb(me, vict);
    if (IS_SET_DELETE(ret, DELETE_VICT)) {
      delete vict;
      vict = NULL;
    }
  } else {
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  return TRUE;
}

int Paralyzer(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  affectedData aff;
  TBeing *v;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!(v = me->fight()))
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  if (v->isAffected(AFF_PARALYSIS) || 
      v->isImmune(IMMUNE_PARALYSIS, me->GetMaxLevel())) {
    return FALSE;
  }

  act("$n utters something unintelligible...",TRUE,me,0,0,TO_ROOM);
  act("You utter something in a foreign tongue...",TRUE,me,0,0,TO_CHAR);
  me->doSay("  ...rigor mortis corpus feline a mehi inamici.. ");
  act("$n pulls a dead cat out of a bag and throws it at $N's feet.",
         TRUE,me,0,v,TO_NOTVICT);
  act("$n pulls a dead cat from a bag and throws it at your feet.",
         TRUE,me,0,v,TO_VICT);
  act("You throw a dead cat at $N's feet.",TRUE,me,0,v,TO_CHAR);

  act("Suddenly, the cat comes back to life!!",TRUE,me,0,0,TO_ROOM);
  act("Suddenly, the cat comes back to life!!",TRUE,me,0,0,TO_CHAR);

  aff.type = SPELL_PARALYZE;
  aff.level = me->GetMaxLevel();
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_PARALYSIS;
  aff.duration = 2 * number(1, aff.level);
  aff.modifier = 0;
 
  v->affectTo(&aff);
  *me->roomp += *read_mobile(MOB_SMALL_CAT,VIRTUAL);

  return TRUE;
}

int AcidBlaster(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TObj *obj;
  int rc, ret;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n produces an opaque vial from inside $s robes.",TRUE,me,0,0,TO_ROOM);
  act("You produce an opaque vial from inside your robes.",TRUE,me,0,0,TO_CHAR);
  act("$n grins and heaves it in $N's direction.",TRUE,me,0,me->fight(),TO_ROOM);
  act("You grin and heave it at $N.",TRUE,me,0,me->fight(),TO_CHAR);

  me->setSkillValue(SPELL_ACID_BLAST, 100);

  if (number(1,5)>2) {
    act("The vial shatters and acid spews forth!!",TRUE,me,0,0,TO_ROOM);
    ret = acidBlast(me);
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }
  } else {
    obj = read_object(COMP_ACID_BLAST, VIRTUAL);
    *me->roomp += *obj;
    act("$p remains intact.",TRUE,me,obj,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  return TRUE;
}

int kingUrik(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int rc;
  TBeing *vict;

  if (!me)
    return FALSE;
  if ((cmd != CMD_MOB_COMBAT))
    return FALSE;
  if (!(vict = me->fight()))
    return FALSE;

  if (number(0,2))
    return FALSE;

  me->wait += 4;

  act("$n touches you, sucking your life energy.",TRUE,me,0,vict,TO_VICT);
  act("You touch $N and slurp some emergy.",TRUE,me,0,vict,TO_CHAR);
  act("$n touches $N and drains some life energy from $M.",TRUE,ch,0,vict,TO_NOTVICT);

  int dam = dice((me->GetMaxLevel()/2),3);
  if (!(dam = vict->getActualDamage(me,NULL,dam,DAMAGE_DRAIN))) {
    vict->sendTo("Fortunately, it has no effect on you.\n\r");
    return FALSE;
  }
  rc = me->applyDamage(vict,dam,DAMAGE_DRAIN);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete vict;
    vict = NULL;
    return TRUE;
  }

  vict->addToMana(-min(vict->getMana(),dice(12,4)));
  vict->addToMove(-dice(10,4));

  return TRUE;
}

int rock_worm(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TBeing *vict;

  if (!me)
    return FALSE;
  if ((cmd != CMD_MOB_COMBAT))
    return FALSE;
  if (!(vict = me->fight()))
    return FALSE;

  if (number(0,2))
    return FALSE;

  return TRUE;
}

int hobbitEmissary(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int rc;

  class hunt_struct {
    public:
    byte cur_pos;
    byte cur_path;
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
  char buf[160];

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
  if (job->hunted_victim != NULL) {
    if (!(targ = get_char(job->hunted_victim, EXACT_YES))) {
      return FALSE;
    }
    if (targ->sameRoom(*myself)) {
      if (!strcmp(job->hunted_victim, "ambassador hobbit Grimhaven")) {
        sprintf(buf,"Good %s, your excellency.",describeTime());
        myself->doSay(buf);
        sprintf(buf,"Good %s.",describeTime());
        targ->doSay(buf);
        myself->doSay("I have a message from his lordship.");
        sprintf(buf, "%s sweet nothings",fname(targ->name).c_str());
        myself->doWhisper(buf);
        targ->doSay("Hmm, that is useful news.  Relay this message back for me.");
        sprintf(buf, "%s sweet nothings",fname(myself->name).c_str());
        targ->doWhisper(buf);
        targ->doSay("Hurry off with that and report back soonest.");
        act("$n salutes the ambassador.",0, myself, 0, 0, TO_ROOM);
        delete [] job->hunted_victim;
        job->hunted_victim = mud_str_dup("king Grimhaven");
        job->cur_path = 1;
        job->cur_pos = 0;
      } else if (!strcmp(job->hunted_victim, "king Grimhaven")) {
        sprintf(buf,"Good %s, your lordship.",describeTime());
        myself->doSay(buf);
        sprintf(buf,"Good %s.",describeTime());
        targ->doSay(buf);
        myself->doSay("I have a message from his excellency.");
        sprintf(buf, "%s sweet nothings",fname(targ->name).c_str());
        myself->doWhisper(buf);
        targ->doSay("Hmm, that is useful news.  Relay this message back for me.");
        sprintf(buf, "%s sweet nothings",fname(myself->name).c_str());
        targ->doWhisper(buf);
        targ->doSay("Hurry off with that and tell me his response.");
        act("$n salutes the King.",0, myself, 0, 0, TO_ROOM);
        delete [] job->hunted_victim;
        job->hunted_victim = mud_str_dup("ambassador hobbit Grimhaven");
        job->cur_path = 0;
        job->cur_pos = 0;
      } else {
        vlogf(LOG_PROC,"Error: hobbit emissary hunted undefined target. (%s)",
          job->hunted_victim);
        delete [] job->hunted_victim;
        job->hunted_victim = NULL;
      }
      return TRUE;
    } else if (hobbit_path_pos[job->cur_path][(job->cur_pos + 1)].direction == -1) {
      // end of path
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
    } else if (hobbit_path_pos[job->cur_path][job->cur_pos].cur_room != myself->in_room) {
      // Not in correct room

      // check surrounding rooms, I probably fled
      dirTypeT door;
      for (door = MIN_DIR; door < MAX_DIR; door++) {
        if (myself->canGo(door)) {
          if (myself->roomp->dir_option[door]->to_room ==
                  hobbit_path_pos[job->cur_path][job->cur_pos].cur_room) {
            rc = myself->goDirection(door);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
            return TRUE;
          }
        }
      }

      // trace along entire route and see if I can correct
      do {
        job->cur_pos += 1;
        if (hobbit_path_pos[job->cur_path][job->cur_pos].cur_room == myself->in_room)
          return TRUE;
      } while (hobbit_path_pos[job->cur_path][job->cur_pos].cur_room != -1);

      act("$n seems to have gotten a little bit lost.",0, myself, 0, 0, TO_ROOM);
      act("$n goes to ask directions.", 0, myself, 0, 0, TO_ROOM);
      //vlogf(LOG_PROC, "Hobbit got lost: path: %d, pos: %d", job->cur_path, myself->in_room);
      if (myself->riding)
        myself->dismount(POSITION_STANDING);
      --(*myself);
      thing_to_room(myself, hobbit_path_pos[job->cur_path][0].cur_room);
      act("$n has arrived.", 0, myself, 0, 0, TO_ROOM);
      return TRUE;
    } else if (myself->getPosition() < POSITION_STANDING) {
      myself->doStand();
    } else {
      rc = myself->goDirection(hobbit_path_pos[job->cur_path][(job->cur_pos + 1)].direction);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      } else if (rc) {
        // if go_dir == 0, then it either can't go that way or is opening a door
        job->cur_pos += 1;
      }
    }
  } else {
    delete [] job->hunted_victim;
    job->hunted_victim = mud_str_dup("ambassador hobbit Grimhaven");
  }
  return 0;
}

int paralyzeGaze(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *){
  TBeing *v;
  affectedData aff;

  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  if (!(v = myself->fight()) || !v->sameRoom(*myself)) 
    return FALSE;

  if (::number(0,10))
    return FALSE;

  if (v->isAffected(AFF_PARALYSIS) || 
      v->isImmune(IMMUNE_PARALYSIS, myself->GetMaxLevel())) {
    return FALSE;
  }

  act("$n fixes $N with a penetrating gaze.", 
              TRUE, myself, NULL, v, TO_NOTVICT);
  act("$n fixes you with a penetrating gaze.  Suddenly, you have trouble moving.", TRUE, myself, NULL, v, TO_VICT);
  act("You fix $N with a penetrating gaze.", TRUE, myself, NULL, v, TO_CHAR);

  if(!v->isImmortal()){
    aff.type = SPELL_PARALYZE;
    aff.level = myself->GetMaxLevel();
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_PARALYSIS;
    aff.duration = 1;
    aff.modifier = 0;
    
    v->affectTo(&aff);
  }

  return TRUE;

}

int banshee(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (::number(0,4))
    return FALSE;

  act("You unleash a hideous screech.", FALSE, myself, 0, 0, TO_CHAR);
  act("$n unleashes a hideous screech.", FALSE, myself, 0, 0, TO_ROOM);

  TThing *t;
  for (t = myself->roomp->getStuff(); t; t = t->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (!tbt)
      continue;
    if (tbt == myself)
      continue;
    if (tbt->isImmortal())
      continue;
    TObj *muff = dynamic_cast<TObj *>(tbt->equipment[WEAR_HEAD]);
    if (muff && muff->objVnum() == OBJ_EARMUFF)
      continue;

    act("The scream rocks you to your core and you feel more aged.", FALSE, tbt, 0, 0, TO_CHAR);
    tbt->age_mod += 1;
  }

  return FALSE;
}

int corpseMuncher(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int found=0, msg;
  const char *munch[]={
      "$n stops eating, looks up at you and burps loudly, then resumes feasting on $p.",
      "<r>Blood<1> spatters about as $n bites deeply into $p.",
      "$n utters a low growl as $e rips a chunk of flesh off $p.",
      "$n stops to spit out a piece of <W>bone<1>, then resumes eating $p."};


  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake() || ch->fight())
    return FALSE;

  for (t = myself->roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
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

int fishTracker(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
{
  char buf[256];
  TThing *t;
  TDatabase db("sneezy");

  if(!ch || !ch->awake() || ch->fight())
    return FALSE;

  
  switch(cmd){
    case CMD_MOB_GIVEN_ITEM:
      if(!o || !isname("caughtfish", o->name)){
	return FALSE;
      }

      db.query("select 1 from fishkeeper where name='%s'", ch->name);
      if(!db.fetchRow()){
	db.query("insert into fishkeeper values ('%s', %f)", 
		 ch->name, o->getWeight());
      } else {
	db.query("update fishkeeper set weight=weight+%f where name='%s'", o->getWeight(), ch->name);
      }

      // check for largest
      db.query("select weight, name from fishlargest where type='%s'", o->shortDescr);
      db.fetchRow();

      if(o->getWeight() > atoi_safe(db.getColumn(0))){
	db.query("update fishlargest set name='%s', weight=%f where type='%s'", ch->getName(), o->getWeight(), o->shortDescr);

	sprintf(buf, "Oh my, you've broken %s's record!  This the largest %s I've seen, weighing in at %f!  Very nice! (%i talens)",
		db.getColumn(1), o->shortDescr, o->getWeight(), (int)(o->getWeight()*100));
	myself->doSay(buf);
	ch->addToMoney((int)(o->getWeight()*100), GOLD_COMM);	
      } else {
	sprintf(buf, "Ok, I tallied your fish, weighing in at %f.  Nice one! (%i talens)", 
		o->getWeight(), (int)(o->getWeight()*2));
	myself->doSay(buf);
	ch->addToMoney((int)(o->getWeight()*2), GOLD_COMM);
      }


      // heh why'd I do this instead of returning DELETE_ ??
      // beats me, I ain't gonna touch it now though
      for(t=myself->getStuff();t;t=t->nextThing){
	if(isname("caughtfish", t->name)){
	  delete t;
	  break;
	}
      }

      ch->doSave(SILENT_YES);

      break;
    case CMD_WHISPER:
      arg = one_argument(arg, buf);
      
      if(!isname(buf, myself->name))
	return FALSE;

      arg = one_argument(arg, buf);

      if(!strcmp(buf, "records")){
	db.query("select name, type, weight from fishlargest order by weight desc");

	while(db.fetchRow()){
	  sprintf(buf, "%s caught %s weighing in at %f.",
		  db.getColumn(0), db.getColumn(1), atof_safe(db.getColumn(2)));
	  myself->doSay(buf);
	}      
      } else {
	if(!strcmp(buf, "topten")){
	  db.query("select o.name, o.weight, count(l.name) from fishkeeper o left join fishlargest l on o.name=l.name group by o.name, o.weight order by weight desc limit 10");
	} else {
	  db.query("select o.name, o.weight, count(l.name) from fishkeeper o left join fishlargest l on o.name=l.name where o.name='%s' group by o.name, o.weight order by weight desc limit 10", buf);
	}
	
	while(db.fetchRow()){
	  sprintf(buf, "%s has %f pounds of fish and %i records.",
		  db.getColumn(0), atof_safe(db.getColumn(1)), atoi_safe(db.getColumn(2)));
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
  char *hookername, *johnname, tmp[256];

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
	      !myself->awake() || myself->fight() || ::number(0,25))
    return FALSE;

  if (!myself->act_ptr) {
    //    if(::number(0,5))
    //      return FALSE;

    // find a john
    for(t=myself->roomp->getStuff(); t && !found; t=t->nextThing){
      if((tmons=dynamic_cast<TMonster *>(t))){
	if(tmons!=myself && tmons->getRace()==myself->getRace() &&
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
    	  
  hookername=mud_str_dup(myself->name);
  add_bars(hookername);
  johnname=mud_str_dup(job->john->name);
  add_bars(johnname);
  
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
      switch(::number(0,3)){
	case 0:
	  job->john->doAction("", CMD_SNICKER);
	  job->john->doSay("Yeah, right.  As if.");
	  break;
	case 1:
	  job->john->doAction("", CMD_SNEER);
	  job->john->doSay("Get the hell away from you skank ghetto whore!");
	  break;
	case 2:
	  job->john->doAction("", CMD_BLUSH);
	  job->john->doSay("Um, no thanks.");
	  break;
	case 3:
	  job->john->doSay("Not me sexy, why pay when I get it for free in the pasture?");
	  job->john->doAction("", CMD_CHUCKLE);
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
	  sprintf(tmp, "%s Do you swallow?", hookername);
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
      switch(::number(0,1)){
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
      }

      if(::number(0,1))
	job->state=STATE_REJECT2;
      else
	job->state=STATE_ACCEPT;
      break;
    case STATE_REJECT2:
      switch(::number(0,1)){
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
      }
      job->john=NULL;
      job->state=STATE_NONE;
      break;
    case STATE_ACCEPT:
      switch(::number(0,2)){
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
      }
      job->john->doFollow(hookername);
      job->state=STATE_WALKING;
      break;
    case STATE_WALKING:
      if(::number(0,2))
	break;

      if(myself->in_room != homes[job->cur_path]){
	switch((dir=find_path(myself->in_room, is_target_room_p, 
			 (void *) homes[job->cur_path], myself->trackRange(), 0))){
          case -1: // lost
	    myself->doSay("I'm lost and shit");
	    break;
          case 0: case 1: case 2: case 3: case 4: 
          case 5: case 6: case 7: case 8: case 9:
	    myself->goDirection(dir);
	    break;
  	  default:
	    myself->doSay("I need to enter a portal");
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
      for(t=myself->roomp->getStuff();t;t=t->nextThing){
	if(dynamic_cast<TBeing *>(t)){
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
	}
	delete myself;
      }
      break;
  }
  
  delete hookername;
  delete johnname;
  return FALSE;
}


// this proc is kind of ugly, but it works
int bankGuard(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
{
  Descriptor *i;
  int zone_nr=real_roomp(31750)->getZoneNum(), v=0;
  TBeing *victims[10], *vict;
  int saferooms[8]={31750, 31751, 31756, 31757, 31758, 31759, 31764, 31788};  

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
	i->character->in_room != saferooms[7]){
      victims[v]=i->character;
      ++v;
    }
  }

  if(!v)
    return FALSE;

  vict=victims[::number(0,v-1)];
  vlogf(LOG_PEEL, "bank guard hunting %s", vict->getName());
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
  TThing *t = NULL, *t2 = NULL;
  TBeing *tb = NULL;;
  bool found = false;

  if (cmd != CMD_GENERIC_QUICK_PULSE || myself->fight())
    return FALSE;
  
  // look for people to run from
  for (t = myself->roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
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


int stockBroker(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
{
  int num, count=0;
  char buf2[1024], buf[1024];
  TDatabase db("sneezy");

  if(cmd != CMD_LIST &&
     cmd != CMD_BUY &&
     cmd != CMD_SELL &&
     cmd != CMD_VALUE)
    return FALSE;

  arg = one_argument(arg, buf2);
  
  if(cmd == CMD_LIST){
    if(*buf2){
      // list details
      
      // list their portfolio
      if(is_abbrev(buf2, "portfolio")){
	db.query("select o.ticker, o.shares, i.price from stockowners o, stockinfo i where owner='%s' and o.ticker=i.ticker", ch->getName());
	
	sprintf(buf, "%s, Here are the stocks you own:", 
		fname(ch->name).c_str());
	myself->doTell(buf);

	while(db.fetchRow()){
	  if(atoi_safe(db.getColumn(1))>0){
	    count+=(int)(atof_safe(db.getColumn(2))*atoi_safe(db.getColumn(1)));
	    sprintf(buf, "%s, You own %s shares of %s, worth %f",
		    fname(ch->name).c_str(), db.getColumn(1), db.getColumn(0),
		    atof_safe(db.getColumn(2))*atoi_safe(db.getColumn(1)));
	    myself->doTell(buf);
	  }
	}
	sprintf(buf, "%s, Your portfolio is worth %i talens right now.",
		fname(ch->name).c_str(), count);
	myself->doTell(buf);

      } else {
	db.query("select ticker, price from stockinfo where upper('%s')=ticker", buf2);
	if(db.fetchRow())
	  return FALSE;
	
	sprintf(buf, "%s, %s - %s talens per share.",
		fname(ch->name).c_str(), db.getColumn(0), db.getColumn(1));

	myself->doTell(buf);
	//	sprintf(buf, "%s, %s", fname(ch->name).c_str(), row[2]);
	//	myself->doTell(buf);
      }
    } else {
      // generic list
      db.query("select ticker, price from stockinfo");
    
      sprintf(buf, "%s, I charge a 10%% commission on all transactions.",
	      fname(ch->name).c_str());
      myself->doTell(buf);

      sprintf(buf, "%s, You can 'list <stock name>' to see details.",
	      fname(ch->name).c_str());
      myself->doTell(buf);

      sprintf(buf, "%s, You can 'list portfolio' to see your portfolio.",
	      fname(ch->name).c_str());
      myself->doTell(buf);      

      sprintf(buf, "%s, Stock graphs can be seen at http://sneezy.stanford.edu/peel/stocks",
	      fname(ch->name).c_str());
      myself->doTell(buf);      


      while(db.fetchRow()){
	sprintf(buf, "%s, %s - %s talens per share.", 
		fname(ch->name).c_str(), db.getColumn(0), db.getColumn(1));
	myself->doTell(buf);
      }
    }
    return TRUE;
  } else if(cmd == CMD_BUY){
    if(!ch->isImmortal()){
      myself->doSay("Buying is disabled right now.");
      return TRUE;
    }
    

    char tmpname[80] = "\0";
    char *bptr=buf2;
    
    sscanf(bptr, "%d*%s", &num, tmpname);
    if (tmpname[0] == '\0')
      return FALSE;
    if (num < 1)
      num=0;

    while (*bptr != '*')
      bptr++;
    
    bptr++;

    if(num)
      strcpy(buf2, bptr);
    else 
      return FALSE;

    db.query("select ticker, price, talens from stockinfo where upper('%s')=ticker", buf2);

    if(!db.fetchRow())
      return FALSE;

    // row[0] ticker
    // row[1] price
    // num amount to buy

    float modprice=atof_safe(db.getColumn(1))*(float)num;
    modprice*=1.01;

    if(ch->getMoney() < modprice){
      sprintf(buf, "%s, You can't afford that.", fname(ch->name).c_str());
      myself->doTell(buf);
      return TRUE;
    }

    if(atof_safe(db.getColumn(1)) < 1){
      sprintf(buf, "%s, Because of government regulations, I can't sell stock that is worth less than 1 talen per share.", fname(ch->name).c_str());
      myself->doTell(buf);
      return TRUE;
    }
    
    ch->addToMoney(-modprice, GOLD_COMM);

    db.query("update stockinfo set talens=talens+%i", (int)(modprice/1.01));

    db.query("select 1 from stockowners where owner='%s' and ticker='%s'",
	     ch->getName(), db.getColumn(0));
    if(!db.fetchRow()){
      db.query("insert into stockowners values ('%s', '%s', %i)", ch->getName(), db.getColumn(0), num);
    } else {
      db.query("update stockowners set shares=shares+%i where owner='%s' and ticker='%s'", num, ch->getName(), db.getColumn(0));
    }

    sprintf(buf, "%s, Ok, you just purchased %i shares of %s, for a price of %f.",
	    fname(ch->name).c_str(), num, db.getColumn(0), modprice);
    myself->doTell(buf);

    return TRUE;
  } else if(cmd == CMD_SELL){
    char tmpname[80] = "\0";
    char *bptr=buf2;
    
    sscanf(bptr, "%d*%s", &num, tmpname);
    if (tmpname[0] == '\0')
      return FALSE;
    if (num < 1)
      num=0;

    while (*bptr != '*')
      bptr++;
    
    bptr++;

    if(num)
      strcpy(buf2, bptr);
    else 
      return FALSE;

    db.query("select i.ticker, i.price, o.shares from stockinfo i, stockowners o where o.owner='%s' and i.ticker='%s' and i.ticker=o.ticker", ch->getName(), buf2);

    if(!db.fetchRow())
      return FALSE;
    
    if(atoi_safe(db.getColumn(2)) < num){
      sprintf(buf, "%s, You don't own enough shares of that stock.",
	      fname(ch->name).c_str());
      myself->doTell(buf);
      return TRUE;
    }

    ch->addToMoney(((float)num * atof_safe(db.getColumn(1))), GOLD_COMM);

    db.query("update stockinfo set talens=talens-%i", (int)(((float)num * atof_safe(db.getColumn(1)))));
    db.query("update stockowners set shares=shares-%i where owner='%s' and ticker='%s'", num, ch->getName(), db.getColumn(0));

    sprintf(buf, "%s, Ok, you just sold %i shares of %s, for a price of %f, minus my 1%% commission of %f.",
	    fname(ch->name).c_str(), num, db.getColumn(0), (float)num * atof_safe(db.getColumn(1)),
	    (float)num * atof_safe(db.getColumn(1)) * 0.01);
    myself->doTell(buf);
    
    
    
    return TRUE;
  } else if(cmd==CMD_VALUE){
    char tmpname[80] = "\0";
    char *bptr=buf2;
    
    sscanf(bptr, "%d*%s", &num, tmpname);
    if (tmpname[0] == '\0')
      return FALSE;
    if (num < 1)
      num=0;

    while (*bptr != '*')
      bptr++;
    
    bptr++;

    if(num)
      strcpy(buf2, bptr);
    else 
      return FALSE;

    db.query("select ticker, price, talens from stockinfo where upper('%s')=ticker", buf2);

    if(!db.fetchRow())
      return FALSE;

    if(atof_safe(db.getColumn(1)) < 1){
      sprintf(buf, "%s, Because of government regulations, I can't sell stock that is worth less than 1 talen per share.", fname(ch->name).c_str());
      myself->doTell(buf);
      return TRUE;
    }

    float modprice=atof_safe(db.getColumn(1));
    modprice*=1.01;

    sprintf(buf, "%s, %i shares of %s would cost %i talens.",
	    fname(ch->name).c_str(), num, db.getColumn(0), (int)((float)num*modprice));

    myself->doTell(buf);

    return TRUE;
  }

  return FALSE;
}

static int divCost(TObj *obj)
{
  const float FEE = 0.8;
  int cost;

  cost = (int) (FEE * (obj->obj_flags.cost));

  if(cost<0)
    cost=0;

  return cost;
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
  string str;

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
      if (dynamic_cast<TBeing *>(ch->riding)) {
        sprintf(buf, "Hey, get that damn %s out of my shop!",
            fname(ch->riding->name).c_str());
        me->doSay(buf);
        act("You throw $N out.", FALSE, me, 0, ch, TO_CHAR);
        act("$n throws you out of $s shop.", FALSE, me, 0, ch, TO_VICT);
        act("$n throws $N out of $s shop.", FALSE, me, 0, ch, TO_NOTVICT);
        --(*ch->riding);
        thing_to_room(ch->riding, (int) o);
        --(*ch);
        thing_to_room(ch, (int) o);
        return TRUE;
      } else if (dynamic_cast<TBeing *>(ch->rider)) {
        --(*ch->rider);
        thing_to_room(ch->rider, (int) o);
        --(*ch);
        thing_to_room(ch, (int) o);
        return TRUE;
      }
      return FALSE;
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
      if (!(ts = searchLinkedListVis(ch, arg, ch->getStuff())) ||
          !(valued = dynamic_cast<TObj *>(ts))) {
        sprintf(buf, "%s You don't have that item.", ch->getName());
        me->doTell(buf);
        return TRUE;
      }
      if (valued->obj_flags.decay_time >= 0) {
        sprintf(buf, "%s Sorry, but this item won't last long and isn't worth the money spent.", ch->getName());
        me->doTell(buf);
        return TRUE;
      }

      cost = divCost(valued);

      sprintf(buf, "%s It will cost %d talens to identify your %s.", ch->getName(), cost, fname(valued->name).c_str());
      me->doTell(buf);
      return TRUE;
      }
    case CMD_MOB_GIVEN_ITEM:
      if (!(item = o)) {
        sprintf(buf, "%s, You don't have that item!", ch->getName());
        me->doTell(buf);
        return TRUE;
      }
      // prohibit polys and charms from engraving 
      if (dynamic_cast<TMonster *>(ch)) {
        sprintf(buf, "%s, I don't identify for beasts.", fname(ch->name).c_str());
        me->doTell(buf);
        me->doGive(ch, item,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      me->logItem(item, CMD_EAST);  // log the receipt of the item
      cost = divCost(item);
      if (ch->getMoney() < cost) {
        sprintf(buf, "%s, I have to make a living! If you don't have the money, I don't do the work!", ch->getName());
        me->doTell(buf);
        me->doGive(ch,item,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      sprintf(buf, "Thanks for your business, I'll take your %d talens payment in advance!", cost);
      me->doSay(buf);
      ch->addToMoney(-cost, GOLD_HOSPITAL);
      ch->sendTo("%s concentrates deeply on %s.\n\r", me->getName(), item->getName());
      ch->sendTo("%s conjours a cloud of smoke.\n\rInside the cloud of smoke you see...\n\r", me->getName());
      ch->statObjForDivman(item);
      sprintf(buf, "Thank you, %s, for your business! Please come again!", ch->getName());
      me->doSay(buf);
      me->doGive(ch,item,GIVE_FLAG_IGN_DEX_TEXT);
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}
int gardener(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *)
{
  TThing *t;
  static unsigned int pulse;

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  ++pulse;
  if(pulse%150)
    return FALSE;

  if (ch->task)
    return FALSE;

  if (!(t = get_thing_char_using(ch, "seed", 0, FALSE, FALSE))) {
    return FALSE;
  }

  ch->doAction("", CMD_WHISTLE);
  ch->doPlant("seed");
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
  TThing *t;
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
      vlogf(LOG_PROC, "Archer couldn't load his bow %d.  DASH!!!", bownum);
      return TRUE;
    }
    strcpy(temp, bow->name);
    add_bars(temp);
    ch->doJunk(temp, NULL); // just in case its loaded, no point making tons
    act("You quickly unpack $p.", FALSE, ch, bow, 0, TO_CHAR);
    act("$n quickly unpacks $p.", FALSE, ch, bow, 0, TO_ROOM);
    ch->equipChar(bow, ch->getPrimaryHold(), SILENT_YES);
    return TRUE;
  }


  if (!bow->getStuff()) {
    //    vlogf(LOG_DASH, "archer loading an arrow");

    if (!(arrow = read_object(arrownum, VIRTUAL))) {
      vlogf(LOG_PROC, "Archer couldn't load his arrow %d.  DASH!!!", arrownum);
      return TRUE;
    }
    *bow += *arrow;
    act("You quickly load $N into $p.", FALSE, ch, bow, arrow, TO_CHAR);
    act("$n quickly loads $N into $p.", FALSE, ch, bow, arrow, TO_ROOM);
    return TRUE;
  }

  if (bow->isBowFlag(BOW_STRING_BROKE)) {
    //    vlogf(LOG_DASH, "archer fixing a bowstring");

    
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

      

      for (t = real_roomp(rm)->getStuff(); t; t = t->nextThing) {
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
      for (t = real_roomp(rm)->getStuff(); t; t = t->nextThing) {
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
      for (t = real_roomp(rm)->getStuff(); t; t = t->nextThing) {
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

      add_bars(temp);


      Hi = tbt->getHit();

      if(ch->in_room == 15344) {
	
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
      //      vlogf(LOG_DASH, "Brightmoon Defense: %s shooting at %s (%d.%s)", ch->getName(), tbt->getName(), numsimilar, temp);

      strcpy(temp, tbt->getName());

      ch->doShoot(buf);
      //      vlogf(LOG_DASH, "archer shooting at a target");
      Hf = tbt->getHit();

#if 1
      //      vlogf(LOG_DASH, "archer debug: %d->%d, temp/name: (%s)/(%s), tbt?: %s",
      //    Hi, Hf, temp, (tbt->getName() ? tbt->getName() : "(NULL)"), (tbt ? "exists" : "(NULL)"));
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

int adventurer(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  bool fighting=false;
  TThing *t;
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
  for(t=myself->roomp->getStuff();t;t=t->nextThing){
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

  TThing *t, *t2=NULL;
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
    t= myself->getStuff();
    while(t) {
      if((glass=dynamic_cast<TBaseCup *>(t))){
	t=t->nextThing;
	--(*glass);
        delete glass;
        glass = NULL;
        myself->addToWait(2);
      } else {
	t=t->nextThing;
      }
      
    }
    myself->addToWait(50);
    return TRUE;
  }
  
  for(t=myself->roomp->getStuff();t;t=t->nextThing){
    if((table=dynamic_cast<TTable *>(t))){
      
      for(t2=table->rider;t2;t2=t2->nextRider){
	if((glass=dynamic_cast<TBaseCup *>(t2))){
	  //	  vlogf(LOG_DASH, "Barmaid: found  %s with %d units left.", glass->getName(), glass->getDrinkUnits());
	  if (glass->getDrinkUnits() <= 0) {
	    //	    vlogf(LOG_DASH, "Barmaid: found empty %s on %s.", glass->getName(), table->getName());

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
      //      vlogf(LOG_DASH, "Barmaid: found  %s with %d units left.", glass->getName(), glass->getDrinkUnits());
      if(glass->getDrinkUnits() <= 0) {
	//	vlogf(LOG_DASH, "Barmaid: found empty  %s.", glass->getName());
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



extern int cityguard(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *);
extern int tudy(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *);
extern int moneyTrain(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *);
extern int factionRegistrar(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int grimhavenPosse(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int coroner(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);


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
  {TRUE, "fighter", fighter},
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
  {FALSE, "mage guildmaster", MageGuildMaster},
  {FALSE, "deikhan guildmaster", DeikhanGuildMaster},        // 30 
  {FALSE, "monk guildmaster", MonkGuildMaster},
  {FALSE, "warrior guildmaster", WarriorGuildMaster},
  {FALSE, "thief guildmaster", ThiefGuildMaster},
  {FALSE, "cleric guildmaster", ClericGuildMaster},
  {FALSE, "ranger guildmaster", RangerGuildMaster},        // 35 
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
  {FALSE,"Trainer: physical", CDGenericTrainer},       // 120 
  {FALSE,"Trainer: smythe", CDGenericTrainer}, 
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
  {FALSE,"Trainer: ranger fight", CDGenericTrainer},
  {FALSE,"shaman guildmaster", ShamanGuildMaster},
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
  {FALSE, "BOGUS", bogus_mob_proc},
  {FALSE, "Coroner", coroner},
  {FALSE, "Faction Registrar", factionRegistrar},
  {FALSE, "Trainer: defense", CDGenericTrainer},
  {FALSE, "Scared Kid", scaredKid},               // 160
  {FALSE, "stock broker", stockBroker},
  {FALSE,"Trainer: psionics", CDGenericTrainer},
  {TRUE, "Divination Man", divman},
  {FALSE,"Trainer: healing abilities", CDGenericTrainer},           // 164
  {FALSE, "Gardener", gardener}, // 165
  {FALSE, "Brightmoon Archer", bmarcher},
  {FALSE, "Money Train", moneyTrain},
  {FALSE, "Adventurer", adventurer},
  {FALSE, "Trainer: iron body", CDGenericTrainer},
  {FALSE, "Tudy", tudy},
  {TRUE, "Barmaid", barmaid},
  {FALSE, "tattoo artist", tattooArtist},
// replace non-zero, bogus_mob_procs above before adding
};


