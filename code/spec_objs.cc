////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "spec_objs.cc" - Special procedures for Objects 
//
/////////////////////////////////////////////////////////////////////////


//  cmd = CMD_OBJ_HITTING : a pre-hit spec, for denying, or overriding oneHit
//    returning TRUE prevents the hit from happening
//    ch parm is the victim, use obj->equippedBy for caster
//    if victim dies, return DELETE_VICT
//    if object gone, return DELETE_ITEM

//  cmd = CMD_OBJ_HIT : a post-hit spec, extra damage, etc.
//    cast arg to a wearSlotT to get part hit
//    ch parm is the victim
//    obj2 is also the wielder, but you have to cast it up/down to get it
//    obj->equippedBy is alternative to above method for getting wielder
//    if victim dies, return DELETE_VICT
//    if object gone, return DELETE_ITEM
//
//  cmd = CMD_OBJ_MISS : a post-hit spec, extra damage, etc.
//    ch parm is the victim
//    cast obj2 to a Being and it is the hitter : hopefully same me->equippedBy
//    if victim dies, return DELETE_THIS
//    if object gone, return DELETE_ITEM
//    if obj2 gone, return DELETE_VICT
//
//  cmd = CMD_OBJ_BEEN_HIT : spec for an object that has been hit
//    if victim (the hitter) dies, return DELETE_VICT
//    if object (the hitter's weapon) gone, return DELETE_ITEM
//    if the person being hit dies, return DELETE_THIS
// 
//  cmd == CMD_OBJ_MOVEMENT
//    called whenever anyone in my room moves out of room
//    arg can be cast to a dirTypeT
// 
//  cmd == CMD_GENERIC_PULSE
//    if obj gone, return DELETE_THIS
//    other parms are null
//    return TRUE if something happened
//
//  cmd == CMD_OBJ_STUCK_IN
//     if ch dies, return DELETE_VICT
//
//  cmd == CMD_OBJ_HAVING_SOMETHING_PUT_INTO, CMD_OBJ_PUT_INSIDE_SOMETHING,
//    if obj2 goes poof, return DELETE_ITEM
//
//  cmd == CMD_OBJ_GOTTEN
//    item has already been picked up, me = item gotten, obj2 = bag
//
//  cmd == CMD_OBJ_WEATHER_TIME
//    return DELETE_THIS if item2 goes bye, everyting else is NULL
//
//  cmd otherwise
//    if victim (1st parm) dies
//        leave it valid (do not delete) and return DELETE_VICT
//    if item1 goes poof, return DELETE_THIS
//  


#include "stdsneezy.h"
#include "disease.h"
#include "obj_base_corpse.h"
#include "obj_open_container.h"
#include "obj_trap.h"
#include "obj_portal.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"


// CMD_OBJ_GOTTEN returns DELETE_THIS if this goes bye bye
// returns DELETE_VICT if t goes bye bye
// returns DELETE_ITEM if t2 goes bye bye
int TObj::checkSpec(TBeing *t, cmdTypeT cmd, const char *arg, TThing *t2)
{
  int rc;

  // we use static_cast here because we don't ALWAYS pass the proper kind of
  // parameter through the pointer fields.
  if (spec) {
    rc = (objSpecials[GET_OBJ_SPE_INDEX(spec)].proc) 
          (t, cmd, arg, this, static_cast<TObj *>(t2));
    return rc;
  }
  return FALSE;
}

const int GET_OBJ_SPE_INDEX(int d)
{
  return ((d > NUM_OBJ_SPECIALS) ? 0 : d);
}

void obj_act(const char *message, const TThing *ch, const TObj *o, const TBeing *ch2, const char *color)
{
  char buffer[256];

  if (!ch) {
    vlogf(LOG_PROC,"NULL ch in obj_act");
    return;
  }
  if (!o) {
    vlogf(LOG_PROC,"NULL obj in obj_act");
    return;
  }
  sprintf(buffer, "$n's $o %s", message);
  act(buffer, TRUE, ch, o, ch2, TO_ROOM, color);
  sprintf(buffer, "Your $o %s",message);
  act(buffer, TRUE, ch, o, ch2, TO_CHAR, color);
}

static TBeing *genericWeaponProcCheck(TBeing *vict, cmdTypeT cmd, TObj *o, int chance)
{
  TBeing *ch;

  if (cmd != CMD_OBJ_HIT)
    return NULL;
  if (!o || !vict)
    return NULL;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return NULL;       // weapon not equipped (carried or on ground)
  if(::number(0,chance))
    return NULL;
  return ch;
}


int warMaker(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  char buf[256];
  TBeing *tmp;

  if (cmd < MAX_CMD_LIST) {
    if ((o->equippedBy != ch) && (o->parent != ch))
      return FALSE;             

    switch (cmd) {
      case CMD_CAST:
      case CMD_RECITE:
        obj_act("hums 'Get this:  $n wants to use some sissy magic!'", ch, o, NULL, NULL);
        ch->sendTo("You feel confused... what was it you were going to do?\n\r");
        return TRUE;
      case CMD_FLEE:
        if (o->equippedBy) {
          act("$n's $p glows red-hot in $s hands!", 1, ch, o, NULL, TO_ROOM, NULL);
          act("Your $p glows red-hot in your hands!", 1, ch, o, NULL, TO_CHAR);
          ch->addToHit(-dice(5, 5));
          if (ch->getHit() < 0) {
            ch->setHit(0);
            ch->setPosition(POSITION_STUNNED);
          }
          if (!ch->isTough()) {
            *ch->roomp += *ch->unequip(o->eq_pos);
            act("$n screams loudly, dropping $s $p.", 1, ch, o, NULL, TO_ROOM);
            act("You scream loudly, dropping your $p.", 1, ch, o, NULL, TO_CHAR);
          }
        }
        return TRUE;
      default:
        return FALSE;
    }
  } else if ((cmd == CMD_GENERIC_PULSE)) { 
    if (number(0,13))
      return FALSE;
    if (o->in_room != -1) {
      sprintf(buf, "%s moves a bit... as if alive!\n\r", o->shortDescr);
      sendToRoom(buf, roomOfObject(o));
    } else if ((tmp = dynamic_cast<TBeing *>(o->equippedBy))) {
      if (!tmp->fight()) {
        switch (number(1, 30)) {
          case 1:
            obj_act("sings 'Follow the Yellow Brick Road.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 2:
            obj_act("says, 'Let's go to the Shire, I want to waste some halfling youths.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 3:
            obj_act("says, 'Use the Force, L.., I mean:  go get um tiger.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 4:
            obj_act("bemoans, 'How is it such a wonderous sword as me gets stuck with a wimp and coward like you?'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 5:
            obj_act("says, 'We gonna stand here all day, or we gonna kill this thing?'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 6:
            obj_act("says, 'Group me.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 7:
            obj_act("says, 'That's it.  Puff must die.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 8:
            obj_act("says, 'Some of my closest friends are training daggers.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 9:
            obj_act("grins, 'I love it when they buff up mobs, it gives me a challenge.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 10:
            obj_act("wonders, 'Is it me, or are most other swords rather quiet?'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 11:
            obj_act("says, 'In a past life, I was a pipeweed bread.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 12:
            obj_act("says, 'Puff sure is a friendly dragon, isn't he.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 13:
            obj_act("says, 'The imps must hate the players to make asword SOooO AnnOYing!'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 14:
            obj_act("wonders, 'How come I'm never forced to shout how much I love Puff?'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 15:
            obj_act("says, 'Let's see who's using Tin tin.  Snowy shouts 'Yo''", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 16:
            obj_act("screams, 'BANZAI, CHARGE, SPORK, GERONIMO, DIE VERMIN, HIEYAH!'", tmp, o, NULL, ANSI_ORANGE);
            obj_act("blushes, 'Ooops, sorry, got over anxious to kill something.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 17:
            obj_act("offers, 'A good Swedish massage would cure what ails you.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 18:
            obj_act("muses: 'How exactly *does* an inanimate object talk?'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 19:
            obj_act("states, 'I'd rather be playing scrabble.'", tmp, 
                    o, NULL, ANSI_ORANGE);
            break;
          case 20:
            obj_act("says, 'A corpse is something to be cherished forever...until it rots.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 21:
            obj_act("sniffs the air, 'I love the smell of blood and ichor in the morning!'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 22:
            obj_act("chants, 'You might have armor or you might have fur, I'll hack them both, cuz I'm War-make-er.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 23:
            obj_act("scoffs, 'Sword of the Ancients, what a piece of junk!'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 24:
            obj_act("says, 'Do I have to fight again?  I just got all the gore off from last time.'", tmp, o, NULL, ANSI_ORANGE);
            break;
          case 26:
            obj_act("moans, 'If I have to kill Aunt Bee one more time, I'm gonna scream!'", tmp, o, NULL, ANSI_ORANGE);
            break;
          default:
            obj_act("grumbles 'Lets go kill something!'", tmp, o, NULL, ANSI_ORANGE);
            break;
        }
      }
    } else if (o->parent && dynamic_cast<TObj *>(o->parent)) {
      sprintf(buf, "Something grumbles 'Damnit, I'm %s.  Let me out of here.  It's dark.'\n\r",
              o->shortDescr);
      sendToRoom(buf, roomOfObject(o));
    } else if (o->parent) {
      act("$n's $o begs $m to wield it.", 1, o->parent, o, NULL, TO_ROOM);
      act("Your $o begs you to wield it.", 1, o->parent, o, NULL, TO_CHAR);
    }
    return TRUE;
  } else if ((cmd == CMD_OBJ_HITTING) && (dice(1, 10) == 1)) {
    if (!(tmp = dynamic_cast<TBeing *>(o->equippedBy)->fight()))
      return FALSE;
    if (number(0,1)) {
    switch (number(1, 20)) {
      case 1:
        obj_act("criticizes $N's lineage.", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 2:
        obj_act("says to $N, 'I fart in your general direction.'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 3:
        obj_act("looks at $N and asks, 'Is that your real face or are you just naturally ugly?'",
                o->equippedBy,o, tmp, ANSI_ORANGE);
        break;
      case 4:
        obj_act("tells $N, 'If they held an ugly pageant, you'd be a shoe in.'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 5:
        obj_act("tells $N, 'I wave my private parts in your direction!'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 6:
        obj_act("snickers something about $N's mother and combat boots.", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 7:
        obj_act("asks $N with a smirk, 'So, how many newbies have killed you today?'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 8:
        obj_act("states, 'Ya know, your sister was a mighty fine piece of ...'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 9:
        obj_act("looks at $N and says 'Not worth the time.'", o->equippedBy, (o), tmp, ANSI_ORANGE);
        break;
      case 10:
        obj_act("tells $N, 'Killing you is about as challenging as buying bread.'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 11:
        obj_act("laughs at the stupidity of $N.", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 12:
        obj_act("ponders: 'think I can kill $N in 5 rounds or less?'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 13:
        obj_act("snickers, 'OOH! Look at the fangs on that one.'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 14:
        obj_act("groans, '$N?!!  Surely you jest?  Haven't we killed $M enough already?'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 15:
        obj_act("looks at $N and sighs, 'When you get around to attacking something real, wake me.'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 16:
        obj_act("grins, 'Your head will look great mounted up above my mantle piece.'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      case 17:
        obj_act("looks at $N and says, 'Are you as stupid as you look, or do you just look stupid?'", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
      default:
        obj_act("taunts $N mercilessly.", o->equippedBy, o, tmp, ANSI_ORANGE);
        break;
    }
    }
    if (tmp->fight() != o->equippedBy) {
      act("$N turns towards $n with rage in $S eyes.", 1, o->equippedBy, o, tmp, TO_ROOM);
      act("$N turns towards you with rage in $S eyes.", 1, o->equippedBy, o, tmp, TO_CHAR);
      tmp->stopFighting();
      tmp->setCharFighting(dynamic_cast<TBeing *>(o->equippedBy));
    }
    return FALSE;
  }
  return FALSE;
}

int weaponFumbler(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  TThing *obj;

  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (::number(0,19))
    return FALSE;
  if (cmd != CMD_OBJ_HITTING)
    return FALSE;

  act("You cleave $N's body hard with your $o.",TRUE,ch,o,vict,TO_CHAR, ANSI_ORANGE);
  act("$n cleaves your body hard with $s $o.",TRUE,ch,o,vict,TO_VICT, ANSI_ORANGE);
  act("$n cleaves $N's body hard with $s $o.",TRUE,ch,o,vict,TO_NOTVICT, ANSI_ORANGE);

  if (ch->reconcileDamage(vict,::number(1,3), o->getWtype()) == -1) {
    return DELETE_VICT;
  }

  if ((obj = vict->heldInPrimHand())) {
    act("The blow knocks $n backwards and $e drops $s $o!",
        TRUE,vict,obj,0,TO_ROOM, ANSI_ORANGE);
    act("The blow knocks you backwards and you drop $p!",
        TRUE,vict,obj,0,TO_CHAR, ANSI_ORANGE);
    *vict->roomp += *vict->unequip(vict->getPrimaryHold());
  } else {
    act("The blow knocks $n backwards!",
        TRUE,vict,0,0,TO_ROOM, ANSI_ORANGE);
    act("The blow knocks you backwards!",
        TRUE,vict,0,0,TO_CHAR, ANSI_ORANGE);
  }       

  if (vict->riding) {
    int rc = vict->fallOffMount(vict->riding, POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;
  }
  vict->setPosition(POSITION_SITTING);

  vict->cantHit += vict->loseRound(0.5);
  return TRUE;
}

int poisonWhip(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  affectedData aff, aff2;

  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (cmd != CMD_OBJ_HITTING)
    return FALSE;
  if (vict->isImmune(IMMUNE_POISON, 20))
    return FALSE;
  if (vict->affectedBySpell(SPELL_POISON))
    return FALSE;

  if (!::number(0,49)) {
    aff.type = SPELL_POISON;
    aff.level = 10;
    aff.duration = (20) * UPDATES_PER_MUDHOUR;
    aff.modifier = -20;
    aff.location = APPLY_STR;
    aff.bitvector = AFF_POISON;

    aff2.type = AFFECT_DISEASE;
    aff2.level = 0;
    aff2.duration = aff.duration;
    aff2.modifier = DISEASE_POISON;
    aff2.location = APPLY_NONE;
    aff2.bitvector = AFF_POISON;

    act("The stingers on $p slice through your skin.", 0, vict, o, 0, TO_CHAR, ANSI_ORANGE);
    act("The stingers on $p slice through $n's skin.", 0, vict, o, 0, TO_ROOM, ANSI_ORANGE);
    vict->affectTo(&aff);
    vict->affectTo(&aff2);
    disease_start(vict, &aff2);

    return TRUE;
  }
  return FALSE;
}

int poisonViperBlade(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  affectedData aff, aff2;

  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
 if (cmd != CMD_OBJ_HITTING)
    return FALSE;
  if (vict->isImmune(IMMUNE_POISON, 20))
    return FALSE;
  if (vict->affectedBySpell(SPELL_POISON))
    return FALSE;

  if (!::number(0,36)) {
    aff.type = SPELL_POISON;
    aff.level = 15;
    aff.duration = (25) * UPDATES_PER_MUDHOUR;
    aff.modifier = -25;
    aff.location = APPLY_STR;
    aff.bitvector = AFF_POISON;

    aff2.type = AFFECT_DISEASE;
    aff2.level = 0;
    aff2.duration = aff.duration;
    aff2.modifier = DISEASE_POISON;
    aff2.location = APPLY_NONE;
    aff2.bitvector = AFF_POISON;

    act("<G>A strange green mist eminates from<1> $p.", 0, vict, o, 0, TO_CHAR);
    act("<G>A strange green mist eminates from<1> $p.", 0, vict, o, 0, TO_ROOM);
    act("<G>The green mist quickly forms into the shape of a venomous viper!<1>", 0, vict, o, 0, TO_CHAR);
      act("<G>The green mist quickly forms into the shape of a venomous viper!<1>", 0, vict, o, 0, TO_ROOM);
	act("<G>The viper quickly strikes $n, and just as quickly disappears!<1>", 0, vict, o, 0, TO_CHAR);
	act("<G>The viper quickly strikes $n, and just as quickly disappears!<1>", 0, vict, o, 0, TO_ROOM);
	vict->affectTo(&aff);
	vict->affectTo(&aff2);
	disease_start(vict, &aff2);

	return TRUE;
  }
  return FALSE;
} 



int poisonSap(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  affectedData aff, aff2;

  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (cmd != CMD_OBJ_HITTING)
    return FALSE;
  if (vict->isImmune(IMMUNE_POISON, 20))
    return FALSE;
  if (vict->affectedBySpell(SPELL_POISON))
    return FALSE;

  if (!::number(0,49)) {
    aff.type = SPELL_POISON;
    aff.level = 10;
    aff.duration = (20) * UPDATES_PER_MUDHOUR;
    aff.modifier = -20;
    aff.location = APPLY_STR;
    aff.bitvector = AFF_POISON;

    aff2.type = AFFECT_DISEASE;
    aff2.level = 0;
    aff2.duration = aff.duration;
    aff2.modifier = DISEASE_POISON;
    aff2.location = APPLY_NONE;
    aff2.bitvector = AFF_POISON;

    act("Fluid from $p mixes with your blood, causing a burning sensation.", 0, vict, o, 0, TO_CHAR, ANSI_GREEN);
    act("Fluid from $p mixes with $n's blood, causing $m to grimace in pain.", 0, vict, o, 0, TO_ROOM, ANSI_GREEN);
    vict->affectTo(&aff);
    vict->affectTo(&aff2);
    disease_start(vict, &aff2);

    return TRUE;
  }
  return FALSE;
}


int flameWeapon(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (!ch)
    return FALSE;

  dam = ::number(4,10);
  if (dam < 8) {
    act("$p erupts into roaring flames and sears $n.", 
                  0, vict, o, 0, TO_ROOM, ANSI_ORANGE);
    act("$p erupts into roaring flames and sears you.", 
                  0, vict, o, 0, TO_CHAR, ANSI_ORANGE);
  } else {
    act("$p roars into a blaze of fire and scorches $n.", 
                  0, vict, o, 0, TO_ROOM, ANSI_ORANGE_BOLD);
   act("$p roars into a blaze of fire and scorches you.", 
                  0, vict, o, 0, TO_CHAR, ANSI_ORANGE_BOLD);
  }

  rc = ch->reconcileDamage(vict, dam, DAMAGE_FIRE);
  if (rc == -1)
    return DELETE_VICT;
  return TRUE;
}

int frostWeapon(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (!ch)
    return FALSE;

  dam = ::number(4,10);
  if (dam < 8) {
    act("$p becomes covered with ice and freezes $n.", 
                  0, vict, o, 0, TO_ROOM, ANSI_CYAN);
    act("$p becomes covered with ice and freezes you.", 
                  0, vict, o, 0, TO_CHAR, ANSI_CYAN);
  } else {
    act("$p becomes covered with ice and sends a violent chill through $n.", 
                  0, vict, o, 0, TO_ROOM, ANSI_BLUE);
    act("$p becomes covered with ice and sends a violent chill through you.", 
                  0, vict, o, 0, TO_CHAR, ANSI_BLUE);
  }

  rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
  if (rc == -1)
    return DELETE_VICT;
  return TRUE;
}

int glowCutlass(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc;

  if (cmd != CMD_OBJ_HITTING)
    return FALSE;
  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)

  primaryTypeT primary = (ch->heldInPrimHand() == o) ? HAND_PRIMARY : HAND_SECONDARY;
  
  if (!::number(0,9)) {
    // this potentially sets up infinite loop
    rc = ch->oneHit(vict, primary, o, 0, 0);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete ch;
      ch = NULL;
      REM_DELETE(rc, DELETE_THIS);
    }
    if (IS_SET_DELETE(rc, DELETE_VICT) || IS_SET_DELETE(rc, DELETE_ITEM)) {
      return rc;
    }
    return FALSE;
  }
  return FALSE;
}

int weaponBreaker(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;

  char buf[256];

  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (::number(0,49))
    return FALSE;
  if (cmd != CMD_OBJ_HITTING)
    return FALSE;
  if (vict->isImmune(IMMUNE_BONE_COND, 0) || vict->raceHasNoBones()) {
    return FALSE;
  }

  if (!ch->canBoneBreak(vict, SILENT_YES))
    return FALSE;

  wearSlotT slot;
  for (slot = pickRandomLimb();; slot = pickRandomLimb()) {
    if (notBreakSlot(slot, true))
      continue;
    if (!vict->slotChance(slot))
      continue;
    break;
  }

  char limb[80];
  sprintf(limb, "%s", vict->describeBodySlot(slot).c_str());

  vict->addToLimbFlags(slot, PART_BROKEN);
  sprintf(buf, "A muffled SNAP leaps from your %s as $n hits it with $s $p!", limb);
  act(buf, FALSE, ch, o, vict, TO_VICT, ANSI_ORANGE);
  sprintf(buf, "Extreme pain shoots through your %s!\n\rYour %s has been broken and is now useless!", limb, limb);
  act(buf, FALSE, vict, NULL, NULL, TO_CHAR, ANSI_ORANGE);

  sprintf(buf, "You hit $N's %s hard with your $p.", limb);
  act(buf, FALSE, ch, o, vict, TO_CHAR, ANSI_ORANGE);
  sprintf(buf, "$n hits $N's %s hard with $s $p.", limb);
  act(buf, FALSE, ch, o, vict, TO_NOTVICT, ANSI_ORANGE);

  sprintf(buf, "You hear a muffled SNAP as $n clutches $s %s in extreme pain!"
, limb);
  act(buf, FALSE, vict, NULL, NULL, TO_ROOM, ANSI_ORANGE);

  vict->dropWeapon(slot);

  return TRUE;
}

int weaponDisruption(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int hardness, rc;
  wearSlotT part;
  TThing *obj;
  char buf[256];

  if (cmd != CMD_OBJ_HITTING) 
    return FALSE;
  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (::number(0,10))
    return FALSE;

  part = vict->getPartHit(ch, TRUE);
  if (!(obj = vict->equipment[part]))
    hardness = material_nums[vict->getMaterial()].hardness;
  else
    hardness = material_nums[obj->getMaterial()].hardness;
  spellNumT w_type = o->getWtype();
  obj_act("hums softly which quickly becomes a high pitched whine.",
            ch,o,vict, ANSI_ORANGE);
  sprintf(buf,"$n's $p screams with power as $e swings it at your %s!",
     vict->describeBodySlot(part).c_str());
  act(buf,TRUE,ch,o,vict,TO_VICT,ANSI_RED);
  sprintf(buf,"$n's $p screams with power as $e swings it at $N's %s!",
     vict->describeBodySlot(part).c_str());
  act(buf,TRUE,ch,o,vict,TO_NOTVICT,ANSI_ORANGE);
  sprintf(buf,"Your $p screams with power as you swing it at $N's %s!",
     vict->describeBodySlot(part).c_str());
  act(buf,TRUE,ch,o,vict,TO_CHAR,ANSI_GREEN);

  sprintf(buf,
    "A soft WOMPF! is heard as $p releases a shock wave into $n's %s!",
       (obj ? obj->getName() : (vict->isHumanoid() ? "skin" : "hide")));
  act(buf, TRUE, vict,o,0,TO_ROOM,ANSI_ORANGE);
  sprintf(buf,
    "A soft WOMPF! is heard as $p releases a shock wave into your %s!",
       (obj ? obj->getName() : (vict->isHumanoid() ? "skin" : "hide")));
  act(buf, TRUE, vict,o,0,TO_CHAR,ANSI_RED);
  
  if (obj)
    vict->damageItem(ch,part,w_type,o,hardness);
  else {
    rc = ch->damageLimb(vict,part,o,&hardness);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }
  if (ch->reconcileDamage(vict,hardness,DAMAGE_DISRUPTION) == -1)
    return DELETE_VICT;
  return TRUE;
}

int rainbowBridge(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  int rc;

  if (!o || !vict)
    return FALSE;
  if ((cmd != CMD_UP) && (cmd != CMD_DOWN) && 
      (cmd != CMD_CLIMB) && (cmd != CMD_DESCEND)) 
    return FALSE;

  // this is a portal so just translate these commands into an "enter"
  TPortal *por = dynamic_cast<TPortal *>(o);
  if (!por) return FALSE;

  rc = vict->doEnter(NULL, por);
  if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS)) {
    return DELETE_VICT | DELETE_THIS;
  }
  if (IS_SET_DELETE(rc, DELETE_ITEM)) {
    return DELETE_THIS;
  }
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_VICT;
  }
  return rc;
}

int ladder(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  roomDirData *exitp;
  int going_to_fall = FALSE,
      nRc = TRUE;

  if (!o || !vict)
    return FALSE;
  if ((cmd != CMD_UP) && (cmd != CMD_DOWN) && 
      (cmd != CMD_CLIMB) && (cmd != CMD_DESCEND)) 
    return FALSE;

  if (!vict->canSee(o))
    return FALSE;

  if (!vict->hasHands()) {
    vict->sendTo(COLOR_OBJECTS, "I'm afraid you need hands to climb %s.\n\r", o->getName());
    return TRUE;
  }
  if (vict->bothArmsHurt()) {
    vict->sendTo(COLOR_OBJECTS, "I'm afraid you need working arms to climb %s.\n\r", o->getName());
    return TRUE;
  }
  if (vict->riding) {
    vict->sendTo(COLOR_OBJECTS, "You can't ride your %s on %s.\n\r", fname(vict->riding->getName()).c_str(), o->getName());
    return TRUE;
  }
  if (vict->rider) {
    return TRUE;
  }
  if (vict->bothLegsHurt() && ::number(0,1))
    going_to_fall = TRUE;
  if (vict->eitherArmHurt() && ::number(0,1))
    going_to_fall = TRUE;
  if (vict->eitherLegHurt() && !::number(0,2))
    going_to_fall = TRUE;

  if (going_to_fall && (bSuccess(vict, vict->getSkillValue(SKILL_CLIMB)/4, SKILL_CLIMB))) {
    vict->sendTo("Whoa!  You almost fell there but your climbing ability saved you.\n\r");
    going_to_fall = FALSE;
  }
  if ((cmd == CMD_UP) || (cmd == CMD_CLIMB)) {
    exitp = vict->exitDir(DIR_UP);
    if (exit_ok(exitp, NULL)) {
      if (going_to_fall) {
        act("You start to climb $p but your busted limb makes you fall.",
            TRUE,vict,o,0,TO_CHAR);
        act("$n's busted limb causes $m to fall while climbing $p.",
            TRUE,vict,o,0,TO_ROOM);
        if (vict->reconcileDamage(vict,number(3,5),DAMAGE_FALL) == -1) {
          return DELETE_VICT;
        }
        vict->setPosition(POSITION_SITTING);
        return TRUE;
      } else {
        act("You climb up $p.",TRUE,vict,o,0,TO_CHAR);
        act("$n climbs up $p.",TRUE,vict,o,0,TO_ROOM);

        if ((nRc = vict->doMove(DIR_UP)) == FALSE)
          return TRUE;

        return nRc;
      }
    }
    return FALSE;
  } else if ((cmd == CMD_DOWN) || (cmd == CMD_DESCEND)) {
    exitp = vict->exitDir(DIR_DOWN);
    if (exit_ok(exitp, NULL)) {
      if (going_to_fall) {
        act("You start to climb $p but your busted limb makes you fall.",
            TRUE,vict,o,0,TO_CHAR);
        act("$n's busted limb causes $m to fall while climbing $p.",
            TRUE,vict,o,0,TO_ROOM);
        if (vict->reconcileDamage(vict,number(3,5),DAMAGE_FALL) == -1)
          return DELETE_VICT;
        vict->setPosition(POSITION_SITTING);
        return TRUE;
      } else {
        act("You climb down $p.",TRUE,vict,o,0,TO_CHAR);
        act("$n climbs down $p.",TRUE,vict,o,0,TO_ROOM);
        if ((nRc = vict->doMove(DIR_DOWN)) == FALSE)
          return TRUE;

        return nRc;
      }
    }
    return FALSE;
  }
  return FALSE;
}

int weatherArmor(TBeing *, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  wearSlotT pos;

  if (cmd != CMD_OBJ_WEATHER_TIME)
    return FALSE;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  ch->unequip(pos = o->eq_pos);

  // do whatever changes here

  ch->equipChar(o, pos, SILENT_YES);

  return TRUE;
}

int TObj::foodItemUsed(TBeing *ch, const char *)
{
  vlogf(LOG_LOW, "Undefined item (%s) with special proc: foodItem", getName());
  act("Oily black smoke pours from $p as something goes wrong.",
      TRUE, ch, this, 0, TO_CHAR);
  act("Oily black smoke pours from $p as something goes wrong.",
      TRUE, ch, this, 0, TO_ROOM);
  return FALSE;
}


int TWand::foodItemUsed(TBeing *ch, const char *arg)
{
  char buffer[256];
  TBeing *vict = NULL;

  one_argument(arg, buffer);
  if (!(vict = get_char_room_vis(ch, buffer))) {
    ch->sendTo("That person isn't here.\n\r");
    return FALSE;
  }
  if (ch == vict) {
    act("$n points $o at $mself.",
        TRUE, ch, this, 0, TO_ROOM);
    act("You point $o at yourself.",
        TRUE, ch, this, 0, TO_CHAR);
  } else {
    act("You point $o at $N.",
        TRUE, ch, this, vict, TO_CHAR);
    act("$n points $o at you.",
        TRUE, ch, this, vict, TO_VICT);
    act("$n points $o at $N.",
        TRUE, ch, this, vict, TO_NOTVICT);
  }
  if (getCurCharges() <= 0) {
    act("Nothing seems to happen.", TRUE, ch, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", TRUE, ch, 0, 0, TO_ROOM);
    return FALSE;
  }
  addToCurCharges(-1);
  if (vict->getCond(FULL) > -1)
    vict->gainCondition(FULL, 25);
  if (vict->getCond(THIRST) > -1)
    vict->gainCondition(THIRST, 25);
  act("You feel completely satiated.", TRUE, vict, 0, 0, TO_CHAR);
  act("$n smacks $s lips and looks very satisfied.",
            TRUE, vict, 0, 0, TO_ROOM);
  return TRUE;
}

int TStaff::foodItemUsed(TBeing *ch, const char *)
{
  TBeing *vict = NULL;
  TThing *t;

  act("$n taps $o three times on the $g.",
            TRUE, ch, this, 0, TO_ROOM);
  act("You tap $o three times on the $g.",
            TRUE, ch, this, 0, TO_CHAR);
  if (getCurCharges() <= 0) {
    act("Nothing seems to happen.", TRUE, ch, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", TRUE, ch, 0, 0, TO_ROOM);
    return FALSE;
  }
  addToCurCharges(-1);
  for (t = ch->roomp->getStuff(); t; t = t->nextThing) {
    vict = dynamic_cast<TBeing *>(t);
    if (!vict)
      continue;
    if (!vict->inGroup(*ch))
      continue;
    if (vict->getCond(FULL) > -1)
      vict->gainCondition(FULL, 25);
    if (vict->getCond(THIRST) > -1)
      vict->gainCondition(THIRST, 25);
    act("You feel completely satiated.", TRUE, vict, 0, 0, TO_CHAR);
    act("$n smacks $s lips and looks very satisfied.",
                TRUE, vict, 0, 0, TO_ROOM);
  }
  return TRUE;
}

int foodItem(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  char buffer[256];

  if (cmd != CMD_USE)
    return FALSE;
  if (!arg || !*arg)
    return FALSE;
  arg = one_argument(arg, buffer);

  if (isname(buffer, o->getName())) {
    return o->foodItemUsed(ch, arg);
  }

  return FALSE;
}

int orbOfDestruction(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  int rc;
  char buffer[256];
  TBeing *vict = NULL;
  TThing *t, *t2;
  int d = 0;
  int resCode = 0;

  if (cmd != CMD_USE &&
      cmd != CMD_PUSH)
    return FALSE;

  if (!arg || !*arg)
    return FALSE;

  one_argument(arg, buffer);

  if (cmd == CMD_USE || (cmd == CMD_PUSH && is_abbrev(buffer, "button"))) {
    arg = one_argument(arg, buffer);
    if (isname(buffer, o->getName())) {
      if (ch->getStat(STAT_CURRENT, STAT_PER) < 90) {
        ch->sendTo("You can't figure out how to use it.\n\r");
        return TRUE;
      } else if (!o->equippedBy) {
        ch->sendTo("You must be holding it to use it.\n\r");
        return TRUE;
      } else {
        resCode = TRUE;

        act("You trigger $p, causing it to explode in a fiery blast of light!", 1, ch, o, NULL, TO_CHAR);
        act("$n's $o explodes in a fiery blast of light!", 1, ch, o, NULL, TO_ROOM);
        for (t = ch->roomp->getStuff(); t; t = t2) {
          t2 = t->nextThing;
          vict = dynamic_cast<TBeing *>(t);
          if (!vict)
            continue;
          if (vict != ch) {
            d = ch->getActualDamage(vict, NULL, dice(10, 100), SPELL_ATOMIZE);
            if (ch->willKill(vict, d, SPELL_ATOMIZE, TRUE)) {
              act("You are consumed in the explosion!", 1, vict, o, NULL, TO_CHAR);
              act("$n is consumed in the explosion!", 1, vict, o, NULL, TO_ROOM);
            } else if (d > 0) {
              act("You are fried in the explosion!", 1, vict, o, NULL, TO_CHAR);
              act("$n is fried in the explosion!", 1, vict, o, NULL, TO_ROOM);
            } else {
              act("You laugh at the puny explosion.", 1, vict, o, NULL, TO_CHAR);
              act("$n laughs at the explosion.", 1, vict, o, NULL, TO_ROOM);
            }
            if (ch->reconcileDamage(vict, d, SPELL_ATOMIZE) == -1) {
              delete vict;
              vict = NULL;
            }
          }
        }
        ch->setHit(1);
        rc = ch->genericTeleport(SILENT_NO);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          ADD_DELETE(resCode, DELETE_VICT);

        // destroy item
        ADD_DELETE(resCode, DELETE_THIS);
      }
      return resCode;
    }
  }
  return FALSE;
}

int orbOfTeleportation(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  int rc;
  char buffer[256];
  TBeing *vict = NULL;
  TThing *t, *t2;
  int resCode = 0;

  if (!arg || !*arg)
    return FALSE;

  one_argument(arg, buffer);

  if (cmd == CMD_USE || (cmd == CMD_PUSH && is_abbrev(buffer, "button"))) {
    arg = one_argument(arg, buffer);
    if (isname(buffer, o->getName())) {
      if (ch->getStat(STAT_CURRENT, STAT_PER) < 90) {
        ch->sendTo("You can't figure out how to use it.\n\r");
        return TRUE;
      } else if (!o->equippedBy) {
        ch->sendTo("You must be holding it to use it.\n\r");
        return TRUE;
      } else {
        resCode = TRUE;

        act("You trigger $p, causing it to make a loud, throbbing noise!", 
             1, ch, o, NULL, TO_CHAR);
        act("$n's $o makes a loud, throbbing noise!", 
             1, ch, o, NULL, TO_ROOM);
        for (t = ch->roomp->getStuff(); t; t = t2) {
          t2 = t->nextThing;
          vict = dynamic_cast<TBeing *>(t);
          if (!vict)
            continue;
          if (vict != ch) {
            rc = vict->genericTeleport(SILENT_NO, TRUE);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete vict;
              vict = NULL;
            }
          }
        }
        act("Your $o seems to have blinked out of existence too...",
             1, ch, o, NULL, TO_CHAR);
        act("$n's $o seems to have blinked out of existence too...",
             1, ch, o, NULL, TO_ROOM);
        // destroy item
        ADD_DELETE(resCode, DELETE_THIS);
      }
      return resCode;
    }
  }
  return FALSE;
}

int Gwarthir(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *vict, *tmp;

  tmp = genericWeaponProcCheck(ch, cmd, o, 0);
  if (!tmp)
    return FALSE;
  if (!(vict = tmp->fight()))
    return FALSE;

  int dmg = ::number(1,3);
  if (tmp->reconcileDamage(vict, dmg, DAMAGE_DRAIN) == -1) {
    if (vict == ch)
      return DELETE_VICT;
    delete vict;
    vict = NULL;
  }
  tmp->addToHit(dmg);

  return TRUE;
}

void invert(const char *arg1, char *arg2)
{
  register int i = 0;
  register int len = strlen(arg1) - 1;

  while (i <= len) {
    *(arg2 + i) = *(arg1 + (len - i));
    i++;
  }
  *(arg2 + i) = '\0';
}


int jive_box(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *, TObj **)
{
  char buf[255], buf2[255], buf3[255], tmp[255];

  switch (cmd) {
    case CMD_SAY:
    case CMD_SAY2:
      invert(arg, buf);
      ch->doSay(buf);
      return TRUE;
      break;
    case CMD_TELL:
      half_chop(arg, tmp, buf);
      invert(buf, buf2);
      sprintf(buf3, "%s %s", tmp, buf);
      ch->doTell(buf3);
      return TRUE;
      break;
    case CMD_SHOUT:
      invert(arg, buf);
      ch->doShout(buf);
      return TRUE;
      break;
    default:
      return FALSE;
  }
}

int statue_of_feeding(TBeing *ch, cmdTypeT cmd, const char *argum, TObj *me, TObj *)
{
  char arg[160];

  *arg = '\0';

  if (cmd == CMD_WORSHIP) {
    act("$n utters a blessing unto $s deities.", TRUE, ch, me, NULL, TO_ROOM);
    act("You utter a blessing unto your deities.", TRUE, ch, me, NULL, TO_CHAR);

#if FACTIONS_IN_USE
    followData *k;

    for (k = ch->followers; k; k = k->next) {
      if (k->follower && !k->follower->isPc()) {
        if (k->follower->mobVnum() == FACTION_FAERY) {
          ch->sendTo("Your deity ignores you.\n\r");
          return TRUE;
        }
      }
    }

    TBeing *mob = read_mobile(FACTION_FAERY, VIRTUAL);
    if (!mob) {
      ch->sendTo("Problem!  Tell a god.\n\r");
      return FALSE;
    }
    *ch->roomp += *mob;
    ch->addFollower(mob);
    mob->setExp(0);
#endif
    return TRUE;
  }

  if (cmd != CMD_PRAY)
    return FALSE;

  one_argument(argum, arg);
  
  if (*arg && !isname(arg, me->getName()))
    return FALSE;

  act("$n begins to pray quietly before $p.", TRUE, ch, me, NULL, TO_ROOM);
  act("You begin to pray quietly before $p.", TRUE, ch, me, NULL, TO_CHAR);

  if (ch->isImmortal()) {
    act("$p turns in $n's direction and laughs uproariously.",
         TRUE, ch, me, NULL, TO_ROOM);
    act("$p spits on you and calls you a twit.",
         TRUE, ch, me, NULL, TO_CHAR);
  } else if (ch->GetMaxLevel() > 5)
    ch->sendTo("A statue lacks the power to help you any longer.\n\r");
  else if (ch->getCond(FULL) == 24)
    ch->sendTo("Nothing happens.\n\r");
  else {
    act("Suddenly, a ray of light shoots from $p towards $n.", TRUE, ch, me, NULL, TO_ROOM);
    act("A ray of light shoots towards you from $p.", TRUE, ch, me, NULL, TO_CHAR);
    ch->sendTo("You feel much less hungry.\n\r");
    if (ch->getCond(FULL) >= 0)
      ch->setCond(FULL, 24);
  }
  return TRUE;
}

int magicGills(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  int rc;
  TBeing *tmp;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (!(tmp = dynamic_cast<TBeing *>(me->equippedBy)))
    return FALSE;

  // if they areholding it, don't be silly
  if (me->eq_pos != WEAR_NECK)
    return FALSE;

  if (!tmp->roomp)
    return FALSE;

  if (tmp->roomp->isWaterSector() || tmp->roomp->isUnderwaterSector())
    return FALSE;

  if (tmp->isImmune(IMMUNE_SUFFOCATION, 50))
    return FALSE;

  if (!tmp->isPc())
    return FALSE;

  act("$p constricts about your throat!", 0, tmp, me, 0, TO_CHAR);
  act("$p constricts about $n's throat!", 0, tmp, me, 0, TO_ROOM);
  rc = tmp->applyDamage(tmp, ::number(1,3), DAMAGE_SUFFOCATION);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete tmp;
    tmp = NULL;
  }
  return TRUE;
}

int JewelJudgment(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  TBeing *tmp;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;
  if (!(tmp = dynamic_cast<TBeing *>(me->equippedBy)))
    return FALSE;
  if (number(0,2)) {
    return FALSE;
  } else {
    obj_act("pulses with a warm glow.",tmp,me,tmp, ANSI_ORANGE);
    act("$n looks drained as energy seeps from $m into $p.",TRUE,tmp,me, 0, TO_ROOM, ANSI_ORANGE);
    act("You grunt softly as energy seeps from your body into $p.",TRUE,tmp,me,0,TO_CHAR, ANSI_ORANGE);
    if (tmp->reconcileDamage(dynamic_cast<TBeing *>(me->equippedBy),number(3,8),DAMAGE_DRAIN) == -1) {
      delete tmp;
      tmp = NULL;
    }
      
    return TRUE;
  }
}


int fountain(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  int bits;
  char buf[MAX_INPUT_LENGTH];
  TBeing *tmp_char;
  TObj *obj;

  if (cmd == CMD_FILL) {
    arg = one_argument(arg, buf);
    bits = generic_find(buf, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, 
    ch, &tmp_char, &obj);
    if (!bits) {
      ch->sendTo("You must fill SOMETHING!!\n\r");
      return TRUE;
    }
    if (!ch->canSee(me)) {
      ch->sendTo("What do you expect to fill from?\n\r");
      return TRUE;
    }
    obj->fillMe(ch, LIQ_WATER);
    return TRUE;
  } else if (cmd == CMD_DRINK) {        
    only_argument(arg, buf);
    if (!isname(buf, me->getName()))
      return FALSE;

    if (ch->riding) {
      ch->sendTo("While mounted?\n\r");
      return TRUE;
    }
    if (ch->fight()) {
      ch->sendTo("You are too busy fending off your foes!\n\r");
      return TRUE;
    }
    if (ch->hasDisease(DISEASE_FOODPOISON)) {
      ch->sendTo("Uggh, your stomach is queasy and the thought of doing that is unappetizing.\n\r");
      ch->sendTo("You decide to skip this drink until you feel better.\n\r");
      return TRUE;
    }
    if (ch->getCond(THIRST) > 20 ||
        ch->getCond(THIRST) < 0) {
      act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
      return TRUE;
    }

    act("You drink from $p.", FALSE, ch, me, 0, TO_CHAR);
    act("$n drinks from $p.", FALSE, ch, me, 0, TO_ROOM);

    if (ch->getCond(THIRST) >= 0)
      ch->setCond(THIRST, 24);
    act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);

    return TRUE;
  }
  return FALSE;
}

int wine_fountain(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  int bits;
  char buf[MAX_INPUT_LENGTH];
  TBeing *tmp_char;
  TObj *obj;

  if (cmd == CMD_FILL) {
    arg = one_argument(arg, buf);
    bits = generic_find(buf, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP,
    ch, &tmp_char, &obj);
    if (!bits) {
      ch->sendTo("You must fill SOMETHING!!\n\r");
      return TRUE;
    }
    if (!ch->canSee(me)) {
      ch->sendTo("What do you expect to fill from?\n\r");
      return TRUE;
    }
    obj->fillMe(ch, LIQ_RED_WINE);
    return TRUE;
  } else if (cmd == CMD_DRINK) {        
    only_argument(arg, buf);
    if (!isname(buf, me->getName()))
      return FALSE;

    if (ch->riding) {
      ch->sendTo("While mounted?\n\r");
      return TRUE;
    }
    if (ch->fight()) {
      ch->sendTo("You are too busy fending off your foes!\n\r");
      return TRUE;
    }
    if (ch->hasDisease(DISEASE_FOODPOISON)) {
      ch->sendTo("Uggh, your stomach is queasy and the thought of doing that is unappetizing.\n\r");
      ch->sendTo("You decide to skip this drink until you feel better.\n\r");
      return TRUE;
    }
    if (ch->getCond(THIRST) > 20 ||
        ch->getCond(THIRST) < 0) {
      act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
      return TRUE;
    }

    act("You drink from $p.", FALSE, ch, me, 0, TO_CHAR);
    if (ch->getCond(THIRST) >= 0)
      ch->setCond(THIRST, 24);
    ch->gainCondition(DRUNK, ::number(1,5));
    act("Yum, nice, full, fruity bouquet.", FALSE, ch, 0, 0, TO_CHAR);

    return TRUE;
  }
  return FALSE;
}

int bless_fountain(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  int bits;
  char buf[MAX_INPUT_LENGTH];
  TBeing *tmp_char;
  TObj *obj;

  // works like normal fountain, but if drunk from, does short term blessing

  if (cmd == CMD_FILL) {
    arg = one_argument(arg, buf);
    bits = generic_find(buf, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP,
    ch, &tmp_char, &obj);
    if (!bits) {
      ch->sendTo("You must fill SOMETHING!!\n\r");
      return TRUE;
    }
    if (!ch->canSee(me)) {
      ch->sendTo("What do you expect to fill from?\n\r");
      return TRUE;
    }
    obj->fillMe(ch, LIQ_WATER);
    return TRUE;
  } else if (cmd == CMD_DRINK) {        
    only_argument(arg, buf);
    if (!isname(buf, me->getName()))
      return FALSE;

    if (ch->riding) {
      ch->sendTo("While mounted?\n\r");
      return TRUE;
    }
    if (ch->fight()) {
      ch->sendTo("You are too busy fending off your foes!\n\r");
      return TRUE;
    }
    if (ch->hasDisease(DISEASE_FOODPOISON)) {
      ch->sendTo("Uggh, your stomach is queasy and the thought of doing that is unappetizing.\n\r");
      ch->sendTo("You decide to skip this drink until you feel better.\n\r");
      return TRUE;
    }
    if (ch->getCond(THIRST) > 20 ||
        ch->getCond(THIRST) < 0) {
      act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
      return TRUE;
    }

    act("You drink from $p and fell divinely pure!", FALSE, ch, me, 0, TO_CHAR);
    if (ch->getCond(THIRST) >= 0)
      ch->setCond(THIRST, 24);

    genericBless(ch, ch, 5, false);

    return TRUE;
  }
  return FALSE;
}

int bowl_of_blood(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  char buf[MAX_INPUT_LENGTH];

  if (cmd == CMD_DRINK) {        
    only_argument(arg, buf);
    if (!isname(buf, me->getName()))
      return FALSE;

    if (ch->fight()) {
      ch->sendTo("You are too busy fending off your foes!\n\r");
      return TRUE;
    }
    if (ch->hasDisease(DISEASE_FOODPOISON)) {
      ch->sendTo("Uggh, your stomach is queasy and the thought of doing that is unappetizing.\n\r");
      ch->sendTo("You decide to skip this drink until you feel better.\n\r");
      return TRUE;
    }
    if (ch->getCond(THIRST) > 20 ||
        ch->getCond(THIRST) < 0) {
      act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
      return TRUE;
    }

    act("You drink from $p.", FALSE, ch, me, 0, TO_CHAR);
    act("$n drinks from $p.", FALSE, ch, me, 0, TO_ROOM);
    ch->sendTo("It tastes as horrible as it looks!\n\r");

    int level = 5;
    if (ch->isImmune(IMMUNE_DISEASE, level)) {
      act("$n shakes off the effects as if immune.",
          FALSE, ch, 0, 0, TO_ROOM);
      act("You shake off the effects of that disease-spewing $o.",
          FALSE, ch, me, 0, TO_CHAR);
    } else
      genericDisease(ch, level);
    genericCurse(ch, ch, level, SPELL_CURSE);

    return TRUE;
  }
  return FALSE;
}

void explode(TObj *obj, int room, int dam)
{
  TRoom *rm;
  TThing *t, *t2;
  int rc;
  TBeing *v = NULL;

  if (!(rm = real_roomp(room))) {
    vlogf(LOG_PROC, "Explosion in room : ROOM_NOWHERE. (explode() spec_objs.c)");
    return;
  }

  for (t = rm->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    v = dynamic_cast<TBeing *>(t);
    if (!v)
      continue;
    rc = v->objDamage(DAMAGE_TRAP_TNT, dam, obj);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      delete v;
      v = NULL;
    }
  }
  delete obj;
  obj = NULL;
}

int vending_machine(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *ob2)
{
  TObj *dew;
  int result, token;

  if (cmd == CMD_OBJ_HAVING_SOMETHING_PUT_INTO) {
    switch (obj_index[o->getItemIndex()].virt) {
      case 9998:
        result = 9996;
        token = 9995;
        break;
      case 9999:
        result = 9997;
        token = 9995;
        break;
      default:
        vlogf(LOG_PROC, "Unknown vending machine.  Yikes");
        return TRUE;
    }
    if (obj_index[ob2->getItemIndex()].virt == token) {
      act("You insert $p into $P.", TRUE, ch, ob2, o, TO_CHAR);
      act("$n inserts $p into $P.", TRUE, ch, ob2, o, TO_ROOM);
      act("$p begins to beep and shake.", TRUE, ch, o, NULL, TO_CHAR);
      act("$p begins to beep and shake.", TRUE, ch, o, NULL, TO_ROOM);
      if (!(dew = read_object(result, VIRTUAL))) {
        vlogf(LOG_PROC, "Damn vending machine couldn't read a Dew.  Stargazer!");
        return TRUE;
      }
      act("$p appears in the can receptical.", TRUE, ch, dew, NULL, TO_CHAR);
      act("$p appears in the can receptical.", TRUE, ch, dew, NULL, TO_ROOM);
      *o += *dew;
      return DELETE_ITEM;    // delete ob2
    } else
      ch->sendTo("It doesn't seem to fit.\n\r");

    return TRUE;
  }
  return FALSE;
}

int vending_machine2(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *ob2)
{
  class vendingmachine_struct {
  public:
    bool isOn;

    vendingmachine_struct() :
      isOn(false)
    {
    }
    ~vendingmachine_struct()
    {
    }
  };

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<vendingmachine_struct *>(o->act_ptr);
    o->act_ptr = NULL;
    return FALSE;
  } else if (cmd == CMD_GENERIC_CREATED) {
    o->act_ptr = new vendingmachine_struct();
    return FALSE;
  }
  TObj *drinkobj;

  char capbuf[80];
  vendingmachine_struct *job;
  int result, token = 9995;
  char arg1[30], arg2[30], arg3[30], drink[30];

  if (!ch)
    return FALSE;
  if (!(job = static_cast<vendingmachine_struct *>(o->act_ptr))) {
    vlogf(LOG_PROC, "Vending machine lost its memory. DASH!!");
    return FALSE;
  }

  strcpy(capbuf, ch->getName());
  if (cmd == CMD_OBJ_HAVING_SOMETHING_PUT_INTO) {
    if (obj_index[ob2->getItemIndex()].virt == token) {
      act("You insert $p into $P.", TRUE, ch, ob2, o, TO_CHAR);
      act("$n inserts $p into $P.", TRUE, ch, ob2, o, TO_ROOM);
      if (job->isOn) {
	act("$P beeps once, then spits $p out into the coin receptical.", TRUE, ch, ob2, o, TO_CHAR);
	act("$P beeps once, then spits $p out into the coin receptical.", TRUE, ch, ob2, o, TO_ROOM);
      } else {
	act("$P beeps loudly, and the <R>correct change<1> button lights up.", TRUE, ch, ob2, o, TO_CHAR);
	act("$P beeps loudly, and the <R>correct change<1> button lights up.", TRUE, ch, ob2, o, TO_ROOM);
	job->isOn = TRUE;
        return DELETE_ITEM;
      }
      return TRUE;
    } else {
      ch->sendTo("It doesn't seem to fit.\n\r");
      return TRUE;
    }
  } else if ((cmd == CMD_PUSH || cmd == CMD_PRESS)) {
    arg = one_argument(arg, arg1);
    arg = one_argument(arg, arg2);
    arg = one_argument(arg, arg3);
    if ((is_abbrev(arg1, "button") || is_abbrev(arg1, "machine") || is_abbrev(arg1, "vending")) &&
	(!is_abbrev(arg2, "button") || !is_abbrev(arg2, "machine") || !is_abbrev(arg2, "vending")) && arg2)
      drink = arg2;
    else if ((is_abbrev(arg1, "button") || is_abbrev(arg1, "machine") || is_abbrev(arg1, "vending")) &&
	     (is_abbrev(arg2, "button") || is_abbrev(arg2, "machine") || is_abbrev(arg2, "vending")) && arg3)
      drink = arg3;
    else if (arg1)
      drink = arg1;
    else
      return FALSE;
    
    
    if (is_abbrev(drink, "coke")) {
      result = 9994;
    } else if (is_abbrev(drink, "pepsi")) {
      result = 9993;
    } else if (is_abbrev(drink, "dew")) {
      result = 9996;
    } else if (is_abbrev(drink, "7up") || is_abbrev(drink, "Nup")) {
      result = 9997;
    } else if (is_abbrev(drink, "water")) {
      result = 9990;
    } else if (is_abbrev(drink, "juice") || is_abbrev(drink, "grapefruit")) {
      result = 9991;
    } else if (is_abbrev(drink, "jack") || is_abbrev(drink, "daniels")) {
      result = 9992;
    } else {
      ch->sendTo("The vending machine does not appear to have that button.\n\r");
      ch->sendTo("Bug Dash if you want him to stock the machine with something.\n\r");
      return TRUE;
    }
    act("$P beeps once as you select your drink.", TRUE, ch, ob2, o, TO_CHAR);
    act("$P beeps once as $n selects $s drink.", TRUE, ch, ob2, o, TO_ROOM);
    if (!job->isOn) {
      act("$P's <R>insert correct change<1> light blinks twice.", TRUE, ch, ob2, o, TO_CHAR);
      act("$P's <R>insert correct change<1> light blinks twice.", TRUE, ch, ob2, o, TO_ROOM);
      return TRUE;
    }
    else if (!(drinkobj = read_object(result, VIRTUAL))) {
      vlogf(LOG_PROC, "Damn vending machine couldn't read drink, obj %d.  DASH!!!", result);
      return TRUE;
    }
    else {
      act("With a loud *clunk* $p appears in the can receptical.", TRUE, ch, drinkobj, NULL, TO_CHAR);
      act("With a loud *clunk* $p appears in the can receptical.", TRUE, ch, drinkobj, NULL, TO_ROOM);
      *o += *drinkobj;
      job->isOn = FALSE;
      return DELETE_ITEM;    // delete ob2
    }
    
  }
  return FALSE;
}

int dagger_of_death(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  int rc;

  if (cmd == CMD_OBJ_STUCK_IN) {
    if (o->eq_stuck == WEAR_HEAD) {
      vlogf(LOG_PROC, "%s killed by ITEM:dagger-of-death at %s (%d)",
            ch->getName(), ch->roomp->getName(), ch->inRoom());

      rc = ch->die(DAMAGE_NORMAL);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
      return TRUE;
    }
  }
  return FALSE;
}


int dispenser(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TObj *note, *quill;
  char arg1[128], arg2[128];

  if ((cmd != CMD_GET) && (cmd != CMD_TAKE))
    return FALSE;

  half_chop(arg, arg1, arg2);

  if (is_abbrev(arg1, "note")) {
    if (isname(arg2, o->getName())) {
      act("You get a note from $p.", FALSE, ch, o, 0, TO_CHAR);
      act("$n gets a note from $p.", FALSE, ch, o, 0, TO_ROOM);
      if (!(note = read_object(GENERIC_NOTE, VIRTUAL))) {
        vlogf(LOG_PROC, "Bad note dispenser! NO note can be loaded!");
        return FALSE;
      }
      *ch += *note;
      return TRUE;
    }
  } else if (is_abbrev(arg1, "quill")) {
    if (isname(arg2, o->getName())) {
      act("You get a quill from $p.", FALSE, ch, o, 0, TO_CHAR);
      act("$n gets a quill from $p.", FALSE, ch, o, 0, TO_ROOM);
      if (!(quill = read_object(GENERIC_PEN, VIRTUAL))) {
        vlogf(LOG_PROC, "Bad quill dispenser! NO quill can be loaded!");
        return FALSE;
      }
      *ch += *quill;
      return TRUE;
    }
  }
  return FALSE;
}


int pager(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *ob2)
{
  class pager_struct {
    public:
      bool isOn;

      pager_struct() : 
        isOn(false)
      {
      }
      ~pager_struct()
      {
      }
  };

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<pager_struct *>(o->act_ptr);
    o->act_ptr = NULL;
    return FALSE;
  } else if (cmd == CMD_GENERIC_CREATED) {
    o->act_ptr = new pager_struct();
    return FALSE;
  }

  TBeing *t;
  char capbuf[80];
  pager_struct *job;
  

  if (!ch)
    return FALSE;
  if (!(job = static_cast<pager_struct *>(o->act_ptr))) {
    vlogf(LOG_PROC, "Pager lost its memory.");
    return FALSE;
  }

  strcpy(capbuf, ch->getName());

  if (cmd == CMD_USE || cmd == CMD_OPERATE) {
    if (o->equippedBy != ch)
      ch->sendTo("You must have it equipped to use it!\n\r");
    else if (job->isOn) {
      act("$n discretely turns off $s $o.", TRUE, ch, o, 0, TO_ROOM);
      ch->sendTo("You turn off your $o, trying to be very discrete about it.\n\r", fname(o->getName()).c_str());
      job->isOn = FALSE;
    } else {
      act("$n turns on $s $o, causing it to beep obnoxiously.", FALSE, ch, o, 0, TO_ROOM);
      ch->sendTo("You turn on your $o, producing a series of annoying beeps.\n\r", fname(o->getName()).c_str());
      job->isOn = TRUE;
    }
    return TRUE;
  } else if ((cmd == CMD_OBJ_TOLD_TO_PLAYER) && job->isOn) {
    // who got told to = ch, who told = ob2
    // user of pager = t

    // cast down and then up since ob2 is really a TBeing
    TThing *ttt = ob2;
    TBeing *tbob2 = dynamic_cast<TBeing *>(ttt);
    t = dynamic_cast<TBeing *>((o->parent) ? o->parent : o->equippedBy);
    strcpy(capbuf, tbob2->getName());

    if (t->hasColor())
      t->sendTo("%s%s%s tells you %s\"%s\"%s, triggering your pager.\n\r",             t->purple(), cap(capbuf), t->norm(), t->cyan(), arg, t->norm());
    else if (t->vt100())
      t->sendTo("%s%s%s tells you \"%s\", triggering your pager.\n\r",
             t->bold(), cap(capbuf), t->norm(), arg);
    else
      t->sendTo("%s tells you \"%s\", triggering your pager.\n\r",
              cap(capbuf), arg);

    tbob2->sendTo(COLOR_MOBS, "You tell %s \"%s\".\n\r", t->getName(), arg);

    strcpy(capbuf, t->getName());
    act("$n looks startled as $s $o begins to beep!",
         FALSE, t, o, NULL, TO_ROOM);
    return TRUE;
  }
  return FALSE;
}

// ch = told to
// ob2 = teller
int ear_muffs(TBeing *, cmdTypeT cmd, const char *, TObj *o, TObj *ob2)
{
  if ((cmd == CMD_OBJ_TOLD_TO_PLAYER) && (o->equippedBy)) {
    // cast down and then up since ob2 is really a TBeing
    TThing *ttt = ob2;
    TBeing *tb = dynamic_cast<TBeing *>(ttt);
    if (tb->isImmortal()) {
      tb->sendTo("You fail.  Perhaps they are busy?\n\r");
      return TRUE;
    }
  }
  return FALSE;
}

#if 0
int lightning_hammer(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  char keyword[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  TBeing *victim;
  affectedData af;
  const char KEYWORD[] =  "jank";

  if (cmd != CMD_SAY)
    return FALSE;

  half_chop(arg, keyword, name);

  if (!strcmp(keyword, KEYWORD)) {
    if (ch->affectedBySpell(SPELL_LIGHTNING_BOLT)) {
      ch->sendTo("You can only use the hammer's powers once a week!!\n\r");
      return TRUE;
    }
    if (!(victim = get_char_room_vis(ch, name))) {
      if (ch->fight()) {
        victim = ch->fight();
      } else {
        ch->sendTo("Whom do you want to use the hammers power on?\n\r");
        return TRUE;
      }
    }
#if 0
    ((*spell_info[SPELL_LIGHTNING_BOLT].spell_pointer) (6, ch, "", SPELL_TYPE_WAND, victim, NULL, o));
#endif
    if (!ch->isImmortal()) {
      af.type = SPELL_LIGHTNING_BOLT;
      af.duration = 168 * UPDATES_PER_MUDHOUR;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = 0;
      ch->affectTo(&af);
    }
  }
  return FALSE;
}
#endif

int daggerOfHunting(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  char objbuf[160], targbuf[160];
  TBeing *target;
  char buf[256];
  TRoom *rp;
  int rc;
  wearSlotT phit;
  int dam;

  if (cmd != CMD_THROW && cmd != CMD_REMOVE && cmd != CMD_OBJ_EXPELLED)
    return FALSE;

  if (cmd == CMD_REMOVE) {
    one_argument(arg, objbuf);
    if (!*objbuf || !isname(objbuf, me->getName())) {
      return FALSE;
    }

    if (me->stuckIn != ch && me->equippedBy != ch) {
      ch->sendTo("That item is nowhere on your body!\n\r");
      return FALSE;
    }
    rc = ch->doRemove("", me);
    act("$p mystically dissolves.", FALSE, ch, me, 0, TO_ROOM);
    act("$p mystically dissolves.", FALSE, ch, me, 0, TO_CHAR);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT | DELETE_THIS;
    return DELETE_THIS;
  } else if (cmd == CMD_OBJ_EXPELLED) {
    // the object is expelled
    // possibly it is now on ground (expel spell), or on ch (hospital)
    act("$p mystically dissolves.", FALSE, ch, me, 0, TO_ROOM);
    act("$p mystically dissolves.", FALSE, ch, me, 0, TO_CHAR);
    return DELETE_THIS;
  }

  two_arg(arg, objbuf, targbuf);

  if (!*objbuf || !isname(objbuf, me->getName()) || !*targbuf)
    return FALSE;

  if (me->equippedBy)
     dynamic_cast<TBeing *>(me->equippedBy)->unequip(me->eq_pos);
  else if (me->parent)
    --(*me);
  else
    return FALSE;

  if (me->isObjStat(ITEM_PROTOTYPE) && !ch->isImmortal()) {
    // oooh bad!
    // possible to do since don't need to actually hold to throw
    // bypassing the prototype acheck

    // cause it to target the thrower  :)
    ch->sendTo("A malicious force intervenes.\n\r");
    strcpy(targbuf, ch->getName());
  }

  act("With a mighty heave, you toss $p straight up.",
       FALSE, ch, me, 0, TO_CHAR);
  act("With a mighty heave, $n tosses $p straight up.",
       FALSE, ch, me, 0, TO_ROOM);
  if (!(target = get_char_vis(ch, targbuf, NULL))) {
    *(ch->roomp) += *me;
    act("$n falls to the $g.", TRUE, me, 0, 0, TO_ROOM);
    return TRUE;
  }
  *(ch->roomp) += *me;

  // OK, so I'm an evil evil bastard, so sue me   :)
  TTrap * ttr = dynamic_cast<TTrap *>(me);
  if (ttr) {
    // the infamous Batopr's Grenade of Hunting!!!
    // i'm ignoring sanity checks for insuring its a grenade, oh well
    ttr->armGrenade(ch);
  }

  act("A few feet up, $n stops its upward trajectory suddenly.", 
             TRUE, me, 0, 0, TO_ROOM);
  act("$n rotates quickly as if seeking something.",
             TRUE, me, 0, 0, TO_ROOM);

  while (!me->sameRoom(*target)) {
    int answer;
    dirTypeT dir = find_path(me->in_room, is_target_room_p, (void *) target->in_room, 5000, false, &answer);
    if (dir < 0) {
      act("$n falls to the $g.", TRUE, me, 0, 0, TO_ROOM);
      act("$p fails to find its target.", FALSE, ch, me, 0, TO_CHAR);
      act("$p fails to find its target.", FALSE, ch, me, 0, TO_ROOM);
       ch->sendTo("Unable to find path.  dir=%d, answer=%d\n\r", dir, answer);
      return TRUE;
    }
    sprintf(buf, "With blinding speed, $n streaks out of the room %s.",
             dirs_to_blank[dir]);
    act(buf, TRUE, me, 0, 0, TO_ROOM);
  
    if (!(rp = real_roomp(me->exitDir(dir)->to_room))) {
      return TRUE;
    }
    --(*me);
    *rp += *me;

    sprintf(buf, "With blinding speed, $n streaks into the room from the %s.",
             dirs[rev_dir[dir]]);
    act(buf, TRUE, me, 0, 0, TO_ROOM);
  }

  dam = 0;  // double purpose
  do {
    dam++;
    phit = target->getPartHit(NULL, FALSE);
  } while (target->getStuckIn(phit) && (dam < MAX_WEAR));

  if (dam >= MAX_WEAR) {
    // too much already impaled
    act("$p puffs into inconsequential smoke.", 
                      TRUE, ch, me, 0, TO_CHAR);
    act("$n puffs into inconsequential smoke.", 
                      TRUE, me, 0, 0, TO_ROOM);
    return DELETE_THIS;  // delete me
  }

  sprintf(buf, "$p impales itself into $n's %s.", target->describeBodySlot(phit).c_str());
  act(buf, TRUE, target, me, ch, TO_VICT);
//  act(buf, TRUE, target, me, NULL, TO_ROOM);
  act(buf, TRUE, target, me, ch, TO_NOTVICT);
  sprintf(buf, "$p impales itself into your %s.", target->describeBodySlot(phit).c_str());
  act(buf, TRUE, target, me, NULL, TO_CHAR);

  --(*me);
  rc = target->stickIn(me, phit);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete target;
    target = NULL;
    return TRUE;
  }
  TBaseWeapon * tbw = dynamic_cast<TBaseWeapon *>(me);
  if (tbw) {
    dam = (int)(tbw->baseDamage());
    rc = ch->applyDamage(target, dam, tbw->getWtype()); 
    sprintf(buf, "You do %d damage to $M.", dam);
    act(buf, TRUE, ch, 0, target, TO_CHAR);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete target;
      target = NULL;
    }
  }
  return TRUE;
}

int crystal_ball(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  int target;
  char buf[256], buf1[256], buf2[256];
  const char *c;
  TBeing *victim;

  if ((cmd != CMD_SHOW) && (cmd != CMD_WHERE) && (cmd != CMD_SAY) &&
      (cmd != CMD_SAY2) && (cmd != CMD_OBJ_GOTTEN))
    return FALSE;

  switch (cmd) {
    case CMD_OBJ_GOTTEN:
      obj_act("says 'Say \"show me <person>\" and I will show them to you.'", 
                  ch, me, NULL, ANSI_GREEN);
      return TRUE;
    case CMD_SAY:
    case CMD_SAY2:
      c = arg;
      c = one_argument(c, buf2);
      if (is_abbrev(buf2, "show")) {
        c = one_argument(c, buf2);
        if (is_abbrev(buf2, "me")) {
          c = one_argument(c, buf2);
        } else {
          ch->doSay(arg);
          obj_act("says '$n, you must speak the words correctly!'", 
                  ch, me, NULL, ANSI_GREEN);
          obj_act("says 'Say \"show me <person>\" and I will show them to you.'", 
                  ch, me, NULL, ANSI_GREEN);
          return TRUE;
        }
      } else if (is_abbrev(buf2, "where")) {
        c = one_argument(c, buf2);
      } else
        return FALSE;

      if (!(victim = get_char_vis_world(ch, buf2, NULL, EXACT_YES))) {
        ch->doSay(arg);
        obj_act("says 'Sorry $n, I have trouble finding that person.'", 
                ch, me, NULL, ANSI_GREEN);
        return TRUE;
      }
      break;
    case CMD_SHOW:
      half_chop(arg, buf, buf2);
      if (!is_abbrev(buf, "me")) {
        ch->doSay(arg);
        obj_act("says '$n, you must speak the words correctly!'", 
                ch, me, NULL, ANSI_GREEN);
        obj_act("says 'Say \"show me <person>\" and I will show them to you.'", 
                ch, me, NULL, ANSI_GREEN);
        return TRUE;
      }
      if (!(victim = get_char_vis_world(ch, buf2, NULL, EXACT_YES))) {
        ch->doSay(arg);
        obj_act("says 'Sorry $n, I have trouble finding that person.'", 
                ch, me, NULL, ANSI_GREEN);
        return TRUE;
      }
      break;
    case CMD_WHERE:
      one_argument(arg, buf2);
      if (!(victim = get_char_vis_world(ch, buf2, NULL, EXACT_YES))) {
        ch->doSay(arg);
        obj_act("says 'Sorry $n, I have trouble finding that person.'", 
               ch, me, NULL, ANSI_GREEN);
        return TRUE;
      }
      break;
    default:
      ch->doSay(arg);
      return TRUE;
  }
  if (cmd == CMD_SAY || cmd == CMD_SAY2)
    ch->doSay(arg);

  if (!victim->roomp) {
    obj_act("says 'Woah, big problem, talk to Brutius!'", 
        ch, me, NULL, ANSI_GREEN);
    return TRUE;
  }
  target = victim->roomp->number;

  if (victim->GetMaxLevel() > ch->GetMaxLevel()) {
    obj_act("says 'You are not powerful enough to see that person, $n!'", 
        ch, me, NULL, ANSI_GREEN);
    return TRUE;
  }
  sprintf(buf1, "%d look", target);
  ch->doAt(buf1, true);
  return TRUE;
}

int caravan_wagon(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  class wagon_struct {
    public:
      TThing *driver;

      wagon_struct() : driver(NULL)
      {
      }
      ~wagon_struct()
      {
      }
  };
  dirTypeT dir;
  wagon_struct *car;
  char buf[256];
  TRoom *rp2;

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<wagon_struct *>(me->act_ptr);
    me->act_ptr = NULL;
    return FALSE;
  }

  if (cmd == CMD_OBJ_WAGON_INIT) {
    if (me->act_ptr)
      delete static_cast<wagon_struct *>(me->act_ptr);

    car = new wagon_struct();
    if (!car) {
      vlogf(LOG_PROC, "Bad alloc (1) of caravan wagon");
      return FALSE;
    }
    me->act_ptr = car;

    car->driver = ch;

    return FALSE;
  } else if (cmd == CMD_OBJ_WAGON_UNINIT) {
    if (!(car = (wagon_struct *) me->act_ptr)) {
      vlogf(LOG_PROC, "Bad alloc (3) of caravan wagon");
      return FALSE;
    }
    if (me->act_ptr)
    car->driver = NULL;

    return FALSE;
  } else if (cmd == CMD_OBJ_MOVEMENT) {
    if (!(car = (wagon_struct *) me->act_ptr)) {
      vlogf(LOG_PROC, "Bad alloc (2) of caravan wagon");
      return FALSE;
    }
    if (ch != car->driver)
      return FALSE;

    int dum = (int) arg;
    dir = dirTypeT(dum);

    if (dir < MIN_DIR || dir >= MAX_DIR) {
      vlogf(LOG_PROC, "Problematic direction in CMD_OBJ_MOVEMENT");
      return FALSE;
    }
    sprintf(buf, "$n rolls %s.", dirs[dir]);
    act(buf, TRUE, me, 0, 0, TO_ROOM);

    rp2 = real_roomp(me->exitDir(dir)->to_room);

    (*me)--;
    *rp2 += *me;

    sprintf(buf, "$n rolls in from the %s.", dirs[rev_dir[dir]]);
    act(buf, TRUE, me, 0, 0, TO_ROOM);

    return TRUE;
  }
  return FALSE;
}

bool genericPotion(TBeing *ch, TObj *me, cmdTypeT cmd, const char *arg, int &rc)
{
  char buf[256];

  if (cmd != CMD_QUAFF && cmd != CMD_DRINK) {
    rc = FALSE;
    return true;
  }

  one_argument(arg, buf);
  if (!isname(buf, me->getName())) {
    rc = FALSE;
    return true;
  }

  if (ch->hasDisease(DISEASE_FOODPOISON)) {
    ch->sendTo("Uggh, your stomach is queasy and the thought of doing that is unappetizing.\n\r");
    ch->sendTo("You decide to skip this drink until you feel better.\n\r");
    rc = TRUE;
    return true;
  }
  return false;
}

int goofersDust(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  dirTypeT dir;
  char buf[256];
  int dum = (int) arg;

  if (cmd == CMD_OBJ_MOVEMENT) {
    dir = dirTypeT(dum);
    act("$n stumbles on $p.", TRUE, ch, me, 0, TO_ROOM);
    act("You stumble on $p.", TRUE, ch, me, 0, TO_CHAR);
    if (dir < MIN_DIR || dir >= MAX_DIR) {
      vlogf(LOG_PROC, "Problematic direction in CMD_OBJ_MOVEMENT for Goofers");
      return FALSE;
    }
    if (::number(0,3) == 0) {
      sprintf(buf, "As you moved %sward, you somehow tripped and fell down.", dirs[dir]);
      act(buf, TRUE, ch, me, 0, TO_CHAR);
      sprintf(buf, "$n trips and falls as $e moves in from a %sward direction.", dirs[dir]);
      act(buf, TRUE, ch, me, 0, TO_ROOM);
      ch->setPosition(POSITION_SITTING);
      delete me;
      return TRUE;
    } else {
      return FALSE;
    }
  }
  return FALSE;
}

int youthPotion(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  int rc;
  if (genericPotion(ch, me, cmd, arg, rc))
    return rc;

  act("$n imbibes $p.", TRUE, ch, me, 0, TO_ROOM);
  act("You imbibe $p.", TRUE, ch, me, 0, TO_CHAR);

  // every 2-weeks adds a year just from time
  // add on deaths.  Folks probably are not getting potions very often
  // so we can probably have a fairly high number on this
  ch->age_mod -= 7;

  ch->sendTo("You feel much younger.\n\r");

  return DELETE_THIS;
}

int statPotion(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  int rc;
  if (genericPotion(ch, me, cmd, arg, rc))
    return rc;

  TPotion *pot = dynamic_cast<TPotion *>(me);
  if (!pot)
    return true;

  act("$n imbibes $p.", TRUE, ch, pot, 0, TO_ROOM);
  act("You imbibe $p.", TRUE, ch, pot, 0, TO_CHAR);

  statTypeT whichStat = statTypeT(number(0, MAX_STATS - 1));

  ch->addToStat(STAT_CHOSEN, whichStat, 1);

  switch (whichStat) {
    case STAT_STR:
      ch->sendTo("You feel stronger.\n\r");
      break;
    case STAT_BRA:
      ch->sendTo("You feel brawnier.\n\r");
      break;
    case STAT_AGI:
      ch->sendTo("You feel more agile.\n\r");
      break;
    case STAT_CON:
      ch->sendTo("You feel more hardy.\n\r");
      break;
    case STAT_DEX:
      ch->sendTo("You feel more dexterous.\n\r");
      break;
    case STAT_INT:
      ch->sendTo("You feel smarter.\n\r");
      break;
    case STAT_WIS:
      ch->sendTo("You feel more wise.\n\r");
      break;
    case STAT_FOC:
      ch->sendTo("You feel more focused.\n\r");
      break;
    case STAT_KAR:
      ch->sendTo("You feel luckier.\n\r");
      break;
    case STAT_CHA:
      ch->sendTo("You feel more charismatic.\n\r");
      break;
    case STAT_SPE:
      ch->sendTo("You feel faster.\n\r");
      break;
    case STAT_PER:
      ch->sendTo("You feel more perceptive.\n\r");
      break;
    case STAT_LUC:
    case STAT_EXT:
    case MAX_STATS:
      break;
  }

  return DELETE_THIS;
}

int bogusObjProc(TBeing *, cmdTypeT, const char *, TObj *me, TObj *)
{
  if (me)
    vlogf(LOG_PROC, "WARNING:  %s is running around with a bogus spec_proc #%d",
       me->getName(), me->spec);
  else
    vlogf(LOG_PROC, "WARNING: indeterminate obj has bogus spec_proc");
  return FALSE;
}

int bleedChair(TBeing *ch, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  char buf[256], limb[256];
  int duration;
  wearSlotT slot;

  if (!ch)
    return FALSE;

  if (cmd != CMD_SIT)
    return FALSE;

  if (ch->isImmune(IMMUNE_BLEED, 0))
    return FALSE;

  ch->doSit(me->getName());

  ch->sendTo("Ouch that %shurt!%s\n\r", ch->red(), ch->norm());

  // insure some limb can be bled first...
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (notBleedSlot(slot))
      continue;
    if (!ch->slotChance(slot))
      continue;
    if (ch->isLimbFlags(slot, PART_BLEEDING))
      continue;
    break;
  }
  if (slot >= MAX_WEAR) {
    // no slots to bleed...
    return TRUE;
  }

  // now pick a random slot
  for (slot = pickRandomLimb(); ;slot = pickRandomLimb()) {
    if (notBleedSlot(slot))
      continue;

    if (!ch->slotChance(slot))
      continue;

    if (ch->isLimbFlags(slot, PART_BLEEDING))
      continue;

    break;
  }
  sprintf(limb, "%s", ch->describeBodySlot(slot).c_str());
  sprintf(buf, "A gaping gash opens up on your %s!\n\rBright red blood begins to course out!", limb);
  act(buf, FALSE, ch, NULL, NULL, TO_CHAR);
  sprintf(buf, "The flesh on $n's %s opens up and begins to bleed profusely!", limb);
  act(buf, FALSE, ch, NULL, NULL, TO_ROOM);
  duration = (ch->GetMaxLevel() * 3) + 200;
  ch->rawBleed(slot, duration, SILENT_YES, CHECK_IMMUNITY_NO);
  return TRUE;
}

int harmChair(TBeing *ch, cmdTypeT cmd, const char *, TObj *, TObj *)
{
  if (!ch)
      return FALSE;

  if (cmd != CMD_SIT)
    return FALSE;

  ch->sendTo("This is a test.");
  return FALSE;
}

int featherFallItem(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  if (cmd != CMD_OBJ_START_TO_FALL)
    return FALSE;

  TBeing *ch = dynamic_cast<TBeing *>(me->equippedBy);
  if (!ch) 
    return FALSE;

  if (ch->affectedBySpell(SPELL_FEATHERY_DESCENT))
    return FALSE;

  obj_act("glows with a brilliant white light.", ch, me, NULL, ANSI_WHITE);

  affectedData aff;
  aff.type = SPELL_FEATHERY_DESCENT;
  aff.level = 10;
  aff.duration = 5;  // they don't really need it to last that long

  ch->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES);

  return FALSE;
}

int wickedDagger(TBeing *vict, cmdTypeT cmd, const char *, TObj *me, TObj *ch_obj)
{
  int dam = ::number(1,10);
  spellNumT wtype = me->getWtype();

  if (::number(0,10) || !ch_obj || !vict || vict->getHit() <= dam ||
      (dynamic_cast<TBeing *>(dynamic_cast<TThing *>(ch_obj)))->getHit() <= dam)
    return FALSE;

  sendrpf(COLOR_OBJECTS, vict->roomp, "%s<k> sheds a light of iniquity.<z>\n\r",
          (me->getName() ? good_cap(me->getName()).c_str() : "Bogus Object"));

  if (cmd == CMD_OBJ_MISS) {
    // victim = vict
    // swinger = ch_obj as TBeing, so must cast back to being
    TThing *ch_thing = ch_obj;
    TBeing *ch = dynamic_cast<TBeing *>(ch_thing);

    ch->sendTo("You feel the life within you slowly ebb away.\n\r");

    // missing does dam to swinger
    int rc = ch->reconcileDamage(ch, dam, wtype);
    if (rc == -1)
      return DELETE_VICT;

    return FALSE;
  } else if (cmd == CMD_OBJ_HIT) {
    // victim = vict
    // hitting does extra dam to victim

    // we can safely use equippedBy since ch takes no damage
    TBeing *ch = dynamic_cast<TBeing *>(me->equippedBy);

    vict->sendTo("You feel the life within you slowly ebb away.\n\r");

    int rc = ch->reconcileDamage(vict, dam, wtype);
    if (rc == -1)
      return DELETE_VICT;

    return FALSE;
  }
  return FALSE;
}

int dragonSlayer(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (!ch)
    return FALSE;
  if(vict->getRace() != RACE_DRAGON)
    return FALSE;


  dam = ::number(1,ch->GetMaxLevel());
  act("$p hums with power and slams into $n seemingly of its own accord!", 
      0, vict, o, 0, TO_ROOM, ANSI_WHITE_BOLD);
  act("$p hums with power and slams into you seemingly of its own accord!", 
      0, vict, o, 0, TO_CHAR, ANSI_WHITE_BOLD);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_NORMAL);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;

}

int daySword(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  // it used to do magic-missile every round
  // this is a reasonable facsimile

  TBeing *ch;
  int rc;

  ch = genericWeaponProcCheck(vict, cmd, o, 6);
  if (!ch)
    return FALSE;

  if (!ch->outside())
    return false;
  if (!is_daytime())
    return false;

  act("A pulse of light as bright as the sun travels up the blade of $p.",
      FALSE, ch, o, NULL, TO_CHAR, ANSI_YELLOW);
  act("A pulse of light as bright as the sun travels up the blade of $p.",
      FALSE, ch, o, NULL, TO_ROOM, ANSI_YELLOW);

  act("<r>WOOMPF!!<z>", FALSE, ch, NULL, NULL, TO_CHAR);
  act("<r>WOOMPF!!<z>", FALSE, ch, NULL, NULL, TO_ROOM);

  act("$p discharges its energy into $n.", false, vict, o, NULL, TO_ROOM);
  act("$p discharges its energy into you!", false, vict, o, NULL, TO_CHAR);

  int dam = ::number(5,8);
  rc = ch->reconcileDamage(vict, dam, DAMAGE_NORMAL);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  return TRUE;
}

int nightBlade(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  // it used to do magic-missile every round
  // this is a reasonable facsimile

  TBeing *ch;
  int rc;

  ch = genericWeaponProcCheck(vict, cmd, o, 6);
  if (!ch)
    return FALSE;

  if (!ch->outside())
    return false;
  if (!is_nighttime())
    return false;

  act("A pulse of darkness as black as the new moon travels up the blade of $p.",
      FALSE, ch, o, NULL, TO_CHAR, ANSI_BLACK);
  act("A pulse of darkness as black as the new moon travels up the blade of $p.",
      FALSE, ch, o, NULL, TO_ROOM, ANSI_BLACK);

  act("<p>WOOMPF!!<z>", FALSE, ch, NULL, NULL, TO_CHAR);
  act("<p>WOOMPF!!<z>", FALSE, ch, NULL, NULL, TO_ROOM);

  act("$p discharges its energy into $n.", false, vict, o, NULL, TO_ROOM);
  act("$p discharges its energy into you!", false, vict, o, NULL, TO_CHAR);

  int dam = ::number(5,8);
  rc = ch->reconcileDamage(vict, dam, DAMAGE_NORMAL);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  return TRUE;
}



int bloodDrain(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 8);
  if (!ch)
    return FALSE;

  dam = ::number(4,10);
  act("$p <1><r>pulses with an <k>unholy<1><r> light and oozes blood as it draws <y>life<1><r> essence from<1> $n.",
      0, vict, o, 0, TO_ROOM);
  act("$p <1><r>pulses with an <k>unholy<1><r> light and oozes blood as it draws your very <y>life<1><r> essence from you<1>.",
      0, vict, o, 0, TO_CHAR);

  ch->dropPool(3, LIQ_BLOOD);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_DRAIN);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}

int energyBeam(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 8);
  if (!ch)
    return FALSE;

  dam = ::number(4,10);
  act("$p <1><W>glows in a sparkling, bright white light<1>.\n\r<W>You hear a deafening crackle as <1>$p <1><W>jolts $m!<1>",
      0, vict, o, 0, TO_ROOM);
  act("$p <1><W>glows in a sparkling, bright white light<1>.\n\r<W>You hear a deafening crackle as $n's <1>$p <1><W>jolts you!<1>",
      0, vict, o, 0, TO_CHAR);

  rc = ch->reconcileDamage(vict, dam, DAMAGE_ELECTRIC);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}                

int scirenDrown(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam;

  ch = genericWeaponProcCheck(vict, cmd, o, 8);
  if (!ch)
    return FALSE;

  if (vict->affectedBySpell(SPELL_SUFFOCATE))
    return FALSE;   

  dam = ::number(4,10);
  act("$p <1><B>pulsates with a glowing blue hue<1>.\n\r$p <1><B>emits a stream of salty water directed at $n's mouth<1>!!",
      0, vict, o, 0, TO_ROOM);
  act("$p <1><B>pulsates with a glowing blue hue<1>.\n\r$p <1><B>emits a stream of salty water directed at your mouth<1>!!",
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
  return TRUE;   

}                    

int stoneAltar(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *obj, TObj *)
{
  if (!ch)
    return FALSE;

  if (cmd != CMD_PUSH && cmd != CMD_PRESS)
    return FALSE;

  char buf[256];
  one_argument(arg, buf);
  if (is_abbrev(buf, "eye") || is_abbrev(buf, "diamond")) {
    TOpenContainer *trc = dynamic_cast<TOpenContainer *>(obj);
    if (!trc)
      return FALSE;
    if (trc->isClosed()) {
      act("You push on the diamond eye, causing $p to open.",
             TRUE, ch, trc, NULL, TO_CHAR);
      act("$n fiddles with $p, causing it to open.",
             TRUE, ch, trc, NULL, TO_ROOM);
      trc->remContainerFlag(CONT_CLOSED);
    } else {
      act("You push on the diamond eye, causing $p to close.",
             TRUE, ch, trc, NULL, TO_CHAR);
      act("$n fiddles with $p, causing it to close.",
             TRUE, ch, trc, NULL, TO_ROOM);
      trc->addContainerFlag(CONT_CLOSED);
    }
    return TRUE;
  }
  return FALSE;
}

int boneStaff(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  wearSlotT part_hit = wearSlotT((int) arg);

  ch = genericWeaponProcCheck(vict, cmd, o, 0);
  if (!ch)
    return FALSE;

  // if we hit body, head, or neck, suck some life into user
  // we've already "hit" them, so life from vict has already been taken
  if (part_hit != WEAR_BODY && part_hit != WEAR_NECK && part_hit != WEAR_HEAD)
    return FALSE;

  int amount = min(1, ch->hitLimit() - ch->getHit());
  if (amount) {
    act("$p draws your life force through it into $N.",
       TRUE, vict, o, ch, TO_CHAR, ANSI_GREEN_BOLD);
    act("$p draws the life force of $n through it into you.",
       TRUE, vict, o, ch, TO_VICT, ANSI_GREEN_BOLD);
    act("$p draws the life force of $n through it into $N.",
       TRUE, vict, o, ch, TO_NOTVICT, ANSI_GREEN_BOLD);
    ch->addToHit(amount);
    ch->addToLifeforce(amount);
  }

  return TRUE;
}

// o is being hit, ch is o's owner, v is doing the hitting, with weapon
int behirHornItem(TBeing *v, cmdTypeT cmd, const char *, TObj *o, TObj *weapon)
{
  TBeing *ch;
  int rc, dam;
  wearSlotT t;
  TObj *savedby=NULL;
  char buf[256];

  if(cmd != CMD_OBJ_BEEN_HIT || !v || !o)
    return FALSE;
  if(::number(0, 3))
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;     

  if (weapon && !material_nums[weapon->getMaterial()].conductivity)
    savedby=weapon;
  t=((!weapon || (weapon->eq_pos==HOLD_RIGHT))?WEAR_HAND_R:WEAR_HAND_L);
  if(v->equipment[t] && 
     !material_nums[v->equipment[t]->getMaterial()].conductivity)
    savedby=dynamic_cast<TObj *>(v->equipment[t]);

  act("$p <B>flares up brightly and <W>jolts<B> $n <B>with an electric shock!<1>"
      , 0, v, o, 0, TO_ROOM);
  act("$p <B>flares up brightly and <W>jolts<B> you with an electric shock!<1>"
      , 0, v, o, 0, TO_CHAR);
    
  if(savedby){
    sprintf(buf, "<k>Luckily, $s <1>$o<k> is not conductive and saves $m from harm.<1>");
    act(buf, 0, v, savedby, 0, TO_ROOM);
    sprintf(buf, "<k>Luckily, your <1>$o<k> is not conductive and saves you from harm.<1>");
    act(buf, 0, v, savedby, 0, TO_CHAR);
  } else {
    dam = ::number(5, 20);
    
    rc = ch->reconcileDamage(v, dam, DAMAGE_ELECTRIC);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }

  return TRUE;
}

int bloodspike(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam;
  TObj *obj;
  wearSlotT slot;
  char buf[256];

  if(!(ch=genericWeaponProcCheck(vict, cmd, o, 3)))
     return FALSE;

  slot=pickRandomLimb();

  if (!vict->slotChance(wearSlotT(slot)) || 
      vict->getStuckIn(wearSlotT(slot)) ||
      notBleedSlot(slot))
    return FALSE;

  obj = read_object(13713, VIRTUAL);
  dam = ::number(3,12);

  sprintf(buf, "<k>There is a sharp crack as one of <1>$p<k>'s spikes breaks off while embedded in <1>$n<k>'s <1>%s<k>.<1>", vict->describeBodySlot(slot).c_str());
  act(buf, 0, vict, o, 0, TO_ROOM);
  sprintf(buf, "<k>There is a sharp crack as one of <1>$p<k>'s spikes breaks off while embedded in your <1>%s<k>.<1>", vict->describeBodySlot(slot).c_str());
  act(buf, 0, vict, o, 0, TO_CHAR);

  vict->stickIn(obj, wearSlotT(slot));

  if(!vict->isImmune(IMMUNE_BLEED, 50)){
    vict->rawBleed(slot, 250, SILENT_YES, CHECK_IMMUNITY_NO);
    vict->rawInfect(slot, 250, SILENT_YES, CHECK_IMMUNITY_NO);

    sprintf(buf, "<r>Blood<k> squirts through the hollow spike uncontrollably!<1>");
    act(buf, 0, vict, o, 0, TO_ROOM);
    sprintf(buf, "<r>Blood<k> squirts through the hollow spike uncontrollably!<1>");
    act(buf, 0, vict, o, 0, TO_CHAR);
  }    

  rc = ch->reconcileDamage(vict, dam, TYPE_STAB);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}

// spinoff of Peel's Bloodspike
int splinteredClub(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam;
  TObj *obj;
  wearSlotT slot;
  char buf[256];

  if(!(ch=genericWeaponProcCheck(vict, cmd, o, 3)))
     return FALSE;

  slot=pickRandomLimb();

  if (!vict->slotChance(wearSlotT(slot)) || 
      vict->getStuckIn(wearSlotT(slot)) ||
      notBleedSlot(slot))
    return FALSE;

  obj = read_object(31349, VIRTUAL);
  dam = ::number(3,12);

  sprintf(buf, "<o>A splinter from <1>$p<o> breaks off and embeds in <1>$n<o>'s <1>%s<o>.<1>", vict->describeBodySlot(slot).c_str());
  act(buf, 0, vict, o, 0, TO_ROOM);
  sprintf(buf, "<o>A splinter from <1>$p<o> breaks off and embeds in your <1>%s<o>.<1>", vict->describeBodySlot(slot).c_str());
  act(buf, 0, vict, o, 0, TO_CHAR);

  vict->stickIn(obj, wearSlotT(slot));

  if(!vict->isImmune(IMMUNE_BLEED, 50)){
    vict->rawBleed(slot, 250, SILENT_YES, CHECK_IMMUNITY_NO);
    vict->rawInfect(slot, 250, SILENT_YES, CHECK_IMMUNITY_NO);

    sprintf(buf, "<R>Blood<1> <o>drips out of the wound created by the splinter.<1>");
    act(buf, 0, vict, o, 0, TO_ROOM);
    sprintf(buf, "<R>Blood<1> <o>drips out of the wound created by a large splinter.<1>");
    act(buf, 0, vict, o, 0, TO_CHAR);
  }    

  rc = ch->reconcileDamage(vict, dam, TYPE_STAB);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}



int vorpal(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *){
  TThing *weap=dynamic_cast<TThing *>(o);
  int dam, rc=0;
  wearSlotT part;
  spellNumT wtype;
  TBeing *ch;
  int crits[20]={67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,91,92,98,99};

  if(!(ch=genericWeaponProcCheck(vict, cmd, o, 50)))
     return FALSE;

  part = vict->getPartHit(ch, TRUE);
  dam = ch->getWeaponDam(vict, weap, HAND_PRIMARY);

  if (weap)
    wtype = ch->getAttackType(weap);
  else
    wtype = TYPE_HIT;

  act("$p <r>begins glowing deep red!<1>", 0, vict, o, 0, TO_ROOM);

  if(!::number(0,3)){
    o->setMaxStructPoints(o->getMaxStructPoints()-1);
    o->setStructPoints(o->getStructPoints()-1);
  }    

  rc = ch->critSuccessChance(vict, weap, &part, wtype, &dam, crits[::number(0,20)]);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_VICT;
  } else if (!rc) {
    act("$p swings abruptly, but fails to hit anything.", 0, vict, o, 0, TO_ROOM);
    return FALSE;
  }
  rc = ch->applyDamage(vict, dam, wtype);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_VICT;
  }
  return FALSE;
}


// Is what is says, This is a special proc that will one day help newbies
// understand sneezy more.
int newbieHelperWProc(TBeing *vict, cmdTypeT cmd, const char *Parg, TObj *o, TObj *)
{
  TBeing *ch;
  char PargA[30], // Should be 'help'
       Topic[30], // One of a series of possible help topics.
       buf[30];

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (((o->equippedBy != ch) && (o->parent != ch)) ||
      (!ch->isImmortal() && (ch->GetMaxLevel() > 4)))
    return FALSE;

  Parg = one_argument(Parg, PargA);
  Parg = one_argument(Parg, Topic);
  ch->sendTo("Newbie Weapon Info: %s %s\n\r", PargA, Topic);

  switch (cmd) {
    case CMD_SAY:
      if (is_abbrev(PargA, "help"))
        if (!*Topic) {
          ch->sendTo("%s: Hello %s.  I am a newbie helper weapon.\n\r",
                     o->getName(), ch->getName());
          ch->sendTo("%s: I will lend assistance when you need it.\n\r",
                     o->getName());
          ch->sendTo("%s: This is how you use me:\n\r",
                     o->getName());
          ch->sendTo("%s:      help ??? where ??? is one of the following:\n\r",
                     o->getName());
          ch->sendTo("%s:  basics   : A basic helpfile to get your started.\n\r",
                     o->getName());
          ch->sendTo("%s:  goto     : Covers the 'donation' command.\n\r",
                     o->getName());
          ch->sendTo("%s:  donation : Covers the donation room.\n\r",
                     o->getName());
          ch->sendTo("%s:  kill     : This covers the 'kill' command.\n\r",
                     o->getName());
          ch->sendTo("%s:  consider : This covers a very important command, 'consider'.\n\r",
                     o->getName());
          return TRUE;
        } else {
          if (is_abbrev(Topic, "basics")) {
            ch->sendTo("%s: The first command to get used to is 'help' without the say.\n\r",
                       o->getName());
            ch->sendTo("%s: The help command will cover most everything you need plus more.\n\r",
                       o->getName());
            ch->sendTo("%s: You should have also gotten a newbie book.  To use this book:\n\r",
                       o->getName());
            ch->sendTo("%s:   read newbie\n\r",
                       o->getName());
            ch->sendTo("%s: If you get in a jam and need help that you can't seem to find.\n\r",
                       o->getName());
            ch->sendTo("%s: Then do a:  who  and look for anyone with (Newbie-helper) to\n\r",
                       o->getName());
            ch->sendTo("%s: the right of there name.  These people are here to help you learn\n\r",
                       o->getName());
            ch->sendTo("%s: and understand the mud.  If there is nobody on with that, then\n\r",
                       o->getName());
            ch->sendTo("%s: use the  who  command and look for a friendly name.\n\r",
                       o->getName());
            ch->sendTo("%s: Once you have a person you wish to ask, talk to them with:\n\r",
                       o->getName());
            ch->sendTo("%s:   tell player_name what_to_ask\n\r",
                       o->getName());
            ch->sendTo("%s: Example:  tell mrfriendly Hi, i'm new here.  Can you help me?\n\r",
                       o->getName());
            ch->sendTo("%s: There are certain communication rules which you should know:\n\r",
                       o->getName());
            ch->sendTo("%s:   1. Please don't use all CAPITAL LETTERS.\n\r",
                       o->getName());
            ch->sendTo("%s:   2. Please avoid profanity, try and keep it clean.\n\r",
                       o->getName());
            ch->sendTo("%s:   3. If you ask someone for help and they don't reply or choose\n\r",
                       o->getName());
            ch->sendTo("%s:      not to help you, please don't give repetive tells to them.\n\r",
                       o->getName());
            ch->sendTo("%s:   4. When you obtain level 2 and the shout command, please don't\n\r",
                       o->getName());
            ch->sendTo("%s:      abuse it.  See  help shout  about this command and rules.\n\r",
                       o->getName());
          } else if (is_abbrev(Topic, "donation")) {
            ch->sendTo("%s: Surplus items are left here for others.\n\r",
                       o->getName());
            ch->sendTo("%s: When taking from surplus please use these rules:\n\r",
                       o->getName());
            ch->sendTo("%s:    1. Take only what you need and can use.\n\r",
                       o->getName());
            ch->sendTo("%s:    2. Please don't take things from surplus then sell them.\n\r",
                       o->getName());
            ch->sendTo("%s:    3. If you take from surpluse, please put something back at\n\r",
                       o->getName());
            ch->sendTo("%s:       a later date.\n\r",
                       o->getName());
            ch->sendTo("%s: To get to donation, please use the  goto surplus  command.\n\r",
                       o->getName());
          } else if (is_abbrev(Topic, "kill")) {
            ch->sendTo("%s: You use this command to start a fight.  It is usually a good idea\n\r",
                       o->getName());
            ch->sendTo("%s: to  consider  your target before attacking him.  And also a good\n\r",
                       o->getName());
            ch->sendTo("%s: idea to make sure you have all your equipment and any weapons you\n\r",
                       o->getName());
            ch->sendTo("%s: may have equiped.  Also a good idea to have used any of your\n\r",
                       o->getName());
            ch->sendTo("%s: practices you might have.\n\r",
                       o->getName());
          } else if (is_abbrev(Topic, "consider")) {
            ch->sendTo("%s: When you consider something the mud will tell you what your\n\r",
                       o->getName());
            ch->sendTo("%s: probable chances are at winning.  The consider command is not\n\r",
                       o->getName());
            ch->sendTo("%s: always accurate.  But it makes for a good idea of what level your\n\r",
                       o->getName());
            ch->sendTo("%s: target is.\n\r",
                       o->getName());
          } else if (is_abbrev(Topic, "goto")) {
            ch->sendTo("%s: While you're in Grimhaven you can use the goto command to get\n\r",
                       o->getName());
            ch->sendTo("%s: to the more important places.  These are the more important ones:\n\r",
                       o->getName());
            ch->sendTo("%s:    goto cs        : Center Square  [Food/Water]\n\r",
                       o->getName());
            ch->sendTo("%s:    goto mail      : Post office    [MudMail/You have mail!]\n\r",
                       o->getName());
            if (!ch->hasClass(CLASS_MONK))
            ch->sendTo("%s:    goto weapon    : Weapon Shop    [Swords/Daggers/Clubs]\n\r",
                       o->getName());
            if (!ch->hasClass(CLASS_MONK) && !ch->hasClass(CLASS_MAGIC_USER))
            ch->sendTo("%s:    goto armor     : Armor Shop     [Metallic Armor]\n\r",
                       o->getName());
            ch->sendTo("%s:    goto food      : Food Shop      [Provisions/Rations/Bread]\n\r",
                       o->getName());
            if (ch->hasClass(CLASS_CLERIC) || ch->hasClass(CLASS_DEIKHAN))
            ch->sendTo("%s:    goto symbol    : Symbol Shop    [Holy Symbols]\n\r",
                       o->getName());
            ch->sendTo("%s:    goto commodity : Commodity Shop [bits of tin/ingots of copper]\n\r",
                       o->getName());
            if (ch->hasClass(CLASS_MAGIC_USER) || ch->hasClass(CLASS_RANGER))
            ch->sendTo("%s:    goto component : Component Shop [Mage Spell Components]\n\r",
                       o->getName());
            ch->sendTo("%s:    goto doctor    : Doctor         [Heal bleeding Wounds]\n\r",
                       o->getName());
            if (ch->hasClass(CLASS_CLERIC) || ch->hasClass(CLASS_DEIKHAN))
            ch->sendTo("%s:    goto attuner   : Attuner        [Makes Holy Symbols Usable]\n\r",
                       o->getName());
            ch->sendTo("%s:    goto tanner    : Tanning Shop   [Leather Armor]\n\r",
                       o->getName());
            ch->sendTo("%s:    goto surplus   : Donation Room  [Surplus Items]\n\r",
                       o->getName());
            if (ch->GetMaxLevel() < 2)
            ch->sendTo("%s:    goto park      : Newbie Area    [Basic area for newbies]\n\r",
                       o->getName());
          } else return FALSE; // He didn't call on us for help, maybe another player?
          return TRUE; // If we got here, we had a topic so lets eat the command.
        }
      return FALSE;
    case CMD_LIST:
      if (!*Topic) {
        ch->sendTo("%s: You should use one of the following when using the list command:\n\r",
                   o->getName());
        ch->sendTo("%s: list fit    : To list only that which will fit you.\n\r",
                         o->getName());
        ch->sendTo("%s: list <slot> : To list armor by worn location\n\r",
                         o->getName());
        ch->sendTo("%s:   head, neck, body, back, arm, wrist, hand, waist, leg, feet, finger\n\r",
                         o->getName());
        ch->sendTo("%s: list #1 #2: To list armor which cost no more than #2 but at least #1\n\r",
                         o->getName());
        ch->sendTo("%s: And when buying, always be careful not to exceed your rent limit.\n\r",
                         o->getName());
      }
      return FALSE;
    case CMD_CONSIDER:
      if (!*Topic) {
        ch->sendTo("%s: It is very important to understand what this command tells you.\n\r",
                         o->getName());
        ch->sendTo("%s: If the outcome looks bad, you might not want to try it.  But always\n\r",
                         o->getName());
        ch->sendTo("%s: be wary no matter how easy a target may look.\n\r",
                         o->getName());
        ch->sendTo("%s: You can even consider yourself to get an idea of how good/bad your\n\r",
                         o->getName());
        ch->sendTo("%s: own armor is at the moment for your level.\n\r",
                         o->getName());
        sprintf(buf, "consider %s", ch->getName());
        ch->addCommandToQue(buf);
        return TRUE;
      }
    default:
      break;
  }

  return FALSE;
}

int maquahuitl(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  int randomizer = ::number(0,9);
  TGenWeapon *weapon = dynamic_cast<TGenWeapon *>(o);
  // Proc goes off like mad but damage is way minimal to produce the 
  // affect of a blunt item slashing

  if(!(ch=genericWeaponProcCheck(vict, cmd, weapon, 0)))
     return FALSE;

  if (randomizer >= 5) {
    weapon->setWeaponType(WEAPON_TYPE_SLASH);
  } else {
    weapon->setWeaponType(WEAPON_TYPE_SMITE);
  }
  return TRUE;
}

int bluntPierce(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  int randomizer = ::number(0,9);
  TGenWeapon *weapon = dynamic_cast<TGenWeapon *>(o);
  // Proc goes off like mad but damage is way minimal to produce the 
  // affect of a blunt item slashing

  if(!(ch=genericWeaponProcCheck(vict, cmd, weapon, 0)))
     return FALSE;

  if (randomizer >= 5) {
    weapon->setWeaponType(WEAPON_TYPE_PIERCE);
  } else {
    weapon->setWeaponType(WEAPON_TYPE_SMITE);
  }
  return TRUE;
}

int randomizer(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  int randomizer = ::number(0,9);
  TGenWeapon *weapon = dynamic_cast<TGenWeapon *>(o);
  // Proc goes off like mad but damage is way minimal to produce the 
  // affect of a blunt item slashing

  if(!(ch=genericWeaponProcCheck(vict, cmd, weapon, 0)))
     return FALSE;

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
  return TRUE;
}

int teleportRing(TBeing *, cmdTypeT cmd, const char *arg, TObj *o, TObj *){
  int rc;
  TBeing *vict;

  if (!(vict = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
 
  if(cmd!=CMD_GENERIC_PULSE || ::number(0,100))
    return FALSE;

  act("Your $o flares up brightly and you suddenly feel very dizzy and disoriented.", TRUE, vict, o, NULL, TO_CHAR);
  act("$n's $o flares up brightly and $e disappears!", TRUE, vict, o, NULL, TO_ROOM);
  
  rc = vict->genericTeleport(SILENT_NO, FALSE);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete vict;
    vict = NULL;
  }

  return TRUE;
}

int trolley(TBeing *, cmdTypeT cmd, const char *, TObj *myself, TObj *){  
  int *job=NULL, where=0, i;
  int path[]={-1, 100, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185,
	    200, 215, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659,
	    660, 667, 668, 669, 670, 671, 672, 673, 674, 700, 702,
	    703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714,
	    715, 716, 728, 729, 730, 731, 732, 733, 734, 735, 736, 737,
	    738, 739, 1381, 1200, 1201, 1204, 1207, 1215, 1218, 1221, 
	    1301, 1302, 1303, -1};
  TRoom *trolleyroom=real_roomp(15344);
  static int timer;
  char buf[256], shortdescr[256];

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<int *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if(!myself->in_room)
    return FALSE;

  if(timer>0){
    --timer;
    return FALSE;
  }

  strcpy(shortdescr, myself->shortDescr);
  cap(shortdescr);

  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new int)) {
     perror("failed new of trolley.");
     exit(0);
    }
    job = static_cast<int *>(myself->act_ptr);
    *job=1;
  } else {
    job = static_cast<int *>(myself->act_ptr);
  }

  for(where=1;path[where]!=-1 && myself->in_room != path[where];++where);

  if(path[where]==-1){
    vlogf(LOG_PEEL, "trolley lost");
    return FALSE;
  }

  if((path[where+*job])==-1){
    switch(*job){
      case -1:
	sendrpf(COLOR_OBJECTS, trolleyroom, "%s has arrived in Grimhaven.\n\r",
		shortdescr);
	break;
      case 1:
	sendrpf(COLOR_OBJECTS, trolleyroom, "%s has arrived in Brightmoon.\n\r",
		shortdescr);
	break;
    }

    *job=-*job;
    timer=10;
    return TRUE;
  }

  
  for(i=MIN_DIR;i<MAX_DIR;++i){
    if(myself->roomp->dir_option[i] &&
       myself->roomp->dir_option[i]->to_room==path[where+*job]){
      break;
    }
  }
  
  switch(*job){
    case -1: 
      sprintf(buf, "$n continues %s towards Grimhaven.",
	      (i==MAX_DIR)?"on":dirs[i]);
      act(buf,FALSE, myself, 0, 0, TO_ROOM); 
      sendrpf(COLOR_OBJECTS, trolleyroom, "%s rumbles %s towards Grimhaven.\n\r",
	      shortdescr, (i==MAX_DIR)?"on":dirs[i]);
      sendrpf(COLOR_OBJECTS, real_roomp(path[where+*job]), 
	      "%s enters the room, heading towards Grimhaven.\n\r",
	      shortdescr);
      break;
    case 1: 
      sprintf(buf, "$n continues %s towards Brightmoon.",
	      (i==MAX_DIR)?"on":dirs[i]);
      act(buf,FALSE, myself, 0, 0, TO_ROOM); 
      sendrpf(COLOR_OBJECTS, trolleyroom, "%s rumbles %s towards Brightmoon.\n\r",
	      shortdescr, (i==MAX_DIR)?"on":dirs[i]);
      sendrpf(COLOR_OBJECTS, real_roomp(path[where+*job]), 
	      "%s enters the room, heading towards Brightmoon.\n\r",
	      shortdescr);
      break;
  }
  
  --(*myself);
  *real_roomp(path[where+*job])+=*myself;

  if(!trolleyroom->dir_option[0]){
    trolleyroom->dir_option[0] = new roomDirData();
  }
  
  trolleyroom->dir_option[0]->to_room=path[where+*job];

  return TRUE;
}

int fishingBoat(TBeing *, cmdTypeT cmd, const char *, TObj *myself, TObj *)
{  
  int *job=NULL, where=0, i, found=1;
  int path[]={-1, 15150, 
	      2439, 2440, 2441, 2442, 2443, 2444, 2445, 2446, 2447, 2448,
	      2449, 2450, 2451, 2452, 2453, 2454, 2455, 2456, 2457, 2458,
	      2459, 2460, 2461, 2462, 2463, 2464, 2465, 2466, 2467, 2468,
	      2469, 2470, 2471, 2475, 12551, 12583, 12616, 12651, 12690,
	      12733, 12770, 12803, 12831, 12857, 12886, 12911, 12935, 12958,
	      12982, 13006, 13030, 13052, 13072, 13091, -1};
  const char *boatleaving[]={
                   "The fishing boat is going to be leaving immediately.\n\r",
		   "The fishing boat is almost ready to leave.\n\r",
		   "The fishing boat will be leaving very soon.\n\r",
		   "The fishing boat will be leaving soon.\n\r",
		   "The fishing boat is getting ready to leave.\n\r"};
  TRoom *boatroom=real_roomp(15349);
  static int timer;
  char buf[256], shortdescr[256];
  TThing *tt;

  // docks 15150

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<int *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if(!myself->in_room)
    return FALSE;

  if(::number(0,4)) // slow it down a bit, this is a fishing trip after all
    return FALSE;

  // we just idle in 15150 if we're empty
  if(myself->in_room == 15150){
    found=0;
    for(tt=boatroom->getStuff();tt;tt=tt->nextThing){
      if(dynamic_cast<TPerson *>(tt)){
	found=1;
	break;
      }
    }
    if(!found)
      return FALSE;
    else if(timer <= 0){
      if(myself->in_room == 15150){
	sendrpf(real_roomp(15150), "The fishing boat is preparing to leave.\n\r");
      } else if(myself->in_room == 13091){
	sendrpf(real_roomp(13108), "The fishing boat is preparing to leave.\n\r");
      }
      sendrpf(boatroom, "The fishing boat is preparing to leave.\n\r");
      
      timer=5;
    }
  }

  if((--timer)>0){

    if(timer<=5 && timer>0){
      if(myself->in_room == 15150){
	sendrpf(real_roomp(15150), boatleaving[timer-1]);
      } else if(myself->in_room == 13091){
	sendrpf(real_roomp(13108), boatleaving[timer-1]);
      }
      sendrpf(boatroom, boatleaving[timer-1]);
    }

    return FALSE;
  }

  strcpy(shortdescr, myself->shortDescr);
  cap(shortdescr);

  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new int)) {
     perror("failed new of fishing boat.");
     exit(0);
    }
    job = static_cast<int *>(myself->act_ptr);
    *job=1;
  } else {
    job = static_cast<int *>(myself->act_ptr);
  }

  for(where=1;path[where]!=-1 && myself->in_room != path[where];++where);

  if(path[where]==-1){
    vlogf(LOG_PEEL, "fishing boat lost");
    return FALSE;
  }

  if((path[where+*job])==-1){
    switch(*job){
      case -1:
	sendrpf(COLOR_OBJECTS, boatroom, "%s has arrived at the docks.\n\r",
		shortdescr);
	break;
      case 1:
	sendrpf(COLOR_OBJECTS, boatroom, "%s has arrived at the island.\n\r",
		shortdescr);
	timer=10;
	break;
    }

    *job=-*job;
    return TRUE;
  }

  
  for(i=MIN_DIR;i<MAX_DIR;++i){
    if(myself->roomp->dir_option[i] &&
       myself->roomp->dir_option[i]->to_room==path[where+*job]){
      break;
    }
  }
  
  switch(*job){
    case -1: 
      sprintf(buf, "$n continues %s towards the docks.",
	      (i==MAX_DIR)?"on":dirs[i]);
      act(buf,FALSE, myself, 0, 0, TO_ROOM); 
      sendrpf(COLOR_OBJECTS, boatroom, "%s sails %s towards land.\n\r",
	      shortdescr, (i==MAX_DIR)?"on":dirs[i]);
      sendrpf(COLOR_OBJECTS, real_roomp(path[where+*job]), 
	      "%s enters the room, heading towards land.\n\r",
	      shortdescr);
      break;
    case 1: 
      sprintf(buf, "$n continues %s out to sea.",
	      (i==MAX_DIR)?"on":dirs[i]);
      act(buf,FALSE, myself, 0, 0, TO_ROOM); 
      sendrpf(COLOR_OBJECTS, boatroom, "%s sails %s out to sea.\n\r",
	      shortdescr, (i==MAX_DIR)?"on":dirs[i]);
      sendrpf(COLOR_OBJECTS, real_roomp(path[where+*job]), 
	      "%s enters the room, heading out to sea.\n\r",
	      shortdescr);
      break;
  }
  
  --(*myself);
  *real_roomp(path[where+*job])+=*myself;

  if(!boatroom->dir_option[0]){
    boatroom->dir_option[0] = new roomDirData();
  }
  
  boatroom->dir_option[0]->to_room=path[where+*job];

  return TRUE;
}


int squirtGun(TBeing *vict, cmdTypeT cmd, const char *Parg, TObj *o, TObj *)
{
  TBeing *ch;
  char //Command[30], // Should be 'squirt'
    Target[30]; // target to be soaked!
    
  TDrinkCon *gun=dynamic_cast<TDrinkCon *>(o);

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (((o->equippedBy != ch) && (o->parent != ch)))
    return FALSE;
  Parg = one_argument(Parg, Target);
  if (!(cmd == CMD_SHOOT))
    return FALSE;
  if (!gun) {
    vlogf(LOG_PROC, "Squirt Gun proc on an object that isn't a drink container.");
    return FALSE;
  }
  
  if(gun->getDrinkUnits() < 1) {
    act("<1>You squeeze the trigger with all your might, but $p appears to be empty.",TRUE,ch,o,vict,TO_CHAR,NULL);  
    act("<1>$n squeezes the trigger on $s $p, but nothing happens.",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    return TRUE;
  }
  else {
    TBeing *squirtee;
    int bits = generic_find(Target, FIND_CHAR_ROOM, ch, &squirtee, &o);
    if(!bits) {
      ch->sendTo("You don't see them here.\n\r");
      return TRUE;
    } else {
      const char *liqname =DrinkInfo[gun->getDrinkType()]->name;
      int shot = (::number(1,min(5,gun->getDrinkUnits())));
      gun->addToDrinkUnits(-shot);
      ch->dropPool(shot, gun->getDrinkType());
      
      /*act("<1>You squeeze the trigger on your $p.",TRUE,ch,gun,squirtee,TO_CHAR,NULL);
	ch->sendTo(COLOR_OBJECTS, "A deadly stream of %s squirts at %s!\n\r",liqname, squirtee->getName());
	act("<1>$n squeezes the trigger on $s $p, shooting a deadly stream of liquid at $N!"
	,TRUE,ch,gun,squirtee,TO_NOTVICT,NULL);
	
	act("<1>$n squeezes the trigger on $s $p.",TRUE,ch,gun,squirtee,TO_VICT,NULL);
	squirtee->sendTo(COLOR_OBJECTS, "A deadly stream of %s squirts at you!\n\r",liqname);
      */
      char Buf[256];
      sprintf(Buf, "You squeeze the trigger on $p, squirting a deadly stream of %s at $N!", liqname);
    act(Buf, TRUE, ch, gun, squirtee, TO_CHAR);
    sprintf(Buf, "$n squeezes the trigger on $p, squirting a deadly stream of %s at $N!", liqname);
    act(Buf, TRUE, ch, gun, squirtee, TO_NOTVICT);
    sprintf(Buf, "$n squeezes the trigger on $p, squirting a deadly stream of %s at you!", liqname);
    act(Buf, TRUE, ch, gun, squirtee, TO_VICT);
    if (shot>4) {
    char Buf2[256];
    sprintf(Buf2, "$N is totally soaked with %s!", liqname);
    act(Buf2, TRUE, ch, gun, squirtee, TO_CHAR);
    act(Buf2, TRUE, ch, gun, squirtee, TO_NOTVICT);
    sprintf(Buf2, "You're totally soaked with %s!", liqname);
    act(Buf2, TRUE, ch, gun, squirtee, TO_VICT);
    }
    return TRUE;
  }
}
 return FALSE;
}

int fireGlove(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam = 1;

  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (::number(0,5))
    return FALSE;
  if (cmd != CMD_OBJ_HIT)
    return FALSE;
  dam = (::number( 1, (ch->GetMaxLevel()) / 10 + 2));

  act("<o>Your $o bursts into <r>flame<1><o> as you strike $N<1><o>!<1>",
      TRUE,ch,o,vict,TO_CHAR,NULL);
  act("<o>$n's $o bursts into <r>flame<1><o> as $e strikes $N<1><o>!<1>",
      TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("<o>$n'a $o bursts into <r>flame<1><o> as $e strikes you!<1>",
      TRUE,ch,o,vict,TO_VICT,NULL);
  
  rc = ch->reconcileDamage(vict, dam, DAMAGE_FIRE);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}


int razorGlove(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam = 1, which;
  
  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (::number(0,5))
    return FALSE;
  if (cmd != CMD_OBJ_HIT)
    return FALSE;
  dam = (::number( 1, (ch->GetMaxLevel()) / 10 + 2));
  which = ::number(1,2);

  act("<k>Three long, thin blades spring from your <1>$o<k>.<1>",
      TRUE,ch,o,vict,TO_CHAR,NULL);
  act("<k>Three long, thin blades spring from $n's <1>$o<k>.<1>",
      TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("<k>Three long, thin blades spring from $n's <1>$o<k>.<1>",
      TRUE,ch,o,vict,TO_VICT,NULL);


  if (which == 1) {
    act("<k>You <g>slice<k> $N with the <1>blades<k> of your $o, which then retract.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<k>$n <1>slices<k> $N with the <1>blades<k> of $s $o, which then retract.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<k>$n <r>slices<k> you with the <1>blades<k> of $s $o, which then retract.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
  }
  else {
    act("<k>You <g>stab<k> $N with the <1>blades<k> of your $o, which then retract.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<k>$n <1>stabs<k> $N with the <1>blades<k> of $s $o, which then retract.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<k>$n <r>stabs<k> you with the<1> blades<k> of $s $o, which then retract.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
  }

  rc = ch->reconcileDamage(vict, dam, TYPE_PIERCE);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}

int keyInKnife(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *) 
{
  if (!(ch = dynamic_cast<TBeing *>(o->parent)))
    return FALSE;
  TObj *key = NULL; 
  char buf[256];

  if (cmd != CMD_PUSH && cmd != CMD_PRESS) 
    return FALSE;
 
  if (!(key = read_object(17211, VIRTUAL))) {
    vlogf(LOG_PROC, "Key in Knife -- bad read of object (%s)", ch->getName());
    return FALSE;
  }
 
  one_argument(arg, buf);
  
  
  if(!strcmp(buf,"panel")) {
    *ch += *key;
    act("You press on the hilt of $p. *click*",TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("The entire $o splits down the center, revealing a <Y>golden key<1>.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("$n fiddles with the hilt of $p. *click*.",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("The entire $o splits down the center, revealing a small <Y>golden<1> object.",TRUE,ch,o,NULL,TO_ROOM,NULL);
    o->makeScraps();
    delete o;
    return TRUE;
  }
  return FALSE;
}


int teleportVial(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *) 
{
  int targetroom;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  char objname[256],buf[256];
  if (cmd != CMD_THROW) 
    return FALSE;
  strcpy(objname,o->getName());
  one_argument(one_argument(one_argument(objname,buf),buf),buf); //vial

  if (sscanf(buf,"%d",&targetroom) != 1) {
    act("Teleport vial with no target room. How crappy.",TRUE,ch,o,NULL,TO_CHAR,NULL);
  } else {
    TRoom *newRoom;
    if (!(newRoom = real_roomp(targetroom))) {
      act("Teleport vial targeted to a non-existant room. How crappy.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      return TRUE;
    }
    act("You throw $p to the ground.",TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("The $o shatters, releasing a cloud of thick smoke all around you.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    
    act("$n throws $p to the ground.",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("The $o shatters, releasing a cloud of thick smoke all around $m.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    
    
    act("You feel the world shift around you.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    act("When the smoke clears, $n is gone!<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    --(*ch);
    delete o;  
    *newRoom += *ch;
    vlogf(LOG_PROC, "TELEPORT VIAL: %s transfered to room #%d", ch->getName(), targetroom);
    act("$n appears in the room with a puff of smoke.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    ch->doLook("", CMD_LOOK);
    ch->addToWait(combatRound(2));
    ch->cantHit += ch->loseRound(1);
    if (ch->riding) {
      int rc = ch->fallOffMount(ch->riding, POSITION_STANDING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	return DELETE_THIS;
      }
    }
    return TRUE;
  }
  return FALSE;
}




int blazeOfGlory(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  // Use of this quest prize weapon takes two seprate actions. One is a sort of
  // 'charge-up' move, where the char steps back from combat, and focuses on
  // charging up the weapon.
  // the release part of the proc happens on a normal swing attempt. The energy
  // stored in the first part is released in a giant ball of fire, consuming the
  // char, and hopefully his victim as well. Kaboooom!   -love, Dash
  int dam = 0, rc;
  TBeing *ch;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  
  if (cmd == CMD_OBJ_HITTING && ch->checkForSkillAttempt(SPELL_BLAST_OF_FURY)) {
      act("<o>The air about you seems to crackle with power as you level your $o at $N<o>.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
      act("<o>Brandishing $p<o>, you prepare to charge!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
      act("  ...as you begin your rush, flames spread out from your $o, and envelope you...",TRUE,ch,o,vict,TO_CHAR,NULL);
      act("    ...your vision begins to go red...",TRUE,ch,o,vict,TO_CHAR,NULL);
      act("      ...white hot flames tear across your entire body, the pain is unbearable...",TRUE,ch,o,vict,TO_CHAR,NULL);
      act("        ...with $N just ahead of you, you prepare your $o for impact...",TRUE,ch,o,vict,TO_CHAR,NULL);


      act("<o>The air about $n<o> seems to crackle with power as $e levels $s $o at $N.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
      act("<o>Brandishing $p<o>, $n<o> prepares to charge!<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
      act("  ...flames spread out from $s $o, and envelope $m...",TRUE,ch,o,vict,TO_NOTVICT,NULL);
      act("    ...$n screams with rage as $e barrels at $N...",TRUE,ch,o,vict,TO_NOTVICT,NULL);
      act("      ...white hot flames tear across $s entire body...",TRUE,ch,o,vict,TO_NOTVICT,NULL);
      act("        ...you see $n prepare $s $o for impact...",TRUE,ch,o,vict,TO_NOTVICT,NULL);


      act("<o>The air about $n<o> seems to crackle with power as $e levels $s $o at you. Uh oh.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
      act("<o>Brandishing $p<o>, $n<o> prepares to charge!<1>",TRUE,ch,o,vict,TO_VICT,NULL);
      act("  ...flames spread out from $s $o, and envelope $m...",TRUE,ch,o,vict,TO_VICT,NULL);
      act("    ...$n screams with rage as $e barrels at you...",TRUE,ch,o,vict,TO_VICT,NULL);
      act("      ...white hot flames tear across $s entire body...",TRUE,ch,o,vict,TO_VICT,NULL);
      act("        ...you see $n prepare $s $o for impact...",TRUE,ch,o,vict,TO_VICT,NULL);

      dam = min(30000,ch->hitLimit() * 100); // kill them DEAD. I want NO survivors.
      o->addToStructPoints(-5);
      o->setDepreciation(o->getDepreciation() + 5);
      if (o->getStructPoints() <= 0) {
	o->makeScraps();
	delete o;
      }
      int rc2 = ch->reconcileDamage(ch, dam, DAMAGE_FIRE);
      act("<R>KA-BOOOOOOOOOOM! You explode in a <O>blaze of glory<R> as you crash into $N<R>!!!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
      act("<R>KA-BOOOOOOOOOOM! $n explodes in a <O>blaze of glory<R> as $e crashes into $N<R>!!!<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
      act("<R>KA-BOOOOOOOOOOM! You're covered in <O>searing flames<R> as $e crashes into you!!!<1>",TRUE,ch,o,vict,TO_VICT,NULL);
    act("<o>With a loud crack, $n<o>'s corpse shatters!<1>",TRUE,ch,o,vict,TO_ROOM,NULL);


    dam = min(30000,ch->GetMaxLevel() * 40);
    rc = ch->reconcileDamage(vict, dam, DAMAGE_FIRE);

    ch->makeBodyPart(WEAR_HEAD);
    ch->makeBodyPart(WEAR_ARM_L);
    ch->makeBodyPart(WEAR_ARM_R);
    ch->makeBodyPart(WEAR_LEGS_L);
    ch->makeBodyPart(WEAR_LEGS_R);
    //muahaha corpse explodes... limbs fly EVERYWHERE!!
    if (IS_SET_DELETE(rc2, DELETE_VICT) || (ch->getHit() < -10))
      delete ch;

    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
    return TRUE;
    
  } else if ((cmd == CMD_SAY || cmd == CMD_SAY2) && !(ch->checkForSkillAttempt(SPELL_BLAST_OF_FURY))) {
    affectedData aff;
    char buf[256];
      
    one_argument(arg, buf);
    if(!strcmp(buf, "aerolithe")) {  //this is the activation keyword
      aff.type = AFFECT_SKILL_ATTEMPT;
      aff.level = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = 0;
      aff.duration = 20; 
      aff.modifier = SPELL_BLAST_OF_FURY;
      ch->affectTo(&aff);
      ch->addToWait(combatRound(4));
      ch->cantHit += ch->loseRound(3);
      
      act("$n holds $p high above $s head, shouting <p>a word of power<1>.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("A gust of scorching wind whips past $m, and flames lick out from $s $o.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<o>The air crackles with intense power as $n<o> is suddenly immolated in <r>flames<o>!<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);

      act("You hold $p high above your head, shouting the <p>word of power<1>, <p>Aerolithe<1>!",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("A gust of scorching wind whips past you, and flames lick out from your $o.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<o>The air crackles with intense power as you are suddenly immolated in <r>flames<o>!<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);

      dam = ch->hitLimit()/10;
      rc = ch->reconcileDamage(ch, dam, DAMAGE_FIRE);
      if (IS_SET_DELETE(rc, DELETE_VICT) || (ch->getHit() < -10))
	delete ch;
      return TRUE;
    }
    return FALSE;
  } if (cmd == CMD_GENERIC_PULSE && ch->checkForSkillAttempt(SPELL_BLAST_OF_FURY)) {
    act("<r>$n<r> seems to shudder in pain, and there is a strange fire burning in $s eyes.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("<r>The power contained within you is almost unbearable!<R> You're burning up!!!<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    dam = ch->hitLimit()/10;
    rc = ch->reconcileDamage(ch, dam, DAMAGE_FIRE);
    if (IS_SET_DELETE(rc, DELETE_VICT) || (ch->getHit() < -10))
      delete ch; 
    return TRUE;
  }
  return FALSE;
}

int mechanicalWings(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (cmd == CMD_FLY && ch->getPosition() == POSITION_STANDING) {
    act("<k>$n<k>'s $o silently unfold and begin to beat the air forcefully.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("<k>Your $o silently unfold and begin to beat the air forcefully.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    return FALSE;
  } else if (cmd == CMD_LAND && ch->getPosition() == POSITION_FLYING) {
    act("<k>$n<k>'s $o stop beating the air and silently fold behind $s back.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("<k>Your $o stop beating the air and silently fold behind your back.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    return FALSE;
  } else if ((cmd == CMD_GENERIC_PULSE) && (::number(1,10) == 1) && (ch->getPosition() == POSITION_FLYING)) {
    act("<k>The $o on $n<k>'s back forcefully beat the air.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("<k>The $o on your back forcefully beat the air.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    return FALSE;
  }
  return FALSE;
}


int elementalWeapon(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  int dam = 0, rc = 0;
  TBeing *ch;
  //blaaaah blah blah
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (cmd == CMD_OBJ_HIT && genericWeaponProcCheck(vict, cmd, o, 3)) {
    dam = ::number(4,10);
    if(ch->affectedBySpell(SPELL_CONJURE_WATER)) {
      if (dam < 8) {
	act("<b>Your <c>$o <b>becomes covered with ice and freezes $N.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
	act("<b>$n's <c>$o <b>becomes covered with ice and freezes $N.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
	act("<b>$n's <c>$o <b>becomes covered with ice and freezes you.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
      } else {
	act("<B>Your <C>$o <B>becomes covered with ice and sends a violent chill through $N.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
	act("<B>$n's <C>$o <B>becomes covered with ice and sends a violent chill through $N.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
	act("<B>$n's <C>$o <B>becomes covered with ice and sends a violent chill through you.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
      }
      rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
    } else if(ch->affectedBySpell(SPELL_CONJURE_FIRE)) {
      if (dam < 8) {
	act("<o>Your <r>$o <o>erupts into roaring flames and sears $N.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
	act("<o>$n's <r>$o <o>erupts into roaring flames and sears $N.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
	act("<o>$n's <r>$o <o>erupts into roaring flames and sears you.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
      } else {
	act("<O>Your <R>$o <O>roars into a blaze of fire and scorches $N.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
	act("<O>$n's <R>$o <O>roars into a blaze of fire and scorches $N.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
	act("<O>$n's <R>$o <O>roars into a blaze of fire and scorches you.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
      }
      rc = ch->reconcileDamage(vict, dam, DAMAGE_FIRE);
    } else if(ch->affectedBySpell(SPELL_CONJURE_AIR)) {
      if (dam < 8) {
	act("<c>Your <w>$o <c>crackles with electricity and shocks $N.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
	act("<c>$n's <w>$o <c>crackles with electricity and shocks $N.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
	act("<c>$n's <w>$o <c>crackles with electricity and shocks you.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
      } else {
	act("<C>Your <W>$o <C>discharges a large jolt of electricity at $N.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
	act("<C>$n's <W>$o <C>discharges a large jolt of electricity at $N.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
	act("<C>$n's <W>$o <C>discharges a large jolt of electricity at you.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
      }
      rc = ch->reconcileDamage(vict, dam, DAMAGE_ELECTRIC);
    } else return FALSE;
    if (rc == -1)
      return DELETE_VICT;
    return TRUE;
  } else if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    affectedData aff, aff2;
    char buf[256];
    
    one_argument(arg, buf);
    if(!strcmp(buf, "rime")) {  //this is the activation keyword
      if(ch->checkForSkillAttempt(SPELL_CONJURE_WATER)) {
	act("The $o's power of ice can only be used once a day!",TRUE,ch,o,NULL,TO_CHAR,NULL);
	return TRUE;
      } else if(ch->affectedBySpell(SPELL_CONJURE_FIRE)) {
	act("The $o's power of ice cannot be used at the same time as the power of fire!",TRUE,ch,o,NULL,TO_CHAR,NULL);
	return TRUE;
      } else if(ch->affectedBySpell(SPELL_CONJURE_AIR)) {
	act("The $o's power of ice cannot be used at the same time as the power of lightning!",TRUE,ch,o,NULL,TO_CHAR,NULL);
	return TRUE;
      }
      aff.type = AFFECT_SKILL_ATTEMPT;
      aff.level = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = 0;
      aff.duration = 24 * UPDATES_PER_MUDHOUR;
      aff.modifier = SPELL_CONJURE_WATER;
      aff2.type = SPELL_CONJURE_WATER;
      aff2.level = 0;
      aff2.location = APPLY_NONE;
      aff2.bitvector = 0;
      aff2.duration = 2 * UPDATES_PER_MUDHOUR;
      aff2.modifier = 0;
      if (!(ch->isImmortal())) ch->affectTo(&aff);
      ch->affectTo(&aff2);
      ch->addToWait(combatRound(2));
      
      act("$n brandishes $p, shouting a strange <p>word of power<1>.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<b>A chill wind swirls around $n<b>, and $s <B>$o<1><b> forms a thin layer of ice<1>.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      
      
      act("You brandish $p, shouting the command word, <p>rime<1>!",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<b>A chill wind swirls around you, and your <B>$o<1><b> forms a thin layer of ice<1>.",TRUE,ch,o,NULL,TO_CHAR,NULL);
    } else if(!strcmp(buf, "incandesce")) {  //this is the activation keyword
      if(ch->checkForSkillAttempt(SPELL_CONJURE_FIRE)) {
	act("The $o's power of fire can only be used once a day!",TRUE,ch,o,NULL,TO_CHAR,NULL);
	return TRUE;
      } else if(ch->affectedBySpell(SPELL_CONJURE_WATER)) {
	act("The $o's power of fire cannot be used at the same time as the power of ice!",TRUE,ch,o,NULL,TO_CHAR,NULL);
	return TRUE;
      } else if(ch->affectedBySpell(SPELL_CONJURE_AIR)) {
	act("The $o's power of fire cannot be used at the same time as the power of lightning!",TRUE,ch,o,NULL,TO_CHAR,NULL);
	return TRUE;
      }
      aff.type = AFFECT_SKILL_ATTEMPT;
      aff.level = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = 0;
      aff.duration = 24 * UPDATES_PER_MUDHOUR;
      aff.modifier = SPELL_CONJURE_FIRE;
      aff2.type = SPELL_CONJURE_FIRE;
      aff2.level = 0;
      aff2.location = APPLY_NONE;
      aff2.bitvector = 0;
      aff2.duration = 2 * UPDATES_PER_MUDHOUR;
      aff2.modifier = 0;
      if (!(ch->isImmortal())) ch->affectTo(&aff);
      ch->affectTo(&aff2);
      ch->addToWait(combatRound(2));
      
      act("$n brandishes $p, shouting a strange <p>word of power<1>.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<r>A scorching wind swirls around $n<r>, and $s <R>$o<1><r> bursts into flame<1>.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      
      act("You brandish $p, shouting the command word, <p>incandesce<1>!",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<r>A scorching wind swirls around you, and your <R>$o<1><r> bursts into flame<1>.",TRUE,ch,o,NULL,TO_CHAR,NULL);
    } else if(!strcmp(buf, "evoke")) {  //this is the activation keyword
      if(ch->checkForSkillAttempt(SPELL_CONJURE_AIR)) {
	act("The $o's power of lightning can only be used once a day!",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return TRUE;
      } else if(ch->affectedBySpell(SPELL_CONJURE_FIRE)) {
        act("The $o's power of lightning cannot be used at the same time as the power of fire!",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return TRUE;
      } else if(ch->affectedBySpell(SPELL_CONJURE_WATER)) {
        act("The $o's power of lightning cannot be used at the same time as the power of ice!",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return TRUE;
      } else 
        aff.type = AFFECT_SKILL_ATTEMPT;
      aff.level = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = 0;
      aff.duration = 24 * UPDATES_PER_MUDHOUR;
      aff.modifier = SPELL_CONJURE_AIR;
      aff2.type = SPELL_CONJURE_AIR;
      aff2.level = 0;
      aff2.location = APPLY_NONE;
      aff2.bitvector = 0;
      aff2.duration = 2 * UPDATES_PER_MUDHOUR;
      aff2.modifier = 0;
      if (!(ch->isImmortal())) ch->affectTo(&aff);
      ch->affectTo(&aff2);
      ch->addToWait(combatRound(2));
      
      act("$n brandishes $p, shouting a strange <p>word of power<1>.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<c>A charged wind swirls around $n<c>, and $s <C>$o<1><c> releases a shower of sparks.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
      
      act("You brandish $p, shouting the command word, <p>evoke<1>!",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<c>A charged wind swirls around you, and your <C>$o<1><c> releases a shower of sparks.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    } else 
      return FALSE;
    return TRUE;
  } if (cmd == CMD_GENERIC_PULSE && ::number(1,6) == 1) {
    if(ch->affectedBySpell(SPELL_CONJURE_WATER)) {
      act("<b>A few ice crystals break off from $n<b>'s <B>$o<1><b> and drift to the ground.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<b>A few ice crystals break off from your <B>$o<1><b> and drift to the ground.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    } else if(ch->affectedBySpell(SPELL_CONJURE_FIRE)) {
      act("<r>$n<r>'s <R>$o<1><r> flares up momentarily, releasing a blast of heat.<1><1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<r>Your <R>$o<1><r> flares up momentarily, releasing a blast of heat.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
    } else if(ch->affectedBySpell(SPELL_CONJURE_AIR)) {
      act("<c>$n<c>'s <C>$o<1><c> literally hums with power, releasing a few sparks into the air.<1><1>",
          TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<c>Your <C>$o<1><c> literally hums with power, releasing a few sparks into the air.<1>",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
    } else return FALSE;
    return TRUE;
  }
  return FALSE;
}

int weaponShadowSlayer(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam = 1;
  //  ch = genricWeaponProcCheck(vict,cmd,o,5);
  //  if ((!(ch)) || !(vict->getFaction() == FACT_CULT || vict->isUndead()))
  //    return FALSE;
  //  
  
  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (::number(0,6))
    return FALSE;
  if (cmd != CMD_OBJ_HIT)
    return FALSE;
  if (!(vict->getFaction() == FACT_CULT || vict->isUndead()))
    return FALSE;
  int hitterLev = ch->GetMaxLevel();
  dam = (::number((hitterLev / 10 + 1),(hitterLev / 3 + 4)));  
  act("<1>Your $o hums, and begins to glow with an incredible <W>white light<1>.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
  act("<1>$n's $o hums, and begins to glow with an incredible <W>white light<1>.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("<1>$n's $o hums, and begins to glow with a painful <W>white light<1>.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
  
  if (dam >= ( ( ((hitterLev/3+4)-(hitterLev/10+1))*4 )/5 + (hitterLev/10+1))) {
    act("<W>$N howls in pain as a HUGE flash of energy from your $o is released into $m! <1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<W>$N howls in pain as a HUGE flash of energy from $n's $o is released into $m!<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<W>There is a huge flash as the energy from $n's $o is released into you!<1>  That really hurt!!<1>",TRUE,ch,o,vict,TO_VICT,NULL);
  } else {
    act("<W>$N grunts as the energy from your $o is released into $m.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<W>$N grunts as the energy from $n's $o is released into $m.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<W>You grunt in pain as the energy from $n's blasted $o is released into you.<1>",TRUE,ch,o,vict,TO_VICT,NULL);
  }
  
  if (!(ch->getFaction() == FACT_BROTHERHOOD)) {
    dam = dam/2;
    act("<1>Your $o rebels against you, releasing <W>energy<1> into your hand!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<1>$n's $o rebels against $m, releasing <W>energy<1> into $s hand!<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
    act("<1>$n's $o rebels against $m, releasing <W>energy<1> into $s hand!  Sucker!<1>",TRUE,ch,o,vict,TO_VICT,NULL);
    rc = ch->reconcileDamage(ch, dam, TYPE_SMITE);
    if (ch->getHit() < 0) {
      ch->setHit(0);
      ch->setPosition(POSITION_STUNNED);
    }
    if (!ch->isTough()) {
      *ch->roomp += *ch->unequip(o->eq_pos);
      act("$n screams loudly, dropping $s $p.", 1, ch, o, NULL, TO_ROOM);
      act("You scream loudly, dropping your $p.", 1, ch, o, NULL, TO_CHAR);
    }
  }
  
  
  rc = ch->reconcileDamage(vict, dam, TYPE_SMITE);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}


int stoneSkinAmulet(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    char buf[256];
    one_argument(arg, buf);
    if(!strcmp(buf, "fortify")) {
      if(ch->affectedBySpell(SPELL_FLAMING_FLESH)) {
        act("The $o's cannot function while you are affected by flaming flesh.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return FALSE;
      } else if(ch->checkForSkillAttempt(SPELL_STONE_SKIN)) {
        act("The $o's powers can only be used once per day.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return FALSE;
      }
      affectedData aff1, aff2, aff3;
      
      act("$n grips $p in one hand, and utters the word, '<p>fortify<1>'.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<k>The $o glows for a moment, and $s skin suddenly turns rock hard.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
      
      act("You grip $p in one hand, and utter the word, '<p>fortify<1>'.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<k>The $o glows for a moment, and your skin suddenly turns rock hard.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);
      
      // ARMOR APPLY
      aff1.type = SPELL_STONE_SKIN;
      aff1.level = 30;
      aff1.duration = 8 * UPDATES_PER_MUDHOUR;
      aff1.location = APPLY_ARMOR;
      aff1.modifier = -75;
      
      // PIERCE IMMUNITY
      aff2.type = SPELL_STONE_SKIN;
      aff2.level = 30;
      aff2.duration = 8 * UPDATES_PER_MUDHOUR;
      aff2.location = APPLY_IMMUNITY;
      aff2.modifier = IMMUNE_PIERCE;
      aff2.modifier2 = 15;
      
      // SKILL ATTEMPT (PREVENT IMMEDIATE RE-USE)
      aff3.type = AFFECT_SKILL_ATTEMPT;
      aff3.level = 0;
      aff3.duration = 24 * UPDATES_PER_MUDHOUR;
      aff3.location = APPLY_NONE;
      aff3.modifier = SPELL_STONE_SKIN;
      
      ch->affectTo(&aff1);
      ch->affectTo(&aff2);
      if (!(ch->isImmortal())) ch->affectTo(&aff3);
      ch->addToWait(combatRound(3));
      return TRUE;
    }
  }
  return FALSE;
}
  

int lifeLeechGlove(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  char target[30];
  TBeing *victim = NULL;
  TObj *corpse = NULL;
  TBaseCorpse *body = NULL;

  if (cmd != CMD_GRAB)
    return FALSE;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  arg = one_argument(arg, target);
  int bits = generic_find(target, FIND_CHAR_ROOM | FIND_OBJ_ROOM, ch, &victim, &corpse);
  if(!bits)
    return FALSE;
  if(victim == ch) {
    act("Dude... like, no.",TRUE,ch,o,victim,TO_CHAR,NULL);
    return FALSE;
  }

  if(victim) {
    int chance = 0,roll = 0;
    chance = victim->GetMaxLevel() + 30;
    roll = ::number(1,50+ch->GetMaxLevel());
    if (chance > roll && victim->getPosition() > POSITION_SLEEPING) {
      act("You try to grab $N, but $E dodges out of the way.",TRUE,ch,o,victim,TO_CHAR,NULL);
      act("$n tries to grab you, but you dodge out of the way.",TRUE,ch,o,victim,TO_VICT,NULL);
      act("$n tries to grab $N, but $E dodges out of the way.",TRUE,ch,o,victim,TO_NOTVICT,NULL);
      ch->addToWait(combatRound(3));
      ch->cantHit += ch->loseRound(2);
      return TRUE;
    } else if (victim->isUndead()) {  // heheh trying to drain negative plane monsters is BAD!!
      act("You deftly grab $N, and your $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_CHAR,NULL);
      act("<k>You scream in pain as your life is sucked backwards through the conduit!<1>",TRUE,ch,o,victim,TO_CHAR,NULL);
      act("Maybe that wasn't such a good idea....",TRUE,ch,o,victim,TO_CHAR,NULL);
      act("$n deftly grabs you, and $s $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_VICT,NULL);
      act("<k>$n<k> screams in pain as $s life is sucked backwards through the conduit!<1>",TRUE,ch,o,victim,TO_VICT,NULL);
      act("Heh heh heh. That sucker tried to drain an undead!",TRUE,ch,o,victim,TO_VICT,NULL);
      act("$n deftly grabs $N, and $s $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_NOTVICT,NULL);
      act("<k>$n<k> screams in pain as $s life is sucked backwards through the conduit!<1>",TRUE,ch,o,victim,TO_NOTVICT,NULL);       
      int dam = victim->GetMaxLevel();
      int rc = victim->reconcileDamage(ch, dam, DAMAGE_DRAIN);
      victim->setHit(min((int)(victim->getHit() + victim->GetMaxLevel()),(int)(victim->hitLimit())));
      if (rc == -1)
        delete victim;
      ch->addToWait(combatRound(3));
      ch->cantHit += ch->loseRound(2);
      return TRUE;
    }
    act("You deftly grab $N, and your $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_CHAR,NULL);
    act("<k>$N<k> screams in pain as you leech the life from $S body!<1>",TRUE,ch,o,victim,TO_CHAR,NULL);
    act("$n deftly grabs you, and $s $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_VICT,NULL);
    act("<k>You scream in pain as $n<k> leeches the life from your body!<1>",TRUE,ch,o,victim,TO_VICT,NULL);
    act("$n deftly grabs $N, and $s $o begins to glow with a <r>sickly light<1>.",TRUE,ch,o,victim,TO_NOTVICT,NULL);
    act("<k>$N<k> screams in pain as $n<k> leeches the life from $S body!<1>",TRUE,ch,o,victim,TO_NOTVICT,NULL);       
    int dam = victim->GetMaxLevel();
    int rc = ch->reconcileDamage(victim, dam, DAMAGE_DRAIN);
    if (!ch->isUndead()) {
      dam = dam/5;
    }
    ch->setHit(min((int)(ch->getHit() + dam),(int)(ch->hitLimit())));
    if (rc == -1)
      delete victim;
    ch->addToWait(combatRound(3));
    ch->cantHit += ch->loseRound(2);
    return TRUE;
  }
  if ((body = dynamic_cast<TBaseCorpse *>(corpse))) {
    if (corpse->getMaterial() == MAT_POWDER || body->getCorpseLevel() <= 0) {
      act("There is no life left in $N to leech!",TRUE,ch,o,body,TO_CHAR,NULL);
      return TRUE;
    }
    act("You place your hand over $N, and your $o begins to glow with <r>sickly light<1>.",TRUE,ch,o,body,TO_CHAR,NULL);
    act("<k>As you leech life, $N<k> visibly withers and begins to decompose rapidly.<1>",TRUE,ch,o,body,TO_CHAR,NULL);
    
    act("$n places $s hand over $N, and $s $o begins to glow with <r>sickly light<1>.",TRUE,ch,o,body,TO_ROOM,NULL);
    act("<k>As $e leechs life, $N<k> visibly withers and begins to decompose rapidly<1>.",TRUE,ch,o,body,TO_ROOM,NULL);
    
    body->obj_flags.decay_time = 0;
    ch->setHit(min((int)(ch->getHit() + body->getCorpseLevel()),(int)(ch->hitLimit())));
    body->setCorpseLevel(0);
    
    ch->addToWait(combatRound(3));
    ch->cantHit += ch->loseRound(2);
    
    return TRUE;
  }
  return FALSE;
}

int manaBurnRobe(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *) {
#if 0
  
    TBeing *ch;

    if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
      return FALSE;

    if (cmd == CMD_SAY || cmd == CMD_SAY2) {
      char buf[256];
      one_argument(arg, buf);
      if(!strcmp(buf, "manifest")) {
	double currentMana = ch-> getMana();
	double percentBurn = ch->hitLimit() * .2;
	double healthSteal = min((ch->hitLimit()- percentBurn),(ch->getHit() - percentBurn));
	double manaGain = healthSteal * .8;
	//	ch->setManaLimit(currentMana + manaGain);
	ch->setHit(healthSteal);
	act ("Your robe begins to glow with an eerie <b>light<1>, thin tendrils of light thrash wildly and then burrow into your skin, you scream as they rip the lifeforce from you.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);
	act ("&n screams in agony as thin <b>tendrils emerge from his robe and burrow into his skin!",TRUE,ch,NULL,NULL,TO_ROOM,NULL);
	return TRUE;
      }
    }

#endif
    return FALSE;

  } // end manaBurnRobe

int healingNeckwear(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  TBeing *tmp;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (!(tmp = dynamic_cast<TBeing *>(me->equippedBy)))
    return FALSE;

  // if they areholding it, don't be silly
  if (me->eq_pos != WEAR_NECK)
    return FALSE;

  if (!tmp->roomp)
    return FALSE;

  if (!tmp->isPc())
    return FALSE;

  act("$p constricts about your throat!", 0, tmp, me, 0, TO_CHAR);
  act("$p constricts about $n's throat!", 0, tmp, me, 0, TO_ROOM);
  if (tmp->hasClass(CLASS_SHAMAN)) {
    tmp->addToHit(::number(1, (tmp->getHit() / 3)));
    act("The power of the loa enters your body through $p!", 0, tmp, me, 0, TO_CHAR);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears from $n's throat!", 0, tmp, me, 0, TO_ROOM);
    delete me;
  } else {
    int rc;
    rc = tmp->applyDamage(tmp, ::number(1,10), DAMAGE_SUFFOCATION);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears from $n's throat!", 0, tmp, me, 0, TO_ROOM);
    delete me;
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete tmp;
      tmp = NULL;
    }
  }
  return TRUE;
}

int moveRestoreNeckwear(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  TBeing *tmp;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (!(tmp = dynamic_cast<TBeing *>(me->equippedBy)))
    return FALSE;

  // if they areholding it, don't be silly
  if (me->eq_pos != WEAR_NECK)
    return FALSE;

  if (!tmp->roomp)
    return FALSE;

  if (!tmp->isPc())
    return FALSE;

  act("$p constricts about your throat!", 0, tmp, me, 0, TO_CHAR);
  act("$p constricts about $n's throat!", 0, tmp, me, 0, TO_ROOM);
  if (tmp->hasClass(CLASS_SHAMAN)) {
    tmp->addToMove(::number(20, 75));
    act("The power of the loa enters your body through $p!", 0, tmp, me, 0, TO_CHAR);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("$p's power refreshes you!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears from $n's throat!", 0, tmp, me, 0, TO_ROOM);
    delete me;
  } else {
    int rc;
    rc = tmp->applyDamage(tmp, ::number(1,10), DAMAGE_SUFFOCATION);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears from $n's throat!", 0, tmp, me, 0, TO_ROOM);
    delete me;
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete tmp;
      tmp = NULL;
    }
  }
  return TRUE;
}

int blessingHoldItem(TBeing *, cmdTypeT cmd, const char *, TObj *me, TObj *)
{
  TBeing *tmp;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (!(tmp = dynamic_cast<TBeing *>(me->equippedBy)))
    return FALSE;

  if (!tmp->roomp)
    return FALSE;

  if (!tmp->isPc())
    return FALSE;

  if (tmp->hasClass(CLASS_SHAMAN)) {
    genericBless(tmp, tmp, 5, false);
    act("The power of the loa blesses you through $p!", 0, tmp, me, 0, TO_CHAR);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears!", 0, tmp, me, 0, TO_ROOM);
    delete me;
  } else {
    int rc;
    rc = tmp->applyDamage(tmp, ::number(1,10), DAMAGE_DRAIN);
    act("$p disappears in a puff of smoke!", 0, tmp, me, 0, TO_CHAR);
    act("In a puff of smoke, $p disappears! OW!", 0, tmp, me, 0, TO_ROOM);
    delete me;
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete tmp;
      tmp = NULL;
    }
  }
  return TRUE;
}

int chippedTooth(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  int rc;

  if (cmd != CMD_EAT)
    return FALSE;

  if (!ch->isPc())
    return FALSE;

  rc = ch->applyDamage(ch, ::number(1,5), DAMAGE_NORMAL);
  act("OWWIE!! You feel like you've chipped a tooth on $p!", 0, ch, o, 0, TO_CHAR);
  act("$n cringes in pain as $e bites into $p.", 0, ch, o, 0, TO_ROOM);
  delete o;
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete ch;
    ch = NULL;
  }
  return TRUE;

}

int sunCircleAmulet(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    char buf[256];
    one_argument(arg, buf);
    if(!strcmp(buf, "whullalo")) {
      TObj *portal;
      act("You grasp $p and utter the word '<p>whullalo<1>'.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("$n grasps $p and utters the word '<p>whullalo<1>'.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      if(ch->inRoom() != 30770) {
               act("Nothing seems to happen.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        act("Nothing seems to happen.",TRUE,ch,o,NULL,TO_ROOM,NULL);
        return TRUE;
      } else if (!(portal = read_object(30750, VIRTUAL))) {
        act("Problem in Sun Circle Amulet, tell a god you saw this.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        act("Nothing seems to happen.",TRUE,ch,o,NULL,TO_ROOM,NULL);
        vlogf(LOG_PROC, "Unable to load portal for sunCircleAmulet() proc. DASH!!");
        return TRUE;
      }
      act("The runes on the center stone flare in respone to the <Y>$o's power<1>.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("A beam of <c>energy<1> erupts from the center stone, ripping a hole in the fabric of reality!",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("It seems $n has caused a <W>portal<1> to another realm to open here.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("The runes on the center stone flare in respone to the <Y>$o's power<1>.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("A beam of <c>energy<1> erupts from the center stone, ripping a hole in the fabric of reality!",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("It seems you have caused a <W>portal<1> to another realm to open here.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      *ch->roomp += *portal;
      
      return TRUE;
    }
  }
  return FALSE;
}


int minecart(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *myself, TObj *)
{
  int where = 0, doswitch = 0, dontswitch = 0, status = 0;
  int nextroom = 0, i, dam = 0; //num_in_cart = 0, MAX_IN_CART = 5;
  TThing *in_cart, *next_in_cart;
  TBeing *beingic;  
  char arg1[30], arg2[30], arg3[30];
  char buf[256];
  TObj *switchtrack = NULL, *o = myself;
  class minecart_struct {
  public:
    bool handbrakeOn;
    int speed;
    int timer;

    minecart_struct() :
      handbrakeOn(true),
      speed(0),
      timer(-1)
    {
    }
    ~minecart_struct()
    {
    }
  };

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<minecart_struct *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  } else if (cmd == CMD_GENERIC_CREATED) {
    myself->act_ptr = new minecart_struct();
    return FALSE;
  }
  minecart_struct *job;
  if (!(job = static_cast<minecart_struct *>(myself->act_ptr))) {
    vlogf(LOG_PROC, "Minecart lost its memory. DASH!!");
    return FALSE;
  }
  if (cmd == CMD_STAND) {
    if (ch->riding == myself && job->speed >= 2) {
      ch->sendTo("You're moving much too fast to get off now!\n\r");
      return TRUE;
    } else
      return FALSE;
  }
  if (cmd == CMD_SIT) {
    arg = one_argument(arg, arg1);
    arg = one_argument(arg, arg2);
    arg = one_argument(arg, arg3);
    if ((is_abbrev(arg1, "minecart") || is_abbrev(arg1, "cart")) && job->speed >=2 ) {
      ch->sendTo("The mine cart is moving much too fast to get on now!\n\r");
      return TRUE;
    } else
      return FALSE;
  }
  if (cmd == CMD_PUSH || cmd == CMD_PULL || cmd == CMD_OPERATE || cmd == CMD_USE) {
    arg = one_argument(arg, arg1);
    arg = one_argument(arg, arg2);
    arg = one_argument(arg, arg3);
    if (is_abbrev(arg1, "handbrake") || is_abbrev(arg1, "brake")) {
      if (ch->riding != myself) {
        act("You must be sitting on $p to operate the handbrake.", TRUE, ch, o, NULL, TO_CHAR);
        return TRUE;
      } else {
        if (job->handbrakeOn) {
          job->handbrakeOn = FALSE;
          act("You release the handbrake on $p.", TRUE, ch, o, NULL, TO_CHAR);
          act("$n releases the handbrake on $p.", TRUE, ch, o, NULL, TO_ROOM);
          return TRUE;
        } else {
          job->handbrakeOn = TRUE;
          act("You engage the handbrake on $p.", TRUE, ch, o, NULL, TO_CHAR);
          act("$n engages the handbrake on $p.", TRUE, ch, o, NULL, TO_ROOM);
          return TRUE;
        }
      }
    } else if ((is_abbrev(arg1, "cart") || is_abbrev(arg1, "minecart")) && cmd == CMD_PUSH) {
      if (ch->riding == myself) {
        act("How do you intend to push $p while sitting on it?", TRUE, ch, o, NULL, TO_CHAR);
        return TRUE;
      } else if (job->handbrakeOn) {
        act("You push and push, but can't seem to move $p.", TRUE, ch, o, NULL, TO_CHAR);
        act("Oh hey, look at that.  The handbrake is still engaged, doofus.", TRUE, ch, o, NULL, TO_CHAR);
        act("$n strains with all $s might, but fails to budge $p.", TRUE, ch, o, NULL, TO_ROOM);
        act("What a loser, the handbrake is still engaged.", TRUE, ch, o, NULL, TO_ROOM);
        return TRUE;
      } else {
        act("You give $p a mighty shove, and it starts to roll slowly down the tracks.", TRUE, ch, o, NULL, TO_CHAR);
        act("$n gives $p a mighty shove, and it starts to roll slowly down the tracks.", TRUE, ch, o, NULL, TO_ROOM);
        job->speed = 1;
        job->timer = 10;
        return TRUE;
      }
    } else
      return FALSE;
  } else if (cmd == CMD_GENERIC_QUICK_PULSE && job->speed > 0) {
    where = myself->in_room;
    if (job->timer >= job->speed) {
      job->timer--;
      if (job->timer == 10 - ((10 - job->speed)/2 )) {
        if (where == 18007 || where == 18011 || where == 18020) {
          act("$n <W>rattles as it passes over the <k>switchtracks<1>.",FALSE, myself, 0, 0, TO_ROOM);
        }
      }
      if (job->timer != 0) return FALSE;
    }
    if (where < 18000 || where > 18059) {
      vlogf(LOG_PROC, "Minecart got lost. Dash will NOT be pleased.");
      return FALSE;
    } else {
      switch(where) {
      case 18007:
        status = 1;
        doswitch = 18016;
        dontswitch = 18008;
        break;
      case 18011:
        status = 1;
	doswitch = 18014;
	dontswitch = 18012;
	break;
      case 18020:
	status = 1;
	doswitch = 18033;
	dontswitch = 18021;
	break;
      case 18013:
      case 18015:
      case 18032:
      case 18059:
	status = 2;
	// code for EotL
	break;
      case 18014:
	status = 3;
	// code for breaking through the wall
	break;
      default:
	status = 0;
	break;
      }
      if (status == 1) {
	if (!(switchtrack =dynamic_cast<TObj *>( searchLinkedList("switchtrack", myself->roomp->getStuff(), TYPEOBJ)))) {
	  vlogf(LOG_PROC, "Minecart looking for switchtrack that wasn't there. Dash sucks.");
	  return FALSE;
	} else {
	  if (isname("switchtrackdoswitch", switchtrack->getName()))
	    nextroom = doswitch;
	  else if (isname("switchtrackdontswitch", switchtrack->getName()))
	    nextroom = dontswitch;
	  else {
	    vlogf(LOG_PROC, "Minecart found an indecisive switchtrack. Dash sucks.");
	    return FALSE;
	  }
	}
      } else if (status == 2) {

	if (job->speed > 8) {
	  sprintf(buf, "There is a resounding metallic *CLANG* as $n collides with the end of the track at top speed.");
	  act(buf,FALSE, myself, 0, 0, TO_ROOM);
	} else if (job->speed > 5) {
          sprintf(buf, "There is a metallic *CLANG* as $n hits the end of the track at high speed.");
          act(buf,FALSE, myself, 0, 0, TO_ROOM);
	} else if (job->speed > 2) {
          sprintf(buf, "There is a soft metallic *CLANG* as $n hits the end of the track.");
          act(buf,FALSE, myself, 0, 0, TO_ROOM);
	} else if (job->speed > 0) {
          sprintf(buf, "There is a soft metallic *ping* as $n lightly taps the end of the track.");
          act(buf,FALSE, myself, 0, 0, TO_ROOM);
	}
	if (myself->rider) {
	  for (in_cart = myself->rider; in_cart; in_cart = next_in_cart) {
	    next_in_cart = in_cart->nextRider;
	    if (::number(1,12) < job->speed) {
	      sprintf(buf, "<r>$n<1><r> loses $s balance and flips forward over the rim of $p<1><r>.  Ouch.<1>");
	      act(buf,FALSE, in_cart, myself, 0, TO_ROOM);
	      sprintf(buf, "<r>You lose your balance and flip forward over the rim of $p<1><r>.  Ouch.<1>");
              act(buf,FALSE, in_cart, myself, 0, TO_CHAR);
	      dam = job->speed * 2;
	      
	      if ((beingic = dynamic_cast<TBeing *>(in_cart))) {
		beingic->dismount(POSITION_SITTING);
		beingic->reconcileDamage(beingic, min(dam, beingic->getHit()+2), DAMAGE_COLLISION);
	      }
	    }
	 
	  }
	}

	job->speed = 0;
	job->timer = -1;
	// do some stop message
	return FALSE;
      } else if (status == 3) {
	roomDirData *exitp;
	dirTypeT dir;
	for(i=MIN_DIR;i<MAX_DIR;++i){
	  if(myself->roomp->dir_option[i] &&
	     myself->roomp->dir_option[i]->to_room==where+1){
	    break;
	  }
	}
	dir = mapFileToDir(i);
	if (!(exitp = myself->roomp->exitDir(dir))) {
	  vlogf(LOG_PROC, "bad exit for minecart smash-wall code, bug Dash.");
	  return FALSE;
	}
        if ((IS_SET(exitp->condition, EX_DESTROYED)) ||
            !IS_SET(exitp->condition, EX_CLOSED)) {

        } else {	
	  sprintf(buf, "$n slams into the wall to the east, and it collapses in a shower of rocks!");
	  act(buf, FALSE, myself, 0, 0, TO_ROOM);
	  exitp->destroyDoor(dir, where);
	  --(*myself);
	  *real_roomp(nextroom) += *myself;
	  sprintf(buf, "The wall to the west suddenly explodes inwards in a shower of rocks!");
          act(buf, FALSE, myself, 0, 0, TO_ROOM);
          --(*myself);
          *real_roomp(where) += *myself; 

	}
	nextroom = where+1;

      } else {
	nextroom = where+1;
      }
      
      for(i=MIN_DIR;i<MAX_DIR;++i){
	if(myself->roomp->dir_option[i] &&
	   myself->roomp->dir_option[i]->to_room==nextroom){
	  break;
	}
      }
      if (job->speed > 8) {
        sprintf(buf, "$n goes barreling %s down the tracks of the mining %s at breakneck speed.",
                (i==MAX_DIR)?"on":dirs[i], (where > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 5) {
        sprintf(buf, "$n rolls %s down the tracks of the mining %s at an impressive speed.",
                (i==MAX_DIR)?"on":dirs[i], (where > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 2) {
        sprintf(buf, "$n rolls %s down the tracks of the mining %s at a steady rate.",
                (i==MAX_DIR)?"on":dirs[i], (where > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 0) {
        sprintf(buf, "$n inches its way %s down the tracks of the mining %s.",
                (i==MAX_DIR)?"on":dirs[i], (where > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      }
      

      //move cart
      --(*myself);
      *real_roomp(nextroom)+=*myself;       
      
#if 0
      //move people in the cart
      if (myself->rider) {
        for (in_cart = myself->rider; in_cart; in_cart = next_in_cart) {
          next_in_cart = in_cart->nextRider;

          --(*in_cart);
          *real_roomp(nextroom)+=*in_cart;
          if(dynamic_cast<TBeing *>(in_cart))
            dynamic_cast<TBeing *>(in_cart)->doLook("",CMD_LOOK);
        }
      }
#endif //moved this later


      if (job->speed > 8) {
        sprintf(buf, "$n comes crashing down the tracks of the %s, barreling down at an incredible speed.",   
                (nextroom > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 5) {
        sprintf(buf, "$n comes rolling down the tracks of the %s at an impressive speed.",
                (nextroom > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 2) {
        sprintf(buf, "$n comes rolling down the tracks of the %s at a steady speed.",
                (nextroom > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      } else if (job->speed > 0) {
        sprintf(buf, "$n inches its way down the tracks of the %s at a steady speed.",
                (nextroom > 18003)?"tunnels":"camp");
        act(buf,FALSE, myself, 0, 0, TO_ROOM);
      }

      if (myself->rider) {
        for (in_cart = myself->rider; in_cart; in_cart = next_in_cart) {
          next_in_cart = in_cart->nextRider;
	  sprintf(buf, "...$n hangs on for dear life as $e rides $p %s.",
		  (i==MAX_DIR)?"down the tracks":dirs[i]);
          act(buf,FALSE, in_cart, myself, 0, TO_ROOM);
          sprintf(buf, "...you hang on for dear life as you ride $p %s.",
                  (i==MAX_DIR)?"down the tracks":dirs[i]);
	  act(buf,FALSE, in_cart, myself, 0, TO_CHAR);
          --(*in_cart);
          *real_roomp(nextroom)+=*in_cart;
          sprintf(buf, "...$n careens down the tracks, holding onto the $p for dear life.");
          act(buf,FALSE, in_cart, myself, 0, TO_ROOM);
          if(dynamic_cast<TBeing *>(in_cart))
            dynamic_cast<TBeing *>(in_cart)->doLook("",CMD_LOOK);
        }
      }



      if(job->handbrakeOn && job->speed > 0) {
	act("Sparks fly from $n's wheels as it slows down.",FALSE, myself, 0, 0, TO_ROOM);
	job->speed = job->speed - 2;
	if (job->speed <= 0) {
	  job->speed = 0;
	  act("The axles on $n creak a few times as it comes to a complete stop.",FALSE, myself, 0, 0, TO_ROOM);
	}
      }
      else if (job->speed < 10) job->speed++;
      job->timer = 10;
      // code for next room shit
    }
    
  } 
  return FALSE;
}


// DASH MARKER

int switchtrack(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *myself, TObj *)
{
  if (cmd != CMD_PUSH && 
      cmd != CMD_PULL && 
      cmd != CMD_OPERATE && 
      cmd != CMD_USE && 
      cmd != CMD_TURN) 
    return FALSE;
  
  if (!myself->getName())
    return FALSE;
      
  int where = myself->in_room;
  char arg1[30], arg2[30], buf[256];
  arg = one_argument(arg, arg1);
  arg = one_argument(arg, arg2);

  if (is_abbrev(arg1, "switchtracks") || is_abbrev(arg1, "tracks")) {
    switch (where) { 
    case 18007:
      if(!arg2)
	strcpy(arg2,isname("switchtrackdoswitch", myself->getName())?"southwest":"south");
      else if(is_abbrev(arg2, "south") || is_abbrev(arg2, "s")) {
	strcpy(arg2,"southern");
	if(isname("switchtrackdoswitch", myself->getName())) {
	  ch->sendTo("The switchtrack is already aligned with the %s fork.", arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdoswitch");
      }
      else if(is_abbrev(arg2, "southwest") || is_abbrev(arg2, "se")) {
	strcpy(arg2,"southwestern");
	if(isname("switchtrackdontswitch", myself->getName())) {
	  ch->sendTo("The switchtrack is already aligned with the %s fork.", arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdontswitch");
      } else {
	ch->sendTo("This switchtrack can only be moved to the south or southwest.");
	return TRUE;
      }
      
      break;
    case 18011:
      if(!arg2) 
	strcpy(arg2,isname("switchtrackdoswitch", myself->getName())?"south":"east");
      else if(is_abbrev(arg2, "east") || is_abbrev(arg2, "e")) {
	strcpy(arg2,"eastern");
	if(isname("switchtrackdoswitch", myself->getName())) {
	  ch->sendTo("The switchtrack is already aligned with the %s fork.", arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdoswitch");
      }
      else if(is_abbrev(arg2, "south") || is_abbrev(arg2, "s")) {
	strcpy(arg2,"southern");
	if (isname("switchtrackdontswitch", myself->getName())) {
	  ch->sendTo("The switchtrack is already aligned with the %s fork.", arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdontswitch");  
      } else {
        ch->sendTo("This switchtrack can only be moved to the south or east.");
        return TRUE;
      }
      
      break;
    case 18020:
      if(!arg2) 
	strcpy(arg2,isname("switchtrackdoswitch", myself->getName())?"north":"east");
      else if(is_abbrev(arg2, "east") || is_abbrev(arg2, "e")) {
	strcpy(arg2,"eastern");
	if(isname("switchtrackdoswitch", myself->getName())) {
	  ch->sendTo("The switchtrack is already aligned with the %s fork.", arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdoswitch");
      }
      else if(is_abbrev(arg2, "north") || is_abbrev(arg2, "n")) {
	strcpy(arg2,"northern");
	if(isname("switchtrackdontswitch", myself->getName())) {
	  ch->sendTo("The switchtrack is already aligned with the %s fork.", arg2);
	  return TRUE;
	}
	strcpy(myself->name, "switchtracks tracks switchtrackdontswitch");
      } else {
        ch->sendTo("This switchtrack can only be moved to the north or east.");
        return TRUE;
      }
      
      break;
    default:
      ch->sendTo("Uh. This switchtrack shouldn't be here. Tell a god or something?");
      //      vlogf(LOG_PROC, "%s tried to operate a switchtrack (%d) in room with no switchtrack code (%d)",
      //	    ch->getName(), myself->objVnum, where);
    }
    sprintf(buf,"<k>You force the $o into alignment with the %s tunnel.<1>",arg2);
    act(buf, TRUE, ch, myself, NULL, TO_CHAR);
    sprintf(buf,"<k>$n forces the $o into alignment with the %s tunnel.<1>",arg2);
    act(buf, TRUE, ch, myself, NULL, TO_ROOM);
    sprintf(buf,"<k>The switchtracks here are aligned with the %s tunnel.<1>", arg2);
    myself->setDescr(mud_str_dup(buf));
    return TRUE;
  }
  return FALSE;
}


int berserkerWeap(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TBeing *ch;
  int rc, dam;
  affectedData af,aff;

  ch = genericWeaponProcCheck(vict, cmd, o, 5);
  if (!ch)
    return FALSE;
  if(ch->hasClass(CLASS_WARRIOR) && ch->doesKnowSkill(SKILL_BERSERK)) {
    if(ch->isCombatMode(ATTACK_BERSERK) || !ch->isPc()) {
      
      if(!::number(0,3) && !ch->affectedBySpell(SPELL_HASTE)) {
	act("$p<1> glows with a <c>soft blue light<1>, and lends new energy to your attacks!",TRUE,ch,o,vict,TO_CHAR,NULL);
	act("$p<1> glows with a <c>soft blue light<1>, and lends new energy to $n's attacks!",TRUE,ch,o,vict,TO_ROOM,NULL);
	aff.type = SPELL_HASTE;
	aff.level = 45;
	aff.duration = ONE_SECOND * 12; // seconds are weird so this is a 1 min cast of haste
	aff.modifier = 0;
	aff.location = APPLY_NONE;
	aff.bitvector = 0;
	ch->affectTo(&af, -1);
	act("$N has gained a bounce in $S step!",
	    FALSE, ch, NULL, NULL, TO_ROOM);
	act("You seem to be able to move with the greatest of ease!",
	    FALSE, ch, NULL, NULL, TO_CHAR);

	return TRUE;
      }	
      
      act("<o>Your blood boils and you feel your wrath being amplified by $p.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
      dam = ch->getSkillValue(SKILL_BERSERK) / 5 + 2;
      if (ch->getRace() == RACE_OGRE)
	dam +=5;
      if (dam > 21) {
	act("<r>Your $o releases a HUGE burst of concentrated fury upon $N<1><r>!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
	act("<r>$n clenches $s teeth as $s $o releases a HUGE burst of fury upon $N.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
	act("<o>$n clenches $s teeth as $s $o releases a HUGE burst of fury upon you!<1>",TRUE,ch,o,vict,TO_VICT,NULL);
      } else {
	act("<r>Your $o releases a burst of concentrated fury upon $N<1><r>!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
	act("<r>$n clenches $s teeth as $s $o releases a burst of fury upon $N.<1>",TRUE,ch,o,vict,TO_NOTVICT,NULL);
	act("<o>$n clenches $s teeth as $s $o releases a burst of fury upon you!<1>",TRUE,ch,o,vict,TO_VICT,NULL);
      }

      rc = ch->reconcileDamage(vict, dam, DAMAGE_HACKED);
      if (IS_SET_DELETE(rc, DELETE_VICT))
	return DELETE_VICT;
      return TRUE;


    } else if (ch->isPc() && !::number(0,10)) {
     
      ch->setCombatMode(ATTACK_BERSERK);
     
      act("<r>$p<r> amplifies your wrath, and you launch into a blood rage!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
      act("<r>$p<r> amplifies $n's wrath, and $e launches into a blood rage!<1>",TRUE,ch,o,vict,TO_ROOM,NULL);

      act("You go berserk!",TRUE,ch,0,0,TO_CHAR);
      act("$n goes berserk!", TRUE, ch,0,0,TO_ROOM);

      if(ch->getHit() > (ch->hitLimit()/2)){
	af.type = SKILL_BERSERK;
	af.modifier = ::number(ch->getSkillValue(SKILL_BERSERK),
			       ch->getSkillValue(SKILL_BERSERK)*2);
	af.level = ch->GetMaxLevel();
	//      af.duration = ch->getSkillValue(SKILL_BERSERK);;
	af.duration = PERMANENT_DURATION;
	af.location = APPLY_HIT;
	af.bitvector = 0;
	ch->affectTo(&af, -1);

	af.location = APPLY_CURRENT_HIT;
	ch->affectTo(&af, -1);

	ch->sendTo("Berserking increases your ability to withstand damage!\n\r");
      }

      if (!ch->fight())
	ch->goBerserk(NULL);
    }
  } 

  return FALSE;

}


int travelGear(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  // this is a custom proc for the Wanderlust/Cloak of the Traveler combo - Dash
  // i really like this proc :) 
  // ok here is what it does - the procs only work when the player has BOTH objects - the hammer and the cloak
  // the hammer restores moves fairly rapidly, when ever the player is low
  // the cloak projects a force shield around the wearer whenever it is hit, shield lasts for 2 rounds then goes away again

  TBeing *ch;
  TObj *cloak;
  TObj *hammer = NULL;
  affectedData aff;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (obj_index[o->getItemIndex()].virt == 9583) {
    
    if (cmd != CMD_GENERIC_QUICK_PULSE)
      return FALSE;
    
    if(!(cloak = dynamic_cast<TObj *>(ch->equipment[WEAR_BACK]))) 
      return FALSE;
    if (obj_index[cloak->getItemIndex()].virt != 9582)
      return FALSE;
    
    // ok... so he's wielding the hammer, wearing the cloak....
    
    if (!::number(0,1) && (ch->getMaxMove() > 4*ch->getMove())) {
      ch->addToMove(ch->getMaxMove()/(::number(2,5)));
      act("<k>$p<Y> glows softly<1>, and you feel renewed strength flow into your legs.<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
      act("<k>$p<Y> glows softly<1>, and $n seems to look more refreshed.<1>",TRUE,ch,o,vict,TO_ROOM,NULL);
      return TRUE;
    }
    return FALSE;
  } else if (obj_index[o->getItemIndex()].virt == 9582) {
    if (cmd != CMD_OBJ_BEEN_HIT)
      return FALSE;
    if (!((hammer = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT])) && obj_index[hammer->getItemIndex()].virt == 9583)
	||((hammer = dynamic_cast<TObj *>(ch->equipment[HOLD_LEFT])) && obj_index[hammer->getItemIndex()].virt == 9583))
      // no hammer
      return FALSE;
    if (ch->affectedBySpell(SPELL_SORCERERS_GLOBE))
      return FALSE;
    
    
    act("<1>$p<Y> glows brightly<1> as it is struck!",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<1>$p<Y> glows brightly<1> as it is struck!",TRUE,ch,o,vict,TO_ROOM,NULL);
    act("Your $o emits an audible hum and suddenly <W>a shield of force<1> slams into being around you!",
	TRUE,ch,o,vict,TO_CHAR,NULL);
    act("$n's $o emits an audible hum and suddenly <W>a shield of force<1> slams into being around $m!",
	TRUE,ch,o,vict,TO_ROOM,NULL);

    aff.type = SPELL_SORCERERS_GLOBE;
    aff.level = 37;
    aff.duration = ONE_SECOND * 2;
    aff.location = APPLY_ARMOR;
    aff.modifier = -100;
    aff.bitvector = 0;
    ch->affectTo(&aff, -1);
    return TRUE;
  }
  return FALSE;
}




int dualStyleWeapon(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{

  // this code is for weapons with more than one damage type
  // it utilizes two 'styles' that can be changed with the switch <weapon> command
  // to favor one or the other damage types
  // dash - may 2001

  TGenWeapon *weap = dynamic_cast<TGenWeapon *>(o);
  if (!weap)
    return FALSE;
  
  class spectype_struct {
  public:
    weaponT norm;
    weaponT type1;
    weaponT type2;
    int vnum;
    
    spectype_struct()
    {
    }
    ~spectype_struct()
    {
    }
  };

  spectype_struct *weapspec = NULL;
  if (cmd == CMD_GENERIC_CREATED || !(weapspec = static_cast<spectype_struct *>(o->act_ptr))) {
    o->act_ptr = new spectype_struct();
    vlogf(LOG_PROC, "obj (%s) with dualstyle proc ... attempting to alocate.", o->getName());
    if (!(weapspec = static_cast<spectype_struct *>(o->act_ptr))) {
      vlogf(LOG_PROC, "obj (%s) with dualstyle proc had no memory alocated, investigate.", o->getName());
      return FALSE;
    }
    weapspec->type1 = weap->getWeaponType();
    weapspec->norm = weapspec->type1;
    weapspec->vnum = obj_index[o->getItemIndex()].virt;
    switch (weapspec->vnum) {                             // this proc is versatile - add more vnums/secondary damage types
      case 9595:                                // here to make it work with another weapon
	weapspec->type2 = WEAPON_TYPE_SMITE; // hammerblade - dash 05/01
      default:
	weapspec->type2 = WEAPON_TYPE_SMASH;
    }
    return FALSE;
  }

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<spectype_struct *>(o->act_ptr);
    o->act_ptr = NULL;
    return FALSE;
  }  


  TBeing *ch;
  char parg[30];

  if(!(ch=genericWeaponProcCheck(vict, CMD_OBJ_HIT, weap, 0)))
    return FALSE;
  
  if (cmd != CMD_SWITCH && cmd != CMD_OBJ_HIT) {
    if (cmd != CMD_OBJ_HITTING && cmd != CMD_OBJ_MISS)
      weap->setWeaponType(weapspec->type1);
    return FALSE;
  }
  

  if(cmd == CMD_SWITCH) {
    arg = one_argument(arg, parg);
    isname(parg, weap->getName());
    act("<c>You deftly change your grip on $p<c> to use it in a different style!<1>",TRUE,ch,o,vict,TO_CHAR,NULL);
    act("<c>$n deftly changes $s grip on $p to use it in a different style!<1>",TRUE,ch,o,vict,TO_ROOM,NULL);    
    if (weapspec->type1 == weapspec->norm) {
      weapspec->type1 = weapspec->type2;
      weapspec->type2 = weapspec->norm;
    } else {
      weapspec->type2 = weapspec->type1;
      weapspec->type1 = weapspec->norm;
    }
    return TRUE;
  }
 
  if (::number(0,3)) { // 3/4 is type 1, 1/4 is type two
    weap->setWeaponType(weapspec->type1);
  } else {
    weap->setWeaponType(weapspec->type2);
  }
  return TRUE;
}

int selfRepairing(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if(::number(0,9))
    return false;

  if (cmd == CMD_GENERIC_PULSE && o->getStructPoints() < o->getMaxStructPoints()) {
    if(::number(1,100) < (int)(100.0*((float)(o->getStructPoints()) / (float)(o->getMaxStructPoints()))))
      return FALSE;

    act("<W>$n<W>'s $o slowly reconstructs itself, erasing signs of damage.<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);
    act("<W>Your $o slowly reconstructs itself, erasing signs of damage.<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);

    o->addToStructPoints(::number(1,min(5, o->getMaxStructPoints() - o->getStructPoints())));
    return FALSE;
  }
  return FALSE;
}

int USPortal(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *) {
  
  if(cmd == CMD_ENTER || cmd == CMD_LEAVE) {
    if (!ch->isUndead()) {
      act("<k>There is a sharp crackle of negative energy as $n tries to go through the portal.<1>"
	  ,TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<k>There is a sharp crackle of negative energy as you try to go through the portal.<1>"
	  ,TRUE,ch,o,NULL,TO_CHAR,NULL);
      
      act("<k>Suddenly, $n is thrown backwards from the portal with tremendous force!<1>"
	  ,TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<k>Suddenly, you are thrown backwards from the portal with tremendous force!<1>"
	  ,TRUE,ch,o,NULL,TO_CHAR,NULL);
      
      int rc, dam;
      dam = ::number(10,30);
      ch->setPosition(POSITION_SITTING);
      rc = ch->reconcileDamage(ch, dam, DAMAGE_NORMAL);
      if (rc == -1)
	return DELETE_VICT;
      
      return TRUE;
    } else { // ch is undead
      act("<k>A chill radiates from $n as $e enters the portal.<1>"
          ,TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<k>A terrible chill runs through you as you enter the portal.<1>"
          ,TRUE,ch,o,NULL,TO_CHAR,NULL);
      return FALSE;
    }
  }
  return FALSE;
}
    


int AKAmulet(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *) {
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (cmd == CMD_GENERIC_PULSE && !::number(0,3)) {
    if(ch->isUndead() && ch->hitLimit() > ch->getHit()) {
       int dam =::number(1,5);
       act("$n regenerates slightly.",TRUE,ch,o,NULL,TO_ROOM,NULL);
       act("You regenerate slightly.",TRUE,ch,o,NULL,TO_CHAR,NULL);
       ch->addToHit(dam);
       return FALSE;
    } else if (!ch->isUndead()) {
      int dam = ::number(25,100);
      act("$n screams in pain as $p drains the life from $s body!",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("You scream in pain as $p drains the life from your body!",TRUE,ch,o,NULL,TO_CHAR,NULL);
      int rc = ch->reconcileDamage(ch, dam, DAMAGE_DRAIN);
      if (rc == -1)
        return DELETE_THIS;
      return FALSE;
    }
  }
  return FALSE;
  
}


int suffGlove(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{


  TBeing *ch;
  int rc, dam = 1;

  if (!o || !vict)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if (::number(0,40))
    return FALSE;
  if (cmd != CMD_OBJ_HIT)
    return FALSE;


  if (vict->affectedBySpell(SPELL_SUFFOCATE))
    return FALSE;

  affectedData aff;
  aff.type = AFFECT_DISEASE;
  aff.level = ch->GetMaxLevel();
  aff.duration = combatRound(3);  
  aff.modifier = DISEASE_SUFFOCATE;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_SILENT;



  ch->addToWait(combatRound(3));
  ch->cantHit += ch->loseRound(3);
  vict->addToWait(combatRound(3));
  vict->cantHit += ch->loseRound(3);


  act("Your $o seems control your movements as you reach for $N!<1>",
      TRUE,ch,o,vict,TO_CHAR,NULL);
  act("<o>$p <o>covers $N<o>'s nose and mouth, preventing $M from breathing!<1>",
      TRUE,ch,o,vict,TO_CHAR,NULL);
  act("$n's $o seems control $s movements as $e reaches for $N!<1>",
      TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("<o>$p <o>covers $N<o>'s nose and mouth, preventing $M from breathing!<1>",
      TRUE,ch,o,vict,TO_NOTVICT,NULL);
  act("n$'s $o seems control $s movements as $e reaches for you!<1>",
      TRUE,ch,o,vict,TO_VICT,NULL);
  act("<o>$p <o>covers your nose and mouth.  PANIC!  You can't breathe!!!<1>",
      TRUE,ch,o,vict,TO_VICT,NULL);


  dam = ::number(1,10);
  rc = ch->applyDamage(vict, dam, DAMAGE_SUFFOCATION);
  vict->affectJoin(vict, &aff, AVG_DUR_NO, AVG_EFF_YES);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;


}



int totemMask(TBeing *v, cmdTypeT cmd, const char *, TObj *o, TObj *weapon)
{
  TBeing *ch;
  int rc, dam, chance, result;
  wearSlotT t;

  if(cmd != CMD_OBJ_BEEN_HIT || !v || !o)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;     

  t=((!weapon || (weapon->eq_pos==HOLD_RIGHT))?WEAR_HAND_R:WEAR_HAND_L);
  chance = ::number(0, 99);
  result = ::number(5, 16);

  if (chance >= 65) {
    // do nothing
  } else {
    act("<r>The eyes of $o <r>glow blood red as life force is channeled from your body.<1>"
	, 0, v, o, 0, TO_CHAR);
    act("<r>The eyes of $p <r>glow blood red.<1>"
	, 0, v, o, 0, TO_ROOM);
    ch->addToLifeforce(result);    
    dam = ::number(3, 15);
    
    rc = ch->reconcileDamage(v, dam, DAMAGE_DRAIN);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }
  return TRUE;
}


int permaDeathMonument(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o1, TObj *o2)
{
  MYSQL_RES *res;
  MYSQL_ROW row;
  int rc, found=0;
  TThing *o;
  TObj *to;

  if(cmd != CMD_LOOK)
    return FALSE;

  for (o = ch->roomp->getStuff(); o; o = o->nextThing) {
    to = dynamic_cast<TObj *>(o);
    if (to && to->spec == SPEC_PERMA_DEATH &&
	isname(arg, to->name)){
      found=1;
      break;
    }
  }

  if(!found)
    return FALSE;


  if((rc=dbquery(TRUE, &res, "sneezy", "permaDeathMonument", "select name, level, died, killer from permadeath order by level desc limit 10"))){
    if(rc==-1)
      vlogf(LOG_BUG, "Database error in permaDeathMonument");
    else {
      ch->sendTo("The plaque is empty.\n\r");
      return TRUE;
    }
      
    return FALSE;
  }

  ch->sendTo("You examine the plaque:\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");
  ch->sendTo("-     This monument commemorates the bravest of heroes     -\n\r");
  ch->sendTo("-                 who risk permanent death.                -\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");

  int i=1;
  while((row=mysql_fetch_row(res))){
    if(atoi(row[2])==1){
      ch->sendTo(COLOR_BASIC, "%i) %s perished bravely at level %s, killed by %s.\n\r", i, row[0], row[1], row[3]);
    } else {
      ch->sendTo(COLOR_BASIC, "%i) %s lives on at level %s\n\r", i, row[0], row[1]);
    }
    ++i;
  }


  mysql_free_result(res);

  return TRUE;
}

// Dash stuff - dec 2001

int force(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{

  TBeing *ch;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (cmd == CMD_SAY || cmd == CMD_SAY2) {
    char buf[256];
    one_argument(arg, buf);
    if(!strcmp(buf, "force")) {
      if(ch->checkObjUsed(o)) {
        act("You cannot use $p's powers again this soon.",TRUE,ch,o,NULL,TO_CHAR,NULL);
        return FALSE;
      }

      ch->addObjUsed(o, UPDATES_PER_MUDHOUR * 24);

      act("$n holds $p aloft, shouting a <p>word of power<1>.",TRUE,ch,o,NULL,TO_ROOM,NULL);
      act("<c>The air around <1>$n<c> seems to waver and suddenly solidifies, expanding in a wave of force!<1>",TRUE,ch,o,NULL,TO_ROOM,NULL);


      act("You hold $p aloft, shouting the word '<p>force<1>'.",TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("<c>The air around you seems to waver and suddenly solidifies, expanding in a wave of force!<1>",TRUE,ch,o,NULL,TO_CHAR,NULL);

      TThing *t;
      TBeing *vict = NULL;
   




      for (t = ch->roomp->getStuff(); t; t = t->nextThing) {
	vict = dynamic_cast<TBeing *>(t);
	if (!vict)
	  continue;
	if (vict->fight() != ch)
	  continue;
	
	if (vict->riding) {
	  act("The wave of force knocks $N from $S mount!",
	      TRUE,ch,o,vict,TO_CHAR,NULL);
	  act("The wave of force knocks $N from $S mount!",
	      TRUE,ch,o,vict,TO_NOTVICT,NULL);
	  act("<o>The wave of force knocks you from your mount!<1>",
	      TRUE,ch,o,vict,TO_VICT,NULL);
	  vict->dismount(POSITION_RESTING);

	}	  
	act("The wave of force from your $o slams $N into the $g, stunning $M!",
	    TRUE,ch,o,vict,TO_CHAR,NULL);
	act("The wave of force from $n's $o slams $N into the $g, stunning $M!",
	    TRUE,ch,o,vict,TO_NOTVICT,NULL);
	act("The wave of force from $n's $o slams you into the $g, stunning you!",
	    TRUE,ch,o,vict,TO_VICT,NULL);

	affectedData aff;

	aff.type = SKILL_DOORBASH;
	aff.duration = ONE_SECOND;
	aff.bitvector = AFF_STUNNED;
	vict->affectTo(&aff, -1);
	if (vict->fight())
	  vict->stopFighting();
      }
      if (ch->fight())
	ch->stopFighting();
      return TRUE;
    }
  }
  return FALSE;

}


//MARKER: END OF SPEC PROCS


extern int board(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int weaponBlinder(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int weaponManaDrainer(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int weaponLightningRod(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int weaponJambiyaSpecial(TBeing *, cmdTypeT, const char *, TObj *, TObj *);

// assign special procedures to objects

TObjSpecs objSpecials[NUM_OBJ_SPECIALS + 1] =
{
  {TRUE, "BOGUS", bogusObjProc},  // 0
  {TRUE, "fountain", fountain},
  {FALSE, "bulletin board", board},
  {TRUE, "note dispenser", dispenser},
  {TRUE, "statue of feeding", statue_of_feeding},
  {TRUE, "pager", pager},             // 5
  {TRUE, "ear muffs", ear_muffs},
  {FALSE, "Jewel of Judgment", JewelJudgment},   
  {FALSE, "Gwarthir", Gwarthir},
  {FALSE, "vending machine", vending_machine},
  {FALSE, "Orb of Destruction", orbOfDestruction},   // 10
  {FALSE, "War Maker", warMaker},
  {TRUE, "Weapon: disruption", weaponDisruption},
  {TRUE, "Weapon: fumbler", weaponFumbler},
  {TRUE, "ladder", ladder},
  {TRUE, "wine fountain", wine_fountain},      // 15
  {TRUE, "Weapon: bonebreaker", weaponBreaker},
  {FALSE, "Glowing Cutlass", glowCutlass},
  {TRUE, "poison whip", poisonWhip},
  {TRUE, "magic gills", magicGills},
  {FALSE, "potion of youth", youthPotion},      // 20
  {FALSE, "rainbow bridge", rainbowBridge},
  {FALSE, "Hunting Dagger", daggerOfHunting}, 
  {TRUE, "flame weapon", flameWeapon}, 
  {TRUE, "frost weapon", frostWeapon}, 
  {TRUE, "food&drink", foodItem},       // 25
  {TRUE, "crystal ball", crystal_ball},
  {FALSE, "Orb of Teleportation", orbOfTeleportation},
  {FALSE, "Weather Armor", weatherArmor},
  {FALSE, "caravan wagon", caravan_wagon},
  {TRUE, "Bleed Chair", bleedChair},  // 30
  {TRUE, "Harm Chair", harmChair},
  {TRUE, "Dragon Slayer", dragonSlayer},
  {TRUE, "Blood Drainer", bloodDrain},
  {FALSE, "Stone Altar", stoneAltar},
  {FALSE, "Bone Staff", boneStaff},   // 35
  {FALSE, "Bloodspike", bloodspike}, 
  {FALSE, "behir horn item", behirHornItem},
  {FALSE, "Newbie Helper", newbieHelperWProc},
  {FALSE, "Bless Fountain", bless_fountain},
  {FALSE, "Blood Bowl", bowl_of_blood},  // 40
  {FALSE, "feather fall", featherFallItem},
  {FALSE, "wicked dagger", wickedDagger},  
  {TRUE, "poison sap dagger", poisonSap},  
  {FALSE, "blinder weapon", weaponBlinder},
  {FALSE, "mana drain weapon", weaponManaDrainer}, // 45
  {FALSE, "potion of characteristics", statPotion},  
  {FALSE, "daySword", daySword},  
  {FALSE, "nightBlade", nightBlade},  
  {TRUE, "Lightning Rod", weaponLightningRod},
  {FALSE, "Jambiya", weaponJambiyaSpecial}, // 50
  {TRUE, "Sciren's Suffocation", scirenDrown},
  {TRUE, "Energy Beam Weapon", energyBeam},
  {TRUE, "Viper Weapon (poison)", poisonViperBlade},
  {FALSE, "trolley", trolley},
  {FALSE, "Stone Skin Amulet", stoneSkinAmulet}, // 55
  {FALSE, "Razor Glove", razorGlove},
  {FALSE, "ShadowSlayer", weaponShadowSlayer},
  {FALSE, "Squirt Gun", squirtGun},
  {FALSE, "Glory Weapon", blazeOfGlory},
  {TRUE, "Elemental Weapon", elementalWeapon}, // 60
  {FALSE, "Life Leech Glove", lifeLeechGlove},
  {TRUE, "Mechanical Wings", mechanicalWings},
  {FALSE, "Key in Knife",  keyInKnife},
  {FALSE, "Teleport Vial", teleportVial},
  {FALSE, "Sun Circle Amulet", sunCircleAmulet}, // 65
  {FALSE, "Better Vender", vending_machine2},
  {FALSE, "Mine Cart", minecart},
  {FALSE, "Switchtrack", switchtrack},
  {FALSE, "vorpal", vorpal},
  {TRUE, "Berserker Weapon", berserkerWeap},// 70
  {FALSE, "Travel Gear", travelGear},
  {FALSE, "Maquahuitl", maquahuitl},
  {FALSE, "Randomizer", randomizer},
  {FALSE, "Blunt/Pierce", bluntPierce},
  {TRUE, "Dual Style Weapon", dualStyleWeapon}, //75
  {FALSE, "Mana Burn Robe", manaBurnRobe},
  {FALSE, "Chrism: minor heal", healingNeckwear},
  {FALSE, "Chrism: bless hold item", blessingHoldItem},
  {FALSE, "Chrism: vitality restore", moveRestoreNeckwear},
  {FALSE, "Chipped Tooth Food Item", chippedTooth}, // 80
  {FALSE, "Goofers Dust", goofersDust},
  {FALSE, "teleport ring", teleportRing},
  {FALSE, "self repairing", selfRepairing},
  {FALSE, "undead spewing portal", USPortal},
  {FALSE, "Amulet of Aeth Koralm", AKAmulet}, // 85
  {FALSE, "fire glove", fireGlove},
  {FALSE, "Shaman's Totem Mask", totemMask},
  {FALSE, "perma death monument", permaDeathMonument},
  {FALSE, "fishing boat", fishingBoat},
  {FALSE, "Splintered Club", splinteredClub}, // 90 
  {FALSE, "Suffocation Glove", suffGlove},
  {FALSE, "Force", force},
  {FALSE, "last proc", bogusObjProc}
};











