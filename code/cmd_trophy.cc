//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"

// this code is duplicated somewhat in doConsider
void TBeing::doTrophy(const char *arg){
  MYSQL_ROW row;
  MYSQL_RES *res;
  int rc, count;
  char buf[256];
  string sb;
  
  sendTo("Disabled.\n\r");
  return;


  if(!isPc()){
    sendTo("Mobs can't use this command!\n\r");
    return;
  }

  for (; isspace(*arg); arg++);


  rc=dbquery(&res, "sneezy", "doTrophy", "select mobvnum, count from trophy where name='%s'", getName());

  while((row=mysql_fetch_row(res))){
    if(*arg && !isname(arg, mob_index[real_mobile(atoi(row[0]))].name))
      continue;

    count=atoi(row[1]);
    sprintf(buf, "You will gain %s experience when fighting %s.\n\r", 
	    ((count < 5) ? "<Y>full<1>" :
	     ((count < 7) ? "<o>much<1>" :
	     ((count < 9) ? "a fair amount" :
	      ((count < 11) ? "<w>some<1>" : "<k>little<1>")))),
	    mob_index[real_mobile(atoi(row[0]))].short_desc);

    sb += buf;
  }
  if (desc)
    desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);
    

  return;
}



