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
  {"Shaman Totem Mask: Find Ptok to face true evil", MOB_GANDOLFO},
  {"Shaman Totem Mask: Killed Father's Spirit", MOB_PTOK}, // 245
  {"Shaman Totem Mask: Finished", MOB_GANDOLFO},
  {"Perma Death Character", MOB_NONE},
  {"Psionicist", MOB_NONE},
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
  {"Bitten by vampire", MOB_NONE},
  {"Vampire", MOB_NONE},  //265
  {"Lycanthrope", MOB_NONE},
  {"Transformed Lycanthrope", MOB_NONE},
  {"Lycanthropy: virgin flask", 9323},
  {"Lycanthropy: terfefly", 23291},
  {"Lycanthropy: ingredients", 6762}, //270
  {"Lycanthropy: urine", 6762},
  {"Lycanthropy: wolvesbane", 6762},
  {"Lycanthropy: silver", 6762},
  {"Lycanthropy: hemlock", 6762},
  {"BlahBlah Shouts", MOB_NONE},
  {"Monk: Paid Tabuda", MOB_NONE},
  {"Has Skill: Catleap", MOB_NONE},
  {"Trait: Coward", MOB_NONE},
  {"Trait: Blind", MOB_NONE},
  {"", MOB_NONE}, 
};



const char *on_or_off(bool tog){
  if(tog)
    return "<G>on <1>";
  else
    return "<R>off<1>";
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
  arg2=one_argument(arg2, arg);
  for (; isspace(*arg2); arg2++);


  if (!*arg) {
    sendTo(COLOR_BASIC, "\n\r<c>Player Toggles<1>\n\r");
    sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
    
    for (int i = 0;i < MAX_AUTO;i++) {
      if (((unsigned int) (1<<i) == AUTO_SUCCESS))
	++i;
      if (i<MAX_AUTO && *auto_name[i]) {
        sendTo(COLOR_BASIC, fmt("%-17s : %s  | ") %               (((unsigned int) (1 << i) == AUTO_TIPS && isImmortal()) ? "Advanced Menus" : auto_name[i]) %
               on_or_off(IS_SET(desc->autobits, (unsigned) (1<<i))));
      }
      ++i;
      if (((unsigned int) (1<<i) == AUTO_SUCCESS))
	++i;
      if (i<MAX_AUTO && *auto_name[i]) {
        sendTo(COLOR_BASIC, fmt("%-17s : %s  | ") %
               (((unsigned int) (1 << i) == AUTO_TIPS && isImmortal()) ? "Advanced Menus" : auto_name[i]) %
               on_or_off(IS_SET(desc->autobits, (unsigned) (1<<i))));
      }
      ++i;
      if (((unsigned int) (1<<i) == AUTO_SUCCESS))
	++i;
      if (i<MAX_AUTO && *auto_name[i]) {
        sendTo(COLOR_BASIC, fmt("%-17s : %s\n\r") %
               (((unsigned int) (1 << i) == AUTO_TIPS && isImmortal()) ? "Advanced Menus" : auto_name[i]) %
               on_or_off(IS_SET(desc->autobits, (unsigned) (1<<i))));
      }
    }
    

    if(getWimpy())
      sendTo(COLOR_BASIC, fmt("Wimpy             : <G>%-4i<1>\n\r") %
	     getWimpy());
    else
      sendTo(COLOR_BASIC, "Wimpy             : <R>off <1>\n\r");

    sendTo(COLOR_BASIC, fmt("Newbie Helper     : %s  | ") % on_or_off(isPlayerAction(PLR_NEWBIEHELP)));

    sendTo(COLOR_BASIC, fmt("Anonymous         : %s\n\r") % on_or_off(isPlayerAction(PLR_ANONYMOUS)));

    sendTo(COLOR_BASIC, "\n\r<c>Terminal Toggles<1>\n\r");
    sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
    sendTo(COLOR_BASIC, fmt("Screensize        : <G>%-3i<1>  | ") % desc->screen_size);
    sendTo(COLOR_BASIC, fmt("Terminal          : <G>%-5s<1>| ") %
	   (ansi()?"ansi":(vt100()?"vt100":"none")));
    sendTo(COLOR_BASIC, fmt("Boss Mode         : %s\n\r") % on_or_off(IS_SET(desc->account->flags, ACCOUNT_BOSS)));
    sendTo(COLOR_BASIC, fmt("MSP Sound         : %s  | ") % on_or_off(IS_SET(desc->account->flags, ACCOUNT_MSP)));
    sendTo(COLOR_BASIC, fmt("Account Terminal  : <G>%-5s<1>| ") % 
	   ((desc->account->term == TERM_ANSI)?"ansi ":
	   ((desc->account->term == TERM_VT100)?"vt100":"none ")));
    sendTo(COLOR_BASIC, fmt("Allow Pinging     : %s\n\r") % on_or_off(isPlayerAction(PLR_PING)));
    sendTo(COLOR_BASIC, fmt("Brief             : %s  | ") % on_or_off(isPlayerAction(PLR_BRIEF)));
    sendTo(COLOR_BASIC, fmt("Compact           : %s  | ") % on_or_off(isPlayerAction(PLR_COMPACT)));
    sendTo(COLOR_BASIC, fmt("Show Saves        : %s\n\r") % on_or_off(isPlayerAction(PLR_SHOW_SAVES)));

    
    if(isImmortal() || GetMaxLevel() >= GOD_LEVEL1){
      sendTo(COLOR_BASIC, "\n\r<c>Immortal Toggles<1>\n\r");
      sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
      
      if(getInvisLevel())
	sendTo(COLOR_BASIC, fmt("Invisibility      : <G>%-4i<1> | ") % getInvisLevel());
      else
	sendTo(COLOR_BASIC, "Invisibility      : <R>off <1> | ");
      
      sendTo(COLOR_BASIC, fmt("Auto Success      : %s  | ") % on_or_off(IS_SET(desc->autobits, AUTO_SUCCESS)));
      
      sendTo(COLOR_BASIC, fmt("Stealth Mode      : %s\n\r") % on_or_off(isPlayerAction(PLR_STEALTH)));
      
      sendTo(COLOR_BASIC, fmt("No Hassle         : %s  | ") % on_or_off(isPlayerAction(PLR_NOHASSLE)));
      
      sendTo(COLOR_BASIC, fmt("Immortality       : %s\n\r") % on_or_off(isPlayerAction(PLR_IMMORTAL)));
    }
    


    if (hasWizPower(POWER_TOGGLE)){
      sendTo(COLOR_BASIC, "\n\r<c>Global Toggles<1>\n\r");
      sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");

      sendTo(COLOR_BASIC, fmt("Shouting          : %s  | ") % on_or_off(!Silence));
      sendTo(COLOR_BASIC, fmt("Clients           : %s  | ") % on_or_off(Clients));
      sendTo(COLOR_BASIC, fmt("PCs w/mob names   : %s\n\r") %on_or_off(AllowPcMobs));
      sendTo(COLOR_BASIC, fmt("Sleep offensive   : %s  | ") % on_or_off(Sleep));
      sendTo(COLOR_BASIC, fmt("Gravity           : %s\n\r") % on_or_off(Gravity));
      sendTo(COLOR_BASIC, fmt("Allow Wiz-Invis   : %s  | ") % on_or_off(WizInvis));
      sendTo(COLOR_BASIC, fmt("Nuke Inactive     : %s  | ") % on_or_off(nuke_inactive_mobs));
      sendTo(COLOR_BASIC, fmt("NewbiePK          : %s\n\r") % on_or_off(NewbiePK));
      sendTo(COLOR_BASIC, fmt("Time DB Queries   : %s  | ") %on_or_off(timeQueries));
      sendTo(COLOR_BASIC, fmt("Twinky Combat     : %s  | ") % on_or_off(Twink));
      sendTo(COLOR_BASIC, fmt("Game Loop Timing  : %s\n\r") % on_or_off(gameLoopTiming));


      sendTo(COLOR_BASIC, "\n\r<c>Test Code Toggles<1>\n\r");
      sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
      
      sendTo(COLOR_BASIC, fmt("Test code #1      : %s  | ") %on_or_off(TestCode1));
      sendTo(COLOR_BASIC, fmt("Test code #2      : %s  | ") % on_or_off(TestCode2));
      sendTo(COLOR_BASIC, fmt("Test code #3      : %s\n\r") % on_or_off(TestCode3));
      sendTo(COLOR_BASIC, fmt("Test code #4      : %s  | ") % on_or_off(TestCode4));
      sendTo(COLOR_BASIC, fmt("Test code #5      : %s  | ") % on_or_off(TestCode5));
      sendTo(COLOR_BASIC, fmt("Test code #6      : %s\n\r") % on_or_off(TestCode6));
      sendTo(COLOR_BASIC, fmt("Quest code #1     : %s  | ") % on_or_off(QuestCode));
      sendTo(COLOR_BASIC, fmt("Quest code #2     : %s  | ") % on_or_off(QuestCode2));
      sendTo(COLOR_BASIC, fmt("Quest code #3     : %s\n\r") % on_or_off(QuestCode3));
      sendTo(COLOR_BASIC, fmt("Quest code #4     : %s  | ") % on_or_off(QuestCode4));
    }      

    return;

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
  } else if(is_abbrev(arg, "nohassle") && hasWizPower(POWER_TOGGLE)){
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
  } else if(is_abbrev(arg, "stealth") && hasWizPower(POWER_TOGGLE)){
    if (isPlayerAction(PLR_STEALTH)) {
      sendTo("STEALTH mode OFF.\n\r");
      remPlayerAction(PLR_STEALTH);
      if (desc && desc->m_bIsClient)
	desc->clientf(fmt("%d|%d") % CLIENT_STEALTH % FALSE);
    } else {
      sendTo("STEALTH mode ON.\n\r");
      addPlayerAction(PLR_STEALTH);
      if (desc && desc->m_bIsClient)
	desc->clientf(fmt("%d|%d") % CLIENT_STEALTH % TRUE);
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

    if (!WizInvis && !hasWizPower(POWER_TOGGLE_INVISIBILITY)) {
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
      sendTo(fmt("Invis level set to %d.\n\r") % level);
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
    int hl = hitLimit();
    int wimplimit = hl / 2 + hl % 2;
    int num=0;

    if (is_abbrev(arg2, "max") || hasQuestBit(TOG_IS_COWARD)) {
      if(hasQuestBit(TOG_IS_COWARD)){
	sendTo("You can't change your wimpy setting, you're a coward!\n\r");
      } else {
	sendTo(fmt("Setting Wimpy to Max(%d).\n\r") % (wimplimit - 1));
      }
      num = wimplimit - 1;
    } else if (is_abbrev(arg2, "off") || (num = convertTo<int>(arg2)) <= 0) {
      sendTo("Turning wimpy mode off.\n\r");
      setWimpy(0);
      wimpy = 0;
      return;
    }
    
    if ((num < 0) || (wimplimit <= num)) {
      sendTo(fmt("Please enter a number between 0-%d.\n\r") % (wimplimit-1));
      return;
    }
    
    sendTo("You are now a wimp!!\n\r");
    sendTo(fmt("You will now flee at %d hit points!\n\r") % num);
    setWimpy(num);
  } else if (is_abbrev(arg, "boss")) {
    if (!IS_SET(desc->account->flags, ACCOUNT_BOSS)) {
      SET_BIT(desc->account->flags, ACCOUNT_BOSS);
      sendTo("You are now in boss mode.\n\r");
    } else {
      REMOVE_BIT(desc->account->flags, ACCOUNT_BOSS);
      sendTo("You are no longer in boss mode.\n\r");
    }
    desc->saveAccount();
  } else if (is_abbrev(arg, "msp")) {
    if (!IS_SET(desc->account->flags, ACCOUNT_MSP)) {
      SET_BIT(desc->account->flags, ACCOUNT_MSP);
      sendTo("MUD Sound Protocol enabled.\n\r");
      // we need to set the default download directory, so do that by doing
      // a stopsound which will transmit the MSP U= command
      stopsound();
    } else {
      REMOVE_BIT(desc->account->flags, ACCOUNT_MSP);
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
        sendTo(fmt(VT_MARGSET) % 1 % (getScreen() - 3));
        addPlayerAction(PLR_ANSI);
        if (vt100())
          remPlayerAction(PLR_VT100);
        doCls(false);
        sendTo("Setting term type to Ansi...\n\r");	
      } else if(is_abbrev(arg2, "vt100")){
        if (!IS_SET(desc->prompt_d.type, PROMPT_VTANSI_BAR))
          SET_BIT(desc->prompt_d.type, PROMPT_VTANSI_BAR);
        sendTo(fmt(VT_MARGSET) % 1 % (getScreen() - 3));
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
	sendTo(fmt("Your screensize has been set to: %d\n\r") % desc->screen_size);
      } else {
	sendTo(fmt("Your current screensize is set to: %d\n\r") % desc->screen_size);
          sendTo("Screensize needs to be a number from 1-128.\n\r");
      }
    } else {
      sendTo(fmt("Your current screensize is set to: %d\n\r") % desc->screen_size);
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
      sendTo("You will now default to fighting back if attacked or if casting.\n\r");
      sendTo("You are still free to engage rather than fight by using the engage command.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_ENGAGE);
    } else {
      if (IS_SET(desc->autobits, AUTO_ENGAGE_ALWAYS)) {
        sendTo("You can not both auto engage and engage-all.\n\r");

      } else {

        sendTo("You will now engage if you start a fight by casting or praying.\n\r");
        SET_BIT(desc->autobits, AUTO_ENGAGE);
      }
    }
  } else if (is_abbrev(arg, "engage-all") || is_abbrev(arg, "no-fight") || is_abbrev(arg, "engage-always") ) {
    if (IS_SET(desc->autobits, AUTO_ENGAGE_ALWAYS)) {
      sendTo("You will now default to fighting back if attacked and when you cast.\n\r");
      sendTo("You are still free to engage rather than fight by using the engage command.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_ENGAGE_ALWAYS);
    } else {
      if (IS_SET(desc->autobits, AUTO_ENGAGE)) {
        sendTo("You can not both auto engage and engage-all.\n\r");
      } else {
        sendTo("You will now default to engaging if attacked and when you cast to start a fight.\n\r");
        sendTo("You are free to fight rather than engage by using the hit command in battle.\n\r");
        SET_BIT(desc->autobits, AUTO_ENGAGE_ALWAYS);
      }
    }
  } else if (is_abbrev(arg, "hunt") ) {
    if (IS_SET(desc->autobits, AUTO_HUNT)) {
      sendTo("You will no longer head toward things you are hunting.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_HUNT);
    } else {
      sendTo("You will now head automatically toward things you are hunting.\n\r");
      SET_BIT(desc->autobits, AUTO_HUNT);
    }
  } else if (is_abbrev(arg, "silence") && hasWizPower(POWER_TOGGLE)) {
    Silence = !Silence;
    sendTo(fmt("You have now %s shouting.\n\r") % (Silence ? "disallowed" : "allowed"));
    vlogf(LOG_MISC, fmt("%s has turned player shouting %s.") %  getName() % 
	  (Silence ? "off" : "on"));
  } else if (is_abbrev(arg, "gravity") && hasWizPower(POWER_TOGGLE)) {
    Gravity = !Gravity;
    sendTo(fmt("You have now turned gravity %s.\n\r") % 
	   (!Gravity ? "off" : "on"));
    vlogf(LOG_MISC, fmt("%s has turned gravity %s.") %  getName() % 
	  (!Gravity ? "off" : "on"));
  } else if (is_abbrev(arg, "sleep") && hasWizPower(POWER_TOGGLE)) {
    Sleep = !Sleep;
    sendTo(fmt("You have now turned offensive sleep %s.\n\r") % (!Sleep ? "off": "on"));
    vlogf(LOG_MISC, fmt("%s has turned offensive sleep %s.") %  getName() % 
	  (!Sleep ? "off"   : "on"));
  } else if (is_abbrev(arg, "wiznet") && hasWizPower(POWER_TOGGLE)) {
    WizBuild = ! WizBuild;
    sendTo(fmt("Builders can now %s the wiznet.\n\r") % (WizBuild ? "hear" : "not hear"));
    vlogf(LOG_MISC,fmt("%s has turned wiznet %s for builders.") % getName() %
	  (WizBuild ? "on" : "off"));
  } else if (is_abbrev(arg, "wizgoto") && hasWizPower(POWER_TOGGLE)) {
    WizGoto = ! WizGoto;
    sendTo(fmt("Immortals can now %s the enabled zones.\n\r") % (WizGoto ? "goto" : "not goto"));
    vlogf(LOG_MISC,fmt("%s has turned goto %s for immortals.") % getName() %
	  (WizGoto ? "on" : "off"));
  } else if (is_abbrev(arg, "wizshout") && hasWizPower(POWER_TOGGLE)) {
    WizShout = ! WizShout;
    sendTo(fmt("Immortals can now %s.\n\r") % 
	   (WizShout ? "shout" : "not shout"));
    vlogf(LOG_MISC,fmt("%s has turned shout %s for immortals.") % getName() %
	  (WizShout ? "on" : "off"));
  } else if (is_abbrev(arg, "twink") && hasWizPower(POWER_TOGGLE)) {
    Twink = ! Twink;
    sendTo(fmt("Twink combat mode is now %s.\n\r") % (Twink ? "on" : "off"));
    vlogf(LOG_MISC,fmt("%s has turned Twink combat mode %s.") % getName() %
	  (Twink ? "on" : "off"));
  } else if (is_abbrev(arg, "wizinvis") && hasWizPower(POWER_TOGGLE)) {
    if (!isImmortal() || !hasWizPower(POWER_TOGGLE_INVISIBILITY)) {
      sendTo("Invisibility use has been restricted due to overuse.\n\r");
      return;
    }
    WizInvis = ! WizInvis;
    sendTo(fmt("Immortals can now %s invisible.\n\r") % (WizInvis ? "go" : "not go"));
    vlogf(LOG_MISC,fmt("%s has turned invisibility %s.") % getName() %
	  (WizInvis? "on" : "off"));
  } else if ((is_abbrev(arg, "newbiePK") || is_abbrev(arg, "newbiepk"))  && hasWizPower(POWER_TOGGLE)) {
      NewbiePK = ! NewbiePK;
      sendTo(fmt("Newbie Pk toggle is now %s.\n\r") % (NewbiePK ? "in use" : "off"));
      vlogf(LOG_MISC,fmt("%s has now %s newbie pk.") % getName() %
	    (NewbiePK ? "enabled" : "disabled"));
      if (NewbiePK)
        vlogf(LOG_MISC,"Newbies can now be killed by anyone.");
  } else if (is_abbrev(arg, "gamelooptiming") && hasWizPower(POWER_TOGGLE)) {
    gameLoopTiming = ! gameLoopTiming;
    sendTo(fmt("game loop timing is now %s.\n\r") % (gameLoopTiming ? "in use" : "off"));
    vlogf(LOG_MISC,fmt("%s has %s game loop timing.") % getName() %
	  (gameLoopTiming ? "enabled" : "disabled"));    
  } else if (is_abbrev(arg, "testcode1") && hasWizPower(POWER_TOGGLE)) {
#if 0
    // if you are using testcode, change this so we don't collide usages
    if (strcmp(name, "Batopr")) {
      sendTo("Sorry, this is only for Batopr's use in testing.\n\r");
      return;
    }
#endif
    TestCode1 = ! TestCode1;
    sendTo(fmt("TestCode #1 is now %s.\n\r") % (TestCode1 ? "in use" : "off"));
    vlogf(LOG_MISC,fmt("%s has %s TestCode #1.") % getName() %
	  (TestCode1 ? "enabled" : "disabled"));
  } else if (is_abbrev(arg, "testcode2") && hasWizPower(POWER_TOGGLE)) {
#if 0
    // if you are using testcode, change this so we don't collide usages
    if (strcmp(name, "Batopr")) {
      sendTo("Sorry, this is only for Batopr's use in testing.\n\r");
      return;
    }
#endif
    TestCode2 = ! TestCode2;
    sendTo(fmt("TestCode #2 is now %s.\n\r") % (TestCode2 ? "in use" : "off"));
    vlogf(LOG_MISC,fmt("%s has %s TestCode #2.") % getName() %
	  (TestCode2 ? "enabled" : "disabled"));
  } else if (is_abbrev(arg, "testcode3") && hasWizPower(POWER_TOGGLE)) {
#if 0
    // if you are using testcode, change this so we don't collide usages
    if (strcmp(name, "Batopr")) {
      sendTo("Sorry, this is only for Batopr's use in testing.\n\r");
      return;
    }
#endif
    TestCode3 = ! TestCode3;
    sendTo(fmt("TestCode #3 is now %s.\n\r") % (TestCode3 ? "in use" : "off"));
    vlogf(LOG_MISC,fmt("%s has %s TestCode #3.") % getName() %
	  (TestCode3 ? "enabled" : "disabled"));
  } else if (is_abbrev(arg, "testcode4") && hasWizPower(POWER_TOGGLE)) {
#if 0
    // if you are using testcode, change this so we don't collide usages
    if (strcmp(name, "Batopr")) {
      sendTo("Sorry, this is only for Batopr's use in testing.\n\r");
      return;
    }
#endif
    TestCode4 = ! TestCode4;
    sendTo(fmt("TestCode #4 is now %s.\n\r") % (TestCode4 ? "in use" : "off"));
    vlogf(LOG_MISC,fmt("%s has %s TestCode #5.") % getName() %
	  (TestCode4 ? "enabled" : "disabled"));
  } else if (is_abbrev(arg, "testcode5") && hasWizPower(POWER_TOGGLE)) {
#if 1
    // if you are using testcode, change this so we don't collide usages
    if (strcmp(name, "Dash")) {
      sendTo("Sorry, this is only for Dash's use in testing.\n\r");
      return;
    }
#endif
    TestCode5 = ! TestCode5;
    sendTo(fmt("TestCode #5 is now %s.\n\r") % (TestCode5 ? "in use" : "off"));
    vlogf(LOG_MISC,fmt("%s has %s TestCode #5.") % getName() %
	  (TestCode5 ? "enabled" : "disabled"));
  } else if (is_abbrev(arg, "testcode6") && hasWizPower(POWER_TOGGLE)) {
#if 0
    // if you are using testcode, change this so we don't collide usages
    if (strcmp(name, "Batopr")) {
      sendTo("Sorry, this is only for Batopr's use in testing.\n\r");
      return;
    }
#endif
    TestCode6 = ! TestCode6;
    sendTo(fmt("TestCode #6 is now %s.\n\r") % (TestCode6 ? "in use" : "off"));
    vlogf(LOG_MISC,fmt("%s has %s TestCode #6.") % getName() %
	  (TestCode6 ? "enabled" : "disabled"));
    
  } else if (is_abbrev(arg, "questcode") && hasWizPower(POWER_TOGGLE)) {
    QuestCode = !QuestCode;
    sendTo(fmt("Questcode is now %s.\n\r") % (QuestCode ? "in use" : "off"));
    vlogf(LOG_MISC,fmt("%s has %s questcode.") % getName() %
	  (QuestCode ? "enabled" : "disabled"));

  } else if ((is_abbrev(arg, "questcode2") || is_abbrev(arg, "quest2")) && hasWizPower(POWER_TOGGLE)) {
    QuestCode2 = !QuestCode2;
    sendTo(fmt("Questcode 2 is now %s.\n\r") % (QuestCode2 ? "in use" : "off"));
    vlogf(LOG_MISC,fmt("%s has %s questcode 2.") % getName() %
	  (QuestCode2 ? "enabled" : "disabled"));

  } else if ((is_abbrev(arg, "questcode3") || is_abbrev(arg, "quest3")) && hasWizPower(POWER_TOGGLE)) {
    QuestCode3 = !QuestCode3;
    sendTo(fmt("Questcode 3 is now %s.\n\r") % (QuestCode3 ? "in use" : "off"));
    vlogf(LOG_MISC,fmt("%s has %s questcode 3.") % getName() %
	  (QuestCode3 ? "enabled" : "disabled"));

  } else if ((is_abbrev(arg, "questcode4") || is_abbrev(arg, "quest4")) && hasWizPower(POWER_TOGGLE)) {
    QuestCode4 = !QuestCode4;
    sendTo(fmt("Questcode 4 is now %s.\n\r") % (QuestCode4 ? "in use" : "off"));
    vlogf(LOG_MISC,fmt("%s has %s questcode 4.") % getName() %
	  (QuestCode4 ? "enabled" : "disabled"));

  } else if(is_abbrev(arg, "timequeries") && hasWizPower(POWER_TOGGLE)){
    timeQueries = !timeQueries;
    sendTo(fmt("DB query timing is now %s.\n\r") % (timeQueries ? "activated" : "deactivated"));
    vlogf(LOG_MISC,fmt("%s has %s DB query timing.") % getName() %
	  (timeQueries ? "enabled" : "disabled"));
  } else if (is_abbrev(arg, "pcmobs") && hasWizPower(POWER_TOGGLE)) {
    AllowPcMobs = !AllowPcMobs;
    sendTo(fmt("You have now %s mob-named pcs.\n\r") %
              (AllowPcMobs ? "allowed" : "disallowed"));
    vlogf(LOG_MISC, fmt("%s has turned mob/pcs mode %s.") %  getName() % 
              (AllowPcMobs ? "on" : "off"));
  } else if (is_abbrev(arg, "clients") && hasWizPower(POWER_TOGGLE)) {
    Clients = !Clients;
    sendTo(fmt("You have now %s clients.\n\r") % (Clients ? "allowed" : "disallowed"));
    vlogf(LOG_MISC, fmt("%s has turned client mode %s.") %  getName() % 
	  (Clients ? "on" : "off"));

    if (!Clients) {
      sendTo("Severing current client connections.\n\r");
      Descriptor *d, *dn;
      for (d = descriptor_list; d; d = dn) {
        dn = d->next;
        if (d->m_bIsClient) {
          d->writeToQ("Link severed by admin.\n\r");
          sendTo(COLOR_MOBS, fmt("Disconnecting client use by %s.\n\r") % (d->character ? d->character->getName() : "Unknown"));
          delete d;
        }
      }
    }
  
  } else if (is_abbrev(arg, "nuke") && hasWizPower(POWER_TOGGLE)) {
    nuke_inactive_mobs = !nuke_inactive_mobs;
    sendTo(fmt("Mobs in inactive zones are now %s.\n\r") % 
	   (nuke_inactive_mobs ? "nuked" : "preserved"));
    vlogf(LOG_MISC, fmt("%s has turned nuke mode %s.") %  getName() %
             (nuke_inactive_mobs ? "on" : "off"));
    unsigned int zone;
    for (zone = 1; zone < zone_table.size(); zone++) {
      zone_table[zone].zone_value = (nuke_inactive_mobs ? 1 : -1);
    }
  } else {
    sendTo("Unrecognized toggle.  Try toggle with no arguments for a list.\n\r");
    return;
  }
}
