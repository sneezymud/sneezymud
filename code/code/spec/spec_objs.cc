
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "spec_objs.cc" - Special procedures for Objects 
//
/////////////////////////////////////////////////////////////////////////


//  cmd = CMD_OBJ_HITTING : a pre-hit spec, for denying, or overriding oneHit
//    returning TRUE prevents the hit from happening
//    ch parm is the victim, use obj->equippedBy for caster
//    if victim dies, return DELETE_VICT
//    if object gone, return DELETE_ITEM

//  cmd = CMD_OBJ_HIT : a post-hit spec, extra damage, etc.
//    cast arg to a wearSlotT to get part hit
//    ch parm is the victim
//    obj2 is also the wielder, but you have to cast it up/down to get it
//    obj->equippedBy is alternative to above method for getting wielder
//    if victim dies, return DELETE_VICT
//    if object gone, return DELETE_ITEM
//
//  cmd = CMD_OBJ_MISS : a post-hit spec, extra damage, etc.
//    ch parm is the victim
//    cast obj2 to a Being and it is the hitter : hopefully same me->equippedBy
//    if victim dies, return DELETE_THIS
//    if object gone, return DELETE_ITEM
//    if obj2 gone, return DELETE_VICT
//
//  cmd = CMD_OBJ_BEEN_HIT : spec for an object that has been hit
//    if victim (the hitter) dies, return DELETE_VICT
//    if object (the hitter's weapon) gone, return DELETE_ITEM
//    if the person being hit dies, return DELETE_THIS
// 
//  cmd == CMD_OBJ_MOVEMENT
//    called whenever anyone in my room moves out of room
//    arg can be cast to a dirTypeT
// 
//  cmd == CMD_GENERIC_PULSE
//    if obj gone, return DELETE_THIS
//    other parms are null
//    return TRUE if something happened
//
//  cmd == CMD_OBJ_STUCK_IN
//     if ch dies, return DELETE_VICT
//
//  cmd == CMD_OBJ_HAVING_SOMETHING_PUT_INTO, CMD_OBJ_PUT_INSIDE_SOMETHING,
//    if obj2 goes poof, return DELETE_ITEM
//
//  cmd == CMD_OBJ_GOTTEN
//    item has already been picked up, me = item gotten, obj2 = bag
//
//  cmd == CMD_OBJ_WEATHER_TIME
//    return DELETE_THIS if item2 goes bye, everyting else is NULL
//
//  cmd otherwise
//    if victim (1st parm) dies
//        leave it valid (do not delete) and return DELETE_VICT
//    if item1 goes poof, return DELETE_THIS
//  

#include <cmath>

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "low.h"
#include "monster.h"
#include "disease.h"
#include "database.h"
#include "shop.h"
#include "obj_base_corpse.h"
#include "obj_open_container.h"
#include "obj_base_clothing.h"
#include "person.h"
#include "obj_trap.h"
#include "obj_portal.h"
#include "obj_symbol.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_drinkcon.h"
#include "obj_potion.h"
#include "obj_staff.h"
#include "obj_wand.h"
#include "disc_fire.h"
#include "obj_note.h"
#include "pathfinder.h"
#include "liquids.h"
#include "obj_chest.h"
#include "materials.h"
#include "weather.h"

// CMD_OBJ_GOTTEN returns DELETE_THIS if this goes bye bye
// returns DELETE_VICT if t goes bye bye
// returns DELETE_ITEM if t2 goes bye bye
int TObj::checkSpec(TBeing *t, cmdTypeT cmd, const char *arg, TThing *t2)
{
  int rc;

  // we use static_cast here because we don't ALWAYS pass the proper kind of
  // parameter through the pointer fields.
  if (spec) {
    rc = (objSpecials[GET_OBJ_SPE_INDEX(spec)].proc) 
          (t, cmd, arg, this, static_cast<TObj *>(t2));
    return rc;
  }
  return FALSE;
}

const int GET_OBJ_SPE_INDEX(int d)
{
  return ((d > NUM_OBJ_SPECIALS) ? 0 : d);
}

void obj_act(const char *message, const TThing *ch, const TObj *o, const TBeing *ch2, const char *color)
{
  sstring buffer;

  if (!ch) {
    vlogf(LOG_PROC,"NULL ch in obj_act");
    return;
  }
  if (!o) {
    vlogf(LOG_PROC,"NULL obj in obj_act");
    return;
  }
  buffer = format("$n's $o %s") % message;
  act(buffer, TRUE, ch, o, ch2, TO_ROOM, color);
  buffer = format("Your $o %s") %message;
  act(buffer, TRUE, ch, o, ch2, TO_CHAR, color);
}

TBeing *genericWeaponProcCheck(TBeing *vict, cmdTypeT cmd, TObj *o, int chance)
{
  TBeing *ch;

  if (cmd != CMD_OBJ_HIT)
    return NULL;
  if (!o || !vict)
    return NULL;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return NULL;       // weapon not equipped (carried or on ground)
  if(::number(0,chance))
    return NULL;
  return ch;
}
















int rainbowBridge(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  int rc;

  if (!o || !vict)
    return FALSE;
  if ((cmd != CMD_UP) && (cmd != CMD_DOWN) && 
      (cmd != CMD_CLIMB) && (cmd != CMD_DESCEND)) 
    return FALSE;

  // this is a portal so just translate these commands into an "enter"
  TPortal *por = dynamic_cast<TPortal *>(o);
  if (!por) return FALSE;

  // objects with movement proc seem to get caught here
  if (!dynamic_cast<TBeing *>(vict))
    return FALSE;
  
  rc = vict->doEnter(NULL, por);
  if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS)) {
    return DELETE_VICT | DELETE_THIS;
  }
  if (IS_SET_DELETE(rc, DELETE_ITEM)) {
    return DELETE_THIS;
  }
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_VICT;
  }
  return rc;
}

int ladder(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  roomDirData *exitp;
  int going_to_fall = FALSE,
      nRc = TRUE;

  if (!o || !vict)
    return FALSE;
  if ((cmd != CMD_UP) && (cmd != CMD_DOWN) && 
      (cmd != CMD_CLIMB) && (cmd != CMD_DESCEND)) 
    return FALSE;

  // objects with movement proc can probably get caught here
  if (!dynamic_cast<TBeing *>(vict))
    return FALSE;
  
  if (!vict->canSee(o))
    return FALSE;

  if (!vict->hasHands()) {
    vict->sendTo(COLOR_OBJECTS, format("I'm afraid you need hands to climb %s.\n\r") % o->getName());
    return TRUE;
  }
  if (vict->bothArmsHurt()) {
    vict->sendTo(COLOR_OBJECTS, format("I'm afraid you need working arms to climb %s.\n\r") % o->getName());
    return TRUE;
  }
  if (vict->riding) {
    act("You can't ride $N on $p", FALSE, vict, o, vict->riding, TO_CHAR);
    return TRUE;
  }
  if (vict->rider) {
    return TRUE;
  }
  if (vict->bothLegsHurt() && ::number(0,1))
    going_to_fall = TRUE;
  if (vict->eitherArmHurt() && ::number(0,1))
    going_to_fall = TRUE;
  if (vict->eitherLegHurt() && !::number(0,2))
    going_to_fall = TRUE;

  if (going_to_fall && (vict->bSuccess(vict->getSkillValue(SKILL_CLIMB)/4, SKILL_CLIMB))) {
    vict->sendTo("Whoa!  You almost fell there but your climbing ability saved you.\n\r");
    going_to_fall = FALSE;
  }
  if ((cmd == CMD_UP) || (cmd == CMD_CLIMB)) {
    exitp = vict->exitDir(DIR_UP);
    if (exit_ok(exitp, NULL)) {
      if (going_to_fall) {
        act("You start to climb $p but your busted limb makes you fall.",
            TRUE,vict,o,0,TO_CHAR);
        act("$n's busted limb causes $m to fall while climbing $p.",
            TRUE,vict,o,0,TO_ROOM);
        if (vict->reconcileDamage(vict,number(3,5),DAMAGE_FALL) == -1) {
          return DELETE_VICT;
        }
        vict->setPosition(POSITION_SITTING);
        return TRUE;
      } else {
        act("You climb up $p.",TRUE,vict,o,0,TO_CHAR);
        act("$n climbs up $p.",TRUE,vict,o,0,TO_ROOM);

        if ((nRc = vict->doMove(DIR_UP)) == FALSE)
          return TRUE;
		
		act("$n climbs in from below.",TRUE,vict,o,0,TO_ROOM);
        return nRc;
      }
    }
    return FALSE;
  } else if ((cmd == CMD_DOWN) || (cmd == CMD_DESCEND)) {
    exitp = vict->exitDir(DIR_DOWN);
    if (exit_ok(exitp, NULL)) {
      if (going_to_fall) {
        act("You start to climb $p but your busted limb makes you fall.",
            TRUE,vict,o,0,TO_CHAR);
        act("$n's busted limb causes $m to fall while climbing $p.",
            TRUE,vict,o,0,TO_ROOM);
        if (vict->reconcileDamage(vict,number(3,5),DAMAGE_FALL) == -1)
          return DELETE_VICT;
        vict->setPosition(POSITION_SITTING);
        return TRUE;
      } else {
        act("You climb down $p.",TRUE,vict,o,0,TO_CHAR);
        act("$n climbs down $p.",TRUE,vict,o,0,TO_ROOM);
        if ((nRc = vict->doMove(DIR_DOWN)) == FALSE)
          return TRUE;
		act("$n climbs in from above.",TRUE,vict,o,0,TO_ROOM);
        return nRc;
      }
    }
    return FALSE;
  }
  return FALSE;
}

int weatherArmor(TBeing *, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  wearSlotT pos;

  if (cmd != CMD_OBJ_WEATHER_TIME)
    return FALSE;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  ch->unequip(pos = o->eq_pos);

  // do whatever changes here

  ch->equipChar(o, pos, SILENT_YES);

  return TRUE;
}

int TObj::foodItemUsed(TBeing *ch, const char *)
{
  vlogf(LOG_LOW, format("Undefined item (%s) with special proc: foodItem") %  getName());
  act("Oily black smoke pours from $p as something goes wrong.",
      TRUE, ch, this, 0, TO_CHAR);
  act("Oily black smoke pours from $p as something goes wrong.",
      TRUE, ch, this, 0, TO_ROOM);
  return FALSE;
}


int TWand::foodItemUsed(TBeing *ch, const char *arg)
{
  char buffer[256];
  TBeing *vict = NULL;

  one_argument(arg, buffer, cElements(buffer));
  if (!(vict = get_char_room_vis(ch, buffer))) {
    ch->sendTo("That person isn't here.\n\r");
    return FALSE;
  }
  if (ch == vict) {
    act("$n points $o at $mself.",
        TRUE, ch, this, 0, TO_ROOM);
    act("You point $o at yourself.",
        TRUE, ch, this, 0, TO_CHAR);
  } else {
    act("You point $o at $N.",
        TRUE, ch, this, vict, TO_CHAR);
    act("$n points $o at you.",
        TRUE, ch, this, vict, TO_VICT);
    act("$n points $o at $N.",
        TRUE, ch, this, vict, TO_NOTVICT);
  }
  if (getCurCharges() <= 0) {
    act("Nothing seems to happen.", TRUE, ch, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", TRUE, ch, 0, 0, TO_ROOM);
    return FALSE;
  }
  addToCurCharges(-1);
  if (vict->getCond(FULL) > -1)
    vict->gainCondition(FULL, 25);
  if (vict->getCond(THIRST) > -1)
    vict->gainCondition(THIRST, 25);
  act("You feel completely satiated.", TRUE, vict, 0, 0, TO_CHAR);
  act("$n smacks $s lips and looks very satisfied.",
            TRUE, vict, 0, 0, TO_ROOM);
  return TRUE;
}

int TStaff::foodItemUsed(TBeing *ch, const char *)
{
  TBeing *vict = NULL;
  TThing *t=NULL;

  act("$n taps $o three times on the $g.",
            TRUE, ch, this, 0, TO_ROOM);
  act("You tap $o three times on the $g.",
            TRUE, ch, this, 0, TO_CHAR);
  if (getCurCharges() <= 0) {
    act("Nothing seems to happen.", TRUE, ch, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", TRUE, ch, 0, 0, TO_ROOM);
    return FALSE;
  }
  addToCurCharges(-1);

  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (t=*it);++it) {
    vict = dynamic_cast<TBeing *>(t);
    if (!vict)
      continue;
    if (!vict->inGroup(*ch))
      continue;
    if (vict->getCond(FULL) > -1)
      vict->gainCondition(FULL, 25);
    if (vict->getCond(THIRST) > -1)
      vict->gainCondition(THIRST, 25);
    act("You feel completely satiated.", TRUE, vict, 0, 0, TO_CHAR);
    act("$n smacks $s lips and looks very satisfied.",
                TRUE, vict, 0, 0, TO_ROOM);
  }
  return TRUE;
}

int foodItem(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  char buffer[256];

  if (cmd != CMD_USE)
    return FALSE;
  if (!arg || !*arg)
    return FALSE;
  arg = one_argument(arg, buffer, cElements(buffer));

  if (isname(buffer, o->getName())) {
    return o->foodItemUsed(ch, arg);
  }

  return FALSE;
}

int orbOfDestruction(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  int rc;
  char buffer[256];
  TBeing *vict = NULL;
  TThing *t;
  int d = 0;
  int resCode = 0;

  if (cmd != CMD_USE &&
      cmd != CMD_PUSH)
    return FALSE;

  if (!arg || !*arg)
    return FALSE;

  one_argument(arg, buffer, cElements(buffer));

  if (cmd == CMD_USE || (cmd == CMD_PUSH && is_abbrev(buffer, "button"))) {
    arg = one_argument(arg, buffer, cElements(buffer));
    if (isname(buffer, o->getName())) {
      if (ch->getStat(STAT_CURRENT, STAT_PER) < 90) {
        ch->sendTo("You can't figure out how to use it.\n\r");
        return TRUE;
      } else if (!o->equippedBy) {
        ch->sendTo("You must be holding it to use it.\n\r");
        return TRUE;
      } else {
        resCode = TRUE;

        act("You trigger $p, causing it to explode in a fiery blast of light!", 1, ch, o, NULL, TO_CHAR);
        act("$n's $o explodes in a fiery blast of light!", 1, ch, o, NULL, TO_ROOM);
        for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end();){
          t=*(it++);
          vict = dynamic_cast<TBeing *>(t);
          if (!vict)
            continue;
          if (vict != ch) {
            d = ch->getActualDamage(vict, NULL, dice(10, 100), SPELL_ATOMIZE);
            if (ch->willKill(vict, d, SPELL_ATOMIZE, TRUE)) {
              act("You are consumed in the explosion!", 1, vict, o, NULL, TO_CHAR);
              act("$n is consumed in the explosion!", 1, vict, o, NULL, TO_ROOM);
            } else if (d > 0) {
              act("You are fried in the explosion!", 1, vict, o, NULL, TO_CHAR);
              act("$n is fried in the explosion!", 1, vict, o, NULL, TO_ROOM);
            } else {
              act("You laugh at the puny explosion.", 1, vict, o, NULL, TO_CHAR);
              act("$n laughs at the explosion.", 1, vict, o, NULL, TO_ROOM);
            }
            if (ch->reconcileDamage(vict, d, SPELL_ATOMIZE) == -1) {
              delete vict;
              vict = NULL;
            }
          }
        }
        ch->setHit(1);
        rc = ch->genericTeleport(SILENT_NO);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          ADD_DELETE(resCode, DELETE_VICT);

        // destroy item
        ADD_DELETE(resCode, DELETE_THIS);
      }
      return resCode;
    }
  }
  return FALSE;
}

int orbOfTeleportation(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  int rc;
  char buffer[256];
  TBeing *vict = NULL;
  TThing *t;
  int resCode = 0;

  if (!arg || !*arg)
    return FALSE;

  one_argument(arg, buffer, cElements(buffer));

  if (cmd == CMD_USE || (cmd == CMD_PUSH && is_abbrev(buffer, "button"))) {
    arg = one_argument(arg, buffer, cElements(buffer));
    if (isname(buffer, o->getName())) {
      if (ch->getStat(STAT_CURRENT, STAT_PER) < 90) {
        ch->sendTo("You can't figure out how to use it.\n\r");
        return TRUE;
      } else if (!o->equippedBy) {
        ch->sendTo("You must be holding it to use it.\n\r");
        return TRUE;
      } else {
        resCode = TRUE;

        act("You trigger $p, causing it to make a loud, throbbing noise!", 
             1, ch, o, NULL, TO_CHAR);
        act("$n's $o makes a loud, throbbing noise!", 
             1, ch, o, NULL, TO_ROOM);
        for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end();){
          t=*(it++);
          vict = dynamic_cast<TBeing *>(t);
          if (!vict)
            continue;
          if (vict != ch) {
            rc = vict->genericTeleport(SILENT_NO, TRUE);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete vict;
              vict = NULL;
            }
          }
        }
        act("Your $o seems to have blinked out of existence too...",
             1, ch, o, NULL, TO_CHAR);
        act("$n's $o seems to have blinked out of existence too...",
             1, ch, o, NULL, TO_ROOM);
        // destroy item
        ADD_DELETE(resCode, DELETE_THIS);
      }
      return resCode;
    }
  }
  return FALSE;
}


void invert(const char *arg1, char *arg2)
{
  register int i = 0;
  register int len = strlen(arg1) - 1;

  while (i <= len) {
    *(arg2 + i) = *(arg1 + (len - i));
    i++;
  }
  *(arg2 + i) = '\0';
}


int jive_box(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *, TObj **)
{
  char buf[255], buf2[255], tmp[255];

  switch (cmd) {
    case CMD_SAY:
    case CMD_SAY2:
      invert(arg, buf);
      ch->doSay(buf);
      return TRUE;
      break;
    case CMD_TELL:
      half_chop(arg, tmp, buf);
      invert(buf, buf2);
      ch->doTell(tmp, buf);
      return TRUE;
      break;
    case CMD_SHOUT:
      invert(arg, buf);
      ch->doShout(buf);
      return TRUE;
      break;
    default:
      return FALSE;
  }
}

int statue_of_feeding(TBeing *ch, cmdTypeT cmd, const char *argum, TObj *me, TObj *)
{
  char arg[160];

  *arg = '\0';

  if (cmd == CMD_WORSHIP) {
    act("$n utters a blessing unto $s deities.", TRUE, ch, me, NULL, TO_ROOM);
    act("You utter a blessing unto your deities.", TRUE, ch, me, NULL, TO_CHAR);

#if FACTIONS_IN_USE
    followData *k;

    for (k = ch->followers; k; k = k->next) {
      if (k->follower && !k->follower->isPc()) {
        if (k->follower->mobVnum() == Mob::FACTION_FAERY) {
          ch->sendTo("Your deity ignores you.\n\r");
          return TRUE;
        }
      }
    }

    TBeing *mob = read_mobile(Mob::FACTION_FAERY, VIRTUAL);
    if (!mob) {
      ch->sendTo("Problem!  Tell a god.\n\r");
      return FALSE;
    }
    *ch->roomp += *mob;
    ch->addFollower(mob);
    mob->setExp(0);
#endif
    return TRUE;
  }

  if (cmd != CMD_PRAY)
    return FALSE;

  one_argument(argum, arg, cElements(arg));
  
  if (*arg && !isname(arg, me->getName()))
    return FALSE;

  act("$n begins to pray quietly before $p.", TRUE, ch, me, NULL, TO_ROOM);
  act("You begin to pray quietly before $p.", TRUE, ch, me, NULL, TO_CHAR);

  if (ch->isImmortal()) {
    act("$p turns in $n's direction and laughs uproariously.",
         TRUE, ch, me, NULL, TO_ROOM);
    act("$p spits on you and calls you a twit.",
         TRUE, ch, me, NULL, TO_CHAR);
  } else if (ch->GetMaxLevel() > MAX_NEWBIE_LEVEL)
    ch->sendTo("A statue lacks the power to help you any longer.\n\r");
  else if (ch->getCond(FULL) == 24)
    ch->sendTo("Nothing happens.\n\r");
  else {
    act("Suddenly, a ray of light shoots from $p towards $n.", TRUE, ch, me, NULL, TO_ROOM);
    act("A ray of light shoots towards you from $p.", TRUE, ch, me, NULL, TO_CHAR);
    ch->sendTo("You feel much less hungry.\n\r");
    if (ch->getCond(FULL) >= 0)
      ch->setCond(FULL, 24);
  }
  return TRUE;
}

int magicGills(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  int rc;
  TBeing *tmp;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (!(tmp = dynamic_cast<TBeing *>(me->equippedBy)))
    return FALSE;

  // if they areholding it, don't be silly
  if (me->eq_pos != WEAR_NECK)
    return FALSE;

  if (!tmp->roomp)
    return FALSE;

  if (tmp->roomp->isWaterSector() || tmp->roomp->isUnderwaterSector())
    return FALSE;

  if (tmp->isImmune(IMMUNE_SUFFOCATION, WEAR_BODY))
    return FALSE;

  if (!tmp->isPc())
    return FALSE;

  act("$p constricts about your throat!", 0, tmp, me, 0, TO_CHAR);
  act("$p constricts about $n's throat!", 0, tmp, me, 0, TO_ROOM);
  rc = tmp->applyDamage(tmp, ::number(1,3), DAMAGE_SUFFOCATION);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete tmp;
    tmp = NULL;
  }
  return TRUE;
}

int JewelJudgment(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  TBeing *tmp;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;
  if (!(tmp = dynamic_cast<TBeing *>(me->equippedBy)))
    return FALSE;
  if (number(0,2)) {
    return FALSE;
  } else {
    obj_act("pulses with a warm glow.",tmp,me,tmp, ANSI_ORANGE);
    act("$n looks drained as energy seeps from $m into $p.",TRUE,tmp,me, 0, TO_ROOM, ANSI_ORANGE);
    act("You grunt softly as energy seeps from your body into $p.",TRUE,tmp,me,0,TO_CHAR, ANSI_ORANGE);
    if (tmp->reconcileDamage(dynamic_cast<TBeing *>(me->equippedBy),number(3,8),DAMAGE_DRAIN) == -1) {
      delete tmp;
      tmp = NULL;
    }
      
    return TRUE;
  }
}




int bowl_of_blood(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  char buf[MAX_INPUT_LENGTH];

  if (cmd == CMD_DRINK) {        
    strcpy(buf, arg);
    if (!isname(buf, me->getName()))
      return FALSE;

    if (ch->fight()) {
      ch->sendTo("You are too busy fending off your foes!\n\r");
      return TRUE;
    }
    if (ch->hasDisease(DISEASE_FOODPOISON)) {
      ch->sendTo("Uggh, your stomach is queasy and the thought of doing that is unappetizing.\n\r");
      ch->sendTo("You decide to skip this drink until you feel better.\n\r");
      return TRUE;
    }
    if (ch->getCond(THIRST) > 20 ||
        ch->getCond(THIRST) < 0) {
      act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
      return TRUE;
    }

    act("You drink from $p.", FALSE, ch, me, 0, TO_CHAR);
    act("$n drinks from $p.", FALSE, ch, me, 0, TO_ROOM);
    ch->sendTo("It tastes as horrible as it looks!\n\r");

    int level = 5;
    if (ch->isImmune(IMMUNE_DISEASE, WEAR_BODY)) {
      act("$n shakes off the effects as if immune.",
          FALSE, ch, 0, 0, TO_ROOM);
      act("You shake off the effects of that disease-spewing $o.",
          FALSE, ch, me, 0, TO_CHAR);
    } else
      genericDisease(NULL, ch, level);
    genericCurse(ch, ch, level, SPELL_CURSE);

    return TRUE;
  }
  return FALSE;
}

void explode(TObj *obj, int room, int dam)
{
  TRoom *rm;
  TThing *t;
  int rc;
  TBeing *v = NULL;

  if (!(rm = real_roomp(room))) {
    vlogf(LOG_PROC, "Explosion in room : Room::NOWHERE. (explode() spec_objs.c)");
    return;
  }

  for(StuffIter it=rm->stuff.begin();it!=rm->stuff.end();){
    t=*(it++);
    v = dynamic_cast<TBeing *>(t);
    if (!v)
      continue;
    rc = v->objDamage(DAMAGE_TRAP_TNT, dam, obj);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      delete v;
      v = NULL;
    }
  }
  delete obj;
  obj = NULL;
}

int vending_machine(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *ob2)
{
  TObj *dew;
  int result, token;

  if (cmd == CMD_OBJ_HAVING_SOMETHING_PUT_INTO) {
    switch (obj_index[o->getItemIndex()].virt) {
      case 9998:
        result = 9996;
        token = 9995;
        break;
      case 9999:
        result = 9997;
        token = 9995;
        break;
      default:
        vlogf(LOG_PROC, "Unknown vending machine.  Yikes");
        return TRUE;
    }
    if (obj_index[ob2->getItemIndex()].virt == token) {
      act("You insert $p into $P.", TRUE, ch, ob2, o, TO_CHAR);
      act("$n inserts $p into $P.", TRUE, ch, ob2, o, TO_ROOM);
      act("$p begins to beep and shake.", TRUE, ch, o, NULL, TO_CHAR);
      act("$p begins to beep and shake.", TRUE, ch, o, NULL, TO_ROOM);
      if (!(dew = read_object(result, VIRTUAL))) {
        vlogf(LOG_PROC, "Damn vending machine couldn't read a Dew.  Stargazer!");
        return TRUE;
      }
      act("$p appears in the can receptical.", TRUE, ch, dew, NULL, TO_CHAR);
      act("$p appears in the can receptical.", TRUE, ch, dew, NULL, TO_ROOM);
      *o += *dew;
      return DELETE_ITEM;    // delete ob2
    } else
      ch->sendTo("It doesn't seem to fit.\n\r");

    return TRUE;
  }
  return FALSE;
}

int vending_machine2(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *ob2)
{
  class vendingmachine_struct {
  public:
    bool isOn;

    vendingmachine_struct() :
      isOn(false)
    {
    }
    ~vendingmachine_struct()
    {
    }
  };

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<vendingmachine_struct *>(o->act_ptr);
    o->act_ptr = NULL;
    return FALSE;
  } else if (cmd == CMD_GENERIC_CREATED) {
    o->act_ptr = new vendingmachine_struct();
    return FALSE;
  }
  TObj *drinkobj;

  vendingmachine_struct *job;
  int result, token = 9995;
  char arg1[30], arg2[30], arg3[30], drink[30];

  if (!ch)
    return FALSE;
  if (!(job = static_cast<vendingmachine_struct *>(o->act_ptr))) {
    vlogf(LOG_PROC, "Vending machine lost its memory. DASH!!");
    return FALSE;
  }

  if (cmd == CMD_OBJ_HAVING_SOMETHING_PUT_INTO) {
    if (obj_index[ob2->getItemIndex()].virt == token) {
      act("You insert $p into $P.", TRUE, ch, ob2, o, TO_CHAR);
      act("$n inserts $p into $P.", TRUE, ch, ob2, o, TO_ROOM);
      if (job->isOn) {
	act("$P beeps once, then spits $p out into the coin receptical.", TRUE, ch, ob2, o, TO_CHAR);
	act("$P beeps once, then spits $p out into the coin receptical.", TRUE, ch, ob2, o, TO_ROOM);
      } else {
	act("$P beeps loudly, and the <R>correct change<1> button lights up.", TRUE, ch, ob2, o, TO_CHAR);
	act("$P beeps loudly, and the <R>correct change<1> button lights up.", TRUE, ch, ob2, o, TO_ROOM);
	job->isOn = TRUE;
        return DELETE_ITEM;
      }
      return TRUE;
    } else {
      ch->sendTo("It doesn't seem to fit.\n\r");
      return TRUE;
    }
  } else if ((cmd == CMD_PUSH || cmd == CMD_PRESS)) {
    arg = one_argument(arg, arg1, cElements(arg1));
    arg = one_argument(arg, arg2, cElements(arg2));
    arg = one_argument(arg, arg3, cElements(arg3));
    if ((is_abbrev(arg1, "button") || is_abbrev(arg1, "machine") || is_abbrev(arg1, "vending")) &&
	(!is_abbrev(arg2, "button") || !is_abbrev(arg2, "machine") || !is_abbrev(arg2, "vending")) && arg2)
      strncpy(drink, arg2, sizeof(drink));
    else if ((is_abbrev(arg1, "button") || is_abbrev(arg1, "machine") || is_abbrev(arg1, "vending")) &&
	     (is_abbrev(arg2, "button") || is_abbrev(arg2, "machine") || is_abbrev(arg2, "vending")) && arg3)
      strncpy(drink, arg3, sizeof(drink));
    else if (arg1)
      strncpy(drink, arg1, sizeof(drink));
    else
      return FALSE;
    
    
    if (is_abbrev(drink, "coke")) {
      result = 9994;
    } else if (is_abbrev(drink, "pepsi")) {
      result = 9993;
    } else if (is_abbrev(drink, "dew")) {
      result = 9996;
    } else if (is_abbrev(drink, "7up") || is_abbrev(drink, "Nup")) {
      result = 9997;
    } else if (is_abbrev(drink, "water")) {
      result = 9990;
    } else if (is_abbrev(drink, "juice") || is_abbrev(drink, "grapefruit")) {
      result = 9991;
    } else if (is_abbrev(drink, "jack") || is_abbrev(drink, "daniels")) {
      result = 9992;
    } else {
      ch->sendTo("The vending machine does not appear to have that button.\n\r");
      ch->sendTo("Bug Dash if you want him to stock the machine with something.\n\r");
      return TRUE;
    }
    act("$P beeps once as you select your drink.", TRUE, ch, ob2, o, TO_CHAR);
    act("$P beeps once as $n selects $s drink.", TRUE, ch, ob2, o, TO_ROOM);
    if (!job->isOn) {
      act("$P's <R>insert correct change<1> light blinks twice.", TRUE, ch, ob2, o, TO_CHAR);
      act("$P's <R>insert correct change<1> light blinks twice.", TRUE, ch, ob2, o, TO_ROOM);
      return TRUE;
    }
    else if (!(drinkobj = read_object(result, VIRTUAL))) {
      vlogf(LOG_PROC, format("Damn vending machine couldn't read drink, obj %d.  DASH!!!") %  result);
      return TRUE;
    }
    else {
      act("With a loud *clunk* $p appears in the can receptical.", TRUE, ch, drinkobj, NULL, TO_CHAR);
      act("With a loud *clunk* $p appears in the can receptical.", TRUE, ch, drinkobj, NULL, TO_ROOM);
      *o += *drinkobj;
      job->isOn = FALSE;
      return DELETE_ITEM;    // delete ob2
    }
    
  }
  return FALSE;
}

int dagger_of_death(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  int rc;

  if (cmd == CMD_OBJ_STUCK_IN) {
    if (o->eq_stuck == WEAR_HEAD) {
      vlogf(LOG_PROC, format("%s killed by ITEM:dagger-of-death at %s (%d)") % 
            ch->getName() % ch->roomp->getName() % ch->inRoom());

      rc = ch->die(DAMAGE_NORMAL);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
      return TRUE;
    }
  }
  return FALSE;
}


int dispenser(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TObj *note, *quill;
  char arg1[128], arg2[128];

  if ((cmd != CMD_GET) && (cmd != CMD_TAKE))
    return FALSE;

  half_chop(arg, arg1, arg2);

  if (is_abbrev(arg1, "note")) {
    if (isname(arg2, o->getName())) {
      act("You get a note from $p.", FALSE, ch, o, 0, TO_CHAR);
      act("$n gets a note from $p.", FALSE, ch, o, 0, TO_ROOM);
      if (!(note = read_object(Obj::GENERIC_NOTE, VIRTUAL))) {
        vlogf(LOG_PROC, "Bad note dispenser! NO note can be loaded!");
        return FALSE;
      }
      *ch += *note;
      return TRUE;
    }
  } else if (is_abbrev(arg1, "quill")) {
    if (isname(arg2, o->getName())) {
      act("You get a quill from $p.", FALSE, ch, o, 0, TO_CHAR);
      act("$n gets a quill from $p.", FALSE, ch, o, 0, TO_ROOM);
      if (!(quill = read_object(Obj::GENERIC_PEN, VIRTUAL))) {
        vlogf(LOG_PROC, "Bad quill dispenser! NO quill can be loaded!");
        return FALSE;
      }
      *ch += *quill;
      return TRUE;
    }
  }
  return FALSE;
}


int pager(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *ob2)
{
  class pager_struct {
    public:
      bool isOn;

      pager_struct() : 
        isOn(false)
      {
      }
      ~pager_struct()
      {
      }
  };

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<pager_struct *>(o->act_ptr);
    o->act_ptr = NULL;
    return FALSE;
  } else if (cmd == CMD_GENERIC_CREATED) {
    o->act_ptr = new pager_struct();
    return FALSE;
  }

  TBeing *t;
  char capbuf[80];
  pager_struct *job;
  

  if (!ch)
    return FALSE;
  if (!(job = static_cast<pager_struct *>(o->act_ptr))) {
    vlogf(LOG_PROC, "Pager lost its memory.");
    return FALSE;
  }

  strcpy(capbuf, ch->getName());

  if (cmd == CMD_USE || cmd == CMD_OPERATE) {
    if (o->equippedBy != ch)
      ch->sendTo("You must have it equipped to use it!\n\r");
    else if (job->isOn) {
      act("$n discretely turns off $s $o.", TRUE, ch, o, 0, TO_ROOM);
      ch->sendTo(format("You turn off your %s, trying to be very discrete about it.\n\r") % fname(o->getName()));
      job->isOn = FALSE;
    } else {
      act("$n turns on $s $o, causing it to beep obnoxiously....", FALSE, ch, o, 0, TO_ROOM);
      ch->sendTo(format("You turn on your %s, producing a series of annoying beeps....\n\r") % fname(o->getName()));
      job->isOn = TRUE;
    }
    return TRUE;
  } else if ((cmd == CMD_OBJ_TOLD_TO_PLAYER) && job->isOn) {
    // who got told to = ch, who told = ob2
    // user of pager = t

    // cast down and then up since ob2 is really a TBeing
    TThing *ttt = ob2;
    TBeing *tbob2 = dynamic_cast<TBeing *>(ttt);
    t = dynamic_cast<TBeing *>((o->parent) ? o->parent : o->equippedBy);
    strcpy(capbuf, tbob2->getName());

    if (t->hasColor())
      t->sendTo(format("%s%s%s tells you %s\"%s\"%s % triggering your pager....\n\r") %             t->purple() % sstring(capbuf).cap() % t->norm() % t->cyan() % arg % t->norm());
    else if (t->vt100())
      t->sendTo(format("%s%s%s tells you \"%s\" % triggering your pager....\n\r") %
             t->bold() % sstring(capbuf).cap() % t->norm() % arg);
    else
      t->sendTo(format("%s tells you \"%s\" % triggering your pager....\n\r") %
              sstring(capbuf).cap() % arg);

    tbob2->sendTo(COLOR_MOBS, format("You tell %s \"%s\".\n\r") % t->getName() % arg);

    strcpy(capbuf, t->getName());
    act("$n looks startled as $s $o begins to beep!",
         FALSE, t, o, NULL, TO_ROOM);
    return TRUE;
  }
  return FALSE;
}

// ch = told to
// ob2 = teller
int ear_muffs(TBeing *, cmdTypeT cmd, const char *, TObj *o, TObj *ob2)
{
  if ((cmd == CMD_OBJ_TOLD_TO_PLAYER) && (o->equippedBy)) {
    // cast down and then up since ob2 is really a TBeing
    TThing *ttt = ob2;
    TBeing *tb = dynamic_cast<TBeing *>(ttt);
    if (!tb->isImmortal()) {
      tb->sendTo("You fail.  Perhaps they are busy?\n\r");
      return TRUE;
    }
  }
  return FALSE;
}

#if 0
int lightning_hammer(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  char keyword[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  TBeing *victim;
  affectedData af;
  const char KEYWORD[] =  "jank";

  if (cmd != CMD_SAY)
    return FALSE;

  half_chop(arg, keyword, name);

  if (!strcmp(keyword, KEYWORD)) {
    if (ch->affectedBySpell(SPELL_LIGHTNING_BOLT)) {
      ch->sendTo("You can only use the hammer's powers once a week!!\n\r");
      return TRUE;
    }
    if (!(victim = get_char_room_vis(ch, name))) {
      if (ch->fight()) {
        victim = ch->fight();
      } else {
        ch->sendTo("Whom do you want to use the hammers power on?\n\r");
        return TRUE;
      }
    }
#if 0
    ((*spell_info[SPELL_LIGHTNING_BOLT].spell_pointer) (6, ch, "", SPELL_TYPE_WAND, victim, NULL, o));
#endif
    if (!ch->isImmortal()) {
      af.type = SPELL_LIGHTNING_BOLT;
      af.duration = 168 * UPDATES_PER_MUDHOUR;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = 0;
      ch->affectTo(&af);
    }
  }
  return FALSE;
}
#endif


int crystal_ball(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  int target;
  char buf[256], buf1[256], buf2[256];
  const char *c;
  TBeing *victim;

  if ((cmd != CMD_SHOW) && (cmd != CMD_WHERE) && (cmd != CMD_SAY) &&
      (cmd != CMD_SAY2) && (cmd != CMD_OBJ_GOTTEN))
    return FALSE;

  switch (cmd) {
    case CMD_OBJ_GOTTEN:
      obj_act("says 'Say \"show me <person>\" and I will show them to you.'", 
                  ch, me, NULL, ANSI_GREEN);
      return TRUE;
    case CMD_SAY:
    case CMD_SAY2:
      c = arg;
      c = one_argument(c, buf2, cElements(buf2));
      if (is_abbrev(buf2, "show")) {
        c = one_argument(c, buf2, cElements(buf2));
        if (is_abbrev(buf2, "me")) {
          c = one_argument(c, buf2, cElements(buf2));
        } else {
          ch->doSay(arg);
          obj_act("says '$n, you must speak the words correctly!'", 
                  ch, me, NULL, ANSI_GREEN);
          obj_act("says 'Say \"show me <person>\" and I will show them to you.'", 
                  ch, me, NULL, ANSI_GREEN);
          return TRUE;
        }
      } else if (is_abbrev(buf2, "where")) {
        c = one_argument(c, buf2, cElements(buf2));
      } else
        return FALSE;

      if (!(victim = get_char_vis_world(ch, buf2, NULL, EXACT_YES))) {
        ch->doSay(arg);
        obj_act("says 'Sorry $n, I have trouble finding that person.'", 
                ch, me, NULL, ANSI_GREEN);
        return TRUE;
      }
      break;
    case CMD_SHOW:
      half_chop(arg, buf, buf2);
      if (!is_abbrev(buf, "me")) {
        ch->doSay(arg);
        obj_act("says '$n, you must speak the words correctly!'", 
                ch, me, NULL, ANSI_GREEN);
        obj_act("says 'Say \"show me <person>\" and I will show them to you.'", 
                ch, me, NULL, ANSI_GREEN);
        return TRUE;
      }
      if (!(victim = get_char_vis_world(ch, buf2, NULL, EXACT_YES))) {
        ch->doSay(arg);
        obj_act("says 'Sorry $n, I have trouble finding that person.'", 
                ch, me, NULL, ANSI_GREEN);
        return TRUE;
      }
      break;
    case CMD_WHERE:
      one_argument(arg, buf2, cElements(buf2));
      if (!(victim = get_char_vis_world(ch, buf2, NULL, EXACT_YES))) {
        ch->doSay(arg);
        obj_act("says 'Sorry $n, I have trouble finding that person.'", 
               ch, me, NULL, ANSI_GREEN);
        return TRUE;
      }
      break;
    default:
      ch->doSay(arg);
      return TRUE;
  }
  if (cmd == CMD_SAY || cmd == CMD_SAY2)
    ch->doSay(arg);

  if (!victim->roomp) {
    obj_act("says 'Woah, big problem, talk to Brutius!'", 
        ch, me, NULL, ANSI_GREEN);
    return TRUE;
  }
  target = victim->roomp->number;

  if (victim->GetMaxLevel() > ch->GetMaxLevel()) {
    obj_act("says 'You are not powerful enough to see that person, $n!'", 
        ch, me, NULL, ANSI_GREEN);
    return TRUE;
  }
  sprintf(buf1, "%d look", target);
  ch->doAt(buf1, true);
  return TRUE;
}

int caravan_wagon(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  class wagon_struct {
    public:
      TThing *driver;

      wagon_struct() : driver(NULL)
      {
      }
      ~wagon_struct()
      {
      }
  };
  dirTypeT dir;
  wagon_struct *car;
  sstring buf;
  TRoom *rp2;

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<wagon_struct *>(me->act_ptr);
    me->act_ptr = NULL;
    return FALSE;
  }

  if (cmd == CMD_OBJ_WAGON_INIT) {
    if (me->act_ptr)
      delete static_cast<wagon_struct *>(me->act_ptr);

    car = new wagon_struct();
    if (!car) {
      vlogf(LOG_PROC, "Bad alloc (1) of caravan wagon");
      return FALSE;
    }
    me->act_ptr = car;

    car->driver = ch;

    return FALSE;
  } else if (cmd == CMD_OBJ_WAGON_UNINIT) {
    if (!(car = (wagon_struct *) me->act_ptr)) {
      vlogf(LOG_PROC, "Bad alloc (3) of caravan wagon");
      return FALSE;
    }
    if (me->act_ptr)
    car->driver = NULL;

    return FALSE;
  } else if (cmd == CMD_OBJ_MOVEMENT) {
    if (!(car = (wagon_struct *) me->act_ptr)) {
      vlogf(LOG_PROC, "Bad alloc (2) of caravan wagon");
      return FALSE;
    }
    if (ch != car->driver)
      return FALSE;

    int dum = (int) arg;
    dir = dirTypeT(dum);

    if (dir < MIN_DIR || dir >= MAX_DIR) {
      vlogf(LOG_PROC, "Problematic direction in CMD_OBJ_MOVEMENT");
      return FALSE;
    }
    buf = format("$n rolls %s.") % dirs[dir];
    act(buf, TRUE, me, 0, 0, TO_ROOM);

    rp2 = real_roomp(me->exitDir(dir)->to_room);

    (*me)--;
    *rp2 += *me;

    buf = format("$n rolls in from the %s.") % dirs[rev_dir[dir]];
    act(buf, TRUE, me, 0, 0, TO_ROOM);

    return TRUE;
  }
  return FALSE;
}

bool genericPotion(TBeing *ch, TObj *me, cmdTypeT cmd, const char *arg, int &rc)
{
 char buf[256];

  if (cmd != CMD_QUAFF && cmd != CMD_DRINK) {
    rc = FALSE;
    return true;
  }

  one_argument(arg, buf, cElements(buf));
  if (!isname(buf, me->getName())) {
    rc = FALSE;
    return true;
  }

  if (ch->hasDisease(DISEASE_FOODPOISON)) {
    ch->sendTo("Uggh, your stomach is queasy and the thought of doing that is unappetizing.\n\r");
    ch->sendTo("You decide to skip this drink until you feel better.\n\r");
    rc = TRUE;
    return true;
  }
  return false;
}

int goofersDust(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  dirTypeT dir;
  sstring buf;
  int dum = (int) arg;

  if (cmd == CMD_OBJ_MOVEMENT) {
    dir = dirTypeT(dum);
    act("$n stumbles on $p.", TRUE, ch, me, 0, TO_ROOM);
    act("You stumble on $p.", TRUE, ch, me, 0, TO_CHAR);
    if (dir < MIN_DIR || dir >= MAX_DIR) {
      vlogf(LOG_PROC, "Problematic direction in CMD_OBJ_MOVEMENT for Goofers");
      return FALSE;
    }
    if (::number(0,3) == 0) {
      buf = format("As you moved %sward, you somehow tripped and fell down.") % dirs[dir];
      act(buf, TRUE, ch, me, 0, TO_CHAR);
      buf = format("$n trips and falls as $e moves in from a %sward direction.") % dirs[dir];
      act(buf, TRUE, ch, me, 0, TO_ROOM);
      ch->setPosition(POSITION_SITTING);
      delete me;
      return TRUE;
    } else {
      return FALSE;
    }
  }
  return FALSE;
}



int bogusObjProc(TBeing *, cmdTypeT, const char *, TObj *me, TObj *)
{
  if (me)
    vlogf(LOG_PROC, format("WARNING:  %s is running around with a bogus spec_proc #%d") % 
       me->getName() % me->spec);
  else
    vlogf(LOG_PROC, "WARNING: indeterminate obj has bogus spec_proc");
  return FALSE;
}

int bleedChair(TBeing *ch, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  char buf[256], limb[256];
  int duration;
  wearSlotT slot;

  if (!ch)
    return FALSE;

  if (cmd != CMD_SIT)
    return FALSE;

  // insure some limb can be bled first...
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (notBleedSlot(slot))
      continue;
    if (!ch->slotChance(slot))
      continue;
    if (ch->isLimbFlags(slot, PART_BLEEDING))
      continue;
    break;
  }

  if (ch->isImmune(IMMUNE_BLEED, slot))
    return FALSE;

  ch->doSit(me->getName());

  ch->sendTo(format("Ouch that %shurt!%s\n\r") % ch->red() % ch->norm());

  if (slot >= MAX_WEAR) {
    // no slots to bleed...
    return TRUE;
  }

  // now pick a random slot
  for (slot = pickRandomLimb(); ;slot = pickRandomLimb()) {
    if (notBleedSlot(slot))
      continue;

    if (!ch->slotChance(slot))
      continue;

    if (ch->isLimbFlags(slot, PART_BLEEDING))
      continue;

    break;
  }
  sprintf(limb, "%s", ch->describeBodySlot(slot).c_str());
  sprintf(buf, "A gaping gash opens up on your %s!\n\rBright red blood begins to course out!", limb);
  act(buf, FALSE, ch, NULL, NULL, TO_CHAR);
  sprintf(buf, "The flesh on $n's %s opens up and begins to bleed profusely!", limb);
  act(buf, FALSE, ch, NULL, NULL, TO_ROOM);
  duration = (ch->GetMaxLevel() * 3) + 200;
  ch->rawBleed(slot, duration, SILENT_YES, CHECK_IMMUNITY_NO);
  return TRUE;
}

int harmChair(TBeing *ch, cmdTypeT cmd, const char *, TObj *, TObj *)
{
  if (!ch)
      return FALSE;

  if (cmd != CMD_SIT)
    return FALSE;

  ch->sendTo("This is a test.");
  return FALSE;
}

int featherFallItem(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  if (cmd != CMD_OBJ_START_TO_FALL)
    return FALSE;

  TBeing *ch = dynamic_cast<TBeing *>(me->equippedBy);
  if (!ch) 
    return FALSE;

  if (ch->affectedBySpell(SPELL_FEATHERY_DESCENT))
    return FALSE;

  obj_act("glows with a brilliant white light.", ch, me, NULL, ANSI_WHITE);

  affectedData aff;
  aff.type = SPELL_FEATHERY_DESCENT;
  aff.level = 10;
  aff.duration = 5;  // they don't really need it to last that long

  ch->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES);

  return FALSE;
}







int bloodDrain(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 8);
  if (!ch)
    return FALSE;

  dam = ::number(4,10);
  act("$p <1><r>pulses with an <k>unholy<1><r> light and oozes blood as it draws <y>life<1><r> essence from<1> $n.",
      0, vict, o, 0, TO_ROOM);
  act("$p <1><r>pulses with an <k>unholy<1><r> light and oozes blood as it draws your very <y>life<1><r> essence from you<1>.",
      0, vict, o, 0, TO_CHAR);

  ch->dropPool(3, LIQ_BLOOD);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_DRAIN);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}



int stoneAltar(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *obj, TObj *)
{
  if (!ch)
    return FALSE;

  if (cmd != CMD_PUSH && cmd != CMD_PRESS)
    return FALSE;

  char buf[256];
  one_argument(arg, buf, cElements(buf));
  if (is_abbrev(buf, "eye") || is_abbrev(buf, "diamond")) {
    TOpenContainer *trc = dynamic_cast<TOpenContainer *>(obj);
    if (!trc)
      return FALSE;
    if (trc->isClosed()) {
      act("You push on the diamond eye, causing $p to open.",
             TRUE, ch, trc, NULL, TO_CHAR);
      act("$n fiddles with $p, causing it to open.",
             TRUE, ch, trc, NULL, TO_ROOM);
      trc->remContainerFlag(CONT_CLOSED);
    } else {
      act("You push on the diamond eye, causing $p to close.",
             TRUE, ch, trc, NULL, TO_CHAR);
      act("$n fiddles with $p, causing it to close.",
             TRUE, ch, trc, NULL, TO_ROOM);
      trc->addContainerFlag(CONT_CLOSED);
    }
    return TRUE;
  }
  return FALSE;
}


// o is being hit, ch is o's owner, v is doing the hitting, with weapon
int behirHornItem(TBeing *v, cmdTypeT cmd, const char *, TObj *o, TObj *weapon)
{
  TBeing *ch;
  int rc, dam;
  wearSlotT t;
  TObj *savedby=NULL;
  char buf[256];

  if(cmd != CMD_OBJ_BEEN_HIT || !v || !o)
    return FALSE;
  if(::number(0, 3))
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;     

  if (weapon && !material_nums[weapon->getMaterial()].conductivity)
    savedby=weapon;
  t=((!weapon || (weapon->eq_pos==HOLD_RIGHT))?WEAR_HAND_R:WEAR_HAND_L);
  if(v->equipment[t] && 
     !material_nums[v->equipment[t]->getMaterial()].conductivity)
    savedby=dynamic_cast<TObj *>(v->equipment[t]);

  act("$p <B>flares up brightly and <W>jolts<B> $n <B>with an electric shock!<1>"
      , 0, v, o, 0, TO_ROOM);
  act("$p <B>flares up brightly and <W>jolts<B> you with an electric shock!<1>"
      , 0, v, o, 0, TO_CHAR);
    
  if(savedby){
    sprintf(buf, "<k>Luckily, $s <1>$o<k> is not conductive and saves $m from harm.<1>");
    act(buf, 0, v, savedby, 0, TO_ROOM);
    sprintf(buf, "<k>Luckily, your <1>$o<k> is not conductive and saves you from harm.<1>");
    act(buf, 0, v, savedby, 0, TO_CHAR);
  } else {
    dam = ::number(5, 20);
    
    rc = ch->reconcileDamage(v, dam, DAMAGE_ELECTRIC);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }

  return TRUE;
}






// Is what is says, This is a special proc that will one day help newbies
// understand sneezy more.
int newbieHelperWProc(TBeing *vict, cmdTypeT cmd, const char *Parg, TObj *o, TObj *)
{
  TBeing *ch;
  char PargA[30], // Should be 'help'
       Topic[30], // One of a series of possible help topics.
       buf[30];

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (((o->equippedBy != ch) && (o->parent != ch)) ||
      (!ch->isImmortal() && (ch->GetMaxLevel() > 4)))
    return FALSE;

  Parg = one_argument(Parg, PargA, cElements(PargA));
  Parg = one_argument(Parg, Topic, cElements(Topic));
  ch->sendTo(format("Newbie Weapon Info: %s %s\n\r") % PargA % Topic);

  switch (cmd) {
    case CMD_SAY:
      if (is_abbrev(PargA, "help"))
        if (!*Topic) {
          ch->sendTo(format("%s: Hello %s.  I am a newbie helper weapon.\n\r") %
                     o->getName() % ch->getName());
          ch->sendTo(format("%s: I will lend assistance when you need it.\n\r") %
                     o->getName());
          ch->sendTo(format("%s: This is how you use me:\n\r") %
                     o->getName());
          ch->sendTo(format("%s:      help ??? where ??? is one of the following:\n\r") %
                     o->getName());
          ch->sendTo(format("%s:  basics   : A basic helpfile to get your started.\n\r") %
                     o->getName());
          ch->sendTo(format("%s:  goto     : Covers the 'donation' command.\n\r") %
                     o->getName());
          ch->sendTo(format("%s:  donation : Covers the donation room.\n\r") %
                     o->getName());
          ch->sendTo(format("%s:  kill     : This covers the 'kill' command.\n\r") %
                     o->getName());
          ch->sendTo(format("%s:  consider : This covers a very important command, 'consider'.\n\r") %
                     o->getName());
          return TRUE;
        } else {
          if (is_abbrev(Topic, "basics")) {
            ch->sendTo(format("%s: The first command to get used to is 'help' without the say.\n\r") %
                       o->getName());
            ch->sendTo(format("%s: The help command will cover most everything you need plus more.\n\r") %
                       o->getName());
            ch->sendTo(format("%s: You should have also gotten a newbie book.  To use this book:\n\r") %
                       o->getName());
            ch->sendTo(format("%s:   read newbie\n\r") %
                       o->getName());
            ch->sendTo(format("%s: If you get in a jam and need help that you can't seem to find.\n\r") %
                       o->getName());
            ch->sendTo(format("%s: Then do a:  who  and look for anyone with (Newbie-helper) to\n\r") %
                       o->getName());
            ch->sendTo(format("%s: the right of there name.  These people are here to help you learn\n\r") %
                       o->getName());
            ch->sendTo(format("%s: and understand the mud.  If there is nobody on with that, then\n\r") %
                       o->getName());
            ch->sendTo(format("%s: use the  who  command and look for a friendly name.\n\r") %
                       o->getName());
            ch->sendTo(format("%s: Once you have a person you wish to ask, talk to them with:\n\r") %
                       o->getName());
            ch->sendTo(format("%s:   tell player_name what_to_ask\n\r") %
                       o->getName());
            ch->sendTo(format("%s: Example:  tell mrfriendly Hi, i'm new here.  Can you help me?\n\r") %
                       o->getName());
            ch->sendTo(format("%s: There are certain communication rules which you should know:\n\r") %
                       o->getName());
            ch->sendTo(format("%s:   1. Please don't use all CAPITAL LETTERS.\n\r") %
                       o->getName());
            ch->sendTo(format("%s:   2. Please avoid profanity, try and keep it clean.\n\r") %
                       o->getName());
            ch->sendTo(format("%s:   3. If you ask someone for help and they don't reply or choose\n\r") %
                       o->getName());
            ch->sendTo(format("%s:      not to help you, please don't give repetive tells to them.\n\r") %
                       o->getName());
            ch->sendTo(format("%s:   4. When you obtain level 2 and the shout command, please don't\n\r") %
                       o->getName());
            ch->sendTo(format("%s:      abuse it.  See  help shout  about this command and rules.\n\r") %
                       o->getName());
          } else if (is_abbrev(Topic, "donation")) {
            ch->sendTo(format("%s: Surplus items are left here for others.\n\r") %
                       o->getName());
            ch->sendTo(format("%s: When taking from surplus please use these rules:\n\r") %
                       o->getName());
            ch->sendTo(format("%s:    1. Take only what you need and can use.\n\r") %
                       o->getName());
            ch->sendTo(format("%s:    2. Please don't take things from surplus then sell them.\n\r") %
                       o->getName());
            ch->sendTo(format("%s:    3. If you take from surpluse, please put something back at\n\r") %
                       o->getName());
            ch->sendTo(format("%s:       a later date.\n\r") %
                       o->getName());
            ch->sendTo(format("%s: To get to donation, please use the  goto surplus  command.\n\r") %
                       o->getName());
          } else if (is_abbrev(Topic, "kill")) {
            ch->sendTo(format("%s: You use this command to start a fight.  It is usually a good idea\n\r") %
                       o->getName());
            ch->sendTo(format("%s: to  consider  your target before attacking him.  And also a good\n\r") %
                       o->getName());
            ch->sendTo(format("%s: idea to make sure you have all your equipment and any weapons you\n\r") %
                       o->getName());
            ch->sendTo(format("%s: may have equiped.  Also a good idea to have used any of your\n\r") %
                       o->getName());
            ch->sendTo(format("%s: practices you might have.\n\r") %
                       o->getName());
          } else if (is_abbrev(Topic, "consider")) {
            ch->sendTo(format("%s: When you consider something the mud will tell you what your\n\r") %
                       o->getName());
            ch->sendTo(format("%s: probable chances are at winning.  The consider command is not\n\r") %
                       o->getName());
            ch->sendTo(format("%s: always accurate.  But it makes for a good idea of what level your\n\r") %
                       o->getName());
            ch->sendTo(format("%s: target is.\n\r") %
                       o->getName());
          } else if (is_abbrev(Topic, "goto")) {
            ch->sendTo(format("%s: While you're in Grimhaven you can use the goto command to get\n\r") %
                       o->getName());
            ch->sendTo(format("%s: to the more important places.  These are the more important ones:\n\r") %
                       o->getName());
            ch->sendTo(format("%s:    goto cs        : Center Square  [Food/Water]\n\r") %
                       o->getName());
            ch->sendTo(format("%s:    goto mail      : Post office    [MudMail/You have mail!]\n\r") %
                       o->getName());
            if (!ch->hasClass(CLASS_MONK))
            ch->sendTo(format("%s:    goto weapon    : Weapon Shop    [Swords/Daggers/Clubs]\n\r") %
                       o->getName());
            if (!ch->hasClass(CLASS_MONK) && !ch->hasClass(CLASS_MAGE))
            ch->sendTo(format("%s:    goto armor     : Armor Shop     [Metallic Armor]\n\r") %
                       o->getName());
            ch->sendTo(format("%s:    goto food      : Food Shop      [Provisions/Rations/Bread]\n\r") %
                       o->getName());
            if (ch->hasClass(CLASS_CLERIC) || ch->hasClass(CLASS_DEIKHAN))
            ch->sendTo(format("%s:    goto symbol    : Symbol Shop    [Holy Symbols]\n\r") %
                       o->getName());
            ch->sendTo(format("%s:    goto commodity : Commodity Shop [bits of tin/ingots of copper]\n\r") %
                       o->getName());
            if (ch->hasClass(CLASS_MAGE) || ch->hasClass(CLASS_RANGER))
            ch->sendTo(format("%s:    goto component : Component Shop [Mage Spell Components]\n\r") %
                       o->getName());
            ch->sendTo(format("%s:    goto doctor    : Doctor         [Heal bleeding Wounds]\n\r") %
                       o->getName());
            if (ch->hasClass(CLASS_CLERIC) || ch->hasClass(CLASS_DEIKHAN))
            ch->sendTo(format("%s:    goto attuner   : Attuner        [Makes Holy Symbols Usable]\n\r") %
                       o->getName());
            ch->sendTo(format("%s:    goto tanner    : Tanning Shop   [Leather Armor]\n\r") %
                       o->getName());
            ch->sendTo(format("%s:    goto surplus   : Donation Room  [Surplus Items]\n\r") %
                       o->getName());
            if (ch->GetMaxLevel() < 2)
            ch->sendTo(format("%s:    goto park      : Newbie Area    [Basic area for newbies]\n\r") %
                       o->getName());
          } else return FALSE; // He didn't call on us for help, maybe another player?
          return TRUE; // If we got here, we had a topic so lets eat the command.
        }
      return FALSE;
    case CMD_LIST:
      if (!*Topic) {
        ch->sendTo(format("%s: You should use one of the following when using the list command:\n\r") %
                   o->getName());
        ch->sendTo(format("%s: list fit    : To list only that which will fit you.\n\r") %
                         o->getName());
        ch->sendTo(format("%s: list <slot> : To list armor by worn location\n\r") %
                         o->getName());
        ch->sendTo(format("%s:   head, neck, body, back, arm, wrist, hand, waist, leg, feet, finger\n\r") %
                         o->getName());
        ch->sendTo(format("%s: list #1 #2: To list armor which cost no more than #2 but at least #1\n\r") %
                         o->getName());
        ch->sendTo(format("%s: And when buying, always be careful not to exceed your rent limit.\n\r") %
                         o->getName());
      }
      return FALSE;
    case CMD_CONSIDER:
      if (!*Topic) {
        ch->sendTo(format("%s: It is very important to understand what this command tells you.\n\r") %
                         o->getName());
        ch->sendTo(format("%s: If the outcome looks bad, you might not want to try it.  But always\n\r") %
                         o->getName());
        ch->sendTo(format("%s: be wary no matter how easy a target may look.\n\r") %
                         o->getName());
        ch->sendTo(format("%s: You can even consider yourself to get an idea of how good/bad your\n\r") %
                         o->getName());
        ch->sendTo(format("%s: own armor is at the moment for your level.\n\r") %
                         o->getName());
        sprintf(buf, "consider %s", ch->getName());
        ch->addCommandToQue(buf);
        return TRUE;
      }
    default:
      break;
  }

  return FALSE;
}

int maquahuitl(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  int randomizer = ::number(0,9);
  TGenWeapon *weapon = dynamic_cast<TGenWeapon *>(o);
  // Proc goes off like mad but damage is way minimal to produce the 
  // affect of a blunt item slashing

  if(!(ch=genericWeaponProcCheck(vict, cmd, weapon, 0)))
     return FALSE;

  if (randomizer >= 5) {
    weapon->setWeaponType(WEAPON_TYPE_SLASH);
  } else {
    weapon->setWeaponType(WEAPON_TYPE_SMITE);
  }
  return TRUE;
}



int teleportRing(TBeing *, cmdTypeT cmd, const char *arg, TObj *o, TObj *){
  int rc;
  TBeing *vict;

  if (!(vict = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
 
  if(cmd!=CMD_GENERIC_PULSE || ::number(0,100))
    return FALSE;

  act("Your $o flares up brightly and you suddenly feel very dizzy and disoriented.", TRUE, vict, o, NULL, TO_CHAR);
  act("$n's $o flares up brightly and $e disappears!", TRUE, vict, o, NULL, TO_ROOM);
  
  rc = vict->genericTeleport(SILENT_NO, FALSE);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete vict;
    vict = NULL;
  }

  return TRUE;
}


int teleportingObject(TBeing *, cmdTypeT cmd, const char *arg, TObj *o, TObj *){
  int rc;
  TPortal *tp;

  if(cmd!=CMD_GENERIC_PULSE || ::number(0,250))
    return FALSE;

  // don't teleport if it's a portal that is open
  if((tp=dynamic_cast<TPortal *>(o)) && !tp->isPortalFlag(EX_CLOSED))
    return FALSE;

  if(!tp->roomp)
    return FALSE;

  tp->roomp->sendTo(COLOR_BASIC, format("%s flares up brightly and disappears.\n\r") %
		    sstring(o->getName()).cap());
  
  rc = o->genericTeleport(SILENT_YES, FALSE);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete o;
    o = NULL;
  }

  return TRUE;
}




int trolley(TBeing *, cmdTypeT cmd, const char *, TObj *myself, TObj *)
{  
  int *job=NULL, where=0, i;
  int path[]={-1, 100, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185,
	      200, 215, 31050, 31051, 31052, 31053, 31054, 31055, 31056, 
	      31057, 31058, 31059, 31060, 31061, 31062, 31063, 31064, 31065, 
	      31066, 31067, 31068, 31069, 31070, 31071, 31072, 31073, 31074, 
	      31075, 31076, 31077, 31078, 31079, 31080, 31081, 31082, 31083, 
	      31084, 31085, 31086, 31087, 31088, 31089, 
	      650, 651, 652, 653, 654, 655, 656, 657, 658, 659,
	      660, 667, 668, 669, 670, 671, 672, 673, 674, 700, 702,
	      703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714,
	      715, 716, 728, 729, 730, 731, 732, 733, 734, 
	      34768, 34767, 34766, 34765, 34764, 34763, 34762, 34761, 
	      34760, 34759, 34758, 34757, 34756, 34755, 34754, 34753,
	      34751, 34750, 34749, 34748, 34747, 34746, 34745, 34744,
	      34743, 34742, 34741, 34740, 34739, 34738, 34737, 34736,
	      34735, 34734, 34733, 34732, 34731, 34730, 34729, 34728,
	      34727, 34726, 34725, 34724, 34723, 34722, 34721, 34720,
	      34719, 34718, 34717, 34716, 34715, 34714, 34713,
	      34712, 34711, 34710, 34709, 34708, 34707, 34706, 34705,
	      34704, 34703, 34702, 34701, 34700, 735, 736, 737,
	      738, 739, 1381, 1200, 1201, 1204, 1207, 1215, 1218, 1221, 
	      1301, 1302, 1303, -1};
  TRoom *trolleyroom=real_roomp(Room::TROLLEY);
  static int timer;
  char buf[256], shortdescr[256];

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<int *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if(!myself->in_room)
    return FALSE;

  if(timer>0){
    --timer;
    return FALSE;
  }

  strcpy(shortdescr, myself->shortDescr);
  strcpy(shortdescr, sstring(shortdescr).cap().c_str());

  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new int)) {
     perror("failed new of trolley.");
     exit(0);
    }
    job = static_cast<int *>(myself->act_ptr);
    *job=1;
  } else {
    job = static_cast<int *>(myself->act_ptr);
  }

  for(where=1;path[where]!=-1 && myself->in_room != path[where];++where);

  if(path[where]==-1){
    vlogf(LOG_PEEL, "trolley lost");
    return FALSE;
  }

  if((path[where+*job])==-1){
    switch(*job){
      case -1:
	sendrpf(COLOR_OBJECTS, trolleyroom, "%s has arrived in Grimhaven.\n\r",
		shortdescr);
	break;
      case 1:
	sendrpf(COLOR_OBJECTS, trolleyroom, "%s has arrived in Brightmoon.\n\r",
		shortdescr);
	break;
    }

    *job=-*job;
    timer=10;
    return TRUE;
  }

  
  for(i=MIN_DIR;i<MAX_DIR;++i){
    if(myself->roomp->dir_option[i] &&
       myself->roomp->dir_option[i]->to_room==path[where+*job]){
      break;
    }
  }
  
  switch(*job){
    case -1: 
      sprintf(buf, "$n continues %s towards Grimhaven.",
	      (i==MAX_DIR)?"on":dirs[i]);
      act(buf,FALSE, myself, 0, 0, TO_ROOM); 
      sendrpf(COLOR_OBJECTS, trolleyroom, "%s rumbles %s towards Grimhaven.\n\r",
	      shortdescr, (i==MAX_DIR)?"on":dirs[i]);
      sendrpf(COLOR_OBJECTS, real_roomp(path[where+*job]), 
	      "%s enters the room, heading towards Grimhaven.\n\r",
	      shortdescr);
      break;
    case 1: 
      sprintf(buf, "$n continues %s towards Brightmoon.",
	      (i==MAX_DIR)?"on":dirs[i]);
      act(buf,FALSE, myself, 0, 0, TO_ROOM); 
      sendrpf(COLOR_OBJECTS, trolleyroom, "%s rumbles %s towards Brightmoon.\n\r",
	      shortdescr, (i==MAX_DIR)?"on":dirs[i]);
      sendrpf(COLOR_OBJECTS, real_roomp(path[where+*job]), 
	      "%s enters the room, heading towards Brightmoon.\n\r",
	      shortdescr);
      break;
  }
  
  --(*myself);
  *real_roomp(path[where+*job])+=*myself;

  if(!trolleyroom->dir_option[0]){
    trolleyroom->dir_option[0] = new roomDirData();
  }
  
  trolleyroom->dir_option[0]->to_room=path[where+*job];

  return TRUE;
}

int fishingBoat(TBeing *, cmdTypeT cmd, const char *, TObj *myself, TObj *)
{  
  int *job=NULL, where=0, i, found=1;
  int path[]={-1, 15150, 
	      2439, 2440, 2441, 2442, 2443, 2444, 2445, 2446, 2447, 2448,
	      2449, 2450, 2451, 2452, 2453, 2454, 2455, 2456, 2457, 2458,
	      2459, 2460, 2461, 2462, 2463, 2464, 2465, 2466, 2467, 2468,
	      2469, 2470, 2471, 2475, 12551, 12583, 12616, 12651, 12690,
	      12733, 12770, 12803, 12831, 12857, 12886, 12911, 12935, 12958,
	      12982, 13006, 13030, 13052, 13072, 13091, -1};
  const sstring boatleaving[]={
                   "The fishing boat is going to be leaving immediately.\n\r",
		   "The fishing boat is almost ready to leave.\n\r",
		   "The fishing boat will be leaving very soon.\n\r",
		   "The fishing boat will be leaving soon.\n\r",
		   "The fishing boat is getting ready to leave.\n\r"};
  TRoom *boatroom=real_roomp(15349);
  static int timer;
  char buf[256], shortdescr[256];
  TThing *tt=NULL;

  // docks 15150

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<int *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if(!myself->in_room)
    return FALSE;

  if(::number(0,4)) // slow it down a bit, this is a fishing trip after all
    return FALSE;

  // we just idle in 15150 if we're empty
  if(myself->in_room == 15150){
    found=0;
    for(StuffIter it=boatroom->stuff.begin();it!=boatroom->stuff.end() && (tt=*it);++it){
      if(dynamic_cast<TPerson *>(tt)){
	found=1;
	break;
      }
    }
    if(!found)
      return FALSE;
    else if(timer <= 0){
      if(myself->in_room == 15150){
	sendrpf(real_roomp(15150), "The fishing boat is preparing to leave.\n\r");
      } else if(myself->in_room == 13091){
	sendrpf(real_roomp(13108), "The fishing boat is preparing to leave.\n\r");
      }
      sendrpf(boatroom, "The fishing boat is preparing to leave.\n\r");
      
      timer=5;
    }
  }

  if((--timer)>0){

    if(timer<=5 && timer>0){
      if(myself->in_room == 15150){
	sendrpf(real_roomp(15150), boatleaving[timer-1].c_str());
      } else if(myself->in_room == 13091){
	sendrpf(real_roomp(13108), boatleaving[timer-1].c_str());
      }
      sendrpf(boatroom, boatleaving[timer-1].c_str());
    }

    return FALSE;
  }

  strcpy(shortdescr, myself->shortDescr);
  strcpy(shortdescr, sstring(shortdescr).cap().c_str());

  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new int)) {
     perror("failed new of fishing boat.");
     exit(0);
    }
    job = static_cast<int *>(myself->act_ptr);
    *job=1;
  } else {
    job = static_cast<int *>(myself->act_ptr);
  }

  for(where=1;path[where]!=-1 && myself->in_room != path[where];++where);

  if(path[where]==-1){
    vlogf(LOG_PEEL, "fishing boat lost");
    return FALSE;
  }

  if((path[where+*job])==-1){
    switch(*job){
      case -1:
	sendrpf(COLOR_OBJECTS, boatroom, "%s has arrived at the docks.\n\r",
		shortdescr);
	break;
      case 1:
	sendrpf(COLOR_OBJECTS, boatroom, "%s has arrived at the island.\n\r",
		shortdescr);
	timer=10;
	break;
    }

    *job=-*job;
    return TRUE;
  }

  
  for(i=MIN_DIR;i<MAX_DIR;++i){
    if(myself->roomp->dir_option[i] &&
       myself->roomp->dir_option[i]->to_room==path[where+*job]){
      break;
    }
  }
  
  switch(*job){
    case -1: 
      sprintf(buf, "$n continues %s towards the docks.",
	      (i==MAX_DIR)?"on":dirs[i]);
      act(buf,FALSE, myself, 0, 0, TO_ROOM); 
      sendrpf(COLOR_OBJECTS, boatroom, "%s sails %s towards land.\n\r",
	      shortdescr, (i==MAX_DIR)?"on":dirs[i]);
      sendrpf(COLOR_OBJECTS, real_roomp(path[where+*job]), 
	      "%s enters the room, heading towards land.\n\r",
	      shortdescr);
      break;
    case 1: 
      sprintf(buf, "$n continues %s out to sea.",
	      (i==MAX_DIR)?"on":dirs[i]);
      act(buf,FALSE, myself, 0, 0, TO_ROOM); 
      sendrpf(COLOR_OBJECTS, boatroom, "%s sails %s out to sea.\n\r",
	      shortdescr, (i==MAX_DIR)?"on":dirs[i]);
      sendrpf(COLOR_OBJECTS, real_roomp(path[where+*job]), 
	      "%s enters the room, heading out to sea.\n\r",
	      shortdescr);
      break;
  }
  
  --(*myself);
  *real_roomp(path[where+*job])+=*myself;

  if(!boatroom->dir_option[0]){
    boatroom->dir_option[0] = new roomDirData();
  }
  
  boatroom->dir_option[0]->to_room=path[where+*job];

  return TRUE;
}


int squirtGun(TBeing *vict, cmdTypeT cmd, const char *Parg, TObj *o, TObj *)
{
  TBeing *ch;
  char //Command[30], // Should be 'squirt'
    Target[30]; // target to be soaked!
    
  TDrinkCon *gun=dynamic_cast<TDrinkCon *>(o);

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (((o->equippedBy != ch) && (o->parent != ch)))
    return FALSE;
  Parg = one_argument(Parg, Target, cElements(Target));
  if (!(cmd == CMD_SHOOT))
    return FALSE;
  if (!gun) {
    vlogf(LOG_PROC, "Squirt Gun proc on an object that isn't a drink container.");
    return FALSE;
  }
  
  if(gun->getDrinkUnits() < 1) {
    act("<1>You squeeze the trigger with all your might, but $p appears to be empty.",TRUE,ch,o,vict,TO_CHAR,NULL);  
    act("<1>$n squeezes the trigger on $s $p, but nothing happens.",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    return TRUE;
  }
  else {
    TBeing *squirtee;
    int bits = generic_find(Target, FIND_CHAR_ROOM, ch, &squirtee, &o);
    if(!bits) {
      ch->sendTo("You don't see them here.\n\r");
      return TRUE;
    } else {
      const sstring liqname =liquidInfo[gun->getDrinkType()]->name;
      int shot = (::number(1,min(5,gun->getDrinkUnits())));
      gun->addToDrinkUnits(-shot);
      ch->dropPool(shot, gun->getDrinkType());
      
      /*act("<1>You squeeze the trigger on your $p.",TRUE,ch,gun,squirtee,TO_CHAR,NULL);
	ch->sendTo(COLOR_OBJECTS, format("A deadly stream of %s squirts at %s!\n\r") %liqname % squirtee->getName());
	act("<1>$n squeezes the trigger on $s $p, shooting a deadly stream of liquid at $N!"
	,TRUE,ch,gun,squirtee,TO_NOTVICT,NULL);
	
	act("<1>$n squeezes the trigger on $s $p.",TRUE,ch,gun,squirtee,TO_VICT,NULL);
	squirtee->sendTo(COLOR_OBJECTS, format("A deadly stream of %s squirts at you!\n\r") %liqname);
      */
      char Buf[256];
      sprintf(Buf, "You squeeze the trigger on $p, squirting a deadly stream of %s at $N!", liqname.c_str());
    act(Buf, TRUE, ch, gun, squirtee, TO_CHAR);
    sprintf(Buf, "$n squeezes the trigger on $p, squirting a deadly stream of %s at $N!", liqname.c_str());
    act(Buf, TRUE, ch, gun, squirtee, TO_NOTVICT);
    sprintf(Buf, "$n squeezes the trigger on $p, squirting a deadly stream of %s at you!", liqname.c_str());
    act(Buf, TRUE, ch, gun, squirtee, TO_VICT);
    if (shot>4) {
    char Buf2[256];
    sprintf(Buf2, "$N is totally soaked with %s!", liqname.c_str());
    act(Buf2, TRUE, ch, gun, squirtee, TO_CHAR);
    act(Buf2, TRUE, ch, gun, squirtee, TO_NOTVICT);
    sprintf(Buf2, "You're totally soaked with %s!", liqname.c_str());
    act(Buf2, TRUE, ch, gun, squirtee, TO_VICT);
    }
    return TRUE;
  }
}
 return FALSE;
}

int fireGlove(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam = 1;

  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (::number(0,5))
    return FALSE;
  if (cmd != CMD_OBJ_HIT)
    return FALSE;
  dam = (::number( 1, (ch->GetMaxLevel()) / 10 + 2));

  act("<o>Your $o bursts into <r>flame<1><o> as you strike $N<1><o>!<1>",
      TRUE,ch,o,vict,TO_CHAR,NULL);
  act("<o>$n's $o bursts into <r>flame<1><o> as $e strikes $N<1><o>!<1>",
      TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("<o>$n'a $o bursts into <r>flame<1><o> as $e strikes you!<1>",
      TRUE,ch,o,vict,TO_VICT,NULL);
  
  rc = ch->reconcileDamage(vict, dam, DAMAGE_FIRE);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}


int razorGlove(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam = 1, which;
  
  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (::number(0,5))
    return FALSE;
  if (cmd != CMD_OBJ_HIT)
    return FALSE;
  dam = (::number( 1, (ch->GetMaxLevel()) / 10 + 2));
  which = ::number(1,2);

  act("<k>Three long, thin blades spring from your <1>$o<k>.<1>",
      TRUE,ch,o,vict,TO_CHAR,NULL);
  act("<k>Three long, thin blades spring from $n's <1>$o<k>.<1>",
      TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("<k>Three long, thin blades spring from $n's <1>$o<k>.<1>",
      TRUE,ch,o,vict,TO_VICT,NULL);


  if (which == 1) {
    act("<k>You <g>slice<k> $N with the <1>blades<k> of your $o, which then retract.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<k>$n <1>slices<k> $N with the <1>blades<k> of $s $o, which then retract.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<k>$n <r>slices<k> you with the <1>blades<k> of $s $o, which then retract.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
  }
  else {
    act("<k>You <g>stab<k> $N with the <1>blades<k> of your $o, which then retract.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<k>$n <1>stabs<k> $N with the <1>blades<k> of $s $o, which then retract.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<k>$n <r>stabs<k> you with the<1> blades<k> of $s $o, which then retract.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
  }

  rc = ch->reconcileDamage(vict, dam, TYPE_PIERCE);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}

int keyInKnife(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *) 
{
  if (!(ch = dynamic_cast<TBeing *>(o->parent)))
    return FALSE;
  TObj *key = NULL; 
  char buf[256];

  if (cmd != CMD_PUSH && cmd != CMD_PRESS) 
    return FALSE;
 
  if (!(key = read_object(17211, VIRTUAL))) {
    vlogf(LOG_PROC, format("Key in Knife -- bad read of object (%s)") %  ch->getName());
    return FALSE;
  }
 
  one_argument(arg, buf, cElements(buf));
  
  
  if(!strcmp(buf,"panel")) {
    *ch += *key;
    act("You press on the hilt of $p. *click*",TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("The entire $o splits down the center, revealing a <Y>golden key<1>.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("$n fiddles with the hilt of $p. *click*.",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("The entire $o splits down the center, revealing a small <Y>golden<1> object.",TRUE,ch,o,NULL,TO_ROOM,NULL);
    if(!o->makeScraps())
      delete o;
    return TRUE;
  }
  return FALSE;
}


int teleportVial(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *) 
{
  int targetroom;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  char objname[256],buf[256];
  if (cmd != CMD_THROW) 
    return FALSE;
  strcpy(objname,o->getName());
  one_argument(one_argument(one_argument(objname,buf, cElements(buf)),buf, cElements(buf)),buf, cElements(buf)); //vial

  if (sscanf(buf,"%d",&targetroom) != 1) {
    act("Teleport vial with no target room. How crappy.",TRUE,ch,o,NULL,TO_CHAR,NULL);
  } else {
    TRoom *newRoom;
    if (!(newRoom = real_roomp(targetroom))) {
      act("Teleport vial targeted to a non-existant room. How crappy.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      return TRUE;
    }
    act("You throw $p to the ground.",TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("The $o shatters, releasing a cloud of thick smoke all around you.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    
    act("$n throws $p to the ground.",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("The $o shatters, releasing a cloud of thick smoke all around $m.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    
    
    act("You feel the world shift around you.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("When the smoke clears, $n is gone!<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    --(*ch);
    delete o;  
    *newRoom += *ch;
    vlogf(LOG_PROC, format("TELEPORT VIAL: %s transfered to room #%d") %  ch->getName() % targetroom);
    act("$n appears in the room with a puff of smoke.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    ch->doLook("", CMD_LOOK);
    ch->addToWait(combatRound(2));
    ch->cantHit += ch->loseRound(1);
    if (ch->riding) {
      int rc = ch->fallOffMount(ch->riding, POSITION_STANDING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	return DELETE_THIS;
      }
    }
    return TRUE;
  }
  return FALSE;
}





int mechanicalWings(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (cmd == CMD_FLY && ch->getPosition() == POSITION_STANDING) {
    act("<k>$n<k>'s $o silently unfold and begin to beat the air forcefully.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("<k>Your $o silently unfold and begin to beat the air forcefully.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    return FALSE;
  } else if (cmd == CMD_LAND && ch->getPosition() == POSITION_FLYING) {
    act("<k>$n<k>'s $o stop beating the air and silently fold behind $s back.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("<k>Your $o stop beating the air and silently fold behind your back.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    return FALSE;
  } else if ((cmd == CMD_GENERIC_PULSE) && (::number(1,10) == 1) && (ch->getPosition() == POSITION_FLYING)) {
    act("<k>The $o on $n<k>'s back forcefully beat the air.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("<k>The $o on your back forcefully beat the air.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    return FALSE;
  }
  return FALSE;
}




int stoneSkinAmulet(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    sstring buf=sstring(arg).word(0);

    if(buf=="fortify"){
      if(ch->affectedBySpell(SPELL_FLAMING_FLESH)) {
        act("The $o's cannot function while you are affected by flaming flesh.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return FALSE;
      } else if(ch->checkForSkillAttempt(SPELL_STONE_SKIN)) {
        act("The $o's powers can only be used once per day.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return FALSE;
      }
      affectedData aff1, aff2, aff3;
      
      act("$n grips $p in one hand, and utters the word, '<p>fortify<1>'.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<k>The $o glows for a moment, and $s skin suddenly turns rock hard.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
      
      act("You grip $p in one hand, and utter the word, '<p>fortify<1>'.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<k>The $o glows for a moment, and your skin suddenly turns rock hard.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
      
      // ARMOR APPLY
      aff1.type = SPELL_STONE_SKIN;
      aff1.level = 30;
      aff1.duration = 8 * UPDATES_PER_MUDHOUR;
      aff1.location = APPLY_ARMOR;
      aff1.modifier = -75;
      
      // PIERCE IMMUNITY
      aff2.type = SPELL_STONE_SKIN;
      aff2.level = 30;
      aff2.duration = 8 * UPDATES_PER_MUDHOUR;
      aff2.location = APPLY_IMMUNITY;
      aff2.modifier = IMMUNE_PIERCE;
      aff2.modifier2 = 15;
      
      // SKILL ATTEMPT (PREVENT IMMEDIATE RE-USE)
      aff3.type = AFFECT_SKILL_ATTEMPT;
      aff3.level = 0;
      aff3.duration = 24 * UPDATES_PER_MUDHOUR;
      aff3.location = APPLY_NONE;
      aff3.modifier = SPELL_STONE_SKIN;
      
      ch->affectTo(&aff1);
      ch->affectTo(&aff2);
      if (!(ch->isImmortal())) ch->affectTo(&aff3);
      ch->addToWait(combatRound(3));
      return TRUE;
    }
  }
  return FALSE;
}
  

int lifeLeechGlove(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  char target[30];
  TBeing *victim = NULL;
  TObj *corpse = NULL;
  TBaseCorpse *body = NULL;

  if (cmd != CMD_GRAB)
    return FALSE;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  arg = one_argument(arg, target, cElements(target));
  int bits = generic_find(target, FIND_CHAR_ROOM | FIND_OBJ_ROOM, ch, &victim, &corpse);
  if(!bits)
    return FALSE;
  if(victim == ch) {
    act("Dude... like, no.",TRUE,ch,o,victim,TO_CHAR,NULL);
    return FALSE;
  }

  if(victim) {
    int chance = 0,roll = 0;
    chance = victim->GetMaxLevel() + 30;
    roll = ::number(1,50+ch->GetMaxLevel());
    if (chance > roll && victim->getPosition() > POSITION_SLEEPING) {
      act("You try to grab $N, but $E dodges out of the way.",TRUE,ch,o,victim,TO_CHAR,NULL);
      act("$n tries to grab you, but you dodge out of the way.",TRUE,ch,o,victim,TO_VICT,NULL);
      act("$n tries to grab $N, but $E dodges out of the way.",TRUE,ch,o,victim,TO_NOTVICT,NULL);
      ch->addToWait(combatRound(3));
      ch->cantHit += ch->loseRound(2);
      return TRUE;
    } else if (victim->isUndead()) {  // heheh trying to drain negative plane monsters is BAD!!
      act("You deftly grab $N, and your $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_CHAR,NULL);
      act("<k>You scream in pain as your life is sucked backwards through the conduit!<1>",TRUE,ch,o,victim,TO_CHAR,NULL);
      act("Maybe that wasn't such a good idea....",TRUE,ch,o,victim,TO_CHAR,NULL);
      act("$n deftly grabs you, and $s $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_VICT,NULL);
      act("<k>$n<k> screams in pain as $s life is sucked backwards through the conduit!<1>",TRUE,ch,o,victim,TO_VICT,NULL);
      act("Heh heh heh. That sucker tried to drain an undead!",TRUE,ch,o,victim,TO_VICT,NULL);
      act("$n deftly grabs $N, and $s $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_NOTVICT,NULL);
      act("<k>$n<k> screams in pain as $s life is sucked backwards through the conduit!<1>",TRUE,ch,o,victim,TO_NOTVICT,NULL);       
      int dam = victim->GetMaxLevel();
      int rc = victim->reconcileDamage(ch, dam, DAMAGE_DRAIN);
      victim->setHit(min((int)(victim->getHit() + victim->GetMaxLevel()),(int)(victim->hitLimit())));
      if (rc == -1)
        delete victim;
      ch->addToWait(combatRound(3));
      ch->cantHit += ch->loseRound(2);
      return TRUE;
    }
    act("You deftly grab $N, and your $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_CHAR,NULL);
    act("<k>$N<k> screams in pain as you leech the life from $S body!<1>",TRUE,ch,o,victim,TO_CHAR,NULL);
    act("$n deftly grabs you, and $s $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_VICT,NULL);
    act("<k>You scream in pain as $n<k> leeches the life from your body!<1>",TRUE,ch,o,victim,TO_VICT,NULL);
    act("$n deftly grabs $N, and $s $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_NOTVICT,NULL);
    act("<k>$N<k> screams in pain as $n<k> leeches the life from $S body!<1>",TRUE,ch,o,victim,TO_NOTVICT,NULL);       
    int dam = victim->GetMaxLevel();
    int rc = ch->reconcileDamage(victim, dam, DAMAGE_DRAIN);
    if (!ch->isUndead()) {
      dam = dam/5;
    }
    ch->setHit(min((int)(ch->getHit() + dam),(int)(ch->hitLimit())));
    if (rc == -1)
      delete victim;
    ch->addToWait(combatRound(3));
    ch->cantHit += ch->loseRound(2);
    return TRUE;
  }
  if ((body = dynamic_cast<TBaseCorpse *>(corpse))) {
    if (corpse->getMaterial() == MAT_POWDER || body->getCorpseLevel() <= 0) {
      act("There is no life left in $N to leech!",TRUE,ch,o,body,TO_CHAR,NULL);
      return TRUE;
    }
    act("You place your hand over $N, and your $o begins to glow with <r>sickly light<1>.",TRUE,ch,o,body,TO_CHAR,NULL);
    act("<k>As you leech life, $N<k> visibly withers and begins to decompose rapidly.<1>",TRUE,ch,o,body,TO_CHAR,NULL);
    
    act("$n places $s hand over $N, and $s $o begins to glow with <r>sickly light<1>.",TRUE,ch,o,body,TO_ROOM,NULL);
    act("<k>As $e leechs life, $N<k> visibly withers and begins to decompose rapidly<1>.",TRUE,ch,o,body,TO_ROOM,NULL);
    
    body->obj_flags.decay_time = 0;
    ch->setHit(min((int)(ch->getHit() + body->getCorpseLevel()),(int)(ch->hitLimit())));
    body->setCorpseLevel(0);
    
    ch->addToWait(combatRound(3));
    ch->cantHit += ch->loseRound(2);
    
    return TRUE;
  }
  return FALSE;
}

int telekinesisGlove(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  char target[30];
  TBeing *vict = NULL;
  TBeing *vict2 = NULL;
  TObj *obj = NULL;

  int dam, rc;

  if (cmd != CMD_POINT)
    return FALSE;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  arg = one_argument(arg, target, cElements(target));
  int bits = generic_find(target, FIND_CHAR_ROOM | FIND_OBJ_ROOM, ch, &vict, &obj);
  if(!bits)
    return FALSE;
  if(vict == ch) {
    return FALSE;
  }

  if (ch->checkPeaceful("Somehow, you think pointing that thing around here wouldn't go over well.")) 
    return FALSE;

  vict2 = ch->fight();

  if(vict && (!vict2 || vict2 == vict)) {
    // throw somebody
    act("You point at $N, and your glove begins to <G>glow<1>.",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<g>As you raise your arm, <1>$N<g> flails wildly and is lifted into the air.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<R>You spread your fingers wide, and $N is thrown backwards with incredible force!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);

    act("$n points at $N, and $s glove begins to <G>glow<1>.",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<g>As $n raises $s arm, <1>$N<g> flails wildly and is lifted into the air.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<R>$n spreads $s fingers wide, and $N is thrown backwards with incredible force!<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);

    act("$n points at you, and $s glove begins to <G>glow<1>.",TRUE,ch,o,vict,TO_VICT,NULL);
    act("<g>As $n raises $s arm, you flail wildly and are lifted into the air.<G>  Uh oh!<1>",TRUE,ch,o,vict,TO_VICT,NULL);
    act("<R>$n spreads $s fingers wide, and you are thrown backwards with incredible force!<1>",TRUE,ch,o,vict,TO_VICT,NULL);

    
    if (vict->riding) {
      int rc = vict->fallOffMount(vict->riding, POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
	return DELETE_VICT;
    }
    vict->setPosition(POSITION_SITTING);

    ch->addToWait(combatRound(6));
    ch->cantHit += ch->loseRound(3);
    vict->addToWait(combatRound(2));
    vict->cantHit += ch->loseRound(2);

    dam = ::number(10,40);
    rc = ch->applyDamage(vict, dam, SKILL_KINETIC_WAVE);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      vict->reformGroup();
      delete vict;
      vict = NULL;
    }

    return TRUE;
  } else if (vict && vict2 && vict2 != vict) {
    // throw vict into vict2
    act("You point at $N, and your glove begins to <G>glow<1>.",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<g>As <1>$N<g> is lifted into the air you gesture vaugely in $p's direction.<1>",TRUE,ch,vict2,vict,TO_CHAR,NULL);
    act("<R>You spread your fingers wide, and $N is thrown into $p with incredible force!<1>",TRUE,ch,vict2,vict,TO_CHAR,NULL);

    act("$n points at $N, and $s glove begins to <G>glow<1>.",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<g>As <1>$N<g> is lifted into the air $n gestures vaugely in $p's direction.<1>",TRUE,ch,vict2,vict,TO_NOTVICT,NULL);
    act("<R>$n spreads $s fingers wide, and $N is thrown into $p with incredible force!<1>",TRUE,ch,vict2,vict,TO_NOTVICT,NULL);

    act("$n points at you, and $s glove begins to <G>glow<1>.",TRUE,ch,o,vict,TO_VICT,NULL);
    act("<g>As you are lifted into the air $n gesture vaugely in $p's direction.<1>",TRUE,ch,vict2,vict,TO_VICT,NULL);
    act("<R>$n spreads $s fingers wide, and you are thrown into $p with incredible force!<1>",TRUE,ch,vict2,vict,TO_VICT,NULL);

    if (vict2->riding) {
      int rc = vict2->fallOffMount(vict2->riding, POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
    }
    vict2->setPosition(POSITION_SITTING);

    ch->addToWait(combatRound(6));
    ch->cantHit += ch->loseRound(2);
    vict->addToWait(combatRound(2));
    vict->cantHit += ch->loseRound(2);
    vict2->addToWait(combatRound(2));
    vict2->cantHit += ch->loseRound(2);


    dam = ::number(10,30);
    rc = ch->applyDamage(vict, dam, SKILL_KINETIC_WAVE);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      vict->reformGroup();
      delete vict;
      vict = NULL;
    }
    rc = ch->applyDamage(vict2, dam, SKILL_KINETIC_WAVE);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      vict2->reformGroup();
      delete vict2;
      vict2 = NULL;
    }


    return TRUE;
  } 
  return FALSE;
}


int manaBurnRobe(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *) {
#if 0
  
    TBeing *ch;

    if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
      return FALSE;

    if (cmd == CMD_SAY || cmd == CMD_SAY2) {
      sstring buf=sstring(arg).word(0);

      if(buf=="manifest"){
	double currentMana = ch-> getMana();
	double percentBurn = ch->hitLimit() * .2;
	double healthSteal = min((ch->hitLimit()- percentBurn),(ch->getHit() - percentBurn));
	double manaGain = healthSteal * .8;
	//	ch->setManaLimit(currentMana + manaGain);
	ch->setHit(healthSteal);
	act ("Your robe begins to glow with an eerie <b>light<1>, thin tendrils of light thrash wildly and then burrow into your skin, you scream as they rip the lifeforce from you.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);
	act ("&n screams in agony as thin <b>tendrils emerge from his robe and burrow into his skin!",TRUE,ch,NULL,NULL,TO_ROOM,NULL);
	return TRUE;
      }
    }

#endif
    return FALSE;

  } // end manaBurnRobe

int healingNeckwear(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  TBeing *tmp;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (!(tmp = dynamic_cast<TBeing *>(me->equippedBy)))
    return FALSE;

  // if they areholding it, don't be silly
  if (me->eq_pos != WEAR_NECK)
    return FALSE;

  if (!tmp->roomp)
    return FALSE;

  if (!tmp->isPc())
    return FALSE;

  act("$p constricts about your throat!", 0, tmp, me, 0, TO_CHAR);
  act("$p constricts about $n's throat!", 0, tmp, me, 0, TO_ROOM);
  if (tmp->hasClass(CLASS_SHAMAN)) {
    tmp->addToHit(::number(1, (tmp->getHit() / 3)));
    act("The power of the loa enters your body through $p!", 0, tmp, me, 0, TO_CHAR);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears from $n's throat!", 0, tmp, me, 0, TO_ROOM);
    delete me;
  } else {
    int rc;
    rc = tmp->applyDamage(tmp, ::number(1,10), DAMAGE_SUFFOCATION);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears from $n's throat!", 0, tmp, me, 0, TO_ROOM);
    delete me;
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete tmp;
      tmp = NULL;
    }
  }
  return TRUE;
}

int moveRestoreNeckwear(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  TBeing *tmp;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (!(tmp = dynamic_cast<TBeing *>(me->equippedBy)))
    return FALSE;

  // if they areholding it, don't be silly
  if (me->eq_pos != WEAR_NECK)
    return FALSE;

  if (!tmp->roomp)
    return FALSE;

  if (!tmp->isPc())
    return FALSE;

  act("$p constricts about your throat!", 0, tmp, me, 0, TO_CHAR);
  act("$p constricts about $n's throat!", 0, tmp, me, 0, TO_ROOM);
  if (tmp->hasClass(CLASS_SHAMAN)) {
    tmp->addToMove(::number(20, 75));
    act("The power of the loa enters your body through $p!", 0, tmp, me, 0, TO_CHAR);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("$p's power refreshes you!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears from $n's throat!", 0, tmp, me, 0, TO_ROOM);
    delete me;
  } else {
    int rc;
    rc = tmp->applyDamage(tmp, ::number(1,10), DAMAGE_SUFFOCATION);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears from $n's throat!", 0, tmp, me, 0, TO_ROOM);
    delete me;
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete tmp;
      tmp = NULL;
    }
  }
  return TRUE;
}

int blessingHoldItem(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  TBeing *tmp;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (!(tmp = dynamic_cast<TBeing *>(me->equippedBy)))
    return FALSE;

  if (!tmp->roomp)
    return FALSE;

  if (!tmp->isPc())
    return FALSE;

  if (tmp->hasClass(CLASS_SHAMAN)) {
    genericBless(tmp, tmp, 5, false);
    act("The power of the loa blesses you through $p!", 0, tmp, me, 0, TO_CHAR);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears!", 0, tmp, me, 0, TO_ROOM);
    delete me;
  } else {
    int rc;
    rc = tmp->applyDamage(tmp, ::number(1,10), DAMAGE_DRAIN);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears! OW!", 0, tmp, me, 0, TO_ROOM);
    delete me;
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete tmp;
      tmp = NULL;
    }
  }
  return TRUE;
}

int chippedTooth(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  int rc;

  if (cmd != CMD_EAT)
    return FALSE;

  if (!ch->isPc())
    return FALSE;

  rc = ch->applyDamage(ch, ::number(1,5), DAMAGE_NORMAL);
  act("OWWIE!! You feel like you've chipped a tooth on $p!", 0, ch, o, 0, TO_CHAR);
  act("$n cringes in pain as $e bites into $p.", 0, ch, o, 0, TO_ROOM);
  delete o;
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete ch;
    ch = NULL;
  }
  return TRUE;

}

int sunCircleAmulet(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    sstring buf;
    buf=sstring(arg).word(0);

    if(buf=="whullalo"){
      TObj *portal;
      act("You grasp $p and utter the word '<p>whullalo<1>'.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("$n grasps $p and utters the word '<p>whullalo<1>'.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      if(ch->inRoom() != 30770) {
               act("Nothing seems to happen.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        act("Nothing seems to happen.",TRUE,ch,o,NULL,TO_ROOM,NULL);
        return TRUE;
      } else if (!(portal = read_object(30750, VIRTUAL))) {
        act("Problem in Sun Circle Amulet, tell a god you saw this.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        act("Nothing seems to happen.",TRUE,ch,o,NULL,TO_ROOM,NULL);
        vlogf(LOG_PROC, "Unable to load portal for sunCircleAmulet() proc. DASH!!");
        return TRUE;
      }
      act("The runes on the center stone flare in respone to the <Y>$o's power<1>.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("A beam of <c>energy<1> erupts from the center stone, ripping a hole in the fabric of reality!",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("It seems $n has caused a <W>portal<1> to another realm to open here.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("The runes on the center stone flare in respone to the <Y>$o's power<1>.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("A beam of <c>energy<1> erupts from the center stone, ripping a hole in the fabric of reality!",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("It seems you have caused a <W>portal<1> to another realm to open here.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      *ch->roomp += *portal;
      
      return TRUE;
    }
  }
  return FALSE;
}


int minecart(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *myself, TObj *)
{
  int where = 0, doswitch = 0, dontswitch = 0, status = 0;
  int nextroom = 0, i, dam = 0; //num_in_cart = 0, MAX_IN_CART = 5;
  TThing *in_cart, *next_in_cart;
  TBeing *beingic;  
  char arg1[30], arg2[30], arg3[30];
  char buf[256];
  TObj *switchtrack = NULL, *o = myself;
  class minecart_struct {
  public:
    bool handbrakeOn;
    int speed;
    int timer;

    minecart_struct() :
      handbrakeOn(true),
      speed(0),
      timer(-1)
    {
    }
    ~minecart_struct()
    {
    }
  };

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<minecart_struct *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  } else if (cmd == CMD_GENERIC_CREATED) {
    myself->act_ptr = new minecart_struct();
    return FALSE;
  }
  minecart_struct *job;
  if (!(job = static_cast<minecart_struct *>(myself->act_ptr))) {
    vlogf(LOG_PROC, "Minecart lost its memory. DASH!!");
    return FALSE;
  }
  if (cmd == CMD_STAND) {
    if (ch->riding == myself && job->speed >= 2) {
      ch->sendTo("You're moving much too fast to get off now!\n\r");
      return TRUE;
    } else
      return FALSE;
  }
  if (cmd == CMD_SIT) {
    arg = one_argument(arg, arg1, cElements(arg1));
    arg = one_argument(arg, arg2, cElements(arg2));
    arg = one_argument(arg, arg3, cElements(arg3));
    if ((is_abbrev(arg1, "minecart") || is_abbrev(arg1, "cart")) && job->speed >=2 ) {
      ch->sendTo("The mine cart is moving much too fast to get on now!\n\r");
      return TRUE;
    } else
      return FALSE;
  }
  if (cmd == CMD_PUSH || cmd == CMD_PULL || cmd == CMD_OPERATE || cmd == CMD_USE) {
    arg = one_argument(arg, arg1, cElements(arg1));
    arg = one_argument(arg, arg2, cElements(arg2));
    arg = one_argument(arg, arg3, cElements(arg3));
    if (is_abbrev(arg1, "handbrake") || is_abbrev(arg1, "brake")) {
      if (ch->riding != myself) {
        act("You must be sitting on $p to operate the handbrake.", TRUE, ch, o, NULL, TO_CHAR);
        return TRUE;
      } else {
        if (job->handbrakeOn) {
          job->handbrakeOn = FALSE;
          act("You release the handbrake on $p.", TRUE, ch, o, NULL, TO_CHAR);
          act("$n releases the handbrake on $p.", TRUE, ch, o, NULL, TO_ROOM);
          return TRUE;
        } else {
          job->handbrakeOn = TRUE;
          act("You engage the handbrake on $p.", TRUE, ch, o, NULL, TO_CHAR);
          act("$n engages the handbrake on $p.", TRUE, ch, o, NULL, TO_ROOM);
          return TRUE;
        }
      }
    } else if ((is_abbrev(arg1, "cart") || is_abbrev(arg1, "minecart")) && cmd == CMD_PUSH) {
      if (ch->riding == myself) {
        act("How do you intend to push $p while sitting on it?", TRUE, ch, o, NULL, TO_CHAR);
        return TRUE;
      } else if (job->handbrakeOn) {
        act("You push and push, but can't seem to move $p.", TRUE, ch, o, NULL, TO_CHAR);
        act("Oh hey, look at that.  The handbrake is still engaged, doofus.", TRUE, ch, o, NULL, TO_CHAR);
        act("$n strains with all $s might, but fails to budge $p.", TRUE, ch, o, NULL, TO_ROOM);
        act("What a loser, the handbrake is still engaged.", TRUE, ch, o, NULL, TO_ROOM);
        return TRUE;
      } else {
        act("You give $p a mighty shove, and it starts to roll slowly down the tracks.", TRUE, ch, o, NULL, TO_CHAR);
        act("$n gives $p a mighty shove, and it starts to roll slowly down the tracks.", TRUE, ch, o, NULL, TO_ROOM);
        job->speed = 1;
        job->timer = 10;
        return TRUE;
      }
    } else
      return FALSE;
  } else if (cmd == CMD_GENERIC_QUICK_PULSE && job->speed > 0) {
    where = myself->in_room;
    if (job->timer >= job->speed) {
      job->timer--;
      if (job->timer == 10 - ((10 - job->speed)/2 )) {
        if (where == 18007 || where == 18011 || where == 18020) {
          act("$n <W>rattles as it passes over the <k>switchtracks<1>.",FALSE, myself, 0, 0, TO_ROOM);
        }
      }
      if (job->timer != 0) return FALSE;
    }
    if (where < 18000 || where > 18059) {
      vlogf(LOG_PROC, "Minecart got lost. Dash will NOT be pleased.");
      return FALSE;
    } else {
      switch(where) {
      case 18007:
        status = 1;
        doswitch = 18016;
        dontswitch = 18008;
        break;
      case 18011:
        status = 1;
	doswitch = 18014;
	dontswitch = 18012;
	break;
      case 18020:
	status = 1;
	doswitch = 18033;
	dontswitch = 18021;
	break;
      case 18013:
      case 18015:
      case 18032:
      case 18059:
	status = 2;
	// code for EotL
	break;
      case 18014:
	status = 3;
	// code for breaking through the wall
	break;
      default:
	status = 0;
	break;
      }
      if (status == 1) {
	if (!(switchtrack =dynamic_cast<TObj *>( searchLinkedList("switchtrack", myself->roomp->stuff, TYPEOBJ)))) {
	  vlogf(LOG_PROC, "Minecart looking for switchtrack that wasn't there. Dash sucks.");
	  return FALSE;
	} else {
	  if (isname("switchtrackdoswitch", switchtrack->getName()))
	    nextroom = doswitch;
	  else if (isname("switchtrackdontswitch", switchtrack->getName()))
	    nextroom = dontswitch;
	  else {
	    vlogf(LOG_PROC, "Minecart found an indecisive switchtrack. Dash sucks.");
	    return FALSE;
	  }
	}
      } else if (status == 2) {

	if (job->speed > 8) {
	  sprintf(buf, "There is a resounding metallic *CLANG* as $n collides with the end of the track at top speed.");
	  act(buf,FALSE, myself, 0, 0, TO_ROOM);
	} else if (job->speed > 5) {
          sprintf(buf, "There is a metallic *CLANG* as $n hits the end of the track at high speed.");
          act(buf,FALSE, myself, 0, 0, TO_ROOM);
	} else if (job->speed > 2) {
          sprintf(buf, "There is a soft metallic *CLANG* as $n hits the end of the track.");
          act(buf,FALSE, myself, 0, 0, TO_ROOM);
	} else if (job->speed > 0) {
          sprintf(buf, "There is a soft metallic *ping* as $n lightly taps the end of the track.");
          act(buf,FALSE, myself, 0, 0, TO_ROOM);
	}
	if (myself->rider) {
	  for (in_cart = myself->rider; in_cart; in_cart = next_in_cart) {
	    next_in_cart = in_cart->nextRider;
	    if (::number(1,12) < job->speed) {
	      sprintf(buf, "<r>$n<1><r> loses $s balance and flips forward over the rim of $p<1><r>.  Ouch.<1>");
	      act(buf,FALSE, in_cart, myself, 0, TO_ROOM);
	      sprintf(buf, "<r>You lose your balance and flip forward over the rim of $p<1><r>.  Ouch.<1>");
              act(buf,FALSE, in_cart, myself, 0, TO_CHAR);
	      dam = job->speed * 2;
	      
	      if ((beingic = dynamic_cast<TBeing *>(in_cart))) {
		beingic->dismount(POSITION_SITTING);
		beingic->reconcileDamage(beingic, min(dam, beingic->getHit()+2), DAMAGE_COLLISION);
	      }
	    }
	 
	  }
	}

	job->speed = 0;
	job->timer = -1;
	// do some stop message
	return FALSE;
      } else if (status == 3) {
	roomDirData *exitp;
	dirTypeT dir;
	for(i=MIN_DIR;i<MAX_DIR;++i){
	  if(myself->roomp->dir_option[i] &&
	     myself->roomp->dir_option[i]->to_room==where+1){
	    break;
	  }
	}
	dir = mapFileToDir(i);
	if (!(exitp = myself->roomp->exitDir(dir))) {
	  vlogf(LOG_PROC, "bad exit for minecart smash-wall code, bug Dash.");
	  return FALSE;
	}
        if ((IS_SET(exitp->condition, EX_DESTROYED)) ||
            !IS_SET(exitp->condition, EX_CLOSED)) {

        } else {	
	  sprintf(buf, "$n slams into the wall to the east, and it collapses in a shower of rocks!");
	  act(buf, FALSE, myself, 0, 0, TO_ROOM);
	  exitp->destroyDoor(dir, where);
	  --(*myself);
	  *real_roomp(nextroom) += *myself;
	  sprintf(buf, "The wall to the west suddenly explodes inwards in a shower of rocks!");
          act(buf, FALSE, myself, 0, 0, TO_ROOM);
          --(*myself);
          *real_roomp(where) += *myself; 

	}
	nextroom = where+1;

      } else {
	nextroom = where+1;
      }
      
      for(i=MIN_DIR;i<MAX_DIR;++i){
	if(myself->roomp->dir_option[i] &&
	   myself->roomp->dir_option[i]->to_room==nextroom){
	  break;
	}
      }
      if (job->speed > 8) {
        sprintf(buf, "$n goes barreling %s down the tracks of the mining %s at breakneck speed.",
                (i==MAX_DIR)?"on":dirs[i], (where > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 5) {
        sprintf(buf, "$n rolls %s down the tracks of the mining %s at an impressive speed.",
                (i==MAX_DIR)?"on":dirs[i], (where > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 2) {
        sprintf(buf, "$n rolls %s down the tracks of the mining %s at a steady rate.",
                (i==MAX_DIR)?"on":dirs[i], (where > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 0) {
        sprintf(buf, "$n inches its way %s down the tracks of the mining %s.",
                (i==MAX_DIR)?"on":dirs[i], (where > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      }
      

      //move cart
      --(*myself);
      *real_roomp(nextroom)+=*myself;       
      
#if 0
      //move people in the cart
      if (myself->rider) {
        for (in_cart = myself->rider; in_cart; in_cart = next_in_cart) {
          next_in_cart = in_cart->nextRider;

          --(*in_cart);
          *real_roomp(nextroom)+=*in_cart;
          if(dynamic_cast<TBeing *>(in_cart))
            dynamic_cast<TBeing *>(in_cart)->doLook("",CMD_LOOK);
        }
      }
#endif //moved this later


      if (job->speed > 8) {
        sprintf(buf, "$n comes crashing down the tracks of the %s, barreling down at an incredible speed.",   
                (nextroom > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 5) {
        sprintf(buf, "$n comes rolling down the tracks of the %s at an impressive speed.",
                (nextroom > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 2) {
        sprintf(buf, "$n comes rolling down the tracks of the %s at a steady speed.",
                (nextroom > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 0) {
        sprintf(buf, "$n inches its way down the tracks of the %s at a steady speed.",
                (nextroom > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      }

      if (myself->rider) {
        for (in_cart = myself->rider; in_cart; in_cart = next_in_cart) {
          next_in_cart = in_cart->nextRider;
	  sprintf(buf, "...$n hangs on for dear life as $e rides $p %s.",
		  (i==MAX_DIR)?"down the tracks":dirs[i]);
          act(buf,FALSE, in_cart, myself, 0, TO_ROOM);
          sprintf(buf, "...you hang on for dear life as you ride $p %s.",
                  (i==MAX_DIR)?"down the tracks":dirs[i]);
	  act(buf,FALSE, in_cart, myself, 0, TO_CHAR);
          --(*in_cart);
          *real_roomp(nextroom)+=*in_cart;
          sprintf(buf, "...$n careens down the tracks, holding onto the $p for dear life.");
          act(buf,FALSE, in_cart, myself, 0, TO_ROOM);
          if(dynamic_cast<TBeing *>(in_cart))
            dynamic_cast<TBeing *>(in_cart)->doLook("",CMD_LOOK);
        }
      }



      if(job->handbrakeOn && job->speed > 0) {
	act("Sparks fly from $n's wheels as it slows down.",FALSE, myself, 0, 0, TO_ROOM);
	job->speed = job->speed - 2;
	if (job->speed <= 0) {
	  job->speed = 0;
	  act("The axles on $n creak a few times as it comes to a complete stop.",FALSE, myself, 0, 0, TO_ROOM);
	}
      }
      else if (job->speed < 10) job->speed++;
      job->timer = 10;
      // code for next room shit
    }
    
  } 
  return FALSE;
}


// DASH MARKER

int switchtrack(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *myself, TObj *)
{
#if 0
  if (cmd != CMD_PUSH && 
      cmd != CMD_PULL && 
      cmd != CMD_OPERATE && 
      cmd != CMD_USE && 
      cmd != CMD_TURN) 
    return FALSE;
  
  if (!myself->getName())
    return FALSE;
      
  int where = myself->in_room;
  char arg1[30], arg2[30], buf[256];
  arg = one_argument(arg, arg1);
  arg = one_argument(arg, arg2);

  if (is_abbrev(arg1, "switchtracks") || is_abbrev(arg1, "tracks")) {
    switch (where) { 
    case 18007:
      if(!arg2)
	strcpy(arg2,isname("switchtrackdoswitch", myself->getName())?"southwest":"south");
      else if(is_abbrev(arg2, "south") || is_abbrev(arg2, "s")) {
	strcpy(arg2,"southern");
	if(isname("switchtrackdoswitch", myself->getName())) {
	  ch->sendTo(format("The switchtrack is already aligned with the %s fork.") % arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdoswitch");
      }
      else if(is_abbrev(arg2, "southwest") || is_abbrev(arg2, "se")) {
	strcpy(arg2,"southwestern");
	if(isname("switchtrackdontswitch", myself->getName())) {
	  ch->sendTo(format("The switchtrack is already aligned with the %s fork.") % arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdontswitch");
      } else {
	ch->sendTo("This switchtrack can only be moved to the south or southwest.");
	return TRUE;
      }
      
      break;
    case 18011:
      if(!arg2) 
	strcpy(arg2,isname("switchtrackdoswitch", myself->getName())?"south":"east");
      else if(is_abbrev(arg2, "east") || is_abbrev(arg2, "e")) {
	strcpy(arg2,"eastern");
	if(isname("switchtrackdoswitch", myself->getName())) {
	  ch->sendTo(format("The switchtrack is already aligned with the %s fork.") % arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdoswitch");
      }
      else if(is_abbrev(arg2, "south") || is_abbrev(arg2, "s")) {
	strcpy(arg2,"southern");
	if (isname("switchtrackdontswitch", myself->getName())) {
	  ch->sendTo(format("The switchtrack is already aligned with the %s fork.") % arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdontswitch");  
      } else {
        ch->sendTo("This switchtrack can only be moved to the south or east.");
        return TRUE;
      }
      
      break;
    case 18020:
      if(!arg2) 
	strcpy(arg2,isname("switchtrackdoswitch", myself->getName())?"north":"east");
      else if(is_abbrev(arg2, "east") || is_abbrev(arg2, "e")) {
	strcpy(arg2,"eastern");
	if(isname("switchtrackdoswitch", myself->getName())) {
	  ch->sendTo(format("The switchtrack is already aligned with the %s fork.") % arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdoswitch");
      }
      else if(is_abbrev(arg2, "north") || is_abbrev(arg2, "n")) {
	strcpy(arg2,"northern");
	if(isname("switchtrackdontswitch", myself->getName())) {
	  ch->sendTo(format("The switchtrack is already aligned with the %s fork.") % arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdontswitch");
      } else {
        ch->sendTo("This switchtrack can only be moved to the north or east.");
        return TRUE;
      }
      
      break;
    default:
      ch->sendTo("Uh. This switchtrack shouldn't be here. Tell a god or something?");
      //      vlogf(LOG_PROC, format("%s tried to operate a switchtrack (%d) in room with no switchtrack code (%d)") % 
      //	    ch->getName() % myself->objVnum % where);
    }
    sprintf(buf,"<k>You force the $o into alignment with the %s tunnel.<1>",arg2);
    act(buf, TRUE, ch, myself, NULL, TO_CHAR);
    sprintf(buf,"<k>$n forces the $o into alignment with the %s tunnel.<1>",arg2);
    act(buf, TRUE, ch, myself, NULL, TO_ROOM);
    sprintf(buf,"<k>The switchtracks here are aligned with the %s tunnel.<1>", arg2);
    myself->setDescr(mud_str_dup(buf));
    return TRUE;
  }
#endif
  return FALSE;
}




int travelGear(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  // this is a custom proc for the Wanderlust/Cloak of the Traveler combo - Dash
  // i really like this proc :) 
  // ok here is what it does - the procs only work when the player has BOTH objects - the hammer and the cloak
  // the hammer restores moves fairly rapidly, when ever the player is low
  // the cloak projects a force shield around the wearer whenever it is hit, shield lasts for 2 rounds then goes away again

  TBeing *ch;
  TObj *cloak;
  TObj *hammer = NULL;
  affectedData aff;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (obj_index[o->getItemIndex()].virt == 9583) {
    
    if (cmd != CMD_GENERIC_QUICK_PULSE)
      return FALSE;
    
    if(!(cloak = dynamic_cast<TObj *>(ch->equipment[WEAR_BACK]))) 
      return FALSE;
    if (obj_index[cloak->getItemIndex()].virt != 9582)
      return FALSE;
    
    // ok... so he's wielding the hammer, wearing the cloak....
    
    if (!::number(0,1) && (ch->getMaxMove() > 4*ch->getMove())) {
      ch->addToMove(ch->getMaxMove()/(::number(2,5)));
      act("<k>$p<Y> glows softly<1>, and you feel renewed strength flow into your legs.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
      act("<k>$p<Y> glows softly<1>, and $n seems to look more refreshed.<1>",TRUE,ch,o,vict,TO_ROOM,NULL);
      return TRUE;
    }
    return FALSE;
  } else if (obj_index[o->getItemIndex()].virt == 9582) {
    if (cmd != CMD_OBJ_BEEN_HIT)
      return FALSE;
    if (!((hammer = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT])) && obj_index[hammer->getItemIndex()].virt == 9583)
	||((hammer = dynamic_cast<TObj *>(ch->equipment[HOLD_LEFT])) && obj_index[hammer->getItemIndex()].virt == 9583))
      // no hammer
      return FALSE;
    if (ch->affectedBySpell(SPELL_SORCERERS_GLOBE))
      return FALSE;
    
    
    act("<1>$p<Y> glows brightly<1> as it is struck!",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<1>$p<Y> glows brightly<1> as it is struck!",TRUE,ch,o,vict,TO_ROOM,NULL);
    act("Your $o emits an audible hum and suddenly <W>a shield of force<1> slams into being around you!",
	TRUE,ch,o,vict,TO_CHAR,NULL);
    act("$n's $o emits an audible hum and suddenly <W>a shield of force<1> slams into being around $m!",
	TRUE,ch,o,vict,TO_ROOM,NULL);

    aff.type = SPELL_SORCERERS_GLOBE;
    aff.level = 37;
    aff.duration = ONE_SECOND * 2;
    aff.location = APPLY_ARMOR;
    aff.modifier = -100;
    aff.bitvector = 0;
    ch->affectTo(&aff, -1);
    return TRUE;
  }
  return FALSE;
}





int selfRepairing(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if(::number(0,9))
    return false;

  if (cmd == CMD_GENERIC_PULSE && o->getStructPoints() < o->getMaxStructPoints()) {
    if(::number(1,100) < (int)(100.0*((float)(o->getStructPoints()) / (float)(o->getMaxStructPoints()))))
      return FALSE;

    act("<W>$n<W>'s $o slowly reconstructs itself, erasing signs of damage.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("<W>Your $o slowly reconstructs itself, erasing signs of damage.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);

    o->addToStructPoints(::number(1,min(5, o->getMaxStructPoints() - o->getStructPoints())));
    return FALSE;
  }
  return FALSE;
}

int USPortal(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *) {
  
  if(cmd == CMD_ENTER || cmd == CMD_LEAVE) {
    if (!ch->isUndead()) {
      act("<k>There is a sharp crackle of negative energy as $n tries to go through the portal.<1>"
	  ,TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<k>There is a sharp crackle of negative energy as you try to go through the portal.<1>"
	  ,TRUE,ch,o,NULL,TO_CHAR,NULL);
      
      act("<k>Suddenly, $n is thrown backwards from the portal with tremendous force!<1>"
	  ,TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<k>Suddenly, you are thrown backwards from the portal with tremendous force!<1>"
	  ,TRUE,ch,o,NULL,TO_CHAR,NULL);
      
      int rc, dam;
      dam = ::number(10,30);
      ch->setPosition(POSITION_SITTING);
      rc = ch->reconcileDamage(ch, dam, DAMAGE_NORMAL);
      if (rc == -1)
	return DELETE_VICT;
      
      return TRUE;
    } else { // ch is undead
      act("<k>A chill radiates from $n as $e enters the portal.<1>"
          ,TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<k>A terrible chill runs through you as you enter the portal.<1>"
          ,TRUE,ch,o,NULL,TO_CHAR,NULL);
      return FALSE;
    }
  }
  return FALSE;
}
    


int AKAmulet(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *) {
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (cmd == CMD_GENERIC_PULSE && !::number(0,3)) {
    if(ch->isUndead() && ch->hitLimit() > ch->getHit()) {
       int dam =::number(1,5);
       act("$n regenerates slightly.",TRUE,ch,o,NULL,TO_ROOM,NULL);
       act("You regenerate slightly.",TRUE,ch,o,NULL,TO_CHAR,NULL);
       ch->addToHit(dam);
       return FALSE;
    } else if (!ch->isUndead()) {
      int dam = ::number(25,100);
      act("$n screams in pain as $p drains the life from $s body!",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("You scream in pain as $p drains the life from your body!",TRUE,ch,o,NULL,TO_CHAR,NULL);
      int rc = ch->reconcileDamage(ch, dam, DAMAGE_DRAIN);
      if (rc == -1)
        return DELETE_THIS;
      return FALSE;
    }
  }
  return FALSE;
  
}


int suffGlove(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{


  TBeing *ch;
  int rc, dam = 1;

  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (::number(0,40))
    return FALSE;
  if (cmd != CMD_OBJ_HIT)
    return FALSE;


  if (vict->affectedBySpell(SPELL_SUFFOCATE))
    return FALSE;

  affectedData aff;
  aff.type = AFFECT_DISEASE;
  aff.level = ch->GetMaxLevel();
  aff.duration = combatRound(3);  
  aff.modifier = DISEASE_SUFFOCATE;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_SILENT;



  ch->addToWait(combatRound(3));
  ch->cantHit += ch->loseRound(3);
  vict->addToWait(combatRound(3));
  vict->cantHit += ch->loseRound(3);


  act("Your $o seems control your movements as you reach for $N!<1>",
      TRUE,ch,o,vict,TO_CHAR,NULL);
  act("<o>$p <o>covers $N<o>'s nose and mouth, preventing $M from breathing!<1>",
      TRUE,ch,o,vict,TO_CHAR,NULL);
  act("$n's $o seems control $s movements as $e reaches for $N!<1>",
      TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("<o>$p <o>covers $N<o>'s nose and mouth, preventing $M from breathing!<1>",
      TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("n$'s $o seems control $s movements as $e reaches for you!<1>",
      TRUE,ch,o,vict,TO_VICT,NULL);
  act("<o>$p <o>covers your nose and mouth.  PANIC!  You can't breathe!!!<1>",
      TRUE,ch,o,vict,TO_VICT,NULL);


  dam = ::number(1,10);
  rc = ch->applyDamage(vict, dam, DAMAGE_SUFFOCATION);
  vict->affectJoin(vict, &aff, AVG_DUR_NO, AVG_EFF_YES);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;


}



int totemMask(TBeing *v, cmdTypeT cmd, const char *, TObj *o, TObj *weapon)
{
  TBeing *ch;
  int rc, dam, chance, result;
  wearSlotT t;

  if(cmd != CMD_OBJ_BEEN_HIT || !v || !o)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;     

  t=((!weapon || (weapon->eq_pos==HOLD_RIGHT))?WEAR_HAND_R:WEAR_HAND_L);
  chance = ::number(0, 99);
  result = ::number(5, 16);

  if (chance >= 65) {
    if (chance >=80)
      return FALSE;
    if (o->getStructPoints() < o->getMaxStructPoints()) {
      if(::number(1,100) < (int)(100.0*((float)(o->getStructPoints()) / (float)(o->getMaxStructPoints()))))
        return FALSE;
      act("<k>Worms crawl from behind <1>$n<k>'s <1>$p<k> and then liquify filling in the damaged places.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<k>Worms crawl from behind <1>$o<k> and then liquify, filling in the damaged places.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
      o->addToStructPoints(::number(1,min(5, o->getMaxStructPoints() - o->getStructPoints())));
      return FALSE;
    }
    return FALSE;
  } else {
    act("<r>The eyes of $o <r>glow blood red as life force is channeled from your body.<1>"
	, 0, v, o, 0, TO_CHAR);
    act("<r>The eyes of $p <r>glow blood red.<1>"
	, 0, v, o, 0, TO_ROOM);
    ch->addToLifeforce(result);    
    dam = ::number(3, 15);
    
    rc = ch->reconcileDamage(v, dam, DAMAGE_DRAIN);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }
  return TRUE;

}


int permaDeathMonument(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o1, TObj *o2)
{
  int found=0;
  TThing *o=NULL;
  TObj *to;

  if(cmd != CMD_LOOK)
    return FALSE;

  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (o=*it);++it) {
    to = dynamic_cast<TObj *>(o);
    if (to && to->spec == SPEC_PERMA_DEATH &&
	isname(arg, to->name)){
      found=1;
      break;
    }
  }

  if(!found)
    return FALSE;

  TDatabase db_dead(DB_SNEEZY);
  TDatabase db_living(DB_SNEEZY);

  ch->sendTo("You examine the plaque:\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");
  ch->sendTo("-     This monument commemorates the bravest of heroes     -\n\r");
  ch->sendTo("-                 who risk permanent death.                -\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");
  ch->sendTo("Living:             Dead:\n\r");

  db_dead.query("select name, level, died, killer from permadeath where died=1 order by level desc limit 25");
  db_living.query("select name, level, died, killer from permadeath where died=0 order by level desc limit 25");

  int i=1;
  while(i<=25){
    if(db_living.fetchRow()){
      ch->sendTo(COLOR_BASIC, format("%2s| %-13s | ") %
		 db_living["level"] % db_living["name"]);
    } else {
      ch->sendTo(COLOR_BASIC, format(" 0) %-13s | ") % "no one");
    }

    if(db_dead.fetchRow()){
      ch->sendTo(COLOR_BASIC, format("%2s| %-13s killed by %s.\n\r")%
		 db_dead["level"] % db_dead["name"] % db_dead["killer"]);
    } else {
      ch->sendTo(COLOR_BASIC, format(" 0) %-13s\n\r") % "no one");
    }

    ++i;
  }

  return TRUE;
}


int trophyBoard(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o1, TObj *o2)
{
  int found=0;
  TThing *o=NULL;
  TObj *to;

  if(cmd != CMD_LOOK)
    return FALSE;

  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (o=*it);++it) {
    to = dynamic_cast<TObj *>(o);
    if (to && to->spec == SPEC_TROPHY_BOARD &&
	isname(arg, to->name)){
      found=1;
      break;
    }
  }

  if(!found)
    return FALSE;

  TDatabase db(DB_SNEEZY);
  TDatabase db2(DB_SNEEZY);

  //  db.query("select name, count(*) from trophy group by name order by count(*) desc limit 10");
  //  db.query("select name, count from trophyplayer order by count desc limit 25");
  db.query("select p.name, t.count, t.player_id from player p, trophyplayer t where t.player_id=p.id order by count desc limit 25");
  db2.query("select p.name, t.total, t.player_id from player p, trophyplayer t where t.player_id=p.id and t.total is not null order by t.total desc limit 25");

  if(!db.isResults() || !db2.isResults()){
    ch->sendTo("The board is empty.\n\r");
    return TRUE;
  }

  ch->sendTo("You examine the board:\n\r");
  ch->sendTo("__________________________________________________________________\n\r");
  ch->sendTo("\n\r");
  ch->sendTo("-     According to the research of the shaman guildmaster,       -\n\r");
  ch->sendTo("-     these people have done a wider range of death bringing     -\n\r");
  ch->sendTo("-     than any other.                                            -\n\r");
  ch->sendTo("__________________________________________________________________\n\r");
  ch->sendTo("\n\r");
  ch->sendTo("-  Number of distinct           |  Total number of               -\n\r");
  ch->sendTo("-  life forms killed.           |  lives taken.                  -\n\r");
  ch->sendTo("__________________________________________________________________\n\r");
  ch->sendTo("\n\r");

  // set the mob count to the highest players kill count
  int activemobcount=1;
  if(db.fetchRow())
    activemobcount=convertTo<int>(db["count"]);

  float mostkilled=0.1;
  if(db2.fetchRow())
    mostkilled=convertTo<float>(db2["total"]);
    
  int i=1;
  bool found1=false, found2=false;
  bool is_me1=false, is_me2=false;
  do {
    is_me1=is_me2=false;
    if(ch->getPlayerID()==convertTo<int>(db["player_id"])){
      found1=true;
      is_me1=true;
    }
    if(ch->getPlayerID()==convertTo<int>(db2["player_id"])){
      found2=true;
      is_me2=true;
    }

    ch->sendTo(COLOR_BASIC, format("%2i) %s%-13s<1> - %i (%3d%c) | %2i) %s%-13s<1> - %i (%3d%c)\n\r") % 
	       i % 
	       (is_me1?"<r>":"") %
	       db["name"] % 
	       convertTo<int>(db["count"]) % 
	       (int)(((float)convertTo<int>(db["count"])/(float)activemobcount)*100) % '%' %
	       i % 
	       (is_me2?"<r>":"") %
	       db2["name"] % (int)(convertTo<float>(db2["total"])) %
	       (int)((convertTo<float>(db2["total"])/mostkilled)*100) % '%');
	       
    ++i;
  } while(db.fetchRow() && db2.fetchRow());


  if(!found1 || !found2){
    if(!found1){
      db.query("select p.name, t.count, t.player_id from player p, trophyplayer t where t.player_id=p.id and p.id=%i", ch->getPlayerID());
      if(db.fetchRow()){
	ch->sendTo(COLOR_BASIC, format("XX) <r>%-13s<1> - %i (%3d%c) ") % 
		   db["name"] % 
		   convertTo<int>(db["count"]) % 
		   (int)(((float)convertTo<int>(db["count"])/(float)activemobcount)*100) % '%');
      } else {
	ch->sendTo(COLOR_BASIC, "                                ");   
      }
    } else {
      ch->sendTo(COLOR_BASIC, "                                ");
    }

    if(!found2){
      db2.query("select p.name, t.total, t.player_id from player p, trophyplayer t where t.player_id=p.id and t.total is not null and p.id=%i", ch->getPlayerID());
      if(db2.fetchRow()){
	ch->sendTo(COLOR_BASIC, format("| XX) %s%-13s<1> - %i (%3d%c)\n\r") % 
		   (!found2?"<r>":"") %
		   db2["name"] % (int)(convertTo<float>(db2["total"])) %
		   (int)((convertTo<float>(db2["total"])/mostkilled)*100) % '%');
      } else {
	ch->sendTo(COLOR_BASIC, "\n\r");
      }
    } else {
      ch->sendTo(COLOR_BASIC, "\n\r");
    }
    
    
  }


  return TRUE;
}


int highrollersBoard(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o1, TObj *o2)
{
  int found=0;
  TThing *o=NULL;
  TObj *to;

  if(cmd != CMD_LOOK)
    return FALSE;

  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (o=*it);++it) {
    to = dynamic_cast<TObj *>(o);
    if (to && to->spec == SPEC_HIGHROLLERS_BOARD &&
	isname(arg, to->name)){
      found=1;
      break;
    }
  }

  if(!found)
    return FALSE;

  TDatabase db(DB_SNEEZY);

  db.query("select getPlayerName(player_id) as name, money from gamblers where money > 0 order by money desc limit 10");

  ch->sendTo("You examine the board:\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");
  ch->sendTo(" The high rollers and the big losers at the Grimhaven Casino\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");

  int i=1;
  while(db.fetchRow()){
    ch->sendTo(COLOR_BASIC, format("%i) %s has won %s talens!\n\r") % 
	       i % db["name"] % db["money"]);
    ++i;
  }

  db.query("select getPlayerName(player_id) as name, money from gamblers where money < 0 order by money limit 10");
  
  ch->sendTo(COLOR_BASIC, "\n\r");

  i=1;
  while(db.fetchRow()){
    ch->sendTo(COLOR_BASIC, format("%i) %s has lost %i talens.\n\r") % 
	       i % db["name"] % abs(convertTo<int>(db["money"])));
    ++i;
  }

  return TRUE;
}


int shopinfoBoard(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o1, TObj *o2)
{
  int found=0;
  TThing *o=NULL;
  TObj *to;

  if(cmd != CMD_LOOK)
    return FALSE;

  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (o=*it);++it) {
    to = dynamic_cast<TObj *>(o);
    if (to && to->spec == SPEC_SHOPINFO_BOARD &&
	isname(arg, to->name)){
      found=1;
      break;
    }
  }

  if(!found)
    return FALSE;

  ch->sendTo("You examine the board:\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");
  ch->sendTo("-  This financial information is provided free of charge   -\n\r");
  ch->sendTo("-  by the Grimhaven Bank and the King of Grimhaven.        -\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");

  
  TDatabase db(DB_SNEEZY);

  //////////////////////////////////////
  // number of shops and owned status
  //
  db.query("select count(*) as count from shop");

  if(!db.isResults()){
    ch->sendTo("The board is empty.\n\r");
    return TRUE;
  }
  db.fetchRow();
  int nshops=convertTo<int>(db["count"]);

  db.query("select count(distinct shop_nr) as count from shopowned");
  int nowned=0;
  if(db.fetchRow())
    nowned=convertTo<int>(db["count"]);
    
  ch->sendTo(format("There are %i shops, %i of which are privately owned.\n\r") %
	     nshops % nowned);

  ////////////////////////////
  // broke shops
  db.query("select count(*) as count from shopowned where gold<100000");

  if(db.fetchRow())
    ch->sendTo(format("%s shops have less than 100000 talens.\n\r") % db["count"]);


  /////////////////////////////
  // average talens
  db.query("select round(avg(gold)) as gold from shopowned");

  if(db.fetchRow())
    ch->sendTo(format("Average talens per shop is %s.\n\r") % db["gold"]);
  
  ////////////////////////////
  // top ten shops
  db.query("select s.in_room, so.gold from shop s, shopowned so where s.shop_nr=so.shop_nr order by gold desc limit 10");

  TRoom *tr=NULL;
  int i=1;
  ch->sendTo("\n\rThe ten wealthiest shops are:\n\r");
  while(db.fetchRow()){
    if((tr=real_roomp(convertTo<int>(db["in_room"])))){
      ch->sendTo(COLOR_BASIC, format("%i) %s with %s talens.\n\r") %
		 i % tr->getName() % db["gold"]);
    }
    ++i;
  }

  /////////////////////////////
  // shop types
  db.query("select type, count(*) as count from shoptype group by type order by count(*) desc");
  ch->sendTo("\n\rThe number of shops that deal in each commodity are:\n\r");
  
  while(db.fetchRow()){
    ch->sendTo(format("[%2s] %-17s   ") %
	       db["count"] % ItemInfo[convertTo<int>(db["type"])]->name);

    if(db.fetchRow()){
      ch->sendTo(format("[%2s] %-17s   ") %
		 db["count"] % ItemInfo[convertTo<int>(db["type"])]->name);
    }

    if(db.fetchRow()){
      ch->sendTo(format("[%2s] %-17s   ") %
		 db["count"] % ItemInfo[convertTo<int>(db["type"])]->name);
    }

    ch->sendTo("\n\r");
  }

  return TRUE;
}



int brickScorecard(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o1, TObj *o2) {
  int found=0;
  TThing *o=NULL;
  TObj *to;

  if(cmd != CMD_LOOK)
    return FALSE;

  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (o=*it);++it) {
    to = dynamic_cast<TObj *>(o);
    if (to && to->spec == SPEC_BRICKQUEST &&
	isname(arg, to->name)){
      found=1;
      break;
    }
  }

  if(!found)
    return FALSE;

  TDatabase db_brickquest(DB_SNEEZY);

  ch->sendTo("You examine the buletin board:\n\r");
  ch->sendTo("+------------------------------------------------+\n\r");
  ch->sendTo("| The players with the most collected bricks for |\n\r");
  ch->sendTo("|        the brick house quest are....           |\n\r");
  ch->sendTo("+------------------------------------------------+\n\r\n\r");

  db_brickquest.query("select * from brickquest order by numbricks desc limit 25");

  int i=1;
  while(i<=25){
    if(db_brickquest.fetchRow()){
      ch->sendTo(COLOR_BASIC, format("%s has %s bricks collected so far.\r\n") %
		 db_brickquest["name"] % db_brickquest["numbricks"]);
    }
    ++i;
  }
  return TRUE;
}


// Dash stuff - dec 2001

int force(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{

  TBeing *ch;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    sstring buf=sstring(arg).word(0);

    if(buf=="force"){
      if(ch->checkObjUsed(o)) {
        act("You cannot use $p's powers again this soon.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return FALSE;
      }

      ch->addObjUsed(o, UPDATES_PER_MUDHOUR * 24);

      act("$n holds $p aloft, shouting a <p>word of power<1>.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<c>The air around <1>$n<c> seems to waver and suddenly solidifies, expanding in a wave of force!<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);


      act("You hold $p aloft, shouting the word '<p>force<1>'.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<c>The air around you seems to waver and suddenly solidifies, expanding in a wave of force!<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);

      TThing *t=NULL;
      TBeing *vict = NULL;

      for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (t=*it);++it) {
	vict = dynamic_cast<TBeing *>(t);
	if (!vict)
	  continue;
	if (vict->fight() != ch)
	  continue;
	
	if (vict->riding) {
	  act("The wave of force knocks $N from $S mount!",
	      TRUE,ch,o,vict,TO_CHAR,NULL);
	  act("The wave of force knocks $N from $S mount!",
	      TRUE,ch,o,vict,TO_NOTVICT,NULL);
	  act("<o>The wave of force knocks you from your mount!<1>",
	      TRUE,ch,o,vict,TO_VICT,NULL);
	  vict->dismount(POSITION_RESTING);

	}	  
	act("The wave of force from your $o slams $N into the $g, stunning $M!",
	    TRUE,ch,o,vict,TO_CHAR,NULL);
	act("The wave of force from $n's $o slams $N into the $g, stunning $M!",
	    TRUE,ch,o,vict,TO_NOTVICT,NULL);
	act("The wave of force from $n's $o slams you into the $g, stunning you!",
	    TRUE,ch,o,vict,TO_VICT,NULL);

	affectedData aff;

	aff.type = SKILL_DOORBASH;
	aff.duration = ONE_SECOND;
	aff.bitvector = AFF_STUNNED;
	vict->affectTo(&aff, -1);
	if (vict->fight())
	  vict->stopFighting();
      }
      if (ch->fight())
	ch->stopFighting();
      return TRUE;
    }
  }
  return FALSE;

}






int frostArmor(TBeing *v, cmdTypeT cmd, const char *, TObj *o, TObj *weapon)
{
  TBeing *ch;
  int rc, dam;
  wearSlotT t;
  TObj *savedby=NULL;
  sstring buf;

  if(cmd != CMD_OBJ_BEEN_HIT || !v || !o)
    return FALSE;
  if(::number(0, 3))
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (weapon && !material_nums[weapon->getMaterial()].conductivity)
    savedby=weapon;
  t=((!weapon || (weapon->eq_pos==HOLD_RIGHT))?WEAR_HAND_R:WEAR_HAND_L);
  if(v->equipment[t] &&
     !material_nums[v->equipment[t]->getMaterial()].conductivity)
    savedby=dynamic_cast<TObj *>(v->equipment[t]);

  act("$p <b>glows a cold blue<1> and <c>freezes<1> $n!<1>"
      , 0, v, o, 0, TO_ROOM);
  act("$p <b>glows a cold blue<1> and <c>freezes<1> you!<1>"
      , 0, v, o, 0, TO_CHAR);

  if(savedby){
    buf="<k>Luckily, $s <1>$o<k> is not conductive and saves $m from harm.<1>";
    act(buf, 0, v, savedby, 0, TO_ROOM);
    buf="<k>Luckily, your <1>$o<k> is not conductive and saves you from harm.<1>";
    act(buf, 0, v, savedby, 0, TO_CHAR);
  } else {
    dam = ::number(2, 10);

    rc = ch->reconcileDamage(v, dam, DAMAGE_FROST);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }

  return TRUE;
}

int fireArmor(TBeing *v, cmdTypeT cmd, const char *arg, TObj *o, TObj *weapon)
{
  TBeing *ch;
  int rc, dam;
  //  wearSlotT t;
  //  TObj *savedby=NULL;
  //  char buf[256];

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;


  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    //    char buf[256];
    //    one_argument(arg, buf);
    if(!strcmp(arg, "scorching redemption")) {
      if(ch->checkObjUsed(o)) {
        act("The $o's powers can only be used once per day.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return FALSE;
      }
      affectedData aff1;

      act("$n closes $s eyes and whispers, '<p>scorching redemption<1>'.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<o>$p<o> becomes covered in searing flames, <R>completely engulfing<1><o> $n!<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("The $o slowly stops burning, but $n remains wreathed in <r>flames<1>.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);

      act("You close your eyes and whipser, '<p>scorching redemption<1>'.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<o>$p<o> becomes covered in searing flames, <R>completely engulfing<1><o> you!<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("The $o slowly stops burning, but you remain wreathed in <r>flames<1>.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);



      // ARMOR APPLY
      aff1.type = SPELL_FLAMING_FLESH;
      aff1.level = 30;
      aff1.duration = 12 * UPDATES_PER_MUDHOUR;
      aff1.location = APPLY_ARMOR;
      aff1.modifier = -75;


      ch->affectTo(&aff1);
      ch->addObjUsed(o, 24 * UPDATES_PER_MUDHOUR);
      ch->addToWait(combatRound(3));
      return TRUE;
    }
  }


  if(cmd != CMD_OBJ_BEEN_HIT || !v || !o)
    return FALSE;
  if(::number(0, 5))
    return FALSE;
 

  if (!::number(0,2)) {
    act("$p<o> emits a few dim sparks, then sputters out.<1>"
	, 0, v, o, 0, TO_ROOM);

    act("$p<o> emits a few dim sparks, then sputters out.<1>"
	, 0, v, o, ch, TO_CHAR);

  } else {
    act("$p<o> emits a <Y>dazzling<1><o> shower of <R>incandescent<1><o> sparks at $n!<1>"
	, 0, v, o, 0, TO_ROOM);
    act("$n is <r>burned<1> by the <o>fireworks<1> from the $o!<1>"
	, 0, v, o, ch, TO_ROOM);
    
    act("$p<o> emits a <Y>dazzling<1><o> shower of <R>incandescent<1><o> sparks at you!<1>"
	, 0, v, o, ch, TO_CHAR);
    act("You are <r>burned<1> by the <o>fireworks<1> from the $o!<1>"
	, 0, v, o, ch, TO_CHAR);
    
    
    dam = ::number(2, 30);
    
    rc = ch->reconcileDamage(v, dam, DAMAGE_FIRE);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }


  return TRUE;
}

int arcticHeart(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    sstring buf, buf2;
    buf=sstring(arg).word(0);
    buf2=sstring(arg).word(1);

    if(buf=="blizzard" && buf2=="soul"){
      if(ch->checkObjUsed(o)) {
        act("You cannot use $p's powers again this soon.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return FALSE;
      }

      ch->addObjUsed(o, UPDATES_PER_MUDHOUR);

      act("$n grips $p in one hand, and utters the word, '<b>blizzard soul<1>'.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<c>The $o glows for a moment, and a powerful <b>chill<1><c>runs through the room.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("You grip $p in one hand, and utter the word, '<b>blizzard soul<1>'.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<c>The $o glows for a moment, and a powerful <b>chill<1><c>runs through the room.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);



      Weather::changeWeatherT change = Weather::CHANGE_NONE;
      Weather::addToChange(-10);
      Weather::setPressure(960);
      Weather::AlterWeather(&change);

      
      ch->addToWait(combatRound(3));
      return TRUE;
    }
  }
  return FALSE;
}

int symbolBlindingLight(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;


  TSymbol *symbol = NULL;
  
  if (!(symbol = dynamic_cast<TSymbol *>(o)))
    return FALSE;
  
  TThing *t = NULL;
  TBeing *tmp_victim = NULL;
  
  if(!::number(0,1000) && cmd == CMD_GENERIC_PULSE) {
    
    act("$n's eyes suddenly glaze over as $e begins to chant in a monotonous voice.",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("You suddenly feel possesed by a higher power, and are compelled to chant.",TRUE,ch,o,NULL,TO_CHAR,NULL);
    
    
    ch->doSay("Oh Blinding Light!");
    ch->doSay("Oh Light That Blinds!");
    ch->doSay("I Cannot See!");
    ch->doSay("Look Out For Me!");    

    act("<Y>$n<Y>'s $o suddenly starts to glow and quickly becomes unbearably bright!<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("<Y>Your $o suddenly starts to glow and quickly becomes unbearably bright!<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    
    
    for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (t=*it);++it) {
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim)
	continue;
      
      
      act("$N is blinded by the light!", FALSE, tmp_victim, NULL, tmp_victim, TO_NOTVICT);
      act("You are blinded by the light!", FALSE, tmp_victim, NULL, NULL, TO_CHAR);
      tmp_victim->rawBlind(100, UPDATES_PER_MUDHOUR / 4, SAVE_NO);
      
    }

    symbol->addToSymbolCurStrength(symbol->getSymbolMaxStrength() - symbol->getSymbolCurStrength());
    return FALSE;

    
  }
  return FALSE;
}



int blizzardRing(TBeing *being, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  int rc;

  TBeing *ch;
  if (!o || !(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    sstring buf, buf2;
    buf=sstring(arg).word(0);
    buf2=sstring(arg).word(1);

    if(buf=="cold" && buf2=="shoulder"){
      if(ch->checkObjUsed(o)) {
        act("You cannot use $p's powers again this soon.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return FALSE;
      }

      ch->addObjUsed(o, UPDATES_PER_MUDHOUR * 24);

      act("$n's $o glows <b>a cold blue<1> as $e yells a <p>word of power<1>.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<c>The air around <1>$n<c> seems to waver, then becomes <B>extremely cold<1><c>!<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<c>A blast of frigid air radiates from <1>$n<c>!<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);


      act("Your $o glows <b>a cold blue<1> as you yell the word '<p>cold shoulder<1>'.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<c>The air around you seems to waver, then becomes <B>extremely cold<1><c>!<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<c>A blast of frigid air radiates from you<c>!<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);


      TThing *t=NULL;
      TBeing *vict = NULL;

      for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (t=*it);++it) {
        vict = dynamic_cast<TBeing *>(t);
        if (!vict)
          continue;
        if (vict->fight() != ch)
          continue;

        if (vict->riding) {
          act("The blast of <c>fro<b>zen <c>air<1> knocks $N from $S mount!",
              TRUE,ch,o,vict,TO_CHAR,NULL);
          act("The blast of <c>fro<b>zen <c>air<1> knocks $N from $S mount!",
              TRUE,ch,o,vict,TO_NOTVICT,NULL);
          act("<o>The blast of <c>fro<b>zen <c>air<1> knocks you from your mount!<1>",
              TRUE,ch,o,vict,TO_VICT,NULL);
          vict->dismount(POSITION_RESTING);

        }
        act("The blast of <c>fro<b>zen <c>air<1> from your $o slams $N into the $g, stunning $M!",
            TRUE,ch,o,vict,TO_CHAR,NULL);
        act("The blast of <c>fro<b>zen <c>air<1> from $n's $o slams $N into the $g, stunning $M!",
            TRUE,ch,o,vict,TO_NOTVICT,NULL);
        act("The blast of <c>fro<b>zen <c>air<1> from $n's $o slams you into the $g, stunning you!",
            TRUE,ch,o,vict,TO_VICT,NULL);

	      int dam = ::number(10,60);
	      rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
	      if (IS_SET_DELETE(rc, DELETE_VICT)) {
	        vict->reformGroup();
	        delete vict;
	        vict = NULL;
	        continue;
	      }

        affectedData aff;

        aff.type = SKILL_DOORBASH;
        aff.duration = ONE_SECOND;
        aff.bitvector = AFF_STUNNED;
        vict->affectTo(&aff, -1);
        if (vict->fight())
          vict->stopFighting();
      }
      if (ch->fight())
        ch->stopFighting();
      return TRUE;
    }
  }
  return FALSE;

}


int factionScoreBoard(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o1, TObj *o2)
{
  int found=0;
  TThing *o=NULL;
  TObj *to;

  if(cmd != CMD_LOOK)
    return FALSE;

  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (o=*it);++it) {
    to = dynamic_cast<TObj *>(o);
    if (to && to->spec == SPEC_FACTIONSCORE_BOARD &&
	isname(arg, to->name)){
      found=1;
      break;
    }
  }

  if(!found)
    return FALSE;

  ch->sendTo("You examine the board:\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");
  ch->sendTo(" Concerning the relative power of the 3 main factions:\n\r");
  ch->sendTo("------------------------------------------------------------\n\r\n\r");

  const sstring factnames[]={"cult", "snake", "brother"};
  TDatabase db(DB_SNEEZY);
  int totalscore=0;
  int score=0;

  for(int i=0;i<=2;++i){
    // faction header
    ch->sendTo(format("%s\n\r") % FactionInfo[factionNumber(factnames[i])].faction_name);

#if 0
    // get the number of members, we use this in a few places
    db.query("select count(*) as count from factionmembers where faction='%s'", factnames[i].c_str());
    db.fetchRow();
    int nmembers=convertTo<int>(db["count"]);
#endif

    totalscore=0;


    // number of members and total levels
    db.query("select level from factionmembers where faction='%s'", factnames[i].c_str());
    score=0;
    float level=0;
    while(db.fetchRow()){
      if(!db["level"].empty()){
	level=(float) convertTo<int>(db["level"]);
	score+=(int)(level * (level / 25.0));
      }
    }
    score=score/10; // scale down a bit

    ch->sendTo(COLOR_BASIC, format("<g>[<1>%3i<g>]<1> average level\n\r") % score);
    totalscore+=score;



    // fishing
    db.query("select sum(fk.weight) as weight from fishkeeper fk, factionmembers fm where fk.name=fm.name and fm.faction='%s'", factnames[i].c_str());
    score=0;
    if(db.fetchRow()){
      float pounds=0.0;

      if(!db["weight"].empty()){
	pounds=convertTo<float>(db["weight"]);
	score=(int)(pounds/10000.0);
      }
    }

    db.query("select count(*) as count from fishlargest fl, factionmembers fm where  fl.name=fm.name and fm.faction='%s'", factnames[i].c_str());
    if(db.fetchRow()){
      score+=convertTo<int>(db["count"]);
    }

    
    ch->sendTo(COLOR_BASIC, format("<g>[<1>%3i<g>]<1> pounds of fish caught and number of records\n\r") % score);
    totalscore+=score;




    // trophy
    //    db.query("select fm.level, count(*) from trophy t, factionmembers fm where t.name=fm.name and fm.faction='%s' group by fm.name, fm.level", factnames[i].c_str());
    //    db.query("select fm.level, t.count from trophyplayer t, factionmembers fm where t.name=fm.name and fm.faction='%s' group by fm.level, t.count", factnames[i].c_str());
    db.query("select fm.level, t.count from trophyplayer t, factionmembers fm, player p where p.name=fm.name and t.player_id=p.id and fm.faction='%s' group by fm.level, t.count", factnames[i].c_str());

    score=0;

    while(db.fetchRow()){
      score += convertTo<int>(db["count"]) * convertTo<int>(db["level"]);
    }
    score /= 10000;

    ch->sendTo(COLOR_BASIC, format("<g>[<1>%3i<g>]<1> average trophy percentage\n\r") % score);
    totalscore+=score;




    // shops
    db.query("select count(distinct soa.shop_nr) as count from shopownedaccess soa, factionmembers fm where (soa.access & %i)>0 and upper(fm.name) = upper(soa.name) and fm.faction='%s'", SHOPACCESS_OWNER, factnames[i].c_str());
    score=0;
    if(db.fetchRow()){
      score=convertTo<int>(db["count"])*10;

      ch->sendTo(COLOR_BASIC, format("<g>[<1>%3i<g>]<1> shops owned by faction members\n\r") % 
		 score);
      totalscore+=score;
    }




    // faction bank account
    score=(int)(FactionInfo[factionNumber(factnames[i])].getMoney()/100000.0);
    ch->sendTo(COLOR_BASIC, format("<g>[<1>%3i<g>]<1> faction wealth\n\r") % score);
    totalscore+=score;




    // total score
    ch->sendTo(COLOR_BASIC, format("<g>[<R>%3i<1><g>]<1> total score\n\r") % totalscore);

    // end of faction
    ch->sendTo("\n\r\n\r");

  }

  return TRUE;
}


int fragileArrow(TBeing *v, cmdTypeT cmd, const char *, TObj *o, TObj *)
{


  
  switch (cmd) {
    case CMD_ARROW_GLANCE:
    case CMD_ARROW_MISSED:
    case CMD_ARROW_DODGED:
    case CMD_ARROW_HIT_OBJ:
    case CMD_OBJ_EXPELLED:
      //   vlogf(LOG_DASH,"fragile arrow proc called case 1");
      act("$n<1> strikes the $g at an odd angle and snaps in two.", FALSE, o,0,0,TO_ROOM,NULL);
      delete o;
      break;
    case CMD_REMOVE:
    case CMD_ARROW_RIPPED:
      //      vlogf(LOG_DASH,"fragile arrow proc called case 2");
      act("$n accidentally snaps $p in two as $e rips it out.", FALSE, v,o,0,TO_ROOM,NULL);
      act("You accidentally snap $p in two as you rip it out.", FALSE, v,o,0,TO_CHAR,NULL);
      delete o;
      break;
    case CMD_OBJ_GOTTEN:
      act("$n accidentally snaps $p in two as $e picks it up.", FALSE, v,o,0,TO_ROOM,NULL);
      act("You accidentally snap $p in two as you pick it up.", FALSE, v,o,0,TO_CHAR,NULL);
      delete o;
      break;
    default:
      return FALSE;
  }

  return TRUE;
}


int starfire(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam = 1;

  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (::number(0,10))
    return FALSE;
  if (cmd != CMD_OBJ_HIT)
    return FALSE;

  int hitterLev = ch->GetMaxLevel();
  dam = (::number((hitterLev / 10 + 1),(hitterLev / 3 + 4)));
  act("<c>The sapphires on your $o begin to radiate a soft blue light as you strike $N!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
  act("<c>The sapphires on $n's $o begin to radiate a soft blue light as $e strikes $N!<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("<c>The sapphires on $n's $o begin to radiate a soft blue light as $e strikes you!<1>",TRUE,ch,o,vict,TO_VICT,NULL);

  if (dam >= ( ( ((hitterLev/3+4)-(hitterLev/10+1))*4 )/5 + (hitterLev/10+1))) {
    act("<W>$p<W> emits an <Y>enourmously<1><W> dazzling burst of light and heat!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<W>$p<W> emits an <Y>enourmously<1><W> dazzling burst of light and heat!<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<W>$p<W> emits an <Y>enourmously<1><W> dazzling burst of light and heat!<1>",TRUE,ch,o,vict,TO_VICT,NULL);
 
    act("<W>$N<W> screams in pain as the heated metal burns $M!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<W>$N<W> screams in pain as the heated metal burns $M!<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<W>You scream in pain as the heated metal burns you!<1>",TRUE,ch,o,vict,TO_VICT,NULL);


  } else {
    act("<W>$p<W> emits a dazzling burst of light and heat!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<W>$p<W> emits a dazzling burst of light and heat!<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<W>$p<W> emits a dazzling burst of light and heat!<1>",TRUE,ch,o,vict,TO_VICT,NULL);

    act("<W>$N<W> screams in pain as the heated metal burns $M!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<W>$N<W> screams in pain as the heated metal burns $M!<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<W>You scream in pain as the heated metal burns you!<1>",TRUE,ch,o,vict,TO_VICT,NULL);

  }
  
  rc = ch->reconcileDamage(vict, dam, TYPE_SMITE);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}


int starfiresheath(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  TObj *sword = NULL;
  TBeing *ch2 = NULL;

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    sstring buf;
    buf=sstring(arg).word(0);

    if(buf=="kaeshite"){
      act("<c>$n<c> utters a word of <p>power<c>.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<c>You utter a word of <p>power<c>.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);

      if (!(sword = get_obj("blade starfire sword two handed quest", EXACT_NO))) {
	act("<c>$n's $o glows pale blue for a moment, but nothing seems to happen.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
        act("<c>Your $o glows pale blue for a moment, but nothing seems to happen.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
	return TRUE;
      } else {
        act("<c>$n's $o glows pale blue for a moment.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
        act("<c>Your $o glows pale blue for a moment.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);

	if (sword->equippedBy) {
	  if (sword->eq_pos > WEAR_NOWHERE) {
	    ch2 = dynamic_cast<TBeing *>(sword->equippedBy);

	    act("<W>$p <W>suddenly turns incredibly hot in your hands, and you drop it!<1>",TRUE,ch2,sword,NULL,TO_CHAR,NULL);

	    act("<W>$p <W>suddenly turns incredibly hot in $n's hands, and $e drops it!<1>",TRUE,ch2,sword,NULL,TO_ROOM,NULL);

	    *ch2->roomp += *ch2->unequip(sword->eq_pos);
	  } else {
	    vlogf(LOG_BUG, format("starfire proc trying to unequip %s in slot -1 on %s") %  sword->getName() %
		  sword->equippedBy->getName());
	    return FALSE;
	  }
	}

	if(sword->stuckIn){
	  ch2 = dynamic_cast<TBeing *>(sword->stuckIn);
	  ch2->setStuckIn(sword->eq_stuck, NULL);
	  sword->equippedBy = NULL;
	  sword->stuckIn = NULL;
	  sword->eq_pos = WEAR_NOWHERE;
	  sword->eq_stuck = WEAR_NOWHERE;
	  *ch2->roomp += *sword;
	}


	if(!(ch2 = dynamic_cast<TBeing *>(sword->parent))) {
	  
	  act("<c>The sapphires on $n'<c>s hilt suddenly emit a bright burst of light!<1>",TRUE,sword,NULL,NULL,TO_ROOM,NULL);
	  act("<W>$n <W>dissapears in a blinding flash!<1>",TRUE,sword,NULL,NULL,TO_ROOM,NULL);
	} else {
	  act("<c>The sapphires on $p<c>'s hilt suddenly emit a bright burst of light!<1>",TRUE,ch2,sword,NULL,TO_CHAR,NULL);
          act("<W>$p <W>dissapears from your hands in a blinding flash!<1>",TRUE,ch2,sword,NULL,TO_CHAR,NULL);

	  act("<c>The sapphires on $p<c>'s hilt suddenly emit a bright burst of light!<1>",TRUE,ch2,sword,NULL,TO_ROOM,NULL);
          act("<W>$p <W>dissapears from $n<W>'s hands in a blinding flash!<1>",TRUE,ch2,sword,NULL,TO_ROOM,NULL);
	}
	--(*sword);
	*o += *sword;
	act("<W>$N<W> suddenly materializes in $n's $o with a faint metallic sound.<1>",TRUE,ch,o,sword,TO_ROOM,NULL);
        act("<W>$N<W> suddenly materializes in your $o with a faint metallic sound.<1>",TRUE,ch,o,sword,TO_CHAR,NULL);
	return TRUE;
      }
    }
    return FALSE;
  }
  
  if (!::number(0,9) && cmd == CMD_GENERIC_PULSE && o->getStructPoints() < o->getMaxStructPoints()) {
    if(::number(1,100) < (int)(100.0*((float)(o->getStructPoints()) / (float)(o->getMaxStructPoints()))))
      return FALSE;
    
    act("<c>$n<c>'s $o slowly reconstructs itself, erasing signs of damage.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("<c>Your $o slowly reconstructs itself, erasing signs of damage.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    
    o->addToStructPoints(::number(1,min(5, o->getMaxStructPoints() - o->getStructPoints())));
    return FALSE;
  }
  return FALSE;
}



int teleportRescue(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TRoom *room;
  
  if(cmd != CMD_OBJ_OWNER_HIT)
    return FALSE;

  if(ch->getHit() < (ch->hitLimit()*0.15)){
    act("<W>Your $o glows brightly and then shatters into hundreds of pieces!<1>", TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("<W>$n's $o glows brightly and then shatters into hundreds of pieces!<1>", TRUE,ch,o,NULL,TO_ROOM,NULL);

    --*ch;
    room=real_roomp(100);
    *room += *ch;

    delete o;

    act("A sense of nausea comes over you and you find youself in another place.", TRUE,ch,NULL,NULL,TO_CHAR,NULL);
    act("$n appears in the room, looking confused.", TRUE,ch,NULL,NULL,TO_ROOM,NULL);

    return TRUE;
  }

  return FALSE;
}



//dashmark

int HSCopsi(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
#if 0
  // weird proc
  // three procs that work together, two hammers that charge the pendant, pendant does weird magic
  // stuff when used.
  // nothing works unless all three are being used.

  if (::number(0,15))
    return FALSE;
  if (cmd != CMD_OBJ_HIT)
    return FALSE;

  TBeing *ch;
  TObj *pendant = NULL;
  TObj *hammer1 = NULL;
  TObj *hammer2 = NULL;
  affectedData aff;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  
  
  if(!(pendant = dynamic_cast<TObj *>(ch->equipment[WEAR_NECK])))
    return FALSE;
  if (obj_index[pendant->getItemIndex()].virt != 17258)
    return FALSE;
  
  if(!(hammer1 = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT])))
    return FALSE;
  if (obj_index[hammer1->getItemIndex()].virt != 17256 || obj_index[hammer1->getItemIndex()].virt != 17257)
    return FALSE;
  
  if(!(hammer2 = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT])))
    return FALSE;
  if (obj_index[hammer2->getItemIndex()].virt != 17256 || obj_index[hammer2->getItemIndex()].virt != 17257)
    return FALSE;

  // yay, he's using all three. 
  int charge = 0;
  sscanf(pendant->name, "pendant crystal hammer [quest] [charge=%d]", &charge);


  int dam = (::number(1,5));
  act("$p shines with a <c>co<b>ol <B>cobalt<1><b> au<1><c>ra<1> as it drains energy from $N.",TRUE,ch,o,vict,TO_CHAR,NULL);
  act("$p shines with a <c>co<b>ol <B>cobalt<1><b> au<1><c>ra<1> as it drains energy from $N.",TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("$p shines with a <c>co<b>ol <B>cobalt<1><b> au<1><c>ra<1> as it drains energy from you!.",TRUE,ch,o,vict,TO_VICT,NULL);


  act("Your $o flashes pale <b>blue<1> for a moment.",TRUE,ch,pendant,vict,TO_CHAR,NULL);
  act("$n's $o flashes pale <b>blue<1> for a moment.",TRUE,ch,pendant,vict,TO_NOTVICT,NULL);
  act("$n's $o flashes pale <b>blue<1> for a moment.",TRUE,ch,pendant,vict,TO_VICT,NULL);
 
  
  charge += dam;
  sprintf(pendant->name, "pendant crystal hammer [quest] [charge=%d]", charge);
  
  int rc = ch->reconcileDamage(vict, dam, DAMAGE_DRAIN);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
#endif

  return FALSE;
}


int HSPeke(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
#if 0
  // weird proc
  // three procs that work together, two hammers that charge the pendant, pendant does weird magic
  // stuff when used.
  // nothing works unless all three are being used.

  if (::number(0,15))
    return FALSE;
  if (cmd != CMD_OBJ_HIT)
    return FALSE;

  TBeing *ch;
  TObj *pendant = NULL;
  TObj *hammer1 = NULL;
  TObj *hammer2 = NULL;
  affectedData aff;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;


  if(!(pendant = dynamic_cast<TObj *>(ch->equipment[WEAR_NECK])))
    return FALSE;
  if (obj_index[pendant->getItemIndex()].virt != 17258)
    return FALSE;

  if(!(hammer1 = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT])))
    return FALSE;
  if (obj_index[hammer1->getItemIndex()].virt != 17256 || obj_index[hammer1->getItemIndex()].virt != 17257)
    return FALSE;

  if(!(hammer2 = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT])))
    return FALSE;
  if (obj_index[hammer2->getItemIndex()].virt != 17256 || obj_index[hammer2->getItemIndex()].virt != 17257)
    return FALSE;

  // yay, he's using all three.
  int charge = 0;
  sscanf(pendant->name, "pendant crystal hammer [quest] [charge=%d]", &charge);


  int dam = (::number(1,5));

  act("$p shines with a <o>wa<r>rm <R>scarlet<1><r> au<1><o>ra<1> as it drains energy from $N.",TRUE,ch,o,vict,TO_CHAR,NULL);
  act("$p shines with a <o>wa<r>rm <R>scarlet<1><r> au<1><o>ra<1> as it drains energy from $N.",TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("$p shines with a <o>wa<r>rm <R>scarlet<1><r> au<1><o>ra<1> as it drains energy from you!.",TRUE,ch,o,vict,TO_VICT,NULL);


  act("Your $o flashes pale <r>red<1> for a moment.",TRUE,ch,pendant,vict,TO_CHAR,NULL);
  act("$n's $o flashes pale <r>red<1> for a moment.",TRUE,ch,pendant,vict,TO_NOTVICT,NULL);
  act("$n's $o flashes pale <r>red<1> for a moment.",TRUE,ch,pendant,vict,TO_VICT,NULL);

  charge += dam;
  sprintf(pendant->name, "pendant crystal hammer [quest] [charge=%d]", charge);

  int rc = ch->reconcileDamage(vict, dam, DAMAGE_DRAIN);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;

#endif

  return FALSE;
}


int HSPendant(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
#if 0
  // weird proc
  // three procs that work together, two hammers that charge the pendant, pendant does weird magic
  // stuff when used.
  // nothing works unless all three are being used.

  if (::number(0,5))
    return FALSE;
  if (cmd != CMD_GENERIC_QUICK_PULSE)
    return FALSE;

  TBeing *ch;
  TObj *pendant = NULL;
  TObj *hammer1 = NULL;
  TObj *hammer2 = NULL;
  affectedData aff;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;


  if(!(pendant = dynamic_cast<TObj *>(ch->equipment[WEAR_NECK])))
    return FALSE;
  if (obj_index[pendant->getItemIndex()].virt != 17258)
    return FALSE;

  if(!(hammer1 = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT])))
    return FALSE;
  if (obj_index[hammer1->getItemIndex()].virt != 17256 || obj_index[hammer1->getItemIndex()].virt != 17257)
    return FALSE;

  if(!(hammer2 = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT])))
    return FALSE;
  if (obj_index[hammer2->getItemIndex()].virt != 17256 || obj_index[hammer2->getItemIndex()].virt != 17257)
    return FALSE;

  // yay, he's using all three.
  int charge = 0;
  sscanf(pendant->name, "pendant crystal hammer [quest] [charge=%d]", &charge);

  if (charge < 50)
    return FALSE;

  act("Your $o shines with a <p>violet<1> aura briefly.",TRUE,ch,o,vict,TO_CHAR,NULL);
  act("$n's $o shines with a <p>violet<1> aura briefly.",TRUE,ch,o,vict,TO_NOTVICT,NULL);

  aff.type = SPELL_PLASMA_MIRROR;
  aff.duration = UPDATES_PER_MUDHOUR/2;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  aff.level = 40;
  ch->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES);

  charge -= 50;
  sprintf(pendant->name, "pendant crystal hammer [quest] [charge=%d]", charge);

#endif



  return FALSE;
}




// use this for a non-closeable object
// can close the object in the zonefile
int mobSpawnOpen(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  
  if (cmd != CMD_OBJ_OPENED && cmd != CMD_GENERIC_RESET) 
    return FALSE;

  TOpenContainer *cont;
  if (!(cont=dynamic_cast<TOpenContainer *>(o)))
    return FALSE;

  if (cmd == CMD_GENERIC_RESET) {
    if (!cont->isContainerFlag(CONT_CLOSED)) 
    {
      cont->addContainerFlag(CONT_CLOSED);
      o->roomp->sendTo(COLOR_BASIC, format("The %s slams shut.\n\r") % o->name);
    }
    return FALSE;
  }

  TBeing *mob = read_mobile(obj_index[cont->getItemIndex()].virt , VIRTUAL);
  if (!mob) {
    ch->sendTo("Problem!  Tell a god.\n\r");
    return FALSE;
  }
  *ch->roomp += *mob;

  colorAct(COLOR_MOBS, ((mob->ex_description && mob->ex_description->findExtraDesc("repop")) ?
                        mob->ex_description->findExtraDesc("repop") :
                        "$n appears suddenly in the room."),
           TRUE, mob, 0, 0, TO_ROOM);

  return FALSE;
}

int energyShield(TBeing *v, cmdTypeT cmd, const char *, TObj *o, TObj *weapon)
{
#if 0
  TBeing *ch;
  TObj *generator = NULL;
  sstring buf, buf2;

  if (cmd != CMD_GENERIC_QUICK_PULSE)
    return FALSE;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy))) {

    delete o;
    return FALSE; 
  }
  if(!(generator = dynamic_cast<TObj *>(ch->equipment[WEAR_WAIST]))) {
    act("Your $o collapses.",TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("$n's $o collapses.",TRUE,ch,o,NULL,TO_ROOM,NULL);
    delete o;
    return FALSE;
  }
  if (obj_index[generator->getItemIndex()].virt != 18580) {
    act("Your $o collapses.",TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("$n's $o collapses.",TRUE,ch,o,NULL,TO_ROOM,NULL);
    delete o;
    return FALSE;
  }

  
  int isOn = 0; //0 is false, 1 is true
  
  int charge = 0;
  sscanf(generator->name, "generator shield belt [on=%d] [charge=%d]", &isOn, &charge);
  int newcharge = charge;
  
  if (!isOn || charge < 1) {
    act("Your $o collapses.",TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("$n's $o collapses.",TRUE,ch,o,NULL,TO_ROOM,NULL);
    delete o;
    return FALSE;
  }
  
  newcharge = newcharge - max(0,::number(-8,1)); // upkeep.. basically .1 point/round/shield
  
  if(o->getStructPoints() < o->getMaxStructPoints()) {
    o->addToStructPoints(1);
    newcharge = newcharge - 1; // recharge
  }
  
  if ((charge-1) / 100 != (newcharge-1) / 100 || (newcharge == 1000 && charge < 1000)) {
    if (newcharge / 100 <= 3) buf="<r>red";
    else if ( newcharge / 100 <= 6) buf="<Y>yellow";
    else if ( newcharge / 100 <= 9) buf="<g>green";
    else buf="<c>blue";
    
    buf2 = format("The display panel on your $o glows %s<1> as it reads %d0%c.") % buf % (newcharge/100) % '%';
    act(buf2,TRUE,ch,generator,NULL,TO_CHAR,NULL);
    buf2 = format("The display panel on $n's $o glows %s<1>.") % buf;
    act(buf2,TRUE,ch,generator,NULL, TO_ROOM,NULL);
  }
  
  
  sprintf(generator->name, "generator shield belt [on=%d] [charge=%d]", isOn, newcharge);
  
#endif

  return FALSE;
  
}


int energyShieldGenerator(TBeing *v, cmdTypeT cmd, const char *arg, TObj *o, TObj *weapon)
{
#if 0
  TBeing *ch = NULL;
  //  TObj *shield = NULL;
  sstring buf, buf2, arg1, arg2;

  int charge;
  int newcharge;
  int isOn; //0 is false, 1 is true

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (cmd == CMD_GENERIC_PULSE) {

    sscanf(o->name, "generator shield belt [on=%d] [charge=%d]", &isOn, &charge);
    
    newcharge = charge;

    if (charge < 1000 && ch->outside() && is_daytime()) {

      newcharge = min(1000,newcharge + ::number(10,30));
    }

    if (isOn) {
      
      
      wearSlotT il;
      
      for (il = MIN_WEAR; il < MAX_WEAR; il++) {
	if (il == HOLD_RIGHT || il == HOLD_LEFT)
	  continue;
	if (ch->slotChance(il) && !ch->equipment[il] && newcharge > 10) {
	  TObj *shield = NULL;
	  if(!(shield = read_object(18581, VIRTUAL))) {
	    vlogf(LOG_PROC, "Shield generator couldn't load energy shield!");
	    return TRUE;
	  }
	  
	  int bit = (1<<14);
	  
	  if(il == WEAR_HEAD) bit = ITEM_WEAR_HEAD;
	  if(il == WEAR_NECK) bit = ITEM_WEAR_NECK;
	  if(il == WEAR_BACK) bit = ITEM_WEAR_BACK;
	  if(il == WEAR_BODY) bit = ITEM_WEAR_BODY;
	  if(il == WEAR_ARM_R) bit = ITEM_WEAR_ARMS;
	  if(il == WEAR_ARM_L) bit = ITEM_WEAR_ARMS;
	  if(il == WEAR_WRIST_R) bit = ITEM_WEAR_WRISTS;
	  if(il == WEAR_WRIST_L) bit = ITEM_WEAR_WRISTS;
	  if(il == WEAR_HAND_R) bit = ITEM_WEAR_HANDS;
	  if(il == WEAR_HAND_L) bit = ITEM_WEAR_HANDS;
	  if(il == WEAR_FINGER_R) bit = ITEM_WEAR_FINGERS;
	  if(il == WEAR_FINGER_L) bit = ITEM_WEAR_FINGERS;
	  if(il == WEAR_WAIST) bit = ITEM_WEAR_WAIST;
	  if(il == WEAR_LEG_R) bit = ITEM_WEAR_LEGS;
	  if(il == WEAR_LEG_L) bit = ITEM_WEAR_LEGS;
	  if(il == WEAR_FOOT_R) bit = ITEM_WEAR_FEET;
	  if(il == WEAR_FOOT_L) bit = ITEM_WEAR_FEET;
	  if(il == WEAR_EX_LEG_R) bit = ITEM_WEAR_LEGS;
	  if(il == WEAR_EX_LEG_L) bit = ITEM_WEAR_LEGS;
	  if(il == WEAR_EX_FOOT_R) bit = ITEM_WEAR_FEET;
	  if(il == WEAR_EX_FOOT_L) bit = ITEM_WEAR_FEET;
	  
	  shield->obj_flags.wear_flags = bit;
	  TBaseClothing *armor = NULL;
	  
	  if ((armor = dynamic_cast<TBaseClothing *>(shield))) {
	    armor->setDefArmorLevel(ch->GetMaxLevel());
	    armor->setVolume((int) (((100. * (double) ch->getHeight())) * race_vol_constants[mapSlotToFile(il)] / 100));
	  }
	  
	  ch->equipChar(shield, il);
	  
	  buf2 = format("Your %s is surrounded by a crackling blue aura.") % ch->describeBodySlot((wearSlotT)il);
	  
	  act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
	  buf2 = format("$n's %s is surrounded by a crackling blue aura.") % ch->describeBodySlot((wearSlotT)il);
	  
	  act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
	  
	  newcharge = newcharge - 10;
	  
	}
      }

    }
    if ((charge-1) / 100 != (newcharge-1) / 100 || (newcharge == 1000 && charge < 1000)) {
      if (newcharge / 100 <= 3) buf="<r>red";
      else if ( newcharge / 100 <= 6) buf="<Y>yellow";
      else if ( newcharge / 100 <= 9) buf="<g>green";
      else buf="<c>blue";
      
      buf2 = format("The display panel on your $o glows %s<1> as it reads %d0%c.") % buf % (newcharge/100) % '%';
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2 = format("The display panel on $n's $o glows %s<1>.") % buf;
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
      
    }
    
    sprintf(o->name, "generator shield belt [on=%d] [charge=%d]", isOn, newcharge);
    return FALSE;

  } else if ((cmd == CMD_PUSH || cmd == CMD_PRESS)) {
    arg1=sstring(arg).word(0);
    arg2=sstring(arg).word(1);

    if (is_abbrev(arg1, "display") && is_abbrev(arg2, "button")) {
     
      sscanf(o->name, "generator shield belt [on=%d] [charge=%d]", &isOn, &charge);
      
      if (charge / 100 <= 3) buf="<r>red";
      else if (charge / 100 <= 6) buf="<Y>yellow";
      else if (charge / 100 <= 9) buf="<g>green";
      else buf="<c>blue";


      act("You press the display button on $p.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("$n presses the display button on $p.",TRUE,ch,o,NULL, TO_ROOM,NULL);


      buf2 = format("The display panel on your $o glows %s<1> as it reads %d%c.") % buf % (charge/10) % '%';
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2 = format("The display panel on $n's $o glows %s<1>.") % buf;
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
      buf2 = format("The power LED on your $o is currently %s<1>.") % (isOn ? "<g>on<1>" : "<r>off<1>");
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);

      
      return TRUE;
    } else if (is_abbrev(arg1, "power") && is_abbrev(arg2, "button")) {
      int charge = 0;
      sscanf(o->name, "generator shield belt [on=%d] [charge=%d]", &isOn, &charge);
      


      act("You press the power button on $p.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("$n presses the power button on $p.",TRUE,ch,o,NULL, TO_ROOM,NULL);


      if (isOn) {
	isOn = 0;
      } else {
	isOn = 1;
      }

      buf2 = format("The power LED on your $o turns %s<1>.") % (isOn ? "<g>on<1>" : "<r>off<1>");
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);

      sprintf(o->name, "generator shield belt [on=%d] [charge=%d]", isOn, charge);


      return TRUE;

    }
  }       

#endif
  return FALSE;
}

int stimPack(TBeing *v, cmdTypeT cmd, const char *arg, TObj *o, TObj *weapon)
{
#if 0
  TBeing *ch = NULL;
  //  TObj *shield = NULL;

  sstring buf2, arg1, arg2;

  int charge;
  int newcharge;
  int isOn = 0; // 1 = on, 0 = off
  bool ret = FALSE;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  sscanf(o->name, "forearm-guard guard plastic lights stim wristband [on_%d] [charge_%d]", &isOn,  &charge);
  newcharge = charge;

  if (cmd == CMD_GENERIC_PULSE) {

    if (charge < 1000 && ch->getHit() > 1 && isOn == 1 && !(::number(0,9))) {
      // if we aren't fully charged, we have hitpoints left, the charger is on, then 1 in 10 chance of...
      ch->setHit(max(1,ch->getHit() - ::number(10,20))); // subtract 1-10 hp from player, but don't kill
      newcharge = min(1000,newcharge + ::number(10,50));
      act("You wince as your <W>forearm guard<1> drains some <r>blood<1> from your arm.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("$n winces and grabs $s forearm in pain.",TRUE,ch,o,NULL, TO_ROOM,NULL);
      act("The <P>charge<1> light on your <W>forearm guard<1> blinks briefly.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("A small <P>light<1> on $n's <W>forearm guard<1> blinks briefly.",TRUE,ch,o,NULL, TO_ROOM,NULL);
    }



  } else if ((cmd == CMD_PUSH || cmd == CMD_PRESS)) {
    arg1=sstring(arg).word(0);
    arg2=sstring(arg).word(1);

    if (is_abbrev(arg1, "display") && is_abbrev(arg2, "button")) {
      act("You press the display button on your <W>forearm guard<1>.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("$n presses a button on $s <W>forearm guard<1>.",TRUE,ch,o,NULL, TO_ROOM,NULL);
      
      
      buf2="The display panel on your <W>forearm guard<1> flips open, revealing a row of lights.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="A panel on $n's <W>forearm guard<1> flips open, revealing a row of lights.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
      buf2 = format("The light representing the first stim is %s.") % (charge >= 1000 ? "<B>lit<1>" : "<k>dim<1>");
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2 = format("The light representing the second stim is %s.") % (charge >= 800 ? "<C>lit<1>" : "<k>dim<1>");
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2 = format("The light representing the third stim is %s.") % (charge >= 600 ? "<G>lit<1>" : "<k>dim<1>");
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2 = format("The light representing the fourth stim is %s.") % (charge >= 400 ? "<Y>lit<1>" : "<k>dim<1>");
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2 = format("The light representing the fifth stim is %s.") % (charge >= 200 ? "<R>lit<1>" : "<k>dim<1>");
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="$n glances quickly at the panel before flipping it closed again.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
      buf2 = format("The charging LED on your <W>forearm guard<1> is currently %s<1>.") % (isOn ? "<P>on<1>" : "<k>off<1>");
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="You quickly flip the display panel closed again.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);

      
      ret = TRUE;
    } else if (is_abbrev(arg1, "stim") && is_abbrev(arg2, "button")) {
      
      act("You press the stim button on your <W>forearm guard<1>.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("$n presses a button on $s <W>forearm guard<1>.",TRUE,ch,o,NULL, TO_ROOM,NULL);
      
      if (charge < 200) {
	buf2="Nothing seems to happen.";
	act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      } else {
      
	newcharge = charge - 200;

	affectedData aff;
	
	aff.type = AFFECT_DRUG;
	aff.level = 50;
	aff.duration = 2 * UPDATES_PER_MUDHOUR;
	aff.modifier = ::number(10,20);
	aff.location = APPLY_SPE;
	aff.bitvector = 0;
	ch->affectTo(&aff, -1);
	aff.type = AFFECT_DRUG;
	aff.level = 50;
	aff.duration = 2 * UPDATES_PER_MUDHOUR;
	aff.modifier = 0 - ::number(10,20);
	aff.location = APPLY_CON;
	aff.bitvector = 0;
	ch->affectTo(&aff, -1);
	if (charge == 1000) {  // give an extra boost when stimming from full 
	  aff.type = SPELL_HASTE;
	  aff.level = 50;
	  aff.duration =  1 * UPDATES_PER_MUDHOUR;
	  aff.modifier = 0;
	  aff.location = APPLY_NONE;
	  aff.bitvector = 0;
	  ch->affectTo(&aff, -1);
	}
	
	act("You feel a <r>sharp prick<1> from your <W>forearm guard<1>.",TRUE,ch,o,NULL,TO_CHAR,NULL);
	act("<o>Suddenly a tendril of burning fire seems to course through your bloodstream!<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
        act("You shudder in pain but simultaneously feel your reflexes become quicker!<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
	act("$n's eyes widen for a second as $e gasps in pain.",TRUE,ch,o,NULL,TO_ROOM,NULL);
        act("$e shudders momentarily.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
	int rc = ch->reconcileDamage(ch, ::number(5,10), DAMAGE_DRAIN);
	if (rc == -1)
	  return DELETE_VICT;
      }
      ret = TRUE;
      
    } else if (is_abbrev(arg1, "charge") && is_abbrev(arg2, "button")) {

      act("You press the charge button on your <W>forearm guard<1>.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("$n presses a button on $s <W>forearm guard<1>.",TRUE,ch,o,NULL, TO_ROOM,NULL);


      if (isOn) {
        isOn = 0;
      } else {
        isOn = 1;
      }

      buf2 = format("The charging LED on your $o turns %s<1>.") % (isOn ? "<P>on<1>" : "<k>off<1>");
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2 = format("A little %s on $n's <W>forearm guard<1> %s.") %
	(isOn ? "<k>light<1>" : "<P>light<1>") % 
	(isOn ? "turns <P>on<1>" : "goes <k>out<1>");
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
      
      sprintf(o->name, "forearm-guard guard plastic lights stim wristband [on_%d] [charge_%d]", isOn, newcharge);
      ret = TRUE;
      
    }
    
  }
  
  
  if (newcharge != charge) {
    // for display lights turning on  
    if (newcharge >= 200 && charge < 200) {
      buf2="The <k>first<1> LED on your <W>forearm guard<1> begins to <R>glow<1>.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="One of the <k>lights<1> on $n's <W>forearm guard<1> begins to <R>glow<1>.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
    }
    if (newcharge >= 400 && charge < 400) {
      buf2="The <k>second<1> LED on your <W>forearm guard<1> begins to <Y>glow<1>.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="One of the <k>lights<1> on $n's <W>forearm guard<1> begins to <Y>glow<1>.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
    }
    if (newcharge >= 600 && charge < 600) {
      buf2="The <k>third<1> LED on your <W>forearm guard<1> begins to <G>glow<1>.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="One of the <k>lights<1> on $n's <W>forearm guard<1> begins to <G>glow<1>.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
    }
    if (newcharge >= 800 && charge < 800) {
      buf2="The <k>fourth<1> LED on your <W>forearm guard<1> begins to <C>glow<1>.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="One of the <k>lights<1> on $n's <W>forearm guard<1> begins to <C>glow<1>.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
    }
    if (newcharge == 1000 && charge < 1000) {
      buf2="The <k>fifth<1> LED on your <W>forearm guard<1> begins to <B>glow<1>.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="One of the <k>lights<1> on $n's <W>forearm guard<1> begins to <B>glow<1>.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
    }
    // for display lights turning off
    if (newcharge < 200 && charge >= 200) {
      buf2="The <R>first<1> LED on your <W>forearm guard<1> stops <k>glowing<1>.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="One of the <R>lights<1> on $n's <W>forearm guard<1> stops <k>glowing<1>.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
    }
    if (newcharge < 400 && charge >= 400) {
      buf2="The <Y>second<1> LED on your <W>forearm guard<1> stops <k>glowing<1>.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="One of the <Y>lights<1> on $n's <W>forearm guard<1> stops <k>glowing<1>.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
    }
    if (newcharge < 600 && charge >= 600) {
      buf2="The <G>third<1> LED on your <W>forearm guard<1> stops <k>glowing<1>.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="One of the <G>lights<1> on $n's <W>forearm guard<1> stops <k>glowing<1>.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
    }
    if (newcharge < 800 && charge >= 800) {
      buf2="The <C>fourth<1> LED on your <W>forearm guard<1> stops <k>glowing<1>.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="One of the <C>lights<1> on $n's <W>forearm guard<1> stops <k>glowing<1>.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
    }
    if (newcharge < 1000 && charge >= 1000) {
      buf2="The <B>fifth<1> LED on your <W>forearm guard<1> stops <k>glowing<1>.";
      act(buf2,TRUE,ch,o,NULL,TO_CHAR,NULL);
      buf2="One of the <B>lights<1> on $n's <W>forearm guard<1> stops <k>glowing<1>.";
      act(buf2,TRUE,ch,o,NULL, TO_ROOM,NULL);
      
    }
    sprintf(o->name, "forearm-guard guard plastic lights stim wristband [on_%d] [charge_%d]", isOn, newcharge);
  }

  return ret;
#endif  
  return FALSE;

}




int finnsGaff(TBeing *, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  sstring target,buf;
  TObj *fish;
  TBeing *ch;
  int amt=20;

  if(cmd != CMD_GENERIC_QUICK_PULSE && cmd != CMD_POINT)
    return false;
  
  if (!o || !(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if((cmd == CMD_GENERIC_QUICK_PULSE) && !::number(0,1)){
    if(ch->getMove()<(ch->moveLimit()-amt) && o->getStructPoints()>1){
      amt=min(amt, o->getStructPoints()-1);
      o->setStructPoints(o->getStructPoints()-amt);
      o->setMaxStructPoints(o->getMaxStructPoints()-amt);
      ch->addToMove(amt);
      act("You feel refreshed as your $o begins to exude a stinky fish smell.",
	  TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("$n looks refreshed as $p begins to exude a stinky fish smell.",
	  TRUE,ch,o,NULL, TO_ROOM,NULL);
      return true;
    }
  }
  
  if(cmd == CMD_POINT && o->getMaxStructPoints()<100){
    one_argument((sstring)arg,target);

    if(!(fish=generic_find_obj(target,FIND_OBJ_INV|FIND_OBJ_ROOM, ch))){
      act("You don't see that anywhere!", TRUE,ch,o,NULL,TO_CHAR,NULL);
      return false;
    }

    if(!isname("caughtfish", fish->name))
      return false;

    buf = format("$n points $p at %s.") % fish->shortDescr;
    act(buf,TRUE,ch,o,NULL, TO_ROOM,NULL);
    buf = format("You point $p at %s.") % fish->shortDescr;
    act(buf,TRUE,ch,o,NULL, TO_CHAR,NULL);

    buf = format("$p makes panicked fishy noises as its lifeforce is absorbed by %s!") % o->shortDescr;
    act(buf, TRUE,ch,fish,NULL,TO_ROOM,NULL);
    buf = format("$p makes panicked fishy noises as its lifeforce is absorbed by %s!") % o->shortDescr;
    act(buf, TRUE,ch,fish,NULL,TO_CHAR,NULL);



    delete fish;
    o->setMaxStructPoints(o->getMaxStructPoints()+5);
    o->setStructPoints(o->getStructPoints()+5);
    

    return true;
  }


  return false;
}


int fortuneCookie(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  if(!ch || !o || cmd != CMD_OBJ_OPENED)
    return false;

  std::vector <sstring> fortunes;
  fortunes.push_back("This paper is better than you in every way, because it can achieve Zen.  You, on the other hand, cannot.  And thus you must resume your misguided existence and continue on forever envious of this little bit of bleached wood pulp.\n\r");
  fortunes.push_back("The greatest danger could be your stupidity.\n\r");
  fortunes.push_back("Our first and last love is... self love.\n\r");
  fortunes.push_back("You have an unusual equipment for success, use it properly.\n\r");
  fortunes.push_back("Because of your melodic nature, the moonlight never misses an appoinment.\n\r");
  fortunes.push_back("Never wear your best pants, when you go to fight for freedom.\n\r");
  fortunes.push_back("A starship ride has been promised to you by the galactic wizzard.\n\r");
  fortunes.push_back("What you left behind is more mellow than wine.\n\r");
  fortunes.push_back("Suppose you can get what you want...\n\r");
  fortunes.push_back("Alas!  The onion you are eating is someone else's water lily.\n\r");
  fortunes.push_back("Buy many dream boxes.  Ask a friend to select one.\n\r");
  fortunes.push_back("You will receive a fortune. (cookie)\n\r");
  fortunes.push_back("Don't behave with cold manners.\n\r");
  fortunes.push_back("Don't forget, you are always on our minds.\n\r");
  fortunes.push_back("Rest is a good thing, but boredom is its brother.\n\r");
  fortunes.push_back("You are filled with life's most precious treasure... hope!\n\r");
  fortunes.push_back("Don't ask, don't say.  Everything lies in silence.\n\r");
  fortunes.push_back("Trust your intuition.  The universe is your guiding light.\n\r");
  fortunes.push_back("This is a really lovely day.  Congratulations!\n\r");
  fortunes.push_back("Everything is not yet lost.\n\r");
  fortunes.push_back("Let there be magic in your smile and firmness in your handshake.\n\r");
  fortunes.push_back("You will enjoy the pleasures of life to the highest degree.\n\r");
  fortunes.push_back("Wealth and renown, a beautiful person and a happy marriage.\n\r");
  fortunes.push_back("Everything will come your way now.\n\r");
  fortunes.push_back("You will enjoy good health, you will be surrounded by luxury.\n\r");
  fortunes.push_back("Patience is the best remedy for every trouble.\n\r");
  fortunes.push_back("Including others in your life will bring you great happiness.\n\r");
  fortunes.push_back("You will win success in whatever calling you adopt.\n\r");
  fortunes.push_back("Your love life will be happy and harmonious.\n\r");
  fortunes.push_back("Let the world be filled with tranquility and good will.\n\r");
  fortunes.push_back("You will have a fine capacity for the enjoyment of life.\n\r");
  fortunes.push_back("Success in everything.\n\r");
  fortunes.push_back("Some think you handsome, others not.\n\r");
  fortunes.push_back("Among the lucky, you are the chosen one.\n\r");
  fortunes.push_back("A librarian will be the key to your success. \n\r");
  fortunes.push_back("We know where you live.\n\r");
  fortunes.push_back("You will need good reading material in approximately 15 minutes.\n\r");
  fortunes.push_back("Everyone's meal today is on you!\n\r");
  fortunes.push_back("A recent prison escapee that is sitting near by wants to love you long time.\n\r");
  fortunes.push_back("If you do something right once, someone will ask you to do it again.\n\r");
  fortunes.push_back("Nothing makes a person more productive than the last minute.\n\r");
  fortunes.push_back("The only thing we learn from history is that we learn nothing from history.\n\r");
  fortunes.push_back("The other line moves faster.\n\r");
  fortunes.push_back("He who steps on others to get to the top has good balance.\n\r");
  fortunes.push_back("Lots of folks confuse bad management with destiny.\n\r");
  fortunes.push_back("Show me a man who is a good loser and I'll show you a man who is playing golf with his boss.\n\r");
  fortunes.push_back("If we could sell our experiences for what they cost us we would all be millionaires.\n\r");
  fortunes.push_back("Our policy is: When in doubt, do the right thing.\n\r");
  fortunes.push_back("Even if you win the rat race, you're still a rat.\n\r");
  fortunes.push_back("No problem is so formidable that you can't just walk away from it.\n\r");
  fortunes.push_back("If ignorance is bliss, why aren't there more happy people?\n\r");
  fortunes.push_back("The cost of living hasn't affected its popularity.\n\r");
  fortunes.push_back("Carry the water for the one you love.\n\r");
  fortunes.push_back("Good seed don't grow on a barren ground.\n\r");
  fortunes.push_back("One fine day you will find your way across the River.\n\r");
  fortunes.push_back("You might get stranded on Easy Street.\n\r");
  fortunes.push_back("You will take your seat in the Royal Suite.\n\r");
  fortunes.push_back("Thats just the way it is, but don't you believe it.\n\r");
  fortunes.push_back("You will wind up king of the hill.\n\r");
  fortunes.push_back("You will find comfort a long way from your resting place.\n\r");
  fortunes.push_back("Round and round it goes, first to laugh is the last to know.\n\r");
  fortunes.push_back("We will all come together across the great divide.\n\r");
  fortunes.push_back("Wake up, you are the eyes of the world.\n\r");
  fortunes.push_back("You will see a sunlight moon and be home soon.\n\r");
  fortunes.push_back("You may listen to the silence and hear what you're looking for.\n\r");
  fortunes.push_back("Its nice to be here with all you good people.\n\r");
  fortunes.push_back("Let your spirit linger with the spider fingers.\n\r");
  fortunes.push_back("Be careful what you look for, you just might find it.\n\r");
  fortunes.push_back("Look deep and you might find, you like it when you cross the line.\n\r");

  sstring buf=fortunes[::number(0,fortunes.size()-1)];

  // create fortune
  TNote *fortune = createNote(mud_str_dup(buf));
  delete [] fortune->name;
  fortune->name = mud_str_dup("fortune paper strip small");
  delete [] fortune->shortDescr;
  fortune->shortDescr = mud_str_dup("<W>a fortune<1>"); 
  delete [] fortune->getDescr();
  fortune->setDescr(mud_str_dup("<W>A small strip of paper lies here.<1>"));

  // convert cookie to food
  TObj *cookie = makeNewObj(ITEM_FOOD);  // new food object
  if(o->equippedBy)
    ch->unequip(o->eq_pos);
  --(*o);  // remove from owner
  *cookie = *o;  // TObj assignment, copy values
  cookie->assignFourValues(3,0,0,0); // 3 food value, no flags
  cookie->swapToStrung();

  *ch += *cookie;
  *ch += *fortune;

  // there is a memory leak here, o is just left hanging
  // returning DELETE_ITEM doesn't work as it should.

  buf = format("$n tears open $p and pulls out %s.") % fortune->shortDescr;
  act(buf,TRUE,ch,cookie,NULL, TO_ROOM,NULL);
  buf = format("You tear open $p and pull out %s.") % fortune->shortDescr;
  act(buf,TRUE,ch,cookie,NULL, TO_CHAR,NULL);

  return DELETE_ITEM;
}


int lycanthropyCure(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if (cmd != CMD_EAT || !ch)
    return FALSE;

  if(!isname(arg, o->name))
    return FALSE;

  if(!ch->hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE) &&
     !ch->hasQuestBit(TOG_LYCANTHROPE))
    return FALSE;

  ch->remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
  ch->remQuestBit(TOG_LYCANTHROPE);

  int returnVal = FALSE;
  if(ch->desc && ch->desc->original) {
    TBeing *per = ch->desc->original;
    ch->doReturn("", WEAR_NOWHERE, CMD_RETURN);
    act("A whispy wolf-like form detaches itself from your body and then dissipates.", TRUE, per, NULL, NULL, TO_CHAR, NULL);
    act("A whispy wolf-like form detaches itself from $n's body and then dissipates.",
      TRUE, per, NULL, NULL, TO_ROOM, NULL);
    returnVal = 1; // since we doReturn, we must stop all interaction with ch (it got modified)
  } else {

    act("A whispy wolf-like form detaches itself from your body and then dissipates.", TRUE, ch, NULL, NULL, TO_CHAR, NULL);
  act("A whispy wolf-like form detaches itself from $n's briefly and then dissipates.",
      TRUE, ch, NULL, NULL, TO_ROOM, NULL);
  }

  return returnVal;  // return false here so that the glop goes away
}


// proc to join keys for vella island quest
// joins the two held keys
// star 27561 + sphere 27707 => doesn't do anything
// star 27561 + pyramidal 27706 => star pyramidal 27706
// pyramidal + sphere => sphere pyramidal 27705
// sphere-pyr + star => final key 27704
// star-pyr + sphere => final key

int vellaKeyJoin(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *) {
  // joins the two held keys
  sstring buf;
  if (cmd != CMD_COMBINE) 
    return FALSE;
  buf=sstring(arg).word(0);
  if (buf != "keys")
    return FALSE;
 
  if (!o || !ch || !(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
    
  TObj *key1 = NULL;
  TObj *key2 = NULL;
  TObj *linked_key = NULL;

  std::map <int, short int> vnumToVec;
  std::map <short int, int> vecToVnum;
  
  vnumToVec[27561] = 1;
  vnumToVec[27705] = 6;
  vnumToVec[27706] = 5;
  vnumToVec[27707] = 2;
  vnumToVec[27708] = 4;
  vnumToVec[27704] = 7;
  vecToVnum[1] = 27561;
  vecToVnum[6] = 27705;
  vecToVnum[5] = 27706;
  vecToVnum[2] = 27707;
  vecToVnum[4] = 27708;
  vecToVnum[7] = 27704;

  if (!(key1 = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT])) ||
      vnumToVec.find(obj_index[key1->getItemIndex()].virt) == vnumToVec.end() ||
      !(key2 = dynamic_cast<TObj *>(ch->equipment[HOLD_LEFT])) ||
      vnumToVec.find(obj_index[key2->getItemIndex()].virt) == vnumToVec.end())
  {
    act("You must hold keys that fit each other, one in each hand.",TRUE,ch,o,NULL,TO_CHAR,NULL);
    return TRUE;
  }
  //return quietly if the proc is firing from both hands
  if (o == ch->equipment[HOLD_LEFT])
    return TRUE;

// have a key in each hand, now do something if they match

  act("$n fiddles with some keys.",TRUE,ch,o,NULL,TO_ROOM,NULL);
  act("You fiddle with the keys.",TRUE,ch,o,NULL,TO_CHAR,NULL);

  short int keycombo;
  keycombo = vnumToVec[obj_index[key1->getItemIndex()].virt] +
      vnumToVec[obj_index[key2->getItemIndex()].virt];
      
  if (vecToVnum.find(keycombo) != vecToVnum.end())
    linked_key = read_object(vecToVnum[keycombo], VIRTUAL);
  else {
     act("You can't seem to fit these keys together.",TRUE,ch,o,NULL,TO_CHAR,NULL);
     return TRUE;
  }

  act("Click! $n fits two keys together to make one.",TRUE,ch,o,NULL,TO_ROOM,NULL);
  act("Click! You fit the two keys together.",TRUE,ch,o,NULL,TO_CHAR,NULL);

  delete key1;
  delete key2;
ch->equipChar(linked_key, ch->getPrimaryHold(), SILENT_YES);

  return TRUE;
}

int fillBucket (TBeing *me, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  sstring buf;
  if (cmd != CMD_FILL) 
    return FALSE;
  buf=sstring(arg).word(0);
  if (buf != "bucket")
    return FALSE;

  TDrinkCon *bucket=dynamic_cast<TDrinkCon *>(o);
  if (!o) 
    return FALSE;

  if (!me->hasHands()) {
    me->sendTo(COLOR_OBJECTS, "I'm afraid you need hands to manipulate the bucket.\n\r");
    return TRUE;
  }
  if (me->bothArmsHurt()) {
    me->sendTo(COLOR_OBJECTS, "I'm afraid you need working arms to manipulate the bucket.\n\r");
    return TRUE;
  }

  act("You throw the bucket into the well and crank it back up again, full of fresh water.", TRUE, me, bucket, 0, TO_CHAR);
  act("$n throws the bucket into the well and cranks it back up again.", TRUE, me, bucket, 0, TO_ROOM);
  
  int water;
  if ((water = (bucket->getMaxDrinkUnits() - bucket->getDrinkUnits())) > 0)
  {
    bucket->setDrinkType(LIQ_WATER);
    bucket->addToDrinkUnits(water);
    bucket->weightChangeObject( water * SIP_WEIGHT);
  }

  return TRUE;
}
    
int statueArmTwist(TBeing *me, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if (cmd != CMD_TWIST)
    return FALSE;
  sstring buf = sstring(arg);
  TObj *packet = NULL;
  if (buf.word(0) == "arm" && buf.word(1) == "off") {
    if (!(packet = read_object(15969, VIRTUAL))) {
      vlogf(LOG_PROC, "Error reading object 15969 in proc statueArmTwist");
      return TRUE;
    }
    act("You manage to move the arm of the statue a bit.", TRUE, me, o, 0, TO_CHAR);
    act("$n fiddles with the arm of the statue.", TRUE, me, o, 0, TO_ROOM);
    act(format("%s falls out of the join between the arm and the statue.") % 
        packet->getName(), TRUE, me, o, 0, TO_CHAR);
    act(format("%s falls out of the join between the arm and the statue.") % 
        packet->getName(), TRUE, me, o, 0, TO_ROOM);
    *me->roomp += *packet;
    act("The statue was not well made and starts to crumble as you watch.", TRUE, me, 0, 0, TO_CHAR);
    act("The statue was not well made and starts to crumble as you watch.", TRUE, me, 0, 0, TO_ROOM);
    if(!o->makeScraps())
      delete o;
    return TRUE;
  }
  return FALSE;
}

/* Intended use:
 * If a PC is holding an object with this proc, it will allow him/her to
 * switch into the mob with the same vnum
 *
 * This proc has attendant code in immortal.cc
 */
int switchObject(TBeing *me, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  if (!cmd || cmd != CMD_SWITCH || !me || !o)
    return FALSE;
  if (!(me = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  
// This just makes it easier to keep track of these items, and to check for
// the item quickly later in doSwitch in immortal.cc 
  if (o->eq_pos != WEAR_NECK)
  {
    me->sendTo("The switch proc should not be on an item that is not neckwear.  Bug an immort.\n\r");
    return TRUE;
  }
  
  // I hope there's a better way to do this, but I don't have the time to
  // look for it/make it
  TBeing *mob = read_mobile(obj_index[o->getItemIndex()].virt, VIRTUAL);
  const char * mob_name = add_bars(mud_str_dup(mob->name)).c_str();
  delete mob;
  me->doSwitch(mob_name);
  
  return TRUE;
}  

int pirateHatDispenser(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBaseContainer *tbc=dynamic_cast<TBaseContainer *>(o);

  if(cmd!=CMD_PULL)
    return FALSE;
  
  if(!tbc || !ch || !arg)
    return FALSE;

  if(!isname(arg, o->name)){
    return FALSE;
  }

  if(!tbc->stuff.empty()){
    ch->sendTo("The dispenser is full already.\n\r");
    return TRUE;
  }
    

  TObj *pirate_hat=read_object(19015, VIRTUAL);

  act("You pull the handle on $p.",
      TRUE, ch, o, 0, TO_CHAR);
  act("You hear gears turning, then there is a soft thud as $p lands in the dispensing slot.", 
      TRUE, ch, pirate_hat, 0, TO_CHAR);

  act("$n pulls the handle on $p",
      TRUE, ch, o, 0, TO_ROOM);
  act("There is a soft thud as $p lands in the dispensing slot.",
      TRUE, ch, pirate_hat, 0, TO_ROOM);

  *tbc += *pirate_hat;
  return true;
}

int regeneration(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  if (!o)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)

  if(cmd == CMD_GENERIC_PULSE)
    ch->addToHit(max(1, (int)(ch->hitGain()/10.0)));

  return FALSE;
}

int stickerBush(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  if (cmd != CMD_OBJ_MOVEMENT)
    return FALSE;

  act("You get scratched up by $p.", TRUE, ch, o, 0, TO_CHAR);
  act("$n gets scratched up by $p.", TRUE, ch, o, 0, TO_ROOM);

  int rc = ch->reconcileDamage(ch, ::number(1,3), DAMAGE_NORMAL);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  return TRUE;
}

int rechargingWand(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TWand *tw;

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if(!(tw=dynamic_cast<TWand *>(o)))
    return FALSE;

  if(::number(0,99))
      return FALSE;

  if(tw->getCurCharges() < tw->getMaxCharges())
    tw->addToCurCharges(1);

  return TRUE;
}



int skittishObject (TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{	
	if (cmd != CMD_GENERIC_PULSE && cmd != CMD_OBJ_GOTTEN)
		return FALSE;
	
	if (cmd == CMD_GENERIC_PULSE){
		// if item on ground -> wander
		// if item in inventory -> jump free
		// if item equipped -> wiggle
		// if item in a carried, closed container -> wiggle
		// if item in open container (carried or in room) -> jump free
		
		if (::number(0, 5))
			return FALSE;
		
		sstring msg;
		dirTypeT use_dir;
		if (!o)
      return FALSE;
    
    sstring oname(o->getName());
    
		if (o->roomp){
			// #### on the ground -> wander the object

			// find permissible exits - must be open
			std::vector <dirTypeT> possible_exits;
			for(use_dir = MIN_DIR; use_dir < MAX_DIR; use_dir++){
				if (o->roomp->exitDir(use_dir) && !IS_SET(o->roomp->exitDir(use_dir)->condition, EX_CLOSED))
					possible_exits.push_back(use_dir);
			}
			
			if (!possible_exits.size()){
				msg = format("$p spins around in a circle.");
				act(msg, FALSE, o, o, 0, TO_ROOM);
				return FALSE;
			}
			
			// grab a random exit and move through it
			use_dir = possible_exits[::number(0, possible_exits.size() - 1)];
			TRoom *rp2 = real_roomp(o->roomp->exitDir(use_dir)->to_room);
			
			if (!rp2)
			  return FALSE;
			
      if (rp2->isFlyingSector()
          || rp2->isUnderwaterSector()
          || rp2->isAirSector()) {
				msg = format("$p spins around in a circle.");
				act(msg, FALSE, o, o, 0, TO_ROOM);
				return FALSE;
      }
      
			// movement out of room
			if (o->roomp->isWaterSector()){
				msg = format("$p swims %s.") % dirs[use_dir];
			} else {
				msg = format("$p skitters %s.") % dirs[use_dir];
			}
			act(msg, FALSE, o, o, 0, TO_ROOM);
			
			--(*o);
			*rp2 += *o;
			
			// movement into room
			if (o->roomp->isWaterSector() || o->roomp->isUnderwaterSector()){
				msg = format("$p swims in from the %s.") % dirs[rev_dir[use_dir]];
			} else {
				msg = format("$p skitters in from the %s.") % dirs[rev_dir[use_dir]];
			}
			act(msg, FALSE, o, o, 0, TO_ROOM);
			
			return FALSE;
		}
		
		TBeing *ch2;
		
		if (o && (ch2 = dynamic_cast<TBeing *>(o->equippedBy))){
			// ########## equipped -> wriggle
			act("Your $o wriggles around frantically, but cannot break free.", FALSE, ch2, o, 0, TO_CHAR, NULL);
			return FALSE;
		}
		
		
		if (o && (ch2 = dynamic_cast<TBeing *>(o->parent))){
			// ############ in someone's inventory -> jump out
			
			if (::number(0, 2)){
				act("Your $o wriggles around frantically, but cannot escape.", FALSE, ch2, o, 0, TO_CHAR, NULL);
			} else {
				act("$p wriggles free of $n and falls to the $g!", FALSE, ch2, o, 0, TO_ROOM, NULL);
				act("$p wriggles free and falls to the $g!", FALSE, ch2, o, 0, TO_CHAR, NULL);
				
				--(*o);
				*ch2->roomp += *o;
			}
			return FALSE;
		}
		
		TBaseContainer *container;
		TOpenContainer *open_container;
		
		if (o && (container = dynamic_cast<TBaseContainer *>(o->parent))){
			// ######### in a container somewhere
			
			if ((ch = dynamic_cast<TBeing *>(container->parent))){
				// container is on someone
				
				if ((open_container = dynamic_cast<TOpenContainer *>(container))){
					// closable container
					if (open_container->isClosed() || ::number(0, 1)){
						// container closed -> just wriggle
						act("Something is wriggling around in your $o.", FALSE, ch, open_container, 0, TO_CHAR, NULL);
						return FALSE;
					} else {
						// container is open -> jump out
						msg = format("$p leaps from your %s and falls to the $g!") % fname(open_container->name);
						act(msg, FALSE, ch, o, 0, TO_CHAR, NULL);
						act("$p wriggles itself free from $n and falls to the $g!", FALSE, ch, o, 0, TO_ROOM, NULL);
						
						--(*o);
						*ch->roomp += *o;
						return FALSE;
					}
				} else {
					// this means its an always open container? -> jump out
					if (::number(0, 1)){
						msg = format("$p leaps from your %s and falls to the $g!") % fname(container->name);
						act(msg, FALSE, ch, o, 0, TO_CHAR, NULL);
						act("$p wriggles itself free from $n and falls to the $g!", FALSE, ch, o, 0, TO_ROOM, NULL);
						
						--(*o);
						*ch->roomp += *o;
						return FALSE;
					} else {
						act("Something is wriggling around in your $s.", FALSE, ch, container, 0, TO_CHAR, NULL);
						return FALSE;
					}
				}
			} else if (container->roomp && ::number(0, 1)){
				// container is on ground -> jump out if open
				if ((open_container = dynamic_cast<TOpenContainer *>(container))){
					if (!open_container->isClosed()){
						// container is open -> jump out
						act("$p wriggles itself free from $n and falls to the $g!", FALSE, open_container, o, 0, TO_ROOM, NULL);
						--(*o);
						*open_container->roomp += *o;
						return FALSE;
					}
				} else {
					// this means its an always open container? -> jump out
					act("$p wriggles itself free from $n and falls to the $g!", FALSE, container, o, 0, TO_ROOM, NULL);
					
					--(*o);
					*container->roomp += *o;
					return FALSE;
				}
			}
			
			return FALSE;
		}
		// container parent is not TBeing
		return FALSE;
		
	}
	

	if (cmd == CMD_OBJ_GOTTEN && ch){
		if (::number(0, 2))
			return FALSE;
		
		// escape!!!
		act("$p wriggles free from $n's grasp and falls to the $g!", FALSE, ch, o, 0, TO_ROOM, NULL);
		act("$p wriggles free of your grasp and falls to the $g!", FALSE, ch, o, 0, TO_CHAR, NULL);
		
		--(*o);
		*ch->roomp += *o;
		
		return FALSE;
	}
	
	return FALSE;
}

int dwarfPower(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  // item can only be equipped by dwarves and harms any elves that attempt to equip it
  // if applied to a weapon, does a bone breaker thing to elven victims
  // if applied to a wearable item, will occasionally knock the wearer's opponent down (if opponent is elven)
  
  TBeing *ch;
  
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  
  if (ch->getRace() != RACE_DWARF) {
    if (ch->getRace() == RACE_ELVEN || ch->getRace() == RACE_DROW) {
      // give them a little damage, but don't kill them
      // then drop the item
      act("A cold, invisible force crushes your hand as you grip $p.", FALSE, ch, o, 0, TO_CHAR, NULL);
      act("Your hand throbs with pain as $p falls from your grasp!", FALSE, ch, o, 0, TO_CHAR, NULL);
      act("Wincing, $n loses $s grip on $p and it falls to the $g.", FALSE, ch, o, 0, TO_ROOM, NULL);
      ch->addToHit(-::number(1, 5));
      if (ch->getHit() <= 0) {
        ch->setHit(0);
        ch->setPosition(POSITION_STUNNED);
		act("You are stunned!", FALSE, ch, 0, 0, TO_CHAR, NULL);
		act("$n is stunned!", FALSE, ch, 0, 0, TO_ROOM, NULL);
      }
    } else {
      // just drop the item
      act("A cold, invisible force pries your fingers from $p.", FALSE, ch, o, 0, TO_CHAR, NULL);
      act("$p falls from your grasp!", FALSE, ch, o, 0, TO_CHAR, NULL);
      act("$n loses $s grip on $p and it falls to the $g.", FALSE, ch, o, 0, TO_ROOM, NULL);
    }
    *ch->roomp += *ch->unequip(o->eq_pos);
    return TRUE;
  }
  
  // other affects only trigger if vict is elven
  
  // bone break stuff - for weapons
  if (cmd == CMD_OBJ_HIT) {
    if (!vict || (vict->getRace() != RACE_ELVEN && vict->getRace() != RACE_DROW))
      return FALSE;
    
    // using weaponBreaker as a rough guideline: it triggers 2% of the time whether ch hits or not, and almost always finds a breakable limb
    // dwarfPower triggers after a successful hit on a breakable body part
    // so; assuming a 100% hit rate, that's about 1/3 of the breaks (considering 34% of hits land on breakable limbs - this might be wrong tho)
    // i'm going with an assumed hit rate of 80%, which is higher than is likely for any fight where the user doesn't entirely outclass the victim
    // if my math is anywhere near right, ch would have to land an average of 42 blows for a break, compared to 1 in 50 regardless of hitroll
    if (::number(0, 13))
      return FALSE;
    
    if (!ch->canBoneBreak(vict, SILENT_YES))
      return FALSE;
      
    wearSlotT part = wearSlotT(int(arg));
    
    if (notBreakSlot(part, FALSE))
      return FALSE;
    
    if (vict->isImmune(IMMUNE_BONE_COND, part))
      return FALSE;
    
    // ok, break that bone
    vict->addToLimbFlags(part, PART_BROKEN);
    
    sstring limb = vict->describeBodySlot(part);

    act("<o>You hear a muffled SNAP as $n<o>'s " + fname(o->name) + "<1> <Y>flashes<1> <r>angrily<1><o>.<1>", FALSE, ch, o, vict, TO_VICT, NULL);
    act("<o>Intense pain shoots through your " + limb + "!<1>\n\r<o>Your " + limb + " has been broken and is now useless!<1>", FALSE, vict, NULL, NULL, TO_CHAR, NULL);

    act("<o>Your " + fname(o->name) + "<1> <Y>flashes<1> <r>angrily<1> <o>upon contact with $N<o>'s " + limb + ".<1>", FALSE, ch, o, vict, TO_CHAR, NULL);
    
    act("<o>$n<o>'s " + fname(o->name) + "<1> <Y>flashes<1> <r>angrily<1> <o>upon contact with $N<o>'s " + limb + ".<1>", FALSE, ch, o, vict, TO_NOTVICT, NULL);

    act("<o>You hear a muffled SNAP as $n <o>clutches $s " + limb + " in pain!<1>", FALSE, vict, NULL, NULL, TO_ROOM, NULL);

    vict->dropWeapon(part);
    
    return TRUE;
  }
    
  // guardian spirit thing - intended for worn items or armor
  if (cmd == CMD_OBJ_BEEN_HIT) {
    if (!vict || (vict->getRace() != RACE_ELVEN && vict->getRace() != RACE_DROW))
      return FALSE;
      
    // making this up- behirHornItem triggers 1 in 4
    // success scales on opponent and item level, base 66% hit rate
    // this is also spammy and quirky enough to be annoying, so i'll just say...
    if (::number(0, 6))
      return FALSE;
    
	int rc;
	
    // ok, do a some kind of level check and try to knock them over or kick them while down
    TBaseClothing *armor;
    double item_level;
    
    if (armor = dynamic_cast<TBaseClothing *>(o)) {
      item_level = armor->armorLevel(ARMOR_LEV_REAL);
    } else {
      // this part is meant for worn gear
      return FALSE;
    }
	double base_chance = 66.7;
    int level_diff = (int) (item_level - vict->GetMaxLevel());
    int chance;
	if (level_diff == 0) {
	  chance = (int) base_chance;
	} else {
	  // coding out of my ass here, but if item is equal to victim in level give it a 66% chance of success
	  // a level 40 item should go auto success/fail against level 14/75 victims with this curve...
      double abs_diff = abs(level_diff);
	  chance = (int) (base_chance + ((double) level_diff * (log(abs_diff / 10.0+2) / ((item_level / abs_diff) * .75))));
      // never say never...
	  chance = min(95, chance);
	  chance = max(1, chance);
	}

    switch (::number(1, 3)) {
      case 1:
        act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr appears and quickly tags you.<1>", FALSE, ch, o, 0, TO_CHAR, NULL);
        act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr appears and quickly tags $n<1><o>.<1>", FALSE, ch, o, 0, TO_ROOM, NULL);
        break;
      case 2:
        act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr appears and winks mightily at you.<1>", FALSE, ch, o, 0, TO_CHAR, NULL);
        act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr appears and winks mightily at $n<1><o>.<1>", FALSE, ch, o, 0, TO_ROOM, NULL);
        break;
      default:
        act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr appears and motions for you to stand back.<1>", FALSE, ch, o, 0, TO_CHAR, NULL);
        act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr appears and motions for $n<o> to stand back.<1>", FALSE, ch, o, 0, TO_ROOM, NULL);
        break;
    }

    // spirit's hitroll
	if (::number(1, 100) <= chance) {
      // spirit success - knock vict over or do a little damage if already down
      if (vict->getPosition() <= POSITION_SITTING || !vict->hasLegs()) {
	    act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr executes a<1> <c>flying<1> <o>knee drop on $n<1><o>!<1>", FALSE, vict, o, 0, TO_ROOM, NULL);
        act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr executes a<1> <c>flying<1> <o>knee drop on you!<1>", FALSE, ch, o, vict, TO_VICT, NULL);
		vict->addToWait(combatRound(1));
		rc = vict->reconcileDamage(vict, ::number(1, max((int) item_level / 3, 1)), SKILL_CHOP);
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          // spirit killed the vict
          act("<o>The<1> <k>wrathful spirit<1> <o>high-fives you and bursts into a swirl of<1> <Y>fiery<1> <r>cinders<1><o>.<1>", FALSE, ch, o, 0, TO_CHAR, NULL);
          act("<o>The<1> <k>wrathful spirit<1> <o>high-fives $n<1><o> and bursts into a swirl of<1> <Y>fiery<1> <r>cinders<1><o>.<1>", FALSE, ch, o, 0, TO_ROOM, NULL);
          ch->dropGas(6, GAS_SMOKE);
          return DELETE_VICT;
        }
	  } else {
	    act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr shoves $n<1><o> to the $g<1><o>!<1>", FALSE, vict, o, 0, TO_ROOM, NULL);
        act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr shoves you to the $g<1><o>!<1>", FALSE, ch, o, vict, TO_VICT, NULL);
        vict->setPosition(POSITION_SITTING);
        vict->addToWait(2 * combatRound(1));
        vict->cantHit += vict->loseRound(0.6);
	  }
      if (vict->riding) {
        rc = vict->fallOffMount(vict->riding, POSITION_SITTING, FALSE);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          // spirit killed the vict
          act("<o>The<1> <k>wrathful spirit<1> <o>high-fives you and bursts into a swirl of<1> <Y>fiery<1> <r>cinders<1><o>.<1>", FALSE, ch, o, 0, TO_CHAR, NULL);
          act("<o>The<1> <k>wrathful spirit<1> <o>high-fives $n<1><o> and bursts into a swirl of<1> <Y>fiery<1> <r>cinders<1><o>.<1>", FALSE, ch, o, 0, TO_ROOM, NULL);
          ch->dropGas(6, GAS_SMOKE);
          return DELETE_VICT;
        }
      }
      vict->addToDistracted(1, FALSE);
      act("<o>The<1> <k>wrathful spirit<1> <o>flips its beard at $n<1><o> and bursts into a swirl of<1> <Y>fiery<1> <r>cinders<1><o>.<1>", FALSE, vict, o, 0, TO_ROOM, NULL);
      act("<o>The<1> <k>wrathful spirit<1> <o>flips its beard at you and bursts into a swirl of<1> <Y>fiery<1> <r>cinders<1><o>.<1>", FALSE, ch, o, vict, TO_VICT, NULL);
      vict->dropGas(6, GAS_SMOKE);
      
    } else {
      // spirit missed
      act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr hurls itself at $n<1><o>!<1>", FALSE, vict, o, 0, TO_ROOM, NULL);
      act("<o>The<1> <k>wrathful spirit<1> <o>of a dwarven martyr hurls itself at you!<1>", FALSE, ch, o, vict, TO_VICT, NULL);
      if (::number(0, 1)) {
        act("<o>The<1> <k>wrathful spirit<1> <o>careens wide and disappears in a cloud of<1> <k>smoke<1><o>.<1>", FALSE, vict, o, 0, TO_ROOM, NULL);
        act("<o>The<1> <k>wrathful spirit<1> <o>careens wide and disappears in a cloud of<1> <k>smoke<1><o>.<1>", FALSE, ch, o, vict, TO_VICT, NULL);
      } else {
        act("<o>The<1> <k>wrathful spirit<1> <o>caroms off $n<1><o> and dissolves into a plume of<1> <k>smoke<1><o>.<1>", FALSE, vict, o, 0, TO_ROOM, NULL);
        act("<o>The<1> <k>wrathful spirit<1> <o>caroms off you and dissolves into a plume of<1> <k>smoke<1><o>.<1>", FALSE, ch, o, vict, TO_VICT, NULL);
      }
      vict->addToDistracted(1, FALSE);
      vict->dropGas(12, GAS_SMOKE);
    }
    return TRUE;
  }
  return FALSE;
}

int mobSpawnGrab(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *cont)
{
  /* this proc spawns 3 guards when the item is taken from the table */
  /* for destiny's zone */
  if (cmd != CMD_OBJ_GOTTEN || !cont || !ch || !me)
    return FALSE;
  
#if 0 
/* screw it */
  // look for the proc flag in the name
  // it will contain the number of seconds since it was first grabbed, as recorded at the time
  sstring proc_flag = me->name;
  size_t found = proc_flag.find("[proc_152-");
  if ((int) found > -1) {
    size_t end = proc_flag.find("]", found + 1);
    proc_flag.assign(proc_flag, found + 10, end - 1);
    double grabbed = convertTo<double>(proc_flag);
    if (grabbed) {
      if ((double) time(NULL) - grabbed > 2629743) {
        // decay the object after one real month
        
        // equipped, inventory, on ground, on table, in a bag... yeesh
        if (ch) {
          act("Your $p goes poof.", FALSE, ch, me, 0, TO_CHAR, NULL);
          return TRUE;
        } else if (me->roomp) {
          // on the ground i guess
          act("$n goes poof.", FALSE, o, 0, 0, TO_ROOM);
        }
        
      }
    }
    return FALSE;
  }
#endif

  // check to see if it is indeed the right table they are getting the glove from
  int table_vnum = 32761; // diamond pedestal
  if (cont->objVnum() != table_vnum)
    return FALSE;
  
  // check to see if it has already been taken from the table
  sstring proc_flag = me->name;
  size_t found = proc_flag.find("[proc_152-");
  if ((int) found > -1)
    return FALSE;
  
  // first time grab, flag it
  me->name = mud_str_dup(format("%s [proc_152-%d]") % me->name % time(NULL));
  
  // spawn the 3 mobs
  int mob_vnum = 32762; // guard deranged spirit hobbit
  for (int loop = 0; loop < 3; loop = loop + 1) {
    TBeing *mob = read_mobile(mob_vnum, VIRTUAL);
    if (!mob) {
      vlogf(LOG_PROC, format("Proc mobSpawnGrab failing to grab spawn mob #%i in room %i.") % mob_vnum % ch->in_room);
      return FALSE;
    }
    *ch->roomp += *mob;
    colorAct(COLOR_MOBS, 
            ((mob->ex_description && mob->ex_description->findExtraDesc("repop")) ?
            mob->ex_description->findExtraDesc("repop") :
            "$n appears suddenly in the room."), TRUE, mob, 0, 0, TO_ROOM);
  }
  return TRUE;
}

struct rubiks_cube {
  char blue[3][3], yellow[3][3], red[3][3];
  char orange[3][3], white[3][3], green[3][3];
};

void rotate_side(char side[3][3], bool top_or_bottom)
{
  char buf[3][3];

  if(top_or_bottom){
    buf[2][0]=side[0][0];
    buf[1][0]=side[0][1];
    buf[0][0]=side[0][2];
    
    buf[2][1]=side[1][0];
    buf[1][1]=side[1][1];
    buf[0][1]=side[1][2];
    
    buf[2][2]=side[2][0];
    buf[1][2]=side[2][1];
    buf[0][2]=side[2][2];
   
    for(int i=0;i<3;++i)
      for(int j=0;j<3;++j)
	side[i][j]=buf[i][j];    
  } else {
    buf[0][2]=side[0][0];
    buf[1][2]=side[0][1];
    buf[2][2]=side[0][2];
    
    buf[0][1]=side[1][0];
    buf[1][1]=side[1][1];
    buf[2][1]=side[1][2];
    
    buf[0][0]=side[2][0];
    buf[1][0]=side[2][1];
    buf[2][0]=side[2][2];
    
    for(int i=0;i<3;++i)
      for(int j=0;j<3;++j)
	side[i][j]=buf[i][j];
  }


}

void twist_front_top(rubiks_cube *rc)
{
  char buf;

  buf=rc->white[0][2];
  rc->white[0][2]=rc->red[0][2];
  rc->red[0][2]=rc->green[2][0];
  rc->green[2][0]=rc->orange[2][0];
  rc->orange[2][0]=buf;

  buf=rc->white[1][2];
  rc->white[1][2]=rc->red[1][2];
  rc->red[1][2]=rc->green[1][0];
  rc->green[1][0]=rc->orange[1][0];
  rc->orange[1][0]=buf;
  
  buf=rc->white[2][2];
  rc->white[2][2]=rc->red[2][2];
  rc->red[2][2]=rc->green[0][0];
  rc->green[0][0]=rc->orange[0][0];
  rc->orange[0][0]=buf;
  
  rotate_side(rc->blue, true);
}

void twist_front_left(rubiks_cube *rc)
{
  char buf;

  buf=rc->white[0][2];
  rc->white[0][2]=rc->blue[0][0];
  rc->blue[0][0]=rc->green[0][2];
  rc->green[0][2]=rc->yellow[0][2];
  rc->yellow[0][2]=buf;

  buf=rc->white[0][1];
  rc->white[0][1]=rc->blue[1][0];
  rc->blue[1][0]=rc->green[0][1];
  rc->green[0][1]=rc->yellow[0][1];
  rc->yellow[0][1]=buf;

  buf=rc->white[0][0];
  rc->white[0][0]=rc->blue[2][0];
  rc->blue[2][0]=rc->green[0][0];
  rc->green[0][0]=rc->yellow[0][0];
  rc->yellow[0][0]=buf;

  rotate_side(rc->red, false);
}

void twist_front_right(rubiks_cube *rc)
{
  char buf;

  buf=rc->blue[2][0];
  rc->blue[2][0]=rc->red[2][0];
  rc->red[2][0]=rc->yellow[2][2];
  rc->yellow[2][2]=rc->orange[2][0];
  rc->orange[2][0]=buf;

  buf=rc->blue[2][1];
  rc->blue[2][1]=rc->red[2][1];
  rc->red[2][1]=rc->yellow[1][2];
  rc->yellow[1][2]=rc->orange[2][1];
  rc->orange[2][1]=buf;

  buf=rc->blue[2][2];
  rc->blue[2][2]=rc->red[2][2];
  rc->red[2][2]=rc->yellow[0][2];
  rc->yellow[0][2]=rc->orange[2][2];
  rc->orange[2][2]=buf;

  rotate_side(rc->white, false);
}



void twist_back_top(rubiks_cube *rc)
{
  char buf;

  buf=rc->orange[0][2];
  rc->orange[0][2]=rc->green[0][2];
  rc->green[0][2]=rc->red[2][0];
  rc->red[2][0]=rc->white[2][0];
  rc->white[2][0]=buf;

  buf=rc->orange[1][2];
  rc->orange[1][2]=rc->green[1][2];
  rc->green[1][2]=rc->red[1][0];
  rc->red[1][0]=rc->white[1][0];
  rc->white[1][0]=buf;
  
  buf=rc->orange[2][2];
  rc->orange[2][2]=rc->green[2][2];
  rc->green[2][2]=rc->red[0][0];
  rc->red[0][0]=rc->white[0][0];
  rc->white[0][0]=buf;
  
  rotate_side(rc->yellow, true);
}

void twist_back_left(rubiks_cube *rc)
{
  char buf;

  buf=rc->orange[0][2];
  rc->orange[0][2]=rc->yellow[0][0];
  rc->yellow[0][0]=rc->red[0][2];
  rc->red[0][2]=rc->blue[0][2];
  rc->blue[0][2]=buf;

  buf=rc->orange[0][1];
  rc->orange[0][1]=rc->yellow[1][0];
  rc->yellow[1][0]=rc->red[0][1];
  rc->red[0][1]=rc->blue[0][1];
  rc->blue[0][1]=buf;

  buf=rc->orange[0][0];
  rc->orange[0][0]=rc->yellow[2][0];
  rc->yellow[2][0]=rc->red[0][0];
  rc->red[0][0]=rc->blue[0][0];
  rc->blue[0][0]=buf;

  rotate_side(rc->green, false);
}

void twist_back_right(rubiks_cube *rc)
{
  char buf;

  buf=rc->yellow[2][0];
  rc->yellow[2][0]=rc->green[2][0];
  rc->green[2][0]=rc->blue[2][2];
  rc->blue[2][2]=rc->white[2][0];
  rc->white[2][0]=buf;

  buf=rc->yellow[2][1];
  rc->yellow[2][1]=rc->green[2][1];
  rc->green[2][1]=rc->blue[1][2];
  rc->blue[1][2]=rc->white[2][1];
  rc->white[2][1]=buf;

  buf=rc->yellow[2][2];
  rc->yellow[2][2]=rc->green[2][2];
  rc->green[2][2]=rc->blue[0][2];
  rc->blue[0][2]=rc->white[2][2];
  rc->white[2][2]=buf;

  rotate_side(rc->orange, false);
}


void randomize_cube(rubiks_cube *rc)
{
  // randomize
  for(int i=0;i<100;++i){
    switch(::number(0,5)){
      case 0:
	twist_front_top(rc);
	break;
      case 1:
	twist_front_left(rc);
	break;
      case 2:
	twist_front_right(rc);
	break;
      case 3:
	twist_back_top(rc);
	break;
      case 4:
	twist_back_left(rc);
	break;
      case 5:
	twist_back_right(rc);
	break;
    }
  }
}

// for the love I god I hope no one ever has to debug this proc
int rubiksCube(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *myself, TObj *)
{ 
  // I went with colors instead of orientation to make it easier for me
  // the names of the arrays only correspond to the solved cube in front
  // of me right now, but are otherwise meaningless
  rubiks_cube *rc=NULL;
  
  TChest *cube;

  if(!(cube=dynamic_cast<TChest *>(myself)))
    return false;

  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new rubiks_cube)) {
      perror("failed new of rubiks cube.");
      exit(0);
    }

    if(!(rc=(rubiks_cube *) myself->act_ptr)){
      perror("failed assign of rubiks cube.");
      exit(0);
    }
    
    for(int i=0;i<3;++i){
      for(int j=0;j<3;++j){
	rc->blue[i][j]='b';
	rc->yellow[i][j]='Y';
	rc->red[i][j]='r';
	rc->orange[i][j]='o';
	rc->white[i][j]='W';
	rc->green[i][j]='g';
      }
    }
    randomize_cube(rc);
    cube->addContainerFlag(CONT_LOCKED);
    cube->addContainerFlag(CONT_CLOSED);
  } else {
    if(!(rc=(rubiks_cube *) myself->act_ptr)){
      perror("failed assign of rubiks cube.");
      exit(0);
    }
  }
  

  if(cmd == CMD_TWIST){
    sstring buf = sstring(arg);

    if(!isname(buf.word(0), myself->name))
       return false;

    if(buf.word(1) == "front"){
      if(buf.word(2) == "top"){
	twist_front_top(rc);
      } else if(buf.word(2) == "left"){
	twist_front_left(rc);
      } else if(buf.word(2) == "right"){
	twist_front_right(rc);
      }

      ch->sendTo(COLOR_BASIC, "You give it a twist...\n\r");
    } else if(buf.word(1) == "back"){
      if(buf.word(2) == "top"){
	twist_back_top(rc);
      } else if(buf.word(2) == "left"){
	twist_back_left(rc);
      } else if(buf.word(2) == "right"){
	twist_back_right(rc);
      }

      ch->sendTo(COLOR_BASIC, "You give it a twist...\n\r");
    } else if(buf.word(1) == "mix"){
      randomize_cube(rc);
      cube->addContainerFlag(CONT_LOCKED);
      cube->addContainerFlag(CONT_CLOSED);
    } else if(buf.word(1) == "solve" && ch->isImmortal()){
      for(int i=0;i<3;++i){
	for(int j=0;j<3;++j){
	  rc->blue[i][j]='b';
	  rc->yellow[i][j]='Y';
	  rc->red[i][j]='r';
	  rc->orange[i][j]='o';
	  rc->white[i][j]='W';
	  rc->green[i][j]='g';
	}
      }
    } else {
      ch->sendTo(COLOR_BASIC, "Usage: twist puzzle <front|back> <top|left|right>\n\r");
    }

  }

  if((cmd==CMD_LOOK && isname(arg, myself->name)) || cmd==CMD_TWIST){
    // oh god my eyes my eyes my brain my brain
    ch->sendTo(COLOR_BASIC, 
	       format("       <%c>#<1>                   <%c>#<1>             \n\r") %
	       rc->blue[0][2] % rc->yellow[0][2]);
    ch->sendTo(COLOR_BASIC, 
	       format("    <%c>#<1>     <%c>#<1>             <%c>#<1>     <%c>#<1>         \n\r") %
	       rc->blue[0][1] % rc->blue[1][2] %
	       rc->yellow[0][1] % rc->yellow[1][2]);
    ch->sendTo(COLOR_BASIC, 
	       format("  <%c>#<1>    <%c>#<1>    <%c>#<1>         <%c>#<1>    <%c>#<1>    <%c>#<1>    \n\r") %
	       rc->blue[0][0] % rc->blue[1][1] % rc->blue[2][2] %
	       rc->yellow[0][0] % rc->yellow[1][1] % rc->yellow[2][2]);
    ch->sendTo(COLOR_BASIC, 
	       format("<%c>#<1>   <%c>#<1>     <%c>#<1>   <%c>#<1>     <%c>#<1>   <%c>#<1>     <%c>#<1>   <%c>#<1>\n\r") %
	       rc->red[0][2] % rc->blue[1][0] % rc->blue[2][1] % rc->white[2][2] %
	       rc->green[0][2] % rc->yellow[1][0] % rc->yellow[2][1] % rc->orange[2][2]);
    ch->sendTo(COLOR_BASIC, 
	       format("   <%c>#<1>   <%c>#<1>   <%c>#<1>           <%c>#<1>   <%c>#<1>   <%c>#<1>     \n\r") %
	       rc->red[1][2] % rc->blue[2][0] % rc->white[1][2] %
	       rc->green[1][2] % rc->yellow[2][0] % rc->orange[1][2]);
    ch->sendTo(COLOR_BASIC, 
	       format("<%c>#<1>     <%c>#<1> <%c>#<1>     <%c>#<1>     <%c>#<1>     <%c>#<1> <%c>#<1>     <%c>#<1>\n\r") %
	       rc->red[0][1] % rc->red[2][2] % rc->white[0][2] % rc->white[2][1] %
	       rc->green[0][1] % rc->green[2][2] % rc->orange[0][2] % rc->orange[2][1]);
    ch->sendTo(COLOR_BASIC, 
	       format("   <%c>#<1>       <%c>#<1>           <%c>#<1>       <%c>#<1>       \n\r") %
	       rc->red[1][1] % rc->white[1][1] %
	       rc->green[1][1] % rc->orange[1][1]);
    ch->sendTo(COLOR_BASIC, 
	       format("<%c>#<1>     <%c>#<1> <%c>#<1>     <%c>#<1>     <%c>#<1>     <%c>#<1> <%c>#<1>     <%c>#<1>\n\r") %
	       rc->red[0][0] % rc->red[2][1] % rc->white[0][1] % rc->white[2][0] %
	       rc->green[0][0] % rc->green[2][1] % rc->orange[0][1] % rc->orange[2][0]);
    ch->sendTo(COLOR_BASIC, 
	       format("   <%c>#<1>       <%c>#<1>           <%c>#<1>       <%c>#<1>       \n\r") %
	       rc->red[1][0] % rc->white[1][0] %
	       rc->green[1][0] % rc->orange[1][0]);
    ch->sendTo(COLOR_BASIC, 
	       format("      <%c>#<1> <%c>#<1>                 <%c>#<1> <%c>#<1>          \n\r") %
	       rc->red[2][0] % rc->white[0][0] %
	       rc->green[2][0] % rc->orange[0][0]);
    
    bool solved=true;
    
    for(int i=0;i<3;++i){
      for(int j=0;j<3;++j){
	if(rc->blue[i][j]!=rc->blue[0][0] ||
	   rc->red[i][j]!=rc->red[0][0] ||
	   rc->white[i][j]!=rc->white[0][0] ||
	   rc->yellow[i][j]!=rc->yellow[0][0] ||
	   rc->green[i][j]!=rc->green[0][0] ||
	   rc->orange[i][j]!=rc->orange[0][0])
	  solved=false;
      }
    }

    if(solved){
      ch->sendTo("The puzzle solved, the lock clicks open.\n\r");
      cube->remContainerFlag(CONT_LOCKED);
    }
    return true;
  }
  
  return false;
}


int liquidSource(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if(cmd!=CMD_GENERIC_PULSE)
    return false;
  
  TDrinkCon *tdc;

  if(!(tdc=dynamic_cast<TDrinkCon *>(o)))
    return false;

  if(!tdc->roomp)
    return false;

  if(::number(0,9))
    return false;


  sendrpf(COLOR_BASIC, tdc->roomp,
	  ((sstring)(format("Some %s flows out of %s.\n\r") % 
	   liquidInfo[tdc->getDrinkType()]->name %
	   tdc->getName())).c_str());

  tdc->roomp->dropPool(tdc->getDrinkUnits(), tdc->getDrinkType());

  return false;
}

int ieComputer(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
	// immortal exchange computer
	// should provide a comprehensive account of all coins made, redeemed
	
	if (!(cmd == CMD_SAY || cmd == CMD_SAY2)) {
		return FALSE;
	}
	
	if (!o->roomp || sstring(arg).word(0).lower() != "computron!") {
		return FALSE;
	}
	
	ch->doSay(arg);
	
	if (!ch->isImmortal() || !ch->hasWizPower(POWER_SET_IMP_POWER)) {
		act("$p intones, \"Computron does not suffer such fools.\"", FALSE, o, o, 0, TO_ROOM);
		act("$p dims slightly.", FALSE, o, o, 0, TO_ROOM);
	    return TRUE;
	}
	sstring arg2 = sstring(arg).word(1).lower();
	
	if (arg2 == "show") {
		sstring arg3 = sstring(arg).word(2).lower();
		if (is_abbrev(arg3, "created") || is_abbrev(arg3, "redeemed")) {
			TNote *note;
			TDatabase db(DB_SNEEZY);
			if (is_abbrev(arg3, "redeemed")) {
				sstring contents = "<o>Immortal Exchange Coin Redemption<1>\n\r";
				db.query("select p1.name as redeemed_for, p2.name as redeemed_by, count(*) as coins, date_format(now(), '%%M %%e %%Y %%l:%%i %%p') as date_printed from immortal_exchange_coin c1 left join player p1 on p1.id = c1.redeemed_for left join player p2 on c1.redeemed_by = p2.id group by p1.name, p2.name order by p1.name, p2.name;");
				if (db.fetchRow()) {
					contents += format("<o>as of<1> <c>%s<1>\n\r\n\r") % db["date_printed"];
					contents += format("<o>%-25s %-25s %8s<1>\n\r") % "Redeemed for" % "Redeemed by" % "Coins";
					contents += format("<c>%-25s %-25s %8s<1>\n\r") % db["redeemed_for"].cap() % db["redeemed_by"].cap() % db["coins"];
					while (db.fetchRow()) {
						contents += format("<c>%-25s %-25s %8s<1>\n\r") % db["redeemed_for"].cap() % db["redeemed_by"].cap() % db["coins"];
					}
				} else {
					contents += "\n\r<c>No coins on record!<1>\n\r";
				}
				note = createNote(mud_str_dup(contents));
			} else {
				sstring contents = "<g>Immortal Exchange Coin Distribution<1>\n\r";
				db.query("select p1.name as created_for, p2.name as created_by, count(*) as coins, date_format(now(), '%%M %%e %%Y %%l:%%i %%p') as date_printed from immortal_exchange_coin c1 left join player p1 on p1.id = c1.created_for left join player p2 on c1.created_by = p2.id group by p1.name, p2.name order by p1.name, p2.name;");
				if (db.fetchRow()) {
					contents += format("<g>as of<1> <c>%s<1>\n\r\n\r") % db["date_printed"];
					contents += format("<g>%-25s %-25s %8s<1>\n\r") % "Created for" % "Created by" % "Coins";
					contents += format("<c>%-25s %-25s %8s<1>\n\r") % db["created_for"].cap() % db["created_by"].cap() % db["coins"];
					while (db.fetchRow()) {
						contents += format("<c>%-25s %-25s %8s<1>\n\r") % db["created_for"].cap() % db["created_by"].cap() % db["coins"];
					}
				} else {
					contents += "\n\r<c>No coins on record!<1>\n\r";
				}
				note = createNote(mud_str_dup(contents));
			}
			act("A thin arm extends from $p.", FALSE, o, o, 0, TO_ROOM);
			act("$p gives a note to $n.", FALSE, ch, o, 0, TO_ROOM);
			act("$p gives you a <o>note<1>.", FALSE, ch, o, 0, TO_CHAR);
			*ch += *note;
			act("$p belches a puff of <k>smoke<1>.", FALSE, o, o, 0, TO_ROOM);
			o->dropGas(1, GAS_SMOKE);
			return TRUE;
		}
	}
	// we've fallen through, show viable options
	act("$p intones, \"I respond to <c>computron! show created<1>.\".", FALSE, o, o, 0, TO_ROOM);
	act("$p intones, \"I respond to <c>computron! show redeemed<1>.\".", FALSE, o, o, 0, TO_ROOM);
	act("You hear the sound of a spring breaking.", FALSE, o, o, 0, TO_ROOM);
	
	return TRUE;
}


//MARKER: END OF SPEC PROCS

extern int stickerBush(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int ballotBox(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int board(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int weaponBlinder(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int weaponManaDrainer(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int weaponLightningRod(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int weaponJambiyaSpecial(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int deikhanSword(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int blackSun(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int tequilaCutlass(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int poisonCutlass(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int holyCutlass(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int unholyCutlass(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int lotteryTicket(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int sweepsScratch(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int sweepsSplitJoin(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int graffitiMaker(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int graffitiObject(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int Gwarthir(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int warMaker(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int weaponDisruption(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int weaponFumbler(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int weaponBreaker(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int glowCutlass(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int poisonWhip(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int daggerOfHunting(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *);
extern int flameWeapon(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int frostWeapon(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int dragonSlayer(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int boneStaff(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int bloodspike(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int brokenBottle(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int wickedDagger(TBeing *vict, cmdTypeT cmd, const char *, TObj *me, TObj *ch_obj);
extern int poisonSap(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int daySword(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int nightBlade(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int scirenDrown(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int energyBeam(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int poisonViperBlade(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int weaponShadowSlayer(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int blazeOfGlory(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int elementalWeapon(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int vorpal(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int berserkerWeap(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int randomizer(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int bluntPierce(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int dualStyleWeapon(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int splinteredClub(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int sonicBlast(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int ghostlyShiv(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int frostSpear(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int iceStaff(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int weaponUnmaker(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int chromaticWeapon(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int fireballWeapon(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int gnomeTenderizer(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int marukalia(TBeing *targ, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int lightSaber(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int demonSlayer(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *);
extern int objWornAstralWalk(TBeing *targ, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int objWornMinorAstralWalk(TBeing *targ, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int objWornPortal(TBeing *targ, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int comboEQCast(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);
extern int ofManyPotions(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *);

// assign special procedures to objects

TObjSpecs objSpecials[NUM_OBJ_SPECIALS + 1] =
{
  {TRUE, "BOGUS", bogusObjProc},  // 0
  {TRUE, "ballot box", ballotBox},
  {TRUE, "bulletin board", board},
  {TRUE, "note dispenser", dispenser},
  {TRUE, "statue of feeding", statue_of_feeding},
  {FALSE, "pager", pager},          // 5
  {FALSE, "ear muffs", ear_muffs},
  {FALSE, "Jewel of Judgment", JewelJudgment},   
  {FALSE, "Gwarthir", Gwarthir},
  {FALSE, "vending machine", vending_machine},
  {FALSE, "Orb of Destruction", orbOfDestruction},   // 10
  {FALSE, "War Maker", warMaker},
  {TRUE, "Weapon: disruption", weaponDisruption},
  {TRUE, "Weapon: fumbler", weaponFumbler},
  {TRUE, "ladder", ladder},
  {TRUE, "regeneration", regeneration},      // 15
  {TRUE, "Weapon: bonebreaker", weaponBreaker},
  {FALSE, "Glowing Cutlass", glowCutlass},
  {TRUE, "poison whip", poisonWhip},
  {TRUE, "magic gills", magicGills},
  {FALSE, "recharging wand", rechargingWand},      // 20
  {FALSE, "rainbow bridge", rainbowBridge},
  {FALSE, "Hunting Dagger", daggerOfHunting}, 
  {TRUE, "flame weapon", flameWeapon}, 
  {TRUE, "frost weapon", frostWeapon}, 
  {TRUE, "food&drink", foodItem},       // 25
  {TRUE, "crystal ball", crystal_ball},
  {FALSE, "Orb of Teleportation", orbOfTeleportation},
  {FALSE, "Weather Armor", weatherArmor},
  {FALSE, "caravan wagon", caravan_wagon},
  {TRUE, "Bleed Chair", bleedChair},  // 30
  {TRUE, "Harm Chair", harmChair},
  {TRUE, "Dragon Slayer", dragonSlayer},
  {TRUE, "Blood Drainer", bloodDrain},
  {FALSE, "Stone Altar", stoneAltar},
  {FALSE, "Bone Staff", boneStaff},   // 35
  {FALSE, "Bloodspike", bloodspike}, 
  {FALSE, "behir horn item", behirHornItem},
  {FALSE, "Newbie Helper", newbieHelperWProc},
  {FALSE, "pirate hat dispenser", pirateHatDispenser},
  {FALSE, "Blood Bowl", bowl_of_blood},  // 40
  {FALSE, "feather fall", featherFallItem},
  {FALSE, "wicked dagger", wickedDagger},  
  {TRUE, "poison sap dagger", poisonSap},  
  {FALSE, "blinder weapon", weaponBlinder},
  {FALSE, "mana drain weapon", weaponManaDrainer}, // 45
  {FALSE, "tequila cutlass", tequilaCutlass},
  {FALSE, "daySword", daySword},  
  {FALSE, "nightBlade", nightBlade},  
  {TRUE, "Lightning Rod", weaponLightningRod},
  {FALSE, "Jambiya", weaponJambiyaSpecial}, // 50
  {TRUE, "Sciren's Suffocation", scirenDrown},
  {TRUE, "Energy Beam Weapon", energyBeam},
  {TRUE, "Viper Weapon (poison)", poisonViperBlade},
  {FALSE, "trolley", trolley},
  {FALSE, "Stone Skin Amulet", stoneSkinAmulet}, // 55
  {FALSE, "Razor Glove", razorGlove},
  {FALSE, "ShadowSlayer", weaponShadowSlayer},
  {FALSE, "Squirt Gun", squirtGun},
  {FALSE, "Glory Weapon", blazeOfGlory},
  {TRUE, "Elemental Weapon", elementalWeapon}, // 60
  {FALSE, "Life Leech Glove", lifeLeechGlove},
  {TRUE, "Mechanical Wings", mechanicalWings},
  {FALSE, "Key in Knife",  keyInKnife},
  {FALSE, "Teleport Vial", teleportVial},
  {FALSE, "Sun Circle Amulet", sunCircleAmulet}, // 65
  {FALSE, "Better Vender", vending_machine2},
  {FALSE, "Mine Cart", minecart},
  {FALSE, "Switchtrack", switchtrack},
  {FALSE, "vorpal", vorpal},
  {TRUE, "Berserker Weapon", berserkerWeap},// 70
  {FALSE, "Travel Gear", travelGear},
  {FALSE, "Maquahuitl", maquahuitl},
  {FALSE, "Randomizer", randomizer},
  {FALSE, "Blunt/Pierce", bluntPierce},
  {TRUE, "Dual Style Weapon", dualStyleWeapon}, //75
  {FALSE, "Mana Burn Robe", manaBurnRobe},
  {FALSE, "Chrism: minor heal", healingNeckwear},
  {FALSE, "Chrism: bless hold item", blessingHoldItem},
  {FALSE, "Chrism: vitality restore", moveRestoreNeckwear},
  {FALSE, "Chipped Tooth Food Item", chippedTooth}, // 80
  {FALSE, "Goofers Dust", goofersDust},
  {FALSE, "teleport ring", teleportRing},
  {FALSE, "self repairing", selfRepairing},
  {FALSE, "undead spewing portal", USPortal},
  {FALSE, "Amulet of Aeth Koralm", AKAmulet}, // 85
  {FALSE, "fire glove", fireGlove},
  {FALSE, "Shaman's Totem Mask", totemMask},
  {FALSE, "perma death monument", permaDeathMonument},
  {FALSE, "fishing boat", fishingBoat},
  {FALSE, "Splintered Club", splinteredClub}, // 90 
  {FALSE, "Suffocation Glove", suffGlove},
  {FALSE, "Force", force},
  {FALSE, "trophy board", trophyBoard},
  {FALSE, "shopinfo board", shopinfoBoard},
  {FALSE, "Sonic Blast", sonicBlast},  // 95
  {FALSE, "highrollers board", highrollersBoard},
  {FALSE, "faction score board", factionScoreBoard},
  {FALSE, "fragile arrow", fragileArrow},
  {FALSE, "Weapon: Starfire", starfire},
  {FALSE, "Sheath: Starfire", starfiresheath}, // 100
  {FALSE, "Teleport Rescue Item", teleportRescue},
  {FALSE, "Deikhan Sword", deikhanSword},
  {FALSE, "black sun", blackSun},
  {FALSE, "poison cutlass", poisonCutlass},
  {FALSE, "unholy cutlass", unholyCutlass},
  {FALSE, "ghostly shiv", ghostlyShiv},
  {FALSE, "hammer set: peke", HSPeke},
  {FALSE, "hammer set: copsi", HSCopsi},
  {FALSE, "hammer set: pendant", HSPendant}, 
  {FALSE, "blizzard ring", blizzardRing}, //110
  {FALSE, "frost spear", frostSpear},
  {FALSE, "ice staff", iceStaff},
  {FALSE, "heart of the arctic", arcticHeart},
  {FALSE, "frost armor", frostArmor}, 
  {FALSE, "telekinesis glove", telekinesisGlove}, //115
  {FALSE, "Symbol of the Blinding Light", symbolBlindingLight},
  {TRUE, "Weapon: Unmaker", weaponUnmaker},
  {TRUE, "Chromatic Weapon", chromaticWeapon},
  {FALSE, "spawning object: open", mobSpawnOpen},
  {FALSE, "Energy Shield: generator", energyShieldGenerator}, //120
  {FALSE, "Energy Shield: shield", energyShield},
  {FALSE, "teleporting object", teleportingObject},
  {FALSE, "holy cutlass", holyCutlass},
  {FALSE, "fortune cookie", fortuneCookie},
  {TRUE, "Fireball Weapon", fireballWeapon}, //125
  {FALSE, "Fire Shield", fireArmor},
  {FALSE, "Finns Gaff", finnsGaff},
  {FALSE, "stim pack", stimPack},
  {FALSE, "lottery ticket", lotteryTicket},
  {FALSE, "lycanthropy cure", lycanthropyCure}, // 130
  {FALSE, "vella key join", vellaKeyJoin},
  {FALSE, "Gnath well bucket", fillBucket},
  {FALSE, "sweeps scratch", sweepsScratch},
  {FALSE, "sweeps split join", sweepsSplitJoin},
  {FALSE, "graffiti maker", graffitiMaker}, // 135
  {FALSE, "graffiti object", graffitiObject},
  {FALSE, "statue arm twist", statueArmTwist},
  {FALSE, "sticker bush", stickerBush},
  {FALSE, "switch object", switchObject},
  {FALSE, "gnome tenderizer", gnomeTenderizer}, // 140
  {FALSE, "Marukalia", marukalia},
  {FALSE, "lightsaber", lightSaber},
  {FALSE, "broken bottle", brokenBottle},
  {FALSE, "Demon Slayer", demonSlayer},
  {FALSE, "Astral Walk", objWornAstralWalk}, //145
  {FALSE, "Minor Astral Walk", objWornMinorAstralWalk},
  {FALSE, "Portal", objWornPortal},
  {FALSE, "brick quest scorecard", brickScorecard},
  {FALSE, "EQ Combo Casting", comboEQCast},
  {FALSE, "Skittish Object", skittishObject}, //150
  {FALSE, "Dwarf Power", dwarfPower}, 
  {FALSE, "Mob Spawn Grab", mobSpawnGrab}, 
  {FALSE, "rubik's cube", rubiksCube},
  {FALSE, "Immortal Exchange Computer", ieComputer}, 
  {FALSE, "liquid source", liquidSource}, // 155
  {FALSE, "of many potions", ofManyPotions},
  {FALSE, "last proc", bogusObjProc}
};
