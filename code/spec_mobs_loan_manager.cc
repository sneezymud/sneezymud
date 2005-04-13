#include "stdsneezy.h"
#include "database.h"
#include "shop.h"
#include "shopowned.h"
#include "corporation.h"

// need a periodic function to create the loans

// loan table:
// amount of loan
// interest rate
// risk

TMonster *findSBA()
{
  int shop_nr=160;
  TMonster *keeper;
  TBeing *t;

  for(t=character_list;t;t=t->next){
    if(t->number==shop_index[shop_nr].keeper)
      break;
  }

  if(t && (keeper=dynamic_cast<TMonster *>(t))){
    return keeper;
  }

  return NULL;
}



void loanBuy(TBeing *ch, TMonster *myself, sstring arg)
{
  int loan_id=convertTo<int>(arg.word(0));
  TDatabase db(DB_SNEEZY);
  int shop_nr=find_shop_nr(myself->number);
  TShopOwned tso(shop_nr, myself, ch);
  TCorporation corp(tso.getCorpID());
  
  if(!loan_id){
    myself->doTell(ch->getName(), "That isn't a valid loan number");
    return;
  }

  db.query("select amt from shopownednpcloans where loan_id=%i",
	   loan_id);
  
  if(!db.fetchRow()){
    myself->doTell(ch->getName(), "That isn't a valid loan number.");
    return;
  }

  // make sure they have money
  int amt=convertTo<int>(db["amt"]);
  
  if(ch->getMoney() < amt){
    myself->doTell(ch->getName(), "You don't have enough money.");
    return;
  }
  
  // make sure they have an account at my bank
  db.query("select 1 from shopownedbank where player_id=%i and shop_nr=%i",
	   ch->getPlayerID(), corp.getBank());

  if(!db.fetchRow()){
    TRoom *tr=real_roomp(shop_index[corp.getBank()].in_room);
    myself->doTell(ch->getName(), fmt("You need to have a bank account at %s in order to finance loans.") % tr->getName());
    return;
  }

  db.query("update shopownednpcloans set owner=%i where loan_id=%i",
	   ch->getPlayerID(), loan_id);

#if 0  
  TMonster *sba;
  // give loan amount to SBA
  if(!(sba=findSBA())){
    vlogf(LOG_BUG, "couldn't find SBA!");
    return;
  }
  
  sba->addToMoney(amt, GOLD_SHOP);
  shoplog(160, ch, sba, "talens", amt, "loaning");
  sba->saveItems(fmt("%s/%d") % SHOPFILE_PATH % 160);

  // give fee amount to myself
#endif
  
}

void loanList(TBeing *ch, TMonster *myself)
{
  TDatabase db(DB_SNEEZY);

  db.query("select loan_id, amt, rate, risk from shopownednpcloans order by loan_id");

  while(db.fetchRow()){
    ch->sendTo(fmt("[%s] %s talens at %s, risk %s\n\r") %
	       db["loan_id"] % db["amt"] % db["rate"] % db["risk"]);
  }
}


int loanManager(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  int shop_nr;

  if(cmd!=CMD_WHISPER && cmd!=CMD_BUY && cmd!=CMD_LIST)
    return false;

  shop_nr=find_shop_nr(myself->number);

  switch(cmd){
    case CMD_WHISPER:
      return shopWhisper(ch, myself, shop_nr, arg);
    case CMD_LIST:
      loanList(ch, myself);
      break;
    case CMD_BUY:
      loanBuy(ch, myself, arg);
      break;
    default:
      break;
  }

  return true;
}
