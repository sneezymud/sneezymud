//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_zones.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include <algorithm>

#include "stdsneezy.h"

class zoneSorter {
  public:
    float avgLevel;
    string zoneName;
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

void TBeing::doZonesSingle(string tStString)
{
  unsigned int tZone;
  char         tString[256] = "\0",
               tBuffer[256] = "\0";
  string       tStBuffer(""),
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

    // strip up the zone creator info
    char *s = strrchr(tString, '-');

    if (s)
      *s = '\0';

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

    if (file_to_string(tString, tStTemp, true)) {
      tStBuffer += "\n\r";
      tStBuffer += tStTemp;
    }
  }

  desc->page_string(tStBuffer.c_str(), 0);
}

void TBeing::doZones(string tStString)
{
  if (!tStString.empty()) {
    doZonesSingle(tStString);
    return;
  }

  unsigned int zone;
  string str;
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

    // strip up the zone creator info
    char *s = strrchr(buf, '-');
    if (s)
      *s = '\0';

    float avg = (zd.num_mobs ? zd.mob_levels/zd.num_mobs : 0);
    sprintf(buf2, "%-30.30s : Level: avg: %3.0f, min: %3.0f, max %3.0f\n\r",
         buf, avg,
         zd.min_mob_level, zd.max_mob_level);

    sortZoneVec.push_back(zoneSorter(avg, buf2));
  }

  // sort the vector
  sort(sortZoneVec.begin(), sortZoneVec.end(), zoneSorter());

  // list is sorted by avg level, cat it all together now
  for (zone = 0; zone < sortZoneVec.size(); zone++) {
    char tString[256];
    sprintf(tString, "%s%s<z>", colorStrings[cIndex], sortZoneVec[zone].zoneName.c_str());
    str += tString;

    if (++cIndex == 8)
      cIndex = 0;
  }

  desc->page_string(str.c_str(), 0);
}
