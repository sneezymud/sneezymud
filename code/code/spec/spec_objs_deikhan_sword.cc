#include "comm.h"
#include "obj_base_weapon.h"
#include "being.h"

// Previously we used item VNUM to control affects of different deikhan swords
// Hoping to acheive similar results using item levels to control which affects
// are active TBaseWeapon::damageLevel() Avenger = 31.75 Vindicator = 40.75
// Devasator = 52.75
//
// Additionally char levels are used to differentiate the strength of affects.
// (No twinked nebies) This seems more fair and also doesn't require a code
// change to add a new deikhan sword

// heal ser for avenger, heal crit for vindicator, heal for devastator
void doHeal(TBeing* ch, TObj* o) {
  int hp;

  // Get char level to control strength of heal
  int charLevel = ch->GetMaxLevel();

  act("$n's $o emanates a gentle <r>w<R>a<Y>rm<R>t<1><r>h.<1>", 0, ch, o, 0,
    TO_ROOM);
  act("Your $o emanates a gentle <r>w<R>a<Y>rm<R>t<1><r>h.<1>", 0, ch, o, 0,
    TO_CHAR);

  if (charLevel >= 45) {
    // Devastator level heals
    act("$n glows briefly with an <b>ultramarine hue<1>.", false, ch, nullptr, 0,
      TO_ROOM);
    act("You glow briefly with an <b>ultramarine hue<1>.", false, ch, nullptr, 0,
      TO_CHAR);

    hp = ch->getSkillDam(ch, SPELL_HEAL, 50, 100);
  } else if (charLevel >= 30) {
    // Vindicator level heals
    colorAct(COLOR_SPELLS, "$n glows briefly with an <p>indigo hue<1>.", false,
      ch, nullptr, 0, TO_ROOM);
    colorAct(COLOR_SPELLS, "You glow briefly with an <p>indigo hue<1>.", false,
      ch, nullptr, 0, TO_CHAR);

    hp = ch->getSkillDam(ch, SPELL_HEAL_CRITICAL, 50, 100);
  } else {
    // Avenger level heals
    colorAct(COLOR_SPELLS, "$n glows briefly with a <b>blue hue<z>.", false, ch,
      nullptr, 0, TO_ROOM);
    colorAct(COLOR_SPELLS, "You glow briefly with a <b>blue hue<z>.", false, ch,
      nullptr, 0, TO_CHAR);

    hp = ch->getSkillDam(ch, SPELL_HEAL_SERIOUS, 50, 100);
  }

  ch->addToHit(hp);
  ch->updatePos();
}

void doBlind(TBeing* ch, TBeing* vict, TObj* o) {
  TBaseWeapon* tWeap;

  if (!(tWeap = dynamic_cast<TBaseWeapon*>(o)) || !vict)
    return;

  if (vict->affectedBySpell(SPELL_BLINDNESS) ||
      vict->isAffected(AFF_TRUE_SIGHT) || vict->isAffected(AFF_CLARITY) ||
      ch->isNotPowerful(vict, (int)tWeap->weaponLevel(), SPELL_BLINDNESS,
        SILENT_YES))
    return;

  act("A searing light shines from $p, blinding $N.", false, ch, o, vict,
    TO_CHAR);
  act("$n shields $s eyes as a searing light shines from $p, blinding $N.",
    false, ch, o, vict, TO_NOTVICT);
  act("The world goes white then black as a searing light shines from $n's $p.",
    false, ch, o, vict, TO_VICT);

  int tDuration = (int)(tWeap->weaponLevel() * Pulse::UPDATES_PER_MUDHOUR);
  saveTypeT tSave = SAVE_NO;

  vict->rawBlind((int)tWeap->weaponLevel(), tDuration, tSave);
}

void doHolyWrath(TBeing* ch, TObj* o) {
  affectedData aff, aff2;

  // normalize bonus between 1 and 3
  int modifier = max(1, (int)(ch->GetMaxLevel() / 15));
  modifier = min(3, modifier);

  aff.type = AFFECT_HOLY_WRATH;
  aff.duration = Pulse::UPDATES_PER_MUDHOUR / 40 * ch->GetMaxLevel();
  aff.location = APPLY_HITROLL;
  aff.modifier = modifier;
  aff.bitvector = 0;

  if (!ch->affectedBySpell(AFFECT_HOLY_WRATH)) {
    act("$n's $o glows with <y>Holy Wrath<1>!", 0, ch, o, 0, TO_ROOM);
    act("Your $o glows with <y>Holy Wrath<1>!", 0, ch, o, 0, TO_CHAR);
  }

  ch->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES, false);
}

int doHarm(TBeing* ch, TBeing* vict, TObj* o) {
  int rc = 0;

  // Get char level to control strength of harm
  int charLevel = ch->GetMaxLevel();

  // Dam is random between 1 and 2-10 depending on char level
  int dam = ::number(1, max(2, (int)(charLevel / 5)));

  act("$n's $o projects righteous <Y>fury<1> into $N.", 0, ch, o, vict,
    TO_ROOM);
  act("Your $o projects righteous <Y>fury<1> into $N.", 0, ch, o, vict,
    TO_CHAR);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_DRAIN);
  if (rc == -1)
    return DELETE_VICT;
  return true;
}

int deikhanSword(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;

  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)

  if (cmd == CMD_GENERIC_PULSE) {
    if (!::number(0, 29)) {
      doHolyWrath(ch, o);
      return true;
    }

    if (!::number(0, 49)) {
      doHeal(ch, o);
      return true;
    }
    return true;
  }

  if (cmd == CMD_OBJ_HIT && vict) {
    TBaseWeapon* tWeap;
    if (!(tWeap = dynamic_cast<TBaseWeapon*>(o)))
      return false;
    // damageLevel of the weapon (devastator is 52)
    int weaponLevel = tWeap->damageLevel();

    if (!::number(0, 24) && weaponLevel >= 40) {
      doBlind(ch, vict, o);
      return true;
    }

    if (!::number(0, 3)) {
      return doHarm(ch, vict, o);
    }
    return true;
  }

  return false;
}
