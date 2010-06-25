
//////////////////////////////////////////////////////////////////////////
//
//   SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//   "socket.cc" - All methods for TSocket class
//               
//
//////////////////////////////////////////////////////////////////////////

#include <csignal>
#include <cstdarg>
#include <errno.h>

extern "C" {
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/param.h>
#include <ares.h>

#ifdef SOLARIS
#include <sys/file.h>
#endif

int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);   
}

#include "room.h"
#include "monster.h"
#include "configuration.h"
#include "extern.h"
#include "statistics.h"
#include "database.h"
#include "spelltask.h"
#include "systemtask.h"
#include "socket.h"
#include "person.h"
#include "weather.h"
#include "colorstring.h"
#include "obj_gas.h"
#include "obj_vehicle.h"
#include "obj_trash_pile.h"
#include "obj_base_cup.h"
#include "pathfinder.h"
#include "timing.h"
#include "process.h"
#include "liquids.h"
#include "obj_pool.h"
#include "shop.h"
#include "shopaccounting.h"
#include "shopowned.h"
#include "obj_commodity.h"
#include "low.h"
#include "obj_tool.h"
#include "obj_plant.h"

int maxdesc, avail_descs;  
bool Shutdown = 0;               // clean shutdown
int tics = 0;
TMainSocket *gSocket;
long timeTill = 0;
ares_channel channel;
struct in_addr ares_addr;
int ares_status, nfds;
Descriptor *descriptor_list = NULL, *next_to_process; 

struct timeval timediff(struct timeval *a, struct timeval *b)
{
  struct timeval tmp, rslt;

  tmp = *a;

  if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0) {
    rslt.tv_usec += 1000000;
    --(tmp.tv_sec);
  }
  if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0) {
    rslt.tv_usec = 0;
    rslt.tv_sec = 0;
  }
  return rslt;
}

void TMainSocket::addNewDescriptorsDuringBoot(sstring tStString)
{
  fd_set input_set, output_set, exc_set;
  static struct timeval null_time;
  Descriptor *point;
  static bool been_called = false;
  static sigset_t mask;


  if (!been_called) {
    // prepare the time values 
    null_time.tv_sec = 0;
    null_time.tv_usec = 0;

    maxdesc=0;
    for(unsigned int i=0;i<m_sock.size();++i)
      maxdesc = max(maxdesc, m_sock[i]);

    avail_descs = 150;

    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGPIPE);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGURG);
    sigaddset(&mask, SIGXCPU);
    sigaddset(&mask, SIGHUP);
    // don't trap SIG_PROF, it is needed for debugging.
    
    been_called = true;
  }

  // Check what's happening out there 
  FD_ZERO(&input_set);
  FD_ZERO(&output_set);
  FD_ZERO(&exc_set);

  for(unsigned int i=0;i<m_sock.size();++i)
    FD_SET(m_sock[i], &input_set);

  for (point = descriptor_list; point; point = point->next) {
    FD_SET(point->socket->m_sock, &input_set);
    FD_SET(point->socket->m_sock, &exc_set);
    FD_SET(point->socket->m_sock, &output_set);
  }


  sigprocmask(SIG_SETMASK, &mask, NULL);
  
#ifdef LINUX
  // linux uses a nonstandard style of "timedout" (the last parm of select)
  // it gets hosed each select() so must be reinited here
  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
#endif
  if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
    perror("Error in Select (poll)");
    return;
  }


  sigprocmask(SIG_UNBLOCK, &mask, NULL);


  // establish any new connections 
  for(unsigned int i=0;i<m_sock.size();++i){
    if (FD_ISSET(m_sock[i], &input_set)) {
      int tFd;
      
      if ((tFd = newDescriptor(m_sock[i], m_port[i])) < 0)
	perror("New connection");
      else if (!tStString.empty() && tFd)
	descriptor_list->writeToQ(tStString);
    }
  }
  // close any connections with an exceptional condition pending 
  for (point = descriptor_list; point; point = next_to_process) {
    next_to_process = point->next;
    if (FD_ISSET(point->socket->m_sock, &exc_set)) {
      FD_CLR(point->socket->m_sock, &input_set);
      FD_CLR(point->socket->m_sock, &output_set);
      delete point;
    }
  }
  // read any incoming input, and queue it up 
  for (point = descriptor_list; point; point = next_to_process) {
    next_to_process = point->next;
    if (FD_ISSET(point->socket->m_sock, &input_set)) {
      if (point->inputProcessing() < 0) {
        delete point;
        point = NULL;
      }
    }
  }
  ////////////////////////////////////////////
  // process async dns queries
  ////////////////////////////////////////////
  fd_set read_fds, write_fds;
  struct timeval *tvp, tv;

  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  nfds = ares_fds(channel, &read_fds, &write_fds);
  if (nfds > 0) {
    tvp = ares_timeout(channel, NULL, &tv);
    select(nfds, &read_fds, &write_fds, NULL, tvp);
    ares_process(channel, &read_fds, &write_fds);
  }
}



// updates the data in the wholist table in the database
// returns the count of players logged in now
int updateWholist()
{
  Descriptor *p;
  TPerson *p2;
  int count = 0;
  static int last_count;
  static sstring wholist_last;
  sstring wholist = "";

  for (p = descriptor_list; p; p = p->next) {
    if (p && p->connected == CON_PLYNG || p->connected > MAX_CON_STATUS && p->character && 
	p->character->name && p->character->isPc() && !p->character->isLinkdead() && p->character->polyed == POLY_TYPE_NONE) {
      if ((p2 = dynamic_cast<TPerson *>(p->character))) {
	wholist += p2->getName();
      }
    }
  }


  if (wholist != wholist_last) {
    // every 10 RL seconds
    TDatabase db(DB_SNEEZY);
    
    
    db.query("delete from wholist where port=%i", gamePort);
    
    //  vlogf(LOG_DASH, format("Updating who table for port %d") %  gamePort);
  for (p = descriptor_list; p; p = p->next) {
    if (p && p->connected == CON_PLYNG || p->connected > MAX_CON_STATUS && p->character &&
        p->character->name && p->character->isPc() && !p->character->isLinkdead() && p->character->polyed == POLY_TYPE_NONE) {
      if ((p2 = dynamic_cast<TPerson *>(p->character))) {
	  db.query("insert into wholist (name, title, port, invis) VALUES('%s', '%s', %i, %i)", p2->getName(), p2->title,  gamePort, (p2->getInvisLevel() >MAX_MORT)?1:0);
	  count++;
	}
      }
    }
  } else {
    return last_count;
  }
  wholist_last = wholist;
  last_count = count;
  return count;
}

// updates the usagelogs table in the database
// takes the count of players currently logged on
void updateUsagelogs(int count)
{
  time_t ct=time(0);
  static time_t logtime;
  static time_t lastlog;

  int TIME_BETWEEN_LOGS = 300;
  
  // every 10 RL seconds
  TDatabase db(DB_SNEEZY);




  if(logtime/TIME_BETWEEN_LOGS < ct/TIME_BETWEEN_LOGS) {
    //	vlogf(LOG_DASH, format("Webstuff: collecting game usage data - %d seconds since last log") %  ct-lastlog);
    //        vlogf(LOG_DASH, format("Webstuff:  logtime = %d,  ct = %d, players = %d") %  logtime % ct % count);
    
    
    if (logtime != 0) logtime += TIME_BETWEEN_LOGS;
    else logtime = ct;
    lastlog = ct;
    db.query("insert into usagelogs (time, players, port) VALUES(%i, %i, %i)", logtime, count, gamePort);
    // delete logs older than two months
    db.query("insert into usagelogsarchive select * from usagelogs where port=%i and time>%i", gamePort, logtime + 5184000);
    db.query("delete from usagelogs where port=%i and time>%i", gamePort, logtime + 5184000);
  }
}


// procWholistAndUsageLogs
procWholistAndUsageLogs::procWholistAndUsageLogs(const int &p)
{
  trigger_pulse=p;
  name="procWholistAndUsageLogs";
}

void procWholistAndUsageLogs::run(const TPulse &) const
{
  int count=updateWholist();
  updateUsagelogs(count);
}



// procNukeInactiveMobs
procNukeInactiveMobs::procNukeInactiveMobs(const int &p)
{
  trigger_pulse=p;
  name="procNukeInactiveMobs";
}

void procNukeInactiveMobs::run(const TPulse &) const
{
  unsigned int i;

  if(!Config::NukeInactiveMobs())
    return;

  for (i = 0; i < zone_table.size(); i++) {
    if (!zone_table[i].isEmpty())
      continue;
    if (zone_table[i].zone_value == 1)
      zone_table[i].nukeMobs();
    if (zone_table[i].zone_value > 0) {
      zone_table[i].zone_value -= 1;
    }
  }
}


// procUpdateAvgPlayers
procUpdateAvgPlayers::procUpdateAvgPlayers(const int &p)
{
  trigger_pulse=p;
  name="procUpdateAvgPlayers";
}

void procUpdateAvgPlayers::run(const TPulse &) const
{
// update the average players displayed in "who"
  // statistics stuff
  if (time(0) - stats.useage_timer > (1 * SECS_PER_REAL_MIN)) {
    // figure out average user load
    stats.useage_timer = time(0);
    stats.useage_iters++;
    Descriptor *d;
    for (d = descriptor_list; d; d = d->next) {
      if (d->connected == CON_PLYNG || d->connected > MAX_CON_STATUS)
	stats.num_users++;
    }
  }
}


////////////////////////////////////////////
// handle shutdown
////////////////////////////////////////////
bool TMainSocket::handleShutdown()
{
  sstring buf;
  static bool sent = false;

  if(Shutdown)
    return true;

  if (timeTill  && (timeTill <= time(0))) {
    if (descriptor_list) {
      buf=format("%s time has arrived!\n\r") % shutdown_or_reboot();
      descriptor_list->worldSend(buf, NULL);
      descriptor_list->outputProcessing();
    }
    return true;
  } else if (timeTill && !((timeTill - time(0)) % 60)) {
    int minutes=(timeTill - time(0)) / 60;
    if (!sent) {
      buf="<r>******* SYSTEM MESSAGE ******<z>\n\r";
      buf+=format("<c>%s in %ld minute%s.<z>\n\r") % 
	shutdown_or_reboot() % minutes % ((minutes == 1) ? "" : "s");
      descriptor_list->worldSend(buf, NULL);
    }
    sent = true;
  } else if (timeTill && ((timeTill- time(0)) <= 5)) {
    long secs = timeTill - time(0);
    if (!sent) {
      buf="<r>******* SYSTEM MESSAGE ******<z>\n\r";
      buf+=format("<c>%s in %ld second%s.<z>\n\r") %
	shutdown_or_reboot() % secs % ((secs == 1) ? "" : "s");
      descriptor_list->worldSend(buf, NULL);
      sent = true;
    }
  } else
    sent = false;

  return false;
}


// this function handles time regulation, new socket connections,
// queueing up socket input, and prompt displaying
struct timeval TMainSocket::handleTimeAndSockets()
{
  fd_set input_set, output_set, exc_set;
  static struct timeval last_time;
  struct timeval now, timespent, timeout, null_time, opt_time;
  Descriptor *point;
  static sigset_t mask;

  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  opt_time.tv_usec = OPT_USEC;
  opt_time.tv_sec = 0;
  
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGUSR2);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGPIPE);
  sigaddset(&mask, SIGTERM);
  sigaddset(&mask, SIGURG);
  sigaddset(&mask, SIGXCPU);
  sigaddset(&mask, SIGHUP);
  // don't trap SIG_PROF, it is needed for debugging.

  ////////////////////////////////////////////
  // do some socket stuff or something
  ////////////////////////////////////////////
  // Check what's happening out there 
  FD_ZERO(&input_set);
  FD_ZERO(&output_set);
  FD_ZERO(&exc_set);

  for(unsigned int i=0;i<m_sock.size();++i)
    FD_SET(m_sock[i], &input_set);

  for (point = descriptor_list; point; point = point->next) {
    FD_SET(point->socket->m_sock, &input_set);
    FD_SET(point->socket->m_sock, &exc_set);
    FD_SET(point->socket->m_sock, &output_set);
  }
  ////////////////////////////////////////////
  ////////////////////////////////////////////

  ////////////////////////////////////////////
  // do some time related stuff
  ////////////////////////////////////////////
  // check out the time 
  gettimeofday(&now, NULL);
  timespent=timediff(&now, &last_time);
  timeout=timediff(&opt_time, &timespent);


  last_time.tv_sec = now.tv_sec + timeout.tv_sec;
  last_time.tv_usec = now.tv_usec + timeout.tv_usec;
  if (last_time.tv_usec >= 1000000) {
    last_time.tv_usec -= 1000000;
    last_time.tv_sec++;
  }

  sigprocmask(SIG_SETMASK, &mask, NULL);

  // this gets our list of socket connections that are ready for handling
  if(select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0){
    perror("Error in Select (poll)");
    exit(-1);
  }

  // this regulates the speed of the mud
  sleeptime=timeout;

  // this isn't working under linux
  //  if (select(0, 0, 0, 0, &timeout) < 0) {
  //    perror("Error in select (sleep)");
  //  }

  sigprocmask(SIG_UNBLOCK, &mask, NULL);

  ////////////////////////////////////////////
  ////////////////////////////////////////////

  ////////////////////////////////////////////
  // establish any new connections 
  ////////////////////////////////////////////
  for(unsigned int i=0;i<m_sock.size();++i){
    if (FD_ISSET(m_sock[i], &input_set)) {
      int rc = newDescriptor(m_sock[i], m_port[i]);
      if (rc < 0)
	perror("New connection");
      else if (rc) {
	// we created a new descriptor
	// so send the login to the first desc in list
	if (!descriptor_list->m_bIsClient)
	  descriptor_list->sendLogin("1");
      }
    }
  }
  ////////////////////////////////////////////
  ////////////////////////////////////////////

  ////////////////////////////////////////////
  // process async dns queries
  ////////////////////////////////////////////
  fd_set read_fds, write_fds;
  struct timeval *tvp, tv;

  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  nfds = ares_fds(channel, &read_fds, &write_fds);
  if (nfds > 0) {
    tvp = ares_timeout(channel, NULL, &tv);
    select(nfds, &read_fds, &write_fds, NULL, tvp);
    ares_process(channel, &read_fds, &write_fds);
  }
  ////////////////////////////////////////////
  ////////////////////////////////////////////

  ////////////////////////////////////////////
  // close any connections with an exceptional condition pending 
  ////////////////////////////////////////////
  for (point = descriptor_list; point; point = next_to_process) {
    next_to_process = point->next;
    if (FD_ISSET(point->socket->m_sock, &exc_set)) {
      FD_CLR(point->socket->m_sock, &input_set);
      FD_CLR(point->socket->m_sock, &output_set);
      delete point;
    }
  }
  ////////////////////////////////////////////
  ////////////////////////////////////////////

  ////////////////////////////////////////////
  // read any incoming input, and queue it up 
  ////////////////////////////////////////////
  for (point = descriptor_list; point; point = next_to_process) {
    next_to_process = point->next;
    if (FD_ISSET(point->socket->m_sock, &input_set)) {
      if (point->inputProcessing() < 0) {
	delete point;
	point = NULL;
      }
    }
  }
  processAllInput();
  setPrompts(output_set);
  afterPromptProcessing(output_set);
  ////////////////////////////////////////////
  ////////////////////////////////////////////

  return timeout;
}

void pulseLog(sstring name, TTiming timer, int pulse)
{
  if(!toggleInfo[TOG_GAMELOOP]->toggle)
    return;

  vlogf(LOG_MISC, format("%i %i) %s: %i") % 
	(pulse % 2400) % (pulse%12) % name % 
	(int)(timer.getElapsedReset()*1000000));
}





// procPingData
procPingData::procPingData(const int &p)
{
  trigger_pulse=p;
  name="procPingData";
}

void procPingData::run(const TPulse &) const
{
  static FILE *p;
  Descriptor *d;
  
  if(p) pclose(p);
  
  if(gamePort == Config::Port::PROD){
    p=popen("/mud/prod/lib/bin/ping sneezy", "w");
  } else if(gamePort == Config::Port::BUILDER){
    p=popen("/mud/prod/lib/bin/ping sneezybuilder", "w");
  } else {
    p=popen("/mud/prod/lib/bin/ping sneezybeta", "w");
  }
  
  
  for (d = descriptor_list; d; d = d->next) {
    if (!(d->host.empty()) && d->character && d->character->isPlayerAction(PLR_PING)){
      fprintf(p, "%s\n", d->host.c_str());
    }
  }
  fprintf(p, "EOM\n");
  fflush(p);
}


// procSetZoneEmpty
procSetZoneEmpty::procSetZoneEmpty(const int &p)
{
  trigger_pulse=p;
  name="procSetZoneEmpty";
}

void procSetZoneEmpty::run(const TPulse &) const 
{
  // set zone emptiness flags
  for (unsigned int i = 0; i < zone_table.size(); i++)
    zone_table[i].zone_value=zone_table[i].isEmpty()?1:-1;
}



// procMobHate
procMobHate::procMobHate(const int &p)
{
  trigger_pulse=p;
  name="procMobHate";
}

void procMobHate::run(const TPulse &) const
{
  TBeing * b = NULL;
  
  for (b = character_list; b; b = b->next) {
    TMonster *tmons = dynamic_cast<TMonster *>(b);
    charList *list;
    
    if (tmons && IS_SET(tmons->hatefield, HATE_CHAR) && tmons->hates.clist){
      for (list = tmons->hates.clist; list; list = list->next){
	if (list->name) {
	  list->iHateStrength--;
	  
	  if (list->iHateStrength <= 0) {
	    vlogf(LOG_LAPSOS, format("%s no longer hates %s") % 
		  tmons->getName() % list->name);
	    
	    tmons->remHated(NULL, list->name);
	  }
	}
      }
    }
  }
}


// Mudadmin Resolution, April 19th 2005
// 217.) Global load rates will be lowered by 1% per day for 3
// consecutive months, achieving an end result of 1/10th of the
// former rates. An additional period of 3 months with no global
// load rate changes will be observed in order to monitor the
// results of this change.
//
// The global load rate modifier is currently at 0.46, so we will
// decrease it by 1% (0.0046) per real day. until we arrive at 0.0460
// but we'll do it per hour (.00019166666666666666)

// procTweakLoadRate
procTweakLoadRate::procTweakLoadRate(const int &p)
{
  trigger_pulse=p;
  name="procTweakLoadRate";
}

void procTweakLoadRate::run(const TPulse &) const
{
  if(stats.equip <= 0.046){
    vlogf(LOG_BUG, "procTweakLoadRate: desired load rate achieved.");
    return;
  }

  stats.equip -= 0.00019166666666666666;
  save_game_stats();
  vlogf(LOG_LOW, format("procTweakLoadRate: adjusted load rate to %f") %
	stats.equip);
}

procCheckTriggerUsers::procCheckTriggerUsers(const int &p)
{
  trigger_pulse=p;
  name="procCheckTriggerUsers";
}

void procCheckTriggerUsers::run(const TPulse &) const
{
  Descriptor *d;
  sstring buf;
  std::vector <sstring> str;
  std::map <sstring, unsigned char> cmds;
  unsigned char c='a';
  int count=0;

  for (d = descriptor_list; d; d = d->next) {
    if(!d->character || d->connected)
      continue;

    count=0;
    c='a';
    cmds.clear();
    str.clear();

    // assign a byte code to each type of command
    for(int i=0;i<128;++i){
      if(!d->history[i][0])
	continue;
      if(cmds.find(d->history[i]) == cmds.end()){
	cmds[d->history[i]]=c++;
      }
      count++;
    }

    // don't bother if they don't have enough command history
    if(count < 100)
      continue;

    vlogf(LOG_MISC, format("procCheckTriggerUsers: %s has %i unique commands") % 
	  d->character->getName() % (c-'a'));

    unsigned int longest=0;

    buf="";
    for(unsigned int i=0;i<128;++i){
      if(!d->history[i][0])
	continue;
      
      buf+=cmds[d->history[i]];
      
      if(find(str.begin(), str.end(), buf) == str.end()){
	if(buf.length() > longest)
	  longest=buf.length();
	str.push_back(buf);
	buf="";
      }
    }

    vlogf(LOG_MISC, format("procCheckTriggerUsers: %s has longest pattern of %i") % 
	  d->character->getName() % longest);
  }
}

procCloseAccountingBooks::procCloseAccountingBooks(const int &p)
{
  trigger_pulse=p;
  name="procCloseAccountingBooks";
}

void procCloseAccountingBooks::run(const TPulse &) const
{
  // close out the accounting year.
  TDatabase db(DB_SNEEZY);
  db.query("select distinct shop_nr from shoplogjournal where sneezy_year=%i order by shop_nr", GameTime::getYear()-1);
  while (db.fetchRow()){
      TShopJournal tsj(convertTo<int>(db["shop_nr"]), GameTime::getYear()-1);
      tsj.closeTheBooks();
  }
}

procRecordCommodPrices::procRecordCommodPrices(const int &p)
{
  trigger_pulse=p;
  name="procRecordCommodPrices";
}

void procRecordCommodPrices::run(const TPulse &) const
{
#if 0
  TDatabase db(DB_SNEEZY);

  db.query("select shop_nr from shoptype where type=%i", 
	   ITEM_RAW_MATERIAL);

  while(db.fetchRow()){
    unsigned int shop_nr=convertTo<int>(db["shop_nr"]);
    TShopOwned tso(shop_nr, NULL);
    TCommodity *commod;
    
    // REVIEW: getStuff() should be returning NULL, since all shop objs are in database
    for(TThing *t=tso.getStuff();t;t=t->nextThing){
      if((commod=dynamic_cast<TCommodity *>(t))){
	db.query("insert into commodprices values (now(), %i, %i, %f)",
		 shop_nr, commod->getMaterial(), 
		 commod->shopPriceFloat(1, shop_nr, -1, NULL));
      }
    }
  }
#endif
}



procWeightVolumeFumble::procWeightVolumeFumble(const int &p)
{
  trigger_pulse=p;
  name="procWeightVolumeFumble";
}

void procWeightVolumeFumble::run(const TPulse &) const
{
  Descriptor *d;
  TBeing *ch;
  TThing *t;
  int loop_check=0;

  for (d = descriptor_list; d; d = d->next) {
    if(!(ch=d->character) || d->connected || 
       !d->character->roomp || d->character->isImmortal())
      continue;

    if(ch->getCarriedVolume() > ch->carryVolumeLimit() ||
       ch->getCarriedWeight() > ch->carryWeightLimit())
      ch->sendTo("You are carrying too much and lose control of your inventory!\n\r");

    loop_check=0;
    
    while(ch->getCarriedVolume() > ch->carryVolumeLimit() ||
	  ch->getCarriedWeight() > ch->carryWeightLimit()){
      t=ch->stuff.front();

      ch->doDrop("", t, true);

      if(ch->stuff.front() == t){
	// drop failed; cursed or something
	ch->stuff.pop_front();
	ch->stuff.push_back(t);
      }

      if(++loop_check > 100)
	break;
    }
  }
}


procObjVehicle::procObjVehicle(const int &p)
{
  trigger_pulse=p;
  name="procObjVehicle";
}

bool procObjVehicle::run(const TPulse &pl, TObj *obj) const
{
  TVehicle *vehicle;

  // vehicle movement
  if((vehicle=dynamic_cast<TVehicle *>(obj)))
    vehicle->vehiclePulse(pl.pulse);

  return false;
}

procObjDetonateGrenades::procObjDetonateGrenades(const int &p)
{
  trigger_pulse=p;
  name="procObjDetonateGrenades";
}

bool procObjDetonateGrenades::run(const TPulse &pl, TObj *obj) const
{
  // this stuff all happens every time we go through here, which is
  // about every 12 pulses, ie "combat" or "teleport" pulse
  int rc = obj->detonateGrenade();
  if (IS_SET_DELETE(rc, DELETE_THIS)) 
    return true;
  return false;
}

procObjFalling::procObjFalling(const int &p)
{
  trigger_pulse=p;
  name="procObjFalling";
}

bool procObjFalling::run(const TPulse &pl, TObj *obj) const
{
  int rc = obj->checkFalling();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}

procObjRiverFlow::procObjRiverFlow(const int &p)
{
  trigger_pulse=p;
  name="procObjRiverFlow";
}

bool procObjRiverFlow::run(const TPulse &pl, TObj *obj) const
{
  int rc = obj->riverFlow(pl.pulse);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}

procObjTeleportRoom::procObjTeleportRoom(const int &p)
{
  trigger_pulse=p;
  name="procObjTeleportRoom";
}

bool procObjTeleportRoom::run(const TPulse &pl, TObj *obj) const
{
  int rc = obj->teleportRoomFlow(pl.pulse);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}

procObjSpecProcsQuick::procObjSpecProcsQuick(const int &p)
{
  trigger_pulse=p;
  name="procObjSpecProcsQuick";
}

bool procObjSpecProcsQuick::run(const TPulse &pl, TObj *obj) const
{
  if (obj->spec) {
    int rc = obj->checkSpec(NULL, CMD_GENERIC_QUICK_PULSE, "", NULL);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return true;
  }
  return false;
}


procObjTickUpdate::procObjTickUpdate(const int &p)
{
  trigger_pulse=p;
  name="procObjTickUpdate";
}

bool procObjTickUpdate::run(const TPulse &pl, TObj *obj) const
{
  int rc = obj->objectTickUpdate(pl.pulse);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}


procObjRust::procObjRust(const int &p)
{
  trigger_pulse=p;
  name="procObjRust";
}

bool procObjRust::run(const TPulse &pl, TObj *obj) const
{
  // rust
  if(!::number(0,99) && obj->canRust()){
    TRoom *rp=NULL;
    
    if(obj->equippedBy)
      rp=obj->equippedBy->roomp;
    
    if(dynamic_cast<TBeing *>(obj->parent))
      rp=obj->parent->roomp;
    
    if(obj->roomp)
      rp=obj->roomp;
    
    if(rp && (Weather::getWeather(*rp)==Weather::RAINY ||
	      Weather::getWeather(*rp)==Weather::LIGHTNING ||
	      rp->isWaterSector())){
      obj->addObjStat(ITEM_RUSTY);
    }
  }
  return false;
}
      
procObjFreezing::procObjFreezing(const int &p)
{
  trigger_pulse=p;
  name="procObjFreezing";
}

bool procObjFreezing::run(const TPulse &pl, TObj *obj) const
{
  // freezing
  // find base cups that are either in an arctic room, or in the inventory
  // of a being in an arctic room, with < 10 drunk
  // note we're avoid frostEngulfed() because it is a bit extreme for this
  // thawing is done with thawEngulfed() in characterPulse
  TBaseCup *cup=dynamic_cast<TBaseCup *>(obj);
  if(cup){
    TRoom *r=NULL;
    TThing *t;
    TBeing *ch=NULL;
    
    if((t = cup->equippedBy) || (t = cup->parent)){
      ch = dynamic_cast<TBeing *>(t);
      if(ch)
	r=ch->roomp;
    } else
      r = cup->roomp;
    
    if(r && (!ch || !ch->affectedBySpell(AFFECT_WAS_INDOORS))){
      if(r->isArcticSector() && cup->getDrinkUnits() > 0 && 
	 cup->getLiqDrunk() < 7 && !cup->isDrinkConFlag(DRINK_FROZEN)){
	int rc=cup->freezeObject(ch, 0);
	if (IS_SET_DELETE(rc, DELETE_THIS))
	  return true;
	
	// freeze any pools that were dropped
	TPool *tp;
	for(StuffIter it=r->stuff.begin();it!=r->stuff.end() && (t=*it);++it){
	  if((tp=dynamic_cast<TPool *>(t)) && tp->getLiqDrunk() < 7 &&
	     !tp->isDrinkConFlag(DRINK_FROZEN))
	    tp->freezeObject(ch, 0);
	}
      }
    }
    
    if(cup->roomp && !cup->roomp->isArcticSector() &&
       cup->isDrinkConFlag(DRINK_FROZEN)){
      cup->thawObject(ch, 0);
    }
  }
  return false;
}
 
procObjAutoPlant::procObjAutoPlant(const int &p)
{
  trigger_pulse=p;
  name="procObjAutoPlant";
}

bool procObjAutoPlant::run(const TPulse &pl, TObj *obj) const
{     
  // seeds sitting on the ground will sometimes auto-plant themselves
  TTool *seed=dynamic_cast<TTool *>(obj);
  if(seed && seed->getToolType()==TOOL_SEED &&
     !::number(0,100) &&
     seed->roomp &&
     !(seed->roomp->isFallSector() || 
       seed->roomp->isWaterSector() || 
       seed->roomp->isIndoorSector() || 
       seed->roomp->isUnderwaterSector())){
    
    int count=0;
    for(StuffIter it=seed->roomp->stuff.begin();
	it!=seed->roomp->stuff.end();++it){
      if(dynamic_cast<TPlant *>(*it))
	++count;
    }    
    
    if(count<8){
      TObj *tp;
      TPlant *tplant;
      tp = read_object(Obj::GENERIC_PLANT, VIRTUAL);
      if((tplant=dynamic_cast<TPlant *>(tp))){
	tplant->setType(seed_to_plant(obj->objVnum()));
	tplant->updateDesc();
	
	*seed->roomp += *tp;
	
	seed->addToToolUses(-1);
	
	if (seed->getToolUses() <= 0) 
	  return true;
      }
    }
  }      
  return false;
}

procObjSmoke::procObjSmoke(const int &p)
{
  trigger_pulse=p;
  name="procObjSmoke";
}

bool procObjSmoke::run(const TPulse &pl, TObj *obj) const
{
  // fun with smoke
  TGas *gas=dynamic_cast<TGas *>(obj);
  if (gas){
    gas->doDrift();
    gas->doSpecials();
  }
  return false;
}

procObjPools::procObjPools(const int &p)
{
  trigger_pulse=p;
  name="procObjPools";
}

bool procObjPools::run(const TPulse &pl, TObj *obj) const
{
  // fun with pools
  TPool *pool=dynamic_cast<TPool *>(obj);
  if(pool)
    pool->overFlow();

  return false;
}

procObjTrash::procObjTrash(const int &p)
{
  trigger_pulse=p;
  name="procObjTrash";
}

bool procObjTrash::run(const TPulse &pl, TObj *obj) const
{
  // trash piles
  if(!::number(0,999))
    obj->joinTrash();
  
  TTrashPile *pile=dynamic_cast<TTrashPile *>(obj);
  if(pile){
    // delete empty piles
    if(pile->stuff.empty()){
      return true;
    } else {
      pile->overFlow();
      pile->updateDesc();
      pile->doDecay();
    }
  }
  return false;
}


procObjSinking::procObjSinking(const int &p)
{
  trigger_pulse=p;
  name="procObjSinking";
}

bool procObjSinking::run(const TPulse &pl, TObj *obj) const
{
  // sinking
  check_sinking_obj(obj, obj->in_room);
  return false;
}

procObjSpecProcs::procObjSpecProcs(const int &p)
{
  trigger_pulse=p;
  name="procObjSpecProcs";
}

bool procObjSpecProcs::run(const TPulse &pl, TObj *obj) const
{
  // procs
  if (obj->spec) {
    int rc = obj->checkSpec(NULL, CMD_GENERIC_PULSE, "", NULL);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return true;
  }
  return false;
}

procObjBurning::procObjBurning(const int &p)
{
  trigger_pulse=p;
  name="procObjBurning";
}

bool procObjBurning::run(const TPulse &pl, TObj *obj) const
{
  // burning
  int rc = obj->updateBurning();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}



procCharSpecProcs::procCharSpecProcs(const int &p)
{
  trigger_pulse=p;
  name="procCharSpecProcs";
}

bool procCharSpecProcs::run(const TPulse &pl, TBeing *tmp_ch) const
{
  if (tmp_ch->spec) {
    int rc = tmp_ch->checkSpec(tmp_ch, CMD_GENERIC_PULSE, "", NULL);
    if (IS_SET_DELETE(rc, DELETE_THIS)) 
      return true;
  }
  return false;
}

procCharDrowning::procCharDrowning(const int &p)
{
  trigger_pulse=p;
  name="procCharDrowning";
}

bool procCharDrowning::run(const TPulse &pl, TBeing *tmp_ch) const
{
  int rc = tmp_ch->checkDrowning();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}

procCharResponses::procCharResponses(const int &p)
{
  trigger_pulse=p;
  name="procCharResponses";
}

bool procCharResponses::run(const TPulse &pl, TBeing *tmp_ch) const
{
  TMonster *tmon = dynamic_cast<TMonster *>(tmp_ch);
  if(tmon){
    tmon->checkResponses((tmon->opinion.random ? tmon->opinion.random : 
			  (tmon->targ() ? tmon->targ() : tmon)),
			 NULL, NULL, CMD_RESP_PULSE);
    
  }
  return false;
}

procCharSinking::procCharSinking(const int &p)
{
  trigger_pulse=p;
  name="procCharSinking";
}

bool procCharSinking::run(const TPulse &pl, TBeing *tmp_ch) const
{
  if (toggleInfo[TOG_GRAVITY]->toggle)
    tmp_ch->checkSinking(tmp_ch->in_room);
  return false;
}

procCharFalling::procCharFalling(const int &p)
{
  trigger_pulse=p;
  name="procCharFalling";
}

bool procCharFalling::run(const TPulse &pl, TBeing *tmp_ch) const
{
  if (toggleInfo[TOG_GRAVITY]->toggle){
    int rc = tmp_ch->checkFalling();
    if (IS_SET_DELETE(rc, DELETE_THIS)) 
      return true;
  }
  return false;
}
	
procCharMobileActivity::procCharMobileActivity(const int &p)
{
  trigger_pulse=p;
  name="procCharMobileActivity";
}

bool procCharMobileActivity::run(const TPulse &pl, TBeing *tmp_ch) const
{
  if (!tmp_ch->isPc() && dynamic_cast<TMonster *>(tmp_ch) &&
      (zone_table[tmp_ch->roomp->getZoneNum()].zone_value!=1 || 
       tmp_ch->isShopkeeper() || 
       IS_SET(tmp_ch->specials.act, ACT_HUNTING))){
    int rc = dynamic_cast<TMonster *>(tmp_ch)->mobileActivity(pl.pulse);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return true;
  }
  return false;
}

procCharTasks::procCharTasks(const int &p)
{
  trigger_pulse=p;
  name="procCharTasks";
}

bool procCharTasks::run(const TPulse &pl, TBeing *tmp_ch) const
{
  if (tmp_ch->task && (pl.pulse >= tmp_ch->task->nextUpdate)) {
    TObj *tmper_obj = NULL;
    if (tmp_ch->task->obj) {
      tmper_obj = tmp_ch->task->obj; 
    } 
    int rc = (*(tasks[tmp_ch->task->task].taskf))
      (tmp_ch, CMD_TASK_CONTINUE, "", pl.pulse, tmp_ch->task->room, tmp_ch->task->obj);
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
      if (tmper_obj) {
	delete tmper_obj;
	tmper_obj = NULL;
      } else {
	vlogf(LOG_BUG, "bad item delete in gameloop -- task calling");
      }
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return true;
  }
  return false;
}

procCharImmLeash::procCharImmLeash(const int &p)
{
  trigger_pulse=p;
  name="procCharImmLeash";
}

bool procCharImmLeash::run(const TPulse &pl, TBeing *tmp_ch) const
{
  if (tmp_ch->isPc() && tmp_ch->desc && tmp_ch->GetMaxLevel() > MAX_MORT &&
      !tmp_ch->limitPowerCheck(CMD_GOTO, tmp_ch->roomp->number)
      && !tmp_ch->affectedBySpell(SPELL_POLYMORPH) &&
      !IS_SET(tmp_ch->specials.act, ACT_POLYSELF)) {
    char tmpbuf[256];
    strcpy(tmpbuf, "");
    tmp_ch->sendTo("An incredibly powerful force pulls you back into Imperia.\n\r");
    act("$n is pulled back whence $e came.", TRUE, tmp_ch, 0, 0, TO_ROOM);
    vlogf(LOG_BUG,format("%s was wandering around the mortal world (R:%d) so moving to office.") % 
	  tmp_ch->getName() % tmp_ch->roomp->number);
    
    if (!tmp_ch->hasWizPower(POWER_GOTO)) {
      tmp_ch->setWizPower(POWER_GOTO);
      tmp_ch->doGoto(tmpbuf);
      tmp_ch->remWizPower(POWER_GOTO);
    } else {
      tmp_ch->doGoto(tmpbuf);
    }
    act("$n appears in the room with a sheepish look on $s face.", TRUE, tmp_ch, 0, 0, TO_ROOM);
  }
  return false;
}


procCharSpellTask::procCharSpellTask(const int &p)
{
  trigger_pulse=p;
  name="procCharSpellTask";
}

bool procCharSpellTask::run(const TPulse &pl, TBeing *tmp_ch) const
{
  if (tmp_ch->spelltask) {
    int rc = (tmp_ch->cast_spell(tmp_ch, CMD_TASK_CONTINUE, pl.pulse));
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return true;
  }
  return false;
}

procCharAffects::procCharAffects(const int &p)
{
  trigger_pulse=p;
  name="procCharAffects";
}

bool procCharAffects::run(const TPulse &pl, TBeing *tmp_ch) const
{
  int rc = tmp_ch->updateAffects();
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    // died in update (disease) 
    return true;
  }
  return false;
}

procCharRegen::procCharRegen(const int &p)
{
  trigger_pulse=p;
  name="procCharRegen";
}

bool procCharRegen::run(const TPulse &pl, TBeing *tmp_ch) const
{
  // this was in hit(), makes more sense here I think
  if (tmp_ch->roomp && !tmp_ch->roomp->isRoomFlag(ROOM_NO_HEAL) && 
      tmp_ch->getMyRace()->hasTalent(TALENT_FAST_REGEN) &&
      tmp_ch->getHit() < tmp_ch->hitLimit() &&
      tmp_ch->getCond(FULL) && tmp_ch->getCond(THIRST) &&
      !::number(0, 10)){
    // mostly for trolls
    int addAmt = (int)(tmp_ch->hitGain() / 10.0);
    if (addAmt > 0) {
      act("You regenerate slightly.", TRUE, tmp_ch, 0, 0, TO_CHAR);
      act("$n regenerates slightly.", TRUE, tmp_ch, 0, 0, TO_ROOM);
      tmp_ch->addToHit(addAmt);
    }
  }
  return false;
}

procCharCantHit::procCharCantHit(const int &p)
{
  trigger_pulse=p;
  name="procCharCantHit";
}

bool procCharCantHit::run(const TPulse &pl, TBeing *tmp_ch) const
{
  // soak up attack if not in combat
  if ((tmp_ch->cantHit > 0) && !tmp_ch->fight())
    tmp_ch->cantHit--;
  return false;
}

procCharRiverFlow::procCharRiverFlow(const int &p)
{
  trigger_pulse=p;
  name="procCharRiverFlow";
}

bool procCharRiverFlow::run(const TPulse &pl, TBeing *tmp_ch) const
{
  int rc = tmp_ch->riverFlow(pl.pulse);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}

procCharTeleportRoom::procCharTeleportRoom(const int &p)
{
  trigger_pulse=p;
  name="procCharTeleportRoom";
}

bool procCharTeleportRoom::run(const TPulse &pl, TBeing *tmp_ch) const
{
  int rc = tmp_ch->teleportRoomFlow(pl.pulse);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}

procCharNoise::procCharNoise(const int &p)
{
  trigger_pulse=p;
  name="procCharNoise";
}

bool procCharNoise::run(const TPulse &pl, TBeing *tmp_ch) const
{
  TMonster *tmon = dynamic_cast<TMonster *>(tmp_ch);
  if (!number(0, 3) && !tmp_ch->isPc() && tmon)
    tmon->makeNoise();
  return false;
}

procCharHalfTickUpdate::procCharHalfTickUpdate(const int &p)
{
  trigger_pulse=p;
  name="procCharHalfTickUpdate";
}

bool procCharHalfTickUpdate::run(const TPulse &pl, TBeing *tmp_ch) const
{
  int rc = tmp_ch->updateHalfTickStuff();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}
      
procCharTickUpdate::procCharTickUpdate(const int &p)
{
  trigger_pulse=p;
  name="procCharTickUpdate";
}

bool procCharTickUpdate::run(const TPulse &pl, TBeing *tmp_ch) const
{
  int rc = tmp_ch->updateTickStuff();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}

procCharThaw::procCharThaw(const int &p)
{
  trigger_pulse=p;
  name="procCharThaw";
}

bool procCharThaw::run(const TPulse &pl, TBeing *tmp_ch) const
{
  // thawing
  tmp_ch->thawEngulfed();
  return false;
}

procCharLightning::procCharLightning(const int &p)
{
  trigger_pulse=p;
  name="procCharLightning";
}

bool procCharLightning::run(const TPulse &pl, TBeing *tmp_ch) const
{
    // lightning strikes
  if(!tmp_ch->roomp->isIndoorSector() &&
     !tmp_ch->roomp->isRoomFlag(ROOM_INDOORS) &&
     Weather::getWeather(*tmp_ch->roomp) == Weather::LIGHTNING){
    TThing *eq=NULL;
    
    if(tmp_ch->equipment[WEAR_HEAD] &&
       tmp_ch->equipment[WEAR_HEAD]->isMetal()){
      eq=tmp_ch->equipment[WEAR_HEAD];
    } else if(tmp_ch->equipment[HOLD_RIGHT] &&
	      tmp_ch->equipment[HOLD_RIGHT]->isMetal()){
      eq=tmp_ch->equipment[HOLD_RIGHT];
    } else if(tmp_ch->equipment[HOLD_LEFT] &&
	      tmp_ch->equipment[HOLD_LEFT]->isMetal()){
      eq=tmp_ch->equipment[HOLD_LEFT];
    }
    
    
    if(eq && !::number(0,4319)){
      // at this point, they're standing outside in a lightning storm,
      // either holding something metal or wearing a metal helmet. zzzap.
      act(format("A bolt of lightning streaks down from the heavens and hits your %s!") % fname(eq->name),
	  FALSE, tmp_ch, 0, 0, TO_CHAR);
      act("BZZZZZaaaaaappppp!!!!!",
	  FALSE, tmp_ch, 0, 0, TO_CHAR);
      act(format("A bolt of lightning streaks down from the heavens and hits $n's %s!") % fname(eq->name),
	  FALSE, tmp_ch, 0, 0, TO_ROOM);
      
      // stolen from ego blast
      if (tmp_ch->reconcileDamage(tmp_ch, tmp_ch->getHit()/2, DAMAGE_ELECTRIC) == -1)
	return true;
      tmp_ch->setMove(tmp_ch->getMove()/2);
    }
  }
  return false;
}


procCharNutrition::procCharNutrition(const int &p)
{
  trigger_pulse=p;
  name="procCharNutrition";
}

bool procCharNutrition::run(const TPulse &pl, TBeing *tmp_ch) const
{
  if(dynamic_cast<TPerson *>(tmp_ch) && !tmp_ch->isImmortal()){
    // nutrition is a measure of the amount of calories eaten subtracting
    // the amount worked off
    // if the balance gets out of wack far enough, you gain or lose
    // a pound and the balance is reset
    
    // threshold is the net balance required before weight gain or loss
    int threshold=5000;
    
    int nutrition=0;
    if(tmp_ch->getCond(FULL) <= 5) // stomach growling
      nutrition--;
    if(tmp_ch->getCond(FULL) >= 6) // little bite to eat
      nutrition++;
    if(tmp_ch->getCond(FULL) >= 10) // slighly hungry
      nutrition++;
    if(tmp_ch->getCond(FULL) >= 20) // full
      nutrition++;
    
    if(tmp_ch->getMove() < (tmp_ch->getMaxMove() * 0.25))
      nutrition-=2;
    if(tmp_ch->getMove() < (tmp_ch->getMaxMove() * 0.50))
      nutrition-=2;
    if(tmp_ch->getMove() < (tmp_ch->getMaxMove() * 0.75))
      nutrition-=2;
    if(tmp_ch->getMove() > (tmp_ch->getMaxMove() * 0.90))
      nutrition-=2;
    
    tmp_ch->addToNutrition(nutrition);
    
    
    if(tmp_ch->getNutrition() > threshold){
      if((tmp_ch->getWeight()+1) <= 
	 tmp_ch->getMyRace()->getMaxWeight(tmp_ch->getSex())){
	tmp_ch->sendTo("You feel as though you've been putting on some weight.\n\r");
	tmp_ch->setWeight(tmp_ch->getWeight()+1);
      }
      tmp_ch->setNutrition(0);
    } else if(tmp_ch->getNutrition() < -threshold){
      if((tmp_ch->getWeight()-1) >= 
	 tmp_ch->getMyRace()->getMinWeight(tmp_ch->getSex())){
	tmp_ch->sendTo("You feel as though you've been losing some weight.\n\r");
	tmp_ch->setWeight(tmp_ch->getWeight()-1);
      }
      tmp_ch->setNutrition(0);
    }
  }
  return false;
}
	
procCharLycanthropy::procCharLycanthropy(const int &p)
{
  trigger_pulse=p;
  name="procCharLycanthropy";
}

bool procCharLycanthropy::run(const TPulse &pl, TBeing *tmp_ch) const
{
    // lycanthrope transformation
  if(tmp_ch->hasQuestBit(TOG_LYCANTHROPE) &&
     !tmp_ch->hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE)
     && !tmp_ch->isLinkdead() &&
     
     Weather::moonType() == "full" && 
     !Weather::sunIsUp() && Weather::moonIsUp()) {
    lycanthropeTransform(tmp_ch);
  } else if(tmp_ch->hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE)){
    if(Weather::moonType() != "full" || 
       Weather::sunIsUp() || !Weather::moonIsUp()){
      tmp_ch->remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
      tmp_ch->doReturn("", WEAR_NOWHERE, CMD_RETURN);
    } else if(!tmp_ch->fight() && tmp_ch->roomp && 
	      !tmp_ch->roomp->isRoomFlag(ROOM_PEACEFUL) &&
	      !::number(0,24)){
      tmp_ch->setCombatMode(ATTACK_BERSERK);
      tmp_ch->goBerserk(NULL);
    }
  }
  return false;
}


procCharSpecProcsQuick::procCharSpecProcsQuick(const int &p)
{
  trigger_pulse=p;
  name="procCharSpecProcsQuick";
}

bool procCharSpecProcsQuick::run(const TPulse &pl, TBeing *tmp_ch) const
{
  if (tmp_ch->spec) {
    int rc = tmp_ch->checkSpec(tmp_ch, CMD_GENERIC_QUICK_PULSE, "", NULL);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return true;
  }
  return false;
}

procCharScreenUpdate::procCharScreenUpdate(const int &p)
{
  trigger_pulse=p;
  name="procCharScreenUpdate";
}

bool procCharScreenUpdate::run(const TPulse &pl, TBeing *tmp_ch) const
{
  if (tmp_ch->desc && (tmp_ch->vt100() || tmp_ch->ansi())) {
    time_t t1;
    struct tm *tptr;
    if ((t1 = time((time_t *) 0)) != -1) {
      tptr = localtime(&t1);
      if (tptr->tm_min != tmp_ch->desc->last.minute) {
	tmp_ch->desc->last.minute = tptr->tm_min;
	if (tmp_ch->ansi()) 
	  tmp_ch->desc->updateScreenAnsi(CHANGED_TIME);
	else
	  tmp_ch->desc->updateScreenVt100(CHANGED_TIME);
      }
    }
  }
  return false;
}

procCharVampireBurn::procCharVampireBurn(const int &p)
{
  trigger_pulse=p;
  name="procCharVampireBurn";
}

bool procCharVampireBurn::run(const TPulse &pl, TBeing *tmp_ch) const
{
  // check for vampires in daylight
  if(!tmp_ch->roomp->isIndoorSector() && 
     !tmp_ch->roomp->isRoomFlag(ROOM_INDOORS) &&
     (tmp_ch->inRoom() != Room::VOID) && Weather::sunIsUp()){
    
    if(tmp_ch->hasQuestBit(TOG_VAMPIRE)){
      act("<r>Exposure to sunlight causes your skin to ignite!<1>",
	  FALSE, tmp_ch, NULL, NULL, TO_CHAR);
      act("<r>$n's skin ignites in flames as the sunlight shines on $m!<1>",
	  FALSE, tmp_ch, NULL, NULL, TO_ROOM);
      
      int rc=tmp_ch->reconcileDamage(tmp_ch, ::number(20,200), SPELL_RAZE);
      
      if(IS_SET_DELETE(rc, DELETE_THIS))
	return true;
    } else if(tmp_ch->hasQuestBit(TOG_BITTEN_BY_VAMPIRE) &&
	      !::number(0,40)){
      act("Exposure to sunlight makes your skin itch.",
	  FALSE, tmp_ch, NULL, NULL, TO_CHAR);
    }
  }

  return false;
}



procRoomPulse::procRoomPulse(const int &p)
{
  trigger_pulse=p;
  name="procRoomPulse";
}

void procRoomPulse::run(const TPulse &pl) const
{
  int count=0;

  for(int i=0;i<WORLD_SIZE;i++){
    TRoom *rp = real_roomp(i);
    
    if(!rp)
      continue;

    //    ++count;

    // rain
    if(pl.mobstuff){
      if((Weather::getWeather(*rp)==Weather::RAINY ||
	  Weather::getWeather(*rp)==Weather::LIGHTNING) &&
	 !::number(0,999)){
	rp->dropPool(::number(2,5), LIQ_WATER);
	++count;
      }
    }
  }

  //  return count;
}

procCheckTask::procCheckTask(const int &p)
{
  trigger_pulse=p;
  name="procCheckTask";
}

void procCheckTask::run(const TPulse &pl) const
{
  systask->CheckTask();
}

procLagInfo::procLagInfo(const int &p)
{
  trigger_pulse=p;
  name="procLagInfo";
}

void procLagInfo::run(const TPulse &pl) const
{
  static time_t lagtime_t = time(0);
  // get some lag info
  // this needs to remain at pulse%100
  if (!(pl.pulse %100)){
    int which=(pl.pulse/100)%10;
    
    lag_info.current=lag_info.lagtime[which]=time(0)-lagtime_t;
    lagtime_t=time(0);
    lag_info.lagcount[which]=1;
    
    lag_info.high = max(lag_info.lagtime[which], lag_info.high);
    lag_info.low = min(lag_info.lagtime[which], lag_info.low);
  }
}

procHandleTimeAndSockets::procHandleTimeAndSockets(const int &p)
{
  trigger_pulse=p;
  name="procHandleTimeAndSockets";
}

void procHandleTimeAndSockets::run(const TPulse &pl) const
{
  gSocket->handleTimeAndSockets();
}

procIdle::procIdle(const int &p)
{
  trigger_pulse=p;
  name="procIdle";
}

void procIdle::run(const TPulse &pl) const
{
  sleep(gSocket->sleeptime.tv_sec);
  usleep(gSocket->sleeptime.tv_usec);
}

procDoubleXP::procDoubleXP(const int &p)
{
  trigger_pulse=p;
  name="procDoubleXP";
}

void procDoubleXP::run(const TPulse &pl) const
{
  time_t ct = time(0);
  struct tm today = *localtime(&ct);
  ct = time(0) - (24*60*60*1);
  struct tm today_m1 = *localtime(&ct);
  ct = time(0) - (24*60*60*2);
  struct tm today_m2 = *localtime(&ct);

  ct = time(0) + (24*60*60*7);
  struct tm next_week = *localtime(&ct);
  ct = time(0) + (24*60*60*6);
  struct tm next_week_m1 = *localtime(&ct);
  ct = time(0) + (24*60*60*5);
  struct tm next_week_m2 = *localtime(&ct);

  static bool turnedOn=false;
  

  enum tm_days {
    Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday
  };

  if(/* if today is friday and next friday (+7 days) is a different month */
     (today.tm_wday==Friday && next_week.tm_mon!=today.tm_mon) ||
     /* if today is saturday, and next friday (+6 days) is a different month */
     (today.tm_wday==Saturday && next_week_m1.tm_mon!=today_m1.tm_mon) ||
     /* if today is sunday, and next friday (+5 days) is a different month */
     (today.tm_wday==Sunday && next_week_m2.tm_mon!=today_m2.tm_mon)){
    toggleInfo[TOG_DOUBLEEXP]->toggle = true;
    turnedOn=true;
  } else if(turnedOn){
    toggleInfo[TOG_DOUBLEEXP]->toggle = false;
    turnedOn=false;
  }
}




int TMainSocket::gameLoop()
{
  Descriptor *point;
  int pulse = 0;
  sstring str;
  TScheduler scheduler;

  // pulse every  (1/10th of a second)
  scheduler.add(new procHandleTimeAndSockets(PULSE_EVERY));
  scheduler.add(new procIdle(PULSE_EVERY));
  scheduler.add(new procSetZoneEmpty(PULSE_EVERY));
  scheduler.add(new procCallRoomSpec(PULSE_EVERY));
  scheduler.add(new procDoRoomSaves(PULSE_EVERY));
  scheduler.add(new procDoPlayerSaves(PULSE_EVERY));
  scheduler.add(new procCheckTask(PULSE_EVERY));
  scheduler.add(new procLagInfo(PULSE_EVERY));
  scheduler.add(new procCharScreenUpdate(PULSE_EVERY));

  // pulse combat  (1.2 seconds)
  scheduler.add(new procPerformViolence(PULSE_COMBAT));
  scheduler.add(new procWeightVolumeFumble(PULSE_COMBAT));
  scheduler.add(new procQueryQueue(PULSE_COMBAT));
  scheduler.add(new procRoomPulse(PULSE_COMBAT));
  scheduler.add(new procObjVehicle(PULSE_COMBAT));
  scheduler.add(new procObjDetonateGrenades(PULSE_COMBAT));
  scheduler.add(new procObjFalling(PULSE_COMBAT));
  scheduler.add(new procObjRiverFlow(PULSE_COMBAT));
  scheduler.add(new procObjTeleportRoom(PULSE_COMBAT));
  scheduler.add(new procObjSpecProcsQuick(PULSE_COMBAT));
  scheduler.add(new procCharVampireBurn(PULSE_COMBAT));
  scheduler.add(new procCharSinking(PULSE_COMBAT));
  scheduler.add(new procCharFalling(PULSE_COMBAT));
  scheduler.add(new procCharMobileActivity(PULSE_COMBAT));
  scheduler.add(new procCharTasks(PULSE_COMBAT));
  scheduler.add(new procCharImmLeash(PULSE_COMBAT));
  scheduler.add(new procCharSpellTask(PULSE_COMBAT));
  scheduler.add(new procCharAffects(PULSE_COMBAT));
  scheduler.add(new procCharRegen(PULSE_COMBAT));
  scheduler.add(new procCharCantHit(PULSE_COMBAT));
  scheduler.add(new procCharRiverFlow(PULSE_COMBAT));
  scheduler.add(new procCharTeleportRoom(PULSE_COMBAT));
  scheduler.add(new procCharLightning(PULSE_COMBAT));
  scheduler.add(new procCharLycanthropy(PULSE_COMBAT));
  scheduler.add(new procCharSpecProcsQuick(PULSE_COMBAT));

  // pulse spec_procs (3.6 seconds)
  scheduler.add(new procObjBurning(PULSE_SPEC_PROCS));
  scheduler.add(new procObjSinking(PULSE_SPEC_PROCS));
  scheduler.add(new procObjSpecProcs(PULSE_SPEC_PROCS));
  scheduler.add(new procObjSmoke(PULSE_SPEC_PROCS));
  scheduler.add(new procObjPools(PULSE_SPEC_PROCS));
  scheduler.add(new procObjTrash(PULSE_SPEC_PROCS));
  scheduler.add(new procCharSpecProcs(PULSE_SPEC_PROCS));
  scheduler.add(new procCharDrowning(PULSE_SPEC_PROCS));
  scheduler.add(new procCharResponses(PULSE_SPEC_PROCS));

  // pulse noises (4.8 seconds)
  scheduler.add(new procCharNoise(PULSE_NOISES));
  scheduler.add(new procCharNutrition(PULSE_NOISES));

  // pulse update  (36 seconds)
  scheduler.add(new procGlobalRoomStuff(PULSE_UPDATE));
  scheduler.add(new procDeityCheck(PULSE_UPDATE));
  scheduler.add(new procApocCheck(PULSE_UPDATE));
  scheduler.add(new procSaveFactions(PULSE_UPDATE));
  scheduler.add(new procSaveNewFactions(PULSE_UPDATE));
  scheduler.add(new procWeatherAndTime(PULSE_UPDATE));
  scheduler.add(new procWholistAndUsageLogs(PULSE_UPDATE));
  scheduler.add(new procObjRust(PULSE_UPDATE));
  scheduler.add(new procObjFreezing(PULSE_UPDATE));
  scheduler.add(new procObjAutoPlant(PULSE_UPDATE));
  scheduler.add(new procCharHalfTickUpdate(PULSE_UPDATE));
  scheduler.add(new procCharThaw(PULSE_UPDATE));
  scheduler.add(new procDoubleXP(PULSE_UPDATE));

  // pulse mudhour  (144 seconds (2.4 mins))
  scheduler.add(new procFishRespawning(PULSE_MUDHOUR));
  scheduler.add(new procReforestation(PULSE_MUDHOUR));
  scheduler.add(new procZoneUpdate(PULSE_MUDHOUR));
  scheduler.add(new procLaunchCaravans(PULSE_MUDHOUR));
  scheduler.add(new procUpdateAvgPlayers(PULSE_MUDHOUR));
  scheduler.add(new procCheckGoldStats(PULSE_MUDHOUR));
  scheduler.add(new procAutoTips(PULSE_MUDHOUR));
  scheduler.add(new procPingData(PULSE_MUDHOUR));
  scheduler.add(new procRecalcFactionPower(PULSE_MUDHOUR));
  scheduler.add(new procNukeInactiveMobs(PULSE_MUDHOUR));
  scheduler.add(new procUpdateTime(PULSE_MUDHOUR));
  scheduler.add(new procMobHate(PULSE_MUDHOUR));
  scheduler.add(new procDoComponents(PULSE_MUDHOUR));
  scheduler.add(new procObjTickUpdate(PULSE_MUDHOUR));
  scheduler.add(new procCharTickUpdate(PULSE_MUDHOUR));

  // pulse wayslow  (240 seconds (4 mins))
  scheduler.add(new procCheckForRepo(PULSE_WAYSLOW));
  scheduler.add(new procCheckMail(PULSE_WAYSLOW));
  //  scheduler.add(new procCheckTriggerUsers(PULSE_WAYSLOW));
  
  // pulse mudday   (3456 seconds (57.6 mins))
  scheduler.add(new procUpdateAuction(PULSE_MUDDAY));
  scheduler.add(new procBankInterest(PULSE_MUDDAY));
  scheduler.add(new procCloseAccountingBooks(PULSE_MUDDAY));
  scheduler.add(new procRecordCommodPrices(PULSE_MUDDAY));
  scheduler.add(new procFactoryProduction(PULSE_MUDDAY));

  // pulse realhour
//  scheduler.add(new procTweakLoadRate(PULSE_REALHOUR)); // desired load rate achieved
  scheduler.add(new procTrophyDecay(PULSE_REALHOUR));
  scheduler.add(new procSeedRandom(PULSE_REALHOUR));

  avail_descs = 150;		

  // players may have connected before this point via 
  // addNewDescriptorsDuringBoot, so send all those descriptors the login
  for (point = descriptor_list; point; point = point->next)
    if (!point->m_bIsClient)
      point->sendLogin("1");

  while (!handleShutdown()) {
    scheduler.run(++pulse);
    
    tics++;			// tics since last checkpoint signal 
  }

  // flush the query queue
  TDatabase db(DB_SNEEZY);
  while(!queryqueue.empty()){
    db.query(queryqueue.front().c_str());
    queryqueue.pop();
  }

  ares_destroy(channel);
  return TRUE;
}


TSocket *TMainSocket::newConnection(int t_sock, int port)
{
  struct sockaddr_in isa;
#if defined(LINUX)
  unsigned int i;
#else
  int i;
#endif
  TSocket *s;

  i = sizeof(isa);
  if (getsockname(t_sock, (struct sockaddr *) &isa, &i)) {
    perror("getsockname");
    return NULL;
  }
  s = new TSocket();
  if ((s->m_sock = accept(t_sock, (struct sockaddr *) (&isa), &i)) < 0) {
    perror("Accept");
    return NULL;
  }
  s->nonBlock();
  s->port=port;
  return (s);
}

static const sstring IP_String(in_addr &_a)
{
  sstring buf;
#if (defined SUN)
  sprintf( buf, "%d.%d.%d.%d", 
          _a.S_un.S_un_b.s_b1,
          _a.S_un.S_un_b.s_b2,
          _a.S_un.S_un_b.s_b3,
          _a.S_un.S_un_b.s_b4);
#else
  int n1, n2, n3, n4; 
  n1 = _a.s_addr >> 24;
  n2 = (_a.s_addr >> 16) - (n1 * 256);
  n3 = (_a.s_addr >> 8) - (n1 * 65536) - (n2 * 256);
  n4 = (_a.s_addr) % 256;
  buf = format("%d.%d.%d.%d") % n4 % n3 % n2 % n1;
#endif
  return buf;
}

void sig_alrm(int){return;}

void gethostbyaddr_cb(void *arg, int status, struct hostent *host_ent)
{
  Descriptor *d;
  Descriptor *d2;
  sstring ip_string, pend_ip_string;

  ip_string = (char *)arg;
  pend_ip_string = ip_string + "...";

  if (status != ARES_SUCCESS) {
    char *ares_errmem;

    vlogf(LOG_MISC, format("gethostbyaddr_cb: %s: %s") % ip_string % ares_strerror(status, &ares_errmem));
    ares_free_errmem(ares_errmem);

    for (d = descriptor_list; d; d = d2) {
      d2 = d->next;
      if (!d->getHostResolved() && d->host == pend_ip_string) {
        d->setHostResolved(true, ip_string);
        if (numberhosts) {
          int a;
          for (a = 0; a <= numberhosts - 1; a++) {
            if (isdigit(hostlist[a][0])) {
              if (d->host.find(hostlist[a], 0) != sstring::npos) {
                d->socket->writeToSocket("Sorry, your site is banned.\n\r");
                d->socket->writeToSocket("Questions regarding this may be addressed to: ");
                d->socket->writeToSocket(MUDADMIN_EMAIL);
                d->socket->writeToSocket(".\n\r");
                if (!lockmess.empty())
                  d->socket->writeToSocket(lockmess.c_str());

                // descriptor deletion handles socket closing
                delete d;
              }
            }
          }
        }
      }
    }
  } else {
    char **p;
    struct in_addr addr;
    sstring resolved_name;

    p = host_ent->h_addr_list;
    memcpy(&addr, *p, sizeof(struct in_addr));
    resolved_name = sstring(host_ent->h_name).lower();

    vlogf(LOG_MISC, format("gethostbyaddr_cb: %s resolved to %-32s") % ip_string % resolved_name);

    for (d = descriptor_list; d; d = d2) {
      d2 = d->next;
      if (!d->getHostResolved() && d->host == pend_ip_string) {
        d->setHostResolved(true, resolved_name);
        if (numberhosts) {
          int a;
          for (a = 0; a <= numberhosts - 1; a++) {
            if (d->host.find(sstring(hostlist[a]).lower(), 0) != sstring::npos) {
              d->socket->writeToSocket("Sorry, your site is banned.\n\r");
              d->socket->writeToSocket("Questions regarding this may be addressed to: ");
              d->socket->writeToSocket(MUDADMIN_EMAIL);
              d->socket->writeToSocket(".\n\r");
              if (!lockmess.empty())
                d->socket->writeToSocket(lockmess.c_str());

              // descriptor deletion handles socket closing
              delete d;
            }
          }
        }
      }
    }
  }

  delete (char *)arg;
}      

int TMainSocket::newDescriptor(int t_sock, int port)
{
#if defined(LINUX)
  unsigned int size;
#else
  int size;
#endif
  Descriptor *newd;
  struct sockaddr_in saiSock;
  TSocket *s = NULL;
  char *ip_cstr;

  if (!(s = newConnection(t_sock, port))) 
    return 0;

  if ((maxdesc + 1) >= avail_descs) {
    vlogf(LOG_MISC, "Descriptor being dumped due to high load - Bug Batopr");
    s->writeToSocket("Sorry.. The game is full...\n\r");
    s->writeToSocket("Please try again later...\n\r");
    close(s->m_sock);
    delete s;
    return 0;
  } else if (s->m_sock > maxdesc)
    maxdesc = s->m_sock;

  newd = new Descriptor(s);

  size = sizeof(saiSock);
  if (getpeername(s->m_sock, (struct sockaddr *) &saiSock, &size) < 0) {
    perror("getpeername");
    newd->host = "";
  } else {
    // we sometimes hang here, so lets log any suspicious events
    // I _think_ the problem is caused by a site that has changed its DNS
    // entry, but the mud's site has not updated the new list yet.
    signal(SIGALRM, sig_alrm);
    
    newd->setHostResolved(false, IP_String(saiSock.sin_addr) + "...");

    ip_cstr = mud_str_dup(IP_String(saiSock.sin_addr));
    memcpy(&ares_addr, &saiSock.sin_addr, sizeof(struct in_addr));

    ares_gethostbyaddr(channel, &ares_addr, sizeof(ares_addr), AF_INET, gethostbyaddr_cb, ip_cstr);
  }

  if (newd->inputProcessing() < 0) {
    delete newd;
    newd = NULL;
    return 0;
  }

  return 1;
}

int TSocket::writeNull()
{
  char txt='\0';

  if (write(m_sock, &txt, 1) < 0){
    if (errno == EWOULDBLOCK)
      return 0;
    
    perror("TSocket::writeToSocket(char *)");
    return (-1);
  }
  
  return 0;
}

int TSocket::writeToSocket(const char *txt)
{
  int sofar, thisround, total;

  total = strlen(txt);
  sofar = 0;

  //txt >> m_sock;
 
  do {
    thisround = write(m_sock, txt + sofar, total - sofar);
    if (thisround < 0) {
      if (errno == EWOULDBLOCK)
	break;

      perror("TSocket::writeToSocket(char *)");
      return (-1);
    }
    sofar += thisround;
  }
  while (sofar < total);
  return 0;
}


void TMainSocket::closeAllSockets()
{
  vlogf(LOG_MISC, "Closing all sockets.");

  while (descriptor_list)
    delete descriptor_list;

  for(unsigned int i=0;i<m_sock.size();++i)
    close(m_sock[i]);
}


void TSocket::nonBlock()
{
  if (fcntl(m_sock, F_SETFL, FNDELAY) == -1) {
    perror("Noblock");
    exit(1);
  }
}

void TMainSocket::initSocket(int t_port)
{
  const char *opt = "1";
  char hostname[MAXHOSTNAMELEN];
  struct sockaddr_in sa;
  struct hostent *hp;
  struct linger ld;
  int t_sock;

#if defined(SUN)
  bzero((char *) &sa, sizeof(struct sockaddr_in));
#else
  memset((char *) &sa, 0, sizeof(sa));
#endif

#if 0
  gethostname(hostname, MAXHOSTNAMELEN);
  if (!(hp = gethostbyname(hostname))) {
#else
  if (!(hp = gethostbyname("localhost"))) {
#endif
    vlogf(LOG_BUG, format("failed getting hostname structure.  hostname: %s") %  hostname);
    perror("gethostbyname");
    exit(1);
  }
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons(t_port);
  if ((t_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Init-socket");
    exit(1);
  }
  if (setsockopt(t_sock, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt REUSEADDR");
    exit(1);
  }
  ld.l_linger = 1000;
  ld.l_onoff = 0;
#ifdef OSF
  if (setsockopt(t_sock, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0) {
#else
  if (setsockopt(t_sock, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0) {
#endif
    perror("setsockopt LINGER");
    exit(1);
  }
  if (bind(t_sock, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    perror("bind");
    vlogf(LOG_BUG, format("initSocket: bind: errno=%d") %  errno);
    close(t_sock);
    exit(0);
  }
  listen(t_sock, 3);

  m_sock.push_back(t_sock);
  m_port.push_back(t_port);
}

TSocket::TSocket()
{
}

TSocket::~TSocket()
{
}

TMainSocket::TMainSocket()
{
}

TMainSocket::~TMainSocket()
{
}
