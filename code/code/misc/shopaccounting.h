#ifndef __SHOPACCOUNTING_H
#define __SHOPACCOUNTING_H

#include <map>

class TShopJournal {
  std::map <sstring, int> values;
  int shop_nr;
  int year;

 public:
  int getValue(const sstring &);

  int getExpenses();
  int getNetIncome();
  int getRetainedEarnings();
  int getAssets();
  int getLiabilities();
  int getShareholdersEquity();
  void closeTheBooks();

  TShopJournal(int);
  TShopJournal(int,int);
};

#endif
