#include "stdsneezy.h"
#include <algorithm>

vector<discNumT>disclist;

void TBeing::assignSkillsClass()
{
  int i;
  sh_int pracs;
  classIndT Class;
  discNumT prim;
  int value;
  CDiscipline *cd;
  vector<discNumT>favorites;

  mud_assert(discs != NULL, "Somehow got to assignSkillsClass without a discs %s", getName());

  // first, lets assign some "free" discs
  // keep track of how much free stuff we gave out though
  unsigned int freebies = 0;
  if ((cd = getDiscipline(DISC_ADVENTURING))) {
    value = min((3*(GetMaxLevel())), 100);
    cd->setNatLearnedness(value);
    cd->setLearnedness(value);
    freebies += value/3;
  }
  if (hasClass(CLASS_MAGE)) {
    if ((cd = getDiscipline(DISC_WIZARDRY))) {
      value = min((3*(GetMaxLevel())), 100);
      cd->setNatLearnedness(value);
      cd->setLearnedness(value);
      freebies += value/3;
    }
    if ((cd = getDiscipline(DISC_LORE))) {
      value = min((3*(GetMaxLevel())), 100);
      cd->setNatLearnedness(value);
      cd->setLearnedness(value);
      freebies += value/3;
    }
  } else if (hasClass(CLASS_SHAMAN)) {
    if ((cd = getDiscipline(DISC_RITUALISM))) {
      value = min((3*(GetMaxLevel())), 100);
      cd->setNatLearnedness(value);
      cd->setLearnedness(value);
      freebies += value/3;
    }
  } else if (hasClass(CLASS_CLERIC) || hasClass(CLASS_DEIKHAN)) {
    if ((cd = getDiscipline(DISC_THEOLOGY))) {
      value = min((3*(GetMaxLevel())), 100);
      cd->setNatLearnedness(value);
      cd->setLearnedness(value);
      freebies += value/3;
    }
  }
  for (Class = MIN_CLASS_IND; Class < MAX_CLASSES; Class++) {
    pracs = 0;
    if (!hasClass(1<<Class))
      continue;
    // a new char is started at level 0 and advanced 1 level automatically
    for (i = 1; i <= getLevel(Class); i++) {
      // adjust for basic/ vs spelcialization practices
      if (pracs < 200) 
        pracs += calcNewPracs(Class, true);
      else 
        pracs += calcNewPracs(Class, false);
    }
    // but discount for the freebies given above
    // PCs get these for free at gain time, may as well not charge mobs
    //    pracs -= freebies;

    setPracs(max((int) pracs, 0), Class);
  }

  if(hasClass(CLASS_MAGE)){
    Class=MAGE_LEVEL_IND;
    prim=DISC_MAGE;
    favorites.clear();
    favorites.push_back(DISC_SORCERY);
    favorites.push_back(DISC_FIRE);
    favorites.push_back(DISC_WATER);
    favorites.push_back(DISC_EARTH);
    favorites.push_back(DISC_AIR);
    assignSkills(Class, prim, favorites);
  }

  if (hasClass(CLASS_CLERIC)){
    Class=CLERIC_LEVEL_IND;
    prim=DISC_CLERIC;
    favorites.clear();
    favorites.push_back(DISC_CURES);
    favorites.push_back(DISC_WRATH);
    favorites.push_back(DISC_AFFLICTIONS);
    assignSkills(Class, prim, favorites);
  }

  if (hasClass(CLASS_WARRIOR)){
    Class=WARRIOR_LEVEL_IND;
    prim=DISC_WARRIOR;
    favorites.clear();
    favorites.push_back(DISC_BRAWLING);
    favorites.push_back(DISC_DUELING);
    favorites.push_back(DISC_SOLDIERING);
    assignSkills(Class, prim, favorites);
  }

  if (hasClass(CLASS_THIEF)){
    Class=THIEF_LEVEL_IND;
    prim=DISC_THIEF;
    favorites.clear();
    favorites.push_back(DISC_MURDER);
    favorites.push_back(DISC_THIEF_FIGHT);
    assignSkills(Class, prim, favorites);
  }

  if (hasClass(CLASS_DEIKHAN)){
    Class=DEIKHAN_LEVEL_IND;
    prim=DISC_DEIKHAN;
    favorites.clear();
    favorites.push_back(DISC_DEIKHAN_FIGHT);
    favorites.push_back(DISC_MOUNTED);
    favorites.push_back(DISC_DEIKHAN_WRATH);
    assignSkills(Class, prim, favorites);
  }

  if (hasClass(CLASS_MONK)){
    Class=MONK_LEVEL_IND;
    prim=DISC_MONK;
    favorites.clear();
    favorites.push_back(DISC_LEVERAGE);
    favorites.push_back(DISC_MEDITATION_MONK);
    favorites.push_back(DISC_MINDBODY);
    favorites.push_back(DISC_FOCUSED_ATTACKS);
    favorites.push_back(DISC_IRON_BODY);
    assignSkills(Class, prim, favorites);
  }
  
  if (hasClass(CLASS_SHAMAN)){
    Class=SHAMAN_LEVEL_IND;
    prim=DISC_SHAMAN;
    favorites.clear();
    favorites.push_back(DISC_SHAMAN_CONTROL);
    favorites.push_back(DISC_SHAMAN_ARMADILLO);
    favorites.push_back(DISC_SHAMAN_FROG);
    favorites.push_back(DISC_SHAMAN_SPIDER);
    assignSkills(Class, prim, favorites);
  }


  affectTotal();
  discNumT disc_num;
  for (disc_num = MIN_DISC; disc_num < MAX_DISCS; disc_num++) {
    if (!(cd = getDiscipline(disc_num))) 
      continue;   
    if (cd->getNatLearnedness() < 0) {
      continue;
    } 
    initSkillsBasedOnDiscLearning(disc_num);
  }
}

void TBeing::assignSkills(classIndT Class, discNumT primDisc,
			  vector<discNumT>favorites)
{
  CDiscipline *cd, *prim, *combat, *favored=NULL;
  discNumT favoredNum=DISC_NONE, discnum;
  
  if(!(prim=getDiscipline(primDisc))){
    vlogf(LOG_BUG, fmt("%s didn't have prim discipline.") %  getName());
    return;
  }

  if(!(combat=getDiscipline(DISC_COMBAT))){
    vlogf(LOG_BUG, fmt("%s didn't have combat discipline.") %  getName());
    return;
  }

  // make a list of disciplines that we have
  // we'll use this later to randomly choose discs to practice
  disclist.clear();
  for (discnum=MIN_DISC; discnum < MAX_DISCS; discnum++) {
    cd = getDiscipline(discnum);
    if(cd && cd->ok_for_class){
      disclist.push_back(discnum);
    }
  }

  while (getPracs(Class) > 0) {
    addPracs(-1, Class);
    
    // Get combat (tactics) up to a minimum
    if(combat->getLearnedness() < 15) {
      raiseDiscOnce(DISC_COMBAT);
      continue;
    }

    // start raising combat and prim until they are both maxed
    bool combatDone=(combat->getLearnedness() >= MAX_DISC_LEARNEDNESS);
    bool primDone=(prim->getLearnedness() >= MAX_DISC_LEARNEDNESS);
    
    if(combatDone && !primDone){
      raiseDiscOnce(primDisc);
      continue;
    } else if(!combatDone && primDone){
      raiseDiscOnce(DISC_COMBAT);
      continue;
    } else if(!combatDone && !primDone){
      if(::number(0,1))
	raiseDiscOnce(primDisc);
      else
	raiseDiscOnce(DISC_COMBAT);
      continue;
    }
    // fall through only if combat and prim are both maxed


    // first, let's choose a favored disc, this is our "specialization"
    if(!favored){
      std::random_shuffle(favorites.begin(), favorites.end());
      favoredNum=favorites[0];
      
      if(!(favored = getDiscipline(favoredNum))){
	vlogf(LOG_BUG, fmt("%s didn't have discipline %i.") %  getName() % favoredNum);
	return;
      }
    }

    // learn our favored disc up a bit
    if(favored->getLearnedness() < 75){
      raiseDiscOnce(favoredNum);
      continue;
    }

    // now let's learn some random discs
    // first, shuffle our list
    std::random_shuffle(disclist.begin(), disclist.end());

    // now go down the list and practice the first one that isn't maxed
    bool found=false;
    for(unsigned int i=0;i < disclist.size();++i){
      if(!(cd=getDiscipline(disclist[i]))){
	vlogf(LOG_BUG, fmt("%s didn't have discipline %i.") %  getName() %disclist[i]);
	return;
      }
      if(cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	raiseDiscOnce(disclist[i]);
	found=true;
	break;
      }
    }
    if(found)
      continue;
    
    // fall through means we maxed all disciplines
    break;
  }
}




