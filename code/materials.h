//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////



#ifndef __MATERIALS_H
#define __MATERIALS_H

const int    MAT_UNDEFINED       =0;
const int    MAT_PAPER           =1;
const int    MAT_CLOTH           =2;
const int    MAT_WAX             =3;
const int    MAT_GLASS           =4;
const int    MAT_WOOD            =5;
const int    MAT_SILK            =6;
const int    MAT_FOODSTUFF       =7;
const int    MAT_PLASTIC         =8;
const int    MAT_RUBBER          =9;
const int    MAT_CARDBOARD       =10;
const int    MAT_STRING          =11;
const int    MAT_PLASMA          =12;
const int    MAT_TOUGH_CLOTH     =13;
const int    MAT_CORAL           =14;
const int    MAT_HORSEHAIR       =15;
const int    MAT_HAIR            =16;
const int    MAT_POWDER          =17;
const int    MAT_PUMICE	         =18;
const int    MAT_LAMINATED       =19;

const int    MAX_MAT_GENERAL     =20;     // Move and change

const int    MAT_GEN_ORG         =50;
const int    MAT_LEATHER         =51;
const int    MAT_TOUGH_LEATHER   =52;
const int    MAT_DRAGON_SCALE    =53;
const int    MAT_WOOL            =54;
const int    MAT_FUR             =55;
const int    MAT_FEATHERED       =56;
const int    MAT_WATER           =57;
const int    MAT_FIRE            =58;
const int    MAT_EARTH           =59;
const int    MAT_ELEMENTAL       =60;
const int    MAT_ICE             =61;
const int    MAT_LIGHTNING       =62;
const int    MAT_CHAOS           =63;
const int    MAT_CLAY            =64;
const int    MAT_PORCELAIN       =65;
const int    MAT_STRAW           =66;
const int    MAT_PEARL           =67;
const int    MAT_HUMAN_FLESH     =68;
const int    MAT_FUR_CAT         =69;
const int    MAT_FUR_DOG         =70;
const int    MAT_FUR_RABBIT      =71;
const int    MAT_GHOSTLY         =72;
const int    MAT_DWARF_LEATHER   =73;
const int    MAT_SOFT_LEATHER    =74;
const int    MAT_FISHSCALE       =75;
const int    MAT_OGRE_HIDE       =76;
const int    MAT_HEMP            =77;

const int    MAX_MAT_NATURE      =28;     // Move and change  (max+1 - 50)

const int    MAT_GEN_MINERAL     =100;
const int    MAT_JEWELED         =101;
const int    MAT_RUNED           =102;
const int    MAT_CRYSTAL         =103;
const int    MAT_DIAMOND         =104;
const int    MAT_EBONY           =105;
const int    MAT_EMERALD         =106;
const int    MAT_IVORY           =107;
const int    MAT_OBSIDIAN        =108;
const int    MAT_ONYX            =109;
const int    MAT_OPAL            =110;
const int    MAT_RUBY            =111;
const int    MAT_SAPPHIRE        =112;
const int    MAT_MARBLE          =113;
const int    MAT_STONE           =114;
const int    MAT_BONE            =115;
const int    MAT_JADE            =116;
const int    MAT_AMBER           =117;
const int    MAT_TURQUOISE       =118;
const int    MAT_AMETHYST        =119;
const int    MAT_MICA            =120;
const int    MAT_DRAGONBONE      =121;
const int    MAT_MALACHITE       =122;
const int    MAT_GRANITE         =123;
const int    MAT_QUARTZ          =124;
const int    MAT_JET             =125;
const int    MAT_CORUNDUM        =126;
const int    MAX_MAT_MINERAL     =27;     // Move and change  (max+1 - 100)

const int    MAT_GEN_METAL       =150;  
const int    MAT_COPPER          =151;
const int    MAT_SCALE_MAIL      =152;
const int    MAT_BANDED_MAIL     =153;
const int    MAT_CHAIN_MAIL      =154;
const int    MAT_PLATE           =155;
const int    MAT_BRONZE          =156;
const int    MAT_BRASS           =157;
const int    MAT_IRON            =158;
const int    MAT_STEEL           =159;
const int    MAT_MITHRIL         =160;
const int    MAT_ADAMANTITE      =161;
const int    MAT_SILVER          =162;
const int    MAT_GOLD            =163;
const int    MAT_PLATINUM        =164;
const int    MAT_TITANIUM        =165;
const int    MAT_ALUMINUM        =166;
const int    MAT_RINGMAIL        =167;
const int    MAT_GNOMEMAIL       =168;
const int    MAT_ELECTRUM        =169;
const int    MAT_ATHANOR         =170;
const int    MAT_TIN             =171;
const int    MAT_TUNGSTEN        =172;
const int    MAT_ADMINTITE       =173;
const int    MAT_TERBIUM         =174;
const int    MAT_ELVENMAIL       =175;
const int    MAT_ELVENSTEEL      =176;

const int    MAX_MAT_METAL       =27;      //Move and change  (max+1 - 150)

struct material_type_numbers
{
  byte cut_susc;
  byte smash_susc;
  byte burned_susc;
  byte pierced_susc;
  byte hardness;
  ubyte water_susc;
  ubyte fall_susc;
  ubyte float_weight;
  byte noise;
  ubyte vol_mult;
  ubyte conductivity;
  int flammability;
  ubyte acid_susc;
  int price;
  int availability;
  int (*repair_proc)(TBeing *,TObj *o);
  char mat_name[20];
};

extern const struct material_type_numbers material_nums[200];

extern sstring describeMaterial(const int);
extern sstring describeMaterial(const TThing *);

#endif
