/* These are new functions added by Russ Russell, after the code was stolen.
** These are mainly skills and spells for the new classes.                 */

#include <stdio.h>
#include <assert.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "db.h"
#include "race.h"


extern struct index_data *obj_index;
extern struct dex_app_type dex_app[];

/* These are some new skills for the new classes and for thiefs and warriors */

#define WIND_POTION 3671
#define HEAL_POTION 6109
#define SANC_POTION 15811
#define FULL_POTION 29900
#define FLY_POTION  29907
#define TRUE_POTION 29908
#define FIRE_POTION 29909
#define SAC1_FOR_HEAL 29910
#define SAC2_FOR_HEAL 29911
#define SAC3_FOR_HEAL 29912
#define SAC1_FOR_SANC 29913
#define SAC2_FOR_SANC 29914
#define SAC3_FOR_SANC 29915
#define SAC1_FOR_FULL 29916
#define SAC2_FOR_FULL 29917
#define SAC3_FOR_FULL 29918
#define SAC1_FOR_FLY  29919
#define SAC2_FOR_FLY  29920
#define SAC3_FOR_FLY  29921
#define SAC1_FOR_TRUE 29922
#define SAC2_FOR_TRUE 29923
#define SAC3_FOR_TRUE 29924
#define SAC1_FOR_FIRE 29925
#define SAC2_FOR_FIRE 29926
#define SAC3_FOR_FIRE 29927
#define SAC1_FOR_WIND 29928
#define SAC2_FOR_WIND 29929
#define SAC3_FOR_WIND 29930

void do_brew(struct char_data *ch, char *arg, int cmd)
{
   char buf[MAX_STRING_LENGTH];
   int obj, sacr, sacr1, sacr2, percent;
   struct obj_data *obje, *sac, *sac1, *sac2;
   struct obj_data *next_obj;
   struct affected_type af;
  
   if (!ch->skills)  
     return;

   one_argument(arg, buf);

   if (!str_cmp(buf, "sanctuary")) {
     obj  = SANC_POTION;
     sacr = SAC1_FOR_SANC;
     sacr1= SAC2_FOR_SANC;
     sacr2= SAC3_FOR_SANC;
   } else if (!str_cmp(buf, "fireshield")) {
     obj  = FIRE_POTION;
     sacr = SAC1_FOR_FIRE;
     sacr1= SAC2_FOR_FIRE;
     sacr2= SAC3_FOR_FIRE;
   } else if (!str_cmp(buf, "fly")) {
     obj = FLY_POTION;
     sacr = SAC1_FOR_FLY;
     sacr1= SAC2_FOR_FLY;
     sacr2= SAC3_FOR_FLY;
   } else if (!str_cmp(buf, "true")) {
     obj  = TRUE_POTION;
     sacr = SAC1_FOR_TRUE;
     sacr1= SAC2_FOR_TRUE;
     sacr2= SAC3_FOR_TRUE;
   } else if (!str_cmp(buf, "heal")) {
     obj  = HEAL_POTION;
     sacr = SAC1_FOR_HEAL;
     sacr1= SAC2_FOR_HEAL;
     sacr2 = SAC3_FOR_HEAL;
   } else if (!str_cmp(buf, "full")) {
     obj  = FULL_POTION;
     sacr = SAC1_FOR_FULL;
     sacr1= SAC2_FOR_FULL;
     sacr2= SAC3_FOR_FULL;
   } else if (!str_cmp(buf, "wind")) {
     obj = WIND_POTION;
     sacr = SAC1_FOR_WIND;
     sacr1= SAC2_FOR_WIND;
     sacr2= SAC3_FOR_WIND;
   } else {
     send_to_char("You can't mix a potion of that type.\n\r", ch);
     return;
   }

  percent = number(1,101);

 sacr = real_object(sacr);
 sacr1= real_object(sacr1);
 sacr2= real_object(sacr2);

 sac = get_obj_in_list_vis(ch, obj_index[sacr].name, ch->carrying);
 sac1 = get_obj_in_list_vis(ch, obj_index[sacr1].name, ch->carrying);
 sac2 = get_obj_in_list_vis(ch, obj_index[sacr2].name, ch->carrying);
if (sac) {
  if (sac1) {
    if (sac2) {
     if (percent < ch->skills[SKILL_BREW].learned) {
    obje = read_object(obj, VIRTUAL);
    if (!obje) {
      send_to_char("There are no potions of that type available\n\r", ch);
      return;
    } else {
      send_to_char("You start to brew your potion.\n\r", ch);
      act("$n starts to brew a potion.", TRUE, ch, 0, 0, TO_ROOM);
 
       if(!IS_IMMORTAL(ch)) {
        af.type      = SKILL_BREW;
        af.duration  = 3;
        af.modifier  = 0;
        af.location  = 0;
        af.bitvector = AFF_BREWING;

        affect_to_char(ch, &af);
       }

      obj_to_char(obje, ch);
      obj_from_char(sac);
      extract_obj(sac);
      obj_from_char(sac1);
      extract_obj(sac1);
      obj_from_char(sac2);
      extract_obj(sac2);
    }
    } else {
      send_to_char("You try to mix a potion.\n\r", ch);
      send_to_char("Your incompetence results in an unusable potion.\n\r", ch);
      obj_from_char(sac);
      extract_obj(sac);
      obj_from_char(sac1);
      extract_obj(sac1);
      obj_from_char(sac2);
      extract_obj(sac2);
      return;
    }
   } else {
   send_to_char("You need all three items.\n\r", ch);
   return;
   }
  } else {
   send_to_char("You must have the correct items to make the potion.\n\r", ch);
   return;
  }
   } else {
     send_to_char("You need all three components for your potion.\n\r", ch);
     return;
   }
}

void do_search(struct char_data *ch, char *arg, int cmd)
{
   int j, num, learned;
   char buf[256];
   struct room_data *rp;
   struct room_direction_data *fdd;
   extern char *exits[];
   *buf = '\0';

   if (!HasClass(ch, CLASS_THIEF)) {
      send_to_char("Only thieves have the ability to search!\n\r", ch);
      return;
      }

   rp = real_roomp(ch->in_room);

     for (j=0;j<6;j++) {
       fdd = rp->dir_option[j];
      if (fdd) {
           num = number(1,100);
     
        if (j <4) {
          sprintf(buf,"$n searches the %s wall for secret doors.",exits[j]);
          act(buf,FALSE, ch, 0, 0, TO_ROOM);
        } else if (j < 5) {
          act("$n searches the ceiling for secret doors.",FALSE, ch, 0, 0, TO_ROOM);
        } else {
          act("$n searches the floor for secret doors.",FALSE,ch,0,0,TO_ROOM);
        }
        
           learned = ((GET_DEX(ch) + GET_INT(ch) + GET_LEVEL(ch, THIEF_LEVEL_IND)+ ch->skills[SKILL_DETECT_SECRET].learned)/2);
         if ((IS_SET(fdd->exit_info, EX_SECRET)) && (num <= learned)) {
           sprintf(buf, "Secret door found %s! Door is named %s.\n\r", 
                  exits[j],fname(fdd->keyword));
           send_to_char(buf, ch);
           sprintf(buf, "$n exclaims 'Look %s! A SECRET door named %s!\n\r",
                  exits[j],fname(fdd->keyword));
           act(buf, FALSE, ch, 0, 0, TO_ROOM);
           send_to_char("You are totally exhausted from your searching!\n\r", ch);
           act("$n is totally exhausted from $s search.",FALSE,ch,0,0,TO_ROOM);
           GET_MOVE(ch) = MAX(0,(GET_MOVE(ch)-75));
           return;
           }
        }
     }
     send_to_char("No secret doors in this room.\n\r", ch);
     send_to_char("You are totally exhausted from your searching!\n\r", ch);
     act("$n is totally exhausted from $s search.",FALSE,ch,0,0,TO_ROOM);
     GET_MOVE(ch) = MAX(0,(GET_MOVE(ch)-75)); 
     return;
}

void do_set_traps(struct char_data *ch, char *arg, int cmd)
{
   char buf[80];
   struct obj_data *trap;
}

void do_grapple(struct char_data *ch, char *arg, int cmd) 
{
   struct char_data *victim;
   char name[256];
   byte percent;
   struct affected_type af;
   int i;
 
   if (!ch->skills)
     return;

   if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n\r"))
   return;

   only_argument(arg, name);

   if (!(victim = get_char_room_vis(ch, name))) {
     if (ch->specials.fighting) {
        victim = ch->specials.fighting;
     } else {
        send_to_char("Grapple with whom?\n\r", ch);
        return;
     }
   }
  
   if (victim == ch) {
      send_to_char("Aren't we funny today?\n\r", ch);
      return;
   }


   percent=number(1,101); /* As usual 101 is a COMPLETE failure. */

   percent -= dex_app[GET_DEX(ch)].reaction*5;
   percent += dex_app[GET_DEX(victim)].reaction*10;

   if (GetMaxLevel(victim) > 20)
      percent += ((GetMaxLevel(victim)-18)*5);

   if (percent < ch->skills[SKILL_GRAPPLE].learned) {
      send_to_char("You tie your opponent up, with an excellent maneuver.\n\r", ch);
      act("$n wrestles $N to the ground with an excellent maneuver.", TRUE,
           ch, 0, victim, TO_NOTVICT);
      act("$n wrestles you to the ground.", TRUE, ch, 0, 0, TO_VICT);
            
      SET_BIT(ch->specials.affected_by, AFF_GRAPPLE);
      SET_BIT(victim->specials.affected_by, AFF_GRAPPLE);
      GET_POS(ch) = POSITION_SITTING;
      GET_POS(victim) = POSITION_SITTING;
      WAIT_STATE(ch, 5*PULSE_VIOLENCE);

    if (ch->specials.fighting)
      stop_fighting(ch);
    if (victim->specials.fighting)
      stop_fighting(victim); 
    act("$N now turns $S attention to $n!", TRUE,ch, 0, victim, TO_ROOM);
    set_fighting(victim,ch);


   } else {
      SET_BIT(ch->specials.affected_by, AFF_GRAPPLE);
      GET_POS(ch) = POSITION_SITTING;
      WAIT_STATE(ch, 3*PULSE_VIOLENCE);
      act("You try to wrestle $N to the ground, but end up falling on your butt.", TRUE, ch, 0 , victim, TO_CHAR);
      act("$n makes a nice wrestling move, but falls on his butt.",
           TRUE, ch, 0, 0, TO_ROOM);
      
      if (ch->specials.fighting)
        stop_fighting(ch);
      if (victim->specials.fighting)
        stop_fighting(victim);
      act("$N turns $S attention to $n", TRUE,ch,0,victim, TO_ROOM);
      set_fighting(victim,ch);
   }
}  


#define PAPER 17005
#define PEN 17006
#define HEAL_SCROLL 29931
#define SANC_SCROLL 29932
#define FLY_SCROLL 29933
#define WIND_SCROLL 29934
#define TRUE_SCROLL 29935
#define SAC_FOR_HEAL 29936
#define SAC_FOR_SANC 29937
#define SAC_FOR_FLY 29938
#define SAC_FOR_WIND 29939
#define SAC_FOR_TRUE 29940

void do_scribe(struct char_data *ch, char *arg, int cmd)
{
   char buf[MAX_STRING_LENGTH];
   int obj, sacr, pen, paper, percent;
   struct obj_data *obje, *sac, *penw, *paperw;
   struct obj_data *next_obj;
   struct affected_type af;

   if (!ch->skills)
     return;

   one_argument(arg, buf); 

     if (!str_cmp(buf, "heal")) {
      obj = HEAL_SCROLL;
      sacr = SAC_FOR_HEAL;
     } else if (!str_cmp(buf, "fly")) {
      obj = FLY_SCROLL;
      sacr = SAC_FOR_FLY;
     } else if (!str_cmp(buf, "true")) {
      obj  = TRUE_SCROLL;
      sacr = SAC_FOR_TRUE;
     } else if (!str_cmp(buf, "wind")) {
      obj = WIND_SCROLL;
      sacr = SAC_FOR_WIND;
     } else if (!str_cmp(buf, "sanc")) {
      obj = SANC_SCROLL;
      sacr = SAC_FOR_SANC;
     } else {
      send_to_char("You can't write that type of scroll!\n\r", ch);
      return;
     }

   pen = PEN;
   paper = PAPER;

     percent = number(1,101);

   sacr = real_object(sacr);
   pen = real_object(pen);
   paper = real_object(paper);

   sac = get_obj_in_list_vis(ch, obj_index[sacr].name, ch->carrying);
   penw = get_obj_in_list_vis(ch, obj_index[pen].name, ch->carrying);
   paperw = get_obj_in_list_vis(ch, obj_index[paper].name, ch->carrying);

  if (sac) {
   if (penw) {
    if (paperw) { 
    if (percent < ch->skills[SKILL_SCRIBE].learned) {
    obje = read_object(obj, VIRTUAL);
    if (!obje) {
      send_to_char("There are no scrolls of that type available.\n\r", ch);
      return;
    } else {
      send_to_char("You start to write your scroll.\n\r", ch);
      act("$n starts to write a scroll.", TRUE, ch, 0, 0, TO_ROOM);

      if(!IS_IMMORTAL(ch)) {
        af.type      = SKILL_SCRIBE;
        af.duration  = 3;
        af.modifier  = 0;
        af.location  = 0;
        af.bitvector = AFF_BREWING;

        affect_to_char(ch, &af);
       }

      obj_to_char(obje, ch);
      obj_from_char(sac);
      extract_obj(sac);
      obj_from_char(paperw);
      extract_obj(paperw);
    }
  } else {
    send_to_char("You try to write a scroll.\n\r", ch);
      send_to_char("Your incompetence results in an unusable scroll.\n\r", ch);
      obj_from_char(sac);
      obj_from_char(paperw);
  }
     } else {
       send_to_char("You need a piece of papaer to write a scroll.\n\r", ch);
       return;
     }
    } else {
      send_to_char("You need a pen to write a scroll.\n\r", ch);
      return;
    }
  } else {
    send_to_char("You need an item to sacrifice.\n\r", ch);
    return;
  }
}

void do_glance(struct char_data *ch, char *argument, int cmd)
{
char buffer[MAX_STRING_LENGTH];
int bits, percent;
struct char_data *tmp_char;
struct obj_data *found_object;
 
tmp_char=0;
found_object=0;
if (*argument) {
 
   bits=generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
   if (tmp_char) {
      if (GET_MAX_HIT(tmp_char) > 0)
             percent = (100*GET_HIT(tmp_char))/GET_MAX_HIT(tmp_char);
           else
             percent = -1; /* How could MAX_HIT be < 1?? */
 
       if (IS_NPC(tmp_char))
       	  strcpy(buffer, tmp_char->player.short_descr);
        else
          strcpy(buffer, GET_NAME(tmp_char));
 
       if (percent >= 100)
              strcat(buffer, " is in an excellent condition.\n\r");
        else if (percent >= 90)
            strcat(buffer, " has a few scratches.\n\r");
        else if (percent >= 75)
            strcat(buffer, " has some small wounds and bruises.\n\r");
       	else if (percent >= 50)
            strcat(buffer, " has quite a few wounds.\n\r");
       	else if (percent >= 30)
            strcat(buffer, " has some big nasty wounds and scratches.\n\r");
       	else if (percent >= 15)
             strcat(buffer, " looks pretty hurt.\n\r");
       	else if (percent >= 0)
             strcat(buffer, " is in an awful condition.\n\r");
       	else
             strcat(buffer, " is bleeding awfully from big wounds.\n\r");
if (CAN_SEE(ch,tmp_char))
       send_to_char(buffer, ch);
    
    }
  } else {
     send_to_char("Glance at whom?\n\r", ch);
     return;
  }
}

void do_deathstroke(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  byte percent;
 
  if (!ch->skills)
    return;
 
  if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n\r"))
    return;


  if (GET_MOVE(ch) < 50) {
    send_to_char("You don't have the vitality to make the move!\n\r", ch);
    return;
  }
 
  only_argument(argument, name);
 
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Death-stroke who?\n\r", ch);
      return;
    }
  }
 
  if (victim == ch) {
    send_to_char("Do you REALLY want to kill yourself?...\n\r", ch);
    return;
  }
 
  /* to prevent from misuse in groups ... */
  if (ch->attackers > 3) {
    send_to_char("You can't get a clear shot at your opponents vitals!\n\r",ch);

    return;
  }
 
  if (victim->attackers >= 3) {
    send_to_char("You are too busy fending off other attackers!\n\r", ch);
    return;
  }
 
  /* AC makes a big difference here ... */
  percent=((10-(GET_AC(victim)/5))<<1) + number(1,101);   /* 101% is a
complete failure */
  percent -= dex_app[GET_DEX(ch)].reaction*10;
  percent += dex_app[GET_DEX(victim)].reaction*10;
 
  if (percent > ch->skills[SKILL_DEATHSTROKE].learned) {
    if (GET_POS(victim) > POSITION_DEAD)
      damage(ch, victim, 0, SKILL_DEATHSTROKE);
    /* with great failure, comes great sorrow *grin* */
    WAIT_STATE(ch, PULSE_VIOLENCE*10);                  /* player stuck
for 10 rounds */
    GET_MOVE(ch) = 0;                                   /* player
exhausted */
    percent=((10-(GET_AC(ch)/5))<<1) + number(1,101);   /* 101% is a
complete failure */
    percent -= dex_app[GET_DEX(victim)].reaction*10;
    percent += dex_app[GET_DEX(ch)].reaction*10;
    /* what happens here is that the mob gets a shot at the players vitals */
    /* ... fair is fair right?  */
    if (percent > ch->skills[SKILL_DEATHSTROKE].learned) {
        /* monster hits player vitals while player is exposed */
        damage(victim, ch, (3*GET_STR(victim) + 3*(GetMaxLevel(victim) + GET_ADD(victim))), SKILL_DEATHSTROKE);
    }
  } else {
    if (GET_POS(victim) > POSITION_DEAD)
       damage(ch, victim, (3*GET_STR(ch) + 3*(GET_LEVEL(ch,
WARRIOR_LEVEL_IND)) + GET_ADD(ch)), SKILL_DEATHSTROKE);
    /* success is not without drawbacks */
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    GET_MOVE(ch) -= 50;
    GET_MOVE(ch) = MAX(0, GET_MOVE(ch));
  }
 
  /* success OR failure make monster attack this player as player is now */
  /* perceived as a greatest threat to the monster's livelyhood */
 if (ch->specials.fighting) 
  stop_fighting(ch);
 if (victim->specials.fighting)
  stop_fighting(victim);
 act("$N turns $S attention to $n.",1,ch,0,victim,TO_ROOM);
  set_fighting(victim, ch);
}

void do_bodyslam(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  byte percent;
 
  if (!ch->skills)
    return;
 
  if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n\r"))
  return;


 
  only_argument(argument, name);
 
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Bodyslam who?\n\r", ch);
      return;
    }
  }
 
  if (victim == ch) {
    send_to_char("Heh heh, cute...\n\r", ch);
    return;
  }

  if (GET_MOVE(ch) < 50) {
     send_to_char("You don't have the vitality to bodyslam anyone!\n\r", ch);
     return;
  }
 
  /* to prevent from misuse in groups ... */
  if (ch->attackers > 3) {
    send_to_char("Too many people in the way!\n\r",ch);
    return;
  }
 
  if (victim->attackers >= 3) {
    send_to_char("You are too busy fending off other attackers!\n\r", ch);
    return;
  }
 
  setKillerFlag(ch,victim);

  /* AC makes less difference here ... */
  percent=((10-(GET_AC(victim)/20))<<1) + number(1,101);   /* 101% is a
complete failure */
  percent -= dex_app[GET_DEX(ch)].reaction*10;
  percent += dex_app[GET_DEX(victim)].reaction*10;
 
  /* remember, F = MA :) need to take  ch->player.weight into account */
 
  if (percent > ch->skills[SKILL_BODYSLAM].learned) {
    if (GET_POS(victim) > POSITION_DEAD) {
       damage(ch, victim, 0, SKILL_BODYSLAM);   /* notify PC he missed */
       /* player has thrown his body weight at someone and missed -- oops */
       WAIT_STATE(ch, PULSE_VIOLENCE*4);                        /* player
stuck for 4 rounds */
       /* player does 1/4 the damage to himself that he would have done to
enemy */
       send_to_char("You hurt yourself as you fall on your face.\n\r", ch);
       damage(victim, ch, ((victim->player.weight / 10) * GET_STR(victim)
/4), TYPE_HIT);
       GET_POS(ch) = POSITION_SITTING;
       /* idea... force all things in room to "giggle" at this ch */
    }
  } else {
    if (GET_POS(victim) > POSITION_DEAD) {
       damage(ch, victim, (((ch->player.weight / 10) * GET_STR(ch)
*(GET_MOVE(ch)/10))/30), SKILL_BODYSLAM);
       GET_POS(victim) = POSITION_SITTING;
       WAIT_STATE(ch, PULSE_VIOLENCE*2);
    }
  }
  GET_MOVE(ch) = MAX(0,(GET_MOVE(ch)-50));
}



   
