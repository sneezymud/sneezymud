

#ifndef __DISC_SHAMAN_SKUNK_H
#define __DISC_SHAMAN_SKUNK_H


class CDShamanSkunk : public CDiscipline
{
public:
    CDShamanSkunk()
      : CDiscipline() {
    }
    CDShamanSkunk(const CDShamanSkunk &a)
      : CDiscipline(a) {
    }
    CDShamanSkunk & operator=(const CDShamanSkunk &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      return *this;
    }
    virtual ~CDShamanSkunk() {}
    virtual CDShamanSkunk * cloneMe() { return new CDShamanSkunk(*this); }

private:
};


#endif

