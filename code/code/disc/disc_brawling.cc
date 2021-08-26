//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "disc_brawling.h"

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

  addSkillLag(SKILL_TAUNT, 0);

  return TRUE;
}
