#include <cmath>

#include "extern.h"
#include "being.h"
#include "monster.h"
#include "statistics.h"
#include "obj_component.h"
#include "person.h"
#include "database.h"
#include "spec_mobs.h"
#include "materials.h"
#include "skills.h"

#define REPRAC_COST_PER_PRAC 1000

// if logic changes, please change some of the duplicate code in pracsBetween()
void TBeing::setSpellEligibleToggle(TMonster *trainer, spellNumT spell, silentTypeT silent) 
{
  if (!silent && trainer) {
    trainer->doTell(fname(name), format("You now have the training to learn %s!") % discArray[spell]->name);
  }

  switch (spell) {
    case SPELL_TORNADO:
      if (!silent && trainer) {
        trainer->doTell(fname(name), "Alas, I do not have the knowledge to train you in tornado.");
        trainer->doTell(fname(name), "Seek out the wise elf Salrik to see if you can learn it from him.  I will let him know that I have sent you.");
      }
      setQuestBit(TOG_TORNADO_ELIGIBLE);
      break;

    case SKILL_BARKSKIN:
      if (!silent && trainer) {
        trainer->doTell(fname(name), "However, before I train you in barkskin, I must ask you  to perform a small task to prove your worth.");
	trainer->doTell(fname(name), "In order to prove you are ready for such knowledge, bring me some barkskin.");
      }
      setQuestBit(TOG_ELIGIBLE_BARKSKIN);
      break;
      
    case SPELL_EARTHQUAKE:
      if (!silent && trainer) {
        trainer->doTell(fname(name), "However, before I train you in earthquake, I must ask you to perform a small task to prove your worth.");
        trainer->doTell(fname(name), "In order to prove you are ready for such knowledge, bring me a yellow boot.");
      }
      setQuestBit(TOG_ELIGIBLE_EARTHQUAKE);
      break;
      
    case SKILL_DUAL_WIELD:
      if (!silent && trainer) {
        trainer->doTell(fname(name), "However, before I train you in dual wield, I must ask you to perform a small task to prove your worth.");
        trainer->doTell(fname(name), "In order to prove you are ready for such knowledge, bring me some mandrake.");
      }
      setQuestBit(TOG_ELIGIBLE_DUAL_WIELD);
      break;
      
    case SPELL_FIREBALL:
      if (!silent && trainer) {
        trainer->doTell(fname(name), "Alas, I do not have the knowledge to train you in fireball.");
        trainer->doTell(fname(name), "Seek out the mischevious mage Kallam to see if you can learn it from him.  I will let him know that I have sent you.");
      }
      setQuestBit(TOG_ELIGIBLE_FIREBALL);
      break;

    case SPELL_ICE_STORM:
      if (!silent && trainer) {
        trainer->doTell(fname(name), "Alas, I do not have the knowledge to train you in ice storm.");
        trainer->doTell(fname(name), "Seek out the water mage, Cardac, to see if you can learn it from him.  I will let him know that I have sent you.");
      }
      setQuestBit(TOG_ELIGIBLE_ICE_STORM);
      break;

    case SPELL_STONE_SKIN:
      if (!silent && trainer) {
        trainer->doTell(fname(name), "Alas, I do not have the knowledge to train you in stone skin.");
        trainer->doTell(fname(name), "However, I do remember a dwarf that perhaps has such knowledge of using defensive Earth Magic.");
        trainer->doTell(fname(name), "I think the City in the Clouds has the dwarf you seek.  He is a very important abassador there.");
        trainer->doTell(fname(name), "Seek him out, he may hold the secrets you desire.");
      }
      setQuestBit(TOG_ELIGIBLE_STONESKIN);
      break;

    case SPELL_GALVANIZE:
      if (!silent && trainer) {
        trainer->doTell(fname(name), "Alas, I do not have the knowledge to train you in galvanize.");
        trainer->doTell(fname(name), "Seek out the wizened mage, Fabnir, to see if you can learn it from him.  I will let him know that I have sent you.");
      }
      setQuestBit(TOG_ELIGIBLE_GALVANIZE);
      break;

    case SPELL_POWERSTONE:
      if (!silent && trainer) {
        trainer->doTell(fname(name), "Alas, I do not have the knowledge to train you in powerstone.");
        trainer->doTell(fname(name), "Seek out the wise alchemist, Fabnir, to see if you can learn it from him.  I will let him know that I have sent you.");
      }
      setQuestBit(TOG_ELIGIBLE_POWERSTONE);
      break;

    case SKILL_ADVANCED_KICKING:
      if(!silent && trainer){
	trainer->doTell(fname(name), "Although you are eligible to learn advanced kicking, you must first master kick.");
	trainer->doTell(fname(name), "When you have mastered kick, speak with your guildmaster and I will tell you how to learn advanced kicking.");
      }
      break;

    default:
      return;
  }

  return;
}


// this function determines how many pracs each disc requires
int CalcRaiseDisc(int natLearn, CDiscipline *disc, bool drop)
{
  if (!drop && (natLearn >= MAX_DISC_LEARNEDNESS)) {
    return 0;
  }
  if (drop && (natLearn <= 0)) {
    return 0;
  }

  if(disc->isBasic())
    return 1;
  else if(disc->isFast())
    return min(5, (MAX_DISC_LEARNEDNESS - natLearn));

  // Pappy 12-08-2007
  // Because it is now possible to reset your practices, I am making the number of pracs spent
  // to be deterministic always.  With Battys below formula, the gain of skill given 60 pracs
  // would fluctuate between 123 and 94 because it would treat decimal gains as a percentage to gain 1.
  // This means that often it was possible to get to 100% on an advanced disc with 54 to 65 pracs,
  // given your luck.  I am applying the same formula as was previously used, however I'm making the
  // 'chances' deterministic, so that it always takes 60 pracs to get to 100.  This keeps players
  // from skewing their practices by repeatedly practicing and resetting practices.
  // The formula remains the same.  The number of pracs needed to hit 100 now is always just 60.
  natLearn = min(max(0, natLearn), (int)MAX_DISC_LEARNEDNESS);
  if (natLearn >= MAX_DISC_LEARNEDNESS)
    return 0;
  else if (natLearn <= 15)
    return 3;
  else if (natLearn <= 72)
    return 2;
  else
    return 1;

#ifdef OLDSCHOOL_PRACS
  int L, i_inc;

  // this logic gets a bit involved.  I am writing it all down for
  // posterity and incase someone decides to screw things up later.
  // Batopr 8-6-96

  // we desire a system where the increase in a discs learning is variable.
  // we want the increase to be a lot if the present learning is low, and
  // increase to be low if present learning is high.
  // let p = the number of pracs i have spent in a disc
  // let P = PRACS_TO_MAX
  // f(p) = learning in disc
  // f'(p) = amount of increase
  // for simplicity, let the increase formula be linear
  // f'(p) = Ap + C    A and C constants to be determined.
  // this makes f(p) = A/2 p^2 + C p + K,    K another constant
  // Boundary conditions:
  //    f(0) = 0     learning starts at 0
  //    f(P) = 100   max learnedness in disc is 100 after P pracs spent
  //    f'(P) = 1    this forces a minimum increase to be >= 1

  // f(0)=0   --> K = 0
  // f'(P)=1  --> A = (1-C)/P
  // 3rd boundary cond. + above + a bunch of algebra yields:
  // C = 200/P - 1
  double C = 200./(double) PRACS_TO_MAX - 1.;
  // plugging into A, yields
  // A = 2/P - (200/(P^2))
  double A = (2./(double) PRACS_TO_MAX - (200./(double) PRACS_TO_MAX/(double) PRACS_TO_MAX));

  // we now have a function f'(p) that will tell us how much increase to
  // give.  Unfortunately, this is variable in p (the number of pracs i
  // have spent).  The value we have stored is the present learning in the
  // so we need to work backwards a bit
  // L = f(p) = A/2 p^2 + Cp
  L = natLearn;

  // A/2 p^2 + Cp - L = 0
  // quadratic formula solves this for us for 2 values of p
  // p = (-C +/- sqrt( C^2 - 4*(A/2)*(-L)))/(2*(A/2))

  // one of these p values is > PRACS_TO_MAX so ignore it
  double p = ((double) -C + (double) sqrt( ((double) C)*((double) C) + 2.*(double) A * (double) L))/(double) A;

  // hence p represents the number of pracs it thinks I have spent in the
  // disc based on my present learnedness.

  // use this value of p in the f'(p) formula and we know how much we 
  // should increment for this practice.
  double d_inc = (double) A * p + (double) C;
  i_inc = (int) d_inc;

  // treat the leftover as a chance at an extra point.
  double leftover = d_inc - (double) i_inc;
  if (::number(0,999) < (1000. * leftover))
    i_inc++;

  // prevent overflow
  i_inc = min(i_inc, MAX_DISC_LEARNEDNESS - L);

  return i_inc;
#endif
}

// this function determines how many pracs each disc requires, given this beings learnedness
int TBeing::calcRaiseDisc(discNumT which, bool drop) const
{
  int i_inc = CalcRaiseDisc(getDiscipline(which)->getNatLearnedness(), getDiscipline(which), drop);

  // people report practicing and getting no gain, trap this event.
  if (i_inc <= 0)
    vlogf(LOG_BUG, format("Bad discipline increase - did %s prac and get nothing from it?") % getName());

  return i_inc;
}



// this will return negative if over target
int TBeing::pracsBetween(discNumT which, int target) const
{
  int num;
  int learnedness = getDiscipline(which)->getNatLearnedness();
  target = min(target, (int)MAX_DISC_LEARNEDNESS);

  for (num = 0;learnedness < target;num++)
    learnedness += CalcRaiseDisc(learnedness, getDiscipline(which), false);

  return num;

#ifdef OLDSCHOOL_PRACS
  // see the raiseDisc commentary for what all these numbers mean
  // Batopr 8-6-96

  double C = 200./(double) PRACS_TO_MAX - 1.;
  double A = (2./(double) PRACS_TO_MAX - (200./(double) PRACS_TO_MAX/(double) PRACS_TO_MAX));

  int L = getDiscipline(which)->getNatLearnedness();

  // We made an assumption about the range in teh quadratic formula logic of
  // raiseDisc.  Keep things in this range
  target = min(max(0, target), 100);

  double p = ((double) -C + (double) sqrt( ((double) C)*((double) C) + 2.*(double) A * (double) L))/(double) A;

  // hence p represents the number of pracs it thinks I have spent in the
  // disc based on my present learnedness.

  double p2 = ((double) -C + (double) sqrt( ((double) C)*((double) C) + 2.*(double) A * (double) target))/(double) A;

  // p2 represents the number of pracs I would have to have to get to the
  // target value

  // the diffrence is how many i would have to spend to get from where i am
  // to target
  num = (int) (p2 - p);

  // make sure that if we aren't at target, it takes at least 1 prac.
  if (num <= 0 && L < target)
    num = 1;

  return num;
#endif
}

void TBeing::raiseDiscOnce(discNumT which)
{
  CDiscipline *d;
  d = getDiscipline(which);
  mud_assert(d != NULL, "Bad discipline in raiseDiscOnce");

  int amount = d->getNatLearnedness();

  if (amount >= MAX_DISC_LEARNEDNESS) {
    d->setNatLearnedness(min((int) MAX_DISC_LEARNEDNESS, amount));
    d->setLearnedness(min((int) MAX_DISC_LEARNEDNESS, amount));
    if (isPc() || desc)
      affectTotal();
  }
  max(amount, 0);
  amount += calcRaiseDisc(which, FALSE);
  d->setNatLearnedness(min((int) MAX_DISC_LEARNEDNESS, amount));
  d->setLearnedness(min((int) MAX_DISC_LEARNEDNESS, amount ));
  if (isPc() || desc)
    affectTotal();
}

const char *how_good(int percent)
{
  if (percent <= 0)
    return " (not learned)";
  else if (percent <= 5)
    return " (terrible)";
  else if (percent <= 10)
    return " (horrid)";
  else if (percent <= 15)
    return " (awful)";
  else if (percent <= 20)
    return " (bad)";
  else if (percent <= 25)
    return " (lousy)";
  else if (percent <= 30)
    return " (poor)";
  else if (percent <= 40)
    return " (so-so)";
  else if (percent <= 49)
    return " (average)";
  else if (percent <= 55)
    return " (fair)";
  else if (percent <= 60)
    return " (ok)";
  else if (percent <= 70)
    return " (decent)";
  else if (percent <= 75)
    return " (good)";
  else if (percent <= 80)
    return " (pretty good)";
  else if (percent <= 85)
    return " (very good)";
  else if (percent <= 90)
    return " (extremely good)";
  else if (percent >= MAX_SKILL_LEARNEDNESS)
    return " (maxed)";
  else
    return " (superb)";
}

void give_avenger(TBeing *gm, TBeing *ch)
{
  if (ch->hasClass(CLASS_DEIKHAN)) {
    act("$n says, \"$N, it takes true bravery to reach level 20!\"", FALSE, gm, 0, ch, TO_ROOM);
    act("$n says, \"You have shown yourself worthy of your Holy Avenger.\"", FALSE, gm, 0, ch, TO_ROOM);
    personalize_object(gm, ch, 319, -1);
  }
}

void TPerson::setSelectToggles(TBeing *gm, classIndT Class, silentTypeT silent)
{
  bool bHasQuestAvailable = false;
  char buf[256];

  if (getLevel(Class) >= 8 &&
      !hasQuestBit(TOG_FACTIONS_ELIGIBLE) &&
      isUnaff()) {
    if (!silent) {
      gm->doSay("Your advanced level makes you eligible to join a faction if you so choose.");
      gm->doSay("The mayors of Brightmoon, Logrus and Amber can provide details on their respective faction.");
    }

    if (gm)
      setQuestBit(TOG_FACTIONS_ELIGIBLE);
    else if (getLevel(Class) == 8)
      bHasQuestAvailable = true;
  }

  switch (Class) {
    case DEIKHAN_LEVEL_IND:
    // 1 = avenger eligible, 6 = avenger gotten
      if (getLevel(Class) >= 15 &&
          !hasQuestBit(TOG_AVENGER_ELIGIBLE) &&
          !hasQuestBit(TOG_AVENGER_RULES) &&
          !hasQuestBit(TOG_AVENGER_HUNTING) &&
          !hasQuestBit(TOG_AVENGER_SOLO) &&
          !hasQuestBit(TOG_AVENGER_COMPLETE) &&
          !hasQuestBit(TOG_AVENGER_RECEIVED) &&
          !hasQuestBit(TOG_AVENGER_CHEAT) &&
          !hasQuestBit(TOG_AVENGER_PENANCED)) {
        if (!silent) {
          sprintf(buf, "Congratulations, %s.", getName());
          gm->doSay(buf);
          gm->doSay("Your achievements have earned you the right to quest for a holy avenger.");
          gm->doSay("Seek out the Bishop of Brightmoon and ask him about an avenger quest.");
        }

        if (gm)
          setQuestBit(TOG_AVENGER_ELIGIBLE);
        else if (getLevel(Class) == 15)
          bHasQuestAvailable = true;
      }

      if (getLevel(Class) >= 28 && hasQuestBit(TOG_AVENGER_RECEIVED) &&
          !hasQuestBit(TOG_VINDICATOR_ELIGIBLE) &&
          !hasQuestBit(TOG_VINDICATOR_FOUND_BLACKSMITH) &&
          !hasQuestBit(TOG_VINDICATOR_HUNTING_1) &&
          !hasQuestBit(TOG_VINDICATOR_SOLO_1) &&
          !hasQuestBit(TOG_VINDICATOR_CHEAT) &&
          !hasQuestBit(TOG_VINDICATOR_GOOD_ORE) &&
          !hasQuestBit(TOG_VINDICATOR_START_2) &&
          !hasQuestBit(TOG_VINDICATOR_SEEK_PHOENIX) &&
          !hasQuestBit(TOG_VINDICATOR_RULES_2) &&
          !hasQuestBit(TOG_VINDICATOR_HUNTING_2) &&
          !hasQuestBit(TOG_VINDICATOR_SOLO_2) &&
          !hasQuestBit(TOG_VINDICATOR_CHEAT_2) &&
          !hasQuestBit(TOG_VINDICATOR_GOT_FEATHER) &&
          !hasQuestBit(TOG_VINDICATOR_GOOD_FEATHER)  &&
          !hasQuestBit(TOG_VINDICATOR_COMPLETE) &&
          !hasQuestBit(TOG_VINDICATOR_DISHONOR) &&
          !hasQuestBit(TOG_VINDICATOR_ON_PENANCE) &&
          !hasQuestBit(TOG_VINDICATOR_GOT_BOOK) &&
          !hasQuestBit(TOG_VINDICATOR_PURIFIED)) {
        if (!silent) {
          sprintf(buf, "Congratulations, %s.", getName());
          gm->doSay(buf);
          gm->doSay("Your achievements have earned you the right to quest for a holy vindicator.");
          gm->doSay("You do not have to quest now. You can wait until you have more power.");
          gm->doSay("When you are ready, seek out Fistandantilus the Blacksmith and ask him about getting a holy vindicator made.");
        }

        if (gm)
          setQuestBit(TOG_VINDICATOR_ELIGIBLE);
        else if (getLevel(Class) == 28)
          bHasQuestAvailable = true;
      }
      if (getLevel(Class) >= 45 && hasQuestBit(TOG_VINDICATOR_COMPLETE) &&
          !hasQuestBit(TOG_DEVASTATOR_FIND_BEN) &&
          !hasQuestBit(TOG_DEVASTATOR_TOOK_BRIBE) &&
          !hasQuestBit(TOG_DEVASTATOR_FIND_OPAL) &&
          !hasQuestBit(TOG_DEVASTATOR_DO_RIDDLE) &&
          !hasQuestBit(TOG_DEVASTATOR_KILL_BEN) &&
          !hasQuestBit(TOG_DEVASTATOR_FIND_MED) &&
          !hasQuestBit(TOG_DEVASTATOR_FIND_CRUCIFIX) &&
          !hasQuestBit(TOG_DEVASTATOR_FOUND_CRUCIFIX) &&
          !hasQuestBit(TOG_DEVASTATOR_GAVE_CRUCIFIX) &&
          !hasQuestBit(TOG_DEVASTATOR_KILL_PC) &&
          !hasQuestBit(TOG_DEVASTATOR_GOT_WINE) &&
          !hasQuestBit(TOG_DEVASTATOR_DO_RIDDLE2) &&
          !hasQuestBit(TOG_DEVASTATOR_FIND_SWORD) &&
          !hasQuestBit(TOG_DEVASTATOR_FIND_RING) &&
          !hasQuestBit(TOG_DEVASTATOR_FORFEIT_VIN) &&
          !hasQuestBit(TOG_DEVASTATOR_NODEAL) &&
          !hasQuestBit(TOG_DEVASTATOR_TOOK_DEAL) &&
          !hasQuestBit(TOG_DEVASTATOR_GOT_INFO) &&
          !hasQuestBit(TOG_DEVASTATOR_GET_FLOWER) &&
          !hasQuestBit(TOG_DEVASTATOR_DO_RIDDLE3) &&
          !hasQuestBit(TOG_DEVASTATOR_FIND_LORTO) &&
          !hasQuestBit(TOG_DEVASTATOR_A_LIE) &&
          !hasQuestBit(TOG_DEVASTATOR_FIND_SULT) &&
          !hasQuestBit(TOG_DEVASTATOR_FIND_BARA) &&
          !hasQuestBit(TOG_DEVASTATOR_GAVE_DRESS) &&
          !hasQuestBit(TOG_DEVASTATOR_FIND_SLOTH) &&
          !hasQuestBit(TOG_DEVASTATOR_HAS_DEVASTATOR) &&
          !hasQuestBit(TOG_DEVASTATOR_CHEAT_MISER_BEN) &&
          !hasQuestBit(TOG_DEVASTATOR_CHEAT_SPARTAGUS) &&
          !hasQuestBit(TOG_DEVASTATOR_CHEAT_MARCUS) &&
          !hasQuestBit(TOG_DEVASTATOR_CHEAT_TAILLE) &&
          !hasQuestBit(TOG_DEVASTATOR_CHEAT_ABNOR) &&
          !hasQuestBit(TOG_DEVASTATOR_CHEAT_SULTRESS) &&
          !hasQuestBit(TOG_DEVASTATOR_CHEAT_NESMUM)) {
        if (!silent) {
          sprintf(buf, "Congratulations, %s.", getName());
          gm->doSay(buf);
          gm->doSay("Your achievements have earned you the right to quest for a holy Devastator.");
          gm->doSay("You do not have to quest now. You can wait until you have more power.");
          gm->doSay("When you are ready, seek out Creed and ask him about getting a holy devastator made.");
        }

        if (gm)
          setQuestBit(TOG_DEVESTATOR_ELIGIBLE);
        else if (getLevel(Class) == 45)
          bHasQuestAvailable = true;
      }
      break;

    /////
    case MONK_LEVEL_IND:
      if(!hasQuestBit(TOG_HAS_MONK_WHITE) &&
	 !hasQuestBit(TOG_ELIGIBLE_MONK_WHITE) &&
	 !hasQuestBit(TOG_STARTED_MONK_WHITE) &&
	 getLevel(Class)>=2){
	if(!silent)
	  gm->doSay("You are now eligible to quest for your white belt.  Say \"white belt\" for more information.");

        if (gm)
	  setQuestBit(TOG_ELIGIBLE_MONK_WHITE);
        else if (getLevel(Class) == 2)
          bHasQuestAvailable = true;
      }
      if(hasQuestBit(TOG_HAS_MONK_WHITE) &&
	 !hasQuestBit(TOG_HAS_MONK_YELLOW) &&
	 !hasQuestBit(TOG_ELIGIBLE_MONK_YELLOW) &&
	 !hasQuestBit(TOG_FINISHED_MONK_YELLOW) &&
	 getLevel(Class)>=5){
	if(!silent)
	  gm->doSay("You are now eligible to quest for your yellow sash.  Say \"yellow sash\" for more information.");

        if (gm)
	  setQuestBit(TOG_ELIGIBLE_MONK_YELLOW);
        else if (getLevel(Class) == 5)
          bHasQuestAvailable = true;
      }
      if(hasQuestBit(TOG_HAS_MONK_YELLOW) &&
	 !hasQuestBit(TOG_MONK_PURPLE_ELIGIBLE) &&
	 !hasQuestBit(TOG_MONK_PURPLE_STARTED) &&
	 !hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED1) &&
	 !hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED2) &&
	 !hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED3) &&
	 !hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED4) &&
	 !hasQuestBit(TOG_MONK_PURPLE_FINISHED) &&
	 !hasQuestBit(TOG_MONK_PURPLE_OWNED) &&
	 getLevel(Class)>=15){
	if(!silent)
	  gm->doSay("You are now eligible to quest for your purple sash.  Say \"purple sash\" for more information.");

        if (gm)
	  setQuestBit(TOG_MONK_PURPLE_ELIGIBLE);
        else if (getLevel(Class) == 15)
          bHasQuestAvailable = true;
      }
      if(hasQuestBit(TOG_MONK_PURPLE_OWNED) &&
	 !hasQuestBit(TOG_HAS_MONK_BLUE) &&
	 !hasQuestBit(TOG_ELIGIBLE_MONK_BLUE) &&
	 !hasQuestBit(TOG_STARTED_MONK_BLUE) &&
	 !hasQuestBit(TOG_MONK_KILLED_SHARK) &&
	 !hasQuestBit(TOG_FINISHED_MONK_BLUE) &&
	 getLevel(Class)>=25){
	if(!silent)
	  gm->doSay("You are now eligible to quest for your blue sash.  Say \"blue sash\" for more information.");

        if (gm)
	  setQuestBit(TOG_ELIGIBLE_MONK_BLUE);
        else if (getLevel(Class) == 25)
          bHasQuestAvailable = true;
      }
      if(hasQuestBit(TOG_HAS_MONK_BLUE) &&
	 !hasQuestBit(TOG_MONK_GREEN_ELIGIBLE) &&
	 !hasQuestBit(TOG_MONK_GREEN_STARTED) &&
	 !hasQuestBit(TOG_MONK_GREEN_FALLING) &&
	 !hasQuestBit(TOG_MONK_GREEN_FALLEN) &&
	 !hasQuestBit(TOG_MONK_GREEN_OWNED) &&
	 getLevel(Class)>=35){
	if(!silent)
	  gm->doSay("You are now eligible to quest for your green sash.  Say \"green sash\" for more information.");

        if (gm)
	  setQuestBit(TOG_MONK_GREEN_ELIGIBLE);
        else if (getLevel(Class) == 35)
          bHasQuestAvailable = true;
      }
      if(hasQuestBit(TOG_MONK_GREEN_OWNED) &&
	 !hasQuestBit(TOG_STARTED_MONK_RED) &&
	 !hasQuestBit(TOG_FINISHED_MONK_RED) &&
	 !hasQuestBit(TOG_HAS_MONK_RED) &&
	 getLevel(Class)>=45){
	if(!silent)
	  gm->doSay("You are now eligible to quest for your red sash.  Say \"red sash\" for more information.");

        if (gm)
	  setQuestBit(TOG_MONK_RED_ELIGIBLE);
        else if (getLevel(Class) == 45)
          bHasQuestAvailable = true;
      }
      if(hasQuestBit(TOG_HAS_MONK_RED) &&
	 !hasQuestBit(TOG_MONK_BLACK_STARTED) &&
	 !hasQuestBit(TOG_MONK_BLACK_FINISHED) &&
	 !hasQuestBit(TOG_MONK_BLACK_OWNED) &&
	 getLevel(Class)==50){
	if(!silent)
	  gm->doSay("You are now eligible to quest for your black sash.  Say \"black sash\" for more information.");

        if (gm)
	  setQuestBit(TOG_MONK_BLACK_ELIGIBLE);
        else if (getLevel(Class) == 50)
          bHasQuestAvailable = true;
      }
      break;
    case SHAMAN_LEVEL_IND:
      if (getLevel(Class)==6) {
	if(!silent){
	  gm->doSay("<Y>Your status as a newbie has been removed.<1>");
	  gm->doSay("<B>You will now be subject to penalty if you let<1>");
	  gm->doSay("<B>your lifeforce fall to 0.<1>");
	}
        bHasQuestAvailable = true;
      }
      if (getLevel(Class)>=15 &&
            !hasQuestBit(TOG_ELIGIBLE_JUJU) &&
            !hasQuestBit(TOG_GET_THONG) &&
            !hasQuestBit(TOG_MARE_HIDE) &&
            !hasQuestBit(TOG_GET_BEADS) &&
            !hasQuestBit(TOG_HAS_ORB) &&
            !hasQuestBit(TOG_DONE_JUJU)) {
	if(!silent){
	  gm->doAction(name, CMD_BEAM);
	  gm->doSay("Congratulations! You have earned the right to make a juju bag.");
	  gm->doSay("It will aid in your communications with the loa as well as store components.");
	  gm->doSay("Say 'juju bag' if you want more information on this quest.");
	}

        if (gm)
	  setQuestBit(TOG_ELIGIBLE_JUJU);
        else if (getLevel(Class) == 15)
          bHasQuestAvailable = true;
      }
      if (getLevel(Class)>=30 &&
            !hasQuestBit(TOG_TOTEM_MASK_ELIGIBLE) &&
            !hasQuestBit(TOG_TOTEM_MASK_STARTED) &&
            !hasQuestBit(TOG_TOTEM_MASK_FIND_FORSAKEN) &&
            !hasQuestBit(TOG_TOTEM_MASK_FIND_WOODEN_PLANK) &&
            !hasQuestBit(TOG_TOTEM_MASK_HAS_SAPLESS_WOOD) &&
            !hasQuestBit(TOG_TOTEM_MASK_FIND_SCALED_HIDE) &&
            !hasQuestBit(TOG_TOTEM_MASK_GIVE_COVERED_GONDOLFO) &&
            !hasQuestBit(TOG_TOTEM_MASK_RECOVER_VIAL) &&
            !hasQuestBit(TOG_TOTEM_MASK_FIND_ELRIC_GRIS_GRIS) &&
            !hasQuestBit(TOG_TOTEM_MASK_KILL_BARON_SAMEDI) &&
            !hasQuestBit(TOG_TOTEM_MASK_HAS_BARONS_VISION) &&
            !hasQuestBit(TOG_TOTEM_MASK_KILLED_ELRIC) &&
            !hasQuestBit(TOG_TOTEM_MASK_FACE_TRUE_EVIL) &&
            !hasQuestBit(TOG_TOTEM_MASK_KILLED_FATHERS_SPIRIT) &&
            !hasQuestBit(TOG_TOTEM_MASK_FINISHED)) {
	if(!silent){
	  gm->doAction(name, CMD_BEAM);
          sprintf(buf, "Excellent %s!", getName());
          gm->doSay(buf);
	  gm->doSay("Congratulations! You have learned enough of the ways of the Shaman");
	  gm->doSay("to seek a better way to communicate with the loa!");
	  gm->doSay("In order to do this you will have to seek out a very powerful and");
	  gm->doSay("wise houngan by the name of <Y>Gandolfo<z>. He has the power to speak with");
	  gm->doSay("the dead and can help you in your quest.");
	  gm->doSay("You may of course wait until a time that you feel more confident in your");
          gm->doSay("powers as a practitioner, however it is to your advantage to seek a more");
          gm->doSay("higher power with the aid of Gandolfo.");
	  gm->doSay("It is believed that Gandolfo lives in a small hut in Brazzed-Dum.");
	  gm->doSay("I wish I knew more to help you.");
	}

        if (gm)
	  setQuestBit(TOG_TOTEM_MASK_ELIGIBLE);
        else if (getLevel(Class) == 30)
          bHasQuestAvailable = true;
      }
      break;
    case MAGE_LEVEL_IND:
      if (getLevel(Class)>=10 &&
            !hasQuestBit(TOG_MAGE_BELT_ELIGIBLE) &&
            !hasQuestBit(TOG_MAGE_BELT_STARTED) &&
            !hasQuestBit(TOG_MAGE_BELT_THREAD_HUNT) &&
            !hasQuestBit(TOG_MAGE_BELT_OWNED)) {
	if(!silent){
	  gm->doAction(name, CMD_BEAM);
	  gm->doSay("Congratulations young magius! You have earned the right to make your own magician's belt.");
	  gm->doSay("It will aid in your casting as well as provide a better way to store your spell components.");
	  gm->doSay("When your wizadry is sufficient you will be able to cast while wearing your components");
	  gm->doSay("at your waist.  This task is not required of you and only the most dedicated mage will");
	  gm->doSay("want to attempt it.");
	  gm->doSay("Say 'magician's belt' if you want more information on this quest.");
	}

        if (gm)
	  setQuestBit(TOG_MAGE_BELT_ELIGIBLE);
        else if (getLevel(Class) == 10)
          bHasQuestAvailable = true;
      }
      if (getLevel(Class)>=25 && 
            !hasQuestBit(TOG_ELIGIBLE_MAGE_ROBE) &&
            !hasQuestBit(TOG_MAGE_ROBE_SEEK_DRUID) &&
            !hasQuestBit(TOG_MAGE_ROBE_GET_OIL) &&
            !hasQuestBit(TOG_MAGE_ROBE_GET_SYMBOL) &&
            !hasQuestBit(TOG_MAGE_ROBE_GET_METAL) &&
            !hasQuestBit(TOG_MAGE_ROBE_GET_FABRIC) &&
            !hasQuestBit(TOG_HAS_MAGE_ROBE)) {
        if(!silent){
          gm->doAction(name, CMD_BEAM);
          gm->doSay("You have reached an important phase in your studies.");
          gm->doSay("You may now choose to fashion a mage's robe.");
          gm->doSay("If you wish to undertake this task...");
          gm->doSay("Go see the Spellcrafter in the Mage Academy.");
        }

        if (gm)
          setQuestBit(TOG_ELIGIBLE_MAGE_ROBE);
        else if (getLevel(Class) == 25)
          bHasQuestAvailable = true;
      }
      break;

    case RANGER_LEVEL_IND:
      if (getLevel(Class) >= 7 &&
            !hasQuestBit(TOG_RANGER_FIRST_ELIGIBLE) &&
            !hasQuestBit(TOG_RANGER_FIRST_FOUND_HERMIT) &&
            !hasQuestBit(TOG_RANGER_FIRST_STARTED) &&
            !hasQuestBit(TOG_RANGER_FIRST_GNOBLE) &&
            !hasQuestBit(TOG_RANGER_FIRST_FARMER) &&
            !hasQuestBit(TOG_RANGER_FIRST_CHILDREN) &&
            !hasQuestBit(TOG_RANGER_FIRST_FARMHAND) &&
            !hasQuestBit(TOG_RANGER_FIRST_KILLED_OK) &&
            !hasQuestBit(TOG_RANGER_FIRST_GAVE_HIDE) &&
            !hasQuestBit(TOG_RANGER_FIRST_GAVE_PELT) &&
            !hasQuestBit(TOG_RANGER_FIRST_HUNTING) &&
            !hasQuestBit(TOG_RANGER_FIRST_GOT_SCROLL) &&
            !hasQuestBit(TOG_RANGER_FIRST_FINISHED)) {
	if(!silent){
	  gm->doSay("There is a hermit that lives in the Grimhaven Park.");
	  gm->doSay("I wish for you to go and speak with him, he has a task for you.");
	  gm->doSay("He can be found observing the mallards nest.  Ask him about the <w>ranger's first quest<1>.");
	}

        if (gm)
	  setQuestBit(TOG_RANGER_FIRST_ELIGIBLE);
        else if (getLevel(Class) == 7);
          bHasQuestAvailable = true;
      }
      if (getLevel(Class) >= 14 &&
         hasQuestBit(TOG_RANGER_FIRST_FINISHED) &&
         !hasQuestBit(TOG_ELIGIBLE_RANGER_L14) &&
         !hasQuestBit(TOG_STARTED_RANGER_L14) &&
         !hasQuestBit(TOG_SEEN_KOBOLD_POACHER) &&
         !hasQuestBit(TOG_SEEKING_ORC_POACHER) &&
         !hasQuestBit(TOG_SEEN_ORC_POACHER) &&
         !hasQuestBit(TOG_SEEKING_BONE_WOMAN) &&
         !hasQuestBit(TOG_SEEKING_APPLE) &&
         !hasQuestBit(TOG_GOT_CARVED_BUCKLE) &&
         !hasQuestBit(TOG_SEEKING_ORC_MAGI) &&
         !hasQuestBit(TOG_FAILED_TO_KILL_MAGI) &&
         !hasQuestBit(TOG_PROVING_SELF) &&
         !hasQuestBit(TOG_KILLED_ORC_MAGI) &&
	 !hasQuestBit(TOG_FINISHED_RANGER_L14)) {
	if(!silent){
	  gm->doSay("You are now ready for another quest.");
	  gm->doSay("Seek out Jed the Hermit within the valley that is southwest of Grimhaven.");
	  gm->doSay("Ask him about the <W>hunter's belt<1>.");
	}

        if (gm)
	  setQuestBit(TOG_ELIGIBLE_RANGER_L14);
        else if (getLevel(Class) == 14)
          bHasQuestAvailable = true;
      }
      break;
    case WARRIOR_LEVEL_IND:
      if (getLevel(Class)>=40 && 
          !hasQuestBit(TOG_FINISHED_WARRIOR_L41) &&
          !hasQuestBit(TOG_KILL_SHAMAN) &&
          !hasQuestBit(TOG_KILL_CHIEF) &&
          !hasQuestBit(TOG_GAVE_HEAD_CHIEF) &&
          !hasQuestBit(TOG_ELIGIBLE_WARRIOR_L41)) {
	if(!silent){
	  gm->doSay("Congratulations Warrior!.");
	  gm->doSay("I believe you are ready to obtain your scabbard.");
	  gm->doSay("This task will challenge your skills, but is well worth the risk.");
	  gm->doSay("Only a true champion can wear the scabbard.");
          gm->doSay("Are you the ready to face your greatest test?");
          gm->doSay("Say 'I am ready' if you are ready to undertake this task.");
	}

        if (gm)
	  setQuestBit(TOG_ELIGIBLE_WARRIOR_L41);
        else if (getLevel(Class) == 40)
          bHasQuestAvailable = true;
      }
      break;
    default:
      break;
  }

  if (bHasQuestAvailable)
    sendTo("You have a sinking feeling you need to see your guildmaster over something important...\n\r"
	   "When you find your guildmaster use the \"gain\" command to get the information.\n\r");

  doSave(SILENT_YES);
}

void TPerson::advanceSelectDisciplines(classIndT Class, int numx, silentTypeT silent)
{
  int learnAdd = 0;
  int i, count, initial, final;

  for (i = 0; i < numx; i++) {
    if ((Class == MAGE_LEVEL_IND)) {
      CDiscipline *cd = getDiscipline(DISC_WIZARDRY);
      if (cd) {
        initial = cd->getNatLearnedness();
        if (initial < 100) {
          learnAdd = 2;
          if (plotStat(STAT_NATURAL, STAT_WIS, 30, 180, 105, 1.0) > (::number(0,200))) 
            learnAdd += 1;
          
          for (count = 1; count <= learnAdd; count++) {
            if (cd->getNatLearnedness() < MAX_DISC_LEARNEDNESS)
             raiseDiscOnce(DISC_WIZARDRY);
          }
          final = cd->getNatLearnedness();
          if (!silent) 
            sendTo("You feel your natural wizardry increase.\n\r");

          doLevelSkillsLearn(DISC_WIZARDRY, initial, final);
        }
      }
    }
    if ((Class == SHAMAN_LEVEL_IND)) {
      CDiscipline *cd = getDiscipline(DISC_RITUALISM);
      if (cd) {
        initial = cd->getNatLearnedness();
        if (initial < 100) {
          learnAdd = 2;
          if (plotStat(STAT_NATURAL, STAT_WIS, 30, 180, 105, 1.0) > (::number(0,200))) 
            learnAdd += 1;
          
          for (count = 1; count <= learnAdd; count++) {
            if (cd->getNatLearnedness() < MAX_DISC_LEARNEDNESS)
             raiseDiscOnce(DISC_RITUALISM);
          }
          final = cd->getNatLearnedness();
          if (!silent) 
            sendTo("Your channel to the ancestors grows within you.\n\r");

          doLevelSkillsLearn(DISC_RITUALISM, initial, final);
        }
      }
    }
    if ((Class == CLERIC_LEVEL_IND) || (Class == DEIKHAN_LEVEL_IND)) {
      CDiscipline *cd = getDiscipline(DISC_FAITH);
      if (cd) {
        initial = cd->getNatLearnedness();
        if (initial < 100) {
          learnAdd = 2;
          if (plotStat(STAT_NATURAL, STAT_WIS, 30, 180, 105, 1.0) > (::number(0,200))) 
            learnAdd += 1;
          
          for (count = 1; count <= learnAdd; count++) {
            if(cd->getNatLearnedness() < MAX_DISC_LEARNEDNESS)
              raiseDiscOnce(DISC_FAITH);
          }
          final = cd->getNatLearnedness();
          if (!silent) 
            sendTo("You feel your natural faith increase.\n\r");

          doLevelSkillsLearn(DISC_FAITH, initial, final);
        }
      }
    }
    CDiscipline *cd = getDiscipline(DISC_ADVENTURING);
    if (cd) {
      initial = cd->getNatLearnedness();
      if (initial < 100) {
        learnAdd =  2;
        if (plotStat(STAT_NATURAL, STAT_WIS, 30, 180, 105, 1.0) > (::number(0,200))) {
          learnAdd += 1;
        }
        if (isTripleClass()) {
          if (::number(0,2))
            learnAdd = 0;
        } else if (isDoubleClass()) {
          if (::number(0,1))
            learnAdd = 0;
        }
        if (learnAdd) {
          for (count = 1; count <= learnAdd; count++) {
            if(cd->getNatLearnedness() < MAX_DISC_LEARNEDNESS)
              raiseDiscOnce(DISC_ADVENTURING);
          }
          final = cd->getNatLearnedness();
          if (!silent)
            sendTo("You feel that you have learned more about adventuring.\n\r");

          doLevelSkillsLearn(DISC_ADVENTURING, initial, final);
        }
      }
    }
  }
}


void logPermaDeathLevel(TBeing *ch){
  TDatabase db(DB_SNEEZY);
  db.query("delete from permadeath where name='%s'", ch->name);
  db.query("insert into permadeath (name, level, killer, died) values ('%s', %i, 'no one', 0)", ch->name, ch->GetMaxLevel());
}

void clearPermaDeathLevel(TBeing *ch){
  TDatabase db(DB_SNEEZY);
  db.query("delete from permadeath where name='%s'", ch->name);
}


void TBeing::raiseLevel(classIndT)
{
}

void TMonster::raiseLevel(classIndT Class)
{
  if (getExp() < getExpClassLevel(Class, getLevel(Class) + 1)){
    vlogf(LOG_BUG, format("raiseLevel() called on %s when exp too low") %
	  getName());
    return;
  }

  // set the level
  setLevel(Class, (int) getLevel(Class)+1);
  calcMaxLevel();


  if (desc && desc->original) { // polymorphed, level the PC in storage
    //setLevel(Class, (int) getLevel(Class)+1);
    desc->original->setExp(getExp());
    desc->original->setMaxExp(getMaxExp());
    desc->original->raiseLevel(Class);
  } else { // level as mob
    // set the hit and damage levels
    setHPLevel(getHPLevel()+1);
    setHPFromHPLevel();
    setDamLevel(getDamLevel()+1);
    setACLevel(getACLevel()+1);
    setACFromACLevel();

    assignSkillsClass();
  }
}


void TPerson::raiseLevel(classIndT Class)
{
  int maxhit;

  if (getExp() < getExpClassLevel(Class, getLevel(Class) + 1)){
    vlogf(LOG_BUG, format("raiseLevel() called on %s when exp too low") %  getName());
    return;
  }


  // Advance them in a few select disciplines
  advanceSelectDisciplines(Class, 1, SILENT_NO);

  //The fix statement below set the client who window with correct level
  fixClientPlayerLists(TRUE);

  maxhit=points.maxHit;
  advanceLevel(Class);

  fixClientPlayerLists(FALSE);
  setTitle(false);
    
  if(hasQuestBit(TOG_PERMA_DEATH_CHAR)){
    logPermaDeathLevel(this);
  } else {
    clearPermaDeathLevel(this);
  }


  // keep track of how long it took
  // note, we use "50" instead of a macro due to how the array is declared
  if (getLevel(Class) -1 < 50) {
    long mins;
    struct time_info_data playing_time;

    GameTime::realTimePassed((time(0) - player.time->logon) +
			     player.time->played, 0, &playing_time);

    mins = playing_time.minutes + 
      (playing_time.hours * 60) +
      (playing_time.day * 24 * 60);

    if (mins < 60 && getLevel(Class) > 10) {
      // ignore if being quick-leveled by an imm
    } else {
      stats.time_levels[Class][getLevel(Class) -1] += mins;
      stats.levels[Class][getLevel(Class) -1] += 1;
      save_game_stats();
    }
  }
  doSave(SILENT_YES);
}

void TPerson::doLevelSkillsLearn(discNumT discipline, int initial, int final)
{
  spellNumT i;
  int value, discLearn = 0;
  char buf[256];
  
  discLearn = getDiscipline(discipline)->getNatLearnedness();

  for (i=MIN_SPELL; i < MAX_SKILL; i++) {
    if (hideThisSpell(i))
      continue;

    if ((discArray[i]->disc) != discipline)
      continue;

    if ( (initial < discArray[i]->start) &&
         (final >= discArray[i]->start)) {

      if (discArray[i]->startLearnDo > 0)
        value = min((int) discArray[i]->startLearnDo, discArray[i]->learn);
      else
        value = discArray[i]->learn;

      value = max(value, (int) getRawNatSkillValue(i));
      value = max(value, 1);
      value = min(value, (int) MAX_SKILL_LEARNEDNESS);
      setNatSkillValue(i, value);
      setSkillValue(i,value);
      affectTotal();

      sprintf(buf,"You have just learned %s!",discArray[i]->name);
      act (buf, FALSE, this, 0, NULL, TO_CHAR);
    } else if ((*discArray[i]->name) && (initial >= discArray[i]->start) && 
               !(discArray[i]->toggle && !hasQuestBit(discArray[i]->toggle))) {
      if (discArray[i]->startLearnDo == -1) { // doesnt use learn by doing
        value = discArray[i]->learn * (1 + discLearn - discArray[i]->start);
        value = max(value, (int) getRawNatSkillValue(i));
        value = max(value, 1);
        value = min(value, (int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i,value);
        setSkillValue(i, value);
        affectTotal();
      } else {
        if (discArray[i]->startLearnDo > 0) {
          if (getRawSkillValue(i) >= discArray[i]->startLearnDo) {
          // do nothing
          } else if (getRawSkillValue(i) >= getMaxSkillValue(i)) {
            // do nothing
          } else {
            value = min(discArray[i]->startLearnDo, getMaxSkillValue(i));
            value = max(value, (int) getRawNatSkillValue(i));
            value = max(value, 1);
            value = min(value, (int) MAX_SKILL_LEARNEDNESS);
            setNatSkillValue(i, value);
            setSkillValue(i, value);
            affectTotal();
          }
        } else {
          // do nothing
        }
      }
    }
  }
}

int generic_trainer_stuff(TBeing *me, const TBeing *ch)
{
  if (ch->checkSoundproof()) {
    act("$n waves $s hands near $s mouth, trying to say something to $N.",
         FALSE, me, 0, ch, TO_NOTVICT);
    act("$n waves $s hands near $s mouth, trying to say something to you.",
         FALSE, me, 0, ch, TO_VICT);
    return TRUE;
  }
  if (ch->getPosition() <= POSITION_SITTING) {
    me->doSay("How can I train you if you don't stand up?");
    return TRUE;
  }
  if (!ch->isPc() || !ch->desc) {
    if (ch != me) {
      act("$n laughs at $N.",
          FALSE, me, 0, ch, TO_NOTVICT);
      act("$n laughs at you.",
          FALSE, me, 0, ch, TO_VICT);
      me->doSay("You're just a silly mob.");
    }
    return TRUE;
  }

  if (ch->desc->original) {
// should prevent poly type things
    me->doSay("I can not train you in that form.");
    return TRUE;
  }
  return FALSE;
}

struct TRAININFO {
  int spec;
  char abbrev[60];
  char art[60];
  discNumT disc;
  unsigned short int accclass;
};

TRAININFO TrainerInfo[] =
{
  {SPEC_TRAINER_AIR, "air", "the art of Air Magic", DISC_AIR, CLASS_MAGE},
  {SPEC_TRAINER_ALCHEMY, "alchemy", "the art of Alchemy", DISC_ALCHEMY, CLASS_MAGE},
  {SPEC_TRAINER_EARTH, "earth", "the art of Earth Magic", DISC_EARTH, CLASS_MAGE},
  {SPEC_TRAINER_FIRE, "fire", "the art of Fire Magic", DISC_FIRE, CLASS_MAGE},
  {SPEC_TRAINER_SORCERY, "sorcery", "the art of Sorcery", DISC_SORCERY, CLASS_MAGE},
  {SPEC_TRAINER_SPIRIT, "spirit", "the art of Spirit Magic", DISC_SPIRIT, CLASS_MAGE},
  {SPEC_TRAINER_WATER, "water", "the art of Water Magic", DISC_WATER, CLASS_MAGE},
  {SPEC_TRAINER_WRATH, "wrath", "the Wrath of the Deities", DISC_WRATH, CLASS_CLERIC},
  {SPEC_TRAINER_AFFLICTIONS, "afflictions", "the Art of Afflictions", DISC_AFFLICTIONS, CLASS_CLERIC},
  {SPEC_TRAINER_CURE, "cures", "the Healing Arts", DISC_CURES, CLASS_CLERIC},
  {SPEC_TRAINER_HAND_OF_GOD, "hand", "the Hand of the Deities", DISC_HAND_OF_GOD, CLASS_CLERIC},
  {SPEC_TRAINER_RANGER, "ranger", "the Ways of the Ranger", DISC_RANGER, CLASS_RANGER},
  {SPEC_TRAINER_LOOTING, "looting", "Looting and Plundering", DISC_LOOTING, CLASS_THIEF},
  {SPEC_TRAINER_MURDER, "murder", "about Deadly Murder", DISC_MURDER, CLASS_THIEF},
  {SPEC_TRAINER_DUELING, "dueling", "Dueling", DISC_DUELING, CLASS_WARRIOR},
  {SPEC_TRAINER_ADVENTURING, "adventuring", "Adventurers' Lore", DISC_ADVENTURING, CLASS_MAGE | CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR | CLASS_MONK | CLASS_RANGER | CLASS_DEIKHAN | CLASS_SHAMAN},
  {SPEC_TRAINER_COMBAT, "combat", "Combat Skills", DISC_COMBAT, CLASS_MAGE | CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR | CLASS_MONK | CLASS_RANGER | CLASS_DEIKHAN | CLASS_SHAMAN},
  {SPEC_TRAINER_WARRIOR, "warrior", "the Ways of the Warrior", DISC_WARRIOR, CLASS_WARRIOR},
  {SPEC_TRAINER_WIZARDRY, "wizardry", "Wizardry", DISC_WIZARDRY, CLASS_MAGE},
  {SPEC_TRAINER_FAITH, "faith", "Faith", DISC_FAITH, CLASS_CLERIC | CLASS_DEIKHAN},
  {SPEC_TRAINER_SLASH, "slash", "Slash Specialization", DISC_SLASH, CLASS_WARRIOR | CLASS_RANGER | CLASS_THIEF | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_SHAMAN},
  {SPEC_TRAINER_BLUNT, "blunt", "Blunt Specialization", DISC_BLUNT, CLASS_WARRIOR | CLASS_RANGER | CLASS_THIEF | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_SHAMAN},
  {SPEC_TRAINER_PIERCE, "pierce", "Pierce Specialization", DISC_PIERCE, CLASS_WARRIOR | CLASS_RANGER | CLASS_THIEF | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_SHAMAN},
  {SPEC_TRAINER_RANGED, "ranged", "Ranged Specialization", DISC_RANGED, CLASS_WARRIOR | CLASS_RANGER | CLASS_THIEF | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_SHAMAN},
  {SPEC_TRAINER_DEIKHAN, "deikhan", "the Ways of the Deikhan", DISC_DEIKHAN, CLASS_DEIKHAN},
  {SPEC_TRAINER_BRAWLING, "brawling", "Brawling Moves", DISC_BRAWLING, CLASS_WARRIOR},
  {SPEC_TRAINER_MEDITATION_MONK, "meditation", "about Meditation", DISC_MEDITATION_MONK, CLASS_MONK},
  {SPEC_TRAINER_SURVIVAL, "bogus", "bogus", DISC_BOGUS2, CLASS_RANGER},
  {SPEC_TRAINER_SHAMAN_ARMADILLO, "armadillo", "about the Abilities of the Armadillo", DISC_SHAMAN_ARMADILLO, CLASS_SHAMAN},
  {SPEC_TRAINER_ANIMAL, "animal", "Animal Magic", DISC_ANIMAL, CLASS_RANGER},
  {SPEC_TRAINER_AEGIS, "aegis", "the Aegis of the Deities", DISC_AEGIS, CLASS_CLERIC},
  {SPEC_TRAINER_SHAMAN, "shaman", "The Ways of the Shaman", DISC_SHAMAN, CLASS_SHAMAN},
  {SPEC_TRAINER_MAGE, "mage", "the arts of Magic", DISC_MAGE, CLASS_MAGE},
  {SPEC_TRAINER_MONK, "monk", "the ways of the Monk", DISC_MONK, CLASS_MONK},
  {SPEC_TRAINER_CLERIC, "cleric", "the Ways of the Cleric", DISC_CLERIC, CLASS_CLERIC},
  {SPEC_TRAINER_THIEF, "thief", "the Ways of the Thief", DISC_THIEF, CLASS_THIEF},
  {SPEC_TRAINER_PLANTS, "plants", "about Plants and Herbs", DISC_PLANTS, CLASS_RANGER},
  {SPEC_TRAINER_SOLDIERING, "soldiering", "Soldiering Skills", DISC_SOLDIERING, CLASS_WARRIOR},
  {SPEC_TRAINER_BLACKSMITHING, "blacksmithing", "Blacksmithing Skills", DISC_BLACKSMITHING, CLASS_WARRIOR},
  {SPEC_TRAINER_DEIKHAN_FIGHT, "fighting", "Fighting Skills for Deikhans", DISC_DEIKHAN_FIGHT, CLASS_DEIKHAN},
  {SPEC_TRAINER_MOUNTED, "mounted", "about Mounted Abilities", DISC_MOUNTED, CLASS_DEIKHAN},
  {SPEC_TRAINER_DEIKHAN_AEGIS, "aegis", "Aegis Spells for Deikhans", DISC_DEIKHAN_AEGIS, CLASS_DEIKHAN},
  {SPEC_TRAINER_DEIKHAN_CURES, "cures", "Cure Spells for Deikhans", DISC_DEIKHAN_CURES, CLASS_DEIKHAN},
  {SPEC_TRAINER_DEIKHAN_WRATH, "wrath", "Wrath Spells for Deikhans", DISC_DEIKHAN_WRATH, CLASS_DEIKHAN},
  {SPEC_TRAINER_LEVERAGE, "leverage", "about Balance and Leverage", DISC_LEVERAGE, CLASS_MONK},
  {SPEC_TRAINER_MINDBODY, "mindbody", "about Mind and Body Control", DISC_MINDBODY, CLASS_MONK},
  //  {129, "judoki", "about Judoki", DISC_JUDOKI, CLASS_MONK},
  //  {130, "kuksoki", "about Kuksoki", DISC_KUKSOKI, CLASS_MONK},
  {SPEC_TRAINER_FOCUSED_ATTACKS, "focused", "about Focused Attacks", DISC_FOCUSED_ATTACKS, CLASS_MONK},
  //  {132, "ninjoki", "about Ningjoki", DISC_NINJOKI, CLASS_MONK},
  {SPEC_TRAINER_BAREHAND, "barehand", "about Barehand Specialization", DISC_BAREHAND, CLASS_MONK},
  {SPEC_TRAINER_THIEF_FIGHT, "fighting", "Fighting Skills for Thieves", DISC_THIEF_FIGHT, CLASS_THIEF},
  {SPEC_TRAINER_POISONS, "poisons", "about Poisons", DISC_POISONS, CLASS_THIEF},
  {SPEC_TRAINER_SHAMAN_FROG, "frog", "about the Abilities of the Frog", DISC_SHAMAN_FROG, CLASS_SHAMAN},
  {SPEC_TRAINER_SHAMAN_ALCHEMY, "alchemy", "Alchemy Spells for Shamans", DISC_SHAMAN_ALCHEMY, CLASS_SHAMAN},
  {SPEC_TRAINER_SHAMAN_SKUNK, "skunk", "about the Abilities of the Skunk", DISC_SHAMAN_SKUNK, CLASS_SHAMAN},
  {SPEC_TRAINER_SHAMAN_SPIDER, "spider", "about the Abilities of the Spider", DISC_SHAMAN_SPIDER, CLASS_SHAMAN},
  {SPEC_TRAINER_SHAMAN_CONTROL, "control", "about Being Controling", DISC_SHAMAN_CONTROL, CLASS_SHAMAN},
  {SPEC_TRAINER_RITUALISM, "ritualism", "about Rituals", DISC_RITUALISM, CLASS_SHAMAN},
  {SPEC_PALADIN_PATROL, "bogus", "bogus", DISC_BOGUS1, 0},
  {SPEC_TRAINER_STEALTH, "stealth", "about Stealthiness", DISC_STEALTH, CLASS_THIEF},
  {SPEC_TRAINER_TRAPS, "traps", "about Locks and Traps", DISC_TRAPS, CLASS_THIEF},
  {SPEC_TRAINER_LORE, "lore", "about Magic Lores", DISC_LORE, CLASS_MAGE},
 {SPEC_TRAINER_THEOLOGY, "theology", "about Theology", DISC_THEOLOGY, CLASS_CLERIC | CLASS_DEIKHAN},
  {SPEC_TRAINER_DEFENSE, "defense", "about Defense", DISC_DEFENSE, CLASS_WARRIOR | CLASS_RANGER | CLASS_DEIKHAN | CLASS_MONK},
  {SPEC_TRAINER_PSIONICS, "psionics", "about psionics", DISC_PSIONICS, CLASS_WARRIOR | CLASS_RANGER | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_THIEF | CLASS_SHAMAN},
  {SPEC_TRAINER_SHAMAN_HEALING, "healing", "about healing abilities for shaman", DISC_SHAMAN_HEALING, CLASS_SHAMAN},
  {SPEC_TRAINER_IRON_BODY, "iron body", "about iron body techniques", DISC_IRON_BODY, CLASS_MONK},
  {SPEC_TRAINER_ADV_ADVENTURING, "advanced adventuring", "about advanced adventuring", DISC_ADVANCED_ADVENTURING, CLASS_WARRIOR | CLASS_RANGER | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_THIEF | CLASS_SHAMAN},
  {-1}          /* required terminator */
};


int CDGenericTrainer(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  int offset = 0;
  int pracs = 0, practices =0 ;
  bool doneBas = FALSE;
  char buf[160];
  char pracbuf[128];
  char classbuf[128];
  char discbuf[128];
  int var;
  discNumT discipline;
  classIndT accclass=(classIndT)-1;

  if (cmd == CMD_GENERIC_CREATED && me){
    if(me->GetMaxLevel() != 60 &&
       me->GetMaxLevel() != 100){
      // trainers should be level 60 or level 100
      // this determines the maximum amount they can train, and
      // basic trainers go to 60%, second trainers to 100%
      vlogf(LOG_LOW, format("%s: trainer with inappropriate level (%i)") %
	    me->getName() % me->GetMaxLevel());
    }
  }

  if (cmd == CMD_GENERIC_PULSE)
    dynamic_cast<TMonster *>(ch)->aiMaintainCalm();

  if (cmd != CMD_PRACTICE)
    return FALSE;

  for (offset=0; ; offset++) {
    if (me->spec == TrainerInfo[offset].spec) {
      discipline = TrainerInfo[offset].disc;
      break;
    }
    if (TrainerInfo[offset].spec == -1) {
      vlogf(LOG_BUG, format("TrainerMob lacked setup in TrainerInfo array (%s)") %  
             me->getName());
      return FALSE;
    }
  }

  arg = one_argument(arg, discbuf, cElements(discbuf));
  if (!discbuf || !*discbuf || !is_abbrev(discbuf , TrainerInfo[offset].abbrev)) {
    sprintf(buf, "I teach %s.", TrainerInfo[offset].art);
    me->doSay(buf);
    sprintf(buf,
         "Type \"practice %s <number>\" to learn this discipline.", 
         TrainerInfo[offset].abbrev);
    me->doSay(buf);
    return FALSE;
  }
  arg = one_argument(arg, pracbuf, cElements(pracbuf));
  if (!*pracbuf || !(pracs = convertTo<int>(pracbuf))) {
    sprintf(buf,
         "Type \"practice %s <number>\" to learn this discipline.", 
         TrainerInfo[offset].abbrev);
    me->doSay(buf);
    return FALSE;
  }
  if (generic_trainer_stuff(me, ch))
    return TRUE;

  if (pracs <= 0) {
    act("$n growls, \"Don't tick me off, $N.  You won't like me when I'm mad.\"", TRUE, me, 0, ch, TO_ROOM);
    return TRUE;
  }
  arg = one_argument(arg, classbuf, cElements(classbuf));

  if (!IS_SET(TrainerInfo[offset].accclass, ch->getClass())) {
    /* none of my classes match up */
    act("$n growls, \"Go away, $N.\"", FALSE, me, 0, ch, TO_ROOM);
    act("$n growls, \"I will not teach you the secrets of our masters!\"", 
                 FALSE, me, 0, ch, TO_ROOM);
    return TRUE;
  }
  /* next, see if I only have one of the appropriate classes */
  var = (ch->getClass() & TrainerInfo[offset].accclass);

  if (var == CLASS_WARRIOR)
      accclass = WARRIOR_LEVEL_IND;
  else if (var == CLASS_THIEF)
      accclass = THIEF_LEVEL_IND;
  else if (var == CLASS_CLERIC)
      accclass = CLERIC_LEVEL_IND;
  else if (var == CLASS_MAGE)
      accclass = MAGE_LEVEL_IND;
  else if (var == CLASS_DEIKHAN)
      accclass = DEIKHAN_LEVEL_IND;
  else if (var == CLASS_RANGER)
      accclass = RANGER_LEVEL_IND;
  else if (var == CLASS_MONK)
      accclass = MONK_LEVEL_IND;
  else if (var == CLASS_SHAMAN)
      accclass = SHAMAN_LEVEL_IND;
  else if (!*classbuf) {
    /* more than 1 class is appropriate, user needs to specify */
    me->doTell(fname(ch->name), "You need to specify a class.");
    sprintf(buf,
         "Type \"practice %s <number>\" to learn this discipline.", 
         TrainerInfo[offset].abbrev);
    me->doSay(buf);
    return TRUE;
  } else {
    for(int i=0;i<MAX_CLASSES;++i){
      if(is_abbrev(classbuf, classInfo[i].name) &&
	 ch->hasClass(classInfo[i].class_num)){
	accclass = classInfo[i].class_lev_num;
	break;
      }
    }

    if(accclass==-1) {
      act("$n growls, \"Get real, $N. I'm not an idiot!\"", 
                   FALSE, me, 0, ch, TO_ROOM);
      return TRUE;
    }
  }
  if (!((1<<accclass) & TrainerInfo[offset].accclass)) {
    act("$n growls, \"Go away, $N.\"", FALSE, me, 0, ch, TO_ROOM);
    act("$n growls, \"I will not teach you the secrets of our masters!\"", FALSE, me, 0, ch, TO_ROOM);
    return TRUE;
  }

// First set doneBasic and check for it
  doneBas = ch->checkDoneBasic(ch, accclass, FALSE, FALSE);

// Get how many practices the trainer himself will allow
  practices = ch->getTrainerPracs(ch, me, accclass, discipline, pracs);

// Make sure Im not maxxed, other small checks
  if (ch->checkTrainDeny(ch, me, discipline, min(practices, pracs))) 
    return TRUE;

  if (ch->checkForPreReqs(ch, me, discipline, accclass, doneBas, min(practices, pracs))) {
    if (practices <= 0) {
      me->doTell(fname(ch->name), "I also would not be able to train you further in this discipline.");
    }
    return TRUE;
  }
 
  if (practices <= 0) {
    me->doTell(fname(ch->name), "You have come far.  I can train you no farther in this discipline.");
    me->doTell(fname(ch->name), "You must find another master who can further your training.");
    return TRUE;
  }

  // set the number they actually have as another limiting factor
  practices = min((int) ch->getPracs(accclass), practices);
  if (practices <= 0) {
    me->doTell(fname(ch->name), "You have no more practices you can use here.");
    return TRUE;
  } else if (practices < pracs) {
      me->doTell(fname(ch->name), format("I will only be able to use %d of your requested practices.") % practices);
  }
  if (ch->doTraining(ch, me, accclass, offset, min(practices, pracs))) 
    return TRUE;
  
  return TRUE;
}
  
int TBeing::getCombatPrereqNumber(classIndT accclass) const
{
  switch (accclass) {
    default:
      return MAX_DISC_LEARNEDNESS;
  }
  return MAX_DISC_LEARNEDNESS;
}

int TBeing::checkDoneBasic(TBeing *ch, classIndT accclass, int guild, int amountCheck)
{
  int combat = 0;
  int bas = 0;

  if (!ch)
    ch = this;

  switch (accclass) {
    case MAGE_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness() + ch->getDiscipline(DISC_LORE)->getNatLearnedness();
      bas = ch->getDiscipline(DISC_MAGE)->getNatLearnedness();
      break;
    case SHAMAN_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
      bas = ch->getDiscipline(DISC_SHAMAN)->getNatLearnedness();
      break;
    case RANGER_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
      bas = ch->getDiscipline(DISC_RANGER)->getNatLearnedness();
      break;
   case CLERIC_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness() + ch->getDiscipline(DISC_THEOLOGY)->getNatLearnedness();
      bas = ch->getDiscipline(DISC_CLERIC)->getNatLearnedness();
      break;
    case DEIKHAN_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness() + ch->getDiscipline(DISC_THEOLOGY)->getNatLearnedness();
      bas = ch->getDiscipline(DISC_DEIKHAN)->getNatLearnedness();
      break;
    case WARRIOR_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
      bas = ch->getDiscipline(DISC_WARRIOR)->getNatLearnedness();
      break;
    case MONK_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
      bas = ch->getDiscipline(DISC_MONK)->getNatLearnedness();
      break;
    case THIEF_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
      bas = ch->getDiscipline(DISC_THIEF)->getNatLearnedness();
      break;
    default:
      vlogf(LOG_BUG,format("Wierd case in checkDoneBasic %d") %  accclass);
  }

  if (amountCheck) {
    return (min(combat, (int) getCombatPrereqNumber(accclass)) + min(bas, (int) MAX_DISC_LEARNEDNESS));
  }
  if (ch->player.doneBasic[accclass]) {
    return FALSE;
  }
  if ((combat >= getCombatPrereqNumber(accclass)) && (bas >= MAX_DISC_LEARNEDNESS)) {
    if (!ch->player.doneBasic[accclass]) {
      ch->player.doneBasic[accclass] = ch->getLevel(accclass);
      if (guild) 
        return 4;
    }
    return FALSE;
  } else if (bas >= MAX_DISC_LEARNEDNESS) {
    return 1;
  } else if (combat >= getCombatPrereqNumber(accclass)) {
    return 2;
  } else {
    return 3;
  }   
  return 3;
}
  
int TBeing::getTrainerPracs(const TBeing *ch, const TMonster *me, classIndT accclass, discNumT discipline, int pracs) const
{
  return ch->pracsBetween(discipline, me->GetMaxLevel());

  /*
  int trainLevel = 0, discLearn = 0;

  trainLevel = me->GetMaxLevel();
  discLearn = ch->getDiscipline(discipline)->getNatLearnedness();

  if ((discipline == DISC_COMBAT) || (discipline == DISC_LORE) ||
      (discipline == DISC_THEOLOGY)) {
    bakpracs = trainLevel - discLearn;
  } else if (discipline == DISC_SHAMAN || discipline == DISC_MAGE || 
             discipline == DISC_CLERIC || discipline == DISC_WARRIOR ||
             discipline == DISC_RANGER || discipline == DISC_DEIKHAN ||
             discipline == DISC_MONK || discipline == DISC_THIEF) {
    bakpracs = trainLevel - discLearn;
  } else if (((discipline == DISC_SLASH) || (discipline == DISC_PIERCE) ||
              (discipline == DISC_BLUNT) || (discipline == DISC_RANGED) ||
	      (discipline == DISC_BAREHAND)|| (discipline == DISC_DEFENSE) ||
	      (discipline == DISC_PSIONICS) ||
	      (discipline == DISC_ADVANCED_ADVENTURING)) &&
              (ch->getDiscipline(DISC_COMBAT)->getNatLearnedness() < MAX_DISC_LEARNEDNESS)) {
    if (trainLevel == discLearn) 
      bakpracs = 0;
    else 
      bakpracs = max(1, ((trainLevel - discLearn) / 5));
  } else {
    bakpracs = ch->pracsBetween(discipline, me->GetMaxLevel());
  }
  return bakpracs;*/
}

int TBeing::checkTrainDeny(const TBeing *ch, TMonster *me, discNumT discipline, int pracs) const
{

  if ((ch->getDiscipline(discipline))->getNatLearnedness() >= MAX_DISC_LEARNEDNESS) {
    me->doTell(fname(ch->name), "You are already fully learned in this discipline.");
    return TRUE;
  }
  if ((ch->getDiscipline(discipline))->getNatLearnedness() + pracs > MAX_DISC_LEARNEDNESS) {
    ch->sendTo("You cannot practice that many times!\n\r");
    return TRUE;
  }
  return FALSE;
}

int TBeing::checkForPreReqs(const TBeing *ch, TMonster *me, discNumT discipline, classIndT accclass, int prereqs, int pracs) const
{
  char buf[256];
  sstring tmp_buf;
  bool found = FALSE;
  int combat = 0;
  bool combatLearn = FALSE;
  int WEAPON_GAIN_LEARNEDNESS = 92;
  pracs = 1;

 if (discipline == DISC_BAREHAND) {
   if (ch->getRawNatSkillValue(SKILL_BAREHAND_PROF) < WEAPON_GAIN_LEARNEDNESS) {
     me->doTell(fname(ch->name), "You aren't proficient enough yet.");
     return TRUE;
   }
 }
 if (discipline == DISC_SLASH) {
   if (ch->getRawNatSkillValue(SKILL_SLASH_PROF) < WEAPON_GAIN_LEARNEDNESS) {
      me->doTell(fname(ch->name), "You aren't proficient enough yet.");
      return TRUE;
    }
  }
  if (discipline == DISC_PIERCE) {
    if (ch->getRawNatSkillValue(SKILL_PIERCE_PROF) < WEAPON_GAIN_LEARNEDNESS) {
      me->doTell(fname(ch->name), "You aren't proficient enough yet.");
      return TRUE;
    }
  }
  if (discipline == DISC_BLUNT) {
    if (ch->getRawNatSkillValue(SKILL_BLUNT_PROF) < WEAPON_GAIN_LEARNEDNESS) {
      me->doTell(fname(ch->name), "You aren't proficient enough yet.");
      return TRUE;
    }
  }
  if (discipline == DISC_RANGED) {
    if (ch->getRawNatSkillValue(SKILL_RANGED_PROF) < WEAPON_GAIN_LEARNEDNESS) {
      me->doTell(fname(ch->name), "You aren't proficient enough yet.");
      return TRUE;
    }
  }
  if (discipline == DISC_DEFENSE) {
    if (ch->getRawNatSkillValue(SKILL_DEFENSE) < WEAPON_GAIN_LEARNEDNESS) {
      me->doTell(fname(ch->name), "You aren't proficient enough yet.");
      return TRUE;
    }
  }
  if (discipline == DISC_ADVANCED_ADVENTURING) {
    if (ch->getDiscipline(DISC_ADVENTURING)->getNatLearnedness() < WEAPON_GAIN_LEARNEDNESS) {
      me->doTell(fname(ch->name), "You aren't proficient enough yet.");
      return TRUE;
    }
  }
  if (discipline == DISC_PSIONICS) {
    if (!ch->hasQuestBit(TOG_PSIONICIST)){
      me->doTell(fname(ch->name), "You do not have the ability to learn psionics.");
      return TRUE;
    }

  }

  if (!prereqs)
    return FALSE;

  // all classes uses combat as a base requirement
  combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
  tmp_buf = sstring(discNames[DISC_COMBAT].name).cap();
  
  if(classInfo[accclass].sec_disc != DISC_NONE){
    combat+=getDiscipline(classInfo[accclass].sec_disc)->getNatLearnedness();
    tmp_buf += " or ";
    tmp_buf += sstring(discNames[classInfo[accclass].sec_disc].name).cap();
  }


  if ((combat >= 100) ||
      (ch->getLevel(accclass) < 3) ||
      (combat >= (((35*ch->getLevel(accclass)) /10) - 4))) {
    combatLearn = TRUE;
  }


  if (discipline == DISC_COMBAT || 
                  discipline == DISC_LORE || 
                  discipline == DISC_THEOLOGY ||
                  discipline == DISC_FAITH ||
                  discipline == DISC_WIZARDRY ||
                  discipline == DISC_RITUALISM ||
  // No restrictions on DISC_COMBAT and EQUIVALENTs
                  discipline == DISC_SLASH || 
                  discipline == DISC_BLUNT || 
                  discipline == DISC_PIERCE || 
                  discipline == DISC_RANGED ||
                  discipline == DISC_BAREHAND ||
                  discipline == DISC_DEFENSE ||
                  discipline == DISC_ADVENTURING ||
                  discipline == DISC_ADVANCED_ADVENTURING){
    // No restrictions on these disciplines if prof maxxed see first checks
    return FALSE;
  } else {  // needs basic skills for class
    for(classIndT i=MIN_CLASS_IND;i<MAX_CLASSES;i++){
      if(accclass == i){
	if(discipline == classInfo[i].base_disc){
	  if(combatLearn)
	    return FALSE;
	  else
	    found = 1;
	} else {
	  found = 2;
	}
      }
    }
    
    if(!found){
      vlogf(LOG_BUG, format("Bad case in gaining pre requisites (%d) (%s)") %  accclass % ch->getName());
      ch->sendTo("Bug that you got this at the gain trainer.");
      return TRUE;
    }
  }

  switch (found) {
    case 2:
      if (combat >= MAX_DISC_LEARNEDNESS) {
        me->doTell(fname(ch->name), "Tsk! Tsk! You have not kept up with your basic training and you expect advanced learning.");
        sprintf(buf, "Hmmm. I think you should learn more from your %s trainer.", me->getProfName().c_str());
      } else if (!combatLearn) {
        me->doTell(fname(ch->name), "Tsk! Tsk! You have not kept up with your basic training and you expect advanced learning.");
        sprintf(buf, "Hmmm. I think you should learn more from the %s trainer.", tmp_buf.c_str());
      } else {
        me->doTell(fname(ch->name), "Tsk! Tsk! You have not finished any of your basic training and you expect advanced learning.");
        sprintf(buf, "Hmmm. I think you should learn more from one of your basic trainers.");
      }
      me->doTell(fname(ch->name), buf);
      return TRUE;
    case 1:
      me->doTell(fname(ch->name), "Tsk! Tsk! You have not kept up with your general training and you expect me to teach you more.");
      me->doTell(fname(ch->name), format("Go learn more about %s before you come back to me.") % tmp_buf);
      return TRUE;
  }
  return FALSE;
}

extern struct PolyType DisguiseList[];
static const int MaxDisguiseType = 18; // Non-Race Specific Ones

extern struct PolyType ShapeShiftList[];
static const int MaxShapeShiftType = 18; // Non-Race Specific Ones

int TBeing::doTraining(TBeing *ch, TMonster *me, classIndT accclass, int offset, int pracs) const
{
  int j;
  spellNumT i;
  char buf[256];
  discNumT discipline = DISC_NONE;
  int final, initial;
  int value;
  int tOldSL = ch->getDiscipline(getDisciplineNumber(SKILL_DISGUISE, FALSE))->getLearnedness();
  int tOldSL2 = ch->getDiscipline(getDisciplineNumber(SPELL_SHAPESHIFT, FALSE))->getLearnedness();

  if (pracs <= 0) {
    vlogf(LOG_BUG, format("Bogus pracs used %s (%d)") %  ch->getName() % ch->in_room);
    return TRUE;
  }

  discipline = TrainerInfo[offset].disc;

  act ("$n places both $s hands on your head for a second.", FALSE, me, 0, ch, TO_VICT);
  act ("$n places both $s hands on $N's head for a second.", FALSE, me, 0, ch, TO_NOTVICT);

  for (j = 0;j < pracs; j++) {
    int bump = 0;
    // since things went up > 1% per prac, verify we didn't hit any hard limits
    // basic level check
    if ((ch->getDiscipline(TrainerInfo[offset].disc))->getNatLearnedness() >=
         me->GetMaxLevel()) {
       me->doTell(fname(ch->name), "I can train you no farther in this discipline.");
       me->doTell(fname(ch->name), "You must find another master who can further your training.");

      break;
    }
    // basic maxed check
    if ((ch->getDiscipline(TrainerInfo[offset].disc))->getNatLearnedness() >=
         MAX_DISC_LEARNEDNESS) {
      me->doTell(fname(ch->name), "You are now fully trained in this discipline.");
      break;
    }

    if (ch->getPracs(accclass) <= 0) {
      act("$n says, \"$N, now is not the time for training.\"",
                FALSE, me, 0, ch, TO_ROOM);
      act("$n says, \"Now is the time for action!\"", 
                FALSE, me, 0, ch, TO_ROOM);
      return TRUE;
    } else {
      ch->addPracs(-1, accclass);
      ch->sendTo(format("You have %d %s practices left.\n\r") %       ch->getPracs(accclass) % classInfo[accclass].name);
    }
    initial = (ch->getDiscipline(TrainerInfo[offset].disc))->getNatLearnedness();
    ch->raiseDiscOnce(TrainerInfo[offset].disc);
    final = (ch->getDiscipline(TrainerInfo[offset].disc))->getNatLearnedness();
    bump = final-initial;

    for (i = MIN_SPELL; i < MAX_SKILL; i++) {
      if (hideThisSpell(i))
        continue;

      if ((discArray[i]->disc) != discipline)
        continue;

      if ((initial < discArray[i]->start) &&
          (final >= discArray[i]->start)) {
        if (discArray[i]->toggle && !ch->hasQuestBit(discArray[i]->toggle)) {
          ch->setSpellEligibleToggle(me, i, SILENT_NO);
          continue;
        }
        if ((discArray[i]->startLearnDo >= 0)) { // learned by doing with a bump
          value = min((int) discArray[i]->startLearnDo, discArray[i]->learn);
          value = max(value, (int) getRawNatSkillValue(i));
          value = max(value, 1);
          value = min(value, (int) MAX_SKILL_LEARNEDNESS);
          ch->setNatSkillValue(i, value);
          ch->setSkillValue(i,value);
          ch->affectTotal();
        } else {
          value = discArray[i]->learn;
          value = max(value, (int) getRawNatSkillValue(i));
          value = max(value, 1);
          value = min(value, (int) MAX_SKILL_LEARNEDNESS);
          ch->setNatSkillValue(i, value);
          ch->setSkillValue(i,value);
          ch->affectTotal();
        }
        sprintf(buf,"You have just learned %s!",discArray[i]->name);
        act (buf, FALSE, me, 0, ch, TO_VICT);
        act ("$N's head glows for a brief second.", FALSE, me, 0, ch, TO_NOTVICT);

        unsigned int comp;
        for (comp = 0; (comp < CompInfo.size()) && (i != CompInfo[comp].spell_num);comp++);

        if (comp != CompInfo.size() && CompInfo[comp].comp_num >= 0) {
          TObj *obj = NULL;
          obj = read_object(CompInfo[comp].comp_num, VIRTUAL);
          sprintf(buf, "%s personalized %s", obj->name, ch->getName());
          obj->swapToStrung();
          delete [] obj->name;
          obj->name = mud_str_dup(buf);

          obj->obj_flags.cost = 0;
          dynamic_cast<TComponent *>(obj)->setComponentCharges(10);
          obj->obj_flags.decay_time = -1;
          *ch += *obj;
          me->doTell(fname(ch->getName()), format("Here is %s for you to help in your learning of %s.") % obj->getName() % discArray[i]->name);
        }
      } else if (ch->doesKnowSkill(i)) {
        if ((discArray[i]->start <=final) && (ch->getRawSkillValue(i) < 0)) {
          vlogf(LOG_BUG, format("%s: ch->doesKnowSkill %s (%d) with no actual learning..could be array change or bug") %  ch->getName() % discArray[i]->name % i);
          ch->setNatSkillValue(i, 1);
          ch->setSkillValue(i, 1);
        }

        if (discArray[i]->startLearnDo < 0) { // doesnt use learn by doing
          value = (final - discArray[i]->start + 1) * discArray[i]->learn;
          value = max(value, (int) getRawNatSkillValue(i));
          value = max(value, 1);
          value = min(value, (int) MAX_SKILL_LEARNEDNESS);
          ch->setNatSkillValue(i, value);
          ch->setSkillValue(i, min((int)ch->getSkillValue(i), value));
          ch->affectTotal();
        } else {
          if (discArray[i]->startLearnDo > 0) {
            if (ch->getRawSkillValue(i) >= discArray[i]->startLearnDo) {
            // do nothing
              ch->affectTotal();
            } else if (ch->getRawSkillValue(i) >= ch->getMaxSkillValue(i)) {
              // do nothing
              ch->affectTotal();
            } else {
              value = min(discArray[i]->startLearnDo, ch->getMaxSkillValue(i));
              value = max(value, (int) ch->getRawNatSkillValue(i)); 
              value = max(value, 1);
              value = min(value, (int) MAX_SKILL_LEARNEDNESS);
              ch->setNatSkillValue(i, value);
              ch->setSkillValue(i, value);
              ch->affectTotal();
            }
          } else {
            // do nothing
          }
        }
      }
    }
  }

  if (TrainerInfo[offset].disc == DISC_WIZARDRY) {
    wizardryLevelT wiz = ch->getWizardryLevel();
    if (wiz >= WIZ_LEV_COMP_BELT)
        ch->sendTo("It is ok to have components contained on your belt.\n\r");
    else if (wiz == WIZ_LEV_COMP_NECK)
        ch->sendTo("It is ok to have components contained in a neck pouch.\n\r");
    else if (wiz == WIZ_LEV_COMP_WRIST)
        ch->sendTo("It is ok to have components contained in a wristpouch.\n\r");
    else if (wiz == WIZ_LEV_NO_MANTRA)
        ch->sendTo("You no longer need to speak the incantation.\n\r");
    else if (wiz == WIZ_LEV_NO_GESTURES)
        ch->sendTo("You no longer need to make hand gestures while casting!\n\r");
    else if (wiz == WIZ_LEV_COMP_INV)
        ch->sendTo("Components in your inventory will now be used.\n\r");
    else if (wiz == WIZ_LEV_COMP_EITHER)
      ch->sendTo("Components may be in either hand.\n\r");
    else if (wiz == WIZ_LEV_COMP_EITHER_OTHER_FREE)
      ch->sendTo("Components may be in either hand, and the other hand must be free.\n\r");
    else if (wiz <= WIZ_LEV_COMP_PRIM_OTHER_FREE)
      ch->sendTo("Components must be in your primary hand, and the other hand must be free.\n\r");
  }
  if (TrainerInfo[offset].disc == DISC_RITUALISM) {
    ritualismLevelT wiz = ch->getRitualismLevel();
    if (wiz >= RIT_LEV_COMP_BELT)
        ch->sendTo("It is ok to have components contained on your belt.\n\r");
    else if (wiz == RIT_LEV_COMP_NECK)
        ch->sendTo("It is ok to have components contained in a neck pouch.\n\r");
    else if (wiz == RIT_LEV_COMP_WRIST)
        ch->sendTo("It is ok to have components contained in a wristpouch.\n\r");
    else if (wiz == RIT_LEV_NO_MANTRA)
        ch->sendTo("You no longer need to speak the incantation.\n\r");
    else if (wiz == RIT_LEV_NO_GESTURES)
        ch->sendTo("You no longer need to make hand gestures while casting!\n\r");
    else if (wiz == RIT_LEV_COMP_INV)
        ch->sendTo("Components in your inventory will now be used.\n\r");
    else if (wiz == RIT_LEV_COMP_EITHER)
      ch->sendTo("Components may be in either hand.\n\r");
    else if (wiz == RIT_LEV_COMP_EITHER_OTHER_FREE)
      ch->sendTo("Components may be in either hand, and the other hand must be free.\n\r");
    else if (wiz <= RIT_LEV_COMP_PRIM_OTHER_FREE)
      ch->sendTo("Components must be in your primary hand, and the other hand must be free.\n\r");
  }
  if (TrainerInfo[offset].disc == DISC_FAITH) {
    devotionLevelT wiz = ch->getDevotionLevel();
    if (wiz >= DEV_LEV_NO_MANTRA)
        ch->sendTo("You can now pray silently if you have to!\n\r");
    else if (wiz == DEV_LEV_NO_GESTURES)
        ch->sendTo("You can pray without gestures if you have to!\n\r");
    else if (wiz == DEV_LEV_SYMB_NECK)
        ch->sendTo("You can now use a neck symbol.\n\r");
    else if (wiz == DEV_LEV_SYMB_EITHER_OTHER_EQUIP)
        ch->sendTo("You do not need to have your other hand free to pray.\n\r");
    else if (wiz == DEV_LEV_SYMB_PRIM_OTHER_EQUIP)
      ch->sendTo("Your secondary hand no longer has to be free if you have a symbol in your primary hand.\n\r");
    else if (wiz == DEV_LEV_SYMB_EITHER_OTHER_FREE)
      ch->sendTo("Symbols may be in either hand, and the other hand must be free.\n\r");
    else if (wiz <= DEV_LEV_SYMB_PRIM_OTHER_FREE)
      ch->sendTo("Symbols must be in your primary hand, and the other hand must be free.\n\r");
  }
  if (TrainerInfo[offset].disc == DISC_STEALTH) {
    if (ch->doesKnowSkill(SKILL_DISGUISE)) {
      int tNewSL = ch->getDiscipline(getDisciplineNumber(SKILL_DISGUISE, FALSE))->getLearnedness();

      for (int tType = 0; tType < MaxDisguiseType; tType++) {
        // Can not use right now, regardless.
        if (DisguiseList[tType].learning > tNewSL ||
            DisguiseList[tType].level    > ch->GetMaxLevel())
          continue;

        // Already been told, so skip it.
        if (DisguiseList[tType].learning < tOldSL &&
            DisguiseList[tType].level    < (ch->GetMaxLevel() - 1))
          continue;

        // Shouldn't happen, but you never know.
        // Thing is here we ONLY express the general disguises.
        if ((signed) DisguiseList[tType].tRace != RACE_NORACE)
          continue;

        if ((isname("male", DisguiseList[tType].name) &&
             ch->getSex() != SEX_MALE) ||
            (isname("female", DisguiseList[tType].name) &&
             ch->getSex() != SEX_FEMALE))
          continue;

        char tString[256];
        sstring tStArg(DisguiseList[tType].name),
               tStRes("");

        one_argument(tStArg, tStRes);

        sprintf(tString, "%s you can now use the '%s' disguise.",
                ch->getNameNOC(ch).c_str(), tStRes.c_str());
        me->doWhisper(tString);
      }
    }
  }

  if (TrainerInfo[offset].disc == DISC_SHAMAN_FROG) {
    if (ch->doesKnowSkill(SPELL_SHAPESHIFT)) {
      int tNewSL2 = ch->getDiscipline(getDisciplineNumber(SPELL_SHAPESHIFT, FALSE))->getLearnedness();

      for (int tType = 0; tType < MaxShapeShiftType; tType++) {
        // Can not use right now, regardless.
        if (ShapeShiftList[tType].learning > tNewSL2 ||
            ShapeShiftList[tType].level    > ch->GetMaxLevel())
          continue;

        // Already been told, so skip it.
        if (ShapeShiftList[tType].learning < tOldSL2 &&
            ShapeShiftList[tType].level    < (ch->GetMaxLevel() - 1))
          continue;

        // Shouldn't happen, but you never know.
        // Thing is here we ONLY express the general disguises.
        if ((signed) ShapeShiftList[tType].tRace != RACE_NORACE)
          continue;

        char tString[256];
        sstring tStArg(ShapeShiftList[tType].name),
               tStRes("");

        one_argument(tStArg, tStRes);

        sprintf(tString, "%s you can now shapeshift into a %s.",
                ch->getNameNOC(ch).c_str(), tStRes.c_str());
        me->doWhisper(tString);
      }
    }
  }
  act ("$n smiles as $e removes $s hands from your head.",
            FALSE, me, 0, ch, TO_VICT);
  act ("$n removes $s hands from $N's head and smiles.", FALSE, me, 0, ch, TO_NOTVICT);
  ch->doSave(SILENT_YES);
  return TRUE;
}


int TBeing::initiateSkillsLearning(discNumT discipline, int initial, int final)
{
  spellNumT i;
  int amount = 0;
  int value;

  for (i = MIN_SPELL; i < MAX_SKILL; i++) {
    if (hideThisSpell(i))
      continue;

    if ((discArray[i]->disc) != discipline)
      continue;

    if ((initial >= discArray[i]->start) &&
        (final < discArray[i]->start)) {
      setNatSkillValue(i, SKILL_MIN);
      setSkillValue(i,SKILL_MIN);
      continue;
    }
    if (discArray[i]->toggle && !hasQuestBit(discArray[i]->toggle)) {
      continue;
    }
    amount = final - initial;
    if ((*discArray[i]->name) &&
           (initial < discArray[i]->start) &&
           (final >= discArray[i]->start)) {
      amount = amount - (discArray[i]->start - initial);
      if ((discArray[i]->startLearnDo > 0)) { // learned by do with a bump 
        value = min((int) discArray[i]->startLearnDo, (amount * discArray[i]->learn));
        value = max(value, (int) getRawNatSkillValue(i));
        value = max(value, 1);
        value = min(value, (int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
      } else if ((discArray[i]->startLearnDo == 0)) {
        value = max(1, discArray[i]->learn);
        value = max(value, (int) getRawNatSkillValue(i));
        value = max(value, 1);
        value = min(value, (int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
      } else if ((discArray[i]->startLearnDo < 0)) {
        value = max(1, (amount*discArray[i]->learn));
        value = max(value, (int) getRawNatSkillValue(i));
        value = max(value, 1);
        value = min(value, (int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
      }
    } else if (final >= discArray[i]->start) {
      if (discArray[i]->startLearnDo < 0) { // doesnt use learn by doing
        value = (final - discArray[i]->start + 1) *  discArray[i]->learn;
        value = max(value, (int) getRawNatSkillValue(i));
        value = max(value, 1);
        value = min(value, (int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
      } else if (discArray[i]->startLearnDo == 0) {
        value = max((int) getNatSkillValue(i), discArray[i]->learn);
        value = max(value, (int) getRawNatSkillValue(i));
        value = max(value, 1);
        value = min(value,(int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
      } else {
        value = min((int) discArray[i]->startLearnDo, (discArray[i]->learn * (final + 1 - discArray[i]->start)));
        value = max(value, (int) getRawNatSkillValue(i));
        value = max(value, 1);
        value = min(value,(int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i, value);
        setSkillValue(i, value);
      }
    }
  }
  affectTotal();
  doSave(SILENT_YES);
  return TRUE;
}


int GenericGuildMaster(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)  
{
  if (cmd == CMD_GENERIC_PULSE)
    me->aiMaintainCalm();

  if(cmd!=CMD_GAIN)
    return false;

  if(!ch || !me)
    return false;

  classIndT cit=me->bestClass();
  unsigned short Class=me->getClass();
  sstring argument=arg;
  int practices = 0;

  if (!ch->hasClass(Class)) {
    sstring buf;

    buf = format("$n growls, \"Go away, $N.  You're no %s!\"") %
	     classInfo[cit].name;
    act(buf, FALSE, me, 0, ch, TO_ROOM);

    return TRUE;
  }
    
  if (ch->isImmortal() || !me)
    return FALSE;

  if (generic_trainer_stuff(me, ch))
    return TRUE;

  if (ch->getLevel(cit) < (me->GetMaxLevel()/2) ||
    (MAX_MORT == ch->getLevel(cit) && MAX_MORT <= me->GetMaxLevel()/2)){
    TPerson *tp;

    // the specific quest required for a reset, given the guildmaster level
    int resetQuest = 0; // TOG_RESET_PRAC_LEVEL50 - no quest exists for this yet
    int myTrainerLevel = (me->GetMaxLevel()/2);
    if (myTrainerLevel < 50)
      resetQuest--;
    if (myTrainerLevel < 40)
      resetQuest--;

    ch->resetPractices(cit, practices, false);
    int cost = practices * REPRAC_COST_PER_PRAC;
    if (resetQuest && ch->hasQuestBit(resetQuest))
      cost = cost / 2;

    if (is_abbrev(arg, "reset")) {
      if (ch->getMoney() < cost) {
        me->doSay(format("I'm sorry, you don't have enough money for me to reset your spent practices.  The cost is %d talens.") % cost);
      } else if (ch->resetPractices(cit, practices, true)) {
        me->doSay(format("I have reset %d practices for you.  You will now have to visit your trainers to relearn your disciplines.") % practices);
        ch->giveMoney(me, cost, GOLD_SHOP_RESPONSES);
        if (resetQuest)
          ch->remQuestBit(resetQuest);
      } else
        me->doSay("I cannot reset any of your practices at this time.");

      return TRUE;
    }

    if ((tp = dynamic_cast<TPerson *>(ch)))
      tp->setSelectToggles(me, cit, SILENT_NO);

    if (MAX_MORT == ch->getLevel(cit)) {
      act("$n beams, \"No one can teach you anymore in this, $N\"", FALSE, me, NULL, ch, TO_ROOM);
    } else {
      me->doSay("Let me give you a little advice...");
      ch->pracPath(me, cit);
    }

    if (practices > 0) {
      me->doSay(format("I could also reset all of your %d spent practices for you.") % practices);
      me->doSay(format("To do so, type 'gain reset'.  There will be a fee of %d talens.") % cost);
      if (resetQuest && !ch->hasQuestBit(resetQuest))
         me->doSay(format("I'll help you for only %d talens if you do me a small <c>favor<1> first.") % (cost / 2));
    }

  } else if (ch->getLevel(cit) < MAX_MORT) {
    act("$n sighs, \"I cannot teach you, $N.  You MUST find your next guildmaster.\"", FALSE, me, 0, ch, TO_ROOM);
  }
  return TRUE;
}

TMonster *FindMobInRoomWithProcNum(int room, int num)
{
  TThing *t=NULL;

  if (room < 0)
    return NULL;

  for(StuffIter it=real_roomp(room)->stuff.begin();it!=real_roomp(room)->stuff.end() && (t=*it);++it) {
    TMonster *tmons = dynamic_cast<TMonster *>(t);
    if (tmons && (tmons->spec == num))
      return tmons;
  }
  return NULL;
}

wizardryLevelT TBeing::getWizardryLevel() const
{
  int skill;

  if (!doesKnowSkill(SKILL_WIZARDRY))
    return WIZ_LEV_NONE;
  else if ((skill = getSkillValue(SKILL_WIZARDRY)) < 15 && !isAmbidextrous())
    return WIZ_LEV_COMP_PRIM_OTHER_FREE;
  else if (skill < 30)
    return WIZ_LEV_COMP_EITHER_OTHER_FREE;
  else if (skill < 40)
    return WIZ_LEV_COMP_EITHER;
  else if (skill < 50)
    return WIZ_LEV_COMP_INV;
  else if (skill < 60)
    return WIZ_LEV_NO_GESTURES;
  else if (skill < 75)
    return WIZ_LEV_NO_MANTRA;
  else if (skill < 98)
    return WIZ_LEV_COMP_NECK;
  else if (skill < 99)
    return WIZ_LEV_COMP_NECK;
  else if (skill < MAX_SKILL_LEARNEDNESS)
    return WIZ_LEV_COMP_BELT;
  else
    return WIZ_LEV_MAXED;
}

ritualismLevelT TBeing::getRitualismLevel() const
{
  int skill;

  if (!doesKnowSkill(SKILL_RITUALISM))
    return RIT_LEV_NONE;
  else if ((skill = getSkillValue(SKILL_RITUALISM)) < 15 && !isAmbidextrous())
    return RIT_LEV_COMP_PRIM_OTHER_FREE;
  else if (skill < 30)
    return RIT_LEV_COMP_EITHER_OTHER_FREE;
  else if (skill < 40)
    return RIT_LEV_COMP_EITHER;
  else if (skill < 50)
    return RIT_LEV_COMP_INV;
  else if (skill < 60)
    return RIT_LEV_NO_GESTURES;
  else if (skill < 75)
    return RIT_LEV_NO_MANTRA;
  else if (skill < 98)
    return RIT_LEV_COMP_NECK;
  else if (skill < 99)
    return RIT_LEV_COMP_NECK;
  else if (skill < MAX_SKILL_LEARNEDNESS)
    return RIT_LEV_COMP_BELT;
  else
    return RIT_LEV_MAXED;
}

devotionLevelT TBeing::getDevotionLevel() const
{
  int skill;

  if (!doesKnowSkill(SKILL_DEVOTION))
    return DEV_LEV_NONE;
  else if ((skill = getSkillValue(SKILL_DEVOTION)) < 15 && !isAmbidextrous())
    return DEV_LEV_SYMB_PRIM_OTHER_FREE;
  else if (skill < 30)
    return DEV_LEV_SYMB_EITHER_OTHER_FREE;
  else if (skill < 40 && !isAmbidextrous())
    return DEV_LEV_SYMB_PRIM_OTHER_EQUIP;
  else if (skill < 50)
    return DEV_LEV_SYMB_EITHER_OTHER_EQUIP;
  else if (skill < 60)
    return DEV_LEV_SYMB_NECK;
  else if (skill < 75)
    return DEV_LEV_NO_GESTURES;
  else if (skill < MAX_SKILL_LEARNEDNESS)
    return DEV_LEV_NO_MANTRA;
  else
    return DEV_LEV_MAXED;
}

void TBeing::pracPath(TMonster *gm, classIndT Class)
{
  char buf[256];
  sstring tmp_buf, tmp_buf2;
  int combat = 0, basic = 0;
  bool basicLearn = FALSE, combatLearn = FALSE;
  int combatMax=0;

  if (!checkDoneBasic(this, Class, FALSE, FALSE)) {
    sprintf(buf, "Hmmm, looks like you are free to use your practices at any advanced %s trainer.", gm->getProfName().c_str());
    gm->doSay(buf);
    return;
  }

  int combatReq=(((35*getLevel(Class)) /10) - 4);

  combat = getDiscipline(DISC_COMBAT)->getNatLearnedness();
  tmp_buf=sstring(discNames[DISC_COMBAT].name).cap();
  
  if(classInfo[Class].sec_disc != DISC_NONE){
    combat+=getDiscipline(classInfo[Class].sec_disc)->getNatLearnedness();
    tmp_buf += " or ";
    tmp_buf += sstring(discNames[classInfo[Class].sec_disc].name).cap();
    tmp_buf2 = sstring(discNames[classInfo[Class].sec_disc].name).cap();
  }

  basic = getDiscipline(classInfo[Class].base_disc)->getNatLearnedness();

  if (basic >= MAX_DISC_LEARNEDNESS) {
    basicLearn=TRUE;
  }
  if (combat >= getCombatPrereqNumber(Class)) {
    combatLearn = TRUE;
    combatMax = 1;
  }
  if (combat == MAX_DISC_LEARNEDNESS) {
    combatMax = 2;
    combatLearn = TRUE;
  }
  if (combat >= combatReq) {
    combatLearn = TRUE;
  }


  if (combatMax && basicLearn) {
    sprintf(buf, "Hmmm, looks like you are free to use these practices at any advanced %s trainer.", gm->getProfName().c_str());
  } else if (!combatLearn) {
      sprintf(buf, "I think you need to first use %i practices at your %s trainer.", (int)((combatReq-combat)+0.5), tmp_buf.c_str());
  } else if (!combatMax && !basicLearn) {
    if (getDiscipline(DISC_COMBAT)->getNatLearnedness() >= MAX_DISC_LEARNEDNESS)
      sprintf(buf, "You could train in a weapon speciality or continue to use these practices at your %s or %s trainer.", gm->getProfName().c_str(),tmp_buf2.c_str());
    else
      sprintf(buf, "You can use these practices at your %s trainer or you could always continue to train in %s.", gm->getProfName().c_str(), tmp_buf.c_str());

  } else if (!combatMax && basicLearn) {
    if (getDiscipline(DISC_COMBAT)->getNatLearnedness() >= MAX_DISC_LEARNEDNESS)
      sprintf(buf, "You could train in a weapon speciality or continue to use these practices at your %s trainer.", tmp_buf.c_str());
    else
      sprintf(buf, "You need to use these practices to finish your basic training at your %s trainer.", tmp_buf.c_str()); 
  } else if (combatMax && !basicLearn) {
    if (getDiscipline(DISC_COMBAT)->getNatLearnedness() >= MAX_DISC_LEARNEDNESS) {
     sprintf(buf, "You should use these practices at your basic %s trainer or you can pursue a weapon specialization.", gm->getProfName().c_str());
    } else if (combatMax == 1) {
      sprintf(buf, "You can use these practices at your basic %s trainer or you could always continue to train in %s.", gm->getProfName().c_str(), tmp_buf.c_str());
    } else if (combatMax == 2) {
      sprintf(buf, "You need to use these practices at your basic %s trainer or you could pursue a weapon specialization.", gm->getProfName().c_str());
    }
  } else {
    vlogf(LOG_BUG, format("Bad case in pracPath for %s") %  getName());
    sendTo("Please bug that there is a bad place in guildmaster instructions.");
    return;
  }
  gm->doSay(buf);
  return;
}

double getExpClassLevel(classIndT Class, int level)
{
  // developed: 10-3-97  batopr
  // formulatic method for getting exp for each class for each level.
  // This replaced old method of having huge arrays for each.

  // If Class is used to change XP, then a change is needed in prompt_d
  // for xptnl.  - Bat 3/9/99

  double exp_amt;

  if (level <= 1)
    return 0;

  // avoid roundoff thing
  if (level == MAX_MORT)
    return 1000000000;

  exp_amt = getExpClassLevel(Class, level-1);

  double M = mob_exp(level - 1);

  // make the kills you need increase with level
  // the start value here is set such taht we wind up with 100M at L50
  double K = kills_to_level(level-1);
  exp_amt += (K * M);

  // and round it up to closest int
  exp_amt += 0.5;

  return exp_amt;
}
