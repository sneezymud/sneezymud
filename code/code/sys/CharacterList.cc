#include "CharacterList.h"
#include "being.h"
#include "connect.h"

extern TBeing *character_list;

TCharacterList::iterator::iterator(TBeing* d)
  : d(d)
{}

TCharacterList::iterator TCharacterList::iterator::operator++()
{
  d = d->next;
  return *this;
}

TBeing* TCharacterList::iterator::operator*()
{
  return d;
}

const TBeing* TCharacterList::iterator::operator*() const
{
  return d;
}

bool TCharacterList::iterator::operator==(const TCharacterList::iterator& other) const
{
  return d == other.d;
}

bool TCharacterList::iterator::operator!=(const TCharacterList::iterator& other) const
{
  return !(*this == other);
}

///

const TCharacterList::iterator TCharacterList::begin() const
{
  return iterator(character_list);
}

const TCharacterList::iterator TCharacterList::end() const
{
  return iterator(nullptr);
}
