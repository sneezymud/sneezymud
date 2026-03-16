/*
  Room special procedures may be called under five different conditions:
    - By a player command
    - When the room is first allocated in memory
    - During the room's zone reset
    - When the room is deallocated from memory
    - Periodically (every PULSE_MOBILE heartbeats - at the time of writing, a
  little over 7 seconds)

  A room's special procedure must return int.

  A return value of zero (false) indicates that nothing exceptional happened.

  A non-zero return value (true) indicates that some action needs to be taken
  (such as ignoring the player's command). When in doubt, return false.

  Parameters:
    ch  - The mob/player that triggered the room's special procedure.
    cmd - What triggered the procedure. A value greater than zero indicates
          a player command. A value less than zero indicates something like
          ROOM_RESET.
    arg - Non-null only when catching a player command.
    rp  - A pointer to the room for which the spec proc was called.
*/

#pragma once

#include <algorithm>
#include <array>

class TBeing;
class TRoom;
enum cmdTypeT : int;

using RoomProcFn = int(TBeing*, cmdTypeT, const char*, TRoom*);
using RoomProc = RoomProcFn*;

// Forward declarations of all room special procedures
RoomProcFn BankMainEntrance;
RoomProcFn BankVault;
RoomProcFn SecretDoors;
RoomProcFn SecretPortalDoors;
RoomProcFn Whirlpool;
RoomProcFn bankRoom;
RoomProcFn belimusBlowHole;
RoomProcFn belimusLungs;
RoomProcFn belimusStomach;
RoomProcFn belimusThroat;
RoomProcFn boulderRoom;
RoomProcFn dayGateRoom;
RoomProcFn duergarWater;
RoomProcFn emergency_room;
RoomProcFn healing_room;
RoomProcFn monkQuestProcFall;
RoomProcFn monkQuestProcLand;
RoomProcFn moonGateRoom;
RoomProcFn noiseBoom;
RoomProcFn oft_frequented_room;
RoomProcFn prisonDump;
RoomProcFn randomMobDistribution;
RoomProcFn sleepTagControl;
RoomProcFn sleepTagRoom;
RoomProcFn slide;
RoomProcFn theKnot;
RoomProcFn weirdCircle;
RoomProcFn blazingroom;

struct TRoomSpecs {
    int id;
    bool assignable;
    const char* name;
    RoomProc proc;
};

inline constexpr std::array roomSpecials = std::to_array<TRoomSpecs>({
  {2, false, "Bank Main Entrance", BankMainEntrance},
  {3, false, "Bank Vault", BankVault},
  {4, false, "Secret Doors", SecretDoors},
  {5, false, "Secret Portal Doors", SecretPortalDoors},
  {6, false, "Whirlpool", Whirlpool},
  {7, false, "Bank Room", bankRoom},
  {8, false, "belimus Blow Hole", belimusBlowHole},
  {9, false, "belimus Lungs", belimusLungs},
  {10, false, "belimus Stomach", belimusStomach},
  {11, false, "belimus Throat", belimusThroat},
  {12, false, "boulder Room", boulderRoom},
  {14, false, "dayGate Room", dayGateRoom},
  {15, false, "duergar Water", duergarWater},
  {17, false, "emergency room", emergency_room},
  {19, false, "healing room", healing_room},
  {20, false, "monk Quest Proc Fall", monkQuestProcFall},
  {21, false, "monk Quest Proc Land", monkQuestProcLand},
  {22, false, "moonGate Room", moonGateRoom},
  {23, false, "noise Boom", noiseBoom},
  {24, false, "oft frequented room", oft_frequented_room},
  {26, false, "prison Dump", prisonDump},
  {27, false, "random Mob Distribution", randomMobDistribution},
  {28, false, "sleep Tag Control", sleepTagControl},
  {29, false, "sleep Tag Room", sleepTagRoom},
  {30, false, "slide", slide},
  {31, false, "the Knot", theKnot},
  {32, false, "weird Circle", weirdCircle},
  {33, true, "blazingroom", blazingroom},
});

// Compile-time check: no duplicate IDs in the array.
static_assert(
  [] {
    std::array<int, roomSpecials.size()> ids{};
    for (size_t i = 0; i < roomSpecials.size(); ++i) {
      ids[i] = roomSpecials[i].id;
    }
    std::ranges::sort(ids);
    return std::ranges::adjacent_find(ids) == ids.end();
  }(),
  "roomSpecials contains duplicate IDs");

inline constexpr const TRoomSpecs* findRoomSpec(int id) {
  const auto* found = std::ranges::find_if(roomSpecials,
    [id](const TRoomSpecs& spec) { return spec.id == id; });
  return found != roomSpecials.end() ? &(*found) : nullptr;
}

inline constexpr const char* getRoomSpecName(int id) {
  const auto* spec = findRoomSpec(id);
  return spec ? spec->name : "None";
}
