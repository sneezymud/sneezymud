#include "being.h"
#include "handler.h"
#include "combat.h"
#include "disc_deikhan_vengeance.h"

int TBeing::doSmite(const char* arg, TBeing* victim) {
  TBeing* vict;
  char tmp[80];
  int rc;

  if (checkBusy()) {
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_SMITE)) {
    sendTo("You know nothing about smiting.\n\r");
    return FALSE;
  }
  strcpy(tmp, arg);
  if (!(vict = victim)) {
    if (!(vict = get_char_room_vis(this, tmp))) {
      if (!(vict = fight())) {
        sendTo("Smite whom?\n\r");
        return FALSE;
      }
    }
  }

  if (!sameRoom(*vict)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  if (noHarmCheck(vict))
    return FALSE;
  rc = smite(this, vict);
  if (rc)
    addSkillLag(SKILL_SMITE, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (victim)
      return rc;
    delete vict;
    vict = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

int smite(TBeing* ch, TBeing* v) {
  TThing* weap;

  if (ch->getPosition() < POSITION_STANDING) {
    ch->sendTo(
      "It is undignified to smite someone while not on your own two feet.\n\r");
    return FALSE;
  }
  if (ch == v) {
    ch->sendTo(
      "You contemplate smiting yourself.\n\rYou realize there are better ways "
      "to punish yourself.\n\r");
    return FALSE;
  }
  if (ch->checkPeaceful(
        "This room is too peaceful to contemplate violence in.\n\r"))
    return FALSE;

  if (!(weap = ch->heldInPrimHand())) {
    ch->sendTo("Perhaps you'd like to smite with something next time...?\n\r");
    return FALSE;
  }
  return weap->smiteWithMe(ch, v);
}

int TThing::smiteWithMe(TBeing* ch, TBeing*) {
  ch->sendTo("Perhaps you'd like to smite with a weapon next time...?\n\r");
  return FALSE;
}
