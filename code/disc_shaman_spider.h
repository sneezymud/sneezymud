
#ifndef __DISC_SHAMAN_SPIDER_H
#define __DISC_SHAMAN_SPIDER_H



class CDShamanSpider : public CDiscipline
{
public:
    CSkill skCreateGolem;

    CDShamanSpider()
      : CDiscipline(),
      skCreateGolem() {
    }
    CDShamanSpider(const CDShamanSpider &a)
      : CDiscipline(a),
      skCreateGolem(a.skCreateGolem) {
    }
    CDShamanSpider & operator=(const CDShamanSpider &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skCreateGolem = a.skCreateGolem;
      return *this;
    }
    virtual ~CDShamanSpider() {}
    virtual CDShamanSpider * cloneMe() { return new CDShamanSpider(*this); } 
private:
};


#endif
