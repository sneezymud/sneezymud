//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_tree.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// tree.cc

#include "stdsneezy.h"

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

string TTree::statObjInfo() const
{
  string a("");
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

