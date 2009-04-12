#include "obj_player_corpse.h"
#include "obj_base_corpse.h"
#include "being.h"

float TPCorpse::getExpLost() const
{
  return exp_lost;
}
void TPCorpse::setExpLost(float exp)
{
  exp_lost=exp;
}

int TPCorpse::checkOnLists()
{
  return on_lists;
}

void TPCorpse::togOnCorpseListsOn()
{
  on_lists = TRUE;
}

void TPCorpse::togOnCorpseListsOff()
{
  on_lists = 0;
}

void TPCorpse::setRoomNum(int n)
{
  corpse_in_room = n;
}

int TPCorpse::getRoomNum() const
{
  return corpse_in_room;
}

void TPCorpse::setNumInRoom(int n)
{
  num_corpses_in_room = n;
}

int TPCorpse::getNumInRoom() const
{
  return num_corpses_in_room;
}

void TPCorpse::addToNumInRoom(int n)
{
  num_corpses_in_room += n;
}

void TPCorpse::setOwner(const sstring &Name)
{
  fileName = Name;
}

sstring TPCorpse::getOwner() const
{
  sstring buf=name;

  if(fileName.empty() &&
     buf.word(0) == "corpse" &&
     buf.word(2) == "pcorpse"){
    return buf.word(1).lower();
  }

  return fileName;
}

void TPCorpse::clearOwner()
{
  fileName.erase();
}

void TPCorpse::setNext(TPCorpse *n)
{
  nextCorpse = n;
}

void TPCorpse::removeNext()
{
  nextCorpse = NULL;
}

TPCorpse * TPCorpse::getNext() const
{
  return nextCorpse;
}

void TPCorpse::setPrevious(TPCorpse *n)
{
  previousCorpse = n;
}

void TPCorpse::removePrevious()
{
  previousCorpse = NULL;
}

TPCorpse * TPCorpse::getPrevious() const
{
  return previousCorpse;
}

void TPCorpse::setNextGlobal(TPCorpse *n)
{
  nextGlobalCorpse = n;
}

void TPCorpse::removeGlobalNext()
{
  nextGlobalCorpse = NULL;
}

TPCorpse * TPCorpse::getGlobalNext() const
{
  return nextGlobalCorpse;
}

TPCorpse::TPCorpse() :
  TBaseCorpse(),
  on_lists(0),
  corpse_in_room(0),
  num_corpses_in_room(0),
  exp_lost(0),
  fileName(""),
  nextGlobalCorpse(NULL),
  nextCorpse(NULL),
  previousCorpse(NULL)
{
}

TPCorpse::TPCorpse(const TPCorpse &a) :
  TBaseCorpse(a),
  on_lists(a.on_lists),
  corpse_in_room(a.corpse_in_room),
  num_corpses_in_room(a.num_corpses_in_room),
  exp_lost(a.exp_lost),
  fileName(a.fileName),
  nextGlobalCorpse(a.nextGlobalCorpse),
  nextCorpse(a.nextCorpse),
  previousCorpse(a.previousCorpse)
{
}

TPCorpse & TPCorpse::operator=(const TPCorpse &a)
{
  if (this == &a) return *this;
  TBaseCorpse::operator=(a);
  on_lists = a.on_lists;
  corpse_in_room = a.corpse_in_room;
  exp_lost = a.exp_lost;
  num_corpses_in_room = a.num_corpses_in_room;
  fileName = a.fileName;
  nextGlobalCorpse = a.nextGlobalCorpse;
  nextCorpse = a.nextCorpse;
  previousCorpse = a.previousCorpse;
  return *this;
}

TPCorpse::~TPCorpse()
{
   if (getGlobalNext() || getRoomNum() || getPrevious() || getNext())
     removeCorpseFromList();
}

void TPCorpse::assignFourValues(int x1, int x2, int x3, int x4)
{
  setCorpseFlags(x1);
  setCorpseRace((race_t) GET_BITS(x2, 7, 8));
  setCorpseLevel(GET_BITS(x2, 15, 8));
  setExpLost(x3);
  setRoomNum(x4);
}

void TPCorpse::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  int x = 0;

  *x1 = getCorpseFlags();

  SET_BITS(x, 7, 8, getCorpseRace());
  SET_BITS(x, 15, 8, getCorpseLevel());
  *x2 = x;

  *x3 = (int) getExpLost();

  *x4 = getRoomNum();
}

sstring TPCorpse::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Corpse Flags: %d, Corpse race: %d\n\r",
       getCorpseFlags(), getCorpseRace());
  sprintf(buf, "Corpse Owner: %s\n\r",
	  getOwner().c_str());
  sprintf(buf + strlen(buf), "Corpse Level: %d, Corpse Vnum: %d\n\r",
       getCorpseLevel(), getCorpseVnum());
  sprintf(buf + strlen(buf), "Corpse Experience: %f\n\r",
	  getExpLost());
  sprintf(buf + strlen(buf), "Lootable: %s\n\r",
	  (isCorpseFlag(CORPSE_DENY_LOOT) ? "no" : "yes"));

  sstring a(buf);
  return a;
}

int TPCorpse::getMe(TBeing *ch, TThing *sub)
{
  // do baseclass stuff for recusivity
  int rc = TObj::getMe(ch, sub);
  if (rc)
    return rc;

  if (ch) {
    removeCorpseFromList();
    return TRUE;
  }
  if (sub) {
    vlogf(LOG_BUG,format("Problem with a sub in corpse.getMe %s %s") %  (ch ? ch->getName() : "") % getName());
    removeCorpseFromList();
    return DELETE_THIS;
  }
  return TRUE;
}

int TPCorpse::scavengeMe(TBeing *ch, TObj **)
{
  return FALSE;
}



