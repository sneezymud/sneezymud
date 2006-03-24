/* ************************************************************************
*  file: magic2.c , Implementation of (new)spells.        Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
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
extern struct room_data *world;
extern struct obj_data  *object_list;
extern struct char_data *character_list;

/* Extern procedures */

void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
void MissileDamage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
bool saves_spell(struct char_data *ch, sh_int spell);
void weight_change_object(struct obj_data *obj, int weight);
char *strdup(char *source);
int dice(int number, int size);
char in_group(struct char_data *ch1, struct char_data *ch2);
int IsExtraPlanar( struct char_data *ch);

/*
  cleric spells
*/

/*
 **   requires the sacrifice of 150k coins, victim loses a con point, and
 **   caster is knocked down to 1 hp, 1 mp, 1 mana, and sits for a LONG
 **   time (if a pc)
 */

void spell_resurrection(byte level, Mob *ch, Mob *victim, Obj *obj) {
  struct char_file_u st;
  struct affected_type af;
  struct obj_data *obj_object, *next_obj;  
  FILE *fl;  
  
  if (!obj) return;
  
  if (IS_CORPSE(obj)) {
     if (obj->char_vnum) {  /* corpse is a npc */
        if (GET_GOLD(ch) < 10000) {
           send_to_char("The gods are not happy with your sacrifice.\n\r",ch);
           return;
        } else
		     GET_GOLD(ch) -= 10000;
      
     victim = read_mobile(obj->char_vnum, VIRTUAL);
	  if (IS_SET(victim->specials.act, ACT_IMMORTAL)) {
	  	  send_to_char("You can't seem to control the magic.\n\r", ch);
		  extract_char(victim);
		  return;
	  }
	  
     if (!IsImmune(victim, IMM_CHARM) || (!IsResist(victim, IMM_CHARM))) {
        char_to_room(victim, ch->in_room);
        GET_GOLD(victim)=0;
        GET_EXP(victim)=0;
        GET_HIT(victim)=1;
        GET_POS(victim)=POSITION_STUNNED;
     } else {
       send_to_char("You just don't have the power to resurrect that corpse.\n\r", ch);
		 extract_char(victim);
       return;
     }
      
      act("With mystic power, $n resurrects a corpse.", TRUE, ch,
     0, 0, TO_ROOM);
      act("$N slowly rises from the ground.", FALSE, ch, 0, victim, TO_ROOM);
      
      /*
   should be charmed and follower ch
   */
      
      if (IsImmune(victim, IMM_CHARM) || IsResist(victim, IMM_CHARM)) {
   act("$N says 'Thank you'", FALSE, ch, 0, victim, TO_ROOM);
      } else {
        af.type      = SPELL_CHARM_PERSON;
        af.duration  = 36;
        af.modifier  = 0;
        af.location  = 0;
        af.bitvector = AFF_CHARM;
   
        affect_to_char(victim, &af);
   
         add_follower(victim, ch);
      }
      
      IS_CARRYING_W(victim) = 0;
      IS_CARRYING_N(victim) = 0;
      
      /*
   take all from corpse, and give to person
   */
      
      for (obj_object=obj->contains; obj_object; obj_object=next_obj) {
   next_obj = obj_object->next_content;
   obj_from_obj(obj_object);
   obj_to_char(obj_object, victim);
      }
      
      /*
   get rid of corpse
   */
      extract_obj(obj);
      
      
    } else {          /* corpse is a pc  */
     send_to_char("You can't resurrect players anymore, sorry./n/r", ch);
    }
  }  
}

void spell_vitalize_mana(byte level, struct char_data *ch,
 struct char_data *victim, struct obj_data *obj)
{
  send_to_char("Vitalize mana has been disabled.\n\r",ch);
}


void spell_cause_light(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(1,8);

  damage(ch, victim, dam, SPELL_CAUSE_LIGHT);

}

void spell_cause_critical(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(3,8) + 3;

  damage(ch, victim, dam, SPELL_CAUSE_CRITICAL);

}

void spell_cause_serious(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(2,8) + 2;

  damage(ch, victim, dam, SPELL_CAUSE_SERIOUS);

}

void spell_cure_serious(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(2,8)+2;

  if ( (dam + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += dam;

  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);

}

void spell_mana(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level,4);
  dam = MAX(dam, level*2);

  if (GET_MANA(ch)+dam > GET_MAX_MANA(ch))
    GET_MANA(ch) = GET_MAX_MANA(ch);
  else
    GET_MANA(ch) += dam;

}

void spell_second_wind(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level,8)+level;

  if ( (dam + GET_MOVE(victim)) > move_limit(victim) )
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += dam;

  send_to_char("You feel less tired\n\r", victim);

}



void spell_flamestrike(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  dam = dice(6,8);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  MissileDamage(ch, victim, dam, SPELL_FLAMESTRIKE);

}



void spell_dispel_good(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
   assert((level >= 1) && (level<=ABS_MAX_LVL));

  if (IsExtraPlanar(victim)) {
   if (IS_GOOD(ch)) {
             victim = ch;
   } else if (IS_EVIL(victim)) {
            act("Evil protects $N.", FALSE, ch, 0, victim, TO_CHAR);
       return;
   }
      
        if (!saves_spell(victim, SAVING_SPELL) ) {
       act("$n forces $N from this plane.",TRUE,ch,0,victim,TO_NOTVICT);
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

void spell_turn(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int diff,i;

  assert(ch && victim);
  assert((level >= 1) && (level<=ABS_MAX_LVL));

  if (IsUndead(victim)) {

    diff = level - GetTotLevel(victim);
    if (diff <= 0) {
      act("You are powerless to affect $N", TRUE, ch, 0, victim, TO_CHAR);
      return;
    } else {
      for (i = 1; i <= diff; i++) {
        if (!saves_spell(victim, SAVING_SPELL) ) {
       act("$n forces $N from this room.",TRUE,ch,0,victim,TO_NOTVICT);
       act("You force $N from this room.", TRUE, ch, 0, victim, TO_CHAR);
       act("$n forces you from this room.", TRUE, ch, 0, victim,TO_VICT);
       do_flee(victim,"",0);
       break;
   }
      }
      if (i < diff) {
   act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
   act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
   act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
      }
    }
  } else {
   act("$n just tried to turn you, what a moron!", TRUE, ch, 0, victim, TO_VICT);
   act("$N thinks $n is really strange.", TRUE, ch, 0, victim, TO_NOTVICT);
   act("Um... $N isn't undead...", TRUE, ch, 0, victim, TO_CHAR);
  }
}

void spell_remove_paralysis(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

  assert(ch && victim);

  if (affected_by_spell(victim,SPELL_PARALYSIS)) {
      affect_from_char(victim,SPELL_PARALYSIS);
      act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
      act("$N looks better.",FALSE,ch,0,victim,TO_ROOM);
  } 

}





void spell_holy_word(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_unholy_word(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_farlook(byte level, struct char_data *ch,
      struct char_data *victim, struct obj_data *obj)
{
   sh_int target;
   struct char_data *tmpv, *temp;
   char buf[MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH];
   
   target = real_roomp(victim->in_room)->number;

   if (GetMaxLevel(victim) > 50) {
     send_to_char("You failed.\n\r", ch);
     return;
   }

   strcpy(buf, "You conjure up a large cloud which shimmers, and\n\r"
   "then ... turns transparent ans shows ... \n\r");
   strcpy(buf1, "$n conjures up a large cloud which shimmers, and\n\r"
   "then ... turns transparent and shows ...\n\r");
   act(buf, FALSE, ch, 0, 0, TO_CHAR);
   act(buf1, FALSE, ch, 0, 0, TO_ROOM);

   sprintf(buf1, "%d look", target);
   for(tmpv = character_list; tmpv; tmpv = temp) {
       temp = tmpv->next;
       if((ch->in_room == tmpv->in_room) && !IS_NPC(tmpv))
          do_at(tmpv, buf1, 0);
   }
} 

void spell_dispel_invisible(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  assert((ch && obj) || victim);

  if (obj) {
   if (IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE) ) {
    act("$p fades into visibility.", FALSE,ch,obj,0,TO_CHAR);
    act("$p fades into visibility.", TRUE, ch,obj,0,TO_ROOM);
    obj->obj_flags.extra_flags -= ITEM_INVISIBLE;
   }
  } else {
   if (affected_by_spell(victim, SPELL_INVISIBLE)) {
     act("$n slowly fades into existance.", TRUE, victim, 0, 0, TO_ROOM);
     send_to_char("You turn visible.\n\r", ch);
     affect_from_char(ch, SPELL_INVISIBLE);
    if (IS_SET(ch->specials.affected_by, AFF_INVISIBLE))
     ch->specials.affected_by -= AFF_INVISIBLE;
   }
  }
} 

void spell_succor(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *o;

  o = read_object(3052,VIRTUAL);
  obj_to_char(o,ch);

  act("$n waves $s hand, and creates $p", TRUE, ch, o, 0, TO_ROOM);
  act("You wave your hand and create $p.", TRUE, ch, o, 0, TO_CHAR);

}

void spell_well_of_knowledge(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *o;

  o = read_object(3050,VIRTUAL);
  obj_to_char(o,ch);

  act("$n waves $s hand, and creates $p", TRUE, ch, o, 0, TO_ROOM);
  act("You wave your hand and create $p.", TRUE, ch, o, 0, TO_CHAR);

}

void spell_detect_charm(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{

}

void spell_true_seeing(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim && ch);

  if (!IS_AFFECTED(victim, AFF_TRUE_SIGHT)) {    
    if (ch != victim) {
       send_to_char("Your eyes glow silver for a moment.\n\r", victim);
       act("$n's eyes take on a silvery hue.\n\r", FALSE, victim, 0, 0, TO_ROOM);
    } else {
       send_to_char("Your eyes glow silver.\n\r", ch);
       act("$n's eyes glow silver.\n\r", FALSE, ch, 0, 0, TO_ROOM);
    }

    af.type      = SPELL_TRUE_SIGHT;
    af.duration  = 2*level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_TRUE_SIGHT;
    affect_to_char(victim, &af);
  } else {
    send_to_char("Nothing seems to happen\n\r", ch);
  }
}




/*
   magic user spells
*/

void spell_track(byte level, struct char_data *ch,
   struct char_data *targ, struct obj_data *obj)
{
  struct char_data *scan;
  char name[256], buf[256], found=FALSE;
  int dist, code;
  struct affected_type af;

  extern struct char_data *character_list;

  if (ch != targ) {
    send_to_char("You feel your awareness grow!\n\r", targ);
  } else {
    send_to_char("You feel your awareness grow!\n\r", ch);
  }

  act("$N's eyes take on an emerald hue for just a moment.", 0,  ch, 0, targ, TO_ROOM);

  if (!obj) {
     af.type      = SPELL_MINOR_TRACK;
     af.duration  = level;
  } else {
     af.type      = SPELL_MAJOR_TRACK;
     af.duration  = level*2;
  }

  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = 0;
  affect_to_char(targ, &af);


}

void spell_poly_self(byte level, struct char_data *ch,
   struct char_data *mob, struct obj_data *obj)
{
 
  char *buf;
 
  void do_snoop(struct char_data *ch, char *argument, int cmd);
 
  /*
   *  Check to make sure that there is no snooping going on.
   */
   if (!ch->desc || ch->desc->snoop.snooping) {
      send_to_char("Godly interference prevents the spell from working.", ch);
      extract_char(mob);
      return;
   }
 
  if (ch->desc->snoop.snoop_by) {  /* force the snooper to stop */
      do_snoop(ch->desc->snoop.snoop_by, 
               GET_NAME(ch->desc->snoop.snoop_by), 0);
  }
 
 
  /*
   * Put mobile in correct room
   */
  
  char_to_room(mob, ch->in_room);
 
  SwitchStuff(ch, mob);
 
  /*
   *  move char to storage
   */

  if ((IS_SET(ch->specials.act, PLR_ANSI)) ||
      (IS_SET(ch->specials.act,PLR_VT100)))
        do_terminal(ch,"none",0);

  act("$n's flesh melts and flows into the shape of $N", 
      TRUE, ch, 0, mob, TO_ROOM);
 
  act("Your flesh melts and flows into the shape of $N", 
      TRUE, ch, 0, mob, TO_CHAR);
 
  char_from_room(ch);
  char_to_room(ch, 3); 
 
  /*
    stop following whoever you are following..
   */
   if (ch->master)
     stop_follower(ch);
 
  
  /*
   *  switch caster into mobile
   */
  
  ch->desc->character = mob;
  ch->desc->original = ch;
  
  mob->desc = ch->desc;
  ch->desc = 0;
 
  SET_BIT(mob->specials.act, ACT_POLYSELF);
  SET_BIT(mob->specials.act, ACT_NICE_THIEF);
  SET_BIT(mob->specials.act, ACT_SENTINEL);
  REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
  REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
 
  GET_MANA(mob) = MIN((GET_MANA(mob)-15), 85);
  WAIT_STATE(mob, PULSE_VIOLENCE*2);
 
 
  /* do some fiddling with the strings */
  buf = (char *)malloc(strlen(GET_NAME(mob)) + strlen(GET_NAME(ch)) + 2);
  sprintf(buf, "%s %s", GET_NAME(ch), GET_NAME(mob));
 
#if TITAN
#else
  /* this code crashes ardent titans */
  if (GET_NAME(mob))
    free(GET_NAME(mob));
#endif
 
  GET_NAME(mob) = buf;
  buf = (char *)malloc(strlen(mob->player.short_descr) 
                       + strlen(GET_NAME(ch)) + 2);
  sprintf(buf, "%s %s", GET_NAME(ch), mob->player.short_descr);
 
#if TITAN
  if (mob->player.short_descr)
    free(mob->player.short_descr);
#endif
  mob->player.short_descr = buf;
 
  buf = (char *)malloc(strlen(mob->player.short_descr)+12);
  sprintf(buf, "%s is here\n\r", mob->player.short_descr);
 
#if TITAN
#else
  if (mob->player.long_descr)
    free(mob->player.long_descr);
#endif
 
  mob->player.long_descr = buf;
 
}


void spell_minor_create(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{

  assert(ch && obj);
  
  act("$n claps $s hands together.", TRUE, ch, 0, 0, TO_ROOM);
  act("You clap your hands together.", TRUE, ch, 0, 0, TO_CHAR);
  act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_ROOM);
  act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_CHAR);

  obj_to_room(obj, ch->in_room);

}


void spell_stone_skin(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch);

  if (!affected_by_spell(ch, SPELL_STONE_SKIN)) {
    act("$n's skin turns grey and granite-like.", TRUE, ch, 0, 0, TO_ROOM);
    act("Your skin turns to a stone-like substance.", TRUE, ch, 0, 0, TO_CHAR);

    af.type      = SPELL_STONE_SKIN;
    af.duration  = level;
    af.modifier  = -40;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(ch, &af);

    /* resistance to piercing weapons */

    af.type      = SPELL_STONE_SKIN;
    af.duration  = level;
    af.modifier  = 32;
    af.location  = APPLY_IMMUNE;
    af.bitvector = 0;
    affect_to_char(ch, &af);
  } 
}



void spell_infravision(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim && ch);

  if (!IS_AFFECTED(victim, AFF_INFRAVISION)) {
    if (ch != victim) {
       send_to_char("Your eyes glow red.\n\r", victim);
       act("$n's eyes glow red.\n\r", FALSE, victim, 0, 0, TO_ROOM);
    } else {
       send_to_char("Your eyes glow red.\n\r", ch);
       act("$n's eyes glow red.\n\r", FALSE, ch, 0, 0, TO_ROOM);
    }

    af.type      = SPELL_INFRAVISION;
    af.duration  = 4*level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_INFRAVISION;
    affect_to_char(victim, &af);

  } 
}

void spell_shield(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim && ch);

  if (!affected_by_spell(victim, SPELL_SHIELD)) {
    act("$N is surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_NOTVICT);
    if (ch != victim) {
       act("$N is surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_CHAR);
       act("You are surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_VICT);
     } else {
       act("You are surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_VICT);
     }

    af.type      = SPELL_SHIELD;
    af.duration  = 8+level;
    af.modifier  = -10;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  } 
}

void spell_weakness(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  float modifier;

  assert(ch && victim);

  if (!affected_by_spell(victim,SPELL_WEAKNESS))
     if (!saves_spell(victim, SAVING_SPELL)) {
        modifier = (77.0 - level)/100.0;
        act("You feel weaker.", FALSE, victim,0,0,TO_VICT);
        act("$n seems weaker.", FALSE, victim, 0, 0, TO_ROOM);

        af.type      = SPELL_WEAKNESS;
        af.duration  = (int) level/2;
        af.modifier  = (int) 0 - (victim->abilities.str * modifier);
        if (victim->abilities.str_add) 
           af.modifier -= 2;
        af.location  = APPLY_STR;
        af.bitvector = 0;

        affect_to_char(victim, &af);
      }
}

void spell_invis_group(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tmpv, *temp;
  struct affected_type af;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

   for ( tmpv = real_roomp(ch->in_room)->people; tmpv; 
   tmpv = tmpv->next_in_room) {
      if ( (ch->in_room == tmpv->in_room) && (ch != tmpv))
         if (in_group(ch,tmpv)) {
             if (!affected_by_spell(tmpv, SPELL_INVISIBLE)) {

          act("$n slowly fades out of existence.", TRUE, tmpv,0,0,TO_ROOM);
          send_to_char("You vanish.\n\r", tmpv);

          af.type      = SPELL_INVISIBLE;
             af.duration  = 24;
             af.modifier  = -40;
          af.location  = APPLY_AC;
          af.bitvector = AFF_INVISIBLE;
          affect_to_char(tmpv, &af);
        }
    }         
    }
}


void spell_acid_blast(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{

  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  dam = dice(level,6);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  MissileDamage(ch, victim, dam, SPELL_ACID_BLAST);

}

void spell_cone_of_cold(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{

  int dam;
  struct char_data *tmpv, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level,3) + level + 1;

  send_to_char("A cone of freezing air fans out before you\n\r", ch);
  act("$n sends a cone of ice shooting from the fingertips!\n\r",
     FALSE, ch, 0, 0, TO_ROOM);

   for ( tmpv = real_roomp(ch->in_room)->people; tmpv; 
   tmpv = tmpv->next_in_room) {
      if ( (ch->in_room == tmpv->in_room) && (ch != tmpv)) {
         if ((GetMaxLevel(tmpv)>LOW_IMMORTAL) && (!IS_NPC(tmpv)))
            return;
         if (!in_group(ch, tmpv)) {
            act("You are chilled to the bone!\n\r",
                 FALSE, ch, 0, tmpv, TO_VICT);
            if ( saves_spell(tmpv, SAVING_SPELL) )
                dam >>= 1;
          MissileDamage(ch, tmpv, dam, SPELL_CONE_OF_COLD);
    } else {
            act("You are able to avoid the cone!\n\r",
                 FALSE, ch, 0, tmpv, TO_VICT);
     }
       }
    }
}

void spell_ice_storm(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmpv, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(3,10);

  send_to_char("You conjure a storm of ice.\n\r", ch);
  act("$n conjures an ice storm!\n\r", FALSE, ch, 0, 0, TO_ROOM);

  for (tmpv = real_roomp(ch->in_room)->people; tmpv; tmpv = tmpv->next_in_room) {
    if ((ch->in_room == tmpv->in_room) && (ch != tmpv)) {
      if (!in_group(ch, tmpv)) {
        act("You are blasted by the storm!\n\r", FALSE, ch, 0, tmpv, TO_VICT);
        if (saves_spell(tmpv, SAVING_SPELL))
          dam >>= 1;

        MissileDamage(ch, tmpv, dam, SPELL_ICE_STORM);
      } else 
        act("You are able to dodge the storm!\n\r", FALSE, ch, 0, tmpv, TO_VICT);
    }
  }
}


void spell_poison_cloud(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{



}

void spell_chain_lightning(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_major_create(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_summon_obj(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_pword_blind(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_pword_kill(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}


void spell_sending(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_meteor_swarm(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  dam = dice(level,10);

  if (saves_spell(victim, SAVING_SPELL))
    dam >>= 1;

  MissileDamage(ch, victim, dam, SPELL_METEOR_SWARM);
}

void spell_disintegrate(byte level, struct char_data *ch,
      struct char_data *victim, struct obj_data *obj)
{
  int dam;

   assert(victim && ch);
   assert((level >= 1) && (level <= ABS_MAX_LVL));

   dam = dice(level,15);
  
  if (OnlyClass(ch, CLASS_MAGIC_USER))
   MissileDamage(ch, victim, dam, SPELL_DISINTEGRATE);

}
  

void spell_Create_Monster(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  struct char_data *mob;
  int rnum,chance;

   /* load in a monster of the correct type, determined by
      level of the spell */

/* really simple to start out with */

   if (level <= 5) {
      rnum = number(1,10)+200;      
      mob = read_mobile(rnum, VIRTUAL);
   } else if (level <= 7) {
      rnum = number(1,10)+210;
      mob = read_mobile(rnum, VIRTUAL);
   } else if (level <= 9) {
      rnum = number(1,10)+220;
      mob = read_mobile(rnum, VIRTUAL);
   } else if (level <= 11) {
      rnum = number(1,10)+230;
      mob = read_mobile(rnum, VIRTUAL);
   } else if (level <= 13) {
      rnum = number(1,10)+240;
      mob = read_mobile(rnum, VIRTUAL);
   } else if (level <= 15) {
      rnum = 251;
      mob = read_mobile(rnum, VIRTUAL);      
   } else {
      rnum = 261;
      mob = read_mobile(rnum, VIRTUAL);
   }


    char_to_room(mob, ch->in_room);

    act("$n waves $s hand, and $N appears!", TRUE, ch, 0, mob, TO_ROOM);
    act("You wave your hand, and $N appears!", TRUE, ch, 0, mob, TO_CHAR);

   /* charm them for a while */
    if (mob->master)
      stop_follower(mob);

    add_follower(mob, ch);

    af.type      = SPELL_CHARM_PERSON;

    if (GET_INT(mob))
      af.duration  = 24*18/GET_INT(mob);
    else
      af.duration  = 24*18;

    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(mob, &af);


/*
  adjust the bits...
*/

/*
 get rid of aggressive, add sentinel
*/

  if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
  }
  if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
    SET_BIT(mob->specials.act, ACT_SENTINEL);
  }


}




/*
   either
*/

void spell_light(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
   
/*
   creates a ball of light in the hands.
*/
  struct obj_data *tmp_obj;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

#if 0
  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = strdup("ball light");
  tmp_obj->short_description = strdup("A ball of light");
  tmp_obj->description = strdup("There is a ball of light on the ground here.");

  tmp_obj->obj_flags.type_flag = ITEM_LIGHT;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[2] = 24+level;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_char(tmp_obj,ch);

  tmp_obj->item_number = -1;
#else
  tmp_obj = read_object(20, VIRTUAL);  /* this is all you have to do */
  if (tmp_obj) {
      tmp_obj->obj_flags.value[2] = 24+level;
      obj_to_char(tmp_obj,ch);
  } else {
    send_to_char("Sorry, I can't create the ball of light\n\r", ch);
    return;
  }


#endif

  act("$n twiddles $s thumbs and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("You twiddle your thumbs and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);

}

void spell_portal(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  int location;
  char buf[30000];
  struct obj_data *tmp_obj;
  struct obj_data *next_tmp_obj;
  struct room_data *rp;

  assert(ch && victim);

  rp = real_roomp(victim->in_room);
 
  if ((!IS_NPC(victim)) && (GetMaxLevel(victim) > MAX_MORT)) {
      send_to_char("Portalling to a God could be hazardous to your health.\n\r", ch);
      return;
  }

  if (GetMaxLevel(victim) >= 50 ||
      !rp ||
      IS_SET(rp->room_flags, NO_SUM) ||
      IS_SET(rp->room_flags, HAVE_TO_WALK) ||
      IS_SET(rp->room_flags, NO_MAGIC))  {

     send_to_char("You can't seem to penetrate the defenses of that area.\n\r", ch);
      return;
   } 
         
  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

    tmp_obj->name = strdup("portal");
    tmp_obj->short_description = strdup("A Magic Portal");
    sprintf(buf,"A portal going to %s is in the room.", real_roomp(victim->in_room)->name);
    tmp_obj->description = strdup(buf);
    tmp_obj->obj_flags.type_flag = ITEM_TRASH;
    tmp_obj->obj_flags.wear_flags = 0;
    tmp_obj->obj_flags.decay_time = 5;
    tmp_obj->obj_flags.weight = 0;
    tmp_obj->obj_flags.cost = 1;
    tmp_obj->obj_flags.cost_per_day = 1;
    tmp_obj->obj_flags.value[0] = victim->in_room;

    tmp_obj->next = object_list;
    object_list = tmp_obj;

    obj_to_room(tmp_obj, ch->in_room);
    tmp_obj->item_number = -1;
    CREATE(next_tmp_obj, struct obj_data, 1);
    clear_object(next_tmp_obj);
    next_tmp_obj->name = strdup("portal");
    next_tmp_obj->short_description = strdup("A Magic Portal");
    sprintf(buf, "A portal going to %s is in the room.",  real_roomp(ch->in_room)->name);
    next_tmp_obj->description = strdup(buf);
    next_tmp_obj->obj_flags.type_flag = ITEM_TRASH;
    next_tmp_obj->obj_flags.wear_flags = 0;
    next_tmp_obj->obj_flags.decay_time = 5;
    next_tmp_obj->obj_flags.weight = 1;
    next_tmp_obj->obj_flags.cost = 1;
    next_tmp_obj->obj_flags.cost_per_day = 0;
    next_tmp_obj->obj_flags.value[0] = ch->in_room;

    next_tmp_obj->next = object_list;
    object_list = next_tmp_obj;

    obj_to_room(next_tmp_obj, victim->in_room);

    next_tmp_obj->item_number = -1;
   act("$p suddenly appears out of a swirling mist.",TRUE,ch,tmp_obj,0,TO_ROOM);
   act("$p suddenly appears out of a swirling mist.",TRUE,ch,tmp_obj,0,TO_CHAR);
}





void spell_fly(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim);
  
  act("You feel lighter than air!", TRUE, ch, 0, victim, TO_VICT);
  if (victim != ch) {
     act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_CHAR);
   } else {
     send_to_char("Your feet rise up off the ground.\n\r", ch);
   }
  act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_NOTVICT);
  
    af.type      = SPELL_FLY;
    af.duration  = GET_LEVEL(ch, BestMagicClass(ch))+3;
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char(victim, &af);
}

void spell_fly_group(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  struct char_data *tch;
  
  assert(ch);

  if (real_roomp(ch->in_room) == NULL)  {
    return;
  }
  
  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
    if (in_group(ch, tch)) {
      act("You feel lighter than air!", TRUE, ch, 0, tch, TO_VICT);
      if (tch != ch) {
   act("$N's feet rise off the ground.", TRUE, ch, 0, tch, TO_CHAR);
      } else {
   send_to_char("Your feet rise up off the ground.", ch);
      }
      act("$N's feet rise off the ground.", TRUE, ch, 0, tch, TO_NOTVICT);
      
      af.type      = SPELL_FLY;
      af.duration  = GET_LEVEL(ch, BestMagicClass(ch))+3;
      af.modifier  = 0;
      af.location  = 0;
      af.bitvector = AFF_FLYING;
      affect_to_char(tch, &af);
    }
  }
}

void spell_refresh(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level,4)+level;
  dam = MAX(dam,20);

  if ( (dam + GET_MOVE(victim)) > move_limit(victim) )
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += dam;

  send_to_char("You feel less tired\n\r", victim);

}


void spell_water_breath(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{

  struct affected_type af;

  assert(ch && victim);
  
  act("You feel fishy!", TRUE, ch, 0, victim, TO_VICT);
  if (victim != ch) {
     act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_CHAR);
   }
  act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_NOTVICT);
  
    af.type      = SPELL_WATER_BREATH;
    af.duration  = 6;
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_WATERBREATH;
    affect_to_char(victim, &af);
  

}



void spell_cont_light(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
/*
   creates a ball of light in the hands.
*/
  struct obj_data *tmp_obj;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));



#if 0
  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = strdup("ball light");
  tmp_obj->short_description = strdup("A bright ball of light");
  tmp_obj->description = strdup("There is a bright ball of light on the ground here.");

  tmp_obj->obj_flags.type_flag = ITEM_LIGHT;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[2] = -1;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 40;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_char(tmp_obj,ch);

  tmp_obj->item_number = -1;
#else
  tmp_obj = read_object(20, VIRTUAL);  /* this is all you have to do */
  if (tmp_obj)
     obj_to_char(tmp_obj,ch);
  else {
    send_to_char("Sorry, I can't create the ball of light\n\r", ch);
    return;
  }    
#endif

  act("$n twiddles $s thumbs and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("You twiddle your thumbs and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);

}

void spell_animate_dead(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *corpse)
{
   struct char_data *mob;
   struct obj_data *obj_object, *sub_object, *next_obj, *i;
   char buf[MAX_STRING_LENGTH];
   int r_num=100; /* virtual # for zombie */
   int k;
/*
 some sort of check for corpse hood
*/
        if (!IS_CORPSE(corpse)) {
     send_to_char("The magic fails abruptly!\n\r",ch);
     return;
   }

        mob = read_mobile(r_num, VIRTUAL);
         char_to_room(mob, ch->in_room);

         act("With mystic power, $n animates a corpse.", TRUE, ch,
         0, 0, TO_ROOM);
         act("$N slowly rises from the ground.", FALSE, ch, 0, mob, TO_ROOM);

/*
  zombie should be charmed and follower ch
*/

         SET_BIT(mob->specials.affected_by, AFF_CHARM);
   GET_EXP(mob) = 100*GET_LEVEL(ch, BestMagicClass(ch));
         add_follower(mob, ch);
         IS_CARRYING_W(mob) = 0;
         IS_CARRYING_N(mob) = 0;

/*
        mob->killer = obj->killer;
*/
/*
  take all from corpse, and give to zombie 
*/

         for (obj_object=corpse->contains; obj_object; obj_object=next_obj) {
          next_obj = obj_object->next_content;
             obj_from_obj(obj_object);
       obj_to_char(obj_object, mob);
        }

/*
   set up descriptions and such
*/ 
    sprintf(buf,"%s is here, slowly animating\n\r",corpse->short_description);
      mob->player.long_descr = strdup(buf);

/*
  set up hitpoints
*/

   mob->points.max_hit = dice((level+1),8);
   mob->points.hit = (int)(mob->points.max_hit/2);

   for (k=MAGE_LEVEL_IND;k<=RANGER_LEVEL_IND; k++)
         mob->player.level[k] = ch->player.level[k];
   mob->player.sex = 0;

   GET_RACE(mob) = RACE_UNDEAD;
   mob->player.class = ch->player.class;

/*
  get rid of corpse
*/
   extract_obj(corpse);

}

void spell_know_alignment(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
   int ap;
   char buf[200], name[100];

   assert(victim && ch);

   if (IS_NPC(victim))
      strcpy(name,victim->player.short_descr);
   else
      strcpy(name,GET_NAME(victim));
   
   ap = GET_ALIGNMENT(victim);
   
   if (ap > 700) 
      sprintf(buf,"%s has an aura as white as the driven snow.\n\r",name);
   else if (ap > 350)
      sprintf(buf, "%s is of excellent moral character.\n\r",name);
   else if (ap > 100)
      sprintf(buf, "%s is often kind and thoughtful.\n\r",name);
   else if (ap > 25)
      sprintf(buf, "%s isn't a bad sort...\n\r",name);
   else if (ap > -25)
      sprintf(buf, "%s doesn't seem to have a firm moral commitment\n\r",name);
   else if (ap > -100)
    sprintf(buf, "%s isn't the worst you've come across\n\r",name);
   else if (ap > -350)
    sprintf(buf, "%s could be a little nicer, but who couldn't?\n\r",name);
   else if (ap > -700)
    sprintf(buf, "%s probably just had a bad childhood\n\r",name);
   else 
     sprintf(buf,"I'd rather just not say anything at all about %s\n\r",name);

   send_to_char(buf,ch);
   
}

void spell_dispel_magic(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
   int yes=0;  

   assert(ch && victim);

/* gets rid of infravision, invisibility, detect, etc */

   if (GetMaxLevel(victim)>GetMaxLevel(ch)) {
      send_to_char("You failed.\n\r", ch);
      return;
   }

   if (GetMaxLevel(victim)<=GetMaxLevel(ch))
      yes = TRUE;
   else 
     yes = FALSE;

    if (affected_by_spell(victim,SPELL_INVISIBLE)) 
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
         affect_from_char(victim,SPELL_INVISIBLE);
         send_to_char("You feel exposed.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_INVISIBLE))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
         affect_from_char(victim,SPELL_DETECT_INVISIBLE);
         send_to_char("You feel less perceptive.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_EVIL))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
         affect_from_char(victim,SPELL_DETECT_EVIL);
         send_to_char("You feel less morally alert.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_MAGIC))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_DETECT_MAGIC);
        send_to_char("You stop noticing the magic in your life.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_SILENCE))
      if (yes || !saves_spell(victim,SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_SILENCE);
        send_to_char("You can speak again.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_SENSE_LIFE))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_SENSE_LIFE);
        send_to_char("You feel less in touch with living things.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_SANCTUARY)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_SANCTUARY);
        send_to_char("You don't feel so invulnerable anymore.\n\r",victim);
        act("The white glow around $n's body fades.",FALSE,victim,0,0,TO_ROOM);
      }
      /*
       *  aggressive Act.
       */
      if ((victim->attackers < 6) && (!victim->specials.fighting) &&
       (IS_NPC(victim))) {
     set_fighting(victim, ch);
   }
    }
    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
   REMOVE_BIT(victim->specials.affected_by, AFF_SANCTUARY);
   send_to_char("You don't feel so invulnerable anymore.\n\r",victim);
   act("The white glow around $n's body fades.",FALSE,victim,0,0,TO_ROOM);      }
      /*
       *  aggressive Act.
       */
      if ((victim->attackers < 6) && (!victim->specials.fighting) &&
     (IS_NPC(victim))) {
   set_fighting(victim, ch);
      }
    }
    if (affected_by_spell(victim,SPELL_PROTECT_FROM_EVIL))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_PROTECT_FROM_EVIL);
        send_to_char("You feel less morally protected.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_INFRAVISION))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_INFRAVISION);
        send_to_char("Your sight grows dimmer.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_SLEEP))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_SLEEP);
        send_to_char("You don't feel so tired.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_CHARM_PERSON))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_CHARM_PERSON);
        send_to_char("You feel less enthused about your master.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_WEAKNESS))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_WEAKNESS);
        send_to_char("You don't feel so weak.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_STRENGTH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_STRENGTH);
        send_to_char("You don't feel so strong.\n\r",victim);
    }
    
    if (affected_by_spell(victim,SPELL_ARMOR))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_ARMOR);
        send_to_char("You don't feel so well protected.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_POISON))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_DETECT_POISON);
        send_to_char("You don't feel so sensitive to fumes.\n\r",victim);
    }
    
    if (affected_by_spell(victim,SPELL_BLESS))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_BLESS);
        send_to_char("You don't feel so blessed.\n\r",victim);
    }

    if (affected_by_spell(victim,SPELL_FLY))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FLY);
        send_to_char("You don't feel lighter than air anymore.\n\r",victim);
    }

    if (affected_by_spell(victim,SPELL_WATER_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_WATER_BREATH);
        send_to_char("You don't feel so fishy anymore.\n\r",victim);
    }

    if (affected_by_spell(victim,SPELL_FIRE_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FIRE_BREATH);
        send_to_char("You don't feel so fiery anymore.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_LIGHTNING_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_LIGHTNING_BREATH);
        send_to_char("You don't feel so electric anymore.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_GAS_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_GAS_BREATH);
        send_to_char("You don't have gas anymore.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_FROST_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FROST_BREATH);
        send_to_char("You don't feel so frosty anymore.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_FIRESHIELD))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FIRESHIELD);
        send_to_char("You don't feel so firey anymore.\n\r",victim);
   act("The red glow around $n's body fades.", TRUE, ch, 0, 0, TO_ROOM);
    }
    if (affected_by_spell(victim,SPELL_FAERIE_FIRE))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FAERIE_FIRE);
        send_to_char("You don't feel so pink anymore.\n\r",victim);
   act("The pink glow around $n's body fades.", TRUE, ch, 0, 0, TO_ROOM);
    }
    if (affected_by_spell(victim,SPELL_MINOR_TRACK))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_MINOR_TRACK);
        send_to_char("You lose the trail.\n\r",victim);
      
      }

    if (affected_by_spell(victim,SPELL_MAJOR_TRACK))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_MAJOR_TRACK);
        send_to_char("You lose the trail.\n\r",victim);
      }

    if (affected_by_spell(victim,SPELL_WEB)) {
        affect_from_char(victim,SPELL_WEB);
        send_to_char("You don't feel so sticky anymore.\n\r",victim);
    }


   if (level == IMPLEMENTOR)  {
    if (affected_by_spell(victim,SPELL_BLINDNESS)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_BLINDNESS);
        send_to_char("Your vision returns.\n\r",victim);
      }
    }
    if (affected_by_spell(victim,SPELL_PARALYSIS)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_PARALYSIS);
        send_to_char("You feel freedom of movement.\n\r",victim);
      }
    }
    if (affected_by_spell(victim,SPELL_POISON)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_POISON);
      }
    }
   }

}



void spell_paralyze(byte level, struct char_data *ch,
          struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  
  assert(victim);
  
  
  if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
    if (IsImmune(victim, IMM_HOLD)) {
      FailPara(victim, ch);
      return;
    }
    if (IsResist(victim, IMM_HOLD)) {
      if (saves_spell(victim, SAVING_PARA)) {
   FailPara(victim, ch);
   return;
      }
      if (saves_spell(victim, SAVING_PARA)) {
   FailPara(victim, ch);
   return;
      }
    } else if (!IsSusc(victim, IMM_HOLD)) {
      if (saves_spell(victim, SAVING_PARA)) {
   FailPara(victim, ch);
   return;
      }    
    }
    
    af.type      = SPELL_PARALYSIS;
    af.duration  = 1;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_PARALYSIS;
    affect_join(victim, &af, FALSE, FALSE);
    
    act("Your limbs freeze in place",FALSE,victim,0,0,TO_CHAR);
    act("$n is paralyzed!",TRUE,victim,0,0,TO_ROOM);
    GET_POS(victim)=POSITION_STUNNED;
    
  } else {
    send_to_char("Someone tries to paralyze you AGAIN!\n\r",victim);
  }
}

void spell_fear(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim && ch);
  
  if (GetMaxLevel(ch) >= GetMaxLevel(victim)-2) {
     if ( !saves_spell(victim, SAVING_SPELL))  {

/* 
        af.type      = SPELL_FEAR;
        af.duration  = 4+level;
        af.modifier  = 0;
        af.location  = APPLY_NONE;
   af.bitvector = 0;

        affect_join(victim, &af, FALSE, FALSE);
*/
   do_flee(victim, "", 0);

      } else {
   send_to_char("You feel afraid, but the effect fades.\n\r",victim);
   return;
      }
   }
}


void spell_prot_align_group(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_calm(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{  
  assert(ch && victim);
/* 
   removes aggressive bit from monsters 
*/
  if (IS_NPC(victim)) {
     if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
       if (!saves_spell(victim, SAVING_PARA)) {
          REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
   } else {
     FailCalm(victim, ch);
   }
     } else {
       send_to_char("You feel calm\n\r", victim);
     }
  } else {
    send_to_char("You feel calm.\n\r", victim);
  }
}

void spell_web(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{  
  struct affected_type af;

  assert(ch && victim);

    af.type = SPELL_WEB;
    af.location = 0;
    af.modifier = level;
    af.duration = level;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_HITROLL;
    af.modifier = -level/5;
    affect_to_char(victim, &af);

    af.location = APPLY_AC;
    af.modifier = 5;
    affect_to_char(victim, &af);


}

void spell_heroes_feast(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct room_data *rp;
  struct char_data *tch;

  if (real_roomp(ch->in_room) == NULL)  {
    return;
  }

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
     if ((in_group(tch, ch)) && (GET_POS(ch) > POSITION_SLEEPING)) {
        send_to_char("You partake of a magnificent feast!\n\r", ch); 

     if (GET_COND(ch,FULL)>=0)
   gain_condition(tch,FULL,24);
     if (GET_COND(ch,THIRST)>=0)
   gain_condition(tch,THIRST,24);
   if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
     GET_HIT(ch)+=1;
   }
     }
  }
}




void spell_conjure_elemental(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  /*
   *   victim, in this case, is the elemental
   *   object could be the sacrificial object
   */

   assert(ch && victim && obj);

   /*
   ** objects:
   **     fire  : red stone
   **     water : pale blue stone
   **     earth : grey stone
   **     air   : clear stone
   */

   act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_ROOM);
   act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_CHAR);
   act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_ROOM);
   act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_CHAR);
   obj_from_char(obj);
   extract_obj(obj);
   char_to_room(victim, ch->in_room);
   act("Out of the smoke, $N emerges", TRUE, ch, 0, victim, TO_NOTVICT);

   /* charm them for a while */
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type      = SPELL_CHARM_PERSON;
    af.duration  = 48;
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;

    affect_to_char(victim, &af);

}

void spell_faerie_fire (byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim);

  if (affected_by_spell(victim, SPELL_FAERIE_FIRE)) {
    send_to_char("Nothing new seems to happen",ch);
    return;
  }

     act("$n points at $N.", TRUE, ch, 0, victim, TO_ROOM);
     act("You point at $N.", TRUE, ch, 0, victim, TO_CHAR);
     act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_ROOM);
     act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_CHAR);

    af.type      = SPELL_FAERIE_FIRE;
    af.duration  = level;
    af.modifier  = 10;
    af.location  = APPLY_ARMOR;
    af.bitvector = 0;

    affect_to_char(victim, &af);

}

void spell_faerie_fog (byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
   struct char_data *tmpv;

  assert(ch);

  act("$n snaps $s fingers, and a cloud of purple smoke billows forth",
      TRUE, ch, 0, 0, TO_ROOM);
  act("You snap your fingers, and a cloud of purple smoke billows forth",
      TRUE, ch, 0, 0, TO_CHAR);


   for ( tmpv = real_roomp(ch->in_room)->people; tmpv; 
   tmpv = tmpv->next_in_room) {
      if ( (ch->in_room == tmpv->in_room) && (ch != tmpv)) {
         if (IS_IMMORTAL(tmpv))
            break;
         if (!in_group(ch, tmpv)) {
      if (IS_AFFECTED(tmpv, AFF_INVISIBLE)) {
            if ( saves_spell(tmpv, SAVING_SPELL) ) {
         REMOVE_BIT(tmpv->specials.affected_by, AFF_INVISIBLE);
         act("$n is briefly revealed, but dissapears again.",
        TRUE, tmpv, 0, 0, TO_ROOM);
         act("You are briefly revealed, but dissapear again.",
        TRUE, tmpv, 0, 0, TO_CHAR);
         SET_BIT(tmpv->specials.affected_by, AFF_INVISIBLE);
       } else {
         REMOVE_BIT(tmpv->specials.affected_by, AFF_INVISIBLE);
         act("$n is revealed!",
        TRUE, tmpv, 0, 0, TO_ROOM);
         act("You are revealed!",
        TRUE, tmpv, 0, 0, TO_CHAR);
       }
      }
    }
       }
    }
}



void spell_cacaodemon(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim && obj);

   act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0, TO_ROOM);
   act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0, TO_CHAR);
   act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0, TO_ROOM);
   act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0, TO_CHAR);
   obj_from_char(obj);
   extract_obj(obj);
   char_to_room(victim, ch->in_room);

   act("With an evil laugh, $N emerges from the smoke", TRUE, ch, 0, victim, TO_NOTVICT);

   /* charm them for a while */
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type      = SPELL_CHARM_PERSON;
    af.duration  = 48;
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;

    affect_to_char(victim, &af);

    if (IS_SET(victim->specials.act, ACT_AGGRESSIVE))
      REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);

    if (!IS_SET(victim->specials.act, ACT_SENTINEL))
      SET_BIT(victim->specials.act, ACT_SENTINEL);

}

/*
 neither
*/

void spell_improved_identify(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}



void spell_geyser(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  struct char_data *tmpv, *temp;

  if (ch->in_room<0)
    return;
   dam =  dice(level,3);

  act("The Geyser erupts in a huge column of steam!\n\r",
     FALSE, ch, 0, 0, TO_ROOM);


   for(tmpv = real_roomp(ch->in_room)->people; tmpv; tmpv = temp) {
      temp = tmpv->next_in_room;
      if ((ch != tmpv) && (ch->in_room == tmpv->in_room)) {
            if ((GetMaxLevel(tmpv)<LOW_IMMORTAL)||(IS_NPC(tmpv))) {
                MissileDamage(ch, tmpv, dam, SPELL_GEYSER);
               act("You are seared by the boiling water!!\n\r",
                   FALSE, ch, 0, tmpv, TO_VICT);
       } else {
               act("You are almost seared by the boiling water!!\n\r",
                 FALSE, ch, 0, tmpv, TO_VICT);
       }
      }
    }
}



void spell_green_slime(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int dam;
   int hpch;

   assert(victim && ch);
   assert((level >= 1) && (level <= ABS_MAX_LVL)); 

   hpch = GET_MAX_HIT(ch);
   if(hpch<10) hpch=10;

   dam = (int)(hpch/10);

   if ( saves_spell(victim, SAVING_BREATH) )
      dam >>= 1;

   send_to_char("You are attacked by green slime!\n\r",victim);

   damage(ch, victim, dam, SPELL_GREEN_SLIME);

}


