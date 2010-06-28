//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "player_data.cc" - Function related to saving and loading of
//                        player files.
//
//////////////////////////////////////////////////////////////////////////

extern "C" {
#include <unistd.h>
#include <dirent.h>
}

#include <errno.h>

#include "extern.h"
#include "room.h"
#include "being.h"
#include "client.h"
#include "low.h"
#include "handler.h"
#include "monster.h"
#include "configuration.h"
#include "account.h"
#include "person.h"
#include "charfile.h"
#include "shop.h"
#include "skills.h"
#include "mail.h"
#include "statistics.h"
#include "combat.h"
#include "database.h"
#include "materials.h"


wizListInfo *wiz;

// defined in gaining.cc
int CalcRaiseDisc(int natLearn, CDiscipline *disc, bool drop);

void TBeing::initDescStuff(charFile *st)
{
  Descriptor *d;

  if (!(d = desc)) {
    vlogf(LOG_BUG, "Big ole problems, character in initDescStuff() with no descriptor. TELL BRUTIUS!");
    return;
  }
  if (load_char(getName(), st)) {
    d->screen_size = st->screen;
    d->last.hit = getHit();
    d->last.mana = getMana();
    d->last.lifeforce = getLifeforce();
    d->last.piety = getPiety();
    d->last.move = getMove();
    d->last.exp = getExp();
    d->last.money = getMoney();
    d->bad_login = st->bad_login;
    d->playerID = 0;
  } else {
    vlogf(LOG_BUG, format("Big ole problems. Player reconnected with no player file (%s)!") %  getName());
    return;
  }
  if (st->plr_act & PLR_STEALTH)
    d->clientf(format("%d|%d") % CLIENT_STEALTH % TRUE);

  wizFileRead();
}

void TBeing::resetEffectsChar()
{
  return;
}

void TPerson::resetChar()
{
  char recipient[100], *tmp, *tmstr;
  affectedData *af;

  roomp = NULL;

  if (isPlayerAction(PLR_MAILING))
    remPlayerAction(PLR_MAILING);
  if (isPlayerAction(PLR_BUGGING))
    remPlayerAction(PLR_BUGGING);

  setProtection(0);

  if (getCombatMode() == ATTACK_BERSERK)
    setCombatMode(ATTACK_NORMAL);

  if (affectedBySpell(SKILL_BERSERK)) {
    affectFrom(SKILL_BERSERK);
  }

  desc->point_roll = 0;
  visionBonus = 0;

  if(hasQuestBit(TOG_HAS_NIGHTVISION))
    visionBonus+=2;

  if (getRace() == RACE_OGRE)
    setMaterial(MAT_OGRE_HIDE);
  else
    setMaterial(MAT_HUMAN_FLESH);

  *desc->last_teller = '\0';
  *desc->last_told = '\0';

  desc->playerID=0;

  if (!getRace())
    setRace(RACE_HUMAN);

  if ((getRace() == RACE_DWARF) || (getRace() == RACE_HOBBIT) || (getRace() == RACE_GNOME)) {
    if (!isAffected(AFF_INFRAVISION))
      SET_BIT(specials.affectedBy, AFF_INFRAVISION);
  }

  if(hasQuestBit(TOG_IS_BLIND)){
    SET_BIT(specials.affectedBy, AFF_BLIND);
  }

  if ((getRace() == RACE_HOBBIT) && getNatSkillValue(SKILL_STEAL) < 23) {
    setNatSkillValue(SKILL_STEAL,23);
    setSkillValue(SKILL_STEAL,23);
  }
  if (hasClass(CLASS_RANGER)) {
    if (getNatSkillValue(SKILL_CONS_VEGGIE) < MAX_SKILL_LEARNEDNESS) {
      setNatSkillValue(SKILL_CONS_VEGGIE,MAX_SKILL_LEARNEDNESS);
      setSkillValue(SKILL_CONS_VEGGIE,MAX_SKILL_LEARNEDNESS);
    }
    if (getNatSkillValue(SKILL_CONS_ANIMAL) < MAX_SKILL_LEARNEDNESS) {
      setNatSkillValue(SKILL_CONS_ANIMAL,MAX_SKILL_LEARNEDNESS);
      setSkillValue(SKILL_CONS_ANIMAL,MAX_SKILL_LEARNEDNESS);
    }
    if (getNatSkillValue(SKILL_CONS_REPTILE) < MAX_SKILL_LEARNEDNESS) {
      setNatSkillValue(SKILL_CONS_REPTILE,MAX_SKILL_LEARNEDNESS);
      setSkillValue(SKILL_CONS_REPTILE,MAX_SKILL_LEARNEDNESS);
    }
    if (getNatSkillValue(SKILL_CONS_PEOPLE) < MAX_SKILL_LEARNEDNESS) {
      setNatSkillValue(SKILL_CONS_PEOPLE,MAX_SKILL_LEARNEDNESS);
      setSkillValue(SKILL_CONS_PEOPLE,MAX_SKILL_LEARNEDNESS);
    }
    if (getNatSkillValue(SKILL_CONS_GIANT) < MAX_SKILL_LEARNEDNESS) {
      setNatSkillValue(SKILL_CONS_GIANT,MAX_SKILL_LEARNEDNESS);
      setSkillValue(SKILL_CONS_GIANT,MAX_SKILL_LEARNEDNESS);
    }
    if (getNatSkillValue(SKILL_CONS_OTHER) < MAX_SKILL_LEARNEDNESS) {
      setNatSkillValue(SKILL_CONS_OTHER,MAX_SKILL_LEARNEDNESS);
      setSkillValue(SKILL_CONS_OTHER,MAX_SKILL_LEARNEDNESS);
    }
    if (getNatSkillValue(SKILL_RIDE) < 50) {
      setNatSkillValue(SKILL_RIDE,50);
      setSkillValue(SKILL_RIDE,50);
    }
  }
  if (hasClass(CLASS_CLERIC) || hasClass(CLASS_SHAMAN) || hasClass(CLASS_MAGE) || (race->getRace() == RACE_ELVEN)) {
    if (!hasQuestBit(TOG_SPELL_READ_MAGIC)) {
      setQuestBit(TOG_SPELL_READ_MAGIC);
    }
//    if (getSkillValue(SKILL_READ_MAGIC) < MAX_SKILL_LEARNEDNESS)
//     setSkillValue(SKILL_READ_MAGIC,MAX_SKILL_LEARNEDNESS);
  }
  classIndT ij;
  for (ij = MIN_CLASS_IND; ij < MAX_CLASSES; ij++) {
    if (getLevel(ij) > MAX_IMMORT) {
      vlogf(LOG_BUG, format("%s was above level %d in class %d! Setting level to 1.\n\r") %  name % MAX_IMMORT % ij);
      setLevel(ij, 1);
      calcMaxLevel();
    }
  }
  hunt_dist = 0;

// this hitroll is set by spells (affects) which were already zero'd and
// are (at this point) in effect.  Bad, bad, bad!
//  setSpellHitroll(0);

  setHitroll(0);
  setDamroll(0);

  next_fighting = NULL;
  specials.fighting = NULL;
  setPosition(POSITION_STANDING);

  if (isAffected(AFF_SLEEP) && (getPosition() > POSITION_SLEEPING)) {
    sendTo("You grow sleepy and can remain awake no longer.\n\r");
    act("$n collapses as $e falls asleep.", TRUE, this, 0, 0, TO_ROOM);
    setPosition(POSITION_SLEEPING);
  }

  if (getHit() <= 0) {
    // this can happen, bat 9-21-96
    autoDeath();
  }
  if (getMove() <= 0)
    setMove(1);
  if (getMana() <= 0)
    setMana(1);
  if (getPiety() < 0.0)
    setPiety(0.0);

  points.maxMana = 0;
  points.maxMove = 0;

  if ((GetMaxLevel() == 52) || (GetMaxLevel() == 51))
    setMoney(0);

  if (getBank() > GetMaxLevel() * 10000)
    vlogf(LOG_PIO, format("%s has %d talens in bank.") %  getName() % getBank());

  if (getMoney() > GetMaxLevel() * 10000)
    vlogf(LOG_PIO, format("%s has %d talens.") %  getName() % getMoney());

  if (isPc() && hasWizPower(POWER_GOD)) {
    statTypeT iStat;
    for(iStat=MIN_STAT;iStat<MAX_STATS;iStat++)
      setStat(STAT_CURRENT, iStat, 250);
  }

  // This so the save-bonus it gives is corrected for appropriately
  if (affectedBySpell(SPELL_BLESS)) {
    affectFrom(SPELL_BLESS);
    sendTo("Your blessing has expired.\n\r");
  }

  if (affectedBySpell(SPELL_BLESS_DEIKHAN)) {
    affectFrom(SPELL_BLESS_DEIKHAN);
    sendTo("Your blessing has expired.\n\r");
  }
#if 0
  if (affectedBySpell(SPELL_SYNOSTODWEOMER)) {
    affectFrom(SPELL_SYNOSTODWEOMER);
    sendTo("You lose the affects from synostodweomer.\n\r");
  }
#endif
  classSpecificStuff();

  _parse_name(getName(), recipient);

  for (tmp = recipient; *tmp; tmp++) {
    if (isupper(*tmp))
      *tmp = tolower(*tmp);
  }
  if (!Config::NoMail() && has_mail(recipient) && 
      gamePort != Config::Port::BUILDER)
    sendTo(format("\n\rYou have %sMAIL%s.\n\r") % bold() % norm());

  time_t ct = player.time->last_logon ? player.time->last_logon : time(0);
  tmstr = (char *) asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  if (*lastHost)
    sendTo(format("\n\rYou last logged in %s from %s\n\r\n\r") % tmstr % lastHost);
  else {
    sendTo("This is your first login.\n\r");
  }
  // Now that we sent the character where they last logged in from
  // reset the lastHost to the current hostname 

  strncpy(lastHost, desc->host.c_str(), 38);
  lastHost[39] = '\0';

  if (desc && desc->bad_login)
    sendTo(format("%sYou have had %d unsuccessful logins to your player since last login.%s\n\r") % red() % desc->bad_login % norm());

  // Now that bad logins have been sent, reset it to 0 
  desc->bad_login = 0;

  //  setArmor(1000);  // need to set this to 100 or we have double spell affect

  // racial stuff 
  setRacialStuff();

// This works in conjunction with the end of loadFromSt which excludes
// certain affects which are then put on here.
  for (af = affected; af; af = af->next) {
    affectModify(af->location, (unsigned) af->modifier,
              (unsigned) af->modifier2, af->bitvector, TRUE, SILENT_YES);
  }

  if(doesKnowSkill(SKILL_TELE_SIGHT))
    SET_BIT(specials.affectedBy, AFF_SENSE_LIFE);


  if (isImmortal()) {
    setHit(hitLimit());
    setMana(manaLimit());
    setLifeforce(9000);
    setMove(moveLimit());
    setPiety(pietyLimit());
  }

  if (isPc() && isPlayerAction(PLR_LOGGED))
    vlogf(LOG_PIO, format("%s has LOG bit set.") %  getName());
  if (isPc() && isPlayerAction(PLR_GODNOSHOUT))
    vlogf(LOG_PIO, format("%s has GOD-NOSHOUT bit set.") %  getName());

  // DO NOT SAVE HERE - doStart makes assumptions
}

bool raw_save_char(const char *name, charFile *char_element)
{
  FILE *fl;
  char buf[256];

  sprintf(buf, "player/%c/%s", LOWER(name[0]), sstring(name).lower().c_str());

  if (!(fl = fopen(buf, "w")))
    return FALSE;

  if (!(fwrite(char_element, sizeof(charFile), 1, fl))) {
    fclose(fl);
    return FALSE;
  }
  fclose(fl);

  TDatabase db(DB_SNEEZY);
  db.query("update player p, account a set p.talens=%i, p.account_id=a.account_id where lower(p.name)=lower('%s') and a.name='%s'",
	   char_element->money, name, char_element->aname);

  return TRUE;
}

// Load a char, TRUE if loaded, FALSE if not
bool load_char(const sstring &name, charFile *char_element)
{
  FILE *fl;
  char buf[256];

  sprintf(buf, "player/%c/%s", LOWER(name[0]), name.lower().c_str());

  if (!(fl = fopen(buf, "r")))
    return FALSE;

  int rc = fread(char_element, sizeof(charFile), 1, fl);
  fclose(fl);

  TDatabase db(DB_SNEEZY);
  db.query("select talens from player where lower(name)=lower('%s') and talens is not null",
	   name.c_str());
  if(db.fetchRow()){
    char_element->money=convertTo<int>(db["talens"]);
  } else {
    db.query("update player set talens=%i where lower(name)=lower('%s')",
	     char_element->money, name.c_str());
  }

  if (rc == 1)
    return TRUE;
  return FALSE;
}


// copy vital data from a players char-structure to the file structure
void TPerson::storeToSt(charFile *st)
{
  affectedData *af;
  TThing *char_eq[MAX_WEAR];

  saveToggles();
  saveWizPowers();
  //msgVariables.savedown();

  st->flags = 0;

  if (desc && desc->account) 
    strcpy(st->aname, desc->account->name.c_str());
  else {
    vlogf(LOG_BUG, format("storeToSt for %s with no account info") %  getName());
  }

  wearSlotT ij;
  for (ij = MIN_WEAR; ij < MAX_HUMAN_WEAR; ij++) {
    int mapped_slot = mapSlotToFile(ij);
    st->body_health[mapped_slot] = getCurLimbHealth(ij);
    st->body_flags[mapped_slot] = getLimbFlags(ij);
  }
  for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
    if (equipment[ij])
      char_eq[ij] = unequip_char_for_save(this, ij);
    else
      char_eq[ij] = NULL;
  }

  int j;
  for (af = affected, j = 0; j < MAX_AFFECT; j++) {
    // the 3 track skills requires a specials.hunting
    // since this isn't maintained, don't save these effects.
    if (af &&
        (af->type != SKILL_SEEKWATER &&
         af->type != SKILL_TRACK &&
         af->type != SPELL_TRAIL_SEEK)) {
      st->affected[j] = *af;

      applyTypeT att = mapFileToApply(st->affected[j].location);
      affectModify(att,
                   applyTypeShouldBeSpellnum(att) ? mapFileToSpellnum(st->affected[j].modifier) : st->affected[j].modifier,
		   st->affected[j].modifier2,
		   st->affected[j].bitvector, FALSE, SILENT_YES);
      af = af->next;
    } else {
      st->affected[j].type = TYPE_UNDEFINED;	     // Zero signifies not used 
      st->affected[j].duration = 0;
      st->affected[j].location = mapApplyToFile(APPLY_NONE);
      st->affected[j].modifier = 0;
      st->affected[j].bitvector = 0;
      st->affected[j].unused2 = 0;
      if (af)
        af = af->next;
    }
  }

  if ((j >= MAX_AFFECT) && af && af->next)
    vlogf(LOG_SILENT, format("WARNING: (%s) OUT OF STORE ROOM FOR AFFECTED TYPES!!!") %  getName());

  // Save the discipline learning
  // unused disc_learning values should be 0 from charFile ctor
  discNumT dnt;
  for (dnt = MIN_DISC; dnt < MAX_DISCS ; dnt++) {
    CDiscipline *cd = getDiscipline(dnt);
    if (cd)
      st->disc_learning[mapDiscToFile(dnt)] = cd->getNatLearnedness();
  }

// Save all skill numbers
  memset(&st->skills, SKILL_MIN, sizeof(st->skills));
  spellNumT snt;
  for (snt = MIN_SPELL; snt < MAX_SKILL; snt++) {
    int mappedskill = mapSpellnumToFile(snt);
    if(discArray[snt])
      st->skills[mappedskill] = getRawNatSkillValue(snt);
  }

  st->birth = player.time->birth;
  st->played = player.time->played;
  st->played += (long) (time(0) - player.time->logon);
  st->last_logon = time(0);

  player.time->played = st->played;
  player.time->logon = time(0);

  st->hometown = player.hometown;
  st->hometerrain = ubyte(player.hometerrain);
  st->weight = getWeight();
  st->height = getHeight();
  st->sex = getSex();
  st->Class = getClass();

  classIndT cit;
  for (cit = MIN_CLASS_IND; cit < MAX_CLASSES; cit++) {
    st->level[cit] = getLevel(cit);
    st->doneBasic[cit] = player.doneBasic[cit];
  }
  st->race = getRace();
  st->attack_type = getCombatMode();

  for(int i=0;i<MAX_STATS;++i){
    st->stats[i]=chosenStats.values[i];
  }
  st->wimpy = wimpy;

  
  st->practices = practices;
  short tmp=0;
  // these pracs are switched in the actual pfiles for some dumb reason
  tmp=st->practices.prac[WARRIOR_LEVEL_IND];
  st->practices.prac[WARRIOR_LEVEL_IND]=st->practices.prac[THIEF_LEVEL_IND];
  st->practices.prac[THIEF_LEVEL_IND]=tmp;

  tmp=st->practices.prac[MONK_LEVEL_IND];
  st->practices.prac[MONK_LEVEL_IND]=st->practices.prac[RANGER_LEVEL_IND];
  st->practices.prac[RANGER_LEVEL_IND]=tmp;


//  st->points = points;
// storing pointsData
  st->mana = points.mana;
  st->maxMana = points.maxMana;
  st->piety = points.piety;
  st->lifeforce = points.lifeforce;
  st->hit = points.hit;
  st->maxHit = points.maxHit;
  st->move = points.move;
  st->maxMove = points.maxMove;
  st->money = points.money;
  st->bankmoney = points.bankmoney;
  st->exp = points.exp;
  st->max_exp = points.max_exp;
  st->spellHitroll = points.spellHitroll;
  st->hitroll = points.hitroll;
  st->damroll = points.damroll;
  //  st->armor = points.armor;
// end storing pointsData
#if FACTIONS_IN_USE
  st->f_percent = getPerc();
  factionTypeT ik;
  for (ik = MIN_FACTION; ik < MAX_FACTIONS; ik++)
    st->f_percx[ik] = getPercX(ik);
  for (; ik < ABS_MAX_FACTION; ik++)
    st->f_percx[ik] = (double) 0.0;
#else
  st->f_percent = (double) 0.0;
  factionTypeT ik;
  for (ik = MIN_FACTION; ik < MAX_FACTIONS; ik++)
    st->f_percx[ik] = (double) 0.0;
  for (; ik < ABS_MAX_FACTION; ik++)
    st->f_percx[ik] = (double) 0.0;
#endif
  st->align_ge = faction.align_ge;
  st->align_lc = faction.align_lc;
  st->whichguild = faction.whichguild;

  st->f_type = getFaction();
  st->f_actions = getFactAct();

  if (!desc) {
    vlogf(LOG_BUG, "No desc while writing charFile");
  }

  st->screen = getScreen();
  st->autobits = desc->autobits;
  st->bad_login = desc->bad_login;

  for (j = 0; j < 16; j++)
    st->alias[j] = desc->alias[j];

  st->best_rent_credit = desc->best_rent_credit;

  st->base_age = getBaseAge();
  st->age_mod = age_mod;

  st->fatigue = 0;
  st->hero_num = getHeroNum();

  st->plr_act = desc->plr_act;
  st->plr_color = desc->plr_color;
  st->plr_colorSub = desc->plr_colorSub;
  st->plr_colorOff = desc->plr_colorOff;

  st->armor = 100;
  st->spellHitroll = 0;
  st->hitroll = 0;
  st->damroll = 0;

#if 0
  if (title)
    strcpy(st->title, title);
  else
    *st->title = '\0';
#endif

  strcpy(st->lastHost, lastHost);

  if (getDescr())
    strcpy(st->description, getDescr());
  else
    *st->description = '\0';

  strcpy(st->name, name);

  condTypeT ic;
  for (ic = MIN_COND; ic < MAX_COND_TYPE; ++ic)
    st->conditions[ic] = getCond(ic);

  int i;
  for (af = affected, i = 0; i < MAX_AFFECT; i++) {
    if (af) {
      applyTypeT att = mapFileToApply(st->affected[i].location);
      affectModify(att,
                   applyTypeShouldBeSpellnum(att) ? mapFileToSpellnum(st->affected[i].modifier) : st->affected[i].modifier,
                    st->affected[i].modifier2,
		    st->affected[i].bitvector, TRUE, SILENT_YES);
      af = af->next;
    }
  }
  for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
    if (char_eq[ij])
      equipChar(char_eq[ij], ij, SILENT_YES);
  }
  affectTotal();
}				/* Char to store */

void TPerson::loadFromSt(charFile *st)
{
  int i;

  name = mud_str_dup(st->name);

  setBaseAge(st->base_age);
  age_mod = st->age_mod;

  for (i = 0; i < 16; i++) 
    desc->alias[i] = st->alias[i];

  desc->playerID=0;
  
  setRace(race_t(st->race));

  wearSlotT iw;
  for (iw = MIN_WEAR; iw < MAX_HUMAN_WEAR; iw++) {
    wearSlotT mapped_slot = mapFileToSlot(iw);
    setCurLimbHealth(mapped_slot, st->body_health[iw]);
    setLimbFlags(mapped_slot, st->body_flags[iw]);
  }
  for (;iw < MAX_WEAR;iw++) {
    setCurLimbHealth(iw, 255);
    setLimbFlags(iw, 0);
  }

  setSexUnsafe(st->sex);
  setClass(st->Class);

  classIndT cit;
  for (cit = MIN_CLASS_IND; cit < MAX_CLASSES; cit++) {
    setLevel(cit, st->level[cit]);
    player.doneBasic[cit] = st->doneBasic[cit];
  }

  calcMaxLevel();

  setCombatMode(st->attack_type);

  shortDescr = NULL;
  player.longDescr = NULL;
#if 0
  if (*st->title) {
    title = mud_str_dup(st->title);
  } else
    title = NULL;
#endif

  if (*st->description) {
    setDescr(mud_str_dup(st->description));
  } else
    setDescr(NULL);

// Need toggles loaded before skills
  loadToggles();
  loadWizPowers();
  msgVariables.tPlayer = this;
  msgVariables.initialize();

  // we have to assign discs before we set the learnedness
  // BUT make sure it has a class before calling this
  assignDisciplinesClass();


  practices = st->practices;

  short tmp=0;
  // these pracs are switched in the actual pfiles for some dumb reason
  tmp=practices.prac[WARRIOR_LEVEL_IND];
  practices.prac[WARRIOR_LEVEL_IND]=practices.prac[THIEF_LEVEL_IND];
  practices.prac[THIEF_LEVEL_IND]=tmp;

  tmp=practices.prac[MONK_LEVEL_IND];
  practices.prac[MONK_LEVEL_IND]=practices.prac[RANGER_LEVEL_IND];
  practices.prac[RANGER_LEVEL_IND]=tmp;


  discNumT dnt;
  for (dnt = MIN_DISC; dnt < MAX_DISCS; dnt++) {
    CDiscipline *tmpCD;
    byte dl = st->disc_learning[mapDiscToFile(dnt)];
    if (dl > MAX_DISC_LEARNEDNESS) {
      vlogf(LOG_BUG, format("disc %d on %s pfile with learning %d.  ERROR") % 
             dnt % getName() % dl);
    }
    tmpCD = getDiscipline(dnt);
    if (tmpCD) {
      if (GetMaxLevel() > MAX_MORT) {
        tmpCD->setNatLearnedness(MAX_DISC_LEARNEDNESS);
        tmpCD->setLearnedness(MAX_DISC_LEARNEDNESS);
        continue;
      }
      tmpCD->setNatLearnedness(min((int) MAX_DISC_LEARNEDNESS, max((int) dl, 0)));
      tmpCD->setLearnedness(tmpCD->getNatLearnedness());
      if (!(tmpCD->ok_for_class) && (GetMaxLevel() <= MAX_MORT) &&
                                   (tmpCD->getLearnedness() > 0)) {
        vlogf(LOG_BUG, format("disc %d on %s with learning %d.  ERROR") % 
             dnt % getName() % tmpCD->getLearnedness());
      }
    }
  }

  spellNumT snt;
  for (snt = MIN_SPELL; snt < MAX_SKILL;snt++) {
    if (discArray[snt]) {
      int mappedskill = mapSpellnumToFile(snt);
      setNatSkillValue(snt, 
          ((GetMaxLevel() > MAX_MORT) ? MAX_SKILL_LEARNEDNESS : st->skills[mappedskill]));
      int xi = getMaxSkillValue(snt);
      setSkillValue(snt, min((int) st->skills[mappedskill], xi));
    }
  }

  player.hometown = st->hometown;
  player.hometerrain = territoryT(st->hometerrain);

  player.time->birth = st->birth;
  player.time->played = st->played;
  player.time->logon = time(0);
  player.time->last_logon = st->last_logon;

  
  
  strcpy(lastHost, st->lastHost);

  setWeight(st->weight);
  height = st->height;

  for(int i=0;i<MAX_STATS;++i){
    chosenStats.values[i]=st->stats[i];
  }
  convertAbilities();

  // points = st->points;
  // loading pointData info
   points.mana = st->mana;
   points.maxMana = st->maxMana;
   points.piety = st->piety;
   points.lifeforce = st->lifeforce;
   points.hit = st->hit;
   points.maxHit = st->maxHit;
   points.move = st->move;
   points.maxMove = st->maxMove;
   points.money = st->money;
   points.bankmoney = st->bankmoney;
   points.exp = st->exp;
   points.max_exp = st->max_exp;
   points.spellHitroll = st->spellHitroll;
   points.hitroll = st->hitroll;
   points.damroll = st->damroll;
   //   points.armor = st->armor;
  // end loading pointData info
  if (desc) {
    desc->bad_login = st->bad_login;
    desc->screen_size = st->screen;
  }
  wimpy = st->wimpy;

#if FACTIONS_IN_USE
  setPerc(st->f_percent);
  factionTypeT ij;
  for (ij = MIN_FACTION; ij < MAX_FACTIONS; ij++)
    setPercX(st->f_percx[ij], ij);
#endif
#if 0
  faction.whichguild = st->whichguild;
  faction.align_ge = st->align_ge;
  faction.align_lc = st->align_lc;
#endif

  mud_assert(st->f_type >= MIN_FACTION && st->f_type < MAX_FACTIONS, "bad faction");
  setFaction(factionTypeT(st->f_type));
  setFactAct(st->f_actions);

  desc->autobits = st->autobits;
  desc->best_rent_credit = st->best_rent_credit;

  setCaptive(NULL);
  setNextCaptive(NULL);
  setCaptiveOf(NULL);

  // fatigue = st->fatigue;
  setHeroNum(st->hero_num);

  TDatabase db(DB_SNEEZY);

  desc->playerID=0;
  db.query("select p_type, hp, mana, move, money, exp, room, opp, tank, piety, lifeforce, time from playerprompt where player_id=%i", getPlayerID());
  
  if(db.fetchRow()){
    desc->prompt_d.type = convertTo<int>(db["p_type"]);
    
    strcpy(desc->prompt_d.hpColor, db["hp"].c_str());
    strcpy(desc->prompt_d.manaColor, db["mana"].c_str());
    strcpy(desc->prompt_d.moveColor, db["move"].c_str());
    strcpy(desc->prompt_d.moneyColor, db["money"].c_str());
    strcpy(desc->prompt_d.expColor, db["exp"].c_str());
    strcpy(desc->prompt_d.roomColor, db["room"].c_str());
    strcpy(desc->prompt_d.oppColor, db["opp"].c_str());
    strcpy(desc->prompt_d.tankColor, db["tank"].c_str());
    strcpy(desc->prompt_d.pietyColor, db["piety"].c_str());
    strcpy(desc->prompt_d.lifeforceColor, db["lifeforce"].c_str());
    strcpy(desc->prompt_d.timeColor, db["time"].c_str());
  } else {
    db.query("insert into playerprompt (player_id, p_type, hp, mana, move, money, exp, room, opp, tank, piety, lifeforce, time) values (%i, 267869, '', '', '', '', '', '', '', '', '', '', '')", getPlayerID());
  }




  desc->plr_act = st->plr_act;
  desc->plr_color = st->plr_color;
  desc->plr_colorSub = st->plr_colorSub;
  desc->plr_colorOff = st->plr_colorOff;

  specials.act = 0;
  //  setArmor(1000);
  setSpellHitroll(0);
  setHitroll(0);
  setDamroll(0);

  condTypeT ic;
  for (ic = MIN_COND; ic < MAX_COND_TYPE; ++ic)
    setCond(ic, st->conditions[ic]);

// this works with resetChar.  Its kind of backwards the way this works but
// we exclude some affects from this applay affects to put them on in
// resetChar.  Others of these we strip away and then add back in like
// armor spell.  I left it this way cause although duplicative, I forsee
// someone changing things around and seperating the functions of load and
// reset char 
  for (i = 0; i < MAX_AFFECT; i++) {
    if (st->affected[i].type) {
        rentAffectTo(&st->affected[i]);
    } 
  }

  // tacked on account and player id here
  db.query("select id as player_id, account_id, load_room from player where id = %i", getPlayerID());
  db.fetchRow();
  in_room = convertTo<int>(db["load_room"]);
  player.account_id = convertTo<int>(db["account_id"]);
  player.player_id = convertTo<int>(db["player_id"]);
  if(!in_room && st->load_room)
    in_room = st->load_room;

  affectTotal();

}

void TPerson::rentAffectTo(saveAffectedData *af)
{
  if (af->type == TYPE_UNDEFINED)
    return;

  affectedData *a;

  applyTypeT att = mapFileToApply(af->location);
  spellNumT snt = mapFileToSpellnum(af->type);

// Most things send on to affectTo
  if ((snt == SKILL_BERSERK) || (snt == SPELL_SYNOSTODWEOMER)) {
    return;
  } else if ((att == APPLY_CURRENT_HIT) || (att == APPLY_HIT)) {
    //mud_assert(af->duration != 0, "affectTo() with 0 duration affect");
    a = new affectedData(*af);
    a->next = affected;
    affected = a;
  } else {
    a = new affectedData(*af);

    // when assigning, lets not lose track of the renew value we saved off
    // pass it through in the affectTo call
    affectTo(a, af->renew);
    return;
  } 
}

void TBeing::convertAbilities()
{
}

void TBeing::saveChar(int load_room)
{
  charFile st;
  FILE *fl;
  TBeing *tmp = NULL;
  char buf[256];
  char buf2[256];

  if (dynamic_cast<TMonster *>(this)) {
    // save money for shop keepers, if they're owned
    if(isShopkeeper()){
      unsigned int shop_nr;
    
      for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != this->number); shop_nr++);
    
      if (shop_nr >= shop_index.size()) {
	vlogf(LOG_BUG, format("Warning... shop # for mobile %d (real nr) not found.") %  mob_index[this->number].virt);
	return;
      }
      
      TDatabase db(DB_SNEEZY);
      db.query("update shopowned set gold=%i where shop_nr=%i",
	       getMoney(), shop_nr);
    }


    if (!IS_SET(specials.act, ACT_POLYSELF) || !desc)
      return;
  
    if (!(tmp = desc->original))
      return;
    tmp->desc = desc;

  } else {
    if (!desc)
      return;

    if ((desc->connected != CON_PLYNG))
      return;

    tmp = NULL;
  }

  memset(&st, 0, sizeof(charFile));
  st.load_room = (short) load_room;

  if (!tmp)
    dynamic_cast<TPerson *>(this)->storeToSt(&st);
  else
    dynamic_cast<TPerson *>(tmp)->storeToSt(&st);

  if (!desc || !desc->account) {
    vlogf(LOG_BUG, format("Character %s has no account in saveChar()!!!  Save aborted.") %  getName());
    return;
  }
  if (!desc->account->name[0]) {
    vlogf(LOG_BUG, format("Character %s has a NULL account name! Save aborted.") %  getName());
    return;
  }
  if (!tmp) { 
    strcpy(buf2, sstring(name).lower().c_str());
    sprintf(buf, "account/%c/%s/%s", LOWER(desc->account->name[0]), sstring(desc->account->name).lower().c_str(), buf2);
  } else {
    strcpy(buf2, sstring(tmp->name).lower().c_str());
    sprintf(buf, "account/%c/%s/%s", LOWER(tmp->desc->account->name[0]), sstring(tmp->desc->account->name).lower().c_str(), buf2);
  }
  TDatabase db(DB_SNEEZY);
  Descriptor *mydesc=tmp?tmp->desc:desc;
    
  if(!isImmortal()){
    db.query("update player p, account a set p.talens=%i, p.account_id=a.account_id, p.load_room=%i, p.last_logon=%i where p.id=%i and a.name='%s'",
	     st.money, load_room, st.last_logon,
	     getPlayerID(), mydesc->account->name.c_str());
    st.load_room=0;
  }


  db.query("update playerprompt set p_type=%i, hp='%s', mana='%s', move='%s', money='%s', exp='%s', room='%s', opp='%s', tank='%s', piety='%s', lifeforce='%s', time='%s' where player_id=%i", 
	   mydesc->prompt_d.type, mydesc->prompt_d.hpColor, 
	   mydesc->prompt_d.manaColor, mydesc->prompt_d.moveColor,
	   mydesc->prompt_d.moneyColor, mydesc->prompt_d.expColor,
	   mydesc->prompt_d.roomColor, mydesc->prompt_d.oppColor,
	   mydesc->prompt_d.tankColor, mydesc->prompt_d.pietyColor,
	   mydesc->prompt_d.lifeforceColor, mydesc->prompt_d.timeColor,
	   getPlayerID());
	   

  fl = fopen(buf, "w");
  mud_assert(fl != NULL, "Failed fopen in save char: %s", buf);
  fwrite(&st, sizeof(charFile), 1, fl);
  if (fclose(fl) != 0) 
    vlogf(LOG_BUG, format("Problem closing %s's charFile") %  name);

  // Make a hard link to player directory of actual file in account
  // Directory. This is done for easy access by load_char and other
  // commands without needing to know account name.

  if (!tmp)
    sprintf(buf2, "player/%c/%s", LOWER(name[0]), sstring(name).lower().c_str());
  else
    sprintf(buf2, "player/%c/%s", LOWER(tmp->name[0]), sstring(tmp->name).lower().c_str());

  link(buf, buf2);

  // save mobile followers
  saveFollowers((load_room != Room::AUTO_RENT && load_room != Room::NOWHERE));

  // save career stats, saves info on desc, no need to use tmp
  saveCareerStats();
  saveGuildStats();
  saveDrugStats();
  saveTitle();

  // tmp which is the original character should not have a desc when it leaves
  if (tmp) 
    tmp->desc = NULL;
}


void TBeing::wipeChar(int)
{
  char abuf[80];

  if (desc) {
    sprintf(abuf, "account/%c/%s/%s",
       LOWER(desc->account->name[0]), sstring(desc->account->name).lower().c_str(), sstring(name).lower().c_str());

    if (unlink(abuf) != 0)
      vlogf(LOG_FILE, format("error in unlink (9) (%s) %d") %  abuf % errno);
  }

  removePlayerFile();
  removeRent();
  removeFollowers();

  DeleteHatreds(this, NULL);

  AccountStats::player_count--;
}


time_t lastAccountLogin(sstring name)
{
  TAccount account;

  if(!account.read(name)){
    vlogf(LOG_PEEL, format("error reading account in lastAccountLogin for %s") %
	  name);
    return time(0);
  }

  return account.last_logon;
}



void do_the_player_stuff(const char *name)
{
  char buf[128];
  char longbuf[16000];
  charFile st;
  char *tmp;
  int i,
      tLevel = -1;
  bool isGagged = false;
  FILE *fp = NULL;
  bool tPowers[MAX_POWER_INDEX];

  memset(&tPowers, 0, sizeof(tPowers));

  // skip toggle data
  if (strlen(name) > 7 && !strcmp(&name[strlen(name) - 7], ".toggle"))
    return;

  // skip career data
  if (strlen(name) > 7 && !strcmp(&name[strlen(name) - 7], ".career"))
    return;

  // skip sstrings data.
  if (strlen(name) > 8 && !strcmp(&name[strlen(name) - 8], ".strings"))
    return;

  // skip faction data
  if (strlen(name) > 6 && !strcmp(&name[strlen(name) - 8], ".faction"))
    return;

  // skip wizpowers data if there was an error up above.
  if (strlen(name) > 9 && !strcmp(&name[strlen(name) - 9], ".wizpower")) {
    char tString[256];

    sprintf(tString, "player/%c/%s", LOWER(name[0]), sstring(name).lower().c_str());

    strcpy(longbuf, sstring(name).cap().c_str());
    longbuf[(strlen(longbuf) - 9)] = '\0';

    if ((fp = fopen(tString, "r"))) {
      unsigned int tValue;

      while (fscanf(fp, "%u ", &tValue) == 1) {
        wizPowerT wpt = mapFileToWizPower(tValue);

        tPowers[wpt] = true;

        if (wpt == POWER_BUILDER)
          tLevel = 3;
        else if (wpt == POWER_GOD)
          tLevel = 2;
        else if (wpt == POWER_WIZARD)
          tLevel = 1;
        else if (wpt == POWER_IDLED && !tPowers[POWER_WIZARD]) {
          tLevel = 0;
          isGagged = true;
        }
      }

      fclose(fp);

      charFile tChar;

#if 0
      //disabled this stupid thing - dash
      if (!tPowers[POWER_WIZARD] && load_char(longbuf, &tChar)) {
        double tMIdle = (SECS_PER_REAL_DAY * 30.0);
        int tCount = 0;
        wizPowerT tWiz = MIN_POWER_INDEX;

        if (difftime(time(0), tChar.last_logon) >= tMIdle) {
          tPowers[POWER_IDLED] = true;
          isGagged = true;
          tLevel = 0;


          if ((fp = fopen(tString, "w"))) {
            for (; tWiz < MAX_POWER_INDEX; tWiz++)
              if (tPowers[tWiz]) {
                tCount++;
                fprintf(fp, "%u ", mapWizPowerToFile(tWiz));
              }

            fclose(fp);
          } else
            vlogf(LOG_FILE, format("Unable to re-open [%s] for wizlist.") %  name);
        }
      }
#endif

      if (tLevel == 1) {
        // vlogf(LOG_MISC, format("Adding Creator: %s") %  longbuf);
        tmp = new char[strlen(wiz->buf1) + strlen(longbuf) + 3];
        sprintf(tmp, "%s %s", wiz->buf1, longbuf);
        delete [] wiz->buf1;
        wiz->buf1 = tmp;
      } else if (tLevel == 2) {
        // vlogf(LOG_MISC, format("Adding God: %s") %  longbuf);
        tmp = new char[strlen(wiz->buf2) + strlen(longbuf) + 3];
        sprintf(tmp, "%s %s", wiz->buf2, longbuf);
        delete [] wiz->buf2;
        wiz->buf2 = tmp;
      } else if (tLevel == 3) {
        // vlogf(LOG_MISC, format("Adding Demigod: %s") %  longbuf);
        tmp = new char[strlen(wiz->buf3) + strlen(longbuf) + 3];
        sprintf(tmp, "%s %s", wiz->buf3, longbuf);
        delete [] wiz->buf3;
        wiz->buf3 = tmp;
      } else if (!isGagged)
        vlogf(LOG_BUG, format("%s has powerfile but doesn't have a power title.") %  longbuf);

    } else
      vlogf(LOG_FILE, format("Error opening [%s] for wizlist checking.") %  name);

    return;
  }

  AccountStats::player_count++;

  if (load_char(name, &st)) {
    byte max_level = 0;
    for (i = 0; i < MAX_SAVED_CLASSES; i++)
      max_level = max(max_level, st.level[i]);

    if (sstring(st.name).lower() != name){
      vlogf(LOG_BUG, format("%s had a corrupt st.name (%s). Moving player file.") %  st.name % name);
      handleCorrupted(name, st.aname);
      return;
    } else if (strlen(st.description) > 500) {
      vlogf(LOG_BUG, format("%s had a corrupt st.description. Moving player file.") %  name);
      handleCorrupted(name, st.aname);
      return;
    } else if (strlen(st.lastHost) > 40) {
      vlogf(LOG_BUG, format("%s had a corrupt st.lastHost. Moving player file.") %  name);
      handleCorrupted(name, st.aname);
      return;
#if 0
    } else if ((max_level <= MAX_MORT) && ((st.money < 0) || (st.money > 2500000000))) {
      vlogf(LOG_BUG, format("%s had a bad talens number(< 0 or > 25billion). Moving player file.") %  name);
      handleCorrupted(name, st.aname);
      return;
#endif
    } else if (st.f_type < MIN_FACTION || st.f_type >= MAX_FACTIONS) {
      vlogf(LOG_BUG, format("%s had a bad faction(%d). Moving player file.") %  name % st.f_type);
      handleCorrupted(name, st.aname);
      return;
    }
    if(max_level <= MAX_MORT && st.f_type != FACT_NONE) {
      char factname[8];

      if(st.f_type == FACT_BROTHERHOOD){
	strncpy(factname, "brother", 8);
      } else if (st.f_type == FACT_CULT){
	strncpy(factname, "cult", 8);
      } else if (st.f_type == FACT_SNAKE){
	strncpy(factname, "snake", 8);
      } else { // jic
	strncpy(factname, "error", 8);
      }

      TDatabase db(DB_SNEEZY);
      db.query("insert into factionmembers values ('%s', '%s', %i)",
	       st.name, factname, max_level);
    }

    // count active
    if ((time(0) - st.last_logon) <= (7 * SECS_PER_REAL_DAY))
      AccountStats::active_player7++;
    if ((time(0) - st.last_logon) <= (30 * SECS_PER_REAL_DAY))
      AccountStats::active_player30++;
    
    if (Config::AutoDeletion()){
      time_t ltime = time(0);
      tm *curtime;
      curtime = localtime(&ltime);
      time_t lastlogin=lastAccountLogin(st.aname);
      unsigned int elapsed_time = (ltime - lastlogin) / SECS_PER_REAL_DAY;

      // This gives a player at least 3 months before delete occurs
      if((time(0) - lastlogin) > (90 * SECS_PER_REAL_DAY)){
	if (!Config::RentOnlyDeletion()){
	  vlogf(LOG_MISC, format("%s (level %d) did not log in for %d days. Deleting.") %  
		name %
		max_level % elapsed_time);
	  wipePlayerFile(name);
	  sprintf(buf, "rm account/%c/%s/%s",
		  LOWER(st.aname[0]), sstring(st.aname).lower().c_str(), sstring(name).lower().c_str());
	  vsystem(buf);
	  wipeRentFile(name);
	  wipeCorpseFile(sstring(name).lower().c_str());
	  return;
	} else {
	  sprintf(buf, "rent/%c/%s", LOWER(name[0]), sstring(name).lower().c_str());
	  if ((fp = fopen(buf, "r"))) {
	    fclose(fp);
	    vlogf(LOG_MISC, format("%s (level %d) did not log in for %d days. Deleting rent.") % 
		  name % max_level % elapsed_time);
	    wipeRentFile(name);
	    wipeCorpseFile(sstring(name).lower().c_str());

	    sprintf(longbuf, "%s detected this character has been inactive for %d days.  To avoid\n\r", MUD_NAME, elapsed_time);
	    sprintf(longbuf + strlen(longbuf), "having equipment tied up on players that no longer play, players that have not\n\rconnected within a reasonable length of time have their rent files removed in\n\rorder for that equipment to go back into circulation.  Due to your inactivity,\n\ryour rent file has been wiped.  The %s administration apologizes\n\rfor any inconvenience this may cause.  Reimbursements for this eventuality\n\rare not typically granted since the item(s) in question have gone back into\n\rgeneral circulation, however an extremely basic set of equipment (newbie), and\n\rsimple adventuring supplies (lantern, food, drink) may be requested that you\n\rbe able to bootstrap your way back up the ladder.\n\r\n\rIf, however, you made arrangements prior to going inactive for things of\n\ryours to be preserved in stasis, then whatever deal was made at that time\n\rmay apply.  If such is the case, contact whichever 59+ immortal placed\n\ryour character into stasis.\n\r\n\rOn a final note, welcome back!\n\r", MUD_NAME);


	    autoMail(NULL, name, longbuf);
	  }
	}
	// rent_only deletion should fall through here
      }
    }
    if (max_level <= MAX_MORT)
      return;

    return;
  } else {
    vlogf(LOG_FILE, format("Problems loading %s player file in dirwalk of do_the_player_stuff()!") %  name);
    return;
  }
}

void fixup_players(void)
{
  FILE *fp;

  wiz = new wizListInfo();

  TDatabase db(DB_SNEEZY);
  db.query("truncate factionmembers");

 
  dirwalk("player/a", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/b", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/c", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/d", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/e", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/f", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/g", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/h", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/i", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/j", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/k", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/l", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/m", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/n", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/o", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/p", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/q", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/r", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/s", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/t", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/u", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/v", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/w", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/x", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/y", do_the_player_stuff);
  bootPulse(".", false);
  dirwalk("player/z", do_the_player_stuff);
  bootPulse(".", false);

  // make the wizlist
  if (!(fp = fopen(File::WIZLIST,"w"))) {
    vlogf(LOG_FILE,"ERROR: Error opening wizlist");
    return;
  }

        char  tString[256];
  const char *cArg;
        int   tIter = 0;

  fprintf(fp, "<p>Creators<z>:\n");
  fprintf(fp, "-----------------------\n");
  if (strlen(wiz->buf1) > 2) {
    vlogf(LOG_MISC, "Creating wizlist entry: Creator");
    cArg = one_argument(wiz->buf1, tString, cElements(tString));

    for (tIter = 0; ; cArg = one_argument(cArg, tString, cElements(tString))) {
      fprintf(fp, "%-13s", tString);

      if ((++tIter % 6) == 0)
        fprintf(fp, "\n");

      if (!*cArg)
        break;
    }

    if ((tIter % 6) != 0)
      fprintf(fp, "\n");

    fprintf(fp, "\n");
  }
  fprintf(fp, "<r>Gods<z>:\n");
  fprintf(fp, "-----------------------\n");
  if (strlen(wiz->buf2) > 2) {
    vlogf(LOG_MISC, "Creating wizlist entry: Gods");
    cArg = one_argument(wiz->buf2, tString, cElements(tString));

    for (tIter = 0; ; cArg = one_argument(cArg, tString, cElements(tString))) {
      fprintf(fp, "%-13s", tString);

      if ((++tIter % 6) == 0)
        fprintf(fp, "\n");

      if (!*cArg)
        break;
    }

    if ((tIter % 6) != 0)
      fprintf(fp, "\n");

    fprintf(fp, "\n");
  }
  fprintf(fp, "<c>Demigods<z>:\n");
  fprintf(fp, "-----------------------\n");
  if (strlen(wiz->buf3) > 2) {
    vlogf(LOG_MISC, "Creating wizlist entry: Demigods");
    cArg = one_argument(wiz->buf3, tString, cElements(tString));

    for (tIter = 0; ; cArg = one_argument(cArg, tString, cElements(tString))) {
      fprintf(fp, "%-13s", tString);

      if ((++tIter % 6) == 0)
        fprintf(fp, "\n");

      if (!*cArg)
        break;
    }

    if ((tIter % 6) != 0)
      fprintf(fp, "\n");

    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");

  if (fclose(fp)) {
    vlogf(LOG_FILE,"ERROR: Error closing wizlist");
    return;
  }
  delete wiz;
  wiz = NULL;
  bootPulse(".", false);

  dirwalk("account/a", countAccounts);
  bootPulse(".", false);
  dirwalk("account/b", countAccounts);
  bootPulse(".", false);
  dirwalk("account/c", countAccounts);
  bootPulse(".", false);
  dirwalk("account/d", countAccounts);
  bootPulse(".", false);
  dirwalk("account/e", countAccounts);
  bootPulse(".", false);
  dirwalk("account/f", countAccounts);
  bootPulse(".", false);
  dirwalk("account/g", countAccounts);
  bootPulse(".", false);
  dirwalk("account/h", countAccounts);
  bootPulse(".", false);
  dirwalk("account/i", countAccounts);
  bootPulse(".", false);
  dirwalk("account/j", countAccounts);
  bootPulse(".", false);
  dirwalk("account/k", countAccounts);
  bootPulse(".", false);
  dirwalk("account/l", countAccounts);
  bootPulse(".", false);
  dirwalk("account/m", countAccounts);
  bootPulse(".", false);
  dirwalk("account/n", countAccounts);
  bootPulse(".", false);
  dirwalk("account/o", countAccounts);
  bootPulse(".", false);
  dirwalk("account/p", countAccounts);
  bootPulse(".", false);
  dirwalk("account/q", countAccounts);
  bootPulse(".", false);
  dirwalk("account/r", countAccounts);
  bootPulse(".", false);
  dirwalk("account/s", countAccounts);
  bootPulse(".", false);
  dirwalk("account/t", countAccounts);
  bootPulse(".", false);
  dirwalk("account/u", countAccounts);
  bootPulse(".", false);
  dirwalk("account/v", countAccounts);
  bootPulse(".", false);
  dirwalk("account/w", countAccounts);
  bootPulse(".", false);
  dirwalk("account/x", countAccounts);
  bootPulse(".", false);
  dirwalk("account/y", countAccounts);
  bootPulse(".", false);
  dirwalk("account/z", countAccounts);
  bootPulse(".", false);

  bootPulse(NULL, true);

  vlogf(LOG_FILE, format("7-Day:  There are %d active players in %d active accounts.") %  AccountStats::active_player7 % AccountStats::active_account7);
  vlogf(LOG_FILE, format("30-Day: There are %d active players in %d active accounts.") %  AccountStats::active_player30 % AccountStats::active_account30);
  return;
}

void TBeing::checkForStr(silentTypeT silent)
{
  if (!silent) {
    // each doSave will cause an unequip_for_save to be done which will
    // float into here eventually for +str items.  Since we don't want these
    // fake unequips to trigger this, only activate in the affectModify() was
    // passed a parm to show the event.

    TObj *obj = dynamic_cast<TObj *>(heldInPrimHand());
    if (obj && obj->isPaired()) {
      if (checkWeaponWeight(obj, HAND_TYPE_BOTH, FALSE)) {
      } else {
        act("You lack the strength to continue to hold $p.",
            FALSE, this, obj, 0, TO_CHAR);
        act("$n lacks the strength to continue to hold $p.",
            FALSE, this, obj, 0, TO_ROOM);
        *this += *unequip(getPrimaryHold());
      }
    }  
    if (obj && !obj->isPaired()) { 
      if (checkWeaponWeight(obj, HAND_TYPE_PRIM, FALSE)) {
      } else {
        act("You lack the strength to continue to hold $p.",
            FALSE, this, obj, 0, TO_CHAR);
        act("$n lacks the strength to continue to hold $p.",
            FALSE, this, obj, 0, TO_ROOM);
        *this += *unequip(getPrimaryHold());
      }
    }
    obj = dynamic_cast<TObj *>(heldInSecHand());
    if (obj && !obj->isPaired()) {
      if (isAmbidextrous()) {
        if (checkWeaponWeight(obj, HAND_TYPE_PRIM, FALSE)) {
        } else {
          act("You lack the strength to continue to hold $p.",
              FALSE, this, obj, 0, TO_CHAR);
          act("$n lacks the strength to continue to hold $p.",
              FALSE, this, obj, 0, TO_ROOM);
          *this += *unequip(getSecondaryHold());
        }
      } else {
        if (checkWeaponWeight(obj, HAND_TYPE_SEC, FALSE)) {
        } else {
          act("You lack the strength to continue to hold $p.",
            FALSE, this, obj, 0, TO_CHAR);
          act("$n lacks the strength to continue to hold $p.",
              FALSE, this, obj, 0, TO_ROOM);
          *this += *unequip(getSecondaryHold());
        }
      }
    }
  }
}

// This function can retrieve the total number of practices spent by the character
// and reset it to 0, returning those used practices.
// Pass false for reset to just get the number of pracs into practices
// returns true if a reset is possible and practices were counted
// returns false if practices could not be reset for this character
bool TBeing::resetPractices(classIndT resetClass, int &practices, bool reset)
{
  discNumT dnt;
  practices = 0;

  if (!hasClass(1<<resetClass))
    return false;

  // loop disciplines, gather spent practices and reset if needed
  for (dnt = MIN_DISC; dnt < MAX_DISCS; dnt++)
  {
    CDiscipline *cd = getDiscipline(dnt);
    if (!cd || cd->isAutomatic())
      continue;

    // skip these disciplines which arent in the class to be reset
    if (discNames[dnt].class_num != resetClass &&
       !(cd->ok_for_class & (1<<resetClass)))
      continue;

    // get my learnedness in this disc
    int natLearn = cd->getNatLearnedness();
    int pracLearn = 0;
    if (natLearn <= 0)
      continue;

    // get expected max number of pracs
    int newPracs = 0;
    int discPracs = 0;
    if (cd->isBasic())
      discPracs = 100;
    else if (cd->isFast())
      discPracs = 20;
    else
      discPracs = 60;

    // try to get to the same level of learnedness
    while(discPracs > 0 && pracLearn < natLearn)
    {
      pracLearn += CalcRaiseDisc(pracLearn, cd, false);
      newPracs++;
      discPracs--;
    }
    practices += newPracs;

    if (!reset)
      continue;

    vlogf(LOG_MISC, format("Resetting practices for %s, disc %d, was %d%% learned, got %d pracs.") %  name % dnt % natLearn % newPracs);
    cd->setNatLearnedness(0);
    cd->setLearnedness(0);
  }

  if (practices == 0)
    return false;

  if (reset)
  {
    // have to set the doneBasic so this is reset for the player
    player.doneBasic[resetClass] = 0;
    setPracs(getPracs(resetClass) + practices, resetClass);
    resetEffectsChar();
    doSave(SILENT_YES);
  }

  return true;
}


void TBeing::doReset(sstring arg)
{
  int j, num;
  sstring buf;
  TMonster *keeper;
  TThing *tmp;
  int zone;

  arg = one_argument(arg, buf);

  if(buf.empty()){
    sendTo("What do you want to reset?\n\r");
    return;
  }

  if (!hasWizPower(POWER_RESET)) {
    sendTo("You lack the power to reset.\n\r");
    return;
  }

  if (is_abbrev(buf, "practices")) {
    TBeing *player = NULL;

    // get player name
    arg = one_argument(arg, buf);
    if (buf.empty() || !(player = get_char_vis_world(this, buf, NULL, EXACT_NO))) {
      sendTo(format("Could not find %s.\n\r") % buf);
      sendTo("Syntax:\n\r     reset practices <target>\n\r");
      return;
    }
    // reset the practices of this character
    for (classIndT resetClass = MIN_CLASS_IND; resetClass < MAX_CLASSES; resetClass++) {
      int practices = 0;
      if (player->resetPractices(resetClass, practices, true)) {
        sendTo(format("You have reset %ss %s practices.  %d practices were returned.\n\r") % player->name % classInfo[resetClass].name % practices);
        player->sendTo(format("%s has reset your %s practices.  %d practices were returned.\n\r") % name % classInfo[resetClass].name % practices);
      }
    }

  } else if (is_abbrev(buf, "gold") && isImmortal()) {

    memset(&gold_statistics, 0, sizeof(gold_statistics));
    memset(&gold_positive, 0, sizeof(gold_positive));
    sendTo("Global statistics tracking gold (info gold) reset.\n\r");
  } else if (is_abbrev(buf, "shops") && isImmortal()) {

    if(buf.lower() != "shops"){
      sendTo("You must type out the whole word <r>shops<z> to use this.\n\r");
      return;
    }
    sendTo("Resetting shops.\n\r");
    unsigned int isi;
    for (isi = 0; isi < shop_index.size(); isi++) {
      num = shop_index[isi].keeper;
      if ((keeper = dynamic_cast<TMonster *>(get_char_num(num)))) {
	for(StuffIter it=keeper->stuff.begin();it!=keeper->stuff.end();){
	  tmp=*(it++);
          delete tmp;
        }
        keeper->autoCreateShop(isi);
        keeper->saveItems(isi);
      }
    }
    sendTo("Shops reset.\n\r");
    sendTo("You may also want to do: \"purge shops\" to clear fluxuating prices.\n\r");
    return;
  } else if (is_abbrev(buf, "zone") && isImmortal()) {

    one_argument(arg, buf);
    if (buf.empty()){

      zone = (roomp ? roomp->getZoneNum() : 0);
      /*
      sendTo("Syntax: reset zone <zone#>\n\r");
      return;
      */
    }
    unsigned int i;
    if (is_abbrev(buf, "all")) {
      for (i= 0; i < zone_table.size(); i++) {
        zone_table[i].resetZone(FALSE);
      }
      sendTo(format("Zone 0-%d reset.\n\r") % (i-1));
      return;
    }
    zone = convertTo<int>(buf);
    zone_table[zone].resetZone(FALSE);
    sendTo(format("Zone %d reset.\n\r") % zone);
    return;
  } else if (is_abbrev(buf, "levels") && isImmortal()) {
    if (!hasWizPower(POWER_RESET)) {
      sendTo("You lack the power to reset.\n\r");
      return;
    }
    unsigned int i;
    for (i = 0; i < 50; i++) {
      for (j = 0; j < MAX_CLASSES; j++) {
        stats.levels[j][i] = 0;
        stats.time_levels[j][i] = 0;
      }
    }
    sendTo("Level stat information reset.\n\r");
    return;
  } else if (is_abbrev(buf, "logins") && isImmortal()) {

    time_t tnow;
    time(&tnow);
    stats.first_login = tnow;
    stats.logins = 0;
    sendTo("Login information reset.\n\r");
    return;
  } else {
      sendTo("Syntax: reset <\"practices\" | \"shops\" | \"gold\" | \"zone\" | \"logins\" | \"levels\">\n\r");
    return;
  }
}

bool TBeing::isLinkdead() const
{
  return (isPc() && !desc && polyed == POLY_TYPE_NONE);
}

void TBeing::saveTitle()
{
  TDatabase db(DB_SNEEZY);
  TPerson *tp;

  if(!(tp=dynamic_cast<TPerson *>(this)))
    return;

  db.query("update player set title='%s' where id=%i", 
	   tp->title, getPlayerID());
}

void TBeing::loadTitle()
{
  TDatabase db(DB_SNEEZY);
  TPerson *tp;

  if(!(tp=dynamic_cast<TPerson *>(this)))
    return;

  db.query("select title from player where id=%i",
	   getPlayerID());
  if(db.fetchRow()){
    delete [] tp->title;
    tp->title = mud_str_dup(db["title"]);
  }
}


void TBeing::saveDrugStats()
{
  TDatabase db(DB_SNEEZY);

  if (!isPc() || !desc)
    return;


  db.query("delete from drug_use where player_id=%i", getPlayerID());
  
  for(int i=MIN_DRUG;i<MAX_DRUG;++i){
    if(desc->drugs[i].total_consumed){
      db.query("insert into drug_use values (%i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i)",
	       i, getPlayerID(), desc->drugs[i].first_use.seconds,
	       desc->drugs[i].first_use.minutes, desc->drugs[i].first_use.hours,
	       desc->drugs[i].first_use.day, desc->drugs[i].first_use.month,
	       desc->drugs[i].first_use.year, desc->drugs[i].last_use.seconds,
	       desc->drugs[i].last_use.minutes, desc->drugs[i].last_use.hours,
	       desc->drugs[i].last_use.day, desc->drugs[i].last_use.month,
	       desc->drugs[i].last_use.year, desc->drugs[i].total_consumed,
	       desc->drugs[i].current_consumed);
    }
  }
}



void TBeing::loadDrugStats()
{
  TDatabase db(DB_SNEEZY);
  int i;

  if (!isPc() || !desc)
    return;

  for(i=0;i<MAX_DRUG;++i){
    desc->drugs[i].first_use.seconds=0;
    desc->drugs[i].first_use.minutes=0;
    desc->drugs[i].first_use.hours=0;
    desc->drugs[i].first_use.day=0;
    desc->drugs[i].first_use.month=0;
    desc->drugs[i].first_use.year=0;
    desc->drugs[i].last_use.seconds=0;
    desc->drugs[i].last_use.minutes=0;
    desc->drugs[i].last_use.hours=0;
    desc->drugs[i].last_use.day=0;
    desc->drugs[i].last_use.month=0;
    desc->drugs[i].last_use.year=0;
    desc->drugs[i].total_consumed=0;
    desc->drugs[i].current_consumed=0;
  }


  db.query("select drug_id, first_use_sec,first_use_min,first_use_hour,first_use_day,first_use_mon,first_use_year,last_use_sec,last_use_min,last_use_hour,last_use_day,last_use_mon,last_use_year,total_consumed,current_consumed from drug_use where player_id=%i", getPlayerID());

  while(db.fetchRow()){
    i=convertTo<int>(db["drug_id"]);
    desc->drugs[i].first_use.seconds=convertTo<int>(db["first_use_sec"]);
    desc->drugs[i].first_use.minutes=convertTo<int>(db["first_use_min"]);
    desc->drugs[i].first_use.hours=convertTo<int>(db["first_use_hour"]);
    desc->drugs[i].first_use.day=convertTo<int>(db["first_use_day"]);
    desc->drugs[i].first_use.month=convertTo<int>(db["first_use_mon"]);
    desc->drugs[i].first_use.year=convertTo<int>(db["first_use_year"]);
    desc->drugs[i].last_use.seconds=convertTo<int>(db["last_use_sec"]);
    desc->drugs[i].last_use.minutes=convertTo<int>(db["last_use_min"]);
    desc->drugs[i].last_use.hours=convertTo<int>(db["last_use_hour"]);
    desc->drugs[i].last_use.day=convertTo<int>(db["last_use_day"]);
    desc->drugs[i].last_use.month=convertTo<int>(db["last_use_mon"]);
    desc->drugs[i].last_use.year=convertTo<int>(db["last_use_year"]);
    desc->drugs[i].total_consumed=convertTo<int>(db["total_consumed"]);
    desc->drugs[i].current_consumed=convertTo<int>(db["current_consumed"]);
  }
}


void TBeing::saveCareerStats()
{
  FILE *fp;
  char buf[160];
  int current_version = 20;
// version 9  : 7/3/98
// version 10 : 8/17/98
// version 11 : 10/06/98
// version 12 : 10/20/98
// version 13 : 10/21/98
// version 14 : 11/16/98
// version 15 : 12/04/98
// version 16 : 12/13/98
// version 17 : 02/15/00
  // version 18 : opening of game version 5.2
  // version 19: 11/04/03
  int i;

  if (!isPc() || !desc)
    return;

  sprintf(buf, "player/%c/%s.career", LOWER(name[0]), sstring(name).lower().c_str());

  if (!(fp = fopen(buf, "w"))) {
    vlogf(LOG_BUG, format("Unable to open file (%s) for saving career stats. (%d)") %  buf % errno);
    return;
  }

  fprintf(fp, "%u\n", 
      current_version);
  fprintf(fp, "%u %u\n", 
      desc->career.kills,
      desc->career.group_kills);
  fprintf(fp, "%u %f\n", 
      desc->career.deaths,
      desc->career.exp);
  fprintf(fp, "%u %u %u\n", 
      desc->career.crit_hits,
      desc->career.crit_misses,
      desc->career.crit_kills);
  fprintf(fp, "%u %u\n", 
      desc->career.crit_hits_suff,
      desc->career.crit_kills_suff);
  fprintf(fp, "%u %u %u %u %u\n", 
      desc->career.crit_beheads,
      desc->career.crit_sev_limbs,
      desc->career.crit_cranial_pierce,
      desc->career.crit_broken_bones,
      desc->career.crit_crushed_skull);
  fprintf(fp, "%u %u %u %u %u\n", 
      desc->career.crit_beheads_suff,
      desc->career.crit_sev_limbs_suff,
      desc->career.crit_cranial_pierce_suff,
      desc->career.crit_broken_bones_suff,
      desc->career.crit_crushed_skull_suff);
  fprintf(fp, "%u %u %u %u\n", 
      desc->career.crit_cleave_two,
      desc->career.crit_cleave_two_suff,
      desc->career.crit_disembowel,
      desc->career.crit_disembowel_suff);
  fprintf(fp, "%u %u\n", 
      desc->career.crit_crushed_nerve,
      desc->career.crit_crushed_nerve_suff);
  fprintf(fp, "%u %u %u %u\n", 
      desc->career.crit_voice,
      desc->career.crit_voice_suff,
      desc->career.crit_eye_pop,
      desc->career.crit_eye_pop_suff);
  fprintf(fp, "%u %u %u %u\n", 
      desc->career.crit_lung_punct,
      desc->career.crit_lung_punct_suff,
      desc->career.crit_impale,
      desc->career.crit_impale_suff);
  fprintf(fp, "%u %u %u %u\n", 
      desc->career.crit_eviscerate,
      desc->career.crit_eviscerate_suff,
      desc->career.crit_kidney,
      desc->career.crit_kidney_suff);
  fprintf(fp, "%u %u\n",
      desc->career.crit_genitalia,
      desc->career.crit_genitalia_suff);
  fprintf(fp, "%u %u\n", 
      desc->career.arena_victs,
      desc->career.arena_loss);
  fprintf(fp, "%u %u\n", 
      desc->career.skill_success_attempts,
      desc->career.skill_success_pass);
  fprintf(fp, "%u %u %u %u\n", 
      desc->career.spell_success_attempts,
      desc->career.spell_success_pass,
      desc->career.prayer_success_attempts,
      desc->career.prayer_success_pass);
  for (i = 0; i < MAX_ATTACK_MODE_TYPE; i++) {
    fprintf(fp, "%u %u %u %u\n",
        desc->career.hits[i],
        desc->career.swings[i],
        desc->career.dam_done[i],
        desc->career.dam_received[i]);
  }
  fprintf(fp, "%u %u\n", 
      desc->career.pets_bought,
      desc->career.pet_levels_bought);
  fprintf(fp, "%lu %lu\n", 
	  (unsigned long) desc->career.hit_level40,
	  (unsigned long) desc->career.hit_level50);
  fprintf(fp, "%u %u\n",
      desc->career.stuck_in_foot,
      desc->career.ounces_of_blood);
  fprintf(fp, "%u %u\n",
      desc->career.crit_tooth,
      desc->career.crit_tooth_suff);
  fprintf(fp, "%u %u\n",
	  desc->career.crit_ripped_out_heart,
	  desc->career.crit_ripped_out_heart_suff);

  if (fclose(fp)) 
      vlogf(LOG_BUG, format("Problem closing %s's saveCareerStats") %  name);
  
}


void TBeing::loadCareerStats()
{
  FILE *fp = NULL;
  char buf[160];
  int current_version;
  unsigned int num1, num2, num3, num4, num5;
  long unsigned int lu_num1, lu_num2;
  float f_num1;
  int i;

  if (!isPc() || !desc)
    return;

  sprintf(buf, "player/%c/%s.career", LOWER(name[0]), sstring(name).lower().c_str());

  // lets just set some common values first
  desc->career.setToZero();

  if (!(fp = fopen(buf, "r"))) {
    // file may not exist
    return;
  }

  if (fscanf(fp, "%d\n", 
      &current_version) != 1) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  if (current_version <= 9) {
    // we've done a player wipe so this should never happen
    vlogf(LOG_BUG, format("Bad data for cur vers(%s)") %  getName());
    fclose(fp);
    return;
  }

  if (fscanf(fp, "%u %u\n", 
      &num1, &num2) != 2) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.kills = num1;
  desc->career.group_kills = num2;

if (current_version < 15) {
  if (fscanf(fp, "%u %lu\n", 
      &num1, &lu_num2) != 2) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.deaths = num1;
  desc->career.exp = 0;
} else {
  if (fscanf(fp, "%u %f\n", 
      &num1, &f_num1) != 2) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.deaths = num1;
  desc->career.exp = f_num1;
}
if (current_version < 16) {
  desc->career.exp = 0;
}

  if (fscanf(fp, "%u %u %u\n", 
      &num1, &num2, &num3) != 3) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.crit_hits = num1;
  desc->career.crit_misses = num2;
  desc->career.crit_kills = num3;

  if (fscanf(fp, "%u %u\n", 
      &num1, &num2) != 2) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.crit_hits_suff = num1;
  desc->career.crit_kills_suff = num2;

  if (fscanf(fp, "%u %u %u %u %u\n", 
      &num1, &num2, &num3, &num4, &num5) != 5) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.crit_beheads = num1;
  desc->career.crit_sev_limbs = num2;
  desc->career.crit_cranial_pierce = num3;
  desc->career.crit_broken_bones = num4;
  desc->career.crit_crushed_skull = num5;

  if (fscanf(fp, "%u %u %u %u %u\n", 
      &num1, &num2, &num3, &num4, &num5) != 5) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.crit_beheads_suff = num1;
  desc->career.crit_sev_limbs_suff = num2;
  desc->career.crit_cranial_pierce_suff = num3;
  desc->career.crit_broken_bones_suff = num4;
  desc->career.crit_crushed_skull_suff = num5;

  if (fscanf(fp, "%u %u %u %u\n", 
      &num1, &num2, &num3, &num4) != 4) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.crit_cleave_two = num1;
  desc->career.crit_cleave_two_suff = num2;
  desc->career.crit_disembowel = num3;
  desc->career.crit_disembowel_suff = num4;

  if (fscanf(fp, "%u %u\n", 
      &num1, &num2) != 2) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.crit_crushed_nerve = num1;
  desc->career.crit_crushed_nerve_suff = num2;

  if (fscanf(fp, "%u %u %u %u\n", 
      &num1, &num2, &num3, &num4) != 4) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.crit_voice = num1;
  desc->career.crit_voice_suff = num2;
  desc->career.crit_eye_pop = num3;
  desc->career.crit_eye_pop_suff = num4;

  if (fscanf(fp, "%u %u %u %u\n", 
      &num1, &num2, &num3, &num4) != 4) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.crit_lung_punct = num1;
  desc->career.crit_lung_punct_suff = num2;
  desc->career.crit_impale = num3;
  desc->career.crit_impale_suff = num4;

  if (current_version >= 14) {
    if (fscanf(fp, "%u %u %u %u\n", 
	       &num1, &num2, &num3, &num4) != 4) {
      vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
      fclose(fp);
      return;
    }
    desc->career.crit_eviscerate = num1;
    desc->career.crit_eviscerate_suff = num2;
    desc->career.crit_kidney = num3;
    desc->career.crit_kidney_suff = num4;
  }
  
  if (current_version >= 17) {
    if (fscanf(fp, "%u %u\n",
	       &num1, &num2) !=2){
      vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
      fclose(fp);
      return;
    }
    desc->career.crit_genitalia = num1;
    desc->career.crit_genitalia_suff = num2;    
  }

  if (fscanf(fp, "%u %u\n", 
      &num1, &num2) != 2) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.arena_victs = num1;
  desc->career.arena_loss = num2;

  if (fscanf(fp, "%u %u\n", 
      &num1, &num2) != 2) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.skill_success_attempts = num1;
  desc->career.skill_success_pass = num2;

  if (fscanf(fp, "%u %u %u %u\n", 
      &num1, &num2, &num3, &num4) != 4) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.spell_success_attempts = num1;
  desc->career.spell_success_pass = num2;
  desc->career.prayer_success_attempts = num3;
  desc->career.prayer_success_pass = num4;

  for (i = 0; i < MAX_ATTACK_MODE_TYPE; i++) {
    if (fscanf(fp, "%u %u %u %u\n", 
        &num1, &num2, &num3, &num4) != 4) {
      vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
      fclose(fp);
      return;
    }
    desc->career.hits[i] = num1;
    desc->career.swings[i] = num2;
    desc->career.dam_done[i] = num3;
    desc->career.dam_received[i] = num4;
  }

  if (fscanf(fp, "%u %u\n", 
      &num1, &num2) != 2) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.pets_bought = num1;
  desc->career.pet_levels_bought = num2;

if (current_version >= 11) {
  if (fscanf(fp, "%lu %lu\n", 
      &lu_num1, &lu_num2) != 2) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.hit_level40 = lu_num1;
  desc->career.hit_level50 = lu_num2;
}
  
if (current_version >= 12) {
  if (fscanf(fp, "%u %u\n", &num1, &num2) != 2) {
    vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  desc->career.stuck_in_foot = num1;
  desc->career.ounces_of_blood = num2;
}

if (current_version < 13) {
  // set to zero forgot to init these, so all vers 13 does is correct
  desc->career.stuck_in_foot = 0;
  desc->career.ounces_of_blood = 0;
}

 if (current_version >= 18){
   if(fscanf(fp, "%u %u\n", &num1, &num2) != 2){
     vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
     fclose(fp);
     return;
   }
   desc->career.crit_tooth=num1;
   desc->career.crit_tooth_suff=num2;
 }

 if(current_version == 19){
   if(fscanf(fp, "%u %u\n", &num1, &num2) != 2){
     vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
     fclose(fp);
     return;
   }
   desc->career.crit_ripped_out_heart=0;
   desc->career.crit_ripped_out_heart_suff=0;
 }

 if(current_version >= 20){
   if(fscanf(fp, "%u %u\n", &num1, &num2) != 2){
     vlogf(LOG_BUG, format("Bad data in career stat read (%s)") %  getName());
     fclose(fp);
     return;
   }
   desc->career.crit_ripped_out_heart=num1;
   desc->career.crit_ripped_out_heart_suff=num2;
 }


  fclose(fp);
}

std::vector<sstring> listAccountCharacters(sstring name)
{
  std::vector<sstring> list;

  sstring fileName = "account/";
  fileName += LOWER(name[0]);
  fileName += "/";
  fileName +=  name.lower();

  DIR *dfd;
  struct dirent *dp;

  if (!(dfd = opendir(fileName.c_str()))) {
    vlogf(LOG_FILE, format("Unable to walk directory for character listing (%s account)") %  name);
    return list;
  }

  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, "account") || !strcmp(dp->d_name, "comment") ||
        !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;
    list.push_back(sstring(dp->d_name));
  }
  closedir(dfd);
  return list;
}

int listAccount(sstring name, sstring &buf)
{
  std::vector<sstring> chars = listAccountCharacters(name);
  int count = 0;

  buf += "The following characters are in the ";
  buf += name;
  buf += " account.\n\r";
  buf += "--------------------------------------------------\n\r";
  for(unsigned int iChar = 0; iChar < chars.size(); iChar++) {
    charFile st;

    load_char(chars[iChar], &st);
    byte max_level = 0;
    for (classIndT iClass = MIN_CLASS_IND; iClass < MAX_SAVED_CLASSES; iClass++)
      max_level = max(max_level, st.level[iClass]);

    time_t ct = st.last_logon;
    char * tmstr = (char *) asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    
    buf += format("%d) %s (L%d) %s\n\r") % ++count % chars[iChar].c_str() % int(max_level) % tmstr;
  }
  return count;
}

int numFifties(race_t race, bool perma, sstring account_name)
{
  int num, num_fifties = 0;
  charFile st;
  FILE *fp;
  DIR *dfd;
  struct dirent *dp;
  sstring account_path = format("account/%c/%s") % LOWER(account_name[0]) %
          account_name.lower();
  sstring togfile_name;
  char tog_file_name[128];
  bool char_is_perma;

  if (!(dfd = opendir(account_path.c_str()))) {
    vlogf(LOG_BUG, format("Unable to walk directory in numFifties (%s account)") 
        % account_name);
    return 0;
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, "account") || !strcmp(dp->d_name, "comment") ||
        !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;

    charFile st;

    load_char(dp->d_name, &st);
    byte max_level = 0;

    // perma death characters only get the bonus from perma 50's
    sprintf(tog_file_name, "player/%c/%s.toggle", LOWER(dp->d_name[0]),
      dp->d_name);
    if(!(fp = fopen(tog_file_name, "r"))) {
      vlogf(LOG_MAROR, format("Error loading toggles for player %s in numFifties.")
        % dp->d_name);
      continue;
    }

    char_is_perma = FALSE;
    while (fscanf(fp, "%d ", &num) == 1) {
      if (num == TOG_PERMA_DEATH_CHAR) {
        char_is_perma = TRUE;
        break;
      }
    }
    fclose(fp);
    
    if (perma && !char_is_perma)
     continue; 
    // end of permadeath section
    
    for (classIndT i = MIN_CLASS_IND; i < MAX_SAVED_CLASSES; i++)
      max_level = max(max_level, st.level[i]);

    if (max_level == 50 && (race == RACE_NORACE || st.race == race))
      num_fifties += 1;
  }
  closedir(dfd);

  return num_fifties;
}

