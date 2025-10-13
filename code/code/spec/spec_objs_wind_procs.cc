#include "comm.h"
#include "obj_base_weapon.h"
#include "being.h"

void doFlight(TBeing* ch, TObj* o) {
  affectedData aff;

  if (ch->checkObjUsed(o)) {
    act("You cannot use $p's powers again this soon.", TRUE, ch, o, NULL,
      TO_CHAR, NULL);
    return;
  }

  if (ch->affectedBySpell(SPELL_FLY) || ch->affectedBySpell(SPELL_FALCON_WINGS))
    return;

  ch->addObjUsed(o, 6 * Pulse::UPDATES_PER_MUDHOUR);

  aff.type = SPELL_FLY;
  aff.level = 25;
  aff.duration = 10 * ch->GetMaxLevel();
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_FLYING;

  act("$n twirls $m $o as $e whispers <W>\"East Wind\"<z>!", 0, ch, o, 0, TO_ROOM);
  act("You twirl your $o as you whisper <W>\"East Wind\"<z>!", 0, ch, o, 0, TO_CHAR);

  act("You feel much \"lighter\"!", 0, ch, o, 0, TO_CHAR);
  act("$n seems lighter on $s feet!", 0, ch, o, 0, TO_ROOM);

  ch->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES, FALSE);

}

void doCeler(TBeing* ch, TObj* o) {
  affectedData aff;

  if (ch->affectedBySpell(SPELL_CELERITE))
    return;

  aff.type = SPELL_CELERITE;
  aff.level = 25;
  aff.duration = 20 + ch->GetMaxLevel();
  aff.location = APPLY_NONE;
  aff.bitvector = 0;

  act("$n's $o <Y>glows<1> and $e seems to move more easily.", 0, ch, o, 0, TO_ROOM);
  act("Your $o <Y>glows<1> and you feel your pace quicken.", 0, ch, o, 0, TO_CHAR);

  ch->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES, FALSE);
}

int doWindDamage(TBeing* ch, TBeing* vict, TObj* o) {
  int rc = 0;
  int dam = ::number(4, 12);

  act("<W>A gust of wind bursts from $n's $o to buffet $N.<z>", FALSE, ch, o, vict, TO_NOTVICT);
  act("<W>A gust of wind bursts from $n's $o to buffet you.<z>", FALSE, ch, o, vict, TO_VICT);
  act("<W>A gust of wind bursts from your $o to buffet $N.<z>", FALSE, ch, o, vict, TO_CHAR);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_GUST);
  if (rc == -1)
    return DELETE_VICT;
  return TRUE;
}

int doGust(TBeing* ch, TBeing* vict, TObj* o) {
  int rc = 0;
  TThing* v;
  int dam = ::number(11, 25);

  act("<W>A gust of wind bursts from $n's $o to buffet $N.<z>", FALSE, ch, o, vict, TO_NOTVICT);
  act("The <W>wind<1> picks $N up in the <C>air<1> and slams $M to the $G.<z>", FALSE, ch, o, vict, TO_NOTVICT);

  act("<W>A gust of wind bursts from $n's $o to buffet you.<z>", FALSE, ch, o, vict, TO_VICT);
  act("The <W>wind<1> picks you up in the <C>air<1> and slams you to the $G.<z>", FALSE, ch, o, vict, TO_VICT);

  act("<W>A gust of wind bursts from your $o to buffet $N.<z>", FALSE, ch, o, vict, TO_CHAR);
  act("The <W>wind<1> picks $N up in the <C>air<1> and slams $M to the $G.<z>", FALSE, ch, o, vict, TO_CHAR);


  // Dismount victim or dismount riders
  if (vict->riding)
    vict->dismount(POSITION_STANDING);
  while ((v = vict->rider)) {
    TBeing* tb = dynamic_cast<TBeing*>(v);
    if (tb) {
      rc = tb->fallOffMount(vict, POSITION_STANDING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tb;
        tb = NULL;
      }
    } else {
      v->dismount(POSITION_DEAD);
    }
  }

  vict->addToDistracted(1, FALSE);
  vict->setPosition(POSITION_SITTING);
  vict->addToWait(combatRound(1));
  rc = ch->reconcileDamage(vict, dam, DAMAGE_GUST);
  if (rc == -1)
    return DELETE_VICT;
  return TRUE;
}


// Tier 1 has wind damage only
int gustWeaponT1(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;
  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (!ch)
    return FALSE;
  
  return doWindDamage(ch, vict, o);
}

// Tier 2 has wind damge and a chance to knock down as well as celerite buff
int gustWeaponT2(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;
  TBeing* combatant;

  if (!o)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return FALSE;  // weapon not equipped (carried or on ground)

  if (cmd == CMD_GENERIC_PULSE) {
    if ( (combatant = ch->fight()) && (!::number(0,11)) ) {
      doCeler(ch, o);
      return TRUE;
    }
    return FALSE;
  }

  if (cmd == CMD_OBJ_HIT && vict) {
    if (!::number(0, 3)) {
      if (!::number(0, 4))
        return doGust(ch, vict, o);
      return doWindDamage(ch, vict, o);
    }
  }

  return FALSE;
}

// Tier 3 has all of the above and also a flurry proc and spoken command
int gustWeaponT3(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;
  TBeing* combatant;

  if (!o)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return FALSE;  // weapon not equipped (carried or on ground)

  if (cmd == CMD_GENERIC_PULSE) {
    if ( (combatant = ch->fight()) && (!::number(0,7)) ) {
      doCeler(ch, o);
      return TRUE;
    }
    return FALSE;
  }

  if (cmd == CMD_OBJ_HIT && vict) {
    if (!::number(0, 3)) {
      if (!::number(0, 3))
        return doGust(ch, vict, o);
      return doWindDamage(ch, vict, o);
    }
  }

  if (cmd == CMD_OBJ_HITTING && vict) {
    if (!::number(0, 49)) {
      SET_BIT(ch->specials.affectedBy, AFF_FLURRY);
      return FALSE;
    }
  }

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    sstring buf, buf2;
    buf = sstring(arg).word(0);
    buf2 = sstring(arg).word(1);

    if (buf == "east" && buf2 == "wind") {
      doFlight(ch, o);
      return TRUE;
    }
  
  }

  return FALSE;

}
