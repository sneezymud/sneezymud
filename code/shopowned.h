#ifndef __SHOPOWNED_H
#define __SHOPOWNED_H

class TShopOwned {
  int shop_nr;
  TMonster *keeper;
  TBeing *ch;
  bool owned;
  int access;

 public:
  bool isOwned();
  bool hasAccess(int);
  int getPurchasePrice(int, int);

  int getMaxNum(const TObj *);
  void showInfo();
  int setRates(sstring);
  int buyShop();
  int sellShop();
  int giveMoney(sstring);
  int setAccess(sstring);
  int doLogs(sstring);

  TShopOwned(int, TMonster *, TBeing *);
  ~TShopOwned();
};


#endif
