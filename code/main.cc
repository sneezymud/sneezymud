//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "socket.h"

extern "C" {
#include <unistd.h>
}

extern int run_the_game();


#ifndef LOWTOOLS

int main(int argc, char *argv[])
{
#if 0
  printf("Checking new/delete\n");
  char *s = mud_str_dup("test sstring");
  delete s;
  printf("Checking unused.\n");
  int x = 5;
  int j;
  printf("done\n");
  exit(0);
#endif
  int a, pos = 1;
  char dir[256];
  bool bTrimmed = false;

  gamePort = PROD_GAMEPORT;   // set as default
  strcpy(dir, DFLT_DIR);

  while ((pos < argc) && (*(argv[pos]) == '-')) {
    switch (*(argv[pos] + 1)) {
      case 'd':
	if (*(argv[pos] + 2))
	  strcpy(dir, argv[pos] + 2);
	else if (++pos < argc)
	  strcpy(dir, argv[pos]);
	else {
	  vlogf(LOG_MISC, "Directory arg expected after option -d.");
	  exit(0);
	}
	break;

      case 's':
	noSpecials = 1;
	vlogf(LOG_MISC, "Suppressing assignment of special routines.");
	break;

      case 't':
        bTrimmed = true;
        vlogf(LOG_MISC, "Loading as trimmed port.");
        break;

      default:
	vlogf(LOG_MISC, fmt("Unknown option -% in argument sstring.") %  *(argv[pos] + 1));
	break;
    }
    pos++;
  }

  if (pos < argc) {
    if (!isdigit(*argv[pos])) {
      vlogf(LOG_MISC, fmt("Usage: %s [-s] [-d pathname] [ port # ]\n") %  argv[0]);
      exit(0);
    } else if ((gamePort = convertTo<int>(argv[pos])) <= 1024) {
      printf("Illegal port #\n");
      exit(0);
    }
  }

  if (bTrimmed)
    GAMMA_GAMEPORT = gamePort;

  Uptime = time(0);

  vlogf(LOG_MISC, fmt("Running %s on port %d.") %  MUD_NAME % gamePort);

  if (chdir(dir) < 0) {
    perror("chdir");
    exit(0);
  }
  vlogf(LOG_MISC, fmt("Using %s as data directory.") %  dir);

  srandom(time(0));

  WizLock = false;

  if (gamePort == BETA_GAMEPORT) {
    vlogf(LOG_MISC, "Running on beta test site.  Wizlocking by default.");
    WizLock = TRUE;
  }

  vlogf(LOG_MISC, "Blanking denied hosts.");
  for (a = 0; a < MAX_BAN_HOSTS; a++) {
    strcpy(hostLogList[a], "");
    strcpy(hostlist[a], "");
  }
  numberhosts = 0;
  numberLogHosts = 0;

#if 0
  extern void convert_all_pfiles();
  convert_all_pfiles();
#endif
#if 0
  extern void convert_all_rentfiles();
  convert_all_rentfiles();
#endif
#if 0
  // graceful, but too bad its not informative about the exception
  try {
    run_the_game();
  } catch (...) {
    mud_assert(0, "Caught an exception");
  }
#else
  run_the_game();
#endif

  generic_cleanup();

  return (0);
}

#endif
