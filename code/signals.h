//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: signals.h,v $
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

