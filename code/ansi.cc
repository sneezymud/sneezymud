//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "ansi.cc" - Various color functions.
//
//      Last major revision : April 1997
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

void TBeing::setColor(setColorFieldT num, setColorKolorT col)
{
  if (!desc)
    return;
    
  if (col == SET_COL_KOL_OFF) 
    sendTo("Unsetting color....\n\r");
  else {
    sendTo("Setting color....\n\r");
    desc->prompt_d.type |= PROMPT_COLOR;
  }

  sstring buf;
  switch (col) {
    case SET_COL_KOL_OFF:
      buf = "";
      break;
    case SET_COL_KOL_BLUE:
      buf = ANSI_BLUE;
      break;
    case SET_COL_KOL_RED:
      buf = ANSI_RED;
      break;
    case SET_COL_KOL_GREEN:
      buf = ANSI_GREEN;
      break;
    case SET_COL_KOL_WHITE:
      buf = VT_BOLDTEX;
      break;
    case SET_COL_KOL_PURPLE:
      buf = ANSI_PURPLE;
      break;
    case SET_COL_KOL_CYAN:
      buf = ANSI_CYAN;
      break;
    case SET_COL_KOL_ORANGE:
      buf = ANSI_ORANGE;
      break;
    case SET_COL_KOL_YELLOW:
      buf = ANSI_YELLOW_BOLD;
      break;
    case SET_COL_KOL_CHARCOAL:
      buf = ANSI_GRAY;
      break;
    case SET_COL_KOL_WH_ON_BL:
      buf = ANSI_WH_ON_BL;
      break;
    case SET_COL_KOL_INVERT:
      buf = VT_INVERTT;
      break;
    case SET_COL_KOL_WH_ON_CY:
      buf = ANSI_WH_ON_CY;
      break;
    case SET_COL_KOL_WH_ON_RD:
      buf = ANSI_WH_ON_RD;
      break;
    case SET_COL_KOL_WH_ON_PR:
      buf = ANSI_WH_ON_PR;
      break;
    case SET_COL_KOL_WH_ON_GR:
      buf = ANSI_WH_ON_GR;
      break;
    case SET_COL_KOL_WH_ON_OR:
      buf = ANSI_WH_ON_OR;
      break;
    case SET_COL_KOL_BLINK:
      sendTo("Color not found.\n\r");
      buf = "";
      break;
    case SET_COL_KOL_BOLDRED:
      buf = ANSI_RED_BOLD;
      break;
    case SET_COL_KOL_BOLDGREEN:
      buf = ANSI_GREEN_BOLD;
      break;
    case SET_COL_KOL_BOLDBLUE:
      buf = ANSI_BLUE_BOLD;
      break;
    case SET_COL_KOL_BOLDPURPLE:
      buf = ANSI_PURPLE_BOLD;
      break;
    case SET_COL_KOL_BOLDCYAN:
      buf = ANSI_CYAN_BOLD;
      break;
  }

  switch (num) {
    case SET_COL_FIELD_HIT:
      strcpy(desc->prompt_d.hpColor, buf.c_str());
      break;
    case SET_COL_FIELD_MANA:
      strcpy(desc->prompt_d.manaColor, buf.c_str());
      break;
    case SET_COL_FIELD_MOVE:
      strcpy(desc->prompt_d.moveColor, buf.c_str());
      break;
    case SET_COL_FIELD_TALEN:
      strcpy(desc->prompt_d.moneyColor, buf.c_str());
      break;
    case SET_COL_FIELD_XP:
      strcpy(desc->prompt_d.expColor, buf.c_str());
      break;
    case SET_COL_FIELD_OPP:
      strcpy(desc->prompt_d.oppColor, buf.c_str());
      break;
    case SET_COL_FIELD_ROOM:
      strcpy(desc->prompt_d.roomColor, buf.c_str());
      break;
    case SET_COL_FIELD_TANK:
      strcpy(desc->prompt_d.tankColor, buf.c_str());
      break;
    case SET_COL_FIELD_TANK_OTHER:
      strcpy(desc->prompt_d.tankColor, buf.c_str());
      break;
    case SET_COL_FIELD_PIETY:
      strcpy(desc->prompt_d.pietyColor, buf.c_str());
      break;
    case SET_COL_FIELD_LIFEFORCE:
      strcpy(desc->prompt_d.lifeforceColor, buf.c_str());
      break;
  }
}

bool TBeing::hasColor() const
{
  return (color() || ansi());
}

bool TBeing::hasColorVt() const
{
  return (color() || ansi() || vt100());
}

const sstring TBeing::ansi_color_bold(const char *s) const
{
  sstring buf;

  if (hasColor()) {
    buf = bold();
    buf += s;
  } else if (hasColorVt()) {
      return "";
//    return VT_BOLDTEX;
  } else {
    s = "";
    return s;
  }
  return buf;
}

const sstring TBeing::doColorSub() const
{
  char buf[80];
      if (hasColor()) {
        switch (desc->plr_colorSub) {
          case COLOR_SUB_NONE:
            return "";
            break;
          case COLOR_SUB_BLACK:
            strcpy(buf, black());
            break;
          case COLOR_SUB_RED:
            strcpy(buf, red());
            break;
          case COLOR_SUB_GREEN:
            strcpy(buf, green());
            break;
          case COLOR_SUB_ORANGE:
            strcpy(buf, orange());
            break;
          case COLOR_SUB_BLUE:
            strcpy(buf, blue());
            break;
          case COLOR_SUB_PURPLE:
            strcpy(buf, purple());
            break;
          case COLOR_SUB_CYAN:
            strcpy(buf, cyan());
            break;
          case COLOR_SUB_WHITE:
            strcpy(buf, white());
            break;
          case COLOR_SUB_YELLOW:
            strcpy(buf, orangeBold());
            break;
          case COLOR_SUB_GRAY:
            strcpy(buf, blackBold());
            break;
          case COLOR_SUB_BOLD_RED:
            strcpy(buf, redBold());
            break;
          case COLOR_SUB_BOLD_GREEN:
            strcpy(buf, greenBold());
            break;
          case COLOR_SUB_BOLD_BLUE:
            strcpy(buf, blueBold());
            break;
          case COLOR_SUB_BOLD_PURPLE:
            strcpy(buf, purpleBold());
            break;
          case COLOR_SUB_BOLD_CYAN:
            strcpy(buf, cyanBold());
            break;
          case COLOR_SUB_BOLD_WHITE:
            strcpy(buf, whiteBold());
            break;
          case COLOR_SUB_BOLD:
            strcpy(buf, bold());
            break;
          default:
            return "";
            vlogf(LOG_BUG, fmt("Problem in color substituting (%s)") %  getName());
            break;
        }
        return buf;
      } else if (hasColorVt()) {
        strcpy(buf, "");
      } else {
        strcpy(buf,"");
      }
      return buf;
}

const sstring TBeing::ansi_color_bold(const char *s, unsigned int ans_color) const
{
  sstring buf;
  int repFound = FALSE;


  if (desc) {
    if (IS_SET(desc->plr_colorOff, ans_color)) {
      repFound = TRUE;
    }
  }

  if (repFound) {
    if (desc->plr_colorSub) {
      return doColorSub();
    } else {
      s = "";
      return s;
    }
  } else {
    if (hasColor()) {
      buf = bold();
      buf += s;
    } else if (hasColorVt()) {
      if (ans_color == COLOR_BOLD) {
        return VT_BOLDTEX;
      } else {
        return "";
      }
    } else {
      s = "";
      return s;
    }
  }
  return buf;
}

const sstring TBeing::ansi_color(const char *s) const
{
  if (hasColor()) {
    return s;
  } else if (hasColorVt()) {
    return "";
  } else {
    return "";
  }
  return "";
//  return (hasColor() ? s : (hasColorVt() ? VT_BOLDTEX : ""));
}

const sstring TBeing::ansi_color(const char *s, unsigned int ans_color) const
{
  int repFound = FALSE;

  if (desc) {
    if (IS_SET(desc->plr_colorOff, ans_color)) {
      repFound = TRUE;
    }
  }

  if (repFound) {
    if (desc->plr_colorSub) {
      return doColorSub();
    } else {
      s = "";
      return s;
    }
  } else {
    if (hasColor()) {
      return s;
    } else if (hasColorVt()) {
      if (ans_color == COLOR_BOLD) {
        return VT_BOLDTEX;
      } else if (ans_color == COLOR_NONE) {
        return ANSI_NORMAL;
      } else {
        return "";
      }
    } else {
      return "";
    }
//    return (hasColor() ? s : (hasColorVt() ? VT_BOLDTEX : ""));
  }
}

const char *TBeing::highlight(char *s) const
{
  return (hasColorVt() ? s : "");
}

const char *TBeing::whiteBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_WHITE, COLOR_BOLD_WHITE).c_str());
  return buf;
}

const char *TBeing::white() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WHITE, COLOR_WHITE).c_str());
  return buf;
}

const char *TBeing::blackBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_BLACK, COLOR_GRAY).c_str());
  return buf;
}

const char *TBeing::black() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_BLACK, COLOR_BLACK).c_str());
  return buf;
}

const char *TBeing::redBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_RED, COLOR_BOLD_RED).c_str());
  return buf;
}

const char *TBeing::red() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_RED, COLOR_RED).c_str());
  return buf;
}

const char *TBeing::underBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_UNDER, COLOR_UNDER).c_str());
  return buf;
}

const char *TBeing::under() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_UNDER, COLOR_UNDER).c_str());
  return buf;
}

const char *TBeing::bold() const
{
  static char buf[256];
  strcpy(buf, ansi_color(VT_BOLDTEX, COLOR_BOLD).c_str());
  return buf;
}

const char *TBeing::norm() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_NORMAL, COLOR_NONE).c_str());
  return buf;
}

const char *TBeing::blueBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_BLUE, COLOR_BOLD_BLUE).c_str());
  return buf;
}

const char *TBeing::blue() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_BLUE, COLOR_BLUE).c_str());
  return buf;
}

const char *TBeing::cyanBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_CYAN, COLOR_BOLD_CYAN).c_str());
  return buf;
}

const char *TBeing::cyan() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_CYAN, COLOR_CYAN).c_str());
  return buf;
}

const char *TBeing::greenBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_GREEN, COLOR_BOLD_GREEN).c_str());
  return buf;
}

const char *TBeing::green() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_GREEN, COLOR_GREEN).c_str());
  return buf;
}

const char *TBeing::orangeBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_ORANGE, COLOR_YELLOW).c_str());
  return buf;
}

const char *TBeing::orange() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_ORANGE, COLOR_ORANGE).c_str());
  return buf;
}

const char *TBeing::purpleBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_PURPLE, COLOR_BOLD_PURPLE).c_str());
  return buf;
}

const char *TBeing::purple() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_PURPLE, COLOR_PURPLE).c_str());
  return buf;
}

const char *TBeing::invert() const
{
  static char buf[256];
  strcpy(buf, ansi_color(VT_INVERTT).c_str());
  return buf;
}

const char *TBeing::flash() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_FLASH).c_str());
  return buf;
}

const char *TBeing::BlackOnBlack() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_BK_ON_BK).c_str());
  return buf;
}

const char *TBeing::BlackOnWhite() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_BK_ON_WH).c_str());
  return buf;
}

const char *TBeing::WhiteOnBlue() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_BL).c_str());
  return buf;
}

const char *TBeing::WhiteOnCyan() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_CY).c_str());
  return buf;
}

const char *TBeing::WhiteOnGreen() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_GR).c_str());
  return buf;
}

const char *TBeing::WhiteOnOrange() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_OR).c_str());
  return buf;
}

const char *TBeing::WhiteOnPurple() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_PR).c_str());
  return buf;
}

const char *TBeing::WhiteOnRed() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_RD).c_str());
  return buf;
}

bool Descriptor::hasColor() const
{
  return ((plr_act & PLR_COLOR) || (plr_act & PLR_ANSI));
}

bool Descriptor::hasColorVt() const
{
  return ((plr_act & PLR_COLOR) || (plr_act & PLR_ANSI) || (plr_act & PLR_VT100));
}


const sstring Descriptor::doColorSub() const
{
  char buf[80];
      if (hasColor()) {
        switch (plr_colorSub) {
          case COLOR_SUB_NONE:
            return "";
            break;
          case COLOR_SUB_BLACK:
            strcpy(buf, black());
            break;
          case COLOR_SUB_RED:
            strcpy(buf, red());
            break;
          case COLOR_SUB_GREEN:
            strcpy(buf, green());
            break;
          case COLOR_SUB_ORANGE:
            strcpy(buf, orange());
            break;
          case COLOR_SUB_BLUE:
            strcpy(buf, blue());
            break;
          case COLOR_SUB_PURPLE:
            strcpy(buf, purple());
            break;
          case COLOR_SUB_CYAN:
            strcpy(buf, cyan());
            break;
          case COLOR_SUB_WHITE:
            strcpy(buf, white());
            break;
          case COLOR_SUB_YELLOW:
            strcpy(buf, orangeBold());
            break;
          case COLOR_SUB_GRAY:
            strcpy(buf, blackBold());
            break;
          case COLOR_SUB_BOLD_RED:
            strcpy(buf, redBold());
            break;
          case COLOR_SUB_BOLD_GREEN:
            strcpy(buf, greenBold());
            break;
          case COLOR_SUB_BOLD_BLUE:
            strcpy(buf, blueBold());
            break;
          case COLOR_SUB_BOLD_PURPLE:
            strcpy(buf, purpleBold());
            break;
          case COLOR_SUB_BOLD_CYAN:
            strcpy(buf, cyanBold());
            break;
          case COLOR_SUB_BOLD_WHITE:
            strcpy(buf, whiteBold());
            break;
          case COLOR_SUB_BOLD:
            strcpy(buf, bold());
            break;
          default:
            return "";
            vlogf(LOG_BUG, "Problem in color substituting/desc");
            break;
        }
        return buf;
      } else if (hasColorVt()) {
        strcpy(buf, "");
      } else {
        strcpy(buf,"");
      }
      return buf;
}


const sstring Descriptor::ansi_color_bold(const char *s) const
{
  sstring buf;

  if (hasColor()) {
    buf = bold();
    buf += s;
  } else if (hasColorVt()) 
    return "";
  else 
    return"";

  return buf;
}

const sstring Descriptor::ansi_color_bold(const char *s, unsigned int color) const
{
  sstring buf;
  int repFound = FALSE;

  if (IS_SET(plr_colorOff, color)) 
    repFound = TRUE;

  if (repFound) {
    if (plr_colorSub) 
      return doColorSub();
    else {
      s = "";
      return s;
    }
  } else {
    if (hasColor()) {
      buf = bold();
      buf += s;
    } else if (hasColorVt()) {
      if (color == COLOR_BOLD) 
        return VT_BOLDTEX;
      else {
        s = "";
        return s;
      }
    } else {
      s = "";
      return s;
    }
  }
  return buf;
}

const sstring Descriptor::ansi_color(const char *s) const
{
  if (hasColor()) {
    return s;
  } else if (hasColorVt()) {
    return "";
//    return VT_BOLDTEX;
  } else {
    return "";
  }
//  return (hasColor() ? s : (hasColorVt() ? VT_BOLDTEX : ""));
}

const sstring Descriptor::ansi_color(const char *s, unsigned int color) const
{
  int repFound = FALSE;

  if (IS_SET(plr_colorOff, color)) {
    repFound = TRUE;
  }

  if (repFound) {
    if (plr_colorSub) {
      return doColorSub();
    } else {
      s = "";
      return s;
    }
  } else {
//    return (hasColor() ? s : (hasColorVt() ? VT_BOLDTEX : ""));
    if (hasColor()) {
      return s;
    } else if (hasColorVt()) {
      if (color == COLOR_BOLD) {
        return VT_BOLDTEX;
      } else if (color == COLOR_NONE) {
        return ANSI_NORMAL;
      } else {
        return "";
      }
    } else {
      return "";
    }
  }
}


const char *Descriptor::highlight(char *s) const
{
  return (hasColorVt() ? s : "");
}

const char *Descriptor::whiteBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_WHITE, COLOR_BOLD_WHITE).c_str());
  return buf;
}

const char *Descriptor::white() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WHITE, COLOR_WHITE).c_str());
  return buf;
}

const char *Descriptor::blackBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_BLACK, COLOR_GRAY).c_str());
  return buf;
}

const char *Descriptor::black() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_BLACK, COLOR_BLACK).c_str());
  return buf;
}

const char *Descriptor::redBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_RED, COLOR_BOLD_RED).c_str());
  return buf;
}

const char *Descriptor::red() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_RED, COLOR_RED).c_str());
  return buf;
}

const char *Descriptor::underBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_UNDER).c_str());
  return buf;
}

const char *Descriptor::under() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_UNDER).c_str());
  return buf;
}

const char *Descriptor::bold() const
{
  static char buf[256];
  strcpy(buf, ansi_color(VT_BOLDTEX, COLOR_BOLD).c_str());
  return buf;
}

const char *Descriptor::norm() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_NORMAL, COLOR_NONE).c_str());
  return buf;
}

const char *Descriptor::blueBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_BLUE, COLOR_BOLD_BLUE).c_str());
  return buf;
}

const char *Descriptor::blue() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_BLUE, COLOR_BLUE).c_str());
  return buf;
}

const char *Descriptor::cyanBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_CYAN, COLOR_BOLD_CYAN).c_str());
  return buf;
}

const char *Descriptor::cyan() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_CYAN, COLOR_CYAN).c_str());
  return buf;
}

const char *Descriptor::greenBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_GREEN, COLOR_BOLD_GREEN).c_str());
  return buf;
}

const char *Descriptor::green() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_GREEN, COLOR_GREEN).c_str());
  return buf;
}

const char *Descriptor::orangeBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_ORANGE, COLOR_YELLOW).c_str());
  return buf;
}

const char *Descriptor::orange() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_ORANGE, COLOR_ORANGE).c_str());
  return buf;
}

const char *Descriptor::purpleBold() const
{
  static char buf[256];
  strcpy(buf, ansi_color_bold(ANSI_PURPLE, COLOR_BOLD_PURPLE).c_str());
  return buf;
}

const char *Descriptor::purple() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_PURPLE, COLOR_PURPLE).c_str());
  return buf;
}

const char *Descriptor::invert() const
{
  static char buf[256];
  strcpy(buf, ansi_color(VT_INVERTT).c_str());
  return buf;
}

const char *Descriptor::flash() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_FLASH).c_str());
  return buf;
}

const char *Descriptor::BlackOnBlack() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_BK_ON_BK).c_str());
  return buf;
}

const char *Descriptor::BlackOnWhite() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_BK_ON_WH).c_str());
  return buf;
}

const char *Descriptor::WhiteOnBlue() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_BL).c_str());
  return buf;
}

const char *Descriptor::WhiteOnCyan() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_CY).c_str());
  return buf;
}

const char *Descriptor::WhiteOnGreen() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_GR).c_str());
  return buf;
}

const char *Descriptor::WhiteOnOrange() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_OR).c_str());
  return buf;
}

const char *Descriptor::WhiteOnPurple() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_PR).c_str());
  return buf;
}

const char *Descriptor::WhiteOnRed() const
{
  static char buf[256];
  strcpy(buf, ansi_color(ANSI_WH_ON_RD).c_str());
  return buf;
}

bool TBeing::color() const
{
  return isPlayerAction(PLR_COLOR);
}

bool TBeing::ansi() const
{
  return isPlayerAction(PLR_ANSI);
}

bool TBeing::vt100() const
{
  return isPlayerAction(PLR_VT100);
}

void TBeing::cls() const
{
  if ((ansi() || vt100()))
    sendTo(VT_HOMECLR);
}

void TBeing::fullscreen() const
{
  if ((ansi() || vt100()))
    sendTo(fmt(VT_MARGSET) % 1 % getScreen());
}

int TBeing::getScreen() const
{
   if (!desc)
     return 0;

   return (desc->screen_size ? desc->screen_size : 24);
}

