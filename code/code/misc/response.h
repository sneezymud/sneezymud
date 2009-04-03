//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: response.h,v $
// Revision 5.3  2003/12/02 23:40:03  peel
// did some char * -> sstring conversions
//
// Revision 5.2  2002/01/08 18:26:40  peel
// removed being.h from stdsneezy.h
// fixed some header file interdependencies
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


#ifndef __RESPONSE_H
#define __RESPONSE_H


class command {
  public:
    cmdTypeT cmd;
    char *args;
    command *next;

  private:
    command();  //intentionally private, so can't be called
  public:
    command(cmdTypeT c, char *d);
    command(const command &a);
    command & operator=(const command &a);
    ~command();
};

  
class resp {
  public:
    cmdTypeT cmd;
    char *args;
    command *cmds;
    resp *next;
  
  private:
    resp();  // made private, so intentionally can not be called
  public:
    resp(cmdTypeT c, char *d);
    resp(const resp &a);
    resp & operator=(const resp &a);
    ~resp();
};

class RespMemory {
  public:
    char             *name,
                     *args;
    cmdTypeT          cmd;
    class RespMemory *next;

  private:
    RespMemory();

  public:
    RespMemory(cmdTypeT, TBeing *, const sstring &);
    RespMemory(const RespMemory &);
    RespMemory & operator = (const RespMemory &);
    ~RespMemory();
};

class Responses {
  public:
    resp       *respList;
    int	        respCount;
    RespMemory *respMemory;

    Responses();
    Responses(const Responses &a);
    Responses & operator=(const Responses &a);
    virtual ~Responses();
};

#endif // __RESPONSE_H__
