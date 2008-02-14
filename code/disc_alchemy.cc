//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// disc_alchemy.cc : The alchemy discipline
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_alchemy.h"
#include "obj_component.h"
#include "obj_opal.h"
#include "obj_component.h"
#include "obj_magic_item.h"
#include "obj_scroll.h"

int identify(TBeing *caster, TObj *obj, int, byte bKnown)
{
  sstring buf, buf2;
  int x, y;
  double z;

  if (caster->bSuccess(bKnown, SPELL_IDENTIFY)) {
    buf2=sstring(ItemInfo[obj->itemType()]->name).lower();
    buf=sstring(material_nums[obj->getMaterial()].mat_name).lower();

    caster->sendTo(COLOR_OBJECTS, fmt("You feel informed about %s...\n\rIt appears to be a kind of %s %s.\n\r") % obj->getName() % buf % buf2);
    caster->sendTo("You feel it will last ");

    if ((obj->obj_flags.decay_time == -1) || (obj->obj_flags.decay_time > 800))
      caster->sendTo("well into the future.\n\r");
    else if (obj->obj_flags.decay_time < 100)
      caster->sendTo("a few days *tops*.\n\r");
    else if (obj->obj_flags.decay_time < 200)
      caster->sendTo("about a week.\n\r");
    else if (obj->obj_flags.decay_time < 400)
      caster->sendTo("only a couple of weeks.\n\r");
    else if (obj->obj_flags.decay_time < 800)
      caster->sendTo("around a month.\n\r");

    if (obj->getVolume() > 100)
      x = ((int) obj->getVolume() / 100) * 100;
    else if (obj->getVolume() > 10)
      x = ((int) obj->getVolume() / 10) * 10;
    else
      x = obj->getVolume();

    if ((int) obj->getWeight()> 100)
      z = ((int) obj->getWeight() / 100) * 100;
    else if ((int) obj->getWeight() > 10)
      z = ((int) obj->getWeight() / 10) * 10;
    else
      z = obj->getWeight();

    if ((z - (int) z) == 0.0) {
      caster->sendTo(fmt("It seems to be about %d cubic inch%s in volume, and %d pound%s in weight.\n\r") %
              x % ((x != 1) ? "es" : "") %
              (int) z % (((int) z != 1) ? "s" : ""));
    } else if (z >= 1.0) {
      caster->sendTo(fmt("It seems to be about %d cubic inch%s in volume, %d pound%s, %d drechel%s in weight.\n\r") %
		     x % ((x != 1) ? "es" : "") %
		     (int) (z) % (((int) (z) != 1) ? "s" : "") %
		     (int) (10.0 * (z - (int) z)) % (((int) (10.0 * (z - (int) z)) != 1) ? "s" : ""));
    } else {
      caster->sendTo(fmt("It seems to be about %d cubic inch%s in volume, and %d drechel%s in weight.\n\r") % x % ((x != 1) ? "es" : "") %
              (int) (10.0 * (z - (int) z)) % (((int) (10.0 * (z - (int) z)) != 1) ? "s" : ""));
    }

    if (obj->obj_flags.cost > 100)
      x = ((int) obj->obj_flags.cost / 100) * 100;
    else if (obj->obj_flags.cost > 10)
      x = ((int) obj->obj_flags.cost / 10) * 10;
    else
      x = obj->obj_flags.cost;

    if (obj->rentCost() > 100)
      y = (obj->rentCost() / 100) * 100;
    else if (obj->rentCost() > 10)
      y = (obj->rentCost() / 10) * 10;
    else
      y = obj->rentCost();

    if ((x <= 0) && (y <= 0 || obj->max_exist > 10))
      caster->sendTo("You'd judge it to be completely worthless.\n\r");
#if 0
    else if (x <= 0)
      caster->sendTo(fmt("Although it looks worthless, you'd guess they'll charge you %d talen%s to rent it.\n\r") % y % (y != 1) ? "s" : "");
    else if (y <= 0)
      caster->sendTo(fmt("Although it looks worth at least %d talen%s, you guess its unrentable.\n\r") % x % (x != 1) ? "s" : "");
    else
      caster->sendTo(fmt("You'd judge its worth to be about %d talen%s.  Rent, around %d.\n\r") % x % (x != 1) ? "s" : "" % y);
#else
    else if(obj->max_exist <= 10 && x <= 0)
      caster->sendTo(fmt("Although it looks worthless, you'd guess they'll charge you %d talen%s to rent it.\n\r") % y % ((y !=1) ? "s" : ""));
    else if(obj->max_exist <= 10 && x > 0)
      caster->sendTo(fmt("You'd judge its worth to be about %d talen%s.  Rent, around %d.\n\r") % x % ((x != 1) ? "s" : "") % y);
    else
      caster->sendTo(fmt("You'd judge its worth to be about %d talen%s.\n\r") % x % ((x != 1) ? "s" : ""));
#endif
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void identify(TBeing *caster, TObj * idobj, TMagicItem *usedobj)
{
  int level;

  level = usedobj->getMagicLevel();

  identify(caster,idobj,level,usedobj->getMagicLearnedness());
}

int identify(TBeing *caster, TObj *obj)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_IDENTIFY, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_IDENTIFY]->lag;
  diff = discArray[SPELL_IDENTIFY]->task;

  start_cast(caster, NULL, obj, NULL, SPELL_IDENTIFY, diff, 2,"", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castIdentify(TBeing *caster, TObj *obj)
{
  int level;

  level = caster->getSkillLevel(SPELL_IDENTIFY);

  act("$n studies $p intently.", FALSE, caster, obj, NULL, TO_ROOM);
  identify(caster, obj, level, caster->getSkillValue(SPELL_IDENTIFY));
    return TRUE;
}

static sstring identifyBeingStuff(const TBeing *caster, TBeing *victim, showMeT show)
{
  sstring str;
  char buf[256];

  sprintf(buf, "You sense that %s is a %s %s.\n\r", victim->hssh(), 
              describe_level(victim->GetMaxLevel()), 
              victim->getMyRace()->getPluralName().c_str());
  str += buf;

  if (dynamic_cast<const TPerson *>(victim)) {
    sprintf(buf, "%d years, %d months, %d days, %d hours old.\n\r",
             victim->age()->year, victim->age()->month, 
             victim->age()->day, victim->age()->hours / 2);
    str += buf;
  }

  sprintf(buf, "Height %d inches, weight %d pounds.\n\r", victim->getHeight(), (int) victim->getWeight());
  str += buf;

  sprintf(buf, "%s is %s.\n\r", sstring(victim->getName()).cap().c_str(), ac_for_score(victim->getArmor()));
  str += buf;

  Stats tempStat;
  tempStat = victim->getCurStats();

  for(statTypeT the_stat=MIN_STAT; the_stat<MAX_STATS; the_stat++) {
    int temp = caster->plotStat(STAT_CURRENT,STAT_PER, 0, 30, 20);
    temp = ::number(0, temp) - 10;
    if (temp > 0) {
      continue;
    } else {  
      tempStat.add(the_stat, temp);
    }
  }
  str += "<c>Current:<z>";
  str += tempStat.printStatHeader();
  str += "        ";

  str += tempStat.printRawStats(caster);

  str += "Affected by: ";

  str += sprintbit_64(victim->specials.affectedBy, affected_bits);;
  str += "\n\r";

  str += caster->describeAffects(victim, show);

  return str;
}

int identify(TBeing *caster, TBeing * victim, int, byte bKnown)
{
  if (caster->bSuccess(bKnown, SPELL_IDENTIFY)) {
    if (caster->desc) {
      sstring str = identifyBeingStuff(caster, victim, DONT_SHOW_ME);
      str += caster->describeImmunities(victim, bKnown);

      caster->desc->page_string(str);
    }

    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void identify(TBeing *caster, TBeing *victim, TMagicItem *usedobj)
{
  int level;

  level = usedobj->getMagicLevel();

  identify(caster,victim,level,usedobj->getMagicLearnedness());
}

int identify(TBeing *caster, TBeing * victim)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_IDENTIFY, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_IDENTIFY]->lag;
  diff = discArray[SPELL_IDENTIFY]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_IDENTIFY, diff, 1,"", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castIdentify(TBeing *caster, TBeing * victim)
{
  int ret = 0,level;

  level = caster->getSkillLevel(SPELL_IDENTIFY);

  ret=identify(caster,victim,level,caster->getSkillValue(SPELL_IDENTIFY));
  return ret;
}

void TObj::divinateMe(TBeing *caster) const
{
  caster->sendTo("You can't discern anything but the obvious in regards to its type.\n\r");
}

int divinationObj(TBeing *caster, const TObj *obj, int, byte bKnown)
{
  sstring buf;
  int i, found = FALSE;

  if (caster->bSuccess(bKnown, SPELL_DIVINATION)) {
    buf=obj->shortDescr;
    caster->sendTo(COLOR_OBJECTS, fmt("Your mind analyzes %s...\n\r") % buf.uncap());
    caster->sendTo(fmt("It is %s.\n\r") % ItemInfo[obj->itemType()]->common_name);
    caster->sendTo(obj->wear_flags_to_sentence());
    caster->sendTo(fmt("%s\n\r") % obj->statObjInfo());
    obj->divinateMe(caster);
    
    // caster->sendTo("It will give you following abilities when equipped:  ");
    // buf=sprintbit(obj->obj_flags.bitvector, affected_bits);
    // buf+="\n\r";
    // caster->sendTo(buf);
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) && (obj->affected[i].modifier || obj->affected[i].modifier2)) {
        if (!found) {
          caster->sendTo("It can affect you with:\n\r");
          found = TRUE;
        }
        if (obj->affected[i].location == APPLY_SPELL) {
          if (discArray[obj->affected[i].modifier])
            caster->sendTo(fmt("    Affect:  %s: %s by %ld\n\r") % apply_types[obj->affected[i].location].name % discArray[obj->affected[i].modifier]->name %obj->affected[i].modifier2);
          else
            vlogf(LOG_BUG, fmt("BOGUS AFFECT (%d) on %s") %  obj->affected[i].modifier % obj->getName());
        } else if (obj->affected[i].location == APPLY_DISCIPLINE) {
          if (discNames[obj->affected[i].modifier].name)
            caster->sendTo(fmt("    Affect:  %s: %s by %ld\n\r") % apply_types[obj->affected[i].location].name % discNames[obj->affected[i].modifier].name % obj->affected[i].modifier2);
          else
            vlogf(LOG_BUG, fmt("BOGUS AFFECT (%d) on %s") %  obj->affected[i].modifier % obj->getName());

        } else if (obj->affected[i].location == APPLY_IMMUNITY) {
          caster->sendTo(fmt("    Affect:  %s: %s by %ld\n\r") % apply_types[obj->affected[i].location].name % immunity_names[obj->affected[i].modifier] % obj->affected[i].modifier2);
        } else {
          
          if (!strcmp(apply_types[obj->affected[i].location].name, "Magic Affect")) {
            for (unsigned long nr = 0; ; ++nr) {
              // loop over all item perma-affect flags
              if (*affected_bits[nr] == '\n')
                break;
              if (1<<nr & obj->affected[i].modifier) {
                // item has affect
                if (*affected_bits[nr]) {
                  caster->sendTo(fmt("    Affect:  Magic Affect of %s\n\r") % affected_bits[nr]);
                } else {
                  caster->sendTo(fmt("    Affect:  Magic Affect of %d\n\r") % (1<<nr));
                }
              }
            }
          } else {
            caster->sendTo(fmt("    Affect:  %s by %d\n\r") % apply_types[obj->affected[i].location].name % obj->affected[i].modifier);
          }
        }
      }
    }
    // seems silly, but the use of "%" in this text makes it necessary
    caster->sendTo(COLOR_OBJECTS, fmt("%s") % describeMaterial(obj));
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void divinationObj(TBeing *caster, TObj *obj, TMagicItem *usedobj)
{
  int level;

  level = usedobj->getMagicLevel();

  divinationObj(caster,obj,level,usedobj->getMagicLearnedness());
}


int divinationObj(TBeing *caster, TObj *obj)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_DIVINATION, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_DIVINATION]->lag;
  diff = discArray[SPELL_DIVINATION]->task;

  start_cast(caster, NULL, obj, caster->roomp, SPELL_DIVINATION, diff, 2,"", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castDivinationObj(TBeing *caster, const TObj *obj)
{
  if (caster->GetMaxLevel() > MAX_MORT && !caster->hasWizPower(POWER_WIZARD)) {
    caster->sendTo("Shame, Shame.  You shouldn't do this...\n\r");
    vlogf(LOG_CHEAT, fmt("%s used Divination Object on: %s") % 
          caster->getName() % obj->getName());
    return FALSE;
  }

  int ret = 0,level;

  level = caster->getSkillLevel(SPELL_DIVINATION);

  ret=divinationObj(caster,obj,level,caster->getSkillValue(SPELL_DIVINATION));
     return ret;
}

int divinationBeing(TBeing *caster, TBeing * victim, int, byte bKnown)
{
  if (caster->bSuccess(bKnown, SPELL_DIVINATION)) {
    if (caster->desc) {
      sstring str = identifyBeingStuff(caster, victim, SHOW_ME);

      char buf[256];
      for (immuneTypeT i = MIN_IMMUNE;i < MAX_IMMUNES; i++) {
        if (victim->getImmunity(i) == 0 || !*immunity_names[i])
          continue;
        if (victim->getImmunity(i) > 0) {
          sprintf(buf, "%d%% resistant to %s.\n\r", victim->getImmunity(i),
             immunity_names[i]);
          str += buf;
        }
        if (victim->getImmunity(i) < 0) {
          sprintf(buf, "%d%% susceptible to %s.\n\r", victim->getImmunity(i),
             immunity_names[i]);
          str += buf;
        }
      }
      str += describeMaterial(victim);

      caster->desc->page_string(str);
    }
  
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void divinationBeing(TBeing *caster, TBeing *victim, TMagicItem *usedobj)
{
  int level;

  level = usedobj->getMagicLevel();

  divinationBeing(caster,victim,level,usedobj->getMagicLearnedness());
}

int divinationBeing(TBeing *caster, TBeing * victim)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_DIVINATION, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_DIVINATION]->lag;
  diff = discArray[SPELL_DIVINATION]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_DIVINATION, diff, 1,"", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castDivinationBeing(TBeing *caster, TBeing * victim)
{
  if (caster->GetMaxLevel() > MAX_MORT && !caster->hasWizPower(POWER_WIZARD)) {
    caster->sendTo("Shame, Shame.  You shouldn't do this...\n\r");
    vlogf(LOG_CHEAT, fmt("%s used Divination Being on: %s") % 
          caster->getName() % victim->getName());
    return FALSE;
  }

  int ret = 0,level;

  level = caster->getSkillLevel(SPELL_DIVINATION);

  ret=divinationBeing(caster,victim,level,caster->getSkillValue(SPELL_DIVINATION));
     return ret;
}

int eyesOfFertuman(TBeing *caster, const char * tofind, int level, byte bKnown)
{
  TObj *obj;
  TBeing *ch;
  int j = 0;
  bool found = FALSE;
  char capbuf[256], buf[256];
  char *chr;
  sstring mod_to_find;

  // prevent people from looking for things with brackets
  mod_to_find = tofind;
  chr = strchr(mod_to_find.c_str(), '[');
  if (chr) {
    *chr = '\0';
  }
  // sanity check
  while (!mod_to_find.empty() && isspace(*mod_to_find.c_str()))
    mod_to_find.erase(mod_to_find.begin());

  if (mod_to_find.empty()) {
    caster->nothingHappens();
  }

  if (caster->bSuccess(bKnown, SPELL_EYES_OF_FERTUMAN)) {
    switch (critSuccess(caster, SPELL_EYES_OF_FERTUMAN)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_EYES_OF_FERTUMAN);
        j = level/ 3 + 10;
        break;
      default:
        j = level/ 4 + 10;
        break;
    }

    caster->sendTo("The eyes of Fertuman look far and wide across the world and find:\n\r");
    for(TObjIter iter=object_list.begin();iter!=object_list.end() && j;++iter){
      obj=*iter;
      if (isname(mod_to_find, obj->getName())) {
      /* this should randomize display a bit */
        if (obj->isObjStat(ITEM_MAGIC)) {
          if (number(0,5))
            continue;
        } else if (number(0,2)) {
          continue;
        }

        TMonster * tMon = dynamic_cast<TMonster *>(obj->parent);

	// added to skip on items flagged with nolocate 8-28-2000 -jh
	if (obj->isObjStat(ITEM_NOLOCATE))
	  continue;

	// this gets used too much for item hunting
	if (obj->obj_flags.cost > 5000)
	  continue;

	if (obj->objVnum() == YOUTH_POTION ||
            obj->objVnum() == STATS_POTION ||
            obj->objVnum() == MYSTERY_POTION ||
	    obj->objVnum() == LEARNING_POTION ||
	    obj->objVnum() == 23091 || // for brick quest
            obj->parent    == caster       ||
            (tMon && tMon->isShopkeeper()))
	  continue;
        //added to skip items on gods 10-19-00 -dash
	if(dynamic_cast<TBeing *>(obj->parent) && dynamic_cast<TBeing *>(obj->parent)->isImmortal())
	  continue;
        if (dynamic_cast<TBeing *>(obj->parent)) {
          if (strlen(caster->pers(obj->parent)) > 0) {
            strcpy(capbuf, obj->getName());
            act("$p carried by $N.", TRUE, caster, obj, obj->parent, TO_CHAR);
            found = TRUE;
          }
        } else if (obj->equippedBy) {
          if (strlen(caster->pers(obj->equippedBy)) > 0) {
            strcpy(capbuf, obj->getName());
            act("$p equipped by $N.", TRUE, caster, obj, obj->equippedBy, TO_CHAR);
            found = TRUE;
          }
        } else if (obj->parent) {
          strcpy(capbuf, obj->getName());
          strcpy(buf, obj->parent->getName());
          act("$p in $N.", TRUE, caster, obj, obj->parent, TO_CHAR);

          if (obj->parent->parent && dynamic_cast<TMonster *>(obj->parent->parent))
            act("...carried by $N.", TRUE, caster, NULL, obj->parent->parent, TO_CHAR);

          found = TRUE;
        } else {
          strcpy(capbuf, obj->getName());
          sprintf(capbuf, colorString(caster, caster->desc, capbuf, NULL, COLOR_OBJECTS, TRUE).c_str());
          if (obj->in_room == ROOM_NOWHERE || !caster->canSee(obj)) {
            act("$p is in use but you can't tell the location.", TRUE, caster, obj,NULL, TO_CHAR);
          } else if (obj->inImperia() && !caster->isImmortal()) {
            continue;
          } else {
            if (IS_SET(caster->desc->plr_color, PLR_COLOR_ROOM_NAME)) {
              if (hasColorStrings(NULL, obj->roomp->getName(), 2)) {
                 caster->sendTo(COLOR_ROOM_NAME, fmt("%s is in %s.\n\r") % capbuf % caster->dynColorRoom(obj->roomp, 1, TRUE));
              } else {
                caster->sendTo(COLOR_ROOM_NAME, fmt("%s is in %s%s%s.\n\r") %  
                   capbuf % 
                   caster->addColorRoom(obj->roomp, 1) %
                   obj->roomp->getName() % caster->norm());
              }
            } else {
              caster->sendTo(COLOR_BASIC, fmt("%s is in %s%s%s.\n\r") % capbuf % caster->purple() % colorString(caster, caster->desc, obj->roomp->getName(), NULL, COLOR_NONE, TRUE) % caster->norm());
            }
            found = TRUE;
          }
        }
        j--;
      }
    }
    for (ch = character_list; ch && j; ch = ch->next) {
      if (isname(mod_to_find, ch->getName())) {
        if ((ch->getInvisLevel() > caster->GetMaxLevel()) ||
            (ch->getInvisLevel() >= GOD_LEVEL1 &&
             !caster->isImmortal())) {
          continue;
        }
#if 1
        strcpy(capbuf, ch->getName());
        sprintf(capbuf, colorString(caster, caster->desc, capbuf, NULL, COLOR_MOBS, TRUE).c_str());
        if (ch->in_room == ROOM_NOWHERE || !caster->canSee(ch)) {
          act("$N is somewhere but you can't tell the location.", TRUE, caster, NULL, ch, TO_CHAR);
        } else if (ch->inImperia() && !caster->isImmortal()) {
	  continue;
        } else {
          if (IS_SET(caster->desc->plr_color, PLR_COLOR_ROOM_NAME)) {
            if (hasColorStrings(NULL, ch->roomp->getName(), 2)) {
              caster->sendTo(COLOR_ROOM_NAME, fmt("%s is in %s.\n\r") % capbuf % caster->dynColorRoom(ch->roomp, 1, TRUE));
            } else {
              caster->sendTo(COLOR_ROOM_NAME, fmt("%s is in %s%s%s.\n\r") %
                   capbuf %
                   caster->addColorRoom(ch->roomp, 1) %
                   ch->roomp->getName() % caster->norm());
            }
          } else {
            caster->sendTo(COLOR_BASIC, fmt("%s is in %s%s%s.\n\r") % capbuf % caster->purple() % colorString(caster, caster->desc, ch->roomp->getName(), NULL, COLOR_NONE, TRUE) % caster->norm());
          }
          found = TRUE;
        }
#else
// old
        strcpy(capbuf, ch->getName());
        caster->sendTo(fmt("%s at %s.\n\r") % cap(capbuf) % 
          (ch->roomp ? ch->roomp->name : "God only knows where..."));
#endif
        j--;
      }
    }
    if (found) {
      act("$n finishes chanting and smiles to $mself.",
          TRUE, caster, NULL , NULL, TO_ROOM);
    } else {
      caster->sendTo("You are unable to detect anything like that.\n\r");
      act("$n finishs chanting and a frown appears on $s face.",
           TRUE, caster, NULL , NULL, TO_ROOM);
    }
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int eyesOfFertuman(TBeing *caster, const char * tofind)
{
  taskDiffT diff;

  if (!tofind)
    return FALSE;

//  if (caster->affectedBySpell(SPELL_BLINDNESS)) {
  if (caster->isAffected(AFF_BLIND) && !caster->isAffected(AFF_TRUE_SIGHT) && !caster->isAffected(AFF_CLARITY)) {
    act("How do you expect to see while you are blind?",
        FALSE, caster, NULL, NULL, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }

  if (caster->GetMaxLevel() > MAX_MORT &&
      caster->GetMaxLevel() < commandArray[CMD_WHERE]->minLevel) {
    caster->sendTo("You are unable to locate things at your level.\n\r");
    vlogf(LOG_CHEAT, fmt("%s using %s to locate '%s'") %  caster->getName() % discArray[SPELL_EYES_OF_FERTUMAN]->name % tofind);
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_EYES_OF_FERTUMAN, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_EYES_OF_FERTUMAN]->lag;
  diff = discArray[SPELL_EYES_OF_FERTUMAN]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_EYES_OF_FERTUMAN, diff, 1, tofind, rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castEyesOfFertuman(TBeing *caster, const char * tofind)
{
  int ret = 0,level;

  if (!tofind) {
    vlogf(LOG_BUG, "Somehow someone lost the argument in eyes of fert");
    return FALSE;
  }

  level = caster->getSkillLevel(SPELL_EYES_OF_FERTUMAN);

  ret=eyesOfFertuman(caster,tofind,level,caster->getSkillValue(SPELL_EYES_OF_FERTUMAN));
  return ret;
}

int TThing::powerstoneMe(TBeing *caster, int, byte)
{
  act("It is too late before you realize your blunder -- that's no opal!", 
            FALSE, caster, NULL, NULL, TO_CHAR);
  caster->nothingHappens(SILENT_YES);
  return SPELL_FAIL;
}

int TOpal::powerstoneMe(TBeing *caster, int, byte bKnown)
{
  int str;
  sstring buf;

  if ((psGetStrength() == psGetCarats()) || (psGetConsecFails() >= 2)) {
    // But the thing was already maxed out 
    act("You sense that this powerstone is at full possible strength.",FALSE, caster, NULL, NULL, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, SPELL_POWERSTONE)) {
    switch (critSuccess(caster, SPELL_POWERSTONE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_POWERSTONE);
        break;
      default:
        break;
    }
    // Raise the strength by one.
    psAddStrength(1);
    psSetConsecFails(0);
    str = psGetStrength();

    // **NOTE** If you change this value please aleart the LOWs of it.

    obj_flags.cost = suggestedPrice();

    // **NOTE**

    if (psGetStrength() > 1) {
      act("The glow surrounding the powerstone intensifies for a brief moment!", FALSE, caster, NULL, NULL, TO_CHAR);
      act("The glow surrounding $n's powerstone intensifies for a brief moment!", FALSE, caster, NULL, NULL, TO_ROOM);
      act("You have succeeded in increasing the strength of the powerstone!", FALSE, caster, NULL, NULL, TO_CHAR);
    } else {
      // ch has turned the opal into a powerstone 
      act("You have created a powerstone -- $p begins to glow softly!", FALSE, caster, this, NULL, TO_CHAR);
      act("$n seems to have somehow enchanted $p! It begins to glow softly!", FALSE, caster, this, NULL, TO_ROOM);
      // init the four values
      psSetConsecFails(0);
      buf=name;
      buf+=" stone powerstone power";
      swapToStrung();
      delete [] name;
      name = mud_str_dup(buf);

      addObjStat(ITEM_MAGIC);
      addObjStat(ITEM_GLOW);
      addGlowEffects();
    }
    if (psGetStrength() >= psGetCarats())
      caster->sendTo("Congratulations! You have brought this powerstone to its maximum strength!\n\r");
    return SPELL_SUCCESS;
  } else {
    // Do another check to see if the casting fails 
    caster->nothingHappens();

    switch (critFail(caster, SPELL_POWERSTONE)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_POWERSTONE);
        psAddConsecFails(1);

        if (psGetConsecFails() >= 2) {
          act("Oops. You failed twice in a row on this powerstone.",
                 FALSE, caster, NULL, NULL, TO_CHAR);
          act("It can no longer grow in strength.",
                 FALSE, caster, NULL, NULL, TO_CHAR);
        }
        return SPELL_CRIT_FAIL;
      default:
        break;
    }
    return SPELL_FAIL;
  }
}

int powerstone(TBeing *caster, TObj *obj, int x, byte bKnown)
{
  return obj->powerstoneMe(caster, x, bKnown);
}

int powerstone(TBeing *caster, TObj *obj)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_POWERSTONE, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_POWERSTONE]->lag;
  diff = discArray[SPELL_POWERSTONE]->task;

  start_cast(caster, NULL, obj, NULL, SPELL_POWERSTONE, diff, 2,"", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castPowerstone(TBeing *caster, TObj *obj)
{
  int ret = 0,level;

  level = caster->getSkillLevel(SPELL_POWERSTONE);

  ret=powerstone(caster,obj,level,caster->getSkillValue(SPELL_POWERSTONE));
     return ret;
}

int shatter(TBeing *caster, TBeing * victim, int level, byte bKnown)
{
  bool scrap_it = FALSE;
  wearSlotT hand;

  if (caster->isNotPowerful(victim, level, SPELL_SHATTER, SILENT_NO)) {
    return SPELL_FAIL;
  }

  // success = scrapped item.  no item damage allowed in arena
  if (caster->roomp && caster->roomp->isRoomFlag(ROOM_ARENA)) {
    act("A magic power prevents anything from happening here.",
         FALSE, caster, 0, 0, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FALSE;
  }

  caster->reconcileHurt(victim, discArray[SPELL_SHATTER]->alignMod);

  if (caster->bSuccess(bKnown,SPELL_SHATTER)) {
    switch (critSuccess(caster, SPELL_SHATTER)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_SHATTER);
        scrap_it = TRUE;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (victim->heldInPrimHand() || victim->heldInSecHand()) {
    // pick a hand that is holding something */
      for (hand = wearSlotT(number(HOLD_RIGHT, HOLD_LEFT));
             !victim->equipment[hand];
           hand = wearSlotT(number(HOLD_RIGHT, HOLD_LEFT)));
      victim->shatterWeapon(hand, scrap_it);
    } else
      act("But $N isn't holding anything!", FALSE, caster, NULL, victim, TO_CHAR);

    return SPELL_SUCCESS;
  } else {
    switch(critFail(caster, SPELL_SHATTER)) {
      case CRIT_F_HITSELF:
        CF(SPELL_SHATTER);
        act("Oops! You have a terrible feeling something went horribly wrong!",
              FALSE, caster, NULL, NULL, TO_CHAR);
        act("You've cast your 'shatter' on yourself!", 
              FALSE, caster, NULL, NULL, TO_CHAR);
        act("$n's spell backfires on $m!", FALSE, caster, NULL, NULL, TO_ROOM);
        shatter(caster,caster,100,100);
        return SPELL_CRIT_FAIL;
        break;
      default:
        caster->nothingHappens();
        break;
    }
    return SPELL_FAIL;
  }
}

int shatter(TBeing *caster, TBeing * victim)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_SHATTER, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_SHATTER]->lag;
  diff = discArray[SPELL_SHATTER]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_SHATTER, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castShatter(TBeing *caster, TBeing * victim)
{
  int ret = 0,level;

  level = caster->getSkillLevel(SPELL_SHATTER);

  ret=shatter(caster,victim,level,caster->getSkillValue(SPELL_SHATTER));
    return ret;
}

int farlook(TBeing *caster, TBeing * victim, int level, byte bKnown)
{
  vector<TBeing *>tBeing(0);

  int target;
  char buf1[128];
  TThing *t;
  TBeing *ch;

  if (caster->isNotPowerful(victim, level, SPELL_FARLOOK, SILENT_NO)) 
    return SPELL_FAIL;
 
  target = victim->roomp->number;

  if (target == ROOM_NOCTURNAL_STORAGE) {
    caster->nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.",
        false, caster, 0, 0, TO_CHAR);
    return SPELL_FAIL;
  }
  if (target == ROOM_VOID) {
    caster->nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.",
        false, caster, 0, 0, TO_CHAR);
    return SPELL_FAIL;
  }
  if (target == ROOM_IMPERIA) {
    caster->nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.",
        false, caster, 0, 0, TO_CHAR);
    return SPELL_FAIL;
  }
  if (target == ROOM_HELL) {
    caster->nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.",
        false, caster, 0, 0, TO_CHAR);
    return SPELL_FAIL;
  }
  if (target == ROOM_STORAGE) {
    caster->nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.",
        false, caster, 0, 0, TO_CHAR);
    return SPELL_FAIL;
  }
  if (target == ROOM_POLY_STORAGE) {
    caster->nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.",
        false, caster, 0, 0, TO_CHAR);
    return SPELL_FAIL;
  }
  if (target == ROOM_CORPSE_STORAGE) {
    caster->nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.",
        false, caster, 0, 0, TO_CHAR);
    return SPELL_FAIL;
  }
  if (target == ROOM_Q_STORAGE) {
    caster->nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.",
        false, caster, 0, 0, TO_CHAR);
    return SPELL_FAIL;
  }
  if (target == ROOM_DONATION) {
    caster->nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.",
        false, caster, 0, 0, TO_CHAR);
    return SPELL_FAIL;
  }
  if (target == ROOM_DUMP) {
    caster->nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.",
        false, caster, 0, 0, TO_CHAR);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, SPELL_FARLOOK)) {
    act("You conjure up a large cloud which shimmers slightly before revealing...",
              FALSE, caster, 0, 0, TO_CHAR);
    act("$n conjures up a large cloud which shimmers slightly before revealing...",
              FALSE, caster, 0, 0, TO_ROOM);

    for (t = caster->roomp->getStuff(); t; t = t->nextThing)
      if ((ch = dynamic_cast<TBeing *>(t)) && ch->isPc() && ch->desc)
        tBeing.push_back(ch);

    // be warned, this is quirky due to the use of doAt
    // remember that doAt takes them out of current room, does thing, and
    // then puts them back in.  Thus the stuff list changes
    //
    sprintf(buf1, "%d look", target);
    for (unsigned int tBeingIndex = 0; tBeingIndex < tBeing.size(); tBeingIndex++) {
      int tBrief = FALSE;

      if ((tBrief = tBeing[tBeingIndex]->isPlayerAction(PLR_BRIEF)))
        tBeing[tBeingIndex]->remPlayerAction(PLR_BRIEF);

      tBeing[tBeingIndex]->doAt(buf1, true);

      if (tBrief)
        tBeing[tBeingIndex]->addPlayerAction(PLR_BRIEF);
    }

    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_FARLOOK)) {
      case CRIT_F_HITSELF:
        CF(SPELL_FARLOOK);
        act("You conjure up a large cloud which shimmers slightly before revealing...",
              FALSE, caster, 0, 0, TO_CHAR);
        act("$n conjures up a large cloud which shimmers slightly before revealing...",
              FALSE, caster, 0, 0, TO_ROOM);
        strcpy(buf1, "70 look");
        caster->doAt(buf1, true);
        return SPELL_CRIT_FAIL;
        break;
      default:
        caster->nothingHappens();
        break;
    }
    return SPELL_FAIL;
  }

  return SPELL_FAIL;
}

int farlook(TBeing *caster, TBeing * victim)
{
  taskDiffT diff;

  if (caster->affectedBySpell(SPELL_BLINDNESS)) {
    caster->nothingHappens();
    return TRUE;
  }

  if (!bPassMageChecks(caster, SPELL_FARLOOK, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_FARLOOK]->lag;
  diff = discArray[SPELL_FARLOOK]->task;

  start_cast(caster, victim, NULL, NULL, SPELL_FARLOOK, diff, 1, "", rounds, caster->in_room, 0, CASTFLAG_MAYBE_SENSE_CAST,TRUE, 0);

  return TRUE;
}

int castFarlook(TBeing *caster, TBeing * victim)
{
  int ret = 0,level;

  level = caster->getSkillLevel(SPELL_FARLOOK);

  ret=farlook(caster,victim,level,caster->getSkillValue(SPELL_FARLOOK));
  return ret;
}

int illuminate(TBeing *caster, TObj *obj, int x, byte bKnown)
{
  return obj->illuminateMe(caster, x, bKnown);
}

int TObj::illuminateMe(TBeing *caster, int level, byte bKnown)
{
  int i;

  if (isObjStat(ITEM_MAGIC)) {
    caster->sendTo("Magical forces protect that object from your magic!\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (obj_flags.decay_time >= 0) {
    caster->sendTo("That object is too intemporal for this magic.\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (obj_flags.struct_points < 0) {
    caster->sendTo("Sorry, nope, uh uh, no can do, N, O, No.\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown,SPELL_ILLUMINATE)) {
    for (i = 0;i < MAX_OBJ_AFFECT;i++) {
      if (affected[i].location == APPLY_LIGHT) {
        caster->sendTo("That item is already lit up.  It can't get any brighter.\n\r");
        caster->nothingHappens(SILENT_YES);
        return SPELL_FAIL;
      }
    }

    // ok to apply to the item at this point
    for (i = 0;i < MAX_OBJ_AFFECT;i++) {
      if (affected[i].location == APPLY_NONE) {
        affected[i].location = APPLY_LIGHT;
        affected[i].modifier = (level + 1)/2;
        addToLight(affected[i].modifier);
        obj_flags.decay_time = obj_flags.struct_points;
        addObjStat(ITEM_HUM);
        addObjStat(ITEM_GLOW);

        // no real sense, since was done above
        // addGlowEffects();

        caster->sendTo("The item's energy begins to seep out of the item as it starts to glow.\n\r");
        act("The item's energy begins to seep out of the item as it starts to glow.", FALSE, caster, 0, 0, TO_ROOM);
        return SPELL_SUCCESS;
      }
    }
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void illuminate(TBeing *caster, TMagicItem *usedobj, TObj * idobj)
{
  int level;

  level = usedobj->getMagicLevel();

  illuminate(caster,idobj,level,usedobj->getMagicLearnedness());
}

int illuminate(TBeing *caster, TObj *obj)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_ILLUMINATE, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_ILLUMINATE]->lag;
  diff = discArray[SPELL_ILLUMINATE]->task;

  start_cast(caster, NULL, obj, caster->roomp, SPELL_ILLUMINATE, diff, 2,"", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castIlluminate(TBeing *caster, TObj *obj)
{
  int ret = 0, level;
  level = caster->getSkillLevel(SPELL_ILLUMINATE);
  
  ret=illuminate(caster,obj,level,caster->getSkillValue(SPELL_ILLUMINATE));
  return ret;
}

int detectMagic(TBeing *caster, TBeing * victim, int level, byte bKnown)
{
  affectedData aff;
// COMMENTED OUT FOR DURATIONS
  // to make compile uncommented 4 lines below
  if (victim->affectedBySpell(SPELL_DETECT_MAGIC)) {
    caster->nothingHappens();
    return SPELL_FAIL;
  }

  aff.type = SPELL_DETECT_MAGIC;
  aff.duration = level * 2 * UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_DETECT_MAGIC;

  if (caster->bSuccess(bKnown, SPELL_DETECT_MAGIC)) {

    switch (critSuccess(caster, SPELL_DETECT_MAGIC)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_DETECT_MAGIC);
        aff.duration *=2 ;
        break;
      case CRIT_S_NONE:
        break;
    }

    victim->sendTo("Your eyes tingle.\n\r");
    act("$n's eyes twinkle for a brief moment.",
              FALSE, victim, NULL, NULL, TO_ROOM);
    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }
    // to make compile uncommented 2 lines below
    return SPELL_SUCCESS;
    victim->affectTo(&aff);
  } else {
    // to make compile uncommented 2 lines below
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}
// XXX
void detectMagic(TBeing *caster, TBeing * victim, TMagicItem *obj)
{
  detectMagic(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int detectMagic(TBeing *caster, TBeing * victim)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_DETECT_MAGIC, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_DETECT_MAGIC]->lag;
  diff = discArray[SPELL_DETECT_MAGIC]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_DETECT_MAGIC, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castDetectMagic(TBeing *caster, TBeing * victim)
{
  int ret = 0,level;

  level = caster->getSkillLevel(SPELL_DETECT_MAGIC);

  ret=detectMagic(caster,victim,level,caster->getSkillValue(SPELL_DETECT_MAGIC));
    return ret;
}

int dispelMagic(TBeing *caster, TObj * obj, int, byte bKnown)
{
  int i;

  if (caster->bSuccess(bKnown, SPELL_DISPEL_MAGIC)) {

    // assumes item not being used so don't have to affectFrom()
    // this is the same list in checkObjStat for affects requiring MAGIC be set
    for (i = 0; i < MAX_OBJ_AFFECT; i++) { 
      if ((obj->affected[i].location != APPLY_NONE) &&
          (obj->affected[i].location != APPLY_LIGHT) &&
          (obj->affected[i].location != APPLY_NOISE) &&
          (obj->affected[i].location != APPLY_HIT) &&
          (obj->affected[i].location != APPLY_CHAR_WEIGHT) &&
          (obj->affected[i].location != APPLY_CHAR_HEIGHT) &&
          (obj->affected[i].location != APPLY_MOVE) &&
          (obj->affected[i].location != APPLY_ARMOR)) {
        obj->affected[i].location = APPLY_NONE;
        obj->affected[i].modifier = 0;
        obj->affected[i].modifier2 = 0;
        obj->affected[i].bitvector = 0;
      }
    }
    obj->remObjStat(ITEM_MAGIC);

    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void dispelMagic(TBeing *caster, TObj * tar_obj, TMagicItem *obj)
{
  int ret = 0;
  int level = obj->getMagicLevel();

  ret = dispelMagic(caster,tar_obj, level,obj->getMagicLearnedness());
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$p dispels the magical forces affecting $n...", 
           FALSE, tar_obj, obj, NULL, TO_ROOM);
  }
}

int dispelMagic(TBeing *caster, TObj *obj)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_DISPEL_MAGIC, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_DISPEL_MAGIC]->lag;
  diff = discArray[SPELL_DISPEL_MAGIC]->task;

  start_cast(caster, NULL, obj, caster->roomp, SPELL_DISPEL_MAGIC, diff, 2,"", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castDispelMagic(TBeing *caster, TObj *obj)
{
  int ret = 0,level;

  level = caster->getSkillLevel(SPELL_DISPEL_MAGIC);

  ret=dispelMagic(caster,obj,level,caster->getSkillValue(SPELL_DISPEL_MAGIC));

  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("You dispel the magical forces affecting $N...", 
         FALSE, caster, NULL, obj, TO_CHAR);
    act("$n dispels the magical forces affecting $N...", 
         FALSE, caster, NULL, obj, TO_ROOM);
  }
  return ret;
}

int dispelMagic(TBeing *caster, TBeing * victim, int, byte bKnown)
{
  caster->reconcileHurt(victim,discArray[SPELL_DISPEL_MAGIC]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_DISPEL_MAGIC)) {
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int dispelMagic(TBeing *caster, TBeing * victim, TMagicItem *obj)
{
  mud_assert(caster != NULL, "dispelMagic(): no caster");
  mud_assert(victim != NULL, "dispelMagic(): no victim");

  int level = obj->getMagicLevel();

  int ret = dispelMagic(caster,victim, level,obj->getMagicLearnedness());
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$p dispels the magical forces affecting you...", 
           FALSE, victim, obj, NULL, TO_CHAR);
    act("$p dispels the magical forces affecting $n...", 
           FALSE, victim, obj, NULL, TO_ROOM);
    int rc = generic_dispel_magic(caster, victim, level, caster->isImmortal());
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }
  return 0;
}

int dispelMagic(TBeing *caster, TBeing * victim)
{
  if (!bPassMageChecks(caster, SPELL_DISPEL_MAGIC, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_DISPEL_MAGIC]->lag;
  taskDiffT diff = discArray[SPELL_DISPEL_MAGIC]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_DISPEL_MAGIC, diff, 1,"", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castDispelMagic(TBeing *caster, TBeing * victim)
{
  mud_assert(caster != NULL, "castDispelMagic(): no caster");
  mud_assert(victim != NULL, "castDispelMagic(): no victim");

  int level = caster->getSkillLevel(SPELL_DISPEL_MAGIC);
  if (caster->isNotPowerful(victim, level, SPELL_DISPEL_MAGIC, SILENT_NO)) {
    return 0;
  }

  int ret=dispelMagic(caster,victim,level,caster->getSkillValue(SPELL_DISPEL_MAGIC));

  if (IS_SET(ret, SPELL_SUCCESS)) {
    if (caster != victim) {
      act("You dispel the magical forces affecting $N...", 
           FALSE, caster, NULL, victim, TO_CHAR);
      act("$n dispels the magical forces affecting you...", 
           FALSE, caster, NULL, victim, TO_VICT);
      act("$n dispels the magical forces affecting $N...", 
           FALSE, caster, NULL, victim, TO_NOTVICT);
    } else {
      act("You dispel the magical forces affecting you...", 
           FALSE, caster, NULL, 0, TO_CHAR);
      act("$n dispels the magical forces affecting $m...", 
           FALSE, caster, NULL, 0, TO_ROOM);
    }
    int rc = generic_dispel_magic(caster, victim, level, caster->isImmortal());
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }
  return TRUE;
}

// returns DELETE_VICT (vict)
int generic_dispel_magic(TBeing *caster, TBeing *victim, int, immortalTypeT immortal, safeTypeT safe)
{
  // caster might be NULL (death-time), however, in such cases immortal is generally
  // true.  Some of the logic that follows doesn't check for !caster, but uses
  // immortal==true instead.

  mud_assert(victim != NULL, "generic_dispel_magic(): no victim");

  TMonster *tvm = dynamic_cast<TMonster *>(victim);
  spellNumT spell;
  int rc;

  struct dispelStruct {
    spellNumT spell;
    bool aggressive_act;
    bool needs_saving_throw;
    bool death_time_only;  // this is primarily for prayers
  };

  dispelStruct dispelArray[] = {
    // air disc
    { SPELL_FEATHERY_DESCENT, false, true, false },
    { SPELL_FLY, true, true, false },
    { SPELL_ANTIGRAVITY, true, true, false },
    { SPELL_LEVITATE, true, true, false },
    { SPELL_FALCON_WINGS, true, true, false },
    { SPELL_PROTECTION_FROM_AIR, true, true, false },
    // alchemy
    { SPELL_DETECT_MAGIC, false, true, false },
    // earth
    { SPELL_STONE_SKIN, false, true, false },
    { SPELL_TRAIL_SEEK, false, true, false },
    { SPELL_PROTECTION_FROM_EARTH, true, true, false },
    // fire
    { SPELL_FAERIE_FIRE, false, false, false },
    { SPELL_FLAMING_FLESH, false, true, false },
    { SPELL_INFRAVISION, false, true, false },
    { SPELL_PROTECTION_FROM_FIRE, true, true, false },
    // sorcery
    { SPELL_SORCERERS_GLOBE, true, true, false },
    { SPELL_BIND, false, false, false },
    { SPELL_PROTECTION_FROM_ELEMENTS, true, true, false },
    // spirit
    { SPELL_SILENCE, false, true, false },
    { SPELL_ENSORCER, false, false, false },
    { SPELL_INVISIBILITY, false, true, false },
    { SPELL_STEALTH, false, true, false },
    { SPELL_ACCELERATE, true, true, false },
    { SPELL_HASTE, true, true, false },
    { SPELL_CALM, false, true, false },
    { SPELL_SENSE_LIFE, false, true, false },
    { SPELL_DETECT_INVISIBLE, false, true, false },
    { SPELL_TRUE_SIGHT, false, true, false },
    { SPELL_FEAR, false, false, false },
    { SPELL_SLUMBER, false, false, false },
    // water
    { SPELL_ICY_GRIP, false, false, false },
    { SPELL_GILLS_OF_FLESH, true, true, false },
    { SPELL_PROTECTION_FROM_WATER, true, true, false },
    { SPELL_PLASMA_MIRROR, true, true, false },
    { SPELL_GARMULS_TAIL, false, true, false },

    // cleric prayers - these should be death-time only stuff
    { SPELL_SANCTUARY, true, true, true },
    { SPELL_ARMOR, true, true, true },
    { SPELL_ARMOR_DEIKHAN, true, true, true },
    { SPELL_BLESS, true, true, true },
    { SPELL_BLESS_DEIKHAN, true, true, true },
    { SPELL_BLINDNESS, false, false, true },
    { SPELL_PARALYZE, false, false, true },
    { SPELL_POISON, false, false, true },
    { SPELL_POISON_DEIKHAN, false, false, true },
    { SPELL_CURSE, false, false, true },
    { SPELL_CURSE_DEIKHAN, false, false, true },

    { SPELL_STUPIDITY, true, true, false },
    { SPELL_CELERITE, true, true, false },
    { SPELL_LEGBA, true, true, false },
    { SPELL_DJALLA, true, true, false },
    { SPELL_SENSE_LIFE_SHAMAN, true, true, false },
    { SPELL_DETECT_SHADOW, true, true, false },
    { SPELL_SHADOW_WALK, false, true, false },
    { SPELL_INTIMIDATE, true, true, false },
    { SPELL_CHEVAL, true, true, false },
    { SPELL_HYPNOSIS, true, true, false },
    { SPELL_CLARITY, true, true, false },
    { SPELL_AQUALUNG, true, true, false },
    { SPELL_THORNFLESH, true, true, false },
    { SPELL_SHIELD_OF_MISTS, true, true, false },
    { SPELL_CONTROL_UNDEAD, true, true, false },
    { SPELL_RESURRECTION, true, true, false },
    { SPELL_DANCING_BONES, true, true, false },
    { SPELL_VOODOO, true, true, false },

#if 0
    // these effects are on mobs
    // death-time-only is silly to check for
    { SPELL_STICKS_TO_SNAKES, false, false, true },
    { SPELL_LIVING_VINES, false, false, true },
    { SPELL_PLAGUE_LOCUSTS, false, false, true },
#endif

#if 0
    // not yet implemented
    { SPELL_DETECT_POISON, false, true, false },
    { SPELL_DETECT_POISON_DEIKHAN, false, true, false },
#endif

#if 0
    // skills that should be usable again if they die
    // these use to have a check for ARENA-death
    // not sure how to do this in new setup, so commented out for time being
    { SKILL_TRANSFIX, false, false, true },
    { SKILL_CHI, false, false, true },
    { SKILL_DOORBASH, false, false, true },
    { SKILL_TRANSFORM_LIMB, false, false, true },
    { SKILL_BARKSKIN, false, false, true },
    { SKILL_TRACK, false, false, true },
    { SKILL_CONCEALMENT, false, false, true },
    { SKILL_FORAGE, false, false, true },
    { SKILL_SEEKWATER, false, false, true },
    { SKILL_ENCAMP, false, false, true },
    { SKILL_DIVINATION, false, false, true },
    { SKILL_SPY, false, false, true },
    { SKILL_DISGUISE, false, false, true },
    { SKILL_BERSERK, false, false, true },
    { SKILL_DEATHSTROKE, false, false, true },
    { SKILL_DOORBASH, false, false, true },
    { SKILL_QUIV_PALM, false, false, true },
#endif

    { TYPE_UNDEFINED, false, false, false}   // this is final terminator
    // spell, aggressive, saving throw, death_time_only
  };

  int iter;
  for (iter = 0; dispelArray[iter].spell != TYPE_UNDEFINED; iter++) {
    spell = dispelArray[iter].spell;

    // check if they have the spell
    // should decay if !caster (death-time) or if set to decay all the time
    if ((!caster || !dispelArray[iter].death_time_only) &&
        victim->affectedBySpell(spell)) {

      // immortals should always succeed
      // make a save otherwise
      // there is assumption that !caster (death-time) will have immortal=true
      if (immortal || !dispelArray[iter].needs_saving_throw ||
          !victim->isLucky(caster->spellLuckModifier(SPELL_DISPEL_MAGIC))) {
        rc = victim->spellWearOff(spell, safe);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_VICT;
        victim->affectFrom(spell);
      }
      // aggressive Act 
      if (caster && !victim->fight() && tvm) {
        caster->setCharFighting(victim);
        caster->setVictFighting(victim);
      }
    }
  }

  if (!caster && victim->isAffected(AFF_SANCTUARY)) {
    if (immortal || !victim->isLucky(caster->spellLuckModifier(SPELL_DISPEL_MAGIC))) {
      REMOVE_BIT(victim->specials.affectedBy, AFF_SANCTUARY);
      victim->sendTo("You feel more vulnerable as your white aura slowly fades.\n\r");
      act("The white glow around $n's body fades.", FALSE, victim, NULL, NULL, TO_ROOM);
    }
    // aggressive Act 
    if (caster && !victim->fight() && tvm) {
      caster->setCharFighting(victim);
      caster->setVictFighting(victim);
    }
  }
  return FALSE;
}

int enhanceWeapon(TBeing *caster, TObj *obj, int level, byte bKnown)
{
  return obj->enhanceMe(caster, level, bKnown);
}

int TThing::enhanceMe(TBeing *caster, int, byte)
{
  caster->sendTo("Uhh, you can't enhance something that isn't a weapon...\n\r");
  caster->nothingHappens(SILENT_YES);
  return SPELL_FAIL;
}

// may return DELETE_ITEM (toenhance)
int enhanceWeapon(TBeing *caster, TMagicItem *usedobj, TObj * toenhance)
{
  int ret = 0;

  ret = enhanceWeapon(caster,toenhance,usedobj->getMagicLevel(),usedobj->getMagicLearnedness());
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$p begins to glow with a soft yellow light.", 
          FALSE, caster, toenhance, NULL, TO_CHAR);
    act("$p begins to glow with a soft yellow light.", 
          FALSE, caster, toenhance, NULL, TO_ROOM);
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    act("A nasty odor eminates from $p, looks like a malfunction!", 
          FALSE, caster, usedobj, NULL, TO_CHAR);
    act("A nasty odor eminates from $p, looks like a malfunction!", 
          FALSE, caster, usedobj, NULL, TO_CHAR);
    act("$p flickers with a red light for a second, which then slowly fades away.", 
          FALSE, caster, toenhance, NULL, TO_CHAR);
    act("$p flickers with a red light for a second, which then slowly fades away.", 
          FALSE, caster, toenhance, NULL, TO_ROOM);
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL_2)) {
    act("A nasty odor eminates from $p, looks like a malfunction!", 
          FALSE, caster, usedobj, NULL, TO_CHAR);
    act("A nasty odor eminates from $p, looks like a malfunction!", 
          FALSE, caster, usedobj, NULL, TO_CHAR);
    act("$p flares up in an explosion of white light!", 
          FALSE, caster, toenhance, NULL, TO_CHAR);
    act("$p flares up in an explosion of white light!", 
          FALSE, caster, toenhance, NULL, TO_ROOM);
    return DELETE_ITEM;
  }
  return FALSE;
}

int enhanceWeapon(TBeing *caster, TObj *obj)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_ENHANCE_WEAPON, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_ENHANCE_WEAPON]->lag;
  diff = discArray[SPELL_ENHANCE_WEAPON]->task;

  start_cast(caster, NULL, obj, caster->roomp, SPELL_ENHANCE_WEAPON, diff, 2,"", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castEnhanceWeapon(TBeing *caster, TObj *obj)
{
  int ret = 0,level;

  if (MAX_OBJ_AFFECT < 2)
    return FALSE;

  level = caster->getSkillLevel(SPELL_ENHANCE_WEAPON);

  ret=enhanceWeapon(caster,obj,level,caster->getSkillValue(SPELL_ENHANCE_WEAPON));

  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$n draws some elemental magic from the ether and guides it into $p.",
              FALSE, caster, obj, NULL, TO_ROOM);
    act("You draw some elemental magic from the ether and guide it into $p.", 
              FALSE, caster, obj, NULL, TO_CHAR);
    act("$p begins to glow with a soft yellow light.", 
          FALSE, caster, obj, NULL, TO_CHAR);
    act("$p begins to glow with a soft yellow light.", 
          FALSE, caster, obj, NULL, TO_ROOM);
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    caster->spellMessUp(SPELL_ENHANCE_WEAPON);
    act("$p flickers with a red light for a second, which then slowly fades away.", 
          FALSE, caster, obj, NULL, TO_CHAR);
    act("$p flickers with a red light for a second, which then slowly fades away.", 
          FALSE, caster, obj, NULL, TO_ROOM);
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL_2)) {
    caster->spellMessUp(SPELL_ENHANCE_WEAPON);
    act("$p flares up in an explosion of white light!", 
          FALSE, caster, obj, NULL, TO_CHAR);
    act("$p flares up in an explosion of white light!", 
          FALSE, caster, obj, NULL, TO_ROOM);
    return DELETE_ITEM;
  }
  return FALSE;
}



bool alchemy_create_deny(int numberx)
{
  objIndexData oid = obj_index[numberx];

  if (oid.value < 0) 
    return true;
  if (oid.max_exist < 1000) 
    return true;
  if (oid.spec)
    return true;

  // see if zone the obj comes from is disabled
  unsigned int zone;

  for (zone = 0; zone < zone_table.size(); zone++) {
    if (zone_table[zone].top > oid.virt) {
      // the obj belongs to this zone
      if (zone_table[zone].enabled == FALSE)
        return true;

      break;
    }
  }

  if (oid.itemtype == ITEM_WINDOW)
    return true;
  if (oid.itemtype == ITEM_MONEY)  // piles of coins
    return true;
  if (oid.itemtype == ITEM_HOLY_SYM)
    return true;
  if (oid.itemtype == ITEM_COMPONENT)
    return true;
  if (oid.itemtype == ITEM_BOOK)
    return true;
  if (oid.itemtype == ITEM_FOOD)  // domain of clerics
    return true;
  if (oid.itemtype == ITEM_STAFF)
    return true;
  if (oid.itemtype == ITEM_POTION)
    return true;
  if (oid.itemtype == ITEM_WAND)
    return true;
  if (oid.itemtype == ITEM_SCROLL)
    return true;
  if (oid.itemtype == ITEM_TREASURE)
    return true;
  if (oid.itemtype == ITEM_RAW_MATERIAL)
    return true;
  if (oid.itemtype == ITEM_RAW_ORGANIC)
    return true;
  if (oid.itemtype == ITEM_FLAME)
    return true;
  if (oid.itemtype == ITEM_KEY)
    return true;
  if (oid.itemtype == ITEM_PORTAL)
    return true;
  if (oid.itemtype == ITEM_BED)
    return true;
  if (oid.itemtype == ITEM_TABLE)
    return true;
  if (oid.itemtype == ITEM_STATUE)
    return true;
  if (oid.itemtype == ITEM_TRAP)
    return true;
  if (oid.itemtype == ITEM_CHEST)
    return true;
  if (!IS_SET(oid.where_worn, ITEM_TAKE))
    return true;
  if (isname("belt monk", oid.name))
    return true;
  if (isname("sash monk", oid.name))
    return true;
  if (isname("script bundle", oid.name))
    return true;
  if (isname("[quest_object]", oid.name))
    return true;
  if (isname("[prop_object]", oid.name))
    return true;
  if (isname("pager beeper", oid.name))
    return true;
  if (isname("muffs ear", oid.name))
    return true;

  // the above checks remain as a safeguard against putting bad objects
  // in this list
  int allowed[]={6,9,105,106,108,321,330,410,1012,3090,3091,13841,13860,
		 26686,28917,28918,28919,148,318, -1};

  for(int i=0;allowed[i]!=-1;++i){
    if(allowed[i] == oid.virt)
      return false;
  }

  return true;
}
        
int materialize(TBeing *caster, TObj **obj, int, const char * name, byte bKnown)
{
  unsigned int numberx;

  caster->sendTo("You throw the talens into the air and they wink out of existence.\n\r");
  act("$n tosses some money into the air and it magically disappears.",
      TRUE, caster, 0, 0, TO_ROOM, ANSI_YELLOW);
  caster->addToMoney(-MATERIALIZE_PRICE, GOLD_HOSPITAL);

  for (numberx = 0; numberx < obj_index.size(); numberx++) {
    if (!isname(name, obj_index[numberx].name)) 
      continue;
    if (obj_index[numberx].value > MATERIALIZE_PRICE) 
      continue;
    if (alchemy_create_deny(numberx))
      continue;
    break;
  }
  if (numberx >= obj_index.size()) {
    caster->sendTo("You get the feeling that such an item cannot be created.\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, SPELL_MATERIALIZE)) {

    int i, num;
    if (obj_index[numberx].value)
      num = MATERIALIZE_PRICE / obj_index[numberx].value;
    else
      num = 100;
    num = min(10, ::number(1,num));

    bool grabbed = false;
    for (i = 0; i < num; i++) {
      *obj = read_object(numberx, REAL);

      (*obj)->remObjStat(ITEM_NEWBIE);
      (*obj)->setEmpty();

      if (((*obj)->isPaired() && !caster->heldInPrimHand() &&
	  !caster->heldInSecHand()) ||
	  (!(*obj)->isPaired() && !caster->heldInPrimHand())) {
        caster->equipChar(*obj, caster->getPrimaryHold(), SILENT_YES);
        grabbed = true;
      } else {
        *caster->roomp += **obj;
      }
    }

    act("In a flash of light, $p appears.", TRUE, caster, *obj, NULL, TO_ROOM);
    act("In a flash of light, $p appears.", TRUE, caster, *obj, NULL, TO_CHAR);

    if (grabbed)
      act("You grab $p right out of the air.",
          TRUE, caster, *obj, NULL, TO_CHAR);

    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int materialize(TBeing *caster, const char * name)
{
  if (!bPassMageChecks(caster, SPELL_MATERIALIZE, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_MATERIALIZE]->lag;
  taskDiffT diff = discArray[SPELL_MATERIALIZE]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_MATERIALIZE, diff, 2, name, rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castMaterialize(TBeing *caster, const char * name)
{
  if (caster->getMoney() < MATERIALIZE_PRICE) {
    caster->sendTo("You don't have the money for that!\n\r");
    return FALSE;
  }

  if (!name || !*name) {
    caster->sendTo("You need to specify an item.\n\r");
    return FALSE;
  }
  if (strlen(name) < 3) {
    caster->sendTo("You must specify something more specific.\n\r");
    return FALSE;
  }

  int level = caster->getSkillLevel(SPELL_MATERIALIZE);
  TObj *obj = NULL;
  
  act("$n claps $s hands together.", TRUE, caster, NULL, NULL, TO_ROOM);
  act("You clap your hands together.", TRUE, caster, NULL, NULL, TO_CHAR);

  materialize(caster, &obj, level, name, caster->getSkillValue(SPELL_MATERIALIZE));
  return TRUE;
}

int spontaneousGeneration(TBeing *caster, TObj **obj, const char * name, int, byte bKnown)
{
  unsigned int numberx;

  caster->sendTo("You throw the talens into the air and they wink out of existence.\n\r");
  act("$n tosses some money into the air and it magically disappears.",
      TRUE, caster, 0, 0, TO_ROOM, ANSI_YELLOW);
  caster->addToMoney(-SPONT_PRICE, GOLD_HOSPITAL);

  for (numberx = 0; numberx < obj_index.size(); numberx++) {
    if (!isname(name, obj_index[numberx].name)) 
      continue;
    if (obj_index[numberx].value > SPONT_PRICE)
      continue;
    if (alchemy_create_deny(numberx))
      continue;

    break;
  }
  if (numberx >= obj_index.size()) {
    caster->sendTo("You get the feeling that such an item cannot be created.\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, SPELL_SPONTANEOUS_GENERATION)) {

    int i;
    int num;
    if (obj_index[numberx].value)
      num = SPONT_PRICE / obj_index[numberx].value;
    else
      num = 100;
    num = min(10, ::number(1,num));
    for (i = 0; i < num; i++) {
      *obj = read_object(numberx, REAL);

      (*obj)->remObjStat(ITEM_NEWBIE);
      (*obj)->setEmpty();

      if (!caster->heldInPrimHand())
        caster->equipChar(*obj, caster->getPrimaryHold(), SILENT_YES);
      else {
        *caster->roomp += **obj;
      }
    }

    act("In a flash of light, $p appears.", TRUE, caster, *obj, NULL, TO_ROOM);
    act("In a flash of light, $p appears.", TRUE, caster, *obj, NULL, TO_CHAR);

    TObj *obj2 = dynamic_cast<TObj *>(caster->heldInPrimHand());
    if (obj2 && obj2->objVnum() == (*obj)->objVnum()) {
      act("You grab $p right out of the air.",
          TRUE, caster, *obj, NULL, TO_CHAR);
    }

    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void spontaneousGeneration(TBeing *caster, const char * name)
{
  if (!bPassMageChecks(caster, SPELL_SPONTANEOUS_GENERATION, NULL))
    return;

  lag_t rounds = discArray[SPELL_SPONTANEOUS_GENERATION]->lag;
  taskDiffT diff = discArray[SPELL_SPONTANEOUS_GENERATION]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_SPONTANEOUS_GENERATION, diff, 2, name, rounds, caster->in_room, 0, 0,TRUE, 0);
}

int castSpontaneousGeneration(TBeing *caster, const char * name)
{
  TObj *obj;
  int level;

  if (caster->getMoney() < SPONT_PRICE) {
    caster->sendTo("You don't have the money for that!\n\r");
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }
  if (!name || !*name) {
    caster->sendTo("You need to specify an item.\n\r");
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }
  if (strlen(name) < 3) {
    caster->sendTo("You must specify something more specific.\n\r");
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_SPONTANEOUS_GENERATION, NULL))
    return FALSE;

  level = caster->getSkillLevel(SPELL_SPONTANEOUS_GENERATION);

  obj = NULL;

  act("$n claps $s hands together.", TRUE, caster, NULL, NULL, TO_ROOM);
  act("You clap your hands together.", TRUE, caster, NULL, NULL, TO_CHAR);

  spontaneousGeneration(caster,&obj,name,level,caster->getSkillValue(SPELL_SPONTANEOUS_GENERATION));
  return TRUE;
}

int TObj::copyMe(TBeing *caster, byte)
{
  caster->sendTo("That's not a scroll!\n\r");
  caster->nothingHappens(SILENT_YES);
  return SPELL_FAIL;
}

int copy(TBeing *caster, TObj *obj, int, byte bKnown)
{
  return obj->copyMe(caster, bKnown);
}

int TScroll::copyMe(TBeing *caster, byte bKnown)
{
  TObj *new_obj;
  int x, spell1, spell2, spell3;

  getFourValues(&x, &spell1, &spell2, &spell3);

  if((spell1!=-1 && !caster->doesKnowSkill((spellNumT)spell1)) || 
     (spell2!=-1 && !caster->doesKnowSkill((spellNumT)spell2)) ||
     (spell3!=-1 && !caster->doesKnowSkill((spellNumT)spell3))){
    caster->sendTo("You can only copy scrolls of spells that you know.\n\r");
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, SPELL_COPY)) {
    new_obj = read_object(objVnum(),VIRTUAL);
    if (!new_obj) {
      caster->nothingHappens();
      return SPELL_FAIL;
    }
    /* lets not make this a gold creating bug  -- bat */
    new_obj->obj_flags.cost = 1;
    new_obj->obj_flags.decay_time = (int)((double)caster->getSkillLevel(SPELL_COPY)*95) + 50;
    *caster->roomp += *new_obj;
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void copy(TBeing *caster, TMagicItem *usedobj, TObj * tocopy)
{
  int ret = 0;

  ret = copy(caster,tocopy,usedobj->getMagicLevel(),usedobj->getMagicLearnedness());

  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("In a flash of light, $p appears.", 
          TRUE, caster, tocopy, NULL, TO_ROOM);
    act("In a flash of light, $p appears.", 
         TRUE, caster, tocopy, NULL, TO_CHAR);
  }
}

int copy(TBeing *caster, TObj *obj)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_COPY, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_COPY]->lag;
  diff = discArray[SPELL_COPY]->task;

  start_cast(caster, NULL, obj, NULL, SPELL_COPY, diff, 2,"", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castCopy(TBeing *caster, TObj *obj)
{
  int level, ret;

  level = caster->getSkillLevel(SPELL_COPY);

  ret = copy(caster,obj,level,caster->getSkillValue(SPELL_COPY));
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("In a flash of light, $p appears.", 
          TRUE, caster, obj, NULL, TO_ROOM);
    act("In a flash of light, $p appears.", 
         TRUE, caster, obj, NULL, TO_CHAR);
  }
  return TRUE;
}

int galvanize(TBeing *caster, TObj *obj, int, byte bKnown)
{
  return obj->galvanizeMe(caster, bKnown);
}

// returns DELETE_ITEM if obj is toasted
int galvanize(TBeing *caster, TObj *obj)
{
  if (!bPassMageChecks(caster, SPELL_GALVANIZE, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_GALVANIZE]->lag;
  taskDiffT diff = discArray[SPELL_GALVANIZE]->task;

  start_cast(caster, NULL, obj, NULL, SPELL_GALVANIZE, diff, 2,"", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castGalvanize(TBeing *caster, TObj *obj)
{
  int ret = 0,level;

  // failure = scrapped item.  no item damage allowed in arena
  if (caster->roomp && caster->roomp->isRoomFlag(ROOM_ARENA)) {
    act("A magic power prevents anything from happening here.",
         FALSE, caster, 0, 0, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FALSE;
  }
  level = caster->getSkillLevel(SPELL_GALVANIZE);

  act("$n concentrates on $p, laying $s hands on it.", 
             TRUE, caster, obj, NULL, TO_ROOM);
  act("You concentrate on $p, laying your hands on it.", 
             TRUE, caster, obj, NULL, TO_CHAR);
  ret=galvanize(caster,obj,level,caster->getSkillValue(SPELL_GALVANIZE));

  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$p throbs with an aquamarine glow briefly.", 
             FALSE, caster, obj, 0, TO_CHAR);
    act("$p throbs with an aquamarine glow briefly.", 
             FALSE, caster, obj, 0, TO_ROOM);
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL_2)) {
    act("Brown goo seeps from $p.  Ooops, that didn't go quite right.", 
             FALSE, caster, obj, 0, TO_CHAR);
    act("Brown goo seeps from $p.", 
             FALSE, caster, obj, 0, TO_ROOM);
    obj->makeScraps();
    return DELETE_ITEM;
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    act("Brown goo seeps from $p.  Ooops, that didn't go quite right.", 
             FALSE, caster, obj, 0, TO_CHAR);
    act("Brown goo seeps from $p.", 
             FALSE, caster, obj, 0, TO_ROOM);
    if (obj->getStructPoints() <= 0 &&
        obj->getMaxStructPoints() >= 0) {
      obj->makeScraps();
      return DELETE_ITEM;
    }
  }
  return FALSE;
}

int ethrealGate(TBeing *caster, TObj *obj, int, byte bKnown)
{
  return SPELL_FALSE;
}

int ethrealGate(TBeing *caster, TObj *obj)
{
  if (!bPassMageChecks(caster, SPELL_ETHER_GATE, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_ETHER_GATE]->lag;
  taskDiffT diff = discArray[SPELL_ETHER_GATE]->task;

  start_cast(caster, NULL, obj, NULL, SPELL_ETHER_GATE, diff, 2,"", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castEthrealGate(TBeing *caster, TObj *obj)
{
  int level = caster->getSkillLevel(SPELL_ETHER_GATE);

  int ret=ethrealGate(caster,obj,level,caster->getSkillValue(SPELL_ETHER_GATE));

  if (IS_SET(ret, SPELL_SUCCESS)) {
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL)) {
  }
  return FALSE;
}

void TBeing::doScribe(const char *arg)
{
  char argm[MAX_INPUT_LENGTH], newarg[MAX_INPUT_LENGTH];
  sstring buf;
  TComponent *comp_gen, *comp_spell, *comp_scribe;
  TThing *t;
  spellNumT which = TYPE_UNDEFINED;
  int i;
  int want_num = 1;

  *argm = '\0';
  strcpy(argm, arg);

  if ((want_num = getabunch(argm, newarg))) {
    strcpy(argm, newarg);
  } else {
    want_num = 1;
  }

  // for (;arg && *arg && isspace(*arg); arg++);

  if (!*argm || !argm) {
    sendTo("You need to specify a scroll type to scribe!\n\r");
    return;
  }
  if (((which = searchForSpellNum(argm, EXACT_YES)) < MIN_SPELL) &&
      ((which = searchForSpellNum(argm, EXACT_NO)) < MIN_SPELL)) {
    sendTo("You can't scribe a scroll of that type.\n\r");
    return;
  }

  if(!doesKnowSkill(which)){
    sendTo("You don't know that spell.\n\r");
    return;
  }

  // find the 3 necessary pieces
  // generic component (spell == -1, type = scribe)
  comp_gen = NULL;
  // spell comp (spell = which, type = spell)
  comp_spell = NULL;
  // brew comp (spell = which, type = scribe)
  comp_scribe = NULL;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if ((t = equipment[i])) {
      t->findSomeComponent(&comp_gen, &comp_spell, &comp_scribe, which, 2);
    }
  }
  for (t = getStuff(); t; t = t->nextThing) {
    t->findSomeComponent(&comp_gen, &comp_spell, &comp_scribe, which, 2);
  }

  if (!comp_gen) {
    sendTo("You seem to be lacking a parchment.\n\r");
    return;
  }
  if (!comp_spell) {
    sendTo("You seem to be lacking the spell component.\n\r");
    return;
  }
  if (!comp_scribe) {
    sendTo("You seem to be lacking the scribe component.\n\r");
    return;
  }
  if (!doesKnowSkill(SKILL_SCRIBE)) {
    sendTo("You lack any knowledge of how to scribe scrolls.\n\r");
    return;
  }
  if (checkBusy()) {
    return;
  }
  if (isSwimming()) {
    sendTo("You can't scribe while swimming.\n\r");
    return;
  }
  if (riding) {
    sendTo("You can't brew while riding.\n\r");
    return;
  }

  // trash all items first
  int how_many = comp_scribe->getComponentCharges();
  how_many = min(how_many, comp_spell->getComponentCharges());
  how_many = min(how_many, comp_gen->getComponentCharges());

  if (how_many >= want_num) {
    how_many = want_num;
    buf = fmt("You begin to scribe %d scroll%s of %s.") %
      how_many % (how_many == 1 ? "" : "s") % discArray[which]->name;
    act(buf, FALSE, this, 0, 0, TO_CHAR);
  } else {
    buf = fmt("You only have enough to begin to scribe %d scroll%s of %s.") %
      how_many % (how_many == 1 ? "" : "s") % discArray[which]->name;
    act(buf, FALSE, this, 0, 0, TO_CHAR);
  }
  if (how_many > 1) {
    buf = fmt("$n begins to scribe some scrolls.");
    act(buf, FALSE, this, 0, 0, TO_ROOM);
  } else {
    buf = fmt("$n begins to scribe a scroll.");
    act(buf, FALSE, this, 0, 0, TO_ROOM);
  }

  buf = fmt("You use up %d charge%s of $p.") %
    how_many % (how_many == 1 ? "" : "s");
  act(buf, FALSE, this, comp_gen, 0, TO_CHAR);
  comp_gen->addToComponentCharges(-how_many);
  if (comp_gen->getComponentCharges() <= 0) {
    buf = fmt("$p is consumed in the process.");
    act(buf, FALSE, this, comp_gen, 0, TO_CHAR);
    delete comp_gen;
    comp_gen = NULL;
  }

  buf = fmt("You use up %d charge%s of $p.") %
    how_many % (how_many == 1 ? "" : "s");
  act(buf, FALSE, this, comp_scribe, 0, TO_CHAR);
  comp_scribe->addToComponentCharges(-how_many);
  if (comp_scribe->getComponentCharges() <= 0) {
    buf = fmt("$p is consumed in the process.");
    act(buf, FALSE, this, comp_scribe, 0, TO_CHAR);
    delete comp_scribe;
    comp_scribe = NULL;
  }
  
  buf = fmt("You use up %d charge%s of $p.") %
          how_many % (how_many == 1 ? "" : "s");
  act(buf, FALSE, this, comp_spell, 0, TO_CHAR);
  comp_spell->addToComponentCharges(-how_many);
  if (comp_spell->getComponentCharges() <= 0) {
    buf = fmt("$p is consumed in the process.");
    act(buf, FALSE, this, comp_spell, 0, TO_CHAR);
    delete comp_spell;
    comp_spell = NULL;
  }

  if(task){
    stopTask();
  }

  start_task(this, NULL, NULL, TASK_SCRIBING, "", 0, in_room, how_many, which, 0);

  return;
}
