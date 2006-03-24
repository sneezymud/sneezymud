/* *************************************************************************
*  file: spec_procs2.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information.  *
*************************************************************************  */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "s_list.h"
#include "opinion.h"
#include "hash.h"
#include "area.h"
#include "race.h"

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern struct weather_data weather_info;
   extern int top_of_world;
   extern struct int_app_type int_app[26];

extern struct title_type titles[8][ABS_MAX_LVL];
extern char *dirs[]; 

/* extern procedures */

void hit(struct char_data *ch, struct char_data *victim, int type);
void gain_exp(struct char_data *ch, int gain);
struct char_data *FindVictim( struct char_data *ch);
int is_target_room_p(int room, void *tgt_room);
struct char_data *char_holding( struct obj_data *obj);
void send_to_all(char *messg);
void do_shout(struct char_data *ch, char *argument, int cmd);
int IsUndead( struct char_data *ch);
struct time_info_data age(struct char_data *ch);
int CountLims(struct obj_data *obj);
struct char_data *FindAnAttacker( struct char_data *ch);
void NailThisSucker( struct char_data *ch);
void BouncerThrow(struct char_data *ch);
int NumCharmedFollowersInRoom(struct char_data *ch);
struct char_data *FindMobDiffZoneSameRace(struct char_data *ch);
struct char_data *FindMobInRoomWithFunction(int room, int (*func)());

/* beginning of Batoprs shit */

int bouncer(struct char_data *ch, int cmd, char *arg)
{
   struct char_data *tmp_victim, *temp;
   int dir;

   if (cmd || !AWAKE(ch) || !ch->specials.fighting)
     return(FALSE);

   for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
       if (tmp_victim->specials.fighting) {
         if (tmp_victim->specials.fighting == ch) { 
          if (!saves_spell(tmp_victim, SAVING_PETRI)) {
            BouncerThrow(tmp_victim);
          }
        }
      }
    }
  }
}

int gilbert(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *victim;
    struct obj_data *o;
    int num;
    char buf[255];


    if (cmd) return(FALSE);

    if (!AWAKE(ch)) return(FALSE);

    num = number(1,2);

    if (num == 1) return(FALSE);

    if (!ch->specials.fighting) return(FALSE);
     
    victim = ch->specials.fighting;

    act("$n becomes disgusted by $N and pulls a scroll of recall from his pocket.", FALSE, ch, 0, victim, TO_ROOM);
    o = read_object(3052,VIRTUAL);
    obj_to_char(o,ch);
    send_to_room("Gilbert screams ' Scum like you don't belong in my bar!\n\r", ch->in_room);
  if (!saves_spell(victim, SAVING_PARA)) {
    sprintf(buf, "scroll %s", GET_NAME(victim));
    do_recite(ch, buf,207);
    sprintf(buf, "%s .......and stay out.", GET_NAME(victim));
    do_tell(ch, buf, 19);
  } else {
    send_to_char("As Gilbert is about to recite the scroll, you jolt his arm and he drops it.\n\r", victim);
    act("$N slams $n into the wall and jolts the scroll from his hand.",
             FALSE, ch, 0, victim, TO_NOTVICT);
    obj_from_char(o);
    obj_to_room(o, ch->in_room);
   }
}

    
    


int toilet_thing(struct char_data *ch, int cmd, char *arg)
{

   struct char_data *tmp_victim, *temp;
   int dam;

   if (cmd) return(FALSE);
  
   if (!AWAKE(ch)) return(FALSE);

   if (!ch->specials.fighting) return(FALSE);

   dam = 40+(dice(1,15));

   send_to_room("The thing floating in the toilet spews forth filth and noxious crud.\n\r", ch->in_room);

   for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
       if (IS_IMMORTAL(tmp_victim))
        send_to_char("You are unaffected by the funk from the toilet.\n\r", tmp_victim);
       else
        MissileDamage(ch, tmp_victim, dam, 300);
      }
   }
}

int zombie_hater(struct char_data *ch, int cmd, char *arg)
{
     struct char_data *vict, *next_v;
     
     if (cmd || !AWAKE(ch) || !ch->specials.fighting)
        return(FALSE);

   for (vict = real_roomp(ch->in_room)->people; vict; vict = next_v) {
        next_v = vict->next_in_room;
       if (GET_RACE(vict) == RACE_UNDEAD) {
        act("$n screams 'How dare you let an undead creature attack me!"
                     ,FALSE, ch, 0, vict, TO_ROOM);
        act("$n waves $s hands and totally demolishes $N", 
                      FALSE, ch, 0, vict, TO_ROOM);
        extract_char(vict);
      }
   } 
}

int dishboy(struct char_data *ch, int cmd, char *arg)
{
     struct char_data *tmp_victim, *temp;
     int dam;

     if (cmd || !AWAKE(ch) || !ch->specials.fighting)
        return(FALSE);

     dam = dice(1,10)+30;

   send_to_room("The dishboy pulls out a hose and aims it at you.\n\r", ch->in_room);
   for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
       if (IS_IMMORTAL(tmp_victim))
        send_to_char("The dishboys spray is turned away by your power.\n\r", ch);
       else
        MissileDamage(ch, tmp_victim, dam, 301);
      }
   }
}
     

int game_wizard(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *tmp_victim, *temp;
    int dam;


    if (cmd || !AWAKE(ch) || !ch->specials.fighting)
             return(FALSE);

    dam = dice(1,15)+25;
 
    send_to_room("The Game Wizard pulls a Super-Bomb from within his outfit and hurls it at you...\n\r", ch->in_room);

 for (tmp_victim = character_list;tmp_victim;tmp_victim = temp) {
  temp = tmp_victim->next;
   if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
    if(IS_IMMORTAL(tmp_victim))
      send_to_char("You spit on the game wizards super bomb and dissolve it.\n\r", tmp_victim);
    else
        MissileDamage(ch,tmp_victim,dam,302);
    }
  }
}

     



/* monks */

int monk(struct char_data *ch, int cmd, char *arg)
{
  
  if (cmd || !AWAKE(ch))
    return(FALSE);
    
  if (ch->specials.fighting) {
      MonkMove(ch);
  }
  return(FALSE);
}

int monk_master(struct char_data *ch, int cmd, char *arg)
{
  char buf[256];
  static char *n_skills[] = {
    "quivering palm", /* No. 245 */
    "feign death", /* No. 259 */
    "retreat",
    "kick",
    "hide",
    "sneak",
    "pick locks",
    "disarm",
    "dodge",
    "switch opponents",
    "springleap",
    "\n",
  };
  int percent=0, number=0;
  int charge, sk_num, mult;
 
  if (!AWAKE(ch))
    return(FALSE);  
 
  if (!cmd) {
    if (ch->specials.fighting) {
      return(fighter(ch, cmd, arg));
    }
    return(FALSE);
  }
 
  if (!ch->skills) return(FALSE);
 
  if (check_soundproof(ch)) return(FALSE);
  
  for(; *arg==' '; arg++); /* ditch spaces */
  
  if ((cmd==164)||(cmd==170)) {
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf, "You have %d practices remaining\n\r", ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      sprintf(buf," disarm  :        %s\n\r",how_good(ch->skills[SKILL_DISARM].learned));
      send_to_char(buf,ch);
      sprintf(buf," quivering palm:  %s [Must be 30th level]\n\r",how_good(ch->skills[SKILL_QUIV_PALM].learned));
      send_to_char(buf,ch);
      sprintf(buf," springleap:      %s\n\r",how_good(ch->skills[SKILL_SPRING_LEAP].learned));
      send_to_char(buf,ch);
      sprintf(buf," retreat:         %s\n\r",how_good(ch->skills[SKILL_RETREAT].learned));
      send_to_char(buf,ch);
      sprintf(buf," pick locks:      %s\n\r",how_good(ch->skills[SKILL_PICK_LOCK].learned));
      send_to_char(buf,ch);
      sprintf(buf," hide:            %s\n\r",how_good(ch->skills[SKILL_HIDE].learned));
      send_to_char(buf,ch);
      sprintf(buf," sneak:           %s\n\r",how_good(ch->skills[SKILL_SNEAK].learned));
      send_to_char(buf,ch);
      sprintf(buf," feign death:     %s\n\r",how_good(ch->skills[SKILL_FEIGN_DEATH].learned));
      send_to_char(buf,ch);
      sprintf(buf," dodge:           %s\n\r",how_good(ch->skills[SKILL_DODGE].learned));
      send_to_char(buf,ch);
      sprintf(buf," switch:          %s\n\r",how_good(ch->skills[SKILL_SWITCH_OPP].learned));
      send_to_char(buf,ch);
      sprintf(buf," kick:            %s\n\r",how_good(ch->skills[SKILL_KICK].learned));
      send_to_char(buf,ch);
      return(TRUE);
    } else {
      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("The ancient master says ",ch);
      if (number == -1) {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return(TRUE);
      }
      charge = GetMaxLevel(ch) * 100;
      switch(number) {
      case 0:
      case 1:
        sk_num = SKILL_QUIV_PALM;
        if (!HasClass(ch, CLASS_MONK)) {
          send_to_char("'You do not possess the proper skills'\n\r", ch);
          return(TRUE);
        } else if (GET_LEVEL(ch, MONK_LEVEL_IND) < 30) {
          send_to_char("'You are not high enough level'\n\r", ch);
          return(TRUE);
        }
        break;
      case 2:
        sk_num = SKILL_FEIGN_DEATH;
        if (!HasClass(ch, CLASS_MONK)) {
          send_to_char("'You do not possess the proper skills'\n\r", ch);
          return(TRUE);
        }
        break;
      case 3:
        sk_num = SKILL_RETREAT;
        if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK))) {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
      case 4:
        sk_num = SKILL_KICK;
        if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK))) {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
      case 5:
        sk_num = SKILL_HIDE;
        if (!HasClass(ch, CLASS_THIEF) && !(HasClass(ch, CLASS_MONK))) {
          send_to_char
            ("'You do not possess the necessary skills'\n\r",ch);
          return(TRUE);
        }
        break;
      case 6:
        sk_num = SKILL_SNEAK;
        if (!HasClass(ch, CLASS_THIEF) && !(HasClass(ch, CLASS_MONK))) {
          send_to_char
            ("'You do not possess the necessary skills'\n\r",ch);
          return(TRUE);
        }
        break;
      case 7:
        sk_num = SKILL_PICK_LOCK;
        if (!HasClass(ch, CLASS_THIEF) && !(HasClass(ch, CLASS_MONK))) {
          send_to_char
            ("'You do not possess the necessary skills'\n\r",ch);
          return(TRUE);
        }
        break;
      case 8:
        sk_num = SKILL_DISARM;
        if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK))) {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
      case 9:
        sk_num = SKILL_DODGE;
        if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK))) {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
      case 10:
        sk_num = SKILL_SWITCH_OPP;
        if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK))) {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
      case 11:
        sk_num = SKILL_SPRING_LEAP;
        if (!HasClass(ch, CLASS_MONK)) {
          send_to_char("'You do not possess the proper skills'\n\r", ch);
          return(TRUE);
        }
        break;
 
      default:
        sprintf(buf, "Strangeness in monk master (%d)", number);
        log(buf);
        send_to_char("'Ack!  I feel faint!'\n\r", ch);
        return(TRUE);
      }
    } 
    
    if (!HasClass(ch, CLASS_MONK) && GET_GOLD(ch) < charge){
      send_to_char
        ("'Ah, but you do not have enough money to pay.'\n\r",ch);
      return(TRUE);
    } 
    
    if ((sk_num == SKILL_SNEAK) || (sk_num == SKILL_DODGE)) {
      if (ch->skills[sk_num].learned >= 95) {
        send_to_char
          ("'You are a master of this art, I can teach you no more.'\n\r",ch);
        return(TRUE);
      }
    } else {
      if (ch->skills[sk_num].learned > 45) {
        send_to_char("'You must learn from practice and experience now.'\n\r", ch);
        return;
      }
    }
    
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char 
        ("'You must first use the knowledge you already have.\n\r",ch);
      return(TRUE);
    }
    
    if (!HasClass(ch, CLASS_MONK)) {
      GET_GOLD(ch) -= charge;
    }
    send_to_char("'We will now begin.'\n\r",ch);
    ch->specials.spells_to_learn--;
    
    percent = ch->skills[sk_num].learned +
      int_app[GET_INT(ch)].learn;
    ch->skills[sk_num].learned = MIN(95, percent);
 
    if (ch->skills[sk_num].learned >= 95) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
      return(TRUE);        
    }   
  } else if (cmd == 243){
    if (HasClass(ch, CLASS_MONK)) {
        GainLevel(ch, MONK_LEVEL_IND);
        return(TRUE);
        }
    
  } else {
    return(FALSE);
  }
}

#define NOD  35
#define MONK_MOB  650
#define FLEE 151
int monk_challenge_room(struct char_data *ch, int cmd, char *arg) 
{
  struct char_data *i;
  struct room_data *me;
  int rm;
 
   rm = ch->in_room;
 
   me = real_roomp(ch->in_room);
   if (!me) return(FALSE);
 
   if (!me->river_speed) return(FALSE);
 
   if (IS_PC(ch)) {
    REMOVE_BIT(ch->specials.act, PLR_WIMPY);
   }
 
   if (cmd == FLEE) {
     /* this person just lost */
     send_to_char("You lose\n\r",ch);
     if (IS_PC(ch)) {
       GET_EXP(ch) = MIN(titles[MONK_LEVEL_IND]
                         [GET_LEVEL(ch, MONK_LEVEL_IND)].exp, 
                         GET_EXP(ch));
       send_to_char("Go home\n\r", ch);
       char_from_room(ch);
       char_to_room(ch, rm-1);
       me->river_speed = 0;
       return(TRUE);
     } else {
         extract_char(ch);
         /*
           find pc in room;
           */
         for (i=me->people;i;i=i->next_in_room)
           if (IS_PC(i)) {
             GET_EXP(i) = MAX(titles[MONK_LEVEL_IND]
                              [GET_LEVEL(i, MONK_LEVEL_IND)+1].exp+1, 
                              GET_EXP(i));
             send_to_char("You win. You are obviously ready for the next level.\n\r", i);
             GainLevel(i, MONK_LEVEL_IND);
             char_from_room(i);
             char_to_room(i, rm-1);
             
             while (me->people)
               extract_char(me->people);
 
             while (me->contents)
               extract_obj(me->contents);
             
             me->river_speed = 0;
             return(TRUE);
       } else {
         return(FALSE);
       }
     }
   }
  return(FALSE);
 
}
 
int monk_challenge_prep_room(struct char_data *ch, int cmd, char *arg) 
{
  struct room_data *me, *chal;
  int i, newr;
  struct obj_data *o;
  struct char_data *mob;
 
   me = real_roomp(ch->in_room);
   if (!me) return(FALSE);
 
  chal = real_roomp(ch->in_room+1);
  if (!chal) {
    send_to_char("The challenge room is gone.. please contact a god\n\r", ch);
    return(TRUE);
  }
   
  if (cmd == NOD) {
 
    if (!HasClass(ch, CLASS_MONK)) {
      send_to_char("You're no monk\n\r", ch);
      return(FALSE);
    }
 
    if (GET_LEVEL(ch, MONK_LEVEL_IND) < 10) {
      send_to_char("You have no business here, kid.\n\r", ch);
      return(FALSE);
    }
 
    if (GET_EXP(ch) <= titles[MONK_LEVEL_IND]
                         [GET_LEVEL(ch, MONK_LEVEL_IND)+1].exp-100) {
      send_to_char("You cannot advance now\n\r", ch);
      return(TRUE);
    }
 
    if (chal->river_speed != 0) {
      send_to_char("The challenge room is busy.. please wait\n\r", ch);
      return(TRUE);
    }
    for (i=0;i<MAX_WEAR;i++) {
      if (ch->equipment[i]) {
        o = unequip_char(ch, i);
        obj_to_char(o, ch);
      }
    }
    while (ch->carrying)
      extract_obj(ch->carrying);
 
    spell_dispel_magic(IMPLEMENTOR,ch,ch,0);
    send_to_char("You are taken into the combat room.\n\r", ch);
    act("$n is ushered into the combat room", FALSE, ch, 0, 0, TO_ROOM);
    newr = ch->in_room+1;
    char_from_room(ch);
    char_to_room(ch, newr);
    /* load the mob at the same lev as char */
    mob = read_mobile(MONK_MOB+GET_LEVEL(ch, MONK_LEVEL_IND)-10, VIRTUAL);
    if (!mob) {
      send_to_char("The fight is called off.  go home\n\r", ch);
      return(TRUE);
    }
    char_to_room(mob, ch->in_room);
    chal->river_speed = 1;
    do_look(ch, "", 0);
    REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    return(TRUE);
  }
 
  return(FALSE);
}

/* Bjs Shit Begin */

#define Bandits_Path   2180
#define BASIL_GATEKEEPER_MAX_LEVEL 10
#define Fountain_Level 20
#define DEMON   29000

int ghost(struct char_data *ch, int cmd, char *arg)
{
  void cast_energy_drain( byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );

  if (cmd || !AWAKE(ch))
    return(FALSE);


  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                      ch->specials.fighting, 0);

    return TRUE;
  }
  return FALSE;
}

int Magic_Fountain(struct char_data *ch, int cmd, char *arg)
{

  int bits, water, level;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *tmp_char, *mob;
  struct obj_data *obj;
  int cost;

  extern int drink_aff[][3];

  extern struct weather_data weather_info;
        void name_to_drinkcon(struct obj_data *obj,int type);
        void name_from_drinkcon(struct obj_data *obj);


  if (cmd==11) { /* drink */

    cost = ((3*GetMaxLevel(ch)) * (2*GetMaxLevel(ch)));

    if (GET_GOLD(ch) < cost) {
       send_to_char("You don't have enough money to use the magic fountain.\n\r",ch);
       return(FALSE);
      } else {
        sprintf(buf, "You drop %d coins in the fountain\n\r", cost);
        send_to_char(buf, ch);
        GET_GOLD(ch) -= cost;
        }
     
    only_argument(arg,buf);

    if (str_cmp(buf, "fountain") && str_cmp(buf, "water")) {
      return(FALSE);
    }

    send_to_char("You drink from the fountain\n\r", ch);
    act("$n drinks from the fountain", FALSE, ch, 0, 0, TO_ROOM);


    switch (number(0, 45)) {

    /* Lets try and make 1-10 Good, 11-26 Bad, 27-40 Nothing */
   case 1:
          cast_refresh(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
   case 2:
     cast_stone_skin(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
        case 3:
     cast_cure_serious(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
   case 4:
     cast_cure_light(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
        case 5:
     cast_armor(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
        case 6:
     cast_bless(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
        case 7:
     cast_invisibility(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
        case 8:
     cast_strength(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
   case 9:
     cast_remove_poison(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
   case 10:
          cast_true_seeing(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;

/* Time for the nasty Spells */

   case 11:
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'All your magic will now cease.'",
               FALSE, mob, 0, ch, TO_ROOM);
     cast_dispel_magic(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears.....\n\r", ch);
   break;
   case 12:
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room );
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'Get out of my face!'",
              FALSE, mob, 0, ch, TO_ROOM); 
     cast_teleport(GetMaxLevel(mob), ch, "", SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
   break;
   case 13:
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'Let's get sticky!'",
               FALSE, mob, 0, ch, TO_ROOM);
     cast_web(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
   break;
   case 14:
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'Does this hurt?'",
               FALSE, mob, 0, ch, TO_ROOM);
     cast_curse(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
   break;
   case 15:
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'Ray Charles'",
             FALSE, mob, 0, ch, TO_ROOM);
     cast_blindness(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
   break;
        case 16:
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'Wimpy girly man'",
              FALSE, mob, 0, ch, TO_ROOM);
     cast_weakness(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);  
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
   break;
   case 17:
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'You look sickly'",
              FALSE, mob, 0, ch, TO_ROOM);
     cast_poison(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
   break;
        case 18:
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'This will only hurt a little.'",
            FALSE, mob, 0, ch, TO_ROOM);
     cast_cause_light(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
   break;
   case 19:
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'This will only hurt a good bit.'",
            FALSE, mob, 0, ch, TO_ROOM);
     cast_cause_critic(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
   break;
   case 20:
   case 21:
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'Fourth of July!'",
             FALSE, mob, 0, ch, TO_ROOM);
     cast_magic_missile(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
   break;
        case 22:
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'Ah, easier to hit!'",
            FALSE, mob, 0, ch, TO_ROOM);
     cast_faerie_fire(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
   break;
   case 23:
     cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
        case 24:
     cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
        case 25:
     cast_acid_blast(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
   case 26:
     cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
   break;
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40: {
          send_to_char("The fluid tastes like dry sand in your mouth.\n\r", ch);
   break;
       }
        case 41: {
          cast_heal(40, ch, "", SPELL_TYPE_SPELL,ch,0);
          break;
          }
        case 42: {
          cast_full_heal(40, ch, "", SPELL_TYPE_SPELL, ch, 0);
          break;
          }
        case 43: {
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'This is gonna hurt.'",
              FALSE, mob, 0, ch, TO_ROOM);
          cast_meteor_swarm(GetMaxLevel(mob), mob, "" , SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
          break;
          }
        case 44: {
          send_to_room("A large demon appears from the fountain.\n\r", ch->in_room);
          mob = read_mobile(DEMON, VIRTUAL);
          char_to_room(mob, ch->in_room);
          act("$n utters the words, 'BZZZZZZZZZZZZZZZZT'",
              FALSE, mob, 0, ch, TO_ROOM);
          cast_lightning_bolt(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
          extract_char(mob);
          send_to_char("And just as soon disappears....\n\r", ch);
          break;
          }
           default : {
              send_to_char("The water feels cool and refreshing..\n\r", ch);
              break;
              }
         }
  return(TRUE);
  }

  /* All commands except fill and drink */
  return(FALSE);
}


typedef struct T1000_data {
   int state;
   struct char_data *vict;
} T1000_data; 

#if 0
#define T1000_SEARCHING   0
#define T1000_HUNTING     1

int T1000( struct char_data *ch, char *line, int cmd)
{
   int count;
   struct descriptor_data *i;

   extern struct descriptor_data *descriptor_list;

   if (!ch->act_ptr)
      ch->act_ptr = (int *) calloc(1, sizeof(int));
   if (ch->specials.hunting == 0)
     (*((int *) ch->act_ptr)) = T1000_SEARCHING;

    switch(*((int *) ch->act_ptr)) {
       case T1000_SEARCHING: {
     count = number(0,200);
     for (i = descriptor_list; count>0; i= i->next) {
        if (!i) {
          i = descriptor_list;
        }
     }
     if (i) {
        ch->specials.hunting = i->character;
        (*((int *) ch->act_ptr)) = T1000_HUNTING;
     }
       }
       case T1000_HUNTING: {
     if (ch->in_room == ch->specials.hunting->in_room) {
     } else {
     }
       }
    }
}
#endif


void invert(char *arg1, char *arg2)
{
 register int i = 0;
 register int len = strlen(arg1) - 1;
 
 while(i <= len) {
    *(arg2 + i) = *(arg1 + (len - i));
    i++;
 }
 *(arg2 + i) = '\0';
}
 
 
int jive_box(Mob *ch, int cmd, char *arg, Obj *me) {
   char buf[255], buf2[255], buf3[255], tmp[255];

   switch(cmd) {
       case 17:
       case 169: invert(arg, buf);
                 do_say(ch, buf, cmd);
                 return TRUE;
                 break;
       case 19:  half_chop(arg, tmp, buf);
                 invert(buf, buf2);
                 sprintf(buf3, "%s %s", tmp, buf);
                 do_tell(ch, buf3, cmd);
                 return TRUE;
                 break;
       case 18:  invert(arg, buf);
                 do_shout(ch, buf, cmd);
                 return TRUE;
                 break;
       default:  return FALSE;
   }
}


int new_ninja_master(struct char_data *ch, int cmd, char *arg)
{
  char buf[256];
  static char *n_skills[] = {
    "disarm", /* No. 245 */
    "doorbash", /* No. 259 */
    "spy",
    "retreat",
    "\n",
  };
  int percent=0, number=0;
  int charge, sk_num, mult;

  if (!AWAKE(ch))
    return(FALSE);  

  if (!cmd) {
    if (ch->specials.fighting) {
      return(fighter(ch, cmd, arg));
    }
    return(FALSE);
  }

  if (!ch->skills) return(FALSE);

  if (check_soundproof(ch)) return(FALSE);
  
  for(; *arg==' '; arg++); /* ditch spaces */
  
  if ((cmd==164)||(cmd==170)) {
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf," disarm  :  %s\n\r",how_good(ch->skills[SKILL_DISARM].learned));
      send_to_char(buf,ch);
      sprintf(buf," doorbash:  %s\n\r",how_good(ch->skills[SKILL_DOORBASH].learned));
      send_to_char(buf,ch);
      sprintf(buf," spy:       %s\n\r",how_good(ch->skills[SKILL_SPY].learned));
      send_to_char(buf,ch);
      sprintf(buf," retreat:   %s\n\r",how_good(ch->skills[SKILL_RETREAT].learned));
      send_to_char(buf,ch);
      return(TRUE);
    } else {
      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("The ninja master says ",ch);
      if (number == -1) {
   send_to_char("'I do not know of this skill.'\n\r", ch);
   return(TRUE);
      }
      charge = GetMaxLevel(ch) * 1000;
      switch(number) {
      case 0:
      case 1:
   sk_num = SKILL_DISARM;
   if (!HasClass(ch, CLASS_WARRIOR)) {
     send_to_char
       ("'You do not possess the necessary fighting skills'\n\r",ch);
     return(TRUE);
   }
   break;
      case 2:
   sk_num = SKILL_DOORBASH;
   if (!HasClass(ch, CLASS_WARRIOR) &&
       !HasClass(ch, CLASS_PALADIN)) {
     send_to_char
       ("'You do not possess the necessary fighting skills'\n\r",ch);
     return(TRUE);
   }
   break;
      case 3:
   sk_num = SKILL_SPY;
   if (!HasClass(ch, CLASS_THIEF)) {
     send_to_char
       ("'You do not possess the necessary thieving skills'\n\r",ch);
     return(TRUE);
   }
   break;
      case 4:
   sk_num = SKILL_RETREAT;
   if (!HasClass(ch, CLASS_THIEF)) {
     send_to_char
       ("'I only teach this skill to thieves'\n\r",ch);
     return(TRUE);
   }
   break;
      default:
   sprintf(buf, "Strangeness in ninjamaster (%d)", number);
   log(buf);
   send_to_char("'Ack!  I feel faint!'\n\r", ch);
   return(TRUE);
      }
    } 
    
   if (sk_num == SKILL_HUNT) {
      if (ch->skills[sk_num].learned >= 90) {
   send_to_char
     ("'You are a master of this art, I can teach you no more.'\n\r",ch);
   return(TRUE);
      }
    } else {
     if (ch->skills[sk_num].learned >= 45) {
      send_to_char("You must learn from practice and experience now.\n\r", ch);
      return(TRUE);
      }          
    }

    if (GET_GOLD(ch) < charge){
      send_to_char
   ("'Ah, but you do not have enough money to pay.'\n\r",ch);
      return(TRUE);
    }
    
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char 
   ("'You must first use the knowledge you already have.'\n\r",ch);
      return(FALSE);
    }
    
    GET_GOLD(ch) -= charge;   
    send_to_char("'We will now begin.'\n\r",ch);
    ch->specials.spells_to_learn--;
    
    percent = ch->skills[sk_num].learned +
      int_app[GET_INT(ch)].learn;
    ch->skills[sk_num].learned = MIN(90, percent);
    
    if (ch->skills[sk_num].learned >= 90) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
      return(TRUE);        
    } 
  } else {
    return(FALSE);
  }
}


int RepairGuy( struct char_data *ch, int cmd, char *arg)
{
  char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
  int cost, ave;
  struct char_data *vict;
  struct obj_data *obj;
  int (*rep_guy)();  /* special procedure for this mob/obj       */
  
  
  if (!AWAKE(ch))
    return(FALSE);

  rep_guy = RepairGuy;
  
  if (IS_NPC(ch)) {
    send_to_char("I don't deal with monsters.\n\r", ch);
    return(FALSE);
  }
  
  if (cmd == 72) { /* give */
    /* determine the correct obj */
    arg=one_argument(arg,obj_name);
    if (!*obj_name) {
      send_to_char("Give what?\n\r",ch);
      return(FALSE);
    }
    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
      send_to_char("Give what?\n\r",ch);
      return(TRUE);
    }
    arg=one_argument(arg, vict_name);
    if(!*vict_name)  {
      send_to_char("To who?\n\r",ch);
      return(FALSE);
    }
    if (!(vict = get_char_room_vis(ch, vict_name)))   {
      send_to_char("To who?\n\r",ch);
      return(FALSE);
    }
    /* the target is the repairman, or an NPC */
    if (!IS_NPC(vict))
      return(FALSE);

    if (mob_index[vict->nr].func == rep_guy) {  
      /* we have the repair guy, and we can give him the stuff */
      act("You give $p to $N.",TRUE,ch,obj,vict,TO_CHAR);
      act("$n gives $p to $N.",TRUE,ch,obj,vict,TO_ROOM);   
    } else {
      return(FALSE);
    }
    
    act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
    act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);
    
    /* make all the correct tests to make sure that everything is kosher */
    
    if (ITEM_TYPE(obj) == ITEM_ARMOR && obj->obj_flags.value[1] > 0) {
      if (obj->obj_flags.value[1] > obj->obj_flags.value[0]) {
   /* get the value of the object */
   cost = obj->obj_flags.cost;
   /* divide by value[1]   */
   cost /= obj->obj_flags.value[1];  
   /* then cost = difference between value[0] and [1] */
   cost *= (obj->obj_flags.value[1] - obj->obj_flags.value[0]); 
   if (GetMaxLevel(vict) > 25) /* super repair guy */
     cost *= 2;
   if (cost > GET_GOLD(ch)) {
          if (check_soundproof(ch)) {
       act("$N shakes $S head.\n\r", 
         TRUE, ch, 0, vict, TO_ROOM);
       act("$N shakes $S head.\n\r", 
         TRUE, ch, 0, vict, TO_CHAR);
          } else {
           act("$N says 'I'm sorry, you don't have enough money.'", 
            TRUE, ch, 0, vict, TO_ROOM);
        act("$N says 'I'm sorry, you don't have enough money.'", 
            TRUE, ch, 0, vict, TO_CHAR);
      }
   } else {
     GET_GOLD(ch) -= cost;
     
     sprintf(buf, "You give $N %d coins.",cost);
     act(buf,TRUE,ch,0,vict,TO_CHAR);
     act("$n gives some money to $N.",TRUE,ch,obj,vict,TO_ROOM);
     
     /* fix the armor */
     act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_ROOM);
     act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_CHAR);
     if (GetMaxLevel(vict) > 25) {
       obj->obj_flags.value[0] = obj->obj_flags.value[1];
     } else {
       ave = MAX(obj->obj_flags.value[0], 
            (obj->obj_flags.value[0] + 
             obj->obj_flags.value[1] ) /2);
       obj->obj_flags.value[0] = ave;
       obj->obj_flags.value[1] = ave;
     }
          if (check_soundproof(ch)) {
       act("$N smiles broadly.",TRUE,ch,0,vict,TO_ROOM);
       act("$N smiles broadly.",TRUE,ch,0,vict,TO_CHAR);
     } else {
          act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_ROOM);
       act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_CHAR);
     }
   }      
      } else {
   if (check_soundproof(ch)) {
     act("$N shrugs.",
          TRUE,ch,0,vict,TO_ROOM);
     act("$N shrugs.",
          TRUE,ch,0,vict,TO_CHAR);
   }else{
      act("$N says 'Your armor looks fine to me.'",
          TRUE,ch,0,vict,TO_ROOM);
      act("$N says 'Your armor looks fine to me.'",
          TRUE,ch,0,vict,TO_CHAR);
        }
      }
    } else {
      if (GetMaxLevel(vict) < 25 || (ITEM_TYPE(obj)!=ITEM_WEAPON)) {
   if (check_soundproof(ch)) {
     act("$N shakes $S head.\n\r", 
         TRUE, ch, 0, vict, TO_ROOM);
     act("$N shakes $S head.\n\r", 
         TRUE, ch, 0, vict, TO_CHAR);
   } else {
     if (ITEM_TYPE(obj) != ITEM_ARMOR) {
       act("$N says 'That isn't armor.'",TRUE,ch,0,vict,TO_ROOM);
       act("$N says 'That isn't armor.'",TRUE,ch,0,vict,TO_CHAR);
     } else {
       act("$N says 'I can't fix that...'", TRUE, ch, 0, vict, TO_CHAR);
       act("$N says 'I can't fix that...'", TRUE, ch, 0, vict, TO_ROOM);
     }
   }
      } else {

   struct obj_data *new;

/* weapon repair.  expensive!   */
   cost = obj->obj_flags.cost;
   new = read_object(obj->item_number, REAL);
   if (obj->obj_flags.value[2])
     cost /= obj->obj_flags.value[2];

   cost *= (new->obj_flags.value[2] - obj->obj_flags.value[2]); 

   if (cost > GET_GOLD(ch)) {
          if (check_soundproof(ch)) {
       act("$N shakes $S head.\n\r", 
         TRUE, ch, 0, vict, TO_ROOM);
       act("$N shakes $S head.\n\r", 
         TRUE, ch, 0, vict, TO_CHAR);
          } else {
           act("$N says 'I'm sorry, you don't have enough money.'", 
            TRUE, ch, 0, vict, TO_ROOM);
        act("$N says 'I'm sorry, you don't have enough money.'", 
            TRUE, ch, 0, vict, TO_CHAR);
      }
   } else {
     GET_GOLD(ch) -= cost;
     
     sprintf(buf, "You give $N %d coins.",cost);
     act(buf,TRUE,ch,0,vict,TO_CHAR);
     act("$n gives some money to $N.",TRUE,ch,obj,vict,TO_ROOM);
     
     /* fix the weapon */
     act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_ROOM);
     act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_CHAR);

     obj->obj_flags.value[2] = new->obj_flags.value[2];
     extract_obj(new);

          if (check_soundproof(ch)) {
       act("$N smiles broadly.",TRUE,ch,0,vict,TO_ROOM);
       act("$N smiles broadly.",TRUE,ch,0,vict,TO_CHAR);
     } else {
          act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_ROOM);
       act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_CHAR);
     }
   }      



      }
    }
    
    act("$N gives you $p.",TRUE,ch,obj,vict,TO_CHAR);
    act("$N gives $p to $n.",TRUE,ch,obj,vict,TO_ROOM);
    return(TRUE);
  } else {
    if (cmd) return FALSE;
    return(fighter(ch, cmd, arg));
  }
}


int Samah( struct char_data *ch, int cmd, char *arg)
{
  char *p, buf[256];
  struct char_data *Sammy;  /* Samah's own referent pointer */
  struct char_data *t, *t2, *t3;
  int purge_nr;
  struct room_data *rp;

  rp = real_roomp(ch->in_room);
  if (!rp) return(FALSE);

  if (cmd) {

    if (GET_RACE(ch) == RACE_SARTAN || GET_RACE(ch) == RACE_PATRYN ||
   GetMaxLevel(ch) == BRUTIUS)
      return(FALSE);

    Sammy = (struct char_data *)FindMobInRoomWithFunction(ch->in_room, Samah);

    for (;*arg==' ';arg++); /* skip whitespace */
    strcpy(buf, arg);

    if (cmd == 207) { /* recite */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      p = (char *)strtok(buf, " ");
      if (strncmp("recall", p, strlen(p))==0) {
   act("$n says 'And just where do you think you're going, Mensch?", FALSE, Sammy, 0, 0, TO_ROOM);
   return(TRUE);
      }
    } else if (cmd == 84) { /* cast */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      /* we use strlen(p)-1 because if we use the full length, there is
    the obligatory ' at the end.  We must ignore this ', and get 
    on with our lives */
      p = (char *)strtok(buf, " ");
      if (strncmp("'word of recall'", p, strlen(p)-1)==0) {
   act("$n says 'And just where do you think you're going, Mensch?", FALSE, Sammy, 0, 0, TO_ROOM);
   return(TRUE);
      } else if (strncmp("'astral walk'", p, strlen(p)-1)==0) {
   act("$n says 'Do you think you can astral walk in and out of here like the ", FALSE, Sammy, 0, 0, TO_ROOM);
   act("wind,...Mensch?'", FALSE, Sammy, 0, 0, TO_ROOM);
   return(TRUE);
      } else if (strncmp("'teleport'", p, strlen(p)-1)==0) {
   act("$n says 'And just where do you think you're going, Mensch?", FALSE, Sammy, 0, 0, TO_ROOM);
   return(TRUE);
      } else if (strncmp("'polymorph'", p, strlen(p)-1)==0) {
   act("$n says 'I like you the way you are...Mensch.", FALSE, Sammy, 0, 0, TO_ROOM);
   return(TRUE);
      }
    } else if (cmd == 17 || cmd == 169) { /* say */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Mensch should be seen, and not heard'", FALSE, Sammy, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 40 || cmd == 214 || cmd == 177) { /* emote */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Cease your childish pantonimes, Mensch.'", FALSE, Sammy, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 19 || cmd == 18 || cmd == 83) { /* say, shout whisp */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Speak only when spoken to, Mensch.'", FALSE, Sammy, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 86) {  /* ask */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Your ignorance is too immense to be rectified at this time. Mensch.'", FALSE, Sammy, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 87 || cmd == 46) {  /* order, force */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'I'll be the only one giving orders here, Mensch'", FALSE, Sammy, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 151) {
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Cease this cowardly behavior, Mensch'", FALSE, Sammy, 0, ch, TO_ROOM);
      return(TRUE);
    } else if (cmd == 63) {
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Pay attention when I am speaking, Mensch'", FALSE, Sammy, 0, ch, TO_ROOM);
      return(TRUE);
    }
  } else {
    if (ch->specials.fighting) {
    } else {      
      /*
   check for followers in the room
   */
      for (t = rp->people; t; t= t->next_in_room) {
   if (IS_NPC(t) && !IS_PC(t) && t->master && t != ch && 
       t->master != ch) {
     break;
   }
      }
      if (t) {
   act("$n says 'What is $N doing here?'", FALSE, ch, 0, t, TO_ROOM);
   act("$n makes a magical gesture", FALSE, ch, 0, 0, TO_ROOM);
   purge_nr = t->nr;
   for (t2 = rp->people; t2; t2 = t3) {
     t3 = t2->next_in_room;
     if (t2->nr == purge_nr && !IS_PC(t2)) {
       act("$N, looking very surprised, quickly fades out of existence.", FALSE, ch, 0, t2, TO_ROOM);
       extract_char(t2);
     }
   }
      } else {
   /*
     check for polymorphs in the room
     */
   for (t = rp->people; t; t= t->next_in_room) {
     if (IS_NPC(t) && IS_PC(t)) {  /* ah.. polymorphed :-) */
       /*
         I would like to digress at this point, and state that
         I feel that George Bush is an incompetent fool.
         Thank you.
         */
       act("$n glares at $N", FALSE, ch, 0, t, TO_NOTVICT);
       act("$n glares at you", FALSE, ch, 0, t, TO_VICT);
       act("$n says 'Seek not to disguise your true form from me...Mensch.", FALSE, ch, 0, t, TO_ROOM);
       act("$n traces a small rune in the air", FALSE, ch, 0, 0, TO_ROOM);
       act("$n has forced you to return to your original form!", FALSE, ch, 0, t, TO_VICT);
       do_return(t, "", 1);
       return(TRUE);
     }
   }
      }
    }
  }
  return(FALSE);

}


#if EGO
int board(struct char_data *ch, int cmd, char *arg, struct obj_data *obj);
int BitterBlade(struct char_data *ch, int cmd, char *arg,struct obj_data *tobj, int type)
{
   extern struct str_app_type str_app[];
   struct obj_data *obj, *blade;
   struct char_data *joe, *holder;
   struct char_data *lowjoe = 0;
   char arg1[128];

   if (type != PULSE_COMMAND)
     return(FALSE);
   
   if (IS_IMMORTAL(ch)) return(FALSE);
   if (!real_roomp(ch->in_room)) return(FALSE);
   
   for (obj = real_roomp(ch->in_room)->contents; 
   obj ; obj = obj->next_content) {
     if (obj_index[obj->item_number].func == BitterBlade) {
       /* I am on the floor */
       for (joe = real_roomp(ch->in_room)->people; joe ; 
       joe = joe->next_in_room) {
    if ((GET_ALIGNMENT(joe) <= -400) && (!IS_IMMORTAL(joe))) {
      if (lowjoe) {
        if (GET_ALIGNMENT(joe) < GET_ALIGNMENT(lowjoe)){
          lowjoe = joe;
        } 
      } else lowjoe = joe; 
    }
       }
       if (lowjoe) {     
    if (CAN_GET_OBJ(lowjoe, obj)) {
      obj_from_room(obj);
      obj_to_char(obj,lowjoe);
      send_to_char("A blade leaps in to your hands!\n\r", lowjoe);
      act("A blade jumps from the floor and leaps in to $n's hands!",FALSE, lowjoe, 0, 0, TO_ROOM);
      if (!EgoBladeSave(lowjoe)) {
        if (!lowjoe->equipment[WIELD]) {
          send_to_char("The blade forces you to wield it!\n\r", lowjoe);
          wear(lowjoe, obj, 12);
          return(FALSE);
        } else {
          send_to_char("You can feel the blade attempt to make you wield it!\n\r", lowjoe);
          return(FALSE);
        }
      }
    }
       }
     }
   }
   for (holder = real_roomp(ch->in_room)->people; holder ; 
   holder = holder->next_in_room) {
     for (obj = holder->carrying; obj ; obj = obj->next_content) {
       if ((obj_index[obj->item_number].func) && 
      (obj_index[obj->item_number].func != board)){
    /*held*/
    if (holder->equipment[WIELD]) {
      if ((!EgoBladeSave(holder)) && (!EgoBladeSave(holder))) {
        send_to_char("The blade gets pissed off that you are wielding another weapon!!\n\r", holder);
        act("The blade knocks $p out of your hands!!",FALSE, holder, holder->equipment[WIELD], 0, TO_CHAR);
        blade = unequip_char(holder,WIELD);
        if (blade) obj_to_room(blade,holder->in_room);
        if (!holder->equipment[WIELD]) {
          send_to_char("The blade forces you to wield it!\n\r", holder);
          wear(holder, obj, 12);
          return(FALSE);
        }
      }     
    }
    if (!EgoBladeSave(holder)) {
      if (!EgoBladeSave(holder)) {
        if (!holder->equipment[WIELD]) {
          send_to_char("The blade forces you to wield it!\n\r", holder);
          wear(holder, obj, 12);
          return(FALSE);
        }
      }
    }
    if (affected_by_spell(holder,SPELL_CHARM_PERSON)) {
      affect_from_char(holder,SPELL_CHARM_PERSON);
      send_to_char("Due to the blade, you feel less enthused about your master.\n\r",holder);
    }
       } 
     }
     if (holder->equipment[WIELD]) {
       if ((obj_index[holder->equipment[WIELD]->item_number].func) 
      && (obj_index[holder->equipment[WIELD]->item_number].func != board)){
    /*YES! I am being held!*/
    obj = holder->equipment[WIELD];
    if (affected_by_spell(holder,SPELL_CHARM_PERSON)) {
      affect_from_char(holder,SPELL_CHARM_PERSON);
      send_to_char("Due to the blade, you feel less enthused about your master.\n\r",holder);
    }
    if (holder->specials.fighting) {
      send_to_char("The blade almost sings in your hand!!\n\r", holder);
      act("You can hear $n's blade almost sing with joy!",FALSE, holder, 0, 0, TO_ROOM);
      if ((holder == ch) && (cmd == 151)) {
        if (EgoBladeSave(ch) && EgoBladeSave(ch)) {
          send_to_char("You can feel the blade attempt to stay in the fight!\n\r", ch);
          return(FALSE);  
        } else {
          send_to_char("The blade laughs at your attempt to flee from a fight!!\n\r", ch);
          send_to_char("The blade gives you a little warning...\n\r", ch);
          send_to_char("The blade twists around and smacks you!\n\r", ch);
          act("Wow! $n's blade just whipped around and smacked $m one!",FALSE, ch, 0, 0, TO_ROOM);
          GET_HIT(ch) -= 25;
          if (GET_HIT(ch) < 0) {
       GET_HIT(ch) = 0;
       GET_POS(ch) = POSITION_STUNNED;
          }
          return(TRUE);
        }        
      }
    }
    if ((cmd == 66) && (holder == ch)) {
      one_argument(arg, arg1);
      if (strcmp(arg1,"all") == 0) {
        if (!EgoBladeSave(ch)) {
          send_to_char("The blade laughs at your attempt to remove it!\n\r", ch);
          send_to_char("The blade gives you a little warning...\n\r", ch);
          send_to_char("The blade twists around and smacks you hard!\n\r", ch);
          act("Wow! $n's blade just whipped around and smacked $m one!",FALSE, ch, 0, 0, TO_ROOM);
          GET_HIT(ch) -= 25;
          if (GET_HIT(ch) < 0) {
       GET_HIT(ch) = 0;
       GET_POS(ch) = POSITION_STUNNED;
          }
          return(TRUE);
        } else {
          send_to_char("You can feel the blade attempt to stay wielded!\n\r", ch);
          return(FALSE);  
        }
      } else { 
        if (isname(arg1,obj->name)) {
          if (!EgoBladeSave(ch)) {
       send_to_char("The blade laughs at your attempt to remove it!\n\r", ch);
       send_to_char("The blade gives you a little warning...\n\r", ch);
       send_to_char("The blade twists around and smacks you hard!\n\r", ch);
       act("Wow! $n's blade just whipped around and smacked $m one!",FALSE, ch, 0, 0, TO_ROOM);
       GET_HIT(ch) -= 25;
       if (GET_HIT(ch) < 0) {
         GET_HIT(ch) = 0;
         GET_POS(ch) = POSITION_STUNNED;
       }
       return(TRUE);
          } else {
       send_to_char("You can feel the blade attempt to stay wielded!\n\r", ch);
       return(FALSE);
          }
        }
      }
    }
    for (joe = real_roomp(holder->in_room)->people; joe ;
         joe = joe->next_in_room) {
      if ((GET_ALIGNMENT(joe) >= 500) && 
          (IS_MOB(joe)) && (CAN_SEE(holder,joe)) && (holder != joe)) {
        if (lowjoe) {
          if (GET_ALIGNMENT(joe) > GET_ALIGNMENT(lowjoe)){
       lowjoe = joe;
          } 
        } else lowjoe = joe; 
      }
    }
    if (lowjoe) {     
      if (!EgoBladeSave(holder)) {
        if (GET_POS(holder) != POSITION_STANDING) {
          send_to_char("The blade yanks you to your feet!\n\r", ch);
          GET_POS(holder) = POSITION_STANDING;
        }
        send_to_char("The blade leaps out of control!!\n\r", holder);
        act("A blade jumps for $n's neck!",FALSE, lowjoe, 0, 0, TO_ROOM);
        do_hit(holder,lowjoe->player.name, 0);
        return(TRUE);
      } else {
        return(FALSE);
      }
    }
    if ((cmd == 70) && (holder == ch)) {
      send_to_char("The blade almost sings in your hand!!\n\r", ch);
      act("You can hear $n's blade almost sing with joy!",FALSE, ch, 0, 0, TO_ROOM);
      return(FALSE);
    }      
       }
     }
   }
   return(FALSE);
 }
#endif

#define GIVE 72
#define GAIN 243

int MakeQuest(struct char_data *ch, struct char_data *gm, int Class, char *arg, int cmd) 
{
  char obj_name[50], vict_name[50];
  struct char_data *vict;
  struct obj_data *obj;

  int i;

  extern struct QuestItem QuestList[4][IMMORTAL];

#if EASY_LEVELS
  if (GET_LEVEL(ch, Class) > 0) {  /* for now.. so as not to give it away */
    if (cmd == GAIN) {
      GainLevel(ch, Class);
      return(TRUE);
    }
    return(FALSE);
  }
#endif

   if (cmd == GIVE) {
     arg=one_argument(arg,obj_name);
     arg=one_argument(arg, vict_name);
     if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))   {
       send_to_char("You do not seem to have anything like that.\n\r", ch);
       return(FALSE);
     }
     if (!(vict = get_char_room_vis(ch, vict_name)))  {
       send_to_char("No one by that name around here.\n\r", ch);
       return;
     }
     if (vict == gm) {
       if (obj_index[obj->item_number].virtual == QuestList[Class][GET_LEVEL(ch, Class)].item) {
    act("$n graciously takes your gift of $p", FALSE, gm, obj, ch, TO_VICT);
    obj_from_char(obj);
    extract_obj(obj);
    GainLevel(ch, Class);
    return(TRUE);
       } else {
    act("$n shakes $s head", FALSE, gm, 0, 0, TO_ROOM);
    act("$n says 'That is not the item which i desire'", FALSE, gm, 0, 0, TO_ROOM);
    return(FALSE);
       }
     } else {
       return(FALSE);
     }

   } else if (cmd == GAIN) {
     if (GET_EXP(ch)<
    titles[Class][GET_LEVEL(ch, Class)+1].exp) {
       send_to_char("You are not yet ready to gain\n\r", ch);
       return(FALSE);
     }

     if (QuestList[Class][GET_LEVEL(ch, Class)].item) {
       act("$n shakes $s head", FALSE, gm, 0, 0, TO_ROOM);
       act("$n tells you 'First you must prove your mastery of knowledge'", 
      FALSE, gm, 0, ch, TO_VICT);
       act("$n tells you 'Give to me the item that answers this riddle'", 
      FALSE, gm, 0, ch, TO_VICT);
       act("$n tells you 'And you shall have your level'\n\r", 
      FALSE, gm, 0, ch, TO_VICT);
       send_to_char(QuestList[Class][GET_LEVEL(ch, Class)].where, ch);
       send_to_char("\n\rGood luck", ch);
       return(FALSE);
     }
   } else {
     return(FALSE);
   }

}




int creeping_death( struct char_data *ch, int cmd, char *arg)
{
  struct char_data *t, *next;
  struct room_data *rp;
  struct obj_data *co, *o;

  
  if (cmd) return(FALSE);
  if (!ch->act_ptr)
     ch->act_ptr = (int *) calloc(1, sizeof(int));
  if (ch->specials.fighting) {  /* kill */

    t = ch->specials.fighting;
    if (t->in_room == ch->in_room) {
      act("$N is engulfed by $n!", FALSE, ch, 0, t, TO_NOTVICT);
      act("You are engulfed by $n, and are quickly disassembled", FALSE,
     ch, 0, t, TO_VICT);
      act("$N is quickly reduced to a bloody pile of bones by $n", FALSE,
     ch, 0, t, TO_NOTVICT);
      GET_HIT(ch) -= GET_HIT(t);
      die(t);
      /* find the corpse and destroy it */
      rp = real_roomp(ch->in_room);
      if (!rp) return(FALSE);
      for (co = rp->contents; co; co = co->next_content) {
   if (IS_CORPSE(co))  {  /* assume 1st corpse is victim's */
     while (co->contains) {
       o = co->contains;
       obj_from_obj(o);
       obj_to_room(o, ch->in_room);
     }
     extract_obj(co);  /* remove the corpse */
   }
      }
    }
    if (GET_HIT(ch) < 0) {
      act("$n dissapates, you breath a sigh of relief", FALSE, ch,
     0, 0, TO_ROOM);
      GET_HIT(ch) = 1;
      ch->points.max_hit = 10;
      REMOVE_BIT(ch->specials.act, ACT_SPEC);
      return(TRUE);
    }
    return(TRUE);
  }

  /* the act_ptr is the direction of travel */

  if (number(0,1)==0) {  /* move */
    if (!ValidMove(ch, (*((int *) ch->act_ptr)))) {
      act("$n dissapates, you breath a sigh of relief", FALSE, ch,
     0, 0, TO_ROOM);
      GET_HIT(ch) = 1;
      ch->points.max_hit = 10;
      REMOVE_BIT(ch->specials.act, ACT_SPEC);
      return(FALSE);
    } else {
      do_move(ch, "\0", (*((int *) ch->act_ptr)));
      return(FALSE);
    }
  } else {

    /* make everyone with any brains flee */
    for (t = real_roomp(ch->in_room)->people; t; t = next) {
      next = t->next_in_room;
      if (t != ch) {
   if (!saves_spell(t, SAVING_PETRI)) {
     do_flee(t, "\0", 0);
   }
      }
    }

    /* find someone in the room to flay */
    for (t = real_roomp(ch->in_room)->people; t; t = next) {
      next = t->next_in_room;
      if (!IS_IMMORTAL(t) && t != ch && number(0,2)==0) {

   act("$N is engulfed by $n!", FALSE, ch, 0, t, TO_NOTVICT);
   act("You are engulfed by $n, and are quickly disassembled", FALSE,
       ch, 0, t, TO_VICT);
   act("$N is quickly reduced to a bloody pile of bones by $n", FALSE,
       ch, 0, t, TO_NOTVICT);
   GET_HIT(ch) -= GET_HIT(t);
   die(t);
   /* find the corpse and destroy it */
   rp = real_roomp(ch->in_room);
   if (!rp) return(FALSE);
   for (co = rp->contents; co; co = co->next_content) {
     if (IS_CORPSE(co))  {  /* assume 1st corpse is victim's */
       while (co->contains) {
         o = co->contains;
         obj_from_obj(o);
         obj_to_room(o, ch->in_room);
       }
       extract_obj(co);  /* remove the corpse */
     }
   }

   if (GET_HIT(ch) < 0) {
     act("$n dissapates, you breath a sigh of relief", FALSE, ch,
         0, 0, TO_ROOM);
     GET_HIT(ch) = 1;
     ch->points.max_hit = 10;
     REMOVE_BIT(ch->specials.act, ACT_SPEC);
     return(TRUE);
   }

   break;  /* end the loop */

      }
    }    
  }
}

#if 0
/*
   shanty town kids
*/
int shanty_town_kids( struct char_data *ch, int cmd, char *arg)
{

  if (!AWAKE(ch)) return(FALSE);


  /*
    harrass low levellers.
  */
  if (cmd >= 1 && cmd <= 6) {
    if (GetMaxLevel(ch) <= 5 && number(0,2)==0) {
      act("A street kid sticks out a foot and trips you as you try to leave",
     FALSE, ch, 0, 0, TO_CHAR);
      act("A street kid sticks out a foot and trips $n",
     FALSE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      act("The street kid laughs at you", FALSE,ch, 0, 0, TO_CHAR);
      act("The street kid laughs at $n", FALSE,ch, 0, 0, TO_ROOM);
    }
  }

  /*
    steal from mid-levellers
  */
  

  /*
    backstab high levellers
    */


  if (cmd) {
    
    
  }

  if (ch->specials.fighting) {
    act("$N runs between $n's legs", FALSE, ch->specials.fighting, 0, ch, TO_ROOM);
    act("$N runs between your legs", FALSE, ch->specials.fighting, 0, ch, TO_CHAR);
    vict = ch->specials.fighting;
    stop_fighting(ch);
    stop_fighting(vict);
  }

}

#endif




int GenericCityguardHateUndead(struct char_data *ch, int cmd, char *arg, int type)
{
  struct char_data *tch, *evil, *i;
  int max_evil, lev;
  
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);

    if (!check_soundproof(ch)) {
    
       if (number(0,20) == 0) {
         do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
       } else {
         act("$n shouts 'To me, my fellows! I need thy aid!'", 
        TRUE, ch, 0, 0, TO_ROOM);
       }
    
       if (ch->specials.fighting)
         CallForGuard(ch, ch->specials.fighting, 3, type);
    
       return(TRUE);
     }
  }
  
  max_evil = 0;
  evil = 0;
  
  if (check_peaceful(ch, ""))
    return FALSE;
  
  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if ((IS_NPC(tch)) && (IsUndead(tch)) && CAN_SEE(ch, tch)) {
      max_evil = -1000;
      evil = tch;
      if (!check_soundproof(ch))
         act("$n screams 'EVIL!!!  BANZAI!  SPOOON!'", 
     FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, evil, TYPE_UNDEFINED);
      return(TRUE);      
    }
    if (!IS_PC(tch)) {
      if (tch->specials.fighting) {
   if ((GET_ALIGNMENT(tch) < max_evil) &&
       (!IS_PC(tch) || !IS_PC(tch->specials.fighting))) {
     max_evil = GET_ALIGNMENT(tch);
     evil = tch;
   }
      }
    }
  }
  
  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    if (GET_HIT(evil->specials.fighting) > GET_HIT(evil) ||
   (evil->specials.fighting->attackers > 3)) {
      if (!check_soundproof(ch))
   act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'", 
       FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, evil, TYPE_UNDEFINED);
      return(TRUE);
    } else {
      if (!check_soundproof(ch))
   act("$n yells 'There's no need to fear! $n is here!'", 
       FALSE, ch, 0, 0, TO_ROOM);

      if (!ch->skills)
   SpaceForSkills(ch);

      if (!ch->skills[SKILL_RESCUE].learned)
         ch->skills[SKILL_RESCUE].learned = GetMaxLevel(ch)*3+30;
      do_rescue(ch, GET_NAME(evil->specials.fighting), 0);
    }
  }
  
  return(FALSE);
}



int GenericCityguard(struct char_data *ch, int cmd, char *arg, int type)
{
  struct char_data *tch, *evil, *i;
  int max_evil, lev;
  
  if (cmd || !AWAKE(ch))
    return (FALSE);
  
  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);

    if (!check_soundproof(ch)) {
       if (number(0,20) == 0) {
         do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
       } else {
         act("$n shouts 'To me, my fellows! I need thy aid!'", 
        TRUE, ch, 0, 0, TO_ROOM);
       }
    
       if (ch->specials.fighting)
         CallForGuard(ch, ch->specials.fighting, 3, type);
    
       return(TRUE);
     }
  }
  
  max_evil = 1000;
  evil = 0;
  
  if (check_peaceful(ch, ""))
    return FALSE;
  
  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
     (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
   max_evil = GET_ALIGNMENT(tch);
   evil = tch;
      }
    }
  }
  
  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    if (!check_soundproof(ch)) {
       act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'", 
   FALSE, ch, 0, 0, TO_ROOM);
    }
    hit(ch, evil, TYPE_UNDEFINED);
    return(TRUE);
  }
  
  return(FALSE);
}



int loremaster(struct char_data *ch, int cmd, char *arg)
{
  char buf[256];
  static char *n_skills[] = {
    "necromancy",
    "vegetable lore",
    "animal lore",
    "reptile lore",
    "people lore",
    "giant lore",
    "other lore",
    "read magic",
    "demonology",
    "sign language",
    "\n",
  };
  int percent=0, number=0;
  int charge, sk_num, mult;
 
  if (!AWAKE(ch))
    return(FALSE);  
 
  if (!ch->skills) return(FALSE);
 
  if (!cmd) {
    if (ch->specials.fighting) {
      return(fighter(ch, cmd, arg));
    }
    return(FALSE);
  }
 
  if (check_soundproof(ch)) return(FALSE);
  
  for(; *arg==' '; arg++); /* ditch spaces */
  
  if ((cmd==164)||(cmd==170)) {
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf," necromancy      :  %s\n\r",how_good(ch->skills[SKILL_CONS_UNDEAD].learned));
      send_to_char(buf,ch);
      sprintf(buf," vegetable lore  :  %s\n\r",how_good(ch->skills[SKILL_CONS_VEGGIE].learned));
      send_to_char(buf,ch);
      sprintf(buf," demonology      :  %s\n\r",how_good(ch->skills[SKILL_CONS_DEMON].learned));
      send_to_char(buf,ch);
      sprintf(buf," animal lore     :  %s\n\r",how_good(ch->skills[SKILL_CONS_ANIMAL].learned));
      send_to_char(buf,ch);
      sprintf(buf," reptile lore  :  %s\n\r",how_good(ch->skills[SKILL_CONS_REPTILE].learned));
      send_to_char(buf,ch);
      sprintf(buf," people lore     :  %s\n\r",how_good(ch->skills[SKILL_CONS_PEOPLE].learned));
      send_to_char(buf,ch);
      sprintf(buf," giant lore      :  %s\n\r",how_good(ch->skills[SKILL_CONS_GIANT].learned));
      send_to_char(buf,ch);
      sprintf(buf," other lore      :  %s\n\r",how_good(ch->skills[SKILL_CONS_OTHER].learned));
      send_to_char(buf,ch);
      sprintf(buf," read magic      :  %s\n\r",how_good(ch->skills[SKILL_READ_MAGIC].learned));
      send_to_char(buf,ch);
      sprintf(buf," sign language   :  %s\n\r",how_good(ch->skills[SKILL_SIGN].learned));
      send_to_char(buf,ch);
      return(TRUE);
    } else {
      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("The loremaster says ",ch);
      if (number == -1) {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return(TRUE);
      }
      charge = GetMaxLevel(ch) * 100;
      switch(number) {
      case 0:
      case 1:
        sk_num = SKILL_CONS_UNDEAD;
        break;
      case 2:
        sk_num = SKILL_CONS_VEGGIE;
        break;
      case 3:
        sk_num = SKILL_CONS_ANIMAL;
        break;
      case 4:
        sk_num = SKILL_CONS_REPTILE;
        break;
      case 5:
        sk_num = SKILL_CONS_PEOPLE;
        break;
      case 6:
        sk_num = SKILL_CONS_GIANT;
        break;
      case 7:
        sk_num = SKILL_CONS_OTHER;
        break;
      case 8:
        if (!HasClass(ch, CLASS_CLERIC) && !HasClass(ch, CLASS_MAGIC_USER)) {
          sk_num = SKILL_READ_MAGIC;
        } else {
          send_to_char("'You already have this skill!'\n\r",ch);
          if (ch->skills) 
            if (!ch->skills[SKILL_READ_MAGIC].learned)
              ch->skills[SKILL_READ_MAGIC].learned = 95;
          return;
        }
        break;
 
      case 9:
        sk_num = SKILL_CONS_DEMON;
        break;
      case 10:
        sk_num = SKILL_SIGN;
        break;
 
      default:
        sprintf(buf, "Strangeness in loremaster (%d)", number);
        log(buf);
        send_to_char("'Ack!  I feel faint!'\n\r", ch);
        return(TRUE);
      }
    } 
    
    if (GET_GOLD(ch) < charge){
      send_to_char
        ("'Ah, but you do not have enough money to pay.'\n\r",ch);
      return(TRUE);
    } 
    
    
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char
        ("'You must first earn more practices you already have.\n\r",ch);
      return(TRUE);
    }
    
    GET_GOLD(ch) -= charge;   
    send_to_char("'We will now begin.'\n\r",ch);
    ch->specials.spells_to_learn--;
    
    percent = ch->skills[sk_num].learned +
      int_app[GET_INT(ch)].learn;
    ch->skills[sk_num].learned = MIN(95, percent);
    
    if (ch->skills[sk_num].learned >= 95) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
      return(TRUE);        
    }   
  } else {
    return(FALSE);
  }
}

void station()
{
int t;
int i;
struct obj_data *obj;
struct char_data *ch;
t=time_info.hours;
if ((t==6)||(t==10)||(t==14)||(t==18)||(t==22)) {
        send_to_room("The train to the casino has arrived, and will leave in one hour.\n\r",18999);
        obj=read_object(9001,VIRTUAL);
        obj_to_room(obj,18999);
}
if ((t==9)||(t==13)||(t==17)||(t==21)||(t==1)) {    
        send_to_room("The train back to the mainland has arrived, and will be heading back in one hour.\n\r",8600);
        obj=read_object(9001,VIRTUAL);                
        obj_to_room(obj,8600);       
}
 
if ((t==9)||(t==13)||(t==17)||(t==21)||(t==1)) {
        send_to_room("The train has arrived at its destination and you are pushed immediatly off.\n\r",8602);
        ch=real_roomp(8602)->people;
        while(ch){
                char_from_room(ch);
                char_to_room(ch,8600);
                ch=real_roomp(8602)->people;
                }
}
 
if ((t==12)||(t==16)||(t==20)||(t==0)||(t==4)) { 
        send_to_room("The train has arrived at its destination and you are pushed immediatly off.\n\r",8601);
        ch=real_roomp(8601)->people;
        while(ch){ 
                char_from_room(ch); 
                char_to_room(ch,18999);
                ch=real_roomp(8601)->people; 
                }
}
 
if ((t==7)||(t==11)||(t==15)||(t==19)||(t==23)) {
        send_to_room("The train has left for the casino!\n\r",18999);
        send_to_room("The train rumbles as you depart for the casino!\n\r",8602);
        obj=get_obj_in_list("train",real_roomp(18999)->contents);
        if(obj) extract_obj(obj);
        }
 
 
if ((t==10)||(t==14)||(t==18)||(t==22)||(t==2)) {   
        send_to_room("The train has departed for the city.\n\r",8600);  
        send_to_room("The train rumbles and shakes as it heads back to the city.\n\r",8601); 
        obj=get_obj_in_list("train",real_roomp(8600)->contents);     
        if(obj) extract_obj(obj);  
        }  
 
}   
 
int train_station(struct char_data *ch, int cmd, char *arg)
{
int t;
if(cmd!=312) return FALSE;
t=time_info.hours;
if ((ch->in_room==18999)&&((t==6)||(t==10)||(t==14)||(t==18)||(t==22))){
        act("$n boards the train to the casino!",FALSE,ch,0,0,TO_ROOM);
        send_to_char("You board the train to the casino.\n\r",ch);
        char_from_room(ch);
        char_to_room(ch,8602);
        act("$n joins the trip to the casino!",FALSE,ch,0,0,TO_ROOM);
        return TRUE;
        } else if (ch->in_room==18999) {
                send_to_char("The train isn't here!\n\r",ch);
                return TRUE;
                }
 
if ((ch->in_room==8600)&&((t==9)||(t==13)||(t==17)||(t==21)||(t==1))){
        act("$n boards the train to the city.",FALSE,ch,0,0,TO_ROOM); 
        send_to_char("You board the train to the city.\n\r",ch); 
        char_from_room(ch); 
        char_to_room(ch,8601);  
        act("$n has just joined you for the ride back to the city.",FALSE,ch,0,0,TO_ROOM);
        return TRUE;
        } else if (ch->in_room==8600) send_to_char("The train isnt here!\n\r",ch);
return TRUE;
}


int dragon(struct char_data *ch, int cmd, char *arg, struct char_data *ch2)
{
        struct char_data *t1,*t2;
        int damage;
        if(cmd) return FALSE;
        if(!ch->specials.fighting) return FALSE;
        if(number(1,10)<7) {
                act("$n breathes a cone of frost!",FALSE,ch,0,0,TO_ROOM);
                for(t1=real_roomp(ch->in_room)->people;t1;t1=t1->next_in_room) {
                        if(!IS_MOB(t1)) {
                                send_to_char("The cold is FREEZING you!\n\r",t1);
                                damage=MIN(number(10,100),GET_HIT(t1)+8);
                                if(damage>0) GET_HIT(t1)-=damage;
                                }
                        }
                } else {
                t2=0;
                for(t1=real_roomp(ch->in_room)->people;t1;t1=t1->next_in_room) {
                        if(!IS_MOB(t1)) {
                                if(!t2) t2=t1;
                                        else if((GET_HIT(t1)>-1)&&(GET_HIT(t1)<GET_HIT(t2))) t2=t1;
                                }
                        }
                if(t2) { 
                        act("The Dragon whips $n with his tail!",FALSE,t2,0,0,TO_ROOM);
                        send_to_char("The Dragon slashes you with his spiked tail! OUCH!\n\r",t2);
                        damage=MIN(number(8,100),GET_HIT(t2)+1);
                        if(damage>0) GET_HIT(t2)-=damage;
                        }
                }
}



int hunter(struct char_data *ch, int cmd, char *arg)
{
  char buf[256];
  static char *n_skills[] = {
    "track",
    "find traps",
    "disarm traps",
    "\n",
  };
  int percent=0, number=0;
  int charge, sk_num, mult;

  if (!AWAKE(ch))
    return(FALSE);  

  if (!cmd) {
    if (ch->specials.fighting) {
      return(fighter(ch, cmd, arg));
    }
    return(FALSE);
  }

  if (!ch->skills) return(FALSE);

  if (check_soundproof(ch)) return(FALSE);
  
  for(; *arg==' '; arg++); /* ditch spaces */
  
  if ((cmd==164)||(cmd==170)) {
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf," track          :  %s\n\r",how_good(ch->skills[SKILL_HUNT].learned));
      send_to_char(buf,ch);
      sprintf(buf," find traps     :  %s\n\r",how_good(ch->skills[SKILL_FIND_TRAP].learned));
      send_to_char(buf,ch);
      sprintf(buf," disarm traps   :  %s\n\r",how_good(ch->skills[SKILL_REMOVE_TRAP].learned));
      send_to_char(buf,ch);
      return(TRUE);
    } else {

      if (!HasClass(ch, CLASS_THIEF)) {
   send_to_char("'You're not the sort I'll teach to'\n\r", ch);
   return(TRUE);
      }

      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("The hunter says ",ch);
      if (number == -1) {
   send_to_char("'I do not know of this skill.'\n\r", ch);
   return(TRUE);
      }
      charge = GetMaxLevel(ch) * 1000;
      switch(number) {
      case 0:
      case 1:
   sk_num = SKILL_HUNT;
   break;
      case 2:
   sk_num = SKILL_FIND_TRAP;
   break;
      case 3:
   sk_num = SKILL_REMOVE_TRAP;
   break;
      default:
   sprintf(buf, "Strangeness in hunter (%d)", number);
   log(buf);
   send_to_char("'Ack!  I feel faint!'\n\r", ch);
   return(FALSE);
      }
    }
    
    if (GET_GOLD(ch) < charge){
      send_to_char
   ("'Ah, but you do not have enough money to pay.'\n\r",ch);
      return(TRUE);
    } 

    if (ch->skills[sk_num].learned >= 90) {
      send_to_char
   ("'You are a master of this art, I can teach you no more.'\n\r",ch);
      return(TRUE);
    }
    
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char
   ("'You must first earn more practices you already have.\n\r",ch);
      return(TRUE);
    }
    
    GET_GOLD(ch) -= charge;   
    send_to_char("'We will now begin.'\n\r",ch);
    ch->specials.spells_to_learn--;
    
    percent = ch->skills[sk_num].learned +
      int_app[GET_INT(ch)].learn;
    ch->skills[sk_num].learned = MIN(90, percent);
    
    if (ch->skills[sk_num].learned >= 90) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
      return(TRUE);        
    } 
  } else {
    return(FALSE);
  }
}


int scraps(struct char_data *ch, int cmd, char *arg, struct obj_data *obj, int type)
{

  if (type == PULSE_COMMAND) {
    return(FALSE);
  } else {
    if (obj->obj_flags.value[0])
      obj->obj_flags.value[0]--;

    if (obj->obj_flags.value[0] == 0 && obj->in_room) {
      if ((obj->in_room != NOWHERE) &&(real_roomp(obj->in_room)->people)) {
   act("$p disintegrates into atomic particles!", 
       FALSE, real_roomp(obj->in_room)->people, obj, 0, TO_ROOM);
   act("$p disintegrates into atomic particles!", 
       FALSE, real_roomp(obj->in_room)->people, obj, 0, TO_CHAR);
      }
      extract_obj(obj);
    }
  }
}


/* START:  Stargazer Work: */


struct char_data *char_with_name(char *name) {
  extern struct descriptor_data *descriptor_list;
  struct descriptor_data *d; 

   for (d=descriptor_list; d; d = d->next) {
      if ((d->connected == CON_PLYNG) && (d->character) &&
          (!strcasecmp(GET_NAME(d->character), name))) 
      return d->character;
   }

   return NULL;
}


 
int adjacent_room(int room) {
  struct room_data *rp;
  int dir;
 
  if (rp = real_roomp(room))
     for (dir = 0; dir < 6; dir++) 
        if (rp->dir_option[dir] && (rp->dir_option[dir]->to_room > 0))
           return rp->dir_option[dir]->to_room;
 
  return -1;
}




#define HUNTER_ID "Hunter,"
 
int bounty_hunter( struct char_data *ch, int cmd, char *arg) {
  extern char *spells[];
  static char buf[256];
  static char buf2[256];
  struct char_data *me;
  struct char_data *targ;
  struct obj_data *temp_obj;
  int room, dir, offset, spell;
  static struct {
    char hunted_item[80];
    char hunted_victim[80];
    int num_chances;
    int level_command;
    int num_retrieved;
  } *job;
 
  if (cmd) {
     if (!arg)
        return FALSE;   
     for (; *arg == ' '; arg++);
     if ((cmd == 92) || (cmd == 60) || (cmd == 72)) {       /* catch rent/drop/give */
        for (me = real_roomp(ch->in_room)->people; me; me = me->next_in_room)
           if (IS_MOB(me) && mob_index[me->nr].func == bounty_hunter)
              if (me->act_ptr) {
                 job = (void *) me->act_ptr;
                 if (*(job->hunted_item) != '\0') {
                     sprintf(buf, "1.%s", job->hunted_item);
                     if ((temp_obj = get_obj_vis_world(me, buf, NULL)) && (ch == char_holding(temp_obj))) {
                        if (cmd == 92) {
                           do_say(me, "You're not getting away that easy.", 17);
                           act("$n stands between you and the receptionist!", FALSE, me, 0, ch, TO_VICT);
                           act("$n blocks $N from renting!", FALSE, me, 0, ch, TO_NOTVICT);     
                           return TRUE;
                        } else if (cmd == 72) {
                           if ((sscanf(arg, " %s %s", buf, buf2) == 2) && (isname(buf, temp_obj->name))) {
                              if (isname(buf2, GET_NAME(me))) {
                                 do_give(ch, arg, 72);
                                 do_action(me, GET_NAME(ch), 23);
                                 do_say(me, "I'm glad you've come to your senses!  I would surely smite thee!", 17); 
                              }
                              else {
                                 do_say(me, "I'll take that!", 17);
                                 act("$n almost grabs it from your hands!  Whew!", FALSE, me, 0, ch, TO_VICT);
                                 act("$n almost grabs something from $N's hands!", FALSE, me, 0, ch, TO_NOTVICT);    
                                 return TRUE;
                              }
                           }
                        } else {    /* cmd == 60 */
                           sscanf(arg, " %s ", buf);
                           if (isname(buf, temp_obj->name)) {
                              do_drop(ch, arg, 60);
                              do_say(me, "Thanks, sport!", 17);
                              do_get(me, arg, 60);    
                              return TRUE;
                           }
                        }        
                     }
                 }
              }
        return FALSE;
     } else if (cmd == 202) {                         /* catch group */
        me = FindMobInRoomWithFunction(ch->in_room, bounty_hunter);
        do_action(me, ch->player.name, 190);                /* growl */
        do_say(me, "Noone is grouping here, scum.", 17);
        return TRUE;
     }
     else if ((cmd == 207) && ((strcasestr(arg, "recall")) || (strcasestr(arg, "scroll")))) {
        me = FindMobInRoomWithFunction(ch->in_room, bounty_hunter);
        do_say(me, "Noone is recalling until I finish my job.", 17);
        act("$n knocks the scroll from your hand!", FALSE, me, 0, ch, TO_VICT);
        act("$n knocks the scroll from $N's hand!", FALSE, me, 0, ch, TO_NOTVICT);
        return TRUE;
     } 
     else if (cmd == 84) {                                     /* cast */
        if (*arg != '\'')
           return FALSE; 
        for (offset = 1; *(arg + offset) && (*(arg + offset) != '\''); offset++)
           *(arg + offset) = LOWER(*(arg + offset));
        if (*(arg + offset) != '\'')
           return FALSE;
        spell = old_search_block(arg, 1, offset - 1, spells, 0);
        if ((spell == 101) || (spell == 2) || (spell == 111) || (spell == 42)) {
           me = FindMobInRoomWithFunction(ch->in_room, bounty_hunter);
           act("$n disrupts your spell!", FALSE, me, 0, ch, TO_VICT);
           act("$n disrupts $N's spell!", FALSE, me, 0, ch, TO_NOTVICT);
           do_say(me, "Noone is going anywhere until I finish my job!", 17);
           return TRUE;
        }
     }
     else if (((cmd == 17) || (cmd == 169)) && (IS_IMMORTAL(ch))) {
        if (strncasecmp(arg, HUNTER_ID, strlen(HUNTER_ID)))
           return FALSE;
        arg += strlen(HUNTER_ID);
        me = FindMobInRoomWithFunction(ch->in_room, bounty_hunter);
        if (!strncasecmp(arg, " version", 8)) {
           act("$n says 'I am version 1.11 of the Bounty Hunter spec_proc.'", FALSE, me, 0, 0, TO_ROOM);
           act("$n says 'I was conceived of and designed by Stargazer.'", FALSE, me, 0, 0, TO_ROOM);
           return TRUE;
        }
        job = (void *) me->act_ptr;
        act("$n whispers something to $N.", FALSE, me, 0, ch, TO_NOTVICT);
        if (!strncasecmp(arg, " status", 7)) {
           if (!job) {
              act("$n whispers 'Master $N, I am currently idle with no memory.'", FALSE, me, 0, ch, TO_VICT);
              return TRUE;  
           }
           if (*(job->hunted_item) != '\0') {
              if (*(job->hunted_victim) != '\0')
                 sprintf(buf2, "$n whispers 'Master $N, I am hunting the %s carried by %s.'",
                    job->hunted_item, job->hunted_victim);
              else
                 sprintf(buf2, "$n whispers 'Master $N, I am currently hunting %s.'", job->hunted_item);
              if (job->num_retrieved > 0) {
                 act(buf2, FALSE, me, 0, ch, TO_VICT);
                 sprintf(buf2, "$n whispers 'I have already retrieved and destroyed %d.'", job->num_retrieved);
              }
           }
           else if (*(job->hunted_victim) != '\0') {
              sprintf(buf2, "$n whispers 'Master $N, I am going to kill %s.'", job->hunted_victim);
              if (job->num_retrieved > 0) {
                 act(buf2, FALSE, me, 0, ch, TO_VICT);
                 sprintf(buf2, "$n whispers 'I have already killed him %d %s.'",job->num_retrieved,
                     (job->num_retrieved == 1) ? "time" : "times");
              }
           }
           else
              sprintf(buf2, "$n whispers 'Master $N, I am currently idle with memory allocated.'");
           act(buf2, FALSE, me, 0, ch, TO_VICT);
           if (job->num_chances == -99)
              act("$n whispers 'I am showing no mercy.'", FALSE, me, 0, ch, TO_VICT);
           else
              act("$n whispers 'I am showing mercy.  Do I *have* to?'", FALSE, me, 0, ch, TO_VICT);
           return TRUE;
        }
        if (!(me->act_ptr))  {
           (void *) me->act_ptr = (void *) calloc (1,  sizeof(*job));
           job = (void *) me->act_ptr;
           *(job->hunted_item) = '\0';
           *(job->hunted_victim) = '\0';
           act("$n whispers 'Master $N, I am preparing to remember your will.", FALSE, me, 0, ch, TO_VICT);
        }
        job = (void *) me->act_ptr;
        if (!job) {
           log("Unable to allocate memory for bounty hunter!  This is bad!");
           return TRUE;
        }
        if (job->level_command > GetMaxLevel(ch)) {
           act("$n whispers 'Sorry, $N.  I report to a higher authority.'", FALSE, me, 0, ch, TO_VICT);
           return TRUE;
        }
        if (sscanf(arg, " repo %s", buf) == 1) {
           if (strlen(buf) > 79)
              return TRUE;
           sprintf(buf2, "$n whispers 'Master $N, I will now reposess all %s.'", buf);
           act(buf2, FALSE, me, 0, ch, TO_VICT);
           logf("%s just ordered %s to repo %s", GET_NAME(ch), me->player.short_descr, buf);
           *(job->hunted_victim) = '\0';
           strcpy(job->hunted_item, buf);
           job->level_command = GetMaxLevel(ch);
           job->num_retrieved = 0;
           if (job->num_chances != -99)
              job->num_chances = 9;
        }
        else if (sscanf(arg, " kill %s", buf) == 1) {
           if (strlen(buf) > 79)
              return TRUE;
           sprintf(buf2, "$n whispers 'Master $N, I will now kill %s.'", buf);
           act(buf2, FALSE, me, 0, ch, TO_VICT);
           logf("%s just sent out %s to kill %s", GET_NAME(ch), me->player.short_descr, buf);
           *(job->hunted_item) = '\0';
           strcpy(job->hunted_victim, buf);
           job->level_command = GetMaxLevel(ch);
           job->num_retrieved = 0;
           if (job->num_chances != -99)
              job->num_chances = 9;
        }
        else if (!strncasecmp(arg, " forget", 7)) {
           act("$n whispers 'Master $N, I am clearing my memory.'", FALSE, me, 0, ch, TO_VICT);
           free(job);
           me->act_ptr = NULL;
        }
        else if (!strncasecmp(arg, " no mercy", 9)) {
           act("$n whispers 'Master $N, I will have no mercy.'", FALSE, me, 0, ch, TO_VICT);
           job->num_chances = -99;
           logf("%s just ordered %s to show no mercy! (and better have a damn good reason)", GET_NAME(ch), me->player.short_descr);
        }
        else if (!strncasecmp(arg, " show mercy", 11)) {
           act("$n whispers 'Master $N, I will now show mercy.'", FALSE, me, 0, ch, TO_VICT);
           job->num_chances = 9;
        }
        else {
           act("$n whispers 'Master $N, I don't understand your request.'", FALSE, me, 0, ch, TO_VICT);
           act("$n whispers 'Please forgive me!'", FALSE, me, 0, ch, TO_VICT);
        }
        return TRUE;
     }
     return FALSE;
  }
 
  /* Bounty Hunter executes his job */
  if (!(ch->act_ptr) || !AWAKE(ch) || !IS_NPC(ch) || (ch->specials.fighting))
     return FALSE;
 
  job = (void *) ch->act_ptr;
  if (*(job->hunted_item) != '\0') {
     sprintf(buf, "1.%s", job->hunted_item);
     if (!(temp_obj = get_obj_vis_world(ch, buf, NULL))) {
        *(job->hunted_victim) = '\0';
        return FALSE;
     }
     if (targ = char_holding(temp_obj)) {
        if (targ == ch)   {
           if (temp_obj->equipped_by)
              unequip_char(targ, temp_obj->eq_pos);
           extract_obj(temp_obj);
           (job->num_retrieved)++;
           if (*(job->hunted_victim) != '\0') {
              do_action(ch, job->hunted_victim, 141);
              *(job->hunted_victim) = '\0';
           }
           else
              act("$n appears pleased.", FALSE, ch, NULL, targ, TO_ROOM);
           if (ch->master) 
              stop_follower(ch);
           if (job->num_chances != -99)
              job->num_chances = 9; 
           return TRUE;
        }
        if (*(job->hunted_victim) != '\0') {
           if (strcasecmp(job->hunted_victim, GET_NAME(targ))) {
              strcpy(job->hunted_victim, GET_NAME(targ));
              if (ch->master)
                 stop_follower(ch);
              if (job->num_chances != -99)  
                 job->num_chances = 9;
           }
        }
        else {
           strcpy(job->hunted_victim, GET_NAME(targ));
           if (ch->master)
              stop_follower(ch);
           if (job->num_chances != -99)
              job->num_chances = 9;
        }
     }
     room = room_of_object(temp_obj);
     if (room != ch->in_room)  {
        if ((dir = find_path(ch->in_room, is_target_room_p, room, -5000, 0)) < 0) {
           /* unable to find a path */
           if (room > 0) {
              do_emote(ch, "looks about for clues of passage, as if tracking someone.", 0);
              do_emote(ch, "fades almost unnoticed into the shadows.", 0);
              char_from_room(ch);
              char_to_room(ch, room);
              do_emote(ch, "steps from the shadows.", 0);
           }
           return FALSE;
        }
        go_direction(ch, dir);
     } else {
        if (targ) {
           do_wake(ch, targ->player.name, 46);
           if (!circle_follow(ch, targ))
              if (ch->master != targ) {
                 if (ch->master)
                    stop_follower(ch);
                 add_follower(ch, targ);
              }
           switch (job->num_chances) {
             case 9 : do_action(ch, targ->player.name, 187);
                      sprintf(buf, "Thats a pretty nice %s you have there, %s.",job->hunted_item, targ->player.name);
                      do_say(ch, buf, 17);
                      do_say(ch, "If you do not give the item to me, I will kill you and take it myself.", 17);
                      if (IS_NPC(targ))
                         do_say(targ, "Buzz off, creep.", 17);
             case 8 :
             case 5 :
             case 4 :
             case 2 :
             case 1 :
             case 7 : (job->num_chances)--;
                      break;
             case 6 : do_action(ch, targ->player.name, 190);
                      sprintf(buf, "I'm not kidding.  Hand over the %s!", job->hunted_item);
                      do_say(ch, buf, 17);
                      (job->num_chances)--;
                      if (IS_NPC(targ))
                         do_say(targ, "I said, BUZZ OFF!", 17);
                      break;
             case 3 : do_action(ch, targ->player.name, 94);
                      sprintf(buf, "This is your *LAST* warning.  Give me the %s or DIE!", job->hunted_item);
                      do_say(ch, buf, 17);
                      (job->num_chances)--;
                      if (IS_NPC(targ))
                         act("$n falls down laughing at $N.", FALSE, targ, NULL, ch, TO_ROOM);
                      break;
             default: if (IS_NPC(targ)) {
                         act("$n puts $N in a sleeper hold.", FALSE, ch, NULL, targ, TO_ROOM);
                         sprintf(buf, "$n quickly surrenders the %s.", job->hunted_item);
                         act(buf, FALSE, targ, NULL, targ, TO_ROOM);
                         if (temp_obj->in_obj)
                            obj_from_obj(temp_obj);
                         else if (temp_obj->carried_by)
                            obj_from_char(temp_obj);
                         else if (temp_obj->equipped_by)
                             unequip_char(targ, temp_obj->eq_pos);
                         obj_to_char(temp_obj, ch);
                      }
                      else if ((targ->desc) && (targ->desc->connected == CON_PLYNG)) {
                         if (check_peaceful(ch, "")) {
                            if (job->num_chances > -5) {
                               (job->num_chances)--;
                               sprintf(buf, "%s, I want your %s, and I MIGHT VERY WELL get tired of waiting!",
                                   GET_NAME(targ), job->hunted_item);
                               do_say(ch, buf, 17); 
                               if (job->num_chances == -5)
                                  do_say(ch, "Hell, thats it.  I *AM* tired of waiting for you.", 17);
                            }
                            else {
                               do_action(ch, GET_NAME(targ), 143);
                               if ((room = adjacent_room(ch->in_room)) < 1)  {
                                  act("$n mystically waves his hands at $N.", FALSE, ch, NULL, targ, TO_NOTVICT);
                                  act("$n mystically waves his hands at you!", FALSE, ch, NULL, targ, TO_VICT);
                                  act("$n and $N disappear into a swirling vortex!", FALSE, ch, NULL, targ, TO_NOTVICT);
                                  act("$n and you are sucked into a swirling vortex!", FALSE, ch, NULL, targ, TO_VICT);
                                  room = 6000;                  /* 6000 == edge of forest */
                               }
                               else  {
                                  /* throw them in some direction */
                                  act("$n grabs $N and throws $M from the room!", FALSE, ch, NULL, targ, TO_NOTVICT);
                                  act("$n grabs you and throws you from the room!", FALSE, ch, NULL, targ, TO_VICT);
                               }
                               char_from_room(ch);
                               char_from_room(targ);
                               char_to_room(ch, room);
                               char_to_room(targ, room);
                               do_look(ch, "\0", 0);
                               do_look(targ, "\0", 0);
                               hit(ch, targ, 0);
                            }
                         }
                         else 
                            hit(ch, targ, 0);
                      } else {
                        act("$n gets something from a link dead player and laughs evily.", FALSE, ch, NULL, targ, TO_ROOM);
                        if (temp_obj->carried_by)
                           obj_from_char(temp_obj);
                        else if (temp_obj->equipped_by)
                           unequip_char(targ, temp_obj->eq_pos);
                        else if (temp_obj->in_obj)
                           obj_from_obj(temp_obj); 
                        obj_to_char(temp_obj, ch);
                      }
                      break;
           }
        } else if (temp_obj->in_obj) {   /* item is in object */
           obj_from_obj(temp_obj);
           obj_to_char(temp_obj, ch);
           act("$n picks up something quickly.", FALSE, ch, NULL, 0, TO_ROOM);
        } else if (temp_obj->in_room != NOWHERE) {
           obj_from_room(temp_obj);
           obj_to_char(temp_obj, ch);
           act("$n picks up something quickly.", FALSE, ch, NULL, 0, TO_ROOM);
        } else {
           sprintf(buf, "Bounty hunter stuck looking for disconnected %s.", job->hunted_item);
           log(buf);
           do_say(ch, "Damn am I confused!", 17);
           free(job);
           ch->act_ptr = NULL;
        }
     }
     return TRUE;
  }
  else if (*(job->hunted_victim) != '\0') {
     if ((targ = char_with_name(job->hunted_victim)) == NULL)
        return FALSE;           
     if (targ->in_room == ch->in_room){
        do_wake(ch, targ->player.name, 46);
        if (!circle_follow(ch, targ))
           if (ch->master != targ) {
              if (ch->master)
                 stop_follower(ch);
              add_follower(ch, targ);
           }
        switch (job->num_chances) {
          case 9 : do_action(ch, targ->player.name, 187);
                   do_say(ch, "You have been requested to log off by my employer.", 17);
                   do_say(ch, "I suggest you comply.", 17);
          case 8 : case 7 : case 5: case 4: case 1 : case 2 :
                   (job->num_chances)--;
                   break;
          case 6 : do_action(ch, targ->player.name, 190);
                   do_say(ch, "I'm not kidding.", 17);
                   (job->num_chances)--;
                   break;
          case 3 : do_action(ch, targ->player.name, 94);
                   do_say(ch, "This is your *LAST* warning.", 17);
                   (job->num_chances)--;
                   break;
          default: if (check_peaceful(ch, ""))
                      do_say(ch, "You can't stay here forever, you know.", 17);
                   else 
                      hit(ch, targ, 0);
                   break;
        }
        return TRUE;
     }
     else if ((dir = find_path(ch->in_room, is_target_room_p, targ->in_room, -5000, 0)) >= 0)   {
        go_direction(ch, dir);
        return TRUE;
     }
  }
  return FALSE;
}

int potentially_annoying(int cmd) {
   return ((cmd == 27) ||       /* laugh  */
           (cmd == 30) ||       /* puke   */
           (cmd == 31) ||       /* growl  */
           (cmd == 33) ||       /* insult */
           (cmd == 111) ||      /* fart   */
           (cmd == 112) ||      /* flip   */
           (cmd == 113) ||      /* fondle */
           (cmd == 118) ||      /* grope  */
           (cmd == 130) ||      /* slap   */
           (cmd == 137) ||      /* spit   */
           (cmd == 160) ||      /* french */
           (cmd == 182) ||      /* fume   */
           (cmd == 189) ||      /* punch  */
           (cmd == 190) ||      /* snarl  */
           (cmd == 191) ||      /* spank  */
           (cmd == 193) ||      /* tackle */
           (cmd == 194) ||      /* taunt  */
           (cmd == 196) ||      /* whine  */
           (cmd == 261) ||      /* bonk   */
           (cmd == 262) ||      /* scold  */
           (cmd == 264) ||      /* rip    */
           (cmd == 271) ||      /* belittle */
           (cmd == 272) ||      /* piledrive */
           (cmd == 285) ||      /* flipoff */
           (cmd == 286) ||      /* moon   */
           (cmd == 287) ||      /* pinch  */
           (cmd == 288));       /* bite   */
}
 
/* DO NOT use the following as a stand-alone spec_proc... it is meant to be  */
/* called by other spec_procs.  Use the more general i_am_irritable instead  */
int utility_irritable(struct char_data *ch, int cmd, char *arg, int (*func)()) {
  struct char_data *me;
  struct char_data *targ;
 
  if (!cmd || IS_NPC(ch))
     return FALSE;
 
  if (potentially_annoying(cmd)) {
     do_action(ch, arg, cmd);
     for (; ((*arg == ' ') || (*arg == '\t')); arg++);
     if (targ = get_char_room_vis(ch, arg))
        for (me = real_roomp(ch->in_room)->people; me; me = me->next_in_room)
           if (IS_MOB(me) && mob_index[me->nr].func == func)
              if (!(me->specials.fighting) && !strcmp(GET_NAME(targ), GET_NAME(me))) {
                 act("$n becomes enraged by $N's behavior!", 1, me, 0, ch, TO_NOTVICT);
                 act("$n becomes enraged with you!", 1, me, 0, ch, TO_VICT);
                 do_hit(me, GET_NAME(ch), TYPE_UNDEFINED);
              }
     return TRUE;
  }
  return FALSE;
}
 

/* use this for mobs whose _ONLY_ spec_proc function is to be irritable */
int i_am_irritable(struct char_data *ch, int cmd, char *arg) {
   utility_irritable(ch, cmd, arg, i_am_irritable);
}


void PoliceHunt(struct char_data *ch, struct char_data *tch) {
#if NOTRACK
return;
#endif
   SET_BIT(ch->specials.act, ACT_HUNTING);
   ch->specials.hunting = tch;
   ch->hunt_dist = 30;
   ch->persist = 100;
   ch->old_room = ch->in_room;
}


/* like utility_irritable, this function is to be called by OTHER spec_procs.
   If you want a mob to JUST be a police person, call the i_am_police function
   below.  - SG */ 
int utilityPolice(struct char_data *ch, int cmd, char *arg, int (*func)()) {
   struct char_data *targ, *me;
   
   if (cmd) {                                /* "ch" is not me.  It is a player. */
      if (IS_PC(ch) && (IS_OUTLAW(ch) || IS_KILLER(ch)) && (ch->in_room != NOWHERE)) {
         for (me = real_roomp(ch->in_room)->people; me; me = me->next_in_room)
            if (IS_MOB(me) && (mob_index[me->nr].func == func))
               if (AWAKE(me) && !(me->specials.fighting) && (me->specials.hunting != ch) && CAN_SEE(me, ch)) {
                  PoliceHunt(me, ch);
                  if (!check_peaceful(ch, "")) {
                     act("$n points at $N screaming 'DIE CRIMINAL!'", 1, me, 0, ch, TO_NOTVICT);
                     act("$n points at you screaming 'DIE CRIMINAL!'", 1, me, 0, ch, TO_VICT);
                     hit(me, ch, TYPE_UNDEFINED);
                  }
               }
      }
   }
   else {                                    /* "ch" is me, the policeman */
      if (AWAKE(ch) && !(ch->specials.fighting) && (ch->in_room != NOWHERE)) 
         for (targ = real_roomp(ch->in_room)->people; targ; targ = targ->next_in_room)
            if (IS_PC(ch) && (ch->specials.hunting != targ) && (IS_OUTLAW(targ) || IS_KILLER(targ)) && (CAN_SEE(ch, targ))) {
                  PoliceHunt(ch, targ);
                  if (!check_peaceful(ch, "")) {
                     hit(ch, targ, TYPE_UNDEFINED);
                     act("$n points at $N screaming 'DIE CRIMINAL!'", 1, ch, 0, targ, TO_NOTVICT);
                     act("$n points at you screaming 'DIE CRIMINAL!'", 1, ch, 0, targ, TO_VICT);
                  }              
                  break;
               }     
   }
   
   return FALSE;
}


/* use this for mobs whose _ONLY_ spec_proc function is to be police */
int i_am_police(Mob *ch, int cmd, char *arg) {
   return utilityPolice(ch, cmd, arg, i_am_police);
}


void obj_act(char *message, Mob *ch, Obj *o, Mob *vict) {
   char buffer[256];

   sprintf(buffer, "$n's $p %s", message);
   act(buffer, 1, ch, o, vict, TO_ROOM);
   sprintf(buffer, "Your $p %s", message);
   act(buffer, 1, ch, o, vict, TO_CHAR);
}


int warMaker(Mob *ch, int cmd, char *arg, Obj *o) {
   char buf[256];
   
   if ((cmd > 83) && (o->in_room == -1)) {
      if ((o->equipped_by != ch) && (o->carried_by != ch))
         return FALSE;   /* safety net... should never happen */
      switch (cmd) {
         case 84  : 
         case 207 :  act("$n's $p hums 'Get this:  $n wants to use some sissy magic!'", 1, ch, o, NULL, TO_ROOM);
                     act("Your $p hums 'Get this:  $n wants to use some sissy magic!'", 1, ch, o, NULL, TO_CHAR);
                     send_to_char("You feel confused... what was it you were going to do?\n\r", ch);
                     return TRUE;
         case 151 :  if (o->equipped_by) {
                        act("$n's $p glows red-hot in $s hands!", 1, ch, o, NULL, TO_ROOM);
                        act("Your $p glows red-hot in your hands!", 1, ch, o, NULL, TO_CHAR);
                        GET_HIT(ch) -= dice(5, 5);
                        if (GET_HIT(ch) < 0) {
                           GET_HIT(ch) = 0;
                           GET_POS(ch) = POSITION_STUNNED;
                        }
                        if (dice(1, 20) > GET_CON(ch)) {
                           unequip_char(ch, o->eq_pos);
                           obj_to_room(o, ch->in_room);
                           act("$n screams loudly, dropping $s $p.", 1, ch, o, NULL, TO_ROOM);
                           act("You scream loudly, dropping your $p.", 1, ch, o, NULL, TO_CHAR);
                        }
                     }
         default  :  return FALSE;
      }
   } else if (!cmd) {
      if (o->in_room != -1) {
         sprintf(buf, "%s moves a bit... as if alive!\n\r", o->short_description);
         send_to_room(buf, room_of_object(o));
      }
      else if (ch = o->equipped_by) {
         if (!(ch->specials.fighting)) {
            switch (dice(1, 30)) {
               case 1:  obj_act("sings 'Follow the Yellow Brick Road.'", ch, o, NULL);
                        break;
               case 2:  obj_act("says, 'Let's go to the Shire, I want to waste some halfling youths.'", ch, o, NULL);
                        break;
               case 3:  obj_act("says, 'Use the Force, L.., I mean:  go get um tiger.'", ch, o, NULL);
                        break;
               case 4:  obj_act("bemoans, 'How is it such a wonderous sword as me gets stuck with a wimp and coward like you?'", ch, o, NULL);
                        break;
               case 5:  obj_act("says, 'We gonna stand here all day, or we gonna kill this thing?'", ch, o, NULL);
                        break;
               case 6:  obj_act("says, 'Group me.'", ch, o, NULL);
                        break;
               case 7:  obj_act("says, 'That's it.  Puff must die.'", ch, o, NULL);
                        break;
               case 8:  obj_act("says, 'Some of my closest friends are training daggers.'", ch, o, NULL);
                        break;
               case 9:  obj_act("grins, 'I love it when they buff up mobs, it gives me a challenge.'", ch, o, NULL);
                        break;
               case 10: obj_act("wonders, 'Is it me, or are most other swords rather quiet?'", ch, o, NULL);
                        break;
               case 11: obj_act("says, 'In a past life, I was a pipeweed bread.'", ch, o, NULL);
                        break;
               case 12: obj_act("says, 'Puff sure is a friendly dragon, isn't he.'", ch, o, NULL);
                        break;
               case 13: obj_act("says, 'The imps must hate the players to make a sword SOooO AnnOYing!'", ch, o, NULL);
                        break;
               case 14: obj_act("wonders, 'How come I'm never forced to shout how much I love Puff?'", ch, o, NULL);
                        break;
               case 15: obj_act("says, 'Let's see who's using Tin tin.  Snowy shouts 'Yo''", ch, o, NULL);
                        break;
               case 16: obj_act("screams, 'BANZAI, CHARGE, SPORK, GERONIMO, DIE VERMIN, HIEYAH!'", ch, o, NULL);
                        obj_act("blushes, 'Ooops, sorry, got over anxious to kill something.'", ch, o, NULL);
                        break;
               case 17: obj_act("offers, 'A good Swedish massage would cure what ails you.'", ch, o, NULL);
                        break;
               case 18: obj_act("muses: 'How exactly *does* an inanimate object talk?'", ch, o, NULL);
                        break;
               case 19: obj_act("states, 'I'd rather be playing scrabble.'", ch, o, NULL);
                        break;
               case 20: obj_act("says, 'A corpse is something to be cherished forever...until it rots.'", ch, o, NULL);
                        break;
               case 21: obj_act("sniffs the air, 'I love the smell of blood and ichor in the morning!'", ch, o, NULL);
                        break;
               case 22: obj_act("chants, 'You might have armor or you might have fur, I'll hack them both, cuz I'm War-make-er.'", ch, o, NULL);
                        break;
               case 23: obj_act("scoffs, 'Sword of the Ancients, what a piece of junk!'", ch, o, NULL);
                        break;
               case 24: obj_act("says, 'Do I have to fight again?  I just got all the gore off from last time.'", ch, o, NULL);
                        break;
					case 26: obj_act("moans, 'If I have to kill Aunt Bee one more time, I'm gonna scream!'", ch, o, NULL);
                        break;
               default: obj_act("grumbles 'Lets go kill something!'", ch, o, NULL);
                        break;         
            }              
         }
      }
      else if (o->in_obj) {
         sprintf(buf, "Something grumbles 'Damnit, I'm %s.  Let me out of here.  Its dark.'\n\r",
                 o->short_description);
         send_to_room(buf, room_of_object(o));     
      }
      else if (ch = o->carried_by) {
            act("$n's $p begs $m to wield it.", 1, ch, o, NULL, TO_ROOM);
            act("Your $p begs you to wield it.", 1, ch, o, NULL, TO_CHAR);         
      }
   } else if ((cmd == OBJECT_HITTING) && (dice(1,10) == 1)) {
      switch (dice(1, 20)) {
         case 1:  obj_act("criticizes $N's lineage.", o->equipped_by, o, ch);
                  break;         
         case 2:  obj_act("says to $N, 'I fart in your general direction.'", o->equipped_by, o, ch);
                  break;
         case 3:  obj_act("looks at $N and asks, 'Is that your real face or are you just naturally ugly?'",
                     o->equipped_by, o, ch);
                  break;
         case 4:  obj_act("tells $N, 'If they held an ugly pageant, you'd be a shoe in.'", o->equipped_by, o, ch);
                  break;
         case 5:  obj_act("tells $N, 'I wave my private parts in your direction!'", o->equipped_by, o, ch);
                  break;
         case 6:  obj_act("snickers something about $N's mother and combat boots.", o->equipped_by, o, ch);
                  break;
         case 7:  obj_act("asks $N with a smirk, 'So, how many newbies have killed you today?'", o->equipped_by, o, ch);
                  break;
         case 8:  obj_act("states, 'Ya know, your sister was a mighty fine piece of ...'", o->equipped_by, o, ch);
                  break;
         case 9:  obj_act("looks at $N and says 'Not worth the time.'", o->equipped_by, o, ch);
                  break;
         case 10: obj_act("tells $N, 'Killing you is about as challenging as buying bread.'", o->equipped_by, o, ch);
                  break;
         case 11: obj_act("laughs at the stupidity of $N.", o->equipped_by, o, ch);
                  break;
         case 12: obj_act("ponders: 'think I can kill $N in 5 rounds or less?'", o->equipped_by, o, ch);
                  break;
         case 13: obj_act("snickers, 'OOH! Look at the the fangs on that one.'", o->equipped_by, o, ch);
                  break;
         case 14: obj_act("groans, '$N?!!  Surely you jest?  Haven't we killed $M enough all ready?'", o->equipped_by, o, ch);
                  break;
         case 15: obj_act("looks at $N and sighs, 'When you get around to attacking something real, wake me.'", o->equipped_by, o, ch);
                  break;
         case 16: obj_act("grins, 'Your head will look great mounted up above my mantle piece.'", o->equipped_by, o, ch);
                  break;
         case 17: obj_act("looks at $N and says, 'Are you as stupid as you look, or do you just look stupid?'", o->equipped_by, o, ch);
                  break;
	  		default: obj_act("taunts $N mercilessly.", o->equipped_by, o, ch);
                  break;         
     }            
     if (ch->specials.fighting != o->equipped_by) {
        act("$N turns towards $n with rage in $S eyes.", 1, o->equipped_by, o, ch, TO_ROOM);
        act("$N turns towards you with rage in $S eyes.", 1, o->equipped_by, o, ch, TO_CHAR);
        stop_fighting(ch);
        set_fighting(ch, o->equipped_by);
     }
   }
      
   return FALSE;
}

int orbOfDestruction(Mob *ch, int cmd, char *arg, Obj *o) {
   char buffer[256];
   Mob  *v, *n;
   Room *r;
 
   if (cmd == 172) {    /* use */
      arg = one_argument(arg, buffer);
      if (isname(buffer, o->name)) {
         if (GET_INT(ch) < 15)
            send_to_char("You can't figure out how to use it.\n\r", ch);
         else if (!o->equipped_by)
            send_to_char("You must be holding it to use it.\n\r", ch);
         else {
            act("You trigger $p, causing it to explode in a fiery blast of light!", 1, ch, o, NULL, TO_CHAR);
            act("$n's $p explodes in a fiery blast of light!", 1, ch, o, NULL, TO_ROOM);
            unequip_char(ch, o->eq_pos);
            extract_obj(o);
            r = real_roomp(ch->in_room);
            for (v = r->people; v; v = n) {
                n = v->next_in_room;
                if (v != ch)
                   MissileDamage(ch, v, dice(10, 100), SPELL_DISINTEGRATE);
            }
            GET_HIT(ch) = 1;
            spell_teleport(50, ch, ch, NULL);
         }
         return TRUE;
      }
   }
   return FALSE;
}


