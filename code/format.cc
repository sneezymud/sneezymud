#include "stdsneezy.h"
#include "format.h"


sstring fmt::doFormat(const sstring &fmt, const sstring &x)
{
  sstring buf;

  ssprintf(buf, fmt.c_str(), x.c_str());

  return buf;
}

