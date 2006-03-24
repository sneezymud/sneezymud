/* ************************************************************************
 *  file: limits.c , Limit and gain control module.        Part of DIKUMUD *
 *  Usage: Procedures controling gain and limit.                           *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include <assert.h>
#include "structs.h"
#include "limits.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "race.h"

struct room_data *real_roomp(int);

extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct title_type titles[8][ABS_MAX_LVL];
extern struct room_data *world;
extern const char *RaceName[];
extern const int RacialMax[][4];

/* External procedures */

void update_pos( struct char_data *victim );                 /* in fight.c */
void damage(struct char_data *ch, struct char_data *victim,  /*    "       */
            int damage, int weapontype);
struct time_info_data age(struct char_data *ch);
int ClassSpecificStuff( struct char_data *ch);

char *ClassTitles(struct char_data *ch)
{
  int i, count=0;
  static char buf[256];
  
    for (i = MAGE_LEVEL_IND; i <= RANGER_LEVEL_IND; i++) {
      if (GET_LEVEL(ch, i)) {
	count++;
	if (count > 1) {
	  sprintf(buf + strlen(buf), "/%s",GET_CLASS_TITLE(ch, i,GET_LEVEL(ch,i)));
	} else {
	  sprintf(buf, "%s", GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i)));
	}
      }
    }
  return(buf);
}


/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{
  
  if (age < 15)
    return(p0);                               /* < 15   */
  else if (age <= 29) 
    return (int) (p1+(((age-15)*(p2-p1))/15));  /* 15..29 */
  else if (age <= 44)
    return (int) (p2+(((age-30)*(p3-p2))/15));  /* 30..44 */
  else if (age <= 59)
    return (int) (p3+(((age-45)*(p4-p3))/15));  /* 45..59 */
  else if (age <= 79)
    return (int) (p4+(((age-60)*(p5-p4))/20));  /* 60..79 */
  else
    return(p6);                               /* >= 80 */
}


/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data *ch)
{
  int max;
  
  max = 100;

  if (IS_NPC(ch)) return(max);

  if (HasClass(ch, CLASS_MAGIC_USER))  {
    max += GET_LEVEL(ch, MAGE_LEVEL_IND) * 6;
  }  else if (HasClass(ch, CLASS_CLERIC)) { 
    max += GET_LEVEL(ch, CLERIC_LEVEL_IND) * 5;
  }  else if
       ((HasClass(ch, CLASS_ANTIPALADIN)) ||
        (HasClass(ch, CLASS_PALADIN)) ||
        (HasClass(ch, CLASS_RANGER))) {
    max += GetMaxLevel(ch) * 3;
  } else {
    max = 100;
  }

  max += ch->points.max_mana;   /* bonus mana */
  
  return(max);
}


int hit_limit(struct char_data *ch)
{
  int max;
  
  if (!IS_NPC(ch))
    max = (ch->points.max_hit) +
      (graf(age(ch).year, 2,4,17,14,8,4,3));
  else 
    max = (ch->points.max_hit);
  
  
  /* Class/Level calculations */
  
  /* Skill/Spell calculations */
  
  return (max);
}


int move_limit(struct char_data *ch)
{
  int max;
  
  if (!IS_NPC(ch))
    max = 100 + age(ch).year + GET_CON(ch) + GetTotLevel(ch);
  else
    max = ch->points.max_move;
  
  if (GET_RACE(ch) == RACE_DWARF)
    max -= 30;
  else if (GET_RACE(ch) == RACE_HOBBIT)
    max -= 40;
  else if (GET_RACE(ch) == RACE_GNOME)
    max -= 35;
  else if (GET_RACE(ch) == RACE_ELVEN)
    max += 25;
  else if (GET_RACE(ch) == RACE_OGRE)
    max += 40;

  max += ch->points.max_move;  /* move bonus */
  
  return (max);
}




/* manapoint gain pr. game hour */
int mana_gain(struct char_data *ch)
{
  int gain;
  
  if(IS_NPC(ch)) {
    /* Neat and fast */
    gain = 2*(GetTotLevel(ch)/3);
  } else {
    gain = graf(age(ch).year, 2,4,4,6,8,10,12);
    
    /* Class calculations */
    
    /* Skill/Spell calculations */
    
    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
      gain += gain;
      break;
    case POSITION_RESTING:
      gain+= (gain>>1);  /* Divide by 2 */
      break;
    case POSITION_SITTING:
      gain += (gain>>2); /* Divide by 4 */
      break;
    }

    if (HasClass(ch, CLASS_MAGIC_USER) ||
        HasClass(ch, CLASS_CLERIC))
      gain += gain;
  }

  if (GET_RACE(ch) == RACE_DWARF)
    gain -= 3;
  if (GET_RACE(ch) == RACE_GNOME)
    gain += 3;
  if (GET_RACE(ch) == RACE_ELVEN)
    gain += 2;
  
  if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
    gain >>= 2;
  
  return (gain);
}


int hit_gain(struct char_data *ch)
     /* Hitpoint gain pr. game hour */
{
  int gain;
  
  if(IS_NPC(ch)) {
    if (GetMaxLevel(ch) == 70) {
     gain = 3*GetMaxLevel(ch);
    } else {
     gain = 2*GetMaxLevel(ch);
    }
  } else {
    
    if (GET_POS(ch) == POSITION_FIGHTING) {
      gain = 0;
    } else {
      gain = graf(age(ch).year, 2,8,10,18,6,4,2);
    }
    
    /* Class/Level calculations */
    
    /* Skill/Spell calculations */
    
    /* Position calculations    */
    
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
      gain += gain>>1;
      break;
    case POSITION_RESTING:
      gain+= gain>>2;
      break;
    case POSITION_SITTING:
      gain += gain>>3;
      break;
    }
    
  }
  
  if (GET_RACE(ch) == RACE_DWARF)
    gain += 3;
  if (GET_RACE(ch) == RACE_GNOME)
    gain -= 1;
  if (GET_RACE(ch) == RACE_ELVEN)
    gain -= 1;

   return(gain);
  
}



int move_gain(struct char_data *ch)
     /* move gain pr. game hour */
{
  int gain;
  
  if(IS_NPC(ch)) {
    return(GetTotLevel(ch));	
    /* Neat and fast */
  } else {
    if (GET_POS(ch) != POSITION_FIGHTING)
      gain = 5 + GET_CON(ch);
    else
      gain = 0;
    
    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
      gain += (gain>>1); /* Divide by 2 */
      break;
    case POSITION_RESTING:
      gain+= (gain>>2);  /* Divide by 4 */
      break;
    case POSITION_SITTING:
      gain += (gain>>3); /* Divide by 8 */
      break;
    }
  }
  
  
  if (GET_RACE(ch) == RACE_DWARF)
    gain += 8;
  if (GET_RACE(ch) == RACE_OGRE)
    gain += 12;
  
  if (IS_AFFECTED(ch,AFF_POISON))
    gain >>= 2;
  
  if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
    gain >>= 2;
  
  return (gain);
}



/* Gain maximum in various points */
void advance_level(struct char_data *ch, int class)
{
  int add_hp, i;
  
  extern struct wis_app_type wis_app[];
  extern struct con_app_type con_app[];

  if (GET_LEVEL(ch, class) > 0 && 
      GET_EXP(ch) < titles[class][GET_LEVEL(ch, class)+1].exp) {
    /*  they can't advance here */
    vlog("Bad advance_level");
    return;
  }

  GET_LEVEL(ch, class) += 1;

  if ((OnlyClass(ch, CLASS_WARRIOR)) || (con_app[GET_CON(ch)].hitp < 0))
     add_hp = con_app[GET_CON(ch)].hitp;
  else 
     add_hp = (con_app[GET_CON(ch)].hitp/HowManyClasses(ch));
  

  switch(class) {
    
  case MAGE_LEVEL_IND : {
    if (GET_LEVEL(ch, MAGE_LEVEL_IND) < 12)
      add_hp += number(1,5);
    else
      add_hp += 2;
  } break;
    
  case CLERIC_LEVEL_IND : {
    if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < 12)
      add_hp += number(2,6);
    else
      add_hp += 2;
  } break;
    
  case THIEF_LEVEL_IND : {
    if (GET_LEVEL(ch, THIEF_LEVEL_IND) < 12)
      add_hp += number(2,8);
    else
      add_hp += 3;
  } break;
    
  case WARRIOR_LEVEL_IND : {
    if ((!HasClass(ch, CLASS_THIEF)) && (!HasClass(ch, CLASS_MAGIC_USER)) &&
      (!HasClass(ch, CLASS_CLERIC))) 
      add_hp += number(1,12);
    else if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < 10) 
      add_hp += number(6,10);
    else 
      add_hp += 4;
  } break;
  case ANTIPALADIN_LEVEL_IND : {
    if (GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND) < 12)
      add_hp += number(7,11);
    else
       add_hp += number(5,10);
  } break;
  case PALADIN_LEVEL_IND : {
    if (GET_LEVEL(ch, PALADIN_LEVEL_IND) < 12)
      add_hp += number(7,11);
    else
       add_hp += number(5,10);
  } break; 
case RANGER_LEVEL_IND : {
    if (GET_LEVEL(ch, RANGER_LEVEL_IND) < 12)
      add_hp += number(6,11);
    else
       add_hp += number(3,9);
  } break;  
  case MONK_LEVEL_IND:
    if (GET_LEVEL(ch, MONK_LEVEL_IND) < 12)
      add_hp += number(9,18);
    else
      add_hp += number(5,10);
    break;
  }

  if (!HasClass(ch, CLASS_MONK))
  ch->points.max_hit += MAX(1,(MIN(10, add_hp)));
  else
  ch->points.max_hit += MAX(1,add_hp);
  
  ch->specials.spells_to_learn += MAX(1, wis_app[GET_WIS(ch)].bonus);

  ClassSpecificStuff(ch);
  
  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    for (i = 0; i < 3; i++)
      ch->specials.conditions[i] = -1;

}	



/* Lose in various points */
/*
** Damn tricky for multi-class...
*/

void drop_level(struct char_data *ch, int class)
{
  int add_hp, lin_class;
  
  extern struct wis_app_type wis_app[];
  extern struct con_app_type con_app[];
  
  
  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    return;
  if (GetMaxLevel(ch) == 1)
    return;
  
  add_hp = con_app[GET_CON(ch)].hitp;
  
  switch(class) {
    
  case CLASS_MAGIC_USER : {
    lin_class = MAGE_LEVEL_IND;
    if (GET_LEVEL(ch, MAGE_LEVEL_IND) < 12)
      add_hp += number(2,6);
    else
      add_hp += 2;
  } break;
    
  case CLASS_CLERIC : {
    lin_class = CLERIC_LEVEL_IND;
    if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < 12)
      add_hp += number(3,7);
    else
      add_hp += 3;
  } break;
    
  case CLASS_THIEF : {
    lin_class = THIEF_LEVEL_IND;
    if (GET_LEVEL(ch, THIEF_LEVEL_IND) < 12)
      add_hp += number(4,9);
    else
      add_hp += 3;
  } break;
    
  case CLASS_WARRIOR : {
    lin_class = WARRIOR_LEVEL_IND;
    if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < 10)
      add_hp += number(5,10);
    else
      add_hp += 4;
  } break;
  case CLASS_ANTIPALADIN : {
    if (GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND) < 12)
      add_hp += number(7,11);
    else
       add_hp += number(2,10);
  } break;
  case CLASS_PALADIN : {
    if (GET_LEVEL(ch, PALADIN_LEVEL_IND) < 12)
      add_hp += number(7,11);
    else
       add_hp += number(2,10);
  } break; 
    case CLASS_RANGER : {
    if (GET_LEVEL(ch, RANGER_LEVEL_IND) < 12)
      add_hp += number(6,11);
    else
       add_hp += number(3,9);
  } break;  
  }


  GET_LEVEL(ch, class) -= 1;
  
  if (GET_LEVEL(ch, class) < 1) {
    GET_LEVEL(ch, class) = 1;
  }

  ch->points.max_hit -= MAX(1,add_hp);
  if (ch->points.max_hit < 1)
    ch->points.max_hit = 1;
  
  ch->specials.spells_to_learn -= MAX(2, wis_app[GET_WIS(ch)].bonus);
  
  ch->points.exp = 
    MIN(titles[lin_class][GET_LEVEL(ch, lin_class)].exp, GET_EXP(ch));
  
}	



void set_title(struct char_data *ch)
{
  
  char buf[256];
  
  sprintf(buf, 
     "the %s %s", RaceName[ch->race], ClassTitles(ch));
  
  ch->player.title = (char *) strdup(buf);
}

void gain_exp(struct char_data *ch, int gain)
{
  int i;
  bool is_altered = FALSE;
  char buf[256];
  
  save_char(ch,AUTO_RENT);
  
  if (!IS_IMMORTAL(ch)) {
    if (gain > 0) {
      gain /= HowManyClasses(ch);
      if (GetMaxLevel(ch) == 1) {
	gain *= 2;
      }

      if (!IS_NPC(ch) || (IS_SET(ch->specials.act, ACT_POLYSELF))) {
	for (i = MAGE_LEVEL_IND; i <= RANGER_LEVEL_IND; i++) {
	  if (GET_LEVEL(ch, i)) {
   	    if (GET_EXP(ch) >= titles[i][GET_LEVEL(ch,i)+2].exp) {
	      send_to_char("You must practice at a guild before you can gain any more experience\n\r", ch);
	      GET_EXP(ch) = titles[i][GET_LEVEL(ch,i)+2].exp - 1;
	      return;	      
	    } else if (GET_EXP(ch) >= titles[i][GET_LEVEL(ch,i)+1].exp) {
	      /* do nothing..this is cool */
	    } else if (GET_EXP(ch)+gain >= titles[i][GET_LEVEL(ch,i)+1].exp) {
	      sprintf(buf, "You have gained enough to be a(n) %s\n\r", GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i)+1));
	      send_to_char(buf, ch);
	      send_to_char("You must practice at a guild before you can gain any more experience\n\r", ch);
	      if (GET_EXP(ch)+gain >= titles[i][GET_LEVEL(ch,i)+2].exp) {
		GET_EXP(ch) = titles[i][GET_LEVEL(ch,i)+2].exp - 1;
		return;
	      }
	    }
	  }
	}
      }

      GET_EXP(ch)+=gain;
      if (!IS_NPC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
         for (i=MAGE_LEVEL_IND; i<= RANGER_LEVEL_IND; i++) {
           if (GET_LEVEL(ch,i)) {
             if (GET_EXP(ch) > titles[i][GET_LEVEL(ch,i)+2].exp) {
               GET_EXP(ch) = titles[i][GET_LEVEL(ch,i)+2].exp - 1;
             }
          }
        }
      }
    }

    if (gain < 0) {
      GET_EXP(ch) += gain;
      if (GET_EXP(ch) < 0)
	GET_EXP(ch) = 0;
    }
  }
}


void gain_exp_regardless(struct char_data *ch, int gain, int class)
{
  int i;
  bool is_altered = FALSE;
  
  save_char(ch,AUTO_RENT);
  if (!IS_NPC(ch)) {
    if (gain > 0) {
      GET_EXP(ch) += gain;

      for (i=0;(i<ABS_MAX_LVL) &&(titles[class][i].exp <= GET_EXP(ch)); i++) {
	if (i > GET_LEVEL(ch,class)) {
	  send_to_char("You raise a level\n\r", ch);
	  GET_LEVEL(ch,class) = i;
	  advance_level(ch,class);
	  is_altered = TRUE;
	}
      }
    }
    if (gain < 0) 
      GET_EXP(ch) += gain;
    if (GET_EXP(ch) < 0)
      GET_EXP(ch) = 0;
  }
  if (is_altered)
    set_title(ch);
}

void gain_condition(struct char_data *ch,int condition,int value)
{
  bool intoxicated;
  
  if(GET_COND(ch, condition)==-1) /* No change */
    return;
  
  intoxicated=(GET_COND(ch, DRUNK) > 0);
  
  GET_COND(ch, condition)  += value;
  
  GET_COND(ch,condition) = MAX(0,GET_COND(ch,condition));
  GET_COND(ch,condition) = MIN(24,GET_COND(ch,condition));
  
  if(GET_COND(ch,condition))
    return;
  
  switch(condition){
  case FULL :
    {
      send_to_char("You are hungry.\n\r",ch);
      return;
    }
  case THIRST :
    {
      send_to_char("You are thirsty.\n\r",ch);
      return;
    }
  case DRUNK :
    {
      if(intoxicated)
	send_to_char("You are now sober.\n\r",ch);
      return;
    }
    default : break;
  }
  
}


void check_idling(struct char_data *ch)
{
  int save_room;
  void do_save(struct char_data *ch, char *argument, int cmd);


  if (ch->specials.timer == 10) {
    if (ch->specials.was_in_room == NOWHERE && 
        ch->in_room != NOWHERE && ch->in_room != 3) {
      ch->specials.was_in_room = ch->in_room;
      if (ch->specials.fighting)     	{
	stop_fighting(ch->specials.fighting);
	stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\n\r", ch);
      char_from_room(ch);
      char_to_room(ch, 0);  /* Into room number 0 */
    }
  } else if (ch->specials.timer == 60)  {
     struct obj_cost cost;
     if (ch->in_room != 3) {
      if (ch->in_room != NOWHERE)
	char_from_room(ch);
      
      char_to_room(ch, 3);
      
      if (ch->desc) 
	close_socket(ch->desc);			      
      ch->desc = 0;

     save_obj(ch, &cost,1);
     save_room = ch->in_room;
     extract_char(ch);
     ch->in_room = save_room;
     save_char(ch, ch->in_room);
    }
  }
}



int ObjFromCorpse( struct obj_data *c)
{
  struct obj_data *jj, *next_thing;

        for(jj = c->contains; jj; jj = next_thing) {
	  next_thing = jj->next_content; /* Next in inventory */
	  if (jj->in_obj) {
	     obj_from_obj(jj);
	     if (c->in_obj)
	       obj_to_obj(jj,c->in_obj);
	     else if (c->carried_by)
	       obj_to_room(jj,c->carried_by->in_room);
	     else if (c->in_room != NOWHERE)
	       obj_to_room(jj,c->in_room);
	     else
	       assert(FALSE);
	   } else {
	     /*
	     **  hmm..  it isn't in the object it says it is in.
	     **  don't extract it.
	     */
	     c->contains = 0;
             vlog("Memory lost in ObjFromCorpse.");
	     return(TRUE);
	   }
	}
       	extract_obj(c);
}

int ClassSpecificStuff( struct char_data *ch)
{
 
  if (HasClass(ch, CLASS_WARRIOR) || HasClass(ch, CLASS_MONK)) {
 
    ch->mult_att = 1.0;
      if (HasClass(ch, CLASS_MONK)) {
        ch->mult_att+= (GET_LEVEL(ch, MONK_LEVEL_IND)/16.0);
      }
      /* fix up damage stuff */
      switch(GET_LEVEL(ch, MONK_LEVEL_IND)) {
      case 1:
      case 2:
      case 3:
        ch->specials.damnodice = 2;
        ch->specials.damsizedice = 4;
        break;
      case 4:
      case 5:
        ch->specials.damnodice = 2;
        ch->specials.damsizedice = 5;
        break;
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
        ch->specials.damnodice = 2;
        ch->specials.damsizedice = 6;
        break;
      case 12:
      case 13:
      case 14:
        ch->specials.damnodice = 3;
        ch->specials.damsizedice = 5;
        break;
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
        ch->specials.damnodice = 3;
        ch->specials.damsizedice = 6;
        break;
      case 20:
      case 21:
        ch->specials.damnodice = 4;
        ch->specials.damsizedice = 6;
        break;
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
        ch->specials.damnodice = 5;
        ch->specials.damsizedice = 6;
        break;
      case 27:
      case 28:
      case 29:
        ch->specials.damnodice = 6;
        ch->specials.damsizedice = 6;
        break;
      case 30:
      case 31:
      case 32:
      case 33:
      case 34:
        ch->specials.damnodice = 6;
        ch->specials.damsizedice = 7;
        break;
      case 35:
      case 36:
        ch->specials.damnodice = 7;
        ch->specials.damsizedice = 7;
        break;
      case 37:
      case 38:
      case 39:
      case 40:
      case 41:
        ch->specials.damnodice = 7;
        ch->specials.damsizedice = 7;
        break;
      case 42:
      case 43:
      case 44:
        ch->specials.damnodice = 7;
        ch->specials.damsizedice = 8;
        break;
      case 45:
      case 46:
      case 47:
      case 48:
      case 49:
        ch->specials.damnodice = 8;
        ch->specials.damsizedice = 8;
        break;
      case 50:
        ch->specials.damnodice = 8;
        ch->specials.damsizedice = 9;
        break;
      default:
        ch->specials.damnodice=1;
        ch->specials.damsizedice = 2;
        break;
    }
  }
 
  if (HasClass(ch, CLASS_MONK)) {
    if (GET_LEVEL(ch, MONK_LEVEL_IND) > 10)
      SET_BIT(ch->M_immune, IMM_HOLD);
    if(GET_LEVEL(ch, MONK_LEVEL_IND) > 18)
      SET_BIT(ch->immune, IMM_CHARM);
    if (GET_LEVEL(ch, MONK_LEVEL_IND) > 22)
      SET_BIT(ch->M_immune, IMM_POISON);
    if (GET_LEVEL(ch, MONK_LEVEL_IND) > 36)
      SET_BIT(ch->M_immune, IMM_CHARM);
  }
}
