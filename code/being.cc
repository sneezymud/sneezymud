//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "being.h"
#include "database.h"
#include "obj_opal.h"

extern bool affectShouldApply(const TObj *obj, wearSlotT pos);



playerData::playerData() :
  longDescr(NULL),
  sex(SEX_NEUTER),
  max_level(0),
  Class(0),
  hometown(ROOM_CS),
  hometerrain(HOME_TER_NONE)
{
  classIndT i;
  for (i = MIN_CLASS_IND; i < MAX_SAVED_CLASSES; i++) {
    level[i] = 0;
    doneBasic[i] = 0;
  }
  time.birth = ::time(0);
  time.logon = ::time(0);
  time.played = 0;
  time.last_logon = ::time(0);
}

playerData::playerData(const playerData &a) :
  longDescr(NULL),
  sex(a.sex),
  max_level(a.max_level),
  Class(a.Class), hometown(a.hometown), hometerrain(a.hometerrain)
{
  // just assign by default, this gets overridden in TBeing stuff
  longDescr = a.longDescr;

  classIndT i;
  for (i = MIN_CLASS_IND; i < MAX_SAVED_CLASSES; i++) {
    level[i] = a.level[i];
    doneBasic[i] = a.doneBasic[i];
  }
  time.birth = a.time.birth;
  time.logon = a.time.logon;
  time.played = a.time.played;
  time.last_logon = a.time.last_logon;
}

playerData & playerData::operator=(const playerData &a)
{
  if (this == &a) return *this;

  // be careful about calling this is longDescr needs deletion before hand.
  // just assign by default, this gets overridden in TBeing stuff
  longDescr = a.longDescr;

  sex = a.sex;
  hometerrain = a.hometerrain;
  max_level = a.max_level;

  classIndT i;
  for (i = MIN_CLASS_IND; i < MAX_SAVED_CLASSES; i++) {
    level[i] = a.level[i];
    doneBasic[i] = a.doneBasic[i];
  }
  Class = a.Class;
  hometown = a.hometown;

  time.birth = a.time.birth;
  time.logon = a.time.logon;
  time.played = a.time.played;
  time.last_logon = a.time.last_logon;
  return *this;
}

playerData::~playerData()
{
  delete [] longDescr;
  longDescr = NULL;
}

pointData::pointData() :
  mana(0),
  maxMana(0),
  piety(0.0),
  lifeforce(0),
  hit(0),
  maxHit(0),
  move(0),
  maxMove(0),
  money(0),
  bankmoney(0),
  exp(0), 
  max_exp(0), 
  spellHitroll(0),
  hitroll(0),
  damroll(0),
  armor(1000)
{
}

pointData::pointData(const pointData &a) :
  mana(a.mana),
  maxMana(a.maxMana),
  piety(a.piety),
  lifeforce(a.lifeforce),
  hit(a.hit),
  maxHit(a.maxHit),
  move(a.move),
  maxMove(a.maxMove),
  money(a.money),
  bankmoney(a.bankmoney),
  exp(a.exp), 
  max_exp(a.max_exp), 
  spellHitroll(a.spellHitroll), 
  hitroll(a.hitroll),
  damroll(a.damroll),
  armor(a.armor)
{
}

pointData & pointData::operator=(const pointData &a)
{
  if (this == &a) return *this;
  armor = a.armor;
  money = a.money;
  bankmoney = a.bankmoney;
  exp = a.exp;
  max_exp = a.max_exp;
  mana = a.mana;
  maxMana = a.maxMana;
  hit = a.hit;
  maxHit = a.maxHit;
  move = a.move;
  maxMove = a.maxMove; 
  spellHitroll = a.spellHitroll;
  hitroll = a.hitroll;
  damroll = a.damroll;
  piety = a.piety;
  lifeforce = a.lifeforce;
  return *this;
}

pointData::~pointData()
{
}

immunityData::immunityData()
{
  for( int i = 0;i < MAX_IMMUNES; i++)
    immune_arr[i] = 0;
}

immunityData::immunityData(const immunityData &a)
{
  for( int i = 0;i < MAX_IMMUNES; i++)
    immune_arr[i] = a.immune_arr[i];
}

immunityData & immunityData::operator=(const immunityData &a)
{
  if (this == &a) return *this;
  for( int i = 0;i < MAX_IMMUNES; i++)
    immune_arr[i] = a.immune_arr[i];
  return *this;
}

immunityData::~immunityData()
{
}

factionData::factionData() :
  percent(0.0),
  captive(NULL),
  next_captive(NULL),
  captiveOf(NULL),
  target(NULL),
  type(FACT_UNDEFINED),
  actions(0),
  align_ge(0),
  align_lc(0),
  whichfaction(0)
{
#if FACTIONS_IN_USE
  for (int i = 0; i < MAX_FACTIONS; i++)
    percX[i] = 0.00;
#endif
}

factionData::factionData(const factionData &a) :
  percent(a.percent),
  captive(a.captive),
  next_captive(a.next_captive),
  captiveOf(a.captiveOf),
  target(a.target),
  type(a.type), 
  actions(a.actions),
  align_ge(a.align_ge),
  align_lc(a.align_lc),
  whichfaction(a.whichfaction)
{
#if FACTIONS_IN_USE
  for (int i = 0; i < MAX_FACTIONS; i++)
    percX[i] = a.percX[i];
#endif
}

factionData & factionData::operator=(const factionData &a)
{
  if (this == &a) return *this;
  type = a.type; 
  actions = a.actions;
  captive = a.captive; 
  next_captive = a.next_captive;
  captiveOf = a.captiveOf;
  target = a.target;
  percent = a.percent;
#if FACTIONS_IN_USE
  for (int i = 0; i < MAX_FACTIONS; i++)
    percX[i] = a.percX[i];
#endif
  return *this;
}

factionData::~factionData()
{
}

pracData::pracData()
{
  for(int i=0;i<MAX_SAVED_CLASSES;++i)
    prac[i]=0;
}

pracData::pracData(const pracData &a)
{
  for(int i=0;i<MAX_SAVED_CLASSES;++i)
    prac[i]=a.prac[i];
}

pracData & pracData::operator=(const pracData &a)
{
  if (this == &a) return *this;

  for(int i=0;i<MAX_SAVED_CLASSES;++i)
    prac[i]=a.prac[i];

  return *this;
}

pracData::~pracData()
{
}

bodyPartsDamage::bodyPartsDamage() 
  : flags(0), stuckIn(NULL), health(0)
{
}

bodyPartsDamage::bodyPartsDamage(const bodyPartsDamage &a) 
  : flags(a.flags), stuckIn(a.stuckIn), health(a.health)
{
}

bodyPartsDamage & bodyPartsDamage::operator=(const bodyPartsDamage &a)
{
  if (this == &a) return *this;
  flags = a.flags;
  stuckIn = a.stuckIn;
  health = a.health;
  return *this;
}

bodyPartsDamage::~bodyPartsDamage()
{
}

skillApplyData::skillApplyData()
  : skillNum(0), amount(0), numApplys(0), nextApply(NULL)
{
}

skillApplyData::skillApplyData(const skillApplyData &a)
  : skillNum(a.skillNum), amount(a.amount), 
    numApplys(a.numApplys), nextApply(a.nextApply)
{
  if (a.nextApply)
    nextApply = new skillApplyData(*a.nextApply);
  else
    nextApply = NULL;
}

skillApplyData & skillApplyData::operator=(const skillApplyData &a)
{
  if (this == &a) return *this;
  skillNum = a.skillNum;
  amount = a.amount;
  numApplys = a.numApplys;
  nextApply = a.nextApply;
  return *this;
}

skillApplyData::~skillApplyData()
{
}

followData::followData() 
  : follower(NULL), next(NULL)
{
}

followData::followData(const followData &a) 
  : follower(a.follower)
{
  if (a.next)
    next = new followData(*a.next);
  else
    next = NULL;
}

followData & followData::operator=(const followData &a)
{
  if (this == &a) return *this;
  follower = a.follower;

  followData *a2, *a3;
  for (a2 = next; a2; a2 = a3) {
    a3 = a2->next;
    delete a2;
  }

  if (a.next)
    next = new followData(*a.next);
  else
    next = NULL;
  return *this;
}

followData::~followData()
{
}

spellTaskData::spellTaskData() :
  spell(TYPE_UNDEFINED),
  target(0),
  start_pos(0),
  distracted(0),
  difficulty(TASK_NORMAL),
  component(0),
  nextUpdate(0),
  rounds(0),
  orig_arg(NULL),
  wasInRoom(0),
  status(0),
  text(0),
  flags(0),
  victim(NULL),
  object(NULL),
  room(NULL)
{
}

spellTaskData::spellTaskData(const spellTaskData &a) :
  spell(a.spell),
  target(a.target),
  start_pos(a.start_pos),
  distracted(a.distracted),
  difficulty(a.difficulty), 
  component(a.component),
  nextUpdate(a.nextUpdate),
  rounds(a.rounds),
  wasInRoom(a.wasInRoom),
  status(a.status),
  text(a.text),
  flags(a.flags),
  victim(a.victim),
  object(a.object),
  room(a.room)
{
  orig_arg = mud_str_dup(a.orig_arg);
}

spellTaskData & spellTaskData::operator=(const spellTaskData &a)
{
  if (this == &a) return *this;
  spell = a.spell;
  status = a.status;
  nextUpdate = a.nextUpdate;
  rounds = a.rounds;
  flags = a.flags;
  start_pos = a.start_pos;
  distracted = a.distracted; 
  difficulty = a.difficulty;
  component = a.component;
  wasInRoom = a.wasInRoom;
  text = a.text;
  target = a.target;
  victim = a.victim; 
  object = a.object;
  room = a.room;
  delete [] orig_arg;
  orig_arg = mud_str_dup(a.orig_arg);
  return *this;
}

spellTaskData::~spellTaskData()
{
  delete [] orig_arg;
  orig_arg = NULL;
}


spellStoreData::spellStoreData() :
  spelltask(NULL),
  storing(FALSE)
{
}

spellStoreData::spellStoreData(const spellStoreData &a) :
  spelltask(a.spelltask),
  storing(a.storing)
{
}

spellStoreData & spellStoreData::operator=(const spellStoreData &a)
{
  if (this == &a) return *this;
  spelltask=a.spelltask;
  storing=a.storing;
  return *this;
}

spellStoreData::~spellStoreData()
{
}


equipmentData::equipmentData(){
  wearSlotT i;
  for (i = MIN_WEAR; i < MAX_WEAR; i++)
    equipment[i] = NULL;
}

equipmentData::equipmentData(const equipmentData &a){
  wearSlotT i;
  for (i = MIN_WEAR; i < MAX_WEAR; i++)
    equipment[i] = a.equipment[i];
}

equipmentData & equipmentData::operator= (const equipmentData &a){
  wearSlotT i;
  for (i = MIN_WEAR; i < MAX_WEAR; i++)
    equipment[i] = a.equipment[i];
  
  return *this;
}

equipmentData::~equipmentData(){
}



charFile::charFile() :
  sex(SEX_MALE),
  race(0),
  screen(0),
  attack_type(ATTACK_NORMAL),
  weight((float) 0.0),
  height(0),
  Class(0),
  played(0),
  birth(0),
  last_logon(0),
  hometown(0),
  hometerrain(0),
  load_room(0),
  p_type(0),
  bad_login(0),
  base_age(0),
  age_mod(0),
  wimpy(0),
  autobits(0),
  best_rent_credit(0),
//  point data members
  mana(0),
  maxMana(0),
  piety(0.0),
  lifeforce(0),
  hit(0),
  maxHit(0),
  move(0),
  maxMove(0),
  money(0),
  bankmoney(0),
  exp(0),
  spellHitroll(0),
  hitroll(0),
  damroll(0),
  armor(1000),
// points(),
// end pointData members
  fatigue(0),
  hero_num(0),
  f_percent(0),
  f_type(0),
  f_actions(0),
  whichfaction(0),
  align_ge(0),
  align_lc(0),
  practices(),
  pColor(0),
  plr_act(0),
  flags(0),
  plr_color(0),
  plr_colorSub(COLOR_SUB_NONE),
  plr_colorOff(0),
  temp1(-999),
  temp2(-999),
  temp3(-999),
  temp4(-999)
{
  classIndT cit;
  for (cit=MIN_CLASS_IND; cit < MAX_SAVED_CLASSES; cit++) {
    level[cit] = 0;
    doneBasic[cit] = 0;
  }
  wearSlotT wst;
  for (wst=MIN_WEAR;wst<MAX_HUMAN_WEAR;wst++) {
    body_flags[wst]=0;
    body_health[wst]=0;
  }
  condTypeT ctt;
  for (ctt=MIN_COND;ctt<MAX_COND_TYPE;++ctt)
    conditions[ctt]=0;

  memset(title, '\0', sizeof(title));
  memset(name, '\0', sizeof(name));
  memset(aname, '\0', sizeof(aname));
  memset(pwd, '\0', sizeof(pwd));

  memset(description, '\0', sizeof(description));
  memset(lastHost, '\0', sizeof(lastHost));
  memset(hpColor, '\0', sizeof(hpColor));
  memset(manaColor, '\0', sizeof(manaColor));
  memset(moveColor, '\0', sizeof(moveColor));
  memset(moneyColor, '\0', sizeof(moneyColor));
  memset(expColor, '\0', sizeof(expColor));
  memset(roomColor, '\0', sizeof(roomColor));
  memset(oppColor, '\0', sizeof(oppColor));
  memset(tankColor, '\0', sizeof(tankColor));
  memset(pietyColor, '\0', sizeof(pietyColor));
  memset(lifeforceColor, '\0', sizeof(lifeforceColor));

  statTypeT ij;
  for (ij=MIN_STAT;ij<MAX_STATS;ij++) {
    stats[ij]=0;
  }
  int i;
  for (i=0;i<MAX_AFFECT;i++) {
    affected[i] = affectedData();
  }
  factionTypeT il;
  for (il=MIN_FACTION;il<ABS_MAX_FACTION;il++) {
    f_percx[il] = (double) 0.0;
  }

  // this should be an int and not a discNumT
  // we want to 0 out all values, even unused ones
  int dnt;
  for (dnt=MIN_DISC; dnt < MAX_SAVED_DISCS; dnt++)
    disc_learning[dnt] = 0;

  for (i=0;i<ABSOLUTE_MAX_SKILL;i++) {
    skills[i] = 0;
  }
  // alias should set up OK
}

charFile::~charFile()
{
}

TAccount::TAccount() :
  status(FALSE),
  birth(time(0)),
  login(0),
  term(TERM_NONE),
  desc(NULL),
  time_adjust(0),
  flags(0)
{
  *name = '\0';
  *passwd = '\0';
  *email = '\0';
}

TAccount::TAccount(const TAccount &a) :
  status(a.status),
  birth(a.birth),
  login(a.login),
  term(a.term),
  desc(a.desc),
  time_adjust(a.time_adjust),
  flags(a.flags)
{
  strcpy(name, a.name);
  strcpy(passwd, a.passwd);
  strcpy(email, a.email);
}

TAccount & TAccount::operator=(const TAccount &a)
{
  if (this == &a) return *this;
  strcpy(name, a.name);
  strcpy(passwd, a.passwd);
  strcpy(email, a.email);
  birth = a.birth;
  login = a.login;
  desc = a.desc;
  term = a.term;
  status = a.status;
  time_adjust = a.time_adjust;
  flags = a.flags;
  return *this;
}

TAccount::~TAccount()
{
}

const char * TBeing::getName() const
{
  return (shortDescr ? shortDescr : "");
}

// this returns the ID in the database, or creates a new one if needed
int TBeing::getPlayerID() const
{
  TDatabase db(DB_SNEEZY);
  sstring myname;
  int playerID=0;

  if(!isPc())
    return 0;

  if(desc && desc->playerID)
    return desc->playerID;

  if (specials.act & ACT_POLYSELF) {
    myname=desc->original->getName();
  } else { 
    myname=getName();
  }

  db.query("select id from player where lower(name)=lower('%s')", myname.c_str());

  if(db.fetchRow()){
    playerID=convertTo<int>(db["id"]);
  } else {
    vlogf(LOG_BUG, fmt("Couldn't find a player_id for '%s', creating a new one.") % myname);

    db.query("insert into player (name) values (lower('%s'))", myname.c_str());
    db.query("select id from player where lower(name)=('%s')", myname.c_str());
    
    if(db.fetchRow()){
      playerID=convertTo<int>(db["id"]);
    } else {
      vlogf(LOG_BUG,fmt("Couldn't create an entry in player table for '%s'!") %
	    myname);
      return 0;
    }
  }
  
  if(desc)
    desc->playerID=playerID;

  return playerID;
}

void TBeing::showMultTo(const TThing *t, showModeT i, unsigned int n)
{
  t->show_me_mult_to_char(this, i, n);
}

void TBeing::showTo(const TThing *t, showModeT i)
{
  t->show_me_to_char(this, i);
}

const char * TBeing::hshr() const
{
  return (getSex() != SEX_NEUTER ? ((getSex() == SEX_MALE) ? "his" : "her") : "its");
}

const char * TBeing::hssh() const
{
  return (getSex() != SEX_NEUTER ? ((getSex() == SEX_MALE) ? "he" : "she") : "it");
}

const char * TBeing::hmhr() const
{
  return (getSex() != SEX_NEUTER ? ((getSex() == SEX_MALE) ? "him" : "her") : "it");
}

bool TBeing::isPlayerAction(unsigned long num) const
{
  if (desc)
    return desc->plr_act & num;
  return false;
}
void TBeing::remPlayerAction(unsigned long num)
{
  if (desc)
    desc->plr_act &= ~num;
}

void TBeing::addPlayerAction(unsigned long num)
{
  if (desc)
    desc->plr_act |= num;
}

wearSlotT TBeing::getPrimaryArm() const
{
  return (isRightHanded() ? WEAR_ARM_R : WEAR_ARM_L);
}

wearSlotT TBeing::getSecondaryArm() const
{
  return (isRightHanded() ? WEAR_ARM_L : WEAR_ARM_R);
}

wearSlotT TBeing::getPrimaryHold() const
{
  return (isRightHanded() ? HOLD_RIGHT : HOLD_LEFT);
}

wearSlotT TBeing::getSecondaryHold() const
{
  return (isRightHanded() ? HOLD_LEFT : HOLD_RIGHT);
}

wearSlotT TBeing::getPrimaryHand() const
{
  return (isRightHanded() ? WEAR_HAND_R : WEAR_HAND_L);
}

wearSlotT TBeing::getSecondaryHand() const
{
  return (isRightHanded() ? WEAR_HAND_L : WEAR_HAND_R);
}

wearSlotT TBeing::getPrimaryFinger() const
{
  return (isRightHanded() ? WEAR_FINGER_R : WEAR_FINGER_L);
}

wearSlotT TBeing::getSecondaryFinger() const
{
  return (isRightHanded() ? WEAR_FINGER_L : WEAR_FINGER_R);
}

wearSlotT TBeing::getPrimaryWrist() const
{
  return (isRightHanded() ? WEAR_WRIST_R : WEAR_WRIST_L);
}

wearSlotT TBeing::getSecondaryWrist() const
{
  return (isRightHanded() ? WEAR_WRIST_L : WEAR_WRIST_R);
}

wearSlotT TBeing::getPrimaryFoot() const
{
  return (isRightHanded() ? WEAR_FOOT_R : WEAR_FOOT_L);
}

wearSlotT TBeing::getSecondaryFoot() const
{
  return (isRightHanded() ? WEAR_FOOT_L : WEAR_FOOT_R);
}

wearSlotT TBeing::getPrimaryLeg() const
{
  return (isRightHanded() ? WEAR_LEGS_R : WEAR_LEGS_L);
}

wearSlotT TBeing::getSecondaryLeg() const
{
  return (isRightHanded() ? WEAR_LEGS_L : WEAR_LEGS_R);
}

int TBeing::maxWimpy()
{
  int hl = hitLimit();
  return (hl / 2 + hl % 2);
}

int TBeing::getWimpy()
{
  return wimpy;
}

void TBeing::setWimpy(int w)
{
  wimpy=w;
}

short int TBeing::hitLimit() const
{
  return points.maxHit;
}

int TBeing::getHit() const
{
  return points.hit;
}

void TBeing::addToHit(int add)
{
  points.hit = min(points.hit + add, (int) hitLimit());
// this prevents folks from dying
//  points.hit = max((short int) 0, points.hit);
}

void TBeing::setHit(int newhit)
{
  points.hit = newhit;
}

void TBeing::setMaxHit(int newhit)
{
  points.maxHit = newhit;
}

double TBeing::getPercHit(void)
{
  return (hitLimit() ? (100.0 * (double) getHit() / (double) hitLimit()) : 0.0);
}

int TBeing::getMove() const
{
  return points.move;
}

void TBeing::addToMove(int add)
{
  points.move += add;
  points.move = max((short int) 0, points.move);
// I don't think we need this check, since it is probably checked already
// in any event, having it here is making this command take a big amt of
// cpu% time - bat 12/23/98
//  points.move = min(points.move, moveLimit());
}

void TBeing::setMove(int move)
{
  points.move = move;
}

int TBeing::getMaxMove() const
{
  return points.maxMove;
}

void TBeing::setMaxMove(int move)
{
  points.maxMove = move;
}

int TBeing::getMana() const
{
  return points.mana;
}

void TBeing::setMana(int mana)
{
  points.mana = mana;
}

void TBeing::addToMana(int mana)
{
  points.mana += mana;
  points.mana = max((short int) 0, points.mana);
  points.mana = min(points.mana, manaLimit());
}

double TBeing::getPercMana(void)
{
  return (manaLimit() ? (100.0 * (double) getMana() / (double) manaLimit()) : 0.0);
}

void TBeing::setMaxMana(int mana)
{
  points.maxMana = mana;
}

// LIFEFORCE 

int TBeing::getLifeforce() const
{
  return points.lifeforce;
}

void TBeing::setLifeforce(int lifeforce)
{
  points.lifeforce = lifeforce;
}

void TBeing::addToLifeforce(int lifeforce)
{
  points.lifeforce += lifeforce;
  points.lifeforce = max((short int) 0, points.lifeforce);
  points.lifeforce = min(points.lifeforce, (sh_int)32000);
}

bool TBeing::noLifeforce(int lifeforce) const
{
  return (points.lifeforce < lifeforce);
}

// END LIFEFORCE

short int TBeing::manaLimit() const
{
  return 100;
}


double TBeing::pietyLimit() const
{
  return 100.0;
}

positionTypeT TBeing::getPosition() const
{
  return specials.position;
}

bool TBeing::noMana(int mana) const
{
  return ((mostPowerstoneMana() + points.mana) < mana);
}

#if FACTIONS_IN_USE
bool TBeing::percLess(double perc) const
{
  return (faction.percent < perc);
}
#endif

bool TBeing::awake() const
{
  return ((specials.position > POSITION_SLEEPING) &&
          !(specials.affectedBy & (AFF_PARALYSIS | AFF_STUNNED)));
}

bool TBeing::isFlying() const
{
  return ((getPosition() == POSITION_FLYING));
}

bool TBeing::isGuildmaster() const
{
  return FALSE;
}


bool TBeing::isLevitating() const
{
  return isAffected(AFF_LEVITATING);
}

TBeing * TBeing::getFactionTarget() const
{
  return faction.target;
}

void TBeing::setFactionTarget(TBeing *ch)
{
  faction.target = ch;
}

bool TBeing::isBrother() const
{
  return (faction.type == FACT_BROTHERHOOD);
}

bool TBeing::isCult() const
{
  return (faction.type == FACT_CULT);
}

bool TBeing::isSnake() const
{
  return (faction.type == FACT_SNAKE);
}

bool TBeing::isUnaff() const
{
  return (faction.type == FACT_NONE);
}

bool TBeing::isSameFaction(TBeing *ch) const
{
  return (getFaction() == ch->getFaction());
}

void TBeing::setCaptiveOf(TBeing *ch)
{
  faction.captiveOf = ch;
}

void TBeing::addToHero(int num)
{
  heroNum += ((-num > getHeroNum()) ? -getHeroNum() :
    ((getHeroNum() + num > 100) ? (100 - getHeroNum()) : num));
}

factionTypeT TBeing::getFaction() const
{
  return faction.type;
}

void TBeing::setFaction(factionTypeT num)
{
  faction.type = num;
}

double TBeing::getPerc() const
{
  return faction.percent;
}

void TBeing::setPerc(double num)
{
  faction.percent = num;
}

#if FACTIONS_IN_USE
void TBeing::addToPerc(double num)
{
  faction.percent += num;
  if (faction.percent > 100.0)
    faction.percent = 100.0;
  if (faction.percent < 0.0)
    faction.percent = 0.0;
}
#endif

void TBeing::setFactActBit(int i)
{
  faction.actions |= i;
}

void TBeing::removeFactActBit(int i)
{
  faction.actions = faction.actions & ~i;
}

TBeing *TBeing::getCaptive() const
{
  return faction.captive;
}

TBeing * TBeing::getNextCaptive() const
{
  return faction.next_captive;
}

TBeing *TBeing ::getCaptiveOf() const
{
  return faction.captiveOf;
}

void TBeing::setCaptive(TBeing *ch)
{
  faction.captive = ch;
}

void TBeing::setNextCaptive(TBeing *ch)
{
  faction.next_captive = ch;
}

#if FACTIONS_IN_USE
double TBeing::getPercX(factionTypeT num) const
{
  return faction.percX[num];
}

void TBeing::setPercX(double num, factionTypeT fact)
{
  faction.percX[fact] = num;
}

void TBeing::addToPercX(double num, factionTypeT fact)
{
  faction.percX[fact] += num;
  if (faction.percX[fact] > 100.0)
    faction.percX[fact] = 100.0;
  if (faction.percX[fact] < 0.0)
    faction.percX[fact] = 0.0;
}
#endif

unsigned int TBeing::getFactAct() const
{
  return faction.actions;
}

void TBeing::setFactAct(unsigned int r)
{
  faction.actions = r;
}

void TBeing::setRace(race_t r)
{
  race = Races[r];
}

double TBeing::getMaxExp() const
{
  if (snum > -1)
    return 0;

  return points.max_exp;
}  


void TBeing::setMaxExp(double n)
{
  points.max_exp = n;
}


double TBeing::getExp() const
{
  if (snum > -1)
    return 0;

  return points.exp;
}

void TBeing::setExp(double n)
{
  points.exp = n;
}

void TBeing::addToExp(double n)
{
  points.exp += n;

  if (desc) {
    desc->session.xp += n;
    if (n > 0)
      desc->career.exp += n;
  }
}

double TBeing::getPiety() const
{
  return points.piety;
}

void TBeing::setPiety(double num)
{
  points.piety = num;
}

void TBeing::addToPiety(double num)
{
  points.piety += num;
  if (points.piety > pietyLimit())
    points.piety = pietyLimit();
  else if (points.piety < 0.0)
    points.piety = 0.0;
}

int TBeing::getSpellHitroll() const
{
   return points.spellHitroll;
}

void TBeing::setSpellHitroll(int h)
{
  points.spellHitroll = h;
}

int TBeing::getHitroll() const
{
  return points.hitroll;
}

void TBeing::setHitroll(int h)
{
  points.hitroll = h;
}

int TBeing::getDamroll() const
{
  return points.damroll;
}

void TBeing::setDamroll(int d)
{
  points.damroll = d;
}

int TBeing::getBank() const
{
  TDatabase db(DB_SNEEZY);
  int bankmoney=0;

  if(isPc()){
    db.query("select sum(talens) as talens from shopownedbank where player_id=%i",
	     getPlayerID());
    if(db.fetchRow())
      bankmoney=convertTo<int>(db["talens"]);
  }

  return bankmoney;
}

void TBeing::setBank(int bank)
{
  vlogf(LOG_BUG, fmt("%s called setBank(%i)!") % getName() % bank);
  points.bankmoney = bank;
}

int TBeing::getMoney() const
{
  return points.money;
}

void TBeing::setMoney(int money)
{
  points.money = money;
}

sexTypeT TBeing::getSex() const
{
  return player.sex;
}

void TBeing::setSexUnsafe(int sex)
{
  if (sex < SEX_NEUTER || sex > SEX_FEMALE) {
    vlogf(LOG_LOW, fmt("Bad sex on %s during set (%d)") %  getName() % sex);
    sex = 0;
  }

  setSex(sexTypeT(sex));
}

// is this character a pk character, 
// did they choose the pk option when creating etc
bool TBeing::isPkChar() const
{
  if(GetMaxLevel() >= MAX_NEWBIE_LEVEL && hasQuestBit(TOG_IS_PK_CHAR))
    return true;
  else
    return false;
}


// is this character actively pking, have they been involved in a pk
// conflict just recently etc
bool TBeing::isPking() const
{
  if(affectedBySpell(AFFECT_PLAYERKILL))
    return true;
  else
    return false;
}

bool TBeing::isValidPkTarget(const TBeing *attacker) const
{
  if(isPc() && attacker->isPc() && attacker->isPkChar() && isPkChar() && 
     (attacker->GetMaxLevel() >= (GetMaxLevel()-MAX_NEWBIE_LEVEL)) &&
     (attacker->GetMaxLevel() <= (GetMaxLevel()+MAX_NEWBIE_LEVEL)))
    return true;
  else
    return false;
}



void TBeing::setSex(sexTypeT sex)
{
  player.sex = sex;
}

byte TBeing::getInvisLevel() const
{
  return invisLevel;
}

void TBeing::setInvisLevel(byte num)
{
  if (desc && desc->m_bIsClient) {
    if ((invisLevel > 50) && (num < 50))
      desc->clientf(fmt("%d|%d") % CLIENT_INVIS % FALSE);

    if (num > 50)
      desc->clientf(fmt("%d|%d") % CLIENT_INVIS % TRUE);
  }

  invisLevel = num;
}

double TBeing::getMult() const
{
  return multAtt;
}

void TBeing::setMult(double mult)
{
  // having mult=0.0 causes some bad divbyzero things to happen
  // speculation that negative is just as bad
  mult = max(0.1, mult);

  multAtt = mult;
}

// I took these numbers from black leather (level 29)
// this is a crummy way to do this, but the right way is too hard
int getIronFleshArmor(wearSlotT slot){
  switch(slot){
    case WEAR_HEAD:
      return -86;
    case WEAR_NECK:
      return -49;
    case WEAR_BODY:
      return -184;
    case WEAR_BACK:
      return -86;
    case WEAR_ARM_R:
    case WEAR_ARM_L:
      return -49;
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
      return -24;
    case WEAR_HAND_R:
    case WEAR_HAND_L:
      return -37;
    case WEAR_FINGER_R:
    case WEAR_FINGER_L:
      return -12;
    case WEAR_WAISTE:
      return -98;
    case WEAR_EX_LEG_R:
    case WEAR_EX_LEG_L:
    case WEAR_LEGS_R:
    case WEAR_LEGS_L:
      return -61;
    case WEAR_FOOT_R:
    case WEAR_FOOT_L:
    case WEAR_EX_FOOT_R:
    case WEAR_EX_FOOT_L:
      return -24;
    case WEAR_NOWHERE:
    case HOLD_RIGHT:
    case HOLD_LEFT:
    case MAX_WEAR:
      return 0;
  }

  return 0;
}


sh_int TBeing::getArmor() const
{
  sh_int armor=1000;
  affectedData *af;
  TObj *to;
  int i;
  const TMonster *tm;

  // eq
  for(wearSlotT wearIndex = MIN_WEAR; wearIndex < MAX_WEAR; wearIndex++){
    if((to=dynamic_cast<TObj *>(equipment[wearIndex])) &&
       affectShouldApply(to, wearIndex)){
      for(i=0;i<MAX_OBJ_AFFECT;++i){
	if(to->affected[i].location == APPLY_ARMOR)
	  armor+=to->affected[i].modifier;
      }
    }

    // nekkid monk
    if(discs && !equipment[wearIndex] && doesKnowSkill(SKILL_IRON_FLESH)){
      armor+=(int)(getIronFleshArmor(wearIndex)*(getSkillValue(SKILL_IRON_FLESH)/100.0));
    }
    // correction for paired items
    if(discs && to && to->isPaired() && doesKnowSkill(SKILL_IRON_FLESH)) {
      armor-=(int)(getIronFleshArmor(wearIndex)*(getSkillValue(SKILL_IRON_FLESH)/100.0));
    }

    
  }

  // mobs have a different base AC
  if((tm=dynamic_cast<const TMonster *>(this))){
    sh_int num = 600 - ((sh_int) (20 * tm->getACLevel()));

    // mobs get either their default AC or their normal calculated AC,
    // whichever is better.
    armor=min(num, armor);
  }
  
  // spell affects
  for (af = affected; af; af = af->next) {
    if(af->location == APPLY_ARMOR)
      armor+=af->modifier;
  }

  return armor;
}

void TBeing::setArmor(sh_int armor)
{
  vlogf(LOG_PEEL, fmt("Something called setArmor(%i) on %s") % 
	armor % getName());

  //  points.armor = armor;
}

bool TBeing::isAffected(unsigned long bv) const
{
  if (!this) 
    {
      vlogf(LOG_BUG, "BAD - isAffected got called with this = null, investigate!");
      return 0;
    }
 
  return (specials.affectedBy & bv);
}

TBeing * TBeing::fight() const
{
  return specials.fighting;
}

ubyte TBeing::getLevel(classIndT i) const
{
  return player.level[i];
}

ush_int TBeing::getClass() const
{
  return player.Class;
}

ubyte TBeing::GetMaxLevel() const
{
  return player.max_level;
}

void TBeing::setMaxLevel(ubyte num)
{
  player.max_level = num;
}

sstring TBeing::displayExp() const
{
  char buf[256];

#if 0
  if (GetMaxLevel() <= 2)
    sprintf(buf, "%.5f", getExp());
  else if (GetMaxLevel() <= 5)
    sprintf(buf, "%.2f", getExp());
  else if (GetMaxLevel() <= 25)
    sprintf(buf, "%d", (int) getExp());
  else
    sprintf(buf, "%.4E", getExp());
#elif 1
  if (getExp() < 100)
    sprintf(buf, "%.3f", getExp());
  else
    sprintf(buf, "%.0f", getExp());
#else
  if (getExp() < 100)
    sprintf(buf, "%.3f", getExp());
  else if (getExp() < 10000)
    sprintf(buf, "%d", (int) getExp());
  else if (getExp() < 1000000)
    sprintf(buf, "%6.2fK", (getExp() / 1000.0));
  else if (getExp() < 1000000000)
    sprintf(buf, "%6.2fM", (getExp() / 1000000.0));
  else
    sprintf(buf, "%6.2fG", (getExp() / 1000000000.0));
#endif

  sstring xp_sstring = buf;
  return xp_sstring;
}

byte TBeing::getProtection() const
{
  return my_protection;
}

void TBeing::setProtection(byte num)
{
  my_protection = num;
}

void TBeing::addToProtection(byte num)
{
  my_protection = (byte) min(max((int) (num + my_protection), -100), 100);
}

// returns the number of points of AC we think is right based on what was
// set-up in the balance discussions.  This is AC from EQ only.
sh_int TBeing::suggestArmor() const
{
  double lev = GetMaxLevel() - getLevMod(getClass(), GetMaxLevel());
  int suggest = (int) (500 + 25*lev);

  // because monks need 2 barehands, they can't use shields
  // oomlat makes up for this in thac0 stuff
  // but lets just account for the AC here...
  if (hasClass(CLASS_MONK)) {
    // assumption is that for a given suit of eq, the shield is 25% of the AC
    suggest = (suggest * 3 / 4);
  }

  return suggest;
}

void TBeing::peeOnMe(const TBeing *ch)
{
  act("$n has just pissed on your foot!  Yuck!", TRUE, ch, 0, this, TO_VICT);
  act("$n cackles as $e pees on $N's foot!", TRUE, ch, 0, this, TO_NOTVICT);
  act("You relieve yourself on $N's foot.", TRUE, ch, 0, this, TO_CHAR);
}

const char * TBeing::getLongDesc() const
{
  if (player.longDescr)
    return player.longDescr;

  if (msgVariables.tMessages.msgLongDescr &&
      *msgVariables.tMessages.msgLongDescr)
    return msgVariables.tMessages.msgLongDescr;

  return NULL;
}

int TBeing::chiMe(TBeing *tLunatic)
{
  int bKnown  = tLunatic->getSkillValue(SKILL_CHI),
      tDamage,
      tMana;

  if (tLunatic->getSkillValue(SKILL_CHI) < 50 ||
      tLunatic->getDiscipline(DISC_MEDITATION_MONK)->getLearnedness() < 10) {
    tLunatic->sendTo("I'm afraid you don't have the training to do this.\n\r");
    return RET_STOP_PARSING;
  }

  if (tLunatic->checkPeaceful("You feel too peaceful to contemplate violence here.\n\r"))
    return RET_STOP_PARSING;

  if (tLunatic == this) {
    tMana = 100 - ::number(1, getSkillValue(SKILL_CHI) / 2);
    affectedData aff;

    if (affectedBySpell(SKILL_CHI)) {
      sendTo("You are already projecting your chi upon yourself.\n\r");
      return FALSE;
    }

    if (bSuccess(bKnown, SKILL_CHI)) {
      reconcileMana(TYPE_UNDEFINED, 0, tMana);

      act("You close your eyes and concentrate for a moment, then begin radiating an intense <R>heat<1> from your body.", TRUE, this, NULL, NULL, TO_CHAR);
      act("$n closes $s eyes in concentration, then begins radiating an intense <R>heat<1> from $s body.", TRUE, this, NULL, NULL, TO_ROOM);

      aff.type      = SKILL_CHI;
      aff.level     = bKnown;
      aff.duration  = (3 + (bKnown / 2)) * UPDATES_PER_MUDHOUR;
      aff.location  = APPLY_IMMUNITY;
      aff.modifier  = IMMUNE_COLD;
      aff.modifier2 = ((bKnown * 2) / 3);
      aff.bitvector = 0;
      affectTo(&aff, -1);
    } else {
      act("You are unable to focus your mind.",
          TRUE, this, NULL, NULL, TO_CHAR);

      if (getMana() >= 0)
        reconcileMana(TYPE_UNDEFINED, 0, tMana/2);
    }

    return true;
  }

  if (this->isImmortal() || this->inGroup(*tLunatic))
    return FALSE;

  if (tLunatic->bSuccess(bKnown, SKILL_CHI)) {
    tDamage = getSkillDam(this, SKILL_CHI,
                          tLunatic->getSkillLevel(SKILL_CHI),
                          tLunatic->getAdvLearning(SKILL_CHI));
    tMana = ::number((tDamage / 2), tDamage);

    if (tLunatic->getMana() < tMana) {
      tLunatic->sendTo("You lack of the chi to do this.\n\r");
      return RET_STOP_PARSING;
    } else
      tLunatic->reconcileMana(TYPE_UNDEFINED, 0, tMana);

    act("You unleash your chi upon $N!",
        FALSE, tLunatic, NULL, this, TO_CHAR);

    if (affectedBySpell(SKILL_CHI)) {
      act("A bright <W>aura<1> flares up around $N, deflecting your attack and then striking back!\n\rYour vision goes <r>red<1> as the pain overwhelms you!", TRUE, tLunatic, NULL, this, TO_CHAR);
      act("A bright <W>aura<1> flares up around $N, deflecting $n's chi attack and then striking back!", TRUE, tLunatic, NULL, this, TO_NOTVICT);
      act("A bright <W>aura<1> flares up around you, deflecting $n's chi attack and then striking back!", TRUE, tLunatic, NULL, this, TO_VICT);

      if (this->reconcileDamage(tLunatic, ::number((tDamage / 2), tDamage), SKILL_CHI) == -1)
        return (DELETE_THIS | RET_STOP_PARSING);
    } else {
      act("...$N screws up $S face in agony.",
          TRUE, tLunatic, NULL, this, TO_CHAR);
      act("$n exerts $s <r>chi force<1> on you, causing extreme pain.",
          TRUE, tLunatic, NULL, this, TO_VICT);
      act("$N screws up $S face in agony.",
          TRUE, tLunatic, NULL, this, TO_NOTVICT);

      if (tLunatic->reconcileDamage(this, tDamage, SKILL_CHI) == -1)
        return DELETE_VICT;
    }
  } else {
    act("You fail to harm $N with your <r>blast of chi<z>!",
        FALSE, tLunatic, NULL, this, TO_CHAR);
    act("You escape $n's attempt to chi you!",
        TRUE, tLunatic, NULL, this, TO_VICT);
    act("$N avoids $n's attempt to chi them!",
        TRUE, tLunatic, NULL, this, TO_NOTVICT);
  }

  return true;
}

void TBeing::addObjUsed(TObj *obj, int duration) {
  
  affectedData aff;

  aff.type = AFFECT_OBJECT_USED;
  aff.duration = duration;
  aff.modifier = obj->number;

  affectTo(&aff);

}

bool TBeing::checkObjUsed(TObj *obj) {

  affectedData *hjp;

  if (!affected)
    return FALSE;

  for (hjp = affected; hjp; hjp = hjp->next) {
    if (hjp->type == AFFECT_OBJECT_USED) {
      if (hjp->modifier == obj->number)
        return TRUE;
    }
  }
  return FALSE;

}

sstring TBeing::thirdPerson(const int pos)
{
  if (pos == POS_OBJECT) {
    switch (getSex()) {
      case SEX_MALE:
        return "him";
      case SEX_FEMALE:
        return "her";
      default:
        return "it";
    }
  } else if (pos == POS_SUBJECT) {
    switch (getSex()) {
      case SEX_MALE:
        return "he";
      case SEX_FEMALE:
        return "she";
      default:
        return "it";
    }
  } else if (pos == POS_POSSESS) {
    switch (getSex()) {
      case SEX_MALE:
        return "his";
      case SEX_FEMALE:
        return "her";
      default:
        return "its";
    }
  }
  vlogf(LOG_BUG, fmt("thirdPerson called on %s in strange circumstance") % name);
  return "???";
}
