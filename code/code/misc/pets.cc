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
  non-nullptr "this->rider"

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

void TBeing::petSave() {
  TDatabase db(DB_SNEEZY);
  affectedData *aff = nullptr, *an = nullptr;
  char* owner;
  int owner_id;

  // don't save non-named pets
  if (!(specials.act & ACT_STRINGS_CHANGED))
    return;
  // get the owner name
  for (an = affected; an; an = an->next) {
    if (an->type == AFFECT_PET) {
      aff = an;
      break;
    }
  }
  if (!aff)
    return;

  owner = (char*)aff->be;

  // get the owner player_id
  db.query("select id from player where name='%s'", owner);
  if (!db.fetchRow())
    return;
  owner_id = convertTo<int>(db["id"]);

  // get the pet name
  sstring short_desc = name;
  sstring name = "";
  if ((specials.act & ACT_STRINGS_CHANGED)) {
    for (int i = 0; !short_desc.word(i).empty(); ++i) {
      name = short_desc.word(i);
    }
  }

  // save
  db.query("delete from pet where player_id=%i and vnum=%i and name='%s'",
    owner_id, mobVnum(), name.c_str());
  db.query(
    "insert into pet (player_id, vnum, name, exp, level) values (%i, %i, '%s', "
    "%f, %i)",
    owner_id, mobVnum(), name.c_str(), getExp(), GetMaxLevel());
}

// bv is one of: the PETTYPES constants
bool TBeing::isPet(const unsigned int bv) const {
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
    if (affectedBySpell(AFFECT_PET) && !affectedBySpell(AFFECT_CHARM))
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
    return false;
  }


  if (rc <= 0 || ((numx = real_mobile(rc)) <= 0)) {
    vlogf(LOG_BUG, format("Problem in ReloadNPCAsNew (ERR 1).  Trying to reload %s.") % getName ());
    return false;
  }

  if (numx < 0 || numx >= (signed int) mob_index.size()) {
    vlogf(LOG_BUG, format("Problem in ReloadNPCAsNew (ERR 2).  Trying to reload %s.") % getName ());
    return false;
  }

  if (!(newMob = read_mobile(rc, REAL))) {
    vlogf(LOG_BUG, format("Problem in ReloadNPCAsNew (ERR 3).  Trying to reload %s.") % getName ());
    return false;
  }
  if (newMob->isShopkeeper()){
    vlogf(LOG_BUG, format("Problem in ReloadNPCAsNew (ERR 4).  Trying to reload shopkeepere-%s.") % getName ());
    delete newMob;
    return false;
  }
  if (mob_index[rc].spec == SPEC_NEWBIE_EQUIPPER) {
    vlogf(LOG_BUG, format("Problem in ReloadNPCAsNew (ERR 5).  Trying to reload newbieHelper -%s.") % getName ());
    delete newMob;
    return false;
  }

  *roomp += *newMob;
  newMob->oldRoom = inRoom();
  newMob->createWealth();
  return true;
}
#endif
int TBeing::getAffectedDataFromType(spellNumT whichAff, double whichField) {
  affectedData *an = nullptr, *aff = nullptr;
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
    vlogf(LOG_BUG,
      format("Somehow %s has 2 affectedDatas with same type (%d)") % getName() %
        whichAff);
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

int TBeing::getPetOrderLevel() {
  // Returns both true/false if not even a valid concept
  // returns an updated reference as well
  /* **Commented out so code could be compiled**
  affectedData *an = nullptr, *aff = nullptr;
  */

  // **Added so code could be compiled**
  return 0;
}

int TBeing::getPetAge() {
  // Returns both true/false if not even a valid concept
  // returns an updated reference as well

  /* **Commented out so code could be compiled**
  affectedData *an = nullptr, *aff = nullptr;
  */

  // **Added so code could be compiled**
  return 0;
}
#if 1
bool TMonster::isRetrainable() {
  affectedData* aff;

  for (aff = affected; aff; aff = aff->next) {
    if (aff->type != AFFECT_ORPHAN_PET)
      continue;
    if (aff->level == 0)
      return true;
  }
  return false;
}
#endif
bool TBeing::doRetrainPet(const char* argument, TBeing* vict) {
  TRoom* rp = nullptr;
  TBeing* mob = nullptr;
  TMonster* v = nullptr;

  // no room
  if (!(rp = roomp)) {
    vlogf(LOG_BUG,
      format("%s was in doRetrainPet without a roomp") % getName());
    return false;
  }

  // find target in room - visibility important

  if (!(mob = vict)) {
    if (!(mob = get_char_room_vis(this, argument))) {
      act("The one you want to retrain isnt here.", false, this, nullptr, nullptr,
        TO_CHAR);
      return false;
    }
  }
  // various checks

  if (!(v = dynamic_cast<TMonster*>(mob))) {
    act("You cant retrain $N.", false, this, nullptr, mob, TO_CHAR);
    return false;
  }

  if (v->master) {
    act("$N already has a master.", false, this, nullptr, v, TO_CHAR);
    return false;
  }

#if 1
  if (!v->isRetrainable()) {
    act("$N is not trainable.  Perhaps you have made a mistake.", false, this,
      nullptr, v, TO_CHAR);
    return false;
  }
#else
  if (!v->affectedBySpell(AFFECT_ORPHAN_PET)) {
    act("$N is not trainable.  Perhaps you have made a mistake.", false, this,
      nullptr, v, TO_CHAR);
    return false;
  }

#endif
  if (v->fight() || !v->awake()) {
    act("$N is busy now. Perhaps you should wait till the fight is over.",
      false, this, nullptr, v, TO_CHAR);
    return false;
  }

  if (v->isAffected(AFF_BLIND)) {
    act("$N is blind and not trainable.", false, this, nullptr, v, TO_CHAR);
    return false;
  }

  if (hasClass(CLASS_RANGER) && v->isAnimal()) {
    if (GetMaxLevel() >= v->GetMaxLevel()) {
      if (v->restorePetToPc(this)) {
        return true;
      } else {
        return false;
      }
    } else {
      act("$N is too powerful for you to retrain for its old master.", false,
        this, nullptr, v, TO_CHAR);
      return false;
    }
  }

  // 20% chance of rejection

  if (!::number(0, 4)) {
    act("$N rejects your retraining and remains wild.", false, this, nullptr, v,
      TO_CHAR);
    v->affectFrom(AFFECT_ORPHAN_PET);
    return false;
  }
  if (v->restorePetToPc(this)) {
    return true;

  } else {
    // taking affect orphan done in retore Pet
    return false;
  }
}

// **Added so code would compile**
bool TBeing::restorePetToPc(TBeing* ch) {
  TMonster* tMonster = dynamic_cast<TMonster*>(this);

  return (tMonster ? tMonster->restorePetToPc(ch) : false);
}

bool TMonster::restorePetToPc(TBeing* ch) {
  affectedData *aff = nullptr, *an = nullptr;
  char* affName = nullptr;
  TThing* t = nullptr;
  TBeing* pc = nullptr;
  bool found = false;

  if (fight() || !awake()) {
    return false;
  }
  for (an = affected; an; an = an->next) {
    if (an->type == AFFECT_PET) {
      aff = an;
      break;
    }
  }

  // **aff.be changed to aff->be**
  if (!aff || !aff->be) {
    act("$N has never been a pet and can not be retrained.", false, ch, nullptr,
      this, TO_CHAR);
    if (affectedBySpell(AFFECT_ORPHAN_PET)) {
      affectFrom(AFFECT_ORPHAN_PET);
      vlogf(LOG_BUG,
        format("A non pet with AFFECT_ORPHAN_PET (%s).") % getName());
    }
    return false;
  }
  // **semicolon added to end of line**
  // **aff.be changed to aff->be**
  affName = (char*)aff->be;

  for (StuffIter it = roomp->stuff.begin();
       it != roomp->stuff.end() && (t = *it); ++it) {
    if (!(pc = dynamic_cast<TBeing*>(t)))
      continue;
    if (is_exact_name(affName, pc->getName())) {
      found = true;
      break;
    }
  }

  if (!pc) {
    act("$N's owner is not in this room.", false, ch, nullptr, this, TO_CHAR);
    return false;
  }

  if (!found) {
    act("$N's owner is not in this room.", false, ch, nullptr, this, TO_CHAR);
    return false;
  }

  if ((ch != pc) && !ch->hasClass(CLASS_RANGER) && !ch->isImmortal()) {
    act("Only rangers can retrain a pet for someone other than themselves.",
      false, ch, nullptr, this, TO_CHAR);
    return false;
  }

  if (pc->tooManyFollowers(this, FOL_PET)) {
    if (ch == pc) {
      act("Your charmisma won't support the retraining of this $N.", false, ch,
        nullptr, this, TO_CHAR);
    } else {
      act("$p's charmisma won't support the retraining of this $N.", false, ch,
        pc, this, TO_CHAR);
    }
    return false;
  }

  SET_BIT(specials.affectedBy, AFF_CHARM);
  pc->addFollower(this);
  affectFrom(AFFECT_ORPHAN_PET);
  if (pc == ch) {
    act("$N has been restored to your side.", false, pc, nullptr, this, TO_CHAR);
    act("$N has been restored to $s place at $n's side.", false, pc, nullptr, this,
      TO_ROOM);
  } else {
    act("$N has been restored to your side.", false, pc, nullptr, this, TO_CHAR);
    act("You have restored $N to $E owner.", false, ch, nullptr, this, TO_VICT);
    act("$N has been restored to $S place at $n's side.", false, pc, nullptr, this,
      TO_ROOM);
  }
  return true;
}
