/************************************************************************
*  file: act.obj1.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands mainly moving around objects.                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "trap.h"

/* extern variables */

extern struct str_app_type str_app[];
extern struct dex_skill_type dex_app_skill[];
extern struct descriptor_data *descriptor_list;
extern int vol_mult[];
		 
/* extern functions */

struct obj_data *create_money( int amount );
struct room_data *world;
char getall(char *name, char *newname);
int getabunch(char *name, char  *newname);


/* procedures related to get */
void get(struct char_data *ch, struct obj_data *obj_object, 
	struct obj_data *sub_object) 
{
   char buffer[256];

   if (sub_object) {
      if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
		obj_from_obj(obj_object);
		obj_to_char(obj_object, ch);
       		act("You get $p from $P.",0,ch,obj_object,sub_object,TO_CHAR);
       	        act("$n gets $p from $P.",1,ch,obj_object,sub_object,TO_ROOM);
      } else {
		act("$P must be opened first.",1,ch,0,sub_object,TO_CHAR);
		return;
      }
   } else {
/*  jdb -- 11-9 */
        if (obj_object->in_room == NOWHERE) {
	  obj_object->in_room = ch->in_room;
	}

       	obj_from_room(obj_object);
       	obj_to_char(obj_object, ch);
       	act("You get $p.", 0, ch, obj_object, 0, TO_CHAR);
       	act("$n gets $p.", 1, ch, obj_object, 0, TO_ROOM);
   }
	if((obj_object->obj_flags.type_flag == ITEM_MONEY) && 
		(obj_object->obj_flags.value[0]>=1))
	{
		obj_from_char(obj_object);
		sprintf(buffer,"There was %d coins.\n\r", obj_object->obj_flags.value[0]);
		send_to_char(buffer,ch);
		GET_GOLD(ch) += obj_object->obj_flags.value[0];
                if (GET_GOLD(ch) > 500000 && obj_object->obj_flags.value[0] > 100000) {
                   char buf[MAX_INPUT_LENGTH];
                   sprintf(buf,"%s just got %d coins",
			   GET_NAME(ch),obj_object->obj_flags.value[0]);
		   log(buf);
		}
		extract_obj(obj_object);
	}
}


void do_get(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *sub_object;
  struct obj_data *obj_object;
  struct obj_data *next_obj;
  bool found = FALSE;
  bool fail  = FALSE;
  int type   = 3;
  char newarg[100];
  int num, p;
  
  argument_interpreter(argument, arg1, arg2);
  
  /* get type */
  if (!*arg1) {
    type = 0;
  }
  if (*arg1 && !*arg2) {
    if (!str_cmp(arg1,"all")) {
      type = 1;
    } else {
      type = 2;
    }
  }
  if (*arg1 && *arg2) {
    if (!str_cmp(arg1,"all")) {
      if (!str_cmp(arg2,"all")) {
	type = 3;
      } else {
	type = 4;
      }
    } else {
      if (!str_cmp(arg2,"all")) {
	type = 5;
      } else {
	type = 6;
      }
    }
  }
  
  switch (type) {
    /* get */
  case 0:{ 
    send_to_char("Get what?\n\r", ch); 
  } break;
    /* get all */
  case 1:{ 
    sub_object = 0;
    found = FALSE;
    fail	= FALSE;
    for(obj_object = real_roomp(ch->in_room)->contents;
	obj_object;
	obj_object = next_obj) {
      next_obj = obj_object->next_content;
      /*
	check for a trap (traps fire often)
	*/
      if (CheckForAnyTrap(ch, obj_object)) 
	return;
      if (CAN_SEE_OBJ(ch,obj_object)) {
	if ((IS_CARRYING_N(ch) + obj_object->obj_flags.volume) <=
            CAN_CARRY_N(ch)) {
	  if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <= 
	      CAN_CARRY_W(ch)) {
	    if (CAN_WEAR(obj_object,ITEM_TAKE)) {
             if (ObjLevelCheck(obj_object,ch)) {
	      get(ch,obj_object,sub_object);
	      found = TRUE;
             } else {
              act("You wouldnt know how to use the $o if you took it!",
                 FALSE,ch,obj_object,0,TO_CHAR);
              fail = TRUE;
              }
	    } else {
	      send_to_char("You can't take that\n\r", ch);
	      fail = TRUE;
	    }
	  } else {
	    sprintf(buffer,"%s : You can't carry that much weight.\n\r", 
		    obj_object->short_description);
	    send_to_char(buffer, ch);
	    fail = TRUE;
	  }
	} else {
	  sprintf(buffer,"%s : You can't carry that much volume.\n\r", obj_object->short_description);
	  send_to_char(buffer, ch);
	  fail = TRUE;
	}
      }
    }
    if (found) {
      send_to_char("OK.\n\r", ch);
    } else {
      if (!fail) send_to_char("You see nothing here.\n\r", ch);
    }
  } break;
    /* get ??? (something) */
  case 2:{
    sub_object = 0;
    found = FALSE;
    fail	= FALSE;
    if (getall(arg1,newarg)) {
      strcpy(arg1,newarg);
      num = -1;
    } else if ((p = getabunch(arg1,newarg))) {
      strcpy(arg1,newarg);
      num = p;
    } else {
      num = 1;
    }
    
    while (num != 0) {
      obj_object = get_obj_in_list_vis(ch, arg1, 
				       real_roomp(ch->in_room)->contents);
      if (obj_object) {
	if ((IS_CARRYING_N(ch) + obj_object->obj_flags.volume) < 
            CAN_CARRY_N(ch)) {
	  if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < 
	      CAN_CARRY_W(ch)) {
	    if (CAN_WEAR(obj_object,ITEM_TAKE)) {
             if (ObjLevelCheck(obj_object,ch)) {
	      get(ch,obj_object,sub_object);
	      found = TRUE;
             } else {
              act("You wouldn't know how to use the $o if you took it!",
                  FALSE,ch,obj_object,0,TO_CHAR);
              fail = TRUE;
              num = 0;
             }
	    } else {
	      send_to_char("You can't take that\n\r", ch);
	      fail = TRUE;
	      num = 0;
	    }
	  } else {
	    sprintf(buffer,"%s : You can't carry that much weight.\n\r", 
		    obj_object->short_description);
	    send_to_char(buffer, ch);
	    fail = TRUE;
	    num = 0;
	  }
	} else {
	  sprintf(buffer,"%s : You can't carry that much volume\n\r", 
		  obj_object->short_description);
	  send_to_char(buffer, ch);
	  fail = TRUE;
	  num = 0;
	}
      } else {
	if (num > 0) {
	  sprintf(buffer,"You do not see a %s here.\n\r", arg1);
	  send_to_char(buffer, ch);
	}
	num = 0;
	fail = TRUE;
      }
      if (num > 0) num--;
    }
  } break;
    /* get all all */
  case 3:{ 
    send_to_char("You must be joking?!\n\r", ch);
  } break;
    /* get all ??? */
  case 4:{
    found = FALSE;
    fail	= FALSE; 
    sub_object = (struct obj_data *) 
      get_obj_vis_accessible(ch, arg2);
    if (sub_object) {
      if (GET_ITEM_TYPE(sub_object)==ITEM_CONTAINER){
	for(obj_object = sub_object->contains;
	    obj_object;
	    obj_object = next_obj) {
	  /* check for trap (jdb - 11/9) */
	  if (CheckForGetTrap(ch, obj_object))
	    return;
	  next_obj = obj_object->next_content;
	  if (CAN_SEE_OBJ(ch,obj_object)) {
	    if ((IS_CARRYING_N(ch) + (obj_object->obj_flags.volume)) < 
		 CAN_CARRY_N(ch)) {
	      if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < 
		  CAN_CARRY_W(ch)) {
		if (CAN_WEAR(obj_object,ITEM_TAKE)) {
                 if (ObjLevelCheck(obj_object,ch)) {
		  get(ch,obj_object,sub_object);
		  found = TRUE;
                 } else {
                   act("You wouldn't know how to use the $o if you took it!",
                       FALSE,ch,obj_object,0,TO_CHAR);
                   fail = TRUE;
                 }
		} else {
		  send_to_char("You can't take that\n\r", ch);
		  fail = TRUE;
		}
	      } else {
		sprintf(buffer,"%s : You can't carry that much weight.\n\r", 
			obj_object->short_description);
		send_to_char(buffer, ch);
		fail = TRUE;
	      }
	    } else {
	      sprintf(buffer,"%s : You can't carry that much volume\n\r", 
		      obj_object->short_description);
	      send_to_char(buffer, ch);
	      fail = TRUE;
	    }
	  }
	}
	if (!found && !fail) {
	  sprintf(buffer,"You do not see anything in %s.\n\r", 
		  sub_object->short_description);
	  send_to_char(buffer, ch);
	  fail = TRUE;
	}
      } else {
	sprintf(buffer,"%s is not a container.\n\r",
		sub_object->short_description);
	send_to_char(buffer, ch);
	fail = TRUE;
      }
    } else { 
      sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
      send_to_char(buffer, ch);
      fail = TRUE;
    }
  } break;
  case 5:{ 
    send_to_char("You can't take a thing from more than one container.\n\r", 
		 ch);
  } break;
    /*  
      take ??? from ???   (is it??) 
      */
    
  case 6:{
    found = FALSE;
    fail	= FALSE;
    sub_object = (struct obj_data *)
      get_obj_vis_accessible(ch, arg2);
    if (sub_object) {
      if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
	
	if (getall(arg1,newarg)) {
	  num = -1;
	  strcpy(arg1,newarg);
	} else if ((p = getabunch(arg1,newarg))) {
	  num = p;                     
	  strcpy(arg1,newarg);
	} else {
	  num = 1;
	}
	
	while (num != 0) {
	  
	  obj_object = get_obj_in_list_vis(ch, arg1, 
					   sub_object->contains);
	  if (obj_object) {
	    /* check for trap (jdb - 11/9) */					
	    if (CheckForInsideTrap(ch, sub_object))
	      return;
	    if ((IS_CARRYING_N(ch) + obj_object->obj_flags.volume) <
                CAN_CARRY_N(ch)) {
	      if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < 
		  CAN_CARRY_W(ch)) {
		if (CAN_WEAR(obj_object,ITEM_TAKE)) {
                 if (ObjLevelCheck(obj_object,ch)) {
		  get(ch,obj_object,sub_object);
		  found = TRUE;
                 } else {
                  act("You wouldn't know how to use the $o if you took it!",
                      FALSE,ch,obj_object,0,TO_CHAR);
                  fail = TRUE;
                  num = 0;
                 }
		} else {
		  send_to_char("You can't take that\n\r", ch);
		  fail = TRUE;
		  num = 0;
		}
	      } else {
		sprintf(buffer,"%s : You can't carry that much weight.\n\r", 
			obj_object->short_description);
		send_to_char(buffer, ch);
		fail = TRUE;
		num = 0;
	      }
	    } else {
	      sprintf(buffer,"%s : You can't carry that much volume\n\r", 
		      obj_object->short_description);
	      send_to_char(buffer, ch);
	      fail = TRUE;
	      num = 0;
	    }
	  } else {
	    if (num > 0) {
	      sprintf(buffer,"%s does not contain the %s.\n\r",sub_object->short_description, arg1);
	      send_to_char(buffer, ch);
	    }
	    num = 0;
	    fail = TRUE;
	  }
	  
	  if (num > 0) num--;
	  
	}
      } else {
	sprintf(buffer,"%s is not a container.\n\r", sub_object->short_description);
	send_to_char(buffer, ch);
	fail = TRUE;
      }
    } else {
      sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
      send_to_char(buffer, ch);
      fail = TRUE;
    }
  } break;
  }
}


void do_drop(struct char_data *ch, char *argument, int cmd) 
{
  char arg[MAX_INPUT_LENGTH];
  int amount;
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *tmp_object;
  struct obj_data *next_obj;
  bool test = FALSE;
  char newarg[100];
  char *s;
  int num, p;
  
  s=one_argument(argument, arg);
  if(is_number(arg))	{
    amount = atoi(arg);
    strcpy(arg, s);
    
    if (0!=str_cmp("coins",arg) && 0!=str_cmp("coin",arg))  {
      send_to_char("Sorry, you can't do that (yet)...\n\r",ch);
      return;
    }
    if(amount<0)  	{
      send_to_char("Sorry, you can't do that!\n\r",ch);
      return;
    }
    if(GET_GOLD(ch)<amount)	 {
      send_to_char("You haven't got that many coins!\n\r",ch);
      return;
    }
    send_to_char("OK.\n\r",ch);
    if(amount==0)
      return;
    
    act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
    tmp_object = create_money(amount);
    obj_to_room(tmp_object,ch->in_room);
    GET_GOLD(ch)-=amount;
    return;
  } else {
    only_argument(argument, arg);
  }
  
  if (*arg) {
    if (!str_cmp(arg,"all")) {
      for(tmp_object = ch->carrying;
	  tmp_object;
	  tmp_object = next_obj) {
	next_obj = tmp_object->next_content;
	if (! IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
	  if (CAN_SEE_OBJ(ch, tmp_object)) {
	    sprintf(buffer, "You drop %s.\n\r", tmp_object->short_description);
	    send_to_char(buffer, ch);
	  } else {
	    send_to_char("You drop something.\n\r", ch);
	  }
	  act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
	  obj_from_char(tmp_object);
	  obj_to_room(tmp_object,ch->in_room);
	  test = TRUE;
	} else {
	  if (CAN_SEE_OBJ(ch, tmp_object)) {
	    sprintf(buffer, "You can't drop  %s, it must be CURSED!\n\r", tmp_object->short_description);
	    send_to_char(buffer, ch);
	    test = TRUE;
	  }
	}
      }
      if (!test) {
	send_to_char("You do not seem to have anything.\n\r", ch);
      }
#if   NODUPLICATES
      do_save(ch, "", 0);
#endif
    } else {
      /* &&&&&& */
      if (getall(arg,newarg)) {
	num = -1;
	strcpy(arg,newarg);
      } else if ((p = getabunch(arg,newarg))) {
	num = p;                     
	strcpy(arg,newarg);
      } else {
	num = 1;  
      }
      
      while (num != 0) {
	tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
	if (tmp_object) {
	  if (! IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
	    sprintf(buffer, "You drop %s.\n\r", tmp_object->short_description);
	    send_to_char(buffer, ch);
	    act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
	    obj_from_char(tmp_object);
	    obj_to_room(tmp_object,ch->in_room);
	  } else {
	    send_to_char("You can't drop it, it must be CURSED!\n\r", ch);
	    num = 0;
	  }
	} else {
	  if (num > 0)
	    send_to_char("You do not have that item.\n\r", ch);
	  
	  num = 0;
	}
	if (num > 0) num--;
      }
#if   NODUPLICATES
      do_save(ch, "", 0);
#endif
    }
  } else {
    send_to_char("Drop what?\n\r", ch);
  }
}



void do_put(struct char_data *ch, char *argument, int cmd)
{
  char buffer[256];
  char arg1[128];
  char arg2[128];
  struct obj_data *obj_object;
  struct obj_data *sub_object;
  struct obj_data *vol_object;
  struct obj_data *next_obj;
  struct char_data *tmp_char;
  int bits;
  int volume;
  char newarg[100];
  int num, p;
  
  argument_interpreter(argument, arg1, arg2);
  
  if (*arg1) {
    if (*arg2) {
      
      if (getall(arg1,newarg)) {
	num = -1;
	strcpy(arg1,newarg);
      } else if ((p = getabunch(arg1,newarg))) {
	num = p;                     
	strcpy(arg1,newarg);
      } else {
	num = 1;  
      }
      
      if (!strcmp(arg1,"all")) {
	
	send_to_char("sorry, you can't do that (yet)\n\r",ch);
	return;
	
      } else {
	while (num != 0) {
#if 1
	  bits = generic_find(arg1, FIND_OBJ_INV,
			      ch, &tmp_char, &obj_object);
#else
	  obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
#endif
	  
	  if (obj_object) {
	    if (IS_OBJ_STAT(obj_object,ITEM_NODROP)) {
	      send_to_char
		("You can't let go of it, it must be CURSED!\n\r", ch);
	      return;
	    }
            if (GET_ITEM_TYPE(obj_object) == ITEM_CONTAINER) {
                 send_to_char("Putting a bag in a bag can be hazardous for your health!\n\r", ch);
                 return;
            }
	    bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM,
				ch, &tmp_char, &sub_object);
	    if (sub_object) {
	      if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
		if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
		  if (obj_object == sub_object) {
		    send_to_char("You attempt to fold it into itself, but fail.\n\r", ch);
		    return;
		  }
               if (((sub_object->obj_flags.weight) + 
	            (obj_object->obj_flags.weight)) <
	            (sub_object->obj_flags.value[0])) {
		    if (bits==FIND_OBJ_INV) {
		      obj_from_char(obj_object);
		      /* make up for above line */
		      IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(obj_object);
		      obj_to_obj(obj_object, sub_object);
                 if (!IS_OBJ_STAT(sub_object, ITEM_HOLDING)) {
                  if (obj_object->obj_flags.type_flag != ITEM_CONTAINER) 
                    IS_CARRYING_N(ch) += (GET_OBJ_VOLUME(obj_object)/vol_mult[obj_object->obj_flags.material_points]);
                  else
                    IS_CARRYING_N(ch) += GET_OBJ_VOLUME(obj_object); 
                 }


                switch (obj_object->obj_flags.material_points) {
                 case 1:
                 case 2:
                 case 6:
                 case 54:
                 case 55: {
                   act("You fold $p into $P.",
                      TRUE,ch,obj_object,sub_object,TO_CHAR);
                   act("$n folds $p into $P.",
                      TRUE,ch,obj_object,sub_object,TO_ROOM);
                   }
                   break;
                 case 4:
                 case 61: {
                   act("You gently place $p in $P.",
                      TRUE,ch,obj_object,sub_object,TO_CHAR);
                   act("$n gently places $p in $P.",
                      TRUE,ch,obj_object,sub_object,TO_ROOM);
                   }
                   break;
                default : {
                   act("You pack $p into $P.",
                      TRUE,ch,obj_object,sub_object,TO_CHAR);
                   act("$n packs $s $p into $P,",
                      TRUE,ch,obj_object,sub_object,TO_ROOM);
                   }
                   break;
              }
                 
		    }
		    num--;
		  } else {
		    send_to_char("It won't fit.\n\r", ch);
		    num = 0;
		  }
		} else {
		  send_to_char("It seems to be closed.\n\r", ch);
		  num = 0;
		}
	      } else {
		sprintf(buffer,"%s is not a container.\n\r", sub_object->short_description);
		send_to_char(buffer, ch);
		num = 0;
	      }
	    } else {
	      sprintf(buffer, "You don't have the %s.\n\r", arg2);
	      send_to_char(buffer, ch);
	      num = 0;
	    }
	  } else {
	    if ((num > 0) || (num == -1)) { 
	      sprintf(buffer, "You don't have the %s.\n\r", arg1);
	      send_to_char(buffer, ch);
	    }
	    num = 0;
	  }
	}
      }
    } else {
      sprintf(buffer, "Put %s in what?\n\r", arg1);
      send_to_char(buffer, ch);
    }
  } else {
    send_to_char("Put what in what?\n\r",ch);
  }
}





void do_give(struct char_data *ch, char *argument, int cmd)
{
  char obj_name[80], vict_name[80], buf[132];
  char arg[80], newarg[100];
  int amount, num, p;
  struct char_data *vict;
  struct obj_data *obj;
  
  argument=one_argument(argument,obj_name);
  if(is_number(obj_name))	{
    amount = atoi(obj_name);
    argument=one_argument(argument, arg);
    if (str_cmp("coins",arg) && str_cmp("coin",arg))      	{
      send_to_char("Sorry, you can't do that (yet)...\n\r",ch);
      return;
    }
    if(amount<0)	{
      send_to_char("Sorry, you can't do that!\n\r",ch);
      return;
    }
    if((GET_GOLD(ch)<amount) && (IS_NPC(ch) || (GetMaxLevel(ch) < DEMIGOD))) {
      send_to_char("You haven't got that many coins!\n\r",ch);
      return;
    }

    argument=one_argument(argument, vict_name);

    if(!*vict_name)	{
      send_to_char("To who?\n\r",ch);
      return;
    }

    if (!(vict = get_char_room_vis(ch, vict_name)))	{
      send_to_char("To who?\n\r",ch);
      return;
    }

    send_to_char("Ok.\n\r",ch);
    sprintf(buf,"%s gives you %d gold coins.\n\r",PERS(ch,vict),amount);
    send_to_char(buf,vict);
    act("$n gives some gold to $N.", 1, ch, 0, vict, TO_NOTVICT);
    if (IS_NPC(ch) || (GetMaxLevel(ch) < DEMIGOD))
      GET_GOLD(ch)-=amount;
    GET_GOLD(vict)+=amount;
    save_char(ch, AUTO_RENT);
    if ((GET_GOLD(vict) > 500000) && (amount > 100000)) {
      sprintf(buf, "%s gave %d coins to %s", GET_NAME(ch), amount, GET_NAME(vict));
      log(buf);
    }
      
    return;
  } else {
    
    argument=one_argument(argument, vict_name);
    
    
    if (!*obj_name || !*vict_name)	{
      send_to_char("Give what to who?\n\r", ch);
      return;
    }
    /* &&&& */
    if (getall(obj_name,newarg)) {
      num = -1;
      strcpy(obj_name,newarg);
    } else if ((p = getabunch(obj_name,newarg))) {
      num = p;                     
      strcpy(obj_name,newarg);
    } else {
      num = 1;  
    }
    
    while (num != 0) {
      if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))	{
	if (num >= -1)
	  send_to_char("You do not seem to have anything like that.\n\r",
		       ch);
	return;
      }
      if ((IS_OBJ_STAT(obj, ITEM_NODROP)) && (!IS_IMMORTAL(ch))) {
	send_to_char
	  ("You can't let go of it, it must be CURSED!\n\r", ch);
        return;
      }
      if (!(vict = get_char_room_vis(ch, vict_name)))	{
	send_to_char("No one by that name around here.\n\r", ch);
	return;
      }
      if (vict == ch) {
	send_to_char("Ok.\n\r", ch);
	return;
      }
      if (!ObjLevelCheck(obj,vict)) {
        act("$N wouldn't know how to use the $o if you gave it to $M!",
            FALSE,ch,obj,vict,TO_CHAR);
        return;
      } 
      if ((obj->obj_flags.volume+IS_CARRYING_N(vict))>CAN_CARRY_N(vict))   {
	act("$N seems to have $S hands full.", 0, ch, 0, vict, TO_CHAR);
	return;
      }
      if (obj->obj_flags.weight+IS_CARRYING_W(vict)>CAN_CARRY_W(vict))   {
	act("$E can't carry that much weight.", 0, ch, 0, vict, TO_CHAR);
	return;
      }
      obj_from_char(obj);
      obj_to_char(obj, vict);
      act("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT);
      act("$n gives you $p.", 0, ch, obj, vict, TO_VICT);
      act("You give $p to $N", 0, ch, obj, vict, TO_CHAR);
      
      if (num > 0) num--;
      
    }
#if   NODUPLICATES
    do_save(ch, "", 0);
    do_save(vict, "", 0);
#endif
    
  }
}

