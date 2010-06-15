#include "room.h"
#include "database.h"
#include "corporation.h"
#include "monster.h"

sstring TCorporation::getName()
{
  TDatabase db(DB_SNEEZY);

  db.query("select name from corporation where corp_id=%i", corp_id);
  db.fetchRow();
  return db["name"];
}

TCorporation::TCorporation(int c) :
  corp_id(c)
{
}

TCorporation::~TCorporation(){
}


void TCorporation::corpLog(const sstring &name, const sstring &action, int talens)
{
  TDatabase db(DB_SNEEZY);
  

  db.query("insert into corplog values (%i, '%s', '%s', %i, %i, now())", 
	   corp_id, name.c_str(), action.c_str(), talens, getMoney());
}


int TCorporation::getAccess(TBeing *ch){
  TDatabase db(DB_SNEEZY);
  
  db.query("select access from corpaccess where corp_id=%i and lower(name)='%s'", corp_id, sstring(ch->getName()).lower().c_str());

  if(db.fetchRow()){
    return convertTo<int>(db["access"]);
  }

  return 0;
}

bool TCorporation::hasAccess(TBeing *ch, int perm){
  TDatabase db(DB_SNEEZY);
  int access;

  db.query("select access from corpaccess where corp_id=%i and lower(name)='%s'",
	   corp_id, sstring(ch->getName()).lower().c_str());

  if(db.fetchRow()){
    access=convertTo<int>(db["access"]);
  } else {
    access=0;
  }

  return (access & perm);
}


int TCorporation::getBank()
{
  TDatabase db(DB_SNEEZY);

  db.query("select bank from corporation where corp_id=%i", corp_id);

  if(db.fetchRow())
    return convertTo<int>(db["bank"]);
  else
    return 4; // default to GH bank
}


void TCorporation::setBank(int bank)
{
  TDatabase db(DB_SNEEZY);

  db.query("update corporation set bank=%i where corp_id=%i", bank, corp_id);
  
}

int TCorporation::getMoney()
{
  TDatabase db(DB_SNEEZY);

  db.query("select talens from shopownedcorpbank b, corporation c where c.corp_id=%i and c.corp_id=b.corp_id and b.shop_nr=c.bank", corp_id);

  if(db.fetchRow())
    return convertTo<int>(db["talens"]);
  else
    return 0;
}


void TCorporation::setMoney(int g)
{
  TDatabase db(DB_SNEEZY);

  db.query("update shopownedcorpbank scb, corporation c set scb.talens=%i where scb.corp_id=c.corp_id and c.corp_id=%i and scb.shop_nr=c.bank", g, corp_id);
}

int TCorporation::getCorpID()
{
  return corp_id;
}


int TCorporation::getAssets()
{
  TDatabase db(DB_SNEEZY);

  db.query("select sum(price) as price from rent where owner_type='shop' and owner in (select shop_nr from shopowned where corp_id=%i)", corp_id);
  db.fetchRow();

  return convertTo<int>(db["price"]);
}


std::vector <corp_list_data> getCorpListingData(void)
{
  TDatabase db(DB_SNEEZY);
  int corp_id=0, gold=0, bankowner=0, bankgold=0;
  std::vector <corp_list_data> corp_list;
  corp_list_data corp_list_item;

  db.query(" \
select c.corp_id, c.name, sum(so.gold) as gold, b.talens as bankgold, \
  count(so.shop_nr) as shopcount, sob.corp_id as bankowner \
from \
  corporation c \
  left outer join shopownedcorpbank b \
    on (b.corp_id=c.corp_id and c.bank=b.shop_nr) \
  left outer join shopowned sob on (sob.shop_nr=c.bank) \
  left outer join shopowned so on (c.corp_id=so.corp_id) \
group by c.corp_id, c.name, b.talens, sob.corp_id \
order by sum(so.gold)+b.talens desc \
");
  
  while(db.fetchRow()){
    corp_id=convertTo<int>(db["corp_id"]);
    gold=convertTo<int>(db["gold"]);
    bankgold=convertTo<int>(db["bankgold"]);
    bankowner=convertTo<int>(db["bankowner"]);

    TCorporation corp(corp_id);
   
    // if we don't own the bank, record our gold that's in the bank
    // otherwise we end up counting it twice
    if(bankowner!=corp_id)
      gold += bankgold;

    corp_list_item.corp_id=corp_id;
    corp_list_item.name=db["name"];
    corp_list_item.gold=gold;
    corp_list_item.assets=corp.getAssets();
    corp_list_item.rank=gold+corp_list_item.assets;
    corp_list.push_back(corp_list_item);
  }

  return corp_list;
}


