#include "comm.h"
#include "obj_base_weapon.h"
#include "being.h"

bool isEvil(TBeing* vict) {

  switch (vict->getRace()) {
    case RACE_DEMON:
    case RACE_BANSHEE:
    case RACE_DEVIL:
    case RACE_UNDEAD:
    case RACE_VAMPIRE:
    case RACE_VAMPIREBAT:
    case RACE_GOLEM:
    case RACE_LYCANTH:
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;

}

// Previously we used item VNUM to control affects of different deikhan swords
// Hoping to acheive similar results using item levels to control which affects
// are active TBaseWeapon::damageLevel() Avenger = 31.75 Vindicator = 40.75
// Devasator = 52.75
//
// Additionally char levels are used to differentiate the strength of affects.
// (No twinked nebies) This seems more fair and also doesn't require a code
// change to add a new deikhan sword

int doHolyLight(TBeing* ch, TObj* o) {
  TBeing* vict = NULL;
  int rc = FALSE;

  if (ch->checkObjUsed(o)) {
    act("You cannot use $p's powers again this soon.", TRUE, ch, o, NULL,
      TO_CHAR, NULL);
    return rc;
  }
  if (!(vict = ch->fight())) {
    act("You cannot use $p's powers unless you are fighting.", TRUE, ch, o,
      NULL, TO_CHAR, NULL);
    return rc;
  }

  ch->addObjUsed(o, 3 * Pulse::UPDATES_PER_MUDHOUR);

  affectedData aff;
  int level = ch->GetMaxLevel();

  aff.type = AFFECT_HOLY_BEAM;
  aff.level = level;
  aff.bitvector = 0;
  aff.location = APPLY_IMMUNITY;
  aff.modifier = IMMUNE_HOLY;
  aff.modifier2 = -(int(level / 2));
  aff.duration = (level / 5) * Pulse::UPDATES_PER_MUDHOUR;

  act(
    "<y>$n's $o glows as $e chants a <z><W>word of "
    "power<1><y>.<1>",
    TRUE, ch, o, NULL, TO_ROOM, NULL);
  act(
    "<y>Your $o glows as you chant the <z><W>word of "
    "power<1><y>.<1>",
    TRUE, ch, o, NULL, TO_CHAR, NULL);

  act(
    "<y>A beam of holy light bursts from $n's $o<y>, and strikes $N<y> full on!<1>",
    TRUE, ch, o, vict, TO_NOTVICT, NULL);
  act("<y>A beam of holy light bursts from $n's $o<y>, and strikes you full on!<1>",
    TRUE, ch, o, vict, TO_VICT, NULL);
  act(
    "<y>A beam of holy light bursts from your $o<y>, and strikes $N<y> full on!<1>",
    TRUE, ch, o, vict, TO_CHAR, NULL);

  vict->affectTo(&aff);

  int dam = ::number(10, 60);
  rc = ch->reconcileDamage(vict, dam, DAMAGE_HOLY);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    vict->reformGroup();
    delete vict;
    vict = NULL;
  }
  return TRUE;
}

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
    act("$n glows briefly with an <b>ultramarine hue<1>.", FALSE, ch, NULL, 0,
      TO_ROOM);
    act("You glow briefly with an <b>ultramarine hue<1>.", FALSE, ch, NULL, 0,
      TO_CHAR);

    hp = ch->getSkillDam(ch, SPELL_HEAL, 50, 100);
  } else if (charLevel >= 30) {
    // Vindicator level heals
    colorAct(COLOR_SPELLS, "$n glows briefly with an <p>indigo hue<1>.", FALSE,
      ch, NULL, 0, TO_ROOM);
    colorAct(COLOR_SPELLS, "You glow briefly with an <p>indigo hue<1>.", FALSE,
      ch, NULL, 0, TO_CHAR);

    hp = ch->getSkillDam(ch, SPELL_HEAL_CRITICAL, 50, 100);
  } else {
    // Avenger level heals
    colorAct(COLOR_SPELLS, "$n glows briefly with a <b>blue hue<z>.", FALSE, ch,
      NULL, 0, TO_ROOM);
    colorAct(COLOR_SPELLS, "You glow briefly with a <b>blue hue<z>.", FALSE, ch,
      NULL, 0, TO_CHAR);

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

  act("A searing light shines from $p, blinding $N.", FALSE, ch, o, vict,
    TO_CHAR);
  act("$n shields $s eyes as a searing light shines from $p, blinding $N.",
    FALSE, ch, o, vict, TO_NOTVICT);
  act("The world goes white then black as a searing light shines from $n's $p.",
    FALSE, ch, o, vict, TO_VICT);

  int tDuration = (int)(tWeap->weaponLevel() * Pulse::UPDATES_PER_MUDHOUR);
  saveTypeT tSave = SAVE_NO;

  vict->rawBlind((int)tWeap->weaponLevel(), tDuration, tSave);
}

void doHolyWrath(TBeing* ch, TObj* o, bool evil) {
  int WRATH_MOD = 2;
  affectedData* ch_affected;
  affectedData aff;

  if (evil)
    WRATH_MOD += 3;
  
  // Ensure we apply the right modifier
  if (ch->affectedBySpell(AFFECT_HOLY_WRATH)) {
    for (ch_affected = ch->affected; ch_affected;
         ch_affected = ch_affected->next) {
      if (ch_affected->type == AFFECT_HOLY_WRATH) {
        ch->affectRemove(ch_affected, SILENT_YES);
        break;
      }
    } 
  } else {
    act("$n's $o glows with <y>Holy Wrath<1>!", 0, ch, o, 0, TO_ROOM);
    act("Your $o glows with <y>Holy Wrath<1>!", 0, ch, o, 0, TO_CHAR);
  }

  aff.type = AFFECT_HOLY_WRATH;
  aff.duration = Pulse::TICK * 18;
  aff.location = APPLY_HITROLL;
  aff.modifier = WRATH_MOD;
  aff.bitvector = 0;

  ch->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES, FALSE);
}

int doHarm(TBeing* ch, TBeing* vict, TObj* o) {
  int rc = 0;

  // Get char level to control strength of harm
  int charLevel = ch->GetMaxLevel();

  // 2-6 scales to 11-15
  int dam = ::number(1, 5) + (int)(charLevel / 5);

  act("$n's $o projects righteous <Y>fury<1> into $N.", 0, ch, o, vict,
    TO_ROOM);
  act("Your $o projects righteous <Y>fury<1> into $N.", 0, ch, o, vict,
    TO_CHAR);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_HOLY);
  if (rc == -1)
    return DELETE_VICT;
  return TRUE;
}

// Damage is higher than a standard proc and based on wielder level
int doSmiteEvil(TBeing* ch, TBeing* vict, TObj* o) {
  int rc = 0;

  if (!isEvil(vict))
    return FALSE;

  // Get char level to control strengthd of harm
  int charLevel = ch->GetMaxLevel();

  // 5-20 scales up to 14-29
  int dam = ::number(4, 19) + (int)(charLevel / 5);

  if (ch->hasClass(CLASS_DEIKHAN))
    dam += 3;
  if (ch->hasClass(CLASS_CLERIC))
    dam += 2;

  act("<W>$n's $o tears into $N <W>with holy <Y>fury<1>.", 0, ch, o, vict,
    TO_ROOM);
  act("<W>Your $o tears into $N <W>with holy <Y>fury<1>.", 0, ch, o, vict,
    TO_CHAR);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_HOLY);
  if (rc == -1)
    return DELETE_VICT;
  return TRUE;
}

int deikhanSword(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  TBeing* ch;
  TBeing* victim;

  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return FALSE;  // weapon not equipped (carried or on ground)

  if (!(ch->hasClass(CLASS_DEIKHAN)))
    return FALSE;
  
  if (cmd == CMD_GENERIC_PULSE) {

    if (!::number(0, 49)) {
      doHeal(ch, o);
      return TRUE;
    }

    if ( (victim = ch->fight()) ) {
      doHolyWrath(ch, o, isEvil(victim));
      return TRUE;
    }

    return FALSE;
  }

  if (cmd == CMD_OBJ_HIT && vict) {
    TBaseWeapon* tWeap;
    if (!(tWeap = dynamic_cast<TBaseWeapon*>(o)))
      return FALSE;

    if (!::number(0, 3)) {
      if (!::number(0, 2) && isEvil(vict))
        return doSmiteEvil(ch, vict, o);
      return doHarm(ch, vict, o);
    }
    return TRUE;
  }

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    sstring buf, buf2;
    buf = sstring(arg).word(0);
    buf2 = sstring(arg).word(1);

    if (buf == "holy" && buf2 == "light")
      return doHolyLight(ch, o);
  
  }
  return FALSE;
}

int demonSlayer(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;

  ch = genericWeaponProcCheck(vict, cmd, o, 4);
  if (!ch)
    return FALSE;
  
  return doSmiteEvil(ch, vict, o);
}

