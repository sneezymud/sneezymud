//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __SYSTEMTASK_H
#define __SYSTEMTASK_H

const int SYSTEM_MAIL_IMMORT_DIR = 1;
const int SYSTEM_TRACEROUTE = 2;
const int SYSTEM_LOGLIST = 3;
const int SYSTEM_CHECKLOG = 4;
const int SYSTEM_FIND_EMAIL = 5;
const int SYSTEM_STATISTICS = 6;
const int SYSTEM_SEARCH_HELP = 7;

class	_task;

class SystemTask {
  bool taskstatus;
  _task	*top;
  _task	*bot;
  public:
    SystemTask() :
      taskstatus(true),
      top(NULL),
      bot(NULL)
    {
    }
    void AddTask(TBeing *, char, const char *);
    void CheckTask();
    sstring Tasks(TBeing *, const char *);
    int	forktask(_task *);
    void remove(_task *);
    void start_task();
};

extern SystemTask *systask;

#endif
