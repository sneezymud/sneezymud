//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


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
#include <deque>
#include <queue>

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

#include <boost/regex.hpp>
#include <boost/format.hpp>
using namespace boost;


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
#include "obj.h"
#include "obj_mergeable.h"
#include "room.h"
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


#endif
