
#ifndef __DISC_SHAMAN_CONTROL_H
#define __DISC_SHAMAN_CONTROL_H


class CDShamanControl : public CDiscipline
{
public:
    CSkill skVampiricTouch;
    CSkill skLifeLeech;

    CDShamanControl()
      : CDiscipline(),
      skVampiricTouch(),
      skLifeLeech() {
    }
    CDShamanControl(const CDShamanControl &a)
      : CDiscipline(a),
      skVampiricTouch(a.skVampiricTouch),
      skLifeLeech(a.skLifeLeech) {
    }
    CDShamanControl & operator=(const CDShamanControl &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skVampiricTouch = a.skVampiricTouch;
      skLifeLeech = a.skLifeLeech;
      return *this;
    }
    virtual ~CDShamanControl() {}
    virtual CDShamanControl * cloneMe() { return new CDShamanControl(*this); }
private:
};

    void vampiricTouch(TBeing *, TBeing *);
    void lifeLeech(TBeing *);

#endif
