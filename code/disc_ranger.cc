#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_ranger.h"
#include "obj_portal.h"

void TBeing::doTrack(const char *argument)
{
  char namebuf[256], found = FALSE;
  int dist = 0, targrm, worked, skill;
  int code;
  TBeing *scan;
  affectedData aff, *Vaff;
  TThing *t = NULL;
  char buf[256]="\0\0\0", buf2[256]="\0\0\0";

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
      !roomp->isRoomFlag(ROOM_ALWAYS_LIT) &&
      !isAffected(AFF_TRUE_SIGHT)) {
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
      dist = max(25, (int) (((((roomCount*((level+1)/2))/1000)+
                           min(50, learning)))/2));
    else
      dist = max(20, (int) (((((roomCount*((level+1)/2))/1000)+
                           min(20, learning)))/2));
  } else if (affectedBySpell(SPELL_TRAIL_SEEK)) {
    learning = getSkillValue(SPELL_TRAIL_SEEK);
    level = getSkillLevel(SPELL_TRAIL_SEEK);
    dist = max(15, (int) (((((roomCount*((level+1)/2))/1000)+
                         min(5, learning)))/2));
  }

  if (hasClass(CLASS_THIEF) || hasClass(CLASS_RANGER))
    dist *= 2;
  else if (hasClass(CLASS_WARRIOR))
    dist /= 2;
  else if (hasClass(CLASS_MAGE))
    dist += getLevel(MAGE_LEVEL_IND);
  else
    dist = dist;
 
  switch (getRace()) {
    case RACE_GIANT:
    case RACE_ELVEN:
      dist *= 2;                // even better 
      break;
    case RACE_DEVIL:
    case RACE_DEMON:
      dist = MAX_ROOMS;         //  4 as good as can be 
      break;
    default:
      break;
  }
 
  if (isImmortal())
    dist = MAX_ROOMS;
 
  hunt_dist = dist;
  specials.hunting = 0;
  targrm = inRoom();
 
  // note: -dist will look THRU doors.
  // all subsequent calls use track() which does not go thru doors
  // this is intentional so they lose track after 1 step
  if ((level < MIN_GLOB_TRACK_LEV) ||
      (affectedBySpell(SPELL_TRAIL_SEEK)))
    code = find_path(in_room, find_closest_being_by_name, (void *) namebuf,
                      -dist, 1, &targrm);
  else
    code = find_path(in_room, find_closest_being_by_name, (void *) namebuf,
                      -dist, 0, &targrm);

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
    for (Vaff = specials.hunting->affected; Vaff; Vaff = Vaff->next)
      if (Vaff->type == SKILL_CONCEALMENT)
        skill = -Vaff->modifier;
    if (affectedBySpell(SPELL_TRAIL_SEEK)) {
      skill += 50;
      worked = (::number(0, 110) < skill);
    } else {
      skill += getSkillValue(SKILL_TRACK);
      worked = bSuccess(this, skill, SKILL_TRACK);
    }
    if (worked) {
      if (code <= 9)
        sendTo(fmt("%sYou see traces of your quarry %s.%s\n\r") % purple() %
               dirs_to_blank[code] % norm());
      else {
        int count = code - 9, seen = 0;
        for (t = roomp->getStuff(); t; t = t->nextThing) {
          TPortal *tp = dynamic_cast<TPortal *>(t);
          if (tp) {
            seen++;
            if (count == seen) {
              sendTo(COLOR_OBJECTS, fmt("%sYou see traces of your quarry through %s.%s\n\r") % 
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
      sendTo(COLOR_MOBS, fmt("You begin tracking %s.\n\r") % specials.hunting->getName());
    }
  }

  aff.type = SKILL_TRACK;
  aff.level = getSkillLevel(SKILL_TRACK);
  aff.duration = PERMANENT_DURATION;
  affectTo(&aff);

  if (desc && desc->m_bIsClient)
    desc->clientf("%d|%d", CLIENT_TRACKING, 1 << code);

  if (code <= 9) {
    if (code >= 0 && desc && (desc->autobits & AUTO_HUNT)) {
      strcpy(buf, dirs[code]);
      addCommandToQue(buf);
    }
  } else if (desc && (desc->autobits & AUTO_HUNT) && t) {
      strcpy(buf, t->name);
      strcpy(buf, add_bars(buf).c_str());
      addToWait(combatRound(1));
      sprintf(buf2, "enter %s", buf);
      addCommandToQue(buf2);
  }

  bool isTR = affectedBySpell(SPELL_TRAIL_SEEK);

  addToWait(combatRound(1));
  addToMove((int) min(10, (-2-((110-getSkillValue((isTR ? SKILL_TRACK : SPELL_TRAIL_SEEK))))/6)));

  start_task(this, NULL, NULL, TASK_TRACKING, "", 1, in_room, 1, code+1, 40);
}
 
// used by doLook() to display next direction to go.
// return FALSE to cease tracking
int TBeing::track(TBeing *vict)
{
  int code;
  TThing *t;
  int targetRm = -1;
  int isSW = affectedBySpell(SKILL_SEEKWATER);
  char buf[256];
  char buf2[256];

  if (!vict && !isSW) {
    vlogf(LOG_BUG, "Problem in track() %s", getName());
    return TRUE;
  }
  if (roomp && !isImmortal() && 
      (roomp->getLight() + visionBonus <= 0) &&
      !roomp->isRoomFlag(ROOM_ALWAYS_LIT) &&
      !isAffected(AFF_TRUE_SIGHT)) {
    return TRUE;
  }
  if (!vict) {
    if (isSW)
      code = find_path(in_room, find_closest_water, NULL, hunt_dist, 0, &targetRm);
    else {
      vlogf(LOG_BUG, "problem in track()");
      stopTask();
      return FALSE;
    }
  } else {
    if (isImmortal())  // look through doors
      code = choose_exit_global(in_room, vict->in_room, -hunt_dist);
    else if ((GetMaxLevel() < MIN_GLOB_TRACK_LEV) ||
           affectedBySpell(SPELL_TRAIL_SEEK))
      code = choose_exit_in_zone(in_room, vict->in_room, hunt_dist);
    else
      code = choose_exit_global(in_room, vict->in_room, hunt_dist);
  }
  if ((vict && sameRoom(*vict)) ||
      (targetRm != -1 && targetRm == inRoom())) {
    sendTo(fmt("%s###You have found %s!%s\n\r") % orange() % (isSW ? "some water" :
           "your quarry") % norm());
    addToWait(combatRound(1));
    if (desc && desc->m_bIsClient)
      desc->clientf("%d", CLIENT_TRACKOFF);
    stopTask();
    addToWait(combatRound(1));
    return FALSE;
  }
  if (task && task->flags > 0 && task->flags != 100) {
    if (task->flags != (code + 1)) {
      sendTo(fmt("%s###For some reason the path you found is gone.%s\n\r") %
             purple() % norm());
      task->flags = 0;
      return TRUE;
    } else if (code <= 9) {
      sendTo(fmt("%s###You track %s %s.%s\n\r") % purple() %
             (isSW ? "some water" : "your target") % dirs_to_blank[code] % norm());
      if (desc && (desc->autobits & AUTO_HUNT)) {
        strcpy(buf, dirs[code]);
        addCommandToQue(buf);
      }
    } else {
      int count = code - 9, seen = 0;
      for (t = roomp->getStuff(); t; t = t->nextThing) {
        TPortal *tp = dynamic_cast<TPortal *>(t);
        if (tp) {
          seen++;
          if (count == seen) {
            sendTo(COLOR_OBJECTS, fmt("%sYou track %s through %s.%s\n\r") % purple() %
                   (isSW ? "some water" : "your quarry") % tp->getName() % norm());
            if (desc && (desc->autobits & AUTO_HUNT)) {
              strcpy(buf, tp->name);
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
dirTypeT TBeing::dirTrack(TBeing *vict)
{
  dirTypeT code;
  affectedData *aff;
  TThing *t;

  if (roomp && !isImmortal() && 
      (roomp->getLight() + visionBonus <= 0) &&
      !roomp->isRoomFlag(ROOM_ALWAYS_LIT) &&
      !isAffected(AFF_TRUE_SIGHT)) {
    sendTo("You can't see well enough to find a trail.\n\r");
    return DIR_NONE;
  }

  for (aff = vict->affected; aff; aff = aff->next) {
    if (aff->type == SKILL_CONCEALMENT) {
      if (::number(1,150) < aff->modifier) {
        sendTo(fmt("%s##You have lost the trail.%s\n\r") % orange() % norm());

        if (aff->be == vict) {
          act("You have successfully concealed your path from $N.",
                    FALSE, vict, 0, this, TO_CHAR, ANSI_GREEN);
          return DIR_NONE;
        } else if (aff->be && vict->sameRoom(*aff->be)) {
          act("$N has successfully concealed your path from $P.",
                    FALSE, vict, this, aff->be, TO_CHAR, ANSI_GREEN);
          act("You have successfully concealed $n's path from $P.",
                    FALSE, vict, this, aff->be, TO_VICT, ANSI_GREEN);
          return DIR_NONE;
        }
      }
    }
  }

  if ((GetMaxLevel() >= MIN_GLOB_TRACK_LEV) || affectedBySpell(SPELL_TRAIL_SEEK)
      || IS_SET(specials.act, ACT_HUNTING)) {
    code = choose_exit_global(in_room, vict->in_room, hunt_dist);
  } else
    code = choose_exit_in_zone(in_room, vict->in_room, hunt_dist);

  if (code == DIR_NONE) {
    if (sameRoom(*vict))
      sendTo(fmt("%s##You have found your target!%s\n\r") % orange() % norm());
    else
      sendTo(fmt("%s##You have lost the trail.%s\n\r") % orange() % norm());
 
    return DIR_NONE;                // false to continue the hunt 
  } else if (code < MAX_DIR) {
    sendTo(fmt("%s##You see a faint trail %s.%s\n\r") % 
         purple() % dirs_to_leading[code] % norm());
    return code;
  } else {
    int count = code - 9, seen = 0;
    for (t = roomp->getStuff(); t; t = t->nextThing) {
      TPortal *tp = dynamic_cast<TPortal*>(t);
      if (tp) {
         seen++;
        if (count == seen) {
          sendTo(COLOR_OBJECTS, fmt("%sYou see a faint trail through %s.%s\n\r") %
             purple() % tp->getName() % norm());
          break;
        }
      }
    }
    if (!t) {
      sendTo("Error finding path target!  Tell a god.\n\r");
      vlogf(LOG_BUG, "Error finding path (dirTrack)");
    }
    return code;
  }
}
 
// returns max track range that should be used for NPC's in find_path calls
int TBeing::trackRange()
{
  int range = -2000;
  return range;
}

void TBeing::doConceal(sstring argument)
{
  sstring name_buf;
  int rc;
  TBeing *vict;
 
  if (getSkillValue(SKILL_CONCEALMENT) <= 0) {
    sendTo("You do not have the ability to conceal paths.\n\r");
    return;
  }
  argument = one_argument(argument, name_buf);
 
  if (name_buf.empty()) {
    vict = this;
  } else {
    if (is_abbrev(name_buf, "off") || 
	is_abbrev(name_buf, "stop")) {
      one_argument(argument, name_buf);
      if (!name_buf.empty()) {
        vict = get_char_room_vis(this, name_buf);
        if (!vict) {
           sendTo("No such person present.\n\r");
           sendTo("Syntax: conceal off <person>\n\r");
           return;
        } else {
          affectedData *AffL;

          for(AffL = vict->affected; AffL; AffL = AffL->next)
            if (AffL->type == SKILL_CONCEALMENT)
              break;
          if (!AffL) {
            sendTo("How can you stop concealing their trail?  You never started.\n\r");
            return;
          } else if (AffL->be != this) {
            sendTo("Yes, their trail is concealed...But not by you.\n\r");
            return;
          }

          sendTo(COLOR_MOBS, fmt("You stop concealing %s's trail.\n\r") % vict->getName());
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

  return;
}
 
// return FALSE to cease tracking
int conceal(TBeing *caster, TBeing *vict)
{
  affectedData aff;
  int level = caster->getSkillLevel(SKILL_CONCEALMENT);
  int lnd = caster->getSkillValue(SKILL_CONCEALMENT);

  if (caster->getPosition() != POSITION_STANDING) {
    caster->sendTo("You need to be standing in order to conceal trails.\n\r");
    return FALSE;
  }
  if (caster->fight()) {
    caster->sendTo("The ensuing battle makes it difficult to conceal a trail.\n\r");
    return FALSE;
  }
  if (vict->fight()) {
    act("You can't conceal $N's path while $E is fighting.", 
         FALSE, caster, 0, vict, TO_CHAR);
    return FALSE;
  }
  if (vict->affectedBySpell(SKILL_CONCEALMENT)) {
    if (vict == caster)
      act("Your path is already being concealed.",
              FALSE, caster, 0, 0, TO_CHAR);
    else
      act("$N's path is already being concealed.",
              FALSE, caster, 0, vict, TO_CHAR);
    return FALSE;
  }
  if (vict != caster) {
    if (lnd <= 50) {
      act("You lack the training to conceal other people's path with any degree of success.",
         FALSE, caster, 0, 0, TO_CHAR);
      return FALSE;
    }
    // even if they can, reduce the chance of success
    lnd /= 2;
  }
  
  if (caster == vict) {  
    act("You attempt to conceal your path.",
        FALSE, caster, 0, 0, TO_CHAR);
    act("$n attempts to conceal $s path.",
        FALSE, caster, 0, 0, TO_ROOM);
  } else {
    act("You attempt to conceal $N's path.",
        FALSE, caster, 0, vict, TO_CHAR);
    act("$n attempts to conceal your path.",
        FALSE, caster, 0, vict, TO_VICT);
    act("$n attempts to conceal $N's path.",
        FALSE, caster, 0, vict, TO_NOTVICT);
  }

  aff.type = SKILL_CONCEALMENT;
  aff.duration = (level/2 + 1) * UPDATES_PER_MUDHOUR;
  aff.be = caster;

  if (bSuccess(caster, lnd, SKILL_CONCEALMENT)) {
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
