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
#include "extern.h"
#include "person.h"

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

  interval.tv_sec = 1200;
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
  vlogf(LOG_MISC, "Received USR1 or USR2 signal - request to purge linkdeads");

  genericPurgeLdead(NULL);
}

void shutdownRequest(int)
{
  vlogf(LOG_MISC, "Received USR2 or QUIT - shutdown request");
  char buf[2000];

  int num = 5;
  if (!timeTill)
    timeTill = time(0) + (num * SECS_PER_REAL_MIN);
  else if (timeTill < (time(0) + (num * SECS_PER_REAL_MIN))) {
    vlogf(LOG_MISC, "Shutdown in progress overrides request.");
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
  vlogf(LOG_MISC, "Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
  exit(0); /* something more elegant should perhaps be substituted */
}

void logsig(int)
{
  vlogf(LOG_MISC, "Signal received. Ignoring.");
}

void profsig(int)
{
// prof signals come in if prof/gprof is enabled
// we have to process these sigs, but ignore them
  vlogf(LOG_MISC, "SIGPROF caught.  Ignoring.");
}
