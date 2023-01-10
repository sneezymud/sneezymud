#pragma once

#include <cstdio>
#include <functional>
#include <vector>

#include "body.h"
#include "enum.h"
#include "spells.h"

class TBeing;
class sstring;

// Namespace containing functions and classes to allow for efficient, modular,
// and 'DRY' handling of skill/spell failure conditions
namespace SkillHandler {
  class Config;

  // Alias representing a callback function that executes a predefined `act()`
  // call defined within, using the config data passed in at that moment
  using Act = std::function<void(const Config&)>;

  // Alias representing a callback function that executes and returns the result
  // of a predefined expression defined within which resolves to a boolean,
  // using the config data passed in at that moment
  using Test = std::function<bool(const Config&)>;

  // Simply a type to differentiate between a basic `Test` callback by itself,
  // and a basic `Test` callback that's been combined with an `Act` callback
  // using `make_test()` to create a new callback function that can be passed to
  // `run_tests()`
  using TestWithAct = Test;
  using Tests = std::vector<TestWithAct>;

  // A class representing the dynamic information needed by the callback
  // functions generated via the higher order functions defined below
  class Config {
    public:
      spellNumT skill{MAX_SKILL};
      const TBeing* ch{nullptr};
      const TBeing* target{nullptr};
      silentTypeT silent{SILENT_NO};

      Config(spellNumT skill, const TBeing* ch, const TBeing* target,
        silentTypeT silent) :
        skill(skill),
        ch(ch),
        target(target),
        silent(silent){};
  };

  // Higher-order function overload to allow the OR-ing of two or more
  // other tests, for representing more complex potential failure situations
  TestWithAct operator||(const TestWithAct& test_1, const TestWithAct& test_2);

  // Higher-order function overload to allow the AND-ing of two or more
  // other tests, for representing more complex potential failure situations
  TestWithAct operator&&(const TestWithAct& test_1, const TestWithAct& test_2);

  // Higher-order function which combines one `Test` and one `Act` callback
  // into a new TestWithAct callback, which can be passed to `run_tests()`
  TestWithAct make_test(
    const Test& test, const Act& do_act = [](const Config&) {});

  // Higher-order function to allow easy creation of an `Act` callback using
  // default values aside from the message itself. For use when the message
  // simply needs to be sent to `ch` without any extra considerations.
  Act make_simple_act(const sstring& message);

  // Function that expects references to a `Config` object and a vector of
  // `TestWithAct`s. Passes `config` to and executes each callback in `tests`.
  // Returns `true` if all tests return true, or `false` if *any* test returns
  // false. If a test returns false, it will execute its predefined `act()`
  // callback beforehand, outputting the correct message for its failure
  // conditions.
  bool run_tests(const Config& config, const Tests& tests);

  // Higher-order test callback creation functions. Each of these functions must
  // return a callback function with a function signature `(const Config&) ->
  // bool`
  TestWithAct ch_can_harm_target();
  TestWithAct ch_has_valid_body_type(const std::vector<body_t>& valid_types,
    const sstring& message);
  TestWithAct ch_is_flying(const sstring& message);
  TestWithAct ch_knows_skill(const sstring& message);
  TestWithAct ch_not_busy();
  TestWithAct ch_not_flying(const sstring& message);
  TestWithAct ch_not_mounted(const sstring& message);
  TestWithAct not_peaceful_room(
    const sstring& message =
      "You feel too peaceful to contemplate violence.\n\r");
  TestWithAct target_not_flying(const sstring& message);
  TestWithAct target_not_immortal(const sstring& message);
  TestWithAct target_not_mounted(const sstring& message);
  TestWithAct target_not_on_furniture(const sstring& message);
  TestWithAct target_not_self(const sstring& message);

  // Allows for dynamic creation of test callbacks in situations where a one-off
  // test is needed that wouldn't make sense to add here
  std::function<TestWithAct()> custom(const Test& test, const Act& act);
}  // namespace SkillHandler
