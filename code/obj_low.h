
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "obj_LOW.cc" - routines related to checking stats on objects.
//
//////////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------
  New LOW classification rules for ARMOR

  See obj_low.cc for description
 ------------------------------------------------------------------*/

enum Tier
{
  // equipment
  Tier_Clothing = 0,
  Tier_Light,
  Tier_Medium,
  Tier_Heavy,
  Tier_Jewelry,

  // weapons
  Tier_Common,
  Tier_Simple,
  Tier_Martial,

  Tier_Max
};

enum PointType
{
  PointType_All = 0,
  PointType_Stats,
  PointType_Main,

  PointType_Max
};

// constants used to try to approximate armor vs stats
#define low_acPerHitrate (25.0 / 3.0) // ac to affect hit rate by 1%
#define low_statValue (0.25) // change to hit rate for 1 stat point
#define low_acModifier (0.25) // inflation multiplier for stat costs (used for more than combat, etc)
#define low_acPerLevel (25.0) // the base AC you get per item level?
#define low_exchangeRate (low_acPerHitrate * low_statValue * low_acModifier) // cost in ac for 1 stat point

// base class for TObj
class ObjectEvaluator
{
public:
  ObjectEvaluator(const TObj *o);

  sstring getTierString();
  int getPointValue(PointType type = PointType_All);
  double getLoadLevel(PointType type = PointType_All);

private:
  int m_stat;
  const TObj *m_obj;
  bool m_gotStats;

  int getStatPointsRaw();
  //int getStructPointsRaw();  // struct we want to move off of level calc and into weight (so its shown in value)

protected:
  virtual int getMainPointsRaw() = 0;
  virtual Tier getTier() = 0;
  virtual bool IgnoreApply(applyTypeT t) { return false; }
};

// class for TBaseClothing
class ArmorEvaluator : public ObjectEvaluator
{
public:
  ArmorEvaluator(const TBaseClothing *o);

private:
  const TBaseClothing *m_clothing;
  int m_main;
  bool m_gotMain;

protected:
  virtual int getMainPointsRaw();
  virtual Tier getTier();
  virtual bool IgnoreApply(applyTypeT t) { return t == APPLY_ARMOR; }
};

// class for TBaseWeapon
class WeaponEvaluator : public ObjectEvaluator
{
public:
  WeaponEvaluator(const TBaseWeapon *o);

private:
  const TBaseWeapon *m_weap;
  int m_main;

protected:
  virtual int getMainPointsRaw();
  virtual Tier getTier();
};
