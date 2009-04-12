//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////
// Peel

#include "being.h"
#include "obj_tooth_necklace.h"
#include "extern.h"
#include "monster.h"
#include "obj_card_deck.h"
#include "handler.h"

void TBeing::doShuffle(const sstring &arg)
{
  TObj *deck=NULL;

  if(arg.empty() || !(deck=generic_find_obj(arg, FIND_OBJ_INV, this))){
    sendTo("Shuffle what?\n\r");
    return;
  }

  if(!(dynamic_cast<TCardDeck *>(deck))){
    sendTo("Try as you might, you just don't have the dexterity to shuffle that.\n\r");
    return;
  }


  std::vector <TThing *> cards;
  TThing *t;
  for(StuffIter it=deck->stuff.begin();it!=deck->stuff.end();){
    t=*(it++);
    --(*t);
    cards.push_back(t);
  }

  std::random_shuffle(cards.begin(), cards.end());
  
  for(unsigned int i=0;i<cards.size();++i){
    *deck += *cards[i];
  }
  
  act("$n shuffles $p.",
      TRUE,this,deck,NULL, TO_ROOM,NULL);
  act("You shuffle $p.",
      TRUE,this,deck,NULL,TO_CHAR,NULL);
}


void TCardDeck::getObjFromMeText(TBeing *ch, TThing *obj, getTypeT, bool)
{
  --(*obj);
  *ch += *obj;
  act("You get $p from $P.", 0, ch, obj, this, TO_CHAR);
  act("$n gets a card from $P.", 1, ch, obj, this, TO_ROOM);
}


void TCardDeck::lookObj(TBeing *ch, int bits) const
{
  int count=0;
  for(StuffIter it=stuff.begin();it!=stuff.end();++it){
    count++;
  }

  ch->sendTo(format("It has %i cards in it.\n\r") % count);
}


TCardDeck::TCardDeck() :
  TExpandableContainer()
{
}

TCardDeck::TCardDeck(const TCardDeck &a) :
  TExpandableContainer(a)
{
}

TCardDeck & TCardDeck::operator=(const TCardDeck &a)
{
  if (this == &a) return *this;
  TExpandableContainer::operator=(a);
  return *this;
}

TCardDeck::~TCardDeck()
{
}

void TCardDeck::describeObjectSpecifics(const TBeing *ch) const
{
  int count=0;
  for(StuffIter it=stuff.begin();it!=stuff.end();++it){
    count++;
  }

  ch->sendTo(format("It has %i cards in it.\n\r") % count);
}


void TCardDeck::assignFourValues(int x1, int x2, int x3, int x4)
{
  TExpandableContainer::assignFourValues(x1, x2, x3, x4);
}

void TCardDeck::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TExpandableContainer::getFourValues(x1, x2, x3, x4);
}

sstring TCardDeck::statObjInfo() const
{
  return TExpandableContainer::statObjInfo();
}

bool TCardDeck::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "I can't repair that.");
  }
  return TRUE;
}
