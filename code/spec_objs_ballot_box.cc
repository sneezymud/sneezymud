#include "stdsneezy.h"
#include "database.h"

// create table poll (poll_id int primary key, descr varchar(127), status varchar(8));
// create table poll_option (option_id int, poll_id int, descr varchar(127), primary key (option_id, poll_id));
// create table poll_vote (account varchar(80), poll_id int, option_id int, primary key (account, poll_id, option_id));

bool voteAdmin(TBeing *ch)
{
  if(ch->isImmortal())
    return true;
  // this shouldn't be hardcoded, but I'm too lazy to fix it proper
  if(!strcmp(ch->getName(), "Sidartha"))
    return true;
  return false;
}

int ballotBox(TBeing *ch, cmdTypeT cmd, const char *argument, TObj *o, TObj *)
{
  TDatabase db(DB_SNEEZY);

  if(cmd==CMD_LIST){
    sstring arg=argument;
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
    sstring arg=argument;
    sstring usage="Usage: vote <poll_id> <option_id>";
    int poll_id, option_id;
    sstring tmp;
    arg=one_argument(arg, tmp);

    if(tmp.empty()){
      if(!voteAdmin(ch)){
	ch->sendTo(fmt("%s\n\r") % usage);
      } else {
	ch->sendTo("Usage:\n\r");
	ch->sendTo("vote <poll_id> <option_id>\n\r");
	ch->sendTo("vote <poll_id> add <option_id> <descr>\n\r");
	ch->sendTo("vote <poll_id> remove <option_id>\n\r");
	ch->sendTo("vote add <poll_id> <descr>\n\r");
	ch->sendTo("vote close <poll_id>\n\r");
	//	ch->sendTo("vote open <poll_id>\n\r");
      }
      return true;
    } if(tmp == "add"){
      // vote add <poll_id> <descr>
      if(!voteAdmin(ch)){
	ch->sendTo(fmt("%s\n\r") % usage);
	return true;
      }

      arg=one_argument(arg, tmp);

      db.query("insert into poll values (%i, '%s', 'open')",
	       convertTo<int>(tmp), arg.c_str());
      ch->sendTo("Done.\n\r");
    } else if(tmp == "close"){
      // vote close <poll_id>
      if(!voteAdmin(ch)){
	ch->sendTo(fmt("%s\n\r") % usage);
	return true;
      }
      arg=one_argument(arg, tmp);

      db.query("update poll set status='closed' where poll_id=%i",
	       convertTo<int>(tmp));
      ch->sendTo("Done.\n\r");
#if 0
    } else if(tmp == "open"){
      // vote open <poll_id>
      if(!voteAdmin(ch)){
	ch->sendTo(fmt("%s\n\r") % usage);
	return true;
      }
      arg=one_argument(arg, tmp);

      db.query("update poll set status='open' where poll_id=%i",
	       convertTo<int>(tmp));
      ch->sendTo("Done.\n\r");
#endif
    } else {
      poll_id=convertTo<int>(tmp);
      arg=one_argument(arg, tmp);
      
      if(tmp == "add"){
	// vote <poll_id> add <option_id> <descr>
	if(!voteAdmin(ch)){
	  ch->sendTo(fmt("%s\n\r") % usage);
	  return true;
	}
	arg=one_argument(arg, tmp);
	option_id=convertTo<int>(tmp);

	db.query("insert into poll_option values (%i, %i, '%s')",
		 option_id, poll_id, arg.c_str());
	ch->sendTo("Done.\n\r");
      } else if(tmp == "remove"){
	// vote <poll_id> remove <option_id>
	if(!voteAdmin(ch)){
	  ch->sendTo(fmt("%s\n\r") % usage);
	  return true;
	}
	arg=one_argument(arg, tmp);
	option_id=convertTo<int>(tmp);

	db.query("delete from poll_option where option_id=%i",
		 option_id);
	ch->sendTo("Done.\n\r");
      } else {
	// vote <poll_id> <option_id>
	option_id=convertTo<int>(tmp);

	db.query("select 1 from poll_vote where poll_id=%i and account='%s'",
		 poll_id, ch->desc->account->name.c_str());

	if(db.fetchRow()){
	  ch->sendTo("You've already voted in that poll.\n\r");
	  return true;
	}

	db.query("select 1 from poll where poll_id=%i and status='open'",
		 poll_id);

	if(!db.fetchRow()){
	  ch->sendTo("That poll isn't open for voting.\n\r");
	  return true;
	}

	db.query("select 1 from poll_option where poll_id=%i and option_id=%i",
		 poll_id, option_id);

	if(!db.fetchRow()){
	  ch->sendTo("That doesn't seem to be an option in that poll.\n\r");
	  return true;
	}

	
	db.query("insert into poll_vote values ('%s', %i, %i)",
		 ch->desc->account->name.c_str(), poll_id, option_id);


	db.query("select po.descr as descr from poll_option po, poll_vote pv where po.poll_id=%i and po.option_id=%i and pv.poll_id=%i and pv.option_id=%i and pv.account='%s'", poll_id, option_id, poll_id, option_id, ch->desc->account->name.c_str());
	
	if(db.fetchRow()){
	  ch->sendTo(fmt("You cast your vote for %s.\n\r") % 
		     db["descr"]);
	} else {
	  ch->sendTo("There was an error - speak to an admin.\n\r");
	}
      }
    }

    return true;
  }


  return false;
}


int TBeing::doVote(const sstring &)
{
  sendTo(COLOR_BASIC, "There doesn't seem to be a ballot box around.\n\r");
  return true;
}
