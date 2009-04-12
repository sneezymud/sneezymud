//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "handler.h"
#include "being.h"
#include "monster.h"
#include "room.h"
#include "extern.h"
#include "database.h"

/* **********************************************************************
Every mob that follows a PC should be classified as one of the following:

A Mount: obvious.  Handled elsewhere and distinguished by having
  non-NULL "this->rider"

A Thrall: Basically, a mob that owes its life force to the master.  Will
  do the masters bidding without question or hesitation.  Has no sense
  of self-preservation.  e.g. zombies, automatons, golems, etc.
        AFF_CHARM: yes
        affect->type: AFFECT_THRALL

A Charm: A mob beguiled by the master into following orders, most likely,
  against the mob's will.  The charmed mob is "fighting" against the
  master, so obviously destructive commands can be ignored, but otherwise
  will do master's bidding.
        AFF_CHARM: yes
        affect->type: SPELL_ENSORCER

A Pet: A faithful companion that will generally assist its master.  Pets are
  not compelled to follow orders, but may do simple actions.  Pets have an
  unhindered sense of self-preservation.
        AFF_CHARM: yes
        affect->type: AFFECT_PET

Pets and Thralls may be "orphaned" if master dies.

The effects of being a thrall trump the effects of being a charm, and in turn
a pet.  That is, if I charm a pet, its a charm (not a pet).

********************************************************************** */

void TBeing::petSave()
{
  TDatabase db(DB_SNEEZY);
  affectedData *aff = NULL, *an = NULL;
  char *owner;
  int owner_id;

  // don't save non-named pets
  if(!(specials.act & ACT_STRINGS_CHANGED))
    return;
  // get the owner name
  for (an = affected; an; an = an->next) {
    if (an->type == AFFECT_PET) {
      aff = an;
      break;
    }
  }
  if(!aff)
    return;

  owner=(char *)aff->be;
  
  // get the owner player_id
  db.query("select id from player where name='%s'", owner);
  if(!db.fetchRow())
    return;
  owner_id=convertTo<int>(db["id"]);

  // get the pet name
  sstring short_desc=name;
  sstring name="";
  if((specials.act & ACT_STRINGS_CHANGED)){
    for(int i=0;!short_desc.word(i).empty();++i){
      name=short_desc.word(i);
    }
  }
  
  // save
  db.query("delete from pet where player_id=%i and vnum=%i and name='%s'",
	   owner_id, mobVnum(), name.c_str());
  db.query("insert into pet (player_id, vnum, name, exp, level) values (%i, %i, '%s', %f, %i)", owner_id, mobVnum(), name.c_str(), getExp(), GetMaxLevel());

  return;
}

// bv is one of: the PETTYPES constants
bool TBeing::isPet(const unsigned int bv) const
{
  if (isPc() || !master)
    return false;

  // if not taking orders, don't consider me any of the above
  if (!isAffected(AFF_CHARM))
    return false;

  // check special case first (for speed)
  // if considering ANY of the valid types, all I really need to look
  // for is the AFF_CHARM bit, if we got here we have it, therefore...
  if (bv == (PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL))
    return true;

  if (IS_SET(bv, PETTYPE_PET)) {
    if (affectedBySpell(AFFECT_PET) &&
        !affectedBySpell(AFFECT_CHARM))
      return true;
  }
  if (IS_SET(bv, PETTYPE_CHARM)) {
    if (affectedBySpell(AFFECT_CHARM))
      return true;
  }
  if (IS_SET(bv, PETTYPE_THRALL)) {
    if (affectedBySpell(AFFECT_THRALL))
      return true;
  }
  return false;
}

#if 0
  bool TMonster::reloadNPCAsNew()
{
  int rc, numx;

  rc = mobVnum();

  if (mobVnum() < 0) {
    vlogf(LOG_BUG, format("Attempt to reload a prototype in ReloadNPCAsNew.  Trying to reload %s.") % getName ());
    return FALSE;
  }


  if (rc <= 0 || ((numx = real_mobile(rc)) <= 0)) {
    vlogf(LOG_BUG, format("Problem in ReloadNPCAsNew (ERR 1).  Trying to reload %s.") % getName ());
    return FALSE;
  }

  if (numx < 0 || numx >= (signed int) mob_index.size()) {
    vlogf(LOG_BUG, format("Problem in ReloadNPCAsNew (ERR 2).  Trying to reload %s.") % getName ());
    return FALSE;
  }

  if (!(newMob = read_mobile(rc, REAL))) {
    vlogf(LOG_BUG, format("Problem in ReloadNPCAsNew (ERR 3).  Trying to reload %s.") % getName ());
    return FALSE;
  }
  if (newMob->isShopkeeper()){
    vlogf(LOG_BUG, format("Problem in ReloadNPCAsNew (ERR 4).  Trying to reload shopkeepere-%s.") % getName ());
    delete newMob;
    return FALSE;
  }
  if (mob_index[rc].spec == SPEC_NEWBIE_EQUIPPER) {
    vlogf(LOG_BUG, format("Problem in ReloadNPCAsNew (ERR 5).  Trying to reload newbieHelper -%s.") % getName ());
    delete newMob;
    return FALSE;
  }

  *roomp += *newMob;
  newMob->oldRoom = inRoom();
  newMob->createWealth();
  return TRUE;
}
#endif
int TBeing::getAffectedDataFromType(spellNumT whichAff, double whichField) 
{
  affectedData *an = NULL,
               *aff = NULL;
  int numAffs = 0;

  for (an = affected; an; an = an->next) {
    if (an->type == whichAff) {
      aff = an;
      numAffs++;
    }
  }

  if (!aff)
    return 0;

  if (numAffs > 1) {
    vlogf(LOG_BUG, format("Somehow %s has 2 affectedDatas with same type (%d)") % 
          getName() % whichAff);
  }

  /* **Commented out so code could be compiled**
  if (IS_SET(whichField, AFFECT_FIELD_LEVEL)) {
  } else if (IS_SET(whichField, AFFECT_FIELD_DURATION)) {
  } else if (IS_SET(whichField, AFFECT_FIELD_RENEW)) {
  } else if (IS_SET(whichField, AFFECT_FIELD_MODIFIER)) {
  } else if (IS_SET(whichField, AFFECT_FIELD_MODIFIER2)) {
  } else if (IS_SET(whichField, AFFECT_FIELD_LOCATION)) {
  } else if (IS_SET(whichField, AFFECT_FIELD_BITVECTOR)) {
  } else if (IS_SET(whichField, AFFECT_FIELD_BE)) {
  } else if (IS_SET(whichField, AFFECT_FIELD_NEXT)) {
  }
  */

  // **Added so code could be compiled**
  return 0;
} 

int TBeing::getPetOrderLevel()
{
// Returns both true/false if not even a valid concept
// returns an updated reference as well
  /* **Commented out so code could be compiled**
  affectedData *an = NULL, *aff = NULL;
  */

  // **Added so code could be compiled**
  return 0;
}

int TBeing::getPetAge() 
{
// Returns both true/false if not even a valid concept
// returns an updated reference as well

  /* **Commented out so code could be compiled**
  affectedData *an = NULL, *aff = NULL;
  */

  // **Added so code could be compiled**
  return 0;
}
#if 1 
bool TMonster::isRetrainable() 
{
  affectedData *aff;

  for (aff = affected; aff; aff = aff->next) {
    if (aff->type != AFFECT_ORPHAN_PET)
      continue;
    if (aff->level == 0)
      return TRUE; 
  }
  return FALSE;
}
#endif
bool TBeing::doRetrainPet(const char *argument, TBeing *vict)
{
  TRoom *rp = NULL;
  TBeing *mob = NULL;
  TMonster *v = NULL;

// no room
  if (!(rp = roomp)) {
    vlogf(LOG_BUG, format("%s was in doRetrainPet without a roomp") %  getName());
    return FALSE;
  }

// find target in room - visibility important

  if (!(mob = vict)) {
    if (!(mob = get_char_room_vis(this, argument))) {
      act("The one you want to retrain isnt here.",
          FALSE, this, NULL, NULL, TO_CHAR);
      return FALSE;
    }
  }
// various checks

  if (!(v = dynamic_cast<TMonster *>(mob))) {
    act("You cant retrain $N.",
        FALSE, this, NULL, mob, TO_CHAR);
    return FALSE;
  }

  if (v->master) {
    act("$N already has a master.",
        FALSE, this, NULL, v, TO_CHAR);
    return FALSE;
  }

#if 1 
  if (!v->isRetrainable()) {
    act("$N is not trainable.  Perhaps you have made a mistake.",
        FALSE, this, NULL, v, TO_CHAR);
    return FALSE;
  }
#else
  if (!v->affectedBySpell(AFFECT_ORPHAN_PET)) {
    act("$N is not trainable.  Perhaps you have made a mistake.",
        FALSE, this, NULL, v, TO_CHAR);
    return FALSE;
  }

#endif
  if (v->fight() || !v->awake()) {
    act("$N is busy now. Perhaps you should wait till the fight is over.",
        FALSE, this, NULL, v, TO_CHAR);
    return FALSE;
  }

  if (v->isAffected(AFF_BLIND)) {
    act("$N is blind and not trainable.",
        FALSE, this, NULL, v, TO_CHAR);
    return FALSE;
  }

  if (hasClass(CLASS_RANGER) && v->isAnimal()) {
    if (GetMaxLevel() >= v->GetMaxLevel()) {
      if (v->restorePetToPc(this)) {
        return TRUE;
      } else {
        return FALSE;
      }
    } else {
      act("$N is too powerful for you to retrain for its old master.",
          FALSE, this, NULL, v, TO_CHAR);
      return FALSE;
    }
  }

// 20% chance of rejection

  if (!::number(0,4)) {
    act("$N rejects your retraining and remains wild.",
        FALSE, this, NULL, v, TO_CHAR);
    v->affectFrom(AFFECT_ORPHAN_PET);
    return FALSE;
  }
  if (v->restorePetToPc(this)) {
    return TRUE;

  } else {
// taking affect orphan done in retore Pet
    return FALSE;
  }
}

// **Added so code would compile**
bool TBeing::restorePetToPc(TBeing *ch)
{
  TMonster *tMonster = dynamic_cast<TMonster *>(this);

  return (tMonster ? tMonster->restorePetToPc(ch) : false);
}

bool TMonster::restorePetToPc(TBeing *ch)
{
  TRoom *rp = NULL;
  affectedData *aff = NULL, *an = NULL;
  char * affName = NULL;
  TThing *t = NULL;
  TBeing *pc = NULL;
  bool found = FALSE;

  if (fight() || !awake()) {
    return FALSE;
  }
  for (an = affected; an; an = an->next) {
    if (an->type == AFFECT_PET) {
      aff = an;
      break;
    }
  }

  // **aff.be changed to aff->be**
  if (!aff || !aff->be) {
    act("$N has never been a pet and can not be retrained.",
        FALSE, ch, NULL, this, TO_CHAR);
    if (affectedBySpell(AFFECT_ORPHAN_PET)) {
      affectFrom(AFFECT_ORPHAN_PET); 
      vlogf(LOG_BUG, format("A non pet with AFFECT_ORPHAN_PET (%s).") %  getName());
    }
    return FALSE;
  }
  // **semicolon added to end of line**
  // **aff.be changed to aff->be**
  affName = (char *) aff->be;
  rp = roomp;

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
    if (!(pc = dynamic_cast<TBeing *>(t)))
      continue;
    if (is_exact_name(affName, pc->getName())) {
      found = TRUE;
      break;
    }
  }

  if (!pc) {
    act("$N's owner is not in this room.",
        FALSE, ch, NULL, this, TO_CHAR);
    return FALSE;
  }

  if (!found) {
    act("$N's owner is not in this room.",
        FALSE, ch, NULL, this, TO_CHAR);
    return FALSE;
  }

  if ((ch != pc) && !ch->hasClass(CLASS_RANGER) && !ch->isImmortal()) {
    act("Only rangers can retrain a pet for someone other than themselves.",
        FALSE, ch, NULL, this, TO_CHAR);
    return FALSE;
  }

  if (pc->tooManyFollowers(this, FOL_PET)) {
    if (ch == pc) {
      act("Your charmisma won't support the retraining of this $N.",
          FALSE, ch, NULL, this, TO_CHAR);
    } else {
      act("$p's charmisma won't support the retraining of this $N.",
          FALSE, ch, pc, this, TO_CHAR);
    }
    return FALSE;
  }


  SET_BIT(specials.affectedBy, AFF_CHARM);
  pc->addFollower(this);
  affectFrom(AFFECT_ORPHAN_PET); 
  if (pc == ch) {
    act("$N has been restored to your side.",
          FALSE, pc, NULL, this, TO_CHAR);
    act("$N has been restored to $s place at $n's side.",
          FALSE, pc, NULL, this, TO_ROOM);
  } else {
    act("$N has been restored to your side.",
         FALSE, pc, NULL, this, TO_CHAR);
    act("You have restored $N to $E owner.",
         FALSE, ch, NULL, this, TO_VICT);
    act("$N has been restored to $S place at $n's side.",
          FALSE, pc, NULL, this, TO_ROOM);
  }
  return TRUE;
}

