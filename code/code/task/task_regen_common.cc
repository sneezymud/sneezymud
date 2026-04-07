#include "task_regen_common.h"

#include <algorithm>
#include <array>
#include <functional>
#include <unordered_map>

#include "ansi.h"
#include "being.h"
#include "connect.h"
#include "defs.h"
#include "enum.h"
#include "parse.h"
#include "room.h"
#include "spells.h"
#include "sstring.h"
#include "task.h"

namespace {

  struct BonusConfig {
      std::function<bool(const TBeing&)> test;
      int amount{1};
      const char* message;
      const char* startupMessage;
  };

  const std::array bonusChecks = std::to_array<BonusConfig>({
    {
      .test = [](const TBeing& ch) { return ch.inCamp(); },
      .message = "Being in a camp makes you feel better.\n\r",
      .startupMessage = "<o>being in a camp<1>",
    },
    {
      .test = [](const TBeing& ch) { return ch.hasGroupmateInRoom(); },
      .message = "Having a companion to guard your back puts you at ease.\n\r",
      .startupMessage = "<p>having a companion to guard your back<1>",
    },
    {
      .test =
        [](const TBeing& ch) { return ch.roomp && ch.roomp->hasCampfire(); },
      .message = "The crackling fire warms your body and spirits.\n\r",
      .startupMessage = "<r>the warmth of a campfire<1>",
    },
    {
      .test = [](const TBeing& ch) { return ch.isAffected(AFF_HIDE); },
      .message = "Being hidden from view helps you relax and recover.\n\r",
      .startupMessage = "<k>being hidden from view<1>",
    },
    {
      .test = [](const TBeing& ch) { return ch.homeTurf(); },
      .message = "The familiar surroundings put you at ease.\n\r",
      .startupMessage = "<g>familiar surroundings<1>",
    },
    {
      .test = [](const TBeing& ch) { return ch.backgroundBonus(); },
      .message = "Your experience helps you find a comfortable spot.\n\r",
      .startupMessage = "<G>your experience in this terrain<1>",
    },
  });

  // TODO(Cirius): Convert these to global tweaks
  constexpr int MANA_GAIN_PER_TICK = 1;
  constexpr double PIETY_GAIN_PER_TICK = 0.10;

  constexpr int MESSAGE_CHANCE = 20;

  int processRegen(TBeing& ch, int pulse, int regenMod, bool silent) {
    ch.task->calcNextUpdate(pulse, regenMod * ch.regenTime());

    if (ch.task->status) {
      ch.updatePos();
      ch.task->status = 0;
      return 1;
    }

    if (!ch.roomp || ch.roomp->isRoomFlag(ROOM_NO_HEAL)) {
      return 1;
    }

    int regenAmt = 1;

    for (const auto& bonus : bonusChecks) {
      if (bonus.test(ch)) {
        regenAmt += bonus.amount;

        if (!silent && percentChance(MESSAGE_CHANCE)) {
          ch.sendTo(bonus.message);
        }
      }
    }

    ch.addToHit(regenAmt);

    if (ch.getMove() < ch.moveLimit()) {
      ch.addToMove(std::min(regenAmt, ch.moveLimit() - ch.getMove()));
    }

    ch.addToMana(MANA_GAIN_PER_TICK);
    ch.addToPiety(PIETY_GAIN_PER_TICK);
    ch.updatePos();

    if (ch.desc) {
      constexpr auto flags = CHANGED_HP | CHANGED_MANA | CHANGED_LIFEFORCE |
                             CHANGED_PIETY | CHANGED_MOVE;
      if (ch.ansi()) {
        ch.desc->updateScreenAnsi(flags);
      } else if (ch.vt100()) {
        ch.desc->updateScreenVt100(flags);
      }
    }

    return 1;
  }

  struct RegenTaskConfig {
      taskTypeT taskType;
      std::function<bool(TBeing&, cmdTypeT, const char*)> cmdHandler;
      std::function<void(TBeing&)> applyFightingPenalty;
      int tickRateMultiplier{1};
      bool silent{false};
      const char* description;
  };

  const std::unordered_map<RegenTaskType, RegenTaskConfig> configs{
    {
      RegenTaskType::REST,
      {
        .taskType = TASK_REST,
        .cmdHandler = [](TBeing& ch, cmdTypeT cmd, const char* arg) -> bool {
          switch (cmd) {
            case CMD_ABORT:
            case CMD_STOP:
            case CMD_STAND:
              ch.stopTask();
              ch.doStand();
              break;
            case CMD_SLEEP:
              ch.stopTask();
              ch.doSleep(arg);
              break;
            case CMD_SIT:
              ch.stopTask();
              ch.doSit(arg);
              break;
            case CMD_REST:
              ch.sendTo("You are already nice and comfy.\n\r");
              break;
            default:
              return false;
          }
          return true;
        },
        .applyFightingPenalty =
          [](TBeing& ch) {
            ch.cantHit += ch.loseRound(1);
            if (!::number(0, 2)) {
              ch.cantHit += ch.loseRound(1);
            }
          },
        .tickRateMultiplier = 2,
        .description = "resting",
      },
    },
    {
      RegenTaskType::SLEEP,
      {
        .taskType = TASK_SLEEP,
        .cmdHandler = [](TBeing& ch, cmdTypeT cmd, const char* arg) -> bool {
          switch (cmd) {
            case CMD_ABORT:
            case CMD_STOP:
            case CMD_STAND:
            case CMD_WAKE:
              ch.stopTask();
              ch.doWake(arg);
              break;
            case CMD_SLEEP:
              ch.sendTo("You start to dream about sleeping.\n\r");
              break;
            default:
              return false;
          }
          return true;
        },
        .applyFightingPenalty =
          [](TBeing& ch) {
            ch.cantHit += ch.loseRound(1);
            if (!::number(0, 1)) {
              ch.cantHit += ch.loseRound(1);
            }
          },
        .silent = true,
        .description = "sleeping",
      },
    },
    {
      RegenTaskType::SIT,
      {
        .taskType = TASK_SIT,
        .cmdHandler = [](TBeing& ch, cmdTypeT cmd, const char* arg) -> bool {
          switch (cmd) {
            case CMD_ABORT:
            case CMD_STOP:
            case CMD_STAND:
              ch.stopTask();
              ch.doStand();
              break;
            case CMD_REST:
              ch.stopTask();
              ch.doRest(arg);
              break;
            case CMD_SLEEP:
              ch.stopTask();
              ch.doSleep(arg);
              break;
            case CMD_SIT:
              ch.sendTo(
                "You look around and notice your butt is already on "
                "something.\n\r");
              break;
            default:
              return false;
          }
          return true;
        },
        .applyFightingPenalty =
          [](TBeing& ch) {
            if (!::number(0, 2)) {
              ch.cantHit += ch.loseRound(1);
            }
          },
        .tickRateMultiplier = 4,
        .description = "sitting",
      },
    },
  };

}  // namespace

void sendRegenStartupMessage(TBeing& ch) {
  sstring msg = "Your recovery will be <c>enhanced<1> by...\n\r";
  bool hasBonuses = false;

  for (const auto& bonus : bonusChecks) {
    if (bonus.test(ch)) {
      msg += "   ...";
      msg += bonus.startupMessage;
      msg += "...\n\r";
      hasBonuses = true;
    }
  }

  if (hasBonuses) {
    ch.sendTo(msg + "\n\r");
  }
}

void startRegenTask(TBeing& ch, RegenTaskType type) {
  // Tasks are a player-state mechanism. NPCs receive their position-aware
  // recovery through updateHalfTickStuff (see periodic.cc), which already
  // accounts for inCamp/bedRegen/etc. Attaching a task to a mob would
  // conflict with mob AI's position management and leak taskData on mob
  // deletion paths that don't call stopTask.
  if (!ch.isPc()) {
    return;
  }

  // Historical timeLeft value used by every regen-task start_task call site.
  // Effectively a sentinel - the actual update cadence is driven by the
  // nextUpdate parameter computed from regenTime() below.
  constexpr int REGEN_TASK_TIME_LEFT = 350;

  const auto& config = configs.at(type);

  sendRegenStartupMessage(ch);
  start_task(&ch, nullptr, nullptr, config.taskType, "", REGEN_TASK_TIME_LEFT,
    0, 1, 0, config.tickRateMultiplier * ch.regenTime());
}

int task_regen(TBeing& ch, RegenTaskType type, int pulse, cmdTypeT cmd,
  const char* arg) {
  if (!ch.task) {
    return 0;
  }

  if (ch.isLinkdead() || ch.getPosition() != static_cast<positionTypeT>(type)) {
    ch.stopTask();
    return 0;
  }

  [[maybe_unused]] const auto& [taskType, cmdHandler, applyFightingPenalty,
    tickRateMultiplier, silent, description] = configs.at(type);

  const int regenMod =
    std::max(tickRateMultiplier / (ch.hasClass(CLASS_THIEF) ? 2 : 1), 1);

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      return processRegen(ch, pulse, regenMod, silent);
    case CMD_TASK_FIGHTING:
      ch.sendTo(format("You are unable to fight while %s!\n\r") % description);
      applyFightingPenalty(ch);
      ch.stopTask();
      return 1;
    default:
      // false = let caller process; true = eat command
      return cmdHandler(ch, cmd, arg) || cmd >= MAX_CMD_LIST;
  }
}
