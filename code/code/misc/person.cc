#include "person.h"
#include "database.h"

std::pair<bool, int> TPerson::doPersonCommand(cmdTypeT cmd, const sstring & argument, TThing *, bool)
{
  switch (cmd) {
    default:
      return std::make_pair(false, 0);
  }
  return std::make_pair(true, 0);
}
