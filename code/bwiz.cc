#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include "stdsneezy.h"
#include "connect.h"

#define WIZ_KEY_FILE "/mud/prod/lib/IPC_Wiznet"
#define WIZNET_FORMAT "%s %s"
#define MAX_MSGBUF_LENGTH 256

int my_ipc_id, other_ipc_id;

key_t keyval;
int qid=-2;
struct mud_msgbuf
{
    long mtype;
    char mtext[MAX_MSGBUF_LENGTH+1];
};

int openQueue()
{
  int oldqid=qid;
  
  if (qid==-2) {
    extern int gamePort;
    if (gamePort==PROD_GAMEPORT) {
      my_ipc_id = PROD_GAMEPORT;
      other_ipc_id = BUILDER_GAMEPORT;
    } else {
      my_ipc_id = BUILDER_GAMEPORT;
      other_ipc_id = PROD_GAMEPORT;
    }
    keyval = ftok(WIZ_KEY_FILE, 'm');
  }
  if ((qid = msgget(keyval, IPC_CREAT|0666)) == -1) {
    vlogf(LOG_BUG, fmt("Unable to msgget keyval %d, errno: %d.") %  keyval % errno);
    return -1;
  }
  if (oldqid!=qid) {
    vlogf(LOG_BUG, fmt("msgget successful, qid: %d, keyval: %d") %  qid % keyval);
    oldqid = qid;
  }
  return 1;
}

void closeQueue()
{
    msgctl(qid, IPC_RMID, 0);
    vlogf(LOG_BUG, "closeQueue");
}

void mudSendMessage(int mtype, int ctype, const char *arg)
{
  struct mud_msgbuf qbuf;
  int ret;
  
  if (openQueue()<0)
    return;

  if (mtype<=0) {
    vlogf(LOG_BUG, "invalid mtype, must be > 0, setting to 1");
    mtype = 1;
  }
  snprintf(qbuf.mtext, MAX_MSGBUF_LENGTH, "%d %s", ctype, arg);
  qbuf.mtype = mtype;
  
  if ((ret = msgsnd(qid, (struct msgbuf *)&qbuf, strlen(qbuf.mtext)+1, 0)) == -1)
    vlogf(LOG_BUG, fmt("mudSendMessage: errno: %d ret: %d") %  errno % ret);
}

void TBeing::mudMessage(TBeing *ch, int channel, const char *arg)
{
  char tbuf[MAX_MSGBUF_LENGTH+1];
  snprintf(tbuf, MAX_MSGBUF_LENGTH, "%s: %s", ch->getName(), arg);
  mudSendMessage(other_ipc_id, channel, tbuf);
}

void recvTextHandler(const char *str)
{
  Descriptor *d;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], chname[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH-1];
  //  int channel=-1, level=-1;

  str = one_argument(str, arg1);
  str = one_argument(str, arg2);
  str = one_argument(str, chname);
  //  channel = convertTo<int>(arg1);
  //  level = convertTo<int>(arg2);

  snprintf(buf, MAX_STRING_LENGTH, WIZNET_FORMAT, chname, str);

  for (d = descriptor_list; d; d = d->next) {
    TBeing *och;
    och = d->original ? d->original : d->character;
    if (!och)
      continue;
    if (och->hasWizPower(POWER_WIZNET)) {
      och->sendTo(buf);
    } else {
    }
  }
}
 
void mudRecvMessage()
{
  struct mud_msgbuf qbuf;
  int ret;
  
  if (openQueue() < 0)
    return;

  while ((ret = msgrcv(qid, (struct msgbuf *)&qbuf, MAX_MSGBUF_LENGTH, my_ipc_id, IPC_NOWAIT)) > 0)
    recvTextHandler(qbuf.mtext);
  
  if (ret==-1 && errno!=ENOMSG)
    vlogf(LOG_BUG, fmt("mudRecvMessage: errno: %d ret: %d") %  errno % ret);
}

