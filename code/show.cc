//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: show.cc,v $
// Revision 1.2  1999/09/17 17:38:45  peel
// Added show maxed, for listing maxed objects
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//                                                                      //
//    SneezyMUD++ - All rights reserved, SneezyMUD Coding Team      //
//                                                                      //
//    "show.cc" - Functions related to showing something
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <dirent.h>

#include "stdsneezy.h"
#include "disc_looting.h"
#include "combat.h"
#include "components.h"
#include "cmd_dissect.h"
#include "disc_alchemy.h"

void TThing::showMe(TBeing *ch) const
{
  ch->sendTo("You see nothing special.\n\r");
}

void TNote::showMe(TBeing *ch) const
{
  string sb;
  const char HEADER_TXT_NOTE[] = "There is something written upon it:\n\r\n\r";

  if (action_description) {
    sb += HEADER_TXT_NOTE;
    sb += action_description;
    if (ch->desc && !ch->desc->client) {
      if (ch->desc)
        ch->desc->page_string(sb.c_str(), 0, FALSE);
    } else {
      processStringForClient(sb);
      ch->desc->clientf("%d", CLIENT_NOTE);
      ch->sendTo("%s", sb.c_str());  // tmpbuf may have "%" in it, do it this way
      ch->desc->clientf("%d", CLIENT_NOTE_END);
    }
  } else
    ch->sendTo("It's blank.\n\r");
  return;
}

//   Obviously, all this is really hokey for a procedure that gets used so
//   often.  This probably needs to get redone eventually.  I am happy to
//   say that none of this is my mess.  - SG 
void TObj::show_me_to_char(TBeing *ch, showModeT mode) const
{
  char buffer[10000];
  char buf[256];
  char capbuf[256];

  if (mode == SHOW_MODE_DESC_PLUS && getDescr()) {
    if (roomp && roomp->isWaterSector()) {
      sprintf(buffer, "%s is floating here.", getName());
      cap(buffer);
    } else {
      sprintf(capbuf, "%s", addNameToBuf(ch, ch->desc, this, getDescr(), COLOR_OBJECTS).c_str());
      string cStrbuf = capbuf;
      while (cStrbuf.find("$$g") != string::npos)
        cStrbuf.replace(cStrbuf.find("$$g"), 3,
                        (roomp ? roomp->describeGround().c_str() : "TELL A GOD"));
      while (cStrbuf.find("$g") != string::npos)
        cStrbuf.replace(cStrbuf.find("$g"), 2,
                        (roomp ? roomp->describeGround().c_str() : "TELL A GOD"));
      strcpy(capbuf, cStrbuf.c_str());
      cap(capbuf);
      strcpy(buffer, colorString(ch, ch->desc, capbuf, NULL, COLOR_OBJECTS, TRUE).c_str());
    }
  } else if ((mode == SHOW_MODE_SHORT_PLUS || 
              mode == SHOW_MODE_SHORT_PLUS_INV ||
              mode == SHOW_MODE_SHORT) && getName()) {
    strcpy(buffer, getName());
    cap(buffer);
  } else if (mode == SHOW_MODE_TYPE) {
    showMe(ch);
    return;
  } else 
    buffer[0] = '\0';// we need this before we start doing strcat
  
  // this is an item-type-specific modifier
  strcat(buffer, showModifier(mode, ch).c_str());

  if (isObjStat(ITEM_INVISIBLE))
    strcat(buffer, " (invisible)");
  if (isObjStat(ITEM_MAGIC) && ch->isAffected(AFF_DETECT_MAGIC))
    sprintf(buffer + strlen(buffer), " %s(blue aura)%s", ch->cyan(), ch->norm());
  if (isObjStat(ITEM_GLOW))
    sprintf(buffer + strlen(buffer), " %s(glowing)%s", ch->orange(), ch->norm());
  if (isObjStat(ITEM_SHADOWY))
    strcat(buffer, " (shadowy)");
  if (isObjStat(ITEM_HOVER))
    strcat(buffer, " (hovering)");
  if (isObjStat(ITEM_HUM))
    strcat(buffer, " (humming)");
  if (isObjStat(ITEM_ATTACHED))
    strcat(buffer, " (attached)");
  if (isObjStat(ITEM_BURNING))
    sprintf(buffer + strlen(buffer), " %s(burning)%s", ch->red(), ch->norm());
  if (isObjStat(ITEM_CHARRED))
    sprintf(buffer + strlen(buffer), " %s(charred)%s", ch->blackBold(), ch->norm());


  if (parent && dynamic_cast<TObj *>(parent)) {
    strcpy(capbuf, parent->getName());
    sprintf(buf, " (in %s)", uncap(capbuf));
    strcat(buffer, buf);
  }
  if (riding) {
    strcpy(capbuf, ch->objs(riding));
    sprintf(buf, " (on %s)", uncap(capbuf));
    strcat(buffer, buf);
  }
  if (dynamic_cast<const TTable *>(this) && rider) {
    strcat(buffer, "\n\r");
    ch->sendTo(buffer);
    list_thing_on_heap(rider, ch, false);
    return;
  }
  if (*buffer)
    strcat(buffer, "\n\r");
  sprintf(buf, "%s",colorString(ch, ch->desc, buffer, NULL, COLOR_OBJECTS, TRUE).c_str());
//  ch->sendTo(COLOR_OBJECTS, buffer);
//  COSMO_COLOR
  ch->sendTo(buf);
}

void TObj::show_me_mult_to_char(TBeing *ch, showModeT mode, unsigned int num) const
{
  char buffer[MAX_STRING_LENGTH];
  char tmp[80];
  char capbuf[256];

  buffer[0] = '\0';

  // uses page string (needs desc), so don't bother unless PC
  if (!ch->desc)
    return;

  if (mode == SHOW_MODE_DESC_PLUS && getDescr()) {
    sprintf(capbuf, "%s", addNameToBuf(ch, ch->desc, this, getDescr(), COLOR_OBJECTS).c_str());
    string cStrbuf = capbuf;
    while (cStrbuf.find("$$g") != string::npos)
      cStrbuf.replace(cStrbuf.find("$$g"), 3, 
                      (roomp ? roomp->describeGround().c_str() : "TELL A GOD"));
    while (cStrbuf.find("$g") != string::npos)
      cStrbuf.replace(cStrbuf.find("$g"), 2,
                      (roomp ? roomp->describeGround().c_str() : "TELL A GOD"));
    strcpy(capbuf, cStrbuf.c_str());
    strcpy(buffer, capbuf);
  } else if (getName() && (mode == SHOW_MODE_SHORT_PLUS ||
          mode == SHOW_MODE_SHORT_PLUS_INV || mode == SHOW_MODE_SHORT)) {
    strcpy(buffer, getName());
  } else if (mode == SHOW_MODE_TYPE) {
    showMe(ch);
  }
  if (mode == SHOW_MODE_DESC_PLUS ||
      mode == SHOW_MODE_SHORT_PLUS ||
      mode == SHOW_MODE_SHORT_PLUS_INV ||
      mode == SHOW_MODE_TYPE ||
      mode == SHOW_MODE_PLUS) {
    strcat(buffer, showModifier(mode, ch).c_str());

    if (isObjStat(ITEM_INVISIBLE)) 
      strcat(buffer, " (invisible)");
    if (isObjStat(ITEM_MAGIC) && ch->isAffected(AFF_DETECT_MAGIC)) 
      sprintf(buffer + strlen(buffer), " %s(glowing blue)%s", ch->cyan(), ch->norm());
    
    if (isObjStat(ITEM_GLOW)) 
      sprintf(buffer + strlen(buffer), " %s(glowing)%s", ch->orange(), ch->norm());
    
    if (isObjStat(ITEM_SHADOWY))
      strcat(buffer, " (shadowy)");
    if (isObjStat(ITEM_HOVER))
      strcat(buffer, " (hovering)");

    if (isObjStat(ITEM_HUM)) 
      strcat(buffer, " (humming)");
    if (isObjStat(ITEM_ATTACHED))
      strcat(buffer, " (attached)");
    if (isObjStat(ITEM_BURNING))
      sprintf(buffer + strlen(buffer), " %s(burning)%s", ch->red(), ch->norm());
    if (isObjStat(ITEM_CHARRED))
      sprintf(buffer + strlen(buffer), " %s(charred)%s", ch->blackBold(), ch->norm());
    
  }
  if (num > 1) {
    sprintf(tmp, " [%d]", num);
    strcat(buffer, tmp);
  }
  if (parent && dynamic_cast<TObj *>(parent)) {
    strcpy(capbuf, parent->getName());
    sprintf(tmp, " (in %s)", uncap(capbuf));
    strcat(buffer, tmp);
  }
  if (riding) {
    strcpy(capbuf, ch->objs(riding));
    sprintf(tmp, " (on %s)", uncap(capbuf));
    strcat(buffer, tmp);
  }
  strcat(buffer, "\n\r");
  cap(buffer);
  sprintf(buffer, "%s",colorString(ch, ch->desc, buffer, NULL, COLOR_OBJECTS, TRUE).c_str());
  ch->desc->page_string(buffer, 0);
}

void TTrap::listMeExcessive(TBeing *ch) const
{
  if ((getTrapCharges() > 0)) {
    if (ch->doesKnowSkill(SKILL_DETECT_TRAP)) {
      if (detectTrapObj(ch, this))
        ch->showTo(this, SHOW_MODE_DESC_PLUS);
    }
  } else {
    // used up trap
    ch->showTo(this, SHOW_MODE_DESC_PLUS);
  }
}

void TThing::listMeExcessive(TBeing *ch) const
{
  ch->showTo(this, SHOW_MODE_DESC_PLUS);
}

void TTrap::listMe(TBeing *ch, unsigned int num) const
{
  if ( getTrapCharges() > 0) {
    if (ch->doesKnowSkill(SKILL_DETECT_TRAP)) {
      if (detectTrapObj(ch, this)) {
        if (num > 1) 
          ch->showMultTo(this, SHOW_MODE_DESC_PLUS, num);
        else 
          ch->showTo(this, SHOW_MODE_DESC_PLUS);
      }
    }
  } else {
    // used up traps
    if (num > 1) 
      ch->showMultTo(this, SHOW_MODE_DESC_PLUS, num);
    else 
      ch->showTo(this, SHOW_MODE_DESC_PLUS);
  }
}

void TThing::listMe(TBeing *ch, unsigned int num) const
{
  if (num > 1) 
    ch->showMultTo(this, SHOW_MODE_DESC_PLUS, num);
  else 
    ch->showTo(this, SHOW_MODE_DESC_PLUS);
}

// returns true if should not be listed
bool TBeing::listThingRoomMe(const TBeing *ch) const
{
  if (ch == this) {
    return true;
  } else if (rider) {
    // a horse
    // in general, don't display
    // if I can detect the horse, but not the rider, this would be bad
    if (!ch->isAffected(AFF_SENSE_LIFE) &&
        !ch->canSee(this, INFRA_YES) &&
        !ch->canSee(this, INFRA_NO)) {
      // horse is concealed
      return true;
    } else {
      TThing *t2;
      for (t2 = rider; t2; t2 = t2->nextRider) {
        // if one of the riders is vis, skip showing the horse
        if (ch == dynamic_cast<TBeing *>(t2) ||
            ch->isAffected(AFF_SENSE_LIFE) ||
            ch->canSee(t2, INFRA_YES) ||
            ch->canSee(t2)) {
          return true;
        }
      } 
    }
  } else if (!ch->isAffected(AFF_SENSE_LIFE) &&
      !ch->canSee(this, INFRA_YES) &&
      !ch->canSee(this)) {
    return true;
  }
  return false;
}

// returns true if should not be listed
bool TObj::listThingRoomMe(const TBeing *ch) const
{
  // list tables, but not chairs
  if (!ch->canSee(this) || dynamic_cast<TBeing *>(rider))
    return true;
  return false;
}

// returns true if should not be listed
bool TWindow::listThingRoomMe(const TBeing *ch) const
{
  return true;
}

// returns true if should not be listed
bool TThing::listThingRoomMe(const TBeing *ch) const
{
  if (!ch->canSee(this))
    return true;
  return false;
}

void list_thing_in_room(const TThing *list, TBeing *ch)
{
  const TThing *t, *cond_ptr[50];
  int k, cond_top = 0, cond_tot[50];

  for (t = list; t; t = t->nextThing) {
    if (t->listThingRoomMe(ch))
      continue;
    if (cond_top < 50 && !t->riding) {
      bool found = FALSE;
      for (k = 0; (k < cond_top && !found); k++) {
        if (dynamic_cast<const TObj *>(t) || dynamic_cast<const TMonster *>(t)) {
          if (cond_top > 0) {
            if (t->isSimilar(cond_ptr[k])) {
              cond_tot[k] += 1;
              found = TRUE;
            }
          }
        }
      }
      if (!found) {
        cond_ptr[cond_top] = t;
        cond_tot[cond_top] = 1;
        cond_top += 1;
      }
// these handle overflow
    } else {
      t->listMeExcessive(ch);
    }
  }
  if (cond_top) {
    for (k = 0; k < cond_top; k++) {
      cond_ptr[k]->listMe(ch, cond_tot[k]);
    }
  }
}

void list_in_heap(const TThing *list, TBeing *ch, bool show_all, int perc)
{
  const TThing *i;
  vector<const TThing *>cond_ptr(0);
  vector<unsigned int>cond_tot(0);
  unsigned int k;

  for (i = list; i; i = i->nextThing) {
    if (number(0,99) > perc)
      continue;
    if (ch->canSee(i)) {
      for (k = 0; k < cond_ptr.size(); k++) {
        if (i->isSimilar(cond_ptr[k])) {
          cond_tot[k] += 1;
          break;
        }
      }
      if (k >= cond_ptr.size()) {
        cond_ptr.push_back(i);
        cond_tot.push_back(1);
      }
    }
    if (show_all && i->stuff)
      list_in_heap(i->stuff, ch, true, 100);
  } // for loop

  if (cond_ptr.empty())
    ch->sendTo("Nothing.\n\r");

//  int Num_Inventory = 1;
  for (k = 0; k < cond_ptr.size(); k++) {
    if (cond_tot[k] > 1) {
//      Num_Inventory += cond_tot[k] - 1;
      ch->showMultTo(cond_ptr[k], SHOW_MODE_SHORT_PLUS_INV, cond_tot[k]);
    } else
      ch->showTo(cond_ptr[k], SHOW_MODE_SHORT_PLUS_INV);
  }
}

void list_thing_on_heap(const TThing *list, TBeing *ch, bool show_all)
{
  const TThing *i, *cond_ptr[50];
  int k, cond_top;
  unsigned int cond_tot[50];
  bool found = FALSE;

  int Num_Inventory = 1;
  cond_top = 0;

  for (i = list; i; i = i->nextRider) {
    if (ch->canSee(i)) {
      if (cond_top < 50) {
        found = FALSE;
        for (k = 0; (k < cond_top && !found); k++) {
          if (cond_top > 0) {
            if (i->isSimilar(cond_ptr[k])) {
              cond_tot[k] += 1;
              found = TRUE;
            }
          }
        }
        if (!found) {
          cond_ptr[cond_top] = i;
          cond_tot[cond_top] = 1;
          cond_top += 1;
        }
      } else
        ch->showTo(i, SHOW_MODE_SHORT_PLUS_INV);
    }
    if (show_all)
      list_thing_on_heap(i->rider, ch, true);
  } // for loop
  if (cond_top) {
    for (k = 0; k < cond_top; k++) {
      if (cond_tot[k] > 1) {
        Num_Inventory += cond_tot[k] - 1;
        ch->showMultTo(cond_ptr[k], SHOW_MODE_SHORT_PLUS_INV, cond_tot[k]);
      } else
        ch->showTo(cond_ptr[k], SHOW_MODE_SHORT_PLUS_INV);
    }
  }
}

extern string describeDuration(const TBeing *, int);

static string displayShowApprox(const TBeing *looked, const TBeing *looker, spellNumT tSkill, float tDiff)
{
  // This function is still experimental.  Don't use it in the main world yet.
  if (strcmp(looker->getName(), "Lapsos") != 0 || !looker->isImmortal())
    return ("");

  if (!looker->doesKnowSkill(tSkill))
    return ("");

  affectedData *tAff;
  char          tBuf[256];

  // -49, ..., 49
  float tDeffer = (float) (looked->GetMaxLevel() - looker->GetMaxLevel());

  // (-1, ..., 1), ..., (-49, ..., 49)
  if (tDeffer > 0)
    tDeffer = (float) ::number((int) tDeffer, (int) -(tDeffer));
  else
    tDeffer = 0;

  // Assuming 2.0 tDiff:
  // 0, (-6, ..., 6), ..., (-102, ..., 102), ..., 100
  tDeffer = max((float) 100.0, min((float) 0.0, (tDiff * (tDeffer + ::number(-2, 2)))));
  // 0, ..., 2
  tDeffer += (float) ((100 - looker->getSkillValue(tSkill)) / 100);
  // -.20, ..., .00, ..., .20
  tDeffer = ((float) ::number((int) tDeffer, (int) -(tDeffer))) / 10;

  for (tAff = looked->affected; tAff; tAff = tAff->next)
    if (tAff->type == tSkill) {
      tDeffer = (float) (tAff->duration + (tDeffer * tAff->duration));

      sprintf(tBuf, "Approx Time Left: %s\n\r",
              describeDuration(looker, (int) tDeffer).c_str());
      return (tBuf);
    }

  return ("");
}

static void describeSpellEffects(const TBeing *me, const TBeing *ch, bool verbose)
{
  char   bufspell[1024];
  char   bufpray[1024];
  int    totspell = 0;
  int    totpray  = 0;
  string tStSpell(""),
         tStPray("");

  // these are the less common or more urgent affects that we want noticed
  if (me->getCaptiveOf()) {
    char buf[256];
    sprintf(buf, ".....$n is held captive by %s.",me->getCaptiveOf()->getName());
    act(buf, FALSE, me, 0, ch, TO_VICT);
  }

  if (me->isAffected(AFF_MUNCHING_CORPSE))
    act(".....$n is munching on a corpse!", FALSE, me, 0, ch, TO_VICT);

  if (me->isFlying()) {
    if (me->roomp && me->roomp->isUnderwaterSector())
      act(".....$n is swimming around.", FALSE, me, 0, ch, TO_VICT);
    else
      act(".....$n is flying around.", FALSE, me, 0, ch, TO_VICT);
  }
  if (me->isLevitating())
    act(".....$n is hovering above the $g!", FALSE, me, 0, ch, TO_VICT);

  if (me->isAffected(AFF_BLIND) && !me->affectedBySpell(SPELL_TRUE_SIGHT) &&
      me->getPosition() > POSITION_SITTING && me->getPosition() <= POSITION_STANDING)
    act(".....$n blindly stumbles around!", FALSE, me, 0, ch, TO_VICT);

  if (me->isAffected(AFF_WEB)) {
    if (me->affectedBySpell(SPELL_LIVING_VINES))
      act(".....$n is entangled in a mass of vines!", FALSE, me, 0, ch, TO_VICT);
    else
      act(".....$n is enshrouded in magical webs!", FALSE, me, 0, ch, TO_VICT);
  }

  if (me->isAffected(AFF_SANCTUARY)) {
    act(".....$n glows with a bright light!", FALSE, me, 0, ch, TO_VICT);
    //act(displayShowApprox(me, ch, SPELL_SANCTUARY, 2.0).c_str(),
    //    FALSE, me, 0, ch, TO_VICT);
  }

  // more common effects that we wish to suppress
  bufspell[0] = '\0';
  bufpray[0]  = '\0';

  if (me->affectedBySpell(SPELL_FAERIE_FIRE)) {
    sprintf(bufspell, ".....$n is surrounded by a pink aura!\n\r");
    tStSpell += bufspell;
    tStSpell += displayShowApprox(me, ch, SPELL_FAERIE_FIRE, 1.0);
    ++totspell;
  }

  if (me->affectedBySpell(SPELL_FLAMING_FLESH)) {
    sprintf(bufspell, ".....$n has a ring of fire about $m!\n\r");
    tStSpell += bufspell;
    tStSpell += displayShowApprox(me, ch, SPELL_FLAMING_FLESH, 1.3);
    ++totspell;
  }

  if (me->affectedBySpell(SPELL_STONE_SKIN)) {
    sprintf(bufspell, ".....$n's skin has a gritty, rock-like look!\n\r");
    tStSpell += bufspell;
    tStSpell += displayShowApprox(me, ch, SPELL_STONE_SKIN, 1.3);
    ++totspell;
  }

  if (me->affectedBySpell(SPELL_SORCERERS_GLOBE)) {
    sprintf(bufspell, ".....pulsating swirls of magic encircle $n.\n\r");
    tStSpell += bufspell;
    tStSpell += displayShowApprox(me, ch, SPELL_SORCERERS_GLOBE, .5);
    ++totspell;
  }

  if (me->affectedBySpell(SPELL_ARMOR)) {
    sprintf(bufpray, ".....$n is surrounded by a radiant hue.\n\r");
    tStPray += bufpray;
    tStPray += displayShowApprox(me, ch, SPELL_ARMOR, .6);
    ++totpray;
  }

  if (me->affectedBySpell(SPELL_BLESS)) {
    sprintf(bufpray, ".....$n emits a brilliant aura.\n\r");
    tStPray += bufpray;
    tStPray += displayShowApprox(me, ch, SPELL_BLESS, .2);
    ++totpray;
  }

  if (me->affectedBySpell(SKILL_BARKSKIN)) {
    sprintf(bufspell, ".....$n seems coated in a thin veneer.\n\r");
    tStSpell += bufspell;
    tStSpell += displayShowApprox(me, ch, SKILL_BARKSKIN, 1.2);
    ++totspell;
  }

  if (me->affectedBySpell(SPELL_PLASMA_MIRROR)) {
    sprintf(bufspell, ".....$n has great swirls of plasma surrounding $m.\n\r");
    tStSpell += bufspell;
    tStSpell += displayShowApprox(me, ch, SPELL_PLASMA_MIRROR, 2.0);
    ++totspell;
  }

  strcpy(bufspell, tStSpell.c_str());
  strcpy(bufpray, tStPray.c_str());

  if (totspell > 0) {
    if (verbose || totspell == 1) {
      bufspell[strlen(bufspell) - 2] = '\0'; // strip off the trailing \n\r, act() re-adds it
      act(bufspell, FALSE, me, 0, ch, TO_VICT);
    } else {
      bufspell[0] = '\0';
      sprintf(bufspell, ".....$n is surrounded by ");
      if (totspell < 3)
	strcat(bufspell, "a couple of");
      else if (totspell < 6)
	strcat(bufspell, "a few");
      else if (totspell < 9)
	strcat(bufspell, "several");
      else
	strcat(bufspell, "a great many");

      strcat(bufspell, " magical effects.");
      act(bufspell, FALSE, me, 0, ch, TO_VICT);
    }
  }

  if (totpray > 0) {
    if (verbose || totpray == 1) {
      bufpray[strlen(bufpray) - 2] = '\0'; // strip off the trailing \n\r, act() re-adds it
      act(bufpray, FALSE, me, 0, ch, TO_VICT);
    } else {
      bufpray[0] = '\0';
      sprintf(bufpray, ".....$n is surrounded by ");
      if (totpray < 3)
	strcat(bufpray, "a couple of");
      else if (totpray < 6)
	strcat(bufpray, "a few");
      else if (totpray < 9)
	strcat(bufpray, "several");
      else
	strcat(bufpray, "a great many");

      strcat(bufpray, " spiritual effects.");
      act(bufpray, FALSE, me, 0, ch, TO_VICT);
    }
  }
}

void TBeing::show_me_to_char(TBeing *ch, showModeT mode) const
{
  char buffer[10000];
  char buf[80], capbuf[256];
  int found, percent;
  TThing *t;

  if (mode == SHOW_MODE_DESC_PLUS) {
    if (!ch->canSee(this)) {
      if (ch->canSee(this, INFRA_YES)) {
        sprintf(buffer,"A blob of heat is here in the shape of %s %s.\n\r",
          startsVowel(getMyRace()->getSingularName().c_str()) ? "an" : "a",
          getMyRace()->getSingularName().c_str());
        ch->sendTo(buffer);
      } else if (ch->isAffected(AFF_SENSE_LIFE) && (GetMaxLevel() < 51))
        ch->sendTo("You sense a hidden life form in the room.\n\r");

      return;
    }
    const TMonster *tm = dynamic_cast<const TMonster *>(this);
    if (riding || spelltask || fight() ||
        (roomp->isWaterSector() && !isAffected(AFF_WATERBREATH)) ||
        !(player.longDescr) ||
        (tm && tm->getPosition() != tm->default_pos)) {
      // A player char or a mobile without long descr, or not in default pos. 
      if (hasColorStrings(NULL, getName(), 2)) {
        if (dynamic_cast<const TPerson *>(this))
          sprintf(buffer, "%s", colorString(ch, ch->desc, getName(), NULL, COLOR_MOBS, FALSE).c_str());
        else 
          sprintf(buffer, "%s", colorString(ch, ch->desc, good_cap(getName()).c_str(),NULL, COLOR_MOBS, FALSE).c_str());
      } else 
        sprintf(buffer, "%s%s%s", ch->cyan(), good_cap(getName()).c_str(), ch->norm());
      
      if (isAffected(AFF_INVISIBLE) || getInvisLevel() > MAX_MORT)
        strcat(buffer, " (invisible)");
      if (isZombie())
        strcat(buffer, " (thrall)");
      if (isCharm())
        strcat(buffer, " (charm)");
      if (isPet())
        strcat(buffer, " (pet)");
      if (ch->isImmortal() && isLinkdead())
        strcat(buffer, " (link-dead)");
      if (getTimer() >= 10)
        strcat(buffer, " (AFK)");
      if (desc && desc->connected)
        strcat(buffer, " (editing)");
      if (ch->isImmortal() && isDiurnal())
        strcat(buffer, " (diurnal)");
      if (ch->isImmortal() && isNocturnal())
        strcat(buffer, " (nocturnal)");
      if (spelltask) {
        if (((discArray[(spelltask->spell)])->typ) == SPELL_MAGE) 
          strcat(buffer, " is here, casting a spell.");
        else if (((discArray[(spelltask->spell)])->typ) == SPELL_CLERIC) 
          strcat(buffer, " is here, reciting a prayer.");
      } else if (fight()) {
        strcat(buffer, " is here, fighting ");
        if (fight() == ch)
          strcat(buffer, " YOU!");
        else {
          if (sameRoom(fight())) {
            strcat(buffer, ch->pers(fight()));
            strcat(buffer, ".");
          } else
            strcat(buffer, "someone who has already left.");
        }
      } else {
        TBeing *tbr;
        switch (getPosition()) {
          case POSITION_STUNNED:
            strcat(buffer, " is lying here, stunned.");
            break;
          case POSITION_INCAP:
            strcat(buffer, " is lying here, incapacitated.");
            break;
          case POSITION_MORTALLYW:
            strcat(buffer, " is lying here, mortally wounded.");
            break;
          case POSITION_DEAD:
            strcat(buffer, " is lying here, dead.");
            break;
          case POSITION_MOUNTED:
            tbr = dynamic_cast<TBeing *>(riding);
            if (tbr &&
                dynamic_cast<const TBeing *>(riding->horseMaster()) == this) {
              strcat(buffer, " is here, riding ");
              strcat(buffer, ch->pers(riding));
              if (tbr->isAffected(AFF_INVISIBLE))
                strcat(buffer, " (invisible)");
              strcat(buffer, ".");
            } else if (tbr && tbr->horseMaster()) {
              if (ch == tbr->horseMaster())
                sprintf(buffer+strlen(buffer)," is here, also riding your horse. ");
              else
                sprintf(buffer+strlen(buffer)," is here, also riding on %s's ",
                    ch->pers(tbr->horseMaster()));
              strcat(buffer, ch->persfname(riding).c_str());
              if (tbr->isAffected(AFF_INVISIBLE))
                strcat(buffer, " (invisible)");
              strcat(buffer, ".");
            } else
              strcat(buffer, " is standing here.");

            break;
          case POSITION_FLYING:
            if (roomp && roomp->isUnderwaterSector()) 
              strcat(buffer, " is swimming about.");
	    else
	      strcat(buffer, " is flying about.");
            break;
          case POSITION_STANDING:
            if(roomp->isWaterSector())
              strcat(buffer, " is floating here.");
            else if (isCombatMode(ATTACK_BERSERK))
              sprintf(buffer + strlen(buffer), " is standing here with a crazy glint in %s eye.", hshr());
            else
              strcat(buffer, " is standing here.");
            break;
          case POSITION_CRAWLING:
            strcat(buffer, " is crawling here.");
            break;
          case POSITION_SITTING:
            if(roomp->isWaterSector())
              strcat(buffer, " is floating here.");
            else if (riding) {
              strcat(buffer, " is sitting on ");
              if (riding->getName())
                strcat(buffer,ch->objs(riding));
              else
                strcat(buffer, "A bad object");

              strcat(buffer,".");
            } else
              strcat(buffer, " is sitting here.");
            break;
          case POSITION_RESTING:
            if (roomp->isWaterSector())
              strcat(buffer, "is resting here in the water.");
            else if (riding) {
              strcat(buffer, " is resting on ");
              if (riding->getName())
                strcat(buffer,ch->objs(riding));
              else
                strcat(buffer, "A bad object");

              strcat(buffer,".");
            } else
              strcat(buffer, " is resting here.");
            break;
          case POSITION_SLEEPING:
            if(roomp->isWaterSector())
              strcat(buffer, "is sleeping here in the water.");
            else if (riding) {
              strcat(buffer, " is sleeping on ");
              if (riding->getName())
                strcat(buffer,ch->objs(riding));
              else
                strcat(buffer, "A bad object");

              strcat(buffer,".");
            } else
              strcat(buffer, " is sleeping here.");
            break;
          default:
            strcat(buffer, " is floating here.");
            break;
        }
      }
      strcat(buffer, "\n\r");
      sprintf(buffer,"%s",colorString(ch,ch->desc,buffer,NULL,COLOR_MOBS, TRUE).c_str());
      ch->sendTo(buffer);
    } else {        // npc with long 
      if (isAffected(AFF_INVISIBLE))
        strcpy(buffer, "*");
      else
        *buffer = '\0';

      sprintf(capbuf, "%s",
              addNameToBuf(ch, ch->desc, this, player.longDescr, COLOR_MOBS).c_str());
      string Strng = capbuf;
      // let's concat the name of a loser god that didn't put it in their
      // long desc
      if (isPc() && (Strng.find(getName()) == string::npos) && ch->isImmortal() &&
          (hasWizPower(POWER_WIZARD) || ch->GetMaxLevel() > GetMaxLevel()) &&
          ch->isPc())
        sprintf(capbuf + strlen(capbuf), " (%s)", getName());
      Strng = capbuf;
      while (Strng.find("$$g") != string::npos)
        Strng.replace(Strng.find("$$g"), 3,
                      (roomp ? roomp->describeGround().c_str() : "TELL A GOD"));
      while (Strng.find("$g") != string::npos)
        Strng.replace(Strng.find("$g"), 2,
                      (roomp ? roomp->describeGround().c_str() : "TELL A GOD"));
      strcpy(capbuf, Strng.c_str());
      cap(capbuf);
      strcat(buffer, capbuf);
   
      while ((buffer[strlen(buffer) - 1] == '\r') ||
             (buffer[strlen(buffer) - 1] == '\n') ||
             (buffer[strlen(buffer) - 1] == ' ')) {
        buffer[strlen(buffer) - 1] = '\0';
      }
      if (isZombie())
        strcat(buffer, " (thrall)");
      if (isCharm())
        strcat(buffer, " (charm)");
      if (isPet())
        strcat(buffer, " (pet)");
      if (ch->isImmortal() && isLinkdead())
        strcat(buffer, " (link-dead)");
      if (getTimer() >= 10)
        strcat(buffer, " (AFK)");
      if (desc && desc->connected)
        strcat(buffer, " (editing)");
      if (ch->isImmortal() && isDiurnal())
        strcat(buffer, " (diurnal)");
      if (ch->isImmortal() && isNocturnal())
        strcat(buffer, " (nocturnal)");

      strcat(buffer, "\n\r");
      sprintf(buffer,"%s",colorString(ch,ch->desc,buffer,NULL,COLOR_MOBS, TRUE).c_str());
      ch->sendTo(buffer);
    }

    if (task && task->task != TASK_SIT && 
        task->task != TASK_REST && 
        task->task != TASK_SLEEP)
      ch->sendTo(COLOR_MOBS,".....%s is busy %s.\n\r", getName(), tasks[task->task].name);
    if (checkSlots() && getPosition() == POSITION_SITTING)
      act(".....$n is sitting at the slot machine!", FALSE, this, 0, ch, TO_VICT);
    if(!ch->isPlayerAction(PLR_BRIEF))
      describeSpellEffects(this, ch, FALSE);
  } else if (mode == SHOW_MODE_SHORT_PLUS) {
    if (getDescr()) {
      sprintf(capbuf, "%s", addNameToBuf(ch, ch->desc, this, getDescr(), COLOR_MOBS).c_str());
      string cStrbuf = capbuf;
      while (cStrbuf.find("$$g") != string::npos)
        cStrbuf.replace(cStrbuf.find("$$g"), 3,
                        (roomp ? roomp->describeGround().c_str() : "TELL A GOD"));
      while (cStrbuf.find("$g") != string::npos)
        cStrbuf.replace(cStrbuf.find("$g"), 2,
                        (roomp ? roomp->describeGround().c_str() : "TELL A GOD"));
      strcpy(capbuf, cStrbuf.c_str());
      cap(capbuf);
      ch->sendTo(COLOR_MOBS, "%s", capbuf);
    } else 
      act("You see nothing special about $m.", FALSE, this, 0, ch, TO_VICT);
    
    describeSpellEffects(this, ch, TRUE);

    if (isPc() || (getRace() <= RACE_OGRE)) {
      strcpy(capbuf, race->proper_name.c_str());
      sprintf(buffer, "$n is of the %s race.", uncap(capbuf));
      sprintf(buffer,"%s",colorString(ch,ch->desc,buffer,NULL,COLOR_MOBS, TRUE).c_str());
      act(buffer, FALSE, this, 0, ch, TO_VICT);
    }
    if (riding && dynamic_cast<TObj *>(riding)) {
      sprintf(buffer, "$n is %s on %s.", 
            good_uncap(position_types[getPosition()]).c_str(), 
            riding->getName());
      sprintf(buffer,"%s",colorString(ch,ch->desc,buffer,NULL,COLOR_MOBS, TRUE).c_str());
      act(buffer, FALSE, this, 0, ch, TO_VICT);
    }
    if (riding && dynamic_cast<TBeing *>(riding)) {
      sprintf(buffer, "$n is mounted on $p.");
      act(buffer, FALSE, this, riding, ch, TO_VICT);
    }
    if (rider && dynamic_cast<TBeing *>(rider)) {
      sprintf(buffer, "$n is ridden by $p.");
      act(buffer, FALSE, this, horseMaster(), ch, TO_VICT);
      for (t = rider; t; t = t->nextRider) {
        if (t == horseMaster())
          continue;
        if (!dynamic_cast<TBeing *>(t))
          continue;
        sprintf(buffer, "$n is also being ridden by $p.");
        act(buffer, FALSE, this, t, ch, TO_VICT);
      }
    }

    if (hitLimit() > 0)
      percent = (100 * getHit()) / hitLimit();
    else
      percent = -1;        // How could MAX_HIT be < 1?? 

    if (percent >= 100)
      strcpy(buffer, "$n looks healthy.");
    else if (percent >= 90)
      strcpy(buffer, "$n looks slightly scratched.");
    else if (percent >= 75)
      strcpy(buffer, "$n has some small wounds and bruises.");
    else if (percent >= 50)
      strcpy(buffer, "$n has many wounds.");
    else if (percent >= 30)
      strcpy(buffer, "$n has large wounds and scratches.");
    else if (percent >= 15)
      strcpy(buffer, "$n looks very hurt.");
    else if (percent >= 0)
      strcpy(buffer, "$n is in awful condition.");
    else
      strcpy(buffer, "$n has many large wounds and is near death.");
    act(buffer, FALSE, this, 0, ch, TO_VICT);

    if (!isPc()) {
      percent = dynamic_cast<const TMonster *>(this)->anger();
      if (percent >= 95)
        strcpy(buffer,"$n is raving mad!");
      else if (percent >= 80)
        strcpy(buffer,"$n is infuriated.");
      else if (percent >= 65)
        strcpy(buffer,"$n is fuming.");
      else if (percent >= 50)
        strcpy(buffer,"$n is really angry.");
      else if (percent >= 35)
        strcpy(buffer,"$n seems mad.");
      else if (percent >= 20)
        strcpy(buffer,"$n seems irritated.");
      else if (percent >= 5)
        strcpy(buffer,"$n is displeased.");
      else 
        strcpy(buffer,"$n seems peaceful.");
      act(buffer, FALSE,this, 0,ch,TO_VICT);
    }
    if (curStats.get(STAT_STR) > 190)
      strcpy(buffer,"$e is unhumanly muscular, ");
    else if (curStats.get(STAT_STR) > 170)
      strcpy(buffer,"$e is extremely brawny, ");
    else if (curStats.get(STAT_STR) > 150)
      strcpy(buffer,"$e is brawny, ");
    else if (curStats.get(STAT_STR) > 130)
      strcpy(buffer,"$e is muscular, ");
    else if (curStats.get(STAT_STR) > 110)
      strcpy(buffer,"$e is fairly muscular, ");
    else if (curStats.get(STAT_STR) > 90)
      strcpy(buffer,"$e has an average build, ");
    else if (curStats.get(STAT_STR) > 70)
      strcpy(buffer,"$e is somewhat frail, ");
    else if (curStats.get(STAT_STR) > 50)
      strcpy(buffer,"$e is frail, ");
    else if (curStats.get(STAT_STR) > 30)
      strcpy(buffer,"$e is feeble, ");
    else
      strcpy(buffer,"$e is extremely feeble, ");

    if (curStats.get(STAT_AGI) > 190)
      strcat(buffer,"astoundingly agile, and ");
    else if (curStats.get(STAT_AGI) > 160)
      strcat(buffer,"amazingly agile, and ");
    else if (curStats.get(STAT_AGI) > 140)
      strcat(buffer,"agile, and ");
    else if (curStats.get(STAT_AGI) > 120)
      strcat(buffer,"graceful, and ");
    else if (curStats.get(STAT_AGI) > 80)
      strcat(buffer,"able-bodied, and ");
    else if (curStats.get(STAT_AGI) > 60)
      strcat(buffer,"clumsy, and ");
    else if (curStats.get(STAT_AGI) > 30)
      strcat(buffer,"awkward and clumsy, and ");
    else
      strcat(buffer,"a bumbling clutz, and ");
   
    if (curStats.get(STAT_CHA) > 190)
      strcat(buffer,"has godlike grace.");
    else if (curStats.get(STAT_CHA) > 160)
      strcat(buffer, "is exceptionally well-favored.");
    else if (curStats.get(STAT_CHA) > 140)
      strcat(buffer, "is well-favored.");
    else if (curStats.get(STAT_CHA) > 120)
      strcat(buffer, "is attractive.");
    else if (curStats.get(STAT_CHA) > 100)
      strcat(buffer, "is comely.");
    else if (curStats.get(STAT_CHA) > 80)
      strcat(buffer, "is average looking.");
    else if (curStats.get(STAT_CHA) > 60)
      strcat(buffer, "is dumpy.");
    else if (curStats.get(STAT_CHA) > 40)
      strcat(buffer, "is ill-favored.");
    else
      strcat(buffer, "is extremely ill-favored.");

    if (isHumanoid())
      act(buffer, TRUE,this, 0,ch,TO_VICT);

    sprintf(buffer, "$e is about %d'%d\" tall and weighs around %d pounds.",getHeight()/INCHES_PER_FOOT,getHeight()%INCHES_PER_FOOT,(int) getWeight());
    act(buffer, TRUE,this,0,ch,TO_VICT);

    act("\n\r",FALSE,this,0,ch,TO_VICT);
    found = FALSE;
    wearSlotT ij;
    for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
      if (equipment[ij]) {
        if (ch->canSee(equipment[ij]))
          found = TRUE;
      }
      if (isLimbFlags(ij, PART_TRANSFORMED))
          found = TRUE;
    }
    if (found && ch->GetMaxLevel() != GOD_LEVEL1) {
      act("$n is using:", FALSE, this, 0, ch, TO_VICT);
      for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
        if (isLimbFlags(ij, PART_TRANSFORMED)) {
          if (shouldDescTransLimb(ij)) {
            sprintf(buf, "<%s>", describeTransEquipSlot(ij).c_str());
            ch->sendTo("%s\n\r", buf);
          }
        }
        if (equipment[ij]) {
          if (ch->canSee(equipment[ij])) {
            if (!equipment[ij]->shouldntBeShown(ij)) {
              sprintf(buf, "<%s>", describeEquipmentSlot(ij).c_str());
              ch->sendTo("%-25s", buf);
              ch->showTo(equipment[ij], SHOW_MODE_SHORT_PLUS);
            }
          }
        }
      }
    }

    ch->describeLimbDamage(this);

    found = FALSE;
    if ((ch != this) && !ch->isImmortal() && ch->affectedBySpell(SKILL_SPY)) {
      int value = getMoney();
      int skill = ch->getSkillValue(SKILL_SPY);
      ch->sendTo("\n\rYou attempt to peek at the inventory:\n\r");
      if (ch->isAffected(AFF_SCRYING)) {
        list_in_heap(stuff, ch, FALSE, skill);
      }

      // randomize wealth report
      // this is anywhere from 0* to 2* as much at 0 learning, to exact at 100%
      if (!ch->isAffected(AFF_SCRYING))
        skill = 0;
      value *= ::number(skill/10, (20-skill/10));
      value /= 10;
      ch->sendTo(COLOR_MOBS,"\n\rYou estimate %s has %d talens.\n\r", getName(), value);

    } else if (ch->isImmortal()) {
      ch->sendTo("Inventory:\n\r");

      for (t = stuff; t; t = t->nextThing) {
        ch->showTo(t, SHOW_MODE_SHORT_PLUS);
        found = TRUE;
      }
      if (!found)
        ch->sendTo("Nothing.\n\r");
    }
  } else if (mode == SHOW_MODE_SHORT_PLUS_INV) {
    act("$N is carrying:", FALSE, ch, 0, this, TO_CHAR);
    list_in_heap(stuff, ch, false, 100);
  }
}


void TBeing::show_me_mult_to_char(TBeing *ch, showModeT, unsigned int num) const
{
  char buffer[MAX_STRING_LENGTH];
  char tmp[10];
  char capbuf[256];


  if (!ch->canSee(this)) {
    if (ch->canSee(this, INFRA_YES)) {
      if (num == 1)
        sprintf(buffer,"A blob of heat is here in the shape of %s %s.\n\r",
          startsVowel(getMyRace()->getSingularName().c_str()) ? "an" : "a",
          getMyRace()->getSingularName().c_str());
      else
        sprintf(buffer,"A few blobs of heat are here in the shape of %s.\n\r",
          getMyRace()->getPluralName().c_str());
      ch->sendTo(buffer);
    } else if (ch->isAffected(AFF_SENSE_LIFE) && (GetMaxLevel() < 51))
      if (num == 1)
        ch->sendTo("You sense a hidden life form in the room.\n\r");
      else
        ch->sendTo("You sense hidden life forms in the room.\n\r");
    return;
  }
  const TMonster *tm = dynamic_cast<const TMonster *>(this);
  if (riding || spelltask || fight() ||
        (roomp->isWaterSector() && !isAffected(AFF_WATERBREATH)) ||
        !(player.longDescr) ||
        (tm && tm->getPosition() != tm->default_pos)) {
    // A player char or a mobile without long descr, or not in default pos. 
    strcpy(buffer, getName());
    cap(buffer);
    if (isAffected(AFF_INVISIBLE))
      strcat(buffer, " (invisible)");
    if (isZombie())
      strcat(buffer, " (thrall)");
    if (isCharm())
      strcat(buffer, " (charm)");
    if (isPet())
      strcat(buffer, " (pet)");
    if (ch->isImmortal() && isLinkdead())
      strcat(buffer, " (link-dead)");
    if (getTimer() >= 10)
      strcat(buffer, " (AFK)");
    if (desc && desc->connected)
      strcat(buffer, " (editing)");
    if (ch->isImmortal() && isDiurnal())
      strcat(buffer, " (diurnal)");
    if (ch->isImmortal() && isNocturnal())
      strcat(buffer, " (nocturnal)");

    if (fight()) {
      strcat(buffer, " is here, fighting ");
      if (fight() == ch)
        strcat(buffer, " YOU!");
      else {
        if (sameRoom(fight())) {
          strcat(buffer, ch->pers(fight()));
          strcat(buffer, ".");
        } else
          strcat(buffer, "someone who has already left.");
      }
    } else {
      switch (getPosition()) {
        case POSITION_STUNNED:
          strcat(buffer, " is lying here, stunned.");
          break;
        case POSITION_INCAP:
          strcat(buffer, " is lying here, incapacitated.");
          break;
        case POSITION_MORTALLYW:
          strcat(buffer, " is lying here, mortally wounded.");
          break;
        case POSITION_DEAD:
          strcat(buffer, " is lying here, dead.");
          break;
        case POSITION_FLYING:
          if (roomp && roomp->isUnderwaterSector()) 
            strcat(buffer, " is swimming about.");
	  else
	    strcat(buffer, " is flying about.");
          break;
        case POSITION_STANDING:
          strcat(buffer, " is standing here.");
          break;
        case POSITION_CRAWLING:
          strcat(buffer, " is crawling here.");
          break;
        case POSITION_SITTING:
          if (riding) {
            strcat(buffer, " is sitting here on ");
            if (riding->getName())
              strcat(buffer,ch->objs(riding));
            else
              strcat(buffer, "A bad object");

            strcat(buffer,".");
          } else
            strcat(buffer, " is sitting here.");
          break;
        case POSITION_RESTING:
          if (riding) {
            strcat(buffer, " is resting here on ");
            if (riding->getName())
              strcat(buffer,ch->objs(riding));
            else
              strcat(buffer, "A bad object");

            strcat(buffer,".");
          } else
            strcat(buffer, " is resting here.");
          break;
        case POSITION_SLEEPING:
          if (riding) {
            strcat(buffer, " is sleeping here on ");
            if (riding->getName())
// COSMO MARKER --test
              if (dynamic_cast<TBeing *>(riding)) {
                strcat(buffer,ch->objs(riding));
              } else {
                if (IS_SET(ch->desc->plr_color, PLR_COLOR_OBJECTS)) {
                  sprintf(capbuf, "%s", colorString(ch, ch->desc, ch->objs(riding), NULL, COLOR_OBJECTS, FALSE).c_str());

                  sprintf(capbuf, "%s", ch->objs(riding));
                  strcat(buffer, capbuf);
//                  strcat(buffer,ch->objs(riding));
                }
              }
            else
              strcat(buffer, "A bad object");

            strcat(buffer,".");
          } else
            strcat(buffer, " is sleeping here.");
          break;
        default:
          strcat(buffer, " is floating here.");
          break;
      }
    }
    if (num > 1) {
      sprintf(tmp, " [%d]", num);
      strcat(buffer, tmp);
    }
    strcat(buffer, "\n\r");
// COSMO MARKER -- test
    ch->sendTo(COLOR_MOBS, cap(buffer));
  } else {  // npc with long 
    if (isAffected(AFF_INVISIBLE)) 
      strcpy(buffer, "*");
    else 
      *buffer = '\0';
    
    sprintf(capbuf, "%s", addNameToBuf(ch, ch->desc, this, player.longDescr, COLOR_MOBS).c_str());
    string cStrbuf = capbuf;
    while (cStrbuf.find("$$g") != string::npos)
      cStrbuf.replace(cStrbuf.find("$$g"), 3,
                      (roomp ? roomp->describeGround().c_str() : "TELL A GOD"));
    while (cStrbuf.find("$g") != string::npos)
      cStrbuf.replace(cStrbuf.find("$g"), 2,
                      (roomp ? roomp->describeGround().c_str() : "TELL A GOD"));
    strcpy(capbuf, cStrbuf.c_str());
    cap(capbuf);
    strcat(buffer, capbuf);

    if (num > 1) {
      while ((buffer[strlen(buffer) - 1] == '\r') ||
             (buffer[strlen(buffer) - 1] == '\n') ||
             (buffer[strlen(buffer) - 1] == ' ')) {
        buffer[strlen(buffer) - 1] = '\0';
      }
      if (isZombie())
        strcat(buffer, " (thrall)");
      if (isCharm())
        strcat(buffer, " (charm)");
      if (isPet())
        strcat(buffer, " (pet)");
      if (ch->isImmortal() && isLinkdead())
        strcat(buffer, " (link-dead)");
      if (getTimer() >= 10)
        strcat(buffer, " (AFK)");
      if (desc && desc->connected)
        strcat(buffer, " (editing)");
      if (ch->isImmortal() && isDiurnal())
        strcat(buffer, " (diurnal)");
      if (ch->isImmortal() && isNocturnal())
        strcat(buffer, " (nocturnal)");

      sprintf(tmp, " [%d]\n\r", num);
      strcat(buffer, tmp);
    }
      sprintf(buffer,"%s",colorString(ch,ch->desc,buffer,NULL,COLOR_MOBS, TRUE).c_str());
      ch->sendTo(buffer);

//    ch->sendTo(COLOR_MOBS, buffer);
  }
#if 1
  // these no longer show up, just seen on look-at which isn't part of 
  // show mult
  describeSpellEffects(this, ch, FALSE);
#endif
}

void TBeing::doGlance(const char *argument)
{
  char buffer[1024];
  int percent;
  TBeing *i;

  for (; isspace(*argument); argument++);

  if (*argument) {
    if ((i = get_char_room_vis(this, argument))) {
      if (i->hitLimit() > 0)
        percent = (100 * i->getHit()) / i->hitLimit();
      else
        percent = -1;                // How could MAX_HIT be < 1?? 

      if (percent >= 100)
        strcpy(buffer, "$n looks healthy.");
      else if (percent >= 90)
        strcpy(buffer, "$n looks slightly scratched.");
      else if (percent >= 75)
        strcpy(buffer, "$n has some small wounds and bruises.");
      else if (percent >= 50)
        strcpy(buffer, "$n has many wounds.");
      else if (percent >= 30)
        strcpy(buffer, "$n has large wounds and scratches.");
      else if (percent >= 15)
        strcpy(buffer, "$n looks very hurt.");
      else if (percent >= 0)
        strcpy(buffer, "$n is in awful condition.");
      else
        strcpy(buffer, "$n has many large wounds and is near death.");

      act(buffer, FALSE, i, 0, this, TO_VICT);
      describeLimbDamage(i);
//      describeAffects(i);
      sendTo(COLOR_MOBS, "%s look%s %s.\n\r",
        (i == this ? "You" : good_cap(i->getName())).c_str(),
        (i == this ? "" : "s"),
        DescMoves((((double) i->getMove()) / ((double) i->moveLimit()))));

      describeSpellEffects(i, this, TRUE);
    } else {
      sendTo("You don't see that here.\n\r");
      return;
    }
  } else {
    sendTo("You usually glance at someone!\n\r");
    return;
  }
}

static void print_room(int rnum, TRoom *rp, string &sb, struct show_room_zone_struct *)
{
  char buf[10240];
  int dink, bits, scan;

  sprintf(buf, "%5d %4d %-12s     %s\n\r", rp->number, rnum,
        TerrainInfo[rp->getSectorType()]->name, (rp->name ? rp->name : "Empty"));
  if (rp->getRoomFlags()) {
    strcat(buf, "    [");

    dink = 0;
    for (bits = rp->getRoomFlags(), scan = 0; bits; scan++) {
      if (bits & (1 << scan)) {
        if (dink)
          strcat(buf, " ");
        if (scan < MAX_ROOM_BITS)
          strcat(buf, room_bits[scan]);
        dink = 1;
        bits ^= (1 << scan);
      }
    }
    strcat(buf, "]\n\r");
  }

  sb += buf;
}

static void print_lit_room(int rnum, TRoom *rp, string &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_ALWAYS_LIT))
    print_room(rnum, rp, sb, NULL);
}

static void print_save_room(int rnum, TRoom *rp, string &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_SAVE_ROOM))
    print_room(rnum, rp, sb, NULL);
}

static void print_death_room(int rnum, TRoom *rp, string &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_DEATH))
    print_room(rnum, rp, sb, NULL);
}

static void print_hospital_room(int rnum, TRoom *rp, string &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_HOSPITAL))
    print_room(rnum, rp, sb, NULL);
}

static void print_noheal_room(int rnum, TRoom *rp, string &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_NO_HEAL))
    print_room(rnum, rp, sb, NULL);
}

static void print_arena_room(int rnum, TRoom *rp, string &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_ARENA))
    print_room(rnum, rp, sb, NULL);
}

static void print_noflee_room(int rnum, TRoom *rp, string &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_NO_FLEE))
    print_room(rnum, rp, sb, NULL);
}

static void print_private_room(int rnum, TRoom *rp, string &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_PRIVATE))
    print_room(rnum, rp, sb, NULL);
}

static void show_room_zone(int rnum, TRoom *rp, string &, struct show_room_zone_struct *srzs)
{
  char buf[MAX_STRING_LENGTH];

  *buf = '\0';

  if (!rp || rp->number < srzs->bottom || rp->number > srzs->top)
    return;            // optimize later 

  if (srzs->blank && (srzs->lastblank + 1 != rp->number)) {
    sprintf(buf, "rooms %d-%d are blank.\n\r", srzs->startblank, srzs->lastblank);
    srzs->sb += buf;
    srzs->blank = 0;
  }
  if (!rp->name) {
    vlogf(10, "room %d's name is screwed!\n\r", rp->number);
    return;
  } else if (1 == sscanf(rp->name, "%d", &srzs->lastblank) && srzs->lastblank == rp->number) {
    if (!srzs->blank) {
      srzs->startblank = srzs->lastblank;
      srzs->blank = 1;
    }
    return;
  } else if (srzs->blank) {
    sprintf(buf, "rooms %d-%d are blank.\n\r", srzs->startblank, srzs->lastblank);
    srzs->sb += buf;
    srzs->blank = 0;
  }
  print_room(rnum, rp, srzs->sb, NULL);
}

string TStaff::getNameForShow(bool useColor, bool useName, const TBeing *ch) const
{
  char buf2[256];
  sprintf(buf2, "%s [%s]", 
       useName ? name : (useColor ? getName() : getNameNOC(ch).c_str()),
       (getSpell() >= 0 ? (discArray[getSpell()] ? discArray[getSpell()]->name : "Unknown") : "None"));
  return buf2;
}

string TScroll::getNameForShow(bool useColor, bool useName, const TBeing *ch) const
{
  char buf2[256];
  sprintf(buf2, "%s [%s/%s/%s]", 
       useName ? name : (useColor ? getName() : getNameNOC(ch).c_str()),
       (getSpell(0) >= 0 ? (discArray[getSpell(0)] ? discArray[getSpell(0)]->name : "Unknown") : "None"),
       (getSpell(1) >= 0 ? (discArray[getSpell(1)] ? discArray[getSpell(1)]->name : "Unknown") : "None"),
       (getSpell(2) >= 0 ? (discArray[getSpell(2)] ? discArray[getSpell(2)]->name : "Unknown") : "None"));

  return buf2;
}

string TPotion::getNameForShow(bool useColor, bool useName, const TBeing *ch) const
{
  char buf2[256];
  sprintf(buf2, "%s [%s/%s/%s]", 
       useName ? name : (useColor ? getName() : getNameNOC(ch).c_str()),
       (getSpell(0) >= 0 ? (discArray[getSpell(0)] ? discArray[getSpell(0)]->name : "Unknown") : "None"),
       (getSpell(1) >= 0 ? (discArray[getSpell(1)] ? discArray[getSpell(1)]->name : "Unknown") : "None"),
       (getSpell(2) >= 0 ? (discArray[getSpell(2)] ? discArray[getSpell(2)]->name : "Unknown") : "None"));

  return buf2;
}

unsigned long int showFreeMobObj(int shFrZoneNumber, string *sb,
                                 bool isMobileF, bool shFrLoop=false)
{
  if (shFrZoneNumber < 0 || shFrZoneNumber >= ((signed int) zone_table.size())) {
    *sb += "Zone Number incorect.\n\r";
    return 0;
  }
                int shFrTop = 0,
                    shFrBot = 0,
                    shFrTopR = -1,
                    shFrBotR = -1;
  unsigned long int shFrTotalCount[2] = {0, 0};
  zoneData          &zd = zone_table[shFrZoneNumber];

  if (!zd.enabled)
    return 0;

  shFrTop = zd.top;
  shFrBot = (shFrZoneNumber ? zone_table[shFrZoneNumber - 1].top + 1: 0);

  int  shFrCountSize = (shFrTop - shFrBot + 1),
       shFrCountMax  = (isMobileF ? mob_index.size() : obj_index.size());
  bool shFrCountList[shFrCountSize];
  char tString[256];

  for (int Runner = 0; Runner < shFrCountSize; Runner++)
    shFrCountList[Runner] = false;

  shFrTotalCount[0] = shFrCountSize;

  for (int Runner = 0; Runner < shFrCountMax; Runner++) {
    if (( isMobileF && (mob_index[Runner].virt < shFrBot || mob_index[Runner].virt > shFrTop)) ||
        (!isMobileF && (obj_index[Runner].virt < shFrBot || obj_index[Runner].virt > shFrTop)))
      continue;

    int shFrWalkVirt = (isMobileF ? mob_index[Runner].virt : obj_index[Runner].virt) - shFrBot;

    shFrCountList[max(min(shFrWalkVirt, (shFrCountSize - 1)), 0)] = true;
    shFrTotalCount[0]--;
  }

  if (shFrTotalCount[0] > 0) {
    if (shFrLoop) {
      sprintf(tString, "**** Zone: %d\n\r", shFrZoneNumber);
      *sb += tString;
    }

    for (int Runner = 0; Runner < shFrCountSize; Runner++) {
      if (shFrCountList[Runner]) {
        if (shFrBotR != -1) {
          shFrTotalCount[1] = (shFrTopR - shFrBotR + 1);
          sprintf(tString, "%5d - %5d : %5lu Free\n\r",
                  shFrBotR, shFrTopR, shFrTotalCount[1]);
          *sb += tString;
        }

        shFrBotR = shFrTopR = -1;
      } else {
        if (shFrBotR == -1)
          shFrBotR = shFrTopR = (Runner + shFrBot);
        else
          shFrTopR = (Runner + shFrBot);

        if (Runner == (shFrCountSize - 1)) {
          shFrTotalCount[1] = (shFrTopR - shFrBotR + 1);
          sprintf(tString, "%5d - %5d : %5lu Free\n\r",
                  shFrBotR, shFrTopR, shFrTotalCount[1]);
          *sb += tString;
        }
      }
    }

    sprintf(tString, "----- Total Count: %5lu\n\r", shFrTotalCount[0]);
    *sb += tString;

    return shFrTotalCount[0];
  }

  return 0;
}

// Does major searching and returns the following:
// Dissection loads
// 'Nature' loads
// Scriptfile loads
string showComponentTechnical(const int tValue)
{
  string         tStString(""),
                 tStBuffer("");
  char           tString[256],
                 tBuffer[256];
  int            tMobNum;
  struct dirent *tDir;
  DIR           *tDirInfo;
  FILE          *tFile;

  // Check for dissection loads.
  // This doesn't check for hard-coded ones such as 'by race' and such.
  for (unsigned int tDissectIndex = 0; tDissectIndex < dissect_array.size(); tDissectIndex++)
    if ((dissect_array[tDissectIndex].loadItem == (unsigned) tValue)) {
      tMobNum = real_mobile(tDissectIndex);

      if (tMobNum < 0 || tMobNum > (signed) mob_index.size())
        strcpy(tBuffer, "[Unknown]");
      else
        strcpy(tBuffer, mob_index[tMobNum].name);

      sprintf(tString, "Dissect Load: %d %s\n\r", tDissectIndex, tBuffer);
      tStString += tString;
    }

  // Check for natural loads.  Unfortunatly it's easy to do a double entry here
  // so we have to be careful.
  for (unsigned int tCompIndex = 0; tCompIndex < component_placement.size(); tCompIndex++)
    if (component_placement[tCompIndex].number == tValue &&
        (component_placement[tCompIndex].place_act & CACT_PLACE)) {
      if (component_placement[tCompIndex].room2 == -1)
        tBuffer[0] = '\0';
      else
        sprintf(tBuffer, "-%d", component_placement[tCompIndex].room2);

      sprintf(tString, "Natural Load: Room%s %d%s\n\r",
              (!tBuffer[0] ? "" : "s"),
              component_placement[tCompIndex].room1,
              tBuffer);
      tStString += tString;
    }

  // Check for script loads.  This will go through ALL of the scripts and check.
  // We only do this on !PROD because of the lag it will generate, and I do mean a
  // LOT of lag it will make.
  if (gamePort != PROD_GAMEPORT) {
    if (!(tDirInfo = opendir("mobdata/responses"))) {
      vlogf(10, "Unable to dirwalk directory mobdata/resposnes");
      tStString += "ERROR.  Unable to open mobdata/responses for reading.";
      return tStString;
    }

    while ((tDir = readdir(tDirInfo))) {
      if (!strcmp(tDir->d_name, ".") || !strcmp(tDir->d_name, ".."))
        continue;

      sprintf(tBuffer, "mobdata/responses/%s", tDir->d_name);

      if (!(tFile = fopen(tBuffer, "r")))
        continue;

      while (fgets(tString, 256, tFile)) {
        char *tChar = tString;

        for (; isspace(*tChar) || *tChar == '\t'; tChar++);

        sprintf(tBuffer, "load %d;\n", tValue);

        if (!strcmp(tChar, tBuffer)) {
          tMobNum = real_mobile(atoi(tDir->d_name));

          if (tMobNum < 0 || tMobNum > (signed) mob_index.size())
            strcpy(tString, "[Unknown]");
          else
            strcpy(tString, mob_index[tMobNum].name);

          sprintf(tBuffer, "Script: %s %s\n\r",
                  tDir->d_name, tString);
          tStString += tBuffer;

          // Don't show the same entry twice.
          break;
        }
      }

      fclose(tFile);
    }

    closedir(tDirInfo);
  }

  return tStString;
}

void TBeing::doShow(const char *)
{
  return;
}

void TPerson::doShow(const char *argument)
{
  char      buf[256],
            zonenum[256],
            buf2[256];
  int       bottom = 0,
            top    = 0;
  string    sb;
  TBeing   *ch     = NULL,
           *b;
  TMonster *k;
  TObj     *obj;

  if (!isImmortal())
    return;

  if (!hasWizPower(POWER_SHOW)) {
    sendTo("You lack the power to show things.\n\r");
    return;
  }

  argument = one_argument(argument, buf);

// Show Race:
//   First, check for the race option.  Assign the second argument to target.
//   If target has a string (ie. not empty), check to see if they passed an
//   index number.  If so, call the appropriate race's showTo method and
//   return.  Make sure the number is less than the MAX_RACIAL_TYPES, else
//   send them a list of valid races.
//
//   If the index passed is 0, it will fail the first check, but they might
//   be checking on the RACE_NORACE stats, so see if they passed "0".  If so,
//   call NORACE's showTo method.  Otherwise, pass the string to the
//   getRaceIndex() function.  If it returns a valid index, call the
//   appropriate showTo method.  Otherwise, let it fall out of the "if" and
//   list all valid races.

  if (is_abbrev(buf, "races")) {
    if (!hasWizPower(POWER_SHOW_MOB)) {
      sendTo("You lack the power to show mob information.\n\r");
      return;
    }
    argument = one_argument(argument, buf);
    if (buf && *buf) {
      int raceIndex = atoi(buf);
      if (raceIndex < MAX_RACIAL_TYPES) {
        if ((raceIndex) && (Races[raceIndex])) {
          Races[raceIndex]->showTo(this);
          return;
        } else {
          if (!strcmp(buf, "0") && (Races[raceIndex])){
            Races[raceIndex]->showTo(this);
            return;
          } else {
            raceIndex = getRaceIndex(buf);
            if ((raceIndex >= RACE_NORACE) && (Races[raceIndex])) {
              Races[raceIndex]->showTo(this);
              return;
            }
          }
        }
      }
    }
    listRaces(this);
    return;
  }

  if (is_abbrev(buf, "factions")) {
    sb += "Faction                   Power    Wealth   Tithe\n\r";
    factionTypeT i;
    for (i = MIN_FACTION; i < MAX_FACTIONS; i++) {
      sprintf(buf, "%-25.25s %7.2f %-7ld %5.2f\n\r",
          FactionInfo[i].faction_name,
          FactionInfo[i].faction_power,
          FactionInfo[i].faction_wealth,
          FactionInfo[i].faction_tithe);
      sb += buf;
      sprintf(buf, "      %s%-10.10s%s %-10.10s %-10.10s %-10.10s\n\r",
          blue(), FactionInfo[i].leader[0], norm(),
          FactionInfo[i].leader[1],
          FactionInfo[i].leader[2],FactionInfo[i].leader[3]);
      sb += buf;
      sprintf(buf, "      %sCaravan:%s interval: %d, counter: %d, value: %d, defense: %d\n\r",
          blue(), norm(),
          FactionInfo[i].caravan_interval,
          FactionInfo[i].caravan_counter,
          FactionInfo[i].caravan_value,
          FactionInfo[i].caravan_defense);
      sb += buf;
      sprintf(buf, "             : attempts: %d, successes: %d\n\r",
          FactionInfo[i].caravan_attempts,
          FactionInfo[i].caravan_successes);
      sb += buf;
      sprintf(buf, "      %sHelp Ratio:%s %.1f, %.1f, %.1f, %.1f\n\r",
          blue(), norm(),
          FactionInfo[i].faction_array[FACT_NONE][OFF_HELP],
          FactionInfo[i].faction_array[FACT_BROTHERHOOD][OFF_HELP],
          FactionInfo[i].faction_array[FACT_CULT][OFF_HELP],
          FactionInfo[i].faction_array[FACT_SNAKE][OFF_HELP]);
      sb += buf;
      sprintf(buf, "      %sHarm Ratio:%s %.1f, %.1f, %.1f, %.1f\n\r",
          blue(), norm(),
          FactionInfo[i].faction_array[FACT_NONE][OFF_HURT],
          FactionInfo[i].faction_array[FACT_BROTHERHOOD][OFF_HURT],
          FactionInfo[i].faction_array[FACT_CULT][OFF_HURT],
          FactionInfo[i].faction_array[FACT_SNAKE][OFF_HURT]);
      sb += buf;
      sprintf(buf, "\n\r");
      sb += buf;
    }
    sprintf(buf, "average power: %5.2f\n\r", avg_faction_power);
    sb += buf;
  } else if (is_abbrev(buf, "fights")) {
    sb += "Combatant                      Fighting                       Room\n\r";
    sb += "------------------------------------------------------------------\n\r";
    for (ch = gCombatList; ch; ch = ch->next_fighting) {
      sprintf(buf, "%-30s %-30s %d\n\r", ch->getName(), ch->fight()->getName(), ch->inRoom());
      sb += buf;
    }
  } else if (is_abbrev(buf, "trapped")) {
    sb += "Trapped Containers\n\r";
    sb += "-------------------------------------\n\r";
    for (obj = object_list; obj; obj = obj->next) {
      TRealContainer *tc = dynamic_cast<TRealContainer *>(obj);
      if (tc && tc->isContainerFlag(CONT_TRAPPED)) {
        do_where_thing(this, tc, FALSE, sb);
      }
    }
  } else if (is_abbrev(buf, "zones")) {
    bottom = 0;
    sb += "Zone#    Name                                life  age     rooms      act Lvl\n\r";

    // Using argument which was returned from one_argument earlier in doShow
    // to add addition functionality for zone searching - Russ 11/07/98
    for (; isspace(*argument); argument++);

    unsigned int zone;
    for (zone = 0; zone < zone_table.size(); zone++) {
      zoneData &zd = zone_table[zone];
      if (!*argument ||
           isname(argument, zd.name) ||
           (!strcasecmp(argument, "disabled") && !zd.enabled)) {
        if (zd.enabled)
          strcpy(buf2, zd.name);
        else
          sprintf(buf2, "DISABLED: %s", zd.name);
       
        sprintf(buf, "%3d %-38.38s %4dm %4dm %6d-%-6d %3d %.1f\n\r", 
             zone, buf2, zd.lifespan, zd.age, bottom, zd.top, 
             zd.zone_value,
             (zd.num_mobs ? zd.mob_levels/zd.num_mobs : 0));
        sb += buf;
      }
      bottom = zd.top + 1;
    }
  } else if (is_abbrev(buf, "objects")) {
    if (!hasWizPower(POWER_SHOW_OBJ)) {
      sendTo("You lack the power to show obj information.\n\r");
      return;
    }
    if (*argument) {
      const char *tString;

      tString = argument;
      tString = one_argument(tString, buf);
      tString = one_argument(tString, buf2);

      if (*buf && is_abbrev(buf, "type")) {
        ubyte itemType = (unsigned char)(*buf2 ? atoi(buf2) : 0);

        if (!hasWizPower(POWER_SHOW_TRUSTED)) {
          sb += "VNUM  rnum   names\n\r";
        } else {
          sb += "VNUM  count max_exist str AC value names\n\r";
        }

        if (!*buf2 || !is_number(buf2) || top > MAX_OBJ_TYPES) {
          sendTo("Syntax: show objects type <type>\n\rSee OEDIT for item numbers.\n\r");
          return;
        }

        for (unsigned int objectIndex = 0;
             objectIndex < obj_index.size();
             objectIndex++) {
          if (obj_index[objectIndex].itemtype != itemType ||
              obj_index[objectIndex].virt < 0)
            continue;

          TObj *tObj = read_object(obj_index[objectIndex].virt, VIRTUAL);
          strcpy(buf, tObj->getNameForShow(false, true, this).c_str());

          if (!hasWizPower(POWER_SHOW_TRUSTED))
            sprintf(buf2, "%5d %5d   %s\n\r",
                    obj_index[objectIndex].virt, objectIndex, buf);
          else
            sprintf(buf2, "%5d %3d    %5d%c   %3d %2d %5d %s\n\r",
                    obj_index[objectIndex].virt, obj_index[objectIndex].number,
                    obj_index[objectIndex].max_exist,
                    (obj_index[objectIndex].value <= LIM_ITEM_COST_MIN ? ' ' : '*'),
                    obj_index[objectIndex].max_struct,
                    max(obj_index[objectIndex].armor, (sh_int) 0),
                    obj_index[objectIndex].value,
                    buf);

          sb += buf2;
        }

        if (desc)
          desc->page_string(sb.c_str(), 0, TRUE);

        return;
      }
    }

    int zone = -1;

    // If we gave no zone, lets Auto-select our present one.
    // Else find the one the player wants.
    if (*argument) {
      only_argument(argument, zonenum);
      sscanf(zonenum, "%i", &zone);
    } else
      zone = roomp->getZone();
    if ((zone < 0 || zone >= (signed int) zone_table.size()) && !*zonenum) {
      sb += "That is not a valid zone_number\n\r";
      if (desc)
        desc->page_string(sb.c_str(), 0, FALSE);

      return;
    }
    if (zone >= 0) {
      bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
      top = zone_table[zone].top;
    }
    if (!hasWizPower(POWER_SHOW_TRUSTED)) {
      sb += "VNUM  rnum   names\n\r";
    } else {
      sb += "VNUM  count max_exist str AC value names\n\r";
    }
    unsigned int objnx;
    for (objnx = 0; objnx < obj_index.size(); objnx++) {

      if (zone >= 0 && 
          (obj_index[objnx].virt < bottom || obj_index[objnx].virt > top) 
       || zone < 0 && !isname(zonenum, obj_index[objnx].name))
        continue;

      obj = read_object(obj_index[objnx].virt, VIRTUAL);
      sprintf(buf2, "%s", obj->getNameForShow(false, true, this).c_str());
      delete obj;

      if (!hasWizPower(POWER_SHOW_TRUSTED)) {
        sprintf(buf, "%5d %5d   %s\n\r", obj_index[objnx].virt, objnx, buf2);
      } else {
        sprintf(buf, "%5d %3d    %5d%c   %3d %2d %5d %s\n\r", 
              obj_index[objnx].virt, obj_index[objnx].number,
              obj_index[objnx].max_exist, 
              (obj_index[objnx].value <= LIM_ITEM_COST_MIN ? ' ' : '*'), 
               obj_index[objnx].max_struct,
               max(obj_index[objnx].armor, (sh_int) 0),
               obj_index[objnx].value,
              buf2);
      }
      sb += buf;
    }
  } else if (is_abbrev(buf, "maxed")) {
    if (!hasWizPower(POWER_SHOW_OBJ) || !hasWizPower(POWER_SHOW_TRUSTED)) {
      sendTo("You lack the power to show maxed obj information.\n\r");
      return;
    }

    sb += "VNUM  count max_exist str AC value names\n\r";

    unsigned int objnx;
    for (objnx = 0; objnx < obj_index.size(); objnx++) {
      if(obj_index[objnx].number<obj_index[objnx].max_exist) continue;
      obj = read_object(obj_index[objnx].virt, VIRTUAL);
      sprintf(buf2, "%s", obj->getNameForShow(false, true, this).c_str());
      delete obj;

      sprintf(buf, "%5d %3d    %5d%c   %3d %2d %5d %s\n\r", 
              obj_index[objnx].virt, obj_index[objnx].number,
              obj_index[objnx].max_exist, 
              (obj_index[objnx].value <= LIM_ITEM_COST_MIN ? ' ' : '*'), 
	      obj_index[objnx].max_struct,
	      max(obj_index[objnx].armor, (sh_int) 0),
	      obj_index[objnx].value,
              buf2);
      sb += buf;
    }
  } else if (is_abbrev(buf, "mobiles")) {
    if (!hasWizPower(POWER_SHOW_MOB)) {
      sendTo("You lack the power to show mob information.\n\r");
      return;
    }
    only_argument(argument, zonenum);

    if (is_abbrev(zonenum, "pets")) {
      sb += "Pet                            Master\n\r";
      sb += "-------------------------------------\n\r";
      for (b = character_list; b; b = b->next) {
        if (b->master && dynamic_cast<TMonster *>(b)) {
          sprintf(buf, "%-30s", b->getNameNOC(this).c_str());
          sb += buf;
          sprintf(buf, " %s%s\n\r",
              b->master->getNameNOC(this).c_str(), 
              b->master->isPc() ? " (PC)" : "");
          sb += buf;
        }  
      }
      if (desc)
        desc->page_string(sb.c_str(), 0, TRUE);

      return;
    } else if (is_abbrev(zonenum, "hunters")) {
      sb += "Hunting Mobs\n\r";
      sb += "-------------------------------------\n\r";
      for (b = character_list; b; b = b->next) {
        if (!b->isPc() && IS_SET(b->specials.act, ACT_HUNTING) && 
             (ch = b->specials.hunting)) {
          TMonster *tmons = dynamic_cast<TMonster *>(b);
          sprintf(buf, "%-20.20s (room: %5d)    %-20.20s (room: %5d) %7s\n\r", 
                        tmons->getName(), tmons->inRoom(),
                        ch->getName(), ch->inRoom(),
                        (tmons->Hates(ch, NULL) ? "(HATED)" : ""));
          sb += buf;
          sprintf(buf, "       persist: %d, range: %d, origin: %d\n\r",
              tmons->persist, tmons->hunt_dist,
              tmons->oldRoom);
          sb += buf;
        }
      }
      if (desc)
        desc->page_string(sb.c_str(), 0, TRUE);
      return;
    } else if (is_abbrev(zonenum, "response")) {
      sb += "Response Mobs\n\r";
      sb += "-------------------------------------\n\r";
      for (b = character_list; b; b = b->next) {
        if ((k = dynamic_cast<TMonster *>(b)) && 
            k->resps && k->resps->respList) {
          sprintf(buf, "%-30.30s (room: %5d)\n\r",
                      b->getName(), b->in_room);
          sb += buf;
        }
      }
      if (desc)
        desc->page_string(sb.c_str(), 0, TRUE);
      return;
    } else if (is_abbrev(zonenum, "bounty")) {
      struct bounty_hunt_struct *job;

      sb += "Bounty Hunter\n\r";
      sb += "-------------------------------------\n\r";
      for (b = character_list; b; b = b->next) {
        if (b->spec == SPEC_BOUNTY_HUNTER && b->act_ptr) {
          job = (bounty_hunt_struct *) b->act_ptr;
          if (job && job->hunted_item && *job->hunted_item)
            sprintf(buf, "%-30.30s (room: %5d)     item: %20.20s\n\r",
                        b->getName(), b->in_room, job->hunted_item);
          else if (job && job->hunted_victim && *job->hunted_victim)
            sprintf(buf, "%-30.30s (room: %5d)     vict: %20.20s\n\r",
                        b->getName(), b->in_room, job->hunted_victim);
          else
            sprintf(buf, "%-30.30s (room: %5d)     BOGUS\n\r", 
                        b->getName(), b->in_room);
          sb += buf;
        }
      }
      if (desc)
        desc->page_string(sb.c_str(), 0, TRUE);
      return;
    }

    if (*argument) {
      argument = one_argument(argument, buf);
      argument = one_argument(argument, buf2);

      if (*buf && is_abbrev(buf, "race")) {
        top = (*buf2 ? atoi(buf2) : -1);

        if (!hasWizPower(POWER_SHOW_TRUSTED)) {
          sb += "VNUM  level class aff names\n\r";
        } else {
          sb += "VNUM  max  count level class aff names\n\r";
        }

        if (!*buf2 || !is_number(buf2) || top < 0 || top > MAX_RACIAL_TYPES) {
          sendTo("Syntax: show mobiles race <race>\n\rSee HELP RACES for race numbers.\n\r");
          return;
        }

        for (unsigned int mobileIndex = 0;
             mobileIndex < mob_index.size();
             mobileIndex++) {
          if (mob_index[mobileIndex].race != top ||
              mob_index[mobileIndex].virt < 0)
            continue;

          if (!hasWizPower(POWER_SHOW_TRUSTED))
            sprintf(buf2, "%5d %3ld   %3ld  %3ld %s\n\r",
                    mob_index[mobileIndex].virt, mob_index[mobileIndex].level,
                    mob_index[mobileIndex].Class, mob_index[mobileIndex].faction,
                    mob_index[mobileIndex].name);
          else
            sprintf(buf2, "%5d %4d  %3d   %3ld   %3ld  %3ld %s\n\r",
                    mob_index[mobileIndex].virt, mob_index[mobileIndex].max_exist,
                    mob_index[mobileIndex].number, mob_index[mobileIndex].level,
                    mob_index[mobileIndex].Class, mob_index[mobileIndex].faction,
                    mob_index[mobileIndex].name);

          sb += buf2;
        }

        if (desc)
          desc->page_string(sb.c_str(), 0, TRUE);

        return;
      }
    }

    int zone = -1;

    if (!*zonenum)
      zone = roomp->getZone();
    else
      sscanf(zonenum, "%i", &zone);

    if ((zone < 0 || zone >= (signed int) zone_table.size()) && !*zonenum) {
      sb += "That is not a valid zone_number\n\r";
      if (desc)
        desc->page_string(sb.c_str(), 0, TRUE);
      return;
    }
    if (zone >= 0) {
      bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
      top = zone_table[zone].top;
    }
    if (!hasWizPower(POWER_SHOW_TRUSTED)) {
      sb += "VNUM  level class aff names\n\r";
    } else {
      sb += "VNUM  max  count level class aff names\n\r";
    }
    unsigned int objnx;
    for (objnx = 0; objnx < mob_index.size(); objnx++) {

      if (zone >= 0 && (mob_index[objnx].virt < bottom || mob_index[objnx].virt > top) || zone < 0 && !isname(zonenum, mob_index[objnx].name))
    continue;

      if (!hasWizPower(POWER_SHOW_TRUSTED)) {
        sprintf(buf, "%5d %3ld   %3ld  %3ld %s\n\r",
               mob_index[objnx].virt, mob_index[objnx].level,
               mob_index[objnx].Class, mob_index[objnx].faction,
               mob_index[objnx].name);
      } else {
        sprintf(buf, "%5d %4d  %3d   %3ld   %3ld  %3ld %s\n\r", 
               mob_index[objnx].virt, mob_index[objnx].max_exist,
               mob_index[objnx].number, mob_index[objnx].level, 
               mob_index[objnx].Class, mob_index[objnx].faction,
               mob_index[objnx].name);
      }
      sb += buf;
    }
  } else if (is_abbrev(buf, "rooms")) {
    only_argument(argument, zonenum);

    sb += "VNUM  rnum type         name [BITS]\n\r";
    if (is_abbrev(zonenum, "death"))
      room_iterate(room_db, print_death_room, sb, NULL);
    else if (is_abbrev(zonenum, "lit"))
      room_iterate(room_db, print_lit_room, sb, NULL);
    else if (is_abbrev(zonenum, "saverooms"))
      room_iterate(room_db, print_save_room, sb, NULL);
    else if (is_abbrev(zonenum, "hospital"))
      room_iterate(room_db, print_hospital_room, sb, NULL);
    else if (is_abbrev(zonenum, "noheal"))
      room_iterate(room_db, print_noheal_room, sb, NULL);
    else if (is_abbrev(zonenum, "private"))
      room_iterate(room_db, print_private_room, sb, NULL);
    else if (is_abbrev(zonenum, "noflee"))
      room_iterate(room_db, print_noflee_room, sb, NULL);
    else if (is_abbrev(zonenum, "arena"))
      room_iterate(room_db, print_arena_room, sb, NULL);
    else {
      int zone;
      if (1 != sscanf(zonenum, "%i", &zone))
        zone = roomp->getZone();
      if (zone < 0 || zone >= (signed int) zone_table.size())
        sb += "Zone number too high or too low.\n\r";
      else {
        struct show_room_zone_struct srzs;
        srzs.bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
        srzs.top = zone_table[zone].top;

        srzs.blank = 0;
        room_iterate(room_db, show_room_zone, sb, &srzs);

        if (srzs.blank) {
          sb += srzs.sb;
          sprintf(buf, "rooms %d-%d are blank.\n\r", srzs.startblank, srzs.lastblank);
          sb += buf;
          srzs.blank = 0;
        } else
          sb += srzs.sb;
      }
    }
  } else if (is_abbrev(buf, "materials")) {
    int matnum=-1, i;

    only_argument(argument, buf2);
    if(*buf2){  
      // one material
      matnum=atoi(buf2);
      describeMaterial(matnum);
    } else {
      // list materials
      sb += "Material Material\n\r";
      sb += "Number   Name\n\r";
      sb += "------------------\n\r";
      for(i=0;i<200;++i){
	if(material_nums[i].mat_name[0]){
	  buf[0]='\0';
	  sprintf(buf, "%-9i %s\n\r", i, 
		  good_uncap(material_nums[i].mat_name).c_str());
	  sb += buf;
	}
      }
    }
  } else if (is_abbrev(buf, "free")) {
    bool shFrError = false,
         isMobileF = true;
    unsigned long int shFrTotalCount = 0;

    for (; isspace(*argument); argument++);

    if (!*argument) {
      sb += "Syntax: show free <mob/obj> <#> <#>\n\r";
      shFrError = true;
    } else {
      argument = one_argument(argument, buf2); // get <mob/obj>

      if (!is_abbrev(buf2, "mobiles") && !is_abbrev(buf2, "objects")) {
        sb += "Syntax: show free <mob/obj> <#> <#>\n\r";
        shFrError = true;
      } else {
        if (is_abbrev(buf2, "objects"))
          isMobileF = false;

        for (; isspace(*argument); argument++);

        if (*argument) {
          argument = one_argument(argument, buf2); // get <all/#>

          if (is_abbrev(buf2, "all")) {
            if (!hasWizPower(POWER_SHOW_TRUSTED)) {
              sb += "Syntax: show free <mob/obj> <#> <#>\n\r";
              shFrError = true;
            } else {
              bottom = 0;
              top = zone_table.size() - 1;
            }
          } else {
            bottom = atoi(buf2);
            for (; isspace(*argument); argument++);

            if (!*argument)
              top = bottom;
            else {
              argument = one_argument(argument, buf2); // get 2nd <#>
              top = atoi(buf2);
            }
          }
        } else {
          if (!roomp) {
            vlogf(7, "show free called by being with no current room.");
            return;
          }

          top = bottom = roomp->getZone();
        }
      }
    }

    if (!shFrError && ((bottom < 0 || top > ((signed int) zone_table.size() - 1)) ||
                       top < bottom)) {
      sb += "Zone number incorrect.\n\r";
      shFrError = true;
    }

    if (!shFrError) {
      if (top == bottom)
        sprintf(buf2, "%d", top);
      else if (!hasWizPower(POWER_SHOW_TRUSTED))
        sprintf(buf2, "%d/%d", bottom, top);
      else
        sprintf(buf2, "%d,...,%d", bottom, top);

      sprintf(buf, "Showing Free %s Entires in Zone: %s\n\r--------------------\n\r",
              (isMobileF ? "Mobiles" : "Objects"), buf2);
      sb += buf;

      if (top == bottom)
        shFrTotalCount += showFreeMobObj(top, &sb, isMobileF);
      else if (!hasWizPower(POWER_SHOW_TRUSTED)) {
        shFrTotalCount += showFreeMobObj(top, &sb, isMobileF);
        shFrTotalCount += showFreeMobObj(bottom, &sb, isMobileF);
      } else
        for (int Runner = bottom; Runner < (top + 1); Runner++)
          shFrTotalCount += showFreeMobObj(Runner, &sb, isMobileF, true);

      sprintf(buf, "Total Count of %s: %lu\n\r",
              (isMobileF ? "Mobiles" : "Objects"), shFrTotalCount);
      sb += buf;
    }
  } else if (is_abbrev(buf, "created")) {
    char tString[256],
         tBuffer[256],
         tBuf[256];
    int  tTotalCount = 0;

    for (; isspace(*argument); argument++);

    string tStArgument(argument),
           tStType(""),
           tStItemType("");

    tStArgument = two_arg(tStArgument, tStType, tStItemType);

    if (tStType.empty() ||
        (!is_abbrev(tStType, "materialize") &&
         !is_abbrev(tStType, "spontaneous")))
      sb = "Syntax: show created <materialize/spontaneous>\n\r";
    else {
      int maxCost = (is_abbrev(tStType, "materialize") ? MATERIALIZE_PRICE : SPONT_PRICE),
          minCost = (is_abbrev(tStType, "materialize") ? -1 : MATERIALIZE_PRICE);

      if (is_abbrev(tStType, "materialize"))
        sb = "Materialize Objects:\n\r";
      else
        sb = "Spontaneous Generation Objects:\n\r";

      for (int tObjectIndex = 0; tObjectIndex < (signed) obj_index.size(); tObjectIndex++)
        if (!alchemy_create_deny(tObjectIndex) &&
	    obj_index[tObjectIndex].value <= maxCost &&
            obj_index[tObjectIndex].value >  minCost &&
            (tStItemType.empty() ||
             is_abbrev(tStItemType, ItemInfo[obj_index[tObjectIndex].itemtype]->name))) {
          strcpy(tBuffer, obj_index[tObjectIndex].short_desc);

          if (strlen(colorString(this, desc, tBuffer, NULL, COLOR_NONE, TRUE).c_str()) > 40) {
            tBuffer[38] = '\0';
            strcat(tBuffer, "...<z>");
          }

          // This corrects the 'have color code will misalign' problem.
          int factualSpace = strlen(tBuffer) - strlen(colorString(this, desc, tBuffer,
                                                                  NULL, COLOR_NONE,
                                                                  TRUE).c_str());

          sprintf(tBuf, "[%%5d] [%%4d] %%-%ds (%%s)\n\r", (40 + factualSpace));

          sprintf(tString, tBuf,
                  obj_index[tObjectIndex].virt, obj_index[tObjectIndex].value,
                  tBuffer,
                  ItemInfo[obj_index[tObjectIndex].itemtype]->name);
           sb += tString;
           tTotalCount++;
        }

      sprintf(tString, "Total Count: %d\n\r", tTotalCount);
      sb += "\n\r";
      sb += tString;

      tStArgument = tString;
      tStArgument += "\n\r";
      tStArgument += sb;
      sb = tStArgument;
    }
  } else if (is_abbrev(buf, "components")) {
    TComponent *tComponent = NULL;
    int         tValue = -1;
    char        tString[256],
                tBuffer[256];

    for (; isspace(*argument); argument++);

    if (*argument)
      for (int tCompIndex = 0; tCompIndex < (signed) CompInfo.size() && tValue == -1; tCompIndex++)
        if (discArray[CompInfo[tCompIndex].spell_num] &&
            is_abbrev(argument, discArray[CompInfo[tCompIndex].spell_num]->name)) {
          tValue = CompInfo[tCompIndex].spell_num;
          break;
        }

    sb = "Showing Component Information:\n\r";

    for (int tObjectIndex = 0; tObjectIndex < (signed) obj_index.size(); tObjectIndex++)
      if (obj_index[tObjectIndex].itemtype == ITEM_COMPONENT)
        if ((tComponent = dynamic_cast<TComponent *>(read_object(obj_index[tObjectIndex].virt, VIRTUAL)))) {
          if (tValue == -1 || tComponent->getComponentSpell() == tValue) {
            int tError = (tComponent->getComponentSpell() <= TYPE_UNDEFINED ? 0 :
                          (!discArray[tComponent->getComponentSpell()] ? -1 : 1));

            tBuffer[0] = '\0';

            if (tComponent->isComponentType(COMP_DECAY))
              strcat(tBuffer, "D");
            else
              strcat(tBuffer, " ");

            if (tComponent->isComponentType(COMP_POTION))
              strcat(tBuffer, "B");
            else
              strcat(tBuffer, " ");

            if (tComponent->isComponentType(COMP_SCRIBE))
              strcat(tBuffer, "S");
            else
              strcat(tBuffer, " ");

            sprintf(tString, "%25s [%2d][%s] %s\n\r",
                    (!tError ? "Undefined" : (tError == -1 ? "UNKNOWN/BOGUS" :
                     discArray[tComponent->getComponentSpell()]->name)),
                    tComponent->getComponentMaxCharges(),
                    tBuffer, tComponent->getName());
            sb += tString;

            // This little function has alot of slap to it.  We don't want
            // to call this 20+ times in rapid succession or we will probably
            // mimic 'where leather'.  Therefore we Only show it when the user
            // has requested a specific spell type.  This way we are really
            // limited to at Most 3 iterations.
            if (*argument)
              sb += showComponentTechnical(tComponent->objVnum());
          }

          delete tComponent;
          tComponent = NULL;
        }
  } else {
    sb += "Usage:\n\r";
    sb += "  show zones (<zonename> | \"disabled\")\n\r";
    sb += "  show objects (zone#|name)\n\r";
    sb += "  show mobiles (zone#|name|\"pets\"|\"hunters\"|\"bounty\"|\"response\")\n\r";

    sb += "  show free (mobiles|objects) (zone#/all) <zone#>\n\r";

    sb += "  show rooms (zone#)\n\r";
    sb += "  show rooms (\"death\"|\"saverooms\"|\"lit\"|\"noflee\"|\"private\"|\"hospital\"|\"noheal\")\n\r";
    sb += "  show <races | factions | trapped | fights >\n\r";
    sb += "  show materials (<material number>)\n\r";
    sb += "  show created <materialize/spontaneous> <itemtype>\n\r";
    sb += "  show components <spellname>\n\r";
  }

  if (desc)
    desc->page_string(sb.c_str(), 0, TRUE);
  return;
}



