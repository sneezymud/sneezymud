/* ************************************************************************
*  file: spells2.c , Implementation of magic.             Part of DIKUMUD *
*  Usage : All the non-offensive magic handling routines.                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "poly.h"

/* Global data */


extern struct room_data *world;
extern struct char_data *character_list;
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern struct obj_data  *object_list;
extern int rev_dir[];
extern char *dirs[]; 
extern int movement_loss[];
extern struct weather_data weather_info;
extern struct time_info_data time_info;
extern struct index_data *obj_index;

/* Extern procedures */

void die(struct char_data *ch);
void update_pos( struct char_data *victim );
void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
void clone_char(struct char_data *ch);
void say_spell( struct char_data *ch, int si );
bool saves_spell(struct char_data *ch, sh_int spell);
void add_follower(struct char_data *ch, struct char_data *victim);
char *strdup(char *str);
char in_group(struct char_data *ch1, struct char_data *ch2);
void ChangeWeather( int change);

const struct PolyType PolyList[40] = {
  {"goblin",      4, 201},
  {"parrot",      4, 9001},
  {"frog",        4, 215},
  {"gnoll",       6, 211 },
  {"parrot",      6, 9010 },
  {"lizard",      6, 224},
  {"ogre",        8, 4113},
  {"parrot",      8, 9011},
  {"wolf",        8, 3094},
  {"spider",      9, 227},
  {"beast",       9, 242},
  {"minotaur",    9, 247},
  {"snake",       10, 249},
  {"bull",        10, 1008},
  {"warg",        10, 6100},
  {"sapling",     12, 1421},
  {"ogre-maji",   12, 257},
  {"black",       12, 230},
  {"troll",       14, 4101},
  {"crocodile",   14, 259},
  {"mindflayer",  14, 7202},
  {"giant",       16, 261}, 
  {"bear",        16, 9024},
  {"blue",        16, 233},
  {"enfan",       18, 21001},
  {"lamia",       18, 5201},
  {"drider",      18, 5011},
  {"wyvern",      20, 3415},
  {"mindflayer",  20, 7201},
  {"spider",      20, 20010},
  {"snog",        22, 27008},
  {"roc",         22, 3724},
  {"giant",       24, 9406},
  {"white",       26, 243},
  {"master",      28, 7200},
  {"mulichort",   35, 15830},
  {"beholder",    40, 5200}
};

#define LAST_POLY_MOB 40

void cast_resurrection( byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch(type){
  case SPELL_TYPE_SPELL:
    if (!tar_obj) return;
    spell_resurrection(level, ch, 0, tar_obj);
    break;
  case SPELL_TYPE_STAFF:
    if (!tar_obj) return;
    spell_resurrection(level, ch, 0, tar_obj);
    break;
  default:
    log("Serious problem in 'resurrection'");
    break;
  }

}



void cast_major_track( byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch(type){
  case SPELL_TYPE_SPELL:
    if (!tar_ch) tar_ch = ch;
    spell_track(level, ch, tar_ch, 1);
    break;
  case SPELL_TYPE_POTION:
    spell_track(level, ch, ch, 1);
    break;
  case SPELL_TYPE_WAND:
    if (!tar_ch) tar_ch = ch;
    spell_track(level, ch, tar_ch, 1);
    break;
  case SPELL_TYPE_SCROLL:
    if (!tar_ch) tar_ch = ch;
    spell_track(level, ch, tar_ch, 1);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) 
           spell_track(level, ch, tar_ch, 1);
    }
    break;
  default:
    log("Serious problem in 'track'");
    break;
  }

}

void cast_minor_track( byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch(type){
  case SPELL_TYPE_SPELL:
    if (!tar_ch) tar_ch = ch;
    spell_track(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_track(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (!tar_ch) tar_ch = ch;
    spell_track(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (!tar_ch) tar_ch = ch;
    spell_track(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) 
           spell_track(level, ch, tar_ch, 0);
    }
    break;
  default:
    log("Serious problem in 'track'");
    break;
  }

}

void cast_mana( byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch(type){
  case SPELL_TYPE_POTION:
    spell_mana(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (!tar_ch) tar_ch = ch;
    spell_mana(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch) 
	spell_mana(level, ch, tar_ch, 0);
  default:
    log("Serious problem in 'mana'");
    break;
  }
  
}
void cast_armor( byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(tar_ch, SPELL_ARMOR) ){
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    if (ch != tar_ch)
      act("$N is protected.", FALSE, ch, 0, tar_ch, TO_CHAR);
    
    spell_armor(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_ARMOR) )
      return;
    spell_armor(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    if ( affected_by_spell(tar_ch, SPELL_ARMOR) )
      return;
    spell_armor(level,ch,ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if ( affected_by_spell(tar_ch, SPELL_ARMOR) )
      return;
    spell_armor(level,ch,ch,0);
    break;
    default : 
      log("Serious screw-up in armor!");
    break;
  }
}

void cast_stone_skin( byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(ch, SPELL_STONE_SKIN) ){
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    spell_stone_skin(level,ch,ch,0);
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_STONE_SKIN) )
      return;
    spell_stone_skin(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if ( affected_by_spell(ch, SPELL_STONE_SKIN) )
      return;
    spell_stone_skin(level,ch,ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if ( affected_by_spell(ch, SPELL_STONE_SKIN) )
      return;
    spell_stone_skin(level,ch,ch,0);
    break;
    default : 
      log("Serious screw-up in stone_skin!");
    break;
  }
}

void cast_vitalize_mana(byte level, struct char_data *ch, char *arg, 
int type, struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_vitalize_mana(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_vitalize_mana(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room].people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_vitalize_mana(level,ch,tar_ch,0);
    break;
    default :
      log("Serious screw-up in vitalize mana!");
    break;
    
  }
}


void cast_astral_walk( byte level, struct char_data *ch, char *arg, int type,
		   struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SPELL:
    
    if(!tar_ch) send_to_char("Yes, but who do you wish to walk to?\n",ch);
    else spell_astral_walk(level, ch, tar_ch, 0);
    break;
    
    default : 
      log("Serious screw-up in astral walk!");
    break;
  }
}

void cast_farlook( byte level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch(type) {
   case SPELL_TYPE_SPELL:
   case SPELL_TYPE_SCROLL:
   case SPELL_TYPE_POTION:
   case SPELL_TYPE_WAND:


      if(!tar_ch) send_to_char("Yes, but who do you wish to base your farlook on?\n", ch);
      else spell_farlook(level, ch, tar_ch, 0);
      break;

     default :
      log("Serious fuck up in farlook!");
     break;
    }
}

void cast_portal( byte level, struct char_data *ch, char *arg, int type, struct
char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {

  case SPELL_TYPE_SPELL:
   
 if(!tar_ch) send_to_char("Yes, but who do you wish to portal to?\n",ch);
    else spell_portal(level, ch, tar_ch, 0);
    break;
    
    default : 
      log("Serious screw-up in portal!");
    break;
  }
}

         
void cast_teleport( byte level, struct char_data *ch, char *arg, int type,
		   struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SPELL:
    if (!tar_ch)
      tar_ch = ch;
    spell_teleport(level, ch, tar_ch, 0);
    break;
    
  case SPELL_TYPE_WAND:
    if(!tar_ch) tar_ch = ch;
    spell_teleport(level, ch, tar_ch, 0);
    break;
    
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch) 
	spell_teleport(level, ch, tar_ch, 0);
    break;
    
    default : 
      log("Serious screw-up in teleport!");
    break;
  }
}


void cast_bless( byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj )
{
  struct affected_type af;
  
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) ) {
	send_to_char("Nothing seems to happen.\n\r", ch);
	return;
      }
      spell_bless(level,ch,0,tar_obj);
      
    } else {              /* Then it is a PC | NPC */
      
      if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING)) {
	send_to_char("Nothing seems to happen.\n\r", ch);
	return;
      } 
      spell_bless(level,ch,tar_ch,0);
    }
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_BLESS) ||
	(GET_POS(ch) == POSITION_FIGHTING))
      return;
    spell_bless(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
	return;
      spell_bless(level,ch,0,tar_obj);
      
    } else {              /* Then it is a PC | NPC */
      
      if (!tar_ch) tar_ch = ch;
      
      if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING))
	return;
      spell_bless(level,ch,tar_ch,0);
    }
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
	return;
      spell_bless(level,ch,0,tar_obj);
      
    } else {              /* Then it is a PC | NPC */
      
      if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING))
	return;
      spell_bless(level,ch,tar_ch,0);
    }
    break;
    default : 
      log("Serious screw-up in bless!");
    break;
  }
}

void cast_infravision( byte level, struct char_data *ch, char *arg, int type,
		      struct char_data *tar_ch, struct obj_data *tar_obj )
{
  
  struct affected_type af;
  
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( IS_AFFECTED(tar_ch, AFF_INFRAVISION) ){
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    spell_infravision(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    if ( IS_AFFECTED(ch, AFF_INFRAVISION) )
      return;
    spell_infravision(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    if ( IS_AFFECTED(tar_ch, AFF_INFRAVISION) )
      return;
    spell_infravision(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if ( IS_AFFECTED(tar_ch, AFF_INFRAVISION) )
      return;
    spell_infravision(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	if (!(IS_AFFECTED(tar_ch, AFF_INFRAVISION)))
	  spell_infravision(level,ch,tar_ch,0);
    break;
    default : 
      log("Serious screw-up in infravision!");
    break;
  }
  
}

void cast_true_seeing( byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj )
{
  
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT) ){
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    spell_true_seeing(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    if ( IS_AFFECTED(ch, AFF_TRUE_SIGHT) )
      return;
    spell_true_seeing(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    if ( IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT) )
      return;
    spell_true_seeing(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if ( IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT) )
      return;
    spell_true_seeing(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	if (!(IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)))
	  spell_true_seeing(level,ch,tar_ch,0);
    break;
    default : 
      log("Serious screw-up in true_seeing!");
    break;
  }
  
}

void cast_blindness( byte level, struct char_data *ch, char *arg, int type,
		    struct char_data *tar_ch, struct obj_data *tar_obj )
{
  struct affected_type af;
  
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( IS_AFFECTED(tar_ch, AFF_BLIND) ){
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    spell_blindness(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    if ( IS_AFFECTED(ch, AFF_BLIND) )
      return;
    spell_blindness(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    if ( IS_AFFECTED(tar_ch, AFF_BLIND) )
      return;
    spell_blindness(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    if ( IS_AFFECTED(tar_ch, AFF_BLIND) )
      return;
    spell_blindness(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (!in_group(ch, tar_ch)) 
	if (!(IS_AFFECTED(tar_ch, AFF_BLIND)))
	  spell_blindness(level,ch,tar_ch,0);
    break;
    default : 
      log("Serious screw-up in blindness!");
    break;
  }
}

void cast_light( byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_light(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    spell_light(level,ch,ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    spell_light(level,ch,ch,0);
    break;
    default : 
      log("Serious screw-up in light!");
    break;
  }
}

void cast_cont_light( byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cont_light(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    spell_cont_light(level,ch,ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    spell_cont_light(level,ch,ch,0);
    break;
    default : 
      log("Serious screw-up in continual light!");
    break;
  }
}

void cast_calm( byte level, struct char_data *ch, char *arg, int type,
	       struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_calm(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_calm(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_calm(level,ch,ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people; 
	 tar_ch; tar_ch = tar_ch->next_in_room)
      spell_calm(level,ch,tar_ch,0);
    break;
    default : 
      log("Serious screw-up in continual light!");
    break;
  }
}

void cast_web( byte level, struct char_data *ch, char *arg, int type,
	       struct char_data *tar_ch, struct obj_data *tar_obj )

{
 if (affected_by_spell(tar_ch, SPELL_WEB)) return;
 spell_web(level, ch, tar_ch, 0);
}

void cast_clone( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
	struct char_data *vict;
	char buf[MAX_STRING_LENGTH];

	send_to_char("Not *YET* implemented.", ch);
	return;

  /* clone both char and obj !!*/

/*
  switch (type) {
    case SPELL_TYPE_SPELL:
			if (tar_ch) {	
				sprintf(buf, "You create a duplicate of %s.\n\r", GET_NAME(tar_ch));
				send_to_char(buf, ch);
				sprintf(buf, "%%s creates a duplicate of %s,\n\r", GET_NAME(tar_ch));
				perform(buf, ch, FALSE);

				spell_clone(level,ch,tar_ch,0);
			} else {
				sprintf(buf, "You create a duplicate of %s %s.\n\r",SANA(tar_obj),tar_obj->short_description);
				send_to_char(buf, ch);
				sprintf(buf, "%%s creates a duplicate of %s %s,\n\r",SANA(tar_obj),tar_obj->short_description);
				perform(buf, ch, FALSE);

				spell_clone(level,ch,0,tar_obj);
			};
			break;


    default : 
         log("Serious screw-up in clone!");
         break;
	}
*/
		 /* MISSING REST OF SWITCH -- POTION, SCROLL, WAND */
}


void cast_control_weather(byte level,struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
	char buffer[MAX_STRING_LENGTH];
	extern struct weather_data weather_info;

  switch (type) {
    case SPELL_TYPE_SPELL:

			one_argument(arg,buffer);

			if (str_cmp("better",buffer) && str_cmp("worse",buffer))
			{
				send_to_char("Do you want it to get better or worse?\n\r",ch);
				return;
			}
                        if (!OUTSIDE(ch)) {
			  send_to_char("You need to be outside.\n\r",ch);
                        }

			if(!str_cmp("better",buffer)) {
                          if (weather_info.sky == SKY_CLOUDLESS)
                             return;
                          if (weather_info.sky == SKY_CLOUDY) {
			    send_to_outdoor("The clouds disappear.\n\r");
			    weather_info.sky=SKY_CLOUDLESS;
     			  }
			  if (weather_info.sky == SKY_RAINING) {
                             if ((time_info.month>3)&&(time_info.month < 14)) 
	   		       send_to_outdoor("The rain has stopped.\n\r");
                             else 
                               send_to_outdoor("The snow has stopped. \n\r");
			     weather_info.sky=SKY_CLOUDY;
			  }
			  if (weather_info.sky == SKY_LIGHTNING) {
                             if ((time_info.month>3)&&(time_info.month<14)) 
		                send_to_outdoor("The lightning has gone, but it is still raining.\n\r");
			     else 
			       send_to_outdoor("The blizzard is over, but it is still snowing.\n\r");
		 	     weather_info.sky=SKY_RAINING;
			  }
			  return;
			} else {
                          if (weather_info.sky == SKY_CLOUDLESS) {
			send_to_outdoor("The sky is getting cloudy.\n\r");
			weather_info.sky=SKY_CLOUDY;
                             return;
			   }
                          if (weather_info.sky == SKY_CLOUDY) {
                        if ((time_info.month > 3) && (time_info.month < 14)) 
			   send_to_outdoor("It starts to rain.\n\r");
                        else 
                           send_to_outdoor("It starts to snow. \n\r");
			weather_info.sky=SKY_RAINING;
     			  }
			  if (weather_info.sky == SKY_RAINING) {
                             if ((time_info.month>3)&&(time_info.month < 14))
	      	     send_to_outdoor("You are caught in lightning storm.\n\r");
                          else 
                        send_to_outdoor("You are caught in a blizzard. \n\r");
			   weather_info.sky=SKY_LIGHTNING;
			  }
			  if (weather_info.sky == SKY_LIGHTNING) {
                             return;
			  }

			  return;
                        }
                  break;
		    
      default : 
         log("Serious screw-up in control weather!");
         break;
	}
}



void cast_create_food( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
			act("$n magically creates a mushroom.",FALSE, ch, 0, 0, TO_ROOM);
         spell_create_food(level,ch,0,0);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if(tar_ch) return;
         spell_create_food(level,ch,0,0);
			break;
    default : 
         log("Serious screw-up in create food!");
         break;
	}
}

void cast_create_water( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
				send_to_char("It is unable to hold water.\n\r", ch);
				return;
			}
			spell_create_water(level,ch,0,tar_obj);
			break;
      default : 
         log("Serious screw-up in create water!");
         break;
	}
}




void cast_water_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
       	spell_water_breath(level,ch,tar_ch,0);
       	break;
      case SPELL_TYPE_POTION:
       	spell_water_breath(level,ch,tar_ch,0);
       	break;
      case SPELL_TYPE_WAND:
       	spell_water_breath(level,ch,tar_ch,0);
       	break;

      default : 
         log("Serious screw-up in water breath");
         break;
	}
}



void cast_flying( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{


  switch (type) {
    case SPELL_TYPE_SPELL:
       	spell_fly(level,ch,tar_ch,0);
       	break;
      case SPELL_TYPE_POTION:
       	spell_fly(level,ch,tar_ch,0);
       	break;
      case SPELL_TYPE_WAND:
       	spell_fly(level,ch,tar_ch,0);
       	break;

      default : 
         log("Serious screw-up in fly");
         break;
	}
}


void cast_fly_group( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
       	spell_fly_group(level,ch,0,0);
       	break;
    case SPELL_TYPE_POTION:
       	spell_fly(level,ch,tar_ch,0);
       	break;
      default : 
         log("Serious screw-up in fly");
         break;
  }
}

void cast_heroes_feast( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
       	spell_heroes_feast(level,ch,0,0);
       	break;
      default : 
         log("Serious screw-up in heroes feast");
         break;
  }
}


void cast_synostodweomer( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
   switch(type) {
    case SPELL_TYPE_SPELL:
      spell_synostodweomer(level,ch,tar_ch,0);
      break;
    default :
     log("Serious screw up in synostodweomer!!");
     break;
    }
}
void cast_heal_spray( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_heal_spray(level,ch,0,0);
       	break;
     default : 
         log("Serious screw-up in heal spray!");
         break;
  }
}


void cast_cure_blind( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_cure_blind(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			spell_cure_blind(level,ch,ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cure_blind(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in cure blind!");
         break;
	}
}



void cast_cure_critic( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_cure_critic(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_SCROLL:			
			spell_cure_critic(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			spell_cure_critic(level,ch,ch,0);
			break;
    case SPELL_TYPE_WAND:
			if (!tar_ch) tar_ch = ch;
			spell_cure_critic(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cure_critic(level,ch,tar_ch,0);
         break;
      default : 
         log("Serious screw-up in cure critic!");
         break;

	}
}



void cast_cure_light( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_light(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_SCROLL:
                 if (tar_obj) return;
                 if (!tar_ch) tar_ch = ch;
                        spell_cure_light(level,ch,tar_ch,0);
                        break;
    case SPELL_TYPE_POTION:
			spell_cure_light(level,ch,ch,0);
			break;
    case SPELL_TYPE_WAND:
			if (!tar_ch) tar_ch = ch;
			spell_cure_light(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cure_light(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in cure light!");
         break;
  }
}

void cast_cure_serious( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_serious(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
			spell_cure_serious(level,ch,ch,0);
			break;
    case SPELL_TYPE_WAND:
			if (!tar_ch) tar_ch = ch;
			spell_cure_serious(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cure_serious(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in cure serious!");
         break;
  }
}

void cast_refresh( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_refresh(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
			spell_refresh(level,ch,ch,0);
			break;
    case SPELL_TYPE_WAND:
			if (!tar_ch) tar_ch = ch;
			spell_refresh(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_refresh(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in refresh!");
         break;
  }
}

void cast_second_wind( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_second_wind(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) return;
      if (!tar_ch) tar_ch = ch;
       spell_second_wind(level,ch,tar_ch,0);
       break;
    case SPELL_TYPE_POTION:
			spell_second_wind(level,ch,ch,0);
			break;
    case SPELL_TYPE_WAND:

    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_second_wind(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in second_wind!");
         break;
  }
}

void cast_shield( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_shield(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
			spell_shield(level,ch,ch,0);
			break;
    case SPELL_TYPE_WAND:
			if (!tar_ch) tar_ch = ch;
			spell_shield(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_shield(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in shield!");
         break;
  }

}


void cast_curse( byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj )
{
  char buf[255];

  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj)   /* It is an object */ 
      spell_curse(level,ch,0,tar_obj);
    else {              /* Then it is a PC | NPC */
      spell_curse(level,ch,tar_ch,0);
    }
    break;
  case SPELL_TYPE_POTION:
    spell_curse(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)   /* It is an object */ 
      spell_curse(level,ch,0,tar_obj);
    else {              /* Then it is a PC | NPC */
      if (!tar_ch) tar_ch = ch;
      spell_curse(level,ch,tar_ch,0);
    }
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)   /* It is an object */ 
      spell_curse(level,ch,0,tar_obj);
    else {              /* Then it is a PC | NPC */
      if (!tar_ch) tar_ch = ch;
      spell_curse(level,ch,tar_ch,0);
    }
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch) 
	spell_curse(level,ch,tar_ch,0);
    break;
    default : 
      sprintf(buf,"Serious screw up in curse! Char = %s.",ch->player.name);
      log(buf);
    break;
  }
}

void cast_dispel_invisible(byte level, struct char_data *ch, char *arg, int
  type, struct char_data *tar_ch, struct obj_data *tar_obj )
{
    char buf[255];

    switch(type) {
    case SPELL_TYPE_SPELL:
     if (tar_obj)
       spell_dispel_invisible(level,ch,0,tar_obj);
     else {
       spell_dispel_invisible(level,ch,tar_ch,0);
     }
     break;
     case SPELL_TYPE_POTION:
       spell_dispel_invisible(level,ch,ch,0);
       break;
     default :
       sprintf(buf,"Serious fuck up in dispel invisible!\n\r", ch);
       log(buf);
     break;
     }
}
void cast_detect_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if ( affected_by_spell(tar_ch, SPELL_DETECT_EVIL) ){
				send_to_char("Nothing seems to happen.\n\r", tar_ch);
				return;
			}
			spell_detect_evil(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			if ( affected_by_spell(ch, SPELL_DETECT_EVIL) )
				return;
			spell_detect_evil(level,ch,ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               if(!(IS_AFFECTED(tar_ch, AFF_DETECT_EVIL)))
                  spell_detect_evil(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in detect evil!");
         break;
	}
}



void cast_detect_invisibility( byte level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
                if (IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE)) {
       			  send_to_char("Nothing seems to happen.\n\r", tar_ch);
			  return;
			}
			spell_detect_invisibility(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			if ( IS_AFFECTED(ch, AFF_DETECT_INVISIBLE) )
				return;
			spell_detect_invisibility(level,ch,ch,0);
			break;
    case SPELL_TYPE_SCROLL:
       if (tar_obj) return;
       if (!tar_ch) tar_ch = ch;
        if ( IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE) )
                return;
            spell_detect_invisibility(level,ch,tar_ch,0);
            break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (in_group(ch,tar_ch)) 
               if (!(IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE)))
                  spell_detect_invisibility(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in detect invisibility!");
         break;
	}
}



void cast_detect_magic( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if ( affected_by_spell(tar_ch, SPELL_DETECT_MAGIC) ){
				send_to_char("Nothing seems to happen.\n\r", tar_ch);
				return;
			}
			spell_detect_magic(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			if ( affected_by_spell(ch, SPELL_DETECT_MAGIC) )
				return;
			spell_detect_magic(level,ch,ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               if (!(IS_AFFECTED(tar_ch, SPELL_DETECT_MAGIC)))
                  spell_detect_magic(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in detect magic!");
         break;
	}
}



void cast_detect_poison( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_detect_poison(level, ch, tar_ch,tar_obj);
			break;
    case SPELL_TYPE_POTION:
			spell_detect_poison(level, ch, ch,0);
			break;
    case SPELL_TYPE_SCROLL:
         if (tar_obj) {
				spell_detect_poison(level, ch, 0, tar_obj);
            return;
         }
         if (!tar_ch) tar_ch = ch;
			spell_detect_poison(level, ch, tar_ch, 0);
			break;
    default : 
         log("Serious screw-up in detect poison!");
         break;
	}
}



void cast_dispel_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_dispel_evil(level, ch, tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			spell_dispel_evil(level,ch,ch,0);
			break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) return;
      if (!tar_ch) tar_ch = ch;
			spell_dispel_evil(level, ch, tar_ch,0);
			break;
    case SPELL_TYPE_WAND:
      if (tar_obj) return;
			spell_dispel_evil(level, ch, tar_ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch)) 
              spell_dispel_evil(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in dispel evil!");
         break;
	}
}

void cast_dispel_good( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_dispel_good(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
			spell_dispel_good(level,ch,ch,0);
			break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) return;
      if (!tar_ch) tar_ch = ch;
			spell_dispel_good(level, ch, tar_ch,0);
			break;
    case SPELL_TYPE_WAND:
      if (tar_obj) return;
			spell_dispel_good(level, ch, tar_ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch)) 
              spell_dispel_good(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in dispel good!");
         break;
	}
}

void cast_faerie_fire( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_faerie_fire(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
			spell_faerie_fire(level,ch,ch,0);
			break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) return;
      if (!tar_ch) tar_ch = ch;
			spell_faerie_fire(level, ch, tar_ch,0);
			break;
    case SPELL_TYPE_WAND:
      if (tar_obj) return;
			spell_faerie_fire(level, ch, tar_ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch)) 
              spell_faerie_fire(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in dispel good!");
         break;
	}
}



void cast_enchant_weapon( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_enchant_weapon(level, ch, 0,tar_obj);
			break;

    case SPELL_TYPE_SCROLL:
			if(!tar_obj) return;
			spell_enchant_weapon(level, ch, 0,tar_obj);
			break;
    default : 
      log("Serious screw-up in enchant weapon!");
      break;
	}
}


void cast_enchant_armor( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
/*			spell_enchant_armor(level, ch, 0,tar_obj);
			break;
*/
    case SPELL_TYPE_SCROLL:
/*			if(!tar_obj) return;
			spell_enchant_armor(level, ch, 0,tar_obj);
			break;
*/
    default : 
      log("Serious screw-up in enchant armor!");
      break;
	}
}


void cast_heal( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			act("$n heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
			act("You heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
			spell_heal(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
         spell_heal(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if (!tar_ch) tar_ch=ch;
         spell_heal(level,ch,tar_ch,0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
              spell_heal(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in heal!");
         break;
	}
}

void cast_full_heal( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			act("$n heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
			act("You heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
			spell_full_heal(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
         spell_full_heal(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
              spell_full_heal(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in full heal!");
         break;
	}
}


void cast_invisibility( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if (tar_obj) {
				if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE) )
					send_to_char("Nothing new seems to happen.\n\r", ch);
				else
					spell_invisibility(level, ch, 0, tar_obj);
			} else { /* tar_ch */
				if ( IS_AFFECTED(tar_ch, AFF_INVISIBLE) )
					send_to_char("Nothing new seems to happen.\n\r", ch);
				else
					spell_invisibility(level, ch, tar_ch, 0);
			}
			break;
    case SPELL_TYPE_POTION:
         if (!IS_AFFECTED(ch, AFF_INVISIBLE) )
            spell_invisibility(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
			if (tar_obj) {
				if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
					spell_invisibility(level, ch, 0, tar_obj);
			} else { /* tar_ch */
            if (!tar_ch) tar_ch = ch;

				if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
					spell_invisibility(level, ch, tar_ch, 0);
			}
			break;
    case SPELL_TYPE_WAND:
			if (tar_obj) {
				if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
					spell_invisibility(level, ch, 0, tar_obj);
			} else { /* tar_ch */
				if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
					spell_invisibility(level, ch, tar_ch, 0);
			}
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
                  spell_invisibility(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in invisibility!");
         break;
	}
}




void cast_locate_object( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_locate_object(level, ch, 0, tar_obj);
			break;
      default : 
         log("Serious screw-up in locate object!");
         break;
	}
}


void cast_poison( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
			spell_poison(level, ch, tar_ch, tar_obj);
			break;
    case SPELL_TYPE_POTION:
			spell_poison(level, ch, ch, 0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_poison(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in poison!");
         break;
	}
}


void cast_protection_from_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_protection_from_evil(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
         spell_protection_from_evil(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if(!tar_ch) tar_ch = ch;
			spell_protection_from_evil(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_protection_from_evil(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in protection from evil!");
         break;
	}
}

void cast_protection_from_good( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
                        spell_protection_from_good(level, ch, tar_ch, 0);
                        break;
    case SPELL_TYPE_POTION:
         spell_protection_from_good(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if(!tar_ch) tar_ch = ch;
                        spell_protection_from_good(level, ch, tar_ch, 0);
                        break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_protection_from_good(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in protection from good!");
         break;
        }
}

void cast_remove_curse( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_remove_curse(level, ch, tar_ch, tar_obj);
			break;
    case SPELL_TYPE_POTION:
         spell_remove_curse(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) {
				spell_remove_curse(level, ch, 0, tar_obj);
 				return;
			}
         if(!tar_ch) tar_ch = ch;
			spell_remove_curse(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_remove_curse(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in remove curse!");
         break;
	}
}



void cast_remove_poison( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_remove_poison(level, ch, tar_ch, tar_obj);
			break;
    case SPELL_TYPE_POTION:
         spell_remove_poison(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_remove_poison(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in remove poison!");
         break;
	}
}


void cast_remove_paralysis( byte level, struct char_data *ch, char *arg, int type,  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
       	spell_remove_paralysis(level, ch, tar_ch, tar_obj);
       	break;
    case SPELL_TYPE_POTION:
         spell_remove_paralysis(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
	if (!tar_ch) tar_ch = ch;
         spell_remove_paralysis(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_remove_paralysis(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in remove paralysis!");
         break;
	}
}




void cast_sanctuary( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_sanctuary(level, ch, tar_ch, 0);
			break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
         spell_sanctuary(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj)
 				return;
         if(!tar_ch) tar_ch = ch;
			spell_sanctuary(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_sanctuary(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in sanctuary!");
         break;
	}
}


void cast_silence( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_silence(level, ch, tar_ch, 0);
			break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
         spell_silence(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj)
 				return;
         if(!tar_ch) tar_ch = ch;
			spell_silence(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_silence(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in silence!");
         break;
	}
}



void cast_fireshield( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_fireshield(level, ch, tar_ch, 0);
			break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
         spell_fireshield(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj)
 				return;
         if(!tar_ch) tar_ch = ch;
			spell_fireshield(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_fireshield(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in fireshield!");
         break;
	}
}


void cast_sleep( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_sleep(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
			spell_sleep(level, ch, ch, 0);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_sleep(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(tar_obj) return;
         spell_sleep(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_sleep(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in sleep!");
         break;
	}
}


void cast_strength( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
			spell_strength(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
			spell_strength(level, ch, ch, 0);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_strength(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_strength(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in strength!");
         break;
	}
}


void cast_ventriloquate( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
	struct char_data *tmp_ch;
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];

	if (type != SPELL_TYPE_SPELL) {
		log("Attempt to ventriloquate by non-cast-spell.");
		return;
	}
	for(; *arg && (*arg == ' '); arg++);
	if (tar_obj) {
		sprintf(buf1, "The %s says '%s'\n\r", fname(tar_obj->name), arg);
		sprintf(buf2, "Someone makes it sound like the %s says '%s'.\n\r",
		  fname(tar_obj->name), arg);
	}	else {
		sprintf(buf1, "%s says '%s'\n\r", GET_NAME(tar_ch), arg);
		sprintf(buf2, "Someone makes it sound like %s says '%s'\n\r",
		  GET_NAME(tar_ch), arg);
	}

	sprintf(buf3, "Someone says, '%s'\n\r", arg);

	for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch;
	  tmp_ch = tmp_ch->next_in_room) {

		if ((tmp_ch != ch) && (tmp_ch != tar_ch)) {
			if ( saves_spell(tmp_ch, SAVING_SPELL) )
				send_to_char(buf2, tmp_ch);
			else
				send_to_char(buf1, tmp_ch);
		} else {
			if (tmp_ch == tar_ch)
				send_to_char(buf3, tar_ch);
		}
	}
}
     
void cast_word_of_recall( byte level, struct char_data *ch, char *arg,int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_word_of_recall(level, ch, ch, 0);
			break;
    case SPELL_TYPE_POTION:
			spell_word_of_recall(level, ch, ch, 0);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_word_of_recall(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(tar_obj) return;
         spell_word_of_recall(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_word_of_recall(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in word of recall!");
         break;
	}
}



void cast_summon( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {

    case SPELL_TYPE_SPELL:
			spell_summon(level, ch, tar_ch, 0);
			break;
      default : 
         log("Serious screw-up in summon!");
         break;
	}
}



void cast_charm_person( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
		case SPELL_TYPE_SPELL:
			spell_charm_person(level, ch, tar_ch, 0);
			break;
      case SPELL_TYPE_SCROLL:
         if(!tar_ch) return;
         spell_charm_person(level, ch, tar_ch, 0);
         break;
      case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch)) 
                  spell_charm_person(level,ch,tar_ch,0);
         break;
      default : 
         log("Serious screw-up in charm person!");
         break;
	}
}

void cast_charm_monster( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
      case SPELL_TYPE_SPELL:
         spell_charm_monster(level, ch, tar_ch, 0);
       	 break;
      case SPELL_TYPE_SCROLL:
         if(!tar_ch) return;
         spell_charm_monster(level, ch, tar_ch, 0);
         break;
      case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch)) 
                  spell_charm_monster(level,ch,tar_ch,0);
         break;
      default : 
         log("Serious screw-up in charm monster!");
         break;
	}
}


void cast_control_undead( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
      case SPELL_TYPE_SPELL:
         spell_control_undead(level, ch, tar_ch, 0);
       	 break;
      case SPELL_TYPE_SCROLL:
         if(!tar_ch) return;
         spell_control_undead(level, ch, tar_ch, 0);
         break;
      case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch)) 
                  spell_control_undead(level,ch,tar_ch,0);
         break;
      default : 
         log("Serious screw-up in control undead!");
         break;
	}
}

void cast_sense_life( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
		case SPELL_TYPE_SPELL:
			spell_sense_life(level, ch, ch, 0);
			break;
      case SPELL_TYPE_SCROLL:
       if (tar_obj) return;
        if (!tar_ch) tar_ch = ch;
           spell_sense_life(level, ch, tar_ch, 0);
      case SPELL_TYPE_POTION:
         spell_sense_life(level, ch, ch, 0);
         break;
      case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_sense_life(level,ch,tar_ch,0);
         break;
      default : 
         log("Serious screw-up in sense life!");
         break;
	}
}


void cast_identify( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
		case SPELL_TYPE_SCROLL:
			spell_identify(level, ch, tar_ch, tar_obj);
			break;
		default : 
			log("Serious screw-up in identify!");
			break;
	}
}

#define MAX_BREATHS 3
struct pbreath {
  int	vnum, spell[MAX_BREATHS];
} breath_potions[] = {
  { 3970, {201, 0} },
  { 3971, {202, 0} },
  { 3972, {203, 0} },
  { 3973, {204, 0} },
  { 3974, {205, 0} },
  { 0 },
};

void cast_dragon_breath(byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *potion )
{
  struct pbreath	*scan;
  int	i;
  struct affected_type af;

  for (scan=breath_potions;
       scan->vnum && scan->vnum != obj_index[potion->item_number].virtual;
       scan++)
    ;
  if (scan->vnum==0) {
    char	buf[MAX_STRING_LENGTH];
    send_to_char("Hey, this potion isn't in my list!\n\r", ch);
    sprintf(buf,"unlisted breath potion %s %d", potion->short_description,
	    obj_index[potion->item_number].virtual);
    log(buf);
    return;
  }

  for (i=0; i<MAX_BREATHS && scan->spell[i]; i++) {
    if (!affected_by_spell(ch, scan->spell[i])) {
      af.type = scan->spell[i];
      af.duration = 1+dice(1,2);
      if (GET_CON(ch) < 4) {
	send_to_char("You are too weak to stomach the potion and spew it all over the floor.\n\r", ch);
	act("$n gags and pukes glowing goop all over the floor.",
	    FALSE, ch, 0,ch, TO_NOTVICT);
	break;
      }
      if (level > MIN(GET_CON(ch)-1, GetMaxLevel(ch)) ) {
	send_to_char("!GACK! You are too weak to handle the full power of the potion.\n\r", ch);
	act("$n gags and flops around on the floor a bit.",
	    FALSE, ch, 0,ch, TO_NOTVICT);
	level = MIN(GET_CON(ch)-1, GetMaxLevel(ch));
      }
      af.modifier = -level;
      af.location = APPLY_CON;
      af.bitvector = 0;
      affect_to_char(ch, &af);
      send_to_char("You feel powerful forces build within your stomach...\n\r", ch);
    }
  }
}

void cast_fire_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_fire_breath(level, ch, tar_ch, 0);
			break;   /* It's a spell.. But people can'c cast it! */
      default : 
         log("Serious screw-up in firebreath!");
         break;
	}
}

void cast_frost_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_frost_breath(level, ch, tar_ch, 0);
			break;   /* It's a spell.. But people can'c cast it! */
      default : 
         log("Serious screw-up in frostbreath!");
         break;
	}
}

void cast_acid_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_acid_breath(level, ch, tar_ch, 0);
			break;   /* It's a spell.. But people can'c cast it! */
      default : 
         log("Serious screw-up in acidbreath!");
         break;
	}
}

void cast_gas_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
				spell_gas_breath(level,ch,tar_ch,0);
         break;
			/* THIS ONE HURTS!! */
      default : 
         log("Serious screw-up in gasbreath!");
         break;
	}
}

void cast_lightning_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_lightning_breath(level, ch, tar_ch, 0);
			break;   /* It's a spell.. But people can'c cast it! */
      default : 
         log("Serious screw-up in lightningbreath!");
         break;
	}
}



void cast_knock( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
        byte percent;
	int door, other_room;
	char dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
        char otype[MAX_INPUT_LENGTH];
	struct room_direction_data *back;
	struct obj_data *obj;
        struct char_data *victim;

   switch(type) {
   case SPELL_TYPE_SPELL:
   case SPELL_TYPE_SCROLL:
   case SPELL_TYPE_WAND: {

   argument_interpreter(arg, otype, dir);

   if (!otype) {
      send_to_char("Knock on what?\n\r",ch);
      return;
   }

   if (generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {

       	if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
                sprintf(buf," %s is not a container.\n\r ",obj->name);
        } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
                sprintf(buf, " Silly! %s isn't even closed!\n\r ", obj->name);
        } else if (obj->obj_flags.value[2] < 0) {
                sprintf(buf,"%s doesn't have a lock...\n\r",obj->name);
       	} else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
                sprintf(buf,"Hehe.. %s wasn't even locked.\n\r",ch);
	} else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) {
                sprintf(buf,"%s resists your magic.\n\r",obj->name);
	} else {
	     REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	     sprintf(buf,"<Click>\n\r");
	     act("$n magically opens $p", FALSE, ch, obj, 0, TO_ROOM);
        }
        send_to_char(buf,ch);
        return;
   } else if ((door = find_door(ch, otype, dir)) >= 0) {

       	if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	       	send_to_char("That's absurd.\n\r", ch);
       	else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	    send_to_char("You realize that the door is already open.\n\r", ch);
       	else if (EXIT(ch, door)->key < 0)
	      send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
       	else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	       	send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
	else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
	     send_to_char("You seem to be unable to knock this...\n\r", ch);
	else {
	     REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	     if (EXIT(ch, door)->keyword)
		 act("$n magically opens the lock of the $F.", 0, ch, 0,
		      EXIT(ch, door)->keyword, TO_ROOM);
	     else
		 act("$n magically opens the lock.", TRUE, ch, 0, 0, TO_ROOM);
       	     send_to_char("The lock quickly yields to your skills.\n\r", ch);
	     if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	       	if (back = real_roomp(other_room)->dir_option[rev_dir[door]])
	       	   if (back->to_room == ch->in_room)
		      	REMOVE_BIT(back->exit_info, EX_LOCKED);
	   }
         }
      }
      break;
   default:
      log("serious error in Knock.");
      break;
   }
}


void cast_know_alignment(byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_know_alignment(level, ch, tar_ch,tar_obj);
			break;
    case SPELL_TYPE_POTION:
			spell_know_alignment(level, ch, ch,0);
			break;
    case SPELL_TYPE_SCROLL:
         if (!tar_ch) tar_ch = ch;
			spell_know_alignment(level, ch, tar_ch, 0);
			break;
    default : 
         log("Serious screw-up in know alignment!");
         break;
	}
}

void cast_weakness( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
			spell_weakness(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
			spell_weakness(level, ch, ch, 0);
			break;
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
    default : 
         log("Serious screw-up in weakness!");
         break;
	}
}

void cast_dispel_magic( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
       	spell_dispel_magic(level, ch, tar_ch, tar_obj);
       	break;
    case SPELL_TYPE_POTION:
         spell_dispel_magic(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
/*
         if(tar_obj) {
				spell_dispel_magic(level, ch, 0, tar_obj);
 				return;
			}
*/
         if(!tar_ch) tar_ch = ch;
			spell_dispel_magic(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_dispel_magic(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in dispel magic");
         break;
	}
}


void cast_animate_dead( byte level, struct char_data *ch, char *arg, int type,
	struct char_data *tar_ch, struct obj_data *tar_obj )
{

    struct obj_data *i;

    switch(type) {

    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
       if (tar_obj) {
      if (IS_CORPSE(tar_obj)) {
             spell_animate_dead(level, ch, 0, tar_obj);
	  } else {
	    send_to_char("That's not a corpse!\n\r",ch);
	    return;
	  }
	} else {
	  send_to_char("That isn't a corpse!\n\r",ch);
	  return;
	}
       	break;
    case SPELL_TYPE_POTION:
       send_to_char("Your body revolts against the magic liquid.\n\r",ch);
       ch->points.hit = 0;
       break;
    case SPELL_TYPE_STAFF:
	for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
          if (IS_CORPSE(i)) {
              spell_animate_dead(level,ch,0,i);
	    }
	 }
         break;
    default : 
         log("Serious screw-up in animate_dead!");
         break;
	}
}


void cast_succor( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

   switch(type) {
   case SPELL_TYPE_SPELL:
   case SPELL_TYPE_WAND:
   case SPELL_TYPE_STAFF:
       spell_succor(level, ch, 0, 0);
   }

}

void cast_well_of_knowledge(byte level, struct char_data *ch, char *arg, 
   int type, struct char_data *victim, struct obj_data *tar_obj)
{

   switch(type) {
   case SPELL_TYPE_SPELL:
   case SPELL_TYPE_WAND:
   case SPELL_TYPE_STAFF:
      spell_well_of_knowledge(level, ch, 0, 0);
   }

}

void cast_paralyze( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_paralyze(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
			spell_paralyze(level, ch, ch, 0);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_paralyze(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(tar_obj) return;
         spell_paralyze(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_paralyze(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in paralyze");
         break;
	}
}

void cast_fear( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_fear(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_fear(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_fear(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_fear(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (!in_group(tar_ch,ch) )
	spell_fear(level,ch,tar_ch,0);
    break;
    default : 
      log("Serious screw-up in fear");
    break;
  }
}

void cast_turn( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
       	spell_turn(level, ch, tar_ch, 0);
       	break;
    case SPELL_TYPE_SCROLL:
        if(tar_obj) return;
        if (!tar_ch) tar_ch = ch;
          spell_turn(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
         if (tar_obj) return;
         if (!tar_ch) tar_ch = ch;
            spell_turn(level, ch, tar_ch, 0);
            break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch) )
                  spell_turn(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in turn");
         break;
	}
}

void cast_faerie_fog( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
	switch (type) {
		case SPELL_TYPE_SPELL:
	        case SPELL_TYPE_STAFF:
                case SPELL_TYPE_SCROLL:
			spell_faerie_fog(level, ch, 0, 0);
			break;
                default : 
                     log("Serious screw-up in faerie fog!");
                break;
       }
}


void cast_poly_self( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  char buffer[40];
  int mobn, X=LAST_POLY_MOB, found=FALSE;
  struct char_data *mob;

  one_argument(arg,buffer);

  if (IS_NPC(ch)) {
    send_to_char("You don't really want to do that.\n\r",ch);
    return;
  }
  
  switch(type) {
  case SPELL_TYPE_SPELL:   {
    
    while (!found) {
      if (PolyList[X].level > level) {
	X--;
      } else {
	if (!str_cmp(PolyList[X].name, buffer)) {
	  mobn = PolyList[X].number;
	  found = TRUE;
	} else {
      	   X--;
	}
	if (X < 0)
          break;
      }
    }
    
    if (!found) {
      send_to_char("Couldn't find any of those\n\r", ch);
      return;
    } else {
      mob = read_mobile(mobn, VIRTUAL);
      if (mob) {
	spell_poly_self(level, ch, mob, 0);
      } else {
	send_to_char("You couldn't summon an image of that creature\n\r", ch);
      }
      return;
    }
    
  } break;  
    
  default: {
    log("Problem in poly_self");
  } break;
  }

}


#define LONG_SWORD   3022
#define SHIELD       3042     
#define RAFT         3060
#define BAG          3032
#define WATER_BARREL 6013
#define BREAD        3010

void cast_minor_creation(byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  char buffer[40];
  int mob, obj;
  struct obj_data *o;

  one_argument(arg,buffer);

  if (!str_cmp(buffer, "sword")) {
    obj = LONG_SWORD;
  } else if (!str_cmp(buffer, "shield")) {
    obj=SHIELD;
  } else if (!str_cmp(buffer, "raft")) {
    obj=RAFT;
  } else if (!str_cmp(buffer, "bag")) {
    obj=BAG;
  } else if (!str_cmp(buffer, "barrel")) {
    obj=WATER_BARREL;
  } else if (!str_cmp(buffer, "bread")) {
    obj=BREAD;
  } else {
    send_to_char("There is nothing of that available\n\r", ch);
    return;
  }

  o = read_object(obj, VIRTUAL);
  if (!o) {
      send_to_char("There is nothing of that available\n\r", ch);
      return;
  }

  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
      spell_minor_create(level, ch, 0, o);
      break;
  default:
      log("serious screw-up in minor_create.");
      break;
  }

}

#define FIRE_ELEMENTAL  10
#define WATER_ELEMENTAL 11
#define AIR_ELEMENTAL   13
#define EARTH_ELEMENTAL 12

#define RED_STONE       5233
#define PALE_BLUE_STONE 5230
#define GREY_STONE     5239
#define CLEAR_STONE      5243

void cast_conjure_elemental( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  char buffer[40];
  int mob, obj;
  struct obj_data *sac;
  struct char_data *el;

  one_argument(arg,buffer);

  if (!str_cmp(buffer, "fire")) {
    mob = FIRE_ELEMENTAL;
    obj = RED_STONE;
  } else if (!str_cmp(buffer, "water")) {
    mob = WATER_ELEMENTAL;
    obj = PALE_BLUE_STONE;
  } else if (!str_cmp(buffer, "air")) {
    mob = AIR_ELEMENTAL;
    obj = CLEAR_STONE; 
 } else if (!str_cmp(buffer, "earth")) {
    mob = EARTH_ELEMENTAL;
    obj = GREY_STONE;
  } else {
    send_to_char("There are no elementals of that type available\n\r", ch);
    return;
  }
  if (!ch->equipment[HOLD]) {
    send_to_char(" You must be holding the correct stone\n\r", ch);
    return;
  }
    
  sac = unequip_char(ch, HOLD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != obj) {
     send_to_char("You must have the correct item to sacrifice.\n\r", ch);
     return;
    }
    el = read_mobile(mob, VIRTUAL);
    if (!el) {
      send_to_char("There are no elementals of that type available\n\r", ch);
      return;
    }
  } else {
     send_to_char("You must be holding the correct item to sacrifice.\n\r", ch);
     return;
  }

  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
      spell_conjure_elemental(level, ch, el, sac);
      break;
  default:
      log("serious screw-up in conjure_elemental.");
      break;
  }

}

#define DEMON_TYPE_I     20
#define DEMON_TYPE_II    21
#define DEMON_TYPE_III   22
#define DEMON_TYPE_IV    23
#define DEMON_TYPE_V     24
#define DEMON_TYPE_VI    25

#define SWORD_ANCIENTS   25000
#define SHADOWSHIV       25014
#define FIRE_SWORD       25015
#define SILVER_TRIDENT   25016
#define JEWELLED_DAGGER  25019
#define SWORD_SHARPNESS  25017

void cast_cacaodemon( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  char buffer[40];
  int mob, obj;
  struct obj_data *sac;
  struct char_data *el;

  one_argument(arg,buffer);

  if (!str_cmp(buffer, "one")) {
    mob = DEMON_TYPE_I;
    obj = SWORD_SHARPNESS;
  } else if (!str_cmp(buffer, "two")) {
    mob = DEMON_TYPE_II;
    obj = JEWELLED_DAGGER;
  } else if (!str_cmp(buffer, "three")) {
    mob = DEMON_TYPE_III;
    obj = SILVER_TRIDENT; 
 } else if (!str_cmp(buffer, "four")) {
    mob = DEMON_TYPE_IV;
    obj = FIRE_SWORD;
 } else if (!str_cmp(buffer, "five")) {
    mob = DEMON_TYPE_V;
    obj = SHADOWSHIV;
 } else if (!str_cmp(buffer, "six")) {
    mob = DEMON_TYPE_VI;
    obj = SWORD_ANCIENTS;
  } else {
    send_to_char("There are no demons of that type available\n\r", ch);
    return;
  }
  if (!ch->equipment[WIELD]) {
    send_to_char(" You must be wielding the correct item\n\r", ch);
    return;
  }

  if (obj_index[ch->equipment[WIELD]->item_number].virtual != obj) {
    send_to_char(" You must be wielding the correct item\n\r", ch);
    return;
  }
    
  sac = unequip_char(ch, WIELD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != obj) {
     send_to_char("You must have the correct item to sacrifice.\n\r", ch);
     return;
    }
    el = read_mobile(mob, VIRTUAL);
    if (!el) {
      send_to_char("There are no demons of that type available\n\r", ch);
      return;
    }
  } else {
     send_to_char("You must be holding the correct item to sacrifice.\n\r", ch);
     return;
  }

  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
      spell_cacaodemon(level, ch, el, sac);
      break;
  default:
      log("serious screw-up in conjure_elemental.");
      break;
  }

}

void cast_mon_sum1( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(5, ch, 0, 0);
         break;
       default:
     log("Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum2( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(7, ch, 0, 0);
         break;
       default:
     log("Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum3( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(9, ch, 0, 0);
         break;
       default:
     log("Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum4( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(11, ch, 0, 0);
         break;
       default:
     log("Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum5( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(13, ch, 0, 0);
         break;
       default:
     log("Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum6( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(15, ch, 0, 0);
         break;
       default:
     log("Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum7( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(17, ch, 0, 0);
         break;
       default:
     log("Serious screw-up in monster_summoning_1");
	 break;
       }
}
