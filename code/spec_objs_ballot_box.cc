#include "stdsneezy.h"
#include "database.h"

// create table poll (poll_id int, descr varchar(127), status varchar(8));
// create table poll_option (option_id int, poll_id int, descr varchar(127));
// create table poll_vote (account varchar(80), poll_id int, option_id int);

int ballotBox(TBeing *ch, cmdTypeT cmd, const char *argument, TObj *o, TObj *)
{
  TDatabase db(DB_SNEEZY);
  sstring arg=argument;

  if(cmd==CMD_LIST){
    if(arg.empty()){
      /////////// list all polls
      ch->sendTo(COLOR_BASIC, fmt("You have a look at %s...\n\r") % 
		 o->getName());

      db.query("select poll_id, descr from poll where status='open' order by poll_id");
      
      if(db.fetchRow()){
	ch->sendTo(COLOR_BASIC, "These polls are open:\n\r");
	do {
	  ch->sendTo(COLOR_BASIC, fmt("%-2i| <r>%s<1>\n\r") %
		     convertTo<int>(db["poll_id"]) % db["descr"]);
	} while(db.fetchRow());
	ch->sendTo("\n\r");
      }

      db.query("select poll_id,descr from poll where status='closed' order by poll_id");

      if(db.fetchRow()){
	ch->sendTo(COLOR_BASIC, "These polls are closed:\n\r");
	do {
	  ch->sendTo(COLOR_BASIC, fmt("%-2i| <r>%s<1>\n\r") %
		     convertTo<int>(db["poll_id"]) % db["descr"]);
	} while(db.fetchRow());
	ch->sendTo("\n\r");
      }
    } else {
      //////////// list details of specified poll
      int which=convertTo<int>(arg);
      sstring status="open";

      db.query("select poll_id, descr, status from poll where poll_id=%i",
	       which);

      if(db.fetchRow()){
	ch->sendTo(COLOR_BASIC, fmt("%-2i| <r>%s<1>\n\r") %
		   convertTo<int>(db["poll_id"]) % db["descr"]);
	ch->sendTo(COLOR_BASIC, fmt("Poll is <r>%s<1>.\n\r\n\r") %
		   db["status"]);
	status=db["status"];
      }

      if(status=="open"){
	db.query("select option_id, descr from poll_option where poll_id=%i",
		 which);
	
	while(db.fetchRow()){
	  ch->sendTo(COLOR_BASIC, fmt("%-2i| <b>%s<1>\n\r") %
		     convertTo<int>(db["option_id"]) % db["descr"]);
	}
      } else if(status=="closed"){
	db.query("select pv.option_id as option_id, po.descr as descr, count(pv.option_id) as count from poll_vote pv, poll_option po where pv.poll_id=%i and pv.poll_id=po.poll_id and pv.option_id=po.option_id group by pv.option_id, po.descr order by count desc", which);
	
	while(db.fetchRow()){
	  ch->sendTo(COLOR_BASIC, fmt("%-2i| <b>%s<1> (%i votes)\n\r") %
		     convertTo<int>(db["option_id"]) % db["descr"] %
		     convertTo<int>(db["count"]));
	}
      }
    }
    return true;
  } else if(cmd==CMD_VOTE){

    return true;
  }


  return false;
}


int TBeing::doVote(const sstring &)
{
  sendTo(COLOR_BASIC, "There doesn't seem to be a ballot box around.\n\r");
  return true;
}
