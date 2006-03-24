/* ************************************************************************
 *  file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD *
 *  Usage : Interpreter of spells                                          *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h" 
#include "spells.h"
#include "handler.h"

#define MANA_MU 1
#define MANA_CL 1

#define SPELLO(nr, beat, pos, mlev, clev, mana, alev, plev, rlev, tar, func) {      \
                spell_info[nr].spell_pointer = (func);    \
               spell_info[nr].beats = (beat);            \
               spell_info[nr].minimum_position = (pos);  \
               spell_info[nr].min_usesmana = (mana);     \
               spell_info[nr].min_level_cleric = (clev); \
               spell_info[nr].min_level_magic = (mlev);  \
               spell_info[nr].min_level_anti = (alev);   \
               spell_info[nr].min_level_pal = (plev);    \
               spell_info[nr].min_level_ranger = (rlev); \
               spell_info[nr].targets = (tar);           \
         }


/* 100 is the MAX_MANA for a character */
#define USE_MANA(ch, sn)                            \
  MAX(spell_info[sn].min_usesmana, 100/MAX(2,(2+GET_LEVEL(ch, BestMagicClass(ch))-SPELL_LEVEL(ch,sn))))

/* Global data */
extern struct index_data *obj_index;
extern struct room_data *world;
extern struct char_data *character_list;
extern char *spell_wear_off_msg[];
extern struct obj_data *object_list;
extern char *spell_wear_off_soon_msg[];
extern char *spell_wear_off_room_msg[];
extern char *spell_wear_off_soon_room_msg[];

/* Inter procedures. */

void SpellWearOffSoon(int s, struct char_data *ch);
void check_drowning(struct char_data *ch);
void check_decharm(struct char_data *ch);
void SpellWearOff(int s, struct char_data *ch);



/* Extern procedures */

char *strdup(char *str);

/* Extern procedures */
void cast_animate_dead( byte level, struct char_data *ch, char *arg, int type,
             struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_conjure_elemental( byte level, struct char_data *ch, char *arg, 
             int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_acid_blast( byte level, struct char_data *ch, char *arg, int type,
           struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_armor( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_teleport( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_bless( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blindness( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_burning_hands( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_call_lightning( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_charm_person( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_charm_monster( byte level, struct char_data *ch, char *arg, int si, 
         struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cacaodemon( byte level, struct char_data *ch, char *arg, int si, 
         struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_chill_touch( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shocking_grasp( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_clone( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_colour_spray( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_control_weather( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_food( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_water( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_blind( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_critic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_critic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_curse( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cont_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, 
           struct obj_data *tar_obj);
void cast_calm( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, 
          struct obj_data *tar_obj);
void cast_detect_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_magic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_good( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_magic( byte level, struct char_data *ch, char *arg, int type,
             struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_earthquake( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_weapon( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_energy_drain( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fear( byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_fireball( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_flamestrike( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_flying( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_flying( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_harm( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_heal( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_full_heal( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_infravision( byte level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cone_of_cold( byte level, struct char_data *ch, char *arg, int type,
             struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_ice_storm( byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_knock( byte level, struct char_data *ch, char *arg, int type,
      struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_know_alignment(byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_true_seeing(byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_minor_creation(byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_faerie_fire(byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_faerie_fog(byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_heroes_feast(byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_fly_group(byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_web(byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_minor_track(byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_major_track(byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_mana(byte level, struct char_data *ch, char *arg, int type,
          struct char_data *tar_ch, struct obj_data *tar_obj );



void cast_lightning_bolt( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, 
      struct obj_data *tar_obj);
void cast_locate_object( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_magic_missile( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_vitalize_mana( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_mon_sum1( byte level, struct char_data *ch, char *arg, int si, 
         struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum2( byte level, struct char_data *ch, char *arg, int si, 
         struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum3( byte level, struct char_data *ch, char *arg, int si, 
         struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum4( byte level, struct char_data *ch, char *arg, int si, 
         struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum5( byte level, struct char_data *ch, char *arg, int si, 
         struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum6( byte level, struct char_data *ch, char *arg, int si, 
         struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum7( byte level, struct char_data *ch, char *arg, int si, 
         struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_meteor_swarm( byte level, struct char_data *ch, char *arg, int si, 
             struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_disintegrate( byte level, struct char_data *ch, char *arg, int si,
                      struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_poly_self( byte level, struct char_data *ch, char *arg, int si, 
       struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_poison( byte level, struct char_data *ch, char *arg, int si, 
       struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_protection_from_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_curse( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sanctuary( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sleep( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_strength( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_stone_skin( byte level, struct char_data *ch, char *arg, int si, 
           struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_summon( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_ventriloquate( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_word_of_recall( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_water_breath( byte level, struct char_data *ch, char *arg, int si, 
             struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_remove_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_paralysis( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_weakness( byte level, struct char_data *ch, char *arg, int type,
         struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_sense_life( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_identify( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_paralyze( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_dragon_breath( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *potion);
void cast_fireshield( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_serious( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_serious( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_refresh( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_second_wind( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shield( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_turn( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_well_of_knowledge( byte level, struct char_data *ch, char *arg, 
int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_succor( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_astral_walk( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_resurrection( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_portal( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_farlook( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_invisible( byte level, struct char_data *ch, char *arg,
int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_silence( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_heal_spray( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_vampiric_touch( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_life_leech( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_synostodweomer( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_control_undead( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_golem( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);
struct spell_info_type spell_info[MAX_SPL_LIST];

char *spells[]=
{
   "armor",               /* 1 */
   "teleport",
   "bless",
   "blindness",
   "burning hands",
   "call lightning",
   "charm person",
   "chill touch",
   "clone",
   "colour spray",
   "control weather",     /* 11 */
   "create food",
   "create water",
   "cure blind",
   "cure critic",
   "cure light",
   "curse",
   "detect evil",
   "detect invisibility",
   "detect magic",
   "detect poison",       /* 21 */
   "dispel evil",
   "earthquake",
   "enchant weapon",
   "energy drain",
   "fireball",
   "harm",
   "heal",
   "invisibility",
   "lightning bolt",
   "locate object",      /* 31 */
   "magic missile",
   "poison",
   "protection from evil",
   "remove curse",
   "sanctuary",
   "shocking grasp",
   "sleep",
   "strength",
   "summon",
   "ventriloquate",      /* 41 */
   "word of recall",
   "remove poison",
   "sense life",         /* 44 */

   /* RESERVED SKILLS */
   "Sneak",        /* 45 */
   "Hide",
   "Steal",
   "Backstab",
   "Pick Locks",
   "Kick",         /* 50 */
   "Bash",
   "Rescue",
   /* NON-CASTABLE SPELLS (Scrolls/potions/wands/staffs) */

   "identify",           /* 53 */
   "infravision",        
   "cause light",        
   "cause critical",
   "flamestrike",
   "dispel good",      
   "weakness",
   "dispel magic",
   "knock",
   "know alignment",
   "animate dead",
   "paralyze",
   "remove paralysis",
   "fear",
   "acid blast",  /* 67 */
   "water breath",
   "fly",
   "cone of cold",   /* 70 */
   "meteor swarm",
   "ice storm",
   "shield",
   "monsum one",
   "monsum two",
   "monsum three",
   "monsum four",
   "monsum five",
   "monsum six",
   "monsum seven",  /* 80 */
   "fireshield",
   "charm monster",
   "cure serious",
   "cause serious",
   "refresh",
   "second wind",
   "turn",
   "succor",
   "create light",
   "continual light",   /* 90 */
   "calm",
   "stone skin",
   "conjure elemental",
   "true sight",
   "minor creation",
   "faerie fire",
   "faerie fog",
   "cacaodemon",
   "polymorph self",
   "mana",  /* 100 */
   "astral walk",
   "resurrection",
   "heroes feast",  /* 103 */
   "group fly",
   "breath",
   "web",
   "minor track",
   "major track",
   "full heal",
   "vitalize mana",  /* 110 */
   "portal",
   "silence",
   "Special",
   "heal spray",
   "vampiric touch",
   "synostodweomer",
   "life leech",
   "control undead",
   "well of knowledge",
   "farlook",  /* 120 */
   "dispel invisible",
   "Special",
   "protect from good",
   "disintegrate",
   "create golem",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 130 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 140 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 150 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "Sign",  /* 160 */
   "Swim",
   "Con Undead",
   "Con Vegetable",
   "Con Demon",
   "Con Animal",
   "Con Reptile",
   "Con People",
   "Con Giant",
   "Con Other",
   "Switch Opponent",   /* 170 */
   "Feign Death",
   "First Aid",
   "Dodge",
   "Quivering Palm",
   "Spring Leap",
   "Lay Hands",
   "Remove Traps",
   "Find Traps",
   "Retreat",
   "Track", /* 180 */
   "",
   "Set Traps",
   "Disarm",
   "Read Magic",
   "",
   "Grapple",
   "Headbutt",
   "Subterfuge",
   "Throw",
   "Brew",  /* 190 */
   "Scribe",
   "Double Attack",
   "Deathstroke",
   "Bodyslam",
   "****",
   "****",
   "SKILL_SPY",
   "****",
   "****",
   "",   /* 200 */
   "fire breath",
   "gas breath",
   "frost breath",
   "acid breath",
   "lightning breath",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 210 */
   "****",
   "SKILL_HUNT",    /*  (180) */
   "\n"
};


const byte saving_throws[8][5][ABS_MAX_LVL] = {
{
  {16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10,10, 8, 6, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0},
  {13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0}
}, {
  {11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0},
  {15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}, {
  {15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 7, 6, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,16,16,15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12,11, 9, 5, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,15,15,13,13,13,13,11,11,11,11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}, {
  {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}, {
  {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}, {  {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}, {
  {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}, {
  {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}
};

int SPELL_LEVEL(struct char_data *ch, int sn) 
{
  if (HasClass(ch, CLASS_ANTIPALADIN)) {
     return(spell_info[sn].min_level_anti);
  } else if (HasClass(ch, CLASS_PALADIN)) {
      return(spell_info[sn].min_level_pal);
  } else if ((HasClass(ch, CLASS_MAGIC_USER)) &&
              (HasClass(ch, CLASS_CLERIC))) {
   return(MIN(spell_info[sn].min_level_magic,spell_info[sn].min_level_cleric));
  } else if (HasClass(ch, CLASS_MAGIC_USER)) {
    return(spell_info[sn].min_level_magic);
  } else {
    return(spell_info[sn].min_level_cleric);
  }
}


void affect_update( int pulse )
{
  static struct affected_type *af, *next_af_dude;
  register struct char_data *i;
  register struct obj_data *j;
  struct obj_data *next_thing;
  struct char_data  *next_char;
  struct room_data *rp;
  int dead=FALSE, room, cost, k;
  char buf[200];
  
  extern struct time_info_data time_info;
  
  
  void update_char_objects( struct char_data *ch ); /* handler.c */
  void do_save(struct char_data *ch, char *arg, int cmd); /* act.other.c */
  
  
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
    /*
     *  check the effects on the char
     */
    dead=FALSE;
    for (af = i->affected; af&&!dead; af = next_af_dude) {
      next_af_dude = af->next;
      if (af->duration >= 1) {
   af->duration--;
 
        if (af->duration == 1) {
                  SpellWearOffSoon(af->type, i);
        }
      } else {
   /* It must be a spell */
   if ((af->type > 0) && (af->type <= FIRST_BREATH_WEAPON)) { /* urg.. a constant */
     if (!af->next || (af->next->type != af->type) ||
         (af->next->duration > 0)) {

             k = af->type;
             SpellWearOff(k, i);
             affect_remove(i, af);
     }
   } else if (af->type>=FIRST_BREATH_WEAPON &&
         af->type <=LAST_BREATH_WEAPON ) {
     extern funcp bweapons[];
     bweapons[af->type-FIRST_BREATH_WEAPON](-af->modifier/2, i, "",
                   SPELL_TYPE_SPELL, i, 0);
     if (!i->affected) {
       /* oops, you're dead :) */
       dead = TRUE;
       break;
     }
          affect_remove(i, af);
   }
   
      }
    }
    if (!dead) {
     if (real_roomp(i->in_room)==real_roomp(3198)) {
      if (IsSingleClass(i))
        cost = 25*GetMaxLevel(i);
      else
        cost = 50*GetMaxLevel(i);
        if (GET_GOLD(i) < cost) {
             send_to_char("The hospital guard tells you 'No vagrants allowed'.\n\r", i);
             send_to_char("You are thrown out by the hospital guards.\n\r", i);
             char_from_room(i);
             char_to_room(i, 3001);
             return;
             } else {
        sprintf(buf, "You are charged %d coins by the hospital.\n\r", cost);
        send_to_char(buf, i);
        GET_GOLD(i) -= cost;
        send_to_char("You feel much fresher than usual.\n\r", i);
        GET_HIT(i)  = MIN(GET_HIT(i)  + 4*hit_gain(i),  hit_limit(i));
        GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
        GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
        }
     } else if ((GET_POS(i) > POSITION_STUNNED) && 
          (IS_SET(real_roomp(i->in_room)->room_flags, NO_HEAL))) {
        GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
     } else if (IS_SET(real_roomp(i->in_room)->room_flags, HOSPITAL)) {
        if (!strncmp(GET_NAME(i), real_roomp(i->in_room)->name, 
                strlen(GET_NAME(i)))) {
        send_to_char("Your house refreshes you.\n\r", i);
        GET_HIT(i)  = MIN(GET_HIT(i)  + 2*hit_gain(i),  hit_limit(i));
        GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
        GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
        }
     }  else if (IS_AFFECTED(i, AFF_POISON)) {
         if (IS_PC(i))
           damage(i,i,5,SPELL_POISON);
     }  else if ((GET_COND(i,FULL)==0) || (GET_COND(i,THIRST)==0)) {
        if (i->desc) {
             GET_HIT(i) -= 1;
             GET_MOVE(i) -= 1;
             update_pos(i);
              if (GET_HIT(i) < -10) {
                sprintf(buf, "%s killed by starving", GET_NAME(i));
                log(buf);
                die(i);
              }
         }
     }  else if ((GET_POS(i) > POSITION_STUNNED) && 
         (!IS_SET(real_roomp(i->in_room)->room_flags, NO_HEAL))) {
        GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
        GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
        GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
      } else if (GET_POS(i) == POSITION_STUNNED) { 
     update_pos( i );
      } else if (GET_POS(i) == POSITION_INCAP) {
        /* do nothing */  damage(i, i, 0, TYPE_SUFFERING);
         } else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW)) {
           damage(i, i, 1, TYPE_SUFFERING);
         }
      if (!IS_NPC(i))   {
        update_char_objects(i);
        if ((GetMaxLevel(i) < DEMIGOD) && (i->in_room != 3))
          i->specials.timer++;
     check_idling(i);
        rp = real_roomp(i->in_room);
   if (rp) {
     if (rp->sector_type == SECT_WATER_SWIM ||
         rp->sector_type == SECT_WATER_NOSWIM) {
             gain_condition(i,FULL,-1);
             gain_condition(i,DRUNK,-1);
      } else if (rp->sector_type == SECT_DESERT) {
             gain_condition(i,FULL,-1);
             gain_condition(i,DRUNK,-2);
             gain_condition(i,THIRST,-2);
      } else if (rp->sector_type == SECT_MOUNTAIN ||
            rp->sector_type == SECT_HILLS) {
             gain_condition(i,FULL,-2);
             gain_condition(i,DRUNK,-2);
      } else {
             gain_condition(i,FULL,-1);
             gain_condition(i,DRUNK,-1);
             gain_condition(i,THIRST,-1);
      }
         }
        if (i->specials.tick == time_info.hours) {/* works for 24, change for
                       anything else        */
     if (!IS_IMMORTAL(i))
        do_save(i,"",0);
       }
      }
      check_drowning(i);
    }
  }
  
  for(j = object_list; j ; j = next_thing){
    next_thing = j->next; /* Next in object list */

    if (obj_index[j->item_number].func)
       (*obj_index[j->item_number].func)(NULL, 0, NULL, j);

  if (j->obj_flags.decay_time > -1) 
  {
   /* update_char_objects takes care of worn, carried */
    if (j->in_room != NOWHERE)  /*This takes care of objects in room*/
      j->obj_flags.decay_time--;

    if (j->obj_flags.decay_time == 0) {
      if (j->name && !strncmp(j->name,"portal",6)) {	/* PORTALS */
        if ((j->in_room != NOWHERE) && (real_roomp(j->in_room)->people)) {
            act("$p flickers out of view.", TRUE,real_roomp(j->in_room)->people, j, 0, TO_ROOM);
            act("$p flickers out of view.", TRUE,real_roomp(j->in_room)->people, j, 0, TO_CHAR);
        }
      }
      else if (j->name && !strncmp(j->name,"corpse",6)) {	/* CORPSES */
         if (j->carried_by)
            act("$p biodegrades in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
         else if ((j->in_room != NOWHERE) && (real_roomp(j->in_room)->people)) {
            act("$p dissolves into a fertile soil.",TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
            act("$p dissolves into a fertile soil.", TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
         }  
         ObjFromCorpse(j);
      }
      /* FOOD */
      else if (GET_ITEM_TYPE(j) == ITEM_FOOD) {
         if (j->carried_by && !j->in_obj)
            act("$p spoils in your inventory.", FALSE, j->carried_by, j, 0, TO_CHAR);
         else if ((j->in_room != NOWHERE) && (real_roomp(j->in_room)->people)) {
            act("$p totally rots, and is gone.", TRUE,real_roomp(j->in_room)->people, j, 0, TO_ROOM);
            act("$p totally rots, and is gone.", TRUE,real_roomp(j->in_room)->people, j, 0, TO_CHAR);
         } else if (j->in_obj && j->in_obj->carried_by) {
            sprintf(buf,"%s rots in %s, and leaves a filthy residue.\n\r", j->short_description, j->in_obj->short_description);
            send_to_char(buf, j->in_obj->carried_by);
         }
      } 
      else {
	 if (j->equipped_by)   /* Worn in equipment */  /* EVERYTHING ELSE that DECAYS */
            act("$p decays into nothing.", FALSE, j->equipped_by, j, 0, TO_CHAR);
         if (j->carried_by && !j->in_obj)  /*  In inverntory but not in a bag */
            act("$p biodegrades in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
      }
    }
  }
      /*
       *  Sound objects
       */
      if (ITEM_TYPE(j) == ITEM_AUDIO) {
   if (((j->obj_flags.value[0]) && 
        (pulse % j->obj_flags.value[0])==0) ||
       (!number(0,5))) {
     if (j->carried_by) {
       room = j->carried_by->in_room;
     } else if (j->equipped_by) {
       room = j->equipped_by->in_room;
     } else if (j->in_room != NOWHERE) {
       room = j->in_room;
     } else {
       room = RecGetObjRoom(j);
     }
     /*
      *  broadcast to room
      */
     
     if (j->action_description) {     
       MakeNoise(room, j->action_description, j->action_description);
    }
   }
  }    
 }
}  


void clone_char(struct char_data *ch)
{
        send_to_char("Nosir, i don't like it\n\r", ch);
}



void clone_obj(struct obj_data *obj)
{

}



/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
   struct char_data *k;

   for(k=victim; k; k=k->master) {
      if (k == ch)
         return(TRUE);
   }

   return(FALSE);
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
   struct follow_type *j, *k;

   if (!ch->master) return;

   if (IS_AFFECTED(ch, AFF_CHARM)) {
      act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
      act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
      act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
      if (affected_by_spell(ch, SPELL_CHARM_PERSON))
         affect_from_char(ch, SPELL_CHARM_PERSON);
   } else {
      act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
           if (!IS_SET(ch->specials.act,PLR_STEALTH)) {
      act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
      act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
      }
   }

   if (ch->master->followers->follower == ch) { /* Head of follower-list? */
      k = ch->master->followers;
      ch->master->followers = k->next;
      free(k);
   } else { /* locate follower who is not head of list */

      for(k = ch->master->followers; k->next && k->next->follower!=ch; k=k->next)  ;

      if (k->next) {
         j = k->next;
         k->next = j->next;
         free(j);
      }
   }

   ch->master = 0;
   REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
   struct follow_type *j, *k;

   if (ch->master)
      stop_follower(ch);

   for (k=ch->followers; k; k=j) {
      j = k->next;
      stop_follower(k->follower);
   }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader)
{
   struct follow_type *k;

   assert(!ch->master);

   ch->master = leader;

   CREATE(k, struct follow_type, 1);

   k->follower = ch;
   k->next = leader->followers;
   leader->followers = k;

        
   act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
        if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
      act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
      act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
   }
}



say_spell( struct char_data *ch, int si )
{
   char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   int j, offs;
   struct char_data *temp_char;


   struct syllable {
      char org[10];
      char new[10];
   };

   struct syllable syls[] = {
   { " ", " " },
   { "ar", "abra"   },
   { "au", "kada"    },
   { "bless", "fido" },
  { "blind", "nose" },
  { "bur", "mosa" },
   { "cu", "judi" },
        { "ca", "jedi" },
   { "de", "oculo"},
   { "en", "unso" },
   { "light", "dies" },
   { "lo", "hi" },
   { "mor", "zak" },
   { "move", "sido" },
  { "ness", "lacri" },
  { "ning", "illa" },
   { "per", "duda" },
   { "ra", "gru"   },
  { "re", "candus" },
   { "son", "sabru" },
        { "se",  "or"},
  { "tect", "infra" },
   { "tri", "cula" },
   { "ven", "nofo" },
   {"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
   {"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
   {"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
   {"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
   };



   strcpy(buf, "");
   strcpy(splwd, spells[si-1]);

   offs = 0;

   while(*(splwd+offs)) {
      for(j=0; *(syls[j].org); j++)
         if (strncmp(syls[j].org, splwd+offs, strlen(syls[j].org))==0) {
            strcat(buf, syls[j].new);
            if (strlen(syls[j].org))
               offs+=strlen(syls[j].org);
            else
               ++offs;
         }
   }


   sprintf(buf2,"$n utters the words, '%s'", buf);
   sprintf(buf, "$n utters the words, '%s'", spells[si-1]);

   for(temp_char = real_roomp(ch->in_room)->people;
      temp_char;
      temp_char = temp_char->next_in_room)
      if(temp_char != ch) {
/*
**  Remove-For-Multi-Class
*/
         if (ch->player.class == temp_char->player.class)
            act(buf, FALSE, ch, 0, temp_char, TO_VICT);
         else
            act(buf2, FALSE, ch, 0, temp_char, TO_VICT);

      }

}



bool saves_spell(struct char_data *ch, sh_int save_type)
{
   int save;

   /* Negative apply_saving_throw makes saving throw better! */

   save = ch->specials.apply_saving_throw[save_type];

   if (!IS_NPC(ch)) {
/*
**  Remove-For-Multi-Class
*/
      save += saving_throws[BestMagicClass(ch)][save_type][GET_LEVEL(ch,BestMagicClass(ch))];
      if (GetMaxLevel(ch) > MAX_MORT)
         return(TRUE);
   }

   return(MAX(1,save) < number(1,20));
}

bool ImpSaveSpell(struct char_data *ch, sh_int save_type, int mod)
{
   int save;

        /* Positive mod is better for save */

   /* Negative apply_saving_throw makes saving throw better! */

   save = ch->specials.apply_saving_throw[save_type] - mod;


   if (!IS_NPC(ch)) {
/*
**  Remove-For-Multi-Class
*/
      save += saving_throws[BestMagicClass(ch)][save_type][GET_LEVEL(ch,BestMagicClass(ch))];
      if (GetMaxLevel(ch) >= LOW_IMMORTAL)
         return(TRUE);
   }

   return(MAX(1,save) < number(1,20));
}



char *skip_spaces(char *string)
{
   for(;*string && (*string)==' ';string++);

   return(string);
}



/* Assumes that *argument does start with first letter of chopped string */

void do_cast(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *tar_obj;
  struct char_data *tar_char;
  char name[MAX_INPUT_LENGTH];
  int qend, spl, i;
  bool target_ok;
  
  if (IS_NPC(ch) && (!IS_SET(ch->specials.act, ACT_POLYSELF)))
    return;

  if (!HasHands(ch)) {
    send_to_char("Sorry, you don't have the right form for that.\n\r",ch);
    return;
  }
  
  if (!IS_IMMORTAL(ch)) {
    if (BestMagicClass(ch) == WARRIOR_LEVEL_IND) {
      send_to_char("Think you had better stick to fighting...\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
      send_to_char("Think you should stick to robbing and killing...\n\r", ch);
      return;
    }
  }

  if  ((HasClass(ch, CLASS_ANTIPALADIN)) && (GET_ALIGNMENT(ch) > -350)) {
       send_to_char("You aren't evil enuff to cast spells now!!\n\r",ch);
       return;
       }

 if (!IS_SET(ch->specials.act, ACT_POLYSELF)) {
  if  ((HasClass(ch, CLASS_PALADIN)) && (GET_ALIGNMENT(ch) < 350)) {
       send_to_char("You aren't holy enough to cast spells now!!\n\r", ch);
       return;
       }
     }

  if (apply_soundproof(ch))
     return;

  if (IS_SET(real_roomp(ch->in_room)->room_flags, NO_MAGIC)) {
      send_to_char("Sorry, this is a no-magic area.\n\r", ch);
      return;
      }

  argument = skip_spaces(argument);
  
  /* If there is no chars in argument */
  if (!(*argument)) {
    send_to_char("Cast which what where?\n\r", ch);
    return;
  }
  
  if (*argument != '\'') {
    send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
    return;
  }
  
  /* Locate the last quote && lowercase the magic words (if any) */
  
  for (qend=1; *(argument+qend) && (*(argument+qend) != '\'') ; qend++)
    *(argument+qend) = LOWER(*(argument+qend));
  
  if (*(argument+qend) != '\'') {
    send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
    return;
  }
  
  spl = old_search_block(argument, 1, qend-1,spells, 0);
  
  if (!spl) {
    send_to_char("Your lips do not move, no magic appears.\n\r",ch);
    return;
  }

  if (!ch->skills)
    return;
  
  if ((spl > 0) && (spl < MAX_SKILLS) && spell_info[spl].spell_pointer) {
    if (GET_POS(ch) < spell_info[spl].minimum_position) {
      switch(GET_POS(ch)) {
      case POSITION_SLEEPING :
   send_to_char("You dream about great magical powers.\n\r", ch);
   break;
      case POSITION_RESTING :
   send_to_char("You can't concentrate enough while resting.\n\r",ch);
   break;
      case POSITION_SITTING :
   send_to_char("You can't do this sitting!\n\r", ch);
   break;
      case POSITION_FIGHTING :
   send_to_char("Impossible! You can't concentrate enough!.\n\r", ch);
   break;
      default:
   send_to_char("It seems like you're in pretty bad shape!\n\r",ch);
   break;
      } /* Switch */
    } else {
      
      if (!IS_IMMORTAL(ch)) {

   if ((spell_info[spl].min_level_magic > 
        GET_LEVEL(ch,MAGE_LEVEL_IND)) &&
       (spell_info[spl].min_level_cleric >
        GET_LEVEL(ch,CLERIC_LEVEL_IND)) &&
            (spell_info[spl].min_level_anti >
              GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND)) &&
            (spell_info[spl].min_level_pal >
             GET_LEVEL(ch, PALADIN_LEVEL_IND)) &&
            (spell_info[spl].min_level_ranger >
             GET_LEVEL(ch, RANGER_LEVEL_IND)))  {
     send_to_char("Sorry, you can't do that.\n\r", ch);
     return;
   }
      }
      
      argument+=qend+1; /* Point to the last ' */
      for(;*argument == ' '; argument++);
      
      /* **************** Locate targets **************** */
      
      target_ok = FALSE;
      tar_char = 0;
      tar_obj = 0;
      
      if (IS_SET(spell_info[spl].targets, TAR_VIOLENT) &&
     check_peaceful(ch, "Impolite magic is banned here."))
   return;


      if (IS_SET(spell_info[spl].targets, TAR_SINGLE)) {
         if ((!IsSingleClass(ch)) || (IS_IMMORTAL(ch))) {
          send_to_char("This spell is for single classes only.\n\r", ch);
          return;
         }
       }
         
      if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {
   
   argument = one_argument(argument, name);
   
   if (*name) {
     if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM)) {
       if (tar_char = get_char_room_vis(ch, name)) {
         if (tar_char == ch || tar_char == ch->specials.fighting ||
        tar_char->attackers < 6 || 
        tar_char->specials.fighting == ch)
      target_ok = TRUE;
         else {
      send_to_char("Too much fighting, you can't get a clear shot.\n\r", ch);
      target_ok = FALSE;
         }
       } else {
         target_ok = FALSE;
       }
     }
     if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
       if (tar_char = get_char_vis(ch, name))
         target_ok = TRUE;
     
     if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
       if (tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))
         target_ok = TRUE;
     
     if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
       if (tar_obj = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents))
         target_ok = TRUE;
     
     if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
       if (tar_obj = get_obj_vis(ch, name))
         target_ok = TRUE;

          

     
     if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
       for(i=0; i<MAX_WEAR && !target_ok; i++)
         if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0) {
      tar_obj = ch->equipment[i];
      target_ok = TRUE;
         }
     }
     
     if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
       if (str_cmp(GET_NAME(ch), name) == 0) {
         tar_char = ch;
         target_ok = TRUE;
       }
     
     if (!target_ok && IS_SET(spell_info[spl].targets, TAR_NAME)) {
       tar_obj = (void*)name;
       target_ok = TRUE;
     }
     
     if (tar_char) {
       if (IS_NPC(tar_char)) 
         if (IS_SET(tar_char->specials.act, ACT_IMMORTAL)) {
      send_to_char("You can't cast magic on that!",ch);
      return;
         }
     }

   } else { /* No argument was typed */
     
     if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
       if (ch->specials.fighting) {
         tar_char = ch;
         target_ok = TRUE;
       }
     
     if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
       if (ch->specials.fighting) {
         /* WARNING, MAKE INTO POINTER */
         tar_char = ch->specials.fighting;
         target_ok = TRUE;
       }
     
     if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
       tar_char = ch;
       target_ok = TRUE;
     }
     
   }
   
      } else {
   target_ok = TRUE; /* No target, is a good target */
      }
      
      if (!target_ok) {
   if (*name) {
     if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
       send_to_char("Nobody playing by that name.\n\r", ch);
     else if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
       send_to_char("Nobody here by that name.\n\r", ch);
     else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
       send_to_char("You are not carrying anything like that.\n\r", ch);
     else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
       send_to_char("Nothing here by that name.\n\r", ch);
     else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
       send_to_char("Nothing at all by that name.\n\r", ch);
     else if (IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
       send_to_char("You are not wearing anything like that.\n\r", ch);
     else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
       send_to_char("Nothing at all by that name.\n\r", ch);
     
   } else { /* Nothing was given as argument */
     if (spell_info[spl].targets < TAR_OBJ_INV)
       send_to_char("Who should the spell be cast upon?\n\r", ch);
     else
       send_to_char("What should the spell be cast upon?\n\r", ch);
   }
   return;
      } else { /* TARGET IS OK */
   if ((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
     send_to_char("You can not cast this spell upon yourself.\n\r", ch);
     return;
   }
   else if ((tar_char != ch) && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
     send_to_char("You can only cast this spell upon yourself.\n\r", ch);
     return;
   } else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
     send_to_char("You are afraid that it could harm your master.\n\r", ch);
     return;
   }
      }
      
      if (GetMaxLevel(ch) < LOW_IMMORTAL) {
   if (GET_MANA(ch) < USE_MANA(ch, spl)) {
     send_to_char("You can't summon enough energy to cast the spell.\n\r", ch);
     return;
   }
      }
         if ((spl != SPELL_VENTRILOQUATE) && can_do_verbal(ch))  /* :-) */
         say_spell(ch, spl);
      
     if (!IS_IMMORTAL(ch))
      WAIT_STATE(ch, spell_info[spl].beats);
      
      if ((spell_info[spl].spell_pointer == 0) && spl>0)
   send_to_char("Sorry, this magic has not yet been implemented :(\n\r", ch);
      else {
   if (number(1,101) > ch->skills[spl].learned) { /* 101% is failure */
     send_to_char("You lost your concentration!\n\r", ch);
     GET_MANA(ch) -= (USE_MANA(ch, spl)>>1);
     return;
   }
      if (IS_SET(spell_info[spl].targets, TAR_VIOLENT)) {
       if (tar_char) {
        if (IS_PC(tar_char) && (tar_char != ch)) {
          act("$N tries to harm you by casting a malicious spell.",
              FALSE, tar_char, 0, ch, TO_CHAR);
          if (!IS_SET(ch->specials.act, PLR_KILLER) &&
              !IS_SET(tar_char->specials.act, PLR_OUTLAW) &&
              !IS_SET(tar_char->specials.act, PLR_KILLER)) {
            send_to_char("You are now a killer!\n\r", ch);
            SET_BIT(ch->specials.act, PLR_KILLER);
          }
        }
       }
      }

   send_to_char("Ok.\n\r",ch);
   ((*spell_info[spl].spell_pointer) (GET_LEVEL(ch, BestMagicClass(ch)), ch, argument, SPELL_TYPE_SPELL, tar_char, tar_obj));
   GET_MANA(ch) -= (USE_MANA(ch, spl));
      }
      
    } /* if GET_POS < min_pos */
    
    return;
  }
  
  switch (number(1,5)){
  case 1: send_to_char("Bylle Grylle Grop Gryf???\n\r", ch); break;
  case 2: send_to_char("Olle Bolle Snop Snyf?\n\r",ch); break;
  case 3: send_to_char("Olle Grylle Bolle Bylle?!?\n\r",ch); break;
  case 4: send_to_char("Gryffe Olle Gnyffe Snop???\n\r",ch); break;
  default: send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r",ch); break;
  }
}


void assign_spell_pointers(void)
{
  int i;
  
  for(i=0; i<MAX_SPL_LIST; i++)
    spell_info[i].spell_pointer = 0;
  
  
  /* From spells1.c */

  
  
  SPELLO(32,12,POSITION_FIGHTING, 1, LOW_IMMORTAL, 15,
         LOW_IMMORTAL,LOW_IMMORTAL,LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_magic_missile);
  
  SPELLO( 8,12,POSITION_FIGHTING, 3, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_chill_touch);
  
  SPELLO( 5,24,POSITION_FIGHTING, 5, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE | TAR_VIOLENT, cast_burning_hands);
  
  SPELLO(37,12,POSITION_FIGHTING, 1, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_shocking_grasp);
  
  SPELLO(23,24,POSITION_FIGHTING, LOW_IMMORTAL, 8, 15,
         21, LOW_IMMORTAL, 25,
    TAR_IGNORE | TAR_VIOLENT, cast_earthquake);
  
  SPELLO(30,24,POSITION_FIGHTING, 9, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_lightning_bolt);
  
  SPELLO(10,24,POSITION_FIGHTING, 11,LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_colour_spray);
  
  SPELLO(22,24,POSITION_FIGHTING, LOW_IMMORTAL, 12, 15,
         LOW_IMMORTAL, 35, LOW_IMMORTAL,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_evil);
  
  SPELLO(26,36,POSITION_FIGHTING, 15, LOW_IMMORTAL, 15,
         LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
        TAR_IGNORE | TAR_VIOLENT, cast_fireball);
  
  SPELLO( 6,36,POSITION_FIGHTING, LOW_IMMORTAL, 15, 15,
         LOW_IMMORTAL, LOW_IMMORTAL, 27,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_call_lightning);
  
  SPELLO(25,36,POSITION_FIGHTING, 17, LOW_IMMORTAL, 35,
         40, LOW_IMMORTAL, LOW_IMMORTAL,        
        TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_energy_drain);
  
  SPELLO(27,36,POSITION_FIGHTING, LOW_IMMORTAL, 17, 35,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_harm);
  
  
  /* Spells2.c */
  
  SPELLO( 1,12,POSITION_STANDING, 4,  1, 5,
          9, 8, 8,
    TAR_CHAR_ROOM, cast_armor);
  
  SPELLO( 2,12,POSITION_FIGHTING, 8, LOW_IMMORTAL, 33,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_teleport);
  
  SPELLO( 3,12,POSITION_STANDING,LOW_IMMORTAL,  1, 5,
          LOW_IMMORTAL, 13, LOW_IMMORTAL,
    TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM, cast_bless);
  
  SPELLO( 4,24,POSITION_FIGHTING, 8,  6, 5,
          19, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_blindness);
  
  SPELLO(7,12,POSITION_STANDING, 2, LOW_IMMORTAL, 5,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT,
    cast_charm_person);
  
  /* */  SPELLO( 9,12,POSITION_STANDING,15, LOW_IMMORTAL, 40, 
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,   
          TAR_CHAR_ROOM, cast_clone);
  
  SPELLO(11,36,POSITION_FIGHTING,10, 13, 25,
LOW_IMMORTAL, LOW_IMMORTAL, 20,
    TAR_IGNORE, cast_control_weather);
  
  SPELLO(12,12,POSITION_STANDING,LOW_IMMORTAL,  3, 5,
LOW_IMMORTAL, LOW_IMMORTAL, 10,
    TAR_IGNORE, cast_create_food);
  
  SPELLO(13,12,POSITION_STANDING,LOW_IMMORTAL,  2, 5,
LOW_IMMORTAL, LOW_IMMORTAL, 10,
    TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_create_water);
  
  SPELLO(14,12,POSITION_STANDING,LOW_IMMORTAL,  4, 5,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_cure_blind);
  
  SPELLO(15,24,POSITION_FIGHTING,LOW_IMMORTAL,  9, 11,
LOW_IMMORTAL, 23, 25,
    TAR_CHAR_ROOM, cast_cure_critic);
  
  SPELLO(16,12,POSITION_FIGHTING,LOW_IMMORTAL,  1, 5,
         LOW_IMMORTAL, 9, 8,
    TAR_CHAR_ROOM, cast_cure_light);
  
  SPELLO(17,24,POSITION_STANDING,12, 12, 20,
         7, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_FIGHT_VICT | TAR_VIOLENT, cast_curse);
  
  SPELLO(18,12,POSITION_STANDING, LOW_IMMORTAL, 1, 5,
LOW_IMMORTAL, 15, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_detect_evil);
  
  SPELLO(19,12,POSITION_STANDING, 2,  5, 5,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_detect_invisibility);
  
  SPELLO(20,12,POSITION_STANDING, 5,  3, 5,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_detect_magic);
  
  SPELLO(21,12,POSITION_STANDING,LOW_IMMORTAL,  2, 5,
         12, 14, 15,
    TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_detect_poison);
  
  SPELLO(24,48,POSITION_STANDING,20, LOW_IMMORTAL, 100,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_enchant_weapon);
  
  SPELLO(28,12,POSITION_FIGHTING,LOW_IMMORTAL, 17, 50,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_heal);
  
  SPELLO(29,12,POSITION_STANDING, 4, LOW_IMMORTAL, 5,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP, cast_invisibility);
  
  SPELLO(31,12,POSITION_STANDING, LOW_IMMORTAL, 4, 20,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_NAME, cast_locate_object);
  
  SPELLO(33,24,POSITION_FIGHTING,LOW_IMMORTAL,  8, 10,
         5, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP | 
    TAR_FIGHT_VICT,  cast_poison);
  
  SPELLO(34,12,POSITION_STANDING,LOW_IMMORTAL,  6, 5,
LOW_IMMORTAL, 18, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_protection_from_evil);
  
  SPELLO(35,12,POSITION_STANDING,LOW_IMMORTAL, 7, 5,
LOW_IMMORTAL, 25, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, cast_remove_curse);
  
  SPELLO(36,36,POSITION_STANDING, LOW_IMMORTAL, 19, 50,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_sanctuary);
  
  SPELLO(38,24,POSITION_STANDING, 3, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_sleep);
  
  SPELLO(39,12,POSITION_STANDING, 4, LOW_IMMORTAL, 10,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_strength);
  
  SPELLO(40,36,POSITION_STANDING, 18,  16, 20,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_WORLD, cast_summon);
  
  /* */  SPELLO(41,12,POSITION_STANDING, 1, LOW_IMMORTAL, 5,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
          TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, cast_ventriloquate);
  
  SPELLO(42,12,POSITION_STANDING,LOW_IMMORTAL, 10, 5,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_word_of_recall);
  
  SPELLO(43,12,POSITION_STANDING,LOW_IMMORTAL,  5, 5,
LOW_IMMORTAL, 19, 21,
    TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, cast_remove_poison);
  
  SPELLO(44,12,POSITION_STANDING,LOW_IMMORTAL,  7, 5,
LOW_IMMORTAL, LOW_IMMORTAL, 17,
    TAR_CHAR_ROOM, cast_sense_life);
  
  SPELLO(45,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, 0);
  SPELLO(46,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, 0);
  SPELLO(47,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, 0);
  SPELLO(48,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, 0);
  SPELLO(49,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, 0);
  SPELLO(50,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, 0);
  SPELLO(51,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, 0);
  SPELLO(52,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, 0);
  
  SPELLO(53,1,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1, 100,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
 TAR_IGNORE, cast_identify);
  
  SPELLO(54,12,POSITION_STANDING, 5, LOW_IMMORTAL, 7,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_infravision); 
  
  SPELLO(55,12,POSITION_FIGHTING, LOW_IMMORTAL,  1, 8,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_cause_light);
  
  SPELLO(56,24,POSITION_FIGHTING, LOW_IMMORTAL,  9, 11,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT , cast_cause_critic);
  
  SPELLO(57,36,POSITION_FIGHTING, LOW_IMMORTAL , 11, 15,
         30, 27, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_flamestrike);
  
  SPELLO(58,36,POSITION_FIGHTING, LOW_IMMORTAL, 12, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_good);
  
  SPELLO(59,12,POSITION_FIGHTING, 4, LOW_IMMORTAL, 10,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_weakness);
  
  SPELLO(60,12,POSITION_FIGHTING, 6, 6, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_magic);
  
  SPELLO(61,12,POSITION_STANDING, 3, LOW_IMMORTAL, 10,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_knock);
  
  SPELLO(62,12,POSITION_FIGHTING, 4, 3, 10,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_know_alignment);
  
  SPELLO(63,24,POSITION_STANDING, 10, 7, 15,
14, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_OBJ_ROOM, cast_animate_dead);
  
  SPELLO(64,36,POSITION_STANDING, 15, LOW_IMMORTAL, 50,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_paralyze);
  
  SPELLO(65,12,POSITION_STANDING, LOW_IMMORTAL, 4, 10,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_remove_paralysis);
  
  SPELLO( 66, 12, POSITION_FIGHTING, 8, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_fear);
  
  SPELLO(67,24,POSITION_FIGHTING, 7, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_acid_blast);
  
  SPELLO(68,12,POSITION_FIGHTING, 4, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_water_breath);
  
  SPELLO(69,12,POSITION_FIGHTING, 3, 9, 15,
LOW_IMMORTAL, LOW_IMMORTAL, 21,
    TAR_CHAR_ROOM, cast_flying);
  
  SPELLO(70,24,POSITION_FIGHTING, 11, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE | TAR_VIOLENT,  cast_cone_of_cold);
  
  SPELLO(71,24,POSITION_FIGHTING, 20, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_meteor_swarm);
  
  SPELLO(72,12,POSITION_FIGHTING, 7, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, 30,
    TAR_IGNORE | TAR_VIOLENT,  cast_ice_storm);
  
  SPELLO(73,24,POSITION_FIGHTING, 1, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_shield);
  
  SPELLO(74,24,POSITION_FIGHTING, 5, LOW_IMMORTAL, 10,
LOW_IMMORTAL, LOW_IMMORTAL, 10,
    TAR_IGNORE, cast_mon_sum1);
  
  SPELLO(75,24,POSITION_FIGHTING, 7, LOW_IMMORTAL, 12,
LOW_IMMORTAL, LOW_IMMORTAL, 13,
    TAR_IGNORE, cast_mon_sum2);
  
  SPELLO(76,24,POSITION_FIGHTING, 9, LOW_IMMORTAL, 15,
LOW_IMMORTAL, LOW_IMMORTAL, 16,
    TAR_IGNORE, cast_mon_sum3);
  
  SPELLO(77,24,POSITION_FIGHTING, 11, LOW_IMMORTAL, 17,
LOW_IMMORTAL, LOW_IMMORTAL, 19,
    TAR_IGNORE, cast_mon_sum4);
  
  SPELLO(78,24,POSITION_FIGHTING, 13, LOW_IMMORTAL, 20,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_mon_sum5);
  
  SPELLO(79,24,POSITION_FIGHTING, 15, LOW_IMMORTAL, 22,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_mon_sum6);
  
  SPELLO(80,24,POSITION_STANDING, 17, LOW_IMMORTAL, 25,
LOW_IMMORTAL, LOW_IMMORTAL, 35,
    TAR_IGNORE, cast_mon_sum7);

 SPELLO(81,12,POSITION_STANDING, 20, 51, 73,
51,51,51, 
         TAR_SELF_ONLY | TAR_SINGLE, cast_fireshield);
 
  
  SPELLO(82,12,POSITION_STANDING, 8, LOW_IMMORTAL, 5,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_VIOLENT, cast_charm_monster);
  
  SPELLO(83,12,POSITION_FIGHTING, 30, 7, 9,
LOW_IMMORTAL, 17, 17,
    TAR_CHAR_ROOM, cast_cure_serious);
  
  SPELLO(84,12,POSITION_FIGHTING, 30, 7, 9,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_VIOLENT, cast_cause_serious);
  
  SPELLO(85,12,POSITION_STANDING, 3, 2, 5,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_refresh);
  
  SPELLO(86,12,POSITION_STANDING, 12, 6, 5,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_second_wind);
  
  SPELLO(87,12,POSITION_STANDING, LOW_IMMORTAL, 1, 5,
10, 10, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_turn);
  
  SPELLO(88,24,POSITION_STANDING, 21, 18, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_succor);
  
  SPELLO(89,12,POSITION_STANDING, 1, 2, 5,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_light);
  
  SPELLO(90,24,POSITION_STANDING, 3, 4, 10,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_cont_light);
  
  SPELLO(91,24,POSITION_STANDING, 4, 2, 15,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_calm);
  
  SPELLO(92,24,POSITION_STANDING, 16, LOW_IMMORTAL, 20,
LOW_IMMORTAL, LOW_IMMORTAL, 51,
    TAR_SELF_ONLY, cast_stone_skin);
  
  SPELLO(93,24,POSITION_STANDING, 16, 14, 30,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_conjure_elemental);
  
  SPELLO(94,24,POSITION_STANDING, LOW_IMMORTAL, 12, 20,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_true_seeing);
  
  SPELLO(95,24,POSITION_STANDING, 8, LOW_IMMORTAL, 30,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_minor_creation);
  
  SPELLO(96,12,POSITION_STANDING, 5, 3, 10,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT, cast_faerie_fire);
  
  SPELLO(97,24,POSITION_STANDING, 13, 10, 20,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_faerie_fog);
  
  SPELLO(98,24,POSITION_STANDING, 25, 25, 50,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_cacaodemon);
  
  SPELLO(99,12,POSITION_FIGHTING, 8, LOW_IMMORTAL, 10,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_poly_self);
    
  SPELLO( 101,12,POSITION_STANDING, 21, 25, 33,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_WORLD, cast_astral_walk);
  
  SPELLO( 102,36,POSITION_STANDING, 50, 21, 33,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_OBJ_ROOM, cast_resurrection);

  SPELLO( 103,12,POSITION_STANDING, 20, 15, 40,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_heroes_feast);

  SPELLO( 104,12,POSITION_STANDING, 20, 14, 30,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_IGNORE, cast_fly_group);

  SPELLO( 106,12,POSITION_STANDING, 4, LOW_IMMORTAL, 3,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_web);

  SPELLO( 107,12,POSITION_STANDING, 7, LOW_IMMORTAL, 10,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_minor_track);

  SPELLO( 108,12,POSITION_STANDING, 16, LOW_IMMORTAL, 20,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_CHAR_ROOM, cast_major_track);
  
  SPELLO( 109,12,POSITION_FIGHTING, LOW_IMMORTAL, 30, 75,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
         TAR_CHAR_ROOM, cast_full_heal);

  SPELLO( 111,12, POSITION_STANDING, 23, 30, 100,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
         TAR_CHAR_WORLD, cast_portal);

  SPELLO( 112,12, POSITION_STANDING, 5, 15, 25,
LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
         TAR_CHAR_ROOM, cast_silence);

 SPELLO( 114,12, POSITION_STANDING, LOW_IMMORTAL, 40, 100,
         LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
         TAR_IGNORE, cast_heal_spray);

 SPELLO( 115,12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 15,
         15, LOW_IMMORTAL, LOW_IMMORTAL,
         TAR_VIOLENT | TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_vampiric_touch);
  
 SPELLO( 116,12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 50,
         LOW_IMMORTAL, 30, LOW_IMMORTAL,
         TAR_CHAR_ROOM, cast_synostodweomer);

 SPELLO( 117,12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 30,
         25, LOW_IMMORTAL, LOW_IMMORTAL,
         TAR_IGNORE | TAR_VIOLENT | TAR_FIGHT_VICT,  cast_life_leech);

 SPELLO( 118,12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 10,
         20, LOW_IMMORTAL, LOW_IMMORTAL,
         TAR_CHAR_ROOM, cast_control_undead);
 
 SPELLO( 119,12, POSITION_STANDING, 20, 20, 25,
         51,51,51,
         TAR_IGNORE, cast_well_of_knowledge);

 SPELLO( 120,12, POSITION_STANDING, 15, 15, 25,
         51,51,51,
         TAR_CHAR_WORLD, cast_farlook);

 SPELLO( 121,12, POSITION_STANDING, 5, 8, 10,
         51,51,51,
         TAR_CHAR_ROOM | TAR_OBJ_INV, cast_dispel_invisible);

 SPELLO( 124,21, POSITION_STANDING, 25, 51, 75,
         51,51,51,
         TAR_CHAR_ROOM | TAR_VIOLENT | TAR_SINGLE, cast_disintegrate);

 SPELLO( 125, 24, POSITION_STANDING,35, 51, 300, 51,
         51, 51, TAR_IGNORE, cast_create_golem);

 SPELLO(180,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
         LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
    TAR_SELF_NONO | TAR_VIOLENT | TAR_IGNORE, 0);
  
}

void SpellWearOffSoon(int s, struct char_data *ch)
{
 
  if (s > MAX_SKILLS+10) 
    return;
 
  if (spell_wear_off_soon_msg[s] && *spell_wear_off_soon_msg[s]) {
    send_to_char(spell_wear_off_soon_msg[s], ch);
    send_to_char("\n\r", ch);
  }
 
  if (spell_wear_off_soon_room_msg[s] && *spell_wear_off_soon_room_msg[s]) {
    act(spell_wear_off_soon_room_msg[s], FALSE, ch, 0, 0, TO_ROOM);
  }
 
}
 
 
void SpellWearOff(int s, struct char_data *ch)
{
 
  if (s > MAX_SKILLS+10) 
    return;
 
  if (spell_wear_off_msg[s] && *spell_wear_off_msg[s]) {
    send_to_char(spell_wear_off_msg[s], ch);
    send_to_char("\n\r", ch);
  }
 
  if (spell_wear_off_room_msg[s] && *spell_wear_off_room_msg[s]) {
    act(spell_wear_off_room_msg[s], FALSE, ch, 0, 0, TO_ROOM);
  }
 
 
  if (s == SPELL_CHARM_PERSON || s == SPELL_CHARM_MONSTER) {
    check_decharm(ch);
  }
 
  if (s == SPELL_WATER_BREATH) {
    check_drowning(ch);
  }
 
}

void check_decharm( struct char_data *ch)
{
  struct char_data *m;
 
  if (!ch->master) return;
 
  m = ch->master;
  stop_follower(ch);   /* stop following the master */
  REMOVE_BIT(ch->specials.act, ACT_SENTINEL);
  AddFeared( ch, m);
  do_flee(ch, "", 0);
 
}

void check_drowning( struct char_data *ch)
{
  struct room_data *rp;
  char buf[256];
 
  if (IS_AFFECTED(ch, AFF_WATERBREATH))
    return;
 
  rp = real_roomp(ch->in_room);
 
  if (!rp) return;
 
  if (rp->sector_type == SECT_UNDERWATER) {
      send_to_char("PANIC!  You're drowning!!!!!!\n\r", ch);
      act("$n flails $s hands and turns a deeper shade of blue as $e is drowning.", FALSE,ch,0,0,TO_ROOM);
     if (!IS_IMMORTAL(ch)) {
      GET_HIT(ch)-=number(1,300);
      GET_MOVE(ch) -= number(10,200);
     }
      update_pos(ch);
      if (GET_HIT(ch) < -10) {
        sprintf(buf, "%s killed by drowning", GET_NAME(ch));
        log(buf);
        if (!ch->desc)
          GET_GOLD(ch) = 0;
        die(ch);
      }
   }
}


int perform_gestural(struct char_data *ch) {

   if (!ch)
      return FALSE;
   
   if (ch->equipment[WIELD]) {
      send_to_char("You cannot perform the required gestures while wielding something!\n\r", ch);
      return FALSE;
   }
   if (ch->equipment[WEAR_SHIELD]) {
      send_to_char("You cannot perform the required gestures while using an item as a shield!\n\r", ch);
      return FALSE;
   }
   
   act("$n traces a magical rune in the air with his hands.", TRUE, ch, 0,
       NULL, TO_ROOM);
   send_to_char("You trace a rune in the air with your hands.\n\r", ch);
   return TRUE;
}


struct obj_data *find_component(struct char_data *ch, int vnum) {
   struct obj_data *item;

   if ((!ch) || !(item = ch->equipment[HOLD]))
      return NULL;

   if (((item->item_number >= 0) ? obj_index[item->item_number].virtual : 0) == vnum)
      return item;

   if (ITEM_TYPE(item) == ITEM_SPELLBAG)
      for (item = item->contains; item; item = item->next_content)
         if (((item->item_number >= 0) ? obj_index[item->item_number].virtual : 0) == vnum)
            return item;
            
   return NULL; 
}
     

int use_component(struct char_data *ch, struct obj_data *o) {
   int strength;
   
   if (!o)
      return 0;
   
   strength = (ITEM_TYPE(o) == ITEM_COMPONENT) ? o->obj_flags.value[0] : 1;
   act("$n throws $p into the air... it explodes in a blast of light!", TRUE,
      ch, o, NULL, TO_ROOM);
   act("You throw $p into the air... it explodes in a blast of light!", TRUE,
      ch, o, NULL, TO_CHAR);
   extract_obj(o);
         
   return strength;
}


#define BASE_CF 10 
#define BASE_CS 10
int task_check(struct char_data *ch, int difficulty, int modifier) {
   int cf, cs, check;
 
   if ((ch) && (IS_IMMORTAL(ch)))
      return CRITICAL_SUCCESS;

   modifier = MIN(modifier, 7); 
   modifier = MAX(modifier, -7);

   switch (difficulty) {
      case TASK_TRIVIAL:   cf = (BASE_CF / 3) - (modifier / 3);
                           cs = (BASE_CS * 3) + (modifier * 3); 
                           break;
      case TASK_EASY:      cf = (BASE_CF / 2) - (modifier / 2);
                           cs = (BASE_CS * 2) + (modifier * 2);
                           break;
      case TASK_DIFFICULT: cf = (BASE_CF * 2) - (modifier * 2);
                           cs = (BASE_CS / 2) + (modifier / 2);
                           break;
      case TASK_DANGEROUS: cf = (BASE_CF * 3) - (modifier * 3);
                           cs = (BASE_CS / 3) + (modifier / 3);
                           break;
      case TASK_NORMAL:
      default:             cf = BASE_CF - modifier; 
                           cs = BASE_CS + modifier;
                           break;
   }
   cs = cf + cs;
   check = dice(1, 100);
   if (cf >= check)
      return CRITICAL_FAILURE;
   if (cs >= check)
      return CRITICAL_SUCCESS;
   return NORMAL_RESULT;
}


int can_do_verbal(struct char_data *ch) {
   struct room_data *rp;
   
   return (ch && !IS_AFFECTED(ch, AFF_SILENT) &&
          (rp=(real_roomp(ch->in_room))) 
          && (!IS_SET(rp->room_flags, SILENCE)));
}


int enforce_verbal(struct char_data *ch) {

   if (!ch)
      return FALSE;
   
   if (!can_do_verbal(ch))   {
      act("$n opens his mouth as if to say something.", TRUE, ch,
         0, NULL, TO_ROOM);
      send_to_char("You are unable to chant the mantra!\n\r", ch);
      return FALSE;
   }

   return TRUE;
}
