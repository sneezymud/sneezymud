//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: opal.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD++ 4.5 - All rights reserved, SneezyMUD Coding Team
//      "opal.cc" - Methods for TOpal class
//
//      Last revision December 18, 1997.
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

TOpal::TOpal() :
  TObj(),
  psSize(0),
  psStrength(0),
  psMana(0),
  psFails(0)
{
}

TOpal::TOpal(const TOpal &a) :
  TObj(a),
  psSize(a.psSize),
  psStrength(a.psStrength),
  psMana(a.psMana),
  psFails(a.psFails)
{
}

TOpal & TOpal::operator=(const TOpal &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  psSize = a.psSize;
  psStrength = a.psStrength;
  psMana = a.psMana;
  psFails = a.psFails;
  return *this;
}

TOpal::~TOpal()
{
}

int TOpal::psGetStrength() const
{
  return (psStrength);
}

void TOpal::psSetStrength(int num)
{
  psStrength = num;
}

void TOpal::psAddStrength(int num)
{
  psStrength += num;
}

int TOpal::psGetMana() const
{
  return (psMana);
}

void TOpal::psSetMana(int num)
{
  psMana = num;
}

void TOpal::psAddMana(int num)
{
  psMana += num;
}

int TOpal::psGetConsecFails() const
{
  return (psFails);
}

void TOpal::psSetConsecFails(int num)
{
  psFails = num;
}

void TOpal::psAddConsecFails(int num)
{
  psFails += num;
}

int TOpal::psGetMaxMana() const
{
  return (psStrength * 10);
}

int TOpal::psGetCarats() const
{
  return (psSize);
}

void TOpal::psSetCarats(int num)
{
  psSize = num;
}

void TOpal::assignFourValues(int x1, int x2, int x3, int x4)
{
  psSetCarats(x1);
  psSetStrength(x2);
  psSetMana(x3);
  psSetConsecFails(x4);
}

void TOpal::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = psGetCarats();
  *x2 = psGetStrength();
  *x3 = psGetMana();
  *x4 = psGetConsecFails();
}

int TOpal::objectSell(TBeing *ch, TMonster *keeper)
{
  char buf[256];

  sprintf(buf,"%s I'm sorry, I don't buy back opal powerstones.",ch->getName());
  keeper->doTell(buf);
  return TRUE;
}

string TOpal::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Carats: %d, Strength: %d, Mana Charge: %d, Consecutive Fails: %d",
                psGetCarats(),
                psGetStrength(), psGetMana(), psGetConsecFails());

  string a(buf);
  return a;
}

void TOpal::describeObjectSpecifics(const TBeing *ch) const
{
  double diff = (double) ((double) psGetMana() / (double) psGetMaxMana());
  ch->sendTo(COLOR_OBJECTS,"It appears %s is %s.\n\r",good_uncap(getName()).c_str(),
         ((diff <= 0.0) ? "completely uncharged" :
          ((diff >= 1.0) ? "fully charged" :
          ((diff >= 0.8) ? "almost fully charged" :
          ((diff >= 0.6) ? "mostly charged" :
          ((diff >= 0.4) ? "about half charged" :
          ((diff >= 0.2) ? "partially charged" :
                    "uncharged")))))));
}

TOpal *find_biggest_powerstone(TBeing *ch)
{
  TOpal *stone = NULL;
  TThing *t;
  int i;

  // Check through char's equipment -- only the biggest powerstone charges
  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (!(t = ch->equipment[i]))
      continue;

    t->powerstoneCheck(&stone);
  }
  // Check through char's inventory -- only the biggest powerstone charges
  for (t = ch->stuff; t; t = t->nextThing) 
    t->powerstoneCheck(&stone);
  
  return stone;
}

void TOpal::powerstoneCheckCharged(TOpal **topMax)
{
  if (!*topMax || ((psGetStrength() > (*topMax)->psGetStrength()) && (psGetMana() > 0))) 
    *topMax = this;
}


void TOpal::powerstoneCheck(TOpal **topMax)
{
  if (!*topMax || (psGetStrength() > (*topMax)->psGetStrength())) 
    *topMax = this;
}

void TOpal::powerstoneMostMana(int *topMax)
{
  *topMax = max(*topMax, psGetMana());
}

int TOpal::suggestedPrice() const
{
  // these formulas are strictly for consistency and have not been evaluated
  // with respect to balance
  int str = psGetStrength();
  
  // first term increases based on how charged the powerstone is
  // second term is simply to make larger opals more expensive
  return (80 * str * str * str) + (100 * psGetCarats());
}

void TOpal::lowCheck()
{
  int ap = suggestedPrice();
  if (ap != obj_flags.cost && ap) {
    vlogf(LOW_ERROR, "Opal (%s:%d) has a bad price (%d).  should be (%d)",
         getName(), objVnum(), obj_flags.cost, ap);
    obj_flags.cost = ap;
  }

  TObj::lowCheck();
}
