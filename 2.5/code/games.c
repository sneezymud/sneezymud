/*  Module for playing casino type games on DikuMUD, made for SneezyMUD

     by Russ Russell.                                                  */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
 
#include "structs.h"
#include "games.h"
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

struct char_data *FindMobInRoomWithFunction(int room, int (*func)());
int craps_table_man(struct char_data *ch, int cmd, char *arg);
extern struct index_data *obj_index;
extern struct obj_data *object_list;
extern struct char_data *character_list;

void spin_slot(struct char_data *ch);
void roll_dice(struct char_data *ch);

char *ChooseFirstFruit()
{
   int num;
   static char *fruits[8] = {
     "cherry",
     "lime",
     "orange",
     "apple",
     "banana",
     "bell",
     "bally",
     "Seven",
    };
    num = number(1,15);
 
   if (num == 1)
     return(fruits[7]);
   else if (num <= 3)
     return(fruits[6]);
   else if (num <= 12)
     return(fruits[number(1,5)]);     
   else
     return(fruits[0]);
}

char *ChooseSecondFruit()
{
   int num;
   static char *fruits[8] = {
     "cherry",
     "lime",
     "orange",
     "apple",
     "banana",
     "bell",
     "bally",
     "Seven",
    };
    num = number(1,15);
   if (num == 1)
     return(fruits[7]);
   else if (num <= 3)
     return(fruits[6]);
   else if (num <= 12)
     return(fruits[number(1,5)]);     
   else
     return(fruits[0]);
}

char *ChooseThirdFruit()
{
   int num;
   static char *fruits[8] = {
     "cherry",
     "lime",
     "orange",
     "apple",
     "banana",
     "bell",
     "bally",
     "Seven",
    };
    num = number(1,15);
   if (num == 1)
     return(fruits[7]);
   else if (num <= 3)
     return(fruits[6]);
   else if (num <= 12)
     return(fruits[number(1,5)]);     
   else
     return(fruits[0]);
}

void lose_dice(struct char_data *ch)
{
  struct obj_data *dice;
  struct char_data *crap_man;

  dice = ch->equipment[HOLD];

  crap_man = FindMobInRoomWithFunction(ch->in_room, craps_table_man); 

  if (crap_man && dice)
    obj_to_char(unequip_char(ch,HOLD), crap_man);
  
  do_say(crap_man,"Next roller please?", 0);
}

void get_dice(struct char_data *ch)
{
   struct obj_data *dice;
   struct char_data *crap_man;
   char buf[80];
 
   crap_man = FindMobInRoomWithFunction(ch->in_room, craps_table_man);

   if (!crap_man)
        return;
   else {
     dice = get_obj_in_list_vis(crap_man, "qwert", crap_man->carrying);
     if (!dice) return;   
      sprintf(buf, "Fine %s, Here are the dice!", GET_NAME(ch));
      do_say(crap_man,buf,0);
      do_say(crap_man,"Hold the dice, and throw them when I say its ok!", 0);
      obj_from_char(dice);
      obj_to_char(dice, ch);

      if (!crap_man->act_ptr)
       crap_man->act_ptr = (int *) calloc(1, (sizeof(int)));
    
      (*((int *) crap_man->act_ptr)) = START_BETS;
    }
}
int check_for_dice_held(struct char_data *ch)
{
  struct obj_data *dice;

  dice = get_obj_in_list_vis(ch, "qwert", ch->equipment[HOLD]);

  if(dice) 
    return(TRUE);
  else
    return(FALSE);
}

int check_for_dice_in_inv(struct char_data *ch)
{
   struct obj_data *dice;
  
   dice = get_obj_in_list_vis(ch,"qwert", ch->carrying);

   if (dice)
     return(TRUE);
   else
     return(FALSE);
}

int check_pointroll(struct char_data *ch)
{
  struct char_data *tmp_char, *temp;
  
  for (tmp_char = character_list;tmp_char;tmp_char=temp) {
     temp = tmp_char->next;
      if  (tmp_char->point_roll != 0)
              return(TRUE);
   }
   return(FALSE);
}
  


int check_slots(struct char_data *ch)
{
  if ((ch->in_room < 8414) && (ch->in_room > 8403))
      return(TRUE);
  else
      return(FALSE);
}

int check_slot_player(struct char_data *ch)
{
  struct char_data *better, *temp;

  for (better = character_list;better;better = temp) {
    temp = better->next;
   if (ch->in_room == better->in_room) {
    if (GET_POS(better) == POSITION_SITTING) {
       return(TRUE);
    }
   }
  }
  return(FALSE);
}

void do_bet(struct char_data *ch, char *arg, int cmd)
{
  char buf[255];
  char amount[15], craps[255];  
  int num, number, opt;

  half_chop(arg, amount, craps);
 
  if (check_blackjack(ch))
    do_bj_bet(ch, arg, 0);
  
  if (!*amount) {
     send_to_char(BET_OPTIONS, ch);
     return;
   } else {
      if (is_abbrev(amount,"one")) {
        if (!*craps) {
         send_to_char(ONEROLL_OPTIONS, ch);
         return;
        }
      }
    if (*craps) {
     if (isdigit(*craps)) {
      num = atoi(craps);
      if (num > GET_GOLD(ch)) {
       send_to_char("You don't have that much to bet!\n\r", ch);
       return;
      }
      if (num <= 0) {
       send_to_char("Nice try.\n\r", ch);
       return;
      }
      if (is_abbrev(amount,"crap")) {
        if (ch->bet.crap == 0) {
           ch->bet_opt.craps_options += CRAP_OUT;
           sprintf(buf, "You just placed %d gold down on a no pass bet.\n\r", num);
           send_to_char(buf, ch);
        } else {
           sprintf(buf, "You just added %d gold to your no pass bet.\n\r",num);
           send_to_char(buf, ch);
        } 
        GET_GOLD(ch) -= num;
        ch->bet.crap += num;
      } else if (is_abbrev(amount,"come")) {
        get_dice(ch);
        if (ch->bet.come == 0) {
           ch->bet_opt.craps_options += COME_OUT;
           sprintf(buf, "You just placed %d gold down on the come out roll.\n\r", num);
        } else {
           sprintf(buf, "You add %d gold to your bet on the come out roll.\n\r", num);
        }
         send_to_char(buf, ch);
         GET_GOLD(ch) -= num;
         ch->bet.come += num; 
      } else if (is_abbrev(amount,"three")) {
        if (ch->bet.three == 0) {
           ch->bet_opt.one_roll += THREE3;
           sprintf(buf, "You just placed %d gold down on a oneroll three (3) bet\n\r", num);
         } else {
           sprintf(buf, "You add %d gold to your bet on the oneroll three (3).\n\r", num);
         }
         send_to_char(buf, ch);
         GET_GOLD(ch) -= num;
         ch->bet.three += num;
       } else if (is_abbrev(amount,"two")) {
         if (ch->bet.two == 0) {
           ch->bet_opt.one_roll += TWO2;
           sprintf(buf, "You just placed %d gold down on a oneroll two (2) bet\n\r", num);
         } else {
           sprintf(buf, "You add %d gold to your bet on the oneroll two (2).\n\r", num);
         }
         send_to_char(buf, ch);
         GET_GOLD(ch) -= num;
         ch->bet.two += num;
       } else if (is_abbrev(amount,"eleven")) {
          if (ch->bet.eleven == 0) {
            ch->bet_opt.one_roll += ELEVEN;
            sprintf(buf, "You just placed %d gold down on a oneroll eleven (11) bet.\n\r", num);
         } else {
            sprintf(buf, "You add %d gold to your bet on the oneroll eleven (11).\n\r", num);
         } 
         send_to_char(buf, ch);
         GET_GOLD(ch) -= num;
         ch->bet.eleven += num;
       } else if (is_abbrev(amount, "twelve")) {
          if (ch->bet.twelve == 0) {
            ch->bet_opt.one_roll += TWELVE;
            sprintf(buf, "You just placed %d gold down on a oneroll twelve (12) bet.\n\r", num);
          } else {
            sprintf(buf, "You add %d gold to your bet on a oneroll twelve (12) bet.\n\r", num);
          }  
          send_to_char(buf, ch);
          GET_GOLD(ch) -= num;
          ch->bet.twelve += num;  
       } else if (is_abbrev(amount, "horn")) {
          if (ch->bet.horn_bet == 0) {
            ch->bet_opt.one_roll += HORN_BET;
            sprintf(buf, "You just placed %d gold down on a oneroll horn bet.\n\r", num);
          } else {
            sprintf(buf, "You add %d gold to your bet on a oneroll horn bet.\n\r", num);
          }  
          send_to_char(buf, ch);
          GET_GOLD(ch) -= num;
          ch->bet.horn_bet += num;  
       } else if (is_abbrev(amount, "field")) {
          if (ch->bet.field_bet == 0) {
            ch->bet_opt.one_roll += FIELD_BET;
            sprintf(buf, "You just placed %d gold down on a oneroll field bet.\n\r", num);
          } else {
            sprintf(buf, "You add %d gold to your bet on a oneroll field bet.\n\r", num);
          }  
          send_to_char(buf, ch);
          GET_GOLD(ch) -= num;
          ch->bet.field_bet += num;  
       } else if (is_abbrev(amount, "seven")) {
          if (ch->bet.seven == 0) {
            ch->bet_opt.one_roll += SEVEN;
            sprintf(buf, "You just placed %d gold down on a oneroll seven (7) bet.\n\r", num);
          } else {
            sprintf(buf, "You add %d gold to your bet on a oneroll seven (7) bet.\n\r", num);
          }  
          send_to_char(buf, ch);
          GET_GOLD(ch) -= num;
          ch->bet.seven += num;
         } else {
           send_to_char("Wrong option.\n\r", ch);
           return;
         } 
       }
     }
   }
}



void do_play(struct char_data *ch, char *arg, int cmd)
{
   char bet[255], buf[255], game[255], options[255];
   struct obj_data *slot;
   int option, opt;
   
  half_chop(arg, game, options);

  if (!*game) {
     send_to_char("Syntax <play game options>\n\r", ch);
     send_to_char("Games include : Craps, Roulette, Slots, and 21\n\r", ch);
     send_to_char("Typing <play game> will show the options for each game.\n\r", ch);
     return;
  } else if (is_abbrev(game, "slots")) {
   
   if (!(GET_POS(ch) == POSITION_SITTING)) {
     send_to_char("You must sit at the slot machine to play it.\n\r", ch);
     return;
   }

   if (!check_slots) {
        send_to_char("No slot machine in this room!\n\r", ch);
        return;
        }

    if (!*options) {
     send_to_char("Slot machine options :\n\r", ch);
     send_to_char("1) Play the cheap slots, and bet 1 coins (100 coins).\n\r", ch);
     send_to_char("2) Play the cheap slots, and bet 2 coins (200 coins).\n\r", ch);
     send_to_char("3) Play the cheap slots, and bet 3 coins (300 coins).\n\r", ch);
     send_to_char("4) Play the expensive slots and bet 1 coin (1000 mud coins.\n\r", ch);
     send_to_char("5) Play the expensive slots and bet 2 coins (2000 mud coins.\n\r", ch);
     send_to_char("6) Play the expensive slots and bet 3 coins (3000 mud coins.\n\r", ch);
     send_to_char("You MUST play the 3000 coins machine to have a chance at the jackpot\n\r", ch);
     } else {
      if (!strcmp(options, "1")) {
       if (GET_GOLD(ch) >= 100) {
       ch->bet.slot = 100;
       GET_GOLD(ch) -= 100;
       spin_slot(ch);
       } else { 
        send_to_char("You search your pockets for coins but come up emtpy-handed.\n\r", ch);
        return;
       }
      } else if (!strcmp(options, "2")) {
       if (GET_GOLD(ch) >= 200) {
       ch->bet.slot = 200;
       GET_GOLD(ch) -= 200;
       spin_slot(ch);
       } else {
         send_to_char("You search your pockets for coins but come up emtpy-handed.\n\r", ch);
         return;
       }
      } else if (!strcmp(options, "3")) {
       if (GET_GOLD(ch) >= 300) {
       ch->bet.slot = 300;
       GET_GOLD(ch) -= 300;
       spin_slot(ch);
       } else {
         send_to_char("You search your pockets for coins but come up emtpy-handed.\n\r", ch);
         return;
       }
      } else if (!strcmp(options, "4")) {
       if (GET_GOLD(ch) >= 1000) {
       ch->bet.slot = 1000;
       GET_GOLD(ch) -= 1000;
       spin_slot(ch);
       } else {
         send_to_char("You search your pockets for coins but come up emtpy-handed.\n\r", ch);
         return;
       }
      } else if (!strcmp(options, "5")) {
       if (GET_GOLD(ch) >= 2000) {
       ch->bet.slot = 2000;
       GET_GOLD(ch) -= 2000;
       spin_slot(ch);
       } else {
         send_to_char("You search your pockets for coins but come up emtpy-handed.\n\r", ch);
         return;
       }
      } else if (!strcmp(options, "6")) {
       if (GET_GOLD(ch) >= 3000) {
       ch->bet.slot = 3000;
       GET_GOLD(ch) -= 3000;
       spin_slot(ch);
       } else {
         send_to_char("You search your pockets for coins but come up emtpy-handed.\n\r", ch);
         return;
       }
      } else {
        send_to_char("Incorrect Option.\n\r", ch);
        return;
      }
    }
  } else if (is_abbrev(game, "roulette")) {
    if (!*options) {
        send_to_char(R_TABLE, ch);
        return;
    }
  } else if (is_abbrev(game, "craps")) {
    if (!*options) {
        send_to_char(CRAPS_OPTIONS, ch);
        return;
    } else {
       if (isdigit(*options)) {
          option = atoi(options);
          roll_dice(ch);
          return;
       } else {
          send_to_char("Options need to be numbers between 1-20\n\r", ch);
          return;
       }
     }
   }
}                

void clear_bets(struct char_data *ch)
{
  ch->bet.come      = 0;
  ch->bet.crap      = 0;
  ch->bet.eleven    = 0;
  ch->bet.horn_bet  = 0;
  ch->bet.slot      = 0;
  ch->bet.twelve    = 0;
  ch->bet.two       = 0;
  ch->bet.three     = 0;
  ch->bet.field_bet = 0;
  ch->bet.hard_four = 0;
  ch->bet.hard_eight = 0;
  ch->bet.hard_six  = 0;
  ch->bet.hard_ten  = 0;
  ch->bet.seven     = 0;
  ch->bet.one_craps = 0;
}


int can_bet_craps(struct char_data *ch)
{
   struct char_data *crap_man;
   char buf[80];
   
   crap_man = FindMobInRoomWithFunction(ch->in_room, craps_table_man);

   if (!crap_man) return(FALSE);

   if (!crap_man->act_ptr)
     crap_man->act_ptr = (int *) calloc(1, (sizeof(int)));

   if ((*((int *) crap_man->act_ptr))==0)
    return(TRUE);
   else
    return(FALSE);
}

   


void check_craps(struct char_data *ch, int diceroll)
{
   struct char_data *better, *temp;
    char buf[255];

  if (diceroll != 2)
    if (diceroll != 3)
      if (diceroll != 12)
         return;

     if (ch->point_roll != 0) return;


  for (better = character_list;better;better=temp) {
        temp = better->next;
      if (ch->in_room == better->in_room) {
        if (IS_SET(better->bet_opt.craps_options, COME_OUT)) {
         send_to_char("Craps....You Lose!\n\r", better);
         REMOVE_BIT(better->bet_opt.craps_options, COME_OUT);
         better->bet.come = 0;
         if (better==ch)
          lose_dice(ch);
        }
        if (IS_SET(better->bet_opt.craps_options, CRAP_OUT)) {
         send_to_char("Craps....You Win!\n\r", better);
         REMOVE_BIT(better->bet_opt.craps_options, CRAP_OUT);
         GET_GOLD(better) += 2*better->bet.crap;
         better->bet.crap = 0;
         }
      }
   }
}
        

int CheckForPoint(struct char_data *ch) 
{
    struct obj_data *i;

   for (i = object_list;i;i = i->next) {
     if (isname("qwert", i->name)) {
       if (i->carried_by) {
         if (i->carried_by->in_room == ch->in_room) {
          if (i->carried_by->point_roll != 0) {
            return(TRUE);
          } else {
            return(FALSE);
          }
        }
      }
    } 
  }
}

            
void check_seven(struct char_data *ch, int diceroll)
{
   struct char_data *better, *temp;

     if (diceroll != 7) return;

     for (better = character_list;better;better = temp) {
       temp =  better->next;
      
     if (ch->in_room == better->in_room) {
      if (ch->point_roll == 0) {
       if (IS_SET(better->bet_opt.craps_options, COME_OUT)) {
        send_to_char("Seven! You win your come out bet!\n\r", better);
        GET_GOLD(better) += 2*better->bet.come;
        REMOVE_BIT(better->bet_opt.craps_options, COME_OUT);
        better->bet.come = 0;
       } 
       if (IS_SET(better->bet_opt.craps_options, CRAP_OUT)) {
        send_to_char("Seven! You lose your crap out bet!\n\r", better);
        REMOVE_BIT(better->bet_opt.craps_options, CRAP_OUT);
        better->bet.crap = 0;
        if (better==ch)
         lose_dice(ch);
       }
      } else {
       if (IS_SET(better->bet_opt.craps_options, COME_OUT)) {
        send_to_char("Seven! You lose your come out bet!\n\r", better);
        REMOVE_BIT(better->bet_opt.craps_options, COME_OUT);
        better->bet.come = 0;
        if (better==ch)
         lose_dice(ch);
       }
       if (IS_SET(better->bet_opt.craps_options, CRAP_OUT)) {
        send_to_char("Seven! You win your crap out bet!\n\r", better);
        GET_GOLD(better) += 2*better->bet.crap;
        REMOVE_BIT(better->bet_opt.craps_options, CRAP_OUT);
        better->bet.crap = 0;
        } 
      }
    }
  }
  ch->point_roll = 0;
}
        

void check_eleven(struct char_data *ch, int diceroll)
{
  
    struct char_data *better, *temp;
    char buf[255];

  if (diceroll != 11) return;

  if (ch->point_roll != 0) return;

  for (better = character_list;better;better=temp) {
        temp = better->next;
      if (ch->in_room == better->in_room) {
       if (IS_SET(better->bet_opt.craps_options, COME_OUT)) {
         send_to_char("Seven come ELEVEN! You hit your come out bet!\n\r", better);
         GET_GOLD(better) += 2*better->bet.come;
         REMOVE_BIT(better->bet_opt.craps_options, COME_OUT);
         better->bet.come = 0;
       }
       if (IS_SET(better->bet_opt.craps_options, CRAP_OUT)) {
        send_to_char("Seven come ELEVEN! You hit the come out bet. You lose!\n\r", better);
        REMOVE_BIT(better->bet_opt.craps_options, CRAP_OUT);
        better->bet.crap = 0;
        if (better==ch)
         lose_dice(ch);
      }
    }
  }
}
       


void set_point(struct char_data *ch, int diceroll)
{
   char buf[255];
 
   if ((diceroll == 2) || (diceroll == 3) || (diceroll == 7) ||
       (diceroll == 11) || (diceroll == 12)) return;
 
   ch->point_roll = diceroll;

   sprintf(buf, "%d is the point. The point is %d.\n\r", diceroll, diceroll);
   send_to_room(buf, ch->in_room);
}

void check_oneroll_seven(struct char_data *better, int diceroll)
{

   if (diceroll == 7) {
    send_to_char("You win your oneroll seven bet!\n\r", better);
    GET_GOLD(better) += 4*better->bet.seven;
   } else {
    send_to_char("You lose your oneroll seven bet.\n\r", better);
   }
   REMOVE_BIT(better->bet_opt.one_roll, SEVEN);
   better->bet.seven = 0;
}

void check_horn(struct char_data *better, int diceroll)
{
   char buf[255];

   if ((diceroll >3) || (diceroll < 11)) {
     send_to_char("You lose your horn bet.\n\r", better);
   } else {
     sprintf(buf, "The roll is a [%d]. You win your horn bet!\n\r", diceroll);
     send_to_char(buf, better);
     if ((diceroll == 2) || (diceroll == 12))
       GET_GOLD(better) += 7*better->bet.horn_bet;
     else
       GET_GOLD(better) += (int) (3.5*better->bet.horn_bet);
   }
   REMOVE_BIT(better->bet_opt.one_roll, HORN_BET);
   better->bet.horn_bet = 0;
}

void check_field(struct char_data *better, int diceroll)
{
    struct char_data *crap_man;
    char buf[100];

    crap_man = FindMobInRoomWithFunction(better->in_room, craps_table_man);

    if ((diceroll >= 5) && (diceroll <= 8)) {
     if (crap_man) {
      sprintf(buf, "%s The roll is %d. You lose your bet on the field.",
             GET_NAME(better), diceroll);
      do_tell(crap_man, buf, 0);
     }
    } else {
      if (crap_man) {
       sprintf(buf, "%s The roll is %d. You win your bet on the field!",
               GET_NAME(better), diceroll);
       do_tell(crap_man, buf, 0);
      }
         if ((diceroll < 12) && (diceroll > 2))
            GET_GOLD(better) += 2*better->bet.field_bet;
         else
            GET_GOLD(better) += better->bet.field_bet;
    }
    better->bet.field_bet = 0;
    REMOVE_BIT(better->bet_opt.one_roll, FIELD_BET);
}
      
      

void check_hard_four(struct char_data *better, int diceroll)
{
   struct char_data *tmp_better, *temp;
   char buf[255];
}

void check_hard_six(struct char_data *better, int diceroll)
{
}

void check_hard_eight(struct char_data *better, int diceroll)
{
}

void check_hard_ten(struct char_data *better, int diceroll)
{
}

void check_hardrolls(struct char_data *better, int diceroll)
{
}


void check_two(struct char_data *better, int diceroll)
{
  char buf[255];

        if (diceroll == 2) {
         send_to_char("Two hit!. Nice bet!\n\r", better);
         GET_GOLD(better) += 31*better->bet.two;
         REMOVE_BIT(better->bet_opt.one_roll, TWO2);
         better->bet.two = 0;
        } else {
         send_to_char("No two....You lose your two bet!\n\r", better);
         REMOVE_BIT(better->bet_opt.one_roll, TWO2);
         better->bet.two = 0;
      }
}

void check_three(struct char_data *better, int diceroll)
{
  char buf[255];

        if (diceroll == 3) {
         send_to_char("Three hit! Nice bet!\n\r", better);
         GET_GOLD(better) += 16*better->bet.three;
         REMOVE_BIT(better->bet_opt.one_roll, THREE3);
         better->bet.three = 0;
        } else {
         send_to_char("No three....You lose your three bet!\n\r", better);
         REMOVE_BIT(better->bet_opt.one_roll, THREE3);
         better->bet.three = 0;
        }
}

void check_oneroll_eleven(struct char_data *better, int diceroll)
{
  char buf[255];
 
        if (diceroll == 11) {
         send_to_char("Eleven hit! Nice bet!\n\r", better);
         GET_GOLD(better) += 16*better->bet.eleven;
         REMOVE_BIT(better->bet_opt.one_roll, ELEVEN);
         better->bet.eleven = 0;
        } else {
         send_to_char("No eleven....You lose your eleven bet!\n\r", better);
         REMOVE_BIT(better->bet_opt.one_roll, ELEVEN);
         better->bet.eleven = 0;
         }
}

void check_twelve(struct char_data *better, int diceroll)
{
  char buf[255];
 
        if (diceroll == 12) {
         send_to_char("Twelve hit! Nice bet!\n\r", better);
         GET_GOLD(better) += 31*better->bet.twelve;
        } else {
         send_to_char("No twelve....You lose your twelve bet!\n\r", better);
        }
        REMOVE_BIT(better->bet_opt.one_roll, TWELVE);
        better->bet.twelve = 0;
}

void check_oneroll_craps(struct char_data *better, int diceroll)
{
  char buf[255];
 
        if (diceroll == 3) {
         send_to_char("Three hit! Your bet on craps hit!\n\r", better);
         REMOVE_BIT(better->bet_opt.one_roll, CRAPS);
         GET_GOLD(better) += 7*better->bet.one_craps;
        } else if (diceroll == 2) {
         send_to_char("Two hit! Your bet on craps hit!\n\r", better);
         REMOVE_BIT(better->bet_opt.one_roll, CRAPS);
         GET_GOLD(better) += 7*better->bet.one_craps;
        } else if (diceroll == 12) {
         send_to_char("Twelve hit. Your bet on craps hit!\n\r", better); 
         REMOVE_BIT(better->bet_opt.one_roll, CRAPS);
         GET_GOLD(better) += 7*better->bet.one_craps;
        } else {
         send_to_char("No craps....You lose your bet on craps!\n\r", better);
         REMOVE_BIT(better->bet_opt.one_roll, CRAPS);
        }
        better->bet.one_craps = 0;
}

void check_onerolls(struct char_data *ch, int diceroll)
{

    struct char_data *better, *temp;

  for (better = character_list;better;better=temp) {
        temp = better->next;
      if (ch->in_room == better->in_room) {

  if (IS_SET(better->bet_opt.one_roll, TWO2)) { 
    check_two(better, diceroll);
  }
  if (IS_SET(better->bet_opt.one_roll, THREE3)) {
    check_three(better, diceroll);
  }
  if (IS_SET(better->bet_opt.one_roll, ELEVEN)) {
    check_oneroll_eleven(better, diceroll);
  }
  if (IS_SET(better->bet_opt.one_roll, TWELVE)) {
    check_twelve(better, diceroll);
  }
  if (IS_SET(better->bet_opt.one_roll, CRAPS)) {
    check_oneroll_craps(better, diceroll);
   }
  if (IS_SET(better->bet_opt.one_roll, SEVEN)) {
     check_oneroll_seven(better, diceroll);
   }
  if (IS_SET(better->bet_opt.one_roll, FIELD_BET)) {
    check_field(better, diceroll);
   }
  }
 }
}



void WinLoseCraps(struct char_data *ch, int diceroll)
{
   struct char_data *better, *temp;
   char buf[255];

  for (better = character_list;better;better=temp) {
        temp = better->next;
      if ((ch->in_room == better->in_room) && (IS_PC(better))) {
        if (IS_SET(better->bet_opt.craps_options, COME_OUT)) {
         sprintf(buf, "The point [%d] was hit. You win!\n\r", diceroll);
         send_to_char(buf, better);
         GET_GOLD(better) += 2*better->bet.come;
         REMOVE_BIT(better->bet_opt.craps_options, COME_OUT);
         better->bet.come = 0;
        }
        if (IS_SET(better->bet_opt.craps_options, CRAP_OUT)) {
         sprintf(buf, "The point [%d] was hit. You lose.\n\r", diceroll);
         send_to_char(buf, better);
         REMOVE_BIT(better->bet_opt.craps_options, CRAP_OUT);
         better->bet.crap = 0;
         if (better==ch)
          lose_dice(ch);
        }
      }
   }
   ch->point_roll = 0;
}

void roll_dice(struct char_data *ch)
{

  int die_one, die_two, dice_roll;
  char buf[255]; 
  struct char_data *better, *temp, *table_man;

  if (!check_for_dice_held(ch)) {
     send_to_char("You dont have control of the dice!\n\r", ch);
     return;
  }


  table_man = FindMobInRoomWithFunction(ch->in_room, craps_table_man);
   if (table_man) {
     if (!can_bet_craps) {
       sprintf(buf, "%s You can't roll until I say so!", GET_NAME(ch));
       do_tell(table_man, buf, 0);
       return;
     }
     if (ch->bet.come == 0) {
       sprintf(buf, "%s Sorry to keep the table, you need to place a come bet.",
                GET_NAME(ch));
       do_tell(table_man, buf, 0);
       return;
     }
   }

   die_one = dice(1,6);
   die_two = dice(1,6);
   dice_roll = (die_one + die_two);

  if (die_one == 1)
   send_to_room(ONE, ch->in_room);
  else if (die_one == 2)
   send_to_room(TWO, ch->in_room);
  else if (die_one == 3)
   send_to_room(THREE, ch->in_room);
  else if (die_one == 4)
   send_to_room(FOUR, ch->in_room);
  else if (die_one == 5)
   send_to_room(FIVE, ch->in_room);
  else if (die_one == 6)
   send_to_room(SIX, ch->in_room);

  if (die_two == 1)
   send_to_room(ONE, ch->in_room);
  else if (die_two == 2)
   send_to_room(TWO, ch->in_room);
  else if (die_two == 3)
   send_to_room(THREE, ch->in_room);
  else if (die_two == 4)
   send_to_room(FOUR, ch->in_room);
  else if (die_two == 5)
   send_to_room(FIVE, ch->in_room);
  else if (die_two == 6)
   send_to_room(SIX, ch->in_room);


         sprintf(buf, "%s rolled a %d and a %d. Total = %d\n\r", 
                     GET_NAME(ch), die_one, die_two, dice_roll);
         send_to_room(buf,ch->in_room);


  check_craps(ch, dice_roll);
  check_seven(ch, dice_roll);
  check_eleven(ch, dice_roll);
  check_onerolls(ch, dice_roll);

  if (ch->point_roll == 0) {
    set_point(ch, dice_roll);
  } else if (ch->point_roll == dice_roll) { 
    WinLoseCraps(ch, dice_roll);
  }
}
 
    
  

void spin_slot(struct char_data *ch)
{
  char buf[255];
  char *fruit1;
  char *fruit2;
  char *fruit3;
  struct obj_data *coins, *slot;
  int bits;
  struct char_data *tmp_char;

       send_to_char("You stick your coins in the machine.\n\r", ch); 
       send_to_char("You pull the arm of the slot machine.\n\r", ch);

  fruit1 = ChooseFirstFruit();

 if (strcmp(fruit1, "Seven")) {
   if (number(1,15)==1) {
    fruit2 = fruit1;
    fruit3 = fruit1;
   } else {
    fruit2 = ChooseSecondFruit();
    fruit3 = ChooseThirdFruit();
   }
  } else {
    fruit2 = ChooseSecondFruit();
    fruit3 = ChooseThirdFruit();
  }

  sprintf(buf, "%-10s %-10s %-10s", fruit1, fruit2, fruit3);
  strcat(buf, "\n\r");
  send_to_char(buf, ch);
  sprintf(buf, "$n spins a [%-10s %-10s %-10s]", fruit1, fruit2, fruit3);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);

  bits = generic_find("slot", FIND_OBJ_ROOM, ch, &tmp_char, &slot);


  if (!strcmp(fruit1, "cherry")) {
   if (strcmp(fruit1,fruit2)) {
     send_to_char("You win!\n\r", ch);
     coins = create_money(2*(ch->bet.slot));
     if (coins && (bits==FIND_OBJ_ROOM)) {
       obj_to_obj(coins, slot);
       ch->bet.slot = 0;
       return;
     } else {
       sprintf(buf, "No slot machine in room %d", ch->in_room);
       vlog(buf);
       return;
     }
   } else {
     send_to_char("You win!\n\r", ch);
     coins = create_money(4*(ch->bet.slot));
     if (coins && (bits==FIND_OBJ_ROOM)) {
       obj_to_obj(coins, slot);
       ch->bet.slot = 0;
       return;
     } else {
       sprintf(buf, "No slot machine in room %d", ch->in_room);
       vlog(buf);
       return;
     }
   }
  } else if (strcmp(fruit1, "Seven") && 
            (fruit1 == fruit2) && (fruit2 == fruit3))  {
     send_to_char("You win!\n\r", ch);
     coins = create_money(9*(ch->bet.slot));
     if (coins && (bits==FIND_OBJ_ROOM)) {
       obj_to_obj(coins, slot);
       ch->bet.slot = 0;
       return;
     } else {
       sprintf(buf, "No slot machine in room %d", ch->in_room);
       vlog(buf);
       return;
     }
  } else if (!strcmp(fruit3, "bally") && (fruit2 == fruit1)) {
     send_to_char("You win!\n\r", ch);
     coins = create_money(20*(ch->bet.slot));
     if (coins && (bits==FIND_OBJ_ROOM)) {
       obj_to_obj(coins, slot);
       ch->bet.slot = 0;
       return;
     } else {
       sprintf(buf, "No slot machine in room %d", ch->in_room);
       vlog(buf);
       return;
     }
   } else {
     send_to_char("You lose!\n\r", ch);
     ch->bet.slot = 0;
     return;
   }
}


#define DICE 9002

int craps_table_man(struct char_data *ch, int cmd, char *arg) 
{
   struct char_data *crap_man;
   char buf[255], amount[255], options[255];
   char dice[255];
   int bits;
 
  if (cmd) {
    if ((cmd != 0) &&
        (cmd != 1) &&
        (cmd != 2) &&
        (cmd != 3) &&
        (cmd != 4) &&
        (cmd != 5) &&
        (cmd != 274) &&
        (cmd != 280)) {
       return(FALSE);
     }

   crap_man = FindMobInRoomWithFunction(ch->in_room, craps_table_man);

   if (!crap_man) return(FALSE);

   if (cmd == 274) {
    if (!*arg) return(FALSE);
     half_chop(arg, options, amount);
       if (check_pointroll(ch)) {
        if (is_abbrev(options, "come") ||
            is_abbrev(options, "craps")) {
          sprintf(buf,"Sorry %s, no bets can be placed on the come", 
                  GET_NAME(ch));
          do_say(crap_man,buf, 0);
          do_say(crap_man, "or no-pass after a pointroll has been called.", 0); 
          return(TRUE);
        } else
        return(FALSE);
      } else
      return(FALSE);  
    }
        
   if (cmd == 280) {
    if (!*arg) return(FALSE);
     one_argument(arg, dice);
      if (is_abbrev(dice, "dice")) {
         roll_dice(ch);
         return(TRUE);
      }
    } else {
    if (check_for_dice_held(ch)) {
      send_to_char("You cant leave the table with the dice!\n\r", ch);
      return(TRUE);
    }
    clear_bets(ch);
    return(FALSE);
    }
  } else if (check_for_dice_in_inv(ch)) {
      if (number(1,9)==1) {
       do_say(ch, "Who wants to roll the dice next?", 0);
       return(TRUE);
      }
  } else {
    if (!ch->act_ptr)
     ch->act_ptr = (int *) calloc(1, sizeof(int));

     switch ((*((int *) ch->act_ptr))) {

       case START_BETS:
          do_say(ch,"Place all bets now!", 0);
          (*((int *) ch->act_ptr))--;
          break;
       case 9: case 8: case 7:
          (*((int *) ch->act_ptr))--;  
          break;
       case SECOND_CALL:
          do_say(ch,"Place all bets now!", 0);
          (*((int *) ch->act_ptr))--;
          break;
       case 5: case 4: case 3:
          (*((int *) ch->act_ptr))--; 
          break;
       case LAST_CALL:
          do_say(ch,"Last call for bets!", 0);
          (*((int *) ch->act_ptr))--;
          break;
       case 1:
          do_say(ch,"Ok roller, roll at your will", 0);
          (*((int *) ch->act_ptr))--;
          break;  
       default:
          break;
     }
  }
}


