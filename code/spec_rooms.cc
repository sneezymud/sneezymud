//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      spec_rooms.cc : special procedures for rooms
//
///////////////////////////////////////////////////////////////////////////

// cmd = CMD_ROOM_ENTERED
//      if ch dies, leave ch valid and return DELETE_THIS
//
// cmd = CMD_GENERIC_PULSE
//
//  cmd = normal
//      return DELETE_VICT to kill ch


#include <cmath>

#include "stdsneezy.h"
#include "statistics.h"
#include "obj_food.h"
#include "obj_note.h"
#include "obj_portal.h"
#include "spec_rooms.h"


int TRoom::checkSpec(TBeing *ch, cmdTypeT cmd, const char *arg, TThing *)
{
  int rc;

  if (funct) {
    rc = (*funct) (ch, cmd, arg, this);
    return rc;
  }
  return FALSE;
}

struct room_special_proc_entry {
  int vnum;
  int (*proc) (TBeing *, cmdTypeT, const char *, TRoom *);
};

int oft_frequented_room(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TBeing *mob;
  int i, q;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  switch (rp->number) {
    case ROOM_KINDRED_INN:
    case ROOM_GREEN_DRAGON_INN:
      // enter between 7PM and 10PM
      if ((time_info.hours >= 19) && (time_info.hours <= 22)) {
        q = ::number(1, 2);
        for (i = 1; i <= q; i++) {
          if (::number(0,10))
            continue;
          int rmob = real_mobile(::number(0,1) ? MOB_MALE_HOPPER : MOB_FEMALE_HOPPER);  
          if (mob_index[rmob].getNumber() >= mob_index[rmob].max_exist)
             continue;
          mob = read_mobile(rmob, REAL);
          *rp += *mob;
	  *rp << *mob;
          act("$n saunters into the bar anticipating $s first ale of the evening.", FALSE, mob, NULL, NULL, TO_ROOM);
        }
      }
      break;
    case ROOM_PEW1:
    case ROOM_PEW2:
    case ROOM_PEW3:
    case ROOM_PEW4:
      if (!((time_info.day + 1) % 7) &&  // on Sunday
          (time_info.hours == 10)) {  // at 10
        for (i = 1; i <= 8; i++) {
          if (::number(0,9))
            continue;
          int rmob = real_mobile(::number(0,1) ? MOB_MALE_CHURCH_GOER : MOB_FEMALE_CHURCH_GOER);  
          if (mob_index[rmob].getNumber() >= mob_index[rmob].max_exist)
             continue;
          mob = read_mobile(rmob, REAL);
          *rp += *mob;
          act("$n quietly sits down on the pew here for church.", FALSE, mob, NULL, NULL, TO_ROOM);
        }
      }
      break;
    case ROOM_TOP_OF_TREE:
      // april 4th at noon
      if ((time_info.month == 3) && (time_info.day == 3) && (time_info.hours == 12)) {
        int rom = real_mobile(MOB_SONGBIRD);
        if (mob_index[rom].getNumber() > 100)
          break;
        for (i = 1; i <= 200; i++) {
          mob = read_mobile(rom, REAL);
          *rp += *mob;
        }
        act("It's a once-a-year event! And you're here to witness it!",
                    FALSE, mob, NULL, NULL, TO_ROOM);
	act("Two hundred songbirds descend from the skies and land on the big tree!",
                     FALSE, mob, NULL, NULL, TO_ROOM);
      }
      break;
    default:
      vlogf(LOG_PROC, fmt("Room %d has an oft_frequented_room() with no code for it.") % 
               rp->number);
      break;
  }
  return FALSE;
}

int isBelimusAlive(void)
{
  if (mob_index[real_mobile(MOB_BELIMUS)].getNumber() >= 1)
    return TRUE;
  return FALSE;
}

int lava_room(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  if ((cmd != CMD_GENERIC_PULSE) || number(0, 9))
    return FALSE;

  rp->sendTo(fmt("A scorching burst of lava spits from a crevice in the %s!\n\r") % rp->describeGround());

#if 0
  for (k = rp->contents; k; k = next) {
    next = k->next_content;
    if (!number(0, 5)) {
      strcpy(buf, k->shortDescr);
      sendrpf(rp, "%s is splashed by the lava.", cap(buf));
    }
  }
#endif
  return FALSE;
}

void TObj::thingDumped(TBeing *ch, int *value)
{
  int val;

  act("$p vanishes in a puff of smoke.", TRUE, ch, this, 0, TO_ROOM);
  act("$p vanishes in a puff of smoke.", TRUE, ch, this, 0, TO_CHAR);

  // this will take struct and current street value into account
  if (!isObjStat(ITEM_NEWBIE)) {
    val = adjPrice();

    *value += min(1000, max(1, (int) (val *
           ch->plotStat(STAT_CURRENT, STAT_KAR, 0.05, 0.25, 0.18))));
  }

  delete this;
}

void TNote::thingDumped(TBeing *ch, int *)
{
  act("$p vanishes in a puff of smoke.", TRUE, ch, this, 0, TO_ROOM);
  act("$p vanishes in a puff of smoke.", TRUE, ch, this, 0, TO_CHAR);

  delete this;
}

int grimhavenDump(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  TThing *t, *t2;
  TRoom *roomp;

  if(cmd!=CMD_GENERIC_PULSE)
    return FALSE;

  if(!(roomp=real_roomp(18975))){
    vlogf(LOG_BUG, "couldn't find sewage pipe in grimhavenDump!");
    return FALSE;
  }
    

  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    
    // Only objs get nuked
    TObj *obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    
    // portals should not be nuked
    if (dynamic_cast<TPortal *>(obj))
      continue;
    
    if (obj->isObjStat(ITEM_NOJUNK_PLAYER))
      continue;
    
    // nor should flares
    if (obj->objVnum() == GENERIC_FLARE)
      continue;
    
    sendrpf(rp, "A %s slides down the chute into the disposal pipe below.\n\r", fname(obj->name).c_str());
    
    --(*obj);
    *roomp += *obj;
  }
  return FALSE;
}

int prisonDump(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  TThing *t, *t2;
  TRoom *roomp;

  if(cmd!=CMD_GENERIC_PULSE)
    return FALSE;

  if(!(roomp=real_roomp(31904))){
    vlogf(LOG_BUG, "couldn't find sewage pipe in prisonDump!");
    return FALSE;
  }
    

  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    
    // Only objs get nuked
    TObj *obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    
    // portals should not be nuked
    if (dynamic_cast<TPortal *>(obj))
      continue;
    
    if (obj->isObjStat(ITEM_NOJUNK_PLAYER))
      continue;
    
    // nor should flares
    if (obj->objVnum() == GENERIC_FLARE)
      continue;
    
    sendrpf(rp, "A %s slides down the chute into the disposal pipe below.\n\r", fname(obj->name).c_str());
    
    --(*obj);
    *roomp += *obj;
  }
  return FALSE;
}

int dump(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  TThing *t, *t2;
  int value = 0;
  int rc;
  bool wasProp = false;

  if (cmd == CMD_GENERIC_PULSE) {
    for (t = rp->getStuff(); t; t = t2) {
      t2 = t->nextThing;

      // Only objs get nuked
      TObj *obj = dynamic_cast<TObj *>(t);
      if (!obj)
        continue;

      // portals should not be nuked
      if (dynamic_cast<TPortal *>(obj))
        continue;

      if (obj->isObjStat(ITEM_NOJUNK_PLAYER))
	continue;

      // nor should flares
      if (obj->objVnum() == GENERIC_FLARE)
        continue;

      // refuse haulers cart
      if(obj->objVnum() == 33270)
	continue;

      sendrpf(rp, "A %s vanishes in a puff of smoke.\n\r", fname(obj->name).c_str());

      obj->logMe(NULL, "Dump nuking");

      delete obj;
      obj = NULL;
    }
    return FALSE;
  } else if (cmd != CMD_DROP)
    return FALSE;

  if (!ch)
    return FALSE;

  rc = ch->doDrop(arg, NULL);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;

    if (isname("[prop]", t->getName()))
      wasProp = true;

    TObj *obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;


    if (obj->isObjStat(ITEM_NOJUNK_PLAYER))
      continue;

    t->thingDumped(ch, &value);
    // t may be invalid afterwards
  }
  if (value && !wasProp) {
    ch->sendTo("You are rewarded for outstanding performance.\n\r");
    act("$n has been rewarded for good citizenship.", TRUE, ch, 0, 0, TO_ROOM);

    if (ch->GetMaxLevel() < 3)
      gain_exp(ch, min(0.010, value/1000.0), -1);
    else { 
      // take the global income modifier into account, in times of drought, we
      // don't want folks resorting to using the dump to get their money
      value = (int) (value * gold_modifier[GOLD_INCOME].getVal());
      ch->addToMoney(value, GOLD_DUMP);
    }
  }
  return TRUE;
}


int pools_of_chaos_and_cleansing(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *)
{
  int rc;

  if ((cmd != CMD_ENTER) || !ch->awake())
    return FALSE;

  for (; isspace(*arg); arg++);

  if (is_abbrev(arg, "chaos")) {
    ch->sendTo("You slowly dip yourself into the pool of chaos.\n\r");
    act("$n slowly dips $mself into the pool of chaos.",
              TRUE, ch, NULL, NULL, TO_ROOM);
    if (ch->isImmortal())
      return TRUE;

    switch (number(0, 10)) {
      case 0:
	ch->sendTo("As you step into the pool, you suddenly feel much weaker.\n\r");
	act("$n shivers as the water in the pool weakens $m.", TRUE, ch, NULL, NULL, TO_ROOM);
	ch->setHit(1);
	return TRUE;
      case 1:
	ch->sendTo("As you step into the pool, your mind becomes clouded.\n\r");
	act("$n looks disorientated as $e steps into the pool.", TRUE, ch, NULL, NULL, TO_ROOM);
	ch->setMana(0);
	return TRUE;
      case 2:
	ch->sendTo("As you step into the pool, you feel your vision disappear.\n\r");
	act("$n looks around blindly as $e steps into the pool.", TRUE, ch, NULL, NULL, TO_ROOM);

        ch->rawBlind(50, 24 * UPDATES_PER_MUDHOUR, SAVE_YES);
	return TRUE;
      case 3:
	ch->sendTo("As you step into the pool, you feel yourself magically moved.\n\r");
	act("$n magically disappears as soon as $e steps into the pool.", TRUE, ch, NULL, NULL, TO_ROOM);

        rc = ch->genericTeleport(SILENT_NO);

        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
	return TRUE;
      case 4:
	ch->sendTo("As you step into the pool, you feel totally refreshed!\n\r");
	act("$n looks totally refreshed as $e steps into the pool.", TRUE, ch, NULL, NULL, TO_ROOM);
	ch->setHit(ch->hitLimit());
	return TRUE;
      case 5:
	ch->sendTo("As you step into the pool, many thoughts rush into your head.\n\r");
	act("$n looks a little dizzy as $h steps into the pool.", TRUE, ch, NULL, NULL, TO_ROOM);
	ch->setMana(ch->manaLimit());
	return TRUE;
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      default:
	return TRUE;
    }
  } else if (is_abbrev(arg, "cleansing")) {
    ch->sendTo("You slowly dip yourself into the pool of cleansing.\n\r");
    act("$n slowly dips $mself into the pool of cleansing.", TRUE, ch, NULL, NULL, TO_ROOM);
    return TRUE;
  }
  return FALSE;
}

int goblin_smoke_amber(TBeing *, cmdTypeT cmd, const char *, TRoom *)
{
  if ((cmd != CMD_GENERIC_PULSE) || ::number(0,9))
    return FALSE;

#if 0
  for (vict = rp->people; vict; vict = temp) {
    temp = vict->next_in_room;
    if (vict->isImmortal())
      continue;
    if (!number(0, 6)) {
      vict->sendTo("The smoke in the room denies you from breathing!\n\r");
      vict->sendTo("You suddenly feel much weaker because of the lack or air.\n\r");
      act("$n coughs and wheezes as the smoke cause $m to choke.", TRUE, vict, NULL, NULL, TO_ROOM);
      vict->reconcileDamage(vict, 10,DAMAGE_SUFFOCATE);
    }
  }
#endif
  return FALSE;
}

int faerie_mound(TBeing *, cmdTypeT, const char *, TRoom *)
{
  return FALSE;
}

// this is a crappy proc but will work in a pinch
int personalHouse(TBeing *ch, cmdTypeT cmd, const char *, TRoom *rp)
{
  char buf[80];
  sh_int save_room;
  objCost cost;
  TPerson *pers;

  if ((cmd != CMD_RENT) && (cmd != CMD_SNAP))
   return FALSE;

  pers = dynamic_cast<TPerson *>(ch);
  if (!pers)
    return FALSE;

  sprintf(buf, fname(rp->name).c_str());

  if (strcmp(buf, pers->getName()))
    return FALSE;

  pers->sendTo("You snap your fingers with authority.\n\r");
  act("$n snaps $s fingers with authority.",
        TRUE, pers, 0,0, TO_ROOM);

  pers->recepOffer(pers, &cost);
  pers->sendTo("Your house swallows you whole.\n\r");
  act("Uh oh, $n's house just swallowed $m.",
        TRUE, pers, 0, 0, TO_ROOM);

  pers->cls();
  pers->fullscreen();

  pers->saveRent(&cost, TRUE, 2);
  save_room = pers->in_room;  /* backup what room the PC was in */
  pers->saveChar(save_room);
  pers->in_room = save_room;

  pers->preKillCheck(TRUE);

  return DELETE_VICT;
}

int Whirlpool(TBeing *ch, cmdTypeT cmd, const char *, TRoom *rp)
{
  TThing *t, *t2;
  TRoom *rp2;
  int new_room;
  int rc;

  if (cmd == CMD_GENERIC_PULSE) {
    if (!rp->getStuff())
      return FALSE;

    // transport stuff out of here
    for (t = rp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      TBeing *tch = dynamic_cast<TBeing *>(t);
      if (tch && tch->isImmortal())
        continue;

      for (new_room = ::number(12500, 13100);;
           new_room = ::number(12500, 13100)) {
        if (!(rp2 = real_roomp(new_room)) || 
            !(rp2->isWaterSector()))
          continue;
        break;
      }
      --(*t);
      *rp2 += *t;
    }

    return FALSE;
  }

  // 50% chance of death

  if (cmd != CMD_ROOM_ENTERED)
   return FALSE;

  if (ch->isImmortal())
    return FALSE;

  if ((ch->getRace() == RACE_DWARF) || (::number(1,100) <= 50)) {
    act("The whirlpool tears $n limb from limb.", TRUE, ch, 0, 0, TO_ROOM);
    ch->sendTo("The whirlpool tears you limb from limb.\n\r");
    ch->rawKill(DAMAGE_WHIRLPOOL);
    return DELETE_THIS;
  } else {
    act("$n miraculously manages to swim out of the whirlpool's clutches.", TRUE, ch, 0, 0, TO_ROOM);
    ch->sendTo("Miraculously, you somehow manage to escape the whirlpool's clutches!\n\r");
    ch->sendTo("You desparately swim for the surface with your lungs burning for a breath!\n\r");
    if (ch->riding) {
      rc = ch->fallOffMount(ch->riding, POSITION_STANDING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      }
    }
    while ((t = ch->rider)) {
      rc = t->fallOffMount(ch, POSITION_STANDING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete t;
        t = NULL;
      }
    }

    --(*ch);

    for (new_room = ::number(12500, 13100);;new_room = ::number(12500, 13100)) {
      if (!(rp2 = real_roomp(new_room)) || 
          !(rp2->isWaterSector()))
        continue;
      break;
    }
    *rp2 += *ch;
    return TRUE;
  }
}

int belimusThroat(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TThing *t, *t2;
  TBeing *ch = NULL;

  if ((cmd != CMD_GENERIC_PULSE) || ::number(0,9))
   return FALSE;

  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    ch = dynamic_cast<TBeing *>(t);
    if (!ch)
      continue;

    if (ch->isImmortal())
      continue;

    if (!isBelimusAlive()) {
      ch->sendTo("Belimus has expired and the weight of his corpse collapses upon you!!\n\r");
      ch->sendTo("Man, sucks to be buried in blubber...\n\r");
      ch->rawKill(DAMAGE_NORMAL);
      delete ch;
      ch = NULL;
      continue;
    }

    ch->sendTo("Belimus's throat muscles constrict slightly.\n\r");

    int dam = ::number(3,5);
    if (ch->reconcileDamage(ch, dam, DAMAGE_SUFFOCATION) == -1) {
      delete ch;
      ch = NULL;
    }
  }

  if (rp->number == 13480) {
    TRoom *rp2;
    roomDirData *exitp, *back;
    exitp = rp->dir_option[DIR_NORTH];
    rp2 = real_roomp(exitp->to_room);
    back = rp2->dir_option[DIR_SOUTH];
    if (IS_SET(exitp->condition, EX_CLOSED)) {
      if (!::number(0,60)) {
        sendrpf(rp, "A rumbling is heard as the windpipe opens.\n\r");
        sendrpf(rp2, "A rumbling is heard as the windpipe opens.\n\r");
        REMOVE_BIT(exitp->condition, EX_CLOSED);
        REMOVE_BIT(back->condition, EX_CLOSED);
      }
    } else {
      if (!::number(0,20)) {
        sendrpf(rp, "A rumbling is heard as the windpipe closes.\n\r");
        sendrpf(rp2, "A rumbling is heard as the windpipe closes.\n\r");
        SET_BIT(exitp->condition, EX_CLOSED);
        SET_BIT(back->condition, EX_CLOSED);
      }
    }
  }
  return TRUE;
}

int belimusStomach(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TThing *t, *t2;
  TBeing *ch = NULL;
  int rc;

  if ((cmd != CMD_GENERIC_PULSE) || ::number(0,9))
   return FALSE;

  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    ch = dynamic_cast<TBeing *>(t);
    if (!ch)
      continue;

    if (!isBelimusAlive()) {
      ch->sendTo("Belimus has expired and the weight of his corpse collapses upon you!!\n\r");
      ch->sendTo("Man, sucks to be buried in blubber...\n\r");
      ch->rawKill(DAMAGE_NORMAL);
      delete ch;
      ch = NULL;
      continue;
    }

    if (ch->isImmortal())
      continue;

    ch->sendTo("The acid in Belimus's stomach corrodes you.\n\r");
    rc = ch->acidEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete ch;
      ch = NULL;
      continue;
    }

    int dam = ::number(3,5);
    if (ch->reconcileDamage(ch, dam, DAMAGE_ACID) == -1) {
      delete ch;
      ch = NULL;
      continue;
    }
  }
  if (rp->number == 13490) {
    TRoom *rp2;
    roomDirData *exitp;
    exitp = rp->dir_option[DIR_SOUTH];
    rp2 = real_roomp(exitp->to_room);
    if (IS_SET(exitp->condition, EX_CLOSED)) {
      if (!::number(0,60)) {
        sendrpf(rp, "A rumbling is heard as the intestines distend.\n\r");
        sendrpf(rp2, "A rumbling is heard as the intestines distend.\n\r");
        REMOVE_BIT(exitp->condition, EX_CLOSED);
      }
    } else {
      if (!::number(0,20)) {
        sendrpf(rp, "A rumbling is heard as the intestines contract.\n\r");
        sendrpf(rp2, "A rumbling is heard as the intestines contract.\n\r");
        SET_BIT(exitp->condition, EX_CLOSED);
      }
    }
  }
  return TRUE;
}

int belimusLungs(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TThing *t, *t2;
  TBeing *ch = NULL;

  if ((cmd != CMD_GENERIC_PULSE) || ::number(0,9))
   return FALSE;

  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    ch = dynamic_cast<TBeing *>(t);
    if (!ch)
      continue;

    if (!isBelimusAlive()) {
      ch->sendTo("Belimus has expired and the weight of his corpse collapses upon you!!\n\r");
      ch->sendTo("Man, sucks to be buried in blubber...\n\r");
      ch->rawKill(DAMAGE_NORMAL);
      delete ch;
      ch = NULL;
      continue;
    }

    if (ch->isImmortal())
      continue;

    ch->sendTo("Belimus inhales and you are buffeted by the wind.\n\r");

    int dam = ::number(3,5);
    if (ch->reconcileDamage(ch, dam, DAMAGE_GUST) == -1) {
      delete ch;
      ch = NULL;
    }
  }
  if (rp->number == 13496) {
    TRoom *rp2;
    roomDirData *exitp, *back;
    exitp = rp->dir_option[DIR_WEST];
    rp2 = real_roomp(exitp->to_room);
    back = rp2->dir_option[DIR_EAST];
    if (IS_SET(exitp->condition, EX_CLOSED)) {
      if (!::number(0,60)) {
        sendrpf(rp, "A rumbling is heard as Belimus inhales.\n\r");
        sendrpf(rp2, "A rumbling is heard as Belimus inhales.\n\r");
        REMOVE_BIT(exitp->condition, EX_CLOSED);
        REMOVE_BIT(back->condition, EX_CLOSED);
      }
    } else {
      if (!::number(0,20)) {
        sendrpf(rp, "A rumbling is heard as Belimus exhales.\n\r");
        sendrpf(rp2, "A rumbling is heard as Belimus exhales.\n\r");
        SET_BIT(exitp->condition, EX_CLOSED);
        SET_BIT(back->condition, EX_CLOSED);
      }
    }
  }
  if (rp->number == 13498) {
    TRoom *rp2;
    roomDirData *exitp, *back;
    exitp = rp->dir_option[DIR_EAST];
    rp2 = real_roomp(exitp->to_room);
    back = rp2->dir_option[DIR_WEST];
    if (IS_SET(exitp->condition, EX_CLOSED)) {
      if (!::number(0,60)) {
        sendrpf(rp, "A rumbling is heard as Belimus inhales.\n\r");
        sendrpf(rp2, "A rumbling is heard as Belimus inhales.\n\r");
        REMOVE_BIT(exitp->condition, EX_CLOSED);
        REMOVE_BIT(back->condition, EX_CLOSED);
      }
    } else {
      if (!::number(0,20)) {
        sendrpf(rp, "A rumbling is heard as Belimus exhales.\n\r");
        sendrpf(rp2, "A rumbling is heard as Belimus exhales.\n\r");
        SET_BIT(exitp->condition, EX_CLOSED);
        SET_BIT(back->condition, EX_CLOSED);
      }
    }
  }
  return TRUE;
}

int belimusBlowHole(TBeing *me, cmdTypeT cmd, const char *, TRoom *rp)
{
    TThing *t, *t2;
    TBeing *ch = NULL, *mob;

  if ((cmd != CMD_UP) && ((cmd != CMD_GENERIC_PULSE) || ::number(0,9)))
    return FALSE;

  if (cmd == CMD_GENERIC_PULSE) {
    for (t = rp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      ch = dynamic_cast<TBeing *>(t);
      if (!ch)
        continue;

      if (!isBelimusAlive()) {
        ch->sendTo("Belimus has expired and the weight of his corpse collapses upon you!!\n\r");
        ch->sendTo("Man, sucks to be buried in blubber...\n\r");
        ch->rawKill(DAMAGE_NORMAL);
        delete ch;
        ch = NULL;
        continue;
      }
    }
    return TRUE;
  }

  if (me && (cmd == CMD_UP)) {
    roomDirData *exitp;
    TRoom *rp2;

    if (!(exitp = rp->dir_option[DIR_UP]))
      return FALSE;
    if (IS_SET(exitp->condition, EX_CLOSED))
      return FALSE;

    for (mob = character_list;mob;mob = mob->next) {
      if (mob->mobVnum() == MOB_BELIMUS) {
        int room2 = mob->in_room;
        rp2 = real_roomp(room2);
        for (int i = 0; i <= 9; i++)
          if ((exitp = rp2->dir_option[i]) && (room2 = exitp->to_room))
            break;
        
        --(*me);
        thing_to_room(me, room2);
        act("$n is ejected from Belimus's blowhole.", 0, me, 0, 0, TO_ROOM);
        me->sendTo("You are ejected from the blowhole and land somewhere nearby.\n\r");
        return TRUE;
      }
    } 
    return FALSE;
  }

  return TRUE;
}

int wierdCircle(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  char buf[256];
  TBeing *mob;
  int mobnum = real_mobile(17111);  // 17111
  TObj *obj;

  if (mobnum < 0) {
    vlogf(LOG_PROC, "Bogus mob specified in wierdCircle.");
    return FALSE;
  }

  if (cmd == CMD_ENTER) {
    one_argument(arg, buf);
    if (is_abbrev(buf, "circle")) {
      if (mob_index[mobnum].getNumber() != 0)
        return FALSE;   // already loaded

      if (!(mob = read_mobile(mobnum, REAL))) {
        vlogf(LOG_PROC, "bad load of mob in wierdCircle");
        return FALSE;
      }
      *rp += *mob;
      mob->doSay("Hi there!  It's been quite some time since anyone's come to visit me.");
      act("$n looks in your direction.", TRUE, mob, 0, ch, TO_VICT);
      act("$n looks in $N's direction.", TRUE, mob, 0, ch, TO_NOTVICT);
      
      if (!(obj = ch->findMostExpensiveItem()))
        return TRUE;
      mob->doSay("And I see you've brought me a gift.");
      mob->doSay("Allow me to remove you of the burden.");
      
      if (obj->parent)
        --(*obj);
      else if (obj->eq_pos > WEAR_NOWHERE)
        ch->unequip(obj->eq_pos);

      *mob += *obj;

      return TRUE; 
    }
    return FALSE;
  }
  if (cmd == CMD_GENERIC_PULSE) {
    if (mob_index[mobnum].getNumber() && !::number(0,29)) {
      // keeps it around roughly 60 secs
      if (!(mob = get_char_room(mob_index[mobnum].name, rp->number)))
        return FALSE;
      if (!mob->fight()) {
        mob->doSay("Thanks!  You may leave me to my slumber.");
        while (mob->getStuff()) {
          delete mob->getStuff();
        }
        delete mob;
        return TRUE;
      }
    }
  }
  return FALSE;
}

#if 0
// this was written for a quest, works but no reason to keep it permanently
int elfForest(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  TRoom *new_room;
  TBeing *mob;
  TObj *arrow, *bow;
  char buf[256], capbuf[256];
  TBeing *vict;
  TThing *t;
  int found = FALSE;
  
  if ((cmd != CMD_GENERIC_PULSE) || ::number(0,1))
   return FALSE;

  for (t = rp->getStuff(), vict = NULL; t; t = t->nextThing, vict = NULL) {
    vict = dynamic_cast<TBeing *>(t);
    if (!vict)
      continue;
    if (vict->isAnimal() || vict->getRace() == RACE_WOODELF ||
        vict->isImmortal())
      continue;
    break;
  }

  if (!vict)
    return FALSE;

  if (!(mob = read_mobile(10113, VIRTUAL))) {
    return FALSE;
  }
  thing_to_room(mob, ROOM_VOID);   // safety net to prevent nowhere extract
  if (!(bow = read_object(10145, VIRTUAL))) {
    delete mob;
    return FALSE;
  }
  if (!(arrow = read_object(3412, VIRTUAL))) {
    delete mob;
    delete bow;
    return FALSE;
  }
  arrow->obj_flags.decay_time = 3;
  strcpy(capbuf, mob->getName());
  
  for (dirTypeT door = MIN_DIR; door < MAX_DIR; door++) {
    if (rp->dir_option[door] && 
            (new_room = real_roomp(rp->dir_option[door]->to_room))) {
      if (new_room == rp || ::number(0,2)) 
        continue;
      found = FALSE;
      for (t = new_room->getStuff(); t && !found; t = t->nextThing) {
        TBeing *tbt = dynamic_cast<TBeing *>(t);
        if (tbt && tbt->isPc() && !tbt->isImmortal())
           found = TRUE;
      }
      if (!found) {
        --(*mob);  // move out of void
        thing_to_room(mob, rp->dir_option[door]->to_room);
        *mob += *arrow;
        *mob += *bow;
        mob->doBload("bow arrow", 0);
        mob->setSkillValue(SKILL_RANGED_SPEC, MAX_SKILL_LEARNEDNESS);
         
        sprintf(buf, "%s leaps from the trees to the %s and draws %s %s.\n\r",
             cap(capbuf), dirs[door], mob->hshr(), fname(bow->name).c_str());
        sendrpf(rp, buf);

        sprintf(buf, "%s %s 1", fname(vict->name).c_str(), dirs[rev_dir[door]]);
        mob->doShoot(buf, 0);
        sprintf(buf, "%s scrambles back into the brush.\n\r", cap(capbuf));
        sendrpf(rp, buf);
        // vict is possibly invalid here.
        delete bow;
        delete mob;
        return FALSE;
      }
    }
  }
  delete arrow;
  delete bow;
  delete mob;
  return FALSE;
}
#endif

int noiseBoom(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TRoom *new_room;
  
  if ((cmd != CMD_GENERIC_PULSE) || ::number(0,99))
   return FALSE;

  sendrpf(rp, "BOOM!!\n\r");

  for (dirTypeT door = MIN_DIR; door < MAX_DIR; door++) {
    if (rp->dir_option[door] && (new_room = 
            real_roomp(rp->dir_option[door]->to_room)))
      if (new_room != rp)
        sendrpf(new_room, "An eerie moaning sound echoes through the tunnel.");
  }
  return FALSE;
}

// FALSE if didn't slide
// DELETE_THIS on death
int genericSlide(TThing *t, TRoom *rp)
{
  TThing *t2, *t3;
  int rc;

  if (t->isLevitating() || t->isFlying())
    return FALSE;

  t->sendTo("You seem to be sliding down the incline!!!\n\r");
  act("$n slides down the incline.", TRUE, t, 0, 0, TO_ROOM); 

  for (t2 = t->rider; t2; t2 = t3) {
    t3 = t2->nextRider;
    rc = genericSlide(t2, rp);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t2;
      t2 = NULL;
    }
  }

  (*t)--;
  switch (rp->number) {
    case 11347:
      thing_to_room(t, 11349);
      break;
    case 11349:
      thing_to_room(t, 11351);
      break;
    case 11351:
      thing_to_room(t, 11353);
      break;
    case 11423:
      thing_to_room(t, 11422);
      break;
    case 20593:
      thing_to_room(t, 20412);
      break;
    case 20594:
      thing_to_room(t, 20593);
      break;
    case 20597:
      thing_to_room(t, 20594);
      break;
    default:
      vlogf(LOG_PROC, fmt("Bogus room for generic slide %d") %  rp->number);
      thing_to_room(t, ROOM_VOID);
  }

  TBeing *tbt = dynamic_cast<TBeing *>(t);
  if (tbt)
    tbt->doLook("", CMD_LOOK);

  act("$n slides into the room.",
      TRUE, t, 0, 0, TO_ROOM);
  
  rc = t->genericMovedIntoRoom(t->roomp, -1);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_THIS;
  }

  // a special case for Mithros's Penguins
  if (tbt && tbt->in_room == 20412) {
    // at end of slide
    if (tbt->mobVnum() == MOB_PENGUIN_ADULT ||
        tbt->mobVnum() == MOB_PENGUIN_YOUNG) {
      act("You quickly dive under the water and swim away.",
        FALSE, tbt, 0, 0, TO_CHAR);
      act("$n quickly dives under the water and swims away.",
        FALSE, tbt, 0, 0, TO_ROOM);

      --(*tbt);
      thing_to_room(tbt, 20597);

      act("You find your way into a new room.",
        FALSE, tbt, 0, 0, TO_CHAR);
      act("From high above you, $n suddenly slides into the room.",
        FALSE, tbt, 0, 0, TO_ROOM);
    }
  }

  return TRUE;
}

int slide(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TThing *t, *t2;
  int rc;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;

    if (t->riding)
      continue;

    if (::number(0,2))
      return FALSE;

    rc = genericSlide(t, rp);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t;
      t = NULL;
    }
  }

  return TRUE;
}

int SecretPortalDoors(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  char buf[255];
  TObj *portal = NULL;
  int found = FALSE;
  TThing *i;
  TRoom *other_room = NULL;
  TThing * temp = NULL;
  int found_other = FALSE;

//  int destination = NULL;

  if (ch && cmd < MAX_CMD_LIST) {
    if (rp->number != ch->in_room) {
      vlogf(LOG_PROC,fmt("char %s not in proper room (SecretPortalDoors)") % ch->getName());
      return FALSE;
    }
    one_argument(arg,buf);
  }

  switch (rp->number) {
    case 7228:
      if (cmd != CMD_PULL && cmd != CMD_TUG)
        return FALSE;
      if (is_abbrev(buf, "lever") || is_abbrev(buf, "pole")) {
        act("As you pull the lever the lift begins to move with a loud creak.", TRUE,ch,0,0,TO_CHAR);
        act("As $n pulls the lever the lift begins to move with a loud creak.", TRUE,ch,0,0,TO_ROOM);
      } else
        return FALSE;
      if (obj_index[real_object(OBJ_MINELIFT_DOWN)].getNumber() >= 1) {
        act("Nothing seems to happen.", TRUE,ch,0,0,TO_CHAR);
        act("Nothing seems to happen.", TRUE,ch,0,0,TO_ROOM);
        return TRUE;
      }

      if (!(portal = read_object(OBJ_MINELIFT_DOWN, VIRTUAL))) {
        vlogf(LOG_PROC, fmt("Problem loading object in SecretPortal. (%d)") %  OBJ_MINELIFT_DOWN);
        ch->sendTo("Serious problem, contact a god.\n\r");
        return FALSE;
      }
      *ch->roomp += *portal;
      // loading the portal
      act("With a loud boom the entry platform is dropped as the lift arrives.",          TRUE,ch,portal,0,TO_CHAR);
      act("With a loud boom the entry platform is dropped as the lift arrives.",          TRUE,ch,portal,0,TO_ROOM);
      if (obj_index[real_object(OBJ_MINELIFT_UP)].getNumber() >= 1) {
        other_room = real_roomp(7266);
        temp = NULL;
        found_other = FALSE;
        if (other_room) {
          for (temp = other_room->getStuff(); temp; temp = temp->nextThing) {
            if (!dynamic_cast<TPortal *> (temp))
              continue;
            if (temp->number == real_object(OBJ_MINELIFT_UP)) {
              delete temp;
              temp = NULL;
              found_other = TRUE;
              break;
            }
          }
          if (found_other) {
            sendToRoom("With a loud boom, the entry platform disappears.\n\r", 7266);
          }
        }
      }
      return TRUE;
    case 7268:
      if (cmd == CMD_GENERIC_PULSE) {
        // automatic shift change at 6AM and 6PM
        if (time_info.hours != 6 && time_info.hours != 18)
          return FALSE;

        if (obj_index[real_object(7214)].getNumber())
          return TRUE;
  
        if (!(portal = read_object(7214, VIRTUAL))) {
          vlogf(LOG_PROC, fmt("Problem loading object in SecretPortal. (%d)") %  7214);
          ch->sendTo("Serious problem, contact a god.\n\r");
          return FALSE;
        }
        thing_to_room(portal, 7268);
        act("The drawbridge is lowered.", false, portal, 0, 0, TO_ROOM);

        // load into other room
        if (!(portal = read_object(7215, VIRTUAL))) {
          vlogf(LOG_PROC, fmt("Problem loading object in SecretPortal. (%d)") %  7215);
          ch->sendTo("Serious problem, contact a god.\n\r");
          return FALSE;
        }
        thing_to_room(portal, 7265);
        act("The drawbridge is lowered.", false, portal, 0, 0, TO_ROOM);

        return TRUE;
      } else if (cmd == CMD_LOWER) {
        if (!is_abbrev(buf, "bridge") && !is_abbrev(buf, "drawbridge")) {
          return FALSE;
        }
        int rob = real_object(7214);

        if (obj_index[rob].getNumber()) {
          act("The drawbridge is already lowered.", TRUE,ch,0,0,TO_CHAR);
          return TRUE;
        }
  
        act("You lower the drawbridge.", false, ch, 0, 0, TO_CHAR);
        act("$n lowers the drawbridge.", false, ch, 0, 0, TO_ROOM);

        if (!(portal = read_object(rob, REAL))) {
          vlogf(LOG_PROC, fmt("Problem loading object in SecretPortal. (%d)") %  7214);
          ch->sendTo("Serious problem, contact a god.\n\r");
          return FALSE;
        }
        *ch->roomp += *portal;

        // load into other room
        if (!(portal = read_object(7215, VIRTUAL))) {
          vlogf(LOG_PROC, fmt("Problem loading object in SecretPortal. (%d)") %  7215);
          ch->sendTo("Serious problem, contact a god.\n\r");
          return FALSE;
        }
        thing_to_room(portal, 7265);
        act("The drawbridge is lowered from the other side.", false, portal, 0, 0, TO_ROOM);

        return TRUE;
      } else if (cmd == CMD_RAISE) {
        if (!is_abbrev(buf, "bridge") && !is_abbrev(buf, "drawbridge")) {
          return FALSE;
        }
        int rob = real_object(7214);

        if (!obj_index[rob].getNumber()) {
          act("The drawbridge is already raised.", TRUE,ch,0,0,TO_CHAR);
          return TRUE;
        }
  
        act("You raise the drawbridge.", false, ch, 0, 0, TO_CHAR);
        act("$n raises the drawbridge.", false, ch, 0, 0, TO_ROOM);

        // remove it from this room
        for (temp = ch->roomp->getStuff(); temp; temp = temp->nextThing) {
          if (!dynamic_cast<TPortal *>(temp))
            continue;
          if (temp->number == rob) {
            delete temp;
            temp = NULL;
            break;
          }
        }
        // remove it from other room
        rob = real_object(7215);
        for (temp = real_roomp(7265)->getStuff(); temp; temp = temp->nextThing) {
          if (!dynamic_cast<TPortal *>(temp))
            continue;
          if (temp->number == rob) {
            act("$n is raised from the other side.", false, temp, 0, 0, TO_ROOM);
            delete temp;
            temp = NULL;
            break;
          }
        }
        return TRUE;
      }
      return FALSE;
    case 7266:
      if (cmd != CMD_PULL && cmd != CMD_TUG)
        return FALSE;
      if (is_abbrev(buf, "lever") || is_abbrev(buf, "pole")) {
        act("As you pull the lever the lift begins to move with a loud creak.",
            TRUE,ch,0,0,TO_CHAR);
        act("As $n pulls the lever the lift begins to move with a loud creak.",
            TRUE,ch,0,0,TO_ROOM);
      } else
        return FALSE;
      if (obj_index[real_object(OBJ_MINELIFT_UP)].getNumber() >= 1) {
        act("Nothing seems to happen.", TRUE,ch,0,0,TO_CHAR);
        act("Nothing seems to happen.", TRUE,ch,0,0,TO_ROOM);
        return TRUE;
      }

      if (!(portal = read_object(OBJ_MINELIFT_UP, VIRTUAL))) {
        vlogf(LOG_PROC, fmt("Problem loading object in SecretPortal. (%d)") %  OBJ_MINELIFT_DOWN);
        ch->sendTo("Serious problem, contact a god.\n\r");
        return FALSE;
      }
      *ch->roomp += *portal;
      // loading the portal
      act("With a loud boom the entry platform is dropped as the lift arrives.",
          TRUE,ch,portal,0,TO_CHAR);
      act("With a loud boom the entry platform is dropped as the lift arrives.",
          TRUE,ch,portal,0,TO_ROOM);
      if (obj_index[real_object(OBJ_MINELIFT_DOWN)].getNumber() >= 1) {
        other_room = real_roomp(7228);
        temp = NULL;
        found_other = FALSE;
        if (other_room) {
          for (temp = other_room->getStuff(); temp; temp = temp->nextThing) {
            if (!dynamic_cast<TPortal *> (temp))
              continue;
            if (temp->number == real_object(OBJ_MINELIFT_DOWN)) {
              delete temp;
              temp = NULL;
              found_other = TRUE;
              break;
            }
          }
          if (found_other) {
            sendToRoom("With a loud boom, the entry platform disappears.\n\r", 7228);
          }
        }
      } 

      return TRUE;
    case 15277:
      if (cmd != CMD_PULL && cmd != CMD_TUG)
        return FALSE;
      for (i = rp->getStuff(); i; i = i->nextThing) {
        TObj *io = dynamic_cast<TObj *>(i);
        if (io && io->objVnum() == ROOM_TREE_BRIDGE)  {
          portal = io;
          found = TRUE;
        }
      }

      if (is_abbrev(buf, "lever")) {
        act("You pull the lever.", TRUE,ch,0,0,TO_CHAR);
        act("$n pulls on a lever.", TRUE,ch,0,0,TO_ROOM);

        // deleting the portal
        if (found) {
          act("As you pull on the lever you hear a noise from the bridge.",
              TRUE,ch,0,0,TO_CHAR);
          act("The bridge rises until it looks just like one of the surrounding trees.", TRUE,ch,0,0,TO_CHAR);
          act("As $n pulls on the lever you hear a noise from the bridge.",
              TRUE,ch,0,0,TO_ROOM);
          act("The bridge rises until it looks just like one of the surrounding trees.", TRUE,ch,0,0,TO_ROOM);
          delete portal;
          portal = NULL;
          return TRUE;
        } else {
          // loading the portal
          act("As you pull on the lever some rope releases from above you.",
              TRUE,ch,0,0,TO_CHAR);
          act("One of the trees descends to make a bridge across the river.",                 TRUE,ch,0,0,TO_CHAR);
          act("As $n pull on the lever some rope releases from above.",
              TRUE,ch,0,0,TO_ROOM);
          act("One of the trees descends to make a bridge across the river.",                 TRUE,ch,0,0,TO_ROOM);
          portal = read_object(ROOM_TREE_BRIDGE, VIRTUAL);
          *ch->roomp += *portal;
          return TRUE;
        }
      }
      break;
    case 16173:
      if (cmd != CMD_PUSH && cmd != CMD_PRESS)
        return FALSE;
      if (is_abbrev(buf, "button")) {
        act("You push the button.", TRUE,ch,0,0,TO_CHAR);
        act("$n pushes the button.", TRUE,ch,0,0,TO_ROOM);
      } else
        return FALSE;
      if (obj_index[real_object(OBJ_FLAMING_PORTAL)].getNumber() >= 1) {
        act("Nothing seems to happen.", TRUE,ch,0,0,TO_CHAR);
        act("Nothing seems to happen.", TRUE,ch,0,0,TO_ROOM);
        return TRUE;
      }

      if (!(portal = read_object(OBJ_FLAMING_PORTAL, VIRTUAL))) {
        vlogf(LOG_PROC, fmt("Problem loading object in SecretPortal. (%d)") %  OBJ_FLAMING_PORTAL);
        ch->sendTo("Serious problem, contact a god.\n\r");
        return FALSE;
      }
      *ch->roomp += *portal;
      // loading the portal
      act("$p shimmers into view.", TRUE,ch,portal,0,TO_CHAR);
      act("$p shimmers into view.", TRUE,ch,portal,0,TO_ROOM);
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}


int getRandomRoom(){
  int to_room;
  TRoom *rp;
  int breakout=0;

  for (;;) {
    // this keeps getting caught in a loop on builder mud
    // and I don't want to fix it properly.
    if(++breakout>1000000){ // presumably we won't ever have > 1 mil rooms
      vlogf(LOG_BUG, "getRandomRoom got caught in a loop");
      return FALSE;
    }

    // note, all rooms below 100 are ignored

    to_room = ::number(100, top_of_world);

    if (!(rp = real_roomp(to_room)))
      continue;
    if (rp->isRoomFlag(ROOM_PRIVATE))
      continue;
    if (rp->isRoomFlag(ROOM_HAVE_TO_WALK))
      continue;
    if (rp->isFlyingSector())
      continue;
    if (zone_table[rp->getZoneNum()].enabled == FALSE)
      continue;

    break;
  }
  return to_room;
}

int randomMobDistribution(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  int exitrnum=0, rc;
  TMonster *tm;
  TThing *tmp_t;

  static int pulse;
  ++pulse;

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  // loop through all directions
  for(dirTypeT d=DIR_NORTH;d<MAX_DIR;d++){
    if(d==DIR_NORTHEAST || d==DIR_NORTHWEST || 
       d==DIR_SOUTHEAST || d==DIR_SOUTHWEST)
      continue;

    // remove the old exit, if any
    delete rp->dir_option[d];

    // choose a random location
    if(!(exitrnum=getRandomRoom()))
      continue;
      
    // create the exit
    if(!(rp->dir_option[d] = new roomDirData()))
      continue;
    
    // link the exit
    rp->dir_option[d]->to_room = exitrnum;
  }

  // this zone is inactive, so let's force a wanderAround
  if(zone_table[rp->getZoneNum()].zone_value==1){
    for(TThing *t=rp->getStuff();t;t=tmp_t){
      tmp_t = t->nextThing;  // just for safety
      
      if((tm=dynamic_cast<TMonster *>(t))){
	rc = tm->mobileActivity(pulse);
	if (IS_SET_DELETE(rc, DELETE_THIS)) {
	  tmp_t = t->nextThing;
	  delete tm;
	  tm = NULL;
	}
      }
    }
  }

  return TRUE;
}


int theKnot(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  static bool done[24];
  int n=rp->number-2375, exitrnum=0;
  
  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if(done[n] && ::number(0,9999))
    return FALSE;
  
  // loop through all directions
  for(dirTypeT d=DIR_NORTH;d<MAX_DIR;d++){
    if(d==DIR_NORTHEAST || d==DIR_NORTHWEST || 
       d==DIR_SOUTHEAST || d==DIR_SOUTHWEST)
      continue;

    // if we're redoing our exits and this is an out of zone exit, delete it
    if(rp->dir_option[d] && 
       rp->dir_option[d]->to_room<2375 &&
       rp->dir_option[d]->to_room>2399)
      delete rp->dir_option[d];

    // if no exit, chance to add new one
    if(!rp->dir_option[d] && !::number(0,9)){
      if(!(exitrnum=getRandomRoom()))
	continue;
      
      if(!(rp->dir_option[d] = new roomDirData()))
	continue;

      rp->dir_option[d]->to_room = exitrnum;
    }
  }

  //  vlogf(LOG_PEEL, fmt("the knot: did exits for room %i") %  rp->number);

  done[n]=true;
  return TRUE;
}

// Peel
int duergarWater(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  if(cmd != CMD_GENERIC_PULSE || !rp)
    return FALSE;

  if(rp->number>=13755 && rp->number<=13771){
    switch((time_info.hours)%12){
      case 0:
	if(rp->getSectorType()!=SECT_TEMPERATE_RIVER_SURFACE){
	  // send message here
	  rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
	}
	break;
      case 1: 
      case 2: 
      case 3: 
      case 4: 
      case 5: 
      case 6:
	if(rp->getSectorType()!=SECT_TEMPERATE_UNDERWATER){
	  // send message here
	  rp->setSectorType(SECT_TEMPERATE_UNDERWATER);
	}
	break;
      case 7:
	if(rp->getSectorType()!=SECT_TEMPERATE_RIVER_SURFACE){
	  // send message here
	  rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
	}
	break;
      case 8: 
      case 9: 
      case 10: 
      case 11:
	if(rp->getSectorType()!=SECT_TEMPERATE_CAVE){
	  // send message here
	  rp->setSectorType(SECT_TEMPERATE_CAVE);
	}
	break;
    }
  } else if(rp->number>=13738 && rp->number<=13754){
    switch((time_info.hours)%12){
      case 1:
	if(rp->getSectorType()!=SECT_TEMPERATE_RIVER_SURFACE){
	  //send message here
	  rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
	}
	break;
      case 2: 
      case 3: 
      case 4: 
      case 5:
	if(rp->getSectorType()!=SECT_TEMPERATE_UNDERWATER){
	  // send message here
	  rp->setSectorType(SECT_TEMPERATE_UNDERWATER);
	}
	break;
      case 6:
	if(rp->getSectorType()!=SECT_TEMPERATE_RIVER_SURFACE){
	  //send message here
	  rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
	}
	break;
      case 7: 
      case 8: 
      case 9: 
      case 10: 
      case 11: 
      case 0:
	if(rp->number<13740 &&
	   rp->getSectorType()!=SECT_TEMPERATE_ATMOSPHERE){
	  //send message here
	  rp->setSectorType(SECT_TEMPERATE_ATMOSPHERE);
	} else if(rp->number>=13740 &&
		  rp->getSectorType()!=SECT_TEMPERATE_CAVE){
	  //send message here
	  rp->setSectorType(SECT_TEMPERATE_CAVE);
	}
	break;
    }
    return FALSE;
  } else if(rp->number==13773){
    switch((time_info.hours)%12){
      case 2:
	if(rp->getSectorType()!=SECT_TEMPERATE_RIVER_SURFACE){
	  //send message here
	  rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
	}
	break;
      default:
	if(rp->getSectorType()!=SECT_TEMPERATE_ATMOSPHERE){
	  //send message here
	  rp->setSectorType(SECT_TEMPERATE_ATMOSPHERE);
	}
	break;
    }
    return FALSE;
  } else if(rp->number==13772){
    switch((time_info.hours)%12){
      case 2:
	if(rp->getSectorType()!=SECT_TEMPERATE_RIVER_SURFACE){
	  //send message here
	  rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
	}
	break;
      case 3:
	if(rp->getSectorType()!=SECT_TEMPERATE_UNDERWATER){
	  //send message here
	  rp->setSectorType(SECT_TEMPERATE_UNDERWATER);
	}
	break;
      default:
	if(rp->getSectorType()!=SECT_TEMPERATE_ATMOSPHERE){
	  //send message here
	  rp->setSectorType(SECT_TEMPERATE_ATMOSPHERE);
	}
	break;
    }
    return FALSE;
  } else if(rp->number==13731){
    switch((time_info.hours)%12){
      case 3:
	if(rp->getSectorType()!=SECT_TEMPERATE_RIVER_SURFACE){
	  //send message here
	  rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
	}
	break;
      default:
	if(rp->getSectorType()!=SECT_TEMPERATE_CAVE){
	  //send message here
	  rp->setSectorType(SECT_TEMPERATE_CAVE);
	}
        break;
    }
    return FALSE;
  }

  return TRUE;
}

int monkQuestProcLand(TBeing *ch, cmdTypeT cmd, const char *, TRoom *rp)
{
#if 0
  TMonster *tmon;
  TThing *t=NULL;
#endif

  if(cmd!=CMD_ROOM_ENTERED)
    return FALSE;
  if(!ch || !ch->hasQuestBit(TOG_MONK_GREEN_FALLING))
    return FALSE;
  if(ch->specials.last_direction != DIR_DOWN)
    return FALSE;

  // We figure if they are riding an elephant when they started the fall
  // and if there is an elephant in the room when they land, must be the
  // same one.  And if not, no big deal, probably not abusable.
#if 0
  for(t=rp->getStuff();t;t=t->nextThing){
    if((tmon=dynamic_cast<TMonster *>(t)) &&
       tmon->mobVnum()==MOB_ELEPHANT){
      break;
    }
  }
  if(!t)
    return FALSE;
#endif 

  ch->remQuestBit(TOG_MONK_GREEN_FALLING);
  ch->setQuestBit(TOG_MONK_GREEN_FALLEN);
  act("<c>Having successfully witnessed the elephants fall, you are now prepared to return to your guildmaster.<1>", FALSE, ch, NULL, NULL, TO_CHAR);
  return TRUE;
}

int monkQuestProcFall(TBeing *ch, cmdTypeT cmd, const char *, TRoom *rp)
{
  TMonster *tmon;

  if(cmd != CMD_ROOM_ENTERED)
    return FALSE;
  if(!ch->riding || !(tmon=dynamic_cast<TMonster *>(ch->riding)) ||
     tmon->mobVnum()!=MOB_ELEPHANT)
    return FALSE;
  if(!ch->hasQuestBit(TOG_MONK_GREEN_STARTED))
    return FALSE;

  ch->setQuestBit(TOG_MONK_GREEN_FALLING);
  ch->remQuestBit(TOG_MONK_GREEN_STARTED);
  act("<c>You urge your elephant over the precipice and prepare to observe its motions...<1>", FALSE, ch, NULL, NULL, TO_CHAR);

  return TRUE;
}

int BankVault(TBeing *, cmdTypeT cmd, const char *, TRoom *roomp)
{
  TRoom *rp;
  TThing *tt;
  TBeing *tb;
  int rc;

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if(::number(0,14))
    return FALSE;

  // close and lock vault doors
  //  vlogf(LOG_PEEL, "Bank: closing/locking vault doors");

  if(roomp->number==31780){
    if(!IS_SET(roomp->dir_option[DIR_WEST]->condition, EX_CLOSED)){
      SET_BIT(roomp->dir_option[DIR_WEST]->condition, EX_CLOSED);
      roomp->sendTo("The door to the west swings closed.\n\r");
    }
    if(!IS_SET(roomp->dir_option[DIR_WEST]->condition, EX_LOCKED)){
      SET_BIT(roomp->dir_option[DIR_WEST]->condition, EX_LOCKED);
      roomp->sendTo("The door to the west locks with an audible *click*.\n\r");
    }
    
    rp=real_roomp(31779);
    if(!IS_SET(rp->dir_option[DIR_EAST]->condition, EX_CLOSED)){
      SET_BIT(rp->dir_option[DIR_EAST]->condition, EX_CLOSED);
      rp->sendTo("The door to the east swings closed.\n\r");
    }
    if(!IS_SET(rp->dir_option[DIR_EAST]->condition, EX_LOCKED)){
      SET_BIT(rp->dir_option[DIR_EAST]->condition, EX_LOCKED);
      rp->sendTo("The door to the east locks with an audible *click*.\n\r");
    }
  }

  if(roomp->number==31786){
    if(!IS_SET(roomp->dir_option[DIR_WEST]->condition, EX_CLOSED)){
      SET_BIT(roomp->dir_option[DIR_WEST]->condition, EX_CLOSED);
      roomp->sendTo("The door to the west swings closed.\n\r");
    }
    if(!IS_SET(roomp->dir_option[DIR_WEST]->condition, EX_LOCKED)){
      SET_BIT(roomp->dir_option[DIR_WEST]->condition, EX_LOCKED);
      roomp->sendTo("The door to the west locks with an audible *click*.\n\r");
    }
    
    rp=real_roomp(31785);
    if(!IS_SET(rp->dir_option[DIR_EAST]->condition, EX_CLOSED)){
      SET_BIT(rp->dir_option[DIR_EAST]->condition, EX_CLOSED);
      rp->sendTo("The door to the east swings closed.\n\r");
    }
    if(!IS_SET(rp->dir_option[DIR_EAST]->condition, EX_LOCKED)){
      SET_BIT(rp->dir_option[DIR_EAST]->condition, EX_LOCKED);
      rp->sendTo("The door to the east locks with an audible *click*.\n\r");
    }
  }
  

  // check for player in this room and poison if so
  for(tt=roomp->getStuff();tt;tt=tt->nextThing){
    if((tb=dynamic_cast<TBeing *>(tt)) && tb->isPc()){
      tb->sendTo(COLOR_BASIC, "<G>Acidic gas shoots out of small holes in the ceiling.<1>\n\r");
      tb->sendTo(COLOR_BASIC, "<r>It burns your skin and you choke uncontrollably!<1>\n\r");

      //  vlogf(LOG_PEEL, fmt("Bank: %s caught in vault") %  tb->getName());

      rc=tb->reconcileDamage(tb, ::number(20,50), DAMAGE_TRAP_POISON);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
	delete tb;
	continue;
      }

      rc=tb->reconcileDamage(tb, ::number(20,50), DAMAGE_TRAP_ACID);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
	delete tb;
	continue;
      }

    }
  }


  return TRUE;
}


int BankMainEntrance(TBeing *, cmdTypeT cmd, const char *, TRoom *roomp)
{
  TRoom *rp;
  static unsigned int pulse;
  TBeing *mob, *boss;
  int i=0;
  bool found=false;
  Descriptor *d;
  int saferooms[]={31750, 31751, 31756, 31757, 31758, 31759, 31760, 31764,
  31788, 31789,31790,31791,31792,31793,31794,31795,31796,31797,31798,31799,-1};

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  ++pulse;
  if(pulse%180)
    return FALSE;

  // first, let's close entrance doors
  rp=real_roomp(31764);

  if(!IS_SET(rp->dir_option[DIR_NORTH]->condition, EX_CLOSED)){
    rp->sendTo("The door to the north swings closed.\n\r");
    SET_BIT(rp->dir_option[DIR_NORTH]->condition, EX_CLOSED);
  }
  if(!IS_SET(rp->dir_option[DIR_NORTH]->condition, EX_LOCKED)){
    rp->sendTo("The door to the north locks with an audible *click*.\n\r");
    SET_BIT(rp->dir_option[DIR_NORTH]->condition, EX_LOCKED);
  }
  if(!IS_SET(rp->dir_option[DIR_SOUTH]->condition, EX_CLOSED)){
    rp->sendTo("The door to the south swings closed.\n\r");
    SET_BIT(rp->dir_option[DIR_SOUTH]->condition, EX_CLOSED);
  }

  rp=real_roomp(31767);
  if(!IS_SET(rp->dir_option[DIR_SOUTH]->condition, EX_CLOSED)){
    rp->sendTo("The door to the south swings closed.\n\r");
    SET_BIT(rp->dir_option[DIR_SOUTH]->condition, EX_CLOSED);
  }
  if(!IS_SET(rp->dir_option[DIR_SOUTH]->condition, EX_LOCKED)){
    rp->sendTo("The door to the south locks with an audible *click*.\n\r");
    SET_BIT(rp->dir_option[DIR_SOUTH]->condition, EX_LOCKED);
  }

  rp=real_roomp(31758);
  if(!IS_SET(rp->dir_option[DIR_NORTH]->condition, EX_CLOSED)){
    rp->sendTo("The door to the north swings closed.\n\r");
    SET_BIT(rp->dir_option[DIR_NORTH]->condition, EX_CLOSED);
  }

  // now search for people in the zone and smite'em
  for (d = descriptor_list; d ; d = d->next){
    if (!d->connected && d->character && d->character->roomp &&
	d->character->roomp->getZoneNum() == rp->getZoneNum() &&
	!d->character->isImmortal()){
      found=true;

      for(i=0;saferooms[i]!=-1;++i){
	if(d->character->in_room == saferooms[i])
	  found=false;
      }

      break;
    }
  }

  
  if(found){
    vlogf(LOG_PEEL, "Bank: here comes the wrecking crew");
//QUEST - commented lines are for easier versions of mobs
    rp=real_roomp(31784);

    boss = read_mobile(31759, VIRTUAL);
//    boss = read_mobile(29218, VIRTUAL);
    *rp += *boss;
    SET_BIT(boss->specials.affectedBy, AFF_GROUP);

    for(i=0;i<4;++i){
      mob = read_mobile(31753+::number(0,3), VIRTUAL);
//      mob = read_mobile(29213+::number(0,3), VIRTUAL);
      *rp += *mob;
      boss->addFollower(mob);
      SET_BIT(mob->specials.affectedBy, AFF_GROUP);
    }
  }



  return TRUE;
}

int dayGateRoom(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TObj *to = NULL;
  TPortal *obj = NULL;
  TThing *t = NULL, *t2 = NULL;
  bool found = false;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  
  //  vlogf(LOG_DASH, "daygate proc PULSE");
  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    if(!(to=dynamic_cast<TObj *>(t)))
      continue;
    if(obj_index[to->getItemIndex()].virt == ITEM_DAYGATE) {
      found = true;
      break;
    }
  }
  //vlogf(LOG_DASH, fmt("daygate proc: found: %s") %  found ? "true" : "false");
  //vlogf(LOG_DASH, fmt("daygate proc: hmt: %d  suntime: %d") %  hourminTime() % sunTime(SUN_TIME_DAY));
  if (hourminTime() > 50   || hourminTime() < 46) {
    // code to remove gate
    if (found && to) {
      //     vlogf(LOG_DASH, "daygate proc found gate, removing");
      act("<Y>The radiant portal flares up once and is gone.<1>",
	  TRUE, to, 0, 0, TO_ROOM);
      --(*to);
      delete to;
    }
  } else {
    // code to place gate
    if (!found) {
      //      vlogf(LOG_DASH, "daygate proc didn't find gate, placing");
      if (!(to = read_object(ITEM_DAYGATE, VIRTUAL))) {
	vlogf(LOG_LOW, "Error loading daygate");
	return FALSE;;
      }
      obj = dynamic_cast<TPortal *>(to);
      if(rp->number == 1303)
	obj->setTarget(5700);
      if(rp->number == 5700)
	obj->setTarget(1303);
      obj->setPortalNumCharges(-1);
      obj->setPortalType(10);
      *rp += *to;
      act("<Y>A shimmering portal as bright as the noonday sun suddenly bursts into existance.<1>",
	  TRUE, to, 0, 0, TO_ROOM);
    }
    
  }

  return TRUE;
}

int moonGateRoom(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TObj *to = NULL;
  TPortal *obj = NULL;
  TThing *t = NULL, *t2 = NULL;
  bool found = false;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;


  
  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    if(!(to=dynamic_cast<TObj *>(t)))
      continue;
    if(obj_index[to->getItemIndex()].virt == ITEM_MOONGATE) {
      found = true;
      break;
    }
  }
  if ( hourminTime() > 2 && hourminTime() < 94) {
    // code to remove gate
    if (found && to) {
      //     vlogf(LOG_DASH, "moongate proc found moongate, removing");
      
      act("<k>The dark portal silently disperses into nothingness.<1>",
	  TRUE, to, 0, 0, TO_ROOM);
      --(*to);
      delete to;
    }
  } else {
    // code to place gate
    if (!found) {
        if (!(to = read_object(ITEM_MOONGATE, VIRTUAL))) {
	vlogf(LOG_LOW, "Error loading moongate");
	return FALSE;;
      }
      obj = dynamic_cast<TPortal *>(to);
      if(rp->number == 5895)
	obj->setTarget(28800);
	if(rp->number == 28800)
	  obj->setTarget(5895);
	obj->setPortalNumCharges(-1);
	obj->setPortalType(10);
	*rp += *to;
	//vlogf(LOG_DASH, "moongate proc didn't find moongate, placing");
	
	act("<k>A portal of midnight darkness suddenly shimmers into reality.<1>",
	    TRUE, to, 0, 0, TO_ROOM);
      }
    }
 
  return TRUE;
}

int waterfallRoom(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TObj *to = NULL;
  TThing *t = NULL, *t2 = NULL;
  bool found = false;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    if(!(to=dynamic_cast<TObj *>(t)))
      continue;
    if(obj_index[to->getItemIndex()].virt == OBJ_RAINBOW_MIST) {
      found = true;
      break;
    }
  }
  if (weather_info.sunlight != SUN_LIGHT || weather_info.sky != SKY_CLOUDLESS) {
    // code to remove rainbow
    if (!found || !to)
      return FALSE;
    act("<W>The sunlight no longer reaches the mist, and $p<W> fades from sight.<1>",
        TRUE, to, 0, 0, TO_ROOM);
    --(*to);
    delete to;

  } else {
    // code to place rainbow
    if (found || to)
      return FALSE;
    if (!(to = read_object(OBJ_RAINBOW_MIST, VIRTUAL))) {
      vlogf(LOG_LOW, "Error loading rainbow mist object");
      return FALSE;;
    }
    *rp += *to;
    act("<W>Suddenly, light from the sun strikes the mist, and $p<W> is formed.<1>",
	TRUE, to, 0, 0, TO_ROOM);
  

  }

  return TRUE;
}

int boulderRoom(TBeing *, cmdTypeT cmd, const char *, TRoom *roomp)
{
  TRoom *rp;
  TThing *t = NULL;
  TThing *t2 = NULL;
  TObj *rock;
  int found = 0;
  static unsigned int pulse;

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  ++pulse;
  if(pulse%150)
    return FALSE;

  for (t = roomp->getStuff();t;t = t2) {
    t2 = t->nextThing;
    if (!(rock=dynamic_cast<TObj *>(t)))
      continue;
    if (obj_index[rock->getItemIndex()].virt == BOULDER_ITEM) {
      found = 1;
      //      vlogf(LOG_LAPSOS, "found assigned to 1");
      break;
    }
  }

  if (found == 1) {
    //    vlogf(LOG_LAPSOS, fmt("!found so closing exit - found = %d") %  found);
    rp=real_roomp(4189);
    SET_BIT(rp->dir_option[DIR_DOWN]->condition, EX_CLOSED);
    rp=real_roomp(4284);
    REMOVE_BIT(rp->dir_option[DIR_UP]->condition, EX_CLOSED);
    return TRUE;
  } else {
    //    vlogf(LOG_LAPSOS, fmt("found so opening exit - found = %d") %  found);
    rp=real_roomp(4189);
    REMOVE_BIT(rp->dir_option[DIR_DOWN]->condition, EX_CLOSED);
    rp=real_roomp(4284);
    REMOVE_BIT(rp->dir_option[DIR_UP]->condition, EX_CLOSED);
    return TRUE;
  }
}

int collapsingTunnel(TBeing *ch, cmdTypeT cmd, const char *, TRoom *rp)
{

  if(cmd!=CMD_ROOM_ENTERED)
    return FALSE;

  act("<k>Rubble<1> collapses <k>behind you, blocking the way you came.<1>", FALSE, ch, NULL, NULL, TO_CHAR);
  int dam = ::number(21,40);
  if(::number(0,100) > ch->plotStat(STAT_CURRENT, STAT_DEX, 0, 100, 50)) {
    act("<k>You're hit by the falling rocks!  Ouch!<1>", FALSE, ch, NULL, NULL, TO_CHAR);
  } else {
    act("<k>You manage to dodge most of the falling rocks!  Whew.<1>", FALSE, ch, NULL, NULL, TO_CHAR);
    dam -= 20;
  }
  
  TRoom *rp2 = NULL;



  if(ch->in_room == 24643) {

    rp2 = real_roomp(24674);

  }
  if(ch->in_room == 24674) {

    rp2 = real_roomp(24643);

  }
  --(*ch);
  *rp2 += *ch;

  if (ch->reconcileDamage(ch, dam, TYPE_CRUSH) == -1) {
    delete ch;
    ch = NULL;
  }


  return TRUE;
}


extern int healing_room(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp);
extern int emergency_room(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp);
extern int SecretDoors(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp);

/* assign special procedures to rooms */
void assign_rooms(void)
{
  struct room_special_proc_entry specials[] =
  {
    {416, healing_room},
    {418, emergency_room},
#if 0
    {419, operating_room},
    {476, oft_frequented_room},
#endif
    {450, SecretDoors},     // Adders Coffeehouse
    {451, SecretDoors},     // Adders Coffeehouse
    {553, oft_frequented_room},
    {556, oft_frequented_room},
    {600, grimhavenDump},
    {757, oft_frequented_room},
    {758, oft_frequented_room},
    {761, oft_frequented_room},
    {763, oft_frequented_room},
    {774, SecretDoors},     // twist-lid for 2nd Floor Cathedral trapdoor
    {1295, bankRoom},
    {1303, dayGateRoom},
    {1353, healing_room},
    //    {1385, dump},
    {2104, noiseBoom},
    {2351, bankRoom},
    {2375, theKnot},
    {2376, theKnot},
    {2377, theKnot},
    {2378, theKnot},
    {2379, theKnot},
    {2380, theKnot},
    {2381, theKnot},
    {2382, theKnot},
    {2383, theKnot},
    {2384, theKnot},
    {2385, theKnot},
    {2386, theKnot},
    {2388, theKnot},
    {2389, theKnot},
    {2390, theKnot},
    {2391, theKnot},
    {2392, theKnot},
    {2393, theKnot},
    {2394, theKnot},
    {2395, theKnot},
    {2396, theKnot},
    {2397, theKnot},
    {2398, theKnot},
    {2399, theKnot},
    //    {3700, dump},
    {3736, healing_room},
    {3755, bankRoom},
    {4189, boulderRoom},
    {4284, boulderRoom},
    {5700, dayGateRoom},
    {5895, moonGateRoom},
    {6156, SecretDoors},
    {6158, SecretDoors},
    {7005, SecretDoors},
    {7015, SecretDoors},
    {7016, SecretDoors},
    {7023, SecretDoors},
    {7228, SecretPortalDoors},
    {7266, SecretPortalDoors},
    {7268, SecretPortalDoors},
    {8756, bankRoom},
    {9050, SecretDoors},      // GH Cathedral Expansion secret doors
    {9064, SecretDoors},
    {9390, SecretDoors},      // badlands brush
    {9391, SecretDoors},
    {9581, SecretDoors},      // bandit camp doors
    {9582, SecretDoors},
    {9625, SecretDoors},
    {9626, SecretDoors},
    {10020, monkQuestProcLand},
    {10111, SecretDoors},     // Batopr-elf doors
    {10144, SecretDoors},
    {10721, SecretDoors},     // Mithros's castle doors
    {10722, SecretDoors},
    {10727, SecretDoors},
    {10730, SecretDoors},
    {10752, SecretDoors},
    {10753, SecretDoors},
    {10759, SecretDoors},
    {10760, SecretDoors},
    {10764, SecretDoors},
    {10772, SecretDoors},
    {10782, SecretDoors},
    {10785, SecretDoors},
    {10790, SecretDoors},
    {10791, SecretDoors},
    {10814, SecretDoors},
    {10815, SecretDoors},
    {10820, SecretDoors},
    {10821, SecretDoors},
    {11089, monkQuestProcFall},
    {11347, slide},
    {11349, slide},
    {11351, slide},
//    {11353, slide},   air room, not needed
    {11423, slide},
    {12781, Whirlpool},
    {13111, SecretDoors},
    {13480, belimusThroat},
    {13481, belimusThroat},
    {13482, belimusThroat},
    {13483, belimusStomach},
    {13484, belimusStomach},
    {13485, belimusStomach},
    {13486, belimusStomach},
    {13487, belimusStomach},
    {13488, belimusStomach},
    {13489, belimusStomach},
    {13490, belimusStomach},
    {13491, belimusStomach},
    {13493, belimusLungs},
    {13494, belimusLungs},
    {13495, belimusLungs},
    {13496, belimusLungs},
    {13497, belimusLungs},
    {13498, belimusLungs},
    {13499, belimusBlowHole},
    {13700, randomMobDistribution},
    {13738, duergarWater},
    {13739, duergarWater},
    {13740, duergarWater}, 
    {13741, duergarWater},
    {13742, duergarWater},
    {13743, duergarWater}, 
    {13744, duergarWater},
    {13745, duergarWater},
    {13746, duergarWater}, 
    {13747, duergarWater},
    {13748, duergarWater},
    {13749, duergarWater}, 
    {13750, duergarWater},
    {13751, duergarWater},
    {13752, duergarWater}, 
    {13753, duergarWater},
    {13754, duergarWater},
    {13755, duergarWater}, 
    {13756, duergarWater},
    {13757, duergarWater},
    {13758, duergarWater}, 
    {13759, duergarWater},
    {13760, duergarWater},
    {13761, duergarWater}, 
    {13762, duergarWater},
    {13763, duergarWater},
    {13764, duergarWater}, 
    {13765, duergarWater},
    {13766, duergarWater},
    {13767, duergarWater}, 
    {13768, duergarWater},
    {13769, duergarWater},
    {13770, duergarWater}, 
    {13771, duergarWater},
    {13772, duergarWater},
    {13773, duergarWater},
    {14152, SecretDoors},
    {14153, SecretDoors},
    {14296, SecretDoors},
    {14299, SecretDoors},
    {14302, SecretDoors},
    {14319, SecretDoors},
    {15257, SecretDoors},
    {15277, SecretPortalDoors},
    {15293, SecretDoors},
    {15984, SecretDoors},
    {15986, SecretDoors},
    {15992, SecretDoors},
    {15996, SecretDoors},
    {16173, SecretPortalDoors},
    {16206, healing_room},
    {16231, emergency_room},
    {16238, SecretDoors},
    {16249, SecretDoors},
    {17106, wierdCircle},
    {20541, SecretDoors},
    {20582, SecretDoors},
    {20593, slide},
    {20594, slide},
    {20597, slide},
    {23400, sleepTagRoom},
    {23401, sleepTagRoom},
    {23402, sleepTagRoom},
    {23403, sleepTagRoom},
    {23404, sleepTagRoom},
    {23405, sleepTagRoom},
    {23406, sleepTagRoom},
    {23407, sleepTagRoom},
    {23408, sleepTagRoom},
    {23409, sleepTagRoom},
    {23410, sleepTagRoom},
    {23411, sleepTagRoom},
    {23412, sleepTagRoom},
    {23413, sleepTagRoom},
    {23414, sleepTagRoom},
    {23415, sleepTagRoom},
    {23416, sleepTagRoom},
    {23417, sleepTagRoom},
    {23418, sleepTagRoom},
    {23419, sleepTagRoom},
    {23420, sleepTagRoom},
    {23421, sleepTagRoom},
    {23422, sleepTagRoom},
    {23423, sleepTagRoom},
    {23424, sleepTagRoom},
    {23425, sleepTagRoom},
    {23426, sleepTagRoom},
    {23427, sleepTagRoom},
    {23428, sleepTagRoom},
    {23429, sleepTagRoom},
    {23430, sleepTagRoom},
    {23431, sleepTagRoom},
    {23432, sleepTagRoom},
    {23433, sleepTagRoom},
    {23434, sleepTagRoom},
    {23435, sleepTagRoom},
    {23436, sleepTagRoom},
    {23437, sleepTagRoom},
    {23438, sleepTagRoom},
    {23439, sleepTagRoom},
    {23440, sleepTagRoom},
    {23441, sleepTagRoom},
    {23442, sleepTagRoom},
    {23443, sleepTagRoom},
    {23444, sleepTagRoom},
    {23445, sleepTagRoom},
    {23446, sleepTagRoom},
    {23447, sleepTagRoom},
    {23448, sleepTagRoom},
    {23449, sleepTagRoom},
    {23450, sleepTagRoom},
    {23451, sleepTagRoom},
    {23452, sleepTagRoom},
    {23453, sleepTagRoom},
    {23454, sleepTagRoom},
    {23455, sleepTagRoom},
    {23456, sleepTagRoom},
    {23457, sleepTagRoom},
    {23458, sleepTagRoom},
    {23459, sleepTagRoom},
    {23460, sleepTagRoom},
    {23461, sleepTagRoom},
    {23462, sleepTagRoom},
    {23463, sleepTagRoom},
    {23464, sleepTagRoom},
    {23465, sleepTagRoom},
    {23466, sleepTagRoom},
    {23467, sleepTagRoom},
    {23468, sleepTagRoom},
    {23469, sleepTagRoom},
    {23470, sleepTagRoom},
    {23471, sleepTagRoom},
    {23472, sleepTagRoom},
    {23473, sleepTagRoom},
    {23474, sleepTagRoom},
    {23475, sleepTagRoom},
    {23476, sleepTagRoom},
    {23477, sleepTagRoom},
    {23478, sleepTagRoom},
    {23479, sleepTagRoom},
    {23480, sleepTagRoom},
    {23481, sleepTagRoom},
    {23482, sleepTagRoom},
    {23483, sleepTagRoom},
    {23484, sleepTagRoom},
    {23485, sleepTagRoom},
    {23486, sleepTagRoom},
    {23487, sleepTagRoom},
    {23488, sleepTagRoom},
    {23489, sleepTagRoom},
    {23490, sleepTagRoom},
    {23491, sleepTagRoom},
    {23492, sleepTagRoom},
    {23493, sleepTagRoom},
    {23494, sleepTagRoom},
    {23495, sleepTagRoom},
    {23496, sleepTagRoom},
    {23497, sleepTagRoom},
    {23498, sleepTagRoom},
    {23499, sleepTagRoom},
    {23500, sleepTagRoom},
    {23501, sleepTagRoom},
    {23502, sleepTagRoom},
    {23503, sleepTagRoom},
    {23504, sleepTagRoom},
    {23505, sleepTagRoom},
    {23506, sleepTagRoom},
    {23507, sleepTagRoom},
    {23508, sleepTagRoom},
    {23509, sleepTagRoom},
    {23510, sleepTagRoom},
    {23511, sleepTagRoom},
    {23512, sleepTagRoom},
    {23513, sleepTagRoom},
    {23514, sleepTagRoom},
    {23515, sleepTagRoom},
    {23516, sleepTagRoom},
    {23517, sleepTagRoom},
    {23518, sleepTagRoom},
    {23519, sleepTagRoom},
    {23520, sleepTagRoom},
    {23521, sleepTagRoom},
    {23522, sleepTagRoom},
    {23523, sleepTagRoom},
    {23524, sleepTagRoom},
    {23525, sleepTagRoom},
    {23526, sleepTagRoom},
    {23527, sleepTagRoom},
    {23528, sleepTagRoom},
    {23529, sleepTagRoom},
    {23530, sleepTagRoom},
    {23531, sleepTagRoom},
    {23532, sleepTagRoom},
    {23533, sleepTagRoom},
    {23534, sleepTagRoom},
    {23535, sleepTagRoom},
    {23536, sleepTagRoom},
    {23537, sleepTagRoom},
    {23538, sleepTagRoom},
    {23539, sleepTagRoom},
    {23540, sleepTagRoom},
    {23541, sleepTagRoom},
    {23542, sleepTagRoom},
    {23543, sleepTagRoom},
    {23544, sleepTagRoom},
    {23545, sleepTagRoom},
    {23546, sleepTagRoom},
    {23547, sleepTagRoom},
    {23548, sleepTagRoom},
    {23549, sleepTagRoom},
    {23550, sleepTagRoom},
    {23551, sleepTagRoom},
    {23552, sleepTagRoom},
    {23553, sleepTagRoom},
    {23554, sleepTagRoom},
    {23555, sleepTagRoom},
    {23556, sleepTagRoom},
    {23557, sleepTagRoom},
    {23558, sleepTagRoom},
    {23559, sleepTagRoom},
    {23560, sleepTagRoom},
    {23561, sleepTagRoom},
    {23562, sleepTagRoom},
    {23563, sleepTagRoom},
    {23564, sleepTagRoom},
    {23565, sleepTagRoom},
    {23566, sleepTagRoom},
    {23567, sleepTagRoom},
    {23568, sleepTagRoom},
    {23569, sleepTagRoom},
    {23570, sleepTagRoom},
    {23571, sleepTagRoom},
    {23572, sleepTagRoom},
    {23573, sleepTagRoom},
    {23574, sleepTagRoom},
    {23575, sleepTagRoom},
    {23576, sleepTagRoom},
    {23577, sleepTagRoom},
#if 0
    {23578, sleepTagRoom},
    {23579, sleepTagRoom},
    {23580, sleepTagRoom},
    {23581, sleepTagRoom},
    {23582, sleepTagRoom},
    {23583, sleepTagRoom},
    {23584, sleepTagRoom},
    {23585, sleepTagRoom},
    {23586, sleepTagRoom},
    {23587, sleepTagRoom},
    {23588, sleepTagRoom},
    {23589, sleepTagRoom},
    {23590, sleepTagRoom},
    {23591, sleepTagRoom},
    {23592, sleepTagRoom},
    {23593, sleepTagRoom},
    {23594, sleepTagRoom},
    {23595, sleepTagRoom},
    {23596, sleepTagRoom},
    {23597, sleepTagRoom},
    {23598, sleepTagRoom},
#endif
    {23599, sleepTagControl},
    {24643, collapsingTunnel},
    {24674, collapsingTunnel},
    {25487, oft_frequented_room},
    {27103, SecretDoors},
    {27104, SecretDoors},
    {27106, SecretDoors},
    {27107, SecretDoors},
    {27108, SecretDoors},
    {27111, SecretDoors},
    {27113, SecretDoors},
    {27116, SecretDoors},
    {27118, SecretDoors},
    {27122, SecretDoors},
    {27124, SecretDoors},
    {27128, SecretDoors},
    {27129, SecretDoors},
    {27134, SecretDoors},
    {27135, SecretDoors},
    {27136, SecretDoors},
    {27137, SecretDoors},
    {27138, SecretDoors},
    {27139, SecretDoors},
    {27140, SecretDoors},
    {27141, SecretDoors},
    {27142, SecretDoors},
    {27143, SecretDoors},
    {27144, SecretDoors},
    {27145, SecretDoors},
    {27146, SecretDoors},
    {27305, SecretDoors},
    {27306, SecretDoors},
    {27828, SecretDoors},
    {27890, SecretDoors},
    {28800, moonGateRoom},
    {31751, bankRoom},
    {31756, bankRoom},
    {31764, BankMainEntrance},
    {31774, BankVault},
    {31775, BankVault},
    {31780, BankVault},
    {31781, BankVault},
    {31786, BankVault},
    {31787, BankVault},
    {31905, prisonDump},
    {33679, SecretDoors},
    {33690, SecretDoors},
    {-1, NULL},
  };

  int i;
  TRoom *rp;

  for (i = 0; specials[i].vnum >= 0; i++) {
    rp = real_roomp(specials[i].vnum);
    if (rp) {
      rp->funct = specials[i].proc;
      roomspec_db.push_back(rp);
    } else
      vlogf(LOG_PROC, fmt("assign_rooms: unknown room (%d)") %  specials[i].vnum);
  }
}
