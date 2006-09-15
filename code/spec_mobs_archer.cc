#include "stdsneezy.h"
#include "obj_bow.h"
#include "obj_arrow.h"
#include "obj_quiver.h"
#include "being.h"

#define MAX_RANGE 3

vector <TBow *> TBeing::getBows() 
{
  TBow *temp = NULL;
  vector <TBow *> bows;
  wearSlotT i;
  TThing *j;
  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
// if you are carrying it you know it's there
//    if (!canSee(equipment[i]))
//      continue;
    if ((temp = dynamic_cast<TBow *>(equipment[i])))
      bows.push_back(temp);
  }
  for (j = getStuff(); j; j = j->nextThing) {
    if (!canSee(j))
      continue;
    if ((temp = dynamic_cast<TBow *>(j)))
      bows.push_back(temp);
  }
  return bows;
}
 
TArrow *TBeing::autoGetAmmoQuiver(TBow *bow, TQuiver *quiver) 
{
  TThing *i;
  TArrow *temp = NULL;
  TArrow *ammo = NULL;
  
  if (!bow || !quiver || quiver->isClosed())
    return ammo;
  
  for (i = quiver->getStuff(); i; i = i->nextThing)
  {
    if ((temp = dynamic_cast<TArrow *>(i)) &&
        bow->getArrowType() == temp->getArrowType() &&
        canSee(i)) {
      ammo = temp;
      break;
    }
  }
  return ammo;
}

TArrow *TBeing::autoGetAmmo(TBow *bow)
{
  // do search inventory for ammo - might want to check first that the
  // bow isn't already loaded
  TArrow *temp = NULL;
  TArrow *ammo = NULL;
  TQuiver *quiver = NULL;
  wearSlotT i;
  TThing *j;

  if (!bow)
    return ammo;
  
  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
// if you are carrying it then you know it's there
//    if (!canSee(i))
//      continue;
    if ((quiver = dynamic_cast<TQuiver *>(equipment[i]))) {
      if ((ammo = autoGetAmmoQuiver(bow, quiver)))
        break;
    }
    else if ((temp = dynamic_cast<TArrow *>(equipment[i])) 
        && bow->getArrowType() == temp->getArrowType()) {
      ammo = temp;
      break;
    }
  }
  if (!ammo) { // no ammo on person - check inventory
    for (j = getStuff(); j; j = j->nextThing) {
      if (!canSee(j))
        continue;
      if ((quiver = dynamic_cast<TQuiver *>(j))) {
        if ((ammo = autoGetAmmoQuiver(bow, quiver)))
          break;
      }
      else if ((temp = dynamic_cast<TArrow *>(j)) 
          && bow->getArrowType() == temp->getArrowType()) {
        ammo = temp;
        break;
      }
    }
  }
  return ammo; 
}

// returns TRUE for shot or restring, otherwise FALSE
// DELETE_THIS and DELETE_OBJECT for a couple of calls
int archer(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *)
{
  if (cmd != CMD_GENERIC_PULSE) 
    return FALSE;
  
  int rm = 0, new_rm = 0;
  TThing *t;
  const char *directions[][2] =
  {
    {"north", "south"},
    {"east", "west"},
    {"south", "north"},
    {"west", "east"},
    {"up", "below"},
    {"down", "above"},
    {"northeast", "southwest"},
    {"northwest", "southeast"},
    {"southeast", "northwest"},
    {"southwest", "northeast"}
  };

  sstring temp, buf;
  int count = 0, numsimilar, range;
  int which;
  int Hi = 0, Hf = 0; //hp initial, hp final
  dirTypeT i;
  TBeing *tbt = NULL;
  TBeing *tbt2 = NULL;


// ammo check
  TBow *bow = NULL;
  TArrow *ammo = NULL;
  TArrow *tempArr = NULL;
  unsigned int j;
  vector <TBow *> bows = ch->getBows();
  for (j = 0; j < bows.size(); j++)
  {
    bow = bows[j];
  if (bow->getStuff())
      break;
    if (!bow) vlogf(LOG_BUG, fmt("spec_mobs_archer.cc: archer: bow is null somehow"));
    if (bow && (tempArr = ch->autoGetAmmo(bow))) {
      ammo = tempArr;
      break;
    }
  }

  if (!bow || (!ammo && !bow->getStuff()))
      return FALSE;

// if I have ammo, then I don't want to fight straight up
// if I'm aggro and in the same room as a Pc, or I hate a PC
// in my room, flee out first
// only want to do this fleeing if they have a bow
  for (t = ch->roomp->getStuff(); t; t = t->nextThing) {
    if ((tbt = dynamic_cast<TBeing *>(t))) {
      if (tbt->isPc() && (IS_SET(ch->specials.act, ACT_AGGRESSIVE)) ||
          ch->Hates(tbt, NULL))
        ch->doFlee("");
      if (!ch->canSee(bow) || !ch->canSee(ammo)) // fled to darker spot
      {
        vector <TBow *> bows = ch->getBows();
        for (j = 0; j < bows.size(); j++)
        {
          bow = bows[j];
        if (bow->getStuff())
            break;
          if (!bow) vlogf(LOG_BUG, fmt("spec_mobs_archer.cc: archer: bow is null somehow"));
          if (bow && (tempArr = ch->autoGetAmmo(bow))) {
            ammo = tempArr;
            break;
          }
        }
        if (!bow || (!ammo && !bow->getStuff()))
          return FALSE;
      }
    }
  }
 
  
  tbt = NULL;
// find target
  for (i = MIN_DIR; i <= (MAX_DIR - 1); i++) {
// keep looking to max range
    rm = ch->in_room;
    for (range = 1; range <= MAX_RANGE; range++) {
      if (clearpath(rm, i)) {
        new_rm = real_roomp(rm)->dir_option[i]->to_room;
        if (new_rm == rm || (real_roomp(rm)->isRoomFlag(ROOM_PEACEFUL)))
          continue;
        else
          rm = new_rm;
        
        count = 0;
  
        for (t = real_roomp(rm)->getStuff(); t; t = t->nextThing) {
          tbt = dynamic_cast<TBeing *>(t);
          if (!tbt || !tbt->isPc())
            continue;
          if (!ch->canSee(tbt))
            continue;
          if (!ch->Hates(tbt, NULL) && !IS_SET(ch->specials.act, ACT_AGGRESSIVE))
            continue;
          //we have a room with a mob we are after
          count++;
        }
      }
      if (count == 0)
	      continue;
      
      which = ::number(1,count);// which target to pick on the next pass
      count = 0;
     for (t = real_roomp(rm)->getStuff(); t; t = t->nextThing) {
      	tbt = dynamic_cast<TBeing *>(t);
      	if (!tbt || !tbt->isPc())
	        continue;
	      if (!ch->canSee(tbt))
          continue;
        if (!ch->Hates(tbt, NULL) && !IS_SET(ch->specials.act, ACT_AGGRESSIVE))
          continue;
        count++;
        
	      if (count == which) // found victim
	        break;
      }
	
      // ok now we have to do a bit of trickery - there could be multiple identical mobs
      // in the same room, ie, 3 orc guards, and even if we pick the third one our code
      // will only aim at the first one.  So we have to figure out which we're aiming at
      // before we can lob one off and hope to hit him.


      count = 0;
      if (tbt) {
        temp = tbt->getName();
      } else {
        vlogf(LOG_BUG, fmt("spec_mobs_archer.cc: archer: no tbt for some reason"));
        return FALSE;
      }
      numsimilar = 0;
      for (t = real_roomp(rm)->getStuff(); t; t = t->nextThing) {
        tbt2 = dynamic_cast<TBeing *>(t);
        if (!tbt2)
          continue;
        if (!ch->canSee(tbt2))
          continue;
      	if (ch->Hates(tbt, NULL))
	        count++;
	      if (temp.find(tbt2->name) == sstring::npos)
        // this mob has the same name as our target, so count him
	        numsimilar++;
	      if (count == which)
          break;
      }

      numsimilar = max(numsimilar, 1);// sometimes we get 0 instead of 1 if there is only one in the room
      temp = add_bars(temp);
    // check for bow and ammo combination

    if (bow->isBowFlag(BOW_STRING_BROKE)) {
    
      act("You quickly restring $p.", FALSE, ch, bow, 0, TO_CHAR);
      act("$n quickly restrings $p.", FALSE, ch, bow, 0, TO_ROOM);

      bow->remBowFlags(BOW_STRING_BROKE);
      return TRUE;
    }
  
    // check for ammo and load into bow if necessary
    // bload handles case of arrow already in bow for us
      int rc;
      TThing *t = NULL;
      t = ch->equipment[HOLD_LEFT];
      rc = ch->doRemove("", t);   
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
                  
      t = ch->equipment[HOLD_RIGHT];
      rc = ch->doRemove("", t);   
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      
    bow->bloadArrowBow(ch, ammo);
    if(!(bow = dynamic_cast<TBow *>(ch->equipment[ch->getPrimaryHold()]))
        || !bow->getStuff())
      return FALSE; // in case bload fails for some reason
    
    // shoot target and remove bow
    Hi = tbt->getHit();
  
    // text to character
        
    buf = fmt("%s %d.%s %d") % directions[i][0] % numsimilar % temp % range;

    temp = tbt->getName();
  
// too mean? removed for now - otherwise engage-all won't help you even
// when he's out of arrows
    if (!ch->specials.hunting || ch->specials.hunting != tbt)
      ch->setHunting(tbt);
//    if (!ch->Hates(tbt, NULL))
//      ch->addHated(tbt);

    if (!(ch->doShoot(buf.c_str())))
      vlogf(LOG_BUG, fmt("spec_mobs_archer.cc: archer: error shooting bow with arguments: %s") % buf);
        
      t = ch->equipment[HOLD_LEFT];
      rc = ch->doRemove("", t);   
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
                  
      t = ch->equipment[HOLD_RIGHT];
      rc = ch->doRemove("", t);   
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
        
      Hf = tbt->getHit();
  
#if 0
        vlogf(LOG_MAROR, fmt("archer: %d->%d, temp/name: (%s)/(%s), tbt?: %s") % 
            Hi % Hf % temp % (tbt->getName() ? tbt->getName() : "(NULL)") % (tbt ? "exists" : "(NULL)"));
#endif
      return TRUE; 
    }
  }
  return FALSE;
}
