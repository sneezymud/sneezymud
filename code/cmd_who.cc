//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "cmd_who.cc" - the who command
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "statistics.h"
#include "games.h"

void TBeing::parseTitle(char *arg, Descriptor *)
{
  strcpy(arg, getName());
  return;
}

void TPerson::parseTitle(char *arg, Descriptor *user)
{
  int flag = FALSE;
  if (!title) {
    strcpy(arg, getName());
    return;
  }

  strcpy(arg, nameColorString(this, user, title, &flag, COLOR_BASIC, FALSE).c_str());
  if (!flag &&
      colorString(this, user, title, NULL, COLOR_NONE, TRUE).find(getNameNOC(this).c_str()) ==
      string::npos)
    strcpy(arg, getName());  // did not specify a <n>

  // explicitely terminate it since players are sloppy
  strcat(arg, "<1>");

  return;
}

void Descriptor::menuWho() 
{
  TBeing *person;
  char buf[256];
  char buf2[256];
  char send[4096] = "\0";

  strcpy(send, "\n\r");

  for (person = character_list; person; person = person->next) {
    if (person->isPc() && person->polyed == POLY_TYPE_NONE) {
      if (dynamic_cast<TPerson *>(person) &&
          (person->getInvisLevel() < GOD_LEVEL1)) {
        person->parseTitle(buf, this);

        sprintf(buf2, "%s", colorString(person, this, buf, NULL, COLOR_BASIC, FALSE).c_str());
        strcat(buf2, "\n\r");
        strcat(send, buf2);
      }
    }
  }
  strcat(send, "\n\r");
  writeToQ(send);
  writeToQ("[Press return to continue]\n\r");
}

static const string getWizDescriptLev(const TBeing *ch)
{
  if (ch->hasWizPower(POWER_WIZARD))
    return "creator";
  else if (ch->hasWizPower(POWER_GOD))
    return "  god  ";
  else if (ch->hasWizPower(POWER_BUILDER))
    return "demigod";
  else
    return "BUG ME!";
}

static const string getWhoLevel(const TBeing *ch, TBeing *p)
{
  char tempbuf[256];
  char colorBuf[256] = "\0";

  if (p->hasWizPower(POWER_WIZARD))
    strcpy(colorBuf, ch->purple());
  else if (p->hasWizPower(POWER_GOD))
    strcpy(colorBuf, ch->red());
  else if (p->hasWizPower(POWER_BUILDER))
    strcpy(colorBuf, ch->cyan());

  // Do it this way so you get the default-titles also.
  if (p && p->GetMaxLevel() > MAX_MORT) {
    string str = p->msgVariables(MSG_IMM_TITLE);
    unsigned int len = str.size();
    unsigned int padding = 14-len;
    unsigned int frontpadding = padding/2;
    for (unsigned int iter = 0; iter < frontpadding; iter++)
      str.insert(0, " ");
    
    sprintf(tempbuf, "%sLevel:[%-14s%s][%s] %s",
            colorBuf, str.c_str(),
            colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  } else {
    string tmpstring;

    if(p->isPlayerAction(PLR_ANONYMOUS) && !ch->isImmortal()){
      tmpstring = "Anonymous";
    } else {
      sprintf(tempbuf, "%-5s Lev %2d", p->getProfAbbrevName(), p->GetMaxLevel());
      tmpstring += tempbuf;
    }

    while (tmpstring.length() < 13)
      tmpstring = " " + tmpstring + " ";
    if (tmpstring.length() < 14)
      tmpstring += " ";

    sprintf(tempbuf, "Level:[%s] ", tmpstring.c_str());
    TFaction *f = NULL;
    if((f = p->newfaction()) && TestCode5) {
      if (f->ID && (IS_SET(f->flags, FACT_ACTIVE) || ch->newfaction() == p->newfaction() || ch->isImmortal()) &&
	  (!IS_SET(f->flags, FACT_HIDDEN) || ch->newfaction() == p->newfaction() || ch->isImmortal()) &&
	  (!p->isImmortal() || ch->isImmortal())) {
	sprintf(tempbuf, "%s %s[<1>%s%s]<1>", tempbuf,
		heraldcodes[p->newfaction()->colors[0]],
		p->newfaction()->getName(),
		heraldcodes[p->newfaction()->colors[0]]);
      }
    }
       
  }

  return tempbuf;
}

void TBeing::doWho(const char *argument)
{
  // New Who Code to handle: who -ol j 20 40

#if 0
  if (gamePort == BETA_GAMEPORT) {
    // 28 Who Flags upon ( 4-14-00):
    //  i=Idle          l=Levels        q=Quests
    //  h=Hit/Mana/Move z=Seeks-Group   p=Grouped
    //  d=Linkdead      g=Gods/Creators b=Builders/Gods/Creators
    //  o=Mortals       s=Stats         f=Faction
    //  1=Mage          2=Cleric        3=Warrior
    //  4=Thief         5=Deikhan       6=Monk
    //  7=Ranger        8=Shaman        e=Elf
    //  t=Hobbit        n=Gnome         u=Human
    //  r=Ogre          w=Dwarven       y=Not-Grouped
    //  a=Account Name
    const  char *tPerfMatches = "ilqhzpydgbosf12345678etnurwa";
    char   tString[256],
           tBuffer[256]  = "\0",
           tOutput[1024] = "\0";
    int    tLow   =  0,
           tHigh  = 60,
           tCount =  0,
           tLinkD =  0;
    string tSb("");
    unsigned long int tBits = 0;
    TBeing *tBeing;
      //           *tFollower;

    while ((argument = one_argument(argument, tString))) {
      if (tString[0] == '-') {
        for (const char *tMarker = (tString + 1); *tMarker; tMarker++)
          if (strchr(tPerfMatches, *tMarker))
            tBits |= (1 << (strchr(tPerfMatches, *tMarker) - tPerfMatches));
          else {
            if (*tMarker != '?') {
              sprintf(tBuffer, "Unknown Option '%c':\n\r", *tMarker);
              tSb += tBuffer;
            }

            if (isImmortal()) {
              tSb += "[-] [i]idle [l]levels [q]quests [h]hit/mana/move/lf\n\r";
              tSb += "[-] [z]seeks-group [p]groups [y]currently-not-grouped\n\r";
              tSb += "[-] [d]linkdead [g]God [b]Builders [o]Mort [s]stats [f]action\n\r";
              tSb += "[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Deikhan[6]Monk[7]Ranger[8]Shaman\n\r";
              tSb += "[-] [e]elf [t]hobbit [n]gnome [u]human [r]ogre [w]dwarven\n\r";

              if (hasWizPower(POWER_WIZARD))
                tSb += "[-] [a]ccount\n\r";

              tSb += "\n\r";
            } else {
              tSb += "[-] [q]quests [g]god [b]builder [o]mort [f]faction\n\r";
              tSb += "[-] [z]seeks-group [p]groups [y]currently-not-grouped\n\r";
              tSb += "[-] [e]elf [t]hobbit [n]gnome [u]human [r]ogre [w]dwarven\n\r\n\r";
              tSb += "[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Deikhan[6]Monk[7]Ranger[8]Shaman\n\r";
            }

            if (desc)
              desc->page_string(tSb.c_str(), SHOWNOW_NO, ALLOWREP_YES);

            return;
          }
      } else if (is_number(tBuffer)) {
        if (!tLow)
          tLow = atoi(tBuffer);
        else
          tHigh = atoi(tBuffer);

	if (tLow <= 0 || tHigh > 60) {
          sendTo("Level numbers must be between 1 and 60.\n\r");
          return;
        }

        if (tHigh < tLow) {
          tCount = tLow;
          tLow   = tHigh;
          tHigh  = tCount;
        }
      } else
        strcpy(tBuffer, tString);
    }

    tSb += "Players: (Use who -? for online help)\n\r----------\n\r";
    tCount = tLinkD = 0;

    for (tBeing = character_list; tBeing; tBeing = tBeing->next)
      if (tBeing->isPc() && (tBeing->polyed == POLY_TYPE_NONE || isImmortal()) &&
          dynamic_cast<TPerson *>(tBeing) && canSeeWho(tBeing)) {
        if (tBeing->isLinkdead())
          tLinkD++;
        else
          tCount++;

        bool anonCheck = (isImmortal() || !tBeing->isPlayerAction(PLR_ANONYMOUS));

        if ((!(tBits & (1 <<  2)) || tBeing->inQuest()) &&
            (!(tBits & (1 <<  4)) || tBeing->isPlayerAction(PLR_SEEKSGROUP)) &&
            (!(tBits & (1 <<  5)) || (tBeing->isAffected(AFF_GROUP) && !tBeing->master && tBeing->followers)) &&
            (!(tBits & (1 <<  6)) || (!tBeing->isAffected(AFF_GROUP) && !tBeing->isImmortal())) &&
            (!(tBits & (1 <<  7)) || (tBeing->isLinkdead() && isImmortal())) && 
            (!(tBits & (1 <<  8)) || tBeing->hasWizPower(POWER_GOD)) &&
            (!(tBits & (1 <<  9)) || tBeing->hasWizPower(POWER_BUILDER)) &&
            (!(tBits & (1 << 10)) || !tBeing->hasWizPower(POWER_BUILDER)) &&
            (!(tBits & (1 << 13)) || (anonCheck && tBeing->hasClass(CLASS_MAGE))) &&
            (!(tBits & (1 << 14)) || (anonCheck && tBeing->hasClass(CLASS_CLERIC))) &&
            (!(tBits & (1 << 15)) || (anonCheck && tBeing->hasClass(CLASS_WARRIOR))) &&
            (!(tBits & (1 << 16)) || (anonCheck && tBeing->hasClass(CLASS_THIEF))) &&
            (!(tBits & (1 << 17)) || (anonCheck && tBeing->hasClass(CLASS_DEIKHAN))) &&
            (!(tBits & (1 << 18)) || (anonCheck && tBeing->hasClass(CLASS_MONK))) &&
            (!(tBits & (1 << 19)) || (anonCheck && tBeing->hasClass(CLASS_RANGER))) &&
            (!(tBits & (1 << 20)) || (anonCheck && tBeing->hasClass(CLASS_SHAMAN))) &&
            (!(tBits & (1 << 21)) || tBeing->getRace() == RACE_ELVEN) &&
            (!(tBits & (1 << 22)) || tBeing->getRace() == RACE_GNOME) &&
            (!(tBits & (1 << 23)) || tBeing->getRace() == RACE_HUMAN) &&
            (!(tBits & (1 << 24)) || tBeing->getRace() == RACE_DWARF) &&
            (!(tBits & (1 << 25)) || tBeing->getRace() == RACE_OGRE) &&
            (!(tBits & (1 << 26)) || tBeing->getRace() == RACE_HOBBIT) &&
            (!*tBuffer || is_abbrev(tBuffer, tBeing->getName())) &&
            in_range(tBeing->GetMaxLevel(), tLow, tHigh)) {
          tOutput[0] = '\0';

          if ((tBits & (1 << 0)) && isImmortal()) {
            sprintf(tString, "Idle:[%-3d] ", tBeing->getTimer());
            strcat(tOutput, tString);
          }

          if ((tBits & (1 << 1)))
            strcat(tOutput, getWhoLevel(this, tBeing).c_str());

          if (tBeing->isLinkdead())
            sprintf(tString, "[%-12s]", pers(tBeing));
          else if (tBeing->polyed == POLY_TYPE_SWITCH)
            sprintf(tString, "[%-12s] (switched)", pers(tBeing));
          else if (dynamic_cast<TMonster *>(tBeing) &&
                   (tBeing->specials.act & ACT_POLYSELF))
            sprintf(tString, "(%-12s)", pers(tBeing));
          else
            sprintf(tString, "%-14s", pers(tBeing));

          strcat(tOutput, tString);
	  // Name goes here.

          if ((tBits & (1 << 12))) {
            if ((isImmortal() || getFaction() == tBeing->getFaction()) && !tBeing->isImmortal())
#if FACTIONS_IN_USE
              sprintf(tString, " [%s] %5.2f%%",
                      FactionInfo[tBeing->getFaction()].faction_name,
                      tBeing->getPerc());
#else
              sprintf(tString, " [%s]",
                      FactionInfo[tBeing->getFaction()].faction_name);
#endif

            strcat(tOutput, tString);
          }

          if ((tBits & (1 << 3)) && isImmortal()) {
            if (tBeing->hasClass(CLASS_CLERIC) || tBeing->hasClass(CLASS_DEIKHAN))
              sprintf(tString, "\n\r\tHit:[%-3d] Pty:[%-5.2f] Move:[%-3d] Talens:[%-8d] Bank:[%-8d]",
                      tBeing->getHit(), tBeing->getPiety(), tBeing->getMove(), tBeing->getMoney(), tBeing->getBank());
            else if (tBeing->hasClass(CLASS_SHAMAN))
              sprintf(tString, "\n\r\tHit:[%-3d] Pty:[%-4d] Move:[%-3d] Talens:[%-8d] Bank:[%-8d]",
                      tBeing->getHit(), tBeing->getLifeforce(), tBeing->getMove(), tBeing->getMoney(), tBeing->getBank());
            else
              sprintf(tString, "\n\r\tHit:[%-3d] Mna:[%-3d] Move:[%-3d] Talens:[%-8d] Bank:[%-8d]",
                      tBeing->getHit(), tBeing->getMana(), tBeing->getMove(), tBeing->getMoney(), tBeing->getBank());

            strcat(tOutput, tString);
          }

          if ((tBits & (1 << 27)) && isImmortal() &&
              hasWizPower(POWER_WIZARD)) {
            if (tBeing->desc && tBeing->desc->account)
              sprintf(tString, " Account[%s]", tBeing->desc->account->name);
            else
              sprintf(tString, " Account[Unknown]");

            strcat(tOutput, tString);
          }

          if ((tBits & (1 << 11)) && isImmortal()) {
            sprintf(tString, "\n\r\t[St:%-3d Br:%-3d Co:%-3d De:%-3d Ag:%-3d In:%-3d Wi:%-3d Fo:%-3d Pe:%-3d Ch:%-3d Ka:%-3d Sp:%-3d]",
                    tBeing->curStats.get(STAT_STR),
                    tBeing->curStats.get(STAT_BRA),
                    tBeing->curStats.get(STAT_CON),
                    tBeing->curStats.get(STAT_DEX),
                    tBeing->curStats.get(STAT_AGI),
                    tBeing->curStats.get(STAT_INT),
                    tBeing->curStats.get(STAT_WIS),
                    tBeing->curStats.get(STAT_FOC),
                    tBeing->curStats.get(STAT_PER),
                    tBeing->curStats.get(STAT_CHA),
                    tBeing->curStats.get(STAT_KAR),
                    tBeing->curStats.get(STAT_SPE));

            strcat(tOutput, tString);
          }
        }
      }

    return;
  }
#endif

  TBeing *k, *p;
  char buf[1024] = "\0\0\0";
  int listed = 0, lcount, l;
  unsigned int count;
  char arg[1024], tempbuf[1024];
  char tString[256];
  string sb;
  int which1 = 0;
  int which2 = 0;

  for (; isspace(*argument); argument++);

  sb += "Players: (Add -? for online help)\n\r--------\n\r";
  lcount = count = 0;

  if (!*argument || 
       ((sscanf(argument, "%d %d", &which1, &which2) == 2) && 
          which1 > 0 && which2 > 0) ||
       ((sscanf(argument, "%d %d", &which1, &which2) == 1) && 
          which1 > 0  && (which2 = MAX_IMMORT))) {
    // plain old 'who' command 
    // who <level>      level2 assigned to 60
    // who <level> <level2>
    for (p = character_list; p; p = p->next) {
      if (p->isPc() && p->polyed == POLY_TYPE_NONE) {
        if (dynamic_cast<TPerson *>(p)) {
          if (canSeeWho(p) && (!*argument || ((!p->isPlayerAction(PLR_ANONYMOUS) || isImmortal()) && p->GetMaxLevel() >= which1 && p->GetMaxLevel() <= which2))){
            count++;

            p->parseTitle(buf, desc);
            if (!*argument) {
              if (p->isPlayerAction(PLR_SEEKSGROUP))
                sprintf(buf + strlen(buf), "   (Seeking Group)");

              if (p->isPlayerAction(PLR_NEWBIEHELP))
                sprintf(buf + strlen(buf), "   (Newbie-Helper)");

              strcat(buf, "\n\r");
            } else {
              sprintf(buf + strlen(buf), "   %s", getWhoLevel(this, p).c_str());
              if (p->isPlayerAction(PLR_SEEKSGROUP))
                sprintf(buf + strlen(buf), "   (Seeking Group)");

              if (p->isPlayerAction(PLR_NEWBIEHELP))
                sprintf(buf + strlen(buf), "   (Newbie-Helper)");

              sprintf(buf + strlen(buf), "\n\r");
            }
            char tmp[256];
            if (isImmortal() && p->isLinkdead()) {
              sprintf(tmp, "** %s", buf);
            } else {
              sprintf(tmp, "%s%s",
                  (p->polyed == POLY_TYPE_SWITCH ?  "(switched) " : ""), buf);
              sb += tmp;
            }
            //            sb += tmp;
          }
        } else if (isImmortal()) {
// only immortals will see this to provide them some concealment
          if (canSeeWho(p) && 
              (!*argument || 
                (p->GetMaxLevel() >= which1 && p->GetMaxLevel() <= which2)) &&
              IS_SET(p->specials.act, ACT_POLYSELF)) {
            count++;
            strcpy(tempbuf, pers(p));
            sprintf(buf, "%s (polymorphed magic user)\n\r", cap(tempbuf));
            sb += buf;
          } else if (canSeeWho(p) &&
                (!*argument || 
                (p->GetMaxLevel() >= which1 && p->GetMaxLevel() <= which2)) &&
                     IS_SET(p->specials.act, ACT_DISGUISED)) {
            count++;
            strcpy(tempbuf, pers(p));
            sprintf(buf, "%s (disguised thief)\n\r", cap(tempbuf));
            sb += buf;
          }
        }
      }
    }
    accStat.max_player_since_reboot = max(accStat.max_player_since_reboot, count);
    sprintf(buf, "\n\rTotal Players : [%d] Max since last reboot : [%d] Avg Players : [%.1f]\n\r", count, accStat.max_player_since_reboot, stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
    sb += buf;
    if (desc)
      desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);

    return;
  } else {
    argument = one_argument(argument, arg);
    if (*arg == '-') {
      if (strchr(arg, '?')) {
        if (isImmortal()) {
          sb += "[-] [i]idle [l]levels [q]quests [h]hit/mana/move/lf\n\r";
          sb += "[-] [z]seeks-group [p]groups [y]currently-not-grouped\n\r";
          sb += "[-] [d]linkdead [g]God [b]Builders [o]Mort [s]stats [f]action\n\r";
          sb += "[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Deikhan[6]Monk[7]Ranger[8]Shaman\n\r";
          sb += "[-] [e]elf [t]hobbit [n]gnome [u]human [r]ogre [w]dwarven\n\r\n\r";

          if (hasWizPower(POWER_WIZARD))
            sb += "[-] [a]ccount\n\r";

          sb += "\n\r";
        } else {
          sb += "[-] [q]quests [g]god [b]builder [o]mort [f]faction\n\r";
          sb += "[-] [z]seeks-group [p]groups [y]currently-not-grouped\n\r";
          sb += "[-] [e]elf [t]hobbit [n]gnome [u]human [r]ogre [w]dwarven\n\r\n\r";
          sb += "[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Deikhan[6]Monk[7]Ranger[8]Shaman\n\r";
        }
        if (desc)
          desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);
        return;
      }
      bool level, statsx, iPoints, quest, idle, align, group;
      for (p = character_list; p; p = p->next) {
        align = level = statsx = idle = iPoints = quest = group = FALSE;
        if (dynamic_cast<TPerson *>(p) && canSeeWho(p)) {
          count++;
          if (p->isLinkdead())
            lcount++;

          if ((canSeeWho(p) &&
              (!strchr(arg, 'g') || (p->GetMaxLevel() >= GOD_LEVEL1)) &&
              (!strchr(arg, 'b') || (p->GetMaxLevel() >= GOD_LEVEL1)) &&
              (!strchr(arg, 'q') || (p->inQuest())) &&
              (!strchr(arg, 'o') || (p->GetMaxLevel() <= MAX_MORT)) &&
              (!strchr(arg, 'z') || (p->isPlayerAction(PLR_SEEKSGROUP))) &&
              (!strchr(arg, 'p') || (p->isAffected(AFF_GROUP) && !p->master && p->followers)) &&
              (!strchr(arg, 'y') || (!p->isAffected(AFF_GROUP) && !p->isImmortal())) &&
              (!strchr(arg, '1') || (p->hasClass(CLASS_MAGIC_USER) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '2') || (p->hasClass(CLASS_CLERIC) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '3') || (p->hasClass(CLASS_WARRIOR) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '4') || (p->hasClass(CLASS_THIEF) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '5') || (p->hasClass(CLASS_DEIKHAN) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '6') || (p->hasClass(CLASS_MONK) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '7') || (p->hasClass(CLASS_RANGER) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, '8') || (p->hasClass(CLASS_SHAMAN) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
              (!strchr(arg, 'd') || (p->isLinkdead() && isImmortal())) &&
              (!strchr(arg, 'e') || p->getRace() == RACE_ELVEN) &&
              (!strchr(arg, 'n') || p->getRace() == RACE_GNOME) &&
              (!strchr(arg, 'u') || p->getRace() == RACE_HUMAN) &&
              (!strchr(arg, 'w') || p->getRace() == RACE_DWARF) &&
              (!strchr(arg, 'r') || p->getRace() == RACE_OGRE) &&
              (!strchr(arg, 't') || p->getRace() == RACE_HOBBIT))) {
            if (p->isLinkdead() && isImmortal())
              sprintf(buf, "[%-12s] ", pers(p));
            else if (p->polyed == POLY_TYPE_SWITCH && isImmortal())
              sprintf(buf, "[%-12s] (switched) ", pers(p));
            else if (dynamic_cast<TMonster *>(p) &&
                     (p->specials.act & ACT_POLYSELF))
              sprintf(buf, "(%-14s) ", pers(p));
            else 
              sprintf(buf, "%-11s ", pers(p));
            listed++;
            for (l = 1; l <= (int) strlen(arg); l++) {
              switch (arg[l]) {
                case 'p':
                  // we trapped only group leaders above...
                  if (!group) {
                    TBeing *ch;
                    followData *f;
                    for (f = p->followers; f; f = f->next) {
                      ch = f->follower;
                      if (!ch->isPc())
                        continue;
                      if (!canSeeWho(ch))
                        continue;
                      if (ch->isLinkdead() && isImmortal())
                        sprintf(buf, "[%-12s] ", pers(ch));
                      else if (ch->polyed == POLY_TYPE_SWITCH && isImmortal())
                        sprintf(buf, "[%-12s] (switched) ", pers(ch));
                      else if (dynamic_cast<TMonster *>(ch) &&
                               (ch->specials.act & ACT_POLYSELF))
                        sprintf(buf + strlen(buf), "(%-14s) ", pers(ch));
                      else if (ch->isPlayerAction(PLR_ANONYMOUS) && !isImmortal())
                        sprintf(buf + strlen(buf), "%-11s (???) ", pers(ch));
                      else
                        sprintf(buf + strlen(buf), "%-11s (L%d) ", pers(ch), ch->GetMaxLevel());
                    }

                    group = true;
                  }
                  break;
                case 'i':
                  if (!idle) {
                    if (isImmortal())
                      sprintf(buf + strlen(buf), "Idle:[%-3d] ", p->getTimer());
                  }
                  idle = TRUE;
                  break;
                case 'l':
                case 'y':
                  if (!level) {
                    strcat(buf, getWhoLevel(this, p).c_str());
                    if (p->isPlayerAction(PLR_SEEKSGROUP))
                      sprintf(buf + strlen(buf), "   (Seeking Group)");

                    if (p->isPlayerAction(PLR_NEWBIEHELP))
                      sprintf(buf + strlen(buf), "   (Newbie-Helper)");
                  }
                  level = TRUE;
                  break;
                case 'g':
                case 'b':
                  // canSeeWho already separated out invisLevel > my own
                  // only a god can go invis, mortals technically have
                  // invisLevel if they are linkdead, ignore that though
                  if (p->getInvisLevel() > MAX_MORT)
                    sprintf(buf + strlen(buf), "  (invis %d)  ",
                        p->getInvisLevel());
                  break;
                case 'h':
                  if (!iPoints) {
                    if (isImmortal())
                      if (p->hasClass(CLASS_CLERIC)||p->hasClass(CLASS_DEIKHAN))
                        sprintf(buf + strlen(buf), "Hit:[%-3d] Pty:[%-.2f] Move:[%-3d], Talens:[%-8d], Bank:[%-8d]",
                              p->getHit(), p->getPiety(), p->getMove(), p->getMoney(), p->getBank());
                      else if (p->hasClass(CLASS_SHAMAN))
                        sprintf(buf + strlen(buf), "Hit:[%-3d] LF:[%-4d] Move:[%-3d], Talens:[%-8d], Bank:[%-8d]",
                              p->getHit(), p->getLifeforce(), p->getMove(), p->getMoney(), p->getBank());
                      else
                          sprintf(buf + strlen(buf), "Hit:[%-3d] Mana:[%-3d] Move:[%-3d], Talens:[%-8d], Bank:[%-8d]",
                              p->getHit(), p->getMana(), p->getMove(), p->getMoney(), p->getBank());
                  }
                  iPoints = TRUE;
                  break;
                case 'f':
                  if (!align) {
                    // show factions of everyone to immorts
                    // mortal version will show non-imms that are in same fact
                    if(TestCode5) {
		      TFaction *f = NULL;
		      if((f = p->newfaction()) && TestCode5) {
			if (f->ID && (IS_SET(f->flags, FACT_ACTIVE) || newfaction()== p->newfaction()||isImmortal()) &&
			    (!IS_SET(f->flags, FACT_HIDDEN) || newfaction() == p->newfaction() || isImmortal()) &&
			    (!p->isImmortal() || isImmortal())) {
			  sprintf(buf + strlen(buf), "%s[<1>%s%s]<1>",
				  heraldcodes[p->newfaction()->colors[0]],
				  p->newfaction()->getName(),
				  heraldcodes[p->newfaction()->colors[0]]);
			  if(!IS_SET(f->flags, FACT_HIDE_RANKS) || newfaction() == p->newfaction()
			     || isImmortal()) 
			  sprintf(buf + strlen(buf), " %s[<1>%s%s]<1>",
				  heraldcodes[p->newfaction()->colors[1]],
				  p->rank(),
                                  heraldcodes[p->newfaction()->colors[1]]);
			}
		      }
		      

		    } else {
		      if ((getFaction()==p->getFaction() &&
			   p->GetMaxLevel() <= MAX_MORT) || isImmortal()) {
#if FACTIONS_IN_USE
			sprintf(buf + strlen(buf), "[%s] %5.2f%%", 
				FactionInfo[p->getFaction()].faction_name,
				p->getPerc());
#else
			sprintf(buf + strlen(buf), "[%s]", 
				FactionInfo[p->getFaction()].faction_name);
#endif
		      }
		    }
		  }
                  align = TRUE;
                  break;
                case 's':
                  if (!statsx) {
                    if (isImmortal())
                      sprintf(buf + strlen(buf), "\n\r\t[St:%-3d Br:%-3d Co:%-3d De:%-3d Ag:%-3d In:%-3d Wi:%-3d Fo:%-3d Pe:%-3d Ch:%-3d Ka:%-3d Sp:%-3d]",
                        p->curStats.get(STAT_STR),
                        p->curStats.get(STAT_BRA),
                        p->curStats.get(STAT_CON),
                        p->curStats.get(STAT_DEX),
                        p->curStats.get(STAT_AGI),
                        p->curStats.get(STAT_INT),
        		p->curStats.get(STAT_WIS),
			p->curStats.get(STAT_FOC),
			p->curStats.get(STAT_PER),
			p->curStats.get(STAT_CHA),
			p->curStats.get(STAT_KAR),
			p->curStats.get(STAT_SPE));
                  }
                  statsx = TRUE;
                  break;
                case 'q':
                  if (!quest) {
                    if (p->isPlayerAction(PLR_SOLOQUEST))
                      sprintf(buf + strlen(buf), " (%sSOLO QUEST%s)", red(), norm());
                    
                    if (p->isPlayerAction(PLR_GRPQUEST))
                      sprintf(buf + strlen(buf), " (%sGROUP QUEST%s)", blue(), norm());
                  }
                  quest = TRUE;
                  break;
                case 'a':
                  if (isImmortal() && hasWizPower(POWER_WIZARD)) {
                    if (p->desc && p->desc->account)
                      sprintf(tString, " Account[%s]", p->desc->account->name);
                    else
                      sprintf(tString, " Account[Unknown]");

                    strcat(buf, tString);
                  }
                  break;
                default:
                  break;
              }        // end of switch statement 
            }        // end of for-loop 
            strcat(buf, "\n\r");
            sb += buf;
          }        // end of 'should I skip this fool' if-statement 
        }        // end of !NPC(p) loop 
      }                // end of 'step through the character list loop 
    } else {
      // 'who playername' command 
      int c = 0;
      for (k = character_list; k; k = k->next) {
        if (!k->isPc() || !isname(arg, k->name) || !canSee(k)) 
          continue;
 
        c++;
        *buf = '\0';
        k->parseTitle(buf, desc);
        strcat(buf, "    ");
        strcat(buf, getWhoLevel(this, k).c_str());
        if (k->isPlayerAction(PLR_SEEKSGROUP))
          sprintf(buf + strlen(buf), "   (Seeking Group)");

        if (k->isLinkdead() && isImmortal())
          strcat(buf, "   (link-dead)");

        if (k->polyed == POLY_TYPE_SWITCH && isImmortal())
          strcat(buf, "   (switched)");

        if (k->isPlayerAction(PLR_NEWBIEHELP))
          sprintf(buf + strlen(buf), "   (Newbie-Helper)");

        strcat(buf, "\n\r");
        sb += buf;
      }
      if (!c)
        sb += "No one logged in with that name.\n\r";

      if (desc)
        desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);
      return;
    }
  }
  accStat.max_player_since_reboot = max(accStat.max_player_since_reboot, count);
  if (isImmortal()) {
    if (!listed)
      sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%)\n\rMax since Reboot [%d]  Avg Players : [%.1f]\n\r",
           count, lcount, ((double) lcount / (int) count) * 100, 
           accStat.max_player_since_reboot,
           stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
    else
      sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%)\n\rNumber Listed: %d  Max since Reboot [%d]  Avg Players : [%.1f]\n\r", 
           count, lcount, ((double) lcount / (int) count) * 100, listed,
           accStat.max_player_since_reboot,
           stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
  } else {
    sprintf(buf, "\n\rTotal Players : [%d] Max since last reboot : [%d] Avg Players : [%.1f]\n\r", count, accStat.max_player_since_reboot, stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
  }
  sb += buf;
  if (desc)
    desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);
  return;
}

void TBeing::doWhozone()
{
  Descriptor *d;
  TRoom *rp = NULL;
  char buf[256];
  TBeing *person = NULL;
  int count = 0;

  sendTo("Players:\n\r--------\n\r");
  for (d = descriptor_list; d; d = d->next) {
    if (!d->connected && canSee(d->character) &&
        (rp = real_roomp((person = (d->original ? d->original : d->character))->in_room)) &&
        (rp->getZoneNum() == roomp->getZoneNum())) {
      sprintf(buf, "%-25s - %s ", person->getName(), rp->name);
      if (GetMaxLevel() > MAX_MORT)
        sprintf(buf + strlen(buf), "[%d]", person->in_room);
      strcat(buf, "\n\r");
      sendTo(COLOR_BASIC, buf);
      count++;
    }
  }
  sendTo("\n\rTotal visible players: %d\n\r", count);
}
