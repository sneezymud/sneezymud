//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: response.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __RESPONCE_H
#define __RESPONCE_H


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
    RespMemory(cmdTypeT, TBeing *, const char *);
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

#endif // __RESPONCE_H__
