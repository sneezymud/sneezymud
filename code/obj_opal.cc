///////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD++ 4.5 - All rights reserved, SneezyMUD Coding Team
//      "opal.cc" - Methods for TOpal class
//
//      Last revision December 18, 1997.
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_opal.h"

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
  psSetConsecFails(x4);
}

void TOpal::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = psGetCarats();
  *x2 = psGetStrength();
  *x3 = -1;
  *x4 = psGetConsecFails();
}

int TOpal::objectSell(TBeing *ch, TMonster *keeper)
{
  keeper->doTell(ch->getName(), "I'm sorry, I don't buy back opal powerstones.");
  return TRUE;
}

sstring TOpal::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Carats: %d, Strength: %d, Consecutive Fails: %d",
                psGetCarats(),
                psGetStrength(),psGetConsecFails());

  sstring a(buf);
  return a;
}

void TOpal::describeObjectSpecifics(const TBeing *ch) const
{
  if(psGetStrength() < psGetCarats())
    ch->sendTo(COLOR_OBJECTS, "Is is not at full strength.\n\r");
  else
    ch->sendTo(COLOR_OBJECTS, "Is is at full strength.\n\r");
}

TOpal *find_biggest_powerstone(const TBeing *ch)
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
  for (t = ch->getStuff(); t; t = t->nextThing) 
    t->powerstoneCheck(&stone);
  
  return stone;
}

void TOpal::powerstoneCheck(TOpal **topMax)
{
  if (!*topMax || (psGetStrength() > (*topMax)->psGetStrength())) 
    *topMax = this;
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
    vlogf(LOG_LOW, fmt("Opal (%s:%d) has a bad price (%d).  should be (%d)") % 
         getName() % objVnum() % obj_flags.cost % ap);
    obj_flags.cost = ap;
  }

  TObj::lowCheck();
}

