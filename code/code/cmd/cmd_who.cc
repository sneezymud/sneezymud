//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "cmd_who.cc" - the who command
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <set>

#include "extern.h"
#include "room.h"
#include "being.h"
#include "colorstring.h"
#include "statistics.h"
#include "database.h"
#include "cmd_message.h"
#include "account.h"
#include "person.h"
#include "monster.h"

sstring TBeing::parseTitle(Descriptor*) { return getName(); }

sstring TPerson::parseTitle(Descriptor* user) {
  sstring buf;
  int flag = FALSE;

  if (!title) {
    return getName();
  }

  buf = nameColorString(this, user, title, &flag, COLOR_BASIC, FALSE);
  if (!flag && colorString(this, user, title, NULL, COLOR_NONE, TRUE)
                   .find(getNameNOC(this)) == sstring::npos)
    buf = getName();  // did not specify a <n>

  // explicitely terminate it since players are sloppy
  buf += "<1>";

  return buf;
}

void Descriptor::menuWho() {
  TBeing* person;
  sstring buf, buf2, send;

  send = "\n\r";

  for (person = character_list; person; person = person->next) {
    if (person->isPc() && person->polyed == POLY_TYPE_NONE) {
      if (dynamic_cast<TPerson*>(person) &&
          (person->getInvisLevel() < GOD_LEVEL1)) {
        buf = person->parseTitle(this);
        buf2 = format("%s\n\r") %
               colorString(person, this, buf, NULL, COLOR_BASIC, FALSE);
        send += buf2;
      }
    }
  }
  send += "\n\r";
  writeToQ(send);
  writeToQ("[Press return to continue]\n\r");
}

namespace {

  enum class WhoMode {
    Default,
    Brief,
    Admin,
    Name
  };

  sstring getZoneName(const TBeing* p) {
    if (!p->roomp || !p->roomp->getZone())
      return "Unknown";

    sstring name = p->roomp->getZone()->name;
    // zone names often follow "Author - Zone Name" format; strip the author
    auto sep = name.find(" - ");
    if (sep != sstring::npos)
      return name.substr(sep + 3);
    return name;
  }

  sstring getClassDisplay(const TBeing* p) {
    return TBeing::getProfAbbrevName(p->player.Class);
  }

  sstring getImmColor(const TBeing* viewer, const TBeing* p) {
    if (p->hasWizPower(POWER_WIZARD))
      return viewer->purple();
    if (p->hasWizPower(POWER_GOD))
      return viewer->red();
    if (p->hasWizPower(POWER_BUILDER))
      return viewer->cyan();
    return "";
  }

  sstring getImmRank(const TBeing* p) {
    if (p->hasWizPower(POWER_WIZARD))
      return "Creator";
    if (p->hasWizPower(POWER_GOD))
      return p->getSex() == SEX_FEMALE ? "Goddess" : "God";
    if (p->hasWizPower(POWER_BUILDER))
      return "Demigod";
    return "Immortal";
  }

  sstring getRaceDisplay(const TBeing* p) {
    auto* race = Races[p->getRace()];
    if (race)
      return race->getProperName();
    return "Unknown";
  }

  sstring getResourceDisplay(const TBeing* p) {
    if (p->hasClass(CLASS_CLERIC) || p->hasClass(CLASS_DEIKHAN))
      return format("HP:%d Pty:%.0f Mv:%d") % p->getHit() % p->getPiety() %
             p->getMove();
    if (p->hasClass(CLASS_SHAMAN))
      return format("HP:%d LF:%d Mv:%d") % p->getHit() % p->getLifeforce() %
             p->getMove();
    return format("HP:%d Mana:%d Mv:%d") % p->getHit() % p->getMana() %
           p->getMove();
  }

  void appendTags(sstring& buf, const TBeing* viewer, const TBeing* p) {
    if (p->isPlayerAction(PLR_SEEKSGROUP))
      buf += "  (Seeking Group)";
    if (p->isPlayerAction(PLR_NEWBIEHELP))
      buf += "  (Newbie-Helper)";
    if (p->desc && p->desc->account &&
        (time(nullptr) - p->desc->account->birth) < NEWBIE_PURGATORY_LENGTH)
      buf += "  (Newbie)";
    if (!p->msgVariables(MSG_NOTE).empty())
      buf += format("  (%s)") % p->msgVariables(MSG_NOTE);
    if (viewer->isImmortal()) {
      if (p->polyed == POLY_TYPE_SWITCH)
        buf += "  (switched)";
      if (p->getInvisLevel() > MAX_MORT)
        buf += format("  (invis %d)") % p->getInvisLevel();
    }
  }

  sstring formatPlayerLine(const TBeing* viewer, const TBeing* p) {
    sstring line;
    if (p->GetMaxLevel() > MAX_MORT) {
      sstring immTitle = stripColorCodes(p->msgVariables(MSG_IMM_TITLE));
      line = format("%-14s %sL%-2d %-7s  %-7s  %s%s") %
             sstring(viewer->pers(p)).cap() % getImmColor(viewer, p) %
             p->GetMaxLevel() % getImmRank(p) % getRaceDisplay(p) % immTitle %
             viewer->norm();
    } else {
      line = format("%-14s L%-2d %-7s  %-7s  %s") %
             sstring(viewer->pers(p)).cap() % p->GetMaxLevel() %
             getClassDisplay(p) % getRaceDisplay(p) % getZoneName(p);
    }
    appendTags(line, viewer, p);
    line += "\n\r";
    return line;
  }

}  // namespace

void TBeing::doWho(const char* argument) {
  sstring args(argument);

  auto start = args.find_first_not_of(" \t");
  if (start != sstring::npos)
    args = args.substr(start);
  else
    args = "";

  auto mode = WhoMode::Default;
  sstring nameArg;

  if (args.empty()) {
    mode = WhoMode::Default;
  } else if (args == "--brief" || args == "-b") {
    mode = WhoMode::Brief;
  } else if (args == "--admin") {
    if (!isImmortal()) {
      sendTo("You don't have access to that option.\n\r");
      return;
    }
    mode = WhoMode::Admin;
  } else if (args == "--help" || args == "-?") {
    sstring help = "Usage: who [option | name]\n\r";
    help +=
      "  who              Show all players with level, class, race, and "
      "area\n\r";
    help += "  who --brief      Show names and titles only\n\r";
    if (isImmortal())
      help +=
        "  who --admin      Show detailed admin info (idle, HP, account)\n\r";
    help += "  who <name>       Look up a specific player\n\r";
    if (desc)
      desc->page_string(help, SHOWNOW_NO, ALLOWREP_YES);
    return;
  } else if (args[0] == '-') {
    sendTo("Unknown option. Try 'who --help'.\n\r");
    return;
  } else {
    mode = WhoMode::Name;
    nameArg = args;
  }

  sstring sb;
  unsigned int count = 0;
  int lcount = 0;
  std::set<std::string> uniqueAccounts;

  // name lookup mode — detailed single-player view
  if (mode == WhoMode::Name) {
    sb = "";
    int found = 0;
    for (auto* p = character_list; p; p = p->next) {
      // uses canSeeWho (not canSee) for consistency with list modes —
      // room-level visibility (lighting, hide) shouldn't affect a global lookup
      if (!p->isPc() || !isname(nameArg, p->name) || !canSeeWho(p))
        continue;
      if (!dynamic_cast<TPerson*>(p))
        continue;

      if (found > 0) sb += "--\n\r";
      found++;

      // line 1: title
      sb += format("%s\n\r") % p->parseTitle(desc);

      // line 2: level/class/race/zone (without tags — shown separately below)
      if (p->GetMaxLevel() > MAX_MORT) {
        sstring immTitle = stripColorCodes(p->msgVariables(MSG_IMM_TITLE));
        sb += format("%sL%-2d %-7s  %-7s  %s%s\n\r") % getImmColor(this, p) %
              p->GetMaxLevel() % getImmRank(p) % getRaceDisplay(p) % immTitle %
              norm();
      } else {
        sb += format("L%-2d %-7s  %-7s  %s\n\r") % p->GetMaxLevel() %
              getClassDisplay(p) % getRaceDisplay(p) % getZoneName(p);
      }

      // line 3: group status
      if (p->isPlayerAction(PLR_SEEKSGROUP)) {
        sb += "(Seeking Group)\n\r";
      } else if (p->isAffected(AFF_GROUP)) {
        // find the group leader
        auto* leader = p;
        while (leader->master && leader->master->isAffected(AFF_GROUP))
          leader = leader->master;

        sstring groupLine = "Grouped with: ";
        bool first = true;

        // add leader if it's not the target
        if (leader != p && canSeeWho(leader)) {
          groupLine += sstring(pers(leader)).cap();
          first = false;
        }

        // add other group members
        for (auto* f = leader->followers; f; f = f->next) {
          auto* member = f->follower;
          if (member == p || !member->isPc() || !member->isAffected(AFF_GROUP))
            continue;
          if (!canSeeWho(member))
            continue;
          if (!first)
            groupLine += ", ";
          groupLine += sstring(pers(member)).cap();
          first = false;
        }

        if (!first)
          sb += groupLine + "\n\r";
      }

      // line 4: online time, idle
      if (p->desc) {
        auto online = time(nullptr) - p->desc->session.connect;
        auto hours = online / 3600;
        auto mins = (online % 3600) / 60;
        sstring timeLine = "Online: ";
        if (hours > 0)
          timeLine += format("%dh %dm") % hours % mins;
        else
          timeLine += format("%dm") % mins;

        auto idle = p->getTimer();
        if (idle > 0)
          timeLine += format("  Idle: %dm") % idle;

        sb += timeLine + "\n\r";
      }

      // extra tags on their own line
      sstring tags;
      if (p->isPlayerAction(PLR_NEWBIEHELP))
        tags += "(Newbie-Helper) ";
      if (p->desc && p->desc->account &&
          (time(nullptr) - p->desc->account->birth) < NEWBIE_PURGATORY_LENGTH)
        tags += "(Newbie) ";
      if (!p->msgVariables(MSG_NOTE).empty())
        tags += format("(%s) ") % p->msgVariables(MSG_NOTE);
      if (isImmortal()) {
        if (p->isLinkdead())
          tags += "(link-dead) ";
        if (p->polyed == POLY_TYPE_SWITCH)
          tags += "(switched) ";
        if (p->getInvisLevel() > MAX_MORT)
          tags += format("(invis %d) ") % p->getInvisLevel();
      }
      if (!tags.empty())
        sb += tags + "\n\r";
    }
    if (!found)
      sb += "No one logged in with that name.\n\r";
    if (desc)
      desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
    return;
  }

  // list modes: iterate all players once
  sb = "Players:\n\r--------\n\r";

  for (auto* p = character_list; p; p = p->next) {
    if (!p->isPc())
      continue;
    if (!canSeeWho(p))
      continue;

    if (auto* person = dynamic_cast<TPerson*>(p); !person) {
      // isPc() returns true for mobs with ACT_POLYSELF (polymorph spell) or
      // ACT_DISGUISED (thief disguise). These are TMonster instances, not
      // TPerson, so dynamic_cast fails. polyed == POLY_TYPE_NONE excludes
      // immortals using the 'switch' command (POLY_TYPE_SWITCH), who appear
      // as their switched-into mob and are handled in the TPerson branch.
      if (isImmortal() && p->polyed == POLY_TYPE_NONE) {
        if (IS_SET(p->specials.act, ACT_POLYSELF)) {
          count++;
          if (p->desc && p->desc->account)
            uniqueAccounts.insert(p->desc->account->name);
          sb += format("%s (polymorphed)\n\r") % sstring(pers(p)).cap();
        } else if (IS_SET(p->specials.act, ACT_DISGUISED)) {
          count++;
          if (p->desc && p->desc->account)
            uniqueAccounts.insert(p->desc->account->name);
          sb += format("%s (disguised)\n\r") % sstring(pers(p)).cap();
        }
      }
      continue;
    }

    if (p->desc && p->desc->account)
      uniqueAccounts.insert(p->desc->account->name);
    count++;
    if (p->isLinkdead())
      lcount++;

    // skip link-dead from display unless admin mode
    if (p->isLinkdead() && mode != WhoMode::Admin)
      continue;

    switch (mode) {
      case WhoMode::Brief:
        sb += format("%s\n\r") % p->parseTitle(desc);
        break;

      case WhoMode::Admin: {
        sstring line;
        if (p->isLinkdead())
          line = format("[Linkdead] %-12s") % sstring(pers(p)).cap();
        else
          line = format("%-23s") % sstring(pers(p)).cap();

        line += format(" L%-2d %-5s  %-7s  Idle:%-3d  %s") % p->GetMaxLevel() %
                getClassDisplay(p) % getRaceDisplay(p) % p->getTimer() %
                getResourceDisplay(p);

        if (hasWizPower(POWER_WIZARD)) {
          if (p->desc && p->desc->account)
            line += format("  [%s]") % p->desc->account->name;
          else
            line += "  [Unknown]";
        }

        if (p->polyed == POLY_TYPE_SWITCH)
          line += "  (switched)";
        if (p->getInvisLevel() > MAX_MORT)
          line += format("  (invis %d)") % p->getInvisLevel();
        line += "\n\r";
        sb += line;
        break;
      }

      case WhoMode::Default:
      default:
        sb += formatPlayerLine(this, p);
        break;
    }
  }

  AccountStats::max_player_since_reboot =
    max(AccountStats::max_player_since_reboot, count);

  auto avgPlayers = stats.useage_iters
                      ? static_cast<float>(stats.num_users) / stats.useage_iters
                      : 0.0f;

  sb += "\n\r";
  if (mode == WhoMode::Admin) {
    sb += format(
            "Total: %d  Linkdead: %d  Max since reboot: %d  Avg: %.1f  "
            "Accounts: %d\n\r") %
          count % lcount % AccountStats::max_player_since_reboot % avgPlayers %
          uniqueAccounts.size();
  } else if (mode == WhoMode::Brief) {
    sb += format("Total: %d\n\r") % count;
  } else {
    sb +=
      format("Total: %d  Max since reboot: %d  Avg: %.1f  Accounts: %d\n\r") %
      count % AccountStats::max_player_since_reboot % avgPlayers %
      uniqueAccounts.size();
  }

  if (desc)
    desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
}

void TBeing::doWhozone() {
  Descriptor* d;
  TRoom* rp = NULL;
  sstring sbuf, buf;
  TBeing* person = NULL;
  int count = 0;

  sendTo("Players:\n\r--------\n\r");
  for (d = descriptor_list; d; d = d->next) {
    if (!d->connected && canSee(d->character) &&
        (rp = real_roomp(
           (person = (d->original ? d->original : d->character))->in_room)) &&

        (rp->getZoneNum() == roomp->getZoneNum())) {
      sbuf = format("%-25s - %s ") % person->getName() % rp->name;
      if (GetMaxLevel() > MAX_MORT) {
        buf = format("[%d]") % person->in_room;
        sbuf += buf;
      }
      sbuf += "\n\r";
      sendTo(COLOR_BASIC, sbuf);
      count++;
    }
  }
  sendTo(format("\n\rTotal visible players: %d\n\r") % count);
}
