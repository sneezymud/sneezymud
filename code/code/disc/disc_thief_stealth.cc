#include "defs.h"
#include "extern.h"
#include "handler.h"
#include "being.h"
#include "low.h"
#include "monster.h"
#include "games.h"
#include "disease.h"
#include "combat.h"
#include "person.h"
#include "disc_thief_stealth.h"
#include "obj_tool.h"
#include "room.h"
#include "pathfinder.h"
#include "obj_portal.h"
#include "client.h"
#include "spec_mobs.h"

void TBeing::doTrack(const char* argument) {
  char namebuf[256], found = FALSE;
  int dist = 0, targrm, worked, skill;
  int code;
  TBeing* scan;
  affectedData aff, *Vaff;
  TThing* t = NULL;
  char buf[256] = "\0\0\0", buf2[512] = "\0\0\0";

  strcpy(namebuf, argument);

  if (!*namebuf && !specials.hunting) {
    sendTo("You need to search for SOMEONE.\n\r");
    return;
  }
  if (affectedBySpell(SKILL_SEEKWATER)) {
    sendTo("You can't track something while seeking water.\n\r");
    return;
  }
  int learning = getSkillValue(SKILL_TRACK);
  if (learning <= 0 && !affectedBySpell(SPELL_TRAIL_SEEK)) {
    sendTo("You do not have the ability to track.\n\r");
    return;
  }
  if (getPosition() < POSITION_STANDING) {
    sendTo("You really need to be standing in order to start tracking.\n\r");
    return;
  }
  if (fight()) {
    sendTo("The ensuing battle makes it difficult to track.\n\r");
    return;
  }
  if (roomp && !isImmortal() &&

      (roomp->getLight() + visionBonus <= 0) &&
      !roomp->isRoomFlag(ROOM_ALWAYS_LIT) && !isAffected(AFF_TRUE_SIGHT) &&
      !isAffected(AFF_CLARITY)) {
    sendTo("You can't see well enough to track.\n\r");
    return;
  }
  found = FALSE;
  for (scan = character_list; scan; scan = scan->next) {
    if (isname(namebuf, scan->name)) {
      found = TRUE;
      break;
    }
  }
  if (!found) {
    sendTo("You are unable to find any signs of that.\n\r");
    return;
  }

  // Lets determine the distance: [roomCount == 11865 as of 12-18-98]
  //  Ranger  TRAIL_SEEK :  25 - 176
  //  Ranger !TRAIL_SEEK :  20 - 161
  // !Ranger  TRAIL_SEEK :  15 - 153
  int level = 0;
  if (learning > 0) {
    level = getSkillLevel(SKILL_TRACK);

    if (affectedBySpell(SPELL_TRAIL_SEEK))
      dist = max(25,
        (int)(((((roomCount * ((level + 1) / 2)) / 1000) + min(50, learning))) /
              2));
    else
      dist = max(20,
        (int)(((((roomCount * ((level + 1) / 2)) / 1000) + min(20, learning))) /
              2));
  } else if (affectedBySpell(SPELL_TRAIL_SEEK)) {
    learning = getSkillValue(SPELL_TRAIL_SEEK);
    level = getSkillLevel(SPELL_TRAIL_SEEK);
    dist = max(15,
      (int)(((((roomCount * ((level + 1) / 2)) / 1000) + min(5, learning))) /
            2));
  }

  if (hasClass(CLASS_THIEF) || hasClass(CLASS_RANGER))
    dist *= 2;
  else if (hasClass(CLASS_WARRIOR))
    dist /= 2;
  else if (hasClass(CLASS_MAGE))
    dist += getLevel(MAGE_LEVEL_IND);
  // else no change

  switch (getRace()) {
    case RACE_GIANT:
    case RACE_ELVEN:
      dist *= 2;  // even better
      break;
    case RACE_DEVIL:
    case RACE_DEMON:
      dist = MAX_ROOMS;  //  4 as good as can be
      break;
    default:
      break;
  }

  if (isImmortal())
    dist = MAX_ROOMS;

  hunt_dist = dist;
  specials.hunting = 0;
  TPathFinder path(dist);

  // note: -dist will look THRU doors.
  // all subsequent calls use track() which does not go thru doors
  // this is intentional so they lose track after 1 step
  if ((level < MIN_GLOB_TRACK_LEV) || (affectedBySpell(SPELL_TRAIL_SEEK))) {
    path.setStayZone(true);
    path.setThruDoors(true);

    code = path.findPath(in_room, findBeing(namebuf));
    targrm = path.getDest();
  } else {
    path.setStayZone(false);
    path.setThruDoors(true);

    code = path.findPath(in_room, findBeing(namebuf));
    targrm = path.getDest();
  }

  if (code == -1) {
    addToWait(combatRound(1));
    if (targrm == inRoom()) {
      if (get_char_room(namebuf, targrm) == this)
        sendTo("Do you often lose yourself?\n\r");
      else
        sendTo("I believe what your tracking is right here with you.\n\r");
    } else
      sendTo("You are unable to find any signs of that.\n\r");
    return;
  } else {
    addPlayerAction(PLR_HUNTING);
    specials.hunting = get_char_room(namebuf, targrm);
    skill = 0;

    if (hasQuestBit(TOG_IS_CRAVEN))
      skill -= 25;
    if (specials.hunting && specials.hunting->hasQuestBit(TOG_IS_CRAVEN))
      skill -= 25;
    if (hasQuestBit(TOG_IS_VICIOUS))
      skill += 25;
    if (specials.hunting && specials.hunting->hasQuestBit(TOG_IS_VICIOUS))
      skill += 25;

    for (Vaff = specials.hunting->affected; Vaff; Vaff = Vaff->next)
      if (Vaff->type == SKILL_CONCEALMENT)
        skill = -Vaff->modifier;
    if (affectedBySpell(SPELL_TRAIL_SEEK)) {
      skill += 50;
      worked = (::number(0, 110) < skill);
    } else {
      skill += getSkillValue(SKILL_TRACK);
      worked = bSuccess(skill, SKILL_TRACK);
    }
    if (worked) {
      if (code <= 9)
        sendTo(format("%sYou see traces of your quarry %s.%s\n\r") % purple() %
               dirs_to_blank[code] % norm());
      else {
        int count = code - 9, seen = 0;
        for (StuffIter it = roomp->stuff.begin();
             it != roomp->stuff.end() && (t = *it); ++it) {
          TPortal* tp = dynamic_cast<TPortal*>(t);
          if (tp) {
            seen++;
            if (count == seen) {
              sendTo(COLOR_OBJECTS,
                format("%sYou see traces of your quarry through %s.%s\n\r") %
                  purple() % tp->getName() % norm());
              break;
            }
          }
        }
        if (!t) {
          sendTo("Error finding path target!  Tell a god.\n\r");
          vlogf(LOG_BUG, "Error finding path (doTrack)");
          return;
        }
      }
    } else {
      code = -2;
      sendTo(COLOR_MOBS,
        format("You begin tracking %s.\n\r") % specials.hunting->getName());
    }
  }

  aff.type = SKILL_TRACK;
  aff.level = getSkillLevel(SKILL_TRACK);
  aff.duration = PERMANENT_DURATION;
  affectTo(&aff);

  if (desc && desc->m_bIsClient)
    desc->clientf(format("%d|%d") % CLIENT_TRACKING % (1 << code));

  if (code <= 9) {
    if (code >= 0 && desc && (desc->autobits & AUTO_HUNT)) {
      strcpy(buf, dirs[code]);
      addCommandToQue(buf);
    }
  } else if (desc && (desc->autobits & AUTO_HUNT) && t) {
    strcpy(buf, t->name.c_str());
    strcpy(buf, add_bars(buf).c_str());
    addToWait(combatRound(1));
    sprintf(buf2, "enter %s", buf);
    addCommandToQue(buf2);
  }

  bool isTR = affectedBySpell(SPELL_TRAIL_SEEK);

  addToWait(combatRound(1));
  addToMove((int)min(10,
    (-2 -
      ((110 - getSkillValue((isTR ? SKILL_TRACK : SPELL_TRAIL_SEEK)))) / 6)));

  start_task(this, NULL, NULL, TASK_TRACKING, "", 1, in_room, 1, code + 1, 40);
}

// used by doLook() to display next direction to go.
// return FALSE to cease tracking
int TBeing::track(TBeing* vict) {
  int code;
  TThing* t = NULL;
  int targetRm = -1;
  int isSW = affectedBySpell(SKILL_SEEKWATER);
  char buf[256];
  char buf2[512];
  TPathFinder path(hunt_dist);
  path.setUsePortals(false);

  if (!vict && !isSW) {
    vlogf(LOG_BUG, format("Problem in track() %s") % getName());
    return TRUE;
  }
  if (roomp && !isImmortal() && (roomp->getLight() + visionBonus <= 0) &&
      !roomp->isRoomFlag(ROOM_ALWAYS_LIT) && !isAffected(AFF_TRUE_SIGHT) &&
      !isAffected(AFF_CLARITY)) {
    return TRUE;
  }
  if (!vict) {
    if (isSW) {
      code = path.findPath(in_room, findWater());
      targetRm = path.getDest();
    } else {
      vlogf(LOG_BUG, "problem in track()");
      stopTask();
      return FALSE;
    }
  } else {
    if (isImmortal())  // look through doors
      code = choose_exit_global(in_room, vict->in_room, hunt_dist);
    else if ((GetMaxLevel() < MIN_GLOB_TRACK_LEV) ||
             affectedBySpell(SPELL_TRAIL_SEEK))
      code = choose_exit_in_zone(in_room, vict->in_room, hunt_dist);
    else
      code = choose_exit_global(in_room, vict->in_room, hunt_dist);
  }
  if ((vict && sameRoom(*vict)) || (targetRm != -1 && targetRm == inRoom())) {
    sendTo(format("%s###You have found %s!%s\n\r") % orange() %
           (isSW ? "some water" : "your quarry") % norm());
    addToWait(combatRound(1));
    if (desc && desc->m_bIsClient)
      desc->clientf(format("%d") % CLIENT_TRACKOFF);
    stopTask();
    addToWait(combatRound(1));
    return FALSE;
  }
  if (task && task->flags > 0 && task->flags != 100) {
    if (task->flags != (code + 1)) {
      sendTo(format("%s###For some reason the path you found is gone.%s\n\r") %
             purple() % norm());
      task->flags = 0;
      return TRUE;
    } else if (code <= 9) {
      sendTo(format("%s###You track %s %s.%s\n\r") % purple() %
             (isSW ? "some water" : "your target") % dirs_to_blank[code] %
             norm());
      if (desc && (desc->autobits & AUTO_HUNT)) {
        strcpy(buf, dirs[code]);
        addCommandToQue(buf);
      }
    } else {
      int count = code - 9, seen = 0;
      for (StuffIter it = roomp->stuff.begin();
           it != roomp->stuff.end() && (t = *it); ++it) {
        TPortal* tp = dynamic_cast<TPortal*>(t);
        if (tp) {
          seen++;
          if (count == seen) {
            sendTo(COLOR_OBJECTS,
              format("%sYou track %s through %s.%s\n\r") % purple() %
                (isSW ? "some water" : "your quarry") % tp->getName() % norm());
            if (desc && (desc->autobits & AUTO_HUNT)) {
              strcpy(buf, tp->name.c_str());
              strcpy(buf, add_bars(buf).c_str());
              sprintf(buf2, "enter %s", buf);
              addCommandToQue(buf2);
            }
            break;
          }
        }
      }
    }
    return TRUE;
  }
  return TRUE;
}

// this is called exclusively by TMonster::hunt()
// returns 0-9 for dir to travel, or 10+ for a portal (indexed)
// return -1 to stop the tracking
dirTypeT TBeing::dirTrack(TBeing* vict) {
  dirTypeT code;
  affectedData* aff;

  if (roomp && !isImmortal() && (roomp->getLight() + visionBonus <= 0) &&
      !roomp->isRoomFlag(ROOM_ALWAYS_LIT) && !isAffected(AFF_TRUE_SIGHT) &&
      !isAffected(AFF_CLARITY)) {
    sendTo("You can't see well enough to find a trail.\n\r");
    return DIR_NONE;
  }

  for (aff = vict->affected; aff; aff = aff->next) {
    if (aff->type == SKILL_CONCEALMENT) {
      if (::number(1, 150) < aff->modifier) {
        sendTo(
          format("%s##You have lost the trail.%s\n\r") % orange() % norm());

        if (aff->be == vict) {
          act("You have successfully concealed your path from $N.", FALSE, vict,
            0, this, TO_CHAR, ANSI_GREEN);
          return DIR_NONE;
        } else if (aff->be && vict->sameRoom(*aff->be)) {
          act("$N has successfully concealed your path from $P.", FALSE, vict,
            this, aff->be, TO_CHAR, ANSI_GREEN);
          act("You have successfully concealed $n's path from $P.", FALSE, vict,
            this, aff->be, TO_VICT, ANSI_GREEN);
          return DIR_NONE;
        }
      }
    }
  }

  if ((GetMaxLevel() >= MIN_GLOB_TRACK_LEV) ||
      affectedBySpell(SPELL_TRAIL_SEEK) || IS_SET(specials.act, ACT_HUNTING)) {
    code = choose_exit_global(in_room, vict->in_room, hunt_dist);
  } else
    code = choose_exit_in_zone(in_room, vict->in_room, hunt_dist);

  if (code == DIR_NONE) {
    if (sameRoom(*vict))
      sendTo(
        format("%s##You have found your target!%s\n\r") % orange() % norm());
    else
      sendTo(format("%s##You have lost the trail.%s\n\r") % orange() % norm());

    return DIR_NONE;  // false to continue the hunt
  } else if (code < MAX_DIR) {
    sendTo(format("%s##You see a faint trail %s.%s\n\r") % purple() %
           dirs_to_leading[code] % norm());
    return code;
  } else {
    int count = code - 9, seen = 0;
    TPortal* tp = NULL;
    for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end(); ++it) {
      tp = dynamic_cast<TPortal*>(*it);
      if (tp) {
        seen++;
        if (count == seen) {
          sendTo(COLOR_OBJECTS,
            format("%sYou see a faint trail through %s.%s\n\r") % purple() %
              tp->getName() % norm());
          break;
        }
      }
    }
    if (!tp) {
      sendTo("Error finding path target!  Tell a god.\n\r");
      vlogf(LOG_BUG, "Error finding path (dirTrack)");
    }
    return code;
  }
}

void TBeing::doConceal(sstring argument) {
  sstring name_buf;
  int rc;
  TBeing* vict;

  if (getSkillValue(SKILL_CONCEALMENT) <= 0) {
    sendTo("You do not have the ability to conceal paths.\n\r");
    return;
  }
  argument = one_argument(argument, name_buf);

  if (name_buf.empty()) {
    vict = this;
  } else {
    if (is_abbrev(name_buf, "off") || is_abbrev(name_buf, "stop")) {
      one_argument(argument, name_buf);
      if (!name_buf.empty()) {
        vict = get_char_room_vis(this, name_buf);
        if (!vict) {
          sendTo("No such person present.\n\r");
          sendTo("Syntax: conceal off <person>\n\r");
          return;
        } else {
          affectedData* AffL;

          for (AffL = vict->affected; AffL; AffL = AffL->next)
            if (AffL->type == SKILL_CONCEALMENT)
              break;
          if (!AffL) {
            sendTo(
              "How can you stop concealing their trail?  You never "
              "started.\n\r");
            return;
          } else if (AffL->be != this) {
            sendTo("Yes, their trail is concealed...But not by you.\n\r");
            return;
          }

          sendTo(COLOR_MOBS,
            format("You stop concealing %s's trail.\n\r") % vict->getName());
          vict->affectFrom(SKILL_CONCEALMENT);
          return;
        }
      }
      if (affectedBySpell(SKILL_CONCEALMENT)) {
        sendTo("You stop trying to conceal your trail.\n\r");
        affectFrom(SKILL_CONCEALMENT);
        return;
      } else {
        sendTo("You were not trying to conceal your trail.\n\r");
        return;
      }
    } else {
      vict = get_char_room_vis(this, name_buf);
      if (!vict) {
        sendTo("No such person present.\n\r");
        sendTo("Syntax: conceal <person>\n\r");
        return;
      }
    }
  }
  rc = conceal(this, vict);
  if (rc)
    addSkillLag(SKILL_CONCEALMENT, rc);
}

// return FALSE to cease tracking
int conceal(TBeing* caster, TBeing* vict) {
  affectedData aff;
  int level = caster->getSkillLevel(SKILL_CONCEALMENT);
  int lnd = caster->getSkillValue(SKILL_CONCEALMENT);

  if (caster->getPosition() != POSITION_STANDING) {
    caster->sendTo("You need to be standing in order to conceal trails.\n\r");
    return FALSE;
  }
  if (caster->fight()) {
    caster->sendTo(
      "The ensuing battle makes it difficult to conceal a trail.\n\r");
    return FALSE;
  }
  if (vict->fight()) {
    act("You can't conceal $N's path while $E is fighting.", FALSE, caster, 0,
      vict, TO_CHAR);
    return FALSE;
  }
  if (vict->affectedBySpell(SKILL_CONCEALMENT)) {
    if (vict == caster)
      act("Your path is already being concealed.", FALSE, caster, 0, 0,
        TO_CHAR);
    else
      act("$N's path is already being concealed.", FALSE, caster, 0, vict,
        TO_CHAR);
    return FALSE;
  }
  if (vict != caster) {
    if (lnd <= 50) {
      act(
        "You lack the training to conceal other people's path with any degree "
        "of success.",
        FALSE, caster, 0, 0, TO_CHAR);
      return FALSE;
    }
    // even if they can, reduce the chance of success
    lnd /= 2;
  }

  if (caster == vict) {
    act("You attempt to conceal your path.", FALSE, caster, 0, 0, TO_CHAR);
    act("$n attempts to conceal $s path.", FALSE, caster, 0, 0, TO_ROOM);
  } else {
    act("You attempt to conceal $N's path.", FALSE, caster, 0, vict, TO_CHAR);
    act("$n attempts to conceal your path.", FALSE, caster, 0, vict, TO_VICT);
    act("$n attempts to conceal $N's path.", FALSE, caster, 0, vict,
      TO_NOTVICT);
  }

  aff.type = SKILL_CONCEALMENT;
  aff.duration = (level / 2 + 1) * Pulse::UPDATES_PER_MUDHOUR;
  aff.be = caster;

  if (caster->bSuccess(lnd, SKILL_CONCEALMENT)) {
    aff.level = level;
    aff.modifier = lnd;

    vict->affectTo(&aff);
  } else {
    aff.level = 0;
    aff.modifier = 0;

    vict->affectTo(&aff);
  }

  return TRUE;
}

int TBeing::doDisguise(const char* arg) {
  char name_buf[MAX_INPUT_LENGTH];
  int rc;

  if (!doesKnowSkill(SKILL_DISGUISE)) {
    sendTo("You know nothing about disguising yourself.\n\r");
    return FALSE;
  }
  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("You can't disguise yourself while going berserk.\n\r");
    return FALSE;
  }
  one_argument(arg, name_buf, cElements(name_buf));

  rc = disguise(this, name_buf);
  if (rc)
    addSkillLag(SKILL_DISGUISE, rc);

  return rc;
}

static const int LAST_DISGUISE_MOB = 28;
struct PolyType DisguiseList[LAST_DISGUISE_MOB] = {
  /*
  Use RACE_NORACE when anything can use it.  Race will be subbed
  later, not name/desc tho, for the thiefs race.

  All grabbed, so far, from zones: 0-4

  If you add any between the first one and up to the Racial Specific,
  or remove one either, then please address the stuff in gaining.cc
  also.  Thanks  --Lapsos
   */

  {"citizen male", 1, 1, 108, DISC_STEALTH, RACE_NORACE},
  {"citizen female", 1, 1, 109, DISC_STEALTH, RACE_NORACE},
  {"bar-hopper male", 5, 10, 52, DISC_STEALTH, RACE_NORACE},
  {"bar-hopper female", 5, 10, 53, DISC_STEALTH, RACE_NORACE},
  {"church male", 10, 20, 50, DISC_STEALTH, RACE_NORACE},
  {"church female", 10, 20, 51, DISC_STEALTH, RACE_NORACE},
  {"old-man male", 15, 30, 174, DISC_STEALTH, RACE_NORACE},
  {"old-woman female", 15, 30, 175, DISC_STEALTH, RACE_NORACE},
  {"patron male", 20, 40, 300, DISC_STEALTH, RACE_NORACE},
  {"patron female", 20, 40, 301, DISC_STEALTH, RACE_NORACE},

  {"peasant", 10, 20, 110, DISC_STEALTH, RACE_NORACE},
  {"bard", 25, 50, 188, DISC_STEALTH, RACE_NORACE},
  {"pilgrim", 30, 60, 186, DISC_STEALTH, RACE_NORACE},

  /** Gender Specific (Meaning not a set) **/
  {"drunk male", 15, 30, 106, DISC_STEALTH, RACE_NORACE},
  {"evening-lady female", 15, 30, 130, DISC_STEALTH, RACE_NORACE},
  {"deputy male", 20, 40, 121, DISC_STEALTH, RACE_NORACE},
  {"gypsy male", 20, 40, 191, DISC_STEALTH, RACE_NORACE},
  {"cityguard male", 25, 50, 101, DISC_STEALTH, RACE_NORACE},

  /** Race Specfic **/
  {"merchant dwarf", 20, 40, 197, DISC_STEALTH, RACE_DWARF},
  {"ambassador male", 40, 80, 125, DISC_STEALTH, RACE_DWARF},
  {"young-male gnome", 20, 40, 124, DISC_STEALTH, RACE_GNOME},
  {"missionary male", 40, 80, 126, DISC_STEALTH, RACE_GNOME},
  {"outcast ogre", 20, 40, 196, DISC_STEALTH, RACE_OGRE},
  {"vigilante male", 40, 80, 127, DISC_STEALTH, RACE_OGRE},
  {"trader hobbit", 20, 40, 198, DISC_STEALTH, RACE_HOBBIT},
  {"emissary male", 40, 80, 128, DISC_STEALTH, RACE_HOBBIT},  //***
  {"tradeswoman female", 20, 40, 138, DISC_STEALTH, RACE_ELVEN},
  {"traveler elf", 40, 80, 195, DISC_STEALTH, RACE_ELVEN},
};

int disguise(TBeing* caster, char* buffer) {
  int i, duration, column = 0;
  TMonster* mob;
  affectedData aff;
  affectedData aff2;
  affectedData aff3;

  if (!*buffer)
    caster->sendTo("You know how to apply the following disguises:\n\r");

  // Find not only the first match but the first match that
  // also works out in comparision to the thief.
  for (i = 0; (i < LAST_DISGUISE_MOB); i++) {
    if (((isname("male", DisguiseList[i].name) &&
           caster->getSex() != SEX_MALE) ||
          (isname("female", DisguiseList[i].name) &&
            caster->getSex() != SEX_FEMALE)) &&
        !caster->isImmortal())
      continue;

    if ((signed)DisguiseList[i].tRace != RACE_NORACE && !caster->isImmortal() &&
        caster->getRace() != (signed)DisguiseList[i].tRace)
      continue;

    if (DisguiseList[i].level > caster->GetMaxLevel())
      continue;

    if (DisguiseList[i].learning > caster->getSkillValue(SKILL_DISGUISE))
      continue;

    if (!*buffer) {
      caster->sendTo(format("%-25s") % DisguiseList[i].name);
      if ((column++) == 2) {
        caster->sendTo("\n\r");
        column = 0;
      }
      continue;
    } else if (!isname(buffer, DisguiseList[i].name))
      continue;

    break;
  }

  if (!*buffer) {
    caster->sendTo("\n\r");
    return FALSE;
  }

  if (!caster->isImmortal() && caster->checkForSkillAttempt(SKILL_DISGUISE)) {
    act("You are not prepared to try to disguise yourself again so soon.",
      FALSE, caster, NULL, NULL, TO_CHAR);
    return SPELL_FAIL;
  }

  if (i >= LAST_DISGUISE_MOB) {
    caster->sendTo("You haven't a clue where to start on that one.\n\r");
    return FALSE;
  }

  int level = caster->getSkillLevel(SKILL_DISGUISE);
  int bKnown = caster->getSkillValue(SKILL_DISGUISE);

  discNumT das = getDisciplineNumber(SKILL_DISGUISE, FALSE);
  if (das == DISC_NONE) {
    vlogf(LOG_BUG, "bad disc for SKILL_DISGUISE");
    return FALSE;
  }
  if ((caster->getDiscipline(das)->getLearnedness() <
        DisguiseList[i].learning) &&
      (level < DisguiseList[i].level)) {
    caster->sendTo(
      "You don't seem to have the ability to disguise yourself as that "
      "(yet).\n\r");
    return FALSE;
  }
  if (!(mob = read_mobile(DisguiseList[i].number, VIRTUAL))) {
    caster->sendTo("You couldn't envision an image of that creature.\n\r");
    return FALSE;
  }
  thing_to_room(mob, Room::VOID);
  mob->swapToStrung();

  // Check to make sure that there is no snooping going on.
  if (!caster->desc || caster->desc->snoop.snooping) {
    caster->sendTo("Nothing seems to happen.\n\r");
    delete mob;
    mob = NULL;
    return FALSE;
  }
  if (caster->desc->original) {
    // implies they are switched, while already switched (as x disguise)
    caster->sendTo("You already seem to be switched.\n\r");
    delete mob;
    mob = NULL;
    return FALSE;
  }

  aff.type = AFFECT_SKILL_ATTEMPT;
  aff.location = APPLY_NONE;
  aff.duration = (2 + (level / 5)) * Pulse::UPDATES_PER_MUDHOUR;
  aff.bitvector = 0;
  aff.modifier = SKILL_DISGUISE;

  if (!caster->bSuccess(bKnown, SKILL_DISGUISE)) {
    caster->sendTo("You seem to have screwed something up.\n\r");
    delete mob;
    mob = NULL;
    caster->affectTo(&aff, -1);
    return TRUE;
  }

  switch (critSuccess(caster, SKILL_DISGUISE)) {
    case CRIT_S_KILL:
      CS(SKILL_DISGUISE);
      duration = 20 * Pulse::UPDATES_PER_MUDHOUR;
      break;
    case CRIT_S_TRIPLE:
      if (caster->getSkillLevel(SKILL_DISGUISE) > 20)
        CS(SKILL_DISGUISE);

      duration = 15 * Pulse::UPDATES_PER_MUDHOUR;
      break;
    case CRIT_S_DOUBLE:
      if (caster->getSkillLevel(SKILL_DISGUISE) > 40)
        CS(SKILL_DISGUISE);

      duration = 15 * Pulse::UPDATES_PER_MUDHOUR;
      break;
    default:
      duration = 10 * Pulse::UPDATES_PER_MUDHOUR;
      break;
  }
  act("You apply your skills and make yourself look like $N.", TRUE, caster,
    NULL, mob, TO_CHAR);
  act("$n applies $s skills and makes $mself look like $N.", TRUE, caster, NULL,
    mob, TO_ROOM);

  // first add the attempt -- used to regulate attempts
  aff.duration = duration + (2 * Pulse::UPDATES_PER_MUDHOUR);
  caster->affectTo(&aff, -1);

  aff3.type = AFFECT_SKILL_ATTEMPT;
  aff3.location = APPLY_NONE;
  aff3.duration = duration + (2 * Pulse::UPDATES_PER_MUDHOUR);
  aff3.bitvector = 0;
  aff3.modifier = SKILL_DISGUISE;
  mob->affectTo(&aff3, -1);

  // then the skill -- used to wear off
  aff2.type = SKILL_DISGUISE;
  aff2.location = APPLY_NONE;
  aff2.duration = duration;
  aff2.bitvector = 0;
  aff2.modifier = 0;
  mob->affectTo(&aff2, -1);

  DisguiseStuff(caster, mob);

  --(*mob);
  *caster->roomp += *mob;
  --(*caster);
  thing_to_room(caster, Room::POLY_STORAGE);

  // stop following whoever you are following.
  if (caster->master)
    caster->stopFollower(TRUE);

  // switch caster into mobile
  caster->desc->character = mob;
  caster->desc->original = dynamic_cast<TPerson*>(caster);

  mob->desc = caster->desc;
  caster->desc = NULL;
  caster->polyed = POLY_TYPE_DISGUISE;

  SET_BIT(mob->specials.act, ACT_DISGUISED);
  SET_BIT(mob->specials.act, ACT_POLYSELF);
  SET_BIT(mob->specials.act, ACT_NICE_THIEF);
  SET_BIT(mob->specials.act, ACT_SENTINEL);
  REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
  REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
  REMOVE_BIT(mob->specials.act, ACT_DIURNAL);
  REMOVE_BIT(mob->specials.act, ACT_NOCTURNAL);

  appendPlayerName(caster, mob);

  /*
   It is critical to remember that some diguises are race/sex independent
   so they Must be set here else it'll not always fit.
    */
  mob->setRace(caster->getRace());
  mob->setHeight(caster->getHeight());
  mob->setWeight(caster->getWeight());

  return TRUE;
}

int TBeing::doSneak(const char* argument) {
  int rc = 0;
  char arg[80];

  spellNumT skill = getSkillNum(SKILL_SNEAK);
  argument = one_argument(argument, arg, cElements(arg));

  if (!doesKnowSkill(skill)) {
    sendTo("You know nothing about sneaking.\n\r");
    return FALSE;
  }
  if (!*arg && desc) {
    if (affectedBySpell(skill) || checkForSkillAttempt(skill)) {
      sendTo("You are already trying to be sneaky.\n\r");
      return FALSE;
    }
  }

  if (*arg) {
    if (is_abbrev(arg, "off") || is_abbrev(arg, "stop")) {
      // Deliberate: clear ONLY sneak inline. Do NOT call loseSneak() or
      // breakStealth() here — an explicit 'sneak stop' must not also strip
      // an unrelated skulk affect. Keep this single-affect.
      if (affectedBySpell(skill) || checkForSkillAttempt(skill)) {
        sendTo("You will no longer try to be sneaky.\n\r");
        removeSkillAttempt(skill);
        if (affectedBySpell(skill))
          affectFrom(skill);
        return FALSE;
      } else {
        sendTo("You were not trying to be sneaky.\n\r");
        return FALSE;
      }
    } else if (is_abbrev(arg, "retry")) {
      if (affectedBySpell(skill) || checkForSkillAttempt(skill)) {
        removeSkillAttempt(skill);
        if (affectedBySpell(skill))
          affectFrom(skill);
      }
    }
  }
  rc = sneak(this, skill);
  if (rc)
    addSkillLag(skill, rc);

  return rc;
}

int sneak(TBeing* thief, spellNumT skill) {
  affectedData af;
  const int SNEAK_COST = 3;

  if (thief->fight()) {
    thief->sendTo("No way!! You simply can NOT sneak while fighting!\n\r");
    return FALSE;
  }
  if (thief->affectedBySpell(skill)) {
    thief->affectFrom(skill);
    thief->sendTo("You are no longer sneaky.\n\r");

    // this should technically be a return TRUE, but by sending back false
    // we don't lag them for ending their sneak
    return FALSE;
  }
  if (thief->riding) {
    thief->sendTo("It is impossible to sneak while mounted.\n\r");
    return FALSE;
  }
  if (thief->isFlying()) {
    thief->sendTo("It is impossible to sneak while flying.\n\r");
    return FALSE;
  }
  if (thief->getMove() < SNEAK_COST) {
    thief->sendTo("You don't have the vitality to do that.\n\r");
    return FALSE;
  }
  thief->addToMove(-SNEAK_COST);
  thief->sendTo("Ok, you'll try to move silently for a while.\n\r");

  int level = thief->getSkillLevel(skill);
  int bKnown = thief->getSkillValue(skill);
  bKnown += thief->plotStat(STAT_CURRENT, STAT_DEX, -70, 15, 0);

  if (thief->bSuccess(bKnown, skill)) {
    af.modifier = 1 + level / 2;
    af.type = skill;
    af.duration = 5 * level;
    af.modifier = 1 + level / 2;
    af.location = APPLY_CAN_BE_SEEN;
    af.bitvector = AFF_SNEAK;
    thief->affectTo(&af, -1);
    return TRUE;
  } else {
    af.modifier = 0;
    af.type = AFFECT_SKILL_ATTEMPT;
    af.duration = 5 * level;
    af.modifier = skill;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    thief->affectTo(&af, -1);
    return TRUE;
  }
  return TRUE;
}

int TBeing::doHide() {
  if (!doesKnowSkill(SKILL_HIDE)) {
    sendTo("You know nothing about hiding.\n\r");
    return 0;
  }

  if (fight()) {
    sendTo("No way!! You simply can NOT hide while fighting!\n\r");
    return 0;
  }

  if (riding) {
    sendTo("Yeah... right... while mounted\n\r");
    return 0;
  }

  if (isAffected(AFF_HIDE)) {
    sendTo("You're already blending into your surroundings.\n\r");
    return 0;
  }

  // TODO: Add a separate success check for hiding, after bSuccess passes, that
  // accounts for circumstantial modifiers, such as: room lighting, terrain
  // type, stats, etc. The sorts of things that would make hiding easier or
  // harder regardless of the thief's skill level. bSuccess should only
  // determine whether the thief has practiced the mechanics of hiding enough to
  // properly attempt it, while the second check should determine whether the
  // thief can actually find a good hiding spot in the current environment.

  if (bSuccess(SKILL_HIDE)) {
    SET_BIT(specials.affectedBy, AFF_HIDE);
    sendTo("You deftly blend into your surroundings.\n\r");
  } else {
    sendTo("You fail to blend into your surroundings.\n\r");
  }

  addSkillLag(SKILL_HIDE, 0);
  return 1;
}

int TBeing::doSubterfuge(const char* arg) {
  TBeing* victim;
  char name_buf[MAX_INPUT_LENGTH];
  int rc;

  if (!doesKnowSkill(SKILL_SUBTERFUGE)) {
    sendTo("You know nothing about the art of subterfuge.\n\r");
    return FALSE;
  }
  one_argument(arg, name_buf, cElements(name_buf));
  if (!(victim = get_char_room_vis(this, name_buf))) {
    sendTo("No one here by that name.\n\r");
    return FALSE;
  }

  rc = subterfuge(this, victim);
  if (rc)
    addSkillLag(SKILL_SUBTERFUGE, rc);

  return rc;
}
int subterfugeMiss(TBeing* thief, TBeing* victim) {
  if (victim->isPc()) {
    return false;
  }

  if (!thief->fight()) {
    if (!thief->isAgile(0)) {
      act("$n tries to put on a show for you and falls on $s face!", FALSE,
        thief, NULL, victim, TO_VICT);
      act("$n tries to put on a show for $N and falls on $s face!", FALSE,
        thief, NULL, victim, TO_NOTVICT);
      act("You try to put on a show for $N and fall on your face!", FALSE,
        thief, NULL, victim, TO_CHAR);
      thief->crashLanding();
    } else {
      act("$n does a dumb little dance for you. $e has no talent!", FALSE,
        thief, NULL, victim, TO_VICT);
      act("$n does a dumb little dance for $N. $e has no talent!", FALSE, thief,
        NULL, victim, TO_NOTVICT);
      act("You do a dumb little dance for $N. You have no talent!", FALSE,
        thief, NULL, victim, TO_CHAR);
    }

    thief->setCharFighting(victim);
    victim->setVictFighting(thief);
  }
  return true;
}

int subterfugePlayer(TBeing* thief, TBeing* victim) {
  if (!victim->isPc()) {
    return false;
  }
  // PC success actions
  if (victim->isCombatMode(ATTACK_BERSERK)) {
    victim->setCombatMode(ATTACK_NORMAL);
    victim->affectFrom(SKILL_BERSERK);
    act("$N's berserker rage subsides as you calm $M down.", FALSE, thief, NULL,
      victim, TO_CHAR);
    act("$n's subtle gestures and words calm your rage.", FALSE, thief, NULL,
      victim, TO_VICT);
    act("$n somehow calms $N's berserker rage.", FALSE, thief, NULL, victim,
      TO_NOTVICT);
  } else {
    act("You put on a show to confuse $N.", FALSE, thief, NULL, victim,
      TO_CHAR);
    act("$n puts on a show for you. It is a confusing performance!", FALSE,
      thief, NULL, victim, TO_VICT);
    act("$n puts on a show for $N. It is a confusing performance!", FALSE,
      thief, NULL, victim, TO_NOTVICT);
    victim->addToWait(combatRound(1));
  }

  return true;
}

int subterfugeHit(TBeing* thief, TBeing* victim) {
  // Mob success actions
  int level = thief->getSkillLevel(SKILL_SUBTERFUGE);
  level = (max(1, (level + (thief->getChaReaction()))));

  int advLearning = thief->getAdvLearning(SKILL_SUBTERFUGE);
  act("$N seems pretty confused by your show!", FALSE, thief, NULL, victim,
    TO_CHAR, ANSI_ORANGE);
  act("$n confuses $N with a show.", FALSE, thief, NULL, victim, TO_NOTVICT,
    ANSI_ORANGE);
  act("$n confuses you with a show.", FALSE, thief, NULL, victim, TO_VICT,
    ANSI_ORANGE);

  // Apply subterfuge affect for PER reduction
  affectedData af;
  af.type = SKILL_SUBTERFUGE;
  af.duration = level * Pulse::UPDATES_PER_MUDHOUR;
  af.modifier = -10;
  af.location = APPLY_PER;
  af.bitvector = 0;
  victim->affectTo(&af);

  // Apply subterfuge affect for FOC reduction
  affectedData af2;
  af2.type = SKILL_SUBTERFUGE;
  af2.duration = level * Pulse::UPDATES_PER_MUDHOUR;
  af2.modifier = -10;
  af2.location = APPLY_FOC;
  af2.bitvector = 0;
  victim->affectTo(&af2);

  // Apply subterfuge affect for vision reduction
  affectedData af3;
  af3.type = SKILL_SUBTERFUGE;
  af3.duration = level * Pulse::UPDATES_PER_MUDHOUR;
  af3.modifier = -5;
  af3.location = APPLY_VISION;
  af3.bitvector = 0;
  victim->affectTo(&af3);

  TMonster* mob = dynamic_cast<TMonster*>(victim);

  // Tier 2: 75% skill - Remove hunting/hateful flags
  if (mob && level >= 75) {
    REMOVE_BIT(mob->specials.act, ACT_HUNTING);
    REMOVE_BIT(mob->specials.act, ACT_HATEFUL);
    act("$N seems to forget why $E was so angry.", FALSE, thief, NULL, victim,
      TO_CHAR);
    act("$N seems to forget why $E was so angry.", FALSE, thief, NULL, victim,
      TO_NOTVICT);
    act("You seem to forget why you were so angry.", FALSE, thief, NULL, victim,
      TO_VICT);
  }

  // Tier 3: 20% advanced discipline - Remove thief/cityguard procs
  if (mob && advLearning >= 20) {
    if (IS_SET(mob->specials.act, ACT_NICE_THIEF)) {
      act("$N seems to look a little less mischievous.", FALSE, thief, NULL,
        victim, TO_CHAR, ANSI_PURPLE_BOLD);
      REMOVE_BIT(mob->specials.act, ACT_NICE_THIEF);
    }
    if (IS_SET(mob->specials.act, ACT_GUARDIAN)) {
      act("$N seems to look a little less watchful.", FALSE, thief, NULL,
        victim, TO_CHAR, ANSI_YELLOW);
      REMOVE_BIT(mob->specials.act, ACT_GUARDIAN);
    }
    if (IS_SET(mob->specials.act, ACT_AFRAID)) {
      act("$N seems to look a little less afraid.", FALSE, thief, NULL, victim,
        TO_CHAR, ANSI_ORANGE);
      REMOVE_BIT(mob->specials.act, ACT_AFRAID);
    }

    if (mob->spec == SPEC_CITYGUARD || mob->spec == SPEC_JANITOR ||
        mob->spec == SPEC_ARCHER) {
      if (mob->spec == SPEC_CITYGUARD) {
        act("$N seems to forget $S duties as a <Y>cityguard.<z>", FALSE, thief,
          NULL, victim, TO_CHAR);
      }
      if (mob->spec == SPEC_JANITOR) {
        act("$N seems to forget $S duties as a <B>janitor.<z>", FALSE, thief,
          NULL, victim, TO_CHAR);
      }
      if (mob->spec == SPEC_ARCHER) {
        act("$N seems to forget $S duties as an <o>archer.<z>", FALSE, thief,
          NULL, victim, TO_CHAR);
      }
      mob->spec = 0;  // Set to no special proc
    }
  }

  // Tier 4: 50% advanced discipline - Remove aggro flag
  if (mob && advLearning >= 50 && IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    act("$N's aggressive demeanor softens considerably.", FALSE, thief, NULL,
      victim, TO_CHAR, ANSI_YELLOW);
    act("$N's aggressive demeanor softens considerably.", FALSE, thief, NULL,
      victim, TO_NOTVICT, ANSI_YELLOW);
    act("Your demeanor softens considerably.", FALSE, thief, NULL, victim,
      TO_VICT, ANSI_YELLOW);
  }

  return true;
}

int subterfugeSuccess(TBeing* thief, TBeing* victim) {
  // If PC, handle PC-specific logic and exit
  if (victim->isPc()) {
    return subterfugePlayer(thief, victim);
  }

  // Tier 1: Remove wary status (base effect, always happens on success)
  if (victim->isWary()) {
    victim->affectFrom(AFFECT_WARY);
    act("$N seems less wary now.", FALSE, thief, NULL, victim, TO_CHAR);
    act("$N seems less wary now.", FALSE, thief, NULL, victim, TO_NOTVICT);
    act("You seem less wary now.", FALSE, thief, NULL, victim, TO_VICT);
  }

  // For mobs, do specialAttack
  int situationalMod = 0;
  if (!victim->isWise()) {
    situationalMod -= 10;
  }
  if (!victim->isIntelligent()) {
    situationalMod -= 10;
  }
  if (thief->isCharismatic()) {
    situationalMod += 10;
  }

  int attackValue = thief->specialAttack(victim, SKILL_SUBTERFUGE,
    situationalMod, STAT_CHA, STAT_INT, STAT_PER, STAT_WIS, true);

  if (attackValue <= FAILURE) {
    return subterfugeMiss(thief, victim);
  }

  // Crit actions
  if (attackValue >= GUARANTEED_SUCCESS) {
    victim->cantHit += max(1, thief->getChaReaction());
    act("$N seems <p>TOTALLY befuddled<1> by your skills!", FALSE, thief, NULL,
      victim, TO_CHAR);
    act("$n <p>TOTALLY befuddles<1> you with $S antics!", FALSE, thief, NULL,
      victim, TO_VICT);
    act("$n <p>TOTALLY befuddles<1> $N with $S antics!", FALSE, thief, NULL,
      victim, TO_NOTVICT);
  }

  return subterfugeHit(thief, victim);
}

int subterfugeFail(TBeing* thief, TBeing* victim) {
  thief->sendTo("You fail to execute the subterfuge properly.\n\r");
  if (!thief->isCharismatic() && !victim->isPc()) {
    act("WUH OH! $N looks pretty pissed off.", FALSE, thief, NULL, victim,
      TO_CHAR);
    act("$n tried to trick you! $e's gonna pay for that!", FALSE, thief, NULL,
      victim, TO_VICT);
    act("$n tried to trick $N! $e's gonna pay for that!", FALSE, thief, NULL,
      victim, TO_NOTVICT);
    thief->setCharFighting(victim);
    victim->setVictFighting(thief);
  }
  return true;
}

int subterfuge(TBeing* thief, TBeing* victim) {
  // Hard fail checks
  if (thief->fight()) {
    thief->sendTo("No way!! You simply can NOT concentrate!\n\r");
    return false;
  }

  if (victim->fight()) {
    thief->sendTo("You can't get their attention while they are fighting.\n\r");
    return false;
  }

  if (thief->getMove() < 25) {
    thief->sendTo("You are too tired to make the move.\n\r");
    return false;
  }

  if (victim->isPc() && (thief->getSkillLevel(SKILL_SUBTERFUGE) < 80)) {
    thief->sendTo("You aren't skilled enough to dazzle characters, yet.\n\r");
    return false;
  }

  if (thief->checkPeaceful("You can't subterfuge in a place of refuge.\n\r"))
    return false;

  int level = thief->getSkillLevel(SKILL_SUBTERFUGE);
  level = (max(1, (level + (thief->getChaReaction()))));
  int bKnown = thief->getSkillValue(SKILL_SUBTERFUGE);

  if (thief->isNotPowerful(victim, level, SKILL_SUBTERFUGE, SILENT_YES)) {
    act("$N's mind is too powerful to be confused.", FALSE, thief, NULL, victim,
      TO_CHAR);
    return false;
  }

  if (!victim->isHumanoid() && bKnown < 80) {
    thief->sendTo("You aren't skilled enough to trick monsters, yet.\n\r");
    return false;
  }

  thief->addToMove(-25);

  // bSuccess check
  if (!thief->bSuccess(bKnown, SKILL_SUBTERFUGE)) {
    return subterfugeFail(thief, victim);
  }

  return subterfugeSuccess(thief, victim);
}

int TBeing::SpyCheck() {
  if (bSuccess(SKILL_SPY))
    return TRUE;

  return FALSE;
}

int TBeing::doSpy() {
  if (!doesKnowSkill(SKILL_SPY)) {
    sendTo("You know nothing about spying.\n\r");
    return FALSE;
  }
  return spy(this);
}

int spy(TBeing* thief) {
  affectedData aff;

  if (thief->affectedBySpell(SKILL_SPY)) {
    thief->sendTo("You cease spying.\n\r");
    thief->affectFrom(SKILL_SPY);
    return FALSE;
  }

  if (thief->isAffected(AFF_SCRYING)) {
    thief->sendTo("You are already doing your best spy imitation.\n\r");
    return FALSE;
  }
  thief->sendTo("Ok, try your best to spy.\n\r");

  int bKnown = thief->getSkillValue(SKILL_SPY);

  // We will set an affect regardless of success, failure will just
  // not set the AFF_SCRYING bit so check for isAff(scry) in code to
  // see if spying
  aff.type = SKILL_SPY;
  aff.duration = (((int)bKnown / 10) + 1) * Pulse::UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;

  if (thief->bSuccess(bKnown, SKILL_SPY)) {
    aff.bitvector = AFF_SCRYING;
    thief->affectTo(&aff, -1);
    return TRUE;
  }
  aff.bitvector = 0;
  thief->affectTo(&aff, -1);
  return TRUE;
}

bool willBreakHide(cmdTypeT tCmd) {
  switch (tCmd) {
    case CMD_BACKSTAB:
    case CMD_SLIT:
    case CMD_BASH:
    case CMD_BONEBREAK:
    case CMD_CHOP:
    case CMD_CUDGEL:
    case CMD_HURL:
    case CMD_KICK:
    case CMD_KNEESTRIKE:
    case CMD_QUIVPALM:
    case CMD_SHOOT:
    case CMD_SHOULDER_THROW:
    case CMD_SLAM:
    case CMD_SPIN:
    case CMD_STAB:
    case CMD_THROW:
    case CMD_TRIP:
    case CMD_LOOK:
    case CMD_SCORE:
    case CMD_TROPHY:
    case CMD_INVENTORY:
    case CMD_HELP:
    case CMD_WHO:
    case CMD_NEWS:
    case CMD_EQUIPMENT:
    case CMD_WEATHER:
    case CMD_SAVE:
    case CMD_EXITS:
    case CMD_TIME:
    case CMD_HIDE:
    case CMD_SNEAK:
    case CMD_SKULK:
    case CMD_QUEST:
    case CMD_LEVELS:
    case CMD_WIZLIST:
    case CMD_CONSIDER:
    case CMD_CREDITS:
    case CMD_TITLE:
    case CMD_ATTRIBUTE:
    case CMD_WORLD:
    case CMD_SPY:
    case CMD_CONCEAL:
    case CMD_REST:
    case CMD_SIT:
    case CMD_STAND:
    case CMD_SIGN:
    case CMD_PTELL:
    case CMD_EAT:
    case CMD_DRINK:
    case CMD_CLS:
    case CMD_PROMPT:
    case CMD_ALIAS:
    case CMD_CLEAR:
    case CMD_MOTD:
    case CMD_PRACTICE:
    case CMD_HISTORY:
    case CMD_EVALUATE:
    case CMD_DISGUISE:
    case CMD_EMAIL:
    case CMD_AFK:
    case CMD_SPELLS:
    case CMD_COMPARE:
    case CMD_ZONES:
    case CMD_NEWBIE:
    case CMD_REQUEST:
    case CMD_IGNORE:
    case MAX_CMD_LIST:
      return false;
    default:
      return true;
  }
}

int skulk(TBeing* thief, spellNumT skill) {
  if (thief->fight()) {
    thief->sendTo("You can't skulk while fighting!\n\r");
    return FALSE;
  }

  if (thief->riding) {
    thief->sendTo("You can't skulk while mounted!\n\r");
    return FALSE;
  }

  if (thief->affectedBySpell(SKILL_SKULK)) {
    thief->sendTo("You are already skulking!\n\r");
    return FALSE;
  }

  // Check if they have enough movement points
  if (thief->getMove() < 15) {
    thief->sendTo("You're too tired to skulk right now.\n\r");
    return FALSE;
  }

  // Deduct initial movement cost
  thief->addToMove(-15);

  thief->sendTo("You begin practicing your skulking technique.\n\r");
  act("$n begins practicing $s skulking technique.", TRUE, thief, 0, 0, TO_ROOM);

  // Calculate task time based on skill
  int taskTime = max(5, 20 - (thief->getSkillValue(skill) / 10));

  // Start the task
  start_task(thief, NULL, NULL, TASK_SKULK, "", taskTime, thief->in_room, 0, 0, 0);

  return TRUE;
}

int TBeing::doSkulk(const char* argument) {
  spellNumT skill = getSkillNum(SKILL_SKULK);
  char arg[80];

  if (!doesKnowSkill(skill)) {
    sendTo("You know nothing about skulking.\n\r");
    return FALSE;
  }

  one_argument(argument, arg, cElements(arg));

  if (*arg && (is_abbrev(arg, "off") || is_abbrev(arg, "stop"))) {
    bool wasSkulking = false;

    // If still in the build-up task, abort it cleanly. The task's own
    // CMD_STOP handler isn't reachable from here, so emit the equivalent
    // messages inline.
    if (task && task->task == TASK_SKULK) {
      sendTo("You stop your skulking.\n\r");
      act("$n stops skulking.", TRUE, this, 0, 0, TO_ROOM);
      stopTask();
      wasSkulking = true;
    }

    // If the buff is already active, clear it. Use loseSkulk() — NOT
    // breakStealth() — so an explicit 'skulk stop' does not also strip
    // an unrelated sneak affect.
    if (affectedBySpell(SKILL_SKULK)) {
      loseSkulk();
      wasSkulking = true;
    }

    if (!wasSkulking)
      sendTo("You are not skulking.\n\r");

    return FALSE;
  }

  return skulk(this, skill);
}
