//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// other.cc - Miscellaneous routines
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
#include "obj_drug.h"
#include "skillsort.h"
#include "obj_board.h"
#include "obj_spellbag.h"
#include "obj_base_corpse.h"
#include "obj_player_corpse.h"
#include "obj_drug_container.h"
#include "obj_component.h"
#include "obj_note.h"
#include "disc_air.h"
#include "disc_alchemy.h"
#include "disc_animal.h"
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
#include "disc_shaman_armadillo.h"
#include "disc_shaman_frog.h"
#include "disc_shaman_healing.h"
#include "disc_shaman_skunk.h"
#include "disc_shaman_spider.h"
#include "disc_shaman_control.h"
#include "disc_murder.h"
#include "obj_trap.h"
#include "obj_light.h"
#include "obj_armor.h"
#include "obj_armor_wand.h"
#include "spelltask.h"
#include "obj_magic_item.h"
#include "obj_potion.h"
#include "obj_scroll.h"
#include "database.h"
#include "disc_cures.h"
#include "format.h"
#include "liquids.h"

void TBeing::doGuard(const sstring &argument)
{
  if (isPc()) {
    sendTo("Sorry, you can't just put your brain on autopilot!\n\r");
    return;
  }

  if (argument.empty()) {
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
    if(argument.lower() == "on"){
      if (!IS_SET(specials.act, ACT_GUARDIAN)) {
        SET_BIT(specials.act, ACT_GUARDIAN);
        act("$n alertly watches you.", FALSE, this, 0, master, TO_VICT);
        act("$n alertly watches $N.", FALSE, this, 0, master, TO_NOTVICT);
        sendTo("You snap to attention.\n\r");
      }
    } else if (argument.lower() == "off"){
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

int TBeing::doNoJunk(const char *argument, TObj *obj)
{
  char arg[100], newarg[100];
  TObj *o;
  int num, p, count;


  strcpy(arg, argument);
  if (!*arg && !obj) {
    sendTo("Set the nojunk flag on what?\n\r");
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
      t_o = searchLinkedListVis(this, arg, getStuff());
      o = dynamic_cast<TObj *>(t_o);
    }
    if (o) {
      if(o->isObjStat(ITEM_NOJUNK_PLAYER)) {
	o->remObjStat(ITEM_NOJUNK_PLAYER);
	act("You remove the no-junk flag from $p.", TRUE, this, o, NULL, TO_CHAR);
      } else {
	o->addObjStat(ITEM_NOJUNK_PLAYER);
	act("You set the no-junk flag on $p.", TRUE, this, o, NULL, TO_CHAR);
      }
      count++;
      if (num > 0)
	num--;
      else if(num==-1)
	break;

    } else 
      break;
  }
  if (count)
    return TRUE;

  sendTo("No-Junk what?\n\r");
  return FALSE;
}




int TBeing::doJunk(const char *argument, TObj *obj)
{
  char arg[100], newarg[100];
  TObj *o;
  int num, p, count;
  TThing *t;

  strcpy(arg, argument);
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
      t_o = searchLinkedListVis(this, arg, getStuff());
      o = dynamic_cast<TObj *>(t_o);
    }
    if (o) {
      if (o->isObjStat(ITEM_NOJUNK_PLAYER)) {
	sendTo("You can't junk that, someone has set a no-junk flag on it!  HELP NOJUNK\n\r");
        return FALSE;
      }
      if (o->isObjStat(ITEM_NODROP)) {
        sendTo("You can't let go of it, it must be CURSED!\n\r");
        return FALSE;
      }
      if (o->isPersonalized()) {
        sendTo("Monogrammed items can't be junked.\n\r");
        return FALSE;
      }
      if (o->getStuff() && desc && (desc->autobits & AUTO_POUCH)) {
        sendTo("There is still stuff in there, you choose not to junk it.\n\r");
        return FALSE;
      }
      for (t = o->getStuff(); t; t = t->nextThing) {
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
      for (t = o->getStuff(); t; t = t->nextThing) 
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
  vector<sstring>cmdVec(0);

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
  sstring str;
  str = "The following commands are available:\n\r\n\r";

  for (num = 0; num < cmdVec.size(); num++) {
    sprintf(buf, "%-11s", cmdVec[num].c_str());
    str += buf;
    if ((num%7) == 6)
      str += "\n\r";
  }

  sprintf(buf, "\n\r\n\rTotal number of commands: %u\n\r", cmdVec.size());
  str += buf;

  desc->page_string(str);
}

static int splitShares(const TBeing *ch, const TBeing *k)
{
  if (!ch->inGroup(*k) ||
      !ch->sameRoom(*k))
    return 0;

  if (k->desc)
    return k->desc->session.group_share;

  // what's left are basically mobs
  // pets and mounts are not forced to fight, so if they do, give 1 share
//  if (k->isMount() || k->isPet(PETTYPE_PET))
//    return 1;
// Took out splitting with pets 4/25/01 -- Cos
  return 0;
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

  one_argument(argument, buf, cElements(buf));

  if (is_number(buf)) {
    int amount = 0;
    amount = convertTo<int>(buf);
    if (amount < 0) {
      sendTo("Sorry, you can't do that!\n\r");
      return;
    }
    if (!(k = master))
      k = this;

    // If I am here, I should get my share.
    no_members = splitShares(this, k);

    int num_pc_foll = 0;
    for (f = k->followers; f; f = f->next) {
      no_members += splitShares(this, f->follower);
      // took out splitting with pets 4/25/01 -- cos
        // still need loop no matter what, the no_members tracks total
      if ((k == this)) {
        if (f->follower->desc)
	  num_pc_foll++;
      }
    }

    if ((!master && !num_pc_foll) || (no_members <= 1) || !isAffected(AFF_GROUP)) {
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
      int myshares = splitShares(this, this);
      tmp_amount = amount * (no_members - myshares) / no_members;

      addToMoney(-tmp_amount, GOLD_XFER);
    }

    if (k->isAffected(AFF_GROUP) && sameRoom(*k) && k != this) {
      int myshares = splitShares(this, k);
      tmp_amount = amount * myshares / no_members;

#if 0
      // too easy, find something better
      // maybe a timer to prevent overuse?
      reconcileHelp(k, (double) tmp_amount/100000);
#endif

      k->addToMoney(tmp_amount, GOLD_XFER);
      if (k->getPosition() >= POSITION_RESTING) 
        k->sendTo(COLOR_MOBS, fmt("%s splits %d talens, and you receive %d of them.\n\r") %
                    getName() % amount % tmp_amount);
    }
    for (f = k->followers; f; f = f->next) {
      if (f->follower->isAffected(AFF_GROUP) && sameRoom(*f->follower) && f->follower != this) {
        int myshares = splitShares(this, f->follower);
        tmp_amount = amount * myshares / no_members;
        
#if 0
        reconcileHelp(f->follower, (double) tmp_amount/100000);
#endif

        f->follower->addToMoney(tmp_amount, GOLD_XFER);
        if (f->follower->getPosition() >= POSITION_RESTING)
          f->follower->sendTo(COLOR_MOBS, fmt("%s splits %d talens, and you receive %d of them.\n\r") % getName() % amount % tmp_amount);
      }
    }
    sendTo(fmt("%d talens divided in %d shares of %d talens.\n\r") % amount % no_members % (amount / no_members));
    if ((GetMaxLevel() > MAX_MORT) && (no_members > 1))
      vlogf(LOG_MISC, fmt("%s was kind enough to share %d talens with others...") %  getName() % amount);
  } else {
    int count = 0;
    TThing *obj2;
    if ((!(obj2 = searchLinkedListVis(this, buf, getStuff(), &count)) &&
         !(obj2 = searchLinkedListVis(this, buf, roomp->getStuff(), &count))) ||
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
    vlogf(LOG_BUG, fmt("Person %s in bad room in doReport!") %  getName());
    return;
  }
  if (applySoundproof())
    return;

  if (!canSpeak()) {
    sendTo("You can't make a sound!\n\r");
    return;
  }
  memset(target, '\0', sizeof(target));
  strcpy(target, argument);
  if (hasClass(CLASS_CLERIC) || hasClass(CLASS_DEIKHAN))
    sprintf(info, "$n reports '%s%.1f%% H, %.2f%% P. I am %s%s'",
           red(), getPercHit(), getPiety(),
           DescMoves((((double) getMove()) / ((double) moveLimit()))),
           norm());
  else if (hasClass(CLASS_SHAMAN))
    sprintf(info, "$n reports '%s%.1f%% H, %-4d LF. I am %s%s'",
           red(), getPercHit(), getLifeforce(),
           DescMoves((((double) getMove()) / ((double) moveLimit()))),
           norm());
  else
    sprintf(info, "$n reports '%s%.1f%% H, %.1f%% M. I am %s%s'", 
           red(), getPercHit(), getPercMana(), 
           DescMoves((((double) getMove()) / ((double) moveLimit()))), 
           norm());


  for (t = roomp->getStuff(); t; t = t2) {
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
    sendTo(COLOR_MOBS, fmt("You report your status to %s.\n\r") % targ->getName());
    if (hasClass(CLASS_CLERIC) || hasClass(CLASS_DEIKHAN))
      sprintf(info, "<G>$n directly reports to you  '%s%.1f%% H, %.2f%% P. I am %s%s'<1>",
           red(), getPercHit(), getPiety(),
           DescMoves((((double) getMove()) / ((double) moveLimit()))),
           norm());
    else if (hasClass(CLASS_SHAMAN))
      sprintf(info, "<G>$n directly reports to you  '%s%.1f%% H, %-4d LF. I am %s%s'<1>",
           red(), getPercHit(), getLifeforce(),
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
  if (GetMaxLevel() < MAX_NEWBIE_LEVEL) {
    sendTo(fmt("You must be level %i before you can change your title.\n\r") %
	   MAX_NEWBIE_LEVEL);
    return;
  }

  for (; isspace(*argument); argument++);

  if (*argument) {
    sstring str = argument;

    // Basic name sake checks
    if (str.find("<n>") == sstring::npos &&
        colorString(this, desc, argument, NULL, COLOR_NONE, TRUE).find(getNameNOC(this).c_str()) ==
        sstring::npos) {
      sendTo(fmt("Your %s or <n> Must appear somewhere in here.\n\r") % getNameNOC(this));
      return;
    }

    // keep morts from using flash in their titles
    if (!isImmortal()) {
      while (str.find("<f>") != sstring::npos) {
        str.replace(str.find("<f>"), 3, "");
      }
      while (str.find("<F>") != sstring::npos) {
        str.replace(str.find("<F>"), 3, "");
      }
    }


    sstring stmp=nameColorString(this, desc, str.c_str(), NULL, COLOR_BASIC, FALSE);
    stmp=stripColorCodes(stmp);

    if (stmp.length() > 79) {
      sendTo("Title size is limited to 80 or less characters.\n\r");
      return;
    }    

    delete [] title;
    title = mud_str_dup(str);

    sendTo(fmt("Your title has been set to : <%s>\n\r") % str);
  } else {
    sendTo("The best title is the original anyway right? :)\n\r");
    setTitle(true);
  }
}

void TBeing::doQuit()
{
  sendTo(fmt("To leave SneezyMUD, please %srent%s at the nearest inn.  To find the nearest inn, type \"goto inn\".\n\r") % red() % norm());
  sendTo(fmt("%sQuitting%s from the game will make you lose %sALL%s equipment and talens.\n\r") %
           red() % norm() % blue() % norm());
  sendTo(fmt("If you are sure you would like to do this, type %squit!%s instead of quit.\n\r") %
           blue() % norm());
}

int TMonster::doQuit2()
{
  return FALSE;
}

int TPerson::doQuit2()
{
  int rc;
  // char wizbuf[256];

  doSave(SILENT_YES);

  if (!desc || isAffected(AFF_CHARM))
    return FALSE;

  if (fight()) {
    sendTo("No way! You are fighting.\n\r");
    return FALSE;
  }
  if (getPosition() < POSITION_STUNNED) {
    sendTo("You die before your time!\n\r");
    vlogf(LOG_MISC, fmt("%s killed by quitting while incapacitated at %s (%d)") % 
          getName() % roomp->getName() % inRoom());
    rc = die(DAMAGE_NORMAL);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  fullscreen();
  cls();

  act("Goodbye, friend.. Come back soon!", FALSE, this, 0, 0, TO_CHAR);
  act("$n has left the game.", TRUE, this, 0, 0, TO_ROOM);
  vlogf(LOG_PIO, fmt("%s quit the game at %s (%d).") %  
       getName() % roomp->name % inRoom());
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
#if 0
  // there is an intrinsic round off error present in float
  // in a get all (or similar) situation, this can propigate to a sizeable
  // amount throwing off the weight.  This function will reset the values
  // appropriately.

  float rw = 0.0, bw;
  int rv = 0, bv;
  TThing *t, *t2;
  for (t = getStuff(); t; t = t->nextThing) {
    bv = 0;
    bw = 0.0;
    for (t2 = t->getStuff(); t2; t2 = t2->nextThing) {
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
#endif
}

void TBeing::doNotHere() const
{
  sendTo("Sorry, but you cannot do that here!\n\r");
}

static const sstring describe_practices(TBeing *ch)
{
  sstring buf = "";

  for(int i=0; i<MAX_CLASSES; i++){
    if(ch->hasClass(ch->getClassNum((classIndT)i)) || ch->practices.prac[i]){
      buf += fmt("You have %d %s practice%s left.\n\r") %
        ch->practices.prac[i] % classInfo[i].name %
        ((ch->practices.prac[i] == 1) ? "" : "s");
    }
  }

  return buf;
}


sstring print_discipline(TBeing *tb, discNumT i)
{
  if(i==-1)
    return "";

  CDiscipline *cd=tb->getDiscipline(i);
  sstring buf="";
  sstring col;

  if (cd && (tb->isImmortal() || cd->ok_for_class || cd->getLearnedness())) {
    if (cd->getLearnedness()) {
      if(cd->getLearnedness()==100){
	col="<Y>";
      } else {
	col="<o>";
      }

      if (cd->getLearnedness() == cd->getNatLearnedness()) {
	buf+=fmt("%30s : Learnedness: %s%3d%s<1>\n\r") %
	  discNames[i].properName % col % cd->getLearnedness() % "%";
      } else {
	buf+=fmt("%30s : Learnedness: Current (%1%3d%s<1>) Natural (%3d%s)\n\r") %
	  discNames[i].properName % col % cd->getLearnedness() % "%" %
	  cd->getNatLearnedness() % "%";
      }
    } else {
      buf+=fmt("%30s : Learnedness: <k>Unlearned<1>\n\r") %
	discNames[i].properName;
    }
  }
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
    sendTo("Polymorph types can't use the practice command.");
    return;
  }
  for (; isspace(*argument); argument++);

  if (!argument || !*argument) {
    sstring sbuf;
    sbuf="<r>Basic disciplines:<1>\n\r";
    sbuf+=print_discipline(this, classInfo[bestClass()].base_disc);
    sbuf+=print_discipline(this, classInfo[bestClass()].sec_disc);
    sbuf+=print_discipline(this, DISC_COMBAT);

    sbuf+="<r>Automatically learned disciplines:<1>\n\r";
    sbuf+=print_discipline(this, DISC_ADVENTURING);
    sbuf+=print_discipline(this, DISC_FAITH);
    sbuf+=print_discipline(this, DISC_RITUALISM);
    sbuf+=print_discipline(this, DISC_WIZARDRY);


    sbuf+="<r>Advanced disciplines:<1>\n\r";
    for (i=MIN_DISC; i < MAX_DISCS; i++) {
      if(i!=classInfo[bestClass()].base_disc &&
	 i!=classInfo[bestClass()].sec_disc &&
	 i!=DISC_COMBAT &&
	 i!=DISC_ADVENTURING &&
	 i!=DISC_FAITH &&
	 i!=DISC_RITUALISM &&
	 i!=DISC_WIZARDRY){
	sbuf+=print_discipline(this, i);
      }
    }

    sbuf+=describe_practices(this);
    d->page_string(sbuf);
    return;
  }
  argument = one_argument(argument, arg, cElements(arg));

  if (is_abbrev(arg, "class")) {
    int which = 0;
    argument = one_argument(argument, arg, cElements(arg));

    for(int j=0;j<MAX_CLASSES;++j){
      if(is_abbrev(arg, classInfo[j].name)){
	which=classInfo[j].class_num;
	break;
      }
    }

    if(!which){
      sendTo("That is not a valid class.\n\r");
      sendTo("Syntax: practice class <class>.\n\r");
      return;
    }

    sprintf(buf, "The following disciplines are valid:\n\r");
    for (i=MIN_DISC; i < MAX_DISCS; i++) {
      if (!strcmp(discNames[i].properName, "unused")) 
        continue;
      if (!strcmp(discNames[i].properName, "Psionic Abilities"))
	continue;
      if (!(cd = getDiscipline(i))) {
        vlogf(LOG_BUG, fmt("Somehow %s was not assigned a discipline (%d), used prac class (%d).") % getName() % i % which);
      }
      if ((discNames[i].class_num == 0) || (IS_SET(discNames[i].class_num, which))) {
        if (cd && cd->getLearnedness() >= 0) {
          sprintf(buf + strlen(buf), "%30s : (Learnedness: %3d%%)\n\r",
              discNames[i].properName, cd->getLearnedness());
        } else {
          sprintf(buf + strlen(buf), "%30s : (Learnedness: unlearned)\n\r",
                  discNames[i].properName);
        }
      }
    }
    sprintf(buf + strlen(buf), "%s", describe_practices(this).c_str());
    d->page_string(buf);
    return;
  }

  if (is_abbrev(arg, "discipline")) {
    argument = one_argument(argument, arg, cElements(arg));
    if (!*arg) {
      sendTo("You need to specify a discipline: practice discipline <discipline> <class>.\n\r");
      return;
    } else {
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
      if (strlen(argument) > 3 && is_abbrev(argument, "wizardry")) 
        doPracSkill(argument, SKILL_WIZARDRY);
      else if (strlen(argument) > 3 && is_abbrev(argument, "ritualism")) 
        doPracSkill(argument, SKILL_RITUALISM);
      else if (strlen(argument) > 3 && is_abbrev(argument, "devotion")) 
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
    strcpy(skillbuf, discNames[i].name);
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

extern struct PolyType DisguiseList[];
static const int MaxDisguiseType = 10; // Non-Race Specific Ones

extern struct PolyType ShapeShiftList[];
static const int MaxShapeShiftType = 10;

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
      vlogf(LOG_BUG, fmt("Bad disc for skill %d in doPractice") %  i);
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
          (i != SKILL_WIZARDRY && i != SKILL_RITUALISM && i != SKILL_DEVOTION)) {
      sprintf(how_long, "(Learned: %s)", 
          skill_diff(discArray[i]->start - tmp_var));
    } else if (discArray[i]->toggle && !hasQuestBit(discArray[i]->toggle)) {
      if(i==SKILL_ADVANCED_KICKING){
	if(hasQuestBit(TOG_ELIGIBLE_ADVANCED_KICKING)){
	  strcpy(how_long, "(Learned: When Teacher is Found)");
	} else {
	  strcpy(how_long, "(Learned: Not Eligible Yet)");
	}
      } else {
	strcpy(how_long, "(Learned: When Teacher is Found)");
      }
    } else if ((i == SKILL_WIZARDRY)) {
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
        strcpy(how_long, "\tcomponent=any hand, inventory, waist, wrist, or neck; no speak; no gestures");
      } else if (wiz_lev >= WIZ_LEV_COMP_NECK) {
        strcpy(how_long, "\tcomponent=any hand, inventory, waist, wrist, or neck; no speak; no gestures");
      } else if (wiz_lev >= WIZ_LEV_COMP_WRIST) {
        strcpy(how_long, "\tcomponent=any hand, inventory, waist, wrist, or neck; no speak; no gestures");
      }
    } else if ((i == SKILL_RITUALISM)) {
      ritualismLevelT wiz_lev = getRitualismLevel();
      if (wiz_lev < RIT_LEV_COMP_PRIM_OTHER_FREE) {
        if (isRightHanded())
          sprintf(how_long, "(Learned: %s)\tcomponent=right hand, left hand=free", skill_diff(discArray[i]->start - tmp_var));
        else
          sprintf(how_long, "(Learned: %s)\tcomponent=left hand, right hand=free", skill_diff(discArray[i]->start - tmp_var));
      } else if (wiz_lev == RIT_LEV_COMP_PRIM_OTHER_FREE) {
        if (isRightHanded())
          strcpy(how_long, "\tcomponent=right hand, left hand=free");
        else
          strcpy(how_long, "\tcomponent=left hand, right hand=free");
      } else if (wiz_lev == RIT_LEV_COMP_EITHER_OTHER_FREE) {
        strcpy(how_long,   "\tcomponent=either hand, other free");
      } else if (wiz_lev == RIT_LEV_COMP_EITHER) {
        strcpy(how_long, "\tcomponent=any hand");
      } else if (wiz_lev == RIT_LEV_COMP_INV) {
        strcpy(how_long, "\tcomponent=any hand or inventory");
      } else if (wiz_lev == RIT_LEV_NO_GESTURES) {
        strcpy(how_long, "\tcomponent=any hand or inventory; no gestures");
      } else if (wiz_lev == RIT_LEV_NO_MANTRA) {
        strcpy(how_long, "\tcomponent=any hand or inventory; no speak; no gestures");
      } else if (wiz_lev >= RIT_LEV_COMP_BELT) {
        strcpy(how_long, "\tcomponent=any hand, inventory, waist, wrist, or neck; no speak; no gestures");
      } else if (wiz_lev >= RIT_LEV_COMP_NECK) {
        strcpy(how_long, "\tcomponent=any hand, inventory, waist, wrist, or neck; no speak; no gestures");
      } else if (wiz_lev >= RIT_LEV_COMP_WRIST) {
        strcpy(how_long, "\tcomponent=any hand, inventory, waist, wrist, or neck; no speak; no gestures");
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
    } else if (i == SPELL_SHAPESHIFT) {

      strcpy(how_long, "\n\r\t");
      strcpy(how_long, "\n\r\tYou may ShapeShift into the following creatures:\n\r\t");
      for (int tCount = 0; tCount < MaxShapeShiftType; tCount++) {
        if (ShapeShiftList[tCount].learning > getSkillValue(SPELL_SHAPESHIFT) ||
            ShapeShiftList[tCount].level    > GetMaxLevel())
          continue;

        if ((signed) ShapeShiftList[tCount].tRace != RACE_NORACE)
          continue;

        sstring tStArg(ShapeShiftList[tCount].name),
               tStRes("");

        one_argument(tStArg, tStRes);

        if (strlen(how_long) != 3)
          strcat(how_long, " - ");

        strcat(how_long, tStRes.c_str());
      }

      if (strlen(how_long) == 3)
        strcpy(how_long, " ");
    } else { 
      strcpy(how_long, " ");
    }

    if (!isImmortal()) {
      if (doesKnowSkill(i)) {
        if ((i == SKILL_WIZARDRY) || (i == SKILL_RITUALISM) || (i == SKILL_DEVOTION)) {
            sprintf(buf, "%s%-25.25s%s   Current: %-15s\n\r%-15s.\n\r",
                   cyan(), discArray[i]->name, norm(),
                   how_good(getSkillValue(i)), how_long);
        } else if (getMaxSkillValue(i) < MAX_SKILL_LEARNEDNESS) {
          if (discArray[i]->startLearnDo > 0) {
            sprintf(buf, "%s%-25.25s%s   Current: %-12s Potential: %-12s%s\n\r",
                 cyan(), discArray[i]->name, norm(), 
                 how_good(getSkillValue(i)),
                 how_good(getMaxSkillValue(i)), how_long);
          } else {
            sprintf(buf, "%s%-25.25s%s   Current: %-15s%s\n\r",
                   cyan(), discArray[i]->name, norm(), 
                   how_good(getSkillValue(i)), how_long);
          }
        } else {
          sprintf(buf, "%s%-25.25s%s   Current: %-15s%s\n\r",
            cyan(), discArray[i]->name, norm(), 
            how_good(getSkillValue(i)), how_long);
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
  d->page_string(buffer);
}

void TBeing::doPracSkill(const char *argument, spellNumT skNum)
{
  spellNumT skill = TYPE_UNDEFINED;
  int found = 0;
  int wiz = FALSE;
  char buf[256];
  char how_long[256];
  int tmp_var = FALSE;
  CDiscipline *cd;
  discNumT das;

  if (!*argument && skNum == TYPE_UNDEFINED) 
    return;

  if ((skNum == SKILL_WIZARDRY)) {
    if (hasClass(CLASS_MAGE) ||
	hasClass(CLASS_RANGER)) {
      found=2;
      wiz = 1;
    } else {
      sendTo("You do not know about Wizardry.\n\r");
      found = 1;
    }
  } else if ((skNum == SKILL_RITUALISM)) {
    if (hasClass(CLASS_SHAMAN)) {
      found=2;
      wiz = 1;
    } else {
      sendTo("You do not know about Ritualism.\n\r");
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
    vlogf(LOG_BUG, "Something is sending to doPracSkill with a bad argument.");
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
    sendTo(fmt("Skill %s: Unlearned. You can only use practice skill about skills that you have learned.\n\r") % discArray[skill]->name);
    return;
  } else if (!wiz) {
    sendTo("That does not appear to be a valid skill: practice skill <name>\n\r");
    return;
  } 

  das = getDisciplineNumber(skNum, FALSE);
  if (das == DISC_NONE) {
    vlogf(LOG_BUG, fmt("Bad disc for skill %d in doPracSkill") %  skNum);
    return;
  }
  cd = getDiscipline(das);
  tmp_var = ((!cd || cd->getLearnedness() <= 0) ? 0 : cd->getLearnedness());
  tmp_var = max((int) MAX_DISC_LEARNEDNESS, tmp_var);

  if (wiz == 1) {
    if (hasClass(CLASS_SHAMAN)) {
      ritualismLevelT wiz_lev = getRitualismLevel();
      if (wiz_lev < RIT_LEV_COMP_PRIM_OTHER_FREE) {
	if (isRightHanded()) {
	  sprintf(how_long, "(Learned: %s)\tcomponent=right hand, left hand=free.\n\r", skill_diff(discArray[skNum]->start - tmp_var));
	} else {
	  sprintf(how_long, "(Learned: %s)\tcomponent=left hand, right hand=free.\n\r", skill_diff(discArray[skNum]->start - tmp_var));
	}
	sendTo(COLOR_BASIC, how_long);
	return;
      } else if (wiz_lev == RIT_LEV_COMP_PRIM_OTHER_FREE) {
	if (isRightHanded())
	  strcpy(how_long, "\tcomponent=right hand, left hand=free.\n\r");
	else
	  strcpy(how_long, "\tcomponent=left hand, right hand=free.\n\r");
      } else if (wiz_lev == RIT_LEV_COMP_EITHER_OTHER_FREE) {
	strcpy(how_long,   "\tcomponent=either hand, other free.\n\r");
      } else if (wiz_lev == RIT_LEV_COMP_EITHER) {
	strcpy(how_long, "\tcomponent=any hand.\n\r");
      } else if (wiz_lev == RIT_LEV_COMP_INV) {
	strcpy(how_long, "\tcomponent=any hand or inventory.\n\r");
      } else if (wiz_lev == RIT_LEV_NO_GESTURES) {
	strcpy(how_long, "\tcomponent=any hand or inventory; no gestures.\n\r");
      } else if (wiz_lev == RIT_LEV_NO_MANTRA) {
	strcpy(how_long, "\tcomponent=any hand or inventory; no speak; no gestures.\n\r");
      } else if (wiz_lev >= RIT_LEV_COMP_BELT) {
	strcpy(how_long, "\tcomponent=any hand, waist, wrist, neck, or inventory; no speak; no gestures.\n\r");
      } else if (wiz_lev >= RIT_LEV_COMP_NECK) {
	strcpy(how_long, "\tcomponent=any hand, waist, wrist, neck, or inventory; no speak; no gestures.\n\r");
      } else if (wiz_lev >= RIT_LEV_COMP_WRIST) {
	strcpy(how_long, "\tcomponent=any hand, waist, wrist, neck, or inventory; no speak; no gestures.\n\r");
      }
      sendTo(COLOR_BASIC, how_long);
    } else {
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
	strcpy(how_long, "\tcomponent=any hand, waist, wrist, neck, or inventory; no speak; no gestures.\n\r");
      } else if (wiz_lev >= WIZ_LEV_COMP_NECK) {
	strcpy(how_long, "\tcomponent=any hand, waist, wrist, neck, or inventory; no speak; no gestures.\n\r");
      } else if (wiz_lev >= WIZ_LEV_COMP_WRIST) {
	strcpy(how_long, "\tcomponent=any hand, waist, wrist, neck, or inventory; no speak; no gestures.\n\r");
      }
      sendTo(COLOR_BASIC, how_long);
    }
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
    if (!*discNames[i].properName || !(*discNames[i].name)) {
      continue;
    }
    if (!is_abbrev(arg, discNames[i].name, MULTIPLE_YES)) {
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


void TBeing::doIdea(const sstring &)
{
  sendTo("Monsters can't have ideas - Go away.\n\r");
  return;
}

void TPerson::doIdea(const sstring &arg)
{
  if (fight())  {
    sendTo("You cannot perform that action while fighting!\n\r");
    return;
  }
  if (hasWizPower(POWER_SEE_COMMENTARY) && isImmortal() && !arg.empty()) {
    desc->start_page_file(IDEA_FILE, "Players aint saying nothin\'.\n\r");
    return;
  }
  idea_used_num++;
  if (!desc->m_bIsClient)
    sendTo("Write the subject of your idea then hit return.\n\r");

  if (!desc->m_bIsClient) {
    addPlayerAction(PLR_BUGGING);
    desc->connected = CON_WRITING;
    strcpy(desc->name, "Idea");
    desc->str = new const char *('\0');
    desc->max_str = MAX_MAIL_SIZE;
  } else
    desc->clientf(fmt("%d") % CLIENT_IDEA);
}

void TBeing::doTypo(const sstring &)
{
  sendTo("Monsters can't spell - leave me alone.\n\r");
  return;
}

void TPerson::doTypo(const sstring &arg)
{
  if (fight())  {
    sendTo("You cannot perform that action while fighting!\n\r");
    return;
  }
  if (hasWizPower(POWER_SEE_COMMENTARY) && isImmortal() && !arg.empty()) {
    desc->start_page_file(TYPO_FILE, "Players can't spell worth nothin\'.\n\r");
    return;
  }

  typo_used_num++;
  if (!desc->m_bIsClient)
    sendTo("Write the subject of your typo then hit return.\n\r");

  if (!desc->m_bIsClient) {
    addPlayerAction(PLR_BUGGING);
    desc->connected = CON_WRITING;
    strcpy(desc->name, "Typo");
    desc->str = new const char *('\0');
    desc->max_str = MAX_MAIL_SIZE;
  } else
    desc->clientf(fmt("%d") % CLIENT_TYPO);
}

void TBeing::doBug(const sstring &)
{
  sendTo("You are a monster! Bug off!\n\r");
  return;
}

void TPerson::doBug(const sstring &arg)
{
  if (fight())  {
    sendTo("You cannot perform that action while fighting!\n\r");
    return;
  }
  if (hasWizPower(POWER_SEE_COMMENTARY) && isImmortal() && !arg.empty()) {
    desc->start_page_file(BUG_FILE, "Players aren't saying anything.\n\r");
    return;
  }

  bug_used_num++;
  if (!desc->m_bIsClient) 
    sendTo("Write the subject of your bug then hit return.\n\r");
  
  if (!desc->m_bIsClient) {
    addPlayerAction(PLR_BUGGING);
    desc->connected = CON_WRITING;
    strcpy(desc->name, "Bug");
    desc->str = new const char *('\0');
    desc->max_str = MAX_MAIL_SIZE;
  } else
    desc->clientf(fmt("%d") % CLIENT_BUG);
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

  argument = one_argument(argument, namebuf, cElements(namebuf));

  if (!*namebuf) {
    if (!isAffected(AFF_GROUP))
      sendTo("But you are a member of no group?!\n\r");
    else {
      if(master && master->desc){
	sendTo(COLOR_BASIC, fmt("%s consists of:\n\r\n\r") % 
	       master->desc->session.groupName);
      } else {
	sendTo(COLOR_BASIC, fmt("%s consists of:\n\r\n\r") % 
	       desc->session.groupName);
      }


      if (master)
        k = master;
      else
        k = this;

      sprintf(namebuf, "%s", (k != this ? k->getNameNOC(this).c_str() : "You"));
      if (k->isAffected(AFF_GROUP)) {// && canSee(k)) {  I changed this on 010398 Russ
        if (sameRoom(*k)) {
          tmp_share = splitShares(this, k);

          if (k->hasClass(CLASS_CLERIC) || k->hasClass(CLASS_DEIKHAN)) {
            sendTo(fmt("%s%-15.15s%s [%s%.1f%chp %.1f%cp. %s look%s %s.%s]\n\r\t%s%2d share%s talens, %.1f%c shares XP%s\n\r") % cyan() % sstring(namebuf).cap() % norm() % red() %
              (((double) (k->getHit())) / ((double) k->hitLimit()) * 100) % '%' %
              k->getPiety() % '%' %
              sstring(namebuf).cap().c_str() %
              (k != this ? "s" : "") %
              DescMoves((((double) k->getMove()) / ((double) k->moveLimit()))) %
              norm() % purple() %
              tmp_share % ((tmp_share == 1) ? "" : "s") %
		   k->getExpSharePerc() % '%' %
              norm());
          } else if (k->hasClass(CLASS_SHAMAN)) {
            sendTo(fmt("%s%-15.15s%s [%s%.1f%chp %-4d lf. %s look%s %s.%s]\n\r\t%s%2d share%s talens, %.1f%c shares XP%s\n\r") % cyan() % sstring(namebuf).cap() % norm() % red() %
              (((double) (k->getHit())) / ((double) k->hitLimit()) * 100) % '%' %
              k->getLifeforce() % 
              sstring(namebuf).cap() %
              (k != this ? "s" : "") %
              DescMoves((((double) k->getMove()) / ((double) k->moveLimit()))) %
              norm() % purple() %
              tmp_share % ((tmp_share == 1) ? "" : "s") %
              k->getExpSharePerc() % '%' %
              norm());
          } else {
            sendTo(fmt("%s%-15.15s%s [%s%.1f%chp %.1f%cm. %s look%s %s.%s]\n\r\t%s%2d share%s talens, %.1f%c shares XP%s\n\r") % cyan() % sstring(namebuf).cap() % norm() % red() %
              (((double) (k->getHit())) / ((double) k->hitLimit()) * 100) % '%' %
              (((double) (k->getMana())) / ((double) k->manaLimit()) * 100) % '%' %
              sstring(namebuf).cap() %
              (k != this ? "s" : "") %
              DescMoves((((double) k->getMove()) / ((double) k->moveLimit()))) %
              norm() % purple() %
              tmp_share % ((tmp_share == 1) ? "" : "s") % 
              k->getExpSharePerc() % '%' %
              norm());
          }
        } else 
          sendTo(fmt("%-15.15s [not around]\n\r") % sstring(namebuf).cap());
      }
      for (f = k->followers; f; f = f->next) {
        sprintf(namebuf, "%s", (f->follower != this ? f->follower->getNameNOC(this).c_str() : "You"));
        if (f->follower->isAffected(AFF_GROUP) && canSee(f->follower)) {
          tmp_share = splitShares(this, f->follower);

          if (sameRoom(*f->follower)) { 
            if (f->follower->hasClass(CLASS_CLERIC) || 
                f->follower->hasClass(CLASS_DEIKHAN))
              sendTo(fmt("%s%-15.15s%s [%s%.1f%chp %.1f%cp. %s look%s %s.%s]\n\r\t%s%2d share%s talens, %.1f%c shares XP%s\n\r") % cyan() % sstring(namebuf).cap() % norm() % red() %
                (((double) (f->follower->getHit())) / ((double) f->follower->hitLimit()) * 100) % '%' %
                f->follower->getPiety() % '%' %
                sstring(namebuf).cap() %
                (f->follower != this ? "s" : "") %
                DescMoves((((double) f->follower->getMove()) / ((double) f->follower->moveLimit()))) %
                norm() % purple() %
                tmp_share % ((tmp_share == 1) ? "" : "s") % 
                f->follower->getExpSharePerc() % '%' %
                norm());
            else if (f->follower->hasClass(CLASS_SHAMAN))
              sendTo(fmt("%s%-15.15s%s [%s%.1f%chp %-4d lf. %s look%s %s.%s]\n\r\t%s%2d share%s talens, %.1f%c shares XP%s\n\r") % cyan() % sstring(namebuf).cap() % norm() % red() %
                (((double) (f->follower->getHit())) / ((double) f->follower->hitLimit()) * 100) % '%' %
                f->follower->getLifeforce() %
                sstring(namebuf).cap() %
                (f->follower != this ? "s" : "") %
                DescMoves((((double) f->follower->getMove()) / ((double) f->follower->moveLimit()))) %
                norm() % purple() %
                tmp_share % ((tmp_share == 1) ? "" : "s") % 
                f->follower->getExpSharePerc() % '%' %
                norm());

            else {
              sendTo(fmt("%s%-15.15s%s [%s%.1f%chp %.1f%cm. %s look%s %s.%s]\n\r\t%s%2d share%s talens, %.1f%c shares XP%s\n\r") % cyan() % sstring(namebuf).cap() % norm() % red() % 
                (((double) (f->follower->getHit())) / ((double) f->follower->hitLimit()) * 100) % '%' %
                (((double) (f->follower->getMana())) / ((double) f->follower->manaLimit()) * 100) % '%' %
                sstring(namebuf).cap() %
                (f->follower != this ? "s" : "") %
                DescMoves((((double) f->follower->getMove()) / ((double) f->follower->moveLimit()))) %
                norm() % purple() %
                tmp_share % ((tmp_share == 1) ? "" : "s") % 
                f->follower->getExpSharePerc() % '%' %
                norm());
            }
          } else 
            sendTo(fmt("%-15.15s [not around]\n\r") % sstring(namebuf).cap());
        }
      }
    }
    return;
  }
  if(is_abbrev(namebuf, "name")){
    if(!argument || !*argument){
      sendTo("Syntax: group name <name>\n\r");
      if(isAffected(AFF_GROUP)) {
	if(master){
	  sendTo(fmt("Current group name: %s") % 
		 master->desc->session.groupName);
	} else {
	  sendTo(fmt("Current group name: %s") % 
		 desc->session.groupName);
	}
      }

      return;
    }
    if (!isAffected(AFF_GROUP)) {
      sendTo("You don't seem to have a group.\n\r");
      return;
    }
    if (master) {
      sendTo("Only the group leader may set the group name.\n\r");
      return;
    }
    desc->session.groupName=fmt("%.60s<1>") % argument;

    sprintf(buf, "I have just set the group name to %s.", argument);
    doGrouptell(buf);
  } else if (is_abbrev(namebuf, "lots")) {
    if (!isAffected(AFF_GROUP) || master) {
      sendTo("Only the master of a group may throw lots.\n\r");
      return;
    }
    vector <sstring> gnames;
    gnames.push_back(name);
    for (f = followers; f; f = f->next) {
      if (!f->follower->isPc()) continue;
      gnames.push_back(f->follower->name);
    }
    int rnum = ::number(1,gnames.size()) -1;
    sstring sbuf = fmt("%s throws lots.  <Y>%s<z> is chosen.") % name % gnames[rnum];
    doGrouptell(sbuf.c_str());
    return;
  } else if (is_abbrev(namebuf, "share")) {
    int amt;

    if (!argument || !*argument) {
      sendTo("Syntax: group share <target> <amount>\n\r");
      return;
    }
    argument = one_argument(argument, namebuf, cElements(namebuf));
    if (!namebuf || !*namebuf) {
      sendTo("Syntax: group share <target> <amount>\n\r");
      return;
    }
    argument = one_argument(argument, buf, cElements(buf));
    if (!buf || !*buf) {
      sendTo("Syntax: group share <target> <amount>\n\r");
      return;
    }
    if ((amt = convertTo<int>(buf)) <= 0 || amt > 10) {
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
    one_argument(argument, typebuf, cElements(typebuf));
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
    for (t = roomp->getStuff();t;t = t->nextThing) {
      victim = dynamic_cast<TBeing *>(t);
      if (!victim)
        continue;

      if (victim->isAffected(AFF_GROUP) || !canSee(victim) || 
          (victim == riding))
        continue;

      if (IS_SET(victim->specials.act, ACT_IMMORTAL) && !victim->isPc() 
          && victim->master == this && this != victim )
      {
        sendTo(COLOR_MOBS, fmt("%s is immortal and has no need of you.  %s does not join your group.\n\r") % victim->getName() % victim->getName());
        continue;
      }
      
      if (victim->isPlayerAction(PLR_SOLOQUEST) && (this != victim)
          && (victim->master == this)) {
        sendTo(COLOR_MOBS, fmt("%s is on a quest! No grouping allowed!\n\r") % victim->getName());
        continue;
      }
      if (victim->isPlayerAction(PLR_GRPQUEST)) {
        if (!isPlayerAction(PLR_GRPQUEST)) {
          sendTo(COLOR_MOBS, fmt("%s is on a group quest that you aren't on! You can't group!\n\r") %victim->getName());
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
	if (hasClass(CLASS_SHAMAN)) {
          if (desc && (desc->m_bIsClient || IS_SET(desc->prompt_d.type, PROMPT_CLIENT_PROMPT))) 
	    desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(desc->character, desc, getName(), NULL, COLOR_NONE, FALSE) % getHit() % getLifeforce() % attack_modes[getCombatMode()]);
	} else { 
          if (desc && (desc->m_bIsClient || IS_SET(desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))
	    desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(desc->character, desc, getName(), NULL, COLOR_NONE, FALSE) % getHit() % getMana() % attack_modes[getCombatMode()]);
	}       
        if (victim->desc)
          victim->desc->session.group_share = 1;

        found = TRUE;
        continue;
      } else if (victim->master == this) {
        sendTo(COLOR_MOBS, fmt("You add %s to your group.\n\r") %victim->getName());
        act("$n adds $N to $s group.",TRUE,this,0,victim,TO_NOTVICT);
        victim->sendTo(COLOR_MOBS, fmt("You are now a member of %s's group.\n\r") %getName());
        SET_BIT(victim->specials.affectedBy, AFF_GROUP);
	if (hasClass(CLASS_SHAMAN)) {
	  if (desc && (desc->m_bIsClient || IS_SET(desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))
	    desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(desc->character, desc, victim->getName(), NULL, COLOR_NONE, FALSE) % victim->getHit() % victim->getLifeforce() % attack_modes[victim->getCombatMode()]);
	} else {
	  if (desc && (desc->m_bIsClient || IS_SET(desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))
	    desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(desc->character, desc, victim->getName(), NULL, COLOR_NONE, FALSE) % victim->getHit() % victim->getMana() % attack_modes[victim->getCombatMode()]);
	}        
        for (f = followers; f; f = f->next) {
          TBeing *b = f->follower;
          if (victim->desc && (victim->desc->m_bIsClient || IS_SET(victim->desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))  {
            victim->desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(victim, victim->desc, b->getName(), NULL, COLOR_NONE, FALSE) % b->getHit() % b->getMana() % attack_modes[b->getCombatMode()]);
          } 
          if (b->desc && (b->desc->m_bIsClient || IS_SET(b->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))) {
	    if (hasClass(CLASS_SHAMAN)) {
	      b->desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(b, b->desc, victim->getName(), NULL, COLOR_NONE, FALSE) % victim->getHit() % victim->getLifeforce() % attack_modes[victim->getCombatMode()]);
	    } else {
	      b->desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(b, b->desc, victim->getName(), NULL, COLOR_NONE, FALSE) % victim->getHit() % victim->getMana() % attack_modes[victim->getCombatMode()]);
	    }
          }
        }
        found = TRUE;
        if (victim->desc) {
	  if (hasClass(CLASS_SHAMAN)) {
            if (victim->desc->m_bIsClient || IS_SET(victim->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
	      victim->desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(victim, victim->desc, getName(), NULL, COLOR_NONE, FALSE) % getHit() % getLifeforce() % attack_modes[getCombatMode()]);
	    victim->desc->session.group_share = 1;
	  } else {
            if (victim->desc->m_bIsClient || IS_SET(victim->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
	      victim->desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(victim, victim->desc, getName(), NULL, COLOR_NONE, FALSE) % getHit() % getMana() % attack_modes[getCombatMode()]);
	    victim->desc->session.group_share = 1;
	  }
        }
        continue;
      }
    }
    if (!found)
      sendTo("Sorry, there is no one else here you can group.\n\r");
  } else if (!(victim = get_char_room_vis(this, namebuf)))
    sendTo("No one here by that name.\n\r");
  else {

    if (IS_SET(victim->specials.act, ACT_IMMORTAL) && !victim->isPc()
          && victim->master == this && this != victim )
    {
      sendTo(COLOR_MOBS, fmt("%s is immortal and has no need of you.  %s does not join your group.\n\r") % victim->getName() % victim->getName());
      return;
    }

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
              if (desc->m_bIsClient || IS_SET(desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
                desc->clientf(fmt("%d") % CLIENT_GROUPDELETEALL);
            }
            for (f = followers; f; f = f->next) {
              if (IS_SET(f->follower->specials.affectedBy, AFF_GROUP)) {
                REMOVE_BIT(f->follower->specials.affectedBy, AFF_GROUP);
                if (f->follower->desc) {
                  f->follower->desc->session.group_share = 1;
                  if (f->follower->desc->m_bIsClient || IS_SET(f->follower->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
                    f->follower->desc->clientf(fmt("%d") % CLIENT_GROUPDELETEALL);
                }
              }
            }
          } else {
            sendTo("You ungroup yourself.\n\r");
            REMOVE_BIT(specials.affectedBy, AFF_GROUP);
            if (desc) {
              desc->session.group_share = 1;
              if (desc->m_bIsClient || IS_SET(desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
                desc->clientf(fmt("%d") % CLIENT_GROUPDELETEALL);
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
            if (victim->desc->m_bIsClient || IS_SET(victim->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
              victim->desc->clientf(fmt("%d") % CLIENT_GROUPDELETEALL);
          }
          for (f = followers; f; f = f->next) {
            if (IS_SET(f->follower->specials.affectedBy, AFF_GROUP)) {
              if (f->follower->desc && (f->follower->desc->m_bIsClient || IS_SET(f->follower->desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))
                f->follower->desc->clientf(fmt("%d|%s") % CLIENT_GROUPDELETE % colorString(f->follower, f->follower->desc, victim->getName(), NULL, COLOR_NONE, FALSE));
	    }
          }
          if (desc && (desc->m_bIsClient || IS_SET(desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))
            desc->clientf(fmt("%d|%s") % CLIENT_GROUPDELETE % colorString(this, desc, victim->getName(), NULL, COLOR_NONE, FALSE));
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
	if (hasClass(CLASS_SHAMAN)) {
	  if (desc && (desc->m_bIsClient || IS_SET(desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))
	    desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(desc->character, desc, victim->getName(), NULL, COLOR_NONE, FALSE) % victim->getHit() % victim->getLifeforce() % attack_modes[victim->getCombatMode()]);
	} else {
	  if (desc && (desc->m_bIsClient || IS_SET(desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))
	    desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(desc->character, desc, victim->getName(), NULL, COLOR_NONE, FALSE) % victim->getHit() % victim->getMana() % attack_modes[victim->getCombatMode()]);
	}        
        if (victim != this) {
          for (f = followers; f; f = f->next) {
            TBeing *b = f->follower;
            if (victim->desc && (victim->desc->m_bIsClient || IS_SET(victim->desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))  {
	      if (hasClass(CLASS_SHAMAN)) {
		victim->desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(victim, victim->desc, b->getName(), NULL, COLOR_NONE, FALSE) % b->getHit() % b->getLifeforce() % attack_modes[b->getCombatMode()]);
	      } else {
		victim->desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(victim, victim->desc, b->getName(), NULL, COLOR_NONE, FALSE) % b->getHit() % b->getMana() % attack_modes[b->getCombatMode()]);
	      }
            }
            if (b->desc && (b->desc->m_bIsClient || IS_SET(b->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))) {
	      if (hasClass(CLASS_SHAMAN)) {
		b->desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(b, b->desc, victim->getName(), NULL, COLOR_NONE, FALSE) % victim->getHit() % victim->getLifeforce() % attack_modes[victim->getCombatMode()]);
	      } else {
		b->desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(b, b->desc, victim->getName(), NULL, COLOR_NONE, FALSE) % victim->getHit() % victim->getMana() % attack_modes[victim->getCombatMode()]);
	      }
            }
          }
        }
        if (victim->desc) {
	  if (hasClass(CLASS_SHAMAN)) {
            if (victim->desc && (victim->desc->m_bIsClient || IS_SET(victim->desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))
	      victim->desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(victim, victim->desc, getName(), NULL, COLOR_NONE, FALSE) % getHit() % getLifeforce() % attack_modes[getCombatMode()]);
	    victim->desc->session.group_share = 1;
	  } else {
            if (victim->desc && (victim->desc->m_bIsClient || IS_SET(victim->desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))
	      victim->desc->clientf(fmt("%d|%s|%d|%d|%s") % CLIENT_GROUPADD % colorString(victim, victim->desc, getName(), NULL, COLOR_NONE, FALSE) % getHit() % getMana() % attack_modes[getCombatMode()]);
	    victim->desc->session.group_share = 1;
	  }
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
int TBeing::doQuaff(sstring argument)
{
  sstring buf;
  TThing *t;
  int rc = 0;

  one_argument(argument, buf);

  if (!(t = searchLinkedListVis(this, buf, getStuff()))) {
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
  if (IS_SET_DELETE(rc, DELETE_VICT)) 
    return DELETE_THIS;

  // add some lag.  Prevents multiple quaffs per round
  addToWait(combatRound(1));

  return FALSE;
}


// this function handles any special affect that drinking a liquid has
int doLiqSpell(TBeing *ch, TBeing *vict, liqTypeT liq, int amt)
{
  int rc=0, i;
  int level=max(30, amt*6), learn=max(100, amt*20);
  int duration = (level << 2) * UPDATES_PER_MUDHOUR;
  affectedData aff, aff5[5];
  statTypeT whichStat;

  // if it's an alcoholic liquid, put it in their bloodstream
  if(liquidInfo[liq]->drunk > 0){
    vict->gainCondition(DRUNK, (liquidInfo[liq]->drunk / 10));
    // use leftover as chance to go 1 more unit up/down
    if (::number(0,9) < (abs(liquidInfo[liq]->drunk) % 10))
      vict->gainCondition(DRUNK, (liquidInfo[liq]->drunk > 0 ? 1 : -1));
  }


  switch(liq){
    case LIQ_BLOOD:
      if(vict->isVampire()){
	vict->sendTo("Drinking the dead blood causes you great harm!\n\r");
	poison(ch,vict,level,learn,SPELL_POISON);
	slumber(ch,vict,level,learn);
	rc=harm(ch,vict,level,learn,SPELL_HARM,0);
      }
      break;
    case LIQ_HOLYWATER:
      if(vict->isUndead())
	harm(ch,vict,level,learn,SPELL_HARM,0);
      else if(vict->isDiabolic())
	harmLight(ch,vict,level,learn,SPELL_HARM,0);
      else
	bless(ch,vict,level/10,learn/10,SPELL_BLESS);
      break;
    case LIQ_COFFEE:
      aff.type = AFFECT_DRUG;
      aff.level = level;
      aff.duration = UPDATES_PER_MUDHOUR;
      aff.modifier = 3;
      aff.location = APPLY_SPE;
      vict->affectJoin(NULL, &aff, AVG_DUR_NO, AVG_EFF_YES, FALSE);

      aff.type = AFFECT_DRUG;
      aff.level = level;
      aff.duration = UPDATES_PER_MUDHOUR;
      aff.modifier = 3;
      aff.location = APPLY_FOC;
      vict->affectJoin(NULL, &aff, AVG_DUR_NO, AVG_EFF_YES, FALSE);

      aff.type = AFFECT_DRUG;
      aff.level = level;
      aff.duration = UPDATES_PER_MUDHOUR;
      aff.modifier = -5;
      aff.location = APPLY_CHA;
      vict->affectJoin(NULL, &aff, AVG_DUR_NO, AVG_EFF_YES, FALSE);

      aff.type = AFFECT_DRUG;
      aff.level = level;
      aff.duration = UPDATES_PER_MUDHOUR;
      aff.modifier = -3;
      aff.location = APPLY_DEX;
      vict->affectJoin(NULL, &aff, AVG_DUR_NO, AVG_EFF_YES, FALSE);

      break;
    case LIQ_POT_CURE_POISON:
      curePoison(ch,vict,level,learn,SPELL_CURE_POISON);
      break;
    case LIQ_POT_HEAL_LIGHT:
    case LIQ_POT_HEAL_LIGHT2:
      healLight(ch,vict,level,learn,SPELL_HEAL_LIGHT,0);
      break;
    case LIQ_POT_HEAL_CRIT:
      healCritical(ch,vict,level,learn,SPELL_HEAL_CRITICAL,0);
      break;
    case LIQ_POT_HEAL:
    case LIQ_POT_HEAL2:
      heal(ch,vict,level,learn,SPELL_HEAL,0);
      break;
    case LIQ_POT_SANCTUARY:
    case LIQ_POT_SANCTUARY2:
      sanctuary(ch,vict,level,learn);
      break;
    case LIQ_POT_FLIGHT:
      aff.type = SPELL_FLY;
      aff.level = level;
      aff.duration = 1 * UPDATES_PER_MUDHOUR * level;
      aff.modifier = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = AFF_FLYING;
      
      // correct for weight
      weightCorrectDuration(vict, &aff);
      
      rc = fly(ch,vict,level,&aff,learn);
      if (IS_SET(rc, SPELL_SUCCESS)) {
        act("$n seems lighter on $s feet!", FALSE, vict, NULL, 0, TO_ROOM);
      } else {
	vict->nothingHappens();
      }
      break;
    case LIQ_POT_BIND:
      bind(ch,vict,level,learn);
      break;
    case LIQ_POT_BLINDNESS:
      blindness(ch,vict,level,learn);
      break;
    case LIQ_POT_ARMOR:
      rc = armor(ch,vict,level,learn,SPELL_ARMOR);
      if (IS_SET(rc, SPELL_SUCCESS)) {
        if (ch != vict) {
          act("$N is now defended by $d!",
            FALSE, ch, NULL, vict, TO_CHAR);
          act("$N is now defended by $d!",
            FALSE, ch, NULL, vict, TO_NOTVICT);
          act("You are now defended by $d!",
            FALSE, ch, NULL, vict, TO_VICT);
        } else {
          act("$n is now defended by $d!",
            FALSE, ch, NULL, 0, TO_ROOM);
          act("You are now defended by $d!",
            FALSE, ch, NULL, 0, TO_CHAR);
        }
      } else if (IS_SET(rc, SPELL_CRIT_SUCCESS)) {
        if (ch != vict) {
          act("$N is now strongly defended by $d!",
            FALSE, ch, NULL, vict, TO_CHAR);
          act("$N is now strongly defended by $d!",
            FALSE, ch, NULL, vict, TO_NOTVICT);
          act("You are now strongly defended by $d!",
            FALSE, ch, NULL, vict, TO_VICT);
        } else {
          act("$n is now strongly defended by $d!",
            FALSE, ch, NULL, 0, TO_ROOM);
          act("You are now strongly defended by $d!",
            FALSE, ch, NULL, 0, TO_CHAR);
        }
      } else if (IS_SET(rc, SPELL_FAIL)) {
        vict->sendTo("Your potion fails to bring forth any protection.\n\r");
        act("$n's potion fails to bring forth any protection.", 
          FALSE, ch, NULL, NULL, TO_ROOM);
        ch->deityIgnore(SILENT_YES);
      }
      break;
    case LIQ_POT_REFRESH:
      refresh(ch,vict,level,learn,SPELL_REFRESH);
      break;
    case LIQ_POT_SECOND_WIND:
    case LIQ_POT_SECOND_WIND2:
      secondWind(ch,vict,level,learn);
      break;
    case LIQ_POT_CURSE:
      curse(ch,vict,level,learn,SPELL_CURSE);
      break;
    case LIQ_POT_DETECT_INVIS:
      detectInvisibility(ch,vict,level,learn);
      break;
    case LIQ_POT_BLESS:
    case LIQ_POT_BLESS2:
      bless(ch,vict,level,learn,SPELL_BLESS);
      break;
    case LIQ_POT_INVIS:
      invisibility(ch,vict,level,learn);
      break;
    case LIQ_POT_HEAL_FULL:
      healFull(ch,vict,level,learn,SPELL_HEAL_FULL);
      break;
    case LIQ_POT_SUFFOCATE:
      suffocate(ch,vict,level,learn);
      break;
    case LIQ_POT_FEATHERY_DESCENT:
    case LIQ_POT_FEATHERY_DESCENT2:
      featheryDescent(ch,vict,level,learn);
      break;
    case LIQ_POT_DETECT_MAGIC:
      detectMagic(ch,vict,level,learn);
      break;
    case LIQ_POT_DISPEL_MAGIC:
      dispelMagic(ch,vict,level,learn);
      break;
    case LIQ_POT_STONE_SKIN:
    case LIQ_POT_STONE_SKIN2:
      stoneSkin(ch,vict,level,learn);
      break;
    case LIQ_POT_TRAIL_SEEK:
      trailSeek(ch,vict,level,learn);
      break;
    case LIQ_POT_FAERIE_FIRE:
      faerieFire(ch,vict,level,learn);
      break;
    case LIQ_POT_FLAMING_FLESH:
      flamingFlesh(ch,vict,level,learn);
      break;
    case LIQ_POT_CONJURE_ELE_EARTH:
      conjureElemEarth(vict,level,learn);
      break;
    case LIQ_POT_SENSE_LIFE:
      senseLife(ch,vict,level,learn);
      break;
    case LIQ_POT_STEALTH:
      stealth(ch,vict,level,learn);
      break;
    case LIQ_POT_TRUE_SIGHT:
      trueSight(ch,vict,level,learn);
      break;
    case LIQ_POT_ACCELERATE:
      accelerate(ch,vict,level,learn);
      break;
    case LIQ_POT_INFRAVISION:
    case LIQ_POT_INFRAVISION2:
      infravision(ch,vict,level,learn);
      break;
    case LIQ_POT_SORC_GLOBE:
      sorcerersGlobe(ch,vict,level,learn);
      break;
    case LIQ_POT_POISON:
      poison(ch,vict,level,learn,SPELL_POISON);
      break;
    case LIQ_POT_BONE_BREAKER:
      boneBreaker(ch,vict,level,learn,SPELL_BONE_BREAKER);
      break;
    case LIQ_POT_AQUALUNG:
      aqualung(ch,vict,level,learn);
      break;
    case LIQ_POT_HASTE:
      haste(ch,vict,level,learn);
      break;
    case LIQ_POT_TELEPORT:
    case LIQ_POT_TELEPORT2:
      teleport(ch,vict,level,learn);
      break;
    case LIQ_POT_GILLS_OF_FLESH:
    case LIQ_POT_GILLS_OF_FLESH2:
      gillsOfFlesh(ch,vict,level,learn);
      break;
    case LIQ_POT_CURE_BLINDNESS:
      cureBlindness(ch,vict,level,learn);
      break;
    case LIQ_POT_CURE_DISEASE:
      cureDisease(ch,vict,level,learn,SPELL_CURE_DISEASE);
      break;
    case LIQ_POT_SHIELD_OF_MISTS:
      shieldOfMists(ch,vict,level,learn);
      break;
    case LIQ_POT_SENSE_PRESENCE:
      senseLifeShaman(ch,vict,level,learn);
      break;
    case LIQ_POT_CHEVAL:
      cheval(ch,vict,level,learn);
      break;
    case LIQ_POT_DJALLAS_PROTECTION:
      djallasProtection(ch,vict,level,learn);
      break;
    case LIQ_POT_LEGBAS_GUIDANCE:
      legbasGuidance(ch,vict,level,learn);
      break;
    case LIQ_POT_DETECT_SHADOW:
      detectShadow(ch,vict,level,learn);
      break;
    case LIQ_POT_CELERITE:
    case LIQ_POT_CELERITE2:
    case LIQ_POT_CELERITE3:
      celerite(ch,vict,level,learn);
      break;
    case LIQ_POT_QUICKSILVER:
      celerite(ch,vict,level,learn);
      cheval(ch,vict,level,learn);
      haste(ch,vict,level,learn);

      aff.type = SPELL_HASTE;
      aff.level = level;
      aff.duration = UPDATES_PER_MUDHOUR * 10;
      aff.modifier = 50;
      aff.location = APPLY_SPE;
      vict->affectJoin(NULL, &aff, AVG_DUR_NO, AVG_EFF_YES);

      break;
    case LIQ_POT_CLARITY:
      clarity(ch,vict,level,learn);
      break;
    case LIQ_POT_BOILING_BLOOD:
      bloodBoil(ch,vict,level,learn,SPELL_BLOOD_BOIL);
      break;
    case LIQ_POT_STUPIDITY:
      stupidity(ch,vict,level,learn);
      break;
    case LIQ_POT_SLUMBER:
      slumber(ch,vict,level,learn);
      break;
    case LIQ_POT_HEALING_GRASP:
      healingGrasp(ch,vict,level,learn,SPELL_HEALING_GRASP);
      break;
    case LIQ_POT_CLEANSE:
      cleanse(ch,vict,level,learn,SPELL_CLEANSE);
      break;
    case LIQ_POT_MULTI1: // harm crit, infravision, armor
      harmCritical(ch,vict,level,learn,SPELL_HARM_CRITICAL,0);
      infravision(ch,vict,level,learn);
      armor(ch,vict,level,learn,SPELL_ARMOR);
      break;
    case LIQ_POT_MULTI2: // heal, remove curse, cure poison
      heal(ch,vict,level,learn,SPELL_HEAL,0);
      vict->removeCurseBeing(vict,level,learn,SPELL_REMOVE_CURSE);
      curePoison(ch,vict,level,learn,SPELL_CURE_POISON);
      break;
    case LIQ_POT_MULTI3: // sanc, bless
      sanctuary(ch,vict,level,learn);
      bless(ch,vict,level,learn,SPELL_BLESS);
      break;
    case LIQ_POT_MULTI4: // flight, gills of flesh
      aff.type = SPELL_FLY;
      aff.level = level;
      aff.duration = 1 * UPDATES_PER_MUDHOUR * level;
      aff.modifier = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = AFF_FLYING;
      
      // correct for weight
      weightCorrectDuration(vict, &aff);
      
      rc = fly(ch,vict,level,&aff,learn);
      if (IS_SET(rc, SPELL_SUCCESS)) {
        act("$n seems lighter on $s feet!", FALSE, vict, NULL, 0, TO_ROOM);
      } else {
	vict->nothingHappens();
      }

      gillsOfFlesh(ch,vict,level,learn);
      break;
    case LIQ_POT_MULTI5: // harm, stealth, invis
      harm(ch,vict,level,learn,SPELL_HARM,0);
      stealth(ch,vict,level,learn);
      invisibility(ch,vict,level,learn);
      break;
    case LIQ_POT_MULTI6: // heal, salve, refresh
      heal(ch,vict,level,learn,SPELL_HEAL,0);
      salve(ch,vict,level,learn,SPELL_SALVE);
      refresh(ch,vict,level,learn,SPELL_REFRESH);
      break;
    case LIQ_POT_MULTI7: // sanc, harm crit
      sanctuary(ch,vict,level,learn);
      harmCritical(ch,vict,level,learn,SPELL_HARM_CRITICAL,0);
      break;
    case LIQ_POT_MULTI8: // sanc, harm ser
      sanctuary(ch,vict,level,learn);
      harmSerious(ch,vict,level,learn,SPELL_HARM_SERIOUS,0);
      break;
    case LIQ_POT_MULTI9: // sanc, armor, bless
      sanctuary(ch,vict,level,learn);
      armor(ch,vict,level,learn,SPELL_ARMOR);
      bless(ch,vict,level,learn,SPELL_BLESS);
      break;
    case LIQ_POT_MULTI10: // blind, sanc, stone skin
      blindness(ch,vict,level,learn);
      sanctuary(ch,vict,level,learn);
      stoneSkin(ch,vict,level,learn);
      break;
    case LIQ_POT_MULTI11: // heal, second wind, sterilize
      heal(ch,vict,level,learn,SPELL_HEAL,0);
      secondWind(ch,vict,level,learn);
      sterilize(ch,vict,level,learn,SPELL_STERILIZE);
      break;
    case LIQ_POT_YOUTH:
      if(amt==1)
	vict->sendTo("You feel a little younger.\n\r");
      else
	vict->sendTo("You feel much younger.\n\r");

      while(amt--)
	vict->age_mod -= ::number(1, 2);
      break;
    case LIQ_POT_STAT:
      whichStat = statTypeT(number(0, MAX_STATS_USED - 1));
      vict->addToStat(STAT_CHOSEN, whichStat, amt);
      
      switch (whichStat) {
	case STAT_STR:
	  vict->sendTo("You feel stronger.\n\r");
	  break;
	case STAT_BRA:
	  vict->sendTo("You feel brawnier.\n\r");
	  break;
	case STAT_AGI:
	  vict->sendTo("You feel more agile.\n\r");
	  break;
	case STAT_CON:
	  vict->sendTo("You feel more hardy.\n\r");
	  break;
	case STAT_DEX:
	  vict->sendTo("You feel more dexterous.\n\r");
	  break;
	case STAT_INT:
	  vict->sendTo("You feel smarter.\n\r");
	  break;
	case STAT_WIS:
	  vict->sendTo("You feel more wise.\n\r");
	  break;
	case STAT_FOC:
	  vict->sendTo("You feel more focused.\n\r");
	  break;
	case STAT_KAR:
	  vict->sendTo("You feel luckier.\n\r");
	  break;
	case STAT_CHA:
	  vict->sendTo("You feel more charismatic.\n\r");
	  break;
	case STAT_SPE:
	  vict->sendTo("You feel faster.\n\r");
	  break;
	case STAT_PER:
	  vict->sendTo("You feel more perceptive.\n\r");
	  break;
	case STAT_LUC:
	case STAT_EXT:
	case MAX_STATS:
	  vict->age_mod -= 1;
	  vict->sendTo("You feel younger.\n\r");
	  break;
      }
      break;
    case LIQ_POT_LEARNING:
      for (classIndT Class = MIN_CLASS_IND; Class < MAX_CLASSES; Class++) {
	if (vict->hasClass(1<<Class)){
	  vict->addPracs(amt, Class);
	  break;
	}
      }
      
      vict->sendTo("You feel ready to learn more.\n\r");
      break;
    case LIQ_POT_MYSTERY:
      switch (::number(0,12)) {
	case 1:
	  ch->genericRestore(RESTORE_FULL);
	  ch->sendTo("You have been healed.\n\r");
	  break;
	case 2:
	  aff.type = SPELL_STONE_SKIN;
	  aff.level = 30;
	  aff.duration = 8 * UPDATES_PER_MUDHOUR;
	  aff.location = APPLY_ARMOR;
	  aff.modifier = -75;
	  ch->affectTo(&aff);
	  aff.type = SPELL_STONE_SKIN;
	  aff.level = 30;
	  aff.duration = 8 * UPDATES_PER_MUDHOUR;
	  aff.location = APPLY_IMMUNITY;
	  aff.modifier = IMMUNE_PIERCE;
	  aff.modifier2 = 15;
	  ch->affectTo(&aff);
	  aff.type = AFFECT_SKILL_ATTEMPT;
	  aff.level = 0;
	  aff.duration = 24 * UPDATES_PER_MUDHOUR;
	  aff.location = APPLY_NONE;
	  aff.modifier = SPELL_STONE_SKIN;
	  ch->sendTo("Your skin becomes as hard as stone!\n\r");
	  if (!(ch->isImmortal())) ch->affectTo(&aff);
	  break;
	case 10:
	  ch->age_mod += 1;
	  ch->sendTo("You feel a tiny bit older.\n\r");
	  break;
	case 3:
	  for (classIndT Class = MIN_CLASS_IND; Class < MAX_CLASSES; Class++) {
	    if (ch->hasClass(1<<Class)) {
	      ch->addPracs(1, Class);
	      break;
	    }
	  }
	  ch->sendTo("You feel ready to learn more.\n\r");
	  break;
	case 4:
	  aff.type = SPELL_POISON;
	  aff.level = 10;
	  aff.duration = (20) * UPDATES_PER_MUDHOUR;
	  aff.modifier = -20;
	  aff.location = APPLY_STR;
	  aff.bitvector = AFF_POISON;
	  ch->affectTo(&aff);	  
	  aff.type = AFFECT_DISEASE;
	  aff.level = 0;
	  aff.duration = aff.duration;
	  aff.modifier = DISEASE_POISON;
	  aff.location = APPLY_NONE;
	  aff.bitvector = AFF_POISON;
	  ch->sendTo("You have been poisoned!\n\r");
	  ch->affectTo(&aff);
	  disease_start(ch, &aff);
	  break;
	case 5:
	case 11:
	  ch->age_mod -= 5;
	  ch->sendTo("You feel a bit younger.\n\r");
	  break;
	case 6:
	  ch->age_mod += 3;
	  ch->sendTo("You feel a bit older.\n\r");
	  break;
	case 7:
	case 8:
	case 9:
	  ch->age_mod -= 3;
	  ch->sendTo("You feel a tiny bit younger.\n\r");
	  break;
	default:
	  ch->sendTo("Nothing seems to have happened.\n\r");
	  break;
      }
      break;
    case LIQ_MAGICAL_ELIXIR:
      vict->sendTo("A warm tingle runs through your body.\n\r");
      vict->addToMana(amt);
      break;
    case LIQ_POISON_STANDARD:
    case LIQ_POISON_CAMAS:
    case LIQ_POISON_ANGEL:
    case LIQ_POISON_JIMSON:
    case LIQ_POISON_HEMLOCK:
    case LIQ_POISON_MONKSHOOD:
    case LIQ_POISON_GLOW_FISH:
    case LIQ_POISON_SCORPION:
    case LIQ_POISON_VIOLET_FUNGUS:
    case LIQ_POISON_DEVIL_ICE:
    case LIQ_POISON_FIREDRAKE:
    case LIQ_POISON_INFANT:
    case LIQ_POISON_PEA_SEED:
    case LIQ_POISON_ACACIA:
      if(!vict->isImmune(IMMUNE_POISON, WEAR_BODY)){
	addPoison(aff5, liq, level, duration);
	
	for(i=0;i<5;++i){
	  if(aff5[i].type != TYPE_UNDEFINED){
	    vict->affectTo(&(aff5[i]), -1);
	    
	    if (aff5[i].type == AFFECT_DISEASE)
	      disease_start(vict, &(aff5[i]));
	  }
	}
      }
      break;
    case LIQ_POT_ENLIVEN:
      enliven(ch,vict,level,learn);
      break;
    case LIQ_POT_PLASMA_MIRROR:
      plasmaMirror(ch,level,learn);
      break;
    case LIQ_POT_FILTH:
    case LIQ_GUANO:
      ch->sendTo("Your stomach twists into a knot and you feel like wretching.\r\n");
      if (ch->hasDisease(DISEASE_DYSENTERY))
        break;
      if (::number(1, 100) <= ch->getImmunity(IMMUNE_DISEASE)) {
        ch->sendTo("Something tries to sour your guts, but you fight it off.\r\n");
        break;
      }
      aff.type = AFFECT_DISEASE;
      aff.level = 0;
      aff.duration = 250 + ::number(-50, 50);
      aff.modifier = DISEASE_DYSENTERY;
      aff.location = APPLY_NONE;
      aff.bitvector = 0;
      ch->affectTo(&aff);
      disease_start(ch, &aff);
      break;
    default:
      rc=0;
  }

  return rc;
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
      vlogf(LOG_BUG, fmt("Bad targetting in doObjSpell() %d") %  spell);
      return 0;
    }

    // invoked on a being
    // recite, use staff (non-area), use wand, quaff
    if (victim && 
        !IS_SET(discArray[spell]->targets, TAR_CHAR_ROOM | TAR_CHAR_WORLD | TAR_FIGHT_SELF | TAR_SELF_ONLY)) {
      // tried to invoke on being, not designed for that
      caster->sendTo(COLOR_BASIC, fmt("You can not invoke '%s' on %s.\n\r") %
          discArray[spell]->name % victim->getName());
      return 0;
    }

    // invoked on an obj
    // recite, use wand
    if (target && 
        !IS_SET(discArray[spell]->targets, TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM | TAR_OBJ_WORLD)) {
      // tried to invoke on being, not designed for that
      caster->sendTo(COLOR_BASIC, fmt("You can not invoke '%s' on %s.\n\r") %
          discArray[spell]->name % target->getName());
      return 0;
    }
  }

  // check noharm - otherwise players can use this item to get around noharm checks
  if (victim && SPELL_GUST <= spell && spell <= MAX_SKILL &&
    (spell != SPELL_SLUMBER || !toggleInfo[TOG_SLEEP]->toggle) &&
    discArray[spell]->targets & TAR_VIOLENT && caster->noHarmCheck(victim))
    return 0;

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
      if (target) 
        dispelMagic(caster,target,obj);
      else
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
    case SPELL_CHASE_SPIRIT:
      if (target) 
        chaseSpirits(caster,target,obj);
      else
        rc = chaseSpirits(caster,victim,obj);
      break;
    case SPELL_DJALLA:
      djallasProtection(caster,victim,obj);
      break;
    case SPELL_LEGBA:
      legbasGuidance(caster,victim,obj);
      break;
    case SPELL_PROTECTION_FROM_EARTH:
      protectionFromEarth(caster,victim,obj);
      break;
    case SPELL_PROTECTION_FROM_ELEMENTS:
      protectionFromElements(caster,victim,obj);
      break;
    case SPELL_PROTECTION_FROM_FIRE:
      protectionFromFire(caster,victim,obj);
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
    case SPELL_AQUATIC_BLAST:
      rc = aquaticBlast(caster,victim,obj);
      break;
    case SPELL_CARDIAC_STRESS:
      rc = cardiacStress(caster,victim,obj);
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
    case SPELL_THORNFLESH:
      // intentionally prohibited - too powerful
      break;
    case SPELL_GILLS_OF_FLESH:
      gillsOfFlesh(caster,victim,obj);
      break;
    case SPELL_AQUALUNG:
      aqualung(caster,victim,obj);
      break;
    case SPELL_MYSTIC_DARTS:
      rc = mysticDarts(caster,victim,obj);
      break;
    case SPELL_STICKS_TO_SNAKES:
      sticksToSnakes(caster,victim,obj);
      break;
    case SPELL_STUPIDITY:
      stupidity(caster,victim,obj);
      break;
    case SPELL_DISTORT:
      rc = distort(caster,victim,obj);
      break;
    case SPELL_DEATHWAVE:
      rc = deathWave(caster,victim,obj);
      break;
    case SPELL_FLARE:
      rc = flare(caster,obj);
      break;
    case SPELL_STUNNING_ARROW:
      rc = stunningArrow(caster,victim,obj);
      break;
    case SPELL_SOUL_TWIST:
      rc = soulTwist(caster,victim,obj);
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
    case SPELL_SENSE_LIFE_SHAMAN:
      senseLifeShaman(caster,victim,obj);
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
    case SPELL_DETECT_SHADOW:
      detectShadow(caster,victim,obj);
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
    case SPELL_CHEVAL: // shaman
      cheval(caster,victim,obj);
      break;
    case SPELL_CELERITE: // shaman
      celerite(caster,victim,obj);
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
    case SPELL_HEROES_FEAST:
      heroesFeast(caster);
      break;
    case SPELL_ANTIGRAVITY:
      antigravity(caster);
      break;
    case SPELL_LEVITATE:
      levitate(caster, victim);
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
        vlogf(LOG_BUG, "doObjSpell:poison had bogus targets");
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
    case SPELL_CLEANSE:
      //      cleanse(caster,victim,obj, spell);
      cleanse(caster,victim,obj);
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
    case SPELL_SHIELD_OF_MISTS:
      shieldOfMists(caster,victim,obj);
      break;
    case SPELL_CONTROL_UNDEAD:
      controlUndead(caster,victim,obj);
      break;
    case SPELL_SHADOW_WALK:
      shadowWalk(caster,victim,obj);
      break;
    case SPELL_ENLIVEN:
      enliven(caster,victim,obj);
      break;
    case SPELL_HEALING_GRASP:
      healingGrasp(caster,victim,obj, spell);
      break;
    case SPELL_RESURRECTION:
      resurrection(caster,obj);
      break;
    case SPELL_DANCING_BONES:
      dancingBones(caster,obj);
      break;
    case SPELL_VOODOO:
      voodoo(caster,obj);
      break;
    case SKILL_BARKSKIN:
      barkskin(caster,victim,obj);
      break;
    case SPELL_RAZE:
      rc = raze(caster,victim,obj);
      break;
    case SPELL_LICH_TOUCH:
      rc = lichTouch(caster,victim,obj);
      break;
    case SPELL_VAMPIRIC_TOUCH:
      rc = vampiricTouch(caster,victim,obj);
      break;
    case SPELL_HYPNOSIS:
      hypnosis(caster,victim,obj);
      break;
    case SPELL_BLOOD_BOIL:
      rc = bloodBoil(caster, victim, obj);
      break;
    case SPELL_CLARITY:
      clarity(caster,victim,obj);
      break;
    case SPELL_LIVING_VINES:
      livingVines(caster,victim,obj);
      break;
    case SPELL_STORMY_SKIES:
      stormySkies(caster,victim,obj);
      break;
    case SPELL_ATOMIZE:
      rc = atomize(caster,victim,obj);
      break;
    case SPELL_ENERGY_DRAIN:
      rc = energyDrain(caster,victim,obj);
      break;
    case SPELL_BLAST_OF_FURY:
      rc = blastOfFury(caster, victim, obj);
      break;
    case SPELL_ROOT_CONTROL:
      rc = rootControl(caster, victim, obj);
      break;
    case SKILL_BEAST_SOOTHER:
      rc = beastSoother(caster, victim, obj);
      break;
    case SPELL_DIVINATION:
      if (!target) {
        divinationBeing(caster, victim, obj);
      } else 
        divinationObj(caster, target, obj);
      break;
    default:
      vlogf(LOG_BUG,fmt("Object (%s) with uncoded spell (%d)!") %  obj->getName() % spell);
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

  argument = one_argument(argument, buf, cElements(buf));

  if (!(t = searchLinkedListVis(this, buf, getStuff()))) {
    t = heldInPrimHand();
    if (!t || !isname(buf, t->name)) {
      if (isAffected(AFF_BLIND)) {
        sendTo("How do you expect to read something when you are blind???\n\r");
      } else {
        act("You do not have that item.", FALSE, this, 0, 0, TO_CHAR);
      }
      return FALSE;
    }
  }

  if (nomagic("Sorry, you can't recite that here.")) {
    return FALSE;
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

  return FALSE;
}

int TScroll::reciteMe(TBeing *ch, const char * argument)
{
  TObj *obj = NULL;
  TBeing *victim = NULL;
  int i, bits, rc;

  if (ch->isAffected(AFF_SILENT)) {
    ch->sendTo("You can't recite a scroll, you are silenced!\n\r");
    return FALSE;
  }

  bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &victim, &obj);

  if (!bits) {
    if (!ch->fight() || !ch->sameRoom(*ch->fight())) {
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


  int skill=ch->getSkillValue(SKILL_READ_MAGIC);
  if(ch->hasClass(CLASS_MAGE))
    skill=max(skill+50, 100);

  if (!ch->bSuccess(SKILL_READ_MAGIC)) {
    ch->sendTo("You flub the words and the spell does not fire.\n\r");
    return DELETE_THIS;
  }
  
  lag_t max_lag=LAG_0;

  for (i = 0; i < 3; i++)  {
    spellNumT the_spell = getSpell(i);
    if (the_spell >= MIN_SPELL) {
      if (!discArray[the_spell]) {
        vlogf(LOG_BUG,fmt("doRecite called spell (%d) that doesnt exist! - Don't do that!") %  the_spell);
        continue;
      }
      if ((discArray[the_spell]->targets & TAR_VIOLENT) &&
          ch->checkPeaceful("Impolite magic is banned here.\n\r"))
        continue;

      if(max_lag<discArray[the_spell]->lag)
	max_lag=discArray[the_spell]->lag;
      

      setLocked(true);
      rc=doObjSpell(ch,victim,this,obj,argument,the_spell);
      setLocked(false);
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

  ch->addToWait(combatRound(max_lag+2));

  return DELETE_THIS;
}

// returns DELETE_THIS if this should be toasted.
int TBeing::doUse(sstring argument)
{
  sstring buf;
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
  rc = t->useMe(this, argument.c_str());
  setQuaffUse(FALSE);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete t;
    t = NULL;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_THIS;
  }

  // add some lag.  Prevents multiple uses per round
  addToWait(combatRound(1));

  return FALSE;
}

void TBeing::doLight(const sstring & argument)
{
  TThing *t = NULL;
  char tmpname[256], *tmp;
  bool roomOnly, heldOnly;
  int num;
  sstring arg1, arg2;

  arg1=argument.word(0);
  arg2=argument.word(1);

  strcpy(tmpname, arg1.c_str());
  tmp = tmpname;
  
  // WARNING: the room and equipped lists do not stack
  // meaning that if there are 2 lanterns in the room and 2 equipped, light 4.lantern will not find any lantern.
  // thus the room and held arguments are needed, and supplying a numbered 1st arg without a 2nd arg can produce odd results.
  // one could write a routine to count objects in the linked list, but perhaps one would then want to reconcile seen vs unseen objects for non-lights.
  roomOnly = is_abbrev(arg2, "room");
  heldOnly = is_abbrev(arg2, "held");
  
  if (arg1.empty()) {
    sendTo("Light what?\n\r");
    return;
  }
  
  if (!(num = get_number(&tmp)))
    return;

  // no visibility checks
  // look in room first
  if (!heldOnly && !(t = searchLinkedList(arg1, roomp->getStuff()))) {
    if (roomOnly) {
      sendTo("You cannot find any such object in your surroundings.\n\r");
      return;
    }
  }
  
  if (!t) {
    // check eq
    int stack_num = 1; // keep track of which object they are trying to light
    for (int i = MIN_WEAR; i < MAX_WEAR; i++) {
      if ((t = equipment[i]) && isname(tmp, t->name)) {
        // keep looking unless name and number argument match
        if (num != stack_num) {
          ++stack_num;
          continue;
        } else {
          // don't allow the burning of the clothes on your back -
          // so, if the item is equipped, it can only be lit if it is a light
          if (dynamic_cast<TLight *>(t) ||
	      dynamic_cast<TDrugContainer *>(t)){
            t->lightMe(this, SILENT_NO);
          } else {
            act("You cannot light $p while equipped.", FALSE, this, t, 0, TO_CHAR);
          }
          return;
        }
      }
    }
    sendTo("You cannot find any such object.\n\r");
    return;
  } else {
    t->lightMe(this, SILENT_NO);
  }
  
}

void TObj::setBurning(TBeing *ch){
  if(dynamic_cast<TPCorpse *>(this))
    return;

  if(!isObjStat(ITEM_BURNING)){
    addObjStat(ITEM_BURNING);

    if(isObjStat(ITEM_CHARRED))
      remObjStat(ITEM_CHARRED);

  }
}

void TObj::remBurning(TBeing *ch){
  if(isObjStat(ITEM_BURNING)){
    remObjStat(ITEM_BURNING);
    if(material_nums[getMaterial()].flammability &&
       !isObjStat(ITEM_CHARRED))
      addObjStat(ITEM_CHARRED);

  }
}

void TThing::extinguishMe(TBeing *ch)
{
  TObj *o;

  if (!(o=dynamic_cast<TObj *>(this)) || !o->isObjStat(ITEM_BURNING)) {
    ch->sendTo("You begin to extinguish yourself.\n\r");
    act("You stop, drop, and roll on the ground.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops, drops, and rolls on the ground.", FALSE, ch, 0, 0, TO_ROOM);
    start_task(ch, 0, 0, TASK_EXTINGUISH_MY_ASS, "", 2, ch->inRoom(), 0, 0, 5);
  } else {
    o->remBurning(ch);
    act("You extinguish $p, and it smolders slightly before going out.",  
	FALSE, ch, o, 0, TO_CHAR);
    act("$n extinguishes $p, and it smolders slightly before going out.",
	FALSE, ch, o, 0, TO_ROOM);
  }
  return;
}

void TBeing::doExtinguish(const sstring & argument)
{
  TThing *t = NULL;
  char tmpname[256], *tmp;
  int num;
  sstring arg1, arg2;

  arg1=argument.word(0);
  arg2=argument.word(1);

  strcpy(tmpname, arg1.c_str());
  tmp = tmpname;

  bool roomOnly = is_abbrev(arg2, "room");
  bool heldOnly = is_abbrev(arg2, "held");

  if (POSITION_STANDING >= getPosition()) {
    if (riding)
      dismount(POSITION_STANDING);
    setPosition(POSITION_STANDING);
  }
  if (arg1.empty()) {
    sendTo("Extinguish what?\n\r");
    return;
  }
  
  if (!(num = get_number(&tmp)))
    return;

  // no visibility checks
  // look in room first
  if (!heldOnly && !(t = searchLinkedList(arg1, roomp->getStuff()))) {
    if (roomOnly) {
      sendTo("You cannot find any such object in your surroundings.\n\r");
      return;
    }
  }
  
  if (!t) {
    // check eq
    int stack_num = 1; // keep track of which object they are trying to extinguish
    for (int i = MIN_WEAR; i < MAX_WEAR; i++) {
      if ((t = equipment[i]) && isname(tmp, t->name)) {
        // keep looking unless name and number argument match
        if (num != stack_num) {
          ++stack_num;
          continue;
        } else {
          t->extinguishMe(this);
          return;
        }
      }
    }
    sendTo("You cannot find any such object.\n\r");
    return;
  } else {
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

void TBeing::doRefuel(const sstring &argument)
{
  TThing *fuel;
  TThing *t;
  sstring arg, arg2, arg3;
  int roomOnly, heldOnly;

  arg=argument.word(0);
  arg2=argument.word(1);
  arg3=argument.word(2);

  if (arg.empty()) {
    sendTo("Refuel what?\n\r");
    sendTo("Syntax: refuel <light> <fuel> {\"room\" | \"held\"}\n\r");
    return;
  }
  if (arg2.empty()) {
    sendTo("Refuel with what?\n\r");
    sendTo("Syntax: refuel <light> <fuel> {\"room\" | \"held\"}\n\r");
    return;
  }
  if (!(fuel = get_thing_char_using(this, arg2.c_str(), 0, FALSE, FALSE))) {
    sendTo("You don't have that fuel in your inventory!\n\r");
    sendTo("Syntax: refuel <light> <fuel> {\"room\" | \"held\"}\n\r");
    return;
  }
  roomOnly = is_abbrev(arg3, "room");
  heldOnly = is_abbrev(arg3, "held");

  if (roomOnly || 
      !(t = get_thing_char_using(this, arg.c_str(), 0, FALSE, FALSE))) {
    if (heldOnly ||
        !(t = searchLinkedListVis(this, arg, roomp->getStuff()))) {
      sendTo("You can only refuel an object in the room, or held.\n\r");
      sendTo("Syntax: refuel <light> <fuel> {\"room\" | \"held\"}\n\r");
      return;
    }
  }
  // Do some checks to see if it can be refueled. 
  t->refuelMeLight(this, fuel);
}

void TBeing::doStop(const sstring &tStArg)
{
  if (tStArg.empty()) {
    sendTo("Stop what?  You aren't doing anything!\n\r");
    return;
  }

  if (!followers) {
    sendTo("Nobody is following you, so how can you stop them?\n\r");
    return;
  }

  TBeing * tBeing = get_best_char_room(this, tStArg.c_str());

  if (!tBeing) {
    sendTo(fmt("Whom?  You look around for '%s', but fail to find them...\n\r") %
           tStArg);
    return;
  }

  if (isAffected(AFF_CHARM) ||
      tBeing->isAffected(AFF_CHARM) ||
      !tBeing->isPc())
    return;

  if (tBeing->isImmortal() &&
      (!isImmortal()) || GetMaxLevel() < tBeing->GetMaxLevel()) {
    sendTo("You just don't have the heart, or the guts, to tell them to stop.\n\r");
    return;
  }

  if (tBeing->master != this) {
    sendTo("You are not their leader, so don't tell them what to do!\n\r");
    return;
  }

  act("$n orders you to stop...You are forced to comply.",
      TRUE, this, NULL, tBeing, TO_VICT);

  // This just goes through the normal setup to verify that future
  // changes in the follow code are easily filtered into this.
  tBeing->doFollow("self");
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
  argument = one_argument(argument, arg, cElements(arg));

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
  } else if ((spellType == SPELL_DANCER)) {
    if (!reconcileLifeforce(spelltask->spell, TRUE)) { 
      // will need to change to lifeforce
      sendTo("You can not continue a invokation without enough lifeforce.\n\r");
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
    value = convertTo<int>(arg);    
    if (value < 1) {
      sendTo("You can not change your prayer in this manner.\n\r");
      return;
    }
    if ((spellType == SPELL_PRAYER) && (getPiety() < (value * usePiety(spelltask->spell)))) {
      sendTo("You do not have the piety to continue your prayer that many times.\n\r");
      return;
    } else if ((spellType == SPELL_CASTER) && (getMana() < (value * useMana(spelltask->spell)))) {
      sendTo("You do not have the mana to continue your spell that many times.\n\r");
      return;
    } else if ((spellType == SPELL_DANCER) && (getLifeforce() < (value * useLifeforce(spelltask->spell)))) {
      sendTo("You do not have the lifeforce to invoke that many times.\n\r");
      return;
    }
    if (IS_SET(discArray[(spelltask->spell)]->comp_types, SPELL_TASKED_EVERY)) {
      spelltask->rounds += value;
      if (!IS_SET(spelltask->flags, CASTFLAG_CAST_INDEFINITE)) {
        sendTo(COLOR_SPELLS, fmt("<o>You continue your prayer for %d additional round%s (%d total round%s).<1>\n\r") %
         value % (value != 1 ? "s" : "") %
         spelltask->rounds % (spelltask->rounds != 1 ? "s" : ""));
      } else {
         sendTo(COLOR_SPELLS, fmt("<o>You add %d round%s to your indefinite prayer (%d total round%s).<1>\n\r") % 
         value % (value != 1 ? "s" : "") %
         spelltask->rounds % (spelltask->rounds != 1 ? "s" : ""));

      }
      return;
    }
    return;
  } else if (IS_SET(discArray[(spelltask->spell)]->comp_types, SPELL_TASKED_EVERY)) {
    if (!IS_SET(spelltask->flags, CASTFLAG_CAST_INDEFINITE)) {
      sendTo(COLOR_SPELLS,"<B>You decide to continue your praying indefinitely.<1>\n\r");
      SET_BIT(spelltask->flags, CASTFLAG_CAST_INDEFINITE);
    } else {
      sendTo(COLOR_SPELLS,fmt("<b>You are praying indefinitely with %d rounds to go.<1>\n\r") % max(spelltask->rounds, 1));
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

  if (d->m_bIsClient) {
    sendTo("The client keeps its own history, please use that!\n\r");
    return;
  }
  sendTo("Your command history :\n\r\n\r");
  for (i = 0; i < 10; i++)
    sendTo(fmt("[%d] %s\n\r") % i % d->history[i]);

  TDatabase db(DB_SNEEZY);

  sendTo("\n\rYour tell history :\n\r\n\r");

  db.query("select tellfrom, tell from tellhistory where tellto='%s' order by telltime desc", getName());

  for(i=0;i<25 && db.fetchRow();i++){
    sendTo(COLOR_BASIC, fmt("[%d] <p>%s<1> told you, \"<c>%s<1>\"\n\r") %
	   i % db["tellfrom"] % db["tell"]);
  }
  

}


void TBeing::doDrag(TBeing *v, dirTypeT tdir)
{
  int rc;
  TBeing *heap_ptr[50];
  int i, heap_top, heap_tot[50], result;
  followData *k, *n;
  char buf[256];
  int oldr, oldroom;
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
    act("$n strains with all $s might to drag $N out of the room but fails.",
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
  oldr = v->roomp->dir_option[tdir]->to_room;
  rp = real_roomp(oldr);
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
  int oldroom, oldr, rc;
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
    act("$n strains with all $s might to drag $N out of the room but fails.",
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
  oldr = o->roomp->dir_option[tdir]->to_room;
  rp = real_roomp(oldr);
  --(*o);
  --(*this);
  *rp += *this;
  *rp += *o;
  act("$n enters the room dragging $N!", TRUE, this, NULL, o, TO_ROOM);
  TPCorpse *tmpcorpse = dynamic_cast<TPCorpse *>(o);
  if (tmpcorpse) {
    tmpcorpse->setRoomNum(oldr);
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

void TBeing::doDrag(const sstring &arg)
{
  sstring caName, dir;
  TBeing *v;
  dirTypeT tdir;
  unsigned int bits;
  roomDirData *exitp;
  TObj *o;
  const char *syntax="Syntax : drag <object|person> <direction>\n\r";

  caName=arg.word(0);
  dir=arg.word(1);


  if(caName.empty() || dir.empty()){
    sendTo(syntax);
    return;
  }

  bits = generic_find(caName.c_str(), FIND_CHAR_ROOM | FIND_OBJ_ROOM, this, &v, &o);

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
  } 

  exitp = exitDir(tdir);

  if (!exit_ok(exitp, NULL) || IS_SET(exitp->condition, EX_CLOSED)) {
    sendTo(fmt("You are blocked from dragging %s.\n\r") % dirs[tdir]);
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


void TBeing::doEmail(const char *arg)
{
  char buf[256];

  one_argument(arg, buf, cElements(buf));

  if (!desc || !desc->account)
    return;

  if (!*buf) {
    sendTo(fmt("Your present email address is: %s\n\r") % desc->account->email);
    return;
  }
  if (illegalEmail(buf, desc, SILENT_NO)) {
    return;
  }
  sendTo(fmt("Changing email address from %s to %s.\n\r") %
           desc->account->email % buf);
  desc->account->email=buf;
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

  sprintf(cmd_buf, "account/%c/%s/comment", LOWER(who[0]), sstring(who).lower().c_str());

  if (!(fp = fopen(cmd_buf, "a+"))) {
    perror("doComment");
    character->sendTo("Could not open the comment-file.\n\r");
    return;
  }

  fputs(buf, fp);
  fputs("\n", fp);
  fclose(fp);



  i=0;
  while(buf[i++]!='\n');

  char *notebuf=(char *)malloc(strlen(&buf[i]));
  strcpy(notebuf, &buf[i]);
  TNote *mynote=createNote(notebuf);
  if (!mynote) {
    character->sendTo("Could not create a note in add_comment, please tell a god.\n\r");
    return;
  }
  sprintf(buf, "****** Comment on %s", sstring(who).lower().c_str());
  
  *character += *mynote;

  character->doAt((fmt("8 post note %s") % buf).c_str(), false);

  //  mynote->postMe(character, buf, FindBoardInRoom(8, "board"));

}

void Descriptor::send_bug(const char *type, const char *msg) 
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
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
       character->getName(),
       character->inRoom(), 
       (gamePort == PROD_GAMEPORT ? "" : (gamePort == 7901 ? "(gamma) " : "(beta) ")), 
       (!m_bIsClient ? "" : "(client) "), 
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
    vlogf(LOG_BUG, fmt("Bogus type (%s) in send_bug.") %  type);
    return;
  }
  fputs(buf, fp);
  fputs("\n", fp);
  fclose(fp);

//  ----------

  if (!(fp = fopen(BUG_TEMP_FILE, "w"))) {
    vlogf(LOG_FILE, "Error opening dummy file for bug mailing.");
    return;
  }
  fputs(buf2, fp);
  fclose(fp);

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

bool TBeing::isWary() const
{
  return affectedBySpell(AFFECT_WARY);
}

void TBeing::makeWary()
{
  if (isPc())
    return;

  affectedData aff;

  aff.type = AFFECT_WARY;
  aff.duration = 2 * UPDATES_PER_MUDHOUR;
  affectTo(&aff);

}


void TBeing::doRoll(TBeing *v, dirTypeT tdir)
{
  int rc;
  TBeing *heap_ptr[50];
  int i, heap_top, heap_tot[50], result;
  followData *k, *n;
  char buf[256];
  int oldr, oldroom;
  TRoom *rp;

  
  if (v == this) {
    sendTo("You can't roll yourself!\n\r");
    sendTo("Syntax : roll <person> <direction>\n\r");
    return;
  }
  if (v->getPosition() > POSITION_SLEEPING) {
    sendTo("You can't roll someone who is conscious.\n\r");
    return;
  }
  if (v->riding) {
    act("You can't roll them off the $o.",TRUE,this,v->riding,0,TO_CHAR);
    return;
  }
  if (v->rider) {
    sendTo("Sorry, you can't roll creatures carrying other creatures.\n\r");
    return;
  }


  // v-weight > 5 * free carry weight
  if (compareWeights(v->getTotalWeight(TRUE),
          (5.0 * (carryWeightLimit() - getCarriedWeight()))) == -1) {
    act("You strain with all your might to roll $N out of the room but fail.",
        TRUE, this, NULL, v, TO_CHAR);
    act("$n strains with all $s might to roll $N out of the room but fails.",
        TRUE, this, NULL, v, TO_ROOM);
    return;
  }

  sprintf(buf, "%i", tdir);
  rc = roomp->checkSpec(this, CMD_ROOM_ATTEMPTED_EXIT, buf, roomp);
  if(rc==TRUE) // not allowed to move
    return;

  // We can drag now. Do necessary checks and make it so. - Brutius 
  sprintf(buf, "You roll $N %s.", dirs[tdir]);
  act(buf, TRUE, this, NULL, v, TO_CHAR);
  sprintf(buf, "$n rolls $N %s.", dirs[tdir]);
  act(buf, TRUE, this, NULL, v, TO_ROOM);
  oldroom = v->in_room;
  oldr = v->roomp->dir_option[tdir]->to_room;
  rp = real_roomp(oldr);
  --(*v);
  --(*this);
  *rp += *this;
  *rp += *v;
  act("$n enters the room rolling $N!", TRUE, this, NULL, v, TO_ROOM);

  if (v->followers) {
    heap_top = 0;
    for (k = v->followers; k; k = n) {
      n = k->next;
      if ((oldroom == k->follower->in_room) && !k->follower->fight() &&
          (k->follower->getPosition() >= POSITION_CRAWLING)) {
        sprintf(buf, "You follow $N as $E is rolled out of the room.");
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
        sprintf(buf, "You follow $N as $E rolls $p out of the room.");
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
  addToMove(-10);
  addToWait(combatRound(1));
}

void TBeing::doRoll(TObj *o, dirTypeT tdir)
{
  TBeing *heap_ptr[50];
  int i, heap_top, heap_tot[50], result;
  followData *k, *n;
  char buf[256];
  int oldroom, oldr, rc;
  TRoom *rp;

  if(!o) return;
  if (!isImmortal()) {
    if(!o->canWear(ITEM_TAKE)) {
      act("$N : You can't roll that.\n\r", TRUE, this, NULL, o, TO_CHAR);
      return;
    }
    if (dynamic_cast<TTrap *>(o)) {
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
    act("You strain with all your might to roll $N out of the room but fail.",
	TRUE, this, NULL, o, TO_CHAR);
    act("$n strains with all $s might to roll $N out of the room but fails.",
        TRUE, this, NULL, o, TO_ROOM);
    return;
  }

  sprintf(buf, "%i", tdir);
  rc = roomp->checkSpec(this, CMD_ROOM_ATTEMPTED_EXIT, buf, roomp);
  if(rc==TRUE) // not allowed to move
    return;


  sprintf(buf, "You roll $N %s.", dirs[tdir]);
  act(buf, TRUE, this, NULL, o, TO_CHAR);
  sprintf(buf, "$n rolls $N %s.", dirs[tdir]);
  act(buf, TRUE, this, NULL, o, TO_ROOM);
  oldroom = o->in_room;
  oldr = o->roomp->dir_option[tdir]->to_room;
  rp = real_roomp(oldr);
  --(*o);
  --(*this);
  *rp += *this;
  *rp += *o;
  act("$n enters the room rolling $N!", TRUE, this, NULL, o, TO_ROOM);
  TPCorpse *tmpcorpse = dynamic_cast<TPCorpse *>(o);
  if (tmpcorpse) {
    tmpcorpse->setRoomNum(oldr);
    tmpcorpse->saveCorpseToFile();
  }
  if (followers) {
    heap_top = 0;
    for (k = followers; k; k = n) {
      n = k->next;
      if ((oldroom == k->follower->in_room) && !k->follower->fight() &&
          (k->follower->getPosition() >= POSITION_CRAWLING)) {
        sprintf(buf, "You follow $N as $E rolls $p out of the room.");
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
  addToMove(-10);
  addToWait(combatRound(1));
}

void TBeing::doRoll(const sstring &arg)
{
  sstring caName, dir;
  TBeing *v;
  dirTypeT tdir;
  unsigned int bits;
  roomDirData *exitp;
  TObj *o;
  const char *syntax="Syntax : roll <object|person> <direction>\n\r";

  caName=arg.word(0);
  dir=arg.word(1);

  if(caName.empty() || dir.empty()){
    sendTo(syntax);
    return;
  }

  bits = generic_find(caName.c_str(), FIND_CHAR_ROOM | FIND_OBJ_ROOM, this, &v, &o);

  if(!bits){
    sendTo("You see nothing by that name to roll!\n\r");
    sendTo(syntax);
    return;
  } else {
    if (riding) {
      sendTo("You have to dismount first.\n\r");
      return;
    }
    if (!hasHands() || bothArmsHurt()) {
      sendTo("You need working arms and hands to roll things.\n\r");
      return;
    }
    if (getMove() < 20) {
      sendTo("You don't have the necessary movement to roll anything!\n\r");
      return;
    }
    tdir = getDirFromChar(dir);
    if (tdir == DIR_NONE) {
      sendTo("No such direction!\n\r");
      sendTo(syntax);
      return;
    }
    if (equipment[HOLD_LEFT] || equipment[HOLD_RIGHT]) {
      sendTo("Roll something with your hands full!?!\n\r");
      return;
    }
  } 

  exitp = exitDir(tdir);

  if (!exit_ok(exitp, NULL) || IS_SET(exitp->condition, EX_CLOSED)) {
    sendTo(fmt("You are blocked from rolling %s.\n\r") % dirs[tdir]);
    sendTo(syntax);
    return;
  }
  if (!validMove(tdir)) 
    return;
  
  if(bits==FIND_CHAR_ROOM){
    doRoll(v, tdir);
  } else if(bits==FIND_OBJ_ROOM){
    doRoll(o, tdir);
  } else {
    // not reached
    sendTo("You see nothing by that name to roll!\n\r");
    sendTo(syntax);
    return;
  }

  return;
}

// adds to stats randomly, but in a weighted function that favours the first
// stats to which points are added
// ** also does some subtractions -- the idea here is to mix things up 
// a bit **
void TBeing::addToRandomStat(int extra_points) {
  statTypeT whichStat;
  int amt;
  unsigned i=0;
  bool firstPass=TRUE;
  vector<statTypeT>stats;
  for (whichStat=MIN_STAT;whichStat<MAX_STATS_USED;whichStat++){
    stats.push_back(whichStat);
  }
  std::random_shuffle(stats.begin(), stats.end());
  while(extra_points != 0) {
    if (i >= stats.size()) {
      i = 0;
      firstPass=FALSE;
    }
    whichStat = stats[i++];
    if (firstPass) {
      amt = ::number(-10,extra_points)/2;
    } else amt = max(1,::number(0,extra_points)/2);
    addToStat(STAT_CHOSEN, whichStat, amt);
    extra_points -= amt;
  }
}

