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
  int getCorpID();

  void doBuyTransaction(int, const sstring &, const sstring &, TObj *obj=NULL);
  void doSellTransaction(int, const sstring &, const sstring &, TObj *obj=NULL);

  double getExpenseRatio();
  int doExpenses(int, TObj *);


  void setDividend(sstring);
  double getDividend();
  int doDividend(int, const sstring &);

  void setReserve(sstring);
  int getMinReserve();
  int getMaxReserve();
  int doReserve();
  int chargeTax(int, const sstring &, TObj *);

  // repair specific
  double getQuality();
  void setQuality(sstring);
  double getSpeed();
  void setSpeed(sstring);

  int getMaxNum(const TObj *);
  void showInfo();
  int setRates(sstring);
  int buyShop(sstring);
  int sellShop();
  int giveMoney(sstring);
  int setAccess(sstring);
  int doLogs(sstring);
  int setString(sstring);

  // accounting stuff
  void journalize(const sstring &, const sstring &, const sstring &, 
		  int, int, int, int);
  void journalize_debit(int post_ref, const sstring &customer,
			const sstring &name, int amt, bool new_id=false);
  void journalize_credit(int post_ref, const sstring &customer,
			 const sstring &name, int amt, bool new_id=false);
  void COGS_add(const sstring &name, int amt);
  void COGS_remove(const sstring &name);
  int COGS_get(const sstring &name);


  TShopOwned(int, TMonster *, TBeing *);
  ~TShopOwned();
};


#endif
