#include "stdsneezy.h"

void TBeing::assignSkillsClass()
{
  int i;
  sh_int pracs;
  classIndT Class;
  int value;
  CDiscipline *cd;

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

  if(hasClass(CLASS_MAGE))
    assignSkillsMage();

  if (hasClass(CLASS_CLERIC))
    assignSkillsCleric();

  if (hasClass(CLASS_WARRIOR))
    assignSkillsWarrior();

  if (hasClass(CLASS_THIEF))
    assignSkillsThief();

  if (hasClass(CLASS_DEIKHAN))
    assignSkillsDeikhan();

  if (hasClass(CLASS_MONK))
    assignSkillsMonk();

  if (hasClass(CLASS_RANGER))
    assignSkillsRanger();

  if (hasClass(CLASS_SHAMAN))
    assignSkillsShaman();


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

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////


void TBeing::assignSkillsWarrior(){
  int found = FALSE, num;
  CDiscipline *cd;

// 2.  Now use practices

    while (getPracs(WARRIOR_LEVEL_IND) > 0) {
      addPracs(-1, WARRIOR_LEVEL_IND);
      found = FALSE;
 
// 3. Get combat (tactics) up to some minimum
      if ((cd = getDiscipline(DISC_COMBAT)) &&
           cd->getLearnedness() < 15) {

        raiseDiscOnce(DISC_COMBAT);
        continue;
// 4.  Check to see if Basic Discipline is Maxxed    
      } else if ((cd = getDiscipline(DISC_WARRIOR)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {

// 5.  If combat has been maxxed, then put everything into basic

        if ((cd = getDiscipline(DISC_COMBAT)) &&
             cd->getLearnedness() >= MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_WARRIOR);
          continue; 
// 6.  If combat is not maxxed, then divide it evenly 
        } else {
          if (::number(0,1)) {
            raiseDiscOnce(DISC_WARRIOR);
            continue;
          } else {
            raiseDiscOnce(DISC_COMBAT);
            continue;
          }
        }  
// 7.  Check to make sure that combat is maxxed before moving to spec 
      } else if ((cd = getDiscipline(DISC_COMBAT)) &&
                  cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
        raiseDiscOnce(DISC_COMBAT);
        continue;
// 8.  SPECIALIZATIONS NEXT.

// 9.   IS there a favored Discipline 

//    COSMO MARK-- TO BE CODED (CHANGES TO MOB FILE REQUIRED) 

// 10.   Divide the learning into the specialites 
      } else {

// 11.   First Favored Disciplines
//    Note that if a disc is maxxed it will drop to the next one
        switch ((::number(1,4))) {
          case 1:
            if ((cd = getDiscipline(DISC_BRAWLING)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {   
              raiseDiscOnce(DISC_BRAWLING);
              found = TRUE;
              break;          
            }
          case 2:
            if ((cd = getDiscipline(DISC_HTH)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_HTH);
              found = TRUE;
              break;
            }
          case 3:
            if ((cd = getDiscipline(DISC_PHYSICAL)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_PHYSICAL);
              found = TRUE;
              break;
            }
          default:
            break;
        } 

// 12.  Then remaining disciplines in order.
        num = ::number(0,6);
        if (found) {
          continue;  
        } else if ((cd = getDiscipline(DISC_SLASH)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num <= 0)) {
          raiseDiscOnce(DISC_SLASH);
        } else if ((cd = getDiscipline(DISC_BLUNT)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num <= 1)) {
          raiseDiscOnce(DISC_BLUNT);
        } else if ((cd = getDiscipline(DISC_PIERCE)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num <= 2)) {
          raiseDiscOnce(DISC_PIERCE);
        } else if ((cd = getDiscipline(DISC_SMYTHE)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num <=3)) {
          raiseDiscOnce(DISC_SMYTHE);

        } else if ((cd = getDiscipline(DISC_BRAWLING)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_BRAWLING);
        } else if ((cd = getDiscipline(DISC_HTH)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_HTH);
        } else if ((cd = getDiscipline(DISC_PHYSICAL)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_PHYSICAL);
        } else if ((cd = getDiscipline(DISC_SLASH)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SLASH);
        } else if ((cd = getDiscipline(DISC_BLUNT)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_BLUNT);
        } else if ((cd = getDiscipline(DISC_PIERCE)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_PIERCE);
        } else if ((cd = getDiscipline(DISC_SMYTHE)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SMYTHE);
	} else if ((cd = getDiscipline(DISC_DEFENSE)) &&
		   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
	  raiseDiscOnce(DISC_DEFENSE);
        } else {
        // what disc is left?
// this logs a lot for high level mobs
//          vlogf(LOG_BUG, "Mob (%s) has maxxed all warrior disciplines (assignDisc", getName()); 
          break;
        }
      }
    }

}
void TBeing::assignSkillsDeikhan(){
  int found = FALSE, num;
  CDiscipline *cd;

// 2.  Now use practices

    while (getPracs(DEIKHAN_LEVEL_IND) > 0) {
      addPracs(-1, DEIKHAN_LEVEL_IND);
      found = FALSE;

// 3. Get combat (tactics) up to some minimum
      if ((cd = getDiscipline(DISC_COMBAT)) &&
           cd->getLearnedness() < 15) {

        raiseDiscOnce(DISC_COMBAT);
        continue;
// 4.  Check to see if Basic Discipline is Maxxed
      } else if ((cd = getDiscipline(DISC_DEIKHAN)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {

// 5.  If combat has been maxxed, then put everything into basic


        if ((cd = getDiscipline(DISC_COMBAT)) &&
             cd->getLearnedness() == MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_DEIKHAN);
          continue;
// 6.  If combat is not maxxed, then divide it evenly
        } else {
          if (::number(0,1)) {
            raiseDiscOnce(DISC_DEIKHAN);
            continue;
          } else {
            raiseDiscOnce(DISC_COMBAT);
            continue;
          }
        }
// 7.  Check to make sure that combat is maxxed before moving to spec
      } else if ((cd = getDiscipline(DISC_COMBAT)) &&
                  cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
        raiseDiscOnce(DISC_COMBAT);
        continue;
// 8.  SPECIALIZATIONS NEXT.

// 9.   IS there a favored Discipline
//    COSMO MARK-- TO BE CODED (CHANGES TO MOB FILE REQUIRED)

// 10.   Divide the learning into the specialites
      } else {

// 11.   First Favored Disciplines
//    Note that if a disc is maxxed it will drop to the next one
        switch ((::number(1,6))) {
          case 1:
            if ((cd = getDiscipline(DISC_DEIKHAN_FIGHT)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_DEIKHAN_FIGHT);
              found = TRUE;
              break;
            }
          case 2:
            if ((cd = getDiscipline(DISC_MOUNTED)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_MOUNTED);
              found = TRUE;
              break;
            }
          case 3:
            if ((cd = getDiscipline(DISC_DEIKHAN_WRATH)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_DEIKHAN_WRATH);
              found = TRUE;
              break;
            }
          default:
            break;
        }

// 12.  Then remaining disciplines in order.
        num = ::number(0,5);
        if (found) {
          continue;
        } else if ((cd = getDiscipline(DISC_SLASH)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num == 1)) {
          raiseDiscOnce(DISC_SLASH);
        } else if ((cd = getDiscipline(DISC_DEIKHAN_CURES)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num == 2)) {
          raiseDiscOnce(DISC_DEIKHAN_CURES);
        } else if ((cd = getDiscipline(DISC_DEIKHAN_AEGIS)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num == 3)) {
          raiseDiscOnce(DISC_DEIKHAN_AEGIS);
        } else if ((cd = getDiscipline(DISC_DEIKHAN_FIGHT)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_DEIKHAN_FIGHT);
        } else if ((cd = getDiscipline(DISC_SLASH)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SLASH);
        } else if ((cd = getDiscipline(DISC_DEIKHAN_WRATH)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_DEIKHAN_WRATH);
        } else if ((cd = getDiscipline(DISC_MOUNTED)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_MOUNTED);
        } else if ((cd = getDiscipline(DISC_DEIKHAN_CURES)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_DEIKHAN_CURES);
        } else if ((cd = getDiscipline(DISC_DEIKHAN_CURES)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_DEIKHAN_CURES);
        } else if ((cd = getDiscipline(DISC_BLUNT)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_BLUNT);
	} else if ((cd = getDiscipline(DISC_DEFENSE)) &&
		   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
	  raiseDiscOnce(DISC_DEFENSE);
        } else {
        // what disc is left?
// this logs a lot for high level mobs
//          vlogf(LOG_BUG, "Mob (%s) has maxxed all deikhan disciplines (assignDisc", getName());
          break;
        }
      }
    }

}
void TBeing::assignSkillsRanger(){
  int found = FALSE, num;
  CDiscipline *cd;

// 2.  Now use practices

    while (getPracs(RANGER_LEVEL_IND) > 0) {
      addPracs(-1, RANGER_LEVEL_IND);
      found = FALSE;

// 3. Get combat (tactics) up to some minimum
      if ((cd = getDiscipline(DISC_COMBAT)) &&
          cd->getLearnedness() < 15) {

        raiseDiscOnce(DISC_COMBAT);
        continue;
// 4.  Check to see if Basic Discipline is Maxxed
      } else if ((cd = getDiscipline(DISC_RANGER)) &&
                  cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {

// 5.  If combat has been maxxed, then put everything into basic


        if ((cd = getDiscipline(DISC_COMBAT)) &&
             cd->getLearnedness() == MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_RANGER);
          continue;
// 6.  If combat is not maxxed, then divide it evenly
        } else {
          if (::number(0,1)) {
            raiseDiscOnce(DISC_RANGER);
            continue;
          } else {
            raiseDiscOnce(DISC_COMBAT);
            continue;
          }
        }
// 7.  Check to make sure that combat is maxxed before moving to spec
      } else if ((cd = getDiscipline(DISC_COMBAT)) &&
                  cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
        raiseDiscOnce(DISC_COMBAT);
        continue;
// 8.  SPECIALIZATIONS NEXT.

// 9.   IS there a favored Discipline

//    COSMO MARK-- TO BE CODED (CHANGES TO MOB FILE REQUIRED)

// 10.   Divide the learning into the specialites
      } else {

// 11.   First Favored Disciplines
//    Note that if a disc is maxxed it will drop to the next one
        switch ((::number(1,3))) {
          case 1:
            if ((cd = getDiscipline(DISC_RANGER_FIGHT)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_RANGER_FIGHT);
              found = TRUE;
              break;
            }
            break;
          default:
            break;
        }

// 12.  Then remaining disciplines in order.
        num = ::number(0,6);
        if (found) {
          continue;
        } else if ((cd = getDiscipline(DISC_SLASH)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num <= 3)) {
          raiseDiscOnce(DISC_SLASH);
        } else if ((cd = getDiscipline(DISC_NATURE)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num <= 3)) {
          raiseDiscOnce(DISC_NATURE);
        } else if ((cd = getDiscipline(DISC_RANGED)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num == 2)) {
          raiseDiscOnce(DISC_RANGED);
        } else if ((cd = getDiscipline(DISC_ANIMAL)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num == 4)) {
          raiseDiscOnce(DISC_ANIMAL);
        } else if ((cd = getDiscipline(DISC_PLANTS)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num == 5)) {
          raiseDiscOnce(DISC_PLANTS);
        } else if ((cd = getDiscipline(DISC_RANGER_FIGHT)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_RANGER_FIGHT);
        } else if ((cd = getDiscipline(DISC_SLASH)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SLASH);
        } else if ((cd = getDiscipline(DISC_RANGED)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_RANGED);

        } else if ((cd = getDiscipline(DISC_ANIMAL)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_ANIMAL);
        } else if ((cd = getDiscipline(DISC_PLANTS)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_PLANTS);
        } else if ((cd = getDiscipline(DISC_PIERCE)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_PIERCE);
        } else if ((cd = getDiscipline(DISC_SURVIVAL)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SURVIVAL);
	} else if ((cd = getDiscipline(DISC_DEFENSE)) &&
		   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
	  raiseDiscOnce(DISC_DEFENSE);
        } else {
        // what disc is left?
// this logs a lot for high level mobs
//          vlogf(LOG_BUG, "Mob (%s) has maxxed all ranger disciplines (assignDisc", getName());
          break;
        }
      }
    }

}
void TBeing::assignSkillsThief(){
  int found = FALSE, num;
  CDiscipline *cd;

// 2.  Now use practices

    while (getPracs(THIEF_LEVEL_IND) > 0) {
      addPracs(-1, THIEF_LEVEL_IND);
      found = FALSE;

// 3. Get combat (tactics) up to some minimum
      if ((cd = getDiscipline(DISC_COMBAT)) &&
          cd->getLearnedness() < 15) {

        raiseDiscOnce(DISC_COMBAT);
        continue;
// 4.  Check to see if Basic Discipline is Maxxed
      } else if ((cd = getDiscipline(DISC_THIEF)) &&
                  cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {

// 5.  If combat has been maxxed, then put everything into basic


        if ((cd = getDiscipline(DISC_COMBAT)) &&
             cd->getLearnedness() == MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_THIEF);
          continue;
// 6.  If combat is not maxxed, then divide it evenly
        } else {
          if (::number(0,1)) {
            raiseDiscOnce(DISC_THIEF);
            continue;
          } else {
            raiseDiscOnce(DISC_COMBAT);
            continue;
          }
        }
// 7.  Check to make sure that combat is maxxed before moving to spec
      } else if ((cd = getDiscipline(DISC_COMBAT)) &&
                  cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
        raiseDiscOnce(DISC_COMBAT);
        continue;
// 8.  SPECIALIZATIONS NEXT.

// 9.   IS there a favored Discipline

//    COSMO MARK-- TO BE CODED (CHANGES TO MOB FILE REQUIRED)

// 10.   Divide the learning into the specialites
      } else {

// 11.   First Favored Disciplines
//    Note that if a disc is maxxed it will drop to the next one
        switch ((::number(1,4))) {
          case 1:
            if ((cd = getDiscipline(DISC_MURDER)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_MURDER);
              found = TRUE;
              break;
            }
          case 2:
            if ((cd = getDiscipline(DISC_THIEF_FIGHT)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_THIEF_FIGHT);
              found = TRUE;
              break;
            }
          default:
            break;
        }

// 12.  Then remaining disciplines in order.
        num = ::number(0,3);
        if (found) {
          continue;
        } else if ((cd = getDiscipline(DISC_PIERCE)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num == 1)) {
          raiseDiscOnce(DISC_PIERCE);
        } else if ((cd = getDiscipline(DISC_SLASH)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num == 3)) {
          raiseDiscOnce(DISC_SLASH);
        } else if ((cd = getDiscipline(DISC_MURDER)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_MURDER);
        } else if ((cd = getDiscipline(DISC_THIEF_FIGHT)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_THIEF_FIGHT);
        } else if ((cd = getDiscipline(DISC_PIERCE)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_PIERCE);
        } else if ((cd = getDiscipline(DISC_SLASH)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SLASH);
        } else if ((cd = getDiscipline(DISC_LOOTING)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_LOOTING);
        } else if ((cd = getDiscipline(DISC_TRAPS)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_TRAPS);
        } else if ((cd = getDiscipline(DISC_POISONS)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_POISONS);
        } else {
        // what disc is left?
// this logs a lot for high level mobs
//          vlogf(LOG_BUG, "Mob (%s) has maxxed all thief disciplines (assignDisc", getName());
          break;
        }
      }
    }

}
void TBeing::assignSkillsMonk(){
  int found = FALSE, num;
  CDiscipline *cd;

// 2.  Now use practices

    while (getPracs(MONK_LEVEL_IND) > 0) {
      addPracs(-1, MONK_LEVEL_IND);
      found = FALSE;

// 3. Get combat (tactics) up to some minimum
      if ((cd = getDiscipline(DISC_COMBAT)) &&
           cd->getLearnedness() < 15) {

        raiseDiscOnce(DISC_COMBAT);
        continue;
// 4.  Check to see if Basic Discipline is Maxxed
      } else if ((cd = getDiscipline(DISC_MONK)) &&
                  cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {

// 5.  If combat has been maxxed, then put everything into basic


        if ((cd = getDiscipline(DISC_COMBAT)) &&
             cd->getLearnedness() == MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_MONK);
          continue;
// 6.  If combat is not maxxed, then divide it evenly
        } else {
          if (::number(0,1)) {
            raiseDiscOnce(DISC_MONK);
            continue;
          } else {
            raiseDiscOnce(DISC_COMBAT);
            continue;
          }
        }
// 7.  Check to make sure that combat is maxxed before moving to spec
      } else if ((cd = getDiscipline(DISC_COMBAT)) &&
                  cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
        raiseDiscOnce(DISC_COMBAT);
        continue;
// 8.  SPECIALIZATIONS NEXT.

// 9.   IS there a favored Discipline

//    COSMO MARK-- TO BE CODED (CHANGES TO MOB FILE REQUIRED)

// 10.   Divide the learning into the specialites
      } else {

// 11.   First Favored Disciplines
//    Note that if a disc is maxxed it will drop to the next one
        switch ((::number(1,6))) {
          case 1:
            if ((cd = getDiscipline(DISC_LEVERAGE)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_LEVERAGE);
              found = TRUE;
              break;
            }
          case 2:
            if ((cd = getDiscipline(DISC_MEDITATION_MONK)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_MEDITATION_MONK);
              found = TRUE;
              break;
            }
          case 3:
            if ((cd = getDiscipline(DISC_MINDBODY)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_MINDBODY);
              found = TRUE;
              break;
            }
  	  case 4:
	    if ((cd = getDiscipline(DISC_FOCUSED_ATTACKS)) &&
		cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
	      raiseDiscOnce(DISC_FOCUSED_ATTACKS);
	      found = TRUE;
	      break;
	    }
  	  case 5:
	    if ((cd = getDiscipline(DISC_IRON_BODY)) &&
		cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
	      raiseDiscOnce(DISC_IRON_BODY);
	      found = TRUE;
	      break;
	    }
          default:
            break;
        }

// 12.  Then remaining disciplines in order.
        num = ::number(0,6);
        if (found) {
          continue;
        } else if ((cd = getDiscipline(DISC_BAREHAND)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num == 1)) {
          raiseDiscOnce(DISC_BAREHAND);
        } else if ((cd = getDiscipline(DISC_BAREHAND)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_BAREHAND);
	} else if ((cd = getDiscipline(DISC_DEFENSE)) &&
		   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
	  raiseDiscOnce(DISC_DEFENSE);
        } else {
        // what disc is left?
// this logs a lot for high level mobs
//          vlogf(LOG_BUG, "Mob (%s) has maxxed all monk disciplines (assignDisc", getName());
          break;
        }
      }
    }

}
void TBeing::assignSkillsCleric(){
  int found = FALSE, num;
  CDiscipline *cd;

// 2.  Now use practices

    while (getPracs(CLERIC_LEVEL_IND) > 0) {
      addPracs(-1, CLERIC_LEVEL_IND);
      found = FALSE;

// 3. Get combat (tactics) up to some minimum
      if ((cd = getDiscipline(DISC_COMBAT)) &&
           cd->getLearnedness() < 15) {
        raiseDiscOnce(DISC_COMBAT);
        continue;
// 4.  Check to see if Basic Discipline is Maxxed
      } else if ((cd = getDiscipline(DISC_CLERIC)) &&
                  cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {

// 5.  If combat has been maxxed, then put everything into basic
        if ((cd = getDiscipline(DISC_COMBAT)) &&
             cd->getLearnedness() >= MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_CLERIC);
          continue;
// 6.  If combat is not maxxed, then divide it evenly
        } else {
          if (::number(0,1)) {
            raiseDiscOnce(DISC_CLERIC);
            continue;
          } else {
            raiseDiscOnce(DISC_COMBAT);
            continue;
          }
        }
// 7.  Check to make sure that combat is maxxed before moving to spec
      } else if ((cd = getDiscipline(DISC_COMBAT)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
        raiseDiscOnce(DISC_COMBAT);
        continue;
// 8.  SPECIALIZATIONS NEXT.
// 9.   IS there a favored Discipline

//    COSMO MARK-- TO BE CODED (CHANGES TO MOB FILE REQUIRED)

// 10.   Divide the learning into the specialites
      } else {

// 11.   First Favored Disciplines
//    Note that if a disc is maxxed it will drop to the next one
        switch ((::number(1, 5))) {
          case 1:
            if ((cd = getDiscipline(DISC_CURES)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_CURES);
              found = TRUE;
              break;
            }
          case 2:
            if ((cd = getDiscipline(DISC_WRATH)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_WRATH);
              found = TRUE;
              break;
            }
          case 3:
            if ((cd = getDiscipline(DISC_AFFLICTIONS)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_AFFLICTIONS);
              found = TRUE;
              break;
            }

          default:
            break;
        }

// 12.  Then remaining disciplines in order.
        num = ::number(0,5);
        if (found) {
          continue;
        } else if ((cd = getDiscipline(DISC_BLUNT)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num <= 0)) {
          raiseDiscOnce(DISC_BLUNT);
        } else if ((cd = getDiscipline(DISC_AEGIS)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num <= 2)) {
          raiseDiscOnce(DISC_AEGIS);
        } else if ((cd = getDiscipline(DISC_HAND_OF_GOD)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num <= 4)) {
          raiseDiscOnce(DISC_HAND_OF_GOD);

        } else if ((cd = getDiscipline(DISC_WRATH)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_WRATH);
        } else if ((cd = getDiscipline(DISC_AFFLICTIONS)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_AFFLICTIONS);
        } else if ((cd = getDiscipline(DISC_CURES)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_CURES);
        } else if ((cd = getDiscipline(DISC_BLUNT)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_BLUNT);

        } else if ((cd = getDiscipline(DISC_HAND_OF_GOD)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_HAND_OF_GOD);
        } else if ((cd = getDiscipline(DISC_AEGIS)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_AEGIS);
        } else {
        // what disc is left?
// this logs a lot for high level mobs
//          vlogf(LOG_BUG, "Mob (%s) has maxxed all cleric disciplines (assignDisc", getName());
          break;
        }
      }
    }

}

void TBeing::assignSkillsShaman(){
  int found = FALSE, num;
  CDiscipline *cd;

// 2.  Now use practices

    while (getPracs(SHAMAN_LEVEL_IND) > 0) {
      addPracs(-1, SHAMAN_LEVEL_IND);
      found = FALSE;

// 3. Get combat (tactics) up to some minimum
      if ((cd = getDiscipline(DISC_COMBAT)) &&
           cd->getLearnedness() < 15) {

        raiseDiscOnce(DISC_COMBAT);
        continue;
// 4.  Check to see if Basic Discipline is Maxxed
      } else if ((cd = getDiscipline(DISC_SHAMAN)) &&
                  cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {

// 5.  If combat has been maxxed, then put everything into basic


        if ((cd = getDiscipline(DISC_COMBAT)) &&
             cd->getLearnedness() == MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SHAMAN);
          continue;
// 6.  If combat is not maxxed, then divide it evenly
        } else {
          if (::number(0,1)) {
            raiseDiscOnce(DISC_SHAMAN);
            continue;
          } else {
            raiseDiscOnce(DISC_COMBAT);
            continue;
          }
        }
// 7.  Check to make sure that combat is maxxed before moving to spec
      } else if ((cd = getDiscipline(DISC_COMBAT)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
        raiseDiscOnce(DISC_COMBAT);
        continue;
// 8.  SPECIALIZATIONS NEXT.

// 9.   IS there a favored Discipline

//    COSMO MARK-- TO BE CODED (CHANGES TO MOB FILE REQUIRED)

// 10.   Divide the learning into the specialites
      } else {

// 11.   First Favored Disciplines
//    Note that if a disc is maxxed it will drop to the next one
        switch ((::number(1,6))) {
          case 1:
            if ((cd = getDiscipline(DISC_SHAMAN_CONTROL)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_SHAMAN_CONTROL);
              found = TRUE;
              break;
            }
          case 2:
	    if ((cd = getDiscipline(DISC_SHAMAN_ARMADILLO)) &&
                cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_SHAMAN_ARMADILLO);
              found = TRUE;
              break;
	    }
          case 3:
            if ((cd = getDiscipline(DISC_SHAMAN_FROG)) &&
                 cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_SHAMAN_FROG);
              found = TRUE;
              break;
            }
         case 4:
           if ((cd = getDiscipline(DISC_SHAMAN_SPIDER)) &&
                cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
              raiseDiscOnce(DISC_SHAMAN_SPIDER);
              found = TRUE;
              break;
            }
          default:
            break;
        }

// 12.  Then remaining disciplines in order.
        num = ::number(0,4);
        if (found) {
          continue;
        } else if ((cd = getDiscipline(DISC_BLUNT)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num == 1)) {
          raiseDiscOnce(DISC_BLUNT);
        } else if ((cd = getDiscipline(DISC_SHAMAN_SPIDER)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS && (num == 3)) {
          raiseDiscOnce(DISC_SHAMAN_SPIDER);
        } else if ((cd = getDiscipline(DISC_SHAMAN_HEALING)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SHAMAN_HEALING);
        } else if ((cd = getDiscipline(DISC_SHAMAN_CONTROL)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SHAMAN_CONTROL);
        } else if ((cd = getDiscipline(DISC_SHAMAN_FROG)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SHAMAN_FROG);
	} else if ((cd = getDiscipline(DISC_SHAMAN_ARMADILLO)) &&
	        cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
	  raiseDiscOnce(DISC_SHAMAN_ARMADILLO);
        } else if ((cd = getDiscipline(DISC_BLUNT)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_BLUNT);
        } else if ((cd = getDiscipline(DISC_SHAMAN_SPIDER)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SHAMAN_SPIDER);
        } else if ((cd = getDiscipline(DISC_PIERCE)) &&
                   cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_PIERCE);
        } else if ((cd = getDiscipline(DISC_SHAMAN_SKUNK)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SHAMAN_SKUNK);
        } else if ((cd = getDiscipline(DISC_SHAMAN_ALCHEMY)) &&
                    cd->getLearnedness() < MAX_DISC_LEARNEDNESS) {
          raiseDiscOnce(DISC_SHAMAN_ALCHEMY);
        } else {
        // what disc is left?
// this logs a lot for high level mobs
//          vlogf(LOG_BUG, "Mob (%s) has maxxed all shaman disciplines (assignDisc", getName());
          break;
        }
      }
    }

}



void TBeing::assignSkillsMage(){
  int found = FALSE;
  CDiscipline *cd, *mage, *combat, *favored=NULL;
  discNumT favoredNum=DISC_NONE;

  if(!(mage=getDiscipline(DISC_MAGE))){
    vlogf(LOG_BUG, "%s didn't have mage discipline.", getName());
    return;
  }

  if(!(combat=getDiscipline(DISC_COMBAT))){
    vlogf(LOG_BUG, "%s didn't have combat discipline.", getName());
    return;
  }

  while (getPracs(MAGE_LEVEL_IND) > 0) {
    addPracs(-1, MAGE_LEVEL_IND);
    found = FALSE;
    
    // Get combat (tactics) up to a minimum
    if(combat->getLearnedness() < 15) {
      raiseDiscOnce(DISC_COMBAT);
      continue;
    }

    // start raising combat and mage until they are both maxed
    bool combatDone=(combat->getLearnedness() >= MAX_DISC_LEARNEDNESS);
    bool mageDone=(mage->getLearnedness() >= MAX_DISC_LEARNEDNESS);
    
    if(combatDone && !mageDone){
      raiseDiscOnce(DISC_MAGE);
      continue;
    } else if(!combatDone && mageDone){
      raiseDiscOnce(DISC_COMBAT);
      continue;
    } else if(!combatDone && !mageDone){
      if(::number(0,1))
	raiseDiscOnce(DISC_MAGE);
      else
	raiseDiscOnce(DISC_COMBAT);
      continue;
    }
    // fall through only if combat and mage are both maxed


    // first, let's choose a favored disc, this is our "specialization"
    if(!favored){
      switch(::number(1,5)){
	case 1:
	  favoredNum=DISC_SORCERY;
	  break;
	case 2:
	  favoredNum=DISC_FIRE;
	  break;
	case 3:
	  favoredNum=DISC_WATER;
	  break;
	case 4:
	  favoredNum=DISC_EARTH;
	  break;
	case 5:
	  favoredNum=DISC_AIR;
	  break;
      }
      
      if(!(favored = getDiscipline(favoredNum))){
	vlogf(LOG_BUG, "%s didn't have discipline %i.", getName(), favoredNum);
	return;
      }
    }

    // learn our favored disc up a bit
    if(favored->getLearnedness() < 75){
      raiseDiscOnce(favoredNum);
      continue;
    }


    // now just raise stuff randomly
    // if something is maxed, we just fall through to the next thing
    switch(::number(0,10)){
      case 0:
	if((cd = getDiscipline(DISC_PIERCE)) &&
	   cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	  raiseDiscOnce(DISC_PIERCE);
	  continue;
	}
      case 1:
	if((cd = getDiscipline(DISC_SPIRIT)) &&
	   cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	  raiseDiscOnce(DISC_SPIRIT);
	  continue;
	}
      case 2:
	if((cd = getDiscipline(DISC_ALCHEMY)) &&
	   cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	  raiseDiscOnce(DISC_ALCHEMY);
	  continue;
	}
      case 3:
	if((cd = getDiscipline(DISC_SORCERY)) &&
	   cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	  raiseDiscOnce(DISC_SORCERY);
	  continue;
	}
      case 4:
	if((cd = getDiscipline(DISC_EARTH)) &&
	   cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	  raiseDiscOnce(DISC_EARTH);
	  continue;
	}
      case 5:
	if((cd = getDiscipline(DISC_FIRE)) &&
	   cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	  raiseDiscOnce(DISC_FIRE);
	  continue;
	}
      case 6:
	if((cd = getDiscipline(DISC_WATER)) &&
	   cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	  raiseDiscOnce(DISC_WATER);
	  continue;
	}
      case 7:
	if((cd = getDiscipline(DISC_AIR)) &&
	   cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	  raiseDiscOnce(DISC_AIR);
	  continue;
	}
      case 8:
	if((cd = getDiscipline(DISC_SPIRIT)) &&
	   cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	  raiseDiscOnce(DISC_SPIRIT);
	  continue;
	}
      case 9:
	if((cd = getDiscipline(DISC_PIERCE)) &&
	   cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	  raiseDiscOnce(DISC_PIERCE);
	  continue;
	}
      case 10:
	if((cd = getDiscipline(DISC_ALCHEMY)) &&
	   cd->getLearnedness() < MAX_DISC_LEARNEDNESS){
	  raiseDiscOnce(DISC_ALCHEMY);
	  continue;
	}
    }

    // maxed all disciplines
    break;
  }
}




