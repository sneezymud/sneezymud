//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//  cmd_zones.cc : The "zones" command
//
//////////////////////////////////////////////////////////////////////////


#include <algorithm>

#include "stdsneezy.h"
#include "database.h"
#include "cmd_trophy.h"

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
  unsigned int iZone        = 0/*,
	       iZoneIter    = 0*/;
  char         tString[256] = "\0",
               tBuffer[256] = "\0",
               tStats[2048] = "\0";
  sstring      tStBuffer(""),
               tStTemp("");

  for (iZone = 0; iZone < zone_table.size(); iZone++) {
    // skip the void (0) : this is for misc. mobs
    // skip immort zone (1)
    if (iZone == 0 || iZone == 1)
      continue;

    zoneData & zd      = zone_table[iZone];
    int        iBottom = (iZone ? (zone_table[(iZone - 1)].top + 1) : 0);

    // If the string is not in this zone, just continue.
    if (!strstr(zd.name, tStString.c_str()))
      continue;

    // Now trim the zonename so it matches what is shown in 'zones'.
    char buf[256];

    strcpy(buf, zd.name);
    char *s = strchr(buf, '-');
    char *n = buf;

    if (s) {
      --s;       // get the space before the -
      *s = '\0';
      s += 2;    // get the space after the -
      *s = '\0'; // after zone name
      ++s;
    } else {
      s = buf;
      n = NULL;
    }

    if (!s || !zd.enabled || !zd.num_mobs || !is_abbrev(tStString, s))
      continue;

    strcpy(tString, zd.name);

    float avg = (zd.num_mobs ? zd.mob_levels / zd.num_mobs : 0);
    sprintf(tBuffer, "%-30.30s : Level: avg: %3.0f, min: %3.0f, max %3.0f\n\r", tString, avg, zd.min_mob_level, zd.max_mob_level);

    if (isPc() && trophy) {
      // { -- Deduce zone size --
/* removed at Damescena's request
        int iRCount = ((zd.top - zd.bottom) + 1);
        strcpy(tStats, "This zone is ");

        if (iRCount > 500)
          strcat(tStats, "Huge!");
        else if (iRCount > 200)
          strcat(tStats, "Large!");
        else if (iRCount > 100)
          strcat(tStats, "a good size.");
        else if (iRCount > 50)
          strcat(tStats, "medium sized.");
        else
          strcat(tStats, "small, but has a down home feeling...");

        strcat(tStats, "\n\r");
*/
      // }

      // { -- Deduce mobiles fought in this zone, by this player, for both distict fight count and average xp gain --
        unsigned int iCount  = 0,
                     iFought = 0;
        float        fXPAvg  = 0.0; // 1.00 = 100%
        TTrophy    & tTrophy = *trophy;
        TDatabase    db(DB_SNEEZY);

        db.query("select mobvnum, count from trophy where player_id=%i order by mobvnum", getPlayerID());

        while (1) {
          if (!db.fetchRow())
            break;

          int iVNum = convertTo<int>(db["mobvnum"]);

          if (!iVNum)
            continue;

          if (iVNum > zd.top)
            break;

          int iRNum = real_mobile(convertTo<int>(db["mobvnum"]));

          if (iRNum < 0) {
            vlogf(LOG_BUG, fmt("doZones detected bad mobvnum=%d for name='%s'") % iVNum % getName());
            continue;
          }

          if (!mob_index[iRNum].doesLoad)
            continue;

          if (convertTo<float>(db["count"]))
            for (unsigned int iRunner = 0; iRunner < mob_index.size(); iRunner++)
              if ((mob_index[iRunner].virt >= iBottom) && (mob_index[iRunner].virt <= zd.top) && mob_index[iRunner].doesLoad) {
                iCount++;

                fXPAvg += tTrophy.getExpModVal(tTrophy.getCount(mob_index[iRunner].virt), mob_index[iRunner].virt);

                if (tTrophy.getCount(mob_index[iRunner].virt) > 0)
                  iFought++;
              }

          break;
        }

        if (iCount) { // If there are no mobs here, don't bother with the below.
          double fFPer = ((iFought * 100) / iCount);

          if (fXPAvg)
            fXPAvg /= iCount;

          sprintf(&tStats[strlen(tStats)], "You have fought %u distict creatures in this zone\n\r", iFought);
          sprintf(&tStats[strlen(tStats)], "...which equates to %.0f%% of all the creatures found within it.\n\r", fFPer);
          strcat(tStats, "All in all you will get ");

          if (fXPAvg >= 1.0)
            strcat(tStats, "full");
          else if (fXPAvg >= .90)
            strcat(tStats, "most of the");
          else if (fXPAvg >= .70)
            strcat(tStats, "a lot of the");
          else if (fXPAvg >= .50)
            strcat(tStats, "a decent amount of the");
          else if (fXPAvg >= .20)
            strcat(tStats, "some of the");
          else if (fXPAvg >= .10)
            strcat(tStats, "a tiny amount of the");
          else if (fXPAvg)
            strcat(tStats, "a little of the");
          else
            strcat(tStats, "no");

          strcat(tStats, " experience from creatures in this zone.\n\r");
        }
      // }
    }

    break;
  }

  if (!tBuffer[0]) {
    unsigned int iZIndex = 0,
                 zone    = 0;

    // Might be a number, check it and find out.
    iZIndex = atoi(tStString.c_str());

    // Is a digit, find the appropriate zone here.
    vector <zoneSorter> sortZoneVec(0);

    for (zone = 0; zone < zone_table.size(); zone++) {
      // skip the void (0) : this is for misc. mobs
      // skip immort zone (1)
      if (zone == 0 || zone == 1)
	continue;

      zoneData &zd = zone_table[zone];

      if (!zd.enabled || !zd.num_mobs)
	continue;

      char buf[256];
      strcpy(buf, zd.name);

      char *s = strchr(buf, '-');
      char *n = buf;

      if (s) {
	--s;       // get the space before the -
	*s = '\0'; // after builder name
	s += 2;    // get the space after the -
	*s = '\0'; // after zone name
	++s;
      } else {
	s = buf;
	n = NULL;
      }

      // buf is now the builder name, s is the zone name
      float avg = (zd.num_mobs ? zd.mob_levels / zd.num_mobs : 0);

      double x, total = 0, dev, minlev = 1000, maxlev = -1;
      int virt, count = 0;

      for (unsigned int mobnum = 0; mobnum < mob_index.size(); mobnum++) {
	virt = mob_index[mobnum].virt;

	if(virt > zone_table[zone-1].top && virt <= zone_table[zone].top && mob_index[mobnum].doesLoad) {
	  count += mob_index[mobnum].getMaxNumber();
	  x=(mob_index[mobnum].level * mob_index[mobnum].getMaxNumber());

	  if(mob_index[mobnum].level>maxlev)
	    maxlev = mob_index[mobnum].level;

	  if(mob_index[mobnum].level<minlev)
	    minlev = mob_index[mobnum].level;

	  x -= avg;
	  x *= x;
	  total += x;
	}
      }
      total /= (count-1);
      dev = sqrt(total);

      total = count = 0;

      // now go through the mobs again and average up only a few
      for (unsigned int mobnum = 0; mobnum < mob_index.size(); mobnum++) {
	virt = mob_index[mobnum].virt;
	if (virt > zone_table[zone-1].top && virt <=  zone_table[zone].top && mob_index[mobnum].doesLoad) {
	  if(mob_index[mobnum].level < avg+dev && mob_index[mobnum].level > avg-dev) {
	    total+= (mob_index[mobnum].level * mob_index[mobnum].getMaxNumber());
	    count+= mob_index[mobnum].getMaxNumber();
	  }
	}
      }

      if (count > 0 && total > 0)
	avg = total / count;

      sortZoneVec.push_back(zoneSorter(avg, s));
    }

    // sort the vector
    sort(sortZoneVec.begin(), sortZoneVec.end(), zoneSorter());

    if ((iZIndex == 0) || (iZIndex >= sortZoneVec.size())) {
      iZIndex = (sortZoneVec.size() - 1);

      sendTo(fmt("Requested zone index is out of range.  Range is: 1, ..., %u\n\r") % iZIndex);
    } else
      doZonesSingle(sortZoneVec[(iZIndex - 1)].zoneName);

    return;
  }

  tStBuffer += tBuffer;

  if (iZone >= 0 && iZone < zone_table.size()) {
    sprintf(tString, "zoneHelp/%d", (iZone > 0 ? (zone_table[iZone - 1].top + 1) : 0));

    if (file_to_sstring(tString, tStTemp)) {
      tStBuffer += "\n\r";
      tStBuffer += tStTemp;
    }
  }

  if (strlen(tStats)) {
    tStBuffer += "\n\r";
    tStBuffer += tStats;
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
  sstring      str;
  int          cIndex = 0;

  const char *colorStrings[] = { "<r>", "<g>", "<p>", "<k>", "<b>", "<c>", "<o>", "<z>" };

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
    if (zone == 0 || zone == 1)
      continue;

    zoneData & zd = zone_table[zone];

    if (!zd.enabled)
      continue;

    if (!zd.num_mobs)
      continue;

    char buf[256],
         buf2[256];

    strcpy(buf, zd.name);
    char *s = strchr(buf, '-');
    char *n = buf;

    if (s) {
      --s;       // get the space before the -
      *s = '\0'; // after builder name
      s += 2;    // get the space after the -
      *s = '\0'; // after zone name
      ++s;
    } else {
      s = buf;
      n = NULL;
    }

    // buf is now the builder name, s is the zone name
    float  avg    = (zd.num_mobs ? zd.mob_levels / zd.num_mobs : 0);
    double x,
           total  = 0,
           dev,
           minlev = 1000,
           maxlev = -1;
    int    virt,
           count  = 0;

    for(unsigned int mobnum=0;mobnum<mob_index.size();mobnum++){
      virt=mob_index[mobnum].virt;

      if (virt > zone_table[zone-1].top && virt <=  zone_table[zone].top && mob_index[mobnum].doesLoad) {
	count += mob_index[mobnum].getMaxNumber();
	x      = (mob_index[mobnum].level * mob_index[mobnum].getMaxNumber());
	
	if (mob_index[mobnum].level>maxlev)
	  maxlev = mob_index[mobnum].level;

	if (mob_index[mobnum].level<minlev)
	  minlev = mob_index[mobnum].level;

	x     -= avg;
	x     *= x;
	total += x;

      }
    }

    total /= (count-1);
    dev    = sqrt(total);
    total  = count = 0;

    // now go through the mobs again and average up only a few
    for (unsigned int mobnum = 0; mobnum < mob_index.size(); mobnum++) {
      virt = mob_index[mobnum].virt;

      if (virt > zone_table[zone-1].top && virt <= zone_table[zone].top && mob_index[mobnum].doesLoad) {
	if (mob_index[mobnum].level < avg+dev && mob_index[mobnum].level > avg-dev) {
	  total += (mob_index[mobnum].level * mob_index[mobnum].getMaxNumber());
	  count += mob_index[mobnum].getMaxNumber();
	}
      }
    }
    
    if(count > 0 && total > 0)
      avg = total / count;

    if(minlev == 1000)
      minlev = zd.min_mob_level;

    if(maxlev == -1)
      maxlev = zd.max_mob_level;

    sprintf(buf2, "%-25.25s : %-10.10s : Level: avg: %i, min: %3.0f, max %3.0f\n\r", s, (n ? n : ""), (int)avg, minlev, maxlev);

    sortZoneVec.push_back(zoneSorter(avg, buf2));
  }

  // sort the vector
  sort(sortZoneVec.begin(), sortZoneVec.end(), zoneSorter());

  // list is sorted by avg level, cat it all together now
  int lastavg = -1;

  for (zone = 0; zone < sortZoneVec.size(); zone++) {
    char tString[256];

    if ((int)(sortZoneVec[zone].avgLevel) != lastavg) {
      if (++cIndex == 8)
	cIndex = 0;

      lastavg = (int)(sortZoneVec[zone].avgLevel);
    }
    
    sprintf(tString, "%d. %s%s<z>", (zone + 1), colorStrings[cIndex], sortZoneVec[zone].zoneName.c_str());
    str += tString;
  }

  desc->page_string(str);
}
