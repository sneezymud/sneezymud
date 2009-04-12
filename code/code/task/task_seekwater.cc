//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "extern.h"

class TBeing;
class TRoom;
class TObj;

// For now the seekwater task is part of the tracking task, found right
// above this.  Will strip it later on.
int task_seekwater(TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *)
{
  return FALSE;
}
