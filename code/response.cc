//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"

command::command() :
  cmd(MAX_CMD_LIST),
  args(NULL),
  next(NULL)
{
}

command::command(const command &a) :
  cmd(a.cmd)
{
  args = mud_str_dup(args);
  if (a.next)
    next = new command(*a.next);
  else
    next = NULL;
}

command & command::operator=(const command &a)
{
  if (this == &a) return *this;
  command *t, *n;
  for (t = next; t; t = n) {
    n = t->next;
    delete t;
  }
  delete [] args;

  cmd = a.cmd;
  args = mud_str_dup(args);
  if (a.next)
    next = new command(*a.next);
  else
    next = NULL;
  return *this;
}

command::command(cmdTypeT c, char *d) :
  cmd(c),
  next(NULL)
{
  args = mud_str_dup(d);
}

command::~command()
{
  delete [] args;
  args = NULL;
}

resp::resp() :
  cmd(MAX_CMD_LIST),
  args(NULL),
  cmds(NULL),
  next(NULL)
{
}

resp::resp(const resp &a) :
  cmd(a.cmd)
{
  args = mud_str_dup(a.args);
  if (a.cmds)
    cmds = new command(*a.cmds);
  else
    cmds = NULL;
  if (a.next)
    next = new resp(*a.next);
  else
    next = NULL;
}

resp & resp::operator=(const resp &a)
{
  if (this == &a) return *this;
  resp *t, *n;
  for (t = next; t; t = n) {
    n = t->next;
    delete t;
  }
  delete [] args;
  delete cmds;

  cmd = a.cmd;
  args = mud_str_dup(a.args);

  if (a.cmds)
    cmds = new command(*a.cmds);
  else
    cmds = NULL;

  if (a.next)
    next = new resp(*a.next);
  else
    next = NULL;
  return *this;
}

resp::resp(cmdTypeT c, char *d) :
  cmd(c),
  cmds(NULL),
  next(NULL)
{
  args = mud_str_dup(d);
}

resp::~resp()
{
  delete [] args;
  args = NULL;

  command *c = cmds;
  while( c ) {
    command *tmp = c;
    c = tmp->next;
    delete tmp;
  }
}

RespMemory::RespMemory() :
  name(NULL),
  args(NULL),
  cmd(MAX_CMD_LIST),
  next(NULL)
{
}

RespMemory::RespMemory(cmdTypeT newCmd, TBeing *tBeing, const sstring &tArg)
{
  if (tBeing && !tBeing->getNameNOC(tBeing).empty()) {
    name = mud_str_dup(tBeing->getNameNOC(tBeing));
  } else {
    name = NULL;
  }

  if (!tArg.empty())
    args = mud_str_dup(tArg);

  cmd  = newCmd;
  next = NULL;
}

RespMemory::RespMemory(const RespMemory &a) :
  cmd(a.cmd)
{
  if (name) {
    delete [] name;
    name = NULL;
  }

  if (a.name)
    name = mud_str_dup(a.name);

  if (args) {
    delete [] args;
    args = NULL;
  }

  if (a.args)
    args = mud_str_dup(a.args);

  next = a.next;
}

RespMemory & RespMemory::operator = (const RespMemory &a)
{
  if (this == &a) return *this;

  if (name) {
    delete [] name;
    name = NULL;
  }

  if (a.name)
    name = mud_str_dup(a.name);

  if (args) {
    delete [] args;
    args = NULL;
  }

  if (a.args)
    args = mud_str_dup(a.args);

  next = a.next;

  return *this;
}

RespMemory::~RespMemory()
{
  delete [] name;
  name = NULL;
  delete [] args;
  args = NULL;
  next = NULL;
}

Responses::Responses() :
  respList(NULL),
  respCount(0),
  respMemory(NULL)
{
}

Responses::Responses(const Responses &a) :
  respCount(a.respCount)
{
  if (a.respList)
    respList = new resp(*a.respList);
  else
    respList = NULL;

  if (a.respMemory)
    respMemory = new RespMemory(*a.respMemory);
  else
    respMemory = NULL;
}

Responses & Responses::operator=(const Responses &a)
{
  if (this == &a) return *this;

  delete respList;

  if (a.respList)
    respList = new resp(*a.respList);
  else
    respList = NULL;

  respCount = a.respCount;

  if (a.respMemory)
    respMemory = new RespMemory(*a.respMemory);
  else
    respMemory = NULL;

  return *this;
}

Responses::~Responses()
{
  resp *r;
  RespMemory *m;

  while ((r = respList)) {
    respList = r->next;
    delete r;
  }

  while ((m = respMemory)) {
    respMemory = m->next;
    delete m;
  }

  respList   = NULL;
  respMemory = NULL;
}
