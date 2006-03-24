#include <stdio.h>
#include <sys/time.h>

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"
#include "limits.h"
#include "heap.h"
#include "hash.h"
#include "race.h"
  
char *path[] =
{
  "wizards/",
  "\n"
};
  
struct blk_save
{
  char *poofin;
  char *poofout;
};

void pput(char *buf, char *temp)
{
  sprintf(temp,"%s",buf);
}

void blk_read(struct char_data *ch)
{
  FILE *fl;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int len,i;

  if (IS_NPC(ch))
    return;

  sprintf(buf,"%s%s.blk",path[0],GET_NAME(ch));
  if (!(fl=fopen(buf,"r"))) {
    vlog("Has no wizard file.");
    return;
  }

  fgets(buf,255,fl);
  fgets(buf2,255,fl);
  fclose(fl);

  for(i=0;i<strlen(buf);i++)
    if (buf[i] == '\n')
      buf[i] = '\0';
  for(i=0;i<strlen(buf2);i++)
    if (buf2[i] == '\n')
      buf2[i] = '\0';
      
  SET_BIT(ch->poof.pmask,BIT_POOF_OUT);
  SET_BIT(ch->poof.pmask,BIT_POOF_IN);
  len = strlen(buf);
  if (ch->poof.poofin && len >= strlen(ch->poof.poofin)) {
    free(ch->poof.poofin);
    ch->poof.poofin = (char *)malloc(len+1);
  } else {
    if (!ch->poof.poofin)
      ch->poof.poofin = (char *)malloc(len+1);
  }
  strcpy(ch->poof.poofin,buf);
  len = strlen(buf2);
  if (ch->poof.poofout && len >= strlen(ch->poof.poofout)) {
    free(ch->poof.poofout);
    ch->poof.poofout = (char *)malloc(len+1);
  } else {
    if (!ch->poof.poofout)
      ch->poof.poofout = (char *)malloc(len+1);
  }
  strcpy(ch->poof.poofout,buf2);
  vlog("Restoring wizard file.");
   
  return;
}

void blk_save(struct char_data *ch)
{
  FILE *fl;
  struct blk_save *blk;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  if (IS_SET(ch->poof.pmask, BIT_POOF_IN) &&
      (IS_SET(ch->poof.pmask, BIT_POOF_OUT))) {
    sprintf(buf,"%s%s.blk",path[0],GET_NAME(ch));
    unlink(buf);
    if (!(fl = fopen(buf,"wa+"))) {
      vlog("Couldn't write wizard file.");
      return;
    }
    sprintf(buf,"%s\n",ch->poof.poofin);
    fputs(buf,fl);
    sprintf(buf,"%s\n",ch->poof.poofout);
    fputs(buf,fl);
    fclose(fl);
  }
}
  

  
  


