//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: drug.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DRUG_H
#define __DRUG_H

enum drugTypeT {
     DRUG_NONE,
     DRUG_PIPEWEED,
     DRUG_OPIUM,
     MAX_DRUG
};
const drugTypeT MIN_DRUG = drugTypeT(1);

class TDrugInfo
{
  public:
    const char *name;
    int potency; // max units of drug in body that takes affect
    int duration; 
    TDrugInfo(const char *n, int p, int d);
    TDrugInfo(const TDrugInfo &);
    TDrugInfo & operator=(const TDrugInfo &);
    ~TDrugInfo();

    TDrugInfo();  // not supposed to be called, but needed for vector
};
extern vector<TDrugInfo>drugTypes;

class drugData {
 public:
    struct time_info_data first_use;
    struct time_info_data last_use;
    unsigned int total_consumed;
    unsigned int current_consumed;

    drugData();
    ~drugData();
    drugData(const drugData &t);
    drugData & operator =(const drugData &t);

};

#endif
