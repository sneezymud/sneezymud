#pragma once

// Integration test fixture providing lightweight game state for testing
// functions that need characters, rooms, and spells but not a database
// or running server.
//
// Cleanup design: Characters stay in their rooms during deletion.
// ~TPerson moves inventory/equipment to the room via dropItemsToRoom().
// ~TBeing sees roomp is valid and skips the thing_to_room(Room::VOID)
// path (which would crash without a room database).
// ~TRoom deletes any non-PC objects left in the room.
// Components/containers placed into characters are owned by the game's
// object lifecycle — the fixture does NOT retain ownership of them.

#include <cassert>
#include <gtest/gtest.h>
#include <memory>
#include <utility>
#include <vector>

#include "being.h"
#include "comm.h"
#include "connect.h"
#include "db.h"
#include "enum.h"
#include "extern.h"
#include "obj_chest.h"
#include "obj_component.h"
#include "person.h"
#include "race.h"
#include "room.h"
#include "socket.h"
#include "spells.h"
#include "toggle.h"

// RAII wrapper for a test character and its associated descriptor/socket.
// Handles the messy cleanup sequence required by safety invariants.
struct TestCharacter {
    TSocket* socket = nullptr;
    Descriptor* desc = nullptr;
    TPerson* ch = nullptr;

    TestCharacter() = default;
    TestCharacter(const TestCharacter&) = delete;
    TestCharacter& operator=(const TestCharacter&) = delete;

    TestCharacter(TestCharacter&& other) noexcept :
      socket(other.socket),
      desc(other.desc),
      ch(other.ch) {
      other.socket = nullptr;
      other.desc = nullptr;
      other.ch = nullptr;
    }

    TestCharacter& operator=(TestCharacter&& other) noexcept {
      if (this != &other) {
        destroy();
        socket = other.socket;
        desc = other.desc;
        ch = other.ch;
        other.socket = nullptr;
        other.desc = nullptr;
        other.ch = nullptr;
      }
      return *this;
    }

    ~TestCharacter() { destroy(); }

    void destroy() {
      if (ch) {
        ch->reformGroup();
        DeleteHatreds(ch, nullptr);
        DeleteFears(ch, nullptr);

        // Sever the character↔descriptor link BEFORE deletion.
        // ~TBeing tries to call doAccountMenu() on the descriptor and
        // remove from character_list via a desc->connected check.
        // With desc null, ~TBeing skips all descriptor processing
        // and still removes from character_list correctly.
        // ~TPerson::dropItemsToRoom() doesn't need the descriptor.
        if (desc) {
          desc->character = nullptr;
          ch->desc = nullptr;
        }

        // ~TPerson drops items to room, ~TBeing removes from room
        // (via --(*this)) and from character_list.
        delete ch;
        ch = nullptr;
      }
      if (desc) {
        // ~Descriptor removes itself from descriptor_list, calls
        // close() on the socket (close(-1) logs a warning to stderr),
        // and deletes the socket. character is already null so
        // ~Descriptor skips the heavy character cleanup path.
        delete desc;
        desc = nullptr;
        socket = nullptr;  // ~Descriptor deleted socket
      } else if (socket) {
        delete socket;
        socket = nullptr;
      }
    }

    // Drain all queued output and return concatenated text
    [[nodiscard]] sstring drainOutput() const {
      sstring result;
      while (!desc->output.empty()) {
        result += desc->output.front()->getComm();
        desc->output.pop();
      }
      return result;
    }
};

class GameFixture : public ::testing::Test {
  protected:
    // One-time setup: initialize spell array and race data.
    // These are global state that persists across all tests in the suite.
    static void SetUpTestSuite() {
      if (!suiteInitialized) {
        buildSpellArray();

        for (int i = 0; i < MAX_RACIAL_TYPES; i++) {
          if (!Races[i]) {
            Races[i] = new Race(static_cast<race_t>(i));
          }
        }

        suiteInitialized = true;
      }
    }

    void SetUp() override {
      savedDescriptorList = descriptor_list;
      savedCharacterList = character_list;
    }

    void TearDown() override {
      // Characters must be destroyed BEFORE rooms. ~TPerson drops items
      // to the room, and ~TBeing removes from room and character_list.
      characters.clear();
      rooms.clear();

      descriptor_list = savedDescriptorList;
      character_list = savedCharacterList;
    }

    // Create a test character with a descriptor and socket.
    // Set to POSITION_STANDING, CON_PLYNG, RACE_HUMAN, level 1 —
    // the minimum state for act() and similar functions to work.
    // Call placeInRoom() to put the character in a room.
    TestCharacter& makeCharacter(const sstring& charName) {
      auto tc = std::make_unique<TestCharacter>();
      tc->socket = new TSocket();
      tc->socket->m_sock = -1;  // no real socket
      tc->desc = new Descriptor(tc->socket);
      tc->desc->connected = CON_PLYNG;

      tc->ch = new TPerson(tc->desc);
      tc->ch->name = charName;
      tc->ch->shortDescr = charName;
      tc->ch->setRace(RACE_HUMAN);
      tc->ch->setMaxLevel(1);
      tc->ch->setPosition(POSITION_STANDING);
      tc->ch->addPlayerAction(PLR_RT_HANDED);

      // Register in character_list so ~TBeing can remove naturally.
      tc->ch->next = character_list;
      character_list = tc->ch;

      characters.push_back(std::move(tc));
      return *characters.back();
    }

    // Create a room with ROOM_ALWAYS_LIT so visibility checks pass.
    // Vnum must be < WORLD_SIZE (50000). The room is registered in room_db
    // so ~TRoom can clean up without warnings.
    TRoom& makeRoom(int vnum) {
      assert(vnum >= 0 && vnum < WORLD_SIZE);
      auto room = std::make_unique<TRoom>(vnum);
      room->setRoomFlagBit(ROOM_ALWAYS_LIT);
      room_db[vnum] = room.get();
      rooms.push_back(std::move(room));
      return *rooms.back();
    }

    // Place a test character into a room.
    static void placeInRoom(TestCharacter& tc, TRoom& room) { room += *tc.ch; }

    // Create a spell component. Ownership transfers to whatever container
    // or character the component is placed into (via operator+= or equipChar).
    // The game's destruction chain handles cleanup.
    static TComponent* makeComponent(spellNumT spell, int charges) {
      auto* comp = new TComponent();
      comp->setComponentSpell(spell);
      comp->setComponentType(COMP_SPELL);
      comp->setComponentCharges(charges);
      comp->name = "a spell component";
      comp->shortDescr = "a spell component";
      return comp;
    }

    // Create a container (TChest). Ownership transfers to whatever entity
    // it is placed into. Open by default; call addContainerFlag(CONT_CLOSED)
    // to close.
    static TChest* makeContainer() {
      auto* chest = new TChest();
      chest->name = "a chest";
      chest->shortDescr = "a chest";
      return chest;
    }

  private:
    static inline bool suiteInitialized = false;
    Descriptor* savedDescriptorList = nullptr;
    TBeing* savedCharacterList = nullptr;

    // Owned test objects, destroyed in TearDown.
    // Characters destroyed before rooms (order matters).
    std::vector<std::unique_ptr<TestCharacter>> characters;
    std::vector<std::unique_ptr<TRoom>> rooms;
};
