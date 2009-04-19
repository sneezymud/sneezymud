#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

// defines the port of the running muds
extern const int PROD_GAMEPORT;
extern const int PROD_XMLPORT;
extern const int BETA_GAMEPORT;
extern       int GAMMA_GAMEPORT; // quick boot
extern const int ALPHA_GAMEPORT;
extern const int BUILDER_GAMEPORT;

class Config {
 private:
  Config();

  // modifies rate at which items take damage
  // The higher the number, the lower the damage
  // Any hit doing less then this amount has no chance of damaging
  // All other hits get modifier on damage rate based on this value
  // 4.1 balanced at 2 prior to depreciation
  static int ITEM_DAMAGE_RATE;

  // used to determine rent credit
  // credit = level * maxi(20, level) * x
  // the values of the "model" items (soft leather, plate, etc) were set based
  // on this rent credit being at 75
  // 4.0's no rent, and 4.1's big rent credit have distorted this so feel free
  // to adjust if this no longer works
  static int RENT_CREDIT_VAL;
  
  // if player goes over rent, items are "sold" to pay for it
  // this handles what is done with the sold item(s)
  // If turned on, the pawnguy gets them
  // otherwise they get deleted
  static bool RENT_SELL_TO_PAWN;
  
  // causes innkeepers to grant rentCredit based on the innkeepers levels
  // otherwise, it is based on the players level.
  // the chief use of this is to encourage high level pc's to use certain inns
  static bool RENT_RESTRICT_INNS_BY_LEVEL;
  
  // causes bad things to happen to player based on time in autorent
  // there is a grace period to handle crashes
  static bool PENALIZE_FOR_AUTO_RENTING;
  
  // the minimum "hardness" for a material to damage/blunt a weapon
  // when hitting.
  static int WEAPON_DAM_MIN_HARDNESS;
  
  // the max value of a hardness roll, raising it = weapon
  // damage/blunt DECREASE
  static int WEAPON_DAM_MAX_HARDNESS;
  
  // the max value of a sharpness roll, raising it = weapon blunt DECREASE
  static int WEAPON_DAM_MAX_SHARP;
  
  // speef is too important for comments so we don't know what this does
  static bool SPEEF_MAKE_BODY;
  
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
  
  // Causes mobs in inactive zones to be deleted.  Typically, 50% of the mud's
  // mobs would qualify.  Dramatically speeds up the mobileActivity loop and
  // improves CPU performance.
  static bool nuke_inactive_mobs;


 public:
  static bool doConfiguration(int argc=0, char *argv[]=0);
  
  static int ItemDamageRate(){ return ITEM_DAMAGE_RATE; }
  static int RentCreditVal(){ return RENT_CREDIT_VAL;}
  static bool RentSellToPawn(){ return RENT_SELL_TO_PAWN;}
  static bool RentRestrictInnsByLevel(){return RENT_RESTRICT_INNS_BY_LEVEL;}
  static bool PenalizeForAutoRenting(){ return PENALIZE_FOR_AUTO_RENTING;}
  static int WeaponDamMinHardness(){ return WEAPON_DAM_MIN_HARDNESS;}
  static int WeaponDamMaxHardness(){ return WEAPON_DAM_MAX_HARDNESS;}
  static int WeaponDamMaxSharp(){ return WEAPON_DAM_MAX_SHARP;}
  static bool SpeefMakeBody(){ return SPEEF_MAKE_BODY;}
  static bool NukeRepairItems(){ return NUKE_REPAIR_ITEMS; }
  static bool CheckMultiplay(){ return CHECK_MULTIPLAY; }
  static bool ForceMultiplayCompliance(){return FORCE_MULTIPLAY_COMPLIANCE;}
  static bool RepoMobs(){ return REPO_MOBS; }
  static bool SuperRepoMobs(){ return SUPER_REPO_MOBS; }
  static bool NoDamagedItemsShop(){ return NO_DAMAGED_ITEMS_SHOP; }
  static bool AutoDeletion(){ return auto_deletion; }
  static bool RentOnlyDeletion(){ return rent_only_deletion; }
  static bool NukeInactiveMobs(){ return nuke_inactive_mobs; }
};




#endif
