#include <unistd.h>
#include <sys/stat.h>

#include "stdsneezy.h"
#include "statistics.h"
#include "obj_note.h"
int counter_done;  // Global variable used to count # of done items/repairman 
int counter_work;  // Global variable used to count # of undone items/man 

static int global_repair = 0;

int TObj::maxFix(const TBeing *repair, depreciationTypeT dep_done) const
{
  int amount = getMaxStructPoints() - getDepreciation();
#if 0
  if (!dep_done)
    amount--;   // fudge depreciation for "value"
#endif

  if (repair) {
    amount *= max(95, (int) repair->GetMaxLevel());
    amount /= 100;
  }

  return amount;
}

int TObj::repairPrice(const TBeing *repair, const TBeing *buyer, depreciationTypeT dep_done) const
{
  // dep_done will be true if depreciation accounted for
  // since value doesn't dep o, we need to fudge it

  // makes an assumption that struct is > 0
  // pre assumes that it needs repairing.
  int discount = 100;
  int gsp = getShopPrice(&discount);

  // ideally, this price will be < gsp, but gold_mod should handle that for us
  int price = (int) (gsp * gold_modifier[GOLD_REPAIR].getVal());

  price *= maxFix(repair, dep_done) - getStructPoints();
  price /= getMaxStructPoints();

#if FACTIONS_IN_USE
  if (!repair->isUnaff() && repair->isSameFaction(buyer)) {
    price *= (200 - (int) buyer->getPerc());
    price /= 200;
  }
#endif

  // arbitrary maximum
  price = min(price, 5 * gsp / 2);

  price = (price * 75) / 100;

  return (price);
}

// time it will take to repair an item in seconds 
static int repair_time(const TObj *o)
{
  int structs;
  double percDam;
  double iTime;
  int MINS_AT_60TH = 60; // maximum (full repair) for 60th level eq

  if (!(structs = (o->getMaxStructPoints() - o->getStructPoints())))
    return (0);
  percDam = ((double)(structs*75) / (double)(o->getMaxStructPoints())) + 25.0;
  double levmod = (double)(o->objLevel() * o->objLevel());
#if 1
  iTime = levmod * (percDam);
  iTime /= 100.0*60.0; // correct for the % thing
  iTime *= (double)MINS_AT_60TH;
  // max repair time * % damage to object
#endif

  // i took the following out, it kind of sucks. 
#if 0
  // price is based on struct (armor/weapons) so no real need to
  // take both into account
  // let's just make time based on level of the gear
  iTime = (int) (o->objLevel() * 1);
  // #else
  // Let's make the time be proportional to cost, and total damage - Russ 
  iTime = ((structs * 800) + (2 * o->obj_flags.cost));

  // Check to see how many items the repairman has, and 
  // adjust time accordingly - Russ                     
  //  iTime += wait_for_items_already_here() 

  iTime *= max(11, 10 + o->obj_flags.cost/10000);
  iTime /= 10;
 
  iTime /= 500;  // Kludge since it is taking too long
#endif
  iTime = max(1.0,iTime);

  return (int)(iTime);
}

static void save_repairman_file(TBeing *repair, TBeing *buyer, TObj *o, int iTime, int)
{
  char buf[256], buf2[256];
  FILE *fp;
  long then;
  int cost;
  unsigned char version;

  if (!repair || !buyer) {
    vlogf(LOG_BUG, "save_repairman_file() called with NULL ch!");
    return;
  }
  if (!buyer->isPc()) {
    vlogf(LOG_BUG, "Non-PC got into save_repairman_file() somehow!!! BUG BRUTIUS!!", buyer->getName());
    return;
  }
  then = (long) iTime;
  cost = o->repairPrice(repair, buyer, DEPRECIATION_YES);

  sprintf(buf, "mobdata/repairs/%d/%d", repair->mobVnum(), repair_number);
  if (!(fp = fopen(buf, "w"))) {
    sprintf(buf2, "mobdata/repairs/%d", repair->mobVnum());
    if (mkdir(buf2, 0770)) {
      vlogf(LOG_BUG, "Unable to create a repair directory for %s.", repair->getName());
      return;
    } else
      vlogf(LOG_BUG, "Created a repair directory for %s", repair->getName());

    if (!(fp = fopen(buf, "w"))) {
      vlogf(LOG_BUG, "Major problems trying to save %s repair file.", repair->getName());
      return;
    }
  }
  if (fwrite(&then, sizeof(then), 1, fp) != 1) {
    vlogf(LOG_BUG, "Error writing time for repairman_file!");
    fclose(fp);
    return;
  }
  if (fwrite(&cost, sizeof(cost), 1, fp) != 1) {
    vlogf(LOG_BUG, "Error writing cost for repairman_file!");
    fclose(fp);
    return;
  }
  version = CURRENT_RENT_VERSION;
  if (fwrite(&version, sizeof(version), 1, fp) != 1) {
    vlogf(LOG_BUG, "Error writing version for repairman_file!");
    fclose(fp);
    return;
  }

  raw_write_item(fp, o, version);
  fclose(fp);

  // Save the repair number so we can keep up with it.
  save_game_stats();
}

static int check_time_and_gold(TBeing *repair, TBeing *buyer, int ticket, TNote *obj)
{
  FILE *fp;
  char buf[80], buf2[256];
  long tmp, cur_time;
  int tmp_cost, diff, hours, minutes, seconds;
  TObj *fixed_obj;
  unsigned char version;

  if (!repair || !buyer) {
    vlogf(LOG_BUG, "check_time called with NULL character! BUG BRUTIUS!");
    return FALSE;
  }
  sprintf(buf, "mobdata/repairs/%d/%d", repair->mobVnum(), ticket);
  if (!(fp = fopen(buf, "r"))) {
    sprintf(buf2, "I don't seem to have an item for ticket number %d", ticket);
    repair->doSay(buf2);
    return FALSE;
  }
  if (fread(&tmp, sizeof(tmp), 1, fp) != 1) {
    vlogf(LOG_BUG, "No timer on item number %d for repairman %s", ticket, repair->getName());
    repair->doSay("Something is majorly wrong(Timer). Talk to a god");
    fclose(fp);
    return FALSE;
  }
  if (fread(&tmp_cost, sizeof(tmp_cost), 1, fp) != 1) {
    vlogf(LOG_BUG, "No cost on item number %d for repairman %s", ticket, repair->getName());
    repair->doSay("Something is majorly wrong(Cost). Talk to a god");
    fclose(fp);
    return FALSE;
  }
  if (fread(&version, sizeof(version), 1, fp) != 1) {
    vlogf(LOG_BUG, "No version on item number %d for repairman %s", ticket, repair->getName());
    repair->doSay("Something is majorly wrong(version). Talk to a god");
    fclose(fp);
    return FALSE;
  }
  cur_time = time(0) + 3600 * obj->getTimeAdj();
  if (cur_time < tmp) {
    diff = tmp - cur_time;
    hours = diff/3600;
    minutes = (diff % 3600)/60;
    seconds = diff % 60;

    sprintf(buf, "%s Your item isn't ready yet.", fname(buyer->name).c_str());
    repair->doTell(buf);
    sprintf(buf, "%s It will be ready in %d hours, %d minutes and %d seconds.", fname(buyer->getName()).c_str(), hours, minutes, seconds);
    repair->doTell(buf);
    fclose(fp);
    return FALSE;
  } else if (tmp_cost > buyer->getMoney()) {
    sprintf(buf, "%s I don't repair items for free! No money, no item!", buyer->getName());
    repair->doSay(buf);
    sprintf(buf, "Remember the price is %d.", tmp_cost);
    repair->doSay(buf);
    fclose(fp);
    return FALSE;
  } else {
    if ((fixed_obj = raw_read_item(fp, version))) {
      obj_index[fixed_obj->getItemIndex()].addToNumber(-1);
      sprintf(buf2, "%s Ah yes, %s, here is %s.",
           fname(buyer->name).c_str(), buyer->getName(),fixed_obj->shortDescr);
      repair->doTell(buf2);
      repair->doSay("Thank you for your business!");
      fixed_obj->setStructPoints(fixed_obj->maxFix(repair, DEPRECIATION_YES));
      buyer->addToMoney(-tmp_cost, GOLD_REPAIR);
      // I think this is just for gold stats, the repair guy doesn't use it
      repair->addToMoney(tmp_cost, GOLD_REPAIR); 

      *buyer += *fixed_obj;
      fclose(fp);
      sprintf(buf, "mobdata/repairs/%d/%d", repair->mobVnum(), ticket);
      unlink(buf);
      buyer->doSave(SILENT_YES);
      buyer->logItem(fixed_obj, CMD_NORTH);   // cmd indicates repair-retrieval

      // acknowledge the depreciation after all work is done
      // this way the price doesn't change during the process
      // and also makes the first repair "depreciation free"

      fixed_obj->addToDepreciation(1);
      if (fixed_obj->isObjStat(ITEM_CHARRED))
	fixed_obj->addToDepreciation(2); // we want charred objects to deteriorate much faster

      return TRUE;
    } else {
      repair->doSay("Whoa, serious problems, tell a god.");
      vlogf(LOG_BUG, "Bogus load of repair item problem!!!!!!");
      fclose(fp);
      return TRUE;
    }
  }
}

static bool will_not_repair(TBeing *ch, TMonster *repair, TObj *obj, silentTypeT silent)
{
  char buf[160];

  if (obj->objectRepair(ch, repair, silent)) 
    return TRUE;

  if (!obj->isRentable()) {
    if (!silent) {
      sprintf(buf, "%s I'm sorry, but that item is unrepairable.",fname(ch->name).c_str());
      repair->doTell(buf);
    }
    return TRUE;
  }
  if (obj->getStructPoints() == obj->getMaxStructPoints()) {
    if (!silent) {
      sprintf(buf, "%s It doesn't look like that item needs any repairing.",
               fname(ch->name).c_str());
      repair->doTell(buf);
    }
    return TRUE;
  } 
  if (obj->getStructPoints() >= obj->maxFix(NULL, DEPRECIATION_NO)) {
    // check depreciation alone
    if (!silent) {
      sprintf(buf, "%s That item's damage isn't something that can be repaired.",
               fname(ch->name).c_str());
      repair->doTell(buf);
    }
    return TRUE;
  } 
  if (obj->getStructPoints() >= obj->maxFix(repair, DEPRECIATION_NO)) {
    // check repairman's skill
    if (!silent) {
      sprintf(buf, "%s I hate to admit it, but I don't think I have the skill to fix that further.",
             fname(ch->name).c_str());
      repair->doTell(buf);
    }
    return TRUE;
  } 
  if (!repair_time(obj)) {
    // probably superfluous
    if (!silent) {
      sprintf(buf, "%s %s looks fine to me.", fname(ch->name).c_str(), obj->getName());
      repair->doTell(buf);
    }
    return TRUE;
  }
  if (obj->objVnum() == -1) {
    if (!silent) {
      sprintf(buf, "%s I can't take temporary items like %s.", fname(ch->name).c_str(), obj->getName());
      repair->doTell(buf);
    }
    return TRUE;
  }
  if (obj->isObjStat(ITEM_NODROP)) {
    if (!silent) {
      sprintf(buf, "%s I can't take cursed items.", fname(ch->name).c_str());
      repair->doTell(buf);
    }
    return TRUE;
  }
  if (obj->isObjStat(ITEM_BURNING)) {
    if (!silent) {
      sprintf(buf, "%s I can't repair burning items.", fname(ch->name).c_str());
      repair->doTell(buf);
    }
    return TRUE;
  }
  if (obj->isObjStat(ITEM_CHARRED)) {
    if (!silent) {
      sprintf(buf, "%s I can repair this, but it is very badly fire-damaged.", fname(ch->name).c_str());
      repair->doTell(buf);
    }
  }
  if (obj_index[obj->getItemIndex()].number > 
      obj_index[obj->getItemIndex()].max_exist) {
    // item over max-exist, never supposed to happen, but could
    // make it unrepairable to encourage it to scrap
    if (!silent) {
      sprintf(buf, "%s Someone has put out a contract to reclaim that item.  It's just too dangerous for me to take it.", fname(ch->name).c_str());
      repair->doTell(buf);
    }

    return TRUE;
  }

  if (obj->getStuff()) {
    // probably a mage-belt with components in it....
    if (!silent) {
      sprintf(buf, "%s Sorry, you'll have to empty it out before I can do any work on it.", fname(ch->name).c_str());
      repair->doTell(buf);
    }
    return TRUE;
  }
  return FALSE;
}

void repairman_value(const char *arg, TMonster *repair, TBeing *buyer)
{
  char buf[256];
  char *ready;
  TObj *valued;
  time_t when_ready, ct;

  if (buyer->desc && buyer->desc->account)
    ct = time(0) + 3600 * buyer->desc->account->time_adjust;
  else
    ct = time(0);

  for (;*arg && *arg == ' ';arg++);

  if (!*arg) {
    sprintf(buf, "%s Can you be a little more specific about what you want to value....\n\r", fname(buyer->name).c_str());
    repair->doTell(buf);
    return;
  }
  TThing *t_valued = searchLinkedListVis(buyer, arg, buyer->getStuff());
  valued = dynamic_cast<TObj *>(t_valued);
  if (!valued) {
    sprintf(buf, "%s %s, You don't have that item.\n\r", fname(buyer->name).c_str(), buyer->getName());
    repair->doTell(buf);
    return;
  }
  if (will_not_repair(buyer, repair, valued, SILENT_NO))
    return;

  repair->doTell(fname(buyer->name).c_str(),
     "It'll cost you %d talens to repair %s to a status of %s.", 
     (valued->repairPrice(repair, buyer, DEPRECIATION_NO)),
     valued->getName(), 
     valued->equip_condition(valued->maxFix(repair, DEPRECIATION_NO)).c_str());

  when_ready = ct + repair_time(valued);
  ready = asctime(localtime(&when_ready));
  *(ready + strlen(ready) - 9) = '\0';
  repair->doTell(fname(buyer->name).c_str(), "I can have it ready by %s.", ready);
  repair->doTell(fname(buyer->name).c_str(), "That's %s.",  secsToString(when_ready-ct).c_str());
}

// returns DELETE_THIS if buyer goes poof
int repairman_give(const char *arg, TMonster *repair, TBeing *buyer)
{
  char buf[256], obj_name[MAX_INPUT_LENGTH];
  char rep_name[MAX_INPUT_LENGTH];
  char obj_amt[MAX_INPUT_LENGTH];
  TThing *t, *t2;

  arg = one_argument(arg, obj_name);

  if (atoi_safe(obj_name) && (obj_name[1] != '.') && (obj_name[2] != '.')) {
    strcpy(obj_amt, obj_name);
    arg = one_argument(arg, obj_name);
  } else
    *obj_amt = '\0';

  arg = one_argument(arg, rep_name);
  if (*rep_name && !strcmp(rep_name, "to"))
    arg = one_argument(arg, rep_name);

  if (!*obj_name || !*rep_name) {
    buyer->sendTo("Give WHAT to WHOM?!?\n\r");
    return FALSE;
  }
  if ((get_char_room_vis(buyer, rep_name) != repair)) {
    if (*obj_amt)
      sprintf(buf, "%s %s %s", obj_amt, obj_name, rep_name);
    else
      sprintf(buf, "%s %s", obj_name, rep_name);

    if (buyer->doGive(buf) == DELETE_THIS)
      return DELETE_THIS;
    return FALSE;
  }
  if (is_abbrev(obj_name, "all.damaged")) {
    int total = 0;
    bool found = false;
    for (t = buyer->getStuff();t; t = t2) {
      t2 = t->nextThing;
      TObj *tobj = dynamic_cast<TObj *>(t);
      if (!tobj)
        continue;
      if (!will_not_repair(buyer, repair, tobj, SILENT_YES)) {
        // a damaged note would get handed over here, but oh well
        int rc5;
        tobj->giveToRepair(repair, buyer, &rc5);
        if (IS_SET_DELETE(rc5, DELETE_THIS)) {
          found = TRUE;
          total += tobj->repairPrice(repair, buyer, DEPRECIATION_YES);
          delete tobj;
          tobj = NULL;
          buyer->doSave(SILENT_YES);
        }
      }
    }
    if (!found) 
      buyer->sendTo("You have no damaged items in your inventory!\n\r");
    else
      buyer->sendTo("You gave a total of %d talens in damaged equipment.\n\r", total);

    return FALSE;
  }
  if (is_abbrev(obj_name, "all.ticket")) {
    bool found = false;
    for (t = buyer->getStuff();t; t = t2) {
      t2 = t->nextThing;
      if (!isname("ticket", t->name))
        continue;
      int rc5;
      t->giveToRepairNote(repair, buyer, &rc5);
      if (IS_SET_DELETE(rc5, DELETE_THIS)) {
        found = TRUE;
        delete t;
        t= NULL;
      }
    }
    if (!found)
      buyer->sendTo("Your inventory contains no tickets!\n\r");

    return FALSE;
  }
  t = searchLinkedListVis(buyer, obj_name, buyer->getStuff());
  TObj *tobj = dynamic_cast<TObj *>(t);
  if (!tobj) {
    sprintf(buf, "%s You don't have that item.", fname(buyer->name).c_str());
    repair->doTell(buf);
    return FALSE;
  }
  int rc5;
  tobj->giveToRepair(repair, buyer, &rc5);
  if (IS_SET_DELETE(rc5, DELETE_THIS)) {
    delete tobj;
    tobj = NULL;
    buyer->doSave(SILENT_YES);
  }

  return FALSE;
}

static TObj *make_ticket(TMonster *repair, TBeing *buyer, TObj *repaired, time_t when_ready, int tick_num)
{
  TObj *tmp_obj;
  if (!(tmp_obj = read_object(GENERIC_NOTE, VIRTUAL))) {
    vlogf(LOG_BUG, "Couldn't read in note for make_ticket. BUG BRUTIUS!!!");
    return NULL;
  }
  tmp_obj->noteMe(repair, buyer, repaired, when_ready, tick_num);
  tmp_obj->max_exist = repaired->max_exist;
  return tmp_obj;
}

// returns TRUE if item given, otherwise false
// note TRUE = o should be deleted
void TObj::giveToRepair(TMonster *repair, TBeing *buyer, int *found)
{
  extern int repair_number;
  char buf[256];
  time_t when_ready, ct;
  char *ready;
  TObj *ticket;

  *found = FALSE;

  if (buyer->desc && buyer->desc->account)
    ct = time(0) + 3600 * buyer->desc->account->time_adjust;
  else
    ct = time(0);

  if (will_not_repair(buyer, repair, this, SILENT_NO))
    return;

  sprintf(buf, "%s It'll cost you %d talens to repair %s to a status of %s.", 
         fname(buyer->name).c_str(), (repairPrice(repair, buyer, DEPRECIATION_YES)),
         getName(), equip_condition(maxFix(repair, DEPRECIATION_YES)).c_str());
  repair->doTell(buf);
  when_ready = ct + repair_time(this);
  ready = asctime(localtime(&when_ready));
  *(ready + strlen(ready) - 9) = '\0';
  sprintf(buf, "%s It will be ready %s.", fname(buyer->name).c_str(), ready);
  repair->doTell(buf);
  sprintf(buf, "%s That's %s.",  fname(buyer->name).c_str(), secsToString(when_ready-ct).c_str());
  repair->doTell(buf);
  repair_number++;
  sprintf(buf, "%s Payment is due when you pick your item up.", fname(buyer->name).c_str());
  repair->doTell(buf);
  sprintf(buf, "%s Here is your ticket, %s", fname(buyer->name).c_str(), buyer->getName());
  repair->doTell(buf);
  sprintf(buf, "%s If you lose the ticket, it might be hard to reclaim your item.", fname(buyer->name).c_str());
  repair->doTell(buf);
  ticket = make_ticket(repair, buyer, this, when_ready, repair_number);
  *buyer += *ticket;
  save_repairman_file(repair, buyer, this, when_ready, repair_number);
  // vlogf(LOG_DASH, "%s repairing %s - str %d/%d, lev %d.  Repair time: %s.", 
  //fname(buyer->name).c_str(), getName(), (int)getStructPoints(), (int)getMaxStructPoints(),
  //(int)(this->objLevel()*1), secsToString(when_ready-ct).c_str());

  buyer->logItem(this, CMD_REPAIR);

  // we haven't really destroyed the item, repair still keeps
  // track of it.  ~TObj() will decrease number, so arbitrarily increment
  // it prior to deleting
  if (number)
    obj_index[number].addToNumber(1);
  buyer->doSave(SILENT_YES);

  // o should always get deleted following return here

  *found = DELETE_THIS;
  return;
}

void TNote::giveToRepair(TMonster *repair, TBeing *buyer, int *found)
{
  giveToRepairNote(repair, buyer, found);
}

void TNote::giveToRepairNote(TMonster *repair, TBeing *buyer, int *found)
{
  char buf[256];
  int iNumber;

  // found indicates a ticket was found for give all.ticket
  *found = TRUE;

  if (!action_description) {
    sprintf(buf, "%s That ticket is blank!", fname(buyer->name).c_str());
    repair->doTell(buf);
    return;
  }
  if (getRepairman() != mob_index[repair->getMobIndex()].virt) {
    sprintf(buf, "%s That isn't one of my tickets!", fname(buyer->name).c_str());
    repair->doTell(buf);
    return;
  }
  strcpy(buf, getName());
  if (sscanf(buf, "a small ticket marked number %d", &iNumber) != 1) {
    sprintf(buf, "%s That ticket isn't from THIS shop!", fname(buyer->name).c_str());
    repair->doTell(buf);
  } else {
    if (check_time_and_gold(repair, buyer, iNumber, this)) {
      *found = DELETE_THIS;
      buyer->doSave(SILENT_NO);
      return;
    }
  }
}

// may return DELETE_THIS (buyer dead)
int repairman(TBeing *buyer, cmdTypeT cmd, const char *arg, TMonster *repair, TObj *o)
{
  extern int counter_work;
  extern int counter_done;
  char buf[256];
  int rc = 0;
  dirTypeT dir;
  wearSlotT j;
  roomDirData *exitp;
  TThing *t;
  //  TObj *o;

  class job_struct {
    public:
    int number_being_worked_on; // # of items in directory being worked on 
    int number_finished;        // # of items in dir that timer is out on  
    int is_idle;                // Is the repairman idle or talking to cust. 
    int wait;                   // Used to make him wait a few pulses       
  };
  static job_struct *work;

  // I want to have him check for customers, and go back into his shop 
  // if he has no customers present. Maybe have a bell to get him back 
  switch (cmd) {
    case CMD_GENERIC_DESTROYED:
      delete (job_struct *) repair->act_ptr;
      repair->act_ptr = NULL;
      return FALSE;
    case CMD_GENERIC_CREATED:
      if (!repair->act_ptr) {
        if (!(repair->act_ptr = new job_struct())) {
          perror("failed new of repairman.");
          exit(0);
        }
      }

      sprintf(buf, "mobdata/repairs/%d", (repair)->mobVnum());
      global_repair = (repair)->mobVnum();
      dirwalk(buf, count_repair_items);

      work = (job_struct *) repair->act_ptr;
      work->number_being_worked_on = counter_work;
      work->number_finished = counter_done;
      return FALSE;
    case CMD_MOB_MOVED_INTO_ROOM:
      if (dynamic_cast<TBeing *>(buyer->riding)) {
        sprintf(buf, "Hey, get that damn %s out of my shop!",
            fname(buyer->riding->name).c_str());
        repair->doSay(buf);
        act("You throw $N out.", FALSE, repair, 0, buyer, TO_CHAR);
        act("$n throws you out of $s shop.", FALSE, buyer, 0, buyer, TO_VICT);
        act("$n throws $N out of $s shop.", FALSE, buyer, 0, buyer, TO_NOTVICT);
        --(*buyer->riding);
        thing_to_room(buyer->riding, (int) o);
        --(*buyer);
        thing_to_room(buyer, (int) o);
        return TRUE;
      } else if (dynamic_cast<TBeing *>(buyer->rider)) {
        --(*buyer->rider);
        thing_to_room(buyer->rider, (int) o);
        --(*buyer);
        thing_to_room(buyer, (int) o);
        return TRUE;
      }
      return FALSE;
    case CMD_MOB_VIOLENCE_PEACEFUL:
      repair->doSay("Hey!  Take it outside.");
      for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
        if (exit_ok(exitp = repair->exitDir(dir), NULL)) {
          // since "o" is really a being, cast up, and back down
          TThing *ttt = o;
          TBeing *tbt = dynamic_cast<TBeing *>(ttt);
          act("$n throws you from $s shop.",
                 FALSE, repair, 0, buyer, TO_VICT);
          act("$n throws $N from $s shop.",
                 FALSE, repair, 0, buyer, TO_NOTVICT);
          repair->throwChar(buyer, dir, FALSE, SILENT_NO, true);
          act("$n throws you from $s shop.",
                 FALSE, repair, 0, tbt, TO_VICT);
          act("$n throws $N from $s shop.",
                 FALSE, repair, 0, tbt, TO_NOTVICT);
          repair->throwChar(tbt, dir, FALSE, SILENT_NO, true);
          return TRUE;
        }
      }
      return TRUE;
    case CMD_VALUE:
      repair->sendTo("You ask the repairman for an estimate.\n\r");
      act("$n asks $N for an estimate on an item.", FALSE, buyer, NULL, repair, TO_ROOM);
      repairman_value(arg, repair, buyer);
      return TRUE;
    case CMD_GIVE:
      if (repairman_give(arg, repair, buyer) == DELETE_THIS) {
        return DELETE_THIS;  // buyer is poof
      }
      return TRUE;
    case CMD_REMOVE:
      one_argument(arg, buf);
      if (is_abbrev(buf, "all.damaged")) {
        act("$N helps you decide what equipment you own that $E can fix.", 0, buyer, 0, repair, TO_CHAR);
        for (j = MIN_WEAR; j < MAX_WEAR; j++) {
          if ((t = buyer->equipment[j]) && 
              (o = dynamic_cast<TObj *>(t)) &&
              (!will_not_repair(buyer, repair, o, SILENT_YES)) &&
             (o->getStructPoints() < o->maxFix(repair, DEPRECIATION_NO))) {
            if (buyer->carryVolumeLimit() > buyer->getCarriedVolume()) {
              if ((t = buyer->unequip(j))) {
                act("You stop using your damaged $o.", 0, buyer, t, 0, TO_CHAR);
                rc = t->removeMe(buyer, j);
                if (IS_SET_DELETE(rc, DELETE_THIS)) {
                  delete t;
                  t = NULL;
                }
                if (IS_SET_DELETE(rc, DELETE_THIS)) 
                  return DELETE_THIS;
              }
            } else {
              buyer->sendTo("You can't carry any more stuff.\n\r");
              break;
            }
          }
        }
        act("$n stops using $s damaged equipment.", TRUE, buyer, o, 0, TO_ROOM);
        return TRUE;
      }
      return FALSE;
    default:
      return FALSE;
  }
}

void TNote::noteMe(TMonster *repair, TBeing *buyer, TObj *repaired, time_t when_ready, int tick_num)
{
  char buf[512];
  char *ready;

  swapToStrung();

  sprintf(buf, "A receipt ticket from %s's shop lies here.", repair->getName());
  delete [] getDescr();
  setDescr(mud_str_dup(buf));

  sprintf(buf, "a small ticket marked number %d", tick_num);
  delete [] shortDescr;
  shortDescr = mud_str_dup(buf);

  delete [] name;
  name = mud_str_dup("ticket");

  sprintf(buf, "Ticket number %d from %s's shop:\n\r\n\r", tick_num, repair->getName());
  sprintf(buf + strlen(buf), "Item being repaired : %s\n\r", repaired->shortDescr);
  sprintf(buf + strlen(buf), "Estimated cost : %d talens.\n\r", repaired->repairPrice(repair, buyer, DEPRECIATION_YES));
  sprintf(buf + strlen(buf), "Condition after repair : %s.\n\r", 
          repaired->equip_condition(repaired->maxFix(repair, DEPRECIATION_YES)).c_str());
  
  ready = asctime(localtime(&when_ready));

  *(ready + strlen(ready) - 9) = '\0';
  sprintf(buf + strlen(buf), "Estimated time of finish : %s.\n\r", ready);
  delete [] action_description;
  action_description = mud_str_dup(buf);

  obj_flags.cost = repaired->obj_flags.cost;
  setRepairman(mob_index[repair->getMobIndex()].virt);
  setTimeAdj(buyer->desc && buyer->desc->account ? buyer->desc->account->time_adjust : 0);
  setObjV(repaired->objVnum());

  if (repaired->isObjStat(ITEM_NODROP))
    addObjStat(ITEM_NODROP);
}

void count_repair_items(const char *name)
{
  long tmp;
  FILE *fp;
  char buf[128];

  sprintf(buf, "mobdata/repairs/%d/%s",global_repair, name);
  if (!(fp = fopen(buf, "r"))) {
    vlogf(LOG_BUG, "Had a bad time opening repair file (%s) for initialization.", name);
    return;
  }
  if (fread(&tmp, sizeof(tmp), 1, fp) != 1) {
    vlogf(LOG_BUG, "Couldn't find a timer for repaiman file %s", name);
    fclose(fp);
    return;
  }
  if (tmp > time(0))
    counter_work++;
  else
    counter_done++;

  fclose(fp);
}

void processRepairFile(const char *name)
{
  rentObject item;
  long then;
  int cost;
  unsigned char version;
  FILE *fp;

  if (!(fp = fopen(name, "r"))) {
    vlogf(LOG_BUG, "Error reading repairman_file %s for limited item count.  Point 1!", name);
    return;
  }
  if (fread(&then, sizeof(then), 1, fp) != 1) {
    vlogf(LOG_BUG, "Error reading repairman_file %s for limited item count.  Point 2!", name);
    fclose(fp);
    return;
  }
#if NUKE_REPAIR_ITEMS
  if ((time(0) - then) > 15 * SECS_PER_REAL_DAY) {
    fclose(fp);
    vlogf(LOG_MISC, "REPAIR: Item %s was in repair %d days", name,
         (time(0) - then)/SECS_PER_REAL_DAY);
    unlink(name);
    return;
  }
#endif
  if (fread(&cost, sizeof(cost), 1, fp) != 1) {
    vlogf(LOG_BUG, "Error reading repairman_file %s for limited item count.  Point 3!", name);
    fclose(fp);
    return;
  }
  if (fread(&version, sizeof(version), 1, fp) != 1) {
    vlogf(LOG_BUG, "Error reading repairman_file %s for limited item count.  Point 3b!", name);
    fclose(fp);
    return;
  }
  if (fread(&item, sizeof(item), 1, fp) != 1) {
    vlogf(LOG_BUG, "Error reading repairman_file %s for limited item count.  Point 4!", name);
    fclose(fp);
    return;
  }
  if ((item.cost > LIM_ITEM_COST_MIN) && (item.item_number >= 0)) {
    vlogf(LOG_BUG, "     [%d] - %s", item.item_number, name);
    obj_index[real_object(item.item_number)].addToNumber(1);
  }
  fclose(fp);
}

void processRepairFiles(void)
{
  dirwalk_subs_fullname("mobdata/repairs", processRepairFile);
}

int repairMetal(TBeing *ch, TObj *o)
{
  if (!ch->doesKnowSkill(SKILL_SMYTHE)) {
    ch->sendTo("You really don't know enough about repairing metal items.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_SMYTHE, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
  return 0;
}

int repairDead(TBeing *ch, TObj *o)
{
  if (!ch->doesKnowSkill(SKILL_REPAIR_SHAMAN)) {
    ch->sendTo("You really don't know enough about mending bodily materials.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_REPAIR_DEAD, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
  return 0;
}

int repairOrganic(TBeing *ch, TObj *o)
{
  if (!ch->doesKnowSkill(SKILL_REPAIR_RANGER)) {
    ch->sendTo("You really don't know enough about repairing organic materials.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_REPAIR_ORGANIC, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
  return 0;
}

int repairMagical(TBeing *ch, TObj *o)
{
  if (!ch->doesKnowSkill(SKILL_REPAIR_MAGE)) {
    ch->sendTo("You really don't know enough about repairing magical materials.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_REPAIR_MAGICAL, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
  return 0;
}

int repairRock(TBeing *ch, TObj *o)
{
  if (!ch->doesKnowSkill(SKILL_REPAIR_MAGE) && !ch->doesKnowSkill(SKILL_REPAIR_RANGER)) {
    ch->sendTo("You really don't know enough about repairing rocks.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_REPAIR_ROCK, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
  return 0;
}

int repairCrystal(TBeing *ch, TObj *o)
{
  if (!ch->doesKnowSkill(SKILL_SMYTHE_ADVANCED) && !ch->doesKnowSkill(SKILL_REPAIR_THIEF)) {
    ch->sendTo("You really don't know enough about repairing crystalline materials.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_SMYTHE_ADVANCED, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
  return 0;
}

int repairHide(TBeing *ch, TObj *o)
{
  if (!ch->doesKnowSkill(SKILL_REPAIR_RANGER) && !ch->doesKnowSkill(SKILL_REPAIR_MONK)) {
    ch->sendTo("You really don't know enough about mending hides.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_MEND_HIDE, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
  return 0;
}

int repairGeneric(TBeing *ch, TObj *o)
{
  if (!ch->doesKnowSkill(SKILL_MEND)) {
    ch->sendTo("You really don't know enough about basic mending.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_MEND, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
  return 0;
}

int repairSpiritual(TBeing *ch, TObj *o)
{
  if (!ch->doesKnowSkill(SKILL_REPAIR_CLERIC) && !ch->doesKnowSkill(SKILL_REPAIR_DEIKHAN)) {
    ch->sendTo("You really don't know enough about repairing holy items.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_REPAIR_SPIRITUAL, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
  return 0;
}



