//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "extern.h"
#include "being.h"
#include "database.h"
#include "person.h"

#include <unordered_map>

int NumClasses(int Class)
{
  int tot = 0;

  for(classIndT i=MIN_CLASS_IND;i<MAX_CLASSES;i++)
    if(Class & classInfo[i].class_num)
      tot++;

  return(tot);
}

// originally developed for multiclass stuff, but now more generalized
unsigned int CountBits(unsigned int Class)
{
   if (Class == (1<<0))
       return (1);
   if (Class == (1<<1))
      return (2);
   if (Class == (1<<2))
      return (3);
   if (Class == (1<<3))
      return (4);
   if (Class == (1<<4))
      return (5);
   if (Class == (1<<5))
      return (6);
   if (Class == (1<<6))
      return (7);
   if (Class == (1<<7))
      return (8);
   if (Class == (1<<8))
      return (9);
   if (Class == (1<<9))
      return (10);
   if (Class == (1<<10))
      return (11);
   if (Class == (1<<11))
      return (12);
   if (Class == (1<<12))
      return (13);
   if (Class == (1<<13))
      return (14);
   if (Class == (1<<14))
      return (15);
   if (Class == (1<<15))
      return (16);
   if (Class == (1<<16))
      return (17);
   if (Class == (1<<17))
      return (18);
   if (Class == (1<<18))
      return (19);
   if (Class == (1<<19))
      return (20);
   if (Class == (1<<20))
      return (21);
   if (Class == (1<<21))
      return (22);

   vlogf(LOG_BUG, format("Bad call to CountBits (%d)") %  Class);
   return FALSE;
}

int TBeing::getClassLevel(int Class) const
{
   if (getClass() & Class) {
       return (getLevel(classIndT(CountBits(Class) - 1)));
   }
   return (0);
}

int TBeing::onlyClass(int Class) const
{
   int i;

   for (i = 1; i <= (1<<(MAX_CLASSES-1)); i *= 2) {
      if (getClassLevel(i) != 0)
         if (i != Class)
            return FALSE;
   }
   return TRUE;
}

bool TBeing::isSingleClass() const
{
   int i;

   for (i = 1; i <= (1<<(MAX_CLASSES-1)); i *= 2) {
      if (onlyClass(i))
         return TRUE;
   }
   return FALSE;
}

bool TBeing::isDoubleClass() const
{
  return NumClasses(getClass()) == 2;
} 

bool TBeing::isTripleClass() const
{
  return NumClasses(getClass()) >= 3;
}

int TBeing::getClassNum(const char *arg, exactTypeT exact) const
{
  int which = 0;

  if (exact) {
    for(classIndT i=MIN_CLASS_IND;i<MAX_CLASSES;i++){
      if(!strcmp(arg, classInfo[i].name.c_str())){
	which=classInfo[i].class_num;
	break;
      }
    }
      
    if(!which)
      return FALSE;
  } else {
    for(classIndT i=MIN_CLASS_IND;i<MAX_CLASSES;i++){
      if(is_abbrev(arg, classInfo[i].name)){
	which=classInfo[i].class_num;
	break;
      }
    }
      
    if(!which)
      return FALSE;
  }
  return which;
}

int TBeing::getClassNum(classIndT arg) const
{
  return classInfo[arg].class_num;
}


classIndT TBeing::getClassIndNum(const char *arg, exactTypeT exact) const {
  auto which = static_cast<unsigned short>(getClassNum(arg, exact));

  return getClassIndNum(which);
}

classIndT TBeing::getClassIndNum(unsigned short which) const {
  for (classIndT i = MIN_CLASS_IND; i < MAX_CLASSES; i++)
    if (classInfo[i].class_num == which)
      return i;

  return MAX_CLASSES;
}


bool TBeing::hasClass(const char *arg, exactTypeT exact) const
{
  int which=0;

  if (exact) {
    for(classIndT i=MIN_CLASS_IND;i<MAX_CLASSES;i++){
      if(!strcmp(arg, classInfo[i].name.c_str())){
	which=classInfo[i].class_num;
	break;
      }
    }
      
    if(!which)
      return FALSE;
  } else {
    for(classIndT i=MIN_CLASS_IND;i<MAX_CLASSES;i++){
      if(is_abbrev(arg, classInfo[i].name)){
	which=classInfo[i].class_num;
	break;
      }
    }
      
    if(!which)
      return FALSE;
  }

  if (getClass() & which)
    return TRUE;

  return FALSE;

}

bool TBeing::hasClass(unsigned short bit, exactTypeT exact) const
{
  if (!exact) {
    if (getClass() & bit) 
      return true;
  } else {
    if ((getClass() & bit) == bit)
      return true;
  }
  
  return false;
}

int TBeing::howManyClasses() const
{

  short tot = 0;
  classIndT i;

  for (i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
    if (getLevel(i))
      tot++;
  }
  if (tot)
    return (tot);
  else
    return NumClasses(getClass());
}

void TBeing::calcMaxLevel()
{
  int riMax = 0;
  classIndT i;

  for (i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
    if (getLevel(i) > riMax)
      riMax = getLevel(i);
  }
  setMaxLevel(riMax);  
}

int TBeing::GetTotLevel() const
{
  int tot = 0;
  classIndT i;
  for (i = MIN_CLASS_IND; i < MAX_CLASSES; i++)
    tot = getLevel(i);
  return tot;
}

void TPerson::startLevels()
{
  for(classIndT i=MIN_CLASS_IND;i<MAX_CLASSES;i++){
    if(hasClass(classInfo[i].class_num))
      advanceLevel(i);
  }

  // The first player automatically gets admin powers
  TDatabase db(DB_SNEEZY);
  db.query("select count(1) as num from player");
  assert(db.fetchRow());
  if(db["num"] == "1") {
    setLevel(MAGE_LEVEL_IND, MAX_IMMORT);
    setLevel(CLERIC_LEVEL_IND, MAX_IMMORT);
    setLevel(THIEF_LEVEL_IND, MAX_IMMORT);
    setLevel(WARRIOR_LEVEL_IND, MAX_IMMORT);
    setExp(2000000000);
    setWizPowers(this,this,"allpowers");
    remWizPower(POWER_IDLED);
    calcMaxLevel();
  }

  if (GetMaxLevel() > MAX_MORT) {
    // basically, if we are an autoleveleded person
    wizPowerT wpt;
    for (wpt = MIN_POWER_INDEX; wpt < MAX_POWER_INDEX; wpt++)
      setWizPower(wpt);
  }
}

classIndT TBeing::bestClass() const
{
  int iMax=0;
  classIndT Class=MIN_CLASS_IND, i;

  for (i=MIN_CLASS_IND; i < MAX_CLASSES; i++) {
    if (iMax < getLevel(i)) {
      iMax = getLevel(i);
      Class = i;
    }
  }
  return(Class);
}

void TBeing::setLevel(classIndT i, unsigned short lev)
{
  if (i >= MAX_CLASSES) {
    vlogf(LOG_BUG, "Bad class value");
    return;
  }

  player.level[i] = lev;
}

classIndT & operator++(classIndT &c, int)
{
  return c = (c == MAX_SAVED_CLASSES) ? MIN_CLASS_IND : classIndT(c+1);
}

sstring const TBeing::getProfName() const
{
  sstring buf;

  for(classIndT i=MIN_CLASS_IND;i<MAX_CLASSES;i++){
    if(hasClass(classInfo[i].class_num)){
      buf += classInfo[i].name.cap();
      buf += "/";
    }
  }
  if(buf.size()>0)
    buf.erase(buf.size()-1, buf.size()); // take off that trailing /

  return buf;
}

namespace {
  int count_set_bit(int n) {
    int count = 0;
    while(n != 0) {
      count += n & 1;
      n >>= 1;
    }
    return count;
  }
}

sstring TBeing::getProfAbbrevName(unsigned short code)
{
  sstring buf = "";
  bool multiclass = false;

  if (count_set_bit(code) > 1)
    multiclass = true;

  for (classIndT iClass = MIN_CLASS_IND; iClass < MAX_CLASSES; iClass++){
    if (code & (1<<iClass)) {
      if (!multiclass) {
        if (classInfo[iClass].name == "thief"){
          return classInfo[iClass].name.cap();
        }

        return sstring(classInfo[iClass].name.substr(0, 4)).cap();
      } else {
        buf += classInfo[iClass].abbr;
        buf += "/";
      }
    }
  }
  if (multiclass)
    buf = buf.substr(0, buf.size() - 1);
  return buf;
}

void TBeing::setClass(unsigned short num)
{
  player.Class = num;
}
