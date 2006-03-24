/* ************************************************************************
 *  file: handler.c , Handler module.                      Part of DIKUMUD *
 *  Usage: Various routines for moving about objects/players               *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "spells.h"

#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct obj_data  *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct str_app_type str_app[];
extern int vol_mult[];

/* External procedures */

int str_cmp(char *arg1, char *arg2);
void free_char(struct char_data *ch);
void stop_fighting(struct char_data *ch);
void remove_follower(struct char_data *ch);



char *fname(char *namelist)
{
  static char holder[30];
  register char *point;
  
  for (point = holder; isalpha(*namelist); namelist++, point++)
    *point = *namelist;
  
  *point = '\0';
  
  return(holder);
}


int split_string(char *str, char *sep, char **argv)
     /* str must be writable */
{
  char	*s;
  int	argc=0;
  
  s = strtok(str, sep);
  if (s)
    argv[argc++] = s;
  else {
    *argv = str;
    return 1;
  }
  
  while  (s=strtok(NULL, sep)) {
    argv[argc++] = s;
  }
  return argc;
}

int isname(const char *str, const char *namelist)
{
  char *argv[100], *xargv[100];
  int argc, xargc, i, j;
  int exact;
  char buf[MAX_INPUT_LENGTH], names[MAX_INPUT_LENGTH], *s;

  strcpy(buf, str);
  argc = split_string(buf, "- \t\n\r,", argv);

  strcpy(names, namelist);
  xargc = split_string(names, "- \t\n\r,", xargv);

  s = argv[argc - 1];
  s += strlen(s);
  if (*(--s) == '.') {
    exact = TRUE;
    *s = 0;
  } else
    exact = FALSE;

  if (exact && argc != xargc)
    return FALSE;

  for (i = 0; i < argc; i++) {
    for (j = 0; j < xargc; j++) {
      if (xargv[j] && is_abbrev(argv[i], xargv[j])) {
        xargv[j] = NULL;
        break;
      }
    }
    if (j >= xargc)
      return FALSE;
  }
  return TRUE;
}

void init_string_block(struct string_block *sb)
{
  sb->data = (char*)malloc(sb->size=128);
  *sb->data = '\0';
}

void append_to_string_block(struct string_block *sb, char *str)
{
  int	len;
  len = strlen(sb->data) + strlen(str) + 1;
  if (len > sb->size) {
    if ( len > (sb->size*=2))
      sb->size = len;
    sb->data = (char*)realloc(sb->data, sb->size);
  }
  strcat(sb->data, str);
}

void page_string_block(struct string_block *sb, struct char_data *ch)
{
  page_string(ch->desc, sb->data, 1);
}

void destroy_string_block(struct string_block *sb)
{
  free(sb->data);
  sb->data = NULL;
}

void affect_modify(struct char_data *ch,byte loc, long mod, long bitv, bool add)
{
  int maxabil;
  int i, diff;
  
  
  if (loc == APPLY_IMMUNE) {
    if (add) {
      SET_BIT(ch->immune, mod);
    } else {
      REMOVE_BIT(ch->immune, mod);
    }
  } else if (loc == APPLY_SUSC) {
    if (add) {
      SET_BIT(ch->susc, mod);
    } else {
      REMOVE_BIT(ch->susc, mod);
    }
    
  } else if (loc == APPLY_M_IMMUNE) {
    if (add) {
      SET_BIT(ch->M_immune, mod);
    } else {
      REMOVE_BIT(ch->M_immune, mod);
    }
  } else if (loc == APPLY_SPELL) {
    if (add) {
      SET_BIT(ch->specials.affected_by, mod);
    } else {
      REMOVE_BIT(ch->specials.affected_by, mod);
    }
  } else if (loc == APPLY_WEAPON_SPELL) {
    return;
  } else {
    if (add) {
      SET_BIT(ch->specials.affected_by, bitv);
    } else {
      REMOVE_BIT(ch->specials.affected_by, bitv);
      mod = -mod;
    }
  }
  
  maxabil = (IS_NPC(ch) ? 25:18);
  
  switch(loc)
    {
    case APPLY_NONE:
      break;
      
    case APPLY_STR:
      GET_STR(ch) += mod;
      break;
      
    case APPLY_DEX:
      GET_DEX(ch) += mod;
      break;
      
    case APPLY_INT:
      GET_INT(ch) += mod;
      break;
      
    case APPLY_WIS:
      GET_WIS(ch) += mod;
      break;
      
    case APPLY_CON:
      GET_CON(ch) += mod;
      break;
      
    case APPLY_SEX:
      GET_SEX(ch) = (!(ch->player.sex-1))+1;
      break;
      
    case APPLY_CLASS:
      break;
      
    case APPLY_LEVEL:
      break;
      
    case APPLY_AGE:
      ch->player.time.birth -= SECS_PER_MUD_YEAR*mod;
      break;
      
    case APPLY_CHAR_WEIGHT:
      GET_WEIGHT(ch) += mod;
      break;
      
    case APPLY_CHAR_HEIGHT:
      GET_HEIGHT(ch) += mod;
      break;
      
    case APPLY_MANA:
      ch->points.max_mana += mod;
      break;
      
    case APPLY_HIT:
      ch->points.max_hit += mod;
      break;
      
    case APPLY_MOVE:
      ch->points.max_move += mod;
      break;
      
    case APPLY_GOLD:
      break;
      
    case APPLY_EXP:
      break;
      
    case APPLY_AC:
      GET_AC(ch) += mod;
      break;
      
    case APPLY_HITROLL:
      GET_HITROLL(ch) += mod;
      break;
      
    case APPLY_DAMROLL:
      GET_DAMROLL(ch) += mod;
      break;
      
    case APPLY_SAVING_PARA:
      ch->specials.apply_saving_throw[0] += mod;
      break;
      
    case APPLY_SAVING_ROD:
      ch->specials.apply_saving_throw[1] += mod;
      break;
      
    case APPLY_SAVING_PETRI:
      ch->specials.apply_saving_throw[2] += mod;
      break;
      
    case APPLY_SAVING_BREATH:
      ch->specials.apply_saving_throw[3] += mod;
      break;
      
    case APPLY_SAVING_SPELL:
      ch->specials.apply_saving_throw[4] += mod;
      break;
      
    case APPLY_SAVE_ALL: 
      {
	for (i=0;i<=4;i++)
	  ch->specials.apply_saving_throw[i] += mod;
      }
      break;
    case APPLY_IMMUNE:
      break;
    case APPLY_SUSC:
      break;
    case APPLY_M_IMMUNE:
      break;
    case APPLY_SPELL:
      break;
    case APPLY_HITNDAM:
      GET_HITROLL(ch) += mod;
      GET_DAMROLL(ch) += mod;
      break; 
    case APPLY_WEAPON_SPELL:
    case APPLY_EAT_SPELL:
      break;
    case APPLY_BACKSTAB:
      if (!ch->skills) return;
      ch->skills[SKILL_BACKSTAB].learned += mod;
      break;
    case APPLY_KICK:
      if (!ch->skills) return;
      ch->skills[SKILL_KICK].learned += mod;
      break;
    case APPLY_SNEAK:
      if (!ch->skills) return;
      ch->skills[SKILL_SNEAK].learned += mod;
      break;
    case APPLY_HIDE:
      if (!ch->skills) return;
      ch->skills[SKILL_HIDE].learned += mod;
      break;
    case APPLY_BASH:
      if (!ch->skills) return;
      ch->skills[SKILL_BASH].learned += mod;
      break;
    case APPLY_PICK:
      if (!ch->skills) return;
      ch->skills[SKILL_PICK_LOCK].learned += mod;
      break;
    case APPLY_STEAL:
      if (!ch->skills) return;
      ch->skills[SKILL_STEAL].learned += mod;
      break;
    case APPLY_TRACK:
      if (!ch->skills) return;
      ch->skills[SKILL_HUNT].learned += mod;
      break;
    case APPLY_DEATHSTROKE:
       if (!ch->skills) return;
       ch->skills[SKILL_DEATHSTROKE].learned += mod;
       break;
    case APPLY_DOUBLE_ATTACK:  
       if (!ch->skills) return;
      ch->skills[SKILL_DOUBLE_ATTACK].learned += mod;
      break;
    case APPLY_GRAPPLE:
     if (!ch->skills) return;
      ch->skills[SKILL_GRAPPLE].learned += mod;
      break;
    case APPLY_THROW:
     if (!ch->skills) return;
      ch->skills[SKILL_THROW].learned += mod;
      break;
    default:
      log("Unknown apply adjust attempt (handler.c, affect_modify).");
      log(ch->player.name);
      
      break;
      
    } /* switch */
}



/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data *ch)
{
  struct affected_type *af;
  int i,j;
  
  for(i=0; i<MAX_WEAR; i++) {
    if (ch->equipment[i])
      for(j=0; j<MAX_OBJ_AFFECT; j++)
	affect_modify(ch, ch->equipment[i]->affected[j].location,
		      ch->equipment[i]->affected[j].modifier,
		      ch->equipment[i]->obj_flags.bitvector, FALSE);
  }
  
  for(af = ch->affected; af; af=af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
  
  ch->tmpabilities = ch->abilities;
  
  for(i=0; i<MAX_WEAR; i++) {
    if (ch->equipment[i])
      for(j=0; j<MAX_OBJ_AFFECT; j++)
	affect_modify(ch, ch->equipment[i]->affected[j].location,
		      ch->equipment[i]->affected[j].modifier,
		      ch->equipment[i]->obj_flags.bitvector, TRUE);
  }
  
  
  for(af = ch->affected; af; af=af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
  
  /* Make certain values are between 0..25, not < 0 and not > 25! */
  
  i = (IS_NPC(ch) ? 25 :18);
  
 if (ch->abilities.dex == 19)
       GET_DEX(ch) = MAX(1,MIN(GET_DEX(ch), 19));
 else
       GET_DEX(ch) = MAX(1,MIN(GET_DEX(ch), 18));

 if (ch->abilities.intel == 19)
       GET_INT(ch) = MAX(1,MIN(GET_INT(ch), 19));
 else
       GET_INT(ch) = MAX(1,MIN(GET_INT(ch), 18));

 if (ch->abilities.wis == 19)
       GET_WIS(ch) = MAX(1,MIN(GET_WIS(ch), 19));
 else
       GET_WIS(ch) = MAX(1,MIN(GET_WIS(ch), 18));

 if (ch->abilities.con == 19)
       GET_CON(ch) = MAX(1,MIN(GET_CON(ch), 19));
 else
       GET_CON(ch) = MAX(1,MIN(GET_CON(ch), 18));

 if (ch->abilities.str == 19)
       GET_STR(ch) = MAX(1,MIN(GET_STR(ch), 19));
 else
       GET_STR(ch) = MAX(1,MIN(GET_STR(ch), 18));
  
  if (IS_NPC(ch)) {
    GET_STR(ch) = MIN(GET_STR(ch), i);
  } else {
    if (GET_STR(ch) > 18) {
      i = GET_ADD(ch) + ((GET_STR(ch)-18)*10);
      GET_ADD(ch) = MIN(i, 100);
    }
  }
}



/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char( struct char_data *ch, struct affected_type *af )
{
  struct affected_type *affected_alloc;
  
  CREATE(affected_alloc, struct affected_type, 1);
  
  *affected_alloc = *af;
  affected_alloc->next = ch->affected;
  ch->affected = affected_alloc;
  
  affect_modify(ch, af->location, af->modifier,
		af->bitvector, TRUE);
  affect_total(ch);
}



/* Remove an affected_type structure from a char (called when duration
   reaches zero). Pointer *af must never be NIL! Frees mem and calls 
   affect_location_apply                                                */
void affect_remove( struct char_data *ch, struct affected_type *af )
{
  struct affected_type *hjp;
  
  if (!ch->affected) {
    log("affect removed from char without affect");
    log(GET_NAME(ch));
    return;
  }
  
  affect_modify(ch, af->location, af->modifier,
		af->bitvector, FALSE);
  
  
  /* remove structure *af from linked list */
  
  if (ch->affected == af) {
    /* remove head of list */
    ch->affected = af->next;
  } else {
    
    for(hjp = ch->affected; (hjp->next) && (hjp->next != af); hjp = hjp->next);
    
    if (hjp->next != af) {
      log("Could not locate affected_type in ch->affected. (handler.c, affect_remove)");
      return;
    }
    hjp->next = af->next; /* skip the af element */
  }
  
  free ( af );
  
  affect_total(ch);
}



/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char( struct char_data *ch, short skill)
{
  struct affected_type *hjp;
  
  for(hjp = ch->affected; hjp; hjp = hjp->next)
    if (hjp->type == skill)
      affect_remove( ch, hjp );
  
}



/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates 
   not affected                                                        */
bool affected_by_spell( struct char_data *ch, short skill )
{
  struct affected_type *hjp;
  
  for (hjp = ch->affected; hjp; hjp = hjp->next)
    if ( hjp->type == skill )
      return( TRUE );
  
  return( FALSE );
}



void affect_join( struct char_data *ch, struct affected_type *af,
		 bool avg_dur, bool avg_mod )
{
  struct affected_type *hjp;
  bool found = FALSE;
  
  for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {
    if ( hjp->type == af->type ) {
      
      af->duration += hjp->duration;
      if (avg_dur)
	af->duration /= 2;
      
      af->modifier += hjp->modifier;
      if (avg_mod)
	af->modifier /= 2;
      
      affect_remove(ch, hjp);
      affect_to_char(ch, af);
      found = TRUE;
    }
  }
  if (!found)
    affect_to_char(ch, af);
}

/* move a player out of a room */
void char_from_room(struct char_data *ch)
{
  char buf[MAX_INPUT_LENGTH];
  struct char_data *i;
  struct room_data *rp;
  
  if (ch->in_room == NOWHERE) {
    log("NOWHERE extracting char from room (handler.c, char_from_room)");
    return;
  }
  
  if (ch->equipment[WEAR_LIGHT])
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) /* Light is ON */
	real_roomp(ch->in_room)->light--;
  
  rp = real_roomp(ch->in_room);
  if (rp==NULL) {
    sprintf(buf, "ERROR: char_from_room: %s was not in a valid room (%d)",
	    (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr),
	    ch->in_room);
    log(buf);
    return;
  }
  
  if (ch == rp->people)  /* head of list */
    rp->people = ch->next_in_room;
  
  else {   /* locate the previous element */
    for (i = rp->people; i && i->next_in_room != ch; i = i->next_in_room)
      ;
    if (i)
      i->next_in_room = ch->next_in_room;
    else {
      sprintf(buf, "SHIT, %s was not in people list of his room %d!",
	      (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr),
	      ch->in_room);
      log(buf);
    }
  }
  
  ch->in_room = NOWHERE;
  ch->next_in_room = 0;
}


/* place a character in a room */
void char_to_room(struct char_data *ch, int room)
{
  struct room_data *rp;
  
  rp = real_roomp(room);
  if (!rp) {
    room = 0;
    rp = real_roomp(room);
    if (!rp)
      exit(0);
  }
  ch->next_in_room = rp->people;
  rp->people = ch;
  ch->in_room = room;
  
  if (ch->equipment[WEAR_LIGHT])
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) /* Light is ON */
	rp->light++;
}


/* give an object to a char   */
void obj_to_char(struct obj_data *object, struct char_data *ch)
{

  assert(!object->in_obj && !object->carried_by && !object->equipped_by &&
	 object->in_room == NOWHERE);
  
  if (ch->carrying)
    object->next_content = ch->carrying;
  else
    object->next_content = 0;
  
  ch->carrying = object;
  object->carried_by = ch;
  object->in_room = NOWHERE;
  object->equipped_by = 0;
  object->in_obj = 0;
  IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
  IS_CARRYING_N(ch) += GET_OBJ_VOLUME(object);
}


/* take an object from a char */
void obj_from_char(struct obj_data *object)
{
  struct obj_data *tmp;
  
  if (!object) {
    log("No object to be take from char.");
    abort();
  }
  
  
  if (!object->carried_by) {
    log("this object is not carried by anyone");
    abort();
  }
  
  if (!object->carried_by->carrying) {
    log("No one is carrying this object");
    abort();
  }

  if (object->in_obj) {
    log("Obj in more than one place.");
    abort();
  }

  if (object->equipped_by) {
    log("Obj in more than one place.");
    abort();
  }
  
  if (object->carried_by->carrying == object)   /* head of list */
    object->carried_by->carrying = object->next_content;
  
  else
    {
      for (tmp = object->carried_by->carrying; 
	   tmp && (tmp->next_content != object); 
	   tmp = tmp->next_content); /* locate previous */
      
      if (!tmp) {
	log("Couldn't find object on character");
	abort();
      }
      
      tmp->next_content = object->next_content;
    }
  
  IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
  IS_CARRYING_N(object->carried_by) -= GET_OBJ_VOLUME(object);
  object->carried_by = 0;
  object->equipped_by = 0; /* should be unnecessary, but, why risk it */
  object->next_content = 0;
  object->in_obj = 0;
}



/* Return the effect of a piece of armor in position eq_pos */
int apply_ac(struct char_data *ch, int eq_pos)
{
  assert(ch->equipment[eq_pos]);
  
  if (!(GET_ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_ARMOR))
    return 0;
  
  switch (eq_pos) {
    
  case WEAR_BODY:
    return (2*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
  case WEAR_HEAD:
    return (2*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
  case WEAR_LEGS:
    return (ch->equipment[eq_pos]->obj_flags.value[0]);  /* 10% */
  case WEAR_FEET:
    return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
  case WEAR_HANDS:
    return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
  case WEAR_ARMS:
    return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
  case WEAR_SHIELD:
    return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
  }
  return 0;
}



void equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
  int j;
  
  assert(pos>=0 && pos<MAX_WEAR);
  assert(!(ch->equipment[pos]));
  
  if (obj->carried_by) {
    log("EQUIP: Obj is carried_by when equip.");
    abort();
  }
  
  if (obj->in_room!=NOWHERE) {
    log("EQUIP: Obj is in_room when equip.");
    abort();
    return;
  }
  
  if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
    if (ch->in_room != NOWHERE) {
      
      act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
      act("$n is zapped by $p and instantly drops it.", FALSE, ch, obj, 0, TO_ROOM);
      obj_to_room(obj, ch->in_room);
      return;
    }
  }
 if ((GET_ITEM_TYPE(obj) == ITEM_ARMOR) &&
     (HasClass(ch, CLASS_MONK))) {
   send_to_char("Your monk vows wont let you wear such items.\n\r", ch);
   obj_to_char(obj, ch);
  return;
} 
 if ((IS_OBJ_STAT(obj, ITEM_LEVEL10) && (GetMaxLevel(ch) < 10)) ||
     (IS_OBJ_STAT(obj, ITEM_LEVEL15) && (GetMaxLevel(ch) < 15)) ||
     (IS_OBJ_STAT(obj, ITEM_LEVEL20) && (GetMaxLevel(ch) < 20)) ||
     (IS_OBJ_STAT(obj, ITEM_LEVEL25) && (GetMaxLevel(ch) < 25)) ||
     (IS_OBJ_STAT(obj, ITEM_LEVEL30) && (GetMaxLevel(ch) < 30)) ||
     (IS_OBJ_STAT(obj, ITEM_LEVEL35) && (GetMaxLevel(ch) < 35)) ||
     (IS_OBJ_STAT(obj, ITEM_LEVEL40) && (GetMaxLevel(ch) < 40))) {
  if (ch->in_room != NOWHERE) {

    act("You do not know how to use the $p.", FALSE, ch, obj, 0, TO_CHAR);
    send_to_char("Maybe a little more experience will help you understand!\n\r",                  ch);
    act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n is zapped by $p and instantly drops it.", FALSE, ch, obj, 0, TO_ROOM);
    obj_to_room(obj, ch->in_room);
    return;
   } else {

      log("ch->in_room = NOWHERE when equipping char.");
      abort();
    }
  }
  
  ch->equipment[pos] = obj;
  obj->equipped_by = ch;
  obj->eq_pos = pos;
  
  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
    GET_AC(ch) -= apply_ac(ch, pos);
  
  for(j=0; j<MAX_OBJ_AFFECT; j++)
    affect_modify(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, TRUE);

  if (GET_ITEM_TYPE(obj) == ITEM_WEAPON) {
    /* some nifty manuevering for strength */
    if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF))
       GiveMinStrToWield(obj, ch);
  }
  
  affect_total(ch);
}


int GiveMinStrToWield(struct obj_data *obj, struct char_data *ch)
{
  int str=0;

  GET_STR(ch) = 16;  /* nice, semi-reasonable start */
  /* 
    will have a problem with except. str, that i do not care to solve
  */

  while (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
     GET_STR(ch)++;

  return(str);

}

struct obj_data *unequip_char(struct char_data *ch, int pos)
{
  int j;
  struct obj_data *obj;
  
  assert(pos>=0 && pos<MAX_WEAR);
  assert(ch->equipment[pos]);
  
  obj = ch->equipment[pos];

  assert(!obj->in_obj && obj->in_room == NOWHERE && !obj->carried_by);

  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
    GET_AC(ch) += apply_ac(ch, pos);
  
  ch->equipment[pos] = 0;
  obj->equipped_by = 0;
  obj->eq_pos = -1;
  
  for(j=0; j<MAX_OBJ_AFFECT; j++)
    affect_modify(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, FALSE);
  
  affect_total(ch);
  if (GET_MANA(ch) >= mana_limit(ch))
       GET_MANA(ch) = mana_limit(ch);
  if (GET_HIT(ch) >= hit_limit(ch))
       GET_HIT(ch) = hit_limit(ch);
  
  return(obj);
}

struct obj_data *unequip_char_for_save(struct char_data *ch, int pos)
{
  int j;
  struct obj_data *obj;
   
  assert(pos>=0 && pos<MAX_WEAR);
  assert(ch->equipment[pos]);
  
  obj = ch->equipment[pos];
 
  assert(!obj->in_obj && obj->in_room == NOWHERE && !obj->carried_by);
 
  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
    GET_AC(ch) += apply_ac(ch, pos);
  
  ch->equipment[pos] = 0;
  obj->equipped_by = 0;
  obj->eq_pos = -1;
  
  for(j=0; j<MAX_OBJ_AFFECT; j++)
    affect_modify(ch, obj->affected[j].location,
                  obj->affected[j].modifier,
                  obj->obj_flags.bitvector, FALSE);
  
  affect_total(ch);
  
  return(obj);
}


int get_number(char **name) {
  
  int i;
  char *ppos;
  char number[MAX_INPUT_LENGTH] = "";
  
  if ((ppos = (char *)index(*name, '.')) && ppos[1]) {
    *ppos++ = '\0';
    strcpy(number,*name);
    strcpy(*name, ppos);
    
    for(i=0; *(number+i); i++)
      if (!isdigit(*(number+i)))
	return(0);
    
    return(atoi(number));
  }
  
  return(1);
}


/* Search a given list for an object, and return a pointer to that object */
struct obj_data *get_obj_in_list(char *name, struct obj_data *list)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  
  
  if (!(number = get_number(&tmp)))
    return(0);
  
  for (i = list, j = 1; i && (j <= number); i = i->next_content)
    if (isname(tmp, i->name)) {
      if (j == number) 
	return(i);
      j++;
    }
  
  return(0);
}



/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
  struct obj_data *i;
  
  for (i = list; i; i = i->next_content)
    if (i->item_number == num) 
      return(i);
  
  return(0);
}





/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj(char *name)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  for (i = object_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, i->name)) {
      if (j == number)
	return(i);
      j++;
    }
  
  return(0);
}





/*search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
  struct obj_data *i;
  
  for (i = object_list; i; i = i->next)
    if (i->item_number == nr) 
      return(i);
  
  return(0);
}





/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, int room)
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  for (i = real_roomp(room)->people, j = 1; i && (j <= number); i = i->next_in_room)
    if (isname(tmp, GET_NAME(i))) {
      if (j == number)
        return(i);
      j++;
    }
  
  return(0);
}





/* search all over the world for a char, and return a pointer if found */
struct char_data *get_char(char *name)
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  for (i = character_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, GET_NAME(i))) {
      if (j == number)
	return(i);
      j++;
    }
  
  return(0);
}



/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
  struct char_data *i;
  
  for (i = character_list; i; i = i->next)
    if (i->nr == nr)
      return(i);
  
  return(0);
}




/* put an object in a room */
void obj_to_room(struct obj_data *object, int room)
{
  
  if (room == -1)
    room = 4;

  assert(!(object->equipped_by) && (object->eq_pos == -1));

  if (object->in_room > NOWHERE) {
    obj_from_room(object);
  }

  object->next_content = real_roomp(room)->contents;
  real_roomp(room)->contents = object;
  object->in_room = room;
  object->carried_by = 0;
  object->equipped_by = 0; /* should be unnecessary */
}


/* Take an object from a room */
void obj_from_room(struct obj_data *object)
{
  struct obj_data *i;
  
  /* remove object from room */

  if (object->in_room <= NOWHERE) {
    if (object->carried_by || object->equipped_by) {
       log("Eek.. an object was just taken from a char, instead of a room");
       abort();
    }
    return;  /* its not in a room */
  }
  
  if (object == real_roomp(object->in_room)->contents)  /* head of list */
    real_roomp(object->in_room)->contents = object->next_content;
  
  else     /* locate previous element in list */
    {
      for (i = real_roomp(object->in_room)->contents; i && 
	   (i->next_content != object); i = i->next_content);
      
      if (i) {
         i->next_content = object->next_content;
      } else {
	log("Couldn't find object in room");
	abort();
      }
    }
  
  object->in_room = NOWHERE;
  object->next_content = 0;
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to)
{
  struct obj_data *tmp_obj;
  
  obj->next_content = obj_to->contains;
  obj_to->contains = obj;
  obj->in_obj = obj_to;
  /*  
    (jdb)  hopefully this will fix the object problem   
    */
  obj->carried_by = 0;
  obj->equipped_by = 0;

  for(tmp_obj = obj->in_obj; tmp_obj;
      GET_OBJ_WEIGHT(tmp_obj) +=GET_OBJ_WEIGHT(obj),tmp_obj = tmp_obj->in_obj);

if (!IS_OBJ_STAT(obj_to, ITEM_HOLDING)) {
 if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER)
  for(tmp_obj = obj->in_obj; tmp_obj;
      GET_OBJ_VOLUME(tmp_obj) += (GET_OBJ_VOLUME(obj)/vol_mult[obj->obj_flags.material_points]), tmp_obj = tmp_obj->in_obj);
 else
  for (tmp_obj = obj->in_obj;tmp_obj;
      GET_OBJ_VOLUME(tmp_obj) +=GET_OBJ_VOLUME(obj),tmp_obj = tmp_obj->in_obj);
  }
}


/* remove an object from an object */
void obj_from_obj(struct obj_data *obj)
{
  struct obj_data *tmp, *obj_from;

  assert(!obj->carried_by && !obj->equipped_by && obj->in_room == NOWHERE);
  
  if (obj->in_obj) {
    obj_from = obj->in_obj;
    if (obj == obj_from->contains)   /* head of list */
      obj_from->contains = obj->next_content;
    else {
      for (tmp = obj_from->contains; 
	   tmp && (tmp->next_content != obj);
	   tmp = tmp->next_content); /* locate previous */
      
      if (!tmp) {
	perror("Fatal error in object structures.");
	abort();
      }
      
      tmp->next_content = obj->next_content;
    }
        
    /* Subtract weight from containers container */
    for(tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj) { 
      GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
    if (!IS_OBJ_STAT(tmp, ITEM_HOLDING)) {
     if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER)
      GET_OBJ_VOLUME(tmp) -= (GET_OBJ_VOLUME(obj)/vol_mult[obj->obj_flags.material_points]);
     else
      GET_OBJ_VOLUME(tmp) -= GET_OBJ_VOLUME(obj);
      }
     }
    
    GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
   if (!IS_OBJ_STAT(tmp, ITEM_HOLDING)) {
   if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER)
    GET_OBJ_VOLUME(tmp) -= (GET_OBJ_VOLUME(obj)/vol_mult[obj->obj_flags.material_points]);
   else 
    GET_OBJ_VOLUME(tmp) -= GET_OBJ_VOLUME(obj);
    }

    /* Subtract weight from char that carries the object */
    if (tmp->carried_by) {
      IS_CARRYING_W(tmp->carried_by) -= GET_OBJ_WEIGHT(obj);
   if (!IS_OBJ_STAT(tmp, ITEM_HOLDING)) {
    if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER)
      IS_CARRYING_N(tmp->carried_by) -= (GET_OBJ_VOLUME(obj)/vol_mult[obj->obj_flags.material_points]);
    else
      IS_CARRYING_N(tmp->carried_by) -= GET_OBJ_VOLUME(obj);
    }
   }
    
    obj->in_obj = 0;
    obj->next_content = 0;
  } else {
    perror("Trying to object from object when in no object.");
    abort();
  }
}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data *list, struct char_data *ch)
{
  if (list) {
    object_list_new_owner(list->contains, ch);
    object_list_new_owner(list->next_content, ch);
    list->carried_by = ch;
  }
}


/* Extract an object from the world */
void extract_obj(struct obj_data *obj)
{
  struct obj_data *temp1, *temp2;
  extern int obj_count;
  
  if(obj->in_room != NOWHERE)
    obj_from_room(obj);
  else if(obj->carried_by)
    obj_from_char(obj);
  else if (obj->equipped_by) {
    if (obj->eq_pos > -1) {
      /*
       **  set players equipment slot to 0; that will avoid the garbage items.
       */
      obj->equipped_by->equipment[obj->eq_pos] = 0;
      
    } else {
      log("Extract on equipped item in slot -1 on:");
      log(obj->equipped_by->player.name);
      log(obj->name);
      return;
    }
  } else if(obj->in_obj)	{
    temp1 = obj->in_obj;
    if(temp1->contains == obj)   /* head of list */
      temp1->contains = obj->next_content;
    else		{
      for( temp2 = temp1->contains ;
	  temp2 && (temp2->next_content != obj);
	  temp2 = temp2->next_content );
      
      if(temp2) {
	temp2->next_content =
	  obj->next_content; 
      }
    }
  }
  
  for( ; obj->contains; extract_obj(obj->contains)); 
  /* leaves nothing ! */
  
  if (object_list == obj )       /* head of list */
    object_list = obj->next;
  else {
    for(temp1 = object_list; 
	temp1 && (temp1->next != obj);
	temp1 = temp1->next);
    
    if(temp1) {
      temp1->next = obj->next;
    } else {
      log("Couldn't find object in object list.");
      abort();
    }
  }
  
  if(obj->item_number>=0)
    (obj_index[obj->item_number].number)--;
  free_obj(obj);

  obj_count--;
}

void update_object( struct obj_data *obj, int use){
  
  if (obj->obj_flags.decay_time > 0)	obj->obj_flags.decay_time -= use;
  if (obj->contains) update_object(obj->contains, use);
  if (obj->next_content) 
    if (obj->next_content != obj)
      update_object(obj->next_content, use);
}

void update_char_objects( struct char_data *ch )
{
  
  int i;
  
  if (ch->equipment[WEAR_LIGHT])
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] > 0)
	(ch->equipment[WEAR_LIGHT]->obj_flags.value[2])--;
  
  for(i = 0;i < MAX_WEAR;i++) 
    if (ch->equipment[i])
      update_object(ch->equipment[i],1);
  
  if (ch->carrying) update_object(ch->carrying,1);
}



/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(struct char_data *ch)
{
  struct obj_data *i, *o;
  struct char_data *k, *next_char;
  struct descriptor_data *t_desc;
  int l, was_in, j;

  extern long mob_count;  
  extern struct char_data *combat_list;
  
  void do_save(struct char_data *ch, char *argument, int cmd);
  void do_return(struct char_data *ch, char *argument, int cmd);
  
  void die_follower(struct char_data *ch);
  
  if(!IS_NPC(ch) && !ch->desc)	{
    for(t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
      if(t_desc->original==ch)
	do_return(t_desc->character, "", 0);
  }
  
  if (ch->in_room == NOWHERE) {
    log("NOWHERE extracting char. (handler.c, extract_char)");
    /*
     **  problem from linkdeath
     */
    char_to_room(ch, 4);  /* 4 == all purpose store */
  }
  
  if (ch->followers || ch->master)
    die_follower(ch);
  
  if(ch->desc) {
    /* Forget snooping */
    if ((ch->desc->snoop.snooping) && (ch->desc->snoop.snooping->desc))
      ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
    
    if (ch->desc->snoop.snoop_by) {
      send_to_char("Your victim is no longer among us.\n\r",
		   ch->desc->snoop.snoop_by);
      if (ch->desc->snoop.snoop_by->desc)
      ch->desc->snoop.snoop_by->desc->snoop.snooping = 0;
    }
    
    ch->desc->snoop.snooping = ch->desc->snoop.snoop_by = 0;
  }
  
  if (ch->carrying)	{
    /* transfer ch's objects to room */

    if (!IS_IMMORTAL(ch)) {

      while(ch->carrying) {
	i=ch->carrying;
	obj_from_char(i);
	obj_to_room(i, ch->in_room);
      }
    } else {

      send_to_char("Here, you dropped some stuff, let me help you get rid of that.\n\r",ch);
      while (ch->carrying) {
         i = ch->carrying;
	 obj_from_char(i);
	 extract_obj(i);
      }
      /*
	equipment too
	*/
      for (j=0; j<MAX_WEAR; j++)
	if (ch->equipment[j])
	  extract_obj(unequip_char(ch, j));
      
    }
    
  } else {
    if (IS_IMMORTAL(ch)) {
       for (j=0; j<MAX_WEAR; j++)
          if (ch->equipment[j])
                 extract_obj(unequip_char(ch, j));
      }
  }
  
  if (ch->specials.fighting)
    stop_fighting(ch);
  
  for (k = combat_list; k ; k = next_char)	{
    next_char = k->next_fighting;
    if (k->specials.fighting == ch)
      stop_fighting(k);
  }
  
  /* Must remove from room before removing the equipment! */
  was_in = ch->in_room;
  char_from_room(ch);
  
  /* clear equipment_list */
  for (l = 0; l < MAX_WEAR; l++)
    if (ch->equipment[l])
      obj_to_room(unequip_char(ch,l), was_in);
  
  
  if (IS_NPC(ch)) {
    for (k=character_list; k; k=k->next) {
      if (k->specials.hunting)
	if (k->specials.hunting == ch) {
	  k->specials.hunting = 0;
	}
      if (Hates(k, ch)) {
	RemHated(k, ch);
      }
      if (Fears(k, ch)) {
	RemFeared(k, ch);
      }
    }          
  } else {
    for (k=character_list; k; k=k->next) {
      if (k->specials.hunting)
	if (k->specials.hunting == ch) {
	  k->specials.hunting = 0;
	}
      if (Hates(k, ch)) {
	ZeroHatred(k, ch);
      }
      if (Fears(k, ch)) {
	ZeroFeared(k, ch);
      }
    }
    
  }
  /* pull the char from the list */
  
  if (ch == character_list)  
    character_list = ch->next;
  else
    {
      for(k = character_list; (k) && (k->next != ch); k = k->next);
      if(k)
	k->next = ch->next;
      else {
	log("Trying to remove ?? from character_list. (handler.c, extract_char)");
	abort();

      }
    }
  
  GET_AC(ch) = 100;
  
  if (ch->desc)	{
    if (ch->desc->original)
      do_return(ch, "", 0);
    save_char(ch, NOWHERE);
  }
  
  if (IS_NPC(ch)) 	{
    if (ch->nr > -1) /* if mobile */
      mob_index[ch->nr].number--;
    //FreeHates(ch);
    //FreeFears(ch);
    mob_count--;
    free_char(ch);
  }
  
  if (ch->desc) {
    ch->desc->connected = CON_SLCT;
    SEND_TO_Q(MENU, ch->desc);
  }
}



/* ***********************************************************************
   Here follows high-level versions of some earlier routines, ie functionst
   which incorporate the actual player-data.
   *********************************************************************** */


struct char_data *get_char_room_vis(struct char_data *ch, char *name)
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  for (i = real_roomp(ch->in_room)->people, j = 1; 
       i && (j <= number); i = i->next_in_room)
    if (isname(tmp, GET_NAME(i)))
      if (CAN_SEE(ch, i))	{
	if (j == number) 
	  return(i);
	j++;
      }
  
  return(0);
}


/* get a character from anywhere in the world, doesn't care much about
   being in the same room... */
struct char_data *get_char_vis_world(struct char_data *ch, char *name,
				     int *count)
     
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  j = count ? *count : 1;
  for (i = character_list; i && (j <= number); i = i->next)
    if (isname(tmp, GET_NAME(i)))
      if (CAN_SEE(ch, i))	{
	if (j == number)
	  return(i);
	j++;
      }
  if (count) *count = j;
  return 0;
}

struct char_data *get_char_vis(struct char_data *ch, char *name)
{
  struct char_data *i;
  
  /* check location */
  if (i = get_char_room_vis(ch, name))
    return(i);
  
  return get_char_vis_world(ch,name, NULL);
}






struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, 
				     struct obj_data *list)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  for (i = list, j = 1; i && (j <= number); i = i->next_content)
    if (isname(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i)) {
	if (j == number)
	  return(i);
	j++;
      }
  return(0);
}



struct obj_data *get_obj_vis_world(struct char_data *ch, char *name,
				   int *count)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  j = count ? *count : 1;
  
  /* ok.. no luck yet. scan the entire obj list   */
  for (i = object_list; i && (j <= number); i = i->next)
    if (isname(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i)) {
	if (j == number)
	  return(i);
	j++;
      }
  if (count) *count = j;
  return(0);
}

/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
  struct obj_data *i;
  
  /* scan items carried */
  if (i = get_obj_in_list_vis(ch, name, ch->carrying))
    return(i);
  
  /* scan room */
  if (i = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents))
    return(i);
  
  return get_obj_vis_world(ch, name, NULL);
}

struct obj_data *get_obj_vis_accessible(struct char_data *ch, char *name)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  /* scan items carried */
  for (i = ch->carrying, j=1; i && j<=number; i = i->next_content)
    if (isname(tmp, i->name) && CAN_SEE_OBJ(ch, i))
      if (j == number)
	return(i);
      else
	j++;
  for (i = real_roomp(ch->in_room)->contents; i && j<=number; i = i->next_content)
    if (isname(tmp, i->name) && CAN_SEE_OBJ(ch, i))
      if (j==number)
	return(i);
      else
	j++;
  return 0;
}




struct obj_data *create_money( int amount )
{
  struct obj_data *obj;
  struct extra_descr_data *new_descr;
  char buf[80];
  
  
  if(amount<=0) {
      log("ERROR: Try to create negative money.");
      exit(1);
    }
  
  CREATE(obj, struct obj_data, 1);
  CREATE(new_descr, struct extra_descr_data, 1);
  clear_object(obj);
  
  if(amount==1){
      obj->name = strdup("coin gold");
      obj->short_description = strdup("a gold coin");
      obj->description = strdup("One miserable gold coin.");
      
      new_descr->keyword = strdup("coin gold");
      new_descr->description = strdup("One miserable gold coin.");
    }  else {
      obj->name = strdup("coins gold");
      obj->short_description = strdup("gold coins");
      obj->description = strdup("A pile of gold coins.");
      
      new_descr->keyword = strdup("coins gold");
      if(amount<10) {
	sprintf(buf,"There is %d coins.",amount);
	new_descr->description = strdup(buf);
      } 
      else if (amount<100) {
	sprintf(buf,"There is about %d coins",10*(amount/10));
	new_descr->description = strdup(buf);
      }
      else if (amount<1000) {
	sprintf(buf,"It looks like something round %d coins",100*(amount/100));
	new_descr->description = strdup(buf);
      }
      else if (amount<100000) {
	sprintf(buf,"You guess there is %d coins",1000*((amount/1000)+ number(0,(amount/1000))));
	new_descr->description = strdup(buf);
      }
      else 
	new_descr->description = strdup("There is A LOT of coins");			
    }
  
  new_descr->next = 0;
  obj->ex_description = new_descr;
  
  obj->obj_flags.type_flag = ITEM_MONEY;
  obj->obj_flags.wear_flags = ITEM_TAKE;
  obj->obj_flags.decay_time = -1;
  obj->obj_flags.value[0] = amount;
  obj->obj_flags.cost = amount;
  obj->item_number = -1;
  
  obj->next = object_list;
  object_list = obj;
  
  return(obj);
}



/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(char *arg, int bitvector, struct char_data *ch,
		 struct char_data **tar_ch, struct obj_data **tar_obj)
{
  static char *ignore[] = {
    "the",
    "in",
    "on",
    "at",
    "\n" };
  
  int i;
  char name[256];
  bool found;
  
  found = FALSE;
  
  
  /* Eliminate spaces and "ignore" words */
  while (*arg && !found) {
    
    for(; *arg == ' '; arg++)   ;
    
    for(i=0; (name[i] = *(arg+i)) && (name[i]!=' '); i++)   ;
    name[i] = 0;
    arg+=i;
    if (search_block(name, ignore, TRUE) > -1)
      found = TRUE;
    
  }
  
  if (!name[0])
    return(0);
  
  *tar_ch  = 0;
  *tar_obj = 0;
  
  if (IS_SET(bitvector, FIND_CHAR_ROOM)) {      /* Find person in room */
    if (*tar_ch = get_char_room_vis(ch, name)) {
      return(FIND_CHAR_ROOM);
    }
  }
  
  if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
    if (*tar_ch = get_char_vis(ch, name)) {
      return(FIND_CHAR_WORLD);
    }
  }
  
  if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
    for(found=FALSE, i=0; i<MAX_WEAR && !found; i++)
      if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0) {
	*tar_obj = ch->equipment[i];
	found = TRUE;
      }
    if (found) {
      return(FIND_OBJ_EQUIP);
    }
  }
  
  if (IS_SET(bitvector, FIND_OBJ_INV)) {
    if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
      if (*tar_obj = get_obj_vis_accessible(ch, name)) {
	return(FIND_OBJ_INV);
      }
    } else {
      if (*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)) {
	return(FIND_OBJ_INV);
      }
    }
  }
  
  if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
    if (*tar_obj = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents)) {
      return(FIND_OBJ_ROOM);
    }
  }
  
  if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
    if (*tar_obj = get_obj_vis(ch, name)) {
      return(FIND_OBJ_WORLD);
    }
  }
  
  return(0);
}
