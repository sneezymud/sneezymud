#include "stdsneezy.h"
#include "database.h"
#include "corporation.h"


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

  db.query("update shopownedcorpbank set talens=%i where corp_id=corporation.corp_id and corporation.corp_id=%i and shop_nr=corporation.bank", g, corp_id);
}

int TCorporation::getCorpID()
{
  return corp_id;
}
