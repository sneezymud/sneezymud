//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_mend_limb.cc,v $
// Revision 5.2  2003/05/26 14:44:11  peel
// char * to sstring conversions
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"

int TBeing::doMendLimb(const sstring &)
{
  sendTo("Command not yet implemented.\n\r");
  return FALSE;
}
