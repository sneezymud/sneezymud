/* ************************************************************************
*  file: reception.c, Special module for Inn's.           Part of DIKUMUD *
*  Usage: Procedures handling saving/loading of player objects            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <sys/time.h>

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"

#define OBJ_SAVE_FILE "pcobjs.obj"
#define OBJ_FILE_FREE "\0\0\0"

extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern int top_of_objt;
extern struct player_index_element *player_table;
extern int top_of_p_table;

/* Extern functions */

void store_to_char(struct char_file_u *st, struct char_data *ch);
void do_tell(struct char_data *ch, char *argument, int cmd);
int str_cmp(char *arg1, char *arg2);
void clear_char(struct char_data *ch);
void zero_rent(struct char_data *ch);
void PrintLimitedItems(void);
void CountLimitedItems(struct obj_file_u *st);
char *lower(char *s);
void ZeroRent( char *n);




/* ************************************************************************
* Routines used for the "Offer"                                           *
************************************************************************* */

void add_obj_cost(struct char_data *ch, struct char_data *re,
                  struct obj_data *obj, struct obj_cost *cost)
{
  char buf[MAX_INPUT_LENGTH];
  int  temp;
  
  /* Add cost for an item and it's contents, and next->contents */
  
  if (obj) {
    if ((obj->item_number > -1) && (cost->ok)) {
      cost->no_carried++;
         if ((IS_OBJ_STAT(obj,ITEM_LEVEL10) && GetMaxLevel(ch) < 10) ||
             (IS_OBJ_STAT(obj,ITEM_LEVEL20) && GetMaxLevel(ch) < 20) ||
             (IS_OBJ_STAT(obj,ITEM_LEVEL30) && GetMaxLevel(ch) < 30)) {
           if (re) {
                act("$n tells you 'You are too lowly to rent with $p'",
                       FALSE,re,obj,ch,TO_VICT);
            }
         }
    } else
      if (cost->ok) {
	if (re) {
	  act("$n tells you 'I refuse storing $p'",FALSE,re,obj,ch,TO_VICT);
	} else {
#if NODUPLICATES
#else
	  act("Sorry, but $p don't keep in storage.",FALSE,ch,obj,0,TO_CHAR);
#endif
	}
      }
  }
}


bool recep_offer(struct char_data *ch,	struct char_data *receptionist,
		 struct obj_cost *cost)
{
  int i;
  char buf[MAX_INPUT_LENGTH];
  
  cost->total_cost = 100; /* Minimum cost */
  cost->no_carried = 0;
  cost->ok = TRUE; /* Use if any "-1" objects */

  if (!cost->ok)
    return(FALSE);
  
  if (cost->no_carried > MAX_OBJ_SAVE) {
    if (receptionist) {
      sprintf(buf,"$n tells you 'Sorry, but I can't store more than %d items.",
	      MAX_OBJ_SAVE);
      act(buf,FALSE,receptionist,0,ch,TO_VICT);
    }
    return(FALSE);
  }
	cost->total_cost = 0;
  
  if ( cost->total_cost > GET_GOLD(ch) )
    return(FALSE);
  else
    return(TRUE);
}


/* ************************************************************************
* General save/load routines                                              *
************************************************************************* */

void update_file(struct char_data *ch, struct obj_file_u *st, int save)
{
  FILE *fl;
  int loc, t;
  struct obj_file_u tmp;
  char buf[200];

  sprintf(buf, "rent/%s", lower(ch->player.name));

  if (!(fl = fopen(buf, "w")))  {
       perror("saving PC's objects");
       exit(1);  
  }
  
  rewind(fl);

  strcpy(st->owner, GET_NAME(ch));

  WriteObjs(fl, st, save);

  fclose(fl);
  
}


/* ************************************************************************
* Routines used to load a characters equipment from disk                  *
************************************************************************* */

void obj_store_to_char(struct char_data *ch, struct obj_file_u *st)
{
  struct obj_data *obj;
  char buf[256];
  int i, j;
  
  void obj_to_char(struct obj_data *object, struct char_data *ch);
  
  for(i=0; i<st->number; i++) {
     if (st->objects[i].item_number > -1 && 
	real_object(st->objects[i].item_number) > -1) {
      obj = read_object(st->objects[i].item_number, VIRTUAL);
      obj->obj_flags.value[0] = st->objects[i].value[0];
      obj->obj_flags.value[1] = st->objects[i].value[1];
      obj->obj_flags.value[2] = st->objects[i].value[2];
      obj->obj_flags.value[3] = st->objects[i].value[3];
      obj->obj_flags.extra_flags = st->objects[i].extra_flags;
      obj->obj_flags.weight      = st->objects[i].weight;
      obj->obj_flags.timer       = st->objects[i].timer;
      obj->obj_flags.bitvector   = st->objects[i].bitvector;
      obj->obj_flags.struct_points = st->objects[i].struct_points;
      obj->obj_flags.max_struct_points = st->objects[i].max_struct_points;
      obj->obj_flags.material_points = st->objects[i].material_points;
      obj->obj_flags.decay_time = st->objects[i].decay_time;

/*  new, saving names and descrips stuff */
      if (obj->name)
         free(obj->name);
      if (obj->short_description)
         free(obj->short_description);
      if (obj->description)
         free(obj->description);

      obj->name = (char *)malloc(strlen(st->objects[i].name)+1);
      obj->short_description = (char *)malloc(strlen(st->objects[i].sd)+1);
      obj->description = (char *)malloc(strlen(st->objects[i].desc)+1);

      strcpy(obj->name, st->objects[i].name);
      strcpy(obj->short_description, st->objects[i].sd);
      strcpy(obj->description, st->objects[i].desc);
/* end of new, possibly buggy stuff */
      
      for(j=0; j<MAX_OBJ_AFFECT; j++)
	obj->affected[j] = st->objects[i].affected[j];
      
      obj_to_char(obj,ch);
    }
  }
  sprintf(buf,"%s has [%d] items.", st->owner, st->number);
  vlog(buf);
}


void load_char_objs(struct char_data *ch)
{
  FILE *fl;
  int i, j, loc;
  bool found = FALSE;
  float timegold;
  struct obj_file_u st;
  char buf[200];
 
  sprintf(buf, "rent/%s", lower(ch->player.name));
  
  
  /* r+b is for Binary Reading/Writing */
  if (!(fl = fopen(buf, "r+b")))  {
    vlog("Char has no equipment");
    //fclose(fl);
    return;
  }
 
  rewind(fl);
 
  if (!ReadObjs(fl, &st)) {
    vlog("No objects found");
    //fclose(fl);
    return;
  }
 
  if (str_cmp(st.owner, GET_NAME(ch)) != 0) {
    vlog("Hmm.. bad item-file write. someone is losing thier objects");
    fclose(fl);
    return;
  }
 
/*
  if the character has been out for 12 real hours, they are fully healed
  upon re-entry.  if they stay out for 24 full hours, all affects are
  removed, including bad ones.
*/
 
    if (st.last_update + 12*SECS_PER_REAL_HOUR < time(0))
      RestoreChar(ch);
 
    if (st.last_update + 24*SECS_PER_REAL_HOUR < time(0))
      RemAllAffects(ch);
    
    if (ch->in_room == NOWHERE &&
        st.last_update + 6*SECS_PER_REAL_HOUR > time(0)) {
      found = TRUE;
    } else {
      char      buf[MAX_STRING_LENGTH];
      if (ch->in_room == NOWHERE)
        vlog("Char reconnecting after autorent");
      timegold = 0;
      found = TRUE;    
    }
 
  fclose(fl);
 
  if (found)
      obj_store_to_char(ch, &st);
  else {
    ZeroRent(GET_NAME(ch));
  }
  
  /* Save char, to avoid strange data if crashing */
  save_char(ch, AUTO_RENT);
  
}

/* ************************************************************************
* Routines used to save a characters equipment from disk                  *
************************************************************************* */

/* Puts object in store, at first item which has no -1 */
void put_obj_in_store(struct obj_data *obj, struct obj_file_u *st)
{
  int i, j;
  bool found = FALSE;
  struct obj_file_elem *oe;
  char buf[256];

  if (st->number>=MAX_OBJ_SAVE) {
    printf("holy shit, you want to rent more than %d items?!\n", st->number);
    return;
  }

  oe = st->objects + st->number;
  
  oe->item_number = obj_index[obj->item_number].virtual;
  oe->value[0] = obj->obj_flags.value[0];
  oe->value[1] = obj->obj_flags.value[1];
  oe->value[2] = obj->obj_flags.value[2];
  oe->value[3] = obj->obj_flags.value[3];
  
  oe->extra_flags = obj->obj_flags.extra_flags;
  oe->weight  = obj->obj_flags.weight;
  oe->volume = obj->obj_flags.volume;
  oe->timer  = obj->obj_flags.timer;
  oe->bitvector  = obj->obj_flags.bitvector;
  oe->struct_points = obj->obj_flags.struct_points;
  oe->max_struct_points = obj->obj_flags.max_struct_points;
  oe->decay_time = obj->obj_flags.decay_time;
  oe->material_points = obj->obj_flags.material_points;

/*  new, saving names and descrips stuff */
      if (obj->name)
         strcpy(oe->name, obj->name);
      else {
	sprintf(buf, "object %d has no name!", obj_index[obj->item_number].virtual);
	vlog(buf);
	
      }
	
      if (obj->short_description)
         strcpy(oe->sd, obj->short_description);
      else
	*oe->sd = '\0';
      if (obj->description)
         strcpy(oe->desc, obj->description);
      else 
	*oe->desc = '\0';

/* end of new, possibly buggy stuff */


  for(j=0; j<MAX_OBJ_AFFECT; j++)
    oe->affected[j] = obj->affected[j];

  st->number++;
}

static int contained_weight(struct obj_data *container)
{
  struct obj_data *tmp;
  int	rval = 0;

  for (tmp = container->contains; tmp; tmp = tmp->next_content)
    rval += GET_OBJ_WEIGHT(tmp);
  return rval;
}

/* Destroy inventory after transferring it to "store inventory" */
void obj_to_store(struct obj_data *obj, struct obj_file_u *st,
                  struct char_data * ch, int delete)
{
  static char buf[240];
  
  if (!obj)
    return;

  obj_to_store(obj->contains, st, ch, delete);
  obj_to_store(obj->next_content, st, ch, delete);
    
  if ((obj->obj_flags.timer < 0) && (obj->obj_flags.timer != OBJ_NOTIMER)) {
#if NODUPLICATES
#else
    sprintf(buf, "You're told: '%s is just old junk, I'll throw it away for you.'\n\r", obj->short_description);
    send_to_char(buf, ch);
#endif
  } else if (obj->obj_flags.cost_per_day < 0) {
#if NODUPLICATES
#else
    sprintf(buf, "You're told: '%s is just old junk, I'll throw it away for you.'\n\r", obj->short_description);
    send_to_char(buf, ch);
#endif
    if (delete) {
       if (obj->in_obj) 
	 obj_from_obj(obj);
       extract_obj(obj);
     }
  } else if (obj->item_number == -1) {
    if (delete) {
       if (obj->in_obj) 
	 obj_from_obj(obj);
       extract_obj(obj);
     }
  }else {
    int weight = contained_weight(obj);
          GET_OBJ_WEIGHT(obj) -= weight;
    put_obj_in_store(obj, st);
    GET_OBJ_WEIGHT(obj) += weight;
    if (delete) {
      if (obj->in_obj)
	obj_from_obj(obj);
      extract_obj(obj);
    }
  }
}



/* write the vital data of a player to the player file */
void save_obj(struct char_data *ch, struct obj_cost *cost, int delete)
{
  static struct obj_file_u st;
  FILE *fl;
  int pos, i, j;
  bool found = FALSE;
    
  st.number = 0;
  st.gold_left = GET_GOLD(ch);
  st.total_cost = cost->total_cost;
  st.last_update = time(0);
  st.minimum_stay = 0; /* XXX where does this belong? */
  
  for(i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i]) {
      if (delete) {
	 obj_to_store(unequip_char(ch, i), &st, ch, delete);
      } else {
         obj_to_store(ch->equipment[i], &st, ch, delete);
      }
    }
  
  obj_to_store(ch->carrying, &st, ch, delete);
  if (delete)
     ch->carrying = 0;

  update_file(ch, &st, 1);
  
}


/* write the vital data of a player to the player file */
void save_obj_for_save(struct char_data *ch, struct obj_cost *cost, int delete)
{
  static struct obj_file_u st;
  FILE *fl;
  int pos, i, j;
  bool found = FALSE;

  st.number = 0;
  st.gold_left = GET_GOLD(ch);
  st.total_cost = cost->total_cost;
  st.last_update = time(0);
  st.minimum_stay = 0; /* XXX where does this belong? */

  for(i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i]) {
      if (delete) {
         obj_to_store(unequip_char(ch, i), &st, ch, delete);
      } else {
         obj_to_store(ch->equipment[i], &st, ch, delete);
      }
    }

  obj_to_store(ch->carrying, &st, ch, delete);
  if (delete)
     ch->carrying = 0;

  update_file(ch, &st, 0);

}

/* ************************************************************************
* Routines used to update object file, upon boot time                     *
************************************************************************* */

void update_obj_file(void)
{
  FILE *fl, *char_file;
  struct obj_file_u st;
  struct char_file_u ch_st;
  struct char_data tmp_char;
  int pos, no_read, i, cost_per_day;
  long days_passed, secs_lost;
  char buf[MAX_INPUT_LENGTH];
  struct obj_file_u *lim;
  struct obj_data *obj;
  
  int find_name(char *name);
  extern int errno;
  
  if (!(char_file = fopen(PLAYER_FILE, "r+"))) {
    perror("Opening player file for reading. (reception.c, update_obj_file)");
    exit(1);
  }
  
  
  for (i=0; i<= top_of_p_table; i++) {
    sprintf(buf, "rent/%s", player_table[i].name);
    /* r+b is for Binary Reading/Writing */
    if ((fl = fopen(buf, "r+b")) != NULL) {
      if (ReadObjs(fl, &st)) {
	if (str_cmp(st.owner, player_table[i].name) != 0) {
	  vlog("Ack!  wrong person written into object file!");
	  abort();
	} else {
	  sprintf(buf, "   Processing %s[%d].", st.owner, i);
	  vlog(buf);
	  days_passed = ((time(0) - st.last_update) / SECS_PER_REAL_DAY);
	  secs_lost = ((time(0) - st.last_update) % SECS_PER_REAL_DAY);
	  
	  fseek(char_file, (long) (player_table[i].nr *
				   sizeof(struct char_file_u)), 0);
	  fread(&ch_st, sizeof(struct char_file_u), 1, char_file);
	 
          if (ch_st.load_room == AUTO_RENT) {  /* this person was autorented */
	    ch_st.load_room = NOWHERE;
	    st.last_update = time(0);

#if LIMITED_ITEMS
	      CountLimitedItems(&st);
#endif
             fseek(char_file, (long) (player_table[i].nr *
					 sizeof(struct char_file_u)), 0);
	     fwrite(&ch_st, sizeof(struct char_file_u), 1, char_file);

	    fclose(fl);
	  } else {
 
	    if (days_passed > 0) {
	      
	      if ((st.total_cost*days_passed) > st.gold_left) {
		
		sprintf(buf, "   Dumping %s from object file.", ch_st.name);
		vlog(buf);
		
		ch_st.points.gold = 0;
		ch_st.load_room = NOWHERE;
		fseek(char_file, (long) (player_table[i].nr *
					 sizeof(struct char_file_u)), 0);
		fwrite(&ch_st, sizeof(struct char_file_u), 1, char_file);
		
		fclose(fl);
		ZeroRent(ch_st.name);
		
	      } else {
		
		sprintf(buf, "   Updating %s", st.owner);
		vlog(buf);
		st.gold_left  -= (st.total_cost*days_passed);
		st.last_update = time(0)-secs_lost;
		rewind(fl);
		WriteObjs(fl, &st, 0);
		fclose(fl);
#if LIMITED_ITEMS
		CountLimitedItems(&st);
#endif
		
	      }
	    } else {
	      
#if LIMITED_ITEMS
	      CountLimitedItems(&st);
#endif
	      fclose(fl);
	    }
	  }
	}
      }
    } else {
    }
  }
  fclose(char_file);
}


void CountLimitedItems(struct obj_file_u *st)
{
    int i, cost_per_day;
    struct obj_data *obj;

    if (!st->owner[0]) return;  /* don't count empty rent units */

    for(i=0; i<st->number; i++) {
      if (st->objects[i].item_number > -1 && 
          real_object(st->objects[i].item_number) > -1) {
	    /*
            ** eek.. read in the object, and then extract it.
	    ** (all this just to find rent cost.)  *sigh*
            */
            obj = read_object(st->objects[i].item_number, VIRTUAL);
	    cost_per_day = obj->obj_flags.cost_per_day;
	    /*
            **  if the cost is > LIM_ITEM_COST_MIN, then mark before extractin
            */
	    if (cost_per_day > LIM_ITEM_COST_MIN) {
	      obj_index[obj->item_number].number++;  
	    }
	    extract_obj(obj);
	}
    }
}


void PrintLimitedItems(void)
{
  int i;
  char buf[200];

  for (i=0;i<=top_of_objt;i++) {
    if (obj_index[i].number > 0) {
      sprintf(buf, "item> %d [%d]", obj_index[i].virtual, obj_index[i].number);
      vlog(buf);
    }
  }
}


/* ************************************************************************
* Routine Receptionist                                                    *
************************************************************************* */



int receptionist(struct char_data *ch, int cmd, char *arg)
{
  char buf[240];
  struct obj_cost cost;
  struct char_data *recep = 0;
  struct char_data *temp_char;
  sh_int save_room;
  sh_int action_tabel[9] = {23,24,36,105,106,109,111,142,147};
  
  void do_action(struct char_data *ch, char *argument, int cmd);
  int number(int from, int to);
  int citizen(struct char_data *ch, int cmd, char *arg);
  
  if (!ch->desc)
    return(FALSE); /* You've forgot FALSE - NPC couldn't leave */
  
  for (temp_char = real_roomp(ch->in_room)->people; (temp_char) && (!recep);
       temp_char = temp_char->next_in_room)
    if (IS_MOB(temp_char))
      if (mob_index[temp_char->nr].func == receptionist)
	recep = temp_char;
  
  if (!recep) {
    vlog("No receptionist.\n\r");
    exit(1);
  }
  
  if (IS_NPC(ch))
    return(FALSE);
  
  if ((cmd != 92) && (cmd != 93)) {
    if (!cmd) {
      if (recep->specials.fighting) {
	return(citizen(recep,0,""));
      }
    }
    if (!number(0, 30))
      do_action(recep, "", action_tabel[number(0,8)]);
    return(FALSE);
  }
  
  if (!AWAKE(recep)) {
    act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
    return(TRUE);
  }

  if (IS_SET(ch->specials.act, PLR_KILLER) ||
     (IS_SET(ch->specials.act, PLR_OUTLAW))) {
      sprintf(buf,"$n tells you 'Sorry, but we don't harbor criminals.");
      act(buf,FALSE,recep,0,ch,TO_VICT);
     return(TRUE);
   }
  
  
  if (!CAN_SEE(recep, ch)) 
    {
      act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
      return(TRUE);
    }
  
  if (cmd == 92) { /* Rent  */
    if (recep_offer(ch, recep, &cost)) {
      
      act("$n stores your stuff in the safe, and helps you into your chamber.",
	  FALSE, recep, 0, ch, TO_VICT);
      act("$n helps $N into $S private chamber.",FALSE, recep,0,ch,TO_NOTVICT);
      
      save_obj(ch, &cost,1);
      save_room = ch->in_room;
      extract_char(ch);
      ch->in_room = save_room;
      save_char(ch, ch->in_room);
    }
    
  } else {         /* Offer */
    recep_offer(ch, recep, &cost);
    act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
  }
  
  return(TRUE);
}

int receptionist_for_outlaws(struct char_data *ch, int cmd, char *arg)
{
  char buf[240];
  struct obj_cost cost;
  struct char_data *recep = 0;
  struct char_data *temp_char;
  sh_int save_room;
  sh_int action_tabel[9] = {23,24,36,105,106,109,111,142,147};
  
  void do_action(struct char_data *ch, char *argument, int cmd);
  int number(int from, int to);
  int citizen(struct char_data *ch, int cmd, char *arg);
  
  if (!ch->desc)
    return(FALSE); /* You've forgot FALSE - NPC couldn't leave */
  
  for (temp_char = real_roomp(ch->in_room)->people; (temp_char) && (!recep);
       temp_char = temp_char->next_in_room)
    if (IS_MOB(temp_char))
      if (mob_index[temp_char->nr].func == receptionist_for_outlaws)
        recep = temp_char;
  
  if (!recep) {
    vlog("No receptionist.\n\r");
    exit(1);
  }
  
  if (IS_NPC(ch))
    return(FALSE);
  
  if ((cmd != 92) && (cmd != 93)) {
    if (!cmd) {
      if (recep->specials.fighting) {
        return(citizen(recep,0,""));
      }
    }
    if (!number(0, 30))
      do_action(recep, "", action_tabel[number(0,8)]);
    return(FALSE);
  }

  if (!AWAKE(recep)) {
    act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
    return(TRUE);
  }
 
  if (!CAN_SEE(recep, ch)) 
    {
      act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0
 
, TO_ROOM);
      return(TRUE);
    }
  
  if (cmd == 92) { /* Rent  */
    if (recep_offer(ch, recep, &cost)) {
      if (IS_SET(ch->specials.act, PLR_KILLER) ||
         (IS_SET(ch->specials.act, PLR_OUTLAW))) {
      sprintf(buf,"$n tells you 'Hurry, before the cops catch you!'");
      act(buf,FALSE,recep,0,ch,TO_VICT);
     }       
      act("$n stores your stuff in the safe, and helps you into your chamber.",
          FALSE, recep, 0, ch, TO_VICT);
      act("$n helps $N into $S private chamber.",FALSE, recep,0,ch,TO_NOTVICT);
      
      save_obj(ch, &cost,1);
      save_room = ch->in_room;
      extract_char(ch);
      ch->in_room = save_room;
      save_char(ch, ch->in_room);
    }
    
  } else {         /* Offer */
    recep_offer(ch, recep, &cost);
    act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
  }
  
  return(TRUE);
}


/*
    removes a player from the list of renters
*/

void zero_rent( struct char_data *ch) 
{

  if (IS_NPC(ch))
    return;

  ZeroRent(GET_NAME(ch));

}

void ZeroRent( char *n)
{
  FILE *fl;
  char buf[200];

  sprintf(buf, "rent/%s", lower(n));

  if (!(fl = fopen(buf, "w"))) {
    perror("saving PC's objects");
    exit(1);
  }
  
  fclose(fl);
  return;
  
}

int ReadObjs( FILE *fl, struct obj_file_u *st)
{
  int i;

  if (feof(fl)) {
    puts("Ending at 1.");
    fclose(fl);
    return(FALSE);
  }
  fread(&st->owner, sizeof(st->owner), 1, fl);
  if (feof(fl)) {
    puts("Ending at 2.");
    fclose(fl);
    return(FALSE);
  }
  fread(&st->gold_left, sizeof(st->gold_left), 1, fl);
  if (feof(fl)) {
    puts("Ending at 3.");
    fclose(fl);
    return(FALSE);
  }
  fread(&st->total_cost, sizeof(st->total_cost), 1, fl);
  if (feof(fl)) {
    puts("Ending at 4.");
    fclose(fl);
    return(FALSE);
  }
  fread(&st->last_update, sizeof(st->last_update), 1, fl);
  if (feof(fl)) {
    puts("Ending at 5.");
    fclose(fl);
    return(FALSE);
  }
  fread(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
  if (feof(fl)) {
    puts("Ending at 6.");
    fclose(fl);
    return(FALSE);
  }
  fread(&st->number, sizeof(st->number), 1, fl);
  if (feof(fl)) {
    puts("Ending at 7.");
    fclose(fl);
    return(FALSE);
  }
   
  for (i=0;i<st->number;i++) {
   fread(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
/*
   printf("%d [%d %d %d %d] %d %d %d %d\r\n",
	  st->objects[i].item_number,
	  st->objects[i].value[0],
	  st->objects[i].value[1],
	  st->objects[i].value[2],
	  st->objects[i].value[3],
	  st->objects[i].extra_flags,
	  st->objects[i].weight,
	  st->objects[i].timer,
	  st->objects[i].bitvector);
   printf("%s %s %s.\r\n",st->objects[i].name,st->objects[i].sd,st->objects[i].desc);
  
*/ 
  }
}

int WriteObjs( FILE *fl, struct obj_file_u *st, int save)
{
  int i;
  char buf[80];

  fwrite(&st->owner, sizeof(st->owner), 1, fl);
  fwrite(&st->gold_left, sizeof(st->gold_left), 1, fl);
  fwrite(&st->total_cost, sizeof(st->total_cost), 1, fl);
  fwrite(&st->last_update, sizeof(st->last_update), 1, fl);
  fwrite(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
  fwrite(&st->number, sizeof(st->number), 1, fl);
   
  for (i=0;i<st->number;i++) {
     fwrite(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
  }
  if (save==1) 
  {
    sprintf(buf,"%s rented out with [%d] items.", st->owner, st->number);
    vlog(buf);
  }
}
