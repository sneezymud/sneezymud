//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//  toggle.cc
//  Basic Toggle information
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

TOGINFO TogIndex[MAX_TOG_INDEX + 1] =
{
  {"", MOB_NONE},      // 0           Leave Blank
  {"Avenger Quest: eligible", MOB_NONE},
  {"Avenger Quest: got rules", MOB_BISHOP_BMOON},
  {"Avenger Quest: hunting troll", MOB_BISHOP_BMOON},
  {"Avenger Quest: killed troll", MOB_TROLL_GIANT},
  {"Avenger Quest: quest finale", MOB_BISHOP_BMOON},    //  5
  {"Avenger Quest: has avenger", MOB_KING_GH},
  {"Avenger Quest: failed", MOB_NONE},
  {"Avenger Quest: penanced", MOB_OLD_WOMAN},
  {"Vindicator Quest: eligible", MOB_NONE},
  {"Vindicator Quest: found blacksmith", MOB_FISTLAND}, // 10
  {"Vindicator Quest: hunting captain", MOB_FISTLAND},
  {"Vindicator Quest: killed captain", MOB_CAPTAIN_RYOKEN},
  {"Vindicator Quest: failed task", MOB_NONE},
  {"Vindicator Quest: right ore", MOB_FISTLAND},
  {"Vindicator Quest: start (2)", MOB_FISTLAND},  // 15
  {"Vindicator Quest: seeking phoenix", MOB_FISTLAND},
  {"Vindicator Quest: phoenix found", MOB_PHOENIX},
  {"Vindicator Quest: hunting demon", MOB_PHOENIX},
  {"Vindicator Quest: killed demon", MOB_NONE},
  {"Vindicator Quest: failed task (2)", MOB_NONE},  // 20
  {"Vindicator Quest: got feather", MOB_PHOENIX},
  {"Vindicator Quest: right feather", MOB_FISTLAND},
  {"Vindicator Quest: has vindicator", MOB_FISTLAND},
  {"Vindicator Quest: rules dishonor", MOB_FISTLAND},
  {"Vindicator Quest: seeking penance", MOB_GM_IRIS},  // 25
  {"Vindicator Quest: got penance object", MOB_NITELLION},
  {"Vindicator Quest: purified", MOB_GM_IRIS},
  {"Ranger 1st Quest: Found Hermit", MOB_HERMIT_GHPARK},

  {"Immortal Quest: On Quest", MOB_NONE},
  {"Silverclaw Quest: find Scar", MOB_SILVERCLAW},  // 30
  {"Silverclaw Quest: gave collar", MOB_SCAR},
  {"Silverclaw Quest: find map", MOB_HOBBIT_ADVENTURER},
  {"Silverclaw Quest: find Gruum", MOB_HOBBIT_ADVENTURER},
  {"Silverclaw Quest: find Warlord", MOB_GRUUM},   
  {"Silverclaw Quest: find Bishop", MOB_GRUUM}, // 35 
  {"Silverclaw Quest: find tablet", MOB_GHOST_BISHOP},
  {"Silverclaw Quest: on solo", MOB_GHOST_BISHOP},
  {"Silverclaw Quest: load tablet", MOB_UNDEAD_CHIEF}, 
  {"Silverclaw Quest: find cloud dragon", MOB_DRAGON_CLOUD},
  {"Silverclaw Quest: find bronze dragon", MOB_DRAGON_BRONZE}, // 40
  {"Silverclaw Quest: find worker dragon", MOB_DRAGON_WORKER},  
  {"Silverclaw Quest: find gold dragon", MOB_DRAGON_GOLD},
  {"Silverclaw Quest: find silver dragon", MOB_DRAGON_SILVER},
  {"Silverclaw Quest: find Raliki", MOB_RALIKI},
  {"Silverclaw Quest: kill Raliki", MOB_RALIKI}, // 45 
  {"Holy Devastator Quest:  find Miser Ben", MOB_CREED},
  {"Holy Devastator Quest:  took bribe", MOB_MISER_BEN},
  {"Holy Devastator Quest:  find opal", MOB_CREED},
  {"Holy Devastator Quest:  do riddle", MOB_CREED},
  {"Holy Devastator Quest:  killed Miser Ben", MOB_MISER_BEN},  // 50
  {"Holy Devastator Quest:  find medicine man", MOB_SPARTAGUS},
  {"Holy Devastator Quest:  searching for crucifix", MOB_MEDICINE_MAN},
  {"Holy Devastator Quest:  found crucifix", MOB_PRIEST_HOLY},
  {"Holy Devastator Quest:  gave crucifix", MOB_MEDICINE_MAN},
  {"Holy Devastator Quest:  killed Spartagus", MOB_SPARTAGUS}, // 55
  {"Holy Devastator Quest:  got wine", MOB_WORKER_WINERY},
  {"Holy Devastator Quest:  answered Taille's riddle", MOB_TAILLE},
  {"Holy Devastator Quest:  seaching for great sword <r>*Natural Load*<1>", MOB_OVERLORD},
  {"Holy Devastator Quest:  find polished wooden ring <r>*Natural Load*<1>", MOB_TAILLE},
  {"Holy Devastator Quest:  forfeit vindicator", MOB_GRIZWALD}, // 60
  {"Holy Devastator Quest:  did not take gang member's deal", MOB_GANGMEMBER_GIBBETT},
  {"Holy Devastator Quest:  took gang member's deal", MOB_GANGMEMBER_GIBBETT},
  {"Holy Devastator Quest:  got deikhan shield info", MOB_ABNOR},
  {"Holy Devastator Quest:  get flower <r>*Natural Load*<1>", MOB_POACHER},
  {"Holy Devastator Quest:  answered assassin's riddle", MOB_ASSASSIN}, // 65
  {"Holy Devastator Quest:  find Lorto", MOB_YOLA},
  {"Holy Devastator Quest:  deceived Lorto", MOB_LORTO},
  {"Holy Devastator Quest:  find Sultress", MOB_ABNOR},
  {"Holy Devastator Quest:  find Bararakna", MOB_JAQUIN},
  {"Holy Devastator Quest:  received dress of rites", MOB_BARARAKNA}, // 70
  {"Holy Devastator Quest:  find Sloth", MOB_SULTRESS},
  {"Holy Devastator Quest:  can get Devastator", MOB_NESMUM},
  {"Holy Devastator Quest:  cheat Miser Ben", MOB_MISER_BEN},
  {"Holy Devastator Quest:  cheat Spartagus", MOB_SPARTAGUS},
  {"Holy Devastator Quest:  cheat Marcus", MOB_MARCUS},    //75
  {"Holy Devastator Quest:  cheat Taille", MOB_TAILLE},
  {"Holy Devastator Quest:  cheat Abnor", MOB_ABNOR},
  {"Holy Devastator Quest:  cheat Sultress", MOB_SULTRESS},
  {"Holy Devastator Quest:  cheat Nesmum", MOB_NESMUM},
  {"Has Skill:  Read Magic", MOB_NONE},  //80
  {"Monk Red Quest: Is Eligible", MOB_MONK_GM_LEVEL50},
  {"Monk Red Quest: Started Quest", MOB_MONK_GM_LEVEL50},
  {"Monk Red Quest: Finished Quest", MOB_NONE},
  {"Monk Red Quest: Has Sash", MOB_MONK_GM_LEVEL50},
  {"Immortal Skill: Stat", MOB_NONE},     //85
  {"Immortal Skill: Logs", MOB_NONE},
  {"Holy Devastator Quest:  find Abnor", MOB_GRIZWALD},
  {"", MOB_NONE},       // toggles 88-96 are open for future immortal toggles
  {"", MOB_NONE},
  {"", MOB_NONE},   //90
  {"", MOB_NONE},
  {"", MOB_NONE},
  {"", MOB_NONE},
  {"", MOB_NONE},
  {"", MOB_NONE},   //95
  {"Holy Devastator Quest:  has Devastator", MOB_CREED},
  {"Spirit of Warrior Quest:  has ancient symbol", MOB_SPIRIT_OF_WARRIOR},  
  {"Spirit of Warrior Quest:  has holy white defender", MOB_SPIRIT_OF_WARRIOR},
  {"Spirit of Warrior Quest:  has moss covered robes", MOB_SPIRIT_OF_WARRIOR},
  {"Sculpture Quest:  find the high priest", MOB_DWARVEN_AMBASSADOR}, //100
  {"Sculpture Quest:  has a sculpture", MOB_DWARVEN_HIGH_PRIEST},
  {"Sculpture Quest:  completed quest already", MOB_LENGE_MERCHANT},
  {"Is Eligible for Skill: Tornado", MOB_NONE},
  {"Has Skill:  Tornado", MOB_GERSARD},
  {"Is Eligible for Skill: Barkskin", MOB_NONE},   //105
  {"Has Skill:  Barkskin", MOB_FIRST_RANGER_BASIC_TRAINER},
  {"Is Eligible for Skill: Earthquake", MOB_NONE},
  {"Has Skill:  Earthquake", MOB_TALAR},
  {"Is Eligible for Skill:  Dual Wield", MOB_NONE},
  {"Has Skill: Dual Wield", MOB_FIRST_RANGER_BASIC_TRAINER},  //110
  {"Is Eligible for Skill:  Shapeshift", MOB_NONE},
  {"Has Skill: Shapeshift", MOB_FIRST_ANIMAL_TRAINER},
  {"Is Eligible for Skill:  Fireball", MOB_NONE},
  {"Has Skill: Fireball", MOB_ASH},
  {"Is Eligible for Skill:  Ice Storm", MOB_NONE},     //115
  {"Find Dark Robes:  Ice Storm", MOB_RAULDOPLIC},
  {"Has Skill:  Ice Storm", MOB_OLD_SAGE},
  {"Is Eligible for Skill:  Stoneskin", MOB_NONE},
  {"Find Granite Signet:  Stoneskin", MOB_DAKINOR},
  {"Has Granite Signet:  Stoneskin", MOB_DAKINOR},  //120
  {"Has Skill:  Stoneskin", MOB_THALIA},
  {"Is Eligible for Skill:  Galvanize", MOB_NONE},
  {"Find Purple Robes:  Galvanize", MOB_BERARDINIS},
  {"Has Skill:  Galvanize", MOB_BERARDINIS},
  {"Is Eligible for Skill:  Powerstone", MOB_NONE},      //125
  {"Find Snakestaff:  Powerstone", MOB_MERRITT},
  {"Has Skill: Powerstone", MOB_MERRITT},
  {"Has Skill: Advanced Kick", MOB_FONG_CHUN},
  {"Has paid toll", MOB_BULGE},
  {"Is Eligible for Skill:  Advanced Kick", MOB_NONE},  //130
  {"Eligible to enter Logrus initiation", MOB_NONE},
  {"Is Eligible for Monk Sash:  White", MOB_NONE},
  {"Has Started Monk Sash Quest:  White", MOB_MONK_GM_LEVEL15},
  {"Has Monk Sash:  White", MOB_HUANG_LO},
  {"Is Eligible for Monk Sash:  Yellow", MOB_NONE},  //135
  {"Has Finished Monk Sash Quest:  Yellow", MOB_MONK_GM_LEVEL15},
  {"Has Monk Sash:  Yellow", MOB_MONK_GM_LEVEL15},
  {"Monk Purple Quest: eligible", MOB_NONE},
  {"Monk Purple Quest: started", MOB_MONK_GM_LEVEL15},
  {"Monk Purple Quest: Lepers Killed:  1", MOB_NONE}, // 140
  {"Monk Purple Quest: Lepers Killed:  2", MOB_NONE},
  {"Monk Purple Quest: Lepers Killed:  3", MOB_NONE},
  {"Monk Purple Quest: Lepers Killed:  4", MOB_NONE},
  {"Monk Purple Quest: complete", MOB_NONE},
  {"Monk Purple Quest: owned", MOB_MONK_GM_LEVEL15}, // 145
  {"Is Eligible for Monk Sash:  Blue", MOB_NONE},
  {"Has Started Monk Sash Quest:  Blue", MOB_MONK_GM_LEVEL40},
  {"Monk Blue Quest Killed Tiger Shark", MOB_NONE},
  {"Has Finished Monk Sash Quest:  Blue", MOB_MONK_GM_LEVEL40},
  {"Has Monk Sash:  Blue", MOB_MONK_GM_LEVEL40},  // 150
  {"Monk Green Quest: eligible", MOB_NONE},
  {"Monk Green Quest: started", MOB_MONK_GM_LEVEL40},
  {"Monk Green Quest: falling", MOB_NONE},
  {"Monk Green Quest: fallen", MOB_NONE},
  {"Rat King Quest: Gave Essence to Rat King", MOB_RAT_KING}, // 155
  {"Monk Green Quest: owned", MOB_MONK_GM_LEVEL40},
  {"Mage Belt Quest: eligible", MOB_NONE},
  {"Mage Belt Quest: started", MOB_MAGE_GM_LEVEL15},
  {"Mage Belt Quest: searching for thread", MOB_MAGE_GM_LEVEL15},
  {"Mage Belt Quest: owned", MOB_MAGE_GM_LEVEL15},  // 160
  {"Has Skill: Catfall", MOB_MONK_GM_LEVEL40},
  {"Ranger 1st Quest: Eligible", MOB_NONE},
  {"Ranger 1st Quest: Started", MOB_HERMIT_GHPARK},
  {"Ranger 1st Quest: Talked to Gnome Gnoble of Farmers", MOB_GNOBLE_FARMER},
  {"Ranger 1st Quest: Talked to Gnome Farmer", MOB_GNOME_FARMER}, // 165
  {"Ranger 1st Quest: Talked to Gnome Male Child", MOB_GNOME_CHILD},
  {"Ranger 1st Quest: Talked to Gnome Farmhand", MOB_GNOME_FARMHAND},
  {"Ranger 1st Quest: Killed John the Rustler", MOB_NONE},
  {"Ranger 1st Quest: Gave Hermit the Gnomish Cattle Hide", MOB_HERMIT_GHPARK},
  {"Ranger 1st Quest: Gave Hermit the Squirrel Pelts", MOB_HERMIT_GHPARK}, // 170
  {"Ranger 1st Quest: Seeking Balcor the Mage", MOB_HERMIT_GHPARK},
  {"Ranger 1st Quest: Got Scroll from Balcor", MOB_BALCOR},
  {"Ranger 1st Quest: Finished", MOB_HERMIT_GHPARK},
  {"Is Eligible for Ranger L14 Quest", MOB_NONE},
  {"Has Started Ranger L14 Quest", MOB_HERMIT_JED}, // 175
  {"Has Seen Kobold Poacher", MOB_POACH_KOBOLD}, 
  {"Seeking Orc Poacher", MOB_HERMIT_JED},
  {"Has Seen Orc Poacher", MOB_POACH_ORC},
  {"Seeking Blind Pygmy Bone Woman", MOB_HERMIT_JED},
  {"Seeking Apple for Bone Woman", MOB_BONE_PYGMY}, // 180
  {"Got Carved Buckle from Bone Woman", MOB_BONE_PYGMY},
  {"Seeking Orc Magi", MOB_HERMIT_JED},
  {"Failed to Kill Orc Magi", MOB_NONE},
  {"Proving Self", MOB_HERMIT_JED},
  {"Killed Orc Magi", MOB_NONE}, // 185
  {"Finished Ranger L14 Quest", MOB_HERMIT_JED},
  {"Holy Devastator Quest: eligible", MOB_NONE},
  {"Mage Robe Quest: eligible", MOB_NONE},
  {"Mage Robe Quest: seeking Druidess", MOB_SIMON_SPELLCRAFTER},
  {"Mage Robe Quest: getting oil", MOB_SIMON_SPELLCRAFTER},  //190
  {"Mage Robe Quest: getting symbol", MOB_SIMON_SPELLCRAFTER},
  {"Mage Robe Quest: getting metal",  MOB_SIMON_SPELLCRAFTER},
  {"Mage Robe Quest: getting fabric", MOB_SIMON_SPELLCRAFTER},
  {"Mage Robe Quest: has mage robe", MOB_SIMON_SPELLCRAFTER},
  {"Ranger L21 Quest: Eligible", MOB_NONE},  //195
  {"Ranger L21 Quest: Started", MOB_RANGER_AMBER},
  {"Ranger L21 Quest: Killed Volcano Cleric", MOB_NONE},
  {"Ranger L21 Quest: Failed to kill Volcano Cleric", MOB_NONE},
  {"Ranger L21 Quest: Proving self L21A", MOB_RANGER_AMBER},
  {"Ranger L21 Quest: Seeking Arden Cleric", MOB_RANGER_AMBER},  //200
  {"Ranger L21 Quest: Killed Arden Cleric", MOB_NONE},
  {"Ranger L21 Quest: Failed to kill Arden Cleric", MOB_NONE},
  {"Ranger L21 Quest: Proving self L21B", MOB_RANGER_AMBER},
  {"Ranger L21 Quest: Seeking Aarakocra Chief", MOB_RANGER_AMBER},
  {"Ranger L21 Quest: Talked to Aarakocra Chief", MOB_CHIEF_AARAKOCRA},  //205
  {"Ranger L21 Quest: Got feathers", MOB_CHIEF_AARAKOCRA},
  {"Ranger L21 Quest: Seeking two rabbit skins", MOB_RANGER_AMBER},
  {"Ranger L21 Quest: Gave one rbbit skin", MOB_RANGER_AMBER},
  {"Ranger L21 Quest: Seeking Bullywug Oil", MOB_RANGER_AMBER},
  {"Ranger L21 Quest: Seeking Bandits in Bullywug", MOB_SHAMAN_BULLY},  //210
  {"Ranger L21 Quest: Got Bullywug Oil", MOB_SHAMAN_BULLY},
  {"Ranger L21 Quest: Finished Ranger L21 Quest", MOB_RANGER_AMBER},
  {"Faction Membership: eligible", MOB_NONE},
  {"Caldonia: bought her a shot of whisky", MOB_CALDONIA},
  {"", MOB_NONE},  // 215
  {"", MOB_NONE},
  {"", MOB_NONE},
  {"", MOB_NONE},
  {"Warrior L40 Quest: Eligible", MOB_NONE},                  
  {"Warrior L40 Quest: Started", MOB_CHIEF_MALE_VILLAGER}, //220
  {"Warrior L40 Quest: Killing Chief", MOB_ELDERLY_SHAMAN},
  {"Warrior L40 Quest: Killing Shaman", MOB_ELDERLY_SHAMAN},
  {"Warrior L40 Quest: Finished Warrior", MOB_CHIEF_MALE_VILLAGER},
  {"Shaman L15 Juju: Eligible", MOB_NONE},
  {"Shaman L15 Juju: Get Thong", MOB_JUJU_BUNNY}, //225
  {"Shaman L15 Juju: Get Mare Hide", MOB_JUJU_MARE},
  {"Shaman L15 Juju: Get Sinew", MOB_NONE},
  {"Shaman L15 Juju: Get Beaded Necklace", MOB_JUJU_TETRARCH},
  {"Shaman L15 Juju: Has Juju", MOB_NONE},
  {"Faction: Has paid registration fee", MOB_FACTION_REGISTRAR}, // 230
  {"Faction: Has created a faction", MOB_FACTION_REGISTRAR},
  {"Shaman Totem Mask: Eligible", MOB_NONE},
  {"Shaman Totem Mask: Started", MOB_GANDOLFO},
  {"Shaman Totem Mask: Find Forsaken", MOB_GANDOLFO},
  {"Shaman Totem Mask: Find wooden plank", MOB_FORSAKEN}, // 235
  {"Shaman Totem Mask: Has sapless wood for Ptok", MOB_FORSAKEN},
  {"Shaman Totem Mask: Find Scaled Hide", MOB_PTOK},
  {"Shaman Totem Mask: Give Gondolfo covered plank", MOB_PTOK},
  {"Shaman Totem Mask: Recover vial of sap from Forsaken", MOB_GANDOLFO},
  {"Shaman Totem Mask: Find Elric for gris gris", MOB_GANDOLFO}, // 240
  {"Shaman Totem Mask: Kill Baron Samedi", MOB_ELRIC},
  {"Shaman Totem Mask: Has Baron Samedi's vision", MOB_SAMEDI},
  {"Shaman Totem Mask: Killed Elric", MOB_ELRIC},
  {"Shaman Totem Mask: Find Ptok to face true evil", MOB_PTOK},
  {"Shaman Totem Mask: Killed Father's Spirit", MOB_PTOK}, // 245
  {"Shaman Totem Mask: Finished", MOB_GANDOLFO},
  {"Perma Death Character", MOB_NONE},
  {"Psionicist", MOB_NONE},
  {"Dragon Armor: red scales", 13732},
  {"Dragon Armor: green scales", 13732},
  {"Dragon Armor: white scales", 13732},
  {"Dragon Armor: tungsten suit", 13732},
  {"Dragon Armor: huge opal", 13732},
  {"Dragon Armor: 1 vial of dragon bone", 13732},
  {"Dragon Armor: 2 vials of dragon bone", 13732},
  {"Dragon Armor: 3 vials of dragon bone", 13732},
  {"Dragon Armor: 4 vials of dragon bone", 13732},
  {"Dragon Armor: 5 vials of dragon bone", 13732},
  {"Dragon Armor: sinew", 13732},
  {"Dragon Armor: thread element", 13732},
  {"Dragon Armor: talens", 13732},
  {"Dragon Armor: warhammer", 13732},
  {"Dragon Armor: scales", 13732},
  {"", MOB_NONE}, 
};










