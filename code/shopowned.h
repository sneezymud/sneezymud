#ifndef __SHOPOWNED_H
#define __SHOPOWNED_H

enum transactionTypeT {
  TX_BUYING,
  TX_BUYING_SERVICE,
  TX_RECYCLING,
  TX_SELLING,
  TX_PRODUCING,
  TX_GIVING_TALENS,
  TX_RECEIVING_TALENS,
  TX_PAYING_INTEREST,
  TX_WITHDRAWAL,
  TX_DEPOSIT,
  TX_FACTORY,
};


class TShopOwned {
  unsigned int shop_nr;
  TMonster *keeper;
  TBeing *ch;
  bool owned;
  int access;

 public:
  bool isOwned();
  bool hasAccess(int);
  int getPurchasePrice(int, int);
  int getCorpID();

  void doBuyTransaction(int, const sstring &, transactionTypeT, TObj *obj=NULL);
  void doSellTransaction(int, const sstring &, transactionTypeT, TObj *obj=NULL);

  double getExpenseRatio();
  int doExpenses(int, TObj *);

  TThing *getStuff();
  TMonster *getKeeper();

  void setDividend(sstring);
  double getDividend();
  int doDividend(int, const sstring &);

  void setReserve(sstring);
  int getMinReserve();
  int getMaxReserve();
  int doReserve();
  int chargeTax(int, const sstring &, TObj *);
  int getTaxShopNr();

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
  void giveStatements(sstring);
  void journalize(const sstring &, const sstring &, transactionTypeT, 
		  int, int, int, int);
  void journalize_debit(int post_ref, const sstring &customer,
			const sstring &name, int amt, bool new_id=false);
  void journalize_credit(int post_ref, const sstring &customer,
			 const sstring &name, int amt, bool new_id=false);
  void COGS_add(const sstring &name, int amt);
  void COGS_remove(const sstring &name);
  int COGS_get(const sstring &name);


  TShopOwned(int, TMonster *, TBeing *);
  TShopOwned(TMonster *, TBeing *);
  TShopOwned(int, TBeing *);
  TShopOwned(int);
  ~TShopOwned();
};


#endif
