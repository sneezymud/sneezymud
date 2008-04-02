//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      periodic.cc : functions that are called periodically by the game
//  
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "combat.h"
#include "disease.h"
#include "mail.h"
#include "disc_monk.h"
#include "obj_component.h"
#include "obj_drug.h"
#include "obj_player_corpse.h"
#include "obj_opal.h"
#include "obj_seethru.h"
#include "obj_plant.h"
#include "obj_egg.h"
#include "obj_trash_pile.h"
#include "process.h"
#include "obj_pool.h"
#include "database.h"
#include "obj_light.h"

// procGlobalRoomStuff
procGlobalRoomStuff::procGlobalRoomStuff(const int &p)
{
  trigger_pulse=p;
  name="procGlobalRoomStuff";
}


void procGlobalRoomStuff::run(int pulse) const
{
  int i;
  TRoom *rp;
  TTrashPile *pile;
  int trash_count=0, fire_count=0, water_count=0;
  TObj *o;
  TPool *pool;

  for (i = 0; i < WORLD_SIZE; i++) {
    rp = real_roomp(i);
    if (!rp)
      continue;
    
    if(::number(0,3)){
      if(rp->isRoomFlag(ROOM_ON_FIRE)){
	// alert firemen if needed
	if(rp->inGrimhaven() && !fireInGrimhaven){
	  TRoom *rp;
	  for(unsigned int zone = 0; zone < zone_table.size(); zone++) {
	    if((rp=real_roomp(zone_table[zone].top)) && rp->inGrimhaven()){
	      zone_table[zone].sendTo("<R>Loud firebells begin clanging in the distance!<1>\n\r", 4673);
	    }
	  }
	  
	  if((rp=real_roomp(4673))){
	    rp->sendTo(COLOR_BASIC, "<R>The firebells begin to clang hysterically!<1>\n\r");
	  }
	  fireInGrimhaven=true;
	}

	// spread fire to adjacent rooms
	// we really need a getRandomExit function or something
	dirTypeT dir = dirTypeT(::number(MIN_DIR, MAX_DIR-1));
	TRoom *spread_to;
	if((rp->exitDir(dir) && 
	    (spread_to=real_roomp(rp->exitDir(dir)->to_room)) &&
	    !(rp->exitDir(dir)->condition & EX_CLOSED) && 
	    !spread_to->isWaterSector())){
	  spread_to->setRoomFlagBit(ROOM_ON_FIRE);
	}
      }
    }

    if(!::number(0,9)){
      trash_count=fire_count=water_count=0;
      for(TThing *t=rp->getStuff();t;t=t->nextThing){
	o=dynamic_cast<TObj *>(t);

	// count volume on liquids
	if((pool=dynamic_cast<TPool *>(t)))
	  water_count+=pool->getDrinkUnits();

	// count volume on fire
	if(o && o->isObjStat(ITEM_BURNING) &&
	   material_nums[o->getMaterial()].flammability)
	  fire_count+=o->getVolume();

	// count trash
	if(trash_count>=0 && o && o->isTrash())
	  trash_count++;
	
	// don't create any trash piles if we already have one
	if(dynamic_cast<TTrashPile *>(t))
	  trash_count=-1;
      }

      // not enough water in room, so no more flooding
       if(water_count < ROOM_FLOOD_THRESHOLD)
	rp->removeRoomFlagBit(ROOM_FLOODED);
      else
	rp->setRoomFlagBit(ROOM_FLOODED);

      // not much burning in room, so let fire die out
      if(fire_count < ROOM_FIRE_THRESHOLD)
	rp->removeRoomFlagBit(ROOM_ON_FIRE);
      else
	rp->setRoomFlagBit(ROOM_ON_FIRE);
      
      // trash pile creation
      if(trash_count >= 9){
	o=read_object(GENERIC_TRASH_PILE, VIRTUAL);
	if(!(pile=dynamic_cast<TTrashPile *>(o))){
	  vlogf(LOG_BUG, "generic trash pile wasn't a trash pile!");
	  delete o;
	} else {
	  *rp += *pile;
	}
      }

    }
    
    // weather noise
    if (rp->getWeather() == WEATHER_LIGHTNING) {
      if (!::number(0,9)) {
        TThing *in_room;

        soundNumT snd = pickRandSound(SOUND_THUNDER_1, SOUND_THUNDER_4);
        for (in_room = rp->getStuff(); in_room; in_room = in_room->nextThing) {
          TBeing *ch = dynamic_cast<TBeing *>(in_room);
          if (!ch || !ch->desc)
            continue;

          act("A flash of lightning illuminates the land.", 
	      FALSE, ch, 0, 0, TO_CHAR, ANSI_YELLOW);
          ch->playsound(snd, SOUND_TYPE_NOISE);
        } 
      }
    }
  }
}



// procDeityCheck
procDeityCheck::procDeityCheck(const int &p)
{
  trigger_pulse=p;
  name="procDeityCheck";
}

void procDeityCheck::run(int pulse) const
{
// this function gets called ever 120 pulse (30 secs?)
// it should randomly load a deity and/or extract extra deitys
// based on various condition
  // deities are currently whacky, disable auto-load for time being.
  // Bat 9/9/98
  return;
#if 0
  TBeing *deity;
  int num_d = mob_index[real_mobile(MOB_DEITY_JUDGMENT)].number;
  int num_t = obj_index[real_object(DEITY_TOKEN)].number;


  if (num_d <= 0) {
    if (num_t <= 0) {
      // no tokens, no deitys in game.  chance put in game 
      if (!number(0,300)) {
        deity = read_mobile(MOB_DEITY_JUDGMENT, VIRTUAL);
        thing_to_room(deity,ROOM_IMPERIA);
      }
      return;
    }
    // token in game, do not load 
  } else {
    // there's already a deity somewhere 
    // chance of getting rid of extra deity 
    if (!number(0,200)) {
      for (deity = character_list; deity; deity = deity->next) {
        if (deity->mobVnum() == MOB_DEITY_JUDGMENT)
          break;
      }
      if (!deity) {   
        vlogf(LOG_BUG,"Big 'Ol bug in deityCheck()");
        return;
      }
      vlogf(LOG_BUG,fmt("Purging the deity in room #%d") % deity->in_room);
      delete deity;
      deity = NULL;
    }
  }
  return;
#endif
}

// procApocCheck
procApocCheck::procApocCheck(const int &p)
{
  trigger_pulse=p;
  name="procApocCheck";
}

void procApocCheck::run(int pulse) const
{
  int num, rc;
  TMonster *mob;

  if(gamePort == BUILDER_GAMEPORT)
    return;

  if (number(0,350)) 
    return;

  // these are in biblical order, shrug, seems to make sense
  if ((num = real_mobile(APOC_PESTILENCE)) && mob_index[num].getNumber())
    if ((num = real_mobile(APOC_WAR)) && mob_index[num].getNumber())
      if ((num = real_mobile(APOC_FAMINE)) && mob_index[num].getNumber())
        if ((num = real_mobile(APOC_DEATH)) && mob_index[num].getNumber())
          return;

  if (!num)
    return;

  if (!(mob = read_mobile(num, REAL))) {
    vlogf(LOG_BUG, "Bad mob in apocCheck");
    return;
  }  
  thing_to_room(mob, ROOM_IMPERIA);
  rc = mob->genericTeleport(SILENT_NO);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete mob;
    mob = NULL;
    return;
  }
  //vlogf(LOG_MISC,"Loading a horseman into the game.");
}

// procCallRoomSpec
procCallRoomSpec::procCallRoomSpec(const int &p)
{
  trigger_pulse=p;
  name="procCallRoomSpec";
}

void procCallRoomSpec::run(int pulse) const
{
// this simplifies room specials since we have a huge number of rooms
// and only a handful have specs
  unsigned int i;

  for (i = 0; i < roomspec_db.size(); i++) {
    TRoom *rp = roomspec_db[i];
    if (rp){
      rp->checkSpec(NULL, CMD_GENERIC_PULSE, NULL, rp);
    }
  }
}

// procDoRoomSaves
procDoRoomSaves::procDoRoomSaves(const int &p)
{
  trigger_pulse=p;
  name="procDoRoomSaves";
}

void procDoRoomSaves::run(int pulse) const
{
  // this is not a very good implementation of this idea, but I don't have time
  // to work on it - this is an emergency patch for a dupe bug
  unsigned int i;
  TRoom *rp=NULL;

  for (i = 0; i < roomsave_db.size(); i++) {
    // usually, you'll get several saves queued up in a row,
    // so skip the duplicates
    if(rp == roomsave_db[i])
      continue;

    rp = roomsave_db[i];

    if (rp){
      rp->saveItems("");
    }
  }

  roomsave_db.clear();
}


// returns DELETE_THIS
int TBeing::riverFlow(int)
{
  int was_in = inRoom(), rc, resCode = 0;
  TRoom *to_room;
  char buf[256];
  TThing *t, *t2;

  if (!roomp) 
    return FALSE;

  // I won't float unless my mount does
  if (riding)
    return FALSE;   

  // lets skip floating for generic mobs
  if (!isPc() && (!rider || !rider->isPc()))
    return FALSE; 

  // no water = no float
  if (!roomp->isUnderwaterSector() && !roomp->isWaterSector()) 
    return FALSE;

  // still water
  if (roomp->getRiverSpeed() <= 0) 
    return FALSE;

  if (::number(0,roomp->getRiverSpeed()) &&
      ::number(0,roomp->getRiverSpeed())) {
    return FALSE;
  }

  dirTypeT rd = roomp->getRiverDir();
  if ((rd < MIN_DIR) || (rd >= MAX_DIR)) {
    vlogf(LOG_BUG, fmt("Illegal river direction (%d) in room %d") %  rd % inRoom());
    return FALSE;
  }

  if (!isSwimming())
    return FALSE;

  if (isImmortal()) {
    act("The waters swirl and eddy about you.",
                   FALSE, this, NULL, NULL, TO_CHAR);
    return FALSE;
  }

  if (!exitDir(rd) || (exitDir(rd)->to_room == ROOM_NOWHERE) ||
      IS_SET(exitDir(rd)->condition, EX_CLOSED))
    return FALSE;

  if ((rc = canSwim(rd)) > 0) {
    sendTo(fmt("You swim %s against the current.\n\r") % (isAffected(AFF_SWIM) ? "effortlessly" : "valiantly"));
    return FALSE;
  }

  if (!(to_room = real_roomp(exitDir(rd)->to_room)))
    return FALSE;

  // skip flow, just confuses things
  if (fight())
    return FALSE;

  // all checks done, now cause things to start flowing
  for (t = rider; t; t = t2) {
    t2 = t->nextRider;
    
    --(*t);
    *to_room += *t;

    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (tbt) {
      tbt->sendTo(fmt("You drift %s...\n\r") % dirs[rd]);
      tbt->doLook("", CMD_LOOK);
    }

    if (t == rider) 
      sprintf(buf, "$n drifts in from the %s riding $N.", dirs[rev_dir[rd]]);
    else
      sprintf(buf, "$n also drifts in from the %s riding $N.", dirs[rev_dir[rd]]);
    act(buf, FALSE, t, 0, this, TO_NOTVICT);

  }

  sendTo(fmt("You drift %s...\n\r") % dirs[rd]);
  --(*this);
  *to_room += *this;
  doLook("", CMD_LOOK);

  if (!rider) {
    sprintf(buf, "$n drifts in from the %s.", dirs[rev_dir[rd]]);
    act(buf, FALSE, this, 0, 0, TO_ROOM);
  }

  // all drifting is done, and text has been shown

  if (riding) {
    rc = riding->genericMovedIntoRoom(to_room, was_in);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete riding;
      riding = NULL;
      REM_DELETE(rc, DELETE_THIS);
    }
  } else {
    rc = genericMovedIntoRoom(to_room, was_in);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  return resCode;
}

bool TObj::isTrash()
{
  if(isObjStat(ITEM_NOJUNK_PLAYER) || 
     !canWear(ITEM_TAKE) || 
     !roomp ||
     getStuff() ||
     rider ||
     roomp->isWaterSector() ||
     roomp->isAirSector() ||
     roomp->number==19024 ||  // briny deep, don't want people catching piles
     isObjStat(ITEM_BURNING) ||
     isname("log commodity wood", getName()) ||
     dynamic_cast<TBaseCorpse *>(this))
    return false;

  return true;
}

bool TObj::joinTrash()
{
  TTrashPile *pile=NULL;

  // check if this item is eligible to be trash
  if(!isTrash())
    return false;

  // find a trash pile
  for(TThing *t=roomp->getStuff();t;t=t->nextThing){
    if((pile=dynamic_cast<TTrashPile *>(t)))
      break;
  }
  if(!pile)
    return false;

  sendrpf(COLOR_BASIC, roomp, "%s merges with %s.\n\r",
    sstring(this->getName()).cap().c_str(), pile->getName());

  // add to trash pile
  --(*this);
  *pile += *this;

  pile->updateDesc();

  if(pile->roomp)
    pile->roomp->saveItems("");
  
  return true;
}



int TObj::riverFlow(int)
{
  int rc, was_in = inRoom();
  TRoom *tmprp, *to_room;
  char buf[256];
  TThing *t, *t2;

  if (!roomp)   // this covers carried && equipped
    return FALSE;
 
  // attached objects don't float away
  if (isObjStat(ITEM_ATTACHED))
    return FALSE;

  if (!roomp->isUnderwaterSector() && !roomp->isWaterSector())
    return FALSE;

  if (roomp->getRiverSpeed() <= 0) 
    return FALSE;

  if (this->objVnum() == GENERIC_FLARE)
    return FALSE;

  if (::number(0,roomp->getRiverSpeed()) && ::number(0,roomp->getRiverSpeed()))
    return FALSE;

  dirTypeT rd = roomp->getRiverDir();

  if ((rd < MIN_DIR) || (rd >= MAX_DIR))
    return FALSE;

  if (rd == DIR_UP) {
    if (!willFloat())
      return FALSE;
  }

  if (isLevitating() && (!roomp->isUnderwaterSector())) 
    return FALSE;

  if (!exitDir(rd) || !exitDir(rd)->to_room ||
      (exitDir(rd)->to_room == ROOM_NOWHERE) ||
      IS_SET(exitDir(rd)->condition, EX_CLOSED))
    return FALSE;

  tmprp = roomp;
  if (!(to_room = real_roomp(tmprp->dir_option[rd]->to_room)))
    return FALSE;

  for (t = rider; t; t = t2) {
    t2 = t->nextRider;

    t->sendTo(fmt("Your %s drifts %s...\n\r") % objn(this) % dirs[rd]);
    sprintf(buf, "$n drifts %s in $p.", dirs[rd]);
    act(buf, TRUE, t, this, 0, TO_ROOM);

    --(*t);
    *to_room += *t;

    sprintf(buf, "$n drifts in from the %s on $p.", dirs[rev_dir[rd]]);
    act(buf, TRUE, t, this, 0, TO_ROOM);
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (tbt)
      tbt->doLook("", CMD_LOOK);

  }
  if (!rider) {
    sprintf(buf, "$n drifts %s...", dirs[rd]);
    act(buf, TRUE, this, 0, 0, TO_ROOM);
  }
  --(*this);
  *to_room += *this;

  if (!rider) {
    sprintf(buf, "$n drifts in from the %s...", dirs[rev_dir[rd]]);
    act(buf, TRUE, this, 0, 0, TO_ROOM);
  }
  if (riding) {
    rc = riding->genericMovedIntoRoom(to_room, was_in);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete riding;
      riding = NULL;
      REM_DELETE(rc, DELETE_THIS);
    }
  } else {
    rc = genericMovedIntoRoom(to_room, was_in);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  return TRUE;
}

// returns DELETE_THIS
int TBeing::teleportRoomFlow(int pulse)
{
  TRoom *dest, *tmprp;
  const char *tmp_desc = NULL;
  int rc;

  if (!roomp || roomp->getTeleTarg() <= 0 || roomp->getTeleTime() <= 0)
    return FALSE;

  // pulse will be a multiple of PULSE_TELEPORT
  if (pulse % roomp->getTeleTime())
    return FALSE;

  if (isImmortal()) {
    if((tmp_desc=roomp->ex_description->findExtraDesc("_tele_")) && 
       inRoom() == roomp->getTeleTarg()){
      act(tmp_desc, TRUE, this, NULL, NULL, TO_CHAR);
    } else {
    // Change this to use act so it didnt send while in redit - Russ 011397
    act("Your immortal status allows you to avoid a teleporter in this room.", TRUE, this, NULL, NULL, TO_CHAR);
    //sendTo("Your immortal status allows you to avoid a teleporter in this room.\n\r");
    }
    return FALSE;
  }

  if (!(dest = real_roomp(roomp->getTeleTarg()))) {
    vlogf(LOG_BUG, fmt("Invalid teleTarg room (%d) to room (%d)") %  
             inRoom() % roomp->getTeleTarg());
    return FALSE;
  }
  tmprp = roomp;  // char_from_room will set roomp to NULL
  --(*this);
  thing_to_room(this, tmprp->getTeleTarg());
  if ((tmp_desc = tmprp->ex_description->findExtraDesc("_tele_"))) {
    if (desc)
      desc->page_string(tmp_desc);
  }
  if (tmprp->getTeleLook())
    doLook("", CMD_LOOK);

  rc = genericMovedIntoRoom(dest, -1);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_THIS;
  }

  return TRUE;
}

void TMonster::makeNoise()
{
  char buffer[100];

  if (fight() || desc)
    return;
  if (inRoom() == ROOM_NOCTURNAL_STORAGE)
    return;

  if (!isPc() && sounds && !rider) {
    if (default_pos > POSITION_SLEEPING) {
      if (getPosition() > POSITION_SLEEPING) 
        MakeRoomNoise(this, in_room, sounds, distantSnds);
      else if (getPosition() == POSITION_SLEEPING) {
        sprintf(buffer, "%s snores loudly.\n\r", mud_str_dup(sstring(getName()).cap()));
        MakeRoomNoise(this, in_room, buffer, "You hear a loud snore nearby.\n\r");
      }
    } else if (getPosition() == default_pos)
      MakeRoomNoise(this, in_room, sounds, distantSnds);
  }

#if 0
  checkResponses((opinion.random ? opinion.random : (targ() ? targ() : this)),
                 NULL, NULL, CMD_RESP_PULSE);
#endif
}

// return DELETE_THIS if this should die
int TBeing::updateAffects()
{
  static affectedData *af, *next_af_dude;
  bool shouldReturn = FALSE;
  int rc;

  if (isPc() && !desc)
    return FALSE;

  if (!affected) 
    return FALSE;

  if (isPc()) {
    int test = TRUE;
    test = FALSE;
  }
  for (af = affected; af; af = next_af_dude) {
    next_af_dude = af->next;
    bool couldBeRenewed = af->canBeRenewed();

    if ((af->duration == PERMANENT_DURATION) &&
          (af->type == AFFECT_DISEASE)) {
      rc = (DiseaseInfo[affToDisease(*af)].code) (this, DISEASE_PULSE, af);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      continue;
    }
    if ((af->duration == PERMANENT_DURATION) &&
          (af->type == SKILL_ENCAMP)) {
      if (((TRoom *) af->be) == roomp)
        continue;
    }
    if ((af->duration == PERMANENT_DURATION) &&
          ((af->type == AFFECT_COMBAT) ||
           (af->type == AFFECT_FREE_DEATHS) ||
           (af->type == AFFECT_TEST_FIGHT_MOB) ||
           (af->type == SKILL_TRACK) ||
           (af->type == SKILL_SEEKWATER) ||
           (af->type == AFFECT_PET) ||
           (af->type == AFFECT_THRALL) ||
           (af->type == SPELL_SANCTUARY) ||
           (af->type == SKILL_BERSERK) ||
           (af->type == AFFECT_WET) )) {
      continue;
    }
    if ((af->type == SPELL_POLYMORPH) ||
        (af->type == SKILL_DISGUISE) ||
        (af->type == SPELL_SHAPESHIFT)) {
      if (!desc->original) {
        affectRemove(af);
        continue;
      }
    }

    if (af->duration >= 1) {
      af->duration--;

      // let user know if it can now be renewed
      if (!couldBeRenewed && af->canBeRenewed()) {
        if (af->shouldGenerateText()) {
          if (af->type >= 0 && af->type < MAX_SKILL && discArray[af->type])
            sendTo(fmt("The effects of %s can now be renewed.\n\r") %
              discArray[af->type]->name);
        }
      }

      if (af->type == AFFECT_DISEASE) {
        rc = (DiseaseInfo[affToDisease(*af)].code) 
                    (this, DISEASE_PULSE, af);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
      } else if (af->duration == 1 * UPDATES_PER_MUDHOUR) {
        // some spells have > 1 effect, do not show 2 messages
        if (af->shouldGenerateText())
          spellWearOffSoon(af->type);
      }
    } else {
      // duration == 0
      if (((af->type >= MIN_SPELL) && (af->type < MAX_SKILL)) ||
          ((af->type >= FIRST_TRANSFORMED_LIMB) && (af->type < LAST_TRANSFORMED_LIMB)) ||
          ((af->type >= FIRST_BREATH_WEAPON) && (af->type < LAST_BREATH_WEAPON)) ||
          ((af->type >= FIRST_ODDBALL_AFFECT) && (af->type < LAST_ODDBALL_AFFECT))) {
        if (af->shouldGenerateText() ||
            (af->next->duration > 0)) {
          rc = spellWearOff(af->type);
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
        }

        if (af->type == AFFECT_DISEASE)
          diseaseStop(af);
				
        if(af->type == AFFECT_BITTEN_BY_VAMPIRE){
          if(!hasQuestBit(TOG_BITTEN_BY_VAMPIRE) &&
             !hasQuestBit(TOG_VAMPIRE)){
            sendTo(COLOR_BASIC, "<r>A strange feeling of <1><k>forboding<1><r> comes over you.<1>\n\r");
            sendTo(COLOR_BASIC, "<r>You shiver briefly, as your blood runs cold.<1>\n\r");
            sendTo(COLOR_BASIC, "<r>In a moment, the feeling has passed.<1>\n\r");
            setQuestBit(TOG_BITTEN_BY_VAMPIRE);
          }
          affectRemove(af);
        }

        if ((af->type == SPELL_POLYMORPH) || 
            (af->type == SKILL_DISGUISE) ||
            (af->type == SPELL_SHAPESHIFT)) {
          shouldReturn = TRUE;
        } else if (af->type == AFFECT_TRANSFORMED_ARMS) {
          if (!af->shouldGenerateText()) {
            next_af_dude = af->next->next;
          }
          transformLimbsBack("", WEAR_ARM_R, FALSE);
        } else if (af->type == AFFECT_TRANSFORMED_HANDS) {
          if (!af->shouldGenerateText()) {
            next_af_dude = af->next->next;
          }
          transformLimbsBack("", WEAR_HAND_R, FALSE);
        } else if (af->type == AFFECT_TRANSFORMED_LEGS) {
          if (!af->shouldGenerateText()) {
            next_af_dude = af->next->next;
          }
          transformLimbsBack("", WEAR_LEG_R, FALSE);
        } else if (af->type == AFFECT_TRANSFORMED_HEAD) {
          if (!af->shouldGenerateText()) {
            next_af_dude = af->next->next;
          }
          transformLimbsBack("", WEAR_HEAD, FALSE);
        } else if (af->type == AFFECT_TRANSFORMED_NECK) {
          if (!af->shouldGenerateText()) {
            next_af_dude = af->next->next;
          }
          transformLimbsBack("", WEAR_NECK, FALSE);
        } else {
          affectRemove(af);
        }
      }
    }
  }
  if (shouldReturn) {
    doReturn("", WEAR_NOWHERE, CMD_RETURN); 
    return ALREADY_DELETED;
  }
  return 0;
}

int TBeing::getNutrition()
{
  TDatabase db(DB_SNEEZY);

  db.query("select nutrition from player where id=%i", getPlayerID());
  db.fetchRow();

  return convertTo<int>(db["nutrition"]);
}

void TBeing::addToNutrition(int amt)
{
  TDatabase db(DB_SNEEZY);

  db.query("update player set nutrition=nutrition+%i where id=%i", amt, getPlayerID());
}

void TBeing::setNutrition(int amt)
{
  TDatabase db(DB_SNEEZY);

  db.query("update player set nutrition=%i where id=%i", amt, getPlayerID());
}



// this is called once per mud hour (about 144 real seconds)
// returns DELETE_THIS
int TBeing::updateTickStuff()
{
  int rc;

  if (desc && isPc()) {
    updateCharObjects();
    if (getTimer() < 999)
      addToTimer(1);

    rc = checkIdling();
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      vlogf(LOG_SILENT, fmt("updateTickStuff: %s (desc) caught idling") %  getName());
      return DELETE_THIS;
    }

    if (getCond(DRUNK) > 15) {
      rc = passOut();
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        vlogf(LOG_SILENT, fmt("updateTickStuff: %s passed out") %  getName());
        return DELETE_THIS;
      }
    }

    if(hasQuestBit(TOG_IS_COMBUSTIBLE) && !::number(0,99)){
      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    }


    if(hasQuestBit(TOG_IS_NARCOLEPTIC) && awake() && !::number(0,99)){
      affectedData af;
      af.type = AFFECT_DUMMY;
      af.level = 1;
      af.duration = 30;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_SLEEP;
      affectJoin(NULL, &af, AVG_DUR_NO, AVG_EFF_NO);
      
      if (getPosition() > POSITION_SLEEPING) {
        if (riding) {
          rc = fallOffMount(riding, POSITION_STANDING);
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
        }
        doSleep("");
      }
    }

    // the feathered races automatically poop.  Its a cultural thing.
    if (getMyRace()->isFeathered() && !fight() && getPosition() > POSITION_SLEEPING &&
      (getCond(PEE) + getCond(POOP)) && !::number(0,2))
    {
      doPoop();
    }

    // some races can automatically regrow limbs if missing
    if (getMyRace()->hasTalent(TALENT_LIMB_REGROWTH))
    {
      wearSlotT possibles[MAX_WEAR-MIN_WEAR];
      int cPoss = 0;
      for (wearSlotT p = MIN_WEAR; p < MAX_WEAR; p++)
        if (!isCritPart(p) && !notBleedSlot(p) && isLimbFlags(p, PART_MISSING))
          possibles[cPoss++] = p;
      wearSlotT slot = cPoss > 0 ? possibles[::number(0, cPoss - 1)] : WEAR_NOWHERE;
      if (slot != WEAR_NOWHERE && limbConnections(slot))
      {
        act(fmt("Your %s itches slighty.  It seems to finally be growing back!") % describeBodySlot(slot), FALSE, this, 0, 0, TO_CHAR);
        act(fmt("$n's %s begins to grow back!") % describeBodySlot(slot), FALSE, this, 0, 0, TO_ROOM);
        setLimbFlags(slot, 0);
        if (slot == WEAR_HAND_R || slot == WEAR_HAND_L)
          setLimbFlags(wearSlotT(slot+9), 0);
        setCurLimbHealth(slot, 1);
      }
    }

    if (desc && (desc->character != this))
      vlogf(LOG_BUG, fmt("bad desc in updateTickStuff() (%s)(%s)") %
            (name ? getName() : "unknown") % 
            (desc->character ? desc->character->name ? desc->character->getName() : "unknown" : "no char"));
    if (desc && vt100())
      desc->updateScreenVt100(CHANGED_MUD);
    else if (desc && ansi())
      desc->updateScreenAnsi(CHANGED_MUD);
  } else if (isPc()) {
    // essentially, linkdead
    if (getTimer() < 127)
      addToTimer(1);

    // linkdead too long, get rid of them
    if (getTimer() > 15 && GetMaxLevel() <= MAX_MORT) {
      // mortals get 15 mins
      nukeLdead(this);
      vlogf(LOG_SILENT, fmt("updateTickStuff: %s (ldead) idled") %  getName());
      return DELETE_THIS;
    } else if (getTimer() > 60) {
      // imms get an hour
      nukeLdead(this);
      vlogf(LOG_SILENT, fmt("updateTickStuff: %s (ldead-imm) idled") %  getName());
      return DELETE_THIS;
    }
  } else if (desc && desc->original) {
    if (desc->original->getTimer() < 127)
      desc->original->addToTimer(1);
  } else if (!desc && !master && (gamePort != PROD_GAMEPORT || in_room != ROOM_NOCTURNAL_STORAGE)) {
#if 1
    bool isAnElemental = isElemental();
    bool hasExp = getExp();
    bool shouldGo = TRUE;
    bool isAnOrphanPet = affectedBySpell(AFFECT_ORPHAN_PET);
    bool isAnOldPet = (affectedBySpell(AFFECT_PET) && !isAnOrphanPet);
    int j;    

    if (!fight() && ((isAnElemental || isAnOldPet) || 
                     (!isAnOrphanPet && !hasExp))) {
      if (specials.hunting || act_ptr || getSnum() >= 0 ||
          (dynamic_cast<TMonster *>(this) && 
           dynamic_cast<TMonster *>(this)->hates.clist)) {
        shouldGo = FALSE;
      }
      if (shouldGo && !getStuff()) {
        for (j = MIN_WEAR; j < MAX_WEAR; j++) {
          if (equipment[j]) {
            shouldGo = FALSE;
            break;
          }
        }
        if (shouldGo) {
          if (isAnElemental) {
            rc = checkDecharm(FORCE_YES);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              vlogf(LOG_SILENT, fmt("updateTickStuff: %s decharmed") %  getName());
              return DELETE_THIS;
            }
            return 0;
          } else {
            act("$n passes away from natural causes.",
                TRUE, this, NULL, NULL, TO_ROOM);
            j = die(DAMAGE_NORMAL);
            if (IS_SET_DELETE(j, DELETE_THIS)) {
              vlogf(LOG_SILENT, fmt("updateTickStuff: %s died naturally") %  getName());
              return DELETE_THIS;
            }
          }
          vlogf(LOG_SILENT, fmt("updateTickStuff: %s died (2) naturally") %  getName());
          return DELETE_THIS;
        }
      }
    }
#endif
  }
  return 0;
}

// returns DELETE_THIS
int TBeing::updateBodyParts()
{
  int rc;
  wearSlotT i;
  unsigned short int flags;
  unsigned int conLimbBonus = plotStat(STAT_CURRENT, STAT_CON, 1, 5, 3);

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    // Check for certain flags that disallow healing here - Russ

    // Making transformed legs tranform back if not in water
    if (isLimbFlags(i, PART_TRANSFORMED)) {
      if (((i == WEAR_LEG_R) || (i==WEAR_LEG_L) || (i==WEAR_FOOT_R) || (i==WEAR_FOOT_L)) && !(roomp->isWaterSector() || roomp->isUnderwaterSector())) {
        sendTo("Your dolphin's tail can't survive out of water.\n\r");
        doReturn("", WEAR_LEG_R, 0);
      }
    }

    if (isLimbFlags(i, PART_BANDAGED) &&
          (!equipment[i] || !isname(equipment[i]->name, "bandage")))
      remLimbFlags(i, PART_BANDAGED);
    
    if (IS_SET(specials.act, ACT_SKELETON) &&
        isAffected(AFF_CHARM) && hasPart(i)) {    // skeleton mobs no regen
      flags = getLimbFlags(i);
      if (IS_SET(flags, PART_MISSING)) {
        // remove extraneous
        setLimbFlags(i, PART_MISSING);
      } else if (IS_SET(flags, (unsigned short int) (PART_BLEEDING | PART_LEPROSED | PART_INFECTED | PART_PARALYZED | PART_USELESS | PART_BRUISED | PART_GANGRENOUS))) {
        // change these bits to broken;
        setLimbFlags(i, PART_BROKEN);
      }
      continue;    // no regen of body parts
    }
    if (IS_SET(specials.act, ACT_ZOMBIE) &&
        isAffected(AFF_CHARM) && hasPart(i)) {    // zombie mobs decay
      if (::number(0,1)) {
        addCurLimbHealth(i, -1);
        if (getCurLimbHealth(i) <= 0) {
          sendTo(fmt("The gangrene in your %s causes it to fall off!!\n\r") %
               describeBodySlot(i));
          makePartMissing(i, TRUE);
        }
      }
      if (isLimbFlags(i, PART_MISSING)) {
        setLimbFlags(i, PART_MISSING);   // get rid of superfluous flags
        if (i == WEAR_HEAD) {
          vlogf(LOG_BUG, fmt("%s killed by lack of a head at %s (%d)") % 
            getName() % roomp->getName() % inRoom());

          rc = die(DAMAGE_BEHEADED);
          if (IS_SET_ONLY(rc, DELETE_THIS))
            return DELETE_THIS;
        }
        if (isCritPart(i)) {
          vlogf(LOG_BUG, fmt("%s killed by lack of a critical body spot (%d:1) at %s (%d)") % 
            getName() % i % roomp->getName() % inRoom());
          rc = die(DAMAGE_NORMAL);
          if (IS_SET_ONLY(rc, DELETE_THIS))
            return DELETE_THIS;
        }
      }
      continue;   // zombies never regen
    }

    // normal critters follow
	
    // **removed leprosy limb damage**
    // The big idea being that leprotic limbs do not actually fall off of their own accord.
    // Instead, combat damage vs. leprotic limbs is increased and they do not heal.
    // This was done both to prevent clerics from cooking off limbs for riskless kills
    // and to keep leprosy a little more in line with its real world affects.
    // Gangrene replaces it as a limb-rotter.
    if (isLimbFlags(i, PART_GANGRENOUS) && hasPart(i)) {
      addCurLimbHealth(i, -2);
      if (getCurLimbHealth(i) <= 0) {
        if (isCritPart(i)) {
          // let go to 0, but don't cause "neck to fall off"
          setCurLimbHealth(i, 0);
        } else {
          vlogf(LOG_COMBAT, fmt("Gangrene rotting off the %s of %s.") % describeBodySlot(i) % getName());
          act(fmt("The <k>gangrene<1> in your %s causes it to fall off!") % describeBodySlot(i), FALSE, this, 0, 0, TO_CHAR);
          makePartMissing(i, TRUE);
        }
      }
    }
    flags = getLimbFlags(i);

    if (isLimbFlags(i, PART_MISSING)) {
      setLimbFlags(i, PART_MISSING);   // get rid of superfluous flags
      if (i == WEAR_HEAD) {
        vlogf(LOG_BUG, fmt("%s killed by lack of a head at %s (%d)") % 
            getName() % roomp->getName() % inRoom());
        rc = die(DAMAGE_BEHEADED);
        if (IS_SET_ONLY(rc, DELETE_THIS))
          return DELETE_THIS;
      }
      if ((i == WEAR_BODY) || (i == WEAR_NECK) || (i == WEAR_BACK)) {
        vlogf(LOG_BUG, fmt("%s killed by lack of a critical body spot (i:2) at %s (%d)") % 
            getName() % i % roomp->getName() % inRoom());
        rc = die(DAMAGE_NORMAL);
        if (IS_SET_ONLY(rc, DELETE_THIS))
          return DELETE_THIS;
      }
    }
    if (IS_SET(flags, (unsigned short int) (PART_MISSING | PART_PARALYZED | PART_BROKEN | PART_BLEEDING | PART_INFECTED | PART_USELESS | PART_LEPROSED | PART_TRANSFORMED | PART_BRUISED | PART_GANGRENOUS)))
      continue;

    if (getCurLimbHealth(i) < getMaxLimbHealth(i) &&
          !::number(0,1) && !fight()) {
      addCurLimbHealth(i, ((flags & PART_BANDAGED) ? 1 + conLimbBonus : conLimbBonus));
    }
  }
  return TRUE;
}

// this is called once per tick (about 36 real seconds)
// DELETE_THIS implies this should be deleted
int TBeing::updateHalfTickStuff()
{
  int foodReject = FALSE;
  int rc = FALSE;
  TThing *t;
  int j;
  TRoom *room = NULL;
  int loadRoom = 0;
  int vnum = mobVnum();
  affectedData *af;
  bool berserk_noheal=0;
  TBeing *trider=NULL;
  unsigned int i, hours_first, hours_last, severity;

  updatePos();

  if (hasClass(CLASS_SHAMAN) && !(roomp->number == ROOM_POLY_STORAGE)) {
    if ((isPc()) && (GetMaxLevel() < 51)) {
      if (0 >= getLifeforce()) {
	reconcileDamage(this,::number(0,2),DAMAGE_DRAIN);
	if (GetMaxLevel() > 5) {
	  setLifeforce(0);
	} else {
	  setLifeforce(1);
	}
	updatePos();
	if ((0 > getHit()) && (getHit() > -3)) {
	  updatePos();
	} 
	if ((-3 > getHit()) && (getHit() > -6)) {
	  updatePos();
	}
	if ((-6 > getHit()) && (getHit() > -10)) {
	  updatePos();
	}
	if (-10 > getHit()) {
	  vlogf(LOG_MISC, fmt("%s autokilled by excessive lifeforce drain at %s (%d)") % 
		getName() % (roomp ? roomp->getName() : "nowhere") % inRoom());
	  if (reconcileDamage(this, 1,DAMAGE_DRAIN) == -1) {
	    die(DAMAGE_DRAIN);
	    return DELETE_THIS;
	  }
	  doSave(SILENT_YES);
	}
      } else {
	addToLifeforce(-1);
	updatePos();
      }
    } else {
      setLifeforce(9000);
      updatePos();
    }

  }

  if (isAffected(AFF_SLEEP) && (getPosition() > POSITION_SLEEPING)) {
    sendTo("You grow sleepy and can remain awake no longer.\n\r");
    act("$n collapses as $e falls asleep.", TRUE, this, 0, 0, TO_ROOM);
    setPosition(POSITION_SLEEPING);
  }

  if(hasQuestBit(TOG_IS_NECROPHOBIC) && (getPosition() > POSITION_SLEEPING) && !::number(0,3)){
    TBeing *tb;
    for(TThing *t=roomp->getStuff();t;t=t->nextThing){
      if(dynamic_cast<TBaseCorpse *>(t) || ((tb=dynamic_cast<TBeing *>(t)) && tb->isUndead()))
      {
        sendTo(fmt("You lose your cool at the sight of %s and freak out!\n\r")% t->getName());
        doFlee("");
        addCommandToQue("flee");
        addCommandToQue("flee");
        break;
      }
    }
  }

  // player is scared of flame (exempt or lessen for mobs?)
  if(hasQuestBit(TOG_HAS_PYROPHOBIA) && (getPosition() > POSITION_SLEEPING) && !::number(0,1)){
    TThing *fleeing = NULL;
    TBeing *tBeing = NULL;
    bool flee = roomp->getSectorType() == SECT_VOLCANO_LAVA ||
                  roomp->getSectorType() == SECT_FIRE ||
                  roomp->getSectorType() == SECT_FIRE_ATMOSPHERE;

    for(TThing *t=roomp->getStuff();!flee && t;t=t->nextThing){
      TObj *tObj = dynamic_cast<TObj *>(t);
      TLight *tLight = dynamic_cast<TLight *>(t);
      tBeing = dynamic_cast<TBeing *>(t);
      fleeing = t;

      // check for burning/lit objects in room
      // perhaps later check for objects made of flame (flares)
      flee = ((tObj && tObj->isObjStat(ITEM_BURNING)) || (tLight && tLight->isLit()));

      if (!flee && tBeing)
        flee = tBeing->getMaterial(WEAR_BODY) == MAT_FIRE;

      // check for burning/lit objects on being
      if (!flee && tBeing)
      {
        for(wearSlotT iWear = MIN_WEAR; !flee && iWear < MAX_WEAR; iWear++){
          if (!tBeing->equipment[iWear])
            continue;
          fleeing = tBeing->equipment[iWear];
          tObj = dynamic_cast<TObj *>(tBeing->equipment[iWear]);
          tLight = dynamic_cast<TLight *>(tBeing->equipment[iWear]);
          flee = ((tObj && tObj->isObjStat(ITEM_BURNING)) || (tLight && tLight->isLit()));
        }
      }
    }

    if (flee)
    {
      if (tBeing && tBeing != this && fleeing)
        sendTo(fmt("You lose your cool at the sight of %s's %s and freak out!\n\r")% tBeing->getName() % fleeing->getName());
      else if (fleeing)
        sendTo(fmt("You lose your cool at the sight of %s and freak out!\n\r")% fleeing->getName());
      else
        sendTo("You lose your cool in this fiery place and freak out!\n\r");
      doFlee("");
      addCommandToQue("flee");
      addCommandToQue("flee");
    }
  }

  if(hasQuestBit(TOG_HAS_TOURETTES) && !::number(0,3)){
    sstring buf, buf2;
    TBeing *tb;
    TMonster *tm;
    
    for(TThing *t=roomp->getStuff();t;t=t->nextThing){
      if(!::number(0,1))
	continue;

      if((tb=dynamic_cast<TBeing *>(t))){
	if(tb==this)
	  continue;

	if(!canSee(tb))
	  continue;

	buf=getInsult(tb);
	buf2 = fmt("$n looks at you and says, \"%s\"") %buf;
	act(buf2,TRUE,this,0,tb,TO_VICT);
	buf2 = fmt("$n looks at $N and says, \"%s\"") %buf;
	act(buf2,TRUE,this,0,tb,TO_NOTVICT);
	buf2 = fmt("You look at $N and say, \"%s\"") %buf;
	act(buf2,TRUE,this,0,tb,TO_CHAR);
	
	if((tm=dynamic_cast<TMonster *>(t)))
	  tm->aiUpset(this);
	
	break;
      }
    }
  }


  if(inRoom() >= 31800 && inRoom() <= 31899 && getCond(DRUNK) == 0){
    sendTo("Totally sober now, this world seems to fade away like a dream.\n\r");
    setPosition(POSITION_SLEEPING);

    TRoom *room = real_roomp(12643); // beach near jungle
    --(*this);
    *room += *this;
  }

  rc = updateBodyParts();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  TMonster *tmons = dynamic_cast<TMonster *>(this);
  if (tmons && !tmons->desc) {
    tmons->mobAI();
    loadRoom = tmons->oldRoom;
  }
  if (tmons && !tmons->desc && (trider=dynamic_cast<TBeing *>(tmons->rider)))
    trider->calmMount(this);
  
  if (isFlying()) {
    if (!canFly() && roomp && !roomp->isFlyingSector()) {
      vlogf(LOG_BUG, fmt("Somehow %s was position fly and was not in flying sector") %  getName());
      sendTo("You stop flying around in the air.\n\r");
      setPosition(POSITION_STANDING);
    }
  }
  if (roomp && roomp->isFlyingSector()) {
    TBeing *tbr = dynamic_cast<TBeing *>(riding);
    if (!riding && !isFlying()) {
      setPosition(POSITION_FLYING);
      sendTo("You start to fly up in the air.\n\r");
      vlogf(LOG_BUG, fmt("Somehow %s was not flying in flying sector") %  getName());
    } else if (riding && !tbr) {
      dismount(POSITION_FLYING);
      sendTo("You start to fly up in the air.\n\r");
    } else if (tbr && !tbr->isFlying()) {
      tbr->setPosition(POSITION_FLYING);
      sendTo("Your mount starts to fly up in the air.\n\r");
      vlogf(LOG_BUG, fmt("Somehow %s was not flying in flying sector") %  getName());
    }
  }
  if (roomp && (zone_table[roomp->getZoneNum()].enabled == TRUE) && 
      (!inImperia() || (in_room == ROOM_NOCTURNAL_STORAGE)) && 
      ((specials.act & ACT_DIURNAL) || (specials.act & ACT_NOCTURNAL)) &&
       !fight() && (getPosition() > POSITION_STUNNED) &&
       (!::number(0,2)) &&
       !act_ptr &&  // prevents hornsby taking off
       (getHit() >= hitLimit()) &&
      (number >= 0)) {
    j = 0; // temp holder
    t = NULL; // temp holder
    if (IS_SET(specials.act, ACT_DIURNAL) && !isAffected(AFF_CHARM)) {
      if ((inRoom() == ROOM_NOCTURNAL_STORAGE)) {

        // hoppers are a mob that load periodically from spec-room proc
        // they are removed via nocturnal, but we don't want them
        // "reappearing" by waking up, so if they get sucked out, just
        // whack them
        if ((vnum == MOB_MALE_HOPPER) || (vnum == MOB_FEMALE_HOPPER) ||
            (vnum == MOB_MALE_CHURCH_GOER) || (vnum == MOB_FEMALE_CHURCH_GOER))
          return DELETE_THIS;

        if (is_daytime()) {
          if (loadRoom == ROOM_NOCTURNAL_STORAGE) {
            return DELETE_THIS;
            vlogf(LOG_BUG, fmt("NOC:DIU: %s has oldRoom equal to %d") %  getName() % loadRoom);
          }
          if (!loadRoom || (loadRoom == ROOM_NOWHERE)) {
            vlogf(LOG_BUG, fmt("NOC:DIU: %s was %s.") %  getName() %
                  (!loadRoom ? "without loadRoom" : "in room nowhere"));
            return DELETE_THIS;
          }
          if (!(room = real_roomp(loadRoom))) {
            vlogf(LOG_BUG, fmt("NOC:DIU: %s was in a room that no longer exists.") %  getName());
            return DELETE_THIS;
          }
          --(*this);
          *room += *this;
          if (vnum == MOB_FREEZING_MIST) {
            act("$n forms in the surrounding air.",
               TRUE, this, 0, 0, TO_ROOM);
#if 1
          } else if (ex_description && ex_description->findExtraDesc("bamfin")) {
            act(ex_description->findExtraDesc("bamfin"), TRUE, this, 0, 0, TO_ROOM);
#endif
          } else if (IS_SET(specials.act, ACT_GHOST)) {
            act("$n shimmers into existence.",
                   TRUE, this, 0, 0, TO_ROOM);
          } else if (isAnimal()) {
            act("$n awakens from $s slumber.",
                   TRUE, this, 0, 0, TO_ROOM);
           } else {
             char tString[256];

             sprintf(tString, "$n %s to begin work.", race->moveIn());
             act(tString, TRUE, this, 0, 0, TO_ROOM);
          }
        }
      } else if (!is_daytime() && ((vnum == MOB_FREEZING_MIST) || !specials.hunting)) {
        if (vnum == MOB_FREEZING_MIST) {
          act("$n is dispersed by the coming of morning.", 
              TRUE, this, 0, 0, TO_ROOM);
#if 1
        } else if (ex_description && ex_description->findExtraDesc("bamfout")) {
          act(ex_description->findExtraDesc("bamfout"), TRUE, this, 0, 0, TO_ROOM);
#endif
        } else if (IS_SET(specials.act, ACT_GHOST)) {
          act("$n shimmers out of existence.",
                 TRUE, this, 0, 0, TO_ROOM);
        } else if (isAnimal()) {
          act("$n wanders off to find $s den.",
                 TRUE, this, 0, 0, TO_ROOM);
        } else {
          act("$n notices the time, and hurries on $s way.",
                 TRUE, this, 0, 0, TO_ROOM);
        }
        if (!(room = real_roomp(ROOM_NOCTURNAL_STORAGE))) {
           return DELETE_THIS;
        }
        if (riding)
          dismount(POSITION_STANDING);
        while (rider)
          rider->dismount(POSITION_STANDING);

        --(*this);
        *room += *this;
      } else {
// not in storage and is daytime -- do nothing
      }
    } else if (IS_SET(specials.act, ACT_NOCTURNAL) && !isAffected(AFF_CHARM)) {
      if ((in_room == ROOM_NOCTURNAL_STORAGE)) {
        if (!is_nighttime()) {
          if ((vnum == MOB_MALE_HOPPER) || (vnum == MOB_FEMALE_HOPPER) ||
              (vnum == MOB_MALE_CHURCH_GOER) || (vnum == MOB_FEMALE_CHURCH_GOER))
            return DELETE_THIS;
        } else {
          if (loadRoom == ROOM_NOCTURNAL_STORAGE) {
            return DELETE_THIS;
            vlogf(LOG_BUG, fmt("NOC:DIU: %s has oldRoom equal to %d") %  getName() % loadRoom);
          }
          if (!loadRoom || (loadRoom == ROOM_NOWHERE)) {
	    if ((vnum == MOB_MALE_HOPPER) || (vnum == MOB_FEMALE_HOPPER) ||
		(vnum == MOB_MALE_CHURCH_GOER) || (vnum == MOB_FEMALE_CHURCH_GOER))
	      return DELETE_THIS; // do this before the check to cut down on hopper bug spam
            vlogf(LOG_BUG, fmt("NOC:DIU: %s was %s.") %  getName() %
                  (!loadRoom ? "without loadRoom" : "in room nowhere"));
            return DELETE_THIS;
          }
          if (!(room = real_roomp(loadRoom))) {
            vlogf(LOG_BUG, fmt("NOC:DIU: %s was in a room that no longer exists.") %  getName());
            return DELETE_THIS;
          }
          --(*this);
          *room += *this;
         if (vnum == MOB_FREEZING_MIST) {
           act("$n forms in the surrounding air.", 
               TRUE, this, 0, 0, TO_ROOM);
#if 1
          } else if (ex_description && ex_description->findExtraDesc("bamfin")) {
            act(ex_description->findExtraDesc("bamfin"), TRUE, this, 0, 0, TO_ROOM);
#endif
          } else if (IS_SET(specials.act, ACT_GHOST)) {
            act("$n shimmers into existence.",
                TRUE, this, 0, 0, TO_ROOM);
          } else if (isAnimal()) {
            act("$n awakens from $s slumber.",
                TRUE, this, 0, 0, TO_ROOM);
          } else {
            char tString[256];

            sprintf(tString, "$n %s to begin work.", race->moveIn());
            act(tString, TRUE, this, 0, 0, TO_ROOM);
          }
        }
      } else if (!is_nighttime() && 
                   ((vnum == MOB_FREEZING_MIST) || !specials.hunting)) {
        if (vnum == MOB_FREEZING_MIST) {
          act("$n is dispersed by the coming of morning.", 
              TRUE, this, 0, 0, TO_ROOM);
#if 1
        } else if (ex_description && ex_description->findExtraDesc("bamfout")) {
          act(ex_description->findExtraDesc("bamfout"), TRUE, this, 0, 0, TO_ROOM);
#endif
        } else if (IS_SET(specials.act, ACT_GHOST)) {
          act("$n shimmers out of existence.",
                 TRUE, this, 0, 0, TO_ROOM);
        } else if (isAnimal()) {
          act("$n wanders off to find $s den.",
                 TRUE, this, 0, 0, TO_ROOM);
        } else {
          act("$n notices the time, and hurries on $s way.",
                 TRUE, this, 0, 0, TO_ROOM);
        }
        if (!(room = real_roomp(ROOM_NOCTURNAL_STORAGE))) {
           return DELETE_THIS;
        }
        if (riding)
          dismount(POSITION_STANDING);
        while (rider)
          rider->dismount(POSITION_STANDING);

        --(*this);
        *room += *this;
      } else {
// nocturnal mob in world and not nighttime
      }
    } else {
// do nothing cause it doesnt involve nocturnal or diurnal mobs
    }
  }

 
 // VERY TEMP FIX 10/99 -- COS
  if (!roomp) {
    vlogf(LOG_BUG, fmt("%s has no roomp in updateHalf tick. Try to find") %  getName());
      return FALSE;
  }

  if (!isPc() || desc) {
    int mg = moveGain();
    mg = min(mg, moveLimit() - getMove());
    addToMove(mg);
  }

  if (!fight()) {
    if (isCombatMode(ATTACK_BERSERK)) 
      goBerserk(NULL);
  }
  rc = terrainSpecial();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  if (!fight() && (!isPc() || desc)) {
    if (isCombatMode(ATTACK_BERSERK) || isAffected(SKILL_BERSERK)) {
      if(isCombatMode(ATTACK_BERSERK)){
	setCombatMode(ATTACK_OFFENSE);
	act("You finally calm down and stop berserking.",TRUE, this,0,0,TO_CHAR);
	sendTo("--> Attack mode set to: offense.\n\r");
	act("$n calms down and stops berserking.",TRUE,this,0,0,TO_ROOM);
      }

      for (af = affected; af; af = af->next) {
	if (af->type == SKILL_BERSERK && af->location == APPLY_CURRENT_HIT){
	  if(getHit() < af->modifier){
	    af->modifier=getHit()+2;
	    berserk_noheal=1;
	    setPosition(POSITION_STUNNED);
	    sendTo("You have overexerted yourself and nearly die from the strain.\n\r");
	  } else 
	    sendTo("You feel drained as the bloodlust leaves you.\n\r");
	}
      }
      affectFrom(SKILL_BERSERK);
    }
    if (roomp && roomp->isIndoorSector() && !(roomp->isWaterSector() || roomp->isUnderwaterSector()))  {
      if (hasTransformedLimb()) 
        transformLimbsBack("", MAX_WEAR, FALSE);
      
      if(desc && desc->original && desc->original->polyed && !desc->original->isImmortal() && (desc->original->polyed == POLY_TYPE_SHAPESHIFT)) {
        sendTo("Your shape can not survive without a connection to nature.\n\r");
        doReturn("", WEAR_NOWHERE, CMD_RETURN); 
        return ALREADY_DELETED;
      }
    }

    if(desc && isPc()){
      for(i=0;i<drugTypes.size();i++){
	if(desc->drugs[i].current_consumed>0){
	  --desc->drugs[i].current_consumed;
	  applyDrugAffects(this, (drugTypeT) i, true);
	  saveDrugStats();
	}
	if(desc->drugs[i].total_consumed>0){
	  hours_first=
	    (((time_info.year - desc->drugs[i].first_use.year) * 12 * 28 * 24) +
	     ((time_info.month - desc->drugs[i].first_use.month) * 28 * 24) +
	     ((time_info.day - desc->drugs[i].first_use.day) * 24) +
	     time_info.hours - desc->drugs[i].first_use.hours);
	  hours_last=
	    (((time_info.year - desc->drugs[i].last_use.year) * 12 * 28 * 24) +
	     ((time_info.month - desc->drugs[i].last_use.month) * 28 * 24) +
	     ((time_info.day - desc->drugs[i].last_use.day) * 24) +
	     time_info.hours - desc->drugs[i].last_use.hours);
	  
	  if(hours_first>((30*24)+hours_last))
	    hours_first=(30*24)+hours_last;

	  // how often we smoke (hours_first, total_consumed)
	  // how far into withdrawal are we (hours_last)
	  // problem is, I shouldn't be less addictive, just because I had
          // 1 cigarette 10 years ago - hours_first doesn't work for this
	  //
	  // could make everyone equally addicted once they start
	  // then just use hours_last as indicator of addiction
	  // then after 30 days or whatever, addiction is gone
	  // maybe need new value, consumed in addiction period (30 prior days)
	  // so if you consumed more than X in prior 30 days, you're addicted

	  if(hours_last && hours_first){
	    // this should be average amount of the drug in their body since
	    // they first used it.  We give them 24 extra hours, so you don't
            // get addicted right off the bat.  Multiply by 10 for a little
            // extra precision.
	    severity=(desc->drugs[i].total_consumed / 
		      (max(hours_last-hours_first, (unsigned int) 24*7)+24)) * 10;
	    if(severity>0)
	      applyAddictionAffects(this, (drugTypeT) i, severity);
	  }
	}
      }
    }

    int drunk = getCond(DRUNK);
    if (drunk > 15) {
      rc = passOut();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    }
    // Imperia is immune to Food and Drink
    if (isPc() && !inImperia() && !inLethargica())  {
      int old_food = getCond(FULL);
      int old_drink = getCond(THIRST);

      // not the ideal choice for sector choice if !roomp, but oh well
      foodNDrink(roomp ? roomp->getSectorType() : SECT_INSIDE_MOB,4);

      // Changed to not take timer into account, people want to lose hit points
      if ((!getCond(FULL) && !old_food) || (!getCond(THIRST) && !old_drink)) {
        if (desc) { 
          points.hit -= 1; // + GetMaxLevel()/2; Whoever did this sucks - Russ
          if (!old_food)
            sendTo(COLOR_BASIC, "<R>You are weak from lack of nutrients.<1>\n\r");

          if (!old_drink)
            sendTo(COLOR_BASIC, "<R>You are parched.<1>\n\r");

          foodReject = TRUE;
          updatePos();
          if (points.hit < -10) {
            vlogf(LOG_MISC, fmt("%s killed by starvation at %s (%d)") % 
                getName() % (roomp ? roomp->getName() : "nowhere") % inRoom());
            rc = die(DAMAGE_STARVATION);
            if (IS_SET_ONLY(rc, DELETE_THIS))
              return DELETE_THIS;
          }
        }
      }
    }
    if (roomp && (roomp->getSectorType() == SECT_DESERT) &&
        (getPosition() > POSITION_STUNNED) && is_daytime())
      sendTo("The desert heat makes you uncomfortably warm.\n\r");
    else if (roomp && (roomp->getSectorType() == SECT_JUNGLE) &&
        (getPosition() > POSITION_STUNNED) && is_daytime())
      sendTo("The jungle humidity causes you to sweat uncontrollably.\n\r");
    else if (roomp && (getPosition() > POSITION_STUNNED)) {
      // At the end of this if we do the elemental 'drain' checks.  Regardless of
      // the following 3 conditions we Always want that check to occure.
      if (!roomp->isRoomFlag(ROOM_NO_HEAL) && (drunk < 15) && has_healthy_body(this)) {
        int mana_bump = manaGain();

	if (affectedBySpell(SKILL_MIND_FOCUS)) {
	  mana_bump = (int)((float) mana_bump * 
		      (1.0+((float)getSkillValue(SKILL_MIND_FOCUS)/100.0)));
	}  

        if (foodReject || (drunk > 15)) {
          mana_bump = ::number(1,3);
          if (!foodReject || (0 >= getLifeforce())) {
            addToHit(1);
	    updatePos();
            sendTo("Your condition prevents your body's full recovery.\n\r");
          } else 
            sendTo("Your condition takes its toll on your body.\n\r"); 
        } else if(!berserk_noheal)
          addToHit(hitGain());
      
        addToPiety(pietyGain(0.0));

          addToMana(mana_bump);
      }

       // Check charms *AFTER* the periodic gain
       // if gain is 50, and cost if 65, players will see themselves losing
       // 15 per tick.  We don't want charm to be untenable at what appears
       // to be ~65 mana
      checkCharmMana();

    } else if ((getPosition() == POSITION_STUNNED) && getCond(THIRST) && getCond(FULL) && !berserk_noheal) {
      addToHit(1);
      updatePos();
    } else if (getPosition() == POSITION_INCAP) {
      if (reconcileDamage(this, 1,DAMAGE_NORMAL) == -1)
        return DELETE_THIS;

      doSave(SILENT_YES);
    } else if (getPosition() == POSITION_MORTALLYW) {
      if (reconcileDamage(this, 1,DAMAGE_NORMAL) == -1)
        return DELETE_THIS;

      doSave(SILENT_YES);
    }
  }
  return FALSE;
}

void TPCorpse::decayMe()
{
#if 0
  int found = FALSE;
  TThing *tmp = NULL;

  for(tmp = getStuff(); tmp; tmp = tmp->nextThing) {
    TObj *obj = dynamic_cast<TObj *>(tmp);
    if (!obj)
      continue;
    if (!obj->isObjStat(ITEM_NORENT)) {
      found = TRUE;
      break;
    }
  }
  if (!found) {
    obj_flags.decay_time--;
    if (checkOnLists()) {
      removeCorpseFromList();
    }
    return;
  } 
#endif

  if (inRoom() == ROOM_STORAGE) {
    return;
  }

// valid corpse, see if we have a dead mud
  obj_flags.decay_time--;
  return;
}

void TObj::decayMe()
{
  if (obj_flags.decay_time > 0)
    if (in_room != ROOM_NOWHERE) 
      obj_flags.decay_time--;
}

// returns DELETE_THIS if this has been destroyed (for whatever reason), 
int TObj::objectTickUpdate(int pulse)
{
  TBeing *ch = NULL;
  TThing *t;
  TEgg *egg;
  int rc;

  if (!name) {
    vlogf(LOG_BUG, fmt("Object with NULL name in objectTickUpdate() : %d") %  objVnum());
    return DELETE_THIS;
  }

  for (int j = 0; j < MAX_OBJ_AFFECT; j++) {
    if (affected[j].duration > 0) {
      affected[j].duration--;
      if (affected[j].duration == 0) {

       // don't show multiple times
       if ((j == MAX_OBJ_AFFECT - 1) || 
           affected[j+1].type != affected[j].type) {
          if (((t = equippedBy) && (ch = dynamic_cast<TBeing *>(t))) ||
              ((t = parent) && (ch = dynamic_cast<TBeing *>(t)))) {
            // thing is held or in inventory
            switch (affected[j].type) {
              case SPELL_BLESS:
                act("The blessing surrounding your $o fades.",
                      FALSE, ch, this, 0, TO_CHAR);
                act("The blessing surrounding $n's $o fades.",
                      FALSE, ch, this, 0, TO_ROOM);
                break;
              case SPELL_BLESS_DEIKHAN:
                act("The blessing surrounding your $o fades.",
                      FALSE, ch, this, 0, TO_CHAR);
                act("The blessing surrounding $n's $o fades.",
                      FALSE, ch, this, 0, TO_ROOM);
                break;
              default:
                break;
            }
          } else {
            // thing is lying in the room
            switch (affected[j].type) {
              case SPELL_BLESS:
                act("The blessing surrounding $n fades.",
                      FALSE, this, 0, 0, TO_ROOM);
                break;
              case SPELL_BLESS_DEIKHAN:
                act("The blessing surrounding $n fades.",
                      FALSE, this, 0, 0, TO_ROOM);
                break;
              default:
                break;
            }
          }
        }
        affected[j].type = TYPE_UNDEFINED;
        affected[j].level = 0;
        affected[j].duration = 0;
        affected[j].modifier = 0;
        affected[j].location = APPLY_NONE;
        affected[j].modifier2 = 0;
        affected[j].bitvector = 0;
      }
    }
  }

  TPlant *tp;
  if((tp=dynamic_cast<TPlant *>(this))){
    tp->updateAge();
  }

  if (obj_flags.decay_time > -1) {
    // updateCharObjects takes care of worn, carried It decrements decay
    // but does NOT delete the obj
    decayMe();

    if (!obj_flags.decay_time) {
      if ((rc = objectDecay())) {
        return rc;
      } else {
        if (equippedBy)        {
          // Worn in equipment 
          act("$p decay$Q into nothing.", FALSE, equippedBy, this, 0, TO_CHAR);
          while ((t = getStuff())) {
            (*t)--;
            *equippedBy += *t;
          }
        } else if (parent) {
          act("$p disintegrate$Q in your hands.", FALSE, parent, this, 0, TO_CHAR);
          while ((t = getStuff())) {
            (*t)--;
            *parent += *t;
          }
        } else if (roomp) {  // in room
          act("$n fade$R into insignificance.",
                 TRUE, this, 0, 0, TO_ROOM);
          while ((t = getStuff())) {
            (*t)--;
            *roomp += *t;
          }
        }

        return DELETE_THIS;
      }
    }
  }
  // Lights!!!! - Russ
  lightDecay();

  // Jugged items - Russ 
  if (eq_stuck > WEAR_NOWHERE) {
    if ((t = stuckIn) && (ch = dynamic_cast<TBeing *>(t))) {
      ch->sendTo(COLOR_OBJECTS, fmt("The wounds in your %s start to fester as %s sinks deeper into your flesh.\n\r") % ch->describeBodySlot(eq_stuck) % shortDescr);
      if (::number(0, ch->getConShock()) < 2) {
        if (!ch->isLimbFlags(eq_stuck, PART_INFECTED)) {
          if (ch->rawInfect(eq_stuck, 200, SILENT_YES, CHECK_IMMUNITY_YES)) 
            ch->sendTo(COLOR_OBJECTS, fmt("Your %s has been infected by %s.\n\r") % ch->describeBodySlot(eq_stuck) % shortDescr);
        }
      }
      if (::number(0, ch->getConShock()) < 2) {
        if (ch->isLimbFlags(eq_stuck, PART_INFECTED) && !ch->isLimbFlags(eq_stuck, PART_GANGRENOUS))
          ch->rawGangrene(eq_stuck, 500, SILENT_NO, CHECK_IMMUNITY_YES);
      }
      if (ch->reconcileDamage(ch, (int) baseDamage(), SPELL_INFECT) == -1) {
        delete ch;
        ch = NULL;
      }
      return FALSE;
    }
  }
  // Sound objects 
  audioCheck(pulse);

  // damage things in water
  // portals,windows get a pass on this
  if ((getStructPoints() != -1) &&
      roomp && roomp->isWaterSector() &&
      !dynamic_cast<TSeeThru *>(this)) {
    if (dice(1, 10) <= (material_nums[getMaterial()].water_susc % 10)) {
      if ((obj_flags.struct_points - (material_nums[getMaterial()].water_susc / 10)) <= 0) {
        sendrpf(roomp, "%s sinks into the water and is gone.\n\r", sstring(shortDescr).cap().c_str());
	TRoom *briny_deep=real_roomp(19024);
	--(*this);
	*briny_deep += *this;
	//        return DELETE_THIS;
      } else{
        sendrpf(roomp, "%s becomes water-logged and is damaged.\n\r", sstring(shortDescr).cap().c_str());
	obj_flags.struct_points -=(material_nums[getMaterial()].water_susc / 10);
      }
    }
  }
  
  if(roomp && roomp->isRoomFlag(ROOM_ON_FIRE) && 
     !dynamic_cast<TSeeThru *>(this)) {

    int rc = burnObject(NULL, 30);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
  }

  
  
  // Incubate eggs
  if ((egg = dynamic_cast<TEgg *>(this))) {
    if (egg->getEggTouched()) {
      int myTimer=egg->eggIncubate();
      TRoom *rp;
      TBeing *ch;
	  if (ch = dynamic_cast<TBeing *>(egg->equippedBy)) {
        rp = ch->roomp;
      } else {
        if (!(rp = roomp)) {
	      if (!(rp = parent->roomp)) {
            rp = parent->parent->roomp;
          }
        }
      }
      if (myTimer == 1) {
        if (rp) {
          sendrpf(COLOR_BASIC, rp, "%s begins to wiggle a bit.\n\r", sstring(shortDescr).cap().c_str());
        }
      } else if (myTimer <= 0) {
        if (rp && !(rp->isRoomFlag(ROOM_NO_MOB))) {
          egg->hatch(rp);
          egg->makeScraps();
          return DELETE_THIS;
        }
      }
    }
  }

  // this relies on fact that:
  // weatherAndTime() and objectTickUpdate both trigger off of !tick_update
  // from socket.cc.  weatherAndTime has already been called
  rc = checkSpec(NULL, CMD_OBJ_WEATHER_TIME, "", NULL);
  if (IS_SET_DELETE(rc, DELETE_THIS)) 
    return DELETE_THIS;

  return FALSE;
}


int TObj::updateBurning(void)
{
  TBeing *ch;
  TThing *t;

  if (isObjStat(ITEM_BURNING) && ::number(0,1)){
    // this is for the fireman proc, it tells them that they need to
    // look for a fire.
    if(inGrimhaven()){
      if(!fireInGrimhaven){
	TRoom *rp;
	for(unsigned int zone = 0; zone < zone_table.size(); zone++) {
	  if((rp=real_roomp(zone_table[zone].top)) && rp->inGrimhaven()){
	    zone_table[zone].sendTo("<R>Loud firebells begin clanging in the distance!<1>\n\r", 4673);
	  }
	}
	
	if((rp=real_roomp(4673))){
	  rp->sendTo(COLOR_BASIC, "<R>The firebells begin to clang hysterically!<1>\n\r");
	}

      }

      fireInGrimhaven=true;
    }

    // we calculate the number of structure points per cubic inch
    // flammability is the number of ci's that burn per tick
    // so struct per ci * flam is how much we lose per tick
    // suppose we have a wooden staff, 4000 volume, 20 structure and 500
    // flammability, (20/4000)=.005, .005*500 = 2.5, 20/2.5 = 8
    // so it will burn for approx 8 rounds
    int burnamount = (int)((double)((double)getMaxStructPoints()/getVolume())*
			       material_nums[getMaterial()].flammability);
    burnamount=max(1, burnamount);

    if((t = equippedBy) || (t = parent) || (t = stuckIn)){
      ch = dynamic_cast<TBeing *>(t);
    } else
      ch = NULL;

    dropSmoke(::number(1,5));

    switch(::number(0, 5)){
      case 0:
	if(ch){
	  act("Your $o crackles merrily as it <r>burns<1> down.",
	      FALSE, ch, this, 0, TO_CHAR);
	  act("$n's $o crackles merrily as it <r>burns<1> down.",
	      FALSE, ch, this, 0, TO_ROOM);
	} else 
	  act("$n crackles merrily as it <r>burns<1> down.",
	      FALSE, this, 0, 0, TO_ROOM);
	break;
      case 1:
	if(ch){
	  act("Your $o <r>burns<1> away steadily.",
	      FALSE, ch, this, 0, TO_CHAR);
	  act("$n's $o <r>burns<1> away steadily.",
	      FALSE, ch, this, 0, TO_ROOM);
	} else
	  act("$n <r>burns<1> away steadily.",
	      FALSE, this, 0, 0, TO_ROOM);
	break;
      case 2:
	if(ch){
	  act("<k>Smoke<1> billows forth from $o as it <r>burns<1>.",
	      FALSE, ch, this, 0, TO_CHAR);
	  act("<k>Smoke<1> billows forth from $n's $o as it <r>burns<1>.",
	      FALSE, ch, this, 0, TO_ROOM);
	} else
	  act("<k>Smoke<1> billows forth from $n as it <r>burns<1>.",
	      FALSE, this, 0, 0, TO_ROOM);
	break;
      case 3:
	if(ch){
	  act("Your $o <Y>flares<1> up momentarily as it <r>burns<1>.",
	      FALSE, ch, this, 0, TO_CHAR);
	  act("$n's $o <Y>flares<1> up momentarily as it <r>burns<1>.",
	      FALSE, ch, this, 0, TO_ROOM);
	} else
	  act("$n <Y>flares<1> up momentarily as it <r>burns<1>.",
	      FALSE, this, 0, 0, TO_ROOM);
	break;
      default:
	// nothing
	break;
    }

    if((equippedBy || stuckIn) && ch){
      act("Your $o burns you!",
	  FALSE, ch, this, 0, TO_CHAR);
      ch->reconcileDamage(ch, ::number(1,7), DAMAGE_FIRE);
    }

    // we let non-flammable things burn, but we don't 'decay' them
    if(material_nums[getMaterial()].flammability){
      if(IS_SET_DELETE(damageItem(burnamount), DELETE_THIS)){
	return DELETE_THIS;
      }
      remBurning(ch);
    }

    // spread to other items
    TRoom *tr=real_roomp(this->in_room);
    int fire_count=0;
  
    if(tr && material_nums[getMaterial()].flammability && tr->getStuff()){
      for(TThing *tt=tr->getStuff();tt;tt=tt->nextThing){
	int cf=40; // chance factor: flammability/cf = percent chance
	int chance=(int)(material_nums[tt->getMaterial()].flammability/cf);
	TObj *to=dynamic_cast<TObj *>(tt);

	if(to && !to->isObjStat(ITEM_BURNING) && 
	   material_nums[this->getMaterial()].flammability >0 &&
	   (::number(0,100) < chance)){
	  act("The <Y>fire<1> spreads to $n and it begins to <r>burn<1>!",
	      FALSE, to, 0, 0, TO_ROOM);	  
	  to->setBurning(NULL);

	  // now count up burning volume
	  if(to && to->isObjStat(ITEM_BURNING))
	    fire_count+=to->getVolume();

	}
      }
    }

    if(tr && fire_count >= ROOM_FIRE_THRESHOLD && !tr->isWaterSector())
      tr->setRoomFlagBit(ROOM_ON_FIRE);
    
  }

  return FALSE;
}


// returns DELETE_THIS
int TBeing::terrainSpecial()
{
  int dam, num = 0;
  affectedData af;
  TThing *t;

  if (!roomp)
    return FALSE;

  switch (roomp->getSectorType()) {
    case SECT_FIRE:
    case SECT_FIRE_ATMOSPHERE:
      if (!desc && isPc())
        return FALSE;
      dam = ::number(2,6);
      if(roomp->isRoomFlag(ROOM_PEACEFUL))
	dam=0;
      sendTo("Flames lick about you, scorching your skin.\n\r");
      act("Flames lick about $n and scorch $s skin.",TRUE,this,0,0,TO_ROOM);
      return reconcileDamage(this,dam,SPELL_FIREBALL);
    case SECT_DESERT:
      // drain water
      for (t = getStuff(); t; t = t->nextThing) {
        evaporate(this, SILENT_NO);
      }
      return FALSE;
    case SECT_SUBARCTIC:
    case SECT_ARCTIC_WASTE:
    case SECT_TUNDRA:
      if (!num)
        num = 1;
    case SECT_ARCTIC_ROAD:
    case SECT_ARCTIC_CITY:
      if (!num)
        num = 3;
    case SECT_ARCTIC_MOUNTAINS:
      if (!num)
        num = 1;
    case SECT_ARCTIC_FOREST:
      if (!num)
        num = 4;
    case SECT_ARCTIC_RIVER_SURFACE:
    case SECT_ICEFLOW:
    case SECT_COLD_BEACH:
      if (!num)
        num = 2;
    case SECT_SOLID_ICE:
    case SECT_ARCTIC_ATMOSPHERE:
    case SECT_ARCTIC_CLIMBING:
      if (!num)
        num = 1;
      if (!outside())
        break;
      if (::number(0,num))
        break;
      act("A cold wind blows through, chilling you to the bone.",TRUE,this,0,0,TO_CHAR);
#if 0
      if (affectedBySpell(AFFECT_WAS_INDOORS) || 
	  hasDisease(DISEASE_FROSTBITE))
	return FALSE;  // make it only hit em if they sit outside for a while
      if (isImmune(IMMUNE_COLD, WEAR_BODY))
        break;
      if (dynamic_cast<TMonster *>(this))
        break;
      if (isPc() && isImmortal())
         break;
      af.type = AFFECT_DISEASE;
      af.level = 0;
      af.duration = 200;
      af.modifier = DISEASE_FROSTBITE;
      af.location = APPLY_NONE;
      af.bitvector = 0;
      affectTo(&af);
      disease_start(this, &af);
#endif
      break;
    default:
      break;
  }

  if (!outside() && !(dynamic_cast<TMonster *>(this))) {
    affectedData aff;
    aff.type = AFFECT_WAS_INDOORS;
    aff.level = 50;
    aff.duration = UPDATES_PER_MUDHOUR * 12;
    affectJoin(this, &aff, AVG_DUR_NO, AVG_EFF_YES, FALSE);
  }

  if (toggleInfo[TOG_QUESTCODE4]->toggle) {
    if (affectedBySpell(AFFECT_WAS_INDOORS) || hasDisease(DISEASE_FROSTBITE)) return FALSE;  // make it only hit em if they sit outside for a while
    if (weather_info.sky != SKY_RAINING && weather_info.sky != SKY_CLOUDY && weather_info.sky != SKY_LIGHTNING)
      return FALSE;
    if (isImmune(IMMUNE_COLD, WEAR_BODY))
      return FALSE;
    if (dynamic_cast<TMonster *>(this))
      return FALSE;
    if (isPc() && isImmortal())
      return FALSE;

    af.type = AFFECT_DISEASE;
    af.level = 0;
    af.duration = 200;
    af.modifier = DISEASE_FROSTBITE;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affectTo(&af);
    disease_start(this, &af);
  }

  getWet(this, roomp, SILENT_NO);

  return FALSE;
}

int TBeing::passOut()
{
  affectedData af;
  int rc;

  // Coded by : Glint
  // Coded on : 04/10/2001
  //
  // Old Code for passout Check : {
  // int drunk = max(1,plotStat(STAT_CURRENT,STAT_CON,13,28,23)-getCond(DRUNK))
  // if (::number(0,drunk))
  // return FALSE;
  // } end of Old Code
  //
  // Modified the chance of passing out from being drunk.  
  // If just drunk enough to pass out you have 1/8th chance (human)
  // If MAX drunk, 50% chance to pass out.
  // Constitution is used to modify chance of passing out.

  int getDrunk = getCond(DRUNK)-14;
  int chancePassOut = (int)(4.17*(double)getDrunk+8.33);
  double conEffect = 1.0 - plotStat(STAT_CURRENT, STAT_CON, 0.25, 0.75, 0.50);
  chancePassOut = (int)((double)chancePassOut * 2.0 * conEffect);
  if (::number(1,100) > chancePassOut)
    return FALSE;
  af.type = AFFECT_DRUNK;
  af.level = max(1, (int) getCond(DRUNK));
  af.duration = max(1,getCond(DRUNK) - 15);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SLEEP;
  affectJoin(NULL, &af, AVG_DUR_NO, AVG_EFF_NO);

  if (awake())
    sendTo("You pass out from too much drink.\n\r");

  if (getPosition() > POSITION_SLEEPING) {
    if (riding) {
      rc = fallOffMount(riding, POSITION_STANDING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    }
    if (awake()) {
      act("$n passes out!", TRUE, this, NULL, NULL, TO_ROOM);
      setPosition(POSITION_SLEEPING);
    }
  }
  return TRUE;
}

int TBeing::bumpHead(int *iHeight)
{
  *iHeight = roomp->getRoomHeight();

  if (!willBumpHead(roomp))
    return FALSE;
  
  if (!isPc() && !inImperia() && master && isAffected(AFF_CHARM))
    vlogf(LOG_LOW,fmt("Being (%s) (height %d) in small room (#%d)(roof: %d)") % 
           getName() % getPosHeight() % roomp->number % *iHeight); 

  int num = max(0,(getPosHeight() - *iHeight));
  if (::number(0, num) && (!isPc() || desc)) {
    sendTo("You bump your head on the ceiling.  OUCH!\n\r");
    act("$n bumps $s head on the ceiling.  That had to hurt.", 
              TRUE, this, 0,0,TO_ROOM);
    reconcileDamage(this,::number(1,3),DAMAGE_NORMAL);
  }
  return TRUE;
}

void TBeing::checkCharmMana()
{
  followData *k, *k2;
  int rc;
  TBeing *ch;

  if (!isPc())
    return;
  if (!followers)
    return;

  for (k = followers;k; k = k2) {
    k2 = k->next;
    if (!(ch = k->follower) || !dynamic_cast<TBeing *>(ch)) {
      vlogf(LOG_BUG, fmt("Non-Tbeing in followers of %s") %  getName());
      return;
    }
    bool ischarm = isPet(PETTYPE_CHARM);
    bool isthrall = isPet(PETTYPE_THRALL);
    if (!ischarm && !isthrall)
      continue;
 
    if (!sameRoom(*ch)) {
      ch->stopFollower(TRUE);
      continue;
    }
    int mana = 0;
  
    mana = ch->GetMaxLevel()/5 + 1;

    mana += plotStat(STAT_CURRENT, STAT_FOC, 16, 1, 8);
    mana += plotStat(STAT_CURRENT, STAT_CHA, 16, 1, 8);

    // thralls owe lifeforce to master, so don't fight master's will as much
    if (isthrall)
      mana = 2*mana/3;

    if (!hasClass(CLASS_CLERIC) && !hasClass(CLASS_DEIKHAN) && !hasClass(CLASS_SHAMAN)) {
      if (getMana() < mana && isPc()) {
        sendTo(COLOR_MOBS, fmt("You lack the mental concentration to control %s any longer.\n\r") % ch->getName());
        ch->stopFollower(TRUE);
        if (ch->fight()) {
          rc = ch->doFlee("");
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete ch;
            ch = NULL;
          }
        }
        setMana(0);
      } else {
        if (isthrall)
          act("The effort of keeping $N enthralled weakens you.", 
               FALSE, this, 0, ch, TO_CHAR);
        else
          act("The effort of keeping $N charmed weakens you.", 
               FALSE, this, 0, ch, TO_CHAR);

        reconcileMana(TYPE_UNDEFINED, FALSE, mana);
      }
    } else {
      // clerics and deikhans
      double piety;
      piety = (double) mana / 6.0;
      if (getPiety() < piety) {
        sendTo(COLOR_MOBS, fmt("You lack the concentration to control %s any longer.\n\r") % ch->getName());
        ch->stopFollower(TRUE);
        if (ch->fight()) {
          rc = ch->doFlee("");
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete ch;
            ch = NULL;
          }
        }
        setPiety(0.0);
      } else {
        if (isthrall)
          act("The effort of keeping $N enthralled weakens you.", 
               FALSE, this, 0, ch, TO_CHAR);
        else
          act("The effort of keeping $N charmed weakens you.", 
               FALSE, this, 0, ch, TO_CHAR);

        addToPiety(-piety);
      }
    }
  }
  return;
}

// procAutoTips
procAutoTips::procAutoTips(const int &p)
{
  trigger_pulse=p;
  name="procAutoTips";
}

void procAutoTips::run(int pulse) const
{
  // first, get a tip...
  FILE *fp = fopen("tipsfile", "r");
  if (!fp) {
    vlogf(LOG_SILENT, "Failed opening tipsfile.");
    return;
  }
  // skip any comments
  char buf[256];
  do {
    fgets(buf, 255, fp);
  } while (*buf == '#');

  // how many tips do we have?
  unsigned int numtips = 0;
  while (fgets(buf, 255, fp)) {
    if (*buf != '#')
      numtips++;
  }
  if (!numtips) {
    fclose(fp);
    return;
  }
  unsigned int num = ::number(1, numtips);

  // now startover and get the num'th tip
  rewind(fp);
  numtips = 0;
  do {
    fgets(buf, 255, fp);
    if (*buf != '#')
      numtips++;
  } while (numtips < num);
  
  // clean up the tip
  if (strlen(buf) >= 1)
    buf[strlen(buf) - 1] = '\0';

  fclose(fp);

  Descriptor *d;
  for (d = descriptor_list; d; d = d->next) {
    if (d->connected != CON_PLYNG)
      continue;
    TBeing *ch = d->character;
    if (!ch)
      continue;
    if (!IS_SET(d->autobits, AUTO_TIPS) || ch->isImmortal())
      continue;
    ch->sendTo(COLOR_BASIC, fmt("<y>%s Tip :<z> %s\n\r") % MUD_NAME % buf);
  }
}
