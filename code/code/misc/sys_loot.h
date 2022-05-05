#pragma once

struct TLootStructure {
  int tLevel, // Object Level;
      tRNum;  // Object RealNumber;

  TLootStructure * tNext;
};

extern TLootStructure * tLoot;

extern bool sysLootBoot();
extern bool sysLootLoad(resetCom &, TBeing *, TObj *, bool);
extern bool isLegalLoot(itemTypeT);
extern bool isLegalLoot(int);
