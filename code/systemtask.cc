//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// systemtask.cc : tasks shelled out to the operating system
//
//////////////////////////////////////////////////////////////////////////


extern "C" {
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include "stdsneezy.h"
#include "systemtask.h"
#include "obj_note.h"
#if !defined(LINUX)
extern pid_t vfork(void);
#endif

const char TMPFILE[] = "/mud/prod/lib/tmp/task.output";

class _task {
  public: 
    TBeing *owner;
    char tsk;
    char *cmd;
    int	pid;
    _task *next;
    _task *prev;

    _task(TBeing *m, char t)	{ owner=m; tsk=t; cmd=0;  pid = 0; next = 0; prev = 0; }
    ~_task()			{ delete [] cmd; }
};

//
//  SystemTask::AddTask(TBeing *, char, char *)
//
void SystemTask::AddTask(TBeing *own, char tsk, const char *opt)
{
  char	lbuf[128], opt1[128];
  _task	*tmp;

  //  Check if the task system is enabled.
  if (!taskstatus) {
    own->sendTo("Tasks have been disabled.\n\r");
    return;
  }
  //  See if someone is trying to send shell commands to the script.
  if (opt) {
    if (strchr(opt, '`') || strchr(opt, '|') || strchr(opt, '>') || strchr(opt, ';')) {
      vlogf(LOG_BUG, "Invalid char found in task option.  Tasks disabled.");
      taskstatus = false;
      return;
    }
  }
  //  Allocate space for the new task.
  if (!(tmp = new _task(own, tsk))) {
    vlogf(LOG_BUG, "ERROR: SystemTask::AddTask(): malloc of struct _task failed");
    return;
  }
  //  Insert the new task into the linked list.
  if (bot) {
    bot->next = tmp;
    tmp->prev = bot;
    bot = tmp;
  } else 
    top = bot = tmp;
  
  //  Create the command that is send to the shell.
  switch(tmp->tsk) {
    case SYSTEM_MAIL_IMMORT_DIR:
#if 0
      if (opt && top->owner->GetMaxLevel()) 
        *opt = toupper(*opt);
      else
        sprintf(lbuf, "bin/mid %s", top->owner->getName());
#endif

      break;
    case SYSTEM_TRACEROUTE:
      sscanf(opt, "%s", opt1);
      sprintf(lbuf, "bin/traceroute %s", opt1);
      break;
    case SYSTEM_LOGLIST:
      sprintf(lbuf, "bin/loglist");
      break;
    case SYSTEM_CHECKLOG:
      sprintf(lbuf, "bin/checklog %s", opt);
      break;
    case SYSTEM_FIND_EMAIL:
      sprintf(lbuf, "bin/findemail %s", opt);
      break;
    case SYSTEM_STATISTICS:
      sprintf(lbuf, "bin/statistics");
      break;
    case SYSTEM_SEARCH_HELP:
      sprintf(lbuf, "bin/helpsearch %s", opt);
      break;
    default:
      vlogf(LOG_BUG, "SystemTask::AddTask(): Unknown task!");
      remove(top);
      top = top->next;
      delete tmp;
      return;
  }
  sprintf(strchr(lbuf, '\000'), " >%s 2>&1", TMPFILE);
  tmp->cmd = mud_str_dup(lbuf);

  if (top && top->owner)
    top->owner->sendTo("Your task has been added to the queue.\n\r");
}

//
// SystemTask::CheckTask()
//
void SystemTask::CheckTask() 
{
  char file[32];
  int pstatus;
  struct stat fstatus;

  if (!top) 
    return;

  //  Check if the top task is running and start it if it isn't.
  if (!top->pid) {
    start_task();
  //  Check on the running task.
  } else {
    if (waitpid(top->pid, &pstatus, WNOHANG) < 0) {
      vlogf(LOG_SILENT, fmt("INFO: task '%s' completed.") %  top->cmd);
      //  Process the output.
      memset((char *) &fstatus, 0, sizeof(struct stat));
      if (stat(TMPFILE, &fstatus) < 0) 
#if defined(LINUX)
        vlogf(LOG_BUG, "WARNING: SystemTask::CheckTask(): stat()");
#else
        vlogf(LOG_BUG, fmt("WARNING: SystemTask::CheckTask(): stat(): errno=%d") %  errno);
#endif

      // there's no real technical reason we couldn't shove all the
      // info onto a note.  My fear is someone would checklog "e" log*
      // creating like 20M of data.  This would force mud to allocate
      // 20 meg which would degrade performance though...
      const int maxnotesize = 4096;
      if (fstatus.st_size <= maxnotesize && fstatus.st_size > 0) {
        sstring str;
        file_to_sstring(TMPFILE, str);
        // Create a note and put the output in it.
        TNote * note = createNote(mud_str_dup(str));
        // Inform the requester and give them the note.
	if (note) {
          *(top->owner) += *note;
          top->owner->sendTo("Your task has completed.  A note with your output is in your inventory.\n\r");
        } else {
	  top->owner->sendTo("There was a problem making a note, please tell a god.\n\r");
        }
      } else if (fstatus.st_size > maxnotesize) {
        sprintf(file, "tmp/%s.output", top->owner->getName());
        rename(TMPFILE, file);
        top->owner->sendTo("Your task has completed but is to large to be loaded into a note.  Use\n\rviewoutput to read it.\n\r");
      } else
        top->owner->sendTo("Your task has completed.  You have no output.\n\r");
      
      remove(top);
    }
  }
}

//
//  SystemTask::Tasks(TBeing *, char *)
//

sstring SystemTask::Tasks(TBeing *ch, const char *args) 
{
  _task	*tsk;

  if (is_abbrev(args, "enabled") && ch->hasWizPower(POWER_WIZARD)) {
    taskstatus = true;
    return "The task system has been enabled.\n\r";
  } else if (is_abbrev(args, "disabled") && ch->hasWizPower(POWER_WIZARD)) {
    taskstatus = false;
    return "The task system has been disabled.\n\r";
  } else {
    if (top) {
      char lbuf[256];
      sstring str;
      sprintf(lbuf, "%-10s %s\n\r", "Owner", "Task");
      str = lbuf;
      for(tsk=top; tsk; tsk=tsk->next) {
        sprintf(lbuf, "%-10s %s\n\r", tsk->owner->getName(), tsk->cmd);
        str += lbuf;
      }
      return str;
    } else {
      return "There are no tasks running.\n\r";
    }
  }
}

//
//  SystemTask::remove(_task)
//
void SystemTask::remove(_task *tsk) 
{
  if (!tsk) {
    vlogf(LOG_BUG, "WARNING: SystemTask::remove(): trying to remove NULL task");
    return;
  }
  if (tsk == top) 
    top = tsk->next;

  if (tsk == bot)
     bot = tsk->prev;

  if (tsk->prev)
     tsk->prev->next = tsk->next;

  if (tsk->next) 
    tsk->next->prev = tsk->prev;

  delete tsk;
}

//
// SystemTask::start_task()
//  
void SystemTask::start_task()
{
  if (!top) 
    return;

  if (top->pid) {
    vlogf(LOG_BUG, fmt("ERROR: SystemTask::start_task(): task '%s' is already running.") %  top->cmd);
    return;
  }
  unlink(TMPFILE);
  if (forktask(top)) {
    vlogf(LOG_BUG, fmt("ERROR: SystemTask::AddTask(): forktask() for task '%s' failed.") %  top->cmd);
    top->owner->sendTo("Your task failed and has been deleted.\n\r");
    remove(top);
  }
}

//
//  SystemTask::forktask(_task *)
//
int SystemTask::forktask(_task *tsk)
{
  extern char **environ;
  char cmd[32], *argv[4];

  if (!tsk) {
    vlogf(LOG_BUG, "SystemTask::forktask tsk is NULL!");
    return(1);
  }
  if (!tsk->cmd) {
    vlogf(LOG_BUG, "SystemTask::forktask tsk->cmd is NULL!");
    remove(tsk);
    return(1);
  }
  vlogf(LOG_SILENT, fmt("INFO: task '%s' started.") %  tsk->cmd);
  top->owner->sendTo("Your task has started.\n\r");
  
  sscanf(tsk->cmd, "%s", cmd);
  char tmp[9];
  strcpy(tmp, "-c");
  argv[0] = cmd;
  argv[1] = tmp;
  argv[2] = tsk->cmd;
  argv[3] = NULL;

  if (!(tsk->pid = vfork())) {
    execve("/bin/sh", argv, environ);
    _exit(-1);
  }
  if (tsk->pid < 0) {
    vlogf(LOG_BUG, "ERROR: SystemTask::forktask(): vfork() failed.");
    return(1);
  }
  return(0);
}
