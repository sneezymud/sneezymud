//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "comm.h"
#include "being.h"
#include "obj_base_weapon.h"

int weaponManaDrainer(TBeing* tVictim, cmdTypeT tCmd, const char* arg, TObj* tObj,
  TObj*) {
  TBeing* ch;
  int manaDrawn, rc;

  ch = genericWeaponProcCheck(tVictim, tCmd, tObj, 3);
  if (ch) {
    manaDrawn = ::number(2,5);
    act("A field of <P>energy<z> seeps from $p.", FALSE, ch, tObj, tVictim,
      TO_CHAR);
    act("$n braces $mself as a field of <P>energy<z> seeps from $p.", FALSE, ch,
      tObj, tVictim, TO_NOTVICT);
    act(
      "You feel your energy sucked out of you as a field of <P>energy<z> seeps "
      "from $n's $p.",
      FALSE, ch, tObj, tVictim, TO_VICT);

    tVictim->addToMana(-manaDrawn);
    ch->addToMana(manaDrawn);
    rc = ch->reconcileDamage(tVictim, manaDrawn, DAMAGE_ELECTRIC);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
    return TRUE;
  }

  if (!(ch = dynamic_cast<TBeing*>(tObj->equippedBy)))
    return FALSE;

  if (tCmd == CMD_SAY || tCmd == CMD_SAY2) {
    sstring buf, buf2;
    TBeing* vict = NULL;
    buf = sstring(arg).word(0);
    buf2 = sstring(arg).word(1);

    if (buf == "feed" && buf2 == "me") {
      if (ch->checkObjUsed(tObj)) {
        act("You cannot use $p's powers again this soon.", TRUE, ch, tObj, NULL,
          TO_CHAR, NULL);
        return FALSE;
      }

      if (!(vict = ch->fight())) {
        act("You cannot use $p's powers unless you are fighting.", TRUE, ch, tObj,
          NULL, TO_CHAR, NULL);
        return FALSE;
      }

      ch->addObjUsed(tObj, Pulse::UPDATES_PER_MUDHOUR);

      act("$n's $o glows <P>a bright purple<1> as $e chants a <Y>word of power<1>.",
        TRUE, ch, tObj, NULL, TO_ROOM, NULL);
      act("$n steps back and points $p at $N!<1>", TRUE, ch, tObj, vict,
        TO_NOTVICT, NULL);
      act(
        "<b>A violent ripple of magic shoots from $n's <c>$o<b>, and strikes $N "
        "in the chest!<1>",
        TRUE, ch, tObj, vict, TO_NOTVICT, NULL);
      act("$n braces their $p and points it at you! Uh oh!<1>", TRUE, ch, tObj, vict,
        TO_VICT, NULL);
      act(
        "<b>You feel your soul being yanked by the ray coming from $n's <c>$o<b>",
        TRUE, ch, tObj, vict, TO_VICT, NULL);
      act("Your $o emits a <P>bright purple<z> as you chant, '<p>feed me!<1>'.",
        TRUE, ch, tObj, NULL, TO_CHAR, NULL);
      act("You brace yourself and point $p at $N!<1>", TRUE, ch, tObj, vict, TO_CHAR,
        NULL);
      act(
        "<b>A violent ripple erupts from your <c>$o<b> and strikes $N "
        "in the chest!<1>",
        TRUE, ch, tObj, vict, TO_CHAR, NULL);

      manaDrawn= ::number(10, 60);
          
      tVictim->addToMana(-manaDrawn);
      ch->addToMana(manaDrawn);
      ch->gainCondition(FULL, 15);
      ch->gainCondition(THIRST, 15);

      rc = ch->reconcileDamage(vict, manaDrawn, DAMAGE_ELECTRIC);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_VICT;
      return TRUE;
    }
  }
  return FALSE;
}

