//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"

int RecGetObjRoom(const TThing *obj)
{
  if (obj->in_room != ROOM_NOWHERE)
    return(obj->in_room);
  if (dynamic_cast<TBeing *>(obj->parent))
    return(obj->parent->in_room);
  if (obj->equippedBy)
    return(obj->equippedBy->in_room);
  if (obj->parent)
    return(RecGetObjRoom(obj->parent));
  if (obj->riding)
    return(RecGetObjRoom(obj->riding));

  return ROOM_NOWHERE;
}

void MakeRoomNoise(TMonster *mob, int room, const char *local_snd, const char *distant_snd)
{
  dirTypeT door;
  TBeing *ch = NULL;
  TRoom *rp, *orp;
  TThing *t;
  char buf[256];

  if ((rp = real_roomp(room)) && local_snd && *local_snd) {
    for (t = rp->getStuff(); t; t = t->nextThing) {
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
        for (t = orp->getStuff(); t; t = t->nextThing) {
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
    for (t = rp->getStuff(); t; t = t->nextThing) {
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
    vlogf(LOG_MISC, fmt("Testing log: No rp in MakeNoise for %s") %  ((ch->name) ? ch->name : "null"));
    return;
  }
  for (door = MIN_DIR; door < MAX_DIR; door++) {
    if (rp->dir_option[door] && (orp = real_roomp(rp->dir_option[door]->to_room))) {
      for (t = orp->getStuff(); t; t = t->nextThing) {
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
  TThing *ch;
  for (ch = getStuff(); ch; ch = ch->nextThing) {
    TBeing * chb = dynamic_cast<TBeing *>(ch);
    if (chb)
      chb->playsound(sound, type, vol, prior, loop);
  }
}

void TRoom::stopsound() const
{
  TThing *ch;
  for (ch = getStuff(); ch; ch = ch->nextThing) {
    TBeing * chb = dynamic_cast<TBeing *>(ch);
    if (chb)
      chb->stopsound();
  }
}

void TBeing::stopmusic()
{
  if (desc) {
    if (IS_SET(desc->account->flags, ACCOUNT_MSP) || desc->m_bIsClient) {
      // the U= command for MSP is supposed to set a default download
      // directory, so it oonly needs to be sent once, prior to all downloads
      // we will send a stopsound() when they enable MSP
      sstring url = "http://sneezy.saw.net/sounds/";
      sendTo(fmt("!!MUSIC(Off U=%s)\n\r") % url);
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
    "combat_01.mid",
    "combat_02.mid",
    "combat_03.mid",
  };

  if (desc) {
    if ((IS_SET(desc->account->flags, ACCOUNT_MSP) || desc->m_bIsClient) &&
        music >= MIN_MUSIC_NUM && music < MAX_MUSIC_NUM) {
      // please note, we do NOT send the U= command intentionally.
      // According to Zugg (of zMud), the U is meant to set a default
      // download location which should be saved and used as appropriate
      // the U is only needed the first time, hence I put it in stopmusic()
      // the other options should only get sent if they differ from the
      // defaults (to lessen spam)
      sstring argString = "";
      char buf[160];
      if (vol != 100) {
        sprintf(buf, " V=%d", vol);
        argString += buf;
      }
      if (cont != 1) {
        sprintf(buf, " C=%d", cont);
        argString += buf;
      }
      if (loop != 1) {
        sprintf(buf, " L=%d", loop);
        argString += buf;
      }
      sendTo(fmt("!!MUSIC(%s T=%s%s)\n\r") %
            musicStruct[music] % 
            type % argString);
    }
  }
}

void TBeing::stopsound()
{
  if (desc) {
    if (IS_SET(desc->account->flags, ACCOUNT_MSP) || desc->m_bIsClient) {
      // the U= command for MSP is supposed to set a default download
      // directory, so it oonly needs to be sent once, prior to all downloads
      // we will send a stopsound() when they enable MSP
      sstring url = "http://sneezy.saw.net/sounds/";
      sendTo(fmt("!!SOUND(Off U=%s)\n\r") % url);
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
    "snore.wav",
    "horse1.wav",
    "horse2.wav",
    "angrycat.wav",
    "bark1.wav",
    "bark2.wav",
    "yawn1.wav",
    "yawn2.wav",
    "yawn3.wav",
    "yawn4.wav",
    "thunder1.wav",
    "thunder2.wav",
    "thunder3.wav",
    "thunder4.wav",
    "thunder5.wav",
    "giggle.wav",
    "laugh.wav",
    "burp.wav",
    "bogus.wav",  // clap
    "fart.wav",  // fart
    "sneeze.wav",
    "cackle.wav",
    "Dm-laugh.wav",
    "scream.wav",
    "hit_blunt1.wav",
    "hit_blunt2.wav",
    "hit_blunt3.wav",
    "hit_blunt4.wav",
    "hit_blunt5.wav",
    "hit_blunt6.wav",
    "hit_blunt7.wav",
    "hit_blunt8.wav",
    "hit_blunt9.wav",
    "hit_blunt10.wav",
    "hit_blunt11.wav",
    "hit_blunt12.wav",
    "hit_blunt13.wav",
    "hit_blunt14.wav",
    "hit_blunt15.wav",
    "hit_blunt16.wav",
    "hit_blunt17.wav",
    "hit_blunt18.wav",
    "hit_slash1.wav",
    "hit_slash2.wav",
    "hit_slash3.wav",
    "hit_slash4.wav",
    "hit_pierce1.wav",
    "hit_whip1.wav",
    "hit_whip2.wav",
    "miss1.wav",
    "miss2.wav",
    "miss3.wav",
    "miss4.wav",
    "miss5.wav",
    "miss6.wav",
    "miss7.wav",
    "miss8.wav",
    "miss9.wav",
    "miss10.wav",
    "pathetic1.wav",
    "pathetic2.wav",
    "pathetic3.wav",
    "pathetic4.wav",
    "death_cry1.wav",
    "death_cry2.wav",
    "death_cry3.wav",
    "death_cry4.wav",
    "death_cry5.wav",
    "death_cry6.wav",
    "death_cry7.wav",
    "death_cry8.wav",
    "death_cry9.wav",
    "death_cry10.wav",
    "death_cry11.wav",
    "dont_kill_me.wav",
    "backstab1.wav",
    "backstab2.wav",
    "dooropen1.wav",
    "dooropen2.wav",
    "doorclose1.wav",
    "doorclose2.wav",
    "doorclose3.wav",
    "doorclose4.wav",
    "banzai1.wav",
    "banzai2.wav",
    "banzai3.wav",
    "banzai4.wav",
    "water_drop.wav",
    "water_gurgle.wav",
    "water_wave.wav",
    "boing.wav",
    "come_back.wav",
    "nonepass.wav",
    "flee1.wav",
    "flee2.wav",
    "flee3.wav",
    "crit_01.wav",
    "crit_02.wav",
    "crit_03.wav",
    "crit_04.wav",
    "crit_05.wav",
    "crit_06.wav",
    "crit_07.wav",
    "crit_08.wav",
    "crit_09.wav",
    "crit_10.wav",
    "crit_11.wav",
    "crit_12.wav",
    "crit_13.wav",
    "crit_14.wav",
    "crit_15.wav",
    "crit_16.wav",
    "crit_17.wav",
    "crit_18.wav",
    "crit_19.wav",
    "crit_20.wav",
    "crit_21.wav",
    "crit_22.wav",
    "crit_23.wav",
    "crit_24.wav",
    "crit_25.wav",
    "crit_26.wav",
    "crit_27.wav",
    "crit_28.wav",
    "crit_29.wav",
    "crit_30.wav",
    "crit_31.wav",
    "crit_32.wav",
    "crit_33.wav",
    "crit_34.wav",
    "crit_35.wav",
    "crit_36.wav",
    "crit_37.wav",
    "crit_38.wav",
    "crit_39.wav",
    "crit_40.wav",
    "crit_41.wav",
    "crit_42.wav",
    "crit_43.wav",
    "shock.wav",
    "yahoo.wav",
    "chewed_up.wav",
    "disagree.wav",
    "foodpoison.wav",
    "bring_dead.wav",
    "egoblast1.wav",
    "egoblast2.wav",
    "cast_fail_1.wav",
    "cast_fail_2.wav",
    "rain_start.wav",
    "spell_accelerate.wav",
    "spell_animate_dead.wav",
    "spell_armor.wav",
    "spell_astral_walk.wav",
    "spell_bless.wav",
    "spell_call_lightning.wav",
    "spell_granite_fist.wav",
    "spell_haste.wav",
    "spell_hellfire.wav",
    "spell_meteor_swarm.wav",
    "spell_mystic_dart.wav",
    "spell_pillar_of_salt.wav",
    "spell_portal.wav",
    "spell_rain_brimstone.wav",
    "spell_sanctuary.wav",
    "spell_spontaneous_combust.wav",
    "spell_teleport.wav",
    "spell_word_of_recall.wav",
    "spell_sorcerers_globe.wav",
    "spell_sling_shot.wav",
    "spell_stunning_arrow.wav",
  };

  if (desc) {
    if ((desc->account && IS_SET(desc->account->flags, ACCOUNT_MSP) || desc->m_bIsClient) &&
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

      sstring argString = "";
      char buf[160];
      if (vol != 100) {
        sprintf(buf, " V=%d", vol);
        argString += buf;
      }
      if (prior != 50) {
        sprintf(buf, " P=%d", prior);
        argString += buf;
      }
      if (loop != 1) {
        sprintf(buf, " L=%d", loop);
        argString += buf;
      }
      sendTo(fmt("!!SOUND(%s T=%s%s)\n\r") %
            soundStruct[sound] % 
            type % argString);
    }
  }
}

