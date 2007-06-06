//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "cmd_trophy.h"
#include "database.h"
#include "process.h"

TTrophy::TTrophy(sstring n) :
  db(new TDatabase(DB_SNEEZY)),
  parent(NULL),
  name(n)
{
}

TTrophy::TTrophy(TBeing *p) :
  db(new TDatabase(DB_SNEEZY)),
  parent(p),
  name("")
{
}

sstring TTrophy::getMyName(){
  if(parent){
  
    if (parent->specials.act & ACT_POLYSELF) {
      return parent->desc->original->getName();
    } else { 
      return parent->getName();
    }
  } else {
    // name should be "" if we're uninitialized, so just return it either way
    return name;
  }
}

void TTrophy::setName(sstring n){
  parent=NULL;
  name=n;
}

// procTrophyDecay
procTrophyDecay::procTrophyDecay(const int &p)
{
  trigger_pulse=p;
  name="procTrophyDecay";
}

void procTrophyDecay::run(int) const
{
  TDatabase db(DB_SNEEZY);
  float dec=0.25;

  for(TBeing *tb=character_list;tb;tb=tb->next){
    if(tb->isPc()){
      db.query("update trophy set count=count-%f where player_id=%i and count > %f",
	       dec, tb->getPlayerID(), dec);
    }
  }
}

void TTrophy::addToCount(int vnum, double add){
  if(vnum==-1 || vnum==0 || getMyName()==""){ return; }

  int player_id=parent->getPlayerID();

  // in most cases we just want to do an update, so start with that
  db->query("update trophy set count=count+%f, totalcount=totalcount+%f where player_id=%i and mobvnum=%i",
      add, (add>0 ? add : 0), player_id, vnum);
  if (db->rowCount() == 0) {
    // no row for this player & mob so do an insert instead
    db->query("insert into trophy values (%i, %i, %f, %f)",
        player_id, vnum, add, (add>0 ? add : 0));
  }
  
  db->query("update trophyplayer set total=total+%f where player_id=%i",
     add, player_id);
}


float TTrophy::getCount(int vnum)
{
  db->query("select count from trophy where player_id=%i and mobvnum=%i",
	   parent->getPlayerID(), vnum);
  if(db->fetchRow())
    return convertTo<float>((*db)["count"]);
  else 
    return 0.0;
}

float TTrophy::getTotalCount(int vnum)
{
  db->query("select totalcount from trophy where player_id=%i and mobvnum=%i",
	   parent->getPlayerID(), vnum);
  if(db->fetchRow())
    return convertTo<float>((*db)["totalcount"]);
  else 
    return 0.0;
}



float TTrophy::getExpModVal(float count, int mobvnum)
{
  float min_mod=0.3;
  float max_mod=1.0; // shouldn't ever be above 1.0
  float free_kills=8; // how many kills you get before trophy kicks in
  float step_mod=0.5; // mod per step
  float num_steps=14.0; // number of steps

  if(mob_index[real_mobile(mobvnum)].numberLoad>0)
    count/=mob_index[real_mobile(mobvnum)].numberLoad;

  float t1, t2, t3, t4, t5;

  t1=(double)(count-free_kills);
  t2=step_mod / num_steps;
  t3=t1*t2;
  t4=max_mod-t3;
  t5=(double)(max(t4*100, min_mod*100)/100);
  t5=(double)(min(t5*100, max_mod*100)/100);

  //  vlogf(LOG_PEEL, fmt("%f %f %f %f %f") %  t1 % t2 % t3 % t4 % t5);


  return t5;
}


const char *TTrophy::getExpModDescr(float count, int mobvnum)
{
  float f=getExpModVal(count, mobvnum);

  return((f == 1.0) ? "<Y>full<1>" :
	 ((f >= 0.90) ? "<o>much<1>" :
	  ((f >= 0.80) ? "a fair amount of" :
	   ((f >= 0.70) ? "<w>some<1>" : "<k>little<1>"))));
}

// this function is a little messy, I apologize
void TBeing::doTrophy(const sstring &arg)
{
  int mcount=0, vnum, header=0, zcount=0, bottom=0, zcountt=0;
  int zonesearch=0, processrow=1, active_zcount=0;
  bool summary=false;
  float count;
  sstring buf, sb, arg1, arg2;
  unsigned int zone;

  if(!isPc()){
    sendTo("Mobs can't use this command!\n\r");
    return;
  }

  TBeing *per = NULL;
  if (specials.act & ACT_POLYSELF)
    per = desc->original;
  else per = this;
  
  TTrophy trophy(per->getName());

  arg1=arg.word(0);
  arg2=arg.word(1);

  if(arg1=="zone"){
    if(!arg2.empty()){
      arg1=arg2;
      zonesearch=-1;
    } else {
      zonesearch=roomp->getZoneNum();
    }
  } else if(arg1=="summary"){
    summary=true;
  }

  TDatabase db(DB_SNEEZY);
  db.query("select mobvnum, count from trophy where player_id=%i order by mobvnum", per->getPlayerID());

  for (zone = 0; zone < zone_table.size(); zone++) {
    zoneData &zd = zone_table[zone];
    
    while(1){
      if(processrow){
	if(!db.fetchRow()){
	  break;
	}
      }

      // sometimes we get an entry of 0 for med mobs I think
      vnum=convertTo<int>(db["mobvnum"]);
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

      int rnum = real_mobile(convertTo<int>(db["mobvnum"]));
      if (rnum < 0) {
	vlogf(LOG_BUG, fmt("DoTrophy detected bad mobvnum=%d for name='%s'") %  
	      convertTo<int>(db["mobvnum"]) % per->getName());
	continue;
      }

      if(zonesearch==-1){
	if(!isname(arg1, zd.name))
	  continue;
      } else if(zonesearch>0){
	if(zonesearch!=zd.zone_nr)
	  continue;
      } else if(!summary){
	if(!arg1.empty() && !isname(arg1, mob_index[rnum].name))
	  continue;
      }

      // print the zone header if we haven't already
      // we do it here, so we can prevent printing headers for empty zones
      if(!header){
	buf = fmt("\n--%s\n") % zd.name;
	sb += buf; 
	header=1;
      }

      count=convertTo<float>(db["count"]);

      if(!summary){
	buf = fmt("You will gain %s experience when fighting %s.\n\r") %
		trophy.getExpModDescr(count,vnum) % mob_index[rnum].short_desc;
	sb += buf;
      }

      ++mcount;
      ++zcount;

      if(mob_index[rnum].doesLoad)
	++active_zcount;

      processrow=1; // ok to get the next row
    }

    // we have some mobs for this zone, so do some tallies
    if(header){
      buf = fmt("Total mobs: %i\n\r") % zcount;
      sb += buf;

      unsigned int objnx;
      for (objnx = 0; objnx < mob_index.size(); objnx++) {
	if(mob_index[objnx].virt >= bottom &&
	   mob_index[objnx].virt <= zd.top &&
	   mob_index[objnx].doesLoad){
	  ++zcountt;
	}
      }

      buf = fmt("You have killed %1.2f%c of mobs in this zone.\n\r") %((float)((float)active_zcount/(float)zcountt)*100.0) % '%';
      sb += buf;
    }

    header=zcount=zcountt=active_zcount=0;
    bottom=zd.top+1;
  }



  int activemobcount=0;
  for (unsigned int mobnum = 0; mobnum < mob_index.size(); mobnum++) {
    for (unsigned int zone = 0; zone < zone_table.size(); zone++) {
      if(mob_index[mobnum].virt <= zone_table[zone].top &&
	 mob_index[mobnum].doesLoad){
	if(zone_table[zone].enabled)
	  activemobcount++;
	break;
      }
    }
  }



  buf = fmt("\n--\nTotal mobs: %i\n\r") % mcount;
  sb += buf;
  if(mcount>0){
    buf = fmt("You have killed %1.2f%c of all mobs.\n\r") %((float)((float)mcount/(float)activemobcount)*100.0) % '%';
    sb += buf;
  }

  if (desc)
    desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
    

  return;
}



void TTrophy::wipe(){
  db->query("select id from player where name='%s'", getMyName().c_str());
  
  if(db->fetchRow())
    db->query("delete from trophy where player_id=%i", convertTo<int>((*db)["id"]));

}
