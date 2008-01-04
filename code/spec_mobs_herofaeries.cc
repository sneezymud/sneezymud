#include "stdsneezy.h"
#include "db.h"
#include "database.h"

#define UNIQUE_TROPHY_FAERIE 33313
#define PERMA_DEATH_FAERIE 33314
#define FAERIE_CASTER_LEVEL 50
#define MAX_TIME 5 // max idle time

// only important that higher is better
// rate players by number of unique creatures killed
int getUniqueTrophyRank(TBeing *targ)
{
  TDatabase db(DB_SNEEZY);

  if (!targ) {
    vlogf(LOG_BUG, "getUniqueTrophyRank entered with null targ");
    return 0;
  }
  db.query("select t.count from trophyplayer t where t.player_id=%i",
      targ->getPlayerID());
  if (!db.isResults())
    return 0;
  if (db.fetchRow()) {
    return convertTo<int>(db["count"]);
  } else
    return 0;
}

void uniqueTrophyIntro(TBeing *faerie, TBeing *targ)
{
  if (!targ) {
    vlogf(LOG_BUG, "uniqueTrophyIntro entered with null targ");
    return;
  }
  if (!faerie) {
    vlogf(LOG_BUG, "uniqueTrophyIntro entered with null faerie");
    return;
  }
  faerie->doTell(targ->getName(),
    "You are greatly respected amongst my people for the breadth of your travels.");
  faerie->doTell(targ->getName(),
    "I have been sent to help you in your endeavors!");
}

// feral wrath, but only the stat part (no AC penalty), stat part is halved
void addUniqueTrophyEffects(TBeing *faerie, TBeing *targ)
{
  affectedData aff2; 
   
  if (!targ) {
    vlogf(LOG_BUG, "addUniqueTrophyEffects entered with null targ");
    return;
  }
  if (!faerie) {
    vlogf(LOG_BUG, "addUniqueTrophyEffects entered with null faerie");
    return;
  }
  int modifier = FAERIE_CASTER_LEVEL/2 * ::number(80,125)/100;
  int which = ::number(1,3);

  switch (which) {
    case 1:
      aff2.location = APPLY_STR;
      aff2.modifier = modifier;
      break;
    case 2:
      aff2.location = APPLY_DEX;
      aff2.modifier = modifier;
      break;
    case 3:
      aff2.location = APPLY_SPE;
      aff2.modifier = modifier;
      break;
  }
  aff2.type = SPELL_FERAL_WRATH;
  aff2.duration = 2*UPDATES_PER_MUDHOUR;
  aff2.bitvector = 0;

  if (!targ->affectJoin(targ, &aff2, AVG_DUR_NO, AVG_EFF_YES)) {
    vlogf(LOG_BUG, "Failed to join aff2 in addUniqueTrophyEffects");
    targ->sendTo("The faerie's magic fails.");
  } else {
    if (aff2.location == APPLY_STR) {
      act("The misty shape of a large bear settles on you.\n\rYou feel stronger.", FALSE, targ, NULL, NULL, TO_CHAR);
      act("A light mist in the shape of a large bear settles on $n.",
          FALSE, targ, NULL, NULL, TO_ROOM);
    } else if (aff2.location == APPLY_DEX) {
      act("The misty shape of a great cat settles on you.\n\rYou feel your reflexes quicken.", FALSE, targ, NULL, NULL, TO_CHAR);
      act("A light mist in the shape of a great cat settles on $n.",
          FALSE, targ, NULL, NULL, TO_ROOM);
    } else if (aff2.location == APPLY_SPE) {
      act("The misty shape of a snake settles on you.\n\rYou feel you can strike with great speed.", FALSE, targ, NULL, NULL, TO_CHAR);
      act("A light mist in the shape of a snake settles on $n.",
          FALSE, targ, NULL, NULL, TO_ROOM);
    }

  }
}

int clearUniqueTrophyEffects(TBeing *faerie, TBeing *targ)
{
  int rc;

  if (!targ) {
    vlogf(LOG_BUG, "Missing target in clearUniqueTrophyEffects.");
    return FALSE;
  }
  if (!faerie) {
    vlogf(LOG_BUG, "Missing faerie in clearUniqueTrophyEffects.");
    return FALSE;
  }
  rc = targ->spellWearOff(SPELL_FERAL_WRATH, SAFE_YES);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;
  targ->affectFrom(SPELL_FERAL_WRATH);

  return TRUE;
                  
}  
  

// only important that higher is better
int getPermaDeathRank(TBeing *targ)
{
  if (!targ) {
    vlogf(LOG_BUG, "getPermaDeathRank entered with null target");
    return 0;
  }
  if (!targ->hasQuestBit(TOG_PERMA_DEATH_CHAR))
    return 0;
  else return (int) max(1.,(targ->getExp()/100)); // don't want to overflow int
}

void permaDeathIntro(TBeing *faerie, TBeing *targ)
{
  if (!faerie) {
    vlogf(LOG_BUG, "permaDeathIntro entered with null faerie");
    return;
  }
  if (!targ) {
    vlogf(LOG_BUG, "permaDeathIntro entered with null targ");
    return;
  }
  faerie->doTell(targ->getName(),
    "You are greatly respected amongst my people for your great bravery.");
  faerie->doTell(targ->getName(),
    "I have been sent to help you in your endeavors!");
}

// barkskin
void addPermaDeathEffects(TBeing *faerie, TBeing *targ)
{
  affectedData aff;
  if (!targ) {
    vlogf(LOG_BUG, "addPermaDeathEffects entered with null targ");
    return;
  }
  if (!faerie) {
    vlogf(LOG_BUG, "addPermaDeathEffects entered with null faerie");
    return;
  }
 
  aff.type = SKILL_BARKSKIN;
  aff.location = APPLY_ARMOR;
  aff.duration = 2*UPDATES_PER_MUDHOUR;
  aff.bitvector = 0;
  aff.modifier = -90;

  
  if (!targ->affectJoin(targ, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
    vlogf(LOG_BUG, "Failed to join aff in addPermaDeathEffects");
    targ->sendTo("The faerie's magic fails.");
  } else {
    act("Your skin turns into an extremely hard, oak-like bark.",
        FALSE, targ, NULL, NULL, TO_CHAR);
    act("$n's skin turns into an extremely hard, oak-like bark.",
        TRUE, targ, NULL, NULL, TO_ROOM);
  }

}

int clearPermaDeathEffects(TBeing *faerie, TBeing *targ)
{
  int rc;

  if (!targ) {
    vlogf(LOG_BUG, "Missing target in clearPermaDeathEffects.");
    return FALSE;
  }
  if (!faerie) {
    vlogf(LOG_BUG, "Missing faerie in clearPermaDeathEffects.");
    return FALSE;
  }
  rc = targ->spellWearOff(SKILL_BARKSKIN, SAFE_YES);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;
  targ->affectFrom(SKILL_BARKSKIN);

  return TRUE;
      
}

// wrapper function
int heroFaerie(TBeing *ch, cmdTypeT cmd, const char *arg,
  TMonster *myself, TObj *)
{
  return heroFaerie(ch, cmd, arg, myself, NULL, FALSE);
} 

int heroFaerie(TBeing *ch, cmdTypeT cmd, const char *arg,
  TMonster *myself, TObj *, bool login)
{
  int rc = FALSE;
  
  if (!ch || !cmd || !myself)
    return FALSE;
  
  if (cmd == CMD_GIVE) {
    sstring sarg = arg;
    if (!isname(sarg.word(1), myself->name)) {
      return FALSE;
    } else {
      act("$n tells you, <1>\"<c>But I can't carry your things for you!<1>\"", 
          TRUE, myself, 0, ch, TO_VICT);
      return TRUE;
    }
  }
  
  if (!login && cmd != CMD_GENERIC_PULSE)
    return FALSE;
  
  if (myself->master && myself->roomp == myself->master->roomp && ::number(0,119)
      && !(myself->master->desc && myself->master->desc->autobits & AUTO_NOSPRITE)) {
    // want it on most of the time but not too spammy
    return FALSE;
  }

  if (login)
    vlogf(LOG_MAROR, "Calling heroFaerie on login.");

  if (!myself->awake())
    myself->doWake("");

  if (myself->canFly() && !myself->isFlying()) 
    myself->doFly();
  
  // put faerie and master in the same room
  if (myself->master && myself->roomp != myself->master->roomp) {
    act("$N left without you!  Can't have that!  *pop*", FALSE, myself, 0, myself->master, TO_CHAR);
    act("$n disappears.  *pop*", TRUE, myself, 0, NULL, TO_ROOM);
    --(*myself);
    thing_to_room(myself, myself->master->roomp->number); 
    myself->doLook("",CMD_LOOK);
    act("$n appears in the room.  *pop*", TRUE, myself, 0, NULL, TO_ROOM);
    act("$n tells you, <1>\"<c>Hey!  Why'd you leave me behind?<1>\"", TRUE, myself, 0, myself->master, TO_VICT);
    return TRUE;
  }

  bool stop_following = FALSE;
  if (myself->master && myself->master->desc && myself->master->desc->autobits & AUTO_NOSPRITE) {
    myself->doSay("Oh, so it's like that!");
    stop_following = TRUE;
  }
  
  if (myself->master && myself->master->getTimer() > MAX_TIME) {
    myself->doSay("I'm bored.");
    stop_following = TRUE;
  }
  
  // erase effects on current target
  if (myself->master && myself->roomp == myself->master->roomp) {
    act("Time for something new!  You point at $N, then touch your nose and wiggle your bottom.",
        FALSE, myself, 0, myself->master, TO_CHAR);
    act("$n points at you, then touches $s nose and wiggles $s bottom.",
        FALSE, myself, 0, myself->master, TO_VICT);
    act("$n points at $N, then touches $s nose and wiggles $s bottom.",
        FALSE, myself, 0, myself->master, TO_NOTVICT);
    switch(myself->mobVnum()) {
      case UNIQUE_TROPHY_FAERIE:
        rc = clearUniqueTrophyEffects(myself, myself->master);
        break;
      case PERMA_DEATH_FAERIE:
        rc = clearPermaDeathEffects(myself, myself->master);
        break;
      default:
        break;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      myself->master->reformGroup();
      delete myself->master;
      myself->master = NULL;
    }
  }

  if (myself->master && stop_following) {
    vlogf(LOG_PROC, fmt("Hero Faerie stopping follow in room %d.") % myself->in_room);
    myself->stopFollower(FALSE);
  }
  
  // identify new master
  Descriptor *d;
  TBeing *newMaster = NULL;
  for (d = descriptor_list; d; d = d->next) {
    if (d->connected != CON_PLYNG)
      continue;
    
    TBeing *targ = d->character;
    
    if (!targ || targ->GetMaxLevel() > MAX_MORT)
      continue;

    if (targ->getTimer() > MAX_TIME)
      continue;

    if (targ->roomp == real_roomp(ROOM_VOID))
      continue;
    
    if (targ->desc->autobits & AUTO_NOSPRITE) {
      continue;
    }
    
    switch(myself->mobVnum()) {
      case UNIQUE_TROPHY_FAERIE:
        if ( (getUniqueTrophyRank(targ) > 0 && !newMaster) || 
            ( newMaster &&
              getUniqueTrophyRank(targ) > getUniqueTrophyRank(newMaster))) {
          newMaster = targ;
        }
        break;
      case PERMA_DEATH_FAERIE:
        if ( (getPermaDeathRank(targ) > 0 && !newMaster) ||
            ( newMaster && 
              getPermaDeathRank(targ) > getPermaDeathRank(newMaster))) {
          newMaster = targ;
        }
        break;
      default:
        vlogf(LOG_BUG, fmt("defaulting with myself->mobVnum()=%d") % myself->mobVnum());
        break;
    }
  }
  
  // check for new target, if no target return DELETE_THIS
  // if target is new, move there and give intro message
  if (!newMaster) {
    act("You can't find a master, so you go on your way. *pop*", FALSE, myself, 0, NULL, TO_CHAR);
    act("$n disappears.  *pop*" , TRUE, myself, 0, NULL, TO_ROOM);
    if (myself->master)
    {
      vlogf(LOG_PROC, fmt("Hero Faerie clearing follower: was %s") % myself->master->getName());
      myself->stopFollower(FALSE);
    }
    --(*myself);
    // reinsert at birth room
    thing_to_room(myself, myself->brtRoom);
    myself->doLook("",CMD_LOOK);
    act("$n appears in the room.  *pop*", TRUE, myself, 0, NULL, TO_ROOM);
    return TRUE;
  } else if (myself->master != newMaster) {
    if (myself->master) {
      act("$n tells you, <1>\"<c>Sorry, gotta go.  Someone more interesting has arrived.<1>\"", 
          TRUE, myself, 0, myself->master, TO_VICT);
      vlogf(LOG_PROC, fmt("Hero Faerie switching follower: was %s") % (myself->master ? myself->master->getName() : "None"));
      myself->stopFollower(FALSE);
    }
    
    if (myself->circleFollow(newMaster)) {
      vlogf(LOG_BUG, fmt("Sprite %s following %s in a circle, bugging out.") % myself->name % newMaster->name);
      return TRUE;
    }

    if (myself == newMaster) {
      vlogf(LOG_BUG, fmt("Sprite %s is trying to follow itself, bugging out.") % myself->name);
      return TRUE;
    }
    
    act("You go to $N, your new master.  *pop*", FALSE, myself, 0, newMaster, TO_CHAR);
    act("$n disappears.  *pop*", TRUE, myself, 0, NULL, TO_ROOM);
    --(*myself);
    thing_to_room(myself, newMaster->roomp->number); 
    myself->doLook("",CMD_LOOK);
    act("$n appears in the room.  *pop*", TRUE, myself, 0, NULL, TO_ROOM);

    newMaster->addFollower(myself);
   
    switch(myself->mobVnum()) {
      case UNIQUE_TROPHY_FAERIE:
        uniqueTrophyIntro(myself, newMaster);
        break;
      case PERMA_DEATH_FAERIE:
        permaDeathIntro(myself, newMaster);
        break;
      default:
        break;
    }
   
  } else if (myself->roomp != myself->master->roomp) {
    vlogf(LOG_BUG, "Master and heroFaerie in different rooms - this should not happen at this point.");
    act("$N left without you!  Can't have that!.  *pop*", FALSE, myself, 0, myself->master, TO_CHAR);
    act("$n disappears.  *pop*", TRUE, myself, 0, NULL, TO_ROOM);
    --(*myself);
    thing_to_room(myself, myself->master->roomp->number); 
    myself->doLook("",CMD_LOOK);
    act("$n appears in the room.  *pop", TRUE, myself, 0, NULL, TO_ROOM);
    act("$n tells you, <1>\"<c>Hey!  Why'd you leave me behind?<1>\"", TRUE, myself, 0, myself->master, TO_VICT);
  }
  
  act("$n claps $s hands twice and sprinkles you with glittery dust.",
    TRUE, myself, 0, myself->master, TO_VICT);
  act("$n claps $s hands twice and sprinkles $N with glittery dust.",
    TRUE, myself, 0, myself->master, TO_NOTVICT);


  // spell up target
  switch(myself->mobVnum()) {
    case UNIQUE_TROPHY_FAERIE:
      addUniqueTrophyEffects(myself, newMaster);
      break;
    case PERMA_DEATH_FAERIE:
      addPermaDeathEffects(myself, newMaster);
      break;
    default:
      break;
  }
  
  return TRUE;

}


