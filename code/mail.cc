#include "stdsneezy.h"
#include "mail.h"
#include "database.h"

// may not exceed NAME_SIZE (15) chars
static const char * const SNEEZY_ADMIN = "SneezyMUD Administration";


bool has_mail(const char *recipient)
{
  TDatabase db(DB_SNEEZY);

  db.query("select count(*) from mail where lower(mailto)=lower('%s')", recipient);

  if(db.fetchRow() && convertTo<int>(db.getColumn(0)) != 0)
    return TRUE;

  return FALSE;
}

void store_mail(const char *to, const char *from, const char *message_pointer)
{
  TDatabase db(DB_SNEEZY);
  time_t mail_time;
  char *tmstr;

  mail_time=time(0);
  tmstr = asctime(localtime(&mail_time));
  *(tmstr + strlen(tmstr) - 1) = '\0';


  if(!strcmp(to, "faction")){
    TDatabase fm(DB_SNEEZY);
    fm.query("select name from factionmembers where faction=(select faction from factionmembers where name='%s')", from);
    
    while(fm.fetchRow()){
      db.query("insert into mail (port, mailfrom, mailto, timesent, content) values (%i, '%s', '%s', '%s', '%s')", gamePort, from, fm.getColumn(0), tmstr, message_pointer);
    }
  } else {
    db.query("insert into mail (port, mailfrom, mailto, timesent, content) values (%i, '%s', '%s', '%s', '%s')", gamePort, from, to, tmstr, message_pointer);
  }
}                               /* store mail */

sstring read_delete(const char *recipient, const char *recipient_formatted, sstring &from)
{
  TDatabase db(DB_SNEEZY);
  sstring buf;

  db.query("select mailfrom, timesent, content, mailid from mail where port=%i and lower(mailto)=lower('%s')", gamePort, recipient);
  if(!db.fetchRow())
    return "error!";

  from=db.getColumn(0);

  ssprintf(buf,
	   "The letter has a date stamped in the corner: %s\n\r\n\r"
           "%s,\n\r"
           "%s\n\r"
           "Signed, %s\n\r\n\r",
	   db.getColumn(1),
	   recipient_formatted,
	   db.getColumn(2), 
	   db.getColumn(0));

  db.query("delete from mail where mailid=%s", db.getColumn(3));
  
  return sstring(buf);
}

int postmaster(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  if (!ch->desc)
    return FALSE;               /* so mobs don't get caught here */

  switch (cmd) {
    case CMD_MAIL: 
      ch->postmasterSendMail(arg, myself);
      return TRUE;
      break;
    case CMD_CHECK: 
      ch->postmasterCheckMail(myself);
      return TRUE;
      break;
    case CMD_RECEIVE:
      ch->postmasterReceiveMail(myself);
      return TRUE;
      break;
    default:
      return FALSE;
      break;
  }
}

int mail_ok(TBeing *ch)
{
  if (no_mail) {
    ch->sendTo("Sorry, the mail system is having technical difficulties.\n\r");
    return FALSE;
  }
  return TRUE;
}

void TBeing::postmasterSendMail(const char *arg, TMonster *me)
{
  char buf[200], recipient[100], *tmp;
  charFile st;
  int i, imm = FALSE;

// added this check - bat
  if (!mail_ok(this))
    return;

  if (!*arg) {
    sprintf(buf, "%s You need to specify an addressee!", getName());
    me->doTell(buf);
    return;
  }
  if (_parse_name(arg, recipient)) {
    sendTo("Illegal name, please try another.\n\r");
    return;
  }
  for (tmp = recipient; *tmp; tmp++)
    if (isupper(*tmp))
      *tmp = tolower(*tmp);

  if (strcmp(recipient, "faction") && !load_char(recipient, &st)) {
    sendTo("No such player to mail to!\n\r");
    return;
  }
  imm = isImmortal();

  for (i = 0;!imm && i < 8; i++)
    if (st.level[i] > MAX_MORT)
      imm = TRUE;

  // let anybody mail to immortals
  if (GetMaxLevel() < MIN_MAIL_LEVEL && !imm) {
    sprintf(buf, "%s Sorry, you have to be level %d to send mail!",
            getName(), MIN_MAIL_LEVEL);
    me->doTell(buf);
    return;
  }

  if(!strcmp(recipient, "faction")){
    if(getFaction() == FACT_NONE){
      sprintf(buf, "%s You aren't in a faction!", fname(name).c_str());
      me->doTell(buf);
      return;
    }

    if(getMoney() < FACTION_STAMP_PRICE && !imm){
      sprintf(buf, "%s Bulk mailing costs %d talens.", fname(name).c_str(), FACTION_STAMP_PRICE);
      me->doTell(buf);
      sprintf(buf, "%s ...which I see you can't afford.", fname(name).c_str());
      me->doTell(buf);
      return;
    }
  } else {
    if (getMoney() < STAMP_PRICE && !imm) {
      sprintf(buf, "%s A stamp costs %d talens.", fname(name).c_str(), STAMP_PRICE);
      me->doTell(buf);
      sprintf(buf, "%s ...which I see you can't afford.", fname(name).c_str());
      me->doTell(buf);
      return;
    }
  }

  act("$n starts to write some mail.", TRUE, this, 0, 0, TO_ROOM);
  if (!imm) {
    sprintf(buf, "%s I'll take %d talens for the stamp.", fname(name).c_str(), 
         strcmp(recipient, "faction")?STAMP_PRICE:FACTION_STAMP_PRICE);
    me->doTell(buf);
    addToMoney(-(strcmp(recipient, "faction")?STAMP_PRICE:FACTION_STAMP_PRICE), GOLD_HOSPITAL);
  } else if (isImmortal()) {
    sprintf(buf, "%s Since you're high and mighty, I'll waive the fee.",
         fname(name).c_str());
    me->doTell(buf);
  } else {
    sprintf(buf, "%s Since you're mailing an immortal, I'll waive the fee.",
         fname(name).c_str());
    me->doTell(buf);
  }
  if (!desc->m_bIsClient) {
    sprintf(buf, "%s Write your message, use ~ when done, or ` to cancel.", fname(name).c_str());
    me->doTell(buf);
    addPlayerAction(PLR_MAILING);
    desc->connected = CON_WRITING;
    strcpy(desc->name, recipient);

    desc->str = new (char *);
    *desc->str = new char[1];
    *(*desc->str) = '\0';
    desc->max_str = MAX_MAIL_SIZE;
  }
  if (desc->m_bIsClient)
    desc->clientf("%d|%s", CLIENT_MAIL, recipient);
}


void TBeing::postmasterCheckMail(TMonster *me)
{
  char buf[200], recipient[100], *tmp;

  _parse_name(getName(), recipient);

// added this check - bat
  if (!mail_ok(this))
    return;

  for (tmp = recipient; *tmp; tmp++)
    if (isupper(*tmp))
      *tmp = tolower(*tmp);

  if (has_mail(recipient))
    sprintf(buf, "%s You have mail waiting.", getName());
  else
    sprintf(buf, "%s Sorry, you don't have any mail waiting.", getName());

  me->doTell(buf);
}

void TBeing::postmasterReceiveMail(TMonster *me)
{
  char buf[200], recipient[100], *tmp;
  TObj *note, *envelope;
  sstring msg;
  sstring from;

  _parse_name(getName(), recipient);

  // added this check - bat
  if (!mail_ok(this))
    return;

  for (tmp = recipient; *tmp; tmp++)
    if (isupper(*tmp))
      *tmp = tolower(*tmp);

  if (!has_mail(recipient)) {
    sprintf(buf, "%s Sorry, you don't have any mail waiting.", fname(name).c_str());
    me->doTell(buf);
    return;
  }
  while (has_mail(recipient)) {
// builder port uses stripped down database which was causing problems
// hence this setup instead.
    int robj = real_object(GENERIC_NOTE);
    if (robj < 0 || robj >= (signed int) obj_index.size()) {
      vlogf(LOG_BUG, "postmasterReceiveMail(): No object (%d) in database!", 
            GENERIC_NOTE);
      return;
    }

    if (!(note = read_object(robj, REAL))) {
      vlogf(LOG_BUG, "Couldn't make a note for mail!");
      return;
    }

    note->swapToStrung();
    delete [] note->name;
    note->name = mud_str_dup("letter mail");
    delete [] note->shortDescr;
    note->shortDescr = mud_str_dup("<o>a handwritten <W>letter<1>"); 
    delete [] note->getDescr();
    note->setDescr(mud_str_dup("A wrinkled <W>letter<1> lies here."));
    delete [] note->action_description;
    msg = read_delete(recipient, getName(), from);
    note->action_description = new char[msg.length()+1];
    strcpy(note->action_description, msg.c_str());
    if (!note->action_description)
      note->action_description = mud_str_dup("Mail system buggy, please report!!  Error #8.\n\r");


    if (!(envelope = read_object(124, VIRTUAL))) {
      vlogf(LOG_BUG, "Couldn't load object 124!");
      return;
    }
    
    *envelope += *note;
    *this += *envelope;

    sprintf(buf, "$n gives you $p from %s.", from.c_str());
    act(buf, FALSE, me, envelope, this, TO_VICT);
    act("$N gives $n $p.", FALSE, this, envelope, me, TO_ROOM);
  }
}

void autoMail(TBeing *ch, const char *targ, const char *msg)
{
  // from field limited to 15 chars by mail structure

  if (ch)
    store_mail(ch->getName(), SNEEZY_ADMIN, msg);
  else if (targ)
    store_mail(targ, SNEEZY_ADMIN, msg);
  else
    vlogf(LOG_BUG, "Error in autoMail");

  return;
}

