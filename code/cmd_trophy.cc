//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////
// test

#include "stdsneezy.h"
#include "cmd_trophy.h"

float trophy_exp_mod(float count)
{
  float min_mod=0.3;
  float max_mod=1.0; // shouldn't ever be above 1.0
  float free_kills=8; // how many kills you get before trophy kicks in
  float step_mod=0.5; // mod per step
  float num_steps=14.0; // number of steps

  float t1, t2, t3, t4, t5;

  t1=(double)(count-free_kills);
  t2=step_mod / num_steps;
  t3=t1*t2;
  t4=max_mod-t3;
  t5=(double)(max(t4*100, min_mod*100)/100);
  t5=(double)(min(t5*100, max_mod*100)/100);

  //  vlogf(LOG_PEEL, "%f %f %f %f %f", t1, t2, t3, t4, t5);

  return t5;
}



const char *describe_trophy_exp(float count)
{
  float f=trophy_exp_mod(count);

  return((f == 1.0) ? "<Y>full<1>" :
	 ((f >= 0.90) ? "<o>much<1>" :
	  ((f >= 0.80) ? "a fair amount" :
	   ((f >= 0.70) ? "<w>some<1>" : "<k>little<1>"))));
}

void TBeing::doTrophy(const char *arg)
{
  MYSQL_ROW row;
  MYSQL_RES *res;
  int rc, mcount=0;
  float count;
  char buf[256];
  string sb;


  if(!isPc()){
    sendTo("Mobs can't use this command!\n\r");
    return;
  }

  for (; isspace(*arg); arg++);


  rc=dbquery(&res, "sneezy", "doTrophy", "select mobvnum, count from trophy where name='%s' order by count", getName());

  while((row=mysql_fetch_row(res))){
    if(atoi(row[0])==0){
      continue;
    }
    int rnum = real_mobile(atoi(row[0]));
    if (rnum < 0) {
      vlogf(LOG_BUG, "DoTrophy detected bad mobvnum=%d for name='%s'", 
           atoi(row[0]), getName());
      continue;
    }

    if(*arg && !isname(arg, mob_index[rnum].name))
      continue;

    count=atof(row[1]);
    sprintf(buf, "You will gain %s experience when fighting %s.\n\r", 
	    describe_trophy_exp(count),
	    mob_index[rnum].short_desc);
    ++mcount;
    sb += buf;
  }

  sprintf(buf, "Total mobs: %i\n\r", mcount);
  sb += buf;
  if(mcount>0){
    sprintf(buf, "You have killed %1.2f%% of all mobs.\n\r",((float)((float)mcount/(float)mob_index.size())*100.0));
    sb += buf;
  }

  if (desc)
    desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);
    
  mysql_free_result(res);


  return;
}



