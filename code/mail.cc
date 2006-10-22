#include "stdsneezy.h"
#include "mail.h"
#include "database.h"
#include "shop.h"
#include "shopowned.h"

// may not exceed NAME_SIZE (15) chars
static const char * const SNEEZY_ADMIN = "SneezyMUD Administration";


bool has_mail(const char *recipient)
{
  TDatabase db(DB_SNEEZY);

  db.query("select count(*) as count from mail where lower(mailto)=lower('%s')", recipient);

  if(db.fetchRow() && convertTo<int>(db["count"]) != 0)
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
      db.query("insert into mail (port, mailfrom, mailto, timesent, content) values (%i, '%s', '%s', '%s', '%s')", gamePort, from, fm["name"].c_str(), tmstr, message_pointer);
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

  from=db["mailfrom"];

  buf=fmt("The letter has a date stamped in the corner: %s\n\r\n\r%s,\n\r%s\n\rSigned, %s\n\r\n\r") %
    db["timesent"] % recipient_formatted % db["content"] % db["mailfrom"];

  db.query("delete from mail where mailid=%s", db["mailid"].c_str());
  
  return sstring(buf);
}

int postmaster(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  if (!ch->desc)
    return FALSE;               /* so mobs don't get caught here */

  switch (cmd) {
    case CMD_WHISPER:
      return shopWhisper(ch, myself, find_shop_nr(myself->number), arg);    
    case CMD_MAIL: 
      ch->postmasterSendMail(arg, myself);
      return TRUE;
    case CMD_CHECK: 
      ch->postmasterCheckMail(myself);
      return TRUE;
    case CMD_RECEIVE:
      ch->postmasterReceiveMail(myself);
      return TRUE;
    default:
      return FALSE;
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
  char recipient[100], *tmp;
  charFile st;
  int i, imm = FALSE, amt, shop_nr=find_shop_nr(me->number);

// added this check - bat
  if (!mail_ok(this))
    return;

  if (!*arg) {
    me->doTell(getName(), "You need to specify an addressee!");
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
    me->doTell(getName(), fmt("Sorry, you have to be level %d to send mail!") % MIN_MAIL_LEVEL);
    return;
  }

  if(!strcmp(recipient, "faction")){
    if(getFaction() == FACT_NONE){
      me->doTell(fname(name), "You aren't in a faction!");
      return;
    }

    amt = (int)((float)FACTION_STAMP_PRICE * shop_index[shop_nr].getProfitBuy(NULL, this));

    if(getMoney() < amt && !imm){
      me->doTell(fname(name), fmt("Bulk mailing costs %d talens.") % amt);
      me->doTell(fname(name), "...which I see you can't afford.");
      return;
    }
  } else {
    amt = (int)((float)STAMP_PRICE * shop_index[shop_nr].getProfitBuy(NULL, this));

    if (getMoney() < amt && !imm) {
      me->doTell(fname(name), fmt("A stamp costs %d talens.") % amt);
      me->doTell(fname(name), "...which I see you can't afford.");
      return;
    }
  }

  act("$n starts to write some mail.", TRUE, this, 0, 0, TO_ROOM);
  if (!imm) {
    me->doTell(fname(name), fmt("I'll take %d talens for the stamp.") % amt);
    TShopOwned tso(shop_nr, me, this);
    tso.doBuyTransaction(amt, fmt("mailing %s") % recipient, TX_BUYING_SERVICE);
  } else if (isImmortal()) {
    me->doTell(fname(name), "Since you're high and mighty, I'll waive the fee.");
  } else {
    me->doTell(fname(name), "Since you're mailing an immortal, I'll waive the fee.");
  }
  if (!desc->m_bIsClient) {
    me->doTell(fname(name), "Write your message, use ~ when done, or ` to cancel.");
    addPlayerAction(PLR_MAILING);
    desc->connected = CON_WRITING;
    strcpy(desc->name, recipient);

    desc->str = new const char *('\0');
    desc->max_str = MAX_MAIL_SIZE;
  } else
    desc->clientf(fmt("%d|%s") % CLIENT_MAIL % recipient);
}


void TBeing::postmasterCheckMail(TMonster *me)
{
  char recipient[100], *tmp;

  _parse_name(getName(), recipient);

// added this check - bat
  if (!mail_ok(this))
    return;

  for (tmp = recipient; *tmp; tmp++)
    if (isupper(*tmp))
      *tmp = tolower(*tmp);

  if (has_mail(recipient))
    me->doTell(getName(), "You have mail waiting.");
  else
    me->doTell(getName(), "Sorry, you don't have any mail waiting.");

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
    me->doTell(fname(name), "Sorry, you don't have any mail waiting.");
    return;
  }
  while (has_mail(recipient)) {
// builder port uses stripped down database which was causing problems
// hence this setup instead.
    int robj = real_object(GENERIC_NOTE);
    if (robj < 0 || robj >= (signed int) obj_index.size()) {
      vlogf(LOG_BUG, fmt("postmasterReceiveMail(): No object (%d) in database!") %  
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
    note->action_description = mud_str_dup(msg.c_str());
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

