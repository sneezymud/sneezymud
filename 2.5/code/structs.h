/* ************************************************************************
*  file: structs.h , Structures        .                  Part of DIKUMUD *
*  Usage: Declarations of central data structures                         *
************************************************************************* */

#include <sys/types.h>

typedef signed char sbyte;
typedef unsigned char ubyte;
typedef short int sh_int;
typedef unsigned short int ush_int;
typedef char bool;
typedef signed char byte;

/*  
   my new stuff
*/

#define PULSE_COMMAND 0
#define PULSE_TICK 1

#define MAX_STAT 5

#define BIT_POOF_IN  1
#define BIT_POOF_OUT  2
/*
  Quest stuff
*/


struct QuestItem {
  int item;
  char *where;
};

/*
  tailoring stuff
*/
#define LIMITED_ITEMS 1 
#define PLAYER_AUTH   0 
#define SITELOCK      1

/*
 efficiency stuff
*/
#define MIN_GLOB_TRACK_LEV 31   /* mininum level for global track */
/*
**  Site locking stuff.. written by Scot Gardner
*/
#define MAX_BAN_HOSTS 15

/*
**  Newbie authorization stuff
*/

#define NEWBIE_REQUEST 1
#define NEWBIE_START   100
#define NEWBIE_AXE     0
#define NEWBIE_CHANCES 3

/*
**  Limited item Stuff
*/

#define LIM_ITEM_COST_MIN  199999   /* mininum rent cost of a lim. item */

/*
**  distributed monster stuff
*/

#define TICK_WRAP_COUNT 3   /*  PULSE_MOBILE / PULSE_TELEPORT */
                            /*
			      Note:  This stuff is all code dependent,
			      Don't change it unless you know what you
			      are doing.  comm.c and mobact.c hold the
	                      stuff that you will HAVE to rewrite if you
			      change either of those constants.
                            */
#define PLR_TICK_WRAP   24  /*  this should be a divisor of 24 (hours) */


/* casino stuff */

struct bet_data {
  long come;
  long crap;
  long slot;
  long eleven;
  long twelve;
  long two;
  long three;
  long horn_bet;
  long field_bet;
  long hard_eight;
  long hard_six;
  long hard_ten;
  long hard_four;
  long seven;
  long one_craps;
};

struct char_bet_data {
  long craps_options;
  long one_roll;
  int roul_options;
};

/*
**  multiclassing stuff
*/

#define MAGE_LEVEL_IND        0
#define CLERIC_LEVEL_IND      1
#define WARRIOR_LEVEL_IND     2
#define THIEF_LEVEL_IND       3
#define ANTIPALADIN_LEVEL_IND 4
#define PALADIN_LEVEL_IND     5
#define MONK_LEVEL_IND        6
#define RANGER_LEVEL_IND      7


#define FIRE_DAMAGE 1
#define COLD_DAMAGE 2
#define ELEC_DAMAGE 3
#define BLOW_DAMAGE 4
#define ACID_DAMAGE 5

#define HATE_SEX   1
#define HATE_RACE  2
#define HATE_CHAR  4
#define HATE_CLASS 8
#define HATE_EVIL  16
#define HATE_GOOD  32
#define HATE_VNUM  64

#define FEAR_SEX   1
#define FEAR_RACE  2
#define FEAR_CHAR  4
#define FEAR_CLASS 8
#define FEAR_EVIL  16
#define FEAR_GOOD  32
#define FEAR_VNUM  64

#define OP_SEX   1
#define OP_RACE  2
#define OP_CHAR  3
#define OP_CLASS 4
#define OP_EVIL  5
#define OP_GOOD  6
#define OP_VNUM  7

#define ABS_MAX_LVL  70
#define MAX_MORT     50
#define LOW_IMMORTAL 51
#define IMMORTAL     51
#define CREATOR      52
#define SAINT        53
#define DEMIGOD      54
#define LESSER_GOD   55
#define GOD          56
#define GREATER_GOD  57
#define SILLYLORD    58
#define IMPLEMENTOR  59
#define BRUTIUS      60
#define MAX_IMMORT   60

#define IMM_FIRE        1
#define IMM_COLD        2
#define IMM_ELEC        4
#define IMM_ENERGY      8
#define IMM_BLUNT      16
#define IMM_PIERCE     32
#define IMM_SLASH      64
#define IMM_ACID      128
#define IMM_POISON    256
#define IMM_DRAIN     512
#define IMM_SLEEP    1024
#define IMM_CHARM    2048
#define IMM_HOLD     4096
#define IMM_NONMAG   8192
#define IMM_PLUS1   16384
#define IMM_PLUS2   32768
#define IMM_PLUS3   65536
#define IMM_PLUS4  131072

#define PULSE_RIVER    15
#define PULSE_TELEPORT      10

#define MAX_ROOMS   5000

struct nodes
{
  int visited;
  int ancestor;
};

struct room_q
{
  int room_nr;
  struct room_q *next_q;
};

struct string_block {
  int	size;
  char	*data;
};


/*
  memory stuff 
*/

struct char_list {
  struct char_data *op_ch;
  char name[50];
  struct char_list *next;
};

typedef struct {
       struct char_list  *clist;
       int    sex;   /*number 1=male,2=female,3=both,4=neut,5=m&n,6=f&n,7=all*/
       int    race;  /*number */
       int    class; /* 1=m,2=c,4=f,8=t */
       int    vnum;  /* # */
       int    evil;  /* align < evil = attack */
       int    good;  /* align > good = attack */
}  Opinion;




/*
   old stuff.
*/

#define PULSE_ZONE     240
#define PULSE_MOBILE    60
#define PULSE_VIOLENCE  12
#define WAIT_SEC       4
#define WAIT_ROUND     4

#define MAX_STRING_LENGTH   4096
#define MAX_INPUT_LENGTH     160
#define MAX_MESSAGES          60
#define MAX_ITEMS            153

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR  75
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

/* The following defs are for obj_data  */

/* For 'type_flag' */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION    10 
#define ITEM_WORN      11
#define ITEM_OTHER     12
#define ITEM_TRASH     13
#define ITEM_TRAP      14
#define ITEM_CONTAINER 15
#define ITEM_NOTE      16
#define ITEM_DRINKCON  17
#define ITEM_KEY       18
#define ITEM_FOOD      19
#define ITEM_MONEY     20
#define ITEM_PEN       21
#define ITEM_BOAT      22
#define ITEM_AUDIO     23
#define ITEM_BOARD     24
#define ITEM_BOW       25
#define ITEM_ARROW     26
#define ITEM_RADIO     27
#define ITEM_CORPSE    28
#define ITEM_SPELLBAG  29
#define ITEM_COMPONENT 30
#define ITEM_BOOK      31

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE              1 
#define ITEM_WEAR_FINGER       2
#define ITEM_WEAR_NECK         4
#define ITEM_WEAR_BODY         8
#define ITEM_WEAR_HEAD        16
#define ITEM_WEAR_LEGS        32
#define ITEM_WEAR_FEET        64
#define ITEM_WEAR_HANDS      128 
#define ITEM_WEAR_ARMS       256
#define ITEM_WEAR_SHIELD     512
#define ITEM_WEAR_ABOUT     1024 
#define ITEM_WEAR_WAISTE    2048
#define ITEM_WEAR_WRIST     4096
#define ITEM_WIELD          8192
#define ITEM_HOLD          16384
#define ITEM_THROW         32768
/* UNUSED, CHECKS ONLY FOR ITEM_LIGHT #define ITEM_LIGHT_SOURCE  65536 */
#define ITEM_WEAR_EAR     131072
#define ITEM_WEAR_FACE         262144
#define ITEM_WORN_AS_RADIO 524288

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW            1
#define ITEM_HUM             2
#define ITEM_LEVEL15         4  /* undefined...  */
#define ITEM_LEVEL25         8  /* undefined?    */
#define ITEM_LEVEL35        16  /* undefined?    */
#define ITEM_INVISIBLE      32
#define ITEM_MAGIC          64
#define ITEM_NODROP        128
#define ITEM_BLESS         256
#define ITEM_ANTI_GOOD     512 /* not usable by good people    */
#define ITEM_ANTI_EVIL    1024 /* not usable by evil people    */
#define ITEM_ANTI_NEUTRAL 2048 /* not usable by neutral people */
#define ITEM_ANTI_CLERIC  4096
#define ITEM_ANTI_MAGE    8192
#define ITEM_ANTI_THIEF   16384
#define ITEM_ANTI_FIGHTER 32768
#define ITEM_BRITTLE      65536 /* weapons that break after 1 hit */
                                /* armor that breaks when hit?    */
#define ITEM_LEVEL10      131072 /*cant be worn by levels < 10 */
#define ITEM_LEVEL20      262144 /*cant be worn by levels < 20 */
#define ITEM_LEVEL30      524288 /*Cant be worn by levels < 30 */
#define ITEM_ANTI_ANTI    1048576
#define ITEM_ANTI_PALA    2097152
#define ITEM_ANTI_RANGER  4194304
#define ITEM_ANTI_MONK    8388608
#define ITEM_LEVEL40     16777216
#define ITEM_HOLDING     33554432

/* Some different kind of liquids */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_COKE       15

/* special addition for drinks */
#define DRINK_POISON  (1<<0)
#define DRINK_PERM    (1<<1)


/* for containers  - value[1] */

#define CONT_CLOSEABLE      1
#define CONT_PICKPROOF      2
#define CONT_CLOSED         4
#define CONT_LOCKED         8



struct extra_descr_data
{
	char *keyword;                 /* Keyword in look/examine          */
	char *description;             /* What to see                      */
	struct extra_descr_data *next; /* Next in list                     */
};

#define MAX_OBJ_AFFECT 5         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OBJ_NOTIMER    -7000000

struct obj_flag_data
{
	int value[4];       /* Values of the item (see list)    */
	byte type_flag;     /* Type of item                     */
	int wear_flags;     /* Where you can wear it            */
	long extra_flags;    /* If it hums,glows etc             */
	int weight;         /* Weigt what else                  */
	int cost;           /* Value when sold (gp.)            */
	int cost_per_day;   /* Cost to keep pr. real day        */
	int timer;          /* Timer for object                 */
	long bitvector;     /* To set chars bits                */
        int decay_time;
        int struct_points;
        int max_struct_points;
        ubyte material_points;
        int volume;
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type {
	short location;      /* Which ability to change (APPLY_XXX) */
	unsigned long modifier;      /* How much it changes by              */
};

/* ======================== Structure for object ========================= */
struct obj_data
{
	sh_int item_number;            /* Where in data-base               */
	int in_room;                /* In what room -1 when conta/carr  */ 
	struct obj_flag_data obj_flags;/* Object information               */
	struct obj_affected_type
	    affected[MAX_OBJ_AFFECT];  /* Which abilities in PC to change  */

	struct char_data *killer;      /* for use with corpses */
        sh_int char_vnum;              /* for ressurection     */
	long   char_f_pos;             /* for ressurection     */
	char *name;                    /* Title of object :get etc.        */
	char *description ;            /* When in room                     */
	char *short_description;       /* when worn/carry/in cont.         */
 	char *action_description;      /* What to write when used          */
 	struct extra_descr_data *ex_description; /* extra descriptions     */
	struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
	byte   eq_pos;                 /* what is the equip. pos?          */
	struct char_data *equipped_by; /* equipped by :NULL in room/conta  */

	struct obj_data *in_obj;       /* In what object NULL when none    */
	struct obj_data *contains;     /* Contains objects                 */

	struct obj_data *next_content; /* For 'contains' lists             */
	struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= */

/* The following defs are for room_data  */

#define NOWHERE    -1    /* nil reference for room-database      */
#define AUTO_RENT  -2    /* other special room, for auto-renting */

/* Bitvector For 'room_flags' */

#define DARK           1
#define DEATH          2
#define NO_MOB         4
#define INDOORS        8
#define PEACEFUL      16  /* No fighting */
#define NOSTEAL       32  /* No Thieving */
#define NO_SUM        64  /* no summoning */
#define NO_MAGIC     128
#define TUNNEL       256 /* ? */
#define PRIVATE      512
#define SILENCE      1024
#define NO_ORDER     2048
#define ANARCHY      4096
#define HAVE_TO_WALK 8192
#define ARENA        16384
#define NO_HEAL      32768
#define HOSPITAL     65536

/* For 'dir_option' */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR      	1
#define EX_CLOSED      	2
#define EX_LOCKED      	4
#define EX_SECRET	8
#define EX_RSLOCKED	16
#define EX_PICKPROOF    32

/* For 'Sector types' */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_AIR             8
#define SECT_UNDERWATER      9
#define SECT_DESERT          10

struct room_direction_data
{
	char *general_description;       /* When look DIR.                  */ 
	char *keyword;                   /* for open/close                  */	
	sh_int exit_info;                /* Exit info                       */
	int key;		                   /* Key's number (-1 for no key)    */
	int to_room;                  /* Where direction leeds (NOWHERE) */
};

/* ========================= Structure for room ========================== */
struct room_data
{
	sh_int number;               /* Rooms number                       */
	sh_int zone;                 /* Room zone (for resetting)          */
	int sector_type;             /* sector type (move/hide)            */

        int river_dir;               /* dir of flow on river               */
        int river_speed;             /* speed of flow on river             */

	int  tele_time;              /* time to a teleport                 */
	int  tele_targ;              /* target room of a teleport          */
	char tele_look;              /* do a do_look or not when 
  					teleported                         */
        unsigned char moblim;        /* # of mobs allowed in room.         */

	char *name;                  /* Rooms name 'You are ...'           */
	char *description;           /* Shown when entered                 */
	struct extra_descr_data *ex_description; /* for examine/look       */
	struct room_direction_data *dir_option[6]; /* Directions           */
	long room_flags;             /* DEATH,DARK ... etc                 */ 
	byte light;                  /* Number of lightsources in room     */
	int (*funct)();              /* special procedure                  */
         
	struct obj_data *contents;   /* List of items in room              */
	struct char_data *people;    /* List of NPC / PC in room           */
};
/* ======================================================================== */

/* The following defs and structures are related to char_data   */

/* For 'equipment' */

#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAISTE    13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WIELD          16
#define HOLD           17
#define WEAR_EAR       18
#define WEAR_FACE      19
#define WEAR_RADIO     20


/* For 'char_payer_data' */


/*
**  #2 has been used!!!!  Don't try using the last of the 3, because it is
**  the keeper of active/inactive status for dead characters for ressurection!
*/
#define MAX_TOUNGE  3     /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */


#define MAX_SKILLS  200   /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_WEAR    25
#define MAX_AFFECT  25    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

/* Predifined  conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2

/* Bitvector for 'affected_by' */
#define AFF_BLIND             0x000000001
#define AFF_INVISIBLE         0x000000002
#define AFF_DETECT_EVIL       0x000000004
#define AFF_DETECT_INVISIBLE  0x000000008
#define AFF_DETECT_MAGIC      0x000000010
#define AFF_SENSE_LIFE        0x000000020
#define AFF_LIFE_PROT         0x000000040
#define AFF_SANCTUARY         0x000000080
#define AFF_GROUP             0x000000100

/* there is one missing here....... */
#define AFF_CURSE             0x000000400
#define AFF_FLYING            0x000000800
#define AFF_POISON            0x000001000
#define AFF_PROTECT_EVIL      0x000002000
#define AFF_PARALYSIS         0x000004000
#define AFF_INFRAVISION       0x000008000
#define AFF_WATERBREATH       0x000010000
#define AFF_SLEEP             0x000020000
#define AFF_KILLABLE          0x000040000
#define AFF_SNEAK             0x000080000
#define AFF_HIDE              0x000100000
#define AFF_PROTECT_FROM_GOOD 0x000200000
#define AFF_CHARM             0x000400000
#define AFF_FOLLOW            0x000800000
#define AFF_UNDEF_1           0x001000000  /* saved objects?? */
#define AFF_TRUE_SIGHT        0x002000000
#define AFF_BREWING           0x004000000
#define AFF_FIRESHIELD        0x008000000
#define AFF_SILENT            0x010000000
#define AFF_GRAPPLE           0x020000000
#define AFF_GRAPPLE2          0x040000000
#define AFF_SCRYING           0x080000000

/* modifiers to char's abilities */

#define APPLY_NONE              0
#define APPLY_STR               1
#define APPLY_DEX               2
#define APPLY_INT               3
#define APPLY_WIS               4
#define APPLY_CON               5
#define APPLY_SEX               6
#define APPLY_CLASS             7
#define APPLY_LEVEL             8
#define APPLY_AGE               9
#define APPLY_CHAR_WEIGHT      10
#define APPLY_CHAR_HEIGHT      11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_GOLD             15
#define APPLY_EXP              16
#define APPLY_AC               17
#define APPLY_ARMOR            17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PARA      20
#define APPLY_SAVING_ROD       21
#define APPLY_SAVING_PETRI     22
#define APPLY_SAVING_BREATH    23
#define APPLY_SAVING_SPELL     24
#define APPLY_SAVE_ALL         25
#define APPLY_IMMUNE           26
#define APPLY_SUSC             27
#define APPLY_M_IMMUNE         28
#define APPLY_SPELL            29
#define APPLY_WEAPON_SPELL     30
#define APPLY_EAT_SPELL        31
#define APPLY_BACKSTAB         32
#define APPLY_KICK             33
#define APPLY_SNEAK            34
#define APPLY_HIDE             35
#define APPLY_BASH             36
#define APPLY_PICK             37
#define APPLY_STEAL            38
#define APPLY_TRACK            39
#define APPLY_HITNDAM          40
#define APPLY_DOUBLE_ATTACK    41
#define APPLY_DEATHSTROKE      42
#define APPLY_PARRY            43
#define APPLY_THROW            44
#define APPLY_GRAPPLE          45

/* 'class' for PC's */
#define CLASS_MAGIC_USER   1
#define CLASS_CLERIC       2
#define CLASS_WARRIOR      4
#define CLASS_THIEF        8
#define CLASS_ANTIPALADIN 16
#define CLASS_PALADIN     32
#define CLASS_MONK        64
#define CLASS_RANGER     128
/* sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */
#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_INCAP      2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8


/* for mobile actions: specials.act */
#define ACT_SPEC       (1<<0)  /* special routine to be called if exist   */
#define ACT_SENTINEL   (1<<1)  /* this mobile not to be moved             */
#define ACT_SCAVENGER  (1<<2)  /* pick up stuff lying around              */
#define ACT_ISNPC      (1<<3)  /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF (1<<4)  /* Set if a thief should NOT be killed     */
#define ACT_AGGRESSIVE (1<<5)  /* Set if automatic attack on NPC's        */
#define ACT_STAY_ZONE  (1<<6)  /* MOB Must stay inside its own zone       */
#define ACT_WIMPY      (1<<7)  /* MOB Will flee when injured, and if      */
                               /* aggressive only attack sleeping players */
#define ACT_ANNOYING   (1<<8)  /* MOB is so utterly irritating that other */
                               /* monsters will attack it...              */
#define ACT_HATEFUL    (1<<9)  /* MOB will attack a PC or NPC matching a  */
                               /* specified name                          */
#define ACT_AFRAID    (1<<10)  /* MOB is afraid of a certain PC or NPC,   */
                               /* and will always run away ....           */
#define ACT_IMMORTAL  (1<<11)  /* MOB is a natural event, can't be kiled  */
#define ACT_HUNTING   (1<<12)  /* MOB is hunting someone                  */
#define ACT_DEADLY    (1<<13)  /* MOB has deadly poison                   */
#define ACT_POLYSELF  (1<<14)  /* MOB is a polymorphed person             */
#define ACT_META_AGG  (1<<15)  /* MOB is _very_ aggressive                */
#define ACT_GUARDIAN  (1<<16)  /* MOB will guard master                   */

/* For players : specials.act */
#define PLR_BRIEF     (1<<0)
#define PLR_COMPACT   (1<<1)
#define PLR_WIMPY     (1<<2) /* character will flee when seriously injured */
#define PLR_DONTSET   (1<<3)
#define PLR_NOHASSLE  (1<<4) /* char won't be attacked by aggressives.      */
#define PLR_STEALTH   (1<<5) /* char won't be announced in a variety of situations */
#define PLR_HUNTING   (1<<6) /* the player is hunting someone, do a track each look */
#define PLR_MAILING   (1<<7)
#define PLR_LOGGED    (1<<8)
#define PLR_KILLER    (1<<9)
#define PLR_VT100     (1<<10) /* VT100 capable */
#define PLR_COLOR     (1<<11)
#define PLR_OUTLAW    (1<<12)
#define PLR_ANSI      (1<<13)
#define PLR_NOSHOUT   (1<<14)/* the player is not allowed to shout */
#define PLR_BANISHED  (1<<15)/*The players goes to hell on login*/



/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data
{
	byte hours, day, month;
	sh_int year;
};

/* These data contain information about a players time data */
struct time_data
{
  time_t birth;    /* This represents the characters age                */
  time_t logon;    /* Time of the last logon (used to calculate played) */
  int played;      /* This is the total accumulated time played in secs */
};

struct char_player_data
{
	char *name;    	    /* PC / NPC s name (kill ...  )         */
	char *short_descr;  /* for 'actions'                        */
	char *long_descr;   /* for 'look'.. Only here for testing   */
	char *description;  /* Extra descriptions                   */
	char *title;        /* PC / NPC s title                     */
	char *sounds;       /* Sound that the monster makes (in room) */
	char *distant_snds; /* Sound that the monster makes (other) */
	byte sex;           /* PC / NPC s sex                       */
	unsigned char class;         /* PC s class or NPC alignment          */
	byte level[8];         /* PC / NPC s level                     */
	int hometown;       /* PC s Hometown (zone)                 */
	bool talks[MAX_TOUNGE]; /* PC s Tounges 0 for NPC           */
 	struct time_data time; /* PC s AGE in days                 */
	ubyte weight;       /* PC / NPC s weight                    */
	ubyte height;       /* PC / NPC s height                    */
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data
{
	sbyte str; 
	sbyte str_add;      /* 000 - 100 if strength 18             */
	sbyte intel;
	sbyte wis; 
	sbyte dex; 
	sbyte con; 
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data
{

        sh_int mana;         
        sh_int max_mana;

	sh_int hit;   
	sh_int max_hit;      /* Max hit for NPC                         */
	sh_int move;  
	sh_int max_move;     /* Max move for NPC                        */

	sh_int armor;        /* Internal -100..100, external -10..10 AC */
	int gold;            /* Money carried                           */
        int bankgold;        /* gold in the bank.                       */
	int exp;             /* The experience of the player            */

	sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
	sbyte damroll;       /* Any bonus or penalty to the damage roll */
};

struct char_poofin_data
{
      char *poofin;
      char *poofout; 
      int pmask;
};

struct char_special_data
{
        int zone;   /* zone that an NPC lives in */
	struct char_data *fighting; /* Opponent                             */
	
	struct char_data *hunting;  /* Hunting person..                     */

	long affected_by;  /* Bitvector for spells/skills affected by */ 

        byte tick;             /* the tick that the mob/player is on  */

	byte position;           /* Standing or ...                         */
	byte default_pos;        /* Default position for NPC                */
	unsigned long act;      /* flags for NPC behavior                  */

	unsigned char spells_to_learn;    /* How many can you learn yet this level   */

	int carry_weight;        /* Carried weight                          */
	int carry_items;        /* Number of items carried                 */
	int timer;               /* Timer for update                        */
	int was_in_room;      /* storage of location for linkdead people */
	sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)             */
	sbyte conditions[3];      /* Drunk full etc.                        */

	byte damnodice;           /* The number of damage dice's            */
	byte damsizedice;         /* The size of the damage dice's          */
	byte last_direction;      /* The last direction the monster went    */
	int attack_type;          /* The Attack Type Bitvector for NPC's    */
	int alignment;            /* +-1000 for alignments                  */
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data
{
	byte learned;           /* % chance for success 0 = not learned   */
	bool recognise;         /* If you can recognise the scroll etc.   */
};



/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type
{
	short type;           /* The type of spell that caused this      */
	sh_int duration;      /* For how long its effects will last      */
	sbyte modifier;       /* This is added to apropriate ability     */
	byte location;        /* Tells which ability to change(APPLY_XXX)*/
	long bitvector;       /* Tells which bits to set (AFF_XXX)       */

	struct affected_type *next;
};

struct follow_type
{
	struct char_data *follower;
	struct follow_type *next;
};

/* ================== Structure for player/non-player ===================== */
struct char_data
{
	sh_int nr;                            /* monster nr (pos in file)    */
	int in_room;                       /* Location                    */
/* 
  will need several new affects

  maybe these should go in points?  (and thusly be saved and restored?)

*/

	unsigned immune;                      /* Immunities                  */
	unsigned M_immune;                    /* Meta Immunities             */
        unsigned susc;                        /* susceptibilities            */
	float mult_att;                      /* the number of attacks       */
	byte   attackers;

	sh_int fallspeed;                     /* rate of descent for player */
	sh_int race;
	sh_int hunt_dist;                    /* max dist the player can hunt */

        unsigned short hatefield;
        unsigned short fearfield;

	Opinion hates;
	Opinion fears;

	sh_int  persist;
	int     old_room;

	void    *act_ptr;    /* numeric argument for the mobile actions */

	struct char_player_data player;       /* Normal data                 */
	struct char_ability_data abilities;   /* Abilities                   */
	struct char_ability_data tmpabilities;/* The abilities we will use   */
	struct char_point_data points;        /* Points                      */
	struct char_special_data specials;    /* Special plaing constants    */
	struct char_skill_data *skills;       /* Skills                 */

	struct affected_type *affected;       /* affected by what spells     */
	struct obj_data *equipment[MAX_WEAR]; /* Equipment array             */

	struct obj_data *carrying;            /* Head of list                */
	struct descriptor_data *desc;         /* NULL for mobiles            */
	struct char_data *orig;               /* Special for polymorph       */

	struct char_data *next_in_room;     /* For room->people - list       */
	struct char_data *next;             /* For either monster or ppl-lis */
	struct char_data *next_fighting;    /* For fighting list             */

	struct follow_type *followers;        /* List of chars followers     */
	struct char_data *master;             /* Who is char following?      */
	int	invis_level;		      /* visibility of gods */
        short  wimpy;       /* If wimpy set, max hp before autoflee */
        int point_roll;
        struct bet_data bet;
        struct char_bet_data bet_opt;
        struct char_poofin_data poof;


};


/* ======================================================================== */

/* How much light is in the land ? */

#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET		3

/* And how is the sky ? */

#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3

struct weather_data
{
	int pressure;	/* How is the pressure ( Mb ) */
	int change;	/* How fast and what way does it change. */
	int sky;	/* How is the sky. */
	int sunlight;	/* And how much sun. */
};


/* ***********************************************************************
*  file element for player file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */


struct char_file_u
{
	byte sex;
	unsigned char class;
	byte level[8];
	time_t birth;  /* Time of birth of character     */
  int played;    /* Number of secs played in total */


	int   race;
	ubyte weight;
	ubyte height;

	char title[80];
	sh_int hometown;
	char description[240];
	bool talks[MAX_TOUNGE];

	sh_int load_room;            /* Which room to place char in  */

	struct char_ability_data abilities;

	struct char_point_data points;

	struct char_skill_data skills[MAX_SKILLS];

	struct affected_type affected[MAX_AFFECT];

	/* specials */

        byte spells_to_learn;
	int alignment;

	time_t last_logon;  /* Time (in secs) of last logon */
	unsigned long act;          /* ACT Flags                    */

	/* char data */
	char name[20];
	char pwd[11];
	sh_int apply_saving_throw[5];
	int conditions[3];
};



/* ***********************************************************************
*  file element for object file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */

struct obj_cost { /* used in act.other.c:do_save as
		     well as in reception2.c */
	int total_cost;
	int no_carried;
	bool ok;
};

#define MAX_OBJ_SAVE 200 /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */

struct obj_file_elem 
{
	sh_int item_number;

	int value[4];
	int extra_flags;
	int weight;
	int timer;
	long bitvector;
	char name[128];  /* big, but not horrendously so */
	char sd[128];
        char desc[256];
	struct obj_affected_type affected[MAX_OBJ_AFFECT];
        int decay_time;
        int struct_points;
        int max_struct_points;
        ubyte material_points;
        int volume;
};

struct obj_file_u
{
	char owner[20];    /* Name of player                     */
	int gold_left;     /* Number of goldcoins left at owner  */
	int total_cost;    /* The cost for all items, per day    */
	long last_update;  /* Time in seconds, when last updated */
	long minimum_stay; /* For stasis */
	int  number;       /* number of objects */
	struct obj_file_elem objects[MAX_OBJ_SAVE];
};

#if 0

#define MAX_OBJ_SAVE 200 /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */

struct rental_header {
  char	inuse;
  int	length;
  char owner[20];    /* Name of player                     */
};

struct obj_file_elem {
	sh_int item_number;

	int value[4];
	int extra_flags;
	int weight;
	int timer;
	long bitvector;
	struct obj_affected_type affected[MAX_OBJ_AFFECT];
};

struct obj_file_u
{
	int gold_left;     /* Number of goldcoins left at owner  */
	int total_cost;    /* The cost for all items, per day    */
	long last_update;  /* Time in seconds, when last updated */
	long minimum_stay; /* For stasis */
	int nobjects;	   /* how many objects below */
	struct obj_file_elem objects[MAX_OBJ_SAVE];
			   /* We don't always allocate this much space
			      but it is handy for the times when you
			      need a fast one lying around.  */
};

#endif

/* ***********************************************************
*  The following structures are related to descriptor_data   *
*********************************************************** */



struct txt_block
{
	char *text;
	struct txt_block *next;
};

struct txt_q
{
	struct txt_block *head;
	struct txt_block *tail;
};



/* modes of connectedness */

#define CON_PLYNG   0
#define CON_NME	    1
#define CON_NMECNF  2
#define CON_PWDNRM  3
#define CON_PWDGET  4
#define CON_PWDCNF  5
#define CON_QSEX    6
#define CON_RMOTD   7
#define CON_SLCT    8
#define CON_EXDSCR  9
#define CON_QCLASS  10
#define CON_LDEAD   11
#define CON_PWDNEW  12
#define CON_PWDNCNF 13
#define CON_WIZLOCK 14
#define CON_QRACE   15
#define CON_RACPAR  16
#if PLAYER_AUTH
#define CON_AUTH    17
#endif
#define CON_CITY_CHOICE 18
#define CON_STAT_LIST   19
#define CON_VT_ANSI     20
#define CON_ANSI_NME    21
#define CON_VT_NME      22
#define CON_SCREEN_SIZE 23

struct snoop_data
{
	struct char_data *snooping;	
		/* Who is this char snooping */
	struct char_data *snoop_by;
		/* And who is snooping on this char */
};

struct descriptor_data
{
	int descriptor;	            /* file descriptor for socket */
	char host[50];                /* hostname                   */
	char pwd[12];                 /* password                   */
	int pos;                      /* position in player-file    */
	int connected;                /* mode of 'connectedness'    */
	int wait;                     /* wait for how many loops    */
	char *showstr_head;				/* for paging through texts	*/
	char *showstr_point;				/*       -                    */
	char **str;                   /* for the modify-str system  */
	int max_str;                  /* -                          */
	int prompt_mode;              /* control of prompt-printing */
	char buf[MAX_STRING_LENGTH];  /* buffer for raw input       */
	char last_input[MAX_INPUT_LENGTH];/* the last input         */
        char stat[MAX_STAT];
	struct txt_q output;          /* q of strings to send       */
	struct txt_q input;           /* q of unprocessed input     */
	struct char_data *character;  /* linked to char             */
   struct char_data *original;   /* original char              */
	struct snoop_data snoop;      /* to snoop people.	         */
	struct descriptor_data *next; /* link to next descriptor    */
	char *pagedfile;					/* what file is getting paged */
	long position;						/* where in that file 		 	*/
        char name[20];
        char *prompt;
        int screen_size;
        
        
};

struct msg_type 
{
	char *attacker_msg;  /* message to attacker */
	char *victim_msg;    /* message to victim   */
	char *room_msg;      /* message to room     */
};

struct message_type
{
	struct msg_type die_msg;      /* messages when death            */
	struct msg_type miss_msg;     /* messages when miss             */
	struct msg_type hit_msg;      /* messages when hit              */
	struct msg_type sanctuary_msg;/* messages when hit on sanctuary */
	struct msg_type god_msg;      /* messages when hit on god       */
	struct message_type *next;/* to next messages of this kind.*/
};

struct message_list
{
	int a_type;               /* Attack type 						 */
	int number_of_attacks;	  /* How many attack messages to chose from. */
	struct message_type *msg; /* List of messages.				 */
};

struct dex_skill_type
{
	sh_int p_pocket;
	sh_int p_locks;
	sh_int traps;
	sh_int sneak;
	sh_int hide;
        int volume;
};

struct dex_app_type
{
	sh_int reaction;
	sh_int miss_att;
	sh_int defensive;
};

struct str_app_type
{
	sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
	sh_int todam;    /* Damage Bonus/Penalty                */
	sh_int carry_w;  /* Maximum weight that can be carrried */
	sh_int wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type
{
	byte bonus;       /* how many bonus skills a player can */
	                  /* practice pr. level                 */
};

struct int_app_type
{
	byte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type
{
	sh_int hitp;
	sh_int shock;
};

/************************************************************/

typedef void (*funcp)();

struct breather {
  int	vnum;
  int	cost;
  funcp	*breaths;
};

typedef struct char_data Mob;
typedef struct obj_data Obj;
typedef struct room_data Room;
typedef struct descriptor_data Descriptor;

#define OBJECT_HITTING -1
