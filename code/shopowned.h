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

  void showInfo();
  int setRates(string);
  int buyShop();
  int sellShop();
  int giveMoney(const char *);
  int setAccess(const char *);
  int doLogs(const char *);

  TShopOwned(int, TMonster *, TBeing *);
  ~TShopOwned();
};


#endif
