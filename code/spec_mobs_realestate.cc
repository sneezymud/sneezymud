#include "stdsneezy.h"
#include "database.h"

extern bool bootHome(int, int, int, int, int, int, bool);

int realEstateAgent(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
{
  char tellbuf[1024], buf[1024], buf2[1024];
  TDatabase db("sneezy");

  if(cmd != CMD_LIST &&
     cmd != CMD_BUY)
    return FALSE;
  
  if(cmd == CMD_LIST){
    arg = one_argument(arg, buf2);
    
    if(!strcmp(buf2, "plan")){
      arg = one_argument(arg, buf2);

      db.query("select plan, cost, description from homeplans where plan='%s'", buf2);
            
      if(!db.fetchRow()){
	sprintf(tellbuf, "%s, That plan isn't available.", fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }
      
      sprintf(tellbuf, "%s, Plan %s costs %s talens.", 
	      fname(ch->name).c_str(), db.getColumn(0), db.getColumn(1));
      myself->doTell(tellbuf);
      
      sprintf(tellbuf, "%s, %s", fname(ch->name).c_str(), db.getColumn(2));
      myself->doTell(tellbuf);
    } else if(!*buf2){
      db.query("select plottype, plotnum from homeplots where homeowner is null order by plottype, plotnum");
      
      if(!db.fetchRow()){
	sprintf(tellbuf, "%s, I don't have any plots available.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }

      sprintf(tellbuf,"%s, I have these plots available:",fname(ch->name).c_str());

      do {
	sprintf(tellbuf+strlen(tellbuf)," %s%s", db.getColumn(0),db.getColumn(1));
      } while(db.fetchRow());

      myself->doTell(tellbuf);
    } else {
      db.query("select plottype, plotnum, (plot_end-plot_start)+1 nrooms, cost, description from homeplots where concat(plottype,plotnum)='%s'", buf2);
      
      if(!db.fetchRow()){
	sprintf(tellbuf, "%s, Plot %s%s is not available.",
		fname(ch->name).c_str(), db.getColumn(0), db.getColumn(1));
	myself->doTell(tellbuf);
	return TRUE;
      }
      
      sprintf(tellbuf, "%s, Plot %s%s is available.",
	      fname(ch->name).c_str(), db.getColumn(0), db.getColumn(1));
      myself->doTell(tellbuf);
      
      sprintf(tellbuf, "%s, Plot %s%s has %s rooms.",
	      fname(ch->name).c_str(), db.getColumn(0), db.getColumn(1), db.getColumn(2));
      myself->doTell(tellbuf);
      
      sprintf(tellbuf, "%s, Plot %s%s costs %s talens.",
	      fname(ch->name).c_str(), db.getColumn(0), db.getColumn(1), db.getColumn(3));
      myself->doTell(tellbuf);
      
      sprintf(tellbuf, "%s, %s",
	      fname(ch->name).c_str(), db.getColumn(4));
      myself->doTell(tellbuf);

      db.query("select plan from homeplans where plottype='%s'", db.getColumn(0));
            
      if(!db.fetchRow()){
	sprintf(tellbuf, "%s, I don't have any plans available for this plot.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }

      sprintf(tellbuf, "%s, These plans are available for this plot:",
	      fname(ch->name).c_str());
            
      do {
	sprintf(tellbuf+strlen(tellbuf), " %s", db.getColumn(0));
      } while(db.fetchRow());
      
      myself->doTell(tellbuf);
    }
  } else if(cmd == CMD_BUY){
    // buy <plot> <plan>  eq buy A2 0
    // create db entries
    arg = one_argument(arg, buf2); // plot
    arg = one_argument(arg, buf);  // plan

    if(!strcmp(buf2, "key")){
      // buy key <plot>
      db.query("select count(*) from homeplots where homeowner='%s' and concat(plottype,plotnum)='%s'", ch->getName(), buf);

      if(!db.fetchRow()){
	sprintf(tellbuf, "%s, I can't seem to find any information on that plot.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }

      if(atoi(db.getColumn(0))<=0){
	sprintf(tellbuf, "%s, You don't own that plot!",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }

      // load and give the key
      db.query("select keynum from homeplots where concat(plottype,plotnum)='%s'", buf);

      if(!db.fetchRow()){
	sprintf(tellbuf, "%s, Hmm, I can't seem to find the key for that plot.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }
      
      sprintf(tellbuf, "%s, Ok, here is another key for your home.",
	      fname(ch->name).c_str());
      myself->doTell(tellbuf);
      
      TObj *obj=read_object(atoi(db.getColumn(0)), VIRTUAL);
      *ch += *obj;
      act("$n gives you $p.", FALSE, myself, obj, ch, TO_VICT);
    } else {
      // check that plot is available
      db.query("select count(*) from homeplots plots, homeplans plans where plots.plottype=plans.plottype and concat(plots.plottype, plots.plotnum)='%s' and plans.plan='%s' and plots.homeowner is null", buf2, buf);

      if(!db.fetchRow()){
	sprintf(tellbuf, "%s, That plot or plan isn't available.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }
      
      if(atoi(db.getColumn(0))<=0 || !*buf2 || !*buf){
	sprintf(tellbuf, "%s, That plot or plan isn't available.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }
      
      // update homeplots
      db.query("update homeplots set homeowner='%s', plan=%s where concat(plottype,plotnum)='%s'", ch->getName(), buf, buf2);
      
      // load and give the key
      db.query("select keynum from homeplots where concat(plottype,plotnum)='%s'", buf2);

      if(!db.fetchRow()){
	sprintf(tellbuf, "%s, Hmm, I can't seem to find the key for that plot.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }
      
      sprintf(tellbuf, "%s, Your home will be built as quickly as possible.",
	      fname(ch->name).c_str());
      myself->doTell(tellbuf);
      sprintf(tellbuf, "%s, Congratulations on your purchase. Here is your key.",
	      fname(ch->name).c_str());
      myself->doTell(tellbuf);
      
      TObj *obj=read_object(atoi(db.getColumn(0)), VIRTUAL);
      *ch += *obj;
      act("$n gives you $p.", FALSE, myself, obj, ch, TO_VICT);
      
      int plot_start=0, plot_end=0, plan_i=0, keynum=0, flip, rotate;      

      db.query("select plan, plot_start, plot_end, keynum, flip, rotate from homeplots where concat(plottype,plotnum)='%s'", buf2);
      
      if(!db.fetchRow()){
	vlogf(LOG_BUG, "Problem with home construction in realEstateAgent");
	return TRUE;
      }
     
      plan_i=atoi(db.getColumn(0));
      plot_start=atoi(db.getColumn(1));
      plot_end=atoi(db.getColumn(2));
      keynum=atoi(db.getColumn(3));
      flip=atoi(db.getColumn(4));    
      rotate=atoi(db.getColumn(5));
      
      if(!bootHome(plan_i, plot_start, plot_end, keynum, flip, rotate, TRUE)){
	vlogf(LOG_BUG, "bootHome failed");
      }
    }
  }

  return TRUE;
}
