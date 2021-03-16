#include <cxxtest/TestSuite.h>

#include "code/tests/ValueTraits.h"
#include "configuration.h"
#include "structs.h"

class ConfigFile : public CxxTest::TestSuite
{

 public:
  void testBeta(){
    char *argv[3];
    argv[0]=mud_str_dup("exe");
    argv[1]=mud_str_dup("-c");
    argv[2]=mud_str_dup("sneezy_beta.cfg");

    TS_ASSERT_THROWS_NOTHING(Config::doConfiguration(3, argv));
    delete[] argv[0];
    delete[] argv[1];
    delete[] argv[2];
  }

  void testProd(){
    char *argv[3];
    argv[0]=mud_str_dup("exe");
    argv[1]=mud_str_dup("-c");
    argv[2]=mud_str_dup("sneezy_prod.cfg");

    TS_ASSERT_THROWS_NOTHING(Config::doConfiguration(3, argv));
    delete[] argv[0];
    delete[] argv[1];
    delete[] argv[2];
  }

};
