//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#include "tweaks.h"
#include "database.h"
#include "being.h"


tweakEntry::tweakEntry(double v, double t, double r) :
  cvalue(v),
  tvalue(t),
  rate(r)
{
}

tweakEntry::~tweakEntry()
{
}

tweakEntry *tweakInfoT::operator[] (const tweakTypeT t)
{
  return tweaks.at(t);
}

tweakTypeT & operator++(tweakTypeT &c, int)
{
  return c = (c == MAX_TWEAK_TYPES) ? TWEAK_NONE : tweakTypeT(c+1);
}

tweakInfoT::~tweakInfoT()
{
}

tweakInfoT::tweakInfoT(): 
    tweaks(MAX_TWEAK_TYPES + 1, nullptr), 
    loaded(false)
{
  loaded=false;
}

int tweakInfoT::loadTweaks()
{
  TDatabase db(DB_SNEEZY);

  db.query("select tweak_type, tweak_value, tweak_target, tweak_rate from globaltweaks order by tweak_type");
  tweaks[TWEAK_NONE] = new tweakEntry(0, 0, 0);

  int i = 0;

  while(db.fetchRow()){
    tweakTypeT tweak_id=(tweakTypeT) convertTo<int>(db["tweak_type"]);
    double val=(double) convertTo<double>(db["tweak_value"]);
    double tar=(double) convertTo<double>(db["tweak_target"]);
    double rate=(double) convertTo<double>(db["tweak_rate"]);
    tweaks[tweak_id] = new tweakEntry(val, tar, rate);
    i++;
  }

  loaded=true;
  return i;
}

tweakInfoT tweakInfo;

void TBeing::doTweak(const char *arg2)
{
  char arg[256];
  arg2=one_argument(arg2, arg, cElements(arg));
  TDatabase db(DB_SNEEZY);
  //TPerson *ch = dynamic_cast<TPerson *>(this);

  if (!*arg) {
    sendTo(COLOR_BASIC, "\n\r<c>Global Tweaks<1>\n\r");
    sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
    sendTo(COLOR_BASIC, "<c>Tweak---------Current Value-------Target--------Change Rate------------------<1>\n\r");
    //sendTo(COLOR_BASIC, format("Loaded %d tweaks\n\r") % tweakInfo.loadTweaks());
    
    for(tweakTypeT t=TWEAK_NONE;t<MAX_TWEAK_TYPES;t++){
	    if(t==TWEAK_NONE)
	      continue; 
      sendTo(COLOR_BASIC, format("%-17s : %-10f : %-10f : %-10f ") %
      tweakInfo.getTweakName(t) %
	    tweakInfo[t]->cvalue %
	    tweakInfo[t]->tvalue %
	    tweakInfo[t]->rate);
    }
  }
}

const sstring tweakInfoT::getTweakName(tweakTypeT tt)
{
  // powers command truncs at 20 chars
  switch (tt) {
    case TWEAK_NONE:
    return "none";
    case TWEAK_LOADRATE:
      return "Load Rate:";
    case TWEAK_BURNRATE:
      return "Burn Rate:";
    case MAX_TWEAK_TYPES:
      return "";
  }
  return "";
}