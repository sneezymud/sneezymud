#include "handler.h"
#include "being.h"
#include "client.h"
#include "low.h"
#include "charfile.h"
#include "extern.h"
#include "mail.h"
#include "database.h"
#include "shop.h"
#include "monster.h"
#include "shopowned.h"
#include "rent.h"
#include "obj_trap.h"
#include "obj_open_container.h"
#include "spec_mobs.h"
#include "combat.h"

const int MIN_MAIL_LEVEL = 2;
const int STAMP_PRICE = 50;
const float FACTION_STAMP_PRICE = 5000;

static const char * const SNEEZY_ADMIN = "SneezyMUD Administration";


bool TObj::canBeMailed(sstring name) const
{
  bool canMail = !isObjStat(ITEM_ATTACHED) && !isObjStat(ITEM_NORENT) &&
          !isObjStat(ITEM_BURNING) && !isObjStat(ITEM_PROTOTYPE) &&
          !isObjStat(ITEM_NOPURGE) && !isObjStat(ITEM_NEWBIE) &&
          !isObjStat(ITEM_NODROP) && stuff.empty() &&
          !dynamic_cast<const TTrap*>(this) &&
          (!dynamic_cast<const TOpenContainer*>(this) ||
          !dynamic_cast<const TOpenContainer*>(this)->isContainerFlag(CONT_TRAPPED));

  if (canMail && !name.empty()) {
    sstring owner = monogramOwner();
    return owner.empty() || owner.lower() == name.lower();
  }

  return canMail;
}


bool has_mail(const sstring recipient)
{
  TDatabase db(DB_SNEEZY);
  db.query("select 1 from mail where port=%i and lower(mailto)=lower('%s')", gamePort, recipient);
  return db.isResults();
}


void store_mail(const sstring &to, const sstring &from, const sstring &message, int talens, int rent_id)
{
  TDatabase db(DB_SNEEZY);
  time_t mail_time = time(0);
  char *timestamp = asctime(localtime(&mail_time));
  timestamp[strlen(timestamp) - 1] = '\0';

  if (to == "faction") {
    TDatabase members(DB_SNEEZY);
    members.query("select name from factionmembers where faction=(select faction from factionmembers where name='%s')", from);
    
    while (members.fetchRow()) {
      db.query("insert into mail (port, mailfrom, mailto, timesent, content, talens, rent_id)"
               "values (%i, '%s', '%s', '%s', '%s', 0, 0)",
               gamePort, from, members["name"], timestamp, message);
    }
  } else {
    db.query("insert into mail (port, mailfrom, mailto, timesent, content, talens, rent_id)"
             "values (%i, '%s', '%s', '%s', '%s', %i, %i)",
             gamePort, from, to, timestamp, message, talens, rent_id);
  }
}


sstring read_delete(const sstring &recipient, const sstring &recipient_formatted, sstring &from, int &talens, int &rent_id)
{
  TDatabase db(DB_SNEEZY);
  sstring body;

  db.query("select mailfrom, timesent, content, mailid, talens, rent_id from mail where port=%i and lower(mailto)=lower('%s')", gamePort, recipient.c_str());
  if(!db.fetchRow())
    return "error!";

  from = db["mailfrom"];
  talens = convertTo<int>(db["talens"]);
  rent_id = convertTo<int>(db["rent_id"]);

  body = format("The letter has a date stamped in the corner: %s\n\r\n\r%s,\n\r%s\n\rSigned, %s\n\r\n\r")
      % db["timesent"] % recipient_formatted % db["content"] % db["mailfrom"];

  db.query("delete from mail where mailid=%s", db["mailid"]);
  
  return body;
}


void postmasterValue(TBeing *ch, TBeing *postmaster, sstring args)
{
  int shop_nr = find_shop_nr(postmaster->number);
  float profit_buy = shop_index[shop_nr].getProfitBuy(NULL, ch);

  sstring item, talen;
  args = one_argument(args, item);
  args = one_argument(args, talen);

  if (is_abbrev(item, "talens") || is_abbrev(talen, "talens")) {
    postmaster->doTell(fname(ch->name), format("When sending money, I charge %d talens in handling fees.") % int((float)STAMP_PRICE * profit_buy * 2));
    return;
  }

  if (item == "faction") {
    postmaster->doTell(fname(ch->name), format("Bulk faction mail from me will cost about %d talens.") % int((float)FACTION_STAMP_PRICE * profit_buy));
    return;
  }

  if (item.empty()) {
    postmaster->doTell(fname(ch->name), format("My price for a regular stamp is %d talens.") % int((float)STAMP_PRICE * profit_buy));
    return;
  }

  TThing *thing = get_thing_on_list_vis(ch, item.c_str(), ch->stuff.front());
  TObj *obj = thing ? dynamic_cast<TObj*>(thing) : NULL;
  if (!obj) {
    postmaster->doTell(fname(ch->name), "I don't see that item on you.");
    return;
  }
  int cost = int((float)STAMP_PRICE * profit_buy * (obj->getWeight() + 3));
  if (obj->isMonogrammed()) {
    postmaster->doTell(fname(ch->name), "This item appears to be monogrammed.  You may only mail it to its owner.");
    cost = STAMP_PRICE;
  }
  if (!obj->canBeMailed("")) {
    postmaster->doTell(fname(ch->name), "Sorry, I can't ship that.");
    return;
  }
  postmaster->doTell(fname(ch->name), format("Shipping %s will cost you %d talens.") % obj->getName() % cost);
}


int postmasterGiven(TBeing *ch, TMonster *me, TObj *o)
{
  if (!o || o->objVnum() != Obj::GENERIC_L_TOKEN) {
    me->doTell(ch->getName(), "What in the hells is this?!");
    me->doGive(ch, o);
    return 0;
  }

  // the following code extracts out properties from the name
  sstring nameLower = sstring(ch->getName()).lower();
  sstring objName = o->name;
  sstring playerTag = "[link$" + nameLower + "$";
  size_t startTag = objName.find(playerTag);
  if (startTag == sstring::npos) {
    me->doTell(ch->getName(), "Are you sure this is your token? It doesn't look right...");
    me->doGive(ch, o);
    return 0;
  }
  size_t dueTag = objName.find("$", startTag+playerTag.length());
  if (dueTag == sstring::npos)
    return 0;
  size_t endTag = objName.find("]", dueTag+1);
  if (endTag == sstring::npos)
    return 0;
  
  // raw data
  sstring fullToken = objName.substr(startTag, endTag-startTag+1); // includes [] brackets
  sstring deathDate = objName.substr(startTag+playerTag.length(), dueTag-startTag-playerTag.length());
  sstring dueDate = objName.substr(dueTag+1, endTag-dueTag-1);
  time_t dueTime = convertTo<time_t>(dueDate.c_str());

  vlogf(LOG_BUG, fullToken);
  vlogf(LOG_BUG, deathDate);
  vlogf(LOG_BUG, dueDate);

  // dueDate is uninitialized
  if (dueTime == 0) {
    time_t ct = time(0);
    ct += 10*60; // add 10 minutes

    dueDate = format("%i") % int(ct);
    objName.replace(dueTag+1, endTag-dueTag-1, dueDate.c_str(), dueDate.length());
    o->name = objName;

    me->doTell(ch->getName(), "Your belongings are arriving via magical transport in ten minutes.");
    me->doTell(ch->getName(), "In the meantime, hold on to this token and give it to me when they arrive.");
    me->doGive(ch, o);
    ch->doQueueSave();

    return 0;

  // duedate hasnt expired yet
  } else if (dueTime > time(0)) {

    time_t ct = dueTime - time(0);
    int minutes = ct / 60;
    int seconds = ct % 60;

    me->doTell(ch->getName(), "Sorry, your belongings haven't arrived yet.");
    me->doTell(ch->getName(), format("It looks like they won't be ready for another %i minutes and %i seconds.") % minutes % seconds);
    me->doGive(ch, o);

    return 0;

  // item is ready
  } else {

    TRoom *rp = real_roomp(Room::STORAGE);
    if (!rp)
      return 0;
    fullToken.inlineReplaceString(dueDate, "bag");
    vlogf(LOG_MISC, "Linkbag retrieval: looking for " + fullToken);
    TObj *linkbag = nullptr;
    for(StuffIter it = rp->stuff.begin(); it != rp->stuff.end() && *it; it++) {
      TObj *stored = dynamic_cast<TObj*>(*it);
      if (!stored)
        continue;
      vlogf(LOG_MISC, "Scanning " + sstring(stored->name)); 
      if (sstring(stored->name).find(fullToken) == sstring::npos)
        continue;
      linkbag = stored;
    }
    if (!linkbag) {
      vlogf(LOG_BUG, format("No linkbag found for %s!") % ch->getName());
      me->doTell(ch->getName(), "Well, that's strange!  You don't have a linkbag.");
      act("$n shrugs helplessly.", TRUE, me, 0, 0, TO_ROOM);
      me->doGive(ch, o); 
      ch->doQueueSave();
      return 0;
    }

    --(*linkbag);
    *(ch->roomp) += *linkbag;
    linkbag->addObjStat(ITEM_NORENT | ITEM_NEWBIE);
    linkbag->obj_flags.decay_time = MAX_PC_CORPSE_EQUIPPED_TIME;
    linkbag->obj_flags.wear_flags &= ~ITEM_TAKE;

    me->doTell(ch->getName(), "Great!  It looks like your things are ready.");
    me->doTell(ch->getName(), "I'll just drop them here in the room.  Please take out your items and clean up.");
    ch->doQueueSave();

    return DELETE_ITEM;
  }

  return 0;
}


int postmaster(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
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
    case CMD_VALUE:
      postmasterValue(ch, myself, arg);
      return TRUE;
    case CMD_MOB_GIVEN_ITEM:
      return postmasterGiven(ch, myself, o);
    default:
      return FALSE;
  }
}


void TBeing::postmasterSendMail(sstring args, TMonster *me)
{
  sstring item, recipient, talen;
  charFile st;
  bool sendFaction;
  int i, amt, shop_nr = find_shop_nr(me->number);
  float profit_buy = 0;

  if (args.empty()) {
    me->doTell(getName(), "You need to specify an addressee!");
    return;
  }
  
  args = one_argument(args, recipient);
  args = one_argument(args, item);
  args = one_argument(args, talen);

  if (parse_name_sstring(recipient, recipient)) {
    sendTo("That is not a valid recipient name.\n\r");
    return;
  }

  recipient = recipient.lower();

  if (recipient == "faction") {
    sendFaction = true;
  } else if (!load_char(recipient, &st)) {
    me->doTell(getName(), "Sorry, I don't have an address on file for that person!");
    return;
  }

  bool imm = isImmortal();
  for (i = 0;!imm && i < 8; i++)
    if (st.level[i] > MAX_MORT)
      imm = TRUE;

  if (GetMaxLevel() < MIN_MAIL_LEVEL && !imm) {
    me->doTell(getName(), format("Sorry, you have to be level %d to send mail!") % MIN_MAIL_LEVEL);
    return;
  }

  profit_buy = shop_index[shop_nr].getProfitBuy(NULL, this);

  if (sendFaction) {
    if (getFaction() == FACT_NONE) {
      me->doTell(getName(), "You aren't in a faction!");
      return;
    }

    amt = (int)((float)FACTION_STAMP_PRICE * profit_buy);
    if (getMoney() < amt && !imm) {
      me->doTell(getName(), format("Bulk mailing costs %d talens.") % amt);
      me->doTell(getName(), "...which I see you can't afford.");
      return;
    }

  // sending talens
  } else if (is_abbrev(talen, sstring("talens"))) {

    amt = (int)((float)STAMP_PRICE * profit_buy * 2);
    int talen_amt = convertTo<int>(item);

    if (talen_amt < 1) {
      me->doTell(getName(), "What?! That's not a monetary amount.");
      return;
    }
    if (talen_amt + amt > getMoney()) {
      me->doTell(getName(), format("Mailing %d talens plus a stamp costs a total of %d talens.") % talen_amt % (talen_amt + amt));
      me->doTell(getName(), "...which I see you can't afford.");
      return;
    }
    desc->mail_talens = max(0, talen_amt);

  // sending item
  } else if (item.length() > 0) {

    TThing *thing = searchLinkedListVis(this, item.c_str(), stuff, NULL, TYPEOBJ);
    TObj *obj = thing ? dynamic_cast<TObj*>(thing) : NULL;
    if (obj == NULL) {
      me->doTell(getName(), "I don't see that item on you.");
      return;
    }

    // check for item flags that will stop the deal
    if (!obj->canBeMailed(recipient)) {
      me->doTell(getName(), "You can't mail that item!");
      return;
    }

    // calculate mailing price (monogrammed items are free+letter to mail to owner)
    amt = (int)((float)STAMP_PRICE * profit_buy * (obj->getWeight() + 3));
    if (obj->isMonogrammed())
      amt = STAMP_PRICE;

    // verify they have the money
    if (amt > getMoney() && !imm) {
      me->doTell(getName(), format("Mailing this item plus a stamp costs a total of %d talens.") % amt);
      me->doTell(getName(), "...which I see you can't afford.");
      return;
    }

    desc->obj = obj;

  } else {
    amt = (int)((float)STAMP_PRICE * profit_buy);

    if (getMoney() < amt && !imm) {
      me->doTell(getName(), format("A stamp costs %d talens.") % amt);
      me->doTell(getName(), "...which I see you can't afford.");
      return;
    }
  }

  act("$n starts to write some mail.", TRUE, this, 0, 0, TO_ROOM);
  if (!imm) {
    me->doTell(getName(), format("I'll take %d talens for the stamp.") % amt);
    TShopOwned tso(shop_nr, me, this);
    tso.doBuyTransaction(amt, format("mailing %s") % recipient, TX_BUYING_SERVICE);
  } else if (isImmortal()) {
    me->doTell(getName(), "Since you're high and mighty, I'll waive the fee.");
  } else {
    me->doTell(getName(), "Since you're mailing an immortal, I'll waive the fee.");
  }

  if (desc->m_bIsClient) {
    desc->clientf(format("%d|%s") % CLIENT_MAIL % recipient);
    return;
  }

  addPlayerAction(PLR_MAILING);
  desc->connected = CON_WRITING;
  strncpy(desc->name, recipient.c_str(), cElements(desc->name));
  desc->edit_str = &desc->mail_edit_str;
  desc->edit_str_maxlen = MAX_MAIL_SIZE;

  me->doTell(getName(), "Write your message, use ~ when done, or ` to cancel.");
}


void TBeing::postmasterCheckMail(TMonster *me)
{
  sstring recipient;
  parse_name_sstring(getName(), recipient);

  if (has_mail(recipient))
    me->doTell(getName(), "You have mail waiting.");
  else
    me->doTell(getName(), "Sorry, you don't have any mail waiting.");
}


void TBeing::postmasterReceiveMail(TMonster *me)
{
  sstring recipient;
  parse_name_sstring(getName(), recipient);

  if (!has_mail(recipient)) {
    me->doTell(getName(), "Sorry, you don't have any mail waiting.");
    return;
  }

  TObj *note, *envelope;

  while (has_mail(recipient)) {
    // builder port uses stripped down database which was causing problems
    // hence this setup instead.
    int robj = real_object(Obj::GENERIC_NOTE);
    if (robj < 0 || robj >= (signed int) obj_index.size()) {
      vlogf(LOG_BUG, format("postmasterReceiveMail(): No object (%d) in database!") %  
            Obj::GENERIC_NOTE);
      return;
    }

    if (!(note = read_object(robj, REAL))) {
      vlogf(LOG_BUG, "Couldn't make a note for mail!");
      return;
    }

    int talens = 0;
    int rent_id = 0;
    int env_vnum = 124;
    sstring from;
    sstring msg = read_delete(recipient, getName(), from, talens, rent_id);

    note->swapToStrung();
    note->name = "letter mail";
    note->shortDescr = "<o>a handwritten <W>letter<1>"; 
    note->setDescr("A wrinkled <W>letter<1> lies here.");
    note->action_description = msg;

    if (talens >= 1000 || rent_id > 0)
      env_vnum = 6600; // crate

    int env_robj = real_object(env_vnum);
    if (env_robj < 0 || env_robj >= (signed int) obj_index.size() ||
        !(envelope = read_object(env_robj, REAL))) {
      vlogf(LOG_BUG, "Couldn't load envelope object!");
      return;
    }
    
    envelope->addObjStat(ITEM_NEWBIE);
    envelope->addObjStat(ITEM_NORENT);

    *envelope += *note;

    if (talens > 0) {
      vlogf(LOG_OBJ, format("Mail: %s receiving %i talens from %s") %
          getName() % talens % from);
      *envelope += *create_money(talens);
    }

    if (rent_id > 0) {
      TObj *obj = NULL;
      int slot = -1;
      ItemLoadDB il("mail", GH_MAIL_SHOP);
      TDatabase db(DB_SNEEZY);

      obj = il.raw_read_item(rent_id, slot);
      db.query("delete from rent where rent_id=%i", rent_id);
      db.query("delete from rent_obj_aff where rent_id=%i", rent_id);
      db.query("delete from rent_strung where rent_id=%i", rent_id);

      if (obj) {
        vlogf(LOG_OBJ, format("Mail: retrieved object %s from rent_id:%i from mail for %s from %s") %
          obj->getName() % rent_id % getName() % from);
        *envelope += *obj;
      } else {
        vlogf(LOG_BUG, format("Mail: error retrieving rent_id:%i from mail for %s from %s") %
          rent_id % getName() % from);
      }
    }

    *this += *envelope;

    act(format("$n gives you $p from %s.") % from, FALSE, me, envelope, this, TO_VICT);
    act("$N gives $n $p.", FALSE, this, envelope, me, TO_ROOM);
  }
}


void autoMail(TBeing *ch, const char *targ, const char *msg, int m, int r)
{
  if (ch)
    store_mail(ch->getName(), SNEEZY_ADMIN, msg, m, r);
  else if (targ)
    store_mail(targ, SNEEZY_ADMIN, msg, m, r);
  else
    vlogf(LOG_BUG, "No recipient given for autoMail!");
}

