#include "comm.h"
#include "obj_base_weapon.h"
#include "being.h"

void doWord(TBeing* ch, TObj* o) {
  TBeing* vict = nullptr;

  if (ch->checkObjUsed(o)) {
    act("You cannot use $p's powers again this soon.", true, ch, o, nullptr,
      TO_CHAR, nullptr);
    return;
  }
  if (!(vict = ch->fight())) {
    act("You cannot use $p's powers unless you are fighting.", true, ch, o,
      nullptr, TO_CHAR, nullptr);
    return;
  }

  ch->addObjUsed(o, 2 * Pulse::UPDATES_PER_MUDHOUR);

  affectedData aff1, aff2;
  int level = ch->GetMaxLevel();

  // normalize bonus between 1 and 5
  int modifier = max(1, (int)(level / 10));
  modifier = min(5, modifier);

  aff1.type = SPELL_BIND;
  aff1.level = level;
  aff1.bitvector = AFF_WEB;
  aff1.location = APPLY_ARMOR;
  aff1.modifier = 50 + (level * 2);
  aff1.duration = (level / 5) * Pulse::UPDATES_PER_MUDHOUR;

  aff2.type = SPELL_BIND;
  aff2.level = level;
  aff2.bitvector = AFF_WEB;
  aff2.location = APPLY_HITROLL;
  aff2.modifier = (-modifier);
  aff2.duration = (level / 5) * Pulse::UPDATES_PER_MUDHOUR;

  act(
    "<k>$n's $o glows <z><r>blood red<1><k> as $e chants a <z><r>word of "
    "power<1><k>.<1>",
    true, ch, o, nullptr, TO_ROOM, nullptr);
  act(
    "<k>Your $o glows <z><r>blood red<1><k> as you chant the <z><r>word of "
    "power<1><k>.<1>",
    true, ch, o, nullptr, TO_CHAR, nullptr);

  act(
    "<k>A web of shadows erupts from $n's $o<k>, and strikes $N<k> full on!<1>",
    true, ch, o, vict, TO_NOTVICT, nullptr);
  act("<k>A web of shadows erupts from $n's $o<k>, and strikes you full on!<1>",
    true, ch, o, vict, TO_VICT, nullptr);
  act(
    "<k>A web of shadows erupts from your $o<k>, and strikes $N<k> full on!<1>",
    true, ch, o, vict, TO_CHAR, nullptr);

  vict->affectTo(&aff1);
  vict->affectTo(&aff2);
}

void doWrath(TBeing* ch, TObj* o) {
  affectedData aff;

  // normalize bonus between 1 and 5
  int modifier = max(1, (int)(ch->GetMaxLevel() / 15));
  modifier = min(3, modifier);

  aff.type = AFFECT_UNHOLY_WRATH;
  aff.duration = Pulse::UPDATES_PER_MUDHOUR / 40 * ch->GetMaxLevel();
  aff.location = APPLY_HITROLL;
  aff.modifier = modifier;
  aff.bitvector = 0;

  if (!ch->affectedBySpell(AFFECT_UNHOLY_WRATH)) {
    act("<k>$n's $o glows <z><r>blood red<k> with <R>Rage<1>!", 0, ch, o, 0,
      TO_ROOM);
    act("<k>Your $o glows <z><r>blood red<k> with <R>Rage<1>!", 0, ch, o, 0,
      TO_CHAR);
  }

  ch->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES, false);
}

int doBleed(TBeing* ch, TBeing* vict, const char* arg, TObj* o) {
  int rc, dam;
  char buf[256];
  wearSlotT part_hit = wearSlotT((long int)arg);

  if (notBleedSlot(part_hit))
    return false;

  dam = ::number(4, 10);

  sprintf(buf, "$p<k> howls as it strikes deep into <1>$n<k>'s %s.<1>",
    vict->describeBodySlot(part_hit).c_str());
  act(buf, 0, vict, o, 0, TO_ROOM);
  sprintf(buf, "$p<k> howls as it strikes deep into your %s.<1>",
    vict->describeBodySlot(part_hit).c_str());
  act(buf, 0, vict, o, 0, TO_CHAR);

  if (!vict->isImmune(IMMUNE_BLEED, part_hit)) {
    vict->rawBleed(part_hit, 250, SILENT_YES, CHECK_IMMUNITY_NO);
    vict->rawInfect(part_hit, 250, SILENT_YES, CHECK_IMMUNITY_NO);
    sprintf(buf, "<r>Blood<1> <k>drips out of the wound.<1>");
    act(buf, 0, vict, o, 0, TO_ROOM);
    sprintf(buf, "<r>Blood<1> <k>drips out of the wound.<1>");
    act(buf, 0, vict, o, 0, TO_CHAR);
  }

  rc = ch->reconcileDamage(vict, dam, DAMAGE_NORMAL);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return true;
}

int doLifeLeach(TBeing* ch, TBeing* vict, TObj* o) {
  int rc, dam;

  dam = ::number(18, 40);

  act("$p <k>glows <z><r>blood red<k> as it drains <z>$n<k>'s life.<1>", 0,
    vict, o, 0, TO_ROOM);
  act("$p <k>glows <z><r>blood red<k> as it drains your life.<1>", 0, vict, o,
    0, TO_CHAR);

  ch->addToHit(dam);
  rc = ch->reconcileDamage(vict, dam, DAMAGE_DRAIN);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return true;
}

int dkSword(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;

  if (!o)
    return false;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)

  if (cmd == CMD_GENERIC_PULSE) {
    if (!::number(0, 29)) {
      doWrath(ch, o);
      return true;
    }
  }

  if (cmd == CMD_OBJ_HIT && vict) {
    if (!::number(0, 3)) {
      if (!::number(0, 5))
        return doLifeLeach(ch, vict, o);
      return doBleed(ch, vict, arg, o);
    }
  }

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    sstring buf, buf2;
    buf = sstring(arg).word(0);
    buf2 = sstring(arg).word(1);

    if (buf == "unholy" && buf2 == "word") {
      doWord(ch, o);
      return true;
    }
  }

  return false;
}
