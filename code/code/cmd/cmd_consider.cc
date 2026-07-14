//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "cmd_consider.cc" - The consider command
//
//////////////////////////////////////////////////////////////////////////

#include "handler.h"
#include "extern.h"
#include "being.h"
#include "cmd_trophy.h"
#include "person.h"
#include "monster.h"
#include "obj_base_weapon.h"

namespace {
  // Map a per-swing melee hit percent to a prose description. `incoming`
  // selects the defender's perspective (the mob's blows landing on you) versus
  // the attacker's (your blows landing on the mob). Both ladders mirror the
  // full range so a comfortable fight reassures just as a losing one warns.
  const char* hitChanceDescr(int pct, bool incoming) {
    if (pct >= 90)
      return incoming ? "Their blows will almost always land."
                      : "Your blows will rarely miss.";
    if (pct >= 80)
      return incoming ? "Their attacks will strike you easily."
                      : "Your attacks will connect easily.";
    if (pct >= 70)
      return incoming ? "They are very likely to hit you."
                      : "You are very likely to hit them.";
    if (pct >= 60)
      return incoming ? "They are likely to hit you."
                      : "You are likely to hit them.";
    if (pct >= 50)
      return incoming ? "Their attacks will hit you more often than not."
                      : "You will hit more often than not.";
    if (pct >= 40)
      return incoming ? "Their attacks will miss you more often than not."
                      : "You will miss more often than not.";
    if (pct >= 30)
      return incoming ? "They are likely to miss you."
                      : "You are likely to miss them.";
    if (pct >= 20)
      return incoming ? "They are very likely to miss you."
                      : "You are very likely to miss them.";
    if (pct >= 10)
      return incoming ? "Their attacks will rarely find you."
                      : "Your attacks will rarely connect.";
    return incoming ? "Their blows will almost never find you."
                    : "Your blows will almost never land.";
  }

  // The looker's lore value for this mob: the highest learnedness among the
  // creature-knowledge skills that apply to its type. When `announce`, emit the
  // flavorful "you determine that X is a Y" line (suppressed for the focused
  // skill readout). sknum/roll report which skill drove the value so the caller
  // can grant a learn-by-doing tick.
  int considerLore(TBeing* ch, TMonster* tmon, const char* namebuf,
    bool announce, spellNumT& sknum, int& roll) {
    int learn = 0;
    sknum = TYPE_UNDEFINED;
    roll = 0;

    auto note = [&](spellNumT skill, int rollVal, const char* lore) {
      sknum = skill;
      roll = rollVal;
      learn = max(learn, static_cast<int>(ch->getSkillValue(skill)));
      if (announce)
        ch->sendTo(COLOR_MOBS,
          format(
            "Using your knowledge %s, you determine that %s is %s %s.\n\r") %
            lore % namebuf %
            (tmon->getMyRace()->getSingularName().startsVowel() ? "an" : "a") %
            tmon->getMyRace()->getSingularName());
    };

    if (tmon->isAnimal() && ch->doesKnowSkill(SKILL_CONS_ANIMAL))
      note(SKILL_CONS_ANIMAL, 1, "of animal lore");
    if (tmon->isVeggie() && ch->doesKnowSkill(SKILL_CONS_VEGGIE))
      note(SKILL_CONS_VEGGIE, 1, "of vegetable lore");
    if (tmon->isDiabolic() && ch->doesKnowSkill(SKILL_CONS_DEMON))
      note(SKILL_CONS_DEMON, 1, "of demon lore");
    if (tmon->isReptile() && ch->doesKnowSkill(SKILL_CONS_REPTILE))
      note(SKILL_CONS_REPTILE, 2, "of reptile lore");
    if (tmon->isUndead() && ch->doesKnowSkill(SKILL_CONS_UNDEAD))
      note(SKILL_CONS_UNDEAD, 2, "of the undead");
    if (tmon->isGiantish() && ch->doesKnowSkill(SKILL_CONS_GIANT))
      note(SKILL_CONS_GIANT, 1, "of giant lore");
    if (tmon->isPeople() && ch->doesKnowSkill(SKILL_CONS_PEOPLE))
      note(SKILL_CONS_PEOPLE, 2, "of human and demi-human lore");
    if (tmon->isOther() && ch->doesKnowSkill(SKILL_CONS_OTHER))
      note(SKILL_CONS_OTHER, 1, "of monster lore");

    return min(learn, static_cast<int>(MAX_SKILL_LEARNEDNESS));
  }

  // Whether `consider`'s hit-chance line is meaningful for this skill: true
  // only for skills that resolve through TBeing::specialAttack. For backstab
  // and throatslit the estimate reflects the thief's current stealth state and
  // the non-wary opening strike. Keep in sync with the specialAttack call
  // sites.
  bool usesSpecialAttack(spellNumT skill) {
    switch (skill) {
      case SKILL_BASH:
      case SKILL_BODYSLAM:
      case SKILL_SLAM:
      case SKILL_KICK:
      case SKILL_HEADBUTT:
      case SKILL_KNEESTRIKE:
      case SKILL_STOMP:
      case SKILL_SPIN:
      case SKILL_CHOP:
      case SKILL_WHIRLWIND:
      case SKILL_TRIP:
      case SKILL_DISARM:
      case SKILL_GRAPPLE:
      case SKILL_STABBING:
      case SKILL_BONEBREAK:
      case SKILL_QUIV_PALM:
      case SKILL_CHARGE:
      case SKILL_SHOCK_CAVALRY:
      case SKILL_DEATHSTROKE:
      case SKILL_SUBTERFUGE:
      case SKILL_BACKSTAB:
      case SKILL_THROATSLIT:
      case SKILL_KINETIC_WAVE:
      case SKILL_HURL:
      case SKILL_SHOULDER_THROW:
      case SKILL_DEFENESTRATE:
        return true;
      default:
        return false;
    }
  }
}  // namespace

void TBeing::doConsider(const char* argument) {
  TBeing* victim;
  char namebuf[256];
  char skillbuf[256];
  int diff = 0;
  spellNumT considerSkill = TYPE_UNDEFINED;

  // "consider <target> <skill>" -- the optional trailing skill name asks for
  // that special attack's damage estimate alongside the usual assessment.
  half_chop_safe(argument, namebuf, sizeof(namebuf), skillbuf,
    sizeof(skillbuf));

  if (is_abbrev(namebuf, "self"))
    strcpy(namebuf, getNameNOC(this).c_str());

  if (!(victim = get_char_room_vis(this, namebuf))) {
    if (!isImmortal() || !hasWizPower(POWER_IMM_EVAL)) {
      sendTo("Consider killing whom?\n\r");
      return;
    } else if (!(victim = get_char_vis_world(this, namebuf, NULL, EXACT_YES)) &&
               !(victim = get_char_vis_world(this, namebuf, NULL, EXACT_NO))) {
      sendTo("I'm afraid I was unable to find them.\n\r");
      return;
    } else if (!dynamic_cast<TPerson*>(victim)) {
      sendTo(
        "I'm afraid you can only use this on mortals in this fashion.\n\r");
      return;
    }
  }
  if (!canSee(victim) && canSee(victim, INFRA_YES)) {
    strcpy(namebuf, "a blob");
  } else {
    strcpy(namebuf, victim->getName().c_str());
  }
  if (victim == this) {
    sendTo("You consider yourself...\n\r");
    int armor = 1000 - getArmor();
    int suggest = suggestArmor();
    diff = suggest - armor;
    sendTo(
      format("Your equipment would seem %s for your class and level.\n\r") %
      (diff >= 210
          ? "laughably pathetic"
          : (diff >= 160
                ? "horrid"
                : (diff >= 90
                      ? "bad"
                      : (diff >= 50
                            ? "poor"
                            : (diff >= 30
                                  ? "weak"
                                  : (diff >= 10
                                        ? "o.k."
                                        : (diff > 0
                                              ? "near perfect"
                                              : (diff == 0
                                                    ? "perfect"
                                                    : (diff >= -30
                                                          ? "good"
                                                          : (diff >= -40
                                                                ? "very good"
                                                                : (diff >= -80
                                                                      ? "great"
                                                                      : (diff >=
                                                                              -150
                                                                            ? "fantastic"
                                                                            : (diff >=
                                                                                    -200
                                                                                  ? "superb"
                                                                                  : "incredibly good"))))))))))))));

    int vis = visibility();
    sendTo(
      format("You are %s.\n\r") %
      (vis >= 50
          ? "only a shadow to the eyes of most mortals"
          : (vis >= 40
                ? "barely visible at all"
                : (vis >= 30
                      ? "very well hidden"
                      : (vis >= 20
                            ? "well hidden"
                            : (vis >= 10
                                  ? "fairly well hidden"
                                  : (vis >= 5
                                        ? "slightly hidden"
                                        : (vis >= 0 ? "not especially hidden "
                                                      "or visible"
                                                    : "as visible as a "
                                                      "christmas tree"))))))));

    int n = noise(this);
    sendTo(
      format("You are %s.\n\r") %
      (n >= 500
          ? "too noisy to sneak up on a deaf man"
          : (n >= 300
                ? "making more noise than a herd of stampeding elephants"
                : (n >= 200
                      ? "making a lot of noise"
                      : (n >= 100
                            ? "making some noise"
                            : (n >= 50
                                  ? "making a little noise"
                                  : (n >= 25
                                        ? "somewhat quiet"
                                        : (n >= 0
                                              ? "quiet"
                                              : (n >= -25
                                                    ? "very quiet"
                                                    : "walking in silence, "
                                                      "like a shadow")))))))));
    return;
  }
  if (victim->isPc() && victim->isImmortal()) {
    sendTo("You must sure have a big ego to contemplate fighting gods.\n\r");
    act("$N just considered fighting you.", TRUE, victim, 0, this, TO_CHAR);
    return;
  } else if (dynamic_cast<TPerson*>(victim) ||
             isname("[clone]", victim->name)) {
    if (isImmortal() && hasWizPower(POWER_IMM_EVAL)) {
      diff = (int)(victim->getArmor());
      short suggest = victim->suggestArmor();
      int prefArmorC = (1000 - suggest);

      sendTo(format("You consider %s's equipment...\n\r") % victim->getName());
      sendTo(format("%s should have an AC of [%d] but has an AC of [%d]\n\r") %
             sstring(victim->getName()).cap() % prefArmorC % diff);
      return;
    } else {
      sendTo("Would you like to borrow a cross and a shovel?\n\r");
      playsound(SOUND_DONT_KILL_ME, SOUND_TYPE_COMBAT);
      return;
    }
  }

  if (skillbuf[0]) {
    // Take the first (base) match rather than demanding a unique one: a bare
    // "bash" also name-matches the class variant "bash-deikhan", and a unique
    // search would reject it as ambiguous. Base skills precede their variants
    // in the enum, so the first exact hit is the generic skill -- which
    // getSkillNum then routes to the version this class actually trains, just
    // as the live `bash` command does. Prefer an exact hit before an
    // abbreviation so a short name isn't shadowed by a longer one.
    considerSkill = searchForSpellNum(skillbuf, EXACT_YES, false);
    if (considerSkill == TYPE_UNDEFINED)
      considerSkill = searchForSpellNum(skillbuf, EXACT_NO, false);
    if (considerSkill == TYPE_UNDEFINED ||
        !doesKnowSkill(getSkillNum(considerSkill))) {
      sendTo(format("You don't know a skill called '%s'.\n\r") % skillbuf);
      considerSkill = TYPE_UNDEFINED;
    }
  }

  // everything should be a monster by this point
  TMonster* tmon = dynamic_cast<TMonster*>(victim);

  act("$n looks $N over.", TRUE, this, 0, tmon, TO_NOTVICT);
  act("$n looks you over.", TRUE, this, 0, tmon, TO_VICT);

  // "consider <target> <skill>": a focused readout for one attack -- its
  // execution odds and damage against this target -- and nothing else.
  if (considerSkill != TYPE_UNDEFINED) {
    if (!getDiscipline(DISC_ADVENTURING)) {
      sendTo("You lack the training to size up your attacks that way.\n\r");
      return;
    }
    spellNumT sknum;
    int roll;
    int learn = considerLore(this, tmon, namebuf, false, sknum, roll);
    if (learn < 40) {
      sendTo(
        format("You don't know enough about %s to gauge your attacks.\n\r") %
        namebuf);
      return;
    }
    addToWait(combatRound(1));

    // Display the skill under the name the player typed (the base skill), but
    // drive the odds and damage from the class-specific version they actually
    // train -- its learnedness, not the base skill's, is what governs a real
    // execution. For skills without a class variant these are the same number.
    sstring skillName = sstring(discArray[considerSkill]->name).cap();
    spellNumT trainedSkill = getSkillNum(considerSkill);
    sendTo(format("You have a %d%% chance of successfully executing a %s "
                  "attack.\n\r") %
           skillExecuteChance(trainedSkill) % skillName);

    // Landing chance, once executed -- only meaningful for specialAttack
    // skills.
    if (usesSpecialAttack(considerSkill))
      sendTo(
        format("Once executed, it will connect about %d%% of the time.\n\r") %
        specialAttackChance(tmon, trainedSkill));

    // Some specialAttack skills (trip, disarm, grapple) resolve as pure
    // control moves and deal no direct damage -- say so plainly rather than
    // implying the attack couldn't be evaluated (its odds just were).
    auto [kmin, kmax] = skillDamageRange(tmon, trainedSkill);
    if (kmax > 0)
      sendTo(format("Your %s attack will do %d-%d damage against %s.\n\r") %
             skillName % kmin % kmax % namebuf);
    else
      sendTo(format("A %s deals no direct damage.\n\r") % skillName);
    return;
  }

#if 0
  diff = tmon->GetMaxLevel() - GetMaxLevel();
#else
  // let's use the present real lev so we look at spells and stuff
  diff = (int)(tmon->getRealLevel() + 0.5) - GetMaxLevel();
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

  auto count = trophy->getCount(tmon->mobVnum());
  if (count > 0) {
    sendTo(COLOR_BASIC,
      format("You will gain %s experience when fighting %s.\n\r") %
        trophy->getExpModDescr(count, tmon->mobVnum()) % namebuf);
  } else {
    sendTo(COLOR_BASIC,
      format("You have never fought %s and will gain %s experience.\n\r") %
        namebuf % trophy->getExpModDescr(count, tmon->mobVnum()));
  }

  if (getDiscipline(DISC_ADVENTURING)) {
    spellNumT sknum = TYPE_UNDEFINED;
    int roll = 0;
    int learn = considerLore(this, tmon, namebuf, true, sknum, roll);

    if (!learn)
      return;

    if ((GetMaxLevel() <= MAX_MORT) && !::number(0, roll)) {
      learnFromDoing(sknum, SILENT_NO, 0);
    }

    addToWait(combatRound(1));

    // Figures shared by several readouts. Hit chance is a single
    // character-vs-target value (not per hand), so both hands share it; damage
    // genuinely differs by hand, computed below.
    double itsMult = tmon->getMult();
    int myHit = meleeHitChance(tmon);
    int itsHit = tmon->meleeHitChance(this);

    // Its number of attacks: prose while your lore is rough, a count at 40.
    if (learn >= 40)
      sendTo(
        format("Est. # of attacks: about %d.\n\r") %
        max(1, GetApprox(static_cast<int>(itsMult + 0.5), max(80, learn))));
    else
      sendTo(format("Est. # of attacks: %s.\n\r") %
             DescAttacks(GetApprox(static_cast<int>(itsMult), max(80, learn))));

    // Hit chance, both directions: prose at 40, exact percentages at 50.
    if (learn >= 40) {
      if (learn >= 50) {
        sendTo(format("You will hit %s about %d%% of the time.\n\r") % namebuf %
               myHit);
        sendTo(format("%s will hit you about %d%% of the time.\n\r") %
               sstring(namebuf).cap() % itsHit);
      } else {
        sendTo(format("%s\n\r") % hitChanceDescr(myHit, false));
        sendTo(format("%s\n\r") % hitChanceDescr(itsHit, true));
      }
    }

    // Damage -- your output per hand and its output against you. Prose at 50,
    // exact figures at 70.
    if (learn >= 50) {
      TThing* primWeap = heldInPrimHand();
      TObj* primObj = dynamic_cast<TObj*>(primWeap);
      bool paired = primObj && primObj->isPaired();
      auto [pmin, pmax] = meleeDamageRange(tmon, primWeap, HAND_PRIMARY);
      TThing* secWeap = paired ? nullptr : heldInSecHand();
      auto [smin, smax] = paired
                            ? std::pair<int, int>{0, 0}
                            : meleeDamageRange(tmon, secWeap, HAND_SECONDARY);

      // Its damage against you: a wielded weapon uses the real weapon pipeline
      // (resistance-aware); a natural attacker uses its base damage, since the
      // barehand path models a humanoid punch, not a beast's bite.
      TThing* mobWeap = tmon->heldInPrimHand();
      bool mobArmed = dynamic_cast<TBaseWeapon*>(mobWeap);
      int mmin, mmax;
      if (mobArmed) {
        std::tie(mmin, mmax) =
          tmon->meleeDamageRange(this, mobWeap, HAND_PRIMARY);
      } else {
        int base = max(1, static_cast<int>(tmon->baseDamage()));
        mmin = max(1, base * 3 / 4);
        mmax = max(mmin, base * 5 / 4);
      }
      int mobAvg =
        mobArmed ? (mmin + mmax + 1) / 2 : static_cast<int>(tmon->baseDamage());

      if (learn >= 70) {
        if (pmax > 0) {
          if (paired)
            sendTo(
              format("A hit with %s will do %d-%d damage when held in both "
                     "hands.\n\r") %
              primWeap->getName() % pmin % pmax);
          else if (primWeap)
            sendTo(format("A hit with %s will do %d-%d damage in your primary "
                          "hand.\n\r") %
                   primWeap->getName() % pmin % pmax);
          else
            sendTo(
              format("You will do %d-%d damage with your primary hand.\n\r") %
              pmin % pmax);
        }
        if (!paired && smax > 0) {
          if (secWeap)
            sendTo(
              format("A hit with %s will do %d-%d damage in your secondary "
                     "hand.\n\r") %
              secWeap->getName() % smin % smax);
          else
            sendTo(
              format("You will do %d-%d damage with your secondary hand.\n\r") %
              smin % smax);
        }
        if (mobArmed)
          sendTo(
            format("%s will do an average of %d damage to you with %s.\n\r") %
            sstring(namebuf).cap() % max(1, mobAvg) % mobWeap->getName());
        else
          sendTo(format("%s will hit you for an average of %d damage per "
                        "hit.\n\r") %
                 sstring(namebuf).cap() % max(1, mobAvg));
      } else {
        if (pmax > 0)
          sendTo(format("Your attacks would do: %s.\n\r") %
                 DescDamage((pmin + pmax) / 2.0));
        sendTo(format("Its attacks would do: %s.\n\r") %
               DescDamage((mmin + mmax) / 2.0));
      }
    }

    // Critical-hit chance: the mastery reveal, numbers only.
    if (learn >= 80) {
      double crit = critChancePercent(tmon, heldInPrimHand());
      if (crit > 0.0)
        sendTo(format("You have a %.1f%% chance to score a critical hit.\n\r") %
               crit);
    }
  }
  immortalEvaluation(tmon);
}
