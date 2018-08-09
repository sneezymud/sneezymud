#include "person.h"
#include "database.h"
#include "account.h"
#include "pathfinder.h"

#include <unordered_map>

class TPersonPimpl
{
  public:
    std::unordered_map<std::string, int> favoriteRooms;
};

TPerson::TPerson(Descriptor *thedesc) :
  TBeing(),
  base_age(0),
  tLogFile(NULL),
  title(NULL),
  timer(0)
{
  d = new TPersonPimpl();
  *lastHost = '\0';
  memset(toggles, 0, sizeof(toggles));
  memset(wizPowers, 0, sizeof(wizPowers));
  memset(wizPowersOriginal, 0, sizeof(wizPowersOriginal));

  desc = thedesc;

  // default case for TBeing is to add to mobCount, don't count pc's as mobs
  mobCount--;

  // this resets some values
  // rememebr that it could init some desc stuff, then have new char
  // come in causing bad settings.
  desc->session.setToZero();
  desc->prompt_d.xptnl = 0;

  AccountStats::player_num++;
  AccountStats::max_player_since_reboot = max(AccountStats::max_player_since_reboot, AccountStats::player_num);
}

TPerson::TPerson(const TPerson &a) :
  TBeing(a),
  base_age(a.base_age),
  tLogFile(a.tLogFile),
  timer(a.timer)
{
  d = new TPersonPimpl(*a.d);
  title = mud_str_dup(a.title);
  strcpy(lastHost, a.lastHost);
  memcpy(toggles, a.toggles, sizeof(toggles));
  memcpy(wizPowers, a.wizPowers, sizeof(wizPowers));
  memcpy(wizPowersOriginal, a.wizPowers, sizeof(wizPowersOriginal));

  AccountStats::player_num++;
  AccountStats::max_player_since_reboot = max(AccountStats::max_player_since_reboot, AccountStats::player_num);
}

TPerson & TPerson::operator=(const TPerson &a)
{
  if (this == &a) return *this;
  TBeing::operator=(a);
  d = new TPersonPimpl(*a.d);
  base_age = a.base_age;
  timer = a.timer;

  delete [] title;
  title = mud_str_dup(a.title);

  strcpy(lastHost, a.lastHost);
  memcpy(toggles, a.toggles, sizeof(toggles));
  memcpy(wizPowers, a.wizPowers, sizeof(wizPowers));
  memcpy(wizPowersOriginal, a.wizPowers, sizeof(wizPowersOriginal));
  return *this;
}

TPerson::~TPerson()
{
  Descriptor *t_desc;

  if (!desc) {
    for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next) {
      if (t_desc->original && t_desc->original == this) {
        t_desc->character->remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
        t_desc->character->doReturn("", WEAR_NOWHERE, true);
      }
    }
  }

  setInvisLevel(MAX_IMMORT+1);
  fixClientPlayerLists(TRUE);

  // We use to let this be a handler for quit
  // however, if we accidentally delete a player (bad return code?)
  // this gets called and duplicates items
  // quit should now have similar code to what was here, so regard 
  // getting here as an error.
  dropItemsToRoom(SAFE_NO, DROP_IN_ROOM);

  AccountStats::player_num--;

  delete [] title;
  title = NULL;

  if (tLogFile) {
    logf("Logging out...");
    fclose(tLogFile);
    tLogFile = NULL;
  }
  delete d;
  d = nullptr;
}

std::pair<bool, int> TPerson::doPersonCommand(cmdTypeT cmd, const sstring & argument, TThing *, bool)
{
  switch (cmd) {
    case CMD_MAP:
      doMap(argument);
      break;
    default:
      return std::make_pair(false, 0);
  }
  return std::make_pair(true, 0);
}

void TPerson::loadMapData()
{
  TDatabase db(DB_SNEEZY);
  db.query("select name, room from savedrooms where player_id = %i", getPlayerID());
  while (db.fetchRow())
    d->favoriteRooms[db["name"]] = convertTo<int>(db["room"]);
}

void TPerson::doMapList(sstring const& arg) const
{
  sstring filter = arg.lower();

  bool found = false;
  for (const auto& pair : d->favoriteRooms) {
    if (filter.empty() || sstring(pair.first).lower().find(filter) != sstring::npos) {
      sendTo(format("%s -> %i\n") % pair.first % pair.second);
      found = true;
    }
  }

  if (!found) {
    sendTo(sstring("No rooms in list")
        + (filter.empty() ? "" : " match the filter")
        + ". Add some with `map add myCoolRoom`\n");
  }
}

void TPerson::doMapAdd(sstring const& arg)
{
  if (arg.empty()) {
    sendTo("Usage: map add myCoolRoom\n");
    return;
  }

  d->favoriteRooms[arg] = this->inRoom();
  TTransaction db(DB_SNEEZY);
  db.query("delete from savedrooms where player_id = %i and name = '%s'",
      getPlayerID(), arg.c_str());
  db.query("insert into savedrooms (player_id, name, room) values (%i, '%s', %i)",
      getPlayerID(), arg.c_str(), this->inRoom());
  sendTo(format("Saved %s -> %i") % arg % this->inRoom());
}

void TPerson::doMapRm(sstring const& arg)
{
  if (arg.empty()) {
    sendTo("Usage: map rm myCoolRoom");
    return;
  }

  if (d->favoriteRooms.erase(arg) == 0)
    sendTo("No such room in map.\n");
  else
    sendTo("Found and removed.\n");

  TDatabase db(DB_SNEEZY);
  db.query("delete from savedrooms where player_id = %i and name = '%s'",
      getPlayerID(), arg.c_str());
}

void TPerson::doMapGo(sstring const& arg)
{
  d->favoriteRooms["back"] = this->inRoom();

  auto it = d->favoriteRooms.find(arg);
  if (it == d->favoriteRooms.end()) {
    sendTo(format("You can't seem to locate '%s'.\n\r") % arg);
    return;
  }
  auto dst = it->second;

  auto path = pathfind(*this, findRoom(dst), "Uhm, not for nothing, but I think you are already there...\n\r");
  if (!path)
    return;

  auto run = runify(*path);
  addCommandToQue(run);
}

void TPerson::doMap(sstring const& arg)
{
  sstring cmd = arg.word(0);
  sstring rest = arg.dropWord();

  if (cmd == "list")
    doMapList(rest);
  else if (cmd == "add")
    doMapAdd(rest);
  else if (cmd == "rm")
    doMapRm(rest);
  else if (cmd == "go")
    doMapGo(rest);
  else
    sendTo("Syntax: map list|add|rm|go\n");
}
