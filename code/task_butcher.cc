#include "stdsneezy.h"

void stop_butcher(TBeing *ch)
{
  if (ch->getPosition() >= POSITION_RESTING) {
    act("You stop butchering, and look about confused.",
            FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops butchering, and looks about confused and embarrassed.",
            FALSE, ch, 0, 0, TO_ROOM);
  }
  ch->stopTask();
}

int TThing::butcherPulse(TBeing *ch, TBaseCorpse *corpse)
{
  TThing *Tobj;
  TObj *steak = NULL;
  TObj *item;
  TBaseWeapon *tobj;
  int  learning = ch->getSkillValue(SKILL_BUTCHER),
    Ceffect = (corpse->isCorpseFlag(CORPSE_HALF_BUTCHERED)?2:1),
    maxUnitsP = max(1, (int)((corpse->getWeight()*.10)/2)-1);
  char buf[256];
  //  char msg   [256],
  //       gl_msg[256];

  if (ch->isImmortal()) {
    ch->sendTo("You instantly carve the corpse into steaks in a god-like manner.\n\r");
    act("$n becomes a blur and instantly butchers $p.",
        FALSE, ch, corpse, 0, TO_ROOM);
    ch->task->flags = (int)(maxUnitsP/Ceffect);
  }
  Tobj = ch->heldInPrimHand();
  tobj = dynamic_cast<TBaseWeapon *>(Tobj);


  if ((Tobj && (Tobj->isPierceWeapon() || Tobj->isSlashWeapon())) &&
      (ch->task->flags < (int)(maxUnitsP/Ceffect)) &&
      ch->task->timeLeft > 0) {
    act("You continue to butcher $p.",
        FALSE, ch, corpse, 0, TO_CHAR);
    act("$n slices the steak from $p very carefully.",
        FALSE, ch, corpse, NULL, TO_ROOM);
    ch->task->timeLeft--;

    if (!bSuccess(ch, learning, SKILL_BUTCHER)) {
      CF(SKILL_BUTCHER);
      if (bSuccess(ch, learning, SKILL_BUTCHER)) {
        act("You gently over extend yourself and slightly dull your weapon.",
            FALSE, ch, 0, 0, TO_CHAR);
        if (tobj->getCurSharp() > 2)tobj->addToCurSharp(-1);
        ch->task->flags++;
      } else if (!critFail(ch, SKILL_BUTCHER)) {
        act("You extend yourself a little too far and dull your weapon.",
            FALSE, ch, 0, 0, TO_CHAR);
        if (tobj->getCurSharp() > 3)tobj->addToCurSharp(-2);
      } else {
        act("You really slip up and cut yourself and mangle the steak.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$n really slips up and mangles part of the steak and part of $mself",
            FALSE, ch, 0, 0, TO_ROOM);
        if (tobj->getCurSharp() > 4)tobj->addToCurSharp(-3);
        ch->dropBloodLimb((ch->isRightHanded() ? WEAR_FINGER_L : WEAR_FINGER_R));
        if (ch->reconcileDamage(ch, 5+(min(20, tobj->getCurSharp())/2), SKILL_BUTCHER) == -1) {
          ch->stopTask();
          ch->doSave(SILENT_YES);
          if (corpse->isCorpseFlag(CORPSE_PC_BUTCHERING))
            corpse->remCorpseFlag(CORPSE_PC_BUTCHERING);
          return DELETE_THIS;
        }
      }
    } else {
      CS(SKILL_BUTCHER);
      ch->task->flags += max(1, (int) (learning/25));
      corpse->obj_flags.decay_time++;
      if (tobj->getCurSharp() > 2 && (ch->task->flags % 10) == 0)
        tobj->addToCurSharp(-1);
      ch->task->flags = max(1, min(ch->task->flags, maxUnitsP));
    }
    return FALSE;
  } else {
    if (!Tobj || (!Tobj->isPierceWeapon() && !Tobj->isSlashWeapon())) {
      act("Hey, where'd your weapon go?",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n looks blankly at $s empty hand.",
          FALSE, ch, 0, NULL, TO_ROOM);
      act("With the lack of a weapon, you grab the loose flesh and rip.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n grabs the loose steak and rips it off the carcas.",
          FALSE, ch, 0, NULL, TO_ROOM);
      if (ch->task->flags > 0)
        ch->task->flags--;
    } else if (ch->task->timeLeft <= 0 && ch->task->timeLeft != -1) {
      act("You don't feel as if you could butcher any longer.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n seems as if $e couldn't go on.",
          FALSE, ch, 0, NULL, TO_ROOM);
    } else if (ch->task->timeLeft != -1) {
      act("You feel confident that you got all the meat off of this one.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n looks up with a sense of pride in $s work.",
          FALSE, ch, 0, NULL, TO_ROOM);

    } else if (ch->task->flags == 0) {
      act("You finish up your butchering and realize you have mangled the carcas.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n finishes up and has apparently destroyed the carcas.",
          FALSE, ch, 0, NULL, TO_ROOM);
      ch->stopTask();
      return FALSE;
    } else if (ch->task->timeLeft != -1) {
      act("Something happened that wasn't expected, Tell a god what you did.",
          FALSE, ch, 0, 0, TO_CHAR);
      ch->stopTask();
      return FALSE;
    }
  }

  int totalUnits = ch->task->flags;
  ch->stopTask();
  if (!corpse->isCorpseFlag(CORPSE_HALF_BUTCHERED) && totalUnits <= (int)(maxUnitsP / 2))
    corpse->addCorpseFlag(CORPSE_HALF_BUTCHERED);
  else
    corpse->addCorpseFlag(CORPSE_NO_BUTCHER);
  if (corpse->isCorpseFlag(CORPSE_PC_BUTCHERING))
    corpse->remCorpseFlag(CORPSE_PC_BUTCHERING);

  //  act(   msg, FALSE, ch, item, corpse, TO_CHAR);
  //  act(gl_msg, FALSE, ch, item, corpse, TO_ROOM);

#ifdef WEIGHT
  if (compareWeights(item->getTotalWeight(TRUE),
		     (ch->carryWeightLimit() - ch->getCarriedWeight())) == -1) {
    act("The weight of the meat gets too much for you, so you drop it.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("The weight of $s meat gets to be too much for $n to handle.",
        FALSE, ch, 0, NULL, TO_ROOM);
    //    *corpse->roomp += *item;
  } else if (ch->getCarriedVolume() + (item->getTotalVolume()
				       -item->getReducedVolume(NULL)) > ch->carryVolumeLimit()) {
    act("You struggle to hold onto the steak but it slips from your grasp.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("$n struggles to hold onto the steak but it slips from $s grasp.",
        FALSE, ch, 0, NULL, TO_ROOM);
    //    *corpse->roomp += *item;
  }// else *ch += *item;
#endif

  if(!steak){
#if 1
    // builder port uses stripped down database which was causing problems
    // hence this setup instead.
    int robj = real_object(GENERIC_STEAK);
    if (robj < 0 || robj >= (signed int) obj_index.size()) {
      vlogf(LOG_BUG, "butcherPulse: No object (%d) in database!", GENERIC_STEAK);
      return false;
    }
    item = read_object(robj, REAL);
#else
    item = read_object(GENERIC_STEAK, VIRTUAL);
#endif
    steak = dynamic_cast<TObj *>(item);
    if (!steak)
      return false;
    steak->swapToStrung();
    steak->canBeSeen = 1;

    if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 2) {
      sprintf(buf, "steak elf elven");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of elven meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 3) {
      sprintf(buf, "steak dwarven dwarf");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of dwarven meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 4) {
      sprintf(buf, "steak hobbit");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of hobbit meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 5) {
      sprintf(buf, "filet gnome mignon");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a filet of gnome mignon");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 6) {
      sprintf(buf, "slab meat ogre");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a slab of ogre meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 7) {
      sprintf(buf, "filet pegasus");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a rare filet of pegasus");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 8) {
      sprintf(buf, "steak lycanth");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of lycanth meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 9) {
      sprintf(buf, "steak dragon");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of fresh dragon meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 10) {
      sprintf(buf, "meat grey undead");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "some decaying grey meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 11) {
      sprintf(buf, "steak orcish orc");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of orc meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 12) {
      sprintf(buf, "insect bug slice");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a slice of bug stuff");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 13) {
      sprintf(buf, "chunk arachnid meat");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a chunk of arachnid meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 14) {
      sprintf(buf, "steak dinosaur");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of dinosaur meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 15) {
      sprintf(buf, "filet fish");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a fish filet");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 16) {
      sprintf(buf, "bird piece");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a piece of bird meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 17) {
      sprintf(buf, "steak giant flesh");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of a giant's flesh");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 18) {
      sprintf(buf, "steak birdman");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak from a birdman");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 19) {
      sprintf(buf, "steak parasite");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a tiny steak of parasite meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 20) {
      sprintf(buf, "slice slime");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a slice of slime");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 21) {
      sprintf(buf, "steak demon flesh");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of demon's flesh");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 22) {
      sprintf(buf, "steak snake");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of snake meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 23) {
      sprintf(buf, "steak hippopotomus");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of hippo meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 24) {
      sprintf(buf, "chunk bark tree");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a chunk of edible tree bark");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 25) {
      sprintf(buf, "veggie burger");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a veggie burger");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 26) {
      sprintf(buf, "steak element");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of elemental meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 27) {
      sprintf(buf, "piece ant");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a piece of ant meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 28) {
      sprintf(buf, "flesh piece devil");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a piece of devil's flesh");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 29) {
      sprintf(buf, "steak frogman");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of frogman meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 30) {
      sprintf(buf, "steak goblin");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of goblin meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 31) {
      sprintf(buf, "steak troll");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of troll meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 32) {
      sprintf(buf, "flesh angel piece");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a piece of angel's flesh");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 33) {
      sprintf(buf, "steak mindflayer");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of mindflayer meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 34) {
      sprintf(buf, "steak primate");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of primate meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);

#ifdef JES
      // number and change along the way to the list in race.h

    } else if (corpse->getCorpseRace() == 35) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 36) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 37) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {
      sprintf(buf, "steak human");
      delete [] steak->name;
      steak->name = mud_str_dup(buf);
      sprintf(buf, "a steak of human meat");
      delete [] steak->shortDescr;
      steak->shortDescr = mud_str_dup(buf);
    } else if (corpse->getCorpseRace() == 1) {



#endif
    } else {
    sprintf(buf, "steak generic");
    delete [] steak->name;
    steak->name = mud_str_dup(buf);
    sprintf(buf, "a steak");
    delete [] steak->shortDescr;
    steak->shortDescr = mud_str_dup(buf);
    }
    *ch += *steak;
  }
  return FALSE;
}

int TTool::butcherPulse(TBeing *ch, TBaseCorpse *corpse)
{
  int  learning = ch->getSkillValue(SKILL_BUTCHER);
    //       amount,
    //       num;
    //  TObj *item;
  //  char msg   [256],
  //       gl_msg[256];


  if (getToolType() != TOOL_BUTCHER_KNIFE) {
    act("Hey, where'd your knife go?", FALSE, ch, 0, 0, TO_CHAR);
    stop_butcher(ch);
    return FALSE;
  }

  if (ch->task->timeLeft > 0) {
    act("You continue to butcher $p.", FALSE, ch, corpse, 0, TO_CHAR);
    act("$n slices a steak from $p very carefully.",
          FALSE, ch, corpse, NULL, TO_ROOM);

    addToToolUses(-1);
    if (getToolUses() <= 0) {
      act("Your $o breaks due to overuse.", FALSE, ch, this, 0, TO_CHAR);
      act("$n looks startled as $e breaks $P while butchering.", FALSE, ch, 0, this, TO_ROOM);
      ch->stopTask();
      delete this;
      return FALSE;
    }

    if (!bSuccess(ch, learning, SKILL_BUTCHER)) {
      if (!bSuccess(ch, learning, SKILL_BUTCHER)) {
        // a doubele failure
        CF(SKILL_BUTCHER);  // failure
        act("You slip up and destory a part of the carcas.",
             FALSE, ch, 0, 0, TO_CHAR);
        act("$n really slips up and mangles part of the carcas.",
             FALSE, ch, 0, 0, TO_ROOM);
        corpse->addCorpseFlag(CORPSE_NO_BUTCHER);
        ch->stopTask();
      }
    } else {
      ch->task->timeLeft--;
    }
  } else {
    CS(SKILL_BUTCHER);  // success
    ch->stopTask();
    corpse->addCorpseFlag(CORPSE_NO_BUTCHER);
#ifdef ITEM
    amount = amount/10 + (amount%10 ? 1 : 0);  // each "unit" weighs 1/10 lb.
    item->setWeight(amount);
    item->obj_flags.cost *= amount;

    act(msg, FALSE, ch, item, corpse, TO_CHAR);
    act(gl_msg, FALSE, ch, item, corpse, TO_ROOM);

    *ch += *item;
#endif
  }

  return FALSE;
}

int task_butchering(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *obj)
{
  TThing *knife;
  TBaseCorpse *corpse = NULL;
  int rc;

  // sanity check
  if (ch->isLinkdead() || (ch->in_room < 0) ||
      (ch->getPosition() < POSITION_RESTING)) {
    stop_butcher(ch);
    return FALSE;  // returning FALSE lets command be interpreted
  }
  if (ch->utilityTaskCommand(cmd) ||
      ch->nobrainerTaskCommand(cmd))
    return FALSE;

  if (!obj || !ch->sameRoom(*obj) ||
      !(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    act("Hey, where'd that corpse go?", FALSE, ch, 0, 0, TO_CHAR);
    stop_butcher(ch);
    return FALSE;  // returning FALSE lets command be interpreted
  }
  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // a body part or something
    act("$p: You are not able to butcher that.",
          FALSE, ch, corpse, 0, TO_CHAR);
    return FALSE;
  }
  if (corpse->isCorpseFlag(CORPSE_NO_BUTCHER)) {
    act("Something unfortunate has happened to $p and it can't be butchered",
              FALSE, ch, corpse, 0, TO_CHAR);
    stop_butcher(ch);
    return FALSE;  // returning FALSE lets command be interpreted
  }

  switch (cmd) {
  case CMD_TASK_CONTINUE:
    if (!(knife = ch->heldInPrimHand())) {
        act("Hey, where'd your knife go?", FALSE, ch, 0, 0, TO_CHAR);
        stop_butcher(ch);
        return FALSE;  // returning FALSE lets command be interpreted
    }
    // each pulse is constatnt duration apart
    // the # of pulses was set in start_task as based on skill
      ch->task->calcNextUpdate(pulse, PULSE_MOBACT);
      rc = knife->butcherPulse(ch, corpse);
      // knife may be invalid here
      return rc;
  case CMD_ABORT:
  case CMD_STOP:
      if (!(knife = ch->heldInPrimHand())) {
        act("You go to stop butchering, but your knife is gone!",
            FALSE, ch, 0, 0, TO_CHAR);
        act("You grab the loose steak and tear it the rest of the way off.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$n grabs the loose steak and tears it the rest of the way off.",
            TRUE, ch, 0, 0, TO_ROOM);
      } else {
        act("You make the final slice and stop butchering.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$n makes the final slice and stops butchering.",
            TRUE, ch, 0, 0, TO_ROOM);
      }
      ch->task->timeLeft=-1;
      rc = knife->butcherPulse(ch, corpse);
      return rc;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue butchering while under attack!\n\r");
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}
