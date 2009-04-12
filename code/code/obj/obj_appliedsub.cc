// organic.cc

#include "handler.h"
#include "room.h"
#include "being.h"
#include "create_engine.h"
#include "extern.h"
#include "obj_expandable_container.h"
#include "obj_organic.h"
#include "obj_arrow.h"
#include "obj_flame.h"
#include "obj_appliedsub.h"

TASubstance::TASubstance() :
  TObj(),
  AFlags(0),
  AMethod(0),
  ILevel(0),
  ISpell(0)
{
}

TASubstance::TASubstance(const TASubstance &a) :
  TObj(a),
  AFlags(a.AFlags),
  AMethod(a.AMethod),
  ILevel(a.ILevel),
  ISpell(a.ISpell)
{
}

TASubstance & TASubstance::operator=(const TASubstance &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  AFlags = a.AFlags;
  AMethod = a.AMethod;
  ILevel = a.ILevel;
  ISpell = a.ISpell;
  return *this;
}

TASubstance::~TASubstance()
{
}

void TASubstance::assignFourValues(int x1, int x2, int x3, int x4)
{
  setAFlags(x1);
  setAMethod(x2);
  setILevel(x3);
  setISpell(x4);
}

void TASubstance::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getAFlags();
  *x2 = getAMethod();
  *x3 = getILevel();
  *x4 = getISpell();
}

sstring TASubstance::statObjInfo() const
{
  sstring a("");
  return a;
}

// Class specific functions:
void TASubstance::describeObjectSpecifics(const TBeing *ch) const
{
}

int TBeing::doApplyHerbs(const sstring &tArg)
{
  TBeing      *vict   = NULL;
  TThing      *tArrow = NULL;
  TASubstance *asHerb = NULL;
  int          tValue,
               nRc;
  sstring       tStHerb,
               tStVict,
               tStPart;
  wearSlotT    tSlot;

  tStHerb=tArg.word(0);
  tStPart=tArg.word(1);
  tStVict=tArg.word(2);

  // Temp return until all the apply code is finished and tested.
  return FALSE;

  if (!hasHands() || affectedBySpell(AFFECT_TRANSFORMED_ARMS) ||
                     affectedBySpell(AFFECT_TRANSFORMED_HANDS)) {
    sendTo("You need a good hand to apply herbs!\n\r");
    return FALSE;
  }

  if (!doesKnowSkill(SKILL_APPLY_HERBS)) {
    sendTo("I bet you wish you knew how to use herbs, don't you.\n\r");
    return FALSE;
  }

  if(tStHerb.empty() || tStPart.empty()){
    sendTo("Syntax: apply <herb> <limb> <target>\n\r");
    return FALSE;
  }

  if(tStVict.empty() || tStVict=="self"){
    vict = this;
  } else if (!(vict = get_char_room_vis(this, tStVict))) {
    if (!(tArrow = searchLinkedListVis(this, tStVict, stuff))) {
      sendTo("You do not see that here, perhaps you were seeing things?\n\r");
      return FALSE;
    }
  }

  if (!(asHerb = dynamic_cast<TASubstance *>(searchLinkedListVis(this, tStHerb, stuff)))) {
    sendTo("You can not seem to find that herb.\n\r");
    return FALSE;
  }

  if ((tValue = old_search_block(tStPart.c_str(), 0, tStPart.length(), bodyParts, 0)) <= 0) {
    sendTo("Unfortunatly you cannot find that body part.\n\r");
    return FALSE;
  }

  tSlot = wearSlotT(tValue - 1);

  if (vict) {
    if (!vict->isPc() && vict->canSee(this)) {
      sendTo("You can not seem to get close enough, they don't seem to trust the herb.\n\r");
      return FALSE;
    }

    if (vict->equipment[tSlot]) {
      sendTo("Sadly you can not apply herbs to limbs that are equipped.\n\r");
      return FALSE;
    }

    tArrow = vict;
  } else if (tArrow) {
    if (!dynamic_cast<TArrow *>(tArrow)) {
      sendTo("You have no idea how to apply herbs to that.\n\r");
      return FALSE;
    }
  }

  // pass tArrow which is either a 'TArrow' or a pointer to 'vict'
  nRc = asHerb->applyMe(this, tArrow);

  if (IS_SET_DELETE(nRc, DELETE_VICT))
    if (this == tArrow)
      return DELETE_THIS;
    else {
      delete tArrow;
      tArrow = NULL;
      REM_DELETE(nRc, DELETE_VICT);
    }

  return nRc;
}

int TASubstance::applyMe(TBeing *ch, TThing *vict)
{
  return FALSE;
}

void TASubstance::setAFlags(int x1)
{
  AFlags = x1;
}

int TASubstance::getAFlags() const
{
  return AFlags;
}

void TASubstance::setAMethod(int x2)
{
  AMethod = x2;
}

int TASubstance::getAMethod() const
{
  return AMethod;
}

void TASubstance::setILevel(int x3)
{
  ILevel = x3;
}

int TASubstance::getILevel() const
{
  return ILevel;
}

void TASubstance::setISpell(int x4)
{
  ISpell = x4;
}

int TASubstance::getISpell() const
{
  return ISpell;
}

// Non-Class related functions:
// Count a list of words, make sure it doesn't exceed 10 total.
// Also assigns (ChMaxCnt) words to newChList in the order there found.
int appliedSubstanceCountList(const char *tArg, char **newChList = NULL, int ChMaxCnt = -1)
{
  char tString[256],
       tList[256];
  int  asTotalCount = 0;

  half_chop(tArg, tString, tList);
  tArg = tList;

  while (*tString && asTotalCount++ <= max(10, ChMaxCnt)) {
    if (asTotalCount <= ChMaxCnt) {
      newChList[asTotalCount - 1] = new char[strlen(tString) + 1];
      strcpy(newChList[asTotalCount - 1], tString);
    }

    for (; isspace(*tArg); tArg++);
    half_chop(tArg, tString, tList);
    tArg = tList;
  }

  return asTotalCount;
}

// Check a list of words against a users inventory.
// 1) Inventory
// 2) Worn/Held
// 3) Inventory-Bag
// 4) Worn/Held-Container[backpack, belt pouch, ...]
bool appliedSubstanceCheckList(TBeing *ch, const char *tArg,
                               char **newChList, TThing **tObjList, int LsSize)
{
#if 0
  TThing *tObj,
         *tStObj;
  TExpandableContainer *tCntObj = NULL;
  TOrganic *tOrgObj;
  bool  alRdHsMatch = false;
  int   curCount[2] = {0, 0};

  for (curCount[0] = 0; curCount[0] < LsSize; curCount[0]++) {
    if (!newChList[curCount[0]])
      continue;

    tObj = ch->getStuff();

    alRdHsMatch = true;
    while (alRdHsMatch && tObj && tObj->nextThing &&
           (tObj = searchLinkedListVis(ch, newChList[curCount[0]], tObj->nextThing)))
      if ((tOrgObj = dynamic_cast<TOrganic *>(tObj)) && tOrgObj->getOType() == ORGANIC_HERBAL) {
        alRdHsMatch = false;

        for (curCount[1] = 0; ((curCount[1] < LsSize) && !alRdHsMatch); curCount[1]++)
          if (tObjList[curCount[1]] == tObj)
            alRdHsMatch = true;

        if (!alRdHsMatch)
          tObjList[curCount[0]] = tObj;
      }
  }

  for (curCount[0] = 0; curCount[0] < LsSize; curCount[0]++) {
    if (tObjList[curCount[0]] || !newChList[curCount[0]])
      continue;

    alRdHsMatch = false;

    for (curCount[0] = MIN_WEAR; ((curCount[0] < MAX_WEAR) && !alRdHsMatch); curCount[0]++) {
      tObj = ch->equipment[curCount[0]];

      tOrgObj = dynamic_cast<TOrganic *>(tObj);

      if (!tObj || !ch->canSee(tObj) || !tOrgObj || tOrgObj->getOType() != ORGANIC_HERBAL)
        continue;

      if (isname(newChList[curCount[0]], tObj->name)) {
        tObjList[curCount[0]] = tObj;
        alRdHsMatch = true;
      }
    }
  }

  for (curCount[0] = 0; curCount[0] < LsSize; curCount[0]++) {
    if (tObjList[curCount[0]] || !newChList[curCount[0]])
      continue;

    tObj = ch->getStuff();

    for (; tObj; tObj = tObj->nextThing) {
      if (!(tCntObj = dynamic_cast<TExpandableContainer *>(tObj)))
        continue;

      tStObj = tCntObj->getStuff();
      alRdHsMatch = true;
      while (alRdHsMatch && tStObj && tStObj->nextThing &&
             (tStObj = searchLinkedListVis(ch, newChList[curCount[0]], tStObj->nextThing)))
        if ((tOrgObj = dynamic_cast<TOrganic *>(tObj)) && tOrgObj->getOType() == ORGANIC_HERBAL) {
          alRdHsMatch = false;

          for (curCount[1] = 0; ((curCount[1] < LsSize) && !alRdHsMatch); curCount[1]++)
            if (tObjList[curCount[1]] == tStObj)
              alRdHsMatch = true;

          if (!alRdHsMatch)
            tObjList[curCount[0]] = tStObj;
        }
    }
  }

  for (curCount[0] = 0; curCount[0] < LsSize; curCount[0]++) {
    if (tObjList[curCount[0]] || !newChList[curCount[0]])
      continue;

    alRdHsMatch = false;

    for (curCount[0] = MIN_WEAR; ((curCount[0] < MAX_WEAR) && !alRdHsMatch); curCount[0]++) {
      tObj = ch->equipment[curCount[0]];

      if (!tObj || !ch->canSee(tObj) || !(tCntObj = dynamic_cast<TExpandableContainer *>(tObj)))
        continue;

      tStObj = tCntObj->getStuff();
      alRdHsMatch = true;
      while (alRdHsMatch && tStObj && tStObj->nextThing &&
             (tStObj = searchLinkedListVis(ch, newChList[curCount[0]], tStObj->nextThing))) {

        tOrgObj = dynamic_cast<TOrganic *>(tStObj);

        if (!tOrgObj || tOrgObj->getOType() != ORGANIC_HERBAL)
          continue;

        for (curCount[1] = 0; ((curCount[1] < LsSize) && !alRdHsMatch); curCount[1]++)
          if (tObjList[curCount[1]] == tStObj)
            alRdHsMatch = true;

        if (!alRdHsMatch)
          tObjList[curCount[0]] = tStObj;
      }
    }
  }

  for (curCount[0] = 0; curCount[0] < LsSize; curCount[0]++)
    if (!tObjList[curCount[0]])
      return false;
#endif

  return true;
}

// Checks a passed list VS items list VS what-they-have
// true  = wants and Can use this skill
// false = Either doesn't want to or doesn't have the stuff for this skill
bool appliedSubstanceFindMatch(TThing **tObjList, int ceLevel, int LsSize, int skClassAs)
{
  bool  asFdMatchList[LsSize],
        hsFnMatch;
  int   Runner,
        tCompListRn;
  int   tCompListOrig[LsSize];
  TObj *tObj;

  for (Runner = 0; Runner < LsSize; Runner++) {
    asFdMatchList[Runner] = false;

    if (skClassAs == CLASS_RANGER)
      tCompListOrig[Runner] = AppliedCreate[ceLevel]->CompList[Runner];
    else {
      vlogf(LOG_BUG, format("Person got to SubstanceFindMatch with wrong skClassAs value [%d]") %  skClassAs);
      return false;
    }
  }

  for (Runner = 0; Runner < LsSize; Runner++) {
    if (!(tObj = dynamic_cast<TObj *>(tObjList[Runner])))
      continue;

    for (tCompListRn = 0, hsFnMatch = false; ((tCompListRn < 10) && !hsFnMatch); tCompListRn++) {
      if (asFdMatchList[tCompListRn] || tCompListOrig[tCompListRn] == -1)
        continue;

      if (tCompListOrig[tCompListRn] == tObj->objVnum()) {
        asFdMatchList[tCompListRn] = true;
        hsFnMatch = true;
      }
    }
  }

  for (Runner = 0; Runner < LsSize; Runner++)
    if (!asFdMatchList[Runner])
      return false;

  return true;
}

// Checks the room for a TFFlame object.
bool appliedSubstanceCheckFire(TBeing *ch)
{
  TThing *tThing=NULL;


  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (tThing=*it);++it)
    if (dynamic_cast<TFFlame *>(tThing))
      return true;

  return false;
}

bool appliedSubstanceHasInvItem(TBeing *ch, int tVNum)
{
  TThing *tThing=NULL;
  TObj   *tObj;

  for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end() && (tThing=*it);++it) {
    if (!(tObj = dynamic_cast<TObj *>(tThing)))
      continue;

    if (tObj->objVnum() == tVNum)
      return true;
  }

  return false;
}

//   1. (inventory) (---) Wood Canister  : 556=small, 557=large
//   2. (inventory) (558) Kitchen Grease
//   3. (  room   ) (---) TFFlame fire
//   4. (inventory) (---) Base Balm : 559=small, 560=large
//   5. (inventory) (555) Mortar Pestle
bool appliedSubstanceCheckBalm(TBeing *ch)
{
  bool  asCrInList[5]     = {false, false, false, false, false},
        hasBigSmall[2][2] = {{false, false}, {false, false}};

  hasBigSmall[0][0] = appliedSubstanceHasInvItem(ch, 556);
  hasBigSmall[0][1] = appliedSubstanceHasInvItem(ch, 557);
  asCrInList[0]     = ((hasBigSmall[0][0] || hasBigSmall[0][1]) ? true : false);
  asCrInList[1]     = appliedSubstanceHasInvItem(ch, 558);
  asCrInList[2]     = appliedSubstanceCheckFire (ch);
  hasBigSmall[1][0] = appliedSubstanceHasInvItem(ch, 559);
  hasBigSmall[1][1] = appliedSubstanceHasInvItem(ch, 560);
  asCrInList[3]     = ((hasBigSmall[1][0] || hasBigSmall[1][1]) ? true : false);
  asCrInList[4]     = appliedSubstanceHasInvItem(ch, 555);

  for (int Runner = 0; Runner < 5; Runner++)
    if (!asCrInList[Runner])
      return false;

  if ((!hasBigSmall[0][0] || !hasBigSmall[1][0]) &&
      (!hasBigSmall[0][1] || !hasBigSmall[1][1]))
    return false;

  return true;
}

// Checks for:
//   1. (inventory) (---) Animal Hoof  : 561=small, 562=large
//   2. (inventory) (563) Bacon Grease
//   3. (  room   ) (---) TFFlame fire
//   4. (inventory) (564) Oiled Cloth
//   5. (inventory) (---) Base Salve  : 565=small, 566=large
//   6. (inventory) (555) Mortar Pestle
bool appliedSubstanceCheckSalve(TBeing *ch)
{
  bool asCrInList[6] = {false, false, false, false, false, false},
       hasBigSmall[2][2] = {{false, false}, {false, false}};

  hasBigSmall[0][0] = appliedSubstanceHasInvItem(ch, 561);
  hasBigSmall[0][1] = appliedSubstanceHasInvItem(ch, 562);
  asCrInList[0]     = ((hasBigSmall[0][0] || hasBigSmall[0][1]) ? true : false);
  asCrInList[1]     = appliedSubstanceHasInvItem(ch, 563);
  asCrInList[2]     = appliedSubstanceCheckFire (ch);
  asCrInList[3]     = appliedSubstanceHasInvItem(ch, 564);
  hasBigSmall[1][0] = appliedSubstanceHasInvItem(ch, 565);
  hasBigSmall[1][1] = appliedSubstanceHasInvItem(ch, 566);
  asCrInList[4]     = ((hasBigSmall[1][0] || hasBigSmall[1][1]) ? true : false);
  asCrInList[5]     = appliedSubstanceHasInvItem(ch, 555);

  for (int Runner = 0; Runner < 6; Runner++)
    if (!asCrInList[Runner])
      return false;

  if ((!hasBigSmall[0][0] || !hasBigSmall[1][0]) &&
      (!hasBigSmall[0][1] || !hasBigSmall[1][1]))
    return false;

  return true;
}

// Checks for:
//   1. (inventory) (---) Animal Horn  : 567=small, 568=large
//   2. (inventory) (569) Flour
//   3. (  room   ) (---) TFFlame fire
//   4. (inventory) (570) Cork
//   5. (inventory) (---) Base Powder  : 571=small, 572=large
//   6. (inventory) (555) Mortar Pestle
bool appliedSubstanceCheckPowder(TBeing *ch)
{
  bool asCrInList[6] = {false, false, false, false, false, false},
       hasBigSmall[2][2] = {{false, false}, {false, false}};

  hasBigSmall[0][0] = appliedSubstanceHasInvItem(ch, 567);
  hasBigSmall[0][1] = appliedSubstanceHasInvItem(ch, 568);
  asCrInList[0] = ((hasBigSmall[0][0] || hasBigSmall[0][1]) ? true : false);
  asCrInList[1]     = appliedSubstanceHasInvItem(ch, 569);
  asCrInList[2]     = appliedSubstanceCheckFire (ch);
  asCrInList[3]     = appliedSubstanceHasInvItem(ch, 570);
  hasBigSmall[1][0] = appliedSubstanceHasInvItem(ch, 571);
  hasBigSmall[1][1] = appliedSubstanceHasInvItem(ch, 572);
  asCrInList[4] = ((hasBigSmall[1][0] || hasBigSmall[1][1]) ? true : false);
  asCrInList[5]     = appliedSubstanceHasInvItem(ch, 555);

  for (int Runner = 0; Runner < 6; Runner++)
    if (!asCrInList[Runner])
      return false;

  if ((!hasBigSmall[0][0] || !hasBigSmall[1][0]) &&
      (!hasBigSmall[0][1] || !hasBigSmall[1][1]))
    return false;

  return true;
}

// Checks for:
//   1. (inventory) (---) Animal Bladder  : 573=small, 574=large
//   2. (inventory) (575) Crushed Nuts
//   3. (  room   ) (---) TFFlame fire
//   4. (inventory) (576) Soft silken cord
//   5. (inventory) (---) Base Oil  : 577=small, 578=large
//   6. (inventory) (555) Mortar Pestle
bool appliedSubstanceCheckOil(TBeing *ch)
{
  bool asCrInList[6] = {false, false, false, false, false, false},
       hasBigSmall[2][2] = {{false, false}, {false, false}};

  hasBigSmall[0][0] = appliedSubstanceHasInvItem(ch, 573);
  hasBigSmall[0][1] = appliedSubstanceHasInvItem(ch, 574);
  asCrInList[0] = ((hasBigSmall[0][0] || hasBigSmall[0][1]) ? true : false);
  asCrInList[1]     = appliedSubstanceHasInvItem(ch, 575);
  asCrInList[2]     = appliedSubstanceCheckFire (ch);
  asCrInList[3]     = appliedSubstanceHasInvItem(ch, 576);
  hasBigSmall[1][0] = appliedSubstanceHasInvItem(ch, 577);
  hasBigSmall[1][1] = appliedSubstanceHasInvItem(ch, 578);
  asCrInList[4] = ((hasBigSmall[1][0] || hasBigSmall[1][1]) ? true : false);
  asCrInList[5]     = appliedSubstanceHasInvItem(ch, 555);

  for (int Runner = 0; Runner < 6; Runner++)
    if (!asCrInList[Runner])
      return false;

  if ((!hasBigSmall[0][0] || !hasBigSmall[1][0]) &&
      (!hasBigSmall[0][1] || !hasBigSmall[1][1]))
    return false;

  return true;
}

// Checks for:
//   1. (inventory) (---) Carved Stone Tube  : 579=small, 580=large
//   2. (inventory) (581) Plant Necter
//   3. (inventory) (582) Cut Rubber Stopper
//   4. (inventory) (---) Base Ichor  : 583=small, 584=large
//   5. (inventory) (555) Mortar Pestle
bool appliedSubstanceCheckIchor(TBeing *ch)
{
  bool asCrInList[5] = {false, false, false, false, false},
       hasBigSmall[2][2] = {{false, false}, {false, false}};

  hasBigSmall[0][0] = appliedSubstanceHasInvItem(ch, 579);
  hasBigSmall[0][1] = appliedSubstanceHasInvItem(ch, 580);
  asCrInList[0] = ((hasBigSmall[0][0] || hasBigSmall[0][1]) ? true : false);
  asCrInList[1]     = appliedSubstanceHasInvItem(ch, 581);
  asCrInList[2]     = appliedSubstanceHasInvItem(ch, 582);
  hasBigSmall[1][0] = appliedSubstanceHasInvItem(ch, 583);
  hasBigSmall[1][1] = appliedSubstanceHasInvItem(ch, 584);
  asCrInList[3] = ((hasBigSmall[1][0] || hasBigSmall[1][1]) ? true : false);
  asCrInList[4]     = appliedSubstanceHasInvItem(ch, 555);

  for (int Runner = 0; Runner < 5; Runner++)
    if (!asCrInList[Runner])
      return false;

  if ((!hasBigSmall[0][0] || !hasBigSmall[1][0]) &&
      (!hasBigSmall[0][1] || !hasBigSmall[1][1]))
    return false;

  return true;
}

int appliedSubstanceCreateBalm(TBeing *, cmdTypeT, int, TObj *)
{
  return 0;
}

int appliedSubstanceCreateSalve(TBeing *, cmdTypeT, int, TObj *)
{
  return 0;
}

int appliedSubstanceCreatePowder(TBeing *, cmdTypeT, int, TObj *)
{
  return 0;
}

int appliedSubstanceCreateOil(TBeing *, cmdTypeT, int, TObj *)
{
  return 0;
}

int appliedSubstanceCreateIchor(TBeing *, cmdTypeT, int, TObj *)
{
  return 0;
}
