//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "statistics.h"
#include "obj_player_corpse.h"
#include "obj_money.h"

TMoney::TMoney() :
  TObj(),
  money(0)
{
}

TMoney::TMoney(const TMoney &a) :
  TObj(a),
  money(a.money)
{
}

TMoney & TMoney::operator=(const TMoney &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  money = a.money;
  return *this;
}

TMoney::~TMoney()
{
}

void TMoney::assignFourValues(int x1, int, int, int)
{
  setMoney(x1);
}

void TMoney::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getMoney();
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TMoney::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Talens in pile: %d", getMoney());

  sstring a(buf);
  return a;
}

TMoney *create_money(int amount)
{
  TObj *obj;
  TMoney *money;

  
  if(amount<0){
    vlogf(LOG_BUG, fmt("ERROR: Try to create negative money (%i).") %  amount);
    amount=1;
  }

  obj = read_object(GENERIC_TALEN, VIRTUAL);
  money = dynamic_cast<TMoney *>(obj);
  mud_assert(money != NULL, "create_money created something that was not TMoney.  obj was: %s", obj ? obj->getName() : "NO OBJECT");

  extraDescription *new_descr;
  char buf[80];

  money->swapToStrung();

  // clean off any descriptions that came through from the tinyworld file
  while ((new_descr = money->ex_description)) {
    money->ex_description = money->ex_description->next;
    delete new_descr;
  }

  new_descr = new extraDescription();

  delete [] money->name;
  delete [] money->shortDescr;
  delete [] money->getDescr();
  if (amount == 1) {
    money->name = mud_str_dup("talens money");
    money->shortDescr = mud_str_dup("a talen");
    money->setDescr(mud_str_dup("One miserable talen lies here."));

    new_descr->keyword = mud_str_dup("talen money");
    new_descr->description = mud_str_dup("One miserable talen.\n\r");

  } else {
    money->name = mud_str_dup("talens money");
    money->shortDescr = mud_str_dup("some talens");
    if (amount > 100000)
      sprintf(buf, "A tremendously HUGE pile of talens lies here.");
    else if (amount > 50000)
      sprintf(buf, "A HUGE pile of talens lies here.");
    else if (amount > 10000)
      sprintf(buf, "A LARGE pile of talens lies here.");
    else if (amount > 1000)
      sprintf(buf, "A nice-sized pile of talens lies here.");
    else if (amount > 500)
      sprintf(buf, "A pile of talens lies here.");
    else if (amount > 100)
      sprintf(buf, "A small pile of talens lies here.");
    else if (amount > 50)
      sprintf(buf, "A tiny pile of talens lies here.");
    else
      sprintf(buf, "A few talens have been left in a pile here.");

    money->setDescr(mud_str_dup(buf));
    new_descr->keyword = mud_str_dup("talens money");
    if (amount < 10) {
      sprintf(buf, "There are %d talens.\n\r", amount);
      new_descr->description = mud_str_dup(buf);
    } else if (amount < 100) {
      sprintf(buf, "There are about %d talens.\n\r", 10 * (amount / 10));
      new_descr->description = mud_str_dup(buf);
    } else if (amount < 10000) {
      sprintf(buf, "You guess there are %d talens.\n\r", 100 * (amount / 100));
      new_descr->description = mud_str_dup(buf);
    } else
      new_descr->description = mud_str_dup("There are a LOT of talens.\n\r");
  }
  new_descr->next = NULL;
  money->ex_description = new_descr;

  money->obj_flags.wear_flags = ITEM_TAKE;
  money->obj_flags.decay_time = -1;
  money->setMoney(amount);
  money->obj_flags.cost = amount;

  money->setVolume(amount/2 + amount%2);
  money->setWeight(amount/75.0);

  return money;
}

int TMoney::getMe(TBeing *ch, TThing *sub)
{
  // do baseclass stuff for recusivity
  int rc = TObj::getMe(ch, sub);
  if (rc)
    return rc;

  rc = moneyMeMoney(ch, sub);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    // returning DELETE_THIS would cause "get all" to stop
    delete this;
  }
  return TRUE;
}

int TMoney::getMoney() const
{
  return money;
}

void TMoney::setMoney(int n)
{
  money = n;
}

int TMoney::moneyMeMoney(TBeing *ch, TThing *sub)
{
  int amount;
  TThing *t;
  char buf[256];
  bool isMyCorpse = false;

  if (sub && isname(ch->name, sub->name) && dynamic_cast<TPCorpse*>(sub))
    isMyCorpse = true;

  (*this)--;
  amount = getMoney();
  if (amount == 1) {
    ch->sendTo("There was one talen.\n\r");
  } else {
    int amt2 = 0;
    if (!isMyCorpse && !ch->isImmortal())
      amt2 = (int) (amount * FactionInfo[ch->getFaction()].faction_tithe / 100.0);

    if (!amt2)
      ch->sendTo(fmt("There were %d talens.\n\r") % amount);
    else
      ch->sendTo(fmt("There were %d talens, and you tithe %d of them.\n\r") % amount % amt2);
  }

  if (ch->getMoney() > 500000 && (amount > 100000))
    vlogf(LOG_MISC, fmt("%s just got %d talens") %  ch->getName() % amount);

  for (t = ch->roomp->getStuff(); t; t = t->nextThing) {
    TBeing *tb = dynamic_cast<TBeing *>(t);
    if (!tb)
      continue;
    if (!tb->isPc() && tb->canSee(ch) && (tb != ch) &&
        (tb->getPosition() > POSITION_SLEEPING) && !UtilMobProc(tb)) {
      TMonster *tmons = dynamic_cast<TMonster *>(tb);
      tmons->UG(1 + amount/1000);
      tmons->aiTarget(ch);
      if (tmons->isGreedy())
        tmons->UA(1);
    }
  }
  if (sub && isname("slot", sub->name))
    ch->addToMoney(amount, GOLD_GAMBLE);
  else if (isMyCorpse) {
    ch->addToMoney(amount, GOLD_INCOME);

    // a queer issue, and mostly a kludge
    // corpse create: removed money from being
    // we want to put the money back onto me
    // but lets also fix gold_positive so that a L50 dying with 4M coins
    // doesn't cause gold_statistic to = 0, but gold_positive = 4M
    if (ch->isPc() && ch->GetMaxLevel() <= 60)
      gold_positive[GOLD_INCOME][ch->GetMaxLevel()-1] -= amount;
  } else
    ch->addToMoney(amount, GOLD_INCOME);

  // don't split coins from my own corpse
  if (!sub || !isname(fname(ch->name), sub->name)) {
    if (ch->isAffected(AFF_GROUP) && ch->desc &&
        IS_SET(ch->desc->autobits, AUTO_SPLIT) &&
        (ch->master || ch->followers)){
      sprintf(buf, "%d", amount);
      ch->doSplit(buf, false);
    }
  }

  return DELETE_THIS;
}

bool TMoney::isPluralItem() const
{
  return (getMoney() > 1);
}

void TMoney::onObjLoad()
{
  // adjust the money based on the global modifiers
  int x = getMoney();
  x = (int) (x * gold_modifier[GOLD_INCOME].getVal());
  setMoney(x);
}

sstring TMoney::getNameForShow(bool useColor, bool useName, const TBeing *ch) const
{
  char buf2[256];
  sprintf(buf2, "%s [%d talens]", useName ? name : (useColor ? getName() : getNameNOC(ch).c_str()), 
      getMoney());
  return buf2;
}
