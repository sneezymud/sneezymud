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
    case CMD_REMEMBER:
      doRemember(true, argument);
      break;
    case CMD_REMEMBERPLAYER:
      doRememberPlayer(true, argument);
      break;
    case CMD_RETRIEVE:
      doRetrieve(true, argument);
      break;
    case CMD_DISTRIBUTE:
      doDistribute(argument);
      break;
    default:
      return std::make_pair(false, 0);
  }
  return std::make_pair(true, 0);
}

namespace {
  void doRememberCommon(TPerson& me, bool shouldPrint, sstring const& arg, sstring const& cmd, sstring const& table, sstring const& foreignKey, int foreignValue)
  {
    auto sendTo = [&me, shouldPrint](const sstring& s) {
      if (shouldPrint)
        me.sendTo(s);
    };

    sstring key = arg.word(0);
    sstring value = arg.dropWord();

    if (key.empty()) {
      sendTo(format("Usage: %s key value\n") % cmd);
      sendTo(format("Example: %s tanelorn from spruce e se d 2w sw sw\n") % cmd);
      sendTo("Also check help remember\n");
      return;
    }

    TDatabase db(DB_SNEEZY);
    bool success = false;
    success = db.query(("delete from " + table + " where " + foreignKey + " = %i and name = '%s'").c_str(),
        foreignValue, key.c_str());

    if (success && !value.empty()) {
      success = db.query(("insert into " + table + " (" + foreignKey + ", name, value) values (%i, '%s', '%s')").c_str(),
          foreignValue, key.c_str(), value.c_str());
    }

    if (success)
      sendTo("Saved!\n");
    else
      sendTo("DB error, report a bug :(\n");
  }
}

void TPerson::doRemember(bool print, sstring const& arg)
{
  doRememberCommon(*this, print, arg, "remember", "accountnotes", "account_id", getAccountID());
}

void TPerson::doRememberPlayer(bool print, sstring const& arg)
{
  doRememberCommon(*this, print, arg, "rememberplayer", "playernotes", "player_id", getPlayerID());
}

void TPerson::doRetrieve(bool shouldPrint, sstring const& arg)
{
  auto sendTo = [this, shouldPrint](const sstring& s) {
    if (shouldPrint)
      this->sendTo(s);
  };

  sstring key = arg.word(0);

  TDatabase db(DB_SNEEZY);
  bool success = db.query("select name, value from playernotes where player_id = %i and ('%s' = '' or name = '%s') "
      "union "
      "select name, value from accountnotes where account_id = %i and ('%s' = '' or name = '%s')",
      getPlayerID(), key.c_str(), key.c_str(), getAccountID(), key.c_str(), key.c_str());

  if (!success) {
    sendTo("DB error, report a bug :(\n");
    return;
  }

  if (!db.fetchRow()) {
    sendTo(format("%s: not found") % arg);
    return;
  }

  assert(desc);
  do {
    auto str = sstring("retrieve." + key + " " + db["value"]);
    desc->sendGmcp(str, false);

    sendTo(format("%s: %s\n") % db["name"] % db["value"]);
  } while (arg.empty() && db.fetchRow()); // only print other matches if listing everything
}

void TPerson::loadMapData()
{
  TDatabase db(DB_SNEEZY);
  db.query("select name, room from savedroomsacct where account_id = %i", getAccountID());
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
  db.query("delete from savedroomsacct where account_id = %i and name = '%s'",
      getAccountID(), arg.c_str());
  db.query("insert into savedroomsacct (account_id, name, room) values (%i, '%s', %i)",
      getAccountID(), arg.c_str(), this->inRoom());
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
  db.query("delete from savedroomsacct where account_id = %i and name = '%s'",
      getAccountID(), arg.c_str());
}

void TPerson::doMapGo(sstring const& arg)
{
  auto it = d->favoriteRooms.find(arg);
  if (it == d->favoriteRooms.end()) {
    sendTo(format("You can't seem to locate '%s'.\n\r") % arg);
    return;
  }
  auto dst = it->second;

  auto path = pathfind(*this, findRoom(dst), "Uhm, not for nothing, but I think you are already there...\n\r");
  if (!path)
    return;

  d->favoriteRooms["back"] = this->inRoom();

  auto run = runify(*path);
  addCommandToQue(run);
}

void TPerson::doMap(sstring const& arg)
{
  sstring cmd = arg.word(0);
  sstring rest = arg.dropWord();

  if (is_abbrev(cmd, "list") || cmd == "ls")
    doMapList(rest);
  else if (is_abbrev(cmd, "add") || is_abbrev(cmd, "new"))
    doMapAdd(rest);
  else if (is_abbrev(cmd, "remove") || cmd == "rm")
    doMapRm(rest);
  else if (is_abbrev(cmd, "go"))
    doMapGo(rest);
  else
    sendTo("Syntax: map list/ls | add/new | rm/remove | go\n");
}
