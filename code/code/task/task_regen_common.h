#pragma once

#include "enum.h"
#include "parse.h"

class TBeing;

enum class RegenTaskType : char {
  SLEEP = POSITION_SLEEPING,
  SIT = POSITION_SITTING,
  REST = POSITION_RESTING,
};

int task_regen(TBeing&, RegenTaskType, int, cmdTypeT, const char*);

void sendRegenStartupMessage(TBeing&);

// Starts a regen task with the canonical start_task parameters and emits the
// startup bonus message. Use this instead of calling start_task directly for
// TASK_SLEEP, TASK_REST, or TASK_SIT.
void startRegenTask(TBeing&, RegenTaskType);
