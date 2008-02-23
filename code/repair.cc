#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "stdsneezy.h"
#include "statistics.h"
#include "obj_note.h"
#include "shop.h"
#include "database.h"
#include "shopowned.h"
#include "rent.h"
#include "obj_commodity.h"

extern int kick_mobs_from_shop(TMonster *myself, TBeing *ch, int from_room);

int counter_done;  // Global variable used to count # of done items/repairman 
int counter_work;  // Global variable used to count # of undone items/man 

static int global_repair = 0;

const float repair_mats_ratio=0.10;

int TObj::maxFix(const TBeing *keeper, depreciationTypeT dep_done) const
{
  int amount = getMaxStructPoints() - getDepreciation();

  amount *= 95;
  amount /= 100;


  if(keeper){
    unsigned int shop_nr=0;
    TDatabase db(DB_SNEEZY);
    
    for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != (keeper)->number); shop_nr++);
    
    if (shop_nr >= shop_index.size()) {
      vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") %  mob_index[keeper->number].virt);
      return FALSE;
    }
    
    if(shop_index[shop_nr].isOwned()){
      db.query("select quality from shopownedrepair where shop_nr=%i", shop_nr);
      
      if(db.fetchRow()){
	float quality=convertTo<float>(db["quality"]);
	
	if(quality<=1.0 && quality>0)
	  amount = (int)((float) amount * quality);
      }
    }
  }

  return amount;
}


int findRepairMaterials(unsigned int shop_nr, TBeing *repair, TBeing *buyer, ubyte mat, int &mats_needed, bool purchase)
{
  TThing *t;
  TCommodity *commod;
  int mat_price=0;
  unsigned int commod_shop=15;

  // optimally we'd go with the physically nearest commod shop
  // for now this will work, but it leaves out some commod shops.
  TShopOwned repairshop(shop_nr, buyer);
  switch(repairshop.getCorpID()){
    case 27: // amber
      commod_shop=56;
      break;
    case 28: // logrus
      commod_shop=58;
      break;
    case 29: // bm
      commod_shop=57;
      break;
    case 21: // gh
    default:
      commod_shop=15;
  }

  TShopOwned tso(commod_shop, repair);

  // look through the commod shop inventory
  for(t=tso.getStuff();t;t=t->nextThing){
    // find the appropriate commodity
    if((commod=dynamic_cast<TCommodity *>(t)) && commod->getMaterial() == mat){
      // get the price of the commods we need
      if(commod->numUnits() > mats_needed){
        mat_price += commod->shopPrice(mats_needed, commod_shop, 0, buyer);
        if(purchase){
          tso.doBuyTransaction(mat_price, commod->getName(), TX_BUYING, commod);
          shoplog(shop_nr, buyer, dynamic_cast<TMonster *>(repair), commod->getName(), -mat_price, "buying materials");
          commod->setWeight(commod->getWeight() - mats_needed/10.0);
        }
        mats_needed=0;
      } else {
        mat_price += commod->shopPrice(commod->numUnits(), commod_shop, 0, buyer);
        if(purchase){
          tso.doBuyTransaction(mat_price, commod->getName(), TX_BUYING, commod);
          shoplog(shop_nr, buyer, dynamic_cast<TMonster *>(repair), commod->getName(), -mat_price, "buying materials");
          delete commod;
        }
        mats_needed-=commod->numUnits();
      }
      break;
    }
  }

  return mat_price;
}


int TObj::repairPrice(TBeing *repair, TBeing *buyer, depreciationTypeT dep_done, bool purchase, int *pmatCost) const
{
  unsigned int shop_nr=0;

  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != repair->number); shop_nr++);

  // dep_done will be true if depreciation accounted for
  // since value doesn't dep o, we need to fudge it

  // makes an assumption that struct is > 0
  // pre assumes that it needs repairing.
  int gsp = obj_flags.cost;

  // subtract the cost of the material repair portion
  gsp -= (int)(getWeight() * 10.0 * material_nums[getMaterial()].price);

  // how much of the structure is being repaired
  float perc_repaired = 
    ((float)((maxFix(repair, dep_done) - getStructPoints())/
	     (float) getMaxStructPoints()));

  // ideally, this price will be < gsp, but gold_mod should handle that for us
  int price = (int) (gsp * gold_modifier[GOLD_REPAIR].getVal());

  // scale full repair price by the amount actually being repaired
  price = (int)(price * perc_repaired);

  // "destroyed" items are super expensive to repair
  if(getStructPoints() <= 0)
    price *= 5;

  // units of material needed to repair
  int mat_price=0;

  int mats_needed=(int)(getWeight()* 10.0 * perc_repaired);
  mats_needed = (int)(repair_mats_ratio * mats_needed);

  // add in the price of the raw material
  mat_price+=findRepairMaterials(shop_nr, repair, buyer, getMaterial(), mats_needed, purchase);

  // 200% base cost for materials that we can't purchase
  mat_price+=(int)(mats_needed * material_nums[getMaterial()].price * 2);

  //    vlogf(LOG_PEEL, fmt("gsp=%i, perc_repaired=%f, price=%i, mats_needed=%i, mat_price=%i") % gsp % perc_repaired % price % mats_needed % mat_price);


#if FACTIONS_IN_USE
  if (!repair->isUnaff() && repair->isSameFaction(buyer)) {
    price *= (200 - (int) buyer->getPerc());
    price /= 200;
  }
#endif

  // check for shop setting
  float profit_buy=shop_index[shop_nr].getProfitBuy(NULL, buyer);

    
  //raw materials aren't modified by profit rates
  price = (int)((double) price * profit_buy);

  if (pmatCost)
    *pmatCost = mat_price;

  // I don't know why this is here and it's causing problems
  //  price = (price * 75) / 100;

  return (mat_price+price);
}

// time it will take to repair an item in seconds 
static int repair_time(TBeing *keeper, const TObj *o)
{
  int structs;
//  double percDam;
  double iTime;
  float speed = 0;
//  int MINS_AT_60TH = 60; // maximum (full repair) for 60TH level eq
  unsigned int shop_nr=0;
  TDatabase db(DB_SNEEZY);

  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != (keeper)->number); shop_nr++);

  if (shop_nr >= shop_index.size()) {
    vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") %  mob_index[keeper->number].virt);
    return FALSE;
  }

  if(shop_index[shop_nr].isOwned()){
    db.query("select speed from shopownedrepair where shop_nr=%i", shop_nr);
    
    if(db.fetchRow()){
      speed=convertTo<float>(db["speed"]);

//      if(speed <= 5.0 && speed > 0)
//	MINS_AT_60TH=(int)((float)MINS_AT_60TH * speed);
    }
  }


  if (!(structs = (o->getMaxStructPoints() - o->getStructPoints())))
    return (0);

#ifdef OLDSCHOOL
  percDam = ((double)(structs*75) / (double)(o->getMaxStructPoints())) + 25.0;
  double levmod = (double)(o->objLevel() * o->objLevel());
#if 1
  iTime = levmod * (percDam);
  iTime /= 100.0*60.0; // correct for the % thing
  iTime *= (double)MINS_AT_60TH;
  // max repair time * % damage to object
#endif
#endif // OLDSCHOOL
#if 1 // NEWSCHOOL
  // try to mimic speeds of player repair, with a healthy dose of extra waiting
  // player repair is 4 + success(struct) + fail(struct) (okay this is loose, since fails will require another success)
  // where success is skillcheck > (rand(1,101)-dexbonus*3) and 101 always fails
  // so if we say mob repairs are skilled at around 65% and their dexbonus is 0
  // it should be 4 + struct / 0.3 as the average amount of ticks to repair the object
  // because the approximate average here is 2 * (successrate - .5)  (we ignore the 1/101 chance of failure)
  iTime = 4 + (structs / 0.3);
  iTime *= (PULSE_MOBACT/ONE_SECOND); // seconds per player repair pulse

  // adjust this time by a const to represent the lameness of paying for something you should do yourself
  iTime *= 1.25;

  if (speed <= 5.0 && speed > 0)
    iTime *= speed;
#endif // NEWSCHOOL

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
  ItemSave is;

  // check for valid args
  if (!repair || !buyer) {
    vlogf(LOG_BUG, "save_repairman_file() called with NULL ch!");
    return;
  }
  if (!buyer->isPc()) {
    vlogf(LOG_BUG, fmt("Non-PC got into save_repairman_file() somehow!!! BUG BRUTIUS!!") %  buyer->getName());
    return;
  }

  // open the file we will write to
  sprintf(buf, "mobdata/repairs/%d/%d", repair->mobVnum(), repair_number);
  if (!(fp = fopen(buf, "w"))) {
    sprintf(buf2, "mobdata/repairs/%d", repair->mobVnum());
    if (mkdir(buf2, 0770)) {
      vlogf(LOG_BUG, fmt("Unable to create a repair directory for %s.") % 
	    repair->getName());
      return;
    } else {
      vlogf(LOG_BUG, fmt("Created a repair directory for %s") %  repair->getName());
    }
    
    if (!(fp = fopen(buf, "w"))) {
      vlogf(LOG_BUG, fmt("Major problems trying to save %s repair file.") %  
	    repair->getName());
      return;
    }
  }

  // write the header data
  then = (long) iTime;
  if (fwrite(&then, sizeof(then), 1, fp) != 1) {
    vlogf(LOG_BUG, "Error writing time for repairman_file!");
    fclose(fp);
    return;
  }

  cost = o->repairPrice(repair, buyer, DEPRECIATION_YES, true, NULL);
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

  // write the object
  is.setFile(fp);
  is.raw_write_item(o);

  // Save the repair number so we can keep up with it.
  save_game_stats();
}

TObj *loadRepairItem(TBeing *repair, int ticket, 
		     long &time, int &cost, unsigned char &version)
{
  FILE *fp;
  TObj *obj;

  // open the repair file for this ticket
  sstring filename=fmt("mobdata/repairs/%d/%d") % repair->mobVnum() % ticket;

  if (!(fp = fopen(filename.c_str(), "r"))) {
    repair->doSay(fmt("I don't seem to have an item for ticket number %d")
		  % ticket);
    return NULL;
  }

  // read the repair data
  if (fread(&time, sizeof(time), 1, fp) != 1) {
    vlogf(LOG_BUG, fmt("No timer on item number %d for repairman %s") %  
	  ticket % repair->getName());
    repair->doSay("Something is majorly wrong(Timer). Talk to a god");
    fclose(fp);
    return NULL;
  }
  if (fread(&cost, sizeof(cost), 1, fp) != 1) {
    vlogf(LOG_BUG, fmt("No cost on item number %d for repairman %s") %  
	  ticket % repair->getName());
    repair->doSay("Something is majorly wrong(Cost). Talk to a god");
    fclose(fp);
    return NULL;
  }
  if (fread(&version, sizeof(version), 1, fp) != 1) {
    vlogf(LOG_BUG, fmt("No version on item number %d for repairman %s") %  
	  ticket % repair->getName());
    repair->doSay("Something is majorly wrong(version). Talk to a god");
    fclose(fp);
    return NULL;
  }
  
  // read the object data
  ItemLoad il;
  il.setFile(fp);
  il.setVersion(version);
  obj=il.raw_read_item();

  return obj;
}

static int getRepairItem(TBeing *repair, TBeing *buyer, int ticket, TNote *obj)
{
  char buf[80];
  long tmp, cur_time;
  int tmp_cost, diff, hours, minutes, seconds;
  TObj *fixed_obj;
  unsigned char version;

  // check for valid args
  if (!repair || !buyer) {
    vlogf(LOG_BUG, "check_time called with NULL character! BUG BRUTIUS!");
    return FALSE;
  }

  // load the item
  if(!(fixed_obj=loadRepairItem(repair, ticket, tmp, tmp_cost, version))){
    repair->doSay("Whoa, serious problems, tell a god.");
    vlogf(LOG_BUG, "Bogus load of repair item problem!!!!!!");
    return TRUE;
  }

  cur_time = time(0) + 3600 * obj->getTimeAdj();

  // check if the item is ready yet
  if (cur_time < tmp) {
    diff = tmp - cur_time;
    hours = diff/3600;
    minutes = (diff % 3600)/60;
    seconds = diff % 60;

    repair->doTell(fname(buyer->name), "Your item isn't ready yet.");
    repair->doTell(fname(buyer->getName()),  fmt("It will be ready in %d hours, %d minutes and %d seconds.") % hours % minutes % seconds);
    delete fixed_obj;
    return FALSE;
  }
  
  // check if the player has enough money to pay for it
  if (tmp_cost > buyer->getMoney()) {
    sprintf(buf, "%s I don't repair items for free! No money, no item!", buyer->getName());
    repair->doSay(buf);
    sprintf(buf, "Remember the price is %d.", tmp_cost);
    repair->doSay(buf);
    delete fixed_obj;
    return FALSE;
  }

  unlink((fmt("mobdata/repairs/%d/%d") % repair->mobVnum() % ticket).c_str());

  repair->doTell(fname(buyer->name), fmt("Ah yes, %s, here is %s.") %
		 buyer->getName() % fixed_obj->shortDescr);
  repair->doSay("Thank you for your business!");


  obj_index[fixed_obj->getItemIndex()].addToNumber(-1);
  fixed_obj->setStructPoints(fixed_obj->maxFix(repair, DEPRECIATION_YES));
  
  *buyer += *fixed_obj;
  buyer->doSave(SILENT_YES);
  buyer->logItem(fixed_obj, CMD_NORTH);   // cmd indicates repair-retrieval

  unsigned int shop_nr;
  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != repair->number); shop_nr++);
  
  if (shop_nr >= shop_index.size()) {
    vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") %  mob_index[repair->number].virt);
  }

  TShopOwned tso(shop_nr, dynamic_cast<TMonster *>(repair), buyer);
  tso.doBuyTransaction(tmp_cost, "repair", TX_BUYING_SERVICE);
  
  
  // acknowledge the depreciation after all work is done
  // this way the price doesn't change during the process
  // and also makes the first repair "depreciation free"
  
  fixed_obj->addToDepreciation(1);
  
  return TRUE;
}

static bool will_not_repair(TBeing *ch, TMonster *repair, TObj *obj, silentTypeT silent)
{
  if (obj->objectRepair(ch, repair, silent)) 
    return TRUE;

  if (!obj->isRentable()) {
    if (!silent) {
      repair->doTell(fname(ch->name), 
		     "I'm sorry, but that item is unrepairable.");
    }
    return TRUE;
  }
  if (obj->getStructPoints() == obj->getMaxStructPoints()) {
    if (!silent) {
      repair->doTell(fname(ch->name), 
		     "It doesn't look like that item needs any repairing.");
    }
    return TRUE;
  } 
  if (obj->getStructPoints() >= obj->maxFix(NULL, DEPRECIATION_NO)) {
    // check depreciation alone
    if (!silent) {
      repair->doTell(fname(ch->name), 
		 "That item's damage isn't something that can be repaired.");
    }
    return TRUE;
  } 
  if (obj->getStructPoints() >= obj->maxFix(repair, DEPRECIATION_NO)) {
    // check repairman's skill
    if (!silent) {
      repair->doTell(fname(ch->name), "I hate to admit it, but I don't think I have the skill to fix that further.");
    }
    return TRUE;
  } 
  if (!repair_time(repair, obj)) {
    // probably superfluous
    if (!silent) {
      repair->doTell(fname(ch->name), fmt("%s looks fine to me.") % 
		     obj->getName());
    }
    return TRUE;
  }
  if (obj->objVnum() == -1) {
    if (!silent) {
      repair->doTell(fname(ch->name), 
		     fmt("I can't take temporary items like %s.") % 
		     obj->getName());
    }
    return TRUE;
  }
  if (obj->isObjStat(ITEM_NODROP)) {
    if (!silent) {
      repair->doTell(fname(ch->name), "I can't take cursed items.");
    }
    return TRUE;
  }
  if (obj->isObjStat(ITEM_BURNING)) {
    if (!silent) {
      repair->doTell(fname(ch->name), "HOLY CRAP GET THAT THING OUT OF HERE BEFORE YOU BURN THE WHOLE PLACE DOWN!.");
    }
    return TRUE;
  }
  if (obj->isObjStat(ITEM_CHARRED)) {
    if (!silent) {
      repair->doTell(fname(ch->name), "I can repair this, but it is very badly fire-damaged.");
    }
  }
  if (obj->isObjStat(ITEM_RUSTY)){
    if(!silent){
      repair->doTell(fname(ch->name), "This is a little rusty, but I can polish it up.");
    }
  }
  if (obj_index[obj->getItemIndex()].getNumber() > 
      obj_index[obj->getItemIndex()].max_exist) {
    // item over max-exist, never supposed to happen, but could
    // make it unrepairable to encourage it to scrap
    if (!silent) {
      repair->doTell(fname(ch->name), "Someone has put out a contract to reclaim that item.  It's just too dangerous for me to take it.");
    }

    return TRUE;
  }

  if (obj->getStuff()) {
    // probably a mage-belt with components in it....
    if (!silent) {
      repair->doTell(fname(ch->name), "Sorry, you'll have to empty it out before I can do any work on it.");
    }
    return TRUE;
  }
  return FALSE;
}

void repairman_value(const char *arg, TMonster *repair, TBeing *buyer)
{
  char *ready;
  TObj *valued;
  time_t when_ready, ct;

  if (buyer->desc && buyer->desc->account)
    ct = time(0) + 3600 * buyer->desc->account->time_adjust;
  else
    ct = time(0);

  for (;*arg && *arg == ' ';arg++);

  if (!*arg) {
    repair->doTell(fname(buyer->name), "Can you be a little more specific about what you want to value....\n\r");
    return;
  }

  if (is_abbrev(arg, "all.damaged")) {
    TThing * tListHead   = buyer->getStuff();
    int      iCostForAll = 0;

    while (tListHead) {
      valued = dynamic_cast<TObj *>(tListHead);

      if (valued)
        if (!will_not_repair(buyer, repair, valued, SILENT_YES)) {
          int matCost = 0;
          int repairCost = valued->repairPrice(repair, buyer, DEPRECIATION_NO, false, &matCost);
          const char* plural = matCost != 1 ? "s" : "";
          repair->doTell(fname(buyer->name),
                         fmt("It'll cost you %d talens to repair %s to a status of %s.") %
                         repairCost %
                         valued->getName() %
                         valued->equip_condition(valued->maxFix(repair, DEPRECIATION_NO)));
          repair->doTell(fname(buyer->name),
                         fmt("%d talen%s of that cost is for raw materials.") %
                         matCost %
                         plural);

          iCostForAll += repairCost;
        }

      tListHead = tListHead->nextThing;
    }

    if (!iCostForAll)
      repair->doTell(fname(buyer->name), fmt("%s, You don't have anything I can repair in your inventory...") % buyer->getName());
    else
      repair->doTell(fname(buyer->name), fmt("It will cost a total of %d talens to repair all the listed items.") % iCostForAll);

    return;
  }

  TThing *t_valued = searchLinkedListVis(buyer, arg, buyer->getStuff());
  valued = dynamic_cast<TObj *>(t_valued);
  if (!valued) {
    repair->doTell(fname(buyer->name), 
		   fmt("%s, You don't have that item.\n\r") % 
		   buyer->getName());
    return;
  }
  if (will_not_repair(buyer, repair, valued, SILENT_NO))
    return;

  int singleMatCost = 0;
  int singleRepairCost = valued->repairPrice(repair, buyer, DEPRECIATION_NO, false, &singleMatCost);
  const char* costPlural = singleMatCost != 1 ? "s" : "";
  repair->doTell(fname(buyer->name),
                 fmt("It'll cost you %d talens to repair %s to a status of %s.") %
                 singleRepairCost %
                 valued->getName() %
                 valued->equip_condition(valued->maxFix(repair, DEPRECIATION_NO)));
  repair->doTell(fname(buyer->name),
                 fmt("%d talen%s of that cost is for raw materials.") %
                 singleMatCost %
                 costPlural);

  when_ready = ct + repair_time(repair, valued);
  ready = asctime(localtime(&when_ready));
  *(ready + strlen(ready) - 9) = '\0';
  repair->doTell(fname(buyer->name), fmt("I can have it ready by %s.") % ready);
  repair->doTell(fname(buyer->name), fmt("That's %s.") % secsToString(when_ready-ct));
}

// returns DELETE_THIS if buyer goes poof
int repairman_give(const char *arg, TMonster *repair, TBeing *buyer)
{
  char buf[256], obj_name[MAX_INPUT_LENGTH];
  char rep_name[MAX_INPUT_LENGTH];
  char obj_amt[MAX_INPUT_LENGTH];
  TThing *t, *t2;

  arg = one_argument(arg, obj_name, cElements(obj_name));

  if (convertTo<int>(obj_name) && (obj_name[1] != '.') && (obj_name[2] != '.')) {
    strcpy(obj_amt, obj_name);
    arg = one_argument(arg, obj_name, cElements(obj_name));
  } else
    *obj_amt = '\0';

  arg = one_argument(arg, rep_name, cElements(rep_name));
  if (*rep_name && !strcmp(rep_name, "to"))
    arg = one_argument(arg, rep_name, cElements(rep_name));

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
          total += tobj->repairPrice(repair, buyer, DEPRECIATION_YES, false, NULL);
          delete tobj;
          tobj = NULL;
          buyer->doSave(SILENT_YES);
        }
      }
    }
    if (!found) 
      buyer->sendTo("You have no damaged items in your inventory!\n\r");
    else
      buyer->sendTo(fmt("You gave a total of %d talens in damaged equipment.\n\r") % total);

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
    repair->doTell(fname(buyer->name), "You don't have that item.");
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

  repair->doTell(fname(buyer->name), fmt("It'll cost you %d talens to repair %s to a status of %s.") % (repairPrice(repair, buyer, DEPRECIATION_YES, false, NULL)) % getName() % equip_condition(maxFix(repair, DEPRECIATION_YES)));

  when_ready = ct + repair_time(repair, this);
  ready = asctime(localtime(&when_ready));
  *(ready + strlen(ready) - 9) = '\0';
  repair->doTell(fname(buyer->name), fmt("It will be ready %s.") % ready);
  repair->doTell( fname(buyer->name), fmt("That's %s.") % secsToString(when_ready-ct));
  repair_number++;
  repair->doTell(fname(buyer->name), "Payment is due when you pick your item up.");
  repair->doTell(fname(buyer->name), fmt("Here is your ticket, %s") % buyer->getName());
  repair->doTell(fname(buyer->name), "If you lose the ticket, it might be hard to reclaim your item.");
  ticket = make_ticket(repair, buyer, this, when_ready, repair_number);
  *buyer += *ticket;
  save_repairman_file(repair, buyer, this, when_ready, repair_number);

  // vlogf(LOG_DASH, fmt("%s repairing %s - str %d/%d, lev %d.  Repair time: %s.") %  
  //fname(buyer->name).c_str() % getName() % (int)getStructPoints() % (int)getMaxStructPoints() %
  //(int)(this->objLevel()*1) % secsToString(when_ready-ct).c_str());

  buyer->logItem(this, CMD_REPAIR);

  unsigned int shop_nr=0;
  TDatabase db(DB_SNEEZY);
  
  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != repair->number); shop_nr++);

  if (shop_nr >= shop_index.size()) {
    vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") %  mob_index[number].virt);
  }
  shoplog(shop_nr, buyer, repair, getName(), 0, "receiving");


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
    repair->doTell(fname(buyer->name), "That ticket is blank!");
    return;
  }
  if (getRepairman() != mob_index[repair->getMobIndex()].virt) {
    repair->doTell(fname(buyer->name), "That isn't one of my tickets!");
    return;
  }
  strcpy(buf, getName());
  if (sscanf(buf, "a small ticket marked number %d", &iNumber) != 1) {
    repair->doTell(fname(buyer->name), "That ticket isn't from THIS shop!");
  } else {
    if (getRepairItem(repair, buyer, iNumber, this)) {
      *found = DELETE_THIS;
      buyer->doSave(SILENT_NO);
      return;
    }
  }
}


sstring repairList(TMonster *repair)
{
  struct dirent *dp;
  DIR *dfd;
  sstring buf;
  long time;
  int cost, ticket;
  unsigned char version;
  TObj *o;

  if(!(dfd=opendir((fmt("mobdata/repairs/%d") % repair->mobVnum()).c_str()))){
    vlogf(LOG_BUG, fmt("Unable to dirwalk directory in repairList for %i") % 
	  repair->mobVnum());
    return "Unable to dirwalk directory";
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;
    
    ticket=convertTo<int>(dp->d_name);

    if((o=loadRepairItem(repair, ticket, time, cost, version))){
      buf+=fmt("%i) %s - %i talens\n\r") % ticket %
	o->getName() % cost;
      delete o;
    }
  }
  closedir(dfd);

  return buf;
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
  unsigned int shop_nr=0;

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

      return kick_mobs_from_shop(repair, buyer, (int)o);

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

      repair->saveChar(ROOM_AUTO_RENT);
      buyer->saveChar(ROOM_AUTO_RENT);

      return TRUE;
    case CMD_REMOVE:
      one_argument(arg, buf, cElements(buf));
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
    case CMD_LIST:
      if(buyer->isImmortal()){
	if(buyer->desc)
	  buyer->desc->page_string(repairList(repair));
	else
	  buyer->sendTo(COLOR_BASIC, repairList(repair));
	return TRUE;
      }
      return FALSE;
    case CMD_WHISPER:
      for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != repair->number); shop_nr++);

      if (shop_nr >= shop_index.size()) {
	vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") % mob_index[repair->number].virt);
	return FALSE;
      }
      
      return shopWhisper(buyer, repair, shop_nr, arg);
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
  sprintf(buf + strlen(buf), "Estimated cost : %d talens.\n\r", repaired->repairPrice(repair, buyer, DEPRECIATION_YES, false, NULL));
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
    vlogf(LOG_BUG, fmt("Had a bad time opening repair file (%s) for initialization.") %  name);
    return;
  }
  if (fread(&tmp, sizeof(tmp), 1, fp) != 1) {
    vlogf(LOG_BUG, fmt("Couldn't find a timer for repaiman file %s") %  name);
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
    vlogf(LOG_BUG, fmt("Error reading repairman_file %s for limited item count.  Point 1!") %  name);
    return;
  }
  if (fread(&then, sizeof(then), 1, fp) != 1) {
    vlogf(LOG_BUG, fmt("Error reading repairman_file %s for limited item count.  Point 2!") %  name);
    fclose(fp);
    return;
  }
#if NUKE_REPAIR_ITEMS
  if ((time(0) - then) > 180 * SECS_PER_REAL_DAY) {
    fclose(fp);
    vlogf(LOG_MISC, fmt("REPAIR: Item %s was in repair %d days") %  name %
         ((time(0) - then)/SECS_PER_REAL_DAY));
    unlink(name);
    return;
  }
#endif
  if (fread(&cost, sizeof(cost), 1, fp) != 1) {
    vlogf(LOG_BUG, fmt("Error reading repairman_file %s for limited item count.  Point 3!") %  name);
    fclose(fp);
    return;
  }
  if (fread(&version, sizeof(version), 1, fp) != 1) {
    vlogf(LOG_BUG, fmt("Error reading repairman_file %s for limited item count.  Point 3b!") %  name);
    fclose(fp);
    return;
  }
  if (fread(&item, sizeof(item), 1, fp) != 1) {
    vlogf(LOG_BUG, fmt("Error reading repairman_file %s for limited item count.  Point 4!") %  name);
    fclose(fp);
    return;
  }
  if (item.item_number >= 0) {
    vlogf(LOG_BUG, fmt("     [%d] - %s") %  item.item_number % name);
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
  if (!ch->doesKnowSkill(SKILL_BLACKSMITHING)) {
    ch->sendTo("You really don't know enough about repairing metal items.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_BLACKSMITHING, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
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
  if (!ch->doesKnowSkill(SKILL_REPAIR_MONK)) {
    ch->sendTo("You really don't know enough about repairing organic materials.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_REPAIR_ORGANIC, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
  return 0;
}

int repairWood(TBeing *ch, TObj *o)
{
  if (!ch->doesKnowSkill(SKILL_REPAIR_MONK)) {
    ch->sendTo("You really don't know enough about repairing wood.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_REPAIR_WOOD, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
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
  if (!ch->doesKnowSkill(SKILL_REPAIR_MAGE) && !ch->doesKnowSkill(SKILL_REPAIR_MONK)) {
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
  if (!ch->doesKnowSkill(SKILL_BLACKSMITHING_ADVANCED) && !ch->doesKnowSkill(SKILL_REPAIR_THIEF)) {
    ch->sendTo("You really don't know enough about repairing crystalline materials.\n\r");
    return FALSE;
  }
  act("You begin to prepare to fix $p.", FALSE, ch, o, 0, TO_CHAR);
  act("$n begins to prepare to fix $p.", FALSE, ch, o, 0, TO_ROOM);

  start_task(ch, NULL, NULL, TASK_BLACKSMITHING_ADVANCED, o->name, 999, (ushort) ch->in_room, 0, 0, 0);
  return 0;
}

int repairHide(TBeing *ch, TObj *o)
{
  if (!ch->doesKnowSkill(SKILL_REPAIR_MONK)) {
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



