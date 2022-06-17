#pragma once

#include <stdint.h>

const unsigned long ACT_STRINGS_CHANGED    = (1<<0); //1
const unsigned long ACT_SENTINEL   = (1<<1);// 2
const unsigned long ACT_SCAVENGER  = (1<<2);// 4
const unsigned long ACT_DISGUISED  = (1<<3);// 8
const unsigned long ACT_NICE_THIEF = (1<<4);// 16
const unsigned long ACT_AGGRESSIVE = (1<<5);         // 32
const unsigned long ACT_STAY_ZONE  = (1<<6);         // 64
const unsigned long ACT_WIMPY      = (1<<7);         // 128
const unsigned long ACT_ANNOYING   = (1<<8);         // 256
const unsigned long ACT_HATEFUL    = (1<<9);         // 512
const unsigned long ACT_AFRAID    = (1<<10);         // 1024
const unsigned long ACT_IMMORTAL  = (1<<11);         // 2048
const unsigned long ACT_HUNTING   = (1<<12);         // 4096
const unsigned long ACT_DEADLY    = (1<<13);         // 8192
const unsigned long ACT_POLYSELF  = (1<<14);         // 16384
const unsigned long ACT_GUARDIAN  = (1<<15);         // 32768
const unsigned long ACT_SKELETON  = (1<<16);         // 65536
const unsigned long ACT_ZOMBIE    = (1<<17);         // 131072
const unsigned long ACT_GHOST     = (1<<18);         // 262144
const unsigned long ACT_DIURNAL   = (1<<19);         // 524288
const unsigned long ACT_NOCTURNAL = (1<<20);         // 1048576
const unsigned long ACT_PROTECTOR = (1<<21);         // 2097152
const unsigned long ACT_PROTECTEE = (1<<22);         // 4194304
const unsigned long ACT_HIT_BY_PK = (1<<23);

const int MAX_MOB_ACTS    = 24;

const unsigned short HATE_SEX      = (1<<0);
const unsigned short HATE_RACE     = (1<<1);
const unsigned short HATE_CHAR     = (1<<2);
const unsigned short HATE_CLASS    = (1<<3);
const unsigned short HATE_UNUSED   = (1<<4);
const unsigned short HATE_UNUSED2  = (1<<5);
const unsigned short HATE_VNUM     = (1<<6);

const unsigned short FEAR_SEX      = (1<<0);
const unsigned short FEAR_RACE     = (1<<1);
const unsigned short FEAR_CHAR     = (1<<2);
const unsigned short FEAR_CLASS    = (1<<3);
const unsigned short FEAR_UNUSED   = (1<<4);
const unsigned short FEAR_UNUSED2  = (1<<5);
const unsigned short FEAR_VNUM     = (1<<6);

const int MAX_MORT      = 50;
const int GOD_LEVEL1    = MAX_MORT + 1;
const int MAX_IMMORT    = MAX_MORT + 10;

// this is the parameter for various newbie protection things, like pk
// protection, food praying at CS, etc
const int MAX_NEWBIE_LEVEL = 5;


const unsigned short int PART_BLEEDING     = (1<<0);
const unsigned short int PART_INFECTED     = (1<<1);
const unsigned short int PART_PARALYZED    = (1<<2);
const unsigned short int PART_BROKEN       = (1<<3);
const unsigned short int PART_SCARRED      = (1<<4);
const unsigned short int PART_BANDAGED     = (1<<5);
const unsigned short int PART_MISSING      = (1<<6);
const unsigned short int PART_USELESS      = (1<<7);
const unsigned short int PART_LEPROSED     = (1<<8);
const unsigned short int PART_TRANSFORMED  = (1<<9);
const unsigned short int PART_ENTANGLED    = (1<<10);
const unsigned short int PART_BRUISED      = (1<<11);
const unsigned short int PART_GANGRENOUS   = (1<<12);

const int MAX_PARTS          = 12;       // move and change

/* 'class' for PC's */
const unsigned short CLASS_MAGE         = (1<<0);   // 1
const unsigned short CLASS_CLERIC       = (1<<1);   // 2
const unsigned short CLASS_WARRIOR      = (1<<2);   // 4
const unsigned short CLASS_THIEF        = (1<<3);   // 8
const unsigned short CLASS_SHAMAN       = (1<<4);   // 16 
const unsigned short CLASS_DEIKHAN      = (1<<5);   // 32
const unsigned short CLASS_MONK         = (1<<6);   // 64
const unsigned short CLASS_RANGER       = (1<<7);   // 128
const unsigned short CLASS_COMMONER     = (1<<8);   // 256
const unsigned short CLASS_ALL          = (1<<9)-1;

/* Bitvector for 'affected_by' */
const uint64_t AFF_BLIND             = uint64_t(1<<0);
const uint64_t AFF_INVISIBLE         = uint64_t(1<<1);
const uint64_t AFF_SWIM              = uint64_t(1<<2);
const uint64_t AFF_DETECT_INVISIBLE  = uint64_t(1<<3);
const uint64_t AFF_DETECT_MAGIC      = uint64_t(1<<4);
const uint64_t AFF_SENSE_LIFE        = uint64_t(1<<5);
const uint64_t AFF_LEVITATING        = uint64_t(1<<6);
const uint64_t AFF_SANCTUARY         = uint64_t(1<<7);
const uint64_t AFF_GROUP             = uint64_t(1<<8);
const uint64_t AFF_WEB               = uint64_t(1<<9);
const uint64_t AFF_CURSE             = uint64_t(1<<10);
const uint64_t AFF_FLYING            = uint64_t(1<<11);
const uint64_t AFF_POISON            = uint64_t(1<<12);
const uint64_t AFF_STUNNED           = uint64_t(1<<13);
const uint64_t AFF_PARALYSIS         = uint64_t(1<<14);
const uint64_t AFF_INFRAVISION       = uint64_t(1<<15);
const uint64_t AFF_WATERBREATH       = uint64_t(1<<16);
const uint64_t AFF_SLEEP             = uint64_t(1<<17);
const uint64_t AFF_SCRYING           = uint64_t(1<<18);
const uint64_t AFF_SNEAK             = uint64_t(1<<19);
const uint64_t AFF_HIDE              = uint64_t(1<<20);
const uint64_t AFF_SHOCKED           = uint64_t(1<<21);
const uint64_t AFF_CHARM             = uint64_t(1<<22);
const uint64_t AFF_SYPHILIS          = uint64_t(1<<23);
const uint64_t AFF_SHADOW_WALK       = uint64_t(1<<24);
const uint64_t AFF_TRUE_SIGHT        = uint64_t(1<<25);
const uint64_t AFF_MUNCHING_CORPSE   = uint64_t(1<<26);
const uint64_t AFF_RIPOSTE           = uint64_t(1<<27);
const uint64_t AFF_SILENT            = uint64_t(1<<28);
const uint64_t AFF_ENGAGER           = uint64_t(1<<29);
const uint64_t AFF_AGGRESSOR         = uint64_t(1<<30); // (set automatically)

// switch to literals to avoid bitshift cast to 32-bit long
const uint64_t AFF_CLARITY           = uint64_t(0x0000000080000000LLU); // switch to literals to avoid bitshift cast to 32-bit long
const uint64_t AFF_FLIGHTWORTHY      = uint64_t(0x0000000100000000LLU);
const uint64_t AFF_FOCUS_ATTACK      = uint64_t(0x0000000200000000LLU);
const int AFF_MAX                    = 34;

// these are used to pass deletion bitvectors through functions
// it is used in same functions that return degree of damage done
// these values are essentially negative
//
// damage is sometimes passed in same manner as deletes, so make sure
// value for damage is always less than smallest DELETE value
//
// they are combinations of bits, IS_SET, REMOVE_BIT, SET_BIT can not be used
// use IS_SET_DELETE, ADD_DELETE, REM_DELETE instead
//
#if 0 
const int DELETE_ITEM          = ((1<<5));
const int DELETE_THIS          = ((1<<6));
const int DELETE_VICT          = ((1<<7));
const int ALREADY_DELETED      = ((1<<8));
const int RET_STOP_PARSING     = ((1<<9));

#elif 1
const int DELETE_ITEM          = ((1<<5) | (1<<29));
const int DELETE_THIS          = ((1<<6) | (1<<29));
const int DELETE_VICT          = ((1<<7) | (1<<29));
const int ALREADY_DELETED      = ((1<<8) | (1<<29));
const int RET_STOP_PARSING     = ((1<<9) | (1<<29));

#else
const int DELETE_ITEM          = ((1<<5) | (1<<31));
const int DELETE_THIS          = ((1<<6) | (1<<31));
const int DELETE_VICT          = ((1<<7) | (1<<31));
const int ALREADY_DELETED      = ((1<<8) | (1<<31));
const int RET_STOP_PARSING     = ((1<<9) | (1<<31));
#endif

// This was created to limit the size of getName()
// There are hundreds of places in the code that use char buf[80] to
// hold a copy of getName(), increasing this without changing all of
// those will cause BAD!!! memory corruptions
const int MAX_NAME_LENGTH = 80;

// parts of speech
const int POS_OBJECT            = 1;
const int POS_SUBJECT           = 2;
const int POS_POSSESS           = 3;
