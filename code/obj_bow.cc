//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_bow.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.2  1999/09/27 10:27:34  lapsos
// message change in fast load.
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "range.h"
#include "create.h"

TBow::TBow() :
  TObj(),
  arrowType(0),
  flags(0),
  max_range(0)
{
}

TBow::TBow(const TBow &a) :
  TObj(a),
  arrowType(a.arrowType),
  flags(a.flags),
  max_range(a.max_range)
{
}

TBow & TBow::operator=(const TBow &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  arrowType = a.arrowType;
  flags = a.flags;
  max_range = a.max_range;
  return *this;
}

TBow::~TBow()
{
}

int TBow::getArrowType() const
{
  return arrowType;
}

void TBow::setArrowType(int newArrowType)
{
  arrowType = newArrowType;
}

unsigned int TBow::getBowFlags() const
{
  return flags;
}

void TBow::setBowFlags(unsigned int r)
{
  flags = r;
}

bool TBow::isBowFlag(unsigned int r) const
{
  return ((flags & r) != 0);
}

void TBow::addBowFlags(unsigned int r)
{
  flags |= r;
}

void TBow::remBowFlags(unsigned int r)
{
  flags &= ~r;
}

void TBow::describeObjectSpecifics(const TBeing *ch) const
{
  if (stuff)
    ch->sendTo(COLOR_OBJECTS, "%s is loaded with an arrow.\n\r", good_cap(getName()).c_str());
  else
    ch->sendTo(COLOR_OBJECTS, "%s has no arrow ready.\n\r", good_cap(getName()).c_str());

  if (isBowFlag(BOW_STRING_BROKE))
    act("$p has a broken string.", false, ch, this, 0, TO_CHAR);
  if (isBowFlag(BOW_CARVED))
    act("$p is carved.", false, ch, this, 0, TO_CHAR);
  if (isBowFlag(BOW_SCRAPED))
    act("$p is scraped.", false, ch, this, 0, TO_CHAR);
  if (isBowFlag(BOW_SMOOTHED))
    act("$p is smoothed.", false, ch, this, 0, TO_CHAR);
}

void TBow::assignFourValues(int, int x2, int x3, int x4)
{
  setBowFlags(x2);
  setArrowType(x3);
  setMaxRange(x4);
}

void TBow::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = getBowFlags();
  *x3 = getArrowType();
  *x4 = getMaxRange();
}

string TBow::statObjInfo() const
{
  char    buf[256];
  TArrow *tArrow;

  tArrow = dynamic_cast<TArrow *>(stuff);

  sprintf(buf, "Arrow: %d, flags: %d, type: %d", (tArrow ? tArrow->objVnum() : 0), getBowFlags(), getArrowType());

  string a(buf);
  return a;
}

bool TBow::isBluntWeapon() const
{
  return true;
}

unsigned int TBow::getMaxRange() const
{
  return max_range;
}

void TBow::setMaxRange(unsigned int r)
{
  max_range = r;
}

void TBow::evaluateMe(TBeing *ch) const
{
  int learn = ch->getSkillValue(SKILL_EVALUATE);
  if (learn <= 0) {
    ch->sendTo("You are not sufficiently knowledgeable about evaluation.\n\r");
    return;
  }

  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_EVALUATE, 10);

  ch->sendTo(COLOR_OBJECTS, "You evaluate %s for its battle-worthiness...\n\r\n\r",
             getName());
  ch->describeObject(this);
  ch->describeNoise(this, learn);

  if (learn > 20)
    ch->describeMaxStructure(this, learn);

  if (learn > 25)
    ch->describeBowRange(this, learn);
}

void TBow::stringMeBow(TBeing *ch, TThing *string)
{
  string->stringMeString(ch, this);
}

bool TBow::sellMeCheck(const TBeing *ch, TMonster *keeper) const
{
  int total = 0;
  TThing *t;
  char buf[256];

  for (t = keeper->stuff; t; t = t->nextThing) {
    if ((t->number == number) &&
        (t->getName() && getName() &&
         !strcmp(t->getName(), getName()))) {
      total += 1;
      if (total > 9) {
        sprintf(buf, "%s I already have plenty of those.", ch->name);
        keeper->doTell(buf);
        return TRUE;
      }
    }
  }
  return FALSE;
}

void TBow::bloadArrowBow(TBeing *ch, TArrow *the_arrow)
{
  if (isBowFlag(BOW_STRING_BROKE)) {
    ch->sendTo("The bowstring on your bow has snapped.  It needs repair before use.\n\r");
    return;
  }

  if (stuff) {
    ch->sendTo("That bow has already been loaded, so you hold it.\n\r");
    //--(*this);
    //ch->equipChar(this, ch->getPrimaryHold());
    return;
  }

  if (the_arrow->objVnum() <= 0) {
    ch->sendTo("A strange force prevents you from loading that arrow into the bow.\n\r");
    return;
  }

  if (getArrowType() != the_arrow->getArrowType()) {
    ch->sendTo("That arrow is just too %s to fit properly.\n\r",
               ((getArrowType() > the_arrow->getArrowType()) ? "small" : "big"));

    return;
  }

  --(*the_arrow);
  *this += *the_arrow;
  act("You load $p into $N and hold it.", TRUE, ch, the_arrow, this, TO_CHAR);
  act("$n loads $p into $N and holds it.", TRUE, ch, the_arrow, this, TO_ROOM);

  --(*this);
  ch->equipChar(this, ch->getPrimaryHold());

  ch->addToWait(combatRound(1));
}

int TBow::shootMeBow(TBeing *ch, TBeing *targ, unsigned int count, dirTypeT dir, int shoot_dist)
{
  TObj *the_arrow;
  unsigned int   max_distance;
  int   str_test, rc;
  char  buf[256];

  if (!stuff || !dynamic_cast<TArrow *>(stuff)) {
    act("$p isn't loaded with an arrow!", FALSE, ch, this, 0, TO_CHAR);

    if (stuff && !dynamic_cast<TArrow *>(stuff)) {
      vlogf(7, "Bow loaded with something not an arrow. [%s]", ch->getName());
      TThing *tThing = stuff;
      --(*tThing);
      delete tThing;
      tThing = NULL;
    }
    return FALSE;
  }
  // use the bow's value for the furthest we will go
  max_distance = getMaxRange();

  if ((shoot_dist > 50) || (max_distance < (unsigned int) shoot_dist)) {
    ch->sendTo("Much too far.  Maybe in your dreams!\n\r");
    ch->sendTo("You couldn't possibly shoot it further than %d rooms.\n\r", max_distance);
    return FALSE;
  }

  if (targ &&
      ch->checkPeacefulVictim("They are in a peaceful room. You can't seem to fire the bow.\n\r", targ))
    return FALSE;

  if (targ && ch->noHarmCheck(targ))
    return FALSE;

  // treat fliers as being 1 room further away
  if (targ && targ->isFlying() && ((count+1) > max_distance)) {
    act("Unfortunately, $N is flying and you can't quite reach that far.",
       FALSE, ch, 0, targ, TO_CHAR);
    return FALSE;
  }

  the_arrow = dynamic_cast<TObj *>(stuff);

  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_BOW, 2);
  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_RANGED_SPEC, 40);

  if (ch->getSkillValue(SKILL_BOW) < 10) {
    ch->sendTo("You can't even get the arrow out of the bow properly!\n\r");
    ch->sendTo("You realize you don't know much about shooting...get more training.\n\r");
    act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_CHAR);
    act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_ROOM);
    --(*the_arrow);
    *ch->roomp += *the_arrow;
    return FALSE;
  } else if (ch->getSkillValue(SKILL_BOW) < (::number(0, 30))) {
    ch->sendTo("You have a hard time getting the arrow out of the bow properly!\n\r");
    ch->sendTo("You realize you should get more training and practice more.\n\r");
    act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_CHAR);
    act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_ROOM);
    --(*the_arrow);
    *ch->roomp += *the_arrow;
    return FALSE;
  }
  if (!(str_test = ::number(0, getStructPoints()))) {
    if (!::number(0, getStructPoints()) &&
        ch->roomp && !ch->roomp->isRoomFlag(ROOM_ARENA)) {
      ch->sendTo("As you try to shoot, your bow is shattered!\n\r");
      makeScraps();
      act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_CHAR);
      act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_ROOM);
      --(*the_arrow);
      *ch->roomp += *the_arrow;
      return DELETE_THIS;
    } else {
      ch->sendTo("Your bowstring snaps! It will need repair before further use!\n\r");
      addToStructPoints(-1);
      addBowFlags(BOW_STRING_BROKE);

      act("You hear a loud pop as $n's bowstring snaps!", 
                 FALSE, ch, NULL, NULL, TO_ROOM);
      act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_CHAR);
      act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_ROOM);
      --(*the_arrow);
      *ch->roomp += *the_arrow;
      return FALSE;
    }
  }
  --(*the_arrow);
  string capbuf = colorString(ch, ch->desc, the_arrow->getName(), NULL, COLOR_OBJECTS, TRUE);
  string capbuf2 = colorString(ch, ch->desc, getName(), NULL, COLOR_OBJECTS, TRUE);

  if (targ)
    ch->sendTo(COLOR_MOBS, "You shoot %s out of %s at %s.\n\r",
          good_uncap(capbuf).c_str(), good_uncap(capbuf2).c_str(),
          targ->getName());
  else
    ch->sendTo("You shoot %s out of %s.\n\r",
          good_uncap(capbuf).c_str(), 
          good_uncap(capbuf2).c_str());

  sprintf(buf, "$n points $p %swards, and shoots $N out of it.",
             dirs[dir]);
  act(buf, FALSE, ch, this, the_arrow, TO_ROOM);

  *ch->roomp += *the_arrow;

  ch->addToWait(combatRound(1));

  // construct reload buf, do it here since arrow might go bye-bye
  sprintf(buf, "%s ", fname(name).c_str());
  strcat(buf, fname(the_arrow->name).c_str());

  rc = throwThing(the_arrow, dir, ch->in_room, &targ, shoot_dist, max_distance, ch);
  if (IS_SET_DELETE(rc, DELETE_ITEM)) {
    delete the_arrow;
    the_arrow = NULL;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete targ;
    targ = NULL;
  }

  if (ch->doesKnowSkill(SKILL_FAST_LOAD)) {
    ch->sendTo("You quickly try to reload.\n\r");

    rc = ch->doRemove("", this);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_THIS;

    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;

    if (bSuccess(ch, ch->getSkillValue(SKILL_FAST_LOAD), SKILL_FAST_LOAD)) {
      ch->doBload(buf);
    } else {
      ch->sendTo("You fumble your %s which hampers your reload attempt.\n\r", buf);
      //      ch->sendTo("Oops!  You miscock your %s, and screw up your rhythm.\n\r", buf);
    }
  }
  return FALSE;
}

string TBow::showModifier(showModeT, const TBeing *) const
{
  string a;

  if (stuff)
    a = " (loaded)";
  else
    a = " (unloaded)";

  return a;
}

void TBow::dropMe(TBeing *ch, showMeT showme, showRoomT showroom)
{
  // continue recursion, for text display
  TObj::dropMe(ch, showme, showroom);

  // dislodge arrow
  // the text of the drop (above) should be before the dislodge
  if (stuff) {
    TThing *t = stuff;
    (*t)--;
    *ch->roomp += *t;
    act("$n falls from $p.", false, t, this, 0, TO_ROOM);
  }
}
