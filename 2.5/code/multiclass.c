/*
**  Levels:  int levels[4]
*/

/*
**  0 = Mage, 1 = cleric, 3 = thief, 2 = fighter
*/

/*
**  
*/

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"
#include "limits.h"
#include "opinion.h"
#include "race.h"
#include "multiclass.h"


/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_app_type dex_app[];



int GetClassLevel(struct char_data *ch, int class)
{

  if (IS_SET(ch->player.class, class)) {
    return(GET_LEVEL(ch, CountBits(class)-1));
  }
  return(0);
}

int CountBits(int class)
{

  if (class ==  1) return(1);
  if (class ==  2) return(2);
  if (class ==  4) return(3);
  if (class ==  8) return(4);
  if (class == 16) return(5);
  if (class == 32) return(6);
  if (class == 64) return(7);
  if (class ==128) return(8);

}

int OnlyClass( struct char_data *ch, int class)
{
  int i;

  for (i=1;i<=8; i*=2) {
    if (GetClassLevel(ch, i) != 0)
      if (i != class)
	return(FALSE);
  }
  return(TRUE);

}

int IsSingleClass( struct char_data *ch)
{
  int i;

  for (i=1;i<=8; i*=2) {
    if (OnlyClass(ch, i)) 
      return(TRUE);
  }
  return(FALSE);
}

int HasClass(struct char_data *ch, int class)
{
 
  if (!IS_PC(ch)) {
    if (!IS_SET(class, CLASS_MONK)) {
      return(TRUE);
    }
  }
 
  if (IS_SET(ch->player.class, class))
     return(TRUE);
 
  return FALSE;
}

int HowManyClasses(struct char_data *ch)
{
  short i, tot=0;

  for (i=0;i<8;i++) {
    if (GET_LEVEL(ch, i)) {
      tot++;
    }
  }
  if (tot) 
    return(tot);
  else {
    if (IS_SET(ch->player.class, CLASS_MAGIC_USER)) 
      tot++;

    if (IS_SET(ch->player.class, CLASS_WARRIOR)) 
      tot++;
      
    if (IS_SET(ch->player.class, CLASS_THIEF))
      tot++;

    if (IS_SET(ch->player.class, CLASS_CLERIC))
      tot++;

    if (IS_SET(ch->player.class, CLASS_ANTIPALADIN))
      tot++;

    if (IS_SET(ch->player.class, CLASS_PALADIN))
     tot++;

    if (IS_SET(ch->player.class, CLASS_RANGER))
     tot++;

    if (IS_SET(ch->player.class, CLASS_MONK))
       tot++;
  }
    
}


int BestFightingClass(struct char_data *ch)
{

 if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) 
   return(WARRIOR_LEVEL_IND);
 if (GET_LEVEL(ch, PALADIN_LEVEL_IND))
   return(PALADIN_LEVEL_IND);
 if (GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND))
   return(ANTIPALADIN_LEVEL_IND);
 if (GET_LEVEL(ch, RANGER_LEVEL_IND))
   return(RANGER_LEVEL_IND);
 if (GET_LEVEL(ch, MONK_LEVEL_IND))
   return(MONK_LEVEL_IND);
 if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) 
   return(CLERIC_LEVEL_IND);
 if (GET_LEVEL(ch, THIEF_LEVEL_IND)) 
   return(THIEF_LEVEL_IND);
 if (GET_LEVEL(ch, MAGE_LEVEL_IND)) 
   return(MAGE_LEVEL_IND);
 
  vlog("Massive error.. character has no recognized class.");
  vlog(GET_NAME(ch));


  return(1);
}

int BestThiefClass(struct char_data *ch)
{

 if (GET_LEVEL(ch, THIEF_LEVEL_IND)) 
   return(THIEF_LEVEL_IND);
 if (GET_LEVEL(ch, MONK_LEVEL_IND))
   return(MONK_LEVEL_IND);
 if (GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND))
   return(ANTIPALADIN_LEVEL_IND);
 if (GET_LEVEL(ch, MAGE_LEVEL_IND)) 
   return(MAGE_LEVEL_IND);
 if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) 
   return(WARRIOR_LEVEL_IND);
 if (GET_LEVEL(ch, RANGER_LEVEL_IND))
   return(RANGER_LEVEL_IND);
 if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) 
   return(CLERIC_LEVEL_IND);
 if (GET_LEVEL(ch, PALADIN_LEVEL_IND))
  return(PALADIN_LEVEL_IND);
 
  vlog("Massive error.. character has no recognized class.");
  vlog(GET_NAME(ch));


  return(1);
}

int BestMagicClass(struct char_data *ch)
{

 if (GET_LEVEL(ch, MAGE_LEVEL_IND)) 
   return(MAGE_LEVEL_IND);
 if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) 
   return(CLERIC_LEVEL_IND);
 if (GET_LEVEL(ch, PALADIN_LEVEL_IND))
   return(PALADIN_LEVEL_IND);
 if (GET_LEVEL(ch, RANGER_LEVEL_IND))
   return(RANGER_LEVEL_IND);
 if (GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND))
   return(ANTIPALADIN_LEVEL_IND);
 if (GET_LEVEL(ch, THIEF_LEVEL_IND)) 
   return(THIEF_LEVEL_IND);
 if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) 
   return(WARRIOR_LEVEL_IND);
 if (GET_LEVEL(ch, MONK_LEVEL_IND)) 
   return(MONK_LEVEL_IND);

 
  vlog("Massive error.. character has no recognized class.");
  vlog(GET_NAME(ch));


  return(1);
}

int GetSecMaxLev(struct char_data *ch)
{
 if (GET_LEVEL(ch, PALADIN_LEVEL_IND) ||
    (GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND)) ||
    (GET_LEVEL(ch, RANGER_LEVEL_IND)) ||
    (GET_LEVEL(ch, MONK_LEVEL_IND)))  {
   return;
 } else  
   return(GetALevel(ch, 2));
}

int GetALevel(struct char_data *ch, int which)
{
  byte ind[4],j,k,i;

  for (i=MAGE_LEVEL_IND; i<= THIEF_LEVEL_IND; i++) {
    ind[i] = GET_LEVEL(ch,i);
  }

/*
 *  chintzy sort. (just to prove that I did learn something in college)
 */

  for (i = 0; i<= 2; i++) {
    for (j=i+1;j<=3;j++) {
      if (ind[j] > ind[i]) {
	k = ind[i];
        ind[i] = ind[j];
        ind[j] = k;
      }
    }
  }

  if (which > -1 && which < 4) {
    return(ind[which]);
  }
}

int GetThirdMaxLev(struct char_data *ch)
{
   return(GetALevel(ch, 3));
}

int GetMaxLevel(struct char_data *ch)
{
  register int max=0, i;

  for (i=MAGE_LEVEL_IND; i<= RANGER_LEVEL_IND; i++) {
    if (GET_LEVEL(ch, i) > max)
      max = GET_LEVEL(ch,i);
  }

  return(max);
}

int GetTotLevel(struct char_data *ch)
{

  return(GET_LEVEL(ch, 0)+GET_LEVEL(ch,1)+GET_LEVEL(ch,2)+GET_LEVEL(ch,3)
        +GET_LEVEL(ch, 4)+GET_LEVEL(ch,5)+GET_LEVEL(ch,6)+GET_LEVEL(ch,7));

}

void StartLevels(struct char_data *ch)
{

  if (IS_SET(ch->player.class, CLASS_MAGIC_USER)) {
    advance_level(ch, MAGE_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_CLERIC)) {
    advance_level(ch, CLERIC_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_WARRIOR)) {
    advance_level(ch, WARRIOR_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_THIEF)) {
    advance_level(ch, THIEF_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_ANTIPALADIN)) {
    advance_level(ch, ANTIPALADIN_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_RANGER)) {
    advance_level(ch, RANGER_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_MONK)) {
     advance_level(ch, MONK_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_PALADIN))  {
   advance_level(ch, PALADIN_LEVEL_IND);
  }
}


int BestClass(struct char_data *ch)
{

  int max=0, class=0, i;

  for (i=MAGE_LEVEL_IND; i<= RANGER_LEVEL_IND; i++)
    if (max < GET_LEVEL(ch,i)) {
      max = GET_LEVEL(ch, i);
      class = i;
    }

  if (max == 0) { /* eek */
    abort();
  } else {
    return(class);
  }

}
