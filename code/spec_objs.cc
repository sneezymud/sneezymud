//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: spec_objs.cc,v $
// Revision 1.5  1999/09/26 23:49:47  lapsos
// Changes to the wicked proc.
//
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
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

//CMD_OBJ_GOTTEN returns DELETE_THIS if this goes bye bye
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
    vlogf(5,"NULL ch in obj_act");
    return;
  }
  if (!o) {
    vlogf(5,"NULL obj in obj_act");
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
          act("$n's $o glows red-hot in $s hands!", 1, ch, o, NULL, TO_ROOM, NULL);
          act("Your $o glows red-hot in your hands!", 1, ch, o, NULL, TO_CHAR);
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
    aff.duration = (20) * UPDATES_PER_TICK;
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
    aff.duration = (20) * UPDATES_PER_TICK;
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

  bool primary = FALSE;
  if (ch->heldInPrimHand() == o)
    primary = TRUE;
  
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
  sprintf(buf,"$n's $o screams with power as $e swings it at your %s!",
     vict->describeBodySlot(part).c_str());
  act(buf,TRUE,ch,o,vict,TO_VICT,ANSI_RED);
  sprintf(buf,"$n's $o screams with power as $e swings it at $N's %s!",
     vict->describeBodySlot(part).c_str());
  act(buf,TRUE,ch,o,vict,TO_NOTVICT,ANSI_ORANGE);
  sprintf(buf,"Your $o screams with power as you swing it at $N's %s!",
     vict->describeBodySlot(part).c_str());
  act(buf,TRUE,ch,o,vict,TO_CHAR,ANSI_GREEN);

  sprintf(buf,
    "A soft WOMPF! is heard as $o releases a shock wave into $n's %s!",
       (obj ? obj->getName() : (vict->isHumanoid() ? "skin" : "hide")));
  act(buf, TRUE, vict,o,0,TO_ROOM,ANSI_ORANGE);
  sprintf(buf,
    "A soft WOMPF! is heard as $o releases a shock wave into your %s!",
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
    vict->sendTo(COLOR_OBJECTS, "You can't ride your %s on %s.\n\r", fname(vict->riding->name).c_str(), o->getName());
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
  vlogf(LOW_ERROR, "Undefined item (%s) with special proc: foodItem", getName());
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
  for (t = ch->roomp->stuff; t; t = t->nextThing) {
    vict = dynamic_cast<TBeing *>(t);
    if (!vict)
      continue;
    if (!vict->inGroup(ch))
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

  if (isname(buffer, o->name)) {
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
    if (isname(buffer, o->name)) {
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
        for (t = ch->roomp->stuff; t; t = t2) {
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
    if (isname(buffer, o->name)) {
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
        for (t = ch->roomp->stuff; t; t = t2) {
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
  
  if (*arg && !isname(arg, me->name))
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
    if (!isname(buf, me->name))
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
    if (!isname(buf, me->name))
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
    if (!isname(buf, me->name))
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
    if (!isname(buf, me->name))
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
    vlogf(10, "Explosion in room : ROOM_NOWHERE. (explode() spec_objs.c)");
    return;
  }

  for (t = rm->stuff; t; t = t2) {
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
        vlogf(10, "Unknown vending machine.  Yikes");
        return TRUE;
    }
    if (obj_index[ob2->getItemIndex()].virt == token) {
      act("You insert $p into $P.", TRUE, ch, ob2, o, TO_CHAR);
      act("$n inserts $p into $P.", TRUE, ch, ob2, o, TO_ROOM);
      act("$p begins to beep and shake.", TRUE, ch, o, NULL, TO_CHAR);
      act("$p begins to beep and shake.", TRUE, ch, o, NULL, TO_ROOM);
      if (!(dew = read_object(result, VIRTUAL))) {
        vlogf(10, "Damn vending machine couldn't read a Dew.  Stargazer!");
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


int dagger_of_death(TBeing *ch, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  int rc;

  if (cmd == CMD_OBJ_STUCK_IN) {
    if (o->eq_stuck == WEAR_HEAD) {
      vlogf(5, "%s killed by ITEM:dagger-of-death at %s (%d)",
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
    if (isname(arg2, o->name)) {
      act("You get a note from $p.", FALSE, ch, o, 0, TO_CHAR);
      act("$n gets a note from $p.", FALSE, ch, o, 0, TO_ROOM);
      if (!(note = read_object(GENERIC_NOTE, VIRTUAL))) {
        vlogf(10, "Bad note dispenser! NO note can be loaded!");
        return FALSE;
      }
      *ch += *note;
      return TRUE;
    }
  } else if (is_abbrev(arg1, "quill")) {
    if (isname(arg2, o->name)) {
      act("You get a quill from $p.", FALSE, ch, o, 0, TO_CHAR);
      act("$n gets a quill from $p.", FALSE, ch, o, 0, TO_ROOM);
      if (!(quill = read_object(GENERIC_PEN, VIRTUAL))) {
        vlogf(10, "Bad quill dispenser! NO quill can be loaded!");
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
    vlogf(8, "Pager lost its memory.");
    return FALSE;
  }

  strcpy(capbuf, ch->getName());

  if (cmd == CMD_USE || cmd == CMD_OPERATE) {
    if (o->equippedBy != ch)
      ch->sendTo("You must have it equipped to use it!\n\r");
    else if (job->isOn) {
      act("$n discretely turns off $s $o.", TRUE, ch, o, 0, TO_ROOM);
      ch->sendTo("You turn off your %s, trying to be very discrete about it.\n\r", fname(o->name).c_str());
      job->isOn = FALSE;
    } else {
      act("$n turns on $s $o, causing it to beep obnoxiously.", FALSE, ch, o, 0, TO_ROOM);
      ch->sendTo("You turn on your %s, producing a series of annoying beeps.\n\r", fname(o->name).c_str());
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
      af.duration = 168 * UPDATES_PER_TICK;
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
    if (!*objbuf || !isname(objbuf, me->name)) {
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

  if (!*objbuf || !isname(objbuf, me->name) || !*targbuf)
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
    strcpy(targbuf, ch->name);
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

  while (!me->sameRoom(target)) {
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
  ch->doSay(arg);
  if (!victim->roomp) {
    obj_act("says 'Woah, big problem, talk to Brutius!'", 
        ch, me, NULL, ANSI_GREEN);
    return TRUE;
  }
  target = victim->roomp->number;

  if (victim->GetMaxLevel() > ch->GetMaxLevel()) {
    obj_act("You are not powerful enough to see that person, $n!", 
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
      vlogf(10, "Bad alloc (1) of caravan wagon");
      return FALSE;
    }
    me->act_ptr = car;

    car->driver = ch;

    return FALSE;
  } else if (cmd == CMD_OBJ_WAGON_UNINIT) {
    if (!(car = (wagon_struct *) me->act_ptr)) {
      vlogf(10, "Bad alloc (3) of caravan wagon");
      return FALSE;
    }
    if (me->act_ptr)
    car->driver = NULL;

    return FALSE;
  } else if (cmd == CMD_OBJ_MOVEMENT) {
    if (!(car = (wagon_struct *) me->act_ptr)) {
      vlogf(10, "Bad alloc (2) of caravan wagon");
      return FALSE;
    }
    if (ch != car->driver)
      return FALSE;

    int dum = (int) arg;
    dir = dirTypeT(dum);

    if (dir < MIN_DIR || dir >= MAX_DIR) {
      vlogf(5, "Problematic direction in CMD_OBJ_MOVEMENT");
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
  if (!isname(buf, me->name)) {
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

int youthPotion(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  int rc;
  if (genericPotion(ch, me, cmd, arg, rc))
    return rc;

  act("$n imbibes $p.", TRUE, ch, me, 0, TO_ROOM);
  act("You imbibe $p.", TRUE, ch, me, 0, TO_CHAR);

  ch->age_mod -= 3;
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

  int lev = pot->getMagicLevel();
  statTypeT whichStat = statTypeT(lev);

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
    vlogf(9, "WARNING:  %s is running around with a bogus spec_proc #%d",
       me->name, me->spec);
  else
    vlogf(9, "WARNING: indeterminate obj has bogus spec_proc");
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

  ch->doSit(me->name);

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
      return DELETE_THIS;

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

int stoneAltar(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *obj, TObj *)
{
  if (!ch)
    return FALSE;

  if (cmd != CMD_PUSH && cmd != CMD_PRESS)
    return FALSE;

  char buf[256];
  one_argument(arg, buf);
  if (is_abbrev(buf, "eye") || is_abbrev(buf, "diamond")) {
    TRealContainer *trc = dynamic_cast<TRealContainer *>(obj);
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

  // if we hit body or neck, suck some life into user
  // we've already "hit" them, so life from vict has already been taken
  if (part_hit != WEAR_BODY && part_hit != WEAR_NECK)
    return FALSE;

  int amount = min(1, ch->hitLimit() - ch->getHit());
  if (amount) {
    act("$p draws the your life force through it into $N.",
       TRUE, vict, o, ch, TO_CHAR);
    act("$p draws the life force of $n through it into you.",
       TRUE, vict, o, ch, TO_VICT);
    act("$p draws the life force of $n through it into $N.",
       TRUE, vict, o, ch, TO_NOTVICT);
    ch->addToHit(amount);
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
            ch->sendTo("%s: While your in Grimhaven you can use the  goto  command to get\n\r",
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

extern int board(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int weaponBlinder(TBeing *, cmdTypeT, const char *, TObj *, TObj *);
extern int weaponManaDrainer(TBeing *, cmdTypeT, const char *, TObj *, TObj *);

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
  {FALSE, "BOGUS", bogusObjProc},  
  {FALSE, "BOGUS", bogusObjProc},  
  {FALSE, "BOGUS", bogusObjProc},  
  {FALSE, "BOGUS", bogusObjProc},  // 50
};
