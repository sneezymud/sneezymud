#ifndef __CORPORATION_H
#define __CORPORATION_H

class TCorporation {
  int corp_id;

 public:
  bool hasAccess(TBeing *,int);
  int getAccess(TBeing *);
  int getMoney();
  void setMoney(int);
  int getBank();
  void setBank(int);
  int getCorpID();
  sstring getName();

  void corpLog(const sstring &name, const sstring &action, int talens);

  TCorporation(int);
  ~TCorporation();
};



const unsigned int CORPACCESS_PARTNER   = (1<<0);
const unsigned int CORPACCESS_INFO    = (1<<1);
//const unsigned int CORPACCESS_RATES   = (1<<2);
const unsigned int CORPACCESS_GIVE    = (1<<3);
//const unsigned int CORPACCESS_SELL    = (1<<4);
const unsigned int CORPACCESS_ACCESS  = (1<<5);
const unsigned int CORPACCESS_LOGS  = (1<<6);



#endif
