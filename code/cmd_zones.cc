//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//  cmd_zones.cc : The "zones" command
//
//////////////////////////////////////////////////////////////////////////


#include <algorithm>

#include "stdsneezy.h"

class zoneSorter {
  public:
    float avgLevel;
    sstring zoneName;
    zoneSorter(float n, const char *s) :
      avgLevel(n), zoneName(s)
    {}
    zoneSorter() : avgLevel(0.0), zoneName("") {}
    bool operator() (const zoneSorter &, const zoneSorter &) const;
};

bool zoneSorter::operator()  (const zoneSorter &x, const zoneSorter &y) const
{
  return x.avgLevel < y.avgLevel;
}

void TBeing::doZonesSingle(sstring tStString)
{
  unsigned int tZone;
  char         tString[256] = "\0",
               tBuffer[256] = "\0";
  sstring       tStBuffer(""),
               tStTemp("");

  for (tZone = 0; tZone < zone_table.size(); tZone++) {
    // skip the void (0) : this is for misc. mobs
    // skip immort zone (1)
    if (tZone == 0 || tZone == 1)
      continue;

    zoneData &zd = zone_table[tZone];

    if (!zd.enabled || !zd.num_mobs || !is_abbrev(tStString, zd.name))
      continue;

    strcpy(tString, zd.name);

#if 0
    // strip up the zone creator info
    char *s = strrchr(tString, '-');

    if (s)
      *s = '\0';
#endif

    float avg = (zd.num_mobs ? zd.mob_levels/zd.num_mobs : 0);
    sprintf(tBuffer, "%-30.30s : Level: avg: %3.0f, min: %3.0f, max %3.0f\n\r",
         tString, avg,
         zd.min_mob_level, zd.max_mob_level);

    break;
  }

  if (!tBuffer[0])
    return;

  tStBuffer += tBuffer;

  if (tZone >= 0 && tZone < zone_table.size()) {
    sprintf(tString, "zoneHelp/%d", (tZone > 0 ? (zone_table[tZone - 1].top + 1) : 0));

    if (file_to_sstring(tString, tStTemp)) {
      tStBuffer += "\n\r";
      tStBuffer += tStTemp;
    }
  }

  desc->page_string(tStBuffer);
}

void TBeing::doZones(sstring tStString)
{
  if (!tStString.empty()) {
    doZonesSingle(tStString);
    return;
  }

  unsigned int zone;
  sstring str;
  int    cIndex = 0;

  const char *colorStrings[] =
  {
    "<r>", "<g>", "<p>", "<k>",
    "<b>", "<c>", "<o>", "<z>"
  };

  if (!desc)
    return;

  str = "The level information presented here is based on statistical averaging of the\n\r";
  str += "creatures found therein, and may not accurately reflect the intended level\n\r";
  str += "for players.  The levels shown may also fluxuate from moment to moment.\n\r";
  str += "\n\r";

  vector<zoneSorter>sortZoneVec(0);

  for (zone = 0; zone < zone_table.size(); zone++) {
    // skip the void (0) : this is for misc. mobs
    // skip immort zone (1)
    if (zone == 0 ||
        zone == 1)
      continue;

    zoneData &zd = zone_table[zone];
    if (!zd.enabled)
      continue;

    if (!zd.num_mobs)
      continue;

    char buf[256], buf2[256];
    strcpy(buf, zd.name);

    char *s = strchr(buf, '-');
    char *n = buf;
    if (s){
      --s; // get the space before the -
      *s = '\0'; // after builder name
      s+=2; // get the space after the -
      *s = '\0'; // after zone name
      ++s;
    } else {
      s=buf;
      n=NULL;
    }
    // buf is now the builder name, s is the zone name
    float avg = (zd.num_mobs ? zd.mob_levels/zd.num_mobs : 0);

    double x, total=0, dev, minlev=1000, maxlev=-1;
    int virt, count=0;


    for(unsigned int mobnum=0;mobnum<mob_index.size();mobnum++){
      virt=mob_index[mobnum].virt;

      if(virt > zone_table[zone-1].top && virt <=  zone_table[zone].top &&
	 mob_index[mobnum].doesLoad){
	count += mob_index[mobnum].getMaxNumber();
	x=(mob_index[mobnum].level * mob_index[mobnum].getMaxNumber());
	
	if(mob_index[mobnum].level>maxlev)
	  maxlev=mob_index[mobnum].level;
	if(mob_index[mobnum].level<minlev)
	  minlev=mob_index[mobnum].level;

	x-=avg;
	x*=x;
	total+=x;

      }
    }
    total/=(count-1);
    dev=sqrt(total);
    
    total=count=0;
    // now go through the mobs again and average up only a few
    for(unsigned int mobnum=0;mobnum<mob_index.size();mobnum++){
      virt=mob_index[mobnum].virt;

      if(virt > zone_table[zone-1].top && virt <=  zone_table[zone].top
	 && mob_index[mobnum].doesLoad){
	if(mob_index[mobnum].level < avg+dev &&
	   mob_index[mobnum].level > avg-dev){
	  total+=(mob_index[mobnum].level * mob_index[mobnum].getMaxNumber());
	  count+=mob_index[mobnum].getMaxNumber();
	}
      }
    }
    
    if(count> 0 && total > 0)
      avg = total/count;

    if(minlev==1000)
      minlev=zd.min_mob_level;

    if(maxlev==-1)
      maxlev=zd.max_mob_level;

    sprintf(buf2, "%-25.25s : %-10.10s : Level: avg: %i, min: %3.0f, max %3.0f\n\r",
         s, n?n:"", (int)avg,
	    minlev, maxlev);

    sortZoneVec.push_back(zoneSorter(avg, buf2));
  }

  // sort the vector
  sort(sortZoneVec.begin(), sortZoneVec.end(), zoneSorter());

  // list is sorted by avg level, cat it all together now
  int lastavg=-1;
  for (zone = 0; zone < sortZoneVec.size(); zone++) {
    char tString[256];

    if((int)(sortZoneVec[zone].avgLevel) != lastavg){
      if (++cIndex == 8)
	cIndex = 0;
      lastavg=(int)(sortZoneVec[zone].avgLevel);
    }
    
    sprintf(tString, "%s%s<z>", colorStrings[cIndex], sortZoneVec[zone].zoneName.c_str());
    str += tString;
  }

  desc->page_string(str);
}
