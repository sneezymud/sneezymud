//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_quiver.cc,v $
// Revision 5.3  2003/03/13 22:40:54  peel
// added sstring class, same as string but takes NULL as an empty string
// replaced all uses of string to sstring
//
// Revision 5.2  2002/01/08 21:05:12  peel
// removed the TBaseContainer hierarchy from obj2.h
// added header files for those objects
// inserted appropriate includes
//
// Revision 5.1  2001/07/13 05:32:20  peel
// renamed a bunch of source files
//
// Revision 5.1.1.2  2000/01/23 17:02:18  batopr
// fixed misspelling
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


// quiver.cc

#include "stdsneezy.h"
#include "disc_looting.h"
#include "obj_quiver.h"

TQuiver::TQuiver() :
  TExpandableContainer()
{
}

TQuiver::TQuiver(const TQuiver &a) :
  TExpandableContainer(a)
{
}

TQuiver & TQuiver::operator=(const TQuiver &a)
{
  if (this == &a) return *this;
  TExpandableContainer::operator=(a);
  return *this;
}

TQuiver::~TQuiver()
{
}

void TQuiver::assignFourValues(int x1, int x2, int x3, int x4)
{
  TExpandableContainer::assignFourValues(x1, x2, x3, x4);
}

void TQuiver::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TExpandableContainer::getFourValues(x1, x2, x3, x4);
}

sstring TQuiver::statObjInfo() const
{
  return TExpandableContainer::statObjInfo();
}

bool TQuiver::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    char buf[256];
    sprintf(buf, "%s I can't repair quivers.", fname(ch->name).c_str());
    repair->doTell(buf);
  }
  return true;
}

void TQuiver::closeMe(TBeing *ch)
{
  if (isClosed())
    ch->sendTo("It is already closed.\n\r");
  else if (!isCloseable())
    ch->sendTo("I'm afraid it cannot be closed.\n\r");
  else {
    addContainerFlag(CONT_CLOSED);
    act("You close and strap $p.",
        TRUE, ch, this, 0, TO_CHAR);
    act("$n closes and straps $p.",
        TRUE, ch, this, 0, TO_ROOM);
  }
}

int TQuiver::openMe(TBeing *ch)
{
  char buf[256];

  if (!isClosed()) {
    ch->sendTo("But it's already open!\n\r");
    return FALSE;
  } else if (!isCloseable()) {
    ch->sendTo("You can't do that.\n\r");
    return FALSE;
  } else if (isContainerFlag(CONT_TRAPPED)) {
    if (ch->doesKnowSkill(SKILL_DETECT_TRAP)) {
      if (detectTrapObj(ch, this)) {
        sprintf(buf, "You start to open $p, but then notice an insidious %s trap...",
              good_uncap(trap_types[getContainerTrapType()]).c_str());
        act(buf, TRUE, ch, this, NULL, TO_CHAR);
        return FALSE;
      }
    }
    act("You release the strap on $p and open it.",
        TRUE, ch, this, NULL, TO_CHAR);
    act("$n releases the strap on $p and opens it.",
        TRUE, ch, this, 0, TO_ROOM);
    remContainerFlag(CONT_CLOSED);

    int rc = ch->triggerContTrap(this);
    int res = 0;
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
      ADD_DELETE(res, DELETE_THIS);
    }
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      ADD_DELETE(res, DELETE_VICT);
    }
    return res;
  } else {
    remContainerFlag(CONT_CLOSED);
    act("You release the strap on $p and open it.",
        TRUE, ch, this, NULL, TO_CHAR);
    act("$n releases the strap on $p and opens it.",
        TRUE, ch, this, 0, TO_ROOM);
    return TRUE;
  }
}

void TQuiver::lockMe(TBeing *ch)
{
  ch->sendTo("I'm afraid that cannot be locked.\n\r");
}

void TQuiver::unlockMe(TBeing *ch)
{
  ch->sendTo("That cannot be locked thus it cannot be unlocked.\n\r");
}
