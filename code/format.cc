#include "stdsneezy.h"

sstring fmt::doFormat(const sstring &fmt, const sstring &x)
{
  return doFormat(fmt, x.c_str());
}

