//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_book.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// book.cc

#include "stdsneezy.h"

TBook::TBook() :
  TObj()
{
}

TBook::TBook(const TBook &a) :
  TObj(a)
{
}

TBook & TBook::operator=(const TBook &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TBook::~TBook()
{
}

void TBook::assignFourValues(int, int, int, int)
{
}

void TBook::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

string TBook::statObjInfo() const
{
  string a("");
  return a;
}

void TBook::lookAtObj(TBeing *ch, const char *arg, showModeT) const
{
  char the_filebuf[256];
  char buf[MAX_STRING_LENGTH];
  int vnum, section = 0;

  if (!ch->desc || (number < 0))
    return;

  sprintf(buf, "With curious eyes, you begin to read the book entitled :\n\r");
  sprintf(buf + strlen(buf), "%s...\n\r", good_cap(getName()).c_str());

// spam reduction
//  act("$n begins reading $p...", TRUE, ch, this, 0, TO_ROOM);

  vnum = obj_index[getItemIndex()].virt;
  // in next sscanf, buf eats 'words' like 'sect', 'section', 'chapter'
  sscanf(arg, "%s %d of ", the_filebuf, &section);
  if (section) {
    if (ch->hasColorVt()) {
      sprintf(the_filebuf, "objdata/books/%d.%d.ansi", vnum, section);
      string buf2;
      if (file_to_string(the_filebuf, buf2)) {
        // found ansi section
        strcat(buf, buf2.c_str());
        sprintf(buf + strlen(buf), "\n\rEnd of section %d.\n\r", section);
        ch->desc->page_string(buf, 0);
        return;
      }
    }
    sprintf(the_filebuf, "objdata/books/%d.%d", vnum, section);
    string buf2;
    if (file_to_string(the_filebuf, buf2)) {
      strcat(buf, buf2.c_str());
      sprintf(buf + strlen(buf), "\n\rEnd of section %d.\n\r", section);
      if (!ch->desc->client)
        ch->desc->page_string(buf, 0);
      else {
        string sb = buf;
        processStringForClient(sb);

        ch->desc->clientf("%d", CLIENT_NOTE);
        ch->sendTo(sb.c_str());
        ch->desc->clientf("%d", CLIENT_NOTE_END);
      }
      return;
    }
    sprintf(buf + strlen(buf), "Apparently, %s doesn't have that section.\n\r",
           getName());
    ch->desc->page_string(buf, 0);
    return;
  } else {
    if (ch->hasColorVt()) {
      sprintf(the_filebuf, "objdata/books/%d.ansi", vnum);
      string buf2;
      if (file_to_string(the_filebuf, buf2)) {
        // found ansi section
        strcat(buf, buf2.c_str());
        strcat(buf, "\n\r");
        ch->desc->page_string(buf, 0);
        return;
      }
    }
    sprintf(the_filebuf, "objdata/books/%d", vnum);
    string buf2;
    if (file_to_string(the_filebuf, buf2)) {
      strcat(buf, buf2.c_str());
      strcat(buf, "\n\r");
      if (!ch->desc->client)
        ch->desc->page_string(buf, 0);
      else {
        string sb = buf;
        processStringForClient(sb);

        ch->desc->clientf("%d", CLIENT_NOTE);
        ch->sendTo(sb.c_str());
        ch->desc->clientf("%d", CLIENT_NOTE_END);
      }
      return;
    }
    sprintf(buf + strlen(buf), "Apparently, %s is blank.\n\r", getName());
    vlogf(9, "Object %d has no book file!", vnum);
    ch->desc->page_string(buf, 0);
    return;
  }
}

