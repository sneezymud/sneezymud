//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "socket.h"
#include "configuration.h"
#include "extern.h"
#include "enum.h"
#include "version.h"

#include <stdio.h>

extern "C" {
#include <unistd.h>
}

extern int run_the_game();


#ifndef LOWTOOLS

int main(int argc, char *argv[])
{
  vlogf(LOG_MISC, "Sneezy version " VERSION);
  int a;

  if(!Config::doConfiguration(argc, argv))
    return 0;

  if(Config::NoSpecials())
    vlogf(LOG_MISC, "Suppressing assignment of special routines.");

  if(Config::bTrimmed()){
    vlogf(LOG_MISC, "Loading as trimmed port.");
    gamePort = Config::Port::GAMMA;
  }

  Uptime = time(0);

  vlogf(LOG_MISC, format("Running %s on port %d.") %  MUD_NAME % gamePort);

  if (chdir(Config::DataDir().c_str()) < 0) {
    perror("chdir");
    exit(0);
  }
  vlogf(LOG_MISC, format("Using %s as data directory.") % Config::DataDir());

  srand(time(0));

  WizLock = false;

  vlogf(LOG_MISC, "Blanking denied hosts.");
  for (a = 0; a < MAX_BAN_HOSTS; a++) {
    strcpy(hostLogList[a], "");
    strcpy(hostlist[a], "");
  }
  numberhosts = 0;
  numberLogHosts = 0;

  int ret = run_the_game();

  generic_cleanup();

  return ret;
}

#endif
