//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "egotrip.cc" - The egotrip command
//  
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "disc_shaman.h"
#include "disc_shaman_healing.h"
#include "disc_fire.h"
#include "disc_wrath.h"
#include "disc_aegis.h"
#include "disease.h"
#include "obj_portal.h"

void TBeing::doEgoTrip(const char *arg)
{
  if (powerCheck(POWER_EGOTRIP))
    return;

  if (isPc() && !isImmortal()) {
    incorrectCommand();
    return;
  }

  sstring badsyn;
  badsyn  = "Syntax: egotrip <command>\n\r";
  badsyn += "deity - forces a global MOB_ALIGN_PULSE for checkSpec\n\r";
  badsyn += "bless - blesses all players\n\r";
  badsyn += "blast - removes 50% hp from one player\n\r";
  badsyn += "damn - curses and faerie fires one player\n\r";
  badsyn += "hate - causes all mobs in the room to hate one player\n\r";
  badsyn += "cleanse - removes all disease in the world\n\r";
  badsyn += "wander - forces a wander pulse for all mobs in the room\n\r";
  badsyn += "stupidity - casts the stupidity spell on all players\n\r";
  badsyn += "sanctuary - casts the sanctuary prayer on all players\n\r";
  badsyn += "enliven - casts the enliven spell on all players\n\r";
  badsyn += "crit - forces a target mob to do the number crit if fighting\n\r";
  badsyn += "portal - creates a portal to the target mob/player\n\r";
  badsyn += "teleport - teleports the targeted mob/player, ignoring room flags\n\r";

  sstring argument, sarg = arg, restarg;
  restarg = one_argument(sarg, argument);
  if (!argument.length()) {
    sendTo(badsyn);
    return;
  }
  if(is_abbrev(argument, "teleport")){
    sstring target, buf;
    restarg = one_argument(restarg, target);
    if (target.empty()) {
      sendTo("Syntax: egotrip teleport <target>\n\r");
      return;
    }
    TBeing *ch = get_char_vis_world(this, target, NULL, EXACT_NO);
    if (!ch) {
      sendTo("Could not locate character.\n\r");
      sendTo("Syntax: egotrip teleport <target>\n\r");
      return;
    }
    
    sendTo(fmt("You teleport %s\n\r") % ch->getName());
    
    ch->genericTeleport(SILENT_NO, false, true);
  } else if(is_abbrev(argument, "portal")){
    sstring target, buf;
    restarg = one_argument(restarg, target);
    if (target.empty()) {
      sendTo("Syntax: egotrip portal <target>\n\r");
      return;
    }
    TBeing *ch = get_char_vis_world(this, target, NULL, EXACT_NO);
    if (!ch) {
      sendTo("Could not locate character.\n\r");
      sendTo("Syntax: egotrip portal <target>\n\r");
      return;
    }

    TPortal * tmp_obj = new TPortal(ch->roomp);
    *roomp += *tmp_obj;
    roomp->playsound(SOUND_SPELL_PORTAL, SOUND_TYPE_MAGIC);

    TPortal * next_tmp_obj = new TPortal(roomp);
    *ch->roomp += *next_tmp_obj;
    ch->roomp->playsound(SOUND_SPELL_PORTAL, SOUND_TYPE_MAGIC);

    act("$p suddenly appears out of a swirling mist.", TRUE, this, tmp_obj, NULL, TO_ROOM);
    act("$p suddenly appears out of a swirling mist.", TRUE, this, tmp_obj, NULL, TO_CHAR);

    sendToRoom((fmt("%s suddenly appears out of a swirling mist.") %
		sstring(next_tmp_obj->shortDescr).cap()).c_str(),
	       next_tmp_obj->roomp->number);

    return;
  } else if (is_abbrev(argument, "deity")) {
    sendTo("Forcing a deity pulse to occur.\n\r");
    TBeing *ch, *ch2;
    bool found = false;
    for (ch = character_list; ch; ch = ch2) {
      ch2 = ch->next;
      TMonster *tmons = dynamic_cast<TMonster *>(ch);
      if (!tmons)
        continue;
      if (!tmons->spec) 
        continue;
      int rc = tmons->checkSpec(tmons, CMD_MOB_ALIGN_PULSE, "", NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT)) {
        delete tmons;
        tmons = NULL;
      }
      if (rc)
        found = true;
    }
    vlogf(LOG_MISC, fmt("%s egotripped deities") %  getName());
    if (!found)
      sendTo("No deities in The World.\n\r");
    return;
    // deities
  } else if (is_abbrev(argument, "bless")) {
    if (!isImmortal() || !desc || !IS_SET(desc->autobits, AUTO_SUCCESS)) {
      sendTo("You must be immortal, and have auto-success enabled first.\n\r");
      return;
    }

    vlogf(LOG_MISC, fmt("%s egotripped bless") %  getName());
    Descriptor *d;
    for (d = descriptor_list; d; d = d->next) {
      if (d->connected != CON_PLYNG)
        continue;

      TBeing *ch = d->character;

      // Try and ditch some of the un-needed spam/waste.
      if (!ch || ch->GetMaxLevel() > MAX_MORT)
        continue;
#if 0
// doesn't work if not in room
      act("$N has graciously bestowned upon you $S blessing.",
            FALSE, ch, 0, this, TO_CHAR);
#else
      ch->sendTo(fmt("%s has graciously bestowed upon you %s blessing.\n\r") %
            sstring(ch->pers(this)).cap() % hshr());
#endif
      bless(this, ch);
    }
    return;
  } else if (is_abbrev(argument, "sanctuary")) {
    if (!isImmortal() || !desc || !IS_SET(desc->autobits, AUTO_SUCCESS)) {
      sendTo("You must be immortal, and have auto-success enabled first.\n\r");
      return;
    }

    vlogf(LOG_MISC, fmt("%s egotripped sanctuary") %  getName());
    Descriptor *d;
    for (d = descriptor_list; d; d = d->next) {
      if (d->connected != CON_PLYNG)
        continue;

      TBeing *ch = d->character;

      // Try and ditch some of the un-needed spam/waste.
      if (!ch || ch->GetMaxLevel() > MAX_MORT)
        continue;
#if 0
// doesn't work if not in room
      act("$N has given you sanctuary.",
            FALSE, ch, 0, this, TO_CHAR);
#else
      ch->sendTo(fmt("%s has given you sanctuary.\n\r") %
            sstring(ch->pers(this)).cap());
#endif
      sanctuary(this, ch);
    }
    return;
  } else if (is_abbrev(argument, "stupidity")) {
    if (!isImmortal() || !desc || !IS_SET(desc->autobits, AUTO_SUCCESS)) {
      sendTo("You must be immortal, and have auto-success enabled first.\n\r");
      return;
    }

    vlogf(LOG_MISC, fmt("%s is egotrippin and now everyone is stupid") %  getName());
    Descriptor *d;
    for (d = descriptor_list; d; d = d->next) {
      if (d->connected != CON_PLYNG)
        continue;

      TBeing *ch = d->character;

      // Try and ditch some of the un-needed spam/waste.
      if (!ch || ch->GetMaxLevel() > MAX_MORT)
        continue;
      ch->sendTo(fmt("%s has reconfirmed %s suspicions.\n\r") %
            sstring(ch->pers(this)).cap() % hshr());
      castStupidity(this, ch);
    }
    return;
  } else if (is_abbrev(argument, "enliven")) {
    if (!isImmortal() || !desc || !IS_SET(desc->autobits, AUTO_SUCCESS)) {
      sendTo("You must be immortal, and have auto-success enabled first.\n\r");
      return;
    }

    vlogf(LOG_MISC, fmt("%s has enlivened the game") %  getName());
    Descriptor *d;
    for (d = descriptor_list; d; d = d->next) {
      if (d->connected != CON_PLYNG)
        continue;

      TBeing *ch = d->character;

      // Try and ditch some of the un-needed spam/waste.
      if (!ch || ch->GetMaxLevel() > MAX_MORT)
        continue;
      ch->sendTo(fmt("%s has bestowed upon you enlivenment.\n\r") %
            sstring(ch->pers(this)).cap());
      castEnliven(this, ch);
    }
    return;
  } else if (is_abbrev(argument, "crit")) {
    sstring target;
    restarg = one_argument(restarg, target);
    if (target.empty()) {
      sendTo("Syntax: egotrip crit <target> <crit>\n\r");
      return;
    }
    TBeing *ch = get_char_vis_world(this, target, NULL, EXACT_NO);
    if (!ch) {
      sendTo("Could not locate character.\n\r");
      sendTo("Syntax: egotrip crit <target> <crit>\n\r");
      return;
    }

    if (ch->isImmortal()) {
      sendTo("Do this to an immortal??? Bad god, no bone!\n\r");
      return;
    }
    sstring whichcrit;
    one_argument(restarg, whichcrit);
    if (whichcrit.empty()) {
      sendTo("Syntax: egotrip crit <target> <crit>\n\r");
      return;
    }
    int crit = convertTo<int>(whichcrit);
    if (crit > 100 || crit < 1) {
      sendTo("Crit is outside of range. Must be 1-100.\n\r");
      sendTo("Syntax: egotrip crit <target> <crit>\n\r");
      return;
    }
    affectedData aff;
    aff.type = AFFECT_DUMMY;
    aff.level = 60;
    aff.duration = 5;
    aff.modifier2 = crit;
    ch->affectTo(&aff);
    sendTo(fmt("It looks like some bad luck will befall %s before too long. Heh, heh, heh.\n\r") %ch->getName());
    vlogf(LOG_MISC, fmt("%s egotrip critted %s with crit #%d") %  getName() % ch->getName() % crit);
    return;
  } else if (is_abbrev(argument, "blast")) {
    sstring target;
    one_argument(restarg, target);
    if (target.empty()) {
      sendTo("Syntax: egotrip blast <target>\n\r");
      return;
    }
    TBeing *ch = get_char_vis_world(this, target, NULL, EXACT_NO);
    if (!ch) {
      sendTo("Could not locate character.\n\r");
      sendTo("Syntax: egotrip blast <target>\n\r");
      return;
    }
    vlogf(LOG_MISC, fmt("%s egotrip blasted %s") %  getName() % ch->getName());
    if (ch->isPc() && ch->isImmortal() &&
        ch->GetMaxLevel() > GetMaxLevel()) {
      sendTo("Shame Shame, you shouldn't do that.\n\r");
      return;
    }
    act("You blast $N with a bolt of lightning.",
         FALSE, this, 0, ch, TO_CHAR);

    act("A bolt of lightning streaks down from the heavens right at your feet!",
         FALSE, ch, 0, 0, TO_CHAR);
    act("BZZZZZaaaaaappppp!!!!!",
         FALSE, ch, 0, 0, TO_CHAR);
    act("A bolt of lightning streaks down from the heavens right at $n's feet!",
         FALSE, ch, 0, 0, TO_ROOM);

    sstring worldBuf = "You smell burnt flesh as a bolt of lightning takes the hide off of ";
    worldBuf += ch->getName();
    worldBuf += "!\n\r";
    descriptor_list->worldSend(worldBuf.c_str(), this);

    soundNumT snd = pickRandSound(SOUND_EGOBLAST_1, SOUND_EGOBLAST_2);
    ch->roomp->playsound(snd, SOUND_TYPE_NOISE);

    // this just nails um, but shouldn't actually kill them
    if (ch->reconcileDamage(ch, ch->getHit()/2, DAMAGE_ELECTRIC) == -1) {
      delete ch;
      ch = NULL;
      return;
    }
    ch->setMove(ch->getMove()/2);
    
    return;
  } else if (is_abbrev(argument, "damn")) {
    if (!isImmortal() || !desc || !IS_SET(desc->autobits, AUTO_SUCCESS)) {
      sendTo("You must be immortal, and have auto-success enabled first.\n\r");
      return;
    }

    sstring target;
    one_argument(restarg, target);
    if (target.empty()) {
      sendTo("Syntax: egotrip damn <target>\n\r");
      return;
    }
    TBeing *ch = get_char_vis_world(this, target, NULL, EXACT_NO);
    if (!ch) {
      sendTo("Could not locate character.\n\r");
      sendTo("Syntax: egotrip damn <target>\n\r");
      return;
    }
    
    vlogf(LOG_MISC, fmt("%s egotrip damned %s") %  getName() % ch->getName());
    if (ch->isPc() && ch->isImmortal() &&
        ch->GetMaxLevel() > GetMaxLevel()) {
      sendTo("Shame Shame, you shouldn't do that.\n\r");
      return;
    }
    // at the person, cast the spell.
    TRoom *rp_o = roomp;
    if (ch != this) {
      --(*this);
      *ch->roomp += *this;
    }
    castStupidity(this, ch);
    castFaerieFire(this, ch);
    curse(this, ch);

    if (ch != this) {
      --(*this);
      *rp_o += *this;
    }

    return;
  } else if (is_abbrev(argument, "hate")) {
    sstring target;
    one_argument(restarg, target);
    if (target.empty()) {
      sendTo("Syntax: egotrip hate <target>\n\r");
      return;
    }
    TBeing *ch = get_char_vis_world(this, target, NULL, EXACT_NO);
    if (!ch) {
      sendTo("Could not locate character.\n\r");
      sendTo("Syntax: egotrip hate <target>\n\r");
      return;
    }
    
    TThing *t;
    for (t = roomp->getStuff(); t; t = t->nextThing) {
      TMonster *tmon = dynamic_cast<TMonster *>(t);
      if (!tmon)
        continue;
      if (tmon->Hates(ch, NULL))
        continue;
      tmon->addHated(ch);
      act("$N now hates $p.", false, this, ch, tmon, TO_CHAR);
    }
    return;
  } else if (is_abbrev(argument, "wander")) {
    TThing *t, *t2;
    for (t = roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      TMonster *tmon = dynamic_cast<TMonster *>(t);
      if (!tmon)
        continue;
      if (IS_SET(tmon->specials.act, ACT_SENTINEL)) {
        act("$n is set sentinel.", false, tmon, 0, this, TO_VICT);
        continue;
      }
      int rc = tmon->wanderAround();
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tmon;
        tmon = NULL;
      }
    }
    return;
  } else if (is_abbrev(argument, "cleanse")) {
    if (!isImmortal() || !desc) {
      sendTo("You must be immortal to do this.\n\r");
      return;
    }

    vlogf(LOG_MISC, fmt("%s egotripped cleanse") %  getName());

    TBeing       *tBeing;
    affectedData *tAff = NULL,
                 *tAffLast;

    for (tBeing = character_list; tBeing; tBeing = tBeing->next) {
      if (!tBeing->name) {
        vlogf(LOG_BUG, "Something with NULL name in world being list.");
        continue;
      }

      tAffLast = NULL;

      for (tAff = tBeing->affected; tAff;) {
        if (tAff->type != AFFECT_DISEASE) {
          tAffLast = tAff;
          tAff = tAff->next;
          continue;
        }

        tBeing->sendTo(fmt("%s has cured your %s.\n\r") %
                       sstring(getName()).cap() %
                       DiseaseInfo[affToDisease(*tAff)].name);


	sendTo(COLOR_BASIC, fmt("Your cure %s of: %s.\n\r") %
	       tBeing->getName() %
	       DiseaseInfo[affToDisease(*tAff)].name);

        if (tAff->modifier == DISEASE_POISON) {
          tBeing->affectFrom(SPELL_POISON);
          tBeing->affectFrom(SPELL_POISON_DEIKHAN);
        }

        tBeing->diseaseStop(tAff);
        tBeing->affectRemove(tAff);

        if (tBeing->isPc() && tBeing->desc)
          tBeing->doSave(SILENT_YES);

        if (!tAffLast)
          tAff = tBeing->affected;
        else
          tAff = tAffLast;

        if (!tAff)
          break;
      }
    }

    return;
  } else {
    sendTo(badsyn);
    return;
  }
}
