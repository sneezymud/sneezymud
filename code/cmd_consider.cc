//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "cmd_consider.cc" - The consider command
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

void TBeing::doConsider(const char *argument)
{
  TBeing *victim;
  char namebuf[256];
  int diff=0;

  only_argument(argument, namebuf);

  if (!(victim = get_char_room_vis(this, namebuf))) {
    if (!isImmortal() || !hasWizPower(POWER_IMM_EVAL)) {
      sendTo("Consider killing whom?\n\r");
      return;
    } else if (!(victim = get_char_vis_world(this, namebuf, NULL, EXACT_YES)) &&
               !(victim = get_char_vis_world(this, namebuf, NULL, EXACT_NO))) {
      sendTo("I'm afraid I was unable to find them.\n\r");
      return;
    } else if (!dynamic_cast<TPerson *>(victim)) {
      sendTo("I'm afraid you can only use this on mortals in this fashion.\n\r");
      return;
    }
  }
  if (!canSee(victim) && canSee(victim, INFRA_YES)) {
    strcpy(namebuf, "a blob");
  } else {
    strcpy(namebuf, victim->getName());
  }
  if (victim == this) {
    if (!isImmortal()) {
      sendTo("You consider your equipment...\n\r");
      int armor = 1000 - getArmor();
      sh_int suggest = suggestArmor();
      diff = (int) (suggest - armor);
      sendTo("Your equipment would seem %s for your class and level.\n\r",
             (diff >=  210 ? "laughably pathetic" :
             (diff >=  160 ? "horrid" :
             (diff >=   90 ? "bad" :
             (diff >=   50 ? "poor" :
             (diff >=   30 ? "weak" :
             (diff >=   10 ? "o.k." :
             (diff >     0 ? "near perfect" :
             (diff ==    0 ? "perfect" :
             (diff >= - 30 ? "good" :
             (diff >= - 40 ? "very good" :
             (diff >= - 80 ? "great" :
             (diff >= -150 ? "fantastic" :
             (diff >= -200 ? "superb" : "incredibly good"))))))))))))));
      return;
    } else {
      sendTo("You're funny...  You're a god, what do you need armor for??\n\r");
      return;
    }
  }
  if (victim->isPc() && victim->isImmortal()) {
    sendTo("You must sure have a big ego to contemplate fighting gods.\n\r");
    act("$N just considered fighting you.",TRUE,victim,0,this,TO_CHAR);
    return;
  } else if (dynamic_cast<TPerson *>(victim)) {
    if (isImmortal() && hasWizPower(POWER_IMM_EVAL)) {
      diff       = (int) (victim->getArmor());
      sh_int suggest = victim->suggestArmor();
      int prefArmorC = (1000 - suggest);

      sendTo("You consider %s's equipment...\n\r", victim->getName());
      sendTo("%s should have an AC of [%d] but has an AC of [%d]\n\r",
             good_cap(victim->getName()).c_str(),
             prefArmorC,
             diff);
      return;
    } else {
      sendTo("Would you like to borrow a cross and a shovel?\n\r");
      playsound(SOUND_DONT_KILL_ME, SOUND_TYPE_COMBAT);
      return;
    }
  }

  // everything should be a monster by this point
  TMonster *tmon = dynamic_cast<TMonster *>(victim);

  act("$n looks $N over.", TRUE, this, 0, tmon, TO_NOTVICT);
  act("$n looks you over.", TRUE, this, 0, tmon, TO_VICT);

#if 0
  diff = tmon->GetMaxLevel() - GetMaxLevel();
#else
  // let's use the present real lev so we look at spells and stuff
  diff = (int) (tmon->getRealLevel() + 0.5) - GetMaxLevel();
#endif
  if (diff <= -15)
    sendTo("Shall I tie both hands behind your back, or just one?\n\r");
  else if (diff <= -10)
    sendTo("Why bother???\n\r");
  else if (diff <= -6)
    sendTo("Don't strain yourself.\n\r");
  else if (diff <= -3)
    sendTo("Piece of cake.\n\r");
  else if (diff <= -2)
    sendTo("Odds are in your favor.\n\r");
  else if (diff <= -1)
    sendTo("You have a slight advantage.\n\r");
  else if (!diff)
    sendTo("A fair fight.\n\r");
  else if (diff <= 1)
    act("$E doesn't look that tough...", TRUE, this, 0, tmon, TO_CHAR);
  else if (diff <= 2)
    sendTo("Cross your fingers.\n\r");
  else if (diff <= 3)
    sendTo("Cross your fingers and hope they don't get broken.\n\r");
  else if (diff <= 6)
    sendTo("I hope you have a good plan!\n\r");
  else if (diff <= 10)
    sendTo("Bring friends.\n\r");
  else if (diff <= 15)
    sendTo("You and what army??\n\r");
  else if (diff <= 30)
    act("You'll win if $E never hits you.", TRUE, this, 0, tmon, TO_CHAR);
  else
    sendTo("There are better ways to suicide.\n\r");

  if (getDiscipline(DISC_ADVENTURING)) {
    int learn = 0;
    spellNumT sknum = TYPE_UNDEFINED;
    int roll = 0;

    if (tmon->isAnimal() && doesKnowSkill(SKILL_CONS_ANIMAL)) {
      sknum = SKILL_CONS_ANIMAL;
      roll = 1;
      learn = getSkillValue(SKILL_CONS_ANIMAL);
      sendTo(COLOR_MOBS, "Using your knowledge of animal lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isVeggie() && doesKnowSkill(SKILL_CONS_VEGGIE)) {
      sknum = SKILL_CONS_VEGGIE;
      roll = 1;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_VEGGIE));
      sendTo(COLOR_MOBS, "Using your knowledge of vegetable lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isDiabolic() && doesKnowSkill(SKILL_CONS_DEMON)) {
      sknum = SKILL_CONS_DEMON;
      roll = 1;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_DEMON));
      sendTo(COLOR_MOBS, "Using your knowledge of demon lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isReptile() && doesKnowSkill(SKILL_CONS_REPTILE)) {
      sknum = SKILL_CONS_REPTILE;
      roll = 2;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_REPTILE));
      sendTo(COLOR_MOBS, "Using your knowledge of reptile lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isUndead() && doesKnowSkill(SKILL_CONS_UNDEAD)) {
      sknum = SKILL_CONS_UNDEAD;
      roll = 2;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_UNDEAD));
      sendTo(COLOR_MOBS, "Using your knowledge of the undead, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isGiantish() && doesKnowSkill(SKILL_CONS_GIANT)) {
      sknum = SKILL_CONS_GIANT;
      roll = 1;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_GIANT));
      sendTo(COLOR_MOBS, "Using your knowledge of giant lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isPeople() && doesKnowSkill(SKILL_CONS_PEOPLE)) {
      sknum = SKILL_CONS_PEOPLE;
      roll = 2;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_PEOPLE));
      sendTo(COLOR_MOBS, "Using your knowledge of human and demi-human lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (tmon->isOther() && doesKnowSkill(SKILL_CONS_OTHER)) {
      sknum = SKILL_CONS_OTHER;
      roll = 1;
      learn = max(learn, (int) getSkillValue(SKILL_CONS_OTHER));
      sendTo(COLOR_MOBS, "Using your knowledge of monster lore, you determine that %s is %s %s.\n\r",
            namebuf,
            startsVowel(tmon->getMyRace()->getSingularName().c_str()) ? "an" : "a",
            tmon->getMyRace()->getSingularName().c_str());
    }
    if (learn > MAX_SKILL_LEARNEDNESS)
      learn = MAX_SKILL_LEARNEDNESS;

    if (!learn)
      return;

    if ((GetMaxLevel() <= MAX_MORT) && !::number(0,roll)) {
      learnFromDoing(sknum, SILENT_NO, 0);
    }

    addToWait(combatRound(1));

    int num = GetApprox(tmon->hitLimit(), max(80,learn));
    double fnum = hitLimit() ? ((double) num / (double) hitLimit()) : 
            (num ? 10.0 : 0.0);
    if (learn > 5)
      sendTo("Est Max HP are: %s.\n\r", DescRatio(fnum));

    // mob's AC : expressed as a level (use mob scale for AC-lev)
    num = GetApprox(max(((1000 - tmon->getArmor()) - 400)/20, 0), 10*max(80,learn));
    // my AC : expressed as a level  (use PC scale for AC-lev)
    int num2 = (int) max(((1000 - getArmor()) - 500)/25, 0);
    // take the difference
    num -= num2;  // if positive, mob has better armor
    // normalize it
    fnum = ((double) num/get_doubling_level(GetMaxLevel()));
    // if same ACs, num = 0 and want it to be 1.0
    // if mob had AC of twice as good mob, num = 1, want it to be 2.0
    // if mob had AC of 4X as good mob, num = 2, want it to be 4.0
    // if mob had AC of twice as bad mob, num = -1, want it to be 0.5
    fnum = pow(2.0, fnum);

    if (learn > 20)
      sendTo("Est. armor class is : %s.\n\r", DescRatio(fnum));

    if (learn > 40)
      sendTo("Est. # of attacks: %s.\n\r", DescAttacks(GetApprox((int) tmon->getMult(), max(80,learn))));

    if (learn > 60) {
      num = GetApprox((int) tmon->baseDamage(), max(80,learn));
      fnum = (double) num;
      sendTo("Est. damage of attacks: %s.\n\r", DescDamage(fnum));
    }
  }
  immortalEvaluation(tmon);
}
