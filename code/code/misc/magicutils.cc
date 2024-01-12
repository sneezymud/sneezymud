// magicutils.cc

#include <boost/format.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <vector>

#include "ansi.h"
#include "being.h"
#include "comm.h"
#include "connect.h"
#include "db.h"
#include "defs.h"
#include "discipline.h"
#include "disease.h"
#include "enum.h"
#include "extern.h"
#include "faction.h"
#include "gametime.h"
#include "handler.h"
#include "immunity.h"
#include "limbs.h"
#include "liquids.h"
#include "log.h"
#include "low.h"
#include "monster.h"
#include "obj.h"
#include "obj_component.h"
#include "obj_key.h"
#include "obj_open_container.h"
#include "parse.h"
#include "person.h"
#include "room.h"
#include "sound.h"
#include "spell2.h"
#include "spells.h"
#include "sstring.h"
#include "stats.h"
#include "structs.h"
#include "thing.h"
#include "toggle.h"

void TMonster::balanceMakeNPCLikePC() {
  float scalingValue = ::number(65, 85) / 100.0;

  // modify HP
  float hpl = getHPLevel() * scalingValue;
  setHPLevel(hpl);
  setHPFromHPLevel();

  // modify damage capacity
  float daml = getDamLevel();
  daml /= scalingValue;
  setDamLevel(daml);
}

// set AC, hp, damage, to hit adjustment for poly'd mages and shamans
void setCombatStats(TBeing* ch, TBeing* mob, PolyType shape, spellNumT skill) {
  TMonster* critter = dynamic_cast<TMonster*>(mob);
  if (!critter) {
    vlogf(LOG_BUG,
      "in setCombatStats, couldn't cast as TMonster, getting out!");
    return;
  }

  // get effective level = (skill_learn) * min(mob level, mage level)
  int level = min(shape.level, (int)ch->GetMaxLevel());
  int polySkill = ch->getSkillValue(skill);
  level = level * polySkill / 100;
  level = max(1, level);
  // set AC
  critter->setACLevel(level);
  critter->setACFromACLevel();

  // set max hp
  float newHit = (float)ch->hitLimit();
  newHit /= ch->getConHpModifier();
  newHit *= critter->getConHpModifier();
  critter->setMaxHit((int)newHit);

  // set current hp
  critter->setHit(ch->getHit() * critter->hitLimit() / ch->hitLimit());

  // set number of attacks
  critter->setMult(ch->getMult());

  // set damage
  float damagemod =
    1.9 /
    critter->getMult();  // increase damage, since NPCs generally do 2-fold less
  critter->setDamLevel(level * damagemod);
  critter->setDamPrecision(20);

  // set hit adjustment
  critter->setHitroll(0);
}

// set AC, hp, damage, to hit adjustment for disguised mobs (werewolves too)
void setDisguiseCombatStats(TBeing* ch, TBeing* mob) {
  TMonster* critter = dynamic_cast<TMonster*>(mob);
  if (!critter) {
    vlogf(LOG_BUG,
      "in setDisguiseCombatStats, couldn't cast as TMonster, getting out!");
    return;
  }

  int level = 1;

  // set AC
  critter->setACLevel(level);
  critter->setACFromACLevel();

  // set max hp and hp
  critter->setMaxHit(ch->hitLimit());
  critter->setHit(ch->getHit());

  // set number of attacks
  critter->setMult(ch->getMult());

  // set damage
  float damagemod =
    2 /
    critter->getMult();  // increase damage, since NPCs generally do 2-fold less
  critter->setDamLevel(level * damagemod);
  critter->setDamPrecision(20);

  // set hit adjustment
  critter->setHitroll(0);
}

// adds player name to disguised / polyed creatures
void appendPlayerName(TBeing* ch, TBeing* mob) {
  if (!ch->name.empty()) {
    sstring tStNewNameList(mob->name);
    tStNewNameList += " ";
    tStNewNameList += ch->getNameNOC(ch);
    tStNewNameList += " switched";

    mob->name = tStNewNameList;
  } else
    vlogf(LOG_BUG, "Entered appendPlayerName with ch->name undefined");
}

void switchStat(statTypeT stat, TBeing* giver, TBeing* taker) {
  // assumes mob age mod is zero
  taker->setStat(STAT_CHOSEN, stat,
    giver->getStat(STAT_TERRITORY, stat) -
      taker->getStat(STAT_TERRITORY, stat) + giver->getStat(STAT_RACE, stat) -
      taker->getStat(STAT_RACE, stat) + giver->getStat(STAT_CHOSEN, stat));
  taker->setStat(STAT_CURRENT, stat, taker->getStat(STAT_NATURAL, stat));
}

void SwitchStuff(TBeing* giver, TBeing* taker, bool setStats) {
  TThing* t;
  classIndT cit;  // used as iterator to pass through classes

  mud_assert(giver != nullptr, "Something bogus in SwitchStuff()");
  mud_assert(taker != nullptr, "Something bogus in SwitchStuff()");

  // transfer toggles - do this first to avoid issues with skill swaps
  for (int tmpnum = 1; tmpnum < MAX_TOG_INDEX; tmpnum++) {
    if (giver->hasQuestBit(tmpnum) && !taker->hasQuestBit(tmpnum))
      taker->setQuestBit(tmpnum);
    if (taker->hasQuestBit(tmpnum) && !giver->hasQuestBit(tmpnum))
      taker->remQuestBit(tmpnum);
  }

  // resolve riding
  TBeing* tbr;
  if ((tbr = dynamic_cast<TBeing*>(giver->riding))) {
    giver->dismount(POSITION_STANDING);
    // this must precede deletion of taker->discs, below
    if (taker->isHumanoid()) {
      taker->doMount("", CMD_MOUNT, tbr, SILENT_YES);
    }
  }

  wearSlotT ij;
  for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
    if (giver->equipment[ij]) {
      *taker += *giver->unequip(ij);
    }
  }

  for (StuffIter it = giver->stuff.begin(); it != giver->stuff.end();) {
    t = *(it++);
    --(*t);
    *taker += *t;
  }
  taker->setMoney(giver->getMoney());

  taker->setExp(giver->getExp());
  taker->setMaxExp(giver->getMaxExp());

  // set up practicess
  for (cit = MIN_CLASS_IND; cit < MAX_CLASSES; cit++)
    taker->setPracs(giver->getPracs(cit), cit);

  // this stuff is passed one way to the mob, shouldn't be stuff that doesn't
  // change
  // note that disciplines are included here - no learning while poly'd?
  if (dynamic_cast<TMonster*>(taker)) {
    taker->setClass(giver->getClass());

    for (cit = MIN_CLASS_IND; cit < MAX_CLASSES; cit++)
      taker->setLevel(cit, giver->getLevel(cit));
    taker->calcMaxLevel();

    // transfer skills and disciplines
    delete taker->discs;
    taker->discs = giver->discs;

    // transfer a reasonable amount of max moves
    taker->setMaxMove(giver->getMaxMove() / 2);
    // note that the getMaxMove function is different for the mob
    // and the pc

    taker->setMaxMana(giver->manaLimit());

    if (setStats) {
      // can only change chosen... SO:

      // for physical (most) stats, keep chosens and territory adjustments
      // charisma and perception are included here, rightly or wrongly
      statTypeT iStat;
      for (iStat = MIN_STAT; iStat < MAX_STATS; iStat++) {
        if (iStat == STAT_INT || iStat == STAT_WIS || iStat == STAT_FOC ||
            iStat == STAT_KAR)
          continue;
        taker->setStat(STAT_CHOSEN, iStat,
          giver->getStat(STAT_CHOSEN, iStat) +
            giver->getStat(STAT_TERRITORY, iStat) -
            taker->getStat(STAT_TERRITORY, iStat));
        taker->setStat(STAT_CURRENT, iStat, STAT_NATURAL);
      }

      // for mentals and karma, keep the same race
      switchStat(STAT_INT, giver, taker);
      switchStat(STAT_WIS, giver, taker);
      switchStat(STAT_FOC, giver, taker);
      switchStat(STAT_KAR, giver, taker);
    }
  }
  if (dynamic_cast<TMonster*>(giver))
    giver->discs = nullptr;

  taker->setPiety(giver->getPiety());
  taker->setMana(giver->getMana());
  taker->setMove(giver->getMove());
  taker->setLifeforce(giver->getLifeforce());

  taker->setFaction(giver->getFaction());
#if FACTIONS_IN_USE
  taker->setPerc(giver->getPerc());
  for (factionTypeT i = MIN_FACTION; i < MAX_FACTIONS; i++)
    taker->setPercX(giver->getPercX(i), i);
#endif
  taker->setFactAct(giver->getFactAct());
  taker->setCaptive(giver->getCaptive());
  taker->setNextCaptive(giver->getNextCaptive());
  taker->setCaptiveOf(giver->getCaptiveOf());

  taker->age_mod = giver->age_mod;
  taker->desc = giver->desc;
  *taker->player.time = *giver->player.time;

  taker->setSex(giver->getSex());

  // transfer spells and skills and oddball effects (disease, drunk, ...)
  giver->polyAffectJoin(taker);
}

void DisguiseStuff(TBeing* giver, TBeing* taker) {
  // do the generic polymorph stuff first, with setStats = false
  SwitchStuff(giver, taker, false);

  if (dynamic_cast<TMonster*>(taker)) {
    statTypeT iStat;
    // must cast moves, only transfer max moves to monster (NOT back again)
    for (iStat = MIN_STAT; iStat < MAX_STATS; iStat++) {
      switchStat(iStat, giver, taker);
    }
    setDisguiseCombatStats(giver, taker);
  }
}

void TMonster::failCharm(TBeing* ch) {
  sendTo("You feel charmed, but the feeling fades.\n\r");
  setCharFighting(ch);
}

void TPerson::failCharm(TBeing*) {
  sendTo("You feel charmed, but the feeling fades.\n\r");
}

void TBeing::failSleep(TBeing* ch) {
  sendTo("You feel sleepy for a moment, but then you recover.\n\r");
  if (dynamic_cast<TMonster*>(this))
    if (toggleInfo[TOG_SLEEP]->toggle && !fight() &&
        (getPosition() > POSITION_SLEEPING))
      setCharFighting(ch);
}

void TBeing::failPara(TBeing* ch) {
  sendTo("You feel frozen for a moment, but then you recover.\n\r");
  if (dynamic_cast<TMonster*>(this))
    if (!fight())
      setCharFighting(ch);
}

void TBeing::failCalm(TBeing* ch) {
  sendTo("You feel happy and easy-going, but the effect soon fades.\n\r");
  if (dynamic_cast<TMonster*>(this))
    if (!fight())
      setCharFighting(ch);
}

void TBeing::spellWearOffSoon(spellNumT s) {
  if (s == AFFECT_TRANSFORMED_HANDS || s == AFFECT_TRANSFORMED_ARMS ||
      s == AFFECT_TRANSFORMED_LEGS || s == AFFECT_TRANSFORMED_NECK ||
      s == AFFECT_TRANSFORMED_HEAD)
    s = SKILL_TRANSFORM_LIMB;

  if (s < MIN_SPELL || s >= MAX_SKILL || !discArray[s])
    return;

  if (s == AFFECT_WAS_INDOORS && toggleInfo[TOG_QUESTCODE4]->toggle) {
    sendTo(
      "You begin to shiver and wonder how much longer you can stay outdoors "
      "before catching frostbite.\n\r");
    act("$n's teeth begin to chatter.", true, this, 0, 0, TO_ROOM);
  }

  if (discArray[s]->fadeAwaySoon)
    sendTo(format("%s\n\r") % discArray[s]->fadeAwaySoon);

  if (discArray[s]->fadeAwaySoonRoom)
    act(discArray[s]->fadeAwaySoonRoom, true, this, 0, 0, TO_ROOM);
}

// return DELETE_THIS
int TBeing::spellWearOff(spellNumT s, safeTypeT safe) {
  // Arguably, we should see effects falling off due to death, but it
  // looks real dumb on mobs...
  //  if (!isPc() && getPosition() == POSITION_DEAD)
  if (!isPc() && safe)
    return false;

  int rc;

  if (s == AFFECT_TRANSFORMED_HANDS || s == AFFECT_TRANSFORMED_ARMS ||
      s == AFFECT_TRANSFORMED_LEGS || s == AFFECT_TRANSFORMED_NECK ||
      s == AFFECT_TRANSFORMED_HEAD)
    s = SKILL_TRANSFORM_LIMB;

  // Notify players when the fortify defensive bonus expires
  if (s == AFFECT_FORTIFY) {
    sendTo(
      "You are unable to continue maintaining your shield wall any "
      "longer.\n\r");
    act(
      "$n lowers $s shield and relaxes his defensive posture, unable to "
      "continue maintaining $s shield wall.",
      true, this, 0, 0, TO_ROOM);
  }

  if (s < MIN_SPELL || s >= MAX_SKILL || !discArray[s])
    return 0;

  if (s == AFFECT_WAS_INDOORS && toggleInfo[TOG_QUESTCODE4]->toggle) {
    sendTo(
      "You're getting really cold now, and can barely feel your limbs.\n\r");
    act("$n begins to shiver violently, and looks pale.", true, this, 0, 0,
      TO_ROOM);
  }

  if (discArray[s]->fadeAway)
    sendTo(format("%s\n\r") % discArray[s]->fadeAway);

  if (discArray[s]->fadeAwayRoom)
    act(discArray[s]->fadeAwayRoom, true, this, 0, 0, TO_ROOM);

  if (s == SPELL_ENSORCER || s == SPELL_HYPNOSIS || s == SPELL_CONJURE_AIR ||
      s == SPELL_CONJURE_EARTH || s == SPELL_CONJURE_FIRE ||
      s == SPELL_CONJURE_WATER || s == SPELL_CONTROL_UNDEAD ||
      s == SPELL_VOODOO || s == SPELL_RESURRECTION ||
      s == SPELL_DANCING_BONES) {
    if (!isPc()) {
      rc = checkDecharm(FORCE_NO, safe);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    }
  }

  if (s == SPELL_GILLS_OF_FLESH) {
    // safe = already dead probably, so lets skip "panic your drowning" and
    // all the rest of what checkDrowning does too...
    if (!safe) {
      rc = checkDrowning();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    }
  }

  return false;
}

// returns DELETE_THIS
int TBeing::checkDecharm(forceTypeT force, safeTypeT safe) {
  int j, rc = 0, mastLevel = 1;
  bool release = false;
  TBeing* m;

  if (!(m = master) && !force) {
    return false;
  } else if (m) {
    mastLevel = m->GetMaxLevel();
  }
  mastLevel = 0 - mastLevel;

  stopFollower(false);  // stop following the master

  if (!isPc()) {
    int mVn = mobVnum();
    bool isElemental =
      (mVn == Mob::FIRE_ELEMENTAL || mVn == Mob::WATER_ELEMENTAL ||
        mVn == Mob::EARTH_ELEMENTAL || mVn == Mob::AIR_ELEMENTAL);
    TMonster* tMon = dynamic_cast<TMonster*>(this);
    if (force) {
      release = true;
    } else if (isElemental && tMon &&
               (::number(mastLevel, tMon->anger()) <= 0)) {
      release = true;
    }
    if (stuff.empty()) {
      for (j = MIN_WEAR; j < MAX_WEAR; j++) {
        if (equipment[j]) {
          release = false;
          break;
        }
      }
    }
    if (!isElemental) {
      if (m) {
        act("$n looks elated as you release $m into the world.", true, this,
          nullptr, m, TO_VICT);
        act("$n looks elated as $e is released into the world.", true, this,
          nullptr, m, TO_NOTVICT);
        act("You feel free as your master releases you into the world.", true,
          this, nullptr, m, TO_CHAR);
      } else {
        act("$n is released into the world.", true, this, nullptr, nullptr, TO_ROOM);
        act("You are released into the world.", true, this, nullptr, nullptr,
          TO_CHAR);
      }
      return false;
    }
    if (release && !safe) {
      if (!m || !::number(0, 1)) {
        act("$n breaks free of the mortal plane and returns home.", true, this,
          nullptr, nullptr, TO_ROOM);
        act("You break free of the mortal plane and return home.", true, this,
          nullptr, nullptr, TO_CHAR);
      } else {
        act("$n growls at you then slowly fades away.", false, this, nullptr, m,
          TO_VICT);
        act("$n growls at $N then slowly fades away.", false, this, nullptr, m,
          TO_NOTVICT);
        act("You growl at $n then goes back home.", false, this, nullptr, m,
          TO_CHAR);
      }

      doDrop("all talens", nullptr);
      doDrop("all", nullptr);

      return DELETE_THIS;
    }

    REMOVE_BIT(specials.act, ACT_SENTINEL);
    dynamic_cast<TMonster*>(this)->addFeared(m);
  }
  if (!safe) {
    rc = doFlee("");
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return false;
}

void pick_best_comp(TComponent** best, TThing* item, spellNumT spell) {
  TComponent* comp = dynamic_cast<TComponent*>(item);
  if (!comp)
    return;

  // ensure it's the proper component
  if (!(comp->getComponentSpell() == spell &&
        comp->isComponentType(COMP_SPELL)))
    return;

  // use the one with least charges left
  if (!(*best) ||
      (comp->getComponentCharges() < (*best)->getComponentCharges()))
    *best = comp;
}

TComponent* comp_from_object(TThing* item, spellNumT spell) {
  TComponent* ret = nullptr;

  pick_best_comp(&ret, item, spell);

  TOpenContainer* trc = dynamic_cast<TOpenContainer*>(item);
  if (trc && trc->isClosed())
    return ret;

  // item is wrong component or spellbag
  for (StuffIter it = item->stuff.begin(); it != item->stuff.end(); ++it)
    pick_best_comp(&ret, *it, spell);

  return ret;
}

// This only returns components that are for spell-casting
//
// TODO: this is a mess and doesn't work for multiclass but
// probably no one will notice because ritualism and wizardry
// rise at the same rate.
//
TComponent* TBeing::findComponent(spellNumT spell) const {
  TThing *primary, *secondary, *belt, *juju, *wristpouchL, *wristpouchR;
  TComponent* item;
  wizardryLevelT wizlevel = WIZ_LEV_NONE;
  ritualismLevelT ritlevel = RIT_LEV_NONE;

  primary = heldInPrimHand();
  secondary = heldInSecHand();
  belt = equipment[WEAR_WAIST];
  juju = equipment[WEAR_NECK];
  wristpouchL = equipment[WEAR_WRIST_L];
  wristpouchR = equipment[WEAR_WRIST_R];
  item = nullptr;

  // Let rangers have components anywhere if not fighting
  if (hasClass(CLASS_RANGER)) {
    if (fight())
      wizlevel = WIZ_LEV_COMP_EITHER;
    else
      wizlevel = WIZ_LEV_COMP_BELT;
  } else {
    if (hasClass(CLASS_SHAMAN)) {
      ritlevel = getRitualismLevel();
    } else {
      wizlevel = getWizardryLevel();
    }
  }
  if (hasClass(CLASS_SHAMAN)) {
    if (isPc()) {
      if (ritlevel <= RIT_LEV_COMP_PRIM_OTHER_FREE) {
        if (primary)
          return comp_from_object(primary, spell);
        else
          return nullptr;
      }
      if (ritlevel <= RIT_LEV_COMP_EITHER) {
        if (primary || secondary) {
          if (primary)
            item = comp_from_object(primary, spell);
          if (!item && secondary)
            item = comp_from_object(secondary, spell);
          return item;
        } else
          return nullptr;
      }
      if (ritlevel <= RIT_LEV_NO_MANTRA) {
        if (primary || secondary || !stuff.empty()) {
          if (primary)
            item = comp_from_object(primary, spell);
          if (!item && secondary)
            item = comp_from_object(secondary, spell);
          if (!item && !stuff.empty()) {
            for (StuffIter it = stuff.begin(); it != stuff.end() && !item;
                 ++it) {
              TObj* o = dynamic_cast<TObj*>(*it);
              if (o)
                item = comp_from_object(o, spell);
            }
          }
          return item;
        } else
          return nullptr;
      }
    }
    if (primary || secondary || belt || juju || wristpouchL || wristpouchR ||
        !stuff.empty()) {
      if (primary)
        item = comp_from_object(primary, spell);
      if (!item && secondary)
        item = comp_from_object(secondary, spell);
      if (!item && belt)
        item = comp_from_object(belt, spell);
      if (!item && juju)
        item = comp_from_object(juju, spell);
      if (!item && wristpouchL)
        item = comp_from_object(wristpouchL, spell);
      if (!item && wristpouchR)
        item = comp_from_object(wristpouchR, spell);
      if (!item && !stuff.empty()) {
        for (StuffIter it = stuff.begin(); it != stuff.end() && !item; ++it) {
          TObj* o = dynamic_cast<TObj*>(*it);
          if (o)
            item = comp_from_object(o, spell);
        }
      }
      return item;
    } else
      return nullptr;
  } else {
    if (isPc()) {
      if (wizlevel <= WIZ_LEV_COMP_PRIM_OTHER_FREE) {
        if (primary)
          return comp_from_object(primary, spell);
        else
          return nullptr;
      }
      if (wizlevel <= WIZ_LEV_COMP_EITHER) {
        if (primary || secondary) {
          if (primary)
            item = comp_from_object(primary, spell);
          if (!item && secondary)
            item = comp_from_object(secondary, spell);
          return item;
        } else
          return nullptr;
      }
      if (wizlevel <= WIZ_LEV_NO_MANTRA) {
        if (primary || secondary || !stuff.empty()) {
          if (primary)
            item = comp_from_object(primary, spell);
          if (!item && secondary)
            item = comp_from_object(secondary, spell);
          if (!item && !stuff.empty()) {
            for (StuffIter it = stuff.begin(); it != stuff.end() && !item;
                 ++it) {
              TObj* o = dynamic_cast<TObj*>(*it);
              if (o)
                item = comp_from_object(o, spell);
            }
          }
          return item;
        } else
          return nullptr;
      }
    }
    if (primary || secondary || belt || juju || wristpouchL || wristpouchR ||
        !stuff.empty()) {
      if (primary)
        item = comp_from_object(primary, spell);
      if (!item && secondary)
        item = comp_from_object(secondary, spell);
      if (!item && belt)
        item = comp_from_object(belt, spell);
      if (!item && juju)
        item = comp_from_object(juju, spell);
      if (!item && wristpouchL)
        item = comp_from_object(wristpouchL, spell);
      if (!item && wristpouchR)
        item = comp_from_object(wristpouchR, spell);
      if (!item && !stuff.empty()) {
        for (StuffIter it = stuff.begin(); it != stuff.end() && !item; ++it) {
          TObj* o = dynamic_cast<TObj*>(*it);
          if (o)
            item = comp_from_object(o, spell);
        }
      }
      return item;
    } else
      return nullptr;
  }
}

static void missingComponent(const TBeing* ch) {
  if (ch->hasClass(CLASS_RANGER)) {
    if (ch->fight()) {
      ch->sendTo(
        "You are unable to concentrate on casting while fighting without your "
        "components in hand.\n\r");
    } else
      ch->sendTo(
        "You seem to lack the proper materials to complete this magic "
        "skill.\n\r");
  } else {
    ch->sendTo(
      "You seem to lack the proper materials to complete your task.\n\r");
    act("$n kicks $mself as $e realizes $e just screwed up.", true, ch, 0, 0,
      TO_ROOM);
  }
}

int TBeing::useComponent(TComponent* o, TBeing* vict, checkOnlyT checkOnly) {
  unsigned int i;

  if (isImmortal() && isPlayerAction(PLR_NOHASSLE))
    return true;

  // Until a better solution, mobs dont need components. - Russ
  if (!isPc())
    return true;

  // has already used component
  if (spelltask && spelltask->component)
    return true;

  if (!o) {
    missingComponent(this);
    return false;
  }
  if (checkOnly)
    return true;

  for (i = 0; (i < CompInfo.size()) &&
              (o->getComponentSpell() != CompInfo[i].spell_num);
       i++)
    ;

  if (i >= CompInfo.size()) {
    vlogf(LOG_BUG, format("useComponent had problem finding component for %s") %
                     o->getName());
    sendTo("Uh oh, something bogus happened.\n\r");
    return false;
  }
  if (o->isPersonalized() && !isname(getName(), o->name)) {
    vlogf(LOG_MISC, format("Mage %s using component %s that was personalized "
                           "but not theirs!!! Reprimand at once.") %
                      getName() % o->name);
    sendTo("You can't use a component that is personalized for someone else!");
    return false;
  }
  if (vict && vict != this) {
    if (*CompInfo[i].to_caster && *CompInfo[i].to_vict &&
        *CompInfo[i].to_other) {
      act(CompInfo[i].to_caster, true, this, o, vict, TO_CHAR);
      act(CompInfo[i].to_vict, true, this, o, vict, TO_VICT);
      act(CompInfo[i].to_other, true, this, o, vict, TO_NOTVICT);
    } else if (*CompInfo[i].to_self && *CompInfo[i].to_room) {
      act(CompInfo[i].to_self, true, this, o, 0, TO_CHAR);
      act(CompInfo[i].to_room, true, this, o, 0, TO_ROOM);
    } else {
      vlogf(LOG_BUG, format("Bad component sstring.  component %d  (1)") % i);
    }
  } else {
    if (*CompInfo[i].to_self && *CompInfo[i].to_room) {
      act(CompInfo[i].to_self, true, this, o, 0, TO_CHAR);
      act(CompInfo[i].to_room, true, this, o, 0, TO_ROOM);
    } else {
      vlogf(LOG_BUG, format("Bad component sstring.  component %d  (2)") % i);
    }
  }

  // use up one charge
  if (o->getComponentCharges() > 1)
    o->addToComponentCharges(-1);
  else {
    // this should destroy it even if it is inside a spellbag
    act("$p is all used up and you discard it as worthless.", true, this, o, 0,
      TO_CHAR);
    delete o;
    o = nullptr;
  }
  // Set it so that for chanting purposes the component has been used
  if (spelltask)
    spelltask->component = true;

  return 1;
}

int TBeing::useComponentObj(TComponent* o, TObj* targ, checkOnlyT checkOnly) {
  unsigned int i;

  if (isImmortal())
    return true;

  // Until a better solution, mobs dont need components. - Russ
  if (!isPc())
    return true;

  // has already used component
  if (spelltask && spelltask->component) {
    return true;
  }

  if (!o) {
    missingComponent(this);
    return false;
  }
  if (checkOnly) {
    return true;
  }

  for (i = 0; (i < CompInfo.size()) &&
              (o->getComponentSpell() != CompInfo[i].spell_num);
       i++)
    ;

  if (i >= CompInfo.size()) {
    vlogf(LOG_BUG, format("useComponent had problem finding component for %s") %
                     o->shortDescr);
    sendTo("Uh oh, something bogus happened.\n\r");
    return false;
  }

  if (targ) {
    if (*CompInfo[i].to_self_object && *CompInfo[i].to_room_object) {
      act(CompInfo[i].to_self_object, true, this, o, targ, TO_CHAR);
      act(CompInfo[i].to_room_object, true, this, o, targ, TO_NOTVICT);
    } else if (*CompInfo[i].to_caster && *CompInfo[i].to_other) {
      act(CompInfo[i].to_caster, true, this, o, targ, TO_CHAR);
      act(CompInfo[i].to_other, true, this, o, targ, TO_ROOM);
    } else {
      vlogf(LOG_BUG, format("Bad component sstring.  component %d  (3)") % i);
    }
  } else {
    vlogf(LOG_BUG, format("Bad component sstring.  component %d  (4)") % i);
  }

  if (o->getComponentCharges() > 1)
    o->addToComponentCharges(-1);
  else {
    // this should destroy it even if it is inside a spellbag
    act("$p is all used up and you discard it as worthless.", true, this, o, 0,
      TO_CHAR);
    delete o;
    o = nullptr;
  }
  // Set it so that for chanting purposes the component has been used
  if (spelltask)
    spelltask->component = true;

  return 1;
}

int TBeing::mostPowerstoneMana() const {
  TThing* t;
  int i, most = 0;

  // Check through char's equipment
  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (!(t = equipment[i]))
      continue;

    t->powerstoneMostMana(&most);
  }
  // Check through char's inventory
  for (StuffIter it = stuff.begin(); it != stuff.end() && (t = *it); ++it) {
    t->powerstoneMostMana(&most);
  }
  return most;
}

const char* describe_level(int n) {
  if (n < 10)
    return "very low level";
  else if (n < 20)
    return "low level";
  else if (n < 40)
    return "medium level";
  else if (n < 50)
    return "high level";
  else if (n < 60)
    return "very high level";
  else
    return "extremely high level";
}

const char* describe_damage(int n, const TBeing* tBeing) {
#if 1
  if (!tBeing)
    return "a perfect amount";

  int tDiff = n - tBeing->GetMaxLevel();

  if (tDiff < -20)
    return "a horrid amount";
  else if (tDiff < -15)
    return "a sad amount";
  else if (tDiff < -10)
    return "a pathetic amount";
  else if (tDiff < -5)
    return "a decent amount";
  else if (tDiff <= -1)
    return "a near perfect amount";
  else if (tDiff == 0)
    return "a Perfect amount";
  else if (tDiff <= 2)
    return "a near perfect amount";  // This and -1 is where we confuse them.
  else if (tDiff < 5)
    return "a good amount";
  else if (tDiff < 10)
    return "a really good amount";
  else if (tDiff < 15)
    return "an extremely good amount";
  else
    return "way too much of an amount";

#else
  if (n < 3)
    return "a trivial amount";
  else if (n < 8)
    return "a fair amount";
  else if (n < 14)
    return "a moderate amount";
  else if (n < 18)
    return "quite a bit";
  else if (n < 25)
    return "a lot";
  else if (n < 35)
    return "an awful lot";
  else if (n < 45)
    return "an awesome amount";
  else
    return "a unexplainable amount";
#endif
}

const char* describe_armor(int n) {
  if (n < 1)
    return "no";
  else if (n < 3)
    return "very little";
  else if (n < 8)
    return "little";
  else if (n < 14)
    return "some";
  else if (n < 18)
    return "a good amount of";
  else if (n < 25)
    return "a lot of";
  else if (n < 35)
    return "an awful lot of";
  else if (n < 45)
    return "an awesome amount of";
  else
    return "a unexplainable amount of";
}

const char* describe_light(int n) {
  if (n < 3)
    return "dim";
  else if (n < 8)
    return "moderately-bright";
  else if (n < 15)
    return "bright";
  else if (n < 25)
    return "very bright";
  else if (n < 35)
    return "extremely intense";
  else
    return "blinding";
}

const char* what_does_it_open(const TKey* o) {
  int i, x, vnum;
  TRoom* rp;
  roomDirData* ex;

  vnum = o->objVnum();
  for (i = 0; i < WORLD_SIZE; i++) {  // check if it opens a door
    if ((rp = real_roomp(i))) {
      for (x = 0; x < 10; x++) {
        if ((ex = rp->dir_option[x]) && (ex->key == vnum)) {
          if (IS_SET(ex->condition, EXIT_SECRET))
            return "a secret door";
          else
            return "a door";
        }
      }
    }
  }
  for (TObjIter iter = object_list.begin(); iter != object_list.end(); ++iter) {
    TOpenContainer* trc = dynamic_cast<TOpenContainer*>(*iter);
    if (trc) {
      if (trc->getKeyNum() == vnum)
        return "a container";
    }
  }

  return "something undetectable";
}

// DELETE_THIS  means c got toasted
// DELETE_VICT  means v got toasted
int TBeing::rawSummon(TBeing* v) {
  TBeing* tmp = nullptr;
  TThing* t;
  int rc;
  wearSlotT j;

  act("You hear a small \"pop\" as $n disappears.", false, v, nullptr, nullptr,
    TO_ROOM);
  --(*v);
  *roomp += *v;

  act("You hear a small \"pop\" as $n appears in the middle of the room.", true,
    v, nullptr, nullptr, TO_ROOM);
  act("$n has summoned you!", false, this, nullptr, v, TO_VICT);
  v->doLook("", CMD_LOOK);

  if (!v->isPc() && (v->GetMaxLevel() > GetMaxLevel())) {
    act("$N struggles, and all of $S items are destroyed!", true, this, nullptr, v,
      TO_CHAR);
    for (j = MIN_WEAR; j < MAX_WEAR; j++) {  // remove objects from victim
      if (v->equipment[j] && !dynamic_cast<TKey*>(v->equipment[j])) {
        TThing* t_o = v->unequip(j);
        delete t_o;
        t_o = nullptr;
      }
    }
    for (StuffIter it = v->stuff.begin(); it != v->stuff.end();) {
      t = *(it++);
      if (!dynamic_cast<TKey*>(t)) {
        --(*t);
        delete t;
        t = nullptr;
      }
    }
    v->addHated(this);
  } else
    addToWait(combatRound(2));

  if (v->riding) {
    rc = v->fallOffMount(v->riding, POSITION_STANDING);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_VICT;
    }
  }
  while ((t = v->rider)) {
    rc = t->fallOffMount(v, POSITION_STANDING);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t;
      t = nullptr;
    }
  }

#if 0
  // too easy to abuse for PK, and somewhat silly too
  v->addToMove(-100);
  v->setMove(max(0, v->getMove()));
  v->updatePos();
#endif

  act("You are exhausted from interplanar travel.", false, v, nullptr, nullptr,
    TO_CHAR);
  act("$n is exhausted from interplanar travel.", false, v, nullptr, nullptr,
    TO_ROOM);

  // summon newbie to aggro zone far from GH, allow us to check for it
  vlogf(LOG_SILENT, format("%s summoned %s to %s (%d)") % getName() %
                      v->getName() % roomp->getName() % inRoom());

  if (v->riding) {
    rc = v->riding->genericMovedIntoRoom(roomp, -1);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete v->riding;
      v->riding = nullptr;
      REM_DELETE(rc, DELETE_THIS);
    }
  } else {
    rc = v->genericMovedIntoRoom(roomp, -1);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;
  }

  for (StuffIter it = v->roomp->stuff.begin(); it != v->roomp->stuff.end();) {
    t = *(it++);
    if (!(tmp = dynamic_cast<TBeing*>(t)))
      continue;
    if (!tmp->isPc() && ((IS_SET(tmp->specials.act, ACT_AGGRESSIVE)))) {
      act("$n sneers at you.", 1, tmp, nullptr, this, TO_VICT);
      act("$n sneers at $N. Uh oh...there's gonna be a RUMBLE!", 1, tmp, nullptr,
        this, TO_NOTVICT);

      rc = dynamic_cast<TMonster*>(tmp)->takeFirstHit(*this);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        return DELETE_THIS;
      } else if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tmp;
        tmp = nullptr;
        return false;
      }
    }
  }

  disturbMeditation(v);

  return false;
}

void TBeing::addAffects(const TObj* o) {
  int i;

  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (o->affected[i].location != APPLY_NONE)
      affectModify(o->affected[i].location, (long)o->affected[i].modifier,
        (long)o->affected[i].modifier2, o->obj_flags.bitvector, true,
        SILENT_YES);
    else
      return;
}

// returns DELETE_THIS
int TThing::genericTeleport(silentTypeT silent, bool keepZone, bool unsafe) {
  int to_room;
  TRoom* rp;
  int rc;
  TThing* t;
  int breakout = 0;

  for (;;) {
    // this keeps getting caught in a loop on builder mud
    // and I don't want to fix it properly.
    if (++breakout > 1000000) {  // presumably we won't ever have > 1 mil rooms
      vlogf(LOG_BUG, "genericTeleport got caught in a loop");
      return false;
    }

    // note, all rooms below 100 are ignored

    if (keepZone) {
      int minroom = zone_table[roomp->getZoneNum() - 1].top + 1;
      int maxroom = zone_table[roomp->getZoneNum()].top;
      to_room = ::number(minroom, maxroom);
    } else {
      to_room = ::number(100, top_of_world);
    }
    if (!(rp = real_roomp(to_room)))
      continue;
    if (zone_table[rp->getZoneNum()].enabled == false)
      continue;

    if (!unsafe) {
      if (rp->isRoomFlag(ROOM_PRIVATE))
        continue;
      if (rp->isRoomFlag(ROOM_HAVE_TO_WALK))
        continue;
      if (rp->isRoomFlag(ROOM_DEATH))
        continue;
      if (rp->isFlyingSector())
        continue;
    }

    break;
  }

  if (!silent) {
    act("$n shimmers out of existence.", false, this, nullptr, nullptr, TO_ROOM);
    act("You shimmer out of existence.", false, this, nullptr, nullptr, TO_CHAR);
  }

  while ((t = rider)) {
    rc = t->fallOffMount(this, POSITION_STANDING);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t;
      t = nullptr;
    }
  }

  if (riding) {
    rc = fallOffMount(riding, POSITION_STANDING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  --(*this);
  *rp += *this;

  if (!silent) {
    act("$n shimmers into existence.", false, this, nullptr, nullptr, TO_ROOM);
    act("You shimmer into existence.", false, this, nullptr, nullptr, TO_CHAR);

    TBeing* tbt = dynamic_cast<TBeing*>(this);
    if (tbt) {
      tbt->doLook("", CMD_LOOK);

      rc = tbt->genericMovedIntoRoom(rp, -1);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    }
  }

  return false;
}

void TMonster::elementalFix(TBeing* caster, spellNumT spell, bool flags) {
  int level;
  float hplevel = 1.0;
  float aclevel = 1.0;
  float damlevel = 1.0;

  level = caster->GetMaxLevel();
  level = ::number(level - 3, level);
  level = max(level, 4);

  switch (spell) {
    case SPELL_CONJURE_AIR:
    case SPELL_ENTHRALL_SPECTRE:
    case SPELL_CONJURE_FIRE:
    case SPELL_ENTHRALL_GHAST:
      level = (int)(0.8 * level);
      break;
    case SPELL_ENTHRALL_GHOUL:
    case SPELL_ENTHRALL_DEMON:
    case SPELL_CREATE_WOOD_GOLEM:
    case SPELL_CONJURE_WATER:
      level = (int)(0.9 * level);
      break;
    case SPELL_CREATE_ROCK_GOLEM:
    case SPELL_CREATE_IRON_GOLEM:
    case SPELL_CONJURE_EARTH:
    case SPELL_CREATE_DIAMOND_GOLEM:
      level = (int)(1.0 * level);
      break;
    case SPELL_RESURRECTION:
      level =
        caster->GetMaxLevel() >= 50
          ? (int)min((int)GetMaxLevel(), max(level, (int)GetMaxLevel() - 50))
          : (int)min((int)GetMaxLevel(), level);
      hplevel = (getHPLevel() / (int)GetMaxLevel());
      aclevel = (getACLevel() / (int)GetMaxLevel());
      damlevel = (getDamLevel() / (int)GetMaxLevel());
      break;
    default:
      vlogf(LOG_BUG, format("Bad spellNumT (%d) to elementalFix") % spell);
      break;
  }
  // correct the level
  for (classIndT ij = MIN_CLASS_IND; ij < MAX_CLASSES; ij++)
    if (getLevel(ij))
      setLevel(ij, level);

  calcMaxLevel();

  // modify everything by level
  hplevel = level * hplevel;
  aclevel = level * aclevel;
  damlevel = level * damlevel;

  setHPLevel(hplevel);
  setHPFromHPLevel();
  setACLevel(aclevel);
  setACFromACLevel();
  setDamLevel(damlevel);

  setHitroll(0);

  // Set some basic values across the board
  genericCharmFix();

  // Customize each thrall for a bit more flavor
  customizeThrall(spell);
}

void TMonster::customizeThrall(spellNumT spell) {
  switch (spell) {
    case SPELL_CONJURE_AIR:
      setImmunity(IMMUNE_AIR, 75);
      setImmunity(IMMUNE_EARTH, -75);
      break;
    case SPELL_CONJURE_FIRE:
      setImmunity(IMMUNE_HEAT, 75);
      setImmunity(IMMUNE_WATER, -75);
      setImmunity(IMMUNE_COLD, -35);
      break;
    case SPELL_ENTHRALL_SPECTRE:
      setImmunity(IMMUNE_BLUNT, -15);
      setImmunity(IMMUNE_SLASH, 10);
      setImmunity(IMMUNE_PIERCE, 10);
      break;
    case SPELL_ENTHRALL_GHAST:
      setImmunity(IMMUNE_BLUNT, -25);
      setImmunity(IMMUNE_SLASH, 20);
      setImmunity(IMMUNE_PIERCE, 20);
      break;
    case SPELL_ENTHRALL_GHOUL:
      setImmunity(IMMUNE_BLUNT, -35);
      setImmunity(IMMUNE_SLASH, 25);
      setImmunity(IMMUNE_PIERCE, 25);
      break;
    case SPELL_ENTHRALL_DEMON:
      setImmunity(IMMUNE_PARALYSIS, 50);
      break;
    case SPELL_CREATE_WOOD_GOLEM:
      setImmunity(IMMUNE_ACID, 35);
      setImmunity(IMMUNE_AIR, 35);
      setImmunity(IMMUNE_ELECTRICITY, 35);
      setImmunity(IMMUNE_DRAIN, 35);
      setImmunity(IMMUNE_WATER, 35);
      setImmunity(IMMUNE_HEAT, -75);
      break;
    case SPELL_CONJURE_WATER:
      setImmunity(IMMUNE_WATER, 75);
      setImmunity(IMMUNE_HEAT, -75);
      break;
    case SPELL_CREATE_ROCK_GOLEM:
      setImmunity(IMMUNE_BLUNT, -75);
      setImmunity(IMMUNE_SLASH, 35);
      setImmunity(IMMUNE_PIERCE, 35);
      break;
    case SPELL_CREATE_IRON_GOLEM:
      setImmunity(IMMUNE_SLASH, 25);
      break;
    case SPELL_CONJURE_EARTH:
      setImmunity(IMMUNE_EARTH, 75);
      setImmunity(IMMUNE_AIR, -75);
      setImmunity(IMMUNE_BLUNT, -40);
      setImmunity(IMMUNE_SLASH, 15);
      setImmunity(IMMUNE_PIERCE, 15);
      break;
    case SPELL_CREATE_DIAMOND_GOLEM:
      setImmunity(IMMUNE_PIERCE, 20);
      setImmunity(IMMUNE_SLASH, 20);
      break;
    case SPELL_RESURRECTION:
      break;
    default:
      vlogf(LOG_BUG, format("Bad spellNumT (%d) to customizeThrall") % spell);
      break;
  }
}

void TMonster::genericPetFix() {
  // for pets
}

// fix some values on charms/zombies, etc
void TMonster::genericCharmFix() {
  setMoney(0);
  setExp(0);

  spec = 0;

  SET_BIT(specials.act, ACT_SENTINEL);
  REMOVE_BIT(specials.act, ACT_IMMORTAL);
  REMOVE_BIT(specials.act, ACT_AGGRESSIVE);
  REMOVE_BIT(specials.act, ACT_PROTECTOR);
  REMOVE_BIT(specials.act, ACT_PROTECTEE);

  setMalice(0);
  setDefMalice(0);
  setAnger(0);
  setDefAnger(0);
  setGreed(0);
  setDefGreed(0);
  setSusp(0);
  setDefSusp(0);

  setImmunity(IMMUNE_SLASH, 0);
  setImmunity(IMMUNE_BLUNT, 0);
  setImmunity(IMMUNE_PIERCE, 0);
  setImmunity(IMMUNE_NONMAGIC, 0);

  while (hates.clist)
    remHated(nullptr, hates.clist->name);
  while (fears.clist)
    remFeared(nullptr, fears.clist->name);

  // adjusts the mob down to be more like a pc
  // its ok cause has no exp

  balanceMakeNPCLikePC();
}

void TBeing::rawBlind(int level, int duration, saveTypeT save) {
  affectedData aff;
  int bf_mod = 0, knows_bf;

  aff.type = SPELL_BLINDNESS;
  aff.bitvector = AFF_BLIND;
  aff.level = level;
  aff.duration = duration;

  if (save)
    aff.duration /= 2;

  if ((knows_bf = doesKnowSkill(SKILL_BLINDFIGHTING)))
    bf_mod = (getSkillValue(SKILL_BLINDFIGHTING) + 24) / 25;

  aff.location = APPLY_SPELL_HITROLL;
  aff.modifier = (save ? -40 : -80);  // Make hitroll worse
  if (knows_bf)                       // reduce modifier for blindfighting skill
    aff.modifier /= bf_mod;
  affectJoin(nullptr, &aff, AVG_DUR_NO, AVG_EFF_YES);

  aff.location = APPLY_ARMOR;
  aff.modifier = (save ? 20 : 40);  // Make AC Worse!
  if (knows_bf)                     // reduce modifier for blindfighting skill
    aff.modifier /= bf_mod;
  affectJoin(nullptr, &aff, AVG_DUR_NO, AVG_EFF_YES);

  aff.location = APPLY_DEX;
  aff.modifier = (save ? -10 : -30);  // Make Dex Worse!
  if (knows_bf)                       // reduce modifier for blindfighting skill
    aff.modifier /= bf_mod;
  affectJoin(nullptr, &aff, AVG_DUR_NO, AVG_EFF_YES);
}

int TBeing::rawSleep(int level, int duration, int crit, saveTypeT save) {
  int rc = false;

  affectedData aff;

  aff.type = SPELL_SLUMBER;
  aff.duration = duration;
  aff.level = level;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_SLEEP;

  aff.duration *= crit;
  aff.duration /= (save ? 2 : 1);

  // we've already applied a raw immunity check to prevent entirely
  // however, let immunities also decrease duration
  aff.duration *= (100 - getImmunity(IMMUNE_SLEEP));
  aff.duration /= 100;

  affectTo(&aff);

  if (getPosition() > POSITION_SLEEPING) {
    act("You feel very sleepy...  All you want is a bed...   ZZZZZZ....", false,
      this, nullptr, nullptr, TO_CHAR);
    act("You drift peacefully off to dreamland.", false, this, nullptr, nullptr,
      TO_CHAR);
    act("$n falls asleep!", true, this, nullptr, nullptr, TO_ROOM);
  }
  if (riding) {
    rc = fallOffMount(riding, POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  if (fight()) {
    stopFighting();
  }
  setPosition(POSITION_SLEEPING);

  // stop all fighting me too
  TThing* t = nullptr;
  for (StuffIter it = roomp->stuff.begin();
       it != roomp->stuff.end() && (t = *it); ++it) {
    TBeing* ch = dynamic_cast<TBeing*>(t);
    if (!ch)
      continue;
    if (ch->fight() == this)
      ch->stopFighting();
  }

  return false;
}

// take a given limb and determine how much blood to drop, then call dropPool
// dropping these values a bunch... 20 ounces of blood?!
int TBeing::dropBloodLimb(wearSlotT limb) {
  int amt;

  switch (limb) {
    case WEAR_HEAD:
      amt = 3;
      break;
    case WEAR_NECK:
      amt = 4;
      break;
    case WEAR_BODY:
    case WEAR_BACK:
      amt = 4;
      break;
    case WEAR_ARM_R:
    case WEAR_ARM_L:
      amt = 3;
      break;
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
      amt = 1;
      break;
    case WEAR_HAND_R:
    case WEAR_HAND_L:
      amt = 1;
      break;
    case WEAR_FINGER_R:
    case WEAR_FINGER_L:
      amt = 1;
      break;
    case WEAR_WAIST:
      amt = 4;
      break;
    case WEAR_LEG_R:
    case WEAR_LEG_L:
      amt = 3;
      break;
    case WEAR_FOOT_R:
    case WEAR_FOOT_L:
      amt = 1;
      break;
    case WEAR_EX_LEG_R:
    case WEAR_EX_LEG_L:
      amt = 3;
      break;
    case WEAR_EX_FOOT_R:
    case WEAR_EX_FOOT_L:
      amt = 1;
      break;
    default:
      return false;
  }

  dropPool(amt, LIQ_BLOOD);

  if (desc)
    desc->career.ounces_of_blood += amt;

  return true;
}

static int rawDisease(
  TBeing* being, const wearSlotT pos, const int duration,
  const silentTypeT silent, checkImmunityT immcheck, uint16_t limbFlag,
  immuneTypeT immunityType, diseaseTypeT diseaseType, long modifier2 = 0,
  const sstring& toChar = "", const sstring& toRoom = "",
  const char* color = ANSI_NORMAL,
  const std::function<int(int)>& specialDuration =
    [](int duration) { return duration; },
  const std::function<void()>& followupAction = []() { return; }) {
  if (pos < MIN_WEAR || pos >= MAX_WEAR) {
    vlogf(LOG_BUG,
      format("rawDisease called for disease type %i with bad slot: %i on %s") %
        diseaseType % pos % being->getName());
    return false;
  }

  if (!being->hasPart(pos) || being->isLimbFlags(pos, limbFlag) ||
      (immcheck && being->isImmune(immunityType, pos))) {
    return false;
  }

  affectedData aff;
  aff.type = AFFECT_DISEASE;
  aff.level = pos;
  aff.duration = duration;
  aff.location = APPLY_NONE;
  aff.modifier = diseaseType;
  aff.modifier2 = modifier2;
  aff.bitvector = 0;

  if (immcheck && aff.duration != PERMANENT_DURATION) {
    aff.duration = static_cast<int>(
      aff.duration * (100 - being->getImmunity(immunityType)) / 100.0);
  }

  aff.duration = specialDuration(aff.duration);

  being->affectTo(&aff);
  disease_start(being, &aff);

  followupAction();

  if (silent)
    return true;

  if (!toChar.empty())
    act(toChar, true, being, nullptr, nullptr, TO_CHAR, color);
  if (!toRoom.empty())
    act(toRoom, true, being, nullptr, nullptr, TO_ROOM, color);

  return true;
}

int TBeing::rawBruise(wearSlotT pos, int duration, silentTypeT silent,
  checkImmunityT immcheck) {
  const long modifier2 = 1;

  const sstring toChar =
    format("%sYour %s throbs painfully and begins to bruise.%s\n\r") %
    purple() % describeBodySlot(pos) % norm();
  const sstring toRoom = format("%sA bruise begins forming on $n's %s.%s") %
                         purple() % describeBodySlot(pos) % norm();

  const auto updateDuration = [this, pos](int dur) {
    if (hasQuestBit(TOG_IS_HEMOPHILIAC) || hasDisease(DISEASE_SCURVY) ||
        isLimbFlags(pos, PART_LEPROSED | PART_GANGRENOUS))
      dur *= 2;
    return dur;
  };

  return rawDisease(this, pos, duration, silent, immcheck, PART_BRUISED,
    IMMUNE_BLEED, DISEASE_BRUISED, modifier2, toChar, toRoom, ANSI_PURPLE,
    updateDuration);
}

int TBeing::rawBleed(wearSlotT pos, int duration, silentTypeT silent,
  checkImmunityT immcheck) {
  const long modifier2 = 1;

  const sstring toChar =
    format("A wound opens on your %s and begins to bleed.") %
    describeBodySlot(pos);

  const sstring toRoom =
    format("A wound opens on $n's %s and begins to bleed.") %
    describeBodySlot(pos);

  const auto updateDuration = [this](int dur) {
    return hasQuestBit(TOG_IS_HEMOPHILIAC)
             ? PERMANENT_DURATION
             : static_cast<int>(
                 dur * (100 - getImmunity(IMMUNE_BLEED)) / 100.0);
  };

  const auto followupAction = [this, pos]() { dropBloodLimb(pos); };

  return rawDisease(this, pos, duration, silent, immcheck, PART_BLEEDING,
    IMMUNE_BLEED, DISEASE_BLEEDING, modifier2, toChar, toRoom, ANSI_RED,
    updateDuration, followupAction);
}

int TBeing::rawInfect(wearSlotT pos, int duration, silentTypeT silent,
  checkImmunityT immcheck, int level) {
  const long modifier2 = level ? level : GetMaxLevel();

  const sstring toChar =
    format("Your %s has become totally infected!") % describeBodySlot(pos);

  const sstring toRoom =
    format("$n's %s has become totally infected!") % describeBodySlot(pos);

  return rawDisease(this, pos, duration, silent, immcheck, PART_INFECTED,
    IMMUNE_DISEASE, DISEASE_INFECTION, modifier2, toChar, toRoom);
}

int TBeing::rawGangrene(wearSlotT pos, int duration, silentTypeT silent,
  checkImmunityT immcheck, int level) {
  const long modifier2 = level ? level : GetMaxLevel();

  // Start messages handled in disease_gangrene
  return rawDisease(this, pos, duration, silent, immcheck, PART_GANGRENOUS,
    IMMUNE_DISEASE, DISEASE_GANGRENE, modifier2);
}

void TBeing::spellMessUp(spellNumT spell) {
  int num;
  int type = 0;

  if (!discArray[spell] || !*discArray[spell]->name) {
    vlogf(LOG_BUG, format("Bad spell/skill number in spellMessUp %d") % spell);
    return;
  }

  if ((discArray[spell]->typ == SPELL_MAGE) ||
      (discArray[spell]->typ == SPELL_RANGER) ||
      (discArray[spell]->typ == SPELL_SHAMAN)) {
    type = 0;
  } else if ((discArray[spell]->typ == SPELL_CLERIC) ||
             (discArray[spell]->typ == SPELL_DEIKHAN)) {
    type = 1;
  }
#if 0
  switch (getDisciplineNumber(spell, false)) {
    case DISC_AIR:
    case DISC_WATER:
    case DISC_FIRE:
    case DISC_EARTH:
    case DISC_SPIRIT:
    case DISC_ALCHEMY:
    case DISC_MAGE:
    case DISC_RANGER:
    case DISC_SHAMAN:
      type = 0;
    case DISC_CURES:
    case DISC_AFFLICTIONS:
    case DISC_WRATH:
    case DISC_HAND_OF_GOD:
    case DISC_AEGIS:
    case DISC_CLERIC:
    case DISC_DEIKHAN:
      type = 1;
    default:
      vlogf(LOG_BUG, format("Undefined spell (%d) in spellMessUp") %  spell);
      return;
  }
#endif
  num = ::number(1, 6);

  switch (num) {
    case 1:
    case 2:
      if (type == 1)
        act("Your brain is jumbled and confused, and you flub the prayer.",
          false, this, 0, 0, TO_CHAR);
      else
        act("Your brain is jumbled and confused.", false, this, 0, 0, TO_CHAR);
      act("$n must have done something wrong.", false, this, 0, 0, TO_ROOM);
      break;
    case 3:
      if (getWizardryLevel() < WIZ_LEV_NO_GESTURES &&
          IS_SET(discArray[spell]->comp_types, COMP_GESTURAL)) {
        // requires gestures
        act("Darn it!  You mess up one of the intricate gestures.", false, this,
          0, 0, TO_CHAR);
        act("$n must have done something wrong.", false, this, 0, 0, TO_ROOM);
        break;
      }  // otherwise drop through for different text
      if (getRitualismLevel() < RIT_LEV_NO_GESTURES &&
          IS_SET(discArray[spell]->comp_types, COMP_GESTURAL)) {
        // requires gestures
        act("Darn it!  You mess up one of the intricate gestures.", false, this,
          0, 0, TO_CHAR);
        act("$n must have done something wrong.", false, this, 0, 0, TO_ROOM);
        break;
      }  // otherwise drop through for different text
    case 4:
      if (getWizardryLevel() < WIZ_LEV_NO_GESTURES &&
          IS_SET(discArray[spell]->comp_types, COMP_GESTURAL)) {
        // requires gestures
        if (type == 1)
          act(
            "You clumsily perform the prayer's gestures, and things seem to "
            "have gone wrong.",
            false, this, 0, 0, TO_CHAR);
        else
          act(
            "You clumsily perform the spell's gestures, and things seem to "
            "have gone wrong.",
            false, this, 0, 0, TO_CHAR);
        act("$n must have done something wrong.", false, this, 0, 0, TO_ROOM);
        break;
      }  // otherwise drop through for different text
      if (getRitualismLevel() < RIT_LEV_NO_GESTURES &&
          IS_SET(discArray[spell]->comp_types, COMP_GESTURAL)) {
        act("You pathetic excuse for a houngan....Grrrrrr!!!", false, this, 0,
          0, TO_CHAR);
        act("$n must have done something wrong.", false, this, 0, 0, TO_ROOM);
        break;
      }  // otherwise drop through for different text
    case 5:
      if (getWizardryLevel() < WIZ_LEV_NO_MANTRA &&
          IS_SET(discArray[spell]->comp_types, COMP_VERBAL)) {
        // requires incantation
        act("Oops...  You mis-spoke part of the incantation.", false, this, 0,
          0, TO_CHAR);
        act("$n must have done something wrong.", false, this, 0, 0, TO_ROOM);
        break;
      }  // otherwise drop through for different text
      if (getRitualismLevel() < RIT_LEV_NO_MANTRA &&
          IS_SET(discArray[spell]->comp_types, COMP_VERBAL)) {
        // requires incantation
        act("Oops...  You messed that one up....", false, this, 0, 0, TO_CHAR);
        act("$n must have done something wrong.", false, this, 0, 0, TO_ROOM);
        break;
      }  // otherwise drop through for different text
    case 6:
      if (getWizardryLevel() < WIZ_LEV_NO_MANTRA &&
          IS_SET(discArray[spell]->comp_types, COMP_VERBAL)) {
        // requires incantation
        act("You trip over your tongue and mis-speak the incantation.", false,
          this, 0, 0, TO_CHAR);
        act("$n must have done something wrong.", false, this, 0, 0, TO_ROOM);
        break;
      }  // otherwise drop through for different text
      if (getRitualismLevel() < RIT_LEV_NO_MANTRA &&
          IS_SET(discArray[spell]->comp_types, COMP_VERBAL)) {
        // requires incantation
        act("You trip over your own feet trying to dance for the loa!", false,
          this, 0, 0, TO_CHAR);
        act("$n must have done something wrong.", false, this, 0, 0, TO_ROOM);
        break;
      }  // otherwise drop through for different text
    default:
      act("You aren't sure what, but something seems to have gone wrong.",
        false, this, 0, 0, TO_CHAR);
      act("$n must have done something wrong.", false, this, 0, 0, TO_ROOM);
  }
}

void TBeing::nothingHappens(silentTypeT silent_caster) const {
  soundNumT snd = pickRandSound(SOUND_CAST_FAIL_01, SOUND_CAST_FAIL_02);

  if (!silent_caster)
    roomp->playsound(snd, SOUND_TYPE_MAGIC);
  else {
    TThing* t = nullptr;
    for (StuffIter it = roomp->stuff.begin();
         it != roomp->stuff.end() && (t = *it); ++it) {
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (!tbt || tbt == this)
        continue;
      tbt->playsound(snd, SOUND_TYPE_MAGIC);
    }
  }

  if (hasClass(CLASS_SHAMAN)) {
    int num = ::number(0, 6);
    switch (num) {
      default:
      case 0:
        if (!silent_caster)
          sendTo("Nothing seems to happen.\n\r");
        act("Nothing seems to happen.", true, this, 0, 0, TO_ROOM);
        break;
      case 1:
        if (!silent_caster)
          sendTo("Nothing happens.\n\r");
        act("Nothing happens.", true, this, 0, 0, TO_ROOM);
        break;
      case 2:
        if (!silent_caster)
          act("Uh oh, maybe you ought to try that again.", false, this, nullptr,
            nullptr, TO_CHAR);
        act("Make like it worked.....shhhhhhhhhhh.", false, this, nullptr, nullptr,
          TO_ROOM);
        break;
      case 3:
        if (!silent_caster)
          act("That didn't work...", false, this, nullptr, nullptr, TO_CHAR);
        act("$n's invokation didn't work.", false, this, nullptr, nullptr, TO_ROOM);
        break;
      case 4:
        if (!silent_caster)
          act("Nope, nuh uh, nada, zip.", false, this, nullptr, nullptr, TO_CHAR);
        act("Chant, dance, do the bugaloo...whatever, shaman suck.", false,
          this, nullptr, nullptr, TO_ROOM);
        break;
      case 5:
        if (!silent_caster)
          act("Damn!  Missed again.", false, this, nullptr, nullptr, TO_CHAR);
        act("No luck here! Maybe something more simple for a shaman?", false,
          this, nullptr, nullptr, TO_ROOM);
        break;
      case 6:
        if (!silent_caster)
          act("The power of your ancestors is not there.", false, this, nullptr,
            nullptr, TO_CHAR);
        act("I feel like dancin'....YEAH!", false, this, nullptr, nullptr, TO_ROOM);
        break;
    }
  } else {
    int num = ::number(0, 17);
    switch (num) {
      default:
      case 0:
        if (!silent_caster)
          sendTo("Nothing seems to happen.\n\r");
        act("Nothing seems to happen.", true, this, 0, 0, TO_ROOM);
        break;
      case 1:
        if (!silent_caster)
          sendTo("Nothing happens.\n\r");
        act("Nothing happens.", true, this, 0, 0, TO_ROOM);
        break;
      case 2:
        if (!silent_caster)
          act("Uh oh, maybe you ought to try that again.", false, this, nullptr,
            nullptr, TO_CHAR);
        act("Humor the little mage and pretend the spell worked.", false, this,
          nullptr, nullptr, TO_ROOM);
        break;
      case 3:
        if (!silent_caster)
          act("That didn't work...ONE MORE TIME!", false, this, nullptr, nullptr,
            TO_CHAR);
        act("$n's spell didn't work.", false, this, nullptr, nullptr, TO_ROOM);
        break;
      case 4:
        if (!silent_caster)
          act("Nope, nuh uh, nada, zip, the big mage fizzle.", false, this,
            nullptr, nullptr, TO_CHAR);
        act("Chant, chant, wave hands, wave hands, mages suck.", false, this,
          nullptr, nullptr, TO_ROOM);
        break;
      case 5:
        if (!silent_caster)
          act("Damn!  Missed again.", false, this, nullptr, nullptr, TO_CHAR);
        act("The mage casts and misses!", false, this, nullptr, nullptr, TO_ROOM);
        break;
      case 6:
        if (!silent_caster)
          act("The forces of magic fail to come forth.", false, this, nullptr,
            nullptr, TO_CHAR);
        act("The forces of magic fail to come forth.", false, this, nullptr, nullptr,
          TO_ROOM);
        break;
      case 7:
        if (!silent_caster)
          act("Try as you might, your magic fails you.", false, this, nullptr,
            nullptr, TO_CHAR);
        act("Try as $n might, the magic fails.", false, this, nullptr, nullptr,
          TO_ROOM);
        break;
      case 8:
        if (!silent_caster)
          act("Your attempt at magic is unsuccessful.", false, this, nullptr, nullptr,
            TO_CHAR);
        act("$n's attempt at magic is unsuccessful.", false, this, nullptr, nullptr,
          TO_ROOM);
        break;
      case 9:
        if (!silent_caster)
          act("Your spell dissipates without effect.", false, this, nullptr, nullptr,
            TO_CHAR);
        act("$n's magic dissipates without any effect.", false, this, nullptr,
          nullptr, TO_ROOM);
        break;
      case 10:
        if (!silent_caster)
          act("Your mind lacks the focus to control the magic.", false, this,
            nullptr, nullptr, TO_CHAR);
        act("$n's magic starts to form, but then collapses.", false, this, nullptr,
          nullptr, TO_ROOM);
        break;
      case 11:
        if (!silent_caster)
          act("Your thoughts go awry, and the magic fades harmlessly.", false,
            this, nullptr, nullptr, TO_CHAR);
        act("$n looks perplexed and $s magic fades harmlessly.", false, this,
          nullptr, nullptr, TO_ROOM);
        break;
      case 12:
        if (!silent_caster)
          act("You're pretty sure that should have worked, but no such luck.",
            false, this, nullptr, nullptr, TO_CHAR);
        act(
          "$n blinks in bewilderment.  Perhaps $e was expecting something to "
          "happen...?",
          false, this, nullptr, nullptr, TO_ROOM);
        break;
      case 13:
        if (!silent_caster)
          act("Dang, you forgot part of the incantation and cease casting.",
            false, this, nullptr, nullptr, TO_CHAR);
        act("$n throws $s hands up in disgust.", false, this, nullptr, nullptr,
          TO_ROOM);
        break;
      case 14:
        if (!silent_caster)
          act("Something seems amiss, and you give up on your spell.", false,
            this, nullptr, nullptr, TO_CHAR);
        act("$n acts like $s spell is finished, but the magic ain't there.",
          false, this, nullptr, nullptr, TO_ROOM);
        break;
      case 15:
        if (!silent_caster)
          act("You slip up and manage to fill the air with goose feathers.",
            false, this, nullptr, nullptr, TO_CHAR);
        act("$n fills the air with goose feathers.  Neat!", false, this, nullptr,
          nullptr, TO_ROOM);
        break;
      case 16:
        if (!silent_caster)
          act("You make an error and sparks seem to surround you.", false, this,
            nullptr, nullptr, TO_CHAR);
        act("$n makes a mistake, and becomes surrounded by magical sparks.",
          false, this, nullptr, nullptr, TO_ROOM);
        break;
      case 17:
        if (!silent_caster)
          act("DAMN! Screwed up again!", false, this, nullptr, nullptr, TO_CHAR);
        act("Chant...Chant...Wave hands...Wave hands...Mages suck!", false,
          this, nullptr, nullptr, TO_ROOM);
        break;
    }
  }
}

bool TBeing::canDoSummon() const {
  if (roomp->isFlyingSector())
    return false;

  if ((inRoom() >= 9710 && inRoom() <= 9795))
    return false;
  else
    return true;
}

bool TBeing::isSummonable() const {
  if (roomp->isFlyingSector())
    return false;

  if ((inRoom() >= 9710 && inRoom() <= 9795))
    return false;
  else
    return true;
}

bool genericBless(TBeing* c, TBeing* v, int level, bool crit) {
  affectedData aff1, aff2;

  aff1.type = SPELL_BLESS;
  aff1.level = level;
  aff1.duration = (1 + level) * Pulse::UPDATES_PER_MUDHOUR;
  aff1.location = APPLY_SPELL_HITROLL;
  aff1.modifier = 10;
  aff1.bitvector = 0;

  aff2.type = aff1.type;
  aff2.level = level;
  aff2.duration = (1 + level) * Pulse::UPDATES_PER_MUDHOUR;
  aff2.location = APPLY_IMMUNITY;
  aff2.modifier = IMMUNE_NONMAGIC;
  aff2.modifier2 = 5;
  aff2.bitvector = 0;

  if (crit) {
    aff1.duration += 9 * Pulse::UPDATES_PER_MUDHOUR;
    aff1.modifier *= 2;
    aff2.duration += 9 * Pulse::UPDATES_PER_MUDHOUR;
    aff2.modifier2 *= 2;
  }

  bool success = false;
  if (v->affectJoin(c, &aff1, AVG_DUR_NO, AVG_EFF_YES))
    success = true;

  if (success) {
    if (v->affectJoin(c, &aff2, AVG_DUR_NO, AVG_EFF_YES))
      success = true;
  } else {
    if (v->affectJoin(c, &aff2, AVG_DUR_NO, AVG_EFF_YES, false))
      success = true;
  }
  return success;
}

bool genericDisease(TBeing* caster, TBeing* vict, int level) {
  // assumes check for isImmune already made
  affectedData aff;
  aff.type = AFFECT_DISEASE;
  aff.level = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  aff.duration = level * Pulse::UPDATES_PER_MUDHOUR / 3;
  aff.modifier2 = level;

  std::vector<diseaseTypeT> diseases;  // possible disease types
  // starting with most potent... order matters
  if (!vict->hasDisease(DISEASE_GANGRENE) && level >= 50)
    diseases.push_back(DISEASE_GANGRENE);
  if (!vict->hasDisease(DISEASE_LEPROSY) && level >= 40)
    diseases.push_back(DISEASE_LEPROSY);
  if (!vict->hasDisease(DISEASE_PNEUMONIA) && level >= 30)
    diseases.push_back(DISEASE_PNEUMONIA);
  if (!vict->hasDisease(DISEASE_FLU) && level >= 20)
    diseases.push_back(DISEASE_FLU);
  if (!vict->hasDisease(DISEASE_DYSENTERY) && level >= 10)
    diseases.push_back(DISEASE_DYSENTERY);
  if (!vict->hasDisease(DISEASE_COLD))
    diseases.push_back(DISEASE_COLD);
  if (!diseases.size())
    return false;

  // reverse the probability loop depending on level diff
  double level_diff = (double)(vict->GetMaxLevel() - level);
  if (level_diff < 0) {
    reverse(diseases.begin(), diseases.end());
    level_diff = std::abs(level_diff);
  }
  // this scales depending on the number of diseases available
  // higher level diseases will be  harder to land (on higher level targets)
  // depending on the what the slope is set to conversely, against lower level
  // targets, it will be easier to land a more potent disease
  unsigned int step;
  int roll_size = 0;
  double scaler =
    max(1.0, (double)diseases.size() -
               1.0);   // scales according to number of diseases available
  double slope = 6.0;  // change to a higher number to even out the chances to
                       // land any particular disease
  for (step = 0; step < diseases.size(); step++) {
    roll_size += max(1, (int)(((level_diff / slope) / scaler) * (double)step));
    // vlogf(LOG_MISC, format("%s- Step: %d Chance: %d") %
    // DiseaseInfo[diseases[step]].name % step % max(1, (int) (((level_diff /
    // slope) / scaler) * (double) step)));
  }
  int total = 0;
  int roll = ::number(1, roll_size);
  for (step = 0; step < diseases.size(); step++) {
    total += max(1, (int)(((level_diff / slope) / scaler) * (double)step));
    if (roll <= total) {
      aff.modifier = diseases[step];
      break;
    }
  }
  // vlogf(LOG_MISC, format("%d of %d for %s.") % roll % roll_size %
  // DiseaseInfo[diseases[step]].name);
  if (aff.modifier == DISEASE_GANGRENE) {
    // find a random slot for it
    wearSlotT slot;
    bool found = false;  // need to make sure this doesn't loop 4ever, right?
    for (int i = 0; i < 20; ++i) {
      slot = pickRandomLimb();
      if (notBleedSlot(slot))
        continue;
      if (!vict->hasPart(slot))
        continue;
      if (vict->isLimbFlags(slot, PART_GANGRENOUS))
        continue;
      if (vict->isImmune(IMMUNE_DISEASE, slot))
        continue;
      found = true;
      break;
    }
    if (!found)
      return false;
    aff.level = slot;
  }

  // make leprosy & gangrene permanent
  if (aff.modifier == DISEASE_LEPROSY || aff.modifier == DISEASE_GANGRENE) {
    aff.duration = PERMANENT_DURATION;
  } else {
    // we've already applied a raw immunity check to prevent entirely
    // however, let immunities also decrease duration
    aff.duration *= (100 - vict->getImmunity(IMMUNE_DISEASE));
    aff.duration /= 100;
  }
  if (caster) {
    act("$d breathes a fetid cloud into $N's body.", false, caster, 0, vict,
      TO_CHAR);
    act("$d breathes a fetid cloud into your body.", false, caster, 0, vict,
      TO_VICT);
    act("$d breathes a fetid cloud into $N's body.", false, caster, 0, vict,
      TO_NOTVICT);
  }
  vict->affectTo(&aff);
  disease_start(vict, &aff);
  return true;
}

void genericCurse(TBeing* c, TBeing* v, int level, spellNumT spell) {
  affectedData aff1, aff2;

  aff1.type = spell;
  aff1.level = level;
  aff1.duration = 12 * Pulse::UPDATES_PER_MUDHOUR;
  aff1.bitvector = AFF_CURSE;
  aff1.location = APPLY_SPELL_HITROLL;
  aff1.modifier = -min(5, level / 3);
  aff1.duration = (int)(c->percModifier() * aff1.duration);

  aff2.type = aff1.type;
  aff2.level = aff1.level;
  aff2.duration = aff1.duration;
  aff2.bitvector = aff1.bitvector;

  aff2.location = APPLY_IMMUNITY;
  aff2.modifier = IMMUNE_PARALYSIS; /* Make worse */
  aff2.modifier2 = -min(5, level / 3);

  v->affectTo(&aff1);
  v->affectTo(&aff2);
}

sstring displayDifficulty(spellNumT skill) {
  switch (discArray[skill]->task) {
    case TASK_TRIVIAL:
      return "Trivial";
    case TASK_EASY:
      return "Easy";
    case TASK_NORMAL:
      return "Normal";
    case TASK_DIFFICULT:
      return "Difficult";
    case TASK_DANGEROUS:
      return "Dangerous";
    case TASK_HOPELESS:
      return "Hopeless";
    case TASK_IMPOSSIBLE:
      return "Near-impossible";
  }
  return "BOGUS, tell a god";
}

int lycanthropeTransform(TBeing* ch) {
  TMonster* mob;

  if (!ch->isPc() || IS_SET(ch->specials.act, ACT_POLYSELF) ||
      ch->polyed != POLY_TYPE_NONE) {
    act("You are already transformed into another shape.", true, ch, nullptr, nullptr,
      TO_CHAR);
    return false;
  }

  if (!(mob = read_mobile(23204, VIRTUAL))) {
    return false;
  }
  thing_to_room(mob, Room::VOID);
  mob->swapToStrung();

  act("The presence of the full moon forces you into transformation!", true, ch,
    nullptr, mob, TO_CHAR);
  act("The presence of the full moon forces $n to transform into $N!", true, ch,
    nullptr, mob, TO_ROOM);

  DisguiseStuff(ch, mob);

  --(*mob);
  *ch->roomp += *mob;
  --(*ch);
  thing_to_room(ch, Room::POLY_STORAGE);

  // stop following whoever you are following.
  if (ch->master)
    ch->stopFollower(true);

  mob->setQuestBit(TOG_TRANSFORMED_LYCANTHROPE);

  // switch ch into mobile
  ch->desc->character = mob;
  ch->desc->original = dynamic_cast<TPerson*>(ch);

  mob->desc = ch->desc;
  ch->desc = nullptr;
  ch->polyed = POLY_TYPE_DISGUISE;

  SET_BIT(mob->specials.act, ACT_DISGUISED);
  SET_BIT(mob->specials.act, ACT_POLYSELF);
  SET_BIT(mob->specials.act, ACT_NICE_THIEF);
  SET_BIT(mob->specials.act, ACT_SENTINEL);
  SET_BIT(mob->specials.act, ACT_AGGRESSIVE);
  REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
  REMOVE_BIT(mob->specials.act, ACT_DIURNAL);
  REMOVE_BIT(mob->specials.act, ACT_NOCTURNAL);

  appendPlayerName(ch, mob);

  mob->setHeight(ch->getHeight());
  mob->setWeight(ch->getWeight());

  mob->doAction("", CMD_HOWL);
  mob->roomp->getZone()->sendTo(
    "You hear a chilling wolf howl in the distance.\n\r", mob->in_room);

  return true;
}
