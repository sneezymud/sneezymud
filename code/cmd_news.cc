//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "cmd_news.cc" - The news command
//
//////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/stat.h>

#include "stdsneezy.h"
#include "statistics.h"

void TBeing::doNews(const char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  one_argument(argument, arg);

  if (argument && is_abbrev(argument, "changes")) {
    // check files mod times and see what has changed recently
    DIR *dfd;
    struct dirent *dp;
    time_t now = time(0);
    char buf[256];
    char timebuf[256];
    struct stat theStat;

    sendTo("The following files have changed recently:\n\r");
    sendTo("------------------------------------------\n\r");

    dfd = opendir(HELP_PATH);
    if (!dfd) {
      vlogf(LOG_FILE, "doNews: Failed opening directory.");
      return;
    }
    while ((dp = readdir(dfd))) {
      if (!strcmp(dp->d_name, ".") ||
          !strcmp(dp->d_name, ".."))
        continue;

      sprintf(buf, "%s/%s", HELP_PATH, dp->d_name);
      if (!stat(buf, &theStat)) {
        if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
          strcpy(timebuf, ctime(&(theStat.st_mtime)));
          timebuf[strlen(timebuf) - 1] = '\0';

          sendTo("%s : %s\n\r", dp->d_name, timebuf); 
        }
      }
    }
    closedir(dfd);

    dfd = opendir(SKILL_HELP_PATH);
    if (!dfd) {
      vlogf(LOG_FILE, "doNews: Failed opening directory.");
      return;
    }
    while ((dp = readdir(dfd))) {
      if (!strcmp(dp->d_name, ".") ||
          !strcmp(dp->d_name, ".."))
        continue;

      sprintf(buf, "%s/%s", SKILL_HELP_PATH, dp->d_name);
      if (!stat(buf, &theStat)) {
        if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
          strcpy(timebuf, ctime(&(theStat.st_mtime)));
          timebuf[strlen(timebuf) - 1] = '\0';

          sendTo("%s : %s\n\r", dp->d_name, timebuf); 
        }
      }
    }
    closedir(dfd);

    dfd = opendir(SPELL_HELP_PATH);
    if (!dfd) {
      vlogf(LOG_FILE, "doNews: Failed opening directory.");
      return;
    }
    while ((dp = readdir(dfd))) {
      if (!strcmp(dp->d_name, ".") ||
          !strcmp(dp->d_name, ".."))
        continue;

      sprintf(buf, "%s/%s", SPELL_HELP_PATH, dp->d_name);
      if (!stat(buf, &theStat)) {
        if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
          strcpy(timebuf, ctime(&(theStat.st_mtime)));
          timebuf[strlen(timebuf) - 1] = '\0';

          sendTo("%s : %s\n\r", dp->d_name, timebuf); 
        }
      }
    }
    closedir(dfd);

    if (GetMaxLevel() > MAX_MORTAL && isImmortal()) {
      dfd = opendir(BUILDER_HELP_PATH);
      if (!dfd) {
        vlogf(LOG_FILE, "doNews: Failed opening directory.");
        return;
      }
      while ((dp = readdir(dfd))) {
        if (!strcmp(dp->d_name, ".") ||
            !strcmp(dp->d_name, ".."))
          continue;
  
        sprintf(buf, "%s/%s", BUILDER_HELP_PATH, dp->d_name);
        if (!stat(buf, &theStat)) {
          if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
            strcpy(timebuf, ctime(&(theStat.st_mtime)));
            timebuf[strlen(timebuf) - 1] = '\0';
  
            sendTo("%s : %s\n\r", dp->d_name, timebuf); 
          }
        }
      }
      closedir(dfd);
    }

    if (hasWizPower(POWER_IMMORTAL_HELP) && isImmortal()) {
      dfd = opendir(BUILDER_HELP_PATH);
      if (!dfd) {
        vlogf(LOG_FILE, "doNews: Failed opening directory.");
        return;
      }
      while ((dp = readdir(dfd))) {
        if (!strcmp(dp->d_name, ".") ||
            !strcmp(dp->d_name, ".."))
          continue;
  
        sprintf(buf, "%s/%s", BUILDER_HELP_PATH, dp->d_name);
        if (!stat(buf, &theStat)) {
          if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
            strcpy(timebuf, ctime(&(theStat.st_mtime)));
            timebuf[strlen(timebuf) - 1] = '\0';
  
            sendTo("%s : %s\n\r", dp->d_name, timebuf); 
          }
        }
      }
      closedir(dfd);
    }

    return;
  }

  if (desc) {
    news_used_num++;
    desc->start_page_file(NEWS_FILE, "No news is good news!\n\r");
  }
}
