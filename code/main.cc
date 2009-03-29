//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "socket.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

extern "C" {
#include <unistd.h>
}

extern int run_the_game();


#ifndef LOWTOOLS

void sendHelp(po::options_description desc){
  cout << "Usage: sneezy [options] [port]" << endl;
  cout << desc;  
}

int main(int argc, char *argv[])
{
  int a;
  sstring dir;
  bool bTrimmed = false;
  
  po::options_description cmdline("Command line options");
  cmdline.add_options()
    ("help", 
     "produce help message")
    ("directory,d", po::value<string>(&dir)->default_value(DFLT_DIR), 
     "data directory to run in")
    ("nospecials,s", po::value<bool>(&noSpecials)->zero_tokens(),
     "suppress assignment of special routines")
    ("trimmed,t", po::value<bool>(&bTrimmed)->zero_tokens(),
     "load as trimmed port")
    ("port,p", po::value<int>(&gamePort)->default_value(PROD_GAMEPORT),
     "game port")
    ;

  // first positional argument is port number
  po::positional_options_description p;
  p.add("port", -1);
  
  po::variables_map vm;

  try {
    po::store(po::command_line_parser(argc, argv).
	      options(cmdline).positional(p).run(), vm);
  } catch(po::unknown_option){
    sendHelp(cmdline);
    return 0;    
  }

  po::notify(vm);

  if(vm.count("help")){
    sendHelp(cmdline);
    return 0;
  }

  if(noSpecials)
    vlogf(LOG_MISC, "Suppressing assignment of special routines.");

  if (bTrimmed){
    vlogf(LOG_MISC, "Loading as trimmed port.");
    GAMMA_GAMEPORT = gamePort;
  }

  Uptime = time(0);

  vlogf(LOG_MISC, fmt("Running %s on port %d.") %  MUD_NAME % gamePort);

  if (chdir(dir.c_str()) < 0) {
    perror("chdir");
    exit(0);
  }
  vlogf(LOG_MISC, fmt("Using %s as data directory.") %  dir);

  srand(time(0));

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
  // graceful, but too bad its not informative about the exception
  // (could try vlogf_trace here instead of assert?)
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
