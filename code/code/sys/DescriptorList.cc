#include "DescriptorList.h"
#include "being.h"
#include "connect.h"

extern Descriptor* descriptor_list;

TDescriptorList::iterator::iterator(Descriptor* d) : d(d) {}

TDescriptorList::iterator TDescriptorList::iterator::operator++() {
  d = d->next;
  return *this;
}

Descriptor* TDescriptorList::iterator::operator*() { return d; }

const Descriptor* TDescriptorList::iterator::operator*() const { return d; }

bool TDescriptorList::iterator::operator==(
  const TDescriptorList::iterator& other) const {
  return d == other.d;
}

bool TDescriptorList::iterator::operator!=(
  const TDescriptorList::iterator& other) const {
  return !(*this == other);
}

///

const TDescriptorList::iterator TDescriptorList::begin() const {
  return iterator(descriptor_list);
}

const TDescriptorList::iterator TDescriptorList::end() const {
  return iterator(nullptr);
}

TDescriptorList::iterator TDescriptorList::findByCharName(
  const char* name_buf) const {
  for (auto d : *this) {
    if (d->character && !d->character->name.empty() &&
        !(d->character->name.lower()).compare(name_buf))
      return iterator(d);
  }
  return iterator(nullptr);
}
