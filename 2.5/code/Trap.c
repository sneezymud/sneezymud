/*
  Various utilities for handling traps
*/

#include "structs.h"
#include "utils.h"
#include "spells.h"
#include "trap.h"
#include "comm.h"

extern struct char_data *character_list;
struct room_data *real_roomp(int);
extern int TrapDir[];


void do_settrap( struct char_data *ch, char *arg, int cmd)
{

  /* parse for directions */

/* trap that affects all directions is an AE trap */

  /* parse for type       */
  /* parse for level      */

}

int CheckForMoveTrap(struct char_data *ch, int dir)
{
  struct obj_data *i;

  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
    if ((ITEM_TYPE(i) == ITEM_TRAP) && 
	(IS_SET(GET_TRAP_EFF(i),TRAP_EFF_MOVE)) &&
	(GET_TRAP_CHARGES(i) > 0))
        if (IS_SET(GET_TRAP_EFF(i), TrapDir[dir])) 
	   return(TriggerTrap(ch, i));
  }
  return(FALSE);
}

int CheckForInsideTrap(struct char_data *ch, struct obj_data *i)
{
  struct obj_data *t;

  for (t = i->contains; t; t = t->next_content) {
    if ((ITEM_TYPE(t) == ITEM_TRAP) && 
	(IS_SET(GET_TRAP_EFF(t),TRAP_EFF_OBJECT)) &&
	(GET_TRAP_CHARGES(t) > 0)) {
	   return(TriggerTrap(ch, t));  
	 }
  }
  return(FALSE);
}

int CheckForAnyTrap(struct char_data *ch, struct obj_data *i)
{
    if ((ITEM_TYPE(i) == ITEM_TRAP) && 
	(GET_TRAP_CHARGES(i) > 0))
	   return(TriggerTrap(ch, i));

  return(FALSE);
}



int CheckForGetTrap(struct char_data *ch, struct obj_data *i)
{
    if ((ITEM_TYPE(i) == ITEM_TRAP) && 
	(IS_SET(GET_TRAP_EFF(i),TRAP_EFF_OBJECT)) &&
	(GET_TRAP_CHARGES(i) > 0)) {
	   return(TriggerTrap(ch, i));  
	 }
    return(FALSE);
}



int TriggerTrap( struct char_data *ch, struct obj_data *i)
{
  int adj, fireperc, roll;
  struct char_data *v;

  extern struct dex_app_type dex_app[];

  if (ITEM_TYPE(i) == ITEM_TRAP) {
    if (i->obj_flags.value[TRAP_CHARGES]) {
	adj = GET_TRAP_LEV(i) - GetMaxLevel(ch);
	adj -= dex_app[GET_DEX(ch)].reaction * 5;
	fireperc = 95 + adj;
	roll = number(1,100);
 
    if (!IS_NPC(ch)) {
	if (roll < fireperc) {   /* trap is sprung */
	act("You hear a strange noise...", TRUE, ch, 0, 0, TO_ROOM);
	act("You hear a strange noise...", TRUE, ch, 0, 0, TO_CHAR);
	  GET_TRAP_CHARGES(i) -= 1;
	  if (IS_SET(GET_TRAP_EFF(i),TRAP_EFF_ROOM)) {
	    for (v = real_roomp(ch->in_room)->people;v;v = v->next_in_room) {   
	      FindTrapDamage(v,i);
	    }	    
	  } else {
	    FindTrapDamage(ch,i);
	  }
	return(TRUE);
       }
     }
   }
  }
  return(FALSE);
}

void FindTrapDamage( struct char_data *v, struct obj_data *i)
{
/*
   trap types < 0 are special
*/

  if (GET_TRAP_DAM_TYPE(i) >= 0) {
    TrapDamage(v,GET_TRAP_DAM_TYPE(i),3*GET_TRAP_LEV(i),i);
  } else {
     TrapDamage(v, GET_TRAP_DAM_TYPE(i), 0,i);
  }

}

void TrapDamage(struct char_data *v, int damtype, int amnt, struct obj_data *t)
{
  struct char_data *tmp_ch;
  char buf[132];

  amnt = SkipImmortals(v, amnt);
  if (amnt == -1) {
    return;
  }

  if (IS_AFFECTED(v, AFF_SANCTUARY))
       	   amnt = MAX((int)(amnt/2), 0);  /* Max 1/2 damage when sanct'd */
        
   amnt = PreProcDam(v, damtype, amnt);

   if (saves_spell(v, SAVING_PETRI))
     amnt = MAX((int)(amnt/2),0);

   amnt=MAX(amnt,0);

   GET_HIT(v)-=amnt;

   update_pos(v);

   TrapDam(v, damtype, amnt, t);

   InformMess(v);
   if (GET_POS(v) == POSITION_DEAD) {
      if (!IS_NPC(v)) {
      	  if (real_roomp(v->in_room)->name)
       	     sprintf(buf, "%s killed by a trap at %s",
	       		GET_NAME(v),
	       		real_roomp(v->in_room)->name);
	      vlog(buf);

			/* remove the hatreds of this character */
       	  }
       	  for (tmp_ch=character_list; tmp_ch; tmp_ch=tmp_ch->next) {
              if (Hates(tmp_ch, v)) {
     	         RemHated(tmp_ch, v);
       	       }
	  }

       die(v);
    }
} 

void TrapDam(struct char_data *v, int damtype, int amnt, struct obj_data *t)
{

  char desc[20];
  char buf[132];

  /* easier than dealing with message(ug) */
  switch(damtype) {
  case TRAP_DAM_PIERCE:
    strcpy(desc,"pierced");
    break;
  case TRAP_DAM_SLASH:
    strcpy(desc,"sliced");
    break;
  case TRAP_DAM_BLUNT:
    strcpy(desc,"pounded");
    break;
  case TRAP_DAM_FIRE:
    strcpy(desc,"seared");
    break;
  case TRAP_DAM_COLD:
    strcpy(desc, "frozen");
    break;
  case TRAP_DAM_ACID:
    strcpy(desc, "corroded");
    break;
  case TRAP_DAM_ENERGY:
    strcpy(desc, "blasted");
    break;
  case TRAP_DAM_SLEEP:
    strcpy(desc, "knocked out");
    break;
  case TRAP_DAM_TELEPORT:
    strcpy(desc, "transported");
    break;
  default:
    strcpy(desc, "blown away");
    break;
  }
   
  if ((damtype != TRAP_DAM_TELEPORT)   &&
      (damtype != TRAP_DAM_SLEEP)) {
      if (amnt > 0) {
    sprintf(buf, "$n is %s by $p!", desc);
    act(buf,TRUE,v,t,0,TO_ROOM);
    sprintf(buf, "You are %s by $p!", desc);
    act(buf,TRUE,v,t,0,TO_CHAR);
      } else {
    sprintf(buf, "$n is almost %s by $p!", desc);
    act(buf,TRUE,v,t,0,TO_ROOM);
    sprintf(buf, "You are almost %s by $p!", desc);
    act(buf,TRUE,v,t,0,TO_CHAR);
      }
  }

  if (damtype == TRAP_DAM_TELEPORT) {
    TrapTeleport(v);
  } else if (damtype == TRAP_DAM_SLEEP) {
    TrapSleep(v);
  }

}


void TrapTeleport(struct char_data *v) 
{
	int to_room;
	extern int top_of_world;      /* ref to the top element of world */

    if (saves_spell(v,SAVING_SPELL)) {
       send_to_char("You feel strange, but the effect fades.\n\r",v);
       return;
     } 

     do {
		to_room = number(0, top_of_world);
     } while (IS_SET(real_roomp(to_room)->room_flags, PRIVATE));

     act("$n slowly fade out of existence.", FALSE, v,0,0,TO_ROOM);
     char_from_room(v);
     char_to_room(v, to_room);
     act("$n slowly fade in to existence.", FALSE, v,0,0,TO_ROOM);

     do_look(v, "", 0);

     if (IS_SET(real_roomp(to_room)->room_flags, DEATH) && 
	 GetMaxLevel(v) < LOW_IMMORTAL) {
           death_cry(v);
	   zero_rent(v);
           extract_char(v);
        }
}

void TrapSleep(struct char_data *v)
{

  struct affected_type af;

  if ( !saves_spell(v, SAVING_SPELL) )  {
    af.type      = SPELL_SLEEP;
    af.duration  = 12;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    affect_join(v, &af, FALSE, FALSE);

    if (GET_POS(v)>POSITION_SLEEPING)    {
      act("You feel very sleepy ..... zzzzzz",FALSE,v,0,0,TO_CHAR);
      act("$n goes to sleep.",TRUE,v,0,0,TO_ROOM);
	    GET_POS(v)=POSITION_SLEEPING;
    }
  } else {
    send_to_char("You feel sleepy,but you recover\n\r",v);
  }

}


void InformMess( struct char_data *v)
{

  switch (GET_POS(v)) {
    case POSITION_MORTALLYW:
       	act("$n is mortally wounded, and will die soon, if not aided.", 
	    TRUE, v, 0, 0, TO_ROOM);
       	act("You are mortally wounded, and will die soon, if not aided.", 
	    FALSE, v, 0, 0, TO_CHAR);
	break;
     case POSITION_INCAP:
       	act("$n is incapacitated and will slowly die, if not aided.", 
	    TRUE, v, 0, 0, TO_ROOM);
	act("You are incapacitated and you will slowly die, if not aided.", 
	    FALSE, v, 0, 0, TO_CHAR);
	break;
     case POSITION_STUNNED:
       	act("$n is stunned, but will probably regain consciousness.", 
	    TRUE, v, 0, 0, TO_ROOM);
	act("You're stunned, but you will probably regain consciousness.", 
	    FALSE, v, 0, 0, TO_CHAR);
	break;
      case POSITION_DEAD:
       	act("$n is dead! R.I.P.", TRUE, v, 0, 0, TO_ROOM);
       	act("You are dead!  Sorry...", FALSE, v, 0, 0, TO_CHAR);
       	break;
      default:  /* >= POSITION SLEEPING */
       	break;
      }
}

