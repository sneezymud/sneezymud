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
  int setRates(string);
  int buyShop();
  int sellShop();
  int giveMoney(string);
  int setAccess(string);
  int doLogs(string);

  TShopOwned(int, TMonster *, TBeing *);
  ~TShopOwned();
};


#endif
