//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: combat.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.7  1999/10/14 04:45:39  cosmo
// *** empty log message ***
//
// Revision 1.6  1999/10/14 03:49:57  cosmo
// Made Fix so that any magic weapon is considered a magic weapon.
//
// Revision 1.5  1999/10/12 00:35:25  lapsos
// Added vlogf for shopkeeper kills and invenotry nuking.
//
// Revision 1.3  1999/09/30 01:07:10  lapsos
// Added code for the new mobile strings.
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
//
//      "combat.cc" - All functions and routines related to combat
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "stdsneezy.h"
#include "range.h"
#include "combat.h"
#include "statistics.h"
#include "disease.h"
#include "mail.h"
#include "shop.h"

#define DAMAGE_DEBUG 0

extern class TPeelPk peelPk;

TBeing *gCombatList = NULL;        // head of l-list of fighting chars    

// So WTF are we doing here?  glad you asked
// GCN maintains the next pointer for the perform_violance loop
// Imagine if while I am fighting, I cause the gcn guy to stop fighting
// if GCN was local to perform_violance, this would cause it to walk onto
// a player no longer fighting (bad in itself), plus lose track of later
// fighters (really bad).  So we maintain this globally, and make some
// checks for GCN in stopFighting, adjusting as appropriate.
// If you're still confused, trust me, we need it - Bat 9/1/98
TBeing *gCombatNext = NULL;

struct attack_hit_type attack_hit_text[TYPE_MAX_HIT - TYPE_MIN_HIT] =
{
  {"pound", "pounds", "pounding"},        // 0
  {"bludgeon", "bludgeons", "bludgeoning"},
  {"whip", "whips","whipping"},
  {"crush", "crushes", "crushing"},
  {"smash", "smashes", "smashing"},
  {"smite", "smites", "smiting"},
  {"pummel", "pummels", "pummeling"},
  {"flail", "flails", "flailing"},
  {"beat", "beats", "beating"},
  {"thrash", "thrashes", "thrashing"},
  {"thump","thumps", "thumping"},
  {"wallop", "wallops", "walloping"},
  {"batter","batters", "battering"},
  {"strike","strikes", "striking"},
  {"club","clubs", "clubbing"},
  {"pound","pounds", "pounding"},
  {"pierce", "pierces", "piercing"},        
  {"bite", "bites","biting"},
  {"sting", "stings", "stinging"},
  {"stab", "stabs", "stabbing"},
  {"thrust", "thrusts", "thrusting"},
  {"spear", "spears", "spearing"},
  {"peck", "pecks", "pecking"},
  {"slash", "slashes","slashing"},
  {"claw", "claws", "clawing"},
  {"cleave", "cleaves", "cleaving"},
  {"slice","slices", "slicing"},
  {"dust", "dusts", "dusting"},
  {"rock", "rocks", "rocking"},
  {"torch", "torches", "torching"},
  {"splash", "splashes", "splashing"},
  {"maul", "mauls", "mauling"},  // TYPE_BEAR_CLAW
  {"kick", "kicks", "kicking"},
  {"maul", "mauls", "mauling"},
};

// isTanking() checks to see if I am tanking.  Conditions are if someone in
// this room is beating on me.  There is also an instance where I'm initiating
// combat and for that moment when I'm fighting and my victim isn't, I'm still
// considered the tank.
bool TBeing::isTanking()
{
  TBeing *victim = NULL;
  TThing *contents;

  //  Look through the current room and check each object.  If the object is
  //  fighting and I'm the one it is fighting, then I'm tanking.
  for (contents = roomp->stuff; contents; contents = contents->nextThing) {
    TBeing *tbg = dynamic_cast<TBeing *>(contents);
    if (tbg) {
      victim = tbg;
      if (this == victim->fight())
        return TRUE;
    }
  }

  //  If I'm fighting but the thing I'm fighting isn't fighting back, then
  //  I'm still tanking.
  if (fight() && !fight()->fight())
    return TRUE;

  return FALSE;
}

// value passed thru cur will NOT be selected for bezerking
void TBeing::goBerserk(TBeing *cur)
{
  TThing *t, *temp;
  TBeing *tmp;

  tmp = NULL;
  if (!isCombatMode(ATTACK_BERSERK))
    return;
  if (fight()) {
    vlogf(5,"Whoa, %s was fighting when it tried to goBerserk",getName());
    return;
  }
  if (getPosition() <= POSITION_SLEEPING)
    return;

  if (checkPeaceful("")) {
    vlogf(5,"Hmmm, %s was berserking in a peaceful room.",getName());
    setCombatMode(ATTACK_NORMAL);
    return;
  }
  for (t = roomp->stuff; t; t = temp) {
    temp = t->nextThing;
    tmp = dynamic_cast<TBeing *>(t);
    if (!tmp)
      continue;

    if ((tmp == this) || (dynamic_cast<TPerson *>(tmp)) || tmp->rider || tmp == cur)
      continue;

    if ((tmp->getPosition() <= POSITION_SLEEPING) || !canSee(tmp))
      continue;

    // this used to have a isPet() check too, seems like we should skip
    // all elems, thralls, charms, epts etc, so...
    if ((tmp->inGroup(this) || tmp->master == this))
      continue;

    if (setCharFighting(tmp, 0) == -1)
      continue;

    act("You go berserk on $N.",TRUE,this,0,tmp,TO_CHAR);
    act("$n's berserker rage is unleashed on you!",TRUE,this,0,tmp,TO_VICT);
    act("$n's berserker rage is unleashed on $N.",TRUE,this,0,tmp,TO_ROOM);
    break;
  }
  return;
}

void TBeing::deathCry()
{
  int new_room;
  TRoom *newR;
  char buf[256];
  TThing *i;
  dirTypeT door;

  if ((in_room == ROOM_NOWHERE) || !roomp)
    return;

#if 1
  strcpy(buf, ((ex_description && ex_description->findExtraDesc("deathcry")) ?
               ex_description->findExtraDesc("deathcry") : 
               "Your blood freezes as you hear $N's death cry."));
#else
  sprintf(buf, "Your blood freezes as you hear %s's death cry.\n\r", getName());
#endif

  for (door = MIN_DIR; door < MAX_DIR; door++) {
#if 1
    if (!roomp->dir_option[door])
      continue;

    new_room = roomp->dir_option[door]->to_room;
    newR     = real_roomp(new_room);
    TMonster *tMons;

    if (in_room != new_room && canGo(door)) {
      for (i = newR->stuff; i; i = i->nextThing) {
        if (!inGrimhaven() && (tMons = dynamic_cast<TMonster *>(i))) {
          tMons->UA(1);
          tMons->UM(1);
          tMons->US(4);
        } else if (dynamic_cast<TBeing *>(i)) {
          colorAct(COLOR_MOBS, buf, FALSE, dynamic_cast<TBeing *>(i), NULL, this, TO_CHAR);
        }
      }
    }
#else
    if (canGo(door)) {
      new_room = roomp->dir_option[door]->to_room;
      newR = real_roomp(new_room);
      if (in_room != new_room) {
        sendrpf(COLOR_MOBS, newR, buf);

	if (!inGrimhaven()) {
	  for (i = newR->stuff; i; i = i->nextThing) {
	    TMonster *tmons = dynamic_cast<TMonster *>(i);

	    if (tmons) {
	      tmons->UA(1);
	      tmons->UM(1);
	      tmons->US(4);
	    } else if (dynamic_cast<TBeing *>(i))
              colorAct(COLOR_MOBS, 
	  }
	}
      }
    }
#endif
  }
}

void TBeing::appear()
{
  if (isPc()) {
    if (affectedBySpell(SPELL_INVISIBILITY) && isAffected(AFF_INVISIBLE))
      doVisible("", true);

    // They shouldn't get here.  But things happen.
    if (isAffected(AFF_INVISIBLE))
      REMOVE_BIT(specials.affectedBy, AFF_INVISIBLE);

    act("$n slowly fades into existence.", FALSE, this, 0, 0, TO_ROOM);
    act("You slowly fade into existence.", FALSE, this, 0, 0, TO_CHAR);
  } else {
    if (affectedBySpell(SPELL_INVISIBILITY))
      affectFrom(SPELL_INVISIBILITY);

    if (isAffected(AFF_INVISIBLE))
      REMOVE_BIT(specials.affectedBy, AFF_INVISIBLE);

    act("$n slowly fades into existence.", FALSE, this, 0, 0, TO_ROOM);
  }
}

void TBeing::updatePos()
{
  positionTypeT newPos = POSITION_DEAD;

  if ((getHit() > 0) && (getPosition() > POSITION_STUNNED))
    return;

  else if (getHit() > 0) {
    if (getPosition() == POSITION_STUNNED && !isAffected(AFF_PARALYSIS)) {
      act("You shake the fuzziness from your head and sit up.", 
                 FALSE, this, 0, 0, TO_CHAR);
      act("$n shakes the fuzziness from $s head and sits up.", 
                 FALSE, this, 0, 0, TO_ROOM);
      setPosition(POSITION_SITTING);
    } else if (getPosition() != POSITION_STUNNED) {
      // mortaled, and got healed
      act("It looks like you might survive after all.",
                 FALSE, this, 0, 0, TO_CHAR);
      act("$n looks much better, although $e is still unconscious.",
                 FALSE, this, 0, 0, TO_ROOM);
      setPosition(POSITION_STUNNED);
    }

    if (!isAffected(AFF_PARALYSIS)) {
      if (dynamic_cast<TBeing *>(riding))
        setPosition(POSITION_MOUNTED);
    } else
      setPosition(POSITION_STUNNED);
    
    // save them: got healed, mud crashes, last save would say they were neg...
    doSave(SILENT_YES);
    return;
  }
  if (getHit() <= -11) 
    newPos = POSITION_DEAD;
  else if (getHit() <= -6) 
    newPos = POSITION_MORTALLYW;
  else if (getHit() <= -3) 
    newPos = POSITION_INCAP;
  else 
    newPos = POSITION_STUNNED;

  if (riding && dynamic_cast<TBeing *> (riding)) 
    fallOffMount(riding, newPos);
  else if (riding) 
    fallOffMount(riding, newPos, TRUE);
  
#if 1
  if ((newPos == POSITION_DEAD) || (newPos == POSITION_STUNNED)) 
    setPosition(newPos);
  else if ((newPos <= POSITION_STUNNED)) {
    setPosition(newPos);
    doSave(SILENT_YES);
  } else {
    vlogf(5,"Bad position in update position for %s", getName());
    setPosition(POSITION_STUNNED);
  }
#else
  if (getHit() <= -11)
    setPosition(POSITION_DEAD);
  else if (getHit() <= -6) {
    setPosition(POSITION_MORTALLYW);
    doSave(SILENT_YES);
  } else if (getHit() <= -3) {
    setPosition(POSITION_INCAP);
    doSave(SILENT_YES);
  } else
    setPosition(POSITION_STUNNED);
#endif

}

// always returns DELETE_THIS
int TMonster::rawKill(int dmg_type)
{
  TBeing *mob = NULL, *per = NULL;

  if (desc || isPc()) {
    if (!desc || !desc->original) {
      vlogf(10, "*BUG CODERS BIG TIME* (rawKILL)");
      sendTo("There is a problem. Please contact a god and bug what you did?!\n\r");
      return FALSE;
    }

    sendTo("You return to your original body.\n\r");

    if ((specials.act & ACT_POLYSELF)) {
      mob = this;
      per = desc->original;

      act("$n turns liquid, and reforms as $N.", TRUE, mob, 0, per, TO_ROOM);
      --(*per);
      *mob->roomp += *per;

      SwitchStuff(mob, per);
    }
    desc->original->polyed = POLY_TYPE_NONE;
    desc->character = desc->original;
    desc->original = NULL;

    desc->character->desc = desc;
    desc = NULL;
    if ((per->rawKill(dmg_type)) == DELETE_THIS) {
      per->reformGroup();
      delete per;
      per = NULL;
    }
    return DELETE_THIS;
  }
  return TBeing::rawKill(dmg_type);
}

// always returns DELETE_THIS
int TBeing::rawKill(int dmg_type)
{
  if (fight())  {
    followData *f;
    for (f = fight()->followers;f;f = f->next) {
      if (f->follower->desc && f->follower->sameRoom(fight())) {
        if (IS_SET(f->follower->desc->autobits, AUTO_LIMBS))
          f->follower->doLimbs(fname(fight()->name));
      }
    } 
    stopFighting();
  }

  if (isCombatMode(ATTACK_BERSERK))
    setCombatMode(ATTACK_NORMAL);

  if (affectedBySpell(SKILL_BERSERK))
    affectFrom(SKILL_BERSERK);

  // dead immortals mortal again
  if (GetMaxLevel() > MAX_MORT) {
    if (!isPlayerAction(PLR_IMMORTAL))
      addPlayerAction(PLR_IMMORTAL);  
  }
  if (desc)
    desc->outputProcessing();

  if (spec == SPEC_SHOPKEEPER) {
    vlogf(10, "Shopkeeper inventory being removed.");

    for (TThing *tThing = stuff; tThing; ) {
      TThing *tThingB = tThing->nextThing;

      delete tThing;
      tThing = NULL;

      tThing = tThingB;
    }

    stuff = NULL;
    setMoney(0);
  }

  makeCorpse(dmg_type);
  deathCry();

  genericKillFix();

  if (isPc()) {
    reformGroup();
    removeRent();
    removeFollowers();
  }

  // anything calling, should delete this
  preKillCheck();

  if (spec == SPEC_SHOPKEEPER && number >= 0) {
    vlogf(10, "Shopkeeper [%s] was just killed.  Find out how!", getName());

    unsigned int shop_nr;
    for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != number); shop_nr++);

    if (shop_nr >= shop_index.size()) 
      vlogf(9, "Warning... shop # for mobile %d (real nr) not found.", number);
    else {
      vlogf(9, "Deleting shopkeeper file for shop #%d.", shop_nr);
      waste_shop_file(shop_nr);
    }
  }

  return DELETE_THIS;
}


// returns DELETE_THIS is this should die
// otherwise FALSE
int TBeing::die(int dam_type)
{
  Descriptor *d;
  TRoom *rp;
  int polymorph = 0;
  rp = roomp;

  if (dynamic_cast<TMonster *>(this) && (desc || isPc())) {
    if (!(d = desc) || !d->original) {
      vlogf(10, "*BUG BRUTIUS BIG TIME* (die)");
      return FALSE;
    }
    if ((polymorph = dieReturn("", dam_type, 1)) == DELETE_THIS) {
      rawKill(dam_type);
      return DELETE_THIS;
    } else if (polymorph == FALSE) 
      return FALSE;
    else if (polymorph == 1) {  // switch
      rawKill(dam_type);
      return DELETE_THIS;
    }
  }
  affectedData *aff;
  bool skip_death = false;
  for (aff = affected; aff; aff = aff->next) {
    if (aff->type == AFFECT_FREE_DEATHS) {
      if (aff->modifier > 0 && !rp->isRoomFlag(ROOM_ARENA)) {
        aff->modifier -= 1;
        skip_death = true;
        break;
      }
    }
  }

  stats.deaths[GetMaxLevel()][!isPc()] = stats.deaths[GetMaxLevel()][!isPc()] + 1;

  if (rp) {
    if (isPc() && !rp->isRoomFlag(ROOM_ARENA) && !skip_death) {
      int val_num = ::number(0,3);
      vlogf(0,"%s died and lost %.2f exp (age +%d).  Before death: %.2f exp and %d talens.",
               getName(), deathExp(), val_num, getExp(),getMoney());
      //      vlogf(0,"Average skill loss: %f", deathSkillLoss());
      age_mod += val_num;
      gain_exp(this, -deathExp());
    }
  } else {
    vlogf(10, "Death called with ch (%s) in ROOM_NOWHERE!", getName());
    gain_exp(this, -deathExp());
  }

  // affectedData *aff;
  for (aff = affected; aff && aff->type != AFFECT_TEST_FIGHT_MOB; aff = aff->next);
  if (aff && !isPc()) {
    test_fight_death(this, dynamic_cast<TBeing *>(aff->be), aff->level);
  }

  rawKill(dam_type);

  return DELETE_THIS;
}

int TBeing::dieReturn(const char *, int dam_type, int cmd)
{
  TBeing *mob = NULL, *per = NULL;

  if (!desc || !desc->original) {
      vlogf(10, "*BUG CODERS BIG TIME* (dieReturn)");
    sendTo("There is a problem. Please contact a god and bug what you did?!\n\r");
    return FALSE;
  } else {
    sendTo("You return to your original body.\n\r");

    if ((specials.act & ACT_POLYSELF) && cmd) {
      mob = this;
      per = desc->original;

      act("$n turns liquid, and reforms as $N.", TRUE, mob, 0, per, TO_ROOM);
      --(*per);
      *mob->roomp += *per;

      SwitchStuff(mob, per);
      per->affectFrom(SPELL_POLYMORPH);
      per->affectFrom(SKILL_DISGUISE);
      per->affectFrom(SPELL_SHAPESHIFT);
    }

    desc->original->polyed = POLY_TYPE_NONE;
    desc->character = desc->original;
    desc->original = NULL;

    desc->character->desc = desc;
    desc = NULL;
  }
  if (per && (per->die(dam_type)) == DELETE_THIS) {
    per->reformGroup();
    delete per;
    per = NULL;
  }
  return DELETE_THIS;
}

bool TBeing::checkCut(TBeing *ch, wearSlotT part_hit, spellNumT wtype, TThing *weapon, int dam)
{
  byte sharp;
  char buf[256], buf3[128];
  TThing *t;
  TBeing *temp;

  temp = NULL;
  wtype -= TYPE_MIN_HIT;

  TThing * tt = equipment[part_hit];
  TObj * item = dynamic_cast<TObj *>(tt);
  if (!item) {
    vlogf(10, "Check cut called with no item! BUG BRUTIUS!!!");
    return FALSE;
  }

  if (dam <= ITEM_DAMAGE_RATE) 
    return FALSE;

  if(inPkZone())
    return FALSE;

  TBaseWeapon *w2 = NULL;
  if (weapon && (w2 = dynamic_cast<TBaseWeapon *>(weapon)))
    sharp = w2->getCurSharp();
  else
    sharp = sharpness[wtype];

//  vlogf(5, "tear: susc: %d, sharp: %d, struct: %d, dam: %d", material_nums[item->getMaterial()].cut_susc, sharp, item->getStructPoints(), dam);
  if (item->willTear(sharp)) {
//  vlogf(5, "torn");
    int chance = item->getStructPoints() / dam;
    if (!(::number(0, chance))) {
      int dmgamt = max(dam/3, 1);
      item->addToStructPoints(-dmgamt);

      if (item->getStructPoints() <= 0) {
        item->makeScraps();
        delete item;
        item = NULL;
      } else {
        if (weapon)
          strcpy(buf3, objn(weapon).c_str());

        sprintf(buf, "Your %s$o%s $q %scut%s by $N's %s!",
                blue(), norm(),
                purple(), norm(),
        (weapon ? buf3 : ch->getMyRace()->getBodyLimbSlash().c_str()));
        act(buf, FALSE, this, item, ch, TO_CHAR);

        for (t = roomp->stuff; t; t = t->nextThing) {
          temp = dynamic_cast<TBeing *>(t);
          if (!temp || (temp == this))
            continue;

          strcpy(buf, "$n's $o $q cut by ");
          sprintf(buf + strlen(buf), "%s%s ",
              (temp != ch ? temp->pers(ch) : "your"),
              (temp != ch ? "'s" : "") );
          strcat(buf, (weapon ? temp->objn(weapon).c_str() :
                 ch->getMyRace()->getBodyLimbSlash().c_str()));
          strcat(buf, "!\n\r");
          act(buf, TRUE, this, item, temp, TO_VICT);
        }
      }
      return TRUE;
    } else
      return FALSE;
  } else
    return FALSE;
}

bool TBeing::checkSmashed(TBeing *ch, wearSlotT part_hit, spellNumT wtype, TThing *weapon, int dam)
{
  int sharp;
  char buf[256], buf3[128];
  TThing *t;
  TBeing *temp;

  temp = NULL;
  TThing * tt = equipment[part_hit];
  TObj * item = dynamic_cast<TObj *>(tt);
  if (!item) {
    vlogf(10, "check_smashed called with no item!");
    return FALSE;
  }
  if (dam <= ITEM_DAMAGE_RATE) 
    return FALSE;

  if(inPkZone())
    return FALSE;

  wtype -= TYPE_MIN_HIT;

  TBaseWeapon *w2 = NULL;
  if (weapon && (w2 = dynamic_cast<TBaseWeapon *>(weapon)))
    sharp = w2->getCurSharp();
  else
    sharp = sharpness[wtype];

//    vlogf(5, "dent: susc: %d, sharp: %d, struct: %d, dam: %d", material_nums[item->getMaterial()].smash_susc, sharp, item->getStructPoints(), dam);
  if (item->willDent(sharp)) {
//  vlogf(5, "dented");
    int chance = item->getStructPoints() / dam;
    if (!(::number(0, chance))) {
      int dmgamt = max(dam/3, 1);
      item->addToStructPoints(-dmgamt);

      if (item->getStructPoints() <= 0) {
        item->makeScraps();
        delete item;
        item = NULL;
      } else {
        if (weapon)
          strcpy(buf3, fname(weapon->name).c_str());
        sprintf(buf, "Your %s$o%s $q %ssmashed%s by $N's %s!",
                blue(), norm(),
                purple(), norm(),
        (weapon ? buf3 :  ch->getMyRace()->getBodyLimbBlunt().c_str()));
        act(buf, FALSE, this, item, ch, TO_CHAR);

        for (t = roomp->stuff; t; t = t->nextThing) {
          temp = dynamic_cast<TBeing *>(t);
          if (!temp || (temp == this))
            continue;

          strcpy(buf, "$n's $o $q smashed by ");
          sprintf(buf + strlen(buf), "%s%s ",
              (temp != ch ? temp->pers(ch) : "your"),
              (temp != ch ? "'s" : "") );
          if (weapon) {
            strcat(buf, colorString(temp, temp->desc, temp->objn(weapon).c_str(), NULL, COLOR_OBJECTS, FALSE).c_str());
          } else {
            strcat(buf, ch->getMyRace()->getBodyLimbBlunt().c_str());
          }
          strcat(buf, "!\n\r");
          act(buf, TRUE, this, item, temp, TO_VICT);
        }
      }
      return TRUE;
    } else
      return FALSE;
  } else
    return FALSE;
}

bool TBeing::checkPierced(TBeing *ch, wearSlotT part_hit, spellNumT wtype, TThing *weapon, int dam)
{
  byte sharp;
  char buf[256], buf3[128];
  TThing *t;
  TBeing *temp;

  temp = NULL;
  wtype -= TYPE_MIN_HIT;

  TThing * tt = equipment[part_hit];
  TObj * item = dynamic_cast<TObj *>(tt);
  if (!item) {
    vlogf(10, "Check pierced called with no item! BUG BRUTIUS!!!");
    return FALSE;
  }
  if (dam <= ITEM_DAMAGE_RATE) 
    return FALSE;

  if(inPkZone())
    return FALSE;


  TBaseWeapon *w2 = NULL;
  if (weapon && (w2 = dynamic_cast<TBaseWeapon *>(weapon)))
    sharp = w2->getCurSharp();
  else
    sharp = sharpness[wtype];

//  vlogf(5, "pierce: susc: %d, sharp: %d, struct: %d, dam: %d", material_nums[item->getMaterial()].pierced_susc, sharp, item->getStructPoints(), dam);
  if (item->willPuncture(sharp)) {
//  vlogf(5, "puncted");
    int chance = item->getStructPoints() / dam;
    if (!(::number(0, chance))) {
      int dmgamt = max(dam/3, 1);
      item->addToStructPoints(-dmgamt);

      if (item->getStructPoints() <= 0) {
        item->makeScraps();
        delete item;
        item = NULL;
      } else {
        if (weapon)
          strcpy(buf3, fname(weapon->name).c_str());
        sprintf(buf, "Your %s$o%s $q %spierced%s by $N's %s!",
                blue(), norm(),
                purple(), norm(),
        (weapon ? buf3 :  ch->getMyRace()->getBodyLimbPierce().c_str()));
        act(buf, TRUE, this, item, ch, TO_CHAR);

        for (t = roomp->stuff; t; t = t->nextThing) {
          temp = dynamic_cast<TBeing *>(t);
          if (!temp || (temp == this))
            continue;

          strcpy(buf, "$n's $o $q pierced by ");
          sprintf(buf + strlen(buf), "%s%s ",
              (temp != ch ? temp->pers(ch) : "your"),
              (temp != ch ? "'s" : "") );
          strcat(buf, (weapon ? temp->objn(weapon).c_str() :
                 ch->getMyRace()->getBodyLimbPierce().c_str()));
          strcat(buf, "!\n\r");
          act(buf, TRUE, this, item, temp, TO_VICT);
        }
      }
      return TRUE;
    } else
      return FALSE;
  } else
    return FALSE;
}

// damageLimb will have to determine if the "limb" is a vital area, and 
// if it is, then take off max hp, otherwise damage the limb's health   
// This might get tricky, when we determine how to update a person every 
// update, and decide what damage will entail loss of hp or loss of other 
// things like consciousness. Tweak is the operative word here. - Russ  

// DELETE_VICT, v died
int TBeing::damageLimb(TBeing *v, wearSlotT part_hit, TThing *weapon, int *dam)
{
  int rc = 0;
  int hardness, sharp = 0;

  // These areas take off main hp. We still check to see if            
  // damage was done, but we return false, because main hp are taken   
  // off with hit to these vital body parts. - Russ                    

  if (isCritPart(part_hit) || !v->slotChance(part_hit))
    return FALSE;

  if (part_hit == WEAR_FINGER_L)
    part_hit = WEAR_HAND_L;

  if (part_hit == WEAR_FINGER_R)
    part_hit = WEAR_HAND_R;

  if (part_hit == HOLD_LEFT)
    part_hit = WEAR_HAND_L;

  if (part_hit == HOLD_RIGHT)
    part_hit = WEAR_HAND_R;

  TBaseWeapon *w2 = NULL;
  if (weapon && (w2 = dynamic_cast<TBaseWeapon *>(weapon)))
    sharp = w2->getCurSharp();
  else
    sharp = sharpness[getFormType() - TYPE_MIN_HIT];

 // Let's base the damage done to limb on the following things :      
 // 1) Base damage of the weapon.           (dam)                     
 // 2) Strength in limb(How healthy it is)  (body_parts[].health)     
 // 3) Constitution                         (getConShock())         
 // 4) Skin type                            (getMaterial())           
 // The healthier the limb, the less chance of this happening, and    
 // the more damage, the more chance of it happening. - Russ          

  if (setCharFighting(v, 0) == -1)
    return 0;

  setVictFighting(v, 0);

  // While it makes sense to do more damage as the limb gets bad
  // I felt it needed to be more linear on the MUD. Sometimes
  // realism gives way to playablity. So I changed it from
  // current health, to ma health of limb - Russ 04/28/96

  if (!v->hasPart(part_hit))
    return FALSE;

  if (::number(1, (v->getMaxLimbHealth(part_hit) / 4)) < *dam) {
    // Give them a chance to save based on con.shock
    if (::number(1, 101) < (v->getConShock() / 3))
      return TRUE;

    // Check to see if skin type protects them       
    hardness = material_nums[v->getMaterial()].hardness *
        v->getCurLimbHealth(part_hit) / v->getMaxLimbHealth(part_hit);
    if (hardness && (::number(1, 101) < (hardness / 2))) 
      return TRUE;

    // seeing we do the con-shock test above, don't give two benefits
    // for con.  also, this function is for something totaly diff anyway
    // bat 11/13/98
    // *dam -= (int) v->getConHpModifier();

    // this changes damage done to limb only, mhit already returned from funct
    // lessens damage based on level
    *dam *= 25 + GetMaxLevel();
    // lowering this next number INCREASES limb damage
    // 4.1 balanced at 60. 75 too high, 50 too low.    
    // 4.5 :
    *dam /= 70;

    *dam = max(0, *dam);
    // apply some main HP loss due to limb damage
    rc = applyDamage(v,*dam/5,DAMAGE_NORMAL);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      v->reformGroup();
      return DELETE_VICT;
    }

    rc = v->hurtLimb(*dam, part_hit);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;

    // if bleeding or infected, take extra dam
    if (v->isLimbFlags(part_hit, PART_BLEEDING | PART_INFECTED)) {
      rc = v->hurtLimb(*dam, part_hit);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
    } 

    // Chance to cut and start bleeding, or if already bleeding infect wound 
    if (v->isLimbFlags(part_hit, PART_BLEEDING)) {
      if (!::number(0, 8)) {
        // Infection rocks! - Russ 
        v->rawInfect(part_hit, ((*dam) * 10) + 100, SILENT_NO, CHECK_IMMUNITY_YES);
      }
    } else if (::number(0, 400) < (sharp / 2) && 
               (weapon || !v->isLucky(levelLuckModifier(5))) &&
               !v->isLucky(levelLuckModifier(5))) {
      // if the attacker is barehanded, then lets reduce chance  -- bat
      v->rawBleed(part_hit, ((*dam) * 20) + 200, SILENT_NO, CHECK_IMMUNITY_YES);
    }
  } else
    *dam = 0;

  return TRUE;
}

// damage_hand will be called when a person hits something with his/her   
// bare hand. It will be based on the material type hit, and the strength
// of the persons hand, along with its health points - Russ               

int TBeing::damageHand(TBeing *v, wearSlotT part_hit)
{
  int hardness;
  TThing *tt = v->equipment[part_hit];
  TObj *item = dynamic_cast<TObj *>(tt);

  if (item) {
    if (item->getMaxStructPoints() < 0)
      hardness = material_nums[item->getMaterial()].hardness;
    else if (item->getMaxStructPoints() != 0)
      hardness = material_nums[item->getMaterial()].hardness *
          item->getStructPoints() / item->getMaxStructPoints();
    else
      hardness = 0;
  } else {
    if (v->getMaxLimbHealth(part_hit))
      hardness = material_nums[v->getMaterial()].hardness *
          v->getCurLimbHealth(part_hit) / v->getMaxLimbHealth(part_hit);
    else
      hardness = 0;
  }
  if (::number(1, 100) < hardness) {
    if (!::number(0, getCurLimbHealth(getPrimaryHold()))) {
      if (affectedBySpell(AFFECT_TRANSFORMED_HANDS))
        sendTo(COLOR_MOBS, "You damage your claw on %s's %s.\n\r", pers(v), 
              (item ? fname(item->name).c_str() : (v->isHumanoid() ? "body" : "hide")));
      else if (affectedBySpell(AFFECT_TRANSFORMED_ARMS))
        sendTo(COLOR_MOBS, "You damage your wing on %s's %s.\n\r", pers(v),
              (item ? fname(item->name).c_str() : (v->isHumanoid() ? "body" : "hide")));
      else
        sendTo(COLOR_MOBS, "You damage your hand on %s's %s.\n\r", pers(v),
              (item ? fname(item->name).c_str() : (v->isHumanoid() ? "body" : "hide")));
      return TRUE;
    }
  }
  return FALSE;
}

// all calls to this function should delete obj afterwards
void TObj::makeScraps()
{
  TTrash *o = NULL;
  TThing *x = NULL, *tmp = NULL, *ch = NULL;
  char buf[256];
  char buf2[256];

  if (stuff) {
    if ((ch = parent)) {
      while ((x = stuff)) {
        --(*x);
        *ch += *x;
      }
    } else if ((ch = equippedBy)) {
      while ((x = stuff)) {
        --(*x);
        if (ch->roomp)
          *ch->roomp += *x;
        else
          vlogf(5,"EquippedBy without a roomp %s", ch->getName());
      }
    }
  }

  if (((ch = parent) && parent->roomp) || (ch = equippedBy) || (ch = stuckIn)) {
    if (isMineral()) {
      sprintf(buf, "$p shatters and falls to the $g.");
      sprintf(buf2, "Your $o shatters and falls to the $g.");
    } else {
      sprintf(buf, "$p falls to the $g, scrapped.");
      sprintf(buf2, "Your $o falls to the $g, scrapped.");
    } 
    act(buf, TRUE, ch, this, NULL, TO_ROOM);
    act(buf2, FALSE, ch, this, NULL, TO_CHAR, ANSI_RED);

    if (isRare())
      vlogf(2, "%s's %s just scrapped.", ch->getName(), getName());
  } else {
    if ((parent && (tmp = parent->equippedBy)) || (tmp = parent))  {
      if (isMineral()) {
        sprintf(buf, "$p shatters and is destroyed.");
        sprintf(buf2, "Your $o shatters and is destroyed.");
      } else {
        sprintf(buf, "$p is destroyed.");
        sprintf(buf2, "Your $o is destroyed.");
      }
      while (tmp) {
        if (tmp->roomp) {
          act(buf2, TRUE, tmp, this, NULL, TO_CHAR, ANSI_RED);
          act(buf, TRUE, tmp, this, NULL, TO_ROOM);
          break;
        }
        tmp = tmp->parent;
      }
    } else if (roomp) {
      if (isMineral())
        sprintf(buf, "$n shatters and is destroyed.");
      else
        sprintf(buf, "$n is destroyed.");
      act(buf, TRUE, this, NULL, NULL, TO_ROOM);
    } else 
      vlogf(5,"Something in make scraps isnt in a room %s.", getName());
  }
  
  o = new TTrash();

  if (isMineral()) {
    o->name = mud_str_dup("shattered pile");
    o->shortDescr = mud_str_dup("something shattered");
    sprintf(buf, "What used to be %s lies here, shattered.", getName());
    o->setDescr(mud_str_dup(buf));
  } else {
    o->name = mud_str_dup("scraps pile");
    o->shortDescr = mud_str_dup("a pile of scraps");
    sprintf(buf, "What used to be %s lies here, scrapped.", shortDescr);
    o->setDescr(mud_str_dup(buf));
  }
  o->obj_flags.wear_flags = obj_flags.wear_flags;
  o->setWeight(getWeight() / 2.0);
  o->obj_flags.cost = 10;
  o->canBeSeen = 2;

  TBeing *chb;
  if ((ch = parent)) {
    --(*this);
    if (ch->roomp)
      *ch->roomp += *o;
    else
      *ch += *o;
    return;
  } else if ((chb = stuckIn)) {
    chb->setStuckIn(eq_stuck, NULL);
    if (chb->roomp)
      *chb->roomp += *o;
    else 
      *chb += *o;
    return;
  } else if ((ch = equippedBy)) {
    TBeing * tbt = dynamic_cast<TBeing *>(ch);
    scrapMe(tbt);
    TThing *o2 = NULL;
    o2 = tbt->unequip(eq_pos);
    // unequip deletes bandages
    if (!o2)
      return;

    if (tbt->roomp) 
      *tbt->roomp += *o;
    else 
      *tbt += *o;
    
    return;
  }
  // at this point, "this" is guaranteed to be in roomp

  while (stuff) {
    x = stuff;
    --(*x);
    *roomp += *x;
  }
  if (roomp)
    *roomp += *o;
}

static void absorb_damage(TBeing *v, wearSlotT part_hit, int *dam)
{
  int armor;
  int abs;

  armor = v->acForPos(part_hit) / stats.absorb_damage_divisor[v->isPc() ? PC_STAT : MOB_STAT];
  abs = min(armor/10, *dam);
  stats.ac_absorb[v->isPc() ? PC_STAT : MOB_STAT] += abs;

#if DEBUG
  vlogf(3,"Debug : Damage went from %d to %d with armor absorption!", *dam, (*dam - abs));
#endif
  *dam -= abs;
}

// this = v, ch = hitter
int TBeing::damageItem(TBeing *ch, wearSlotT part_hit, spellNumT wtype, TThing *weapon, int dam)
{
  TThing *tt;
  TObj *item;

  if (!dam)
    return FALSE;

  // Don't damage equipment in arena fights.
  if (ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

  tt = equipment[part_hit];
  item = dynamic_cast<TObj *>(tt);
  if (!item) {
    vlogf(10, "damage_item got started with no item!");
    return FALSE;
  }
  if (item->getStructPoints() < 0)
    return FALSE;

  if (slashType(wtype)) {
    if (checkCut(ch, part_hit, wtype, weapon, dam))
      return TRUE;
  } else if (bluntType(wtype)) {
    if (checkSmashed(ch, part_hit, wtype, weapon, dam))
      return TRUE;
  } else if (pierceType(wtype)) {
    if (checkPierced(ch, part_hit, wtype, weapon, dam))
      return TRUE;
  }
  return FALSE;
}

// damage_weapon must check for two types of damage..... one is 
// "dulling" of the weapon, and the other is general structural 
// decay of the weapon. The sharpness, and struct points will be
// used respectfully to get at these two things - Russ          

// return DELETE_ITEM indicates weapon should be deleted
int TBeing::damageWeapon(TBeing *v, wearSlotT part_hit, TThing **weapon)
{
  if (v->roomp && v->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

  if (!*weapon) {
    damageHand(v, part_hit);
    return FALSE;
  }

  return (*weapon)->damageMe(this, v, part_hit);
}

void TBeing::setFighting(TThing *vict, int dam, bool inFight)
{
//  int bitnum = 0;
//  bool found = FALSE;
//  char buf[256];

  if (this == vict)
    return;

  TBeing *tbv = dynamic_cast<TBeing *>(vict);
  if (!tbv)
    return;

  if (fight()) {
    vlogf(5, "Fighting character (%s) set to fighting another. (%s)",
       getName(), tbv->getName());
    return;
  }

  // extreme engager
  if (inFight && desc && (IS_SET(desc->autobits, AUTO_ENGAGE_ALWAYS))) 
    SET_BIT(specials.affectedBy, AFF_ENGAGER);

  // dont add to attackers
  if (!isAffected(AFF_ENGAGER)) 
    tbv->attackers++;

  next_fighting = gCombatList;
  gCombatList = this;

  if (affectedBySpell(SPELL_SLUMBER))
    affectFrom(SPELL_SLUMBER);

  if (isAffected(AFF_SLEEP))
    REMOVE_BIT(specials.affectedBy, AFF_SLEEP);

  // the getHit check is for situation when we do setVictFighting with enough
  // damage to kill the victim.
  if (getPosition() == POSITION_SLEEPING)
    if (getHit() > dam)
      doWake("");

#if 0
// Possibly delete if mobs on furniture seems ok
  if (tbv->getPosition() < POSITION_STANDING) {
    if (dynamic_cast<TMonster *> (tbv) && !tbv->desc) {
      if (!tbv->isAffected(AFF_CHARM) && ::number(0, 3)) {
        dynamic_cast<TMonster *> (tbv)->standUp();
      }
    }
  }
#endif

  // this is special code for questmob stuff
  if (!tbv->isPc() && isPc()) {
    checkForQuestTog(tbv);
  }
  specials.fighting = tbv;

  musicNumT mus = pickRandMusic(MUSIC_COMBAT_01, MUSIC_COMBAT_03);
  playmusic(mus, MUSIC_TYPE_COMBAT);
}

void TBeing::checkForQuestTog(TBeing *vict)
{
  bool found = FALSE, found2 = FALSE;
  int bitnum = FALSE;
  affectedData af;
  affectedData *aff = NULL;
  char buf[256];

  switch (vict->mobVnum()) {
    case MOB_TROLL_GIANT:
      if (hasQuestBit(bitnum = TOG_AVENGER_HUNTING))
        found = TRUE;
      break;
    case MOB_CAPTAIN_RYOKEN:
      if (hasQuestBit(bitnum = TOG_VINDICATOR_HUNTING_1))
        found = TRUE;
      break;
    case MOB_TREE_SPIRIT:
      if (hasQuestBit(bitnum = TOG_VINDICATOR_HUNTING_2))
        found = TRUE;
      break;
    case MOB_JOHN_RUSTLER:
      if (hasQuestBit(bitnum = TOG_RANGER_FIRST_FARMHAND))
	found = TRUE;
      break;
    case MOB_ORC_MAGI:
      if (hasQuestBit(bitnum = TOG_SEEKING_ORC_MAGI) &&
	  !hasQuestBit(TOG_FAILED_TO_KILL_MAGI) &&
	  !hasQuestBit(TOG_PROVING_SELF))
	found = TRUE;
      break;
    default:
      found = FALSE;
      break;
  }
  if (found) {
    if (!vict->fight() && vict->getHit() >= vict->hitLimit()) {
      af.type = AFFECT_COMBAT;
      af.modifier = COMBAT_SOLO_KILL;
      af.level = bitnum;
      af.duration = PERMANENT_DURATION;
      af.location = APPLY_NONE;
      af.bitvector = 0;
      af.be = this;

      vict->affectTo(&af, -1);
      sprintf(buf, "%s You have acted honorably in attacking me.", getName());
      vict->doTell(buf);
      sprintf(buf, "%s Let this fight be to the death!", getName());
      vict->doTell(buf);
//      vlogf(3, "Setting quest bit for %d on %s vs %s", bitnum, vict->getName(), getName());
      // some specials stun the critter so it doesn't start fighting.
      // we need to set them fighting so that if char stuns and flees
      // it acknowledges as a no good attack.
      if (!vict->fight())
        vict->setFighting(this, 0, FALSE);
    } else {
      if (vict->affected) {
        for (aff = vict->affected; aff; aff = aff->next) {
          if ((aff->type == AFFECT_COMBAT) && (aff->modifier == COMBAT_SOLO_KILL) && (aff->level == bitnum)) {
            found2 = TRUE;
            break;
          } else {
            continue;
          }
        }
      }
      if (!found2 && awake()) {
        sprintf(buf, "%s You have acted dishonorably in attacking me while I was unprepared.", getName());
        vict->doTell(buf);
        sprintf(buf, "%s This battle will avail you naught unless you fight me at my prime.", getName());
        vict->doTell(buf);
      }
    }
  }
}

void TBeing::sendCheatMessage(char *cheater)
{
  char buf[256];
  char nameBuf[MAX_NAME_LENGTH];

  sprintf(nameBuf, "%s", cheater);
  switch (mobVnum()) {
    case MOB_TROLL_GIANT:
    case MOB_CAPTAIN_RYOKEN:
    case MOB_TREE_SPIRIT:
      sprintf(buf, "%s You have failed to defeat me in single combat.", nameBuf);
      doTell(buf);
      sprintf(buf, "%s An honorable deikhan would allow me to heal completely before attacking again.", nameBuf);
      doTell(buf);
      break;
    case MOB_JOHN_RUSTLER:
      sprintf(buf, "%s You have failed to defeat me in single combat.", nameBuf);
      doTell(buf);
      sprintf(buf, "%s An honorable ranger would allow me to heal completely before attacking again.", nameBuf);
      doTell(buf);
      break;      
    case MOB_ORC_MAGI:
      sendTo("<c>You realize you did not follow the guidelines of your quest, so this fight will be for naught.<1>\n\r");
      setQuestBit(TOG_FAILED_TO_KILL_MAGI);
      break;
    default:
      vlogf(5,"Somehow got to getCheatMessage without a valid toggle bit %s.", getName());
      break;
  }
}

// remove a char from the list of fighting chars 
void TBeing::stopFighting()
{
  TBeing *tmp;
  affectedData *af, *af2;
  char nameBuf[MAX_NAME_LENGTH];

  stopmusic();

  if (!fight()) {
    REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);
    //vlogf(4, "Character (%s) not fighting at invocation of stopFighting", getName());
    forceCrash("Character (%s) not fighting at invocation of stopFighting", getName());
    if (desc && (cantHit > 0)) {
      sendTo("You finish orienting yourself.\n\r");
      cantHit = 0;
      addToWait(combatRound(1));
    }
    return;
  }
  if (fight() == this) {
    vlogf(3, "Character fighting self in stopFighting()");
    return;
  }
  // Clear out client fight and tank condition fields
  if (desc && desc->client) 
    desc->clientf("%d|%s|%s|%s|%s", CLIENT_FIGHT, "", "", "", "");

  // see explanation at declaration for details on this
  if (gCombatNext == this)
    gCombatNext = next_fighting;

  // special code for quest mobs
  for (af = affected; af; af = af2) {
    af2 = af->next;
    if (af->type == AFFECT_COMBAT && af->modifier == COMBAT_SOLO_KILL) {
      // stopFighting is called from a bunch of places, including mob death
      // we have trapped death, so anything else that gets here implies
      // combat stopped for some other reason
      // ie, mob fled, got paralyzed, teleported, etc.
      if (awake()) {
        vlogf(3, "Removing Solo Combat Affect from: %s", getName());
        sprintf(nameBuf, "%s", fname(af->be->name).c_str());
        sendCheatMessage(nameBuf);
        affectRemove(af);
      }
#if 0
    } else if (af->type == AFFECT_TEST_FIGHT_MOB) {
      test_fight_death(this, dynamic_cast<TBeing *>(af->be), af->level);
#endif
    }
  }

  if (!(isAffected(AFF_ENGAGER)))
    fight()->attackers--;

  // attackers is unsigned, so this checks for underflow
  if (fight()->attackers > 100) {
    vlogf(3, "too few people attacking. %d", fight()->attackers);
    fight()->attackers = 0;
  }

  REMOVE_BIT(specials.affectedBy, AFF_AGGRESSOR);
  REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);

  if (gCombatList == this)
    gCombatList = next_fighting;
  else {
    for (tmp = gCombatList; tmp && (tmp->next_fighting != this);
         tmp = tmp->next_fighting);
    if (!tmp) {
      vlogf(10, "Char fighting not found Error (combat.cc, stopGighting)");
      abort();
    }
    tmp->next_fighting = next_fighting;
  }
  if (desc && (cantHit > 0)) {
    sendTo("You finish orienting yourself.\n\r");
    cantHit = 0;
    addToWait(combatRound(1));
  }
  // store this for later
  tmp = fight();

  next_fighting = NULL;
  specials.fighting = NULL;

  updatePos();

  // don't let them bezerk on who we just stopped fighting with
  // stopfighting is called by delete so doing otherwise might cause snafu
  goBerserk(tmp);
}

bool TBeing::checkPeaceful(const string &msg) const
{
  if (roomp && roomp->isRoomFlag(ROOM_PEACEFUL)) {
    sendTo(msg.c_str());
    return TRUE;
  }
  if (affectedBySpell(SPELL_CALM)) {
    sendTo("You've got a peaceful easy feeling.\n\r");
    return TRUE;
  }
  return FALSE;
}

bool TBeing::checkPeacefulVictim(const string &msg, const TThing *v) const
{
  if (v->roomp && v->roomp->isRoomFlag(ROOM_PEACEFUL)) {
    sendTo(msg.c_str());
    return TRUE;
  }
  return FALSE;
}


int TBeing::extraDam(const TBeing *vict, const TBaseWeapon *weap) const
{
  int plus = 0;

  if (!vict || !weap)
    return plus;

  if (vict->isUndead() || vict->isLycanthrope()) {
    if (weap->isObjStat(ITEM_BLESS))
      plus += 1;
    if (weap->getMaterial() == MAT_SILVER) 
      plus += 1;
  }
  if (vict->isDiabolic()) {
    if (weap->isObjStat(ITEM_BLESS))
      plus += 1;
  }
  return plus;
}

static int getMonkWeaponDam(const TBeing *ch, const TBeing *v, bool isprimary, int rollDam)
{
  int wepDam;

  // from balance notes, we want monk barehand damage per hit to be roughly
  // 56/15 * sqrt(level)
  // we want to be pulling this stuff from the kubo skill which
  // we roughly assume goes up linearly from L1-L30 : n = 10/3 * L
  // similarly, kubo specializatio (meditation disc) covers L31-L40 linearly
  if (!ch->doesKnowSkill(SKILL_KUBO))
    wepDam = ::number(1,2);
  else {
    // somewhat a clumsy way to do this, but realize that player could be
    // partially learned in KUBO but have some learning in the advanced spec.
    double value = 3.0 * ch->getSkillValue(SKILL_KUBO) / 10.0;

    // in general, we ought to be adding 20 levs for max specialization
    // however, review the balance docs and realize we add a penalty
    // here (the 76% factor) in order to counter the effects of blur.
    double amt = ch->getAdvLearning(SKILL_KUBO);
    if (amt)
      value += (0.7676 * amt)/5.0;

    // enforce range
    value = min(max(value, 0.0), 50.0);

    // essentially, we can treat "value" as being effectively equivalent to
    // level at this point.  Realize it is made up by looking at how learned
    // they are in skills rather than simply doing a GetMaxLevel though.

    // convert level to dam
    float weapDam = (7.0 * sqrt(value) / 2.0);

    // make balance adjustment
    weapDam *= balanceCorrectionForLevel(ch->GetMaxLevel());
    wepDam = (int) weapDam;

    // small randomization around value
    int flux = wepDam/10;
    wepDam = ::number(wepDam-flux, wepDam+flux);

    wepDam = max(1, wepDam);
  }

  // OK, so if they are incap'd, make the kill go quickly...
  if (v->getPosition() <= POSITION_INCAP)
    wepDam += max((3 * wepDam / 10), 1);

  if (v->getPosition() <= POSITION_DEAD)
    return (0);

  // account for stats
  float statDam = (ch->getStrDamModifier() + ch->getDexDamModifier()) / 2;
  int dam = (int) (wepDam * statDam);

  // add bonuses
  dam += rollDam;

  // adjust for global values
  dam = (int) (dam * stats.damage_modifier);

#if DAMAGE_DEBUG
  vlogf(5,"MONK %s (%d %s) barehand dam = %d , wep = %d roll = %d, stats = %.2f", ch->getName(), ch->GetMaxLevel(), ch->getProfName(), dam, wepDam, rollDam, statDam);
#endif

  return (dam);
}

// Quickie commentary on damage:
// MObs: we want to be doing 1/1.1 = 0.9090 dam per level per round
// there bare hand dam + dam modifier * num attacks should be
// constructed to do this already
// so just make sure that any bonus they get is accounted for with
// an appropriate change to the base line
// PCs: we want to be averaging about 1.75 dam per level per round
// furthermore, PCs get about 1.0 attacks per round until they specialize
// SO, make the per hit dam = 1.75 for PCs, but DO NOT give extra dam
// for specializing (more hits will yield more damage on its own)
// Please note that these numbers are NOT arbitrary, but are part of the
// balance discussion, so do not change them lightly !!!!
// -Batopr 12/12/98
int TBeing::getWeaponDam(const TBeing *v, const TThing *wielded, bool isprimary) const
{
  int bonusDam = 0;
  int rollDam = 0;
  int wepLearn = 0;
  float tempDam = 0.0;
  float statDam = 0.0;
  int dam = 0;
  spellNumT skill = SKILL_BAREHAND_PROF;
  char buf[MAX_NAME_LENGTH];

  rollDam += getDamroll();
  strcpy(buf, "Barehand");
  
  // strip off affects of dual wields
  if (isprimary) {
    TObj *obj = dynamic_cast<TObj *>(heldInSecHand());
    if (obj && !obj->isPaired())
      rollDam -= obj->itemDamroll();
  } else {
    TObj *obj = dynamic_cast<TObj *>(heldInPrimHand());
    if (obj && !obj->isPaired())
      rollDam -= obj->itemDamroll();
  }

  int wepDam = 0;
  const TMonster *tmon = dynamic_cast<const TMonster *>(this);
  if (!wielded) {
    if (tmon)
      wepDam += tmon->getMobDamage();
    else if (hasClass(CLASS_MONK))
      return getMonkWeaponDam(this, v, isprimary, rollDam);
    else
      wepDam += ::number(1, 3);        
  } else {                       // swinging some odd item 
    if (tmon) {
      // NPC wielding
      // mob damage is proper for NPC hitting a PC  (0.9 * level)
      // weapon damage is for PC hitting an NPC (1.75 * level)
      // we want the weapon to have some benefit for the mob
      // somewhat counterintuitive, but have damage while wielding be
      // NPC damage + 1/10 weapon damage
      wepDam += tmon->getMobDamage();
      if (!tmon->master || !tmon->master->isPc())
        wepDam += (int) (wielded->swungObjectDamage(this, v) / 10);
    } else {
      // PC wielding
      wepDam += wielded->swungObjectDamage(this, v);
    }
  }

  // fighting from ground is an adjustment to hit, but not an
  // adjustment to damage taken - Bat
  // However, if they are incap'd, make the kill go quickly...
  if (v->getPosition() <= POSITION_INCAP)
    wepDam += max((3 * wepDam / 10), 1);

  if (v->getPosition() <= POSITION_DEAD)
    return (0);

  if (!dynamic_cast<const TMonster *>(this)) {
    if (!wielded) {
      skill = SKILL_BAREHAND_PROF;
      // skill2 = SKILL_BAREHAND_SPEC;
      strcpy(buf, "Barehand");
      statDam = (getStrDamModifier() + getDexDamModifier()) / 2;
    } else if (wielded->isBluntWeapon()) {
      skill = SKILL_BLUNT_PROF;
      // skill2 = SKILL_BLUNT_SPEC;
      strcpy(buf, "Blunt");
      statDam = getStrDamModifier();
    } else if (wielded->isPierceWeapon()) {
      skill = SKILL_PIERCE_PROF;
      // skill2 = SKILL_PIERCE_SPEC;
      strcpy(buf, "Pierce");
      statDam = getStrDamModifier() + (2 * getDexDamModifier());
      statDam /= 3;
    } else if (wielded->isSlashWeapon()) {
      skill = SKILL_SLASH_PROF;
      // skill2 = SKILL_SLASH_SPEC;
      strcpy(buf, "Slash");
      statDam = getStrDamModifier() + getDexDamModifier();
      statDam /= 2;
    }
    wepLearn = max((GetMaxLevel() *2), (int) getSkillValue(skill));
    wepLearn = min(100, wepLearn);

    // Commentary.
    // we want 1.75 dam/level/round here and assume 1 hit per round
    // wepDam should be returning appropriate amount for the level of the
    // weapon

    // we are roughly trying to plot this formula
    // (21 + 5*(wepDam-2) + roll) * stat * wepLearn/100
    
    float weapDam = wepDam;
    weapDam += rollDam;
    weapDam += bonusDam;

    // stats adjustment
    // for average stats, statDam is 1.0
    weapDam = (int) (weapDam * statDam);

    // take proficiency into account
    // this will cut into the actual damage newbies do, but should still be ok
    // rule of thumb, wepLearn will be maxed around L25
    wepLearn = max(40, wepLearn);
    weapDam *= min(wepLearn, 4*GetMaxLevel());
    weapDam /= 100;

#if 0
// Whoa, don't make specialize do more dam, it adds attacks so would
// double count...
    // take specialize into account
    // weapDam is about 25, want to add about 10 for L25 to L35
    weapDam += .004 * max((byte) 0, getSkillValue(skill2)) * weapDam;
#endif

    dam = (int) weapDam;
    tempDam = weapDam - dam;
    if (::number(0,100) < (tempDam * 100))
      dam += 1;

    // c.f. BALANCE NOTES for discussion of why this is done
    // basically, we don't want dual-wielding to be on par with single-wielding
    if (!isprimary) {
      int amt = max(0, (int) getSkillValue(getSkillNum(SKILL_DUAL_WIELD)));
      amt = (int) (amt * 3 / 5 + 10);
      dam = (int) (dam * amt / 100);
    }

    dam = max(1, dam);

#if DAMAGE_DEBUG
    vlogf(5,"PLAYER %s (%d %s) %s dam = %d , wep = %d bon = %d roll = %d, stats = %.2f skill = %d", getName(), GetMaxLevel(), getProfName(), buf, dam, wepDam, bonusDam, rollDam, statDam, wepLearn);
#endif

  } else {
    // isMonster is true
    // technically, we ought to allot for stats too, but this makes things
    // more complex, and our system hasn't properly been set up to balance
    // and then use mob stats as adjustments, so just ignore stats

    wepDam = max(wepDam, 1);
    dam = wepDam + bonusDam + rollDam;
#if DAMAGE_DEBUG
    vlogf(5,"MOB %s (%d %s) dam = %d , wep = %d bon = %d roll = %d.", getName(), GetMaxLevel(), getProfName(), dam, wepDam, bonusDam, rollDam);
#endif

  }

  // adjust for global values
  dam = (int) (dam * stats.damage_modifier);

  dam = max(1, dam);                // Not less than 0 damage 
  return (dam);
}

static void checkLearnFromHit(TBeing * ch, int tarLevel, TThing * o, bool isPrimary, spellNumT w_type)
{
  int myLevel = ch->GetMaxLevel();

  if (ch->desc && !dynamic_cast<TMonster *>(ch))  {
    if (ch->cantHit <= 0) { 
      if (((myLevel - tarLevel) < (10 + (myLevel / 10))) &&
          (!o || dynamic_cast<TBaseWeapon *>(o))) { 
        ch->learnFromDoingUnusual(LEARN_UNUSUAL_PROFICIENCY, (o?w_type:TYPE_HIT), 
                   max(0, 25 - myLevel));

	spellNumT skill = ch->getSkillNum(SKILL_DUAL_WIELD);
        if (ch->doesKnowSkill(skill) && 
            dynamic_cast<TBaseWeapon *>(o) && 
            !isPrimary)
	  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, skill, max(0, (100 - (2* myLevel))));

        if (ch->hasClass(CLASS_WARRIOR))
	  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_OFFENSE, (170 - (2* myLevel)));
	else
	  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_OFFENSE, (225 - (2* myLevel)));
      }
    }
  }

}

// DELETE_VICT: v dead, delete
// DELETE_THIS: this dead, delete

// pulse is somewhat a holdover from roundless combat
// pulse should be -1 if we are trying to start a fight
// otherwise, pulse represents the high level value for the pulse loop.
// what we basically should do is spread the number of hits over a
// certain number of pulses (called a round).
int TBeing::hit(TBeing *target, int pulse)
{
  TThing *o = NULL;
  TThing *o2 = NULL;
  int rc = 0;
  float fx, fy;
  int i;

  // this function is called by perform_violence and other functions
  // that start a fight or call combat each round
  int len_rnd = (PULSE_COMBAT / (TurboMode ? 2 : 1));

  // This will eventually be replaced by limb code.  At least some of it.
  // Instead of arbitrarily choosing how many attacks, it will cycle through
  // valid limbs that I can attack with.  Multiple attacks for speed and
  // specialization will be applied later, maybe per limb.
  blowCount(false, fx, fy);

  if (isPc() && fx <= 0.0 && fy <= 0.0 &&
    !isAffected(AFF_ENGAGER)) {
    // we get here EVERY call (pulse) so will just flood player
    // so limit message some
    if (!(pulse % (len_rnd/2)))
      sendTo("You need to remove something in order to actually hit.\n\r");

    return FALSE;
  }

  if (pulse >= 0 && !(pulse % len_rnd)) {
    // this is once-per round stuff
    if (desc)
      desc->session.rounds[getCombatMode()]++;
    if (target->desc)
      target->desc->session.rounds_received[target->getCombatMode()]++;

    if (getMyRace()->hasTalent(TALENT_FAST_REGEN)) {
      // mostly for trolls
      if (getHit() < hitLimit()) {
        act("You regenerate slightly.", TRUE, this, 0, 0, TO_CHAR);
        act("$n regenerates slightly.", TRUE, this, 0, 0, TO_ROOM);
        addToHit(::number(1,3));
      }
    }
    if (!isPc()) {
      TMonster *tmons = dynamic_cast<TMonster *>(this);
      if (!tmons->isAffected(AFF_CHARM)) {
        tmons->developHatred(target);

        // if we are fighting an NPC pet, develop hatred toward the master
        // however, only do this is the pet was the aggressor (PC ordere pet
        // to attack), to avoid guards "get thee back to.." from hating
        // elemental's owner.  Only hate if the pet was the aggressor, or
        // if the pet's owner is also fighting
        if (!target->isPc() && target->isAffected(AFF_CHARM)) {
          if (target->master && (target->master->isPc() || target->master->desc)) {
            if (target->isAffected(AFF_AGGRESSOR) ||
                target->master->fight() == tmons) {
              tmons->developHatred(target->master);
            }
          }
        }
      }
    } 
  } 

  // we come in here multiple times
  // 1 round is PULSE_COMBAT long
  // we should get fx hits per round, and fy hits per round with offhand
  // we ought to be able to simply calculate if a hit should occur here
  // and if so, pass fx/fy as either 0.0 or 1.0 without concerning ourselves
  // with rest of function

  if (pulse < 0) {
    // this represents a call from like doHit()
    // take swing with stronger arm
    if (!target->fight()) {
      if (fx >= fy) {
        fx = min(fx + fy, (float) 1.0);
        fy = 0;
      } else {
        fy = min(fx + fy, (float) 1.0);
        fx = 0;
      }
    }
  } else {
    // handler for engage.
    if (isAffected(AFF_ENGAGER)) {
      // we basically want to give them a "dummy" attack for each hand if
      // engaged.  They don't actually get a hit with it, but it is used 
      // in the below case to figure out when they would have hit, and then
      // the handler below that works off the combat lag.
      if (fx <= 0)
        fx = 1.0;
      if (fy <= 0)
        fy = 1.0;
    }

    // unable to use doubles into modulus, so multiply by 10 and convert to int
    int hit_wait;
    if (fx) {
      hit_wait = (int) (len_rnd * 10 / fx);
      fx = ((pulse*10 % hit_wait) < 10) ? 1.0 : 0.0;
    }
    // off hand is a bit non-intuitive,
    // basically, if we are getting smae number of hits with both hands
    // stagger the blows.  Basically, what this means is add 1/2 the
    // time to do a hit before doing the true/false thing.
    // this is a little silly since a fully maxed player has 2 prim hits and
    // 1 offhand hit, meaning we still wind up synching off and on-hand hits,
    // but is still good nonetheless
    if (fy) {
      hit_wait = (int) (len_rnd * 10 / fy);
      fy = ((((pulse*10) + (hit_wait/2)) % hit_wait) < 10) ? 1.0 : 0.0;
    }
    if (!fx && !fy)
      return FALSE;
  }

  // If I'm engaged, I work off battle lag.
  if (isAffected(AFF_ENGAGER)) {
    float hitNum = fx + fy;
    for(i = 1; i <= hitNum; i++) {
      if (checkBusy(NULL)) {
        cantHit--;
        if (cantHit > 0)
          cantHit--;
      }
    }
    return FALSE;
  }

  loseSneak();

  // If I'm tasking, tell the task to stop and why.  I lose this round.
  if (task) {
    // notify the task
    (tasks[task->task].taskf) (this, CMD_TASK_FIGHTING, "", 0, NULL, NULL);
    return FALSE;
  }

  // check for various conditions where a person can't hit for the round.
  rc = canFight(target);

  if (IS_SET_DELETE(rc, DELETE_THIS)) 
    return DELETE_THIS;
  else if (!rc)
    return FALSE;

  // monk stuff
  // this is accounted for in balance discussion with a break-even point
  // of 5% rate for blur.
  if (pulse >= 0 && !(pulse % len_rnd) &&
     !heldInPrimHand() && !heldInSecHand() &&
     doesKnowSkill(SKILL_BLUR) && 
     ::number(0, 100) < 5 &&  // this makes it happen 5% of the time
     !isAffected(AFF_ENGAGER) && getMana()>=25 &&
     bSuccess(this, getSkillValue(SKILL_BLUR), SKILL_BLUR)) {
    // the number of extra swings use to be skill dependant too, but
    // this was getting too complex and is not how the balance document
    // devised things to occur.  Let's just double the normal number of
    // swings.
    // recall that we are being called numerous times per "round"
    // we only want to check for blur once per round though
    // so we use the "pulse ==" thing
    // and if we pass, then add a full round worth
    // this effectively doubles the attacks for that round
    float fx2, fy2;
    blowCount(false, fx2, fy2);

    fx += fx2;
    fy += fy2;

    sendTo(COLOR_BASIC, "<b>You focus your mind and body and execute a <C>blur<1><b> of attacks!<1>\n\r");
    reconcileMana(TYPE_UNDEFINED, 0, 25);
  }
    
  // This unequipping stuff should go into a specialized version of this method
  // for mobs.
  if (heldInPrimHand() && !dynamic_cast<TBaseWeapon *>(heldInPrimHand())) {
    if (!isPc()) {
      doSay("Hmm, seems my hands are full!");
      act("$n quickly removes $p so $e can fight!", TRUE, this, heldInPrimHand(), NULL, TO_ROOM);
      *this += *unequip(getPrimaryHold());
    }
    // soak up attack their primary hand would have had
    if (cantHit > 0) 
      cantHit--;
    
    fx = (float) 0.0;
  } 
  TObj *tobj = dynamic_cast<TObj *>(heldInSecHand());
  TBaseClothing *tbc = dynamic_cast<TBaseClothing *>(tobj);
  TBaseWeapon *tbw = dynamic_cast<TBaseWeapon *>(tobj);
  if (tobj &&
      !tobj->isPaired() && 
      (!tbc || !tbc->isShield()) &&
      !tbw) {
    if (!isPc()) {
      doSay("Hmm, seems my hands are full!");
      act("$n quickly removes $p so $e can fight!", TRUE, this, heldInSecHand(), NULL, TO_ROOM);
      *this += *unequip(getSecondaryHold());
    }
    // soak up attack their secondary hand would have had
    if (cantHit > 0) 
      cantHit--;
    
    fy = (float) 0.0;
  } 

  // soak up one can't hit for a shield hit
  tbc = dynamic_cast<TBaseClothing *>(heldInSecHand());
  if (tbc && tbc->isShield()) {
    if (cantHit > 0)
      cantHit--;
  }

  // The meat of the combat round code.  This is actually where we'd be
  // running through usable limbs and then calling speed functions within.

  // Build up attack and defense bonuses for this round.
  int offense = attackRound(target);
  int defense = target->defendRound(this);
  int mod = offense - defense;
  int tarLevel = target->GetMaxLevel();
#if DAMAGE_DEBUG
  int myLevel = GetMaxLevel();
  if ((desc || target->desc) && isImmortal())
    vlogf(4, "hitter = %s (level %d) targ = %s (level = %d) mod = %d, offense = %d, defense = %d", getName(), myLevel, target->getName(), tarLevel, mod, offense, defense);
#endif

  o = heldInPrimHand();
  o2 = heldInSecHand();
  spellNumT w_type = getAttackType(o);

  //// cosmo start learn from doing primary hand-- secondary below
  if (desc && !dynamic_cast<TMonster *>(this)) {
    if (((fx > 0.999) || (fy > 0.999)) && hasClass(CLASS_MONK) && (!o || !o2)){
      if(doesKnowSkill(SKILL_OOMLAT))
        learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_OOMLAT, 20);
      if(doesKnowSkill(SKILL_KUBO))
        learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_KUBO, 20);
      if(doesKnowSkill(SKILL_CINTAI))
        learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_CINTAI, 20);
      if(doesKnowSkill(SKILL_ADVANCED_KICKING))
        learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_ADVANCED_KICKING, 20);
#if 0
      if(doesKnowSkill(SKILL_BLUR))
        learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_BLUR, 20);
#endif
      if(doesKnowSkill(SKILL_CRIT_HIT))
        learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_CRIT_HIT, 20);
    }
    if (awake() && getPosition() < POSITION_CRAWLING && (fx > 0 || fy > 0)) {
      if (doesKnowSkill(SKILL_GROUNDFIGHTING)) {
        if (bSuccess(this, getSkillValue(SKILL_GROUNDFIGHTING), SKILL_GROUNDFIGHTING))
          ; // do nothing, just for sake of learning
      }
    }
    if (doesKnowSkill(SKILL_CHIVALRY) && (getPosition() == POSITION_MOUNTED) &&
        (fx > 0 || fy > 0)) {
      if (bSuccess(this, getSkillValue(SKILL_CHIVALRY), SKILL_CHIVALRY))
        ; // do nothing, just for sake of learning
    }
  }
  /////

  while (fx > 0.999) {
    checkLearnFromHit(this, tarLevel, o, true, w_type);
    if ((rc = oneHit(target, TRUE, o,mod, fx))) {
      if (IS_SET_ONLY(rc, DELETE_ITEM)) {
        delete o;
        o = NULL;
        REM_DELETE(rc, DELETE_ITEM);
        return rc;
      }
      return rc;
    }
    fx -= 1.0;
  }

  if (::number(1,100) <= ((int) (100.0 * fx))) {
    if (o && !dynamic_cast<TBaseWeapon *>(o))
      return FALSE;  // lose the partial attack
    checkLearnFromHit(this, tarLevel, o, true, w_type);
    if ((rc = oneHit(target, TRUE, o,mod, fx))) {
      if (IS_SET_ONLY(rc, DELETE_ITEM)) {
        delete o;
        o = NULL;
        REM_DELETE(rc, DELETE_ITEM);
        return rc;
      }
      return rc;
    }
  }
// learn from doing off hand.
  w_type = getAttackType(o2);

  while (fy > 0.999) {
    checkLearnFromHit(this, tarLevel, o2, false, w_type);
    if ((rc = oneHit(target, FALSE, o2,mod, fy))) {
      if (IS_SET_ONLY(rc, DELETE_ITEM)) {
        delete o2;
        o2 = NULL;
        REM_DELETE(rc, DELETE_ITEM);
        return rc;
      }
      return rc;
    }
    fy -= 1.0;
  }
  if (::number(1,100) <= ((int) (100.0 * fy))) {
    if (o2 && !dynamic_cast<TBaseWeapon *>(o2))
      return FALSE;  // lose the partial attack

    checkLearnFromHit(this, tarLevel, o2, false, w_type);
    if ((rc = oneHit(target, FALSE, o2,mod, fy))) {
      if (IS_SET_ONLY(rc, DELETE_ITEM)) {
        delete o2;
        o2 = NULL;
        REM_DELETE(rc, DELETE_ITEM);
        return rc;
      }
      return rc;
    }
  }
  return FALSE;
}

int TBeing::tooTired()
{
  if (getMove() < 1) {
    sendTo(COLOR_BASIC, "<r>PANIC!<z> You are so exhausted, you cannot attack!\n\r");
    act("$n wheezes and gasps as $e tries to summon the energy to fight!", TRUE, this, NULL, NULL, TO_ROOM);
    return TRUE;
  }
  return FALSE;
}

// don't do bSuccess inside here, or will learn from being stat'd etc
// move any such bSuccess check to a spot in hit() set aside for such things
int TBeing::attackRound(const TBeing * target) const
{
  // OK, start from scratch
  // We want to return a value from 0 - 1000
  // that roughly follows a level based linear format
  // Thus, L60 returns 1000, L0 = 0
  
  // correction: 1000 will correspond to L60, but we will extend the scale
  // up to L70 equivalent = 1167 (70/60*1000)
  
  int bonus = 0;
  int lvl = GetMaxLevel();

  //bonus += lvl * 1000/60;
  bonus += lvl * 50/3;

  // 1 level is 50/3 points of bonus
  // my_lev will basically hold the number of points over my lev which
  // a mob of twice the difficulty would have.  This is a useful number
  // to scale things by.
  int my_lev = max(10, (int) (16.67 * get_doubling_level(GetMaxLevel())));

  // Combat mode modifiers.
  if (desc) {
    // offensive penalty for defense should be greater than defensive reward 
    if (isCombatMode(ATTACK_DEFENSE))
      bonus -= my_lev/2;

    if (isCombatMode(ATTACK_OFFENSE) || isCombatMode(ATTACK_BERSERK))
      bonus += my_lev/4;

    // we intentionally DO NOT give an extra offense benefit for berserk
    // they get other benefits (more hits, etc) so not needed
  } 

  if (doesKnowSkill(SKILL_CHIVALRY) && getPosition() == POSITION_MOUNTED) {
    // C.F. balance for the reasons on these numbers
    // we want to yield a hitrate that is 2/9 higher for fully learned
    // deikhan.  Assuming base hit rate of 60%, that means yielding
    // 13.3% more hits at 3% per level that translates into 4.4444
    // levels.  At a rate of 50/3 points per level, we should add 74.0741
    int amt = 74;
    amt *= max(10, (int) getSkillValue(SKILL_CHIVALRY));
    amt /= MAX_SKILL_LEARNEDNESS;
    bonus += amt;
  }

  // this seems unfair but since we were doing it before, do it again...
  if (hasClass(CLASS_MONK))
    bonus += my_lev/10;

  //  Add CINTAI's benefit
  if (doesKnowSkill(SKILL_CINTAI)) {
    int amt = my_lev/6;
    amt *= max(10, (int) getSkillValue(SKILL_CINTAI));
    amt /= 100;
    bonus += max(1, amt);
  }

  // offense skill should play decent role
  // mobs get combat (not offense)

  // don't let this skew the return: it goes up roughly 4*lev
  // lower it by the theoretical value
  int amt = my_lev;
  amt *= min(100, 4*GetMaxLevel());
  amt /= 100;
  bonus -= amt;

  // raise it by the actual learning
  if (isPc()) {
    if (doesKnowSkill(SKILL_OFFENSE)) {
      amt = my_lev;
      amt *= max(10, (int) getSkillValue(SKILL_OFFENSE));
      amt /= 100;
      bonus += amt;
    }
  } else {
    CDiscipline * cdisc = getDiscipline(DISC_COMBAT);
    if (cdisc) {
      amt = my_lev;
      amt *= max(10, (int) cdisc->getLearnedness());
      amt /= 100;
      bonus += amt;
    }
  }
  
  // treat DEX here as a modifier for +hitroll
  // From BALANCE: we want high DEX to yield 5/4 more hits
  // and low dex to yield 4/5 the hits
  // so assuming that "normal" is a 60% rate, high should be 75%, and low 48%
  // 1 lev = 16.67 pts of bonus = 3%
  // 1 pt of bonus = 0.18%
  // 75% rate would be extra 15% would be 83.3 pts
  // 48% rate would be loss of 12% would be 66.67 pts 
  bonus += (int) plotStat(STAT_CURRENT, STAT_DEX, -67, 84, 0);

  // thaco adjustment
  // +10 hitroll should let me fight evenly with L+1 mob
  // a 1 lev diff is 1000/60 = 50/3 points
  // so each point of thaco should grant 5/3 to bonus
  bonus += 5 * (getHitroll() + getSpellHitroll())/3;

  // Check if you can see your target. (penalty)
  if (target && !canSee(target)){
    int amt = my_lev;
    if(doesKnowSkill(SKILL_BLINDFIGHTING)) {
      amt *= 100 - getSkillValue(SKILL_BLINDFIGHTING);
      amt /= 100;
    }
    bonus -= amt + 1;
  }

  // if casting, penalize
  // obviously, this probably affects mobs more than PCs (engaged)
  if (spelltask)
    bonus -= 2 * my_lev / 3;

  // positional modifiers
  // penalize for attacking on ground
  // note, we do not give a benefit for attacking a victim on ground here, we
  // do that in defend
  int val = 0;
  switch (getPosition()) {
    case POSITION_DEAD:
    case POSITION_MORTALLYW:
    case POSITION_INCAP:
    case POSITION_STUNNED:
    case POSITION_SLEEPING:
      val = -bonus;  // attack while asleep?
      break;
    case POSITION_RESTING:
      val = -(my_lev/3 +1);
      break;
    case POSITION_SITTING:
      val = -(my_lev/4 +1);
      break;
    case POSITION_ENGAGED:
    case POSITION_FIGHTING:
    case POSITION_CRAWLING:
    case POSITION_STANDING:
      break;
    case POSITION_MOUNTED:
      val = my_lev/4 +1;
      break;
    case POSITION_FLYING:
      val = my_lev/3 +1;
      break;
  }
  if (getPosition() < POSITION_STANDING &&
      awake() &&
      val < 0) {
    if(doesKnowSkill(SKILL_GROUNDFIGHTING)) {
      int gsv = getSkillValue(SKILL_GROUNDFIGHTING);
      val = val * (100 - gsv) / MAX_SKILL_LEARNEDNESS;
      val = min(val, -1);
    }
  }
  bonus += val;

  // we use to min/max this between 0-1200
  // I feel that if your modifiers make you REALLy suck, or really rock
  // you deserve to keep the bonus
  return bonus;
}

int TBeing::defendRound(const TBeing * attacker) const
{
  // OK, start from scratch
  // we want to return a value that is roughly linear with level and
  // somewhat corresponds to how attackRound is set up
  // L0 should return 0, and L60 should return 1000

  // However, we should base this on ARMOR, not LEVEL
  // fortunately, armor follows linearly with level  :)
  // PCs: have 50 + 2.5 * level
  // MOBs : have 60 + 2 * level
  int bonus = 0;

  // this is predominantly used later on, but calculate it here
  // just some comments on appropriate values
  // 1 level is 16.67 (1000/60) points of bonus
  // my_lev will basically hold the number of points over my lev which
  // a mob of twice the difficulty would have.  This is a useful number
  // to scale things by.
  int my_lev = max(10, (int) (16.67 * get_doubling_level(GetMaxLevel())));
 
  // this is mostly just AC
  // some classes have some situational effects here (mounted deikhans, etc)
  int armor = 1000 - getArmor();

  if (isPc()) {
    // PCS at L0 have 500 AC : return 0
    // PCs at L50 have 1750 AC : return 833
    // PCs at L60 have 2000 AC : return 1000

    // 1500 points of AC = return 1000
    // bonus = (armor-500) * 1000/1500

    // c.f. the balance note for the rational on this formula
    // we are giving 2/5 more AC for the skill to balance the lack
    // of a shield penalty
    // this is NOT the way a skill should in general modify this function
    // and is only appropriate for this case.
    if (doesKnowSkill(SKILL_OOMLAT))
      armor += armor * getSkillValue(SKILL_OOMLAT) / 250; 

    bonus = max((armor - 500), 0) * 2/3;

    // this is an arbitrary, and arguably, unfair penalty on PCs
    // basically, place a cap on the PCs AC.  The rationale for this
    // is that "powerleveling" now occurs by giving a newbie enough cash
    // to rent high level gear.
    // theoretically, the bonus calculated should be 1000 * level/60
    // we calculated previously my_lev, which is the number of extra points
    // over my level that is 2X more powerful, so set that as an arbitrary
    // ceiling
    // it _might_ be better to do this via some sort of skill (armor
    // proficiency?).  IMO, I don't feel that this ceiling will be noticed
    // except by powerlevelers, which makes it a poor candidate for a "skill"
    // Bat 7/7/99
    bonus = min(bonus, (GetMaxLevel() * 1000 / 60) + my_lev);
  } else {
    // mobs get 400 + 20*level AC
    // a mob at L0 has 400 points of AC : return 0
    // a mob at L60 has 1600 points of AC : return 1000
    // bonus = (armor-400)*1000/1200
    bonus = max((armor-400), 0)*5/6;
  }

  // bonus is now normalized for PC's and Mobs based roughly on level

  // Combat mode modifiers.
  if (desc) {
    if (isCombatMode(ATTACK_DEFENSE))
      bonus += my_lev/4;

    // defensive penalty for offense-mode should be > offensive reward 
    if (isCombatMode(ATTACK_OFFENSE) || isCombatMode(ATTACK_BERSERK))
      bonus -= my_lev/4;

    // BERSERK gets the offense benefit PLUS this...
    if (isCombatMode(ATTACK_BERSERK)) {
      int amt = my_lev/2;

      // If I'm skilled at berserking, I shouldn't be penalized as much.
      amt *= doesKnowSkill(SKILL_BERSERK) ? 100 - getSkillValue(SKILL_BERSERK) : 100;
      amt /= 100;
      bonus -= amt;
    }
  } 

  // add some positional affects here
  if (doesKnowSkill(SKILL_CHIVALRY) && getPosition() == POSITION_MOUNTED) {
    // See the balance discussion why these numbers are what they are
    // we are shooting for a defensive bonus of 12/7 (fully learned)
    // assuming base hit rate of 60% (block 40% of blows normally), this
    // means they ought to block 68.6% (.4 * 12/7) fully learned.  That
    // is to say that the hit rate ought to drop by 28.6%.  At 3% per
    // level, that becomes 9.52 levels.  Since bonus is roughly 50/3 points
    // per level, this becomes a bonus of 158.7 points.
    int amt = 159;  // we div by 4 later on
    amt *= max(10, (int) getSkillValue(SKILL_CHIVALRY));
    amt /= 100;
    bonus += amt;
  }

  // defense skill should play decent role

  // don't let this skew the return: it goes up roughly 4*lev
  // lower it by the theoretical value
  int amt = my_lev;
  amt *= min(100, 4*GetMaxLevel());
  amt /= 100;
  bonus -= amt;

  if (doesKnowSkill(SKILL_DEFENSE)) {
    int amt = my_lev;
    amt *= max(10, (int) getSkillValue(SKILL_DEFENSE));
    amt /= 100;
    bonus += amt;
  }

  // treat AGI as a modifier for +AC
  // From BALANCE: we want high AGI to yield 5/4 more hits
  // and low dex to yield 4/5 more hits
  // so assuming that "normal" is a 60% rate, high should be 75%, and low 48%
  // 1 lev = 16.67 pts of bonus = 3%
  // 1 pt of bonus = 0.18%
  // 75% rate would be extra 15% would be 83.3 pts
  // 48% rate would be loss of 12% would be 66.67 pts 
  if (!spelltask)
    bonus += (int) plotStat(STAT_CURRENT, STAT_AGI, -67, 84, 0);

  // Check if you can see your target. (penalty)
  if (attacker && !canSee(attacker)){
    int amt = my_lev;
    if(doesKnowSkill(SKILL_BLINDFIGHTING)) {
      amt *= 100 - getSkillValue(SKILL_BLINDFIGHTING);
      amt /= 100;
    }
    bonus -= amt + 1;
  }

  // positional modifiers
  // penalize for defending on ground
  // note, we do not give a benefit for attacking a victim on ground here, we
  // do that in defend
  int val = 0;
  switch (getPosition()) {
    case POSITION_DEAD:
    case POSITION_MORTALLYW:
    case POSITION_INCAP:
    case POSITION_STUNNED:
    case POSITION_SLEEPING:
      val = -bonus;  // defend while asleep?
      break;
    case POSITION_RESTING:
      val -= my_lev/3 +1;
      break;
    case POSITION_SITTING:
      val -= my_lev/4 +1;
      break;
    case POSITION_ENGAGED:
    case POSITION_FIGHTING:
    case POSITION_CRAWLING:
    case POSITION_STANDING:
      break;
    case POSITION_MOUNTED:
      val += my_lev/4 +1;
      break;
    case POSITION_FLYING:
      val += my_lev/3 +1;
      break;
  }
  if (getPosition() < POSITION_STANDING && 
        awake() && val < 0) {
    if(doesKnowSkill(SKILL_GROUNDFIGHTING)) {
      int gsv = getSkillValue(SKILL_GROUNDFIGHTING);
      val = val * (100 - gsv) / MAX_SKILL_LEARNEDNESS;
      val = min(val, -1);
    }
  }
  bonus += val;

  // we use to min/max to 0-1200 range, but I feel if you have affects that
  // drive you outside that range, you deserve the advantage/penalty
  return bonus;
}

// specialAttack() is used for combat specials like kick, bash, grapple, etc.
int TBeing::specialAttack(TBeing *target, spellNumT skill)
{
  int offense = attackRound(target);
  int defense = target->defendRound(this);
  int mod = offense - defense;
  return hits(target, mod);
}

// hits() is for an individual hit.
int TBeing::hits(TBeing *v, int mod)
{
  // mod is roughly attack-defend
  // we have constructed those formulas such that they are equal if
  // it is a fair fight.  Since they scale from 0-1000 over 60 levels,
  // a single level is 50/3 = 16.667 points of mod
  // We desire a one level difference to represent a +/- 3% chance
  // of hitting.  
  // an Even fight should hit about 60% of the time
  // Please note, the above factor was chosen arbitrarily, BUT it
  // is now tied up in the XP formula as well, so should not be
  // toyed with lightly!!  - Batopr 12/13/98

  if (desc)
    desc->session.mod_done[getCombatMode()] += mod;
  if (v->desc)
    v->desc->session.mod_received[v->getCombatMode()] += mod;

  if (!v->awake() || (v->getPosition() < POSITION_RESTING))
    return GUARANTEED_SUCCESS;
  
  // factor = 60.0 + (3.0 per level * (mod * 3 / 50 levels));
  int factor = 600 + (9 * mod / 5);
  factor = min(max(factor, 0), 1000);

  int roll = ::number(0,999);

  if (roll < 50)
    return GUARANTEED_SUCCESS;
  else if (roll >= 950)
    return GUARANTEED_FAILURE;

  else if (roll < factor)
    return TRUE;
  else
    return FALSE;
}

// returns DELETE_THIS
// returns DELETE_VICT = v
// returns DELETE_ITEM = weapon
int TBeing::missVictim(TBeing *v, TThing *weapon, spellNumT wtype)
{
  TBeing *other;
  TThing *t;
  int num, rc;
  char namebuf[MAX_NAME_LENGTH];
  char victbuf[MAX_NAME_LENGTH];
  char buf[160];

  other = NULL;
  applyDamage(v, 0, wtype);
  
  soundNumT snd = pickRandSound(SOUND_MISS_01, SOUND_MISS_10);
  roomp->playsound(snd, SOUND_TYPE_COMBAT, 100, 20, 1);

  if (wtype == TYPE_BITE) {
    num = ::number(1,1);
    switch (num) {
      case 1:
      default:
        if (desc && !IS_SET(desc->autobits, AUTO_NOSPAM))
          act("You try to bite $N, but miss.", FALSE, this, 0, v, TO_CHAR);
        if (v->desc && !(v->desc->autobits & AUTO_NOSPAM))
          act("You are missed by $n as $e tries to bite you.",
                 TRUE, this, 0, v, TO_VICT);
        for (t = roomp->stuff; t; t = t->nextThing) {
          other = dynamic_cast<TBeing *>(t);
          if (!other)
            continue;
          if ((other == this) || (other == v))
            continue;
          if (!other->awake())
            continue;
//          sprintf(namebuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(this)), NULL, COLOR_MOBS, TRUE));
//          sprintf(victbuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(v)), NULL, COLOR_MOBS, TRUE));
          strcpy(namebuf, other->pers(this));
          strcpy(victbuf, other->pers(v));
          if (other->desc && !(other->desc->autobits & AUTO_NOSPAM))
            other->sendTo(COLOR_MOBS,"%s tries to bite %s, but misses.\n\r", good_cap(namebuf).c_str(), victbuf);

        }
        break;
    }
  } else if (pierceType(wtype)) {
    num = ::number(1,10);
    if ((num == 3 || num == 4) && v->isNaked())
      num = 1;
    switch(num) {
      case 1:
        if (desc && !(desc->autobits & AUTO_NOSPAM))
          act("You thrust at $N, but miss.", FALSE, this, 0, v, TO_CHAR);
        if (v->desc && !(v->desc->autobits & AUTO_NOSPAM))
          act("You are missed by $n as $e thrusts at you.",
                 TRUE, this, 0, v, TO_VICT);
        for (t = roomp->stuff; t; t = t->nextThing) {
          other = dynamic_cast<TBeing *>(t);
          if (!other)
            continue;        
          if ((other == this) || (other == v))
            continue;
          if (!other->awake())
            continue;
          strcpy(namebuf, other->pers(this));
          strcpy(victbuf, other->pers(v));
//          sprintf(namebuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(this)), NULL, COLOR_MOBS, TRUE));
//          sprintf(victbuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(v)), NULL, COLOR_MOBS, TRUE));

          if (other->desc && !(other->desc->autobits & AUTO_NOSPAM))
            other->sendTo(COLOR_MOBS, "%s thrusts at %s, but misses.\n\r", good_cap(namebuf).c_str(),
                 victbuf);
        }
        break;
      case 2:
        if (desc && !(desc->autobits & AUTO_NOSPAM))
          act("You stab wildly, but miss $N.", FALSE, this, 0, v, TO_CHAR);
        if (v->desc && !(v->desc->autobits & AUTO_NOSPAM))
          act("You are missed by $n as $e stabs wildly.", TRUE, this, 0, v, TO_VICT);
        for (t = roomp->stuff; t; t = t->nextThing) {
          other = dynamic_cast<TBeing *>(t);
          if (!other)
            continue;
          if ((other == this) || (other == v))
            continue;
          if (!other->awake())
            continue;
          strcpy(namebuf, other->pers(this));
          strcpy(victbuf, other->pers(v));
//          sprintf(namebuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(this)), NULL, COLOR_MOBS, TRUE));
//          sprintf(victbuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(v)), NULL, COLOR_MOBS, TRUE));
          if (other->desc && !(other->desc->autobits & AUTO_NOSPAM))
            other->sendTo(COLOR_MOBS, "%s stabs wildly, but misses %s.\n\r", good_cap(namebuf).c_str(),
                 victbuf);
        }
        break;
      case 3:
      case 4:
        sprintf(buf, "You hit $N, but fail to penetrate $S thick %s.", 
              (v->isHumanoid() ? "armor" : "hide"));
        if (desc && !(desc->autobits & AUTO_NOSPAM))
          act(buf, FALSE, this, 0, v, TO_CHAR);
        sprintf(buf, "$n hits you, but fails to penetrate your thick %s.",
              (v->isHumanoid() ? "armor" : "hide"));
        if (v->desc && !(v->desc->autobits & AUTO_NOSPAM))
          act(buf, TRUE, this, 0, v, TO_VICT);
        for (t = roomp->stuff; t; t = t->nextThing) {
          other = dynamic_cast<TBeing *>(t);
          if (!other)
            continue;
          if ((other == this) || (other == v))
            continue;
          if (!other->awake())
            continue;
          strcpy(namebuf, other->pers(this));
          strcpy(victbuf, other->pers(v));
//          sprintf(namebuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(this)), NULL, COLOR_MOBS, TRUE));
//          sprintf(victbuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(v)), NULL, COLOR_MOBS, TRUE));

          if (other->desc && !(other->desc->autobits & AUTO_NOSPAM))
            other->sendTo(COLOR_MOBS, "%s hits %s, but fails to penetrate %s thick %s.\n\r", good_cap(namebuf).c_str(), victbuf, v->hshr(), 
                    (v->isHumanoid() ? "armor" : "hide"));
        }
        break;
      case 7:
      case 8:
      case 9:
      case 10:
        rc = checkShield(v, weapon, v->getSecondaryHold(), wtype, 0);
        if (rc) {
          int retCode = 0;
          if (IS_SET_DELETE(rc, DELETE_ITEM))
            retCode |= DELETE_ITEM;
          if (IS_SET_DELETE(rc, DELETE_VICT))
            retCode |= DELETE_VICT;
          if (IS_SET_DELETE(rc, DELETE_THIS))
            retCode |= DELETE_THIS;
          return retCode;
        }
        // fall thru
      default:
        if (desc && !(desc->autobits & AUTO_NOSPAM))
          act("You miss $N.", FALSE, this, 0, v, TO_CHAR);
        if (v->desc && !(v->desc->autobits &AUTO_NOSPAM))
          act("You are missed by $n.", TRUE, this, 0, v, TO_VICT);
        for (t = roomp->stuff; t; t = t->nextThing) {
          other = dynamic_cast<TBeing *>(t);
          if (!other)
            continue;
          if ((other == this) || (other == v))
            continue;
          if (!other->awake())
            continue;
          strcpy(namebuf, other->pers(this));
          strcpy(victbuf, other->pers(v));
//          sprintf(namebuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(this)), NULL, COLOR_MOBS, TRUE));
//          sprintf(victbuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(v)), NULL, COLOR_MOBS, TRUE));

          if (other->desc && !(other->desc->autobits & AUTO_NOSPAM))
            other->sendTo(COLOR_MOBS, "%s misses %s.\n\r", good_cap(namebuf).c_str(),
                 victbuf);
        }
        break;
    }
  } else {
    num = ::number(1,10);
    if ((num == 2 || num == 3) && v->isNaked())
      num = 1;
    switch(num) {
      case 1:
        if (desc && !(desc->autobits & AUTO_NOSPAM))
          act("You swing wildly, but miss $N.", 
                  FALSE, this, 0, v, TO_CHAR);
        if (v->desc && !(v->desc->autobits & AUTO_NOSPAM))
          act("You are missed by $n as $e swings wildly.",
                  TRUE, this, 0, v, TO_VICT);
        for (t = roomp->stuff; t; t = t->nextThing) {
          other = dynamic_cast<TBeing *>(t);
          if (!other)
            continue;
          if ((other == this) || (other == v))
            continue;
          if (!other->awake())
            continue;
          strcpy(namebuf, other->pers(this));
          strcpy(victbuf, other->pers(v));
//          sprintf(namebuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(this)), NULL, COLOR_MOBS, TRUE));
//          sprintf(victbuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(v)), NULL, COLOR_MOBS, TRUE));

          if (other->desc && !(other->desc->autobits & AUTO_NOSPAM))
            other->sendTo(COLOR_MOBS, "%s swings wildly, but misses %s.\n\r", good_cap(namebuf).c_str(),
                 victbuf);
        }
        break;
      case 2:
      case 3:
        sprintf(buf, "You hit $N, but fail to penetrate $S thick %s.",
              (v->isHumanoid() ? "armor" : "hide"));
        if (desc && !(desc->autobits & AUTO_NOSPAM))
          act(buf, FALSE, this, 0, v, TO_CHAR);
        sprintf(buf, "$n hits you, but fails to penetrate your thick %s.",
              (v->isHumanoid() ? "armor" : "hide"));
        if (v->desc && !(v->desc->autobits & AUTO_NOSPAM))
          act(buf, TRUE, this, 0, v, TO_VICT);
        for (t = roomp->stuff; t; t = t->nextThing) {
          other = dynamic_cast<TBeing *>(t);
          if (!other)
            continue;
          if ((other == this) || (other == v))
            continue;
          if (!other->awake())
            continue;
          strcpy(namebuf, other->pers(this));
          strcpy(victbuf, other->pers(v));
//          sprintf(namebuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(this)), NULL, COLOR_MOBS, TRUE));
//          sprintf(victbuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(v)), NULL, COLOR_MOBS, TRUE));

          if (other->desc && !(other->desc->autobits & AUTO_NOSPAM))
            other->sendTo(COLOR_MOBS, "%s hits %s, but fails to penetrate %s thick %s.\n\r",
                    good_cap(namebuf).c_str(), victbuf, v->hshr(),
                    (v->isHumanoid() ? "armor" : "hide"));
        }
        break;
      case 7:
      case 8:
      case 9:
      case 10:
        rc = checkShield(v, weapon, v->getSecondaryHold(), wtype, 0);
        if (rc) {
          int retCode = 0;
          if (IS_SET_DELETE(rc, DELETE_ITEM))
            retCode |= DELETE_ITEM;
          if (IS_SET_DELETE(rc, DELETE_VICT))
            retCode |= DELETE_VICT;
          if (IS_SET_DELETE(rc, DELETE_THIS))
            retCode |= DELETE_THIS;
          return retCode;
        }
        
      default:
        if (desc && !(desc->autobits & AUTO_NOSPAM))
          act("You miss $N.", FALSE, this, 0, v, TO_CHAR);
        if (v->desc && !(v->desc->autobits & AUTO_NOSPAM))
          act("You are missed by $n.", TRUE, this, 0, v, TO_VICT);
        for (t = roomp->stuff; t; t = t->nextThing) {
          other = dynamic_cast<TBeing *>(t);
          if (!other)
            continue;
          if ((other == this) || (other == v))
            continue;
          if (!other->awake())
            continue;
          strcpy(namebuf, other->pers(this));
          strcpy(victbuf, other->pers(v));
//          sprintf(namebuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(this)), NULL, COLOR_MOBS, TRUE));
//          sprintf(victbuf, colorString(((TBeing *) t), ((TBeing *) t)->desc, (other->pers(v)), NULL, COLOR_MOBS, TRUE));

          if (other->desc && !(other->desc->autobits & AUTO_NOSPAM))
            other->sendTo(COLOR_MOBS, "%s misses %s.\n\r", good_cap(namebuf).c_str(),
                 victbuf);
        }
        break;
    }
  }

  // handle a weapon's spec_proc 
  if (weapon && weapon->spec) {
    rc = weapon->checkSpec(v, CMD_OBJ_MISS, "", this);
    unsigned int retCode = 0;
    if (IS_SET_ONLY(rc, DELETE_THIS))
      retCode |= DELETE_VICT;
    if (IS_SET_ONLY(rc, DELETE_VICT))
      retCode |= DELETE_THIS;
    if (IS_SET_ONLY(rc, DELETE_ITEM))
      retCode |= DELETE_ITEM;
    return retCode;
  }

  return TRUE;
}

const char *describe_dam(int dam, int dam_capacity, spellNumT wtype)
{
  double p;

  if (!dam || !dam_capacity)
    return "pathetically";

  p = ((double) dam) / ((double) dam_capacity);
  if ((p >= 1) || (dam_capacity < 0)) {
    if ((wtype == TYPE_SLASH) || wtype == TYPE_SLICE) 
      return "into shreds";
    else
      return "into a bloody pulp";
  } else if (p > .64)
    return "beyond all recognition";
  else if (p > .32)
    return "incredibly well";
  else if (p > .16)
    return "very severely";
  else if (p > .08)
    return "severely";
  else if (p > .06)
    return "very hard";
  else if (p > .04)
    return "hard";
  else if (p > .02)
    return "lightly";
  else if (p > .01)
    return "very lightly";
  else if (p > .005)
    return "only slightly";
  return "pathetically";
}

static int REALNUM(TBeing *ch, wearSlotT part_hit)
{
  return (VITAL_PART(part_hit) ? 
         ch->getHit() + 11 :
         ch->getCurLimbHealth(part_hit));
}

void TBeing::normalHitMessage(TBeing *v, TThing *weapon, spellNumT w_type, int dam, wearSlotT part_hit)
{
  char buf[256];
  TThing *t;
  TBeing *other;

  other = NULL;

  char namebuf[256];
  char victbuf[256];
 
  soundNumT snd = MIN_SOUND_NUM;
  if (!dam)
    snd = pickRandSound(SOUND_PATHETIC_01, SOUND_PATHETIC_04);
  else if (bluntType(w_type))
    snd = pickRandSound(SOUND_HIT_BLUNT_01, SOUND_HIT_BLUNT_18);
  else if (slashType(w_type))
    snd = pickRandSound(SOUND_HIT_SLASH_01, SOUND_HIT_SLASH_04);
  else if (pierceType(w_type))
    snd = SOUND_HIT_PIERCE_01;
  else if (w_type == TYPE_WATER)
    snd = SOUND_WATER_WAVE;

  // correct w_type for the array offset
  w_type -= TYPE_MIN_HIT;

  for (t = roomp->stuff; t; t = t->nextThing) {

    other = dynamic_cast<TBeing *>(t);
    if (!other || !other->desc || (other == this) || (other == v) || !other->awake())
      continue;

    if (dam || !(other->desc->autobits & AUTO_NOSPAM)) {

      strcpy(namebuf, other->pers(this)); 
      strcpy(victbuf, other->pers(v)); 
      strcpy(buf, good_cap(namebuf).c_str());
      sprintf(buf + strlen(buf), " %s ", attack_hit_text[w_type].plural);
      sprintf(buf + strlen(buf), "%s's ", victbuf);
      sprintf(buf + strlen(buf), "%s ", v->describeBodySlot(part_hit).c_str());
      sprintf(buf + strlen(buf), "%s", describe_dam(dam, REALNUM(v, part_hit),
  w_type));
      sprintf(buf + strlen(buf), "%s", (weapon) ? " with " : "");
      sprintf(buf + strlen(buf), "%s", (weapon) ? hshr() : "");
      sprintf(buf + strlen(buf), "%s", (weapon) ? " " : "");
      sprintf(buf + strlen(buf), "%s.\n\r", weapon ? other->objn(weapon).c_str() : "");

      other->sendTo(COLOR_MOBS, buf);
      if (snd != MIN_SOUND_NUM)
        other->playsound(snd, SOUND_TYPE_COMBAT, 100, 20, 1);
    }
  }
  char colorBuf[40];

  if (desc && (dam || !(desc->autobits & AUTO_NOSPAM))) {
    if (isCritPart(part_hit))
      strcpy(colorBuf, greenBold());
    else
      strcpy(colorBuf, green());

    sprintf(buf, "You %s%s%s $N's %s%s%s %s%s%s.", colorBuf, attack_hit_text[w_type].singular,
          norm(),
          colorBuf,
          v->describeBodySlot(part_hit).c_str(),
          norm(),
          describe_dam(dam, REALNUM(v, part_hit), w_type),
          (weapon) ? " with your " : "",
          (weapon) ? objn(weapon).c_str() : "");

    act(buf, FALSE, this, 0, v, TO_CHAR);
    if (snd != MIN_SOUND_NUM)
      playsound(snd, SOUND_TYPE_COMBAT, 100, 20, 1);
  }

  if (v->desc && (dam || !(v->desc->autobits &AUTO_NOSPAM))) {
    if (isCritPart(part_hit))
      strcpy(colorBuf, v->redBold());
    else
      strcpy(colorBuf, v->red());

    sprintf(buf, "$n %s%s%s your %s%s%s %s%s%s.",
          colorBuf, attack_hit_text[w_type].plural, v->norm(),
          colorBuf, v->describeBodySlot(part_hit).c_str(), v->norm(),
          describe_dam(dam, REALNUM(v, part_hit), w_type),
                 ((weapon) ? " with $s " : ""),
          ((weapon) ? fname(weapon->name).c_str() : ""));
    act(buf, FALSE, this, 0, v, TO_VICT);
    if (snd != MIN_SOUND_NUM)
      v->playsound(snd, SOUND_TYPE_COMBAT, 100, 20, 1);
  }
}

void TBeing::combatFatigue(TThing *o)
{
  float num = maxWieldWeight(o, HAND_TYPE_PRIM);

  if (num <= 0.0)
    return;

  // randomly fatigue, monks will tire faster then others 
  if ((!hasClass(CLASS_MONK) && !::number(0,19)) ||
      (!::number(0,4)))
    addToMove(-1);
  
  if (!o)
    return;

  // on the surface, it might seem like a stat-based check should be
  // added here to cause more/less fatigue based on encumbrance, but realize
  // "num" is factoring in maximum wield weight (STR), so adding in another
  // stat check would be superfluous
  TBaseWeapon * tbw = dynamic_cast<TBaseWeapon *>(o);
  if (tbw && tbw->isPaired()) {
    if (::number(1,100) <= (int) (50 * 2 * tbw->getWeight() / (num * 3)))
      addToMove(-1);
  } else if (tbw && !tbw->isPaired()) 
    if (::number(1,100) <= (int) (50 * 2 * tbw->getWeight() / ( 3 * num * 3 / 2))) {
      addToMove(-1);
  } else {
    if (::number(1, 50) < (int) o->getWeight()) 
      addToMove(-1);
  }
}

// returns DELETE_THIS
// returns DELETE_VICT = v
// returns DELETE_ITEM = weapon
// returns true if shield was hit (hence block the blow)
// otherwise false
int TBeing::checkShield(TBeing *v, TThing *weapon, wearSlotT part_hit, spellNumT w_type, int dam)
{
  TThing *ts;
  TBaseClothing *shield;
  TThing *t;
  TBeing *other;
  char victbuf[256];
  char namebuf[256];
  int rc = 0;
  int retCode = 0;

  // part_hit ought to be either HOLD_RIGHT or HOLD_LEFT

  other = NULL;
  if (!(ts = v->equipment[part_hit]))
    return retCode;

  shield = dynamic_cast<TBaseClothing *>(ts);
  if (!shield || !shield->isShield())
    return retCode;

  // insure arm isn't paralyzed, etc.
  // shields must be in secondary hand, so this should be legitimate
  if (!v->canUseArm(false))
    return retCode;

  if (!v->desc || !(v->desc->autobits & AUTO_NOSPAM))
    act("You parry $N's blow with $p.", TRUE, v, shield, this, TO_CHAR);
  if (!desc || !IS_SET(desc->autobits, AUTO_NOSPAM))
    act("$n parries your blow with $p.", TRUE, v, shield, this, TO_VICT);
  for (t = roomp->stuff; t; t = t->nextThing) {
    other = dynamic_cast<TBeing *>(t);
    if (!other)
      continue;
    if ((other == this) || (other == v))
      continue;
    if (!other->awake())
      continue;
    sprintf(namebuf, other->pers(this));
    sprintf(victbuf, other->pers(v));
    string equipBuf = colorString(other, other->desc, shield->getName(), NULL, COLOR_OBJECTS, TRUE);
    if (!other->desc || !(other->desc->autobits & AUTO_NOSPAM))
      other->sendTo(COLOR_MOBS, "%s parries %s's blow with %s.\n\r",
           cap(victbuf), namebuf, equipBuf.c_str()); 
  }

  if (shield->spec){
    rc = shield->checkSpec(this, CMD_OBJ_BEEN_HIT, NULL, weapon);
    if (IS_SET_ONLY(rc, DELETE_VICT))
      retCode |= DELETE_THIS;
    if (IS_SET_ONLY(rc, DELETE_ITEM))
      retCode |= DELETE_ITEM;
    if (IS_SET_ONLY(rc, DELETE_THIS))
      retCode |= DELETE_VICT;
  }

  v->damageItem(this, part_hit, w_type, weapon, dam);
  rc = damageWeapon(v, part_hit, &weapon);
  if (IS_SET_ONLY(rc, DELETE_ITEM))
    return retCode | DELETE_ITEM;
  return TRUE;
}

// invalidTarget() checks to see if the thing I'm fighting is a valid target
// or not.  Essentially, has the target dropped link of fled?  More basic
// target verification happens for the round in canFight().
bool TBeing::invalidTarget(const TBeing *target) const
{
  if (target->desc) {
    if (target->desc->connected != CON_PLYNG)
      return TRUE;
  }

  if (!sameRoom(target))
    return TRUE;

  return FALSE;
}

// canAttack() checks for "per attack" conditions where the person attacking
// is unable to. Reasons include combat lag and effects of limb damage.
bool TBeing::canAttack(bool isprimary)
{
  // Am I lagged?
  if (checkBusy(NULL)) {
    cantHit--;
    return FALSE;
  }

  // Check legs to see if you fall down instead.
  if ((getPosition() >= POSITION_STANDING) &&
      (bothLegsHurt() || (eitherLegHurt()  && !::number(0,1)))
					   && !isFlying()) {
    sendTo("You attempt to attack, but your injury causes you problems.\n\r");
    act("You fall over.",TRUE, this, 0, 0, TO_CHAR, ANSI_RED);
    act("$n's injury causes $m to fall over.",
        TRUE, this, 0, 0, TO_ROOM, ANSI_ORANGE);
    cantHit += 1;
    addToWait(combatRound(0.5));
    if (riding)
      dismount(POSITION_SITTING);
    setPosition(POSITION_SITTING);
    return FALSE;
  }

  // Check arms.
  if (!canUseArm(isprimary)) {
    if (isprimary) {
      if (!canUseLimb(getPrimaryArm())) {
        sendTo("%sYou didn't hit here because your arm is damaged.%s\n\r",
            purple(), norm());
      } else if (!canUseLimb(getPrimaryWrist())) {
        sendTo("%sYour injured wrist makes it impossible to hit here.%s\n\r",
            purple(), norm());
      } else if (!canUseLimb(getPrimaryHand()) ||
                 !canUseLimb(getPrimaryHold())) {
        sendTo("%sYou didn't hit here because your hand is damaged.%s\n\r",
            purple(), norm());
      } else if (!canUseLimb(getPrimaryFinger())) {
        sendTo("%sYour injured finger makes it impossible to hit here.%s\n\r",
            purple(), norm());
      } else {
        // we checked all the cases right?
        vlogf(10, "WTF is wrong in hit check");
      }
    } else {
      if (!canUseLimb(getSecondaryArm())) {
        sendTo("%sYou didn't hit here because your arm is damaged.%s\n\r",
            purple(), norm());
      } else if (!canUseLimb(getSecondaryWrist())) {
        sendTo("%sYour injured wrist makes it impossible to hit here.%s\n\r",
            purple(), norm());
      } else if (!canUseLimb(getSecondaryHand()) ||
                 !canUseLimb(getSecondaryHold())) {
        sendTo("%sYou didn't hit here because your hand is damaged.%s\n\r",
            purple(), norm());
      } else if (!canUseLimb(getSecondaryFinger())) {
        sendTo("%sYour injured finger makes it impossible to hit here.%s\n\r",
            purple(), norm());
      } else {
        // we checked all the cases right?
        vlogf(10, "WTF is wrong in hit check");
      }
    }

    // basic logic to see if continuing to fight would be stupid
    TMonster *tmon = dynamic_cast<TMonster *>(this);
    if (tmon && !tmon->isPc()) {
      // monster can not hit with at least 1 hand, check other
      // if both hands broken, flee
      // if one hand broken, and not CLEARLY possessing an advantage, flee
      TBeing *v = fight();
      if (tmon->isSmartMob(0) && v &&
          (!tmon->canUseArm(!isprimary) ||
          ((tmon->getPercHit() / v->getPercHit()) < 0.8))) {
        tmon->addFeared(v);
      }
    }

    return FALSE;
  }

  // this needs work - bat 4/29/95
  if (willBump(roomp->getRoomHeight())) {
    if (::number(1,20) < (getHeight() - roomp->getRoomHeight())) {
      if (!(::number(0,3))) {
        sendTo("The cramped quarters makes it difficult to fight.\n\r");
        return FALSE;
      }
    }
  }

  return TRUE;
}

void TBeing::updateStatistics()
{
  stats.combat_blows[isPc() ? PC_STAT : MOB_STAT]++;
  stats.combat_level[isPc() ? PC_STAT : MOB_STAT] += GetMaxLevel();

  if (desc) {
    attack_mode_t amt = getCombatMode();
    desc->career.swings[amt]++;
    desc->session.swings[amt]++;
  }

  TBeing *vict = fight();
  if (vict && vict->desc) {
    attack_mode_t vamt = vict->getCombatMode();
    vict->desc->session.swings_received[vamt]++;
    vict->desc->session.level_attacked[vamt] += GetMaxLevel();
  }
}

// DELETE_VICT, v is dead, delete
// DELETE_THIS, this is dead, delete
// DELETE_ITEM : weapon is destroyed, delete   (this may be |= with the above)
// return true if further hits should cease
// otherwise, returns 0
int TBeing::oneHit(TBeing *v, bool isprimary, TThing *weapon, int mod, double f)
{
  int dam = 0, result;
  wearSlotT part_hit;
  int mess_sent = 0;
  int damaged_limb = FALSE;
  int tmp;
  int rc = 0, retCode = 0;
  bool found = FALSE;

  // Can I even attack the target?
  if (invalidTarget(v)) {
    if (v->desc && v->desc->connected != CON_PLYNG) {
      // potentially they have gone linkdead or something
      // lets check for the obvious cheat
      if (v->desc->connected == CON_WRITING) {
        vlogf(7, "%s using editing trick to prevent combat with %s at room %d",
              v->getName(), getName(), v->in_room);

        // grab their stuff if they were REALLY cheating
        catchLostLink(v);
      }
    }
    
    return retCode;
  }

  // Given that the target is available, am I in a position to attack?
  if(!canAttack(isprimary)) {
    return retCode;
  }

  // Special case for two-handed weapons.
  TObj *tobj = dynamic_cast<TObj *>(weapon);
  if (tobj && tobj->isPaired() && !canUseArm(!isprimary)) 
    return retCode;

  TBaseClothing *tbc = dynamic_cast<TBaseClothing *>(weapon);
  if (tbc && tbc->isShield())
    return retCode; 

  if (dynamic_cast<TArrow *>(weapon)) {
    sendTo("You can't attack with an arrow.\n\r");
    return retCode; 
  }

  // handle a weapon's spec_proc 
  if (weapon && weapon->spec) {
    rc = weapon->checkSpec(v, CMD_OBJ_HITTING, NULL, NULL);
    if (IS_SET_ONLY(rc, DELETE_THIS))
      retCode |= DELETE_ITEM;
    if (IS_SET_ONLY(rc, DELETE_VICT)) {
      retCode |= DELETE_VICT;
      return retCode;
    }
    if (rc)
      return retCode;
  }
  // Handle faction affiliation
  reconcileHurt(v, 0.005);

  // Update combat stats here.
  updateStatistics();

  spellNumT w_type = getAttackType(weapon);

  //   moved learn from doing for weapons and some monk passive skills to
  //   hit() to save resources 
  int myLevel = 0;
  myLevel = GetMaxLevel();
  int tarLevel = 0;
  tarLevel = v->GetMaxLevel();
  bool victimCanAttack = FALSE;

  if (v->isPc() && v->desc) {
    if ((v->canAttack(TRUE) || v->canAttack(FALSE)) &&
        ((!v->heldInPrimHand() || dynamic_cast<TBaseWeapon *>(v->heldInPrimHand())) || (!v->heldInSecHand() || dynamic_cast<TBaseWeapon *>(v->heldInSecHand())))) {
       victimCanAttack = TRUE;
    }

    if (victimCanAttack && ((tarLevel - myLevel) < (10 + (tarLevel / 5)))) {
      if (canAttack(isprimary)) {
        if (v->hasClass(CLASS_WARRIOR)) 
          v->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_DEFENSE, (120 - (2 * myLevel)));
        else 
          v->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_DEFENSE, (170 - (2 * myLevel)));
      }
    }
  }

// 1.  First check hitting vs missing outright
  result = hits(v, mod);
  if (!result) {
    found = TRUE;
    rc = missVictim(v, weapon, w_type);
    if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_VICT | DELETE_THIS)) {
      combatFatigue(weapon);
      updatePos();
      v->updatePos();
      if (IS_SET_DELETE(rc, DELETE_ITEM))
        retCode |= DELETE_ITEM;
      if (IS_SET_DELETE(rc, DELETE_VICT))
        retCode |= DELETE_VICT;
      if (IS_SET_DELETE(rc, DELETE_THIS))
        retCode |= DELETE_THIS;
      return retCode;
    }
  } else if (result == GUARANTEED_FAILURE) {
    found = TRUE;

    tmp = critFailureChance(v, weapon, w_type);
    if (IS_SET_DELETE(tmp, DELETE_THIS))
      return retCode | DELETE_THIS;
    else if (tmp == SENT_MESS && retCode)
      return retCode;
    else if (tmp == SENT_MESS)
      return TRUE;

    rc = missVictim(v, weapon, w_type);
    if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_VICT | DELETE_THIS)) {
      combatFatigue(weapon);
      updatePos();
      v->updatePos();
      if (IS_SET_DELETE(rc, DELETE_ITEM))
        retCode |= DELETE_ITEM;
      if (IS_SET_DELETE(rc, DELETE_VICT))
        retCode |= DELETE_VICT;
      if (IS_SET_DELETE(rc, DELETE_THIS))
        retCode |= DELETE_THIS;
      return retCode;
    }
  }

// 2. Do Block and Parry 
  if (found) {
// temporary for coding purposes, found isnt there for any purpose
   // can tighten up if its not deleted
  } else {
  }

// 3 Will be a hit

  if (found) {
// temporary for coding purposes, found isnt there for any purpose
   // miss

  } else if ((dam = getWeaponDam(v, weapon,isprimary)) < 1) {
    rc = missVictim(v, weapon, w_type);
    if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_VICT | DELETE_THIS)) {
      combatFatigue(weapon);
      updatePos();
      v->updatePos();
      if (IS_SET_DELETE(rc, DELETE_ITEM))
        retCode |= DELETE_ITEM;
      if (IS_SET_DELETE(rc, DELETE_VICT))
        retCode |= DELETE_VICT;
      if (IS_SET_DELETE(rc, DELETE_THIS))
        retCode |= DELETE_THIS;
      return retCode;
    }
  } else {
    if (v->isPc() && v->desc) {
      if ((tarLevel - myLevel) < (10 + (tarLevel / 5))) {
        if (canAttack(isprimary)) {
          if (victimCanAttack) {
            v->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_DEFENSE, (200 - (2 * myLevel)));
          } else {
            if (v->hasClass(CLASS_WARRIOR)) 
              v->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_DEFENSE, (300 - (2 * myLevel)));
            else 
              v->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_DEFENSE, (450 - (2 * myLevel)));
          }
        }
      }
    }
    if (v->getHit() <= 0)
      part_hit = v->getCritPartHit();
    else
      part_hit = v->getPartHit(this, TRUE);

    if (!v->hasPart(part_hit))
      part_hit = v->getCritPartHit();

    if (part_hit == HOLD_LEFT || part_hit == HOLD_RIGHT) {
      rc = checkShield(v, weapon, part_hit, w_type, dam);
      if (rc) {
        combatFatigue(weapon);
        updatePos();
        v->updatePos();
        if (IS_SET_DELETE(rc, DELETE_ITEM))
          retCode |= DELETE_ITEM;
        if (IS_SET_DELETE(rc, DELETE_VICT))
          retCode |= DELETE_VICT;
        if (IS_SET_DELETE(rc, DELETE_THIS))
          retCode |= DELETE_THIS;
        return retCode;
      } else
        part_hit = v->getCritPartHit();
    }

    stats.combat_hits[isPc() ? PC_STAT : MOB_STAT]++;
    if (desc) {
      attack_mode_t amt = getCombatMode();
      desc->career.hits[amt]++;
      desc->session.hits[amt]++;
      desc->session.potential_dam_done[amt] += dam;
    }
    if (v->desc) {
      attack_mode_t amt = v->getCombatMode();
      v->desc->session.hits_received[amt]++;
      v->desc->session.potential_dam_received[amt] += dam;
    }


    if (result == GUARANTEED_SUCCESS)
      mess_sent = critSuccessChance(v, weapon, &part_hit, w_type, &dam, -1);

    if (IS_SET_DELETE(mess_sent, DELETE_VICT)) {
      // we killed them, so increment crit-kill counter
      if (desc)
        desc->career.crit_kills++;
      if (v->desc)
        v->desc->career.crit_kills_suff++;

      v->saveCareerStats();

      return retCode | DELETE_VICT;
    }

    dam = getActualDamage(v, weapon, dam, w_type);
    absorb_damage(v, part_hit, &dam);

    if (!v->isImmortal())
      dam = max(dam, 1);
#if 0
    vlogf(5,"DAMAGE %d (%s) After getActualDamage and absorb.", dam, getName());  
#endif
    if (mess_sent != SENT_MESS && v->awake()) {
      if (monkDodge(v, weapon, &dam, w_type, part_hit))
        mess_sent = SENT_MESS;
    }
    loseSneak();

    if(mess_sent != SENT_MESS){
      if(hasClass(CLASS_MONK) && doesKnowSkill(SKILL_ADVANCED_KICKING) &&
         !weapon && isPc()) {
        // switch some "hits" to "kicks"
        // this is strictly textual and doesn't impact damage at all
        // damage modification form this is taken into account in ADVKICK
        // modification to number of hits per round
	double iskick=getSkillValue(SKILL_ADVANCED_KICKING)/100.0;
	iskick+=1.25;
	iskick*=(isprimary?0.6:0.4);

	if(f <= iskick)
	  normalHitMessage(v, NULL, TYPE_KICK, dam, part_hit);
	else 
	  normalHitMessage(v, NULL, w_type, dam, part_hit);
      } else
	normalHitMessage(v, weapon, w_type, dam, part_hit);

      // we've now hit, so do some post hit stuff
      // handle a weapon's spec_proc 
      if (weapon && weapon->spec) {
        rc = weapon->checkSpec(v, CMD_OBJ_HIT, (const char *) part_hit, this);
        if (IS_SET_ONLY(rc, DELETE_THIS))
          retCode |= DELETE_ITEM;
        if (IS_SET_ONLY(rc, DELETE_VICT)) {
          retCode |= DELETE_VICT;
          return retCode;
        }
      }
      // handle proc on the eq being hit
      if (v->equipment[part_hit] && v->equipment[part_hit]->spec){
	rc = v->equipment[part_hit]->checkSpec(this, CMD_OBJ_BEEN_HIT, NULL, weapon);
	if (IS_SET_ONLY(rc, DELETE_VICT)) 
	  retCode |= DELETE_THIS;
	if (IS_SET_ONLY(rc, DELETE_ITEM))
	  retCode |= DELETE_ITEM;
	if (IS_SET_ONLY(rc, DELETE_THIS)){
	  retCode |= DELETE_VICT;
	  return retCode;
	}
      }


      // this was developed with poison weapons in mind, but may be generic
      // enough for other stuff   - bat
      TObj *tow;
      if (weapon && (tow = dynamic_cast<TObj *>(weapon))) {
        for (int j = 0; j < MAX_SWING_AFFECT; j++) {
          if (tow->oneSwing[j].type != TYPE_UNDEFINED) {
            if (tow->oneSwing[j].renew == -1)
              v->affectTo(&(tow->oneSwing[j]), -1);
            else
              v->affectJoin(this, &(tow->oneSwing[j]), AVG_DUR_NO, AVG_EFF_NO);

            act("There was something nasty on that $o!",
                  FALSE, this, tow, v, TO_VICT, ANSI_RED);
            act("You inflict something nasty on $N!",
                  FALSE, this, tow, v, TO_CHAR, ANSI_RED);
            act("There was something nasty on that $o!",
                  FALSE, this, tow, v, TO_NOTVICT, ANSI_RED);
  
            if (tow->oneSwing[j].type == AFFECT_DISEASE)
              disease_start(v, &(tow->oneSwing[j]));
  
            // kill the affect on the weapon
            tow->oneSwing[j].type = TYPE_UNDEFINED;
            tow->oneSwing[j].level = 0;
            tow->oneSwing[j].duration = 0;
            tow->oneSwing[j].modifier = 0;
            tow->oneSwing[j].location = APPLY_NONE;
            tow->oneSwing[j].modifier2 = 0;
            tow->oneSwing[j].bitvector = 0;
          }
        }
      }
    
      // more absorbtion stuff..
      affectedData *af;
      for (af = v->affected; af; af = af->next) {
        if (af->type == SPELL_PLASMA_MIRROR) {
          int dam_blocked = min(dam-1, 3);
          dam_blocked = max(dam_blocked, 0);
          dam -= dam_blocked;
          if (dam_blocked) {
            act("The swirls of plasma surrounding $n soak up energy, sending it back toward $N!", FALSE, v, 0, this, TO_NOTVICT);
            act("The swirls of plasma surrounding you soak up energy, sending it back toward $N!", FALSE, v, 0, this, TO_CHAR);
            act("The swirls of plasma surrounding $n soak up energy, sending it back toward you!", FALSE, v, 0, this, TO_VICT);
         
            int rc = reconcileDamage(this, dam_blocked, SPELL_PLASMA_MIRROR);
            if (rc == -1) 
              retCode |= DELETE_THIS;
          }
        }
      }
    }

    if (v->equipment[part_hit])
      v->damageItem(this, part_hit, w_type, weapon, dam);

    damaged_limb = damageLimb(v, part_hit, weapon, &dam);
    if (IS_SET_DELETE(damaged_limb, DELETE_VICT))
      return retCode | DELETE_VICT;

    rc = damageWeapon(v, part_hit, &weapon);
    if (IS_SET_ONLY(rc, DELETE_ITEM))
      retCode |= DELETE_ITEM;

    if (!damaged_limb) {
      rc = applyDamage(v, dam, w_type);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        combatFatigue(weapon);
        updatePos();
        return (retCode | DELETE_VICT);
      }
    }
  }
  combatFatigue(weapon);
  updatePos();
  v->updatePos();
  return retCode;;
}

bool TBeing::isHitableAggr(TBeing *v)
{
  if (canSee(v) &&
      !v->isPlayerAction(PLR_NOHASSLE) &&
      (this != v) &&
      (!(specials.act & ACT_WIMPY) || !v->awake()) &&
      (dynamic_cast<TPerson *>(v) || (v->specials.act & ACT_ANNOYING)) &&
      (!isAffected(AFF_CHARM) || (master != v)))
    return TRUE;

  return FALSE;
}

int TBeing::preProcDam(spellNumT type, int dam) const
{
  immuneTypeT bit;
  if ((bit = getTypeImmunity(type)) == IMMUNE_NONE)
    return dam;

  // this handles straight immunity
  // secondary immunities (like immune_plus1) are handled in weaponCheck()
  dam *= (100 - getImmunity(bit));
  dam /= 100;

  return (dam);
}

immuneTypeT getTypeImmunity(spellNumT type)
{
  immuneTypeT bit = IMMUNE_NONE;

  switch (type) {
    case DAMAGE_FIRE:
    case SPELL_FIREBALL:
    case SPELL_HANDS_OF_FLAME:
    case SPELL_FLAMESTRIKE:
    case SPELL_FIRE_BREATH:
    case SPELL_RAIN_BRIMSTONE:
    case SPELL_RAIN_BRIMSTONE_DEIKHAN:
    case SPELL_INFERNO:
    case SPELL_HELLFIRE:
    case SPELL_FLAMING_SWORD:
    case SPELL_SPONTANEOUS_COMBUST:
    case DAMAGE_TRAP_FIRE:
    case DAMAGE_TRAP_TNT:
    case TYPE_FIRE:
      bit = IMMUNE_HEAT;
      break;
//    case SPELL_LIGHTNING_BOLT:
    case SPELL_CALL_LIGHTNING_DEIKHAN:
    case SPELL_CALL_LIGHTNING:
    case SPELL_LIGHTNING_BREATH:
    case DAMAGE_ELECTRIC:
      bit = IMMUNE_ELECTRICITY;
      break;
    case DAMAGE_FROST:
    case SPELL_ICY_GRIP:
    case SPELL_ARCTIC_BLAST:
    case SPELL_ICE_STORM:
    case SPELL_FROST_BREATH:
    case DAMAGE_TRAP_FROST:
      bit = IMMUNE_COLD;
      break;
    case SKILL_CHI:
    case DAMAGE_DISRUPTION:
    case SPELL_MYSTIC_DARTS:
    case SPELL_STUNNING_ARROW:
    case SPELL_COLOR_SPRAY:
    case SPELL_BLAST_OF_FURY:
    case SPELL_ATOMIZE:
    case DAMAGE_TRAP_ENERGY:
      bit = IMMUNE_ENERGY;
      break;
    case SPELL_METEOR_SWARM:
    case SPELL_EARTHQUAKE_DEIKHAN:
    case SPELL_EARTHQUAKE:
    case SPELL_PEBBLE_SPRAY:
    case SPELL_SAND_BLAST:
    case SPELL_LAVA_STREAM:
    case SPELL_SLING_SHOT:
    case SPELL_GRANITE_FISTS:
    case SPELL_PILLAR_SALT:
    case DAMAGE_COLLISION:
    case DAMAGE_FALL:
    case TYPE_EARTH:
      bit = IMMUNE_EARTH;
      break;
    case DAMAGE_GUST:
    case SPELL_GUST:
    case SPELL_DUST_STORM:
    case SPELL_TORNADO:
    case TYPE_AIR:
      bit = IMMUNE_AIR;
      break;
    case SPELL_ENERGY_DRAIN:
    case DAMAGE_DRAIN:
    case SPELL_HARM_DEIKHAN:
    case SPELL_HARM:
    case SPELL_HARM_LIGHT_DEIKHAN:
    case SPELL_HARM_SERIOUS_DEIKHAN:
    case SPELL_HARM_CRITICAL_DEIKHAN:
    case SPELL_HARM_LIGHT:
    case SPELL_HARM_SERIOUS:
    case SPELL_HARM_CRITICAL:
    case SPELL_WITHER_LIMB:
      bit = IMMUNE_DRAIN;
      break;
    case DAMAGE_ACID:
    case SPELL_ACID_BREATH:
    case SPELL_ACID_BLAST:
    case DAMAGE_TRAP_ACID:
      bit = IMMUNE_ACID;
      break;
    case SKILL_BACKSTAB:
    case SKILL_STABBING:
    case TYPE_PIERCE:
    case TYPE_STING:
    case TYPE_STAB:
    case TYPE_THRUST:
    case TYPE_SPEAR:
    case TYPE_BEAK:
    case DAMAGE_TRAP_PIERCE:
    case DAMAGE_ARROWS:
      bit = IMMUNE_PIERCE;
      break;
    case TYPE_SLASH:
    case TYPE_SLICE:
    case TYPE_CLEAVE:
    case TYPE_CLAW:
    case TYPE_BEAR_CLAW:
    case DAMAGE_TRAP_SLASH:
      bit = IMMUNE_SLASH;
      break;
    case TYPE_BLUDGEON:
    case TYPE_WHIP:
    case SKILL_CUDGEL:
    case SKILL_DOORBASH:
    case SKILL_SHOVE_DEIKHAN:
    case SKILL_SHOVE:
    case TYPE_HIT:
    case DAMAGE_KICK_HEAD:
    case DAMAGE_KICK_SIDE:
    case DAMAGE_KICK_SHIN:
    case DAMAGE_KICK_SOLAR:
    case SKILL_KICK_DEIKHAN:
    case SKILL_KICK_THIEF:
    case SKILL_KICK_MONK:
    case SKILL_KICK_RANGER:
    case SKILL_KICK:
    case TYPE_KICK:
    case SKILL_CHOP:
    case TYPE_CRUSH:
    case TYPE_BITE:
    case TYPE_SMASH:
    case TYPE_FLAIL:
    case TYPE_PUMMEL:
    case TYPE_POUND:
    case TYPE_THRASH:
    case TYPE_THUMP:
    case TYPE_WALLOP:
    case TYPE_BATTER:
    case TYPE_BEAT:
    case TYPE_STRIKE:
    case TYPE_CLUB:
    case TYPE_SMITE:
    case TYPE_MAUL:
    case SKILL_BASH_DEIKHAN:
    case SKILL_BASH_RANGER:
    case SKILL_BASH:
    case SKILL_BODYSLAM:
    case DAMAGE_TRAP_BLUNT:
      bit = IMMUNE_BLUNT;
      break;
    case SPELL_POISON_DEIKHAN:
    case SPELL_POISON:
    case SPELL_CHLORINE_BREATH:
    case DAMAGE_TRAP_POISON:
      bit = IMMUNE_POISON;
      break;
    case SPELL_SLUMBER:
    case DAMAGE_TRAP_SLEEP:
      bit = IMMUNE_SLEEP;
      break;
    case SPELL_PARALYZE:
    case SPELL_PARALYZE_LIMB:
    case SPELL_BIND:
    case SPELL_IMMOBILIZE:
    case SPELL_NUMB_DEIKHAN:
    case SPELL_NUMB:
      bit = IMMUNE_PARALYSIS;
      break;
    case SPELL_ENSORCER:
    case SPELL_CACAODEMON:
    case SPELL_CONTROL_UNDEAD:
    case SPELL_CREATE_GOLEM:
    case SPELL_ANIMATE:
    case SPELL_VOODOO:
    case SPELL_DANCING_BONES:
    case SPELL_CONJURE_AIR:
    case SPELL_CONJURE_EARTH:
    case SPELL_CONJURE_WATER:
    case SPELL_CONJURE_FIRE:
    case SKILL_BEAST_SUMMON:
    case SKILL_BEAST_SOOTHER:
    case SKILL_TRANSFIX:
      bit = IMMUNE_CHARM;
      break;
    case DAMAGE_SUFFOCATION:
    case DAMAGE_DROWN:
    case SPELL_SUFFOCATE:
    case SKILL_GARROTTE:
      bit = IMMUNE_SUFFOCATION;
      break;
    case SPELL_BONE_BREAKER:
      bit = IMMUNE_BONE_COND;
      break;
    case SPELL_BLEED:
    case DAMAGE_HEMORRAGE:
      bit = IMMUNE_BLEED;
      break;
    case SPELL_TSUNAMI:
    case SPELL_WATERY_GRAVE:
    case DAMAGE_WHIRLPOOL:
    case SPELL_GUSHER:
    case TYPE_WATER:
      bit = IMMUNE_WATER;
      break;
    case SPELL_FEAR:
      bit = IMMUNE_FEAR;
      break; 
    case SPELL_DISEASE:
    case SPELL_INFECT_DEIKHAN:
    case SPELL_INFECT:
    case DAMAGE_TRAP_DISEASE:
      bit = IMMUNE_DISEASE;
      break;
    case -1:
      bit = IMMUNE_SKIN_COND;
      break;
    case SKILL_QUIV_PALM:
    case SKILL_SPRINGLEAP:
    case SKILL_SMITE:
    case SPELL_FUMBLE:
    case SKILL_HEADBUTT:
    case SKILL_KNEESTRIKE:
    case SKILL_STOMP:
    case SPELL_BLINDNESS:
    case SPELL_CURSE_DEIKHAN:
    case SPELL_CURSE:
    case SPELL_PORTAL:
    case SPELL_RESURRECTION:
    case SPELL_ENHANCE_WEAPON:
    case SPELL_TELEPORT:
    case DAMAGE_TRAP_TELEPORT:
    case SPELL_FEATHERY_DESCENT:
    case SPELL_FLY:
    case SPELL_ANTIGRAVITY:
    case SPELL_FALCON_WINGS:
    case SPELL_LEVITATE:
    case SPELL_IDENTIFY:
    case SPELL_DIVINATION:
    case SPELL_POWERSTONE:
    case SPELL_SHATTER:
    case SPELL_EYES_OF_FERTUMAN:
    case SPELL_FARLOOK:
    case SPELL_ILLUMINATE:
    case SPELL_DETECT_MAGIC:
    case SPELL_DISPEL_MAGIC:
    case SPELL_COPY:
    case SPELL_MATERIALIZE:
    case SPELL_SPONTANEOUS_GENERATION:
    case SPELL_GALVANIZE:
    case SPELL_STONE_SKIN:
    case SPELL_TRAIL_SEEK:
    case SPELL_FAERIE_FIRE:
    case SPELL_FLAMING_FLESH:
    case SPELL_PROTECTION_FROM_FIRE:
    case SPELL_PROTECTION_FROM_ELEMENTS:
    case SPELL_PROTECTION_FROM_EARTH:
    case SPELL_PROTECTION_FROM_AIR:
    case SPELL_PROTECTION_FROM_WATER:  
    case SPELL_FLARE:
    case SPELL_INFRAVISION:
    case SPELL_SENSE_LIFE:
    case SPELL_SILENCE:
    case SPELL_STEALTH:
    case SPELL_CALM:
    case SPELL_CLOUD_OF_CONCEALMENT:
    case SPELL_DETECT_INVISIBLE:
    case SPELL_DISPEL_INVISIBLE:
    case SPELL_TELEPATHY:
//    case SPELL_FREE_ACTION:
    case SPELL_TRUE_SIGHT:
    case SPELL_POLYMORPH:
    case SPELL_ACCELERATE:
    case SPELL_HASTE:
    case SPELL_FAERIE_FOG:
    case SPELL_GILLS_OF_FLESH:
    case SPELL_BREATH_OF_SARAHAGE:
    case SPELL_CREATE_FOOD_DEIKHAN:
    case SPELL_CREATE_WATER_DEIKHAN:
    case SPELL_CREATE_FOOD:
    case SPELL_CREATE_WATER:
    case SPELL_BLESS_DEIKHAN:
    case SPELL_BLESS:
    case SPELL_HEROES_FEAST_DEIKHAN:
    case SPELL_HEROES_FEAST:
    case SPELL_ARMOR_DEIKHAN:
    case SPELL_ARMOR:
    case SPELL_ASTRAL_WALK:
    case SPELL_SANCTUARY:
    case SPELL_WORD_OF_RECALL:
    case SPELL_SUMMON:
    case SPELL_REMOVE_CURSE_DEIKHAN:
    case SPELL_REMOVE_CURSE:
    case SKILL_BEFRIEND_BEAST:
    case SKILL_BEAST_CHARM:
    case SPELL_SHAPESHIFT:
    case SPELL_PLAGUE_LOCUSTS:
    case SPELL_ROOT_CONTROL:
    case SPELL_STICKS_TO_SNAKES:
    case SPELL_LIVING_VINES:
    case SPELL_STORMY_SKIES:
    case SPELL_TREE_WALK:
    case SKILL_BARKSKIN:
    case SPELL_HEAL_LIGHT_DEIKHAN:
    case SPELL_HEAL_SERIOUS_DEIKHAN:
    case SPELL_HEAL_CRITICAL_DEIKHAN:
    case SPELL_HEAL_LIGHT:
    case SPELL_HEAL_SERIOUS:
    case SPELL_HEAL_CRITICAL:
    case SPELL_HEAL_CRITICAL_SPRAY:
    case SPELL_HEAL:
    case SPELL_HEAL_SPRAY:
    case SPELL_HEAL_FULL:
    case SPELL_HEAL_FULL_SPRAY:
    case SPELL_CURE_POISON_DEIKHAN:
    case SPELL_CURE_POISON:
    case SPELL_SALVE_DEIKHAN:
    case SPELL_SALVE:
    case SPELL_RESTORE_LIMB:
    case SPELL_KNIT_BONE:
    case SPELL_CURE_PARALYSIS:
    case SPELL_CLOT_DEIKHAN:
    case SPELL_CLOT:
    case SPELL_STERILIZE_DEIKHAN:
    case SPELL_REFRESH_DEIKHAN:
    case SPELL_STERILIZE:
    case SPELL_CURE_BLINDNESS:
    case SPELL_REFRESH:
    case SPELL_SECOND_WIND:
    case SPELL_EXPEL_DEIKHAN:
    case SPELL_EXPEL:
    case SKILL_DEATHSTROKE:
    case SPELL_SORCERERS_GLOBE:
    case SKILL_CHARGE:
    case SPELL_SYNOSTODWEOMER:
    case SPELL_INVISIBILITY:
    case SKILL_BERSERK:
    case SKILL_SHOULDER_THROW:
    case SKILL_DISARM_DEIKHAN:
    case SKILL_DISARM_THIEF:
    case SKILL_DISARM_MONK:
    case SKILL_DISARM:
    case SKILL_TRANSFORM_LIMB:
    case SPELL_CURE_DISEASE:
    case SPELL_CURE_DISEASE_DEIKHAN:
    case SPELL_PLASMA_MIRROR:
    case SPELL_GARMULS_TAIL:
    case SPELL_ETHER_GATE:
    case SPELL_VAMPIRIC_TOUCH:
    case SPELL_LIFE_LEECH:
    case SKILL_SKIN:
    case SKILL_WHITTLE:
    case SKILL_STAVECHARGE:
    case SKILL_BOW:
    case SKILL_PIERCE_PROF:
    case SKILL_BLUNT_PROF:
    case SKILL_SHARPEN:
    case SKILL_DULL:
    case SKILL_BAREHAND_PROF:
    case SKILL_ATTUNE:
    case SKILL_RANGED_SPEC:
    case SKILL_FAST_LOAD:
    case SKILL_SLASH_PROF:
    case SKILL_PIERCE_SPEC:
    case SKILL_SLASH_SPEC:
    case SKILL_SCRIBE:
    case SKILL_RESCUE:
    case SKILL_SMYTHE:
    case SKILL_SWITCH_OPP:
    case SKILL_RETREAT:
    case SKILL_GRAPPLE:
    case SKILL_HIKING:
    case SKILL_FORAGE:
    case SKILL_SEEKWATER:
    case SKILL_TRACK:
    case SKILL_RESCUE_RANGER:
    case SKILL_DUAL_WIELD:
    case SKILL_SWITCH_RANGER:
    case SKILL_RETREAT_RANGER:
    case SKILL_CONCEALMENT:
    case SKILL_DIVINATION:
    case SKILL_APPLY_HERBS:
    case SKILL_ENCAMP:
    case SKILL_CHIVALRY:
    case SKILL_RESCUE_DEIKHAN:
    case SKILL_SWITCH_DEIKHAN:
    case SKILL_RETREAT_DEIKHAN:
    case SKILL_RIDE:
    case SKILL_CALM_MOUNT:
    case SKILL_TRAIN_MOUNT:
    case SKILL_ADVANCED_RIDING:
    case SKILL_RIDE_DOMESTIC:
    case SKILL_RIDE_NONDOMESTIC:
    case SKILL_RIDE_WINGED:
    case SKILL_RIDE_EXOTIC:
    case SKILL_LAY_HANDS:
    case SKILL_YOGINSA:
    case SKILL_CINTAI:
    case SKILL_OOMLAT:
    case SKILL_ADVANCED_KICKING:
    case SKILL_GROUNDFIGHTING:
    case SKILL_DUFALI:
    case SKILL_RETREAT_MONK:
    case SKILL_SNOFALTE:
    case SKILL_COUNTER_MOVE:
    case SKILL_SWITCH_MONK:
    case SKILL_JIRIN:
    case SKILL_KUBO:
    case SKILL_CATFALL:
    case SKILL_WOHLIN:
    case SKILL_VOPLAT:
    case SKILL_BLINDFIGHTING:
    case SKILL_CRIT_HIT:
    case SKILL_FEIGN_DEATH:
    case SKILL_BLUR:
    case SKILL_HURL:
    case SKILL_SWINDLE:
    case SKILL_SNEAK:
    case SKILL_RETREAT_THIEF:
    case SKILL_PICK_LOCK:
    case SKILL_SEARCH:
    case SKILL_SPY:
    case SKILL_SWITCH_THIEF:
    case SKILL_STEAL:
    case SKILL_DETECT_TRAP:
    case SKILL_SUBTERFUGE:
    case SKILL_DISARM_TRAP:
    case SKILL_HIDE:
    case SKILL_POISON_WEAPON:
    case SKILL_DISGUISE:
    case SKILL_DODGE_THIEF:
    case SKILL_SET_TRAP:
    case SKILL_DUAL_WIELD_THIEF:
    case SKILL_COUNTER_STEAL:
    case SKILL_BREW:
    case SKILL_TURN:
    case SKILL_SIGN:
    case SKILL_SWIM:
    case SKILL_CONS_UNDEAD:
    case SKILL_CONS_VEGGIE:
    case SKILL_CONS_DEMON:
    case SKILL_CONS_ANIMAL:
    case SKILL_CONS_REPTILE:
    case SKILL_CONS_PEOPLE:
    case SKILL_CONS_GIANT:
    case SKILL_CONS_OTHER:
    case SKILL_READ_MAGIC:
    case SKILL_BANDAGE:
    case SKILL_CLIMB:
    case SKILL_FAST_HEAL:
    case SKILL_EVALUATE:
    case SKILL_TACTICS:
    case SKILL_DISSECT:
    case SKILL_DEFENSE:
    case SKILL_OFFENSE:
    case SKILL_WIZARDRY:
    case SKILL_MEDITATE:
    case SKILL_DEVOTION:
    case SKILL_PENANCE:
    case SKILL_BLUNT_SPEC:
    case SKILL_BAREHAND_SPEC:
    case DAMAGE_NORMAL:
    case DAMAGE_BEHEADED:
    case DAMAGE_RAMMED:
    case DAMAGE_HACKED:
    case DAMAGE_CAVED_SKULL:
    case DAMAGE_HEADBUTT_SKULL:
    case DAMAGE_KNEESTRIKE_FOOT:
    case DAMAGE_KNEESTRIKE_SHIN:
    case DAMAGE_KNEESTRIKE_KNEE:
    case DAMAGE_KNEESTRIKE_THIGH:
    case DAMAGE_KNEESTRIKE_CROTCH:
    case DAMAGE_KNEESTRIKE_SOLAR:
    case DAMAGE_KNEESTRIKE_CHIN:
    case DAMAGE_KNEESTRIKE_FACE:
    case DAMAGE_HEADBUTT_JAW:
    case DAMAGE_HEADBUTT_THROAT:
    case DAMAGE_HEADBUTT_BODY:
    case DAMAGE_HEADBUTT_CROTCH:
    case DAMAGE_HEADBUTT_LEG:
    case DAMAGE_HEADBUTT_FOOT:
    case DAMAGE_DISEMBOWLED:
    case DAMAGE_STOMACH_WOUND:
    case DAMAGE_IMPALE:
    case DAMAGE_STARVATION:
    case DAMAGE_EATTEN:
    case TYPE_MAX_HIT:
    case MAX_SKILL:
    case AFFECT_TRANSFORMED_HANDS:
    case AFFECT_TRANSFORMED_ARMS:
    case AFFECT_TRANSFORMED_LEGS:
    case AFFECT_TRANSFORMED_HEAD:
    case AFFECT_TRANSFORMED_NECK:
    case LAST_TRANSFORMED_LIMB:
    case LAST_BREATH_WEAPON:
    case AFFECT_DUMMY:
    case AFFECT_DRUNK:
    case AFFECT_NEWBIE:
    case AFFECT_SKILL_ATTEMPT:
    case AFFECT_FREE_DEATHS:
    case AFFECT_TEST_FIGHT_MOB:
    case AFFECT_DRUG:
    case AFFECT_ORPHAN_PET:
    case AFFECT_DISEASE:
    case AFFECT_COMBAT:
    case AFFECT_PET:
    case AFFECT_PLAYERKILL:
    case LAST_ODDBALL_AFFECT:
      bit = IMMUNE_NONE;
      break;
  }
  return bit;
}

int TBeing::weaponCheck(TBeing *v, TThing *o, spellNumT type, int dam)
{
  int total = 0;
  immuneTypeT imm_type = getTypeImmunity(type);
  int voplat_learn, imm_num;

  // this should only check weapon attacks, other imms handled in preProcDam
  if (imm_type != IMMUNE_SLASH &&
      imm_type != IMMUNE_BLUNT &&
      imm_type != IMMUNE_PIERCE) {
    return dam;
  }

  TObj *tobj = dynamic_cast<TObj *>(o);
//  total = (tobj ? tobj->itemHitroll() : 0);  
  if (tobj) {
    total = tobj->itemHitroll();
    if (!total & tobj->isObjStat(ITEM_MAGIC))
      total = 1;
  }
  if(doesKnowSkill(SKILL_VOPLAT) && !o){
    voplat_learn=getSkillValue(SKILL_VOPLAT);
    total=(voplat_learn-1)/25;
    if(::number(0, 25) >= (voplat_learn-(25*total)))
      total=0;
  } else 
    voplat_learn=0;
  

  if (total == 1) {
    imm_num=v->getImmunity(IMMUNE_PLUS1);
    dam *= (100 - (int) imm_num);
    dam /= 100;
    return dam;
  } else if (total == 2) {
    imm_num=v->getImmunity(IMMUNE_PLUS2);
    dam *= (100 - (int) imm_num);
    dam /= 100;
    return dam;
  } else if (total == 3) {
    imm_num=v->getImmunity(IMMUNE_PLUS3);
    dam *= (100 - (int) imm_num);
    dam /= 100;
    return dam;
  } else {
    imm_num=v->getImmunity(IMMUNE_NONMAGIC);
    dam *= (100 - (int) imm_num);
    dam /= 100;
    return dam;
  }

}

int TBeing::numValidSlots()
{
  int count=0;
  wearSlotT i;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (slotChance(i))
      count++;
  }
  if (!count) {
    vlogf(1, "ch->numValidSlots returned 0 for %s. Setting to 1.", getName());
    count = 1;
  }
  return (count);
}

// the bigger this number is, the better
int TBeing::acForEq() const
{
  int i, armor = 0;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    TObj *o = dynamic_cast<TObj *>(equipment[i]);
    if (o) {
      armor -= o->itemAC();
    }
  }
  return (armor);
}

int TBeing::acForPosSkin(wearSlotT pos) const
{
  int armor;

  armor = 1000 - getArmor();
  armor += acForEq();

  if (!hasPart(pos))
    return (0);
  else {
    // use the map function since the constant array uses the old style
    // of ordering
    armor = (armor * ac_percent_pos[mapSlotToFile(pos)]) / 2000;
    return (armor);
  }
}

int TBeing::acForPos(wearSlotT pos) const
{
  int armor = 0;
  TThing *t;
  TObj *o = NULL;

  if (!(t = equipment[pos]) || !(o = dynamic_cast<TObj *>(t)))
    return (acForPosSkin(pos));
  else {
    armor -= o->itemAC();

    if (!o->getMaxStructPoints())
      return (0);
    else if (o->getMaxStructPoints() == -1)
      return (armor);
    else
      armor *= (o->getStructPoints()) / (o->getMaxStructPoints());

    return (armor);
  }
}

// called once per round to verify conditions
bool TBeing::canFight(TBeing *target)
{
  int rc;
  TThing *t;

  if (roomp && roomp->isRoomFlag(ROOM_PEACEFUL)) {
    vlogf(3, "hit() called in PEACEFUL room.  %s hitting %s", getName(), target->getName());
    stopFighting();
    return FALSE;
  }
  if (tooTired())
    return FALSE;

  if (!sameRoom(target)) {
    vlogf(2, "NOT in same room when fighting : %s, %s", name, target->name);
    if (fight())
      stopFighting();
    return FALSE;
  }
  if (target->attackers >= MAX_COMBAT_ATTACKERS && (specials.fighting != target)) {
    sendTo("You can't attack them,  no room!\n\r");
    return FALSE;
  }
  if ((attackers >= MAX_COMBAT_ATTACKERS) && (target->fight() != this) &&
      (specials.fighting != target)) {
    sendTo("There are too many other people in the way.\n\r");
    return FALSE;
  }
  if (target->getPosition() == POSITION_DEAD)
    return FALSE;

  if (getPosition() <= POSITION_STUNNED)
    return FALSE;

  // See if my arms are transformed.
  if (affectedBySpell(SPELL_FALCON_WINGS)) {
    sendTo("Your feathery wings disappear as you attempt to fight.\n\r");
    act("$n's feathery wings disappear quickly as $e attempts to fight.",
        TRUE,this,0,0,TO_ROOM);
    sendTo("In the shock over your transformation, you forget what it was you wanted to do.\n\r");
    affectFrom(SPELL_FALCON_WINGS);
    return FALSE;
  }

  if (riding) {
    // make these checks trivial in nature by forcing them to fail
    // twice before falling off
    if (dynamic_cast<TBeing *> (riding)) {
      if (!rideCheck(-5) &&
          !rideCheck(-5)) {
        rc = fallOffMount(riding, POSITION_SITTING);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return FALSE;
      }
    } else if (dynamic_cast<TMonster *> (this) && !desc) {
      if (fight() && !isAffected(AFF_CHARM) && ::number(0, 1)) {
        dynamic_cast<TMonster *> (this)->standUp();
      }
    }
  }
  for (t = rider; t; t = t->nextRider) {
    TBeing *tb = dynamic_cast<TBeing *>(t);
    if (tb && 
        !tb->rideCheck(-10) &&
        !tb->rideCheck(-10)) {
      rc = tb->fallOffMount(this, POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tb;
        tb = NULL;
      }
      return FALSE;
    }
  }
  // make um fly if appropriate
  if (!target->isPc() && target->canFly() && !target->isFlying()) 
    target->doFly();

  return TRUE;
}

bool TBeing::damCheckDeny(const TBeing *v, spellNumT type) const
{
  TRoom *rp;

#if 0
  if (IS_DIURNAL(v))
    return TRUE;
  if (IS_NOCTURNAL(v))
    return TRUE;
#endif

  rp = roomp;
  if (rp && rp->isRoomFlag(ROOM_PEACEFUL) &&
      type != SPELL_POISON && type != SPELL_POISON_DEIKHAN && (this != v)) 
    return TRUE;                // true, they are denied from fighting 
  
  return FALSE;
}


bool TBeing::damDetailsOk(const TBeing *v, int dam, bool ranged) const
{
  if (dam < 0)
    return FALSE;

  if (!ranged && !sameRoom(v))
    return FALSE;

  return TRUE;
}


int TBeing::setCharFighting(TBeing *v, int dam)
{
  if (!v) {
    forceCrash("No victim in call to setCharFighting! (%s)", getName());
    sendTo("Something bad happened, tell a god what you did!\n\r");
    return -1;
  }

  if ((getPosition() > POSITION_STUNNED) && !fight()) {
    if (v->isImmortal() && v->isPc() && (v != this) &&
        (v->isPlayerAction(PLR_NOHASSLE) || isPc())) {
      act("You can't attack $M, $E's immortal.",TRUE,this,0,v,TO_CHAR);
      act("$n just tried to attack you.  What a weenie.",
         TRUE,this,0,v,TO_VICT);
      return -1;
    }
    TMonster *tmons = dynamic_cast<TMonster *>(v);
    if (tmons && (tmons->specials.act & ACT_IMMORTAL)) {
      sendTo("Attack an immortal?!?  Don't we have an ego...\n\r");
      tmons->doAction(fname(name),CMD_GROWL);
      return -1;
    }
    setFighting(v, dam, FALSE);
    return 1;
  }
  return 0;
}

int TBeing::setVictFighting(TBeing *v, int dam)
{
  if (v == this)
    return FALSE;

  if ((v->getPosition() > POSITION_STUNNED) && !v->fight()) {
    if (v->isImmortal() && v->isPc() && (v != this) &&
        (v->isPlayerAction(PLR_NOHASSLE) || isPc())) {
      return -1;
    }
    TMonster *tmons = dynamic_cast<TMonster *>(v);
    if (tmons && (tmons->specials.act & ACT_IMMORTAL)) 
      return -1;
    
    if (!desc && v->desc && !v->fight()) // aggro mob against a non fighting pc
      v->setFighting(this, dam, 1);
    else 
      v->setFighting(this, dam, FALSE);

    return TRUE;
  }
  return FALSE;

}

int TBeing::damageTrivia(TBeing *v, TThing *o, int dam, spellNumT type)
{
  dam = v->preProcDam(type, dam);

  if (dam > -1) {
    dam = weaponCheck(v, o, type, dam);
    dam = max(dam, 0);
  }

  return dam;
}

void fleeFlush(TBeing *ch)
{
  if (ch->desc) {
    commText *s;
    while ((s = ch->desc->input.getBegin())) {
      if (strcmp("flee", s->getText())) {
        ch->sendTo("Flushing input command: '%s'\n\r", 
             s->getText());
        ch->desc->input.setBegin(s->getNext());
        delete s;
        s = NULL;
      } else {
        ch->sendTo(COLOR_BASIC, "<R>Your next queued command is a flee.<1>\n\r");
        break;
      }
    }
  }
}

int fleeCheck(TBeing *ch)
{
  if (ch->desc) {
    commText *s;
    s = ch->desc->input.getBegin();
    while (s) {
      if (!strcmp("flee", s->getText())) {
        return TRUE;
      }
      s = s->getNext();
    }
  }
  return FALSE;
}

// returns DELETE_THIS
int TBeing::tellStatus(int dam, int same, int flying)
{
  int new_dam, max_hit;
  int rc;
  TThing *ch, *i;

  new_dam = points.hit;

  if ((new_dam >= -10) && (new_dam <= -6)) {
    act("$n is mortally wounded, and will die soon, if not aided.",
        TRUE, this, 0, 0, TO_ROOM);
    act("You are mortally wounded, and will die soon, if not aided.",
        FALSE, this, 0, 0, TO_CHAR);
  } else if ((new_dam >= -5) && (new_dam <= -3)) {
    act("$n is incapacitated and will slowly die, if not aided.",
        TRUE, this, 0, 0, TO_ROOM);
    act("You are incapacitated and you will slowly die, if not aided.",
        FALSE, this, 0, 0, TO_CHAR);
  } else if ((new_dam >= -2) && (new_dam <= 0)) {
    act("$n is stunned, but will probably regain consciousness.",
        TRUE, this, 0, 0, TO_ROOM);
    act("You're stunned, but you will probably regain consciousness.",
        FALSE, this, 0, 0, TO_CHAR);
  } else if (new_dam < -10) {
    if (spelltask) {
        act("$n's words stop in mid air and $s limbs go limp.",
            TRUE, this, NULL, NULL, TO_ROOM);
    }
    act("$n is dead! R.I.P.", TRUE, this, 0, 0, TO_ROOM);
    sendTo(COLOR_BASIC, "<R>You are dead!  Sorry...<z>\n\r");

    soundNumT snd = pickRandSound(SOUND_DEATH_CRY_01, SOUND_DEATH_CRY_11);
    roomp->playsound(snd, SOUND_TYPE_COMBAT, 100, 100, 1);

    if(!inGrimhaven()){
      for (i = roomp->stuff; i; i = i->nextThing) {
	TMonster *tmons = dynamic_cast<TMonster *>(i);
	if(tmons && tmons != this){
	  tmons->UA(1);
	  tmons->UM(1);
	  tmons->US(4);
	}
      }
    }
  } else {
    max_hit = hitLimit();
    if (dam > (max_hit / 5))
      sendTo("That really did %sHURT%s!\n\r", red(), norm());
    if ((isCharm() || isPet()) && !same) {
      if ((getHit() <= (hitLimit() * 4 / 10)) || (master && !sameRoom(master))) {
        if (fight() && !isCombatMode(ATTACK_BERSERK)) {
          if (!fleeCheck(this)) {
            if (addCommandToQue("flee") == DELETE_THIS)
              return DELETE_THIS;
          }
          fleeFlush(this);
        }
      }
      return TRUE;
    }
    if (dam && (getHit() < (hitLimit() / 6))) 
      sendTo("You wish that your wounds would stop %sHURTING%s so much!!\n\r", red(), norm());
    
    if (getHit() < (hitLimit() / 10)) {
      if (dynamic_cast<TMonster *>(this) && !same && (specials.act & ACT_WIMPY) && (dam > 0)) {
        if (fight() && !isCombatMode(ATTACK_BERSERK)) {
          if (!fleeCheck(this)) {
            if (addCommandToQue("flee") == DELETE_THIS)
              return DELETE_THIS;
          }
          fleeFlush(this);
        }
        return TRUE;
      }
    }
    if (dynamic_cast<TPerson *>(this)) {
      if (!same && wimpy && (getHit() <= wimpy)) {
        // avoid fleeing from things like starve/bleed : fight()
        // prevent berserk from making them flee
        // if the room denies fleeing, make it skip too
        if (fight() && !isCombatMode(ATTACK_BERSERK) &&
            !roomp->isRoomFlag(ROOM_NO_FLEE)) {
          if (!fleeCheck(this)) {
            if (addCommandToQue("flee") == DELETE_THIS)
              return DELETE_THIS;
          }
          fleeFlush(this);
        }
      }
      return TRUE;
    }

    // this whole else case is for hit > 0, so skip remaining checks
    return TRUE;
  }
  if (getPosition() <= POSITION_INCAP) {
    while ((ch = rider)) {
      TBeing * tb = dynamic_cast<TBeing *>(ch);
      if (tb) {
        rc = tb->fallOffMount(this, POSITION_SITTING);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tb;
          tb = NULL;
        }
      } else {
        ch->dismount(POSITION_DEAD);
      }
    }
    if (riding && dynamic_cast<TBeing *> (riding)) {
      rc = fallOffMount(riding, getPosition());
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      }
    } else if (riding) {
      rc = fallOffMount(riding, getPosition(), TRUE);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      }
    }
  }
  if (flying) {
    rc = crashLanding(getPosition(), FALSE, 0);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  return TRUE;
}

void TBeing::catchLostLink(TBeing *v)
{
  char buf[1024];
  TObj *note, *bag;
  TMoney *money;
  TThing *o;
  time_t ct;
  char *tmstr;
  TRoom *rp;

  *buf = '\0';

  act("$n is rescued by divine forces.", TRUE, v, 0, 0, TO_ROOM);
  vlogf(9, "%s lost link while fighting %s (%d)", v->getName(), getName(), in_room);

  v->specials.was_in_room = v->in_room;
  if (v->in_room != ROOM_NOWHERE)
    --(*v);
  rp = real_roomp(ROOM_STORAGE);
  *rp += *v;

  if (((v->getHit() < (v->hitLimit() / 2)) ||
     v->isCombatMode(ATTACK_BERSERK)) && (v->GetMaxLevel() <= MAX_MORT)) {
    vlogf(9, "Creating link-loss bag for %s", v->getName());
    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    sprintf(buf, "Current time is: %s (PST)\n\r", tmstr);

    sprintf(buf + strlen(buf), "%s hp when link lost : %d/%d\n\r", v->getName(), v->getHit(), v->hitLimit());
    if (v->isCombatMode(ATTACK_BERSERK))
      sprintf(buf + strlen(buf), "%s was berserking (link loss is only way to flee)\n\r", v->getName());
    if (v->eitherArmHurt())
      sprintf(buf + strlen(buf), "%s had at least one busted arm.\n\r", v->getName());
    if (v->eitherLegHurt())
      sprintf(buf + strlen(buf), "%s had at least one busted leg.\n\r", v->getName());

    sprintf(buf + strlen(buf), "Opponent (%s) hp when link lost : %d/%d.\n\r", getName(), getHit(), hitLimit());
    sprintf(buf + strlen(buf), "Time was %s when this happened.\n\r", tmstr);

    if (!(note = read_object(GENERIC_NOTE, VIRTUAL))) {
      vlogf(9, "Had trouble loading note in catch_lost_link(). Returning out.");
      return;
    }
    if (!(bag = read_object(GENERIC_L_BAG, VIRTUAL))) {
      vlogf(10, "Had trouble loading bag in catch_lost_link(). Returning out.");
      return;
    }
    bag->swapToStrung();
    note->swapToStrung();
    bag->addObjStat(ITEM_NOPURGE);

//    bag->addObjStat(ITEM_NORENT);    // norent prevents saveRooms
//    note->addObjStat(ITEM_NORENT);   // norent prevents saveRooms

    delete [] note->action_description;
    note->action_description = mud_str_dup(buf);
    delete [] note->name;
    note->name = mud_str_dup("note check link lost");

    sprintf(buf, "A linkbag containing %s's belongings sits here.", v->getName());
    delete [] bag->getDescr();
    bag->setDescr(mud_str_dup(buf));
    sprintf(buf, "linkbag %s", lower(v->getName()).c_str());
    delete [] bag->name;
    bag->name = mud_str_dup(buf);

    while ((o = v->stuff)) {
      (*o)--;
      *bag += *o;
    }
  
    if (v->getMoney() > 0) {
      money = create_money(v->getMoney());
      v->setMoney(0);
      *bag += *money;
    }
    wearSlotT ij;
    for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
      if (v->equipment[ij])
        *bag += *(v->unequip(ij));
    }
    v->setCarriedWeight(0.0);
    v->setCarriedVolume(0);
    v->removeRent();
    *bag += *note;
    *rp += *bag;

    bag->parent = NULL;
    bag->equippedBy = NULL;
    bag->stuckIn = NULL;

    bag->parent = NULL;

    // keep from being aggro'd on return
    DeleteHatreds(this, NULL);
    if (attackers && !isPc()) {
      TMonster *tmons = dynamic_cast<TMonster *>(this);
      tmons->setAnger(0);
      tmons->setMalice(0);
    }

    sprintf(buf, "%s detected you lost link while fighting an apparently superior opponent\n\r", MUD_NAME);
    sprintf(buf + strlen(buf), "(%s).  Under such circumstances, the policy is to\n\r", getName());
    sprintf(buf + strlen(buf), "confiscate your items and gold pending a review of the incident by an immortal.\n\r");
    sprintf(buf + strlen(buf), "To get your items back, you will have to speak with a god.  Be advised that if\n\r");
    sprintf(buf + strlen(buf), "it appears you were truly in a losing situation, the god has been instructed to\n\r");
    sprintf(buf + strlen(buf), "deduct 1 death's worth of xp and confiscate an item or two.  The rationale for\n\r");
    sprintf(buf + strlen(buf), "this policy is to discourage people from dropping link in order to avoid death.\n\r\n\r");
    sprintf(buf + strlen(buf), "If you have questions or comments regarding this policy, please contact a god.\n\r");
    sprintf(buf + strlen(buf), "Type WHO -G to see if any gods are presently connected.\n\r");
    sprintf(buf + strlen(buf), "\n\r\n\rThis message was automatically generated.\n\r");
    autoMail(v, NULL, buf);

  }
}

// return DELETE_THIS if this dies
// else false
int TBeing::checkEngagementStatus()
{
// false will mean pc will be hitting, true means will just engage

  if (isAffected(AFF_ENGAGER)) 
    return TRUE;

  if ((!isPc() && !desc) || !desc) 
    return FALSE;
  else if (IS_SET(desc->autobits, AUTO_ENGAGE)) 
    return TRUE;
  else if (IS_SET(desc->autobits, AUTO_ENGAGE_ALWAYS)) 
    return TRUE;
  else 
    return FALSE;
}

spellNumT TBeing::getAttackType(const TThing *wielded) const
{
  if (wielded)
    return wielded->getWtype();
  else if (affectedBySpell(AFFECT_TRANSFORMED_HANDS))
    return TYPE_BEAR_CLAW;
  else if (affectedBySpell(AFFECT_TRANSFORMED_ARMS))
    return TYPE_CLAW;
  else if (hasClass(CLASS_MONK))
    return monkDamType();
  else if (dynamic_cast<const TMonster *>(this))
    return getFormType();
  else
    return TYPE_HIT;
}

spellNumT TThing::getWtype() const
{
  return TYPE_SMITE;
}

spellNumT TGenWeapon::getWtype() const
{
  switch (getWeaponType()) {
    case WEAPON_TYPE_NONE:
      return TYPE_SMITE;
    case WEAPON_TYPE_STAB:
      return TYPE_STAB;
    case WEAPON_TYPE_WHIP:
      return TYPE_WHIP;
    case WEAPON_TYPE_SLASH:
      return TYPE_SLASH;
    case WEAPON_TYPE_SMASH:
      return TYPE_SMASH;
    case WEAPON_TYPE_CLEAVE:
      return TYPE_CLEAVE;
    case WEAPON_TYPE_CRUSH:
      return TYPE_CRUSH;
    case WEAPON_TYPE_BLUDGEON:
      return TYPE_BLUDGEON;
    case WEAPON_TYPE_CLAW:
      return TYPE_CLAW;
    case WEAPON_TYPE_BITE:
      return TYPE_BITE;
    case WEAPON_TYPE_STING:
      return TYPE_STING;
    case WEAPON_TYPE_PIERCE:
      return TYPE_PIERCE;
    case WEAPON_TYPE_PUMMEL:
      return TYPE_PUMMEL;
    case WEAPON_TYPE_FLAIL:
      return TYPE_FLAIL;
    case WEAPON_TYPE_BEAT:
      return TYPE_BEAT;
    case WEAPON_TYPE_THRASH:
      return TYPE_THRASH;
    case WEAPON_TYPE_THUMP:
      return TYPE_THUMP;
    case WEAPON_TYPE_WALLOP:
      return TYPE_WALLOP;
    case WEAPON_TYPE_BATTER:
      return TYPE_BATTER;
    case WEAPON_TYPE_STRIKE:
      return TYPE_STRIKE;
    case WEAPON_TYPE_CLUB:
      return TYPE_CLUB;
    case WEAPON_TYPE_SLICE:
      return TYPE_SLICE;
    case WEAPON_TYPE_POUND:
      return TYPE_POUND;
    case WEAPON_TYPE_THRUST:
      return TYPE_THRUST;
    case WEAPON_TYPE_SPEAR:
      return TYPE_SPEAR;
    case WEAPON_TYPE_SMITE:
      return TYPE_SMITE;
    case WEAPON_TYPE_BEAK:
      return TYPE_BEAK;
    case WEAPON_TYPE_AIR:
      return TYPE_AIR;
    case WEAPON_TYPE_EARTH:
      return TYPE_EARTH;
    case WEAPON_TYPE_FIRE:
      return TYPE_FIRE;
    case WEAPON_TYPE_WATER:
      return TYPE_WATER;
    case WEAPON_TYPE_BEAR_CLAW:
      return TYPE_BEAR_CLAW;
    default:
      return TYPE_HIT;
  }
}

int TBeing::skipImmortals(int amnt) const
{
  if (isImmortal())
    amnt = -1;

  // special type of monster
  if (dynamic_cast<const TMonster *>(this) && (specials.act & ACT_IMMORTAL))
    amnt = -1;

  return amnt;
}

TBeing *TBeing::findAnAttacker() const
{
  TThing *tmp;

  if (specials.fighting)
    return (specials.fighting);

  if (in_room < 0)
    return NULL;

  for (tmp = roomp->stuff; tmp; tmp = tmp->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(tmp);
    if (!tbt)
      continue;
    if (this == tbt)
      continue;
    if (tbt->fight() == this)
      return tbt;
   // The more evil the mob is, the weaker the person it tries to use 
  }

  return NULL;
}

// may return DELETE_THIS
int TBeing::objDam(spellNumT damtype, int amnt, TThing *t)
{
  int rc;
  char damdesc[20];
  char buf[132];

  switch (damtype) {
    case DAMAGE_TRAP_PIERCE:
      strcpy(damdesc, "pierced");
      break;
    case DAMAGE_TRAP_SLASH:
      strcpy(damdesc, "sliced");
      break;
    case DAMAGE_TRAP_BLUNT:
      strcpy(damdesc, "pounded");
      break;
    case DAMAGE_TRAP_FIRE:
      strcpy(damdesc, "seared");
      break;
    case DAMAGE_TRAP_FROST:
      strcpy(damdesc, "frozen");
      break;
    case DAMAGE_TRAP_ACID:
      strcpy(damdesc, "corroded");
      break;
    case DAMAGE_TRAP_ENERGY:
      strcpy(damdesc, "blasted");
      break;
    case DAMAGE_TRAP_SLEEP:
      strcpy(damdesc, "knocked out");
      break;
    case DAMAGE_TRAP_TELEPORT:
      strcpy(damdesc, "transported");
      break;
    case DAMAGE_TRAP_DISEASE:
      strcpy(damdesc, "diseased");
      break;
    case DAMAGE_TRAP_TNT:
    default:
      strcpy(damdesc, "blown away");
      break;
  }

  if ((damtype != DAMAGE_TRAP_TELEPORT) &&
      (damtype != DAMAGE_TRAP_SLEEP) &&
      (damtype != DAMAGE_TRAP_DISEASE)) {
    if (!t) {
// handle messaging in the trap function
    } else {
      if (amnt > 0) {
        sprintf(buf, "$n is %s by $p!", damdesc);
        act(buf, TRUE, this, t, 0, TO_ROOM);
        sprintf(buf, "You are %s by $p!", damdesc);
        act(buf, TRUE, this, t, 0, TO_CHAR);
      } else {
        sprintf(buf, "$n is almost %s by $p!", damdesc);
        act(buf, TRUE, this, t, 0, TO_ROOM);
        sprintf(buf, "You are almost %s by $p!", damdesc);
        act(buf, TRUE, this, t, 0, TO_CHAR);
      }
    }
  } else if (damtype == DAMAGE_TRAP_TELEPORT) {
    rc = trapTeleport(amnt);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  } else if (damtype == DAMAGE_TRAP_SLEEP) {
    rc = trapSleep(amnt);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  } else if (damtype == DAMAGE_TRAP_DISEASE) 
    trapDisease(amnt);

  return FALSE;
}

// returns DELETE_THIS , or FALSE
int TBeing::objDamage(spellNumT damtype, int amnt, TThing *t)
{
  int rc;

  amnt = skipImmortals(amnt);
  if (amnt == -1)
    return FALSE;

  // check for SANCTUARY
  amnt *= 100 - getProtection();
  amnt /= 100;

  amnt = preProcDam(damtype, amnt);
  if (isLucky(levelLuckModifier(10)))
    amnt = max((int) (amnt / 2), 0);

  amnt = max(amnt, 0);

  points.hit -= amnt;
  updatePos();
  rc = objDam(damtype, amnt, t);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;


  informMess();

  if (getPosition() == POSITION_DEAD) {
    if ((dynamic_cast<TPerson *>(this) || (desc && desc->original)) && roomp && roomp->name) {
      if (desc && desc->original && desc->original != this) {
        vlogf(5, "%s killed by %s at %s (polyed as a %s)", desc->original->getName(), (t ? t->getName() : "a door"), roomp->getName(), getName());
      } else {
        vlogf(5, "%s killed by %s at %s", getName(), 
            (t ? t->getName() : "a door"), roomp->getName());
      }
      if (desc) 
        desc->career.deaths++;
    }
    DeleteHatreds(this, NULL);
    rc = die(damtype);
    if (IS_SET_ONLY(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return FALSE;
}

void perform_violence(int pulse)
{
  TBeing *ch, *vict;
  int rc;
  int tmp_pulse;

  // we come in here every combat round.

  // but on the round, simulate combat like if it were roundless
  // so that blows are spread around in a timed fashion

  // truly "roundless combat" can be done by removing this for loop, and
  // simply passing "pulse" (rather then "tmp_pulse" to hit()
  // oh yeah, have to make call to perform_violence done every "pulse" too
  // it was spammy and confusing as hell though.
  for (tmp_pulse=0; tmp_pulse < (PULSE_COMBAT / (TurboMode ? 2 : 1)); tmp_pulse++) {

    for (ch = gCombatList; ch; ch = gCombatNext) {
      gCombatNext = ch->next_fighting;
      if (!(vict = ch->fight())) {
        vlogf(10, "%s is not fighting in perform_violence!  *BUG BRUTIUS*", ch->getName());
        continue;
      }
      if (!ch->roomp) {
        ch->stopFighting();
        continue;
      }
      if (ch == vict) 
        ch->stopFighting();
      else {
        if (ch->awake() && ch->sameRoom(vict)) {
          vict = ch->fight();
          if (vict) {
            rc = ch->hit(vict, pulse + tmp_pulse);
            if (IS_SET_DELETE(rc, DELETE_VICT)) {
              vict->reformGroup();
              delete vict;
              vict = NULL;
              continue;
            } else if (IS_SET_DELETE(rc, DELETE_THIS)) {
              ch->reformGroup();
              delete ch;
              ch = NULL;
              break;
            }
          } else {
            vlogf(10, "do we ever get here");
          }
        } else { 
          // Not in same room or not awake 
          ch->stopFighting();
        }
      }
    }
  }
}

void TBeing::doAttack(const char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  attack_mode_t new_combat;

  one_argument(argument, arg);

  if (!*arg || is_abbrev(arg, "?")) {
    sendTo("You are in %s%s%s attack mode.\n\r",
        cyan(), attack_modes[getCombatMode()], norm());
    return;
  }
  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("You can't change that now that you are berserking!\n\r");
    return;
  }
  if (is_abbrev(arg, "offense")) {
    new_combat = ATTACK_OFFENSE;
  } else if (is_abbrev(arg, "defense")) {
    new_combat = ATTACK_DEFENSE;
  } else if (is_abbrev(arg, "normal")) {
    new_combat = ATTACK_NORMAL;
  } else if (is_abbrev(arg, "berserk")) {
    sendTo("Just type berserk if you want to go berserk...\n\r");
    return;
  } else {
    sendTo("Syntax : attack <mode>\n\r");
    return;
  }
  if (isCombatMode(new_combat)) {
    sendTo("You are already in attack mode: %s.\n\r", attack_modes[getCombatMode()]);
    return;
  }
  setCombatMode(new_combat);

  sendTo("Switching attack mode to %s.\n\r", attack_modes[getCombatMode()]);
  return;
}

// my share of the exp in a group kill
// should be same as the mob_exp curve, otherwise you either
// allow newbies to leech, or high levels drain too hard.
double TBeing::getExpShare() const
{
  return mob_exp(GetMaxLevel());
}

double TBeing::getExpSharePerc() const
{
  followData *f;
  double totalshares=0;
  
  if(master){
    f=master->followers;
    totalshares=master->getExpShare();
  } else {
    f=followers;
    totalshares=getExpShare();
  }

  for (; f; f = f->next){
    if(f->follower->isAffected(AFF_GROUP))
      totalshares+=f->follower->getExpShare();
  }

  return ((getExpShare()/totalshares)*100);
}

// FRACT: fraction of true xp gained if I am killing way below my level
// this only applies if i am 3+ levels over mob
// will always get 100-95 = 5% of real xp (13+ level diffrence)
static int FRACT(TBeing *ch, TBeing *v)
{
#if 0
  int diff_levs = ch->GetMaxLevel() - v->GetMaxLevel();
  int level_cut = 3;

  if (ch->hasClass(CLASS_WARRIOR) ||
      ch->hasClass(CLASS_DEIKHAN) ||
      ch->hasClass(CLASS_MONK))
    level_cut = 3;
  else if (ch->hasClass(CLASS_RANGER) ||
           ch->hasClass(CLASS_THIEF))
    level_cut = 4;
  else if (ch->hasClass(CLASS_CLERIC))
    level_cut = 5;
  else if (ch->hasClass(CLASS_MAGE))
    level_cut = 6;

  if (diff_levs <= level_cut)
    return 100;

  // scale the diff_levs based on level
  // what testing revealed was that a 10 lev diff at level 50 (50v40) was
  // equiv to a 7 level diff at level 20 (20v13)
  // I'm basically going to allow normalizing by adding (50-lev)/10 to
  // diff_levs.  (add 5 to do good rounding)
  diff_levs += (MAX_MORT - ch->GetMaxLevel() + 5)/10;

  diff_levs -= level_cut;
  diff_levs = max(0, diff_levs);

  return 100 - (min(95, diff_levs * diff_levs));
#else
  // the new exp curve doubles xp based on double the difficulty
  // this should mean that plowing (killing easy mobs for high fraction
  // of xp that mob of same level yields) is no longer possible.
  return 100;
#endif;
}

#define EXP_DEBUG      0 
void TBeing::gainExpPerHit(TBeing *v, double percent)
{
  double no_levels = 0;
  double tmp_exp;
  TBeing *real_master;
  followData *f;
  TBeing *tank;

  if (v->isPc())
    return;

  // Player is ok, go on and start 
  double exp = v->getExp();

  double exp_received = (exp * percent);

  int fract = FRACT(this, v);
#if EXP_DEBUG
  vlogf(3, "FRACT is %d", fract);
#endif

  // new formula (Cos)
  // find out who the tank is

  if (v->fight()) 
    tank = v->fight();
  else 
    tank = this;

  if (!isAffected(AFF_GROUP)) {
    if (v && tank && tank == this) {
      // I am tanking so give me experience
    } else if (!tank->isPc() && !tank->master) {
      // tank is a true npc not a pc, a pet, or a charmie
      // I deserve experience
    } else {
      // else no experience/take the mobs exp
#if EXP_DEBUG
      vlogf(3,"%s destroyed %d xp of %s's.", getName(), exp_received, v->getName());
#endif
      gain_exp(v, -exp_received );
      return;
    }

    // give the experience to solo pc and take from mob
#if EXP_DEBUG
    vlogf(3,"%s got %d.  perc  %f, %s lost %d",getName(),exp_received * fract / 100, percent, v->getName(), exp_received);
#endif
    gain_exp(this, exp_received * fract/ 100);
    gain_exp(v, -exp_received );
    return;

  } else { // grouped
    if (numberInGroupInRoom() == 1) {
      // I am the only groupmember in the room
      if (tank && (tank == this || inGroup(tank))) {
        // I am tanking so I can get experience.
      } else if (!tank->isPc() && !tank->master) {
        // tank is a true npc not a pc, a pet, or a charmie
        // I can get experience/grouped mobs are taken care of above
      } else {
        // else no experience for me
#if EXP_DEBUG
        vlogf(3,"%s destroyed %d xp of %s's.", getName(), exp_received, v->getName());
#endif
        gain_exp(v, -exp_received);
        return;
      }
      // give and take the experience
#if EXP_DEBUG
      vlogf(3,"%s got %d.  perc  %f, %s lost %d",getName(),exp_received * fract / 100, percent, v->getName(), exp_received);
#endif
      gain_exp(this, exp_received * fract/ 100);
      gain_exp(v, -exp_received );
      return;
    } else {  //more than one in my group in room
      if (tank && (tank == this || inGroup(tank))) {
        // my group is tanking so my group can get experience.
      } else if (!tank->isPc() && !tank->master){
        // tank is a true npc not a pc, a pet, or a charmie
        // my group can get experience
      } else {
        // else no experience for my groupmembers
        gain_exp(v, -exp_received);
        return;
      }
      // take the experience from the mob /split will take place below
#if EXP_DEBUG
      vlogf(3,"%s lost %d.  perc  %f, %s's group got exp",v->getName(),exp_received, percent, getName(), exp_received);
#endif
      gain_exp(v, -exp_received );
    }
  }
  // Anything below here is for splitting experience when there is more
  // than one of my group in the room.  Solo experience was given above
  // and cases that give no experiecne have already returned
  // Thus we know the Pc is grouped and has groupmates in room Cos
  // Figure up the group's total level, and use that for giving out exps

  if (!(real_master = master))
    real_master = this;

  if (inGroup(real_master) && sameRoom(real_master))
    no_levels = real_master->getExpShare();
  else
    no_levels = 0;

  for (f = real_master->followers; f; f = f->next) {
    if (inGroup(f->follower) && sameRoom(f->follower))
        no_levels += f->follower->getExpShare();
  }

#if EXP_DEBUG
    vlogf(5, "shares %d", no_levels);
#endif

  if (no_levels) 
    tmp_exp = (double) (exp_received / (double) no_levels);
  else
    tmp_exp = 0;

#if EXP_DEBUG
  vlogf(5, "one share %10.10f", tmp_exp);
#endif

  // Gain exp for master if in room with ch
  if (sameRoom(real_master) && inGroup(real_master)) {
    exp_received = (tmp_exp * real_master->getExpShare());
    gain_exp(real_master, exp_received * fract/ 100);
#if EXP_DEBUG
    vlogf(3,"%s got %d.  perc  %f, %s lost %d",real_master->getName(),exp_received * fract / 100, percent, v->getName(), exp_received);
#endif
  }
  // Gain exp for followers if in room with ch
  for (f = real_master->followers; f; f = f->next) {
    if (inGroup(f->follower) && sameRoom(f->follower)) {
      exp_received = (tmp_exp * f->follower->getExpShare());
      gain_exp(f->follower, exp_received * fract/ 100);
#if EXP_DEBUG
      vlogf(3,"%s got %d.  perc  %f, %s lost %d",f->follower->getName(),exp_received * fract / 100, percent, v->getName(), exp_received);
#endif
    }
  }
}

int TBeing::addToDistracted(int amt, int flags)
{
  if (!spelltask)
    return FALSE;

  if (!flags) {
    spelltask->distracted += amt;
    return spelltask->distracted;
  } else {
    vlogf(5, "Something is using uncoded flags in addToDistract");
    return -1;
  }
  return FALSE;
}

int TBeing::remFromDistracted(int amt)
{
  if (!spelltask)
   return FALSE;

  spelltask->distracted -= amt;
  spelltask->distracted = max(0, spelltask->distracted);
  return spelltask->distracted;
}

int TBeing::setDistracted(int amt, int flags)
{
  if (!spelltask)
    return FALSE;
  if (amt < 0) {
    vlogf(5, "something set a negative number to distract made 0 %s", getName());
    amt = 0;
  }
  if (!flags) {

    spelltask->distracted = amt;
    return spelltask->distracted;
  } else {
    vlogf(5,"Something is using uncoded flags in setDistract");
    return -1;
  }
  return FALSE;
}

int TBeing::clearDistracted()
{
  if (!spelltask)
   return TRUE;

  spelltask->distracted = 0;
  return TRUE;
}


int TBeing::getDistracted()
{
  if (!spelltask)
   return FALSE;

  if (spelltask->distracted < 0)
    spelltask->distracted = 0;
  return spelltask->distracted;
}


void TThing::remCastingList(TThing *obj)
{
  TBeing *ch = NULL;
  TBeing *tmp_ch = NULL;

  if (!obj->getCaster()) {
    return;
  }

  for (ch = obj->getCaster(); ch; ch = tmp_ch) {
    tmp_ch = ch->next_caster;

    if (!ch->spelltask) {
//      vlogf(5,"Somehow, %s object was being cast on by someone (%s) who had no spelltask.", obj->getName(), ch->getName());
      continue;
    }

    ch->spelltask->object = NULL;
    ch->next_caster = NULL;
    ch->stopCast(STOP_CAST_NOT_AROUND);
    ch->spelltask = NULL;
  }
  obj->setCaster(NULL);
}


void TBeing::remCastingList(TThing *v)
{
  TBeing *ch = NULL;
  TBeing *tmp_ch = NULL;

  if (!v->getCaster()) {
    return;
  }

  for (ch = v->getCaster(); ch; ch = tmp_ch) {
    tmp_ch = ch->next_caster;
    if (!ch->spelltask) { 
      vlogf(5,"Somehow, %s was being cast on by someone (%s) who had no spelltask.", v->getName(), ch->getName()); 
      continue;
    }
    if (!ch->spelltask->victim) { 
      vlogf(5,"Somehow, %s was being cast on by someone (%s) who had no casting target.", v->getName(), ch->getName()); 
    }
    ch->spelltask->victim = NULL;
    ch->next_caster = NULL;
    ch->stopCast(STOP_CAST_NOT_AROUND);
    ch->spelltask = NULL;
  }
  v->setCaster(NULL);
}

void TBeing::damageArm(bool isPrimary, int bit)
{
  if (isPrimary) {
    addToLimbFlags(getPrimaryArm(), bit);
    addToLimbFlags(getPrimaryHand(), bit);
//    addToLimbFlags(getPrimaryHold(), bit);
    addToLimbFlags(getPrimaryWrist(), bit);
    addToLimbFlags(getPrimaryFinger(), bit);
    woundedHand(TRUE);
  } else {
    addToLimbFlags(getSecondaryArm(), bit);
    addToLimbFlags(getSecondaryHand(), bit);
//    addToLimbFlags(getSecondaryHold(), bit);
    addToLimbFlags(getSecondaryWrist(), bit);
    addToLimbFlags(getSecondaryFinger(), bit);
    woundedHand(FALSE);
  }
}

// I couldn't figure out if this should have a weapon->canDrop() check or not
void TBeing::woundedHand(bool isPrimary)
{
  TThing *w;
  int bit, bit2;
  wearSlotT slot_hold = isPrimary ? getPrimaryHold() : getSecondaryHold();
  wearSlotT slot_hand = isPrimary ? getPrimaryHand() : getSecondaryHand();

  bit = isLimbFlags(slot_hold, (PART_USELESS | PART_MISSING | PART_BROKEN));
  bit2 = isLimbFlags(slot_hand, (PART_USELESS | PART_MISSING | PART_BROKEN));
  if ((w = equipment[slot_hold]) && (bit || bit2)) {
    act("$p falls from your hand.",TRUE,this,w,0,TO_CHAR, ANSI_RED);
    act("$n drops $s $o.",TRUE,this,w,0,TO_ROOM);
    *roomp += *unequip(slot_hold);
  }
}

// returns DELETE_VICT
int TBeing::dislodgeWeapon(TBeing *v, TThing *weapon, wearSlotT part)
{           
  char buf[160];
  int rc;

  mud_assert(v->slotChance(part), "No slotChance in dislodgeWeapon");

  if (weapon && !v->getStuckIn(part)) {
    string nameBuf = colorString(this, desc, pers(v), NULL, COLOR_MOBS, TRUE);

    sendTo(COLOR_OBJECTS, "Your %s, still stuck in %s's %s, is %storn from your grasp%s.\n\r",
       objn(weapon).c_str(), nameBuf.c_str(), 
       v->describeBodySlot(part).c_str(),red(),norm());
    v->sendTo(COLOR_OBJECTS, "%s%s leaves %s %s stuck in your %s%s.\n\r",
       v->orange(), good_cap(getName()).c_str(), hshr(),
       objn(weapon).c_str(),v->describeBodySlot(part).c_str(), v->norm());

    sprintf(buf,"$n leaves $s $o stuck in $N's %s", v->describeBodySlot(part).c_str());
    act(buf, TRUE, this, weapon, v, TO_NOTVICT);
    rc = v->stickIn(unequip(weapon->eq_pos), part, SILENT_YES);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;
  }
  return FALSE;
}

void TBeing::makePartMissing(wearSlotT slot, bool diseased)
{
  TThing *t;

  if (!hasPart(slot)) {
    vlogf(5,"BOGUS SLOT trying to be made PART_MISSING: %d on %s",
         slot, getName());
    return;
  }
  if (!roomp) {
    // bat 8-16-96, mob could be dead, this is a bug 
    vlogf(8, "!roomp for target (%s) of makePartMissing().", getName());
    return;
  }
  if (!diseased)
    makeBodyPart(slot);
  else
    makeDiseasedPart(slot);

  setLimbFlags(slot, PART_MISSING);

  if ((t = unequip(slot)))
    *roomp += *t;

  for (wearSlotT j=MIN_WEAR; j < MAX_WEAR; j++) {
    if (!hasPart(j))
      continue;
    if (!limbConnections(j)) {
      setLimbFlags(j, PART_MISSING);
      TThing *tmp = unequip(j);
      if (tmp)
        *roomp += *tmp;
    }
  }

  // check for damage to both hands
  woundedHand(TRUE);
  woundedHand(FALSE);
}

bool bluntType(spellNumT wtype)
{
  switch (wtype) {
    case TYPE_SMASH:
    case TYPE_CRUSH:
    case TYPE_BLUDGEON:
    case TYPE_SMITE:
    case TYPE_HIT:
    case TYPE_FLAIL:
    case TYPE_PUMMEL:
    case TYPE_THRASH:
    case TYPE_THUMP:
    case TYPE_WALLOP:
    case TYPE_BATTER:
    case TYPE_BEAT:
    case TYPE_STRIKE:
    case TYPE_POUND:
    case TYPE_CLUB:
    case TYPE_WHIP:
    case TYPE_MAUL:
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}

bool TObj::isBluntWeapon() const
{
  return FALSE;
}

bool slashType(spellNumT wtype)
{
  switch (wtype) {
    case TYPE_CLAW:
    case TYPE_SLASH:
    case TYPE_CLEAVE:
    case TYPE_SLICE:
    case TYPE_BEAR_CLAW:
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}

bool pierceType(spellNumT wtype)
{
  switch (wtype) {
    case TYPE_PIERCE:
    case TYPE_STAB:
    case TYPE_STING:
    case TYPE_BITE:
    case TYPE_THRUST:
    case TYPE_SPEAR:
    case TYPE_BEAK:
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}

spellNumT TBeing::monkDamType() const
{
  if (!hasClass(CLASS_MONK) || !doesKnowSkill(SKILL_KUBO))
    return TYPE_HIT;

  double value;
  value = 3 * getSkillValue(SKILL_KUBO);
  value /= 2;
  value += getLevel(MONK_LEVEL_IND);
  value /= 4;  // 1-50 based 

  if(value<1) return TYPE_POUND;

  switch((int) value){
    case 1:  case 2:  case 3:                    return TYPE_POUND;
    case 4:  case 5:  case 6:                    return TYPE_BLUDGEON;
    case 7:  case 8:  case 9:                    return TYPE_FLAIL;
    case 10: case 11: case 12: case 13: case 14: return TYPE_BEAT;
    case 15: case 16: case 17: case 18: case 19: return TYPE_SMASH;
    case 20: case 21: case 22: case 23: case 24: return TYPE_PUMMEL;
    case 25: case 26: case 27: case 28: case 29: return TYPE_THUMP;
    case 30: case 31: case 32: case 33: case 34: return TYPE_THRASH;
    case 35: case 36: case 37:                   return TYPE_BATTER;
    case 38: case 39: case 40:                   return TYPE_SMITE;
    case 41: case 42: case 43:                   return TYPE_CRUSH;
    case 44: case 45: case 46:                   return TYPE_STRIKE;
    case 47: case 48: case 49:                   return TYPE_WALLOP;
    case 50: default:                            return TYPE_MAUL;
  }
}


void TBeing::reformGroup()
{
  followData *tmp, *tmp2;
  TBeing *new_master = NULL, *survivor = NULL;
  bool found = FALSE;

  if (!followers || master)
    return;

  for (tmp = followers; tmp; tmp = tmp2) {
    tmp2 = tmp->next;

    // exclude pure mobs-- allow poly and linkdead poly
    // (but if the original leader was a mob, it's a mob group - Peel)
    if ((!tmp->follower->isPc() && isPc()) && 
        tmp->follower->polyed == POLY_TYPE_NONE) {
      tmp->follower->stopFollower(TRUE);
      continue;
    }

    if (!tmp->follower->inGroup(this)) {
      tmp->follower->stopFollower(TRUE);
      continue;
    }

    if (!found) {
      new_master = tmp->follower;
      new_master->stopFollower(TRUE, STOP_FOLLOWER_CHAR_VICT);
      found = TRUE;
      new_master->master = NULL;

      if (!new_master->isAffected(AFF_GROUP))
        SET_BIT(new_master->specials.affectedBy, AFF_GROUP);

      new_master->sendTo(COLOR_BASIC, "<R>%s has died and you have taken over leadership of the group.<z>\n\r", getName());
      continue;
    }
    survivor = tmp->follower;
    survivor->stopFollower(TRUE, STOP_FOLLOWER_CHAR_VICT);
    new_master->addFollower(survivor, 1);

    if (!survivor->isAffected(AFF_GROUP))
      SET_BIT(survivor->specials.affectedBy, AFF_GROUP);

    survivor->sendTo(COLOR_BASIC, "<R>%s has died and %s has taken over leadership of the group.<z>\n\r", getName(), new_master->getName());
  }

// ADDING in linkDeads, polyed mobs and Immortals after finding a new leader
// If no good newleader, use an immortal, polyed mob or linkdead

  if (!followers || !followers->follower) {
    return;
  }

  survivor = NULL;
  for (tmp = followers; tmp; tmp = tmp2) {
    tmp2 = tmp->next;

    if (!new_master) {
      new_master = tmp->follower;
      new_master->stopFollower(TRUE, STOP_FOLLOWER_CHAR_VICT);
      found = TRUE;
      new_master->master = NULL;
      if (!new_master->isAffected(AFF_GROUP))
        SET_BIT(new_master->specials.affectedBy, AFF_GROUP);
      new_master->sendTo(COLOR_BASIC, "<R>%s has died and you have taken over leadership of the group.<z>\n\r", getName());
      continue;
    }
   
    survivor = tmp->follower;
    survivor->stopFollower(TRUE, STOP_FOLLOWER_CHAR_VICT);
    new_master->addFollower(survivor, 1);

    if (!survivor->isAffected(AFF_GROUP))
      SET_BIT(survivor->specials.affectedBy, AFF_GROUP);
    survivor->sendTo(COLOR_BASIC, "<R>%s has died and %s has taken over leadership of the group.<z>\n\r", getName(), new_master->getName());
  }
 
  return;
}

void TBeing::genericKillFix(void)
{
  reformGroup();
  DeleteHatreds(this, NULL);
  DeleteFears(this, NULL);

  // we are basically already dead when this gets called.
  // But, because of the spellWearOff, we could die "again"
  // one example would be slay a charm mob.
  int rc = generic_dispel_magic(NULL, this, MAX_IMMORT, IMMORTAL_YES, SAFE_YES);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    forceCrash("Multiple deaths (%s in room %d) occurred!",
         getName(), inRoom());
  }

  if (getCond(THIRST) >= 0)
   setCond(THIRST, 20);
  if (getCond(FULL) >= 0)
    setCond(FULL, 20);
  setMove(moveLimit());

  if (roomp) {
    if (isPc() && !roomp->isRoomFlag(ROOM_ARENA)) {
      for (wearSlotT i = MIN_WEAR; i < MAX_WEAR; i++) {
        setCurLimbHealth(i, getMaxLimbHealth(i));
        setLimbFlags(i, 0);
        setStuckIn(i, NULL);
      }
    }
  }
  affectedData *aff, *aff2;
  // we don't kill diseases on mobs just to avoid spam.
  if (isPc() && affected) {
    for (aff = affected; aff; aff = aff2) {
      aff2 = aff->next;
      if (aff->type == AFFECT_DISEASE) {
        diseaseStop(aff);
        affectRemove(aff);
      }
    }
  }
  if (isPc()) {
    setHit(1);
    if (riding)
      dismount(POSITION_SITTING);
    setPosition(POSITION_STANDING);
  }

  // dump any outstanding commands
  if (desc)
    desc->flushInput();
}

double TBeing::deathExp()
{
  double amt;
  // 4.1        : 30 * and mini(xp/4, amt)
  // 4.5 (beta) : 50 * and mini(4*xp/10, amt)
  amt = 30.0 * mob_exp((float) GetMaxLevel());
  amt = min( 1*getExp()/4,  amt);

  return amt;
}

double TBeing::deathSkillLoss()
{
  int amt, preskill=0;
  double loss=0, count=0;

  if (isImmortal() || dynamic_cast<TMonster *>(this) || !desc) {
    return FALSE;
  }

  spellNumT i;
  for (i = MIN_SPELL; i < MAX_SKILL;i++) {
    if ((!discArray[i]) || 
	(!*discArray[i]->name) || 
	(discArray[i]->startLearnDo == -1) ||
	(!doesKnowSkill(i)) ||
	(!getSkill(i))){
      continue;
    }

    if((amt = preskill = getRawNatSkillValue(i)) <= 5){
      continue;
    }

    amt-=(int)(5*sin((amt+5)/23)+5);

    loss += getRawNatSkillValue(i)-amt;
    count++;

    setSkillValue(i, max(amt, 1));
    setNatSkillValue(i, amt);

    vlogf(0, "skill loss %s (%d) = %d,  %d->%d", 
	  discArray[i]->name, i, preskill-amt,
	  preskill, getRawNatSkillValue(i));
  }
  affectTotal();

  return(count?loss/count:0);
}

bool TBeing::willKill(TBeing *v, int dam, spellNumT dmg_type, bool reduced)
{
  if (!reduced) {
    dam = getActualDamage(v, NULL, dam, dmg_type);
  }

  // account for protection
  dam *= 100 - v->getProtection();
  dam = (dam + 50) / 100;

  if (!dam)
    return false;

  return (v->getHit() - dam < -10);
}

void TBeing::setCombatMode(attack_mode_t n)
{
  combatMode = n;
  if (desc && desc->client) {
    if (n == ATTACK_OFFENSE)
      desc->clientf("%d", CLIENT_OFFENSIVE);
    else if (n == ATTACK_DEFENSE)
      desc->clientf("%d", CLIENT_DEFENSIVE);
    else if (n == ATTACK_BERSERK)
      desc->clientf("%d", CLIENT_BERSERK);
    else
      desc->clientf("%d", CLIENT_NORMAL);
  }
}

attack_mode_t TBeing::getCombatMode() const
{
  return combatMode;
}

bool TBeing::isCombatMode(attack_mode_t n) const
{
  return combatMode == n;
}

