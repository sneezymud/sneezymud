///////////////////////////////////////////////////////////////////////////
//
//     SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
//
//     "color.cc" - Functions related to screen and color
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

void TBeing::doPrompt(const char *arg)
{
  sstring tStString("");
  char str[512];
  char caColor[50], caStat[200];
  int field, statnum;
  int kolor2;
  setColorKolorT kolor;

  const char *colors[] =
  {
    "blue",
    "red",
    "green",
    "white",
    "purple",
    "cyan",
    "orange",
    "yellow",
    "charcoal",
    "white_on_blue",
    "invert",
    "white_on_cyan",
    "white_on_red",
    "white_on_purple",
    "white_on_green",
    "white_on_yellow",
    "blinking",
    "boldred",
    "boldgreen",
    "boldblue",
    "boldpurple",
    "boldcyan",
    "\n"
  };

  const char *stat_fields[] =
  {
    "hit",
    "mana",
    "movement",
    "talens",
    "exp",
    "opponent",
    "color",
    "room",
    "off",
    "all",
    "none",
    "tank",
    "tank-other",
    "piety",
    "builder_assistant",
    "exp_tolevel",
    "bar",
    "lifeforce",
    "client-prompt",
    "classic-ansi-bar",
    "\n"
  };

  if (!desc)
    return;

  if (!arg || !*arg || *arg == '\n') {
    bool tPrompts[] =
    {
      IS_SET(desc->prompt_d.type, PROMPT_HIT),
      IS_SET(desc->prompt_d.type, PROMPT_MANA),
      IS_SET(desc->prompt_d.type, PROMPT_MOVE),
      IS_SET(desc->prompt_d.type, PROMPT_GOLD),
      IS_SET(desc->prompt_d.type, PROMPT_EXP),
      IS_SET(desc->prompt_d.type, PROMPT_NAME),
      IS_SET(desc->prompt_d.type, PROMPT_OPPONENT),
      IS_SET(desc->prompt_d.type, PROMPT_CONDITION),
      IS_SET(desc->prompt_d.type, PROMPT_COND_LDR),
      IS_SET(desc->prompt_d.type, PROMPT_ROOM),
      IS_SET(desc->prompt_d.type, PROMPT_COLOR),
      IS_SET(desc->prompt_d.type, PROMPT_TANK),
      IS_SET(desc->prompt_d.type, PROMPT_TANK_OTHER),
      IS_SET(desc->prompt_d.type, PROMPT_BUILDER_ASSISTANT),
      IS_SET(desc->prompt_d.type, PROMPT_EXPTONEXT_LEVEL),
      IS_SET(desc->prompt_d.type, PROMPT_VTANSI_BAR),
      IS_SET(desc->prompt_d.type, PROMPT_PIETY),
      IS_SET(desc->prompt_d.type, PROMPT_LIFEFORCE),
      IS_SET(desc->prompt_d.type, PROMPT_CLIENT_PROMPT),
      IS_SET(desc->prompt_d.type, PROMPT_CLASSIC_ANSIBAR),
    };

    tStString += "Prompt Line Options:\n\r--------------------\n\r";

    sprintf(str, "Hit        : (%s): H:%d\n\r", (tPrompts[0] ? "yes" : " no"), getHit());
    tStString += str;

    sprintf(str, "Piety      : (%s): P:%.1f\n\r", (tPrompts[16] ? "yes" : " no"), getPiety());
    tStString += str;

    sprintf(str, "Lifeforce  : (%s): LF:%d\n\r", (tPrompts[17] ? "yes" : " no"), getLifeforce());
    tStString += str;

    sprintf(str, "Mana       : (%s): M:%d\n\r", (tPrompts[1] ? "yes" : " no"), getMana());
    tStString += str;

    sprintf(str, "Movement   : (%s): V:%d\n\r", (tPrompts[2] ? "yes" : " no"), getMove());
    tStString += str;

    sprintf(str, "Talens     : (%s): T:%d\n\r", (tPrompts[3] ? "yes" : " no"), getMoney());
    tStString += str;

    strcpy(caStat, displayExp().comify().c_str());

    sprintf(str, "Exp        : (%s): E:%s\n\r", (tPrompts[4] ? "yes" : " no"), caStat);
    tStString += str;

    for (classIndT tClassIndex = MAGE_LEVEL_IND; tClassIndex < MAX_CLASSES; tClassIndex++)
      if (getLevel(tClassIndex) && getLevel(tClassIndex) < MAX_MORT) {
        if (getExp() > desc->prompt_d.xptnl)
          desc->prompt_d.xptnl = getExpClassLevel(tClassIndex, getLevel(tClassIndex) + 1);

        double tNeed = desc->prompt_d.xptnl - getExp();

        sprintf(caStat, "%.0f", tNeed);

        strcpy(caStat, sstring(caStat).comify().c_str());

        sprintf(str, "Exp_tolevel: (%s): N:%s\n\r", (tPrompts[14] ? "yes" : " no"), caStat);
        tStString += str;
      }

    if (isImmortal()) {
      sprintf(str, "Room       : (%s): R:%d\n\r", (tPrompts[9] ? "yes" : " no"), roomp->number);
      tStString += str;
    }

    tStString += "--------------------\n\r";
    sprintf(str, "Opponent  : (%s): Current target when in battle.\n\r",
            (tPrompts[6] ? "yes" : " no"));
    tStString += str;
    sprintf(str, "Tank      : (%s): Current tank when in battle, including self.\n\r",
            (tPrompts[11] ? "yes" : " no"));
    tStString += str;
    sprintf(str, "Tank-other: (%s): Current tank when in battle, excluding self.\n\r",
            (tPrompts[12] ? "yes" : " no"));
    tStString += str;
    tStString += "--------------------\n\r";

    sprintf(str, "Classic Bar   : %s\n\r", (tPrompts[19] ? "On" : "Off"));
    tStString += str;

    sprintf(str, "Client Prompts: %s\n\r", (tPrompts[18] ? "On" : "Off"));
    tStString += str;

    tStString += "--------------------\n\r";

    desc->page_string(tStString);

    return;
  }

  bisect_arg(arg, &field, str, stat_fields);

  switch (field) {
    case 1:
      if (IS_SET(desc->prompt_d.type, PROMPT_HIT)) {
        sendTo("Taking hit points out of prompt.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_HIT);
      } else {
        sendTo("Adding hit points to prompt.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_HIT);
      }
      break;
    case 2:
      if (IS_SET(desc->prompt_d.type, PROMPT_MANA)) {
	sendTo("Taking mana out of prompt.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_MANA);
      } else {
	sendTo("Adding mana points to prompt.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_MANA);
      }
      break;
    case 18:
      if (IS_SET(desc->prompt_d.type, PROMPT_LIFEFORCE)) {
	sendTo("Taking lifeforce out of prompt.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_LIFEFORCE);
      } else {
	sendTo("Adding lifeforce points to prompt.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_LIFEFORCE);
      }
      break;
    case 14:
      if (IS_SET(desc->prompt_d.type, PROMPT_PIETY)) {
	sendTo("Taking piety out of prompt.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_PIETY);
      } else {
	sendTo("Adding piety to prompt.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_PIETY);
      }
      break;
    case 3:
      if (IS_SET(desc->prompt_d.type, PROMPT_MOVE)) {
        sendTo("Taking moves out of prompt.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_MOVE);
      } else {
        sendTo("Adding moves points to prompt.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_MOVE);
      }
      break;
    case 4:
      if (IS_SET(desc->prompt_d.type, PROMPT_GOLD)) {
        sendTo("Taking talens out of prompt.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_GOLD);
      } else {
        sendTo("Adding talens to prompt.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_GOLD);
      }
      break;
    case 5:
      if (IS_SET(desc->prompt_d.type, PROMPT_EXP)) {
        sendTo("Taking exp out of prompt.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_EXP);
      } else {
        sendTo("Adding exp to prompt.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_EXP);
      }
      break;
    case 6:
      if (IS_SET(desc->prompt_d.type, PROMPT_OPPONENT)) {
        sendTo("Taking opponent out of prompt.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_OPPONENT);
      } else {
        sendTo("Adding opponent to prompt.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_OPPONENT);
      }
      break;
    case 7:
      sscanf(str, "%s %s ", caStat, caColor);
      if (is_abbrev(caStat, "off")) {
        sendTo("Turning off color prompting.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_COLOR);
        *desc->prompt_d.hpColor = '\0';
        *desc->prompt_d.manaColor = '\0';
        *desc->prompt_d.moveColor = '\0';
        *desc->prompt_d.moneyColor = '\0';
        *desc->prompt_d.expColor = '\0';
        *desc->prompt_d.roomColor = '\0';
        *desc->prompt_d.oppColor = '\0';
        *desc->prompt_d.tankColor = '\0';
        *desc->prompt_d.pietyColor = '\0';
        *desc->prompt_d.lifeforceColor = '\0';
        return;
      }
      statnum = old_search_block(caStat, 0, strlen(caStat), stat_fields, 0);
      if (!statnum || (statnum <= 0)) {
        sendTo("Invalid stat for coloring.\n\r");
        sendTo("Syntax: prompt color <arg> <color>:\n\r<arg> is one of:\n\r");
        unsigned int ui;
        for (ui = 0; *stat_fields[ui] != '\n'; ui++)
          sendTo(fmt("\t%s\n\r") % stat_fields[ui]);
        return;
      }
      if (is_abbrev(caColor, "off")) 
        kolor2 = 0;
      else {
        kolor2 = old_search_block(caColor, 0, strlen(caColor), colors, 0);
        if (!kolor2 || (kolor2 <= 0)) {
          sendTo("Invalid color.\n\r");
          sendTo(COLOR_BASIC, "Valid arguments are: "
             "  <b>blue<1>               <p>purple<1>\n\r"
             "  <r>red<1>                <g>green<1>\n\r"
             "  <o>orange<1>             <c>cyan<1>\n\r"
             "  <Y>yellow<1>             <k>charcoal<1>\n\r"
             "  <C>boldcyan<1>           <W>white<1> (bright/bold)\n\r"
             "  <R>boldred<1>            <G>boldgreen<1>\n\r"
             "  <B>boldblue<1>           <P>boldpurple<1>\n\r"
             "  <u>white_on_blue<1>      <v>white_on_purple<1>\n\r"
             "  <l>white_on_red<1>       <q>white_on_green<1>\n\r"
             "  <t>white_on_yellow<1>    <x>white_on_cyan<1>\n\r"
             "  <i>inverted<1>");
          return;
        }
      }

      kolor = setColorKolorT(kolor2);

      if (statnum == 1) {
        if (!IS_SET(desc->prompt_d.type, PROMPT_HIT)) {
          sendTo("You can't color hit points, without them in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_HIT, kolor);
      } else if (statnum == 2) {
        if (!IS_SET(desc->prompt_d.type, PROMPT_MANA)) {
          sendTo("You can't color mana points, without them in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_MANA, kolor);
      } else if (statnum == 14) {
         // YES, MANA
        if (!IS_SET(desc->prompt_d.type, PROMPT_PIETY)) {
          sendTo("You can't color piety points, without them in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_PIETY, kolor);
      } else if (statnum == 18) {
         // YES, MANA
        if (!IS_SET(desc->prompt_d.type, PROMPT_LIFEFORCE)) {
          sendTo("You can't color lifeforce points, without them in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_LIFEFORCE, kolor);
      } else if (statnum == 3) {
        if (!IS_SET(desc->prompt_d.type, PROMPT_MOVE)) {
          sendTo("You can't color movement points, without them in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_MOVE, kolor);
      } else if (statnum == 4) {
        if (!IS_SET(desc->prompt_d.type, PROMPT_GOLD)) {
          sendTo("You can't color talens without it in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_TALEN, kolor);
      } else if (statnum == 5) {
        if (!IS_SET(desc->prompt_d.type, PROMPT_EXP)) {
          sendTo("You can't color exp points, without them in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_XP, kolor);
      } else if (statnum == 16) {
        if (!IS_SET(desc->prompt_d.type, PROMPT_EXPTONEXT_LEVEL)) {
          sendTo("You can't color exp to next level, without them in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_XP, kolor);
      } else if (statnum == 6) {
        if (!IS_SET(desc->prompt_d.type, PROMPT_OPPONENT)) {
          sendTo("You can't color opponent, without them in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_OPP, kolor);
      } else if (statnum == 8) {
        if (!IS_SET(desc->prompt_d.type, PROMPT_ROOM)) {
          sendTo("You can't color room, without them in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_ROOM, kolor);
      } else if (statnum == 12) {
        if ((!(IS_SET(desc->prompt_d.type, PROMPT_TANK))) && (!(IS_SET(desc->prompt_d.type, PROMPT_TANK_OTHER)))) {
          sendTo("You can't color tank, without tank or tank-other in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_TANK, kolor);
      } else if (statnum == 13) {
        if ((!(IS_SET(desc->prompt_d.type, PROMPT_TANK))) && (!(IS_SET(desc->prompt_d.type, PROMPT_TANK_OTHER)))) {
          sendTo("You can't color tank-other, without tank or tank-other in your prompt.\n\r");
          return;
        } else
          setColor(SET_COL_FIELD_TANK_OTHER, kolor);
      } else {
        sendTo("Bad arguments.\n\r");
      }
      break;
    case 8:
      if (isImmortal()) {
        if (IS_SET(desc->prompt_d.type, PROMPT_ROOM)) {
          sendTo("Taking room out of prompt.\n\r");
          REMOVE_BIT(desc->prompt_d.type, PROMPT_ROOM);
        } else {
          sendTo("Adding room to prompt.\n\r");
          SET_BIT(desc->prompt_d.type, PROMPT_ROOM);
        }
      }
      break;
    case 9:
      sendTo("Setting prompt to none.\n\r");
      desc->prompt_d.type = 0;
      break;
    case 10:
      sendTo("Setting prompt to all available prompts.\n\r");
      // add PROMPT_TANK_OTHER rather than TANK
      desc->prompt_d.type = PROMPT_HIT | PROMPT_MOVE | PROMPT_MANA |
                    PROMPT_GOLD | PROMPT_EXP | PROMPT_OPPONENT |
                    PROMPT_TANK_OTHER | PROMPT_EXPTONEXT_LEVEL |
	            PROMPT_PIETY | PROMPT_LIFEFORCE |
                    (isImmortal() ? PROMPT_ROOM : 0);
      break;
    case 11:
      sendTo("Setting prompt to none.\n\r");
      desc->prompt_d.type = 0;
      break;
    case 12:
      if (IS_SET(desc->prompt_d.type, PROMPT_TANK)) {
        sendTo("Taking tank out of prompt.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_TANK);
      } else if (IS_SET(desc->prompt_d.type, PROMPT_TANK_OTHER)) {
        sendTo("You can't have both tank and tank-other in prompt.\n\r");
      } else {
        sendTo("Adding tank to prompt.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_TANK);
      }
      break;
    case 13:
      if (IS_SET(desc->prompt_d.type, PROMPT_TANK_OTHER)) {
        sendTo("Taking tank-other out of prompt.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_TANK_OTHER);
      } else if (IS_SET(desc->prompt_d.type, PROMPT_TANK)) {
        sendTo("You can't have both tank and tank-other in prompt.\n\r");
      } else {
        sendTo("Adding tank-other to prompt.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_TANK_OTHER);
      }
      break;
    case 15:
      if (isImmortal())
        if (IS_SET(desc->prompt_d.type, PROMPT_BUILDER_ASSISTANT)) {
          sendTo("Shutting builder assistant down.\n\r");
          REMOVE_BIT(desc->prompt_d.type, PROMPT_BUILDER_ASSISTANT);
        } else {
          sendTo("Builder assistant activated.\n\r");
          SET_BIT(desc->prompt_d.type, PROMPT_BUILDER_ASSISTANT);
        }
      break;
    case 16:
      if (IS_SET(desc->prompt_d.type, PROMPT_EXPTONEXT_LEVEL)) {
        sendTo("Removing Exp to next level from prompt.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_EXPTONEXT_LEVEL);
      } else {
        sendTo("Adding Exp to next level to prompt.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_EXPTONEXT_LEVEL);
      }
      break;
    case 17:
      if (ansi() || vt100())
        if (IS_SET(desc->prompt_d.type, PROMPT_VTANSI_BAR)) {
          REMOVE_BIT(desc->prompt_d.type, PROMPT_VTANSI_BAR);
          cls();
          fullscreen();
          cls();
          sendTo("Shutting down the ansi/vt100 information bar.\n\r");
        } else {
          SET_BIT(desc->prompt_d.type, PROMPT_VTANSI_BAR);
          cls();
          sendTo(fmt(VT_MARGSET) % 1 % (getScreen() - 3));
          doCls(false);
          sendTo("Initilizing ansi/vt100 information bar.\n\r");
        }
        break;
    case 19:
      if (IS_SET(desc->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
        sendTo("Disabling client style code sends.\n\r");
        REMOVE_BIT(desc->prompt_d.type, PROMPT_CLIENT_PROMPT);
      } else {
        sendTo("Enabling client style code sends.\n\r");
        SET_BIT(desc->prompt_d.type, PROMPT_CLIENT_PROMPT);

        if (desc) {
	  Descriptor *d;
	  char buf[256] = "\0";

          desc->send_client_prompt(TRUE, 16383);

	  for (d = descriptor_list; d; d = d->next) {
	    if (d->character) {
	      if (d->character->isLinkdead() && isImmortal())
		sprintf(buf, "[%s]", d->character->getName());
	      else
		strcpy(buf, (d->character->getName() ? d->character->getName() : "UNKNOWN NAME"));

	      if (canSeeWho(d->character)) {
	        desc->prompt_mode = -1;
	        desc->clientf(fmt("%d|%s|%d|0") % CLIENT_WHO % buf % DELETE);
	        desc->clientf(fmt("%d|%s|%d|0") % CLIENT_WHO % d->character->getName() % DELETE);

	        if (d->character->isPlayerAction(PLR_ANONYMOUS) && !isImmortal())
	          desc->clientf(fmt("%d|%s|%d|0|1") % CLIENT_WHO % buf % ADD);
	        else
	          desc->clientf(fmt("%d|%s|%d|%d|1") % CLIENT_WHO % buf % ADD % d->character->GetMaxLevel());
	      }
	    }
	  }
        }
      }
      break;
    case 20:
      if ((ansi() || vt100()) && IS_SET(desc->prompt_d.type, PROMPT_VTANSI_BAR))
        if (IS_SET(desc->prompt_d.type, PROMPT_CLASSIC_ANSIBAR)) {
          REMOVE_BIT(desc->prompt_d.type, PROMPT_CLASSIC_ANSIBAR);
          cls();
          sendTo(fmt(VT_MARGSET) % 1 % (getScreen() - 3));
          doCls(false);
          sendTo("Changing to the modern vt100/ansi bar.\n\r");
        } else {
          SET_BIT(desc->prompt_d.type, PROMPT_CLASSIC_ANSIBAR);
          cls();
          sendTo(fmt(VT_MARGSET) % 1 % (getScreen() - 3));
          doCls(false);
          sendTo("Changing to the classic vt100/ansi bar.\n\r");
        }
      break;
    default:
      sendTo("Invalid argument(s) to prompt command. See help file for PROMPT for more information.\n\r");
      break;
  }
}

void TBeing::doCls(bool tell)
{
  char buf[600] = "\0";

  if (!isPc())
    return;

  if (desc && !desc->connected) {
    if (!isPlayerAction(PLR_VT100 | PLR_ANSI)) {
      if (tell)
        sendTo("Your terminal options do not indicate you can receive the CLS sequence.\n\rChange to VT100 or Ansi if you can.\n\r");
      return;
    }
    if (tell || IS_SET(desc->prompt_d.type, PROMPT_VTANSI_BAR))
      cls();

    if (!IS_SET(desc->prompt_d.type, PROMPT_VTANSI_BAR))
      return;

    sendTo(fmt(VT_MARGSET) % 1 % (getScreen() - 4));
    sprintf(buf + strlen(buf), VT_CURSPOS, getScreen() - 3, 1);
    sprintf(buf + strlen(buf), "_____________________________________________________________________________");
    sprintf(buf + strlen(buf), VT_CURSPOS, getScreen() - 2, 1);

    if (IS_SET(desc->prompt_d.type, PROMPT_CLASSIC_ANSIBAR)) {
      if (hasClass(CLASS_CLERIC) || hasClass(CLASS_DEIKHAN))
        sprintf(buf + strlen(buf), "Hits:                     Piety:                      Moves:                 ");
      else if (hasClass(CLASS_SHAMAN))
        sprintf(buf + strlen(buf), "Hits:                 Lifeforce:                      Moves:                 ");
      else
        sprintf(buf + strlen(buf), "Hits:                      Mana:                      Moves:                 ");

      sprintf(buf + strlen(buf), VT_CURSPOS, getScreen() -1 , 1);

      if (!isImmortal()) {
#if FACTIONS_IN_USE
	sprintf(buf + strlen(buf), "Aff%%:                    Talens:                    Exp:                     ");
#else
        sprintf(buf + strlen(buf), "                         Talens:                    Exp:                     ");
#endif
      } else
        sprintf(buf + strlen(buf), "Room:                    Talens:                    Exp:                     ");

      sprintf(buf + strlen(buf), VT_CURSPOS, getScreen(), 1);
    } else {
      if (hasClass(CLASS_CLERIC) || hasClass(CLASS_DEIKHAN))
        sprintf(buf + strlen(buf), "Hits:                     Piety:                      Moves:                 ");
      else if (hasClass(CLASS_SHAMAN))
        sprintf(buf + strlen(buf), "Hits:                 Lifeforce:                      Moves:                 ");
      else
        sprintf(buf + strlen(buf), "Hits:                      Mana:                      Moves:                 ");

      sprintf(buf + strlen(buf), VT_CURSPOS, getScreen() -1 , 1);

      if (!isImmortal()) {
#if FACTIONS_IN_USE
        sprintf(buf + strlen(buf), "                            Exp:                      Aff %%:                 ");
#else
        sprintf(buf + strlen(buf), "                            Exp:                                             ");
#endif
      } else
        sprintf(buf + strlen(buf), "                            Exp:                       Room:                 ");

      sprintf(buf + strlen(buf), VT_CURSPOS, getScreen(), 1);
      sprintf(buf + strlen(buf), "                            TNL:                     Talens:                 ");
    }

    sendTo(buf);
    sendTo(fmt(VT_CURSPOS) % 1 % 1);

    if (vt100()) {
      desc->updateScreenVt100(2*CHANGED_PIETY - 1);
    } else if (ansi()) 
      desc->updateScreenAnsi(2*CHANGED_PIETY - 1);

    sendTo(fmt("%s") % norm());
  } else if (tell || !desc || IS_SET(desc->prompt_d.type, PROMPT_VTANSI_BAR))
    cls();
}

void TBeing::doColor(const char *)
{
  sendTo("Mobs can't have color.\n\r");
}

void TPerson::doColor(const char *buf)
{
  char arg[160], arg2[160];
  char  tempBuf[20];
  unsigned int i;
  int toggle = TRUE;

  if (!desc) {
    vlogf(LOG_BUG, fmt("Something without a desc (%s) is trying to set a colorLevel") %  getName());
    return;
  }
  if (!isPc()) {
    sendTo("Mobs can't have color.\n\r");
    return;
  } 
  buf = one_argument(buf, arg);
  strcpy(arg2, buf);

  if (!*arg) {
    for (i = 0;i < (MAX_PLR_COLOR - 1);i++) {
      if (*color_options[i]) {
        if (isImmortal() || (!(i == PLR_COLOR_CODES) && !(i == PLR_COLOR_LOGS))) {  
          sendTo(fmt("%-45s : %s\n\r") % color_options[i] %
            ((IS_SET(desc->plr_color, (unsigned) (1<<i))) ? "on" : "off"));
        }
      }
    }
    sendTo(fmt("%-45s : %s\n\r") % "Color Substitute" % (desc->plr_colorSub ? "yes" : "no")); 
    sendTo(fmt("%-45s : %s\n\r") % "Color Replacements" % (desc->plr_colorOff ? "yes" : "no"));
    return;
  }
  if (is_abbrev(arg, "test")) {
    sendTo(COLOR_BASIC, "This is black       : <K>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is gray        : <k>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is white       : <w>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is bold_white  : <W>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is red         : <r>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is bold_red    : <R>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is green       : <g>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is bold_green  : <G>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is yellow      : <y>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is orange      : <o>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is blue        : <b>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is bold_blue   : <B>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is cyan        : <c>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is bold_cyan   : <C>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is purple      : <p>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is bold_purple : <P>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is bold        : <d>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is flash       : <f>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is inverse     : <i>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is red-2       : <a>test<1>\n\r");
    sendTo(COLOR_BASIC, "This is bold_red_2  : <A>test<1>\n\r");
    return;
  } else if (is_abbrev(arg, "enabled") || !strcasecmp(arg, "on")) {
    // one_argument ignores "on", so 2nd option doesn't work at all
    if (IS_SET(desc->plr_color, PLR_COLOR_BASIC)) {
      sendTo("Your basic color is already turned on.\n\r");
      
      if (!(isPlayerAction(PLR_COLOR))) 
        addPlayerAction(PLR_COLOR);
      
      if (!IS_SET(desc->prompt_d.type, PROMPT_COLOR)); 
        SET_BIT(desc->prompt_d.type, PROMPT_COLOR);
    } else {
      SET_BIT(desc->plr_color, PLR_COLOR_BASIC);
      sendTo(fmt("%sC%so%sl%so%sr%s mode enabled.\n\r") % ANSI_RED % ANSI_CYAN % ANSI_BLUE % ANSI_ORANGE % ANSI_PURPLE % ANSI_NORMAL);
      
      if (!(isPlayerAction(PLR_COLOR))) 
        addPlayerAction(PLR_COLOR);
      
      if (!IS_SET(desc->prompt_d.type, PROMPT_COLOR)); 
        SET_BIT(desc->prompt_d.type, PROMPT_COLOR);
    }
  } else if (is_abbrev(arg, "disabled") || is_abbrev(arg, "off")) {
    if (IS_SET(desc->plr_color, PLR_COLOR_BASIC)) {
      sendTo("You will no longer see color.\n\r");
      REMOVE_BIT(desc->plr_color, PLR_COLOR_BASIC);
      if (isPlayerAction(PLR_COLOR)) 
        remPlayerAction(PLR_COLOR);
      
      if (IS_SET(desc->prompt_d.type, PROMPT_COLOR)); 
        REMOVE_BIT(desc->prompt_d.type, PROMPT_COLOR);
    } else {
      sendTo("Your color is already turned off.\n\r");
      if (isPlayerAction(PLR_COLOR)) 
        remPlayerAction(PLR_COLOR);
      
      if (IS_SET(desc->prompt_d.type, PROMPT_COLOR)); 
        REMOVE_BIT(desc->prompt_d.type, PROMPT_COLOR);
    }
  } else if (is_abbrev(arg, "everything") || is_abbrev(arg, "all")) {
    if (!IS_SET(desc->plr_color, PLR_COLOR_BASIC)) {
      sendTo("You must first enable color before setting it to everything.\n\r");
      return;
    }
    addPlayerAction(PLR_COLOR);
    SET_BIT(desc->prompt_d.type, PROMPT_COLOR);
    SET_BIT(desc->plr_color, PLR_COLOR_ALL);
    SET_BIT(desc->plr_color, PLR_COLOR_BASIC);
    SET_BIT(desc->plr_color, PLR_COLOR_COMM);
    SET_BIT(desc->plr_color, PLR_COLOR_OBJECTS);
    SET_BIT(desc->plr_color, PLR_COLOR_MOBS);
    SET_BIT(desc->plr_color, PLR_COLOR_ROOMS);
    SET_BIT(desc->plr_color, PLR_COLOR_ROOM_NAME);
    SET_BIT(desc->plr_color, PLR_COLOR_SHOUTS);
    SET_BIT(desc->plr_color, PLR_COLOR_SPELLS);

    // codes works in reverse
    REMOVE_BIT(desc->plr_color, PLR_COLOR_CODES);

    if (hasWizPower(POWER_COLOR_LOGS))
      SET_BIT(desc->plr_color, PLR_COLOR_LOGS);

    act("Your color has been set to all available color.", FALSE, this, NULL, NULL, TO_CHAR);
  } else if (is_abbrev(arg, "none")) {
    remPlayerAction(PLR_COLOR);
    REMOVE_BIT(desc->prompt_d.type, PROMPT_COLOR);
    REMOVE_BIT(desc->plr_color, PLR_COLOR_ALL);
    REMOVE_BIT(desc->plr_color, PLR_COLOR_BASIC);
    REMOVE_BIT(desc->plr_color, PLR_COLOR_COMM);
    REMOVE_BIT(desc->plr_color, PLR_COLOR_OBJECTS);
    REMOVE_BIT(desc->plr_color, PLR_COLOR_MOBS);
    REMOVE_BIT(desc->plr_color, PLR_COLOR_ROOMS);
    REMOVE_BIT(desc->plr_color, PLR_COLOR_ROOM_NAME);
    REMOVE_BIT(desc->plr_color, PLR_COLOR_SHOUTS);
    REMOVE_BIT(desc->plr_color, PLR_COLOR_SPELLS);

    act("All color options have been toggled off.", FALSE, this, NULL, NULL, TO_CHAR);
  } else if (is_abbrev(arg, "communications")) {
    if (IS_SET(desc->plr_color, PLR_COLOR_COMM)) {
      sendTo("You will no longer see colored communications.\n\r");
      REMOVE_BIT(desc->plr_color, PLR_COLOR_COMM);
    } else {
      if (!IS_SET(desc->plr_color, PLR_COLOR_BASIC)) {
        sendTo("Your basic color must be enabled to add advanced color.\n\r");
        return;
      }
      sendTo("You will now see colored communications.\n\r");
      SET_BIT(desc->plr_color, PLR_COLOR_COMM);
    }
  } else if (is_abbrev(arg, "codes")) {
    if (hasWizPower(POWER_COLOR_LOGS)) {
      if (IS_SET(desc->plr_color, PLR_COLOR_CODES)) {
        sendTo("You will no longer see color codes in things.\n\r");
        REMOVE_BIT(desc->plr_color, PLR_COLOR_CODES);
      } else {
        sendTo("You will now see color codes in things.\n\r");
        SET_BIT(desc->plr_color, PLR_COLOR_CODES);
      }
    } else {
      sendTo("You have to pick one or more supported color level(s).\n\r");
      sendTo("Syntax: color <enabled | disabled>\n\r");
      sendTo("Syntax: color <all | none | communications | objects | mobiles | rooms\n\r               room_name | shouts | spells | substitute | replace | test>\n\r");

    }
  } else if (is_abbrev(arg, "logs")) {
    if (isImmortal()) {
      if (IS_SET(desc->plr_color, PLR_COLOR_LOGS)) {
        sendTo("You will no longer see color in logs.\n\r");
        REMOVE_BIT(desc->plr_color, PLR_COLOR_LOGS);
      } else {
        if (!IS_SET(desc->plr_color, PLR_COLOR_BASIC)) {
          sendTo("Your basic color must be enabled to add advanced color.\n\r");
          return;
        } else {
          sendTo("You will now see color in logs.\n\r");
          SET_BIT(desc->plr_color, PLR_COLOR_LOGS);
        }
      }
    } else {
      sendTo("You have to pick one or more supported color level(s).\n\r");
      sendTo("Syntax: color <enabled | disabled>\n\r");
      sendTo("Syntax: color <all | none | communicate | objects | mobiles | rooms\n\r               room_name | shouts | spells | substitute | replace | test>\n\r");

    }
  } else if (is_abbrev(arg, "objects")) {
    if (IS_SET(desc->plr_color, PLR_COLOR_OBJECTS)) {
      sendTo("You will no longer see color in objects.\n\r");
      REMOVE_BIT(desc->plr_color, PLR_COLOR_OBJECTS);
    } else {
      if (!IS_SET(desc->plr_color, PLR_COLOR_BASIC)) {
        sendTo("Your basic color must be enabled to add advanced color.\n\r");
        return;
      }
      sendTo("You will now see color in objects.\n\r");
      SET_BIT(desc->plr_color, PLR_COLOR_OBJECTS);
    }
  } else if (is_abbrev(arg, "creatures") || is_abbrev(arg, "mobiles") ) {
    if (IS_SET(desc->plr_color, PLR_COLOR_MOBS)) {
      sendTo("You will no longer see color in creatures.\n\r");
      REMOVE_BIT(desc->plr_color, PLR_COLOR_MOBS);
    } else {
      if (!IS_SET(desc->plr_color, PLR_COLOR_BASIC)) {
        sendTo("Your basic color must be turned enabled to add advanced color.\n\r");
        return;
      }
      sendTo("You will now see color in creatures.\n\r");
      SET_BIT(desc->plr_color, PLR_COLOR_MOBS);
    }
  } else if (is_abbrev(arg, "rooms")) {
    if (IS_SET(desc->plr_color, PLR_COLOR_ROOMS)) {
      sendTo("You will no longer see color in room text.\n\r");
      REMOVE_BIT(desc->plr_color, PLR_COLOR_ROOMS);
    } else {
      if (!IS_SET(desc->plr_color, PLR_COLOR_BASIC)) {
        sendTo("Your basic color must be enabled to add advanced color.\n\r");
        return;
      }
      sendTo("You will now see color in room text.\n\r");
      SET_BIT(desc->plr_color, PLR_COLOR_ROOMS);
    }
  } else if (is_abbrev(arg, "room_name")) {
    if (IS_SET(desc->plr_color, PLR_COLOR_ROOM_NAME)) {
      sendTo("You will no longer see different colors in different room names.\n\r");
      sendTo("You will still have them in a color unless you turn all color off.\n\r");
      REMOVE_BIT(desc->plr_color, PLR_COLOR_ROOM_NAME);
    } else {
      if (!IS_SET(desc->plr_color, PLR_COLOR_BASIC)) {
        sendTo("Your basic color must be enabled to add advanced color.\n\r");
        return;
      }
      sendTo("You will now see different colors in different rooms.\n\r");
      sendTo("If you want all of them in only one color, turn this off but use basic color.\n\r");
      SET_BIT(desc->plr_color, PLR_COLOR_ROOM_NAME);
    }
  } else if (is_abbrev(arg, "shouts")) {
    if (IS_SET(desc->plr_color, PLR_COLOR_SHOUTS)) {
      sendTo("You will no longer see color in shouts.\n\r");
      REMOVE_BIT(desc->plr_color, PLR_COLOR_SHOUTS);
    } else {
      if (!IS_SET(desc->plr_color, PLR_COLOR_BASIC)) {
        sendTo("Your basic color must be enabled to add advanced color.\n\r");
        return;
      }
      sendTo("You will now see color in shouts.\n\r");
      SET_BIT(desc->plr_color, PLR_COLOR_SHOUTS);
    }
  } else if (is_abbrev(arg, "spells")) {
    if (IS_SET(desc->plr_color, PLR_COLOR_SPELLS)) {
      sendTo("You will no longer see extra color in spells.\n\r");
      REMOVE_BIT(desc->plr_color, PLR_COLOR_SPELLS);
    } else {
      if (!IS_SET(desc->plr_color, PLR_COLOR_BASIC)) {
        sendTo("Your basic color must be enabled to add advanced color.\n\r");
        return;
      }
      sendTo("You will now see extra color in spells.\n\r");
      SET_BIT(desc->plr_color, PLR_COLOR_SPELLS);
    }
  } else if (is_abbrev(arg, "replace")) {
    if (is_abbrev(arg2, "none")) {
      desc->plr_colorOff = 0;
      sendTo("You will not replace any mud colors with a substitute.\n\r");
      return;
    } else if (is_abbrev(arg2, "black")) {
      sprintf(tempBuf, "%s", "black");
      if (IS_SET(desc->plr_colorOff, COLOR_BLACK)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_BLACK);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_BLACK) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_BLACK);
      }
    } else if (is_abbrev(arg2, "red")) {
      sprintf(tempBuf, "%s", "red");
      if (IS_SET(desc->plr_colorOff, COLOR_RED)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_RED);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_RED) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_RED);
      }
    } else if (is_abbrev(arg2, "green")) {
      sprintf(tempBuf, "%s", "green");
      if (IS_SET(desc->plr_colorOff, COLOR_GREEN)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_GREEN);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_GREEN) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_GREEN);
      }
    } else if (is_abbrev(arg2, "orange")) {
      sprintf(tempBuf, "%s", "orange");
      if (IS_SET(desc->plr_colorOff, COLOR_ORANGE)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_ORANGE);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_ORANGE) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_ORANGE);
      }
    } else if (is_abbrev(arg2, "blue")) {
      sprintf(tempBuf, "%s", "blue");
      if (IS_SET(desc->plr_colorOff, COLOR_BLUE)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_BLUE);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_BLUE) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_BLUE);
      }
    } else if (is_abbrev(arg2, "purple")) {
      sprintf(tempBuf, "%s", "purple");
      if (IS_SET(desc->plr_colorOff, COLOR_PURPLE)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_PURPLE);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_PURPLE) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_PURPLE);
      }
    } else if (is_abbrev(arg2, "cyan")) {
      sprintf(tempBuf, "%s", "cyan");
      if (IS_SET(desc->plr_colorOff, COLOR_CYAN)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_CYAN);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_CYAN) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_CYAN);
      }
    } else if (is_abbrev(arg2, "white")) {
      sprintf(tempBuf, "%s", "white");
      if (IS_SET(desc->plr_colorOff, COLOR_WHITE)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_WHITE);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_WHITE) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_WHITE);
      }
    } else if (is_abbrev(arg2, "yellow")) {
      sprintf(tempBuf, "%s", "yellow");
      if (IS_SET(desc->plr_colorOff, COLOR_YELLOW)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_YELLOW);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_YELLOW) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_YELLOW);
      }
    } else if (is_abbrev(arg2, "gray")) {
      sprintf(tempBuf, "%s", "gray");
      if (IS_SET(desc->plr_colorOff, COLOR_GRAY)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_GRAY);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_GRAY) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_GRAY);
      }
    } else if (is_abbrev(arg2, "underline")) {
      sprintf(tempBuf, "%s", "underline");
      if (IS_SET(desc->plr_colorOff, COLOR_UNDER)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_UNDER);
        toggle = FALSE;
      } else {
        SET_BIT(desc->plr_colorOff, COLOR_UNDER);
      }
#if 0
// not sure if bold will work/ may cause crashs without more code- cos
    } else if (is_abbrev(arg2, "bold")) {
      sprintf(tempBuf, "%s", "bold");
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_BOLD);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_BOLD) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_BOLD);
      }
#endif
    } else if (is_abbrev(arg2, "bold_red")) {
      sprintf(tempBuf, "%s", "bold red");
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_RED)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_BOLD_RED);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_BOLD_RED) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_BOLD_RED);
      }
    } else if (is_abbrev(arg2, "bold_green")) {
      sprintf(tempBuf, "%s", "bold green");
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_GREEN)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_BOLD_GREEN);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_BOLD_GREEN) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_BOLD_GREEN);
      }
    } else if (is_abbrev(arg2, "bold_blue")) {
      sprintf(tempBuf, "%s", "bold blue");
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_BLUE)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_BOLD_BLUE);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_BOLD_BLUE) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_BOLD_BLUE);
      }
    } else if (is_abbrev(arg2, "bold_purple")) {
      sprintf(tempBuf, "%s", "bold purple");
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_PURPLE)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_BOLD_PURPLE);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_BOLD_PURPLE) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_BOLD_PURPLE);
      }
    } else if (is_abbrev(arg2, "bold_cyan")) {
      sprintf(tempBuf, "%s", "bold cyan");
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_CYAN)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_BOLD_CYAN);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_BOLD_CYAN) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_BOLD_CYAN);
      }
    } else if (is_abbrev(arg2, "bold_white")) {
      sprintf(tempBuf, "%s", "bold white");
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_WHITE)) {
        REMOVE_BIT(desc->plr_colorOff, COLOR_BOLD_WHITE);
        toggle = FALSE;
      } else {
        if (desc->plr_colorSub == COLOR_SUB_BOLD_WHITE) {
          sendTo("You can not replace your color substitute.\n\r");
          return;
        }
        SET_BIT(desc->plr_colorOff, COLOR_BOLD_WHITE);
      }
    } else {
      sendTo("You have not chosen a valid color to replace.\n\r");
      sendTo("Syntax: color replace <color>\n\r");
      return;
    } 
    if (toggle) {
      sendTo(fmt("You have chosen to replace %s as a color with your substitute.\n\r") % tempBuf);
    } else {
      sendTo(fmt("You will no longer replace %s as a color.\n\r") % tempBuf);
    }
  } else if (is_abbrev(arg, "substitute.\n\r")) {
    if (is_abbrev(arg2, "none")) { 
      desc->plr_colorSub = COLOR_SUB_NONE;
      sprintf(tempBuf, "%s", "none");
    } else if (is_abbrev(arg2, "black")) {
      if (IS_SET(desc->plr_colorOff, COLOR_BLACK)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_BLACK;
      sprintf(tempBuf, "%s", "black");
    } else if (is_abbrev(arg2, "red")) {
      if (IS_SET(desc->plr_colorOff, COLOR_RED)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_RED;
      sprintf(tempBuf, "%s", "red");
    } else if (is_abbrev(arg2, "green")) {
      if (IS_SET(desc->plr_colorOff, COLOR_GREEN)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_GREEN;
      sprintf(tempBuf, "%s", "green");
    } else if (is_abbrev(arg2, "orange")) {
      if (IS_SET(desc->plr_colorOff, COLOR_ORANGE)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_ORANGE;
      sprintf(tempBuf, "%s", "orange");
    } else if (is_abbrev(arg2, "blue")) {
      if (IS_SET(desc->plr_colorOff, COLOR_BLUE)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_BLUE;
      sprintf(tempBuf, "%s", "blue");
    } else if (is_abbrev(arg2, "purple")) {
      if (IS_SET(desc->plr_colorOff, COLOR_PURPLE)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_PURPLE;
      sprintf(tempBuf, "%s", "purple");
    } else if (is_abbrev(arg2, "cyan")) {
      if (IS_SET(desc->plr_colorOff, COLOR_CYAN)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_CYAN;
      sprintf(tempBuf, "%s", "cyan");
    } else if (is_abbrev(arg2, "white")) {
      if (IS_SET(desc->plr_colorOff, COLOR_WHITE)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_WHITE;
      sprintf(tempBuf, "%s", "white");
    } else if (is_abbrev(arg2, "yellow")) {
      if (IS_SET(desc->plr_colorOff, COLOR_YELLOW)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_YELLOW;
      sprintf(tempBuf, "%s", "yellow");
    } else if (is_abbrev(arg2, "gray")) {
      if (IS_SET(desc->plr_colorOff, COLOR_GRAY)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_GRAY;
      sprintf(tempBuf, "%s", "gray");
    } else if (is_abbrev(arg2, "bold")) {
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_BOLD;
      sprintf(tempBuf, "%s", "bold");
    } else if (is_abbrev(arg2, "bold_red")) {
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_RED)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_BOLD_RED;
      sprintf(tempBuf, "%s", "bold red");
    } else if (is_abbrev(arg2, "bold_green")) {
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_GREEN)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_BOLD_GREEN;
      sprintf(tempBuf, "%s", "bold green");
    } else if (is_abbrev(arg2, "bold_blue")) {
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_BLUE)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_BOLD_BLUE;
      sprintf(tempBuf, "%s", "bold blue");
    } else if (is_abbrev(arg2, "bold_purple")) {
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_PURPLE)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_BOLD_PURPLE;
      sprintf(tempBuf, "%s", "bold purple");
    } else if (is_abbrev(arg2, "bold_cyan")) {
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_CYAN)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_BOLD_CYAN;
      sprintf(tempBuf, "%s", "bold cyan");
    } else if (is_abbrev(arg2, "bold_white")) {
      if (IS_SET(desc->plr_colorOff, COLOR_BOLD_WHITE)) {
        sendTo("You can not use a color you have replaced as a substitute.\n\r");
        return;
      }
      desc->plr_colorSub = COLOR_SUB_BOLD_WHITE;
      sprintf(tempBuf, "%s", "bold white");
    } else {
      sendTo("You have not chosen a valid color as a substitute.\n\r");
      sendTo("Syntax: color substitute <color>\n\r");
      return;
    }
    sendTo(fmt("You have chosen %s as your substitute color.\n\r") % tempBuf);
  } else {
    sendTo("You have to pick one or more supported color level(s).\n\r");
    sendTo("Syntax: color <enabled | disabled>\n\r");
    sendTo("Syntax: color <all | none | communicate | objects | mobiles | rooms\n\r               shouts | spells | substitute | replace | test>\n\r");
  }
}
