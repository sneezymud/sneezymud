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



int TCorporation::getMoney()
{
  TDatabase db(DB_SNEEZY);

  db.query("select gold from corporation where corp_id=%i", corp_id);

  if(db.fetchRow())
    return convertTo<int>(db["gold"]);
  else
    return 0;
}


void TCorporation::setMoney(int g)
{
  TDatabase db(DB_SNEEZY);

  db.query("update corporation set gold=%i where corp_id=%i", g, corp_id);
}

int TCorporation::getCorpID()
{
  return corp_id;
}
