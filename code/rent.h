//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: rent.h,v $
// Revision 5.4  2002/06/14 02:15:40  peel
// changed rent item limit to 10k from 1k
//
// Revision 5.3  2002/06/13 05:00:59  peel
// updated rent file version, woops
//
// Revision 5.2  2002/06/04 19:57:23  peel
// rent file version change
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __RENT_H
#define __RENT_H

const int CURRENT_RENT_VERSION     =9;
// 0        3.x - initial version
// 1        4.1 rev 61  - changed weight from int to float
// 2        pre 4.5 - added depreciation to rentObject structure
// 3        5.0 : Dec 16 1998 : objfile convert adjusting armor price
// 4        5.0 : Dec 28 1998 : objfile convert adjusting weapon price
// 5        5.0 : Jan 14 1999 : Forced all EQ to become like tinyfile
// 6        5.0 : Jan 19 1999 : More of the same
// 7        5.1 : April 16 1999 : More of the same
// 8        5.2 : June 4 2002 : shuffled 4vals for weapons

// This was 200, but i see no technical reason to not be able to increase it
const int MAX_OBJ_SAVE =10000;     

class objCost { 
  public:
    int total_cost;
    int no_carried;
    int lowrentobjs;
    bool ok;
    objCost();
    ~objCost();
};

class pcorpseObject
{
  public:
    int corpse_room;
    int num_corpses_room;
    char charName[20];
    pcorpseObject();
};

// this is binary struct that we actually write out to a file
// altering it will change rent file, repair file, shop file
class rentObjAffData {
  public:
    short type;
    sbyte level;
    int duration;
    int renew;
    long modifier;
    byte location;  // this should NOT be applyTypeT
    long modifier2;
    long bitvector;

    rentObjAffData();
};

class rentObject
{
  public:
    int item_number;
    int value[4];
    unsigned int extra_flags;
    float weight;
    long bitvector;
    rentObjAffData affected[MAX_OBJ_AFFECT];
    int decay_time;
    int struct_points;
    int max_struct_points;
    ubyte material_points;
    int volume;
    int cost; 
    byte depreciation;

    rentObject();
};

class rentHeader {
  public:
    unsigned char version;     /* version # of the rent file */
    char owner[20];    /* Name of player                     */
    int original_gold; /* For use with total charges on login*/
    int gold_left;     /* Number of goldcoins left at owner  */
    int total_cost;    /* The cost for all items, per day    */
    long last_update;  /* Time in seconds, when last updated */
    long first_update; /* Time in seconds, when first updated*/
    int  number;       /* number of objects */
    rentHeader();
};

extern TObj *raw_read_item(FILE * fp, unsigned char version);
extern bool raw_write_item(FILE * fp, TObj *o, unsigned char version);

#endif
