//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "info.cc" - All informative functions and routines                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <iomanip>

#include "configuration.h"
#include "handler.h"
#include "extern.h"
#include "being.h"
#include "low.h"
#include "colorstring.h"
#include "monster.h"
#include "client.h"
#include "guild.h"

#include <algorithm>
#include <sys/types.h>
#include <dirent.h>

#include "account.h"
#include "games.h"
#include "disease.h"
#include "combat.h"
#include "statistics.h"
#include "materials.h"
#include "obj_component.h"
#include "database.h"
#include "room.h"
#include "person.h"
#include "shop.h"
#include "liquids.h"
#include "spec_mobs.h"
#include "weather.h"

#include "skillsort.h"
#include "obj_open_container.h"
#include "obj_corpse.h"
#include "obj_bow.h"
#include "obj_symbol.h"
#include "obj_food.h"
#include "obj_tool.h"
#include "obj_trap.h"
#include "obj_arrow.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_base_cup.h"
#include "obj_base_clothing.h"
#include "obj_magic_item.h"
#include "obj_potion.h"
#include "obj_scroll.h"
#include "obj_staff.h"
#include "obj_wand.h"

#include <boost/algorithm/string/replace.hpp>

sstring describeDuration(const TBeing* ch, int dur) {
  char buf[160];
  sstring ret;
  int weeks = 0, days = 0, hours = 0, mins = 0;
  int errnum = 0;

  if (dur == PERMANENT_DURATION) {
    ret = "permanent";
    return ret;
  }
  // random error
  if (!ch->isImmortal()) {
#if 0
    errnum = ch->plotStat(STAT_CURRENT, STAT_PER, 175, 15, 75);
    errnum = ::number(-1 * errnum, 1 * errnum);
#else
    // bad to randomize it, just have them overestimate it
    errnum = ch->plotStat(STAT_CURRENT, STAT_PER, 400, 30, 175);
#endif
    dur *= 1000 + errnum;
    dur /= 1000;
    dur = max(1, dur);
  }

  // aff->dur decrements once per Pulse::COMBAT
  // total duration (in pulses) it lasts is dur*COMBAT

  hours = dur * Pulse::COMBAT / Pulse::MUDHOUR;
  mins = ((dur * Pulse::COMBAT) % Pulse::MUDHOUR) * 60 / Pulse::MUDHOUR;

  if (hours >= 24) {
    days = hours / 24;
    hours = hours % 24;
  }
  if (days >= 7) {
    weeks = days / 7;
    days = days % 7;
  }
  *buf = '\0';
  if (weeks)
    sprintf(buf + strlen(buf), "%d week%s, ", weeks, (weeks == 1 ? "" : "s"));
  if (days)
    sprintf(buf + strlen(buf), "%d day%s, ", days, (days == 1 ? "" : "s"));
  if (hours)
    sprintf(buf + strlen(buf), "%d hour%s, ", hours, (hours == 1 ? "" : "s"));
  if (mins)
    sprintf(buf + strlen(buf), "%d minute%s, ", mins, (mins == 1 ? "" : "s"));

  if (strlen(buf) > 0) {
    while (buf[strlen(buf) - 1] == ' ' || buf[strlen(buf) - 1] == ',')
      buf[strlen(buf) - 1] = '\0';
  }
  ret = buf;
  return ret;
}

static const sstring describe_part_wounds(const TBeing* ch, wearSlotT pos) {
  int i, flags;
  int last = 0, count = 0;
  char buf[256];

  if (ch->isLimbFlags(pos, PART_MISSING))
    return ("missing.");

  *buf = '\0';

  for (i = 0; i < MAX_PARTS; i++) {
    flags = (1 << i);
    if (ch->isLimbFlags(pos, flags)) {
      last = i;
      count++;
    }
  }
  if (count == 1 && (ch->isLimbFlags(pos, PART_TRANSFORMED)))
    return "";
  for (i = 0; i < MAX_PARTS; i++) {
    flags = (1 << i);
    if (ch->isLimbFlags(pos, flags)) {
      if (count == 1)
        sprintf(buf + strlen(buf), "%s%s%s.", ch->red(), body_flags[i],
          ch->norm());
      else if (i != last)
        sprintf(buf + strlen(buf), "%s%s%s, ", ch->red(), body_flags[i],
          ch->norm());
      else
        sprintf(buf + strlen(buf), "and %s%s%s.", ch->red(), body_flags[i],
          ch->norm());
    }
  }
  if (*buf)
    return (buf);
  else
    return ("");
}

// rp is the room looking at
// can't use roomp since spying into other room possible
void TBeing::listExits(const TRoom* rp) const {
  int num = 0, count = 0;
  dirTypeT door;
  roomDirData* exitdata;
  char buf[1024];

  const char* exDirs[] = {"N", "E", "S", "W", "U", "D", "NE", "NW", "SE", "SW"};

  *buf = '\0';

  if (desc && desc->m_bIsClient)
    return;

  // Red if closed (imm only), Blue if an open exit has a type, purple if normal

  if (isPlayerAction(PLR_BRIEF)) {
    sendTo("[Exits:");
    for (door = MIN_DIR; door < MAX_DIR; door++) {
      exitdata = rp->exitDir(door);

      if (exitdata && (exitdata->to_room != Room::NOWHERE)) {
        bool secret = IS_SET(exitdata->condition, EXIT_SECRET);
        bool open = !IS_SET(exitdata->condition, EXIT_CLOSED);
        bool see_thru = canSeeThruDoor(exitdata);

        if (isImmortal()) {
          if (IS_SET(exitdata->condition, EXIT_CLOSED))
            sendTo(format(" %s%s%s") % red() % exDirs[door] % norm());
          else if (exitdata->door_type != DOOR_NONE)
            sendTo(format(" %s%s%s") % blue() % exDirs[door] % norm());
          else
            sendTo(format(" %s%s%s") % purple() % exDirs[door] % norm());
        } else /*if (canSeeThruDoor(exitdata))*/ {
          TRoom* exitp = real_roomp(exitdata->to_room);

          if (exitp) {
            if (exitdata->door_type != DOOR_NONE &&
                ((!secret || open) || (!secret && see_thru))) {
              if (IS_SET(exitdata->condition, EXIT_CLOSED))
                sendTo(
                  format(" %s*%s%s") %
                  (exitp->getSectorType() == SECT_FIRE
                      ? red()
                      : (exitp->isAirSector()
                            ? cyan()
                            : (exitp->isWaterSector() ? blue() : purple()))) %
                  exDirs[door] % norm());
              else
                sendTo(format(" %s%s%s") %
                       ((exitp->getSectorType() == SECT_FIRE
                           ? redBold()
                           : (exitp->isAirSector()
                                 ? cyanBold()
                                 : (exitp->isWaterSector() ? blueBold()
                                                           : purpleBold())))) %
                       exDirs[door] % norm());
            } else if (exitdata->door_type == DOOR_NONE)
              sendTo(
                format(" %s%s%s") %
                (exitp->getSectorType() == SECT_FIRE
                    ? red()
                    : (exitp->isAirSector()
                          ? cyan()
                          : (exitp->isWaterSector() ? blue() : purple()))) %
                exDirs[door] % norm());
          } else
            vlogf(LOG_LOW, format("Problem with door in room %d") % inRoom());
        }
      }
    }
    sendTo(" ]\n\r");
    return;
  }

  // The following for loop is to figure out which room is the last
  // legal exit, so the word "and" can be put in front of it to make
  // the sentence sent to the player grammatically correct.
  for (door = MIN_DIR; door < MAX_DIR; door++) {
    if (!(exitdata = rp->exitDir(door)))
      continue;

    bool secret = IS_SET(exitdata->condition, EXIT_SECRET);
    bool open = !IS_SET(exitdata->condition, EXIT_CLOSED);
    bool see_thru = canSeeThruDoor(exitdata);

    if ((exitdata->to_room != Room::NOWHERE &&
          ((!secret || open) || (!secret && see_thru))) ||
        isImmortal()) {
      num = door;
      count++;
    }

    if (IS_SET(exitdata->condition, EXIT_DESTROYED)) {
      if (exitdata->keyword.empty()) {
        vlogf(LOG_LOW,
          format("Destroyed door with no name!  Room %d") % in_room);
      } else if (door == 4)
        sendTo(format("%sThe %s in the ceiling has been destroyed.%s\n\r") %
               blue() % fname(exitdata->keyword) % norm());
      else if (door == 5)
        sendTo(format("%sThe %s in the %s has been destroyed.%s\n\r") % blue() %
               fname(exitdata->keyword) % roomp->describeGround() % norm());
      else
        sendTo(format("%sThe %s %s has been destroyed.%s\n\r") % blue() %
               fname(exitdata->keyword) % dirs_to_leading[door] % norm());
    }

    if (IS_SET(exitdata->condition, EXIT_CAVED_IN)) {
      sendTo(format("%sA cave in blocks the way %s.%s\n\r") % blue() %
             dirs[door] % norm());
    }

    // chance to detect secret - bat
    // the || case is a chance at a false-positive   :)
    if ((IS_SET(exitdata->condition, EXIT_SECRET) &&
          IS_SET(exitdata->condition, EXIT_CLOSED)) ||
        (!::number(0, 100) && !isPerceptive())) {
      int chance = max(0, (int)getSkillValue(SKILL_SEARCH));

      if (getRace() == RACE_ELVEN)
        chance += 25;
      if (getRace() == RACE_GNOME)
        chance +=
          plotStat(STAT_CURRENT, STAT_PER, 3, 18, 13) + GetMaxLevel() / 2;
      if (getRace() == RACE_DWARF && rp->isIndoorSector())
        chance += GetMaxLevel() / 2 + 10;

      if ((::number(1, 1000) < chance) && !isImmortal())
        sendTo(
          format("%sYou suspect something out of the ordinary here.%s\n\r") %
          blue() % norm());
    }
  }

  for (door = MIN_DIR; door < MAX_DIR; door++) {
    if (!(exitdata = rp->exitDir(door)))
      continue;

    if (exitdata->to_room == Room::NOWHERE)
      continue;

    if (isImmortal()) {
      // Red if closed, Blue if an open exit has a type, purple if normal
      if (IS_SET(exitdata->condition, EXIT_CLOSED)) {
        if (count == 1)
          sprintf(buf + strlen(buf), "%s%s%s.\n\r", red(), dirs[door], norm());
        else if (door != num)
          sprintf(buf + strlen(buf), "%s%s%s, ", red(), dirs[door], norm());
        else
          sprintf(buf + strlen(buf), "and %s%s%s.\n\r", red(), dirs[door],
            norm());
      } else if (exitdata->door_type != DOOR_NONE) {
        if (count == 1)
          sprintf(buf + strlen(buf), "%s%s%s.\n\r", blue(), dirs[door], norm());
        else if (door != num)
          sprintf(buf + strlen(buf), "%s%s%s, ", blue(), dirs[door], norm());
        else
          sprintf(buf + strlen(buf), "and %s%s%s.\n\r", blue(), dirs[door],
            norm());
      } else {
        if (count == 1)
          sprintf(buf + strlen(buf), "%s%s%s.\n\r", purple(), dirs[door],
            norm());
        else if (door != num)
          sprintf(buf + strlen(buf), "%s%s%s, ", purple(), dirs[door], norm());
        else
          sprintf(buf + strlen(buf), "and %s%s%s.\n\r", purple(), dirs[door],
            norm());
      }
    } else {
      TRoom* exitp = real_roomp(exitdata->to_room);

      if (exitp) {
        bool secret = IS_SET(exitdata->condition, EXIT_SECRET);
        bool open = !IS_SET(exitdata->condition, EXIT_CLOSED);
        bool see_thru = canSeeThruDoor(exitdata);

        if (exitdata->door_type != DOOR_NONE &&
            ((!secret || open) || (!secret && see_thru))) {
          if (IS_SET(exitdata->condition, EXIT_CLOSED)) {
            sprintf(buf + strlen(buf), "%s%s*%s%s%s",
              ((count != 1 && door == num) ? "and " : ""),

              (exitp->getSectorType() == SECT_FIRE
                  ? red()
                  : (exitp->isAirSector()
                        ? cyan()
                        : (exitp->isWaterSector() ? blue() : purple()))),

              dirs[door],

              norm(),

              (count == 1 || door == num ? ".\n\r" : ", "));
          } else
            sprintf(buf + strlen(buf), "%s%s%s%s%s",
              ((count != 1 && door == num) ? "and " : ""),
              (exitp->getSectorType() == SECT_FIRE
                  ? redBold()
                  : (exitp->isAirSector()
                        ? cyanBold()
                        : (exitp->isWaterSector() ? blueBold()
                                                  : purpleBold()))),
              dirs[door], norm(), (count == 1 || door == num ? ".\n\r" : ", "));
        } else if (exitdata->door_type == DOOR_NONE) {
          sprintf(buf + strlen(buf), "%s%s%s%s%s",
            ((count != 1 && door == num) ? "and " : ""),
            (exitp->getSectorType() == SECT_FIRE
                ? red()
                : (exitp->isAirSector()
                      ? cyan()
                      : (exitp->isWaterSector() ? blue() : purple()))),
            dirs[door], norm(), (count == 1 || door == num ? ".\n\r" : ", "));
        }
      } else
        vlogf(LOG_LOW, format("Problem with door in room %d") % inRoom());
    }
  }

  if (*buf) {
    if (count == 1)
      sendTo(format("You see an exit %s") % buf);
    else
      sendTo(format("You can see exits to the %s") % buf);
  } else
    sendTo("You see no obvious exits.\n\r");
}

void list_char_in_room(StuffList list, TBeing* ch) {
  TThing *i, *cond_ptr[50];
  int k, cond_top;
  unsigned int cond_tot[50];
  bool found = FALSE;

  cond_top = 0;

  for (StuffIter it = list.begin(); it != list.end(); ++it) {
    i = *it;
    if (dynamic_cast<TBeing*>(i) && (ch != i) && (!i->rider) &&
        (ch->isAffected(AFF_SENSE_LIFE) || ch->isAffected(AFF_INFRAVISION) ||
          (ch->canSee(i)))) {
      if ((cond_top < 50) && !i->riding) {
        found = FALSE;
        if (dynamic_cast<TMonster*>(i)) {
          for (k = 0; (k < cond_top && !found); k++) {
            if (cond_top > 0) {
              if (i->isSimilar(cond_ptr[k])) {
                cond_tot[k] += 1;
                found = TRUE;
              }
            }
          }
        }
        if (!found) {
          cond_ptr[cond_top] = i;
          cond_tot[cond_top] = 1;
          cond_top += 1;
        }
      } else
        ch->showTo(i, SHOW_MODE_DESC_PLUS);
    }
  }
  if (cond_top) {
    for (k = 0; k < cond_top; k++) {
      if (cond_tot[k] > 1)
        ch->showMultTo(cond_ptr[k], SHOW_MODE_DESC_PLUS, cond_tot[k]);
      else
        ch->showTo(cond_ptr[k], SHOW_MODE_DESC_PLUS);
    }
  }
}

bool wordHasPunctuation(const sstring& s) {
  sstring t, punctuation = ".!?;:";
  size_t last_char = 0;

  t = s;

  // check if the stripped word ends with punctuation
  t += " ";
  last_char = t.find_last_not_of(" ");

  return (t.find_first_of(punctuation, last_char) != t.npos);
}

sstring TBeing::autoFormatDesc(const sstring& regStr, bool indent) {
  sstring line, garbled;

  sstring newDescr = "";
  size_t swlen = 0, swlen_diff = 0, llen_diff = 0;
  bool was_word = false;
  bool sent_end = false;

  if (regStr.empty()) {
    return newDescr;
  }

  if (isImmortal()) {
    garbled = regStr;
  } else {
    garbled = garble(NULL, regStr, Garble::SPEECH_ROOMDESC, Garble::SCOPE_SELF);
  }

  if ((garbled.find("   ")) != sstring::npos) {
    return garbled.toCRLF();
  }

  // indent the first line, if needed
  if (indent) {
    line = "  ";  // intial extra space
  }

  int i = 0;
  while (true) {
    sstring raw_word = garbled.word(i++);

    if (raw_word.empty()) {
      // complete the last line
      line += "\n\r";
      newDescr += line;
      break;
    }

    // count the number of unprintable characters in each word
    sstring stripped_word = stripColorCodes(raw_word);
    swlen = stripped_word.length();
    swlen_diff = raw_word.length() - swlen;

    // add the length difference to the total difference for the current line
    llen_diff += swlen_diff;

    // if the word is just a color code, just append it on the current line
    if (!swlen) {
      line += raw_word;
      was_word = false;
    } else {
      // if the word is too long to fit on the current line
      if ((line.length() + 1 + (sent_end ? 1 : 0) + raw_word.length()) >
          (79 + llen_diff)) {
        // terminate this line
        line += "\n\r";
        newDescr += line;

        // then start a new line
        line = raw_word;
        // we just started a new line, so reset the line length difference
        // to that of the word minus stripped word
        llen_diff = swlen_diff;

        // word fits ok on the current line
      } else {
        // add one extra space to the ends of sentences
        if (sent_end) {
          line += " ";
        }

        // and the previous word was a real word, then append a space to the
        // line before appending the word.
        if (was_word) {
          line += " ";
        }
        line += raw_word;
      }
      was_word = true;

      // check if the stripped word ends with punctuation
      // and set the flag accordingly
      if (wordHasPunctuation(stripped_word)) {
        sent_end = true;
      } else {
        sent_end = false;
      }
    }
  }

  return newDescr;
}

sstring TBeing::dynColorRoom(TRoom* rp, int title, bool) const {
  //  if (rp && title && full) {
  //  }

  int len, letter;
  sstring argument, buf2 = "   ", buf3 = "   ";

  if (title == 1) {
    if (!rp->getName().empty()) {
      argument = rp->getName();
      if (argument[0] == '<') {
        buf3[0] = argument[0];
        buf3[1] = argument[1];
        buf3[2] = argument[2];
        buf2 = buf3;
      } else {
        buf2 = addColorRoom(rp, 1);
      }
    } else {
      vlogf(LOG_BUG, format("%s is in a room with no descr") % getName());
      return "Bogus Name";
    }
  } else if (title == 2) {
    if (!rp->getDescr().empty()) {
      argument = rp->getDescr();
      if (argument[0] == '<') {
        buf2[0] = argument[0];
        buf2[1] = argument[1];
        buf2[2] = argument[2];
      } else {
        buf2 = addColorRoom(rp, 2);
      }
    } else {
      vlogf(LOG_BUG, format("%s is in a room with no descr") % getName());
      return "Bogus Name";
    }
  } else {
    vlogf(LOG_BUG,
      format("%s called a function with a bad dynColorRoom argument") %
        getName());
    return "Something Bogus, tell a god";
  }
  // Found had to initialize with this logic and too tired to figure out why

  sstring buf = "";
  if (!buf2.empty()) {
    buf = buf2;
  }

  len = argument.length();
  for (letter = 0; letter < len; letter++) {
    if (letter < 2) {
      buf += argument[letter];
      continue;
    }
    if ((argument[letter] == '>') && (argument[letter - 2] == '<')) {
      switch (argument[(letter - 1)]) {
        case '1':
        case 'z':
        case 'Z':
          buf += argument[letter];
          if (!buf2.empty()) {
            buf += buf2;
          }
          break;
        default:
          buf += argument[letter];
          break;
      }
    } else {
      buf += argument[letter];
    }
  }
  buf += "<1>";
  return buf;
}

// Peel
sstring TRoom::daynightColorRoom() const {
  if (IS_SET(roomFlags, ROOM_INDOORS))
    return ("<z>");

  switch (Weather::getSunlight()) {
    case Weather::SUN_DAWN:
    case Weather::SUN_RISE:
      return ("<w>");
      break;
    case Weather::SUN_LIGHT:
      return ("<z>");
      break;
    case Weather::SUN_SET:
    case Weather::SUN_TWILIGHT:
      return ("<w>");
      break;
    case Weather::SUN_DARK:
      return ("<k>");
      break;
  }

  return ("<z>");
}

const sstring getSectorDescrColor(sectorTypeT sector, TRoom* rp) {
  sstring buf3 = "<z>";

  switch (sector) {
    case SECT_SUBARCTIC:
      buf3 = "<p>";
      break;
    case SECT_ARCTIC_WASTE:
      buf3 = "<W>";
      break;
    case SECT_ARCTIC_CITY:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_ARCTIC_ROAD:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_TUNDRA:
      buf3 = "<p>";
      break;
    case SECT_ARCTIC_MOUNTAINS:
      buf3 = "<W>";
      break;
    case SECT_ARCTIC_FOREST:
      buf3 = "<W>";
      break;
    case SECT_ARCTIC_MARSH:
      buf3 = "<p>";
      break;
    case SECT_ARCTIC_RIVER_SURFACE:
      buf3 = "<c>";
      break;
    case SECT_ICEFLOW:
      buf3 = "<W>";
      break;
    case SECT_COLD_BEACH:
      buf3 = "<P>";
      break;
    case SECT_SOLID_ICE:
      buf3 = "<C>";
      break;
    case SECT_ARCTIC_BUILDING:
      break;
    case SECT_ARCTIC_CAVE:
      buf3 = "<k>";
      break;
    case SECT_ARCTIC_ATMOSPHERE:
      buf3 = "<C>";
      break;
    case SECT_ARCTIC_CLIMBING:
    case SECT_ARCTIC_FOREST_ROAD:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_PLAINS:
      buf3 = "<g>";
      break;
    case SECT_TEMPERATE_CITY:
    case SECT_TEMPERATE_ROAD:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_GRASSLANDS:
      buf3 = "<g>";
      break;
    case SECT_TEMPERATE_HILLS:
      buf3 = "<g>";
      break;
    case SECT_TEMPERATE_MOUNTAINS:
      buf3 = "<o>";
      break;
    case SECT_TEMPERATE_FOREST:
      buf3 = "<g>";
      break;
    case SECT_TEMPERATE_SWAMP:
      buf3 = "<p>";
      break;
    case SECT_TEMPERATE_OCEAN:
      buf3 = "<c>";
      break;
    case SECT_TEMPERATE_RIVER_SURFACE:
      buf3 = "<b>";
      break;
    case SECT_TEMPERATE_UNDERWATER:
      buf3 = "<b>";
      break;
    case SECT_TEMPERATE_CAVE:
      buf3 = "<k>";
      break;
    case SECT_TEMPERATE_ATMOSPHERE:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_TEMPERATE_CLIMBING:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_TEMPERATE_FOREST_ROAD:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_DESERT:
    case SECT_SAVANNAH:
      buf3 = "<o>";
      break;
    case SECT_VELDT:
      buf3 = "<o>";
      break;
    case SECT_TROPICAL_CITY:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_TROPICAL_ROAD:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_JUNGLE:
      buf3 = "<g>";
      break;
    case SECT_RAINFOREST:
      buf3 = "<g>";
      break;
    case SECT_TROPICAL_HILLS:
      buf3 = "<g>";
      break;
    case SECT_TROPICAL_MOUNTAINS:
      buf3 = "<p>";
      break;
    case SECT_VOLCANO_LAVA:
      buf3 = "<R>";
      break;
    case SECT_TROPICAL_SWAMP:
      buf3 = "<g>";
      break;
    case SECT_TROPICAL_OCEAN:
      buf3 = "<c>";
      break;
    case SECT_TROPICAL_RIVER_SURFACE:
      buf3 = "<B>";
      break;
    case SECT_TROPICAL_UNDERWATER:
      buf3 = "<b>";
      break;
    case SECT_TROPICAL_BEACH:
      buf3 = "<y>";
      break;
    case SECT_TROPICAL_BUILDING:
      break;
    case SECT_TROPICAL_CAVE:
      buf3 = "<k>";
      break;
    case SECT_TROPICAL_ATMOSPHERE:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_TROPICAL_CLIMBING:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_RAINFOREST_ROAD:
      if (rp)
        buf3 = rp->daynightColorRoom();
      break;
    case SECT_ASTRAL_ETHREAL:
      buf3 = "<c>";
      break;
    case SECT_SOLID_ROCK:
      buf3 = "<w>";
      break;
    case SECT_FIRE:
      buf3 = "<R>";
      break;
    case SECT_INSIDE_MOB:
      buf3 = "<r>";
      break;
    case SECT_FIRE_ATMOSPHERE:
      buf3 = "<R>";
      break;
    case SECT_DEAD_WOODS:
      buf3 = "<k>";
      break;
    case SECT_TEMPERATE_BEACH:
    case SECT_TEMPERATE_BUILDING:
    case SECT_MAKE_FLY:
    case MAX_SECTOR_TYPES:
      break;
  }

  return buf3;
}

const sstring getSectorNameColor(sectorTypeT sector, TRoom* rp) {
  sstring buf2;

  switch (sector) {
    case SECT_SUBARCTIC:
      buf2 = "<P>";
      break;
    case SECT_ARCTIC_WASTE:
      buf2 = "<w>";
      break;
    case SECT_ARCTIC_CITY:
      buf2 = "<C>";
      break;
    case SECT_ARCTIC_ROAD:
      buf2 = "<W>";
      break;
    case SECT_TUNDRA:
      buf2 = "<o>";
      break;
    case SECT_ARCTIC_MOUNTAINS:
      buf2 = "<o>";
      break;
    case SECT_ARCTIC_FOREST:
      buf2 = "<G>";
      break;
    case SECT_ARCTIC_MARSH:
      buf2 = "<B>";
      break;
    case SECT_ARCTIC_RIVER_SURFACE:
      buf2 = "<C>";
      break;
    case SECT_ICEFLOW:
      buf2 = "<C>";
      break;
    case SECT_COLD_BEACH:
      buf2 = "<p>";
      break;
    case SECT_SOLID_ICE:
      buf2 = "<c>";
      break;
    case SECT_ARCTIC_BUILDING:
      buf2 = "<p>";
      break;
    case SECT_ARCTIC_CAVE:
      buf2 = "<c>";
      break;
    case SECT_ARCTIC_ATMOSPHERE:
      buf2 = "<C>";
      break;
    case SECT_ARCTIC_CLIMBING:
    case SECT_ARCTIC_FOREST_ROAD:
      buf2 = "<p>";
      break;
    case SECT_PLAINS:
      buf2 = "<G>";
      break;
    case SECT_TEMPERATE_CITY:
    case SECT_TEMPERATE_ROAD:
      buf2 = "<p>";
      break;
    case SECT_GRASSLANDS:
      buf2 = "<G>";
      break;
    case SECT_TEMPERATE_HILLS:
      buf2 = "<o>";
      break;
    case SECT_TEMPERATE_MOUNTAINS:
      buf2 = "<G>";
      break;
    case SECT_TEMPERATE_FOREST:
      buf2 = "<G>";
      break;
    case SECT_TEMPERATE_SWAMP:
      buf2 = "<P>";
      break;
    case SECT_TEMPERATE_OCEAN:
      buf2 = "<C>";
      break;
    case SECT_TEMPERATE_RIVER_SURFACE:
      buf2 = "<B>";
      break;
    case SECT_TEMPERATE_UNDERWATER:
      buf2 = "<C>";
      break;
    case SECT_TEMPERATE_CAVE:
      buf2 = "<o>";
      break;
    case SECT_TEMPERATE_ATMOSPHERE:
      buf2 = "<G>";
      break;
    case SECT_TEMPERATE_CLIMBING:
      buf2 = "<G>";
      break;
    case SECT_TEMPERATE_FOREST_ROAD:
      buf2 = "<g>";
      break;
    case SECT_DESERT:
    case SECT_SAVANNAH:
      buf2 = "<y>";
      break;
    case SECT_VELDT:
      buf2 = "<g>";
      break;
    case SECT_TROPICAL_CITY:
      buf2 = "<G>";
      break;
    case SECT_TROPICAL_ROAD:
      buf2 = "<g>";
      break;
    case SECT_JUNGLE:
      buf2 = "<P>";
      break;
    case SECT_RAINFOREST:
      buf2 = "<G>";
      break;
    case SECT_TROPICAL_HILLS:
      buf2 = "<R>";
      break;
    case SECT_TROPICAL_MOUNTAINS:
      buf2 = "<P>";
      break;
    case SECT_VOLCANO_LAVA:
      buf2 = "<y>";
      break;
    case SECT_TROPICAL_SWAMP:
      buf2 = "<G>";
      break;
    case SECT_TROPICAL_OCEAN:
      buf2 = "<b>";
      break;
    case SECT_TROPICAL_RIVER_SURFACE:
      buf2 = "<C>";
      break;
    case SECT_TROPICAL_UNDERWATER:
      buf2 = "<B>";
      break;
    case SECT_TROPICAL_BEACH:
      buf2 = "<P>";
      break;
    case SECT_TROPICAL_BUILDING:
      buf2 = "<p>";
      break;
    case SECT_TROPICAL_CAVE:
      buf2 = "<P>";
      break;
    case SECT_TROPICAL_ATMOSPHERE:
      buf2 = "<P>";
      break;
    case SECT_TROPICAL_CLIMBING:
      buf2 = "<P>";
      break;
    case SECT_RAINFOREST_ROAD:
      buf2 = "<P>";
      break;
    case SECT_ASTRAL_ETHREAL:
      buf2 = "<C>";
      break;
    case SECT_SOLID_ROCK:
      buf2 = "<k>";
      break;
    case SECT_FIRE:
      buf2 = "<y>";
      break;
    case SECT_INSIDE_MOB:
      buf2 = "<R>";
      break;
    case SECT_FIRE_ATMOSPHERE:
      buf2 = "<y>";
      break;
    case SECT_DEAD_WOODS:
      buf2 = "<k>";
      break;
    case SECT_TEMPERATE_BEACH:
    case SECT_TEMPERATE_BUILDING:
    case SECT_MAKE_FLY:
    case MAX_SECTOR_TYPES:
      buf2 = "<p>";
      break;
  }

  return buf2;
}

const sstring TBeing::addColorRoom(TRoom* rp, int title) const {
  sstring buf2, buf3;

  // Found had to initialize with this logic and too tired to figure out why
  buf3 = "<z>";

  sectorTypeT sector = rp->getSectorType();

  buf2 = getSectorNameColor(sector, rp);
  buf3 = getSectorDescrColor(sector, rp);

  if (title == 1) {
    if (!rp->getName().empty()) {
      return buf2;
    } else {
      vlogf(LOG_BUG, "room without a name for dynamic coloring");
      return "";
    }
  } else if (title == 2) {
    if (!rp->getDescr().empty())
      return buf3;
    else {
      vlogf(LOG_BUG, format("room without a descr for dynamic coloring, %s") %
                       roomp->getName());
      return "";
    }
  } else {
    vlogf(LOG_BUG, "addColorRoom without a correct title variable");
    return "";
  }
}

void TBeing::doRead(const char* argument) {
  sstring buf;

  // This is just for now - To be changed later!
  buf = "at ";
  buf += argument;
  doLook(buf, CMD_READ);
}

void TBaseCup::examineObj(TBeing* ch) const {
  int bits = FALSE;

  if (parent && (ch == parent)) {
    bits = FIND_OBJ_INV;
  } else if (equippedBy && (ch == equippedBy)) {
    bits = FIND_OBJ_EQUIP;
  } else if (parent && (ch->roomp == parent)) {
    bits = FIND_OBJ_ROOM;
  }

  ch->sendTo("When you look inside, you see:\n\r");
  lookObj(ch, bits);
}

void TBeing::doExamine(const char* argument, TThing* specific) {
  sstring buf;
  char caName[100];
  int bits;
  TBeing* tmp = NULL;
  TObj* o = NULL;

  if (specific) {
    doLook("", CMD_LOOK, specific);
    return;
  }

  one_argument(argument, caName, cElements(caName));

  if (!*caName) {
    sendTo("Examine what?\n\r");
    return;
  }
  bits = generic_find(caName,
    FIND_ROOM_EXTRA | FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
      FIND_CHAR_ROOM,
    this, &tmp, &o);

  if (bits) {
    buf = "at ";
    buf += argument;
    doLook(buf, CMD_LOOK);
  }
  if (o)
    o->examineObj(this);

  if (!bits && !o)
    sendTo("Examine what?\n\r");
}

// affect is on ch, this is person looking
sstring TBeing::describeAffects(TBeing* ch, showMeT showme) const {
  affectedData *aff, *af2;
  sstring str;
  int objused;

  // limit what others can see.  Magic should reveal truth, but in general
  // keep some stuff concealed
  bool show = (ch == this) | showme;

  for (aff = ch->affected; aff; aff = af2) {
    af2 = aff->next;

    switch (aff->type) {
      case SKILL_TRACK:
      case SKILL_SEEKWATER:
        str += format("Tracking: %s\n\r") %
               (aff->type == SKILL_TRACK ? ch->specials.hunting->getName()
                                         : "seeking water");
        break;
      case SPELL_GUST:
      case SPELL_DUST_STORM:
      case SPELL_TORNADO:
      case SKILL_QUIV_PALM:
      case SKILL_SHOULDER_THROW:
      case SPELL_CALL_LIGHTNING:
      case SPELL_LIGHTNING_BREATH:
      case SPELL_GUSHER:
      case SPELL_AQUATIC_BLAST:
      case SPELL_ICY_GRIP:
      case SPELL_ARCTIC_BLAST:
      case SPELL_ICE_STORM:
      case SPELL_FROST_BREATH:
      case SPELL_WATERY_GRAVE:
      case SPELL_TSUNAMI:
      case SPELL_CHLORINE_BREATH:
      case SPELL_DUST_BREATH:
      case SPELL_POISON_DEIKHAN:
      case SPELL_POISON:
      case SPELL_ACID_BREATH:
      case SPELL_ACID_BLAST:
      case SKILL_BODYSLAM:
      case SKILL_SPIN:
      case SKILL_ADVANCED_BERSERKING:
      case SKILL_BLOODLUST:
      case SKILL_WHIRLWIND:
      case SKILL_RALLY:
      case SKILL_FORTIFY:
      case SKILL_FOCUS_ATTACK:
      case SKILL_POWERMOVE:
      case SKILL_CHARGE:
      case SKILL_SMITE:
      case SPELL_METEOR_SWARM:
      case SPELL_EARTHQUAKE:
      case SPELL_PILLAR_SALT:
      case SPELL_FIREBALL:
      case SPELL_HANDS_OF_FLAME:
      case SPELL_LAVA_LANCE:
      case SPELL_INFERNO:
      case SPELL_HELLFIRE:
      case SPELL_RAIN_BRIMSTONE:
      case SPELL_FLAMESTRIKE:
      case SPELL_FIRE_BREATH:
      case SPELL_SPONTANEOUS_COMBUST:
      case SPELL_FLAMING_SWORD:
      case SPELL_FLARE:
      case SPELL_MYSTIC_DARTS:
      case SPELL_STUNNING_ARROW:
      case SPELL_COLOR_SPRAY:
      case SPELL_SAND_BLAST:
      case SPELL_PEBBLE_SPRAY:
      case SPELL_LAVA_STREAM:
      case SPELL_DEATH_MIST:
      case SPELL_FLATULENCE:
      case SPELL_SLING_SHOT:
      case SPELL_GRANITE_FISTS:
      case SPELL_ENERGY_DRAIN:
      case SPELL_SOUL_TWIST:
      case SPELL_DEATHWAVE:
      case SPELL_DISTORT:         // shaman
      case SPELL_SQUISH:          // shaman
      case SPELL_LICH_TOUCH:      // shaman
      case SPELL_CARDIAC_STRESS:  // shaman
      case SPELL_SYNOSTODWEOMER:
      case SKILL_DIVINE_GRACE:
      case SKILL_DIVINE_RESCUE:
      case SKILL_GUARDIANS_LIGHT:
      case SPELL_HARM_DEIKHAN:
      case SPELL_HARM:
      case SPELL_HARM_LIGHT_DEIKHAN:
      case SPELL_HARM_SERIOUS_DEIKHAN:
      case SPELL_HARM_CRITICAL_DEIKHAN:
      case SPELL_HARM_LIGHT:
      case SPELL_HARM_SERIOUS:
      case SPELL_HARM_CRITICAL:
      case SPELL_WITHER_LIMB:
      case SPELL_BLEED:
      case SKILL_KICK:
      case SKILL_KICK_MONK:
      case SKILL_SLAM:
      case SKILL_SPRINGLEAP:
      case SKILL_DEATHSTROKE:
      case SKILL_BASH_DEIKHAN:
      case SKILL_BASH:
      case SPELL_BONE_BREAKER:
      case SPELL_PARALYZE:
      case SPELL_PARALYZE_LIMB:
      case SPELL_INFECT_DEIKHAN:
      case SPELL_INFECT:
      case SKILL_CHOP:
      case SPELL_DISEASE:
      case SPELL_SUFFOCATE:
      case SKILL_GARROTTE:
      case SKILL_STABBING:
      case SKILL_BACKSTAB:
      case SKILL_THROATSLIT:
      case SKILL_HEADBUTT:
      case SKILL_BRAWL_AVOIDANCE:
      case SKILL_STOMP:
      case SPELL_BLAST_OF_FURY:
      case SKILL_CHI:
      case SKILL_TRIP:
      case SPELL_FUMBLE:
      case SPELL_BLINDNESS:
      case SPELL_GARMULS_TAIL:
      case SPELL_SORCERERS_GLOBE:
      case SPELL_FAERIE_FIRE:
      case SPELL_STUPIDITY:
      case SPELL_ILLUMINATE:
      case SPELL_DETECT_MAGIC:
      case SPELL_MATERIALIZE:
      case SPELL_CHRISM:
      case SPELL_BLOOD_BOIL:
      case SPELL_DJALLA:
      case SPELL_LEGBA:
      case SPELL_PROTECTION_FROM_EARTH:
      case SPELL_PROTECTION_FROM_AIR:
      case SPELL_PROTECTION_FROM_FIRE:
      case SPELL_PROTECTION_FROM_WATER:
      case SPELL_PROTECTION_FROM_ELEMENTS:
      case SPELL_INFRAVISION:
      case SPELL_IDENTIFY:
      case SPELL_POWERSTONE:
      case SPELL_FAERIE_FOG:
      case SPELL_TELEPORT:
      case SPELL_KNOT:
      case SPELL_SENSE_LIFE:
      case SPELL_SENSE_LIFE_SHAMAN:  // shaman
      case SPELL_CALM:
      case SPELL_ACCELERATE:
      case SPELL_CELERITE:
      case SPELL_CHEVAL:  // shaman
      case SPELL_LEVITATE:
      case SPELL_FEATHERY_DESCENT:
      case SPELL_STEALTH:
      case SPELL_GILLS_OF_FLESH:
      case SPELL_AQUALUNG:
      case SPELL_TELEPATHY:
      case SPELL_ROMBLER:        // shaman
      case SPELL_DETECT_SHADOW:  // shaman
      case SPELL_FEAR:
      case SPELL_INTIMIDATE:  // shaman
      case SPELL_SLUMBER:
      case SPELL_CONJURE_EARTH:
      case SPELL_ENTHRALL_SPECTRE:
      case SPELL_ENTHRALL_GHAST:
      case SPELL_ENTHRALL_GHOUL:
      case SPELL_ENTHRALL_DEMON:
      case SPELL_CREATE_WOOD_GOLEM:
      case SPELL_CREATE_ROCK_GOLEM:
      case SPELL_CREATE_IRON_GOLEM:
      case SPELL_CREATE_DIAMOND_GOLEM:
      case SPELL_CONJURE_AIR:
      case SPELL_CONJURE_FIRE:
      case SPELL_CONJURE_WATER:
      case SPELL_DISPEL_MAGIC:
      case SPELL_CHASE_SPIRIT:  // shaman
      case SPELL_ENHANCE_WEAPON:
      case SPELL_GALVANIZE:
      case SPELL_DETECT_INVISIBLE:
      case SPELL_DISPEL_INVISIBLE:
      case SPELL_FARLOOK:
      case SPELL_FALCON_WINGS:
      case SPELL_INVISIBILITY:
      case SPELL_ENSORCER:
      case SPELL_EYES_OF_FERTUMAN:
      case SPELL_COPY:
      case SPELL_HASTE:
      case SPELL_IMMOBILIZE:
      case SPELL_FLY:
      case SPELL_ANTIGRAVITY:
      case SPELL_DIVINATION:
      case SPELL_SHATTER:
      case SKILL_SCRIBE:
      case SPELL_SPONTANEOUS_GENERATION:
      case SPELL_STONE_SKIN:
      case SPELL_TRAIL_SEEK:
      case SPELL_FLAMING_FLESH:
      case SPELL_ATOMIZE:
      case SPELL_ANIMATE:
      case SPELL_BIND:
      case SPELL_TRUE_SIGHT:
      case SPELL_CLOUD_OF_CONCEALMENT:
      case SPELL_POLYMORPH:
      case SPELL_SILENCE:
      case SPELL_BREATH_OF_SARAHAGE:
      case SPELL_PLASMA_MIRROR:
      case SPELL_THORNFLESH:
      case SPELL_ETHER_GATE:
      case SPELL_HEAL_LIGHT:
      case SPELL_HEALING_GRASP:
      case SPELL_CREATE_FOOD:
      case SPELL_CREATE_WATER:
      case SPELL_ARMOR:
      case SPELL_BLESS:
      case SPELL_CLOT:
      case SPELL_HEAL_SERIOUS:
      case SPELL_STERILIZE:
      case SPELL_EXPEL:
      case SPELL_CURE_DISEASE:
      case SPELL_CURSE:
      case SPELL_REMOVE_CURSE:
      case SPELL_CURE_POISON:
      case SPELL_CLEANSE:
      case SPELL_HEAL_CRITICAL:
      case SPELL_SALVE:
      case SPELL_REFRESH:
      case SPELL_NUMB:
      case SPELL_PLAGUE_LOCUSTS:
      case SPELL_CURE_BLINDNESS:
      case SPELL_SUMMON:
      case SPELL_HEAL:
      case SPELL_WORD_OF_RECALL:
      case SPELL_SANCTUARY:
      case SPELL_RELIVE:
      case SPELL_CRUSADE:
      case SPELL_CURE_PARALYSIS:
      case SPELL_SECOND_WIND:
      case SPELL_HEROES_FEAST:
      case SPELL_ASTRAL_WALK:
      case SPELL_PORTAL:
      case SPELL_HEAL_FULL:
      case SPELL_HEAL_CRITICAL_SPRAY:
      case SPELL_HEAL_SPRAY:
      case SPELL_HEAL_FULL_SPRAY:
      case SPELL_RESTORE_LIMB:
      case SPELL_KNIT_BONE:
      case SPELL_CLARITY:
      case SPELL_HYPNOSIS:
      case SPELL_SHADOW_WALK:
      case SPELL_SHIELD_OF_MISTS:
      case SPELL_ENLIVEN:
      case SKILL_RESCUE:
      case SKILL_BLACKSMITHING:
      case SKILL_REPAIR_MAGE:
      case SKILL_REPAIR_MONK:
      case SKILL_REPAIR_CLERIC:
      case SKILL_REPAIR_DEIKHAN:
      case SKILL_REPAIR_SHAMAN:
      case SKILL_REPAIR_THIEF:
      case SKILL_BLACKSMITHING_ADVANCED:
      case SKILL_MEND:
      case SPELL_RAZE:
      case SKILL_SACRIFICE:
      case SKILL_DISARM:
      case SKILL_PARRY_WARRIOR:
      case SKILL_RIPOSTE:
      case SKILL_DUAL_WIELD:
      case SKILL_BERSERK:
      case SKILL_SWITCH_OPP:
      case SKILL_KNEESTRIKE:
      case SKILL_SHOVE:
      case SKILL_RETREAT:
      case SKILL_GRAPPLE:
      case SKILL_DOORBASH:
      case SKILL_TRANCE_OF_BLADES:
      case SKILL_WEAPON_RETENTION:
      case SKILL_CLOSE_QUARTERS_FIGHTING:
      case SKILL_HIKING:
      case SKILL_FORAGE:
      case SKILL_TRANSFORM_LIMB:
      case SKILL_BEAST_SOOTHER:
      case SPELL_ROOT_CONTROL:
      case SKILL_BEFRIEND_BEAST:
      case SKILL_TRANSFIX:
      case SKILL_SKIN:
      case SKILL_BUTCHER:
      case SPELL_LIVING_VINES:
      case SKILL_BEAST_SUMMON:
      case SKILL_BARKSKIN:
      case SPELL_STICKS_TO_SNAKES:
      case SPELL_STORMY_SKIES:
      case SPELL_TREE_WALK:
      case SKILL_BEAST_CHARM:
      case SPELL_SHAPESHIFT:
      case SKILL_CONCEALMENT:
      case SKILL_APPLY_HERBS:
      case SKILL_DIVINATION:
      case SKILL_ENCAMP:
      case SKILL_FISHLORE:
      case SPELL_HEAL_LIGHT_DEIKHAN:
      case SKILL_CHIVALRY:
      case SPELL_ARMOR_DEIKHAN:
      case SPELL_BLESS_DEIKHAN:
      case SPELL_EXPEL_DEIKHAN:
      case SPELL_CLOT_DEIKHAN:
      case SPELL_STERILIZE_DEIKHAN:
      case SPELL_REMOVE_CURSE_DEIKHAN:
      case SPELL_CURSE_DEIKHAN:
      case SKILL_RESCUE_DEIKHAN:
      case SKILL_TAUNT:
      case SPELL_CURE_DISEASE_DEIKHAN:
      case SPELL_CREATE_FOOD_DEIKHAN:
      case SPELL_HEAL_SERIOUS_DEIKHAN:
      case SPELL_CURE_POISON_DEIKHAN:
      case SKILL_DISARM_DEIKHAN:
      case SPELL_HEAL_CRITICAL_DEIKHAN:
      case SKILL_SWITCH_DEIKHAN:
      case SKILL_RETREAT_DEIKHAN:
      case SKILL_SHOVE_DEIKHAN:
      case SKILL_SHOCK_CAVALRY:
      case SKILL_ORIENT:
      case SKILL_2H_SPEC_DEIKHAN:
      case SKILL_2H_SPEC:
      case SKILL_RIDE:
      case SKILL_CALM_MOUNT:
      case SKILL_TRAIN_MOUNT:
      case SKILL_ADVANCED_RIDING:
      case SKILL_RIDE_DOMESTIC:
      case SKILL_RIDE_NONDOMESTIC:
      case SKILL_RIDE_WINGED:
      case SPELL_CREATE_WATER_DEIKHAN:
      case SKILL_RIDE_EXOTIC:
      case SPELL_HEROES_FEAST_DEIKHAN:
      case SPELL_REFRESH_DEIKHAN:
      case SPELL_SALVE_DEIKHAN:
      case SKILL_LAY_HANDS:
      case SPELL_NUMB_DEIKHAN:
      case SKILL_YOGINSA:
      case SKILL_CINTAI:
      case SKILL_OOMLAT:
      case SKILL_ADVANCED_KICKING:
      case SKILL_DISARM_MONK:
      case SKILL_GROUNDFIGHTING:
      case SKILL_DUFALI:
      case SKILL_RETREAT_MONK:
      case SKILL_SNOFALTE:
      case SKILL_COUNTER_MOVE:
      case SKILL_SWITCH_MONK:
      case SKILL_JIRIN:
      case SKILL_KUBO:
      case SKILL_CATFALL:
      case SKILL_CATLEAP:
      case SKILL_WOHLIN:
      case SKILL_VOPLAT:
      case SKILL_BLINDFIGHTING:
      case SKILL_CRIT_HIT:
      case SKILL_FEIGN_DEATH:
      case SKILL_BLUR:
      case SKILL_CHAIN_ATTACK:
      case SKILL_HURL:
      case SKILL_BONEBREAK:
      case SKILL_DEFENESTRATE:
      case SKILL_SWINDLE:
      case SKILL_SNEAK:
      case SKILL_RETREAT_THIEF:
      case SKILL_PICK_LOCK:
      case SKILL_SEARCH:
      case SKILL_SPY:
      case SKILL_SWITCH_THIEF:
      case SKILL_STEAL:
      case SKILL_DETECT_TRAP:
      case SKILL_SUBTERFUGE:
      case SKILL_DISARM_TRAP:
      case SKILL_CUDGEL:
      case SKILL_HIDE:
      case SKILL_POISON_WEAPON:
      case SKILL_DISGUISE:
      case SKILL_DODGE_THIEF:
      case SKILL_SET_TRAP_CONT:
      case SKILL_SET_TRAP_DOOR:
      case SKILL_SET_TRAP_MINE:
      case SKILL_SET_TRAP_GREN:
      case SKILL_SET_TRAP_ARROW:
      case SKILL_DUAL_WIELD_THIEF:
      case SKILL_DISARM_THIEF:
      case SKILL_COUNTER_STEAL:
      case SPELL_DANCING_BONES:
      case SPELL_CONTROL_UNDEAD:
      case SPELL_RESURRECTION:
      case SPELL_VOODOO:
      case SKILL_BREW:
      case SPELL_VAMPIRIC_TOUCH:
      case SPELL_LIFE_LEECH:
      case SKILL_TURN:
      case SKILL_SIGN:
      case SKILL_SWIM:
      case SKILL_CONS_UNDEAD:
      case SKILL_CONS_VEGGIE:
      case SKILL_CONS_DEMON:
      case SKILL_CONS_ANIMAL:
      case SKILL_CONS_REPTILE:
      case SKILL_CONS_PEOPLE:
      case SKILL_CONS_GIANT:
      case SKILL_CONS_OTHER:
      case SKILL_READ_MAGIC:
      case SKILL_BANDAGE:
      case SKILL_CLIMB:
      case SKILL_FAST_HEAL:
      case SKILL_EVALUATE:
      case SKILL_TACTICS:
      case SKILL_DISSECT:
      case SKILL_DEFENSE:
      case SKILL_ADVANCED_DEFENSE:
      case SKILL_FOCUSED_AVOIDANCE:
      case SKILL_OFFENSE:
      case SKILL_ADVANCED_OFFENSE:
      case SKILL_WHITTLE:
      case SKILL_WIZARDRY:
      case SKILL_RITUALISM:
      case SKILL_MEDITATE:
      case SKILL_DEVOTION:
      case SKILL_PENANCE:
      case SKILL_SLASH_PROF:
      case SKILL_PIERCE_PROF:
      case SKILL_BLUNT_PROF:
      case SKILL_BAREHAND_PROF:
      case SKILL_SLASH_SPEC:
      case SKILL_BLUNT_SPEC:
      case SKILL_PIERCE_SPEC:
      case SKILL_BAREHAND_SPEC:
      case SKILL_RANGED_SPEC:
      case SKILL_RANGED_PROF:
      case SKILL_FAST_LOAD:
      case SKILL_SHARPEN:
      case SKILL_DULL:
      case SKILL_ATTUNE:
      case SKILL_STAVECHARGE:
      case SKILL_MANA:
      case SKILL_PIERCE_RESIST:
      case SPELL_EMBALM:
      case SPELL_EARTHMAW:
      case SPELL_CREEPING_DOOM:
      case SPELL_FERAL_WRATH:
      case SPELL_SKY_SPIRIT:
      case SKILL_GUTTER_CANT:
      case SKILL_GNOLL_JARGON:
      case SKILL_TROGLODYTE_PIDGIN:
      case SKILL_TROLLISH:
      case SKILL_BULLYWUGCROAK:
      case SKILL_AVIAN:
      case SKILL_FISHBURBLE:
      case SKILL_COMMON:
        // some spells have 2 effects, skip over one of them
        if (!aff->shouldGenerateText())
          continue;
        else if (discArray[aff->type]) {
          if (show && strcmp(discArray[aff->type]->name, "sneak")) {
            if (aff->renew < 0) {
              str += format("Affected : '%s'\t: Approx. Duration : %s\n\r") %
                     discArray[aff->type]->name %
                     describeDuration(this, aff->duration);
            } else {
              if (aff->canBeRenewed()) {
                str += format("Affected : '%s'\t: Time Left : %s %s%s%s\n\r") %
                       discArray[aff->type]->name %
                       describeDuration(this, aff->duration) % red() %
                       "(Renewable)" % norm();
              } else {
                str += format("Affected : '%s'\t: Time Left : %s %s%s%s\n\r") %
                       discArray[aff->type]->name %
                       describeDuration(this, aff->duration) % green() %
                       "(Not Yet Renewable)" % norm();
              }
            }
          }
        } else {
          vlogf(LOG_BUG,
            format("BOGUS AFFECT (%d) on %s.") % aff->type % ch->getName());
          ch->affectRemove(aff);
        }
        break;
      case AFFECT_DISEASE:
        if (show) {
          str +=
            format("Disease: '%s'\n\r") % DiseaseInfo[affToDisease(*aff)].name;
        }
        break;
      case AFFECT_DUMMY:
        if (show) {
          if (aff->canBeRenewed()) {
            str += format("Affected : '%s'\t: Time Left : %s %s%s%s\n\r") %
                   "DUMMY" % describeDuration(this, aff->duration) % red() %
                   "(Renewable)" % norm();
          } else {
            str += format("Affected : '%s'\t: Time Left : %s %s%s%s\n\r") %
                   "DUMMY" % describeDuration(this, aff->duration) % green() %
                   "(Not Yet Renewable)" % norm();
          }
        }
        break;
      case AFFECT_WAS_INDOORS:
        if (ch->isImmortal() && show) {
          str +=
            format("Was indoors (immune to frostbite): Time Left : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_FREE_DEATHS:
        str += format("Free deaths remaining: %ld\n\r") % aff->modifier;
        break;
      case AFFECT_HORSEOWNED:
        str += format("Horseowned:\t Time Left : %s\n\r") %
               describeDuration(this, aff->duration);
        break;
      case AFFECT_PLAYERKILL:
        str += format("Player Killer:\t Time Left : %s\n\r") %
               describeDuration(this, aff->duration);
        break;
      case AFFECT_PLAYERLOOT:
        str += format("Player Looter:\t Time Left : %s\n\r") %
               describeDuration(this, aff->duration);
        break;
      case AFFECT_TEST_FIGHT_MOB:
        str += format("Test Fight Mob: %ld\n\r") % aff->modifier;
        break;
      case AFFECT_SKILL_ATTEMPT:
        if (isImmortal()) {
          str += format("Skill Attempt:(%ld) '%s'\t: Time Left : %s\n\r") %
                 aff->modifier %
                 (discArray[aff->modifier] ? discArray[aff->modifier]->name
                                           : "Unknown") %
                 describeDuration(this, aff->duration);
        } else if (aff->modifier != getSkillNum(SKILL_SNEAK)) {
          str += format("Skill Attempt: '%s'\t: Time Left : %s\n\r") %
                 (discArray[aff->modifier] ? discArray[aff->modifier]->name
                                           : "Unknown") %
                 describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_NEWBIE:
        if (show) {
          str += "Donation Recipient: \n\r";
        }
        break;
      case AFFECT_DRUNK:
        if (show) {
          str +=
            format("Affected: Drunken Slumber: approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        } else {
          str += "Affected: Drunken Slumber: \n\r";
        }
        break;
      case AFFECT_DRUG:
        if (!aff->shouldGenerateText())
          continue;
        if (show) {
          str += format("Affected: %s: approx. duration : %s\n\r") %
                 drugTypes[aff->modifier2].name %
                 describeDuration(this, aff->duration);
        } else {
          str += format("Affected: %s: \n\r") % drugTypes[aff->modifier2].name;
        }
        break;
      case AFFECT_TRANSFORMED_ARMS:
        if (show) {
          str += format(
                   "Affected: Transformed Limb: falcon wings: approx. duration "
                   ": %s\n\r") %
                 describeDuration(this, aff->duration);
        } else {
          str += "Affected: Transformed Limb: falcon wings: \n\r";
        }
        break;
      case AFFECT_TRANSFORMED_HANDS:
        if (ch == this)
          str += format(
                   "Affected: Transformed Limb: bear claws: approx. duration : "
                   "%s\n\r") %
                 describeDuration(this, aff->duration);
        else
          str += "Affected: Transformed Limb: bear claws \n\r";
        break;
      case AFFECT_TRANSFORMED_LEGS:
        if (ch == this)
          str += format(
                   "Affected: Transformed Limb: dolphin tail: approx. duration "
                   ": %s\n\r") %
                 describeDuration(this, aff->duration);
        else
          str += "Affected: Transformed Limb: dolphin tail: \n\r";
        break;
      case AFFECT_TRANSFORMED_HEAD:
        if (ch == this)
          str += format(
                   "Affected: Transformed Limb: eagle's head: approx. duration "
                   ": %s\n\r") %
                 describeDuration(this, aff->duration);
        else
          str += "Affected: Transformed Limb: eagle's head: \n\r";
        break;
      case AFFECT_TRANSFORMED_NECK:
        if (ch == this)
          str += format(
                   "Affected: Transformed Limb: fish gills: approx. duration : "
                   "%s\n\r") %
                 describeDuration(this, aff->duration);
        else
          str += "Affected: Transformed Limb: fish gills: \n\r";
        break;
      case AFFECT_GROWTH_POTION:
        if (ch == this)
          str += format("Affected: Abnormal Growth: approx duration : %s\n\r") %
                 describeDuration(this, aff->duration);
        else
          str += "Affected: Abnormal Growth\n\r";
        break;

      case AFFECT_DEFECTED:
        if (ch == this)
          str += format(
                   "You recently defected from your faction.\n\r\ttime left : "
                   "%s\n\r") %
                 describeDuration(this, aff->duration);
        else
          str += "Recently defected from a faction.\n\r";
        break;
      case AFFECT_OFFER:
        if (ch == this) {
          TGuild* f = NULL;
          f = get_guild_by_ID(aff->modifier);
          if (!f)
            break;
          str +=
            format("You received an offer to join %s. (Good for %s.)\n\r") %
            f->getName() % describeDuration(this, aff->duration);
        } else
          str += "Received an offer to join a faction.\n\r";
        break;
      case AFFECT_OBJECT_USED:
        objused = aff->modifier;
        if (show) {
          str += format("Used magical object: %s\n\r") %
                 obj_index[objused].short_desc;
          str += format("     Object is reusable in %s.\n\r") %
                 describeDuration(this, aff->duration);
        } else {
          str += "Used a magical object effect.";
        }
        break;

      case AFFECT_COMBAT:
        // no display
        break;
      case AFFECT_PET:
        if (show) {
          str += format("Pet of: '%s'.  Approx. duration : %s\n\r") %
                 (char*)aff->be % describeDuration(this, aff->duration);
        } else {
          str = "Somebody's Pet.\n\r";
        }
        break;
      case AFFECT_CHARM:
        if (show) {
          str += format("Charm of: '%s'.  Approx. duration : %s\n\r") %
                 (char*)aff->be % describeDuration(this, aff->duration);
        } else {
          str += "Somebody's Charm.\n\r";
        }
        break;
      case AFFECT_THRALL:
        if (show) {
          str += format("Thrall of: '%s'.  Approx. duration : %s\n\r") %
                 (char*)aff->be % describeDuration(this, aff->duration);
        } else {
          str += "Somebody's Thrall.\n\r";
        }
        break;
      case AFFECT_WARY:
      case AFFECT_ORPHAN_PET:
        // no display
        break;

      case SKILL_MIND_FOCUS:
        if (show) {
          str += format("Affected: mind focus.  Approx. duration : %s\n\r") %
                 describeDuration(this, aff->duration);
        }
        break;
      case SKILL_PSI_BLAST:
        if (show && aff->shouldGenerateText()) {
          str += format("Affected: psionic blast.  Approx. duration : %s\n\r") %
                 describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_IMMORTAL_BLESSING:
        if (show) {
          str +=
            format(
              "Affected: Immortal's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_AION_BLESSING:
        if (show) {
          str +=
            format("Affected: Aion's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_VASCO_BLESSING:
        if (show) {
          str +=
            format("Affected: Vasco's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_CORAL_BLESSING:
        if (show) {
          str +=
            format("Affected: Coral's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_SIDARTHA_BLESSING:
        if (show) {
          str +=
            format(
              "Affected: Sidartha's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_BOZ_BLESSING:
        if (show) {
          str +=
            format("Affected: Boz's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_DAMESCENA_BLESSING:
        if (show) {
          str +=
            format(
              "Affected: Damescena's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_BUMP_BLESSING:
        if (show) {
          str +=
            format("Affected: Bump's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_ONSLAUGHT_BLESSING:
        if (show) {
          str +=
            format(
              "Affected: Onslaught's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_DASH_BLESSING:
        if (show) {
          str +=
            format("Affected: Dash's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_DEIRDRE_BLESSING:
        if (show) {
          str +=
            format("Affected: Deirdre's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_GARTHAGK_BLESSING:
        if (show) {
          str +=
            format(
              "Affected: Garthagk's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_MERCURY_BLESSING:
        if (show) {
          str +=
            format("Affected: Mercury's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_METROHEP_BLESSING:
        if (show) {
          str +=
            format(
              "Affected: Metrohep's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_MAGDALENA_BLESSING:
        if (show) {
          str +=
            format(
              "Affected: Magdalena's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_MACROSS_BLESSING:
        if (show) {
          str +=
            format("Affected: Macross's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_PAPPY_BLESSING:
        if (show) {
          str +=
            format("Affected: Pappy's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_STAFFA_BLESSING:
        if (show) {
          str +=
            format("Affected: Staffa's Blessing.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_UNHOLY_WRATH:
        if (show) {
          str += format("Affected: Unholy Wrath.  Approx. duration : %s\n\r") %
                 describeDuration(this, aff->duration);
        }
        break;
      // We want this to show "projecting" to be clear so we handle it different
      case SKILL_AURA_MIGHT:
      case SKILL_AURA_REGENERATION:
      case SKILL_AURA_GUARDIAN:
      case SKILL_AURA_VENGEANCE:
      case SKILL_AURA_ABSOLUTION:
        if (show) {
          str += format("Projecting: '%s'\t: Approx. Duration : %s\n\r") %
                 discArray[aff->type]->name %
                 describeDuration(this, aff->duration);
        }
        break;
      // Not in discarray since it's not a skill
      case SPELL_AURA_MIGHT:
        if (show && aff->shouldGenerateText()) {
          str += format("Affected: Aura of Might.  Approx. duration : %s\n\r") %
                 describeDuration(this, aff->duration);
        }
        break;
      case SPELL_AURA_REGENERATION:
        if (show) {
          str +=
            format(
              "Affected: Aura of Regeneration.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case SPELL_AURA_GUARDIAN:
        if (show && aff->shouldGenerateText()) {
          str += format("Affected: Guardian Aura.  Approx. duration : %s\n\r") %
                 describeDuration(this, aff->duration);
        }
        break;
      case SPELL_AURA_VENGEANCE:
        if (show && aff->shouldGenerateText()) {
          str +=
            format("Affected: Aura of Vengeance.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case SPELL_AURA_ABSOLUTION:
        if (show && aff->shouldGenerateText()) {
          str +=
            format("Affected: Aura of Absolution.  Approx. duration : %s\n\r") %
            describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_HOLY_WRATH:
        if (show) {
          str += format("Affected: Holy Wrath.  Approx. duration : %s\n\r") %
                 describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_HOLY_BEAM:
        if (show) {
          str += format("Affected: Holy Beam.  Approx. duration : %s\n\r") %
                 describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_FORTIFY:
        if (show) {
          str += format("Affected: Shield Wall.  Approx. duration : %s\n\r") %
                 describeDuration(this, aff->duration);
        }
        break;
      case SKILL_TOUGHNESS:
        if (show) {
          sstring noun = (aff->modifier2 > 1) ? "stacks" : "stack";
          str +=
            format("Affected: Toughness : %s %s. Approx. duration : %s\n\r") %
            aff->modifier2 % noun % describeDuration(this, aff->duration);
        }
        break;
      case SKILL_INEVITABILITY:
        if (show) {
          sstring noun = (aff->modifier > 1) ? "stacks" : "stack";
          str +=
            format(
              "Affected: Inevitablity : %s %s. Approx. duration : %s\n\r") %
            aff->modifier % noun % describeDuration(this, aff->duration);
        }
        break;
      case AFFECT_GUARDIANS_LIGHT:
        if (show) {
          str += format(
                   "Affected: Guardians Light : %s percent protection.  "
                   "Approx. duration : %s\n\r") %
                 aff->modifier % describeDuration(this, aff->duration);
        }

      case AFFECT_BITTEN_BY_VAMPIRE:
        // secret!
        break;

      case AFFECT_PREENED:
        if (show)
          str += format("Affected: 'Preened'\t: Time Left : %s\n\r") %
                 describeDuration(this, aff->duration);
        break;

      case AFFECT_WET:
        // we show this in score
        break;

      // cases beyond here are considered BOGUs
      case LAST_ODDBALL_AFFECT:
      case LAST_TRANSFORMED_LIMB:
      case LAST_BREATH_WEAPON:
      case DAMAGE_GUST:
      case DAMAGE_TRAP_TNT:
      case DAMAGE_ELECTRIC:
      case DAMAGE_TRAP_FROST:
      case DAMAGE_FROST:
      case DAMAGE_DROWN:
      case DAMAGE_WHIRLPOOL:
      case DAMAGE_HEMORRHAGE:
      case DAMAGE_IMPALE:
      case DAMAGE_TRAP_POISON:
      case DAMAGE_ACID:
      case DAMAGE_TRAP_ACID:
      case DAMAGE_COLLISION:
      case DAMAGE_FALL:
      case DAMAGE_TRAP_BLUNT:
      case DAMAGE_TRAP_FIRE:
      case DAMAGE_FIRE:
      case DAMAGE_DISRUPTION:
      case DAMAGE_DRAIN:
      case DAMAGE_TRAP_ENERGY:
      case DAMAGE_KICK_HEAD:
      case DAMAGE_KICK_SHIN:
      case DAMAGE_KICK_SIDE:
      case DAMAGE_KICK_SOLAR:
      case DAMAGE_TRAP_DISEASE:
      case DAMAGE_SUFFOCATION:
      case DAMAGE_TRAP_SLASH:
      case DAMAGE_ARROWS:
      case DAMAGE_TRAP_PIERCE:
      case DAMAGE_DISEMBOWLED_HR:
      case DAMAGE_DISEMBOWLED_VR:
      case DAMAGE_EATTEN:
      case DAMAGE_HACKED:
      case DAMAGE_KNEESTRIKE_FOOT:
      case DAMAGE_HEADBUTT_FOOT:
      case DAMAGE_KNEESTRIKE_SHIN:
      case DAMAGE_KNEESTRIKE_KNEE:
      case DAMAGE_KNEESTRIKE_THIGH:
      case DAMAGE_HEADBUTT_LEG:
      case DAMAGE_KNEESTRIKE_SOLAR:
      case DAMAGE_HEADBUTT_BODY:
      case DAMAGE_KNEESTRIKE_CROTCH:
      case DAMAGE_HEADBUTT_CROTCH:
      case DAMAGE_HEADBUTT_THROAT:
      case DAMAGE_KNEESTRIKE_CHIN:
      case DAMAGE_HEADBUTT_JAW:
      case DAMAGE_KNEESTRIKE_FACE:
      case DAMAGE_RIPPED_OUT_HEART:
      case DAMAGE_CAVED_SKULL:
      case DAMAGE_HEADBUTT_SKULL:
      case DAMAGE_STARVATION:
      case DAMAGE_STOMACH_WOUND:
      case DAMAGE_RAMMED:
      case DAMAGE_BEHEADED:
      case DAMAGE_NORMAL:
      case DAMAGE_TRAP_SLEEP:
      case DAMAGE_TRAP_TELEPORT:
      case DAMAGE_HOLY:
      case MAX_SKILL:
      case TYPE_WATER:
      case TYPE_AIR:
      case TYPE_EARTH:
      case TYPE_FIRE:
      case TYPE_KICK:
      case TYPE_CLAW:
      case TYPE_SLASH:
      case TYPE_CLEAVE:
      case TYPE_SLICE:
      case TYPE_BEAR_CLAW:
      case TYPE_MAUL:
      case TYPE_SMASH:
      case TYPE_WHIP:
      case TYPE_CRUSH:
      case TYPE_BLUDGEON:
      case TYPE_SMITE:
      case TYPE_HIT:
      case TYPE_FLAIL:
      case TYPE_PUMMEL:
      case TYPE_THRASH:
      case TYPE_THUMP:
      case TYPE_WALLOP:
      case TYPE_BATTER:
      case TYPE_BEAT:
      case TYPE_STRIKE:
      case TYPE_POUND:
      case TYPE_CLUB:
      case TYPE_PIERCE:
      case TYPE_STAB:
      case TYPE_STING:
      case TYPE_THRUST:
      case TYPE_SPEAR:
      case TYPE_BEAK:
      case TYPE_BITE:
      case TYPE_SHOOT:
      case TYPE_CANNON:
      case TYPE_UNDEFINED:
      case TYPE_SHRED:
      case TYPE_MAX_HIT:
      case SKILL_ALCOHOLISM:
      case SKILL_FISHING:
      case SKILL_LOGGING:
      case SKILL_PSITELEPATHY:
      case SKILL_TELE_SIGHT:
      case SKILL_TELE_VISION:
      case SKILL_MIND_THRUST:
      case SKILL_PSYCHIC_CRUSH:
      case SKILL_KINETIC_WAVE:
      case SKILL_MIND_PRESERVATION:
      case SKILL_TELEKINESIS:
      case SKILL_PSIDRAIN:
      case SKILL_DIMENSIONAL_FOLD:
      case SKILL_IRON_FIST:
      case SKILL_IRON_FLESH:
      case SKILL_IRON_SKIN:
      case SKILL_IRON_BONES:
      case SKILL_IRON_MUSCLES:
      case SKILL_IRON_LEGS:
      case SKILL_IRON_WILL:
      case SKILL_PLANT:
      case ABSOLUTE_MAX_SKILL:
        vlogf(LOG_BUG,
          format("BOGUS AFFECT (%d) on %s.") % aff->type % ch->getName());
        ch->affectRemove(aff);
        break;
    }
  }
  return str;
}

void TBeing::describeLimbDamage(const TBeing* ch) const {
  sstring buf, buf2;
  wearSlotT j;
  TThing* t;

  if (ch == this)
    buf2 = "your";
  else
    buf2 = ch->hshr();

  for (j = MIN_WEAR; j < MAX_WEAR; j++) {
    if (j == HOLD_RIGHT || j == HOLD_LEFT)
      continue;
    if (!ch->slotChance(j))
      continue;
    if (ch->isLimbFlags(j, PART_TRANSFORMED)) {
      const sstring str = describe_part_wounds(ch, j);
      if (!str.empty()) {
        act(format("<y>%s %s %s %s<1>") % buf2.cap() % ch->describeBodySlot(j) %
              ch->slotPlurality(j) % str,
          FALSE, this, NULL, NULL, TO_CHAR);
      }
    }
    if ((t = ch->getStuckIn(j))) {
      if (canSee(t)) {
        buf = format("<y>$p is sticking out of %s %s!<1>") % buf2.uncap() %
              ch->describeBodySlot(j);
        act(buf, FALSE, this, t, NULL, TO_CHAR);
      }
    }
  }
  if (ch->affected) {
    affectedData* aff;
    for (aff = ch->affected; aff; aff = aff->next) {
      if (aff->type == AFFECT_DISEASE) {
        if (!aff->level) {
          if (ch == this)
            sendTo(COLOR_BASIC, format("<y>You have %s.<1>\n\r") %
                                  DiseaseInfo[affToDisease(*aff)].name);
          else
            sendTo(COLOR_BASIC, format("<y>It seems %s has %s.<1>\n\r") %
                                  ch->hssh() %
                                  DiseaseInfo[affToDisease(*aff)].name);
        }
      }
    }
  }
}

void TBeing::doTime(const char* argument) {
  sstring buf, arg;
  int weekday, day, tmp2;

  if (!desc) {
    sendTo("Silly mob, go home.\n\r");
    return;
  }

  one_argument(argument, arg);
  if (!arg.empty()) {
    if (!convertTo<int>(arg) && arg != "0") {
      sendTo(format("Present time differential is set to %d hours.\n\r") %
             desc->account->time_adjust);
      sendTo("Syntax: time <difference>\n\r");
      return;
    }
    desc->account->time_adjust = convertTo<int>(arg);
    sendTo(format("Your new time difference between your site and %s's will "
                  "be: %d hours.\n\r") %
           MUD_NAME % desc->account->time_adjust);
    desc->saveAccount();
    return;
  }
  buf =
    format("It is %s, on ") % GameTime::hmtAsString(GameTime::hourminTime());

  weekday = ((28 * GameTime::getMonth()) + GameTime::getDay() + 1) %
            7;  // 28 days in a month

  buf += weekdays[weekday];
  buf += "\n\r";
  sendTo(buf);

  day = GameTime::getDay() + 1;  // day in [1..28]

  sendTo(format("The %s day of %s, Year %d P.S.\n\r") % numberAsString(day) %
         month_name[GameTime::getMonth()] % GameTime::getYear());

  tmp2 = Weather::sunTime(Weather::Weather::SUN_TIME_RISE);
  buf = format("The sun will rise today at:   %s.\n\r") %
        GameTime::hmtAsString(tmp2);
  sendTo(buf);

  tmp2 = Weather::sunTime(Weather::Weather::SUN_TIME_SET);
  buf = format("The sun will set today at:    %s.\n\r") %
        GameTime::hmtAsString(tmp2);
  sendTo(buf);

  tmp2 = Weather::moonTime(Weather::MOON_TIME_RISE);
  buf = format("The moon will rise today at:  %s    (%s).\n\r") %
        GameTime::hmtAsString(tmp2) % Weather::moonType();
  sendTo(buf);

  tmp2 = Weather::moonTime(Weather::MOON_TIME_SET);
  buf = format("The moon will set today at:   %s.\n\r") %
        GameTime::hmtAsString(tmp2);
  sendTo(buf);

  time_t ct;
  char* tmstr;
  if (desc->account)
    ct = time(0) + 3600 * desc->account->time_adjust;
  else
    ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sendTo(
    format("%sIn the real world, the time is:                     %s%s\n\r") %
    blue() % tmstr % norm());

  if (timeTill) {
    sendTo(format("%sThe game will be shutdown in %s.\n\r%s") % red() %
           secsToString(timeTill - time(0)) % norm());
  }
}

void TBeing::doWeather(const char* arg) {
  char buf[80];
  char buffer[256];
  Weather::changeWeatherT change = Weather::CHANGE_NONE;

  arg = one_argument(arg, buffer, cElements(buffer));

  if (!*buffer || !isImmortal()) {
    if (Weather::getWeather(*roomp) == Weather::SNOWY)
      strcpy(buf, "It is snowing");
    else if (Weather::getWeather(*roomp) == Weather::LIGHTNING)
      strcpy(buf,
        "The sky is lit by flashes of lightning as a heavy rain pours down");
    else if (Weather::getWeather(*roomp) == Weather::RAINY)
      strcpy(buf, "It is raining");
    else if (Weather::getWeather(*roomp) == Weather::CLOUDY)
      strcpy(buf, "The sky is cloudy");
    else if (Weather::getWeather(*roomp) == Weather::CLOUDLESS) {
      if (GameTime::is_nighttime())
        strcpy(buf, "A <K>dark sky<1> stretches before you");
      else
        strcpy(buf, "A <b>clear blue sky<1> stretches before you");
      //  } else if (Weather::getWeather(*roomp) == Weather::WINDY) {
      //    strcpy(buf,"The winds begin to pick up");
    } else if (Weather::getWeather(*roomp) == Weather::NONE) {
      sendTo("You have no feeling about the weather at all.\n\r");
      describeRoomLight();
      return;
    } else {
      vlogf(LOG_BUG, format("Error in getWeather for %s.") % getName());
      return;
    }
    if (isImmortal()) {
      sendTo(
        format("The current barometer is: %d.  Barometric change is: %d\n\r") %
        Weather::getPressure() % Weather::getChange());
    }
    sendTo(COLOR_BASIC, format("%s and %s.\n\r") % buf %
                          (Weather::getChange() >= 0
                              ? "you feel a relatively warm wind from the south"
                              : "your foot tells you bad weather is due"));

    if (Weather::moonIsUp()) {
      sendTo(format("A %s moon hangs in the sky.\n\r") % Weather::moonType());
    }
    describeRoomLight();
    return;
  } else {
    if (is_abbrev(buffer, "worse")) {
      Weather::addToChange(-10);
      sendTo("The air-pressure drops and the weather worsens.\n\r");
      Weather::addToPressure(Weather::getChange());

      Weather::AlterWeather(&change);
      if (Weather::getPressure() >= 1040) {
        sendTo("The weather can't get any better.\n\r");
        Weather::setPressure(1040);
      } else if (Weather::getPressure() <= 960) {
        sendTo("The weather can't get any worse.\n\r");
        Weather::setPressure(960);
      }
      return;
    } else if (is_abbrev(buffer, "better")) {
      Weather::addToChange(10);
      sendTo("The air-pressure climbs and the weather improves.\n\r");
      Weather::addToPressure(Weather::getChange());

      Weather::AlterWeather(&change);
      if (Weather::getPressure() >= 1040) {
        sendTo("The weather can't get any better.\n\r");
        Weather::setPressure(1040);
      } else if (Weather::getPressure() <= 960) {
        sendTo("The weather can't get any worse.\n\r");
        Weather::setPressure(960);
      }
    } else if (is_abbrev(buffer, "month")) {
      arg = one_argument(arg, buffer, cElements(buffer));
      if (!*buffer) {
        sendTo("Syntax: weather month <num>\n\r");
        return;
      }
      int num = convertTo<int>(buffer);
      if (num <= 0 || num > 12) {
        sendTo("Syntax: weather month <num>\n\r");
        sendTo("<num> must be in range 1-12.\n\r");
        return;
      }
      GameTime::setMonth(num - 1);
      sendTo(format("You set the month to: %s\n\r") %
             month_name[GameTime::getMonth()]);
      return;
    } else if (is_abbrev(buffer, "moon")) {
      arg = one_argument(arg, buffer, cElements(buffer));
      if (!*buffer) {
        sendTo("Syntax: weather moon <num>\n\r");
        return;
      }
      int num = convertTo<int>(buffer);
      if (num <= 0 || num > 32) {
        sendTo("Syntax: weather moon <num>\n\r");
        sendTo("<num> must be in range 1-32.\n\r");
        return;
      }
      Weather::setMoon(num);
      sendTo(format("The moon is now in stage %d (%s).\n\r") %
             Weather::getMoon() % Weather::moonType());
      return;
    } else {
      sendTo(
        "Syntax: weather <\"worse\" | \"better\" | \"month\" | \"moon\">\n\r");
      return;
    }
  }
}

void TBeing::doUsers(const sstring&) {
  sendTo("Dumb monsters can't use the users command!\n\r");
}

void TPerson::doUsers(const sstring& argument) {
  sstring line, buf2, buf3, buf4, buf5, sb, arg1, arg2;
  Descriptor* d;
  int count = 0;
  TBeing* k = NULL;

  if (powerCheck(POWER_USERS))
    return;

  const char USERS_HEADER[] =
    "\n\rName              Hostname                           Connected  "
    "Account Name   "
    "Client\n\r----------------------------------------------------------------"
    "---------------------\n\r";

  arg1 = argument.word(0);
  arg2 = argument.word(1);

  if (arg1.empty()) {
    sb += USERS_HEADER;

    for (d = descriptor_list; d; d = d->next) {
      if (d->character && !d->character->getName().empty()) {
        if (!d->connected && !canSeeWho(d->character))
          continue;

        if (d->original)
          line =
            format("%s%-16.16s%s: ") % purple() % d->original->name % norm();
        else
          line = format("%s%-16.16s%s: ") % purple() % d->character->getName() %
                 norm();
      } else
        line = "UNDEFINED       : ";

      // don't let newbie gods blab who imm's mortals are
      if (d->account && IS_SET(d->account->flags, TAccount::IMMORTAL) &&
          !hasWizPower(POWER_VIEW_IMM_ACCOUNTS)) {
        line += "*** Information Concealed ***\n\r";
      } else {
        sstring tmp_host = !(d->host.empty()) ? d->host : "????";
        boost::replace_all(tmp_host, "::ffff:", "");
        buf2 = format("[%s]") % tmp_host;

        buf3 =
          format("[%s]") % ((d->connected < MAX_CON_STATUS && d->connected >= 0)
                               ? connected_types[d->connected]
                               : "Editing");
        buf4 = format("[%s]") % ((d->account && !d->account->name.empty())
                                    ? d->account->name
                                    : "UNDEFINED");

        buf5 = format("%s %s") % d->mudclient % d->clientversion;

        line += format("%s%-34.34s%s %s%-10.10s%s %s%-14.14s%s %s%s\n\r") %
                red() % buf2 % norm() % green() % buf3 % norm() % cyan() %
                buf4 % norm() % buf5 % norm();
      }
      sb += line;
      count++;
    }
    buf2 = format("\n\rTotal Descriptors : %d\n\r") % count;
    sb += buf2;
    if (desc)
      desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
    return;
  } else if (is_abbrev(arg1, "site")) {
    if (arg2.empty()) {
      sendTo("Syntax : users site <sitename>\n\r");
      return;
    } else {
      sendTo(format("\n\rPlayers online from %s:\n\r\n\r") % arg2);
      sendTo(USERS_HEADER);
      for (d = descriptor_list; d; d = d->next) {
        if (d->host.lower().find(arg2.lower(), 0) != sstring::npos) {
          if (d->character && !d->character->getName().empty()) {
            if (!d->connected && !canSeeWho(d->character))
              continue;

            if (d->original)
              line = format("%-16.16s: ") % d->original->name;
            else
              line = format("%-16.16s: ") % d->character->getName();
          } else
            line = "UNDEFINED       : ";

          // don't let newbie gods blab who imm's mortals are
          if (d->account && IS_SET(d->account->flags, TAccount::IMMORTAL) &&
              !hasWizPower(POWER_VIEW_IMM_ACCOUNTS)) {
            line += "*** Information Concealed ***\n\r";
          } else {
            buf2 = format("[%s]") % (!(d->host.empty()) ? d->host : "????");
            buf3 = format("[%s]") %
                   ((d->connected < MAX_CON_STATUS && d->connected >= 0)
                       ? connected_types[d->connected]
                       : "Editing");
            buf4 = format("[%s]") % ((d->account && !d->account->name.empty())
                                        ? d->account->name
                                        : "UNDEFINED");
            line += format("%-34.34s %-10.10s %s\n\r") % buf2 % buf3 % buf4;
          }
          sendTo(line);
          count++;
        }
      }
    }
    if (!count) {
      sendTo("No players online from that site.\n\r");
      return;
    }
  } else if ((k = get_pc_world(this, arg1, EXACT_YES)) ||
             (k = get_pc_world(this, arg1, EXACT_NO))) {
    if (k->desc) {
      // don't let newbie gods blab who imm's mortals are
      if (k->desc->account &&
          IS_SET(k->desc->account->flags, TAccount::IMMORTAL) &&
          !hasWizPower(POWER_VIEW_IMM_ACCOUNTS)) {
        sendTo(COLOR_MOBS,
          format("\n\r%-16.16s : *******Information Concealed*******\n\r") %
            k->getName());
      } else {
        buf2 =
          format("[%s]") % (!(k->desc->host.empty()) ? k->desc->host : "????");
        buf3 = format("[%s]") %
               ((k->desc->connected < MAX_CON_STATUS && k->desc->connected >= 0)
                   ? connected_types[k->desc->connected]
                   : "Editing");
        buf4 = format("[%s]") % (k->desc->account->name);
        sendTo(COLOR_MOBS,
          format("\n\r%-16.16s : %-34.34s %-15.15s %-10.10s\n\r") %
            k->getName() % buf2 % buf3 % buf4);
      }
      return;
    } else {
      sendTo(
        "That person is linkdead. Users can give you no info on them.\n\r");
      return;
    }
  } else {
    sendTo("Syntax : users (no argument list all users)\n\r");
    sendTo("         users <playername>\n\r");
    sendTo("         users site <sitename>\n\r");
    sendTo("The sitename can be abbreviated just like when you wizlock.\n\r");
    return;
  }
}

void TBeing::doInventory(const char* argument) {
  TBeing* victim;
  sstring sarg, arg1, arg2;
  sarg = argument;
  sarg = one_argument(sarg, arg1);
  sarg = one_argument(sarg, arg2);

  if (isImmortal() && !powerCheck(POWER_AT) && !arg1.empty()) {
    // immortal inventory check

    // find the target first
    if (!(victim = get_char_vis_world(this, arg1.c_str(), NULL, EXACT_YES))) {
      victim = get_char_vis_world(this, arg1.c_str(), NULL, EXACT_NO);
    }
    if (victim) {
      act("$N is carrying:", FALSE, this, NULL, victim, TO_CHAR);
      if (!arg2.empty()) {
        list_in_heap_filtered(victim->stuff, this, arg2, 1);
      } else {
        list_in_heap(victim->stuff, this, 1, 100);
      }
    } else {
      sendTo("No such being exists.\n\r");
    }
  } else {
    // checking own inventory

    if (isAffected(AFF_TRUE_SIGHT) || isAffected(AFF_CLARITY) ||
        !isAffected(AFF_BLIND)) {
      sendTo("You are carrying:\n\r");

      if (!arg1.empty()) {
        list_in_heap_filtered(stuff, this, arg1, 0);
      } else {
        list_in_heap(stuff, this, 0, 100);
      }

      if (GetMaxLevel() > 10) {
        sendTo(
          format("\n\r%3.f%c volume, %3.f%c weight.\n\r") %
          (((float)getCarriedVolume() / (float)carryVolumeLimit()) * 100.0) %
          '%' %
          (((float)getCarriedWeight() / (float)carryWeightLimit()) * 100.0) %
          '%');
      }
    } else {
      sendTo("It's pretty hard to take inventory when you can't see.\n\r");
    }
  }
}

void TBeing::doEquipment(const sstring& arg) {
  wearSlotT j;
  int found;
  sstring capbuf, buf, trans, argument = arg;
  TThing* t;

  one_argument(argument, buf);
  if (is_abbrev(buf, "damaged") || is_abbrev(buf, "all.damaged")) {
    sendTo("The following equipment is damaged:\n\r");
    found = FALSE;
    for (j = MIN_WEAR; j < MAX_WEAR; j++) {
      TThing* tt = equipment[j];
      TObj* tobj = dynamic_cast<TObj*>(tt);
      if (tobj && tobj->getMaxStructPoints() != tobj->getStructPoints()) {
        if (!tobj->shouldntBeShown(j)) {
          buf = format("<%s>") % describeEquipmentSlot(j);
          sendTo(format("%s%-25s%s") % cyan() % buf % norm());
          if (canSee(tobj)) {
            showTo(tobj, SHOW_MODE_SHORT_PLUS);
            found = TRUE;
          } else {
            sendTo("Something.\n\r");
            found = TRUE;
          }
        }
      }
    }
  } else if (argument.empty() || !isImmortal()) {
    TDatabase db(DB_SNEEZY);
    sstring tattoos[MAX_WEAR];

    db.query(
      "select location, tattoo from tattoos where name='%s' order by location",
      getName().c_str());
    while (db.fetchRow()) {
      tattoos[convertTo<int>(db["location"])] = db["tattoo"];
    }

    sendTo(format("You are using %i pounds of equipment:\n\r") %
           (int)equipment.getWeight());
    found = FALSE;
    for (j = MIN_WEAR; j < MAX_WEAR; j++) {
      if (equipment[j] && !equipment[j]->shouldntBeShown(j)) {
        buf = format("<%s>") % describeEquipmentSlot(j);
        sendTo(format("%s%-26s%s") % cyan() % buf % norm());
        if (canSee(equipment[j])) {
          showTo(equipment[j], SHOW_MODE_SHORT_PLUS);
          found = TRUE;
        } else {
          sendTo("Something.\n\r");
          found = TRUE;
        }
      } else if (tattoos[j] != "") {
        sstring slot = describeEquipmentSlot(j);
        buf = format("<%s>") %
              (slot.find("Worn") != sstring::npos
                  ? (sstring)slot.replace(slot.find("Worn"), 4, "Tattooed")
                  : slot);
        sendTo(format("%s%-26s%s") % red() % buf % norm());
        sendTo(COLOR_BASIC, tattoos[j]);
        sendTo("\n\r");
      }
    }
  } else {
    if (powerCheck(POWER_AT)) {
      vlogf(LOG_MISC,
        format("%s just tried to do: equipment %s") % getName() % arg);
      return;
    }

    TDatabase db(DB_SNEEZY);
    sstring tattoos[MAX_WEAR];

    // allow immortals to get eq of players
    TBeing* victim = get_char_vis_world(this, argument, NULL, EXACT_YES);
    if (!victim)
      victim = get_char_vis_world(this, argument, NULL, EXACT_NO);

    if (victim) {
      db.query(
        "select location, tattoo from tattoos where name='%s' order by "
        "location",
        victim->getName().c_str());
      while (db.fetchRow()) {
        tattoos[convertTo<int>(db["location"])] = db["tattoo"];
      }

      act("$N is using.", FALSE, this, 0, victim, TO_CHAR);
      for (j = MIN_WEAR; j < MAX_WEAR; j++) {
        if (victim->equipment[j] && !victim->equipment[j]->shouldntBeShown(j)) {
          buf = format("<%s>") % victim->describeEquipmentSlot(j);
          sendTo(format("%s%-26s%s") % cyan() % buf % norm());
          if (canSee(victim->equipment[j])) {
            showTo(victim->equipment[j], SHOW_MODE_SHORT_PLUS);
          } else {
            sendTo("Something.\n\r");
          }
        } else if (tattoos[j] != "") {
          sstring slot = describeEquipmentSlot(j);
          buf = format("<%s>") %
                (slot.find("Worn") != sstring::npos
                    ? (sstring)slot.replace(slot.find("Worn"), 4, "Tattooed")
                    : slot);
          sendTo(format("%s%-26s%s") % red() % buf % norm());

          //	  sprintf(buf, "<%s>", victim->describeEquipmentSlot(j).c_str());
          //	  sendTo(format("%s%-26s%s") % cyan() % buf % norm());
          sendTo(COLOR_BASIC, tattoos[j]);
          sendTo("\n\r");
        }
      }
    } else
      sendTo("No such character exists.\n\r");

    return;
  }
  if (!found)
    sendTo("Nothing.\n\r");

  for (j = MIN_WEAR; j < MAX_WEAR; j++) {
    if (j == HOLD_RIGHT || j == HOLD_LEFT)
      continue;
    if (!slotChance(j))
      continue;
    if (isLimbFlags(j, PART_TRANSFORMED)) {
      switch (j) {
        case WEAR_FINGER_R:
        case WEAR_FINGER_L:
          break;
        case WEAR_NECK:
          trans = format("<%s>") % describeTransLimb(j);
          sendTo(format("%s%s%s\n\r") % cyan() % trans % norm());
          break;
        case WEAR_BODY:
          break;
        case WEAR_HEAD:
          trans = format("<%s>") % describeTransLimb(j);
          sendTo(format("%s%s%s\n\r") % cyan() % trans % norm());
          break;
        case WEAR_LEG_L:
          break;
        case WEAR_LEG_R:
          trans = format("<%s>") % describeTransLimb(j);
          sendTo(format("%s%s%s\n\r") % cyan() % trans % norm());
          break;
        case WEAR_FOOT_R:
        case WEAR_FOOT_L:
          break;
        case WEAR_HAND_R:
          if (isLimbFlags(WEAR_ARM_R, PART_TRANSFORMED))
            break;
          trans = format("<%s>") % describeTransLimb(j);
          sendTo(format("%s%s%s\n\r") % cyan() % trans % norm());
          break;
        case WEAR_HAND_L:
          if (isLimbFlags(WEAR_ARM_L, PART_TRANSFORMED))
            break;
          trans = format("<%s>") % describeTransLimb(j);
          sendTo(format("%s%s%s\n\r") % cyan() % trans % norm());
          break;
        case WEAR_ARM_R:
          trans = format("<%s>") % describeTransLimb(j);
          sendTo(format("%s%s%s\n\r") % cyan() % trans % norm());
          break;
        case WEAR_ARM_L:
          trans = format("<%s>") % describeTransLimb(j);
          sendTo(format("%s%s%s\n\r") % cyan() % trans % norm());
          break;
        case WEAR_BACK:
        case WEAR_WAIST:
        case WEAR_WRIST_R:
        case WEAR_WRIST_L:
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case WEAR_EX_FOOT_R:
        case WEAR_EX_FOOT_L:
          break;
        default:
          break;
      }
    }
    if ((t = getStuckIn(j))) {
      if (canSee(t)) {
        capbuf = t->getName();
        sendTo(COLOR_OBJECTS, format("%s is sticking out of your %s!\n\r") %
                                capbuf.cap() % describeBodySlot(j));
      }
    }
  }
}

void TBeing::doCredits() {
  if (desc)
    desc->start_page_file(File::CREDITS, "Credits file being revised!\n\r");
}

void TBeing::doWizlist() {
  if (desc) {
    FILE* tFile;
    sstring tStString("");

    if (!(tFile = fopen(File::WIZLIST, "r")))
      sendTo("Sorry, wizlist under construction!\n\r");
    else {
      wizlist_used_num++;

      file_to_sstring(File::WIZLIST, tStString);
      desc->page_string(tStString);
      fclose(tFile);
    }
  }
}

void do_where_thing(const TBeing* ch, const TThing* obj, bool recurse,
  sstring& sb) {
  char buf[256];

  if (obj->in_room != Room::NOWHERE) {  // object in a room
    sprintf(buf, "%s\n\r      - ", obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%-35s [%d]\n\r",
      obj->roomp->getNameNOC(ch).c_str(), obj->in_room);
    // object carried by monster
  } else if (dynamic_cast<TBeing*>(obj->parent) && obj->parent->roomp) {
    sprintf(buf, "%s\n\r      - carried by %s -", obj->getNameNOC(ch).c_str(),
      obj->parent->getName().c_str());
    sprintf(buf + strlen(buf), " %-20s [%d]\n\r",
      (!obj->parent->roomp->getName().empty()
          ? obj->parent->roomp->getNameNOC(ch).c_str()
          : "Room Unknown"),
      obj->parent->in_room);
  } else if (dynamic_cast<TBeing*>(obj->parent) && obj->parent->riding &&
             obj->parent->riding->roomp) {
    sprintf(buf, "%s\n\r      - carried by %s - ", obj->getNameNOC(ch).c_str(),
      obj->parent->getName().c_str());
    sprintf(buf + strlen(buf), "riding %s - ",
      obj->parent->riding->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r",
      (!obj->parent->riding->roomp->getName().empty()
          ? obj->parent->riding->roomp->getNameNOC(ch).c_str()
          : "Room Unknown"),
      obj->parent->riding->in_room);
  } else if (dynamic_cast<TBeing*>(obj->parent) && obj->parent->riding) {
    sprintf(buf, "%s\n\r      - carried by %s - ", obj->getNameNOC(ch).c_str(),
      obj->parent->getName().c_str());
    sprintf(buf + strlen(buf), "riding %s - (Room Unknown)\n\r",
      obj->parent->riding->getNameNOC(ch).c_str());
  } else if (dynamic_cast<TBeing*>(obj->parent)) {  // object carried by monster
    sprintf(buf, "%s\n\r      - carried by %s (Room Unknown)\n\r",
      obj->getNameNOC(ch).c_str(), obj->parent->getName().c_str());
    // object equipped by monster
  } else if (obj->equippedBy && obj->equippedBy->roomp) {
    sprintf(buf, "%s\n\r      - equipped by %s - ", obj->getNameNOC(ch).c_str(),
      obj->equippedBy->getName().c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r",
      (!obj->equippedBy->roomp->getName().empty()
          ? obj->equippedBy->roomp->getNameNOC(ch).c_str()
          : "Room Unknown"),
      obj->equippedBy->in_room);
  } else if (obj->equippedBy) {  // object equipped by monster
    sprintf(buf, "%s\n\r      - equipped by %s (Room Unknown)\n\r",
      obj->getNameNOC(ch).c_str(), obj->equippedBy->getName().c_str());
  } else if (obj->stuckIn && obj->stuckIn->roomp) {
    sprintf(buf, "%s\n\r      - stuck in %s - ", obj->getNameNOC(ch).c_str(),
      obj->stuckIn->getName().c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r",
      (!obj->stuckIn->roomp->getName().empty()
          ? obj->stuckIn->roomp->getNameNOC(ch).c_str()
          : "Room Unknown"),
      obj->stuckIn->in_room);
  } else if (obj->stuckIn) {
    sprintf(buf, "%s\n\r      - stuck in %s - (Room Unknown)\n\r",
      obj->getNameNOC(ch).c_str(), obj->stuckIn->getName().c_str());
    // object in object
  } else if (obj->parent && obj->parent->parent) {
    sprintf(buf, "%s\n\r      - ", obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "in %s - ", obj->parent->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "in %s - ",
      obj->parent->parent->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r",
      ((obj->parent->parent->roomp &&
         !obj->parent->parent->roomp->getName().empty())
          ? obj->parent->parent->roomp->getNameNOC(ch).c_str()
          : "Room Unknown"),
      obj->parent->parent->in_room);
  } else if (obj->parent && obj->parent->equippedBy) {
    sprintf(buf, "%s\n\r      - ", obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "in %s - ", obj->parent->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "equipped by %s - ",
      obj->parent->equippedBy->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r",
      ((obj->parent->equippedBy->roomp &&
         !obj->parent->equippedBy->roomp->getName().empty())
          ? obj->parent->equippedBy->roomp->getNameNOC(ch).c_str()
          : "Room Unknown"),
      obj->parent->equippedBy->in_room);
  } else if (obj->parent && obj->parent->stuckIn) {
    sprintf(buf, "%s\n\r      - ", obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "in %s - ", obj->parent->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "stuck in %s - ",
      obj->parent->stuckIn->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r",
      ((obj->parent->stuckIn->roomp &&
         !obj->parent->stuckIn->roomp->getName().empty())
          ? obj->parent->stuckIn->roomp->getNameNOC(ch).c_str()
          : "Room Unknown"),
      obj->parent->stuckIn->in_room);
    // object in object
  } else if (obj->parent) {
    sprintf(buf, "%s\n\r      - in ", obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s - ", obj->parent->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r",
      obj->parent->roomp ? obj->parent->roomp->getNameNOC(ch).c_str()
                         : "(Room Unkown)",
      obj->parent->in_room);
  } else if (obj->riding) {
    sprintf(buf, "%s\n\r      - on ", obj->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s - ", obj->riding->getNameNOC(ch).c_str());
    sprintf(buf + strlen(buf), "%s [%d]\n\r",
      obj->riding->roomp ? obj->riding->roomp->getNameNOC(ch).c_str()
                         : "(Room Unkown)",
      obj->riding->in_room);
  } else {
    sprintf(buf, "%s\n\r      - god doesn't even know where...\n\r",
      obj->getNameNOC(ch).c_str());
  }
  if (*buf)
    sb += buf;

  if (recurse) {
    if (obj->in_room != Room::NOWHERE)
      return;
    else if (dynamic_cast<TBeing*>(obj->parent))
      do_where_thing(ch, obj->parent, TRUE, sb);
    else if (obj->equippedBy)
      do_where_thing(ch, obj->equippedBy, TRUE, sb);
    else if (obj->stuckIn)
      do_where_thing(ch, obj->stuckIn, TRUE, sb);
    else if (obj->parent)
      do_where_thing(ch, obj->parent, TRUE, sb);
    else if (obj->riding)
      do_where_thing(ch, obj->riding, TRUE, sb);
  }
}

void TBeing::doWhere(const char* argument) {
  char namebuf[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char* nameonly;
  TBeing *i, *ch;
  TObj* k;
  Descriptor* d;
  int iNum, count, tcount = 0;
  sstring sb, tmp_sb, last_sb = "";
  bool dash = FALSE, gods = FALSE, found = false;
  unsigned int tot_found = 0;
  sstring tStString(argument), tStName(""), tStArg("");
  std::map<int, bool> vnums_notmatch;

  if (powerCheck(POWER_WHERE))
    return;

  strncpy(namebuf, argument, cElements(namebuf) - 1);
  tStArg = tStString.word(0);
  tStName = tStString.word(1);

  if (hasWizPower(POWER_WIZARD) && (GetMaxLevel() > MAX_MORT) &&
      (is_abbrev(tStArg, "engraved") || is_abbrev(tStArg, "owners"))) {
    count = 0;

    for (TObjIter iter = object_list.begin(); iter != object_list.end();
         ++iter) {
      k = *iter;
      if (k->getName().empty()) {
        vlogf(LOG_BUG,
          format(
            "Item without a name in object_list (doWhere) looking for %s") %
            namebuf);
        continue;
      }

      if (is_abbrev(tStArg, "owners")) {
        if (!k->owners)
          continue;

        const char* tTmpBuffer = k->owners;
        char tTmpString[256];

        while (tTmpBuffer && *tTmpBuffer) {
          tTmpBuffer =
            one_argument(tTmpBuffer, tTmpString, cElements(tTmpString));

          if (!*tTmpString)
            continue;

          if (!is_abbrev(tStName, tTmpString))
            continue;

          break;
        }

        if (!*tTmpString || !is_abbrev(tStName, tTmpString))
          continue;
      }

      if (is_abbrev(tStArg, "engraved")) {
        if (k->action_description.empty())
          continue;
        else if ((sscanf(k->action_description.c_str(),
                   "This is the personalized object of %s.", buf)) != 1)
          continue;
        else if (!is_abbrev(tStName, buf))
          continue;
      }

      sprintf(buf, "[%2d] ", ++count);
      sb += buf;

      if (++tot_found == 100) {
        sb += "Too many objects found.\n\r";
        break;
      }

      do_where_thing(this, k, 0, sb);
    }

    if (sb.empty())
      sendTo("Couldn't find any such object.\n\r");
    else if (desc)
      desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);

    return;
  }

  if (!*namebuf || (dash = (*namebuf == '-'))) {
    if (GetMaxLevel() <= MAX_MORT) {
      sendTo("What are you looking for?\n\r");
      return;
    } else {
      if (dash) {
        if (!strchr(namebuf, 'g')) {
          sendTo("Syntax : where -g\n\r");
          return;
        }
        gods = TRUE;
      }
      sb += "Players:\n\r--------\n\r";

      for (d = descriptor_list; d; d = d->next) {
        if ((ch = d->character) && canSeeWho(ch) &&
            (ch->in_room != Room::NOWHERE) && (!gods || ch->isImmortal())) {
          if (d->original)
            sprintf(buf, "%-20s - %s [%d] In body of %s\n\r",
              d->original->getNameNOC(ch).c_str(), ch->roomp->getName().c_str(),
              ch->in_room, ch->getNameNOC(ch).c_str());
          else
            sprintf(buf, "%-20s - %s [%d]\n\r", ch->getNameNOC(ch).c_str(),
              ch->roomp ? ch->roomp->getName().c_str() : "Bad room",
              ch->in_room);

          sb += buf;
        }
      }
      if (desc)
        desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
      return;
    }
  }
  if (isdigit(*namebuf)) {
    nameonly = namebuf;
    count = iNum = get_number(&nameonly);
  } else
    count = iNum = 0;

  *buf = '\0';

  for (i = character_list; i; i = i->next) {
    if (i->name.empty()) {
      vlogf(LOG_BUG,
        format(
          "Being without a name in character_list (doWhere) looking for %s") %
          namebuf);
      continue;
    }
    if (isname(namebuf, i->name) && canSeeWho(i) && canSee(i)) {
      if ((i->in_room != Room::NOWHERE) &&
          (isImmortal() || (i->roomp->getZoneNum() == roomp->getZoneNum()))) {
        if (!iNum || !(--count)) {
          if (!iNum) {
            sprintf(buf, "[%2d] ", ++count);
            sb += buf;
          }
          if (++tot_found > 500) {
            sb += "Too many creatures found.\n\r";
            break;
          }

          do_where_thing(this, i, TRUE, sb);
          *buf = 1;
          if (iNum != 0)
            break;
        }
        if (GetMaxLevel() <= MAX_MORT)
          break;
      }
    }

    // if it's a repair guy check his saved inventory
    if (i->spec == SPEC_REPAIRMAN) {
      struct dirent* dp;
      DIR* dfd;
      long time;
      int cost, ticket;
      unsigned char version;
      TObj* k;

      if ((dfd =
              opendir(((sstring)(format("mutable/repairs/%d") % i->mobVnum()))
                        .c_str()))) {
        while ((dp = readdir(dfd))) {
          if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
            continue;

          ticket = convertTo<int>(dp->d_name);

          if ((k = loadRepairItem(i, ticket, time, cost, version))) {
            if (k->getName().empty()) {
              vlogf(LOG_BUG, format("Item without a name in object_list "
                                    "(doWhere) looking for %s") %
                               namebuf);
              continue;
            }

            if (isname(namebuf, k->name) && canSee(k)) {
              if (!iNum || !(--count)) {
                if (!iNum) {
                  sb += format("[%2d] ") % ++count;
                }
                if (++tot_found > 500) {
                  sb += "Too many objects found.\n\r";
                  break;
                }

                sb += format("%s\n\r      - being repaired by %s\n\r") %
                      k->getNameNOC(this) % i->getName();

                if (iNum != 0)
                  break;
              }
            }

            delete k;
          }
        }
        closedir(dfd);
      }
    }
  }

  if (GetMaxLevel() > MAX_MORT) {
    for (TObjIter iter = object_list.begin(); iter != object_list.end();
         ++iter) {
      k = *iter;
      if (k->objVnum() != -1 && vnums_notmatch[k->objVnum()])
        continue;

      if (k->getName().empty()) {
        vlogf(LOG_BUG,
          format(
            "Item without a name in object_list (doWhere) looking for %s") %
            namebuf);
        continue;
      }

      if (isname(namebuf, k->name) && canSee(k)) {
        if (!iNum || !(--count)) {
          if (!iNum) {
            sprintf(buf, "[%2d] ", ++count);
          }
          if (++tot_found > 500) {
            sb += "Too many objects found.\n\r";
            break;
          }
          found = true;

          //; get description of where this thing is
          tmp_sb = "";
          do_where_thing(this, k, iNum != 0, tmp_sb);

          // last_sb is "", so this is the first item we've seen, init
          if (last_sb == "")
            last_sb = tmp_sb;

          // not the same as the last item, so print out last item
          if (tmp_sb != last_sb) {
            sb += buf;
            if (tcount > 1)
              sb += format("(%i) ") % tcount;
            sb += last_sb;
            last_sb = tmp_sb;
            tcount = 1;
          } else {
            --count;
            --tot_found;
            ++tcount;
          }

          *buf = 1;
          if (iNum != 0)
            break;
        }
      }

      if (!found && k->objVnum() != -1)
        vnums_notmatch[k->objVnum()] = true;
      else
        found = false;
    }

    sprintf(buf, "[%2d] ", ++count);
    sb += buf;
    if (tcount > 1)
      sb += format("(%i) ") % tcount;
    sb += last_sb;
  }

  TDatabase db(DB_SNEEZY);

  db.query(
    "select coalesce(rs.name, o.name) as objkeywords, coalesce(rs.short_desc, "
    "o.short_desc) as objname, s.shop_nr as shop_nr, m.short_desc as mobname "
    "from obj o, mob m, rent r left outer join rent_strung rs using (rent_id), "
    "shop s where o.vnum=r.vnum and m.vnum=s.keeper and s.shop_nr=r.owner and "
    "r.owner_type='shop'");

  while (db.fetchRow()) {
    if (isname(namebuf, db["objkeywords"])) {
      sb += format("[%2d] ") % ++count;
      if (++tot_found > 500) {
        sb += "Too many objects found.\n\r";
        break;
      }

      sb += format("%s\n\r      -  in the shop of %s (%i)\n\r") %
            db["objname"] % db["mobname"] % convertTo<int>(db["shop_nr"]);
    }
  }

  if (sb.empty())
    sendTo("Couldn't find any such thing.\n\r");
  else {
    if (desc)
      desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
  }
}

void TBeing::doLevels(const char* argument) {
  int i;
  classIndT Class;
  // int RaceMax;
  sstring sb;
  char buf[512], tString[256];

  for (; isspace(*argument); argument++)
    ;

  if (!*argument) {
    if (isSingleClass()) {
      int num = CountBits(getClass()) - 1;
      if (num < MIN_CLASS_IND || num >= MAX_CLASSES) {
        return;
      }
      Class = classIndT(num);
    } else {
      sendTo("You must supply a class!\n\r");
      return;
    }
  } else if (is_abbrev(argument, "mage"))
    Class = MAGE_LEVEL_IND;
  else if (is_abbrev(argument, "monk"))
    Class = MONK_LEVEL_IND;
  else {
    switch (*argument) {
      case 'C':
      case 'c':
        Class = CLERIC_LEVEL_IND;
        break;
      case 'F':
      case 'f':
      case 'W':
      case 'w':
        Class = WARRIOR_LEVEL_IND;
        break;
      case 'T':
      case 't':
        Class = THIEF_LEVEL_IND;
        break;
      case 'R':
      case 'r':
        Class = RANGER_LEVEL_IND;
        break;
      case 'p':
      case 'P':
      case 'd':  // deikhan
      case 'D':
        Class = DEIKHAN_LEVEL_IND;
        break;
      case 's':  // shaman
      case 'S':
        Class = SHAMAN_LEVEL_IND;
        break;

      default:
        sendTo(format("I don't recognize %s\n\r") % argument);
        return;
        break;
    }
  }
  // RaceMax = RacialMax[race->getRace()][Class];

  sb += format("The highest possible mortal level is %i.\n\r") % MAX_MORT;
  sprintf(buf, "Experience needed for level in class %s:\n\r\n\r",
    classInfo[Class].name.cap().c_str());
  sb += buf;

  ubyte cLvl = getLevel(Class);

  int nlevels = 100;  // = MAX_MORT
  sstring color = norm();

  for (i = 1; i <= nlevels / 4 + 1; i++) {
    int j = i + 1 * (nlevels / 4 + 1);
    int k = i + 2 * (nlevels / 4 + 1);
    int m = i + 3 * (nlevels / 4 + 1);

    if (i <= nlevels) {
      if (i > MAX_MORT)
        color = blue();
      else if (i > cLvl)
        color = orange();
      else
        color = green();

      sprintf(tString, "%.0f", getExpClassLevel(i));
      strncpy(tString, sstring(tString).comify().c_str(),
        cElements(tString) - 1);
      sprintf(buf, "%s[%2d]%s %s%13s%s%s", cyan(), i, norm(), color.c_str(),
        tString, norm(), " ");
      sb += buf;
    }
    if (j <= nlevels) {
      if (j > MAX_MORT)
        color = blue();
      else if (j > cLvl)
        color = orange();
      else
        color = green();

      sprintf(tString, "%.0f", getExpClassLevel(j));
      strncpy(tString, sstring(tString).comify().c_str(),
        cElements(tString) - 1);
      sprintf(buf, "%s[%2d]%s %s%13s%s%s", cyan(), j, norm(), color.c_str(),
        tString, norm(), " ");
      sb += buf;
    }
    if (k <= nlevels) {
      if (k > MAX_MORT)
        color = blue();
      else if (k > cLvl)
        color = orange();
      else
        color = green();

      sprintf(tString, "%.0f", getExpClassLevel(k));
      strncpy(tString, sstring(tString).comify().c_str(),
        cElements(tString) - 1);
      sprintf(buf, "%s[%2d]%s %s%13s%s%s", cyan(), k, norm(), color.c_str(),
        tString, norm(), " ");
      sb += buf;
    }
    if (m <= nlevels) {
      if (m > MAX_MORT)
        color = blue();
      else if (m > cLvl)
        color = orange();
      else
        color = green();

      sprintf(tString, "%.0f", getExpClassLevel(m));
      strncpy(tString, sstring(tString).comify().c_str(),
        cElements(tString) - 1);
      sprintf(buf, "%s[%2d]%s %s%13s%s%s", cyan(), m, norm(), color.c_str(),
        tString, norm(), "\n\r");
      sb += buf;
    } else {
      sb += "\n\r";
    }
  }
  sb += "\n\r";
  if (desc)
    desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
}

void TBeing::doWorld() {
  time_t ct, ot, tt;
  char *tmstr, *otmstr;
  int i;
  sstring str, buf;

  if (!desc)
    return;

  ot = Uptime;
  tt = ot + 3600 * desc->account->time_adjust;

  otmstr = asctime(localtime(&tt));
  *(otmstr + strlen(otmstr) - 1) = '\0';
  buf = format("%sStart time was:                      %s%s\n\r") % blue() %
        otmstr % norm();
  str += buf;

  ct = time(0) + 3600 * desc->account->time_adjust;

  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  buf = format("%sCurrent time is:                     %s%s\n\r") % blue() %
        tmstr % norm();
  str += buf;

  time_t upt = ct - tt;
  buf = format("%sUptime is:                           %s%s\n\r") % blue() %
        secsToString(upt) % norm();
  str += buf;

  long int total = 0, count = 0;
  for (i = 0; i < 10; ++i) {
    if (lag_info.lagcount[i]) {
      total += lag_info.lagtime[i];
      count++;
    }
  }
  buf = format("%sMachine Lag: Avg/Cur/High/Low        %ld/%ld/%ld/%ld%s\n\r") %
        blue() % (count ? total / count : 0) % lag_info.current %
        lag_info.high % lag_info.low % norm();
  str += buf;

  TDatabase db(DB_SNEEZY);
  db.query("select count(*) as count from rent");
  db.fetchRow();

  str +=
    format("Total number of rooms in world:               %ld\n\r") % roomCount;
  str += format("Total number of zones in world:               %d\n\r") %
         zone_table.size();
  str += format("Total number of distinct objects in world:%s    %d%s\n\r") %
         green() % obj_index.size() % norm();
  str += format("Total number of objects in game:%s              %ld%s\n\r") %
         green() % objCount % norm();
  str += format("Total number of objects in shops:%s             %ld%s\n\r") %
         green() % convertTo<int>(db["count"]) % norm();
  str += format("Total number of registered accounts:%s          %d%s\n\r") %
         blue() % AccountStats::account_number % norm();
  str += format("Total number of registered players:%s           %d%s\n\r") %
         blue() % AccountStats::player_count % norm();

  if (hasWizPower(POWER_WIZARD)) {
    str += format("Total number of 7-day active accounts:%s        %d%s\n\r") %
           blue() % AccountStats::active_account7 % norm();
    str += format("Total number of 7-day active players:%s         %d%s\n\r") %
           blue() % AccountStats::active_player7 % norm();
    str += format("Total number of 30-day active accounts:%s       %d%s\n\r") %
           blue() % AccountStats::active_account30 % norm();
    str += format("Total number of 30-day active players:%s        %d%s\n\r") %
           blue() % AccountStats::active_player30 % norm();
  }

  char timebuf[256];

  strncpy(timebuf, ctime(&stats.first_login), cElements(timebuf) - 1);
  timebuf[strlen(timebuf) - 1] = '\0';
  strcat(timebuf, ":");
  buf = format("Logins since %-32.32s %s%ld  (%ld per day)%s\n\r") % timebuf %
        blue() % stats.logins %
        (long)((double)stats.logins * SECS_PER_REAL_DAY /
               (time(0) - stats.first_login)) %
        norm();
  str += buf;

  int activemobcount = 0;
  for (unsigned int mobnum = 0; mobnum < mob_index.size(); mobnum++) {
    for (unsigned int zone = 0; zone < zone_table.size(); zone++) {
      if (mob_index[mobnum].virt <= zone_table[zone].top) {
        if (zone_table[zone].enabled)
          activemobcount++;
        break;
      }
    }
  }

  buf = format("Total number of distinct mobiles in world:%s    %d%s\n\r") %
        red() % activemobcount % norm();
  str += buf;

  int unkmobcount = 0;

  // trophymob requires a periodic background task to fill it
  // db.query("select count(*) as count from trophymob");

  // just use the inefficient query for now
  db.query("select count(distinct mobvnum) as count from trophy");
  if (db.fetchRow())
    unkmobcount = convertTo<int>(db["count"]);

  buf =
    format("Percent of distinct mobiles never killed: %s    %d%c (%i)%s\n\r") %
    red() % (100 - (int)(((float)unkmobcount / (float)activemobcount) * 100)) %
    '%' % (activemobcount - unkmobcount) % norm();
  str += buf;

  if (GetMaxLevel() >= GOD_LEVEL1) {
    buf = format("%sDistinct Mobs by level:%s\n\r") % blue() % norm();
    str += buf;
    buf = format(
            "%sL1-5  [%s%3d%s]  L6-10 [%s%3d%s]  L11-15[%s%3d%s]  "
            "L16-20[%s%3d%s]  L21-25 [%s%3d%s] L26-30  [%s%3d%s]%s\n\r") %
          norm() % purple() % stats.mobs_1_5 % norm() % purple() %
          stats.mobs_6_10 % norm() % purple() % stats.mobs_11_15 % norm() %
          purple() % stats.mobs_16_20 % norm() % purple() % stats.mobs_21_25 %
          norm() % purple() % stats.mobs_26_30 % norm() % norm();
    str += buf;
    buf = format(
            "%sL31-40[%s%3d%s]  L41-50[%s%3d%s]  L51-60[%s%3d%s]  "
            "L61-70[%s%3d%s]  L71-100[%s%3d%s] L101-127[%s%3d%s]%s\n\r") %
          norm() % purple() % stats.mobs_31_40 % norm() % purple() %
          stats.mobs_41_50 % norm() % purple() % stats.mobs_51_60 % norm() %
          purple() % stats.mobs_61_70 % norm() % purple() % stats.mobs_71_100 %
          norm() % purple() % stats.mobs_101_127 % norm() % norm();
    str += buf;
  }
  buf = format("Total number of monsters in game:%s             %ld%s\n\r") %
        red() % mobCount % norm();
  str += buf;

  buf = format("%sActual Mobs by level:%s\n\r") % purple() % norm();
  str += buf;

  buf = format(
          "%sL  1-  5  [%s%4u%s]  L  6- 10  [%s%4u%s]  L 11- 15  [%s%4u%s]  L "
          "16- 20  [%s%4u%s]\n\r") %
        norm() % purple() % stats.act_1_5 % norm() % purple() % stats.act_6_10 %
        norm() % purple() % stats.act_11_15 % norm() % purple() %
        stats.act_16_20 % norm();
  str += buf;

  buf = format(
          "%sL 21- 25  [%s%4u%s]  L 26- 30  [%s%4u%s]  L 31- 40  [%s%4u%s]  L "
          "41- 50  [%s%4u%s]\n\r") %
        norm() % purple() % stats.act_21_25 % norm() % purple() %
        stats.act_26_30 % norm() % purple() % stats.act_31_40 % norm() %
        purple() % stats.act_41_50 % norm();
  str += buf;

  buf = format(
          "%sL 51- 60  [%s%4u%s]  L 61- 70  [%s%4u%s]  L 71-100  [%s%4u%s]  "
          "L101-127  [%s%4u%s]\n\r") %
        norm() % purple() % stats.act_51_60 % norm() % purple() %
        stats.act_61_70 % norm() % purple() % stats.act_71_100 % norm() %
        purple() % stats.act_101_127 % norm();
  str += buf;

  desc->page_string(str);
}

const char* DescRatio(double f) {  // theirs / yours
  if (f >= 4.0)
    return ("Way better than yours");
  else if (f > 3.0)
    return ("More than three times yours");
  else if (f > 2.0)
    return ("More than twice yours");
  else if (f > 1.5)
    return ("More than half again greater than yours");
  else if (f > 1.4)
    return ("At least a third greater than yours");
  else if (f > 1.0)
    return ("About the same as yours");
  else if (f > .9)
    return ("A little worse than yours");
  else if (f > .6)
    return ("Worse than yours");
  else if (f > .4)
    return ("About half as good as yours");
  else if (f > .2)
    return ("Much worse than yours");
  else if (f > .1)
    return ("Inferior");
  else
    return ("Extremely inferior");
}

const char* DescDamage(double dam) {
  if (dam < 1.0)
    return ("Minimal Damage");
  else if (dam <= 2.0)
    return ("Slight damage");
  else if (dam <= 4.0)
    return ("A bit of damage");
  else if (dam <= 10.0)
    return ("A decent amount of damage");
  else if (dam <= 15.0)
    return ("A lot of damage");
  else if (dam <= 25.0)
    return ("A whole lot of damage");
  else if (dam <= 35.0)
    return ("A very large amount");
  else
    return ("A TON of damage");
}

const char* DescAttacks(double a) {
  if (a < 1.0)
    return ("Not many");
  else if (a < 2.0)
    return ("About average");
  else if (a < 3.0)
    return ("A few");
  else if (a < 5.0)
    return ("A lot");
  else if (a < 9.0)
    return ("Many");
  else
    return ("A whole bunch");
}

const char* DescMoves(double a) {
  if (a < .1)
    return ("very tired");
  else if (a < .3)
    return ("tired");
  else if (a < .5)
    return ("slightly tired");
  else if (a < .7)
    return ("decently rested");
  else if (a < 1.0)
    return ("well rested");
  else
    return ("totally rested");
}

const char* ac_for_score(int a) {
  if (a > 750)
    return ("scantily clothed");
  else if (a > 500)
    return ("heavily clothed");
  else if (a > 250)
    return ("slightly armored");
  else if (a > 0)
    return ("moderately armored");
  else if (a > -250)
    return ("armored rather heavily");
  else if (a > -500)
    return ("armored very heavily");
  else if (a > -1000)
    return ("armored extremely heavily");
  else
    return ("totally armored");
}

void TBeing::doClear(const char* argument) {
  if (!desc)
    return;

  TDatabase db(DB_SNEEZY);
  if (!*argument) {
    sendTo("Clear which alias?\n\r");
    return;
  } else if (is_abbrev(argument, "all")) {
    db.query("delete from alias where player_id = %i", getPlayerID());
    sendTo("Ok. All Aliases cleared\n\r");
    return;
  }

  int idx = convertTo<int>(argument) - 1;

  if (idx >= 0 && idx < static_cast<int>(desc->alias.size())) {
    auto it = desc->alias.begin();
    for (int i = 0; i < idx; ++i)
      ++it;
    sstring word = it->first;
    desc->alias.erase(it);
    db.query("delete from alias where player_id = %i and word = '%s'",
      getPlayerID(), word.c_str());
    sendTo(
      format("Ok. Alias %d = %s has been removed.\n\r") % (idx + 1) % word);
    return;
  } else {
    sendTo("Syntax: clear <alias number>\n\r");
    return;
  }
}

void TBeing::doAlias(const char* argument) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];

  if (!desc)
    return;

  if (desc->m_bIsClient) {
    sendTo(
      "Use the client aliases. See client help file for #alias command and "
      "options menu.\n\r");
    return;
  }

  size_t maxlen = 0;
  std::for_each(desc->alias.begin(), desc->alias.end(),
    [&](auto it) { maxlen = std::max(maxlen, it.first.length()); });

  if (!*argument) {
    sendTo("Your list of aliases.....\n\r");
    int i = 0;
    for (const auto& alias : desc->alias) {
      ++i;
      assert(static_cast<int>(maxlen) - alias.first.length() >= 0);
      std::string padding(maxlen - alias.first.length(), ' ');
      sendTo(format("%2d) %s%s %s %s\n\r") % i % alias.first % padding %
             (ansi() ? ANSI_BLUE_BAR : "|") % alias.second);
    }
    return;
  }
  half_chop(argument, arg1, arg2);
  if ((!*arg2)) {
    sendTo("You need a second argument.\n\r");
    return;
  }
  if (!strcmp(arg2, "clear")) {
    sendTo("You could get in a loop like that!\n\r");
    return;
  }
  if (!strcmp(arg1, "clear")) {
    sendTo("Use the 'clear' command to clear aliases");
    return;
  }

  if (!strcmp(arg1, arg2)) {
    sendTo("You could get in a loop like that!\n\r");
    return;
  }

  TDatabase db(DB_SNEEZY);
  if (db.query("insert into alias (player_id, word, command) values (%i, '%s', "
               "'%s') on duplicate key update command = '%s'",
        getPlayerID(), arg1, arg2, arg2)) {
    desc->alias[arg1] = arg2;
    sendTo(format("Setting alias %s to %s\n\r") % arg1 % arg2);
  } else {
    sendTo("There was an error. Please contact a coder.\r\n");
  }
}

sstring TObj::equip_condition(int amt) const {
  double p;

  if (!getMaxStructPoints()) {
    sstring a("<C>brand new<1>");
    return a;
  } else if (amt == -1)
    p = ((double)getStructPoints()) / ((double)getMaxStructPoints());
  else
    p = ((double)amt) / ((double)getMaxStructPoints());

  if (p > 1.0) {
    // shouldn't happen theoretically
    sstring a("<W>better than new<1>");
    return a;
  } else if (p == 1) {
    sstring a("<C>brand new<1>");
    return a;
  } else if (p > .9) {
    sstring a("<c>like new<1>");
    return a;
  } else if (p > .8) {
    sstring a("<B>excellent<1>");
    return a;
  } else if (p > .7) {
    sstring a("<b>very good<1>");
    return a;
  } else if (p > .6) {
    sstring a("<P>good<1>");
    return a;
  } else if (p > .5) {
    sstring a("<p>fine<1>");
    return a;
  } else if (p > .4) {
    sstring a("<G>fair<1>");
    return a;
  } else if (p > .3) {
    sstring a("<g>poor<1>");
    return a;
  } else if (p > .2) {
    sstring a("<y>very poor<1>");
    return a;
  } else if (p > .1) {
    sstring a("<o>bad<1>");
    return a;
  } else if (p > .001) {
    sstring a("<R>very bad<1>");
    return a;
  } else {
    sstring a("<r>destroyed<1>");
    return a;
  }
}

void TBeing::doMotd(const char* argument) {
  for (; isspace(*argument); argument++)
    ;

  if (!desc)
    return;

  if (!*argument || !argument) {
    sendTo("Today's message of the day is :\n\r\n\r");
    desc->sendMotd(GetMaxLevel() > MAX_MORT);
    return;
#if 0
sendTo("Feature disabled, bug Batopr.\n\r");
  } else if (is_abbrev(argument, "message") && (GetMaxLevel() >= GOD_LEVEL1)) {
    TThing *t_note = searchLinkedListVis(this, "note", stuff);
    TObj *note = dynamic_cast<TObj *>(t_note);
    if (note) {
      if (!note->action_description) {
        sendTo("Your note has no message for the new motd!\n\r");
        return;
      } else {
        strcpy(motd, note->action_description);
        return;
      }
    } else {
      sendTo("You need a note with what you want the message to be in you inventory.\n\r");
      return;
    }
#endif
  }
}

const char* LimbHealth(double a) {
  if (a < .1)
    return ("<R>in extremely bad shape<Z>");
  else if (a < .3)
    return ("<o>badly beaten<z>");
  else if (a < .5)
    return ("<O>moderately wounded<z>");
  else if (a < .7)
    return ("<g>mildly wounded<z>");
  else if (a < .99)
    return ("<g>slightly wounded<z>");
  else if (a >= 1.00)
    return ("<c>in perfect condition<Z>");
  else
    return ("in near perfect condition");
}

const sstring TBeing::slotPlurality(int limb) const {
  char buf[10];

  if ((race->getBodyType() == BODY_BIRD) && (limb == WEAR_WAIST)) {
    // tail feathers
    sprintf(buf, "are");
  } else if ((race->getBodyType() == BODY_TREE) &&
             ((limb == WEAR_ARM_R) || (limb == WEAR_ARM_L))) {
    // branches
    sprintf(buf, "are");
  } else
    sprintf(buf, "is");

  return buf;
}

void TBeing::doLimbs(const sstring& argument) {
  wearSlotT i;
  sstring who;
  affectedData* aff;
  TThing* t;
  TBeing* v = NULL;

  if (!argument.empty()) {
    if (!(v = get_char_room_vis(this, argument))) {
      if (!(v = fight())) {
        sendTo("Check whose limbs?\n\r");
        return;
      }
    }
    if (!sameRoom(*v)) {
      sendTo("That person doesn't seem to be around.\n\r");
      return;
    }
  }

  if (!v || v == this) {
    v = this;
    who = "Your";
    sendTo("You evaluate your limbs and their health.\n\r");
  } else {
    who = sstring(v->hshr()).cap();
    sendTo(COLOR_BASIC,
      format("You evaluate %s's limbs and their health.\n\r") % v->getName());
  }

  bool found = false;
  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (i == HOLD_RIGHT || i == HOLD_LEFT)
      continue;
    if (!v->slotChance(i))
      continue;
    double perc =
      (double)v->getCurLimbHealth(i) / (double)v->getMaxLimbHealth(i);

    if (v->isLimbFlags(i, PART_MISSING)) {
      sendTo(COLOR_BASIC, format("<R>%s %s%s%s %s missing!<Z>\n\r") % who %
                            red() % v->describeBodySlot(i) % norm() %
                            v->slotPlurality(i));
      found = TRUE;
      continue;
    }
    if (perc < 1.00) {
      sendTo(COLOR_BASIC, format("%s %s %s %s.\n\r") % who %
                            v->describeBodySlot(i) % v->slotPlurality(i) %
                            LimbHealth(perc));
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_USELESS)) {
      sendTo(COLOR_BASIC, format("%s %s%s%s %s <O>useless<Z>!\n\r") % who %
                            red() % v->describeBodySlot(i) % norm() %
                            v->slotPlurality(i));
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_PARALYZED)) {
      sendTo(COLOR_BASIC, format("%s %s%s%s %s <O>paralyzed<Z>!\n\r") % who %
                            red() % v->describeBodySlot(i) % norm() %
                            v->slotPlurality(i));
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_BRUISED)) {
      sendTo(COLOR_BASIC, format("%s %s%s%s %s <p>bruised<Z>!\n\r") % who %
                            red() % v->describeBodySlot(i) % norm() %
                            v->slotPlurality(i));
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_BLEEDING)) {
      sendTo(COLOR_BASIC, format("%s %s%s%s %s <R>bleeding profusely<Z>!\n\r") %
                            who % red() % v->describeBodySlot(i) % norm() %
                            v->slotPlurality(i));
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_INFECTED)) {
      sendTo(COLOR_BASIC,
        format("%s %s%s%s %s infected with many <o>germs<1>!\n\r") % who %
          red() % v->describeBodySlot(i) % norm() % v->slotPlurality(i));
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_BROKEN)) {
      sendTo(COLOR_BASIC, format("%s %s%s%s %s broken!\n\r") % who % red() %
                            v->describeBodySlot(i) % norm() %
                            v->slotPlurality(i));
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_LEPROSED)) {
      sendTo(COLOR_BASIC, format("Leprosy has set into %s %s%s%s!\n\r") %
                            who.uncap() % red() % v->describeBodySlot(i) %
                            norm());
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_GANGRENOUS)) {
      sendTo(COLOR_BASIC, format("<k>Gangrene<1> has set into %s %s%s%s!\n\r") %
                            who.uncap() % red() % v->describeBodySlot(i) %
                            norm());
      found = TRUE;
    }
    if (v->isLimbFlags(i, PART_TRANSFORMED)) {
      sendTo(COLOR_BASIC, format("%s %s%s%s has been transformed!\n\r") % who %
                            red() % v->describeBodySlot2(i) % norm());
      found = TRUE;
    }
    if ((t = v->getStuckIn(i))) {
      if (canSee(t)) {
        sendTo(COLOR_OBJECTS, format("%s is sticking out of %s %s!\n\r") %
                                sstring(t->getName()).cap() % who.uncap() %
                                v->describeBodySlot(i));
      }
    }
  }

  if (v == this)
    who = "You";
  else {
    who = sstring(v->hssh()).cap();
  }

  if (v->affected) {
    for (aff = v->affected; aff; aff = aff->next) {
      if (aff->type == AFFECT_DISEASE) {
        if (!aff->level) {
          sendTo(format("%s %s %s.\n\r") % who %
                 ((v == this) ? "have" : "has") %
                 DiseaseInfo[affToDisease(*aff)].name);
          found = TRUE;
        }
      }
    }
  }

  if (!found) {
    if (v == this)
      who = "your";
    else
      who = v->hshr();

    sendTo(format("All %s limbs are perfectly healthy!\n\r") % who);
  }
}

void TBeing::genericEvaluateItem(const TThing* obj) {
  int learn = getSkillValue(SKILL_EVALUATE);
  if (learn <= 0) {
    sendTo("You are not sufficiently knowledgeable about evaluation.\n\r");
    return;
  }

  describeObject(obj);
}

void TThing::evaluateMe(TBeing* ch) const {}

void TMagicItem::evaluateMe(TBeing* ch) const {
  int learn = ch->getSkillValue(SKILL_EVALUATE);
  int learn2 = learn;
  int learn3 = learn;

  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_EVALUATE, 7);

  // adjust for knowledge about magic stuff
  if (ch->hasClass(CLASS_RANGER)) {
    learn *= ch->getClassLevel(CLASS_RANGER);
    learn /= 200;
  }
  if (ch->hasClass(CLASS_SHAMAN)) {
    learn2 *= ch->getClassLevel(CLASS_SHAMAN);
    learn2 /= 50;
  }
  learn3 *= ch->getSkillValue(SPELL_IDENTIFY);
  learn3 /= 100;

  // take the largest value
  learn2 = max(learn2, learn3);
  learn = max(learn, learn2);

  if (learn > 10)
    ch->describeMagicLevel(this, learn);

  if (learn > 15) {
    ch->describeMagicLearnedness(this, learn);
  }

  if (learn > 50) {
    ch->describeMagicSpell(this, learn);
  }
}

void TBeing::doEvaluate(const char* argument) {
  char arg[160];
  TThing* obj;
  int count = 0, rNatureCount = 0;

  strncpy(arg, argument, cElements(arg) - 1);
  if (!*arg) {
    sendTo("Evaluate what?\n\r");
    return;
  }
  if (is_abbrev(arg, "room")) {
    // room evaluation
    // spit out some info giving hints as to sector type and some room flags
    // get it very general
    if (!roomp) {
      sendTo("You have no idea where you are, do you...\n\r");
      vlogf(LOG_BUG,
        format("Player without room called evaluate room.  [%s]") % getName());
      return;
    }
    if (isAffected(AFF_BLIND) && !isImmortal() && !isAffected(AFF_TRUE_SIGHT) &&
        !isAffected(AFF_CLARITY)) {
      sendTo("You can't see a damn thing -- you're blinded!\n\r");
      return;
    }
    // go over the basic sector groups
    if (roomp->isCitySector())
      sendTo("You are in a city.\n\r");
    else if (roomp->isFlyingSector())
      sendTo("You are floating around in mid-air.\n\r");
    else if (roomp->isVertSector())
      sendTo("The ground here is too sheer to stand on.\n\r");
    else if (roomp->isUnderwaterSector())
      sendTo("You are underwater.\n\r");
    else if (roomp->isSwampSector())
      sendTo("You are in a swamp.\n\r");
    else if (roomp->isBeachSector())
      sendTo("You are on a beach.\n\r");
    else if (roomp->isHillSector())
      sendTo("You are on a hill.\n\r");
    else if (roomp->getSectorType() == SECT_VOLCANO_LAVA)
      sendTo("You are on a hill.\n\r");
    else if (roomp->getSectorType() == SECT_DESERT)
      sendTo("You are in a desert.\n\r");
    else if (roomp->isMountainSector())
      sendTo("You are on a mountain.\n\r");
    else if (roomp->getSectorType() == SECT_DEAD_WOODS)
      sendTo("You are in a dead forest.\n\r");
    else if (roomp->isForestSector()) {
      if (roomp->getSectorType() == SECT_JUNGLE)
        sendTo("You are in a jungle.\n\r");
      else if (roomp->getSectorType() == SECT_RAINFOREST_ROAD)
        sendTo("You are on a road in a rainforest.\n\r");
      else if (roomp->getSectorType() == SECT_RAINFOREST)
        sendTo("You are in a rainforest.\n\r");
      else if (roomp->getSectorType() == SECT_TEMPERATE_FOREST_ROAD ||
               roomp->getSectorType() == SECT_ARCTIC_FOREST_ROAD)
        sendTo("You are on a road in a forest.\n\r");
      else
        sendTo("You are in a forest.\n\r");
    } else if (roomp->getSectorType() == SECT_FIRE_ATMOSPHERE)
      sendTo("You are up in the air surrounded by fire.\n\r");
    else if (roomp->isAirSector())
      sendTo("You are up in the air.\n\r");
    else if (roomp->isOceanSector()) {
      if (roomp->getSectorType() == SECT_ICEFLOW)
        sendTo("You are in icy waters.\n\r");
      else
        sendTo("You are on a body of water.\n\r");
      if (roomp->getRiverSpeed() >= 30)
        sendTo(format("The current flows swiftly towards the %s.\n\r") %
               dirs[roomp->getRiverDir()]);
      else if (roomp->getRiverSpeed() >= 15)
        sendTo(format("The current flows steadily towards the %s.\n\r") %
               dirs[roomp->getRiverDir()]);
      else if (roomp->getRiverSpeed() > 0)
        sendTo(format("The current gently flows towards the %s.\n\r") %
               dirs[roomp->getRiverDir()]);
      else
        sendTo("There is no noticable current.\n\r");
    } else if (roomp->isRiverSector()) {
      if (roomp->getSectorType() == SECT_ARCTIC_RIVER_SURFACE)
        sendTo("You are on an icy river.\n\r");
      else
        sendTo("You are on a river.\n\r");
      if (roomp->getRiverSpeed() >= 30)
        sendTo(format("The current flows swiftly towards the %s.\n\r") %
               dirs[roomp->getRiverDir()]);
      else if (roomp->getRiverSpeed() >= 15)
        sendTo(format("The current flows steadily towards the %s.\n\r") %
               dirs[roomp->getRiverDir()]);
      else if (roomp->getRiverSpeed() > 0)
        sendTo(format("The current gently flows towards the %s.\n\r") %
               dirs[roomp->getRiverDir()]);
      else
        sendTo("There is no noticable current.\n\r");

    } else if (roomp->getSectorType() == SECT_TEMPERATE_CAVE ||
               roomp->getSectorType() == SECT_TROPICAL_CAVE ||
               roomp->getSectorType() == SECT_ARCTIC_CAVE) {
      sendTo("You are spelunking.\n\r");
    } else if (roomp->isIndoorSector())
      sendTo("You are in a building.\n\r");
    else if (roomp->getSectorType() == SECT_INSIDE_MOB)
      sendTo("You are inside something very strange, something alive.\n\r");
    else if (roomp->getSectorType() == SECT_SOLID_ROCK)
      sendTo("You are in an area surrounded by solid rock.\n\r");
    else if (roomp->getSectorType() == SECT_SOLID_ICE)
      sendTo("You are in an area surrounded by solid ice.\n\r");
    else if (roomp->getSectorType() == SECT_FIRE)
      sendTo("You are in a room of fire.\n\r");
    else if (roomp->isRoadSector())
      sendTo("You are on a well traveled road.\n\r");
    else if (roomp->isNatureSector()) {
      // catch a bunch of other naturey sectors - plains, grasslands, savannah,
      // veldt
      sendTo("You are in an open area.\n\r");
    } else if (roomp->getSectorType() == SECT_ASTRAL_ETHREAL) {
      sendTo("You are in an area without substance.\n\r");
    } else {
      // fell all the way through
      vlogf(LOG_MISC,
        format(
          "Sector type fell through on eval room. Room: (%d) Sector: (%d)") %
          roomp->in_room % ((int)roomp->getSectorType()));
      sendTo("Unrecognized room type...\n\r");
    }

    // indoors, not a cave
    if (roomp->isRoomFlag(ROOM_INDOORS) && !roomp->isIndoorSector() &&
        roomp->getSectorType() != SECT_INSIDE_MOB)
      sendTo("You are indoors.\n\r");
    else if (!roomp->isIndoorSector() &&
             roomp->getSectorType() != SECT_INSIDE_MOB)
      sendTo("You are outside.\n\r");

    // climatey stuff
    if (roomp->isArcticSector())
      sendTo("It is freezing cold here.\n\r");
    else if (roomp->isTropicalSector())
      sendTo("It is hot and humid here.\n\r");
    else if (roomp->getSectorType() == SECT_DESERT)
      sendTo("It is hot and dry here.\n\r");

    // some room flag messages
    if (roomp->isRoomFlag(ROOM_ON_FIRE))
      sendTo("There is an out-of-control fire here.\n\r");
    if (roomp->isRoomFlag(ROOM_FLOODED))
      sendTo("The room is flooded with water.\n\r");

    int wetness = getRoomWetness(roomp);
    if (wetness != 0)  // show wetness
      sendTo(format("The room is %s.\n\r") % Weather::describeWet(wetness));

    // advanced adventuring conditions
    // should probably make a more modular isGreatOutdoorsSector() check or
    // something...
    if (doesKnowSkill(SKILL_ENCAMP)) {
      bool can_do = TRUE;
      if (roomp->isCitySector() ||
          !(roomp->isForestSector() || roomp->isBeachSector() ||
            roomp->isHillSector() || roomp->isMountainSector() ||
            roomp->isNatureSector() || roomp->isRoadSector() ||
            roomp->isSwampSector() || roomp->isArcticSector() ||
            roomp->getSectorType() == SECT_TEMPERATE_CAVE ||
            roomp->getSectorType() == SECT_TROPICAL_CAVE ||
            roomp->getSectorType() == SECT_ARCTIC_CAVE)) {
        can_do = FALSE;
      } else if (roomp->isFlyingSector() || roomp->isVertSector() ||
                 roomp->isUnderwaterSector() || roomp->isAirSector() ||
                 roomp->isOceanSector() || roomp->isRiverSector()) {
        can_do = FALSE;
      } else if (roomp->getSectorType() == SECT_TEMPERATE_BUILDING ||
                 roomp->getSectorType() == SECT_TROPICAL_BUILDING ||
                 roomp->getSectorType() == SECT_ARCTIC_BUILDING ||
                 (roomp->isRoomFlag(ROOM_INDOORS) &&
                   !(roomp->getSectorType() == SECT_TEMPERATE_CAVE ||
                     roomp->getSectorType() == SECT_TROPICAL_CAVE ||
                     roomp->getSectorType() == SECT_ARCTIC_CAVE)) ||
                 roomp->isRoomFlag(ROOM_FLOODED) ||
                 roomp->isRoomFlag(ROOM_ON_FIRE)) {
        can_do = FALSE;
      } else if (roomp->isRoomFlag(ROOM_NO_FLEE) ||
                 roomp->isRoomFlag(ROOM_NO_ESCAPE) ||
                 roomp->isRoomFlag(ROOM_NO_HEAL) ||
                 roomp->isRoomFlag(ROOM_HAVE_TO_WALK)) {
        can_do = FALSE;
      }
      if (can_do)
        sendTo("You may camp here.\n\r");
    }

    if (doesKnowSkill(SKILL_FORAGE)) {
      bool can_do = TRUE;
      if (roomp->isCitySector() ||
          !(roomp->isForestSector() || roomp->isBeachSector() ||
            roomp->isHillSector() || roomp->isMountainSector() ||
            roomp->isNatureSector() || roomp->isRoadSector() ||
            roomp->isSwampSector() || roomp->isArcticSector() ||
            roomp->getSectorType() == SECT_TEMPERATE_CAVE ||
            roomp->getSectorType() == SECT_TROPICAL_CAVE ||
            roomp->getSectorType() == SECT_ARCTIC_CAVE)) {
        can_do = FALSE;
      } else if (roomp->isFlyingSector() || roomp->isVertSector() ||
                 roomp->isUnderwaterSector() || roomp->isAirSector() ||
                 roomp->isOceanSector() || roomp->isRiverSector()) {
        can_do = FALSE;
      } else if (roomp->getSectorType() == SECT_TEMPERATE_BUILDING ||
                 roomp->getSectorType() == SECT_TROPICAL_BUILDING ||
                 roomp->getSectorType() == SECT_ARCTIC_BUILDING ||
                 (roomp->isRoomFlag(ROOM_INDOORS) &&
                   !(roomp->getSectorType() == SECT_TEMPERATE_CAVE ||
                     roomp->getSectorType() == SECT_TROPICAL_CAVE ||
                     roomp->getSectorType() == SECT_ARCTIC_CAVE)) ||
                 roomp->isRoomFlag(ROOM_FLOODED) ||
                 roomp->isRoomFlag(ROOM_ON_FIRE)) {
        can_do = FALSE;
      }
      if (can_do)
        sendTo("You may forage here.\n\r");
    }

    if (doesKnowSkill(SKILL_DIVINATION)) {
      bool can_do = TRUE;
      if (!(roomp->isForestSector() || roomp->isBeachSector() ||
            roomp->isHillSector() || roomp->isMountainSector() ||
            roomp->isNatureSector() || roomp->isRoadSector() ||
            roomp->isSwampSector())) {
        can_do = FALSE;
      } else if (roomp->isIndoorSector() || roomp->isArcticSector() ||
                 roomp->isRoomFlag(ROOM_INDOORS) ||
                 roomp->isRoomFlag(ROOM_FLOODED) ||
                 roomp->isRoomFlag(ROOM_ON_FIRE)) {
        can_do = FALSE;
      }
      if (can_do)
        sendTo("You may divine for water here.\n\r");
    }

    if (!hasClass(CLASS_RANGER))
      return;

    for (int Runner = MIN_DIR; Runner < MAX_DIR; Runner++)
      if (Runner != DIR_UP && Runner != DIR_DOWN && roomp->dir_option[Runner] &&
          real_roomp((count = roomp->dir_option[Runner]->to_room))) {
        if (real_roomp(count)->notRangerLandSector())
          rNatureCount--;
        else
          rNatureCount++;
      }

    if (roomp->notRangerLandSector()) {
      if (rNatureCount > 6)
        sendTo("You are getting real close to nature, you can feel it.\n\r");
      else if (rNatureCount > 3)
        sendTo("You are not that far, just a little more.\n\r");
      else if (rNatureCount > 0)
        sendTo("You feel really out of touch from nature here...\n\r");
      else
        sendTo("You don't feel very much in touch with nature here.\n\r");
    } else {
      if (rNatureCount > 6)
        sendTo("Now this is nature, you feel right at home.\n\r");
      else if (rNatureCount > 3)
        sendTo("The soothing sounds of nature rush over your body.\n\r");
      else if (rNatureCount > 0)
        sendTo("Although not that dense, it's still home.\n\r");
      else
        sendTo("It's nature, but it sure doesn't feel like nature...\n\r");
    }
  } else {
    wearSlotT j;
    if (!(obj = get_thing_in_equip(this, arg, equipment, &j, TRUE, &count))) {
      if (!(obj = searchLinkedListVis(this, arg, stuff, &count))) {
        sendTo(format("You do not seem to have the '%s'.\n\r") % arg);
        return;
      }
    }
    genericEvaluateItem(obj);
  }
}

void TTool::describeCondition(const TBeing*) const {
  // intentionally blank
}

void TObj::describeCondition(const TBeing* ch) const {
  ch->sendTo(COLOR_OBJECTS,
    format("It is in %s condition.\n\r") % equip_condition(-1));
}

void TThing::describeContains(const TBeing* ch) const {
  if (!stuff.empty())
    ch->sendTo(COLOR_OBJECTS,
      format("%s seems to have something in it...\n\r") %
        sstring(getName()).cap());
}

void TBaseCup::describeContains(const TBeing* ch) const {
  if (getDrinkUnits())
    ch->sendTo(COLOR_OBJECTS,
      format("%s seems to have some %s%s liquid in it...\n\r") %
        sstring(getName()).cap() %
        (isDrinkConFlag(DRINK_FROZEN) ? "<C>frozen<1> " : "") %
        liquidInfo[getDrinkType()]->color);
}

void TFood::describeCondition(const TBeing* ch) const {
  ch->sendTo(COLOR_OBJECTS,
    format("It is in %s condition.\n\r") % equip_condition(-1));
}

void TFood::describeObjectSpecifics(const TBeing* ch) const {
  if (isFoodFlag(FOOD_SPOILED))
    act("$p looks a bit spoiled.", FALSE, ch, this, 0, TO_CHAR);
}

void TCorpse::describeObjectSpecifics(const TBeing* ch) const {
  if (isCorpseFlag(CORPSE_NO_SKIN))
    act("$p doesn't appear to have any skin left on it.", FALSE, ch, this, 0,
      TO_CHAR);
  else if (isCorpseFlag(CORPSE_HALF_SKIN))
    act("$p appears to have been half skinned.", FALSE, ch, this, 0, TO_CHAR);
  if (isCorpseFlag(CORPSE_NO_DISSECT))
    act("$p appears to have been dissected already.", FALSE, ch, this, 0,
      TO_CHAR);
  if (isCorpseFlag(CORPSE_NO_BUTCHER))
    act("$p doesn't appear to have any meat left on it.", FALSE, ch, this, 0,
      TO_CHAR);
  else if (isCorpseFlag(CORPSE_HALF_BUTCHERED))
    act("$p appears to have been half butchered.", FALSE, ch, this, 0, TO_CHAR);
}

void TSymbol::describeObjectSpecifics(const TBeing* ch) const {
  double diff;
  int attuneCode = 1;
  factionTypeT sym_faction = getSymbolFaction();

  if (attuneCode) {
    switch (sym_faction) {
      case FACT_NONE:
        ch->sendTo(COLOR_OBJECTS,
          format("You can tell that %s has been sanctified but it bears no "
                 "insignia.\n\r") %
            sstring(getName()).cap());
        break;
      case FACT_BROTHERHOOD:
        ch->sendTo(COLOR_OBJECTS, format("%s has the sign of the Brotherhood "
                                         "of Galek stamped upon it.\n\r") %
                                    sstring(getName()).cap());
        break;
      case FACT_CULT:
        ch->sendTo(COLOR_OBJECTS,
          format("%s has been sanctified and bears the insignia of the Cult of "
                 "the Logrus.\n\r") %
            sstring(getName()).cap());
        break;
      case FACT_SNAKE:
        ch->sendTo(COLOR_OBJECTS, format("%s has been sanctified and branded "
                                         "with the image of a snake.\n\r") %
                                    sstring(getName()).cap());
        break;
      case MAX_FACTIONS:
      case FACT_UNDEFINED:
        ch->sendTo(COLOR_OBJECTS,
          format("%s is inert and has not been sanctified for use.\n\r") %
            sstring(getName()).cap());
    }
  }

  if (getSymbolMaxStrength()) {
    if (getSymbolCurStrength() == getSymbolMaxStrength()) {
      ch->sendTo(COLOR_OBJECTS,
        format(
          "%s has all of its original strength, it has not been used.\n\r") %
          sstring(getName()).cap());

    } else {
      diff = (double)((double)getSymbolCurStrength() /
                      (double)getSymbolMaxStrength());
      ch->sendTo(COLOR_OBJECTS,
        format("You can tell that %s has %s strength left.\n\r") % getName() %
          ((diff < .20) ? "very little"
                        : ((diff < .50) ? "some"
                                        : ((diff < .75) ? "a good bit of"
                                                        : "most of its"))));
    }
  } else {
    ch->sendTo(COLOR_OBJECTS, format("%s has none of its strength left.\n\r") %
                                sstring(getName()).cap());
  }
}

void TTool::describeObjectSpecifics(const TBeing* ch) const {
  double diff;

  if (getToolMaxUses())
    diff = ((double)getToolUses()) / ((double)getToolMaxUses());
  else
    diff = 1.00;

  ch->sendTo(COLOR_OBJECTS,
    format("It appears %s is %s.\n\r") % getName() %
      ((diff <= 0.0)
          ? "totally gone"
          : ((diff >= 1.0)
                ? "brand new"
                : ((diff >= 0.8)
                      ? "almost new"
                      : ((diff >= 0.6)
                            ? "like new"
                            : ((diff >= 0.4)
                                  ? "half gone"
                                  : ((diff >= 0.2) ? "more than half gone"
                                                   : "almost gone")))))));
}

void TObj::describeMe(TBeing* ch) const {
  char buf[80], buf2[256];
  char capbuf[256];
  char name_buf[80];
  TThing* t2;

  strncpy(buf, material_nums[getMaterial()].mat_name, cElements(buf) - 1);
  strncpy(buf2, ch->objs(this), cElements(buf2) - 1);
  ch->sendTo(COLOR_OBJECTS,
    format("%s is %s made of %s.\n\r") % sstring(buf2).cap() %
      ItemInfo[itemType()]->common_name % sstring(buf).uncap());

  if (ch->isImmortal() || canWear(ITEM_WEAR_TAKE)) {
    // weight >= 1.0
    float wgt = getTotalWeight(TRUE);

    sstring volumeBuf = volumeDisplay(getVolume());

    if (compareWeights(wgt, 1.0) != 1)
      ch->sendTo(
        format("It weighs about %d pound%s and occupies roughly %s.\n\r") %
        (int)wgt % ((((int)wgt) == 1) ? "" : "s") % volumeBuf);
    else
      ch->sendTo(
        format("It weighs about %d drechel%s and occupies roughly %s.\n\r") %
        getDrechels(TRUE) % ((getDrechels(TRUE) == 1) ? "" : "s") % volumeBuf);
  }
  describeCondition(ch);
  if (isObjStat(ITEM_GLOW))
    act("It is <o>glowing<1>.", FALSE, ch, 0, 0, TO_CHAR);
  if (isObjStat(ITEM_BURNING))
    act("It is <r>burning<1>.", FALSE, ch, 0, 0, TO_CHAR);
  if (isObjStat(ITEM_CHARRED))
    act("It is <k>charred<1>.", FALSE, ch, 0, 0, TO_CHAR);
  if (isObjStat(ITEM_RUSTY))
    act("It is <o>rusty<1>.", FALSE, ch, 0, 0, TO_CHAR);
  describeContains(ch);

  if (dynamic_cast<TBeing*>(rider)) {
    act("$p is being used by $N.", FALSE, ch, this, horseMaster(), TO_CHAR);
    for (t2 = rider->nextRider; t2; t2 = t2->nextRider) {
      if (t2 == horseMaster())
        continue;
      if (!dynamic_cast<TBeing*>(t2))
        continue;
      act("$p is also being used by $N.", FALSE, ch, this, t2, TO_CHAR);
    }
  }
  if (!action_description.empty()) {
    strncpy(capbuf, action_description.c_str(), cElements(capbuf) - 1);
    if ((sscanf(capbuf, "This is the personalized object of %s.", name_buf)) ==
        1)
      sendTo(
        format("A monogram on it indicates it belongs to %s.\n\r") % name_buf);
  }
  describeObjectSpecifics(ch);
  evaluateMe(ch);
}

void TBeing::describeObject(const TThing* t) { t->describeMe(this); }

sstring TBeing::describeSharpness(const TThing* obj) const {
  return obj->describeMySharp(this);
}

sstring TThing::describeMySharp(const TBeing*) const {
  char buf[256];
  sprintf(buf, "%s is not a weapon", getName().c_str());
  return buf;
}

sstring TBeing::describePointiness(const TBaseWeapon* obj) const {
  char buf[256];
  char sharpbuf[80];
  int maxsharp = obj->getMaxSharp();
  int sharp = obj->getCurSharp();
  double diff;

  if (!maxsharp)
    diff = (double)0;
  else
    diff = (double)((double)sharp / (double)maxsharp);
  //  strcpy(capbuf, objs(obj));
  sstring capbuf =
    colorString(this, desc, objs(obj), NULL, COLOR_OBJECTS, TRUE);

  if (diff <= .02)
    strcpy(sharpbuf, "is totally blunt");
  else if (diff < .10)
    strcpy(sharpbuf, "is virtually blunt now");
  else if (diff < .30)
    strcpy(sharpbuf, "has very little of its point left");
  else if (diff < .50)
    strcpy(sharpbuf, "is starting to get blunt");
  else if (diff < .70)
    strcpy(sharpbuf, "has some pointedness");
  else if (diff < .90)
    strcpy(sharpbuf, "has a good point");
  else if (diff < 1.0)
    strcpy(sharpbuf, "is nice and pointy");
  else
    strcpy(sharpbuf, "is as pointed as it is going to get");

  sprintf(buf, "%s %s", capbuf.uncap().c_str(), sharpbuf);
  return buf;
}

sstring TBeing::describeBluntness(const TBaseWeapon* obj) const {
  char buf[256];
  char sharpbuf[80];
  int maxsharp = obj->getMaxSharp();
  int sharp = obj->getCurSharp();
  double diff;
  if (!maxsharp)
    diff = (double)0;
  else
    diff = (double)((double)sharp / (double)maxsharp);
  //  strcpy(capbuf, objs(obj));
  sstring capbuf =
    colorString(this, desc, objs(obj), NULL, COLOR_OBJECTS, TRUE);

  if (diff <= .02)
    strcpy(sharpbuf, "is totally jagged");
  else if (diff < .10)
    strcpy(sharpbuf, "is extremely jagged now");
  else if (diff < .30)
    strcpy(sharpbuf, "has become jagged");
  else if (diff < .50)
    strcpy(sharpbuf, "is fairly jagged");
  else if (diff < .70)
    strcpy(sharpbuf, "has a lot of chips and is starting to get jagged");
  else if (diff < .90)
    strcpy(sharpbuf, "has some chips");
  else if (diff < 1.0)
    strcpy(sharpbuf, "has a few chips");
  else
    strcpy(sharpbuf, "is completely blunt");

  sprintf(buf, "%s %s", capbuf.uncap().c_str(), sharpbuf);
  return buf;
}

void TBeing::describeMaxSharpness(const TBaseWeapon* obj, int learn) const {
  if (obj->isBluntWeapon()) {
    describeMaxBluntness(obj, learn);
    return;
  } else if (obj->isPierceWeapon()) {
    describeMaxPointiness(obj, learn);
    return;
  }
  if (!hasClass(CLASS_THIEF) && !hasClass(CLASS_WARRIOR) &&
      !hasClass(CLASS_DEIKHAN) && !hasClass(CLASS_RANGER))
    learn /= 3;

  int maxsharp = GetApprox(obj->getMaxSharp(), learn);

  char capbuf[80], sharpbuf[80];
  strncpy(capbuf, objs(obj), cElements(capbuf) - 1);

  if (maxsharp >= 99)
    strcpy(sharpbuf, "being unhumanly sharp");
  else if (maxsharp >= 90)
    strcpy(sharpbuf, "being razor sharp");
  else if (maxsharp >= 80)
    strcpy(sharpbuf, "approximating razor sharpness");
  else if (maxsharp >= 72)
    strcpy(sharpbuf, "being incredibly sharp");
  else if (maxsharp >= 64)
    strcpy(sharpbuf, "being very sharp");
  else if (maxsharp >= 56)
    strcpy(sharpbuf, "being sharp");
  else if (maxsharp >= 48)
    strcpy(sharpbuf, "being fairly sharp");
  else if (maxsharp >= 40)
    strcpy(sharpbuf, "having a sword-like edge");
  else if (maxsharp >= 30)
    strcpy(sharpbuf, "having a very sharp edge");
  else if (maxsharp >= 20)
    strcpy(sharpbuf, "having a knife-like edge");
  else if (maxsharp >= 10)
    strcpy(sharpbuf, "having a sharp edge");
  else
    strcpy(sharpbuf, "having a ragged edge");

  sendTo(COLOR_OBJECTS, format("%s seems to be capable of %s.\n\r") %
                          sstring(capbuf).cap() % sharpbuf);
}

void TBeing::describeMaxPointiness(const TBaseWeapon* obj, int learn) const {
  char capbuf[80], sharpbuf[80];
  strncpy(capbuf, objs(obj), cElements(capbuf) - 1);

  if (!hasClass(CLASS_THIEF) && !hasClass(CLASS_WARRIOR) &&
      !hasClass(CLASS_DEIKHAN) && !hasClass(CLASS_RANGER) &&
      !hasClass(CLASS_SHAMAN) && !hasClass(CLASS_MAGE))
    learn /= 3;

  int maxsharp = GetApprox(obj->getMaxSharp(), learn);

  if (maxsharp >= 99)
    strcpy(sharpbuf, "being unhumanly pointy");
  else if (maxsharp >= 90)
    strcpy(sharpbuf, "being awesomly pointy");
  else if (maxsharp >= 80)
    strcpy(sharpbuf, "having an amazing point");
  else if (maxsharp >= 72)
    strcpy(sharpbuf, "being incredibly pointy");
  else if (maxsharp >= 64)
    strcpy(sharpbuf, "being very pointy");
  else if (maxsharp >= 56)
    strcpy(sharpbuf, "being pointy");
  else if (maxsharp >= 48)
    strcpy(sharpbuf, "being fairly pointy");
  else if (maxsharp >= 40)
    strcpy(sharpbuf, "having a dagger-like point");
  else if (maxsharp >= 30)
    strcpy(sharpbuf, "having a nice point");
  else if (maxsharp >= 20)
    strcpy(sharpbuf, "having a spear-like point");
  else if (maxsharp >= 10)
    strcpy(sharpbuf, "having a point");
  else
    strcpy(sharpbuf, "having a dull point");

  sendTo(COLOR_OBJECTS, format("%s seems to be capable of %s.\n\r") %
                          sstring(capbuf).cap() % sharpbuf);
}

void TBeing::describeOtherFeatures(const TGenWeapon* obj, int learn) const {
  char capbuf[80];
  strncpy(capbuf, objs(obj), cElements(capbuf) - 1);

  if (!obj) {
    sendTo("Something went wrong, tell a god what you did.\n\r");
    return;
  }

  if (hasClass(CLASS_THIEF) || isImmortal()) {
    if (obj->canCudgel())
      sendTo(COLOR_OBJECTS,
        format("%s seems small enough to be used for cudgeling.\n\r") %
          sstring(capbuf).cap());
    if (obj->canStab())
      sendTo(COLOR_OBJECTS,
        format("%s seems small enough to be used for stabbing.\n\r") %
          sstring(capbuf).cap());
    if (obj->canBackstab())
      sendTo(COLOR_OBJECTS, format("%s seems small enough to be used for "
                                   "backstabbing or throat slitting.\n\r") %
                              sstring(capbuf).cap());
  }
}

void TBeing::describeMaxBluntness(const TBaseWeapon* obj, int learn) const {
  char capbuf[80], sharpbuf[80];
  strncpy(capbuf, objs(obj), cElements(capbuf) - 1);

  if (!hasClass(CLASS_CLERIC) && !hasClass(CLASS_WARRIOR) &&
      !hasClass(CLASS_SHAMAN) && !hasClass(CLASS_DEIKHAN))
    learn /= 3;

  int maxsharp = GetApprox(obj->getMaxSharp(), learn);

  if (maxsharp >= 99)
    strcpy(sharpbuf, "being polished smooth");
  else if (maxsharp >= 90)
    strcpy(sharpbuf, "being awesomly blunt");
  else if (maxsharp >= 80)
    strcpy(sharpbuf, "being extremely blunt");
  else if (maxsharp >= 72)
    strcpy(sharpbuf, "being incredibly blunt");
  else if (maxsharp >= 64)
    strcpy(sharpbuf, "being very blunt");
  else if (maxsharp >= 56)
    strcpy(sharpbuf, "being blunt");
  else if (maxsharp >= 48)
    strcpy(sharpbuf, "being fairly blunt");
  else if (maxsharp >= 40)
    strcpy(sharpbuf, "having a hammer-like bluntness");
  else if (maxsharp >= 30)
    strcpy(sharpbuf, "being somewhat blunt");
  else if (maxsharp >= 20)
    strcpy(sharpbuf, "having a mace-like bluntness");
  else if (maxsharp >= 10)
    strcpy(sharpbuf, "having a ragged bluntness");
  else
    strcpy(sharpbuf, "having a sharp and ragged bluntness");

  sendTo(COLOR_OBJECTS, format("%s seems to be capable of %s.\n\r") %
                          sstring(capbuf).cap() % sharpbuf);
}

void TBeing::describeMaxStructure(const TObj* obj, int learn) const {
  obj->descMaxStruct(this, learn);
}

void TBeing::describeWeaponDamage(const TBaseWeapon* obj, int learn) const {
  if (!hasClass(CLASS_RANGER) && !hasClass(CLASS_WARRIOR) &&
      !hasClass(CLASS_DEIKHAN) && !hasWizPower(POWER_WIZARD)) {
    learn /= 3;
  }

#if 1
  double av_dam = GetApprox(obj->damageLevel(), learn);

  sendTo(COLOR_OBJECTS,
    format("It is capable of doing %s of damage for your level\n\r") %
      describe_damage((int)av_dam, this));
#else
  double av_dam = obj->baseDamage();
  av_dam += (double)obj->itemDamroll();
  av_dam = GetApprox((int)av_dam, learn);

  sendTo(COLOR_OBJECTS,
    format("It's capable of doing %s damage.\n\r") %
      ((av_dam < 1)
          ? "exceptionally low"
          : ((av_dam < 2)
                ? "incredibly low"
                : ((av_dam < 3)
                      ? "very low"
                      : ((av_dam < 4)
                            ? "low"
                            : ((av_dam < 5)
                                  ? "fairly low"
                                  : ((av_dam < 6)
                                        ? "moderate"
                                        : ((av_dam < 7)
                                              ? "a good bit of"
                                              : ((av_dam < 8)
                                                    ? "fairly good"
                                                    : ((av_dam < 9)
                                                          ? "good"
                                                          : ((av_dam < 10)
                                                                ? "very good"
                                                                : ((av_dam < 11)
                                                                      ? "really"
                                                                        " good"
                                                                      : ((av_dam <
                                                                           12)
                                                                            ? "exceptionally good"
                                                                            : ((av_dam <
                                                                                 13)
                                                                                  ? "very nice"
                                                                                  : ((av_dam <
                                                                                       14)
                                                                                        ? "a sizable amount of"
                                                                                        : ((av_dam <
                                                                                             15)
                                                                                              ? "respectable"
                                                                                              : ((av_dam <
                                                                                                   16)
                                                                                                    ? "whopping"
                                                                                                    : ((av_dam <
                                                                                                         17)
                                                                                                          ? "punishing"
                                                                                                          : ((av_dam <
                                                                                                               18)
                                                                                                                ? "devestating"
                                                                                                                : ((av_dam <
                                                                                                                     19)
                                                                                                                      ? "awesome"
                                                                                                                      : ((av_dam <
                                                                                                                           20)
                                                                                                                            ? "superb"
                                                                                                                            : "super-human")))))))))))))))))))));
#endif
}

void TBeing::describeArmor(const TBaseClothing* obj, int learn) {
  if (!hasClass(CLASS_RANGER) && !hasClass(CLASS_WARRIOR) &&
      !hasClass(CLASS_DEIKHAN))
    learn /= 3;

#if 1
  double tACPref, tStrPref, tSHLvl = suggestArmor(),
                            tIsLvl = obj->armorLevel(ARMOR_LEV_AC), tACMin,
                            tACQua;

  obj->armorPercs(&tACPref, &tStrPref);

  // This tells us what they 'should have' on the slot:
  tSHLvl *= tACPref;

  // Convert tSHLvl to a level
  tACPref *= (obj->isPaired() ? 2.0 : 1.0);
  tACMin = ((500.0 * tACPref) + (obj->isPaired() ? 1.0 : 0.5));
  tACQua = (max(0.0, (tSHLvl - tACMin)) / tACPref);
  tSHLvl = tACQua / 25;

  int tDiff = GetApprox((int)(tIsLvl - tSHLvl), learn);
  sstring tStLevel("");

  if (tDiff < -20)
    tStLevel = "a horrid amount";
  else if (tDiff < -15)
    tStLevel = "a sad amount";
  else if (tDiff < -10)
    tStLevel = "a pathetic amount";
  else if (tDiff < -5)
    tStLevel = "a decent amount";
  else if (tDiff <= -1)
    tStLevel = "a near perfect amount";
  else if (tDiff == 0)
    tStLevel = "a perfect amount";
  else if (tDiff <= 2)
    tStLevel = "a near perfect amount";  // This and -1 is where we confuse them
  else if (tDiff < 5)
    tStLevel = "a good amount";
  else if (tDiff < 10)
    tStLevel = "a really good amount";
  else if (tDiff < 15)
    tStLevel = "an extremely good amount";
  else
    tStLevel = "way too much of an amount";

  sendTo(COLOR_OBJECTS,
    format("This supplies %s of protection for your class and level\n\r") %
      tStLevel);
#else
  int armor = 0;  // works in reverse here.  armor > 0 is GOOD
  armor -= obj->itemAC();
  armor = GetApprox(armor, learn);

  sendTo(COLOR_OBJECTS,
    format("In terms of armor quality, it ranks as %s.\n\r") %
      ((armor < 0)
          ? "being more hurt then help"
          : ((armor < 2)
                ? "being virtually non-existant"
                : ((armor < 4)
                      ? "being exceptionally low"
                      : ((armor < 6)
                            ? "being low"
                            : ((armor < 8)
                                  ? "being fairly good"
                                  : ((armor < 10)
                                        ? "being somewhat good"
                                        : ((armor < 12)
                                              ? "being good"
                                              : ((armor < 14)
                                                    ? "being really good"
                                                    : ((armor < 16)
                                                          ? "protecting you "
                                                            "fairly well"
                                                          : ((armor < 18)
                                                                ? "protecting "
                                                                  "you well"
                                                                : ((armor < 20)
                                                                      ? "protec"
                                                                        "ting "
                                                                        "you "
                                                                        "very "
                                                                        "well"
                                                                      : ((armor <
                                                                           22)
                                                                            ? "protecting you really well"
                                                                            : ((armor <
                                                                                 24)
                                                                                  ? "protecting you exceptionally well"
                                                                                  : ((armor <
                                                                                       26)
                                                                                        ? "armoring you like a fortress"
                                                                                        : ((armor <
                                                                                             28)
                                                                                              ? "armoring you like a dragon"
                                                                                              : ((armor <
                                                                                                   30)
                                                                                                    ? "being virtually impenetrable"
                                                                                                    : "being impenetrable")))))))))))))))));
#endif
}

sstring TBeing::describeImmunities(const TBeing* vict, int learn) const {
  char buf[80];
  char buf2[256];
  sstring str;

  int x;
  for (immuneTypeT i = MIN_IMMUNE; i < MAX_IMMUNES; i++) {
    x = GetApprox(vict->getImmunity(i), learn);

    if (x == 0 || !*immunity_names[i])
      continue;
    if (x > 90 || x < -90)
      strcpy(buf, "extremely");
    else if (x > 70 || x < -70)
      strcpy(buf, "heavily");
    else if (x > 50 || x < -50)
      strcpy(buf, "majorly");
    else if (x > 30 || x < -30)
      strcpy(buf, "greatly");
    else if (x > 10 || x < -10)
      strcpy(buf, "somewhat");
    else
      strcpy(buf, "lightly");

    if (vict == this)
      sprintf(buf2, "You are %s %s to %s.\n\r", buf,
        (x > 0 ? "resistant" : "susceptible"), immunity_names[i]);
    else
      sprintf(buf2, "%s is %s %s to %s.\n\r", sstring(pers(vict)).cap().c_str(),
        buf, (x > 0 ? "resistant" : "susceptible"), immunity_names[i]);
    str += buf2;
  }
  return str;
}

void TBeing::describeArrowDamage(const TArrow* obj, int learn) {
  if (!hasClass(CLASS_RANGER))
    learn /= 3;

  double av_dam = obj->baseDamage();
  av_dam += (double)obj->itemDamroll();
  av_dam = GetApprox((int)av_dam, learn);

  sendTo(COLOR_OBJECTS,
    format("It's capable of doing %s damage.\n\r") %
      ((av_dam < 1)
          ? "exceptionally low"
          : ((av_dam < 2)
                ? "incredibly low"
                : ((av_dam < 3)
                      ? "very low"
                      : ((av_dam < 4)
                            ? "low"
                            : ((av_dam < 5)
                                  ? "fairly low"
                                  : ((av_dam < 6)
                                        ? "moderate"
                                        : ((av_dam < 7)
                                              ? "a good bit of"
                                              : ((av_dam < 8)
                                                    ? "fairly good"
                                                    : ((av_dam < 9)
                                                          ? "good"
                                                          : ((av_dam < 10)
                                                                ? "very good"
                                                                : ((av_dam < 11)
                                                                      ? "really"
                                                                        " good"
                                                                      : ((av_dam <
                                                                           12)
                                                                            ? "exceptionally good"
                                                                            : ((av_dam <
                                                                                 13)
                                                                                  ? "very nice"
                                                                                  : ((av_dam <
                                                                                       14)
                                                                                        ? "a sizable amount of"
                                                                                        : ((av_dam <
                                                                                             15)
                                                                                              ? "respectable"
                                                                                              : ((av_dam <
                                                                                                   16)
                                                                                                    ? "whopping"
                                                                                                    : ((av_dam <
                                                                                                         17)
                                                                                                          ? "punishing"
                                                                                                          : ((av_dam <
                                                                                                               18)
                                                                                                                ? "devestating"
                                                                                                                : ((av_dam <
                                                                                                                     19)
                                                                                                                      ? "awesome"
                                                                                                                      : ((av_dam <
                                                                                                                           20)
                                                                                                                            ? "superb"
                                                                                                                            : "super-human")))))))))))))))))))));
}

void TBeing::describeArrowSharpness(const TArrow* obj, int learn) {
  if (!hasClass(CLASS_RANGER))
    learn /= 3;

  int maxsharp = GetApprox(obj->getCurSharp(), learn);

  char capbuf[80], sharpbuf[80];
  strncpy(capbuf, objs(obj), cElements(capbuf) - 1);

  if (maxsharp >= 99)
    strcpy(sharpbuf, "unhumanly sharp");
  else if (maxsharp >= 90)
    strcpy(sharpbuf, "razor sharp");
  else if (maxsharp >= 80)
    strcpy(sharpbuf, "almost razor sharp");
  else if (maxsharp >= 72)
    strcpy(sharpbuf, "incredibly sharp");
  else if (maxsharp >= 64)
    strcpy(sharpbuf, "very sharp");
  else if (maxsharp >= 56)
    strcpy(sharpbuf, "sharp");
  else if (maxsharp >= 48)
    strcpy(sharpbuf, "fairly sharp");
  else if (maxsharp >= 40)
    strcpy(sharpbuf, "kind of sharp");
  else if (maxsharp >= 30)
    strcpy(sharpbuf, "not really sharp");
  else if (maxsharp >= 20)
    strcpy(sharpbuf, "dull");
  else if (maxsharp >= 10)
    strcpy(sharpbuf, "very dull");
  else
    strcpy(sharpbuf, "extremely dull");

  sendTo(COLOR_OBJECTS,
    format("%s has a tip that is %s.\n\r") % sstring(capbuf).cap() % sharpbuf);
}

void TBeing::describeNoise(const TObj* obj, int learn) const {
  if (!dynamic_cast<const TBaseClothing*>(obj) &&
      !dynamic_cast<const TBaseWeapon*>(obj) && !dynamic_cast<const TBow*>(obj))
    return;

  if (obj->canWear(ITEM_WEAR_HOLD) || obj->canWear(ITEM_WEAR_FINGERS))
    return;

  int iNoise = GetApprox(material_nums[obj->getMaterial()].noise, learn);

  char capbuf[160];
  strncpy(capbuf, objs(obj), cElements(capbuf) - 1);

  sendTo(COLOR_OBJECTS,
    format("%s is %s.\n\r") % sstring(capbuf).cap() %
      ((iNoise < -9)
          ? "beyond silent"
          : ((iNoise < -5)
                ? "extremely silent"
                : ((iNoise < -2)
                      ? "very silent"
                      : ((iNoise < 1)
                            ? "silent"
                            : ((iNoise < 3)
                                  ? "very quiet"
                                  : ((iNoise < 6)
                                        ? "quiet"
                                        : ((iNoise < 10)
                                              ? "pretty quiet"
                                              : ((iNoise < 14)
                                                    ? "mostly quiet"
                                                    : ((iNoise < 19)
                                                          ? "slightly noisy"
                                                          : ((iNoise < 25)
                                                                ? "fairly noisy"
                                                                : ((iNoise < 31)
                                                                      ? "noisy"
                                                                      : ((iNoise <
                                                                           38)
                                                                            ? "very noisy"
                                                                            : "loud")))))))))))));
}

void TBeing::describeRoomLight() {
  int illum = roomp->getLight();

  sendTo(COLOR_BASIC,
    format("This area is %s.\n\r") %
      ((illum < -4)
          ? "<k>super dark<1>"
          : ((illum < 0)
                ? "<k>pitch dark<1>"
                : ((illum < 1)
                      ? "<k>dark<1>"
                      : ((illum < 3)
                            ? "<w>very dimly lit<1>"
                            : ((illum < 5)
                                  ? "<w>dimly lit<1>"
                                  : ((illum < 9)
                                        ? "barely lit"
                                        : ((illum < 13)
                                              ? "lit"
                                              : ((illum < 19)
                                                    ? "brightly lit"
                                                    : ((illum < 25)
                                                          ? "<Y>very brightly "
                                                            "lit<1>"
                                                          : "<Y>bright as "
                                                            "day<1>"))))))))));
}

void TBeing::describeGround() {
  if (!roomp->describeGroundWeather().empty()) {
    sendTo(COLOR_BASIC, format("The %s is %s.\n\r") %
                          roomp->describeGroundType() %
                          roomp->describeGroundWeather());
  }
}

void TBeing::describeBowRange(const TBow* obj, int learn) {
  if (!hasClass(CLASS_RANGER))
    learn /= 3;

  int range = GetApprox((int)obj->getMaxRange(), learn);

  char capbuf[160];
  strncpy(capbuf, objs(obj), cElements(capbuf) - 1);

  sendTo(COLOR_OBJECTS,
    format("%s can %s.\n\r") % sstring(capbuf).cap() %
      ((range < 1)
          ? "not shoot out of the immediate area"
          : ((range < 3)
                ? "barely shoot beyond arm's length"
                : ((range < 5)
                      ? "shoot a short distance"
                      : ((range < 8) ? "fire a reasonable distance"
                                     : ((range < 11)
                                           ? "shoot quite a ways"
                                           : ((range < 15)
                                                 ? "shoot a long distance"
                                                 : "fire incredibly far")))))));
}

void TBeing::describeMagicLevel(const TMagicItem* obj, int learn) const {
  if (!hasClass(CLASS_MAGE) && !hasClass(CLASS_CLERIC) &&
      !hasClass(CLASS_RANGER) && !hasClass(CLASS_DEIKHAN) &&
      !hasClass(CLASS_SHAMAN))
    return;

  int level = GetApprox(obj->getMagicLevel(), learn);
  level = max(level, 0);

  sendTo(COLOR_OBJECTS,
    format("Spells from %s seem to be cast at %s level.\n\r") %
      sstring(objs(obj)).uncap() % numberAsString(level));
}

const sstring numberAsString(int num) {
  char buf[50];

  if (in_range(num % 100, 11, 13))
    sprintf(buf, "%dth", num);
  else if ((num % 10) == 1)
    sprintf(buf, "%dst", num);
  else if ((num % 10) == 2)
    sprintf(buf, "%dnd", num);
  else if ((num % 10) == 3)
    sprintf(buf, "%drd", num);
  else
    sprintf(buf, "%dth", num);

  return buf;
}

void TBeing::describeMagicLearnedness(const TMagicItem* obj, int learn) const {
  if (!hasClass(CLASS_MAGE) && !hasClass(CLASS_CLERIC) &&
      !hasClass(CLASS_RANGER) && !hasClass(CLASS_DEIKHAN) &&
      !hasClass(CLASS_SHAMAN))
    return;

  int level = GetApprox(obj->getMagicLearnedness(), learn);

  sendTo(COLOR_OBJECTS,
    format("The learnedness of the spells in %s is: %s.\n\r") %
      sstring(objs(obj)).uncap() % how_good(level));
}

void TBeing::describeMagicSpell(const TMagicItem* obj, int learn) {
  if (!hasClass(CLASS_MAGE) && !hasClass(CLASS_CLERIC) &&
      !hasClass(CLASS_RANGER) && !hasClass(CLASS_DEIKHAN) &&
      !hasClass(CLASS_SHAMAN))
    return;

  int level = GetApprox(getSkillLevel(SKILL_EVALUATE), learn);

  if (obj->getMagicLevel() > level) {
    sendTo(COLOR_OBJECTS,
      format("You can tell nothing about the spells %s produces.\n\r") %
        sstring(objs(obj)).uncap());
    return;
  }

  obj->descMagicSpells(this);
}

void TWand::descMagicSpells(TBeing* ch) const {
  discNumT das = DISC_NONE;
  spellNumT iSpell;
  char capbuf[160];
  strncpy(capbuf, ch->objs(this), cElements(capbuf) - 1);

  if ((iSpell = getSpell()) >= MIN_SPELL && discArray[iSpell] &&
      ((das = getDisciplineNumber(iSpell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(iSpell))
      ch->sendTo(COLOR_OBJECTS, format("%s produces: %s.\n\r") %
                                  sstring(capbuf).cap() %
                                  discArray[iSpell]->name);
    else
      ch->sendTo(COLOR_OBJECTS,
        format("%s produces: Something from the %s discipline.\n\r") %
          sstring(capbuf).cap() % discNames[das].properName);
  }
}

void TStaff::descMagicSpells(TBeing* ch) const {
  discNumT das = DISC_NONE;
  spellNumT iSpell;
  char capbuf[160];
  strncpy(capbuf, ch->objs(this), cElements(capbuf) - 1);

  if ((iSpell = getSpell()) >= MIN_SPELL && discArray[iSpell] &&
      ((das = getDisciplineNumber(iSpell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(iSpell))
      ch->sendTo(COLOR_OBJECTS, format("%s produces: %s.\n\r") %
                                  sstring(capbuf).cap() %
                                  discArray[iSpell]->name);
    else
      ch->sendTo(COLOR_OBJECTS,
        format("%s produces: Something from the %s discipline.\n\r") %
          sstring(capbuf).cap() % discNames[das].properName);
  }
}

void TScroll::descMagicSpells(TBeing* ch) const {
  discNumT das = DISC_NONE;
  spellNumT spell;
  char capbuf[160];
  strncpy(capbuf, ch->objs(this), cElements(capbuf) - 1);

  spell = getSpell(0);
  if (spell > TYPE_UNDEFINED && discArray[spell] &&
      ((das = getDisciplineNumber(spell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(spell))
      ch->sendTo(COLOR_OBJECTS, format("%s produces: %s.\n\r") %
                                  sstring(capbuf).cap() %
                                  discArray[spell]->name);
    else
      ch->sendTo(COLOR_OBJECTS,
        format("%s produces: Something from the %s discipline.\n\r") %
          sstring(capbuf).cap() % discNames[das].properName);
  }

  spell = getSpell(1);
  if (spell > TYPE_UNDEFINED && discArray[spell] &&
      ((das = getDisciplineNumber(spell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(spell))
      ch->sendTo(COLOR_OBJECTS, format("%s produces: %s.\n\r") %
                                  sstring(capbuf).cap() %
                                  discArray[spell]->name);
    else
      ch->sendTo(COLOR_OBJECTS,
        format("%s produces: Something from the %s discipline.\n\r") %
          sstring(capbuf).cap() % discNames[das].properName);
  }

  spell = getSpell(2);
  if (spell > TYPE_UNDEFINED && discArray[spell] &&
      ((das = getDisciplineNumber(spell, FALSE)) != DISC_NONE)) {
    if (ch->doesKnowSkill(spell))
      ch->sendTo(COLOR_OBJECTS, format("%s produces: %s.\n\r") %
                                  sstring(capbuf).cap() %
                                  discArray[spell]->name);
    else
      ch->sendTo(COLOR_OBJECTS,
        format("%s produces: Something from the %s discipline.\n\r") %
          sstring(capbuf).cap() % discNames[das].properName);
  }
}

void TBeing::describeSymbolOunces(const TSymbol* obj, int learn) const {
  if (obj->getSymbolFaction() != FACT_UNDEFINED) {
    act("$p is already attuned.", false, this, obj, 0, TO_CHAR);
    return;
  }

  if (!hasClass(CLASS_CLERIC) && !hasClass(CLASS_DEIKHAN))
    learn /= 3;

  // number of ounces needed, see attuning
  int amt = obj->obj_flags.cost / 100;
  amt = GetApprox(amt, learn);

  char capbuf[160];
  strncpy(capbuf, objs(obj), cElements(capbuf) - 1);

  sendTo(COLOR_OBJECTS,
    format("%s requires about %d ounce%s of holy water to attune.\n\r") %
      sstring(capbuf).cap() % amt % (amt == 1 ? "" : "s"));
}

void TBeing::describeComponentUseage(const TComponent* obj, int) const {
  char capbuf[160];
  strncpy(capbuf, objs(obj), cElements(capbuf) - 1);

  if (IS_SET(obj->getComponentType(), COMP_SPELL))
    sendTo(COLOR_OBJECTS,
      format("%s is a component used in creating magic.\n\r") %
        sstring(capbuf).cap());
  else if (IS_SET(obj->getComponentType(), COMP_POTION))
    sendTo(COLOR_OBJECTS,
      format("%s is a component used to brew potions.\n\r") %
        sstring(capbuf).cap());
  else if (IS_SET(obj->getComponentType(), COMP_SCRIBE))
    sendTo(COLOR_OBJECTS,
      format("%s is a component used during scribing.\n\r") %
        sstring(capbuf).cap());
}

void TBeing::describeComponentDecay(const TComponent* obj, int learn) const {
  if (!hasClass(CLASS_MAGE) && !hasClass(CLASS_CLERIC) &&
      !hasClass(CLASS_RANGER) && !hasClass(CLASS_DEIKHAN) &&
      !hasClass(CLASS_SHAMAN))
    learn /= 3;

  int level = GetApprox(obj->obj_flags.decay_time, learn);

  char capbuf[160];
  strncpy(capbuf, objs(obj), cElements(capbuf) - 1);

  sendTo(COLOR_OBJECTS, format("%s will last ") % sstring(capbuf).cap());

  if (!obj->isComponentType(COMP_DECAY)) {
    sendTo("well into the future.\n\r");
    return;
  }

  if ((level <= -1) || (level >= 800))
    sendTo("well into the future.\n\r");
  else if (level < 50)
    sendTo("about a day.\n\r");
  else if (level < 100)
    sendTo("a few days *tops*.\n\r");
  else if (level < 200)
    sendTo("about a week.\n\r");
  else if (level < 400)
    sendTo("only a couple of weeks.\n\r");
  else if (level < 800)
    sendTo("around a month.\n\r");
}

void TBeing::describeComponentSpell(const TComponent* obj, int) const {
  int which = obj->getComponentSpell();

  if (which >= 0 && discArray[which])
    sendTo(COLOR_OBJECTS, format("%s is used for: %s.\n\r") %
                            sstring(objs(obj)).cap() % discArray[which]->name);
}

sstring describeMaterial(const TThing* t) {
  sstring str;
  char buf[256];

  int mat = t->getMaterial();
  char mat_name[40];

  strncpy(mat_name, sstring(material_nums[mat].mat_name).uncap().c_str(),
    cElements(mat_name) - 1);

  if (dynamic_cast<const TBeing*>(t))
    sprintf(buf, "%s has a skin type of %s.\n\r",
      sstring(t->getName()).cap().c_str(), mat_name);
  else
    sprintf(buf, "%s is made of %s.\n\r", sstring(t->getName()).cap().c_str(),
      mat_name);
  str += buf;

  str += describeMaterial(mat);

  return str;
}

sstring describeMaterial(int mat) {
  sstring str, mat_name;

  mat_name = material_nums[mat].mat_name;
  mat_name = mat_name.cap();

  str += format("%s is %d%c susceptible to slash attacks.\n\r") % mat_name %
         material_nums[mat].cut_susc % '%';

  str += format("%s is %d%c susceptible to pierce attacks.\n\r") % mat_name %
         material_nums[mat].pierced_susc % '%';

  str += format("%s is %d%c susceptible to blunt attacks.\n\r") % mat_name %
         material_nums[mat].smash_susc % '%';

  str += format("%s is %d%c susceptible to flame attacks.\n\r") % mat_name %
         material_nums[mat].burned_susc % '%';

  str += format("%s is %d%c susceptible to acid attacks.\n\r") % mat_name %
         material_nums[mat].acid_susc % '%';

  str += format(
           "%s is %d%c susceptible to water erosion, and suffers %d damage per "
           "erosion.\n\r") %
         mat_name % (material_nums[mat].water_susc % 10 * 10) % '%' %
         (material_nums[mat].water_susc / 10);

  str += format(
           "%s is %d%c susceptible to fall shock, and suffers %d damage per "
           "shock.\n\r") %
         mat_name % (material_nums[mat].fall_susc % 10 * 10) % '%' %
         (material_nums[mat].fall_susc / 10);

  str += format("%s has a hardness of %d units.\n\r") % mat_name %
         material_nums[mat].hardness;

  str += format("%s has a compaction ratio of %d:1.\n\r") % mat_name %
         material_nums[mat].vol_mult;

  str += format("%s is %sconsidered a conductive material.\n\r") % mat_name %
         (material_nums[mat].conductivity ? "" : "not ");

  str += format("%s is worth %f talens per unit.\n\r") % mat_name %
         material_nums[mat].price;

  return str;
}

void TBeing::sendRoomName(TRoom* rp) const {
  unsigned int rFlags = rp->getRoomFlags();
  Descriptor* d = desc;
  sstring clientBuf = "";
  sstring rFlagStr = "";

  if (!d)
    return;

  clientBuf = format("\200%d|") % CLIENT_ROOMNAME;

  rFlagStr = sstring((rFlags & ROOM_PEACEFUL) ? " [PEACEFUL]" : "") +
             sstring((rFlags & ROOM_NO_HEAL) ? " [NOHEAL]" : "") +
             sstring((rFlags & ROOM_HOSPITAL) ? " [HOSPITAL]" : "") +
             sstring((rFlags & ROOM_ARENA) ? " [ARENA]" : "");

  if (!rFlagStr.empty()) {
    rFlagStr = "    " + rFlagStr;
  }

  if (IS_SET(desc->plr_color, PLR_COLOR_ROOM_NAME)) {
    if (hasColorStrings(this, rp->getName(), 2)) {
      sendTo(COLOR_ROOM_NAME,
        format("%s%s%s%s%s%s\n\r") % (d->m_bIsClient ? clientBuf : "") %
          dynColorRoom(rp, 1, TRUE) % norm() % red() % rFlagStr % norm());
    } else {
      sendTo(COLOR_ROOM_NAME, format("%s%s%s%s%s%s%s\n\r") %
                                (d->m_bIsClient ? clientBuf : "") %
                                addColorRoom(rp, 1) % rp->getName() % norm() %
                                red() % rFlagStr % norm());
    }
  } else {
    if (hasColorStrings(this, rp->getName(), 2)) {
      sendTo(COLOR_BASIC,
        format("%s%s%s%s%s%s\n\r") % (d->m_bIsClient ? clientBuf : "") %
          purple() %
          colorString(this, desc, rp->getName(), NULL, COLOR_NONE, FALSE) %
          red() % rFlagStr % norm());
    } else {
      sendTo(COLOR_BASIC, format("%s%s%s%s%s%s\n\r") %
                            (d->m_bIsClient ? clientBuf : "") % purple() %
                            rp->getName() % red() % rFlagStr % norm());
    }
  }
  if (isImmortal() && (desc->prompt_d.type & PROMPT_BUILDER_ASSISTANT)) {
    sendTo(format("{ %s%s%s%s%s%s%s%s%s%s%s%s%s%s}\n\r") %
           (rFlags == 0 ? "--none-- " : "") %
           (!(rFlags & (ROOM_ALWAYS_LIT | ROOM_NO_MOB | ROOM_INDOORS |
                         ROOM_PEACEFUL | ROOM_NO_STEAL | ROOM_NO_ESCAPE |
                         ROOM_NO_MAGIC | ROOM_NO_PORTAL | ROOM_SILENCE |
                         ROOM_NO_ORDER | ROOM_NO_FLEE | ROOM_HAVE_TO_WALK)) &&
                 (rFlags > 0)
               ? "--others-- "
               : "") %
           ((rFlags & ROOM_ALWAYS_LIT) ? "[Light] " : "") %
           ((rFlags & ROOM_NO_MOB) ? "[!Mob] " : "") %
           ((rFlags & ROOM_INDOORS) ? "[Indoors] " : "") %
           ((rFlags & ROOM_PEACEFUL) ? "[Peaceful] " : "") %
           ((rFlags & ROOM_NO_STEAL) ? "[!Steal] " : "") %
           ((rFlags & ROOM_NO_ESCAPE) ? "[!Escape] " : "") %
           ((rFlags & ROOM_NO_MAGIC) ? "[!Magic] " : "") %
           ((rFlags & ROOM_NO_PORTAL) ? "[!Portal] " : "") %
           ((rFlags & ROOM_SILENCE) ? "[Silent] " : "") %
           ((rFlags & ROOM_NO_ORDER) ? "[!Order] " : "") %
           ((rFlags & ROOM_NO_FLEE) ? "[!Flee] " : "") %
           ((rFlags & ROOM_HAVE_TO_WALK) ? "[Have-To-Walk] " : ""));
  }
}

void TBeing::sendRoomDesc(TRoom* rp) {
  sstring tmp;

  tmp = rp->getDescr();

  if (hasColorStrings(this, tmp, 2)) {
    if (rp->isRoomFlag(ROOM_NO_AUTOFORMAT)) {
      sendTo(COLOR_ROOMS,
        format("%s%s") % dynColorRoom(rp, 2, TRUE).toCRLF() % norm());
    } else {
      sendTo(COLOR_ROOMS, format("%s%s") %
                            autoFormatDesc(dynColorRoom(rp, 2, TRUE), true) %
                            norm());
    }
  } else {
    if (rp->isRoomFlag(ROOM_NO_AUTOFORMAT)) {
      sendTo(COLOR_ROOMS,
        format("%s%s%s") % addColorRoom(rp, 2) % tmp.toCRLF() % norm());
      // sendTo(COLOR_ROOMS, format("%s%s%s") % addColorRoom(rp % 2) %
      // autoFormatDesc(rp->getDescr() % true % true) % norm());
    } else {
      sendTo(COLOR_ROOMS, format("%s%s%s") % addColorRoom(rp, 2) %
                            autoFormatDesc(tmp, true) % norm());
    }
  }
}

void TBeing::describeTrapEffect(const TTrap*, int) const {
  // this tells things like the triggers, why let them know these?
}

void TBeing::describeTrapLevel(const TTrap* obj, int learn) const {
  if (!doesKnowSkill(SKILL_DETECT_TRAP))
    return;

  int level = GetApprox(obj->getTrapLevel(), learn);
  level = max(level, 0);

  sendTo(COLOR_OBJECTS, format("%s seems to be a %s level trap.\n\r") %
                          sstring(objs(obj)).cap() % numberAsString(level));
}

void TBeing::describeTrapCharges(const TTrap* obj, int learn) const {
  if (!doesKnowSkill(SKILL_DETECT_TRAP))
    return;

  int level = GetApprox(obj->getTrapCharges(), learn);
  level = max(level, 0);

  sendTo(COLOR_OBJECTS, format("%s seems to have %d charge%s left.\n\r") %
                          sstring(objs(obj)).cap() % level %
                          (level == 1 ? "" : "s"));
}

void TBeing::describeTrapDamType(const TTrap* obj, int) const {
  if (!doesKnowSkill(SKILL_DETECT_TRAP))
    return;

  sendTo(COLOR_OBJECTS,
    format("You suspect %s is %s %s trap.\n\r") % sstring(objs(obj)).uncap() %
      (trap_types[obj->getTrapDamType()].startsVowel() ? "an" : "a") %
      trap_types[obj->getTrapDamType()].uncap());
}

// Functions used by TBeing::displaySpellInfo
namespace {
  using std::unordered_map;
  using std::vector;

  enum SpellCategory {
    TARGETED_OFFENSIVE,
    TARGETED_UTILITY,
    NONTARGETED_OFFENSIVE,
    NONTARGETED_UTILITY
  };

  using SpellsByCategory = unordered_map<SpellCategory, vector<spellNumT>>;
  using SpellsByCategoryByClass =
    unordered_map<skillUseClassT, SpellsByCategory>;

  const vector<skillUseClassT> VALID_SPELL_TYPES = {SPELL_CLERIC, SPELL_DEIKHAN,
    SPELL_MAGE, SPELL_SHAMAN, SPELL_RANGER};

  auto getAllSpellsSorted(const TBeing* character) {
    vector<skillSorter> output;
    output.reserve(MAX_SKILL);

    auto isValidSpellType = [](const skillUseClassT spellType) {
      return std::find(VALID_SPELL_TYPES.begin(), VALID_SPELL_TYPES.end(),
               spellType) != VALID_SPELL_TYPES.end();
    };

    int index = 0;
    for (const spellInfo* spell : discArray) {
      const auto indexEnum = static_cast<spellNumT>(index);
      ++index;

      if (!spell || sstring(spell->name).empty() ||
          !isValidSpellType(spell->typ) ||
          (getDisciplineNumber(indexEnum, false) == DISC_NONE))
        continue;

      output.emplace_back(skillSorter(character, indexEnum));
    }

    std::sort(output.begin(), output.end(), skillSorter());

    return output;
  };

  auto createEmptySpellsByCategoryMap() {
    SpellsByCategory output;

    for (int category = TARGETED_OFFENSIVE; category <= NONTARGETED_UTILITY;
         ++category) {
      output.emplace(static_cast<SpellCategory>(category), vector<spellNumT>());
      output.at(static_cast<SpellCategory>(category)).reserve(32);
    }

    return output;
  };

  auto createEmptySpellsByCategoryByClassMap() {
    SpellsByCategoryByClass output;

    for (const skillUseClassT spellType : VALID_SPELL_TYPES)
      output.emplace(spellType, createEmptySpellsByCategoryMap());

    return output;
  };

  SpellCategory getSpellCategory(const spellInfo* spell) {
    const bool isViolent = spell->targets & TAR_VIOLENT;
    const bool isAOE = spell->targets & TAR_AREA;
    const bool isTargeted =
      spell->targets & (TAR_CHAR_ROOM | TAR_CHAR_WORLD | TAR_SELF_ONLY |
                         TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_VIS_WORLD);

    return isViolent ? (isAOE ? NONTARGETED_OFFENSIVE : TARGETED_OFFENSIVE)
                     : (isTargeted ? TARGETED_UTILITY : NONTARGETED_UTILITY);
  };

  SpellsByCategoryByClass createSpellsByCategoryByClassMap(
    const TBeing* character) {
    auto output = createEmptySpellsByCategoryByClassMap();

    const vector<skillSorter> spellsSorted = getAllSpellsSorted(character);
    for (const auto& skillSorterObj : spellsSorted) {
      const spellNumT spellNumber = skillSorterObj.theSkill;
      const spellInfo* spell = discArray[spellNumber];
      output.at(spell->typ).at(getSpellCategory(spell)).push_back(spellNumber);
    }

    return output;
  };

  auto getPossibleComponentLocations(const TBeing* character,
    const spellNumT spellNumber) {
    const auto comp = std::find_if(CompInfo.begin(), CompInfo.end(),
      [spellNumber](
        const compInfo& ci) { return ci.spell_num == spellNumber; });

    if (comp == CompInfo.end() || comp->comp_num <= 0)
      return std::tuple(vector<const TThing*>(), comp);

    vector<const TThing*> possibleComponentLocations{
      character,
      character->heldInPrimHand(),
      character->heldInSecHand(),
      character->equipment[WEAR_WAIST],
      character->equipment[WEAR_NECK],
      character->equipment[WEAR_WRIST_R],
      character->equipment[WEAR_WRIST_L],
    };

    return std::tuple(possibleComponentLocations, comp);
  };

  int findComponentCharges(const TThing* thing, const spellNumT spell) {
    if (!thing)
      return 0;

    const auto* comp = dynamic_cast<const TComponent*>(thing);
    if (comp && comp->isComponentType(COMP_SPELL) &&
        comp->getComponentSpell() == spell)
      return comp->getComponentCharges();

    const auto* container = dynamic_cast<const TOpenContainer*>(thing);
    const auto* being = dynamic_cast<const TBeing*>(thing);
    if (!being && (!container || container->isClosed()))
      return 0;

    int count = 0;
    for (const auto* each : thing->stuff)
      count += findComponentCharges(each, spell);

    return count;
  }

  sstring getComponentString(const TBeing* character,
    const spellNumT spellNumber) {
    sstring componentString = "[   ] No Component Found";

    const auto [possibleComponentLocations, comp] =
      getPossibleComponentLocations(character, spellNumber);

    if (possibleComponentLocations.empty() || comp == CompInfo.end())
      return componentString;

    int totalCharges = 0;
    for (const TThing* location : possibleComponentLocations) {
      if (!location)
        continue;
      totalCharges += findComponentCharges(location, spellNumber);
    }

    const int compIndex = real_object(comp->comp_num);
    const sstring compShortDesc = obj_index[compIndex].short_desc;

    componentString = format("[%3i] %s") % totalCharges % compShortDesc;

    return componentString;
  };

  auto generateOutputLines(const TBeing* character,
    const vector<spellNumT>& skills, const bool showUnlearned) {
    vector<vector<sstring>> output;

    for (const spellNumT skillNumber : skills) {
      const CDiscipline* disc =
        character->getDiscipline(getDisciplineNumber(skillNumber, false));

      if (!disc || !IS_SET(character->getClass(),
                     static_cast<uint16_t>(disc->ok_for_class)))
        continue;

      const int16_t skillValue = character->getSkillValue(skillNumber);
      const spellInfo* spell = discArray[skillNumber];
      const int discLearnedness =
        !disc || disc->getLearnedness() <= 0
          ? 0
          : min(disc->getLearnedness(), static_cast<int>(MAX_DISC_LEARNEDNESS));

      const bool spellIsUnlearned =
        skillValue <= 0 && spell->start > discLearnedness;
      const bool spellRequiresQuest =
        spell->toggle && !character->hasQuestBit(spell->toggle);

      if ((spellRequiresQuest || spellIsUnlearned) && !showUnlearned)
        continue;

      const int16_t maxSkillValue = character->getMaxSkillValue(skillNumber);
      const bool spellNotYetMaxable =
        maxSkillValue < MAX_SKILL_LEARNEDNESS && spell->startLearnDo > 0;

      vector<sstring> thisLine;
      thisLine.reserve(3);

      const sstring spellName =
        format("%s%s%s") % character->cyan() % spell->name % character->norm();
      thisLine.emplace_back(spellName);

      if (spellRequiresQuest)
        thisLine.emplace_back("Learned: Quest");
      else if (spellIsUnlearned) {
        const sstring howCloseToLearning =
          skill_diff(spell->start - discLearnedness);
        const sstring whenWillLearn =
          format("Learned: %s") % howCloseToLearning.cap();
        thisLine.emplace_back(whenWillLearn);
      } else {
        sstring learnedness = how_good(skillValue);

        if (spellNotYetMaxable) {
          const sstring maxLearnedness = how_good(maxSkillValue);
          learnedness =
            format("%s/%s") % learnedness.trim() % maxLearnedness.trim();
        }

        thisLine.emplace_back(learnedness.trim());
      }

      if (IS_SET(spell->comp_types, COMP_MATERIAL)) {
        const sstring componentInfo =
          getComponentString(character, skillNumber);
        thisLine.emplace_back(componentInfo);
      } else
        thisLine.emplace_back("");

      output.emplace_back(thisLine);
    }

    return output;
  };

  void calcMaxStringWidths(const vector<vector<sstring>>& outputLines,
    size_t& maxSpellNameSize, size_t& maxLearnInfoSize,
    size_t& maxCompInfoSize) {
    for (const auto& line : outputLines) {
      maxSpellNameSize = max(maxSpellNameSize, line.at(0).size());
      maxLearnInfoSize = max(maxLearnInfoSize, line.at(1).size());
      maxCompInfoSize = max(maxCompInfoSize, line.at(2).size());
    }
  };

  sstring generateFormattedOutput(const vector<vector<sstring>>& outputLines,
    const size_t maxSpellNameSize, const size_t maxLearnInfoSize,
    const size_t maxCompInfoSize) {
    sstring output;
    for (const auto& line : outputLines) {
      output += format("    %-s %-s %-s\n\r") %
                boost::io::group(std::setw(maxSpellNameSize), line.at(0)) %
                boost::io::group(std::setw(maxLearnInfoSize), line.at(1)) %
                boost::io::group(std::setw(maxCompInfoSize), line.at(2));
    }

    return (output += "\n\r");
  };

  auto getClassNameAndBitflagFromSpellType(const TBeing* character,
    const skillUseClassT spellType) {
    static const unordered_map<skillUseClassT, uint16_t> classForSpellType = {
      {SPELL_CLERIC, CLASS_CLERIC}, {SPELL_DEIKHAN, CLASS_DEIKHAN},
      {SPELL_MAGE, CLASS_MAGE}, {SPELL_SHAMAN, CLASS_SHAMAN},
      {SPELL_RANGER, CLASS_RANGER}};
    const uint16_t spellClassBitflag = classForSpellType.at(spellType);
    const classIndT classIndex = character->getClassIndNum(spellClassBitflag);
    const sstring className = classInfo[classIndex].name.cap();

    return std::tuple<const sstring, const uint16_t>(className,
      spellClassBitflag);
  };

  sstring getClassNameFromSpellType(const TBeing* character,
    const skillUseClassT spellType) {
    const auto [className, bitFlag] =
      getClassNameAndBitflagFromSpellType(character, spellType);
    return className;
  };

  uint16_t getClassBitflagFromSpellType(const TBeing* character,
    const skillUseClassT spellType) {
    const auto [className, bitFlag] =
      getClassNameAndBitflagFromSpellType(character, spellType);
    return bitFlag;
  };

  sstring generateCategoryHeader(SpellCategory category) {
    static const unordered_map<SpellCategory, sstring> spellCategoryHeaders = {
      {TARGETED_OFFENSIVE, "Targeted Offensive"},
      {TARGETED_UTILITY, "Targeted Utility"},
      {NONTARGETED_OFFENSIVE, "Non-Targeted Offensive"},
      {NONTARGETED_UTILITY, "Non-Targeted Utility"}};

    return format("%s:\n\r") % spellCategoryHeaders.at(category);
  };

  auto getClassesToDisplay(const TBeing* character,
    const uint16_t whichClasses) {
    vector<skillUseClassT> output;

    for (const auto spellType : VALID_SPELL_TYPES) {
      const auto spellClassBitflag =
        getClassBitflagFromSpellType(character, spellType);
      const bool includeSpellsForThisClass =
        whichClasses == 0 || IS_SET(whichClasses, spellClassBitflag);

      if (includeSpellsForThisClass && character->hasClass(spellClassBitflag))
        output.emplace_back(spellType);
    }

    return output;
  };

  auto getCategoriesToDisplay(const bool showTargeted, const bool showUtility,
    const bool showOffensive, const bool showNonTargeted, const bool showAll) {
    vector<SpellCategory> output;

    if ((showTargeted && !showUtility) || (showOffensive && !showNonTargeted) ||
        showAll)
      output.push_back(TARGETED_OFFENSIVE);
    if ((showTargeted && !showOffensive) || (showUtility && !showNonTargeted) ||
        showAll)
      output.push_back(TARGETED_UTILITY);
    if ((showNonTargeted && !showUtility) || (showOffensive && !showTargeted) ||
        showAll)
      output.push_back(NONTARGETED_OFFENSIVE);
    if ((showNonTargeted && !showOffensive) || (showUtility && !showTargeted) ||
        showAll)
      output.push_back(NONTARGETED_UTILITY);

    return output;
  };
}  // namespace

void TBeing::displaySpellInfo(const sstring& argument, const cmdTypeT command) {
  if (!desc)
    return;

  static const uint16_t validClasses =
    CLASS_MAGE | CLASS_SHAMAN | CLASS_CLERIC | CLASS_DEIKHAN | CLASS_RANGER;

  if (!hasClass(validClasses) && !isImmortal()) {
    sendTo("You don't know any spells, prayers, or rituals.\n\r");
    return;
  }

  bool showAll = false, showTargeted = false, showNonTargeted = false,
       showOffensive = false, showUtility = false, showUnlearned = false;

  uint16_t whichClasses = 0U;

  if (argument.empty())
    showAll = true;

  for (const sstring& word : argument.words()) {
    if (is_abbrev(word, "all"))
      showAll = true;
    else if (is_abbrev(word, "unlearned"))
      showUnlearned = true;
    else if (is_abbrev(word, "offensive"))
      showOffensive = true;
    else if (is_abbrev(word, "utility"))
      showUtility = true;
    else if (is_abbrev(word, "targeted"))
      showTargeted = true;
    else if (is_abbrev(word, "nontargeted"))
      showNonTargeted = true;
    else if (is_abbrev(word, "mage"))
      whichClasses |= CLASS_MAGE;
    else if (is_abbrev(word, "shaman"))
      whichClasses |= CLASS_SHAMAN;
    else if (is_abbrev(word, "cleric"))
      whichClasses |= CLASS_CLERIC;
    else if (is_abbrev(word, "deikhan"))
      whichClasses |= CLASS_DEIKHAN;
    else if (is_abbrev(word, "ranger"))
      whichClasses |= CLASS_RANGER;
    else {
      sendTo(
        "Syntax: <prayers|spells|rituals> "
        "[offensive|utility|targeted|nontargeted] "
        "[mage|shaman|cleric|deikhan|ranger] [unlearned]\n\r");
      sendTo("See 'help spells' for more information.\n\r");
      return;
    }
  }

  // Assume user wants to see all if no specific categories are specified
  showAll = showAll ||
            !(showOffensive || showUtility || showTargeted || showNonTargeted);

  // If no class arguments were passed, choose classes based on the command used
  if (whichClasses == 0U) {
    switch (command) {
      case CMD_SPELLS:
        whichClasses = (CLASS_MAGE | CLASS_RANGER);
        break;
      case CMD_PRAYERS:
        whichClasses = (CLASS_CLERIC | CLASS_DEIKHAN);
        break;
      case CMD_RITUALS:
        whichClasses = CLASS_SHAMAN;
        break;
      default:
        break;
    }
  }

  // Create map of maps. Outer map relates a spell type (e.g. SPELL_MAGE) to an
  // inner map. Inner map relates a spell category (e.g. TARGETED_OFFENSIVE) to
  // a vector of spell numbers that match the outer spell type and inner spell
  // category (e.g. SPELL_GUST).

  // Do this once per use of the spells/rituals/prayers command, as the skills
  // are sorted based on the character's current learnedness at the time, so the
  // order can change potentially every call.
  const auto spellsByCategoryByClass = createSpellsByCategoryByClassMap(this);

  // Determine which categories to display based on combination of arguments
  // provided
  const auto categoriesToDisplay = getCategoriesToDisplay(showTargeted,
    showUtility, showOffensive, showNonTargeted, showAll);

  // Determine which classes to display based on arguments provided (or not
  // provided)
  const auto classesToDisplay = getClassesToDisplay(this, whichClasses);

  unordered_map<skillUseClassT,
    unordered_map<SpellCategory, vector<vector<sstring>>>>
    outputLinesByCategoryByClass;

  size_t maxSpellNameSize = 0, maxLearnInfoSize = 0, maxCompInfoSize = 0;

  // Create map of lines to be displayed, allowing us to count the max size of
  // each field before generating any output. This way we can keep all output
  // aligned while not printing a bunch of extra whitespace.
  for (const auto spellType : classesToDisplay) {
    unordered_map<SpellCategory, vector<vector<sstring>>> outputByCategory;
    for (const auto category : categoriesToDisplay) {
      const auto& spells = spellsByCategoryByClass.at(spellType).at(category);
      const auto& outputLines =
        generateOutputLines(this, spells, showUnlearned);
      calcMaxStringWidths(outputLines, maxSpellNameSize, maxLearnInfoSize,
        maxCompInfoSize);
      outputByCategory.emplace_hint(outputByCategory.end(), category,
        outputLines);
    }
    outputLinesByCategoryByClass.emplace_hint(
      outputLinesByCategoryByClass.end(), spellType, outputByCategory);
  }

  sstring finalOutput;

  for (const auto category : categoriesToDisplay) {
    finalOutput += generateCategoryHeader(category);
    for (const auto spellType : classesToDisplay) {
      const auto& outputLines =
        outputLinesByCategoryByClass.at(spellType).at(category);
      if (outputLines.empty())
        continue;

      finalOutput +=
        format("  %s:\n\r") % getClassNameFromSpellType(this, spellType);
      finalOutput += generateFormattedOutput(outputLines, maxSpellNameSize,
        maxLearnInfoSize, maxCompInfoSize);
    }
  }

  desc->page_string(finalOutput);
}
