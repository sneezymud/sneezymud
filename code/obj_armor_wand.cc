//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_armor_wand.cc,v $
// Revision 5.3  2003/03/13 22:40:53  peel
// added sstring class, same as string but takes NULL as an empty string
// replaced all uses of string to sstring
//
// Revision 5.2  2002/01/12 01:53:24  peel
// removed the remaining class definitions from obj2.h
// obj2.h is no more!
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// armor_wand.cc
//
// armorwand is essentially a virtual object class derived from TArmor and
// TWand.  It is possible since TArmor uses no 4-values
//

#include "stdsneezy.h"
#include "obj_armor_wand.h"

TArmorWand::TArmorWand() :
  TArmor(),
  TWand()
{
}

TArmorWand::TArmorWand(const TArmorWand &a) :
  TArmor(a),
  TWand(a)
{
}

TArmorWand & TArmorWand::operator=(const TArmorWand &a)
{
  if (this == &a) return *this;
  TArmor::operator=(a);
  TWand::operator=(a);
  return *this;
}

TArmorWand::~TArmorWand()
{
}

sstring TArmorWand::statObjInfo() const
{
  return TArmor::statObjInfo() + TWand::statObjInfo();
}

void TArmorWand::lowCheck()
{
  TArmor::lowCheck();
  TWand::lowCheck();
}

void TArmorWand::assignFourValues(int x1, int x2, int x3, int x4)
{
  // TArmor does nothing with 4-values, but what the hey
  TArmor::assignFourValues(x1,x2,x3,x4);
  TWand::assignFourValues(x1,x2,x3,x4);
}

void TArmorWand::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TArmor::getFourValues(x1,x2,x3,x4);
  TWand::getFourValues(x1,x2,x3,x4);
}

int TArmorWand::rentCost() const
{
  return TArmor::rentCost() + TWand::rentCost();
}

int TArmorWand::suggestedPrice() const
{
  return TArmor::suggestedPrice() + TWand::suggestedPrice();
}

void TArmorWand::objMenu(const TBeing *ch) const
{
  // it's assumed that TArmor and TWand sort of have same objMenu functionality
  TWand::objMenu(ch);
}

void TArmorWand::evaluateMe(TBeing *ch) const
{
  TArmor::evaluateMe(ch);
  TWand::evaluateMe(ch);
}

void TArmorWand::generalUseMessage(const TBeing *ch, unsigned int bits, const TBeing *tmp_char, const TObj *o) const
{
  if (bits == FIND_CHAR_ROOM) {
    act("$n grabs $s $o and points at you.", 
         TRUE, ch, this, tmp_char, TO_VICT);
    act("$n grabs $s $o and points at $N.", 
         TRUE, ch, this, tmp_char, TO_NOTVICT);
    act("You grab your $o and point at $N.",
         FALSE, ch, this, tmp_char, TO_CHAR);
  } else {
    act("$n grabs $s $o and point at $P.", TRUE, ch, this, o, TO_ROOM);
    act("You grab your $o and point at $P.", FALSE, ch, this, o, TO_CHAR);
  }
}

sstring TArmorWand::getNameForShow(bool useColor, bool useName, const TBeing *ch) const
{
  char buf[256];

  // put both show names together:  name (level) [spell]
  sprintf(buf, "%s [%s]",
      TArmor::getNameForShow(useColor, useName, ch).c_str(),
      (getSpell() >= 0 ? (discArray[getSpell()] ? discArray[getSpell()]->name : "Unknown") : "None"));

  return buf;
}
