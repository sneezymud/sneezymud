/* ************************************************************************
 *  file: mobact.c , mobile action module.                 part of dikumud *
 *  usage: procedures generating 'intelligent' behavior in the mobiles.    *
 *  copyright (c) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>

#include "structs.h"
#include "utils.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "opinion.h"
#include "trap.h"
#include "spells.h"

extern struct char_data *character_list;
extern struct index_data *mob_index;
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct str_app_type str_app[];
extern struct dex_skill_type dex_app_skill[];
extern struct index_data *mob_index;
extern int vol_mult[];

void do_get(struct char_data *ch, char *argument, int cmd);
void hit(struct char_data *ch, struct char_data *victim, int type);
struct char_data *FindVictim( struct char_data *ch);
struct char_data *FindMetaVictim( struct char_data *ch);
struct char_data *FindAnAttacker( struct char_data *ch);
int SameRace( struct char_data *ch1, struct char_data *ch2);
char in_group ( struct char_data *ch1, struct char_data *ch2);
int dir_track( struct char_data *ch, struct char_data *vict);
int IsHumanoid( struct char_data *ch);
struct char_data *FindAnyVictim( struct char_data *ch);
void mobile_activity(struct char_data *ch);



void mobile_guardian(struct char_data *ch)
{
  struct char_data *targ;
  int i, found=FALSE;
  
  if (ch->in_room > -1) {
    if ((!ch->master) || (!IS_AFFECTED(ch, AFF_CHARM)))
      return;
    if (ch->master->specials.fighting) { /**/
      for(i=0;i<10&&!found;i++) {
	targ = FindAnAttacker(ch->master);
	if (targ)
	  found=TRUE;
      }
      
      if (!found) return;
      
      if (!SameRace(targ, ch)) {
	if (IsHumanoid(ch)) {
	  act("$n screams 'I must protect my master!'", 
	      FALSE, ch, 0, 0, TO_ROOM);
	} else {
	  act("$n growls angrily!", 
	      FALSE, ch, 0, 0, TO_ROOM);
	}
	if (CAN_SEE(ch, targ))
	  hit(ch, targ,0);	  
      }
    }
  }
}

void mobile_wander(struct char_data *ch)
{
  int	door;
  struct room_direction_data	*exitp;
  struct room_data	*rp;
  
  if (! ((GET_POS(ch) == POSITION_STANDING) &&
	 ((door = number(0, 15)) <= 5) &&
	 exit_ok(exitp=EXIT(ch,door), &rp) &&
	 (!IS_SET(rp->room_flags, NO_MOB) || IS_POLICE(ch)) &&
	 !IS_SET(rp->room_flags, DEATH))
      )
    return;
  
  if (IsHumanoid(ch) ? CAN_GO_HUMAN(ch, door) : CAN_GO(ch, door)) {
    if (ch->specials.last_direction == door) {
      ch->specials.last_direction = -1;
    } else {
      if (!IS_SET(ch->specials.act, ACT_STAY_ZONE) ||
	  (rp->zone == real_roomp(ch->in_room)->zone)) {
	ch->specials.last_direction = door;
	go_direction(ch, door);
      }
    }
  }
}

void MobHunt(struct char_data *ch)
{
  int res, k;
  
#if NOTRACK     
  return;    /* too much CPU useage for some machines.  */
#endif
  
  if (ch->persist <= 0) {
    res = choose_exit_in_zone(ch->in_room, ch->old_room, 2000);
    if (res > -1) {
      go_direction(ch, res);
    } else {
      if (ch->specials.hunting) {
	if (ch->specials.hunting->in_room == ch->in_room) {
	  if (Hates(ch, ch->specials.hunting) && 
	      (!IS_AFFECTED(ch->specials.hunting, AFF_HIDE))) {
	    if (check_peaceful(ch, "You'd love to tear your quarry to bits, but you just CAN'T\n\r")) {
	      act("$n fumes at $N", TRUE, ch, 0,
		  ch->specials.hunting, TO_ROOM); 
	    } else {
	      if (IsHumanoid(ch)) {
		act("$n screams 'Time to die, $N'", 
		    TRUE, ch, 0, ch->specials.hunting, TO_ROOM); 
	      } else if (IsAnimal(ch)) {
		act("$n growls.", TRUE, ch, 0, 0, TO_ROOM);
	      }
	      hit(ch,ch->specials.hunting,0);
	      return;
	    }
	  }
	}
      }
      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
      ch->specials.hunting = 0;
      ch->hunt_dist = 0;
    }
  } else if (ch->specials.hunting) {
    if (ch->hunt_dist <= 50) 
      ch->hunt_dist = 100;
    for (k=1;k<=1 && ch->specials.hunting; k++) {
      ch->persist -= 1;
      res = dir_track(ch, ch->specials.hunting);
      if (res!= -1) {
	go_direction(ch, res);
      } else {
	ch->persist = 0;
	ch->specials.hunting = 0;
	ch->hunt_dist = 0;
      }
    }
  } else {
    ch->persist = 0;
  }	       
}

void MobScavenge(struct char_data *ch)
{
  struct obj_data *best_obj=0, *obj=0;
  int max;
  
  if ((real_roomp(ch->in_room))->contents && !number(0,5)) {
    for (max = 1,best_obj = 0,obj = (real_roomp(ch->in_room))->contents;
	 obj; obj = obj->next_content) {
      if (CAN_GET_OBJ(ch, obj)) {
	if (obj->obj_flags.cost > max) {
	  best_obj = obj;
	  max = obj->obj_flags.cost;
	}
      }
    } /* for */
    
    if (best_obj) {
      if (CheckForAnyTrap(ch, best_obj))
	return;
      
      obj_from_room(best_obj);
      obj_to_char(best_obj, ch);
      act("$n gets $p.",FALSE,ch,best_obj,0,TO_ROOM);
    }
  }
}


void check_mobile_activity(int pulse)
{
  register struct char_data *ch; 
  int tick, tm;
  
  tm = pulse % PULSE_MOBILE;    /* this is dependent on P_M = 3*P_T */
  
  if (tm == 0) {
    tick = 0;
  } else if (tm == PULSE_TELEPORT) {
    tick = 1;
  } else if (tm == PULSE_TELEPORT*2) {
    tick = 2;
  }
  
  for (ch = character_list; ch; ch = ch->next) {
    if (IS_MOB(ch)) {
      if (ch->specials.tick == tick) {
	mobile_activity(ch);
      }
    }
  }
}

void mobile_activity(struct char_data *ch)
{
  struct char_data *tmp_ch;
  struct char_data *damsel, *targ;
  struct obj_data *obj, *best_obj, *worst_obj;
  int door, found, max, min, t, res, k;
  char buf[80];
  extern int no_specials;
  
  void do_move(struct char_data *ch, char *argument, int cmd);
  void do_get(struct char_data *ch, char *argument, int cmd);
  
  /* Examine call for special procedure */
  
  /* some status checking for errors */
#if HASH
  if ((ch->in_room < 0) || !hash_find(&room_db,ch->in_room)) {
#else
  if ((ch->in_room < 0) || !room_find(&room_db,ch->in_room)) {
#endif
      log("Char not in correct room.  moving to 50 ");
      char_from_room(ch);
      char_to_room(ch, 50);
  }
    
  if (IS_SET(ch->specials.act, ACT_SPEC) && !no_specials) {
    if (!mob_index[ch->nr].func) {
      log("Attempting to call a non-existing MOB func. (mobact.c)");
      log(ch->player.name);
      REMOVE_BIT(ch->specials.act, ACT_SPEC);
    } else {
      if ((*mob_index[ch->nr].func)	(ch, 0, ""))
	return;
    }
  }
  
  
  /* check to see if the monster is possessed */
  
  if (AWAKE(ch) && (!ch->specials.fighting) && (!ch->desc) &&
      (!IS_SET(ch->specials.act, ACT_POLYSELF))) {
    
    AssistFriend(ch);
    
    if (IS_SET(ch->specials.act, ACT_SCAVENGER)) {
      MobScavenge(ch);
    } /* Scavenger */
    
    
    if (IS_SET(ch->specials.act, ACT_HUNTING)) {
      MobHunt(ch);
    } else if ((!IS_SET(ch->specials.act, ACT_SENTINEL)))
      mobile_wander(ch);
    
    if (GET_HIT(ch) > (GET_MAX_HIT(ch)/2)) {
      if (IS_SET(ch->specials.act, ACT_HATEFUL)) {
	tmp_ch = FindAHatee(ch);
	if (tmp_ch) {
	  if (check_peaceful(ch, "You ask your mortal enemy to step outside to settle matters.\n\r")) {
	    act("$n growls '$N, would you care to step outside where we can settle this?'", TRUE, ch, 0, tmp_ch, TO_ROOM);
	  } else {
	    if (IsHumanoid(ch)) {
	      act("$n screams 'I'm gonna kill you!'", 
		  TRUE, ch, 0, 0, TO_ROOM); 
	    } else if (IsAnimal(ch)) {
	      act("$n growls", TRUE, ch, 0, 0, TO_ROOM);
	    }
	    hit(ch,tmp_ch,0);
	  }
	}
      }
      if (!ch->specials.fighting) {
	if (IS_SET(ch->specials.act, ACT_AFRAID)) {
	  if ((tmp_ch = FindAFearee(ch))!= NULL) {
	    do_flee(ch, "", 0);
	  }
	}
      }
    } else { 
      if (IS_SET(ch->specials.act, ACT_AFRAID)) {
	if ((tmp_ch = FindAFearee(ch))!= NULL) {
	  do_flee(ch, "", 0);
	} else {
	  if (IS_SET(ch->specials.act, ACT_HATEFUL)) {
	    tmp_ch = FindAHatee(ch);
	    if (tmp_ch) {
	      if (check_peaceful(ch, "You ask your mortal enemy to step outside to settle matters.\n\r")) {
		act("$n growls '$N, would you care to step outside where we can settle this?'", TRUE, ch, 0, tmp_ch, TO_ROOM);
	      } else {
		if (IsHumanoid(ch)) {
		  act("$n screams 'I'm gonna get you!'", 
		      TRUE, ch, 0, 0, TO_ROOM); 
		} else if (IsAnimal(ch)) {
		  act("$n growls", TRUE, ch, 0, 0, TO_ROOM);
		}
		hit(ch,tmp_ch,0);
	      }
	    }
	  }	 		
	}
      }
    }
    if (IS_SET(ch->specials.act,ACT_AGGRESSIVE)) {
      for (k=0;k<=5;k++) {
	tmp_ch = FindVictim(ch);
	if (tmp_ch) {
	  if (check_peaceful(ch, "You can't seem to exercise your violent tendencies.\n\r")) {
	    act("$n growls impotently", TRUE, ch, 0, 0, TO_ROOM);
	    return;
	  }
	  hit(ch, tmp_ch, 0);
	  k = 10;
	}
      }
    }
    if (IS_SET(ch->specials.act, ACT_META_AGG)) {
      for (k=0;k<=5;k++) {
	tmp_ch = FindMetaVictim(ch);
	if (tmp_ch) {
	  if (check_peaceful(ch, "You can't seem to exercise your violent tendencies.\n\r")) {
	    act("$n growls impotently", TRUE, ch, 0, 0, TO_ROOM);
	    return;
	  }
	  hit(ch, tmp_ch, 0);
	  k = 10;
	}
      }
      
    }
    if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
      mobile_guardian(ch);
    }    

  } /* If AWAKE(ch)   */
}
  
  
  
int SameRace( struct char_data *ch1, struct char_data *ch2)
{    
    if ((!ch1) || (!ch2))
      return(FALSE);
    
    if (ch1 == ch2)
      return(TRUE);
    
    if (IS_NPC(ch1) && (IS_NPC(ch2)))
      if (mob_index[ch1->nr].virtual ==
	  mob_index[ch2->nr].virtual) {
	return (TRUE);
      }
    
    if (in_group(ch1,ch2))
      return(TRUE);
    
    if (GET_RACE(ch1) == GET_RACE(ch2)) {
      return(TRUE);
    }
    
    return(FALSE);
}
  
int AssistFriend( struct char_data *ch)
{
  struct char_data *damsel, *targ, *tmp_ch, *next;
  int t, found;
  
  
  damsel = 0;
  targ = 0;
  
  if (check_peaceful(ch, ""))
    return;
  
  if (ch->in_room< 0) {
    char_to_room(ch, 0);
    return;
  }
  
  /*
    find the people who are fighting
    */
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;
       tmp_ch=next) {
    next = tmp_ch->next_in_room;
    if (CAN_SEE(ch,tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY)) {
	if (MobFriend(ch, tmp_ch)) {
	  if (tmp_ch->specials.fighting)
	    damsel = tmp_ch;
	}
      }
    }
  }
  
  if (damsel) {
    /*
      check if the people in the room are fighting.
      */
    found = FALSE;
    for (t=1; t<=8 && !found;t++) {
      targ = FindAnAttacker(damsel);
      if (targ) {
	if (targ->specials.fighting)
	    found = TRUE;
      }
    }
    if (targ) {
      if (targ->in_room == ch->in_room) {
	if (!IS_AFFECTED(ch, AFF_CHARM) ||
	    ch->master != targ) {
	  hit(ch,targ,0);
	}
      }
    }
  }
}
  
FindABetterWeapon(struct char_data *mob)
{
  struct obj_data *o, *best;
  /*
    pick up and wield weapons
    Similar code for armor, etc.
    */
  
  /* check whether this mob can wield */
  if (!HasHands(mob)) return(FALSE);
  
  if (!real_roomp(mob->in_room)) return(FALSE);
  
  /* check room */
  best = 0;
  for (o = real_roomp(mob->in_room)->contents; o; o = o->next_content) {
    if (best && IS_WEAPON(o)) {
      if (GetDamage(o,mob) > GetDamage(best,mob)) {
	best = o;
      }
    } else {
      if (IS_WEAPON(o)) {
	best = o;
      }
    }
  }
  /* check inv */
  for (o = mob->carrying; o; o=o->next_content) {
    if (best && IS_WEAPON(o)) {
      if (GetDamage(o,mob) > GetDamage(best,mob)) {
	best = o;
      }
    } else {
      if (IS_WEAPON(o)) {
	best = o;
      }
    }
  }
  
  if (mob->equipment[WIELD]) {
    if (best) {
       if (GetDamage(mob->equipment[WIELD],mob) >= GetDamage(best,mob)) {
          best = mob->equipment[WIELD];
       }
    } else {
      best = mob->equipment[WIELD];
    }
  }

  if (best) {
     if (GetHandDamage(mob) > GetDamage(best, mob)) {
        best = 0;
     }
  } else {
    return(FALSE);  /* nothing to choose from */
  }

  if (best) {
      /*
	out with the old, in with the new
      */
      if (best->carried_by == mob) {
	 if (mob->equipment[WIELD]) {
            do_remove(mob, mob->equipment[WIELD]->name, 0);
	 }
         do_wield(mob, best->name, 0);
      } else if (best->equipped_by == mob) {
	/* do nothing */
	return(TRUE);
      } else {
         do_get(mob, best->name, 0);
      }      
  } else {
    if (mob->equipment[WIELD]) {
      do_remove(mob, mob->equipment[WIELD]->name, 0);
    }
  }
}
      
int GetDamage(struct obj_data *w, struct char_data *ch) 
{
  float ave;
  int num, size, iave;
  /*
    return the average damage of the weapon, with plusses.
  */

  ave = w->obj_flags.value[2]/2.0 + 0.5;
    
  ave *=w->obj_flags.value[1];
  
  ave += GetDamBonus(w);
  /*
    check for immunity:
    */
  iave = ave;
  if (ch->specials.fighting) {
    iave = PreProcDam(ch->specials.fighting, ITEM_TYPE(w), iave);
    iave = WeaponCheck(ch, ch->specials.fighting, ITEM_TYPE(w), iave);
  }
  return(iave);
}
      
int GetDamBonus(struct obj_data *w)
{
   int j, tot=0;

    /* return the damage bonus from a weapon */
   for(j=0; j<MAX_OBJ_AFFECT; j++) {
      if (w->affected[j].location == APPLY_DAMROLL || 
	  w->affected[j].location == APPLY_HITNDAM) {
	  tot += w->affected[j].modifier;	
	}
    }
    return(tot);
}
	      
int GetHandDamage(struct char_data *ch) 
{
  float ave;
  int num, size, iave;
  /*
    return the hand damage of the weapon, with plusses.
	dam += dice(ch->specials.damnodice, ch->specials.damsizedice);

    */

  num  = ch->specials.damnodice;
  size = ch->specials.damsizedice;
  
  ave = size/2.0 + 0.5;
    
  ave *= num;  

  /*
    check for immunity:
    */
  iave = ave;
  if (ch->specials.fighting) {
    iave = PreProcDam(ch->specials.fighting, TYPE_HIT, iave);
    iave = WeaponCheck(ch, ch->specials.fighting, TYPE_HIT, iave);
  }
  return(iave);
}

/*
  check to see if a mob is a friend
*/

int MobFriend( struct char_data *ch, struct char_data *f)
{
 
   if (SameRace(ch, f)) {
     if (IS_GOOD(ch)) {
       if (IS_GOOD(f)) {
          return(TRUE);
        } else {
          return(FALSE);
        }
     } else {
       if (IS_NPC(f))
          return(TRUE);
     }
   } else {
     return(FALSE);
   }
 
}
