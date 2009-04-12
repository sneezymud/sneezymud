//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// tree.cc

#include "comm.h"
#include "obj_tree.h"
#include "thing.h"
#include "being.h"

TTree::TTree() :
  TObj()
{
}

TTree::TTree(const TTree &a)
  : TObj(a)
{
}

TTree & TTree::operator=(const TTree &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TTree::~TTree()
{
}

void TTree::assignFourValues(int, int, int, int)
{
}

void TTree::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TTree::statObjInfo() const
{
  sstring a("");
  return a;
}

int TTree::treeMe(TBeing *caster, const char *tmp, int number, int *count)
{
  if (isname(tmp, name)) {
    if (number == *count) {
      act("You locate $N, and form a magical anchor between $M and you.",
            FALSE, caster, 0, this, TO_CHAR);
      return TRUE;
    }
    (*count)++;
  }
  return FALSE;
}

