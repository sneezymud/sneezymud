#pragma once

// We have way too many places where we look over descriptor_list.
// In some places, we also deallocate elements while looping, leading to
// crashes.
//
// Let's bring some structure here by providing a nicer mechanism for looping,
// with the longer-term view to remove descriptor_list completely
// (perhaps replace with a std::vector or such).
// After we get rid of direct access of descriptor_list, remove the custom
// iterator and instead start returning the actual implementation's iterator.

class Descriptor;

class TDescriptorList {
  public:
    class iterator {
        friend class TDescriptorList;
        // We need a custom iterator just to overload the ++ operator to call
        // ->next
      public:
        iterator operator++();
        Descriptor* operator*();
        const Descriptor* operator*() const;
        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;

      private:
        Descriptor* d;
        explicit iterator(Descriptor*);
    };

    const iterator begin() const;
    const iterator end() const;
    iterator findByCharName(const char* name_buf) const;
};

extern TDescriptorList DescriptorList;
extern Descriptor* descriptor_list;
