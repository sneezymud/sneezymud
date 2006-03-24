/* ************************************************************************
*  file: signals.c , trapping of signals from Unix.       Part of DIKUMUD *
*  Usage : Signal Trapping.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <signal.h>
#include <stdio.h>
#include <sys/time.h>

#include "structs.h"
#include "utils.h"

void checkpointing(int);
void shutdown_request(int);
void logsig(int);
void hupsig(int);

void signal_setup(void)
{
	struct itimerval itime;
	struct timeval interval;

	signal(SIGUSR2, shutdown_request);

	/* just to be on the safe side: */

	signal(SIGHUP, hupsig);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, hupsig);
	signal(SIGALRM, logsig);
	signal(SIGTERM, hupsig);

	/* set up the deadlock-protection */

	interval.tv_sec = 900;    /* 15 minutes */
	interval.tv_usec = 0;
	itime.it_interval = interval;
	itime.it_value = interval;
	setitimer(ITIMER_VIRTUAL, &itime, 0);
	signal(SIGVTALRM, checkpointing);
}



void checkpointing(int tmp)
{
  extern int tics;
	
  if (!tics) {
    log("CHECKPOINT shutdown: tics not updated");
    abort();
  } else
    tics = 0;
}




void shutdown_request(int tmp)
{
	extern int Shutdown;

	log("Received USR2 - shutdown request");
	Shutdown = 1;
}



/* kick out players etc */
void hupsig(int tmp)
{
	extern int Shutdown;

	log("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
	exit(0);   /* something more elegant should perhaps be substituted */
}



void logsig(int tmp)
{
	log("Signal received. Ignoring.");
}
