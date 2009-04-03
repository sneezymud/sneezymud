//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//
//////////////////////////////////////////////////////////////////////////


// Stats.h
//
// A class for defining the stat system.

#ifndef __CHARACTERISTICS_H
#define __CHARACTERISTICS_H

class Stats {
  private:

   int str;
   int brawn;

   int dex;
   int agility;
   int speed;

   int con;
   int hardiness;

   int intel;
   int focus;

   int wis;

   int luck;
   int karma;

   int perception;

   int add;

  public: 
    Stats() {
      str = brawn = dex = agility = speed = con = hardiness = 150;
      intel = focus = wis = luck = karma = perception = add = 150;
    }

    void showStats(TBeing *caller);
    void report();

    inline int getStr() { return str; }
    inline int getBra() { return brawn; }

    inline int getDex() { return dex; }
    inline int getAgi() { return agility; }
    inline int getSpe() { return speed; }

    inline int getCon() { return con; }
    inline int getHar() { return hardiness; }

    inline int getInt() { return intel; }
    inline int getFoc() { return focus; }

    inline int getWis() { return wis; }

    inline int getLuc() { return luck; }
    inline int getKar() { return karma; }

    inline int getPer() { return perception; }

    inline int getAdd() { return add; }

    inline void setStr(int num) { str = num; }
    inline void setBra(int num) { brawn = num; }

    inline void setDex(int num) { dex = num; }
    inline void setAgi(int num) { agility = num; }
    inline void setSpe(int num) { speed = num; }

    inline void setCon(int num) { con = num; }
    inline void setHar(int num) { hardiness = num; }

    inline void setInt(int num) { intel = num; }
    inline void setFoc(int num) { focus = num; }

    inline void setWis(int num) { wis = num; }

    inline void setLuc(int num) { luck = num; }
    inline void setKar(int num) { karma = num; }

    inline void setPer(int num) { perception = num; }

    inline void setAdd(int num) { add = num; }
};

#endif
