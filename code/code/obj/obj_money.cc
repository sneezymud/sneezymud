//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "extern.h"
#include "room.h"
#include "low.h"
#include "monster.h"
#include "handler.h"
#include "statistics.h"
#include "obj_player_corpse.h"
#include "obj_money.h"
#include "spec_mobs.h"

currencyEntry *currencyInfoT::operator[] (const currencyTypeT i)
{
  if(currencies.find(i) == currencies.end()){
    vlogf(LOG_BUG, format("invalid currency detected: %i") % i);
    return currencies[CURRENCY_GRIMHAVEN];
  } else {
    return currencies[i];
  }
}

currencyInfoT::~currencyInfoT()
{
}

currencyInfoT::currencyInfoT()
{
  currencies[CURRENCY_GRIMHAVEN] = new currencyEntry("talen", "Grimhaven");
  currencies[CURRENCY_LOGRUS] = new currencyEntry("dinar", "Logrus");
  currencies[CURRENCY_BRIGHTMOON] = new currencyEntry("kroner", "Brightmoon");
  currencies[CURRENCY_AMBER] = new currencyEntry("guilder", "Amber");
}


float currencyEntry::getExchangeRate(currencyTypeT c)
{
  return 1.0; // no exchange rates... yet
}

currencyEntry::currencyEntry(sstring n, sstring a) :
  name(n),
  affiliation(a)
{
}

currencyEntry & currencyEntry::operator = (const currencyEntry &a) 
{
  if (this == &a) return *this;

  name = a.name;
  affiliation = a.affiliation;

  return *this;
}

currencyEntry::~currencyEntry()
{
}


bool TMoney::willMerge(TMergeable *tm)
{
  TMoney *tMoney;

  if(!(tMoney=dynamic_cast<TMoney *>(tm)) ||
     this==tMoney ||
     tMoney->getCurrency()!=getCurrency())
    return false;

  return true;
}

void TMoney::doMerge(TMergeable *tm)
{
  TMoney *tMoney;

  if(!(tMoney=dynamic_cast<TMoney *>(tm)))
    return;

  // set m to the full amount
  setMoney(getMoney() + tMoney->getMoney());

  --(*tMoney);
  delete tMoney;
}

currencyTypeT TMoney::getCurrency() const
{
  return type;
}

TMoney::TMoney() :
  TMergeable(),
  money(0)
{
}

TMoney::TMoney(const TMoney &a) :
  TMergeable(a),
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

void TMoney::assignFourValues(int x1, int x2, int, int)
{
  setMoney(x1);
  setCurrency((currencyTypeT)x2);
}

void TMoney::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getMoney();
  *x2 = getCurrency();
  *x3 = 0;
  *x4 = 0;
}

sstring TMoney::statObjInfo() const
{
  sstring buf;

  buf=format("%s in pile: %i") % currencyInfo[getCurrency()]->getName().cap() %
    getMoney();

  return buf;
}

sstring TMoney::getCurrencyName() const
{
  return currencyInfo[getCurrency()]->getName();
}

TMoney *create_money(int amount, factionTypeT fact)
{
  currencyTypeT currency=CURRENCY_GRIMHAVEN;

  switch(fact){
    case FACT_NONE:
    case FACT_UNDEFINED:
    case MAX_FACTIONS:
      currency=CURRENCY_GRIMHAVEN;
      break;
    case FACT_BROTHERHOOD:
      currency=CURRENCY_BRIGHTMOON;
      break;
    case FACT_CULT:
      currency=CURRENCY_LOGRUS;
      break;
    case FACT_SNAKE:
      currency=CURRENCY_AMBER;
      break;
  }

  return create_money(amount, currency);
}

TMoney *create_money(int amount, currencyTypeT currency)
{
  TObj *obj;
  TMoney *money;

  if(amount<0){
    vlogf(LOG_BUG, format("ERROR: Try to create negative money (%i).") %  amount);
    amount=1;
  }

  obj = read_object(Obj::GENERIC_TALEN, VIRTUAL);
  money = dynamic_cast<TMoney *>(obj);
  mud_assert(money != NULL, "create_money created something that was not TMoney.  obj was: %s", obj ? obj->getName() : "NO OBJECT");

  extraDescription *new_descr;
  sstring buf;

  money->swapToStrung();
  money->setCurrency(currency);

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
    money->name = mud_str_dup(money->getCurrencyName() + " money");
    money->shortDescr = mud_str_dup("a "+money->getCurrencyName());
    money->setDescr(mud_str_dup(format("One miserable %s lies here.") % money->getCurrencyName()));

    new_descr->keyword = mud_str_dup(money->getCurrencyName() + " money");
    new_descr->description = mud_str_dup(format("One miserable %s.\n\r") % money->getCurrencyName());

  } else {
    money->name = mud_str_dup(money->getCurrencyName()+"s money");
    money->shortDescr = mud_str_dup(format("some %ss") % money->getCurrencyName());
    if (amount > 100000)
      buf=format("A tremendously HUGE pile of %ss lies here.") % money->getCurrencyName();
    else if (amount > 50000)
      buf=format("A HUGE pile of %ss lies here.") % money->getCurrencyName();
    else if (amount > 10000)
      buf=format("A LARGE pile of %ss lies here.") % money->getCurrencyName();
    else if (amount > 1000)
      buf=format("A nice-sized pile of %ss lies here.") % money->getCurrencyName();
    else if (amount > 500)
      buf=format("A pile of %ss lies here.") % money->getCurrencyName();
    else if (amount > 100)
      buf=format("A small pile of %ss lies here.") % money->getCurrencyName();
    else if (amount > 50)
      buf=format("A tiny pile of %ss lies here.") % money->getCurrencyName();
    else
      buf=format("A few %ss have been left in a pile here.") % money->getCurrencyName();

    money->setDescr(mud_str_dup(buf));
    new_descr->keyword = mud_str_dup(money->getCurrencyName()+"s money");
    if (amount < 10) {
      buf=format("There are %i %ss.\n\r") % amount % money->getCurrencyName();
      new_descr->description = mud_str_dup(buf);
    } else if (amount < 100) {
      buf=format("There are about %i %ss.\n\r") % (10 * (amount / 10)) % money->getCurrencyName();
      new_descr->description = mud_str_dup(buf);
    } else if (amount < 10000) {
      buf=format("You guess there are %i %ss.\n\r") % (100 * (amount / 100)) % money->getCurrencyName();
      new_descr->description = mud_str_dup(buf);
    } else
      new_descr->description = mud_str_dup(format("There are a LOT of %ss.\n\r") % money->getCurrencyName());
  }
  new_descr->next = NULL;
  money->ex_description = new_descr;

  money->obj_flags.wear_flags = ITEM_TAKE;
  money->obj_flags.decay_time = -1;
  money->setMoney(amount);
  money->obj_flags.cost = amount;

  money->setVolume(max(1, (int)(amount * 0.0048))); // 0.078 cm3
  money->setWeight(amount/303.0); // 1.5 grams

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

void TMoney::setCurrency(currencyTypeT c)
{
  type = c;
}

int TMoney::moneyMeMoney(TBeing *ch, TThing *sub)
{
  int amount;
  TThing *t=NULL;
  char buf[256];
  bool isMyCorpse = false;

  if (sub && isname(ch->name, sub->name) && dynamic_cast<TPCorpse*>(sub))
    isMyCorpse = true;

  (*this)--;
  amount = getMoney();
  if (amount == 1) {
    ch->sendTo(format("There was one %s.\n\r") % getCurrencyName());
  } else {
    ch->sendTo(format("There were %d %ss.\n\r") % amount % getCurrencyName());
    /*int amt2 = 0;
    if (!isMyCorpse && !ch->isImmortal())
      amt2 = (int) (amount * FactionInfo[ch->getFaction()].faction_tithe / 100.0);

    if (!amt2)
      ch->sendTo(format("There were %d talens.\n\r") % amount);
    else {
      ch->sendTo(format("There were %d talens, and you tithe %d of them.\n\r") % amount % amt2);
      // BUGFIX: tithing was creating money 
      amount = amount - amt2;
    }*/
  }

  if (ch->getMoney() > 500000 && (amount > 100000))
    vlogf(LOG_MISC, format("%s just got %d %ss") %  ch->getName() % amount % getCurrencyName());


  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (t=*it);++it) {
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
    ch->addToMoney(amount, GOLD_INCOME, !isMyCorpse && !ch->isImmortal());

    // a queer issue, and mostly a kludge
    // corpse create: removed money from being
    // we want to put the money back onto me
    // but lets also fix gold_positive so that a L50 dying with 4M coins
    // doesn't cause gold_statistic to = 0, but gold_positive = 4M
    if (ch->isPc() && ch->GetMaxLevel() <= 60)
      gold_positive[GOLD_INCOME][ch->GetMaxLevel()-1] -= amount;
  } else
    ch->addToMoney(amount, GOLD_INCOME, !isMyCorpse && !ch->isImmortal());

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
  sprintf(buf2, "%s [%d %ss]", useName ? name : (useColor ? getName() : getNameNOC(ch).c_str()), 
      getMoney(), getCurrencyName().c_str());
  return buf2;
}
