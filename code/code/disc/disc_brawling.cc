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
  skSpin(),
  skCloseQuartersFighting(),
  skTaunt()
{
}

CDBrawling::CDBrawling(const CDBrawling &a) :
  CDiscipline(a),
  skGrapple(a.skGrapple),
  skStomp(a.skStomp),
  skBrawlAvoidance(a.skBrawlAvoidance),
  skBodyslam(a.skBodyslam),
  skSpin(a.skSpin),
  skCloseQuartersFighting(a.skCloseQuartersFighting),
  skTaunt(a.skTaunt)
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
  skSpin = a.skSpin;
  skCloseQuartersFighting = a.skCloseQuartersFighting;
  skTaunt = a.skTaunt;
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
    af.duration = PULSE_COMBAT * ::number(1, (int)(af.level/10));// 1-10 rounds
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
