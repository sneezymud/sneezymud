//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __SHOP_H
#define __SHOP_H

const unsigned int SHOPACCESS_OWNER   = (1<<0);
const unsigned int SHOPACCESS_INFO    = (1<<1);
const unsigned int SHOPACCESS_RATES   = (1<<2);
const unsigned int SHOPACCESS_GIVE    = (1<<3);
const unsigned int SHOPACCESS_SELL    = (1<<4);
const unsigned int SHOPACCESS_ACCESS  = (1<<5);
const unsigned int SHOPACCESS_LOGS    = (1<<6);
const unsigned int SHOPACCESS_DIVIDEND= (1<<7);

const char * const SHOP_FILE =     "tinyworld.shp";
const char * const SHOPFILE_PATH = "mobdata/shops";
const char * const SHOP_PRICING  = "shop_pricing.dat";

const unsigned int SHOP_FLAG_RECYCLE             = (1<<0);
const unsigned int SHOP_FLAG_MAT_RESTRICTED      = (1<<1);

const unsigned int MAX_SHOP_INVENTORY = 2500;

struct shop_pricing {
  int obj_vnum;
  int num_sold;
  int rel_sold;
  int num_bought;
  int rel_bought;
  time_t last_touch_buy;
  time_t last_touch_sell;
};
const int SHOP_FILE_VERSION = 1;

// forward declaration
class shopData;

#if SHOP_PRICES_FLUXUATE
extern vector<shop_pricing>ShopPriceIndex;
#endif

extern vector<shopData>shop_index;
extern int cached_shop_nr;
extern map <int,float> ratios_cache;
extern map <sstring,float> matches_cache;

extern TObj *loadRepairItem(TBeing *, int, long &, int &, unsigned char &);

extern void factoryProduction(int);
extern void shoplog(int, TBeing *, TMonster *, const sstring &, int, const sstring &);
extern unsigned int find_shop_nr(int);


class shopData {
  public:
    int shop_nr;
    bool owned;
    vector <int> producing;       /* Which item to produce (virtual)      */
    float profit_buy;             /* Factor to multiply cost with.        */
    float profit_sell;            /* Factor to multiply cost with.        */
    vector<unsigned int>type;     /* Which item to trade.                 */
    vector <int> mat_type;       /* Material types to allow              */
    char *no_such_item1;          /* Message if keeper hasn't got an item */
    char *no_such_item2;          /* Message if player hasn't got an item */
    char *missing_cash1;          /* Message if keeper hasn't got cash    */
    char *missing_cash2;          /* Message if player hasn't got cash    */
    char *do_not_buy;             /* If keeper dosn't buy such things.    */
    char *message_buy;            /* Message when player buys item        */
    char *message_sell;           /* Message when player sells item       */
    int temper1;                  /* How does keeper react if no money    */
    int temper2;                  /* How does keeper react when attacked  */
    int keeper;                   /* The mobil who owns the shop (virtual) */
    unsigned int flags;           /* Who does the shop trade with?  */
    int in_room;                  /* Where is the shop?          */
    int open1, open2;             /* When does the shop open?       */
    int close1, close2;           /* When does the shop close?      */

    bool isProducing(const TObj *item);
    bool isOwned();
    bool willTradeWith(TMonster *keeper, TBeing *ch);
    bool willBuy(const TObj *item);
    float getProfitBuy(const TObj *, const TBeing *);
    float getProfitSell(const TObj *, const TBeing *);
    bool isRepairShop();

    shopData();
    ~shopData();
    shopData(const shopData &t);
    shopData & operator =(const shopData &t);
};

extern bool will_not_buy(TBeing *ch, TMonster *keeper, TObj *temp1, int);
extern void waste_shop_file(int shop_nr);
#endif
