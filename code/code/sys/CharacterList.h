#pragma once

// We have way too many places where we look over character_list.
// In some places, we also deallocate elements while looping, leading to crashes.
//
// Let's bring some structure here by providing a nicer mechanism for looping,
// with the longer-term view to remove character_list completely
// (perhaps replace with a std::vector or such).
// After we get rid of direct access of character_list, remove the custom iterator
// and instead start returning the actual implementation's iterator.

class TBeing;

class TCharacterList {
  public:
    class iterator {
      friend class TCharacterList;
      // We need a custom iterator just to overload the ++ operator to call ->next
      public:
        iterator operator++();
        TBeing* operator*();
        const TBeing* operator*() const;
        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;

      private:
        TBeing* d;
        explicit iterator(TBeing*);
    };

    const iterator begin() const;
    const iterator end() const;
    // iterator findByCharName(const char* name_buf) const;
};

extern TCharacterList CharacterList;
