
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

#include "stdsneezy.h"
#include "statistics.h"
#include "database.h"
#include "spelltask.h"
#include "systemtask.h"
#include "socket.h"
#include "weather.h"
#include "obj_smoke.h"
#include "obj_vehicle.h"
#include "obj_trash_pile.h"
#include "pathfinder.h"
#include "timing.h"

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

#ifndef SOLARIS
  static int mask;
#endif

  if (!been_called) {
    // prepare the time values 
    null_time.tv_sec = 0;
    null_time.tv_usec = 0;

    maxdesc=0;
    for(unsigned int i=0;i<m_sock.size();++i)
      maxdesc = max(maxdesc, m_sock[i]);

    avail_descs = 150;

#ifndef SOLARIS
    mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
// blah, trapping PROF PREVENTS the timing signals from working!
//        sigmask(SIGPROF) |  // needed for profile code
// sigmask(SIGALRM) |
        sigmask(SIGPIPE) | sigmask(SIGTERM) |
        sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);
#endif

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

#ifndef SOLARIS
  sigsetmask(mask);
#endif
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


#ifndef SOLARIS
  sigsetmask(0);
#endif

  // establish any new connections 
  for(unsigned int i=0;i<m_sock.size();++i){
    if (FD_ISSET(m_sock[i], &input_set)) {
      int tFd;
      
      if ((tFd = newDescriptor(m_sock[i])) < 0)
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
    
    //  vlogf(LOG_DASH, fmt("Updating who table for port %d") %  gamePort);
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
    //	vlogf(LOG_DASH, fmt("Webstuff: collecting game usage data - %d seconds since last log") %  ct-lastlog);
    //        vlogf(LOG_DASH, fmt("Webstuff:  logtime = %d,  ct = %d, players = %d") %  logtime % ct % count);
    
    
    if (logtime != 0) logtime += TIME_BETWEEN_LOGS;
    else logtime = ct;
    lastlog = ct;
    db.query("insert into usagelogs (time, players, port) VALUES(%i, %i, %i)", logtime, count, gamePort);
    // delete logs older than two months
    db.query("delete from usagelogs where port=%i and time>%i", logtime + 5184000 , gamePort);
  }
}

// check eq of all players online for repo
void checkForRepo(){
  TBeing *tmp_ch, *temp;

  for (tmp_ch = character_list; tmp_ch; tmp_ch = temp) {
    temp = tmp_ch->next; 
    int i;
    TThing *repot, *repot2;
    TObj *repoo;
    // check worn equipment
    for (i = MIN_WEAR;i < MAX_WEAR;i++) {
      if (!(repot = tmp_ch->equipment[i]) || !(repoo = dynamic_cast<TObj *>(repot)))
	continue;
      
      repoCheckForRent(tmp_ch, repoo, false);
    }
    // check inventory
    for (repot = tmp_ch->getStuff(); repot; repot = repot2) {
      repot2 = repot->nextThing;
      repoo = dynamic_cast<TObj *>(repot);
      if (!repoo)
	continue;
      
      repoCheckForRent(tmp_ch, repoo, false);
    }
    
  }
}

void nukeInactiveMobs()
{
  unsigned int i;
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

// update the average players displayed in "who"
void updateAvgPlayers()
{
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
      buf=fmt("%s time has arrived!\n\r") % shutdown_or_reboot();
      descriptor_list->worldSend(buf, NULL);
      descriptor_list->outputProcessing();
    }
    return true;
  } else if (timeTill && !((timeTill - time(0)) % 60)) {
    int minutes=(timeTill - time(0)) / 60;
    if (!sent) {
      buf="<r>******* SYSTEM MESSAGE ******<z>\n\r";
      buf+=fmt("<c>%s in %ld minute%s.<z>\n\r") % 
	shutdown_or_reboot() % minutes % ((minutes == 1) ? "" : "s");
      descriptor_list->worldSend(buf, NULL);
    }
    sent = true;
  } else if (timeTill && ((timeTill- time(0)) <= 5)) {
    long secs = timeTill - time(0);
    if (!sent) {
      buf="<r>******* SYSTEM MESSAGE ******<z>\n\r";
      buf+=fmt("<c>%s in %ld second%s.<z>\n\r") %
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

  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  opt_time.tv_usec = OPT_USEC;
  opt_time.tv_sec = 0;
  
  int mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
    sigmask(SIGPIPE) | sigmask(SIGTERM) |
    sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);

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

  sigsetmask(mask);


  // this gets our list of socket connections that are ready for handling
  if(select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0){
    perror("Error in Select (poll)");
    exit(-1);
  }

  // this regulates the speed of the mud
  if (select(0, 0, 0, 0, &timeout) < 0) {
    perror("Error in select (sleep)");
  }

  sigsetmask(0);

  ////////////////////////////////////////////
  ////////////////////////////////////////////

  ////////////////////////////////////////////
  // establish any new connections 
  ////////////////////////////////////////////
  for(unsigned int i=0;i<m_sock.size();++i){
    if (FD_ISSET(m_sock[i], &input_set)) {
      int rc = newDescriptor(m_sock[i]);
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
  if(!gameLoopTiming)
    return;

  vlogf(LOG_MISC, fmt("%i %i) %s: %i") % 
	pulse % (pulse%12) % name % (int)(timer.getElapsedReset()*1000000));
}


int TMainSocket::characterPulse(TPulseList &pl, int realpulse)
{
  TBeing *temp;
  int rc, count, retcount;
  TTiming t;

  // note on this loop
  // it is possible that temp gets deleted in one of the sub funcs
  // we don't get acknowledgement of this in any way.
  // to avoid problems this might cause, we reinitialize temp at
  // the end (eg, before any deletes, or before we come back around)
  // bottom line is that temp keeps getting set because it might be
  // bogus after the function call.

  // we've already finished going through the character list, so start over
  if(!tmp_ch)
    tmp_ch=character_list;

  retcount=count=(int)((float)mobCount/11.5);


  for (; tmp_ch; tmp_ch = temp) {
    temp = tmp_ch->next;  // just for safety

    if(!count--)
      break;

    if (tmp_ch->getPosition() == POSITION_DEAD) {
      vlogf(LOG_BUG, fmt("Error: dead creature (%s at %d) in character_list, removing.") % 
	    tmp_ch->getName() % tmp_ch->in_room);
      delete tmp_ch;
      tmp_ch = NULL;
      continue;
    }
    if ((tmp_ch->getPosition() < POSITION_STUNNED) &&
	(tmp_ch->getHit() > 0)) {
      vlogf(LOG_BUG, fmt("Error: creature (%s) with hit > 0 found with position < stunned") % 
	    tmp_ch->getName());
      vlogf(LOG_BUG, "Setting player to POSITION_STANDING");
      tmp_ch->setPosition(POSITION_STANDING);
    }

    if (pl.special_procs) {
      if (tmp_ch->spec) {
	rc = tmp_ch->checkSpec(tmp_ch, CMD_GENERIC_PULSE, "", NULL);
	if (IS_SET_DELETE(rc, DELETE_THIS)) {
	  if (!tmp_ch) continue;

	  temp = tmp_ch->next;
	  delete tmp_ch;
	  tmp_ch = NULL;
	  continue;
	}
      }
    }

    if (pl.drowning) {
      rc = tmp_ch->checkDrowning();
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	temp = tmp_ch->next;
	delete tmp_ch;
	tmp_ch = NULL;
	continue;
      }

      TMonster *tmon = dynamic_cast<TMonster *>(tmp_ch);
      if(tmon){
	tmon->checkResponses((tmon->opinion.random ? tmon->opinion.random : 
			      (tmon->targ() ? tmon->targ() : tmon)),
			     NULL, NULL, CMD_RESP_PULSE);

      }

    }

    if (pl.mobstuff) {
      if (Gravity) {
	tmp_ch->checkSinking(tmp_ch->in_room);

	rc = tmp_ch->checkFalling();
	if (IS_SET_DELETE(rc, DELETE_THIS)) {
	  temp = tmp_ch->next;
	  delete tmp_ch;
	  tmp_ch = NULL;
	  continue;
	}
      }
	
      if (!tmp_ch->isPc() && dynamic_cast<TMonster *>(tmp_ch) &&
	  (zone_table[tmp_ch->roomp->getZoneNum()].zone_value!=1 || 
	   tmp_ch->isShopkeeper() || 
	   IS_SET(tmp_ch->specials.act, ACT_HUNTING))){
	rc = dynamic_cast<TMonster *>(tmp_ch)->mobileActivity(realpulse);
	if (IS_SET_DELETE(rc, DELETE_THIS)) {
	  temp = tmp_ch->next;
	  delete tmp_ch;
	  tmp_ch = NULL;
	  continue;
	}
      }
      if (tmp_ch->task && (realpulse >= tmp_ch->task->nextUpdate)) {
	TObj *tmper_obj = NULL;
	if (tmp_ch->task->obj) {
	  tmper_obj = tmp_ch->task->obj; 
	} 
	rc = (*(tasks[tmp_ch->task->task].taskf))
	  (tmp_ch, CMD_TASK_CONTINUE, "", realpulse, tmp_ch->task->room, tmp_ch->task->obj);
	if (IS_SET_DELETE(rc, DELETE_ITEM)) {
	  if (tmper_obj) {
	    delete tmper_obj;
	    tmper_obj = NULL;
	  } else {
	    vlogf(LOG_BUG, "bad item delete in gameloop -- task calling");
	  }
	}
	if (IS_SET_DELETE(rc, DELETE_THIS)) {
	  temp = tmp_ch->next;
	  delete tmp_ch;
	  tmp_ch = NULL;
	  continue;
	}
      }
    }

    if (pl.combat) {

      if (tmp_ch->isPc() && tmp_ch->desc && tmp_ch->GetMaxLevel() > MAX_MORT &&
	  !tmp_ch->limitPowerCheck(CMD_GOTO, tmp_ch->roomp->number)
	  && !tmp_ch->affectedBySpell(SPELL_POLYMORPH) &&
	  !IS_SET(tmp_ch->specials.act, ACT_POLYSELF)) {
	char tmpbuf[256];
	strcpy(tmpbuf, "");
	tmp_ch->sendTo("An incredibly powerful force pulls you back into Imperia.\n\r");
	act("$n is pulled back whence $e came.", TRUE, tmp_ch, 0, 0, TO_ROOM);
	vlogf(LOG_BUG,fmt("%s was wandering around the mortal world (R:%d) so moving to office.") % 
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


      if (tmp_ch->spelltask) {
	rc = (tmp_ch->cast_spell(tmp_ch, CMD_TASK_CONTINUE, realpulse));
	if (IS_SET_DELETE(rc, DELETE_THIS)) {
	  temp = tmp_ch->next;
	  delete tmp_ch;
	  tmp_ch = NULL;
	  continue;
	}
      }

      rc = tmp_ch->updateAffects();
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	// died in update (disease) 
	temp = tmp_ch->next;
	delete tmp_ch;
	tmp_ch = NULL;
	continue;
	// next line is for the case of doReturn in updateAffects
      } else if (rc == ALREADY_DELETED) continue;

      // this was in hit(), makes more sense here I think
      if (tmp_ch->getMyRace()->hasTalent(TALENT_FAST_REGEN) &&
	  (tmp_ch->getHit() < tmp_ch->hitLimit()) &&
	  !::number(0, 10)){
	// mostly for trolls
	act("You regenerate slightly.", TRUE, tmp_ch, 0, 0, TO_CHAR);
	act("$n regenerates slightly.", TRUE, tmp_ch, 0, 0, TO_ROOM);
	tmp_ch->addToHit(::number(1,6));
      }

      // soak up attack if not in combat
      if ((tmp_ch->cantHit > 0) && !tmp_ch->fight())
	tmp_ch->cantHit--;
    }
    if (pl.teleport) {
      rc = tmp_ch->riverFlow(realpulse);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	temp = tmp_ch->next;
	delete tmp_ch;
	tmp_ch = NULL;
	continue;
      }
      rc = tmp_ch->teleportRoomFlow(realpulse);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	temp = tmp_ch->next;
	delete tmp_ch;
	tmp_ch = NULL;
	continue;
      }
    }
    TMonster *tmon = dynamic_cast<TMonster *>(tmp_ch);
    if (pl.update_stuff) {
      if (!number(0, 3) && !tmp_ch->isPc() && tmon)
	tmon->makeNoise();
    }

    if (!tmp_ch) {
      vlogf(LOG_BUG, "how did we get to here: socket");
      temp = tmp_ch->next;
      continue;
    }


    if (pl.pulse_tick) {
      rc = tmp_ch->updateHalfTickStuff();
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	if (!tmp_ch)
	  continue;

	temp = tmp_ch->next;
	delete tmp_ch;
	tmp_ch = NULL;
	continue;
        // Next line is for the case of doReturn in halfUpdateTickStuff
      } else if (rc == ALREADY_DELETED)  continue;
    }
      
    if (pl.pulse_mudhour) {
      rc = tmp_ch->updateTickStuff();
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	temp = tmp_ch->next;
	if (!dynamic_cast<TBeing *>(tmp_ch)) {
	  // something may be corrupting tmp_ch below - bat 8-18-98
	  vlogf(LOG_BUG, "forced crash.  How did we get here?");
	}
	delete tmp_ch;
	tmp_ch = NULL;
	continue;
      }
    }
	
    // check for vampires in daylight
    if(pl.teleport){
      if(!tmp_ch->roomp->isIndoorSector() && 
	 !tmp_ch->roomp->isRoomFlag(ROOM_INDOORS) &&
	 (tmp_ch->inRoom() != ROOM_VOID) && sunIsUp()){
	    
	if(tmp_ch->hasQuestBit(TOG_VAMPIRE)){
	  act("<r>Exposure to sunlight causes your skin to ignite!<1>",
	      FALSE, tmp_ch, NULL, NULL, TO_CHAR);
	  act("<r>$n's skin ignites in flames as the sunlight shines on $m!<1>",
	      FALSE, tmp_ch, NULL, NULL, TO_ROOM);
	      
	  rc=tmp_ch->reconcileDamage(tmp_ch, ::number(20,200), SPELL_RAZE);
	      
	  if(IS_SET_DELETE(rc, DELETE_THIS)) {
	    if (!tmp_ch) continue;
		
	    temp = tmp_ch->next;
	    delete tmp_ch;
	    tmp_ch = NULL;
	    continue;
	  }
	} else if(tmp_ch->hasQuestBit(TOG_BITTEN_BY_VAMPIRE) &&
		  !::number(0,4)){
	  act("Exposure to sunlight makes your skin itch.",
	      FALSE, tmp_ch, NULL, NULL, TO_CHAR);
	}
      }
    }

    // lycanthrope transformation
    if(pl.teleport){
      if(tmp_ch->hasQuestBit(TOG_LYCANTHROPE) &&
	 !tmp_ch->hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE)
	 && !tmp_ch->isLinkdead() &&
           
	 moonType() == "full" && !sunIsUp() && moonIsUp()) {
	lycanthropeTransform(tmp_ch);
	continue;
      } else if(tmp_ch->hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE)){
	if(moonType() != "full" || sunIsUp() || !moonIsUp()){
	  tmp_ch->remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
	  tmp_ch->doReturn("", WEAR_NOWHERE, CMD_RETURN);
	  continue;
	} else if(!tmp_ch->fight() && tmp_ch->roomp && 
		  !tmp_ch->roomp->isRoomFlag(ROOM_PEACEFUL) &&
		  !::number(0,24)){
	  tmp_ch->setCombatMode(ATTACK_BERSERK);
	  tmp_ch->goBerserk(NULL);
	}
      }
    }


    if (pl.teleport) {
      if (tmp_ch->spec) {
	rc = tmp_ch->checkSpec(tmp_ch, CMD_GENERIC_QUICK_PULSE, "", NULL);
	if (IS_SET_DELETE(rc, DELETE_THIS)) {
	  if (!tmp_ch) continue;
	      
	  temp = tmp_ch->next;
	  delete tmp_ch;
	  tmp_ch = NULL;
	  continue;
	}
      }
    }

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

    if(gameLoopTiming){
      rc=(int)(t.getElapsedReset()*1000000);
      
      if(rc>1000){
	vlogf(LOG_MISC, fmt("characterPulse: %s: %i") % 
	      tmp_ch->getName() % rc);
      }
    }

    temp = tmp_ch->next;


  } // character_list

  return retcount-count;
}


int TMainSocket::objectPulse(TPulseList &pl, int realpulse)
{
  TVehicle *vehicle;
  int rc, count, retcount;
  TObj *obj;

  if(!placeholder){
    placeholder=read_object(1, VIRTUAL); // hairball, dummy object
    // get an iterator for our placeholder
    iter=find(object_list.begin(), object_list.end(), placeholder);
  }

  // note on this loop
  // it is possible that next_thing gets deleted in one of the sub funcs
  // we don't get acknowledgement of this in any way.
  // to avoid problems this might cause, we reinitialize at
  // the end (eg, before any deletes, or before we come back around)
  // bottom line is that next_thing keeps getting set because it might be
  // bogus after the function call.

  ++vehiclepulse;


  // we want to go through 1/12th of the object list every pulse
  // obviously the object count will change, so this is approximate.
  retcount=count=(int)((float)objCount/11.5);
  
  while(count--){
    // remove placeholder from object list and increment iterator
    object_list.erase(iter++);
    
    // set object to be processed
    obj=(*iter);
    
    // move to front of list if we reach the end
    // otherwise just stick the placeholder in
    if(++iter == object_list.end()){
      object_list.push_front(placeholder);
      iter=object_list.begin();
    } else {
      object_list.insert(iter, placeholder);
      --iter;
    }


    if (!dynamic_cast<TObj *>(obj)) {
      vlogf(LOG_BUG, fmt("Object_list produced a non-obj().  rm: %d") %  obj->in_room);
      vlogf(LOG_BUG, fmt("roomp %s, parent %s") %  
	    (obj->roomp ? "true" : "false") %
	    (obj->parent ? "true" : "false"));
      // bogus objects tend to have garbage in obj->next
      // it would be dangerous to continue with this loop
      // this is called often enough that one skipped iteration should
      // not be noticed.  Therefore, break out.
      break;
    }

    // vehicle movement
    if((vehicle=dynamic_cast<TVehicle *>(obj)))
      vehicle->vehiclePulse(vehiclepulse);
	
    // this stuff all happens every time we go through here, which is
    // about every 12 pulses, ie "combat" or "teleport" pulse
    rc = obj->detonateGrenade();
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete obj;
      continue;
    }
    rc = obj->checkFalling();
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete obj;
      continue;
    }
    rc = obj->riverFlow(realpulse);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete obj;
      continue;
    }
    if (obj->spec) {
      rc = obj->checkSpec(NULL, CMD_GENERIC_QUICK_PULSE, "", NULL);
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
	delete obj;
	continue;
      }
      if (rc) {
	continue;
      }
    }
    // end 12 pulse

    if (pl.special_procs) { // 36
      check_sinking_obj(obj, obj->in_room);
      if (obj->spec) {
	rc = obj->checkSpec(NULL, CMD_GENERIC_PULSE, "", NULL);
	if (IS_SET_DELETE(rc, DELETE_ITEM)) {
	  delete obj;
	  obj = NULL;
	  continue;
	}
	if (rc) {
	  continue;
	}
      }

      rc = obj->updateBurning();
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	delete obj;
	obj = NULL;
	continue;
      }

      // fun with smoke
      TSmoke *smoke=dynamic_cast<TSmoke *>(obj);
      if(smoke){
	smoke->doMerge();
	smoke->doDrift();
	smoke->doChoke();
      }

      // trash piles
      if(!::number(0,999))
	obj->joinTrash();
      
      TTrashPile *pile=dynamic_cast<TTrashPile *>(obj);
      if(pile){
	// delete empty piles
	if(!pile->getStuff()){
	  delete obj;
	  obj = NULL;
	  continue;
	} else {
	  pile->doMerge();
	  pile->overFlow();
	  pile->attractVermin();
	  pile->updateDesc();
	}
      }

	

    }

    if (pl.pulse_mudhour) { // 1440
      rc = obj->objectTickUpdate(realpulse);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	delete obj;
	obj = NULL;
	continue;
      }
    }
  } // object list

  return retcount-count;
}

void pingData()
{
  static FILE *p;
  Descriptor *d;
  
  if(p) pclose(p);
  
  if(gamePort == PROD_GAMEPORT){
    p=popen("/mud/prod/lib/bin/ping sneezy", "w");
  } else if(gamePort == BUILDER_GAMEPORT){
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


int TMainSocket::gameLoop()
{
  Descriptor *point;
  int pulse = 0;
  TPulseList pl;
  time_t lagtime_t = time(0);
  sstring str;
  int count;
  struct timeval timespent;
  bool doneBankInterest=false;
  TTiming t;

  avail_descs = 150;		

  // players may have connected before this point via 
  // addNewDescriptorsDuringBoot, so send all those descriptors the login
  for (point = descriptor_list; point; point = point->next)
    if (!point->m_bIsClient)
      point->sendLogin("1");

  time_t ticktime = time(0);

  while (!handleShutdown()) {
    timespent=handleTimeAndSockets();
    
    if(gameLoopTiming){
      count=((timespent.tv_sec*1000000)+timespent.tv_usec);
      
      vlogf(LOG_MISC, fmt("%i %i) handleTimeAndSockets: %i (sleep = %i)") %
	    pulse % (pulse%12) % 
	    (int)((t.getElapsedReset()*1000000)-count) % count);
    }
    
    // setup the pulse boolean flags
    pulse++;
    pl.init(pulse);

    // interport communication
    //mudRecvMessage();
    //pulseLog("mudRecvMessage", t, pulse);

    if(pl.wayslowpulse){
      checkForRepo();
      do_check_mail();
    }

    pulseLog("wayslowpulse", t, pulse);

    if (pl.pulse_tick) {
      // these are done per tick (15 mud minutes)
      doGlobalRoomStuff();
      pulseLog("doGlobalRoomStuff", t, pulse);

      deityCheck(FALSE);
      pulseLog("deityCheck", t, pulse);

      apocCheck();
      pulseLog("apocCheck", t, pulse);

      save_factions();
      pulseLog("save_factions", t, pulse);

      save_newfactions();
      pulseLog("save_newfactions", t, pulse);

      weatherAndTime(1);
      pulseLog("weatherAndTime", t, pulse);

      count=updateWholist();
      pulseLog("updateWholist", t, pulse);

      updateUsagelogs(count);
      pulseLog("updateUsagelogs", t, pulse);
    }


    // once per mud day
    if(time_info.seconds==0 && time_info.hours==0 && time_info.minutes==0){
      if(!doneBankInterest){
	calcBankInterest();
	pulseLog("calcBankInterest", t, pulse);
	doneBankInterest=true;
      }
    } else {
      doneBankInterest=false;
    }

    if (pl.combat){
      perform_violence(pulse);
      pulseLog("perform_violence", t, pulse);
    }

    if (pl.pulse_mudhour) {
      // these are done per mud hour
      recalcFactionPower();
      pulseLog("recalcFactionPower", t, pulse);

      pulseLog("Dealing with mob hate.", t, pulse);
      TBeing * b = NULL;

      for (b = character_list; b; b = b->next) {
        TMonster *tmons = dynamic_cast<TMonster *>(b);
        charList *list;

        if (tmons && IS_SET(tmons->hatefield, HATE_CHAR) && tmons->hates.clist)
          for (list = tmons->hates.clist; list; list = list->next)
            if (list->name) {
              list->iHateStrength--;

              if (list->iHateStrength <= 0) {
                vlogf(LOG_LAPSOS, fmt("%s no longer hates %s") % tmons->getName() % list->name);

                tmons->remHated(NULL, list->name);
              }
            }
      }

      // adjust zones for nuking
      if (nuke_inactive_mobs){
	nukeInactiveMobs();
	pulseLog("nukeInactiveMobs", t, pulse);
      }

      // weather and time stuff
      if (time_info.hours == 1){
	update_time();
	pulseLog("update_time", t, pulse);
      }

      zone_update();
      pulseLog("zone_update", t, pulse);

      do_components(-1);
      pulseLog("do_components", t, pulse);

      launch_caravans();
      pulseLog("launch_caravans", t, pulse);

      updateAvgPlayers();
      pulseLog("updateAvgPlayers", t, pulse);

      checkGoldStats();
      pulseLog("checkGoldStats", t, pulse);

      sendAutoTips();
      pulseLog("sendAutoTips", t, pulse);

      pingData();
      pulseLog("pingData", t, pulse);

      handleFishRespawning();
      pulseLog("handleFishRespawning", t, pulse);
    }


    // set zone emptiness flags
    for (unsigned int i = 0; i < zone_table.size(); i++)
      zone_table[i].zone_value=zone_table[i].isEmpty()?1:-1;

    // room procs
    call_room_specials();
    pulseLog("call_room_specials", t, pulse);


    if(gameLoopTiming)
      vlogf(LOG_MISC, fmt("%i %i) normal pulses: %s") % 
	    pulse % (pulse%12) % pl.showPulses());

    // since we're operating on non-multiples of 12 pulses, we need to
    // temporarily put the pulse at the next multiple of 12
    // this is pretty klugey
    int oldpulse=pulse;
    while(pulse % 12)
      ++pulse;

    // reset the pulse flags
    pl.init(pulse);

    if(gameLoopTiming){
      vlogf(LOG_MISC, fmt("%i %i) split pulses: %s") % 
	    oldpulse % (oldpulse%12) % pl.showPulses());

      pulseLog("gameLoop1", t, oldpulse);
    }

    // handle pulse stuff for objects
    count=objectPulse(pl, pulse);

    if(gameLoopTiming)
      vlogf(LOG_MISC, fmt("%i %i) objectPulse: %i, %i objs") % 
	    oldpulse % (oldpulse%12) % 
	    (int)(t.getElapsedReset()*1000000) % count);
    
    // handle pulse stuff for mobs and players
    count=characterPulse(pl, pulse);

    if(gameLoopTiming)
      vlogf(LOG_MISC, fmt("%i %i) characterPulse: %i, %i chars") %
	    oldpulse % (oldpulse%12) % 
	    (int)(t.getElapsedReset()*1000000) % count);

    // reset the old values from the artifical pulse
    pulse=oldpulse;
    pl.init(pulse);



    // get some lag info
    // this needs to remain at pulse%100
    if (!(pulse %100)){
      int which=(pulse/100)%10;
      
      lag_info.current=lag_info.lagtime[which]=time(0)-lagtime_t;
      lagtime_t=time(0);
      lag_info.lagcount[which]=1;

      lag_info.high = max(lag_info.lagtime[which], lag_info.high);
      lag_info.low = min(lag_info.lagtime[which], lag_info.low);
    }

    if (pulse >= 2400) {
      ticktime = time(0);

      // THIS PULSE = 0 IS NOT SIMPLY FOR LOGGING PURPOSES.
      // if it gets removed all tasks go into hyper mode. So don't.
      // (appears to be true: some functions above take the pulse as
      //  an argument - peel 04/20/04)
      pulse = 0;
    }
    pulseLog("lag_info", t, pulse);

    systask->CheckTask();
    tics++;			// tics since last checkpoint signal 
    pulseLog("CheckTask", t, pulse);
  }
  ares_destroy(channel);
  return TRUE;
}


TSocket *TMainSocket::newConnection(int t_sock)
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
  buf = fmt("%d.%d.%d.%d") % n4 % n3 % n2 % n1;
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

    vlogf(LOG_MISC, fmt("gethostbyaddr_cb: %s: %s") % ip_string % ares_strerror(status, &ares_errmem));
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

    vlogf(LOG_MISC, fmt("gethostbyaddr_cb: %s resolved to %-32s") % ip_string % resolved_name);

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

int TMainSocket::newDescriptor(int t_sock)
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

  if (!(s = newConnection(t_sock))) 
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

void TMainSocket::initSocket(int m_port)
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
    vlogf(LOG_BUG, fmt("failed getting hostname structure.  hostname: %s") %  hostname);
    perror("gethostbyname");
    exit(1);
  }
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons(m_port);
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
    vlogf(LOG_BUG, fmt("initSocket: bind: errno=%d") %  errno);
    close(t_sock);
    exit(0);
  }
  listen(t_sock, 3);

  m_sock.push_back(t_sock);
}

TSocket::TSocket()
{
}

TSocket::~TSocket()
{
}

TMainSocket::TMainSocket()
{
  tmp_ch=NULL;
  placeholder=NULL;
  vehiclepulse=0;
}

TMainSocket::~TMainSocket()
{
}
