/* ************************************************************************
*  file: spells1.c , handling of magic.                   Part of DIKUMUD *
*  Usage : Procedures handling all offensive magic.                       *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "s_list.h"

/* Global data */

extern struct room_data *world;
extern struct char_data *character_list;

/* Extern functions */


char in_group(struct char_data *ch1, struct char_data *ch2);



void cast_burning_hands( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
	switch (type) {
         	case SPELL_TYPE_WAND:
		case SPELL_TYPE_SPELL:
	        case SPELL_TYPE_STAFF:
                case SPELL_TYPE_SCROLL:
			spell_burning_hands(level, ch, 0, 0);
			break;
                default : 
                     vlog("Serious screw-up in burning hands!");
                break;
       }
}


void cast_call_lightning(byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  extern struct weather_data weather_info;

	switch (type) {
		case SPELL_TYPE_SPELL:
			if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
				spell_call_lightning(level, ch, victim, 0);
			} else {
				send_to_char("You fail to call upon the lightning from the sky!\n\r", ch);
			}
			break;
      case SPELL_TYPE_POTION:
			if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
				spell_call_lightning(level, ch, ch, 0);
			}
			break;
      case SPELL_TYPE_SCROLL:
			if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
				if(victim) 
					spell_call_lightning(level, ch, victim, 0);
				else if(!tar_obj) spell_call_lightning(level, ch, ch, 0);
			}
			break;
      case SPELL_TYPE_STAFF:
			if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
				for (victim = real_roomp(ch->in_room)->people ;
                 victim ; victim = victim->next_in_room )
				  if (!in_group(victim,ch))
				   spell_call_lightning(level, ch, victim, 0);
			}
			break;
      default : 
         vlog("Serious screw-up in call lightning!");
         break;
	}
}


void cast_chill_touch( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
			spell_chill_touch(level, ch, victim, 0);
			break;
      default : 
         vlog("Serious screw-up in chill touch!");
         break;
	}
}

void cast_vampiric_touch( byte level, struct char_data *ch, char *arg, int type,  struct char_data *victim, struct obj_data *tar_obj )
{
    switch(type) {
     case SPELL_TYPE_SPELL:
     case SPELL_TYPE_WAND:
         spell_vampiric_touch(level, ch, victim, 0);
         break;
      default :
      vlog("Serious screw-up in vampiric touch!");
      break;
      }
}

void cast_life_leech( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
    switch(type) {
      case SPELL_TYPE_SPELL:
         spell_life_leech(level, ch, 0, 0);
         break;
      default :
      vlog("Serious screw-up in life leech!");
      break;
      }
}


void cast_shocking_grasp( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_shocking_grasp(level, ch, victim, 0);
			break;
      default : 
         vlog("Serious screw-up in shocking grasp!");
         break;
	}
}


void cast_colour_spray( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_colour_spray(level, ch, victim, 0);
         break; 
    case SPELL_TYPE_SCROLL:
         if(victim) 
            spell_colour_spray(level, ch, victim, 0);
         else if (!tar_obj)
				spell_colour_spray(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim) 
            spell_colour_spray(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in colour spray!");
         break;
	}
}


void cast_earthquake( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
			spell_earthquake(level, ch, 0, 0);
	      break;
    default : 
         vlog("Serious screw-up in earthquake!");
         break;
	}
}


void cast_energy_drain( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_energy_drain(level, ch, victim, 0);
			break;
    case SPELL_TYPE_POTION:
         spell_energy_drain(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
       		spell_energy_drain(level, ch, victim, 0);
         else if(!tar_obj)
            spell_energy_drain(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
       	       	spell_energy_drain(level, ch, victim, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (victim = real_roomp(ch->in_room)->people ;
              victim ; victim = victim->next_in_room )
            if (!in_group(ch,victim))
               if (victim != ch)
                  spell_energy_drain(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in energy drain!");
         break;
	}
}


void cast_fireball( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{


  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
			spell_fireball(level, ch, 0, 0);
	      break;
    default : 
         vlog("Serious screw-up in fireball");
         break;
	}



/*
  switch (type) {
    case SPELL_TYPE_SPELL:
		  spell_fireball(level, ch, victim, 0);
	    break;
    case SPELL_TYPE_SCROLL:
         if(victim)
				spell_fireball(level, ch, victim, 0);
         else if(!tar_obj)
            spell_fireball(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
				spell_fireball(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in fireball!");
         break;

	}
*/

}


void cast_harm( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
	switch (type) {
    case SPELL_TYPE_SPELL:
         spell_harm(level, ch, victim, 0);
         break;
    case SPELL_TYPE_POTION:
         spell_harm(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (victim = real_roomp(ch->in_room)->people ;
              victim ; victim = victim->next_in_room )
            if (!in_group(ch,victim))
               spell_harm(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in harm!");
         break;

  }
}


void cast_lightning_bolt( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_lightning_bolt(level, ch, victim, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
				spell_lightning_bolt(level, ch, victim, 0);
         else if(!tar_obj)
            spell_lightning_bolt(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
				spell_lightning_bolt(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in lightning bolt!");
         break;

  }
}

void cast_acid_blast( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_acid_blast(level, ch, victim, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
       		spell_acid_blast(level, ch, victim, 0);
         else
            spell_acid_blast(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
       		spell_acid_blast(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in acid blast!");
         break;

  }
}

void cast_cone_of_cold( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
         spell_cone_of_cold(level, ch, 0, 0);
         break;

    default : 
         vlog("Serious screw-up in cone of cold!");
         break;

  }
}

void cast_ice_storm( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
       	spell_ice_storm(level, ch, 0, 0);
        break;

    default : 
         vlog("Serious screw-up in ice storm");
         break;

  }
}

void cast_meteor_swarm( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_meteor_swarm(level, ch, victim, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
       		spell_meteor_swarm(level, ch, victim, 0);
	 else
            spell_meteor_swarm(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
       		spell_meteor_swarm(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in meteor swarm!");
         break;

  }
}

void cast_disintegrate( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
 
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_disintegrate(level, ch, victim, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
                spell_disintegrate(level, ch, victim, 0);
         else
            spell_disintegrate(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
                spell_disintegrate(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in meteor swarm!");
         break;
 
  }
}

void cast_flamestrike( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_flamestrike(level, ch, victim, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
       	      	spell_flamestrike(level, ch, victim, 0);
         else if(!tar_obj)
                spell_flamestrike(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
               	spell_flamestrike(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in flamestrike!");
         break;

  }
}

void cast_magic_missile( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_magic_missile(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
         if(victim)
       	    	spell_magic_missile(level, ch, victim, 0);
         else if(!tar_obj)
            spell_magic_missile(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
       	       	spell_magic_missile(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in magic missile!");
         break;

  }
}


void cast_cause_light( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
	switch (type) {
    case SPELL_TYPE_SPELL:
         spell_cause_light(level, ch, victim, 0);
         break;
    case SPELL_TYPE_POTION:
         spell_cause_light(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (victim = real_roomp(ch->in_room)->people ;
              victim ; victim = victim->next_in_room )
            if (!in_group(ch,victim))
               spell_cause_light(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in cause light wounds!");
         break;

  }
}

void cast_cause_serious( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
	switch (type) {
    case SPELL_TYPE_SPELL:
         spell_cause_serious(level, ch, victim, 0);
         break;
    case SPELL_TYPE_POTION:
         spell_cause_serious(level, ch, ch, 0);
         break;
       case SPELL_TYPE_WAND:
	 if (tar_obj) return;
	 if (!victim) victim = ch;
	 spell_cause_serious(level, ch, victim, 0);
    case SPELL_TYPE_STAFF:
         for (victim = real_roomp(ch->in_room)->people ;
              victim ; victim = victim->next_in_room )
            if (!in_group(ch,victim))
               spell_cause_serious(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in cause serious wounds!");
         break;

  }
}


void cast_cause_critic(byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
	switch (type) {
    case SPELL_TYPE_SPELL:
         spell_cause_critical(level, ch, victim, 0);
         break;
    case SPELL_TYPE_SCROLL:
	 if (!victim)
	   victim = ch;
	 spell_cause_critical(level,ch,victim,0);
	 break;
    case SPELL_TYPE_WAND:
       	 if (!victim) victim = ch;
	 spell_cause_critical(level,ch,victim,0);
	 break;
    case SPELL_TYPE_POTION:
       	 if (!victim) victim = ch;
         spell_cause_critical(level, ch, victim, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (victim = real_roomp(ch->in_room)->people ;
              victim ; victim = victim->next_in_room )
            if (!in_group(ch,victim))
               spell_cause_critical(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in cause critical!");
         break;

  }
}


void cast_geyser( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
			spell_geyser(level, ch, 0, 0);
	      break;
    default : 
         vlog("Serious screw-up in geyser!");
         break;
	}
}


void cast_green_slime( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_green_slime(level, ch, victim, 0);
         break; 
    case SPELL_TYPE_SCROLL:
         if(victim) 
            spell_green_slime(level, ch, victim, 0);
         else if (!tar_obj)
	    spell_green_slime(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim) 
            spell_green_slime(level, ch, victim, 0);
         break;
    default : 
         vlog("Serious screw-up in green Slime!");
         break;
	}
}


void cast_create_golem( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_create_golem(level, ch, victim, 0);
       break; 
    default : 
         vlog("Create golem called in a non-spell way (not defined!)");
         break;
	}
}

