//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "cmd_trophy.h"
#include "database.h"


TTrophy::TTrophy(string n) :
  db(new TDatabase("sneezy")),
  parent(NULL),
  name(n)
{
}

TTrophy::TTrophy(TBeing *p) :
  db(new TDatabase("sneezy")),
  parent(p),
  name("")
{
}

string TTrophy::getMyName(){
  if(parent){
    return parent->getName();
  } else {
    // name should be "" if we're uninitialized, so just return it either way
    return name;
  }
}

void TTrophy::setName(string n){
  parent=NULL;
  name=n;
}

void TTrophy::addToCount(int vnum, double add){
  if(vnum==-1 || vnum==0 || getMyName()==""){ return; }

  db->query("select * from trophy where name='%s' and mobvnum=%i",
	    getMyName().c_str(), vnum);
  if(!db->fetchRow()){
    db->query("insert into trophy values ('%s', %i, %f)",
	      getMyName().c_str(), vnum, add);
  } else {
    db->query("update trophy set count=count+%f where name='%s' and mobvnum=%i",
	      add, getMyName().c_str(), vnum);
  }
}


float TTrophy::getCount(int vnum)
{
  db->query("select count from trophy where name='%s' and mobvnum=%i",
	   getMyName().c_str(), vnum);
  if(db->fetchRow())
    return atof_safe(db->getColumn(0));
  else 
    return 0.0;
}


float TTrophy::getExpModVal(float count)
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


const char *TTrophy::getExpModDescr(float count)
{
  float f=getExpModVal(count);

  return((f == 1.0) ? "<Y>full<1>" :
	 ((f >= 0.90) ? "<o>much<1>" :
	  ((f >= 0.80) ? "a fair amount" :
	   ((f >= 0.70) ? "<w>some<1>" : "<k>little<1>"))));
}

// this function is a little messy, I apologize
void TBeing::doTrophy(const char *arg)
{
  int mcount=0, vnum, header=0, zcount=0, bottom=0, zcountt=0;
  int zonesearch=0, processrow=1, summary=0;
  float count;
  char buf[256];
  string sb;
  unsigned int zone;

  if(!isPc()){
    sendTo("Mobs can't use this command!\n\r");
    return;
  }

  TTrophy trophy(getName());


  for (; isspace(*arg); arg++);

  if(!strncmp(arg, "zone", 4)){
    if(arg[4]){
      for (; !isspace(*arg); arg++);
      zonesearch=-1;
    } else {
      zonesearch=roomp->getZoneNum();
    }
  } else if(!strncmp(arg, "summary", 7)){
    summary=1;
  }

  TDatabase db("sneezy");
  db.query("select mobvnum, count from trophy where name='%s' order by mobvnum", getName());

  for (zone = 0; zone < zone_table.size(); zone++) {
    zoneData &zd = zone_table[zone];
    
    while(1){
      if(processrow){
	if(!db.fetchRow()){
	  break;
	}
      }

      // sometimes we get an entry of 0 for med mobs I think
      vnum=atoi_safe(db.getColumn(0));
      if(vnum==0){
	continue;
      }

      // this mob doesn't belong to this zone, so break out to the zone loop
      if(vnum>zd.top){
	processrow=0; // don't get the next row yet
	break;
      } else {
	processrow=1;
      }

      int rnum = real_mobile(atoi_safe(db.getColumn(0)));
      if (rnum < 0) {
	vlogf(LOG_BUG, "DoTrophy detected bad mobvnum=%d for name='%s'", 
	      atoi_safe(db.getColumn(0)), getName());
	continue;
      }

      if(zonesearch==-1){
	if(*arg && !isname(arg, zd.name))
	  continue;
      } else if(zonesearch>0){
	if(zonesearch!=zd.zone_nr)
	  continue;
      } else if(!summary){
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

      count=atof_safe(db.getColumn(1));

      if(!summary){
	sprintf(buf, "You will gain %s experience when fighting %s.\n\r", 
		trophy.getExpModDescr(count),
		mob_index[rnum].short_desc);
	sb += buf;
      }

      ++mcount;
      ++zcount;

      processrow=1; // ok to get the next row
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
    

  return;
}



void TTrophy::wipe(){
  db->query("delete from trophy where upper(name)=upper('%s')", getMyName().c_str());
}
