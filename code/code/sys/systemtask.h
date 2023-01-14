//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

enum systemTaskT {
  SYSTEM_TRACEROUTE,
  SYSTEM_LOGLIST,
  SYSTEM_CHECKLOG,
  SYSTEM_FIND_EMAIL,
};

class TBeing;
class _task;
class sstring;

class SystemTask {
    bool taskstatus;
    _task* top;
    _task* bot;

  public:
    SystemTask() : taskstatus(true), top(NULL), bot(NULL) {}
    void AddTask(TBeing*, char, const char*);
    void CheckTask();
    sstring Tasks(TBeing*, const char*);
    int forktask(_task*);
    void remove(_task*);
    void start_task();
};

extern SystemTask* systask;
