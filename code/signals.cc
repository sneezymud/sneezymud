//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: signals.cc,v $
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#if defined LINUX
// Linux systems will reset the signal after it gets raised
// According to the man page, we can get around this by using different include
// #include <bsd/signal.h>    doesn't seem to compile though
//#include <bsd/signal.h>
#include <csignal>
#else
#include <csignal>
#endif

#include <sys/time.h>
#include "stdsneezy.h"

void checkpointing(int);
void shutdownRequest(int);
void shutdownAndPurgeRequest(int);
void purgeRequest(int);
void logsig(int);
void hupsig(int);
void profsig(int);
extern void genericPurgeLdead(TBeing *ch);

void signalSetup(void)
{
  signal(SIGUSR1, purgeRequest);
  signal(SIGUSR2, shutdownAndPurgeRequest);
  signal(SIGQUIT, shutdownRequest);
  signal(SIGHUP, hupsig);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, hupsig);
  signal(SIGALRM, logsig);
  signal(SIGTERM, hupsig);
// Trapping PROF PREVENTS the timing signals from working correctly
//   signal(SIGPROF, profsig);

#ifndef SOLARIS
  struct itimerval itime;
  struct timeval interval;

  // This stuff crashes on the Solaris machine
  // set up the deadlock-protection 

  interval.tv_sec = 900;
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, 0);
  signal(SIGVTALRM, checkpointing);
#endif
}

void checkpointing(int)
{
  mud_assert(tics, "CHECKPOINT shutdown: tics not updated. (%d)", tics);
  tics = 0;
}

void shutdownAndPurgeRequest(int num)
{
  purgeRequest(num);
  shutdownRequest(num);
}

void purgeRequest(int)
{
  vlogf(10, "Received USR1 or USR2 signal - request to purge linkdeads");

  genericPurgeLdead(NULL);
}

void shutdownRequest(int)
{
  vlogf(10, "Received USR2 or QUIT - shutdown request");
  char buf[2000];

  int num = 5;
  if (!timeTill)
    timeTill = time(0) + (num * SECS_PER_REAL_MIN);
  else if (timeTill < (time(0) + (num * SECS_PER_REAL_MIN))) {
    vlogf(10, "Shutdown in progress overrides request.");
    return;
  } else {
    timeTill = time(0) + (num * SECS_PER_REAL_MIN);
  }

  sprintf(buf, "<r>******* SYSTEM MESSAGE ******<z>\n\r<c>%s in %ld minute%s.<z>\n\r",
     shutdown_or_reboot().c_str(),
     ((timeTill - time(0)) / 60), (((timeTill - time(0)) / 60) == 1) ? "" : "s");
  descriptor_list->worldSend(buf, NULL);
}

void hupsig(int)
{
  vlogf(10, "Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
  exit(0); /* something more elegant should perhaps be substituted */
}

void logsig(int)
{
  vlogf(10, "Signal received. Ignoring.");
}

void profsig(int)
{
// prof signals come in if prof/gprof is enabled
// we have to process these sigs, but ignore them
  vlogf(10, "SIGPROF caught.  Ignoring.");
}
