//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////




#if 0
struct weapon_type_numbers
{
  char weap_name[30];
  ubyte melee_damage;  //melee damage
  ubyte thrown_damage;  //thrown damage
  ubyte speed;
  ubyte size;
  ubyte length;
  weaponGroup weapongroup;
  ubyte attack_type_1;
  ubyte attack_type_2;
  ubyte attack_type_3;
  ubyte attack_type_4;
  bool twohanded;
  bool cancudgel;
  bool canbackstab;
  bool canstab;
  bool cancharge;
  bool cantrip;
  bool canentangle;
  bool unused1;
  bool unused2;
  bool unused3;
  bool unused4;
};
#endif

struct weapon_type_numbers
{
  char weap_name[30];

  ubyte melee_dam;
  ubyte missile_dam;
  ubyte speed;
  ubyte size;
  ubyte length;
  weaponGroup weapongroup;
  spellNumT attack_types[4];
  unsigned int uses;
};

enum weaponGroup {
  WCLASS_TWO_HANDED_BLADES,
  WCLASS_LONG_BLADES,
  WCLASS_SHORT_BLADES,
  WCLASS_STAVES,
  WCLASS_AXES,
  WCLASS_POLEARMS,
  WCLASS_HAMMERS,
  WCLASS_CLUBS,
  WCLASS_BOWS,
  WCLASS_CROSSBOWS,
  WCLASS_SPEARS,
  WCLASS_LANCES,
  WCLASS_NETS,
  WCLASS_ODDBALL
};

const unsigned int USE_NONE      =  (1<<0);
const unsigned int USE_TRIP      =  (1<<1); 
const unsigned int USE_DEFEND    =  (1<<2); // parry bonus
const unsigned int USE_CUDGEL    =  (1<<3);
const unsigned int USE_STAB      =  (1<<4);  
const unsigned int USE_BACKSTAB  =  (1<<5);
const unsigned int USE_ENTANGLE  =  (1<<6); 
const unsigned int USE_CHARGE    =  (1<<7); 
const unsigned int USE_MARTIAL   =  (1<<8); // monk attacks?
const unsigned int USE_THROWING  =  (1<<9); // throwing weapon, penalties on normal melee damage
                                            // but bonus on thrown damage 
const unsigned int USE_TWOHANDED =  (1<<10);
const unsigned int USE_DISARM    =  (1<<11);

struct weapon_quality_numbers
{
  ubyte modifier;
  ubyte quality_name[40];
};

extern const struct weapon_type_numbers weap_nums[200];
extern const struct weapon_quality_numbers qual_nums[16];

  



