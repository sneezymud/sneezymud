//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: other.cc,v $
// Revision 1.8  1999/09/29 07:46:14  lapsos
// Added code for the Mobile Strings stuff.
//
// Revision 1.7  1999/09/27 01:03:53  lapsos
// *** empty log message ***
//
// Revision 1.5  1999/09/27 00:07:47  lapsos
// Added atomize to doObjSpell function.
//
// Revision 1.4  1999/09/24 02:03:56  batopr
// Fixed array bounds problem on discArray
//
// Revision 1.3  1999/09/23 22:05:35  cosmo
// Simple change of victim to target to avoid null pointer
//
// Revision 1.2  1999/09/14 23:22:20  cosmo
// Fixed crash bug related to grouping npc's no victim->desc
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


//
///////////////////////////////////////////////////////////////////////////

extern "C" {
#include <unistd.h>
}

#include <algorithm>

#include "stdsneezy.h"
#include "range.h"
#include "statistics.h"
#include "combat.h"
#include "account.h"
#include "games.h"
#include "mail.h"
#include "drug.h"
#include "skillsort.h"

#include "disc_air.h"
#include "disc_alchemy.h"
#include "disc_earth.h"
#include "disc_fire.h"
#include "disc_water.h"
#include "disc_sorcery.h"
#include "disc_spirit.h"
#include "disc_wrath.h"
#include "disc_shaman.h"
#include "disc_aegis.h"
#include "disc_afflictions.h"
#include "disc_hand_of_god.h"
#include "disc_cures.h"
#include "disc_nature.h"

#include "spelltask.h"

void TBeing::doGuard(const char *argument)
{
  if (isPc()) {
    sendTo("Sorry, you can't just put your brain on autopilot!\n\r");
    return;
  }
  for (; isspace(*argument); argument++);

  if (!*argument) {
    if (IS_SET(specials.act, ACT_GUARDIAN)) {
      act("$n relaxes.", FALSE, this, 0, 0, TO_ROOM);
      sendTo("You relax.\n\r");
      REMOVE_BIT(specials.act, ACT_GUARDIAN);
    } else {
      SET_BIT(specials.act, ACT_GUARDIAN);
      act("$n alertly watches you.", FALSE, this, 0, master, TO_VICT);
      act("$n alertly watches $N.", FALSE, this, 0, master, TO_NOTVICT);
      sendTo("You snap to attention.\n\r");
    }
  } else {
    if (!strcasecmp(argument, "on")) {
      if (!IS_SET(specials.act, ACT_GUARDIAN)) {
        SET_BIT(specials.act, ACT_GUARDIAN);
        act("$n alertly watches you.", FALSE, this, 0, master, TO_VICT);
        act("$n alertly watches $N.", FALSE, this, 0, master, TO_NOTVICT);
        sendTo("You snap to attention.\n\r");
      }
    } else if (!strcasecmp(argument, "off")) {
      if (IS_SET(specials.act, ACT_GUARDIAN)) {
        act("$n relaxes.", FALSE, this, 0, 0, TO_ROOM);
        sendTo("You relax.\n\r");
        REMOVE_BIT(specials.act, ACT_GUARDIAN);
      }
    }
  }

  return;
}

void TObj::junkMe(TBeing *ch)
{
  if (obj_flags.cost > 0 && !isObjStat(ITEM_NEWBIE) && !isname("[prop]", name))
    ch->addToMoney(max(1, obj_flags.cost / 1000), GOLD_INCOME);
}

static void junkBeing(TBeing *ch, TThing *o, race_t ract)
{
  switch (ract) {
    case RACE_OGRE:
    case RACE_GIANT:
    case RACE_TROLL:
    case RACE_GOLEM:
    case RACE_MINOTAUR:
      act("You rip $p limb by limb and thrash the pieces.",
          TRUE, ch, o, NULL, TO_CHAR);
      act("$n rips $p limb by limb and thrashes the pieces.",
          TRUE, ch, o, NULL, TO_ROOM);
      ch->dropPool(10, LIQ_BLOOD);
      break;
    case RACE_DRAGON:
    case RACE_DINOSAUR:
    case RACE_LION:
    case RACE_LEOPARD:
    case RACE_BEAR:
    case RACE_TIGER:
    case RACE_COUGAR:
      act("Not wanting to waste good meat you devour $p.",
          TRUE, ch, o, NULL, TO_CHAR);
      act("$n devours $p without looking twice.",
          TRUE, ch, o, NULL, TO_ROOM);
      ch->dropPool(14, LIQ_BLOOD);
      break;
    case RACE_TYTAN:
      act("You crumple up $p and cast it over your shoulder.",
          TRUE, ch, o, NULL, TO_CHAR);
      act("$n crumples up $p and throws it over $s shoulder.",
          TRUE, ch, o, NULL, TO_ROOM);
      ch->dropPool(5, LIQ_BLOOD);
      break;
    default:
      if (ch->isImmortal()) {
        act("You throw $p into the air and disintegrate it.",
            TRUE, ch, o, NULL, TO_CHAR);
        act("$n throws $p into the air and disintegrates it.",
            TRUE, ch, o, NULL, TO_ROOM);
      } else {
        act("You trash $p, getting rid of any evidence.",
                  TRUE, ch, o, NULL, TO_CHAR);
        act("$n trashes $p, getting rid of any evidence.",
            TRUE, ch, o, NULL, TO_ROOM);
      }
      break;
  }
}

int TBeing::doJunk(const char *argument, TObj *obj)
{
  char arg[100], newarg[100];
  TObj *o;
  int num, p, count;
  TThing *t;

  only_argument(argument, arg);
  if (!*arg && !obj) {
    sendTo("Junk what?\n\r");
    return FALSE;
  }
  if (!obj) {
    if (getall(arg, newarg)) {
      num = -1;
      strcpy(arg, newarg);
    } else if ((p = getabunch(arg, newarg))) {
      num = p;
      strcpy(arg, newarg);
    } else
      num = 1;
  } else 
    num = 1;

  count = 0;
  while (num != 0) {
    o = obj;
    TThing *t_o = NULL;
    if (!o) {
      t_o = searchLinkedListVis(this, arg, stuff);
      o = dynamic_cast<TObj *>(t_o);
    }
    if (o) {
      if (o->isObjStat(ITEM_NODROP)) {
        sendTo("You can't let go of it, it must be CURSED!\n\r");
        return FALSE;
      }
      if (o->isPersonalized()) {
        sendTo("Monogrammed items can't be junked.\n\r");
        return FALSE;
      }
      if (o->stuff && desc && (desc->autobits & AUTO_POUCH)) {
        sendTo("There is still stuff in there, you choose not to junk it.\n\r");
        return FALSE;
      }
      for (t = o->stuff; t; t = t->nextThing) {
        TObj *tobj = dynamic_cast<TObj *>(t);
        if (!tobj)
          continue;
        if (tobj->isObjStat(ITEM_NODROP)) {
          sendTo("You can't let go of it, something inside it must be CURSED!\n\r");
          return FALSE;
        }
        if (tobj->isPersonalized()) {
          sendTo("There is a monogrammed item inside it which can't be junked.\n\r");
          return FALSE;
        }
      }
      o->junkMe(this);
      
      logItem(o, CMD_JUNK);
      for (t = o->stuff; t; t = t->nextThing) 
        logItem(t, CMD_JUNK);

      --(*o);
      if (num > 0)
        num--;

      if (dynamic_cast<TBaseCorpse *>(o)) {
        junkBeing(this, o, getRace());
      } else {
        switch(o->getMaterial()) {
          case MAT_CLOTH:
          case MAT_SILK:
          case MAT_WOOL:
          case MAT_FUR:
          case MAT_FUR_CAT:
          case MAT_FUR_DOG:
          case MAT_FUR_RABBIT:
          case MAT_GHOSTLY:
          case MAT_DWARF_LEATHER:
          case MAT_SOFT_LEATHER:
            act("You tear up $p then junk the strips.",
                TRUE, this, o, NULL, TO_CHAR);
            act("$n tears up $p then junks the strips.",
                TRUE, this, o, NULL, TO_ROOM);
            break;
          case MAT_PAPER:
            act("You crumple up $p then cast it over your shoulder.",
                TRUE, this, o, NULL, TO_CHAR);
            act("$n crumples up $p then throws it over $s shoulder.",
                TRUE, this, o, NULL, TO_ROOM);
          break;
          case MAT_GLASS:
          case MAT_CORAL:
          case MAT_ICE:
            if (this->roomp->isAirSector()) {
              act("You thrown $p down to it's doom below.",
                  TRUE, this, o, NULL, TO_CHAR);
              act("$n throws $p to it's doom below.",
                  TRUE, this, o, NULL, TO_ROOM);
            } else if (this->roomp->isVertSector()) {
              act("You smash $p against the wall.",
                  TRUE, this, o, NULL, TO_CHAR);
              act("$n smashes $p against the wall.",
                  TRUE, this, o, NULL, TO_ROOM);
            } else {
              act("You throw $p to the ground, shattering it.",
                  TRUE, this, o, NULL, TO_CHAR);
              act("$n throws $p to the ground, shattering it.",
                  TRUE, this, o, NULL, TO_ROOM);
            }
          break;
          case MAT_POWDER:
            act("You cast $p to the wind, scattering it in the breeze.",
                TRUE, this, o, NULL, TO_CHAR);
            act("$n casts $p to the wind, scattering it in the breeze.",
                TRUE, this, o, NULL, TO_ROOM);
          break;
          default:
            act("You junk $p.", TRUE, this, o, NULL, TO_CHAR);
            act("$n junks $p.", TRUE, this, o, NULL, TO_ROOM);
        }
      }
      count++;
      if (!obj) {
        delete o;
        o = NULL;
      }
    } else if (t_o) {
      // a non obj, probably a being

      TBeing *tb = dynamic_cast<TBeing *>(t_o);
      if (tb) {
        junkBeing(this, t_o, getRace());
      } else {
        act("You junk $p.", TRUE, this, t_o, NULL, TO_CHAR);
        act("$n junks $p.", TRUE, this, t_o, NULL, TO_ROOM);
      }

      if (num > 0)
        num--;
      count++;

      delete t_o;
      t_o = NULL;
    } else
      break;
  }
  if (count) 
    return DELETE_ITEM;
  else {
    sendTo("You don't have anything like that.\n\r");
    return FALSE;
  }
}

void TBeing::doCommand(const char *arg)
{
  char buf[256];

  if (!desc)
    return;

  for (; isspace(*arg); arg++);

  if (*arg && arg[0] != '-') {
    sendTo("Syntax: command (-<letter(s)>)\n\r");
    sendTo("To see the commands that start with certain letters\n\r");
    sendTo("Put the letters in place of <letter(s)> in the above syntax.\n\r");
    sendTo("For example to see the commands that start with a, g, and b,\n\r");
    sendTo("The command would be : command -abg\n\r");
    sendTo("Using no argument will show all commands.\n\r");
    return;
  }

  unsigned int i;
  vector<string>cmdVec(0);

  // shove everything into the vector
  for (i = 0; i < MAX_CMD_LIST; i++) {
    if (!commandArray[i])
      continue;
    // skip some commands we want to "hide"
    if (commandArray[i]->minLevel > GetMaxLevel())
      continue;
    if (i == CMD_AS && !isImmortal())
      continue;
    if (*arg && !strchr(arg, commandArray[i]->name[0]))
      continue;
    cmdVec.push_back(commandArray[i]->name);
  }

  // we are just sorting alphabetically, so no need for any special sort
  // routine
  sort(cmdVec.begin(), cmdVec.end());

  unsigned int num;
  string str;
  str = "The following commands are available:\n\r\n\r";

  for (num = 0; num < cmdVec.size(); num++) {
    sprintf(buf, "%-11s", cmdVec[num].c_str());
    str += buf;
    if ((num%7) == 6)
      str += "\n\r";
  }

  sprintf(buf, "\n\r\n\rTotal number of commands: %u\n\r", cmdVec.size());
  str += buf;

  desc->page_string(str.c_str(), 0);
}

void TBeing::doSplit(const char *argument, bool tell)
{
  char buf[256];
  int no_members;
  int tmp_amount;
  TBeing *k;
  followData *f;

  if (checkBlackjack()) {
    gBj.Split(this, argument, 0);
    return;
  }

  if (!isPc())
    return;

  one_argument(argument, buf);

  if (is_number(buf)) {
    int amount = 0;
    amount = atoi(buf);
    if (amount < 0) {
      sendTo("Sorry, you can't do that!\n\r");
      return;
    }
    if (!(k = master))
      k = this;

    if (inGroup(k) && sameRoom(k))
      no_members = ((k->desc) ? k->desc->session.group_share : 
            ((k->isPet() || k->isMount() || dynamic_cast<TMonster *>(k)) ? 0 : 1));
    else
      no_members = 0;


    for (f = k->followers; f; f = f->next)
      if (inGroup(f->follower) && sameRoom(f->follower))
        no_members += (f->follower->desc ? f->follower->desc->session.group_share : ((f->follower->isPet() || f->follower->isMount() || dynamic_cast<TMonster *>(f->follower)) ? 0 : 1));

    if ((no_members <= 1) || !isAffected(AFF_GROUP)) {
      // the auto-split logic kicks us in here
      // for some cases when it is not necessary to split
      // e.g. grouped with my pet
      // don't show the error unless they physically typed split
      if (tell)
        sendTo("Split your talens with whom?\n\r");

      return;
    }
    if (getMoney() < amount && !hasWizPower(POWER_WIZARD)) {
      sendTo("Sure...Giving away money you don't have is going to make you popular!\n\r");
      return;
    }
    if (!hasWizPower(POWER_WIZARD)) {
      if (desc)
        tmp_amount = amount * (no_members - desc->session.group_share) / no_members;
      else if (isPet() || isMount() || dynamic_cast<TMonster *>(this))
        tmp_amount = amount;
      else
        tmp_amount = amount * (no_members - 1) / no_members;

      addToMoney(-tmp_amount, GOLD_XFER);
    }

    if (k->isAffected(AFF_GROUP) && sameRoom(k) && k != this) {
      if (k->desc)
        tmp_amount = amount * (k->desc->session.group_share) / no_members;
      else if (k->isPet() || k->isMount() || dynamic_cast<TMonster *>(k))
        tmp_amount = 0;
      else
        tmp_amount = amount * (1) / no_members;

#if 0
      // too easy, find something better
      // maybe a timer to prevent overuse?
      reconcileHelp(k, (double) tmp_amount/100000);
#endif

      k->addToMoney(tmp_amount, GOLD_XFER);
       if (k->getPosition() >= POSITION_RESTING) 
        k->sendTo(COLOR_MOBS, "%s splits %d talens, and you receive %d of them.\n\r",
                    getName(), amount, tmp_amount);
    }
    for (f = k->followers; f; f = f->next) {
      if (f->follower->isAffected(AFF_GROUP) && sameRoom(f->follower) && f->follower != this) {
        if (f->follower->desc)
          tmp_amount = amount * (f->follower->desc->session.group_share) / no_members;
        else if (f->follower->isPet() || f->follower->isMount() || dynamic_cast<TMonster *>(f->follower))
          tmp_amount = 0;  // pets
        else
          tmp_amount = amount / no_members;

#if 0
        reconcileHelp(f->follower, (double) tmp_amount/100000);
#endif

        f->follower->addToMoney(tmp_amount, GOLD_XFER);
        if (f->follower->getPosition() >= POSITION_RESTING)
          f->follower->sendTo(COLOR_MOBS, "%s splits %d talens, and you receive %d of them.\n\r", getName(), amount, tmp_amount);
      }
    }
    sendTo("%d talens divided in %d shares of %d talens.\n\r", amount, no_members, amount / no_members);
    if ((GetMaxLevel() > MAX_MORT) && (no_members > 1))
      vlogf(9, "%s was kind enough to share %d talens with others...", getName(), amount);
  } else {
    int count = 0;
    TThing *obj2;
    if ((!(obj2 = searchLinkedListVis(this, buf, stuff, &count)) &&
         !(obj2 = searchLinkedListVis(this, buf, roomp->stuff, &count))) ||
         !obj2->splitMe(this, argument)) {
      sendTo("Pardon? Split WHAT?\n\r");
      return;
    }
  }
}

void TBeing::doReport(const char *argument)
{
  char info[256];
  char target[80];
  TThing *t, *t2;
  int found = 0;
  TBeing *targ = NULL;

  if (!roomp) {
    vlogf(10, "Person %s in bad room in doReport!", getName());
    return;
  }
  if (applySoundproof())
    return;

  if (!canSpeak()) {
    sendTo("You can't make a sound!\n\r");
    return;
  }
  memset(target, '\0', sizeof(target));
  only_argument(argument, target);
  if (hasClass(CLASS_CLERIC) || hasClass(CLASS_DEIKHAN))
    sprintf(info, "$n reports '%s%.1f%% H, %.2f%% P. I am %s%s'",
           red(), getPercHit(), getPiety(),
           DescMoves((((double) getMove()) / ((double) moveLimit()))),
           norm());
  else
    sprintf(info, "$n reports '%s%.1f%% H, %.1f%% M. I am %s%s'", 
           red(), getPercHit(), getPercMana(), 
           DescMoves((((double) getMove()) / ((double) moveLimit()))), 
           norm());


  for (t = roomp->stuff; t; t = t2) {
    t2 = t->nextThing;
    TBeing *tb = dynamic_cast<TBeing *>(t);
    if (!tb) continue;
    if (!found && tb->desc && target) {
      if (is_abbrev(target, tb->getName())) {
          found = TRUE;
          targ = tb;
      }
    }
    if (!tb->isPc())
      dynamic_cast<TMonster *>(tb)->aiSay(this, 0);
  }
  if (found) {
    if ((this == targ)) {
      sendTo("You try to report your status to yourself.\n\r");
      return;
    }
    sendTo(COLOR_MOBS, "You report your status to %s.\n\r", targ->getName());
    if (hasClass(CLASS_CLERIC) || hasClass(CLASS_DEIKHAN))
      sprintf(info, "<G>$n directly reports to you  '%s%.1f%% H, %.2f%% P. I am %s%s'<1>",
           red(), getPercHit(), getPiety(),
           DescMoves((((double) getMove()) / ((double) moveLimit()))),
           norm());
    else
      sprintf(info, "<G>$n directly reports to you '%s%.1f%% H, %.1f%% M. I am %s%s'<1>",
           red(), getPercHit(), getPercMana(),
           DescMoves((((double) getMove()) / ((double) moveLimit()))),
           norm());

    colorAct(COLOR_COMM, info, FALSE, this, 0, targ, TO_VICT);
    disturbMeditation(targ);
  } else {
    act(info, FALSE, this, 0, 0, TO_ROOM);
    sendTo("You report your status to the room.\n\r");
  }
}


void TBeing::doTitle(const char *)
{
  sendTo("Mobs don't have titles.\n\r");
  return;
}

void TPerson::doTitle(const char *argument)
{
  if (GetMaxLevel() < 5) {
    sendTo("You must be level 5 before you can change your title.\n\r");
    return;
  }

  for (; isspace(*argument); argument++);

  if (*argument) {
    if (strlen(argument) > 79) {
      sendTo("Title size is limited to 80 or less characters.\n\r");
      return;
    }
    string str = argument;

    // Basic name sake checks
    if (str.find("<n>") == string::npos &&
        colorString(this, desc, argument, NULL, COLOR_NONE, TRUE).find(getNameNOC(this).c_str()) ==
        string::npos) {
      sendTo("Your %s or <n> Must appear somewhere in here.\n\r", getNameNOC(this).c_str());
      return;
    }

    // keep morts from using flash in their titles
    if (!isImmortal()) {
      while (str.find("<f>") != string::npos) {
        str.replace(str.find("<f>"), 3, "");
      }
      while (str.find("<F>") != string::npos) {
        str.replace(str.find("<F>"), 3, "");
      }
    }
    delete [] title;
    title = mud_str_dup(str.c_str());

    sendTo("Your title has been set to : <%s>\n\r", str.c_str());
  } else {
    sendTo("The best title is the original anyway right? :)\n\r");
    setTitle(true);
  }
}

void TBeing::doQuit()
{
  sendTo("%sQuitting%s from the game will make you lose %sALL%s equipment and talens.\n\r",
           red(), norm(), blue(), norm());
  sendTo("If you are sure you would like to do this, type %squit!%s instead of quit.\n\r",
           blue(), norm());
}

int TMonster::doQuit2()
{
  return FALSE;
}

int TPerson::doQuit2()
{
  int rc;

  if (!desc || isAffected(AFF_CHARM))
    return FALSE;

  if (fight()) {
    sendTo("No way! You are fighting.\n\r");
    return FALSE;
  }
  if (getPosition() < POSITION_STUNNED) {
    sendTo("You die before your time!\n\r");
    vlogf(5, "%s killed by quitting while incapacitated at %s (%d)",
          getName(), roomp->getName(), inRoom());
    rc = die(DAMAGE_NORMAL);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  fullscreen();
  cls();

  act("Goodbye, friend.. Come back soon!", FALSE, this, 0, 0, TO_CHAR);
  act("$n has left the game.", TRUE, this, 0, 0, TO_ROOM);
  vlogf(0, "%s quit the game.", getName());

  if (!isImmortal() && getMoney()) {
    *roomp += *create_money(getMoney());
    addToMoney(-getMoney(), GOLD_INCOME);
  }

  // this handles droping items to ground
  // it use to be in ~TPerson, but do it here since the one in ~TPerson is
  // now an error handler, while this is a known good case.
  dropItemsToRoom(SAFE_YES, DROP_IN_ROOM);

  doSave(SILENT_NO);
  removeRent();
  removeFollowers();

  return DELETE_THIS;
}

void TBeing::verifyWeightVolume()
{
  // there is an intrinsic round off error present in float
  // in a get all (or similar) situation, this can propigate to a sizeable
  // amount throwing off the weight.  This function will reset the values
  // appropriately.

  float rw = 0.0, bw;
  int rv = 0, bv;
  TThing *t, *t2;
  for (t = stuff; t; t = t->nextThing) {
    bv = 0;
    bw = 0.0;
    for (t2 = t->stuff; t2; t2 = t2->nextThing) {
      // bypass comps in spellbag
      if (dynamic_cast<TComponent *>(t2) &&
          dynamic_cast<TSpellBag *>(t))
        continue;
      bw += t2->getTotalWeight(TRUE);
      bv += t2->getTotalVolume();
    }
    if (compareWeights(bw, t->getCarriedWeight()) != 0) {
      // bag weighs different then sum of all stuff in it
      // skip volume check since expansion can be screwy
      t->setCarriedWeight(bw);
    }
    rw += t->getTotalWeight(TRUE);
    rv += t->getTotalVolume();
  }
  if ((compareWeights(rw, getCarriedWeight()) != 0) || 
      rv != getCarriedVolume()) {
    setCarriedWeight(rw);
    setCarriedVolume(rv);
  }
}

void TBeing::doNotHere() const
{
  sendTo("Sorry, but you cannot do that here!\n\r");
}

static const string describe_practices(const TBeing *ch)
{
  char buf[1024];

  *buf = '\0';

  if (ch->hasClass(CLASS_MAGIC_USER) || ch->practices.mage)
    sprintf(buf + strlen(buf), "You have %d mage practice%s left.\n\r", 
          ch->practices.mage, ((ch->practices.mage == 1) ? "" : "s"));
  if (ch->hasClass(CLASS_CLERIC) || ch->practices.cleric)
    sprintf(buf + strlen(buf), "You have %d cleric practice%s left.\n\r",
          ch->practices.cleric, ((ch->practices.cleric== 1) ? "" : "s"));
  if (ch->hasClass(CLASS_WARRIOR) || ch->practices.warrior)
    sprintf(buf + strlen(buf), "You have %d warrior practice%s left.\n\r",
          ch->practices.warrior, ((ch->practices.warrior== 1) ? "" : "s"));
  if (ch->hasClass(CLASS_THIEF) || ch->practices.thief)
    sprintf(buf + strlen(buf), "You have %d thief practice%s left.\n\r",
          ch->practices.thief, ((ch->practices.thief== 1) ? "" : "s"));
  if (ch->hasClass(CLASS_DEIKHAN) || ch->practices.deikhan)
    sprintf(buf + strlen(buf), "You have %d deikhan practice%s left.\n\r",
          ch->practices.deikhan, ((ch->practices.deikhan == 1) ? "" : "s"));
  if (ch->hasClass(CLASS_RANGER) || ch->practices.ranger)
    sprintf(buf + strlen(buf), "You have %d ranger practice%s left.\n\r",
          ch->practices.ranger, ((ch->practices.ranger == 1) ? "" : "s"));
  if (ch->hasClass(CLASS_MONK) || ch->practices.monk)
    sprintf(buf + strlen(buf), "You have %d monk practice%s left.\n\r",
          ch->practices.monk, ((ch->practices.monk == 1) ? "" : "s"));
  if (ch->hasClass(CLASS_SHAMAN) || ch->practices.shaman)
    sprintf(buf + strlen(buf), "You have %d shaman practice%s left.\n\r",
          ch->practices.shaman, ((ch->practices.shaman == 1) ? "" : "s"));

  return buf;
}

void TBeing::doPractice(const char *argument)
{
  char buf[MAX_STRING_LENGTH * 2];
  char arg[256];
  char skillbuf[40];
  int first, last;
  discNumT i;
  Descriptor *d;
  CDiscipline *cd;
  int classNum = 0;

  if (!(d = desc))
    return;

  if (polyed && !d->original->isImmortal()) {
    sendTo("Polymorph types can't use the practice Command. Bug if this is an error.");
    return;
  }
  for (; isspace(*argument); argument++);

  if (!argument || !*argument) {
    sprintf(buf, "The following disciplines are valid:\n\r");
    for (i=MIN_DISC; i < MAX_DISCS; i++) {
      cd = getDiscipline(i);
      if (cd && (isImmortal() || cd->ok_for_class)) {
        if (cd->getLearnedness()) {
          if (cd->getLearnedness() == cd->getNatLearnedness()) {
            sprintf(buf + strlen(buf), "%30s : Learnedness: %3d%%\n\r",
                disc_names[i], cd->getLearnedness());
          } else {
            sprintf(buf + strlen(buf), "%30s : Learnedness: Current (%3d%%) Natural (%3d%%)\n\r", disc_names[i], cd->getLearnedness(), cd->getNatLearnedness());
          }
        } else {
          sprintf(buf + strlen(buf), "%30s : Learnedness: Unlearned\n\r",
                  disc_names[i]);
        }
      }
    }
    sprintf(buf + strlen(buf), "%s", describe_practices(this).c_str());
    d->page_string(buf, 0);
    return;
  }
  argument = one_argument(argument, arg);
  if (is_abbrev(arg, "hth") &&
      (classNum = discNames[DISC_HTH].class_num) &&
      hasClass(classNum)) {
    sendSkillsList(DISC_HTH);
    return;
  }

  if (is_abbrev(arg, "class")) {
    int which = 0;
    argument = one_argument(argument, arg);
    if (is_abbrev(arg, "mage") ||
        is_abbrev(arg, "magicuser"))
      which = CLASS_MAGIC_USER;
    else if (is_abbrev(arg, "cleric"))
      which = CLASS_CLERIC;
    else if (is_abbrev(arg, "warrior"))
      which = CLASS_WARRIOR;
    else if (is_abbrev(arg, "thief"))
      which = CLASS_THIEF;
    else if (is_abbrev(arg, "deikhan"))
      which = CLASS_DEIKHAN;
    else if (is_abbrev(arg, "shaman"))
      which = CLASS_SHAMAN;
    else if (is_abbrev(arg, "ranger"))
      which = CLASS_RANGER;
    else if (is_abbrev(arg, "monk"))
      which = CLASS_MONK;
    else {
      sendTo("That is not a valid class.\n\r");
      sendTo("Syntax: practice class <class>.\n\r");
      return;
    }
    sprintf(buf, "The following disciplines are valid:\n\r");
    for (i=MIN_DISC; i < MAX_DISCS; i++) {
      if (!strcmp(disc_names[i], "unused")) 
        continue;
      if (!(cd = getDiscipline(i))) {
        vlogf(5, "Somehow %s was not assigned a discipline (%d), used prac class (%d).",getName(), i, which);
      }
      if ((discNames[i].class_num == 0) || (IS_SET(discNames[i].class_num, which))) {
        if (cd && cd->getLearnedness() >= 0) {
          sprintf(buf + strlen(buf), "%30s : (Learnedness: %3d%%)\n\r",
              disc_names[i], cd->getLearnedness());
        } else {
          sprintf(buf + strlen(buf), "%30s : (Learnedness: unlearned)\n\r",
                  disc_names[i]);
        }
      }
    }
    sprintf(buf + strlen(buf), "%s", describe_practices(this).c_str());
    d->page_string(buf, 0);
    return;
  }

  if (is_abbrev(arg, "discipline")) {
    argument = one_argument(argument, arg);
    if (!*arg) {
      sendTo("You need to specify a discipline: practice discipline <discipline> <class>.\n\r");
      return;
    } else {
      if (is_abbrev(arg, "hth")) {
        strcpy(arg, discNames[DISC_HTH].practice);
      }
      if (is_abbrev(arg, "fighting") || 
          is_abbrev(arg, "alchemy") || 
          is_abbrev(arg, "aegis") || 
          is_abbrev(arg, "wrath") || 
          is_abbrev(arg, "cures")) {
        for (; isspace(*argument); argument++);
        if ((classNum = getClassNum(argument, EXACT_NO))) {
          doPracDisc(arg, classNum);
        } else {
          sendTo("You need to specify a valid class when looking for that discipline.\n\r");
          sendTo("Syntax: practice discipline <discipline> <class>.\n\r");
        }
        return;
      } else {
        doPracDisc(arg, classNum);
        return;
      }
    }
    return;
  }

  if (is_abbrev(arg, "skill")) {
    for (; isspace(*argument); argument++);

    if (!*argument) 
      sendTo("You need to specify what skill: practice skill <\"skill\">.\n\r");
    else {
      if (strlen(argument) > 2 && is_abbrev(argument, "wizardry")) 
        doPracSkill(argument, SKILL_WIZARDRY);
      else if (strlen(argument) > 2 && is_abbrev(argument, "devotion")) 
        doPracSkill(argument, SKILL_DEVOTION);
      else 
        doPracSkill(argument, TYPE_UNDEFINED);
    }
    return; 
  }
  bool found = FALSE;
  bool match = FALSE;
  classNum = FALSE;
  first = last = TRUE;

  discNumT dnt = DISC_NONE;
  for (i=MIN_DISC; i < MAX_DISCS; i++) {
    strcpy(skillbuf, discNames[i].practice);
    classNum = discNames[i].class_num;
    if (is_abbrev(arg, skillbuf, MULTIPLE_YES)) {
      match = TRUE;
      if (classNum && !hasClass(classNum)) 
        continue;
      else {
        dnt = discNames[i].disc_num;
        found = TRUE;
        break;
      }     
    }
  }
  if (!found) {
    if (match) {
      sendTo("That discipline exists, but is not available to you.\n\r");
      sendTo("To find out about that discipline use prac discipline <discipline> <class>.\n\r");
    } else
      sendTo("Which discipline???\n\r");

    return;
  }
// new function to send skills lists to pc's
  sendSkillsList(dnt);
  return;
}

bool skillSorter::operator() (const skillSorter &x, const skillSorter &y) const
{
  int xgsv = x.ch->getSkillValue(x.theSkill);
  int ygsv = y.ch->getSkillValue(y.theSkill);

  if (xgsv > ygsv)
    return true;
  if (xgsv == ygsv &&
      discArray[x.theSkill]->start < discArray[y.theSkill]->start)
    return true;
  return false;
}

void TBeing::sendSkillsList(discNumT which)
{
  char buf[MAX_STRING_LENGTH * 2], buffer[MAX_STRING_LENGTH * 2];
  spellNumT i;
  Descriptor *d;
  CDiscipline *cd;
  discNumT das;

  if (!(d = desc))
    return;
  *buffer = '\0';

  sprintf(buf, "You have knowledge of these abilities:\n\r");
  strcat(buffer, buf);
  char how_long[160];

  vector<skillSorter>sortDiscVec(0);

  for (i = MIN_SPELL; i < MAX_SKILL; i++) {
    if (hideThisSpell(i))
      continue;
    if (discArray[i]->disc != which)
      continue;

    sortDiscVec.push_back(skillSorter(this, i));
  }

  // sort things into correct order
  sort(sortDiscVec.begin(), sortDiscVec.end(), skillSorter());

  unsigned int j;
  for (j = 0; j < sortDiscVec.size(); j++) {
    i = sortDiscVec[j].theSkill;
    das = getDisciplineNumber(i, FALSE);
    if (das == DISC_NONE) {
      vlogf(5, "Bad disc for skill %d in doPractice", i);
      continue;
    }
    cd = getDiscipline(das);

    // getLearnedness is -99 for an unlearned skill, make that seem like a 0
    int tmp_var = ((!cd || cd->getLearnedness() <= 0) ? 0 : cd->getLearnedness());
    tmp_var = min((int) MAX_DISC_LEARNEDNESS, tmp_var);

    if (cd && !cd->ok_for_class && getSkillValue(i) <= 0) 
      strcpy(how_long, "(Learned: Not in this Lifetime)");
    else if ((getSkillValue(i) <= 0) &&
          (!tmp_var || (discArray[i]->start - tmp_var) > 0) &&
          (i != SKILL_WIZARDRY && i != SKILL_DEVOTION)) {
      sprintf(how_long, "(Learned: %s)", 
          skill_diff(discArray[i]->start - tmp_var));
    } else if (discArray[i]->toggle && !hasQuestBit(discArray[i]->toggle)) {
      strcpy(how_long, "(Learned: When Teacher is Found)");
    } else if (i == SKILL_WIZARDRY) {
      wizardryLevelT wiz_lev = getWizardryLevel();
      if (wiz_lev < WIZ_LEV_COMP_PRIM_OTHER_FREE) {
        if (isRightHanded())
          sprintf(how_long, "(Learned: %s)\tcomponent=right hand, left hand=free", skill_diff(discArray[i]->start - tmp_var));
        else
          sprintf(how_long, "(Learned: %s)\tcomponent=left hand, right hand=free", skill_diff(discArray[i]->start - tmp_var));
      } else if (wiz_lev == WIZ_LEV_COMP_PRIM_OTHER_FREE) {
        if (isRightHanded())
          strcpy(how_long, "\tcomponent=right hand, left hand=free");
        else
          strcpy(how_long, "\tcomponent=left hand, right hand=free");
      } else if (wiz_lev == WIZ_LEV_COMP_EITHER_OTHER_FREE) {
        strcpy(how_long,   "\tcomponent=either hand, other free");
      } else if (wiz_lev == WIZ_LEV_COMP_EITHER) {
        strcpy(how_long, "\tcomponent=any hand");
      } else if (wiz_lev == WIZ_LEV_COMP_INV) {
        strcpy(how_long, "\tcomponent=any hand or inventory");
      } else if (wiz_lev == WIZ_LEV_NO_GESTURES) {
        strcpy(how_long, "\tcomponent=any hand or inventory; no gestures");
      } else if (wiz_lev == WIZ_LEV_NO_MANTRA) {
        strcpy(how_long, "\tcomponent=any hand or inventory; no speak; no gestures");
      } else if (wiz_lev >= WIZ_LEV_COMP_BELT) {
        strcpy(how_long, "\tcomponent=any hand, inventory or waist; no speak; no gestures");
      }
    } else if (i == SKILL_DEVOTION) {
      devotionLevelT wiz_lev = getDevotionLevel();
      if (wiz_lev < DEV_LEV_SYMB_PRIM_OTHER_FREE) {
        if (isRightHanded())
          sprintf(how_long, "(Learned: %s)\tsymbol=right hand, left hand=free", skill_diff(discArray[i]->start - tmp_var));
        else
          sprintf(how_long, "(Learned: %s)\tsymbol=left hand, right hand=free", skill_diff(discArray[i]->start - tmp_var));
      } else if (wiz_lev == DEV_LEV_SYMB_PRIM_OTHER_FREE) {
        if (isRightHanded())
          strcpy(how_long, "\tsymbol=right hand, left hand=free");
        else
          strcpy(how_long, "\tsymbol=left hand, right hand=free");
      } else if (wiz_lev == DEV_LEV_SYMB_EITHER_OTHER_FREE) {
        strcpy(how_long,   "\tsymbol=either hand, other free");
      } else if (wiz_lev == DEV_LEV_SYMB_PRIM_OTHER_EQUIP) {
        if (isRightHanded())
          strcpy(how_long, "\tsymbol=right hand, left hand=no restrictions; or symbol=left hand, right hand free");
        else
          strcpy(how_long, "\tsymbol=left hand, right hand=no restrictions; or symbol=right hand, left hand free");
      } else if (wiz_lev == DEV_LEV_SYMB_EITHER_OTHER_EQUIP) {
        strcpy(how_long, "\tsymbol=any hand, no restrictions");
      } else if (wiz_lev == DEV_LEV_SYMB_NECK) {
        strcpy(how_long, "\tsymbol=any hand or neck");
      } else if (wiz_lev == DEV_LEV_NO_GESTURES) {
        strcpy(how_long, "\tsymbol=any hand or neck; no gestures");
      } else if (wiz_lev >= DEV_LEV_NO_MANTRA) {
        strcpy(how_long, "\tsymbol=any hand or neck; pray silently; no gestures");
      }
    } else { 
      strcpy(how_long, " ");
    }
    if (!isImmortal()) {
      if (doesKnowSkill(i)) {
        if ((i == SKILL_WIZARDRY) || (i == SKILL_DEVOTION)) {
            sprintf(buf, "%s%-25.25s%s   Current: %-15s\n\r%-15s.\n\r",
                   cyan(), discArray[i]->name, norm(),
                   how_good(getSkillValue(i)), how_long);
        } else if (getMaxSkillValue(i) < MAX_SKILL_LEARNEDNESS) {
          if (discArray[i]->startLearnDo > 0) {
            sprintf(buf, "%s%-25.25s%s   Current: %-12s Potential: %-12s\n\r",
                 cyan(), discArray[i]->name, norm(), 
                 how_good(getSkillValue(i)),
                 how_good(getMaxSkillValue(i)));
          } else {
            sprintf(buf, "%s%-25.25s%s   Current: %-15s\n\r",
                   cyan(), discArray[i]->name, norm(), 
                   how_good(getSkillValue(i)));
          }
        } else {
          sprintf(buf, "%s%-25.25s%s   Current: %-15s\n\r",
            cyan(), discArray[i]->name, norm(), 
            how_good(getSkillValue(i)));
        }
      } else {
        sprintf(buf, "%s%-25.25s%s   %-25s\n\r", cyan(), discArray[i]->name, norm(), how_long);
      }
    } else {
      if (hasWizPower(POWER_GOD))
        sprintf(buf, "%s%-22.22s%s Disc:[%3d] SkNum:[%3d] Learn:[%3d/%2d/%2d] Diff:%s\n\r",
                cyan(), discArray[i]->name, norm(),
                mapDiscToFile(discNumT(getDisciplineNumber(i, FALSE))),
                i, discArray[i]->learn,
                discArray[i]->startLearnDo,
                discArray[i]->amtLearnDo,
                displayDifficulty(i).c_str());
      else
        sprintf(buf, "%s%-22.22s%s Potential: %-12s (%2d) Current: %-12s (%2d)\n\r", 
                cyan(), discArray[i]->name, norm(), 
                how_good(getMaxSkillValue(i)),
                getMaxSkillValue(i),
                how_good(getSkillValue(i)),
                getSkillValue(i));
    }
    if (strlen(buf) + strlen(buffer) > (MAX_STRING_LENGTH * 2) - 2)
      break;
    strcat(buffer, buf);
  } 
  strcat(buffer, describe_practices(this).c_str());
  d->page_string(buffer, 0);
}

void TBeing::doPracSkill(const char *argument, spellNumT skNum)
{
  spellNumT skill = TYPE_UNDEFINED;
  bool found = FALSE;
  int wiz = FALSE;
  char buf[256];
  char how_long[256];
  int tmp_var = FALSE;
  CDiscipline *cd;
  discNumT das;

  if (!*argument && skNum == TYPE_UNDEFINED) 
    return;

  if (skNum == SKILL_WIZARDRY) {
    if (hasClass(CLASS_MAGE) ||
	hasClass(CLASS_RANGER) ||
	hasClass(CLASS_SHAMAN)) {
      found=2;
      wiz = 1;
    } else {
      sendTo("You do not know about Wizardry.\n\r");
      found = 1;
    }
  } else if (skNum == SKILL_DEVOTION){
    if (hasClass(CLASS_CLERIC) || hasClass(CLASS_DEIKHAN)) {
      found = 2;
      wiz = 2;
    } else {
      sendTo("You do not know anything about Devotion.\n\r");
      found = 1;
    }
  } else if (skNum != TYPE_UNDEFINED) {
    vlogf(5, "Something is sending to doPracSkill with a bad argument.");
    sendTo("That does not appear to be a valid skill: practice skill <name>\n\r");
    return;
  } else {
    for(;*argument && isspace(*argument);argument++);

    for (spellNumT i = MIN_SPELL; (i < MAX_SKILL); i++) {
      if (hideThisSpell(i))
        continue;
      
      if (!(is_abbrev(argument, discArray[i]->name, MULTIPLE_YES))) 
        continue;
      else {
//        skill = getSkillNum(i);
        skill = i;
        if (!doesKnowSkill(skill)) {
          found = 1;
          continue;
        } else {
          found = 2;
          //break;
          sprintf(buf, "%s%-25.25s%s   Current: %-12s Potential: %-12s\n\r", cyan(), discArray[skill]->name, norm(), how_good(getSkillValue(skill)), how_good(getMaxSkillValue(skill)));
          sendTo(COLOR_BASIC, buf);
        } 
      }
    } 
  } 
  if (wiz && !(found == 1)) {
    sprintf(buf, "%s%-25.25s%s   Current: %-12s Potential: %-12s\n\r", cyan(), discArray[skNum]->name, norm(), how_good(getSkillValue(skNum)), how_good(getMaxSkillValue(skNum)));
     sendTo(COLOR_BASIC, buf);
  } else if (found == 2 || (skill > TYPE_UNDEFINED)) {
    //sprintf(buf, "%s%-25.25s%s   Current: %-12s Potential: %-12s\n\r", cyan(), discArray[skill]->name, norm(), how_good(getSkillValue(skill)), how_good(getMaxSkillValue(skill))); 
    //sendTo(COLOR_BASIC, buf);
    return;
  } else if (found == 1 && skill > TYPE_UNDEFINED) {
    sendTo("Skill %s: Unlearned. You can only use practice skill about skills that you have learned.\n\r", discArray[skill]->name);
    return;
  } else if (!wiz) {
    sendTo("That does not appear to be a valid skill: practice skill <name>\n\r");
    return;
  } 

  das = getDisciplineNumber(skNum, FALSE);
  if (das == DISC_NONE) {
    vlogf(5, "Bad disc for skill %d in doPracSkill", skNum);
    return;
  }
  cd = getDiscipline(das);
  tmp_var = ((!cd || cd->getLearnedness() <= 0) ? 0 : cd->getLearnedness());
  tmp_var = max((int) MAX_DISC_LEARNEDNESS, tmp_var);

  if (wiz == 1) {
    wizardryLevelT wiz_lev = getWizardryLevel();
    if (wiz_lev < WIZ_LEV_COMP_PRIM_OTHER_FREE) {
      if (isRightHanded()) {
        sprintf(how_long, "(Learned: %s)\tcomponent=right hand, left hand=free.\n\r", skill_diff(discArray[skNum]->start - tmp_var));
      } else {
        sprintf(how_long, "(Learned: %s)\tcomponent=left hand, right hand=free.\n\r", skill_diff(discArray[skNum]->start - tmp_var));
      }
      sendTo(COLOR_BASIC, how_long);
      return;
    } else if (wiz_lev == WIZ_LEV_COMP_PRIM_OTHER_FREE) {
      if (isRightHanded())
        strcpy(how_long, "\tcomponent=right hand, left hand=free.\n\r");
      else
        strcpy(how_long, "\tcomponent=left hand, right hand=free.\n\r");
    } else if (wiz_lev == WIZ_LEV_COMP_EITHER_OTHER_FREE) {
      strcpy(how_long,   "\tcomponent=either hand, other free.\n\r");
    } else if (wiz_lev == WIZ_LEV_COMP_EITHER) {
      strcpy(how_long, "\tcomponent=any hand.\n\r");
    } else if (wiz_lev == WIZ_LEV_COMP_INV) {
      strcpy(how_long, "\tcomponent=any hand or inventory.\n\r");
    } else if (wiz_lev == WIZ_LEV_NO_GESTURES) {
      strcpy(how_long, "\tcomponent=any hand or inventory; no gestures.\n\r");
    } else if (wiz_lev == WIZ_LEV_NO_MANTRA) {
      strcpy(how_long, "\tcomponent=any hand or inventory; no speak; no gestures.\n\r");
    } else if (wiz_lev >= WIZ_LEV_COMP_BELT) {
      strcpy(how_long, "\tcomponent=any hand, waist or waist; no speak; no gestures.\n\r");
    }
    sendTo(COLOR_BASIC, how_long);

  } else if (wiz == 2) {
    devotionLevelT wiz_lev = getDevotionLevel();
    if (wiz_lev < DEV_LEV_SYMB_PRIM_OTHER_FREE) {
      if (isRightHanded()) {
        sprintf(how_long, "(Learned: %s)\tsymbol=right hand, left hand=free.\n\r", skill_diff(discArray[skNum]->start - tmp_var));
      } else {
        sprintf(how_long, "(Learned: %s)\tsymbol=left hand, right hand=free.\n\r", skill_diff(discArray[skNum]->start - tmp_var));
      }
      sendTo(COLOR_BASIC, how_long);
      return;
    } else if (wiz_lev == DEV_LEV_SYMB_PRIM_OTHER_FREE) {
      if (isRightHanded())
        strcpy(how_long, "\tsymbol=right hand, left hand=free.\n\r");
      else
        strcpy(how_long, "\tsymbol=left hand, right hand=free.\n\r");
    } else if (wiz_lev == DEV_LEV_SYMB_EITHER_OTHER_FREE) {
      strcpy(how_long,   "\tsymbol=either hand, other free.\n\r");
    } else if (wiz_lev == DEV_LEV_SYMB_PRIM_OTHER_EQUIP) {
      strcpy(how_long, "\tsymbol=any hand, primary hand free.\n\r");
    } else if (wiz_lev == DEV_LEV_SYMB_EITHER_OTHER_EQUIP) {
      strcpy(how_long, "\tsymbol=any hand, no restrictions.\n\r");
    } else if (wiz_lev == DEV_LEV_SYMB_NECK) {
      strcpy(how_long, "\tsymbol=any hand or neck.\n\r");
    } else if (wiz_lev == DEV_LEV_NO_GESTURES) {
      strcpy(how_long, "\tsymbol=any hand or neck; no gestures.\n\r");
    } else if (wiz_lev >= DEV_LEV_NO_MANTRA) {
      strcpy(how_long, "\tsymbol=any hand or neck; pray silently; no gestures.\n\r");
    }
    sendTo(COLOR_BASIC, how_long);
  }
  return;
}

void TBeing::doPracDisc(const char *arg, int classNum)
{
//  char buf[256];

  if (!*arg) {
    return;
  }

//  arg = one_argument(arg, buf);

  for (discNumT i = MIN_DISC; (i < MAX_DISCS); i++) {
    if (!*disc_names[i] || !(*discNames[i].practice)) {
      continue;
    }
    if (!is_abbrev(arg, discNames[i].practice, MULTIPLE_YES)) {
      continue;
    } else {
      if (classNum) {
        if (!IS_SET(discNames[i].class_num, classNum) && (discNames[i].class_num != 0)) {
          continue;
        }
        sendSkillsList(i);
        return;
      } else {
        sendSkillsList(i);
        return;
      } 
    }
  }
  sendTo("You must specify a valid discipline and class.\n\r");
  sendTo("Syntax: practice discipline <discipline> <class>.\n\r");
}


void TBeing::doIdea(const char *)
{
  sendTo("Monsters can't have ideas - Go away.\n\r");
  return;
}

void TPerson::doIdea(const char *arg)
{
  char buf[256];
 
  one_argument(arg, buf);

  if (fight())  {
    sendTo("You cannot perform that action while fighting!\n\r");
    return;
  }
  if (hasWizPower(POWER_SEE_COMMENTARY) && isImmortal() && *buf) {
    desc->start_page_file(IDEA_FILE, "Players aint saying nothin\'.\n\r");
    return;
  }
  idea_used_num++;
  if (!desc->client)
    sendTo("Write the subject of your idea then hit return.\n\r");

  if (!desc->client) {
    addPlayerAction(PLR_BUGGING);
    desc->connected = CON_WRITING;
    strcpy(desc->name, "Idea");
    desc->str = new (char *);
    *desc->str = new char[1];
    *(*desc->str) = '\0';
    desc->max_str = MAX_MAIL_SIZE;
  }
  desc->clientf("%d", CLIENT_IDEA);
}

void TBeing::doTypo(const char *)
{
  sendTo("Monsters can't spell - leave me alone.\n\r");
  return;
}

void TPerson::doTypo(const char *arg)
{
  char buf[256];
 
  one_argument(arg, buf);

  if (fight())  {
    sendTo("You cannot perform that action while fighting!\n\r");
    return;
  }
  if (hasWizPower(POWER_SEE_COMMENTARY) && isImmortal() && *buf) {
    desc->start_page_file(TYPO_FILE, "Players can't spell worth nothin\'.\n\r");
    return;
  }

  typo_used_num++;
  if (!desc->client)
    sendTo("Write the subject of your typo then hit return.\n\r");

  if (!desc->client) {
    addPlayerAction(PLR_BUGGING);
    desc->connected = CON_WRITING;
    strcpy(desc->name, "Typo");
    desc->str = new (char *);
    *desc->str = new char[1];
    *(*desc->str) = '\0';
    desc->max_str = MAX_MAIL_SIZE;
  }
  desc->clientf("%d", CLIENT_TYPO);
}

void TBeing::doBug(const char *)
{
  sendTo("You are a monster! Bug off!\n\r");
  return;
}

void TPerson::doBug(const char *arg)
{
  char buf[256];

  one_argument(arg, buf);

  if (fight())  {
    sendTo("You cannot perform that action while fighting!\n\r");
    return;
  }
  if (hasWizPower(POWER_SEE_COMMENTARY) && isImmortal() && *buf) {
    desc->start_page_file(BUG_FILE, "Players aren't saying anything.\n\r");
    return;
  }

  bug_used_num++;
  if (!desc->client) 
    sendTo("Write the subject of your bug then hit return.\n\r");
  
  if (!desc->client) {
    addPlayerAction(PLR_BUGGING);
    desc->connected = CON_WRITING;
    strcpy(desc->name, "Bug");
    desc->str = new (char *);
    *desc->str = new char[1];
    *(*desc->str) = '\0';
    desc->max_str = MAX_MAIL_SIZE;
  }
  desc->clientf("%d", CLIENT_BUG);
}

void TBeing::doGroup(const char *argument)
{
  char namebuf[256];
  char buf[256];
  TBeing *victim, *k;
  followData *f;
  int found=FALSE;
  TThing *t;
  int tmp_share;

  argument = one_argument(argument, namebuf);

  if (!*namebuf) {
    if (!isAffected(AFF_GROUP))
      sendTo("But you are a member of no group?!\n\r");
    else {
      sendTo("Your group consists of:\n\r\n\r");
      if (master)
        k = master;
      else
        k = this;

      sprintf(namebuf, "%s", (k != this ? k->getNameNOC(this).c_str() : "You"));
      if (k->isAffected(AFF_GROUP)) {// && canSee(k)) {  I changed this on 010398 Russ
        if (sameRoom(k)) {
          if (k->desc)
            tmp_share = k->desc->session.group_share;
          else if (k->isPet() || k->isMount() || dynamic_cast<TMonster
*>(k))
            tmp_share = 0;
          else
            tmp_share = 1;
          if (k->hasClass(CLASS_CLERIC) || k->hasClass(CLASS_DEIKHAN)) {
            sendTo("%s%-15.15s%s [%s%.1f%%hp %.1f%%p. %s look%s %s.%s]\n\r\t%s%2d share%s talens, %.1f%% shares XP%s\n\r", cyan(), cap(namebuf), norm(), red(),
              (((double) (k->getHit())) / ((double) k->hitLimit()) * 100),
              k->getPiety(), 
              cap(namebuf),
              (k != this ? "s" : ""),
              DescMoves((((double) k->getMove()) / ((double) k->moveLimit()))),
              norm(), purple(),
              tmp_share, ((tmp_share == 1) ? "" : "s"),
              k->getExpSharePerc(),
              norm());
          } else {
            sendTo("%s%-15.15s%s [%s%.1f%%hp %.1f%%m. %s look%s %s.%s]\n\r\t%s%2d share%s talens, %.1f%% shares XP%s\n\r", cyan(), cap(namebuf), norm(), red(),
              (((double) (k->getHit())) / ((double) k->hitLimit()) * 100),
              (((double) (k->getMana())) / ((double) k->manaLimit()) * 100), 
              cap(namebuf),
              (k != this ? "s" : ""),
              DescMoves((((double) k->getMove()) / ((double) k->moveLimit()))),
              norm(), purple(),
              tmp_share, ((tmp_share == 1) ? "" : "s"), 
              k->getExpSharePerc(),
              norm());
          }
        } else 
          sendTo("%-15.15s [not around]\n\r", cap(namebuf));
      }
      for (f = k->followers; f; f = f->next) {
        sprintf(namebuf, "%s", (f->follower != this ? f->follower->getNameNOC(this).c_str() : "You"));
        if (f->follower->isAffected(AFF_GROUP) && canSee(f->follower)) {
          if (f->follower->desc)
            tmp_share = f->follower->desc->session.group_share;
          else if (f->follower->isPet() || f->follower->isMount() || dynamic_cast<TMonster *>(f->follower))
            tmp_share = 0;
          else
            tmp_share = 1;
          if (sameRoom(f->follower)) { 
            if (f->follower->hasClass(CLASS_CLERIC) || 
                f->follower->hasClass(CLASS_DEIKHAN))
              sendTo("%s%-15.15s%s [%s%.1f%%hp %.1f%%p. %s look%s %s.%s]\n\r\t%s%2d share%s talens, %.1f%% shares XP%s\n\r", cyan(), cap(namebuf), norm(), red(),
                (((double) (f->follower->getHit())) / ((double) f->follower->hitLimit()) * 100),
                f->follower->getPiety(),
                cap(namebuf),
                (f->follower != this ? "s" : ""),
                DescMoves((((double) f->follower->getMove()) / ((double) f->follower->moveLimit()))),
                norm(), purple(),
                tmp_share, ((tmp_share == 1) ? "" : "s"), 
                f->follower->getExpSharePerc(),
                norm());
            else {
              sendTo("%s%-15.15s%s [%s%.1f%%hp %.1f%%m. %s look%s %s.%s]\n\r\t%s%2d share%s talens, %.1f%% shares XP%s\n\r", cyan(), cap(namebuf), norm(), red(), 
                (((double) (f->follower->getHit())) / ((double) f->follower->hitLimit()) * 100),
                (((double) (f->follower->getMana())) / ((double) f->follower->manaLimit()) * 100),
                cap(namebuf),
                (f->follower != this ? "s" : ""),
                DescMoves((((double) f->follower->getMove()) / ((double) f->follower->moveLimit()))),
                norm(), purple(),
                tmp_share, ((tmp_share == 1) ? "" : "s"), 
                f->follower->getExpSharePerc(),
                norm());
            }
          } else 
            sendTo("%-15.15s [not around]\n\r", cap(namebuf));
        }
      }
    }
    return;
  }
  if (is_abbrev(namebuf, "share")) {
    int amt;

    if (!argument || !*argument) {
      sendTo("Syntax: group share <target> <amount>\n\r");
      return;
    }
    argument = one_argument(argument, namebuf);
    if (!namebuf || !*namebuf) {
      sendTo("Syntax: group share <target> <amount>\n\r");
      return;
    }
    argument = one_argument(argument, buf);
    if (!buf || !*buf) {
      sendTo("Syntax: group share <target> <amount>\n\r");
      return;
    }
    if ((amt = atoi(buf)) <= 0 || amt > 10) {
      sendTo("Syntax: group share <target> <amount>\n\r");
      sendTo("Amount must be in range 1-10.\n\r");
      return;
    }
    if (!isAffected(AFF_GROUP)) {
      sendTo("You don't seem to have a group.\n\r");
      return;
    }
    if (master) {
      sendTo("Only the group leader may set the group shares.\n\r");
      return;
    }
    if (!(victim = get_char_room_vis(this, namebuf))) {
      sendTo("No such character around.\n\r");
      return;
    }
    if (!victim->desc) {
      sendTo("You can only define shares for PCs.\n\r");
      sendTo("Pets get 0 shares, and other NPCs receive 1 by default.\n\r");
      return;
    }
    if (((victim->master == this) || (victim == this)) &&
           victim->isAffected(AFF_GROUP)) {
      victim->desc->session.group_share = amt;
      sprintf(buf, "I have just set %s's group-share to %d.", 
         victim->getName(), amt);
      doGrouptell(buf);
    } else {
      sendTo("That person isn't in your group.\n\r");
      return;
    }
  } else if (desc && (is_abbrev(namebuf, "amtank") || is_abbrev(namebuf, "amnottank"))) {
    if (is_abbrev(namebuf, "amtank")) {
      if (desc->session.amGroupTank)
        sendTo("Yes, you are a group tank.\n\r");
      else {
        sendTo("O.k.  You are now flagged as a group tank.\n\r");
        desc->session.amGroupTank = true;
      }
    } else {
      if (!desc->session.amGroupTank)
        sendTo("No, you are not a group tank.\n\r");
      else {
        sendTo("O.k.  You are no longer flagged as a group tank.\n\r");
        desc->session.amGroupTank = false;
      }
    }
  } else if (is_abbrev(namebuf, "seeksgroup")) {
    char typebuf[256]; 
    one_argument(argument, typebuf);
    if (!*typebuf) {
      if (isPlayerAction(PLR_SEEKSGROUP)) {
        remPlayerAction(PLR_SEEKSGROUP);
        sendTo("You are no longer seeking a group.\n\r");
      } else {
        addPlayerAction(PLR_SEEKSGROUP);
        sendTo("You are now flagged as seeking a group in the who command.\n\r");
      }
      return; 
    } else {
      sendTo("Functionality not available at this time.\n\r");
      return;
    }
  } else if (is_abbrev(namebuf,"all") || is_abbrev(namebuf, "followers")) {
    if (master) {
      sendTo("You can't group others without being the leader of the group.\n\r");
      return;
    }
    for (t = roomp->stuff;t;t = t->nextThing) {
      victim = dynamic_cast<TBeing *>(t);
      if (!victim)
        continue;

      if (victim->isAffected(AFF_GROUP) || !canSee(victim) || 
          (victim == riding))
        continue;

      if (victim->isPlayerAction(PLR_SOLOQUEST) && (this != victim)
          && (victim->master == this)) {
        sendTo(COLOR_MOBS, "%s is on a quest! No grouping allowed!\n\r", victim->getName());
        continue;
      }
      if (victim->isPlayerAction(PLR_GRPQUEST)) {
        if (!isPlayerAction(PLR_GRPQUEST)) {
          sendTo(COLOR_MOBS, "%s is on a group quest that you aren't on! You can't group!\n\r",victim->getName());
          continue;
        }
      }
      if (isPlayerAction(PLR_GRPQUEST)) {
        if (!victim->isPlayerAction(PLR_GRPQUEST)) {
          act("$N isn't on your quest, you can't group with $M.",
             TRUE,this,0,victim,TO_CHAR);
          continue;
        }
      }
      if (victim == this) {
        sendTo("You group yourself.\n\r");
        act("$n groups $mself.",TRUE,this,0,0,TO_ROOM);
        SET_BIT(victim->specials.affectedBy, AFF_GROUP);
        if (desc && desc->client) 
          desc->clientf("%d|%s|%d|%d|%s", CLIENT_GROUPADD, getName(), getHit(), getMana(), attack_modes[getCombatMode()]);
        
        if (victim->desc)
          victim->desc->session.group_share = 1;

        found = TRUE;
        continue;
      } else if (victim->master == this) {
        sendTo(COLOR_MOBS, "You add %s to your group.\n\r",victim->getName());
        act("$n adds $N to $s group.",TRUE,this,0,victim,TO_NOTVICT);
        victim->sendTo(COLOR_MOBS, "You are now a member of %s's group.\n\r",getName());
        SET_BIT(victim->specials.affectedBy, AFF_GROUP);
        if (desc && desc->client) 
          desc->clientf("%d|%s|%d|%d|%s", CLIENT_GROUPADD, victim->getName(), victim->getHit(), victim->getMana(), attack_modes[victim->getCombatMode()]);
        
        for (f = followers; f; f = f->next) {
          TBeing *b = f->follower;
          if (victim->desc && victim->desc->client)  {
            victim->desc->clientf("%d|%s|%d|%d|%s", CLIENT_GROUPADD, 
                       b->getName(), b->getHit(), b->getMana(), attack_modes[b->getCombatMode()]);
          } 
          if (b->desc && b->desc->client) {
            b->desc->clientf("%d|%s|%d|%d|%s", CLIENT_GROUPADD,
                             victim->getName(), victim->getHit(), victim->getMana(), attack_modes[victim->getCombatMode()]); 
          }
        }
        found = TRUE;
        if (victim->desc) {
          victim->desc->clientf("%d|%s|%d|%d|%s", CLIENT_GROUPADD, getName(), getHit(), getMana(), attack_modes[getCombatMode()]);
          victim->desc->session.group_share = 1;
        }
        continue;
      }
    }
    if (!found)
      sendTo("Sorry, there is no one else here you can group.\n\r");
  } else if (!(victim = get_char_room_vis(this, namebuf)))
    sendTo("No one here by that name.\n\r");
  else {
    if (victim->isPlayerAction(PLR_SOLOQUEST) && (this != victim)) {
      sendTo("That person is on a quest! No grouping allowed!\n\r");
      return;
    }
    if (victim->isPlayerAction(PLR_GRPQUEST)) {
      if (!isPlayerAction(PLR_GRPQUEST)) {
        sendTo("That person is on a group quest that you aren't on! You can't group!\n\r");
        return;
      }
    }
    if (isPlayerAction(PLR_GRPQUEST)) {
      if (!victim->isPlayerAction(PLR_GRPQUEST)) {
        sendTo("You can only group people with the group quest flag!\n\r");
        return;
      }
    }
    if (master) {
      sendTo("You can not enroll group members without being head of a group.\n\r");
      return;
    }
    if (!isAffected(AFF_GROUP) && (this != victim)) {
      sendTo("You must first group yourself to group others.\n\r");
      return;
    }
    found = FALSE;

    if (victim == this)
      found = TRUE;
    else {
      for (f = followers; f; f = f->next) {
        if (f->follower == victim) {
          found = TRUE;
          break;
        }
      }
    }
    if (found) {
      if (victim->isAffected(AFF_GROUP)) {
        if (victim == this) {
          if (followers) {
            act("$n ungroups $mself, causing the whole group to be disbanded.", FALSE, this, 0, NULL, TO_ROOM);
            sendTo("You ungroup yourself causing the rest of the group to be ungrouped.\n\r");
            REMOVE_BIT(specials.affectedBy, AFF_GROUP);
            
            if (desc) {
              desc->session.group_share = 1;
              if (desc->client)
                desc->clientf("%d", CLIENT_GROUPDELETEALL);
            }
            for (f = followers; f; f = f->next) {
              if (IS_SET(f->follower->specials.affectedBy, AFF_GROUP)) {
                REMOVE_BIT(f->follower->specials.affectedBy, AFF_GROUP);
                if (f->follower->desc) {
                  f->follower->desc->session.group_share = 1;
                  if (f->follower->desc->client)
                    f->follower->desc->clientf("%d", CLIENT_GROUPDELETEALL);
                }
              }
            }
          } else {
            sendTo("You ungroup yourself.\n\r");
            REMOVE_BIT(specials.affectedBy, AFF_GROUP);
            if (desc) {
              desc->session.group_share = 1;
              if (desc->client)
                desc->clientf("%d", CLIENT_GROUPDELETEALL);
            }
          }
        } else {
          if (victim->fight()) {
            act("You can't ungroup $N while $E is fighting.",
                FALSE, this, 0, victim, TO_CHAR);
            return;
          }
          act("$n has been kicked out of $N's group!", FALSE, victim, 0, this, TO_ROOM);
          act("You are no longer a member of $N's group!", FALSE, victim, 0, this, TO_CHAR);
          REMOVE_BIT(victim->specials.affectedBy, AFF_GROUP);
          if (victim->desc) {
            victim->desc->session.group_share = 1;
            if (victim->desc->client)
              victim->desc->clientf("%d", CLIENT_GROUPDELETEALL);
          }
          for (f = followers; f; f = f->next) {
            if (IS_SET(f->follower->specials.affectedBy, AFF_GROUP)) {
              if (f->follower->desc && f->follower->desc->client)
                f->follower->desc->clientf("%d|%s", CLIENT_GROUPDELETE, victim->getName());
            }
          }
          if (desc && desc->client) 
            desc->clientf("%d|%s", CLIENT_GROUPDELETE, victim->getName());
        }
      } else {
        if (fight()) {
          act("Not while fighting.", FALSE, this, 0, victim, TO_CHAR);
          return;
        }
        if (victim->fight()) {
          act("You can't group $N while $E is fighting.", FALSE, this, 0, victim, TO_CHAR);
          return;
        }
        act("$n is now a member of $N's group.", FALSE, victim, 0, this, TO_ROOM);
        act("You are now a member of $N's group.", FALSE, victim, 0, this, TO_CHAR);
        SET_BIT(victim->specials.affectedBy, AFF_GROUP);
        if (desc && desc->client) 
          desc->clientf("%d|%s|%d|%d|%s", CLIENT_GROUPADD, victim->getName(), victim->getHit(), victim->getMana(), attack_modes[victim->getCombatMode()]);
        
        if (victim != this) {
          for (f = followers; f; f = f->next) {
            TBeing *b = f->follower;
            if (victim->desc && victim->desc->client)  {
              victim->desc->clientf("%d|%s|%d|%d|%s", CLIENT_GROUPADD,
                         b->getName(), b->getHit(), b->getMana(), attack_modes[b->getCombatMode()]);
            }
            if (b->desc && b->desc->client) {
              b->desc->clientf("%d|%s|%d|%d|%s", CLIENT_GROUPADD,
                               victim->getName(), victim->getHit(), victim->getMana(), attack_modes[victim->getCombatMode()]);
            }
          }
        }
        if (victim->desc) {
          victim->desc->clientf("%d|%s|%d|%d|%s", CLIENT_GROUPADD, getName(), getHit(), getMana(), attack_modes[getCombatMode()]);
          victim->desc->session.group_share = 1;
        }
      }
    } else
      act("$N must follow you to enter the group", FALSE, this, 0, victim, TO_CHAR);
  }
}

int TThing::quaffMe(TBeing *ch)
{
  ch->sendTo("Quaff is generally used for liquids.\n\r");
  return FALSE;
}

// return DELETE_THIS
int TBeing::doQuaff(const char *argument)
{
  char buf[100];
  TThing *t;
  int rc = 0;

  only_argument(argument, buf);

#if 0
  // why is this needed? - bat
  if (roomp && roomp->isRoomFlag(ROOM_NO_HEAL)) {
    sendTo("You can no longer quaff potions in noheal rooms.\n\r");
    return FALSE;
  }
#endif

  if (!(t = searchLinkedListVis(this, buf, stuff))) {
    t = equipment[HOLD_RIGHT];
    if (!t || !isname(buf, t->name)) {
      act("You do not have that item.", FALSE, this, 0, 0, TO_CHAR);
      return FALSE;
    }
  }
  setQuaffUse(TRUE);
  rc = t->quaffMe(this);
  setQuaffUse(FALSE);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete t;
    t = NULL;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_THIS;
  }
  addToWait(combatRound(1));
  return FALSE;
}

int TPotion::quaffMe(TBeing *ch)
{
  int i;
  int rc;

  if (ch->fight()) {
    if (equippedBy) {
      if (!ch->isAgile(0)) {
        act("$n is jolted and drops $p! It shatters!", TRUE, ch, this, 0, TO_ROOM);
        act("You arm is jolted and $p flies from your hand, *SMASH*", TRUE, ch, this, 0, TO_CHAR);
        if (equippedBy)
          ch->unequip(eq_pos);
        return DELETE_THIS;
      }
    } else {
      if (!ch->isAgile(-35)) {
        act("$n is jolted and drops $p!  It shatters!", TRUE, ch, this, 0, TO_ROOM);
        act("You arm is jolted and $p flies from your hand, *SMASH*", TRUE, ch, this, 0, TO_CHAR);
        return DELETE_THIS;
      }
    }
  }

#if 0
  // semi annoying
  if (ch->getCond(THIRST) > -1) {
    if (ch->getCond(THIRST) > 20) {
      act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
      return FALSE;
    } else {
      ch->gainCondition(THIRST, 1);
      add = TRUE;
    }
  }
#endif

  act("$n quaffs $p.", TRUE, ch, this, 0, TO_ROOM);
  act("You quaff $p which dissolves.", FALSE, ch, this, 0, TO_CHAR);

  ch->playsound(SOUND_WATER_GURGLE, SOUND_TYPE_NOISE);

  for (i = 0; i <= 2; i++) {
    spellNumT spellnum = getSpell(i);
    if (spellnum >= 0) {
      if (!discArray[spellnum]) {
        vlogf(10,"doQuaff (%s) called spell (%d) that does not exist!", 
            getName(), spellnum);
        continue;
      }
      rc = doObjSpell(ch,ch,this,NULL,"", spellnum);
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        // uh, not possible right
        vlogf(5, "whacked out potion return %s", getName());
      }
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT)) {
        if (equippedBy)
          ch->unequip(eq_pos);

        return DELETE_VICT | DELETE_THIS;
      }
    }
  }

  if (equippedBy)
    ch->unequip(eq_pos);

  ch->addToWait(combatRound(1));
  return DELETE_THIS;
}

// DELETE_VICT: victim
// DELETE_THIS: caster
// DELETE_ITEM: target
int doObjSpell(TBeing *caster, TBeing *victim, TMagicItem *obj, TObj *target, const char *, spellNumT spell)
{
  int rc = 0;

  // define the spell being used at the global level
  // this is used to help act() know what to use for yourDeity() stuff
  your_deity_val = spell;

  // validate targetting
  if (!IS_SET(discArray[spell]->targets, TAR_IGNORE | TAR_AREA)) {

    if (!victim && !target) {
      forceCrash("Bad targetting in doObjSpell() %d", spell);
      return 0;
    }

    // invoked on a being
    // recite, use staff (non-area), use wand, quaff
    if (victim && 
        !IS_SET(discArray[spell]->targets, TAR_CHAR_ROOM | TAR_CHAR_WORLD | TAR_FIGHT_SELF | TAR_SELF_ONLY)) {
      // tried to invoke on being, not designed for that
      caster->sendTo(COLOR_BASIC, "You can not invoke '%s' on %s.\n\r",
          discArray[spell]->name, victim->getName());
      return 0;
    }

    // invoked on an obj
    // recite, use wand
    if (target && 
        !IS_SET(discArray[spell]->targets, TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM | TAR_OBJ_WORLD)) {
      // tried to invoke on being, not designed for that
      caster->sendTo(COLOR_BASIC, "You can not invoke '%s' on %s.\n\r",
          discArray[spell]->name, target->getName());
      return 0;
    }
  }

  switch (spell) {
    case SPELL_GUST:
      rc = gust(caster, victim, obj);
      break;
    case SPELL_TORNADO:
      tornado(caster,obj);
      break;
    case SPELL_SUFFOCATE:
      suffocate(caster,victim,obj);
      break;
    case SPELL_FEATHERY_DESCENT:
      featheryDescent(caster,victim,obj);
      break;
    case SPELL_FALCON_WINGS:
      falconWings(caster,victim,obj);
      break;
    case SPELL_FLY:
      fly(caster,victim,obj);
      break;
    case SPELL_ILLUMINATE:
      illuminate(caster,obj,target);
      break;
    case SPELL_IDENTIFY:
      if (target) {
        identify(caster,target, obj);
      } else {
        identify(caster,victim,obj);
      }
      break;
    case SPELL_DETECT_MAGIC:
      detectMagic(caster,victim,obj);
      break;
    case SPELL_DISPEL_MAGIC:
      rc = dispelMagic(caster,victim,obj);
      break;
    case SPELL_COPY:
      copy(caster,obj,target);
      break;
    case SPELL_ENHANCE_WEAPON:
      rc = enhanceWeapon(caster,obj,target);
      break;
    case SPELL_METEOR_SWARM:
      rc = meteorSwarm(caster,victim,obj);
      break;
    case SPELL_PROTECTION_FROM_AIR:
      protectionFromAir(caster,victim,obj);
      break;
    case SPELL_PROTECTION_FROM_EARTH:
      protectionFromEarth(caster,victim,obj);
      break;
    case SPELL_PROTECTION_FROM_ELEMENTS:
      protectionFromElements(caster,victim,obj);
      break;
    case SPELL_PROTECTION_FROM_WATER:
      protectionFromWater(caster,victim,obj);
      break;
    case SPELL_STONE_SKIN:
      stoneSkin(caster,victim,obj);
      break;
    case SPELL_TRAIL_SEEK:
      trailSeek(caster,victim,obj);
      break;
    case SPELL_HANDS_OF_FLAME:
      rc = handsOfFlame(caster,victim,obj);
      break;
    case SPELL_HELLFIRE:
      hellfire(caster,obj);
      break;
    case SPELL_FAERIE_FIRE:
      faerieFire(caster,victim,obj);
      break;
    case SPELL_FIREBALL:
      rc = fireball(caster,obj);
      break;
    case SPELL_FLAMING_FLESH:
      flamingFlesh(caster,victim,obj);
      break;
    case SPELL_FLAMING_SWORD:
      rc = flamingSword(caster,victim,obj);
      break;
    case SPELL_INFERNO:
      rc = inferno(caster,victim,obj);
      break;
    case SPELL_INFRAVISION:
      infravision(caster,victim,obj);
      break;
    case SPELL_GUSHER:
      rc = gusher(caster,victim,obj);
      break;
    case SPELL_ARCTIC_BLAST:
      rc = arcticBlast(caster,obj);
      break;
    case SPELL_ICY_GRIP:
      rc = icyGrip(caster,victim,obj);
      break;
    case SPELL_ICE_STORM:
      rc = iceStorm(caster,obj);
      break;
    case SPELL_PLASMA_MIRROR:
      // intentionally prohibited - too powerful
      break;
    case SPELL_GILLS_OF_FLESH:
      gillsOfFlesh(caster,victim,obj);
      break;
    case SPELL_MYSTIC_DARTS:
      rc = mysticDarts(caster,victim,obj);
      break;
    case SPELL_FLARE:
      rc = flare(caster,obj);
      break;
    case SPELL_STUNNING_ARROW:
      rc = stunningArrow(caster,victim,obj);
      break;
    case SPELL_COLOR_SPRAY:
      rc = colorSpray(caster,obj);
      break;
    case SPELL_SORCERERS_GLOBE:
      sorcerersGlobe(caster,victim,obj);
      break;
    case SPELL_BIND:
      bind(caster,victim,obj);
      break;
    case SPELL_TELEPORT:
      rc = teleport(caster,victim,obj);
      break;
    case SPELL_SENSE_LIFE:
      senseLife(caster,victim,obj);
      break;
    case SPELL_STEALTH:
      stealth(caster,victim,obj);
      break;
    case SPELL_ENSORCER:
      ensorcer(caster,victim,obj);
      break;
    case SPELL_INVISIBILITY:
      if (target) {
        invisibility(caster, target, obj);
      } else {
        invisibility(caster, victim, obj);
      }
      break;
    case SPELL_DISPEL_INVISIBLE:
      if (target) {
        dispelInvisible(caster, target, obj);
      } else {
        dispelInvisible(caster, victim, obj);
      }
      break;
    case SPELL_DETECT_INVISIBLE:
      detectInvisibility(caster,victim,obj);
      break;
    case SPELL_TRUE_SIGHT:
      trueSight(caster,victim,obj);
      break;
    case SPELL_GARMULS_TAIL:
      garmulsTail(caster,victim,obj);
      break;
    case SPELL_ACCELERATE:
      accelerate(caster,victim,obj);
      break;
    case SPELL_HASTE:
      haste(caster,victim,obj);
      break;
    case SPELL_SLUMBER:
      rc = slumber(caster,victim,obj);
      break;
    case SPELL_SPONTANEOUS_COMBUST:
      spontaneousCombust(caster,victim,obj);
      break;
    case SPELL_PILLAR_SALT:
      pillarOfSalt(caster,victim,obj);
      break;
    case SPELL_RAIN_BRIMSTONE_DEIKHAN:
    case SPELL_RAIN_BRIMSTONE:
      rainBrimstone(caster,victim,obj, spell);
      break;
    case SPELL_FLAMESTRIKE:
      flamestrike(caster,victim,obj);
      break;
    case SPELL_PLAGUE_LOCUSTS:
      plagueOfLocusts(caster,victim,obj);
      break;
    case SPELL_CURSE_DEIKHAN:
    case SPELL_CURSE:
      if (target) {
        curse(caster,target,obj, spell);
      } else {
        curse(caster,victim,obj, spell);
      }
      break;
    case SPELL_EARTHQUAKE_DEIKHAN:
    case SPELL_EARTHQUAKE:
      rc = earthquake(caster,obj, spell);
      break;
    case SPELL_CALL_LIGHTNING_DEIKHAN:
    case SPELL_CALL_LIGHTNING:
      rc = callLightning(caster,victim,obj, spell);
      break;
    case SPELL_HEAL_LIGHT:
    case SPELL_HEAL_LIGHT_DEIKHAN:
      healLight(caster,victim,obj, spell);
      break;
    case SPELL_HEAL_SERIOUS:
    case SPELL_HEAL_SERIOUS_DEIKHAN:
      healSerious(caster,victim,obj, spell);
      break;
    case SPELL_HEAL_CRITICAL:
    case SPELL_HEAL_CRITICAL_DEIKHAN:
      healCritical(caster,victim,obj, spell);
      break;
    case SPELL_HEAL:
      heal(caster,victim,obj, SPELL_HEAL);
      break;
    case SPELL_HEAL_FULL:
      healFull(caster,victim,obj);
      break;
    case SPELL_RESTORE_LIMB:
      restoreLimb(caster,victim,obj);
      break;
    case SPELL_CLOT:
    case SPELL_CLOT_DEIKHAN:
      clot(caster,victim,obj, spell);
      break;
    case SPELL_STERILIZE:
    case SPELL_STERILIZE_DEIKHAN:
      sterilize(caster,victim,obj, spell);
      break;
    case SPELL_SALVE:
    case SPELL_SALVE_DEIKHAN:
      salve(caster,victim,obj, spell);
      break;
    case SPELL_HEAL_CRITICAL_SPRAY:
      healCritSpray(caster, obj);
      break;
    case SPELL_HEAL_SPRAY:
      healSpray(caster, obj);
      break;
    case SPELL_HEAL_FULL_SPRAY:
      healFullSpray(caster, obj);
      break;
    case SPELL_KNIT_BONE:
      knitBone(caster,victim,obj);
      break;
    case SPELL_EXPEL:
    case SPELL_EXPEL_DEIKHAN:
      expel(caster,victim,obj, spell);
      break;
    case SPELL_BLEED:
      rc = bleed(caster,victim,obj);
      break;
    case SPELL_BONE_BREAKER:
      rc = boneBreaker(caster,victim,obj);
      break;
    case SPELL_PARALYZE:
      paralyze(caster,victim,obj);
      break;
    case SPELL_POISON_DEIKHAN:
    case SPELL_POISON:
      if (target)
        poison(caster,target,obj, spell);
      else if (victim)
        poison(caster,victim,obj, spell);
      else
        vlogf(5, "doObjSpell:poison had bogus targets");
      break;
    case SPELL_BLINDNESS:
      blindness(caster,victim,obj);
      break;
    case SPELL_HARM_DEIKHAN:
    case SPELL_HARM:
      harm(caster,victim,obj, spell);
      break;
    case SPELL_HARM_LIGHT_DEIKHAN:
    case SPELL_HARM_LIGHT:
      harmLight(caster,victim,obj, spell);
      break;
    case SPELL_HARM_SERIOUS_DEIKHAN:
    case SPELL_HARM_SERIOUS:
      harmSerious(caster,victim,obj, spell);
      break;
    case SPELL_HARM_CRITICAL_DEIKHAN:
    case SPELL_HARM_CRITICAL:
      harmCritical(caster,victim,obj, spell);
      break;
    case SPELL_PARALYZE_LIMB:
      rc = paralyzeLimb(caster,victim,obj);
      break;
    case SPELL_WITHER_LIMB:
      rc = witherLimb(caster,victim,obj);
      break;
    case SPELL_INFECT_DEIKHAN:
    case SPELL_INFECT:
      infect(caster,victim,obj, spell);
      break;
    case SPELL_DISEASE:
      disease(caster,victim,obj);
      break;
    case SPELL_NUMB_DEIKHAN:
    case SPELL_NUMB:
      rc = numb(caster,victim,obj, spell);
      break;
    case SPELL_WORD_OF_RECALL:
      wordOfRecall(caster,victim,obj);
      break;
    case SPELL_BLESS:
    case SPELL_BLESS_DEIKHAN:
      if (target) {
        bless(caster,target,obj, spell);
      } else {
        bless(caster,victim,obj, spell);
      }
      break;
    case SPELL_ARMOR:
    case SPELL_ARMOR_DEIKHAN:
      armor(caster,victim,obj, spell);
      break;
    case SPELL_SANCTUARY:
      sanctuary(caster,victim,obj);
      break;
    case SPELL_REMOVE_CURSE:
    case SPELL_REMOVE_CURSE_DEIKHAN:
      if (target) {
        caster->removeCurseObj(target,obj, spell);
      } else {
        caster->removeCurseBeing(victim,obj,spell);
      }
      break;
    case SPELL_CURE_PARALYSIS:
      cureParalysis(caster,victim,obj);
      break;
    case SPELL_CURE_DISEASE:
    case SPELL_CURE_DISEASE_DEIKHAN:
      cureDisease(caster,victim,obj, spell);
      break;
    case SPELL_CURE_POISON:
    case SPELL_CURE_POISON_DEIKHAN:
      curePoison(caster,victim,obj, spell);
      break;
    case SPELL_CURE_BLINDNESS:
      cureBlindness(caster,victim,obj);
      break;
    case SPELL_REFRESH:
    case SPELL_REFRESH_DEIKHAN:
      refresh(caster,victim,obj, spell);
      break;
    case SPELL_SECOND_WIND:
      secondWind(caster,victim,obj);
      break;
    case SPELL_CONTROL_UNDEAD:
      controlUndead(caster,victim,obj);
      break;
    case SPELL_RESURRECTION:
      rc = resurrection(caster,target,obj);
      break;
    case SPELL_DANCING_BONES:
      rc = dancingBones(caster,target,obj);
      break;
    case SPELL_VOODOO:
      rc = voodoo(caster,target,obj);
      break;
    case SKILL_BARKSKIN:
      barkskin(caster,victim,obj);
      break;
    case SPELL_STICKS_TO_SNAKES:
      sticksToSnakes(caster,victim,obj);
      break;
    case SPELL_LIVING_VINES:
      livingVines(caster,victim,obj);
      break;
    case SPELL_STORMY_SKIES:
      stormySkies(caster,victim,obj);
      break;
    case SPELL_ATOMIZE:
      /*
      atomize(caster,victim,obj);
      break;
      */
    default:
      vlogf(5,"Object (%s) with uncoded spell (%d)!", obj->getName(), spell);
      break;
  }
  return rc;
}

int TThing::reciteMe(TBeing *ch, const char *)
{
  act("Recite is normally used for scrolls.", FALSE, ch, 0, 0, TO_CHAR);
  return FALSE;
}

int TBeing::doRecite(const char *argument)
{
  char buf[100];
  TThing *t;
  int rc;

  argument = one_argument(argument, buf);

  if (isAffected(AFF_BLIND)) {
    sendTo("How do you expect to read something when you are blind???\n\r");
    return FALSE;
  }

  if (!(t = searchLinkedListVis(this, buf, stuff))) {
    t = heldInPrimHand();
    if (!t || !isname(buf, t->name)) {
      act("You do not have that item.", FALSE, this, 0, 0, TO_CHAR);
      return FALSE;
    }
  }
  setQuaffUse(true);
  rc = t->reciteMe(this, argument);
  setQuaffUse(false);

  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete t;
    t = NULL;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_THIS;
  }
  addToWait(combatRound(1));
  return FALSE;
}

int TScroll::reciteMe(TBeing *ch, const char * argument)
{
  TObj *obj = NULL;
  TBeing *victim = NULL;
  int i, bits, rc;

  if (!ch->hasClass(CLASS_MAGIC_USER) && !ch->hasClass(CLASS_CLERIC)) {
    if (!bSuccess(ch, ch->getSkillValue(SKILL_READ_MAGIC), SKILL_READ_MAGIC)) {
      ch->sendTo("You can't understand this...\n\r");
      return FALSE;
    }
  }
  bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &victim, &obj);

  if (!bits) {
    if (!ch->fight() || !ch->sameRoom(ch->fight())) {
      ch->sendTo("No such thing around to recite the scroll on.\n\r");
      return FALSE;
    }
    victim = ch->fight();
    bits = FIND_CHAR_ROOM;
  }

  act("$n recites $p which is consumed by the powers it unleashes.",
       TRUE, ch, this, 0, TO_ROOM);
  act("You recite $p which is consumed by the powers it unleashes.",
       FALSE, ch, this, 0, TO_CHAR);

  for (i = 0; i < 3; i++)  {
    spellNumT the_spell = getSpell(i);
    if (the_spell >= MIN_SPELL) {
      if (!discArray[the_spell]) {
        vlogf(10,"doRecite called spell (%d) that doesnt exist! - Don't do that!", the_spell);
        continue;
      }
      if ((discArray[the_spell]->targets & TAR_VIOLENT) &&
          ch->checkPeaceful("Impolite magic is banned here.\n\r"))
        continue;

      rc=doObjSpell(ch,victim,this,obj,argument,the_spell);
      if (IS_SET_DELETE(rc, DELETE_VICT) && victim != ch) {
        delete victim;
        victim = NULL;
        break;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS) ||
          (IS_SET_DELETE(rc, DELETE_VICT) && ch == victim)) {
        if (equippedBy)
          ch->unequip(ch->getPrimaryHold());

        return DELETE_THIS | DELETE_VICT;
      }
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete obj;
        obj = NULL;
        break;
      }
    }
  }
  if (equippedBy)
    ch->unequip(ch->getPrimaryHold());

  return DELETE_THIS;
}

// returns DELETE_THIS if this should be toasted.
int TBeing::doUse(const char *argument)
{
  char buf[256];
  int rc;

  argument = one_argument(argument, buf);

  TThing *t = NULL;
  if (!t) {
    t = heldInPrimHand();
    if (t && !isname(buf, t->name))
      t = NULL;
  }
  if (!t) {
    t = heldInSecHand();
    if (t && !isname(buf, t->name))
      t = NULL;
  }
  if (!t) {
    wearSlotT wst;
    for (wst = MIN_WEAR; wst < MAX_WEAR; t = NULL, wst++) {
      t = equipment[wst];
      if (!t)
        continue;
      if (!isname(buf, t->name))
        continue;
      TArmorWand * taw = dynamic_cast<TArmorWand *>(t);
      if (!taw)
        continue;
      break;
    }
  }
  if (!t) {
    sendTo("You aren't holding anything like that!\n\r");
    return FALSE;
  }
  if (rider) {
    sendTo("Mounts can't use stuff...\n\r");
    return FALSE;
  }
  setQuaffUse(TRUE); // just tells bSuccess its a staff
  rc = t->useMe(this, argument);
  setQuaffUse(FALSE);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete t;
    t = NULL;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_THIS;
  }
  addToWait(combatRound(1));
  return FALSE;
}

void TBeing::doLight(const string & argument)
{
  TThing *t = NULL;
  char tmpname[256], *tmp;
  bool roomOnly, heldOnly;
  int num;
  string arg1, arg2;

  two_arg(argument, arg1, arg2);

  strcpy(tmpname, arg1.c_str());
  tmp = tmpname;

  roomOnly = is_abbrev(arg2, "room");
  heldOnly = is_abbrev(arg2, "held");
  
  if (arg1.empty()) {
    sendTo("Light what?\n\r");
    return;
  } else {
    if (!(num = get_number(&tmp)))
      return;

    // searchLinkedList not searchLinkedListVis so newbies can light lampposts
    if (heldOnly || !(t = searchLinkedList(arg1, roomp->stuff))) {
      if (roomOnly || (!(t = heldInPrimHand())) ||
          !isname(tmp, t->name) || (num == 2)) {
        if (roomOnly || (!(t = heldInSecHand())) ||
            !isname(tmp, t->name) || (num > 2)) {
          sendTo("You can only light objects that are in the room or held.\n\r");
          return;
        }
      }
    }
    // Do the various checks to see if this object can be lit. - Russ
    t->lightMe(this, SILENT_NO);
  }
}

void TObj::setBurning(TBeing *ch){
  if(!isObjStat(ITEM_BURNING)){
    setObjStat(ITEM_BURNING);
    if(isObjStat(ITEM_CHARRED))
      remObjStat(ITEM_CHARRED);
    
    int lightamount=max(1, getVolume()/500);
    addToLight(lightamount);
#if 0
    if(ch){
      ch->roomp->addToLight(lightamount);
      ch->addToLight(lightamount);
    } else {
      roomp->addToLight(lightamount);
    }
#endif
  }
}

void TObj::remBurning(TBeing *ch){
  if(isObjStat(ITEM_BURNING)){
    remObjStat(ITEM_BURNING);
    if(material_nums[getMaterial()].flammability &&
       !isObjStat(ITEM_CHARRED))
      setObjStat(ITEM_CHARRED);
    
    int lightamount=max(1, getVolume()/500);
    addToLight(-lightamount);
#if 0
    if(ch){
      ch->roomp->addToLight(-lightamount);
      ch->addToLight(-lightamount);
    } else {
      roomp->addToLight(-lightamount);
    }
#endif
  }
}

void TThing::extinguishMe(TBeing *ch)
{
  TObj *o;

  if(!(o=dynamic_cast<TObj *>(this)) || !o->isObjStat(ITEM_BURNING)){
    ch->sendTo("You can't extinguish that; it's not burning.\n\r");
  } else {
    if(ch->isImmortal()){
      o->remBurning(ch);
      act("You extinguish $p, and it smolders slightly before going out.", FALSE, ch, o, 0, TO_CHAR);
      act("$n extinguishes $p, and it smolders slightly before going out.", FALSE, ch, o, 0, TO_ROOM);
    } else
      ch->sendTo("Not supported.\n\r");
  }
  return;
}

void TBeing::doExtinguish(const string & argument)
{
  TThing *t = NULL;
  char tmpname[256], *tmp;
  int num;
  string arg1, arg2;

  two_arg(argument, arg1, arg2);

  strcpy(tmpname, arg1.c_str());
  tmp = tmpname;

  bool roomOnly = is_abbrev(arg2, "room");
  bool heldOnly = is_abbrev(arg2, "held");

  if (arg1.empty()) {
    sendTo("Extinguish what?\n\r");
    return;
  } else {
    if (!(num = get_number(&tmp)))
      return;

    // searchLinkedList not searchLinkedListVis so newbies can light lampposts
    if (heldOnly || !(t = searchLinkedList(arg1, roomp->stuff))) {
      if (roomOnly || (!(t = heldInPrimHand())) ||
          !isname(tmp, t->name) || (num == 2)) {
        if (roomOnly || (!(t = heldInSecHand())) ||
            !isname(tmp, t->name) || (num > 2)) {
          sendTo("You can only extinguish objects that are in the room or held.\n\r");
          return;
        }
      }
    }
    t->extinguishMe(this);
  }
}

void TThing::refuelMeLight(TBeing *ch, TThing *)
{
  ch->sendTo("Refueling is for lights and lamps.\n\r");
  return;
}

void TThing::refuelMeFuel(TBeing *ch, TLight *)
{
  act("$p isn't a fuel!", FALSE, ch, this, NULL, TO_CHAR);
  return;
}

void TThing::refuelMeDrug(TBeing *ch, TDrugContainer *tdc)
{
  char buf[256];
  sprintf(buf, "$p isn't a drug and shouldn't be put into %s.", tdc->getName());
  act(buf, FALSE, ch, this, NULL, TO_CHAR);
  return;
}

void TBeing::doRefuel(const char *argument)
{
  TThing *fuel;
  TThing *t;
  char arg[128], arg2[128], arg3[128];
  int roomOnly, heldOnly;

  three_arg(argument, arg, arg2, arg3);

  if (!*arg) {
    sendTo("Refuel what?\n\r");
    sendTo("Syntax: refuel <light> <fuel> {\"room\" | \"held\"}\n\r");
    return;
  }
  if (!*arg2) {
    sendTo("Refuel with what?\n\r");
    sendTo("Syntax: refuel <light> <fuel> {\"room\" | \"held\"}\n\r");
    return;
  }
  if (!(fuel = get_thing_char_using(this, arg2, 0, FALSE, FALSE))) {
    sendTo("You don't have that fuel in your inventory!\n\r");
    sendTo("Syntax: refuel <light> <fuel> {\"room\" | \"held\"}\n\r");
    return;
  }
  roomOnly = is_abbrev(arg3, "room");
  heldOnly = is_abbrev(arg3, "held");

  if (roomOnly || 
      !(t = get_thing_char_using(this, arg, 0, FALSE, FALSE))) {
    if (heldOnly ||
        !(t = searchLinkedListVis(this, arg, roomp->stuff))) {
      sendTo("You can only refuel an object in the room, or held.\n\r");
      sendTo("Syntax: refuel <light> <fuel> {\"room\" | \"held\"}\n\r");
      return;
    }
  }
  // Do some checks to see if it can be refueled. 
  t->refuelMeLight(this, fuel);
}

void TBeing::doStop()
{
  sendTo("Stop what?  You aren't doing anything!\n\r");
}

void TBeing::doContinue(const char *argument)
{
#if 1 
  int value = 0;
  char arg[256];
  skillUseTypeT spellType;

  if (!spelltask)
    return;

  spellType = getSpellType(discArray[spelltask->spell]->typ);
  argument = one_argument(argument, arg);

  if ((spellType == SPELL_PRAYER)) {
    if (!reconcilePiety(spelltask->spell, TRUE)) {
      sendTo("You can not continue a prayer when you are low on piety.\n\r");
      return;
    }
  } else if ((spellType == SPELL_CASTER)) {
    if (!reconcileMana(spelltask->spell, TRUE)) {
      sendTo("You can not continue a spell when you are low on mana.\n\r");
      return;
    }
  }
 
  if (*arg) {
    if (is_abbrev(arg, "stop") || is_abbrev(arg, "finish")) {
      sendTo(COLOR_SPELLS, "<g>You start finishing your prayer.<1>\n\r");
      spelltask->rounds = max(1, spelltask->rounds);
      REMOVE_BIT(spelltask->flags, CASTFLAG_CAST_INDEFINITE);
      return;
    }
    value = atoi(arg);    
    if (value < 1) {
      sendTo("You can not change your prayer in this manner.\n\r");
      return;
    }
    if ((spellType == SPELL_PRAYER) && (getPiety() < (value *
usePiety(spelltask->spell)))) {
      sendTo("You do not have the piety to continue your prayer that many times.\n\r");
      return;
    } else if ((spellType == SPELL_CASTER) && (getMana() < (value *
useMana(spelltask->spell)))) {
      sendTo("You do not have the mana to continue your spell that many times.\n\r");
      return;
    }
    if (IS_SET(discArray[(spelltask->spell)]->comp_types, SPELL_TASKED_EVERY)) {
      spelltask->rounds += value;
      if (!IS_SET(spelltask->flags, CASTFLAG_CAST_INDEFINITE)) {
        sendTo(COLOR_SPELLS, "<o>You continue your prayer for %d additional round%s (%d total round%s).<1>\n\r",
         value, value != 1 ? "s" : "",
         spelltask->rounds, spelltask->rounds != 1 ? "s" : "");
      } else {
         sendTo(COLOR_SPELLS, "<o>You add %d round%s to your indefinite prayer (%d total round%s).<1>\n\r", 
         value, value != 1 ? "s" : "",
         spelltask->rounds, spelltask->rounds != 1 ? "s" : "");

      }
      return;
    }
    return;
  } else if (IS_SET(discArray[(spelltask->spell)]->comp_types, SPELL_TASKED_EVERY)) {
    if (!IS_SET(spelltask->flags, CASTFLAG_CAST_INDEFINITE)) {
      sendTo(COLOR_SPELLS,"<B>You decide to continue your praying indefinitely.<1>\n\r");
      SET_BIT(spelltask->flags, CASTFLAG_CAST_INDEFINITE);
    } else {
      sendTo(COLOR_SPELLS,"<b>You are praying indefinitely with %d rounds to go.<1>\n\r", max(spelltask->rounds, 1));
    }
  }
#endif
}

void TBeing::doHistory()
{
  int i;
  Descriptor *d;

  if (!(d = desc))
    return;

  if (d->client) {
    sendTo("The client keeps its own history, please use that!\n\r");
    return;
  }
  sendTo("Your command history :\n\r\n\r");
  for (i = 0; i < 10; i++)
    sendTo("[%d] %s\n\r", i, d->history[i]);
}


void TBeing::doDrag(TBeing *v, dirTypeT tdir)
{
  int rc;
  TBeing *heap_ptr[50];
  int i, heap_top, heap_tot[50], result;
  followData *k, *n;
  char buf[256];
  int or, oldroom;
  TRoom *rp;

  
  if (v == this) {
    sendTo("You can't drag yourself!\n\r");
    sendTo("Syntax : drag <person> <direction>\n\r");
    return;
  }
  if (v->getPosition() > POSITION_SLEEPING) {
    sendTo("You can't drag someone who is conscious.\n\r");
    return;
  }
  if (v->riding) {
    act("You can't drag them off the $o.",TRUE,this,v->riding,0,TO_CHAR);
    return;
  }
  if (v->rider) {
    sendTo("Sorry, you can't drag creatures carrying other creatures.\n\r");
    return;
  }


  // v-weight > 5 * free carry weight
  if (compareWeights(v->getTotalWeight(TRUE),
          (5.0 * (carryWeightLimit() - getCarriedWeight()))) == -1) {
    act("You strain with all your might to drag $N out of the room but fail.",
        TRUE, this, NULL, v, TO_CHAR);
    act("$n strains with all $s might to drag $N out of the room buts fails.",
        TRUE, this, NULL, v, TO_ROOM);
    return;
  }

  sprintf(buf, "%i", tdir);
  rc = roomp->checkSpec(this, CMD_ROOM_ATTEMPTED_EXIT, buf, roomp);
  if(rc==TRUE) // not allowed to move
    return;

  // We can drag now. Do necessary checks and make it so. - Brutius 
  sprintf(buf, "You drag $N %s.", dirs[tdir]);
  act(buf, TRUE, this, NULL, v, TO_CHAR);
  sprintf(buf, "$n drags $N %s.", dirs[tdir]);
  act(buf, TRUE, this, NULL, v, TO_ROOM);
  oldroom = v->in_room;
  or = v->roomp->dir_option[tdir]->to_room;
  rp = real_roomp(or);
  --(*v);
  --(*this);
  *rp += *this;
  *rp += *v;
  act("$n enters the room dragging $N!", TRUE, this, NULL, v, TO_ROOM);

  if (v->followers) {
    heap_top = 0;
    for (k = v->followers; k; k = n) {
      n = k->next;
      if ((oldroom == k->follower->in_room) && !k->follower->fight() &&
          (k->follower->getPosition() >= POSITION_CRAWLING)) {
        sprintf(buf, "You follow $N as $E is dragged out of the room.");
        act(buf, FALSE, k->follower, NULL, v, TO_CHAR);
        if (k->follower->followers) {
          rc = k->follower->moveGroup(tdir);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete k->follower;
            k->follower = NULL;
            continue;
          }
        } else {
          if ((result = k->follower->rawMove(tdir))) {
            if (IS_SET_DELETE(result, DELETE_THIS)) {
              delete k->follower;
              k->follower = NULL;
              continue;
            }
            AddToCharHeap(heap_ptr, &heap_top, heap_tot, k->follower);
          }
        }
      }
    }
    for (i = 0; i < heap_top; i++) {
      if (heap_tot[i] > 1)
        rc = heap_ptr[i]->displayGroupMove(tdir, oldroom, heap_tot[i]);
      else
        rc = heap_ptr[i]->displayOneMove(tdir, oldroom);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete heap_ptr[i];
        heap_ptr[i] = NULL;
      }
    }
  }
  if (followers) {
    heap_top = 0;
    for (k = followers; k; k = n) {
      n = k->next;
      if ((oldroom == k->follower->in_room) && !k->follower->fight() &&
          (k->follower->getPosition() >= POSITION_CRAWLING)) {
        sprintf(buf, "You follow $N as $E drags $p out of the room.");
        act(buf, FALSE, k->follower, v, this, TO_CHAR); 
        if (k->follower->followers) {
          rc = k->follower->moveGroup(tdir);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete k->follower;
            k->follower = NULL;
            continue;
          }
        } else {
          if ((result = k->follower->rawMove(tdir))) {
            if (IS_SET_DELETE(result, DELETE_THIS)) {
              delete k->follower;
              k->follower = NULL;
              continue;
            }
            AddToCharHeap(heap_ptr, &heap_top, heap_tot, k->follower);
          }
        }
      }
    }
    for (i = 0; i < heap_top; i++) {
      if (heap_tot[i] > 1)
        rc = heap_ptr[i]->displayGroupMove(tdir, oldroom, heap_tot[i]);
      else
        rc = heap_ptr[i]->displayOneMove(tdir, oldroom);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete heap_ptr[i];
        heap_ptr[i] = NULL;
      }
    }
  }

  doLook("", CMD_LOOK);
  addToMove(-20);
  addToWait(combatRound(1));
}

// Object dragging by Peel
void TBeing::doDrag(TObj *o, dirTypeT tdir)
{
  TBeing *heap_ptr[50];
  int i, heap_top, heap_tot[50], result;
  followData *k, *n;
  char buf[256];
  int oldroom, or, rc;
  TRoom *rp;

  if(!o) return;
  if (!isImmortal()) {
    if(!o->canWear(ITEM_TAKE)) {
      act("$N : You can't drag that.\n\r", TRUE, this, NULL, o, TO_CHAR);
      return;
    }
    if (dynamic_cast<TTrap *>(o)) {
      // drag grenade into room with shopkeeper
      act("$N : Yeah right.\n\r", TRUE, this, NULL, o, TO_CHAR);
      return;
    }
  }
  if(o->rider){
    act("$N : Occupied.\n\r", TRUE, this, NULL, o, TO_CHAR);
    return;
  }

  if (compareWeights(o->getTotalWeight(TRUE),
	      (5.0 * (carryWeightLimit() - getCarriedWeight()))) == -1) {
    act("You strain with all your might to drag $N out of the room but fail.",
	TRUE, this, NULL, o, TO_CHAR);
    act("$n strains with all $s might to drag $N out of the room buts fails.",
        TRUE, this, NULL, o, TO_ROOM);
    return;
  }

  sprintf(buf, "%i", tdir);
  rc = roomp->checkSpec(this, CMD_ROOM_ATTEMPTED_EXIT, buf, roomp);
  if(rc==TRUE) // not allowed to move
    return;


  sprintf(buf, "You drag $N %s.", dirs[tdir]);
  act(buf, TRUE, this, NULL, o, TO_CHAR);
  sprintf(buf, "$n drags $N %s.", dirs[tdir]);
  act(buf, TRUE, this, NULL, o, TO_ROOM);
  oldroom = o->in_room;
  or = o->roomp->dir_option[tdir]->to_room;
  rp = real_roomp(or);
  --(*o);
  --(*this);
  *rp += *this;
  *rp += *o;
  act("$n enters the room dragging $N!", TRUE, this, NULL, o, TO_ROOM);
  TPCorpse *tmpcorpse = dynamic_cast<TPCorpse *>(o);
  if (tmpcorpse) {
    tmpcorpse->setRoomNum(or);
    tmpcorpse->saveCorpseToFile();
  }
  if (followers) {
    heap_top = 0;
    for (k = followers; k; k = n) {
      n = k->next;
      if ((oldroom == k->follower->in_room) && !k->follower->fight() &&
          (k->follower->getPosition() >= POSITION_CRAWLING)) {
        sprintf(buf, "You follow $N as $E drags $p out of the room.");
        act(buf, FALSE, k->follower, o, this, TO_CHAR); 
        if (k->follower->followers) {
          rc = k->follower->moveGroup(tdir);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete k->follower;
            k->follower = NULL;
            continue;
          }
        } else {
          if ((result = k->follower->rawMove(tdir))) {
            if (IS_SET_DELETE(result, DELETE_THIS)) {
              delete k->follower;
              k->follower = NULL;
              continue;
            }
            AddToCharHeap(heap_ptr, &heap_top, heap_tot, k->follower);
          }
        }
      }
    }
    for (i = 0; i < heap_top; i++) {
      if (heap_tot[i] > 1)
        rc = heap_ptr[i]->displayGroupMove(tdir, oldroom, heap_tot[i]);
      else
        rc = heap_ptr[i]->displayOneMove(tdir, oldroom);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete heap_ptr[i];
        heap_ptr[i] = NULL;
      }
    }
  }


  doLook("", CMD_LOOK);
  addToMove(-20);
  addToWait(combatRound(1));
}

void TBeing::doDrag(const char *arg)
{
  char caName[128], dir[128];
  TBeing *v;
  dirTypeT tdir;
  unsigned int bits;
  roomDirData *exitp;
  TObj *o;
  const char *syntax="Syntax : drag <object|person> <direction>\n\r";

  two_arg(arg, caName, dir);

  if (!*caName || !*dir) {
    sendTo(syntax);
    return;
  }

  bits = generic_find(caName, FIND_CHAR_ROOM | FIND_OBJ_ROOM, this, &v, &o);

  if(!bits){
    sendTo("You see nothing by that name to drag!\n\r");
    sendTo(syntax);
    return;
  } else {
    if (riding) {
      sendTo("You have to dismount first.\n\r");
      return;
    }
    if (!hasHands() || bothArmsHurt()) {
      sendTo("You need working arms and hands to drag things.\n\r");
      return;
    }
    if (getMove() < 20) {
      sendTo("You don't have the necessary movement to make the drag!\n\r");
      return;
    }
    tdir = getDirFromChar(dir);
    if (tdir == DIR_NONE) {
      sendTo("No such direction!\n\r");
      sendTo(syntax);
      return;
    }
    if (equipment[HOLD_LEFT] || equipment[HOLD_RIGHT]) {
      sendTo("Drag something with your hands full!?!\n\r");
      return;
    }
    if (isSwimming()) {
      sendTo("You can't drag while swimming.\n\r");
      return;
    }
  } 

  exitp = exitDir(tdir);

  if (!exit_ok(exitp, NULL) || IS_SET(exitp->condition, EX_CLOSED)) {
    sendTo("You are blocked from dragging %s.\n\r", dirs[tdir]);
    sendTo(syntax);
    return;
  }
  if (!validMove(tdir)) 
    return;
  
  if(bits==FIND_CHAR_ROOM){
    doDrag(v, tdir);
  } else if(bits==FIND_OBJ_ROOM){
    doDrag(o, tdir);
  } else {
    // not reached
    sendTo("You see nothing by that name to drag!\n\r");
    sendTo(syntax);
    return;
  }

  return;
}


void TBeing::doResetMargins()
{
  cls();
  fullscreen();
  sendTo("Margins reset.  Use CLS to restore old settings.\n\r");
}

void TBeing::doAuto(const char *buf)
{
  int i;
  char arg[160];

  if (!desc)
    return;
  buf = one_argument(buf, arg);
  if (!*arg) {
    for (i = 0;i < MAX_AUTO;i++) {
      if (!isImmortal() && ((unsigned int) (1<<i) == AUTO_SUCCESS))
        continue;
      if (*auto_name[i]) {
        sendTo("%-35s : %s\n\r",
               ((i == AUTO_TIPS && isImmortal()) ? "Advanced Menus" : auto_name[i]),
               ((IS_SET(desc->autobits, (unsigned) (1<<i))) ? "on" : "off"));
      }
    }
    return;
  }

  if (is_abbrev(arg, "autokill") || is_abbrev(arg, "kill")) {
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
  } else if (is_abbrev(arg, "tips")) {
    if (IS_SET(desc->autobits, AUTO_TIPS)) {
      sendTo("You will no longer see periodic tips.\n\r");
      REMOVE_BIT(desc->autobits, AUTO_TIPS);
    } else {
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
      sendTo("You will now default to fighting back if attacked and when you cast.\n
\r");
      sendTo("You are still free to engage rather than fight by using the engage
 command.\n\r");
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
  } else {
    for (i = 0;i < MAX_AUTO;i++) {
      if (!isImmortal() && ((unsigned int) (1<<i) == AUTO_SUCCESS))
        continue;
      if (*auto_name[i]) {
        sendTo("%-35s : %s\n\r", auto_name[i],
          ((IS_SET(desc->autobits, (unsigned) (1<<i))) ? "on" : "off"));
      }
    }
  }
}

void TBeing::doEmail(const char *arg)
{
  char buf[256];

  one_argument(arg, buf);

  if (!desc || !desc->account)
    return;

  if (!*buf) {
    sendTo("Your present email address is: %s\n\r", desc->account->email);
    return;
  }
  if (illegalEmail(buf, desc, SILENT_NO)) {
    return;
  }
  sendTo("Changing email address from %s to %s.\n\r",
           desc->account->email, buf);
  strcpy(desc->account->email, buf);
  desc->saveAccount();
}

void TBeing::doList(const char *arg)
{
  doNotHere();
  return;
}

void Descriptor::add_comment(const char *who, const char *msg) 
{
  time_t ct;
  char *tmstr;
  char buf[MAX_STRING_LENGTH];
  FILE *fp;
  int i, j;
  char cmd_buf[256];
  struct tm * lt;

  ct = time(0);
  lt = localtime(&ct);
  tmstr = asctime(lt);
  *(tmstr + strlen(tmstr) - 1) = '\0';

  sprintf(buf, "****** Comment from %s on %s:\n",
       character->getName(), tmstr);

  for (i= 0, j=strlen(buf); msg[i];i++) {
    if ((strlen(buf) < (MAX_STRING_LENGTH - 1)) && msg[i] != '\r') {
      // all \r skipped, concat to 1 line
      buf[j++] = ((msg[i] == '\n') ? ' ' : msg[i]);
    }
  }
  buf[j] = '\0';

  sprintf(cmd_buf, "account/%c/%s/comment", LOWER(who[0]), lower(who).c_str());

  if (!(fp = fopen(cmd_buf, "a+"))) {
    perror("doComment");
    character->sendTo("Could not open the comment-file.\n\r");
    return;
  }

  fputs(buf, fp);
  fputs("\n", fp);
  fclose(fp);
}

void Descriptor::send_bug(const char *type, const char *msg) 
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char cmd_buf[256];
  time_t ct;
  char *tmstr;
  FILE *fp;
  int i, j, k;
  const char * const  BUG_TEMP_FILE = "bug.temp";
  struct tm * lt;

  // first off, write the "Subject: " information
  for (i=0, k=0; msg[i] && msg[i] != '\n';i++) {
    if (msg[i] != '\r') {
      // all \r skipped
      buf2[k++] = msg[i];
    }
  }
  buf2[k] = '\0';  // clean terminate

  strcat(buf2, "\n");
  if (msg[i] == '\n')
    i++;

  ct = time(0);
  lt = localtime(&ct);
  tmstr = asctime(lt);
  *(tmstr + strlen(tmstr) - 1) = '\0';

  sprintf(buf3, "****** %s from %s in room %d %s%son %s:\n",
       type,
       IS_SET(account->flags, ACCOUNT_IMMORTAL) ? "(immortal)" : character->getName(),
       character->inRoom(), 
       (gamePort == PROD_GAMEPORT ? "" : (gamePort == 7901 ? "(gamma) " : "(beta) ")), 
       (!client ? "" : "(client) "), 
       tmstr);
  strcpy(buf, buf3);
  strcat(buf, buf2);
  strcat(buf2, buf3);

  // this is advanced over teh subject line (i initted from above)
  for (j=strlen(buf), k=strlen(buf2); msg[i];i++) {
    if ((strlen(buf) < (MAX_STRING_LENGTH - 1)) && msg[i] != '\r') {
      // all \r skipped
      buf[j++] = ((msg[i] == '\n') ? ' ' : msg[i]);
      buf2[k++] = msg[i];
    }
  }
  buf[j] = '\0';
  buf2[k] = '\0';

  if (!strcmp(type, "Idea")) {
    if (!(fp = fopen(IDEA_FILE, "a"))) {
      perror("doIdea");
      character->sendTo("Could not open the idea-file.\n\r");
      return;
    }
  } else if (!strcmp(type, "Bug")) {
    if (!(fp = fopen(BUG_FILE, "a"))) {
      perror("doBug");
      character->sendTo("Could not open the bug-file.\n\r");
      return;
    }
  } else if (!strcmp(type, "Typo")) {
    if (!(fp = fopen(TYPO_FILE, "a"))) {
      perror("doTypo");
      character->sendTo("Could not open the typo-file.\n\r");
      return;
    }
  } else {
    vlogf(9, "Bogus type (%s) in send_bug.", type);
    return;
  }
  fputs(buf, fp);
  fputs("\n", fp);
  fclose(fp);

//  ----------

  if (!(fp = fopen(BUG_TEMP_FILE, "w"))) {
    vlogf(8, "Error opening dummy file for bug mailing.");
    return;
  }
  fputs(buf2, fp);
  fclose(fp);

  if (!strcmp(type, "Idea")) {
#if 1
    // send ideas to the listserv
    sprintf(cmd_buf,  "/usr/lib/sendmail -f%s %s < %s",
// can't send from the user's email.  closed list = bounce if not subbed
//         account->email,
         "ideas@sneezy.stanford.edu",
         "Sneezy_ideas@LIB01.ferris.edu",
          BUG_TEMP_FILE);
    vsystem(cmd_buf);
#elif 0
    // send ideas to the listserv moderator
    sprintf(cmd_buf,  "/usr/lib/sendmail -f%s %s < %s",
         "ideas@sneezy.stanford.edu",
         "tunaboo@hotmail.com",
          BUG_TEMP_FILE);
    vsystem(cmd_buf);
#else
    // send ideas to the coders
    // if they are going to the listserv, don't send it twice (bat & brut)
    sprintf(cmd_buf,  "/usr/lib/sendmail -f%s batopr russrussell@icqmail.com lapsos < %s",
         account->email, BUG_TEMP_FILE);
    vsystem(cmd_buf);
#endif
  }
  if (!strcmp(type, "Bug")) {
    // send bugs to the coders
    sprintf(cmd_buf,  "/usr/lib/sendmail -f%s batopr russrussell@icqmail.com lapsos < %s",
         account->email, BUG_TEMP_FILE);
    vsystem(cmd_buf);
  }
  if (!strcmp(type, "Typo")) {
    // send typos to the lows
    sprintf(cmd_buf, "/usr/lib/sendmail -f%s low < %s",
            account->email, BUG_TEMP_FILE);
    vsystem(cmd_buf);
  }
}

void TBeing::doAfk()
{
  if (isPlayerAction(PLR_AFK)) {
    sendTo("Removing AFK setting.\n\r");
    remPlayerAction(PLR_AFK);
    return;
  } else {
    sendTo("You are now marked as AFK.\n\r");
    addPlayerAction(PLR_AFK);
    return;
  }
}

