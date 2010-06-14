#include "room.h"
#include "low.h"
#include "monster.h"
#include "disease.h"
#include "spelltask.h"
#include "combat.h"
#include "disc_hand_of_god.h"
#include "being.h"
#include "obj_food.h"
#include "obj_portal.h"
#include "obj_drinkcon.h"
#include "person.h"
#include "obj_magic_item.h"
#include "materials.h"

static void moveLoss(TBeing &ch)
{ 
  // used by recall and summon

  // take up to 100 move
  // however, leave them with at least 10 mv
  ch.addToMove(max(-100, -(ch.getMove()-10)));
}

int astralWalk(TBeing * caster, TBeing * victim, int level, short bKnown)
{
  int rc;
  int ret,location;
  TRoom *room = NULL;
  TMonster *tmon;

  location = victim->in_room;
  room = real_roomp(location);

  if (!room) {
    vlogf(LOG_BUG, "Attempt to astral to a NULL room.");
    return SPELL_FAIL;
  }


  if (room->isFlyingSector() || caster->roomp->isFlyingSector()) {
    act("$d refuses to let you astral walk through the magic",
             FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (caster->roomp->isRoomFlag(ROOM_NO_ESCAPE) &&
      !caster->isImmortal()) {
    act("$d refuses to let you astral walk from here.",
             FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  

  if (victim->GetMaxLevel() > MAX_MORT ||
      room->isRoomFlag(ROOM_PRIVATE) ||
      room->isRoomFlag(ROOM_HAVE_TO_WALK) ||
      (zone_table[room->getZoneNum()].enabled == FALSE) ||
      (toggleInfo[TOG_QUESTCODE2]->toggle && 
       room->number >= 5700 && room->number < 5900) ||
      room->isRoomFlag(ROOM_NO_MAGIC)) {
    act("$d refuses to let you astral walk there.", FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }


  if((tmon=dynamic_cast<TMonster *>(victim)) && 
     ((tmon->mobVnum()==Mob::TIGER_SHARK) || (tmon->mobVnum()==Mob::ELEPHANT))){
    act("$d refuses to let you astral walk there.", FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;  
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_ASTRAL_WALK)) {
    ret=SPELL_SUCCESS;

    switch (::number(1,2)) {
      case 1:
        act("$d opens a door to another dimension and you step through.", 
                FALSE, caster, NULL, NULL, TO_CHAR);
        act("$d opens a door to another dimension and $n steps through!", 
                FALSE, caster, NULL, NULL, TO_ROOM);
        break;
      case 2:
        act("$d opens a door to another dimension and guides you through.", 
                FALSE, caster, NULL, NULL, TO_CHAR);
        act("$d opens a door to another dimension and guides $n through!", 
                FALSE, caster, NULL, NULL, TO_ROOM);
        break;
    }

    caster->roomp->playsound(SOUND_SPELL_ASTRAL_WALK, SOUND_TYPE_MAGIC);

  } else {
    switch (critFail(caster, SPELL_ASTRAL_WALK)) {
      case CRIT_F_HITOTHER:

        act("$d opens a door to another dimension and $n steps through!", 
            FALSE, caster, NULL, NULL, TO_ROOM);
        act("$d opens a door to another dimension and you step through.", 
            FALSE, caster, NULL, NULL, TO_CHAR);
        act("Uh oh. You have a funny feeling that door didn't lead where you wanted it to.", 
           FALSE, caster, NULL, NULL, TO_CHAR);

        caster->genericTeleport(SILENT_YES);
        room = caster->roomp;
      case CRIT_F_HITSELF:
        if (!victim->isImmortal() &&
            caster->isNotPowerful(victim, level, SPELL_SUMMON, SILENT_YES) &&
            victim->isSummonable()) {
          act("$d opens a path to another dimension but $n is unable to step through it!",
              FALSE, caster, NULL, NULL, TO_ROOM);
          act("$d opens a path to another dimension but you are unable to step through.",
              FALSE, caster, NULL, NULL, TO_CHAR);
          act("Uh oh. You have a funny feeling that door doesn't do what you intended.",
              FALSE, caster, NULL, victim, TO_CHAR);
          act("You feel $N being summoned by $d to your presence!",
              FALSE, caster, NULL, victim, TO_CHAR);
          rc = caster->rawSummon(victim);
          if (IS_SET_DELETE(rc, DELETE_VICT) && IS_SET_DELETE(rc, DELETE_THIS))
            return SPELL_CRIT_FAIL | VICTIM_DEAD | CASTER_DEAD;
          if (IS_SET_DELETE(rc, DELETE_VICT))
            return SPELL_CRIT_FAIL | VICTIM_DEAD;
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return SPELL_CRIT_FAIL | CASTER_DEAD;
          return SPELL_CRIT_FAIL;
        } else {
// pass through
        }
      default:        
        act("Nothing seems to happen!",
            FALSE, caster, NULL, NULL, TO_ROOM);
        act("Nothing seems to happen.",
            FALSE, caster, NULL, NULL, TO_CHAR);
        ret=SPELL_FAIL;
        return ret;
    }
  }

  --(*caster);
  *room += *caster;
  caster->doLook("", CMD_LOOK);

  act("You are blinded for a moment as $n appears in a flash of light!", 
          FALSE, caster, NULL, NULL, TO_ROOM);

  if (caster->riding) {
    rc = caster->riding->genericMovedIntoRoom(room, -1);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete caster->riding;
      caster->riding = NULL;
    }
  } else {
    rc = caster->genericMovedIntoRoom(room, -1);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ret |= CASTER_DEAD;
  }
  return ret;
}

int astralWalk(TBeing * caster, TBeing * victim)
{
  int ret,level;
  int rc = 0;

  if (!bPassClericChecks(caster,SPELL_ASTRAL_WALK))
    return FALSE;

  level = caster->getSkillLevel(SPELL_ASTRAL_WALK);

  ret=astralWalk(caster,victim,level,caster->getSkillValue(SPELL_ASTRAL_WALK));

  if (IS_SET(ret, CASTER_DEAD)) {
    ADD_DELETE(rc, DELETE_THIS);
  }
  return rc;
}

int createFood(TBeing *c, int level, short bKnown, spellNumT spell)
{
  TFood *o;
  int fill_amt;

  if (c->bSuccess(bKnown, c->getPerc(), spell)) {
    o = new TFood();

    fill_amt = 4 + (level/3);

    if (level < 15) {
      o->name = mud_str_dup("wafer food holy small");
      o->shortDescr = mud_str_dup("a small holy wafer");
      o->setDescr(mud_str_dup("A small holy wafer lies here."));
      o->setWeight(.1);
    } else if (level < 35) {
      o->name = mud_str_dup("wafer food holy medium");
      o->shortDescr = mud_str_dup("a medium holy wafer");
      o->setDescr(mud_str_dup("A medium holy wafer lies here."));
      o->setWeight(.2);
    } else {
      o->name = mud_str_dup("wafer food holy large");
      o->shortDescr = mud_str_dup("a large holy wafer");
      o->setDescr(mud_str_dup("A large holy wafer lies here."));
      o->setWeight(.4);
    }
    o->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
    o->setFoodFill(fill_amt);
    o->obj_flags.cost = 0;
    o->obj_flags.decay_time = (int) (100 * 10 * o->getWeight() / 2);
    o->setVolume(5);
    o->setMaterial(MAT_FOODSTUFF);

    if (critSuccess(c, spell)) {
      CS(spell);
      o->setFoodFill(o->getFoodFill() + 5);
      o->obj_flags.decay_time *= 2;
    }
    *c->roomp += *o;

    o->number = -1;
    act("$p poofs into existence from out of the ether.",1,c, o, NULL, TO_ROOM);
    act("$p poofs into existence from out of the ether.",1,c, o, NULL, TO_CHAR);
    return SPELL_SUCCESS;
  } else 
    return SPELL_FAIL;
}

void createFood(TBeing *caster)
{
  spellNumT spell = caster->getSkillNum(SPELL_CREATE_FOOD);

  if (!caster)
    return;

  if (!bPassClericChecks(caster,spell))
    return;

  int level = caster->getSkillLevel(spell);

  int ret=createFood(caster,level,caster->getSkillValue(spell), spell);
  if (ret==SPELL_SUCCESS) {
  } else {
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
  }
}

int castCreateWater(TBeing *caster, TObj *obj)
{
  TDrinkCon *drink;

  spellNumT spell = caster->getSkillNum(caster->spelltask->spell);
  int ret = createWater(caster, obj, caster->getSkillLevel(spell), caster->getSkillValue(spell), spell);
  if (ret == SPELL_SUCCESS) {
    if ((drink = dynamic_cast<TDrinkCon *>(obj))) {
      if (!(drink->getMaxDrinkUnits() - drink->getDrinkUnits())) {
        caster->sendTo(COLOR_SPELLS, format("<p>Your %s is completely full so you stop your prayer.<z>\n\r") % fname(drink->name));
        act("$n stops praying.", TRUE, caster, NULL, NULL, TO_ROOM);
        caster->stopCast(STOP_CAST_NONE);
      }
    }
  }
  return ret;
}

int createWater(TBeing * caster, TObj * obj, int level, short bKnown, spellNumT spell)
{
  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    obj->waterCreate(caster, level);
    return SPELL_SUCCESS;
  } else {
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
}

void createWater(TBeing * caster, TObj * obj)
{
  spellNumT spell = caster->getSkillNum(SPELL_CREATE_WATER);

  if (!bPassClericChecks(caster, spell))
    return;

  // int level = caster->getSkillLevel(spell);

  act("$n beseeches $d for thirst restoration.", FALSE, caster, NULL, NULL,
TO_ROOM);
  act("You beseech $d for thirst restoration.", FALSE, caster, NULL, NULL,
TO_CHAR);

  lag_t rounds = caster->isImmortal() ? LAG_0 : discArray[spell]->lag;
  taskDiffT diff = discArray[spell]->task;

  start_cast(caster, NULL, obj, caster->roomp, spell, diff, 2, "", rounds,
       caster->in_room, 0, 0,TRUE, 0);

#if 0
  spell = caster->getSkillNum(SPELL_CREATE_WATER);

  if (!bPassClericChecks(caster,spell))
    return;

  level = caster->getSkillLevel(spell);

  if ((ret=createWater(caster,obj,level,caster->getSkillValue(spell), spell)) == SPELL_SUCCESS) {
  } else {
  }
#endif
}

void wordOfRecall(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
int ret;
  ret=wordOfRecall(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int wordOfRecall(TBeing * caster, TBeing * victim, int, short bKnown)
{
  int ret = 0, learning = 0,location = 0;
  TRoom *room;
  char buf[256];

#if 0
  // cast on self = yes
  if (victim != caster) {
    victim = caster;
  }
#endif

  if (dynamic_cast<TMonster *>(victim) && (victim != caster)) {
    caster->sendTo("You can't recall an NPC!\n\r");
    return SPELL_FAIL;
  }

  if (victim->roomp->isRoomFlag(ROOM_ARENA) ||
      victim->roomp->isRoomFlag(ROOM_NO_ESCAPE)) {
    caster->sendTo("You can't recall from here!\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }
  
  if (caster->affectedBySpell(AFFECT_PLAYERKILL) ||
      victim->affectedBySpell(AFFECT_PLAYERKILL)){
    act("$d will not provide refuge to a murderer.",
	TRUE, caster, NULL, NULL, TO_CHAR);
    act("Nothing seems to happen.",
	FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (victim->isImmortal()) {
    caster->sendTo("I don't think that is a good idea...\n\r");
    victim->sendTo(format("%s just tried to recall you, how pathetic...\n\r") %
                   caster->getNameNOC(victim));
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

#if FACTIONS_IN_USE
  learning = bKnown - (100 - ((int) caster->getPerc()));  
#else
  learning = bKnown;
#endif
  if (caster->bSuccess(learning, caster->getPerc(), SPELL_WORD_OF_RECALL)) {

    if (!victim->desc && dynamic_cast<TMonster *>(victim))
      location = dynamic_cast<TMonster *>(victim)->oldRoom;
    else if (victim->player.hometown)
      location = victim->player.hometown;
    else
      location = Room::CS;
    ret=SPELL_SUCCESS;
  } else if (!victim->desc) {
    ret = SPELL_FAIL;
    act("$d refuses to come to your aid.",
        TRUE, caster, NULL, NULL, TO_CHAR);
    act("Nothing seems to happen.",
        TRUE, caster, NULL, NULL, TO_CHAR);
    act("Nothing seems to happen.",
        FALSE, caster, NULL, NULL, TO_ROOM);
    return ret;
  } else {
    switch (critFail(caster, SPELL_WORD_OF_RECALL)) {
      case CRIT_F_HITSELF:
        CF(SPELL_WORD_OF_RECALL);
        ret=SPELL_CRIT_FAIL;
#if FACTIONS_IN_USE
        if (!caster->percLess(10.0)) {   // if > 10.0
#endif
          act("$d is not entirely pleased with how well you are serving him!", 
              FALSE, caster, NULL, NULL, TO_CHAR);
          sprintf(buf, "He throws %s mortal body to the winds!",
              (caster == victim) ? "your" : "$N's");
          act(buf, FALSE, caster, NULL, victim, TO_CHAR);
          for (room = NULL; !room; location = number(0, top_of_world), room = real_roomp(location))
	    room = (!room) ? 0 : (room->isRoomFlag(ROOM_PRIVATE) ? 0 : room);
#if FACTIONS_IN_USE
        } else {
          location = Room::HELL;
          act("$d is upset with your unwillingness to properly serve!", 
              FALSE, caster, NULL, NULL, TO_CHAR);
          sprintf(buf, "He sends %s to Hell!", (caster == victim) ? "you" : "$N");
          act(buf, FALSE, caster, NULL, victim, TO_CHAR);
        }
#endif
        break;
      default:
        ret = SPELL_FAIL;
        act("$d refuses to come to your aid.",
            TRUE, caster, NULL, NULL, TO_CHAR);
        act("Nothing seems to happen.",
            TRUE, caster, NULL, NULL, TO_CHAR);
        act("Nothing seems to happen.",
            FALSE, caster, NULL, NULL, TO_ROOM);
        return ret;
    }
  }
  if (!(room = real_roomp(location)))
    victim->sendTo("You are completely lost.\n\r");
  else {
    act("You hear a small \"pop\" as $n disappears.", TRUE, victim, NULL, NULL, TO_ROOM);
    victim->roomp->playsound(SOUND_SPELL_WORD_OF_RECALL, SOUND_TYPE_MAGIC);

    // not totally sure this is necessary, but added to clear up bits lit
    // engage and aggressor...
    if (victim->fight())
      victim->stopFighting();

    --(*victim);
    *room += *victim;
    act("You hear a small \"pop\" as $n appears in the middle of the room.", 
        TRUE, victim, NULL, NULL, TO_ROOM);
    victim->doLook("", CMD_LOOK);

    moveLoss(*victim);

    victim->updatePos();
    act("You are exhausted from interplanar travel.", 
        FALSE, victim, NULL, NULL, TO_CHAR);
    act("$n is exhausted from interplanar travel.", 
        FALSE, victim, NULL, NULL, TO_ROOM);
  }
  return ret;
}

void wordOfRecall(TBeing * caster, TBeing * victim)
{
int ret,level;

  if (!bPassClericChecks(caster,SPELL_WORD_OF_RECALL))
    return;

  if (caster == victim) {
    act("You call to $d to ask for refuge from this situation!", FALSE, caster, NULL, NULL, TO_CHAR);
    act("$n calls to $d to ask for refuge from this situation!", FALSE, caster, NULL, NULL, TO_ROOM);
  } else {
    act("You call to $d to ask for refuge for $N!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$n calls to $d to ask for refuge for you!", FALSE, caster, NULL, victim, TO_VICT);
    act("$n calls to $d to ask for refuge for $N!", FALSE, caster, NULL, victim, TO_NOTVICT);
  }

  level = caster->getSkillLevel(SPELL_WORD_OF_RECALL);
  ret=wordOfRecall(caster,victim,level,caster->getSkillValue(SPELL_WORD_OF_RECALL));
  if (ret == SPELL_SUCCESS) {
    act("$d awards you for being a loyal follower!", FALSE, caster, NULL, NULL, TO_CHAR);
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } else {
    }
  }
}

int summon(TBeing * caster, TBeing * victim, int level, short bKnown)
{
  int rc = 0;
  TThing *t;
  int location = 0;
  TRoom *room = NULL; 

  int immun = victim->getImmunity(IMMUNE_SUMMON);
  TMonster *tmon=dynamic_cast<TMonster *>(victim);

  // check for immunity to summon
  // prevent some mobs used in quests from being moved around
  if(((immun > 0) && ::number(0,100)<=immun) || 
     (tmon && ((tmon->mobVnum()==Mob::TIGER_SHARK) || 
	       (tmon->mobVnum()==Mob::ELEPHANT)))){
    act("You feel unable to summon $N.", FALSE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  /* M = Max-Exist / PL = Player Level / ML = Monster Level
   *
   * M9999 : PL 10 : ML 10 : ( 0) - (  0) =  0
   * M 100 : PL 10 : ML 10 : ( 0) - (  0) =  0
   * M  10 : PL 10 : ML 10 : (40) - (  0) =  0
   * M   1 : PL 10 : ML 10 : (49) - (  0) =  0
   * M   1 : PL 50 : ML 10 : (49) - ( 40) =  9
   * M   1 : PL 10 : ML 50 : (49) - (-40) = 89
   *
   * The higher the result, the harder it is to summon
   */
  if (tmon && (!tmon->master || !tmon->isAffected(AFF_CHARM) ||
               !tmon->master->isPc()) && !caster->isImmortal()) {
    if (mob_index[tmon->getMobIndex()].max_exist < 10) {
      int tDiff = ((50 - min((short) 10, mob_index[tmon->getMobIndex()].max_exist)) + (caster->GetMaxLevel() - victim->GetMaxLevel()));

      if ((tDiff > 0) && ::number(0, tDiff)) {
        caster->sendTo("Your prayer meets opposition and fails!\n\r");
        act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
        return SPELL_FAIL;
      }
    }

  // Anti-Multiplay code, Pappy 11/7/2007
  // We don't allow people to summon mobs they have tagged from alts
  if (tmon && tmon->isAttackerMultiplay(caster))
  {
    caster->sendTo("Your prayer meets opposition and fails!\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

    if (caster->inGrimhaven() &&
        (tmon->anger() >= 20 || IS_SET(tmon->specials.act, ACT_AGGRESSIVE))) {
      caster->sendTo("Your deity prohibits this mob being summoned here!\n\r");
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
      return SPELL_FAIL;
    }
  }

  if (caster == victim) {
    caster->sendTo("Yeah whatever. Good one.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }
  if (victim->inRoom() == Room::NOWHERE) {
    act("$N cannot be summoned at the moment!", 
            FALSE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  // pseudo special-proc for Gold's zone
  if (!caster->canDoSummon()) {
    act("Your summoning magic gets blocked somewhere near you.", FALSE, caster, NULL, NULL, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  // pseudo special-proc for Gold's zone
  if (!victim->isSummonable()) {
    act("Your summoning magic gets blocked somewhere near your intended victim.", FALSE, caster, NULL, victim, TO_CHAR);
    act("You feel a tugging at your soul and feel like you are in another place for a second.", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("Nothing seems to happen.", FALSE, caster, NULL, victim, TO_NOTVICT);
    return SPELL_FAIL;
  }

  if (caster->isNotPowerful(victim, level, SPELL_SUMMON, SILENT_YES)) {
    caster->sendTo("That's a pretty big fish for you, small fry.  It might work if you try again.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

#if 0 
  if (immun) {
    if (isImmune(IMMUNE_SUMMON), WEAR_BODY) {
      act("You feel great difficulty in summoning $N this time.", FALSE, caster, NULL, victim, TO_CHAR);
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
      return SPELL_FAIL;
    }
  }
#endif
  if (!caster->isImmortal()) {
    if (victim->roomp->isRoomFlag(ROOM_NO_ESCAPE)) {
      caster->sendTo("You cannot penetrate the defenses of that area.\n\r");
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
      return SPELL_FAIL;
    }

    if (victim->roomp->isRoomFlag(ROOM_ARENA)) {
      act("$N cannot be summoned at the moment!", FALSE, caster, NULL, victim, TO_CHAR);
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
      return SPELL_FAIL;
    }

    if (caster->roomp->isRoomFlag(ROOM_HAVE_TO_WALK)) {
      act("You can not summon to this location.", FALSE, caster, 0, 0, TO_CHAR);
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
      return SPELL_FAIL;
    }
  }

  if (victim->isImmortal()) {
    act("Summoning the gods can be hazardous to your health...", 
            FALSE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    act("$n just tried to summon you.", FALSE, caster, NULL, victim, TO_VICT);
    return SPELL_FAIL;
  }

  if (caster->roomp && caster->roomp->isFallSector()) {
    // prevent cheap attempt to make them fall to their death
    act("$d requires you to be in contact with The World when you summon.",
         FALSE, caster, NULL, NULL, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_SUMMON)) {
    rc = caster->rawSummon(victim);
    if (IS_SET_DELETE(rc, DELETE_VICT) && IS_SET_DELETE(rc, DELETE_THIS))
      return SPELL_SUCCESS | VICTIM_DEAD | CASTER_DEAD;
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return SPELL_SUCCESS | VICTIM_DEAD;
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return SPELL_SUCCESS | CASTER_DEAD;
    return SPELL_SUCCESS;
  } else {
    caster->spellMessUp(SPELL_SUMMON);
    if (caster->riding) {
      rc = caster->fallOffMount(caster->riding, POSITION_STANDING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return SPELL_FAIL | CASTER_DEAD;
      }
    }
    while ((t = caster->rider)) {
      TBeing *tb = dynamic_cast<TBeing *>(t);
      if (tb) {
        rc = tb->fallOffMount(caster, POSITION_STANDING);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tb;
          tb = NULL;
        }
      } else {
        t->dismount(POSITION_DEAD);
      }
    }
    switch (critFail(caster, SPELL_SUMMON)) {
      case CRIT_F_HITOTHER:
        if (level < 20) {
        // pass through to next case
        } else {
          act("Your body flails helplessly through the cosmos!",
              FALSE, caster, NULL, NULL, TO_CHAR);
  
          act("You vanish in a flash of irridescent midnight blue..", 
               FALSE, caster, NULL, NULL, TO_CHAR);
          act("$n vanishes in a flash of irridescent midnight blue..", 
               FALSE, caster, NULL, NULL, TO_ROOM);

          rc = caster->genericTeleport(SILENT_YES);

          caster->doLook("", CMD_LOOK);

          moveLoss(*caster);

          caster->setMove(max(0, caster->getMove()));
          caster->updatePos();
          act("You hear a small \"pop\" as $n appears in the middle of the room.",
                TRUE, caster, NULL, NULL, TO_ROOM);
          act("$n is exhausted from interplanar travel.", 
                  FALSE, caster, NULL, NULL, TO_ROOM);

       // genericTeleport, make sure they get to look first
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;

          rc = caster->genericMovedIntoRoom(caster->roomp, -1);
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
          return SPELL_CRIT_FAIL;
        }
      case CRIT_F_HITSELF:
        act("Your body flails helplessly through the cosmos!",
            FALSE, caster, NULL, NULL, TO_CHAR);
        act("You vanish in a flash of fiery red flames..",
            FALSE, caster, NULL, NULL, TO_CHAR);
        act("$n vanishes in a flash of fiery red flames..",
            FALSE, caster, NULL, NULL, TO_ROOM);
        location = victim->in_room;
        room = real_roomp(location);
        --(*caster);
        *room += *caster;
        caster->doLook("", CMD_LOOK);
        act("You are blinded for a moment as $n appears in a flash of light!",
                  FALSE, caster, NULL, NULL, TO_ROOM);
        rc = caster->genericMovedIntoRoom(caster->roomp, -1);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return SPELL_CRIT_FAIL;
      default:
        act("Nothing seems to happen.",
                TRUE, caster, NULL, NULL, TO_CHAR);
        act("Nothing seems to happen.",
                FALSE, caster, NULL, NULL, TO_ROOM);
    }
    return SPELL_FAIL;
  }
}

int summon(TBeing * caster, TBeing * victim)
{
  int ret,level;
  int rc = 0;

  if (!bPassClericChecks(caster,SPELL_SUMMON))
    return FALSE;

  level = caster->getSkillLevel(SPELL_SUMMON);

  act("$d attempts to transfer $N to you through the cosmic ether!", FALSE, caster, NULL, victim, TO_CHAR);
  act("You feel a tug as $n attempts to transfer you through the cosmic ether with the help of $d!", FALSE, caster, NULL, victim, TO_VICT);

  if (!caster->isPc())
    victim->playsound(SOUND_COMEBACK, SOUND_TYPE_NOISE);

  ret=summon(caster,victim,level,caster->getSkillValue(SPELL_SUMMON));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
  }
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  return rc;
}

int heroesFeast(TBeing * caster, int, short bKnown, spellNumT spell)
{
  TBeing *tch = NULL;
  TThing *t=NULL;
  sstring name = caster->getName();
  int gain = 16, hitgain = 1;
  sstring message = "You partake of a magnificent feast!";

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end() && (t=*it);++it) {
      tch = dynamic_cast<TBeing *>(t);
      if (!tch)
        continue;
      if (tch->inGroup(*caster) && (tch->getPosition() > POSITION_SLEEPING)) 
      {
        if(isname("Merkaba", name)) {
          switch(::number(1,20)) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
              message = "You partake of <r>slices of meatloaf drenched in ketchup<z> with <W>warm milk<z>.\n\rAgain.";
              break;
            case 8:
            case 9:
            case 10:
              message = "You feast on a HUGE <G>pollo verde burrito<z> with extra <g>cilantro<z>.  It's just\n\rtoo much to finish it all!";
              break;
            case 11:
            case 12:
            case 13:
              message = "You partake of a heart-warming feast consisting of <K>corned beef<z>, <g>cabbage<z>, and\n\ra <o>measure of ale<z>!";
              break;
            case 14:
            case 15:
              message = "You find your magical meal of <W>icecod<z> and <y>pigsnout<z> <g>orcfish<z> on a bed of\n\r<g>seaweed<z> with <b>fresh truffles<z> on the side, accompanied by <c>fresh spring\n\rwater<z>, very satisfying!";
              break;
            case 16:
            case 17:
              message = "You enjoy <r>Steak Diane<z> smothered in <p>Silverstone red wine<z> topped with\n\r<k>Shiitake mushrooms<z>!";
              break;
            case 18:
              message = "You partake of a strengthening repast of <g>dragon haunch<z> with <k>potatoes<z> and \n\r<g>greens<z>, and <p>spiced red wine<z>!";
              gain *= 2;
              hitgain *= 2;
              break;
            case 19:
              message = "A meal of <P>shrimp<z> and <B>seahorse<z> <G>gumbo<z> now fills your tummy.  You feel ready to\n\rkill again!";
              gain *= 2;
              hitgain *= 2;
              break;
            case 20:
              message = "You feel invigorated by a hearty meal of <c>aarakocra eggs<z>, <o>fried squash<z>, and\n\r<Y>spinefuit<z>, accompanied by a nice <y>pear ice wine<z> (completely free of <K>hobbit\n\rfoot hairs<z>)!";
              gain *= 3;
              hitgain *= 3;
              break;
          }
        }
        act(message, false, tch, 0, 0, TO_CHAR);
      
        if (tch->getCond(FULL) >= 0)
          tch->gainCondition(FULL, gain);
        if (tch->getCond(THIRST) >= 0)
          tch->gainCondition(THIRST, gain);
        if (tch->getHit() < tch->hitLimit())
          tch->addToHit(hitgain);
        caster->reconcileHelp(tch, discArray[spell]->alignMod);
      }
    }
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, spell)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end() && (t=*it);++it) {
          tch = dynamic_cast<TBeing *>(t);
          if (!tch)
            continue;
          if (tch->inGroup(*caster) && (tch->getPosition() > POSITION_SLEEPING)) {
            tch->sendTo("You feel weakened! Something went horribly wrong!\n\r");

            if (tch->getCond(FULL) >= 0)
              tch->gainCondition(FULL, -gain/2);
            if (tch->getCond(THIRST) >= 0)
              tch->gainCondition(THIRST, -gain/2);
            if (tch->getHit() > 1)
              tch->addToHit(-hitgain);
          }
        }
        return SPELL_CRIT_FAIL;
      default:
        return SPELL_FAIL;
    }
  }
  return SPELL_FAIL;
}

void heroesFeast(TBeing * caster)
{
  spellNumT spell = caster->getSkillNum(SPELL_HEROES_FEAST);

  if (!caster->roomp)
    return;

  if (!bPassClericChecks(caster,spell))
    return;

  int level = caster->getSkillLevel(spell);

  int ret=heroesFeast(caster,level,caster->getSkillValue(spell), spell);
  if (ret==SPELL_SUCCESS) {
  } else if (ret == SPELL_CRIT_FAIL) {
    caster->sendTo("Something terrible seems to have happened.\n\r");
  } else {
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
  }
}

struct portalRoomT {
      int roomnum;
      const char *name;
};

portalRoomT portalRooms[] =
{
  {15346, "grimhaven"},
  {15347, "brightmoon"},
  {15348, "logrus"},
  {33760, "amber"},
};

const int NUM_PORTAL_ROOMS = 4;


int portal(TBeing * caster, const char * portalroom, int level, short bKnown)
{
  char buf[256];
  TPerson *tPerson = dynamic_cast<TPerson *>(caster);
  int i, location=0;


  for(i=0;i<NUM_PORTAL_ROOMS;++i){
    if(is_abbrev(portalroom, portalRooms[i].name)){
      location=portalRooms[i].roomnum;
    }
  }
 
  TRoom * rp = real_roomp(location);

  if (!rp || !location) {
    caster->sendTo("You can't seem to portal to that location.\n\r");
    //    vlogf(LOG_BUG, format("Attempt to portal to room %d") % location);
    return SPELL_FAIL;
  }

  if (caster->roomp->isRoomFlag(ROOM_NO_PORTAL) &&
	!caster->isImmortal()) {
    caster->sendTo("You can't seem to escape the defenses of this area.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }


  if (caster->bSuccess(bKnown,caster->getPerc(),SPELL_PORTAL)) {
    TPortal * tmp_obj = new TPortal(rp);
    tmp_obj->setPortalNumCharges(1*((level/5)+1));

    if (critSuccess(caster, SPELL_PORTAL)) {
      CS(SPELL_PORTAL);
      tmp_obj->obj_flags.decay_time *= 2;
      tmp_obj->setPortalNumCharges(2*((level/5)+1));
    }
    *caster->roomp += *tmp_obj;

    if (tPerson)
      tmp_obj->checkOwnersList(tPerson);

    caster->roomp->playsound(SOUND_SPELL_PORTAL, SOUND_TYPE_MAGIC);

    TPortal * next_tmp_obj = new TPortal(caster->roomp);
    *rp += *next_tmp_obj;

    if (tPerson)
      next_tmp_obj->checkOwnersList(tPerson);

    rp->playsound(SOUND_SPELL_PORTAL, SOUND_TYPE_MAGIC);

    act("$p suddenly appears out of a swirling mist.", TRUE, caster, tmp_obj, NULL, TO_ROOM);
    act("$p suddenly appears out of a swirling mist.", TRUE, caster, tmp_obj, NULL, TO_CHAR);

    sprintf(buf, "%s suddenly appears out of a swirling mist.\n\r", (sstring(next_tmp_obj->shortDescr).cap()).c_str());
    sendToRoom(buf, location);

    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}


void portal(TBeing * caster, const char * portalroom)
{
  int ret,level;

  if (!bPassClericChecks(caster,SPELL_PORTAL))
    return;

  level = caster->getSkillLevel(SPELL_PORTAL);

  ret=portal(caster,portalroom,level,caster->getSkillValue(SPELL_PORTAL));
  if (ret==SPELL_SUCCESS) {
  } else  {
    caster->sendTo("Nothing seems to happen.\n\r");
  }
}

