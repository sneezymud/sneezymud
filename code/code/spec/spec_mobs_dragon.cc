#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "monster.h"
#include "disease.h"
#include "obj_base_clothing.h"

struct Breath {
    spellNumT dam_type;
    const char* to_notvict;
    const char* to_char;
    const char* to_vict;

    int engulfBeing(TBeing* vict);
    void engulfRoom(TBeing* ch);
    int attack(TBeing* attacker, TBeing* victim, int lag);
};

Breath frost_breath = {
  SPELL_FROST_BREATH,
  "$N is blasted by <C>jets of frost<z> from $n's breath!",
  "You deep-freeze $N!",
  "$n's breath is <C>FREEZING!!!<z>",
};

Breath fire_breath = {
  SPELL_FIRE_BREATH,
  "$N is engulfed in <R>searing flames<z> from $n's breath!",
  "Stick a fork in $N, $E's done!",
  "$n's breath incinerates you with a <R>blast of FIRE!!!<z>\a",
};

Breath acid_breath = {
  SPELL_ACID_BREATH,
  "$N is burned by the <G>noxious acidic fumes<z> from $n's breath!",
  "You do a litmus test on $N!",
  "<G>Noxious acidic fumes<z> from $n's breath surround you!!!",
};

Breath chlorine_breath = {
  SPELL_CHLORINE_BREATH,
  "$N is immersed in a <Y>cloud of chlorine gas<z> from $n's breath!",
  "You burp a smelly one at $N!",
  "You choke on the <Y>toxic clouds<z> of $n's breath!!!",
};

Breath lightning_breath = {
  SPELL_LIGHTNING_BREATH,
  "$N is struck by a crackling <W>bolt of lightning<z> from $n's breath!",
  "You give $N a little jolt!",
  "$n's breath is <W>ELECTRIFYING!!!<z>",
};

Breath dust_breath = {
  SPELL_DUST_BREATH,
  "$N is scoured by <O>gales of dust<z> from $n's breath!",
  "You sandblast $N!",
  "$n's breath <O>sands off<z> all your rough edges!!!",
};

int Breath::engulfBeing(TBeing* vict) {
  switch (dam_type) {
    case SPELL_FROST_BREATH:
      return vict->frostEngulfed();
    case SPELL_FIRE_BREATH:
      return vict->flameEngulfed();
    case SPELL_ACID_BREATH:
      return vict->acidEngulfed();
    case SPELL_CHLORINE_BREATH:
      return vict->chlorineEngulfed();
    case SPELL_LIGHTNING_BREATH:
      return vict->lightningEngulfed();
    case SPELL_DUST_BREATH:
    default:
      return 0;
  }
}

void Breath::engulfRoom(TBeing* ch) {
  switch (dam_type) {
    case SPELL_FROST_BREATH:
      return ch->freezeRoom();
    case SPELL_FIRE_BREATH:
      return ch->flameRoom();
    case SPELL_ACID_BREATH:
      return ch->acidRoom();
    case SPELL_CHLORINE_BREATH:
      return ch->chlorineRoom();
    case SPELL_LIGHTNING_BREATH:
    case SPELL_DUST_BREATH:
    default:
      break;
  }
}

struct Dragon {
    const int vnum;
    Breath& breath;
    const int lag;
};

Dragon dragons[] = {
  {2107, frost_breath, 5}, {3416, acid_breath, 4}, {4796, chlorine_breath, 5},
  {4822, lightning_breath, 2}, {4858, lightning_breath, 6},
  {6843, fire_breath, 5}, {8962, fire_breath, 5}, {10395, fire_breath, 3},
  {10601, lightning_breath, 6}, {11805, fire_breath, 3},
  {12401, frost_breath, 7}, {12403, fire_breath, 7}, {12404, acid_breath, 7},
  {12405, frost_breath, 7}, {14360, frost_breath, 5}, {14361, fire_breath, 3},
  {20400, frost_breath, 2}, {20875, dust_breath, 5}, {22517, frost_breath, 5},
  {23633, lightning_breath, 5}, {27905, lightning_breath, 2},
  {0, fire_breath, 0},  // sentinel
};

Dragon& find_dragon(TBeing* mob) {
  int i = -1;
  while (dragons[++i].vnum)
    if (dragons[i].vnum == mob->mobVnum())
      return dragons[i];
  return dragons[i];
}

// if player has shield, attempt to block breath weapon
int shield_absorb_damage(TBeing* vict, int dam) {
  TThing *left, *right;
  TBaseClothing* shield = NULL;
  wearSlotT slot = WEAR_NOWHERE;

  left = vict->equipment[HOLD_LEFT];
  right = vict->equipment[HOLD_RIGHT];

  TBaseClothing* tbc = NULL;
  if (left && (tbc = dynamic_cast<TBaseClothing*>(left)) && tbc->isShield()) {
    shield = tbc;
    slot = HOLD_LEFT;
  } else if (right && (tbc = dynamic_cast<TBaseClothing*>(right)) &&
             tbc->isShield()) {
    shield = tbc;
    slot = HOLD_RIGHT;
  }
  if (!shield)
    return dam;

  // shield will absorb 10 to 20 percent of its structure
  int shielddam =
    (int)((shield->getMaxStructPoints() / 100.0) * ::number(10, 20));
  dam = max(0, dam - (shielddam * 5));  // each structure point = 5 hp

  act("You hold your $o up to block the blast.", TRUE, vict, shield, 0,
    TO_CHAR);
  act("$n holds $s $o up to block the blast.", TRUE, vict, shield, 0, TO_ROOM);

  bool destroyed = shielddam >= shield->getStructPoints();

  sstring msg =
    format("$p %s blocks the blast %s") % (dam ? "partially" : "completely") %
    (destroyed ? "but is utterly destroyed!" : "and is seriously damaged.");

  act(msg, TRUE, vict, shield, 0, TO_CHAR);
  act(msg, TRUE, vict, shield, 0, TO_ROOM);

  if (!(vict->roomp && vict->roomp->isRoomFlag(ROOM_ARENA))) {
    if (destroyed) {
      vict->unequip(slot);
      if (!shield->makeScraps())
        delete shield;
      shield = NULL;
    } else {
      shield->addToStructPoints(-shielddam);
    }
  }

  return dam;
}

// returns DELETE_VICT
int Breath::attack(TBeing* attacker, TBeing* victim, int lag) {
  // This is pretty arbitrary, but keep an eye toward matching what
  // skillDam is going to do as this is effectively just a very special attack
  // let damage be 0.5 * lev * rnds of lag
  int dam = (int)(0.5 * attacker->GetMaxLevel() * lag);

  // slight randomization
  int random = (int)(0.20 * dam);
  dam += ::number(-random, random);

  act(to_notvict, TRUE, attacker, NULL, victim, TO_NOTVICT);
  act(to_char, TRUE, attacker, NULL, victim, TO_CHAR);
  act(to_vict, TRUE, attacker, NULL, victim, TO_VICT);

  if (!(dam = shield_absorb_damage(victim, dam)))
    return 1;

  attacker->reconcileHurt(victim, 0.1);

  int rc = engulfBeing(victim);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  if (attacker->reconcileDamage(victim, dam, dam_type) == -1)
    return DELETE_VICT;

  return 1;
}

int DragonBreath(TBeing*, cmdTypeT cmd, const char*, TMonster* myself, TObj*) {
  if (!myself || (cmd != CMD_MOB_COMBAT))
    return FALSE;
  if (!myself->fight() || !myself->fight()->sameRoom(*myself))
    return FALSE;
  if (!myself->awake())
    return FALSE;
  if (myself->getWait() > 0)
    return FALSE;

  Dragon& dragon = find_dragon(myself);

  if (!dragon.vnum) {
    // in general, this is bad, but dumn builders often "test"
    if (myself->number == -1)
      vlogf(LOG_LOW,
        format(
          "Dragon (%s:%d) trying to breathe in room %d and not hard coded.") %
          myself->getName() % myself->mobVnum() % myself->inRoom());
    else
      vlogf(LOG_BUG,
        format("Dragon has no defined breath. (%d)") % myself->mobVnum());
    return FALSE;
  }

  if (myself->hasDisease(DISEASE_DROWNING) ||
      myself->hasDisease(DISEASE_GARROTTE) ||
      myself->hasDisease(DISEASE_SUFFOCATE)) {
    myself->sendTo(
      "ACK!!  Your present situation prevents you from breathing.\n\r");
    act("$n rears back...", 1, myself, 0, 0, TO_ROOM);
    act("Thank the deities $e is unable to breathe.", 1, myself, 0, 0, TO_ROOM);
    return FALSE;
  }

  act("$n rears back and inhales...", 1, myself, 0, 0, TO_ROOM);
  myself->addToWait(combatRound(1) * dragon.lag);

  // have all the mobs in the room try to run for it!
  // copy stuff to avoid modifying the list while iterating over it
  auto stuff = myself->roomp->stuff;
  for (StuffIter it = stuff.begin(); it != stuff.end(); ++it) {
    TMonster* tm = dynamic_cast<TMonster*>(*it);
    if (tm && tm != myself && tm->canSee(myself) && ::number(0, 9))
      tm->doFlee("");
  }

  std::queue<TBeing*> killed;
  for (StuffIter it = myself->roomp->stuff.begin();
       it != myself->roomp->stuff.end(); ++it) {
    TBeing* tmp = dynamic_cast<TBeing*>(*it);
    if (!tmp || tmp == myself)
      continue;
    int rc = dragon.breath.attack(myself, tmp, dragon.lag);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      killed.push(tmp);
  }

  while (!killed.empty()) {
    delete killed.front();
    killed.pop();
  }

  dragon.breath.engulfRoom(myself);

  return TRUE;
}

void TBeing::doBreath(const char* argument) {
  char buf[256];
  Breath breath;
  TBeing* vict;

  if (hasDisease(DISEASE_DROWNING) || hasDisease(DISEASE_GARROTTE) ||
      hasDisease(DISEASE_SUFFOCATE)) {
    sendTo("ACK!!  Your present situation prevents you from breathing.\n\r");
    return;
  }

  if (!(isImmortal()) || !isPc()) {
    sendTo("You inhale and exhale deeply.  Feels good!\n\r");
    sendTo("Fortunately, your brain handles this for you automatically.\n\r");
    return;
  }

  if (!hasWizPower(POWER_BREATHE)) {
    sendTo("You lack the power to breathe dragonbreath.\n\r");
    return;
  }

  argument = one_argument(argument, buf, cElements(buf));
  if (!*buf) {
    sendTo(
      "Syntax: breathe <acid | fire | frost | lightning | chlorine> "
      "<victim>\n\r");
    return;
  }

  if (is_abbrev(buf, "acid")) {
    breath = acid_breath;
  } else if (is_abbrev(buf, "fire")) {
    breath = fire_breath;
  } else if (is_abbrev(buf, "frost")) {
    breath = frost_breath;
  } else if (is_abbrev(buf, "lightning")) {
    breath = lightning_breath;
  } else if (is_abbrev(buf, "chlorine")) {
    breath = chlorine_breath;
  } else if (is_abbrev(buf, "dust")) {
    breath = dust_breath;
  } else {
    sendTo(
      "Syntax: breathe <acid | fire | frost | lightning | chlorine> "
      "<victim>\n\r");
    return;
  }

  argument = one_argument(argument, buf, cElements(buf));
  if (!*buf) {
    sendTo("Breathe on whom?\n\r");
    return;
  }

  if (!(vict = get_char_room_vis(this, buf))) {
    sendTo("I don't see anyone here like that.\n\r");
    return;
  }

  if (vict == this) {
    sendTo("You sure must be bored to do something that dumb.\n\r");
    return;
  }

  act("You inhale deeply and turn to face $N...", TRUE, this, 0, vict, TO_CHAR);
  act("$n inhales deeply and turns in your direction...", TRUE, this, 0, vict,
    TO_VICT);
  act("$n inhales deeply and turns to face $N...", TRUE, this, 0, vict,
    TO_NOTVICT);

  act("You exhale forcefully...", TRUE, this, 0, vict, TO_CHAR);
  act("$e exhales forcefully...", TRUE, this, 0, vict, TO_ROOM);

  int rc = breath.attack(this, vict, 1);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete vict;
    vict = NULL;
  }

  breath.engulfRoom(this);
}
