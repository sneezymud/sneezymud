extern "C" {
#include <unistd.h>
}
#include <cmath>

#include "stdsneezy.h"
#include "statistics.h"
#include "games.h"
#include "obj_base_container.h"
#include "obj_food.h"
#include "obj_bed.h"
#include "obj_drinkcon.h"
#include "obj_opal.h"

#if 0
static const sstring ClassTitles(const TBeing *ch)
{
  int count = 0;
  classIndT i;
  char buf[256];

  for (i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
    if (ch->getLevel(i)) {
      if ((++count) > 1)
        sprintf(buf + strlen(buf), "/Level %d %s", 
                ch->getLevel(i), classInfo[i].name.cap().c_str());
      else
        sprintf(buf, "Level %d %s",
                ch->getLevel(i), classInfo[i].name.cap().c_str());
    }
  }
  return (buf);
}
#endif

int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{
  // age disabled - treat everyone as a 35 year old
    return (int) (p2 + (((35 - 30) * (p3 - p2)) / 15));        /* 30..44 */
    
  if (age < 15)
    return (p0);
  else if (age <= 29)
    return (int) (p1 + (((age - 15) * (p2 - p1)) / 15));        /* 15..29 */
  else if (age <= 44)
    return (int) (p2 + (((age - 30) * (p3 - p2)) / 15));        /* 30..44 */
  else if (age <= 59)
    return (int) (p3 + (((age - 45) * (p4 - p3)) / 15));        /* 45..59 */
  else if (age <= 79)
    return (int) (p4 + (((age - 60) * (p5 - p4)) / 20));        /* 60..79 */
  else
    return (p6);
}

void TPerson::setMaxHit(int newhit)
{
  // TPerson hit points are dynamic now, so this shouldn't happen
  // unless an immortal uses @set
  vlogf(LOG_BUG, fmt("TPerson::setMaxHit() got called on %s") %  getName());
  points.maxHit = newhit;  
}

int eqHpBonus(const TPerson *ch)
{
  wearSlotT i;
  int j, total=0;
  TObj *o;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    for(j=0;j<MAX_OBJ_AFFECT;++j){
      if(ch->equipment[i] &&
	 (o=dynamic_cast<TObj *>(ch->equipment[i])) &&
	 o->affected[j].location == APPLY_HIT){
	total+=o->affected[j].modifier;
      }
    }
  }
  return total;
}

int baseHp()
{
  return 21;
}

float classHpPerLevel(const TPerson *tp){
 float hpgain=0;


 for(int i=0;i<MAX_CLASSES;++i){
   if(tp->hasClass(classInfo[i].class_num)){
     hpgain = classInfo[i].hp_per_level;
     break;
   }
 }

 if(!hpgain){
    vlogf(LOG_BUG, fmt("No class in classHpPerLevel() for %s") %  tp->getName());
    hpgain=7.0;
 } 

 return hpgain;
}

int ageHpMod(const TPerson *tp){
    // age disabled - graf modified to always return age 30-44
  if (!tp) return 0;
  return graf((tp->age()->year - tp->getBaseAge() + 15), 2, 4, 17, 14, 8, 4, 3);
  
}


int affectHpBonus(const TPerson *tp){
  affectedData *aff;
  int total=0;

  for (aff = tp->affected; aff; aff = aff->next) {
    if(aff->location==APPLY_HIT)
      total+=aff->modifier;
  }
  
  return total;
}

short int TPerson::hitLimit() const
{
  if(isImmortal())
    return points.maxHit;


  float defense_amt=((35.0/100.0) * (float) getSkillValue(SKILL_DEFENSE)) +
             ((15.0/100.0) * (float) getSkillValue(SKILL_ADVANCED_DEFENSE));

  float newmax=0;
  newmax += baseHp() + ageHpMod(this);
  newmax += (classHpPerLevel(this) * defense_amt);
  newmax *= getConHpModifier();
  newmax += eqHpBonus(this);
  newmax += affectHpBonus(this);

  return (short int) newmax;
}

short int TPerson::manaLimit() const
{
  int iMax = 100;

  if (hasClass(CLASS_MAGE)){
    iMax += getSkillValue(SKILL_MANA) * 3;

    //    iMax += getLevel(MAGE_LEVEL_IND) * 6;
  } else if (hasClass(CLASS_RANGER))
    iMax += GetMaxLevel() * 3;
  else if (hasClass(CLASS_MONK))
    iMax += GetMaxLevel() * 3;

  if(hasQuestBit(TOG_PSIONICIST))
    iMax += getDiscipline(DISC_PSIONICS)->getLearnedness();

  TOpal *stone = find_biggest_powerstone(this);
  if(stone)
    iMax += stone->psGetMaxMana();

  iMax += points.maxMana;        /* bonus mana */

  return (iMax);
}

int TPerson::getMaxMove() const
{
  // age disabled
  return 100 + 15 + GetTotLevel() +
      plotStat(STAT_CURRENT, STAT_CON, 3, 18, 13);
  return 100 + age()->year - getBaseAge() + 15 + GetTotLevel() +
      plotStat(STAT_CURRENT, STAT_CON, 3, 18, 13);
}

short int TBeing::moveLimit() const
{
  int iMax = getMaxMove() + race->getMoveMod();

  if(discs && doesKnowSkill(SKILL_IRON_LEGS)){
    iMax += getSkillValue(SKILL_IRON_LEGS)*2;
  }

  iMax += points.maxMove;        /* move bonus */

  return (iMax);
}

double TBeing::pietyGain(double modif)
{
  // modif is disc_piety to simulate being of higher percentage then really am
  double gain;

  if (!isPc())
    return 0.0;

  if (getPiety() == 100.0)
    return 0.0;

  if (fight() || spelltask)
    return 0.0;

  if (!hasClass(CLASS_CLERIC) && !hasClass(CLASS_DEIKHAN))
    return 0.5;  // slow regen if they somehow lost it

#if FACTIONS_IN_USE
  double ppx;  // my factions power as a percent of the avg faction power 
  if (isUnaff())
    ppx = 50.0;  // flat regen
  else if (avg_faction_power)
    ppx = (FactionInfo[getFaction()].faction_power) / avg_faction_power * 100.;
  else if (FactionInfo[getFaction()].faction_power)
    ppx = 100.0;
  else
    ppx = 0.0;

  // no healing if weak member of weak faction
  if (((ppx + (getPerc()/5.0) + modif) < 100.0) && !isUnaff()) {
    // allow weak healing
    gain = (1.0 - (GetMaxLevel() * 0.01));
  } else {
    gain = ppx / 100.;
    gain *= (getPerc() + 100.0 + modif)/200.;
    // newbies will essentially get 3/4 of the below
    // at 20.0 : Average piety regen           : 8.14  (attempts : 531)
    // at 30.0 : Average piety regen           : 12.90 (attempts : 2091)
    // at L50, cleric has 100 piety, think we desire regen of 5
    // since this makes it REAL slow at low level, go with 10 or so instead
    gain *= 20.0;  // arbitrary
  }

  // lower this to make faction power drain quicker
  // 50.0 allows slow/constant growth
  // 40.0 allows slow/constant decrease
  double GAIN_DIVISOR = 45.0;

  if (GetMaxLevel() <= MAX_MORT) {
    if (!isUnaff()) {
      if ((gain/GAIN_DIVISOR) > FactionInfo[getFaction()].faction_power)
        gain = GAIN_DIVISOR * FactionInfo[getFaction()].faction_power;

      FactionInfo[getFaction()].faction_power -= gain/GAIN_DIVISOR;
      desc->session.perc -= gain/GAIN_DIVISOR;

      if (FactionInfo[getFaction()].faction_power < 0.0000)
        FactionInfo[getFaction()].faction_power = 0.0000;
    } else {
      // unaff's will drain from from highest powered faction
      int iFact = bestFactionPower();
  
      if ((gain/GAIN_DIVISOR) > FactionInfo[iFact].faction_power)
        gain = GAIN_DIVISOR * FactionInfo[iFact].faction_power;

      FactionInfo[iFact].faction_power -= gain/GAIN_DIVISOR;

      if (FactionInfo[iFact].faction_power < 0.0000)
        FactionInfo[iFact].faction_power = 0.0000;
    }
  }
#else
//  ppx = 50.0;
//  gain = ppx / 100.;
//  gain *= (50.0 + 100.0 + modif)/200.;
//  gain *= 25.0;  // arbitrary
  // simplify math
  gain = (150.0 + modif)/18.0;
#endif

  if (dynamic_cast<TPerson *>(this)) {
    stats.piety_gained_attempts++;
    stats.piety_gained += gain;
  }
//  if (affectedBySpell(SPELL_ENLIVEN))
//    gain *= 2;

  // limit the gain so we don't go over max
  if ((getPiety() + gain) > 100.0)
    gain = 100. - getPiety();
      
  return gain;
}

int TMonster::manaGain()
{
  int gain;

  if (fight() || spelltask)
    return 0;

  gain = 1 + (2 * (GetTotLevel() / 9));
  gain += race->getManaMod();

  if (!getCond(FULL) || !getCond(THIRST))
    gain >>= 2;
//  if (affectedBySpell(SPELL_ENLIVEN))
//    gain *= 2;
  return (gain);
}

int TPerson::manaGain()
{
  int gain;

  if (fight() || spelltask)
    return 0;

  if (!desc)
    return 0;

    // age disabled - graf modified to always return age 30-44
  gain = graf((age()->year - getBaseAge() + 15), 2, 4, 6, 8, 10, 12, 14);

  // arbitrary multiplier
  // at 1.0 : Average mana regen            :  5.69  (attempts : 15624)
  // at 2.0 : Average mana regen            : 11.11  (attempts : 30584)
  // at L50, mage has 400 mana, think we want 20
  gain *= 4;

  if (hasClass(CLASS_MAGE))
    gain += gain;

  gain += race->getManaMod();

  if (!getCond(FULL) || !getCond(THIRST))
    gain >>= 2;
//  if (affectedBySpell(SPELL_ENLIVEN))
//    gain *= 2;

  stats.mana_gained_attempts++;
  stats.mana_gained += gain;

  return (gain);
}

int TMonster::hitGain()
{
  int gain;
  int num;

  // ok well. here's the deal, as of late March 2001, players are able to abuse
  // the slowness of mob healing to run and hide, then heal much faster than
  // the mob they were fighting. I'm going to roughly linearize the monster HP
  // gain formula to heal as fast (in terms of %hp) as PCs

  // evaluating the TPerson version of this function shows that:
  // a level 50 warrior should get about 1/20 hp a regen,
  // and a level 1 warrior should get about 8/20 hp a level
  // thus, we get... (.4-.35*(level/50.0))*hitLimit()
  // i'm also gonna cap level at 50 so that there is no way we can get negative values

  int oldgain = max(2, (GetMaxLevel() / 2));
  // the old version
  
  // base1 is the % for level 1, base50 is the percent for level 50, for easy adjusting

  double base1 = 0.125; // yeash complain complain making this much lower etc
  double base50 = 0.05;


  double level = (double)(min(50, (int)GetMaxLevel()));

  gain = (int)(( base1 - (base1-base50) * (level/50.0)) * (double)hitLimit());

  // just because i'm an asshole, i'm going to make sure this new value wont be lower
  // than the old value

  gain = max(oldgain,gain);
  
  if (fight())
    gain = 0;

  if (affectedBySpell(SKILL_CUDGEL))
    gain = oldgain;


  TBed * tb = dynamic_cast<TBed *>(riding);
  if (tb)
    tb->bedRegen(this, &gain, SILENT_NO);

  //gain += race->getHpMod(); This was meant for leveling.

  if (getCond(DRUNK) > 0)
    gain += 1 + (getCond(DRUNK) / 3);
  if (affectedBySpell(SPELL_ENLIVEN))
    gain *= 2;

  if (roomp && roomp->isRoomFlag(ROOM_HOSPITAL))
    gain *= 2;

  if ((num = inCamp())) {
    gain += (gain * num / 100);
  }

  return (gain);
}

int TPerson::hitGain()
{
  int gain;
  int num;

  if (hasClass(CLASS_SHAMAN)) {
    if (0 >= getLifeforce()) {
      return 0;
    }
  }

  if (!desc)
    return 0;

  if (fight())
    gain = 0;
  else {
    // age disabled - graf modified to always return age 30-44
    gain = graf((age()->year - getBaseAge() + 15), 2, 4, 5, 9, 4, 3, 2);
    gain += 4;
    gain = (int)((double)(gain)*plotStat(STAT_CURRENT,STAT_CON,.80,1.25,1.00));
  }

  // arbitrary multiplier
  // at 1.0 : Average HP regen              :  9.69  (attempts : 15487)
  // at 1.5 : Average HP regen              : 14.15  (attempts : 30465)
  // at 2.0 : Average HP regen              : 19.59  (attempts : 15844)
  // at 2.5 : Average HP regen              : 23.89  (attempts : 86643)
  // L50 warrior has 500 HP, desired regen of 25
  // L1 warrior has 25 HP, regen of 10
  gain *= max(20, (int) GetMaxLevel());
  gain /= 20;

  TBed * tb = dynamic_cast<TBed *>(riding);
  if (tb)
    tb->bedRegen(this, &gain, SILENT_NO);

  //gain += race->getHpMod(); This was meant for leveling.

  if (getCond(DRUNK) > 0)
    gain += 1 + (getCond(DRUNK) / 3);

  if (roomp && roomp->isRoomFlag(ROOM_HOSPITAL))
    gain *= 2;

  if ((num = inCamp())) {
    gain += (gain * num / 100);
  }
  if (affectedBySpell(SPELL_ENLIVEN))
    gain *= 2;

  stats.hit_gained_attempts++;
  stats.hit_gained += gain;
  return (gain);
}

int TBeing::moveGain()
{
  int gain, num;

  gain = plotStat(STAT_CURRENT, STAT_CON, 11, 41, 30);

  TBed * tb = dynamic_cast<TBed *>(riding);
  if (tb)
    tb->bedRegen(this, &gain, SILENT_YES);

  // moveMod is used to calc moveLimit().  don't use it

  if (isAffected(AFF_POISON))
    gain >>= 2;	// rightshift by 2 is div by 4

  if (isAffected(AFF_SYPHILIS))
    gain >>= 2;	// rightshift by 2 is div by 4

  if (!getCond(FULL) || !getCond(THIRST))
    gain >>= 2;	// rightshift by 2 is div by 4

  if ((num = inCamp())) {
    gain += (gain * num / 100);
  }
  if (affectedBySpell(SPELL_ENLIVEN))
    gain *= 2;

  if (roomp && roomp->isRoomFlag(ROOM_NO_HEAL))
    gain /= 3;
  if (roomp && roomp->isRoomFlag(ROOM_HOSPITAL))
    gain *= 2;

  gain = max(gain,1);

  if (dynamic_cast<TPerson *>(this)) {
    stats.move_gained_attempts++;
    stats.move_gained += gain;
  }

  return ((gain * 4) / 3);
}


// find out how many pracs ch needs to max all advanced discs
int getAdvancedPracs(TBeing *ch){
  discNumT i;
  CDiscipline *cd;
  int totalpracs=0;

  for (i=MIN_DISC; i < MAX_DISCS; i++) {
    cd = ch->getDiscipline(i);
    if(cd && cd->ok_for_class){
      if(cd->isBasic() || cd->isAutomatic())
	totalpracs+=0;
      else if(cd->isFast()){
	totalpracs+=20;
      } else {
	totalpracs+=60;
      }
    }
  }
  return totalpracs;
}

double TBeing::pracsPerLevel(classIndT Class, bool forceBasic)
{
  double num;

  // class_tweak is what percentage of advanced discs they should be able to
  // learn.  0.50 would let them learn half of their advanced discs, or 50%
  // in each advanced disc.  This includes "fast" discs like weapon specs.
  double class_tweak=classInfo[Class].prac_tweak;
  
  // what level player should finish basic at (200 pracs)
  double avbasic = 30.0; 

  // modify the level by the int mod, smart people finish sooner
  double advancedlevel = avbasic / getIntModForPracs();

  // this is how many pracs to give per level to hit 200 at advancedlevel
  double basicpracs = (200.0/advancedlevel);

  // the percentage of advanced discs this person can learn
  double learnrate = getIntModForPracs() * class_tweak;

  // advancedpracs is the pracs per level you get after you finish basic
  double advancedpracs = (getAdvancedPracs(this) * learnrate)/(50.0 - advancedlevel);

  if (getLevel(Class) >= advancedlevel && !forceBasic && getLevel(Class)<=50)
    num = advancedpracs;
  else
    num = basicpracs;

  return num;
}

sh_int TBeing::calcNewPracs(classIndT Class, bool forceBasic)
{
  sh_int prac;
  double num;

  num = pracsPerLevel(Class, forceBasic);

  float temp = (int) num; // save for logging
  prac = (int) num;
  num = num - prac;

  if (isTripleClass()) {
    prac *= 4;
    prac /= 6;
    num *= 4;
    num /= 6;
  } else if (isDoubleClass()) {
    prac *= 3;
    prac /= 4;
    num *= 3;
    num /= 4;
  } 

  int roll = ::number(1,99);
  if ((100.0*num) >= roll) {
    prac++;
  }
  
  if(isPc()) {
    vlogf(LOG_DASH, fmt("%s gaining %d pracs roll = %d (%d + %4.2f) lev: %d") %  getName() %
	  prac % roll % (int)temp % num % getLevel(Class));
  }

  return prac;
}

void TBeing::setPracs(sh_int prac, classIndT Class)
{
  if(Class >= MAX_CLASSES)
    vlogf(LOG_BUG, "Bad class in setPracs");
  else
    practices.prac[Class]=prac;
}

sh_int TBeing::getPracs(classIndT Class)
{
  if(Class >= MAX_CLASSES)
    vlogf(LOG_BUG, "Bad class in getPracs");
  else
    return practices.prac[Class];

  return 0;
}

void TBeing::addPracs(sh_int pracs, classIndT Class)
{
  setPracs(getPracs(Class) + pracs, Class);
}

/* I redid this function to allow slight differences in slight amounts of */
/* player intel and wisdom. Before the changes, there was no difference in*/
/* a 3 wis/int and a 10 wis/int, and therefore people just lowered those  */
/* all the way to 3, and had 19 or 18 in other stats because of it.       */

void TPerson::advanceLevel(classIndT Class)
{
  setLevel(Class, getLevel(Class) + 1);
  calcMaxLevel();

  if (desc && GetMaxLevel() >= 40 &&  desc->career.hit_level40 == 0)
    desc->career.hit_level40 = time(0);
  if (desc && GetMaxLevel() >= 50) {
    if (desc->career.hit_level50 == 0)
      desc->career.hit_level50 = time(0);
#if 0
    if (isSingleClass()) {
      SET_BIT(desc->account->flags, ACCOUNT_ALLOW_DOUBLECLASS);
      sendTo(COLOR_BASIC, "<r>Congratulations on obtaining L50!<z>\n\rYou may now create <y>double-class characters<z>!\n\r");
      desc->saveAccount();
    }
    if (isDoubleClass()) {
      SET_BIT(desc->account->flags, ACCOUNT_ALLOW_TRIPLECLASS);
      sendTo(COLOR_BASIC, "<r>Congratulations on obtaining L50!<z>\n\rYou may now create <y>triple-class characters<z>!\n\r");
      desc->saveAccount();
    }
#endif
  }

  doHPGainForLev(Class);

  classSpecificStuff();

  // if gaining to L51 (how the hell does this happen), give them freedoms
  if (GetMaxLevel() > MAX_MORT) {
    condTypeT i;
    for (i = MIN_COND; i < MAX_COND_TYPE; ++i)
      setCond(i, -1);
  }
}

void TPerson::doHPGainForLev(classIndT Class)
{
  points.maxHit += hpGainForLevel(Class);
}

void TBeing::dropLevel(classIndT Class)
{
  double add_hp;
 
  if (GetMaxLevel() > MAX_MORT)
    return;

  if (GetMaxLevel() == 1)
    return;

  add_hp = hpGainForLevel(Class);

  addPracs(-calcNewPracs(Class, FALSE), Class);
  while (getPracs(Class) < 0) {
    addPracs(1, Class);
    discNumT dnt;
    for (dnt = MIN_DISC; dnt < MAX_DISCS; dnt++) {
      CDiscipline *cd = getDiscipline(dnt);
      if (cd && (cd->getLearnedness() > 0)) {
        cd->setNatLearnedness(cd->getNatLearnedness() - calcRaiseDisc(dnt, TRUE));
        break;  /* for loop */
      }
    }
    if (dnt == MAX_DISCS)
      break;  /* player has negitive pracs and no learning */
  }
    
  setLevel(Class,  getLevel(Class) - 1);

  if (getLevel(Class) < 1)
    setLevel(Class, 1);

  calcMaxLevel();

  points.maxHit -= max(1, (int) add_hp);
  if (points.maxHit < 1)
    points.maxHit = 1;

  setExp( min(getExpClassLevel(Class, getLevel(Class)), getExp()));

  if (points.exp < 0)
    points.exp = 0;
}

// tForm == false = update title
// tForm == true  = replace title
void TPerson::setTitle(bool tForm)
{
  char   tString[256] = "\0",
         tBuffer[256] = "\0";
  sstring tStString((title ? title : "ERROR OCCURED"));

  if (tForm) {
    sprintf(tString, "<n> the %s level %s",
          numberAsString(GetMaxLevel()).c_str(),
          getMyRace()->getSingularName().c_str());
  } else {
    sprintf(tString, "%s", numberAsString(GetMaxLevel() - 1).c_str());
    sprintf(tBuffer, "%s", numberAsString(GetMaxLevel()).c_str());

    while (tStString.find(tString) != sstring::npos)
      tStString.replace(tStString.find(tString), strlen(tString), tBuffer);

    strcpy(tString, tStString.c_str());
  }

  delete [] title;
  title = mud_str_dup(tString);
}


// Ok we're doing some wierd stuff in this function, so I'll document
// here.  For the exp cap, we're capping anything with: rawgain >
// (damage_of_hit / max_hp_of_mob) * (exp_for_level /
// min_kills_per_lev) we also modify for number of classes, so that
// the cap is 1/2 as large for a dual class basically, if you play
// with the formula, you'll find that this limits the player so that
// in any particular fight, the player CANNOT gain more than
// 1/min_kills of his exp needed to move from his current level to the
// next.  damage / max_hp is passed as a single integer, dam (no
// damage = 0, full damage = 10000) exp_for_level is calculated from
// the exp needed for next lev minus exp needed for current lev ie,
// (peak - curr) min_kills_per_lev is an AXIOM, since projected kills
// per lev is 17 + 1.25*lev (18-80, linear) we decided on gainmod =
// lev + 1 (2-50, linear) if the cap is being hit too much, it is a
// sign that the PCs have grown too powerful, or the gainmod formula
// is too strict.  if the cap isn't being hit (specifically when
// players are being pleveled) then gainmod is too lenient.  Dash -
// Jan 2001

// NOTE, gainmod is defined in multiple places, if you change it,
// change all of them please adding a small change so that low levs
// don't have a cap of 0 (max(lev*2,newgain)) it looks like my logic
// for multiclasses isn't right, they're getting 1/#c as much exp as
// they should, even with the multiclass penalty... 01/01 - dash


void gain_exp(TBeing *ch, double gain, int dam)
{
  classIndT Class;
  double newgain = 0;
  double oldcap = 0;
  bool been_here = false;

  if (ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA)) {
    // do nothing, these rooms safe
    return;
  }

  if(ch->isImmortal())
    return;
  

  if (gain < 0) {
    ch->addToExp(gain);
    if (ch->getExp() < 0)
      ch->setExp(0);

    return;
  }

  if(TestCode1)
    gain *= 2;
  
  gain /= ch->howManyClasses();

  for (Class = MIN_CLASS_IND; Class < MAX_CLASSES; Class++) {
    if(!ch->getLevel(Class))
      continue;

    const double peak2 = getExpClassLevel(Class,ch->getLevel(Class) + 2);
    const double peak = getExpClassLevel(Class,ch->getLevel(Class) + 1);
    const double curr = getExpClassLevel(Class,ch->getLevel(Class));
    const double gainmod = ((1.15*ch->getLevel(Class)) );
      
    // calculate exp gain
    if (!been_here && gain > ((double)(dam)*(peak-curr))/(gainmod*(double)(ch->howManyClasses()*10000))+2.0 && dam > 0) {
      been_here = TRUE; // don't show multiple logs for multiclasses
	
      // the 100 turns dam into a %
      newgain = ((double)(dam)*(peak-curr))/(gainmod*(double)(ch->howManyClasses()*10000)) + 1.0;
	
      // 05/24/01 - adding a 'soft' cap here
      oldcap = newgain;
      double softmod = (1.0 - pow( 1.1 , -1.0*(gain/newgain))) + 1.0;     
	
      // this gives us a range of 1-2
      newgain *= softmod;
	
      //newgain = (newgain*0.95) +  (((float)::number(0,100))*newgain)/1000.0;
      // don't need this anymore since no hard cap - dash
      if (gain < newgain)
	newgain = gain;
      gain = newgain;
    }
      

    // reset 50th levelers to 1bil exp if their Max Exp is unset (0)
    // : should never happen except for 1time conversion this also
    // verifies first timers get the practices they deserve - dash
    // oct 2003
    if (ch->isPc() && ch->getMaxExp() == 0) {
      vlogf(LOG_DASH, fmt("%s getting exp checked: MaxExp %.2f, Exp %.2f, Current Level Exp %.2f") %  ch->getMaxExp() % ch->getExp() % getExpClassLevel(Class, 50) % curr);
      ch->setExp(min(ch->getExp(), getExpClassLevel(Class,50)));
      ch->setMaxExp(curr);
    }


    // check for prac gain
    if((ch->getExp() + gain) > ch->getMaxExp()){
      double t_curr=curr, t_peak=peak;
      double delta_exp = (t_peak - t_curr) / ch->pracsPerLevel(Class, false);
      double exp = ch->getExp();
      int gain_pracs=0;
      double t_exp=0;
      double new_exp = ch->getExp() + gain;

      if(ch->getMaxExp() < peak){
	// getting some exp in the current level
	t_curr=curr;            // beginning of this level
	t_peak=peak;            // end of this level
	delta_exp = (t_peak - t_curr) / ch->pracsPerLevel(Class, false);
	exp = ch->getMaxExp();  // gain pracs only past this exp
	new_exp = min(t_peak, ch->getExp() + gain);

	// loop through this levels exp, stepping by the delta_exp, ie
	// the amount needed for each practice.
	for(t_exp=t_curr+delta_exp;t_exp<=new_exp && t_exp<=t_peak;t_exp+=delta_exp){
	  // if this exp step is past our max exp and is under the exp
	  // we've gained, then get a prac
	  if(t_exp > exp && t_exp <= new_exp){
	    vlogf(LOG_SILENT, fmt("%s gaining practice (current): t_curr=%f, t_peak=%f, delta_exp=%f, exp=%f, new_exp=%f, t_exp=%f") %  ch->getName() % t_curr % t_peak % delta_exp % exp % new_exp % t_exp); ;
	    gain_pracs++;
	  }
	}

	// crossing the threshold into the next level
	if(new_exp >= peak){
	  // roll for extra prac
	  if(::number(1,(int)delta_exp) < (delta_exp - (t_peak - t_exp))){
	    vlogf(LOG_SILENT, fmt("%s gaining practice (threshold): t_curr=%f, t_peak=%f, delta_exp=%f, exp=%f, new_exp=%f") %  ch->getName() % t_curr % t_peak % delta_exp % exp % new_exp);
	    gain_pracs++;
	  }
	}
      }

      if(new_exp > peak){
	// getting exp in the next level

	if(ch->getLevel(Class)>=MAX_MORT){
	  // for level 50's we need to calculate what level they would
	  // be with the exp they have and use that for peak and peak2
	  // arbitrarily cutting loop off at 127
	  for(int i=50;i<127;++i){
	    if(getExpClassLevel(Class,i) > ch->getExp()){
	      t_curr=getExpClassLevel(Class,i-1);
	      t_peak=getExpClassLevel(Class,i);
	      break;
	    }
	  }
	} else {
	  t_curr=peak;
	  t_peak=peak2;
	}

	delta_exp = (t_peak - t_curr) / ch->pracsPerLevel(Class, false);
	exp = max(ch->getMaxExp(),t_curr+1);
	new_exp = ch->getExp() + gain;

	for(double j=t_curr;j<=new_exp && j<=t_peak;j+=delta_exp){
	  if(j > exp && j < new_exp){
	    vlogf(LOG_SILENT, fmt("%s gaining practice (next): t_curr=%f, t_peak=%f, delta_exp=%f, exp=%f, new_exp=%f, t_exp=%f") %  ch->getName() % t_curr % t_peak % delta_exp % exp % new_exp % t_exp); ;
	    gain_pracs++;
	  }
	}
      }

      if(gain_pracs > 0){
	ch->addPracs(gain_pracs, Class);
	if(gain_pracs == 1){
	  ch->sendTo(COLOR_BASIC, "<W>You have gained a practice!<1>\n\r");
	} else {
	  ch->sendTo(COLOR_BASIC, fmt("<W>You have gained %i practices!<1>\n\r") %
		     gain_pracs);
	}
      }
    }


    // check for level gain
    // do this last, so as not to mess up predefined values
    if ((ch->getExp() >= peak) &&
	(ch->GetMaxLevel() < MAX_MORT)) {
      TPerson * tPerson = dynamic_cast<TPerson *>(ch);

      ch->raiseLevel(Class);
      ch->sendTo(COLOR_BASIC, "<W>You advance a level!<1>\n\r");

      if (tPerson)
        tPerson->setSelectToggles(NULL, Class, SILENT_YES);

      if(ch->hasClass(CLASS_MONK))
	ch->remQuestBit(TOG_MONK_PAID_TABUDA);
    }
  }

  ch->addToExp(gain);


  // update max exp
  if(ch->getExp() > ch->getMaxExp())
    ch->setMaxExp(ch->getExp());

}

void TFood::findSomeFood(TFood **last_good, TBaseContainer **last_cont, TBaseContainer *cont) 
{
  // get item closest to spoiling.
  if (*last_good && obj_flags.decay_time > (*last_good)->obj_flags.decay_time)
    return;

  *last_good = this;
  *last_cont = cont;
}

void TBeing::gainCondition(condTypeT condition, int value)
{
  int intoxicated, i = 0, loopchk=0;
  char buf[160], tmpbuf[40], buf2[256];
  TThing *t = NULL;

  if (getCond(condition) == -1)        // No change 
    return;

  loopchk=getCond(condition);

  intoxicated = (getCond(DRUNK) > 0);

  if (value > 0) {
    // adjust for race
    // i.e. if code says I should gain 1, but I am an ogre, figure I may
    // only gain 0.5 or so...  Same situation, hobbits might get 2
    // this is racial metabolism vs human norm
    switch (condition) {
      case FULL:
        value = (int) (value * getMyRace()->getFoodMod()); 
        value = max(1, value);
	// lets take body mass into consideration for hunger
	value = (int) (value * 180.0 / getWeight());
	value = max(1, value);

        break;
      case THIRST:
	// lets take body mass into consideration for thirst
	value = (int) (value * 180.0 / getWeight());
	value = max(1, value);
	break;
      case DRUNK:
        value = (int) (value * getMyRace()->getDrinkMod()); 
        value = max(1, value);
	// lets take body mass into consideration for drunkenness
	// this may need some revision
	// At the momeny we are scaling linearly and taking drunkenness factors
	// normalized to a 180LB person
	value = (int) (value * 180.0 / getWeight());
	value = max(1, value);

	// modify for SKILL_ALCOHOLISM
	value = (int)((double) value * (double)(((double)(105 - getSkillValue(SKILL_ALCOHOLISM)) / 100)));
        break;
      case PEE:
      case POOP:
      case MAX_COND_TYPE:
        break;
    }
  }
  value = min(value, 24);

  setCond(condition, getCond(condition) + value);
  if (getCond(condition) < 0)
    setCond(condition, 0);
  if (getCond(condition) > 24)
    setCond(condition, 24);

  // this is just a warning message
  // if amt is 0, they get an actual message from the periodic stuff
  int amt = getCond(condition);
  if (amt <= 2 && amt > 0) {
    switch (condition) {
      case FULL:
        sendTo("You can feel your stomach rumbling.\n\r");
        break;
      case THIRST:
        sendTo("You begin to feel your mouth getting dry.\n\r");
        break;
      case DRUNK:
	if(inRoom() >= 31800 && inRoom() <= 31899){    
	  sendTo("As you begin to sober up, your grasp on this strange world begins to loosen.\n\r");
	}

        break;
      case PEE:
      case POOP:
      case MAX_COND_TYPE:
	break;
    }
  }
  if (amt)
    return;

  // this should prevent auto-eat loops
  if(getCond(condition) == loopchk)
    return;


  // beyond here, auto eat

  if ((condition == FULL) && desc && (desc->autobits & AUTO_EAT) && awake() &&
      (!task || task->task == TASK_SIT || task->task == TASK_REST)) {
    // Check to see if they are in center square to use statue Russ 01/06/95
    if ((in_room == ROOM_CS) && (GetMaxLevel() <= 3)) {
      // execute instantly, makes no sense to que this
      parseCommand("pray statue", FALSE);
      return;
    }
    TFood *last_good = NULL;
    TBaseContainer *last_cont = NULL;
    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      if (!(t = equipment[i]))
        continue;

      t->findSomeFood(&last_good, &last_cont, NULL); 
    }
    for (t = getStuff(); t; t = t->nextThing) 
      t->findSomeFood(&last_good, &last_cont, NULL);
            
    if (last_good && !last_cont) {
      sprintf(buf, "%s", fname(last_good->name).c_str());
      doEat(buf);
#if 0
  // Changed this so auto eat didn't make AFK go off
  // Will change the part with the bag later, more complicated
      sprintf(buf, "eat %s", fname(last_good->name).c_str());
      addCommandToQue(buf);
#endif
    } else if (last_good && last_cont) {
      sprintf(buf, "%s", last_cont->name);
      strcpy(buf, add_bars(buf).c_str());
      strcpy(tmpbuf, fname(last_good->name).c_str());
      sprintf(buf2, "get %s %s", tmpbuf, buf);
      addCommandToQue(buf2);
      sprintf(buf2, "eat %s", tmpbuf);
      addCommandToQue(buf2);
    }
    return;
  }
  if ((condition == THIRST) && desc && (desc->autobits & AUTO_EAT) && awake() &&
      (!task || task->task == TASK_SIT || task->task == TASK_REST)) {
    // Check to see if fountain is in room and drink from that before
    // anything else - Russ 01/06/95
    for (t = roomp->getStuff(); t; t = t->nextThing) {
      if (dynamic_cast<TObj *>(t) && (t->spec == SPEC_FOUNTAIN)) {
        char drinkbuf[256];
	sprintf(drinkbuf, "drink %s", t->name);
	parseCommand(drinkbuf, FALSE);
        return;
      }
    }
    TDrinkCon *last_good = NULL;
    TBaseContainer *last_cont = NULL;
    for (i = MIN_WEAR, last_good = NULL; i < MAX_WEAR; i++) {
      if (!(t = equipment[i]))
        continue;
      t->findSomeDrink(&last_good, &last_cont, NULL);

      if (last_good)
        break;
    }

    if (!last_good)
      for (t = getStuff(); t; t = t->nextThing) {
        t->findSomeDrink(&last_good, &last_cont, NULL);

	if (last_good)
	  break;
      }

    if (last_good && !last_cont) {
      sprintf(buf, "%s", fname(last_good->name).c_str());
      doDrink(buf);
#if 0
      sprintf(buf, "drink %s", fname(last_good->name).c_str());
      addCommandToQue(buf);
#endif
      return;
    }
    if (last_good && last_cont) {
      sprintf(buf, "%s", last_cont->name);
      strcpy(buf, add_bars(buf).c_str());
      if (getPosition() == POSITION_RESTING) 
        addCommandToQue("sit");
            
      strcpy(tmpbuf, fname(last_good->name).c_str());
      sprintf(buf2, "get %s %s", tmpbuf, buf);
      addCommandToQue(buf2);
      sprintf(buf2, "drink %s", tmpbuf);
      addCommandToQue(buf2);
      sprintf(buf2, "put %s %s", tmpbuf, buf);
      addCommandToQue(buf2);
      return;
    }
  }
  switch (condition) {
    case DRUNK:
      if (intoxicated)
        sendTo("You are now sober.\n\r");
      return;
    default:
      break;
  }
}

// returns DELETE_THIS
int TBeing::checkIdling()
{
  if (desc && desc->connected >= CON_REDITING)
    return FALSE;

  if (isImmortal() || inRoom() == ROOM_STORAGE)
    return FALSE;

  if (getTimer() == 10) {
    if (specials.was_in_room == ROOM_NOWHERE &&
        inRoom() != ROOM_NOWHERE && inRoom() != ROOM_STORAGE) {
      specials.was_in_room = in_room;
      if (fight()) {
        if (fight()->fight())
          fight()->stopFighting();
        stopFighting();
      }
      act("$n disappears into the void.", TRUE, this, 0, 0, TO_ROOM);
      sendTo("You have been idle, and are pulled into a void.\n\r");
      --(*this);
      thing_to_room(this, ROOM_VOID);
      doSave(SILENT_NO);
    }
  } else if (getTimer() == 20) {
    if (in_room != ROOM_STORAGE) {
      if (in_room != ROOM_NOWHERE)
        --(*this);

      vlogf(LOG_SILENT, fmt("%s booted from game for inactivity.") %  getName());
      thing_to_room(this, ROOM_DUMP);

      // this would be done by ~TPerson
      // but generates an error.  So we do it here in "safe" mode instead
      TPerson *tper = dynamic_cast<TPerson *>(this);
      if (tper)
        tper->dropItemsToRoom(SAFE_YES, NUKE_ITEMS);

      delete desc;
      desc = NULL;
      return DELETE_THIS;
    }
#if 0 
  }
#else 
  } else if (desc && desc->original && (desc->original->getTimer() >= 20)) {
    TBeing *mob = NULL;
    TBeing *per = NULL;

    if ((specials.act & ACT_POLYSELF)) {
      mob = this;
      per = desc->original;
      act("$n turns liquid, and reforms as $N.", TRUE, mob, 0, per, TO_ROOM);
      --(*per);
      *mob->roomp += *per;
      SwitchStuff(mob, per);
      per->polyed = POLY_TYPE_NONE;
    }
    desc->character = desc->original;
    desc->original = NULL;

    desc->character->desc = desc;
    desc = NULL;
    per->setTimer(10);
    per->checkIdling();
    if (IS_SET(specials.act, ACT_POLYSELF)) {
      // WTF is this for?  wouldn't a return make more sense?
vlogf(LOG_BUG, "does this get called?  checkIdle");

      // this would be done by ~TPerson
      // but generates an error.  So we do it here in "safe" mode instead
      TPerson *tper = dynamic_cast<TPerson *>(this);
      if (tper)
        tper->dropItemsToRoom(SAFE_YES, NUKE_ITEMS);

      return DELETE_THIS;
    }

  }
#endif
  return FALSE;
}

int TBeing::moneyMeBeing(TThing *mon, TThing *sub)
{
  int rc = mon->moneyMeMoney(this, sub);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_ITEM;
  return FALSE;
}

int ObjFromCorpse(TObj *c)
{
  TThing *t, *t2, *t3;
  int rc;

  for (t = c->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    --(*t);

    TObj *tobj = dynamic_cast<TObj *>(t);
    if (tobj && tobj->isObjStat(ITEM_NEWBIE) && !tobj->getStuff() &&
          (c->in_room > 80) && (c->in_room != ROOM_DONATION)) {
      sendrpf(c->roomp, "The %s explodes in a flash of white light!\n\r", fname(tobj->name).c_str());
      delete tobj;
      tobj = NULL;
      continue;
    }

    if ((t3 = c->parent)) {
      *(t3) += *t;
      rc = t3->moneyMeBeing(t, c);
      if (IS_SET_DELETE(rc, DELETE_ITEM))
        delete t;
    } else if (c->in_room != ROOM_NOWHERE)
      *c->roomp += *t;
  }
  return TRUE;
}

void TBeing::classSpecificStuff()
{
  if (hasClass(CLASS_WARRIOR)) 
    setMult(1.0);
  
  if (hasClass(CLASS_MONK)) {
    // from balance note
    // we desire monks to be getting 5/7 sqrt(lev) hits per round
    // barehand prof is linear from L1-L30 : n = 10/3 * L 
    // barehand spec is linear roughly L31-L50 n = (L-30) * 5
    // solve for L as a function of n
    double value = 0;
    if (doesKnowSkill(SKILL_BAREHAND_PROF))
      value += 3.0 * getSkillValue(SKILL_BAREHAND_PROF) / 10.0;
    if (doesKnowSkill(SKILL_BAREHAND_SPEC))
      value += getSkillValue(SKILL_BAREHAND_SPEC) / 5.0;

    // value is roughly their actual level, but based on skill instead
    value = min(max(1.0, value), 50.0);
 
    // now make it into numHits
    value = 5.0 * sqrt(value) / 7.0;


    if (doesKnowSkill(SKILL_ADVANCED_KICKING))
      value += getSkillValue(SKILL_ADVANCED_KICKING) / 100.0;

    // adjust for speed
    //    value = value * plotStat(STAT_CURRENT, STAT_SPE, 0.8, 1.25, 1.0);
    value *= getSpeMod();


    // give at least 1 hit per hand
    // reverse engineering, we realize 2.0 comes around L7.8 = 26%barehand
    // so this is also a newbie normalization
    value = max(value, 2.0);

    setMult(value);
  }
}

// computes the time it takes to get back 1 point of hp,mana and move
// it uses worst of the three
int TBeing::regenTime()
{
  int iTime = 100;

  if (getHit() < hitLimit())
    iTime = min(iTime, hitGain());

  if (getMove() < moveLimit())
    iTime = min(iTime, moveGain());

  if (getMana() < manaLimit())
    iTime = min(iTime, manaGain());

  iTime = max(iTime, 1);

  // iTime now holds the lesser of my mana/move/hp gain (per update)
  // regen time for 1 point should simply be the inverse multiplied
  // by how long a points-update is
  iTime = PULSE_UPDATE / iTime;
  return iTime;
}

int TBeing::hpGainForClass(classIndT Class) const
{
  int hpgain = 0;

  // add for classes first
  if (hasClass(CLASS_MAGE) && Class == MAGE_LEVEL_IND)
    hpgain += ::number(3,7); // old 2,8

  if (hasClass(CLASS_CLERIC) && Class == CLERIC_LEVEL_IND)
    hpgain += ::number(5,7); // old 4,8

  if (hasClass(CLASS_WARRIOR) && Class == WARRIOR_LEVEL_IND)
    hpgain += ::number(6,11); // old 5,12

  if (hasClass(CLASS_THIEF) && Class == THIEF_LEVEL_IND)
    hpgain += ::number(4,8); // old 3,9

  if (hasClass(CLASS_DEIKHAN) && Class == DEIKHAN_LEVEL_IND)
    hpgain += ::number(6,9); // old 5,10

  if (hasClass(CLASS_MONK) && Class == MONK_LEVEL_IND)
    hpgain += ::number(4,7); // old 3,8

  if (hasClass(CLASS_RANGER) && Class == RANGER_LEVEL_IND)
    hpgain += ::number(6,8); // old 5,9

  if (hasClass(CLASS_SHAMAN) && Class == SHAMAN_LEVEL_IND)
    hpgain += ::number(3,8); 

  return hpgain;
}


int TBeing::hpGainForLevel(classIndT Class) const
{
  double hpgain = 0;

  hpgain = (double) hpGainForClass(Class);  

  hpgain *= (double) getConHpModifier();

  hpgain /= (double) howManyClasses();

  if(roll_chance(hpgain - (int) hpgain)){
    hpgain += 1.0;
  }

  return max(1, (int)hpgain);
}


