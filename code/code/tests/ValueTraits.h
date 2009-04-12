#ifndef __VALUETRAITS_H
#define __VALUETRAITS_H

#include <cxxtest/TestSuite.h>
#include "sstring.h"

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
#endif
