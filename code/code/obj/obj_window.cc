// window.cc
//

#include "extern.h"
#include "handler.h"
#include "room.h"
#include "being.h"
#include "obj_window.h"

TWindow::TWindow() :
  TSeeThru()
{
}

TWindow::TWindow(const TWindow &a) :
  TSeeThru(a)
{
}

TWindow & TWindow::operator=(const TWindow &a)
{
  if (this == &a) return *this;
  TSeeThru::operator=(a);
  return *this;
}

TWindow::~TWindow()
{
}

void TWindow::assignFourValues(int x1, int x2, int x3, int x4)
{
  TSeeThru::assignFourValues(x1,x2,x3,x4);
}

void TWindow::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TSeeThru::getFourValues(x1,x2,x3,x4);
}

sstring TWindow::statObjInfo() const
{
  char buf[256];
  sprintf(buf, "Target room: %d", getTarget());
  sstring a(buf);
  return a;
}

void TWindow::listMeExcessive(TBeing *) const
{
  // Do nothing, dont list, windows get listed more prettily
  // in the room descriptions!   - Russ
}

void TWindow::listMe(TBeing *, unsigned int) const
{
}

void TWindow::lookObj(TBeing *ch, int) const
{
  ch->windowLook(this);
}

void TWindow::showMe(TBeing *ch) const 
{
  ch->windowLook(this);
}

// don't display windows
void TWindow::show_me_mult_to_char(TBeing *ch, showModeT mode, unsigned int) const
{
  if (mode == SHOW_MODE_TYPE)
    showMe(ch);
}

// don't display windows
void TWindow::show_me_to_char(TBeing *ch, showModeT mode) const
{
  if (mode == SHOW_MODE_TYPE)
    showMe(ch);
}

// theoretically, this could be a TSeeThru function, but fundamentally
// changes portals, so will restrict it to old functionality
void TBeing::windowLook(const TWindow *w)
{
  TRoom *target;
  int    isRandom;

  act("$n peers through $p.", FALSE, this, w, NULL, TO_ROOM);
  if (!(target = real_roomp(w->getTarget(&isRandom)))) {
    sendTo("You see only an empty void.\n\r");
    vlogf(LOG_BUG, format("%s [%d] points to non existant room %d") %  w->shortDescr %
          obj_index[w->getItemIndex()].virt % w->getTarget());
    return;
  }
  if (isRandom == -1)
    act("You peer through $p to a fuzzy picture which clears then shows...",
        FALSE, this, w, NULL, TO_CHAR);
  else
    act("You peer through $p...", FALSE, this, w, NULL, TO_CHAR);
  sendRoomName(target);
  sendRoomDesc(target);

  list_thing_in_room(target->stuff, this);
}

