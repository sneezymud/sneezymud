#include <cmath>

#include "stdsneezy.h"
#include "statistics.h"
#include "obj_component.h"
#include "database.h"


// if logic changes, please change some of the duplicate code in pracsBetween()
void TBeing::setSpellEligibleToggle(TMonster *trainer, spellNumT spell, silentTypeT silent) 
{
  char buf[256];

  if (!silent && trainer) {
    sprintf(buf,"%s You now have the training to learn %s!",
    		fname(name).c_str(), discArray[spell]->name);
    trainer->doTell(buf);
  }

  switch (spell) {
    case SPELL_TORNADO:
      if (!silent && trainer) {
        sprintf(buf,"%s Alas, I do not have the knowledge to train you in tornado.",
                    fname(name).c_str());
        trainer->doTell(buf);
        sprintf(buf,"%s Seek out the wise elf Salrik to see if you can learn it from him.  I will let him know that I have sent you.", fname(name).c_str());
        trainer->doTell(buf);
      }
      setQuestBit(TOG_TORNADO_ELIGIBLE);
      break;

    case SKILL_BARKSKIN:
      if (!silent && trainer) {
        sprintf(buf,"%s However, before I train you in barkskin, I must ask you to perform a small task to prove your worth.",
                    fname(name).c_str());
        trainer->doTell(buf);
        sprintf(buf,"%s In order to prove you are ready for such knowledge, bring me some barkskin.", fname(name).c_str());
        trainer->doTell(buf);
      }
      setQuestBit(TOG_ELIGIBLE_BARKSKIN);
      break;
      
    case SPELL_EARTHQUAKE:
      if (!silent && trainer) {
        sprintf(buf,"%s However, before I train you in earthquake, I must ask you to perform a small task to prove your worth.",
                    fname(name).c_str());
        trainer->doTell(buf);
        sprintf(buf,"%s In order to prove you are ready for such knowledge, bring me a yellow boot.", fname(name).c_str());
        trainer->doTell(buf);
      }
      setQuestBit(TOG_ELIGIBLE_EARTHQUAKE);
      break;
      
    case SKILL_DUAL_WIELD:
      if (!silent && trainer) {
        sprintf(buf,"%s However, before I train you in dual wield, I must ask you to perform a small task to prove your worth.",
                    fname(name).c_str());
        trainer->doTell(buf);
        sprintf(buf,"%s In order to prove you are ready for such knowledge, bring me some mandrake.", fname(name).c_str());
        trainer->doTell(buf);
      }
      setQuestBit(TOG_ELIGIBLE_DUAL_WIELD);
      break;
      
    case SPELL_FIREBALL:
      if (!silent && trainer) {
        sprintf(buf,"%s Alas, I do not have the knowledge to train you in fireball.",
                    fname(name).c_str());
        trainer->doTell(buf);
        sprintf(buf,"%s Seek out the mischevious mage Kallam to see if you can learn it from him.  I will let him know that I have sent you.", fname(name).c_str());
        trainer->doTell(buf);
      }
      setQuestBit(TOG_ELIGIBLE_FIREBALL);
      break;

    case SPELL_ICE_STORM:
      if (!silent && trainer) {
        sprintf(buf,"%s Alas, I do not have the knowledge to train you in ice storm.",
                    fname(name).c_str());
        trainer->doTell(buf);
        sprintf(buf,"%s Seek out the water mage, Cardac, to see if you can learn it from him.  I will let him know that I have sent you.", fname(name).c_str());
        trainer->doTell(buf);
      }
      setQuestBit(TOG_ELIGIBLE_ICE_STORM);
      break;

    case SPELL_STONE_SKIN:
      if (!silent && trainer) {
        sprintf(buf,"%s Alas, I do not have the knowledge to train you in stone skin.",
                    fname(name).c_str());
        trainer->doTell(buf);
        sprintf(buf,"%s However, I do remember a dwarf that perhaps has such knowledge of using defensive Earth Magic.", fname(name).c_str());
        trainer->doTell(buf);
        sprintf(buf,"%s I think the City in the Clouds has the dwarf you seek.  He is a very important abassador there.", fname(name).c_str());
        trainer->doTell(buf);
        sprintf(buf,"%s Seek him out, he may hold the secrets you desire.", fname(name).c_str());
        trainer->doTell(buf);
      }
      setQuestBit(TOG_ELIGIBLE_STONESKIN);
      break;

    case SPELL_GALVANIZE:
      if (!silent && trainer) {
        sprintf(buf,"%s Alas, I do not have the knowledge to train you in galvanize.",
                    fname(name).c_str());
        trainer->doTell(buf);
        sprintf(buf,"%s Seek out the wizened mage, Fabnir, to see if you can learn it from him.  I will let him know that I have sent you.", fname(name).c_str());
        trainer->doTell(buf);
      }
      setQuestBit(TOG_ELIGIBLE_GALVANIZE);
      break;

    case SPELL_POWERSTONE:
      if (!silent && trainer) {
        sprintf(buf,"%s Alas, I do not have the knowledge to train you in powerstone.",
                    fname(name).c_str());
        trainer->doTell(buf);
        sprintf(buf,"%s Seek out the wise alchemist, Fabnir, to see if you can learn it from him.  I will let him know that I have sent you.", fname(name).c_str());
        trainer->doTell(buf);
      }
      setQuestBit(TOG_ELIGIBLE_POWERSTONE);
      break;

    case SKILL_ADVANCED_KICKING:
      if(!silent && trainer){
	sprintf(buf, "%s Although you are eligible to learn advanced kicking, you must first master kick.", fname(name).c_str());
	trainer->doTell(buf);
	sprintf(buf, "%s When you have mastered kick, speak with your guildmaster and I will tell you how to learn advanced kicking.", fname(name).c_str());
	trainer->doTell(buf);
      }
      break;

    default:
      return;
  }  
  return;
}

int TBeing::calcRaiseDisc(discNumT which, bool drop) const
{
  int L, i_inc;

  L = getDiscipline(which)->getNatLearnedness();
  if (!drop && (L >= MAX_DISC_LEARNEDNESS)) {
    return 0;
  }
  if (drop && (L <= 0)) {
    return 0;
  }

  switch (which) {
    case DISC_COMBAT:
    case DISC_MAGE:
    case DISC_MAGE_THIEF:
    case DISC_CLERIC:
    case DISC_THIEF:
    case DISC_WARRIOR:
    case DISC_RANGER:
    case DISC_DEIKHAN:
    case DISC_MONK:
    case DISC_SHAMAN:
    case DISC_LORE:
    case DISC_THEOLOGY:
      return 1;
    case DISC_SLASH:
    case DISC_BLUNT:
    case DISC_PIERCE:
    case DISC_BAREHAND:
    case DISC_DEFENSE:
    case DISC_PSIONICS:
      return min(5, (MAX_DISC_LEARNEDNESS - L));
    default:
      break;
  }

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
  L = getDiscipline(which)->getNatLearnedness();

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

  // people report practicing and getting no gain, trap this event.
  mud_assert(i_inc >= 1, "Bad discipline increase");

  return i_inc;
}

// this will return negative if over target
int TBeing::pracsBetween(discNumT which, int target) const
{
  int num;

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
  char buf[256];

  if (getLevel(Class) >= 8 &&
      !hasQuestBit(TOG_FACTIONS_ELIGIBLE) &&
      isUnaff()) {
    gm->doSay("Your advanced level makes you eligible to join a faction if you so choose.");
    gm->doSay("The mayors of Brightmoon, Logrus and Amber can provide details on their respective faction.");
    setQuestBit(TOG_FACTIONS_ELIGIBLE);
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
        setQuestBit(TOG_AVENGER_ELIGIBLE);
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
       setQuestBit(TOG_VINDICATOR_ELIGIBLE);
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
       setQuestBit(TOG_DEVESTATOR_ELIGIBLE);
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
	setQuestBit(TOG_ELIGIBLE_MONK_WHITE);
      }
      if(hasQuestBit(TOG_HAS_MONK_WHITE) &&
	 !hasQuestBit(TOG_HAS_MONK_YELLOW) &&
	 !hasQuestBit(TOG_ELIGIBLE_MONK_YELLOW) &&
	 !hasQuestBit(TOG_FINISHED_MONK_YELLOW) &&
	 getLevel(Class)>=5){
	if(!silent)
	  gm->doSay("You are now eligible to quest for your yellow sash.  Say \"yellow sash\" for more information.");
	setQuestBit(TOG_ELIGIBLE_MONK_YELLOW);
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
	setQuestBit(TOG_MONK_PURPLE_ELIGIBLE);
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
	setQuestBit(TOG_ELIGIBLE_MONK_BLUE);
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

	setQuestBit(TOG_MONK_GREEN_ELIGIBLE);
      }
      if(hasQuestBit(TOG_MONK_GREEN_OWNED) &&
	 !hasQuestBit(TOG_STARTED_MONK_RED) &&
	 !hasQuestBit(TOG_FINISHED_MONK_RED) &&
	 !hasQuestBit(TOG_HAS_MONK_RED) &&
	 getLevel(Class)>=45){
	if(!silent)
	  gm->doSay("You are now eligible to quest for your red sash.  Say \"red sash\" for more information.");
	setQuestBit(TOG_MONK_RED_ELIGIBLE);
      }
      if(hasQuestBit(TOG_HAS_MONK_RED) &&
	 !hasQuestBit(TOG_MONK_BLACK_STARTED) &&
	 !hasQuestBit(TOG_MONK_BLACK_FINISHED) &&
	 !hasQuestBit(TOG_MONK_BLACK_OWNED) &&
	 getLevel(Class)==50){
	if(!silent)
	  gm->doSay("You are now eligible to quest for your black sash.  Say \"black sash\" for more information.");
	setQuestBit(TOG_MONK_BLACK_ELIGIBLE);
      }
      break;
    case SHAMAN_LEVEL_IND:
      if (getLevel(Class)==6) {
	if(!silent){
	  gm->doSay("<Y>Your status as a newbie has been removed.<1>");
	  gm->doSay("<B>You will now be subject to penalty if you let<1>");
	  gm->doSay("<B>your lifeforce fall to 0.<1>");
	}
      }
      if (getLevel(Class)>=15 &&
            !hasQuestBit(TOG_ELIGABLE_JUJU) &&
            !hasQuestBit(TOG_GET_THONG) &&
            !hasQuestBit(TOG_MARE_HIDE) &&
            !hasQuestBit(TOG_GET_SINEW) &&
            !hasQuestBit(TOG_GET_BEADS) &&
            !hasQuestBit(TOG_DONE_JUJU)) {
	if(!silent){
	  gm->doAction(name, CMD_BEAM);
	  gm->doSay("Congratulations! You have earned the right to make a juju bag.");
	  gm->doSay("It will aid in your communications with the loa as well as store components.");
	  gm->doSay("Say 'juju bag' if you want more information on this quest.");
	}
	setQuestBit(TOG_ELIGABLE_JUJU);
      }
      if (getLevel(Class)>=30 &&
            !hasQuestBit(TOG_TOTEM_MASK_ELIGABLE) &&
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
	setQuestBit(TOG_TOTEM_MASK_ELIGABLE);
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
	setQuestBit(TOG_MAGE_BELT_ELIGIBLE);
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
        setQuestBit(TOG_ELIGIBLE_MAGE_ROBE);
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
	setQuestBit(TOG_RANGER_FIRST_ELIGIBLE);
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
	setQuestBit(TOG_ELIGIBLE_RANGER_L14);
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
	setQuestBit(TOG_ELIGIBLE_WARRIOR_L41);
      }
      break;
    default:
      break;
  }
  doSave(SILENT_YES);
}

void TPerson::advanceSelectDisciplines(TBeing *gm, classIndT Class, int numx, silentTypeT silent)
{
  int learnAdd = 0;
  int i, count, initial, final;

  for (i = 0; i < numx; i++) {
    if ((Class == MAGE_LEVEL_IND) || (Class == MAGE_THIEF_LEVEL_IND)) {
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

          doLevelSkillsLearn(gm, DISC_WIZARDRY, initial, final);
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

          doLevelSkillsLearn(gm, DISC_RITUALISM, initial, final);
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

          doLevelSkillsLearn(gm, DISC_FAITH, initial, final);
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

          doLevelSkillsLearn(gm, DISC_ADVENTURING, initial, final);
        }
      }
    }
  }
}


void logPermaDeathLevel(TBeing *ch){
  TDatabase db("sneezy");
  db.query("replace into permadeath (name, level, killer, died) values ('%s', %i, 'no one', 0)", ch->name, ch->GetMaxLevel());
}

void clearPermaDeathLevel(TBeing *ch){
  TDatabase db("sneezy");
  db.query("delete from permadeath where name='%s'", ch->name);
}


void TPerson::raiseLevel(classIndT Class, TMonster *gm)
{
  char buf[160];
  int amount, maxhit;
//  int count, learnAdd = 0;
//  int initial, final;

  if (getExp() >= getExpClassLevel(Class, getLevel(Class) + 1)) {
#if FACTIONS_IN_USE
    if (percLess(1.5 * ((float) getLevel(Class) + 1))) {
      act("$n scowls at $N and says, \"But I cannot support your actions!\"", 
            FALSE, gm, 0, this, TO_NOTVICT);
      act("$n scowls at you and says, \"But I cannot support your actions!\"", 
            FALSE, gm, 0, this, TO_VICT);
      sprintf(buf, "%s Come back when you have remained more true to your beliefs...", fname(name).c_str());
      gm->doTell(buf);
      return;
    }
#endif

    if (Class == MONK_LEVEL_IND && (getClassLevel(CLASS_MONK) >= 5)) {
      amount = 20 * getClassLevel(CLASS_MONK) * getClassLevel(CLASS_MONK);
      if ((getMoney() < amount)) {
        sprintf(buf, "$n says, \"Sorry $N, the High Tabuda demands %d talens before I can teach you.\"", amount);
        act(buf, FALSE, gm, 0, this, TO_VICT);
        return;
      } else {
        addToMoney(-amount, GOLD_REPAIR);
	saveGovMoney("monk donation", amount);
	

        sprintf(buf, "$n says, \"$N, the High Tabuda thanks you for your %d talen donation to the kwoon.\"", amount);
        act(buf, FALSE, gm, 0, this, TO_VICT);
      }
    }
    act("$n smiles, \"Yes...it is time for you to advance.\"", 
         FALSE, gm, 0, this, TO_NOTVICT);
    act("$n rests $s hand on $N's head.", FALSE, gm, 0, this, TO_NOTVICT);
    act("$n warns, \"Your journeys will be more difficult.\"", 
         FALSE, gm, 0, this, TO_NOTVICT);
    act("$n continues, \"I grant thee my blessing.\"", 
         FALSE, gm, 0, this, TO_NOTVICT);
    act("$N glows with pride as $n raises $s hand from $S head.", 
          FALSE, gm, 0, this, TO_NOTVICT);
    act("$n smiles, \"Yes... It is time for you to advance.\"", 
          FALSE, gm, 0, this, TO_VICT);
    act("$n rests $s hand on your head.", FALSE, gm, 0, this, TO_VICT);
    act("$n warns, \"Your journeys will be more difficult.\"", 
          FALSE, gm, 0, this, TO_VICT);
    act("$n continues, \"I grant thee my blessing.\"", 
          FALSE, gm, 0, this, TO_VICT);
    act("You glow with pride as $n raises $s hand from your head.", 
          FALSE, gm, 0, this, TO_VICT);

// setting basic before the level is raised

    int doneBas = checkDoneBasic(this, Class, TRUE, FALSE);
    if (doneBas == 4) {
      sprintf(buf, "%s Let me add my further congratulations for finishing basic training.", fname(name).c_str());
      gm->doTell(buf);
    }
         
// Advance them in a few select disciplines
    advanceSelectDisciplines(gm, Class, 1, SILENT_NO);

    //The fix statement below set the client who window with correct level
    fixClientPlayerLists(TRUE);

    maxhit=points.maxHit;
    advanceLevel(Class, gm);

    // may as well still give them the hp in case we want to switch back
    // just don't announce it
#if !NEW_HP
    sendTo("You gain %i hitpoints!\n\r", points.maxHit-maxhit);
#endif

    fixClientPlayerLists(FALSE);
    setTitle(false);
    setSelectToggles((TBeing *) gm, Class, SILENT_NO);
    
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

      realTimePassed((time(0) - player.time.logon) +
                                  player.time.played, 0, &playing_time);

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
  } else 
    act("$n cautions, \"You are not ready to advance, $N.\"", FALSE, gm, 0,
this, TO_ROOM);
}

void TPerson::doLevelSkillsLearn(TBeing *gm, discNumT discipline, int initial, int final)
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
      if (discArray[i]->toggle && !hasQuestBit(discArray[i]->toggle)) {
        sprintf(buf,"%s You now have the training to learn %s!",
        fname(name).c_str(), discArray[i]->name);
        gm->doTell(buf);
        sprintf(buf,"%s Alas, I do not have the knowledge to train you in this.", fname(name).c_str());
        gm->doTell(buf);
        sprintf(buf,"%s To learn it, you will have to find another teacher.", fname(name).c_str());
        gm->doTell(buf);
        continue;
      }
      if (discArray[i]->startLearnDo > 0) { // learned by doing with a bump
        value = min((int) discArray[i]->startLearnDo, discArray[i]->learn);
        value = max(value, 1);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
        affectTotal();
      } else {
        value = max(1, discArray[i]->learn);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
        affectTotal();
      }
      sprintf(buf,"You have just learned %s!",discArray[i]->name);
      act (buf, FALSE, this, 0, NULL, TO_CHAR);
    } else if ((*discArray[i]->name) && (initial >= discArray[i]->start) && 
               !(discArray[i]->toggle && !hasQuestBit(discArray[i]->toggle))) {
      if (discArray[i]->startLearnDo == -1) { // doesnt use learn by doing
        value = discArray[i]->learn * (1 + discLearn - discArray[i]->start);
        value = max(value, 1);
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
  {SPEC_TRAINER_AIR, "air", "the art of Air Magic", DISC_AIR, CLASS_MAGIC_USER},
  {SPEC_TRAINER_ALCHEMY, "alchemy", "the art of Alchemy", DISC_ALCHEMY, CLASS_MAGIC_USER},
  {SPEC_TRAINER_EARTH, "earth", "the art of Earth Magic", DISC_EARTH, CLASS_MAGIC_USER},
  {SPEC_TRAINER_FIRE, "fire", "the art of Fire Magic", DISC_FIRE, CLASS_MAGIC_USER},
  {SPEC_TRAINER_SORCERY, "sorcery", "the art of Sorcery", DISC_SORCERY, CLASS_MAGIC_USER},
  {SPEC_TRAINER_SPIRIT, "spirit", "the art of Spirit Magic", DISC_SPIRIT, CLASS_MAGIC_USER},
  {SPEC_TRAINER_WATER, "water", "the art of Water Magic", DISC_WATER, CLASS_MAGIC_USER},
  {SPEC_TRAINER_WRATH, "wrath", "the Wrath of the Deities", DISC_WRATH, CLASS_CLERIC},
  {SPEC_TRAINER_AFFLICTIONS, "afflictions", "the Art of Afflictions", DISC_AFFLICTIONS, CLASS_CLERIC},
  {SPEC_TRAINER_CURE, "cures", "the Healing Arts", DISC_CURES, CLASS_CLERIC},
  {SPEC_TRAINER_HAND_OF_GOD, "hand", "the Hand of the Deities", DISC_HAND_OF_GOD, CLASS_CLERIC},
  {SPEC_TRAINER_RANGER, "ranger", "the Ways of the Ranger", DISC_RANGER, CLASS_RANGER},
  {SPEC_TRAINER_LOOTING, "looting", "Looting and Plundering", DISC_LOOTING, CLASS_THIEF},
  {SPEC_TRAINER_MURDER, "murder", "about Deadly Murder", DISC_MURDER, CLASS_THIEF},
  {SPEC_TRAINER_HAND_TO_HAND, "hth", "Hand-to-Hand Combat", DISC_HTH, CLASS_WARRIOR},
  {SPEC_TRAINER_ADVENTURING, "adventuring", "Adventurers' Lore", DISC_ADVENTURING, CLASS_MAGIC_USER | CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR | CLASS_MONK | CLASS_RANGER | CLASS_DEIKHAN | CLASS_SHAMAN | CLASS_MAGE_THIEF},
  {SPEC_TRAINER_COMBAT, "combat", "Combat Skills", DISC_COMBAT, CLASS_MAGIC_USER | CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR | CLASS_MONK | CLASS_RANGER | CLASS_DEIKHAN | CLASS_SHAMAN | CLASS_MAGE_THIEF},
  {SPEC_TRAINER_WARRIOR, "warrior", "the Ways of the Warrior", DISC_WARRIOR, CLASS_WARRIOR},
  {SPEC_TRAINER_WIZARDRY, "wizardry", "Wizardry", DISC_WIZARDRY, CLASS_MAGIC_USER | CLASS_MAGE_THIEF},
  {SPEC_TRAINER_FAITH, "faith", "Faith", DISC_FAITH, CLASS_CLERIC | CLASS_DEIKHAN},
  {SPEC_TRAINER_SLASH, "slash", "Slash Specialization", DISC_SLASH, CLASS_WARRIOR | CLASS_RANGER | CLASS_THIEF | CLASS_DEIKHAN},
  {SPEC_TRAINER_BLUNT, "blunt", "Blunt Specialization", DISC_BLUNT, CLASS_WARRIOR | CLASS_CLERIC | CLASS_DEIKHAN | CLASS_SHAMAN},
  {SPEC_TRAINER_PIERCE, "pierce", "Pierce Specialization", DISC_PIERCE, CLASS_WARRIOR | CLASS_THIEF | CLASS_MAGIC_USER | CLASS_RANGER | CLASS_MAGE_THIEF},
  {SPEC_TRAINER_RANGED, "ranged", "Ranged Specialization", DISC_RANGED, CLASS_RANGER},
  {SPEC_TRAINER_DEIKHAN, "deikhan", "the Ways of the Deikhan", DISC_DEIKHAN, CLASS_DEIKHAN},
  {SPEC_TRAINER_BRAWLING, "brawling", "Brawling Moves", DISC_BRAWLING, CLASS_WARRIOR},
  {SPEC_TRAINER_MEDITATION_MONK, "meditation", "about Meditation", DISC_MEDITATION_MONK, CLASS_MONK},
  {SPEC_TRAINER_SURVIVAL, "survival", "How to Survive", DISC_SURVIVAL, CLASS_RANGER},
  {SPEC_TRAINER_SHAMAN_ARMADILLO, "armadillo", "about the Abilities of the Armadillo", DISC_SHAMAN_ARMADILLO, CLASS_SHAMAN},
  {SPEC_TRAINER_ANIMAL, "animal", "Animal Magic", DISC_ANIMAL, CLASS_RANGER},
  {SPEC_TRAINER_AEGIS, "aegis", "the Aegis of the Deities", DISC_AEGIS, CLASS_CLERIC},
  {SPEC_TRAINER_SHAMAN, "shaman", "The Ways of the Shaman", DISC_SHAMAN, CLASS_SHAMAN},
  {SPEC_TRAINER_MAGE, "mage", "the arts of Magic", DISC_MAGE, CLASS_MAGE},
  {SPEC_TRAINER_MONK, "monk", "the ways of the Monk", DISC_MONK, CLASS_MONK},
  {SPEC_TRAINER_CLERIC, "cleric", "the Ways of the Cleric", DISC_CLERIC, CLASS_CLERIC},
  {SPEC_TRAINER_THIEF, "thief", "the Ways of the Thief", DISC_THIEF, CLASS_THIEF},
  {SPEC_TRAINER_PLANTS, "plants", "about Plants and Herbs", DISC_PLANTS, CLASS_RANGER},
  {SPEC_TRAINER_PHYSICAL, "physical", "Physical Prowess", DISC_PHYSICAL, CLASS_WARRIOR},
  {SPEC_TRAINER_SMYTHE, "smythe", "Smythe Skills", DISC_SMYTHE, CLASS_WARRIOR},
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
  {SPEC_TRAINER_RANGER_FIGHT, "fighting", "Fighting Skills for Rangers", DISC_RANGER_FIGHT, CLASS_RANGER},
  {SPEC_TRAINER_STEALTH, "stealth", "about Stealthiness", DISC_STEALTH, CLASS_THIEF},
  {SPEC_TRAINER_TRAPS, "traps", "about Locks and Traps", DISC_TRAPS, CLASS_THIEF},
  {SPEC_TRAINER_LORE, "lore", "about Magic Lores", DISC_LORE, CLASS_MAGIC_USER | CLASS_MAGE_THIEF},
 {SPEC_TRAINER_THEOLOGY, "theology", "about Theology", DISC_THEOLOGY, CLASS_CLERIC | CLASS_DEIKHAN},
  {SPEC_TRAINER_DEFENSE, "defense", "about Defense", DISC_DEFENSE, CLASS_WARRIOR | CLASS_RANGER | CLASS_DEIKHAN | CLASS_MONK},
  {SPEC_TRAINER_PSIONICS, "psionics", "about psionics", DISC_PSIONICS, CLASS_WARRIOR | CLASS_RANGER | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_MAGE_THIEF | CLASS_THIEF | CLASS_SHAMAN},
  {SPEC_TRAINER_MAGE_THIEF, "mt", "about mages and thieves", DISC_MAGE_THIEF, CLASS_MAGE_THIEF},
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
  classIndT accclass;

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
      vlogf(LOG_BUG, "TrainerMob lacked setup in TrainerInfo array (%s)", 
             me->getName());
      return FALSE;
    }
  }

  arg = one_argument(arg, discbuf);
  if (!discbuf || !*discbuf || !is_abbrev(discbuf , TrainerInfo[offset].abbrev)) {
    sprintf(buf, "I teach %s.", TrainerInfo[offset].art);
    me->doSay(buf);
    sprintf(buf,
         "Type \"practice %s <number> <class>\" to learn this discipline.", 
         TrainerInfo[offset].abbrev);
    me->doSay(buf);
    return FALSE;
  }
  arg = one_argument(arg, pracbuf);
  if (!*pracbuf || !(pracs = atoi(pracbuf))) {
    sprintf(buf,
         "Type \"practice %s <number> <class>\" to learn this discipline.", 
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
  arg = one_argument(arg, classbuf);

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
  else if (var == CLASS_MAGIC_USER)
      accclass = MAGE_LEVEL_IND;
  else if (var == CLASS_MAGE_THIEF)
      accclass = MAGE_THIEF_LEVEL_IND;
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
    sprintf(buf, "%s You need to specify a class.", fname(ch->name).c_str());
    me->doTell(buf);
    sprintf(buf,
         "Type \"practice %s <number> <class>\" to learn this discipline.", 
         TrainerInfo[offset].abbrev);
    me->doSay(buf);
    return TRUE;
  } else {
    if ((is_abbrev(classbuf, "mage") || is_abbrev(classbuf, "magicuser")) &&
         ch->hasClass(CLASS_MAGIC_USER))
      accclass = MAGE_LEVEL_IND;
    else if ((is_abbrev(classbuf, "cleric")) && ch->hasClass(CLASS_CLERIC))
      accclass = CLERIC_LEVEL_IND;
    else if ((is_abbrev(classbuf, "magethief")) && ch->hasClass(CLASS_MAGE_THIEF))
      accclass = MAGE_THIEF_LEVEL_IND;
    else if ((is_abbrev(classbuf, "thief")) && ch->hasClass(CLASS_THIEF))
      accclass = THIEF_LEVEL_IND;
    else if ((is_abbrev(classbuf, "warrior") || (is_abbrev(classbuf, "fighter")))
           && ch->hasClass(CLASS_WARRIOR))
      accclass = WARRIOR_LEVEL_IND;
    else if ((is_abbrev(classbuf, "deikhan")) && ch->hasClass(CLASS_DEIKHAN))
      accclass = DEIKHAN_LEVEL_IND;
    else if ((is_abbrev(classbuf, "ranger")) && ch->hasClass(CLASS_RANGER))
      accclass = RANGER_LEVEL_IND;
    else if ((is_abbrev(classbuf, "monk")) && ch->hasClass(CLASS_MONK))
      accclass = MONK_LEVEL_IND;
    else if ((is_abbrev(classbuf, "shaman")) && ch->hasClass(CLASS_SHAMAN))
      accclass = SHAMAN_LEVEL_IND;
    else {
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
     sprintf(buf, "%s I also would not be able to train you further in this discipline.", fname(ch->name).c_str());
      me->doTell(buf);
    }
    return TRUE;
  }
 
  if (practices <= 0) {
    sprintf(buf, "%s You have come far.  I can train you no farther in this discipline.", fname(ch->name).c_str());
    me->doTell(buf);
    sprintf(buf, "%s You must find another master who can further your training.", fname(ch->name).c_str());
    me->doTell(buf);
    return TRUE;
  }

  // set the number they actually have as another limiting factor
  practices = min((int) ch->getPracs(accclass), practices);
  if (practices <= 0) {
    sprintf(buf, "%s You have no more practices you can use here.", fname(ch->name).c_str());
    me->doTell(buf);
    return TRUE;
  } else if (practices < pracs) {
     sprintf(buf, "%s I will only be able to use %d of your requested practices.", fname(ch->name).c_str(), practices);
      me->doTell(buf);
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
    case MAGE_THIEF_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness() + ch->getDiscipline(DISC_LORE)->getNatLearnedness();
      bas = ch->getDiscipline(DISC_MAGE_THIEF)->getNatLearnedness();
      break;
    case THIEF_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
      bas = ch->getDiscipline(DISC_THIEF)->getNatLearnedness();
      break;
    default:
      vlogf(LOG_BUG,"Wierd case in checkDoneBasic %d", accclass);
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
  int bakpracs = 0;
  int trainLevel = 0, discLearn = 0;
//  char buf[256];
  if (accclass && pracs) {
  }
  trainLevel = me->GetMaxLevel();
  discLearn = ch->getDiscipline(discipline)->getNatLearnedness();

  if ((discipline == DISC_COMBAT) || (discipline == DISC_LORE) ||
      (discipline == DISC_THEOLOGY)) {
    bakpracs = trainLevel - discLearn;
  } else if (discipline == DISC_SHAMAN || discipline == DISC_MAGE || 
	     discipline == DISC_MAGE_THIEF ||
             discipline == DISC_CLERIC || discipline == DISC_WARRIOR ||
             discipline == DISC_RANGER || discipline == DISC_DEIKHAN ||
             discipline == DISC_MONK || discipline == DISC_THIEF) {
    bakpracs = trainLevel - discLearn;
  } else if (((discipline == DISC_SLASH) || (discipline == DISC_PIERCE) ||
              (discipline == DISC_BLUNT) || (discipline == DISC_RANGED) ||
	      (discipline == DISC_BAREHAND)|| (discipline == DISC_DEFENSE) ||
	      (discipline == DISC_PSIONICS)) &&
              (ch->getDiscipline(DISC_COMBAT)->getNatLearnedness() < MAX_DISC_LEARNEDNESS)) {
    if (trainLevel == discLearn) 
      bakpracs = 0;
    else 
      bakpracs = max(1, ((trainLevel - discLearn) / 5));
  } else {
    bakpracs = ch->pracsBetween(discipline, me->GetMaxLevel());
  }
  return bakpracs;
}

int TBeing::checkTrainDeny(const TBeing *ch, TMonster *me, discNumT discipline, int pracs) const
{
  char buf[256];

  if ((ch->getDiscipline(discipline))->getNatLearnedness() >= MAX_DISC_LEARNEDNESS) {
    sprintf(buf, "%s You are already fully learned in this discipline.", fname(ch->name).c_str());
    me->doTell(buf);
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
  char tmp_buf[40];
  bool found = FALSE;
  int combat = 0;
  bool combatLearn = FALSE;
  int WEAPON_GAIN_LEARNEDNESS = 92;
  pracs = 1;

 if (discipline == DISC_BAREHAND) {
   if (ch->getRawNatSkillValue(SKILL_BAREHAND_PROF) < WEAPON_GAIN_LEARNEDNESS) {
     sprintf(buf, " %s You aren't proficient enough yet.", fname(ch->name).c_str());
     me->doTell(buf);
     return TRUE;
   }
 }
 if (discipline == DISC_SLASH) {
   if (ch->getRawNatSkillValue(SKILL_SLASH_PROF) < WEAPON_GAIN_LEARNEDNESS) {
      sprintf(buf, " %s You aren't proficient enough yet.", fname(ch->name).c_str());
      me->doTell(buf);
      return TRUE;
    }
  }
  if (discipline == DISC_PIERCE) {
    if (ch->getRawNatSkillValue(SKILL_PIERCE_PROF) < WEAPON_GAIN_LEARNEDNESS) {
      sprintf(buf, " %s You aren't proficient enough yet.", fname(ch->name).c_str());
      me->doTell(buf);
      return TRUE;
    }
  }
  if (discipline == DISC_BLUNT) {
    if (ch->getRawNatSkillValue(SKILL_BLUNT_PROF) < WEAPON_GAIN_LEARNEDNESS) {
      sprintf(buf, " %s You aren't proficient enough yet.", fname(ch->name).c_str());
      me->doTell(buf);
      return TRUE;
    }
  }
  if (discipline == DISC_RANGED) {
    if (ch->getRawNatSkillValue(SKILL_RANGED_PROF) < WEAPON_GAIN_LEARNEDNESS) {
      sprintf(buf, " %s You aren't proficient enough yet.", fname(ch->name).c_str());
      me->doTell(buf);
      return TRUE;
    }
  }
  if (discipline == DISC_DEFENSE) {
    if (ch->getRawNatSkillValue(SKILL_DEFENSE) < WEAPON_GAIN_LEARNEDNESS) {
      sprintf(buf, " %s You aren't proficient enough yet.", fname(ch->name).c_str());
      me->doTell(buf);
      return TRUE;
    }
  }
  if (discipline == DISC_PSIONICS) {
    if (!ch->hasQuestBit(TOG_PSIONICIST)){
      sprintf(buf, " %s You do not have the ability to learn psionics.", fname(ch->name).c_str());
      me->doTell(buf);
      return TRUE;
    }

  }


  if (!prereqs) {
  // no prereqs
    return FALSE;
  } 

  switch (accclass) {
    case MAGE_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness() + ch->getDiscipline(DISC_LORE)->getNatLearnedness();
      if ((combat >= 100) || (combat >= (((35*ch->getLevel(MAGE_LEVEL_IND)) /10) - 4))) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat or Magic Lores");
      break;
   case SHAMAN_LEVEL_IND:
     combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
     if ((combat >= 100) || (combat >= (((35*ch->getLevel(SHAMAN_LEVEL_IND)) /10) - 4))) {
       combatLearn = TRUE;
     }
     strcpy(tmp_buf, "Combat");
     break;
   case RANGER_LEVEL_IND:
     combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
     if ((combat >= 100) || (combat >= (((35*ch->getLevel(RANGER_LEVEL_IND)) /10) - 4))) {
       combatLearn = TRUE;
     }
     strcpy(tmp_buf, "Combat");
     break;
   case CLERIC_LEVEL_IND:
     combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness() + ch->getDiscipline(DISC_THEOLOGY)->getNatLearnedness();
     if ((combat >= 100) || (combat >= (((35*ch->getLevel(CLERIC_LEVEL_IND)) /10) - 4))) {
       combatLearn = TRUE;
     }
     strcpy(tmp_buf, "Combat or Theology");
     break;
   case DEIKHAN_LEVEL_IND:
     combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness() + ch->getDiscipline(DISC_THEOLOGY)->getNatLearnedness();
     if ((combat >= 100) || (combat >= (((35*ch->getLevel(DEIKHAN_LEVEL_IND)) /10) - 4))) {
       combatLearn = TRUE;
     }
     strcpy(tmp_buf, "Combat or Theology");
     break;
   case WARRIOR_LEVEL_IND:
     combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
     if ((combat >= 100) || (combat >= (((35*ch->getLevel(WARRIOR_LEVEL_IND)) /10) - 4))) {
       combatLearn = TRUE;
     }
     strcpy(tmp_buf, "Combat");
     break;
   case THIEF_LEVEL_IND:
     combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
     if ((combat >= 100) || (combat >= (((35*ch->getLevel(THIEF_LEVEL_IND)) /10) - 4))) {
       combatLearn = TRUE;
     }
     strcpy(tmp_buf, "Combat");
     break;
   case MONK_LEVEL_IND:
     combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness();
     if ((combat >= 100) || (combat >= (((35*ch->getLevel(MONK_LEVEL_IND)) /10) - 4))) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat");
      break;
    case MAGE_THIEF_LEVEL_IND:
      combat = ch->getDiscipline(DISC_COMBAT)->getNatLearnedness() + ch->getDiscipline(DISC_LORE)->getNatLearnedness();
      if ((combat >= 100) || (combat >= (((35*ch->getLevel(MAGE_THIEF_LEVEL_IND)) /10) - 4))) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat or Magic Lores");
      break;
    case UNUSED2_LEVEL_IND:
    case UNUSED3_LEVEL_IND:
    case MAX_SAVED_CLASSES:
      break;
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
                  discipline == DISC_PSIONICS) {
    // No restrictions on these disciplines if prof maxxed see first checks
    return FALSE;
  } else {  // needs basic skills for class
    switch (accclass) {
      case MAGE_LEVEL_IND:
        if ((discipline == DISC_MAGE)) {
          if (combatLearn) {
            return FALSE;
          } else {
            found = 1;
            break;
          }
        } else {
          found = 2;
          break;
        }
        break;
      case MAGE_THIEF_LEVEL_IND:
        if ((discipline == DISC_MAGE_THIEF)) {
          if (combatLearn) {
            return FALSE;
          } else {
            found = 1;
            break;
          }
        } else {
          found = 2;
          break;
        }
        break;
      case SHAMAN_LEVEL_IND:
        if ((discipline == DISC_SHAMAN)) {
          if (combatLearn) {
            return FALSE;
          } else {
            found = 1;
            break;
          }
        } else {
          found = 2;
        }
        break;
      case RANGER_LEVEL_IND:
        if ((discipline == DISC_RANGER)) {
          if (combatLearn) {
            return FALSE;
          } else {
            found = 1;
            break;
          }
        } else {
          found = 2;
        }
        break;
      case DEIKHAN_LEVEL_IND:
        if ((discipline == DISC_DEIKHAN)) {
          if (combatLearn) {
            return FALSE;
          } else {
            found = 1;
            break;
          }
        } else {
          found = 2;
        }
        break;
      case CLERIC_LEVEL_IND:
        if ((discipline == DISC_CLERIC)) {
          if (combatLearn) {
            return FALSE;
          } else {
            found = 1;
            break;
          }
        } else {
          found = 2;
        }
        break;
      case MONK_LEVEL_IND:
        if ((discipline == DISC_MONK)) {
          if (combatLearn) {
            return FALSE;
          } else {
            found = 1;
            break;
          }
        } else {
          found = 2;
        }
        break;
      case THIEF_LEVEL_IND:
        if ((discipline == DISC_THIEF)) {
          if (combatLearn) {
            return FALSE;
          } else {
            found = 1;
            break;
          }
        } else {
          found = 2;
        }
        break;
      case WARRIOR_LEVEL_IND:
        if ((discipline == DISC_WARRIOR)) {
          if (combatLearn) {
            return FALSE;
          } else {
            found = 1;
            break;
          }
        } else {
          found = 2;
        }
        break;
      default:
        vlogf(LOG_BUG, "Bad case in gaining pre requisiites (%d) (%s)", accclass, ch->getName());
        ch->sendTo("Bug that you got this at the gain trainer.");
        return TRUE;
    }
  }

  switch (found) {
    case 2:
      if (combat >= MAX_DISC_LEARNEDNESS) {
        sprintf(buf, " %s Tsk! Tsk! You have not kept up with your basic training and you expect advanced learning.", fname(ch->name).c_str());
        me->doTell(buf);
        sprintf(buf, " %s Hmmm. I think you should learn more from your %s trainer.", fname(ch->name).c_str(), me->getProfName());
      } else if (!combatLearn) {
        sprintf(buf, " %s Tsk! Tsk! You have not kept up with your basic training and you expect advanced learning.", fname(ch->name).c_str());
        me->doTell(buf);
        sprintf(buf, " %s Hmmm. I think you should learn more from the %s trainer.", fname(ch->name).c_str(), tmp_buf);
      } else {
        sprintf(buf, " %s Tsk! Tsk! You have not finished any of your basic training and you expect advanced learning.", fname(ch->name).c_str());
        me->doTell(buf);
        sprintf(buf, " %s Hmmm. I think you should learn more from one of your basic trainers.", fname(ch->name).c_str());
      }
      me->doTell(buf);
      return TRUE;
    case 1:
      sprintf(buf, " %s Tsk! Tsk! You have not kept up with your general training and you expect me to teach you more.", fname(ch->name).c_str());
      me->doTell(buf);
      sprintf(buf, " %s Go learn more about %s before you come back to me.", fname(ch->name).c_str(), tmp_buf);
      me->doTell(buf);
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
    vlogf(LOG_BUG, "Bogus pracs used %s (%d)", ch->getName(), ch->in_room);
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
       sprintf(buf, "%s I can train you no farther in this discipline.", fname(ch->name).c_str());
       me->doTell(buf);
       sprintf(buf, "%s You must find another master who can further your training.", fname(ch->name).c_str());
       me->doTell(buf);

      break;
    }
    // basic maxed check
    if ((ch->getDiscipline(TrainerInfo[offset].disc))->getNatLearnedness() >=
         MAX_DISC_LEARNEDNESS) {
      sprintf(buf, "%s You are now fully trained in this discipline.", 
                  fname(ch->name).c_str());
      me->doTell(buf);
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
      ch->sendTo("You have %d %s practices left.\n\r", 
      ch->getPracs(accclass), classNames[accclass].name);
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
          value = max(value, 1); 
          ch->setNatSkillValue(i, value);
          ch->setSkillValue(i,value);
          ch->affectTotal();
        } else {
          value = max(1, discArray[i]->learn);
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
          dynamic_cast<TComponent *>(obj)->setComponentMaxCharges(15); 
          obj->obj_flags.decay_time = -1;
          *ch += *obj;
          sprintf(buf, "%s Here is a component (%s) for you to help in your learning of %s.", fname(ch->getName()).c_str(), obj->getName(), discArray[i]->name);
          me->doTell(buf);
        }
      } else if (ch->doesKnowSkill(i)) {
        if ((discArray[i]->start <=final) && (ch->getRawSkillValue(i) < 0)) {
          vlogf(LOG_BUG, "%s: ch->doesKnowSkill %s (%d) with no actual learning..could be array change or bug", ch->getName(), discArray[i]->name, i);
          ch->setNatSkillValue(i, 1);
          ch->setSkillValue(i, 1);
        }

        if (discArray[i]->startLearnDo < 0) { // doesnt use learn by doing
          value = (final - discArray[i]->start + 1) * discArray[i]->learn;
//          value = ch->getRawNatSkillValue(i) + discArray[i]->learn;
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
        string tStArg(DisguiseList[tType].name),
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
        string tStArg(ShapeShiftList[tType].name),
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
        value = max(value, 1);
        value=min(value, (int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
      } else if ((discArray[i]->startLearnDo == 0)) {
        value = max(1, discArray[i]->learn);
        value=min(value, (int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
      } else if ((discArray[i]->startLearnDo < 0)) {
        value = max(1, (amount*discArray[i]->learn));
        value = min(value, (int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
      }
    } else if (final >= discArray[i]->start) {
      if (discArray[i]->startLearnDo < 0) { // doesnt use learn by doing
        value = (final - discArray[i]->start + 1) *  discArray[i]->learn;
        value = max(value, 1);
        value = min(value, (int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
      } else if (discArray[i]->startLearnDo == 0) {
        value = max((int) getNatSkillValue(i), discArray[i]->learn);
        value = max(value, 1);
        value = min(value,(int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(i, value);
        setSkillValue(i,value);
      } else {
        value = min((int) discArray[i]->startLearnDo, (discArray[i]->learn * (final + 1 - discArray[i]->start)));
        value = max(1, value);
        value = max(value, (int) getNatSkillValue(i));
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

static int GenericGuildMaster(TBeing *ch, TMonster *me, cmdTypeT cmd, classIndT cit, ush_int Class)
{
  if (cmd == CMD_GENERIC_PULSE)
    me->aiMaintainCalm();

  if (cmd == CMD_PRACTICE) {
    act("$n says, \"Sorry, you must visit a trainer to practice something.\"",
         FALSE, me, 0, ch, TO_VICT);
    return TRUE;
  }

  if (cmd != CMD_GAIN)
    return FALSE;

  if (ch->isImmortal() || !me)
    return FALSE;

  if (generic_trainer_stuff(me, ch))
    return TRUE;

  if (!ch->hasClass(Class)) {
    switch (cit) {
      case MAGE_LEVEL_IND:
      case SHAMAN_LEVEL_IND:
        act("$n growls, \"Go away, $N.  You know nothing of spirits!\"", FALSE, me, 0, ch, TO_ROOM);
        break;
      case CLERIC_LEVEL_IND:
        act("$n growls, \"Go away, $N.  You're no cleric!\"", FALSE, me, 0, ch, TO_ROOM);
        break;
      case THIEF_LEVEL_IND:
        act("$n growls, \"Go away, $N.  You're no thief!\"", FALSE, me, 0, ch, TO_ROOM);
        break;
      case DEIKHAN_LEVEL_IND:
        act("$n growls, \"Go away, $N.  You know nothing of our order!\"", FALSE, me, 0, ch, TO_ROOM);
        break;
      case RANGER_LEVEL_IND:
        act("$n growls, \"Go away, $N.  You're no ranger!\"", FALSE, me, 0, ch, TO_ROOM);
        break;
      case WARRIOR_LEVEL_IND:
        act("$n growls, \"Go away, $N.  You're no warrior!\"", FALSE, me, 0, ch, TO_ROOM);
        break;
      case MONK_LEVEL_IND:
        act("$n growls, \"Go away, $N.  You are not part of our dojo!\"", FALSE, me, 0, ch, TO_ROOM);
        break;
      case MAGE_THIEF_LEVEL_IND:
        act("$n growls, \"Go away, $N.  You don't know jack about our kind!\"", FALSE, me, 0, ch, TO_ROOM);
        break;
      case UNUSED2_LEVEL_IND:
      case UNUSED3_LEVEL_IND:
      case MAX_SAVED_CLASSES:
        break;
    }
    return TRUE;
  }
  if (ch->getLevel(cit) < (me->GetMaxLevel()/2))
    dynamic_cast<TPerson *>(ch)->raiseLevel(cit, me);
  else if (ch->getLevel(cit) < MAX_MORT)
    act("$n sighs, \"I cannot teach you, $N.  You MUST find your next guildmaster.\"", FALSE, me, 0, ch, TO_ROOM);
  else
    act("$n beams, \"No one can teach you anymore in this, $N\"",
        FALSE, me, NULL, ch, TO_ROOM);
  return TRUE;
}

int ShamanGuildMaster(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  return GenericGuildMaster(ch, me, cmd, SHAMAN_LEVEL_IND, CLASS_SHAMAN);
}

int MageGuildMaster(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  return GenericGuildMaster(ch, me, cmd, MAGE_LEVEL_IND, CLASS_MAGIC_USER);
}

int MageThiefGuildMaster(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  return GenericGuildMaster(ch, me, cmd, MAGE_THIEF_LEVEL_IND, CLASS_MAGE_THIEF);
}

int DeikhanGuildMaster(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  return GenericGuildMaster(ch, me, cmd, DEIKHAN_LEVEL_IND, CLASS_DEIKHAN);
}

int RangerGuildMaster(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  return GenericGuildMaster(ch, me, cmd, RANGER_LEVEL_IND, CLASS_RANGER);
}

int ClericGuildMaster(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  return GenericGuildMaster(ch, me, cmd, CLERIC_LEVEL_IND, CLASS_CLERIC);
}

int ThiefGuildMaster(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  return GenericGuildMaster(ch, me, cmd, THIEF_LEVEL_IND, CLASS_THIEF);
}

int WarriorGuildMaster(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  return GenericGuildMaster(ch, me, cmd, WARRIOR_LEVEL_IND, CLASS_WARRIOR);
}

int MonkGuildMaster(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  return GenericGuildMaster(ch, me, cmd, MONK_LEVEL_IND, CLASS_MONK);
}

TMonster *FindMobInRoomWithProcNum(int room, int num)
{
  TThing *t;

  if (room < 0)
    return NULL;

  for (t = real_roomp(room)->getStuff(); t; t = t->nextThing) {
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
  else if ((skill = getSkillValue(SKILL_WIZARDRY)) < 15)
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
  else if ((skill = getSkillValue(SKILL_RITUALISM)) < 15)
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
  else if ((skill = getSkillValue(SKILL_DEVOTION)) < 15)
    return DEV_LEV_SYMB_PRIM_OTHER_FREE;
  else if (skill < 30)
    return DEV_LEV_SYMB_EITHER_OTHER_FREE;
  else if (skill < 40)
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

void TBeing::pracPath(TMonster *gm, classIndT Class, ubyte pracs)
{
  char buf[256];
  char tmp_buf[40];
  char tmp2_buf[40];
  char classbuf[80];
  int combat = 0, basic = 0;
  bool basicLearn = FALSE, combatMax = FALSE,  combatLearn = FALSE;

  strcpy(classbuf, classNames[Class].name);
  sprintf(buf, "Here are %d %s practice%s.", pracs,
       uncap(classbuf), (pracs == 1 ? "" : "s"));
  gm->doSay(buf);

  if (!checkDoneBasic(this, Class, FALSE, FALSE)) {
    sprintf(buf, "Hmmm, looks like you are free to use these practices at any advanced %s trainer.", gm->getProfName());
    gm->doSay(buf);
    return;
  }
  switch (Class) {
    case MAGE_LEVEL_IND:
      combat = getDiscipline(DISC_COMBAT)->getNatLearnedness() + getDiscipline(DISC_LORE)->getNatLearnedness();
      basic = getDiscipline(DISC_MAGE)->getNatLearnedness();
      if (basic >= MAX_DISC_LEARNEDNESS) {
        basicLearn=TRUE;
      }
      if (combat >= getCombatPrereqNumber(Class)) {
        combatLearn = TRUE;
        combatMax = 1;
      }
      if (combat == 2* MAX_DISC_LEARNEDNESS) {
         combatMax = 2;
         combatLearn = TRUE;
      }
      if (combat >= (((35 * getLevel(Class)) /10) - 4)) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat or Magic Lores");
      strcpy(tmp2_buf, "Magic Lores");
      break;
    case MAGE_THIEF_LEVEL_IND:
      combat = getDiscipline(DISC_COMBAT)->getNatLearnedness() + getDiscipline(DISC_LORE)->getNatLearnedness();
      basic = getDiscipline(DISC_MAGE_THIEF)->getNatLearnedness();
      if (basic >= MAX_DISC_LEARNEDNESS) {
        basicLearn=TRUE;
      }
      if (combat >= getCombatPrereqNumber(Class)) {
        combatLearn = TRUE;
        combatMax = 1;
      }
      if (combat == 2* MAX_DISC_LEARNEDNESS) {
         combatMax = 2;
         combatLearn = TRUE;
      }
      if (combat >= (((35 * getLevel(Class)) /10) - 4)) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat or Magic Lores");
      strcpy(tmp2_buf, "Magic Lores");
      break;
    case SHAMAN_LEVEL_IND:
      combat = getDiscipline(DISC_COMBAT)->getNatLearnedness();
      basic = getDiscipline(DISC_WARRIOR)->getNatLearnedness();
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

      if (combat >= (((35*getLevel(Class)) /10) - 4)) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat");
      break;
    case RANGER_LEVEL_IND:
      combat = getDiscipline(DISC_COMBAT)->getNatLearnedness();
      basic = getDiscipline(DISC_RANGER)->getNatLearnedness();
      if (basic >= MAX_DISC_LEARNEDNESS) {
        basicLearn = TRUE; 
      }
      if (combat >= getCombatPrereqNumber(Class)) {
        combatLearn = TRUE;
        combatMax = 1;
      }
      if (combat == MAX_DISC_LEARNEDNESS) {
         combatMax = 2;
         combatLearn = TRUE;
      }

      if (combat >= (((35*getLevel(Class)) /10) - 4)) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat");
      break;
   case CLERIC_LEVEL_IND:  
      combat = getDiscipline(DISC_COMBAT)->getNatLearnedness() + getDiscipline(DISC_THEOLOGY)->getNatLearnedness();
      basic = getDiscipline(DISC_CLERIC)->getNatLearnedness();
      if (basic >= MAX_DISC_LEARNEDNESS) {
        basicLearn=TRUE;
      }
      if (combat >= getCombatPrereqNumber(Class)) {
        combatLearn = TRUE;
        combatMax = 1;
      }
      if (combat == 2* MAX_DISC_LEARNEDNESS) {
         combatMax = 2;
         combatLearn = TRUE;
      }

      if (combat >= (((35*getLevel(Class)) /10) - 4)) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat or Theology");
      strcpy(tmp2_buf, "Theology");
       break;
   case DEIKHAN_LEVEL_IND:
      combat = getDiscipline(DISC_COMBAT)->getNatLearnedness() + getDiscipline(DISC_THEOLOGY)->getNatLearnedness();
      basic = getDiscipline(DISC_DEIKHAN)->getNatLearnedness();
      if (basic >= MAX_DISC_LEARNEDNESS) {
        basicLearn = TRUE;
      }
      if (combat >= getCombatPrereqNumber(Class)) {
        combatLearn = TRUE;
        combatMax = 1;
      }
      if (combat == 2* MAX_DISC_LEARNEDNESS) {
         combatMax = 2;
         combatLearn = TRUE;
      }

      if (combat >= (((35*getLevel(Class)) /10) - 4)) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat or Theology");
      strcpy(tmp2_buf, "Theology");
      break;
    case WARRIOR_LEVEL_IND:
      combat = getDiscipline(DISC_COMBAT)->getNatLearnedness();
      basic = getDiscipline(DISC_WARRIOR)->getNatLearnedness();
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

      if (combat >= (((35*getLevel(Class)) /10) - 4)) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat");
      break;
    case MONK_LEVEL_IND:
      combat = getDiscipline(DISC_COMBAT)->getNatLearnedness();
      basic = getDiscipline(DISC_MONK)->getNatLearnedness();
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

      if (combat >= (((35*getLevel(Class)) /10) - 4)) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat");
      break;
    case THIEF_LEVEL_IND:
      combat = getDiscipline(DISC_COMBAT)->getNatLearnedness();
      basic = getDiscipline(DISC_THIEF)->getNatLearnedness();
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
      if (combat >= (((35*getLevel(Class)) /10) - 4)) {
        combatLearn = TRUE;
      }
      strcpy(tmp_buf, "Combat");
      break;
    case UNUSED2_LEVEL_IND:
    case UNUSED3_LEVEL_IND:
    case MAX_SAVED_CLASSES:
      forceCrash("bad spot in pracPath");
      return;
  }


  if (combatMax && basicLearn) {
    sprintf(buf, "Hmmm, looks like you are free to use these practices at any advanced %s trainer.", gm->getProfName());
  } else if (!combatLearn) {
      sprintf(buf, "I think you need to first use some of these practices at your %s trainer.", tmp_buf);
  } else if (!combatMax && !basicLearn) {
    if (getDiscipline(DISC_COMBAT)->getNatLearnedness() >= MAX_DISC_LEARNEDNESS)
      sprintf(buf, "You could train in a weapon speciality or continue to use these practices at your %s or %s trainer.", gm->getProfName(),tmp2_buf);
    else
      sprintf(buf, "You can use these practices at your %s trainer or you could always continue to train in %s.", gm->getProfName(), tmp_buf);

  } else if (!combatMax && basicLearn) {
    if (getDiscipline(DISC_COMBAT)->getNatLearnedness() >= MAX_DISC_LEARNEDNESS)
      sprintf(buf, "You could train in a weapon speciality or continue to use these practices at your %s trainer.", tmp2_buf);
    else
      sprintf(buf, "You need to use these practices to finish your basic training at your %s trainer.", tmp_buf); 
  } else if (combatMax && !basicLearn) {
    if (getDiscipline(DISC_COMBAT)->getNatLearnedness() >= MAX_DISC_LEARNEDNESS) {
     sprintf(buf, "You should use these practices at your basic %s trainer or you can pursue a weapon specialization.", gm->getProfName());
    } else if (combatMax == 1) {
      sprintf(buf, "You can use these practices at your basic %s trainer or you could always continue to train in %s.", gm->getProfName(), tmp_buf);
    } else if (combatMax == 2) {
      sprintf(buf, "You need to use these practices at your basic %s trainer or you could pursue a weapon specialization.", gm->getProfName());
    }
  } else {
    vlogf(LOG_BUG, "Bad case in pracPath for %s", getName());
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

  return (int) exp_amt;
}





