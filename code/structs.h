#ifndef __STRUCTS_H
#define __STRUCTS_H

#ifndef _TIME_H
#include <time.h>
#endif

#include "parse.h"

// forward declarations
class TObj;
class TBeing;
class TMonster;
class TPerson;
class TRoom;
class TThing;
class Craps;
class Descriptor;
class TAccount;
class affectedData;
class saveAffectedData;
class roomDirData;
class TFuel;
class TLight;
class TBaseLight;
class TDrug;
class TDrugContainer;
class TFood;
class TEgg;
class TPen;
class TOpal;
class TPortal;
class TOrganic;
class TFFlame;
class TSmoke;
class TASubstance;
class TTrap;
class TKey;
class TTool;
class TComponent;
class TSymbol;
class TVial;
class TPCorpse;
class TMagicItem;
class TDrinkCon;
class TBaseCup;
class TBaseWeapon;
class TArrow;
class TBow;
class TBaseCorpse;
class TTable;
class TBaseContainer;
class TOpenContainer;
class boardStruct;
class liqInfoT;

const int MAX_BUF_LENGTH              = 240;

typedef signed char sbyte;
typedef unsigned char ubyte;
typedef short int sh_int;
typedef unsigned short int ush_int;
typedef signed char byte;

extern void vlogf(logTypeT, const sstring &);
extern char * mud_str_dup(const char *buf);
extern char * mud_str_dup(const sstring &buf);
extern char * mud_str_copy(char *dest, const sstring &src, size_t n);

class time_info_data
{
  public:
    byte seconds, minutes, hours, day, month;
    sh_int year;

  time_info_data();
  time_info_data(const time_info_data &a);
  time_info_data & operator=(const time_info_data &a);
  ~time_info_data();
};

struct time_data
{
  time_t birth;    /* This represents the characters age                */
  time_t logon;    /* Time of the last logon (used to calculate played) */
  int played;      /* This is the total accumulated time played in secs */
  time_t last_logon;
};

class lag_data
{
  public:
    time_t high;
    time_t low;
    time_t total;
    time_t current;
    unsigned long count;
    time_t lagstart[10];
    time_t lagtime[10];
    unsigned long lagcount[10];

  lag_data() :
    high(0),
    low(99999),
    total(0),
    current(0),
    count(0)
  {
    memset(&lagstart, 0, sizeof(lagstart));
    memset(&lagtime, 0, sizeof(lagtime));
    memset(&lagcount, 0, sizeof(lagcount));
  }
};
extern lag_data lag_info;

const bool TRUE = true;
const bool FALSE = false;

extern time_info_data time_info;

const int BIT_POOF_IN  = 1;
const int BIT_POOF_OUT = 2;

const int WAIT_SEC     = 4;
const int WAIT_ROUND   = 4;

const unsigned int MAX_STRING_LENGTH   = 32000;
const int MAX_INPUT_LENGTH    = 1024;
const int MAX_MESSAGES        =  60;
const int MAX_ITEMS           = 15;

const int MESS_ATTACKER = 1;
const int MESS_VICTIM   = 2;
const int MESS_ROOM     = 3;

const int SECS_PER_REAL_MIN  = 60;
const int SECS_PER_REAL_HOUR  = (60*SECS_PER_REAL_MIN);
const int SECS_PER_REAL_DAY   = (24*SECS_PER_REAL_HOUR);
const int SECS_PER_REAL_YEAR  = (365*SECS_PER_REAL_DAY);

const int INCHES_PER_FOOT    = 12;
const int FEET_PER_YARD      = 3;
const int INCHES_PER_YARD = INCHES_PER_FOOT * FEET_PER_YARD;
const int CUBIC_INCHES_PER_FOOT = INCHES_PER_FOOT * INCHES_PER_FOOT * INCHES_PER_FOOT;
const int CUBIC_INCHES_PER_YARD = INCHES_PER_YARD * INCHES_PER_YARD * INCHES_PER_YARD;

const int MIN_GLOB_TRACK_LEV = 31;   /* mininum level for global track */
const int MAX_BAN_HOSTS = 15;

// pet information
const unsigned int PETTYPE_PET    = (1<<0);   // 1
const unsigned int PETTYPE_CHARM  = (1<<1);   // 2
const unsigned int PETTYPE_THRALL = (1<<2);   // 4

struct show_room_zone_struct {
  int blank;
  int startblank, lastblank;
  int bottom, top;
  sstring sb;
};

class snoopData {
  public:
    TBeing *snooping;  // Who am I snooping
    TBeing *snoop_by;  // Who is snooping me
    snoopData();
    snoopData(const snoopData &a);
    snoopData & operator=(const snoopData &a);
    ~snoopData();
};

class aliasData {
  public:
    char word[12];     // Word for new alias
    char command[30];  // Command to be aliased
    aliasData();
    aliasData(const aliasData &a);
    aliasData & operator=(const aliasData &a);
    ~aliasData();
};

class betData {
  public:
    int come;
    int crap;
    int slot;
    int eleven;
    int twelve;
    int two;
    int three;
    int horn_bet;
    int field_bet;
    int hard_eight;
    int hard_six;
    int hard_ten;
    int hard_four;
    int seven;
    int one_craps;

    betData();
    betData(const betData &a);
    betData & operator=(const betData &a);
    ~betData();
};

class cBetData {
  public:
    unsigned int crapsOptions;
    unsigned int oneRoll;
    unsigned int roulOptions;

    cBetData();
    cBetData(const cBetData &a);
    cBetData & operator=(const cBetData &a);
    ~cBetData();
};

class lastChangeData {
  public:
    sh_int hit;
    sh_int mana;
    sh_int move;
    double piety;
    int money;
    double exp;
    int room;
    double perc;
    int mudtime;
    int minute;
    int fighting;
    byte full, thirst, pos;
    sh_int lifeforce;
    lastChangeData();
    lastChangeData(const lastChangeData &a);
    ~lastChangeData();
};

class objAffData {
  public:
    spellNumT type;           /* The type of spell that caused this      */
    sbyte level;          /* The level of the affect                 */
    int duration;         /* For how long its effects will last      */
    int renew;            /* at what duration it can be reuned       */
    applyTypeT location;  /* Tells which ability to change(APPLY_XXX)*/
    long modifier;        /* This is added to apropriate ability     */
    long modifier2;
    long bitvector;       /* Tells which bits to set (AFF_XXX)       */

    objAffData();
    objAffData(const objAffData &a);
    objAffData & operator=(const objAffData &a);
    ~objAffData();

    void checkForBadness(TObj *);
};

class roomDirData {
  public:
    const char *description;         // What you see when you look at the direction
    const char *keyword;             // keyword for opening and closing doors
    doorTypeT door_type;           // type of door
    unsigned int condition;           // bitvector for door status
    byte lock_difficulty;      // how hard to open, -1 = unlockable
    byte weight;               // how heavy door is, -1 = no door
    byte trap_info;            // Trap flags
    sh_int trap_dam;           // Damage trap will do
    int key;                // Number of object that opens door
    int to_room;            // What room we exit to. -1 means no exit

    void destroyDoor(dirTypeT, int);
    void caveinDoor(dirTypeT, int);
    void wardDoor(dirTypeT, int);
    doorTypeT doorType() { return door_type; };
    int destination() { return to_room; };
    const sstring getName() const;
    const sstring closed() const;

    roomDirData();
    roomDirData(const roomDirData &a);
    roomDirData & operator=(const roomDirData &a);
    ~roomDirData();
};

class wizListInfo {
  public:
    char *buf1;
    char *buf2;
    char *buf3;
    /*
    char *buf60;
    char *buf59;
    char *buf58;
    char *buf57;
    char *buf56;
    char *buf55;
    char *buf54;
    char *buf53;
    char *buf52;
    char *buf51;
     */

    wizListInfo();
    ~wizListInfo();
};

const int PERMANENT_DURATION = -9;   // for affectedData.duration

class affectedData {
 public:
    spellNumT type;
    sbyte level;
    int duration;
    int renew;
    long modifier;
    long modifier2;
    applyTypeT location;
    unsigned long bitvector;
    TThing *be;
    affectedData *next;

    affectedData();
    affectedData(const affectedData &a);
    affectedData(const saveAffectedData &a);
    affectedData & operator=(const affectedData &a);
    ~affectedData();

    bool canBeRenewed() const;
    bool shouldGenerateText() const;
};

// affects size of charFile
class saveAffectedData {
 public:
    short type;
    sbyte level;
    int duration;
    int renew;
    long modifier;
    long modifier2;
    byte location;
    unsigned long bitvector;
    void *unused1;  // should be removed 
    void *unused2;

    saveAffectedData();
    saveAffectedData & operator=(const affectedData &a);
};

#if 1
template<class T>
inline bool IS_SET(T a, const T b) { return ((a & b) != 0); }

template<class T>
inline bool IS_SET_ONLY(T a, const T b) { return ((a & b) == b); }

template<class T>
inline bool IS_SET_DELETE(T a, const T b) { return ((a & b) == b); }

template<class T>
inline void SET_BIT(T& a, const T& b) { a |= b; }

template<class T>
inline void REMOVE_BIT(T& a, const T& b) { a &= ~b; }

template<class T>
inline void ADD_DELETE(T& a, const T& b) { a |= b; }

template<class T>
inline void REM_DELETE(T& a, const T& b) { a &= ~b; }
#else
inline bool IS_SET(int a, const int b) { return ((a & b) != 0); }

inline bool IS_SET_ONLY(int a, const int b) { return ((a & b) == b); }

inline bool IS_SET_DELETE(int a, const int b) { return ((a & b) == b); }

inline void SET_BIT(int& a, const int& b) { a |= b; }
inline void SET_BIT(unsigned short& a, const unsigned short& b) { a |= b; }
inline void SET_BIT(unsigned int& a, const unsigned int& b) { a |= b; }
inline void SET_BIT(unsigned long& a, const unsigned long& b) { a |= b; }

inline void REMOVE_BIT(int& a, const int& b) { a &= ~b; }
inline void REMOVE_BIT(unsigned short& a, const unsigned short& b) { a &= ~b; }
inline void REMOVE_BIT(unsigned int& a, const unsigned int& b) { a &= ~b; }
inline void REMOVE_BIT(unsigned long& a, const unsigned long& b) { a &= ~b; }

inline void ADD_DELETE(int& a, const int& b) { a |= b; }

inline void REM_DELETE(int& a, const int& b) { a &= ~b; }
#endif

inline int GET_BITS(int a, int p, int n) 
{
// return ((a >> (p + 1 - n)) & ~(~0 << n)); }
  int x, y;
  x = (a >> (p+1-n));
  y = ~(~0 <<n);
  return x & y;
}

inline void SET_BITS(int& a, int p, int n, const int y)
{
// a = ((a & ~(~(~0 << n) << (p+1-n))) | (y << (p+1-n))); }
  int q,r,s,x;
  q = (~0 << n);
  r = (~q << (p+1-n));
  s = (a & ~r);
  x = (y <<(p+1-n));
  a = (s | x);
  return;
}

#endif  // __STRUCTS_H inclusion sandwich
