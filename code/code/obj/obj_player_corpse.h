//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "enum.h"
#include "obj.h"
#include "obj_base_corpse.h"
#include "sstring.h"

class TBeing;
class TThing;

// corpses for PCs
class TPCorpse : public TBaseCorpse {
  private:
    int on_lists;
    int corpse_in_room;
    int num_corpses_in_room;
    float exp_lost;
    sstring fileName;
    TPCorpse* nextGlobalCorpse;
    TPCorpse* nextCorpse;
    TPCorpse* previousCorpse;

  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual void decayMe();
    virtual int getMe(TBeing*, TThing*);
    virtual void getMeFrom(TBeing*, TThing*);
    virtual void dropMe(TBeing*, showMeT, showRoomT);
    virtual itemTypeT itemType() const { return ITEM_PCORPSE; }
    virtual int scavengeMe(TBeing* ch, TObj**);
    void removeCorpseFromList(bool updateFile = true);
    void addCorpseToLists();
    void saveCorpseToFile();
    //    void assignCorpsesToRooms();
    int checkOnLists();
    void togOnCorpseListsOn();
    void togOnCorpseListsOff();
    void setRoomNum(int n);
    int getRoomNum() const;
    void setNumInRoom(int n);
    int getNumInRoom() const;
    void addToNumInRoom(int n);
    float getExpLost() const;
    void setExpLost(float exp);
    void setOwner(const sstring& Name);
    sstring getOwner() const;
    void clearOwner();
    void setNext(TPCorpse* n);
    void removeNext();
    TPCorpse* getNext() const;
    void setPrevious(TPCorpse* n);
    void removePrevious();
    TPCorpse* getPrevious() const;
    void setNextGlobal(TPCorpse* n);
    void removeGlobalNext();
    virtual void describeObjectSpecifics(const TBeing*) const {};
    TPCorpse* getGlobalNext() const;
    TPCorpse();
    TPCorpse(const TPCorpse& a);
    TPCorpse& operator=(const TPCorpse& a);
    virtual ~TPCorpse();
};
