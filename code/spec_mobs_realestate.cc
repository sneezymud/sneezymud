#include "stdsneezy.h"

extern bool bootHome(int, int, int, int, int, int, bool);

int realEstateAgent(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
{
  MYSQL_RES *res;
  MYSQL_ROW row;
  char tellbuf[1024], buf[1024], buf2[1024];
  int rc;

  if(cmd != CMD_LIST &&
     cmd != CMD_BUY)
    return FALSE;
  
  if(cmd == CMD_LIST){
    arg = one_argument(arg, buf2);
    
    if(!strcmp(buf2, "plan")){
      arg = one_argument(arg, buf2);
      
      if((rc=dbquery(&res, "sneezy", "realEstateAgent(1)", "select plan, cost, description from homeplans where plan='%s'", buf2))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in realEstateAgent");
	return FALSE;
      }
      
      if(!(row=mysql_fetch_row(res))){
	sprintf(tellbuf, "%s, That plan isn't available.", fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }
      
      sprintf(tellbuf, "%s, Plan %s costs %s talens.", 
	      fname(ch->name).c_str(), row[0], row[1]);
      myself->doTell(tellbuf);
      
      sprintf(tellbuf, "%s, %s", fname(ch->name).c_str(), row[2]);
      myself->doTell(tellbuf);
      
      mysql_free_result(res);
    } else if(!*buf2){
      if((rc=dbquery(&res, "sneezy", "realEstateAgent(1)", "select plottype, plotnum from homeplots where homeowner is null order by plottype, plotnum"))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in realEstateAgent");
	return FALSE;
      }
      
      if(!(row=mysql_fetch_row(res))){
	sprintf(tellbuf, "%s, I don't have any plots available.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }

      sprintf(tellbuf,"%s, I have these plots available:",fname(ch->name).c_str());

      do {
	sprintf(tellbuf+strlen(tellbuf)," %s%s", row[0],row[1]);
      } while((row=mysql_fetch_row(res)));

      myself->doTell(tellbuf);
      
      mysql_free_result(res);
    } else {
      if((rc=dbquery(&res, "sneezy", "realEstateAgent(2)", "select plottype, plotnum, (plot_end-plot_start)+1 nrooms, cost, description from homeplots where concat(plottype,plotnum)='%s'", buf2))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in realEstateAgent");
	return FALSE;
      }
      
      if(!(row=mysql_fetch_row(res))){
	sprintf(tellbuf, "%s, Plot %s%s is not available.",
		fname(ch->name).c_str(), row[0], row[1]);
	myself->doTell(tellbuf);
	return TRUE;
      }
      
      sprintf(tellbuf, "%s, Plot %s%s is available.",
	      fname(ch->name).c_str(), row[0], row[1]);
      myself->doTell(tellbuf);
      
      sprintf(tellbuf, "%s, Plot %s%s has %s rooms.",
	      fname(ch->name).c_str(), row[0], row[1], row[2]);
      myself->doTell(tellbuf);
      
      sprintf(tellbuf, "%s, Plot %s%s costs %s talens.",
	      fname(ch->name).c_str(), row[0], row[1], row[3]);
      myself->doTell(tellbuf);
      
      sprintf(tellbuf, "%s, %s",
	      fname(ch->name).c_str(), row[4]);
      myself->doTell(tellbuf);

      mysql_free_result(res);
      
      
      if((rc=dbquery(&res, "sneezy", "realEstateAgent(3)", "select plan from homeplans where plottype='%s'", row[0]))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in realEstateAgent");
	return TRUE;
      }
      
      if(!(row=mysql_fetch_row(res))){
	sprintf(tellbuf, "%s, I don't have any plans available for this plot.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }

      sprintf(tellbuf, "%s, These plans are available for this plot:",
	      fname(ch->name).c_str());
            
      do {
	sprintf(tellbuf+strlen(tellbuf), " %s", row[0]);
      } while((row=mysql_fetch_row(res)));
      
      myself->doTell(tellbuf);
      
      mysql_free_result(res);
      
    }
  } else if(cmd == CMD_BUY){
    // buy <plot> <plan>  eq buy A2 0
    // create db entries
    arg = one_argument(arg, buf2); // plot
    arg = one_argument(arg, buf);  // plan

    if(!strcmp(buf2, "key")){
      // buy key <plot>

      if((rc=dbquery(&res, "sneezy", "realEstateAgent", "select count(*) from homeplots where homeowner='%s' and concat(plottype,plotnum)='%s'", ch->getName(), buf))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in realEstateAgent");
	return FALSE;
      }
      if(!(row=mysql_fetch_row(res))){
	sprintf(tellbuf, "%s, I can't seem to find any information on that plot.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }

      if(atoi(row[0])<=0){
	sprintf(tellbuf, "%s, You don't own that plot!",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }
      mysql_free_result(res);


      // load and give the key
      if((rc=dbquery(&res, "sneezy", "realEstateAgent", "select keynum from homeplots where concat(plottype,plotnum)='%s'", buf))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in realEstateAgent");
	return FALSE;
      }
      if(!(row=mysql_fetch_row(res))){
	sprintf(tellbuf, "%s, Hmm, I can't seem to find the key for that plot.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }
      
      sprintf(tellbuf, "%s, Ok, here is another key for your home.",
	      fname(ch->name).c_str());
      myself->doTell(tellbuf);
      
      TObj *obj=read_object(atoi(row[0]), VIRTUAL);
      *ch += *obj;
      act("$n gives you $p.", FALSE, myself, obj, ch, TO_VICT);
      mysql_free_result(res);
    } else {
      // check that plot is available
      if((rc=dbquery(&res, "sneezy", "realEstateAgent", "select count(*) from homeplots plots, homeplans plans where plots.plottype=plans.plottype and concat(plots.plottype, plots.plotnum)='%s' and plans.plan='%s' and plots.homeowner is null", buf2, buf))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in realEstateAgent");
	return FALSE;
      }
      if(!(row=mysql_fetch_row(res))){
	sprintf(tellbuf, "%s, That plot or plan isn't available.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }
      
      if(atoi(row[0])<=0 || !*buf2 || !*buf){
	sprintf(tellbuf, "%s, That plot or plan isn't available.",
		fname(ch->name).c_str());
	myself->doTell(tellbuf);
	return TRUE;
      }
      mysql_free_result(res);
      
      // update homeplots
      if((rc=dbquery(NULL, "sneezy", "realEstateAgent", "update homeplots set homeowner='%s', plan=%s where concat(plottype,plotnum)='%s'", ch->getName(), buf, buf2))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in realEstateAgent");
	return FALSE;
      }
      
      // load and give the key
      if((rc=dbquery(&res, "sneezy", "realEstateAgent", "select keynum from homeplots where concat(plottype,plotnum)='%s'", buf2))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in realEstateAgent");
	return FALSE;
      }
      if(!(row=mysql_fetch_row(res))){
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
      
      TObj *obj=read_object(atoi(row[0]), VIRTUAL);
      *ch += *obj;
      act("$n gives you $p.", FALSE, myself, obj, ch, TO_VICT);
      
      int plot_start=0, plot_end=0, plan_i=0, keynum=0, flip, rotate;      
      if((rc=dbquery(&res, "sneezy", "bootHomes(1)", "select plan, plot_start, plot_end, keynum, flip, rotate from homeplots where concat(plottype,plotnum)='%s'", buf2))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in realEstateAgent");
	return TRUE;
      }
      
      if(!(row=mysql_fetch_row(res))){
	vlogf(LOG_BUG, "Problem with home construction in realEstateAgent");
	return TRUE;
      }
     
      plan_i=atoi(row[0]);
      plot_start=atoi(row[1]);
      plot_end=atoi(row[2]);
      keynum=atoi(row[3]);
      flip=atoi(row[4]);    
      rotate=atoi(row[5]);
      
      if(!bootHome(plan_i, plot_start, plot_end, keynum, flip, rotate, TRUE)){
	vlogf(LOG_BUG, "bootHome failed");
      }
      
      mysql_free_result(res);
    }
  }

  return TRUE;
}
