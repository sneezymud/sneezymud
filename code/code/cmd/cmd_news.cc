//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "cmd_news.cc" - The news command
//
//////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iosfwd>
#include <istream>
#include <string>

#include "being.h"
#include "connect.h"
#include "db.h"
#include "enum.h"
#include "extern.h"
#include "parse.h"
#include "sstring.h"
#include "statistics.h"
#include "structs.h"

void TBeing::doNews(const char* argument) {
  if (!desc)
    return;

  news_used_num++;

  char arg[MAX_INPUT_LENGTH];
  one_argument(argument, arg, cElements(arg));
  sstring str;

  if (*arg) {
    std::ifstream news(File::NEWS);
    sstring s;

    char buf[256];
    while (news.getline(buf, 256)) {
      if (!*buf) {
        if (s.find(arg) != sstring::npos) {
          str += s;
        }
        s = "";
      }

      s += buf;
      s += "\n\r";
    }
  } else {
    file_to_sstring(File::NEWS, str, CONCAT_YES);
  }

  desc->page_string(str.toCRLF());
}
