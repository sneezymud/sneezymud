/* ************************************************************************
*  file: magic.c , Implementation of spells.              Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

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

/* Extern structures */
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct obj_data  *object_list;
extern struct char_data *character_list;

/* Extern procedures */

extern void add_follower(struct char_data *ch, struct char_data *leader);
extern void stop_follower(struct char_data *ch);
extern void gain_exp(struct char_data *ch, int gain);
void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
void MissileDamage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
bool saves_spell(struct char_data *ch, sh_int spell);
void weight_change_object(struct obj_data *obj, int weight);
char *strdup(char *source);
int dice(int number, int size);
char in_group(struct char_data *ch1, struct char_data *ch2);
void set_fighting(struct char_data *ch, struct char_data *vict);
bool ImpSaveSpell(struct char_data *ch, sh_int save_type, int mod);
int IsPerson( struct char_data *ch);
int IsExtraPlanar( struct char_data *ch);
void RawSummon( struct char_data *v, struct char_data *c);

/* Offensive Spells */

void spell_magic_missile(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  dam = number((int)(level / 2)+1,4)+(level / 2);

  if (affected_by_spell(victim,SPELL_SHIELD))
    dam = 0;

  MissileDamage(ch, victim, dam, SPELL_MAGIC_MISSILE);
}



void spell_chill_touch(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  dam = number(level, 3*level);

  if ( !saves_spell(victim, SAVING_SPELL) )
  {
    af.type      = SPELL_CHILL_TOUCH;
    af.duration  = 6;
    af.modifier  = -1;
    af.location  = APPLY_STR;
    af.bitvector = 0;
    affect_join(victim, &af, TRUE, FALSE);
  } else {
    dam >>= 1;
  }
  damage(ch, victim, dam, SPELL_CHILL_TOUCH);
}

void spell_burning_hands(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(1,6) + level + 1;

  send_to_char("Searing flame fans out in front of you!\n\r", ch);
  act("$n sends a fan of flame shooting from the fingertips!\n\r",
     FALSE, ch, 0, 0, TO_ROOM);

   for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; 
   tmp_victim = tmp_victim->next_in_room) {
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
         if ((GetMaxLevel(tmp_victim)>LOW_IMMORTAL) && (!IS_NPC(tmp_victim)))
            return;
         if (!in_group(ch, tmp_victim)) {
            act("You are seared by the burning flame!\n\r",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
            if ( saves_spell(tmp_victim, SAVING_SPELL) )
                dam >>= 1;
          MissileDamage(ch, tmp_victim, dam, SPELL_BURNING_HANDS);
    } else {
            act("You are able to avoid the flames!\n\r",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
     }
       }
    }
}



void spell_shocking_grasp(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  dam = number(1,8)+level;

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_SHOCKING_GRASP);
}



void spell_lightning_bolt(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  dam = dice(level,6);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}



void spell_colour_spray(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

   assert(victim && ch);
   assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = 4 * level;

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  MissileDamage(ch, victim, dam, SPELL_COLOUR_SPRAY);

}


/* Drain XP, MANA, HP - caster gains HP and MANA */
void spell_energy_drain(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam, xp, mana;

  void set_title(struct char_data *ch);
  void gain_exp(struct char_data *ch, int gain);

  assert(victim && ch);
  assert((level >= 1) && (level <=  ABS_MAX_LVL));

  if ( !saves_spell(victim, SAVING_SPELL) ) {
    GET_ALIGNMENT(ch) = MIN(-1000, GET_ALIGNMENT(ch)-200);

    if (GetMaxLevel(victim) <= 1) {
      damage(ch, victim, 100, SPELL_ENERGY_DRAIN); /* Kill the sucker */
    } else if ((!IS_NPC(victim)) && (GetMaxLevel(victim) >= LOW_IMMORTAL)) {
      send_to_char("Some puny mortal just tried to drain you...\n\r",victim);
    } else {

      if (!IS_SET(victim->M_immune, IMM_DRAIN)) {

         send_to_char("Your life energy is drained!\n\r", victim);
            dam = dice(level,8);  /* nasty spell */
            damage(ch, victim, dam, SPELL_ENERGY_DRAIN); 
      } else {
    if (!IS_SET(ch->M_immune, IMM_DRAIN)) {
       send_to_char("Your spell backfires!\n\r",ch);
               dam = dice(level,8);  /* nasty spell */
               damage(ch, victim, dam, SPELL_ENERGY_DRAIN);           
         } else {
      send_to_char("Your spell fails utterly.\n\r",ch);
    }
       }

     }
  } else {
    damage(ch, victim, 0, SPELL_ENERGY_DRAIN); /* Miss */
  }
}



void spell_fireball(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  dam = dice(level,6);
/*
  this one should be in_world, not in room, so that the message can
  be sent to everyone.
*/

   for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
         if (!in_group(ch,tmp_victim)) {

            if ((GetMaxLevel(tmp_victim)>=LOW_IMMORTAL)&&(!IS_NPC(tmp_victim))) {
               send_to_char("Some puny mortal tries to toast you with a fireball",tmp_victim);
               return;
            } else {
               if ( saves_spell(tmp_victim, SAVING_SPELL) )
                   dam >>= 1;
                   MissileDamage(ch, tmp_victim, dam, SPELL_FIREBALL);
        }
    } else {
            act("You dodge the mass of flame!!\n\r",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
    }
      } else {
    if (tmp_victim->in_room != NOWHERE) {
            if (real_roomp(ch->in_room)->zone == 
      real_roomp(tmp_victim->in_room)->zone) {
                send_to_char("You feel a blast of hot air.\n\r", tmp_victim);
       }
    }
      }
   } 
}


void spell_earthquake(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

   dam =  dice(1,4) + level + 1;

  send_to_char("The earth trembles beneath your feet!\n\r", ch);
  act("$n makes the earth tremble and shiver\n\r",
     FALSE, ch, 0, 0, TO_ROOM);


   for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim))
         if (!in_group(ch,tmp_victim)) {

            if ((GetMaxLevel(tmp_victim)<LOW_IMMORTAL)||(IS_NPC(tmp_victim))) {
                MissileDamage(ch, tmp_victim, dam, SPELL_EARTHQUAKE);
               act("You fall and hurt yourself!!\n\r",
                   FALSE, ch, 0, tmp_victim, TO_VICT);
       }

    } else {
            act("You almost fall and hurt yourself!!\n\r",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
      } else {
         if (real_roomp(ch->in_room)->zone == real_roomp(tmp_victim->in_room)->zone)
            send_to_char("The earth trembles...", tmp_victim);
      }
   } 
}



void spell_dispel_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

   assert(ch && victim);
   assert((level >= 1) && (level<=ABS_MAX_LVL));


    if (IsExtraPlanar(victim)) {
   if (IS_EVIL(ch)) {
      victim = ch;
   } else {
     if (IS_GOOD(victim)) {
        act("Good protects $N.", FALSE, ch, 0, victim, TO_CHAR);
        return;
     }
   }
     if (!saves_spell(victim, SAVING_SPELL) ) {
       act("$n forces $N from this plane.", TRUE, ch, 0, victim, TO_ROOM);
       act("You force $N from this plane.", TRUE, ch, 0, victim, TO_CHAR);
       act("$n forces you from this plane.", TRUE, ch, 0, victim,TO_VICT);
       gain_exp(ch, MIN(GET_EXP(victim)/2, 50000));
       extract_char(victim);
     }
    } else {
   act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
   act("$N laughs at $n.", TRUE,ch, 0, victim, TO_NOTVICT);
   act("You laugh at $n.", TRUE,ch,0,victim,TO_VICT);
    }
}


void spell_call_lightning(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  extern struct weather_data weather_info;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

   dam = dice(MAX(level,15), 6);

  if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {

     if ( saves_spell(victim, SAVING_SPELL) )
     dam >>= 1;
  
   MissileDamage(ch, victim, dam, SPELL_CALL_LIGHTNING);
   }
}



void spell_harm(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  
  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  
  dam = GET_HIT(victim) - dice(1,4);
  
  if (dam < 0)
    dam = 100; /* Kill the suffering bastard */
  else {
    if ( saves_spell(victim, SAVING_SPELL) )
      dam = 0;
  }
  dam = MIN(dam, 100);
  
  damage(ch, victim, dam, SPELL_HARM);
}



/* spells2.c - Not directly offensive spells */

void spell_armor(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

   if (!affected_by_spell(victim, SPELL_ARMOR)) {
     af.type      = SPELL_ARMOR;
        af.duration  = 24;
     af.modifier  = -20;
     af.location  = APPLY_AC;
     af.bitvector = 0;

     affect_to_char(victim, &af);
      send_to_char("You feel someone protecting you.\n\r", victim);
   } else {
     send_to_char("Nothing New seems to happen\n\r", ch);
   }
}

void spell_astral_walk(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int location;
  struct room_data *rp;

  assert(ch && victim);

  location = victim->in_room;
  rp = real_roomp(location);

  if (GetMaxLevel(victim) > MAX_MORT || 
      !rp ||
      IS_SET(rp->room_flags,  PRIVATE) ||
      IS_SET(rp->room_flags,  NO_SUM) ||
      IS_SET(rp->room_flags,  HAVE_TO_WALK) ||
      IS_NPC(victim) ||
      IS_SET(rp->room_flags,  NO_MAGIC))  {

    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (dice(1,8) == 8) {
    send_to_char("You failed.\n\r", ch);
    return;
  } else {
    act("$n opens a door to another dimension and steps through!",FALSE,ch,0,0,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);
  act("You are blinded for a moment as $n appears in a flash of light!",FALSE,ch,0,0,TO_ROOM);
  do_look(ch, "",15);
  }
}

void spell_teleport(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int to_room;
   extern int top_of_world;      /* ref to the top element of world */
   struct room_data *room;

   assert(ch && victim);

        if (victim != ch) {
           if (saves_spell(victim,SAVING_SPELL)) {
              send_to_char("Your spell has no effect.\n\r",ch);
              if (IS_NPC(victim)) {
                 if (!victim->specials.fighting)
                    set_fighting(victim, ch);
              } else {
                 send_to_char("You feel strange, but the effect fades.\n\r",victim);
         }
              return;
           } else {
             ch = victim;  /* the character (target) is now the victim */
           }
   }

   do {
      to_room = number(0, top_of_world);
      room = real_roomp(to_room);
      if (room) {
        if (IS_SET(room->room_flags, PRIVATE))
          room = 0;
      }

   } while (!room);

        act("$n slowly fade out of existence.", FALSE, ch,0,0,TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, to_room);
        act("$n slowly fade in to existence.", FALSE, ch,0,0,TO_ROOM);

   do_look(ch, "", 0);

   if (IS_SET(real_roomp(to_room)->room_flags, DEATH) && 
       GetMaxLevel(ch) < LOW_IMMORTAL) {
            death_cry(ch);
       zero_rent(ch);
            extract_char(ch);
        }
}



void spell_bless(byte level, struct char_data *ch,
       struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  
  assert(ch && (victim || obj));
  assert((level >= 0) && (level <= ABS_MAX_LVL));
  
  if (obj) {
    if ( (5*GET_LEVEL(ch,CLERIC_LEVEL_IND) > GET_OBJ_WEIGHT(obj)) &&
   (GET_POS(ch) != POSITION_FIGHTING) &&
   !IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
      act("$p briefly glows.",FALSE,ch,obj,0,TO_CHAR);
    }
  } else {
    
    if ((GET_POS(victim) != POSITION_FIGHTING) &&
   (!affected_by_spell(victim, SPELL_BLESS))) {
      
      send_to_char("You feel righteous.\n\r", victim);
      af.type      = SPELL_BLESS;
      af.duration  = 6;
      af.modifier  = 1;
      af.location  = APPLY_HITROLL;
      af.bitvector = 0;
      affect_to_char(victim, &af);
      
      af.location = APPLY_SAVING_SPELL;
      af.modifier = -1;                 /* Make better */
      affect_to_char(victim, &af);
    }
  }
}



void spell_blindness(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));


  if (saves_spell(victim, SAVING_SPELL) ||
      affected_by_spell(victim, SPELL_BLINDNESS))
      return;

  act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
  send_to_char("You have been blinded!\n\r", victim);

  af.type      = SPELL_BLINDNESS;
  af.location  = APPLY_HITROLL;
  af.modifier  = -4;  /* Make hitroll worse */
  af.duration  = level / 2;
  af.bitvector = AFF_BLIND;
  affect_to_char(victim, &af);


  af.location = APPLY_AC;
  af.modifier = +20; /* Make AC Worse! */
  affect_to_char(victim, &af);

  if ((!victim->specials.fighting)&&(victim!=ch))
     set_fighting(victim,ch);

}



void spell_clone(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

  assert(ch && (victim || obj));
  assert((level >= 0) && (level <= ABS_MAX_LVL));

   send_to_char("Clone is not ready yet.", ch);

  if (obj) {

   } else {
      /* clone_char(victim); */
   }
}



void spell_control_weather(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   /* Control Weather is not possible here!!! */
   /* Better/Worse can not be transferred     */
}



void spell_create_food(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int foodnum;
  struct obj_data *tmp_obj;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  foodnum = number(1,10);

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  if (foodnum==1) {
    tmp_obj->name = strdup("piece cake");
    tmp_obj->short_description = strdup("A piece of strawberry cake");
    tmp_obj->description = strdup("A scrumptous looking piece of cake lies here.");
  } else {
  tmp_obj->name = strdup("mushroom");
  tmp_obj->short_description = strdup("A Magic Mushroom");
  tmp_obj->description = strdup("A really delicious looking magic mushroom lies here.");
   }

  tmp_obj->obj_flags.type_flag = ITEM_FOOD;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[0] = 5+level;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.decay_time = 100;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_room(tmp_obj,ch->in_room);

  tmp_obj->item_number = -1;

   act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
   act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);
}



void spell_create_water(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int water;

  extern struct weather_data weather_info;
   void name_to_drinkcon(struct obj_data *obj,int type);
   void name_from_drinkcon(struct obj_data *obj);

  assert(ch && obj);

   if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
      if ((obj->obj_flags.value[2] != LIQ_WATER)
           && (obj->obj_flags.value[1] != 0)) {

         name_from_drinkcon(obj);
         obj->obj_flags.value[2] = LIQ_SLIME;
         name_to_drinkcon(obj, LIQ_SLIME);

      } else {

         water = 2*level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

         /* Calculate water it can contain, or water created */
         water = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1], water);

         if (water > 0) {
           obj->obj_flags.value[2] = LIQ_WATER;
            obj->obj_flags.value[1] += water;

            weight_change_object(obj, water);

            name_from_drinkcon(obj);
            name_to_drinkcon(obj, LIQ_WATER);
            act("$p is partially filled.", FALSE, ch,obj,0,TO_CHAR);
         }
      }
   }
}



void spell_cure_blind(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

   if (affected_by_spell(victim, SPELL_BLINDNESS)) {
     affect_from_char(victim, SPELL_BLINDNESS);

     send_to_char("Your vision returns!\n\r", victim);
   }
}



void spell_cure_critic(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  healpoints = dice(3,8)+3;

  if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);
}


void spell_cure_light(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  healpoints = dice(1,8);

  if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);


}





void spell_curse(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim || obj);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (obj) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
    SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

    /* LOWER ATTACK DICE BY -1 */
    if(obj->obj_flags.type_flag == ITEM_WEAPON)
      obj->obj_flags.value[2]--;
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
   } else {
    if ( saves_spell(victim, SAVING_SPELL) ||
         affected_by_spell(victim, SPELL_CURSE))
      return;

    af.type      = SPELL_CURSE;
    af.duration  = 24*7;       /* 7 Days */
    af.modifier  = -1;
    af.location  = APPLY_HITROLL;
    af.bitvector = AFF_CURSE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_PARA;
    af.modifier = 1; /* Make worse */
    affect_to_char(victim, &af);

    act("$n briefly reveal a red aura!", FALSE, victim, 0, 0, TO_ROOM);
    act("You feel very uncomfortable.",FALSE,victim,0,0,TO_CHAR);
    if (IS_NPC(victim) && !victim->specials.fighting)
       set_fighting(victim,ch);
   }
}



void spell_detect_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

   assert(victim);
   assert((level >= 0) && (level <= ABS_MAX_LVL));

   if ( affected_by_spell(victim, SPELL_DETECT_EVIL) )
      return;

  af.type      = SPELL_DETECT_EVIL;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;

  affect_to_char(victim, &af);

  act("$n's eyes briefly glow white", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_detect_invisibility(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if ( affected_by_spell(victim, SPELL_DETECT_INVISIBLE) )
      return;

  af.type      = SPELL_DETECT_INVISIBLE;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVISIBLE;

  affect_to_char(victim, &af);
  act("$n's eyes briefly glow yellow", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_detect_magic(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if ( affected_by_spell(victim, SPELL_DETECT_MAGIC) )
      return;

  af.type      = SPELL_DETECT_MAGIC;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;

  affect_to_char(victim, &af);
  send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_detect_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   assert(ch && (victim || obj));

  if (victim) {
    if (victim == ch)
      if (IS_AFFECTED(victim, AFF_POISON))
        send_to_char("You can sense poison in your blood.\n\r", ch);
      else
        send_to_char("You feel healthy.\n\r", ch);
    else
      if (IS_AFFECTED(victim, AFF_POISON)) {
        act("You sense that $E is poisoned.",FALSE,ch,0,victim,TO_CHAR);
      } else {
        act("You sense that $E is poisoned",FALSE,ch,0,victim,TO_CHAR);
      }
  } else { /* It's an object */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      if (obj->obj_flags.value[3])
        act("Poisonous fumes are revealed.",FALSE, ch, 0, 0, TO_CHAR);
      else
        send_to_char("It looks very delicious.\n\r", ch);
    }
  }
}



void spell_enchant_weapon(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int i;

   assert(ch && obj);
   assert(MAX_OBJ_AFFECT >= 2);

   if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) &&
      !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {

      for (i=0; i < MAX_OBJ_AFFECT; i++)
         if (obj->affected[i].location != APPLY_NONE)
            return;

      SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

      obj->affected[0].location = APPLY_HITROLL;
      obj->affected[0].modifier = 1;
      if (level > 20)
         obj->affected[0].modifier += 1;
      if (level > 40)
            obj->affected[0].modifier += 1;
      if (level > MAX_MORT)
            obj->affected[0].modifier += 1;
      

      obj->affected[1].location = APPLY_DAMROLL;      
      obj->affected[1].modifier = 1;
      if (level > 15)
         obj->affected[1].modifier += 1;
      if (level > 30)
            obj->affected[1].modifier += 1;
      if (level > MAX_MORT)
            obj->affected[1].modifier += 1;

      if (IS_GOOD(ch)) {
         SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
         act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
      } else if (IS_EVIL(ch)) {
                        SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
                        act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
                } else {
                        act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
      }
   }
}


void spell_synostodweomer(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{ 

     int hitp;
     assert(victim);

     hitp = (GET_HIT(ch)/2);

     GET_HIT(victim) += hitp;

        if (GET_HIT(victim) > hit_limit(victim))
            GET_HIT(ch) = hit_limit(ch);

     GET_HIT(ch) -= hitp;
     
     GET_ALIGNMENT(ch) += (hitp/2);
       
        if (GET_ALIGNMENT(ch) > 1000)
            GET_ALIGNMENT(ch) = 1000;
 
     update_pos( victim );
     update_pos( ch );

    send_to_char("You feel like you have been touched by god!!\n\r", victim);
    send_to_char("You give half of your hit points to a needy person!\n\r", ch);

}

void spell_heal(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   assert(victim);

   spell_cure_blind(level, ch, victim, obj);

   GET_HIT(victim) += 100;

   if (GET_HIT(victim) >= hit_limit(victim))
      GET_HIT(victim) = hit_limit(victim)-dice(1,4);

  update_pos( victim );

  send_to_char("A warm feeling fills your body.\n\r", victim);
}

void spell_vampiric_touch(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
    int hitp;

    assert(victim && ch);
    assert ((level >= 1) && (level < ABS_MAX_LVL));

    hitp = MIN(dice(level,2),GET_MAX_HIT(victim));

 if (IS_IMMORTAL(victim)) {
   send_to_char("Drain a gods blood? NEVER!!\n\r", ch);
   send_to_char("Someone just tried to use a vampiric touch on you!!\n\r", ch);
   }

 if (saves_spell(victim, SAVING_SPELL)) 
    hitp >>= 1;

 if (GET_POS(victim) > POSITION_STUNNED) {
    GET_HIT(victim) -= hitp;
    
    if (GET_HIT(victim) <= 0)
    GET_HIT(victim) = 1;

    GET_HIT(ch) += hitp;

    if (GET_HIT(ch) >= hit_limit(ch))
    GET_HIT(ch) = hit_limit(ch);

   SetVictFighting(ch, victim);

   update_pos( victim );
   update_pos( ch );
  
  send_to_char("You drain the blood from your prey!\n\r", ch);
  send_to_char("You feel drained as you lose some blood!\n\r", victim);
  WAIT_STATE(ch, 2*PULSE_VIOLENCE);
  } else {
    send_to_char("The victim has no hit points to drain!\n\r", ch);
    return;
  }
}

void spell_life_leech(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
     int hitp;
     struct char_data *tmp_victim, *temp;

     assert(ch);

     send_to_char("You try to leech everyone in the room.\n\r", ch);

    for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
       temp = tmp_victim->next;
       if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
        if (!in_group(ch,tmp_victim)) {
         if ((GetMaxLevel(tmp_victim)>=LOW_IMMORTAL)&&(!IS_NPC(tmp_victim))) {
           send_to_char("Some puny mortal tried to drink you blood!\n\r", tmp_victim);
           return;
          } else {
            hitp = MIN(dice(level,2),GET_MAX_HIT(tmp_victim));
            GET_HIT(tmp_victim) -= hitp;
            GET_HIT(ch) += hitp;
            send_to_char("You feel your blood pressure drop!!\n\r", tmp_victim);
            SetVictFighting(ch, tmp_victim);
            }
        } else {
            act("You dodge your groupmembers attempt to leech your life!!!\n\r",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
      }
      WAIT_STATE(ch, 2*PULSE_VIOLENCE);
    }
  }
}
void spell_heal_spray(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
       
 struct room_data *rp;
  struct char_data *tch;

  if (real_roomp(ch->in_room) == NULL)  {
    return;
  }

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
     if ((in_group(tch, ch)) && (GET_POS(ch) > POSITION_SLEEPING)) {
         send_to_char("You send out a magnificent heal spray!\n\r", ch);
         GET_HIT(tch) +=100;

         if (GET_HIT(tch) >= hit_limit(tch))
                GET_HIT(tch) = hit_limit(tch)-dice(1,4);
      }
   }
}

void spell_full_heal(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   assert(victim);

   spell_cure_blind(level, ch, victim, obj);

   GET_HIT(victim) += 200;

   if (GET_HIT(victim) >= hit_limit(victim))
      GET_HIT(victim) = hit_limit(victim)-dice(1,8);

  update_pos( victim );

  send_to_char("A hot rush runs through your body.\n\r", victim);
}


void spell_invisibility(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

   assert((ch && obj) || victim);

  if (obj) {
    if ( !IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE) ) {
         act("$p turns invisible.",FALSE,ch,obj,0,TO_CHAR);
         act("$p turns invisible.",TRUE,ch,obj,0,TO_ROOM);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
      }
  } else {              /* Then it is a PC | NPC */
      if (!affected_by_spell(victim, SPELL_INVISIBLE)) {

        act("$n slowly fades out of existence.", TRUE, victim,0,0,TO_ROOM);
     send_to_char("You vanish.\n\r", victim);

       af.type      = SPELL_INVISIBLE;
      af.duration  = 24;
      af.modifier  = -40;
     af.location  = APPLY_AC;
       af.bitvector = AFF_INVISIBLE;
     affect_to_char(victim, &af);
   }
    }
}


void spell_locate_object(byte level, struct char_data *ch,
          struct char_data *victim, char *obj)
{
  struct obj_data *i;
  char name[256];
  char buf[MAX_STRING_LENGTH];
  int j;

   assert(ch);

  strcpy(name, obj);

   j=level>>1;

   for (i = object_list; i && (j>0); i = i->next)
    if (isname(name, i->name)) {
      if(i->carried_by) {
   if (strlen(PERS(i->carried_by, ch))>0) {
          sprintf(buf,"%s carried by %s.\n\r",
            i->short_description,PERS(i->carried_by,ch));
          send_to_char(buf,ch);
   }
      } else if(i->equipped_by) {
   if (strlen(PERS(i->equipped_by, ch))>0) {
          sprintf(buf,"%s equipped by %s.\n\r",
        i->short_description,PERS(i->equipped_by,ch));
          send_to_char(buf,ch);
   }
      } else if (i->in_obj) {
          sprintf(buf,"%s in %s.\n\r",i->short_description,
            i->in_obj->short_description);
          send_to_char(buf,ch);
      } else {
          sprintf(buf,"%s in %s.\n\r",i->short_description,
      (i->in_room == NOWHERE ? "use but uncertain." : real_roomp(i->in_room)->name));
          send_to_char(buf,ch);
         j--;
      }      
    }

  if(j==0)
    send_to_char("You are very confused.\n\r",ch);
  if(j==level>>1)
    send_to_char("No such object.\n\r",ch);
}


void spell_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   struct affected_type af;

   assert(victim || obj);

  if (victim) {
    if (IsImmune(victim, IMM_POISON)) {
      send_to_char("Your spell backfires horribly.\n\r", ch);
      damage(ch,ch,number(1,(GetMaxLevel(ch)*2)),SPELL_POISON);
      return;
    }
    if (IS_NPC(ch)) {
     if (!IS_SET(ch->specials.act, ACT_DEADLY)) {
      if(!ImpSaveSpell(victim, SAVING_PARA, 0))    {
       af.type = SPELL_POISON;
       af.duration = level*2;
       af.modifier = -2;
       af.location = APPLY_STR;
       af.bitvector = AFF_POISON;

       affect_join(victim, &af, FALSE, FALSE);

       send_to_char("You feel very sick.\n\r", victim);
       if (!victim->specials.fighting)
    set_fighting(victim, ch);
     } else {
       return;
     }
    } else {
      if (!ImpSaveSpell(victim, SAVING_PARA, 0)) {
   act("Deadly poison fills your veins.",TRUE, ch, 0, 0, TO_CHAR);
   damage(victim, victim, MAX(100, GET_HIT(victim)*2), SPELL_POISON);
      } else {
   return;
      }
    }
   } else {
      if(!ImpSaveSpell(victim, SAVING_PARA, 0))    {
       af.type = SPELL_POISON;
       af.duration = level*2;
       af.modifier = -2;
       af.location = APPLY_STR;
       af.bitvector = AFF_POISON;

       affect_join(victim, &af, FALSE, FALSE);

       send_to_char("You feel very sick.\n\r", victim);     
      }
    }
    if (!victim->specials.fighting)
        set_fighting(victim,ch);
  } else { /* Object poison */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 1;
    }
  }
}


void spell_protection_from_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

   assert(victim);

  if (IS_EVIL(victim)) {
    act("$N's evilness disallows use of this spell.", FALSE, ch, 0, victim, TO_CHAR);
    return;
   }

  if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) ) {
    af.type      = SPELL_PROTECT_FROM_EVIL;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char(victim, &af);
      send_to_char("You have a righteous feeling!\n\r", victim);
   }
}

void spell_protection_from_good(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
 
        assert(victim);

  if (IS_GOOD(victim)) {
       act("$N is to holy for that spell.", FALSE, ch, 0, victim, TO_CHAR);
       return;
  }
 
  if (!affected_by_spell(victim, SPELL_PROTECT_GOOD) ) {
    af.type      = SPELL_PROTECT_GOOD;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char(victim, &af);
    send_to_char("You have a righteous feeling!\n\r", victim);
  }
}


void spell_remove_curse(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   struct affected_type af;

   assert(ch && (victim || obj));

   if (obj) {

      if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
         act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);
         REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
      }
   } else {      /* Then it is a PC | NPC */
      if (affected_by_spell(victim, SPELL_CURSE) ) {
         act("$n briefly glows red, then blue.",FALSE,victim,0,0,TO_ROOM);
         act("You feel better.",FALSE,victim,0,0,TO_CHAR);
      affect_from_char(victim, SPELL_CURSE);
    }
   }
}


void spell_remove_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

   assert(ch && (victim || obj));

  if (victim) {
    if(affected_by_spell(victim,SPELL_POISON)) {
      affect_from_char(victim,SPELL_POISON);
      act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
         act("$N looks better.",FALSE,ch,0,victim,TO_ROOM);
    }
   } else {
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 0;
         act("The $p steams briefly.",FALSE,ch,obj,0,TO_CHAR);
      }
   }
}



void spell_fireshield(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if (!affected_by_spell(victim, SPELL_FIRESHIELD) ) {

    act("$n is surrounded by a glowing red aura.",TRUE,victim,0,0,TO_ROOM);
      act("You start glowing red.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_FIRESHIELD;
    af.duration  = 3;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_FIRESHIELD;
    affect_to_char(victim, &af);
  }
}

void spell_sanctuary(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if ((!affected_by_spell(victim, SPELL_SANCTUARY)) && 
      (!IS_AFFECTED(victim, AFF_SANCTUARY))) {

    act("$n is surrounded by a white aura.",TRUE,victim,0,0,TO_ROOM);
      act("You start glowing.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_SANCTUARY;
    af.duration  = (level<LOW_IMMORTAL) ? 3 : level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char(victim, &af);
  }
}


void spell_silence(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if (GetMaxLevel(victim)>GetMaxLevel(ch)) {
   send_to_char("Sorry you can't silence a higher level than you.\n\r",ch);
   send_to_char("Some butthead just tried to silence you!\n\r",victim);
   return;
   }

  if ((!affected_by_spell(victim, SPELL_SILENCE)) && 
      (!IS_AFFECTED(victim, AFF_SILENT))) {

    act("$n has been muzzled!",TRUE,victim,0,0,TO_ROOM);
      act("You have been muzzled!",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_SILENCE;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SILENT;
    affect_to_char(victim, &af);
  }
}



void spell_sleep(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

   assert(victim);

  if (IsImmune(victim, IMM_SLEEP)) {
    FailSleep(victim, ch);
    return;
  }
  if (IsResist(victim, IMM_SLEEP)) {
    if (saves_spell(victim, SAVING_SPELL)) {
       FailSleep(victim, ch);
       return;
     }
    if (saves_spell(victim, SAVING_SPELL)) {
       FailSleep(victim, ch);
       return;
     }
  } else if (!IsSusc(victim, IMM_SLEEP)) {
    if (saves_spell(victim, SAVING_SPELL)) {
       FailSleep(victim, ch);
       return;
     }    
  }

    af.type      = SPELL_SLEEP;
    af.duration  = 4+level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    affect_join(victim, &af, FALSE, FALSE);

    if (GET_POS(victim)>POSITION_SLEEPING)    {
      act("You feel very sleepy ..... zzzzzz",FALSE,victim,0,0,TO_CHAR);
      act("$n go to sleep.",TRUE,victim,0,0,TO_ROOM);
       GET_POS(victim)=POSITION_SLEEPING;
    }
}



void spell_strength(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);


  if (!affected_by_spell(victim,SPELL_STRENGTH)) {
     act("You feel stronger.", FALSE, victim,0,0,TO_CHAR);
     act("$n seems stronger!\n\r",
     FALSE, victim, 0, 0, TO_ROOM);
     af.type      = SPELL_STRENGTH;
     af.duration  = 2*level;
     if (IS_NPC(victim))
        if (level >= CREATOR) {
     af.modifier = 25 - GET_STR(victim);
        } else
        af.modifier = number(1,6);
     else {

       if (HasClass(ch, CLASS_WARRIOR)) 
           af.modifier = number(1,8);
       else if (HasClass(ch, CLASS_CLERIC) ||
      HasClass(ch, CLASS_THIEF))
           af.modifier = number(1,6);
       else 
    af.modifier = number(1,4);
     }
     af.location  = APPLY_STR;
     af.bitvector = 0;
     affect_to_char(victim, &af);
   } else {

  act("Nothing seems to happen.", FALSE, ch,0,0,TO_CHAR);

  }
}



void spell_ventriloquate(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   /* Not possible!! No argument! */
}



void spell_word_of_recall(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  extern int top_of_world;
  int location, premove;
  bool found = FALSE;
  struct room_data *rp;

   assert(victim);

  if (IS_NPC(victim))
     return;

  rp = real_roomp(ch->in_room);

  if (IS_SET(rp->room_flags, ARENA)) {
     send_to_char("You can't recall from the arena!\n\r", ch);
     return;
    }

  /*  loc_nr = GET_HOME(ch); */

  if (ch->player.hometown) {
    location = ch->player.hometown;
  } else {
    location = 3001;
  }

  if (!real_roomp(location))    {
    send_to_char("You are completely lost.\n\r", victim);
    return;
  }

   /* a location has been found. */

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, location);
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  do_look(victim, "",15);
  GET_MOVE(victim) -= 100;
  GET_MOVE(victim) = MAX(0,GET_MOVE(victim));
  update_pos(victim);
}


void spell_summon(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tmp;
  int count;
  assert(ch && victim);

  if (victim->in_room == NOWHERE) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (GetMaxLevel(victim) > GET_LEVEL(ch,BestMagicClass(ch))) {
     send_to_char("You failed.\n\r", ch);
     return;
  }

 if (!IS_IMMORTAL(ch)) {
  if (IS_NPC(victim)) {
    send_to_char("You hear a booming voice echo 'due to abuse, that is impossible.'\n\r", ch);
    return;
  }
  if (IS_SET(real_roomp(victim->in_room)->room_flags, NO_SUM)) {
    send_to_char("You cannot penetrate the magical defenses of that area.\n\r", ch);
    return;
  }

  if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
     send_to_char("Sorry, due to problems, you can no longer summon aggressives.\n\r", ch);
      return;
  }
  if (IS_SET(real_roomp(victim->in_room)->room_flags, ARENA)) {
      send_to_char("That person is fighting in the arena. NO SUMMONING!\n\r", ch);
    return;
    }
  }
     RawSummon(victim, ch);
}


void RawSummon( struct char_data *v, struct char_data *c)
{
  sh_int target;
  struct char_data *tmp;
  struct obj_data *o, *n;
  int    j, i;
  

  if (IS_NPC(v) && (!IS_SET(v->specials.act, ACT_POLYSELF)) &&
      (GetMaxLevel(v) > GetMaxLevel(c)+3)) { 
    act("$N struggles, and all of $S items are destroyed!", TRUE, c, 0, v, TO_CHAR);
    /* remove objects from victim */
    for (j = 0; j < MAX_WEAR; j++) {
      if (v->equipment[j]) {
         o = unequip_char(v, j);
    extract_obj(o);
       }
    }
    for (o = v->carrying; o; o = n) {
      n = o->next_content;
      obj_from_char(o);
      extract_obj(o);
    }
    AddHated(v, c);
  } else {
    WAIT_STATE(c, PULSE_VIOLENCE*6);
  }

  act("$n disappears suddenly.",TRUE,v,0,0,TO_ROOM);
  target = c->in_room;
  char_from_room(v);
  char_to_room(v,target);
  
  act("$n arrives suddenly.",TRUE,v,0,0,TO_ROOM);
  act("$n has summoned you!",FALSE,c,0,v,TO_VICT);
  do_look(v,"",15);


  for (tmp = real_roomp(v->in_room)->people; tmp; tmp = tmp->next_in_room) {
    if (IS_NPC(tmp) && !(IS_SET(tmp->specials.act,ACT_POLYSELF)) &&
   ((IS_SET(tmp->specials.act, ACT_AGGRESSIVE) ||
    (IS_SET(tmp->specials.act, ACT_META_AGG))))) {
      act("$n growls at you", 1, tmp, 0, c, TO_VICT);
      act("$n growls at $N", 1, tmp, 0, c, TO_NOTVICT);
      i = number(0,6);
      if (i==0) {
   if (CAN_SEE(tmp, c)) {
      hit(tmp, c, TYPE_UNDEFINED);
   }
      }
    }
  } 
}


void spell_charm_person(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  void add_follower(struct char_data *ch, struct char_data *leader);
  bool circle_follow(struct char_data *ch, struct char_data *victim);
  void stop_follower(struct char_data *ch);

   assert(ch && victim);

   if (victim == ch) {
      send_to_char("You like yourself even better!\n\r", ch);
      return;
   }

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
      if (circle_follow(victim, ch)) {
       send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
       return;
      }

  if (!IsPerson(victim)) {
    send_to_char("Umm,  that's not a person....\n\r",ch);
    return;
  }

  if (GetMaxLevel(victim) > GetMaxLevel(ch)+3) {
    FailCharm(victim, ch);
    return;
  }

      if (IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
          FailCharm(victim,ch);
           return;
      }
      if (IsResist(victim, IMM_CHARM)) {
         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
           return;
    }

         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
           return;
    }
       } else {
          if (!IsSusc(victim, IMM_CHARM)) {
        if (saves_spell(victim, SAVING_PARA)) {
           FailCharm(victim,ch);
      return;
        }
     }
       }

    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type      = SPELL_CHARM_PERSON;

    if (GET_INT(victim))
      af.duration  = 24*18/GET_INT(victim);
    else
      af.duration  = 24*18;

    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?",FALSE,ch,0,victim,TO_VICT);
    }
}

void spell_control_undead(byte level, struct char_data *ch,
          struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  
  void add_follower(struct char_data *ch, struct char_data *leader);
  bool circle_follow(struct char_data *ch, struct char_data *victim);
  void stop_follower(struct char_data *ch);
  
  assert(ch && victim);
  
  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }

   if (!(GET_RACE(victim) = RACE_UNDEAD)) 
     return;


  if (GetMaxLevel(victim) > GetMaxLevel(ch) + 3) {
    FailCharm(victim, ch);
    return;
  }
  
  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch)) {
      send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      return;
    }
      if (IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
          FailCharm(victim,ch);
           return;
      }
      if (IsResist(victim, IMM_CHARM)) {
         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
           return;
    }

         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
           return;
    }
       } else {
          if (!IsSusc(victim, IMM_CHARM)) {
        if (saves_spell(victim, SAVING_PARA)) {
           FailCharm(victim,ch);
      return;
        }
     }
       }
    
    if (victim->master)
      stop_follower(victim);
    
    add_follower(victim, ch);
    
    af.type      = SPELL_CHARM_PERSON;
    
    if (GET_INT(victim))
      af.duration  = 24*18/GET_INT(victim);
    else
      af.duration  = 24*18;
    
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);
    
    act("Isn't $n just such a nice fellow?",FALSE,ch,0,victim,TO_VICT);
  }
}


void spell_charm_monster(byte level, struct char_data *ch,
          struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  
  void add_follower(struct char_data *ch, struct char_data *leader);
  bool circle_follow(struct char_data *ch, struct char_data *victim);
  void stop_follower(struct char_data *ch);
  
  assert(ch && victim);
  
  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }


  if (GetMaxLevel(victim) > GetMaxLevel(ch) + 3) {
    FailCharm(victim, ch);
    return;
  }
  
  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch)) {
      send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      return;
    }
      if (IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
          FailCharm(victim,ch);
           return;
      }
      if (IsResist(victim, IMM_CHARM)) {
         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
           return;
    }

         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
           return;
    }
       } else {
          if (!IsSusc(victim, IMM_CHARM)) {
        if (saves_spell(victim, SAVING_PARA)) {
           FailCharm(victim,ch);
      return;
        }
     }
       }
    
    if (victim->master)
      stop_follower(victim);
    
    add_follower(victim, ch);
    
    af.type      = SPELL_CHARM_PERSON;
    
    if (GET_INT(victim))
      af.duration  = 24*18/GET_INT(victim);
    else
      af.duration  = 24*18;
    
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);
    
    act("Isn't $n just such a nice fellow?",FALSE,ch,0,victim,TO_VICT);
  }
}


void spell_sense_life(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

   assert(victim);

  if (!affected_by_spell(victim, SPELL_SENSE_LIFE)) {
    send_to_char("Your feel your awareness improve.\n\r", ch);

    af.type      = SPELL_SENSE_LIFE;
    af.duration  = 5*level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SENSE_LIFE;
    affect_to_char(victim, &af);
  }

}

/* ***************************************************************************
 *                     Not cast-able spells                                  *
 * ************************************************************************* */


void sprintbit(unsigned long, char *[], char *);

void spell_identify(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  char buf[256], buf2[256];
  int i;
  bool found;
  float av_dam;
  extern int material_types[];
  
  struct time_info_data age(struct char_data *ch);
  
  /* Spell Names */
  extern char *spells[];
  
  /* For Objects */
  extern char *item_types[];
  extern char *extra_bits[];
  extern char *apply_types[];
  extern char *affected_bits[];
  
  
  assert(ch && (obj || victim));
  
  if (obj) {
    send_to_char("You feel informed:\n\r", ch);
    
    sprintf(buf, "Object '%s', Item type: ", obj->name);
    sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
    strcat(buf,buf2); strcat(buf,"\n\r");
    send_to_char(buf, ch);
    
    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit(obj->obj_flags.bitvector,affected_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf, ch);
    }
    
    send_to_char("Item is: ", ch);
    sprintbit( obj->obj_flags.extra_flags,extra_bits,buf);
    strcat(buf,"\n\r");
    send_to_char(buf,ch);

   sprintf(buf, "Item is made out of %s.    ", 
           material_types[obj->obj_flags.material_points]);
   send_to_char(buf, ch);

   send_to_char("Item will be ", ch);
   if ((obj->obj_flags.decay_time == -1) ||
       (obj->obj_flags.decay_time > 800)) 
    send_to_char("around a long time.\n\r", ch);
   else if (obj->obj_flags.decay_time <800) 
    send_to_char("gone one day.\n\r", ch);
   else if (obj->obj_flags.decay_time <400) 
    send_to_char("gone before you know it.\n\r", ch);
   else if (obj->obj_flags.decay_time <200) 
    send_to_char("gone soon.\n\r", ch);
   else if (obj->obj_flags.decay_time <100)
    send_to_char("gone in no time.\n\r", ch);
    
    sprintf(buf,"Volume: %d, Weight: %d, Value: %d, Rent cost: %d  %s\n\r",
       obj->obj_flags.volume, obj->obj_flags.weight, obj->obj_flags.cost, obj->obj_flags.cost_per_day, obj->obj_flags.cost_per_day>LIM_ITEM_COST_MIN?"[RARE]":" ");
    send_to_char(buf, ch);

    
    switch (GET_ITEM_TYPE(obj)) {
      
    case ITEM_SCROLL : 
    case ITEM_POTION :
      sprintf(buf, "Level %d spells of:\n\r",   obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      if (obj->obj_flags.value[1] >= 1) {
   sprinttype(obj->obj_flags.value[1]-1,spells,buf);
   strcat(buf,"\n\r");
   send_to_char(buf, ch);
      }
      if (obj->obj_flags.value[2] >= 1) {
   sprinttype(obj->obj_flags.value[2]-1,spells,buf);
   strcat(buf,"\n\r");
   send_to_char(buf, ch);
      }
      if (obj->obj_flags.value[3] >= 1) {
   sprinttype(obj->obj_flags.value[3]-1,spells,buf);
   strcat(buf,"\n\r");
   send_to_char(buf, ch);
      }
      break;
      
    case ITEM_WAND : 
    case ITEM_STAFF : 
      sprintf(buf, "Has %d charges, with %d charges left.\n\r",
         obj->obj_flags.value[1],
         obj->obj_flags.value[2]);
      send_to_char(buf, ch);
      
      sprintf(buf, "Level %d spell of:\n\r", obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      
      if (obj->obj_flags.value[3] >= 1) {
   sprinttype(obj->obj_flags.value[3]-1,spells,buf);
   strcat(buf,"\n\r");
   send_to_char(buf, ch);
      }
      break;
      
    case ITEM_WEAPON :
      av_dam = obj->obj_flags.value[1]*(obj->obj_flags.value[2]/2.0 + 0.5);
      sprintf(buf, "The weapon does %s\n\r",
              GET_WEAPON_DAMAGE(av_dam));
      send_to_char(buf, ch);
      break;

    case ITEM_FIREWEAPON :
      sprintf(buf, "Bullet number is %d\n\r",
                  obj->obj_flags.value[0]);
      send_to_char(buf,ch);
      sprintf(buf, "Damage Dice is %dD%d\n\r",
              obj->obj_flags.value[1],
              obj->obj_flags.value[2]);
      break;
               
    case ITEM_ARMOR :
      sprintf(buf, "AC-apply is %d\n\r",
         obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      break;
      
    }
    
    found = FALSE;
    
    for (i=0;i<MAX_OBJ_AFFECT;i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
     (obj->affected[i].modifier != 0)) {
   if (!found) {
     send_to_char("Can affect you as :\n\r", ch);
     found = TRUE;
   }
   
   sprinttype(obj->affected[i].location,apply_types,buf2);
   sprintf(buf,"    Affects : %s By %d\n\r", buf2,obj->affected[i].modifier);
   send_to_char(buf, ch);
      }
    }
    
  } else {       /* victim */
    
    if (!IS_NPC(victim)) {
      sprintf(buf,"%d Years,  %d Months,  %d Days,  %d Hours old.\n\r",
         age(victim).year, age(victim).month,
         age(victim).day, age(victim).hours);
      send_to_char(buf,ch);
      
      sprintf(buf,"Height %dcm  Weight %dpounds \n\r",
         GET_HEIGHT(victim), GET_WEIGHT(victim));
      send_to_char(buf,ch);
      
      sprintf(buf,"%s is %s\n\r",GET_NAME(victim),ac_for_score(GET_AC(victim)));
      send_to_char(buf,ch);
      
      /*
   sprintf(buf,"Str %d/%d,  Int %d,  Wis %d,  Dex %d,  Con %d\n\r",
   GET_STR(victim), GET_ADD(victim),
   GET_INT(victim),
   GET_WIS(victim),
   GET_DEX(victim),
   GET_CON(victim) );
   send_to_char(buf,ch);
   */
      
    } else {
      send_to_char("You learn nothing new.\n\r", ch);
    }
  }
  
}


/* ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

void spell_fire_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int dam;
   int hpch;
   struct obj_data *burn;

   assert(victim && ch);
   assert((level >= 1) && (level <= ABS_MAX_LVL)); 

   dam = dice(1,100) + level;

   if ( saves_spell(victim, SAVING_BREATH) )
      dam >>= 1;

   MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);

   /* And now for the damage on inventory */

/*
  DamageStuff(victim, FIRE_DAMAGE);
*/

         for (burn=victim->carrying ; 
        burn && (burn->obj_flags.type_flag!=ITEM_SCROLL) && 
       (burn->obj_flags.type_flag!=ITEM_WAND) &&
       (burn->obj_flags.type_flag!=ITEM_STAFF) &&
       (burn->obj_flags.type_flag!=ITEM_BOAT);
        burn=burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) )  {
            if (burn)  {
               act("$o burns",0,victim,burn,0,TO_CHAR);
               extract_obj(burn);
            }
        }
   }
}


void spell_frost_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int dam;
   int hpch;
   struct obj_data *frozen;

   assert(victim && ch);
   assert((level >= 1) && (level <= ABS_MAX_LVL)); 

   dam = dice(10,100) + level;

   if ( saves_spell(victim, SAVING_BREATH) )
      dam >>= 1;

   MissileDamage(ch, victim, dam, SPELL_FROST_BREATH);

   /* And now for the damage on inventory */

         for (frozen=victim->carrying ; 
             frozen && (frozen->obj_flags.type_flag!=ITEM_DRINKCON) && 
            (frozen->obj_flags.type_flag!=ITEM_ARMOR) &&
       (frozen->obj_flags.type_flag!=ITEM_POTION);
       frozen=frozen->next_content) {

             if (!saves_spell(victim, SAVING_BREATH) ) {
               if (frozen) {
          act("$o shatters.",0,victim,frozen,0,TO_CHAR);
          extract_obj(frozen);
         }
       }
   }
}


void spell_acid_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int dam;
   int hpch;
   int damaged;

   int apply_ac(struct char_data *ch, int eq_pos);
   
   assert(victim && ch);
   assert((level >= 1) && (level <= ABS_MAX_LVL)); 

   dam = dice(1,30) + level;

   if ( saves_spell(victim, SAVING_BREATH) )
      dam >>= 1;

   MissileDamage(ch, victim, dam, SPELL_ACID_BREATH);

}


void spell_gas_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int dam;
   int hpch;

   assert(victim && ch);
   assert((level >= 1) && (level <= ABS_MAX_LVL)); 

   dam = dice(1,100) + level;

   if ( saves_spell(victim, SAVING_BREATH) )
      dam >>= 1;

   MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);


}


void spell_lightning_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int dam;
   int hpch;

   assert(victim && ch);
   assert((level >= 1) && (level <= ABS_MAX_LVL)); 

   dam = dice(1,100)+level;

   if ( saves_spell(victim, SAVING_BREATH) )
      dam >>= 1;

   MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH);


}


#define WOOD_GOLEM        26
#define ROCK_GOLEM        27
#define IRON_GOLEM        28
#define DIAMOND_GOLEM     29
#define WOOD_COMPONENT    26
#define ROCK_COMPONENT    27
#define IRON_COMPONENT    28
#define DIAMOND_COMPONENT 29

void spell_create_golem(byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)  {
   int control, power, nc, modifier = 0, target;
   struct affected_type af;
   struct char_data *golem;

   if (!ch)
      return;

   if (check_peaceful(ch, "The peaceful force protecting this room breaks your concentration!\n\r") || !enforce_verbal(ch) || !perform_gestural(ch)) {
      send_to_char("The spell has failed!\n\r", ch);
      return;
   }
  
   if (power = use_component(ch, find_component(ch, WOOD_COMPONENT))) {
      control = TASK_EASY;
      target = WOOD_GOLEM;
   } else if (power = use_component(ch, find_component(ch, ROCK_COMPONENT))) {
      control = TASK_NORMAL;
      target = ROCK_GOLEM;
   } else if (power = use_component(ch, find_component(ch, IRON_COMPONENT))) {
      control = TASK_DIFFICULT;
      target = IRON_GOLEM;
   } else if (power = use_component(ch, find_component(ch, DIAMOND_COMPONENT))) {
      control = TASK_DANGEROUS;
      target = DIAMOND_GOLEM;
   } else {
      send_to_char("You have nothing to create the golem with!\n\r", ch);
      act("$n looks around stupidly, as if trying to find something.",
           TRUE, ch, 0, golem, TO_ROOM);
      return;
   }

   if (!(golem = read_mobile(target, VIRTUAL))) {
      vlog("Spell 'create golem' unable to load golem [bad!]...");
      send_to_char("Unable to create the golem.  Please report this\n\r.", ch);
      return;
   }

   GET_HIT(golem) = golem->points.max_hit += power;   /* quality of component figures in */
   golem->points.exp = 0;
   
   char_to_room(golem, ch->in_room);
   act("$n arrives in a puff of blue smoke!", TRUE, golem, 0, ch, TO_ROOM);
   
   /* spell requires high INT and WIS  */
   modifier += (GET_WIS(ch) > 17)  +  (GET_WIS(ch) > 16) +  (GET_WIS(ch) > 15)
            + -(GET_WIS(ch) < 14)  + -(GET_WIS(ch) < 13) + -(GET_WIS(ch) < 12)
            +  (GET_INT(ch) > 17)  +  (GET_INT(ch) > 16) +  (GET_INT(ch) > 15)
            + -(GET_INT(ch) < 14)  + -(GET_INT(ch) < 13) + -(GET_INT(ch) < 12);

   if ((nc = num_classes(ch)) == 2) {
      modifier -= 2;                   /* penalty for dual-class */
      GET_HIT(golem) = golem->points.max_hit = (int) (GET_MAX_HIT(golem) * .75);
   }
   else if (nc == 3) {
      modifier -= 3;                /* penalty for triple-class */
      GET_HIT(golem) = golem->points.max_hit = (int) (GET_MAX_HIT(golem) * .55);
   }
            
   if ((control = task_check(ch, control, modifier)) == CRITICAL_FAILURE) {
      act("$n loses control of the magic he has unleashed!", TRUE, ch, 
           0, golem, TO_ROOM);
      act("You lose control of the magic you have unleased!", TRUE, ch, 
           0, golem, TO_CHAR);
      hit(golem, ch, TYPE_UNDEFINED);
   } else {                            /* golem has permanent charm */
      if (golem->master)
         stop_follower(golem);
      add_follower(golem, ch);
      af.type = SPELL_CHARM_PERSON;
      af.duration  = 24*365;
      af.location  = af.modifier  = 0;
      af.bitvector = AFF_CHARM;
      affect_to_char(golem, &af);
      if (control == CRITICAL_SUCCESS) {
         act("$n beams with pride.  $N flexes.", TRUE, ch, 0, golem, TO_ROOM);
         send_to_char("You have created an unusually strong golem!\n\r", ch);
         GET_HIT(golem) = golem->points.max_hit= (int) (GET_MAX_HIT(golem) * 1.5);
      }
      if (!IS_SET(golem->specials.act, ACT_SENTINEL))
         SET_BIT(golem->specials.act, ACT_SENTINEL);
   }
}
