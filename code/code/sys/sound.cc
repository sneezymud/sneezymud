//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "room.h"
#include "being.h"
#include "low.h"
#include "account.h"
#include "extern.h"
#include "colorstring.h"
#include "monster.h"

int RecGetObjRoom(const TThing *obj)
{
  if (obj->in_room != Room::NOWHERE)
    return(obj->in_room);
  if (dynamic_cast<TBeing *>(obj->parent))
    return(obj->parent->in_room);
  if (obj->equippedBy)
    return(obj->equippedBy->in_room);
  if (obj->parent)
    return(RecGetObjRoom(obj->parent));
  if (obj->riding)
    return(RecGetObjRoom(obj->riding));

  return Room::NOWHERE;
}

void MakeRoomNoise(TMonster *mob, int room, const char *local_snd, const char *distant_snd)
{
  dirTypeT door;
  TBeing *ch = NULL;
  TRoom *rp, *orp;
  TThing *t=NULL;
  char buf[256];

  if ((rp = real_roomp(room)) && local_snd && *local_snd) {
    for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
      ch = dynamic_cast<TBeing *>(t);
      if (!ch || !ch->desc) {
        continue;
      }
      if (ch->awake()) {
        sprintf(buf, "%s", addNameToBuf(ch, ch->desc, mob, local_snd, COLOR_MOBS).c_str());
        ch->sendTo(COLOR_BASIC, buf);
      }
    }
  }

  if (rp && distant_snd) {
    for (door = MIN_DIR; door < MAX_DIR; door++) {
      if (rp->dir_option[door] && (orp = real_roomp(rp->dir_option[door]->to_room))) {
        for(StuffIter it=orp->stuff.begin();it!=orp->stuff.end() && (t=*it);++it) {
          ch = dynamic_cast<TBeing *>(t);
          if (!ch) {
            continue;
          }
          if (ch->desc && !IS_SET(ch->desc->autobits, AUTO_NOSHOUT) && ch->awake()) {
            sprintf(buf, "%s", addNameToBuf(ch, ch->desc, mob, distant_snd, COLOR_MOBS).c_str());
//            sprintf(buf, "%s", colorString(ch, ch->desc, distant_snd, NULL, COLOR_BASIC, TRUE));
            ch->sendTo(COLOR_BASIC, buf);
          }
        }
      }
    }
  }
}

void MakeNoise(int room, const char *local_snd, const char *distant_snd)
{
  dirTypeT door;
  TBeing *ch = NULL;
  TRoom *rp = NULL, *orp = NULL;
  TThing *t = NULL;

  if ((rp = real_roomp(room))) {
    for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
      ch = dynamic_cast<TBeing *>(t);
      if (!ch) {
        continue;
      }
      if (ch->awake()) {
        ch->sendTo(COLOR_BASIC, local_snd);
      }
    }
  }
  if (!rp) {
    vlogf(LOG_MISC, format("Testing log: No rp in MakeNoise for %s") %  ((ch->name) ? ch->name : "null"));
    return;
  }
  for (door = MIN_DIR; door < MAX_DIR; door++) {
    if (rp->dir_option[door] && (orp = real_roomp(rp->dir_option[door]->to_room))) {
      for(StuffIter it=orp->stuff.begin();it!=orp->stuff.end() && (t=*it);++it) {
        ch = dynamic_cast<TBeing *>(t);
        if (!ch) {
          continue;
        }
	if (ch->desc && !IS_SET(ch->desc->autobits, AUTO_NOSHOUT) && ch->awake()) {
	  ch->sendTo(COLOR_BASIC, distant_snd);
        }
      }
    }
  }
}

int noise(const TBeing *ch)
{
  int i, total = 0;
  affectedData *af;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if ((i == WEAR_FINGER_L) || (i == WEAR_FINGER_R) || 
        (i == HOLD_RIGHT) || (i == HOLD_LEFT))
      continue;

    TObj *tobj = dynamic_cast<TObj *>(ch->equipment[i]);
    if (tobj)
      total += tobj->itemNoise();
  }
  if (ch->isAffected(AFF_SNEAK) && !ch->fight())
    total -= 1 + ch->GetMaxLevel() / 2;

  // if in natural environs, reduce even further
  // traps areas that SHOULDN'T aid them
  if (ch->getRace() == RACE_ELVEN) {
    switch (ch->roomp->getSectorType()) {
      case SECT_ARCTIC_CITY:
      case SECT_ARCTIC_ROAD:
      case SECT_ARCTIC_BUILDING:
      case SECT_TEMPERATE_CITY:
      case SECT_TEMPERATE_ROAD:
      case SECT_TEMPERATE_BUILDING:
      case SECT_TROPICAL_CITY:
      case SECT_TROPICAL_ROAD:
      case SECT_TROPICAL_BUILDING:
        break;
      default:
        total -= 20;
      break;
    }
  }

  // adjust for height, ogres = noisy, hobbits quiet
  total += ch->getHeight() - 70;

  // big, strong, loud and arrogant
  if ((ch->getRace() == RACE_OGRE) || (ch->getRace() == RACE_DWARF))
    total += 20;

  for (af = ch->affected; af; af = af->next) {
    if (af->location == APPLY_NOISE) {
      total += af->modifier;
    }
  }
  return (total);
}

int TBeing::applySoundproof() const
{
  if (checkSoundproof() && !isImmortal()) {
    sendTo("You are in a silence zone, you can't make a sound!\n\r");
    return TRUE;		/* for shouts, emotes, etc */
  }
  return FALSE;
}

int TThing::checkSoundproof() const
{
  return (roomp->isRoomFlag(ROOM_SILENCE));
}

void TRoom::playsound(soundNumT sound, const sstring &type, int vol, int prior, int loop) const
{
  for(StuffIter it=stuff.begin();it!=stuff.end();++it) {
    TBeing * chb = dynamic_cast<TBeing *>(*it);
    if (chb)
      chb->playsound(sound, type, vol, prior, loop);
  }
}

void TRoom::stopsound() const

{
  for(StuffIter it=stuff.begin();it!=stuff.end();++it) {
    TBeing * chb = dynamic_cast<TBeing *>(*it);
    if (chb)
      chb->stopsound();
  }
}

void TBeing::stopmusic()
{
  if (desc) {
    if (IS_SET(desc->account->flags, TAccount::MSP) || desc->m_bIsClient) {
      // the U= command for MSP is supposed to set a default download
      // directory, so it oonly needs to be sent once, prior to all downloads
      // we will send a stopsound() when they enable MSP
      sendTo(new SoundComm("music", "http://sneezymud.com/sounds/", "Off",
			   "", -1, -1, -1, -1));
    }
  }
}

const sstring & MUSIC_TYPE_DEATH = "death";
const sstring & MUSIC_TYPE_ZONE = "zone";
const sstring & MUSIC_TYPE_COMBAT = "combat";

void TBeing::playmusic(musicNumT music, const sstring &type, int vol, int cont, int loop)
{
  const sstring musicStruct[MAX_MUSIC_NUM] = {
    "Off",
    "combat_01",
    "combat_02",
    "combat_03",
  };

  if (desc) {
    if ((IS_SET(desc->account->flags, TAccount::MSP) || desc->m_bIsClient) &&
        music >= MIN_MUSIC_NUM && music < MAX_MUSIC_NUM) {
      // please note, we do NOT send the U= command intentionally.
      // According to Zugg (of zMud), the U is meant to set a default
      // download location which should be saved and used as appropriate
      // the U is only needed the first time, hence I put it in stopmusic()
      // the other options should only get sent if they differ from the
      // defaults (to lessen spam)

      sendTo(new SoundComm("music", "", musicStruct[music], type,
			   vol, -1, loop, cont));
      
    }
  }
}

void TBeing::stopsound()
{
  if (desc) {
    if (IS_SET(desc->account->flags, TAccount::MSP) || desc->m_bIsClient) {
      // the U= command for MSP is supposed to set a default download
      // directory, so it oonly needs to be sent once, prior to all downloads
      // we will send a stopsound() when they enable MSP
      sendTo(new SoundComm("sound", "http://sneezymud.com/sounds/", "Off", 
			   "", -1,-1,-1, -1));
    }
  }
}

const sstring & SOUND_TYPE_SOCIAL = "socials";
const sstring & SOUND_TYPE_NOISE = "noise";
const sstring & SOUND_TYPE_COMBAT = "combat";
const sstring & SOUND_TYPE_MAGIC = "magic";

// randomly chooses a sound from a sequential range
// we should group any sounds we want together
soundNumT pickRandSound(soundNumT s1, soundNumT s2)
{
  return soundNumT(::number(s1, s2));
}

// randomly chooses a song from a sequential range of musics
// we should group any musics we want together
musicNumT pickRandMusic(musicNumT s1, musicNumT s2)
{
  return musicNumT(::number(s1, s2));
}

void TBeing::playsound(soundNumT sound, const sstring &type, int vol, int prior, int loop)
{
  const sstring soundStruct[MAX_SOUND_NUM] = {
    "Off",
    "snore",
    "horse1",
    "horse2",
    "angrycat",
    "bark1",
    "bark2",
    "yawn1",
    "yawn2",
    "yawn3",
    "yawn4",
    "thunder1",
    "thunder2",
    "thunder3",
    "thunder4",
    "thunder5",
    "giggle",
    "laugh",
    "burp",
    "bogus",  // clap
    "fart",  // fart
    "sneeze",
    "cackle",
    "Dm-laugh",
    "scream",
    "hit_blunt1",
    "hit_blunt2",
    "hit_blunt3",
    "hit_blunt4",
    "hit_blunt5",
    "hit_blunt6",
    "hit_blunt7",
    "hit_blunt8",
    "hit_blunt9",
    "hit_blunt10",
    "hit_blunt11",
    "hit_blunt12",
    "hit_blunt13",
    "hit_blunt14",
    "hit_blunt15",
    "hit_blunt16",
    "hit_blunt17",
    "hit_blunt18",
    "hit_slash1",
    "hit_slash2",
    "hit_slash3",
    "hit_slash4",
    "hit_pierce1",
    "hit_whip1",
    "hit_whip2",
    "miss1",
    "miss2",
    "miss3",
    "miss4",
    "miss5",
    "miss6",
    "miss7",
    "miss8",
    "miss9",
    "miss10",
    "pathetic1",
    "pathetic2",
    "pathetic3",
    "pathetic4",
    "death_cry1",
    "death_cry2",
    "death_cry3",
    "death_cry4",
    "death_cry5",
    "death_cry6",
    "death_cry7",
    "death_cry8",
    "death_cry9",
    "death_cry10",
    "death_cry11",
    "dont_kill_me",
    "backstab1",
    "backstab2",
    "dooropen1",
    "dooropen2",
    "doorclose1",
    "doorclose2",
    "doorclose3",
    "doorclose4",
    "banzai1",
    "banzai2",
    "banzai3",
    "banzai4",
    "water_drop",
    "water_gurgle",
    "water_wave",
    "boing",
    "come_back",
    "nonepass",
    "flee1",
    "flee2",
    "flee3",
    "crit_01",
    "crit_02",
    "crit_03",
    "crit_04",
    "crit_05",
    "crit_06",
    "crit_07",
    "crit_08",
    "crit_09",
    "crit_10",
    "crit_11",
    "crit_12",
    "crit_13",
    "crit_14",
    "crit_15",
    "crit_16",
    "crit_17",
    "crit_18",
    "crit_19",
    "crit_20",
    "crit_21",
    "crit_22",
    "crit_23",
    "crit_24",
    "crit_25",
    "crit_26",
    "crit_27",
    "crit_28",
    "crit_29",
    "crit_30",
    "crit_31",
    "crit_32",
    "crit_33",
    "crit_34",
    "crit_35",
    "crit_36",
    "crit_37",
    "crit_38",
    "crit_39",
    "crit_40",
    "crit_41",
    "crit_42",
    "crit_43",
    "shock",
    "yahoo",
    "chewed_up",
    "disagree",
    "foodpoison",
    "bring_dead",
    "egoblast1",
    "egoblast2",
    "cast_fail_1",
    "cast_fail_2",
    "rain_start",
    "spell_accelerate",
    "spell_animate_dead",
    "spell_armor",
    "spell_astral_walk",
    "spell_bless",
    "spell_call_lightning",
    "spell_granite_fist",
    "spell_haste",
    "spell_hellfire",
    "spell_meteor_swarm",
    "spell_mystic_dart",
    "spell_pillar_of_salt",
    "spell_portal",
    "spell_rain_brimstone",
    "spell_sanctuary",
    "spell_spontaneous_combust",
    "spell_teleport",
    "spell_word_of_recall",
    "spell_sorcerers_globe",
    "spell_sling_shot",
    "spell_stunning_arrow",
  };

  if (desc) {
    if ((desc->account && IS_SET(desc->account->flags, TAccount::MSP) || desc->m_bIsClient) &&
        sound >= MIN_SOUND_NUM && sound < MAX_SOUND_NUM) {
      // please note, we do NOT send the U= command intentionally.
      // According to Zugg (of zMud), the U is meant to set a default
      // download location which should be saved and used as appropriate
      // the U is only needed the first time, hence I put it in stopsound()
      // the other options should only get sent if they differ from the
      // defaults (to lessen spam)

      // This is done so that the client message isn't somehow combined 
      // with other text and missed by the client interpreter - Russ 061299
      desc->outputProcessing();

      sendTo(new SoundComm("sound", "", soundStruct[sound], type,
			   vol, prior, loop, -1));
    }
  }
}

sstring SoundComm::getXML(){
  sstring buf="";

  buf+=format("<sound type=\"%s\">\n") % soundtype;

  buf+=format("  <file>%s</file>\n") % text;

  if(type!="")
    buf+=format("  <type>%s</type>\n") % type;

  if(url!="")
    buf+=format("  <url>%s</url>\n") % url;

  if(volume!=-1)
    buf+=format("  <volume>%i</volume>\n") % volume;

  if(priority!=-1)
    buf+=format("  <priority>%i</priority>\n") % priority;

  if(cont!=-1)
    buf+=format("  <continue>%i</continue>\n") % (cont?"true":"false");

  if(repeats!=-1)
    buf+=format("  <loop>%i</loop>\n") % repeats;

  buf+=format("</sound>\n");

  return buf;
}

sstring SoundComm::getText(){
  sstring buf="";

  buf+=format("!!%s(%s") % soundtype.upper()% text;
  
  if(type!="")
    buf+=format(" T=%s") % type;

  if(url!="")
    buf+=format(" U=%s") % url;

  if(volume!=-1)
    buf+=format(" V=%i") % volume;

  if(priority!=-1)
    buf+=format(" P=%i") % priority;

  if(repeats!=-1)
    buf+=format(" L=%i") % repeats;

  if(cont!=-1)
    buf+=format(" C=%i") % cont;


  buf+=format(")\n\r");

  return buf;
}

sstring SoundComm::getClientText(){
  return getText();
}

