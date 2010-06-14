//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//  toggle.cc
//  Basic Toggle information
//
//////////////////////////////////////////////////////////////////////////

#include "being.h"
#include "client.h"
#include "handler.h"
#include "low.h"
#include "monster.h"
#include "configuration.h"
#include "person.h"
#include "account.h"
#include "obj_player_corpse.h"

TOGINFO TogIndex[MAX_TOG_INDEX + 1] =
{
  {"", Mob::NONE},      // 0           Leave Blank
  {"Avenger Quest: eligible", Mob::NONE},
  {"Avenger Quest: got rules", Mob::BISHOP_BMOON},
  {"Avenger Quest: hunting troll", Mob::BISHOP_BMOON},
  {"Avenger Quest: killed troll", Mob::TROLL_GIANT},
  {"Avenger Quest: quest finale", Mob::BISHOP_BMOON},    //  5
  {"Avenger Quest: has avenger", Mob::KING_GH},
  {"Avenger Quest: failed", Mob::NONE},
  {"Avenger Quest: penanced", Mob::OLD_WOMAN},
  {"Vindicator Quest: eligible", Mob::NONE},
  {"Vindicator Quest: found blacksmith", Mob::FISTLAND}, // 10
  {"Vindicator Quest: hunting captain", Mob::FISTLAND},
  {"Vindicator Quest: killed captain", Mob::CAPTAIN_RYOKEN},
  {"Vindicator Quest: failed task", Mob::NONE},
  {"Vindicator Quest: right ore", Mob::FISTLAND},
  {"Vindicator Quest: start (2)", Mob::FISTLAND},  // 15
  {"Vindicator Quest: seeking phoenix", Mob::FISTLAND},
  {"Vindicator Quest: phoenix found", Mob::PHOENIX},
  {"Vindicator Quest: hunting demon", Mob::PHOENIX},
  {"Vindicator Quest: killed demon", Mob::NONE},
  {"Vindicator Quest: failed task (2)", Mob::NONE},  // 20
  {"Vindicator Quest: got feather", Mob::PHOENIX},
  {"Vindicator Quest: right feather", Mob::FISTLAND},
  {"Vindicator Quest: has vindicator", Mob::FISTLAND},
  {"Vindicator Quest: rules dishonor", Mob::FISTLAND},
  {"Vindicator Quest: seeking penance", Mob::GM_IRIS},  // 25
  {"Vindicator Quest: got penance object", Mob::NITELLION},
  {"Vindicator Quest: purified", Mob::GM_IRIS},
  {"Ranger 1st Quest: Found Hermit", Mob::HERMIT_GHPARK},

  {"Immortal Quest: On Quest", Mob::NONE},
  {"Silverclaw Quest: find Scar", Mob::SILVERCLAW},  // 30
  {"Silverclaw Quest: gave collar", Mob::SCAR},
  {"Silverclaw Quest: find map", Mob::HOBBIT_ADVENTURER},
  {"Silverclaw Quest: find Gruum", Mob::HOBBIT_ADVENTURER},
  {"Silverclaw Quest: find Warlord", Mob::GRUUM},   
  {"Silverclaw Quest: find Bishop", Mob::GRUUM}, // 35 
  {"Silverclaw Quest: find tablet", Mob::GHOST_BISHOP},
  {"Silverclaw Quest: on solo", Mob::GHOST_BISHOP},
  {"Silverclaw Quest: load tablet", Mob::UNDEAD_CHIEF}, 
  {"Silverclaw Quest: find cloud dragon", Mob::DRAGON_CLOUD},
  {"Silverclaw Quest: find bronze dragon", Mob::DRAGON_BRONZE}, // 40
  {"Silverclaw Quest: find worker dragon", Mob::DRAGON_WORKER},  
  {"Silverclaw Quest: find gold dragon", Mob::DRAGON_GOLD},
  {"Silverclaw Quest: find silver dragon", Mob::DRAGON_SILVER},
  {"Silverclaw Quest: find Raliki", Mob::RALIKI},
  {"Silverclaw Quest: kill Raliki", Mob::RALIKI}, // 45 
  {"Holy Devastator Quest:  find Miser Ben", Mob::CREED},
  {"Holy Devastator Quest:  took bribe", Mob::MISER_BEN},
  {"Holy Devastator Quest:  find opal", Mob::CREED},
  {"Holy Devastator Quest:  do riddle", Mob::CREED},
  {"Holy Devastator Quest:  killed Miser Ben", Mob::MISER_BEN},  // 50
  {"Holy Devastator Quest:  find medicine man", Mob::SPARTAGUS},
  {"Holy Devastator Quest:  searching for crucifix", Mob::MEDICINE_MAN},
  {"Holy Devastator Quest:  found crucifix", Mob::PRIEST_HOLY},
  {"Holy Devastator Quest:  gave crucifix", Mob::MEDICINE_MAN},
  {"Holy Devastator Quest:  killed Spartagus", Mob::SPARTAGUS}, // 55
  {"Holy Devastator Quest:  got wine", Mob::WORKER_WINERY},
  {"Holy Devastator Quest:  answered Taille's riddle", Mob::TAILLE},
  {"Holy Devastator Quest:  seaching for great sword <r>*Natural Load*<1>", Mob::OVERLORD},
  {"Holy Devastator Quest:  find polished wooden ring <r>*Natural Load*<1>", Mob::TAILLE},
  {"Holy Devastator Quest:  forfeit vindicator", Mob::GRIZWALD}, // 60
  {"Holy Devastator Quest:  did not take gang member's deal", Mob::GANGMEMBER_GIBBETT},
  {"Holy Devastator Quest:  took gang member's deal", Mob::GANGMEMBER_GIBBETT},
  {"Holy Devastator Quest:  got deikhan shield info", Mob::ABNOR},
  {"Holy Devastator Quest:  get flower <r>*Natural Load*<1>", Mob::POACHER},
  {"Holy Devastator Quest:  answered assassin's riddle", Mob::ASSASSIN}, // 65
  {"Holy Devastator Quest:  find Lorto", Mob::YOLA},
  {"Holy Devastator Quest:  deceived Lorto", Mob::LORTO},
  {"Holy Devastator Quest:  find Sultress", Mob::ABNOR},
  {"Holy Devastator Quest:  find Bararakna", Mob::JAQUIN},
  {"Holy Devastator Quest:  received dress of rites", Mob::BARARAKNA}, // 70
  {"Holy Devastator Quest:  find Sloth", Mob::SULTRESS},
  {"Holy Devastator Quest:  can get Devastator", Mob::NESMUM},
  {"Holy Devastator Quest:  cheat Miser Ben", Mob::MISER_BEN},
  {"Holy Devastator Quest:  cheat Spartagus", Mob::SPARTAGUS},
  {"Holy Devastator Quest:  cheat Marcus", Mob::MARCUS},    //75
  {"Holy Devastator Quest:  cheat Taille", Mob::TAILLE},
  {"Holy Devastator Quest:  cheat Abnor", Mob::ABNOR},
  {"Holy Devastator Quest:  cheat Sultress", Mob::SULTRESS},
  {"Holy Devastator Quest:  cheat Nesmum", Mob::NESMUM},
  {"Has Skill:  Read Magic", Mob::NONE},  //80
  {"Monk Red Quest: Is Eligible", Mob::MONK_GM_LEVEL50},
  {"Monk Red Quest: Started Quest", Mob::MONK_GM_LEVEL50},
  {"Monk Red Quest: Finished Quest", Mob::NONE},
  {"Monk Red Quest: Has Sash", Mob::MONK_GM_LEVEL50},
  {"Immortal Skill: Stat", Mob::NONE},     //85
  {"Immortal Skill: Logs", Mob::NONE},
  {"Holy Devastator Quest:  find Abnor", Mob::GRIZWALD},
  {"", Mob::NONE},       // toggles 88-96 are open for future immortal toggles
  {"", Mob::NONE},
  {"", Mob::NONE},   //90
  {"", Mob::NONE},
  {"", Mob::NONE},
  {"", Mob::NONE},
  {"", Mob::NONE},
  {"", Mob::NONE},   //95
  {"Holy Devastator Quest:  has Devastator", Mob::CREED},
  {"Spirit of Warrior Quest:  has ancient symbol", Mob::SPIRIT_OF_WARRIOR},  
  {"Spirit of Warrior Quest:  has holy white defender", Mob::SPIRIT_OF_WARRIOR},
  {"Spirit of Warrior Quest:  has moss covered robes", Mob::SPIRIT_OF_WARRIOR},
  {"Sculpture Quest:  find the high priest", Mob::DWARVEN_AMBASSADOR}, //100
  {"Sculpture Quest:  has a sculpture", Mob::DWARVEN_HIGH_PRIEST},
  {"Sculpture Quest:  completed quest already", Mob::LENGE_MERCHANT},
  {"Is Eligible for Skill: Tornado", Mob::NONE},
  {"Has Skill:  Tornado", Mob::GERSARD},
  {"Is Eligible for Skill: Barkskin", Mob::NONE},   //105
  {"Has Skill:  Barkskin", Mob::FIRST_RANGER_BASIC_TRAINER},
  {"Is Eligible for Skill: Earthquake", Mob::NONE},
  {"Has Skill:  Earthquake", Mob::TALAR},
  {"Is Eligible for Skill:  Dual Wield", Mob::NONE},
  {"Has Skill: Dual Wield", Mob::FIRST_RANGER_BASIC_TRAINER},  //110
  {"Is Eligible for Skill:  Shapeshift", Mob::NONE},
  {"Has Skill: Shapeshift", Mob::FIRST_ANIMAL_TRAINER},
  {"Is Eligible for Skill:  Fireball", Mob::NONE},
  {"Has Skill: Fireball", Mob::ASH},
  {"Is Eligible for Skill:  Ice Storm", Mob::NONE},     //115
  {"Find Dark Robes:  Ice Storm", Mob::RAULDOPLIC},
  {"Has Skill:  Ice Storm", Mob::OLD_SAGE},
  {"Is Eligible for Skill:  Stoneskin", Mob::NONE},
  {"Find Granite Signet:  Stoneskin", Mob::DAKINOR},
  {"Has Granite Signet:  Stoneskin", Mob::DAKINOR},  //120
  {"Has Skill:  Stoneskin", Mob::THALIA},
  {"Is Eligible for Skill:  Galvanize", Mob::NONE},
  {"Find Purple Robes:  Galvanize", Mob::BERARDINIS},
  {"Has Skill:  Galvanize", Mob::BERARDINIS},
  {"Is Eligible for Skill:  Powerstone", Mob::NONE},      //125
  {"Find Snakestaff:  Powerstone", Mob::MERRITT},
  {"Has Skill: Powerstone", Mob::MERRITT},
  {"Has Skill: Advanced Kick", Mob::FONG_CHUN},
  {"Has paid toll", Mob::BULGE},
  {"Is Eligible for Skill:  Advanced Kick", Mob::NONE},  //130
  {"Eligible to enter Logrus initiation", Mob::NONE},
  {"Is Eligible for Monk Sash:  White", Mob::NONE},
  {"Has Started Monk Sash Quest:  White", Mob::MONK_GM_LEVEL15},
  {"Has Monk Sash:  White", Mob::HUANG_LO},
  {"Is Eligible for Monk Sash:  Yellow", Mob::NONE},  //135
  {"Has Finished Monk Sash Quest:  Yellow", Mob::MONK_GM_LEVEL15},
  {"Has Monk Sash:  Yellow", Mob::MONK_GM_LEVEL15},
  {"Monk Purple Quest: eligible", Mob::NONE},
  {"Monk Purple Quest: started", Mob::MONK_GM_LEVEL15},
  {"Monk Purple Quest: Lepers Killed:  1", Mob::NONE}, // 140
  {"Monk Purple Quest: Lepers Killed:  2", Mob::NONE},
  {"Monk Purple Quest: Lepers Killed:  3", Mob::NONE},
  {"Monk Purple Quest: Lepers Killed:  4", Mob::NONE},
  {"Monk Purple Quest: complete", Mob::NONE},
  {"Monk Purple Quest: owned", Mob::MONK_GM_LEVEL15}, // 145
  {"Is Eligible for Monk Sash:  Blue", Mob::NONE},
  {"Has Started Monk Sash Quest:  Blue", Mob::MONK_GM_LEVEL40},
  {"Monk Blue Quest Killed Tiger Shark", Mob::NONE},
  {"Has Finished Monk Sash Quest:  Blue", Mob::MONK_GM_LEVEL40},
  {"Has Monk Sash:  Blue", Mob::MONK_GM_LEVEL40},  // 150
  {"Monk Green Quest: eligible", Mob::NONE},
  {"Monk Green Quest: started", Mob::MONK_GM_LEVEL40},
  {"Monk Green Quest: falling", Mob::NONE},
  {"Monk Green Quest: fallen", Mob::NONE},
  {"Rat King Quest: Gave Essence to Rat King", Mob::RAT_KING}, // 155
  {"Monk Green Quest: owned", Mob::MONK_GM_LEVEL40},
  {"Mage Belt Quest: eligible", Mob::NONE},
  {"Mage Belt Quest: started", Mob::MAGE_GM_LEVEL15},
  {"Mage Belt Quest: searching for thread", Mob::MAGE_GM_LEVEL15},
  {"Mage Belt Quest: owned", Mob::MAGE_GM_LEVEL15},  // 160
  {"Has Skill: Catfall", Mob::MONK_GM_LEVEL40},
  {"Ranger 1st Quest: Eligible", Mob::NONE},
  {"Ranger 1st Quest: Started", Mob::HERMIT_GHPARK},
  {"Ranger 1st Quest: Talked to Gnome Gnoble of Farmers", Mob::GNOBLE_FARMER},
  {"Ranger 1st Quest: Talked to Gnome Farmer", Mob::GNOME_FARMER}, // 165
  {"Ranger 1st Quest: Talked to Gnome Male Child", Mob::GNOME_CHILD},
  {"Ranger 1st Quest: Talked to Gnome Farmhand", Mob::GNOME_FARMHAND},
  {"Ranger 1st Quest: Killed John the Rustler", Mob::NONE},
  {"Ranger 1st Quest: Gave Hermit the Gnomish Cattle Hide", Mob::HERMIT_GHPARK},
  {"Ranger 1st Quest: Gave Hermit the Squirrel Pelts", Mob::HERMIT_GHPARK}, // 170
  {"Ranger 1st Quest: Seeking Balcor the Mage", Mob::HERMIT_GHPARK},
  {"Ranger 1st Quest: Got Scroll from Balcor", Mob::BALCOR},
  {"Ranger 1st Quest: Finished", Mob::HERMIT_GHPARK},
  {"Is Eligible for Ranger L14 Quest", Mob::NONE},
  {"Has Started Ranger L14 Quest", Mob::HERMIT_JED}, // 175
  {"Has Seen Kobold Poacher", Mob::POACH_KOBOLD}, 
  {"Seeking Orc Poacher", Mob::HERMIT_JED},
  {"Has Seen Orc Poacher", Mob::POACH_ORC},
  {"Seeking Blind Pygmy Bone Woman", Mob::HERMIT_JED},
  {"Seeking Apple for Bone Woman", Mob::BONE_PYGMY}, // 180
  {"Got Carved Buckle from Bone Woman", Mob::BONE_PYGMY},
  {"Seeking Orc Magi", Mob::HERMIT_JED},
  {"Failed to Kill Orc Magi", Mob::NONE},
  {"Proving Self", Mob::HERMIT_JED},
  {"Killed Orc Magi", Mob::NONE}, // 185
  {"Finished Ranger L14 Quest", Mob::HERMIT_JED},
  {"Holy Devastator Quest: eligible", Mob::NONE},
  {"Mage Robe Quest: eligible", Mob::NONE},
  {"Mage Robe Quest: seeking Druidess", Mob::SIMON_SPELLCRAFTER},
  {"Mage Robe Quest: getting oil", Mob::SIMON_SPELLCRAFTER},  //190
  {"Mage Robe Quest: getting symbol", Mob::SIMON_SPELLCRAFTER},
  {"Mage Robe Quest: getting metal",  Mob::SIMON_SPELLCRAFTER},
  {"Mage Robe Quest: getting fabric", Mob::SIMON_SPELLCRAFTER},
  {"Mage Robe Quest: has mage robe", Mob::SIMON_SPELLCRAFTER},
  {"Ranger L21 Quest: Eligible", Mob::NONE},  //195
  {"Ranger L21 Quest: Started", Mob::RANGER_AMBER},
  {"Ranger L21 Quest: Killed Volcano Cleric", Mob::NONE},
  {"Ranger L21 Quest: Failed to kill Volcano Cleric", Mob::NONE},
  {"Ranger L21 Quest: Proving self L21A", Mob::RANGER_AMBER},
  {"Ranger L21 Quest: Seeking Arden Cleric", Mob::RANGER_AMBER},  //200
  {"Ranger L21 Quest: Killed Arden Cleric", Mob::NONE},
  {"Ranger L21 Quest: Failed to kill Arden Cleric", Mob::NONE},
  {"Ranger L21 Quest: Proving self L21B", Mob::RANGER_AMBER},
  {"Ranger L21 Quest: Seeking Aarakocra Chief", Mob::RANGER_AMBER},
  {"Ranger L21 Quest: Talked to Aarakocra Chief", Mob::CHIEF_AARAKOCRA},  //205
  {"Ranger L21 Quest: Got feathers", Mob::CHIEF_AARAKOCRA},
  {"Ranger L21 Quest: Seeking two rabbit skins", Mob::RANGER_AMBER},
  {"Ranger L21 Quest: Gave one rbbit skin", Mob::RANGER_AMBER},
  {"Ranger L21 Quest: Seeking Bullywug Oil", Mob::RANGER_AMBER},
  {"Ranger L21 Quest: Seeking Bandits in Bullywug", Mob::SHAMAN_BULLY},  //210
  {"Ranger L21 Quest: Got Bullywug Oil", Mob::SHAMAN_BULLY},
  {"Ranger L21 Quest: Finished Ranger L21 Quest", Mob::RANGER_AMBER},
  {"Faction Membership: eligible", Mob::NONE},
  {"Caldonia: bought her a shot of whisky", Mob::CALDONIA},
  {"", Mob::NONE},  // 215
  {"", Mob::NONE},
  {"", Mob::NONE},
  {"", Mob::NONE},
  {"Warrior L40 Quest: Eligible", Mob::NONE},                  
  {"Warrior L40 Quest: Started", Mob::CHIEF_MALE_VILLAGER}, //220
  {"Warrior L40 Quest: Killing Chief", Mob::ELDERLY_SHAMAN},
  {"Warrior L40 Quest: Killing Shaman", Mob::ELDERLY_SHAMAN},
  {"Warrior L40 Quest: Finished Warrior", Mob::CHIEF_MALE_VILLAGER},
  {"Shaman L15 Juju: Eligible", Mob::NONE},
  {"Shaman L15 Juju: Get Thong", Mob::JUJU_SHAMAN_GM}, //225
  {"Shaman L15 Juju: Get Mare Hide", Mob::JUJU_SHAMAN_GM}, //226
  {"Shaman L15 Juju: Get Beaded Necklace", Mob::JUJU_SHAMAN_GM}, //227
  {"Shaman L15 Juju: Received Orb", Mob::JUJU_SHAMAN_GM}, //228
  {"Shaman L15 Juju: Has Juju", Mob::TOTEM_TRADER}, //229
  {"Faction: Has paid registration fee", Mob::FACTION_REGISTRAR}, // 230
  {"Faction: Has created a faction", Mob::FACTION_REGISTRAR},
  {"Shaman Totem Mask: Eligible", Mob::NONE},
  {"Shaman Totem Mask: Started", Mob::GANDOLFO},
  {"Shaman Totem Mask: Find Forsaken", Mob::GANDOLFO},
  {"Shaman Totem Mask: Find wooden plank", Mob::FORSAKEN}, // 235
  {"Shaman Totem Mask: Has sapless wood for Ptok", Mob::FORSAKEN},
  {"Shaman Totem Mask: Find Scaled Hide", Mob::PTOK},
  {"Shaman Totem Mask: Give Gondolfo covered plank", Mob::PTOK},
  {"Shaman Totem Mask: Recover vial of sap from Forsaken", Mob::GANDOLFO},
  {"Shaman Totem Mask: Find Elric for gris gris", Mob::GANDOLFO}, // 240
  {"Shaman Totem Mask: Kill Baron Samedi", Mob::ELRIC},
  {"Shaman Totem Mask: Has Baron Samedi's vision", Mob::SAMEDI},
  {"Shaman Totem Mask: Killed Elric", Mob::ELRIC},
  {"Shaman Totem Mask: Find Ptok to face true evil", Mob::GANDOLFO},
  {"Shaman Totem Mask: Killed Father's Spirit", Mob::PTOK}, // 245
  {"Shaman Totem Mask: Finished", Mob::GANDOLFO},
  {"Perma Death Character", Mob::NONE},
  {"Psionicist", Mob::NONE},
  {"Dragon Armor: red scales", 13732},
  {"Dragon Armor: green scales", 13732}, //250
  {"Dragon Armor: white scales", 13732},
  {"Dragon Armor: tungsten suit", 13732},
  {"Dragon Armor: huge opal", 13732},
  {"Dragon Armor: 1 vial of dragon bone", 13732},
  {"Dragon Armor: 2 vials of dragon bone", 13732}, //255
  {"Dragon Armor: 3 vials of dragon bone", 13732},
  {"Dragon Armor: 4 vials of dragon bone", 13732},
  {"Dragon Armor: 5 vials of dragon bone", 13732},
  {"Dragon Armor: sinew", 13732},
  {"Dragon Armor: thread element", 13732},  //260
  {"Dragon Armor: talens", 13732},
  {"Dragon Armor: warhammer", 13732},
  {"Dragon Armor: scales", 13732},
  {"Bitten by vampire", Mob::NONE},
  {"Vampire", Mob::NONE},  //265
  {"Lycanthrope", Mob::NONE},
  {"Transformed Lycanthrope", Mob::NONE},
  {"Lycanthropy: virgin flask", 9323},
  {"Lycanthropy: terfefly", 23291},
  {"Lycanthropy: ingredients", 6762}, //270
  {"Lycanthropy: urine", 6762},
  {"Lycanthropy: wolvesbane", 6762},
  {"Lycanthropy: silver", 6762},
  {"Lycanthropy: hemlock", 6762},
  {"BlahBlah Shouts", Mob::NONE},
  {"Monk: Paid Tabuda", Mob::NONE},
  {"Has Skill: Catleap", Mob::NONE},
  {"Trait: Coward", Mob::NONE},
  {"Trait: Blind", Mob::NONE},
  {"Trait: Mute", Mob::NONE},
  {"Trait: Deaf", Mob::NONE},
  {"Trait: Asthmatic", Mob::NONE},
  {"Trait: Necrophobic", Mob::NONE},
  {"Trait: Narcoleptic", Mob::NONE},
  {"Trait: Combustible", Mob::NONE},
  {"Trait: Hemophiliac", Mob::NONE},
  {"Trait: Ambidextrous", Mob::NONE},
  {"Trait: Disease Resistant", Mob::NONE},
  {"Trait: Nightvision", Mob::NONE},
  {"Trait: Alcoholic", Mob::NONE},
  {"Trait: Tourettes", Mob::NONE},
  {"PK Character", Mob::NONE},
  {"Lightsaber quest: gave ruby", 13745},
  {"Lightsaber quest: gave bluesteel", 13745},
  {"Lightsaber quest: gave essence", 13745},
  {"Lightsaber quest: gave rockfish", 13745},
  {"Lightsaber quest: gave essence", 13745},
  {"Creation Choice: Fae Touched", Mob::NONE}, 
  {"Trait: Real Aging", Mob::NONE},
  {"No Experience Gain", Mob::NONE},
  {"Practices Fixed", Mob::NONE},
  {"Has Right Pegleg", Mob::NONE},
  {"Has Left Pegleg", Mob::NONE},
  {"Has Right Hook Hand", Mob::NONE},
  {"Has Left Hook Hand", Mob::NONE},
  {"Cyclops Camp Quest: killed paladin", Mob::NONE},
  {"February Quest: Gave 1 Candy Heart", Mob::GYPSY_ROMANTIC},  //307
  {"February Quest: Gave 2 Candy Hearts", Mob::GYPSY_ROMANTIC},
  {"February Quest: Gave 3 Candy Hearts", Mob::GYPSY_ROMANTIC},
  {"February Quest: Gave 4 Candy Hearts", Mob::GYPSY_ROMANTIC}, //310
  {"February Quest: Gave 1 box of chocolate", Mob::FLORIST_BUSY},
  {"February Quest: Gave 2 boxes of chocolates", Mob::FLORIST_BUSY},
  {"February Quest: Gave 3 boxes of chocolates", Mob::FLORIST_BUSY},
  {"February Quest: Gave 4 boxes of chocolates", Mob::FLORIST_BUSY},
  {"February Quest: Gave 5 boxes of chocolates", Mob::FLORIST_BUSY}, //315
  {"February Quest: Gave 1 tulip", Mob::WIFE_FARMER},
  {"February Quest: Gave 2 tulips", Mob::WIFE_FARMER},
  {"February Quest: Gave 3 tulips", Mob::WIFE_FARMER},
  {"February Quest: Gave 4 tulips", Mob::WIFE_FARMER},
  {"February Quest: Gave 5 tulips", Mob::WIFE_FARMER}, //320
  {"February Quest: Gave 1 bouquet", Mob::CLOWN_SCARY}, 
  {"February Quest: Gave 2 bouquets", Mob::CLOWN_SCARY},
  {"February Quest: Gave 3 bouquets", Mob::CLOWN_SCARY},
  {"February Quest: Gave 4 bouquets", Mob::CLOWN_SCARY},
  {"February Quest: Gave 5 bouquets", Mob::CLOWN_SCARY},//325
  {"February Quest: Gave 1 balloon", Mob::MIME_ANIMATED},
  {"February Quest: Gave 2 balloons", Mob::MIME_ANIMATED},
  {"February Quest: Gave 3 balloons", Mob::MIME_ANIMATED},
  {"February Quest: Gave 4 balloons", Mob::MIME_ANIMATED},
  {"Newbie: Found Yun the Grocer", Mob::ANY}, //330
  {"February Quest: Gave 1 teddy bear", Mob::GUY_LAZY},
  {"February Quest: Gave 2 teddy bears", Mob::GUY_LAZY},
  {"February Quest: Gave 3 teddy bears", Mob::GUY_LAZY},
  {"February Quest: Gave 4 teddy bears", Mob::GUY_LAZY},
  {"Newbie: Found Surplus", Mob::ANY}, //335
  {"February Quest: Gave 1 bottle of port", Mob::DRUNK_TRADER_RICH},
  {"February Quest: Gave 2 bottles of port", Mob::DRUNK_TRADER_RICH},//337
  {"October Quest: Participant", Mob::NONE}, //338
  {"December Quest: Gave 1 candy cane", Mob::FAT_ELF_1},//339
  {"December Quest: Gave 2 candy canes", Mob::FAT_ELF_1},//340
  {"December Quest: Gave 3 candy canes", Mob::FAT_ELF_1},//341
  {"December Quest: Gave 4 candy canes", Mob::FAT_ELF_1},//342
  {"December Quest: Gave 1 jingle bell collar", Mob::FAT_ELF_2}, //343
  {"December Quest: Gave 2 jingle bell collars", Mob::FAT_ELF_2}, //344
  {"December Quest: Gave 3 jingle bell collars", Mob::FAT_ELF_2}, //345
  {"December Quest: Gave 4 jingle bell collars", Mob::FAT_ELF_2}, //346
  {"December Quest: Gave 1 scroll case", Mob::FAT_ELF_3}, //347
  {"December Quest: Gave 2 scroll cases", Mob::FAT_ELF_3}, //348
  {"December Quest: Gave 3 scroll cases", Mob::FAT_ELF_3}, //349
  {"December Quest: Gave 4 scroll cases", Mob::FAT_ELF_3}, //350
  {"Practice Reset: Level15", Mob::ANY}, //351
  {"Practice Reset: Level40", Mob::ANY}, //352
  {"Practice Reset: Level50", Mob::ANY}, //353
  {"February Quest: Gave 3 bottles of port", Mob::DRUNK_TRADER_RICH},//354
  {"February Quest: Gave 4 bottles of port", Mob::DRUNK_TRADER_RICH},//355
  {"Trait: Pyrophobia", Mob::NONE}, // 356
  {"Trait: Vicious", Mob::NONE}, // 357
  {"Trait: Craven", Mob::NONE}, // 358
  {"Newbie: Doing Newbie Quest", Mob::ANY}, //359
  {"Newbie: Finding Class Trainer", Mob::ANY}, //360
  {"Newbie: Found Class Trainer", Mob::ANY}, //361
  {"Newbie: Finding Secondary Basic Trainer", Mob::ANY}, //362
  {"Newbie: Found Secondary Basic Trainer", Mob::ANY}, //363
  {"Newbie: Running First Errand", Mob::ANY}, //364
  {"Newbie: Ran First Errand", Mob::ANY}, //365
  {"Newbie: Running Second Errand", Mob::ANY}, //366
  {"Newbie: Ran Second Errand", Mob::ANY}, //367
  {"Newbie: Fighting Mouse", Mob::ANY}, //368
  {"Newbie: Killed Mouse", Mob::ANY}, //369
  {"Newbie: Has Completed Newbie Quest", Mob::ANY}, //370
  {"", Mob::NONE}, 
};



const char *on_or_off(bool tog){
  if(tog)
    return "<G>on <1>";
  else
    return "<R>off<1>";
}

sstring int_on_or_off(bool tog, int i){
  if(tog)
    return format("<G>%-4i<1>") % i;
  else
    return "<R>off <1>";
}

void updateCorpseLootFlags(const sstring &name, bool lootable)
{
  TPCorpse *pcorpse;

  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
    if((pcorpse=dynamic_cast<TPCorpse *>(*iter))){
      vlogf(LOG_PEEL, format("name=%s") % pcorpse->name);
      
      if((sstring)pcorpse->name == ((sstring) (format("corpse %s pcorpse") % name))){
	if(!lootable){
	  pcorpse->addCorpseFlag(CORPSE_DENY_LOOT);
	  REMOVE_BIT(pcorpse->obj_flags.wear_flags, ITEM_TAKE);
	} else {
	  pcorpse->remCorpseFlag(CORPSE_DENY_LOOT);
	  SET_BIT(pcorpse->obj_flags.wear_flags, ITEM_TAKE);
	}
      }
    }
  }
}


void closeClientConnections(TBeing *me)
{
  me->sendTo("Severing current client connections.\n\r");
  Descriptor *d, *dn;
  for (d = descriptor_list; d; d = dn) {
    dn = d->next;
    if (d->m_bIsClient) {
      d->writeToQ("Link severed by admin.\n\r");
      me->sendTo(COLOR_MOBS, format("Disconnecting client use by %s.\n\r") % 
		 (d->character ? d->character->getName() : "Unknown"));
      delete d;
    }
  }
}



void TBeing::doToggle(const char *arg2)
{
  char arg[256];

  TPerson *ch = dynamic_cast<TPerson *>(this);
  if (!(ch || (specials.act & ACT_POLYSELF))) {
    sendTo("Dumb monsters cannot toggle!\n\r");
    return;
  }

  for (; isspace(*arg2); arg2++);
  arg2=one_argument(arg2, arg, cElements(arg));
  for (; isspace(*arg2); arg2++);


  if (!*arg) {
    sendTo(COLOR_BASIC, "\n\r<c>Player Toggles<1>\n\r");
    sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
    
    const int width = 3;
    int printed = 0;
    
    // print regular toggles from MAX_AUTO
    for (int iAuto = 0; iAuto < MAX_AUTO; iAuto++)
    {
      if (!*auto_name[iAuto])
        continue;
      sendTo(COLOR_BASIC, format("%-17s : %s%s") % auto_name[iAuto] % on_or_off(IS_SET(desc->autobits, (unsigned)(1<<iAuto))) % ((++printed % width) ? "  | " : "\n\r"));
    }

    // toggles not represented by MAX_AUTO
    sendTo(COLOR_BASIC, format("Wimpy             : %s%s") % int_on_or_off(getWimpy(), getWimpy()).c_str() % ((++printed % width) ? " | " : "\n\r"));   
    sendTo(COLOR_BASIC, format("Deny Corpse Loot  : %s%s") % on_or_off(isPlayerAction(PLR_DENY_LOOT)) % ((++printed % width) ? "  | " : "\n\r"));
    sendTo(COLOR_BASIC, format("Newbie Helper     : %s%s") % on_or_off(isPlayerAction(PLR_NEWBIEHELP)) % ((++printed % width) ? "  | " : "\n\r"));
    sendTo(COLOR_BASIC, format("Anonymous         : %s\n\r") % on_or_off(isPlayerAction(PLR_ANONYMOUS)));

    // terminal toggles
    static const char* termnames[TERM_MAX] = { "none ", "vt100", "ansi "};
    int playerTerm = ansi() ? TERM_ANSI : vt100() ? TERM_VT100 : TERM_NONE;
    sendTo(COLOR_BASIC, "\n\r<c>Terminal Toggles<1>\n\r");
    sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
    sendTo(COLOR_BASIC, format("Screensize        : <G>%-3i<1>  | ") % int(desc->screen_size));
    sendTo(COLOR_BASIC, format("Terminal          : <G>%-5s<1>| ") % termnames[playerTerm]);
    sendTo(COLOR_BASIC, format("Boss Mode         : %s\n\r") % on_or_off(IS_SET(desc->account->flags, TAccount::BOSS)));
    sendTo(COLOR_BASIC, format("MSP Sound         : %s  | ") % on_or_off(IS_SET(desc->account->flags, TAccount::MSP)));
    sendTo(COLOR_BASIC, format("Account Terminal  : <G>%-5s<1>| ") % termnames[desc->account->term]);
    sendTo(COLOR_BASIC, format("Allow Pinging     : %s\n\r") % on_or_off(isPlayerAction(PLR_PING)));
    sendTo(COLOR_BASIC, format("Brief             : %s  | ") % on_or_off(isPlayerAction(PLR_BRIEF)));
    sendTo(COLOR_BASIC, format("Compact           : %s  | ") % on_or_off(isPlayerAction(PLR_COMPACT)));
    sendTo(COLOR_BASIC, format("Show Saves        : %s\n\r") % on_or_off(isPlayerAction(PLR_SHOW_SAVES)));

    // immortal toggles
    if(isImmortal() || GetMaxLevel() >= GOD_LEVEL1){
      sendTo(COLOR_BASIC, "\n\r<c>Immortal Toggles<1>\n\r");
      sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
      
	    sendTo(COLOR_BASIC, format("Invisibility      : %s | ") % int_on_or_off(getInvisLevel(), getInvisLevel()).c_str());    
      sendTo(COLOR_BASIC, format("Auto Success      : %s  | ") % on_or_off(IS_SET(desc->autobits, AUTO_SUCCESS)));
      sendTo(COLOR_BASIC, format("Stealth Mode      : %s\n\r") % on_or_off(isPlayerAction(PLR_STEALTH)));
      sendTo(COLOR_BASIC, format("No Hassle         : %s  | ") % on_or_off(isPlayerAction(PLR_NOHASSLE)));
      sendTo(COLOR_BASIC, format("Immortality       : %s\n\r") % on_or_off(isPlayerAction(PLR_IMMORTAL)));
    }

    if (hasWizPower(POWER_TOGGLE)){
      sendTo(COLOR_BASIC, "\n\r<c>Global Toggles<1>\n\r");
      sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
      int i=0;
      for(togTypeT t=TOG_NONE;t<MAX_TOG_TYPES;t++){
	      if(toggleInfo[t]->testcode || t==TOG_NONE)
	        continue;

	      sendTo(COLOR_BASIC, format("%-17s : %s%s") %
	       toggleInfo[t]->name %
	       on_or_off(toggleInfo[t]->toggle) %
	       ((++i%width) ? "  | " : "\n\r"));
      }
      if(i%width)
	      sendTo("\n\r");

      sendTo(COLOR_BASIC, "\n\r<c>Test Code Toggles<1>\n\r");
      sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
      
      i=0;
      for(togTypeT t=TOG_NONE;t<MAX_TOG_TYPES;t++){
	      if(!toggleInfo[t]->testcode || t==TOG_NONE)
	        continue;

	      sendTo(COLOR_BASIC, format("%-17s : %s%s") %
	       toggleInfo[t]->name %
	       on_or_off(toggleInfo[t]->toggle) %
	       ((++i%width) ? "  | " : "\n\r"));
      }
      if(i%width)
	      sendTo("\n\r");

    } else {
      // mortals are always asking if double exp is on, so just let them see it
      // in the toggle list
      sendTo(COLOR_BASIC, "\n\r<c>Global Toggles<1>\n\r");
      sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
      sendTo(COLOR_BASIC, format("%-17s : %s\n\r") %
	     toggleInfo[TOG_DOUBLEEXP]->name %
	     on_or_off(toggleInfo[TOG_DOUBLEEXP]->toggle));
    }

    return;
  } else if(is_abbrev(arg, "deny-corpse-loot")){
    if (!isPlayerAction(PLR_DENY_LOOT)) {
      sendTo("No one may loot your corpse now, except you.\n\r");
      addPlayerAction(PLR_DENY_LOOT);
      updateCorpseLootFlags(getName(), false);
    } else {
      sendTo("Anyone may loot your corpse now.\n\r");
      remPlayerAction(PLR_DENY_LOOT);
      updateCorpseLootFlags(getName(), true);
    }
  } else if(is_abbrev(arg, "compact")){
    if (isPlayerAction(PLR_COMPACT)) {
      sendTo("You are now in the uncompacted mode.\n\r");
      remPlayerAction(PLR_COMPACT);
    } else {
      sendTo("You are now in compact mode.\n\r");
      addPlayerAction(PLR_COMPACT);
    }
  } else if(is_abbrev(arg, "showsaves")){
    if (isPlayerAction(PLR_SHOW_SAVES)) {
      sendTo("You will no longer receive pfile save notification.\n\r");
      remPlayerAction(PLR_SHOW_SAVES);
    } else {
      sendTo("You will now receive pfile save notification.\n\r");
      addPlayerAction(PLR_SHOW_SAVES);
    }
  } else if(is_abbrev(arg, "brief")){
    if (isPlayerAction(PLR_BRIEF)) {
      sendTo("Brief mode disabled.\n\r");
      remPlayerAction(PLR_BRIEF);
    } else {
      sendTo("Brief mode enabled.\n\r");
      addPlayerAction(PLR_BRIEF);
    }
  } else if(is_abbrev(arg, "ping")){
    if (isPlayerAction(PLR_PING)){
      remPlayerAction(PLR_PING);
      act("You will no longer be pinged by the mud.",
	  FALSE, this, 0, 0, TO_CHAR);
    } else {
      addPlayerAction(PLR_PING);
      act("You will now be pinged by the mud.",
	  FALSE, this, 0, 0, TO_CHAR);
    }
  } else if(is_abbrev(arg, "immortal") && GetMaxLevel() >= GOD_LEVEL1){
    if (GetMaxLevel() <= MAX_MORT)
      return;
    if (isPlayerAction(PLR_IMMORTAL)) {
      remPlayerAction(PLR_IMMORTAL);
      setMoney(0);
    } else {
      addPlayerAction(PLR_IMMORTAL);
      setMoney(100000);
    }
    doCls(false);
    if (isImmortal())
      sendTo("You are now immortal.\n\r");
    else
      sendTo("Playing as a mortal now.\n\r");
  } else if(is_abbrev(arg, "nohassle") && isImmortal()){
    TBeing *vict;
    TObj *dummy;

    if (!*arg2){
      if (isPlayerAction(PLR_NOHASSLE)) {
	sendTo("You can now be hassled again.\n\r");
	remPlayerAction(PLR_NOHASSLE);
      } else {
	sendTo("From now on, you won't be hassled.\n\r");
	addPlayerAction(PLR_NOHASSLE);
      }
    } else if (!generic_find(arg2, FIND_CHAR_WORLD, this, &vict, &dummy)){
      sendTo("Couldn't find any such creature.\n\r");
    } else if (dynamic_cast<TMonster *>(vict)) {
      sendTo("Can't do that to a beast.\n\r");
    } else if (vict->GetMaxLevel() > GetMaxLevel()) {
      act("$E might object to that.. better not.", 0, this, 0, vict, TO_CHAR);
    } else
      sendTo("The implementor won't let you set this on mortals...\n\r");
  } else if(is_abbrev(arg, "stealth") && isImmortal()){
    if (isPlayerAction(PLR_STEALTH)) {
      sendTo("STEALTH mode OFF.\n\r");
      remPlayerAction(PLR_STEALTH);
      if (desc && desc->m_bIsClient)
	desc->clientf(format("%d|%d") % CLIENT_STEALTH % FALSE);
    } else {
      sendTo("STEALTH mode ON.\n\r");
      addPlayerAction(PLR_STEALTH);
      if (desc && desc->m_bIsClient)
	desc->clientf(format("%d|%d") % CLIENT_STEALTH % TRUE);
    }
  } else if (is_abbrev(arg, "newbiehelper") ||
	     is_abbrev(arg, "helper")) {
    if (isPlayerAction(PLR_NEWBIEHELP)) {
      remPlayerAction(PLR_NEWBIEHELP);
      act("You just removed your newbie-helper flag",
	  FALSE, this, 0, 0, TO_CHAR);
    } else {
      addPlayerAction(PLR_NEWBIEHELP);
      act("You just set your newbie-helper flag.",
	  FALSE, this, 0, 0, TO_CHAR);
    }
  } else if (is_abbrev(arg, "anonymous")){
    if(GetMaxLevel()<5){
      sendTo("You must be at least level 5 to go anonymous.\n\r");
      return;
    }
    if (isPlayerAction(PLR_ANONYMOUS)){
      remPlayerAction(PLR_ANONYMOUS);
      act("You are no longer anonymous.",
	  FALSE, this, 0, 0, TO_CHAR);
    } else {
      addPlayerAction(PLR_ANONYMOUS);
      act("You are now anonymous.",
	  FALSE, this, 0, 0, TO_CHAR);
    }
  } else if (is_abbrev(arg, "invisibility") && isImmortal()){
    int level;

    if (!hasWizPower(POWER_TOGGLE_INVISIBILITY)) {
      sendTo("The invis command has been disabled due to overuse.\n\r");
      sendTo("Talk to a more powerful god if you need this power enabled temporarily.\n\r");
      return;
    }
    if((level=convertTo<int>(arg2))){
      if (level < 0)
	level = 0;
      else if (level > GetMaxLevel())
	level = GetMaxLevel();
      setInvisLevel(level);
      sendTo(format("Invis level set to %d.\n\r") % level);
      fixClientPlayerLists(TRUE);
    } else {
      if (getInvisLevel() > 0) {
	setInvisLevel(0);
	sendTo("You are now totally visible.\n\r");
	fixClientPlayerLists(FALSE);
      } else {
	setInvisLevel(GOD_LEVEL1);
	sendTo("You are now invisible to all but gods.\n\r");
	fixClientPlayerLists(TRUE);
      }
    }
  } else if (is_abbrev(arg, "wimpy")) {

    int wimplimit = maxWimpy();
    int cravenlimit = wimplimit/2 + wimplimit%2;
    int num=0;

    if (hasQuestBit(TOG_IS_VICIOUS)) {
      sendTo("Wimpy!?  You are too vicious to even contemplate such an idea!\n\r");
      setWimpy(0);
      return;
    } else if (is_abbrev(arg2, "max") || hasQuestBit(TOG_IS_COWARD)) {
      if(hasQuestBit(TOG_IS_COWARD)){
        sendTo("You can't change your wimpy setting, you're a coward!\n\r");
      } else {
        sendTo(format("Setting Wimpy to Max(%d).\n\r") % (wimplimit - 1));
      }
      num = wimplimit - 1;
    } else if (is_abbrev(arg2, "off") || (num = convertTo<int>(arg2)) <= 0) {
      if(hasQuestBit(TOG_IS_CRAVEN)){
        sendTo("You just can't contemplate never fleeing from danger!\n\r");
        num = max(num, cravenlimit);
      } else {
        sendTo("Turning wimpy mode off.\n\r");
        setWimpy(0);
        return;
      }
    }
    
    if (hasQuestBit(TOG_IS_CRAVEN) && num < cravenlimit){
      sendTo("You just can't muster the bravery to fight that long!\n\r");
      num = cravenlimit;
    }

    if ((num < 0) || (wimplimit <= num)) {
      sendTo(format("Please enter a number between 0-%d.\n\r") % (wimplimit-1));
      return;
    }
    
    sendTo("You are now a wimp!!\n\r");
    sendTo(format("You will now flee at %d hit points!\n\r") % num);
    setWimpy(num);
  } else if (is_abbrev(arg, "boss")) {
    if (!IS_SET(desc->account->flags, TAccount::BOSS)) {
      SET_BIT(desc->account->flags, TAccount::BOSS);
      sendTo("You are now in boss mode.\n\r");
    } else {
      REMOVE_BIT(desc->account->flags, TAccount::BOSS);
      sendTo("You are no longer in boss mode.\n\r");
    }
    desc->saveAccount();
  } else if (is_abbrev(arg, "msp")) {
    if (!IS_SET(desc->account->flags, TAccount::MSP)) {
      SET_BIT(desc->account->flags, TAccount::MSP);
      sendTo("MUD Sound Protocol enabled.\n\r");
      // we need to set the default download directory, so do that by doing
      // a stopsound which will transmit the MSP U= command
      stopsound();
    } else {
      REMOVE_BIT(desc->account->flags, TAccount::MSP);
      sendTo("MUD Sound Protocol disabled.\n\r");
    }
    desc->saveAccount();
  } else if (is_abbrev(arg, "account")){
    if (*arg2) {
      if (is_abbrev(arg2, "ansi")) {
	desc->account->term = TERM_ANSI;
	sendTo("Account is now set to ansi.\n\r");
	desc->saveAccount();
      } else if (is_abbrev(arg2, "vt100")) {
	desc->account->term = TERM_VT100;
	sendTo("Account is now set to vt100.\n\r");
	desc->saveAccount();
      } else if (is_abbrev(arg2, "none")) {
	desc->account->term = TERM_NONE;
	sendTo("Account is now set to none.\n\r");
	desc->saveAccount();
      } else {
	sendTo("Syntax: tog account <ansi | vt100 | none>\n\r");
      }
    }
  } else if (is_abbrev(arg, "terminal")){
    if (*arg2){
      if(is_abbrev(arg2, "ansi")){
        if (!IS_SET(desc->prompt_d.type, PROMPT_VTANSI_BAR))
          SET_BIT(desc->prompt_d.type, PROMPT_VTANSI_BAR);
        cls();
        sendTo(format(VT_MARGSET) % 1 % (getScreen() - 3));
        addPlayerAction(PLR_ANSI);
        if (vt100())
          remPlayerAction(PLR_VT100);
        doCls(false);
        sendTo("Setting term type to Ansi...\n\r");	
      } else if(is_abbrev(arg2, "vt100")){
        if (!IS_SET(desc->prompt_d.type, PROMPT_VTANSI_BAR))
          SET_BIT(desc->prompt_d.type, PROMPT_VTANSI_BAR);
        sendTo(format(VT_MARGSET) % 1 % (getScreen() - 3));
        addPlayerAction(PLR_VT100);
        if (ansi())
          remPlayerAction(PLR_ANSI);

        doCls(false);
        sendTo("Setting term type to vt100...\n\r");
      } else if(is_abbrev(arg2, "none")){
        cls();
        fullscreen();
        if (ansi())
          remPlayerAction(PLR_ANSI);

        if (vt100())
          remPlayerAction(PLR_VT100);

        doCls(false);
        sendTo("Setting term type to NONE...\n\r");
      }
    } else {
      sendTo("Syntax: tog terminal <ansi|vt100|none>\n\r");
    }
  } else if (is_abbrev(arg, "screensize")) {
    if (*arg2) {
      if (isdigit(*arg2)) {
	desc->screen_size = min(128, convertTo<int>(arg2));
	doCls(false);
	sendTo(format("Your screensize has been set to: %d\n\r") % desc->screen_size);
      } else {
	sendTo(format("Your current screensize is set to: %d\n\r") % desc->screen_size);
          sendTo("Screensize needs to be a number from 1-128.\n\r");
      }
    } else {
      sendTo(format("Your current screensize is set to: %d\n\r") % desc->screen_size);
    }
  } else if (is_abbrev(arg, "autokill") || is_abbrev(arg, "kill")) {
    if (IS_SET(desc->autobits, AUTO_KILL)) {
      sendTo("You will stop attacking stunned creatures.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_KILL);
    } else {
      sendTo("You will continue attacking stunned (and no chance to recover) creatures.\n\r");
      SET_BIT(desc->autobits, AUTO_KILL);
    }
  } else if (is_abbrev(arg, "noshout") || is_abbrev(arg, "shout")) {
    if (IS_SET(desc->autobits, AUTO_NOSHOUT)) {
      sendTo("You can now hear shouts again.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_NOSHOUT);
    } else {
      sendTo("From now on, you won't hear shouts.\n\r");
      SET_BIT(desc->autobits, AUTO_NOSHOUT);
    }
  } else if (is_abbrev(arg, "noPG13") || is_abbrev(arg, "PG13")) {
    if (IS_SET(desc->autobits, AUTO_PG13)) {
      sendTo("You can now hear vulgarity again.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_PG13);
    } else {
      sendTo("From now on you won't hear vulgarity.\n\r");
      SET_BIT(desc->autobits, AUTO_PG13);
    }
  } else if (is_abbrev(arg, "noharm") || is_abbrev(arg, "harm")) {
    if (IS_SET(desc->autobits, AUTO_NOHARM) && (GetMaxLevel() >= 5)) {
      sendTo("You may now attack other players.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_NOHARM);
    } else if (GetMaxLevel() >= 5) {
      sendTo("You will no longer INTENTIONALLY attack other players.\n\r");
      SET_BIT(desc->autobits, AUTO_NOHARM);
    } else {
      sendTo("You cannot toggle your nokill flag until level 5.\n\r");
      return;
    }
  } else if (is_abbrev(arg, "nospam") || is_abbrev(arg, "spam")) {
    if (IS_SET(desc->autobits, AUTO_NOSPAM)) {
      sendTo("You will now see combat misses and other \"spam\".\n\r");
      REMOVE_BIT(desc->autobits, AUTO_NOSPAM);
    } else {
      sendTo("You will no longer see combat misses and other \"spam\".\n\r");
      SET_BIT(desc->autobits, AUTO_NOSPAM);
    }
  } else if (is_abbrev(arg, "nospells") || is_abbrev(arg, "spells")) {
    if (IS_SET(desc->autobits, AUTO_NOSPELL)) {
      sendTo("You will now see all spell messages.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_NOSPELL);
    } else {
      if (IS_SET(desc->autobits, AUTO_HALFSPELL)) {
         sendTo("You can not set no spell messages if you have halfspells set.\n\r");
      } else {
        sendTo("You will now only see the first and last spell message.\n\r");
        SET_BIT(desc->autobits, AUTO_NOSPELL);
      }
    }
  } else if (is_abbrev(arg, "halfspells") || is_abbrev(arg, "halfspells")) {
    if (IS_SET(desc->autobits, AUTO_HALFSPELL)) {
      sendTo("You will now see all spell messages.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_HALFSPELL);
    } else {
      if (IS_SET(desc->autobits, AUTO_NOSPELL)) {
        sendTo("You can not set half spell messages if you have nospells set.\n\r");
      } else {
        sendTo("You will now only see half the spell messages randomly.\n\r");
        SET_BIT(desc->autobits, AUTO_HALFSPELL);
      }
    }
  } else if (is_abbrev(arg, "autoeat") || is_abbrev(arg, "eat")) {
    if (IS_SET(desc->autobits, AUTO_EAT)) {
      sendTo("You will now have to eat and drink manually.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_EAT);
    } else {
      sendTo("You will automatically eat and drink now.\n\r");
      SET_BIT(desc->autobits, AUTO_EAT);
    }
  } else if (is_abbrev(arg, "limbs")) {
    if (IS_SET(desc->autobits, AUTO_LIMBS)) {
      sendTo("You will no longer see tank limb status after every fight\n\r");
      REMOVE_BIT(desc->autobits, AUTO_LIMBS);
    } else {
      sendTo("You will now see tank limb status after every fight\n\r");
      SET_BIT(desc->autobits, AUTO_LIMBS);
    }
  } else if (is_abbrev(arg, "money") || is_abbrev(arg, "loot-money")) {
    if (IS_SET(desc->autobits, AUTO_LOOT_MONEY)) {
      sendTo("You will no longer get talens from corpses.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_LOOT_MONEY);
    } else if (IS_SET(desc->autobits, AUTO_LOOT_NOTMONEY)) {
      sendTo("You are already looting everything from corpses.\n\r");
    } else {
      sendTo("You will now get talens from any corpse you slay.\n\r");
      SET_BIT(desc->autobits, AUTO_LOOT_MONEY);
    }
  } else if (is_abbrev(arg, "loot-all") || is_abbrev(arg, "notmoney") || 
             is_abbrev(arg, "all") ) {
    if (IS_SET(desc->autobits, AUTO_LOOT_NOTMONEY)) {
      sendTo("You will no longer get everything from corpses.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_LOOT_NOTMONEY);
    } else {
      sendTo("You will now get all from any corpse you slay.\n\r");
      SET_BIT(desc->autobits, AUTO_LOOT_NOTMONEY);
    }
  } else if (is_abbrev(arg, "afk") ) {
    if (IS_SET(desc->autobits, AUTO_AFK)) {
      sendTo("You will no longer send afk messages when inactive.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_AFK);
    } else {
      sendTo("You will now send an afk message when inactive.\n\r");
      SET_BIT(desc->autobits, AUTO_AFK);
    }
  } else if (is_abbrev(arg, "split") ) {
    if (IS_SET(desc->autobits, AUTO_SPLIT)) {
      sendTo("You will no longer split gold automatically.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_SPLIT);
    } else {
      sendTo("You will now split gold automatically.\n\r");
      SET_BIT(desc->autobits, AUTO_SPLIT);
    }
  } else if (is_abbrev(arg, "success") && isImmortal() ) {
    if (IS_SET(desc->autobits, AUTO_SUCCESS)) {
      sendTo("You will no longer have automatic skill success/failure.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_SUCCESS);
    } else {
      sendTo("You will now have automatic skill success/failure.\n\r");
      SET_BIT(desc->autobits, AUTO_SUCCESS);
    }
  } else if (is_abbrev(arg, "pouch") ) {
    if (IS_SET(desc->autobits, AUTO_POUCH)) {
      sendTo("You will no longer open moneypouches automatically.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_POUCH);
    } else {
      sendTo("You will now open moneypouches automatically.\n\r");
      SET_BIT(desc->autobits, AUTO_POUCH);
    }
  } else if (is_abbrev(arg, "trophy")){
    if (IS_SET(desc->autobits, AUTO_TROPHY)) {
      sendTo("You will no longer check trophy after kills.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_TROPHY);
    } else {
      sendTo("You will now check trophy after kills.\n\r");
      SET_BIT(desc->autobits, AUTO_TROPHY);
    }
  } else if (is_abbrev(arg, "tips")) {
    if (IS_SET(desc->autobits, AUTO_TIPS)) {
      if (isImmortal())
        sendTo("You will now see the basic menus in the editors.\n\r");
      else
        sendTo("You will no longer see periodic tips.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_TIPS);
    } else {
      if (isImmortal())
        sendTo("You will now see the advanced menus in the editors.\n\r");
      else
        sendTo("You will now see periodic tips.\n\r");
      SET_BIT(desc->autobits, AUTO_TIPS);
    }
  } else if (is_abbrev(arg, "join") ) {
    if (IS_SET(desc->autobits, AUTO_JOIN)) {
      sendTo("You can not be admitted to any faction now.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_JOIN);
    } else {
      sendTo("You make yourself available for admission to factions.\n\r");
      SET_BIT(desc->autobits, AUTO_JOIN);
    }
  } else if (is_abbrev(arg, "dissect") ) {
    if (IS_SET(desc->autobits, AUTO_DISSECT)) {
      sendTo("You will no longer dissect corpses automatically.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_DISSECT);
    } else {
      sendTo("You will now dissect corpses for additional booty when appropriate.\n\r");
      SET_BIT(desc->autobits, AUTO_DISSECT);
    }
  } else if (is_abbrev(arg, "engage") ) {
    if (IS_SET(desc->autobits, AUTO_ENGAGE)) {
      sendTo(COLOR_BASIC, format("You will now default to %sfighting back%s if attacked or if casting.\n\r") % redBold() % norm());
      sendTo("You are still free to engage rather than fight by using the engage command.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_ENGAGE);
    } else {
      if (IS_SET(desc->autobits, AUTO_ENGAGE_ALWAYS)) {
        sendTo("You can not both auto engage and engage-all.\n\r");

      } else {

        sendTo(COLOR_BASIC, format("You will now %sengage%s if you start a fight by casting or praying.\n\r") % greenBold() % norm());
        SET_BIT(desc->autobits, AUTO_ENGAGE);
      }
    }
  } else if (is_abbrev(arg, "engage-all") || is_abbrev(arg, "no-fight") || is_abbrev(arg, "engage-always") ) {
    if (IS_SET(desc->autobits, AUTO_ENGAGE_ALWAYS)) {
      sendTo(COLOR_BASIC, format("You will now default to %sfighting back%s if attacked and when you cast.\n\r") % redBold() % norm());
      sendTo("You are still free to engage rather than fight by using the engage command.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_ENGAGE_ALWAYS);
    } else {
      if (IS_SET(desc->autobits, AUTO_ENGAGE)) {
        sendTo("You can not both auto engage and engage-all.\n\r");
      } else {
        sendTo(COLOR_BASIC, format("You will now default to %sengaging%s if attacked and when you cast to start a fight.\n\r") % greenBold() % norm());
        sendTo("You are free to fight rather than engage by using the hit command in battle.\n\r");
        SET_BIT(desc->autobits, AUTO_ENGAGE_ALWAYS);
      }
    }
  } else if (is_abbrev(arg, "hunt") ) {
    if (IS_SET(desc->autobits, AUTO_HUNT)) {
      sendTo("You will no longer head toward things you are tracking.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_HUNT);
    } else {
      sendTo("You will now head automatically toward things you are tracking.\n\r");
      SET_BIT(desc->autobits, AUTO_HUNT);
    }
  } else if (is_abbrev(arg, "no-hero-sprites") || is_abbrev(arg, "hero-sprites") || is_abbrev(arg, "sprites") ) {
    if (IS_SET(desc->autobits, AUTO_NOSPRITE)) {
      sendTo("Hero sprites will now follow you, if you are eligible.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_NOSPRITE);
    } else {
      sendTo("Hero sprites will no longer follow you.\n\r");
      SET_BIT(desc->autobits, AUTO_NOSPRITE);
    }
  } else if (is_abbrev(arg, "notell") || is_abbrev(arg, "tell")) {
    if (IS_SET(desc->autobits, AUTO_NOTELL)) {
      sendTo("People can now initiate tells to you.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_NOTELL);
    } else {
      sendTo("From now on, people will be unable to initiate tells to you.\n\r");
      SET_BIT(desc->autobits, AUTO_NOTELL);
    }
  } else if (is_abbrev(arg, "autogroup") || is_abbrev(arg, "group")) {
    if (IS_SET(desc->autobits, AUTO_AUTOGROUP)) {
      sendTo("You will no longer automatically group new followers.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_AUTOGROUP);
    } else {
      sendTo("You will now automatically group new followers.\n\r");
      SET_BIT(desc->autobits, AUTO_AUTOGROUP);
    }
  } else if(is_abbrev(arg, "list") && hasWizPower(POWER_TOGGLE)){
    for(togTypeT t=TOG_NONE;t<MAX_TOG_TYPES;t++){
     if(t==TOG_NONE)
	    continue;
      
      sendTo(COLOR_BASIC, format("%-17s : %s\n\r") %
	     toggleInfo[t]->name %
	     toggleInfo[t]->descr);
    }
  } else if(hasWizPower(POWER_TOGGLE)){  // check global toggles
    for(togTypeT t=TOG_NONE;t<MAX_TOG_TYPES;t++){
      unsigned int len=toggleInfo[t]->name.length();
      sstring buf="";
      for(unsigned int i=0;i<len;++i){
	      if(toggleInfo[t]->name[i] == ' ')
	        continue;

	      buf += toggleInfo[t]->name[i];
      }
      
      if(is_abbrev(arg, buf)){
	      toggleInfo[t]->toggle = !toggleInfo[t]->toggle;

	      sendTo(format("%s is now %s.\n\r") % 
	       toggleInfo[t]->name %
	       (toggleInfo[t]->toggle ? "on" : "off"));
	      vlogf(LOG_MISC, format("%s has turned %s %s") % getName() %
	            toggleInfo[t]->name % on_or_off(toggleInfo[t]->toggle));
	      vlogf(LOG_MISC, format("- %s") % toggleInfo[t]->descr);

	      if(t==TOG_CLIENTS)
	        closeClientConnections(this);
	      return;
      }
    }
    sendTo("Unrecognized toggle.  Type HELP TOGGLE to see valid toggles.\n\r");
  } else {
    sendTo("Unrecognized toggle.  Type HELP TOGGLE to see valid toggles.\n\r");
    return;
  }
}
