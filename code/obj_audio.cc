//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// audio.cc

#include "stdsneezy.h"
#include "obj_audio.h"

int TAudio::getFreq() const
{
  return freq;
}

void TAudio::setFreq(int n)
{
  freq = n;
}

TAudio::TAudio() :
  TObj(), 
  freq(0)
{
}

TAudio::TAudio(const TAudio &a) :
  TObj(a),
  freq(a.freq)
{
}

TAudio & TAudio::operator=(const TAudio &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  freq = a.freq;
  return *this;
}

TAudio::~TAudio()
{
}

void TAudio::assignFourValues(int x1, int, int, int)
{
  setFreq(x1);
}

void TAudio::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getFreq();
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TAudio::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Frequency: %d", getFreq());

  sstring a(buf);
  return a;
}

void TAudio::audioCheck(int pulse) const
{
  int room = 0;

  if ((getFreq() && !(pulse % getFreq())) ||
      !::number(0, 5)) {
    if (parent)
      room = parent->in_room;
    else if (equippedBy)
      room = equippedBy->in_room;
    else if (in_room != ROOM_NOWHERE)
      room = in_room;
    else
      room = RecGetObjRoom(this);

    if ((room != ROOM_NOWHERE) && action_description)
      MakeNoise(room, action_description, action_description);
  }
}

