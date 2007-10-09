//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//   monster.cc : functions of the TMonster class
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "statistics.h"
#include "shop.h"
#include "database.h"

charList::charList() :
  name(NULL),
  iHateStrength(0),
  account_id(0),
  player_id(0),
  next(NULL)
{
}

charList::charList(const charList &a)
{
  name = mud_str_dup(a.name);
  iHateStrength = a.iHateStrength;
  account_id = a.account_id;
  player_id = a.player_id;
  
  if (a.next)
    next = new charList(*a.next);
  else
    next = NULL;
}

charList & charList::operator=(const charList &a)
{
  if (this == &a) return *this;

  name = mud_str_dup(a.name);
  iHateStrength = a.iHateStrength;
  account_id = a.account_id;
  player_id = a.player_id;
  
  charList *c, *n;
  for (c = next; c; c = n) {
    n = c->next;
    delete c;
  }
  if (a.next)
    next = new charList(*a.next);
  else
    next = NULL;
  return *this;
}

charList::~charList()
{
  delete [] name;
  name = NULL;
}

opinionData::opinionData() :
  clist(NULL),
  sex(SEX_NEUTER),
  race(RACE_NORACE),
  Class(0),
  vnum(0)
{
}

opinionData::opinionData(const opinionData &a) :
  sex(a.sex),
  race(a.race),
  Class(a.Class),
  vnum(a.vnum)
{
  if (a.clist)
    clist = new charList(*a.clist);
  else
    clist = NULL;
}

opinionData & opinionData::operator=(const opinionData &a)
{
  if (this == &a) return *this;
  sex = a.sex;
  race = a.race;
  Class = a.Class;
  vnum = a.vnum;

  if (a.clist)
    clist = new charList(*a.clist);
  else
    clist = NULL;
  return *this;
}

// warning: you must remember to manually delete opinionData::next in a loop
opinionData::~opinionData()
{
  delete clist;
  clist = NULL;
}

Mobile_Attitude::Mobile_Attitude() :
  suspicion(0),
  greed(0),
  malice(0),
  anger(0),
  def_suspicion(0),
  def_greed(0),
  def_malice(0),
  def_anger(0),
  target(NULL),
  random(NULL),
  last_cmd(-1)
{
}

Mobile_Attitude::Mobile_Attitude(const Mobile_Attitude &a)
  : suspicion(a.suspicion),
    greed(a.greed),
    malice(a.malice),
    anger(a.anger),
    def_suspicion(a.def_suspicion),
    def_greed(a.def_greed),
    def_malice(a.def_malice),
    def_anger(a.def_anger),
    target(a.target),
    random(a.random),
    last_cmd(a.last_cmd)
{
}

Mobile_Attitude & Mobile_Attitude::operator=(const Mobile_Attitude &a)
{
  if (this == &a) return *this;
  suspicion = a.suspicion;
  greed = a.greed;
  malice = a.malice;
  anger = a.anger;
  def_suspicion = a.def_suspicion;
  def_greed = a.def_greed;
  def_malice = a.def_malice;
  def_anger = a.def_anger;
  target = a.target;
  random = a.random;
  last_cmd = a.last_cmd;
  return *this;
}

Mobile_Attitude::~Mobile_Attitude()
{
}

TMonster::TMonster() :
  TBeing(),
  resps(NULL),
  opinion(),
  hates(),
  fears(),
  persist(0), 
  oldRoom(ROOM_NOWHERE),
  brtRoom(ROOM_NOWHERE),
  hatefield(0),
  fearfield(0),
  moneyConst(0),
  sounds(NULL),
  distantSnds(NULL),
  hpLevel(0.0),
  damLevel(0.0),
  damPrecision(0),
  acLevel(0.0),
  default_pos(POSITION_STANDING)
{
}

TMonster::TMonster(const TMonster &a) :
  TBeing(a),
  opinion(a.opinion),
  hates(a.hates),
  fears(a.fears),
  persist(a.persist), 
  oldRoom(a.oldRoom),
  brtRoom(a.brtRoom),
  hatefield(a.hatefield),
  fearfield(a.fearfield),
  moneyConst(a.moneyConst),
  hpLevel(a.hpLevel),
  damLevel(a.damLevel),
  damPrecision(a.damPrecision),
  acLevel(a.acLevel),
  default_pos(a.default_pos)
{
  if (a.resps)
    resps = new Responses(*a.resps);
  else
    resps = NULL;

  sounds = mud_str_dup(a.sounds);
  distantSnds = mud_str_dup(a.distantSnds);
}

TMonster & TMonster::operator=(const TMonster &a)
{
  if (this == &a) return *this;
  TBeing::operator=(a);

  if (a.resps)
    resps = new Responses(*a.resps);
  else
    resps = NULL;

  opinion = a.opinion;
  hates = a.hates;
  fears = a.fears;
  persist = a.persist;
  oldRoom = a.oldRoom;
  brtRoom = a.brtRoom;
  hatefield = a.hatefield;
  fearfield = a.fearfield;
  moneyConst = a.moneyConst;
  hpLevel = a.hpLevel;
  damLevel = a.damLevel;
  damPrecision = a.damPrecision;
  acLevel = a.acLevel;
  default_pos = a.default_pos;

  delete [] sounds;
  sounds = mud_str_dup(a.sounds);
  delete [] distantSnds;
  distantSnds = mud_str_dup(a.distantSnds);

  return *this;
}

TMonster::~TMonster()
{
  charList *k2 = NULL, *n2 = NULL;

  if (number >= 0) {
    if (GetMaxLevel() <= 5)
      stats.act_1_5--;
    else if (GetMaxLevel() <= 10)
      stats.act_6_10--;
    else if (GetMaxLevel() <= 15)
      stats.act_11_15--;
    else if (GetMaxLevel() <= 20)
      stats.act_16_20--;
    else if (GetMaxLevel() <= 25)
      stats.act_21_25--;
    else if (GetMaxLevel() <= 30)
      stats.act_26_30--;
    else if (GetMaxLevel() <= 40)
      stats.act_31_40--;
    else if (GetMaxLevel() <= 50)
      stats.act_41_50--;
    else if (GetMaxLevel() <= 60)
      stats.act_51_60--;
    else if (GetMaxLevel() <= 70)
      stats.act_61_70--;
    else if (GetMaxLevel() <= 100)
      stats.act_71_100--;
    else
      stats.act_101_127--;
  }

  if (spec)
    checkSpec(this, CMD_GENERIC_DESTROYED, "", NULL);

  // responses
  if (resps) {
    delete resps;
    resps = NULL;
  }
  // hates and fears.
  for (k2 = hates.clist; k2; k2 = n2) {
    n2 = k2->next;
    delete k2;
  }
  hates.clist = NULL;

  for (k2 = fears.clist; k2; k2 = n2) {
    n2 = k2->next;
    delete k2;
  }
  fears.clist = NULL;

  if (number > -1)
    mob_index[getMobIndex()].addToNumber(-1);

  if (sounds) {
    delete [] sounds;
    sounds = NULL;
  }
  if (distantSnds) {
    delete [] distantSnds;
    distantSnds = NULL;
  }

  // if we are using shared sstrings, reallocate them so ~TThing can purge
  // safely
  int didAloc = FALSE;
  if (!IS_SET(specials.act,ACT_STRINGS_CHANGED)) {
    didAloc = TRUE;
    name = mud_str_dup(name);
    char *tc = mud_str_dup(getDescr());
    setDescr(tc);
    shortDescr = mud_str_dup(shortDescr);
    player.longDescr = mud_str_dup(getLongDesc());

    if (ex_description)
      ex_description = new extraDescription(*ex_description);
    else
      ex_description = NULL;
  }
  mobCount--;
// Looking for bugs below--cos 8/98
  if (getDescr() && getDescr() == mob_index[getMobIndex()].description) { 
    vlogf(LOG_BUG, fmt("TMonster delete: after allocation, monster still had shared sstring (%s) : descr") %  getName());
    vlogf(LOG_BUG, fmt("New Alloc: %s: shared descr is: %s") %  (didAloc ? "True" : "False") % getDescr());
  }
  if (name && name == mob_index[getMobIndex()].name) 
    vlogf(LOG_BUG, fmt("TMonster delete: after allocation, monster still had shared sstring (%s) : name") %  getName());

  if (shortDescr && shortDescr == mob_index[getMobIndex()].short_desc) 
    vlogf(LOG_BUG, fmt("TMonster delete: after allocation, monster still had shared sstring (%s) : short") %  getName());

  if (getLongDesc() && player.longDescr == mob_index[getMobIndex()].long_desc)
    vlogf(LOG_BUG, fmt("TMonster delete: after allocation, monster still had shared sstring (%s) : long") %  getName());

  TRoom *tRoom;

  if (brtRoom == ROOM_NOWHERE)
    ; // Do nothing.  This triggers on immortal loaded mobs so is cool.
  else if (!(tRoom = real_roomp(brtRoom)))
    vlogf(LOG_BUG, fmt("Mobile being destroyed with empty birth room! [%s][%i]") %  getName() % brtRoom);
  else
    *tRoom >> *this;
}

unsigned int TMonster::getTimer() const
{
  return 0;
}

void TMonster::setTimer(unsigned int)
{
}

void TMonster::addToTimer(unsigned int)
{
}

void TMonster::swapToStrung()
{
  if (specials.act & ACT_STRINGS_CHANGED)
    return;

  // Set flags saying editted and point all sstrings to new stuff - Russ 
  specials.act |= ACT_STRINGS_CHANGED;
  name = mud_str_dup(mob_index[getMobIndex()].name);
  shortDescr = mud_str_dup(mob_index[getMobIndex()].short_desc);
  player.longDescr = mud_str_dup(mob_index[getMobIndex()].long_desc);
  setDescr(mud_str_dup(mob_index[getMobIndex()].description));
}

int TMonster::calculateGoldFromConstant()
{
  double rlev = getRealLevel();

  // our balance assumptions is that "typical" mob has 1.5 * L^2 gold
  // presume that moneyConst=2 of typical
  double the_gold = rlev * max(20.0,rlev) * moneyConst * 7.5 / 10;

  // adjust for global gold modifier...
  int CENTRAL_BANK=123;
  the_gold *= shop_index[CENTRAL_BANK].getProfitSell(NULL, NULL);
  //  the_gold *= gold_modifier[GOLD_INCOME].getVal();

  if (isShopkeeper()){
    unsigned int shop_nr;

    for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != this->number); shop_nr++);
    
    if (shop_nr >= shop_index.size()) {
      vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") %  mob_index[this->number].virt);
      return FALSE;
    }

    TDatabase db(DB_SNEEZY);
    db.query("select gold from shopowned where shop_nr=%i", shop_nr);
    
    if(db.fetchRow()){
      the_gold = convertTo<int>(db["gold"]);
    } else {
      the_gold = 1000000;
    }
  }

  setMoney((int) the_gold);
  return FALSE;
}

bool TMonster::isTestmob() const
{
  unsigned int vn = mobVnum();
  if (vn < 1700 || vn > 1750)
    return false;
  return true;
}

void TMonster::setACFromACLevel()
{
  // this is based on the balance doctrine
  // AC should be 600 - 20*level
  //  sh_int num = (sh_int) (20 * getACLevel());
  //  setArmor(600 - num);
}

void TMonster::setHPFromHPLevel()
{
  // this is based on the balance doctrine
  int amt;

  // testmobs and MED mobs should really not fluxuate if at all possible
  if (!isTestmob() && number >= 0)
    amt = dice((int) getHPLevel(), 8);
  else
    amt = (int) (4.5 * getHPLevel());

  // boost for everyone
  amt += (int) (11 * getHPLevel());
  // extra boost for mobs above level 70
  if(getHPLevel()>70)
    amt += (int) ( 11 * getHPLevel()  * 
      (getHPLevel()-70) * (getHPLevel() - 70) / 150 );

  // balance stuff:
  // HP for mobs should roughly balance with damage for PCs
  // PC damage (melee) goes up linearly, but between L25 and L35 number
  // of hits doubles (specialize)
  // we need to account for this
  amt = (int) (amt * balanceCorrectionForLevel(getHPLevel()));

  double sanct_modifier = (100.0 / (100.0 - min(99.0, (double) getProtection())));
  amt = (int) (amt / sanct_modifier);

  if (amt >= 32768) {
    vlogf(LOG_DASH, fmt("mob %s had hp overflow problem with %d hp, capping at 32767.") %  getName() % amt);
    amt = 32767;
  }

  setMaxHit(amt);
  setHit(hitLimit());
}

float TMonster::getACLevel() const
{
  return acLevel;
}

void TMonster::setACLevel(float n)
{
  acLevel = n;
}

float TMonster::getHPLevel() const
{
  return hpLevel;
}

void TMonster::setHPLevel(float n)
{
  hpLevel = n;
}

float TMonster::getDamLevel() const
{
  return damLevel;
}

void TMonster::setDamLevel(float n)
{
  damLevel = n;
}

ubyte TMonster::getDamPrecision() const
{
  return damPrecision;
}

void TMonster::setDamPrecision(ubyte n)
{
  damPrecision = n;
}

int TMonster::getMobDamage() const
{
  // this function is based on the balance doctrine
  // we want mob damage to be 0.909 * lev per round
  double dam_rnd = getDamLevel() / 1.1;

  // account for number of hits
  dam_rnd /= getMult();

  int idamrnd = max(1, (int) dam_rnd);

  int randomizer_max = (int) (idamrnd * getDamPrecision() / 100);
  int randomizer_amt = ::number(-randomizer_max, randomizer_max);

  idamrnd += randomizer_amt;
  return max(1, idamrnd);
}

int TMonster::lookForEngaged(const TBeing *ch)
{
  // here's some AI
  // tank and healer fight mob, tank flees, mob should now beat on
  // healer (regardless of whether he engaged or not?)
  if (isPc())
    return FALSE;
  // if someone is attacking me directly, bypass
  if (attackers > 0)
    return FALSE;
  if (!isSmartMob(0))
    return FALSE;

  TThing * t, *t2;
  for (t = roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    if (t == this || (ch && dynamic_cast<const TBeing *>(t) == ch))
      continue;
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (!tbt)
      continue;
    if (tbt->fight() == this) {
      // I'm a smart mob, and, oh look, tbt is engaged with me...
      int rc = takeFirstHit(*tbt);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        delete tbt;
        tbt = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      }
      return TRUE;
    }
  }
  return FALSE;
}



bool TMonster::isShopkeeper() const {
  if(spec==SPEC_SHOPKEEPER || spec==SPEC_REPAIRMAN ||
     spec==SPEC_LOAN_SHARK || spec==SPEC_BANKER ||
     spec==SPEC_DOCTOR     || spec==SPEC_DIVMAN ||
     spec==SPEC_ATTUNER    || spec==SPEC_ENGRAVER ||
     spec==SPEC_SHARPENER  || spec==SPEC_POSTMASTER ||
     spec==SPEC_PET_KEEPER || spec==SPEC_TAXMAN ||
     spec==SPEC_SIGNMAKER  || spec==SPEC_RECEPTIONIST ||
     spec==SPEC_AUCTIONEER || spec==SPEC_CENTRAL_BANKER ||
     spec==SPEC_COMMOD_MAKER)
    return true;

  return false;
}
