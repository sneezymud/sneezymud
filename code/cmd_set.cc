////////////////////////////////////////////////////////////////////////// 
//
//      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
//
//      "cmd_set.cc" - Functions related to the @set command
//  
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "statistics.h"

void TBeing::doSet(const char *)
{
  sendTo("You can't, you're a mob.\n\r");
}

void TPerson::doSet(const char *argument)
{
  char field[20], namebuf[20], parmstr[50];
  char obj_name[128];
  TBeing *mob = NULL;
  int i, foundNum = 0, doneBasic = 0,parm = 0, value = 0, parm2 = 0;
  int parm3 = 0, amt = 0, initial = 0;
  float percent;
  char buf2[256];
  sstring buf;
  TObj *obj;
  charFile st;
  int faction_num;

  if (!hasWizPower(POWER_SET)) {
    sendTo("You don't have the power to @set\n\r");
    return;
  }
  argument = one_argument(argument, field);
  argument = one_argument(argument, namebuf);
  argument = one_argument(argument, parmstr);

  if (!(mob = get_char_vis_world(this, namebuf, NULL, EXACT_YES)) && 
      !(mob = get_char_vis_world(this, namebuf, NULL, EXACT_NO))) {
    sendTo(fmt("I don't see '%s' here.\n\r") % namebuf);
    return;
  }
  if (mob->isPc() && mob->hasWizPower(POWER_SET_IMP_POWER) && !hasWizPower(POWER_SET_IMP_POWER)) {
    sendTo("You can't do that!\n\r");
    return;
  } 

  if (!strcasecmp(field, "character")) {
#if 1 
    if (sscanf(parmstr, " %d ", &parm) != 1) {
      sendTo("Syntax: @set character <char name> <level> <class> <learn>(optional)\n\r");
      return;
    }
    argument = one_argument(argument, parmstr);
    if (sscanf(parmstr, "%d", &parm2) != 1) {
      sendTo("Syntax: @set character <char name> <level> <class> <learn>(optional)\n\r");
      return;
    }

    if (*argument) {
      argument = one_argument(argument, parmstr);
      if (sscanf(parmstr, "%d", &parm3) != 1) {
        sendTo("Syntax: @set character <char name> <level> <class> <learn>(optional)\n\r");
       return;
      }

    }
    
    TPerson *tper = dynamic_cast<TPerson *>(mob);
    if (!tper || !tper->desc) {
      sendTo("Mobiles have no use for character generation!\n\r");
      return;
    }
    if (parm < 0) {
      sendTo("No negative numbers please!\n\r");
      return;
    }

    if (parm > MAX_MORT) {
      sendTo("You cannot use this to set someone to an immort level.\n\r");
      return;
    }
    if ((parm2 < MIN_CLASS_IND) || (parm2 >= MAX_CLASSES)) {
      sendTo("You must set a valid class number.\n\r");
      return;
    }
    classIndT p2ci = classIndT(parm2);
    initial = tper->getLevel(p2ci);
    if (initial <= 0) {
      sendTo("You must set a class that the player has!\n\r");
      return;
    }
    if (initial > parm) {
      sendTo("For now can't use this function to drop levels!\n\r");
      return;
    }

#if FACTIONS_IN_USE
    factionTypeT ij;
    for(ij = MIN_FACTION; ij < MAX_FACTIONS; ij++) {  
      percent = 2 * parm;
      if (percent > tper->getPercX(ij)) { 
        min(percent, 90.0);
        max(percent, 0.0);
        tper->setPercX((double) percent, ij);
        continue;
      }
     }
#endif
     double f_amt = getExpClassLevel(p2ci, parm) + 0.0001;
     tper->setExp(f_amt); 
     amt = tper->checkDoneBasic(tper, p2ci, FALSE, TRUE);
     for (i = initial; i < parm; i++) {
       if (!tper->player.doneBasic[p2ci]) {
         value = amt + tper->getPracs(p2ci);
         if (value > (MAX_DISC_LEARNEDNESS + tper->getCombatPrereqNumber(p2ci))) {
           if (!tper->player.doneBasic[p2ci]) {
             tper->player.doneBasic[p2ci] = tper->getLevel(p2ci);
             doneBasic = tper->getLevel(p2ci);
           }
         }
       }
       tper->fixClientPlayerLists(TRUE);
       tper->advanceLevel(p2ci);
       tper->fixClientPlayerLists(FALSE);
       tper->setTitle(false);
       tper->setSelectToggles(this, p2ci, SILENT_YES);
     }
     tper->advanceSelectDisciplines(p2ci, (parm - initial), SILENT_YES);
     if (parm3) {
     }
#if 0
// set the players time
     if (tper->isPc && tper->desc) {
       (float) val = power_level_number(tper->GetMaxLevel());
       value = max((tper->player.time.played + ((long) (time(0) - tper->player.time.logon)), value);
       tper->player.time.played = value;
     }
#endif
     sendTo(COLOR_MOBS, fmt("You have set %s to level %d.\n\r") % tper->getName() % parm);
     sendTo(fmt("DoneBasic is set at %d for that class.\n\r") % tper->player.doneBasic[p2ci]);
     tper->sendTo(COLOR_MOBS, fmt("You have been set %s to level %d.\n\r") % tper->getName() % parm);
     tper->doSave(SILENT_YES);
#endif
  } else if (is_abbrev(field, "class")) {
    sscanf(parmstr, "%d", &parm);
    if (parm <= 0 || parm >= (1<<MAX_CLASSES)) {
      sendTo(fmt("Class must be in the range 1-%d.\n\r") % ((1<<MAX_CLASSES)-1));
      return;
    }
    mob->setClass(parm);
    if (mob->isImmortal()) {
      if (mob->discs) {
        delete mob->discs;
        mob->discs = NULL;
        mob->assignDisciplinesClass();
        mob->affectTotal();
        mob->doSave(SILENT_YES);
      }
    }
    sendTo(COLOR_MOBS, fmt("You set %s's class to %d.\n\r") % mob->getName() % mob->getClass());
    return;
  } else if (is_abbrev(field, "playtime")) {
    sscanf(parmstr, "%d", &parm);
    mob->player.time.played += parm * SECS_PER_REAL_MIN;
    sendTo(COLOR_MOBS, fmt("You add %d minutes to %s's playing time.\n\r") % parm % mob->getName());
    return;
  } else if (is_abbrev(field, "exp")) {
    sscanf(parmstr, "%f", &percent);
    mob->setExp(percent);
    sendTo(COLOR_MOBS, fmt("You set %s's exp to %.2f.\n\r") % mob->getName() % mob->getExp());
    mob->doSave(SILENT_NO);
    return;
  } else if (is_abbrev(field, "newfaction")) {
    buf=fmt("%s%s") % parmstr % argument;
    TFaction *f = NULL;
    f = get_faction(buf.c_str());
    if(!f) {
      sendTo("No such factions\n\r");
      return;
    }
    mob->faction.whichfaction = f->ID;
    mob->faction.rank = f->ranks;
    sendTo(COLOR_BASIC,fmt("%s faction set to %s (%d), rank set to %s. (lowest possible)\n\r") % mob->getName() %
	   mob->newfaction()->getName() % mob->newfaction()->ID % mob->rank());
    mob->doSave(SILENT_NO);
    mob->saveFactionStats();
    return;
  } else if (is_abbrev(field, "rank")) {
    sscanf(parmstr, "%d", &parm);
    if (!mob->newfaction() || parm < 0 || parm >= mob->newfaction()->ranks) {
      sendTo("Target must have a valid faction, and rank must be a valid rank in that faction.\n\r");
      return;
    }
    mob->faction.rank = parm;
    sendTo(COLOR_MOBS, fmt("You set %s's rank to %s.\n\r") % mob->getName() % mob->rank());
    mob->doSave(SILENT_NO);
    mob->saveFactionStats();
    return;
  } else if (is_abbrev(field, "title")) {
    TPerson *tper = dynamic_cast<TPerson *>(mob);
    if (!tper) {
      sendTo("Setting titles can only be done to PCs.\n\r");
      return;
    }
    delete [] tper->title;
    buf=fmt("%s%s") % parmstr % argument;
    tper->title = mud_str_dup(buf.c_str());
    buf=tper->parseTitle(desc);
    sendTo(COLOR_MOBS, fmt("%s's title is now: %s\n\r") % 
	   tper->getName() % tper->title);
    return;
  } else if (is_abbrev(field, "toggle")) {
    sscanf(parmstr, "%d", &parm);
    if (parm <= 0 || parm >= MAX_TOG_INDEX) {
      sendTo("Outside of range.\n\r");
      return;
    }
    if(parm == TOG_PSIONICIST && strcmp("Peel", name)){
      sendTo("Don't mess with the psionicist toggle.  It's meant to be permanent and if you try to untoggle it, you'll end up corrupting the pfile and crashing the mud.\n\r");
      return;
    }

    if (mob->isPc()) {
      if (!mob->hasQuestBit(parm)) {
        sendTo(fmt("Toggle Set: %s\n\r") % TogIndex[parm].name);
        mob->setQuestBit(parm);
      } else {
        sendTo(fmt("Toggle Unset: %s\n\r") % TogIndex[parm].name);
        mob->remQuestBit(parm);
      }
    } else {
      sendTo("You can't toggle quest bits for mobs.\n\r");
    }
    return;
  } else if (is_abbrev(field, "wizpower")) {
    unsigned int prm;
    sscanf(parmstr, "%u", &prm);
    if (prm > MAX_POWER_INDEX || prm <= 0) {
      setWizPowers(this, mob, parmstr);
      return;
    }
    // would seem like we'd want to map, however, the number that is
    // intuitive to use, comes from "power" command
    // this number is simply 1 larger than the enum in use
    wizPowerT wpt = wizPowerT(prm-1);
    if (mob->isPc() || mob->GetMaxLevel() <= MAX_MORT) {
      if (wpt == POWER_IDLED && !hasWizPower(POWER_WIZARD)) {
        sendTo("You do not have the authority to modify this power.\n\r");
        return;
      }
      if (!mob->hasWizPower(wpt)) {
        sendTo(fmt("Wiz-Power Set: %s\n\r") % getWizPowerName(wpt));
        mob->setWizPower(wpt);
        mob->doSave(SILENT_NO);
      } else {
        sendTo(fmt("Wiz-Power Removed: %s\n\r") % getWizPowerName(wpt));
        mob->remWizPower(wpt);
      }
    } else {
      sendTo("You can't toggle Wiz-Powers for non-immortals.\n\r");
    }
    return;
  } else if (is_abbrev(field, "faction")) {
    factionTypeT fnum = factionNumber(parmstr);
    if (fnum != FACT_UNDEFINED) {
      mob->setFaction(fnum);
      sendTo(COLOR_MOBS, fmt("You just changed %s's faction to %s.\n\r") %mob->getName() %
            FactionInfo[fnum].faction_name);
    } else {
      sendTo("That is an invalid faction.\n\r");
      sendTo("Syntax: @set faction <person> <faction name>\n\r");
      return;
    }
  } else if (is_abbrev(field, "factionpower")) {
    // @set power <x> <faction> <num>
    if ((faction_num = factionNumber(parmstr)) == -1) {
      sendTo("Bogus faction.\n\r");
      sendTo("Syntax: @set power <x> <faction> <new power>\n\r");
      return;
    }
    argument = one_argument(argument, parmstr);
    sscanf(parmstr, "%f", &percent);
    if (percent < 0.0) {
      sendTo("Invalid power.  Positive only.\n\r");
      sendTo("Syntax: @set power <x> <faction> <new power>\n\r");
      return;
    }
    FactionInfo[faction_num].faction_power = percent;
    sendTo(fmt("You set %s's power to %.2f.\n\r") % FactionInfo[faction_num].faction_name % FactionInfo[faction_num].faction_power);
  } else if (is_abbrev(field, "factionleader")) {
    // @set leader <x> <faction> <who> <num>
    if ((faction_num = factionNumber(parmstr)) == -1) {
      sendTo("Bogus faction.\n\r");
      sendTo("Syntax: @set leader <x> <faction> <leader> <leader slot>\n\r");
      sendTo("Syntax: @set leader <x> <faction> Noone <leader slot>\n\r");
      return;
    }
    argument = one_argument(argument, namebuf);
    argument = one_argument(argument, parmstr);
    parm = convertTo<int>(parmstr);
    strcpy(namebuf, sstring(namebuf).cap().c_str());

    if (strcmp("Noone", namebuf) && !load_char(namebuf, &st)) {
      sendTo("No such person exists.\n\r");
      sendTo("Syntax: @set leader <x> <faction> <leader> <leader slot>\n\r");
      sendTo("Syntax: @set leader <x> <faction> Noone <leader slot>\n\r");
      return;
    }
    if (parm < 0 || parm >= FACT_LEADER_SLOTS) {
      sendTo(fmt("Leader slot in range 0 to %d.\n\r") % (FACT_LEADER_SLOTS - 1));
      sendTo("Syntax: @set leader <x> <faction> <leader> <leader slot>\n\r");
      sendTo("Syntax: @set leader <x> <faction> Noone <leader slot>\n\r");
      return;
    }
    if (faction_num == FACT_NONE) {
      sendTo(fmt("%s don't have or want leaders.\n\r") %
         sstring(FactionInfo[FACT_NONE].faction_name).cap());
      return;
    }

    vlogf(LOG_FACT, fmt("Leader slot %d for faction %s changed.") %  parm %
           FactionInfo[faction_num].faction_name);
    vlogf(LOG_FACT, fmt("Changed from %s to %s.") % FactionInfo[faction_num].leader[parm] %
           namebuf);
    sendTo(fmt("You have set %s's leader %d to %s.\n\r") %
           FactionInfo[faction_num].faction_name % parm % namebuf);

    if (strcmp(namebuf, "Noone"))
      strcpy(FactionInfo[faction_num].leader[parm],namebuf);
    else
      strcpy(FactionInfo[faction_num].leader[parm],"");

    save_factions();
#if FACTIONS_IN_USE
  } else if (is_abbrev(field, "percent")) {
    if (GetMaxLevel() < 60) {
      sendTo("Not allowed.\n\r");
      return;
    }
    sscanf(parmstr, "%f", &percent);
    percent = min(percent, 100.0);
    percent = max(percent, 0.0);
    mob->setPerc((double) percent);
    sendTo("OK.   (be sure to set partial percx's too if you want permanent change)\n\r");
#endif
  } else if (is_abbrev(field, "terrain") || is_abbrev(field, "hometerrain")) {
    sscanf(parmstr, "%d", &parm);
    if (parm < 0 || parm >= MAX_HOME_TERS) {
      sendTo("Illegal value.\n\r");
      return;
    }
    // force proper ranges, anyone can have HOME_ER_NONE
    if (mob->getRace() == RACE_HUMAN) {
      if ((parm != HOME_TER_NONE) && 
          ((parm < HOME_TER_HUMAN_URBAN) || (parm > HOME_TER_HUMAN_MARINER))) {
        sendTo("Bad value for human territory type.\n\r");
        return;
      }
    } else if (mob->getRace() == RACE_ELVEN) {
      if ((parm != HOME_TER_NONE) && 
          ((parm < HOME_TER_ELF_URBAN) || (parm > HOME_TER_ELF_SEA))) {
        sendTo("Bad value for elf territory type.\n\r");
        return;
      }
    } else if (mob->getRace() == RACE_DWARF) {
      if ((parm != HOME_TER_NONE) && 
          ((parm < HOME_TER_DWARF_URBAN) || (parm > HOME_TER_DWARF_MOUNTAIN))) {
        sendTo("Bad value for dwarf territory type.\n\r");
        return;
      }
    } else if (mob->getRace() == RACE_GNOME) {
      if ((parm != HOME_TER_NONE) && 
          ((parm < HOME_TER_GNOME_URBAN) || (parm > HOME_TER_GNOME_SWAMP))) {
        sendTo("Bad value for gnome territory type.\n\r");
        return;
      }
    } else if (mob->getRace() == RACE_OGRE) {
      if ((parm != HOME_TER_NONE) && 
          ((parm < HOME_TER_OGRE_VILLAGER) || (parm > HOME_TER_OGRE_HILL))) {
        sendTo("Bad value for ogre territory type.\n\r");
        return;
      }
    } else if (mob->getRace() == RACE_HOBBIT) {
      if ((parm != HOME_TER_NONE) && 
          ((parm < HOME_TER_HOBBIT_URBAN) || (parm > HOME_TER_HOBBIT_MARITIME))) {
        sendTo("Bad value for hobbit territory type.\n\r");
        return;
      }
    } else {
      if ((parm != HOME_TER_NONE)) {
        sendTo("Bad value for non-civilized territory type.\n\r");
        return;
      }
    }
    mob->player.hometerrain = territoryT(parm);
    sendTo(COLOR_MOBS, fmt("%s grew up as a %s.\n\r") % mob->getName() %
                home_terrains[parm]);
#if FACTIONS_IN_USE
  } else if (is_abbrev(field, "percx")) {
    if (sscanf(parmstr, " %d ", &parm) != 1) {
      sendTo("Syntax: @set percx <char name> <faction #> <value>\n\r");
      return;
    }
    argument = one_argument(argument, parmstr);
    if (sscanf(parmstr, " %f ", &percent) != 1) {
      sendTo("Syntax: @set percx <char name> <faction #> <value>\n\r");
      return;
    }
    if ((parm < MIN_FACTION) || (parm >= MAX_FACTIONS)) {
      sendTo("Syntax: @set percx <char name> <faction #> <value>\n\r");
      return;
    } 
    factionTypeT fnum = factionTypeT(parm);
    percent = min(percent, 100.0);
    percent = max(percent, 0.0);
    mob->setPercX((double) percent, fnum);
#endif
  } else if (is_abbrev(field, "discipline")) {
    if (sscanf(parmstr, " %d ", &parm) != 1) {
      sendTo("Syntax: @set discipline <char name> <discipline> <value>\n\r");
      return;
    }
    argument = one_argument(argument, parmstr);
    if (sscanf(parmstr, " %d ", &parm2) != 1) {
      sendTo("Syntax: @set discipline <char name> <discipline> <value>\n\r");
      return;
    }
    discNumT dnt = mapFileToDisc(parm);
    if (dnt == DISC_NONE) {
      sendTo("Please try to set a valid discipline!\n\r");
      return;
    }
    if (!mob->getDiscipline(dnt)) {
      sendTo(COLOR_MOBS, fmt("Sorry, but %s doesn't have that discipline valid.\n\r") % mob->getName());
      return;
    }

    initial = mob->getDiscipline(dnt)->getNatLearnedness();
    initial = max(0, initial);

    if (!mob->desc && !mob->isPc()) {
      if (mob->getDiscipline(dnt)) 
        mob->getDiscipline(dnt)->setLearnedness(parm2);
    } else {
      mob->getDiscipline(dnt)->setNatLearnedness(parm2);
      mob->getDiscipline(dnt)->setLearnedness(parm2);
      mob->initiateSkillsLearning(dnt, initial, parm2); 
    }
    sendTo(COLOR_MOBS, fmt("You set %s's %s discipline to %d.  (was %d%c)\n\r") %
           mob->getName() % discNames[dnt].properName % parm2 % initial % '%');
    mob->affectTotal();
    mob->doSave(SILENT_YES);
    return;
  } else if (is_abbrev(field, "limb")) {
    if (sscanf(parmstr, "%d", &parm) != 1) {
      sendTo("Syntax: @set limb <char name> <limb#> <value>\n\r");
      return;
    }
    argument = one_argument(argument, parmstr);
    if (sscanf(parmstr, "%d", &parm2) != 1) {
      sendTo("Syntax: @set limb <char name> <limb#> <value>\n\r");
      return;
    }
    if ((parm < MIN_WEAR) || (parm >= MAX_WEAR))
      return;
    mob->setCurLimbHealth(wearSlotT(parm), parm2);
    sendTo(COLOR_MOBS, fmt("%s now has a current limb health of %d on %s %s.\n\r") %
          mob->getName() % mob->getCurLimbHealth(wearSlotT(parm)) % mob->hshr() % mob->describeBodySlot(wearSlotT(parm)));
  } else if (is_abbrev(field, "skill")) { 
#if 1
    if (!strcmp(parmstr, "maximum")) {
      if (!argument) {
        sendTo("Syntax is set skill name maximum number");
        sendTo("maximum must be spelled exactly, number is percentage 60 means 60 percent of max");
        return;
      }
      while (sscanf(argument, "%d", &parm2) != 1) {
       sendTo("Syntax is set skill name maximum number");
       sendTo("maximum must be spelled exactly, number is percentage 60 means 60 percent of max");
       return;
      }
      if ((parm2 > 100) || (parm2 < 1)) {
        sendTo("You can not set it higher than 100 percent or lower than 1");
        return;
      }
      spellNumT snt;
      for (snt = MIN_SPELL; snt < MAX_SKILL;snt++) {
        if (hideThisSpell(snt))
          continue;
        if (!mob->doesKnowSkill(snt)) {
          continue;
        }
        amt = mob->getMaxSkillValue(snt);
        amt *= 100;
        amt *= parm2; 
        amt /= 10000;
        if (discArray[snt]->startLearnDo > amt) {
          amt = min(discArray[snt]->startLearnDo, mob->getMaxSkillValue(snt));
        }
        mob->setSkillValue(snt, max(amt, 1));
        mob->setNatSkillValue(snt, amt);
      }
      mob->affectTotal();
      sendTo(COLOR_MOBS, fmt("Setting %s's skill set to %d percent of his max in each skill.\n\r") %mob->getName() % parm2);
      return;
    }
#endif

//    vlogf(LOG_MISC, fmt("parmstr is %s, argument is %s") %  parmstr % argument);
    while (sscanf(argument, "%d", &parm2) != 1) {
      argument=one_argument(argument, buf2);
      buf=buf2;
      sprintf(buf2," %s",buf.c_str());
      strcat(parmstr,buf2);
 //     vlogf(LOG_MISC,fmt("parmstr is %s, argument is %s, buf is %s, parm2 is %s, buf2 is %s") %  parmstr % argument % buf % parm2 % buf2);
      if (!argument || !strcmp(argument,"")) {
        sendTo("Syntax: @set skill <char name> <skill> <value>\n\r");
        return;
      }     
    }
    spellNumT snt;
    if ((i = convertTo<int>(parmstr))) {
      snt = spellNumT(i);
    } else {
      foundNum = FALSE;
      for (snt = MIN_SPELL; snt < MAX_SKILL;snt++) {
        if (hideThisSpell(snt))
          continue;
        strcpy(buf2,discArray[snt]->name);
        if (is_exact_name(parmstr,buf2)) {
          foundNum = TRUE;
          break;
        }
      }
      if (!foundNum) {
        for (snt = MIN_SPELL; snt < MAX_SKILL;snt++) {
          if (hideThisSpell(snt))
            continue;
          strcpy(buf2,discArray[snt]->name);
// These kludges may not be necessary now, just use a .
           // kludge since chivalry < chi  in discarray
          if (!strcmp(parmstr, "chi") && strcmp(buf2, "chi"))
            continue;
           // kludge since stealth < steal in discarray
          if (!strcmp(parmstr, "steal") && strcmp(buf2, "steal"))
            continue;
           // kludge since paralyze limb < paralyze in discarray
          if (!strcmp(parmstr, "paralyze") && strcmp(buf2, "paralyze"))
            continue;
          if (isname(parmstr,buf2))
            break;
        }
      }
    }
    if ((snt < MIN_SPELL) || (snt >= MAX_SKILL)) {
      sendTo("Invalid skill.\n\r");
      return;
    }

    if (!(mob->getSkill(snt))) {
       if (discArray[snt])
         sendTo(COLOR_MOBS, fmt("%s doesnt appear to have that skill (%s).\n\r") % mob->getName() % (discArray[snt]->name ? discArray[snt]->name : "unknown"));
       else
         sendTo(COLOR_MOBS, fmt("%s doesnt appear to have that skill.\n\r") %
mob->getName());
       return;
    }

    mob->setSkillValue(snt, parm2);
    mob->setNatSkillValue(snt, parm2);
    sendTo(COLOR_MOBS, fmt("Setting %s's skill: %s to %d.\n\r") %mob->getName() %
          discArray[snt]->name % mob->getSkillValue(snt));
    return;
  } else if (is_abbrev(field, "practices")) {
    if (sscanf(parmstr, "%d", &parm) != 1) {
      sendTo("Syntax: @set practices <char name> <number> <class number>\n\r");
      return;
    }
    argument = one_argument(argument, parmstr);
    if (sscanf(parmstr, "%d", &parm2) != 1) {
      sendTo("Syntax: @set practices <char name> <number> <class number>\n\r");
      return;
    }
    if (dynamic_cast<TMonster *>(mob)) {
      sendTo("Mobiles have no use for practices!\n\r");
      return;
    }
    if (parm < 0) {
      sendTo("No negative numbers please!\n\r");
      return;
    }
    if ((parm > 100) && !hasWizPower(POWER_SET_IMP_POWER)) {
      sendTo("Over 100 practices?!?!?! Woah!\n\r");
      vlogf(LOG_MISC, fmt("%s just tried to set %s's practices to a number > 100!") %  getName() % mob->getName());
      return;
    }

    if(parm2 >= MAX_CLASSES){
      sendTo("Invalid class number.\n\r");
      return;
    } else {
      mob->practices.prac[parm2]=parm;
    }


    sendTo(COLOR_MOBS, fmt("Setting %s's %s practices to %d.\n\r") % mob->getName() % classInfo[parm2].name % parm);
  } else if (is_abbrev(field, "level")) {
    if (sscanf(parmstr, "%d", &parm) != 1) {
      sendTo("Syntax: @set level <char name> <level> <class number>\n\r");
      return;
    }
    argument = one_argument(argument, parmstr);
    if (sscanf(parmstr, "%d", &parm2) != 1) {
      sendTo("Syntax: @set level <char name> <level> <class number>\n\r");
      return;
    }

    if (parm2 < 0 || parm2 >= MAX_CLASSES) {
      sendTo("Syntax: @set level <char name> <level> <class number>\n\r");
      sendTo(fmt("<class number> must be between 0 and %d (you entered %d).\n\r") % (MAX_CLASSES-1) % parm2);
      return;
    }

    if (dynamic_cast<TPerson *>(mob)) {
      if ((mob->GetMaxLevel() >= GetMaxLevel()) && (this != mob)) {
        mob->sendTo(COLOR_MOBS, fmt("%s just tried to change your level.\n\r") % getName());
        return;
      }
    } else {
      if (parm2 < MIN_CLASS_IND || parm2 >= MAX_CLASSES) {
        sendTo("Syntax: @set level <char name> <level> <class number>\n\r");
        sendTo(fmt("<class number> must be between 0 and %d.\n\r") % (MAX_CLASSES-1));
        return;
      }
      mob->setLevel(classIndT(parm2), parm);
      mob->calcMaxLevel();
      sendTo(COLOR_MOBS, fmt("Setting mob %s to level %d in class %s.\n\r") % mob->getName() % parm % classInfo[parm2].name);
      return;
    }
    if (parm < 0) {
      sendTo("Sorry, only positive numbers allowed.\n\r");
      return;
    }
    if (parm < GetMaxLevel() ||
        (parm <= GetMaxLevel() && this == mob)) {
      if (hasWizPower(POWER_SET_IMP_POWER)) {
        if (parm2 >= MIN_CLASS_IND && parm2 < MAX_CLASSES) {
          mob->setLevel(classIndT(parm2), parm);
          mob->calcMaxLevel();
          sendTo(COLOR_MOBS, fmt("Setting char %s to level %d in class %s.\n\r") % mob->getName() % parm % classInfo[parm2].name);

          
          mob->setExp(1+getExpClassLevel(classIndT(parm2), parm));
          mob->setMaxExp(1+getExpClassLevel(classIndT(parm2), parm));
          sendTo(COLOR_MOBS, fmt("Setting experience to %.1f.\n\r") %
              mob->getExp());
          int oldPracs = mob->practices.prac[parm2];
          // set to zero then recover as necessary
          mob->practices.prac[parm2] = 0;
          int expectedPracs = mob->expectedPracs();
          int actualPracs = 0;
          for (int j=0;j<10;j++)
            actualPracs += mob->pracsSoFar();
          actualPracs /= 10; // taking a mean to get a better number
          int extraPracs = max(0,expectedPracs-actualPracs);
          mob->practices.prac[parm2] = (sh_int) extraPracs;
          sendTo(COLOR_MOBS, fmt("Setting char %s's extra practices to %d (was %d) to go along with level in class %s.\n\r") 
              % mob->getName() % mob->practices.prac[parm2] % 
              oldPracs % classInfo[parm2].name);
          actualPracs = 0;
          for (int j=0;j<10;j++)
            actualPracs += mob->pracsSoFar();
          actualPracs /= 10; // taking a mean to get a better number
          if (expectedPracs-actualPracs < 0)
            sendTo(COLOR_MOBS, fmt("Char %s has %d pracs more than expected for level %d.\n\r") %
                mob->getName() % (actualPracs-expectedPracs) % parm);

        }
      } else {
        if (parm > MAX_MORT) {
          sendTo(fmt("Sorry, you can't advance past %dth level.\n\r") % MAX_MORT);
          return;
        }
        if (parm2 >= MIN_CLASS_IND && parm2 < MAX_CLASSES) {
          mob->setLevel(classIndT(parm2), parm);
          mob->calcMaxLevel();
          sendTo(COLOR_MOBS, fmt("Setting char %s to level %d in class %s.\n\r") % mob->getName() % parm % classInfo[parm2].name);

          mob->setExp(1+getExpClassLevel(classIndT(parm2), parm));
          mob->setMaxExp(1+getExpClassLevel(classIndT(parm2), parm));
          sendTo(COLOR_MOBS, fmt("Setting experience to %.1f.\n\r") %
              mob->getExp());
          int oldPracs = mob->practices.prac[parm2];
          // set to zero then recover as necessary
          mob->practices.prac[parm2] = 0;
          int expectedPracs = mob->expectedPracs();
          int actualPracs = 0;
          for (int j=0;j<10;j++)
            actualPracs += mob->pracsSoFar();
          actualPracs /= 10; // taking a mean to get a better number
          int extraPracs = max(0,expectedPracs-actualPracs);
          mob->practices.prac[parm2] = (sh_int) extraPracs;
          sendTo(COLOR_MOBS, fmt("Setting char %s's extra practices to %d (was %d) to go along with level in class %s.\n\r") 
              % mob->getName() % mob->practices.prac[parm2] % 
              oldPracs % classInfo[parm2].name);
          actualPracs = 0;
          for (int j=0;j<10;j++)
            actualPracs += mob->pracsSoFar();
          actualPracs /= 10; // taking a mean to get a better number
          if (expectedPracs-actualPracs < 0)
            sendTo(COLOR_MOBS, fmt("Char %s has %d pracs more than expected for level %d.\n\r") %
                mob->getName() % (actualPracs-expectedPracs) % parm);


        }
      }
    } else {
      sendTo("You can only set level less than your own.\n\r");
      return;
    }
    if (!mob->isImmortal() && (parm > MAX_MORT) && mob->isPc()) {
      sendTo("Making them immortal and adjusting hunger/thirsts.\n\r");
      mob->doToggle("immortal");
      mob->setCond(FULL, -1);
      mob->setCond(THIRST, -1);
    }
  } else if (is_abbrev(field, "sex")) {
    if (is_abbrev(parmstr, "male")) {
      mob->setSex(SEX_MALE);
      sendTo(COLOR_MOBS, fmt("%s is now male.\n\r") % mob->getName());
    } else if (is_abbrev(parmstr, "female")) {
      mob->setSex(SEX_FEMALE);
      sendTo(COLOR_MOBS, fmt("%s is now female.\n\r") % mob->getName());
    } else if (is_abbrev(parmstr, "neuter")) {
      mob->setSex(SEX_NEUTER);
      sendTo(COLOR_MOBS, fmt("%s is now neuter.\n\r") % mob->getName());
    } else {
      sendTo("Syntax: @set sex <char name> <male || female || neuter>\n\r");
      return;
    }
  } else if (is_abbrev(field, "donebasic")) {
    if (!mob->desc) {
      sendTo("You can not set doneBasic on a mob or linkdead.");
      return;
    }
    if (sscanf(parmstr, "%d", &parm) != 1) {
      sendTo("Syntax: @set donebasic <char name> <level> <class number>\n\r");
      return;
    }
    argument = one_argument(argument, parmstr);
    if (sscanf(parmstr, "%d", &parm2) != 1) {
      sendTo("Syntax: @set donebasic <char name> <level> <class number>\n\r");
      return;
    }

    if (parm2 < 0 || parm2 >= MAX_CLASSES) {
      sendTo("Syntax: @set donebasic <char name> <level> <class number>\n\r");
      sendTo(fmt("<class number> must be between 0 and %d.\n\r") % (MAX_CLASSES-1));
      return;
    }
    if (parm < 0) {
      sendTo("Sorry, only positive numbers allowed.\n\r");
      return;
    }
    mob->player.doneBasic[parm2] = parm;
    sendTo(COLOR_MOBS, fmt("You just set %s's %s class doneBasic to %d.") % mob->getName() % classInfo[parm2].name % parm);
    return;
  } else if (is_abbrev(field, "race")) {
    if (is_number(parmstr)) {
      sscanf(parmstr, "%d", &parm);
      if(parm <= RACE_NORACE || parm >= MAX_RACIAL_TYPES) {
	sendTo("Invalid race specified.  Check 'show race'\n\r"); 
	return;
      }
      mob->setRace(race_t(parm));
      sendTo(COLOR_MOBS, fmt("%s is now of the %s race.\n\r") % mob->getName() % mob->getMyRace()->getSingularName());

      // log this because changing race *may* cause some equipment problems
      // due to wearability, etc.
      vlogf(LOG_MISC, fmt("%s being changed to the %s race by %s") %  mob->getName() % mob->getMyRace()->getSingularName() % getName());

      // oh yeah, may as well avoid equipment problems too  :)
      wearSlotT ij;
      for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
        if (mob->equipment[ij]) {
          *mob += *mob->unequip(ij);
        }
      }

      mob->sendTo(fmt("You are now of the %s race.\n\r") %
              mob->getMyRace()->getSingularName());
      mob->sendTo("Your equipment has been placed into your inventory.\n\r");

    } else {
      sendTo("argument must be a number\n\r");
      return;
    }
  } else if (is_abbrev(field, "hunger")) {
    sscanf(parmstr, "%d", &parm);
    mob->setCond(FULL, parm);
    sendTo(COLOR_MOBS, fmt("You set %s's hunger to %d.\n\r") % mob->getName() % mob->getCond(FULL));
  } else if (is_abbrev(field, "height")) {
    sscanf(parmstr, "%d", &parm);
    mob->setHeight(parm);
    sendTo(COLOR_MOBS, fmt("You set %s's height to %d inches.\n\r") % 
           mob->getName() % mob->getHeight());
  } else if (is_abbrev(field, "weight")) {
    sscanf(parmstr, "%f", &percent);
    mob->setWeight(percent);
    sendTo(COLOR_MOBS, fmt("You set %s's weight to %.3f pounds.\n\r") % 
           mob->getName() % mob->getWeight());
  } else if (is_abbrev(field, "thirst")) {
    sscanf(parmstr, "%d", &parm);
    mob->setCond(THIRST, parm);
    sendTo(COLOR_MOBS, fmt("You set %s's thirst to %d.\n\r") % mob->getName() % mob->getCond(THIRST));
  } else if (is_abbrev(field, "drunk")) {
    sscanf(parmstr, "%d", &parm);
    mob->setCond(DRUNK, parm);
    sendTo(COLOR_MOBS, fmt("You set %s's drunkenness to %d.\n\r") % mob->getName() % mob->getCond(DRUNK));
  } else if (is_abbrev(field, "hit")) {
    sscanf(parmstr, "%d", &parm);
    mob->setHit(parm);
    sendTo(COLOR_MOBS, fmt("You set %s's hp to %d.\n\r") % mob->getName() % mob->getHit());
    mob->updatePos();
  } else if (is_abbrev(field, "mhit")) {
    sscanf(parmstr, "%d", &parm);
    mob->setMaxHit(parm);
    if (mob->hitLimit() <= 0) {
      sendTo(COLOR_MOBS, "You set a negative value.  Setting 1.\n\r");
      mob->setMaxHit(1);
      mob->setHit(1);
    } else
      sendTo(COLOR_MOBS, fmt("You set %s's max hp to %d.\n\r") % mob->getName() % mob->hitLimit());
  } else if (is_abbrev(field, "tohit")) {
    sscanf(parmstr, "%d", &parm);
    mob->setHitroll(parm);
    sendTo(COLOR_MOBS, fmt("You set %s's +tohit to %d.\n\r") % mob->getName() % mob->getHitroll());
  } else if (is_abbrev(field, "todam")) {
    sscanf(parmstr, "%d", &parm);
    mob->setDamroll(parm);
    sendTo(COLOR_MOBS, fmt("You set %s's +todam to %d.\n\r") % mob->getName() % mob->getDamroll());
  } else if (is_abbrev(field, "ac")) {
    //    sscanf(parmstr, "%d", &parm);
    //    mob->setArmor(parm);
    //    sendTo(COLOR_MOBS, fmt("You set %s's armor to %d.\n\r") % mob->getName() % mob->getArmor());
    sendTo(COLOR_MOBS, "Armor is dynamic now, you can't set it.\n\r");
  } else if (is_abbrev(field, "bank")) {
    sscanf(parmstr, "%d", &parm);
    mob->setBank(parm);
    sendTo(COLOR_MOBS, fmt("You set %s's bank to %d.\n\r") % mob->getName() % mob->getBank());
  } else if (is_abbrev(field, "gold") || is_abbrev(field, "talens")) {
    sscanf(parmstr, "%d", &parm);
    mob->setMoney(parm);
    sendTo(COLOR_MOBS, fmt("You set %s's money to %d.\n\r") % mob->getName() % mob->getMoney());
  } else if (is_abbrev(field, "age")) {
    sscanf(parmstr, "%d", &parm);
    mob->age_mod += parm;
    sendTo(COLOR_MOBS, fmt("You add %d years to %s's age.\n\r") % parm % mob->getName());
  } else if (is_abbrev(field, "strength")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_STR, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_STR, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's strength (chosen: %d) (cur: %d).\n\r") % 
            mob->getName() % 
            mob->getStat(STAT_CHOSEN, STAT_STR) %
            mob->getStat(STAT_CURRENT, STAT_STR));
  } else if (is_abbrev(field, "brawn")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_BRA, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_BRA, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's brawn (chosen: %d) (cur: %d).\n\r") % 
            mob->getName() % 
            mob->getStat(STAT_CHOSEN, STAT_BRA) %
            mob->getStat(STAT_CURRENT, STAT_BRA));
  } else if (is_abbrev(field, "dexterity")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_DEX, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_DEX, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's dexterity (chosen: %d) (cur: %d).\n\r") % 
            mob->getName() % 
            mob->getStat(STAT_CHOSEN, STAT_DEX)%
            mob->getStat(STAT_CURRENT, STAT_DEX));
  } else if (is_abbrev(field, "agility")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_AGI, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_AGI, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's agility (chosen: %d) (cur: %d).\n\r") % 
            mob->getName() % 
            mob->getStat(STAT_CHOSEN, STAT_AGI) %
            mob->getStat(STAT_CURRENT, STAT_AGI));
  } else if (is_abbrev(field, "constitution")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_CON, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_CON, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's constitution (chosen: %d) (cur: %d).\n\r") % 
            mob->getName() % 
            mob->getStat(STAT_CHOSEN, STAT_CON) %
            mob->getStat(STAT_CURRENT, STAT_CON));
  } else if (is_abbrev(field, "intelligence")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_INT, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_INT, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's intellect (chosen: %d) (cur: %d).\n\r") % 
            mob->getName() %
            mob->getStat(STAT_CHOSEN, STAT_INT) %
            mob->getStat(STAT_CURRENT, STAT_INT));
  } else if (is_abbrev(field, "wisdom")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_WIS, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_WIS, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's wisdom (chosen: %d) (cur: %d).\n\r") % 
            mob->getName() % 
            mob->getStat(STAT_CHOSEN, STAT_WIS) %
            mob->getStat(STAT_CURRENT, STAT_WIS));
  } else if (is_abbrev(field, "focus")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_FOC, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_FOC, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's focus (chosen: %d) (cur: %d).\n\r") %             mob->getName() % 
            mob->getStat(STAT_CHOSEN, STAT_FOC) %
            mob->getStat(STAT_CURRENT, STAT_FOC));
  } else if (is_abbrev(field, "karma")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_KAR, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_KAR, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's karma (chosen: %d) (cur: %d).\n\r") %             mob->getName() % 
            mob->getStat(STAT_CHOSEN, STAT_KAR) %
            mob->getStat(STAT_CURRENT, STAT_KAR));
  } else if (is_abbrev(field, "charisma")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_CHA, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_CHA, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's charisma (chosen: %d) (cur: %d).\n\r") %             mob->getName() % 
            mob->getStat(STAT_CHOSEN, STAT_CHA) %
            mob->getStat(STAT_CURRENT, STAT_CHA));
  } else if (is_abbrev(field, "speed")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_SPE, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_SPE, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's speed (chosen: %d) (cur: %d).\n\r") %             mob->getName() % 
            mob->getStat(STAT_CHOSEN, STAT_SPE) %
            mob->getStat(STAT_CURRENT, STAT_SPE));
  } else if (is_abbrev(field, "perception")) {
    sscanf(parmstr, "%d", &parm);
    if (!*argument) {
      mob->setStat(STAT_CURRENT, STAT_PER, parm);
    } else {
      mob->setStat(STAT_CHOSEN, STAT_PER, parm);
    }
    sendTo(COLOR_MOBS, fmt("You set %s's perception (chosen: %d) (cur: %d).\n\r") %             mob->getName() % 
            mob->getStat(STAT_CHOSEN, STAT_PER) %
            mob->getStat(STAT_CURRENT, STAT_PER));
  } else if (is_abbrev(field, "piety")) {
    sscanf(parmstr, "%f", &percent);
    mob->setPiety(percent);
    sendTo(COLOR_MOBS, fmt("You set %s's piety to %.3f.\n\r") % mob->getName() % mob->getPiety());
  } else if (is_abbrev(field, "mmana")) {
    sscanf(parmstr, "%d", &parm);
    mob->setMaxMana(parm);
    sendTo(COLOR_MOBS, fmt("You set %s's max mana to %d.\n\r") % mob->getName() % mob->manaLimit());
  } else if (is_abbrev(field, "mana")) {
    sscanf(parmstr, "%d", &parm);
    mob->setMana(parm);
    sendTo(COLOR_MOBS, fmt("You set %s's mana to %d.\n\r") % mob->getName() % mob->getMana());
  } else if (is_abbrev(field, "lifeforce")) {
    sscanf(parmstr, "%d", &parm);
    mob->setLifeforce(parm);
    sendTo(COLOR_MOBS, fmt("You set %s's lifeforce to %d.\n\r") % mob->getName() % mob->getLifeforce());
  } else if (is_abbrev(field, "mmove")) {
    sscanf(parmstr, "%d", &parm);
    mob->setMaxMove(parm);
    sendTo(COLOR_MOBS, fmt("You set %s's max move to %d.\n\r") % mob->getName() % mob->moveLimit());
  } else if (is_abbrev(field, "move")) {
    sscanf(parmstr, "%d", &parm);
    mob->setMove(parm);
    sendTo(COLOR_MOBS, fmt("You set %s's move to %d.\n\r") % mob->getName() % mob->getMove());
  } else if (is_abbrev(field, "stuck")) {
    if (sscanf(parmstr, "%s", obj_name) != 1) {
      sendTo("Syntax : @set stuck <playername> <object> <body part>\n\r");
      return;
    }
    argument = one_argument(argument, parmstr);
    if (sscanf(parmstr, "%d", &parm) != 1) {
      sendTo("Syntax : @set stuck <playername> <object> <body part>\n\r");
      return;
    }
    TThing *t_obj = searchLinkedListVis(this, obj_name, getStuff());
    obj = dynamic_cast<TObj *>(t_obj);
    if (!obj) {
      sendTo("Syntax : @set stuck <playername> <object> <body part>\n\r");
      sendTo("The object must be in your inventory.\n\r");
      return;
    }
    if (parm >= MAX_WEAR ||
        parm < MIN_WEAR ||
        parm == HOLD_RIGHT ||
        parm == HOLD_LEFT) {
      sendTo("Syntax : @set stuck <playername> <object> <body part>\n\r");
      sendTo(fmt("Body part must be in range %d to %d.\n\r") % MIN_WEAR % (MAX_WEAR - 1));
      return;
    }
    if (!mob->slotChance(wearSlotT(parm))) {
      sendTo("That victim does not have that slot.\n\r");
      return;
    }

    if(mob->getStuckIn(wearSlotT(parm))){
      sendTo("That slot is already occupied.\n\r");
      return;
    }

    buf=fmt("You stick $p into $N's %s.") % mob->describeBodySlot(wearSlotT(parm));
    act(buf, FALSE, this, obj, mob, TO_CHAR);
    --(*obj);
    mob->stickIn(obj, wearSlotT(parm));
    return;
  } else if (is_abbrev(field, "pet")) {
    // set it charmed, and force it to follow
    if (mob->isPc()) {
      sendTo("No, that would be a bad idea.\n\r");
      return;
    }
    mob->setExp(0);
    SET_BIT(mob->specials.affectedBy, AFF_CHARM);
    act("$N is now charmed, force it to follow someone...", false, this, 0, mob, TO_CHAR);
    return;
  } else if (is_abbrev(field, "gold_modifier")) {
    // this is undocumented, as it is an economy thing for batopr
    if (sscanf(parmstr, "%d", &parm) != 1) {
      sendTo("Syntax: @set gold_modifier <char name> <modifier> <new_value>.\n\r");
      return;
    }
    if (parm < 0 || parm >= MAX_MONEY_TYPE) {
      sendTo(fmt("Invalid value (%d) for modifier.\n\r") % parm);
      return;
    }
    argument = one_argument(argument, parmstr);
    if (sscanf(parmstr, " %f ", &percent) != 1) {
      sendTo("Syntax: @set gold_modifier <char name> <modifier> <new_value>.\n\r");
      return;
    }
    
    gold_modifier[parm] = percent;
    sendTo("OK, gold_modifier set.\n\r");

    return;
  } else if (is_abbrev(field, "bodyflags")) {
    if (sscanf(parmstr, "%d", &parm) != 1) {
      sendTo("Syntax: @set bodyflags <char name> <part #> <flags>.\n\r");
      return;
    }
    argument = one_argument(argument, parmstr);
    if (sscanf(parmstr, "%d", &parm2) != 1) {
      sendTo("Syntax: @set bodyflags <char name> <part #> <flags>.\n\r");
      return;
    }
    if (parm < MIN_WEAR || parm >= MAX_WEAR) {
      sendTo("Syntax: @set bodyflags <char name> <part #> <flags>.\n\r");
      sendTo(fmt("Please use numbers between %d and %d for <part #>.\n\r") % MIN_WEAR % (MAX_WEAR - 1));
      return;
    }
    wearSlotT slot = wearSlotT(parm);
    if (!mob->slotChance(slot)) {
      act("$N has not got that part.", FALSE, this, 0, mob, TO_CHAR);
      return;
    }
    mob->setLimbFlags(slot, parm2);
    sendTo(COLOR_MOBS, fmt("You just set the following flags on %s's %s: %s\n\r") % mob->getName() % mob->describeBodySlot(slot) % sprintbit(parm2, body_flags));
  } else if (is_abbrev(field, "office")) {
    if (sscanf(parmstr, "%d", &parm) != 1) {
      sendTo("Syntax: @set office <char name> <office number>\n\r");
      return;
    }

    if ((!mob->desc || mob->GetMaxLevel() <= MAX_MORT ||
         mob->hasWizPower(POWER_WIZARD) ||
         !hasWizPower(POWER_SET_IMP_POWER)) && mob != this) {
      sendTo("You can not do this!\n\r");
      return;
    }

    mob->desc->office = parm;
    sendTo(fmt("You set %s's office to %d.") % mob->getName() % parm);
  } else if (is_abbrev(field, "blocka")) {
    parm = parm2 = 0;

    if (sscanf(parmstr, "%d", &parm) < 1) {
      sendTo("Syntax: @set blocka <char name> <start-room:0 to remove> <end-room>\n\r");
      return;
    }

    argument = one_argument(argument, parmstr);

    if (sscanf(parmstr, "%d", &parm2) < 1) {
      sendTo("Syntax: @set blocka <char name> <start-room:0 to remove> <end-room>\n\r");
      return;
    }

    if ((!mob->desc || mob->GetMaxLevel() <= MAX_MORT ||
         mob->hasWizPower(POWER_WIZARD) ||
         !hasWizPower(POWER_SET_IMP_POWER)) && mob != this) {
      sendTo("You can not do this!\n\r");
      return;
    }

    mob->desc->blockastart = parm;
    mob->desc->blockaend   = parm2;
    sendTo(fmt("You set %s's blocka to %d-%d.") % mob->getName() % parm % parm2);
  } else if (is_abbrev(field, "blockb")) {
    parm = parm2 = 0;

    if (sscanf(parmstr, "%d", &parm) < 1) {
      sendTo("Syntax: @set blockb <char name> <start-room:0 to remove> <end-room>\n\r");
      return;
    }

    argument = one_argument(argument, parmstr);

    if (sscanf(parmstr, "%d", &parm2) < 1) {
      sendTo("Syntax: @set blockb <char name> <start-room:0 to remove> <end-room>\n\r");
      return;
    }

    if ((!mob->desc || mob->GetMaxLevel() <= MAX_MORT ||
         mob->hasWizPower(POWER_WIZARD) ||
         !hasWizPower(POWER_SET_IMP_POWER)) && mob != this) {
      sendTo("You can not do this!\n\r");
      return;
    }

    mob->desc->blockbstart = parm;
    mob->desc->blockbend   = parm2;
    sendTo(fmt("You set %s's blockb to %d-%d.") % mob->getName() % parm % parm2);
  } else {
    sendTo("Wrong option.\n\r");
    return;
  }
}
