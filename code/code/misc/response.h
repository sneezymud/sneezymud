//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team

//
//////////////////////////////////////////////////////////////////////////


#ifndef __RESPONSE_H
#define __RESPONSE_H

#include "parse.h"

class TBeing;

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
