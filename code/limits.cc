//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: limits.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


extern "C" {
#include <unistd.h>
}
#include <cmath>

#include "stdsneezy.h"
#include "statistics.h"
#include "games.h"

#if 0
static const string ClassTitles(const TBeing *ch)
{
  int count = 0;
  classIndT i;
  char buf[256];

  for (i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
    if (ch->getLevel(i)) {
      if ((++count) > 1)
        sprintf(buf + strlen(buf), "/Level %d %s", 
                ch->getLevel(i), classNames[i].capName);
      else
        sprintf(buf, "Level %d %s",
                ch->getLevel(i), classNames[i].capName);
    }
  }
  return (buf);
}
#endif

int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{
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

short int TPerson::hitLimit() const
{
  return points.maxHit + graf((age()->year - getBaseAge() + 15), 2, 4, 17, 14, 8, 4, 3);
}

short int TPerson::manaLimit() const
{
  int iMax = 100;

  if (hasClass(CLASS_MAGIC_USER))
    iMax += getLevel(MAGE_LEVEL_IND) * 6;
#if 0
  else if (hasClass(CLASS_CLERIC))
    iMax += getLevel(CLERIC_LEVEL_IND) * 5;
  else if (hasClass(CLASS_DEIKHAN))
    iMax += GetMaxLevel() * 3;
#endif
  else if (hasClass(CLASS_RANGER))
    iMax += GetMaxLevel() * 3;
  else if (hasClass(CLASS_MONK))
    iMax += GetMaxLevel() * 3;

  iMax += points.maxMana;        /* bonus mana */

  return (iMax);
}

int TPerson::getMaxMove() const
{
  return 100 + age()->year - getBaseAge() + 15 + GetTotLevel() +
      plotStat(STAT_CURRENT, STAT_CON, 3, 18, 13);
}

short int TBeing::moveLimit() const
{
  int iMax = getMaxMove() + race->getMoveMod();

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
    gain *= 25.0;  // arbitrary
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
  gain = (150.0 + modif)/16.0;
#endif

  if (dynamic_cast<TPerson *>(this)) {
    stats.piety_gained_attempts++;
    stats.piety_gained += gain;
  }
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

  return (gain);
}

int TPerson::manaGain()
{
  int gain;

  if (fight() || spelltask)
    return 0;

  if (!desc)
    return 0;

  gain = graf((age()->year - getBaseAge() + 15), 2, 4, 6, 8, 10, 12, 14);

  // arbitrary multiplier
  // at 1.0 : Average mana regen            :  5.69  (attempts : 15624)
  // at 2.0 : Average mana regen            : 11.11  (attempts : 30584)
  // at L50, mage has 400 mana, think we want 20
  gain *= 4;

  if (hasClass(CLASS_MAGIC_USER))
    gain += gain;

  gain += race->getManaMod();

  if (!getCond(FULL) || !getCond(THIRST))
    gain >>= 2;

  stats.mana_gained_attempts++;
  stats.mana_gained += gain;

  return (gain);
}

int TMonster::hitGain()
{
  int gain;
  int num;

  gain = max(2, (GetMaxLevel() / 2));

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

  return (gain);
}

int TPerson::hitGain()
{
  int gain;
  int num;

  if (!desc)
    return 0;

  if (fight())
    gain = 0;
  else {
    gain = graf((age()->year - getBaseAge() + 15), 2, 4, 5, 9, 4, 3, 2);
    gain += plotStat(STAT_CURRENT,STAT_CON,1,8,4);
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

  if (!getCond(FULL) || !getCond(THIRST))
    gain >>= 2;	// rightshift by 2 is div by 4

  if ((num = inCamp())) {
    gain += (gain * num / 100);
  }

  if (roomp && roomp->isRoomFlag(ROOM_NO_HEAL))
    gain /= 3;
  if (roomp && roomp->isRoomFlag(ROOM_HOSPITAL))
    gain *= 2;

  gain = max(gain,1);

  if (dynamic_cast<TPerson *>(this)) {
    stats.move_gained_attempts++;
    stats.move_gained += gain;
  }

  return (gain);
}

sh_int TBeing::calcNewPracs(classIndT Class, bool forceBasic)
{
  sh_int prac;
  float num;
  bool preReqs = FALSE;
  float fMin, fMax, avg ;
  fMin = fMax = avg = 0;
  int combat = 0;
  bool doneCombat = FALSE;

  if (Class == MAGE_LEVEL_IND || Class == SHAMAN_LEVEL_IND) {
    combat = getDiscipline(DISC_COMBAT)->getLearnedness() + getDiscipline(DISC_LORE)->getLearnedness();
  } else if (Class == CLERIC_LEVEL_IND || Class == DEIKHAN_LEVEL_IND) {
    combat = getDiscipline(DISC_COMBAT)->getLearnedness() + getDiscipline(DISC_THEOLOGY)->getLearnedness();
  } else {
    combat = getDiscipline(DISC_COMBAT)->getLearnedness();
  }

  if (combat >= MAX_DISC_LEARNEDNESS) {
    doneCombat = TRUE;
  }

  switch (Class) {
    case MAGE_LEVEL_IND:
      if (!doneCombat || (getDiscipline(DISC_MAGE)->getLearnedness() < MAX_DISC_LEARNEDNESS) || forceBasic == 1) {
        preReqs = TRUE;
        fMin =5.0;
        fMax =8.0;
        avg =6.7;
      } 
      break;
    case CLERIC_LEVEL_IND:
if (!doneCombat || (getDiscipline(DISC_CLERIC)->getLearnedness() < MAX_DISC_LEARNEDNESS) || forceBasic == 1) {
        preReqs = TRUE;
        fMin =5.0;
        fMax =8.0;
        avg =6.7;
      }
      break;
    case WARRIOR_LEVEL_IND:
      if (!doneCombat || (getDiscipline(DISC_WARRIOR)->getLearnedness() < MAX_DISC_LEARNEDNESS) || forceBasic == 1) {
        preReqs = TRUE;
        fMin =6.0;
        fMax =8.0;
        avg =7.0;
      }
      break;
    case THIEF_LEVEL_IND:
      if (!doneCombat || (getDiscipline(DISC_THIEF)->getLearnedness() < MAX_DISC_LEARNEDNESS) || forceBasic == 1) {

        preReqs = TRUE;
        fMin =5.0;
        fMax =8.0;
        avg =7.0;
      }
      break;
    case DEIKHAN_LEVEL_IND:
      if (!doneCombat || (getDiscipline(DISC_DEIKHAN)->getLearnedness() < MAX_DISC_LEARNEDNESS) || forceBasic == 1) {
        fMin =5.0;
        fMax =8.0;
        avg =7.0;
        preReqs = TRUE;
      }
      break;
    case MONK_LEVEL_IND:
      if (!doneCombat || (getDiscipline(DISC_MONK)->getLearnedness() < MAX_DISC_LEARNEDNESS) || forceBasic == 1) {

        preReqs = TRUE;
        fMin =5.0;
        fMax =8.0;
        avg =7.0;
      }
      break;
    case RANGER_LEVEL_IND:
      if (!doneCombat || (getDiscipline(DISC_RANGER)->getLearnedness() < MAX_DISC_LEARNEDNESS) || forceBasic == 1) {

        preReqs = TRUE;
        fMin =5.0;
        fMax =8.0;
        avg =6.7;
      }
      break;
    case SHAMAN_LEVEL_IND:
      if (!doneCombat || (getDiscipline(DISC_SHAMAN)->getLearnedness() < MAX_DISC_LEARNEDNESS) || forceBasic == 1) {

        preReqs = TRUE;
        fMin =5.0;
        fMax =8.0;
        avg =7.0;
      }
      break;
    case UNUSED1_LEVEL_IND:
    case UNUSED2_LEVEL_IND:
    case UNUSED3_LEVEL_IND:
    case MAX_SAVED_CLASSES:
      vlogf(8, "Bad class in calcNewPracs");
      break;
  } 
  if (preReqs) {
    if (!desc){
      num = plotStat(STAT_CURRENT, STAT_INT, fMin, fMax, avg, 1.0);
    } else {
      num = plotStat(STAT_NATURAL, STAT_INT, fMin, fMax, avg, 1.0);
    }
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
    if ((100*num) >= (::number(1,100))) {
      prac++;
    }
    return prac;
  } else {
    fMin = 5.0;
    avg = 6.9;
    fMax = 9.0;
    if (!desc) {
      num = plotStat(STAT_CURRENT, STAT_INT, fMin, fMax, avg, 1.0);
    } else {
      num = plotStat(STAT_NATURAL, STAT_INT, fMin, fMax, avg, 1.0);
    }
    prac = (int) num;
    num = num - prac;
    if ((100*num) >= (::number(1,100))) {
      prac++;
    }
    return prac;
  }
}

void TBeing::setPracs(sh_int prac, classIndT Class)
{
  switch (Class) {
    case MAGE_LEVEL_IND:
      practices.mage = prac;
      break;
    case CLERIC_LEVEL_IND:
      practices.cleric = prac;
      break;
    case THIEF_LEVEL_IND:
      practices.thief = prac;
      break;
    case WARRIOR_LEVEL_IND:
      practices.warrior = prac;
      break;
    case DEIKHAN_LEVEL_IND:
      practices.deikhan = prac;
      break;
    case RANGER_LEVEL_IND:
      practices.ranger = prac;
      break;
    case MONK_LEVEL_IND:
      practices.monk = prac;
      break;
    case SHAMAN_LEVEL_IND:
      practices.shaman = prac;
      break;
    case UNUSED1_LEVEL_IND:
    case UNUSED2_LEVEL_IND:
    case UNUSED3_LEVEL_IND:
    case MAX_SAVED_CLASSES:
      vlogf(8, "Bad class in setPracs");
  }
}

sh_int TBeing::getPracs(classIndT Class)
{
  switch (Class) {
    case MAGE_LEVEL_IND:
      return practices.mage;
    case CLERIC_LEVEL_IND:
      return practices.cleric;
    case THIEF_LEVEL_IND:
      return practices.thief;
    case WARRIOR_LEVEL_IND:
      return practices.warrior;
    case DEIKHAN_LEVEL_IND:
      return practices.deikhan;
    case RANGER_LEVEL_IND:
      return practices.ranger;
    case MONK_LEVEL_IND:
      return practices.monk;
    case SHAMAN_LEVEL_IND:
      return practices.shaman;
    case UNUSED1_LEVEL_IND:
    case UNUSED2_LEVEL_IND:
    case UNUSED3_LEVEL_IND:
    case MAX_SAVED_CLASSES:
      forceCrash("Bad class in getPracs");
  }
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

void TPerson::advanceLevel(classIndT Class, TMonster *gm)
{
  sh_int prac = 0;

  setLevel(Class, getLevel(Class) + 1);
  calcMaxLevel();

  if (desc && GetMaxLevel() >= 40 &&  desc->career.hit_level40 == 0)
    desc->career.hit_level40 = time(0);
  if (desc && GetMaxLevel() >= 50) {
    if (desc->career.hit_level50 == 0)
      desc->career.hit_level50 = time(0);
    if (isSingleClass()) {
      SET_BIT(desc->account->flags, ACCOUNT_ALLOW_DOUBLECLASS);
      sendTo(COLOR_BASIC, "<r>Congratulations on obtaining L50!<z>\n\rYou may now create <y>double-class characters<z>!\n\r");
    }
    if (isDoubleClass()) {
      SET_BIT(desc->account->flags, ACCOUNT_ALLOW_TRIPLECLASS);
      sendTo(COLOR_BASIC, "<r>Congratulations on obtaining L50!<z>\n\rYou may now create <y>triple-class characters<z>!\n\r");
    }
  }

  prac = calcNewPracs(Class, FALSE);
  addPracs(prac, Class);

  if (gm)
    pracPath(gm, Class, (ubyte) prac);

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
  double add_hp = hpGainForLevel(Class);

  int tmp;
  tmp = (int) add_hp;   // tmp is holding the rounded off portion of hp

  // chance of an extra hp
  add_hp -= (double) tmp;
  add_hp *= 100;
  if (((int) add_hp) > (::number(0,99)))
    tmp++;

  points.maxHit += max(1, tmp);
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
  string tStString((title ? title : "ERROR OCCURED"));

  if (tForm) {
    sprintf(tString, "<n> the %s level %s",
          numberAsString(GetMaxLevel()).c_str(),
          getMyRace()->getSingularName().c_str());
  } else {
    sprintf(tString, "%s", numberAsString(GetMaxLevel() - 1).c_str());
    sprintf(tBuffer, "%s", numberAsString(GetMaxLevel()).c_str());

    while (tStString.find(tString) != string::npos)
      tStString.replace(tStString.find(tString), strlen(tString), tBuffer);

    strcpy(tString, tStString.c_str());
  }

  delete [] title;
  title = mud_str_dup(tString);
}

void gain_exp(TBeing *ch, double gain)
{
  classIndT i;

#if 0
  if (!ch->isPc() && ch->isAffected(AFF_CHARM)) {
#else
  if (!ch->isPc() && gain > 0) {
#endif
    // do_nothing so they get no extra exp
    return;
  } else if (ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA)) {
    // do nothing, these rooms safe
    return;
  } else if (!ch->isImmortal()) {
    if (gain > 0) {
      gain /= ch->howManyClasses();
      if (ch->isPc()) {
        for (i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
          double peak2 = getExpClassLevel(i,ch->getLevel(i) + 2);
          double peak = getExpClassLevel(i,ch->getLevel(i) + 1);
          if (ch->getLevel(i)) {
            // intentionally avoid having L50's get this message
            if ((ch->getExp() >= peak2) && (ch->GetMaxLevel() < MAX_MORT)) {
              ch->sendTo("You must gain at a guild or your exp will max 1 short of next level.\n\r");
              ch->setExp(peak2);
              return;
            } else if (ch->getExp() >= peak) {
              // do nothing..this rules! Tell Brutius Hey, I didnt get any exp? 
            } else if ((ch->getExp() + gain >= peak) &&
                 (ch->GetMaxLevel() < MAX_MORT)) {
              ch->sendTo(COLOR_BASIC, "<g>You have gained enough to be a Level %d %s.<1>\n\r", 
#if 0
       (startsVowel(ch->getClassTitle(i, ch->getLevel(i) + 1)) ? "an" : "a"), 
        ch->getClassTitle(i, ch->getLevel(i) + 1));
#else
                   ch->getLevel(i)+1, classNames[i].capName);
#endif
              ch->sendTo("You must gain at a guild or your exp will max 1 short of next level.\n\r");
              if (ch->getExp() + gain >= peak2) {
                ch->setExp(peak2- 1);
                return;
              }
            }
          }
        }
      }
// with new XP scale, not sure limiting to 3K for a single hit is good idea
// large mobs may yield MUCH more...  -Bat 12/98
//      gain = min(gain, (3000.0 * ch->GetMaxLevel()));
      ch->addToExp(gain);
      // we check mortal level here...
      // an imm can go mort to test xp gain (theoretically)
      // for level > 50, the peak would have flipped over, which is bad
      if (ch->isPc() && ch->GetMaxLevel() <= MAX_MORT) {
        for (i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
          double peak2 = getExpClassLevel(i,ch->getLevel(i) + 2);
          if (ch->getLevel(i)) {
            if (ch->getExp() > peak2)
              ch->setExp(peak2 - 1);
          }
        }
      }
    }
    if (gain < 0) {
      ch->addToExp(gain);
      if (ch->getExp() < 0)
        ch->setExp(0);
    }
  }
}

void TContainer::findSomeDrink(TDrinkCon **last_good, TContainer **last_cont, TContainer *) 
{
  TThing *t;

  for (t = stuff; t; t = t->nextThing) 
    t->findSomeDrink(last_good, last_cont, this);
}

void TContainer::findSomeFood(TFood **last_good, TContainer **last_cont, TContainer *)
{
  TThing *t;

  for (t = stuff; t; t = t->nextThing) 
    t->findSomeFood(last_good, last_cont, this);
}

void TFood::findSomeFood(TFood **last_good, TContainer **last_cont, TContainer *cont) 
{
  // get item closest to spoiling.
  if (*last_good && obj_flags.decay_time > (*last_good)->obj_flags.decay_time)
    return;

  *last_good = this;
  *last_cont = cont;
}

void TBeing::gainCondition(condTypeT condition, int value)
{
  int intoxicated, i = 0;
  char buf[160], tmpbuf[40], buf2[256];
  TThing *t = NULL;

  if (getCond(condition) == -1)        // No change 
    return;

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
        break;
      case THIRST:
      case DRUNK:
        value = (int) (value * getMyRace()->getDrinkMod()); 
        value = max(1, value);
        break;
      case MAX_COND_TYPE:
        break;
    }

    // lets take body mass into consideration for drunkenness
    if (condition == DRUNK) {
      // this may need some revision
      // At the momeny we are scaling linearly and taking drunkenness factors
      // normalized to a 180LB person
      value = (int) (value * 180.0 / getWeight());
      value = max(1, value);
    }
    // lets take body mass into consideration for thirst
    if (condition == THIRST) {
      value = (int) (value * 180.0 / getWeight());
      value = max(1, value);
    }
    // lets take body mass into consideration for hunger
    if (condition == FULL) {
      value = (int) (value * 180.0 / getWeight());
      value = max(1, value);
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
      case MAX_COND_TYPE:
      case DRUNK:
        break;
    }
  }
  if (amt)
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
    TContainer *last_cont = NULL;
    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      if (!(t = equipment[i]))
        continue;

      t->findSomeFood(&last_good, &last_cont, NULL); 
    }
    for (t = stuff; t; t = t->nextThing) 
      t->findSomeFood(&last_good, &last_cont, NULL);
            
    if (last_good && !last_cont) {
      sprintf(buf, "eat %s", fname(last_good->name).c_str());
      addCommandToQue(buf);
    } else if (last_good && last_cont) {
      sprintf(buf, "%s", last_cont->name);
      add_bars(buf);
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
    for (t = roomp->stuff; t; t = t->nextThing) {
      if (dynamic_cast<TObj *>(t) && (t->spec == SPEC_FOUNTAIN)) {
        parseCommand("drink fountain", FALSE);
        return;
      }
    }
    TDrinkCon *last_good = NULL;
    TContainer *last_cont = NULL;
    for (i = MIN_WEAR, last_good = NULL; i < MAX_WEAR; i++) {
      if (!(t = equipment[i]))
        continue;
      t->findSomeDrink(&last_good, &last_cont, NULL);
    }
    for (t = stuff; t; t = t->nextThing) 
      t->findSomeDrink(&last_good, &last_cont, NULL);
    
    if (last_good && !last_cont) {
      sprintf(buf, "drink %s", fname(last_good->name).c_str());
      addCommandToQue(buf);
      return;
    }
    if (last_good && last_cont) {
      sprintf(buf, "%s", last_cont->name);
      add_bars(buf);
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

  if (getTimer() == 10) {
    if (specials.was_in_room == ROOM_NOWHERE &&
        inRoom() != ROOM_NOWHERE && inRoom() != ROOM_STORAGE) {
      specials.was_in_room = in_room;
      if (fight()) {
        specials.fighting->stopFighting();
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

      vlogf(-1, "%s booted from game for inactivity.", getName());
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
forceCrash("does this get called?  checkIdle");

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

  for (t = c->stuff; t; t = t2) {
    t2 = t->nextThing;
    --(*t);

    TObj *tobj = dynamic_cast<TObj *>(t);
    if (tobj && tobj->isObjStat(ITEM_NEWBIE) && !tobj->stuff &&
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

    // adjust for speed
    value = value * plotStat(STAT_CURRENT, STAT_SPE, 0.8, 1.25, 1.0);

    // give at least 1 hit per hand
    // reverse engineering, we realize 2.0 comes around L7.8 = 26%barehand
    // so this is also a newbie normalization
    value = max(value, 2.0);

    setMult(value);
  }
  // going to set this stuff in getImmunity instead, messes stuff up here
#if 0
  if (hasClass(CLASS_MONK) && doesKnowSkill(SKILL_DUFALI)) {
    amount = getSkillValue(SKILL_DUFALI);
    addToImmunity(IMMUNE_PARALYSIS, amount/3);
    addToImmunity(IMMUNE_CHARM, amount);
    addToImmunity(IMMUNE_POISON, amount/2);
  }
#endif
}

// computes the time it takes to get back 1 point of hp/mana/move
// it uses average of the 3 numbers but oh well.
int TBeing::regenTime()
{
  int iTime = hitGain();
  iTime += manaGain();
  iTime += moveGain();
  while (iTime % 3)
    iTime++;
  iTime /= 3;

  iTime = max(iTime, 1);
  iTime = PULSE_UPDATES / iTime;  // PULSE_ is time between calls to
                                // updateHalfTickStuff()
  return iTime;
}

double TBeing::hpGainForLevel(classIndT Class) const
{
  double hpgain = 0;

  // add for classes first
  if (hasClass(CLASS_MAGIC_USER) && Class == MAGE_LEVEL_IND)
    hpgain += (double) ::number(2,8);

  if (hasClass(CLASS_CLERIC) && Class == CLERIC_LEVEL_IND)
    hpgain += (double) ::number(4,8);

  if (hasClass(CLASS_WARRIOR) && Class == WARRIOR_LEVEL_IND)
    hpgain += (double) ::number(5,11);

  if (hasClass(CLASS_THIEF) && Class == THIEF_LEVEL_IND)
    hpgain += (double) ::number(3,9);

  if (hasClass(CLASS_DEIKHAN) && Class == DEIKHAN_LEVEL_IND)
    hpgain += (double) ::number(4,9);

  if (hasClass(CLASS_MONK) && Class == MONK_LEVEL_IND)
    hpgain += (double) ::number(5,9);

  if (hasClass(CLASS_RANGER) && Class == RANGER_LEVEL_IND)
    hpgain += (double) ::number(5,10);

  if (hasClass(CLASS_SHAMAN) && Class == SHAMAN_LEVEL_IND)
    hpgain += (double) ::number(2,8);

  // tack on CON modifier
  hpgain += (double) getConHpModifier();

  hpgain /= (double) howManyClasses();

  return hpgain;
}
