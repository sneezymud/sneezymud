/* ************************************************************************
 *  file: act.informative.c , Implementation of commands.  Part of DIKUMUD *
 *  Usage : Informative commands.                                          *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "race.h"
#include "trap.h"
#include "hash.h"

/* extern variables */
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct room_data *world;
extern struct str_app_type str_app[];
extern struct dex_skill_type dex_app_skill[];

extern int  top_of_world;
extern int  top_of_zone_table;
extern int  top_of_mobt;
extern int  top_of_objt;
extern int  top_of_p_table;

extern char *dirs[]; 
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern const char *RaceName[];
extern const int RacialMax[][4];

/* extern functions */

struct time_info_data age(struct char_data *ch);
void page_string(struct descriptor_data *d, char *str, int keep_internal);
int track( struct char_data *ch, struct char_data *vict);
int clearpath(int room, int direc);


/* intern functions */
void read_book(Mob *ch, Obj *o, char *arg);
char *DescDamage(float dam);
char *DescMoves(float a);
char *ac_for_score(int a);
char *DescRatio(float f);  /* theirs / yours */
char *DescAttacks(float a);



/* Procedures related to 'look' */

void argument_split_2(char *argument, char *first_arg, char *second_arg) {
  int look_at, found, begin;
  found = begin = 0;
  
  /* Find first non blank */
  for ( ;*(argument + begin ) == ' ' ; begin++);
  
  /* Find length of first word */
  for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)
    
    /* Make all letters lower case, AND copy them to first_arg */
    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(first_arg + look_at) = '\0';
  begin += look_at;
  
  /* Find first non blank */
  for ( ;*(argument + begin ) == ' ' ; begin++);
  
  /* Find length of second word */
  for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)
    
    /* Make all letters lower case, AND copy them to second_arg */
    *(second_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(second_arg + look_at)='\0';
  begin += look_at;
}

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
       char *arg, struct obj_data *equipment[], int *j) {
  
  for ((*j) = 0; (*j) < MAX_WEAR ; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch,equipment[(*j)]))
   if (isname(arg, equipment[(*j)]->name))
     return(equipment[(*j)]);
  
  return (0);
}

char *find_ex_description(char *word, struct extra_descr_data *list) {
  struct extra_descr_data *i;
  
  for (i = list; i; i = i->next)
     if (isname(word, i->keyword))
        return i->description;
  
  return NULL;
}


void do_whozone(Mob *ch, char *argument, int cmd) {
  Descriptor *d;
  Room *rp;
  char buf[256];
  Mob *person;
  int count = 0;
  
  send_to_char("Players:\n\r--------\n\r", ch);
  for (d = descriptor_list; d; d = d->next)
     if (!d->connected && CAN_SEE(ch, d->character) &&
         (rp = real_roomp((person = (d->original ? d->original : d->character))->in_room)) &&
         (rp->zone == real_roomp(ch->in_room)->zone) &&
         ((!IS_AFFECTED(person, AFF_HIDE)) || (IS_IMMORTAL(ch)))) {
           sprintf(buf,"%-25s - %s ", GET_NAME(person), rp->name);
           if (GetMaxLevel(ch) >= LOW_IMMORTAL)
              sprintf(buf + strlen(buf), "[%d]", person->in_room);
           strcat(buf, "\n\r");
           send_to_char(buf, ch);
           count++;
     }
  sprintf(buf, "\n\rTotal visible players: %d\n\r", count);
  send_to_char(buf, ch);
}


void do_who(struct char_data *ch, char *argument, int cmd) {
  struct char_data *k;
  struct descriptor_data *d;
  char buf[256], buf2[256];
  struct char_data *person;
  struct string_block sb;
  int listed = 0, count, lcount, l;
  char arg[256], tempbuf[256];

  *buf = *buf2 = *arg = *tempbuf = '\0';
 
  send_to_char("Players: (Add -? for online help)\n\r--------\n\r",ch);
  lcount = count = 0;
   
  if (!*argument) {                               /* plain old 'who' command */
     for (person = character_list; person; person = person->next)
        if ((!IS_NPC(person)) || (IS_SET(person->specials.act, ACT_POLYSELF))) {
           count++;
           if (CAN_SEE_FOR_WHO(ch, person)) { 
              if (IS_SET(ch->specials.act, PLR_COLOR)) 
                 sprintf(buf, "%s%s%s %s", ANSI_CYAN, GET_NAME(person), ANSI_NORMAL, 
                         (person->player.title ? person->player.title : "(null)"));
              else if (IS_SET(ch->specials.act, PLR_VT100))
                 sprintf(buf, "%s%s%s %s", VT_BOLDTEX, GET_NAME(person), ANSI_NORMAL,
                         (person->player.title ? person->player.title : "(null)"));
              else
                 sprintf(buf, "%s %s", GET_NAME(person), (person->player.title ? 
                         person->player.title : "(null)"));
              if ((IS_IMMORTAL(ch)) && (person->specials.timer >= 4)) {
                 if (!IS_SET(ch->specials.act, PLR_COLOR))
                    sprintf(buf+strlen(buf)," (Idle[%d])",person->specials.timer);
                 else
                    sprintf(buf+strlen(buf)," (%sIdle%s[%s%d%s])", ANSI_VIOLET, 
                            ANSI_NORMAL, ANSI_CYAN, person->specials.timer,ANSI_NORMAL);
              }
              strcat(buf, "\n\r");
              send_to_char(buf, ch);
           }
        }
     sprintf(buf, "\n\rTotal Players : [%d]\n\r", count);
     send_to_char(buf, ch);
     return;
  } else {                                
     argument = one_argument(argument, arg);   /*  'who playername' command */
     if (k = get_char_vis(ch, arg)) {
        if (IS_NPC(k)) {
           send_to_char("\n\rTotal Players : [0]\n\r", ch);
           return;
        }
        sprintf(buf, "%s %s   ", GET_NAME(k), (k->player.title ? k->player.title : "(NULL)"));
        if (!strcmp(k->player.name, "Spawn"))
           sprintf(tempbuf, "Level:[God O' da Jank] ");
        else if (!strcmp(k->player.name, "Stargazer"))
           sprintf(tempbuf, "Level:[De Code Boy] ");
        else if (!strcmp(k->player.name, "Batopr"))
           sprintf(tempbuf, "Level:[The Lord of Worlds]");
        else if (!strcmp(k->player.name, "Trick"))
           sprintf(tempbuf, "Level:[  Goddess  ] ");
        else if (GetMaxLevel(k) == 60)
           sprintf(tempbuf, "Level:[GrandPoobah] ");
        else if (GetMaxLevel(k) == 59) 
           sprintf(tempbuf, "Level:[GrandWizard] ");
        else if (GetMaxLevel(k) == 58)
           sprintf(tempbuf, "Level:[Senior Lord] ");
        else if (GetMaxLevel(k) == 57) 
           sprintf(tempbuf, "Level:[Junior Lord] ");
        else if (GetMaxLevel(k) == 56)
           sprintf(tempbuf, "Level:[    God    ] ");
        else if (GetMaxLevel(k) == 55)
           sprintf(tempbuf, "Level:[Lesser  God] ");
        else if (GetMaxLevel(k) == 54)
           sprintf(tempbuf, "Level:[  DemiGod  ] ");
        else if (GetMaxLevel(k) == 53)
           sprintf(tempbuf, "Level:[   Saint   ] ");
        else if (GetMaxLevel(k) == 52)
           sprintf(tempbuf, "Level:[LowImmortal] ");
        else if (GetMaxLevel(k) == 51)
           sprintf(tempbuf, "Level:[Area Design] ");
        else if (HasClass(k, CLASS_MONK))
           sprintf(tempbuf, "Level:[Monk     %d] ",k->player.level[6]);
        else if (HasClass(k, CLASS_PALADIN))
           sprintf(tempbuf, "Level:[Paladin  %d] ",k->player.level[5]);
        else if (HasClass(k, CLASS_ANTIPALADIN))
           sprintf(tempbuf, "Level:[Antipal  %d] ",k->player.level[4]);
        else if (HasClass(k, CLASS_RANGER))
           sprintf(tempbuf, "Level:[Ranger   %d] ",k->player.level[7]);
        else 
           sprintf(tempbuf,"Level:[%-2d/%-2d/%-2d/%-2d] ", k->player.level[0], k->player.level[1],
                   k->player.level[2], k->player.level[3]);
        strcat(buf, tempbuf);
        strcat(buf, "\n\r");
        send_to_char(buf, ch);
        return;
     } else if (arg[0] == '-') {
        if (index(arg, '?')) {
           if (IS_IMMORTAL(ch)) {
              send_to_char("[-]i=idle l=levels t=title h=hit/mana/move\n\r",ch);
              send_to_char("[-]d=linkdead g=God o=Mort s=stats\n\r",ch);
              send_to_char("[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Anti[6]Paladin[7]Monk[8]Ranger\n\r\n\r", ch);
           } else {
              send_to_char("[-]l=levels t=title g=god o=mort\n\r",ch);
              send_to_char("[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Anti[6]Paladin[7]Monk[8]Ranger\n\r\n\r",ch);
           }
        }
        for (person = character_list; person; person = person->next) {
           if (!IS_NPC(person)) {
              count++;
              if (!person->desc)
                 lcount++;
              if (!(!CAN_SEE(ch, person)
                    || (index(arg, 'g') && !IS_IMMORTAL(person))
                    || (index(arg, 'k') && !IS_SET(person->specials.act, PLR_KILLER)
                                        && !IS_SET(person->specials.act, PLR_OUTLAW))
                    || (index(arg, 'o') && IS_IMMORTAL(person))
                    || (index(arg, '1') && !HasClass(person, CLASS_MAGIC_USER))
                    || (index(arg, '2') && !HasClass(person, CLASS_CLERIC))
                    || (index(arg, '3') && !HasClass(person, CLASS_WARRIOR))
                    || (index(arg, '4') && !HasClass(person, CLASS_THIEF))
                    || (index(arg, '5') && !HasClass(person, CLASS_ANTIPALADIN))
                    || (index(arg, '6') && !HasClass(person, CLASS_PALADIN))
                    || (!index(arg, 'd') && !person->desc)
                    || (index(arg, '7') && !HasClass(person, CLASS_MONK))
                    || (index(arg, '8') && !HasClass(person, CLASS_RANGER)))) {
                 if (!person->desc)
                    sprintf(buf, "[%-12s] ", GET_NAME(person));
                 else if (IS_NPC(person) && IS_SET(person->specials.act, ACT_POLYSELF))
                    sprintf(buf, "(%-14s) ", GET_NAME(person));
                 else
                    sprintf(buf, "%-14s ", GET_NAME(person));
                 listed++;
                 for (l = 1; l <= strlen(arg) ; l++) {
                    switch (arg[l]) {
                         case 'i' : if (IS_IMMORTAL(ch)) {
                                       sprintf(tempbuf,"Idle:[%-3d] ",person->specials.timer);
                                       strcat(buf, tempbuf);
                                    }
                                    break;
                         case 'l' : if (GetMaxLevel(person) == 60)
                                       sprintf(tempbuf, "Level:[GrandPoobah] ");
                                    else if (GetMaxLevel(person) == 59) 
                                       sprintf(tempbuf, "Level:[GrandWizard] ");
                                    else if (GetMaxLevel(person) == 58)
                                       sprintf(tempbuf, "Level:[Senior Lord] ");
                                    else if (GetMaxLevel(person) == 57) 
                                       sprintf(tempbuf, "Level:[Junior Lord] ");
                                    else if (GetMaxLevel(person) == 56)
                                       sprintf(tempbuf, "Level:[    God    ] ");
                                    else if (GetMaxLevel(person) == 55)
                                       sprintf(tempbuf, "Level:[Lesser  God] ");
                                    else if (GetMaxLevel(person) == 54)
                                       sprintf(tempbuf, "Level:[  DemiGod  ] ");
                                    else if (GetMaxLevel(person) == 53)
                                       sprintf(tempbuf,"Level:[   Saint   ] ");
                                    else if (GetMaxLevel(person) == 52)
                                       sprintf(tempbuf,"Level:[LowImmortal] ");
                                    else if (GetMaxLevel(person) == 51)
                                       sprintf(tempbuf,"Level:[Area Design] ");
                                    else if (HasClass(person, CLASS_MONK)) 
                                       sprintf(tempbuf,"Level:[Monk Lev %d] ", person->player.level[6]);
                                    else if (HasClass(person, CLASS_ANTIPALADIN))
                                       sprintf(tempbuf,"Level:[Anti Lev %d] ", person->player.level[4]);
                                    else if (HasClass(person, CLASS_PALADIN))
                                       sprintf(tempbuf,"Level:[Pala Lev %d] ", person->player.level[5]);
                                    else if (HasClass(person, CLASS_RANGER))
                                       sprintf(tempbuf,"Level:[Rang Lev %d] ", person->player.level[7]);
                                    else 
                                       sprintf(tempbuf,"Level:[%-2d/%-2d/%-2d/%-2d] ",
                                               person->player.level[0],person->player.level[1],
                                               person->player.level[2],person->player.level[3]);
                                    strcat(buf,tempbuf);
                                    break;
                         case 'h' : if (IS_IMMORTAL(ch)) {
                                       sprintf(tempbuf,"Hit:[%-3d] Mana:[%-3d] Move:[%-3d] ",
                                               GET_HIT(person),GET_MANA(person),GET_MOVE(person));
                                       strcat(buf,tempbuf);
                                    }
                                    break;
                         case 's' : if (IS_IMMORTAL(ch)) {
                                       sprintf(tempbuf,"[S:%-2d I:%-2d W:%-2d C:%-2d D:%-2d] ",GET_STR(person),
                                               GET_INT(person),GET_WIS(person),GET_CON(person),GET_DEX(person));
                                       strcat(buf,tempbuf);
                                    }
                                    break;
                         case 't' : sprintf(tempbuf," %-16s ", (person->player.title ? person->player.title : "(null)"));
                                    strcat(buf, tempbuf);
                                    break;
                         default  : break;
                      }  /* end of switch statement */
                   }     /* end of for-loop */
						 strcat(buf,"\n\r");
                   send_to_char(buf, ch); 
                }        /* end of 'should I skip this fool' if-statement */
             }           /* end of !NPC(person) loop */
          }					 /* end of 'step through the character list' loop */
       }
		 if (IS_IMMORTAL(ch)) {
		 	 if (!listed)
              sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%)\n\r", count, lcount,
			    	       ((float)lcount / (int)count) * 100);
			 else
              sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%) Number Listed: %d\n\r",
                      count, lcount,((float)lcount / (int)count) * 100,listed);
		 } 
       else
            sprintf(buf, "\n\rTotal players [%d]\n\r", count);
       send_to_char(buf, ch);
       return;
    }
}


/*  A note about "mode" in "show_obj_to_char".
    mode = 0      means "show description + hum/glow/whatever"
    mode = 1..4   means "show short description + hum/glow/whatever"
    mode = 5      means "deal with object type, maybe do hum/glow/whatever"
    mode = 6      means "JUST show hum/glow/whatever"    
    Obviously, all this is really hokey for a procedure that gets used so
    often.  This probably needs to get redone eventually.  I am happy to
    say that none of this is my mess.  - SG     */

#define HEADER_TXT_NOTE "There is something written upon it:\n\r\n\r"
#define DRINKCON_TXT_NOTE "It looks like a drink container."
void show_obj_to_char(Obj *object, Mob *ch, int mode) {
   char buffer[MAX_STRING_LENGTH];

   if (!mode && object->description)                  /* mode = 0 */
      strcpy(buffer, object->description);            
   else if ((mode < 5) && object->short_description) 
      strcpy(buffer, object->short_description);
   else if (mode == 5) {
      switch (object->obj_flags.type_flag) {
         case ITEM_NOTE:      if (object->action_description) {
                                 strcpy(buffer, HEADER_TXT_NOTE);
                                 strcat(buffer, object->action_description);
                                 page_string(ch->desc, buffer, 1);
                              } else
                                 send_to_char("It's blank.\n\r", ch);
                              return;
         case ITEM_DRINKCON:  strcpy(buffer, DRINKCON_TXT_NOTE);
                              break;
         default:             strcpy(buffer,"You see nothing special..");
                              break;
      }  
   }
   else 
      buffer[0] = '\0';       /* we need this before we start strcat'ing */
      
   if (IS_OBJ_STAT(object,ITEM_INVISIBLE))
      strcat(buffer,"(invisible)");
   if (IS_OBJ_STAT(object,ITEM_ANTI_GOOD)&&IS_AFFECTED(ch,AFF_DETECT_EVIL))
      strcat(buffer,"..It glows red!");
   if (IS_OBJ_STAT(object,ITEM_MAGIC) && IS_AFFECTED(ch,AFF_DETECT_MAGIC))
      strcat(buffer,"..It glows blue!");
   if (IS_OBJ_STAT(object,ITEM_GLOW))
      strcat(buffer,"..It glows with an aura!");
   if (IS_OBJ_STAT(object,ITEM_HUM))
      strcat(buffer,"..It hums softly!");
   strcat(buffer, "\n\r");
   send_to_char(buffer, ch);
}


void show_mult_obj_to_char(struct obj_data *object, struct char_data *ch, int mode, int num)
{
  char buffer[MAX_STRING_LENGTH];
  char tmp[10];

  *buffer = '\0';
  
  if (!mode && object->description)
    strcpy(buffer,object->description);
  else  if (object->short_description && ((mode == 1) ||
                 (mode == 2) || (mode==3) || (mode == 4))) 
    strcpy(buffer,object->short_description);
  else if (mode == 5) {
    if (object->obj_flags.type_flag == ITEM_NOTE)       {
      if (object->action_description)    {
   strcpy(buffer, "There is something written upon it:\n\r\n\r");
   strcat(buffer, object->action_description);
   page_string(ch->desc, buffer, 1);
      }  else
   act("It's blank.", FALSE, ch,0,0,TO_CHAR);
      return;
    } else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
      strcpy(buffer,"You see nothing special..");
    }  else  { /* ITEM_TYPE == ITEM_DRINKCON */
      strcpy(buffer, "It looks like a drink container.");
    }
  }
  
  if (mode != 3) { 
    if (IS_OBJ_STAT(object,ITEM_INVISIBLE)) {
      strcat(buffer,"(invisible)");
    }
    if (IS_OBJ_STAT(object,ITEM_ANTI_GOOD) && 
   IS_AFFECTED(ch,AFF_DETECT_EVIL)) {
      strcat(buffer,"..It glows red!");
    }
    if (IS_OBJ_STAT(object,ITEM_MAGIC) && IS_AFFECTED(ch,AFF_DETECT_MAGIC)) {
      strcat(buffer,"..It glows blue!");
    }
    if (IS_OBJ_STAT(object,ITEM_GLOW)) {
      strcat(buffer,"..It has a soft glowing aura!");
    }
    if (IS_OBJ_STAT(object,ITEM_HUM)) {
      strcat(buffer,"..It emits a faint humming sound!");
    }
  }
  
  if (num>1) {
    sprintf(tmp,"[%d]", num);
    strcat(buffer, tmp);
  }
  strcat(buffer, "\n\r");
  page_string(ch->desc, buffer, 1);
}

void list_obj_in_room(struct obj_data *list, struct char_data *ch)
{
  struct obj_data *i, *cond_ptr[50];
  int Inventory_Num = 1, num;
  int k, cond_top, cond_tot[50], found=FALSE;
  char buf[MAX_STRING_LENGTH];
  
  cond_top = 0; 
  
  for (i=list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      if (cond_top< 50) {
   found = FALSE;
   for (k=0;(k<cond_top&& !found);k++) {
     if (cond_top>0) {
       if ((i->item_number == cond_ptr[k]->item_number) &&
      (i->description && cond_ptr[k]->description &&
       !strcmp(i->description,cond_ptr[k]->description))){
         cond_tot[k] += 1;
         found=TRUE;
       }
     }          
   }
   if (!found) {
     cond_ptr[cond_top] = i;
     cond_tot[cond_top] = 1;
     cond_top+=1;
   }
      } else {
   if ((ITEM_TYPE(i) == ITEM_TRAP) || (GET_TRAP_CHARGES(i) > 0)) {
     num = number(1,100);
     if (ch->skills && num < ch->skills[SKILL_FIND_TRAP].learned/10)
       show_obj_to_char(i,ch,0);
   } else {
     show_obj_to_char(i,ch,0);       
   }
      }
    }
  }
  
  if (cond_top) {
    for (k=0; k<cond_top; k++) {
      if ((ITEM_TYPE(cond_ptr[k]) == ITEM_TRAP) && 
     (GET_TRAP_CHARGES(cond_ptr[k]) > 0)) {
   num = number(1,100);
   if (ch->skills && num < ch->skills[SKILL_FIND_TRAP].learned/10)
     if (cond_tot[k] > 1) {
       sprintf(buf,"[%2d] ",Inventory_Num++);
       send_to_char(buf,ch);
       show_mult_obj_to_char(cond_ptr[k],ch,0,cond_tot[k]);
     } else {
       show_obj_to_char(cond_ptr[k],ch,0);
     }
      } else {
   if (cond_tot[k] > 1) {
     sprintf(buf,"[%2d] ",Inventory_Num++);
     send_to_char(buf,ch);
     show_mult_obj_to_char(cond_ptr[k],ch,0,cond_tot[k]);
   } else {
     show_obj_to_char(cond_ptr[k],ch,0);
   }
      }
    }
  }
}

void list_obj_in_heap(struct obj_data *list, struct char_data *ch)
{
  struct obj_data *i, *cond_ptr[50];
  int k, cond_top, cond_tot[50], found=FALSE;  
  char buf[MAX_STRING_LENGTH];
  
  int Num_Inventory = 1;
  cond_top = 0; 
  
  for (i=list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      if (cond_top< 50) {
   found = FALSE;
   for (k=0;(k<cond_top&& !found);k++) {
     if (cond_top>0) {
       if ((i->item_number == cond_ptr[k]->item_number) &&
      (i->short_description && cond_ptr[k]->short_description &&
       (!strcmp(i->short_description,cond_ptr[k]->short_description)))){
         cond_tot[k] += 1;
         found=TRUE;
       }
     }        
   }
   if (!found) {
     cond_ptr[cond_top] = i;
     cond_tot[cond_top] = 1;
     cond_top+=1;
   }
      } else {
   show_obj_to_char(i,ch,2);
      }
    }
  }
  
  if (cond_top) {
    for (k=0; k<cond_top; k++) {
      sprintf(buf,"[%2d] ",Num_Inventory++);
      send_to_char(buf,ch);
      if (cond_tot[k] > 1) {
   Num_Inventory += cond_tot[k] - 1;
   show_mult_obj_to_char(cond_ptr[k],ch,2,cond_tot[k]);
      } else {
   show_obj_to_char(cond_ptr[k],ch,2);
      } 
    }
  }
}

#if 0
void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode, 
            bool show) {
  char buf[MAX_STRING_LENGTH];
  int Num_In_Bag = 1;
  struct obj_data *i;
  bool found;
  
  found = FALSE;
  for ( i = list ; i ; i = i->next_content ) { 
    if (CAN_SEE_OBJ(ch,i)) {
      sprintf(buf,"[%2d] ",Num_In_Bag++);
      send_to_char(buf,ch);
      show_obj_to_char(i, ch, mode);
      found = TRUE;
    }    
  }  
  if ((! found) && (show)) send_to_char("Nothing\n\r", ch);
}
#endif


void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
  char buffer[MAX_STRING_LENGTH];
  int j, found, percent;
  struct obj_data *tmp_obj;
  
  if (!mode) {
    if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i)) {
      if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
   send_to_char("You sense a hidden life form in the room.\n\r", ch);
      return;
    }
    
    if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos)){
      /* A player char or a mobile without long descr, or not in default pos.*/
      if (!IS_NPC(i)) { 
   strcpy(buffer,GET_NAME(i));
   strcat(buffer," ");
   if (GET_TITLE(i))
     strcat(buffer,GET_TITLE(i));
      } else {
   strcpy(buffer, i->player.short_descr);
   CAP(buffer);
      }
      
      if ( IS_AFFECTED(i,AFF_INVISIBLE) || i->invis_level == LOW_IMMORTAL)
   strcat(buffer," (invisible)");
      if ( IS_AFFECTED(i,AFF_CHARM))
   strcat(buffer," (pet)");
      
      switch(GET_POS(i)) {
      case POSITION_STUNNED  : 
   strcat(buffer," is lying here, stunned."); break;
      case POSITION_INCAP    : 
   strcat(buffer," is lying here, incapacitated."); break;
      case POSITION_MORTALLYW: 
   strcat(buffer," is lying here, mortally wounded."); break;
      case POSITION_DEAD     : 
   strcat(buffer," is lying here, dead."); break;
      case POSITION_STANDING : 
   strcat(buffer," is standing here."); break;
      case POSITION_SITTING  : 
   strcat(buffer," is sitting here.");  break;
      case POSITION_RESTING  : 
   strcat(buffer," is resting here.");  break;
      case POSITION_SLEEPING : 
   strcat(buffer," is sleeping here."); break;
      case POSITION_FIGHTING :
   if (i->specials.fighting) {
     
     strcat(buffer," is here, fighting ");
     if (i->specials.fighting == ch)
       strcat(buffer," YOU!");
     else {
       if (i->in_room == i->specials.fighting->in_room)
         if (IS_NPC(i->specials.fighting))
      strcat(buffer, i->specials.fighting->player.short_descr);
         else
      strcat(buffer, GET_NAME(i->specials.fighting));
       else
         strcat(buffer, "someone who has already left.");
     }
   } else /* NIL fighting pointer */
     strcat(buffer," is here struggling with thin air.");
   break;
   default : strcat(buffer," is floating here."); break;
      }
      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
   if (IS_EVIL(i))
     strcat(buffer, " (Red Aura)");
      }
      
      strcat(buffer,"\n\r");
      send_to_char(buffer, ch);
    } else {  /* npc with long */
      
      if (IS_AFFECTED(i,AFF_INVISIBLE))
   strcpy(buffer,"*");
      else
   *buffer = '\0';
      
      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
   if (IS_EVIL(i))
     strcat(buffer, " (Red Aura)");
      }
      
      strcat(buffer, i->player.long_descr);
      
      send_to_char(buffer, ch);
    }
    
   if (IS_AFFECTED(i,AFF_SANCTUARY))
     act(".....$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
   if (affected_by_spell(i, SPELL_FIRESHIELD)) 
     act(".....$n glows with a red light!", FALSE, i, 0, ch, TO_VICT);
   if (IS_AFFECTED(i, AFF_FLYING))
     act(".....$n is hovering above the ground!", FALSE, i, 0 , ch, TO_VICT);
   if (IS_AFFECTED(i, AFF_BREWING))
     act(".....$n is brewing a potion!", FALSE, i, 0, ch, TO_VICT);
   if (check_slots(i) && GET_POS(i) == POSITION_SITTING)
     act(".....$n is sitting at the slot machine!", FALSE,i,0,ch,TO_VICT);
 
  } else if (mode == 1) {
    
    if (i->player.description)
      send_to_char(i->player.description, ch);
    else {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }
    
    /* Show a character to another */
    
    if (GET_MAX_HIT(i) > 0)
      percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
    else
      percent = -1; /* How could MAX_HIT be < 1?? */
    
    if (IS_NPC(i))
      strcpy(buffer, i->player.short_descr);
    else
      strcpy(buffer, GET_NAME(i));
    
    if (percent >= 100)
      strcat(buffer, " is in an excellent condition.\n\r");
    else if (percent >= 90)
      strcat(buffer, " has a few scratches.\n\r");
    else if (percent >= 75)
      strcat(buffer, " has some small wounds and bruises.\n\r");
    else if (percent >= 50)
      strcat(buffer, " has quite a few wounds.\n\r");
    else if (percent >= 30)
      strcat(buffer, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
      strcat(buffer, " looks pretty hurt.\n\r");
    else if (percent >= 0)
      strcat(buffer, " is in an awful condition.\n\r");
    else
      strcat(buffer, " is bleeding awfully from big wounds.\n\r");
    
    send_to_char(buffer, ch);
    
    found = FALSE;
    for (j=0; j< MAX_WEAR; j++) {
      if (i->equipment[j]) {
   if (CAN_SEE_OBJ(ch,i->equipment[j])) {
     found = TRUE;
   }
      }
    }
    if (found) {
      act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);
      for (j=0; j< MAX_WEAR; j++) {
   if (i->equipment[j]) {
     if (CAN_SEE_OBJ(ch,i->equipment[j])) {
       send_to_char(where[j],ch);
       show_obj_to_char(i->equipment[j],ch,1);
     }
   }
      }
    }
    if (HasClass(ch, CLASS_THIEF) && (ch != i) &&
   (!IS_IMMORTAL(ch))){
      found = FALSE;
      send_to_char
   ("\n\rYou attempt to peek at the inventory:\n\r", ch);
      for(tmp_obj = i->carrying; tmp_obj; 
     tmp_obj = tmp_obj->next_content) {
   if (CAN_SEE_OBJ(ch, tmp_obj) && 
       (number(0,MAX_MORT) < GetMaxLevel(ch))) {
     show_obj_to_char(tmp_obj, ch, 1);
     found = TRUE;
   }
      }
      if (!found)
   send_to_char("You can't see anything.\n\r", ch);
    } else if (IS_IMMORTAL(ch)) {
      send_to_char("Inventory:\n\r",ch);
      for(tmp_obj = i->carrying; tmp_obj; 
     tmp_obj = tmp_obj->next_content) {
   show_obj_to_char(tmp_obj, ch, 1);
   found = TRUE;
      }
      if (!found) {
   send_to_char("Nothing\n\r",ch);
      }
    }
    
  } else if (mode == 2) {
    
    /* Lists inventory */
    act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
    list_obj_in_heap(i->carrying,ch);
  }
}


void show_mult_char_to_char(struct char_data *i, struct char_data *ch, int mode, int num)
{
  char buffer[MAX_STRING_LENGTH];
  char tmp[10];
  int j, found, percent;
  struct obj_data *tmp_obj;
  
  if (!mode) {
    if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i)) {
      if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
   if (num==1)
     send_to_char("You sense a hidden life form in the room.\n\r", ch);
   else 
     send_to_char("You sense hidden life form in the room.\n\r", ch);              
      return;
    }
    
    if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos)){
      /* A player char or a mobile without long descr, or not in default pos. */
      if (!IS_NPC(i)) { 
   strcpy(buffer,GET_NAME(i));
   strcat(buffer," ");
   if (GET_TITLE(i))
     strcat(buffer,GET_TITLE(i));
      } else {
   strcpy(buffer, i->player.short_descr);
   CAP(buffer);
      }
      
      if ( IS_AFFECTED(i,AFF_INVISIBLE))
   strcat(buffer," (invisible)");
      if ( IS_AFFECTED(i,AFF_CHARM))
   strcat(buffer," (pet)");
      
      switch(GET_POS(i)) {
      case POSITION_STUNNED  : 
   strcat(buffer," is lying here, stunned."); break;
      case POSITION_INCAP    : 
   strcat(buffer," is lying here, incapacitated."); break;
      case POSITION_MORTALLYW: 
   strcat(buffer," is lying here, mortally wounded."); break;
      case POSITION_DEAD     : 
   strcat(buffer," is lying here, dead."); break;
      case POSITION_STANDING : 
   strcat(buffer," is standing here."); break;
      case POSITION_SITTING  : 
   strcat(buffer," is sitting here.");  break;
      case POSITION_RESTING  : 
   strcat(buffer," is resting here.");  break;
      case POSITION_SLEEPING : 
   strcat(buffer," is sleeping here."); break;
      case POSITION_FIGHTING :
   if (i->specials.fighting) {
     
     strcat(buffer," is here, fighting ");
     if (i->specials.fighting == ch)
       strcat(buffer," YOU!");
     else {
       if (i->in_room == i->specials.fighting->in_room)
         if (IS_NPC(i->specials.fighting))
      strcat(buffer, i->specials.fighting->player.short_descr);
         else
      strcat(buffer, GET_NAME(i->specials.fighting));
       else
         strcat(buffer, "someone who has already left.");
     }
   } else /* NIL fighting pointer */
     strcat(buffer," is here struggling with thin air.");
   break;
   default : strcat(buffer," is floating here."); break;
      }
      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
   if (IS_EVIL(i))
     strcat(buffer, " (Red Aura)");
      }
      
      if (num > 1) {
   sprintf(tmp," [%d]", num);
   strcat(buffer, tmp);
      }
      strcat(buffer,"\n\r");
      send_to_char(buffer, ch);
    } else {  /* npc with long */
      
      if (IS_AFFECTED(i,AFF_INVISIBLE))
   strcpy(buffer,"*");
      else
   *buffer = '\0';
      
      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
   if (IS_EVIL(i))
     strcat(buffer, " (Red Aura)");
      }
      
      strcat(buffer, i->player.long_descr);
      
      /* this gets a little annoying */
      
      if (num > 1) {
   while ((buffer[strlen(buffer)-1]=='\r') ||
          (buffer[strlen(buffer)-1]=='\n') ||
          (buffer[strlen(buffer)-1]==' ')) {
     buffer[strlen(buffer)-1] = '\0';
   }
   sprintf(tmp," [%d]\n\r", num);
   strcat(buffer, tmp);
      }
      
      send_to_char(buffer, ch);
    }
    
    if (IS_AFFECTED(i,AFF_SANCTUARY))
      act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    if (affected_by_spell(i, SPELL_FIRESHIELD)) 
      act("$n glows with a red light!", FALSE, i, 0, ch, TO_VICT);
    if (affected_by_spell(i, SPELL_FLY))
      act("$n is hovering above the ground!", FALSE, i, 0, ch, TO_VICT);
 
  } else if (mode == 1) {
    
    if (i->player.description)
      send_to_char(i->player.description, ch);
    else {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }
    
    /* Show a character to another */
    
    if (GET_MAX_HIT(i) > 0)
      percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
    else
      percent = -1; /* How could MAX_HIT be < 1?? */
    
    if (IS_NPC(i))
      strcpy(buffer, i->player.short_descr);
    else
      strcpy(buffer, GET_NAME(i));
    
    if (percent >= 100)
      strcat(buffer, " is in an excellent condition.\n\r");
    else if (percent >= 90)
      strcat(buffer, " has a few scratches.\n\r");
    else if (percent >= 75)
      strcat(buffer, " has some small wounds and bruises.\n\r");
    else if (percent >= 50)
      strcat(buffer, " has quite a few wounds.\n\r");
    else if (percent >= 30)
      strcat(buffer, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
      strcat(buffer, " looks pretty hurt.\n\r");
    else if (percent >= 0)
      strcat(buffer, " is in an awful condition.\n\r");
    else
      strcat(buffer, " is bleeding awfully from big wounds.\n\r");
    
    send_to_char(buffer, ch);
    
    found = FALSE;
    for (j=0; j< MAX_WEAR; j++) {
      if (i->equipment[j]) {
   if (CAN_SEE_OBJ(ch,i->equipment[j])) {
     found = TRUE;
   }
      }
    }
    if (found) {
      act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);
      for (j=0; j< MAX_WEAR; j++) {
   if (i->equipment[j]) {
     if (CAN_SEE_OBJ(ch,i->equipment[j])) {
       send_to_char(where[j],ch);
       show_obj_to_char(i->equipment[j],ch,1);
     }
   }
      }
    }
    if (HasClass(ch, CLASS_THIEF) && (ch != i)) {
      found = FALSE;
      send_to_char("\n\rYou attempt to peek at the inventory:\n\r", ch);
      for(tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
   if (CAN_SEE_OBJ(ch,tmp_obj)&&(number(0,MAX_MORT) < GetMaxLevel(ch))) {
     show_obj_to_char(tmp_obj, ch, 1);
     found = TRUE;
   }
      }
      if (!found)
   send_to_char("You can't see anything.\n\r", ch);
    }
    
  } else if (mode == 2) {
    
    /* Lists inventory */
    act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
    list_obj_in_heap(i->carrying,ch);
  }
}


void list_char_in_room(struct char_data *list, struct char_data *ch)
{
  struct char_data *i, *cond_ptr[50];
  int k, cond_top, cond_tot[50], found=FALSE;  
  
  cond_top = 0; 
  
  for (i=list; i; i = i->next_in_room) {
    if ( (ch!=i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
           (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE))) ) {
      if (cond_top< 50) {
   found = FALSE;
   if (IS_NPC(i)) {
     for (k=0;(k<cond_top&& !found);k++) {
       if (cond_top>0) {
         if (i->nr == cond_ptr[k]->nr &&
        (GET_POS(i) == GET_POS(cond_ptr[k])) &&
        (i->specials.affected_by==cond_ptr[k]->specials.affected_by) &&
        (i->specials.fighting == cond_ptr[k]->specials.fighting) &&
        (i->player.short_descr && cond_ptr[k]->player.short_descr &&
         0==strcmp(i->player.short_descr,cond_ptr[k]->player.short_descr))) {
      cond_tot[k] += 1;
      found=TRUE;
         }
       }
     }
   }
   if (!found) {
     cond_ptr[cond_top] = i;
     cond_tot[cond_top] = 1;
     cond_top+=1;
   }
      } else {
   show_char_to_char(i,ch,0);
      }
    }
  }
  
  if (cond_top) {
    for (k=0; k<cond_top; k++) {
      if (cond_tot[k] > 1) {
   show_mult_char_to_char(cond_ptr[k],ch,0,cond_tot[k]);
      } else {
   show_char_to_char(cond_ptr[k],ch,0);
      }
    }
  }
}


void list_char_to_char(struct char_data *list, struct char_data *ch, 
             int mode) {
  struct char_data *i;
  
  for (i = list; i ; i = i->next_in_room) {
    if ( (ch!=i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
           (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE))) )
      show_char_to_char(i,ch,0); 
  } 
}


void do_look(struct char_data *ch, char *argument, int cmd)
{
  char buffer[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int keyword_no, res;
  int j, bits, temp;
  bool found;
  struct obj_data *tmp_object, *found_object;
  struct char_data *tmp_char;
  char *tmp_desc;
  static char *keywords[]= { 
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "in",
    "at",
    "",  /* Look at '' case */
    "room",
    "test",
    "\n" };
  
  if (!ch->desc)
    return;
  
  if (GET_POS(ch) < POSITION_SLEEPING)
    send_to_char("You can't see anything but stars!\n\r", ch);
  else if (GET_POS(ch) == POSITION_SLEEPING)
    send_to_char("You can't see anything, you're sleeping!\n\r", ch);
  else if ( IS_AFFECTED(ch, AFF_BLIND) )
    send_to_char("You can't see a damn thing, you're blinded!\n\r", ch);
  else if  ((IS_DARK(ch->in_room)) && (!IS_IMMORTAL(ch)) &&
            (!IS_AFFECTED(ch, AFF_TRUE_SIGHT))) {
    send_to_char("It is very dark in here...\n\r", ch);
    if (IS_AFFECTED(ch, AFF_INFRAVISION)) {
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
    }
  } else {
 
    only_argument(argument, arg1);
 
    if (0==strn_cmp(arg1,"at",2) && isspace(arg1[2])) {
      only_argument(argument+3, arg2);
      keyword_no = 7;
    } else if (0==strn_cmp(arg1,"in",2) && isspace(arg1[2])) {
      only_argument(argument+3, arg2);
      keyword_no = 6;
    } else {
      keyword_no = search_block(arg1, keywords, FALSE);
    }
 
    if ((keyword_no == -1) && *arg1) {
      keyword_no = 7;
      only_argument(argument, arg2);
    }
    
 
    found = FALSE;
    tmp_object = 0;
    tmp_char     = 0;
    tmp_desc     = 0;
    
    switch(keyword_no) {
      /* look <dir> */
    case 0 :
    case 1 :
    case 2 : 
    case 3 : 
    case 4 :
    case 5 : {   
      struct room_direction_data        *exitp;
      exitp = EXIT(ch, keyword_no);
      if (exitp) {
        if (exitp->general_description) {
          send_to_char(exitp-> general_description, ch);
        } else {
          send_to_char("You see nothing special.\n\r", ch);
        }
        
        if (IS_SET(exitp->exit_info, EX_CLOSED) && 
            (exitp->keyword)) {
           if ((strcmp(fname(exitp->keyword), "secret")) &&
              (!IS_SET(exitp->exit_info, EX_SECRET))) {
              sprintf(buffer, "The %s is closed.\n\r", 
                    fname(exitp->keyword));
              send_to_char(buffer, ch);
            } 
         } else {
           if (IS_SET(exitp->exit_info, EX_ISDOOR) &&
              exitp->keyword) {
              sprintf(buffer, "The %s is open.\n\r",
                      fname(exitp->keyword));
              send_to_char(buffer, ch);
            }
         }
      } else {
        send_to_char("You see nothing special.\n\r", ch);
      }
      if (exitp && exitp->to_room && (!IS_SET(exitp->exit_info, EX_ISDOOR) ||
         (!IS_SET(exitp->exit_info, EX_CLOSED)))) {
        if (IS_AFFECTED(ch, AFF_SCRYING) || IS_IMMORTAL(ch)) {
          struct room_data      *rp;
          sprintf(buffer,"You look %swards.\n\r", dirs[keyword_no]);
          send_to_char(buffer, ch);
 
          sprintf(buffer,"$n looks %swards.", dirs[keyword_no]);
          act(buffer, FALSE, ch, 0, 0, TO_ROOM);
 
          rp = real_roomp(exitp->to_room);
          if (!rp) {
            send_to_char("You see swirling chaos.\n\r", ch);
          } else if(exitp) {
            sprintf(buffer, "%d look", exitp->to_room);
            do_at(ch, buffer, 0);
          } else {
            send_to_char("You see nothing special.\n\r", ch);
          }
        }
      }
    }
      break;
      
      /* look 'in'      */
    case 6: {
      if (*arg2) {
        /* Item carried */
        bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
                            FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
        
        if (bits) { /* Found something */
          if (GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON)        {
            if (tmp_object->obj_flags.value[1] <= 0) {
              act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
            } else {
              temp=((tmp_object->obj_flags.value[1]*3)/tmp_object->obj_flags.value[0]);
              sprintf(buffer,"It's %sfull of a %s liquid.\n\r",
                      fullness[temp],color_liquid[tmp_object->obj_flags.value[2]]);
              send_to_char(buffer, ch);
            }
          } else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
            if (!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)) {
              send_to_char(fname(tmp_object->name), ch);
              switch (bits) {
              case FIND_OBJ_INV :
                send_to_char(" (carried) : \n\r", ch);
                break;
              case FIND_OBJ_ROOM :
                send_to_char(" (here) : \n\r", ch);
                break;
              case FIND_OBJ_EQUIP :
                send_to_char(" (used) : \n\r", ch);
                break;
              }
              list_obj_in_heap(tmp_object->contains, ch);
            } else
              send_to_char("It is closed.\n\r", ch);
          } else {
            send_to_char("That is not a container.\n\r", ch);
          }
        } else { /* wrong argument */
          send_to_char("You do not see that item here.\n\r", ch);
        }
      } else { /* no argument */
        send_to_char("Look in what?!\n\r", ch);
      }
    }
      break;
      
      /* look 'at'      */
    case 7 : {
      if (*arg2) {
        bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
                            FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
        if (tmp_char) {
          show_char_to_char(tmp_char, ch, 1);
          if (ch != tmp_char) {
            act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
            act("$n looks at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
          }
          return;
        }

     /* The following check is for the teleport messages
        we send via extra descriptions. We dont want folx to be
        able to look _tele_ which is the keyword for the code
        to know an ex description is the teleport message.  
                                                          Russ */
         if (!strcmp(arg2, "_tele_")) {
           send_to_char("Look at what?\n\r", ch);
           return;
         }
        /* 
          Search for Extra Descriptions in room and items 
          */
        
        /* Extra description in room?? */
        if (!found) {
          tmp_desc = find_ex_description(arg2, 
                                         real_roomp(ch->in_room)->ex_description);
          if (tmp_desc) {
            page_string(ch->desc, tmp_desc, 0);
            return; 
          }
        }
        
        /* extra descriptions in items */
        
        /* Equipment Used */
        if (!found) {
          for (j = 0; j< MAX_WEAR && !found; j++) {
            if (ch->equipment[j]) {
              if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
                tmp_desc = find_ex_description(arg2, 
                                               ch->equipment[j]->ex_description);
                if (tmp_desc) {
                  page_string(ch->desc, tmp_desc, 1);
                  found = TRUE;
                }
              }
            }
          }
        }
        /* In inventory */
        if (!found) {
          for(tmp_object = ch->carrying; 
              tmp_object && !found; 
              tmp_object = tmp_object->next_content) {
            if CAN_SEE_OBJ(ch, tmp_object) {
              tmp_desc = find_ex_description(arg2, 
                                             tmp_object->ex_description);
              if (tmp_desc) {
                page_string(ch->desc, tmp_desc, 1);
                found = TRUE;
              }
            }
          }
        }
        /* Object In room */
        
        if (!found) {
          for(tmp_object = real_roomp(ch->in_room)->contents; 
              tmp_object && !found; 
              tmp_object = tmp_object->next_content) {
            if CAN_SEE_OBJ(ch, tmp_object) {
              tmp_desc = find_ex_description(arg2, 
                                             tmp_object->ex_description);
              if (tmp_desc) {
                page_string(ch->desc, tmp_desc, 1);
                  found = TRUE;
                return;
              }
            }
          }
        }
        /* wrong argument */
        if (bits) { /* If an object was found */
          if (!found) {
/* note this is here, instead of in show_obj_to_char, because we need
   to consider what section (if any) the player specified in the book
   and that can ONLY be found via arg2, which isn't passed into
   show_obj_to_char  - SG */
            if (GET_ITEM_TYPE(found_object) == ITEM_BOOK)
               read_book(ch, found_object, arg2);
            else
               show_obj_to_char(found_object, ch, 5); /* Show no-description */
          }
          else
            show_obj_to_char(found_object, ch, 6); 
          /* Find hum, glow etc */
        } else if (!found) {
          send_to_char("You do not see that here.\n\r", ch);
        }
      } else {
        /* no argument */       
        send_to_char("Look at what?\n\r", ch);
      }
    }
      break;
      
      /* look ''                */ 
    case 8 : {
    if (IS_SET(ch->specials.act, PLR_COLOR)) { 
      send_to_char(ANSI_VIOLET, ch);
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
      send_to_char(ANSI_NORMAL, ch);
    } else if (IS_SET(ch->specials.act, PLR_VT100)) {
      send_to_char(VT_BOLDTEX, ch);
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
      send_to_char(ANSI_NORMAL, ch);
    } else {
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
    }
        send_to_char(real_roomp(ch->in_room)->description, ch);
      
      if (!IS_NPC(ch)) {
        if (IS_SET(ch->specials.act, PLR_HUNTING)) {
          if (ch->specials.hunting) {
            res = track(ch, ch->specials.hunting);
            if (!res) {
              ch->specials.hunting = 0;
              ch->hunt_dist = 0;
              REMOVE_BIT(ch->specials.act, PLR_HUNTING);
            }
          } else {
            ch->hunt_dist = 0;
            REMOVE_BIT(ch->specials.act, PLR_HUNTING);
          }
        }
      } else {
        if (IS_SET(ch->specials.act, ACT_HUNTING)) {
          if (ch->specials.hunting) {
            res = track(ch, ch->specials.hunting);
            if (!res) {
              ch->specials.hunting = 0;
              ch->hunt_dist = 0;
              REMOVE_BIT(ch->specials.act, ACT_HUNTING);
            }  
          } else {
            ch->hunt_dist = 0;
            REMOVE_BIT(ch->specials.act, ACT_HUNTING);
          }
        }
      }
      
      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
      
    }
      break;
      
      
      /* wrong arg      */
    case -1 : 
      send_to_char("Sorry, I didn't understand that!\n\r", ch);
      break;
      
      /* look 'room' */
    case 9 : {
      
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
      send_to_char(real_roomp(ch->in_room)->description, ch);
      
      if (!IS_NPC(ch)) {
        if (IS_SET(ch->specials.act, PLR_HUNTING)) {
          if (ch->specials.hunting) {
            res = track(ch, ch->specials.hunting);
            if (!res) {
              ch->specials.hunting = 0;
              ch->hunt_dist = 0;
              REMOVE_BIT(ch->specials.act, PLR_HUNTING);
            }
          } else {
            ch->hunt_dist = 0;
            REMOVE_BIT(ch->specials.act, PLR_HUNTING);
          }
        }
      } else {
        if (IS_SET(ch->specials.act, ACT_HUNTING)) {
          if (ch->specials.hunting) {
            res = track(ch, ch->specials.hunting);
            if (!res) {
              ch->specials.hunting = 0;
              ch->hunt_dist = 0;
              REMOVE_BIT(ch->specials.act, ACT_HUNTING);
            }  
          } else {
            ch->hunt_dist = 0;
            REMOVE_BIT(ch->specials.act, ACT_HUNTING);
          }
        }
      }
      
      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
      
    }
      break;
    case 10: {
    if (IS_SET(ch->specials.act, PLR_COLOR)) { 
      send_to_char(ANSI_VIOLET, ch);
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
      send_to_char(ANSI_NORMAL, ch);
    } else if (IS_SET(ch->specials.act, PLR_VT100)) {
      send_to_char(VT_BOLDTEX, ch);
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
      send_to_char(ANSI_NORMAL, ch);
    } else {
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
    }
      if (!IS_SET(ch->specials.act, PLR_BRIEF))
        send_to_char(real_roomp(ch->in_room)->description, ch);
      
      if (!IS_NPC(ch)) {
        if (IS_SET(ch->specials.act, PLR_HUNTING)) {
          if (ch->specials.hunting) {
            res = track(ch, ch->specials.hunting);
            if (!res) {
              ch->specials.hunting = 0;
              ch->hunt_dist = 0;
              REMOVE_BIT(ch->specials.act, PLR_HUNTING);
            }
          } else {
            ch->hunt_dist = 0;
            REMOVE_BIT(ch->specials.act, PLR_HUNTING);
          }
        }
      } else {
        if (IS_SET(ch->specials.act, ACT_HUNTING)) {
          if (ch->specials.hunting) {
            res = track(ch, ch->specials.hunting);
            if (!res) {
              ch->specials.hunting = 0;
              ch->hunt_dist = 0;
              REMOVE_BIT(ch->specials.act, ACT_HUNTING);
            }  
          } else {
            ch->hunt_dist = 0;
            REMOVE_BIT(ch->specials.act, ACT_HUNTING);
          }
        }
      }
      
      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
      
    }
      break;

    }
  }
}
 
/* end of look */
 
 
 
 
void do_read(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  
  /* This is just for now - To be changed later.! */
  sprintf(buf,"at %s",argument);
  do_look(ch,buf,15);
}



void do_examine(struct char_data *ch, char *argument, int cmd)
{
  char name[100], buf[100];
  int bits;
  struct char_data *tmp_char;
  struct obj_data *tmp_object;
  
  sprintf(buf,"at %s",argument);
  do_look(ch,buf,15);
  
  one_argument(argument, name);
  
  if (!*name)
    {
      send_to_char("Examine what?\n\r", ch);
      return;
    }
  
  bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
            FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
  
  if (tmp_object) {
    if ((GET_ITEM_TYPE(tmp_object)==ITEM_DRINKCON) ||
   (GET_ITEM_TYPE(tmp_object)==ITEM_CONTAINER)) {
      send_to_char("When you look inside, you see:\n\r", ch);
      sprintf(buf,"in %s",argument);
      do_look(ch,buf,15);
    }
    if (tmp_object->obj_flags.type_flag == ITEM_FIREWEAPON)
    {
   sprintf (buf,"There are %d shots remaining.\n\r",
      tmp_object->obj_flags.value[3] );
   send_to_char ( buf, ch );
    }
  }
}



void do_exits(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[256];
  struct room_direction_data    *exitdata;
  extern char *exits[];
  
  *buf = '\0';

  for (door = 0; door <= 5; door++) {
    exitdata = EXIT(ch,door);
    if (exitdata) {
      if (!real_roomp(exitdata->to_room)) {
   /* don't print unless immortal */
   if (IS_IMMORTAL(ch)) {
     sprintf(buf + strlen(buf), "%s - swirling chaos of #%d\n\r",
        exits[door], exitdata->to_room);
   }
      } else if (exitdata->to_room != NOWHERE &&
       (!IS_SET(exitdata->exit_info, EX_CLOSED) ||
        IS_IMMORTAL(ch))) {
   if (IS_DARK(exitdata->to_room))
     sprintf(buf + strlen(buf), "%s - Too dark to tell", exits[door]);
   else
     sprintf(buf + strlen(buf), "%s - %s", exits[door],
        real_roomp(exitdata->to_room)->name);
   if (IS_SET(exitdata->exit_info, EX_CLOSED))
     strcat(buf, " (closed)");
   strcat(buf, "\n\r");
      }
    }
  }
  
  send_to_char("Obvious exits:\n\r", ch);
  
  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char("None.\n\r", ch);
}


void do_score(struct char_data *ch, char *argument, int cmd)
{
  struct time_info_data playing_time;
  static char buf[100];
  extern const struct title_type titles[8][ABS_MAX_LVL];
  
  struct time_info_data real_time_passed(time_t t2, time_t t1);
  
  sprintf(buf, "You are %d years old.", GET_AGE(ch));
  
  if ((age(ch).month == 0) && (age(ch).day == 0))
    strcat(buf," It's your birthday today.\n\r");
  else
    strcat(buf,"\n\r");
  send_to_char(buf, ch);
  
  sprintf(buf, 
     "You have %.1f%% hit points and %.1f%% mana. You are %s.\n\r",
     (((float) (GET_HIT(ch)))/((float) GET_MAX_HIT(ch))*100),
     (((float) (GET_MANA(ch)))/((float) GET_MAX_MANA(ch))*100),
     DescMoves(((float) GET_MOVE(ch))/((float) GET_MAX_MOVE(ch))));
  send_to_char(buf,ch);

  if (GET_ALIGNMENT(ch) < -750)
      send_to_char("Your alignment is PURE EVIL.\n\r", ch);
  else if (GET_ALIGNMENT(ch) < -500)
      send_to_char("Your alignment is EVIL.\n\r", ch);
  else if (GET_ALIGNMENT(ch) < -250)
      send_to_char("Your alignment is SLIGHTLY EVIL.\n\r", ch);
  else if (GET_ALIGNMENT(ch) < 0)
      send_to_char("Your alignment is CHAOTIC NEUTRAL.\n\r", ch); 
  else if (GET_ALIGNMENT(ch) < 250)
      send_to_char("Your alignment is CHAOTIC GOOD.\n\r", ch);
  else if (GET_ALIGNMENT(ch) < 500)
      send_to_char("Your alignment is SLIGHTY GOOD.\n\r", ch);
  else if (GET_ALIGNMENT(ch) < 750)
      send_to_char("Your alignment is GOOD.\n\r", ch);
  else
      send_to_char("Your alignment is TOTALLY PURE.\n\r", ch);

  
  sprintf(buf,"You have scored %d exp, and have %d gold coins.\n\r",
     GET_EXP(ch),GET_GOLD(ch));
  send_to_char(buf,ch);

  if ((HasClass(ch, CLASS_MAGIC_USER)) || (HasClass(ch, CLASS_CLERIC)) ||
      (HasClass(ch, CLASS_THIEF)) || (HasClass(ch, CLASS_WARRIOR))) { 
  sprintf(buf, "Your levels: %d/%d/%d/%d\n\r",GET_LEVEL(ch, MAGE_LEVEL_IND),
                  GET_LEVEL(ch, CLERIC_LEVEL_IND),
                  GET_LEVEL(ch, WARRIOR_LEVEL_IND),
                  GET_LEVEL(ch, THIEF_LEVEL_IND));
  } else if (HasClass(ch, CLASS_ANTIPALADIN)) {
  sprintf(buf, "Your levels: ANTI lev %2d\n\r",
   GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND));
  } else if (HasClass(ch, CLASS_PALADIN)) {
  sprintf(buf, "Your levels: PALA lev %2d\n\r",
   GET_LEVEL(ch, PALADIN_LEVEL_IND));
  } else if (HasClass(ch, CLASS_MONK)) {
  sprintf(buf, "Your levels: MONK lev %2d\n\r",
   GET_LEVEL(ch, MONK_LEVEL_IND));
  } else if (HasClass(ch, CLASS_RANGER)) {
  sprintf(buf, "Your levels: RANG lev %2d\n\r",
   GET_LEVEL(ch, RANGER_LEVEL_IND));
  }
  send_to_char(buf,ch);
     
  sprintf(buf,"This ranks you as %s %s\n\r", GET_NAME(ch), GET_TITLE(ch));
  send_to_char(buf,ch);
  
  playing_time = real_time_passed((time(0)-ch->player.time.logon) +
              ch->player.time.played, 0);
  sprintf(buf,"You have been playing for %d days and %d hours.\n\r",
     playing_time.day,
     playing_time.hours);          
  send_to_char(buf, ch);                
  
  switch(GET_POS(ch)) {
  case POSITION_DEAD : 
    send_to_char("You are DEAD!\n\r", ch); break;
  case POSITION_MORTALLYW :
    send_to_char("You are mortally wounded!, you should seek help!\n\r", ch); break;
  case POSITION_INCAP : 
    send_to_char("You are incapacitated, slowly fading away\n\r", ch); break;
  case POSITION_STUNNED : 
    send_to_char("You are stunned! You can't move\n\r", ch); break;
  case POSITION_SLEEPING : 
    send_to_char("You are sleeping.\n\r",ch); break;
  case POSITION_RESTING  : 
    send_to_char("You are resting.\n\r",ch); break;
  case POSITION_SITTING  : 
    send_to_char("You are sitting.\n\r",ch); break;
  case POSITION_FIGHTING :
    if (ch->specials.fighting)
      act("You are fighting $N.\n\r", FALSE, ch, 0,
     ch->specials.fighting, TO_CHAR);
    else
      send_to_char("You are fighting thin air.\n\r", ch);
    break;
  case POSITION_STANDING : 
    send_to_char("You are standing.\n\r",ch); break;
    default :
      send_to_char("You are floating.\n\r",ch); break;
  }
  
}


void do_time(struct char_data *ch, char *argument, int cmd)
{
  char buf[100], *suf;
  int weekday, day;
  extern struct time_info_data time_info;
  extern const char *weekdays[];
  extern const char *month_name[];
  
  sprintf(buf, "It is %d o'clock %s, on ",
     ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
     ((time_info.hours >= 12) ? "pm" : "am") );
  
  weekday = ((35*time_info.month)+time_info.day+1) % 7;/* 35 days in a month */
  
  strcat(buf,weekdays[weekday]);
  strcat(buf,"\n\r");
  send_to_char(buf,ch);
  
  day = time_info.day + 1;   /* day in [1..35] */
  
  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";
  
  sprintf(buf, "The %d%s Day of the %s, Year %d.\n\r",
     day,
     suf,
     month_name[time_info.month],
     time_info.year);
  
  send_to_char(buf,ch);
}


void do_weather(struct char_data *ch, char *argument, int cmd)
{
  extern struct weather_data weather_info;
  static char buf[100];
  static char *sky_look[4]= {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning"};
  
  if (OUTSIDE(ch)) {
    sprintf(buf, 
       "The sky is %s and %s.\n\r",
       sky_look[weather_info.sky],
       (weather_info.change >=0 ? "you feel a warm wind from south" :
        "your foot tells you bad weather is due"));
    send_to_char(buf,ch);
  } else
    send_to_char("You have no feeling about the weather at all.\n\r", ch);
}


void do_help(struct char_data *ch, char *argument, int cmd) {
  static char helppath[200];
  static char topic[MAX_INPUT_LENGTH];
  char *cp;

  if (!ch)
     return;

  for (; (isspace(*argument)); argument++);
  if (*argument == '\0')
     start_page_file(ch->desc, HELP_PAGE_FILE, "General help unavailable.\n\r");
  else {
     for (cp = topic; (isalnum(*argument) || (*argument == '@') || 
           (*argument == ' ') || (*argument == '-')); cp++, argument++)
         if (isupper(*argument))
            *cp = tolower(*argument);
    else
       *cp = *argument;
     *cp = '\0';
     if (topic[0] == '\0') {
        send_to_char("Sorry!  No help on that topic.\n\r", ch);
        return;
     }
     if (GetMaxLevel(ch) > 51) {
        sprintf(helppath, "%s/%s", IMMORTAL_HELP_PATH, topic);
        if (start_page_file(ch->desc, helppath, ""))
      return;
     } 
     if (GetMaxLevel(ch) > 50) {
        sprintf(helppath, "%s/%s", BUILDER_HELP_PATH, topic);
        if (start_page_file(ch->desc, helppath, ""))
      return;
     }
     sprintf(helppath, "%s%s", HELP_PATH, topic);     
     start_page_file(ch->desc, helppath, "Sorry!  No help on that topic.\n\r");
  }
}


do_wizhelp(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int no, i;
  extern char *command[];        /* The list of commands (interpreter.c)  */
  /* First command is command[0]           */
  extern struct command_info cmd_info[];
  /* cmd_info[1] ~~ commando[0]            */
  
  if (IS_NPC(ch))
    return;
  
  send_to_char("The following privileged comands are available:\n\r\n\r", ch);
  
  *buf = '\0';
  
  for (no = 1, i = 0; *command[i] != '\n'; i++)
    if ((GetMaxLevel(ch) >= cmd_info[i+1].minimum_level) &&
   (cmd_info[i+1].minimum_level >= LOW_IMMORTAL))  {
      
      sprintf(buf + strlen(buf), "%-10s", command[i]);
      if (!(no % 7))
   strcat(buf, "\n\r");
      no++;
    }
  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}





void do_users(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH], line[200];
  
  struct descriptor_data *d;
  
  strcpy(buf, "Connections:\n\r------------\n\r");
  
  for (d = descriptor_list; d; d = d->next){
      if (d->character && d->character->player.name){
     if(d->original)
       sprintf(line, "%-16s: ", d->original->player.name);
     else
       sprintf(line, "%-16s: ", d->character->player.name);
   }
      else
   strcpy(line, "UNDEFINED       : ");
      sprintf(line + strlen(line), "[%s]\n\r", (d->host?d->host:"????"));
     strcat(buf, line);
    }
  send_to_char(buf, ch);
}


void do_inventory(struct char_data *ch, char *argument, int cmd) {
  
  send_to_char("You are carrying:\n\r", ch);
  list_obj_in_heap(ch->carrying, ch);
}


void do_equipment(struct char_data *ch, char *argument, int cmd) {
  int j,Worn_Index;
  bool found;
  char String[256];
  
  send_to_char("You are using:\n\r", ch);
  found = FALSE;
  for (Worn_Index = j=0; j< MAX_WEAR; j++) {
    if (ch->equipment[j]) {
      Worn_Index++;
      sprintf(String,"[%d] %s",Worn_Index,where[j]);
      send_to_char(String,ch);
      if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
   show_obj_to_char(ch->equipment[j],ch,1);
   found = TRUE;
      } else {
   send_to_char("Something.\n\r",ch);
   found = TRUE;
      }
    }
  }
  if(!found) {
    send_to_char(" Nothing.\n\r", ch);
  }
}


void do_credits(struct char_data *ch, char *argument, int cmd) {
 
  if (ch) 
    start_page_file(ch->desc, CREDITS_FILE, "Credits file being revised!\n\r");
}


void do_atlas(struct char_data *ch, char *argument, int cmd) {

  char name[200];
  int volume;

  only_argument(argument, name);

  if (!*name) {
    send_to_char("An argument (1-4) is needed to look at different atlas volumes.\n\r", ch);
    return;
   } else if (isdigit(*name)) {
    volume = atoi(name);

    if (volume == 1) {
     if (ch)
      start_page_file(ch->desc, ATLAS1_FILE, "");
    } else if (volume == 2) {
     if (ch)
      start_page_file(ch->desc, ATLAS2_FILE, "");
    } else if (volume == 3) {
     if (ch)
      start_page_file(ch->desc, ATLAS3_FILE, "");
    } else if (volume == 4) {
     if (ch)
      start_page_file(ch->desc, ATLAS4_FILE, "");
    } else {
       send_to_char("Please enter a volume number (1-4)\n\r", ch);
       return;
    }
   } else {
     send_to_char("Volume numbers need to be just that...numbers.\n\r", ch);
     return;
   }
}




void do_news(struct char_data *ch, char *argument, int cmd) {

  if (ch)
     start_page_file(ch->desc, NEWS_FILE, "No news is good news!\n\r");
}


void do_info(struct char_data *ch, char *argument, int cmd) {
 
  if (ch)
     start_page_file(ch->desc, INFO_FILE, "No info available!\n\r"); 
}


void do_wizlist(struct char_data *ch, char *argument, int cmd) {
  if (ch)
     start_page_file(ch->desc, WIZLIST_FILE, "Sorry, wizlist under construction!\n\r");  
}

static int which_number_mobile(struct char_data *ch, struct char_data *mob)
{
  struct char_data      *i;
  char  *name;
  int   number;
  
  name = fname(mob->player.name);
  for (i=character_list, number=0; i; i=i->next) {
    if (isname(name, i->player.name) && i->in_room != NOWHERE) {
      number++;
      if (i==mob)
   return number;
    }
  }
  return 0;
}

char *numbered_person(struct char_data *ch, struct char_data *person)
{
  static char buf[MAX_STRING_LENGTH];
  if (IS_NPC(person) && IS_IMMORTAL(ch)) {
    sprintf(buf, "%d.%s", which_number_mobile(ch, person),
       fname(person->player.name));
  } else {
    strcpy(buf, PERS(person, ch));
  }
  return buf;
}

static void do_where_person(struct char_data *ch, struct char_data *person, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];
  
  sprintf(buf, "%-30s- %s ", PERS(person, ch),
     (person->in_room > -1 ? real_roomp(person->in_room)->name : "Nowhere"));
  
  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    sprintf(buf+strlen(buf),"[%d]", person->in_room);
  
  strcpy(buf+strlen(buf), "\n\r");
  
  append_to_string_block(sb, buf);
}

static void do_where_object(struct char_data *ch, struct obj_data *obj,
             int recurse, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];
  if (obj->in_room != NOWHERE) { /* object in a room */
    sprintf(buf, "%-30s- %s [%d]\n\r",
       obj->short_description,
       real_roomp(obj->in_room)->name,
       obj->in_room);
  } else if (obj->carried_by != NULL) { /* object carried by monster */
    sprintf(buf, "%-30s- carried by %s\n\r",
       obj->short_description,
       numbered_person(ch, obj->carried_by));
  } else if (obj->equipped_by != NULL) { /* object equipped by monster */
    sprintf(buf, "%-30s- equipped by %s\n\r",
       obj->short_description,
       numbered_person(ch, obj->equipped_by));
  } else if (obj->in_obj) { /* object in object */
    sprintf(buf, "%-30s- in %s\n\r",
       obj->short_description,
       obj->in_obj->short_description);
  } else {
    sprintf(buf, "%-30s- god doesn't even know where...\n\r",
       obj->short_description);
  }
  if (*buf)
    append_to_string_block(sb, buf);
  
  if (recurse) {
    if (obj->in_room != NOWHERE)
      return;
    else if (obj->carried_by != NULL)
      do_where_person(ch, obj->carried_by, sb);
    else if (obj->equipped_by != NULL)
      do_where_person(ch, obj->equipped_by, sb);
    else if (obj->in_obj != NULL)
      do_where_object(ch, obj->in_obj, TRUE, sb);
  }
}

void do_where(struct char_data *ch, char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char  *nameonly;
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int   number, count;
  struct string_block   sb;
  
  only_argument(argument, name);
  
  if (!*name) {
    if (GetMaxLevel(ch) < LOW_IMMORTAL)         {
      send_to_char("What are you looking for?\n\r", ch);
      return;
    } else      {
      init_string_block(&sb);
      append_to_string_block(&sb, "Players:\n\r--------\n\r");
      
      for (d = descriptor_list; d; d = d->next) {
   if (d->character && (d->connected == CON_PLYNG) && (d->character->in_room != NOWHERE)) {
     if (d->original)   /* If switched */
       sprintf(buf, "%-20s - %s [%d] In body of %s\n\r",
          d->original->player.name,
          real_roomp(d->character->in_room)->name,
          d->character->in_room,
          fname(d->character->player.name));
     else
       sprintf(buf, "%-20s - %s [%d]\n\r",
          d->character->player.name,
          real_roomp(d->character->in_room)->name,
          d->character->in_room);
     
     append_to_string_block(&sb, buf);
   }
      }
      page_string_block(&sb,ch);
      destroy_string_block(&sb);
      return;
    }
  }
  
  if (isdigit(*name)) {
    nameonly = name;
    count = number = get_number(&nameonly);
  } else {
    count = number = 0;
  }
  
  *buf = '\0';
  
  init_string_block(&sb);
  
  for (i = character_list; i; i = i->next)
    if (isname(name, i->player.name) && CAN_SEE(ch, i) )        {
      if ((i->in_room != NOWHERE) &&
     ((GetMaxLevel(ch)>=LOW_IMMORTAL) || (real_roomp(i->in_room)->zone ==
                    real_roomp(ch->in_room)->zone))) {
   if (number==0 || (--count) == 0) {
     if (number==0) {
       sprintf(buf, "[%2d] ", ++count); /* I love short circuiting :) */
       append_to_string_block(&sb, buf);
     }
     do_where_person(ch, i, &sb);
     *buf = 1;
     if (number!=0)
       break;
   }
   if (GetMaxLevel(ch) < LOW_IMMORTAL)
     break;
      }
    }
  
  /*  count = number;*/
  
  if (GetMaxLevel(ch) >= LOW_IMMORTAL ) {
    for (k = object_list; k; k = k->next)
      if (isname(name, k->name) && CAN_SEE_OBJ(ch, k)) {
   if (number==0 || (--count)==0) {
     if (number==0) {
       sprintf(buf, "[%2d] ", ++count);
       append_to_string_block(&sb, buf);
     }
     do_where_object(ch, k, number!=0, &sb);
     *buf = 1;
     if (number!=0)
       break;
   }
      }
  }
  
  if (!*sb.data)
    send_to_char("Couldn't find any such thing.\n\r", ch);
  else
    page_string_block(&sb, ch);
  destroy_string_block(&sb);
}




void do_levels(struct char_data *ch, char *argument, int cmd)
{
  int i, RaceMax, class;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct string_block sb;
  
  extern const struct title_type titles[8][ABS_MAX_LVL];

  *buf = '\0';
/*
**  get the class
*/

  for (;isspace(*argument);argument++);

  if (!*argument) {
    send_to_char("You must supply a class!\n\r", ch);
    return;
  }

  switch(*argument) {
  case 'C':
  case 'c':
    class = CLERIC_LEVEL_IND;
    break;
  case 'F':
  case 'f':
  case 'W':
  case 'w':
    class = WARRIOR_LEVEL_IND;
    break;
  case 'M':
  case 'm':
    class = MAGE_LEVEL_IND;
    break;
  case 'T':
  case 't':
    class = THIEF_LEVEL_IND;
    break;
  case 'A':
  case 'a':
    class = ANTIPALADIN_LEVEL_IND;
    break;
  case 'R':
  case 'r':
    class = RANGER_LEVEL_IND;
    break;
  case 'p':
  case 'P':
    class = PALADIN_LEVEL_IND;
    break;
  case 'K':
  case 'k':
    class = MONK_LEVEL_IND;
    break;
  default:
    sprintf(buf, "I don't recognize %s\n\r", argument);
    send_to_char(buf,ch);
    return;
    break;
  }

  RaceMax = RacialMax[GET_RACE(ch)][class];
  
  for (i = 1; i <= 50; i++) {
    
    sprintf(buf, "[%2d] %9d : %s\n\r", i,
       titles[class][i].exp, (GET_SEX(ch)==SEX_FEMALE?titles[class][i].title_f:titles[class][i].title_m));
       send_to_char(buf, ch);
       }
       
       return;
}


void do_consider(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256], buf[256];
  int diff;
  
  only_argument(argument, name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Consider killing who?\n\r", ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("Easy! Very easy indeed!\n\r", ch);
    return;
  }
  
  if (!IS_NPC(victim)) {
    send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
    return;
  }
 
  act("$n looks at $N", FALSE, ch, 0, victim, TO_NOTVICT);
  act("$n looks at you", FALSE, ch, 0, victim, TO_VICT);
 
 
  diff =  GetMaxLevel(victim) - GetMaxLevel(ch);
  if (diff <= -10)
    send_to_char("Too easy to be believed.\n\r", ch);
  else if (diff <= -5)
    send_to_char("Not a problem.\n\r", ch);
  else if (diff <= -3)
    send_to_char("Rather easy.\n\r",ch);
  else if (diff <= -2)
    send_to_char("Easy.\n\r", ch);
  else if (diff <= -1)
    send_to_char("Fairly easy.\n\r", ch);
  else if (diff == 0)
    send_to_char("The perfect match!\n\r", ch);
  else if (diff <= 1)
    send_to_char("You would need some luck!\n\r", ch);
  else if (diff <= 2)
    send_to_char("You would need a lot of luck!\n\r", ch);
  else if (diff <= 3)
    send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
  else if (diff <= 5)
    send_to_char("Do you feel lucky, punk?\n\r", ch);
  else if (diff <= 10)
    send_to_char("Are you crazy?  Is that your problem?\n\r", ch);
  else if (diff <= 30)
    send_to_char("You ARE mad!\n\r", ch);
  else
    send_to_char("Why don't I just kill you right now and save you the trouble?\n\r", ch);
  
  if (ch->skills) {
    int skill=0;
    int learn=0;
    int num, num2;
    float fnum;
 
    if (IsAnimal(victim) && ch->skills[SKILL_CONS_ANIMAL].learned) {
      skill = SKILL_CONS_ANIMAL;
      learn = ch->skills[skill].learned;
      act("$N seems to be an animal", FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsVeggie(victim) && ch->skills[SKILL_CONS_VEGGIE].learned) {
      if (!skill)
   skill = SKILL_CONS_VEGGIE;
      learn = MAX(learn, ch->skills[SKILL_CONS_VEGGIE].learned);
      act("$N seems to be an ambulatory vegetable", 
     FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsDiabolic(victim) && ch->skills[SKILL_CONS_DEMON].learned) {
      if (!skill)
   skill = SKILL_CONS_DEMON;
      learn = MAX(learn, ch->skills[SKILL_CONS_DEMON].learned);
      act("$N seems to be a demon!", FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsReptile(victim) && ch->skills[SKILL_CONS_REPTILE].learned) {
      if (!skill)
   skill = SKILL_CONS_REPTILE;
      learn = MAX(learn, ch->skills[SKILL_CONS_REPTILE].learned);
      act("$N seems to be a reptilian creature", 
     FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsUndead(victim) && ch->skills[SKILL_CONS_UNDEAD].learned) {
      if (!skill)
   skill = SKILL_CONS_UNDEAD;
      learn = MAX(learn, ch->skills[SKILL_CONS_UNDEAD].learned);
      act("$N seems to be undead", FALSE, ch, 0, victim, TO_CHAR);
    }
 
    if (IsGiantish(victim)&& ch->skills[SKILL_CONS_GIANT].learned) {
      if (!skill)
   skill = SKILL_CONS_GIANT;
      learn = MAX(learn, ch->skills[SKILL_CONS_GIANT].learned);
      act("$N seems to be a giantish creature", FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsPerson(victim) && ch->skills[SKILL_CONS_PEOPLE].learned) {
      if (!skill)
   skill = SKILL_CONS_PEOPLE;
      learn = MAX(learn, ch->skills[SKILL_CONS_PEOPLE].learned);
      act("$N seems to be a human or demi-human", 
     FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsOther(victim)&& ch->skills[SKILL_CONS_OTHER].learned) {
      if (!skill)
   skill = SKILL_CONS_OTHER;
      learn = MAX(learn, ch->skills[SKILL_CONS_OTHER].learned/2);
      act("$N seems to be a monster you know about", 
     FALSE, ch, 0, victim, TO_CHAR);
    }
 
    if (learn > 95) learn = 95;
 
    if (learn == 0) return;
 
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
 
    num = GetApprox(GET_MAX_HIT(victim), learn);
    fnum = ((float)num/(float)GET_MAX_HIT(ch));
 
    sprintf(buf, "Est Max hits are: %s\n\r", DescRatio(fnum));
    send_to_char(buf, ch);
 
    num = GetApprox(GET_AC(victim), learn);
    fnum = ((float)num/(float)GET_AC(ch));
 
    sprintf(buf, "Est. armor class is : %s\n\r", DescRatio(fnum));
    send_to_char(buf, ch);
 
    if (learn > 60) {
      sprintf(buf, "Est. # of attacks: %s\n\r", 
         DescAttacks(GetApprox((int)victim->mult_att, 
              learn)));
      send_to_char(buf, ch);
    }
    if (learn > 70) {
 
      num =   GetApprox((int)victim->specials.damnodice, 
         learn);
      num2 =  GetApprox((int)victim->specials.damsizedice, 
         learn);
 
      fnum = (float)num*(num2/2.0);
      sprintf(buf, "Est. damage of attacks is %s\n\r", 
         DescDamage(fnum));
       
      send_to_char(buf, ch);
    }
 
    if (learn > 80) {
      
      num =   GetApprox(GET_HITROLL(victim), learn);
      num2 =  21 - CalcThaco(ch);
      if (num2 > 0)
   fnum = ((float)num/(float)num2);
      else
   fnum = 2.0;
 
      sprintf(buf, "Est. Thaco: %s\n\r", DescRatio(fnum));
       
      send_to_char(buf, ch);
 
      num =   GetApprox(GET_DAMROLL(victim), learn);
      num2 =  GET_DAMROLL(ch);
      fnum = (num/(float)num2);
 
      sprintf(buf, "Est. Dam bonus is: %s\n\r", DescRatio(fnum));
 
      send_to_char(buf, ch);
    }
  }
}

void do_spells(struct char_data *ch, char *argument, int cmd)
{
  int spl, i;
  char buf[16384];
  extern char *spells[];
  extern int spell_status[];
  extern struct spell_info_type spell_info[MAX_SKILLS];
  
  if (IS_NPC(ch))  {
    send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
    return;
  }
  
  *buf=0;

  for (i = 1, spl = 0; i <= MAX_EXIST_SPELL; i++, spl++) {
    if (GetMaxLevel(ch) > LOW_IMMORTAL || 
   spell_info[i].min_level_cleric < ABS_MAX_LVL)
      sprintf(buf + strlen(buf),
"[%2d] %-20s Mana:%3d, Cl:%2d, Mu:%2d, An:%2d, Pa:%2d, Ra:%2d\n\r",
         i, spells[spl], 
         spell_info[i].min_usesmana, 
         spell_info[i].min_level_cleric, 
         spell_info[i].min_level_magic,
     spell_info[i].min_level_anti,
     spell_info[i].min_level_pal,
     spell_info[i].min_level_ranger);
  }
  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}

void do_world(struct char_data *ch, char *argument, int cmd)
{
  static char buf[100];
  long ct, ot;
  char *tmstr, *otmstr;
  extern long Uptime;
  extern long room_count;
  extern long mob_count;
  extern int obj_count;

  ot = Uptime;
  otmstr = asctime(localtime(&ot));
  *(otmstr + strlen(otmstr) - 1) = '\0';
  sprintf(buf, "Start time was: %s (CST)\n\r", otmstr);
  send_to_char(buf, ch);
  
  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sprintf(buf, "Current time is: %s (CST)\n\r", tmstr);
  send_to_char(buf, ch);
#if HASH  
  sprintf(buf, "Total number of rooms in world: %d\n\r", room_db.klistlen);
#else
  sprintf(buf, "Total number of rooms in world: %d\n\r", room_count);
#endif
  send_to_char(buf, ch);
  sprintf(buf, "Total number of zones in world: %d\n\r\n\r",
     top_of_zone_table + 1);
  send_to_char(buf, ch);
  sprintf(buf,"Total number of distinct mobiles in world: %d\n\r",
     top_of_mobt + 1);
  send_to_char(buf, ch);
  sprintf(buf,"Total number of distinct objects in world: %d\n\r\n\r",
     top_of_objt + 1);
  send_to_char(buf, ch);
  sprintf(buf,"Total number of registered players: %d\n\r",top_of_p_table + 1);
  send_to_char(buf, ch);

  sprintf(buf, "Total number of monsters in game: %d\n\r", mob_count);
  send_to_char(buf, ch);

  sprintf(buf, "Total number of objects in game: %d\n\r", obj_count);
  send_to_char(buf, ch);

}

void do_attribute(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  extern char *attr_player_bits[];
  struct affected_type *aff;
  extern char *spells[];
  

  sprintf(buf,
  "You are %d years and %d months, %d cms, and you weigh %d lbs.\n\r",
   GET_AGE(ch), age(ch).month,
   ch->player.height,
   ch->player.weight);
  send_to_char(buf, ch);

if (GetMaxLevel(ch) > 10) {
  sprintf(buf, "You have %d lbs of equipment, and can carry upto "
     "%d lbs - %d spare lbs.\n\r",
     IS_CARRYING_W(ch), CAN_CARRY_W(ch), 
     CAN_CARRY_W(ch) - IS_CARRYING_W(ch));
  send_to_char(buf, ch); 

  sprintf(buf, "You have %d volume in inventory.\n\rYou can carry up to %d volume - You have %d spare volume capacity.\n\r",
     IS_CARRYING_N(ch), CAN_CARRY_N(ch),
     CAN_CARRY_N(ch) - IS_CARRYING_N(ch));
  send_to_char(buf, ch);
}


  sprintf(buf, 
"Your alignment is %d (-1000 to 1000), and armor is %s.\n\r",  
     ch->specials.alignment, ac_for_score(ch->points.armor));
  send_to_char(buf,ch);
  
  if (GetMaxLevel(ch) > 9) {
    sprintf(buf,"Your natural stats : %d/%d STR, %d INT, %d WIS, %d DEX and %d CON.\n\r", GET_RSTR(ch), GET_ADD(ch), GET_RINT(ch), GET_RWIS(ch), GET_RDEX(ch), GET_RCON(ch));
    send_to_char(buf,ch);
    sprintf(buf,"Your current stats : %d/%d STR, %d INT, %d WIS, %d DEX and %d CON.\n\r",
              GET_STR(ch), GET_ADD(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch), GET_CON(ch));
    send_to_char(buf,ch);
  }

  sprintf(buf, "Your To-Hit and To-Dam are %d and %d respectively.\n\r",
     GET_HITROLL(ch), GET_DAMROLL(ch));
  send_to_char(buf, ch);

  sprintf(buf, "Your five saving throws are: %d, %d, %d, %d, %d.\n\r",
     ch->specials.apply_saving_throw[0], ch->specials.apply_saving_throw[1],
     ch->specials.apply_saving_throw[2], ch->specials.apply_saving_throw[3],
     ch->specials.apply_saving_throw[4]);
  send_to_char(buf, ch);

  strcpy(buf,"Your social flags are: ");
  sprintbit(ch->specials.act,attr_player_bits,buf2);
  strcat(buf, buf2);
  strcat(buf, "\n\r");
  send_to_char(buf, ch);

  send_to_char("\n\rAffecting Spells:\n\r--------------\n\r", ch);
    for(aff = ch->affected; aff; aff = aff->next) {
      switch(aff->type) {
      case SKILL_SNEAK:
      case SPELL_POISON:
      case SPELL_CURSE:
   break;
      default:
   sprintf(buf, "Spell : '%s'\n\r",spells[aff->type-1]);
   send_to_char(buf, ch);
   break;
     }
  }
  
  if(GET_COND(ch, THIRST) == 0)
    send_to_char("You are totally parched.\n\r", ch);
  if((GET_COND(ch, THIRST) > 0) && (GET_COND(ch, THIRST) < 5))
    send_to_char("You throat is very dry.\n\r", ch);
  if((GET_COND(ch, THIRST) > 5) && (GET_COND(ch, THIRST) < 10))
    send_to_char("You could use a little drink.\n\r", ch);
  if((GET_COND(ch, THIRST) > 10) && (GET_COND(ch, THIRST) < 20))
    send_to_char("You are slightly thirsty.\n\r", ch);
  if(GET_COND(ch, THIRST) > 20)
    send_to_char("Your thirst is the least of your worries.\n\r", ch);
  if(GET_COND(ch, FULL) == 0)
    send_to_char("You are totally famished.\n\r", ch);
  if((GET_COND(ch, FULL) > 0) && (GET_COND(ch, FULL) < 5))
    send_to_char("You stomach is growling loudly.\n\r", ch);
  if((GET_COND(ch, FULL) > 5) && (GET_COND(ch, FULL) < 10))
    send_to_char("You could use a little bite to eat.\n\r", ch);
  if((GET_COND(ch, FULL) > 10) && (GET_COND(ch, FULL) < 20))
    send_to_char("You are slightly hungry.\n\r", ch);
  if(GET_COND(ch, FULL) > 20)
    send_to_char("Your hunger is the least of your worries.\n\r", ch);
  if(GET_COND(ch, DRUNK) > 10)
    send_to_char("You are drunk.\n\r", ch);
}



void do_scan(struct char_data *ch, char *argument, int cmd)
{
   send_to_char("Sorry, scan has again been temporarily disabled.\n\r", ch);
}
/*
{
   int door;
   int j;
   char buf[256];
   int n,s,e,w;
   struct char_data *i;
 
   n=0;
   s=0;
   e=0;
   w=0;
   send_to_char("You peer around you and see:\n\r",ch);
   act("$n peers around them to see who is around.",FALSE,ch,0,0,TO_ROOM);
   if (real_roomp(ch->in_room)->dir_option[0])
      if (real_roomp(ch->in_room)->dir_option[0]->to_room != NOWHERE &&
    !IS_SET(real_roomp(ch->in_room)->dir_option[0]->exit_info, EX_CLOSED) &&
    !IS_DARK(real_roomp(ch->in_room)->dir_option[0]->to_room))
      n=real_roomp(ch->in_room)->dir_option[0]->to_room;
      else n=0;
   if (real_roomp(ch->in_room)->dir_option[1])
      if (real_roomp(ch->in_room)->dir_option[1]->to_room != NOWHERE &&
    !IS_SET(real_roomp(ch->in_room)->dir_option[1]->exit_info, EX_CLOSED) &&
    !IS_DARK(real_roomp(ch->in_room)->dir_option[1]->to_room))
      e=real_roomp(ch->in_room)->dir_option[1]->to_room;
      else e=0;
   if (real_roomp(ch->in_room)->dir_option[2])   
      if (real_roomp(ch->in_room)->dir_option[2]->to_room != NOWHERE &&
    !IS_SET(real_roomp(ch->in_room)->dir_option[2]->exit_info, EX_CLOSED) &&
    !IS_DARK(real_roomp(ch->in_room)->dir_option[2]->to_room))
      s=real_roomp(ch->in_room)->dir_option[2]->to_room;
      else s=0;
   if (real_roomp(ch->in_room)->dir_option[3])   
      if (real_roomp(ch->in_room)->dir_option[3]->to_room != NOWHERE &&
    !IS_SET(real_roomp(ch->in_room)->dir_option[3]->exit_info, EX_CLOSED) &&
    !IS_DARK(real_roomp(ch->in_room)->dir_option[3]->to_room))
      w=real_roomp(ch->in_room)->dir_option[3]->to_room;
      else w=0;
   for (door=0; door<5; door++) {
      for (j=1; j<7; j++) {
   if (door==0 && n>0) {
       for (i=real_roomp(n)->people; i; i=i->next_in_room) {
       if (CAN_SEE(ch,i)) {
       sprintf(buf,"North, square %d: ",j);
       send_to_char(buf, ch);
       if (!IS_NPC(i)) sprintf(buf,"%s\n\r",GET_NAME(i));
       if (IS_NPC(i)) sprintf(buf,"%s\n\r",i->player.short_descr);
       send_to_char(buf,ch);
       }
       }
       if (real_roomp(n)->dir_option[0] &&
real_roomp(n)->dir_option[0]->to_room != NOWHERE &&
!IS_SET(real_roomp(n)->dir_option[0]->exit_info, EX_CLOSED) &&
!IS_DARK(real_roomp(n)->dir_option[0]->to_room))
       n=real_roomp(n)->dir_option[0]->to_room;
       else {
       n=0;
       j=1;
       door=1;
       }
    }
    if (door==1 && e>0) {
       for (i=real_roomp(e)->people; i; i=i->next_in_room) {
       if (CAN_SEE(ch,i)) {
       sprintf(buf,"East , square %d: ",j);
       send_to_char(buf,ch);
       if (!IS_NPC(i)) sprintf(buf,"%s\n\r",GET_NAME(i));
       if (IS_NPC(i)) sprintf(buf,"%s\n\r",i->player.short_descr);
       send_to_char(buf,ch);
       }
    }
       if (real_roomp(e)->dir_option[1] &&
real_roomp(e)->dir_option[1]->to_room != NOWHERE &&
!IS_SET(real_roomp(e)->dir_option[1]->exit_info, EX_CLOSED) &&
!IS_DARK(real_roomp(e)->dir_option[1]->to_room))
       e=real_roomp(e)->dir_option[1]->to_room;
       else {
       e=0;
       j=1;
       door=2;
       }
    }
    if (door==2 && s>0) {
       for (i=real_roomp(s)->people; i; i=i->next_in_room) {
       if (CAN_SEE(ch,i)) {
       sprintf(buf,"South, square %d: ",j);
       send_to_char(buf,ch);
       if (!IS_NPC(i)) sprintf(buf,"%s\n\r",GET_NAME(i));
       if (IS_NPC(i)) sprintf(buf,"%s\n\r",i->player.short_descr);
       send_to_char(buf,ch);
       }
    }
       if (real_roomp(s)->dir_option[2] &&
real_roomp(s)->dir_option[2]->to_room != NOWHERE &&
!IS_SET(real_roomp(s)->dir_option[2]->exit_info, EX_CLOSED) &&
!IS_DARK(real_roomp(s)->dir_option[2]->to_room))
       s=real_roomp(s)->dir_option[2]->to_room;
       else {
       s=0;
       j=1;
       door=3;
       }
    }
    if (door==3 && w>0) {
       for (i=real_roomp(w)->people; i; i=i->next_in_room) {
       if (CAN_SEE(ch,i)) {
       sprintf(buf,"West , square %d: ",j);
       send_to_char(buf,ch);
       if (!IS_NPC(i)) sprintf(buf,"%s\n\r",GET_NAME(ch));
       if (IS_NPC(i)) sprintf(buf,"%s\n\r",i->player.short_descr);
       send_to_char(buf,ch);
       }
    }
       if (real_roomp(w)->dir_option[3] &&
real_roomp(w)->dir_option[3]->to_room != NOWHERE &&
!IS_SET(real_roomp(w)->dir_option[3]->exit_info, EX_CLOSED) &&
!IS_DARK(real_roomp(w)->dir_option[3]->to_room))
       w=real_roomp(w)->dir_option[3]->to_room;
       else {
       w=0;
       j=7;
       door=4;
       }
    }
      }
   }
}
*/

char *DescRatio(float f)  /* theirs / yours */
{
  if (f > 1.0) {
    return("More than twice yours");
  } else if (f > .75) {
    return("More than half again greater than yours");
  } else if (f > .6) {
    return("At least a third greater than yours");
  } else if (f > .4) {
    return("About the same as yours");
  } else if (f > .3) {
    return("A little worse than yours");
  } else if (f > .1) {
    return("Much worse than yours");
  } else {
    return("Extremely inferior");
  }  
}
 
char *DescDamage(float dam)
{
  if (dam < 1.0) {
    return("Minimal Damage");
  } else if (dam <= 2.0) {
    return("Slight damage");
  } else if (dam <= 4.0) {
    return("A bit of damage");
  } else if (dam <= 10.0) {
    return("A decent amount of damage");
  } else if (dam <= 15.0) {
    return("A lot of damage");
  } else if (dam <= 25.0) {
    return("A whole lot of damage");
  } else if (dam <= 35.0) {
    return("A very large amount");
  } else {
    return("A TON of damage");
  }
}
 
char *DescAttacks(float a)
{
  if (a < 1.0) {
    return("Not many");
  } else if (a < 2.0) {
    return("About average");
  } else if (a < 3.0) {
    return("A few");
  } else if (a < 5.0) {
    return("A lot");
  } else if (a < 9.0) {
    return("Many");
  } else {
    return("A whole bunch");
  }
}

char *DescMoves(float a)
{
   if (a < .1)
     return("very tired");
   else if (a < .3)
     return("slightly tired");
   else if (a < .5)
     return("not very tired");
   else if (a < .7)
     return("well rested");
   else 
     return("totally rested");
}


char *ac_for_score(int a) 
{
   if (a > 75) {
     return("scantily clothed");
   } else if (a > 50) {   
     return("heavily clothed");
   } else if (a > 25) {
     return("slightly armored");
   } else if (a > 0) {
     return("moderately armored");
   } else if (a > -25) {
     return("armored rather heavily");
   } else if (a > -50) {
     return("armored very heavily");
   } else if (a > -100) {
     return("armored extremely heavily");
   } else {
     return("totally armored");
   }
}



void read_book(Mob *ch, Obj *o, char *arg) {
   char buf[256];
   int vnum, section = 0;
   
   if (!ch || !o || (GET_ITEM_TYPE(o) != ITEM_BOOK) || (o->item_number < 0))
      return;

   act("With curious eyes, you begin to read $p...", TRUE, ch, o, 0, TO_CHAR);
   act("$n begins reading $p...", TRUE, ch, o, 0, TO_ROOM);

   vnum = obj_index[o->item_number].virtual;   
   /* in next sscanf, buf eats 'words' like 'sect', 'section', 'chapter'... */
   sscanf(arg, "%s %d of ", buf, &section);
   if (section) {
      sprintf(buf, "books/%d.%d", vnum, section);
      start_page_file(ch->desc, buf, "...but can't find that section!\n\r");
   }
   else {
      sprintf(buf, "books/%d", vnum);
      if (!start_page_file(ch->desc, buf, "...oddly its blank!\n\r")) {
         sprintf(buf, "Object %d has no book file!", vnum);
         vlog(buf);
      }
   }
}

