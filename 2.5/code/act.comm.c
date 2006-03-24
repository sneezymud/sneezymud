/* ************************************************************************
*  file: act.comm.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Communication.                                                 *
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

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern char *ch_violate_msg1[], *ch_violate_msg2[];


void do_say(struct char_data *ch, char *argument, int cmd)
{
	int i;
	char buf[MAX_INPUT_LENGTH+40]="\0\0\0\0";
        struct affected_type *af;


	if (apply_soundproof(ch))
	  return;

	for (i = 0; *(argument + i) == ' '; i++);

               if (IS_AFFECTED(ch,AFF_SILENT)) {
           send_to_char("You can't make a sound!\n\r",ch);
           act("$n waves $s hands and points silently toward his $s mouth.",FALSE,ch,0,0,TO_ROOM);
           return;
        }


	if (!*(argument + i))
		send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
	else	{
		sprintf(buf,"$n says '%s'", argument + i);
		act(buf,FALSE,ch,0,0,TO_ROOM);
                sprintf(buf,"You say '%s'\n\r", argument + i);
                send_to_char(buf, ch);
	}
}



void do_shout(struct char_data *ch, char *argument, int cmd)
{
	char buf1[MAX_INPUT_LENGTH+40];
        struct descriptor_data *i;
	extern int Silence;


         if (IS_AFFECTED(ch,AFF_SILENT)) {
           send_to_char("You can't make a sound!\n\r",ch);
           act("$n waves $s hands and points silently toward his $s mouth.",FALSE,ch,0,0,TO_ROOM);
           return;
        }


      

	
	if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
		send_to_char("You can't shout!!\n\r", ch);
		return;
	}


	if (!IS_NPC(ch) && 
	    (Silence == 1) &&
            (!IS_IMMORTAL(ch))) { 
	  send_to_char("Shouting has been banned.\n\r", ch);
	  send_to_char("It will return when the lag is better.\n\r", ch);
	  return;
	}

	if (apply_soundproof(ch))
	  return;

	for (; *argument == ' '; argument++);

	if (ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
	  send_to_char("I don't think so :-)", ch->master);
	  return;
	}
	
	if (!(*argument))
	    send_to_char("Shout? Yes! Fine! Shout we must, but WHAT??\n\r", ch);
	else	{
                sprintf(buf1,"You shout '%s'\n\r", argument);
		send_to_char(buf1, ch);
		sprintf(buf1, "$n shouts '%s'", argument);

       	        for (i = descriptor_list; i; i = i->next)
      	        if (i->character != ch && !i->connected &&
		    (IS_NPC(i->character) ||
			(!IS_SET(i->character->specials.act, PLR_NOSHOUT))) &&
		        !check_soundproof(i->character))
				act(buf1, 0, ch, 0, i->character, TO_VICT);
	}
}






void do_grouptell(struct char_data *ch, char *argument, int cmd)
{
        static char buf1[MAX_INPUT_LENGTH];
        struct follow_type *f;
        struct char_data *k;
        bool found;


         if (IS_AFFECTED(ch,AFF_SILENT)) {
           send_to_char("You can't make a sound!\n\r",ch);
           act("$n waves $s hands and points silently toward his $s mouth.",FALSE,ch,0,0,TO_ROOM);
           return;
        }


        if(!(k=ch->master))
          k=ch;
        for (; *argument == ' '; argument++);

        if (!(*argument))
           send_to_char("Grouptell is a good command, but you need to tell your group SOMEthing!\n\r",ch);
        else {
           sprintf(buf1,"You tell your group: %s\n\r",argument);
           send_to_char(buf1,ch);

           sprintf(buf1,"$n: %s", argument);

         act(buf1,0,ch,0,k,TO_VICT);

         for(f=k->followers; f; f=f->next)
          if IS_AFFECTED(f->follower, AFF_GROUP) 
               act(buf1, 0, ch, 0, f->follower, TO_VICT);
  }
}
 
void do_commune(struct char_data *ch, char *argument, int cmd)
{
	static char buf1[MAX_INPUT_LENGTH];
        struct descriptor_data *i;


	for (; *argument == ' '; argument++);

	if (!(*argument))
       	   send_to_char("Communing among the gods is fine, but WHAT?\n\r",ch);
	else {
                sprintf(buf1,"You tell the gods: %s\n\r", argument);
		send_to_char(buf1, ch);

    	for (i = descriptor_list; i; i = i->next)
      	   if (i->character != ch && !i->connected && !IS_NPC(i->character) &&
                        (GetMaxLevel(i->character) >= 52)) {
                          if (IS_SET(i->character->specials.act, PLR_COLOR)) 
                           sprintf(buf1, "%s$n: %s%s%s", ANSI_VIOLET, ANSI_CYAN,
                                  argument, ANSI_NORMAL);
                          else
                            sprintf(buf1, "$n: %s", argument);
				act(buf1, 0, ch, 0, i->character, TO_VICT);
            }
      }
}

char *RandomWord()
{
  static char *string[50] = {
    "argle",
    "bargle",
    "glop",
    "glyph",
    "hussamah",  /* 5 */
    "rodina",
    "mustafah",
    "angina",
    "the",
    "fribble",  /* 10 */
    "fnort",
    "frobozz",
    "zarp",
    "ripple",
    "yrk",    /* 15 */
    "yid",
    "yerf",
    "oork",
    "grapple",
    "red",   /* 20 */
    "blue",
    "you",
    "me",
    "ftagn",
    "hastur",   /* 25 */
    "brob",
    "gnort",
    "lram",
    "truck",
    "kill",    /* 30 */
    "cthulhu",
    "huzzah",
    "acetacytacylic",
    "hydrooxypropyl",
    "summah",     /* 35 */
    "hummah",
    "cookies",
    "stan",
    "will",
    "wadapatang",   /* 40 */
    "pterodactyl",
    "frob",
    "yuma",
    "gumma",
    "lo-pan",   /* 45 */
    "sushi",
    "yaya",
    "yoyodine",
    "your",
    "mother"   /* 50 */
  };
 
  return(string[number(0,49)]);
 
}


void do_sign(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char buf[MAX_INPUT_LENGTH+40];
  char buf2[MAX_INPUT_LENGTH];
  char *p;
  int diff;
  struct char_data *t;
  struct room_data *rp;
    
  for (i = 0; *(argument + i) == ' '; i++);
  
  if (!*(argument + i))
    send_to_char("Yes, but WHAT do you want to sign?\n\r", ch);
  else  {
 
    rp = real_roomp(ch->in_room);
    if (!rp) return;
 
    if (!HasHands(ch)) {
      send_to_char("Yeah right... WHAT HANDS!!!!!!!!\n\r", ch);
      return;
    }
 
    strcpy(buf, argument+i);
    buf2[0] = '\0';
    /*
      work through the argument, word by word.  if you fail your
      skill roll, the word comes out garbled.
      */
    p = strtok(buf, " ");  /* first word */
 
    diff = strlen(buf);
 
    while (p) {
      if (ch->skills && number(1,75+strlen(p))<ch->skills[SKILL_SIGN].learned){
        strcat(buf2, p);
      } else {
        strcat(buf2, RandomWord());
      }
      strcat(buf2, " ");
      diff -= 1;
      p = strtok(0, " ");  /* next word */
    }
    /*
      if a recipient fails a roll, a word comes out garbled.
      */
 
    /*
      buf2 is now the "corrected" string.
      */
 
    sprintf(buf,"$n signs '%s'", buf2);
 
    for (t = rp->people;t;t=t->next_in_room) {
      if (t != ch) {
        if (t->skills && number(1,diff) < t->skills[SKILL_SIGN].learned) {
          act(buf, FALSE, ch, 0, t, TO_VICT);
        } else {
          act("$n makes funny motions with $s hands", 
              FALSE, ch, 0, t, TO_VICT);          
        }
      }
    }
 
      sprintf(buf,"You sign '%s'\n\r", argument + i);
      send_to_char(buf, ch);
  }
}

void do_send(struct char_data *ch, char *argument, int cmd)
{
        char buf1[MAX_INPUT_LENGTH+40];
        struct descriptor_data *i;
        struct obj_data *radio, *radio2; 
 
        if (!IS_NPC(ch) && (IS_SET(ch->specials.act, PLR_NOSHOUT) ||
             IS_AFFECTED(ch, AFF_SILENT))) {
                send_to_char("You can't send any messages at the moment!!\n\r", ch);
                return;
        }

        radio = ch->equipment[WEAR_RADIO];

        if (!radio) {
           send_to_char("You need to be holding a radio to send a message.\n\r", ch);
               return;
        }
        if (apply_soundproof(ch))
          return;
 
        for (; *argument == ' '; argument++);
 
        if (ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
          send_to_char("I don't think so :-)", ch->master);
          return;
        }
        
        if (!(*argument))
            send_to_char("What exactly did you want your message to be?\n\r", ch);
        else    {
                sprintf(buf1,"Your message is : %s\n\r", argument);
                send_to_char(buf1, ch);
                sprintf(buf1, "$n [Channel %d] : %s",
                               radio->obj_flags.value[3], argument);
 
       	        for (i = descriptor_list; i; i = i->next) {
                  if (i->character && (i->character != ch) &&
                     (i->connected == CON_PLYNG) &&
                     !check_soundproof(i->character) &&
                     (i->character->in_room != NOWHERE)) {

                        radio2 = i->character->equipment[WEAR_RADIO];

                   if ((radio2) &&
                   (radio->obj_flags.value[3] == radio2->obj_flags.value[3])) { 
                                act(buf1, 0, ch, 0, i->character, TO_VICT);
                        }
                 }
             }
        }
}


void do_tell(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH+20],
  buf[MAX_INPUT_LENGTH+20];

    if (IS_AFFECTED(ch,AFF_SILENT)) {
           send_to_char("You can't make a sound!\n\r",ch);
           act("$n waves $s hands and points silently toward his $s mouth.",FALSE,ch,0,0,TO_ROOM);
           return;
        }


	if (apply_soundproof(ch))
	  return;
  
  half_chop(argument,name,message);

  if(!*name || !*message) {
    send_to_char("Who do you wish to tell what??\n\r", ch);
    return;
  } else if (!(vict = get_char_vis(ch, name))) {
    send_to_char("No-one by that name here..\n\r", ch);
    return;
  } else if (ch == vict) {
    send_to_char("You try to tell yourself something.\n\r", ch);
    return;
  } else if (GET_POS(vict) == POSITION_SLEEPING)	{
    act("$E is asleep, shhh.",FALSE,ch,0,vict,TO_CHAR);
    return;
  } else if (IS_NPC(vict) && !(vict->desc)) {
    send_to_char("No-one by that name here..\n\r", ch);
    return;
  } else if (!vict->desc) {
    send_to_char("They can't hear you", ch);
    return;
  }

  if (check_soundproof(vict)) {
    send_to_char("Your words dont reach them, must be in a silent zone.\n\r", ch);
    return;
   }


if (IS_SET(vict->specials.act, PLR_COLOR)) {
  sprintf(buf,"%s%s%s tells you %s'%s'%s\n\r", ANSI_VIOLET,
   (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
   ANSI_NORMAL, ANSI_CYAN,  message, ANSI_NORMAL);
  send_to_char(buf, vict);
  } else if (IS_SET(vict->specials.act, PLR_VT100)) {
  sprintf(buf,"%s%s%s tells you '%s'\n\r", VT_BOLDTEX,
    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
   ANSI_NORMAL, message);
   send_to_char(buf, vict);
  } else {
  sprintf(buf,"%s tells you '%s'\n\r",
      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);
  send_to_char(buf, vict);
  }

     sprintf(buf,"You tell %s '%s'\n\r",
	  (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)), message);
     send_to_char(buf, ch);
  
}



void do_whisper(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH],
  buf[MAX_INPUT_LENGTH];

   if (IS_AFFECTED(ch,AFF_SILENT)) {
           send_to_char("You can't make a sound!\n\r",ch);
           act("$n waves $s hands and points silently toward his $s mouth.",FALSE,ch,0,0,TO_ROOM);
           return;
        }


	if (apply_soundproof(ch))
	  return;
  
  half_chop(argument,name,message);
  
  if(!*name || !*message)
    send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
  else if (!(vict = get_char_room_vis(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (vict == ch) {
    act("$n whispers quietly to $mself.",FALSE,ch,0,0,TO_ROOM);
    send_to_char
      ("You can't seem to get your mouth close enough to your ear...\n\r",ch);
  }  else    {
	if (check_soundproof(vict))
	  return;

      sprintf(buf,"$n whispers to you, '%s'",message);
      act(buf, FALSE, ch, 0, vict, TO_VICT);
        sprintf(buf,"You whisper to %s, '%s'\n\r",
	      (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), message);
        send_to_char(buf, ch);
      act("$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);
    }
}


void do_ask(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH],
  buf[MAX_INPUT_LENGTH];

   if (IS_AFFECTED(ch,AFF_SILENT)) {
           send_to_char("You can't make a sound!\n\r",ch);
           act("$n waves $s hands and points silently toward his $s mouth.",FALSE,ch,0,0,TO_ROOM);
           return;
        }


	if (apply_soundproof(ch))
	  return;
  
  half_chop(argument,name,message);
  
  if(!*name || !*message)
    send_to_char("Who do you want to ask something.. and what??\n\r", ch);
  else if (!(vict = get_char_room_vis(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (vict == ch)	{
    act("$n quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
    send_to_char("You think about it for a while...\n\r", ch);
  }  else	{
	if (check_soundproof(vict))
	  return;

    sprintf(buf,"$n asks you '%s'",message);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    
      sprintf(buf,"You ask %s, '%s'\n\r",
	    (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), message);
      send_to_char(buf, ch);
    act("$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT);
  }
}



#define MAX_NOTE_LENGTH 1000      /* arbitrary */

void do_write(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *paper = 0, *pen = 0;
  char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH],
       buf[MAX_STRING_LENGTH];
  
  argument_interpreter(argument, papername, penname);
  
  if (!ch->desc)
    return;
  
  if (!*papername)  /* nothing was delivered */    {   
      send_to_char("write (on) papername (with) penname.\n\r", ch);
      return;
    }

  if (!*penname) {
      send_to_char("write (on) papername (with) penname.\n\r", ch);
      return;
  }
  if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))	{
	  sprintf(buf, "You have no %s.\n\r", papername);
	  send_to_char(buf, ch);
	  return;
   }
   if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))	{
	  sprintf(buf, "You have no %s.\n\r", papername);
	  send_to_char(buf, ch);
	  return;
    }

  /* ok.. now let's see what kind of stuff we've found */
  if (pen->obj_flags.type_flag != ITEM_PEN) {
      act("$p is no good for writing with.",FALSE,ch,pen,0,TO_CHAR);
  } else if (paper->obj_flags.type_flag != ITEM_NOTE)    {
      act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  } else if (paper->action_description) {
    send_to_char("There's something written on it already.\n\r", ch);
    return;
  } else {
      /* we can write - hooray! */
      send_to_char
	("Ok.. go ahead and write.. end the note with a @.\n\r", ch);
      act("$n begins to jot down a note.", TRUE, ch, 0,0,TO_ROOM);
      ch->desc->str = &paper->action_description;
      ch->desc->max_str = MAX_NOTE_LENGTH;
    }
} 
