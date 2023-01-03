//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "handler.h"
#include "extern.h"
#include "obj_base_weapon.h"
#include "room.h"
#include "being.h"
#include "disc_brawling.h"
#include "obj_base_clothing.h"

CDBrawling::CDBrawling() :
  CDiscipline(),
  skGrapple(),
  skStomp(),
  skBrawlAvoidance(),
  skBodyslam(),
  skHeadbutt(),
  skKneestrike(),
  skSpin(),
  skCloseQuartersFighting(),
  skTaunt(),
  skTrip(),
  skAdvBerserk()
{
}

CDBrawling::CDBrawling(const CDBrawling &a) :
  CDiscipline(a),
  skGrapple(a.skGrapple),
  skStomp(a.skStomp),
  skBrawlAvoidance(a.skBrawlAvoidance),
  skBodyslam(a.skBodyslam),
  skHeadbutt(a.skHeadbutt),
  skKneestrike(a.skKneestrike),
  skSpin(a.skSpin),
  skCloseQuartersFighting(a.skCloseQuartersFighting),
  skTaunt(a.skTaunt),
  skTrip(a.skTrip),
  skAdvBerserk(a.skAdvBerserk)
{
}

CDBrawling & CDBrawling::operator=(const CDBrawling &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skGrapple = a.skGrapple;
  skStomp = a.skStomp;
  skBrawlAvoidance = a.skBrawlAvoidance;
  skBodyslam = a.skBodyslam;
  skHeadbutt = a.skHeadbutt;
  skKneestrike = a.skKneestrike;
  skSpin = a.skSpin;
  skCloseQuartersFighting = a.skCloseQuartersFighting;
  skTaunt = a.skTaunt;
  skTrip = a.skTrip;
  skAdvBerserk = a.skAdvBerserk;
  return *this;
}

CDBrawling::~CDBrawling()
{
}


int TBeing::doTaunt(const sstring &arg)
{
  TBeing *victim;
  char name_buf[256];
  
  strcpy(name_buf, arg.c_str());
  
  if (!(victim = get_char_room_vis(this, name_buf))) {
    if (!(victim = fight())) {
      sendTo("Taunt whom?\n\r");
      return FALSE;
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }

  if(!doesKnowSkill(SKILL_TAUNT) ||

     (roomp && roomp->isRoomFlag(ROOM_PEACEFUL)) ||
     victim==this || noHarmCheck(victim) || checkBusy() ||
     victim->isDumbAnimal() || !canSpeak() || victim != fight()){
    return doAction(arg, CMD_TAUNT);
  }


  if(bSuccess(SKILL_TAUNT)){
    act("You taunt $N ruthlessly, drawing their ire.",
	FALSE, this, 0, victim, TO_CHAR);
    act("$n taunts you ruthlessly, drawing your ire.",
	FALSE, this, 0, victim, TO_VICT);
    act("$n taunts $N ruthlessly, drawing their ire.",
	FALSE, this, 0, victim, TO_NOTVICT);
    
    affectedData af;

    af.type = SKILL_TAUNT;
    af.level = getSkillValue(SKILL_TAUNT);
    af.duration = durationModify(SKILL_TAUNT, Pulse::COMBAT * ::number(1, (int)(af.level/10)));
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = 0;
    victim->affectFrom(SKILL_TAUNT);
    victim->affectTo(&af, -1);

  } else {
    act("You taunt yourself ruthlessly, confusing yourself.",
	FALSE, this, 0, this, TO_CHAR);
    act("$n taunts $mself ruthlessly, confusing $mself.",
	FALSE, this, 0, this, TO_NOTVICT);
  }


  return TRUE;
}

void TBeing::doAdvancedBerserk(TBeing* target) 
{
  // Chance to gain a bloodlust stack
  if (doesKnowSkill(SKILL_BLOODLUST) && !::number(0,5)) 
    doBloodlust();

  // Chance to land a random warrior ability automatically while berserking.
  // The abilities currently planned are:
  //   - slam (1-8)
  //   - headbutt (9,10)
  //   - kneestrike (11,12)
  //   - bash (13-18)
  //   - bodyslam (19-20)
  //   - spin (21-22)
  //   - stomp (23-24) (6)
  //   - focus attack (25-28)
  //   - whirlwind (29)
  //   - deathstroke (30)
  if (bSuccess(getSkillLevel(SKILL_ADVANCED_BERSERKING), SKILL_ADVANCED_BERSERKING) && !::number(0,10)) {
	  int roll = ::number(1,30);
	  if (roll >= 1 && roll <= 8 && doesKnowSkill(SKILL_SLAM) && bSuccess(SKILL_SLAM)) {
	    act("In a <R>berserker rage<z>, you swing wildly at $N!", FALSE, this, 0, target, TO_CHAR);     
	    act("In a <R>berserker rage<z>, $n swings wildly at $N!", FALSE, this, 0, target, TO_NOTVICT);     
	    act("In a <R>berserker rage<z>, $n swings wildly at you!", FALSE, this, 0, target, TO_VICT);     
      slamSuccess(target);
    }
	  else if (roll >= 9 && roll <= 10 && doesKnowSkill(SKILL_HEADBUTT) && bSuccess(SKILL_HEADBUTT)) {
	    act("You're overcome by your <R>berserker rage<z>!", FALSE, this, 0, target, TO_CHAR);     
	    act("$n is overcome by <R>berserker rage<z>!", FALSE, this, 0, target, TO_NOTVICT);     
	    act("$n is overcome by <R>berserker rage<z>!", FALSE, this, 0, target, TO_VICT);     
      headbuttHit(target);
    }
	  else if (roll >= 11 && roll <= 12 && doesKnowSkill(SKILL_KNEESTRIKE) && bSuccess(SKILL_KNEESTRIKE)) {
	    act("You're overcome by your <R>berserker rage<z>!", FALSE, this, 0, target, TO_CHAR);     
	    act("$n is overcome by <R>berserker rage<z>!", FALSE, this, 0, target, TO_NOTVICT);     
	    act("$n is overcome by <R>berserker rage<z>!", FALSE, this, 0, target, TO_VICT);     
      kneestrikeHit(target);
    }
	  else if (roll >= 13 && roll <= 18 && doesKnowSkill(SKILL_BASH) && bSuccess(SKILL_BASH)) {
	    act("In a <R>berserker rage<z>, you crash into $N with all your might!", FALSE, this, 0, target, TO_CHAR);     
	    act("In a <R>berserker rage<z>, $n crashes into $N!", FALSE, this, 0, target, TO_NOTVICT);     
	    act("In a <R>berserker rage<z>, $n crashes into you!", FALSE, this, 0, target, TO_VICT);     
      TBaseClothing *itemInSecondaryHand = dynamic_cast<TBaseClothing *>(heldInSecHand());  
      bool isHoldingShield = itemInSecondaryHand && itemInSecondaryHand->isShield();
      bashSuccess(target, SKILL_BASH, isHoldingShield, itemInSecondaryHand);
	  }
	  else if (roll >= 19 && roll <= 20 && doesKnowSkill(SKILL_BODYSLAM) && bSuccess(SKILL_BODYSLAM)) {
	    act("You're overcome by your <R>berserker rage<z>!", FALSE, this, 0, target, TO_CHAR);     
	    act("$n is overcome by <R>berserker rage<z>!", FALSE, this, 0, target, TO_NOTVICT);     
	    act("$n is overcome by <R>berserker rage<z>!", FALSE, this, 0, target, TO_VICT);     
      bodyslamHit(target); 
	  }
	  else if (roll >= 21 && roll <= 22 && doesKnowSkill(SKILL_SPIN) && bSuccess(SKILL_SPIN)) {
	    act("In a <R>berserker rage<z>, you attempt to throw $N!", FALSE, this, 0, target, TO_CHAR);     
	    act("In a <R>berserker rage<z>, $n attempts to throw $N!", FALSE, this, 0, target, TO_NOTVICT);     
	    act("In a <R>berserker rage<z>, $n attempts to throw you!", FALSE, this, 0, target, TO_VICT);     
      spinHit(target);
	  }
	  else if (roll >= 23 && roll <= 24 && doesKnowSkill(SKILL_STOMP) && bSuccess(SKILL_STOMP)) {
	    act("You're overcome by your <R>berserker rage<z>!", FALSE, this, 0, target, TO_CHAR);     
	    act("$n is overcome by <R>berserker rage<z>!", FALSE, this, 0, target, TO_NOTVICT);     
	    act("$n is overcome by <R>berserker rage<z>!", FALSE, this, 0, target, TO_VICT);     
      stompHit(target);
    }
	  else if (roll >= 25 && roll <= 29 && doesKnowSkill(SKILL_FOCUS_ATTACK) && bSuccess(SKILL_FOCUS_ATTACK)) {
	    act("Your <R>berserker rage<z> fuels your attacks!", FALSE, this, 0, target, TO_CHAR);     
      focusAttackSuccess(target);
	  }
	  else if (roll == 30 && doesKnowSkill(SKILL_DEATHSTROKE) && bSuccess(SKILL_DEATHSTROKE)) {
	    act("In a <R>berserker rage<z>, you attempt to finish $N with a killing blow!", FALSE, this, 0, target, TO_CHAR);     
	    act("In a <R>berserker rage<z>, $n attempts to finish $N!", FALSE, this, 0, target, TO_NOTVICT);     
	    act("In a <R>berserker rage<z>, $n attempts to finish you with a killing blow!", FALSE, this, 0, target, TO_VICT);     
      deathstrokeSuccess(target);
    }
  }
}

namespace {
  class AdvancedBerserkSkill {
    public:
      spellNumT skillNum{MAX_SKILL};
      double chance{0};
      std::function<bool(const TBeing*)> canUseSkill{
        [](const TBeing*) { return true; }};

      double calcRealChance(const TBeing* ch) const;
  };

  bool isWieldingWeapon(const TBeing* ch) {
    return dynamic_cast<TBaseWeapon*>(ch->heldInPrimHand()) != nullptr;
  };

  bool victimIsStanding(const TBeing* ch) {
    return ch->fight() && ch->fight()->getPosition() >= POSITION_STANDING;
  }

  const std::vector<AdvancedBerserkSkill> advancedBerserkSkills = {
    {SKILL_SLAM, 0.8, isWieldingWeapon},
    {SKILL_BASH, 0.4, victimIsStanding},
    {SKILL_FOCUS_ATTACK, 0.8, isWieldingWeapon},
    {SKILL_HEADBUTT, 0.8, victimIsStanding},
    {SKILL_KNEESTRIKE, 0.8},
    {SKILL_BODYSLAM, 0.4, victimIsStanding},
    {SKILL_SPIN, 0.4, victimIsStanding},
    {SKILL_STOMP, 0.8},
    {SKILL_DEATHSTROKE, 0.8, isWieldingWeapon},
  };

  double AdvancedBerserkSkill::calcRealChance(const TBeing* ch) const {
    double chanceToTestSkill = 1.0;

    for (const auto& skill : advancedBerserkSkills) {
      if (!skill.canUseSkill(ch)) continue;

      if (skill.skillNum == skillNum)
        return chance / chanceToTestSkill;

      chanceToTestSkill *= (100.0 - skill.chance) / 100.0;
    }

    return -1.0;
  }
}  // namespace

int TBeing::doAdvancedBerserkAlt(TBeing* target) {
  static const sstring toChar = "You're overcome by your <R>berserker rage<z>!";
  static const sstring toRoom = "$n is overcome by <R>berserker rage<z>!";

  if (doesKnowSkill(SKILL_BLOODLUST) && percentChance(15)) doBloodlust();

  const AdvancedBerserkSkill* which = nullptr;

  for (const auto& skill : advancedBerserkSkills) {
    if (!doesKnowSkill(skill.skillNum) || !skill.canUseSkill(this) ||
        !percentChance(skill.calcRealChance(this)) || !bSuccess(skill.skillNum))
      continue;

    which = &skill;
    break;
  }

  if (!which) return false;

  act(toChar, false, this, nullptr, nullptr, TO_CHAR);
  act(toRoom, false, this, nullptr, nullptr, TO_ROOM);

  switch (which->skillNum) {
    case SKILL_SLAM:
      return slamSuccess(target);
    case SKILL_BASH: {
      auto* item = heldInSecHand();

      return bashSuccess(target, SKILL_BASH, item && item->isShield(),
        dynamic_cast<TObj*>(item));
    }
    case SKILL_FOCUS_ATTACK:
      return focusAttackSuccess(target);
    case SKILL_HEADBUTT:
      return headbuttHit(target);
    case SKILL_KNEESTRIKE:
      return kneestrikeHit(target);
    case SKILL_BODYSLAM:
      return bodyslamHit(target);
    case SKILL_SPIN:
      return spinHit(target);
    case SKILL_STOMP:
      return stompHit(target);
    case SKILL_DEATHSTROKE:
      return deathstrokeSuccess(target);
    default:
      return false;
  }
}
