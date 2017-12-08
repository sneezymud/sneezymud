//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#include "tweaks.h"
#include "database.h"
#include "being.h"


tweakEntry::tweakEntry(double v, const sstring n, const sstring d) :
  value(v),
  name(n),
  descr(d)
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

void tweakInfoT::loadTweaks()
{
  TDatabase db(DB_SNEEZY);

  db.query("select type, value, name, desc from gtweaks order by name");
  tweaks[TWEAK_NONE] = new tweakEntry(0, "none", "none");
/*
  while(db.fetchRow()){
    tweakTypeT tweak_id=(tweakTypeT) convertTo<int>(db["type"]);
    double val=(double) convertTo<double>(db["value"]);
    tweaks[tweak_id] = new tweakEntry(val, db["name"], db["desc"]);
  }
  */
  loaded=true;
}

tweakInfoT tweakInfo;

void TBeing::doTweak(const char *arg2)
{
  char arg[256];
  arg2=one_argument(arg2, arg, cElements(arg));

  //TPerson *ch = dynamic_cast<TPerson *>(this);
  
  if (!*arg) {
    sendTo(COLOR_BASIC, "\n\r<c>Global Tweaks<1>\n\r");
    sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
    sendTo(COLOR_BASIC, format("%u") % tweakInfo.tweaks.size());
    
    for( int a = 0; a < 3; a++ ) {
      if (tweakInfo.tweaks.at(a) != nullptr) { 
        sendTo(COLOR_BASIC, format("%-17s : %f        -%s") %
	      tweakInfo.tweaks.at(1)->name %
	      tweakInfo.tweaks.at(1)->value %
	      tweakInfo.tweaks.at(1)->descr);
      }
    }
    
    for(tweakTypeT t=TWEAK_NONE;t<MAX_TWEAK_TYPES;t++){
	    if(t==TWEAK_NONE)
	      continue;
	    //sendTo(COLOR_BASIC, format("%-17s : %f        -%s") %
	     // tweakInfo[t]->name %
	    //  tweakInfo[t]->value %
	     // tweakInfo[t]->descr);
    }
  }
}