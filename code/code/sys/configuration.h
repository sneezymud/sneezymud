#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

#include "sstring.h"

class Config {
 private:
  Config();

  // modifies rate at which items take damage
  // The higher the number, the lower the damage
  // Any hit doing less then this amount has no chance of damaging
  // All other hits get modifier on damage rate based on this value
  // 4.1 balanced at 2 prior to depreciation
  static int ITEM_DAMAGE_RATE;

  // the minimum "hardness" for a material to damage/blunt a weapon
  // when hitting.
  static int WEAPON_DAM_MIN_HARDNESS;
  
  // the max value of a hardness roll, raising it = weapon
  // damage/blunt DECREASE
  static int WEAPON_DAM_MAX_HARDNESS;
  
  // the max value of a sharpness roll, raising it = weapon blunt DECREASE
  static int WEAPON_DAM_MAX_SHARP;
  
  // causes items left in repair to be deleted after a set number of days.
  // Good to keep things circulating, but bad if extended downtime anticipated.
  // Simply deletes the file in mobdata/repairs/, the tickets still exist
  // and the repairman will say he doesn't have the item.
  static bool NUKE_REPAIR_ITEMS;
  
  // Enables a check to validate that players are not multiplaying. 
  // check is done each login and periodically for all chars logged in.
  static bool CHECK_MULTIPLAY;
  
  // code will disallow any bad multiplay event
  static bool FORCE_MULTIPLAY_COMPLIANCE;

  // Enables automatic generation of repossession mobs based on item max-exist.
  // Not extremely popular, but a good way to get item overturn.
  static bool REPO_MOBS;
  
  // items that are over max-exist get hunted by a buffed up version of the
  // hunter.  Requires REPO_MOBS be TRUE.
  // VERY unpopular
  static bool SUPER_REPO_MOBS;
  
  // shops tend to get a lot of goods that strictly speaking aren't isSimilar()
  // slightly damaged, depreciated, etc.
  // We can eliminate this by turning this on.  Any item not perfect will get
  // deleted.
  static bool NO_DAMAGED_ITEMS_SHOP;

  // causes player/rent files to be automatically purged if inactive for
  // more then a few weeks.  Conserves disk space and speeds up the boot
  // process significantly.  Periods of college breaks are bypassed.
  static bool auto_deletion;
  
  // requires auto-deletion turned on causes deletion only of the rent
  // file.  Otherwise pfile, rent and account go
  static bool rent_only_deletion;
  
  // whether to tax rent at the public hostels
  static bool rent_tax;
  
  // Causes mobs in inactive zones to be deleted.  Typically, 50% of the mud's
  // mobs would qualify.  Dramatically speeds up the mobileActivity loop and
  // improves CPU performance.
  static bool nuke_inactive_mobs;

  // Causes mobs to drop their zonefile loaded gear and randomly
  // generated loot when they die, and not when they spawn.  the
  // skills spy and steal also trigger off of this setting to make
  // their behavior usful when mobs have no gear.  turning this on
  // makes checking loot loads impossible, makes loot appear more
  // 'random' reduces server memory footprint, reduces load time (from
  // having to calculate loot at spawn), and keeps loot items from
  // scapping during a fight
  static bool load_on_death;

  // whether or not boost::format will throw exceptions for bad format
  // strings, extra or missing arguments etc.  false for maximum
  // stability, true for maximum bug detection.
  static bool throw_format_exceptions;

  // suppress assigning of special routines
  static bool no_specials;

  // run as trimmed port
  static bool b_trimmed;

  // data directory to run in (eg "lib")
  static sstring data_dir;

 public:
  static bool doConfiguration(int argc=0, char *argv[]=0);
  
  static int ItemDamageRate(){ return ITEM_DAMAGE_RATE; }
  static int WeaponDamMinHardness(){ return WEAPON_DAM_MIN_HARDNESS;}
  static int WeaponDamMaxHardness(){ return WEAPON_DAM_MAX_HARDNESS;}
  static int WeaponDamMaxSharp(){ return WEAPON_DAM_MAX_SHARP;}
  static bool NukeRepairItems(){ return NUKE_REPAIR_ITEMS; }
  static bool CheckMultiplay(){ return CHECK_MULTIPLAY; }
  static bool ForceMultiplayCompliance(){return FORCE_MULTIPLAY_COMPLIANCE;}
  static bool RepoMobs(){ return REPO_MOBS; }
  static bool SuperRepoMobs(){ return SUPER_REPO_MOBS; }
  static bool NoDamagedItemsShop(){ return NO_DAMAGED_ITEMS_SHOP; }
  static bool AutoDeletion(){ return auto_deletion; }
  static bool RentOnlyDeletion(){ return rent_only_deletion; }
  static bool RentTax(){ return rent_tax; }
  static bool NukeInactiveMobs(){ return nuke_inactive_mobs; }
  static bool LoadOnDeath(){ return load_on_death; }
  static bool ThrowFormatExceptions(){ return throw_format_exceptions; }
  static bool NoSpecials(){ return no_specials; }
  static bool bTrimmed(){ return b_trimmed; }
  static sstring DataDir(){ return data_dir; }

  // defines the port of the running muds
  class Port {
  private:
    Port();
  public:
    static const int PROD;
    static const int GAMMA;
    static const int ALPHA;
  };
};




#endif
