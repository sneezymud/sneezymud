#include "monster.h"
#include "database.h"
#include "extern.h"

int tattooArtist(TBeing* ch, cmdTypeT cmd, const char* arg, TMonster* myself,
  TObj*) {
  // Available animals for tattoos (single stat animals)
  const char* single_animals[] = {
    "lion", "elephant", "bear", "snake", "mongoose",
    "crow", "rat", "owl", "ladybug", "eagle", "capybara", NULL
  };

  // Available animals for tattoos (dual stat animals)
  const char* dual_animals[] = {
    "rhinoceros", "owlbear", "tiger", "horse", "gorilla", "alligator", "beaver", "shark", "panda", "donkey", "dragon",
    "boar", "orca", "lynx", "caterpillar", "turtle", "bull", "hound", "bison", "cow", "moose",
    "coyote", "armadillo", "porcupine", "giraffe", "hippopotamus", "tortoise", "elk", "crocodile", "panther",
    "monkey", "dolphin", "raccoon", "hawk", "rabbit", "badger", "wyvern", "ocelot",
    "magpie", "fox", "sparrow", "antelope", "mouse", "cricket", "ferret",
    "raven", "spider", "heron", "orangutan", "crane", "cheetah",
    "rooster", "wolf", "pigeon", "toad", "squirrel",
    "octopus", "eel", "goldfish", "barracuda",
    "meerkat", "frog", "falcon",
    "peacock", "kangaroo", NULL
  };

  // Available colors for tattoos
  const char* colors[] = {
    "blue", "purple", "green", "yellow", "grey", "gray", "red", "white", "black", NULL
  };

  // Color codes for display
  const char* color_codes[] = {
    "<b>", "<p>", "<g>", "<y>", "<k>", "<k>", "<r>", "<w>", "<k>", NULL
  };

  // Adjectives by stat
  const char* str_adj[] = {"a scary", "a raging", "a muscular", "a fierce", "a jacked", NULL};
  const char* con_adj[] = {"a tough", "a plump", "a thick", "a big", "a broad", NULL};
  const char* bra_adj[] = {"a heavy", "a bold", "a bright", NULL};
  const char* dex_adj[] = {"a nimble", "a skilled", "an acrobatic", NULL};
  const char* agi_adj[] = {"a flowing", "a wispy", "a whimsical", NULL};
  const char* int_adj[] = {"a smart", "a bookish", "a learned", NULL};
  const char* wis_adj[] = {"a noble", "a clever", "a thoughtful", NULL};
  const char* foc_adj[] = {"a sharp", "a determined", "an intense", NULL};
  const char* per_adj[] = {"a wide-eyed", "a knowing", "an astute", NULL};
  const char* cha_adj[] = {"a smiling", "a friendly", "a charming", NULL};
  const char* kar_adj[] = {"a lucky", "a fiendish", "a laughing", NULL};
  const char* spe_adj[] = {"a swift", "a sprightly", "a sporty", NULL};

  // Phrases by stat
  const char* str_phrases[] = {"flexing wildly", "roaring monstrously", "looking mighty", NULL};
  const char* con_phrases[] = {"standing tall", "sitting comfortably", "resting cozily", NULL};
  const char* bra_phrases[] = {"looking immovable", "of immense girth", "with wide shoulders", NULL};
  const char* dex_phrases[] = {"showing off", "poised to strike", "looking deadly", NULL};
  const char* agi_phrases[] = {"prancing about", "striding confidently", "springing into action", NULL};
  const char* int_phrases[] = {"with an open book", "winking devilishly", "wreathed in stars", NULL};
  const char* wis_phrases[] = {"blessed by the gods", "lined by visions of beyond", "in deep thought", NULL};
  const char* foc_phrases[] = {"staring intently", "with laser focus", "concentrating deeply", NULL};
  const char* per_phrases[] = {"looking about", "with perked up ears", "looking alert", NULL};
  const char* cha_phrases[] = {"surrounded by friends", "in a fit of laughter", "beckoning come hither", "winking coyishly", NULL};
  const char* kar_phrases[] = {"looking unbothered", "lounging lavishly", "flipping a coin", "rolling dice", NULL};
  const char* spe_phrases[] = {"zooming about", "looking fast", "dodging away from danger", NULL};

  char buf[256], buf2[256], buf3[256];
  int i;

  if (cmd == CMD_LIST) {
    myself->doTell(ch->getName(),
      "I charge 100000 talens for a tattoo. They are permanent.");
    myself->doTell(ch->getName(),
      "Syntax: BUY <animal> <color> <body_location>");
    myself->doTell(ch->getName(), "");

    myself->doTell(ch->getName(), "<g>Available Animals:<1>");
    for (i = 0; single_animals[i]; ++i) {
      myself->doTell(ch->getName(), format("  A %s") % single_animals[i]);
    }

    myself->doTell(ch->getName(), "");
    for (i = 0; dual_animals[i]; ++i) {
      myself->doTell(ch->getName(), format("  A %s") % dual_animals[i]);
    }

    myself->doTell(ch->getName(), "");
    myself->doTell(ch->getName(), "<g>Available Colors:<1>");
    for (i = 0; colors[i]; ++i) {
      myself->doTell(ch->getName(),
        format("  %s%s<1>") % color_codes[i] % colors[i]);
    }

    myself->doTell(ch->getName(), "");
    myself->doTell(ch->getName(),
      "Example: BUY dragon blue arm");

    return TRUE;
  } else if (cmd == CMD_BUY) {
    // Parse: BUY <animal> <color> <location>
    arg = one_argument(arg, buf, cElements(buf));   // animal
    arg = one_argument(arg, buf2, cElements(buf2)); // color
    one_argument(arg, buf3, cElements(buf3));       // location

    // Validate animal
    bool valid_animal = false;
    for (i = 0; single_animals[i]; ++i) {
      if (!strcasecmp(buf, single_animals[i])) {
        valid_animal = true;
        break;
      }
    }
    if (!valid_animal) {
      for (i = 0; dual_animals[i]; ++i) {
        if (!strcasecmp(buf, dual_animals[i])) {
          valid_animal = true;
          break;
        }
      }
    }

    if (!valid_animal) {
      myself->doTell(ch->getName(),
        "I don't know how to tattoo that animal. Try LIST to see available animals.");
      return FALSE;
    }

    // Validate color
    bool valid_color = false;
    sstring color_code = "";
    for (i = 0; colors[i]; ++i) {
      if (!strcasecmp(buf2, colors[i])) {
        valid_color = true;
        color_code = color_codes[i];
        break;
      }
    }

    if (!valid_color) {
      myself->doTell(ch->getName(),
        "I don't have that color. Try LIST to see available colors.");
      return FALSE;
    }

    // Validate location
    wearSlotT slot = WEAR_NOWHERE;
    int slot_i;
    if ((slot_i = old_search_block(buf3, 0, strlen(buf3), bodyParts, 0)) > 0) {
      slot = wearSlotT(--slot_i);
      if (!ch->slotChance(slot)) {
        myself->doTell(ch->getName(),
          "I need a valid body location. Try: head, arm, body, etc.");
        return FALSE;
      }
    } else {
      myself->doTell(ch->getName(),
        "I need a valid body location. Try: head, arm, body, etc.");
      return FALSE;
    }

    // Check for leg restriction
    if (slot == WEAR_LEG_R || slot == WEAR_LEG_L) {
      myself->doTell(ch->getName(),
        "Sorry, it is against my policy to tattoo legs.");
      myself->doTell(ch->getName(),
        "It's not like you're gonna run around pantless to show it off "
        "anyway!");
      return FALSE;
    }

    TDatabase db(DB_SNEEZY);
    db.query("select 1 from tattoos where name='%s' and location=%i",
      ch->getName().c_str(), slot);

    if (db.fetchRow()) {
      myself->doTell(ch->getName(), "You already have a tattoo there.");
      return FALSE;
    }

    if (ch->getMoney() < 100000) {
      myself->doTell(ch->getName(),
        "Hey buddy, you don't even have the money! I need 100000 talens!");
      return FALSE;
    }

    ch->setMoney(ch->getMoney() - 100000);

    myself->doEmote("takes your money and carefully inks out the tattoo.");

    // Determine which stats this animal provides for independent adjective and phrase selection
    const char** adj_list1 = NULL;
    const char** adj_list2 = NULL;
    const char** phrase_list1 = NULL;
    const char** phrase_list2 = NULL;

    // Map animals to their stat types - store both stats for dual-stat animals
    // STR combinations
    if (!strcasecmp(buf, "rhinoceros")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
      adj_list2 = bra_adj; phrase_list2 = bra_phrases;
    } else if (!strcasecmp(buf, "owlbear")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
      adj_list2 = con_adj; phrase_list2 = con_phrases;
    } else if (!strcasecmp(buf, "tiger")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
      adj_list2 = dex_adj; phrase_list2 = dex_phrases;
    } else if (!strcasecmp(buf, "horse")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
      adj_list2 = agi_adj; phrase_list2 = agi_phrases;
    } else if (!strcasecmp(buf, "gorilla")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
      adj_list2 = int_adj; phrase_list2 = int_phrases;
    } else if (!strcasecmp(buf, "alligator")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
      adj_list2 = wis_adj; phrase_list2 = wis_phrases;
    } else if (!strcasecmp(buf, "beaver")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
      adj_list2 = foc_adj; phrase_list2 = foc_phrases;
    } else if (!strcasecmp(buf, "shark")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
      adj_list2 = per_adj; phrase_list2 = per_phrases;
    } else if (!strcasecmp(buf, "panda")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
      adj_list2 = cha_adj; phrase_list2 = cha_phrases;
    } else if (!strcasecmp(buf, "donkey")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
      adj_list2 = kar_adj; phrase_list2 = kar_phrases;
    } else if (!strcasecmp(buf, "dragon")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
      adj_list2 = spe_adj; phrase_list2 = spe_phrases;

    // BRA combinations
    } else if (!strcasecmp(buf, "boar")) {
      adj_list1 = bra_adj; phrase_list1 = bra_phrases;
      adj_list2 = con_adj; phrase_list2 = con_phrases;
    } else if (!strcasecmp(buf, "orca")) {
      adj_list1 = bra_adj; phrase_list1 = bra_phrases;
      adj_list2 = dex_adj; phrase_list2 = dex_phrases;
    } else if (!strcasecmp(buf, "lynx")) {
      adj_list1 = bra_adj; phrase_list1 = bra_phrases;
      adj_list2 = agi_adj; phrase_list2 = agi_phrases;
    } else if (!strcasecmp(buf, "caterpillar")) {
      adj_list1 = bra_adj; phrase_list1 = bra_phrases;
      adj_list2 = int_adj; phrase_list2 = int_phrases;
    } else if (!strcasecmp(buf, "turtle")) {
      adj_list1 = bra_adj; phrase_list1 = bra_phrases;
      adj_list2 = wis_adj; phrase_list2 = wis_phrases;
    } else if (!strcasecmp(buf, "bull")) {
      adj_list1 = bra_adj; phrase_list1 = bra_phrases;
      adj_list2 = foc_adj; phrase_list2 = foc_phrases;
    } else if (!strcasecmp(buf, "hound")) {
      adj_list1 = bra_adj; phrase_list1 = bra_phrases;
      adj_list2 = per_adj; phrase_list2 = per_phrases;
    } else if (!strcasecmp(buf, "bison")) {
      adj_list1 = bra_adj; phrase_list1 = bra_phrases;
      adj_list2 = cha_adj; phrase_list2 = cha_phrases;
    } else if (!strcasecmp(buf, "cow")) {
      adj_list1 = bra_adj; phrase_list1 = bra_phrases;
      adj_list2 = kar_adj; phrase_list2 = kar_phrases;
    } else if (!strcasecmp(buf, "moose")) {
      adj_list1 = bra_adj; phrase_list1 = bra_phrases;
      adj_list2 = spe_adj; phrase_list2 = spe_phrases;
    // Single stat animals
    } else if (!strcasecmp(buf, "lion")) {
      adj_list1 = str_adj; phrase_list1 = str_phrases;
    } else if (!strcasecmp(buf, "elephant")) {
      adj_list1 = bra_adj; phrase_list1 = bra_phrases;
    } else if (!strcasecmp(buf, "bear")) {
      adj_list1 = con_adj; phrase_list1 = con_phrases;
    } else if (!strcasecmp(buf, "snake")) {
      adj_list1 = dex_adj; phrase_list1 = dex_phrases;
    } else if (!strcasecmp(buf, "crow")) {
      adj_list1 = int_adj; phrase_list1 = int_phrases;
    } else if (!strcasecmp(buf, "owl")) {
      adj_list1 = wis_adj; phrase_list1 = wis_phrases;
    } else if (!strcasecmp(buf, "rat")) {
      adj_list1 = foc_adj; phrase_list1 = foc_phrases;
    } else if (!strcasecmp(buf, "eagle")) {
      adj_list1 = per_adj; phrase_list1 = per_phrases;
    } else if (!strcasecmp(buf, "capybara")) {
      adj_list1 = cha_adj; phrase_list1 = cha_phrases;
    } else if (!strcasecmp(buf, "ladybug")) {
      adj_list1 = kar_adj; phrase_list1 = kar_phrases;
    } else if (!strcasecmp(buf, "mongoose")) {
      adj_list1 = spe_adj; phrase_list1 = spe_phrases;

    // CON combinations
    } else if (!strcasecmp(buf, "coyote")) {
      adj_list1 = con_adj; phrase_list1 = con_phrases;
      adj_list2 = dex_adj; phrase_list2 = dex_phrases;
    } else if (!strcasecmp(buf, "armadillo")) {
      adj_list1 = con_adj; phrase_list1 = con_phrases;
      adj_list2 = agi_adj; phrase_list2 = agi_phrases;
    } else if (!strcasecmp(buf, "porcupine")) {
      adj_list1 = con_adj; phrase_list1 = con_phrases;
      adj_list2 = wis_adj; phrase_list2 = wis_phrases;
    } else if (!strcasecmp(buf, "giraffe")) {
      adj_list1 = con_adj; phrase_list1 = con_phrases;
      adj_list2 = per_adj; phrase_list2 = per_phrases;
    } else if (!strcasecmp(buf, "hippopotamus")) {
      adj_list1 = con_adj; phrase_list1 = con_phrases;
      adj_list2 = cha_adj; phrase_list2 = cha_phrases;
    } else if (!strcasecmp(buf, "tortoise")) {
      adj_list1 = con_adj; phrase_list1 = con_phrases;
      adj_list2 = kar_adj; phrase_list2 = kar_phrases;
    } else if (!strcasecmp(buf, "elk")) {
      adj_list1 = con_adj; phrase_list1 = con_phrases;
      adj_list2 = spe_adj; phrase_list2 = spe_phrases;
    } else if (!strcasecmp(buf, "crocodile")) {
      adj_list1 = con_adj; phrase_list1 = con_phrases;
      adj_list2 = int_adj; phrase_list2 = int_phrases;
    } else if (!strcasecmp(buf, "panther")) {
      adj_list1 = con_adj; phrase_list1 = con_phrases;
      adj_list2 = foc_adj; phrase_list2 = foc_phrases;

    // DEX combinations
    } else if (!strcasecmp(buf, "monkey")) {
      adj_list1 = dex_adj; phrase_list1 = dex_phrases;
      adj_list2 = agi_adj; phrase_list2 = agi_phrases;
    } else if (!strcasecmp(buf, "dolphin")) {
      adj_list1 = dex_adj; phrase_list1 = dex_phrases;
      adj_list2 = int_adj; phrase_list2 = int_phrases;
    } else if (!strcasecmp(buf, "raccoon")) {
      adj_list1 = dex_adj; phrase_list1 = dex_phrases;
      adj_list2 = foc_adj; phrase_list2 = foc_phrases;
    } else if (!strcasecmp(buf, "hawk")) {
      adj_list1 = dex_adj; phrase_list1 = dex_phrases;
      adj_list2 = per_adj; phrase_list2 = per_phrases;
    } else if (!strcasecmp(buf, "rabbit")) {
      adj_list1 = dex_adj; phrase_list1 = dex_phrases;
      adj_list2 = cha_adj; phrase_list2 = cha_phrases;
    } else if (!strcasecmp(buf, "badger")) {
      adj_list1 = dex_adj; phrase_list1 = dex_phrases;
      adj_list2 = kar_adj; phrase_list2 = kar_phrases;
    } else if (!strcasecmp(buf, "wyvern")) {
      adj_list1 = dex_adj; phrase_list1 = dex_phrases;
      adj_list2 = spe_adj; phrase_list2 = spe_phrases;
    } else if (!strcasecmp(buf, "ocelot")) {
      adj_list1 = dex_adj; phrase_list1 = dex_phrases;
      adj_list2 = wis_adj; phrase_list2 = wis_phrases;

    // AGI combinations
    } else if (!strcasecmp(buf, "magpie")) {
      adj_list1 = agi_adj; phrase_list1 = agi_phrases;
      adj_list2 = int_adj; phrase_list2 = int_phrases;
    } else if (!strcasecmp(buf, "fox")) {
      adj_list1 = agi_adj; phrase_list1 = agi_phrases;
      adj_list2 = wis_adj; phrase_list2 = wis_phrases;
    } else if (!strcasecmp(buf, "sparrow")) {
      adj_list1 = agi_adj; phrase_list1 = agi_phrases;
      adj_list2 = foc_adj; phrase_list2 = foc_phrases;
    } else if (!strcasecmp(buf, "antelope")) {
      adj_list1 = agi_adj; phrase_list1 = agi_phrases;
      adj_list2 = per_adj; phrase_list2 = per_phrases;
    } else if (!strcasecmp(buf, "mouse")) {
      adj_list1 = agi_adj; phrase_list1 = agi_phrases;
      adj_list2 = cha_adj; phrase_list2 = cha_phrases;
    } else if (!strcasecmp(buf, "cricket")) {
      adj_list1 = agi_adj; phrase_list1 = agi_phrases;
      adj_list2 = kar_adj; phrase_list2 = kar_phrases;
    } else if (!strcasecmp(buf, "ferret")) {
      adj_list1 = agi_adj; phrase_list1 = agi_phrases;
      adj_list2 = spe_adj; phrase_list2 = spe_phrases;

    // INT combinations
    } else if (!strcasecmp(buf, "raven")) {
      adj_list1 = int_adj; phrase_list1 = int_phrases;
      adj_list2 = wis_adj; phrase_list2 = wis_phrases;
    } else if (!strcasecmp(buf, "spider")) {
      adj_list1 = int_adj; phrase_list1 = int_phrases;
      adj_list2 = foc_adj; phrase_list2 = foc_phrases;
    } else if (!strcasecmp(buf, "heron")) {
      adj_list1 = int_adj; phrase_list1 = int_phrases;
      adj_list2 = per_adj; phrase_list2 = per_phrases;
    } else if (!strcasecmp(buf, "orangutan")) {
      adj_list1 = int_adj; phrase_list1 = int_phrases;
      adj_list2 = cha_adj; phrase_list2 = cha_phrases;
    } else if (!strcasecmp(buf, "crane")) {
      adj_list1 = int_adj; phrase_list1 = int_phrases;
      adj_list2 = kar_adj; phrase_list2 = kar_phrases;
    } else if (!strcasecmp(buf, "cheetah")) {
      adj_list1 = int_adj; phrase_list1 = int_phrases;
      adj_list2 = spe_adj; phrase_list2 = spe_phrases;

    // WIS combinations
    } else if (!strcasecmp(buf, "rooster")) {
      adj_list1 = wis_adj; phrase_list1 = wis_phrases;
      adj_list2 = foc_adj; phrase_list2 = foc_phrases;
    } else if (!strcasecmp(buf, "wolf")) {
      adj_list1 = wis_adj; phrase_list1 = wis_phrases;
      adj_list2 = per_adj; phrase_list2 = per_phrases;
    } else if (!strcasecmp(buf, "pigeon")) {
      adj_list1 = wis_adj; phrase_list1 = wis_phrases;
      adj_list2 = cha_adj; phrase_list2 = cha_phrases;
    } else if (!strcasecmp(buf, "toad")) {
      adj_list1 = wis_adj; phrase_list1 = wis_phrases;
      adj_list2 = kar_adj; phrase_list2 = kar_phrases;
    } else if (!strcasecmp(buf, "squirrel")) {
      adj_list1 = wis_adj; phrase_list1 = wis_phrases;
      adj_list2 = spe_adj; phrase_list2 = spe_phrases;

    // FOC combinations
    } else if (!strcasecmp(buf, "octopus")) {
      adj_list1 = foc_adj; phrase_list1 = foc_phrases;
      adj_list2 = per_adj; phrase_list2 = per_phrases;
    } else if (!strcasecmp(buf, "eel")) {
      adj_list1 = foc_adj; phrase_list1 = foc_phrases;
      adj_list2 = cha_adj; phrase_list2 = cha_phrases;
    } else if (!strcasecmp(buf, "goldfish")) {
      adj_list1 = foc_adj; phrase_list1 = foc_phrases;
      adj_list2 = kar_adj; phrase_list2 = kar_phrases;
    } else if (!strcasecmp(buf, "barracuda")) {
      adj_list1 = foc_adj; phrase_list1 = foc_phrases;
      adj_list2 = spe_adj; phrase_list2 = spe_phrases;

    // PER combinations
    } else if (!strcasecmp(buf, "meerkat")) {
      adj_list1 = per_adj; phrase_list1 = per_phrases;
      adj_list2 = cha_adj; phrase_list2 = cha_phrases;
    } else if (!strcasecmp(buf, "frog")) {
      adj_list1 = per_adj; phrase_list1 = per_phrases;
      adj_list2 = kar_adj; phrase_list2 = kar_phrases;
    } else if (!strcasecmp(buf, "falcon")) {
      adj_list1 = per_adj; phrase_list1 = per_phrases;
      adj_list2 = spe_adj; phrase_list2 = spe_phrases;

    // CHA combinations
    } else if (!strcasecmp(buf, "peacock")) {
      adj_list1 = cha_adj; phrase_list1 = cha_phrases;
      adj_list2 = kar_adj; phrase_list2 = kar_phrases;
    } else if (!strcasecmp(buf, "kangaroo")) {
      adj_list1 = cha_adj; phrase_list1 = cha_phrases;
      adj_list2 = spe_adj; phrase_list2 = spe_phrases;

    } else {
      // Default fallback for any other animals
      adj_list1 = str_adj; phrase_list1 = str_phrases;
    }

    // Independently select adjective and phrase from available stats
    const char* chosen_adj;
    const char* chosen_phrase;

    // Choose adjective randomly from available stats
    if (adj_list2 && ::number(0, 1)) {
      // Use second stat for adjective
      int adj_count = 0;
      while (adj_list2[adj_count]) adj_count++;
      chosen_adj = adj_list2[::number(0, adj_count - 1)];
    } else {
      // Use first stat for adjective
      int adj_count = 0;
      while (adj_list1[adj_count]) adj_count++;
      chosen_adj = adj_list1[::number(0, adj_count - 1)];
    }

    // Choose phrase randomly from available stats (independent of adjective choice)
    if (phrase_list2 && ::number(0, 1)) {
      // Use second stat for phrase
      int phrase_count = 0;
      while (phrase_list2[phrase_count]) phrase_count++;
      chosen_phrase = phrase_list2[::number(0, phrase_count - 1)];
    } else {
      // Use first stat for phrase
      int phrase_count = 0;
      while (phrase_list1[phrase_count]) phrase_count++;
      chosen_phrase = phrase_list1[::number(0, phrase_count - 1)];
    }

    // Create the tattoo description dynamically
    sstring tattoo_desc = format("A tattoo of %s %s%s<1> %s %s.") %
                          chosen_adj % color_code % buf2 % buf % chosen_phrase;

    db.query(
      "insert into tattoos (name, tattoo, location) values ('%s', '%s', %i)",
      ch->getName().c_str(), tattoo_desc.c_str(), slot);

    myself->doSay(format("There you go, all set! Your %s %s looks great!") %
                  buf2 % buf);

    return TRUE;
  }

  return FALSE;
}
