/* ************************************************************************
*  File: fight.c , Combat module.                         Part of DIKUMUD *
*  Usage: Combat system and messages.                                     *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "race.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"

/* Structures */

struct char_data *combat_list = 0;   /* head of l-list of fighting chars    */
struct char_data *missile_list = 0;   /* head of l-list of fighting chars    */
struct char_data *combat_next_dude = 0; /* Next dude global trick           */
struct char_data *missile_next_dude = 0; /* Next dude global trick           */


/* External structures */
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data  *object_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct char_data *character_list;
extern struct spell_info_type spell_info[];
extern char *ItemDamType[];
extern int ItemSaveThrows[22][5];
extern int corpse_volume[];
extern struct str_app_type str_app[];

/* External procedures */

char *fread_string(FILE *f1);
void stop_follower(struct char_data *ch);
void do_flee(struct char_data *ch, char *argument, int cmd);
void hit(struct char_data *ch, struct char_data *victim, int type);
void zero_rent(struct char_data *ch);
struct char_data *FindVictim( struct char_data *ch);
struct char_data *FindAHatee( struct char_data *ch);
struct char_data *FindAnAttacker(struct char_data *ch);
int SameRace( struct char_data *ch1, struct char_data *ch2);
int DamagedByAttack( struct obj_data *i, int dam_type);
int DamageItem( struct char_data *ch, struct obj_data *o, int num);
int GetFormType(struct char_data *ch);


/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit",    "hits"},            /* TYPE_HIT      */
  {"pound",  "pounds"},          /* TYPE_BLUDGEON */
  {"pierce", "pierces"},         /* TYPE_PIERCE   */
  {"slash",  "slashes"},         /* TYPE_SLASH    */
  {"whip",   "whips"},           /* TYPE_WHIP     */
  {"claw",   "claws"},           /* TYPE_CLAW     */
  {"bite",   "bites"},           /* TYPE_BITE     */
  {"sting",  "stings"},          /* TYPE_STING    */
  {"crush",  "crushes"},         /* TYPE_CRUSH    */
  {"cleave", "cleaves"},
  {"stab",   "stabs"},
  {"smash",  "smashes"},
  {"smite",  "smites"}
};




/* The Fight related routines */

/*
** Set the killer flag of attacker.  Note that this function
** checks for ONLY the case where the attacker is a pet
** ordered by someone else, or attacker is attacking another
** nonkiller, nonthief PC.
*/
void setKillerFlag(Mob *ch, Mob *victim) {
  Mob *master;
  char buf[MAX_STRING_LENGTH];
 
  /* You can never have enough checks for NULL. */
  if (!ch || !victim)  
     return;

  /* If victim is a mobile return */
  if (!IS_PC(victim))
     return;

  /* Check for polys victims who are originally killers */
  if ((IS_SET(victim->specials.act, ACT_POLYSELF)) && victim->desc &&
      ((IS_SET(victim->desc->original->specials.act, PLR_KILLER) ||
       IS_SET(victim->desc->original->specials.act, PLR_OUTLAW))))
     return;

  /* Check for poly attackers who are originally killers */
  if ((IS_SET(ch->specials.act, ACT_POLYSELF)) && ch->desc &&
      ((IS_SET(ch->desc->original->specials.act, PLR_KILLER) ||
       IS_SET(ch->desc->original->specials.act, PLR_OUTLAW))))
     return;

  /* Check for  non-poly killer victims  */
  if (IS_SET(victim->specials.act, PLR_KILLER) ||
      IS_SET(victim->specials.act, PLR_OUTLAW))
    return;

  /* Check for non-poly killer attackers  */
  if (IS_SET(ch->specials.act, PLR_KILLER) ||
      IS_SET(ch->specials.act, PLR_OUTLAW))
     return;
 
  if (!IS_PC(ch) && !(master = ch->master))
    return;
  
  if (!IS_PC(ch)) {
    sprintf(buf, "%s's follower attacking %s.",
            GET_NAME(master), GET_NAME(victim));
    vlog(buf);
 
    ch = master;
  }
  else {
    sprintf(buf, "%s is attacking %s.", (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), GET_NAME(victim));
    vlog(buf);
  }
 
  if (IS_SET(ch->specials.act, PLR_KILLER) ||
     (IS_NPC(ch) && ch->desc && ch->desc->original && 
      IS_SET(ch->desc->original->specials.act, PLR_KILLER)))
    return;
 
  send_to_char("Now you are a killer!!!!\n\r", ch);
  if (ch->desc && ch->desc->original)
    SET_BIT(ch->desc->original->specials.act, PLR_KILLER);
  else
    SET_BIT(ch->specials.act, PLR_KILLER);
}

void appear(struct char_data *ch)
{
  act("$n slowly fade into existence.", FALSE, ch,0,0,TO_ROOM);
  
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);
  
  REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}



void load_messages(void)
{
  FILE *f1;
  int i,type;
  struct message_type *messages;
  char chk[100];
  
  if (!(f1 = fopen(MESS_FILE, "r"))){
    perror("read messages");
    exit(0);
  }
  
  for (i = 0; i < MAX_MESSAGES; i++)	{ 
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks=0;
    fight_messages[i].msg = 0;
  }
  
  fscanf(f1, " %s \n", chk);
  
  i = 0;
  
  while(*chk == 'M')	{
    fscanf(f1," %d\n", &type);
    
    if(i>=MAX_MESSAGES){
      vlog("Too many combat messages.");
      exit(0);
    }
    
    CREATE(messages,struct message_type,1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type=type;
    messages->next=fight_messages[i].msg;
    fight_messages[i].msg=messages;
    
    messages->die_msg.attacker_msg      = fread_string(f1);
    messages->die_msg.victim_msg        = fread_string(f1);
    messages->die_msg.room_msg          = fread_string(f1);
    messages->miss_msg.attacker_msg     = fread_string(f1);
    messages->miss_msg.victim_msg       = fread_string(f1);
    messages->miss_msg.room_msg         = fread_string(f1);
    messages->hit_msg.attacker_msg      = fread_string(f1);
    messages->hit_msg.victim_msg        = fread_string(f1);
    messages->hit_msg.room_msg          = fread_string(f1);
    messages->god_msg.attacker_msg      = fread_string(f1);
    messages->god_msg.victim_msg        = fread_string(f1);
    messages->god_msg.room_msg          = fread_string(f1);
    fscanf(f1, " %s \n", chk);
    i++;
  }
  
  fclose(f1);
}


void update_pos( struct char_data *victim )
{
  
  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED)) {
    return;
  } else if (GET_HIT(victim) > 0 ) {
    if (GET_POS(victim) == POSITION_STUNNED) {
       GET_POS(victim) = POSITION_SITTING;
      }
    if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
      GET_POS(victim) = POSITION_STANDING;
    } else {
      GET_POS(victim) = POSITION_STUNNED;
    }
  } else if (GET_HIT(victim) <= -11) {
    GET_POS(victim) = POSITION_DEAD;
  } else if (GET_HIT(victim) <= -6) {
    GET_POS(victim) = POSITION_MORTALLYW;
  } else if (GET_HIT(victim) <= -3) {
    GET_POS(victim) = POSITION_INCAP;
  } else {
    GET_POS(victim) = POSITION_STUNNED;
  }
}


int check_peaceful(struct char_data *ch, char *msg)
{
  struct room_data *rp;
  
  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags&PEACEFUL) {
    send_to_char(msg, ch);
    return 1;
  }
  return 0;
}

int check_no_order(struct char_data *ch, char *msg)
{
   struct room_data *rp;

   rp = real_roomp(ch->in_room);
   if (rp && rp->room_flags&NO_ORDER) {
     send_to_char(msg,ch);
     return 1;
   }
   return 0;
}

/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
  
  if (ch->specials.fighting) {
    vlog("Fighting character set to fighting another.");
    return;
  }
  
  if (vict->attackers <= 5) {
    vict->attackers+=1;
  } else {
    vlog("more than 6 people attacking one target");
  }
  ch->next_fighting = combat_list;
  combat_list = ch;
  
  if(IS_AFFECTED(ch,AFF_SLEEP))
    affect_from_char(ch,SPELL_SLEEP);
  
  ch->specials.fighting = vict;
  GET_POS(ch) = POSITION_FIGHTING;
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
  struct char_data *tmp;
  
  if (!ch->specials.fighting) {
    vlog("Character not fighting at invocation of stop_fighting");
    return;
  }
  
  ch->specials.fighting->attackers-=1;
  if (ch->specials.fighting->attackers < 0) {
    vlog("too few people attacking");
    ch->specials.fighting->attackers = 0;
  }
  
  if (ch == combat_next_dude)
    combat_next_dude = ch->next_fighting;
  
  if (combat_list == ch)
    combat_list = ch->next_fighting;
  else	{
    for (tmp = combat_list; tmp && (tmp->next_fighting != ch); 
	 tmp = tmp->next_fighting);
    if (!tmp) {
      vlog("Char fighting not found Error (fight.c, stop_fighting)");
      abort();
    }
    tmp->next_fighting = ch->next_fighting;
  }
  
  ch->next_fighting = 0;
  ch->specials.fighting = 0;
  GET_POS(ch) = POSITION_STANDING;
  update_pos(ch);
}



#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 10

void make_corpse(struct char_data *ch)
{
  struct obj_data *corpse, *o;
  struct obj_data *money;	
  char buf[MAX_INPUT_LENGTH];
  int i, ADeadBody=FALSE;
  
  struct obj_data *create_money( int amount );
  
  CREATE(corpse, struct obj_data, 1);
  clear_object(corpse);
  
  corpse->item_number = NOWHERE;
  corpse->in_room = NOWHERE;
  
  if (!IS_NPC(ch) || (!IsUndead(ch))) {
    sprintf(buf, "corpse %s",ch->player.name);
    corpse->name = strdup(buf);

  if ((GET_COND(ch,THIRST) == 0) || (GET_COND(ch,FULL) == 0)) {
    sprintf(buf, "The thin, starved corpse of %s is lying here.",
           (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->description = strdup(buf);
  } else if (real_roomp(ch->in_room)->sector_type != SECT_UNDERWATER) {
    sprintf(buf, "The corpse of %s is lying here.", 
	    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->description = strdup(buf);
  } else {
    sprintf(buf, "The bloated, water-filled corpse of %s is floating here.",
            (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->description = strdup(buf);
   }

    sprintf(buf, "the corpse of %s",
            (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->short_description = strdup(buf);

    ADeadBody = TRUE;
    
  } else if (IsUndead(ch)) {
    corpse->name = strdup("dust pile");
    corpse->description = strdup("A pile of dust is here.");
    corpse->short_description = strdup("a pile of dust");	   
  } 
  
  corpse->contains = ch->carrying;
  if(GET_GOLD(ch)>0) {
    money = create_money(GET_GOLD(ch));
    GET_GOLD(ch)=0;
    obj_to_obj(money,corpse);
  }
  
  corpse->obj_flags.type_flag = ITEM_CONTAINER;
  corpse->obj_flags.wear_flags = ITEM_TAKE;
  corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
  corpse->obj_flags.volume   = corpse_volume[GET_RACE(ch)];
  if (ADeadBody) {
    corpse->obj_flags.weight = GET_WEIGHT(ch)+IS_CARRYING_W(ch);
  } else {
    corpse->obj_flags.weight = 1+IS_CARRYING_W(ch);
  }
  corpse->obj_flags.cost_per_day = 100000;
  if (IS_NPC(ch))
    corpse->obj_flags.decay_time = MAX_NPC_CORPSE_TIME;
  else
    corpse->obj_flags.decay_time = MAX_PC_CORPSE_TIME;
  
  for (i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i])
      obj_to_obj(unequip_char(ch, i), corpse);
  
  ch->carrying = 0;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;
  
  if (IS_NPC(ch)) {
    corpse->char_vnum = mob_index[ch->nr].virtual;
    corpse->char_f_pos = 0;
  } else {
    if (ch->desc) {
      corpse->char_f_pos = ch->desc->pos;
      corpse->char_vnum = 0;
    } else {
      corpse->char_f_pos = 0;
      corpse->char_vnum = 100;
    }
  }
  corpse->carried_by = 0;
  corpse->equipped_by = 0;
  
  corpse->next = object_list;
  object_list = corpse;
  
  for(o = corpse->contains; o; o = o->next_content)
    o->in_obj = corpse;
  
  object_list_new_owner(corpse, 0);
  
  obj_to_room(corpse, ch->in_room);
}


void change_alignment(struct char_data *ch, struct char_data *victim)
{
  
  if (IS_NPC(ch)) return;

  if (IS_GOOD(ch) && (IS_GOOD(victim))) {
    GET_ALIGNMENT(ch) -= (GET_ALIGNMENT(victim)  / 5); 
  } else if (IS_EVIL(ch) && (IS_GOOD(victim))) {
    GET_ALIGNMENT(ch) -= (GET_ALIGNMENT(victim) / 5);
  } else if (IS_EVIL(victim) && (IS_GOOD(ch))) {
    GET_ALIGNMENT(ch) -= (GET_ALIGNMENT(victim) / 5);
  } else if (IS_EVIL(ch) && (IS_EVIL(victim))) {
    GET_ALIGNMENT(ch) -= (GET_ALIGNMENT(victim) / 5);
  } else {
    GET_ALIGNMENT(ch) -= (GET_ALIGNMENT(victim) / 10);
  }
  GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch), -1000);
  GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch), 1000);
  
}

void death_cry(struct char_data *ch)
{
  int door, was_in;
  
  if (ch->in_room == -1)
    return;
  
  act("Your blood freezes as you hear $n's death cry.", FALSE, ch,0,0,TO_ROOM);
  was_in = ch->in_room;
  
  for (door = 0; door <= 5; door++) {
    if (CAN_GO(ch, door))	{
      ch->in_room = (real_roomp(was_in))->dir_option[door]->to_room;
      act("Your blood freezes as you hear someones death cry.",FALSE,ch,0,0,TO_ROOM);
      ch->in_room = was_in;
    }
  }
}



void raw_kill(struct char_data *ch)
{
  if (ch->specials.fighting)
    stop_fighting(ch);
  
  death_cry(ch);
  /*
    remove the problem with poison, and other spells
    */
  spell_dispel_magic(IMPLEMENTOR,ch,ch,0);

  if (IS_SET(ch->specials.act, PLR_KILLER))
       REMOVE_BIT(ch->specials.act, PLR_KILLER);
  if (IS_SET(ch->specials.act, PLR_OUTLAW))
       REMOVE_BIT(ch->specials.act, PLR_OUTLAW);

 if (GET_COND(ch,THIRST)>=0)
  GET_COND(ch,THIRST)=20;
 if (GET_COND(ch,FULL)>=0)
  GET_COND(ch,FULL)=20;

  
  /*
   *   return them from polymorph
   */
  
  make_corpse(ch);
  zero_rent(ch);
  extract_char(ch);
}



void die(struct char_data *ch)
{
  struct char_data *pers;
  
  if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
    /*
     *   take char from storage, to room     
     */
    if (ch->desc) {  /* hmmm */
       pers = ch->desc->original;
       char_from_room(pers);
       char_to_room(pers, ch->in_room);
       SwitchStuff(ch, pers);
       extract_char(ch);
       ch = pers;
     } else {
       /* we don't know who the original is.  Gets away with it, i guess*/ 
     }
  }
  
if (!IS_SET(real_roomp(ch->in_room)->room_flags, ARENA)) {
 if (GetMaxLevel(ch) > 30) {
     ch->points.max_hit -= number(1,3);
     gain_exp(ch, -2000000);
 } else if (GET_EXP(ch) > 4000000) {
  gain_exp(ch, -2000000);
 } else {
  gain_exp(ch, -GET_EXP(ch)/3);	
 }
  } else {
    gain_exp(ch, 0);
  }
  /*
   **      Set the talk[2] to be FALSE, i.e. DEAD
   */
  ch->player.talks[2] = FALSE;  /* char is dead */
  
  DeleteHatreds(ch);
  DeleteFears(ch);
  raw_kill(ch);
}



void group_gain(struct char_data *ch, struct char_data *victim)
{
  char buf[256];
  int no_members, share;
  struct char_data *k;
  struct follow_type *f;
  int total;
  int exp_shown;
  
  if (!(k=ch->master))
    k = ch;
  
  /* can't get exp for killing players */
  
  if (!IS_NPC(victim)) {
    return;
  }
  
  if (IS_AFFECTED(k, AFF_GROUP) &&
      (k->in_room == ch->in_room))
    no_members = GetMaxLevel(k);
  else
    no_members = 0;
  
  for (f=k->followers; f; f=f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	(f->follower->in_room == ch->in_room))
      no_members+=(GetMaxLevel(f->follower)/1);

   if (no_members < 1) 
      share = 0;

   if(!IS_NPC(ch)) {
    share =  ((GET_EXP(victim))/no_members);
   } else {
    share = ((GET_EXP(victim))/2/no_members);
   }
     
  share = MIN(share, 300000);
  
  if (IS_AFFECTED(k, AFF_GROUP) &&
      (k->in_room == ch->in_room)) {

      total = share*GetTotLevel(k);
      exp_shown = share*GetMaxLevel(k);

    sprintf(buf,"You receive your share of %d experience.", exp_shown);
    act(buf, FALSE, k, 0, 0, TO_CHAR);
    gain_exp(k,exp_shown);
    change_alignment(k, victim);
  }
  
  for (f=k->followers; f; f=f->next) {
    if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	(f->follower->in_room == ch->in_room)) {

        total = share*GetTotLevel(f->follower);
        exp_shown = share*GetMaxLevel(f->follower);

      sprintf(buf,"You receive your share of %d experience.", exp_shown);
      act(buf, FALSE, f->follower,0,0,TO_CHAR);
      gain_exp(f->follower,  exp_shown);

      change_alignment(f->follower, victim);
    }
  }
}

char *replace_string(char *str, char *weapon, char *weapon_s)
{
  static char buf[256];
  char *cp;
  
  cp = buf;
  
  for (; *str; str++) {
    if (*str == '#') {
      switch(*(++str)) {
      case 'W' : 
	for (; *weapon; *(cp++) = *(weapon++));
	break;
      case 'w' : 
	for (; *weapon_s; *(cp++) = *(weapon_s++));
	break;
	default :
	  *(cp++) = '#';
	break;
      }
    } else {
      *(cp++) = *str;
    }
    
    *cp = 0;
  } /* For */
  
  return(buf);
}



void dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type)
{
  struct obj_data *wield;
  struct char_data *tmp_victim, *temp;
  char *buf;
  int snum;
  int numb;
  
  static struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  } dam_weapons[] = {
    
    {"$n misses $N.",                           /*    0    */
       "You miss $N.",
       "$n misses you." },

    {"$n's #w whispers by $N's head.",
     "Your #w whispers by $N's head.",
     "$n's #w whispers by your head." },
    
    {     "$n bruises $N with $s #w.",                       /*  1.. 2  */
	    "You bruise $N as you #w $M.",
	    "$n bruises you as $e #W you." },

    {"$n tickles $N with $s #w.",
     "You tickles $N as you #w $M.",
     "$n tickles you as $e #W you." },
    
    {"$n barely #W $N.",                                   /*  3.. 4  */
       "You barely #w $N.",
       "$n barely #W you."},
    
    {"$n #W $N.",                                          /*  5.. 6  */
       "You #w $N.",
       "$n #W you."}, 
    
    {"$n #W $N hard.",                                     /*  7..10  */
       "You #w $N hard.",
       "$n #W you hard."},
    
    {"$n #W $N very hard.",                                /* 11..14  */
       "You #w $N very hard.",
       "$n #W you very hard."},
    
    {"$n #W $N extremely well.",                          /* 15..20  */
       "You #w $N extremely well.",
       "$n #W you extremely well."},

    {"$n's #w strikes $N extremely hard.",
     "Your #w strikes $N extremely hard.",
     "$n's #w strikes you extremely hard."},
    
    {"$n massacres $N with $s #w.",     /* > 20    */
       "You massacre $N with your #w.",
       "$n massacres you with $s #w."},
 
    {"$n ANNIHILATES $N with $s #w.",
     "You ANNIHILATE $N with your #w.",
     "$n ANNIHILATES you with $s #w."},

    {"$n OBLITERATES $N with $s #w.",
     "You OBLITERATE $N with your #w.",
     "$n OBLITERATES you with $s #w."},
 
   {"$n DISINTEGRATES $N with $s #w.",
    "You DISINTIGRATE $N with your #w.",
    "$n DISINTEGRATES you with $s #w."}
  };

  w_type -= TYPE_HIT;   /* Change to base of table with text */
  
  wield = ch->equipment[WIELD];

  numb = number(1,2); 

  if ((dam == 0) && (numb==1)) {
    snum = 0;
  } else if (dam==0) {
    snum = 1;
  } else if ((dam <= 2) && (numb==1)) {
    snum = 2;
  } else if (dam <= 2) {
    snum = 3;
  } else if (dam <= 4) {
    snum = 4;
  } else if (dam <= 10) {
    snum = 5;
  } else if (dam <= 15) {
    snum = 6;
  } else if (dam <= 20) {
    snum = 7;
  } else if ((dam <= 30) && (numb==1)) {
    snum = 8;
  } else if (dam <= 30) {
    snum = 9;
  } else if (dam <= 40) {
    snum = 10;
  } else if (dam <= 60) {
    snum = 11;
  } else if (dam <= 80) {
    snum = 12;
  } else {
    snum = 13;
  }
  
  buf = replace_string(dam_weapons[snum].to_room, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular);
  act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
  buf = replace_string(dam_weapons[snum].to_char, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular);
  act(buf, FALSE, ch, wield, victim, TO_CHAR);
  buf = replace_string(dam_weapons[snum].to_victim, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular);
  act(buf, FALSE, ch, wield, victim, TO_VICT);
  
}

#if 1
int DamCheckDeny(struct char_data *ch, struct char_data *victim, int type)
{
  struct room_data *rp;
  char buf[MAX_INPUT_LENGTH];

  rp = real_roomp(ch->in_room);
  if (rp && (rp->room_flags&PEACEFUL) && type!=SPELL_POISON) {
    sprintf(buf, "damage(,,,%d) called in PEACEFUL room", type);
    vlog(buf);
    return(TRUE); /* true, they are denied from fighting */
  }
  return(FALSE);

}

int DamDetailsOk( struct char_data *ch, struct char_data *v, int dam, int type)
{

  if (dam < 0) return(FALSE);
  if (ch->in_room != v->in_room) return(FALSE);
  if (ch == v) return(FALSE);

  return(TRUE);

}


int SetCharFighting(struct char_data *ch, struct char_data *v)
{
  if (GET_POS(ch) > POSITION_STUNNED) {	
    if (!(ch->specials.fighting)) {
       set_fighting(ch, v);
       GET_POS(ch) = POSITION_FIGHTING;
    } else {
       return(FALSE);
    }
  }
  return(TRUE);

}


int SetVictFighting(struct char_data *ch, struct char_data *v)
{

  if ((v != ch) && (GET_POS(v) > POSITION_STUNNED) && 
     (!(v->specials.fighting))) {
     if (ch->attackers < 6) {
        set_fighting(v, ch);
        GET_POS(v) = POSITION_FIGHTING;
      }
  } else {
      return(FALSE);	
  }
  return(TRUE);
}

int DamageTrivia(struct char_data *ch, struct char_data *v, int dam, int type)
{
  if (v->master == ch)
    stop_follower(v);
  
  if (IS_AFFECTED(ch, AFF_INVISIBLE))
    appear(ch);

  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    affect_from_char(ch, SKILL_SNEAK);
  }
  
  if (IS_AFFECTED(v, AFF_SANCTUARY))
    dam = MAX((int)(dam/2), 0);  /* Max 1/2 damage when sanct'd */

  if (IS_AFFECTED(v, AFF_PROTECT_EVIL) && (IS_EVIL(ch))) 
    dam = MAX((int)(dam - 1), 0);
  
  dam = PreProcDam(v,type,dam);
  
 if (dam > -1) {
  dam = WeaponCheck(ch, v, type, dam);

  dam=MAX(dam,0);
  }
  return(dam);
}

int DoDamage(struct char_data *ch, struct char_data *v, int dam, int type)
{

  GET_HIT(v)-=dam;  

  if (IS_AFFECTED(v, AFF_FIRESHIELD)&&
     !IS_AFFECTED(ch, AFF_FIRESHIELD)) {
     if (damage(v, ch, dam, SPELL_FIREBALL)) {
        if (GET_POS(ch) == POSITION_DEAD)
           return(TRUE);
     }
  }
  update_pos(v);
  return(FALSE);

}


int DamageMessages( struct char_data *ch, struct char_data *v, int dam,
		    int attacktype)
{
  int nr, max_hit, i, j, exp;
  struct message_type *messages;
  char buf[MAX_INPUT_LENGTH]; 


  if ((attacktype >= TYPE_HIT) && (attacktype <= TYPE_SMITE)) {
      dam_message(dam, ch, v, attacktype);
    if (ch->equipment[WIELD]) {
      BrittleCheck(ch, dam);
    }
  } else {
    for(i = 0; i < MAX_MESSAGES; i++) {
      if (fight_messages[i].a_type == attacktype) {
	nr=dice(1,fight_messages[i].number_of_attacks);

	for(j=1,messages=fight_messages[i].msg;(j<nr)&&(messages);j++)
	  messages=messages->next;
	
	if (!IS_NPC(v) && (GetMaxLevel(v) > MAX_MORT)){
	  act(messages->god_msg.attacker_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	  act(messages->god_msg.victim_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	  act(messages->god_msg.room_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	} else if (dam != 0) {
	  if (GET_POS(v) == POSITION_DEAD) {
	    act(messages->die_msg.attacker_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	    act(messages->die_msg.victim_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	    act(messages->die_msg.room_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	  } else {
	    act(messages->hit_msg.attacker_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	    act(messages->hit_msg.victim_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	    act(messages->hit_msg.room_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	  }
	} else { /* Dam == 0 */
	  act(messages->miss_msg.attacker_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	  act(messages->miss_msg.victim_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	  act(messages->miss_msg.room_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	}
      }
    }
  }
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
    act("$n is stunned, but will probably regain consciousness again.", 
	TRUE, v, 0, 0, TO_ROOM);
    act("You're stunned, but you will probably regain consciousness again.", 
	FALSE, v, 0, 0, TO_CHAR);
    break;
  case POSITION_DEAD:
    act("$n is dead! R.I.P.", TRUE, v, 0, 0, TO_ROOM);
    act("You are dead!  Sorry...", FALSE, v, 0, 0, TO_CHAR);
    break;
    
  default:  /* >= POSITION SLEEPING */
    
    max_hit = hit_limit(v);

    if (dam > (max_hit/5))
      act("That really did HURT!",FALSE, v, 0, 0, TO_CHAR);
    
    if (GET_HIT(v) < (hit_limit(v)/6)) {
 if (IS_SET(v->specials.act, PLR_COLOR)) {
  if (dam > 0) {
   sprintf(buf,"You wish that your wounds would stop %sBLEEDING%s so much!!\n\r",
               ANSI_RED,
               ANSI_NORMAL );
   send_to_char(buf, v); 
  } 
 } else if (IS_SET(v->specials.act, PLR_VT100)) {
    if (dam > 0) {
      sprintf(buf,"You wish your wounds would stop %sBLEEDING%s so much!!\n\r",
         VT_BOLDTEX, ANSI_NORMAL );
        send_to_char(buf,v);
   } 
 } else {
    if (dam > 0) 
      act("You wish that your wounds would stop BLEEDING that much!", 
      FALSE, v, 0, 0, TO_CHAR);
    }
    if (IS_NPC(v) && (IS_SET(v->specials.act, ACT_WIMPY))) {
                strcpy(buf, "flee");
                command_interpreter(v, buf);
    } else if (!IS_NPC(v)) {
      if (IS_SET(v->specials.act, PLR_WIMPY)) 
        strcpy(buf, "flee");
        command_interpreter(v, buf);
    }
    break;              
  }
 }
}

int DamageEpilog(struct char_data *ch, struct char_data *victim)
{
  int exp;
  char buf[256];

  if (IS_PC(victim) && 
     !(victim->desc)) {
     do_flee(victim, "", 0);
     act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
     victim->specials.was_in_room = victim->in_room;
     if (victim->in_room != NOWHERE)
        char_from_room(victim);
     char_to_room(victim, 4);
     return(FALSE);
     if (GET_POS(victim) != POSITION_DEAD)
       return(FALSE);
  }
  
  if (!AWAKE(victim))
    if (victim->specials.fighting)
      stop_fighting(victim);
  
  if (GET_POS(victim) == POSITION_DEAD) {
    if (ch->specials.fighting == victim)
      stop_fighting(ch);
    if (IS_NPC(victim) || victim->desc)
      if (IS_AFFECTED(ch, AFF_GROUP)) {
	group_gain(ch, victim);
      } else {
	/* Calculate level-difference bonus */
	exp = GET_EXP(victim);
	
	exp = MAX(exp, 1);
	exp = MIN(exp, 100000);
/*
	exp = LevelMod(ch, victim);
*/
	if (IS_NPC(victim) && (!IS_SET(victim->specials.act, ACT_POLYSELF))) {
	  gain_exp(ch, exp);
	}
	change_alignment(ch, victim);
      }
    if (!IS_NPC(victim)) {
      if (victim->in_room > -1) {
	if (IS_NPC(ch)&&!IS_SET(ch->specials.act, ACT_POLYSELF)) {
	   sprintf(buf, "%s killed by %s at %s",
		GET_NAME(victim), ch->player.short_descr,
		(real_roomp(victim->in_room))->name);

	} else {
	   sprintf(buf, "%s killed by %s at %s -- <Player kill>",
		GET_NAME(victim), ch->player.name,
		(real_roomp(victim->in_room))->name);
	}
      } else {
	sprintf(buf, "%s killed by %s at Nowhere.",
		GET_NAME(victim),
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
      }
      vlog(buf);
    }
    die(victim);
    /*
     *  if the victim is dead, return TRUE.
     */
    victim = 0;
    return(TRUE);
  } else {
    return(FALSE);
  }


}


int MissileDamage(struct char_data *ch, struct char_data *victim,
	          int dam, int attacktype)
{
   if (DamCheckDeny(ch, victim, attacktype))
     return(FALSE);

   dam = SkipImmortals(victim, dam);

   if (!DamDetailsOk(ch, victim, dam, attacktype))
     return(FALSE);

   SetVictFighting(ch, victim);

   dam = DamageTrivia(ch, victim, dam, attacktype);

   if (DoDamage(ch, victim, dam, attacktype))
     return(TRUE);

   DamageMessages(ch, victim, dam, attacktype);

   if (DamageEpilog(ch, victim)) return(TRUE);

   return FALSE;
}

int GunMissileDamage(struct char_data *ch, struct char_data *victim,
	          int olddam, int attacktype)
{
   int dam;
   struct obj_data *gun;

   if (!DamDetailsOk(ch, victim, dam, attacktype))
     return(FALSE);

   gun = ch->equipment[HOLD];
   dam = GET_DAMROLL (ch);
   if ( gun->obj_flags.value[2] > 0 )
   {
	dam += dice ( gun->obj_flags.value[1], gun->obj_flags.value[2]);
   } else {
	act ( "$p jams and refuses to fire.", TRUE, ch, gun, 0, TO_CHAR );
	act("$p jams on $n", TRUE, ch, gun, 0, TO_ROOM );
	return (FALSE);
   }

   if ( GET_POS(victim) < POSITION_FIGHTING)
	dam *= 1+(POSITION_FIGHTING-GET_POS(victim))/3;

   dam = MAX(0, dam);

   dam = SkipImmortals(victim, dam);

   SetVictFighting(ch, victim);
/*
   if (!SetCharFighting(ch, victim)) return(FALSE);
*/

   dam = DamageTrivia(ch, victim, dam, attacktype);

   if (DoDamage(ch, victim, dam, attacktype))
     return(TRUE);

   DamageMessages(ch, victim, dam, SPEC_SHOOT);

   if (DamageEpilog(ch, victim)) return(TRUE);

   return(FALSE);  /* not dead */

}

int BowMissileDamage(struct char_data *ch, struct char_data *victim,
                 int olddam, int attacktype)
{
    int dam;
    struct obj_data *bow;

    if (!DamDetailsOk(ch, victim, dam, attacktype))
        return(FALSE);

     bow = ch->equipment[HOLD];
     dam = GET_DAMROLL(ch);
     dam += dice( bow->obj_flags.value[1], bow->obj_flags.value[2]);
 

      dam = MAX(0, dam);

      dam = SkipImmortals(victim, dam);

      SetVictFighting(ch, victim);

      dam = DamageTrivia(ch, victim, dam, attacktype);

      if (DoDamage(ch, victim, dam, attacktype))
         return(TRUE);

       DamageMessages(ch, victim, dam, SPEC_BOW);
 
       if (DamageEpilog(ch, victim))  return(TRUE);

       return(FALSE);
}


int damage(struct char_data *ch, struct char_data *victim,
	          int dam, int attacktype)
{

   if (DamCheckDeny(ch, victim, attacktype))
     return(FALSE);

   dam = SkipImmortals(victim, dam);

   if (!DamDetailsOk(ch, victim, dam, attacktype))
     return(FALSE);

   SetVictFighting(ch, victim);
   SetCharFighting(ch, victim);

   dam = DamageTrivia(ch, victim, dam, attacktype);

   if (DoDamage(ch, victim, dam, attacktype))
     return(TRUE);

   DamageMessages(ch, victim, dam, attacktype);

   if (DamageEpilog(ch, victim)) return(TRUE);

   return(FALSE);  /* not dead */
}

#else    /* this is for an example of the old code, slightly modified &/

int damage(struct char_data *ch, struct char_data *victim,
	   int dam, int attacktype)
{
  char buf[MAX_INPUT_LENGTH];
  struct message_type *messages;
  int i,j,nr,max_hit,exp;
  struct room_data	*rp;
  char buf[MAX_INPUT_LENGTH];
 
  int hit_limit(struct char_data *ch);
  
  assert(GET_POS(victim) > POSITION_DEAD);
  
  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags&PEACEFUL &&
      attacktype!=SPELL_POISON /* poison is allowed */
      ) {
    char	buf[MAX_INPUT_LENGTH];
    sprintf(buf, "damage(,,,%d) called in PEACEFUL room", attacktype);
    vlog(buf);
    return;
  }
  
  dam = SkipImmortals(victim, dam);
  if (dam == -1)
    return(FALSE);
  
  if (ch->in_room != victim->in_room)
    return(FALSE);
  
  if (victim != ch) {
    if (GET_POS(victim) > POSITION_STUNNED) {
      if (!(victim->specials.fighting))
	if ((!IS_NPC(ch))||(!IS_SET(ch->specials.act, ACT_IMMORTAL))) {
	  if (ch->attackers < 6) {
	    set_fighting(victim, ch);
	    GET_POS(victim) = POSITION_FIGHTING;
	  }
	} else {
	  return(FALSE);
	}
    }
  }
  
  if (victim != ch) {
    if (GET_POS(ch) > POSITION_STUNNED) {	
      if (!(ch->specials.fighting))
	if ((!IS_NPC(ch))||(!IS_SET(ch->specials.act, ACT_IMMORTAL))) {
	  set_fighting(ch, victim);
	  GET_POS(ch) = POSITION_FIGHTING;
	} else {
	  return(FALSE);
	}
    }
  }
  
#if 0  
  /*
    Not realistic, or fair, characters can't switch, why should monsters?
    */
  if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
      !number(0,6) && IS_AFFECTED(victim, AFF_CHARM) &&
      (victim->master->in_room == ch->in_room) &&
      !IS_IMMORTAL(victim->master)) {
    /* an NPC will occasionally switch to attack the master of
       its victim if the victim is an NPC and charmed by a mortal */
    if (ch->specials.fighting)
      stop_fighting(ch);
    hit(ch, victim->master, TYPE_UNDEFINED);
    return(FALSE);
  }
#endif 
  
  
  if (victim->master == ch)
    stop_follower(victim);
  
  if (IS_AFFECTED(ch, AFF_INVISIBLE))
    appear(ch);

  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    affect_from_char(ch, SKILL_SNEAK);
  }
  
  if (IS_AFFECTED(victim, AFF_SANCTUARY))
    dam = MAX((int)(dam/2), 0);  /* Max 1/2 damage when sanct'd */
  
  dam = PreProcDam(victim, attacktype,dam);
  
  dam = WeaponCheck(ch, victim, attacktype, dam);

  dam=MAX(dam,0);
  
  
  GET_HIT(victim)-=dam;  

  if (IS_AFFECTED(victim, AFF_FIRESHIELD)&&!IS_AFFECTED(ch, AFF_FIRESHIELD)) {
    if (damage(victim, ch, dam, SPELL_FIREBALL)) {
      update_pos(victim);
      if (GET_POS(victim) != POSITION_DEAD)
        return(FALSE);
      else
        return(TRUE);
    }
  }
  update_pos(victim);
  
  if ((attacktype >= TYPE_HIT) && (attacktype <= TYPE_SMITE)) {
    if (!ch->equipment[WIELD]) {
      dam_message(dam, ch, victim, TYPE_HIT);
    } else {
      dam_message(dam, ch, victim, attacktype);
  /*
   *  check if attacker's weapon is brittle
   */
      BrittleCheck(ch, dam);
  
    }
  } else {
    
    for(i = 0; i < MAX_MESSAGES; i++) {
      if (fight_messages[i].a_type == attacktype) {
	nr=dice(1,fight_messages[i].number_of_attacks);
	for(j=1,messages=fight_messages[i].msg;(j<nr)&&(messages);j++)
	  messages=messages->next;
	
	if (!IS_NPC(victim) && (GetMaxLevel(victim) > MAX_MORT)){
	  act(messages->god_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
	  act(messages->god_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
	  act(messages->god_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	} else if (dam != 0) {
	  if (GET_POS(victim) == POSITION_DEAD) {
	    act(messages->die_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
	    act(messages->die_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
	    act(messages->die_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	  } else {
	    act(messages->hit_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
	    act(messages->hit_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
	    act(messages->hit_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	  }
	} else { /* Dam == 0 */
	  act(messages->miss_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
	  act(messages->miss_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
	  act(messages->miss_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	}
      }
    }
  }
  switch (GET_POS(victim)) {
  case POSITION_MORTALLYW:
    act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    act("You are mortally wounded, and will die soon, if not aided.", FALSE, victim, 0, 0, TO_CHAR);
    break;
  case POSITION_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    act("You are incapacitated and you will slowly die, if not aided.", FALSE, victim, 0, 0, TO_CHAR);
    break;
  case POSITION_STUNNED:
    act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
    act("You're stunned, but you will probably regain consciousness again.", FALSE, victim, 0, 0, TO_CHAR);
    break;
  case POSITION_DEAD:
    act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
    act("You are dead!  Sorry...", FALSE, victim, 0, 0, TO_CHAR);
    break;
    
  default:  /* >= POSITION SLEEPING */
    
    max_hit=hit_limit(victim);
    
    if (dam > (max_hit/5))
      act("That Really HURT!",FALSE, victim, 0, 0, TO_CHAR);
    
    if ((GET_HIT(victim) < (max_hit/5)) && (GET_HIT(victim) > 0)) {
    
   if (IS_SET(victim->specials.act, PLR_COLOR)) { 
    sprintf(buf, "You wish you wounds would stop %sBLEEDING%s so much!!\n\r",
              ANSI_RED,
              ANSI_NORMAL );  
   send_to_char(buf, victim);
   } else {
      act("You wish that your wounds would stop BLEEDING so much!",FALSE,victim,0,0,TO_CHAR);
   }
      if (IS_NPC(victim)) {
	if (IS_SET(victim->specials.act, ACT_WIMPY))
        do_flee(victim, "", 0);
      } else {
    if (IS_SET(victim->specials.act, PLR_WIMPY)) {
      do_flee(victim, "", 0);
      }
    }
    break;
  }
  
  if (IS_PC(victim) && !(victim->desc)) {
     do_flee(victim, "", 0);
     act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
     victim->specials.was_in_room = victim->in_room;
     if (victim->in_room != NOWHERE)
        char_from_room(victim);
     char_to_room(victim, 4);
     return(FALSE);
  }
   
  if (GET_POS(victim) == POSITION_DEAD) {
    if (ch->specials.fighting == victim)
      stop_fighting(ch);
  }
  
  if (!AWAKE(victim))
    if (victim->specials.fighting)
      stop_fighting(victim);
  
  if (GET_POS(victim) == POSITION_DEAD) {
    if (IS_NPC(victim) || victim->desc)
      if (IS_AFFECTED(ch, AFF_GROUP)) {
	group_gain(ch, victim);
      } else {
	/* Calculate level-difference bonus */
	exp = GET_EXP(victim);
	
	exp = MAX(exp, 1);
	exp = MIN(exp, 100000);
/*
	exp = LevelMod(ch, victim);
*/
	if (IS_NPC(victim) && (!IS_SET(victim->specials.act, ACT_POLYSELF))) {
	  gain_exp(ch, exp);
	}
	change_alignment(ch, victim);
      }
    if (!IS_NPC(victim)) {
      if (victim->in_room > -1) {
	sprintf(buf, "%s killed by %s at %s",
		GET_NAME(victim),
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
		(real_roomp(victim->in_room))->name);
      } else {
	sprintf(buf, "%s killed by %s at Nowhere.",
		GET_NAME(victim),
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
      }
      vlog(buf);
    }
    die(victim);
    /*
     *  if the victim is dead, return TRUE.
     */
    victim = 0;
    return(TRUE);
  } else {
    return(FALSE);
  }
}

#endif

int GetWeaponType(struct char_data *ch, struct obj_data **wielded) 
{
  int w_type;

  if (ch->equipment[WIELD] &&
      (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)) {

    *wielded = ch->equipment[WIELD];
    w_type = Getw_type(*wielded);

  }	else {
    if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
      w_type = ch->specials.attack_type;
    else
      w_type = TYPE_HIT;

    *wielded = 0;  /* no weapon */

  }
  return(w_type);

}

int Getw_type(struct obj_data *wielded) 
{
  int w_type;

  switch (wielded->obj_flags.value[3]) {
    case 0  : w_type = TYPE_SMITE; break;
    case 1  : w_type = TYPE_STAB;  break;
    case 2  : w_type = TYPE_WHIP; break;
    case 3  : w_type = TYPE_SLASH; break;
    case 4  : w_type = TYPE_SMASH; break;
    case 5  : w_type = TYPE_CLEAVE; break;
    case 6  : w_type = TYPE_CRUSH; break;
    case 7  : w_type = TYPE_BLUDGEON; break;
    case 8  : w_type = TYPE_CLAW; break;
    case 9  : w_type = TYPE_BITE; break;
    case 10 : w_type = TYPE_STING; break;
    case 11 : w_type = TYPE_PIERCE; break;
      
    default : w_type = TYPE_HIT; break;
  }
  return(w_type);
}

int HitCheckDeny(struct char_data *ch, struct char_data *victim, int type)
{
  struct room_data *rp;
  char buf[256];

  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags&PEACEFUL) {
    sprintf(buf, "hit() called in PEACEFUL room");
    vlog(buf);
    stop_fighting(ch);
    return(TRUE);
  }
  
  if (ch->in_room != victim->in_room) {
    sprintf(buf, "NOT in same room when fighting : %s, %s", ch->player.name, victim->player.name);
    vlog(buf);
    stop_fighting(ch);
    return(TRUE);
  }

  if (GET_MOVE(ch) < -10) {
    send_to_char("You're too exhausted to fight\n\r",ch);
    stop_fighting(ch);
    return(TRUE);
  }

  
  if (victim->attackers >= 6 && ch->specials.fighting != victim) {
    send_to_char("You can't attack them,  no room!\n\r", ch);
    return(TRUE);
  }

/*
   if the character is already fighting several opponents, and he wants
   to hit someone who is not currently attacking him, then deny them.
   if he is already attacking that person, he can continue, even if they
   stop fighting him.
*/  
  if ((ch->attackers >= 6) && (victim->specials.fighting != ch) &&
      ch->specials.fighting != victim) {
    send_to_char("There are too many other people in the way.\n\r", ch);
    return(TRUE);
  }

  if (!IS_PC(ch)) {
    if (ch->specials.fighting && IS_PC(ch->specials.fighting) &&
       !ch->specials.fighting->desc) {
      do_flee(ch, "\0",0);
      return(TRUE);
    }
  }

  if (victim == ch) {
    if (Hates(ch,victim)) {
      RemHatred(ch, victim);
    }
    return(TRUE);
  }

  if (GET_POS(victim) == POSITION_DEAD)
    return(TRUE);

  return(FALSE);

}

int CalcThaco(struct char_data *ch)
{  
  int calc_thaco;
  extern struct str_app_type str_app[];
  extern int thaco[8][ABS_MAX_LVL];

  
  /* Calculate the raw armor including magic armor */
  /* The lower AC, the better                      */
  
  if (!IS_NPC(ch))
    calc_thaco = thaco[BestFightingClass(ch)][GET_LEVEL(ch, BestFightingClass(ch))];
  else
    /* THAC0 for monsters is set in the HitRoll */
    calc_thaco = 20;
  
  calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
  calc_thaco -= GET_HITROLL(ch);
  return(calc_thaco);
}

int HitOrMiss(struct char_data *ch, struct char_data *victim, int calc_thaco) 
{
  int diceroll, victim_ac;

  extern struct dex_app_type dex_app[];

  diceroll = number(1,20);
  
  victim_ac  = GET_AC(victim)/10;
  
  if (!AWAKE(victim))
    victim_ac -= dex_app[GET_DEX(victim)].defensive;
  
  victim_ac = MAX(-10, victim_ac);  /* -10 is lowest */
  
  if ((diceroll < 20) && AWAKE(victim) &&
      ((diceroll==1) || ((calc_thaco-diceroll) > victim_ac))) {
    return(FALSE);
  } else {
    return(TRUE);
  }
}

int MissVictim(struct char_data *ch, struct char_data *v, int type, int w_type,
	       int (*dam_func)())
{
  if (type <= 0) type = w_type;
  (*dam_func)(ch, v, 0, w_type);
}


int GetWeaponDam(struct char_data *ch, struct char_data *v, 
		 struct obj_data *wielded)
{
   int dam;
   struct obj_data *obj;
   extern struct str_app_type str_app[];

    
    dam  = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += GET_DAMROLL(ch);
    
    if (!wielded) {
      if (IS_NPC(ch) || HasClass(ch, CLASS_MONK))
        dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
      else
        dam += number(0,2);  /* Max. 2 dam with bare hands */
    } else { 
      if (wielded->obj_flags.value[2] > 0) {
        dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);
      } else {
        act("$p snaps into peices!", TRUE, ch, wielded, 0, TO_CHAR);
	act("$p snaps into peices!", TRUE, ch, wielded, 0, TO_ROOM);
	if ((obj = unequip_char(ch, WIELD))!=NULL) {
	  dam += 1;
	}
      }
    }
    
    if (GET_POS(v) < POSITION_FIGHTING)
      dam *= 1+(POSITION_FIGHTING-GET_POS(v))/3;
    /* Position  sitting  x 1.33 */
    /* Position  resting  x 1.66 */
    /* Position  sleeping x 2.00 */
    /* Position  stunned  x 2.33 */
    /* Position  incap    x 2.66 */
    /* Position  mortally x 3.00 */
    
    if (GET_POS(v) <= POSITION_DEAD)
      return(0);
    
    dam = MAX(1, dam);  /* Not less than 0 damage */
 
    return(dam);
    
}

int HitVictim(struct char_data *ch, struct char_data *v, int dam, 
		   int type, int w_type, int (*dam_func)())
{
  extern byte backstab_mult[];
  extern byte single_backstab_mult[];
  int dead;

    if (type == SKILL_BACKSTAB) {
     if (OnlyClass(ch, CLASS_THIEF)) {
      dam *= single_backstab_mult[GET_LEVEL(ch,THIEF_LEVEL_IND)];
     } else if (HasClass(ch, CLASS_THIEF)) {
      dam *= backstab_mult[GET_LEVEL(ch,THIEF_LEVEL_IND)];
     } else if (HasClass(ch, CLASS_ANTIPALADIN)); {
      dam *= backstab_mult[GET_LEVEL(ch,ANTIPALADIN_LEVEL_IND)];
     }
      dead = (*dam_func)(ch, v, dam, type);
    } else {
/*
  reduce damage for dodge skill:
*/
      if (v->skills && v->skills[SKILL_DODGE].learned) {
        if (number(1,101) <= v->skills[SKILL_DODGE].learned) {
          dam -= number(1,3);
          if (HasClass(v, CLASS_MONK)) {
            MonkDodge(ch, v, &dam);
          }
        }
      }
       dead = (*dam_func)(ch, v, dam, w_type);
    }
      
      /*
       *  if the victim survives, lets hit him with a 
       *  weapon spell
       */
      
    if (!dead) {
	WeaponSpell(ch,v,w_type);
    
    }
}


void root_hit(struct char_data *ch, struct char_data *victim, int type, 
	      int (*dam_func)())
{
  int w_type, thaco, dam, i;
  struct obj_data *wielded=0;  /* this is rather important. */

  if (IS_AFFECTED(ch, AFF_GRAPPLE)) {
    REMOVE_BIT(ch->specials.affected_by, AFF_GRAPPLE);
    SET_BIT(ch->specials.affected_by, AFF_GRAPPLE2);
    return;
  }

  if (IS_AFFECTED(ch, AFF_GRAPPLE2)) {
     REMOVE_BIT(ch->specials.affected_by, AFF_GRAPPLE2);
     return;
  }

  if ((ch->equipment[WIELD])&&(obj_index[ch->equipment[WIELD]->item_number].func)) {
    if((*obj_index[ch->equipment[WIELD]->item_number].func)(victim,OBJECT_HITTING, NULL,ch->equipment[WIELD])) {
         return;
         }
       } 

  if (HitCheckDeny(ch, victim, type)) return;

   if(IS_PC(ch)) 
     GET_MOVE(ch)-=1;
 
   setKillerFlag(ch,victim);

  w_type = GetWeaponType(ch, &wielded);
   if (w_type == TYPE_HIT)
    w_type = GetFormType(ch);  /* races have different types of attack */


  thaco = CalcThaco(ch);

  if (HitOrMiss(ch, victim, thaco)) {
    if ((dam = GetWeaponDam(ch, victim, wielded)) > 0) {
       HitVictim(ch, victim, dam, type, w_type, dam_func);
    } else {
       MissVictim(ch, victim, type, w_type, dam_func);
    }
  } else {
    MissVictim(ch, victim, type, w_type, dam_func);
  }

}

void MissileHit(struct char_data *ch, struct char_data *victim, int type)
{
  root_hit(ch, victim, type, GunMissileDamage); 
}

void BowHit(struct char_data *ch, struct char_data *victim, int type)
{
   root_hit(ch, victim, type, BowMissileDamage);
}

void hit(struct char_data *ch, struct char_data *victim, int type)
{
  root_hit(ch, victim, type, damage); 
}



/* control the fights going on */
void perform_violence(int pulse)
{
  struct char_data *ch, *vict;
  int i,t,found,perc;
  float x;
  
  for (ch = combat_list; ch; ch=combat_next_dude)	{
    struct room_data *rp;
    
    combat_next_dude = ch->next_fighting;
    assert(ch->specials.fighting);
    
    rp = real_roomp(ch->in_room);
    if (ch == ch->specials.fighting) {
      stop_fighting(ch);
    } else {
      
      if (IS_NPC(ch)) {
	struct char_data *rec;
	DevelopHatred(ch, ch->specials.fighting);
	rec = ch->specials.fighting;
	while (rec->master) {
	  AddHatred(ch, rec->master);
	  rec = rec->master;
	}
      }

      
      if (AWAKE(ch) && (ch->in_room==ch->specials.fighting->in_room) &&
	  (!IS_AFFECTED(ch, AFF_PARALYSIS))) {

	if (!IS_NPC(ch))  {
          /* set x = # of attacks */
          x = ch->mult_att;
 
          /* look to see if double attack skill gives another attack. */

            if (number(1,101) < ch->skills[SKILL_DOUBLE_ATTACK].learned) {
              /* Set mult_att to 2 for double success. */
              x = 2.000;
            }

          /* if dude is a monk, and is wielding something */
 
          if (HasClass(ch, CLASS_MONK)) {
            if (ch->equipment[WIELD]) {
              /* set it to one, they only get one attack */
              x = 1.000;
            }
          }
     
          /* work through all of their attacks, until there is not
             a full attack left */
 
          if (x > 6.0)
            x = 6.0;
 
 
          while (x > 0.999) {
            if (ch->specials.fighting)
              hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
            else {
              x = 0.0;
              break;
            }
            x -= 1.0;
          }
 
          if (x > .01) {
 
            /* check to see if the chance to make the last attack
               is successful 
               */
 
            perc = number(1,100);
            if (perc <= (int)(x*100.0)) {
              if (ch->specials.fighting)
                hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
            }
          }
 
        } else {
          x = ch->mult_att;
 
          if (x > 6.0)
            x = 6.0;
 
 
          while (x > 0.999) {
            if (ch->specials.fighting)
              hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
            else {
              if ((vict = FindAHatee(ch)) != NULL) {
                if (vict->attackers < 6)
                  hit(ch, vict, TYPE_UNDEFINED);
              } else if ((vict = FindAnAttacker(ch)) != NULL) {
                if (vict->attackers < 6)
                  hit(ch, vict, TYPE_UNDEFINED);
              }
            }
            x -= 1.0;
          }
 
          if (x > .01) {
 
            /* check to see if the chance to make the last attack
               is successful 
               */
 
            perc = number(1,100);
            if (perc <= (int)(x*100.0)) {
              if (ch->specials.fighting) {
                hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
              } else {
                if ((vict = FindAHatee(ch)) != NULL) {
                  if (vict->attackers < 6)
                    hit(ch, vict, TYPE_UNDEFINED);
                } else if ((vict = FindAnAttacker(ch)) != NULL) {
                  if (vict->attackers < 6)
                    hit(ch, vict, TYPE_UNDEFINED);
                }
              }
            }
          }
        }
      } else { /* Not in same room or not awake */
        stop_fighting(ch);
      }
    }
  }
}

         


struct char_data *FindVictim( struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found=FALSE;
  unsigned short ftot=0,ttot=0,ctot=0,ntot=0,mtot=0,
                 atot=0,ptot=0,ktot=0,rtot=0;
  unsigned short total;
  unsigned short fjump=0,njump=0,cjump=0,mjump=0,tjump=0,
                 ajump=0,pjump=0,kjump=0,rjump;
  
  if (ch->in_room < 0) return(0);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))&&
        (!IS_AFFECTED(tmp_ch, AFF_SNEAK) ||
        (IS_SET(ch->specials.act, ACT_META_AGG))) && (ch!=tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
	if (!IS_NPC(tmp_ch)||(IS_SET(tmp_ch->specials.act, ACT_ANNOYING))) {
	  if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	    found = TRUE;  /* a potential victim has been found */ 
	    if (!IS_NPC(tmp_ch)) {
	      if(HasClass(tmp_ch, CLASS_WARRIOR))
		ftot++;
	      else if (HasClass(tmp_ch, CLASS_CLERIC))
		ctot++;
	      else if (HasClass(tmp_ch,CLASS_MAGIC_USER))
		mtot++;
	      else if (HasClass(tmp_ch, CLASS_THIEF))
		ttot++;
              else if (HasClass(tmp_ch, CLASS_ANTIPALADIN))
                atot++;
              else if (HasClass(tmp_ch, CLASS_PALADIN))
                ptot++;
              else if (HasClass(tmp_ch, CLASS_RANGER))
                rtot++;
              else if (HasClass(tmp_ch, CLASS_MONK)) 
                ktot++;
	    } else {
	      ntot++;
	    }
	  }
	}
      }
    }
  }
  
  /* if no legal enemies have been found, return 0 */
  
  if (!found) {
    return(0);
  }
  
  /* 
    give higher priority to fighters, clerics, thieves, magic users if int <= 12
    give higher priority to fighters, clerics, magic users thieves is inv > 12
    give higher priority to magic users, fighters, clerics, thieves if int > 15
    */
  
  /*
    choose a target  
    */
  
  if (ch->abilities.intel <= 3) {
    fjump=2; cjump=2; tjump=2; njump=2; mjump=2;
    ajump=2; pjump=2; kjump=2; rjump=2;
  } else if (ch->abilities.intel <= 9) {
    fjump=4; pjump=4; ajump=3; cjump=3; rjump=3;
    kjump=2; tjump=2;njump=2;mjump=1;
  } else if (ch->abilities.intel <= 12) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=2; 
    ajump=2; pjump=2;rjump=2;kjump=2;
  } else if (ch->abilities.intel <= 15) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=3;
    ajump=2; pjump=2;kjump=2;rjump=2;
  } else {  
    fjump=3;cjump=3;tjump=2;njump=1;mjump=3;
    ajump=3;pjump=3;kjump=2;rjump=2;
  }
  
  total = (fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(njump*ntot)+(mjump*mtot)+
          (ajump*atot)+(pjump*ptot)+(kjump*ktot)+(rjump*rtot);
  
  total = number(1,total);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))&&
	(!IS_AFFECTED(tmp_ch, AFF_SNEAK) ||
         (IS_SET(ch->specials.act,ACT_META_AGG))) && (ch != tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
	if (!IS_NPC(tmp_ch) || (IS_SET(tmp_ch->specials.act, ACT_ANNOYING))) {
	  if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	    if (IS_NPC(tmp_ch)) {
	      total -= njump;
	    } else if (HasClass(tmp_ch,CLASS_WARRIOR)) {
	      total -= fjump;
	    } else if (HasClass(tmp_ch,CLASS_CLERIC)) {
	      total -= cjump;
	    } else if (HasClass(tmp_ch,CLASS_MAGIC_USER)) {
	      total -= mjump;
            } else if (HasClass(tmp_ch,CLASS_ANTIPALADIN)) {
              total -= ajump;
            } else if (HasClass(tmp_ch,CLASS_PALADIN)) {
              total -= pjump;
            } else if (HasClass(tmp_ch,CLASS_RANGER)) {
              total -= rjump;
            } else if (HasClass(tmp_ch,CLASS_MONK))  {
              total -= kjump;
	    } else {
	      total -= tjump;
	    }
	    if (total <= 0)
	      return(tmp_ch);
	  }
	}
      }
    }
  }
  
  if (ch->specials.fighting)
    return(ch->specials.fighting);
  
  return(0);
}

struct char_data *FindAnyVictim( struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found=FALSE;
  unsigned short ftot=0,ttot=0,ctot=0,ntot=0,mtot=0,
                 atot=0,ptot=0,ktot=0,rtot=0;
  unsigned short total;
  unsigned short fjump=0,njump=0,cjump=0,mjump=0,tjump=0,
                 ajump=0,kjump=0,rjump=0,pjump=0; 
  if (ch->in_room < 0) return(0);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))) {
      if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	if (!SameRace(ch, tmp_ch) || (!IS_NPC(tmp_ch))) {
	  found = TRUE;  /* a potential victim has been found */ 
	  if (!IS_NPC(tmp_ch)) {
	    if(HasClass(tmp_ch, CLASS_WARRIOR))
	      ftot++;
	    else if (HasClass(tmp_ch, CLASS_CLERIC))
	      ctot++;
	    else if (HasClass(tmp_ch,CLASS_MAGIC_USER))
	      mtot++;
	    else if (HasClass(tmp_ch, CLASS_THIEF))
	      ttot++;
            else if (HasClass(tmp_ch,CLASS_ANTIPALADIN))
              atot++;
            else if (HasClass(tmp_ch,CLASS_PALADIN))
              ptot++;
            else if (HasClass(tmp_ch,CLASS_RANGER))
              rtot++;
            else if (HasClass(tmp_ch,CLASS_MONK))
              ktot++;
	  } else {
	    ntot++;
	  }
	}
      }
    }
  }
  
  /* if no legal enemies have been found, return 0 */
  
  if (!found) {
    return(0);
  }
  
  /* 
    give higher priority to fighters, clerics, thieves, magic users if int <= 12
    give higher priority to fighters, clerics, magic users thieves is inv > 12
    give higher priority to magic users, fighters, clerics, thieves if int > 15
    */
  
  /*
    choose a target  
    */
  
  if (ch->abilities.intel <= 3) {
    fjump=2; cjump=2; tjump=2; njump=2; mjump=2;
    ajump=2; pjump=2; kjump=2; rjump=2;
  } else if (ch->abilities.intel <= 9) {
    fjump=4; cjump=3;tjump=2;njump=2;mjump=1;
    pjump=4; ajump=3;kjump=2;rjump=2;
  } else if (ch->abilities.intel <= 12) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=2;
    pjump=3; ajump=3;kjump=2;rjump=2;
  } else if (ch->abilities.intel <= 15) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=3;
    pjump=3; ajump=3;kjump=2;rjump=2;
  } else {  
    fjump=3;cjump=3;tjump=2;njump=1;mjump=3;
    pjump=3;ajump=3;kjump=2;rjump=1;
  }
  
  total = (fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(njump*ntot)+(mjump*mtot)+
          (ajump*atot)+(pjump*ptot)+(rjump*rtot)+(kjump*ktot); 
  total = number(1,total);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))) {
      if (!SameRace(tmp_ch, ch) || (!IS_NPC(tmp_ch))) {
	if (IS_NPC(tmp_ch)) {
	  total -= njump;
	} else if (HasClass(tmp_ch,CLASS_WARRIOR)) {
	  total -= fjump;
	} else if (HasClass(tmp_ch,CLASS_CLERIC)) {
	  total -= cjump;
	} else if (HasClass(tmp_ch,CLASS_MAGIC_USER)) {
	  total -= mjump;
        } else if (HasClass(tmp_ch,CLASS_ANTIPALADIN)) {
          total -= ajump;
        } else if (HasClass(tmp_ch,CLASS_PALADIN)) {
          total -= pjump;
        } else if (HasClass(tmp_ch,CLASS_RANGER)) {
          total -= rjump;
        } else if (HasClass(tmp_ch,CLASS_MONK)) {
          total -= kjump;
	} else {
	  total -= tjump;
	}
	if (total <= 0)
	  return(tmp_ch);
      }
    }      
  }
  
  if (ch->specials.fighting)
    return(ch->specials.fighting);
  
  return(0);
  
}

int BreakLifeSaverObj( struct char_data *ch)
{

      int found=FALSE, i, j;
      char buf[200];
      struct obj_data *o;

      /*
       *  check eq for object with the effect
       */
      for (i = 0; i< MAX_WEAR && !found; i++) {
	if (ch->equipment[i]) {
	  o = ch->equipment[i];
          for (j=0; j<MAX_OBJ_AFFECT; j++) {
            if (o->affected[j].location == APPLY_SPELL) {
              if (o->affected[j].modifier == AFF_LIFE_PROT) {
		 found = i;		 
	      }
	    }
	  }
	}
      }
      if (found) {

	/*
         *  break the fucker.
         */

	 sprintf(buf,"%s shatters with a blinding flash of light!\n\r", 
		 ch->equipment[found]->name);
	 send_to_char(buf, ch);
	 if ((o = unequip_char(ch, found)) != NULL) {
	 }
      }

}

int BrittleCheck(struct char_data *ch, int dam)
{
  char buf[200];
  struct obj_data *obj;

  if (dam <= 0)
    return(FALSE);

  if (ch->equipment[WIELD]) {
    if (IS_OBJ_STAT(ch->equipment[WIELD], ITEM_BRITTLE)) {
       if ((obj = unequip_char(ch,WIELD))!=NULL) {
	 sprintf(buf, "%s shatters.\n\r", obj->short_description);
	 send_to_char(buf, ch);
         return(TRUE);
       }
    }
  }
}

int PreProcDam(struct char_data *ch, int type, int dam)
{
  
  unsigned Our_Bit;
  
  /*
    long, intricate list, with the various bits and the various spells and
    such determined
    */
  
  switch (type) {
  case SPELL_FIREBALL:
  case SPELL_BURNING_HANDS:
  case SPELL_FLAMESTRIKE:
  case SPELL_FIRE_BREATH:
    Our_Bit = IMM_FIRE;
    break;
    
  case SPELL_SHOCKING_GRASP:
  case SPELL_LIGHTNING_BOLT:
  case SPELL_CALL_LIGHTNING:
  case SPELL_LIGHTNING_BREATH:
    Our_Bit = IMM_ELEC;
    break;
  case SPELL_CHILL_TOUCH:		     
  case SPELL_CONE_OF_COLD:		     
  case SPELL_ICE_STORM:			     
  case SPELL_FROST_BREATH:
    Our_Bit = IMM_COLD;
    break;
    
  case SPELL_MAGIC_MISSILE:
  case SPELL_COLOUR_SPRAY:
  case SPELL_GAS_BREATH:
  case SPELL_METEOR_SWARM:				     
    Our_Bit = IMM_ENERGY;
    break;
    
  case SPELL_ENERGY_DRAIN:
    Our_Bit = IMM_DRAIN;
    break;
    
  case SPELL_ACID_BREATH:
  case SPELL_ACID_BLAST:
    Our_Bit = IMM_ACID;
    break;
  case SKILL_BACKSTAB:				     
  case TYPE_PIERCE:
  case TYPE_STING:
  case TYPE_STAB:
    Our_Bit = IMM_PIERCE;
    break;
  case TYPE_SLASH:
  case TYPE_WHIP:
  case TYPE_CLEAVE:
  case TYPE_CLAW:
    Our_Bit = IMM_SLASH;
    break;
  case TYPE_BLUDGEON:
  case TYPE_HIT:
  case SKILL_KICK:
  case TYPE_CRUSH:
  case TYPE_BITE:
  case TYPE_SMASH:
  case TYPE_SMITE:
    Our_Bit = IMM_BLUNT;
    break;
  case SPELL_POISON:
    Our_Bit = IMM_POISON;
    break;
  default:
    return(dam);
    break;
  }
  
  if (IS_SET(ch->susc, Our_Bit))
    dam <<= 1;
  
  if (IS_SET(ch->immune, Our_Bit))
    dam >>= 1;
  
  if (IS_SET(ch->M_immune, Our_Bit))
    dam = 0;
  
  return(dam);
}


int ItemSave( struct obj_data *i, int dam_type) 
{
  int num, j;

  if (IS_OBJ_STAT(i,ITEM_BRITTLE)) {
    return(FALSE);
  }
  
  num = number(1,20);
  if (num <= 1) return(FALSE);
  if (num >= 20) return(TRUE);
  
  for(j=0; j<MAX_OBJ_AFFECT; j++)
    if ((i->affected[j].location == APPLY_SAVING_SPELL) || 
	(i->affected[j].location == APPLY_SAVE_ALL)) {
      num -= i->affected[j].modifier;
    }
  if (i->affected[j].location != APPLY_NONE) {
    num += 1;
  }
  if (i->affected[j].location == APPLY_HITROLL) {
    num += i->affected[j].modifier;
  }
  
  if (ITEM_TYPE(i) != ITEM_ARMOR)
    num += 1;
  
  if (num <= 1) return(FALSE);
  if (num >= 20) return(TRUE);
  
  if (num >= ItemSaveThrows[(int)GET_ITEM_TYPE(i)-1][dam_type-1]) {
    return(TRUE);
  } else {
    return(FALSE);
  }
}




int WeaponCheck(struct char_data *ch, struct char_data *v, int type, int dam)
{
  int Immunity, total, j;
  
  Immunity = -1;
  if (IS_SET(v->M_immune, IMM_NONMAG)) {
    Immunity = 0;
  }
  if (IS_SET(v->M_immune, IMM_PLUS1)) {
    Immunity = 1;
  }
  if (IS_SET(v->M_immune, IMM_PLUS2)) {
    Immunity = 2;
  }
  if (IS_SET(v->M_immune, IMM_PLUS3)) {
    Immunity = 3;
  }
  if (IS_SET(v->M_immune, IMM_PLUS4)) {
    Immunity = 4;
  }
  
  if (Immunity < 0)
    return(dam);
  
  if ((type < TYPE_HIT) || (type > TYPE_SMITE))  {
    return(dam);
  } else {
    if (type == TYPE_HIT || IS_NPC(ch)) {
      if (IS_NPC(ch) && (GetMaxLevel(ch) > (3*Immunity)+1)) {
	return(dam);
      } else {
	return(0);
      }
    } else {
      total = 0;
      if (!ch->equipment[WIELD])
	return(0);
      for(j=0; j<MAX_OBJ_AFFECT; j++)
	if ((ch->equipment[WIELD]->affected[j].location == APPLY_HITROLL) ||
	    (ch->equipment[WIELD]->affected[j].location == APPLY_HITNDAM)) {
	  total += ch->equipment[WIELD]->affected[j].modifier;
	}
      if (total > Immunity) {
	return(dam);
      } else {
	return(0);
      }     
    }
  }
}



int GetItemDamageType( int type)
{
  
  switch(type) {
  case SPELL_FIREBALL:
  case SPELL_FLAMESTRIKE:
  case SPELL_FIRE_BREATH:
    return(FIRE_DAMAGE);
    break;
    
  case SPELL_LIGHTNING_BOLT:
  case SPELL_CALL_LIGHTNING:
  case SPELL_LIGHTNING_BREATH:
    return(ELEC_DAMAGE);
    break;
    
  case SPELL_CONE_OF_COLD:
  case SPELL_ICE_STORM:
  case SPELL_FROST_BREATH:
    return(COLD_DAMAGE);
    break;
    
  case SPELL_COLOUR_SPRAY:
  case SPELL_METEOR_SWARM:
  case SPELL_GAS_BREATH:
    return(BLOW_DAMAGE);
    break;
    
  case SPELL_ACID_BREATH:
  case SPELL_ACID_BLAST:
    return(ACID_DAMAGE);
  default:
    return(0);
    break;  
  }
  
}

int SkipImmortals(struct char_data *v, int amnt)
{
  /* You can't damage an immortal! */
  
  if ((GetMaxLevel(v)>MAX_MORT) && !IS_NPC(v)) 
    amnt = 0;
  
  /* special type of monster */		
  if (IS_NPC(v) && (IS_SET(v->specials.act, ACT_IMMORTAL))) {
    amnt = -1;
  }
  return(amnt);
  
}


int WeaponSpell( struct char_data *c, struct char_data *v, int type)
{
  int j, num;
  
  if ((c->in_room == v->in_room) && (GET_POS(v) != POSITION_DEAD)) {
    if ((c->equipment[WIELD]) && ((type >= TYPE_BLUDGEON) &&
				  (type <= TYPE_SMITE))) {
      for(j=0; j<MAX_OBJ_AFFECT; j++) {
	if (c->equipment[WIELD]->affected[j].location ==
	    APPLY_WEAPON_SPELL) {
	  num = c->equipment[WIELD]->affected[j].modifier;
	  ((*spell_info[num].spell_pointer)
	   (6, c, "", SPELL_TYPE_WAND, v, 0));
	}
      }
    }
  }
}

struct char_data *FindAnAttacker(struct char_data *ch) 
{
  struct char_data *tmp_ch;
  unsigned char found=FALSE;
  unsigned short ftot=0,ttot=0,ctot=0,ntot=0,mtot=0;
  unsigned short total;
  unsigned short fjump=0,njump=0,cjump=0,mjump=0,tjump=0;
  
  if (ch->in_room < 0) return(0);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;
       tmp_ch=tmp_ch->next_in_room) {
       if (ch!=tmp_ch) {
	  if (tmp_ch->specials.fighting == ch) {
	      found = TRUE;  /* a potential victim has been found */ 
	      if (!IS_NPC(tmp_ch)) {
		if(HasClass(tmp_ch, CLASS_WARRIOR))
		  ftot++;
		else if (HasClass(tmp_ch, CLASS_CLERIC))
		  ctot++;
		else if (HasClass(tmp_ch,CLASS_MAGIC_USER))
		  mtot++;
		else if (HasClass(tmp_ch, CLASS_THIEF))
		  ttot++;
	      } else {
		ntot++;
	      }
	    }
	}
     }
  
  /* if no legal enemies have been found, return 0 */
  
  if (!found) {
    return(0);
  }
  
  /* 
    give higher priority to fighters, clerics, thieves, magic users if int <= 12
    give higher priority to fighters, clerics, magic users thieves is inv > 12
    give higher priority to magic users, fighters, clerics, thieves if int > 15
    */
  
  /*
    choose a target  
    */
  
  if (ch->abilities.intel <= 3) {
    fjump=2; cjump=2; tjump=2; njump=2; mjump=2;
  } else if (ch->abilities.intel <= 9) {
    fjump=4; cjump=3;tjump=2;njump=2;mjump=1;
  } else if (ch->abilities.intel <= 12) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=2;
  } else if (ch->abilities.intel <= 15) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=3;
  } else {  
    fjump=3;cjump=3;tjump=2;njump=1;mjump=3;
  }
  
  total = (fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(njump*ntot)+(mjump*mtot);
  
  total = number(1,total);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;
       tmp_ch=tmp_ch->next_in_room) {
	    if (tmp_ch->specials.fighting == ch) {
	      if (IS_NPC(tmp_ch)) {
		total -= njump;
	      } else if (HasClass(tmp_ch,CLASS_WARRIOR)) {
		total -= fjump;
	      } else if (HasClass(tmp_ch,CLASS_CLERIC)) {
		total -= cjump;
	      } else if (HasClass(tmp_ch,CLASS_MAGIC_USER)) {
		total -= mjump;
	      } else {
		total -= tjump;
	      }
	      if (total <= 0)
		return(tmp_ch);
	    }
	  }
  
  if (ch->specials.fighting)
    return(ch->specials.fighting);
  
  return(0);
}


void fire( struct char_data *ch, struct char_data *victim)
{
   struct obj_data *bow;
   int tohit=0, todam=0;

   bow = ch->equipment[HOLD];

   if (!bow || bow->obj_flags.type_flag != ITEM_BOW ) {
     send_to_char("You must be holding a bow to fire one!\n\r", ch);
     return;
   } else {
      if (bow->obj_flags.value[3] >=1)
      {
        bow->obj_flags.value[3]--;
        BowHit(ch,victim,SPEC_BOW);
      } else {
       send_to_char("Your bow has no arrow. It twangs as you try to shoot it!\n\r", ch);
      }
   }
}

void shoot( struct char_data *ch, struct char_data *victim)
{
  struct obj_data *gun;
  int tohit=0, todam=0;
  
  gun = ch->equipment[HOLD];

  if (!gun || gun->obj_flags.type_flag != ITEM_FIREWEAPON ) {
    send_to_char("You need to be holding a gun.\n\r", ch);
    return;
  } else {
    /*
    **  for guns:  value[0] = arror type
    **             value[1] = rolls
    **             value[2] = dice max 
    **             value[3] = current shots
    **
    **   fire the weapon.
    */
    if ( gun->obj_flags.value[3] >= 1 )
    {
    	gun->obj_flags.value[3]--;
    	MissileHit(ch, victim, SPEC_SHOOT);
    } else
    {
	send_to_char ( "Click!  It seems to be empty.\n\r", ch );
	act ( "Click!  $n tries to fire an empty weapon.", FALSE, ch, 0, 0, 
		TO_ROOM );
    }
  }
}

struct char_data *FindMetaVictim( struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found=FALSE;
  unsigned short total=0;

  
  if (ch->in_room < 0) return(0);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))) {
      if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	if (!IS_NPC(ch)) {
	   found = TRUE;
	   total++;
	}
      }
    }
  }
  
  /* if no legal enemies have been found, return 0 */
  
  if (!found) {
    return(0);
  }
  
  total = number(1,total);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))) {
      if (!SameRace(tmp_ch, ch)){
	total--;
	if (total == 0)
	  return(tmp_ch);
      }
    }
  }
  
  if (ch->specials.fighting)
    return(ch->specials.fighting);
  
  return(0);
  
}


int GetFormType(struct char_data *ch)
{
  int num;
 
  num = number(1,100);
  switch(GET_RACE(ch)) {
  case RACE_LYCANTH:
  case RACE_DRAGON:
  case RACE_PREDATOR:
  case RACE_LABRAT: {
    if (num <= 33) {
      return(TYPE_BITE);
    } else {
      return(TYPE_CLAW);
    }
    break;
   }
  case RACE_INSECT:
    if (num <= 50) {
      return(TYPE_BITE);
    } else {
      return(TYPE_STING);
    }
    break;
  case RACE_ARACHNID:
  case RACE_DINOSAUR:
  case RACE_FISH:
  case RACE_SNAKE:
    return(TYPE_BITE);
    break;
  case RACE_BIRD:
  case RACE_SKEXIE:
    return(TYPE_CLAW);
    break;
  case RACE_GIANT:
  case RACE_GOLEM:
    return(TYPE_BLUDGEON);
    break;
  case RACE_DEMON:
  case RACE_DEVIL:
  case RACE_TROLL:
  case RACE_TROGMAN:
    return(TYPE_CLAW);
    break;
  case RACE_TREE:
    return(TYPE_SMITE);
    break;
  case RACE_MFLAYER:
    if (num <= 60) {
      return(TYPE_WHIP);
    } else if (num < 80) {
      return(TYPE_BITE);
    } else {
      return(TYPE_HIT);
    }
    break;
  case RACE_PRIMATE:
    if (num <= 70) {
      return(TYPE_HIT);
    } else {
      return(TYPE_BITE);
    }
    break;
  default:
    return(TYPE_HIT);
  }
}


int MonkDodge( struct char_data *ch, struct char_data *v, int *dam)
{
  if (number(1, 20000) < v->skills[SKILL_DODGE].learned*
                         GET_LEVEL(ch, MONK_LEVEL_IND)) {
    *dam = 0;
    act("You dodge the attack", FALSE, ch, 0, v, TO_VICT);
    act("$N dodges the attack", FALSE, ch, 0, v, TO_CHAR);
    act("$N dodges $n's attack", FALSE, ch, 0, v, TO_NOTVICT);
  } else {
    *dam -= GET_LEVEL(ch, MONK_LEVEL_IND)/10;
  }
 
  return(0);
}
