//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: loadset.h,v $
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


#ifndef __LOADSET_H
#define __LOADSET_H

enum loadSetTypeT {
  LST_ALL  = -1,
  LST_HELM = 0,
  LST_COLLAR,
  LST_JACKET,
  LST_SLEEVE,
  LST_GLOVE,
  LST_BELT,
  LST_BRACELET,
  LST_LEGGING,
  LST_BOOT,
  LST_RING,
  LST_SHIELD,
  LST_CLOAK,
  LST_MAX
};

class loadSetStruct {
  public:
    const          char   *name;
                   int     equipment[LST_MAX];
                   race_t  suitRace;
                   double  suitLevel;
    unsigned short int     suitClass[LST_MAX],
                           suitClassTotal,
                           suitClassPossible;

    loadSetStruct & operator=(const loadSetStruct &);

    loadSetStruct();
    ~loadSetStruct();
};

class loadSetClass {
  public:
    std::map<unsigned short int, loadSetStruct>suits;

    bool suitLoad(const char *, TBeing *, loadSetTypeT, int, int, bool findLoadPotential=false);
    void SetupLoadSetSuits();
    void suitAdd(const char *, int, int, int, int, int, int,
                 int, int, int, int, int, int, race_t);

    loadSetClass() {}
    ~loadSetClass() {}
};

extern loadSetClass suitSets;


// given a list of numbers and their 'weight', allows us to get random values from them
// passing no weight allows it to 'float' - essentially is max weight
// for added sadism, we can also depress the chance of generating an item by a percent
class weightedRandomizer
{
private:
    struct weightedBucket
    {
      int value;
      int weight;
      int percent;
    };

    std::vector<weightedBucket> m_items;
    static const unsigned int iterMax = 50;

public:
    weightedRandomizer() {}
    int size() { return (int)m_items.size(); }
    int add(int v) { return add(v, -1, 100); }
    int add(int v, int w) { return add(v, w, 100); }
    int add(int v, int w, int p);
    void add(int *pv, unsigned int c) { for(unsigned int i=0; i < c; i++) add(pv[i], -1, 100); }
    void add(int *pv, int *pw, unsigned int c) { for(unsigned int i=0; i < c; i++) add(pv[i], pw[i], 100); }
    void add(int *pv, int *pw, int *pp, unsigned int c) { for(unsigned int i=0; i < c; i++) add(pv[i], pw[i], pp[i]); }

    int getRandomIndex() { return getRandomIndex(-1); }
    int getRandomIndex(int weightMax);
    int getItem(int i) { return (i > (int)m_items.size()) ? 0 : m_items[i].value; }
    int getWeight(int i) { return (i > (int)m_items.size()) ? -1 : m_items[i].weight; }
    int getPercent(int i) { return (i > (int)m_items.size()) ? 100 : m_items[i].percent; }
    int getRandomItem() { return getItem(getRandomIndex(-1)); }
    int getRandomItem(int weightMax) { return getItem(getRandomIndex(weightMax)); }
};

#endif
