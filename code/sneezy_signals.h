//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: sneezy_signals.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __SIGNALS_H
#define __SIGNALS_H

class SignalHandler {
  private:
    int mask, checkpoint;
  public :
    SignalHandler();
    int startup();
    void signalIgnore(void);
    void deadlockCheck(void);
    void signalShutdown(void);
    void sleep();
    void wake();
    void resetCheckpoint() {
      checkpoint = 0;
    }
};

extern SignalHandler *gSigHandler;

#endif
