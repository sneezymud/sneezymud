//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_open_container.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// real_container.cc
//

#include "stdsneezy.h"
#include "create.h"

TRealContainer::TRealContainer() :
  TContainer(),
  max_weight(0.0),
  container_flags(0),
  trap_type(0),
  trap_dam(0),
  key_num(-1),
  max_volume(0)
{
}

TRealContainer::TRealContainer(const TRealContainer &a) :
  TContainer(a),
  max_weight(a.max_weight),
  container_flags(a.container_flags),
  trap_type(a.trap_type),
  trap_dam(a.trap_dam),
  key_num(a.key_num),
  max_volume(a.max_volume)
{
}

TRealContainer & TRealContainer::operator=(const TRealContainer &a)
{
  if (this == &a) return *this;
  TContainer::operator=(a);
  max_weight = a.max_weight;
  container_flags = a.container_flags;
  trap_type = a.trap_type;
  trap_dam = a.trap_dam;
  key_num = a.key_num;
  max_volume = a.max_volume;
  return *this;
}

TRealContainer::~TRealContainer()
{
}

void TRealContainer::assignFourValues(int x1, int x2, int x3, int x4)
{
  setCarryWeightLimit((float) x1);
  setContainerFlags(GET_BITS(x2, 7, 8));
  setContainerTrapType(GET_BITS(x2, 15, 8));
  setContainerTrapDam(GET_BITS(x2, 23, 8));
  setKeyNum(x3);
  setCarryVolumeLimit(x4);
}

void TRealContainer::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = (int) carryWeightLimit();

  int r = 0;
  SET_BITS(r, 7, 8, getContainerFlags());
  SET_BITS(r, 15, 8, getContainerTrapType());
  SET_BITS(r, 23, 8, getContainerTrapDam());
  *x2 = r;
  *x3 = getKeyNum();
  *x4 = carryVolumeLimit();
}

string TRealContainer::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Max Weight :%.3f, Max Volume : %d\n\rTrap type :%s (%d), Trap damage :%d\n\r",
          carryWeightLimit(),
          carryVolumeLimit(),
          trap_types[getContainerTrapType()],
          getContainerTrapType(),
          getContainerTrapDam());
  sprintf(buf + strlen(buf), "Vnum of key that opens: %d",
           getKeyNum());

  string a(buf);
  return a;
}

bool TRealContainer::isCloseable() const
{
  return (isContainerFlag(CONT_CLOSEABLE));
}

bool TRealContainer::isClosed() const
{
  return (isContainerFlag(CONT_CLOSED));
}

void TRealContainer::setCarryWeightLimit(float r)
{
  max_weight = r;
}

float TRealContainer::carryWeightLimit() const
{
  return max_weight;
}

unsigned char TRealContainer::getContainerFlags() const
{
  return container_flags;
}

void TRealContainer::setContainerFlags(unsigned char r)
{
  container_flags = r;
}

void TRealContainer::addContainerFlag(unsigned char r)
{
  container_flags |= r;
}

void TRealContainer::remContainerFlag(unsigned char r)
{
  container_flags &= ~r;
}

bool TRealContainer::isContainerFlag(unsigned char r) const
{
  return ((container_flags & r) != 0);
}

unsigned char TRealContainer::getContainerTrapType() const
{
  return trap_type;
}

void TRealContainer::setContainerTrapType(unsigned char r)
{
  trap_type = r;
}

char TRealContainer::getContainerTrapDam() const
{
  return trap_dam;
}

void TRealContainer::setContainerTrapDam(char r)
{
  trap_dam = r;
}

void TRealContainer::setKeyNum(int r)
{
  key_num = r;
}

int TRealContainer::getKeyNum() const
{
  return key_num;
}

int TRealContainer::carryVolumeLimit() const
{
  return max_volume;
}

void TRealContainer::setCarryVolumeLimit(int r)
{
  max_volume = r;
}

void TRealContainer::changeObjValue2(TBeing *ch)
{
  ch->specials.edit = CHANGE_CHEST_VALUE2;
  change_chest_value2(ch, this, "", ENTER_CHECK);
  return;
}

void TRealContainer::describeContains(const TBeing *ch) const
{
  if (stuff && !isClosed())
    ch->sendTo(COLOR_OBJECTS, "%s seems to have something in it...\n\r", good_cap(getName()).c_str());
}

void TRealContainer::lowCheck()
{
  if (carryWeightLimit() <= 0.0) {
    vlogf(LOW_ERROR, "Container (%s) with bad weight limit (%5.2f).",
            getName(), carryWeightLimit());
  }
  if (carryVolumeLimit() <= 0) {
    vlogf(LOW_ERROR, "Container (%s) with bad volume limit (%d).",
            getName(), carryVolumeLimit());
  }

  TContainer::lowCheck();
}

