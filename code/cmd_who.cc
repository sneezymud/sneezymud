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
#include "database.h"

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
      sstring::npos)
    strcpy(arg, getName());  // did not specify a <n>

  // explicitely terminate it since players are sloppy
  strcat(arg, "<1>");

  return;
}

void Descriptor::menuWho() 
{
  TBeing *person;
  char buf[256];
  sstring buf2, send;

  send="\n\r";

  for (person = character_list; person; person = person->next) {
    if (person->isPc() && person->polyed == POLY_TYPE_NONE) {
      if (dynamic_cast<TPerson *>(person) &&
          (person->getInvisLevel() < GOD_LEVEL1)) {
        person->parseTitle(buf, this);

        buf2 = fmt("%s\n\r") % colorString(person, this, buf, NULL, COLOR_BASIC, FALSE);
	send += buf2;
      }
    }
  }
  send += "\n\r";
  writeToQ(send);
  writeToQ("[Press return to continue]\n\r");
}

static const sstring getWizDescriptLev(const TBeing *ch)
{
  if (ch->hasWizPower(POWER_WIZARD))
    return "creator";
  else if (ch->hasWizPower(POWER_GOD)) {
    if(ch->getSex() == SEX_FEMALE)
      return "godess ";
    else
      return "  god  ";
  }
  else if (ch->hasWizPower(POWER_BUILDER))
    return "demigod";
  else
    return "BUG ME!";
}

static const sstring getWhoLevel(const TBeing *ch, TBeing *p)
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
    sstring str = p->msgVariables(MSG_IMM_TITLE);
    unsigned int len = str.size();
    unsigned int padding = 14-len;
    unsigned int frontpadding = padding/2;
    for (unsigned int iter = 0; iter < frontpadding; iter++)
      str.insert(0, " ");
    
    sprintf(tempbuf, "%sLevel:[%-14s%s][%s] %s",
            colorBuf, str.c_str(),
            colorBuf, getWizDescriptLev(p).c_str(), ch->norm());
  } else {
    sstring tmpstring;

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
  TBeing *k, *p;
  //  char buf[1024] = "\0\0\0";
  sstring buf;
  int listed = 0, lcount, l;
  unsigned int count;
  char arg[1024], tempbuf[1024];
  char tString[256];
  sstring sb;
  int which1 = 0;
  int which2 = 0;

  sstring stmp;
  unsigned int pos;
		  
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

            p->parseTitle(tempbuf, desc);
	    buf = tempbuf;

            if (!*argument) {
              if (p->isPlayerAction(PLR_SEEKSGROUP))
		buf += "   (Seeking Group)";

              if (p->isPlayerAction(PLR_NEWBIEHELP))
		buf += "   (Newbie-Helper)";

	      buf += "\n\r";
            } else {
	      buf += "   " + getWhoLevel(this, p);

              if (p->isPlayerAction(PLR_SEEKSGROUP))
		buf += "   (Seeking Group)";

              if (p->isPlayerAction(PLR_NEWBIEHELP))
		buf += "   (Newbie-Helper)";

	      buf += "\n\r";
            }
            if (isImmortal() && p->isLinkdead()) {
            } else {
	      sb += (p->polyed == POLY_TYPE_SWITCH ?  "(switched) " : "") + buf;
	    }
          }
        } else if (isImmortal()) {
// only immortals will see this to provide them some concealment
          if (canSeeWho(p) && 
              (!*argument || 
                (p->GetMaxLevel() >= which1 && p->GetMaxLevel() <= which2)) &&
              IS_SET(p->specials.act, ACT_POLYSELF)) {
            count++;
            buf = fmt("%s (polymorphed)\n\r") % sstring(pers(p)).cap();
            sb += buf;
          } else if (canSeeWho(p) &&
                (!*argument || 
                (p->GetMaxLevel() >= which1 && p->GetMaxLevel() <= which2)) &&
                     IS_SET(p->specials.act, ACT_DISGUISED)) {
            count++;
            buf = fmt("%s (disguised thief)\n\r") % sstring(pers(p)).cap();
            sb += buf;
          }
        }
      }
    }
    accStat.max_player_since_reboot = max(accStat.max_player_since_reboot, count);
    buf = fmt("\n\rTotal Players : [%d] Max since last reboot : [%d] Avg Players : [%.1f]\n\r") % count % accStat.max_player_since_reboot % (stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
    sb += buf;
    if (desc)
      desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);

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
	  sb += "[-] [x]Perma Death [c]ports\n\r";

          if (hasWizPower(POWER_WIZARD))
            sb += "[-] [a]ccount\n\r";

          sb += "\n\r";
        } else {
          sb += "[-] [q]quests [g]god [b]builder [o]mort [f]faction\n\r";
          sb += "[-] [z]seeks-group [p]groups [y]currently-not-grouped\n\r";
          sb += "[-] [e]elf [t]hobbit [n]gnome [u]human [r]ogre [w]dwarven\n\r\n\r";
          sb += "[-] [1]Mage[2]Cleric[3]War[4]Thief[5]Deikhan[6]Monk[7]Ranger[8]Shaman\n\r";
	  sb += "[-] [x]Perma Death\n\r";
        }
        if (desc)
          desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
        return;
      }

      if(strchr(arg, 'c') && isImmortal()){
	
	buf = fmt("%sList may not be accurate for ports that are not currently running.\n\r") % buf;
	buf = fmt("%s------------------------------------------------------------------\n\r") % buf;
        buf = fmt("%sProduction (Port 7900)\n\r") % buf;
        buf = fmt("%s------------------------------------------------------------------\n\r") % buf;


	TDatabase db(DB_SNEEZYPROD);
        db.query("select title, port, name from wholist order by port");

	while(db.fetchRow()){
	  stmp=db["title"];
	  
	  if((pos=stmp.find("<n>")) != sstring::npos)
	    stmp.replace(pos,3,db["name"]);

	  if((pos=stmp.find("<N>")) != sstring::npos)
	    stmp.replace(pos,3,db["name"]);

	  
	  buf = fmt("%s[%s] %s<1>\n\r") % buf %
		   db["port"] % stmp;
	}
	
        buf = fmt("%s------------------------------------------------------------------\n\r") % buf;
        buf = fmt("%sBuilder (Port 8900)\n\r") % buf;
        buf = fmt("%s------------------------------------------------------------------\n\r") % buf;

        TDatabase db2(DB_SNEEZYBUILDER);
        db2.query("select title, port, name from wholist order by port");

        while(db2.fetchRow()){
          stmp=db2["title"];

          if((pos=stmp.find("<n>")) != sstring::npos)
            stmp.replace(pos,3,db2["name"]);

          if((pos=stmp.find("<N>")) != sstring::npos)
            stmp.replace(pos,3,db2["name"]);


          buf = fmt("%s[%s] %s<1>\n\r") % buf %
                   db2["port"] % stmp;
        }
	
        buf = fmt("%s------------------------------------------------------------------\n\r") % buf;
        buf = fmt("%sTesting (Other Ports)\n\r") % buf;
        buf = fmt("%s------------------------------------------------------------------\n\r") % buf;


        TDatabase db3(DB_SNEEZYBETA);
        db3.query("select title, port, name from wholist order by port");

        while(db3.fetchRow()){
          stmp=db3["title"];

          if((pos=stmp.find("<n>")) != sstring::npos)
            stmp.replace(pos,3,db3["name"]);

          if((pos=stmp.find("<N>")) != sstring::npos)
            stmp.replace(pos,3,db3["name"]);


          buf = fmt("%s[%s] %s<1>\n\r") % buf %
                   db3["port"] % stmp;
        }


	sb += buf;
	
        if (desc)
          desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);

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
              (!strchr(arg, '1') || (p->hasClass(CLASS_MAGE) && (isImmortal() || !p->isPlayerAction(PLR_ANONYMOUS)))) &&
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
              (!strchr(arg, 't') || p->getRace() == RACE_HOBBIT) &&
	      (!strchr(arg, 'x') || p->hasQuestBit(TOG_PERMA_DEATH_CHAR)))) {
            if (p->isLinkdead() && isImmortal())
              buf = fmt("[%-12s] ") % pers(p);
            else if (p->polyed == POLY_TYPE_SWITCH && isImmortal())
              buf = fmt("[%-12s] (switched) ") % pers(p);
            else if (dynamic_cast<TMonster *>(p) &&
                     (p->specials.act & ACT_POLYSELF))
              buf = fmt("(%-14s) ") % pers(p);
            else 
              buf = fmt("%-11s ") % pers(p);
            listed++;
            for (l = 1; l <= (int) strlen(arg); l++) {
              switch (arg[l]) {
                case 'p':
                  // we trapped only group leaders above...
                  if (!group) {
                    TBeing *ch;
                    followData *f;

		    if(p->desc)
		      buf = fmt("Group: %s\n\r%s") %
			   p->desc->session.groupName % buf;

                    for (f = p->followers; f; f = f->next) {
                      ch = f->follower;
                      if (!ch->isPc())
                        continue;
                      if (!canSeeWho(ch))
                        continue;
                      if (ch->isLinkdead() && isImmortal())
                        buf = fmt("%s[%-12s] ") % buf % pers(ch);
                      else if (ch->polyed == POLY_TYPE_SWITCH && isImmortal())
                        buf = fmt("%s[%-12s] (switched) ") % buf % pers(ch);
                      else if (dynamic_cast<TMonster *>(ch) &&
                               (ch->specials.act & ACT_POLYSELF))
                        buf = fmt("%s(%-14s) ") % buf % pers(ch);
                      else if (ch->isPlayerAction(PLR_ANONYMOUS) && !isImmortal())
                        buf = fmt("%s%-11s (??\?) ") % buf % pers(ch);
                      else
                        buf = fmt("%s%-11s (L%d) ") % buf % pers(ch) % ch->GetMaxLevel();
                    }

                    group = true;
                  }
                  break;
                case 'i':
                  if (!idle) {
                    if (isImmortal())
                      buf = fmt("%sIdle:[%-3d] ") % buf % p->getTimer();
                  }
                  idle = TRUE;
                  break;
                case 'l':
                case 'y':
                  if (!level) {
		    buf += getWhoLevel(this, p);

                    if (p->isPlayerAction(PLR_SEEKSGROUP))
		      buf += "   (Seeking Group)";

                    if (p->isPlayerAction(PLR_NEWBIEHELP))
		      buf += "   (Newbie-Helper)";
                  }
                  level = TRUE;
                  break;
                case 'g':
                case 'b':
                  // canSeeWho already separated out invisLevel > my own
                  // only a god can go invis, mortals technically have
                  // invisLevel if they are linkdead, ignore that though
                  if (p->getInvisLevel() > MAX_MORT)
                    buf = fmt("%s  (invis %d)  ") % buf %
                        p->getInvisLevel();
                  break;
                case 'h':
                  if (!iPoints) {
                    if (isImmortal())
                      if (p->hasClass(CLASS_CLERIC)||p->hasClass(CLASS_DEIKHAN))
                        buf = fmt("%sHit:[%-3d] Pty:[%-.2f] Move:[%-3d], Talens:[%-8d], Bank:[%-8d]") % 
			  buf % p->getHit() % p->getPiety() % p->getMove() %
			  p->getMoney() % p->getBank();
                      else if (p->hasClass(CLASS_SHAMAN))
                        buf = fmt("%sHit:[%-3d] LF:[%-4d] Move:[%-3d], Talens:[%-8d], Bank:[%-8d]") %
			  buf % p->getHit() % p->getLifeforce() % 
			  p->getMove() % p->getMoney() % p->getBank();
                      else
			buf = fmt("%sHit:[%-3d] Mana:[%-3d] Move:[%-3d], Talens:[%-8d], Bank:[%-8d]") % 
			  buf % p->getHit() % p->getMana() % p->getMove() %
			  p->getMoney() % p->getBank();
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
			  buf = fmt("%s%s[<1>%s%s]<1>") % buf %
			    heraldcodes[p->newfaction()->colors[0]] %
			    p->newfaction()->getName() %
			    heraldcodes[p->newfaction()->colors[0]];
			  if(!IS_SET(f->flags, FACT_HIDE_RANKS) || newfaction() == p->newfaction()
			     || isImmortal()) 
			    buf = fmt("%s %s[<1>%s%s]<1>") % buf %
			      heraldcodes[p->newfaction()->colors[1]] %
			      p->rank() %
			      heraldcodes[p->newfaction()->colors[1]];
			}
		      }
		      

		    } else {
		      if ((getFaction()==p->getFaction() &&
			   p->GetMaxLevel() <= MAX_MORT) || isImmortal()) {
#if FACTIONS_IN_USE
			buf = fmt("%s[%s] %5.2f%c") % buf %
			  FactionInfo[p->getFaction()].faction_name %
			  p->getPerc() % '%';
#else
			buf = fmt("%s[%s]") % buf %
				FactionInfo[p->getFaction()].faction_name;
#endif
		      }
		    }
		  }
                  align = TRUE;
                  break;
                case 's':
                  if (!statsx) {
                    if (isImmortal())
                      buf = fmt("%s\n\r\t[St:%-3d Br:%-3d Co:%-3d De:%-3d Ag:%-3d In:%-3d Wi:%-3d Fo:%-3d Pe:%-3d Ch:%-3d Ka:%-3d Sp:%-3d]") % 
			buf %
			p->curStats.get(STAT_STR) %
			p->curStats.get(STAT_BRA) %
			p->curStats.get(STAT_CON) %
			p->curStats.get(STAT_DEX) %
			p->curStats.get(STAT_AGI) %
			p->curStats.get(STAT_INT) %
			p->curStats.get(STAT_WIS) %
			p->curStats.get(STAT_FOC) %
			p->curStats.get(STAT_PER) %
			p->curStats.get(STAT_CHA) %
			p->curStats.get(STAT_KAR) %
			p->curStats.get(STAT_SPE);
                  }
                  statsx = TRUE;
                  break;
                case 'q':
                  if (!quest) {
                    if (p->isPlayerAction(PLR_SOLOQUEST))
                      buf = fmt("%s (%sSOLO QUEST%s)") % buf % red() % norm();
                    
                    if (p->isPlayerAction(PLR_GRPQUEST))
                      buf = fmt("%s (%sGROUP QUEST%s)") % buf % blue() % norm();
                  }
                  quest = TRUE;
                  break;
                case 'a':
                  if (isImmortal() && hasWizPower(POWER_WIZARD)) {
                    if (p->desc && p->desc->account)
                      sprintf(tString, " Account[%s]", p->desc->account->name);
                    else
                      sprintf(tString, " Account[Unknown]");

		    buf += tString;
                  }
                  break;
                default:
                  break;
              }        // end of switch statement 
            }        // end of for-loop 
	    buf += "\n\r";
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
        k->parseTitle(tempbuf, desc);
	buf = tempbuf;
	buf += "    ";
	buf += getWhoLevel(this, k);
        if (k->isPlayerAction(PLR_SEEKSGROUP))
	  buf += "   (Seeking Group)";

        if (k->isLinkdead() && isImmortal())
	  buf += "   (link-dead)";

        if (k->polyed == POLY_TYPE_SWITCH && isImmortal())
	  buf += "   (switched)";

        if (k->isPlayerAction(PLR_NEWBIEHELP))
	  buf += "   (Newbie-Helper)";

	buf += "\n\r";
        sb += buf;
      }
      if (!c)
        sb += "No one logged in with that name.\n\r";

      if (desc)
        desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
      return;
    }
  }
  accStat.max_player_since_reboot = max(accStat.max_player_since_reboot, count);
  if (isImmortal()) {
    if (!listed)
      buf = fmt("\n\rTotal players / Link dead [%d/%d] (%2.0f%c)\n\rMax since Reboot [%d]  Avg Players : [%.1f]\n\r") %
	count % lcount % (((double) lcount / (int) count) * 100) % '%' %
	accStat.max_player_since_reboot %
	(stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
    else
      buf = fmt("\n\rTotal players / Link dead [%d/%d] (%2.0f%c)\n\rNumber Listed: %d  Max since Reboot [%d]  Avg Players : [%.1f]\n\r") %
	count % lcount % (((double) lcount / (int) count) * 100) % '%' % listed %
	accStat.max_player_since_reboot %
	(stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
  } else {
    buf = fmt("\n\rTotal Players : [%d] Max since last reboot : [%d] Avg Players : [%.1f]\n\r") % count % accStat.max_player_since_reboot % (stats.useage_iters ? (float) stats.num_users / stats.useage_iters : 0);
  }
  sb += buf;
  if (desc)
    desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
  return;
}

void TBeing::doWhozone()
{
  Descriptor *d;
  TRoom *rp = NULL;
  sstring sbuf, buf;
  TBeing *person = NULL;
  int count = 0;

  sendTo("Players:\n\r--------\n\r");
  for (d = descriptor_list; d; d = d->next) {
    if (!d->connected && canSee(d->character) &&
        (rp = real_roomp((person = (d->original ? d->original : d->character))->in_room)) &&
        (rp->getZoneNum() == roomp->getZoneNum())) {
      sbuf = fmt("%-25s - %s ") % person->getName() % rp->name;
      if (GetMaxLevel() > MAX_MORT){
        buf = fmt("[%d]") % person->in_room;
	sbuf+=buf;
      }
      sbuf += "\n\r";
      sendTo(COLOR_BASIC, sbuf);
      count++;
    }
  }
  sendTo(fmt("\n\rTotal visible players: %d\n\r") % count);
}
