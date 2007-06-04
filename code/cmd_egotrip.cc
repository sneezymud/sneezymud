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
#include "disc_spirit.h"
#include "disease.h"
#include "obj_portal.h"


class ego_imm_blessing {
public:
  sstring name;
  spellNumT aff_type;
  applyTypeT prim_apply;
  sstring msg;

  // vector stuff
  ego_imm_blessing(sstring n, spellNumT a, applyTypeT p, sstring m) :
    name(n), aff_type(a), prim_apply(p), msg(m){}

  ego_imm_blessing() {}
};


map <spellNumT,ego_imm_blessing> init_ego_imm_blessing()
{
  map <spellNumT,ego_imm_blessing> blessings;
  
  blessings[AFFECT_IMMORTAL_BLESSING]=
    ego_imm_blessing("immortal",
		     AFFECT_IMMORTAL_BLESSING,
		     APPLY_SPELL_HITROLL,
		     "immortals");

  blessings[AFFECT_PEEL_BLESSING]=
    ego_imm_blessing("Peel",
		     AFFECT_PEEL_BLESSING,
		     APPLY_SPE, 
		     "<r>speed<1>");
  blessings[AFFECT_VASCO_BLESSING]=
    ego_imm_blessing("Vasco",
		     AFFECT_VASCO_BLESSING,
		     APPLY_DEX,
		     "<k>stealth<1>");
  blessings[AFFECT_DASH_BLESSING]=
    ego_imm_blessing("Dash",
		     AFFECT_DASH_BLESSING,
		     APPLY_FOC,
		     "<B>reflection<1>"); // Steal my blessing again, and I'll break you. -D
  blessings[AFFECT_ANGUS_BLESSING]=
    ego_imm_blessing("Angus",
		     AFFECT_ANGUS_BLESSING,
		     APPLY_WIS,
		     "<g>wisdom<1>");
  blessings[AFFECT_DAMESCENA_BLESSING]=
    ego_imm_blessing("Damescena",
		     AFFECT_DAMESCENA_BLESSING,
		     APPLY_CON,
		     "<W>healing<1>");
  blessings[AFFECT_JESUS_BLESSING]=
    ego_imm_blessing("Jesus",
		     AFFECT_JESUS_BLESSING,
		     APPLY_STR,
		     "<w>power<1>");
  blessings[AFFECT_BUMP_BLESSING]=
    ego_imm_blessing("Bump",
		     AFFECT_BUMP_BLESSING,
		     APPLY_AGI,
		     "<W>flexibility<1>");
  blessings[AFFECT_MAROR_BLESSING]=
    ego_imm_blessing("Maror",
		     AFFECT_MAROR_BLESSING,
		     APPLY_KAR,
		     "<Y>luck<1>");
  blessings[AFFECT_CORAL_BLESSING]=
    ego_imm_blessing("Coral",
		     AFFECT_CORAL_BLESSING,
		     APPLY_CON,
		     "<c>exploration<1>");

	blessings[AFFECT_DEIRDRE_BLESSING]=
		ego_imm_blessing("Deirdre",
		AFFECT_DEIRDRE_BLESSING,
		APPLY_KAR,
		"<o>the rabbit<1>");
	blessings[AFFECT_GARTHAGK_BLESSING]=
		ego_imm_blessing("Garthagk",
		AFFECT_GARTHAGK_BLESSING,
		APPLY_WIS,
		"<g>wisdom<1>");
	blessings[AFFECT_MERCURY_BLESSING]=
		ego_imm_blessing("Mercury",
		AFFECT_MERCURY_BLESSING,
		APPLY_BRA,
		"<r>resilience<1>");
	blessings[AFFECT_METROHEP_BLESSING]=
		ego_imm_blessing("Metrohep",
		AFFECT_METROHEP_BLESSING,
		APPLY_STR,
		"<k>the hippo<1>");
	blessings[AFFECT_MAGDALENA_BLESSING]=
		ego_imm_blessing("Magdalena",
		AFFECT_MAGDALENA_BLESSING,
		APPLY_BRA,
		"<k>hard rock<1>");

  return blessings;
}

void egoAffect(TBeing *c, TBeing *v, spellNumT which, int level)
{
  affectedData aff;
  map <spellNumT,ego_imm_blessing> blessings=init_ego_imm_blessing();
  bool success=false;

  aff.level=level;
  aff.duration=(1+level)*UPDATES_PER_MUDHOUR;

  // each imm blessing is a spell affect + a stat modifier
  // some of the spell affects are set in TBeing::affectedBySpell

  // apply stat modifier first
  aff.type=which;
  aff.location=blessings[which].prim_apply;
  aff.modifier=13;
  aff.modifier2=0;
  aff.bitvector=0;
  success=v->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES);

  // now do the second custom part
  if(which==AFFECT_IMMORTAL_BLESSING){
    // default non-custom blessing
    aff.type=AFFECT_IMMORTAL_BLESSING;
    aff.location=APPLY_IMMUNITY;
    aff.modifier=IMMUNE_NONMAGIC;
    aff.modifier2=5;
    aff.bitvector=0;
    v->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES);
  } else if(which==AFFECT_PEEL_BLESSING){
    // TBeing::affectedBySpell - haste
  } else if(which==AFFECT_VASCO_BLESSING){
    aff.type = AFFECT_VASCO_BLESSING;
    aff.modifier = -40;
    aff.modifier2=0;
    aff.location = APPLY_NOISE;
    aff.bitvector = 0;
    v->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES);
  } else if(which==AFFECT_CORAL_BLESSING){
    aff.type=AFFECT_CORAL_BLESSING;
    aff.location=APPLY_NONE;
    aff.modifier=0;
    aff.modifier2=0;
    aff.bitvector = AFF_LEVITATING;
    v->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES);
  } else if(which==AFFECT_ANGUS_BLESSING){
    // TBeing::affectedBySpell - sanc
  } else if(which==AFFECT_DAMESCENA_BLESSING){
    // TBeing::affectedBySpell - enliven
  } else if(which==AFFECT_JESUS_BLESSING){
    // TBeing::affectedBySpell - true sight
  } else if(which==AFFECT_BUMP_BLESSING){
    // none
  } else if(which==AFFECT_MAROR_BLESSING){
    aff.type=AFFECT_MAROR_BLESSING;
    aff.location=APPLY_CRIT_FREQUENCY;
    aff.modifier = 2;
    aff.modifier2 = 0;
    aff.bitvector = 0;
    v->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES);
  } else if(which==AFFECT_DASH_BLESSING){
    // TBeing::affectedBySpell - plasma mirror
  } else if(which==AFFECT_DEIRDRE_BLESSING){
    // TBeing::affectedBySpell - celerite
  } else if(which==AFFECT_GARTHAGK_BLESSING){
    // TBeing::affectedBySpell - sanct
  } else if(which==AFFECT_MERCURY_BLESSING){
    // TBeing::affectedBySpell - enliven
  } else if(which==AFFECT_MAGDALENA_BLESSING){
    // TBeing::affectedBySpell - stoneskin
  } else if(which==AFFECT_GARTHAGK_BLESSING){
  } else if(which==AFFECT_METROHEP_BLESSING){
	aff.type = AFFECT_METROHEP_BLESSING;
    aff.location = APPLY_ARMOR;
	aff.modifier = -90;
	aff.modifier2 = 0;
    aff.bitvector = 0;
	v->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES);		  
 }

  affectedData *afp;

  // now, each time a blessing is applied, increase the power of all
  // the other blessings.
  // EXCEPTION: not Maror's blessing -- it is a multipler and gets too high
  // ANOTHER EXCEPTION: not the generic immunity blessing - this would change which
  //                    immunity is applied, not the strength of it.
  // ** this only appears to increase the generic blessing nonmagic immunity
  // and vasco's noise modifier -- is that the intention?  almost irrelevant
  // d: this should only increase the primary stat effects, not the 2ndary stuff

  if(success){
    for(afp = v->affected; afp; afp = afp->next){
      if( (afp->type==AFFECT_IMMORTAL_BLESSING && afp->location != APPLY_IMMUNITY) ||
	 afp->type==AFFECT_ANGUS_BLESSING ||
	 afp->type==AFFECT_PEEL_BLESSING ||
	 (afp->type==AFFECT_VASCO_BLESSING && afp->location != APPLY_NOISE) ||
	 afp->type==AFFECT_CORAL_BLESSING ||
	 afp->type==AFFECT_DAMESCENA_BLESSING ||
	 afp->type==AFFECT_JESUS_BLESSING ||
	 afp->type==AFFECT_BUMP_BLESSING ||
         afp->type==AFFECT_DASH_BLESSING ||
         afp->type==AFFECT_DEIRDRE_BLESSING ||
         afp->type==AFFECT_GARTHAGK_BLESSING ||
         afp->type==AFFECT_MERCURY_BLESSING ||
         afp->type==AFFECT_MAGDALENA_BLESSING ||
         (afp->type==AFFECT_METROHEP_BLESSING && afp->location != APPLY_ARMOR) ||
   (afp->type==AFFECT_MAROR_BLESSING && afp->location != APPLY_CRIT_FREQUENCY) ){
	afp->modifier =(int)((float)afp->modifier * 1.5);
	
	// increase the new spell too, but don't announce it - redundant
	if(afp->type != which){
	  v->sendTo(COLOR_SPELLS,fmt("...it increases the power of %s's blessing!\n\r")%
		    blessings[afp->type].name);
	}
      }
    }
  }

}

	


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
  badsyn += "crit - forces a target mob to do the number crit if fighting\n\r";
  badsyn += "portal - creates a portal to the target mob/player\n\r";
  badsyn += "teleport - teleports the targeted mob/player, ignoring room flags\n\r";
  badsyn += "disease <target> <disease> - makes someone feel bad.\n\r";

  sstring argument, sarg = arg, restarg;
  restarg = one_argument(sarg, argument);
  if (!argument.length()) {
    sendTo(badsyn);
    return;
  }
  if (is_abbrev(argument, "disease")) {
    sstring target, disease;
    restarg = one_argument(restarg, target);
    restarg = one_argument(restarg, disease);
    if (target.empty() || disease.empty()) {
      sendTo("Syntax:\n\r     egotrip disease <target> <disease>\n\r");
      sendTo("Viable cruelties include:\n\r");
      sendTo(COLOR_OBJECTS, "     <c>cold<1>, <c>dysentery<1>, <c>flu<1>, <c>pneumonia<1>, <c>leprosy<1>, <c>gangrene<1>, <c>plague<1> & <c>scurvy<1>\n\r");
      return;
    }
    TBeing *sufferer = get_char_vis_world(this, target, NULL, EXACT_NO);
    if (!sufferer) {
      sendTo("Could not locate the ersatz sufferer.\n\r");
      sendTo("Syntax:\n\r     egotrip disease <target> <disease>\n\r");
      return;
    }
    
    if (sufferer->isImmune(IMMUNE_DISEASE, WEAR_BODY)) {
      sendTo("Bummer, they're immune.\n\r");
      return;
    }
    
    affectedData aff;
    aff.type = AFFECT_DISEASE;
    aff.level = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    aff.duration = min((int) sufferer->GetMaxLevel(), (int) GetMaxLevel()) * UPDATES_PER_MUDHOUR / 3;
    aff.modifier2 = sufferer->GetMaxLevel();
    
    if (is_abbrev(disease, "cold")) {
      aff.modifier = DISEASE_COLD;
    } else if (is_abbrev(disease, "dysentery")) {
      aff.modifier = DISEASE_DYSENTERY;
    } else if (is_abbrev(disease, "flu")) {
      aff.modifier = DISEASE_FLU;
    } else if (is_abbrev(disease, "pneumonia")) {
      aff.modifier = DISEASE_PNEUMONIA;
    } else if (is_abbrev(disease, "leprosy")) {
      aff.modifier = DISEASE_LEPROSY;
    } else if (is_abbrev(disease, "gangrene")) {
      aff.modifier = DISEASE_GANGRENE;
      // find a random slot for it
      wearSlotT slot;
      bool found = false; // need to make sure this doesn't loop 4ever, right?
      for (int i = 0; i < 20; ++i) {
        slot = pickRandomLimb();
        if (notBleedSlot(slot))
          continue;
        if (!sufferer->hasPart(slot))
          continue;
        if (sufferer->isLimbFlags(slot, PART_GANGRENOUS))
          continue;
        if (sufferer->isImmune(IMMUNE_DISEASE, slot))
         continue;
        found = TRUE;
        break;
      }
      if (!found) {
        sendTo("Hm... could not find an available slot for gangrene.\n\r");
        sendTo("Keep trying if you're sure they have one left...\n\r");
        return;
      }
      aff.level = slot;
    } else if (is_abbrev(disease, "plague")) {
      aff.modifier = DISEASE_PLAGUE;
    } else if (is_abbrev(disease, "scurvy")) {
      aff.modifier = DISEASE_SCURVY;
    } else {
      sendTo("Syntax:\n\r     egotrip disease <target> <disease>\n\r");
      sendTo("Viable cruelties include:\n\r");
      sendTo(COLOR_OBJECTS, "     <c>cold<1>, <c>dysentery<1>, <c>flu<1>, <c>pneumonia<1>, <c>leprosy<1>, <c>gangrene<1>, <c>plague<1> & <c>scurvy<1>\n\r");
      return;
    }
    if (sufferer->hasDisease((diseaseTypeT) aff.modifier)) {
      sendTo("Bummer, they already have that one.\n\r");
      return;
    }
    // make leprosy & gangrene permanent
    if (aff.modifier == DISEASE_LEPROSY || aff.modifier == DISEASE_GANGRENE) {
      aff.duration = PERMANENT_DURATION;
    } else {
      // we've already applied a raw immunity check to prevent entirely
      // however, let immunities also decrease duration
      aff.duration *= (100 - sufferer->getImmunity(IMMUNE_DISEASE));
      aff.duration /= 100;
    }
    act("You breathe a fetid cloud into $N's body.", TRUE, this, 0, sufferer, TO_CHAR);
    act("Someone around here doesn't like you.", TRUE, this, 0, sufferer, TO_VICT);
    act("Someone around here doesn't like $n.", TRUE, sufferer, 0, this, TO_ROOM);
    sufferer->affectTo(&aff);
    disease_start(sufferer, &aff);
    return;
    
  } else if(is_abbrev(argument, "teleport")){
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
    map <spellNumT,ego_imm_blessing> blessings=init_ego_imm_blessing();
    map <spellNumT,ego_imm_blessing>::iterator iter;
    bool found=false;
    for (d = descriptor_list; d; d = d->next) {
      if (d->connected != CON_PLYNG)
        continue;

      TBeing *ch = d->character;

      // Try and ditch some of the un-needed spam/waste.
      if (!ch || ch->GetMaxLevel() > MAX_MORT)
        continue;

      for(iter=blessings.begin();iter!=blessings.end();++iter){
	if(!strcmp(getName(), (*iter).second.name.c_str())){
	  ch->sendTo(COLOR_SPELLS, fmt("%s has bestowed upon you %s blessing of %s.\n\r") %
		     sstring(ch->pers(this)).cap() % hshr() %
		     (*iter).second.msg);
	  egoAffect(this, ch, (*iter).first, 5);
	  found=true;
	}
      }
      if(!found){
	// default blessing
	ch->sendTo(COLOR_SPELLS,fmt("%s has graciously bestowed upon you %s blessing.\n\r") %
		   sstring(ch->pers(this)).cap() % hshr());
	egoAffect(this, ch, AFFECT_IMMORTAL_BLESSING, 5);
      }
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
