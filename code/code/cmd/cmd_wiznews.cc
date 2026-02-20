//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "cmd_wiznews.cc" - The wiznews command
//
//////////////////////////////////////////////////////////////////////////

#include "cmd_news.h"

#include "account.h"
#include "being.h"
#include "extern.h"
#include "statistics.h"

void TBeing::doWiznews(const char* argument) {
  if (!desc || desc->connected)
    return;

  if (isImmortal() || IS_SET(desc->account->flags, TAccount::IMMORTAL)) {
    wiznews_used_num++;

    char arg[MAX_INPUT_LENGTH];
    one_argument(argument, arg, cElements(arg));

    sstring str = assembleNewsEntries(File::WIZNEWS_DIR, arg);

    if (str.empty())
      str = "No news for the immorts!\n\r";

    desc->page_string(str.toCRLF());
  } else {
    sendTo("This command is for immortals only.\n\r");
  }
}
