//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include <csignal>
#include <cstdarg>
#include <errno.h>

extern "C" {
#include <dirent.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/param.h>
#include <arpa/telnet.h>
#include <arpa/inet.h>

#if defined(LINUX) || defined(SOLARIS)
#include <sys/stat.h>
#endif
}

#include "extern.h"
#include "being.h"
#include "client.h"
#include "handler.h"
#include "low.h"
#include "person.h"
#include "monster.h"
#include "configuration.h"
#include "charfile.h"
#include "account.h"
#include "statistics.h"
#include "socket.h"
#include "mail.h"
#include "games.h"
#include "person.h"
#include "cmd_trophy.h"
#include "colorstring.h"
#include "database.h"
#include "rent.h"
#include "shop.h"
#include "weather.h"

const int DONT_SEND = -1;
const int FORCE_LOW_INVSTE = 1;

static const char * const WIZLOCK_PASSWORD           = "motelvi";
const char * const MUD_NAME      = "SneezyMUD";
const char * const MUD_NAME_VERS = "SneezyMUD v5.2";
static const char * const WELC_MESSG = "\n\rWelcome to SneezyMUD 5.2! May your journeys be enjoyable!\n\r\n\r";

Descriptor::Descriptor() :
  input(false),
  ignored(this)
{
  // this guy is private to prevent being called
  // just need to init member vars that are appropriate
}

Descriptor::Descriptor(TSocket *s) :
  host_resolved(false),
  socket(s),
  edit(),
  connected(CON_CREATION_START),
  wait(1),
  showstr_head(NULL),
  tot_pages(0),
  cur_page(0),
  str(NULL),
  max_str(0),
  prompt_mode(0),
  output(),
  input(true),
  session(),
  career(),
  autobits(0),
  best_rent_credit(0),
  playerID(0),
  character(NULL),
  account(NULL),
  original(NULL),
  snoop(),
  next(descriptor_list),
  pagedfile(NULL),
  amount(0),
  obj(NULL),
  mob(NULL),
  bet(),
  bet_opt(),
  screen_size(24),
  point_roll(0),
  talkCount(time(0)),
  m_bIsClient(FALSE),
  bad_login(0),
  severity(0),
  office(0),
  blockastart(0),
  blockaend(0),
  blockbstart(0),
  blockbend(0),
  last(),
  deckSize(0),
  prompt_d(),
  plr_act(0),
  plr_color(0),
  plr_colorSub(COLOR_SUB_NONE),
  plr_colorOff(0),
  ignored(this)
{
  int i;

  *m_raw = '\0';
  *delname = '\0';

  for (i = 0; i < HISTORY_SIZE; i++)
    *history[i] = '\0';
  
  descriptor_list = this;
}

Descriptor::Descriptor(const Descriptor &a) :
  host_resolved(a.host_resolved),
  socket(a.socket),
  edit(a.edit),
  connected(a.connected),
  wait(a.wait),
  tot_pages(a.tot_pages),
  cur_page(a.cur_page),
  max_str(a.max_str),
  prompt_mode(a.prompt_mode),
  output(a.output),
  input(a.input),
  session(a.session),
  career(a.career),
  autobits(a.autobits),
  best_rent_credit(a.best_rent_credit),
  playerID(a.playerID),
  character(a.character),
  account(a.account),
  original(a.original),
  snoop(a.snoop),
  next(descriptor_list),
  amount(a.amount),
  obj(a.obj),
  mob(a.mob),
  bet(a.bet),
  bet_opt(a.bet_opt),
  screen_size(a.screen_size),
  point_roll(a.point_roll),
  talkCount(a.talkCount),
  m_bIsClient(a.m_bIsClient),
  bad_login(a.bad_login),
  severity(a.severity),
  office(a.office),
  blockastart(a.blockastart),
  blockaend(a.blockaend),
  blockbstart(a.blockbstart),
  blockbend(a.blockbend),
  last(a.last),
  deckSize(a.deckSize),
  prompt_d(a.prompt_d),
  plr_act(a.plr_act),
  plr_color(a.plr_color),
  plr_colorSub(a.plr_colorSub),
  plr_colorOff(a.plr_colorOff),
  ignored(this)
{
  int i;

  // not sure how this is being used, theoretically correct, but watch
  // for duplication stuff
  // str may also be prolematic
  vlogf(LOG_BUG, "Inform Batopr immediately that Descriptor copy constructor was called.");

  showstr_head = mud_str_dup(a.showstr_head);
//  str = mud_str_dup(a.str);
  str = NULL;
  pagedfile = mud_str_dup(a.pagedfile);

  strcpy(m_raw, a.m_raw);
  strcpy(delname, a.delname);

  for (i = 0; i < HISTORY_SIZE; i++)
    strcpy(history[i], a.history[i]);
  
  descriptor_list = this;
}

Descriptor & Descriptor::operator=(const Descriptor &a) 
{
  if (this == &a) return *this;

  // not sure how this is being used, theoretically correct, but watch
  // for duplication stuff
  // str is most likely also screwy
  vlogf(LOG_BUG, "Inform Batopr immediately that Descriptor operator= was called.");

  host_resolved = a.host_resolved;
  socket = a.socket;
  edit = a.edit;
  connected = a.connected;
  wait = a.wait;
  tot_pages = a.tot_pages;
  cur_page = a.cur_page;
  max_str = a.max_str;
  prompt_mode = a.prompt_mode;
  output = a.output;
  input = a.input;
  session = a.session;
  career = a.career;
  autobits = a.autobits;
  best_rent_credit = a.best_rent_credit;
  playerID=a.playerID;
  character = a.character;
  account = a.account;
  original = a.original;
  snoop = a.snoop;
  obj = a.obj;
  mob = a.mob;
  bet = a.bet;
  bet_opt = a.bet_opt;
  screen_size = a.screen_size;
  point_roll = a.point_roll;
  talkCount = a.talkCount;
  m_bIsClient = a.m_bIsClient;
  bad_login = a.bad_login;
  severity = a.severity;
  office = a.office;
  blockastart = a.blockastart;
  blockaend = a.blockaend;
  blockbstart = a.blockbstart;
  blockbend = a.blockbend;
  last = a.last;
  deckSize = a.deckSize;
  prompt_d = a.prompt_d;
  plr_act = a.plr_act;
  plr_color = a.plr_color;
  plr_colorSub = a.plr_colorSub;
  plr_colorOff = a.plr_colorOff;
  amount = a.amount;

  delete [] showstr_head;
  showstr_head = mud_str_dup(a.showstr_head);

//  delete [] str;
//  str = mud_str_dup(a.str);
  str = NULL;

  delete [] pagedfile;
  pagedfile = mud_str_dup(a.pagedfile);

  strcpy(m_raw, a.m_raw);
  strcpy(delname, a.delname);

  for (int i = 0; i < HISTORY_SIZE; i++)
    strcpy(history[i], a.history[i]);
  
  return *this;
}

// returns TRUE if multiplay is detected
bool Descriptor::checkForMultiplay()
{
  if(Config::CheckMultiplay()){
    TBeing *ch;
    unsigned int total = 1;
    Descriptor *d;
    
    if (!character || !account || !character->name)
      return FALSE;
    
    if (gamePort == Config::Port::ALPHA)
      return FALSE;
    
    if (character->hasWizPower(POWER_MULTIPLAY))
      return FALSE;
    
    // determine player load
    unsigned int tot_descs = 0;
    for (d = descriptor_list; d; d = d->next)
      tot_descs++;
    
    // established maximums based on player load thresholds
    unsigned int max_multiplay_chars;
    if (tot_descs < 15)
      max_multiplay_chars = 1;
    else if (tot_descs < 30)
      max_multiplay_chars = 1;
    else if (tot_descs < 60)
      max_multiplay_chars = 1;
    else
      max_multiplay_chars = 1;
    
    // for first 30 mins after a reboot, limit to 1 multiplay
    // this prevents a "race to login" from occurring.
    time_t diff = time(0) - Uptime;
    if (diff < (30 * SECS_PER_REAL_MIN))
      max_multiplay_chars = 1;
    
    for (d = descriptor_list; d; d = d->next) {
      if (d == this)
	continue;
      if (!(ch = d->character) || !ch->name)
	continue;
      
      if (ch->hasWizPower(POWER_MULTIPLAY))
	continue;
      
      // reconnect while still connected triggers otherwise
      if (!strcmp(character->name, ch->name))
	continue;
      
      if (d->account->name==account->name) {
	total += 1;
	if (total > max_multiplay_chars &&
	    gamePort == Config::Port::PROD){
	  vlogf(LOG_CHEAT, format("MULTIPLAY: %s and %s from same account[%s]") % 
		character->name % ch->name % account->name);
	  if(Config::ForceMultiplayCompliance()){
	    character->sendTo(format("\n\rTake note: You have another character, %s, currently logged in.\n\r") % ch->name);
	    character->sendTo("Adding this character would cause you to be in violation of multiplay rules.\n\r");
	    character->sendTo("Please log off your other character and then try again.\n\r");
	    outputProcessing();  // gotta write this to them, before we sever  :)
	  }
	  
	  return TRUE;
	}
      }
      
#if 0
      // beyond here, we start checking for cheaters using multiple accounts
      // since this logic is imprecise, we should first slip around any known
      // accounts that tend to trigger this.
      FILE *fp;
      fp = fopen("allowed_multiplay", "r");
      if (fp) {
	char acc1[256], acc2[256];
	bool allowed = false;
	while (fscanf(fp, "%s %s", acc1, acc2) == 2) {
	  if (!strcmp(account->name, acc1) && !strcmp(d->account->name, acc2))
	    allowed = true;
	  if (!strcmp(account->name, acc2) && !strcmp(d->account->name, acc1))
	    allowed = true;
	}
	fclose(fp);
	if (allowed)
	  continue;
      }
      
      if (max_multiplay_chars == 1) {
	// some diabolical logic to catch multiplay with separate accounts
	// check to see if they are grouped, but haven't spoken recently
	if (character->inGroup(*ch)) {
	  time_t now = time(0);
	  const int trigger_minutes = 1;
	  if (((now - talkCount) > ((trigger_minutes + character->getTimer()) * SECS_PER_REAL_MIN)) &&
	      ((now - ch->desc->talkCount) > ((trigger_minutes + ch->getTimer()) * SECS_PER_REAL_MIN))) {
	    vlogf(LOG_CHEAT, format("MULTIPLAY: Players %s and %s are possibly multiplaying.") %  character->getName() % ch->getName());
	    
	    time_t ct = time(0);
	    struct tm * lt = localtime(&ct);
	    char *tmstr = asctime(lt);
	    *(tmstr + strlen(tmstr) - 1) = '\0';
	    
	    sstring tmpstr;
	    tmpstr = "***** Auto-Comment on ";
	    tmpstr += tmstr;
	    tmpstr += ":\nPlayer ";
	    tmpstr += character->getName();
	    tmpstr += " was potentially multiplaying with ";
	    tmpstr += ch->getName();
	    tmpstr += " from account '";
	    tmpstr += d->account->name;
	    tmpstr += "'.\n";
	    
	    sstring cmd_buf;
	    cmd_buf = "account/";
	    cmd_buf += LOWER(account->name[0]);
	    cmd_buf += "/";
	    cmd_buf += account->name.lower();
	    cmd_buf += "/comment";
	    
	    FILE *fp;
	    if (!(fp = fopen(cmd_buf.c_str(), "a+"))) {
	      perror("doComment");
	      vlogf(LOG_FILE, format("Could not open the comment-file (%s).") %  cmd_buf);
	    } else {
	      fputs(tmpstr.c_str(), fp);
	      fclose(fp);
	    }
	  }
	}
      } // CHAR_LIMIT = 1
#endif
    }
    
    if (character && account && !account->name.empty() && 
	!character->hasWizPower(POWER_MULTIPLAY)) {
      TBeing *tChar = NULL,
	*oChar = NULL;
      char tAccount[256];
      FILE *tFile = NULL;
      
      for (tChar = character_list; tChar;) {
	oChar = tChar->next;
	
	if (tChar != character && tChar->isLinkdead()) {
	  sprintf(tAccount, "account/%c/%s/%s",
		  LOWER(account->name[0]),
		  sstring(account->name).lower().c_str(),
		  tChar->getNameNOC(character).lower().c_str());
	  
	  if (tChar->isPc() && (tFile = fopen(tAccount, "r"))) {
	    if (tChar->hasWizPower(POWER_MULTIPLAY))
	      return FALSE;
	    
	    fclose(tFile);
#if 1
	    character->sendTo(format("\n\rTake note: You have a link-dead character, %s, currently logged in.\n\r") % tChar->name);
	    character->sendTo("Adding this character would cause you to be in violation of multiplay rules.\n\r");
	    character->sendTo("Please reconnect your other character to log them off and then try again.\n\r");
	    outputProcessing();  // gotta write this to them, before we sever  :)
	    return TRUE;
#else
	    nukeLdead(tChar);
	    delete tChar;
	    tChar = NULL;
#endif
	  }
	}
	
	if (!(tChar = oChar))
	  return FALSE;
      }
    }
  }

  return FALSE;
}

sstring SnoopComm::getText(){
  return "<r>%<z> " + text;
}

sstring SnoopComm::getClientText(){
  return getText();
}

sstring SnoopComm::getXML(){
  return format("<snoop victim=\"%s\">%s</snoop>") % 
    vict.escape(sstring::XML) % text.escape(sstring::XML);
}

int Descriptor::outputProcessing()
{
  // seems silly, but we sometimes do descriptor_list->outputProcessing()
  // to send everyone their output.  We need to check for the no-one-connected
  // state just for sanity.
  if (!this)
    return 1;

  char i[MAX_STRING_LENGTH + MAX_STRING_LENGTH];
  int counter = 0;
  char buf[MAX_STRING_LENGTH + MAX_STRING_LENGTH];
  Comm *c;
  Comm::commTypeT commtype;
  TBeing *ch = original ? original : character;

  if (!prompt_mode && !connected && !m_bIsClient)
    if (socket->writeToSocket("\n\r") < 0)
      return -1;

  memset(i, '\0', sizeof(i));
  // Take everything from queued output
  while (c=output.takeFromQ()) {
    if(m_bIsClient){
      commtype=Comm::CLIENT;
    } else if(socket->port==Config::Port::PROD_XML){
      commtype=Comm::XML;
    } else {
      commtype=Comm::TEXT;
    }

    strncpy(i, c->getComm(commtype).c_str(), MAX_STRING_LENGTH + MAX_STRING_LENGTH);
    counter++;

    // I bumped this from 500 to 1000 - Batopr
    // It happens sporadically if a lagged person is dragged on a long
    // speed walk
    if (counter >= 5000) {
      char buf2[MAX_STRING_LENGTH + MAX_STRING_LENGTH];
      strcpy(buf2, i);
      vlogf(LOG_BUG, format("Tell a coder, bad loop in outputProcessing, please investigate %s") %  (character ? character->getName() : "'No char for desc'"));
      vlogf(LOG_BUG, format("i = %s, last i= %s") %  buf2 % buf); 
      // Set everything to NULL, might lose memory but we dont wanna try
      // a delete cause it might crash/ - Russ
      output.clear();
      break;
    } 
    // recall that in inputProcessing we have mangaled any '$' character into
    // '$$' to avoid confusion problems in act/sendTo/etc.
    // it's now time to undo this
    // the tb stuff is so if they type '$$', it becomes '$$$$' in input,
    // and we want to return this to '$$' for output
    char *tb = i;
    char *tc;
    while ((tc = strstr(tb, "$$"))) {
      // note we are shrinking i by 1 char
      // in essence, replace "$$" with a "$"
      char *tmp = mud_str_dup(i);
      int amt = tc - i;
      strcpy(tc, tmp+amt+1);
      delete [] tmp;
      tb = tc+1;
    }
    strcpy(buf, i);
    if (snoop.snoop_by && snoop.snoop_by->desc) {
      snoop.snoop_by->desc->output.putInQ(new SnoopComm(ch->getName(), i));
    }

    // color processing
    sstring colorBuf=colorString(ch, this, i, NULL, COLOR_BASIC, FALSE);

    if (socket->writeToSocket(colorBuf.c_str()))
      return -1;

    if(commtype == Comm::XML)
      socket->writeNull();

    memset(i, '\0', sizeof(i));
    delete c;
  }
  return (1);
}

Descriptor::~Descriptor()
{
  Descriptor *tmp;
  int num = 0;
  TThing *th=NULL, *th2=NULL;
  TRoom *rp;

  if (close(socket->m_sock))
    vlogf(LOG_BUG, format("Close() exited with errno (%d) return value in ~Descriptor") %  errno);
  
  // clear out input/output buffers
  flush();

  // This is a semi-kludge to fix some extra crap we had being sent
  // upon reconnect - Russ 6/15/96
  if (socket->m_sock == maxdesc) 
    --maxdesc;

  // clear up any editing sstrings
  cleanUpStr();

  // Forget snoopers
  if (snoop.snooping)
    snoop.snooping->desc->snoop.snoop_by = 0;

  if (snoop.snoop_by && snoop.snoop_by->desc) {
    snoop.snoop_by->sendTo("Your victim is no longer among us.\n\r");
    snoop.snoop_by->desc->snoop.snooping = 0;
  }
  if (character) {
    if (original)
      character->remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
      character->doReturn("", WEAR_NOWHERE, CMD_RETURN);

    if ((connected >= CON_REDITING) || !connected) {
      if ((connected == CON_OEDITING) && obj) {
        delete obj;
        obj = NULL;
      }
      if ((connected == CON_MEDITING) && mob) {
        extract_edit_char(mob);
        mob = NULL;
      }
      if ((connected == CON_SEDITING) && mob) {
        extract_edit_char(mob);
        mob = NULL;
      }
      if (connected == CON_REDITING) 
        character->roomp->removeRoomFlagBit(ROOM_BEING_EDITTED);
      if ((character->checkBlackjack()) &&
          (gBj.index(character) >= 0)) {
        gBj.exitGame(character);
      }

      act("$n has lost $s link.", TRUE, character, 0, 0, TO_ROOM);
      vlogf(LOG_PIO, format("Closing link to: %s.") %  character->getName());

      // this is partly a penalty for losing link (lose followers)
      // the more practical reason is that the mob and items are saved
      // if the player linkdead and someone comes in and purges the
      // mob, the numbers on the mob's item get messed up
      // this isn't necessarily limited to purge.  The basic problem is
      // the item is in two places (in the mob's rent, and on the mob still
      // in game).
      // The solution seems to remove one of them, and this was just easier.
      // plus it's an incentive not to linkdrop.
      // in any event, if they reconnect, the mob is still there following
      // the first save will recreate the followers...
      character->removeFollowers();

      for(StuffIter it=character->stuff.begin();it!=character->stuff.end() && (th=*it);++it) {
        if (th) {
          for(StuffIter it=th->stuff.begin();it!=th->stuff.end() && (th2=*it);++it)
            num++;
          num++;
        }        
      }

      for (int i = MIN_WEAR; i < MAX_WEAR; i++) {
        if ((th = character->equipment[i])) {
          for(StuffIter it=th->stuff.begin();it!=th->stuff.end() && (th2=*it);++it)
            num++;

          num++;
        }
      }
      vlogf(LOG_PIO, format("Link Lost for %s: [%d talens/%d bank/%.2f xps/%d items/%d age-mod/%d rent]") % 
            character->getName() % character->getMoney() % character->getBank() %
            character->getExp() % num % character->age_mod % 
            (dynamic_cast<TPerson *>(character)?dynamic_cast<TPerson *>(character)->last_rent:0));
      character->desc = NULL;
      if((!character->affectedBySpell(AFFECT_PLAYERKILL) &&
          !character->affectedBySpell(AFFECT_PLAYERLOOT)) ||
	  character->isImmortal()) {
	character->setInvisLevel(GOD_LEVEL1);
      }

      if (character->riding) 
        character->dismount(POSITION_STANDING);
      
      // this is done out of nceness to keep people from walking linkdeads 
      // to someplace nasty
      if (!character->isAffected(AFF_CHARM)) {
        if (character->master)
          character->stopFollower(TRUE);
      }
      character->fixClientPlayerLists(TRUE);
    } else {
      if (connected == CON_PWDNRM)
        bad_login++;
      if (character->getName())
        vlogf(LOG_PIO, format("Losing player: %s [%s].") %  character->getName() % host);

      // shove into list so delete works OK
      character->desc = NULL;
      character->next = character_list;
      character_list = character;

      if (character->inRoom() >= 0) {
        // loadFromSt will have inRoom() == last rent
        // roomp not set yet, so just clear this value
        character->setRoom(Room::VOID);
      }
      rp = real_roomp(Room::VOID);
      *rp += *character;
      delete character;
      character = NULL;
    }
  }
  // to avoid crashing the process loop
  while (next_to_process && next_to_process == this)
    next_to_process = next_to_process->next;

  if (this == descriptor_list)
    descriptor_list = descriptor_list->next;
  else {
    for (tmp = descriptor_list; tmp && (tmp->next != this); tmp = tmp->next);

    mud_assert(tmp != NULL, 
                 "~Descriptor : unable to find previous descriptor.");

    if (tmp)
      tmp->next = next;
  }
  delete [] showstr_head;
  showstr_head = NULL;

  tot_pages = cur_page = 0;

  delete [] pagedfile;
  pagedfile = NULL;

  delete account;
  account = NULL;

  delete socket;
  // Was I editing something???
  // These are not in char_list or obj_list anymore, so don't leak them
  // we already did some MEDIT/OEDIT checks above for obvious handling

  // this is mostly just for sanity
  // we can get here if lose-link while editing, and reconnect
#if 1
  // remove me if stable, i don't think mob would ever be in list
  if (mob) {
    TBeing *tch;
    bool found = false;
    for (tch = character_list; tch; tch = tch->next) {
      if (tch == mob) {
        vlogf(LOG_BUG, "Whoa bad!, in list but adding to list again");
        found = true;
      }
    }
    if (!found) {
      mob->next = character_list;
      character_list = mob;
    }
  }
#endif

  delete mob;
  delete obj;
}

void Descriptor::cleanUpStr()
{
  if (str) {
    if (character && 
          (character->isPlayerAction(PLR_MAILING) ||
           character->isPlayerAction(PLR_BUGGING))) {
      delete [] *str;
      *str = NULL;
      delete [] str;
      str = NULL;
    } else if (character &&
                  (connected == CON_WRITING ||
                   connected == CON_REDITING ||
                   connected == CON_OEDITING ||
                   connected == CON_MEDITING ||
                   connected == CON_SEDITING)) {
      // the str is attached to the mob/obj/room, so this is OK
    } else
      vlogf(LOG_BUG, "Descriptor::cleanUpStr(): Probable memory leak");
  }
}

void Descriptor::flushInput()
{
  char dummy[MAX_STRING_LENGTH];

  while (input.takeFromQ(dummy, sizeof(dummy)));
}

void Descriptor::flush()
{
  char dummy[MAX_STRING_LENGTH];

  output.clear();
  while (input.takeFromQ(dummy, sizeof(dummy)));
}

void Descriptor::add_to_history_list(const char *arg)
{
  int i;

  unsigned int hist_size = sizeof(history[0]) * sizeof(char);
  for (i = HISTORY_SIZE-1; i >= 1; i--) {
    strncpy(history[i], history[i - 1], hist_size - 1);
    history[i][hist_size-1] = '\0';
  }

  strncpy(history[0], arg, hist_size - 1);
  history[0][hist_size-1] = '\0';
}

void TPerson::autoDeath()
{
  char buf[1024];

  vlogf(LOG_PIO, format("%s reconnected with negative hp, auto death occurring.") %  
                        getName());
  sendTo("You reconnected with negative hit points, automatic death occurring.");
  sprintf(buf, "%s detected you reconnecting with %d hit points.\n\r", MUD_NAME, getHit());
  sprintf(buf + strlen(buf), "In order to discourage people from dropping link in order to avoid death,\n\r");
  sprintf(buf + strlen(buf), "it was decided that such an event would result in a partial death.\n\r");
  sprintf(buf + strlen(buf), "As such, you have been penalized %.2f experience.\n\r\n\r", deathExp());
  sprintf(buf + strlen(buf), "Please understand that this is a code generated\n\r");
  sprintf(buf + strlen(buf), "message and does not come from any particular god.\n\r");
  sprintf(buf + strlen(buf), "If you have problems or questions regarding this action, please feel free to\n\r");
  sprintf(buf + strlen(buf), "contact a god.  Type WHO -G to see if any gods are connected.\n\r");
            
  autoMail(this, NULL, buf);
  gain_exp(this, -deathExp(), -1);
  genericKillFix();
  return;
}

bool MakeTimeT(int tMon, int tDay, int tYear, time_t tLast)
{
  time_t     tCurrent = time(0);
  struct tm  tTemp,
           * tTime = localtime(&tCurrent);
  int        tTempDay = 0;

  tTemp.tm_sec   = 0;
  tTemp.tm_min   = 0;
  tTemp.tm_hour  = 0;
  tTemp.tm_isdst = tTime->tm_isdst;
  tTemp.tm_mon   = (tMon - 1);
  tTemp.tm_mday  = tDay;
  tTemp.tm_year  = (in_range(tYear, 0, 50) ? (tYear + 100) : tYear);

  switch (tTemp.tm_mon) {
    case 10:
      tTempDay += 31;
    case 9:
      tTempDay += 30;
    case 8:
      tTempDay += 31;
    case 7:
      tTempDay += 30;
    case 6:
      tTempDay += 31;
    case 5:
      tTempDay += 31;
    case 4:
      tTempDay += 30;
    case 3:
      tTempDay += 31;
    case 2:
      tTempDay += 30;
    case 1:
      tTempDay += 31;
    case 0:
      tTempDay += (!(((1900 + tTemp.tm_year) - 1996) % 4) ? 29 : 28);
  }

  // Jan 1st, 1900 == Monday(1)
  double tDays = tTemp.tm_year;

  tDays         -=   1.0;
  tDays         *= 365.75;
  tTemp.tm_yday  = (tTempDay + (tTemp.tm_mday - 1));
  tDays         += tTemp.tm_yday;
  tTemp.tm_wday  = (((int) tDays - 1) % 7);
  tCurrent       = mktime(&tTemp);

  return (difftime(tCurrent, tLast) > 0.0);
}

void ShowNewNews(TBeing * tBeing)
{
  time_t  tLast = tBeing->player.time->last_logon,
          tTime = time(0);
  struct  stat tData;
  FILE   *tFile;
  char    tString[256];
  int     tMon,
          tDay,
          tYear,
          tCount = 0;
  bool    tPosted = false;
  sstring bufStr;

  // Report for the NEWS file
  if (!stat(File::NEWS, &tData))
    if (tTime - tData.st_mtime <= (3 * SECS_PER_REAL_DAY))
      if ((tFile = fopen(File::NEWS, "r"))) {
        while (!feof(tFile)) {
          fgets(tString, 256, tFile);

          if (sscanf(tString, "%d-%d-%d : ", &tMon, &tDay, &tYear) != 3)
            continue;

          if (!MakeTimeT(tMon, tDay, tYear, tLast))
            break;

          if (!tPosted) {
            tPosted = true;
            tBeing->sendTo("NEWS File Changes:\n\r");
          }

          bufStr = tString;
          tBeing->sendTo(format("%s") % bufStr.toCRLF());

          if (++tCount == 10) {
            tBeing->sendTo("...And there is more, type NEWS to see more.\n\r");
            break;
          }
        }

        fclose(tFile);
      }

  if (tPosted)
    tBeing->sendTo("\n\r");

  tPosted = false;
  tCount  = 0;

  // Report for the NEWS.new file (help nexversion)
  if (!stat("help/nextversion", &tData))
    if (tTime - tData.st_mtime <= (3 * SECS_PER_REAL_DAY))
      if ((tFile = fopen("help/nextversion", "r"))) {
        while (!feof(tFile)) {
          fgets(tString, 256, tFile);

          if (sscanf(tString, "%d-%d-%d : ", &tMon, &tDay, &tYear) != 3)
            continue;

          if (!MakeTimeT(tMon, tDay, tYear, tLast))
            break;

          if (!tPosted) {
            tPosted = true;
            tBeing->sendTo("Future NEWS File Changes:\n\r");
          }

          bufStr = tString;
          tBeing->sendTo(format("%s") % bufStr.toCRLF());

          if (++tCount == 10) {
            tBeing->sendTo("...And there is more, type NEWS to see more.\n\r");
            break;
          }
        }

        fclose(tFile);
      }

  if (tPosted)
    tBeing->sendTo("\n\r");

  tPosted = false;
  tCount  = 0;

  if (tBeing->isImmortal() && !stat(File::WIZNEWS, &tData))
    if (tTime - tData.st_mtime <= (3 * SECS_PER_REAL_DAY))
      if ((tFile = fopen(File::WIZNEWS, "r"))) {
        while (!feof(tFile)) {
          fgets(tString, 256, tFile);

          if (sscanf(tString, "%d-%d-%d : ", &tMon, &tDay, &tYear) != 3)
            continue;

          if (!MakeTimeT(tMon, tDay, tYear, tLast))
            break;

          if (!tPosted) {
            tPosted = true;
            tBeing->sendTo("WIZNEWS File Changes:\n\r");
          }

          bufStr = tString;
          tBeing->sendTo(format("%s") % bufStr.toCRLF());

          if (++tCount == 10) {
            tBeing->sendTo("...And there is more, SEE WIZNEWS to see more.\n\r");
            break;
          }
        }

        fclose(tFile);
      }

  if (tPosted)
    tBeing->sendTo("\n\r");
}

// if descriptor is to be deleted, DELETE_THIS
int Descriptor::nanny(sstring arg)
{
  // character-creation mode
  if (connected >= CON_CREATION_START && connected < CON_CREATION_MAX)
    return creation_nanny(arg);

  sstring buf;
  charFile st;
  TBeing *tmp_ch;
  Descriptor *k, *k2;
  char tmp_name[20];
  int rc;//, which;
  TRoom *rp;
  sstring aw = arg.word(0); // first word of argument, space delimited
  char ac = ((aw.length() >= 1) ? aw[0] : '\0');

  switch (connected) {
    case CON_CONN:
      if (!ac) {
        rp = real_roomp(Room::VOID);
        *rp += *character;
        delete character;
        character = NULL;
        if (account->term == TERM_ANSI) 
          SET_BIT(plr_act, PLR_COLOR);
        
        rc = doAccountMenu("");
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;

        break;
      }
      
      if (_parse_name(aw.c_str(), tmp_name)) {
        writeToQ("Illegal name, please try another.\n\r");
        writeToQ("Name -> ");
        return FALSE;
      }

      if (IS_SET(account->flags, TAccount::EMAIL)) {
        // too bad they can't do this from the menu, but they won't get this
        // far if this was set anyway
        writeToQ("The email account you entered for your account is thought to be bogus.\n\r");
        buf = format("You entered an email address of: %s\n\r") % account->email;
        writeToQ(buf);
        buf = format("If this address is truly valid, please send a mail from it to: %s") % MUDADMIN_EMAIL;
        writeToQ(buf);
        writeToQ("Otherwise, please change your account email address.\n\r");
        rp = real_roomp(Room::VOID);
        *rp += *character;
        delete character;
        character = NULL;
        if (account->term == TERM_ANSI) 
          SET_BIT(plr_act, PLR_COLOR);
        
        rc = doAccountMenu("");
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;

        break;
      }

      if (load_char(tmp_name, &st)) {
        dynamic_cast<TPerson *>(character)->loadFromSt(&st);
      } else {
        writeToQ("No such character, please enter another name.\n\r");
        writeToQ("Name -> ");
        return FALSE;
      }

      if (strcasecmp(account->name.c_str(), st.aname)) {
        writeToQ("No such character, please enter another name.\n\r");
        writeToQ("Name -> ");
        // character existed, but wasn't in my account
        // loadFromSt has initted character (improperly)
        // delete it and recreate it so initialization will be proper

        // deletion at this point is semi-problematic
        // we need to remove desc so the doAccountMenu() in ~TBeing skips
        // but this means character_list assumes presence
        // temporarily shove them into the char_list, and delete will
        // remove them
        character->desc = NULL;
        character->next = character_list;
        character_list = character;

        character->setRoom(Room::NOWHERE);

        delete character;
        character = new TPerson(this);
        return FALSE;
      }


      if(character->hasQuestBit(TOG_PERMA_DEATH_CHAR)){
	character->loadCareerStats();
	if(character->desc->career.deaths){

	  writeToQ("That character is a perma death character and has died.\n\r");
	  writeToQ("Name -> ");
	  
	  // copied from above
	  character->desc = NULL;
	  character->next = character_list;
	  character_list = character;
	  
	  character->setRoom(Room::NOWHERE);
	  
	  delete character;
	  character = new TPerson(this);
	  return FALSE;
	}
      }


      for (k = descriptor_list; k; k = k->next) {
        if ((k->character != character) && k->character) {
          if (k->original) {
            if (k->original->getName() && !strcasecmp(k->original->getName(), character->getName())) {
              writeToQ("That character is already connected.\n\rReconnect? :");
              connected = CON_DISCON;
              return FALSE;
            }
          } else {
            if (k->character->getName() && !strcasecmp(k->character->getName(), character->getName())) {
              writeToQ("That character is already connected.\n\rReconnect? :");
              connected = CON_DISCON;
              return FALSE;
            }
          }
        }
      }
      for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
        if ((!strcasecmp(character->getName(), tmp_ch->getName()) &&
            !tmp_ch->desc && dynamic_cast<TPerson *>(tmp_ch)) ||
            (dynamic_cast<TMonster *>(tmp_ch) && tmp_ch->orig &&
             !strcasecmp(character->getName(),
                      tmp_ch->orig->getName()))) {

          if (character->inRoom() >= 0) {
            // loadFromSt will have inRoom() == last rent
            // roomp not set yet, so just clear this value
            character->setRoom(Room::VOID);
          }
          thing_to_room(character, Room::VOID);
          delete character;
          tmp_ch->desc = this;
          character = tmp_ch;
          tmp_ch->setTimer(0);
          tmp_ch->setInvisLevel(0);

          if (tmp_ch->GetMaxLevel() > MAX_MORT &&
              FORCE_LOW_INVSTE &&
              !tmp_ch->hasWizPower(POWER_VISIBLE)) {
            if (!tmp_ch->isPlayerAction(PLR_STEALTH))
              tmp_ch->addPlayerAction(PLR_STEALTH);

            if (tmp_ch->getInvisLevel() <= MAX_MORT)
              tmp_ch->setInvisLevel(GOD_LEVEL1);
          }

          if (tmp_ch->orig) {
            tmp_ch->desc->original = tmp_ch->orig;
            tmp_ch->orig = 0;
          }
          connected = CON_PLYNG;
          EchoOn();
          // This is a semi-kludge to fix some extra crap we had being sent
          // upon reconnect - Russ 6/15/96
          flush();
          writeToQ("Reconnecting.\n\r");
          tmp_ch->initDescStuff(&st);
          if (tmp_ch->isPlayerAction(PLR_VT100 | PLR_ANSI))
            tmp_ch->doCls(false);

          rc = checkForMultiplay();
	  if(Config::ForceMultiplayCompliance()){
	    if (rc) {
	      // disconnect, but don't cause character to be deleted
	      // do this by disassociating character from descriptor
	      character = NULL;
	      
	      return DELETE_THIS;
	    }
	  }
	  
          if (should_be_logged(character)) {
            objCost cost;

            if (IS_SET(account->flags, TAccount::IMMORTAL)) {
              vlogf(LOG_PIO, format("%s[%s] has reconnected  (account: %s).") % 
	             character->getName() % host % account->name);

	    } else {
              vlogf(LOG_PIO, format("%s[%s] has reconnected  (account: %s).") %  
                     character->getName() % host % account->name);
            }
            character->recepOffer(NULL, &cost);
            dynamic_cast<TPerson *>(character)->saveRent(&cost, FALSE, 1);
          }
          act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
          tmp_ch->loadCareerStats();
	  tmp_ch->loadDrugStats();
	  tmp_ch->loadGuildStats();
	  tmp_ch->loadTitle();
	  tmp_ch->stopsound();
          if (tmp_ch->getHit() < 0) 
            dynamic_cast<TPerson *>(tmp_ch)->autoDeath();
          
          tmp_ch->fixClientPlayerLists(FALSE);

          if (tmp_ch->desc && !tmp_ch->desc->m_bIsClient){
	    Descriptor *d;
	    sstring buf2;

	    if(IS_SET(tmp_ch->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
	      tmp_ch->desc->send_client_prompt(TRUE, 16383);

	    for (d = descriptor_list; d; d = d->next) {
	      if (d->character)
		d->character->fixClientPlayerLists(false);
	    }
          }

          return FALSE;
        }
      }
      if (should_be_logged(character)) {
        if (IS_SET(account->flags, TAccount::IMMORTAL)) {
	  vlogf(LOG_PIO, format("%s[%s] has connected  (account: %s).") %  character->getName() % host % account->name);
        } else {
          vlogf(LOG_PIO, format("%s[%s] has connected  (account: %s).") %  character->getName() % host % account->name);
        }
      }
      
      character->cls();
      sendMotd(character->GetMaxLevel() > MAX_MORT);

      writeToQ("\n\r\n\r*** PRESS RETURN: ");
      connected = CON_RMOTD;
      break;
    case CON_DISCON:

      if (!ac) {
        writeToQ("Please enter 'Y' or 'N'\n\rReconnect? :");
        break;
      }
      switch(ac) {
        case 'Y':
        case 'y':
          for (k = descriptor_list; k; k = k2) {
            k2 = k->next;
            if ((k->character != character) && k->character) {
              if (k->original) {
                if (k->original->getName() && 
                    !strcasecmp(k->original->getName(), character->getName())) {
                  delete k;
                  k = NULL;
                }
              } else {
                if (k->character->getName() && 
                  !strcasecmp(k->character->getName(), character->getName())) {

                  if (k->character) {
                    // disassociate the char from old descriptor before
                    // we delete the old descriptor
                    k->character->desc = NULL;
                    k->character = NULL;
                  }
                  delete k;
                  k = NULL;
                }
              }
            }
          }
          for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
            if ((!strcasecmp(character->getName(), tmp_ch->getName()) &&
                !tmp_ch->desc && dynamic_cast<TPerson *>(tmp_ch)) ||
                (dynamic_cast<TMonster *>(tmp_ch) && tmp_ch->orig &&
                 !strcasecmp(character->getName(),
                          tmp_ch->orig->getName()))) {
  
              if (character->inRoom() >= 0) {
                // loadFromSt will have inRoom() == last rent
                // roomp not set yet, so just clear this value
                character->setRoom(Room::NOWHERE);
              }
              rp = real_roomp(Room::VOID);
              *rp += *character;
              delete character;
              tmp_ch->desc = this;
              character = tmp_ch;
              tmp_ch->setTimer(0);
              tmp_ch->setInvisLevel(0);

              if (tmp_ch->GetMaxLevel() > MAX_MORT &&
                  FORCE_LOW_INVSTE &&
                  !tmp_ch->hasWizPower(POWER_VISIBLE)) {
                if (!tmp_ch->isPlayerAction(PLR_STEALTH))
                  tmp_ch->addPlayerAction(PLR_STEALTH);

                if (tmp_ch->getInvisLevel() <= MAX_MORT)
                  tmp_ch->setInvisLevel(GOD_LEVEL1);
              }

              if (tmp_ch->orig) {
                tmp_ch->desc->original = tmp_ch->orig;
                tmp_ch->orig = 0;
              }
              connected = CON_PLYNG;
              EchoOn();
              flush();
              writeToQ("Reconnecting.\n\r");
              tmp_ch->initDescStuff(&st);
  
              if (tmp_ch->isPlayerAction(PLR_VT100 | PLR_ANSI))
                tmp_ch->doCls(false);
  
              if (should_be_logged(character)) {
                objCost cost;

                if (IS_SET(account->flags, TAccount::IMMORTAL)) 
		  vlogf(LOG_PIO, format("%s[%s] has reconnected  (account: %s).") %  tmp_ch->getName() % host % account->name);
                else 
                  vlogf(LOG_PIO, format("%s[%s] has reconnected  (account: %s).") %  tmp_ch->getName() % host % account->name);
                
                tmp_ch->recepOffer(NULL, &cost);
                dynamic_cast<TPerson *>(tmp_ch)->saveRent(&cost, FALSE, 1);
              }
              act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
              tmp_ch->loadCareerStats();
              tmp_ch->loadDrugStats();
	      tmp_ch->loadGuildStats();
	      tmp_ch->loadTitle();
	      tmp_ch->stopsound();
              if (tmp_ch->getHit() < 0) 
                dynamic_cast<TPerson *>(tmp_ch)->autoDeath();
              
              tmp_ch->fixClientPlayerLists(FALSE);

	      if (tmp_ch->desc && !tmp_ch->desc->m_bIsClient) {
		Descriptor *d;
		sstring buf2;  

		if(IS_SET(tmp_ch->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
		  tmp_ch->desc->send_client_prompt(TRUE, 16383);

		for (d = descriptor_list; d; d = d->next) {
		  if (d->character)
		    d->character->fixClientPlayerLists(false);
		}
	      }

              return FALSE;
            }
          }
          break;
        case 'N':
        case 'n':
          return DELETE_THIS;
        default:
          writeToQ("Please enter 'y' or 'n'\n\rReconnect? :");
          break;
      }
      break;
    case CON_RMOTD:        
      character->doCls(false);
      if (!character->GetMaxLevel())
        dynamic_cast<TPerson *>(character)->doStart();

      rc = dynamic_cast<TPerson *>(character)->genericLoadPC();
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        // we will wind up deleting desc, and in turn desc->character
        // since something bad happened in the load above, char may
        // have eq, which would cause problems, so...
        dynamic_cast<TPerson *>(character)->dropItemsToRoom(SAFE_YES, NUKE_ITEMS);
        return DELETE_THIS;
      }


      if (character->isPlayerAction(PLR_VT100 | PLR_ANSI))
        character->doCls(false);

      character->fixClientPlayerLists(FALSE);

      ShowNewNews(character);

      character->doLook("", CMD_LOOK);

      prompt_mode = 1;
      dynamic_cast<TPerson *>(character)->fixPracs();
      character->doSave(SILENT_YES);
      character->desc->saveAccount();

      if (character->desc && !character->desc->m_bIsClient) {
	Descriptor *d;
	sstring buf2;  

	if(IS_SET(character->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
	  character->desc->send_client_prompt(TRUE, 16383);

	for (d = descriptor_list; d; d = d->next) {
	  if (d->character)
	    d->character->fixClientPlayerLists(false);
	}
      }

      // this has to be set AFTER skill assignment, which happens somewhere
      // between genericLoadPC and here
      if(character->hasQuestBit(TOG_IS_COWARD)){
        character->wimpy=character->maxWimpy();
      } else if (character->hasQuestBit(TOG_IS_VICIOUS)){
        character->wimpy=0;
      } else if (character->hasQuestBit(TOG_IS_CRAVEN)){
        character->wimpy=character->maxWimpy()/2 + character->maxWimpy()%2;
      }

      // we set hitpoints last, since we need to load known skills first (else our maxHp is low)
      character->setHit(character->hitLimit());
      wearSlotT iw;
      for (iw = MIN_WEAR; iw < MAX_WEAR; iw++) {
        character->setLimbFlags(iw, 0);
        character->setStuckIn(iw, NULL);
        character->setCurLimbHealth(iw, character->getMaxLimbHealth(iw));
      }

      // called to trigger responses etc
      rc = character->genericMovedIntoRoom(character->roomp, -1);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        vlogf(LOG_BUG, format("Response trigger in %s caused %s to be deleted and drop eq.") % character->roomp->getName() % character->getName());
        dynamic_cast<TPerson *>(character)->dropItemsToRoom(SAFE_YES, NUKE_ITEMS);
        return DELETE_THIS;
      }

      break;
    default:
      vlogf(LOG_BUG, "Nanny: illegal state of con'ness");
      abort();
      break;
  }
  return FALSE;
}

int TPerson::genericLoadPC()
{
  TRoom *rp;
  int rc;

  rc = desc->checkForMultiplay();
  if(Config::ForceMultiplayCompliance()){
    if (rc)
      return DELETE_THIS;
  }

  if (should_be_logged(this)) {
    vlogf(LOG_PIO, format("Loading %s's equipment") %  name);
  }
  resetChar();
  loadRent();
  if (player.time->last_logon && (player.time->last_logon < Uptime)) {
    if ((time(0) - player.time->last_logon) > 36 * SECS_PER_REAL_HOUR)
      wipeCorpseFile(sstring(name).lower().c_str());
    else
      assignCorpsesToRooms();
  }
  saveChar(Room::AUTO_RENT);
  sendTo(WELC_MESSG);
  next = character_list;
  character_list = this;

  if(Config::SpeefMakeBody()){
    vlogf(LOG_MISC, format("Loading a body for %s\n\r") %  name);
    body = new Body(race->getBodyType(), points.maxHit);
  }

  if (in_room == Room::NOWHERE || in_room == Room::AUTO_RENT) {
    if (banished()) {
      rp = real_roomp(Room::HELL);
      *rp += *this;
      player.hometown = Room::HELL;
    } else if (GetMaxLevel() <= MAX_MORT) {
      if (player.hometown != 0xFFFF) {
        rp = real_roomp(player.hometown);
        if (!rp) {
          vlogf(LOG_LOW, format("Player (%s) had non-existant hometown (%d)") %  getName() % player.hometown);
          rp = real_roomp(Room::NEWBIE);
        }

        if (!rp) {
          vlogf(LOG_LOW, format("Was unable to read center square!  Player being disconnected!  (%s)") % getName());
          return DELETE_THIS;
        }

        *rp += *this;
      } else {
        rp = real_roomp(Room::NEWBIE);

        if (!rp) {
          vlogf(LOG_LOW, format("Was unable to read center square!  Player being disconnected!  (%s) [2]") % getName());
          return DELETE_THIS;
        }

        *rp += *this;
        player.hometown = Room::NEWBIE;
      }
    } else {
      wizFileRead(); // Needed for office

      rp = real_roomp((desc ? desc->office : Room::IMPERIA));

      if (!IS_SET(desc->account->flags, TAccount::IMMORTAL)) {
        vlogf(LOG_BUG, format("%s is immortal but account isn't set immortal.  Setting now.") % 
              getName());
        SET_BIT(desc->account->flags, TAccount::IMMORTAL);
      }

      if (!rp) {
        vlogf(LOG_BUG, format("Attempting to place %s in room that does not exist.\n\r") %  name);
        rp = real_roomp(Room::VOID);
      }

      if (!rp) {
	vlogf(LOG_LOW, format("Was unable to read VOID!  Immortal being disconnected!  (%s) [3]") % getName());
	return DELETE_THIS;
      }

      in_room = Room::NOWHERE;  // change it so it doesn't error in +=
      *rp += *this;
      player.hometown = Room::IMPERIA;
      if (!isImmortal())   // they turned it off
	doToggle("immortal");

      if (FORCE_LOW_INVSTE && !hasWizPower(POWER_VISIBLE)) {
        if (!isPlayerAction(PLR_STEALTH))
          addPlayerAction(PLR_STEALTH);

        if (getInvisLevel() <= MAX_MORT)
          setInvisLevel(GOD_LEVEL1);
      }
    }
  } else {
    if (!banished()) {
      if (in_room >= 0) {
        rp = real_roomp(in_room);

        if (!rp) {
          vlogf(LOG_LOW, format("Was unable to read room %d!  Player being disconnected!  (%s) [3]") % in_room % getName());
          return DELETE_THIS;
        }

        in_room = Room::NOWHERE;  // change it so it doesn't error in +=
        *rp += *this;
        player.hometown = in_room;
      } else {
        rp = real_roomp(Room::NEWBIE);

        if (!rp) {
          vlogf(LOG_LOW, format("Was unable to read center square!  Player being disconnected!  (%s) [4]") % getName());
          return DELETE_THIS;
        }

        in_room = Room::NOWHERE;  // change it so it doesn't error in +=
        *rp += *this;
        player.hometown = Room::NEWBIE;
      }
    } else {
      rp = real_roomp(Room::HELL);

      if (!rp) {
        vlogf(LOG_LOW, format("Was unable to read HELL!  Player being disconnected!  (%s) [5]") % getName());
        return DELETE_THIS;
      }

      in_room = Room::NOWHERE;  // change it so it doesn't error in +=
      *rp += *this;
      player.hometown = Room::HELL;
    }
  }

  act("$n steps in from another world.", TRUE, this, 0, 0, TO_ROOM);
  desc->connected = CON_PLYNG;
  desc->playerID=0;

  // must be after char is placed in valid room
  loadFollowers();
  loadCareerStats();
  loadDrugStats();
  loadGuildStats();
  loadTitle();

  stopsound();

  stats.logins++;
  save_game_stats();
 
  return FALSE;
}


void Descriptor::EchoOn()
{
  if (m_bIsClient)
    return;

  char echo_on[6] = {IAC, WONT, TELOPT_ECHO, '\n', '\r', '\0'};

  write(socket->m_sock, echo_on, 6);
}

void Descriptor::EchoOff()
{
  if (m_bIsClient)
    return;

  char echo_off[4] = {IAC, WILL, TELOPT_ECHO, '\0'};

  write(socket->m_sock, echo_off, 4);
}

bool Descriptor::start_page_file(const char *fpath, const char *errormsg)
{
  if (pagedfile) {
    delete [] pagedfile;
    pagedfile = NULL;
    cur_page = tot_pages = 0;
  }

  pagedfile = new char[strlen(fpath) + 1];
  strcpy(pagedfile, fpath);
  cur_page = 0;
  tot_pages = 0;

  if (!page_file("")) {      // couldn't open file, etc. 
    delete [] pagedfile;
    pagedfile = NULL;
    cur_page = 0;
    tot_pages = 0;
    writeToQ(errormsg);
    return FALSE;
  }
  return TRUE;
}

// page_file returns TRUE if something was paged, FALSE if nothing got sent 
// if (position) comes back < 0 then EOF was hit when outputing file.    
bool Descriptor::page_file(const char *the_input)
{
  FILE *fp;
  char buffer[256];
  int lines;
  int sent_something = FALSE;
  int lines_per_page;

  if (character)
    lines_per_page = ((character->ansi() || character->vt100()) ? (screen_size - 6) : (screen_size - 2));
  else
    lines_per_page = ((account->term == TERM_ANSI || account->term == TERM_VT100) ? (screen_size - 6) : (screen_size - 2));

  if (!pagedfile)
    return FALSE;

  if (!tot_pages) {
    lines = 0;
    if ((fp = fopen(pagedfile, "r"))) {
      while (fgets(buffer, 255, fp)) {
        lines++;
        continue;
      }
      if (lines_per_page <= 0) {
        writeToQ("Bad screensize set.\n\r");
        delete [] pagedfile;
        pagedfile = NULL;
        tot_pages = cur_page = 0;
        fclose(fp);
        return TRUE;
      }
      tot_pages = lines / lines_per_page;
      tot_pages += 1;
      cur_page = 0;
      fclose(fp);
    } else
      return FALSE;
  }
  one_argument(the_input, buffer, cElements(buffer));

  if (*buffer) {
    if (*buffer == 'r' || *buffer == 'R') {
      cur_page -= 1;
      cur_page = max(0, cur_page);
    } else if (*buffer == 'b' || *buffer == 'B') {
      cur_page -= 2;
      cur_page = max(0, cur_page);
    } else if (isdigit(*buffer)) {
      cur_page = max(min((int) tot_pages, convertTo<int>(buffer)), 1) - 1;
    } else {
      delete [] pagedfile;
      pagedfile = NULL;
      tot_pages = cur_page = 0;
      writeToQ("*** INTERRUPTED ***\n\r");
      return TRUE;
    }
  }

  cur_page += 1;
  if (!(fp = fopen(pagedfile, "r")))
    return FALSE;

  for (lines = 0; lines < (cur_page - 1) * lines_per_page; lines++) {
    if (!fgets(buffer, 255, fp)) {
      vlogf(LOG_FILE, format("Error paging file: %s, %d") %  pagedfile % cur_page);
      delete [] pagedfile;
      pagedfile = NULL;
      cur_page = tot_pages = 0;
      fclose(fp);
      return FALSE;
    }
  }
  for (lines = 0; lines < lines_per_page; lines++) {
    if (fgets(buffer, 255, fp)) {
      writeToQ(buffer);
      writeToQ("\r");
      sent_something = TRUE;
    }
    if (feof(fp)) {
      fclose(fp);
      delete [] pagedfile;
      pagedfile = NULL;
      tot_pages = cur_page = 0;
      return sent_something;
    }
  }
  fclose(fp);
  return sent_something;
}

// show should be false, except in nanny() loop
// the prompt handler normmaly will show the "press return" crap
// allow will permit the string to parse for %n in the colorstring
// the chief problem with this is you can make a board message look like the
// reader's name is in it.  (it is default TRUE)
void Descriptor::page_string(const sstring &strs, showNowT shownow, allowReplaceT allowRep)
{
  delete [] showstr_head;
  showstr_head = mud_str_dup(strs);
  mud_assert(showstr_head != NULL, "Bad alloc in page_string");

  cur_page = 0;
  tot_pages = 0;

  show_string("", shownow, allowRep);
}

void Descriptor::show_string(const char *the_input, showNowT showNow, allowReplaceT allowRep)
{
  // this will hold the text of the single page that we are on
  // theortically, it is no more than screen_length * 80, but color
  // codes extend it somewhat
  char buffer[MAX_STRING_LENGTH];

  char buf[MAX_INPUT_LENGTH];
  char *chk;
  int lines = 0, toggle = 1;
  int lines_per_page;
  int i;

  if (account)
    lines_per_page = ((account->term == TERM_ANSI || account->term == TERM_VT100) ? (screen_size - 6) : (screen_size - 2));
  else
    lines_per_page = 20;

  // runs only once to initialize values
  if (!tot_pages) {
    toggle = 1;
    lines = 0;
    for (chk = showstr_head; *chk; chk++) {
      if ((*chk == '\n' || *chk == '\r') && ((toggle = -toggle) < 0)) {
        lines++;
        toggle = 0;
        continue;
      }
      if (!toggle) 
        toggle = 1;
    }
    if (lines_per_page <= 0) {
      writeToQ("Bad screensize set.\n\r");
      delete [] showstr_head;
      showstr_head = NULL;
      tot_pages = cur_page = 0;
      return;
    }
    tot_pages = lines / lines_per_page;
    tot_pages += 1;
    cur_page = 0;
  }
  one_argument(the_input, buf, cElements(buf));

  if (*buf) {
    if (*buf == 'r' || *buf == 'R') {
      cur_page -= 1;
      cur_page = max(0, cur_page);
    } else if (*buf == 'b' || *buf == 'B') {
      cur_page -= 2;
      cur_page = max(0, cur_page);
    } else if (isdigit(*buf)) 
      cur_page = max(min((int) tot_pages, convertTo<int>(buf)), 1) - 1;
    else {
      if (showstr_head) {
        delete [] showstr_head;
        showstr_head = NULL;
        tot_pages = cur_page = 0;
      }
      return;
    } 
  }
  toggle = 1;
  lines = 0;
  cur_page++;
  i = 0;
  *buffer = '\0';
  for (chk = showstr_head; *chk; chk++) {
    if ((*chk == '\n' || *chk == '\r') && ((toggle = -toggle) < 0)) {
      lines++;
      if ((lines/lines_per_page + 1) == cur_page)
        buffer[i++] = *chk;
      continue;
    }
    if ((lines/lines_per_page + 1) > cur_page)
      break;
    if ((lines/lines_per_page + 1) < cur_page)
      continue;
//    if (!i && isspace(*chk))
//      continue;
      
    // some redundant code from colorString()
    if ((*chk == '<') && (*(chk +2) == '>')) {
      if (chk != showstr_head && *(chk-1) == '<') {
        // encountered double <<, so skip 1 of them, 
        // we've already written the first
        continue;
      }
      switch (*(chk + 1)) {
        case 'n':
        case 'N':
          if (allowRep) {
            strcpy(buffer + i, character->getName());
            i += strlen(character->getName());
            chk += 2;
          } else {
            buffer[i++] = *chk;
          }
          break;
        case 'h':
          strcpy(buffer + i, MUD_NAME);
          i+= strlen(MUD_NAME);
          chk += 2;
          break;
        case 'H':
          strcpy(buffer + i, MUD_NAME_VERS);
          i+= strlen(MUD_NAME);
          chk += 2;
          break;
        case 'R':
          strcpy(buffer + i, redBold());
          i += strlen(redBold());
          chk += 2;
          break;
        case 'r':
          strcpy(buffer + i, red());
          i += strlen(red());
          chk += 2;
          break;
        case 'G':
          strcpy(buffer + i, greenBold());
          i += strlen(greenBold());
          chk += 2;
          break;
        case 'g':
          strcpy(buffer + i, green());
          i += strlen(green());
          chk += 2;
          break;
        case 'Y':
        case 'y':
          strcpy(buffer + i, orangeBold());
          i += strlen(orangeBold());
          chk += 2;
          break;
        case 'O':
        case 'o':
          strcpy(buffer + i, orange());
          i += strlen(orange());
          chk += 2;
          break;
        case 'B':
          strcpy(buffer + i, blueBold());
          i += strlen(blueBold());
          chk += 2;
          break;
        case 'b':
          strcpy(buffer + i, blue());
          i += strlen(blue());
          chk += 2;
          break;
        case 'P':
          strcpy(buffer + i, purpleBold());
          i += strlen(purpleBold());
          chk += 2;
          break;
        case 'p':
          strcpy(buffer + i, purple());
          i += strlen(purple());
          chk += 2;
          break;
        case 'C':
          strcpy(buffer + i, cyanBold());
          i += strlen(cyanBold());
          chk += 2;
          break;
        case 'c':
          strcpy(buffer + i, cyan());
          i += strlen(cyan());
          chk += 2;
          break;
        case 'W':
          strcpy(buffer + i, whiteBold());
          i += strlen(whiteBold());
          chk += 2;
          break;
        case 'w':
          strcpy(buffer + i, white());
          i += strlen(white());
          chk += 2;
          break;
        case 'K':
          strcpy(buffer + i, black());
          i += strlen(black());
          chk += 2;
          break;
        case 'k':
          strcpy(buffer + i, blackBold());
          i += strlen(blackBold());
          chk += 2;
          break;
        case 'A':
          strcpy(buffer + i, underBold());
          i += strlen(underBold());
          chk += 2;
          break;
        case 'a':
          strcpy(buffer + i, under());
          i += strlen(under());
          chk += 2;
          break;
        case 'D':
        case 'd':
          strcpy(buffer + i, bold());
          i += strlen(bold());
          chk += 2;
          break;
        case 'F':
        case 'f':
          strcpy(buffer + i, flash());
          i += strlen(flash());
          chk += 2;
          break;
        case 'I':
        case 'i':
          strcpy(buffer + i, invert());
          i += strlen(invert());
          chk += 2;
          break;
        case 'E':
        case 'e':
          strcpy(buffer + i, BlackOnWhite());
          i += strlen(BlackOnWhite());
          chk += 2;
          break;
        case 'j':
        case 'J':
          strcpy(buffer + i, BlackOnBlack());
          i += strlen(BlackOnBlack());
          chk += 2;
          break;
        case 'L':
        case 'l':
          strcpy(buffer + i, WhiteOnRed());
          i += strlen(WhiteOnRed());
          chk += 2;
          break;
        case 'Q':
        case 'q':
          strcpy(buffer + i, WhiteOnGreen());
          i += strlen(WhiteOnGreen());
          chk += 2;
          break;
        case 'T':
        case 't':
          strcpy(buffer + i, WhiteOnOrange());
          i += strlen(WhiteOnOrange());
          chk += 2;
          break;
        case 'U':
        case 'u':
          strcpy(buffer + i, WhiteOnBlue());
          i += strlen(WhiteOnBlue());
          chk += 2;
          break;
        case 'V':
        case 'v':
          strcpy(buffer + i, WhiteOnPurple());
          i += strlen(WhiteOnPurple());
          chk += 2;
          break;
        case 'X':
        case 'x':
          strcpy(buffer + i, WhiteOnCyan());
          i += strlen(WhiteOnCyan());
          chk += 2;
          break;
        case 'Z':
        case 'z':
        case '1':
          strcpy(buffer + i, norm());
          i += strlen(norm());
          chk += 2;
          break;
        default:
          buffer[i++] = *chk;
          break;
      }
    } else
      buffer[i++] = *chk;
  }
  if (!i || cur_page == tot_pages) {
    delete [] showstr_head;
    showstr_head = NULL;
    cur_page = tot_pages = 0;
  }
  buffer[i] = '\0';
  strcat(buffer, norm());

  if (showNow) {
    if (tot_pages) {
      sprintf(buffer + strlen(buffer),
         "\n\r[ %sReturn%s to continue, %s(r)%sefresh, %s(b)%sack, page %s(%d/%d)%s, or %sany other key%s to quit ]\n\r", 
            green(),  norm(),
            green(),  norm(),
            green(),  norm(),
            green(),  
            cur_page, tot_pages, norm(),
            green(),  norm());
    } else {
      sprintf(buffer + strlen(buffer),
          "\n\r[ %sReturn%s to continue ]\n\r",
            green(), norm());
    }
  }
  writeToQ(buffer);
}

void Descriptor::writeToQ(const sstring &arg)
{
  output.putInQ(new UncategorizedComm(arg));
}


void Descriptor::sstring_add(char *s)
{
  char *scan;
  int terminator = 0, t2 = 0;
  char buf[MAX_STRING_LENGTH];

  for (scan = s; *scan; scan++) {
    if ((terminator = (*scan == '~')) || (t2 = (*scan == '`'))) {
      *scan = '\0';
      break;
    }
  }
  if (!*str) {
    if (strlen(s) > (unsigned) max_str) {
      writeToQ("Message too long - Truncated.\n\r");
      *(s + max_str) = '\0';
      terminator = 1;
    }
    unsigned int loop = 0;
    while (strlen(s) > 80) {
      int iter = -1;
      loop++;

      for (iter = 80; iter >= 0 && s[iter] != ' '; iter--);
      if (iter < 0)
        break;
      if (!*str) {
	strncpy(buf, s, iter);
	buf[iter]='\0';
	strcat(buf, "\n\r");
	*str = mud_str_dup(buf);
      } else {
        const char *t = *str;
	strcpy(buf, t);
	strncat(buf, s, iter);
	buf[strlen(t)+iter]='\0';
	strcat(buf, "\n\r");
	*str=mud_str_dup(buf);
        delete [] t;
      }

      // fix s
      char *t = s;
      for (;iter < (int) strlen(s) && isspace(s[iter]); iter++);
      s = mud_str_dup(&s[iter]);
      mud_assert(s != NULL, "sstring_add(): Bad sstring memory");
      if (loop > 1)
        delete [] t;
    }

    if (!*str) {
      *str = mud_str_dup(s);
    } else {
      const char *t = *str;
      strcpy(buf, t);
      strcat(buf, s);
      *str=mud_str_dup(buf);
      delete [] t;
      if (loop >= 1)
        delete [] s;
    }
  } else {
    // preexisting *str
    if (strlen(s) + strlen(*str) > (unsigned) max_str) {
      writeToQ("Message too long. Last line skipped.\n\r");
      terminator = 1;
    } else {
      if (character->isPlayerAction(PLR_BUGGING)) {
        if (!**str) {
          // we are on the first line
          const char *t = s;
          for (;*t && isspace(*t); t++);
    
          if (!*t) {
            writeToQ("Blank lines entered.  Ignoring!\n\r");
            *(name) = '\0';
    
            delete [] *str;
            *str = NULL;
  
            delete str;
            str = NULL;
    
            character->remPlayerAction(PLR_BUGGING);
  
            if (connected == CON_WRITING)
              connected = CON_PLYNG;

            if (m_bIsClient)
              clientf(format("%d|%d") % CLIENT_ENABLEWINDOW % FALSE);

            return;
          }
          sprintf(buf, "Write your %s, use ~ when done, or ` to cancel.\n\r", sstring(name).uncap().c_str());
          writeToQ(buf);
          t = *str;
          strncpy(buf, t, cElements(buf));
          strncat(buf, s, cElements(buf));
          *str=mud_str_dup(buf);
          delete [] t;
        } else {
          // body of idea
          const char *t = *str;
	        strncpy(buf, t, cElements(buf));
	        strncat(buf, s, cElements(buf));
	        *str=mud_str_dup(buf);
          if (!m_bIsClient)
            delete [] t;
        }
      } else {
        // not a bug/idea
#if 0
        char *t = *str;
        *str = new char[strlen(t) + strlen(s) + 3];
        mud_assert(*str != NULL, "sstring_add(): Bad sstring memory");
        strcpy(*str, t);
        strcat(*str, s);
        delete [] t;
#else
        // fix it up so that it all fits on one line
        unsigned int loop = 0;
        while (strlen(s) > 80) {
          int iter = -1;

          for (iter = 80; iter >= 0 && s[iter] != ' '; iter--);
          if (iter < 0)
            break;

          loop++;
          const char *t = *str;
	  strcpy(buf, t);
	  strncat(buf, s, iter);
	  buf[strlen(t) + iter] = '\0';
	  strcat(buf, "\n\r");
	  *str=mud_str_dup(buf);
          delete [] t;

          // fix s
          t = s;
          for (;iter < (int) strlen(s) && isspace(s[iter]); iter++);
          if (iter < (int) strlen(s)) {
            s = mud_str_dup(&s[iter]);
            mud_assert(s != NULL, "sstring_add(): Bad sstring memory");
            if (loop > 1)
              delete [] t;
          } else {
            // don't bother doing anything, whitespace all the way to EOL
            if (loop > 1)
              delete [] t;
            break;
          }
        }

        const char *t = *str;
	strcpy(buf, t);
	strcat(buf, s);
	*str = mud_str_dup(buf);
        delete [] t;
        if (loop > 1)
          delete [] s;
#endif
      }
    }
  }
  if (terminator || t2) {
    if (character->isPlayerAction(PLR_MAILING)) {
      if (ignored.isMailIgnored(this, name))
      {
        vlogf(LOG_OBJ, format("Mail: mail sent by %s was ignored by %s.") % character->getName() % name);
      }
      else if (terminator)
      {
        int rent_id = 0;
        if (obj && obj->canBeMailed(name))
        {
          ItemSaveDB is("mail", GH_MAIL_SHOP);
          rent_id = is.raw_write_item(obj, -1 , 0);
          vlogf(LOG_OBJ, format("Mail: %s mailing %s (vnum:%i) to %s rented as rent_id:%i") %
            character->getName() % obj->getName() % obj->objVnum() % name % rent_id);
          delete obj;
        }
        if (amount > 0)
        {
          vlogf(LOG_OBJ, format("Mail: %s mailing %i talens to %s") %
            character->getName() % amount % name);
          character->addToMoney(min(0, -amount), GOLD_XFER);
        }
        store_mail(name, character->getName(), *str, amount, rent_id);
      }

      // delete mail string
      delete [] *str;
      *str = NULL;
      delete str;
      str = NULL;

      // clear amount, object, name
      obj = NULL;
      *(name) = '\0';
      amount = 0;

      writeToQ(terminator ? "Message sent!\n\r" : "Message deleted!\n\r");
      character->remPlayerAction(PLR_MAILING);
    } else if (character->isPlayerAction(PLR_BUGGING)) {
      if (terminator) {
        const char *t = *str;

        for (;*t && isspace(*t); t++);

        if (!*t) 
          writeToQ("Blank message entered.  Ignoring!\n\r");
        else {
          if (!strcmp(name, "Comment"))
            add_comment(delname, t);
          else
            send_feedback(name, t);
          writeToQ(name);
          writeToQ(" sent!\n\r");
        }
      } else {
        writeToQ(name);
        writeToQ(" deleted!\n\r");
      }
      *(name) = '\0';

      delete [] *str;
      *str = NULL;

      delete str;
      str = NULL;

      character->remPlayerAction(PLR_BUGGING);
    } else {
      if (t2) {
        // Cancalation capability added by Russ 020997
        delete [] *str;
        *str = NULL;
      }
      str = NULL;

    }
    if (connected == CON_WRITING) {
      connected = CON_PLYNG;
      // do not delete the sstring, it has been aplied to the mob/obj/room
    }
    // set the sstring to NULL to insure we don't fall into sstring_add again
    str = NULL;

    if (m_bIsClient)
      clientf(format("%d|%d") % CLIENT_ENABLEWINDOW % FALSE);
  } else {
    const char *t=*str;
    strcpy(buf, t);
    strcat(buf, "\n\r");
    *str=mud_str_dup(buf);
    delete [] t;

    //if (m_bIsClient)
      //prompt_mode = -1;
  }
}

void Descriptor::fdSocketClose(int desc)
{
  Descriptor *d, *d2;

  for (d = this; d; d = d2) {
    d2 = d->next;
    if (d->socket->m_sock == desc) {
      delete d;
      d = NULL;
    }
  }
  return;
}

const char *StPrompts[] =
  {
    "[Z:%d Pr:%s L:%d H:%d C:%d S:%s]\n\r", // Builder Assistant
    "%sH:%d%s ",           // Hit Points
    "%sP:%.1f%s ",         // Piety
    "%sM:%d%s ",           // Mana
    "%sV:%d%s ",           // Moves
    "%sT:%d%s ",           // Talens
    "%sR:%d%s ",           // Room
    "%sE:%s%s ",           // Exp
    "%sN:%s%s ",           // Exp Tnl
    "%s%s<%s%s=%s>%s ",    // Opponent
    "%s%s<%s/tank=%s>%s ", // Tank / Tank-Other
    "%s<%.1f%s> ",         // Lockout
    "%sLF:%d%s ",          // Lifeforce
    "%s%02i:%02i:%02i%s "            // time
  };


sstring getPietyPrompt(TBeing *ch, Descriptor *d, float piety){
  sstring promptbuf="";

  if (IS_SET(d->prompt_d.type, PROMPT_PIETY)) {	    
    promptbuf=format(StPrompts[2]) %
      (IS_SET(d->prompt_d.type, PROMPT_COLOR) ? d->prompt_d.pietyColor : "") %
      piety %
      ch->norm();
  }

  return promptbuf;
}
sstring getLFPrompt(TBeing *ch, Descriptor *d, int lf){
  sstring promptbuf="";
  
  if (IS_SET(d->prompt_d.type, PROMPT_LIFEFORCE)) {	    
    promptbuf=format(StPrompts[12]) %
      (IS_SET(d->prompt_d.type, PROMPT_COLOR) ? d->prompt_d.lifeforceColor : "") %
      lf %
      ch->norm();
  }

  return promptbuf;
}
sstring getMovesPrompt(TBeing *ch, Descriptor *d, int moves){
  sstring promptbuf="";
  
  if (IS_SET(d->prompt_d.type, PROMPT_MOVE))
    promptbuf=format(StPrompts[4]) %
      (IS_SET(d->prompt_d.type, PROMPT_COLOR) ? d->prompt_d.moveColor : "") %
      moves %
      ch->norm();

  return promptbuf;
}
sstring getMoneyPrompt(TBeing *ch, Descriptor *d, int money){
  sstring promptbuf="";
  
  if (IS_SET(d->prompt_d.type, PROMPT_GOLD))
    promptbuf=format(StPrompts[5]) %
      (IS_SET(d->prompt_d.type, PROMPT_COLOR) ? d->prompt_d.moneyColor : "") %
      money %
      ch->norm();

  return promptbuf;
}
sstring getRoomPrompt(TBeing *ch, Descriptor *d, int room){
  sstring promptbuf="";

  if (IS_SET(d->prompt_d.type, PROMPT_ROOM))
    promptbuf=format(StPrompts[6]) %
      (IS_SET(d->prompt_d.type, PROMPT_COLOR) ? d->prompt_d.roomColor : "") %
      room %
      ch->norm();

  return promptbuf;
}

sstring getManaPrompt(TBeing *ch, Descriptor *d, int mana){
  sstring promptbuf="";

  if (IS_SET(d->prompt_d.type, PROMPT_MANA)) {
    promptbuf=format(StPrompts[3]) %
      (IS_SET(d->prompt_d.type, PROMPT_COLOR) ? d->prompt_d.manaColor : "") %
      mana %
      ch->norm();
  }

  return promptbuf;
}

sstring getHitPointsPrompt(TBeing *ch, Descriptor *d, int hp){
  sstring promptbuf="";

  if (IS_SET(d->prompt_d.type, PROMPT_HIT))
    promptbuf=format(StPrompts[1]) % 
      (IS_SET(d->prompt_d.type, PROMPT_COLOR) ? d->prompt_d.hpColor : "") %
      hp %
      ch->norm();

  return promptbuf;
}


time_t getTimePromptData(TBeing *ch){
  time_t ct;
  if (ch->desc->account)
    ct = time(0) + 3600 * ch->desc->account->time_adjust;
  else
    ct = time(0);

  return ct;
}

sstring getTimePrompt(TBeing *ch, Descriptor *d, time_t ct){
  sstring promptbuf="";
  struct tm *tm=localtime(&ct);
  
  if (IS_SET(d->prompt_d.type, PROMPT_TIME)){
    promptbuf=format(StPrompts[13]) % 
      (IS_SET(d->prompt_d.type, PROMPT_COLOR) ? d->prompt_d.timeColor : "") %
      tm->tm_hour % tm->tm_min % tm->tm_sec %
      ch->norm();
  }
  return promptbuf;
}

sstring PromptComm::getText(){
  return text;
}

sstring PromptComm::getClientText(){
  return text;
}

sstring PromptComm::getXML(){
  return format("<prompt time=\"%i\" hp=\"%i\" mana=\"%i\" piety=\"%f\" lifeforce=\"%i\" moves=\"%i\" money=\"%i\" room=\"%i\">%s</prompt>") %
    time % hp % mana % piety % lifeforce % moves % money % room % 
    text.escape(sstring::XML);
}


sstring RoomExitComm::getText(){
  return "";
}

sstring RoomExitComm::getClientText(){
  return "";
}

sstring RoomExitComm::getXML(){
  sstring buf="";

  buf+=format("<roomexits>\n");

  for(dirTypeT dir=MIN_DIR;dir<MAX_DIR;dir++){
    if(exits[dir].exit){
      buf+=format("  <exit>\n");
      buf+=format("    <direction>%s</direction>\n") % dirs[dir];
      if(exits[dir].door){
	buf+=format("    <door>\n");
	buf+=format("      <open>%s</open>\n") % (exits[dir].open?"true":"false");
	buf+=format("    </door>\n");
      }
      buf+=format("  </exit>\n");
    }
  }

  buf+=format("</roomexits>\n");

  return buf;
}


void setPrompts(fd_set out)
{
  Descriptor *d, *nextd;
  TBeing *tank = NULL;
  TBeing *ch;
  TThing *obj;
  char promptbuf[256] = "\0\0\0",
       tString[256];
  unsigned int update;

  for (d = descriptor_list; d; d = nextd) {
    nextd = d->next;
    if ((FD_ISSET(d->socket->m_sock, &out) && d->output.getBegin()) || d->prompt_mode) {
      update = 0;
      ch = d->character;

      if (!d->connected && ch && ch->isPc() &&
          !(ch->isPlayerAction(PLR_COMPACT)) && d->prompt_mode != DONT_SEND)
        d->output.putInQ(new UncategorizedComm("\n\r"));


      if (ch && ch->task) {
        if (ch->task->task == TASK_PENANCE) {
          sprintf(promptbuf, "\n\rPIETY : %5.2f > ", ch->getPiety());
          d->output.putInQ(new UncategorizedComm(sstring(promptbuf).cap()));
        } else

        if (ch->task->task == TASK_MEDITATE) {
          sprintf(promptbuf, "\n\rMANA : %d > ", ch->getMana());
          d->output.putInQ(new UncategorizedComm(sstring(promptbuf).cap()));
        } else

        if (ch->task->task == TASK_SACRIFICE) {
          sprintf(promptbuf, "\n\rLIFEFORCE : %d > ", ch->getLifeforce());
          d->output.putInQ(new UncategorizedComm(sstring(promptbuf).cap()));
        } else

        if (((ch->task->task == TASK_SHARPEN) || (ch->task->task == TASK_DULL)) && (obj = ch->heldInPrimHand())) {
          sprintf(promptbuf, "\n\r%s > ", ch->describeSharpness(obj).c_str());
          d->output.putInQ(new UncategorizedComm(promptbuf));
        } else

        if ((ch->task->task == TASK_BLACKSMITHING)            || (ch->task->task == TASK_REPAIR_DEAD)     ||
            (ch->task->task == TASK_REPAIR_ORGANIC)    || (ch->task->task == TASK_REPAIR_MAGICAL)  ||
            (ch->task->task == TASK_REPAIR_ROCK)       || (ch->task->task == TASK_BLACKSMITHING_ADVANCED) ||
            (ch->task->task == TASK_MEND_HIDE)         || (ch->task->task == TASK_MEND)            ||
            (ch->task->task == TASK_REPAIR_SPIRITUAL)) {
          TThing * tThing = NULL;
          TObj   * tObj   = NULL;

          for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end() && (tThing=*it);++it) {
            if ((tObj = dynamic_cast<TObj *>(tThing)) && isname(ch->task->orig_arg, tThing->name))
              break;

            tObj = NULL;
	  }

          if (tObj) {
            sprintf(promptbuf, "\n\r%s (%s) > ", sstring(tObj->getName()).cap().c_str(), tObj->equip_condition(-1).c_str());
            d->output.putInQ(new UncategorizedComm(colorString(ch, d, promptbuf, NULL, COLOR_BASIC, FALSE)));
          } else {
            strcat(promptbuf, "\n\rERROR!  Unable to find repair target! > ");
            d->output.putInQ(new UncategorizedComm(promptbuf));
            vlogf(LOG_OBJ, format("Unable to find repair item for (%s) for prompt report (%s)") % ch->getName() % ch->task->orig_arg);
          }
	}
      }

      if (d->str && (d->prompt_mode != DONT_SEND)) {
          d->output.putInQ(new UncategorizedComm("-> "));
      } else if (d->pagedfile && (d->prompt_mode != DONT_SEND)) {
        sprintf(promptbuf, "\n\r[ %sReturn%s to continue, %s(r)%sefresh, %s(b)%sack, page %s(%d/%d)%s, or %sany other key%s to quit ]\n\r", 
            d->green(),  d->norm(),
            d->green(),  d->norm(),
            d->green(),  d->norm(),
            d->green(),  
            d->cur_page, d->tot_pages, d->norm(),
            d->green(),  d->norm());
        d->output.putInQ(new UncategorizedComm(promptbuf));
      } else if (!d->connected) {
        if (!ch) {
          vlogf(LOG_BUG, "Descriptor in connected mode with NULL desc->character.");
          continue;
        }
        if (d->showstr_head && (d->prompt_mode != DONT_SEND)) {
          sprintf(promptbuf, "\n\r[ %sReturn%s to continue, %s(r)%sefresh, %s(b)%sack, page %s(%d/%d)%s, or %sany other key%s to quit ]\n\r", 
            d->green(),  d->norm(),
            d->green(),  d->norm(),
            d->green(),  d->norm(),
            d->green(),  
            d->cur_page, d->tot_pages, d->norm(),
            d->green(),  d->norm());
          d->output.putInQ(new UncategorizedComm(promptbuf));
        } else {
          if (((d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) ||
               (ch->isPlayerAction(PLR_VT100 | PLR_ANSI) && IS_SET(d->prompt_d.type, PROMPT_VTANSI_BAR)))) {
            if (ch->getHit() != d->last.hit) {
              d->last.hit = ch->getHit();
              SET_BIT(update, CHANGED_HP);
            }
            if (ch->getMana() != d->last.mana) {
              d->last.mana = ch->getMana();
              SET_BIT(update, CHANGED_MANA);
            }
            if (ch->getPiety() != d->last.piety) {
              d->last.piety = ch->getPiety();
              SET_BIT(update, CHANGED_PIETY);
            }
            if (ch->getLifeforce() != d->last.lifeforce) {
              d->last.lifeforce = ch->getLifeforce();
              SET_BIT(update, CHANGED_LIFEFORCE);
            }
            if (ch->getMove() != d->last.move) {
              d->last.move = ch->getMove();
              SET_BIT(update, CHANGED_MOVE);
            }
            if ((ch->getPosition() > POSITION_SLEEPING) && ch->getMoney() != d->last.money) {
              d->last.money = ch->getMoney();
              SET_BIT(update, CHANGED_GOLD);
            }
            if (ch->getCond(FULL) != d->last.full) {
              d->last.full = ch->getCond(FULL);
              SET_BIT(update, CHANGED_COND);
            }
            if (ch->getCond(THIRST) != d->last.thirst) {
              d->last.thirst = ch->getCond(THIRST);
              SET_BIT(update, CHANGED_COND);
            }
            if (ch->getPosition() != d->last.pos) {
              d->last.pos = ch->getPosition();
              SET_BIT(update, CHANGED_POS);
            }
            if (ch->getExp() != d->last.exp) {
              d->last.exp = ch->getExp();
              SET_BIT(update, CHANGED_EXP);
            }
            if (ch->in_room != d->last.room) {
              d->last.room = ch->in_room;
              SET_BIT(update, CHANGED_ROOM);
            }
#if FACTIONS_IN_USE
            if (ch->getPerc() != d->last.perc) {
              d->last.perc = ch->getPerc();
              SET_BIT(update, CHANGED_PERC);
            }
#endif
            if (GameTime::hourminTime() != d->last.mudtime) {
              d->last.mudtime = GameTime::hourminTime();
              SET_BIT(update, CHANGED_MUD);
            }
            if (update || ch->fight()) {
              if (ch->vt100())
                d->updateScreenVt100(update);
              else if (ch->ansi())
                d->updateScreenAnsi(update);

              if (d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
                d->outputProcessing();
                d->send_client_prompt(TRUE, update); // Send client prompt
              }
            }
            /*
            if (d->prompt_mode != DONT_SEND && d->m_bIsClient) {
              strcpy(promptbuf, "> ");
              d->output.putInQ(promptbuf);
            }
            */
          }

          if (d->prompt_mode != DONT_SEND) {
            bool hasColor = IS_SET(d->prompt_d.type, PROMPT_COLOR);
            *promptbuf = '\0';


            if (ch->isImmortal() && IS_SET(d->prompt_d.type, PROMPT_BUILDER_ASSISTANT)) {
              sprintf(promptbuf + strlen(promptbuf),
                      StPrompts[0],
                      ch->roomp->getZoneNum(),
                      (ch->roomp->spec ? "Y" : "N"),
                      ch->roomp->getLight(),
                      ch->roomp->getRoomHeight(),
                      ch->roomp->getMoblim(),
                      TerrainInfo[ch->roomp->getSectorType()]->name);
            }


	    time_t ct=getTimePromptData(ch);
	    int hp=ch->getHit();
	    int mana=ch->getMana();
	    float piety=ch->getPiety();
	    int lifeforce=ch->getLifeforce();
	    int moves=ch->getMove();
	    int gold=ch->getMoney();
	    int room=ch->roomp->number;

	    sprintf(promptbuf + strlen(promptbuf), 
		    getTimePrompt(ch, d, ct).c_str());
	    sprintf(promptbuf + strlen(promptbuf),
		    getHitPointsPrompt(ch, d, hp).c_str());
	    sprintf(promptbuf + strlen(promptbuf),
		    getManaPrompt(ch, d, mana).c_str());
	    sprintf(promptbuf + strlen(promptbuf),
		    getPietyPrompt(ch, d, piety).c_str());
	    sprintf(promptbuf + strlen(promptbuf),
		    getLFPrompt(ch, d, lifeforce).c_str());
	    sprintf(promptbuf + strlen(promptbuf),
		    getMovesPrompt(ch, d, moves).c_str());
	    sprintf(promptbuf + strlen(promptbuf),
		    getMoneyPrompt(ch, d, gold).c_str());
	    sprintf(promptbuf + strlen(promptbuf),
		    getRoomPrompt(ch, d, room).c_str());

	    if (IS_SET(d->prompt_d.type, PROMPT_EXP)) {
	      strcpy(tString, ch->displayExp().comify().c_str());
	      sprintf(promptbuf + strlen(promptbuf),
		      StPrompts[7],
		      (hasColor ? d->prompt_d.expColor : ""),
		      tString,
		      ch->norm());
	    }
            if (IS_SET(d->prompt_d.type, PROMPT_EXPTONEXT_LEVEL)) {
              classIndT i;

              for (i = MAGE_LEVEL_IND; i < MAX_CLASSES; i++)
                if (ch->getLevel(i) && ch->getLevel(i) < MAX_MORT) {
                  if (ch->getExp() > d->prompt_d.xptnl)
                    d->prompt_d.xptnl = getExpClassLevel(i, ch->getLevel(i) + 1);

                  double need = d->prompt_d.xptnl - ch->getExp();

                  sprintf(tString, "%.0f", need);
		  strcpy(tString, sstring(tString).comify().c_str());
                  sprintf(promptbuf + strlen(promptbuf),
                          StPrompts[8],
                          (hasColor ? ch->desc->prompt_d.expColor : ""),
                          tString,
                          ch->norm());
                  break;
                }
            }
            bool bracket_used = false;
            if (IS_SET(d->prompt_d.type, PROMPT_OPPONENT))
              if (ch->fight() && ch->fight()->sameRoom(*ch)) {
                int ratio = min(10, max(0, ((ch->fight()->getHit() * 9) /
                                ch->fight()->hitLimit())));

                sprintf(promptbuf + strlen(promptbuf),
                        StPrompts[9],
//                        (bracket_used ? "" : ">\n\r"),
                        (bracket_used ? "" : " "),
                        (hasColor ? d->prompt_d.oppColor : ""),
                        (ch->isAffected(AFF_ENGAGER) ? "ENGAGED " : ""),
                        ch->persfname(ch->fight()).c_str(), prompt_mesg[ratio],
                        ch->norm());
                bracket_used = true;
              }
            bool isOther = false;
            if ((isOther = IS_SET(d->prompt_d.type, PROMPT_TANK_OTHER)) ||
                IS_SET(d->prompt_d.type, PROMPT_TANK))
              if (ch->fight() && ch->awake() && ch->fight()->sameRoom(*ch)) {
                tank = ch->fight()->fight();

                if (tank && (!isOther || tank != ch)) {
                  if (ch->sameRoom(*tank)) {
                    int ratio = min(10, max(0, ((tank->getHit() * 9) / tank->hitLimit())));

                    sprintf(promptbuf + strlen(promptbuf),
                            StPrompts[10],
//                        (bracket_used ? "" : ">\n\r"),
                        (bracket_used ? "" : " "),
                            (hasColor ? d->prompt_d.tankColor : ""),
                            ch->persfname(tank).c_str(),
                            prompt_mesg[ratio],
                            ch->norm());
                    bracket_used = true;
                  }
                }
              }
            if (d->wait > 1) {
              float waittime = (float) (d->wait - 1) / ONE_SECOND;

              sprintf(promptbuf + strlen(promptbuf),
                      StPrompts[11],
//                        (bracket_used ? "" : ">\n\r"),
                        (bracket_used ? "" : " "),
                      waittime,
                      (IS_SET(d->autobits, AUTO_NOSPAM) ? "" : " secs lockout"));
            }

            strcat(promptbuf, "> ");

	    RoomExitComm *comm=new RoomExitComm();
	    roomDirData *exitData;

	    for (dirTypeT door = MIN_DIR; door < MAX_DIR; door++) {
	      if((exitData = ch->roomp->exitDir(door)) &&
		 (!IS_SET(exitData->condition, EX_SECRET) ||
		  !IS_SET(exitData->condition, EX_CLOSED))){
		comm->exits[door].exit=true;
		if(exitData->door_type != DOOR_NONE)
		  comm->exits[door].door=true;
		else
		  comm->exits[door].door=false;

		comm->exits[door].open=!IS_SET(exitData->condition, EX_CLOSED);
	      } else {
		comm->exits[door].exit=false;
	      }
	    }
	    

            d->output.putInQ(new PromptComm(ct, hp, mana, piety, lifeforce,
					    moves, gold, room, promptbuf));
	    d->output.putInQ(comm);
          }
        }
      }
    }
  }
}

void afterPromptProcessing(fd_set out)
{
  Descriptor *d, *next_d;

  for (d = descriptor_list; d; d = next_d) {
    next_d = d->next;
    if (FD_ISSET(d->socket->m_sock, &out) && d->output.getBegin())
      if (d->outputProcessing() < 0) {
        delete d;
        d = NULL;
      } else
        d->prompt_mode = 0;
  }
}

void Descriptor::saveAll()
{
  Descriptor *d;

  for (d = descriptor_list; d; d = d->next) {
    if (d->character)
      d->character->doSave(SILENT_NO);
  }
}

void Descriptor::worldSend(const sstring &text, TBeing *ch)
{
  Descriptor *d;

  for (d = descriptor_list; d; d = d->next) {
    if (!d->connected)
      d->output.putInQ(new UncategorizedComm(colorString(ch, d, text, NULL, COLOR_BASIC, TRUE)));
  }
}

bool Descriptor::getHostResolved()
{
  return host_resolved;
}

void Descriptor::setHostResolved(bool flag, const sstring &h)
{
  host = h;
  host_resolved = flag;
}

void processAllInput()
{
  Descriptor *d;
  char comm[20000] = "\0\0\0";
  int rc;
  TRoom *rp;

  for (d = descriptor_list; d; d = next_to_process) {
    next_to_process = d->next;

    // this is where PC wait gets handled
    if (!d->getHostResolved()) {
      d->output.putInQ(new UncategorizedComm("\n\rWaiting for DNS resolution...\n\r"));
      continue;
    }
    if ((--(d->wait) <= 0) && (&d->input)->takeFromQ(comm, sizeof(comm))){
      if (d->character && !d->connected && 
          d->character->specials.was_in_room != Room::NOWHERE) {
        --(*d->character);
        rp = real_roomp(d->character->specials.was_in_room);
        *rp += *d->character;
        d->character->specials.was_in_room = Room::NOWHERE;
        act("$n has returned.", TRUE, d->character, 0, 0, TO_ROOM);
      }
      d->wait = 1;
      if (d->character) {
        d->character->setTimer(0);
        if (d->character->isPlayerAction(PLR_AFK)) {
          d->character->sendTo("Your afk flag has been removed.\n\r");
          d->character->remPlayerAction(PLR_AFK);
        }
      }
      if (d->original) {
        d->original->setTimer(0);
        d->original->remPlayerAction(PLR_AFK);
      }
      if (d->prompt_mode != DONT_SEND)
        d->prompt_mode = 1;

      if (is_client_sstring(comm)) {
        rc = d->read_client(comm);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete d;
          d = NULL;
          continue;
        } 
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          delete d->character;
          d->character = NULL;
          continue;
        }
      } else if (d->str) 
        d->sstring_add(comm);
      else if (d->pagedfile) 
        d->page_file(comm);
      else if (!d->account) {            // NO ACCOUNT
        if (d->m_bIsClient) {
          rc = d->client_nanny(comm);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete d;
            d = NULL;
          } 
          continue;
        }
        rc = d->sendLogin(comm);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete d;
          d = NULL;
          continue;
        }
      } else if (!d->account->status) {       //ACCOUNT STUFF 
        rc = d->doAccountStuff(comm);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete d;
          d = NULL;
          continue;
        }
      } else if (!d->connected) {
        if (d->showstr_head) {
          d->show_string(comm, SHOWNOW_NO, ALLOWREP_YES);
        } else {
          rc = d->character->parseCommand(comm, TRUE);
          // the "if d" is here due to a core that showed d=0x0
          // after a purge ldead
          if (d && IS_SET_DELETE(rc, DELETE_THIS)) {
            // in another wierd core, d was no longer in the descriptor_list
            Descriptor *tempdesc;
            for (tempdesc = descriptor_list; tempdesc; tempdesc = tempdesc->next) {
              if (tempdesc == d || tempdesc == next_to_process)
                break;
            }
            if (d && tempdesc == d) {
              delete d->character;
              d->character = NULL;
            } else {
              // either descriptor_list hit end, or d is the next guy to process
              // in all likelihood, this descriptor has already been deleted and we point to free'd memory
              vlogf(LOG_BUG, format("Descriptor not found in list after parseCommand called.  (%s).  VERY BAD!") %  comm);
            }
            continue;
          }
        }
      } else if (!d->character) {
        if (d->account->term && d->account->term == TERM_ANSI)
          SET_BIT(d->plr_act, PLR_ANSI);

        rc = d->doAccountMenu(comm);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete d;
          d = NULL;
          continue;
        }
      } else if (d->connected == CON_REDITING) 
        room_edit(d->character, comm);
      else if (d->connected == CON_OEDITING) 
        obj_edit(d->character, comm);
      else if (d->connected == CON_MEDITING) 
        mob_edit(d->character, comm);
      else if (d->connected == CON_SEDITING)
        seditCore(d->character, comm);
      else if (d->showstr_head) {
        d->show_string(comm, SHOWNOW_YES, ALLOWREP_YES);
      } else {
        rc = d->nanny(comm);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete d;
          d = NULL;
          continue;
        }
      }
    } else if (d->wait <= 0) 
      d->wait = 1;
  }
}

int bogusAccountName(const char *arg)
{
  char buf[256];
  int i = 0;

  strcpy(buf, arg);
  for (i=0; i < (int) strlen(buf) && i <20; i++) {
    if ((buf[i] < 'A') || ((buf[i] > 'Z') && (buf[i] < 'a')) ||
        (buf[i] > 'z'))
      return TRUE;
  }
  return FALSE;
}


sstring LoginComm::getText(){
  return text;
}

sstring LoginComm::getClientText(){
  return getText();
}

sstring LoginComm::getXML(){
  return format("<login prompt=\"%s\">%s</login>") % 
    prompt.escape(sstring::XML) % text.escape(sstring::XML);
}

// return DELETE_THIS
int Descriptor::sendLogin(const sstring &arg)
{
  char buf[160], buf2[4096] = "\0\0\0";
  sstring my_arg = arg.substr(0,20);
  sstring outputBuf;

  if (arg.length() > 20) {
    vlogf(LOG_MISC, format("Buffer overflow attempt from [%s]") % host);
    vlogf(LOG_MISC, format("Login = '%s'") % arg);
  }

  if (m_bIsClient)
    return FALSE;

  if (my_arg.empty())
    return DELETE_THIS;
  else if (my_arg == "?") {
    writeToQ("Accounts are used to store all characters belonging to a given person.\n\r");
    writeToQ("One account can hold multiple characters.  Creating more than one account\n\r");
    sprintf(buf, "for yourself is a violation of %s multiplay rules and will lead to\n\r", MUD_NAME);
    writeToQ(buf);
    writeToQ("strict sanctions.  Your account holds information that is applied to all\n\r");
    writeToQ("characters that you own (including: generic terminal type, time zone\n\r");
    writeToQ("you play from, etc.).  You are required to enter a valid and unique\n\r");
    writeToQ("E-mail address which will be kept secret and used by the game-maintainers\n\r");
    writeToQ("to inform you if a serious problem occurs with your account.\n\r\n\r");
    writeToQ("Note that the only password protection is at the account level.  Do not\n\r");
    writeToQ("reveal your password to others or they have access to ALL of your characters.\n\r\n\r");

    output.putInQ(new LoginComm("user", "Type NEW to generate a new account.\n\rLogin: "));
    return FALSE;
  } else if (my_arg == "1") {
    FILE * fp = fopen("txt/version", "r");
    if (!fp) {
      vlogf(LOG_FILE, "No version file found");
    } else {
      fgets(buf, 79, fp);
      // strip off the terminating newline char
      buf[strlen(buf) - 1] = '\0';

      sprintf(buf2 + strlen(buf2), "\n\r\n\rWelcome to %s:\n\r%s:\n\r", 
	      MUD_NAME_VERS, buf);
      fclose(fp);
    }
	
    tm current_date;
    time_t conv_time;
    conv_time = time(&conv_time);
    localtime_r(&conv_time, &current_date);
    int years_of_service = current_date.tm_year;
    if ((current_date.tm_mon < 4) || (current_date.tm_mon == 4 && current_date.tm_mday <= 5)){
      years_of_service -=  93;
    } else {
      years_of_service -=  92;
    }
    sprintf(buf2 + strlen(buf2), "Celebrating %d years of quality mudding (est. 1 May 1992)\n\r\n\r", years_of_service);
    output.putInQ(new UncategorizedComm(buf2));

    outputBuf="Please type NEW (case sensitive) for a new account, or ? for help.\n\r";
    outputBuf+=format("If you need assistance you may email %s.\n\r\n\r") % MUDADMIN_EMAIL;
    outputBuf+="\n\rLogin: ";
    output.putInQ(new LoginComm("user", outputBuf));
    return FALSE;
  } else if (my_arg == "NEW") {
    if (WizLock) {
      writeToQ("The game is currently wiz-locked.\n\r");
      if (!lockmess.empty()) {
        page_string(lockmess, SHOWNOW_YES);
      } else {
        FILE *signFile;

        if ((signFile = fopen(File::SIGN_MESS, "r"))) {
          fclose(signFile);
          sstring iosstring;
          file_to_sstring(File::SIGN_MESS, iosstring);
          page_string(iosstring, SHOWNOW_YES);
        }
      }
      output.putInQ(new LoginComm("wizlock", "Wiz-Lock password: "));

      account = new TAccount();
      connected = CON_WIZLOCKNEW;
    } else {
      account = new TAccount();
      output.putInQ(new UncategorizedComm("Enter a login name for your account -> "));
      connected = CON_NEWLOG;
    }
    return FALSE;
  } else {
    account = new TAccount();
    if (my_arg == "#")   // NCSA telnet put # when first logging in.
      my_arg = my_arg.substr(1);

    if (bogusAccountName(my_arg.c_str())) {
      output.putInQ(new UncategorizedComm("Illegal account name.\n\r"));
      delete account;
      account = NULL;
      return (sendLogin("1"));
    }
    if(account->read(my_arg)){
      if (account->term == TERM_ANSI) 
	plr_act = PLR_COLOR;
      account->desc = this;
      strcpy(pwd, account->passwd.c_str()); 
    } else 
      *pwd = '\0';
 
    output.putInQ(new LoginComm("pass", "Password: "));
    EchoOff();
    connected = CON_ACTPWD;
  }
  return FALSE;
}

bool Descriptor::checkForAccount(char *arg, bool silent)
{
  TDatabase db(DB_SNEEZY);

  if (bogusAccountName(arg)) {
    if (!silent)
      writeToQ("Sorry, that is an illegal name for an account.\n\r");
    return TRUE;
  }
  
  db.query("select 1 from account where name=lower('%s')", arg);
  if(db.fetchRow()){
    if (!silent)
      writeToQ("Account already exists, enter another name.\n\r");
    return TRUE;
  }
  return FALSE;
}

bool Descriptor::hasCharacterInAccount(const sstring name) const
{
  sstring namelow = name.lower();
  std::vector<sstring> list = listAccountCharacters(account->name);

  for(unsigned int i = 0; i < list.size(); i++)
    if (list[i].lower() == namelow)
      return true;

  return false;
}


bool Descriptor::checkForCharacter(const sstring arg, bool silent)
{
  char buf[256];
  struct stat timestat;

  if (arg.length() <= 0)
    return FALSE;

  sprintf(buf, "player/%c/%s", LOWER(arg[0]), arg.lower().c_str());
 
  if (!stat(buf, &timestat)) {
    if (!m_bIsClient && !silent)
      writeToQ("Character already exists, enter another name.\n\r--> ");
    return TRUE;
  }
  return FALSE;
}


// return DELETE_THIS
int Descriptor::doAccountStuff(char *arg)
{
  char tmp_name[256];
  char buf[256];
  int count = 0;
  sstring lStr;
  struct stat timestat;
  int rc;
  int tss = screen_size;
  TBeing *ch;
  TTrophy *trophy;
  sstring from;
  TDatabase db(DB_SNEEZY);

  // apparently, crypt() has a mem leak in the lib function
  // By making this static, we limit the number of leaks to one
  // rather than one per call.
  static char *crypted;

  for (;isspace(*arg);arg++);

  switch (connected) {
    case CON_NEWLOG:
      // kick um out so they aren't stuck
      if (!*arg) 
        return DELETE_THIS;
      
      if (checkForAccount(arg)) {
        output.putInQ(new UncategorizedComm("Please enter a login name -> "));
        return FALSE;
      } 
      if (strlen(arg) >= 10) {
        output.putInQ(new UncategorizedComm("Account names must be 9 characters or less.\n\r"));
        output.putInQ(new UncategorizedComm("Please enter a login name -> "));
        return FALSE;
      }
      account->name=arg;
      output.putInQ(new UncategorizedComm("Now enter a password for your new account\n\r-> "));
      EchoOff();

      connected = CON_NEWACTPWD;
      break;
    case CON_NEWACTPWD:
      if (strlen(arg) < 5) {
        writeToQ("Your password must contain at least 5 characters.\n\r");
        writeToQ("Password -> ");
        return FALSE;
      } else if (strlen(arg) > 10) {
        writeToQ("Your password can only contain 10 or fewer characters.\n\r");
        writeToQ("Password -> ");
        return FALSE;
      } 
      if(!sstring(arg).hasDigit()){
        writeToQ("Your password must contain at least one number.\n\r");
        writeToQ("Password -> ");
        return FALSE;
      }
      crypted =(char *) crypt(arg, account->name.c_str());
      strncpy(pwd, crypted, 10);
      *(pwd + 10) = '\0';
      writeToQ("Retype your password for verification -> ");
      connected = CON_PWDCNF;
      break;
    case CON_PWDCNF:
      crypted = (char *) crypt(arg, pwd);
      if (strncmp(crypted, pwd, 10)) {
        writeToQ("Mismatched Passwords. Try again.\n\r");
        writeToQ("Retype password -> ");
        connected = CON_NEWACTPWD;
        return FALSE;
      } else {
        account->passwd=pwd;
        EchoOn();
        writeToQ("Enter your email address.\n\r");
        writeToQ("E-mail addresses are used strictly for administrative purposes, or for\n\r");
        writeToQ("contacting you in the event of a problem.  The information is never used for\n\r");
        writeToQ("advertisements, nor will it be provided to any other person or organization.\n\r");
        writeToQ("Bogus addressed accounts will be deleted.\n\rAddress -> ");
        connected = CON_EMAIL;
      }
      break;
    case CON_EMAIL:
      if (!*arg) 
        return DELETE_THIS;
      
      if (illegalEmail(arg, this, SILENT_NO)) {
        writeToQ("Please enter an email address. Accounts with bogus addresses will be deleted.\n\r");
        writeToQ("Address -> ");
        break;
      }
      account->email=arg;

      sprintf(buf, "%s is presently based in Washington state (Pacific Time)\n\r", MUD_NAME);
      writeToQ(buf);
      writeToQ("For purposes of keeping track of time, please enter the difference\n\r");
      writeToQ("between your home site and Pacific Time.  For instance, players on\n\r");
      writeToQ("the East Coast should enter 3, whereas Hawaii would enter -2.\n\r");
      writeToQ("You may alter this later via the TIME command. (see HELP TIME once online).\n\r\n\r");
      writeToQ("Time Difference -> ");
      connected = CON_TIME;
      break;
    case CON_TIME:
      if (!*arg || (convertTo<int>(arg) > 23) || (convertTo<int>(arg) < -23)) {
        writeToQ("Please enter a number from -23 to 23\n\r");
        writeToQ("Time difference -> ");
        break;
      }
      account->time_adjust = convertTo<int>(arg);

      writeToQ("What is your terminal type? (A)nsi, [V]t100 (default), (N)one. -> ");
      connected = CON_TERM;
      break;
    case CON_TERM:
      if (*arg == 'a' || *arg == 'A') 
        account->term = TERM_ANSI;
      else if (!*arg || *arg == 'V' || *arg == 'v')
        account->term = TERM_VT100;
      else if ((*arg == 'n') || *arg == 'N') 
        account->term = TERM_NONE;
      else {
        writeToQ("What is your terminal type? (A)nsi, [V]t100 (default), (N)one -> ");
        return FALSE;
      }
      saveAccount();
      AccountStats::account_number++;

      vlogf(LOG_MISC, format("New Account: '%s' with email '%s'") %  account->name % account->email);

      account->status = TRUE;
      rc = doAccountMenu("");
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case CON_WIZLOCKNEW:
      if (!*arg || strcasecmp(arg, WIZLOCK_PASSWORD)) 
        return DELETE_THIS;

      vlogf(LOG_MISC, "Person making new character after entering wizlock password.");

      output.putInQ(new UncategorizedComm("Enter a login name for your account -> "));
      connected = CON_NEWLOG;
      break;
    case CON_WIZLOCK:
      if (!*arg || strcasecmp(arg, WIZLOCK_PASSWORD)) 
        return DELETE_THIS;
      
      vlogf(LOG_MISC, "Person entering game by entering wizlock password.");

      account->status = TRUE;
      if (!IS_SET(account->flags, TAccount::BOSS)) {
        if (account->term != TERM_ANSI) {
          screen_size = 40;  // adjust for the size of the menu bar temporarily
          start_page_file(NORM_OPEN, "");
          screen_size = tss;
        } else {
          screen_size = 40;  // adjust for the size of the menu bar temporarily
          start_page_file(ANSI_OPEN, "");
          screen_size = tss;
          writeToQ(ANSI_NORMAL);
        }
      }
      writeToQ("Type 'C' to connect with an existing character, or <enter> to see account menu.\n\r-> ");
      break;
    case CON_ACTPWD:
      EchoOn();
      if (!*pwd) {
        writeToQ("Incorrect login.\n\r");
        delete account;
        account = NULL;
        return (sendLogin("1"));
      }
      crypted = (char *) crypt(arg, pwd);
      if (strncmp(crypted, pwd, 10)) {
        writeToQ("Incorrect login.\n\r");
        delete account;
        account = NULL;
        return (sendLogin("1"));
      }
      if (IS_SET(account->flags, TAccount::BANISHED)) {
        writeToQ("Your account has been flagged banished.\n\r");
        sprintf(buf, "If you do not know the reason for this, contact %s\n\r",
              MUDADMIN_EMAIL);
        writeToQ(buf);
        outputProcessing();
        return DELETE_THIS;
      }
      if (IS_SET(account->flags, TAccount::EMAIL)) {
        writeToQ("The email account you entered for your account is thought to be bogus.\n\r");
        sprintf(buf, "You entered an email address of: %s\n\r", account->email.c_str());
        writeToQ(buf);
        sprintf(buf,"To regain access to your account, please send an email\n\rto: %s\n\r",
              MUDADMIN_EMAIL);
        writeToQ(buf);
        writeToQ("Indicate the name of your account, and the reason for the wrong email address.\n\r");
        outputProcessing();
        return DELETE_THIS;
      }
      // let's yank the password out of their history list
      strcpy(history[0], "");

      if (WizLock && !IS_SET(account->flags, TAccount::IMMORTAL)) {
        writeToQ("The game is currently wiz-locked.\n\r");
        if (!lockmess.empty()) {
          page_string(lockmess, SHOWNOW_YES);
        } else {
#if 0
          ifstream op(File::SIGN_MESS, ios::in | ios::nocreate);
          if (op) {
            op.close();
            sstring iosstring;
            file_to_sstring(File::SIGN_MESS, iosstring);
            page_string(iosstring, SHOWNOW_YES);
          }
#else
          FILE *signFile;

          if ((signFile = fopen(File::SIGN_MESS, "r"))) {
            fclose(signFile);
            sstring iosstring;
            file_to_sstring(File::SIGN_MESS, iosstring);
            page_string(iosstring, SHOWNOW_YES);
          }
#endif
        }
        writeToQ("Wiz-Lock password: ");
        connected = CON_WIZLOCK;
        break;
      } else if (WizLock) {
        // wizlock is on, but I am an IMM, just notify me
        writeToQ("The game is currently wiz-locked.\n\r");
        if (!lockmess.empty()) {
          page_string(lockmess, SHOWNOW_YES);
        } else {
#if 0
          ifstream opp(File::SIGN_MESS, ios::in | ios::nocreate);
          if (opp) {
            opp.close();
            sstring iosstring;
            file_to_sstring(File::SIGN_MESS, iosstring);
            page_string(iosstring, SHOWNOW_YES);
          }
#else
          FILE *signFile;

          if ((signFile = fopen(File::SIGN_MESS, "r"))) {
            fclose(signFile);
            sstring iosstring;
            file_to_sstring(File::SIGN_MESS, iosstring);
            page_string(iosstring, SHOWNOW_YES);
          }
#endif
        }
      }
      account->status = TRUE;
      if (!IS_SET(account->flags, TAccount::BOSS)) {
        if (account->term != TERM_ANSI) {
          screen_size = 40;  // adjust for the size of the menu bar temporarily
          start_page_file(NORM_OPEN, "");
          screen_size = tss;
        } else {
          screen_size = 40;  // adjust for the size of the menu bar temporarily
          start_page_file(ANSI_OPEN, "");
          screen_size = tss;
          writeToQ(ANSI_NORMAL);
        }
      }
      count = listAccount(account->name, lStr);
      if (count) 
        writeToQ("Type 'C' to connect with an existing character, or <enter> to see account menu.\n\r-> ");
      else
        writeToQ("Type 'A' to add a new character, or <enter> to see account menu.\n\r-> ");
      break;
    case CON_DELCHAR:
      if (*arg == '/') {
        writeToQ("Which do you want to do?\n\r");
        writeToQ("1) Delete your account\n\r");
        writeToQ("2) Delete a character in your account\n\r");
        writeToQ("3) Return to main account menu.\n\r-> ");
        connected = CON_DELETE;
        break;
      }

      if (_parse_name(arg, tmp_name)) {
        writeToQ("Illegal name, please try another.\n\r");
        writeToQ("Name -> ");
        return FALSE;
      }

      // lower() returns static buf, so add one at a time
      sprintf(buf, "account/%c/%s", LOWER(account->name[0]), sstring(account->name).lower().c_str());
      sprintf(buf + strlen(buf), "/%s", sstring(arg).lower().c_str());
      // sprintf(buf, "account/%c/%s/%s", LOWER(account->name[0]), 
      //                                  account->name.lower(), arg.lower());
      if (stat(buf, &timestat)) {
        writeToQ("No such character.\n\r");
        account->status = TRUE;
        rc = doAccountMenu("");
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        break;
      }
#if 0
      strcpy(delname, arg.lower().c_str());

      char charname[20];
   
      for (ch = character_list; ch; ch = ch->next) {
        strcpy(charname, ch->name.lower().c_str());
	if (!strcmp(charname, delname)) {
	  
	  writeToQ("That character is still connected, so you'll have to\n\r");
	  writeToQ("reconnect and log off before you can delete.\n\r");
	  go_back_menu(connected);
	  break;

        }
      }
      
#endif      

      writeToQ("Deleting a character will result in total deletion and\n\r");
      writeToQ("equipment loss.  Also, if you character is a perma death\n\r");
      writeToQ("character, you may lose your place on the perma death\n\r");
      writeToQ("monument.  Enter your password to verify or hit enter\n\r");
      writeToQ("to return to the account menu system\n\r-> ");
      EchoOff();
      strcpy(delname, sstring(arg).lower().c_str());
      
      connected = CON_CHARDELCNF;
      break;
    
    case CON_CHARDELCNF:
      EchoOn();
      if (!*pwd) {
        writeToQ("Incorrect password.\n\r");
        writeToQ("Which do you want to do?\n\r");
        writeToQ("1) Delete your account\n\r");
        writeToQ("2) Delete a character in your account\n\r");
        writeToQ("3) Return to main account menu.\n\r-> ");
        connected = CON_DELETE;
        break;
      }
      crypted = (char *) crypt(arg, pwd);
      if (strncmp(crypted, pwd, 10)) {
        writeToQ("Incorrect password.\n\r");
        writeToQ("Which do you want to do?\n\r");
        writeToQ("1) Delete your account\n\r");
        writeToQ("2) Delete a character in your account\n\r");
        writeToQ("3) Return to main account menu.\n\r-> ");
        connected = CON_DELETE;
        break;
      }
#if 1
      // it's possible that char is in game (link-dead), check for this
      for (ch = character_list; ch; ch = ch->next) {
	if(sstring(ch->name).lower() == delname){
	  //        if (!strcmp(ch->name.lower().c_str(), delname)) {
          writeToQ("Character is still connected.  Disconnect before deleting.\n\r");
          writeToQ("Which do you want to do?\n\r");
          writeToQ("1) Delete your account\n\r");
          writeToQ("2) Delete a character in your account\n\r");
          writeToQ("3) Return to main account menu.\n\r-> ");
          connected = CON_DELETE;
          break;
        }
      }
      if (ch)
        break;
#endif

      writeToQ("Character deleted.\n\r");
      vlogf(LOG_PIO, format("Character %s self-deleted. (%s account)") %  delname % account->name);
      DeleteHatreds(NULL, delname);
      autobits = 0;
      // remove trophy entries so they do not carry over if the character is recreated
      trophy=new TTrophy(delname);
      trophy->wipe();
      delete trophy;

      // delete ignore list
      db.query("delete from blockedlist where player_id=%i", playerID);

      // delete player entry
      db.query("delete from player where lower(name)=lower('%s')", delname);

      // delete tats!
      db.query("delete from tattoos where lower(name)=lower('%s')", delname);

      wipePlayerFile(delname);  // handles corpses too
      wipeRentFile(delname);
      wipeFollowersFile(delname);

      vlogf(LOG_PIO, format("Deleting mail for character %s.") %  delname);
      db.query("delete from mail where lower(mailto)=lower('%s')", delname);

      sprintf(buf, "account/%c/%s/%s", LOWER(account->name[0]), 
           sstring(account->name).lower().c_str(), delname);
      if (unlink(buf) != 0)
        vlogf(LOG_FILE, format("error in unlink (3) (%s) %d") %  buf % errno);
      account->status = TRUE;
      rc = doAccountMenu("");
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break; 
    case CON_ACTDELCNF:
      EchoOn();
      if (!*pwd) {
        writeToQ("Incorrect password.\n\r");
        writeToQ("Which do you want to do?\n\r");
        writeToQ("1) Delete your account\n\r");
        writeToQ("2) Delete a character in your account\n\r");
        writeToQ("3) Return to main account menu.\n\r-> ");
        connected = CON_DELETE;
        break;
      }
      crypted = (char *) crypt(arg, pwd);
      if (strncmp(crypted, pwd, 10)) {
        writeToQ("Incorrect password.\n\r");
        writeToQ("Which do you want to do?\n\r");
        writeToQ("1) Delete your account\n\r");
        writeToQ("2) Delete a character in your account\n\r");
        writeToQ("3) Return to main account menu.\n\r-> ");
        connected = CON_DELETE;
        break;
      }
      deleteAccount();
      writeToQ("Your account has been deleted, including all characters and equipment.\n\r");
      sprintf(buf, "Thank you for playing %s, and hopefully you will remake your character(s)!\n\r", MUD_NAME);
      writeToQ(buf);
      outputProcessing();
      return DELETE_THIS;
    case CON_DELETE:
      switch (*arg) {
        case '1':
          writeToQ("Are you sure you want to completely delete your account?\n\r");
          writeToQ("Doing so will delete all characters and their equipment.\n\r");
          writeToQ("If you are sure, enter your password for verification.\n\r");
          writeToQ("Otherwise hit enter.\n\r-> ");
          EchoOff();
          connected = CON_ACTDELCNF;
          break;
        case '2':
          writeToQ("To abort the deletion type '/', otherwise ...\n\r");
          writeToQ("Enter the name of the character you wish to delete.\n\r-> ");
          connected = CON_DELCHAR;
          break; 
        case '3':
          account->status = TRUE;
          rc = doAccountMenu("");
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
          break;
        default:
          writeToQ("Which do you want to do?\n\r");
          writeToQ("1) Delete your account\n\r");
          writeToQ("2) Delete a character in your account\n\r");
          writeToQ("3) Return to main account menu.\n\r-> ");
          break;
      }
      break;
    case CON_OLDPWD:
      EchoOn();
      if (!*pwd) {
        writeToQ("Incorrect password.\n\r");
        writeToQ("[Press return to continue]\n\r");
        account->status = TRUE;
        break;
      }
      crypted = (char *) crypt(arg, pwd);
      if (strncmp(crypted, pwd, 10)) {
        writeToQ("Incorrect password.\n\r");
        writeToQ("[Press return to continue]\n\r");
        account->status = TRUE;
        break;
      }
      writeToQ("Enter new password -> ");
      EchoOff();
      connected = CON_NEWPWD;
      break;
    case CON_NEWPWD:
      EchoOn();
      if (strlen(arg) < 5) {
        writeToQ("Your password must contain at least 5 characters.\n\r");
        writeToQ("Password -> ");
        EchoOff();
        return FALSE;
      } else if (strlen(arg) > 10) {
        writeToQ("Your password can only contain 10 or fewer characters.\n\r");
        writeToQ("Password -> ");
        EchoOff();
        return FALSE;
      }
      if (!sstring(arg).hasDigit()) {
        writeToQ("Your password must contain at least one number.\n\r");
        writeToQ("Password -> ");
        EchoOff();
        return FALSE;
      }
      crypted = (char *) crypt(arg, account->name.c_str());
      strncpy(pwd, crypted, 10);
      *(pwd + 10) = '\0';
      writeToQ("Retype your password for verification -> ");
      EchoOff();
      connected = CON_RETPWD;
      break;
    case CON_RETPWD:
      crypted = (char *) crypt(arg, pwd);
      EchoOn();
      if (strncmp(crypted, pwd, 10)) {
        writeToQ("Mismatched Passwords. Try again.\n\r");
        writeToQ("Retype password -> ");
        connected = CON_NEWPWD;
        EchoOff();
      } else {
        account->passwd=pwd;
        account->status = TRUE;
        saveAccount();
        writeToQ("Password changed successfully.\n\r");
        writeToQ("[Press return to continue]\n\r");
      }
      break;
    default:
      vlogf(LOG_BUG, format("Bad connectivity in doAccountStuff() (%d, %s, %s)") %  
          connected % (character ? character->getName() : "false") % "0");
      vlogf(LOG_BUG, "Trying to delete it.");
      return DELETE_THIS;
  }
  return FALSE;
}

// returns DELETE_THIS
int Descriptor::doAccountMenu(const char *arg)
{
  sstring lStr;
  int count = 1;
  int tss = screen_size;

  bonus_points.total= bonus_points.combat= bonus_points.combat2= bonus_points.learn= bonus_points.util=0;

  if (m_bIsClient) {
    clientf(format("%d") % CLIENT_MENU);
    return DELETE_THIS;
  }
  if (!connected) {
    vlogf(LOG_BUG, "DEBUG: doAM with !connected");
    return DELETE_THIS;
  }
#if 1
// COSMO TEST-fake color setting
  if (account && account->term == TERM_ANSI) {
    SET_BIT(plr_act, PLR_COLOR);
  }
#endif
  switch (*arg) {
    case 'E':
    case 'e':
      return DELETE_THIS;
    case 'A':
    case 'a':
      writeToQ("Enter the name for your character -> ");
      character = new TPerson(this);
      mud_assert(character != NULL, "Mem alloc problem");
      connected = CON_CREATION_START;
      break;
    case 'C':
    case 'c':
      writeToQ("Enter name of character -> ");
      character = new TPerson(this);
      connected = CON_CONN;
      break;
    case 'M':
    case 'm':
      sendMotd(FALSE);
      writeToQ("[Press return to continue]\n\r");
      break;
    case 'N':
    case 'n':
      start_page_file(File::NEWS, "No news today\n\r");
      break;
    case 'L':
    case 'l':
      count = listAccount(account->name, lStr);
      if (count == 0)
        writeToQ("No characters in account.\n\r");
      else
        writeToQ(lStr);

      writeToQ("\n\r");
      writeToQ("[Press return to continue]\n\r");
      break;
    case 'P':
    case 'p':
      writeToQ("Enter old password -> ");
      EchoOff();
      account->status = FALSE;
      connected = CON_OLDPWD;
      break; 
    case 'W':
    case 'w':
      menuWho();
      break;
    case 'D':
    case 'd':
      writeToQ("Which do you want to do?\n\r");
      writeToQ("1) Delete your account\n\r");
      writeToQ("2) Delete a character in your account\n\r");
      writeToQ("3) Return to main account menu.\n\r-> ");
      account->status = FALSE; //set status to FALSE to throw back into loop.
      connected = CON_DELETE;
      break;
    case 'H':
    case 'h':
      start_page_file("txt/accounthelp", "No help for account menu currently.");
      writeToQ("\n\r[Press return to continue]\n\r");
      break;
    case 'F':
    case 'f':
      writeToQ("Fingering other accounts is not yet enabled.\n\r");
      writeToQ("\n\r[Press return to continue]\n\r");
      break;
    default:
      count = ::number(1,3);
      if (!IS_SET(account->flags, TAccount::BOSS)) {
        if (account->term == TERM_ANSI) {
          screen_size = 40;  // adjust for the size of the menu bar temporarily
         
          if (count == 1)
            start_page_file(ANSI_MENU_1, "");
          else if (count == 2)
            start_page_file(ANSI_MENU_2, "");
          else {
//            start_page_file(ANSI_MENU_3, "");
            sstring fileBuf;
            if (file_to_sstring(ANSI_MENU_3, fileBuf)) {
               fileBuf += "\n\r";
               page_string(fileBuf);
            }
          }
          screen_size = tss;
          writeToQ(ANSI_NORMAL);
        } else {
          screen_size = 40;  // adjust for the size of the menu bar temporarily
          if (count == 1)
            start_page_file(NORM_MENU_1, "");
          else if (count == 2)
            start_page_file(NORM_MENU_2, "");
          else 
            start_page_file(NORM_MENU_3, "");
          screen_size = tss;
        }
      } else {
        char buf[256];

        writeToQ("<<C>onnect an existing character       <<A>dd a new character\n\r");
        writeToQ("<<D>elete account or character         <<M>essage of the day\n\r");
        sprintf(buf, "<<N>ews of %-25.25s   <<F>inger an account\n\r", MUD_NAME);
        writeToQ(buf);
        writeToQ("<<W>ho is in the game                  <<P>assword change\n\r");
        writeToQ("<<L>ist characters in account          <<H>elp\n\r");
        sprintf(buf, "<<E>xit %s\n\r", MUD_NAME);
        writeToQ(buf);
        writeToQ("\n\r-> ");
      }
      break;
  }
  return FALSE;
}

void Descriptor::saveAccount()
{
  sstring path;

  if(!account->write(account->name)){
    vlogf(LOG_FILE, format("Big problems in saveAccount (%s)") % 
	  account->name.lower());
  }

  path=((sstring)(format("account/%c/%s") % account->name[0] % account->name)).lower();
  if (mkdir(path.c_str(), 0770) && errno != EEXIST){
    vlogf(LOG_FILE, format("Can't make directory for Descriptor::saveAccount (%s) (%i)") %
	  path % errno);
  }
}

void Descriptor::deleteAccount()
{
  DIR *dfd;
  struct dirent *dp;
  char buf[256];

  vlogf(LOG_PIO, format("Account %s self-deleted.") % account->name);

  sprintf(buf, "account/%c/%s", LOWER(account->name[0]), sstring(account->name).lower().c_str());
  if (!(dfd = opendir(buf))) {
    vlogf(LOG_FILE, format("Unable to walk directory for delete account (%s account)") %  account->name);
    return;
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;

    sprintf(buf, "account/%c/%s/%s", LOWER(account->name[0]), sstring(account->name).lower().c_str(), dp->d_name);
    if (unlink(buf) != 0)
      vlogf(LOG_FILE, format("error in unlink (4) (%s) %d") %  buf % errno);

    // these are in the dir, but are not "players"
    if (!strcmp(dp->d_name, "comment") ||
        !strcmp(dp->d_name, "account"))
      continue;

    wipePlayerFile(dp->d_name);
    wipeRentFile(dp->d_name);
    wipeFollowersFile(dp->d_name);
  }

  TDatabase db(DB_SNEEZY);
  db.query("delete from account where name='%s'",
	   sstring(account->name).lower().c_str());

  sprintf(buf, "account/%c/%s", LOWER(account->name[0]), sstring(account->name).lower().c_str());
  rmdir(buf);
  AccountStats::account_number--;
  closedir(dfd);
}


int Descriptor::inputProcessing()
{
  int sofar, thisround, bgin, squelch, i, k, flag;
  char tmp[20000], buffer[20000];
  int which = -1, total = 0, count = 0;
  char *s, *s2;
  TBeing *ch = original ? original : character;

  sofar = 0;
  flag = 0;
  bgin = strlen(m_raw);

  do {
    if ((thisround = read(socket->m_sock, m_raw + bgin + sofar,
                          4096 - (bgin + sofar) - 1)) > 0) {
      sofar += thisround;
    } else {
      if (thisround < 0) {
        if (errno != EWOULDBLOCK) {
          perror("Read1 - ERROR");
          return (-1);
        } else 
          break;
        
      } else {
        vlogf(LOG_PIO, "EOF encountered on socket read.");
        return (-1);
      }
    }
  } while (!ISNEWL(*(m_raw + bgin + sofar - 1)));

  *(m_raw + bgin + sofar) = 0;

  for (i = bgin; !ISNEWL(*(m_raw + i)); i++)
    if (!*(m_raw + i))
      return (0);

  for (i = 0, k = 0; *(m_raw + i);) {
    if (!ISNEWL(*(m_raw + i)) && !(flag = (k >= (!m_bIsClient ? (MAX_INPUT_LENGTH - 2) : 4096)))) {
      if (*(m_raw + i) == '\b') {      // backspace 
        if (k) {                // more than one char ? 
          if (*(tmp + --k) == '$')
            k--;
          i++;
        } else
          i++;
      } else {
        if ((*(m_raw + i) == '\200') ||
            (isascii(*(m_raw + i)) && isprint(*(m_raw + i)))) {
          // trans char, double for '$' (printf)   
          if ((*(tmp + k) = *(m_raw + i)) == '$')
            *(tmp + ++k) = '$';
          k++;
          i++;
        } else
          i++;
      }
    } else {
      *(tmp + k) = 0;

      // New history related c-shell type commands - Russ 
      if (*tmp == '!') {
        if (!tmp[1] || (tmp[1] == '!'))
          strcpy(tmp, history[0]);
        else if ((tmp[1] >= '0') && (tmp[1] <= '9'))
          strcpy(tmp, history[ctoi(tmp[1])]);
        else {
          for (s = tmp + 1, k = 0; k <= HISTORY_SIZE-1; k++) {
            s2 = history[k];
            while (*s && *s2 && (*s == *s2)) {
              s++;
              s2++;
              count++;
            }
            if (count > total)
              which = k;

            count = 0;
          }
          if (which >= 0)
            strcpy(tmp, history[which]);
        }
      } else if (*tmp == '^') {
      } else {
        // by default, put everything in history
        add_to_history_list(tmp);
      }

      input.putInQ(tmp);

      if (snoop.snoop_by && snoop.snoop_by->desc) {
	sstring outputBuf=tmp;
	outputBuf+="\n\r";
	snoop.snoop_by->desc->output.putInQ(new SnoopComm(ch->getName(), outputBuf));
      }
      if (flag) {
        sprintf(buffer, "Line too long. Truncated to:\n\r%s\n\r", tmp);
        if (socket && socket->writeToSocket(buffer) < 0) {
          return (-1);
        }

        // skip the rest of the line 
        for (; !ISNEWL(*(m_raw + i)); i++);
      }
      // find end of entry 
      for (; ISNEWL(*(m_raw + i)); i++);

      // squelch the entry from the buffer 
      for (squelch = 0;; squelch++) {
        if ((*(m_raw + squelch) = *(m_raw + i + squelch)) == '\0')
          break;
      }
      k = 0;
      i = 0;
    }
  }
  return TRUE;
}

void Descriptor::sendMotd(int wiz)
{
  char wizmotd[MAX_STRING_LENGTH] = "\0\0\0";
  char motd[MAX_STRING_LENGTH] = "\0\0\0";
  sstring version;
  struct stat timestat;

  file_to_sstring("txt/version", version);
  // file_to_str adds \n\r, strip both off
  size_t iter = version.find_last_not_of(" \n\r");
  if (iter != sstring::npos)
    version.erase(iter+1);

  sprintf(motd + strlen(motd), "\n\r\n\r     Welcome to %s\n\r     %s\n\r\n\r", MUD_NAME_VERS, version.c_str());

  file_to_sstring(File::MOTD, version);
  // swap color sstrings
  version = colorString(character, this, version, NULL, COLOR_BASIC,  false);
  strcat(motd, version.c_str());

  if (stat(File::NEWS, &timestat)) {
    vlogf(LOG_BUG, "bad call to news file");
    return;
  }

  sprintf(motd + strlen(motd), "\n\rREAD the NEWS LAST UPDATED       : %s\n\r",
                                ctime(&(timestat.st_mtime)));
  if (wiz) {
    file_to_sstring(File::WIZMOTD, version);
    // swap color sstrings
    version = colorString(character, this, version, NULL, COLOR_BASIC,  false);
    strcat(motd, version.c_str());
    if (stat(File::WIZNEWS, &timestat)) {
      vlogf(LOG_BUG, "bad call to wiznews file");
      return;
    }
    sprintf(wizmotd + strlen(wizmotd), 
                              "\n\rREAD the WIZNEWS LAST UPDATED    : %s\n\r",
                               ctime(&(timestat.st_mtime)));
  }

  if (!m_bIsClient) {
    writeToQ(motd);
    if (wiz)
      writeToQ(wizmotd);
   
    return;
  } else {
    sstring sb;
    if (!wiz) {
      sb = motd;
    } else {
      sb = wizmotd;
    }
    processStringForClient(sb);
    clientf(format("%d|%s") % CLIENT_MOTD % sb);
  }
}

Comm *outputQ::getBegin()
{
  if(!queue.empty()){
    return queue.front();
  }

  return NULL;
}

Comm *outputQ::getEnd()
{
  if(!queue.empty()){
    return queue.back();
  }

  return NULL;
}


Comm *outputQ::takeFromQ()
{
  Comm *c=NULL;

  if(!queue.empty()){
    c=queue.front();
    queue.pop_front();
  }

  return c;
}

void outputQ::putInQ(Comm *c)
{
  queue.push_back(c);
}

bool inputQ::takeFromQ(char *dest, int destsize)
{
  commText *tmp = NULL;

  // Are we empty?
  if (!begin)
    return (0);
 
  if (begin->getText())
    strncpy(dest, begin->getText(), destsize-1);
  else {
    vlogf(LOG_BUG, "There was a begin with no text but a next");
    return (0);
  }
  // store it off for later
  tmp = begin;

  // update linked list-- added the if 12/02/97 cos
  if ((begin = begin->getNext())) {
    if (begin->getNext() == begin) {
      begin->setNext(NULL);
      begin = NULL;
      vlogf(LOG_BUG, "Tell a coder, begin->next = begin");
    }
  } else {
    begin = NULL;
    end = NULL;
  }

  // trash the old text q : causes the old begin->text to be deleted
  delete tmp;
  tmp = NULL;

  return (1);
}

void inputQ::putInQ(const sstring &txt)
{
  commText *n;
 
  n = new commText();
  if (!n) {
    vlogf(LOG_BUG, "Failed mem alloc in putInQ()");
    return;
  }
  char *tx = mud_str_dup(txt);
  n->setText(tx);
  n->setNext(NULL);
 
  if (!begin) {
    begin = end = n;
  } else {
    end->setNext(n);
    end = n;
  }
}

int TBeing::applyAutorentPenalties(int secs)
{
  if(Config::PenalizeForAutoRenting()){
    vlogf(LOG_PIO, format("%s was autorented for %d secs") %
	  (getName() ? getName() : "Unknown name") % secs);    
  }

  return FALSE;
}

int TBeing::applyRentBenefits(int secs)
{
  int local_tics, rc = 0, lfmod = 1;
  affectedData *af = NULL, *next_af_dude = NULL;
  int amt, transFound = FALSE;

  // award healing for every 3 ticks gone gone
  local_tics = secs / SECS_PER_UPDATE;
  local_tics /= 3;  // arbitrary

  vlogf(LOG_PIO, format("%s was rented for %d secs, counting as %d tics out-of-game") % 
        getName() % secs % local_tics);

  setHit(min((int) hitLimit(), getHit() + (local_tics * hitGain())));
  setMana(min((int) manaLimit(), getMana() + (local_tics * manaGain())));
  setMove(min((int) moveLimit(), getMove() + (local_tics * moveGain())));
  setPiety(min(pietyLimit(), getPiety() + (local_tics * pietyGain(0.0))));
  /////////////////
  // LIFEFORCE
  /////////////////
  int X = getLifeforce();
  if (X < 50) {
    setLifeforce(X);
  } else {
    X = getLifeforce() - (secs * lfmod);
    setLifeforce(max(50,X));
  }

  /////////////////////////
  //  setLifeforce(min(getLifeforce(), getLifeforce() + (local_tics * lfmod)));
  //  if ((getLifeforce() + (local_tics * lfmod)) < 50)
  //  setLifeforce(50);
  // THIS WILL NEED TO BE REVIEWED
  ////////////////////////
 
  wearSlotT ij;
  for (ij=MIN_WEAR;ij < MAX_WEAR; ij++) {
    amt = min(local_tics, getMaxLimbHealth(ij) - getCurLimbHealth(ij));  
    addCurLimbHealth(ij, amt);
  }

  // a stripped down updateAffects()
  for (af = affected; af; af = next_af_dude) {
    next_af_dude = af->next;

    if (af->duration == PERMANENT_DURATION)
      continue;
    if ((af->duration - (local_tics * UPDATES_PER_MUDHOUR)) <= 0) {
      if (((af->type >= MIN_SPELL) && (af->type < MAX_SKILL)) ||
          ((af->type >= FIRST_TRANSFORMED_LIMB) && (af->type < LAST_TRANSFORMED_LIMB)) ||
          ((af->type >= FIRST_BREATH_WEAPON) && (af->type < LAST_BREATH_WEAPON)) ||
          ((af->type >= FIRST_ODDBALL_AFFECT) && (af->type < LAST_ODDBALL_AFFECT))) {
        if (af->shouldGenerateText() ||
            (af->next->duration > 0)) {
          if (af->type == AFFECT_DISEASE)
            diseaseStop(af);
          else {
            rc = spellWearOff(af->type);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
          }
        }
        if ((af->type == AFFECT_TRANSFORMED_ARMS) || 
            (af->type == AFFECT_TRANSFORMED_HANDS) ||
            (af->type == AFFECT_TRANSFORMED_LEGS) ||
            (af->type == AFFECT_TRANSFORMED_HEAD) ||
            (af->type == AFFECT_TRANSFORMED_NECK)) {
           transFound = TRUE;
        }
        affectRemove(af);
      }
    } else 
      af->duration -= local_tics * UPDATES_PER_MUDHOUR;
  }
  if (transFound)
    transformLimbsBack("", MAX_WEAR, FALSE);

  if (secs >= 6 * SECS_PER_REAL_HOUR) {
    sendTo("A full night's rest has restored your body!\n\r");
    if (!isImmortal()) {
      setCond(FULL, 24);
      setCond(THIRST, 24);
      setCond(DRUNK, 0);
    }
  }
  return FALSE;
}

bool illegalEmail(char *buf, Descriptor *desc, silentTypeT silent)
{
  sstring arg, username, host;
  unsigned int str_length = 0;

  arg = buf;

  if (arg.length() == 0) {
    if (desc && !silent)
      desc->writeToQ("You must enter an email address.\n\r");
    return TRUE;
  }
  if (arg.length() > 60) {
    if (desc && !silent)
      desc->writeToQ("Email addresses may not exceed 60 characters.\n\r");
    return TRUE;
  }
  if (arg.find("@") == sstring::npos) {
    if (desc && !silent)
      desc->writeToQ("Email addresses must be of the form <user>@<host>.\n\r");
    return TRUE;
  }
  str_length = arg.find_first_of("@");
  if (str_length == sstring::npos) {
    if (desc && !silent)
      desc->writeToQ("You must enter a username.\n\r");
    return TRUE;
  }
  username = arg.substr(0, str_length);
 
  str_length = arg.find_first_not_of("@", str_length);
  if (str_length == sstring::npos) {
    if (desc && !silent)
      desc->writeToQ("You must enter a host.\n\r");
    return TRUE;
  }
  host = arg.substr(str_length);

  if (username.find_first_of(" ") != sstring::npos) {
    if (desc && !silent) {
      desc->writeToQ("User names can not have spaces in them.\n\r");
    }
    return TRUE;
  }

  if (host.find_first_of(" ") != sstring::npos) {
    if (desc && !silent) {
      desc->writeToQ("Host names can not have spaces in them.\n\r");
    }
    return TRUE;
  }

  host = host.lower();

  if ((host.find("localhost") != sstring::npos) ||
      (host.find("127.0.0.1") != sstring::npos)) {
    if (desc && !silent)
      desc->writeToQ("Apologies, but due to abuse, that host is disallowed.\n\r");
    return TRUE;
  }

#if 0
// This works, but was deemed a bad idea...
  char tempBuf[256]; 
  struct hostent *hst;

  hst = gethostbyname(host);
  if (!hst) {
    if (desc && !silent) {
      sprintf(tempBuf, "Host '%s' does not seem to be valid.\n\r", host); 
      desc->writeToQ(tempBuf);
      sprintf(tempBuf, "If this is a valid host, please send e-mail to %s\n\r\n\r\n\r", MUDADMIN_EMAIL);
      desc->writeToQ(tempBuf);
    }
    return TRUE;
  }
#endif

  return FALSE;
}

commText::commText()
      : text(NULL), next(NULL)
{
}

commText::commText(const commText &a)
{
  text = mud_str_dup(a.text);
  if (a.next)
    next = new commText(*a.next);
  else
    next = NULL;
}

commText & commText::operator=(const commText &a)
{
  if (this == &a) return *this;

  delete [] text;
  text = mud_str_dup(a.text);

  commText *ct;
  while ((ct = next)) {
    next = ct->next;
    delete ct;
  }

  if (a.next)
    next = new commText(*a.next);
  else
    next = NULL;
  return *this;
}

commText::~commText()
{
  if (text) {
    delete [] text;
    text = NULL;
  }
}

inputQ::inputQ(bool n) :
  begin(NULL),
  end(NULL)
{
}

inputQ::inputQ(const inputQ &a) :
  begin(NULL),
  end(NULL)
{
  if (a.begin)
    begin = new commText(*a.begin);
  else
    begin = NULL;

  if (a.begin) {
    commText *ct = NULL;
    for (ct = begin; ct->getNext(); ct = ct->getNext());
    end = ct;
  } else
    end = NULL;
}

inputQ & inputQ::operator=(const inputQ &a)
{
  if (this == &a) return *this;
  commText *ct, *ct2;
  for (ct = begin; ct; ct = ct2) {
    ct2 = ct->getNext();
    delete ct;
  }
  if (a.begin)
    begin = new commText(*a.begin);
  else
    begin = NULL;

  if (a.begin) {
    for (ct = begin; ct->getNext(); ct = ct->getNext());
    end = ct;
  } else
    end = NULL;

  return *this;
}

inputQ::~inputQ()
{
  commText *ct, *ct2;
  for (ct = begin; ct; ct = ct2) {
    ct2 = ct->getNext();
    delete ct;
  }
}

outputQ::~outputQ()
{
  queue.clear();
}

editStuff::editStuff()
  : x(1), y(1),
    bottom(0), end(0), lines(NULL)
{
}

editStuff::editStuff(const editStuff &a)
  : x(a.x), y(a.y),
    bottom(a.bottom), end(a.end)
{
  lines = a.lines;  // not positive this is correct
}

editStuff::~editStuff()
{
}

careerData::careerData()
{
  setToZero();
}

careerData::~careerData()
{
}

sessionData::sessionData()
{
  setToZero();
}

sessionData::~sessionData()
{
}

sessionData & sessionData::operator=(const sessionData &assign)
{
  if (this == &assign)
    return *this;

  connect = assign.connect;
  kills = assign.kills;
  groupKills = assign.groupKills;
  xp = assign.xp;
  perc = assign.perc;
  group_share = assign.group_share;
  groupName = assign.groupName;
  amGroupTank = assign.amGroupTank;
  skill_success_attempts = assign.skill_success_attempts;
  skill_success_pass = assign.skill_success_pass;
  spell_success_attempts = assign.spell_success_attempts;
  spell_success_pass = assign.spell_success_pass;
  prayer_success_attempts = assign.prayer_success_attempts;
  prayer_success_pass = assign.prayer_success_pass;
  hones = assign.hones;

  attack_mode_t i;
  for (i= ATTACK_NORMAL; i < MAX_ATTACK_MODE_TYPE; i++)
  {
    hits[i] = assign.hits[i];
    swings[i] = assign.swings[i];
    rounds[i] = assign.rounds[i];
    combat_dam_done[i] = assign.combat_dam_done[i];
    combat_dam_received[i] = assign.combat_dam_received[i];
    potential_dam_done[i] = assign.potential_dam_done[i];
    potential_dam_received[i] = assign.potential_dam_received[i];
    skill_dam_done[i] = assign.skill_dam_done[i];
    skill_dam_received[i] = assign.skill_dam_received[i];
    swings_received[i] = assign.swings_received[i];
    hits_received[i] = assign.hits_received[i];
    rounds_received[i] = assign.rounds_received[i];
    level_attacked[i] = assign.level_attacked[i];
    mod_done[i] = assign.mod_done[i];
    mod_received[i] = assign.mod_received[i];
  }

  return *this;
}


void sessionData::minus(sessionData &sd, const sessionData &first, const sessionData &second)
{
  sd.connect = first.connect - second.connect;
  sd.kills = first.kills - second.kills;
  sd.groupKills = first.groupKills - second.groupKills;
  sd.xp = first.xp - second.xp;
  sd.perc = first.perc - second.perc;
  sd.group_share = first.group_share - second.group_share;
  sd.groupName = first.groupName;
  sd.amGroupTank = first.amGroupTank;
  sd.skill_success_attempts = first.skill_success_attempts - second.skill_success_attempts;
  sd.skill_success_pass = first.skill_success_pass - second.skill_success_pass;
  sd.spell_success_attempts = first.spell_success_attempts - second.spell_success_attempts;
  sd.spell_success_pass = first.spell_success_pass - second.spell_success_pass;
  sd.prayer_success_attempts = first.prayer_success_attempts - second.prayer_success_attempts;
  sd.prayer_success_pass = first.prayer_success_pass - second.prayer_success_pass;
  sd.hones = first.hones - second.hones;

  attack_mode_t i;
  for (i= ATTACK_NORMAL; i < MAX_ATTACK_MODE_TYPE; i++)
  {
    sd.hits[i] = first.hits[i] - second.hits[i];
    sd.swings[i] = first.swings[i] - second.swings[i];
    sd.rounds[i] = first.rounds[i] - second.rounds[i];
    sd.combat_dam_done[i] = first.combat_dam_done[i] - second.combat_dam_done[i];
    sd.combat_dam_received[i] = first.combat_dam_received[i] - second.combat_dam_received[i];
    sd.potential_dam_done[i] = first.potential_dam_done[i] - second.potential_dam_done[i];
    sd.potential_dam_received[i] = first.potential_dam_received[i] - second.potential_dam_received[i];
    sd.skill_dam_done[i] = first.skill_dam_done[i] - second.skill_dam_done[i];
    sd.skill_dam_received[i] = first.skill_dam_received[i] - second.skill_dam_received[i];
    sd.swings_received[i] = first.swings_received[i] - second.swings_received[i];
    sd.hits_received[i] = first.hits_received[i] - second.hits_received[i];
    sd.rounds_received[i] = first.rounds_received[i] - second.rounds_received[i];
    sd.level_attacked[i] = first.level_attacked[i] - second.level_attacked[i];
    sd.mod_done[i] = first.mod_done[i] - second.mod_done[i];
    sd.mod_received[i] = first.mod_received[i] - second.mod_received[i];
  }
}

sessionData sessionData::operator-(const sessionData &that)
{
  sessionData sd;
  minus(sd, *this, that);
  return sd;
}

sessionData & sessionData::operator-=(const sessionData &that)
{
  minus(*this, *this, that);
  return *this;
}


promptData::promptData() :
  type(0),
  prompt(NULL),
  xptnl(0.0)
{
  *hpColor = *moneyColor = *manaColor = *moveColor = *expColor = '\0';
  *roomColor = *oppColor = *tankColor = *timeColor = '\0';
  *pietyColor = *lifeforceColor = '\0';

//  for (classIndT i = MIN_CLASS_IND; i < MAX_CLASSES; i++)
//    xptnl[i] = 0.0;
}

promptData::~promptData()
{
}
