/*********************************************************************
*  file: spec_assign.c , Special module.                  Part of DIKUMUD *
*  Usage: Procedures assigning function pointers.                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include "structs.h"
#include "db.h"
#include "utils.h"

#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern void boot_the_shops();
extern void assign_the_shopkeepers();

struct special_proc_entry {
  int vnum;
  int (*proc)( struct char_data *, int, char *);
};

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void) {
   extern int cityguard(struct char_data *ch, int cmd, char *arg);
   extern int craps_table_man(struct char_data *ch, int cmd, char *arg);
   extern int aunt_bee(struct char_data *ch, int cmd, char *arg);
   extern int sheriff(struct char_data *ch, int cmd, char *arg);
   extern int bow_shooter(struct char_data *ch, int cmd, char *arg);
   extern int magneto(struct char_data *ch, int cmd, char *arg);
   extern int ThalosGuildGuard(struct char_data *ch, int cmd, char *arg);
   extern int SultanGuard(struct char_data *ch, int cmd, char *arg);
   extern int NewThalosCitzen(struct char_data *ch, int cmd, char *arg);
   extern int NewThalosMayor(struct char_data *ch, int cmd, char *arg);
   extern int MordGuard(struct char_data *ch, int cmd, char *arg);
   extern int MordGuildGuard(struct char_data *ch, int cmd, char *arg);
   extern int CaravanGuildGuard(struct char_data *ch, int cmd, char *arg);
   extern int StatTeller(struct char_data *ch, int cmd, char *arg);
   extern int ThrowerMob(struct char_data *ch, int cmd, char *arg);
   extern int Demon(struct char_data *ch, int cmd, char *arg);
   extern int Devil(struct char_data *ch, int cmd, char *arg);
   extern int Inquisitor(struct char_data *ch, int cmd, char *arg);
   extern int temple_labrynth_liar(struct char_data *ch, int cmd, char *arg);
   extern int AbyssGateKeeper(struct char_data *ch, int cmd, char *arg);
   extern int postmaster(struct char_data *ch, int cmd, char *arg);
   extern int temple_labrynth_sentry(struct char_data *ch, int cmd, char *arg);
   extern int NudgeNudge(struct char_data *ch, int cmd, char *arg);
   extern int RustMonster(struct char_data *ch, int cmd, char *arg);
   extern int PaladinGuildGuard(struct char_data *ch, int cmd, char *arg);
   extern int tormentor(struct char_data *ch, int cmd, char *arg);
   extern int receptionist(struct char_data *ch, int cmd, char *arg);
   extern int receptionist_for_outlaws(struct char_data *ch, int cmd, char *arg);
   extern int MageGuildMaster(struct char_data *ch, int cmd, char *arg);
   extern int ThiefGuildMaster(struct char_data *ch, int cmd, char *arg);
   extern int ClericGuildMaster(struct char_data *ch, int cmd, char *arg);
   extern int WarriorGuildMaster(struct char_data *ch, int cmd, char *arg);
   extern int AntiGuildMaster(struct char_data *ch, int cmd, char *arg);
   extern int PaladinGuildMaster(struct char_data *ch, int cmd, char *arg);
   extern int RangerGuildMaster(struct char_data *ch, int cmd, char *arg);
   extern int guild_guard(struct char_data *ch, int cmd, char *arg);
   extern int puff(struct char_data *ch, int cmd, char *arg);
   extern int fido(struct char_data *ch, int cmd, char *arg);
   extern int janitor(struct char_data *ch, int cmd, char *arg);
   extern int mayor(struct char_data *ch, int cmd, char *arg);
   extern int eric_johnson(struct char_data *ch, int cmd, char *arg);
   extern int andy_wilcox(struct char_data *ch, int cmd, char *arg);
   extern int zombie_master(struct char_data *ch, int cmd, char *arg);
   extern int snake(struct char_data *ch, int cmd, char *arg);
   extern int thief(struct char_data *ch, int cmd, char *arg);
   extern int monk_master(struct char_data *ch, int cmd, char *arg);
   extern int monk(struct char_data *ch, int cmd, char *arg);
   extern int magic_user(struct char_data *ch, int cmd, char *arg);
   extern int magic_user2(struct char_data *ch, int cmd, char *arg);
   extern int cleric(struct char_data *ch, int cmd, char *arg);
   extern int ghoul(struct char_data *ch, int cmd, char *arg);
   extern int vampire(struct char_data *ch, int cmd, char *arg);
   extern int arch_vampire(struct char_data *ch, int cmd, char *arg);
   extern int wraith(struct char_data *ch, int cmd, char *arg);
   extern int shadow(struct char_data *ch, int cmd, char *arg);
   extern int geyser(struct char_data *ch, int cmd, char *arg);
   extern int green_slime(struct char_data *ch, int cmd, char *arg);
   extern int BreathWeapon(struct char_data *ch, int cmd, char *arg);
   extern int dragon(struct char_data *ch, int cmd, char *arg);
   extern int DracoLich(struct char_data *ch, int cmd, char *arg);
   extern int Drow(struct char_data *ch, int cmd, char *arg);
   extern int Leader(struct char_data *ch, int cmd, char *arg);
   extern int MidgaardCitizen(struct char_data *ch, int cmd, char *arg);
   extern int NewThalosMayor(struct char_data *ch, int cmd, char *arg);
   extern int NewThalosCitizen(struct char_data *ch, int cmd, char *arg);
   extern int citizen(struct char_data *ch, int cmd, char *arg);
   extern int SultanGuard(struct char_data *ch, int cmd, char *arg);
   extern int NewThalosGuildGuard(struct char_data *ch, int cmd, char *arg);
   extern int new_ninja_master(struct char_data *ch, int cmd, char *arg);
   extern int loremaster(struct char_data *ch, int cmd, char *arg);
   extern int hunter(struct char_data *ch, int cmd, char *arg);
   extern int WizardGuard(struct char_data *ch, int cmd, char *arg);
   extern int AbbarachDragon(struct char_data *ch, int cmd, char *arg);
   extern int Tytan(struct char_data *ch, int cmd, char *arg);
   extern int replicant( struct char_data *ch, int cmd, char *arg);
   extern int nightcrawler( struct char_data *ch, int cmd, char *arg);
   extern int regenerator( struct char_data *ch, int cmd, char *arg);
   extern int mega_regenerator( struct char_data *ch, int cmd, char *arg);
   extern int web_slinger( struct char_data *ch, int cmd, char *arg);
   extern int juggernaut( struct char_data *ch, int cmd, char *arg);
   extern int storm( struct char_data *ch, int cmd, char *arg);
   extern int prof_x( struct char_data *ch, int cmd, char *arg);
   extern int elektro( struct char_data *ch, int cmd, char *arg);
   extern int iceman( struct char_data *ch, int cmd, char *arg);
   extern int blink( struct char_data *ch, int cmd, char *arg);
   extern int RepairGuy( struct char_data *ch, int cmd, char *arg);
   extern int Ringwraith( struct char_data *ch, int cmd, char *arg);
   extern int bounty_hunter( struct char_data *ch, int cmd, char *arg);
   extern int sisyphus( struct char_data *ch, int cmd, char *arg);
   extern int jabberwocky( struct char_data *ch, int cmd, char *arg);
   extern int flame( struct char_data *ch, int cmd, char *arg);
   extern int banana( struct char_data *ch, int cmd, char *arg);
   extern int paramedics( struct char_data *ch, int cmd, char *arg);
   extern int delivery_elf( struct char_data *ch, int cmd, char *arg);
   extern int delivery_beast( struct char_data *ch, int cmd, char *arg);
   extern int Keftab( struct char_data *ch, int cmd, char *arg);
   extern int StormGiant( struct char_data *ch, int cmd, char *arg);
   extern int Kraken( struct char_data *ch, int cmd, char *arg);
   extern int Manticore( struct char_data *ch, int cmd, char *arg);
   extern int i_am_police( struct char_data *ch, int cmd, char *arg);
   extern int fighter(struct char_data *ch, int cmd, char *arg);
   extern int zombie_hater(struct char_data *ch, int cmd, char *arg);
   extern int toilet_thing(struct char_data *ch, int cmd, char *arg);
   extern int gilbert(struct char_data *ch, int cmd, char *arg);
   extern int bouncer(struct char_data *ch, int cmd, char *arg);
   extern int dishboy(struct char_data *ch, int cmd, char *arg);
   extern int game_wizard(struct char_data *ch, int cmd, char *arg);
   extern int AGGRESSIVE(struct char_data *ch, int cmd, char *arg);
   extern int CarrionCrawler(struct char_data *ch, int cmd, char *arg);
   extern int guardian(struct char_data *ch, int cmd, char *arg);
   extern int lattimore(struct char_data *ch, int cmd, char *arg);
   extern int coldcaster(struct char_data *ch, int cmd, char *arg);
   extern int trapper(struct char_data *ch, int cmd, char *arg);
   extern int keystone(struct char_data *ch, int cmd, char *arg);
   extern int ghostsoldier(struct char_data *ch, int cmd, char *arg);
   extern int troguard(struct char_data *ch, int cmd, char *arg);
   extern int shaman(struct char_data *ch, int cmd, char *arg);
   extern int golgar(struct char_data *ch, int cmd, char *arg);
   extern int trogcook(struct char_data *ch, int cmd, char *arg);

   struct special_proc_entry specials[] = {

    { 1, puff },
    { 2, Ringwraith },
    { 3, tormentor },
    { 4, Inquisitor},
    { 6, tormentor },
    { 14, bounty_hunter },
    { 15, bounty_hunter },
    { 16, bounty_hunter },
    { 17, bounty_hunter },
    { 18, bounty_hunter },
    { 19, bounty_hunter },
    { 25, magic_user },
    { 30, MageGuildMaster }, 
    { 31, ClericGuildMaster }, 
    { 32, ThiefGuildMaster }, 
    { 33, WarriorGuildMaster },
    { 34, MageGuildMaster }, 
    { 35, ClericGuildMaster }, 
    { 50, i_am_police },
    { 51, i_am_police },
    { 52, i_am_police },
    { 53, i_am_police },
    { 36, ThiefGuildMaster }, 
    { 37, WarriorGuildMaster },
    { 29902, AntiGuildMaster },
    { 29904, RangerGuildMaster },
    { 29905, PaladinGuildMaster },
    {199, AGGRESSIVE},
    {200, AGGRESSIVE},
    {1699, postmaster},
/*
**  D&D standard
*/

    { 210, snake },     /* spider */
    { 211, fighter},       /* gnoll  */
    { 220, fighter},       /* fighter */
    { 221, fighter},       /* bugbear */
    { 223, ghoul },     /* ghoul */
    { 226, fighter },      /* ogre */
    { 236, ghoul },      /* ghast */
    { 227, snake },  /* spider */
    { 230, BreathWeapon }, /* baby black */
    { 232, blink },       /* blink dog */
    { 233, BreathWeapon }, /* baby blue */
    { 234, cleric }, /* cleric */
    { 239, shadow },      /* shadow    */
    { 240, snake },       /* toad      */
    { 243, BreathWeapon }, /* teenage white */
    { 247, fighter}, /* minotaur */
    { 251, CarrionCrawler },
    { 261, fighter },
    { 271, regenerator },
    { 248, snake },       /* snake       */
    { 249, snake },       /* snake       */
    { 250, snake },       /* snake       */
    { 257, magic_user },  /* magic_user  */

    {650, monk},
    {651, monk},
    {652, monk},
    {653, monk},
    {654, monk},
    {655, monk},
    {656, monk},
    {657, monk},
    {658, monk},
    {659, monk},
    {660, monk},
    {661, monk},
    {662, monk},
    {663, monk},
    {664, monk},
    {665, monk},
    {666, monk},
    {667, monk},
    {668, monk},
    {669, monk},
    {670, monk},
    {671, monk},
    {672, monk},
    {673, monk},
    {674, monk},
    {675, monk},
    {676, monk},
    {677, monk},
    {678, monk},
    {679, monk},
    {680, monk},
    {681, monk},
    {682, monk},
    {683, monk},
    {684, monk},
    {685, monk},
    {686, monk},
    {687, monk},
    {688, monk},
    {689, monk},
    {690, monk},
    {691, monk_master},


/*
**  Abyss part II
*/ 
    { 25126, magic_user }, /* Vascar */
    { 25127, cleric}, /* ralthar */
    { 25128, fighter}, /*draco */
    { 25131, Demon}, /*Balrog */
    { 25134, BreathWeapon}, /*rainbow d. */
    { 25147, fighter},
    { 25148, magic_user},
    { 25149, thief},
    { 25150, cleric },
    { 25154, snake }, /*wyvern*/
    { 25156, janitor}, 
    { 25158, magic_user},
    { 25159, magic_user},
    { 25160, magic_user},
    { 25161, fighter},
    { 25162, magic_user},
    { 25164, cleric},
    { 25165, cleric},
    { 25166, magic_user},
    { 25168, magic_user},
/*
**   shire
*/
    { 1000, magic_user},
    { 1010, fighter},
    { 1011, fighter},
    { 1012, fighter},
    { 1014, fighter},
    { 1015, fighter},
    { 1016, fighter},
    { 1017, fighter},
    { 1001, fighter},
    { 1023, fighter},
    { 1031, receptionist },
    { 1701, monk},

    { 1702, monk},
    { 1703, monk},
    { 1704, monk},
    { 1705, monk},
    { 1706, monk},
    { 1707, monk},
    { 1708, monk},
    { 1709, monk},
    { 1710, monk},
    { 1711, monk},
    { 1712, monk},
    { 1713, monk},
    { 1714, monk},
    { 1715, monk},
    { 1716, monk},
    { 1717, monk},
    { 1718, monk},
/*
**  cacaodemons
    { 20, fighter},
    { 21, fighter},
    { 22, fighter},
    { 23, fighter},
*/

/*
**  G1
*/
    { 9213, CarrionCrawler},
    { 9208, cleric },
    { 9217, BreathWeapon},
/*
**  chessboard
*/
    { 1401, fighter}, 
    { 1404, fighter}, 
    { 1406, fighter}, 
    { 1457, fighter}, 
    { 1460, fighter}, 
    { 1462, fighter}, 
    { 1499, sisyphus }, 
    { 1471, paramedics }, 
    { 1470, jabberwocky },
    { 1472, flame }, 
    { 1437, banana }, 
    { 1495, delivery_elf },  
    { 1493, delivery_beast },

/*
The Undead Temple
*/
    { 28801, cleric },
    { 28802, fighter },
    { 28806, AGGRESSIVE },
    { 28808, cleric },
    { 28809, fighter },
    { 28811, fighter },
    { 28813, arch_vampire },



/*
Batopr
*/
    { 28701, bouncer},
    { 28705, magic_user},
    { 28708, cleric},
    { 28711, fighter},
    { 28722, toilet_thing},
    { 28724, dishboy},
    { 28729, game_wizard},
    { 28742, fighter},
    { 28743, gilbert},
    { 28744, fighter},
    { 28745, magic_user},
    { 28746, fighter}, 
    { 28748, magic_user},
    { 11335, fighter},
    { 11340, magic_user},
    { 3008, thief },
    { 3009, thief },

/*
Evils area
*/

    { 28401, web_slinger },
    { 28402, magic_user },
    { 28403, magic_user },
    { 28404, snake },
    { 28405, snake },
    { 28448, dragon },
    { 28449, AbbarachDragon},

/*
Marvel world
*/
    { 29001, fighter },
    { 29002, magic_user },
    { 29003, mega_regenerator },
    { 29004, web_slinger },
    { 29005, magneto },
    { 29006, fighter },
    { 29008, sheriff },
    { 29010, prof_x },
    { 29011, magic_user },
    { 29012, fighter },
    { 29013, zombie_hater },
    { 29014, storm },
    { 29017, elektro },
    { 29021, fighter },
    { 29022, juggernaut },
    { 29024, fighter },
    { 29025, fighter },
    { 29026, fighter },
    { 29027, fighter },
    { 29028, bow_shooter },
    { 29029, fighter },
    { 29030, fighter },
    { 29031, iceman },
    { 29033, nightcrawler },
    { 29039, janitor },
    { 29040, replicant },
    { 29044, fido },
    { 29954, dragon },
    
/*
**  New Thalos
*/
    { 3600, MageGuildMaster },
    { 3601, ClericGuildMaster },
    { 3602, WarriorGuildMaster },
    { 3603, ThiefGuildMaster },
    { 3604, receptionist_for_outlaws},
#if 0
    { 3606, BattleOfEvermoreSinger },
#endif
    { 3619, fighter},
    { 3620, fighter},
    { 3632, fighter},
    { 3634, fighter},
    { 3636, fighter},
    { 3639, fighter}, /* caramon */
    { 3641, cleric},  /* curley g. */
    { 3640, magic_user},  /* raist */
    { 3656, NewThalosGuildGuard},
    { 3657, NewThalosGuildGuard},
    { 3658, NewThalosGuildGuard},
    { 3659, NewThalosGuildGuard},
    { 3661, SultanGuard},   /* wandering */
    { 3662, SultanGuard},   /* not */
    { 3682, SultanGuard},   /* royal */
    { 3670, BreathWeapon},  /* Cryohydra */
    { 3674, BreathWeapon},  /* Behir */
    { 3675, BreathWeapon},  /* Chimera */
    { 3676, BreathWeapon},  /* Couatl */
    { 3681, cleric },       /* High priest */
    { 3689, NewThalosMayor }, /* Guess */
    { 3644, fido},
    { 3635, thief}, 
/*
**  Skexie
*/
    { 15813, magic_user},
    { 15815, magic_user},
    { 15820, magic_user }, 
    { 15821, vampire }, 
    { 15844, cleric },  
    { 15847, fighter }, 
    { 15831, fighter }, 
    { 15832, fighter }, 
    { 15822, fighter }, 
    { 15819, fighter }, 
    { 15805, fighter }, 
/*
**  Challenge
*/
    { 15858, BreathWeapon },
    { 15861, magic_user },
    { 15862, magic_user },
    { 15863, fighter },
    { 15864, sisyphus },


/*
**   Zombie's
*/

    { 23001, fighter },          /* Bob */

/*
**  abyss
*/
    { 25000, magic_user },      /* Demi-lich  */
    { 25001, Keftab }, 
    { 25009, BreathWeapon },    /* hydra */
    { 25002, vampire },         /* Crimson */
    { 25003, StormGiant },      /* MistDaemon */
    { 25006, StormGiant },      /* Storm giant */
    { 25014, StormGiant },      /* DeathKnight */    
    { 25009, BreathWeapon },    /* hydra */
    { 25017, AbyssGateKeeper }, /* Abyss Gate Keeper */
    { 25013, fighter},          /* kalas */

/*
**  Paladin's guild
*/
    { 25100, PaladinGuildGuard},
    { 25101, PaladinGuildGuard},

/*
**  Abyss Fire Giants
*/
    { 25500, fighter },
    { 25501, fighter },
    { 25502, fighter },
    { 25505, fighter },
    { 25504, BreathWeapon},
    { 25503, cleric  },

/*
**  Temple Labrynth
*/

    { 10900, temple_labrynth_liar },
    { 10901, temple_labrynth_liar },
    { 10902, temple_labrynth_sentry},
/*
**  Gypsy Village
*/

    { 16106, fido},
    { 16107, CaravanGuildGuard},
    { 16108, CaravanGuildGuard},
    { 16109, CaravanGuildGuard},
    { 16110, CaravanGuildGuard},
    { 16111, WarriorGuildMaster},
    { 16112, MageGuildMaster},
    { 16113, ThiefGuildMaster},
    { 16114, ClericGuildMaster},
    { 16122, receptionist},
    { 16105, StatTeller},

/*
**  Draagdim
*/

    { 2500, NudgeNudge },  /* jailer */
/*
**  mordilnia
*/
    {18200, magic_user},
    {18205, receptionist},
    {18206, MageGuildMaster},
    {18207, ClericGuildMaster},    
    {18208, ThiefGuildMaster},
    {18209, WarriorGuildMaster},    
    {18210, MordGuildGuard},  /*18266 3*/  
    {18211, MordGuildGuard},  /*18276 1*/
    {18212, MordGuildGuard},  /*18272 0*/
    {18213, MordGuildGuard},  /*18256 2*/
    {18215, MordGuard },    
    {18216, janitor},
    {18217, fido},    
    {18221, fighter},
    {18222, MordGuard},
    {18223, MordGuard},    

/*
**  Graecia:
*/
    {13779, magic_user},
    {13784, magic_user},
    {13785, magic_user},
    {13787, magic_user},
    {13789, magic_user},
    {13791, magic_user},
    {13793, magic_user},
    {13795, magic_user},
    {13797, magic_user},
    
#if 0
/*
**  Eastern Path
*/
    {160, },
    {160, },
    {160, },
    {160, },
    {160, },
    {160, },
    {160, },
    {160, },
    {160, },
    {160, },
/*
**   Ravenloft
*/
    {161, },
    {161, },
    {161, },
    {161, },
    {161, },
    {161, },
    {161, },
    {161, },
    {161, },
    {161, },
    {161, },
#endif
/*
**  Bay Isle
*/
    {16610, Demon},
    {16620, BreathWeapon},
    {16640, cleric},
    {16650, cleric},

#if 0
    {16630, PortalGuard_X},
#endif

/*
**  King's Mountain
*/
    {16700, BreathWeapon},
    {16702, shadow},
    {16703, magic_user},
    {16709, vampire},
    {16710, Devil},
    {16711, Devil},
    {16712, Devil},
    {16713, ghoul},
    {16714, ghoul},
    {16715, wraith},
    {16717, fighter},
    {16720, Devil},
    {16721, Devil},
    {16724, Devil},
    {16725, magic_user},
    {16726, cleric},
    {16727, Devil},
    {16728, Devil},
    {16730, Devil},
    {16731, Devil},
    {16732, Demon},
    {16733, Demon},
    {16734, Demon},
    {16735, Demon},
    {16736, cleric},
    {16738, BreathWeapon},
/*
**  Mages Tower
*/
    {1500, shadow},
    {1504, magic_user},
    {1506, magic_user},
    {1507, magic_user},
    {1508, magic_user},
    {1510, magic_user},
    {1514, magic_user},
    {1515, magic_user},
    {1516, magic_user},
    {1517, magic_user},
    {1518, magic_user},
    {1520, magic_user},
    {1521, magic_user},
    {1522, magic_user},
    {1523, magic_user},
    {1524, magic_user},
    {1525, magic_user},
    {1526, magic_user},
    {1527, magic_user},
    {1528, magic_user},
    {1529, magic_user},
    {1530, magic_user},
    {1531, magic_user},
    {1532, magic_user},
    {1533, magic_user},
    {1534, magic_user},
    {1537, magic_user},
    {1538, magic_user},
    {1540, magic_user},
    {1541, magic_user},
    {1548, magic_user},
    {1549, magic_user},
    {1552, magic_user},
    {1553, magic_user},
    {1554, magic_user},
    {1556, magic_user},
    {1557, magic_user},
    {1559, magic_user},
    {1560, magic_user},
    {1562, magic_user},
    {1564, magic_user},
    {1565, magic_user},
/*
**  Marvel World
*/
 
/*
**  Forest of Rhowyn
*/

    {13901, ThrowerMob },

#if 0
/*
**  Quikland
*/
    {62, },
    {62, },
    {62, },
    {62, },
    {62, },
    {62, },
    {62, },
    {62, },
    {62, },
    {62, },

/*
**  Lycanthropia
*/
    {169, },
    {169, },
    {169, },
    {169, },
    {169, },
    {169, },
    {169, },
    {169, },
    {169, },
    {169, },
#endif
/*
**  Main City
*/

    { 29898, craps_table_man },
    { 29901, aunt_bee },
    { 29899, sheriff },
    { 3000, magic_user }, 
    { 3060, cityguard }, 
    { 3067, cityguard }, 
    { 3061, janitor },
    { 3063, fighter },
    { 3062, fido }, 
    { 3066, fido },
    { 3005, receptionist },
    { 3020, MageGuildMaster }, 
    { 3021, ClericGuildMaster }, 
    { 3022, ThiefGuildMaster }, 
    { 3023, WarriorGuildMaster },

    { 3007, MidgaardCitizen },    /* Sailor */
    { 3024, guild_guard }, 
    { 3025, guild_guard }, 
    { 3026, guild_guard },
    { 3027, guild_guard },
    { 29950, guild_guard },
    { 29951, guild_guard },
    { 29952, guild_guard },
    { 29953, guild_guard },
    { 3070, RepairGuy }, 
    { 3071, RepairGuy },
    { 3069, cityguard },   /* post guard */
    { 3068, new_ninja_master },
    { 3073, loremaster },
    { 3074, hunter },




/*
**  Lower city
*/
    { 3143, mayor },
/*
**   Hammor's Stuff
*/
    { 3900, eric_johnson }, { 3901, andy_wilcox }, { 3950, zombie_master },
    { 3952, BreathWeapon },

/* 
**  MORIA 
*/
    { 4000, snake }, 
    { 4001, snake }, 
    { 4053, snake },

    { 4103, thief }, 
    { 4100, magic_user }, 
    { 4101, regenerator },
    { 4102, snake },

/*
**  Pyramid
*/

    { 5308, RustMonster },
    { 5303, vampire },

/*
**  Arctica
*/
    { 6801, BreathWeapon },
    { 6802, BreathWeapon },
    { 6815, magic_user },
    { 6821, snake },
    { 6824, BreathWeapon },
    { 6825, thief },

/* 
** SEWERS 
*/
    { 7009, fighter},
    { 7006, snake },
    { 7008, snake },
    { 7042, magic_user },  /* naga       */
    { 7040, BreathWeapon },     /* Red    */
    { 7041, magic_user },  /* sea hag    */
    { 7200, magic_user },  /* mindflayer */ 
    { 7201, magic_user },  /* senior     */
    { 7202, magic_user },  /* junior     */

/* 
** FOREST 
*/

    { 6111, magic_user },  /* tree */
    { 6113, snake },
    { 6114, snake },
    { 6112, BreathWeapon }, /* green */
    { 6910, magic_user },

/*
**  Great Eastern Desert
*/
    { 5000, thief }, /* rag. dervish */
    { 5002, snake }, /* coral snake */
    { 5003, snake }, /* scorpion    */
    { 5004, snake }, /* purple worm  */
    { 5014, cleric },   /* myconoid */
    { 5005, BreathWeapon }, /* brass */

/*
**  Drow (edition 1)
*/
    { 5010, magic_user },  /* dracolich */
    { 5104, cleric },
    { 5103, magic_user },  /* drow mage */
    { 5107, cleric },   /* drow mat. mot */
    { 5108, magic_user },  /* drow mat. mot */
    { 5109, cleric },   /* yochlol */

/*
**   Thalos
*/
    { 5200, magic_user },  /* beholder    */

/*
**  Zoo
*/
    { 9021, snake }, /* Gila Monster */

/*
**   Gonge area
*/
     { 23012, magic_user },
     { 23013, cleric },
     { 23014, cleric },
     { 23016, magic_user },
     { 23017, magic_user },
     { 23018, magic_user },
/*
**  Castle Python
*/

    { 11016, receptionist },
    { 11017, NudgeNudge },

/*
**  miscellaneous
*/
    { 9061, vampire},   /* vampiress  */

/*
**  White Plume Mountain
*/

    { 17004, magic_user }, /* gnyosphinx   */
    { 17017, magic_user }, /* ogre magi   */
    { 17014, ghoul },   /* ghoul  */
    { 17009, geyser },  /* geyser  */
    { 17011, vampire }, /* vampire Amelia  */
    { 17002, wraith },  /* wight*/
    { 17005, shadow },  /* shadow */
    { 17010, green_slime }, /* green slime */

/*
**  Arachnos
*/
    { 20001, snake },   /* Young (large) spider */
    { 20003, snake },   /* wolf (giant) spider  */
    { 20005, snake },   /* queen wasp      */
    { 20006, snake },   /* drone spider    */
    { 20010, snake },   /* bird spider     */
    { 20009, magic_user }, /* quasit         */
    { 20014, magic_user }, /* Arachnos        */
    { 20015, magic_user }, /* Ki Rin          */

    { 20002, BreathWeapon }, /* Yevaud */
    { 20017, BreathWeapon }, /* Elder  */
    { 20016, BreathWeapon }, /* Baby   */

#if 0
/*
**   The Darklands
*/

    { 24050, cleric },
    { 24052, magic_user2 }, 
    { 24053, magic_user2 }, 
    { 24054, magic_user2 }, 
    { 24055, magic_user2 }, 
    { 24056, magic_user2 }, 
    { 24057, magic_user2 }, 
    { 24058, magic_user2 }, 
    { 24059, magic_user2 }, 
#endif

/*
**   Abbarach
*/
    { 27001, magic_user },
    { 27002, magic_user },
    { 27003, magic_user },
    { 27004, magic_user },
    { 27005, magic_user },
    { 27006, Tytan },
    { 27007, replicant },
    { 27016, BreathWeapon },
    { 27014, magic_user },
    { 27017, magic_user },
    { 27018, magic_user },
    { 27019, magic_user },
    { -1, NULL },
  };

  int i, rnum;
  char buf[MAX_STRING_LENGTH];

  for (i=0; specials[i].vnum>=0; i++)
     if ((rnum = real_mobile(specials[i].vnum)) < 0) {
        sprintf(buf, "mobile_assign: Mobile %d not found in database.", specials[i].vnum);
        vlog(buf);
     } else
        mob_index[rnum].func = specials[i].proc;

   boot_the_shops();
   assign_the_shopkeepers();
}


/* assign special procedures to objects */
void assign_objects(void) {
   extern int board(Mob *ch, int cmd, char *arg, Obj *me);
   extern int nodrop(Mob *ch, int cmd, char *arg, Obj *me);
   extern int soap(Mob *ch, int cmd, char *arg, Obj *me);
   extern int vorpal(Mob *ch, int cmd, char *arg, Obj *me);
   extern int jive_box(Mob *ch, int cmd, char *arg, Obj *me);
   extern int warMaker(Mob *ch, int cmd, char *arg, Obj *o);
   extern int orbOfDestruction(Mob *ch, int cmd, char *arg, Obj *o);

   obj_index[real_object(3095)].func = board;
   obj_index[real_object(3097)].func = board;
   obj_index[real_object(3098)].func = board;
   obj_index[real_object(3099)].func = board;
   obj_index[real_object(25102)].func = board;
   obj_index[real_object(29992)].func = jive_box;
   obj_index[real_object(21122)].func = nodrop;
   obj_index[real_object(21130)].func = soap;
   obj_index[real_object(7215)].func = warMaker;
   obj_index[real_object(16754)].func = orbOfDestruction;
   InitBoards();
}


/* assign special procedures to rooms */
void assign_rooms(void) {
   extern int dump(Mob *ch, int cmd, char *arg);
   extern int train_station(Mob *ch, int cmd, char *arg);
   extern int pet_shops(Mob *ch, int cmd, char *arg);
   extern int bank (Mob *ch, int cmd, char *arg);
   extern int House(Mob *ch, int cmd, char *arg);
   extern int mirror_room(Mob *ch, int cmd, char *arg);
   extern int Magic_Fountain(Mob *ch, int cmd, char *arg);
   extern int board_room_entrance(Mob *ch, int cmd, char *arg);
   extern int hospital_entrance(Mob *ch, int cmd, char *arg);
   extern int hospital(Mob *ch, int cmd, char *arg);
   extern int Fountain(Mob *ch, int cmd, char *arg);
   extern int Donation(Mob *ch, int cmd, char *arg);
   extern int monk_challenge_prep_room(Mob *ch, int cmd, char *arg);    
   extern int monk_challenge_room(Mob *ch, int cmd, char *arg);
   extern int metahospital(Mob *ch, int cmd, char *arg);
   extern int no_order(Mob *ch, int cmd, char *arg);
   extern int mag_room(Mob *ch, int cmd, char *arg);

   struct special_proc_entry specials[] = {
    {    99,  Donation },
    {   666,  dump },
    {  1750,  monk_challenge_prep_room },
    {  1751,  monk_challenge_room },
    {  3030,  dump },
    {  3196,  hospital },
    {  3197,  hospital_entrance },
    { 13518,  Fountain },
    { 13547,  dump },
    { 28283,  mag_room },
    { 11014,  Fountain },
    { 23067,  Fountain },
    {  5234,  Fountain },
    {  3141,  Fountain },
    {  3606,  Fountain },
    {  3014,  Fountain },
    { 13530,  pet_shops },
    {  2999,  board_room_entrance },
    {   100,  mirror_room },
    { 11301,  mirror_room },
    { 18999,  train_station },
    {  8600,  train_station },
    { 27835,  House },
    { 27836,  House },
    { 27845,  House },
    { 29991,  House },
    { 27910,  House },
    { 27915,  House },
    { 27920,  House },
    { 27921,  House },
    { 27923,  House },
    { 27925,  House },
    { 27930,  House },
    { 27935,  House },
    { 27940,  House },
    { 27985,  House },
    { 27945,  House },
    { 27950,  House },
    { 27955,  House },
    { 27956,  House },
    { 27960,  House },
    { 27965,  House },
    { 27970,  House },
    { 27975,  House },
    { 27980,  House },
    { 27990,  House },
    { 29993,  House },
    { 29992,  House },
    {  2000,  bank },
    { 13521,  bank },
    {  3199,  metahospital },
    {    -1,  NULL },
  };
  int i;
  struct room_data *rp;
  
  for (i=0; specials[i].vnum >= 0; i++)
     if (rp = real_roomp(specials[i].vnum))
        rp->funct = specials[i].proc;
     else 
        vlog("assign_rooms: unknown room");
}
