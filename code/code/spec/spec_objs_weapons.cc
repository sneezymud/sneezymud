#include <cstdio>

#include "extern.h"
#include "room.h"
#include "handler.h"
#include "low.h"
#include "pathfinder.h"
#include "materials.h"
#include "obj_trap.h"
#include "obj_base_weapon.h"
#include "obj_general_weapon.h"
#include "disc_fire.h"
#include "being.h"
#include "weather.h"

int ghostlyShiv(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;

  if (!(ch = genericWeaponProcCheck(vict, cmd, o, 3)))
    return false;

  act(
    "<k>Ghosts of $n's former enemies announce their presence with a "
    "shriek.<1>",
    0, ch, o, vict, TO_ROOM);

  act(
    "<k>The ghosts of your former enemies assail $N with with frightening "
    "shrieks.<1>",
    0, ch, o, vict, TO_CHAR);

  vict->doFlee("");

  return true;
}

int iceStaff(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;

  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (ch) {
    dam = ::number(4, 10);
    if (dam < 8) {
      act("$p becomes covered with ice and freezes $n.", 0, vict, o, 0, TO_ROOM,
        ANSI_CYAN);
      act("$p becomes covered with ice and freezes you.", 0, vict, o, 0,
        TO_CHAR, ANSI_CYAN);
    } else {
      act("$p becomes covered with ice and sends a violent chill through $n.",
        0, vict, o, 0, TO_ROOM, ANSI_BLUE);
      act("$p becomes covered with ice and sends a violent chill through you.",
        0, vict, o, 0, TO_CHAR, ANSI_BLUE);
    }

    rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
    return true;
  }

  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    sstring buf, buf2;
    TBeing* vict = nullptr;
    buf = sstring(arg).word(0);
    buf2 = sstring(arg).word(1);

    if (buf == "chill" && buf2 == "out") {
      if (ch->checkObjUsed(o)) {
        act("You cannot use $p's powers again this soon.", true, ch, o, nullptr,
          TO_CHAR, nullptr);
        return false;
      }

      if (!(vict = ch->fight())) {
        act("You cannot use $p's powers unless you are fighting.", true, ch, o,
          nullptr, TO_CHAR, nullptr);
        return false;
      }

      ch->addObjUsed(o, Pulse::UPDATES_PER_MUDHOUR);

      act("$n's $o glows <b>a cold blue<1> as $e growls a <p>word of power<1>.",
        true, ch, o, nullptr, TO_ROOM, nullptr);
      act("$n steps back and points $p at $N!<1>", true, ch, o, vict,
        TO_NOTVICT, nullptr);
      act(
        "<b>An incredibly cold ray erupts from $n's <c>$o<b>, and strikes $N "
        "full on!<1>",
        true, ch, o, vict, TO_NOTVICT, nullptr);
      act("$n steps back and points $p at you!  Uh oh!<1>", true, ch, o, vict,
        TO_VICT, nullptr);
      act(
        "<b>An incredibly cold ray erupts from $n's <c>$o<b>, and strikes you "
        "full on!<1>",
        true, ch, o, vict, TO_VICT, nullptr);
      act("Your $o glows <b>a cold blue<1> as you growl, '<p>chill out<1>'.",
        true, ch, o, nullptr, TO_CHAR, nullptr);
      act("You step back and point $p at $N!<1>", true, ch, o, vict, TO_CHAR,
        nullptr);
      act(
        "<b>An incredibly cold ray erupts from your <c>$o<b>, and strikes $N "
        "full on!<1>",
        true, ch, o, vict, TO_CHAR, nullptr);

      int dam = ::number(10, 60);
      rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        vict->reformGroup();
        delete vict;
        vict = nullptr;
      }

      return true;
    }
  }
  return false;
}

int nightBlade(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  // it used to do magic-missile every round
  // this is a reasonable facsimile

  TBeing* ch;
  int rc;

  ch = genericWeaponProcCheck(vict, cmd, o, 6);
  if (!ch)
    return false;

  if (!ch->outside())
    return false;
  if (!GameTime::is_nighttime())
    return false;

  act(
    "A pulse of darkness as black as the new moon travels up the blade of $p.",
    false, ch, o, nullptr, TO_CHAR, ANSI_BLACK);
  act(
    "A pulse of darkness as black as the new moon travels up the blade of $p.",
    false, ch, o, nullptr, TO_ROOM, ANSI_BLACK);

  act("<p>WOOMPF!!<z>", false, ch, nullptr, nullptr, TO_CHAR);
  act("<p>WOOMPF!!<z>", false, ch, nullptr, nullptr, TO_ROOM);

  act("$p discharges its energy into $n.", false, vict, o, nullptr, TO_ROOM);
  act("$p discharges its energy into you!", false, vict, o, nullptr, TO_CHAR);

  int dam = ::number(5, 8);
  rc = ch->reconcileDamage(vict, dam, DAMAGE_NORMAL);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  return true;
}

int daySword(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  // it used to do magic-missile every round
  // this is a reasonable facsimile

  TBeing* ch;
  int rc;

  ch = genericWeaponProcCheck(vict, cmd, o, 6);
  if (!ch)
    return false;

  if (!ch->outside())
    return false;
  if (!GameTime::is_daytime())
    return false;

  act("A pulse of light as bright as the sun travels up the blade of $p.",
    false, ch, o, nullptr, TO_CHAR, ANSI_YELLOW);
  act("A pulse of light as bright as the sun travels up the blade of $p.",
    false, ch, o, nullptr, TO_ROOM, ANSI_YELLOW);

  act("<r>WOOMPF!!<z>", false, ch, nullptr, nullptr, TO_CHAR);
  act("<r>WOOMPF!!<z>", false, ch, nullptr, nullptr, TO_ROOM);

  act("$p discharges its energy into $n.", false, vict, o, nullptr, TO_ROOM);
  act("$p discharges its energy into you!", false, vict, o, nullptr, TO_CHAR);

  int dam = ::number(5, 8);
  rc = ch->reconcileDamage(vict, dam, DAMAGE_NORMAL);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  return true;
}

int weaponShadowSlayer(TBeing* vict, cmdTypeT cmd, const char*, TObj* o,
  TObj*) {
  TBeing* ch;
  int rc, dam = 1;
  //  ch = genricWeaponProcCheck(vict,cmd,o,5);
  //  if ((!(ch)) || !(vict->getFaction() == FACT_CULT || vict->isUndead()))
  //    return false;
  //

  if (!o || !vict)
    return false;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)
  if (::number(0, 6))
    return false;
  if (cmd != CMD_OBJ_HIT)
    return false;
  if (!(vict->getFaction() == FACT_CULT || vict->isUndead()))
    return false;
  int hitterLev = ch->GetMaxLevel();
  dam = (::number((hitterLev / 10 + 1), (hitterLev / 3 + 4)));
  act(
    "<1>Your $o hums, and begins to glow with an incredible <W>white "
    "light<1>.<1>",
    true, ch, o, vict, TO_CHAR, nullptr);
  act(
    "<1>$n's $o hums, and begins to glow with an incredible <W>white "
    "light<1>.<1>",
    true, ch, o, vict, TO_NOTVICT, nullptr);
  act(
    "<1>$n's $o hums, and begins to glow with a painful <W>white light<1>.<1>",
    true, ch, o, vict, TO_VICT, nullptr);

  if (dam >= ((((hitterLev / 3 + 4) - (hitterLev / 10 + 1)) * 4) / 5 +
               (hitterLev / 10 + 1))) {
    act(
      "<W>$N howls in pain as a HUGE flash of energy from your $o is released "
      "into $m! <1>",
      true, ch, o, vict, TO_CHAR, nullptr);
    act(
      "<W>$N howls in pain as a HUGE flash of energy from $n's $o is released "
      "into $m!<1>",
      true, ch, o, vict, TO_NOTVICT, nullptr);
    act(
      "<W>There is a huge flash as the energy from $n's $o is released into "
      "you!<1>  That really hurt!!<1>",
      true, ch, o, vict, TO_VICT, nullptr);
  } else {
    act("<W>$N grunts as the energy from your $o is released into $m.<1>", true,
      ch, o, vict, TO_CHAR, nullptr);
    act("<W>$N grunts as the energy from $n's $o is released into $m.<1>", true,
      ch, o, vict, TO_NOTVICT, nullptr);
    act(
      "<W>You grunt in pain as the energy from $n's blasted $o is released "
      "into you.<1>",
      true, ch, o, vict, TO_VICT, nullptr);
  }

  if (!(ch->getFaction() == FACT_BROTHERHOOD)) {
    dam = dam / 2;
    act(
      "<1>Your $o rebels against you, releasing <W>energy<1> into your "
      "hand!<1>",
      true, ch, o, vict, TO_CHAR, nullptr);
    act("<1>$n's $o rebels against $m, releasing <W>energy<1> into $s hand!<1>",
      true, ch, o, vict, TO_NOTVICT, nullptr);
    act(
      "<1>$n's $o rebels against $m, releasing <W>energy<1> into $s hand!  "
      "Sucker!<1>",
      true, ch, o, vict, TO_VICT, nullptr);
    rc = ch->reconcileDamage(ch, dam, TYPE_SMITE);
    if (ch->getHit() < 0) {
      ch->setHit(0);
      ch->setPosition(POSITION_STUNNED);
    }
    if (!ch->isTough()) {
      *ch->roomp += *ch->unequip(o->eq_pos);
      act("$n screams loudly, dropping $s $p.", 1, ch, o, nullptr, TO_ROOM);
      act("You scream loudly, dropping your $p.", 1, ch, o, nullptr, TO_CHAR);
    }
  }

  rc = ch->reconcileDamage(vict, dam, TYPE_SMITE);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return true;
}

int blazeOfGlory(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  // Use of this quest prize weapon takes two seprate actions. One is a sort of
  // 'charge-up' move, where the char steps back from combat, and focuses on
  // charging up the weapon.
  // the release part of the proc happens on a normal swing attempt. The energy
  // stored in the first part is released in a giant ball of fire, consuming the
  // char, and hopefully his victim as well. Kaboooom!   -love, Dash
  int dam = 0, rc;
  TBeing* ch;

  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;

  if (cmd == CMD_OBJ_HITTING && ch->checkForSkillAttempt(SPELL_BLAST_OF_FURY)) {
    act(
      "<o>The air about you seems to crackle with power as you level your $o "
      "at $N<o>.<1>",
      true, ch, o, vict, TO_CHAR, nullptr);
    act("<o>Brandishing $p<o>, you prepare to charge!<1>", true, ch, o, vict,
      TO_CHAR, nullptr);
    act(
      "  ...as you begin your rush, flames spread out from your $o, and "
      "envelope you...",
      true, ch, o, vict, TO_CHAR, nullptr);
    act("    ...your vision begins to go red...", true, ch, o, vict, TO_CHAR,
      nullptr);
    act(
      "      ...white hot flames tear across your entire body, the pain is "
      "unbearable...",
      true, ch, o, vict, TO_CHAR, nullptr);
    act(
      "        ...with $N just ahead of you, you prepare your $o for impact...",
      true, ch, o, vict, TO_CHAR, nullptr);

    act(
      "<o>The air about $n<o> seems to crackle with power as $e levels $s $o "
      "at $N.<1>",
      true, ch, o, vict, TO_NOTVICT, nullptr);
    act("<o>Brandishing $p<o>, $n<o> prepares to charge!<1>", true, ch, o, vict,
      TO_NOTVICT, nullptr);
    act("  ...flames spread out from $s $o, and envelope $m...", true, ch, o,
      vict, TO_NOTVICT, nullptr);
    act("    ...$n screams with rage as $e barrels at $N...", true, ch, o, vict,
      TO_NOTVICT, nullptr);
    act("      ...white hot flames tear across $s entire body...", true, ch, o,
      vict, TO_NOTVICT, nullptr);
    act("        ...you see $n prepare $s $o for impact...", true, ch, o, vict,
      TO_NOTVICT, nullptr);

    act(
      "<o>The air about $n<o> seems to crackle with power as $e levels $s $o "
      "at you. Uh oh.<1>",
      true, ch, o, vict, TO_VICT, nullptr);
    act("<o>Brandishing $p<o>, $n<o> prepares to charge!<1>", true, ch, o, vict,
      TO_VICT, nullptr);
    act("  ...flames spread out from $s $o, and envelope $m...", true, ch, o,
      vict, TO_VICT, nullptr);
    act("    ...$n screams with rage as $e barrels at you...", true, ch, o,
      vict, TO_VICT, nullptr);
    act("      ...white hot flames tear across $s entire body...", true, ch, o,
      vict, TO_VICT, nullptr);
    act("        ...you see $n prepare $s $o for impact...", true, ch, o, vict,
      TO_VICT, nullptr);

    dam =
      min(30000, ch->hitLimit() * 100);  // kill them DEAD. I want NO survivors.
    o->addToStructPoints(-5);
    o->setDepreciation(o->getDepreciation() + 5);
    if (o->getStructPoints() <= 0) {
      if (!o->makeScraps())
        delete o;
    }
    int rc2 = ch->reconcileDamage(ch, dam, DAMAGE_FIRE);
    act(
      "<R>KA-BOOOOOOOOOOM! You explode in a <O>blaze of glory<R> as you crash "
      "into $N<R>!!!<1>",
      true, ch, nullptr, vict, TO_CHAR, nullptr);
    act(
      "<R>KA-BOOOOOOOOOOM! $n explodes in a <O>blaze of glory<R> as $e crashes "
      "into $N<R>!!!<1>",
      true, ch, nullptr, vict, TO_NOTVICT, nullptr);
    act(
      "<R>KA-BOOOOOOOOOOM! You're covered in <O>searing flames<R> as $e "
      "crashes into you!!!<1>",
      true, ch, nullptr, vict, TO_VICT, nullptr);
    act("<o>With a loud crack, $n<o>'s corpse shatters!<1>", true, ch, nullptr,
      vict, TO_ROOM, nullptr);

    dam = min(30000, ch->GetMaxLevel() * 40);
    rc = ch->reconcileDamage(vict, dam, DAMAGE_FIRE);

    ch->makeBodyPart(WEAR_HEAD);
    ch->makeBodyPart(WEAR_ARM_L);
    ch->makeBodyPart(WEAR_ARM_R);
    ch->makeBodyPart(WEAR_LEG_L);
    ch->makeBodyPart(WEAR_LEG_R);
    // muahaha corpse explodes... limbs fly EVERYWHERE!!
    if (IS_SET_DELETE(rc2, DELETE_VICT) || (ch->getHit() < -10))
      delete ch;

    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
    return true;

  } else if ((cmd == CMD_SAY || cmd == CMD_SAY2) &&
             !(ch->checkForSkillAttempt(SPELL_BLAST_OF_FURY))) {
    affectedData aff;
    sstring buf = sstring(arg).word(0);

    if (buf == "aerolithe") {  // this is the activation keyword
      aff.type = AFFECT_SKILL_ATTEMPT;
      aff.level = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = 0;
      aff.duration = 20;
      aff.modifier = SPELL_BLAST_OF_FURY;
      ch->affectTo(&aff);
      ch->addToWait(combatRound(4));
      ch->cantHit += ch->loseRound(3);

      act("$n holds $p high above $s head, shouting <p>a word of power<1>.",
        true, ch, o, nullptr, TO_ROOM, nullptr);
      act(
        "A gust of scorching wind whips past $m, and flames lick out from $s "
        "$o.",
        true, ch, o, nullptr, TO_ROOM, nullptr);
      act(
        "<o>The air crackles with intense power as $n<o> is suddenly immolated "
        "in <r>flames<o>!<1>",
        true, ch, o, nullptr, TO_ROOM, nullptr);

      act(
        "You hold $p high above your head, shouting the <p>word of power<1>, "
        "<p>Aerolithe<1>!",
        true, ch, o, nullptr, TO_CHAR, nullptr);
      act(
        "A gust of scorching wind whips past you, and flames lick out from "
        "your $o.",
        true, ch, o, nullptr, TO_CHAR, nullptr);
      act(
        "<o>The air crackles with intense power as you are suddenly immolated "
        "in <r>flames<o>!<1>",
        true, ch, o, nullptr, TO_CHAR, nullptr);

      dam = ch->hitLimit() / 10;
      rc = ch->reconcileDamage(ch, dam, DAMAGE_FIRE);
      if (IS_SET_DELETE(rc, DELETE_VICT) || (ch->getHit() < -10))
        delete ch;
      return true;
    }
    return false;
  }
  if (cmd == CMD_GENERIC_PULSE &&
      ch->checkForSkillAttempt(SPELL_BLAST_OF_FURY)) {
    act(
      "<r>$n<r> seems to shudder in pain, and there is a strange fire burning "
      "in $s eyes.<1>",
      true, ch, o, nullptr, TO_ROOM, nullptr);
    act(
      "<r>The power contained within you is almost unbearable!<R> You're "
      "burning up!!!<1>",
      true, ch, o, nullptr, TO_CHAR, nullptr);
    dam = ch->hitLimit() / 10;
    rc = ch->reconcileDamage(ch, dam, DAMAGE_FIRE);
    if (IS_SET_DELETE(rc, DELETE_VICT) || (ch->getHit() < -10))
      delete ch;
    return true;
  }
  return false;
}

int elementalWeapon(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o,
  TObj*) {
  int dam = 0, rc = 0;
  TBeing* ch;
  // blaaaah blah blah
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;

  if (cmd == CMD_OBJ_HIT && genericWeaponProcCheck(vict, cmd, o, 3)) {
    dam = ::number(4, 10);
    if (ch->affectedBySpell(SPELL_CONJURE_WATER)) {
      if (dam < 8) {
        act("<b>Your <c>$o <b>becomes covered with ice and freezes $N.<1>",
          true, ch, o, vict, TO_CHAR, nullptr);
        act("<b>$n's <c>$o <b>becomes covered with ice and freezes $N.<1>",
          true, ch, o, vict, TO_NOTVICT, nullptr);
        act("<b>$n's <c>$o <b>becomes covered with ice and freezes you.<1>",
          true, ch, o, vict, TO_VICT, nullptr);
      } else {
        act(
          "<B>Your <C>$o <B>becomes covered with ice and sends a violent chill "
          "through $N.<1>",
          true, ch, o, vict, TO_CHAR, nullptr);
        act(
          "<B>$n's <C>$o <B>becomes covered with ice and sends a violent chill "
          "through $N.<1>",
          true, ch, o, vict, TO_NOTVICT, nullptr);
        act(
          "<B>$n's <C>$o <B>becomes covered with ice and sends a violent chill "
          "through you.<1>",
          true, ch, o, vict, TO_VICT, nullptr);
      }
      rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
    } else if (ch->affectedBySpell(SPELL_CONJURE_FIRE)) {
      if (dam < 8) {
        act("<o>Your <r>$o <o>erupts into roaring flames and sears $N.<1>",
          true, ch, o, vict, TO_CHAR, nullptr);
        act("<o>$n's <r>$o <o>erupts into roaring flames and sears $N.<1>",
          true, ch, o, vict, TO_NOTVICT, nullptr);
        act("<o>$n's <r>$o <o>erupts into roaring flames and sears you.<1>",
          true, ch, o, vict, TO_VICT, nullptr);
      } else {
        act("<O>Your <R>$o <O>roars into a blaze of fire and scorches $N.<1>",
          true, ch, o, vict, TO_CHAR, nullptr);
        act("<O>$n's <R>$o <O>roars into a blaze of fire and scorches $N.<1>",
          true, ch, o, vict, TO_NOTVICT, nullptr);
        act("<O>$n's <R>$o <O>roars into a blaze of fire and scorches you.<1>",
          true, ch, o, vict, TO_VICT, nullptr);
      }
      rc = ch->reconcileDamage(vict, dam, DAMAGE_FIRE);
    } else if (ch->affectedBySpell(SPELL_CONJURE_AIR)) {
      if (dam < 8) {
        act("<c>Your <w>$o <c>crackles with electricity and shocks $N.<1>",
          true, ch, o, vict, TO_CHAR, nullptr);
        act("<c>$n's <w>$o <c>crackles with electricity and shocks $N.<1>",
          true, ch, o, vict, TO_NOTVICT, nullptr);
        act("<c>$n's <w>$o <c>crackles with electricity and shocks you.<1>",
          true, ch, o, vict, TO_VICT, nullptr);
      } else {
        act("<C>Your <W>$o <C>discharges a large jolt of electricity at $N.<1>",
          true, ch, o, vict, TO_CHAR, nullptr);
        act("<C>$n's <W>$o <C>discharges a large jolt of electricity at $N.<1>",
          true, ch, o, vict, TO_NOTVICT, nullptr);
        act(
          "<C>$n's <W>$o <C>discharges a large jolt of electricity at you.<1>",
          true, ch, o, vict, TO_VICT, nullptr);
      }
      rc = ch->reconcileDamage(vict, dam, DAMAGE_ELECTRIC);
    } else
      return false;
    if (rc == -1)
      return DELETE_VICT;
    return true;
  } else if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    affectedData aff, aff2;
    sstring buf = sstring(arg).word(0);

    if (buf == "rime") {  // this is the activation keyword
      if (ch->checkForSkillAttempt(SPELL_CONJURE_WATER)) {
        act("The $o's power of ice can only be used once a day!", true, ch, o,
          nullptr, TO_CHAR, nullptr);
        return true;
      } else if (ch->affectedBySpell(SPELL_CONJURE_FIRE)) {
        act(
          "The $o's power of ice cannot be used at the same time as the power "
          "of fire!",
          true, ch, o, nullptr, TO_CHAR, nullptr);
        return true;
      } else if (ch->affectedBySpell(SPELL_CONJURE_AIR)) {
        act(
          "The $o's power of ice cannot be used at the same time as the power "
          "of lightning!",
          true, ch, o, nullptr, TO_CHAR, nullptr);
        return true;
      }
      aff.type = AFFECT_SKILL_ATTEMPT;
      aff.level = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = 0;
      aff.duration = 24 * Pulse::UPDATES_PER_MUDHOUR;
      aff.modifier = SPELL_CONJURE_WATER;
      aff2.type = SPELL_CONJURE_WATER;
      aff2.level = 0;
      aff2.location = APPLY_NONE;
      aff2.bitvector = 0;
      aff2.duration = 2 * Pulse::UPDATES_PER_MUDHOUR;
      aff2.modifier = 0;
      if (!(ch->isImmortal()))
        ch->affectTo(&aff);
      ch->affectTo(&aff2);
      ch->addToWait(combatRound(2));

      act("$n brandishes $p, shouting a strange <p>word of power<1>.", true, ch,
        o, nullptr, TO_ROOM, nullptr);
      act(
        "<b>A chill wind swirls around $n<b>, and $s <B>$o<1><b> forms a thin "
        "layer of ice<1>.",
        true, ch, o, nullptr, TO_ROOM, nullptr);

      act("You brandish $p, shouting the command word, <p>rime<1>!", true, ch,
        o, nullptr, TO_CHAR, nullptr);
      act(
        "<b>A chill wind swirls around you, and your <B>$o<1><b> forms a thin "
        "layer of ice<1>.",
        true, ch, o, nullptr, TO_CHAR, nullptr);
    } else if (buf == "incandesce") {  // this is the activation keyword
      if (ch->checkForSkillAttempt(SPELL_CONJURE_FIRE)) {
        act("The $o's power of fire can only be used once a day!", true, ch, o,
          nullptr, TO_CHAR, nullptr);
        return true;
      } else if (ch->affectedBySpell(SPELL_CONJURE_WATER)) {
        act(
          "The $o's power of fire cannot be used at the same time as the power "
          "of ice!",
          true, ch, o, nullptr, TO_CHAR, nullptr);
        return true;
      } else if (ch->affectedBySpell(SPELL_CONJURE_AIR)) {
        act(
          "The $o's power of fire cannot be used at the same time as the power "
          "of lightning!",
          true, ch, o, nullptr, TO_CHAR, nullptr);
        return true;
      }
      aff.type = AFFECT_SKILL_ATTEMPT;
      aff.level = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = 0;
      aff.duration = 24 * Pulse::UPDATES_PER_MUDHOUR;
      aff.modifier = SPELL_CONJURE_FIRE;
      aff2.type = SPELL_CONJURE_FIRE;
      aff2.level = 0;
      aff2.location = APPLY_NONE;
      aff2.bitvector = 0;
      aff2.duration = 2 * Pulse::UPDATES_PER_MUDHOUR;
      aff2.modifier = 0;
      if (!(ch->isImmortal()))
        ch->affectTo(&aff);
      ch->affectTo(&aff2);
      ch->addToWait(combatRound(2));

      act("$n brandishes $p, shouting a strange <p>word of power<1>.", true, ch,
        o, nullptr, TO_ROOM, nullptr);
      act(
        "<r>A scorching wind swirls around $n<r>, and $s <R>$o<1><r> bursts "
        "into flame<1>.",
        true, ch, o, nullptr, TO_ROOM, nullptr);

      act("You brandish $p, shouting the command word, <p>incandesce<1>!", true,
        ch, o, nullptr, TO_CHAR, nullptr);
      act(
        "<r>A scorching wind swirls around you, and your <R>$o<1><r> bursts "
        "into flame<1>.",
        true, ch, o, nullptr, TO_CHAR, nullptr);
    } else if (buf == "evoke") {  // this is the activation keyword
      if (ch->checkForSkillAttempt(SPELL_CONJURE_AIR)) {
        act("The $o's power of lightning can only be used once a day!", true,
          ch, o, nullptr, TO_CHAR, nullptr);
        return true;
      } else if (ch->affectedBySpell(SPELL_CONJURE_FIRE)) {
        act(
          "The $o's power of lightning cannot be used at the same time as the "
          "power of fire!",
          true, ch, o, nullptr, TO_CHAR, nullptr);
        return true;
      } else if (ch->affectedBySpell(SPELL_CONJURE_WATER)) {
        act(
          "The $o's power of lightning cannot be used at the same time as the "
          "power of ice!",
          true, ch, o, nullptr, TO_CHAR, nullptr);
        return true;
      } else
        aff.type = AFFECT_SKILL_ATTEMPT;
      aff.level = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = 0;
      aff.duration = 24 * Pulse::UPDATES_PER_MUDHOUR;
      aff.modifier = SPELL_CONJURE_AIR;
      aff2.type = SPELL_CONJURE_AIR;
      aff2.level = 0;
      aff2.location = APPLY_NONE;
      aff2.bitvector = 0;
      aff2.duration = 2 * Pulse::UPDATES_PER_MUDHOUR;
      aff2.modifier = 0;
      if (!(ch->isImmortal()))
        ch->affectTo(&aff);
      ch->affectTo(&aff2);
      ch->addToWait(combatRound(2));

      act("$n brandishes $p, shouting a strange <p>word of power<1>.", true, ch,
        o, nullptr, TO_ROOM, nullptr);
      act(
        "<c>A charged wind swirls around $n<c>, and $s <C>$o<1><c> releases a "
        "shower of sparks.<1>",
        true, ch, o, nullptr, TO_ROOM, nullptr);

      act("You brandish $p, shouting the command word, <p>evoke<1>!", true, ch,
        o, nullptr, TO_CHAR, nullptr);
      act(
        "<c>A charged wind swirls around you, and your <C>$o<1><c> releases a "
        "shower of sparks.<1>",
        true, ch, o, nullptr, TO_CHAR, nullptr);
    } else
      return false;
    return true;
  }
  if (cmd == CMD_GENERIC_PULSE && ::number(1, 6) == 1) {
    if (ch->affectedBySpell(SPELL_CONJURE_WATER)) {
      act(
        "<b>A few ice crystals break off from $n<b>'s <B>$o<1><b> and drift to "
        "the ground.<1>",
        true, ch, o, nullptr, TO_ROOM, nullptr);
      act(
        "<b>A few ice crystals break off from your <B>$o<1><b> and drift to "
        "the ground.<1>",
        true, ch, o, nullptr, TO_CHAR, nullptr);
    } else if (ch->affectedBySpell(SPELL_CONJURE_FIRE)) {
      act(
        "<r>$n<r>'s <R>$o<1><r> flares up momentarily, releasing a blast of "
        "heat.<1><1>",
        true, ch, o, nullptr, TO_ROOM, nullptr);
      act(
        "<r>Your <R>$o<1><r> flares up momentarily, releasing a blast of "
        "heat.<1>",
        true, ch, o, nullptr, TO_CHAR, nullptr);
    } else if (ch->affectedBySpell(SPELL_CONJURE_AIR)) {
      act(
        "<c>$n<c>'s <C>$o<1><c> literally hums with power, releasing a few "
        "sparks into the air.<1><1>",
        true, ch, o, nullptr, TO_ROOM, nullptr);
      act(
        "<c>Your <C>$o<1><c> literally hums with power, releasing a few sparks "
        "into the air.<1>",
        true, ch, o, nullptr, TO_CHAR, nullptr);
    } else
      return false;
    return true;
  }
  return false;
}

int vorpal(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TThing* weap = dynamic_cast<TThing*>(o);
  int dam, rc = 0;
  wearSlotT part;
  spellNumT wtype;
  TBeing* ch;
  int crits[20] = {67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81,
    82, 91, 92, 98, 99};

  if (!(ch = genericWeaponProcCheck(vict, cmd, o, 50)))
    return false;

  part = vict->getPartHit(ch, true);
  dam = ch->getWeaponDam(vict, weap, HAND_PRIMARY);

  if (weap)
    wtype = ch->getAttackType(weap, HAND_PRIMARY);
  else
    wtype = TYPE_HIT;

  act("$p <r>begins glowing deep red!<1>", 0, vict, o, 0, TO_ROOM);

  if (!::number(0, 3)) {
    o->setMaxStructPoints(o->getMaxStructPoints() - 1);
    o->setStructPoints(o->getStructPoints() - 1);
  }

  rc = ch->critSuccessChance(vict, weap, &part, wtype, &dam,
    crits[::number(0, cElements(crits) - 1)]);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_VICT;
  } else if (!rc) {
    act("$p swings abruptly, but fails to hit anything.", 0, vict, o, 0,
      TO_ROOM);
    return false;
  }
  rc = ch->applyDamage(vict, dam, wtype);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_VICT;
  }

  if (o->getStructPoints() <= 0)
    return DELETE_ITEM;

  return false;
}

int berserkerWeap(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;
  affectedData af, aff;

  ch = genericWeaponProcCheck(vict, cmd, o, 5);
  if (!ch)
    return false;
  if (ch->hasClass(CLASS_WARRIOR) && ch->doesKnowSkill(SKILL_BERSERK)) {
    if (ch->isCombatMode(ATTACK_BERSERK) || !ch->isPc()) {
      if (!::number(0, 3) && !ch->affectedBySpell(SPELL_HASTE)) {
        act(
          "$p<1> glows with a <c>soft blue light<1>, and lends new energy to "
          "your attacks!",
          true, ch, o, vict, TO_CHAR, nullptr);
        act(
          "$p<1> glows with a <c>soft blue light<1>, and lends new energy to "
          "$n's attacks!",
          true, ch, o, vict, TO_ROOM, nullptr);
        aff.type = SPELL_HASTE;
        aff.level = 45;
        aff.duration =
          Pulse::ONE_SECOND *
          12;  // seconds are weird so this is a 1 min cast of haste
        aff.modifier = 0;
        aff.location = APPLY_NONE;
        aff.bitvector = 0;
        ch->affectTo(&aff, -1);
        act("$N has gained a bounce in $S step!", false, ch, nullptr, nullptr,
          TO_ROOM);
        act("You seem to be able to move with the greatest of ease!", false, ch,
          nullptr, nullptr, TO_CHAR);

        return true;
      }

      act(
        "<o>Your blood boils and you feel your wrath being amplified by $p.<1>",
        true, ch, o, vict, TO_CHAR, nullptr);
      dam = ch->getSkillValue(SKILL_BERSERK) / 5 + 2;
      if (ch->getRace() == RACE_OGRE)
        dam += 5;
      if (dam > 21) {
        act(
          "<r>Your $o releases a HUGE burst of concentrated fury upon "
          "$N<1><r>!<1>",
          true, ch, o, vict, TO_CHAR, nullptr);
        act(
          "<r>$n clenches $s teeth as $s $o releases a HUGE burst of fury upon "
          "$N.<1>",
          true, ch, o, vict, TO_NOTVICT, nullptr);
        act(
          "<o>$n clenches $s teeth as $s $o releases a HUGE burst of fury upon "
          "you!<1>",
          true, ch, o, vict, TO_VICT, nullptr);
      } else {
        act(
          "<r>Your $o releases a burst of concentrated fury upon $N<1><r>!<1>",
          true, ch, o, vict, TO_CHAR, nullptr);
        act(
          "<r>$n clenches $s teeth as $s $o releases a burst of fury upon "
          "$N.<1>",
          true, ch, o, vict, TO_NOTVICT, nullptr);
        act(
          "<o>$n clenches $s teeth as $s $o releases a burst of fury upon "
          "you!<1>",
          true, ch, o, vict, TO_VICT, nullptr);
      }

      rc = ch->reconcileDamage(vict, dam, DAMAGE_HACKED);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_VICT;
      return true;

    } else if (ch->isPc() && !::number(0, 10)) {
      ch->setCombatMode(ATTACK_BERSERK);

      act("<r>$p<r> amplifies your wrath, and you launch into a blood rage!<1>",
        true, ch, o, vict, TO_CHAR, nullptr);
      act(
        "<r>$p<r> amplifies $n's wrath, and $e launches into a blood rage!<1>",
        true, ch, o, vict, TO_ROOM, nullptr);

      act("You go berserk!", true, ch, 0, 0, TO_CHAR);
      act("$n goes berserk!", true, ch, 0, 0, TO_ROOM);

      if (ch->getHit() > (ch->hitLimit() / 2)) {
        af.type = SKILL_BERSERK;
        af.modifier = ::number(ch->getSkillValue(SKILL_BERSERK),
          ch->getSkillValue(SKILL_BERSERK) * 2);
        af.level = ch->GetMaxLevel();
        //      af.duration = ch->getSkillValue(SKILL_BERSERK);
        af.duration = PERMANENT_DURATION;
        af.location = APPLY_HIT;
        af.bitvector = 0;
        ch->affectTo(&af, -1);

        af.location = APPLY_CURRENT_HIT;
        ch->affectTo(&af, -1);

        ch->sendTo(
          "Berserking increases your ability to withstand damage!\n\r");
      }

      if (!ch->fight())
        ch->goBerserk(nullptr);
    }
  }

  return false;
}

int randomizer(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;
  int randomizer = ::number(0, 9);
  TGenWeapon* weapon = dynamic_cast<TGenWeapon*>(o);
  // Proc goes off like mad but damage is way minimal to produce the
  // affect of a blunt item slashing

  if (!(ch = genericWeaponProcCheck(vict, cmd, weapon, 0)))
    return false;

  switch (randomizer) {
    case 9:
      weapon->setWeaponType(WEAPON_TYPE_SLASH);
      break;
    case 8:
      weapon->setWeaponType(WEAPON_TYPE_CRUSH);
      break;
    case 7:
      weapon->setWeaponType(WEAPON_TYPE_BITE);
      break;
    case 6:
      weapon->setWeaponType(WEAPON_TYPE_THUMP);
      break;
    case 5:
      weapon->setWeaponType(WEAPON_TYPE_WHIP);
      break;
    case 4:
      weapon->setWeaponType(WEAPON_TYPE_STAB);
      break;
    case 3:
      weapon->setWeaponType(WEAPON_TYPE_CLEAVE);
      break;
    case 2:
      weapon->setWeaponType(WEAPON_TYPE_CLAW);
      break;
    case 1:
      weapon->setWeaponType(WEAPON_TYPE_SLICE);
      break;
    default:
      weapon->setWeaponType(WEAPON_TYPE_SPEAR);
      break;
  }
  return true;
}

int bluntPierce(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;
  int randomizer = ::number(0, 9);
  TGenWeapon* weapon = dynamic_cast<TGenWeapon*>(o);
  // Proc goes off like mad but damage is way minimal to produce the
  // affect of a blunt item slashing

  if (!(ch = genericWeaponProcCheck(vict, cmd, weapon, 0)))
    return false;

  if (randomizer >= 5) {
    weapon->setWeaponType(WEAPON_TYPE_PIERCE);
  } else {
    weapon->setWeaponType(WEAPON_TYPE_SMITE);
  }
  return true;
}

int dualStyleWeapon(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o,
  TObj*) {
  return false;  // buggy

  // this code is for weapons with more than one damage type
  // it utilizes two 'styles' that can be changed with the switch <weapon>
  // command to favor one or the other damage types dash - may 2001

  TGenWeapon* weap = dynamic_cast<TGenWeapon*>(o);
  if (!weap)
    return false;

  class spectype_struct {
    public:
      weaponT norm;
      weaponT type1;
      weaponT type2;
      int vnum;

      spectype_struct() {}
      ~spectype_struct() {}
  };

  spectype_struct* weapspec = nullptr;
  if (cmd == CMD_GENERIC_CREATED ||
      !(weapspec = static_cast<spectype_struct*>(o->act_ptr))) {
    o->act_ptr = new spectype_struct();
    vlogf(LOG_PROC,
      format("obj (%s) with dualstyle proc ... attempting to alocate.") %
        o->getName());
    if (!(weapspec = static_cast<spectype_struct*>(o->act_ptr))) {
      vlogf(LOG_PROC,
        format(
          "obj (%s) with dualstyle proc had no memory alocated, investigate.") %
          o->getName());
      return false;
    }
    weapspec->type1 = weap->getWeaponType();
    weapspec->norm = weapspec->type1;
    weapspec->vnum = obj_index[o->getItemIndex()].virt;
    switch (weapspec->vnum) {  // this proc is versatile - add more
                               // vnums/secondary damage types
      case 9595:               // here to make it work with another weapon
        weapspec->type2 = WEAPON_TYPE_SMITE;  // hammerblade - dash 05/01
      default:
        weapspec->type2 = WEAPON_TYPE_SMASH;
    }
    return false;
  }

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<spectype_struct*>(o->act_ptr);
    o->act_ptr = nullptr;
    return false;
  }

  TBeing* ch;
  char parg[30];

  if (!(ch = genericWeaponProcCheck(vict, CMD_OBJ_HIT, weap, 0)))
    return false;

  if (cmd != CMD_SWITCH && cmd != CMD_OBJ_HIT) {
    if (cmd != CMD_OBJ_HITTING && cmd != CMD_OBJ_MISS)
      weap->setWeaponType(weapspec->type1);
    return false;
  }

  if (cmd == CMD_SWITCH) {
    arg = one_argument(arg, parg, cElements(parg));
    isname(parg, weap->getName());
    act(
      "<c>You deftly change your grip on $p<c> to use it in a different "
      "style!<1>",
      true, ch, o, vict, TO_CHAR, nullptr);
    act("<c>$n deftly changes $s grip on $p to use it in a different style!<1>",
      true, ch, o, vict, TO_ROOM, nullptr);
    if (weapspec->type1 == weapspec->norm) {
      weapspec->type1 = weapspec->type2;
      weapspec->type2 = weapspec->norm;
    } else {
      weapspec->type2 = weapspec->type1;
      weapspec->type1 = weapspec->norm;
    }
    return true;
  }

  if (::number(0, 3)) {  // 3/4 is type 1, 1/4 is type two
    weap->setWeaponType(weapspec->type1);
  } else {
    weap->setWeaponType(weapspec->type2);
  }
  return true;
}

int splinteredClub(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o,
  TObj*) {
  TBeing* ch;
  int rc, dam;
  TObj* obj;
  wearSlotT slot;
  char buf[256];

  if (!(ch = genericWeaponProcCheck(vict, cmd, o, 3)))
    return false;

  slot = pickRandomLimb();

  if (!vict->slotChance(wearSlotT(slot)) || vict->getStuckIn(wearSlotT(slot)) ||
      notBleedSlot(slot))
    return false;

  obj = read_object(31349, VIRTUAL);
  dam = ::number(3, 12);

  sprintf(buf,
    "<o>A splinter from <1>$p<o> breaks off and embeds in <1>$n<o>'s "
    "<1>%s<o>.<1>",
    vict->describeBodySlot(slot).c_str());
  act(buf, 0, vict, o, 0, TO_ROOM);
  sprintf(buf,
    "<o>A splinter from <1>$p<o> breaks off and embeds in your <1>%s<o>.<1>",
    vict->describeBodySlot(slot).c_str());
  act(buf, 0, vict, o, 0, TO_CHAR);

  vict->stickIn(obj, wearSlotT(slot));

  if (!vict->isImmune(IMMUNE_BLEED, slot)) {
    vict->rawBleed(slot, 250, SILENT_YES, CHECK_IMMUNITY_NO);
    vict->rawInfect(slot, 250, SILENT_YES, CHECK_IMMUNITY_NO);

    sprintf(buf,
      "<R>Blood<1> <o>drips out of the wound created by the splinter.<1>");
    act(buf, 0, vict, o, 0, TO_ROOM);
    sprintf(buf,
      "<R>Blood<1> <o>drips out of the wound created by a large splinter.<1>");
    act(buf, 0, vict, o, 0, TO_CHAR);
  }

  rc = ch->reconcileDamage(vict, dam, TYPE_STAB);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return true;
}

int sonicBlast(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc;

  ch = genericWeaponProcCheck(vict, cmd, o, 6);
  if (!ch)
    return false;

  act("\a$p <G>sends an earpiercing blast of sound at<z> $n<G>.<z>", false,
    vict, o, nullptr, TO_ROOM);
  act("\a$p <G>blasts your eardrums with an intense sound!<z>", false, vict, o,
    nullptr, TO_CHAR);
  // how handy to have an annoying sound file to do this with
  vict->roomp->playsound(SOUND_SPELL_ARMOR, SOUND_TYPE_MAGIC);

  int dam = ::number(5, 8);
  rc = ch->reconcileDamage(vict, dam, DAMAGE_NORMAL);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  return true;
}

int frostSpear(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;

  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (ch) {
    dam = ::number(4, 10);
    if (dam < 8) {
      act("$p becomes covered with ice and freezes $n.", 0, vict, o, 0, TO_ROOM,
        ANSI_CYAN);
      act("$p becomes covered with ice and freezes you.", 0, vict, o, 0,
        TO_CHAR, ANSI_CYAN);
    } else {
      act("$p becomes covered with ice and sends a violent chill through $n.",
        0, vict, o, 0, TO_ROOM, ANSI_BLUE);
      act("$p becomes covered with ice and sends a violent chill through you.",
        0, vict, o, 0, TO_CHAR, ANSI_BLUE);
    }

    rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
    return true;
  }

  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    sstring buf, buf2;
    TBeing* vict = nullptr;
    buf = sstring(arg).word(0);
    buf2 = sstring(arg).word(1);

    if (buf == "chill" && buf2 == "out") {
      if (ch->checkObjUsed(o)) {
        act("You cannot use $p's powers again this soon.", true, ch, o, nullptr,
          TO_CHAR, nullptr);
        return false;
      }

      if (!(vict = ch->fight())) {
        act("You cannot use $p's powers unless you are fighting.", true, ch, o,
          nullptr, TO_CHAR, nullptr);
        return false;
      }

      ch->addObjUsed(o, Pulse::UPDATES_PER_MUDHOUR);

      act(
        "The point of $n's $o glows <b>a cold blue<1> as $e growls a <p>word "
        "of power<1>.",
        true, ch, o, nullptr, TO_ROOM, nullptr);
      act("$n steps back and points $p at $N!<1>", true, ch, o, vict,
        TO_NOTVICT, nullptr);
      act(
        "<c>An incredibly cold ray erupts from $n's <b>$o<c>, and strikes $N "
        "full on!<1>",
        true, ch, o, vict, TO_NOTVICT, nullptr);
      act("$n steps back and points $p at you!  Uh oh!<1>", true, ch, o, vict,
        TO_VICT, nullptr);
      act(
        "<c>An incredibly cold ray erupts from $n's <b>$o<c>, and strikes you "
        "full on!<1>",
        true, ch, o, vict, TO_VICT, nullptr);
      act(
        "The point of your $o glows <b>a cold blue<1> as you growls, '<p>chill "
        "out<1>'.",
        true, ch, o, nullptr, TO_CHAR, nullptr);
      act("You step back and point $p at $N!<1>", true, ch, o, vict, TO_CHAR,
        nullptr);
      act(
        "<c>An incredibly cold ray erupts from your <b>$o<c>, and strikes $N "
        "full on!<1>",
        true, ch, o, vict, TO_CHAR, nullptr);

      int dam = ::number(10, 60);
      rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        vict->reformGroup();
        delete vict;
        vict = nullptr;
      }

      return true;
    }
  }
  return false;
}

int weaponUnmaker(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  TObj* clay;

  sstring buf;

  if (!o || !vict)
    return false;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)
  if (::number(0, 49))
    return false;
  if (cmd != CMD_OBJ_HITTING)
    return false;

  if (!ch->canWither(vict, SILENT_YES))
    return false;

  wearSlotT slot;

  bool ok = false;
  bool found = false;

  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (notBreakSlot(slot, false))  // same ones, right?
      continue;
    if (!vict->slotChance(slot))
      continue;
    found |= (vict->isLimbFlags(slot, PART_MISSING));
    ok = true;
  }

  if (found || !ok)
    return false;

  for (slot = pickRandomLimb();; slot = pickRandomLimb()) {
    if (notBreakSlot(slot, true))
      continue;
    if (!vict->slotChance(slot))
      continue;
    break;
  }

  if ((clay = read_object(14, VIRTUAL))) {
    *ch->roomp += *clay;
  }

  char limb[80];
  sprintf(limb, "%s", vict->describeBodySlot(slot).c_str());

  // snipped limb missing code

  TThing* t;

  if (!vict->hasPart(slot)) {
    vlogf(LOG_COMBAT,
      format("BOGUS SLOT trying to be made PART_MISSING: %d on %s") % slot %
        vict->getName());
    return false;
  }
  if (!vict->roomp) {
    // bat 8-16-96, mob could be dead, this is a bug
    vlogf(LOG_COMBAT,
      format("!roomp for target (%s) of makePartMissing().") % vict->getName());
    return false;
  }

  vict->setLimbFlags(slot, PART_MISSING);

  if ((t = vict->unequip(slot)))
    *(vict->roomp) += *t;

  for (wearSlotT j = MIN_WEAR; j < MAX_WEAR; j++) {
    if (!vict->hasPart(j))
      continue;
    if (!vict->isBodyPartAttached(j)) {
      vict->setLimbFlags(j, PART_MISSING);
      TThing* tmp = vict->unequip(j);
      if (tmp)
        *(vict->roomp) += *tmp;
    }
  }

  // check for damage to both hands
  vict->dropItemFromDamagedHand(true);
  vict->dropItemFromDamagedHand(false);

  // snipped limb missing code

  buf =
    format("$n's $o glows with a <g>sickly light<1> as it strikes your %s!") %
    limb;
  act(buf, false, ch, o, vict, TO_VICT, nullptr);
  buf = format(
          "Your %s turns to <o>soft clay<1> and falls to the ground!\n\rYou "
          "look down at your missing %s and scream!") %
        limb % limb;
  act(buf, false, vict, nullptr, nullptr, TO_CHAR, nullptr);

  buf =
    format("Your $o glows with a <g>sickly light<1> as it strikes $N's %s!") %
    limb;
  act(buf, false, ch, o, vict, TO_CHAR, nullptr);
  buf =
    format("$n's $o glows with a <g>sickly light<1> as it strikes $N's %s!") %
    limb;
  act(buf, false, ch, o, vict, TO_NOTVICT, nullptr);

  buf = format(
          "$N looks down in terror as $S %s turns to <o>soft clay<1> before $S "
          "eyes!\n\r<o>A lump of clay falls to the ground.<1>") %
        limb;
  act(buf, false, vict, nullptr, vict, TO_ROOM, nullptr);

  vict->dropWeapon(slot);

  return true;
}

int chromaticWeapon(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 5);
  if (!ch)
    return false;

  spellNumT DamType = DAMAGE_NORMAL;

  char txt1[256], txt2[256];

  switch (::number(0, 4)) {
    case 0:
      sprintf(txt1, "<b>$p<b> becomes covered with ice and freezes $n.<1>");
      sprintf(txt2, "<b>$p<b> becomes covered with ice and freezes you.<1>");
      DamType = DAMAGE_FROST;
      break;
    case 1:
      sprintf(txt1, "<k>$p<k> turns into solid rock as it hits $n.<1>");
      sprintf(txt2, "<k>$p<k> turns into solid rock as it hits you.<1>");
      DamType = TYPE_CRUSH;
      break;
    case 2:
      sprintf(txt1, "<r>$p<r> bursts into flame and sorches $n.<1>");
      sprintf(txt2, "<r>$p<r> bursts into flame and sorches you.<1>");
      DamType = DAMAGE_FIRE;
      break;
    case 3:
      sprintf(txt1, "<c>$p<c> crackles with electricity as it shocks $n.<1>");
      sprintf(txt2, "<c>$p<c> crackles with electricity as it shocks you.<1>");
      DamType = DAMAGE_ELECTRIC;
      break;
    case 4:
    default:
      sprintf(txt1, "<g>$p<g> becomes covered with acid as it burns $n.<1>");
      sprintf(txt2, "<g>$p<g> becomes covered with acid as it burns you.<1>");
      DamType = DAMAGE_ACID;
      break;
  }

  dam = ::number(4, 10);

  act(txt1, 0, vict, o, 0, TO_ROOM, ANSI_CYAN);
  act(txt2, 0, vict, o, 0, TO_CHAR, ANSI_CYAN);

  rc = ch->reconcileDamage(vict, dam, DamType);
  if (rc == -1)
    return DELETE_VICT;
  return true;
}

int fireballWeapon(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing *ch, *temp, *tmp_victim;
  TRoom* rp;
  int damage;
  int chance, rc;
  bool vict_alive = true;

  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (!ch)
    return false;

  rp = ch->roomp;
  if (!rp) {
    vlogf(LOG_BUG,
      "Returned nullptr pointer to room in fireballWeapon.  Exiting.");
    return false;
  }

  chance = ::number(4, 10);  // .33 * .3 = 10% chance the proc will do damage

  if (chance < 8 || (rp && rp->isUnderwaterSector())) {
    act("$n's $p glows fire red.", 0, ch, o, 0, TO_ROOM, ANSI_RED);
    act("Your $p glows fire red.", 0, ch, o, 0, TO_CHAR, ANSI_RED);
  } else {
    act("$n's $p glows fire red and shoots out a ball of fire!.", 0, ch, o, 0,
      TO_ROOM, ANSI_RED_BOLD);
    act("Your $p glows fire red and shoots out a ball of fire!", 0, ch, o, 0,
      TO_CHAR, ANSI_RED_BOLD);

    damage = ::number(1, 19);  // +10 average damage for +1 average per hit
    ch->flameRoom();  // this can kill and delete the victim.... need to make
                      // sure the victim is still around later in the proc

    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next;
      if (ch->sameRoom(*tmp_victim) && (ch != tmp_victim)) {
        if (!ch->inGroup(*tmp_victim) && !tmp_victim->isImmortal()) {
          if (tmp_victim->isLucky(ch->spellLuckModifier(SPELL_FIREBALL))) {
            act("$N is able to dodge part of the explosion!", false, ch, nullptr,
              tmp_victim, TO_CHAR);
            act("$N is able to dodge part of the explosion!", false, ch, nullptr,
              tmp_victim, TO_NOTVICT);
            act("You are able to dodge part of the explosion!", false, ch, nullptr,
              tmp_victim, TO_VICT);
            damage >>= 1;
          } else {
            act("$N had no hope of dodging the lashing flames!", false, ch,
              nullptr, tmp_victim, TO_CHAR);
            act("$N had no hope of dodging the lashing flames!", false, ch,
              nullptr, tmp_victim, TO_NOTVICT);
            act("You had no hope of dodging the lashing flames!", false, ch,
              nullptr, tmp_victim, TO_VICT);
          }
          if (ch->reconcileDamage(tmp_victim, damage, SPELL_FIREBALL) == -1) {
            if (vict && tmp_victim == vict) {
              vict_alive = false;
              continue;
            } else {
              delete tmp_victim;
              tmp_victim = nullptr;
              continue;
            }
          }
          rc = tmp_victim->flameEngulfed();
          rc = 0;
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            if (vict && tmp_victim == vict) {
              vict_alive = false;
              continue;
            } else {
              delete tmp_victim;
              tmp_victim = nullptr;
              continue;
            }
          }
        } else
          act("You are able to avoid the flames!", false, ch, nullptr, tmp_victim,
            TO_VICT);
      } else if ((ch != tmp_victim) && (tmp_victim->in_room != Room::NOWHERE) &&
                 (rp->getZoneNum() == tmp_victim->roomp->getZoneNum())) {
        if (tmp_victim->awake())
          tmp_victim->sendTo(
            "You hear a loud explosion and feel a gust of hot air.\n\r");
      }
    }  // end for loop cycle through character list

  }  // end weapon damaging effect
  if (!vict_alive)
    return DELETE_VICT;
  return true;
}

int poisonViperBlade(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  affectedData aff, aff2;

  if (!o || !vict)
    return false;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)
  if (cmd != CMD_OBJ_HITTING)
    return false;
  if (vict->isImmune(IMMUNE_POISON, WEAR_BODY))
    return false;
  if (vict->affectedBySpell(SPELL_POISON))
    return false;

  if (!::number(0, 36)) {
    act("<G>A strange green mist emanates from<1> $p.", false, o, nullptr,
      nullptr, TO_ROOM);
    act(
      "<G>The green mist swiftly forms into the shape of a venomous viper.<1>",
      false, o, nullptr, nullptr, TO_ROOM);
    act("<G>The viper strikes $N in a flash and just as quickly disappears!<1>",
      false, o, nullptr, vict, TO_ROOM);

    aff.type = SPELL_POISON;
    aff.level = 15;
    aff.duration = (25) * Pulse::UPDATES_PER_MUDHOUR;
    aff.modifier = -25;
    aff.location = APPLY_STR;
    aff.bitvector = AFF_POISON;

    aff2.type = AFFECT_DISEASE;
    aff2.level = 0;
    aff2.duration = aff.duration;
    aff2.modifier = DISEASE_POISON;
    aff2.location = APPLY_NONE;
    aff2.bitvector = AFF_POISON;

    vict->affectTo(&aff);
    vict->affectTo(&aff2);
    disease_start(vict, &aff2);

    return true;
  }
  return false;
}

int energyBlade(TBeing* vict, cmdTypeT cmd, const char*, TObj* obj, TObj*) {
  TBeing* ch = genericWeaponProcCheck(vict, cmd, obj, 8);
  if (!ch)
    return false;

  act(
    "$p <W>s<1>p<W>a<1>r<W>k<1>l<W>e<1>s and flashes with a <W>blinding white "
    "light<1>.",
    false, obj, nullptr, nullptr, TO_ROOM);
  act(
    "With a deafening crackle, a <W>jagged energy bolt<1> leaps from $n to $N!",
    false, obj, nullptr, vict, TO_ROOM);

  int rc = ch->reconcileDamage(vict, ::number(4, 10), DAMAGE_ELECTRIC);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return true;
}

int scirenDrown(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 8);
  if (!ch)
    return false;

  if (vict->affectedBySpell(SPELL_SUFFOCATE))
    return false;

  dam = ::number(4, 10);
  act(
    "$p <1><B>pulsates with a glowing blue hue<1>.\n\r$p <1><B>emits a stream "
    "of salty water directed at $n's mouth<1>!!",
    0, vict, o, 0, TO_ROOM);
  act(
    "$p <1><B>pulsates with a glowing blue hue<1>.\n\r$p <1><B>emits a stream "
    "of salty water directed at your mouth<1>!!",
    0, vict, o, 0, TO_CHAR);

  // makes sense since were shooting salt water at the victims mouth
  // may as well have a puddle -jh
  ch->dropPool(5, LIQ_SALTWATER);

  affectedData aff;
  aff.type = SPELL_SUFFOCATE;
  aff.level = 20;
  aff.duration = 3;  // shortlived spell affect -jh
  aff.modifier = DISEASE_SUFFOCATE;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_SILENT;

  rc = ch->applyDamage(vict, dam, DAMAGE_SUFFOCATION);
  vict->affectJoin(vict, &aff, AVG_DUR_NO, AVG_EFF_YES);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return true;
}

int poisonSap(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  affectedData aff, aff2;

  if (!o || !vict)
    return false;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)
  if (cmd != CMD_OBJ_HITTING)
    return false;
  if (vict->isImmune(IMMUNE_POISON, WEAR_BODY))
    return false;
  if (vict->affectedBySpell(SPELL_POISON))
    return false;

  if (!::number(0, 49)) {
    aff.type = SPELL_POISON;
    aff.level = 10;
    aff.duration = (20) * Pulse::UPDATES_PER_MUDHOUR;
    aff.modifier = -20;
    aff.location = APPLY_STR;
    aff.bitvector = AFF_POISON;

    aff2.type = AFFECT_DISEASE;
    aff2.level = 0;
    aff2.duration = aff.duration;
    aff2.modifier = DISEASE_POISON;
    aff2.location = APPLY_NONE;
    aff2.bitvector = AFF_POISON;

    act("Fluid from $p mixes with your blood, causing a burning sensation.", 0,
      vict, o, 0, TO_CHAR, ANSI_GREEN);
    act("Fluid from $p mixes with $n's blood, causing $m to grimace in pain.",
      0, vict, o, 0, TO_ROOM, ANSI_GREEN);
    vict->affectTo(&aff);
    vict->affectTo(&aff2);
    disease_start(vict, &aff2);

    return true;
  }
  return false;
}

int wickedDagger(TBeing* vict, cmdTypeT cmd, const char*, TObj* me,
  TObj* ch_obj) {
  int dam = ::number(1, 10);
  spellNumT wtype = me->getWtype();

  if (!(cmd == CMD_OBJ_MISS || cmd == CMD_OBJ_HIT ||
        cmd == CMD_GENERIC_PULSE)) {
    return false;
  }

  if (::number(0, 10) || !ch_obj || !vict || vict->getHit() <= dam ||
      (dynamic_cast<TBeing*>(dynamic_cast<TThing*>(ch_obj)))->getHit() <= dam)
    return false;

  if (cmd == CMD_GENERIC_PULSE && !::number(0, 5) && vict->roomp) {
    sendrpf(COLOR_OBJECTS, vict->roomp,
      "%s<k> sheds a light of iniquity.<z>\n\r",
      (!me->getName().empty() ? sstring(me->getName()).cap().c_str()
                              : "Bogus Object"));
  }

  if (cmd == CMD_OBJ_MISS) {
    // victim = vict
    // swinger = ch_obj as TBeing, so must cast back to being
    TThing* ch_thing = ch_obj;
    TBeing* ch = dynamic_cast<TBeing*>(ch_thing);

    ch->sendTo("You feel the life within you slowly ebb away.\n\r");

    // missing does dam to swinger
    int rc = ch->reconcileDamage(ch, dam, wtype);
    if (rc == -1)
      return DELETE_VICT;

    return false;
  } else if (cmd == CMD_OBJ_HIT) {
    // victim = vict
    // hitting does extra dam to victim

    // we can safely use equippedBy since ch takes no damage
    TBeing* ch = dynamic_cast<TBeing*>(me->equippedBy);

    vict->sendTo("You feel the life within you slowly ebb away.\n\r");

    int rc = ch->reconcileDamage(vict, dam, wtype);
    if (rc == -1)
      return DELETE_VICT;

    return false;
  }
  return false;
}

int bloodspike(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;
  TObj* obj;
  wearSlotT slot;
  char buf[256];

  if (!(ch = genericWeaponProcCheck(vict, cmd, o, 3)))
    return false;

  slot = pickRandomLimb();

  if (!vict->slotChance(wearSlotT(slot)) || vict->getStuckIn(wearSlotT(slot)) ||
      notBleedSlot(slot))
    return false;

  obj = read_object(13713, VIRTUAL);
  dam = ::number(3, 12);

  sprintf(buf,
    "<k>There is a sharp crack as one of <1>$p<k>'s spikes breaks off while "
    "embedded in <1>$n<k>'s <1>%s<k>.<1>",
    vict->describeBodySlot(slot).c_str());
  act(buf, 0, vict, o, 0, TO_ROOM);
  sprintf(buf,
    "<k>There is a sharp crack as one of <1>$p<k>'s spikes breaks off while "
    "embedded in your <1>%s<k>.<1>",
    vict->describeBodySlot(slot).c_str());
  act(buf, 0, vict, o, 0, TO_CHAR);

  vict->stickIn(obj, wearSlotT(slot));

  if (!vict->isImmune(IMMUNE_BLEED, slot)) {
    vict->rawBleed(slot, 250, SILENT_YES, CHECK_IMMUNITY_NO);
    vict->rawInfect(slot, 250, SILENT_YES, CHECK_IMMUNITY_NO);

    sprintf(buf,
      "<r>Blood<k> squirts through the hollow spike uncontrollably!<1>");
    act(buf, 0, vict, o, 0, TO_ROOM);
    sprintf(buf,
      "<r>Blood<k> squirts through the hollow spike uncontrollably!<1>");
    act(buf, 0, vict, o, 0, TO_CHAR);
  }

  rc = ch->reconcileDamage(vict, dam, TYPE_STAB);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return true;
}

int brokenBottle(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;
  TObj* obj;
  wearSlotT slot;
  sstring buf;

  if (!(ch = genericWeaponProcCheck(vict, cmd, o, 3)))
    return false;

  o->setMaxStructPoints(o->getMaxStructPoints() - 1);
  o->setStructPoints(o->getStructPoints() - 1);

  slot = pickRandomLimb();

  if (!vict->slotChance(wearSlotT(slot)) || vict->getStuckIn(wearSlotT(slot)) ||
      notBleedSlot(slot))
    return false;

  obj = read_object(33468, VIRTUAL);
  dam = ::number(3, 12);

  buf = format(
          "$p cracks from the impact and %s breaks off while embedded in $n's "
          "%s.") %
        obj->getName() % vict->describeBodySlot(slot);
  act(buf, 0, vict, o, 0, TO_ROOM);
  buf = format(
          "$p cracks from the impact and %s breaks off while embedded in your "
          "%s.") %
        obj->getName() % vict->describeBodySlot(slot);
  act(buf, 0, vict, o, 0, TO_CHAR);

  vict->stickIn(obj, wearSlotT(slot));

  rc = ch->reconcileDamage(vict, dam, TYPE_STAB);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return true;
}

int boneStaff(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;

  // Add logic for if obj has been hit. Rather than creating a
  // new proc this one can be used in different ways
  if (cmd == CMD_OBJ_BEEN_HIT) {
    int rc, dam;
    if (!vict || !o)
      return false;

    if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
      return false;

    if (::number(0, 2))
      return false;

    act("$p draws your life force through it into $N.", true, vict, o, ch,
      TO_CHAR, ANSI_GREEN_BOLD);
    act("$p draws the life force of $n through it into you.", true, vict, o, ch,
      TO_VICT, ANSI_GREEN_BOLD);
    act("$p draws the life force of $n through it into $N.", true, vict, o, ch,
      TO_NOTVICT, ANSI_GREEN_BOLD);

    dam = ::number(1, 2);

    ch->addToHit(dam);
    ch->addToLifeforce(dam);
    rc = ch->reconcileDamage(vict, dam, DAMAGE_NORMAL);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
    return true;
  }

  wearSlotT part_hit = wearSlotT((long int)arg);

  ch = genericWeaponProcCheck(vict, cmd, o, 0);
  if (!ch)
    return false;

  // if we hit body, head, or neck, suck some life into user
  // we've already "hit" them, so life from vict has already been taken
  if (part_hit != WEAR_BODY && part_hit != WEAR_NECK && part_hit != WEAR_HEAD)
    return false;

  int amount = min(1, ch->hitLimit() - ch->getHit());
  if (amount) {
    act("$p draws your life force through it into $N.", true, vict, o, ch,
      TO_CHAR, ANSI_GREEN_BOLD);
    act("$p draws the life force of $n through it into you.", true, vict, o, ch,
      TO_VICT, ANSI_GREEN_BOLD);
    act("$p draws the life force of $n through it into $N.", true, vict, o, ch,
      TO_NOTVICT, ANSI_GREEN_BOLD);
    ch->addToHit(amount);
    ch->addToLifeforce(amount);
  }

  return true;
}

int dragonSlayer(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (!ch)
    return false;
  if (vict->getRace() != RACE_DRAGON)
    return false;

  dam = ::number(1, ch->GetMaxLevel());
  act("$p hums with power and slams into $n seemingly of its own accord!", 0,
    vict, o, 0, TO_ROOM, ANSI_WHITE_BOLD);
  act("$p hums with power and slams into you seemingly of its own accord!", 0,
    vict, o, 0, TO_CHAR, ANSI_WHITE_BOLD);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_NORMAL);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return true;
}

int frostWeapon(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (!ch)
    return false;

  dam = ::number(4, 10);
  if (dam < 8) {
    act("$p becomes covered with ice and freezes $n.", 0, vict, o, 0, TO_ROOM,
      ANSI_CYAN);
    act("$p becomes covered with ice and freezes you.", 0, vict, o, 0, TO_CHAR,
      ANSI_CYAN);
  } else {
    act("$p becomes covered with ice and sends a violent chill through $n.", 0,
      vict, o, 0, TO_ROOM, ANSI_BLUE);
    act("$p becomes covered with ice and sends a violent chill through you.", 0,
      vict, o, 0, TO_CHAR, ANSI_BLUE);
  }

  rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
  if (rc == -1)
    return DELETE_VICT;
  return true;
}

int flameWeapon(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (!ch)
    return false;

  dam = ::number(4, 10);
  if (dam < 8) {
    act("$p erupts into roaring flames and sears $n.", 0, vict, o, 0, TO_ROOM,
      ANSI_ORANGE);
    act("$p erupts into roaring flames and sears you.", 0, vict, o, 0, TO_CHAR,
      ANSI_ORANGE);
  } else {
    act("$p roars into a blaze of fire and scorches $n.", 0, vict, o, 0,
      TO_ROOM, ANSI_ORANGE_BOLD);
    act("$p roars into a blaze of fire and scorches you.", 0, vict, o, 0,
      TO_CHAR, ANSI_ORANGE_BOLD);
  }

  rc = ch->reconcileDamage(vict, dam, DAMAGE_FIRE);
  if (rc == -1)
    return DELETE_VICT;
  return true;
}

int daggerOfHunting(TBeing* ch, cmdTypeT cmd, const char* arg, TObj* me,
  TObj*) {
  sstring objbuf, targbuf;
  TBeing* target;
  sstring buf;
  TRoom* rp;
  int rc;
  wearSlotT phit;
  int dam;

  if (cmd != CMD_THROW && cmd != CMD_REMOVE && cmd != CMD_OBJ_EXPELLED)
    return false;

  if (cmd == CMD_REMOVE) {
    objbuf = sstring(arg).word(0);

    if (objbuf.empty() || !isname(objbuf, me->getName())) {
      return false;
    }

    if (me->stuckIn != ch && me->equippedBy != ch) {
      ch->sendTo("That item is nowhere on your body!\n\r");
      return false;
    }
    rc = ch->doRemove("", me);
    act("$p mystically dissolves.", false, ch, me, 0, TO_ROOM);
    act("$p mystically dissolves.", false, ch, me, 0, TO_CHAR);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT | DELETE_THIS;
    return DELETE_THIS;
  } else if (cmd == CMD_OBJ_EXPELLED) {
    // the object is expelled
    // possibly it is now on ground (expel spell), or on ch (hospital)
    act("$p mystically dissolves.", false, ch, me, 0, TO_ROOM);
    act("$p mystically dissolves.", false, ch, me, 0, TO_CHAR);
    return DELETE_THIS;
  }

  objbuf = sstring(arg).word(0);
  targbuf = sstring(arg).word(1);

  if (objbuf.empty() || !isname(objbuf, me->getName()) || targbuf.empty())
    return false;

  if (me->equippedBy)
    dynamic_cast<TBeing*>(me->equippedBy)->unequip(me->eq_pos);
  else if (me->parent)
    --(*me);
  else
    return false;

  if (me->isObjStat(ITEM_PROTOTYPE) && !ch->isImmortal()) {
    // oooh bad!
    // possible to do since don't need to actually hold to throw
    // bypassing the prototype acheck

    // cause it to target the thrower  :)
    ch->sendTo("A malicious force intervenes.\n\r");
    targbuf = ch->getName();
  }

  act("With a mighty heave, you toss $p straight up.", false, ch, me, 0,
    TO_CHAR);
  act("With a mighty heave, $n tosses $p straight up.", false, ch, me, 0,
    TO_ROOM);
  if (!(target = get_char_vis(ch, targbuf, nullptr))) {
    *(ch->roomp) += *me;
    act("$n falls to the $g.", true, me, 0, 0, TO_ROOM);
    return true;
  }
  *(ch->roomp) += *me;

  // OK, so I'm an evil evil bastard, so sue me   :)
  TTrap* ttr = dynamic_cast<TTrap*>(me);
  if (ttr) {
    // the infamous Batopr's Grenade of Hunting!!!
    // i'm ignoring sanity checks for insuring its a grenade, oh well
    ttr->armGrenade(ch);
  }

  act("A few feet up, $n stops its upward trajectory suddenly.", true, me, 0, 0,
    TO_ROOM);
  act("$n rotates quickly as if seeking something.", true, me, 0, 0, TO_ROOM);

  while (!me->sameRoom(*target)) {
    TPathFinder path;

    dirTypeT dir = path.findPath(me->in_room, findRoom(target->in_room));
    if (dir < 0) {
      act("$n falls to the $g.", true, me, 0, 0, TO_ROOM);
      act("$p fails to find its target.", false, ch, me, 0, TO_CHAR);
      act("$p fails to find its target.", false, ch, me, 0, TO_ROOM);
      ch->sendTo(format("Unable to find path.  dir=%d\n\r") % dir);
      return true;
    }
    buf = format("With blinding speed, $n streaks out of the room %s.") %
          dirs_to_blank[dir];
    act(buf, true, me, 0, 0, TO_ROOM);

    if (!(rp = real_roomp(me->exitDir(dir)->to_room))) {
      return true;
    }
    --(*me);
    *rp += *me;

    buf = format("With blinding speed, $n streaks into the room from the %s.") %
          dirs[rev_dir(dir)];
    act(buf, true, me, 0, 0, TO_ROOM);
  }

  dam = 0;  // double purpose
  do {
    dam++;
    phit = target->getPartHit(nullptr, false);
  } while (target->getStuckIn(phit) && (dam < MAX_WEAR));

  if (dam >= MAX_WEAR) {
    // too much already impaled
    act("$p puffs into inconsequential smoke.", true, ch, me, 0, TO_CHAR);
    act("$n puffs into inconsequential smoke.", true, me, 0, 0, TO_ROOM);
    return DELETE_THIS;  // delete me
  }

  buf =
    format("$p impales itself into $n's %s.") % target->describeBodySlot(phit);
  act(buf, true, target, me, ch, TO_VICT);
  //  act(buf, true, target, me, nullptr, TO_ROOM);
  act(buf, true, target, me, ch, TO_NOTVICT);
  buf =
    format("$p impales itself into your %s.") % target->describeBodySlot(phit);
  act(buf, true, target, me, nullptr, TO_CHAR);

  --(*me);
  rc = target->stickIn(me, phit);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete target;
    target = nullptr;
    return true;
  }
  TBaseWeapon* tbw = dynamic_cast<TBaseWeapon*>(me);
  if (tbw) {
    dam = (int)(tbw->baseDamage());
    rc = ch->applyDamage(target, dam, tbw->getWtype());
    buf = format("You do %d damage to $M.") % dam;
    act(buf, true, ch, 0, target, TO_CHAR);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete target;
      target = nullptr;
    }
  }
  return true;
}

int poisonWhip(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  affectedData aff, aff2;

  if (!o || !vict)
    return false;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)
  if (cmd != CMD_OBJ_HITTING)
    return false;
  if (vict->isImmune(IMMUNE_POISON, WEAR_BODY))
    return false;
  if (vict->affectedBySpell(SPELL_POISON))
    return false;

  if (!::number(0, 49)) {
    aff.type = SPELL_POISON;
    aff.level = 10;
    aff.duration = (20) * Pulse::UPDATES_PER_MUDHOUR;
    aff.modifier = -20;
    aff.location = APPLY_STR;
    aff.bitvector = AFF_POISON;

    aff2.type = AFFECT_DISEASE;
    aff2.level = 0;
    aff2.duration = aff.duration;
    aff2.modifier = DISEASE_POISON;
    aff2.location = APPLY_NONE;
    aff2.bitvector = AFF_POISON;

    act("The stingers on $p slice through your skin.", 0, vict, o, 0, TO_CHAR,
      ANSI_ORANGE);
    act("The stingers on $p slice through $n's skin.", 0, vict, o, 0, TO_ROOM,
      ANSI_ORANGE);
    vict->affectTo(&aff);
    vict->affectTo(&aff2);
    disease_start(vict, &aff2);

    return true;
  }
  return false;
}

int glowCutlass(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc;

  if (cmd != CMD_OBJ_HITTING)
    return false;
  if (!o || !vict)
    return false;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)

  primaryTypeT primary =
    (ch->heldInPrimHand() == o) ? HAND_PRIMARY : HAND_SECONDARY;

  if (!::number(0, 9)) {
    // this potentially sets up infinite loop
    rc = ch->oneHit(vict, primary, o, 0, 0);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete ch;
      ch = nullptr;
      REM_DELETE(rc, DELETE_THIS);
    }
    if (IS_SET_DELETE(rc, DELETE_VICT) || IS_SET_DELETE(rc, DELETE_ITEM)) {
      return rc;
    }
    return false;
  }
  return false;
}

int weaponBreaker(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;

  sstring buf;

  if (!o || !vict)
    return false;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)
  if (::number(0, 49))
    return false;
  if (cmd != CMD_OBJ_HITTING)
    return false;

  wearSlotT slot;
  int count = 0;
  for (slot = pickRandomLimb();; slot = pickRandomLimb()) {
    count++;
    if (count > 100)
      return false;
    if (notBreakSlot(slot, true))
      continue;
    if (!vict->slotChance(slot))
      continue;
    break;
  }

  if (vict->isImmune(IMMUNE_BONE_COND, slot) || vict->raceHasNoBones()) {
    return false;
  }

  if (vict->debuffResist())
    return false;

  if (!ch->canBoneBreak(vict, SILENT_YES))
    return false;

  char limb[80];
  sprintf(limb, "%s", vict->describeBodySlot(slot).c_str());

  vict->addToLimbFlags(slot, PART_BROKEN);
  buf = format("A muffled SNAP leaps from your %s as $n hits it with $s $p!") %
        limb;
  act(buf, false, ch, o, vict, TO_VICT, ANSI_ORANGE);
  buf = format(
          "Extreme pain shoots through your %s!\n\rYour %s has been broken and "
          "is now useless!") %
        limb % limb;
  act(buf, false, vict, nullptr, nullptr, TO_CHAR, ANSI_ORANGE);

  buf = format("You hit $N's %s hard with your $p.") % limb;
  act(buf, false, ch, o, vict, TO_CHAR, ANSI_ORANGE);
  buf = format("$n hits $N's %s hard with $s $p.") % limb;
  act(buf, false, ch, o, vict, TO_NOTVICT, ANSI_ORANGE);

  buf =
    format("You hear a muffled SNAP as $n clutches $s %s in extreme pain!") %
    limb;
  act(buf, false, vict, nullptr, nullptr, TO_ROOM, ANSI_ORANGE);

  vict->dropWeapon(slot);

  return true;
}

int weaponFumbler(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  TThing* obj;

  ch = genericWeaponProcCheck(vict, cmd, o, 19);
  if (!ch)
    return false;

  act("You cleave $N's body hard with your $o.", true, ch, o, vict, TO_CHAR,
    ANSI_ORANGE);
  act("$n cleaves your body hard with $s $o.", true, ch, o, vict, TO_VICT,
    ANSI_ORANGE);
  act("$n cleaves $N's body hard with $s $o.", true, ch, o, vict, TO_NOTVICT,
    ANSI_ORANGE);

  if (ch->reconcileDamage(vict, ::number(1, 3), o->getWtype()) == -1) {
    return DELETE_VICT;
  }

  if ((obj = vict->heldInPrimHand())) {
    act("The blow knocks $n backwards and $e drops $s $o!", true, vict, obj, 0,
      TO_ROOM, ANSI_ORANGE);
    act("The blow knocks you backwards and you drop $p!", true, vict, obj, 0,
      TO_CHAR, ANSI_ORANGE);
    *vict->roomp += *vict->unequip(vict->getPrimaryHold());
  } else {
    act("The blow knocks $n backwards!", true, vict, 0, 0, TO_ROOM,
      ANSI_ORANGE);
    act("The blow knocks you backwards!", true, vict, 0, 0, TO_CHAR,
      ANSI_ORANGE);
  }

  if (vict->riding) {
    int rc = vict->fallOffMount(vict->riding, POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;
  }
  vict->setPosition(POSITION_SITTING);

  vict->cantHit += vict->loseRound(0.5);
  return true;
}

int weaponDisruption(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int hardness, rc;
  wearSlotT part;
  TThing* obj;
  sstring buf;

  if (cmd != CMD_OBJ_HITTING)
    return false;
  if (!o || !vict)
    return false;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)
  if (::number(0, 10))
    return false;

  part = vict->getPartHit(ch, true);
  if (!(obj = vict->equipment[part]))
    hardness = material_nums[vict->getMaterial(part)].hardness;
  else
    hardness = material_nums[obj->getMaterial()].hardness;
  spellNumT w_type = o->getWtype();
  obj_act("hums softly which quickly rises to a high pitched whine.", ch, o,
    vict, ANSI_ORANGE);
  buf = format("$n's $p screams with power as $e swings it at your %s!") %
        vict->describeBodySlot(part);
  act(buf, true, ch, o, vict, TO_VICT, ANSI_RED);
  buf = format("$n's $p screams with power as $e swings it at $N's %s!") %
        vict->describeBodySlot(part);
  act(buf, true, ch, o, vict, TO_NOTVICT, ANSI_ORANGE);
  buf = format("Your $p screams with power as you swing it at $N's %s!") %
        vict->describeBodySlot(part);
  act(buf, true, ch, o, vict, TO_CHAR, ANSI_GREEN);

  buf =
    format("A soft WOMPF! is heard as $p releases a shock wave into $n's %s!") %
    (obj ? obj->getName() : (vict->isHumanoid() ? "skin" : "hide"));
  act(buf, true, vict, o, 0, TO_ROOM, ANSI_ORANGE);
  buf =
    format("A soft WOMPF! is heard as $p releases a shock wave into your %s!") %
    (obj ? obj->getName() : (vict->isHumanoid() ? "skin" : "hide"));
  act(buf, true, vict, o, 0, TO_CHAR, ANSI_RED);

  if (obj)
    vict->damageItem(ch, part, w_type, o, hardness);
  else {
    rc = ch->damageLimb(vict, part, o, &hardness);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }
  if (ch->reconcileDamage(vict, hardness, DAMAGE_DISRUPTION) == -1)
    return DELETE_VICT;
  return true;
}

int warMaker(TBeing* ch, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  sstring buf;
  TBeing* tmp;

  if (cmd < MAX_CMD_LIST) {
    if ((o->equippedBy != ch) && (o->parent != ch))
      return false;

    switch (cmd) {
      case CMD_CAST:
      case CMD_RECITE:
        obj_act("hums 'Get this:  $n wants to use some sissy magic!'", ch, o,
          nullptr, nullptr);
        ch->sendTo(
          "You feel confused... what was it you were going to do?\n\r");
        return true;
      case CMD_FLEE:
        if (o->equippedBy) {
          act("$n's $p glows red-hot in $s hands!", 1, ch, o, nullptr, TO_ROOM,
            nullptr);
          act("Your $p glows red-hot in your hands!", 1, ch, o, nullptr, TO_CHAR);
          ch->addToHit(-dice(5, 5));
          if (ch->getHit() < 0) {
            ch->setHit(0);
            ch->setPosition(POSITION_STUNNED);
          }
          if (!ch->isTough()) {
            *ch->roomp += *ch->unequip(o->eq_pos);
            act("$n screams loudly, dropping $s $p.", 1, ch, o, nullptr, TO_ROOM);
            act("You scream loudly, dropping your $p.", 1, ch, o, nullptr,
              TO_CHAR);
          }
        }
        return true;
      default:
        return false;
    }
  } else if (cmd == CMD_GENERIC_PULSE) {
    if (number(0, 13))
      return false;
    if (o->in_room != -1) {
      buf = format("%s moves a bit... as if alive!\n\r") % o->shortDescr;
      sendToRoom(buf.c_str(), roomOfObject(o));
    } else if ((tmp = dynamic_cast<TBeing*>(o->equippedBy))) {
      if (!tmp->fight()) {
        switch (number(1, 30)) {
          case 1:
            obj_act("sings 'Follow the Yellow Brick Road.'", tmp, o, nullptr,
              ANSI_ORANGE);
            break;
          case 2:
            obj_act(
              "says, 'Let's go to the Shire, I want to waste some halfling "
              "youths.'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 3:
            obj_act("says, 'Use the Force, L.., I mean:  go get um tiger.'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 4:
            obj_act(
              "bemoans, 'How is it such a wonderous sword as me gets stuck "
              "with a wimp and coward like you?'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 5:
            obj_act(
              "says, 'We gonna stand here all day, or we gonna kill this "
              "thing?'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 6:
            obj_act("says, 'Group me.'", tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 7:
            obj_act("says, 'That's it.  Puff must die.'", tmp, o, nullptr,
              ANSI_ORANGE);
            break;
          case 8:
            obj_act("says, 'Some of my closest friends are training daggers.'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 9:
            obj_act(
              "grins, 'I love it when they buff up mobs, it gives me a "
              "challenge.'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 10:
            obj_act(
              "wonders, 'Is it me, or are most other swords rather quiet?'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 11:
            obj_act("says, 'In a past life, I was a pipeweed bread.'", tmp, o,
              nullptr, ANSI_ORANGE);
            break;
          case 12:
            obj_act("says, 'Puff sure is a friendly dragon, isn't he.'", tmp, o,
              nullptr, ANSI_ORANGE);
            break;
          case 13:
            obj_act(
              "says, 'The imps must hate the players to make asword SOooO "
              "AnnOYing!'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 14:
            obj_act(
              "wonders, 'How come I'm never forced to shout how much I love "
              "Puff?'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 15:
            obj_act("says, 'Let's see who's using Tin tin.  Snowy shouts 'Yo''",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 16:
            obj_act(
              "screams, 'BANZAI, CHARGE, SPORK, GERONIMO, DIE VERMIN, HIEYAH!'",
              tmp, o, nullptr, ANSI_ORANGE);
            obj_act(
              "blushes, 'Ooops, sorry, got over anxious to kill something.'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 17:
            obj_act(
              "offers, 'A good Swedish massage would cure what ails you.'", tmp,
              o, nullptr, ANSI_ORANGE);
            break;
          case 18:
            obj_act("muses: 'How exactly *does* an inanimate object talk?'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 19:
            obj_act("states, 'I'd rather be playing scrabble.'", tmp, o, nullptr,
              ANSI_ORANGE);
            break;
          case 20:
            obj_act(
              "says, 'A corpse is something to be cherished forever...until it "
              "rots.'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 21:
            obj_act(
              "sniffs the air, 'I love the smell of blood and ichor in the "
              "morning!'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 22:
            obj_act(
              "chants, 'You might have armor or you might have fur, I'll hack "
              "them both, cuz I'm War-make-er.'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 23:
            obj_act("scoffs, 'Sword of the Ancients, what a piece of junk!'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 24:
            obj_act(
              "says, 'Do I have to fight again?  I just got all the gore off "
              "from last time.'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          case 26:
            obj_act(
              "moans, 'If I have to kill Aunt Bee one more time, I'm gonna "
              "scream!'",
              tmp, o, nullptr, ANSI_ORANGE);
            break;
          default:
            obj_act("grumbles 'Lets go kill something!'", tmp, o, nullptr,
              ANSI_ORANGE);
            break;
        }
      }
    } else if (o->parent && dynamic_cast<TObj*>(o->parent)) {
      buf = format(
              "Something grumbles 'Damnit, I'm %s.  Let me out of here.  It's "
              "dark.'\n\r") %
            o->shortDescr;
      sendToRoom(COLOR_BASIC, buf.c_str(), roomOfObject(o));
    } else if (o->parent) {
      act("$n's $o begs $m to wield it.", 1, o->parent, o, nullptr, TO_ROOM);
      act("Your $o begs you to wield it.", 1, o->parent, o, nullptr, TO_CHAR);
    }
    return true;
  } else if ((cmd == CMD_OBJ_HITTING) && (dice(1, 10) == 1)) {
    if (!(tmp = dynamic_cast<TBeing*>(o->equippedBy)->fight()))
      return false;
    if (number(0, 1)) {
      switch (number(1, 20)) {
        case 1:
          obj_act("criticizes $N's lineage.", o->equippedBy, o, tmp,
            ANSI_ORANGE);
          break;
        case 2:
          obj_act("says to $N, 'I fart in your general direction.'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 3:
          obj_act(
            "looks at $N and asks, 'Is that your real face or are you just "
            "naturally ugly?'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 4:
          obj_act(
            "tells $N, 'If they held an ugly pageant, you'd be a shoe in.'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 5:
          obj_act("tells $N, 'I wave my private parts in your direction!'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 6:
          obj_act("snickers something about $N's mother and combat boots.",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 7:
          obj_act(
            "asks $N with a smirk, 'So, how many newbies have killed you "
            "today?'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 8:
          obj_act(
            "states, 'Ya know, your sister was a mighty fine piece of ...'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 9:
          obj_act("looks at $N and says 'Not worth the time.'", o->equippedBy,
            (o), tmp, ANSI_ORANGE);
          break;
        case 10:
          obj_act(
            "tells $N, 'Killing you is about as challenging as buying bread.'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 11:
          obj_act("laughs at the stupidity of $N.", o->equippedBy, o, tmp,
            ANSI_ORANGE);
          break;
        case 12:
          obj_act("ponders: 'think I can kill $N in 5 rounds or less?'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 13:
          obj_act("snickers, 'OOH! Look at the fangs on that one.'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 14:
          obj_act(
            "groans, '$N?!!  Surely you jest?  Haven't we killed $M enough "
            "already?'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 15:
          obj_act(
            "looks at $N and sighs, 'When you get around to attacking "
            "something real, wake me.'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 16:
          obj_act(
            "grins, 'Your head will look great mounted up above my mantle "
            "piece.'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        case 17:
          obj_act(
            "looks at $N and says, 'Are you as stupid as you look, or do you "
            "just look stupid?'",
            o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
        default:
          obj_act("taunts $N mercilessly.", o->equippedBy, o, tmp, ANSI_ORANGE);
          break;
      }
    }
    if (tmp->fight() != o->equippedBy) {
      act("$N turns towards $n with rage in $S eyes.", 1, o->equippedBy, o, tmp,
        TO_ROOM);
      act("$N turns towards you with rage in $S eyes.", 1, o->equippedBy, o,
        tmp, TO_CHAR);
      tmp->stopFighting();
      tmp->setCharFighting(dynamic_cast<TBeing*>(o->equippedBy));
    }
    return false;
  }
  return false;
}

int Gwarthir(TBeing* ch, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing *vict, *tmp;

  tmp = genericWeaponProcCheck(ch, cmd, o, 0);
  if (!tmp)
    return false;
  if (!(vict = tmp->fight()))
    return false;

  int dmg = ::number(1, 3);
  if (tmp->reconcileDamage(vict, dmg, DAMAGE_DRAIN) == -1) {
    if (vict == ch)
      return DELETE_VICT;
    delete vict;
    vict = nullptr;
  }
  tmp->addToHit(dmg);

  return true;
}

int gnomeTenderizer(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o,
  TObj*) {
  TBeing *ch, *tmp_victim, *temp;
  int dmg = 0;
  if (!cmd || !o)
    return false;
  if (cmd != CMD_OBJ_HIT && cmd != CMD_GENERIC_PULSE)
    return false;
  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;
  if (::number(0, 6))
    return false;

  primaryTypeT hand =
    (ch->heldInPrimHand() == o) ? HAND_PRIMARY : HAND_SECONDARY;

  if (cmd == CMD_GENERIC_PULSE) {
    // recharging and idle room noise
    if (!o->isObjStat(ITEM_GLOW)) {
      act("$p begins to glow softly.", false, ch, o, nullptr, TO_CHAR,
        ANSI_ORANGE);
      act("$n's $o begins to glow softly.", false, ch, o, nullptr, TO_ROOM,
        ANSI_ORANGE);
      o->addObjStat(ITEM_GLOW);
      return true;
    } else if (!o->isObjStat(ITEM_HUM) && o->isObjStat(ITEM_GLOW)) {
      act("$p begins to emit a high-pitched hum.", false, ch, o, nullptr, TO_CHAR,
        ANSI_ORANGE);
      act("$n's $o begins to emit a high-pitched hum.", false, ch, o, nullptr,
        TO_ROOM, ANSI_ORANGE);
      o->addObjStat(ITEM_HUM);
      return true;
    } else if (!::number(0, 5) && o->isObjStat(ITEM_HUM)) {
      act("$p continues its high-pitched song.", false, ch, o, nullptr, TO_CHAR,
        ANSI_ORANGE);
      act("$n's $o continues its high-pitched song.", false, ch, o, nullptr,
        TO_ROOM, ANSI_ORANGE);
      return true;
    }
    return true;
  } else if (cmd == CMD_OBJ_HIT) {
    // combat
    if (!vict || !(vict == ch->fight()))
      return false;

    wearSlotT part = wearSlotT((long int)arg);

    act(
      "A bar from $p swings around and meets $N's flesh flush at its "
      "slightly-hollowed end.",
      false, ch, o, vict, TO_CHAR, ANSI_CYAN);
    act(
      "A bar from $p swings around and meets your flesh flush at its "
      "slightly-hollowed end.",
      false, ch, o, vict, TO_VICT, ANSI_CYAN);
    act(
      "A bar from $n's $o swings around and meets $N's flesh flush at its "
      "slightly-hollowed end.",
      false, ch, o, vict, TO_NOTVICT, ANSI_CYAN);
    if (vict->getRace() == RACE_GNOME) {
      act("You scream like a stuck pig and start to cry like a little baby.",
        false, vict, o, 0, TO_CHAR, ANSI_BLUE_BOLD);
      act("$n screams like a stuck pig and starts to cry like a little baby.",
        false, vict, o, 0, TO_ROOM, ANSI_BLUE_BOLD);
    }

    if (!o->isObjStat(ITEM_HUM) && !vict->isImmune(IMMUNE_SKIN_COND, part)) {
      // if it's not humming/glowing - low damage proc
      act(
        "$N yelps in pain as you pull $p away, tearing off a neat circle of "
        "flesh.",
        false, ch, o, vict, TO_CHAR, ANSI_RED);
      act(
        "You feel a sharp pain as $n pulls the $o away, tearing off a neat "
        "circle of flesh.",
        false, ch, o, vict, TO_VICT, ANSI_RED);
      act(
        "$N yelps in pain as $n pulls the $o away, tearing off a neat circle "
        "of flesh.",
        false, ch, o, vict, TO_NOTVICT, ANSI_RED);

      dmg = ::number(1, max(1, int(ch->getWeaponDam(vict, o, hand) / 10)));
      if (vict->getRace() == RACE_GNOME)
        dmg += dmg / 2;
      if (ch->reconcileDamage(vict, dmg, DAMAGE_NORMAL) == -1) {
        return DELETE_VICT;
      }
      if (!vict->isImmune(IMMUNE_BLEED, part) &&
          !vict->isLimbFlags(part, PART_BLEEDING))
        vict->rawBleed(part, 100, SILENT_NO, CHECK_IMMUNITY_NO);
      return true;
    } else if (o->isObjStat(ITEM_HUM)) {
      act("A loud *BRRAAACK* sounds out as the bar makes contact.", false, ch,
        o, vict, TO_CHAR, ANSI_CYAN);
      act("A loud *BRRAAACK* sounds out as the bar makes contact.", false, ch,
        o, vict, TO_ROOM, ANSI_CYAN);

      if (!vict->isImmune(IMMUNE_SKIN_COND, part)) {
        act("$N wails as $p tenderizes their flesh.", false, ch, o, vict,
          TO_CHAR, ANSI_RED);
        act("You erupt in pain as $n's $o tenderizes your flesh.", false, ch, o,
          vict, TO_VICT, ANSI_RED);
        act("$N howls in pain as $n's $o tenderizes their flesh.", false, ch, o,
          vict, TO_NOTVICT, ANSI_RED);

        dmg = ::number(1, max(1, ch->getWeaponDam(vict, o, hand)));
        if (vict->getRace() == RACE_GNOME)
          dmg += dmg / 2;
        if (ch->reconcileDamage(vict, dmg, DAMAGE_NORMAL) == -1) {
          return DELETE_VICT;
        }
        if (!vict->isImmune(IMMUNE_BLEED, part) &&
            !vict->isLimbFlags(part, PART_BLEEDING))
          vict->rawBleed(part, 250, SILENT_NO, CHECK_IMMUNITY_NO);
      }

      act("You feel a deep silence descend around you.", false, ch, o, vict,
        TO_CHAR, ANSI_CYAN);

      // area effect
      for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        temp = tmp_victim->next;
        if (ch->sameRoom(*tmp_victim) && (ch != tmp_victim) &&
            (!tmp_victim->isImmortal()) && !ch->noHarmCheck(tmp_victim)) {
          dmg = ::number(1, max(1, ch->getWeaponDam(tmp_victim, o, hand)));
          if (vict->getRace() == RACE_GNOME)
            dmg += dmg / 2;
          if (ch->inGroup(*tmp_victim) ||
              (tmp_victim->master && ch->inGroup(*(tmp_victim->master)))) {
            // protect group members
            act("You feel a deep silence descend around you.", false,
              tmp_victim, o, vict, TO_CHAR, ANSI_CYAN);
            continue;
          }

          act(
            "$N winces in pain as an intense shockwave rips through the "
            "vicinity.",
            false, ch, nullptr, tmp_victim, TO_CHAR, ANSI_RED);
          act(
            "$N winces in pain as an intense shockwave rips through the "
            "vicinity.",
            false, ch, nullptr, tmp_victim, TO_NOTVICT, ANSI_RED);
          act(
            "Pain blossoms in your head as an intense shockwave rips through "
            "the vicinity.",
            false, ch, nullptr, tmp_victim, TO_VICT, ANSI_RED_BOLD);
          if (ch->reconcileDamage(tmp_victim, dmg, DAMAGE_NORMAL) == -1) {
            if (tmp_victim == vict)
              return DELETE_VICT;
            else {
              tmp_victim->reformGroup();
              delete tmp_victim;
              tmp_victim = nullptr;
              continue;
            }
          }
        } else if ((ch != tmp_victim) &&
                   (tmp_victim->in_room != Room::NOWHERE) &&
                   (ch->roomp->getZoneNum() ==
                     tmp_victim->roomp->getZoneNum())) {
          if (tmp_victim->awake())
            tmp_victim->sendTo(
              "*BRRAAACK* You hear a loud percussive noise coming from "
              "nearby.\n\r");
        }
      }
      act("The deep silence lifts from around you.", false, ch, o, 0, TO_CHAR,
        ANSI_CYAN);
      for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        temp = tmp_victim->next;
        if (ch->sameRoom(*tmp_victim) && (ch != tmp_victim) &&
            (!tmp_victim->isImmortal())) {
          if (ch->inGroup(*tmp_victim) ||
              (tmp_victim->master && ch->inGroup(*(tmp_victim->master)))) {
            act("The deep silence lifts from around you.", false, tmp_victim, o,
              vict, TO_CHAR, ANSI_CYAN);
            continue;
          }
        }
      }

      o->remObjStat(ITEM_GLOW);
      o->remObjStat(ITEM_HUM);
      act("$p's glow fades away.", false, ch, o, nullptr, TO_CHAR, ANSI_ORANGE);
      act("$n's $o's glow fades away.", false, ch, o, nullptr, TO_ROOM,
        ANSI_ORANGE);

      return true;
    }
  }
  return true;
}

int demonSlayer(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (!ch)
    return false;
  if (vict->getRace() != RACE_DEMON || vict->isImmune(IMMUNE_BLEED, WEAR_BODY))
    return false;

  dam = ch->GetMaxLevel();
  act("$p cleaves into the flesh of $n, trailing gore and a mist of blood!", 0,
    vict, o, 0, TO_ROOM, ANSI_WHITE_BOLD);
  act("$p rends your flesh, leaving a bloody gaping wound!", 0, vict, o, 0,
    TO_CHAR, ANSI_WHITE_BOLD);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_HEMORRHAGE);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return true;
}

int holyCutlass(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBaseWeapon* cutlass;
  TBeing* ch;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  if (::number(0, 19))
    return false;

  if (!(cutlass = dynamic_cast<TBaseWeapon*>(o)))
    return false;

  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)

  cutlass->setPoison(LIQ_HOLYWATER);

  act("<W>Holy water from $n's $o and drips to the $g.<1>", 0, ch, o, 0,
    TO_ROOM);
  act(
    "<W>Your $o oozes holy water, which runs down the length of the blade and "
    "drips to the $g.<1>",
    0, ch, o, 0, TO_CHAR);

  return true;
}

int shadowWeapon(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (!ch)
    return false;

  dam = ::number(4, 10);

  act("$N screams as <p>tendrils<k> of shadow leap from your $o.", 0, ch, o,
    vict, TO_CHAR, ANSI_GRAY);
  act("$N screams as <p>tendrils<k> of shadow leap from $n's $o.", 0, ch, o,
    vict, TO_NOTVICT, ANSI_GRAY);
  act("You scream as <p>tendrils<k> of shadow leap from $n's $o.", 0, ch, o,
    vict, TO_VICT, ANSI_GRAY);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_DRAIN);
  if (rc == -1)
    return DELETE_VICT;
  return true;
}

int livingVines(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 5);
  if (!ch)
    return false;

  dam = ::number(1, 3);
  act("$N stumbles as vines spring from the ground.", 0, ch, o, vict, TO_CHAR,
    ANSI_GREEN);
  act("$N stumbles as vines spring from the ground.", 0, ch, o, vict,
    TO_NOTVICT, ANSI_GREEN);
  act("You stumble as vines spring from the ground.", 0, ch, o, vict, TO_VICT,
    ANSI_GREEN);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_NORMAL);
  if (rc == -1)
    return DELETE_VICT;

  vict->cantHit += vict->loseRound(0.50);
  return true;
}

// Summary of this weapon proc
//    - fire damage proc
//    - 1.75% chance of bonebreak, with a fire theme (withers the limb due to
//    burn damage)
//    - Flavor text proc of dripping lava from the weapon
int moltenWeapon(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  char limb[80];
  int rc, dam = 0, chance;
  TBeing* ch;
  sstring buf;
  wearSlotT slot;

  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)

  // Flavor text pulse
  if (cmd == CMD_GENERIC_PULSE) {
    if (!::number(0, 150)) {
      act("<r>Drops of molten hot lava fall from $n's $o onto the $g.<1>", 0,
        ch, o, 0, TO_ROOM);
      act("<r>Your $o drips molten hot lava onto the $g.<1>", 0, ch, o, 0,
        TO_CHAR);
    }
  }

  // Combat procs
  if (cmd == CMD_OBJ_HIT) {
    // Slightly lowering chance to proc to balance with bone break/unmaker
    // Since this has extra damage components
    chance = ::number(1, 57);

    if (!o || !vict)
      return false;

    // Bonebreak proc + 10-20 damage if successful (max once/fight)
    if (chance == 1) {
      if (!ch->canWither(vict, SILENT_YES))
        return false;

      bool ok = false;
      bool found = false;

      // Initial checks to prevent issues and for balancee reasons
      for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
        if (notBreakSlot(slot, false))  // same ones, right?
          continue;
        if (!vict->slotChance(slot))
          continue;
        found |= (vict->isLimbFlags(slot, PART_MISSING));
        ok = true;
      }

      if (found || !ok)
        return false;

      // Accounting for fire immunity since this is heat-based wither limb
      if (vict->isImmune(IMMUNE_HEAT, slot))
        return false;

      // Reduce chance if Elite
      if (vict->debuffResist())
        return false;

      // Get a random limb slot and make sure it's valid
      for (slot = pickRandomLimb();; slot = pickRandomLimb()) {
        if (notBreakSlot(slot, true))
          continue;
        if (!vict->slotChance(slot))
          continue;
        break;
      }

      if (!vict->hasPart(slot)) {
        vlogf(LOG_COMBAT,
          format("BOGUS SLOT trying to be made PART_USELESS: %d on %s") % slot %
            vict->getName());
        return false;
      }

      if (!vict->roomp) {
        vlogf(LOG_COMBAT,
          format("!roomp for target (%s) when trying to make PART_USELESS.") %
            vict->getName());
        return false;
      }

      vict->setLimbFlags(slot, PART_MISSING);

      TThing* t;
      if ((t = vict->unequip(slot)))
        *(vict->roomp) += *t;

      for (wearSlotT j = MIN_WEAR; j < MAX_WEAR; j++) {
        if (!vict->hasPart(j))
          continue;
        if (!vict->isBodyPartAttached(j)) {
          vict->setLimbFlags(j, PART_MISSING);
          TThing* tmp = vict->unequip(j);
          if (tmp)
            *(vict->roomp) += *tmp;
        }
      }

      // check for damage to both hands
      vict->dropItemFromDamagedHand(true);
      vict->dropItemFromDamagedHand(false);

      sprintf(limb, "%s", vict->describeBodySlot(slot).c_str());

      buf = format(
        "Your $p glows molten red and begins emitting an incredible amount of "
        "heat!");
      act(buf, false, ch, o, vict, TO_CHAR, ANSI_RED);
      buf = format(
        "$n's $p glows molten red and begins emitting an incredible amount of "
        "heat!");
      act(buf, false, ch, o, vict, TO_NOTVICT, ANSI_RED);
      buf = format(
        "$n's $p glows molten red and begins emitting an incredible amount of "
        "heat!");
      act(buf, false, ch, o, vict, TO_VICT, ANSI_RED);

      buf = format(
              "You look down in horror and disbelief, as $n's $p burns into "
              "your %s with molten heat!") %
            limb;
      act(buf, false, ch, o, vict, TO_VICT, ANSI_ORANGE);
      buf = format(
              "The pain overloads your senses, as your %s is burned away "
              "entirely!") %
            limb;
      act(buf, false, vict, nullptr, nullptr, TO_CHAR, ANSI_ORANGE);

      buf = format(
              "Your $p glows brightly as it burns into $N's %s with all the "
              "heat of the sun!") %
            limb;
      act(buf, false, ch, o, vict, TO_CHAR, ANSI_ORANGE);
      buf = format(
              "$n makes contact and burns into $N's %s with all the heat of "
              "the sun.") %
            limb;
      act(buf, false, ch, o, vict, TO_NOTVICT, ANSI_ORANGE);

      buf = format(
              "When the smoke clears, you're horrified to notice that $n's %s "
              "is a withered husk!") %
            limb;
      act(buf, false, vict, nullptr, nullptr, TO_ROOM, ANSI_ORANGE);

      vict->dropWeapon(slot);

      dam = ::number(10, 20);

      rc = vict->reconcileDamage(vict, dam, DAMAGE_FIRE);

      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_VICT;

      return true;
      // Damage Proc
    } else if (chance >= 2 && chance <= 10) {
      dam = ::number(4, 10);

      act("The flames from your $p burn $N.", 0, ch, o, vict, TO_CHAR,
        ANSI_ORANGE);
      act("The flames from $n's $p burn $N.", 0, ch, o, vict, TO_ROOM,
        ANSI_ORANGE);
      act("The flames from $p burn your flesh.", 0, vict, o, 0, TO_CHAR,
        ANSI_ORANGE);

      rc = vict->reconcileDamage(vict, dam, DAMAGE_FIRE);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_VICT;

      return true;
    }
  }
  return false;
}

// Summary of this weapon proc
//    - 1d8 frost  damage proc
//    - 0.4% chance of freezing the opponent (stun for 1 second)
//    - Flavor text proc that's ice-themed
int glacialWeapon(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam, roll;

  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)

  if (cmd == CMD_GENERIC_PULSE) {
    if (!::number(0, 150)) {
      act(
        "<b>Small pieces of ice break off from $n's $o and fall to the $g.<1>",
        0, ch, o, 0, TO_ROOM);
      act(
        "<b>A few pieces of ice break off from your $o and fall to the $g.<1>",
        0, ch, o, 0, TO_CHAR);
    }
  } else if (cmd == CMD_OBJ_HIT) {
    roll = ::number(0, 250);
    if (roll > 0 && roll < 25) {
      // damage proc
      dam = ::number(1, 8);

      act("The extreme cold from your $p chills $N.", 0, ch, o, vict, TO_CHAR,
        ANSI_BLUE);
      act("The extreme cold from $n's $p chills $N.", 0, ch, o, vict, TO_ROOM,
        ANSI_BLUE);
      act("The extreme cold from $p chills your flesh.", 0, vict, o, 0, TO_CHAR,
        ANSI_BLUE);

      rc = vict->reconcileDamage(vict, dam, DAMAGE_FROST);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_VICT;
      return true;
    } else if (roll == 0) {
      // stun proc
      act(
        "<c>The air around <1>$n<c> seems to waver, then becomes <B>extremely "
        "cold<1><c>!<1>",
        true, ch, o, nullptr, TO_ROOM, nullptr);
      act("<c>A blast of frigid air radiates from <1>$n<c>!<1>", true, ch, o,
        nullptr, TO_ROOM, nullptr);
      act(
        "<c>The air around you seems to waver, then becomes <B>extremely "
        "cold<1><c>!<1>",
        true, ch, o, nullptr, TO_CHAR, nullptr);
      act("<c>A blast of frigid air radiates from you<c>!<1>", true, ch, o,
        nullptr, TO_CHAR, nullptr);

      if (vict->riding) {
        act("The blast of <c>fro<b>zen <c>air<1> knocks $N from $S mount!",
          true, ch, o, vict, TO_CHAR, nullptr);
        act("The blast of <c>fro<b>zen <c>air<1> knocks $N from $S mount!",
          true, ch, o, vict, TO_NOTVICT, nullptr);
        act(
          "<o>The blast of <c>fro<b>zen <c>air<1> knocks you from your "
          "mount!<1>",
          true, ch, o, vict, TO_VICT, nullptr);
        vict->dismount(POSITION_RESTING);
      }
      act(
        "The blast of <c>fro<b>zen <c>air<1> from your $o slams $N into the "
        "$g, stunning $M!",
        true, ch, o, vict, TO_CHAR, nullptr);
      act(
        "The blast of <c>fro<b>zen <c>air<1> from $n's $o slams $N into the "
        "$g, stunning $M!",
        true, ch, o, vict, TO_NOTVICT, nullptr);
      act(
        "The blast of <c>fro<b>zen <c>air<1> from $n's $o slams you into the "
        "$g, stunning you!",
        true, ch, o, vict, TO_VICT, nullptr);

      dam = ::number(10, 20);
      rc = vict->reconcileDamage(vict, dam, DAMAGE_FROST);

      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_VICT;

      affectedData aff;
      aff.type = SKILL_DOORBASH;
      aff.duration = Pulse::TICK * 4;
      aff.bitvector = AFF_STUNNED;
      vict->affectTo(&aff, -1);

      if (vict->fight())
        vict->stopFighting();
      return true;
    }
  }
  return false;
}
