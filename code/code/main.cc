//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "socket.h"
#include "configuration.h"
#include "extern.h"
#include "enum.h"
#include "discord.h"
#include "sstring.h"

#include <stdio.h>
#include <filesystem>
#include <fstream>
#include <iterator>

extern "C" {
#include <unistd.h>
}

extern int run_the_game();

#ifndef LOWTOOLS

std::mt19937 rng;
sstring MUD_NAME_VERS;

namespace {
  // Expects to be called only *after* Config::doConfiguration() has executed.
  // lib/version.txt is created by the build system, as defined in the
  // SConstruct file, and should consist of two lines - the first being the
  // commit hash, and the second being the date of the commit.
  sstring readVersionFromFile() {
    const std::filesystem::path versionPath = std::filesystem::current_path() /
                                              Config::DataDir().c_str() /
                                              "version.txt";
    std::ifstream file(versionPath);

    if (!file) {
      vlogf(LOG_FILE, format("Couldn't read commit hash/date from %s.") %
                        versionPath.string());
      return "(build ?, ?)";
    }

    sstring hash;
    std::getline(file, hash);

    sstring date;
    std::getline(file, date);

    return "(build " + hash + ", " + date + ")";
  }
}  // namespace

int main(int argc, char* argv[]) {
  int a;

  if (!Config::doConfiguration(argc, argv))
    return 0;

  // Do this after loading configuration, as version.txt is written to the lib
  // directory, the location of which is defined in the config file.
  MUD_NAME_VERS = format("%s %s") % MUD_NAME % readVersionFromFile();
  vlogf(LOG_MISC, MUD_NAME_VERS);

  if (!Discord::doConfig())
    vlogf(LOG_MISC, "Discord configuration failed.");

  if (Config::NoSpecials())
    vlogf(LOG_MISC, "Suppressing assignment of special routines.");

  if (Config::bTrimmed()) {
    vlogf(LOG_MISC, "Loading as trimmed port.");
    gamePort = Config::Port::GAMMA;
  }

  Uptime = time(0);

  vlogf(LOG_MISC, format("Running %s on port %d.") % MUD_NAME % gamePort);

  if (chdir(Config::DataDir().c_str()) < 0) {
    perror("chdir");
    exit(0);
  }
  vlogf(LOG_MISC, format("Using %s as data directory.") % Config::DataDir());

  srand(time(0));
  std::random_device rd;
  rng = std::mt19937(rd());

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
