#include "skill_handler.h"

#include <algorithm>

#include "being.h"
#include "comm.h"
#include "defs.h"
#include "race.h"
#include "sstring.h"
#include "structs.h"
#include "thing.h"

using namespace SkillHandler;

TestWithAct SkillHandler::operator||(const TestWithAct& test_1,
  const TestWithAct& test_2) {
  return [test_1, test_2](
           const Config& config) { return test_1(config) || test_2(config); };
}

TestWithAct SkillHandler::operator&&(const TestWithAct& test_1,
  const TestWithAct& test_2) {
  return [test_1, test_2](
           const Config& config) { return test_1(config) && test_2(config); };
}

bool SkillHandler::run_tests(const Config& config, const Tests& tests) {
  return std::none_of(tests.begin(), tests.end(),
    [config](const TestWithAct& test) { return !test(config); });
}

TestWithAct SkillHandler::make_test(const Test& test, const Act& do_act) {
  return [test, do_act](const Config& config) {
    const bool result = test(config);
    if (!result && config.silent == SILENT_NO)
      do_act(config);
    return result;
  };
}

Act SkillHandler::make_simple_act(const sstring& message) {
  return [message](const Config& config) {
    act(message, false, config.ch, nullptr, nullptr, TO_CHAR);
  };
}

TestWithAct SkillHandler::ch_can_harm_target() {
  return make_test([](const Config& config) {
    return !config.ch->noHarmCheck(config.target);
  });
}

TestWithAct SkillHandler::ch_has_valid_body_type(
  const std::vector<body_t>& valid_types, const sstring& message) {
  return make_test(
    [valid_types](const Config& config) {
      const body_t body_type = config.ch->getMyRace()->getBodyType();
      return std::any_of(valid_types.begin(), valid_types.end(),
        [body_type](body_t t) { return t == body_type; });
    },
    make_simple_act(message));
}

TestWithAct SkillHandler::ch_is_flying(const sstring& message) {
  return make_test([](const Config& config) { return config.ch->isFlying(); },
    make_simple_act(message));
}

TestWithAct SkillHandler::ch_knows_skill(const sstring& message) {
  return make_test(
    [](const Config& config) { return config.ch->doesKnowSkill(config.skill); },
    make_simple_act(message));
};

TestWithAct SkillHandler::ch_not_busy() {
  return make_test(
    [](const Config& config) { return !config.ch->checkBusy(); });
}

TestWithAct SkillHandler::ch_not_flying(const sstring& message) {
  return make_test([](const Config& config) { return !config.ch->isFlying(); },
    make_simple_act(message));
}

TestWithAct SkillHandler::ch_not_mounted(const sstring& message) {
  return make_test([](const Config& config) { return !config.ch->riding; },
    make_simple_act(message));
}

// Message should end in '\n\r'
TestWithAct SkillHandler::not_peaceful_room(const sstring& message) {
  return make_test([message](const Config& config) {
    return !config.ch->checkPeaceful(
      message.empty() ? "You feel too peaceful to contemplate violence.\n\r"
                      : message);
  });
}

TestWithAct SkillHandler::target_not_flying(const sstring& message) {
  return make_test(
    [](const Config& config) { return !config.target->isFlying(); },
    make_simple_act(message));
}

TestWithAct SkillHandler::target_not_immortal(const sstring& message) {
  return make_test(
    [](const Config& config) {
      return !(config.target->isImmortal() ||
               IS_SET(config.target->specials.act, ACT_IMMORTAL));
    },
    make_simple_act(message));
}

TestWithAct SkillHandler::target_not_mounted(const sstring& message) {
  return make_test(
    [](const Config& config) {
      const TThing* riding = config.target->riding;
      return !riding || !dynamic_cast<const TBeing*>(riding);
    },
    [message](const Config& config) {
      act(message, false, config.ch, config.target->riding, config.target,
        TO_CHAR);
    });
}

TestWithAct SkillHandler::target_not_on_furniture(const sstring& message) {
  return make_test(
    [](const Config& config) {
      const TThing* riding = config.target->riding;
      return !riding || dynamic_cast<const TBeing*>(riding);
    },
    [message](const Config& config) {
      return act(message, false, config.ch, config.target->riding,
        config.target, TO_CHAR);
    });
}

TestWithAct SkillHandler::target_not_self(const sstring& message) {
  return make_test(
    [](const Config& config) { return config.ch != config.target; },
    make_simple_act(message));
}

std::function<TestWithAct()> SkillHandler::custom(const Test& test,
  const Act& act) {
  return [test, act]() { return make_test(test, act); };
}
