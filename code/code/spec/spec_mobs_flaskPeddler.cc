#include "monster.h"
#include "extern.h"

const int FLASK_COST = 50000;
const int FLASK_SMALL_VNUM = 33307;
const int FLASK_LARGE_VNUM = 33308;
const unsigned int LABEL_MAX = 20;

int flaskPeddler(TBeing* ch, cmdTypeT cmd, const char* arg, TMonster* peddler,
  TObj*) {
  if (cmd != CMD_BUY) {
    return FALSE;
  }
  TObj* flask;
  sstring sarg = arg;

  int cost_mult = 1;
  if (is_abbrev(sarg.word(0), "small")) {
    cost_mult = 1;
  } else if (is_abbrev(sarg.word(0), "large")) {
    cost_mult = 4;
  } else {
    peddler->doSay("What the hell are you talking about?");
    return TRUE;
  }

  sstring inscrip = sarg.word(1).upper();

  if (!inscrip.isWord() || inscrip.length() > LABEL_MAX) {
    peddler->doSay("Are you daft?");
    peddler->doSay("I can't write that on here!");
    return TRUE;
  }

  if (ch->getMoney() >= (FLASK_COST * cost_mult)) {
    if (cost_mult == 1)
      flask = read_object(FLASK_SMALL_VNUM, VIRTUAL);
    else if (cost_mult == 4)
      flask = read_object(FLASK_LARGE_VNUM, VIRTUAL);
    else
      return TRUE;
    if (!flask)
      return TRUE;
    ch->addToMoney((-FLASK_COST * cost_mult), GOLD_SHOP_RESPONSES);
  } else {
    peddler->doSay("I don't deal with paupers!");
    peddler->doAction("", CMD_SNICKER);
    return TRUE;
  }

  sstring newName, newShort;
  if (cost_mult == 1) {
    newName = format("flask golden small %s") % inscrip;
    newShort = format("<Y>a small flask <z><b>labelled <z><W>%s<z>") % inscrip;
  } else {
    newName = format("flask golden %s") % inscrip;
    newShort = format("<Y>a flask <z><b>labelled <z><W>%s<z>") % inscrip;
  }

  flask->swapToStrung();
  flask->name = newName;
  flask->shortDescr = newShort;

  act("You work your magic on the flask.", TRUE, peddler, flask, 0, TO_CHAR);
  act("$n mumbles something and touches the flask.", TRUE, peddler, flask, 0,
    TO_ROOM);

  sstring buf =
    format("The letters <W>%s<z> appears on the flask's label.") % inscrip;
  act(buf, TRUE, peddler, NULL, 0, TO_CHAR);
  act(buf, TRUE, peddler, NULL, 0, TO_ROOM);

  peddler->doSay("Well, there you are then.");
  *peddler += *flask;
  sstring giveBuf =
    format("%s %s") % add_bars(flask->name) % add_bars(ch->name);
  peddler->doGive(giveBuf, GIVE_FLAG_IGN_DEX_TEXT);

  peddler->doSay("Please come again!");
  return TRUE;
}
