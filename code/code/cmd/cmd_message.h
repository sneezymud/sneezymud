//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_message.h,v $
// Revision 5.2  2003/03/13 22:40:52  peel
// added sstring class, same as string but takes NULL as an empty string
// replaced all uses of string to sstring
//
// Revision 5.1.1.3  2001/02/08 08:02:19  cosmo
// changed return type of operator ()
//
// Revision 5.1.1.2  1999/12/09 18:40:40  lapsos
// Massive revision of setsev/wizfiles
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


#ifndef __CMD_MESSAGE_H
#define __CMD_MESSAGE_H

#define NEWBIE_PURGATORY_LENGTH 7257600

enum messageTypeT
{
  MSG_ERROR     = 0,
  MSG_MIN       = 1,
  MSG_IMM_TITLE = 1,
  MSG_PURGE,
  MSG_PURGE_TARG,
  MSG_RLOAD,
  MSG_LOAD_OBJ,
  MSG_LOAD_MOB,
  MSG_MEDIT,
  MSG_OEDIT,
  MSG_SWITCH_TARG,
  MSG_MOVE_IN,
  MSG_MOVE_OUT,
  MSG_SLAY,
  MSG_SLAY_TARG,
  MSG_FORCE,
  MSG_BAMFIN,
  MSG_BAMFOUT,
  MSG_LONGDESCR,
  MSG_NOTE,
  MSG_MAX
};

const int MSG_TYPE_MAX   = MSG_MAX;
const int MSG_REQ_GNAME  = (1 << 0); // God Name
const int MSG_REQ_ONAME  = (1 << 1); // Other Name [mobile/object/player]
const int MSG_REQ_STRING = (1 << 2); // Command sstring
const int MSG_REQ_DIR    = (1 << 3); // Direction of travel

struct messageBuffer
{
  sstring msgImmTitle;
  sstring msgPurge;
  sstring msgPurgeTarg;
  sstring msgRLoad;
  sstring msgLoadObj;
  sstring msgLoadMob;
  sstring msgMEdit;
  sstring msgOEdit;
  sstring msgSwitchTarg;
  sstring msgMoveIn;
  sstring msgMoveOut;
  sstring msgSlay;
  sstring msgSlayTarg;
  sstring msgForce;
  sstring msgBamfin;
  sstring msgBamfout;
  sstring msgLongDescr;
  sstring msgNote;
};

class TMessages
{
  public:
    messageBuffer tMessages;
    TBeing *tPlayer = nullptr;

    static sstring getImmortalTitles(TBeing *);
    // Return the old default messages.
    static sstring getDefaultMessage(messageTypeT, TBeing *);
    // ?(Has message type), also checks for appropriate power setting.
    bool operator== (messageTypeT) const;
    // Set message type to message
    void operator() (messageTypeT, sstring);
    // Sets the fields in message  [Call this to actually Get the messages]
    sstring operator() (messageTypeT, TThing * = NULL, const char * = NULL, bool = true) const;
    // Used by  : Get message from type
    sstring const& operator[] (messageTypeT) const;
    void initialize();
    void savedown();
};

#endif
