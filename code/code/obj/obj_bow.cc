#include "extern.h"
#include "range.h"
#include "colorstring.h"
#include "obj_bow.h"
#include "obj_arrow.h"
#include "shop.h"
#include "shopowned.h"
#include "being.h"

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
  if (!stuff.empty())
    ch->sendTo(COLOR_OBJECTS, format("%s is loaded with an arrow.\n\r") % sstring(getName()).cap());
  else
    ch->sendTo(COLOR_OBJECTS, format("%s has no arrow ready.\n\r") % sstring(getName()).cap());

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

sstring TBow::statObjInfo() const
{
  char    buf[256];
  TArrow *tArrow;

  tArrow = dynamic_cast<TArrow *>(stuff.front());

  sprintf(buf, "Arrow: %d, flags: %d, type: %d", (tArrow ? tArrow->objVnum() : 0), getBowFlags(), getArrowType());

  sstring a(buf);
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


  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_EVALUATE, 10);

  ch->describeNoise(this, learn);

  if (learn > 10)
    switch (arrowType) {
      case 0:
        ch->sendTo(COLOR_OBJECTS, format("%s can hold hunting type arrows.\n\r") % getName());
        break;
      case 1:
        ch->sendTo(COLOR_OBJECTS, format("%s can hold fighting type arrows.\n\r") % getName());
        break;
      case 2:
        ch->sendTo(COLOR_OBJECTS, format("%s can hold squabble type quarrels.\n\r") % getName());
        break;
      case 3:
        ch->sendTo(COLOR_OBJECTS, format("%s can hold common type quarrels.\n\r") % getName());
        break;
      case 4:
        ch->sendTo(COLOR_OBJECTS, format("%s can hold sniper type blowdarts.\n\r") % getName());
        break;
      case 5:
        ch->sendTo(COLOR_OBJECTS, format("%s can hold common type blowdarts.\n\r") % getName());
        break;
      case 6:
        ch->sendTo(COLOR_OBJECTS, format("%s can hold heavy type sling ammo.\n\r") % getName());
        break;
      case 7:
        ch->sendTo(COLOR_OBJECTS, format("%s can hold common type sling ammo.\n\r") % getName());
        break;
      default:
        ch->sendTo(COLOR_OBJECTS, format("%s seems to have a messy arrow size.\r") % getName());
        break;
    }

  if (learn > 20)
    ch->describeMaxStructure(this, learn);

  if (learn > 25)
    ch->describeBowRange(this, learn);
}

void TBow::sstringMeBow(TBeing *ch, TThing *sstring)
{
  sstring->sstringMeString(ch, this);
}

bool TBow::sellMeCheck(TBeing *ch, TMonster *keeper, int, int) const
{
  return TObj::sellMeCheck(ch, keeper, 1, 10);
}

void TBow::bloadArrowBow(TBeing *ch, TArrow *the_arrow)
{
  if (isBowFlag(BOW_STRING_BROKE)) {
    ch->sendTo("The bowsstring on your bow has snapped.  It needs repair before use.\n\r");
    return;
  }

  if (!stuff.empty()) {
    ch->sendTo("That bow has already been loaded, so you hold it.\n\r");
    if (!equippedBy)
    {
      --(*this);
      ch->equipChar(this, ch->getPrimaryHold());
      ch->addToWait(combatRound(1));
    }
    return;
  }

  if (the_arrow->objVnum() <= 0) {
    ch->sendTo("A strange force prevents you from loading that arrow into the bow.\n\r");
    return;
  }

  if (getArrowType() != the_arrow->getArrowType()) {
    ch->sendTo(format("That arrow is just too %s to fit properly.\n\r") %
               ((getArrowType() < the_arrow->getArrowType()) ? "small" : "big"));

    return;
  }

  --(*the_arrow);
  *this += *the_arrow;
  act("You load $p into $N and hold it.", TRUE, ch, the_arrow, this, TO_CHAR);
  act("$n loads $p into $N and holds it.", TRUE, ch, the_arrow, this, TO_ROOM);

  --(*this);
  ch->equipChar(this, ch->getPrimaryHold());

  ch->addToWait(combatRound(1.5));
}

int TBow::shootMeBow(TBeing *ch, TBeing *targ, unsigned int count, dirTypeT dir, int shoot_dist)
{
  TObj *the_arrow;
  unsigned int   max_distance;
  int   str_test, rc;
  char  buf[256];

  if (stuff.empty() || !dynamic_cast<TArrow *>(stuff.front())) {
    act("$p isn't loaded with an arrow!", FALSE, ch, this, 0, TO_CHAR);

    if (!stuff.empty() && !dynamic_cast<TArrow *>(stuff.front())) {
      vlogf(LOG_BUG, format("Bow loaded with something not an arrow. [%s]") %  ch->getName());
      TThing *tThing = stuff.front();
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
    ch->sendTo(format("You couldn't possibly shoot it further than %d rooms.\n\r") % max_distance);
    return FALSE;
  }

  if (targ &&
      ch->checkPeacefulVictim("They are in a peaceful room. You can't seem to fire the bow.\n\r", targ))
    return FALSE;

  if (targ && ch->noHarmCheck(targ))
    return FALSE;

  // treat fliers as being 1 room further away
  if (targ && targ->isFlying() && !ch->isFlying() && ((count+1) > max_distance)){
    act("Unfortunately, $N is flying and you can't quite reach that far.",
       FALSE, ch, 0, targ, TO_CHAR);
    return FALSE;
  }

  the_arrow = dynamic_cast<TObj *>(stuff.front());

  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_RANGED_PROF, 2);
  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_RANGED_SPEC, 40);

  if (ch->getSkillValue(SKILL_RANGED_PROF) < 10) {
    ch->sendTo("You can't even get the arrow out of the bow properly!\n\r");
    ch->sendTo("You realize you don't know much about shooting...get more training.\n\r");
    act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_CHAR);
    act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_ROOM);
    --(*the_arrow);
    *ch->roomp += *the_arrow;
    return FALSE;
  } else if (ch->getSkillValue(SKILL_RANGED_PROF) < (::number(0, 30))) {
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
      act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_CHAR);
      act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_ROOM);
      --(*the_arrow);
      *ch->roomp += *the_arrow;
      if (!makeScraps())
        return DELETE_THIS;
      return FALSE;
    } else {
      ch->sendTo("Your bowstring snaps! It will need repair before further use!\n\r");
      addToStructPoints(-1);
      addBowFlags(BOW_STRING_BROKE);

      act("You hear a loud pop as $n's bowsstring snaps!", 
                 FALSE, ch, NULL, NULL, TO_ROOM);
      act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_CHAR);
      act("$p falls to the $g harmlessly.", FALSE, ch, the_arrow, NULL, TO_ROOM);
      --(*the_arrow);
      *ch->roomp += *the_arrow;
      return FALSE;
    }
  }

  // determine how many arrows we can shoot in a round
  float nattacks=1.0;
  nattacks += max((double) 0, ((float)ch->getSkillValue(SKILL_FAST_LOAD)/100.0));
  nattacks += max((double) 0, ((float)ch->getSkillValue(SKILL_RANGED_SPEC)/100.0));

  // for learning - ranged spec is learned elsewhere
  if(ch->doesKnowSkill(SKILL_FAST_LOAD))
    ch->bSuccess(SKILL_FAST_LOAD);
  while(nattacks > 0 && targ){
    // use remainder as a percentage chance of another arrow
    if(nattacks < 1.0 && ::number(0,99) > (nattacks*100))
      break;
    else
      --nattacks;

    --(*the_arrow);
    sstring capbuf = colorString(ch, ch->desc, the_arrow->getName(), NULL, COLOR_OBJECTS, TRUE);
    sstring capbuf2 = colorString(ch, ch->desc, getName(), NULL, COLOR_OBJECTS, TRUE);
    
    if (targ)
      ch->sendTo(COLOR_MOBS, format("You shoot %s out of %s at %s.\n\r") %
		 capbuf.uncap() % capbuf2.uncap() %
		 targ->getName());
    else
      ch->sendTo(format("You shoot %s out of %s.\n\r") %
		 capbuf.uncap() % 
		 capbuf2.uncap());
    
    sprintf(buf, "$n points $p %swards, and shoots $N out of it.",
	    dirs[dir]);
    act(buf, FALSE, ch, this, the_arrow, TO_ROOM);
    
    *ch->roomp += *the_arrow;
    
    // construct reload buf, do it here since arrow might go bye-bye
    // as sanity check, verify that person has an arrow to reload at this
    // point too.
    sprintf(buf, "%s", fname(the_arrow->name).c_str());

    rc = throwThing(the_arrow, dir, ch->in_room, &targ, shoot_dist, max_distance, ch);
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
      delete the_arrow;
      the_arrow = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete targ;
      targ = NULL;
    }


    rc = ch->doRemove("", this);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_THIS;

    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;
    
    // stop if we were unable to remove the bow for some reason
    if (!rc)
      break;

    the_arrow = dynamic_cast<TObj *>(ch->findArrow(buf, SILENT_NO));
    if (the_arrow)
      the_arrow->bloadBowArrow(ch, this);
    else {
      ch->sendTo(format("You seem to have run out of '%s's.\n\r") % buf);
      break;
    }
  }

  // firing multiple arrows above stacks up the combat lag
  // but we don't want that in this case, so use setWait()
  // to just set it appropriately
  ch->setWait(combatRound(4));

  return FALSE;
}

sstring TBow::showModifier(showModeT mode, const TBeing *ch) const
{
  sstring a;

  if (!stuff.empty())
    a = " (loaded)";
  else
    a = " (unloaded)";

  if (mode == SHOW_MODE_SHORT_PLUS ||
       mode == SHOW_MODE_SHORT_PLUS_INV ||
       mode == SHOW_MODE_SHORT) {
    a = " (";
    a += equip_condition(-1);
    a += ")";
    if (ch->hasWizPower(POWER_IMM_EVAL) || toggleInfo[TOG_TESTCODE2]->toggle) {
      char buf[256];
      sprintf(buf, " (L%d)", (int) (objLevel() + 0.5));
      a += buf;
    }
  }
  return a;
}

void TBow::dropMe(TBeing *ch, showMeT showme, showRoomT showroom)
{
  // continue recursion, for text display
  TObj::dropMe(ch, showme, showroom);

  // dislodge arrow
  // the text of the drop (above) should be before the dislodge
  if (!stuff.empty()) {
    TThing *t = stuff.front();
    (*t)--;
    *ch->roomp += *t;
    act("$n falls from $p.", false, t, this, 0, TO_ROOM);
  }
}
