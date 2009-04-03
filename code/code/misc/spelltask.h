//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: spelltask.h,v $
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


/*************************************************************************

      SneezyMUD - All rights reserved, SneezyMUD Coding Team
      spelltask.h : interface for mob/player spell casting tasks

  ----------------------------------------------------------------------

  Tasks provide a mechanism for delayed/sequenced/periodic mob actions.
  Basically, they function a lot like spec_procs.  They tie up the 
  player/mob for a set ammount of time, allowing things to happen in the
  meantime.

*************************************************************************/

#ifndef __SPELLTASK
#define __SPELLTASK 1

//CASTER HAS TO BE ABLE TO SEE THE VICTIM
const unsigned int CASTFLAG_SEE_VICT   = (1<<0);
const unsigned int CASTFLAG_NO_SENSE_CAST   = (1<<1);
const unsigned int CASTFLAG_MAYBE_SENSE_CAST   = (1<<2);
const unsigned int CASTFLAG_CAST_INDEFINITE   = (1<<3);

void cast_warn_busy(const TBeing *, spellNumT);

//  This is what it looks like
//  start_cast(this, victim, objectTarget, room, spell_num, difficulty, targetType, arg, rounds, wasInRoom, status, flags, text, next_update);

extern int start_cast(TBeing *, TBeing *, TThing *, TRoom *, spellNumT, taskDiffT, int, const char *, lag_t, unsigned short, ubyte, int, int, int);

#endif
