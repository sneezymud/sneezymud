//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// db.h , Database module.
// Usage: Loading/Saving chars booting world.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <queue>

#include "sstring.h"
#include "structs.h"

// Strings for the mud name
// also: WELC_MESSG needs to be updated if these change
// also: SNEEZY_ADMIN
extern const char* const MUD_NAME;
extern const char* const MUD_NAME_VERS;
extern bool bootTime;

inline constexpr int MAX_OBJ_AFFECT = 5;

class File {
  public:
    static const char* const SIGN_MESS;
    static const char* const MOB;
    static const char* const ZONE;
    static const char* const CREDITS;
    static const char* const NEWS;
    static const char* const STORY;
    static const char* const WIZNEWS;
    static const char* const MOTD;
    static const char* const WIZMOTD;
    static const char* const TIME;
    static const char* const IDEA;
    static const char* const TYPO;
    static const char* const BUG;
    static const char* const SOCMESS;
    static const char* const HELP_PAGE;
    static const char* const WIZLIST;

    File() = delete;
};

class Path {
  public:
    static const char* const HELP;
    static const char* const DATA;
    static const char* const IMMORTAL_HELP;
    static const char* const BUILDER_HELP;
    static const char* const SKILL_HELP;
    static const char* const SPELL_HELP;

    Path() = delete;
};

inline constexpr const char* const MUDADMIN_EMAIL = "support@sneezymud.org";
inline constexpr const char* const CODERS_EMAIL = "support@sneezymud.org";

inline constexpr int WORLD_SIZE = 50000;
inline constexpr int ZONE_ROOM_RANDOM = -99;

/* public procedures in db.c */

void bootDb();
class TDatabase;
void bootOneZone(TDatabase&, int, int&);
int create_entry(char* name);
void zone_update();
int real_object(int);
int real_mobile(int);

// forward class decl
class resetCom;
class armorSetLoad;
class zoneData;
class extraDescription;
class indexData;
class objIndexData;
class mobIndexData;
class resetQElement;

enum readFileTypeT {
  REAL,
  VIRTUAL
};

using resetFlag = uint32_t;
inline constexpr resetFlag resetFlagNone = 0;
inline constexpr resetFlag resetFlagBootTime = 1U << 0U;
inline constexpr resetFlag resetFlagFindLoadPotential = 1U << 1U;
inline constexpr resetFlag resetFlagPropLoad = 1U << 2U;

inline constexpr resetFlag resetFlagCount = 3;
inline constexpr resetFlag resetFlagMax = 1U << resetFlagCount;

class resetCom {
  public:
    char command{'\0'};
    int if_flag{0};
    int arg1{0};
    int arg2{0};
    int arg3{0};
    int arg4{0};
    char character{'\0'};
    int cmd_no{0};

    resetCom();

    bool hasLoadPotential();
    bool usesRandomRoom();
    bool shouldStickToMob(bool& lastComStuck);

    // returns false to stop execution (critical fail or stop command)
    bool execute(zoneData& zone,
                 resetFlag flags,
                 bool& mobload,
                 TMonster*& mob,
                 bool& objload,
                 TObj*& obj,
                 bool& last_cmd);

  private:
    enum resetCommandId {
      cmd_Stop = 0,             // S
      cmd_LoadMob,              // M
      cmd_LoadMobGrouped,       // K
      cmd_LoadMobCharmed,       // C
      cmd_LoadMobRidden,        // R
      cmd_SetRandomRoom,        // A
      cmd_LoadChance,           // ?
      cmd_LoadObjGround,        // B
      cmd_LoadObjGroundBoot,    // O
      cmd_LoadObjPlaced,        // P
      cmd_LoadObjInventory,     // G
      cmd_LoadObjEquipped,      // E
      cmd_CreateLocalSet,       // X
      cmd_LoadObjSetLocal,      // Z
      cmd_LoadObjSet,           // Y
      cmd_ChangeFourValues,     // V
      cmd_SetTrap,              // T
      cmd_SetHate,              // H
      cmd_SetFear,              // F
      cmd_SetDoor,              // D
      cmd_LoadLoot,             // L
      cmd_LoadObjEquippedProp,  // I
      cmd_LoadObjSetLocalProp,  // J

      cmd_Max
    };

    using exec_fn = void(zoneData&, resetCom&, resetFlag, bool&, TMonster*&, bool&, TObj*&, bool&);
    static exec_fn* executeMethods[cmd_Max];

    resetCommandId getCommandId();
};

class armorSetLoad {
  private:
    struct armor_set_struct {
        int slots[24];  // should be MAX_WEAR
    } local_armor[16];

  public:
    armorSetLoad();
    void resetArmor();
    void setArmor(int set, int slot, int value);
    int getArmor(int set, int slot);
};

class zoneData {
  public:
    sstring name{};   // name of this zone
    int zone_nr{0};   // number of this zone
    int lifespan{0};  // how long between resets (minutes)
    int age{0};       // current age of this zone (minutes)
    int bottom{0};
    int top{0};           // upper limit for rooms in this zone
    int reset_mode{0};    // conditions for reset (see below)
    bool enabled{false};  // whether zone is enabled
    byte zone_value{0};
    unsigned int num_mobs{0};
    double mob_levels{0};
    double min_mob_level{0};
    double max_mob_level{128};
    int random_room{0};

    // the following stat_* variables are intended to be used for zone reporting in the stat zone
    // command they are not air-tight counts and should not be treated as such
    std::map<int, int>
      stat_mobs{};  // key: real mob number, value: count of that mob loading in the zonefile
    std::map<int, int>
      stat_objs{};  // key: real obj number, value: count of that obj loading in the zonefile
    // note the count value for stat_objs ignores things like load rates so is pretty useless info
    // it also doesn't contain global suitset objs and doesn't check to see if local suitsets
    // actually load, so...
    int stat_mobs_total{0};   // total # of mobs loading in the zonefile
    int stat_mobs_unique{0};  // unique # of mobs loading in the zonefile
    int stat_objs_unique{0};  // unique # of objects loading in the zonefile

    std::vector<resetCom> cmd_table{};  // command table for reset

    armorSetLoad armorSets{};

    bool isEmpty();
    void resetZone(bool bootTime, bool findLoadPotential = false);
    void closeDoors();
    void logError(char, const char*, int, int);
    void nukeMobs();
    void sendTo(sstring, int exclude_room = -1);
    bool doGenericReset();
    bool bootZone(int);
    void renumCmd();
};

class indexData {
  public:
    int virt{0};
    long pos{0};
    sstring name{};
    sstring short_desc{};
    sstring long_desc{};
    sstring description{};
    int max_exist{-99};
    int spec{0};
    float weight{0};

    void addToNumber(const short int n) { number += n; }

    void setMaxNumber(const short int n) { max_num = n; }

    int getNumber() const { return number; }

    int getMaxNumber() const { return max_num; }

    indexData() = default;
    indexData(const indexData&) = default;
    indexData& operator=(const indexData&) = default;
    indexData(indexData&&) = default;
    indexData& operator=(indexData&&) = default;
    virtual ~indexData() = default;

  private:
    int number{0};
    int max_num{0};
};

class objIndexData : public indexData {
  public:
    byte max_struct{-99};
    short armor{-99};
    unsigned int where_worn{0};
    ubyte itemtype{0};
    int value{-99};
    extraDescription* ex_description{nullptr};
    objAffData affected[MAX_OBJ_AFFECT]{};

    objIndexData() = default;
    objIndexData(const objIndexData&);

    // Don't allow copy assignment, as ex_description requires a
    // deep copy. Only allow through copy constructor.
    objIndexData& operator=(const objIndexData&) = delete;
    objIndexData(objIndexData&&) = default;
    objIndexData& operator=(objIndexData&&) = default;
    ~objIndexData() override;
};

class mobIndexData : public indexData {
  public:
    long faction{-99};
    long Class{-99};
    long level{-99};
    long race{-99};
    bool doesLoad{false};
    int numberLoad{0};
};

class resetQElement {
  public:
    unsigned int zone_to_reset{0};
    resetQElement* next{nullptr};
};

extern std::queue<sstring> queryqueue;
