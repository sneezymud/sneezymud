//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

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
  MYSQL_ROW row=NULL;
  MYSQL_RES *res;
  int rc, mcount=0, vnum, header=0, zcount=0, bottom=0, zcountt=0;
  int zonesearch=0, processedrow=1;
  float count;
  char buf[256];
  string sb;
  unsigned int zone;

  if(!isPc()){
    sendTo("Mobs can't use this command!\n\r");
    return;
  }

  for (; isspace(*arg); arg++);
  
  if(!strncmp(arg, "zone", 4)){
    zonesearch=1;
    for (; !isspace(*arg); arg++);
  }

  rc=dbquery(&res, "sneezy", "doTrophy", "select mobvnum, count from trophy where name='%s' order by mobvnum", getName());

  for (zone = 0; zone < zone_table.size(); zone++) {
    zoneData &zd = zone_table[zone];
    
    while(1){
      if(processedrow)
	row=mysql_fetch_row(res);

      if(!row)
	break;

      processedrow=0;
      vlogf(LOG_PEEL, "mob=%i, top=%i", atoi(row[0]), zd.top);

      // sometimes we get an entry of 0 for med mobs I think
      vnum=atoi(row[0]);
      if(vnum==0)
	continue;

      // this mob doesn't belong to this zone, so break out to the zone loop
      if(vnum>zd.top)
	break;

      int rnum = real_mobile(atoi(row[0]));
      if (rnum < 0) {
	vlogf(LOG_BUG, "DoTrophy detected bad mobvnum=%d for name='%s'", 
	      atoi(row[0]), getName());
	continue;
      }

      if(zonesearch){
	if(*arg && !isname(arg, zd.name))
	  continue;
      } else {
	if(*arg && !isname(arg, mob_index[rnum].name))
	  continue;
      }

      // print the zone header if we haven't already
      // we do it here, so we can prevent printing headers for empty zones
      if(!header){
	sprintf(buf, "\n--%s\n", zd.name);
	sb += buf; 
	header=1;
      }

      count=atof(row[1]);
      sprintf(buf, "You will gain %s experience when fighting %s.\n\r", 
	      describe_trophy_exp(count),
	      mob_index[rnum].short_desc);
      ++mcount;
      ++zcount;
      sb += buf;

      processedrow=1;
      
#if 0
    sprintf(buf, "%3d %-38.38s %4dm %4dm %6d-%-6d %3d %.1f\n\r", 
	    zone, buf2, zd.lifespan, zd.age, bottom, zd.top, 
	    zd.zone_value,
	    (zd.num_mobs ? zd.mob_levels/zd.num_mobs : 0));
    sb += buf;
#endif
    }

    // we have some mobs for this zone, so do some tallies
    if(header){
      sprintf(buf, "Total mobs: %i\n\r", zcount);
      sb += buf;

      unsigned int objnx;
      for (objnx = 0; objnx < mob_index.size(); objnx++) {
	if(mob_index[objnx].virt >= bottom &&
	   mob_index[objnx].virt <= zd.top){
	  ++zcountt;
	}
      }

      sprintf(buf, "You have killed %1.2f%% of mobs in this zone.\n\r",((float)((float)zcount/(float)zcountt)*100.0));
      sb += buf;
    }

    header=zcount=zcountt=0;
    bottom=zd.top+1;
  }





  sprintf(buf, "\n--\nTotal mobs: %i\n\r", mcount);
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



void wipeTrophy(const char *name){
  dbquery(NULL, "sneezy", "wipeTrophy", "delete from trophy where name='%s'", name);
}
