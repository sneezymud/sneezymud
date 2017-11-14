#pragma once

#include "sstring.h"

enum logTypeT
{
  LOG_SILENT  = -2, // Log is recoreded but not echoed to immortals (anti-spam)
  LOG_NONE    = -1, // Empty
  LOG_MISC    =  0, // Anything not yet defined below
  LOG_LOW     =  1, // LOW Errors
  LOG_FILE    =  2, // File io Errors
  LOG_BUG     =  3, // 'Bugs' and other such reports
  LOG_PROC    =  4, // Errors regarding mob/obj/room procs
  LOG_PIO     =  5, // Player Login/Logout reports
  LOG_IIO     =  6, // Immortal Login/Logout 'additives'
  LOG_CLIENT  =  7, // Various errors associated with the SneezyMUD Client.
  LOG_COMBAT  =  8, // Various errors associated with the combat code.
  LOG_CHEAT   =  9, // Various logs associated with the cheating code.
  LOG_FACT    = 10, // Various Faction Stuff
  LOG_DB      = 11, // Database stuff

  LOG_MOB     = 15, // Errors in Mobiles not yet defined below
  LOG_MOB_AI  = 16, // Errors in Mobile Logic
  LOG_MOB_RS  = 17, // Errors in Mobile Response Scripts

  LOG_OBJ     = 18, // Errors in Objects not yet defined below

  LOG_EDIT    = 21, // Various 'edit' errors

  LOG_MAX     = 23, // This is here to prevent unwarrented use of the belows.

  LOG_BATOPR  = 24, // Batopr only logs
  LOG_BRUTIUS = 25, // Brutius only logs
  LOG_COSMO   = 26, // Cosmo only logs
  LOG_MAROR   = 27, // Maror only logs
  LOG_PEEL    = 28,  // Peel only logs
  LOG_LAPSOS  = 29, // Lapsos only logs
  LOG_DASH    = 30, // Dash only
  LOG_ANGUS   = 31,  // Angus only
  LOG_JESUS   = 23  // Jesus only
};

void vlogf(logTypeT tError, const sstring &errorMsg);
