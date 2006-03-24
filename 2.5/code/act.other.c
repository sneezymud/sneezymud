/* ************************************************************************
*  file: act.other.c , Implementation of commands.        Part of DIKUMUD *
*  Usage : Other commands.                                                *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

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


/* extern variables */

extern struct str_app_type str_app[];
extern struct descriptor_data *descriptor_list;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];
extern struct char_data *character_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;

/* extern procedures */

void hit(struct char_data *ch, struct char_data *victim, int type);
void do_save(struct char_data *ch, char *argument, int cmd);
void do_shout(struct char_data *ch, char *argument, int cmd);
char *how_good(int percent);
char *DescMoves(float a);
void zero_rent(struct char_data *ch);
void do_terminal(struct char_data *ch, char *argument, int cmd);

void do_gain(struct char_data *ch, char *argument, int cmd)
{

}

void do_guard(struct char_data *ch, char *argument, int cmd)
{
  char comm[100];

  if (!IS_NPC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
    send_to_char("Sorry. you can't just put your brain on autopilot!\n\r",ch);
    return;
  }

  for(;isspace(*argument); argument++);

  if (!*argument) {
    if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
      act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You relax.\n\r",ch);
      REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
    } else {
      SET_BIT(ch->specials.act, ACT_GUARDIAN);
      act("$n alertly watches you.", FALSE, ch, 0, ch->master, TO_VICT);
      act("$n alertly watches $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
      send_to_char("You snap to attention\n\r", ch);    
    }  
  } else {
     if (!str_cmp(argument,"on")) {
      if (!IS_SET(ch->specials.act, ACT_GUARDIAN)) {
         SET_BIT(ch->specials.act, ACT_GUARDIAN);
         act("$n alertly watches you.", FALSE, ch, 0, ch->master, TO_VICT);
         act("$n alertly watches $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
         send_to_char("You snap to attention\n\r", ch);
       }
     } else if (!str_cmp(argument,"off")) {
       if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
         act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
         send_to_char("You relax.\n\r",ch);
         REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
       }
     }
  }

  return;
}


void do_junk(struct char_data *ch, char *argument, int cmd)
{
  char arg[100], buf[100], newarg[100];
  struct obj_data *tmp_object;
  int num, p, count;

/*
 *   get object name & verify
 */

  only_argument(argument, arg);
  if (*arg) {
    if (getall(arg,newarg)) {
      num = -1;
      strcpy(arg,newarg);
    } else if ((p = getabunch(arg,newarg))) {
      num = p;                     
      strcpy(arg,newarg);
    } else {
      num = 1;  
    }

    count = 0;
    while (num != 0) {
      tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
      if (tmp_object) {
	if (IS_OBJ_STAT(tmp_object,ITEM_NODROP)) {
	   send_to_char
		("You can't let go of it, it must be CURSED!\n\r", ch);
	   return;
	}
        GET_EXP(ch) += 5;
	obj_from_char(tmp_object);
	extract_obj(tmp_object);
	if (num > 0) num--;
	count++;
        if (count == 1) {
          sprintf(buf, "You junk %s ", arg);
          act(buf, 1, ch, 0, 0, TO_CHAR);
          sprintf(buf, "$n junks %s.", arg);
          act(buf, 1, ch, 0, 0, TO_ROOM);
        }
      } else {
	if (count > 1) {
	  sprintf(buf, "You junk %s (%d).\n\r", arg, count);
	  act(buf, 1, ch, 0, 0, TO_CHAR);
	  sprintf(buf, "$n junks %s.", arg);
	  act(buf, 1, ch, 0, 0, TO_ROOM);
	} else if (count == 1) {
	  sprintf(buf, "You junk %s ", arg);
	  act(buf, 1, ch, 0, 0, TO_CHAR);
	  sprintf(buf, "$n junks %s.", arg);
	  act(buf, 1, ch, 0, 0, TO_ROOM);
	} else {
	  send_to_char("You don't have anything like that\n\r", ch);
	}
	return;
      }
    }
  } else {
    send_to_char("Junk what?", ch);
    return;
  }
}

void do_command(struct char_data *ch, char *arg, int cmd)
{
  char	buf[16384];
  int no, i;

  extern char	*command[];
  extern struct command_info cmd_info[];

  if (IS_NPC(ch))
    return;

  send_to_char("The following commands are available:\n\r\n\r", ch);
  *buf = '\0';

  for (no = 1, i = 0; *command[i] != '\n'; i++) {
    if (GetMaxLevel(ch) >= cmd_info[i+1].minimum_level) {
      sprintf(buf + strlen(buf), "%-11s", command[i]);
      if (!(no % 7))
	strcat(buf, "\n\r");
      no++;
    }
  }
  strcat(buf, "\n\r");
  sprintf(buf + strlen(buf), "Total number of commands on system: %d\n\r", i);
  page_string(ch->desc, buf, 1);
}


void do_qui(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("You have to write quit - no less, to quit!\n\r",ch);
	return;
}

void do_split(struct char_data *ch, char *argument, int cmd)
{
  char buf[256];
  int no_members, share;
  int amount;
  struct char_data *k;
  struct follow_type *f;

  if (IS_NPC(ch))
    return;

  one_argument(argument,buf);
  
  if (is_number(buf)) {
    if((amount = atoi(buf))<0) {
      send_to_char("Sorry, you can't do that!\n\r",ch);
      return;
    }
    if (!(k = ch->master))
      k = ch;
      
    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
      no_members = 1;
    else
      no_members = 0;

      
    for (f=k->followers; f; f=f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (f->follower->in_room == ch->in_room))
	no_members++;
    
    if (no_members && IS_AFFECTED(ch, AFF_GROUP))
      share = amount/no_members;
    else {
      send_to_char("Split your gold with who?\n\r",ch);
      return;
    }
      
         
    if (GET_GOLD(ch)<amount && GetMaxLevel(ch) < 60) {
      send_to_char("Sure...Giving away money you don't have is going",ch);
      send_to_char(" to make you popular!\n\r",ch);
      return;
    }

    if (GetMaxLevel(ch) < 60)
      GET_GOLD(ch)-=share*(no_members-1);
    
    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room) && k != ch) {
      GET_GOLD(k)+=share;
      sprintf(buf,"%s splits %d coins, and you receive %d of them.\n\r",
	      GET_NAME(ch),amount,share);
      send_to_char(buf,k);
    }

    for (f=k->followers; f; f=f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (f->follower->in_room == ch->in_room) && f->follower != ch) {
	GET_GOLD(f->follower)+=share;
	sprintf(buf,"%s splits %d coins, and you receive %d of them.\n\r"
		,GET_NAME(ch),amount,share);
	send_to_char(buf,f->follower);
      }
      
    sprintf(buf,"%d coins divided in %d shares of %d coins.\n\r",
	    amount,no_members,share);
    send_to_char(buf,ch);
    if (GetMaxLevel(ch) > 60 && no_members > 1) {
      sprintf(buf,"%s was kind enough to share %d coins with others...",
	      GET_NAME(ch),amount);
      vlog(buf);
    }
  }
  else {
    send_to_char("Pardon? Split WHAT?\n\r",ch);
    return;
  }
}


void do_report(struct char_data *ch, char *argument, int cmd)
{
  int  i;
  char info[160];
  char buf[MAX_STRING_LENGTH];

  sprintf(info, "%d H, %d M. I am %s",
     (GET_HIT(ch)),
     (GET_MANA(ch)),
     DescMoves(((float) GET_MOVE(ch))/((float) GET_MAX_MOVE(ch))));

  for (i = 0; *(argument + i) == ' '; i++)
    ;

  if (!*(argument + i)) {
    sprintf(buf, "$n reports '%s'", info);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("Ok.\n\r", ch);
  } else {
    sprintf(buf, "$n reports '%s. %s'", info, argument + i);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("Ok.\n\r", ch);
  }
}



void do_title(struct char_data *ch, char *argument, int cmd)
{
   char buf[200];


   if (IS_NPC(ch) || !ch->desc)
       return;

  for(;isspace(*argument); argument++)  ;

  if (*argument) {
    sprintf(buf, "Your title has been set to : <%s>\n\r", argument);
    send_to_char(buf, ch);
    ch->player.title = strdup(argument);
  }      

}

void do_pray(struct char_data *ch, char *argument, int cmd)
{
}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
  void die(struct char_data *ch);
  
  if (IS_NPC(ch) || !ch->desc || IS_AFFECTED(ch, AFF_CHARM))
    return;
 
  if (GET_POS(ch) == POSITION_FIGHTING) {
    send_to_char("No way! You are fighting.\n\r", ch);
    return;
  }
  
  if (GET_POS(ch) < POSITION_STUNNED) {
    send_to_char("You die before your time!\n\r", ch);
    die(ch);
    return;
  }
  
 if (IS_SET(ch->specials.act, PLR_ANSI) ||
    (IS_SET(ch->specials.act,PLR_VT100)))
  send_to_char(VT_CLENSEQ, ch);
  act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
  act("$n has left the game.", TRUE, ch,0,0,TO_ROOM);
  zero_rent(ch);
  extract_char(ch); /* Char is saved in extract char */
}



void do_save(struct char_data *ch, char *argument, int cmd)
{
	struct obj_cost cost;
        struct char_data *tmp;
        struct obj_data *tmp_obj, *tl;
	struct obj_data *teq[MAX_WEAR], *eq[MAX_WEAR], *o;
        int i;

	if (IS_NPC(ch) && !(IS_SET(ch->specials.act, ACT_POLYSELF)))
	  return;

	if (GetMaxLevel(ch) >= LOW_IMMORTAL)
	  blk_save(ch);
#if NODUPLICATES
       send_to_char("Saving\n\r", ch);
#else
	send_to_char("Saving\n\r", ch);
#endif


        if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
/*  
  swap stuff, and equipment
*/
	  if (!ch->desc)  /* fuck you assholes :-) */
	    tmp = ch->orig;
	  else 
	     tmp = ch->desc->original;

	  if (!tmp) return;
	  tl = tmp->carrying;
/*
  there is a bug with this:  When you save, the alignment thing is checked,
  to see if you are supposed to be wearing what you are.  if your stuff gets
  kicked off your body, it will end up in room #3, on the floor, and in
  the inventory of the polymorphed monster.  This is a "bad" thing.  So,
  to fix it, each item in the inventory is checked.  if it is in a room,
  it is moved from the room, back to the correct inventory slot. 
*/
          tmp->carrying = ch->carrying;
	  for (i = 0; i < MAX_WEAR; i++) {
	    teq[i] = tmp->equipment[i];
            tmp->equipment[i] = ch->equipment[i];
          }
	   GET_EXP(tmp) = GET_EXP(ch);
	   GET_GOLD(tmp) = GET_GOLD(ch);
	   GET_ALIGNMENT(tmp) = GET_ALIGNMENT(ch);
	   recep_offer(tmp, NULL, &cost);
	   save_obj_for_save(tmp, &cost, 0);
	   save_char(ch, AUTO_RENT);
	   tmp->carrying = tl;
	  for (i = 0; i < MAX_WEAR; i++) {
            tmp->equipment[i] = teq[i];
	    if (ch->equipment[i] && ch->equipment[i]->in_room != -1) {
	      o = ch->equipment[i];
	      ch->equipment[i] = 0;
	      obj_from_room(o);
	      obj_to_char(o, ch);
	      equip_char(ch, o, i);  /* equip the correct slot */
	    }
          }
          
          return;
        } else {
	   recep_offer(ch, NULL, &cost);
	   save_obj_for_save(ch, &cost, 0);
	   save_char(ch, AUTO_RENT);
	 }
}


void do_not_here(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("Sorry, but you cannot do that here!\n\r",ch);
}



void do_sneak(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  byte percent;
  
  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    affect_from_char(ch, SKILL_SNEAK);
    if (IS_AFFECTED(ch, AFF_HIDE))
      REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    send_to_char("You are no longer sneaky.\n\r",ch);
    return;
  }

  if ((ch->skills[SKILL_SNEAK].learned == 0) && !HasClass(ch, CLASS_THIEF) && 
     !(HasClass(ch, CLASS_MONK)) && 
     (!HasClass(ch, CLASS_ANTIPALADIN)) && (!IS_IMMORTAL(ch))) {
    send_to_char("You're no thief!\n\r", ch);
    return;
  }

  send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
  send_to_char("And you attempt to hide yourself.\n\r", ch);
  
  
  percent=number(1,101); /* 101% is a complete failure */

  if (!ch->skills)
    return;
  
  if (percent > ch->skills[SKILL_SNEAK].learned +
      dex_app_skill[GET_DEX(ch)].sneak)
    return;
  
  SET_BIT(ch->specials.affected_by, AFF_HIDE);
  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch, BestThiefClass(ch));
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
  

}

void do_hide(struct char_data *ch, char *argument, int cmd)
{
  byte percent;
  
  send_to_char("you attempt to hide yourself.\n\r", ch);
  
  if (IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
 
  if (!HasClass(ch, CLASS_THIEF) && !(HasClass(ch, CLASS_MONK)) &&
     (!HasClass(ch, CLASS_ANTIPALADIN)) && (!IS_IMMORTAL(ch))) {
    send_to_char("You're no thief!\n\r", ch);
    return;
  }
 
  percent=number(1,101); /* 101% is a complete failure */
 
  if (!ch->skills)
    return;
  
  if (percent > ch->skills[SKILL_HIDE].learned +
      dex_app_skill[GET_DEX(ch)].hide) {
    LearnFromMistake(ch, SKILL_HIDE, 1, 90);
    WAIT_STATE(ch, PULSE_VIOLENCE*1);
    return;
  }
  
  SET_BIT(ch->specials.affected_by, AFF_HIDE);
  WAIT_STATE(ch, PULSE_VIOLENCE*1);
 
}




void do_bload(struct char_data *ch, char *arg, int cmd)
{
   struct obj_data *obj, *arrow;
   char arrow_name[240];
   char obj_name[240];
   char buf[240];
   int percent;

   arg = one_argument(arg, obj_name);
   only_argument(arg, arrow_name);

   if (!(obj=get_obj_in_list_vis(ch, obj_name, ch->carrying ) ) )
   {
      send_to_char("Load what?\n\r", ch);
      return;
   }

   if (!(arrow=get_obj_in_list_vis(ch, arrow_name, ch->carrying ) ) )
   {
         send_to_char("Load the bow with what?\n\r", ch);
         return;
   }

   if ( obj->obj_flags.type_flag != ITEM_BOW )
   {
        send_to_char("You can not reload that!\n\r", ch);
        return;
   }

   if ( arrow->obj_flags.type_flag != ITEM_ARROW )
   {
       send_to_char("You can't load with that!\n\r", ch);
       return;
   }

   act("You put the arrow into the $p", 1, ch, obj, 0, TO_CHAR);
   act("$n reloads the $p", 1, ch, obj, 0, TO_ROOM);

   obj->obj_flags.value[3]=1;
   obj->obj_flags.value[1]=arrow->obj_flags.value[1];

   obj->obj_flags.value[2]=arrow->obj_flags.value[2];

   obj_from_char(arrow);
   extract_obj(arrow);
   return;
}



void do_reload(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *obj, *shells;
  char shells_name[240];
  char obj_name[240];
  char buf[240];
  int percent;

  argument = one_argument(argument, obj_name);
  only_argument(argument, shells_name);

  if (!(obj=get_obj_in_list_vis(ch, obj_name, ch->carrying ) ) )
  {
	send_to_char("Reload what?\n\r", ch);
	return;
  }

  if (!(shells=get_obj_in_list_vis(ch, shells_name, ch->carrying ) ) )
  {
	send_to_char("Reload with what?\n\r", ch);
	return;
  }
  
  if ((!IS_NPC(ch)) && (GET_POS(ch) == POSITION_FIGHTING)) {
    send_to_char("You don't have time you are FIGHTING!!!!\n\r", ch);
    return;
    }

  if ( obj->obj_flags.type_flag != ITEM_FIREWEAPON )
  {
	send_to_char ("You can not reload that!\n\r", ch);
	return;
  }

  if ( shells->obj_flags.type_flag != ITEM_MISSILE )
  {
	send_to_char ("You can not reload with that!\n\r", ch);
	return;
  }
  if ( obj->obj_flags.value[0] != ObjVnum(shells))
  {
	sprintf ( buf, "You can't get the %s to fit.\n\r",
		shells->short_description );
	send_to_char (buf, ch);
	return;
  }
  act("You reload the $p", 1, ch, obj, 0, TO_CHAR);
  act("$n reloads $p.", 1, ch, obj, 0, TO_ROOM );
  obj->obj_flags.value[3]=shells->obj_flags.value[0];
  if ( obj->obj_flags.value[2] > 0 )
	obj->obj_flags.value[2]--;
  obj_from_char(shells);
  extract_obj(shells);
  return;
}

void do_steal(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  struct obj_data *obj;
  char victim_name[240];
  char obj_name[240];
  char buf[240];
  int percent;
  int gold, eq_pos;
  bool ohoh = FALSE;

  if (!ch->skills)
    return;

  if (check_peaceful(ch, "What if he caught you?\n\r"))
    return;
  
  
  argument = one_argument(argument, obj_name);
  only_argument(argument, victim_name);
  
  
  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Steal what from who?\n\r", ch);
    return;
  } else if (victim == ch) {
    send_to_char("Come on now, that's rather stupid!\n\r", ch);
    return;
   }

  WAIT_STATE(ch, PULSE_VIOLENCE*2);  /* they're gonna have to wait. */
  
  if ((GetMaxLevel(ch) < 2) && (!IS_NPC(victim))) {
    send_to_char("Due to misuse of steal, you can't steal from other players\n\r", ch);
    send_to_char("unless you are at least 2nd level. \n\r", ch);
    return;
  }
 
  if ((!victim->desc) && (!IS_NPC(victim)))
    return;
  
  /* 101% is a complete failure */
  percent=number(1,101) - dex_app_skill[GET_DEX(ch)].p_pocket;
  
  if (GET_POS(victim) < POSITION_SLEEPING)
    percent = -1; /* ALWAYS SUCCESS */
  
  percent += GetTotLevel(victim);
  
  if (GetMaxLevel(victim)>MAX_MORT)
    percent = 101; /* Failure */
  
  if (str_cmp(obj_name, "coins") && str_cmp(obj_name,"gold")) {
    
    if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {
      
      for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
	if (victim->equipment[eq_pos] &&
	    (isname(obj_name, victim->equipment[eq_pos]->name)) &&
	    CAN_SEE_OBJ(ch,victim->equipment[eq_pos])) {
	  obj = victim->equipment[eq_pos];
	  break;
	}
      
      if (!obj) {
	act("$E has not got that item.",FALSE,ch,0,victim,TO_CHAR);
	return;
      } else { /* It is equipment */
	if ((GET_POS(victim) > POSITION_STUNNED)) {
	  send_to_char("Steal the equipment now? Impossible!\n\r", ch);
	  return;
	} else {
	  act("You unequip $p and steal it.",FALSE, ch, obj ,0, TO_CHAR);
	  act("$n steals $p from $N.",FALSE,ch,obj,victim,TO_NOTVICT);
	  obj_to_char(unequip_char(victim, eq_pos), ch);
#if NODUPLICATES
	  do_save(ch, "", 0);
	  do_save(victim, "", 0);
#endif
	}
      }
    } else {  /* obj found in inventory */

      if (IS_OBJ_STAT(obj,ITEM_NODROP)) {
         send_to_char
      	   ("You can't steal it, it must be CURSED!\n\r", ch);
	 return;
      }
      
      percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */

      if ((!ObjLevelCheck(obj,ch)) || (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned))) {
	ohoh = TRUE;
	act("Oops..", FALSE, ch,0,0,TO_CHAR);
	act("$n tried to steal something from you!",FALSE,ch,0,victim,TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
        if (IS_PC(victim)) {
          if (!IS_SET(ch->specials.act, PLR_OUTLAW)) {
            send_to_char("You are now a thief!\n\r", ch);
            SET_BIT(ch->specials.act, PLR_OUTLAW);
          }
        }
      } else { /* Steal the item */
	if ((IS_CARRYING_N(ch) + GET_OBJ_VOLUME(obj)) < CAN_CARRY_N(ch)) {
	  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	    obj_from_char(obj);
	    obj_to_char(obj, ch);
	    send_to_char("Got it!\n\r", ch);
#if NODUPLICATES
	  do_save(ch, "", 0);
	  do_save(victim, "", 0);
#endif
	    
	  }
	} else
	  send_to_char("You cannot carry that much.\n\r", ch);
      }
    }
  } else { /* Steal some coins */
    if (IS_PC(victim) || (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned))) {
      ohoh = TRUE;
      act("Oops..", FALSE, ch,0,0,TO_CHAR);
      act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
      act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
    } else {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
      gold = MIN(4000, gold);
      if (gold > 0) {
	GET_GOLD(ch) += gold;
	GET_GOLD(victim) -= gold;
	sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
	send_to_char(buf, ch);
      } else {
	send_to_char("You couldn't get any gold...\n\r", ch);
      }
    }
  }
  
  if (ohoh && IS_NPC(victim) && AWAKE(victim))
    if (IS_SET(victim->specials.act, ACT_NICE_THIEF)) {
      sprintf(buf, "%s is a bloody thief.", GET_NAME(ch));
      do_shout(victim, buf, 0);
      send_to_char("Don't you ever do that again!\n\r", ch);
    } else {
      hit(victim, ch, TYPE_UNDEFINED);
    }
  
}

void do_practice(struct char_data *ch, char *arg, int cmd) 
{
  char buf[MAX_STRING_LENGTH*2], buffer[MAX_STRING_LENGTH*2];
  int i;
  
  extern char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];
 
  buffer[0] = '\0';
  
  if ((cmd != 164) && (cmd != 170)) return;

  if (IS_IMMORTAL(ch)) {
     send_to_char("You are learned in every skill and spell.\n\r", ch);
     return;
   }
  
  if (!ch->skills)
    return;
  
  for (; isspace(*arg); arg++);
  
  if (!arg) {
    send_to_char("You need to supply a class for that.",ch);
    return;
  }
  
  switch(*arg) {
  case 'w':
  case 'W':
  case 'f':
  case 'F': 
    {
      if (!HasClass(ch, CLASS_WARRIOR)) {
        send_to_char("I bet you think you're a warrior.\n\r", ch);
        return;
      }
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < 200; i++)
        if (!spell_info[i+1].spell_pointer && ch->skills[i+1].learned) {
          sprintf(buf,"%-30s %s \n\r",spells[i],
                  how_good(ch->skills[i+1].learned));
          if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
            break;
          strcat(buffer, buf);
          strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    } break;
  case 'a':
  case 'A':
    {
 
      if (!HasClass(ch, CLASS_ANTIPALADIN)) {
        send_to_char("I bet you think you're an antipaladin.\n\r", ch);
        return;
      }
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < 200; i++)
        if (!spell_info[i+1].spell_pointer && ch->skills[i+1].learned) {
          sprintf(buf,"%-30s %s \n\r",spells[i],
                  how_good(ch->skills[i+1].learned));
          if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
            break;
          strcat(buffer, buf);
          strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    } break;
  case 'p':
  case 'P':
    {
 
      if (!HasClass(ch, CLASS_PALADIN)) {
        send_to_char("I bet you think you're a paladin.\n\r", ch);
        return;
      }
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < 200; i++)
        if (!spell_info[i+1].spell_pointer && ch->skills[i+1].learned) {
          sprintf(buf,"%-30s %s \n\r",spells[i],
                  how_good(ch->skills[i+1].learned));
          if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
            break;
          strcat(buffer, buf);
          strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    } break;
  case 'r':
  case 'R':
    {
 
      if (!HasClass(ch, CLASS_RANGER)) {
        send_to_char("I bet you think you're a ranger.\n\r", ch);
        return;
      }
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < 200; i++)
        if (!spell_info[i+1].spell_pointer && ch->skills[i+1].learned) {
          sprintf(buf,"%-30s %s \n\r",spells[i],
                  how_good(ch->skills[i+1].learned));
          if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
            break;
          strcat(buffer, buf);
          strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    } break;
  case 't':
  case 'T':
    {
 
      if (!HasClass(ch, CLASS_THIEF)) {
        send_to_char("I bet you think you're a thief.\n\r", ch);
        return;
      }
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < 200; i++)
        if (!spell_info[i+1].spell_pointer && ch->skills[i+1].learned) {
          sprintf(buf,"%-30s %s \n\r",spells[i],
                  how_good(ch->skills[i+1].learned));
          if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
            break;
          strcat(buffer, buf);
          strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    } break;
  case 'M':
  case 'm':
    {
      if (!HasClass(ch, CLASS_MAGIC_USER)) {
        send_to_char("I bet you think you're a magic-user.\n\r", ch);
        return;
      }
      send_to_char("Your spellbook holds these spells:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
        if (spell_info[i+1].spell_pointer &&
            (spell_info[i+1].min_level_magic<=GET_LEVEL(ch,MAGE_LEVEL_IND))) {
          sprintf(buf,"[%d] %s %s \n\r",
                  spell_info[i+1].min_level_magic,
                  spells[i],how_good(ch->skills[i+1].learned));
          if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
            break;
          strcat(buffer, buf);
          strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;                     
    } 
    break;
  case 'C':
  case 'c':
    {
      if (!HasClass(ch, CLASS_CLERIC)) {
        send_to_char("I bet you think you're a cleric.\n\r", ch);
        return;
      }
      send_to_char("You can attempt any of these spells:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
        if (spell_info[i+1].spell_pointer &&
           (spell_info[i+1].min_level_cleric<=GET_LEVEL(ch,CLERIC_LEVEL_IND))){
          sprintf(buf,"[%d] %s %s \n\r",
                  spell_info[i+1].min_level_cleric,
                  spells[i],how_good(ch->skills[i+1].learned));
          if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
            break;
          strcat(buffer, buf);
          strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    } 
    break;
  case 'K':
  case 'k': {
      if (!HasClass(ch, CLASS_MONK)) {
        send_to_char("I bet you think you're a monk.\n\r", ch);
        return;
      }
    
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < 200; i++)
        if (!spell_info[i+1].spell_pointer && ch->skills[i+1].learned) {
          sprintf(buf,"%-30s %s \n\r",spells[i],
                  how_good(ch->skills[i+1].learned));
          if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
            break;
          strcat(buffer, buf);
          strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    }
    break;
 
  default:
    send_to_char("Which class???\n\r", ch);
  }
 
  send_to_char("Go to your guildmaster to see the spells you don't have\n\r", ch);
  
}


void do_idea(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_INPUT_LENGTH+20];

	if (IS_NPC(ch))	{
		send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
		return;
	}
        if (GetMaxLevel(ch)==60)  
        {
          if (ch) {
            start_page_file(ch->desc,IDEA_FILE,"The players havent said shit\n\r"); 
            return;
          }
        }
	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)	{
	      send_to_char
		("That doesn't sound like a good idea to me.. Sorry.\n\r",ch);
		return;
	}
	if (!(fl = fopen(IDEA_FILE, "a")))	{
		perror ("do_idea");
		send_to_char("Could not open the idea-file.\n\r", ch);
		return;
	}

	sprintf(str, "**%s: %s\n", GET_NAME(ch), argument);

	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok. Thanks.\n\r", ch);
}







void do_typo(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_INPUT_LENGTH+20];

	if (IS_NPC(ch))	{
		send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
		return;
	}
        if (GetMaxLevel(ch)==60) {
          if(ch) {
            start_page_file(ch->desc,TYPO_FILE,"The players cant spell worth shit.\n\r");
            return;
          }
        }

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)	{
		send_to_char("I beg your pardon?\n\r", 	ch);
		return;
	}
	if (!(fl = fopen(TYPO_FILE, "a")))	{
		perror ("do_typo");
		send_to_char("Could not open the typo-file.\n\r", ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
		GET_NAME(ch), ch->in_room, argument);
	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok. thanks.\n\r", ch);

}





void do_bug(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_INPUT_LENGTH+20];

	if (IS_NPC(ch))	{
		send_to_char("You are a monster! Bug off!\n\r", ch);
		return;
	}
        if (GetMaxLevel(ch)==60) {
          if (ch) {
           start_page_file(ch->desc,BUG_FILE,"Players aint saying shit.\n\r");
           return;
          }
        }

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)	{
		send_to_char("Pardon?\n\r",ch);
		return;
	}
	if (!(fl = fopen(BUG_FILE, "a")))	{
		perror ("do_bug");
		send_to_char("Could not open the bug-file.\n\r", ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
		GET_NAME(ch), ch->in_room, argument);
	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok.\n\r", ch);
      }



void do_monitor(struct char_data *ch, char *argument, int cmd)
{
   struct obj_data *radio;
   char buf[200];
   char num[200];
   int number;

   radio = ch->equipment[WEAR_RADIO];

   only_argument(argument, num);

   if (!radio) {
    send_to_char("You need to be holding a radio to monitor channels\n\r", ch);
    return;
   }

   if (!isdigit(*num)) {
     send_to_char("You must enter a number X such that ", ch);
     send_to_char("X fits in the range 1-X, which is the number of channels", ch);
     send_to_char("that you want to monitor\n\r", ch);
       return;
     } else {
    number = atoi(num);
    if (number > radio->obj_flags.value[1]) {
     send_to_char("Your radio doesnt handle that many channels.\n\r", ch);
      return;
    } else {
    radio->obj_flags.value[2] = number;
    if (number > 1) {
    sprintf(buf, "You are now monitoring channels 1 - %d\n\r", 
                    radio->obj_flags.value[2]);
     send_to_char(buf, ch);
    } else {
       send_to_char("Turning off your monitor\n\r", ch);
         return;
      }
    }
  }
}

void do_channel(struct char_data *ch, char *argument, int cmd)
{
   struct obj_data *radio, *radio2;
   struct descriptor_data *i;
   char buf[MAX_STRING_LENGTH];
   char num[200];
   int number;

   radio = ch->equipment[WEAR_RADIO];
 
   only_argument(argument, num);

   if (!*num) {
      for(i= descriptor_list; i; i = i->next) {
         if (i->character && (CAN_SEE(ch, i->character)) &&
            (i->connected == CON_PLYNG) &&
            (i->character->in_room != NOWHERE)) {

             radio2 = i->character->equipment[WEAR_RADIO];

            if (radio2) {
              sprintf(buf, "%-15s : Channel %d\n\r",
                      GET_NAME(i->character), radio2->obj_flags.value[3]);
              send_to_char(buf,ch);
            }
         }
      }
    } else {
      if (isdigit(*num)) {
        number = atoi(num);

        if (number < 1) {
             send_to_char("Channels must be positive.\n\r", ch);
             return;
        }

        if (radio) {
          if (number <= radio->obj_flags.value[1]) {
             send_to_char("Changing channel.\n\r", ch);
             radio->obj_flags.value[3] = number; 
          } else {
             send_to_char("Your radio doesnt have those channels.\n\r", ch);
             return;
          }
         } else {
            send_to_char("You need to hold your radio to tune it.\n\r", ch);
            return;
         }
      } else {
        send_to_char("Please use a number for your channel.\n\r", ch);
        return;
      }
    }
}

void do_prompt(struct char_data *ch, char *arg, int cmd)
{
   char tmp_name[20], buf[80];

   if (IS_NPC(ch))
      return;

   for (;isspace(*arg);arg++);

   if (strlen(arg) > 40) 
       send_to_char("You must choose a shorter prompt.\n\r", ch);
    else if (*arg) {
     sprintf(buf,"Setting prompt to %s\n\r", arg);
     send_to_char(buf,ch);
     ch->desc->prompt = strdup(arg);
    } else {
     send_to_char("Clearing prompt to NULL", ch);
     ch->desc->prompt = 0;
     }
}
      
      


void do_terminal(struct char_data *ch, char *argument, int cmd)
{
    char buf[80], term[80], screen[80];
  
    if (IS_NPC(ch)) 
      return;

    half_chop(argument,term,screen);
 
    if (*term) {
     if (is_abbrev(term,"screensize")) {
      if (*screen) {
       if (isdigit(*screen)) {
        if (atoi(screen) == 0) {
          send_to_char("Screensize needs to be a number from 1-48.\n\r", ch);
          return;
        } 
        ch->desc->screen_size = MIN(48,atoi(screen));
        send_to_char("Done\n\r", ch);
       } else {
         send_to_char("Screensize needs to be a number from 1-48.\n\r", ch);
         return;
       }
      } else {
        send_to_char("What is the screensize?\n\r", ch);
        return;
      }
     } else if (is_abbrev(term,"ansi")) {
       if (IS_SET(ch->specials.act, PLR_ANSI)) {
           send_to_char("You are already in Ansi mode.\n\r", ch);
           return;
       } else {
           send_to_char(VT_CLENSEQ,ch);
           sprintf(buf,VT_MARGSET,1,(ch->desc->screen_size-2));
           send_to_char(buf,ch);
           send_to_char("Setting term type to Ansi...\n\r", ch);
           SET_BIT(ch->specials.act, PLR_ANSI);
         if (IS_SET(ch->specials.act, PLR_VT100))
            REMOVE_BIT(ch->specials.act, PLR_VT100);
        }
     } else if (is_abbrev(term,"vt")) {
       if (IS_SET(ch->specials.act,PLR_VT100)) {
           send_to_char("You are already in vt100 mode!\n\r", ch);
           return;
       } else {
           send_to_char(VT_CLENSEQ,ch);
           sprintf(buf,VT_MARGSET,1,(ch->desc->screen_size-2));
           send_to_char(buf,ch);
           send_to_char("Setting term type to vt100...\n\r", ch);
           SET_BIT(ch->specials.act, PLR_VT100);
         if (IS_SET(ch->specials.act,PLR_ANSI))
           REMOVE_BIT(ch->specials.act,PLR_ANSI); 
        }
     } else if (is_abbrev(term,"none")) {
       if (!IS_SET(ch->specials.act, PLR_ANSI) &&
           !IS_SET(ch->specials.act,PLR_VT100)) {
           send_to_char("You already don't have a terminal type set.\n\r",ch);
           return;
       } else {
           send_to_char(VT_CLENSEQ,ch);
           sprintf(buf,VT_MARGSET,1,ch->desc->screen_size);
           send_to_char(buf,ch);
           send_to_char("Setting term type to NONE...\n\r", ch);
         if (IS_SET(ch->specials.act, PLR_ANSI)) 
             REMOVE_BIT(ch->specials.act, PLR_ANSI);
         if (IS_SET(ch->specials.act, PLR_VT100))
             REMOVE_BIT(ch->specials.act, PLR_VT100);
       }
     }
   } else {
     send_to_char("What terminal type did you want?(Ansi, Vt100, or None)\n\r", ch);
     return;
   }
}
            
       
   

    

void do_cls(struct char_data *ch, char *argument, int cmd)
{
   char buf[80];

    if (IS_NPC(ch))
       return;

  if (IS_SET(ch->specials.act, PLR_ANSI) ||
      IS_SET(ch->specials.act, PLR_VT100)) {
    send_to_char(VT_CLENSEQ,ch);
    sprintf(buf,VT_MARGSET,1,ch->desc->screen_size-2);
    send_to_char(buf, ch);
  } else 
     send_to_char(VT_CLENSEQ,ch);
}

void do_brief(struct char_data *ch, char *argument, int cmd)
{
  if (IS_NPC(ch))
    return;
  
  if (IS_SET(ch->specials.act, PLR_BRIEF))	{
    send_to_char("Brief mode off.\n\r", ch);
    REMOVE_BIT(ch->specials.act, PLR_BRIEF);
  }	else	{
    send_to_char("Brief mode on.\n\r", ch);
    SET_BIT(ch->specials.act, PLR_BRIEF);
  }
}

void do_mess(struct char_data *ch, char *argument, int cmd)
{
}
 


void do_log(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[100]; 
  char buf[200];

  if (IS_NPC(ch))
       return;

  only_argument(argument, name);

  if (!*name) {
       send_to_char("Put who into the logfile?\n\r", ch);
       return;
   }

   if (!(vict = get_char_vis(ch, name))) {
     send_to_char("Noone here by that name.\n\r", ch);
     return;
   } else if (GetMaxLevel(vict) > GetMaxLevel(ch)) {
     send_to_char("I dont think they would like that.\n\r", ch);
       return;
   } else if (!IS_SET(vict->specials.act, PLR_LOGGED)) {
     sprintf(buf, "%s will now be logged.\n\r", GET_NAME(vict));
     send_to_char(buf, ch);
     SET_BIT(vict->specials.act, PLR_LOGGED);
    } else {
     REMOVE_BIT(vict->specials.act, PLR_LOGGED);
     send_to_char("Logged bit has been removed.\n\r", ch);
    }
}

     
void do_color(struct char_data *ch, char *argument, int cmd)
{

  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.act, PLR_COLOR)) {
   send_to_char("Color mode off.\n\r", ch);
   REMOVE_BIT(ch->specials.act, PLR_COLOR);
  }   else   {

  sprintf(buf,"%sC%so%sl%so%sr%s mode on.\n\r",
          ANSI_RED,ANSI_CYAN,ANSI_BLUE,ANSI_ORANGE,ANSI_VIOLET,ANSI_NORMAL);
  send_to_char(buf, ch);
   SET_BIT(ch->specials.act, PLR_COLOR);
  }
}




void do_compact(struct char_data *ch, char *argument, int cmd)
{
  if (IS_NPC(ch))
    return;
  
  if (IS_SET(ch->specials.act, PLR_COMPACT))	{
    send_to_char("You are now in the uncompacted mode.\n\r", ch);
    REMOVE_BIT(ch->specials.act, PLR_COMPACT);
  }	else	{
    send_to_char("You are now in compact mode.\n\r", ch);
    SET_BIT(ch->specials.act, PLR_COMPACT);
  }
}


void do_group(struct char_data *ch, char *argument, int cmd)
{
  char name[256];
  char buf[256];
  struct char_data *victim, *k;
  struct follow_type *f;
  bool found;
  
  only_argument(argument, name);
  
  if (!*name) {
    if (!IS_AFFECTED(ch, AFF_GROUP)) {
      send_to_char("But you are a member of no group?!\n\r", ch);
    } else {
      send_to_char("Your group consists of:\n\r\n\r", ch);
      if (ch->master)
	k = ch->master;
      else
	k = ch;
      
      if (IS_AFFECTED(k, AFF_GROUP)) {
        sprintf(buf,"%-15s [%.1f%%dhp %.1f%%m. %s looks %s.\n\r",
         (IS_NPC(k) ? k->player.short_descr : GET_NAME(k)),
         (((float) (GET_HIT(k)))/((float) GET_MAX_HIT(k))*100),
         (((float) (GET_MANA(k)))/((float) GET_MAX_MANA(k))*100), 
         (IS_NPC(k) ? k->player.short_descr : GET_NAME(k)),
         DescMoves(((float) GET_MOVE(k))/((float) GET_MAX_MOVE(k))));
         send_to_char(buf,ch);
       }
      
      for(f=k->followers; f; f=f->next)
      {
	if (IS_AFFECTED(f->follower, AFF_GROUP))
	  {
          sprintf(buf,"%-15s [%.1f%%dhp %.1f%%m. %s looks %s.\n\r",
         (IS_NPC(f->follower) ? f->follower->player.short_descr : GET_NAME(f->follower)),
         (((float) (GET_HIT(f->follower)))/((float) GET_MAX_HIT(f->follower))*100),
         (((float) (GET_MANA(f->follower)))/((float) GET_MAX_MANA(f->follower))*100),
         (IS_NPC(f->follower) ? f->follower->player.short_descr : GET_NAME(f->follower)),
         DescMoves(((float) GET_MOVE(f->follower))/((float) GET_MAX_MOVE(f->follower))));
send_to_char(buf,ch);

       }
    }
    
   }
    return;
  }
  
  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("No one here by that name.\n\r", ch);
  } else {
    
    if (ch->master) {
      act("You can not enroll group members without being head of a group.",
	  FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    
    found = FALSE;
    
    if (victim == ch)
      found = TRUE;
    else {
      for(f=ch->followers; f; f=f->next) {
	if (f->follower == victim) {
	  found = TRUE;
	  break;
	}
      }
    }
    
    if (found) {
      if (IS_AFFECTED(victim, AFF_GROUP)) {
	act("$n has been kicked out of $N's group!", FALSE, victim, 0, ch, TO_ROOM);
	act("You are no longer a member of $N's group!", FALSE, victim, 0, ch, TO_CHAR);
	REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
      } else {
	if (GetMaxLevel(victim)>=LOW_IMMORTAL) {
	  act("You really don't want $n in your group.", FALSE, ch, 0, 0, TO_CHAR);
	  return;
	}
	if (GetMaxLevel(ch)>=LOW_IMMORTAL) {
	  act("Now now.  That would be CHEATING!",FALSE,ch,0,0,TO_CHAR);
	  return;  
	  
	}
	act("$n is now a member of $N's group.", 
	    FALSE, victim, 0, ch, TO_ROOM);
	act("You are now a member of $N's group.", 
	    FALSE, victim, 0, ch, TO_CHAR);
	SET_BIT(victim->specials.affected_by, AFF_GROUP);
      }
    } else {
      act("$N must follow you, to enter the group", 
	  FALSE, ch, 0, victim, TO_CHAR);
    }
  }
}


void do_quaff(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp;
  int i;
  bool equipped;
  
  equipped = FALSE;
  
  only_argument(argument,buf);
  
  if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    temp = ch->equipment[HOLD];
    equipped = TRUE;
    if ((temp==0) || !isname(buf, temp->name)) {
      act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  }
  
  if (GET_COND(ch,FULL)>-1) {
    if (GET_COND(ch,FULL)>20) {
      act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
      return;
    } else {
      GET_COND(ch, FULL)+=1;
    }
  }
  
  if (temp->obj_flags.type_flag!=ITEM_POTION) {
    act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
  act("You quaff $p which dissolves.",FALSE, ch, temp,0, TO_CHAR);
  
  /*  my stuff */
  if (ch->specials.fighting) {
    if (equipped) {
      if (number(1,20) > ch->abilities.dex) {
	act("$n is jolted and drops $p!  It shatters!", 
	    TRUE, ch, temp, 0, TO_ROOM);
	act("You arm is jolted and $p flies from your hand, *SMASH*",
	    TRUE, ch, temp, 0, TO_CHAR);
	if (equipped)
	  temp = unequip_char(ch, HOLD);
	extract_obj(temp);
	return;
      }
    } else {
      if (number(1,20) > ch->abilities.dex - 4) {
	act("$n is jolted and drops $p!  It shatters!", 
	    TRUE, ch, temp, 0, TO_ROOM);
	act("You arm is jolted and $p flies from your hand, *SMASH*",
	    TRUE, ch, temp, 0, TO_CHAR);
	extract_obj(temp);
	return;
      }
    }
  }
  
  for (i=1; i<4; i++)
    if (temp->obj_flags.value[i] >= 1)
      ((*spell_info[temp->obj_flags.value[i]].spell_pointer)
       ((byte) temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, temp));
  
  if (equipped)
    temp = unequip_char(ch, HOLD);
  
  extract_obj(temp);
  
  WAIT_STATE(ch, PULSE_VIOLENCE);
  
}


void do_recite(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *scroll, *obj;
  struct char_data *victim;
  int i, bits;
  bool equipped;
  
  equipped = FALSE;
  obj = 0;
  victim = 0;

  argument = one_argument(argument,buf);
  
  if (!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    scroll = ch->equipment[HOLD];
    equipped = TRUE;
    if ((scroll==0) || !isname(buf, scroll->name)) {
      act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  }
  
  if (scroll->obj_flags.type_flag!=ITEM_SCROLL)  {
    act("Recite is normally used for scrolls.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if (*argument) {
    bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM |
			FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &victim, &obj);
    if (bits == 0) {
      send_to_char("No such thing around to recite the scroll on.\n\r", ch);
      return;
    }
  } else {
    victim = ch;
  }
  
  if (!HasClass(ch, CLASS_MAGIC_USER) && 
      !HasClass(ch, CLASS_CLERIC)) {
    if (ch->skills[SKILL_READ_MAGIC].learned < number(1,101)) {
      if (scroll->obj_flags.value[1] != SPELL_WORD_OF_RECALL) {
	send_to_char("You can't understand this...\n\r",ch);
	return;
      }
    }
  }
  
  act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
  act("You recite $p which bursts into flame.",FALSE,ch,scroll,0,TO_CHAR);
  
  for (i=1; i<4; i++)
    if (scroll->obj_flags.value[i] >= 1) {
      if (IS_SET(spell_info[scroll->obj_flags.value[i]].targets, TAR_VIOLENT) &&
	  check_peaceful(ch, "Impolite magic is banned here."))
	continue;
      ((*spell_info[scroll->obj_flags.value[i]].spell_pointer)
       ((byte) scroll->obj_flags.value[0], ch, "", SPELL_TYPE_SCROLL, victim, obj));
    }
  if (equipped)
    scroll = unequip_char(ch, HOLD);
  
  extract_obj(scroll);
}



void do_use(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct char_data *tmp_char;
  struct obj_data *tmp_object, *stick;
  
  int bits;
  
  argument = one_argument(argument,buf);
  
  if (ch->equipment[HOLD] == 0 ||
      !isname(buf, ch->equipment[HOLD]->name)) {
    act("You do not hold that item in your hand.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  /* To avoid unauthorized player killing, I put this in (Brut)
     so that only pc's and mobs who arent charmed can use */

  if (!IS_PC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
    return;
  }

  stick = ch->equipment[HOLD];
  
  if (stick->obj_flags.type_flag == ITEM_STAFF)  {
    act("$n taps $p three times on the ground.",TRUE, ch, stick, 0,TO_ROOM);
    act("You tap $p three times on the ground.",FALSE,ch, stick, 0,TO_CHAR);
    if (stick->obj_flags.value[2] > 0) {  /* Is there any charges left? */
      stick->obj_flags.value[2]--;
      ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
       ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF, 0, 0));
      WAIT_STATE(ch, PULSE_VIOLENCE);
    } else {
      send_to_char("The staff seems powerless.\n\r", ch);
    }
  } else if (stick->obj_flags.type_flag == ITEM_WAND) {
    bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | 
			FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
    if (bits) {
      struct spell_info_type	*spellp;

      spellp = spell_info + (stick->obj_flags.value[3]);

      if (bits == FIND_CHAR_ROOM) {
	act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
	act("You point $p at $N.",FALSE,ch, stick, tmp_char, TO_CHAR);
      } else {
	act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
	act("You point $p at $P.",FALSE,ch, stick, tmp_object, TO_CHAR);
      }

      if (IS_SET(spellp->targets, TAR_VIOLENT) &&
	  check_peaceful(ch, "Impolite magic is banned here.\n\r"))
	return;

      if (IS_SET(spellp->targets, TAR_VIOLENT) &&
          (bits == FIND_CHAR_ROOM) &&
          IS_PC(tmp_char)) {
           act("$N tries to harm you by casting a malicious spell.",
              FALSE, tmp_char, 0, ch, TO_CHAR);
          if (!IS_SET(ch->specials.act, PLR_KILLER) &&
              !IS_SET(tmp_char->specials.act, PLR_OUTLAW) &&
              !IS_SET(tmp_char->specials.act, PLR_KILLER)) {
            send_to_char("You are now a player killer!!\n\r", ch);
            SET_BIT(ch->specials.act, PLR_KILLER);
           }
       }


      if (stick->obj_flags.value[2] > 0) { /* Is there any charges left? */
	stick->obj_flags.value[2]--;
	((*spellp->spell_pointer)
	 ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_WAND, 
	  tmp_char, tmp_object));
	WAIT_STATE(ch, PULSE_VIOLENCE);
      } else {
	send_to_char("The wand seems powerless.\n\r", ch);
      }
    } else {
      send_to_char("What should the wand be pointed at?\n\r", ch);
    }
  } else {
    send_to_char("Use is normally only for wand's and staff's.\n\r", ch);
  }
}

do_plr_noshout(struct char_data *ch, char *argument, int cmd)
{
  char buf[128];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf) {
    if (IS_SET(ch->specials.act, PLR_NOSHOUT)) {
      send_to_char("You can now hear shouts again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_NOSHOUT);
    } else {
      send_to_char("From now on, you won't hear shouts.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_NOSHOUT);
    }
  } else {
    send_to_char("Only the gods can shut up someone else. \n\r",ch);
  }
  
}


do_teams(struct char_data *ch, char *argument, int cmd)
{
  char buf[255];
  struct descriptor_data *d;
  int count;
  struct char_data *person;
  
  if (IS_NPC(ch))
    return;
  
  if (GetMaxLevel(ch) < 57) {
    send_to_char("This command is for gods level 57 or higher.\n\r",ch);
  }
  
  send_to_char("Assigning teams.\n\r",ch);
  count = 0;
  for (d = descriptor_list; d ; d = d->next) {
    if ((!d->connected) && 
	((real_roomp((person = (d->original ? d->original:d->character)
		      )->in_room)->zone) == (real_roomp(ch->in_room)->zone))) {
      if (!IS_IMMORTAL(person)) {
	sprintf(buf, "Doing %s %s", GET_NAME(person), person->player.title);
      }
    }
  }
    send_to_char("Done.\n\r",ch);
}
