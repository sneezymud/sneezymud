/* Games.h  :  Stuff to include for the casino. Russ Russell  02/18/93 */

#define BET_OPTIONS          \
"\n\rSyntax :  bet <option> <amount> \n\r\n\r\
Options :\n\r\n\r\
1)  The craps table :\n\r\
    a) come : Bet an amount on the come out roll of the roller. \n\r\
    b) crap : Bet on the no pass.\n\r\
    c) OneRolls :\n\r\
       Type bet one to see the various one roll bets available to you.\n\r\
2)  The slot machines :\n\r\
      The correct syntax for the slots machines is play slots <option>\n\r\
      Type play slots to see the different options.\n\r"
       
#define R_TABLE         \
"\n\r                  (()()()()()()())\n\r\
                  |    |    |    |\n\r\
__________________|____|____|____|\n\r\
| 1 - 18 |        | 01 | 02 | 03 |\n\r\
|________|01-->12 | 04 | 05 | 06 |\n\r\
|  Even  |        | 07 | 08 | 09 |\n\r\
|________|________| 10 | 11 | 12 |\n\r\
|        |        | 13 | 14 | 15 |\n\r\
|________|13-->24 | 16 | 17 | 18 |\n\r\
|        |        | 19 | 20 | 21 |\n\r\
|________|________| 22 | 23 | 24 |\n\r\
|  Odd   |        | 25 | 26 | 27 |\n\r\
|________|25-->36 | 28 | 29 | 30 |\n\r\
| 19 -36 |        | 31 | 32 | 33 |\n\r\
|________|________| 34 | 35 | 36 |\n\r\
                  |====|====|====|\n\r\
                  |    |    |    |\n\r\
                  |    |    |    |\n\r\
                  (()()()()()()())\n\r" 

#define CRAPS_OPTIONS        \
"\n\rCraps table options : \n\r\
1)    Bet on the come out roll.\n\r\
2)    Bet against the come out roll.\n\r\
In order to place a bet, you must first use the bet command to determine\n\r\
the amount of the bets you will be making.Unfortunately, all your bets must\n\r\
remain constant throughout the dice roll. You can always bet on new things,\n\r\
but your bet must be the same. You can change the amount you are betting \n\r\
only after a point roll has been changed.\n\r\n\r\
Type help craps for more help on this. It is most probably confusing at\n\r\
first, but it works just like a real craps table.\n\r"

#define ONEROLL_OPTIONS      \
"\n\rOne Roll bets : \n\r\
Two   : Bet on a one roll snake eyes (2). Pays 30 to 1.\n\r\
Three : Bet on a one roll acey deucy (3). Pays 15 to 1.\n\r\
Eleven: Bet on a one roll eleven (11). Pays 15 to 1.\n\r\
Twelve: Bet on a one roll box-cars (12). Pay 30 to 1.\n\r\
Craps : Bet on a one roll craps roll(2,3, or 12). Pays 7 to 1.\n\r\
Seven : Bet on a one roll seven roll. Pays 4 to 1.\n\r\
Horn  : Bet on a one roll horn bet. Type help horn for help on the horn bet.\n\r\
Field : Bet on a one roll field bet. Type help field for help on this bet.\n\r"


/* craps_options */

#define COME_OUT     1
#define CRAP_OUT     2

/* One roll bets, and bets that can be laid anytime. */

#define ELEVEN       1
#define TWELVE       2
#define HARD_EIGHT   4
#define HARD_TEN     8
#define HARD_SIX     16
#define HARD_FOUR    32 
#define CRAPS        64
#define HORN_BET     128
#define FIELD_BET    256
#define TWO2         512
#define THREE3       1024
#define SEVEN        2048

/* For the act_ptrs that control when you can throw dice */

#define START_BETS   10
#define SECOND_CALL   6
#define LAST_CALL     2
 
 
 
#define ONE \
" #########\n\r\
 #       #\n\r\
 #   *   #\n\r\
 #       #\n\r\
 #########\n\r\n\r"
 
#define TWO \
" #########\n\r\
 #     * #\n\r\
 #       #\n\r\
 # *     #\n\r\
 #########\n\r\n\r"
 
#define THREE \
" #########\n\r\
 #     * #\n\r\
 #   *   #\n\r\
 # *     #\n\r\
 #########\n\r\n\r"
 
#define FOUR \
" #########\n\r\
 # *   * #\n\r\
 #       #\n\r\
 # *   * #\n\r\
 #########\n\r\n\r"
 
#define FIVE \
" #########\n\r\
 # *   * #\n\r\
 #   *   #\n\r\
 # *   * #\n\r\
 #########\n\r\n\r"
 
#define SIX \
" #########\n\r\
 # *   * #\n\r\
 # *   * #\n\r\
 # *   * #\n\r\
 #########\n\r\n\r"


