#include <cxxtest/TestSuite.h>

#include "configuration.h"
#include "code/tests/ValueTraits.h"
#include "obj_general_weapon.h"
#include "extern.h"
#include "toggle.h"

class Stat : public CxxTest::TestSuite
{
 public:
  void setUp(){
    Config::doConfiguration();
    freopen("code/tests/output/Cast.out", "w", stderr);
    generate_obj_index();
    toggleInfo.loadToggles();
  }

  void testGenWeapon(){
    TObj *w=read_object(6317, VIRTUAL);
    TGenWeapon *tgw;

    TS_ASSERT(w);
    TS_ASSERT((tgw=dynamic_cast<TGenWeapon *>(w)));
    
    TS_ASSERT_THROWS_NOTHING(tgw->statObjInfo());
  }
};
