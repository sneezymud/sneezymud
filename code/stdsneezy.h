//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// These are the headers needed by ALL sneezy modules.
// They are represented by GEN_HEAD in the makefile

#ifndef __STD_SNEEZY
#define __STD_SNEEZY

// defines needed for other header files...

// some faction code is bad  (faction%) without fully enabled factions
#define FACTIONS_IN_USE 0

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sys/types.h>
#include <cctype>
#include <cassert>
#include <cstring>
#include <stdarg.h>
#include <cmath>
#include <cerrno>
#include <string>
#include <vector>
#include <map>
#include <list>

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;


#include "sstring.h"
#include "low.h"
#include "enum.h"
#include "spells.h"
#include "structs.h"
#include "immunity.h"
// #include "damage.h"
#include "discipline.h"
#include "spell2.h"
#include "parse.h"
#include "db.h"
#include "ansi.h"
#include "cmd_dissect.h"
#include "client.h"
#include "response.h"
#include "faction.h"
#include "comm.h"
// #include "spec_objs.h"
#include "toggle.h"
#include "wiz_powers.h"
#include "cmd_message.h"
#include "stats.h"
#include "limbs.h"
#include "body.h"
#include "race.h"
#include "skills.h"
#include "disease.h"
#include "trap.h"
#include "task.h"
#include "weather.h"
#include "sound.h"
#include "create.h"
#include "thing.h"
#include "room.h"
#include "obj.h"
#include "connect.h"
//#include "being.h"
#include "charfile.h"
#include "person.h"
#include "monster.h"
#include "account.h"
#include "extern.h"
#include "handler.h"
//#include "rent.h"
#include "spec_mobs.h"
//#include "spec_rooms.h"
#include "materials.h"
#include "colorstring.h"
#include "format.h"



// defines the port of the running muds
extern const int PROD_GAMEPORT;
extern const int BETA_GAMEPORT;
extern       int GAMMA_GAMEPORT; // quick boot
extern const int ALPHA_GAMEPORT;
extern const int BUILDER_GAMEPORT;

// causes player/rent files to be automatically purged if inactive for
// more then a few weeks.  Conserves disk space and speeds up the boot
// process significantly.  Periods of college breaks are bypassed.
extern bool auto_deletion;

// requires auto-deletion turned on
// causes deletion only of the rent file.  Otherwise pfile, rent and account go
extern bool rent_only_deletion;

// Causes mobs in inactive zones to be deleted.  Typically, 50% of the mud's
// mobs would qualify.  Dramatically speeds up the mobileActivity loop and
// improves CPU performance.
extern bool nuke_inactive_mobs;

// causes items left in repair to be deleted after a set number of days.
// Good to keep things circulating, but bad if extended downtime anticipated.
// Simply deletes the file in mobdata/repairs/, the tickets still exist
// and the repairman will say he doesn't have the item.
#define NUKE_REPAIR_ITEMS    1

// Enables a check to validate that players are not multiplaying. 
// check is done each login and periodically for all chars logged in.
#define CHECK_MULTIPLAY     1
// code will disallow any bad multiplay event
#define FORCE_MULTIPLAY_COMPLIANCE     1

// Enables automatic generation of repossession mobs based on item max-exist.
// Not extremely popular, but a good way to get item overturn.
#define REPO_MOBS         1
// items that are over max-exist get hunted by a buffed up version of the
// hunter.  Requires REPO_MOBS be TRUE.
// VERY unpopular
#define SUPER_REPO_MOBS   0

// shops tend to get a lot of goods that strictly speaking aren't isSimilar()
// slightly damaged, depreciated, etc.
// We can eliminate this by turning this on.  Any item not perfect will get
// deleted.
#define NO_DAMAGED_ITEMS_SHOP    0


// hard coded limits on builder powers, toggled on/off using with wizpower #111

#define LIMITPOWERS 1

// ********************************************************************
// ECONOMIC BALANCE

// modifies rate at which items take damage
// The higher the number, the lower the damage
// Any hit doing less then this amount has no chance of damaging
// All other hits get modifier on damage rate based on this value
// 4.1 balanced at 2 prior to depreciation
extern const int ITEM_DAMAGE_RATE;

// used to determine rent credit
// credit = level * maxi(20, level) * x
// the values of the "model" items (soft leather, plate, etc) were set based
// on this rent credit being at 75
// 4.0's no rent, and 4.1's big rent credit have distorted this so feel free to
// adjust if this no longer works
extern const int RENT_CREDIT_VAL;

// if player goes over rent, items are "sold" to pay for it
// this handles what is done with the sold item(s)
// If turned on, the pawnguy gets them
// otherwise they get deleted
extern const bool RENT_SELL_TO_PAWN;

// causes innkeepers to grant rentCredit based on the innkeepers levels
// otherwise, it is based on the players level.
// the chief use of this is to encourage high level pc's to use certain inns
extern const bool RENT_RESTRICT_INNS_BY_LEVEL;

// causes bad things to happen to player based on time in autorent
// there is a grace period to handle crashes
#define PENALIZE_FOR_AUTO_RENTING     1

// the minimum "hardness" for a material to damage/blunt a weapon when hitting.
extern const int WEAPON_DAM_MIN_HARDNESS;

// the max value of a hardness roll, raising it = weapon damage/blunt DECREASE
extern const int WEAPON_DAM_MAX_HARDNESS;

// the max value of a sharpness roll, raising it = weapon blunt DECREASE
extern const int WEAPON_DAM_MAX_SHARP;


#define SPEEF_MAKE_BODY 0

#endif
