
#ifndef __DISC_SHAMAN_SPIDER_H
#define __DISC_SHAMAN_SPIDER_H

class CDShamanSpider : public CDiscipline
{
public:
    CSkill skSticksToSnakes;
    CSkill skControlUndead;
    CSkill skClarity;

    CDShamanSpider()
      : CDiscipline(),
      skSticksToSnakes(),
      skControlUndead(),
      skClarity() { 
    }
    CDShamanSpider(const CDShamanSpider &a)
      : CDiscipline(a),
      skSticksToSnakes(a.skSticksToSnakes),
      skControlUndead(a.skControlUndead),
      skClarity(a.skClarity) {
    }
    CDShamanSpider & operator=(const CDShamanSpider &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSticksToSnakes = a.skSticksToSnakes;
      skControlUndead = a.skControlUndead;
      skClarity = a.skClarity;
      return *this;
    }
    virtual ~CDShamanSpider() {}
    virtual CDShamanSpider * cloneMe() { return new CDShamanSpider(*this); }
private:
};

    void controlUndead(TBeing *, TBeing *);
    void controlUndead(TBeing *, TBeing *, TMagicItem *);
    int controlUndead(TBeing *, TBeing *, int, byte);

    int sticksToSnakes(TBeing *, TBeing *);
    int sticksToSnakes(TBeing *, TBeing *, TMagicItem *);
    int sticksToSnakes(TBeing *, TBeing *, int, byte);

    int clarity(TBeing *, TBeing *);
    int castClarity(TBeing *, TBeing *);
    void clarity(TBeing *, TBeing *, TMagicItem *);
    int clarity(TBeing *, TBeing *, int, byte);


#endif

