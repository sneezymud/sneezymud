//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "cmd_innate.cc" - Racial innate ability command and registry
//
//////////////////////////////////////////////////////////////////////////

#include <array>

#include "being.h"
#include "cmd_innate.h"
#include "extern.h"
#include "race.h"
#include "room.h"

namespace {
  struct InnateAbility;
  using InnateHandler = int (*)(TBeing*, const InnateAbility&, const char*);

  // Append entries here to register a new racial innate. The handler
  // receives the registry entry, so cooldownTag is the single source of
  // truth for both the cooldown affect and the UI in showInnateList.
  struct InnateAbility {
      const char* name;
      race_t race;
      InnateHandler handler;
      spellNumT cooldownTag;
  };

  // Drow darkness room affect — TRoom::getLight() returns 0 when this
  // affect is on the room, blinding occupants without infravision/true sight.
  // No onTick needed: the affect itself is the source of truth, and the
  // duration counter is decremented by tickRoomAffects automatically.
  sstring drowDarknessLookDesc(bool here) {
    return (format("<k>An unnatural pall of magical darkness shrouds the "
                   "air %s.<z>\n\r") %
             (here ? "here" : "there"))
      .str();
  }

  void drowDarknessRoomExpire(TRoom* room, const RoomAffectData&) {
    MakeNoise(room->in_room,
      "<k>The unnatural darkness lifts and fades.<z>\n\r",
      "<k>The unnatural darkness from nearby lifts and fades.<z>\n\r");
  }

  const RoomAffectVTable kDrowDarknessRoomAffect = {
    .onExpire = &drowDarknessRoomExpire,
    .lookDesc = &drowDarknessLookDesc,
  };

  // ~4 real min at SPEC_PROCS cadence (3.6s/tick)
  inline constexpr int kDrowDarknessTicks = 67;
  inline constexpr int kInnateCooldownHours = 12;

  void installInnateCooldown(TBeing* ch, spellNumT tag, int mudHours) {
    affectedData cd;
    cd.type = tag;
    cd.duration = mudHours * Pulse::UPDATES_PER_MUDHOUR;
    cd.location = APPLY_NONE;
    cd.modifier = 0;
    cd.bitvector = 0;
    ch->affectTo(&cd, -1);
  }

  int doDrowInvis(TBeing* ch, const InnateAbility& entry, const char*) {
    if (ch->roomp->isRoomFlag(ROOM_NO_MAGIC)) {
      ch->sendTo(
        "Some force here prevents you from drawing on the shadows.\n\r");
      return false;
    }
    if (ch->affectedBySpell(entry.cooldownTag)) {
      ch->sendTo("You cannot draw on the shadows again so soon.\n\r");
      return false;
    }

    affectedData inv;
    inv.type = SPELL_INVISIBILITY;
    inv.level = ch->GetMaxLevel();
    inv.duration = 2 * Pulse::UPDATES_PER_MUDHOUR;
    inv.modifier = -40;
    inv.location = APPLY_ARMOR;
    inv.bitvector = AFF_INVISIBLE;
    if (!ch->affectJoin(ch, &inv, AVG_DUR_NO, AVG_EFF_YES))
      return false;

    installInnateCooldown(ch, entry.cooldownTag, kInnateCooldownHours);

    act("$n melts into the shadows.", false, ch, nullptr, nullptr, TO_ROOM);
    ch->sendTo("Shadows wrap around you and you fade from sight.\n\r");
    return true;
  }

  int doDrowDarkness(TBeing* ch, const InnateAbility& entry, const char*) {
    // SKILL_DROW_DARKNESS is intentionally used as two distinct tag values
    // on different containers, with no collision:
    //   - this->affected (per-being list): the player's cooldown timer
    //   - roomp->roomAffects (per-room vector): the active darkness effect
    if (ch->roomp->isRoomFlag(ROOM_NO_MAGIC)) {
      ch->sendTo(
        "Some force here prevents you from drawing on the shadows.\n\r");
      return false;
    }
    if (ch->roomp->isRoomFlag(ROOM_PEACEFUL)) {
      ch->sendTo("This place is too tranquil to corrupt with shadow.\n\r");
      return false;
    }
    // ROOM_ALWAYS_LIT short-circuits TRoom::getLight() above the darkness
    // check, so the affect would have no visible effect here. Refuse upfront
    // to avoid burning the cooldown for nothing.
    if (ch->roomp->isRoomFlag(ROOM_ALWAYS_LIT)) {
      ch->sendTo(
        "The light here is too pure for your shadows to take hold.\n\r");
      return false;
    }
    if (ch->affectedBySpell(entry.cooldownTag)) {
      ch->sendTo("You cannot summon the darkness again so soon.\n\r");
      return false;
    }

    if (auto* existing = ch->roomp->getRoomAffect(SKILL_DROW_DARKNESS)) {
      existing->duration = kDrowDarknessTicks;
      existing->level = ch->GetMaxLevel();
      existing->casterID = ch->getPlayerID();
    } else {
      RoomAffectData aff;
      aff.type = SKILL_DROW_DARKNESS;
      aff.duration = kDrowDarknessTicks;
      aff.level = ch->GetMaxLevel();
      aff.casterID = ch->getPlayerID();
      ch->roomp->addRoomAffect(aff);
    }

    installInnateCooldown(ch, entry.cooldownTag, kInnateCooldownHours);

    act("$n weaves the shadows together, smothering the room in darkness.",
      false, ch, nullptr, nullptr, TO_ROOM);
    ch->sendTo(
      "You weave the shadows together, smothering the room in darkness.\n\r");

    MakeNoise(ch->roomp->in_room,
      "<k>An unnatural darkness creeps into the room.<z>\n\r",
      "<k>An unnatural darkness creeps in from nearby.<z>\n\r");
    return true;
  }

  inline constexpr std::array kInnates = {
    InnateAbility{"invis", RACE_DROW, &doDrowInvis, SKILL_DROW_INVIS},
    InnateAbility{"darkness", RACE_DROW, &doDrowDarkness, SKILL_DROW_DARKNESS},
  };

  bool raceHasInnates(race_t race) {
    return std::ranges::any_of(kInnates,
      [race](const auto& innate) { return innate.race == race; });
  }

  void showInnateList(TBeing* ch) {
    race_t race = ch->getMyRace()->getRace();
    if (!raceHasInnates(race)) {
      ch->sendTo("Your race has no innate abilities.\n\r");
      return;
    }
    sstring out = "<W>Racial innate abilities:<z>\n\r";
    for (const auto& innate : kInnates) {
      if (race != innate.race)
        continue;
      const affectedData* aff = nullptr;
      for (const affectedData* a = ch->affected; a; a = a->next) {
        if (a->type == innate.cooldownTag) {
          aff = a;
          break;
        }
      }
      if (aff) {
        out += format("  %-12s ready in %s\n\r") % innate.name %
               describeDuration(ch, aff->duration);
      } else {
        out += format("  %-12s <g>ready<z>\n\r") % innate.name;
      }
    }
    ch->sendTo(out);
  }
}  // namespace

// Called from buildSpellArray() at boot to wire up the room-affect vtable.
void registerDrowDarknessRoomAffect() {
  registerRoomAffect(SKILL_DROW_DARKNESS, &kDrowDarknessRoomAffect);
}

int TBeing::doInnate(const char* arg) {
  sstring input = sstring(arg).trim();
  if (input.empty()) {
    showInnateList(this);
    return true;
  }
  // First-match wins on ambiguous prefix; type more characters to disambiguate.
  sstring first = input.word(0).lower();
  for (const auto& innate : kInnates) {
    if (getMyRace()->getRace() != innate.race)
      continue;
    sstring name(innate.name);
    if (name.find(first) == 0) {
      return innate.handler(this, innate, arg);
    }
  }
  sendTo("You have no such innate ability. Try 'innate' for a list.\n\r");
  return false;
}
