#include "monster.h"
#include "extern.h"
#include "obj_open_container.h"
#include "obj_arrow.h"
#include "obj_base_weapon.h"
#include "shop.h"
#include "shopowned.h"
#include "materials.h"

TArrow::TArrow() :
  TBaseWeapon(),
  arrowType(0),
  arrowHead(0),
  arrowHeadMat(0),
  arrowFlags(0),
  trap_level(0),
  trap_dam_type(DOOR_TRAP_NONE)
{
}

TArrow::TArrow(const TArrow &a) :
  TBaseWeapon(a),
  arrowType(a.arrowType),
  arrowHead(a.arrowHead),
  arrowHeadMat(a.arrowHeadMat),
  arrowFlags(a.arrowFlags),
  trap_level(a.trap_level),
  trap_dam_type(a.trap_dam_type)
{
}

TArrow & TArrow::operator=(const TArrow &a)
{
  if (this == &a) return *this;
  TBaseWeapon::operator=(a);
  arrowType    = a.arrowType;
  arrowHead    = a.arrowHead;
  arrowHeadMat = a.arrowHeadMat;
  arrowFlags   = a.arrowFlags;
  trap_level   = a.trap_level;
  trap_dam_type= a.trap_dam_type;
  return *this;
}

TArrow::~TArrow()
{
}

unsigned char TArrow::getArrowType() const
{
  return arrowType;
}

unsigned char TArrow::getArrowHead() const
{
  return arrowHead;
}

unsigned short TArrow::getArrowFlags() const
{
  return arrowFlags;
}

unsigned char TArrow::getArrowHeadMat() const
{
  return arrowHeadMat;
}

void TArrow::remArrowFlags(unsigned short n)
{
  arrowFlags &= ~n;
}

bool TArrow::isArrowFlag(unsigned short n)
{
  return arrowFlags & n;
}

void TArrow::addArrowFlags(unsigned short n)
{
  arrowFlags |= n;
}

void TArrow::setArrowType(unsigned int newArrowType)
{
  arrowType = newArrowType;
}

void TArrow::setArrowHead(unsigned char newArrowHead)
{
  arrowHead = newArrowHead;
}

void TArrow::setArrowFlags(unsigned short newArrowFlags)
{
  arrowFlags = newArrowFlags;
}

void TArrow::setArrowHeadMat(unsigned char newArrowHeadMat)
{
  arrowHeadMat = newArrowHeadMat;
}

int TArrow::getTrapDamAmount() const
{
  return dice(getTrapLevel(), 8);
}

int TArrow::getTrapLevel() const
{
  return trap_level;
}

void TArrow::setTrapLevel(int r)
{
  trap_level = r;
}

doorTrapT TArrow::getTrapDamType() const
{
  return trap_dam_type;
}

void TArrow::setTrapDamType(doorTrapT r)
{
  trap_dam_type = r;
}



bool TArrow::sellMeCheck(TBeing *ch, TMonster *keeper, int, int) const
{
  return TBaseWeapon::sellMeCheck(ch, keeper, 1, 50);
}

void TArrow::assignFourValues(int x1, int x2, int x3, int x4)
{
  TBaseWeapon::assignFourValues(x1, x2, x3, x4);

  setArrowType   (GET_BITS(x4,  3,  4));
  setArrowHead   (GET_BITS(x4,  7,  4));
  setArrowHeadMat(GET_BITS(x4, 16,  9));
  setArrowFlags  (GET_BITS(x4, 31, 15));
  setTrapLevel   (GET_BITS(x3, 15,  16));
  setTrapDamType ((doorTrapT)GET_BITS(x3, 31, 16));
}

void TArrow::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TBaseWeapon::getFourValues(x1, x2, x3, x4);

  int tValue = *x4;
  SET_BITS(tValue,  3,  4, getArrowType());
  SET_BITS(tValue,  7,  4, getArrowHead());
  SET_BITS(tValue, 16,  9, getArrowHeadMat());
  SET_BITS(tValue, 31, 15, getArrowFlags());
  *x4 = tValue;

  int t2Value = *x3;
  SET_BITS(t2Value,  15, 16, getTrapLevel());
  SET_BITS(t2Value,  31, 16, getTrapDamType());
  *x3 = t2Value;
}

sstring TArrow::displayFourValues()
{
  char tString[256];
  int  x1,
       x2,
       x3,
       x4;

  getFourValues(&x1, &x2, &x3, &x4);
  sprintf(tString, "Current values : %d %d %d %d\n\r", x1, x2, x3, x4);
  sprintf(tString + strlen(tString),
          "Size: %d Arrow-Head: %d Head-Material: %d Flags: %d Trap-Type: %d Trap-Level %d\n\r",
          getArrowType(), getArrowHead(), getArrowHeadMat(), getArrowFlags(), getTrapDamType(), getTrapLevel());

  return tString;
}

sstring TArrow::statObjInfo() const
{
  char buf[256];
  sstring a;

  sprintf(buf, "Damage Level: %.4f, Damage Deviation: %d\n\r",
           getWeapDamLvl() / 4.0,
           getWeapDamDev());
  a += buf;
  
  sprintf(buf, "Damage inflicted: %d\n\r",
           (int) damageLevel());
  a += buf;
  
  sprintf(buf, "Sharpness   : %d\n\r",
          getCurSharp());
  a += buf;

  sprintf(buf, "Arrow Type  : %d\n\r",
          getArrowType());
  a += buf;

  sprintf(buf, "Arrow Head  : %d",
          getArrowHead());
  a += buf;

  sprintf(buf, "Head Mat   : %d",
          getArrowHeadMat());
  a += buf;

  sprintf(buf, "Arrow Flags: %d",
          getArrowFlags());
  a += buf;

  if(getTrapDamType()!=DOOR_TRAP_NONE){
    sprintf(buf, "Trap Type: %s (%d)", 
	    trap_types[getTrapDamType()].c_str(), getTrapDamType());
    a += buf;
    
    sprintf(buf, "Trap Level: %i", 
	    getTrapLevel());
    a += buf;
  }

  return a;
}

int TArrow::putMeInto(TBeing *, TOpenContainer *)
{
  return FALSE;
}

bool TArrow::engraveMe(TBeing *ch, TMonster *me, bool give)
{
  char buf[256];

  me->doTell(ch->getName(), "Engraving this would destroy its aerodynamics.");

  if (give) {
    strcpy(buf, name);
    strcpy(buf, add_bars(buf).c_str());
    sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
    me->doGive(buf);
  }

  return TRUE;
}

int TArrow::throwMe(TBeing *ch, dirTypeT, const char *)
{
  act("$p isn't designed to be thrown.", FALSE, ch, this, 0, TO_CHAR);
  return FALSE;
}

spellNumT TArrow::getWtype(int which) const
{
  if(objVnum() == 31864 || objVnum() == 31869)
    return TYPE_SHOOT;
  else if(objVnum() == 19090)
    return TYPE_CANNON;
  else
    return TYPE_PIERCE;
}

void TArrow::evaluateMe(TBeing *ch) const
{
  int learn, traptype;

  learn = ch->getSkillValue(SKILL_EVALUATE);
  traptype = getTrapDamType();
  

  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_EVALUATE, 10);

  ch->describeNoise(this, learn);

  if (learn > 5)
    ch->describeMaxSharpness(this, learn);

  if (learn > 10)
    switch (arrowType) {
      case 0:
        ch->sendTo(COLOR_OBJECTS, format("%s is a hunting type arrow.\n\r") % getName());
        break;
      case 1:
        ch->sendTo(COLOR_OBJECTS, format("%s is a fighting type arrow.\n\r") % getName());
        break;
      case 2:
        ch->sendTo(COLOR_OBJECTS, format("%s is a squabble type quarrel.\n\r") % getName());
        break;
      case 3:
        ch->sendTo(COLOR_OBJECTS, format("%s is a common type quarrel.\n\r") % getName());
        break;
      case 4:
        ch->sendTo(COLOR_OBJECTS, format("%s is a sniper type blowdart.\n\r") % getName());
        break;
      case 5:
        ch->sendTo(COLOR_OBJECTS, format("%s is a common type blowdart.\n\r") % getName());
        break;
      case 6:
        ch->sendTo(COLOR_OBJECTS, format("%s is a heavy type sling ammo.\n\r") % getName());
        break;
      case 7:
        ch->sendTo(COLOR_OBJECTS, format("%s is a common type sling ammo.\n\r") % getName());
        break;
      default:
        ch->sendTo(format("%s is a messed up arrow type.\n\r") % getName());
        break;
    }

  if (learn > 15)
    ch->describeArrowSharpness(this, learn);

  if (learn > 20)
    ch->describeMaxStructure(this, learn);

  if (learn > 30) {
    ch->describeArrowDamage(this, learn);
    if(traptype != DOOR_TRAP_NONE)
      ch->sendTo(COLOR_OBJECTS, format("%s is trapped with %s.\n\r") % getName() % trap_types[traptype]);
  }
  if (learn > 35)
    ch->describeWeaponDamage(this, learn);
}

void TArrow::bloadBowArrow(TBeing *ch, TThing *bow)
{
  bow->bloadArrowBow(ch, this);
}

int TArrow::suggestedPrice() const
{
  // don't divide the TObj cost, so subtract it, then divide, then re-add
  int amt = TBaseWeapon::suggestedPrice()-(int)(10.0 * getWeight() * material_nums[getMaterial()].price);
  return (amt / 10) + (int)(10.0 * getWeight() * material_nums[getMaterial()].price);
}

void TArrow::changeObjValue3(TBeing *ch)
{
  ch->specials.edit = CHANGE_ARROW_VALUE3;
  change_arrow_value3(ch, this, "", ENTER_CHECK);
}

void TArrow::changeObjValue4(TBeing *ch)
{
  ch->specials.edit = CHANGE_ARROW_VALUE4;
  change_arrow_value4(ch, this, "", ENTER_CHECK);
}
