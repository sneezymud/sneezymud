//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"

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

   vlogf(LOG_BUG, fmt("Bad call to CountBits (%d)") %  Class);
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

int TBeing::getClassNum(const char *arg, exactTypeT exact)
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

int TBeing::getClassNum(classIndT arg)
{
  return classInfo[arg].class_num;
}


classIndT TBeing::getClassIndNum(const char *arg, exactTypeT exact)
{
  int which = getClassNum(arg, exact);
  classIndT res = MIN_CLASS_IND;

  for(classIndT i=MIN_CLASS_IND;i<MAX_CLASSES;i++){
    if(classInfo[i].class_num==which){
      res=i;
      break;
    }
  }
      
  if(res==MIN_CLASS_IND) {
    vlogf(LOG_BUG, "unknown class result");
  }

  return res;
}


classIndT TBeing::getClassIndNum(ush_int which, exactTypeT exact)
{
  classIndT res = MIN_CLASS_IND;

  for(classIndT i=MIN_CLASS_IND;i<MAX_CLASSES;i++){
    if(classInfo[i].class_num==which){
      res=i;
      break;
    }
  }
      
  if(res==MIN_CLASS_IND) {
    vlogf(LOG_BUG, "unknown class result");
  }

  return res;
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

bool TBeing::hasClass(ush_int bit, exactTypeT exact) const
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

  return 1;

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
  register int riMax = 0;
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

  // there really isn't much need to be hardcoding people here, as we don't
  // do pwipes or setup new muds very often (or ever, really)
  // it just ends up causing problems

  if (!strcmp(name, "Peel")){
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

void TBeing::setLevel(classIndT i, ubyte lev)
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
  buf.erase(buf.size()-1, buf.size()); // take off that trailing /

  return buf;
}

const char * const TBeing::getProfAbbrevName() const
{
  if (hasClass(CLASS_MAGE | CLASS_WARRIOR | CLASS_CLERIC | CLASS_MONK | CLASS_THIEF, EXACT_YES))
    return "Thief";

  if (hasClass(CLASS_MAGE | CLASS_WARRIOR | CLASS_THIEF, EXACT_YES))
    return "M/W/T";
  else if (hasClass(CLASS_MAGE | CLASS_WARRIOR, EXACT_YES))
    return "M/W";
  else if (hasClass(CLASS_MAGE | CLASS_THIEF, EXACT_YES))
    return "M/T";
  else if (hasClass(CLASS_MAGE, EXACT_YES))
    return "Mage";
  else if (hasClass(CLASS_CLERIC | CLASS_WARRIOR | CLASS_THIEF, EXACT_YES))
    return "C/W/T";
  else if (hasClass(CLASS_CLERIC | CLASS_WARRIOR, EXACT_YES))
    return "C/W";
  else if (hasClass(CLASS_CLERIC | CLASS_THIEF, EXACT_YES))
    return "C/T";
  else if (hasClass(CLASS_CLERIC, EXACT_YES))
    return "Cler";
  else if (hasClass(CLASS_WARRIOR | CLASS_THIEF, EXACT_YES))
    return "W/T";
  else if (hasClass(CLASS_WARRIOR, EXACT_YES))
    return "Warr";
  else if (hasClass(CLASS_THIEF, EXACT_YES))
    return "Thief";
  else if (hasClass(CLASS_DEIKHAN, EXACT_YES))
    return "Deik";
  else if (hasClass(CLASS_MONK, EXACT_YES))
    return "Monk";
  else if (hasClass(CLASS_SHAMAN, EXACT_YES))
    return "Sham";
  else if (hasClass(CLASS_RANGER, EXACT_YES))
    return "Rang";
  else
    return "";
}

void TBeing::setClass(ush_int num)
{
  player.Class = num;
}
