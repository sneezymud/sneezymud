//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "cmd_news.cc" - The news command
//
//////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>

#include "extern.h"
#include "being.h"
#include "statistics.h"
#include <fstream>

class newsFileList {
  public:
    sstring fileName;
    time_t modTime;
    sstring prependStr;

  newsFileList(sstring a, time_t tim, sstring b) :
    fileName(a),
    modTime(tim),
    prependStr(b)
  {}
  newsFileList() :
    fileName(""),
    modTime(0),
    prependStr("")
  {}
};

class newsFileSorter {
  public:
    bool operator() (const newsFileList &, const newsFileList &) const;
};

bool newsFileSorter::operator() (const newsFileList &x, const newsFileList &y) const
{
  return  (x.modTime > y.modTime);
}

void TBeing::doNews(const char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  one_argument(argument, arg, cElements(arg));

  // check files mod times and see what has changed recently
  DIR *dfd;
  struct dirent *dp;
  time_t now = time(0);
  char buf[256];
  char timebuf[256];
  struct stat theStat;
  std::vector<newsFileList>vecFiles(0);

  vecFiles.clear();

  dfd = opendir(Path::HELP);
  if (!dfd) {
    vlogf(LOG_FILE, "doNews: Failed opening directory.");
    return;
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") ||
        !strcmp(dp->d_name, "..") ||
        !strcmp(dp->d_name, "_builder") ||
        !strcmp(dp->d_name, "_immortal") ||
        !strcmp(dp->d_name, "_spells") ||
        !strcmp(dp->d_name, "_skills"))
      continue;

    sprintf(buf, "%s/%s", Path::HELP, dp->d_name);
    if (!stat(buf, &theStat)) {
      if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
        newsFileList nfl(dp->d_name, theStat.st_mtime, "help ");

        vecFiles.push_back(nfl);
      }
    }
  }
  closedir(dfd);

  dfd = opendir(Path::SKILL_HELP);
  if (!dfd) {
    vlogf(LOG_FILE, "doNews: Failed opening directory.");
    return;
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") ||
        !strcmp(dp->d_name, ".."))
      continue;

    sprintf(buf, "%s/%s", Path::SKILL_HELP, dp->d_name);
    if (!stat(buf, &theStat)) {
      if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
        newsFileList nfl(dp->d_name, theStat.st_mtime, "help ");
        vecFiles.push_back(nfl);
      }
    }
  }
  closedir(dfd);

  dfd = opendir(Path::SPELL_HELP);
  if (!dfd) {
    vlogf(LOG_FILE, "doNews: Failed opening directory.");
    return;
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") ||
        !strcmp(dp->d_name, ".."))
      continue;

    sprintf(buf, "%s/%s", Path::SPELL_HELP, dp->d_name);
    if (!stat(buf, &theStat)) {
      if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
        newsFileList nfl(dp->d_name, theStat.st_mtime, "help ");
        vecFiles.push_back(nfl);
      }
    }
  }
  closedir(dfd);

  if (GetMaxLevel() > MAX_MORT && isImmortal()) {
    dfd = opendir(Path::BUILDER_HELP);
    if (!dfd) {
      vlogf(LOG_FILE, "doNews: Failed opening directory.");
      return;
    }
    while ((dp = readdir(dfd))) {
      if (!strcmp(dp->d_name, ".") ||
          !strcmp(dp->d_name, ".."))
        continue;

      sprintf(buf, "%s/%s", Path::BUILDER_HELP, dp->d_name);
      if (!stat(buf, &theStat)) {
        if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
          newsFileList nfl(dp->d_name, theStat.st_mtime, "help ");
          vecFiles.push_back(nfl);
        }
      }
    }
    closedir(dfd);
  }

  if (hasWizPower(POWER_IMMORTAL_HELP) && isImmortal()) {
    dfd = opendir(Path::IMMORTAL_HELP);
    if (!dfd) {
      vlogf(LOG_FILE, "doNews: Failed opening directory.");
      return;
    }
    while ((dp = readdir(dfd))) {
      if (!strcmp(dp->d_name, ".") ||
          !strcmp(dp->d_name, ".."))
        continue;

      sprintf(buf, "%s/%s", Path::IMMORTAL_HELP, dp->d_name);
      if (!stat(buf, &theStat)) {
        if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
          newsFileList nfl(dp->d_name, theStat.st_mtime, "help ");
          vecFiles.push_back(nfl);
        }
      }
    }
    closedir(dfd);
  }

  // motd, and wizmotd
  if (!stat(File::MOTD, &theStat)) {
    if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
      newsFileList nfl("motd", theStat.st_mtime, "");
      vecFiles.push_back(nfl);
    } else if (isImmortal()) {
      // slightly bizarre way of doing this, but wizmotd has same cmd as motd
      if (!stat(File::WIZMOTD, &theStat)) {
        if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
          newsFileList nfl("motd", theStat.st_mtime, "");
          vecFiles.push_back(nfl);
        }
      }
    }
  }

  // credits
  if (!stat(File::CREDITS, &theStat)) {
    if (now - theStat.st_mtime <= (3 * SECS_PER_REAL_DAY)) {
      newsFileList nfl("credits", theStat.st_mtime, "");
      vecFiles.push_back(nfl);
    }
  }

  sstring str;

  str += "<H> News\n\r";
  str += "------------------------------------------------------\n\r";

  if (vecFiles.size()) {
    std::sort(vecFiles.begin(), vecFiles.end(), newsFileSorter());

    str += "The following information files have changed recently:\n\r";
    str += "------------------------------------------------------\n\r";

    unsigned int iter;
    for (iter = 0; iter < vecFiles.size(); iter++) {
      strcpy(timebuf, ctime(&(vecFiles[iter].modTime)));
      timebuf[strlen(timebuf) - 1] = '\0';
  
      sprintf(buf, "%s : %s%s\n\r", timebuf, 
            vecFiles[iter].prependStr.c_str(),
            vecFiles[iter].fileName.c_str()); 
      str += buf;
    }
  }

#if 1

  if(*arg){
    std::ifstream news(File::NEWS);
    sstring s;

    while(news.getline(buf, 256)){
      if(!*buf){
	if(s.find(arg) != sstring::npos){
	  str+=s;
	}
	s="";
      }
      
      s+=buf;
      s+="\n\r";
    }
  } else 
    file_to_sstring(File::NEWS, str, CONCAT_YES);

  if (desc) {
    news_used_num++;
    desc->page_string(str.toCRLF());
  }

#else
  if (desc) {
    news_used_num++;
    desc->start_page_file(File::NEWS, "No news is good news!\n\r");
  }
#endif
}
