#pragma once

#include <cxxtest/TestSuite.h>
#include "code/sys/sstring.h"

namespace CxxTest {
  CXXTEST_TEMPLATE_INSTANTIATION
    class ValueTraits<sstring>
    {
      sstring _value;
      
    public:
      ValueTraits( sstring value ) : _value( value ) {}
      const char *asString( void ) const { return _value.c_str(); }
    };

};
