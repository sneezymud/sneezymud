#include <cassert>

#include "being.h"
#include "obj.h"
#include "parse.h"
#include "room.h"
#include "task.h"
#include "task_regen_common.h"

int task_rest(TBeing* ch, cmdTypeT cmd, const char* arg, int pulse, TRoom*,
  TObj*) {
  assert(ch);
  return task_regen(*ch, RegenTaskType::REST, pulse, cmd, arg);
}
