#include "stdsneezy.h"
#include "unistd.h"
#include "obj_note.h"
TNote::TNote() :
  TObj(),
  repairman(0),
  time_adjust(0),
  obj_v(0)
{
}

TNote::TNote(const TNote &a) :
  TObj(a),
  repairman(a.repairman),
  time_adjust(a.time_adjust),
  obj_v(a.obj_v)
{
}

// It is preseumed that msg has already allocated the memory
TNote * createNote(char *msg)
{
  TObj  *obj;
  TNote *note;

  if (!(obj = read_object(GENERIC_NOTE, VIRTUAL))) {
     vlogf(LOG_BUG, "Unable to load note in createNote");
     return NULL;
  }
				  
  note = dynamic_cast<TNote *>(obj);

  //  Create the note with the output.
  note->swapToStrung();
  delete [] note->action_description;
  note->action_description = msg;

  return note;
}

TNote & TNote::operator=(const TNote &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  repairman = a.repairman;
  time_adjust = a.time_adjust;
  obj_v = a.obj_v;
  return *this;
}

TNote::~TNote()
{
}

int TNote::getRepairman() const
{
  return repairman;
}

void TNote::setRepairman(int n)
{
  repairman = n;
}

int TNote::getTimeAdj() const
{
  return time_adjust;
}

void TNote::setTimeAdj(int n)
{
  time_adjust = n;
}

int TNote::getObjV() const
{
  return obj_v;
}

void TNote::setObjV(int n)
{
  obj_v = n;
}

void TNote::assignFourValues(int x1, int x2, int x3, int)
{
  setRepairman(x1);
  setTimeAdj(x2);
  setObjV(x3);
}

void TNote::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getRepairman();
  *x2 = getTimeAdj();
  *x3 = getObjV();
  *x4 = 0;
}

sstring TNote::statObjInfo() const
{
  char buf[256];
  int rc = real_mobile(getRepairman());

  sprintf(buf, "Repairman: %s : %d",
      (rc >= 0 ? mob_index[rc].short_desc : "Unknown"), getRepairman());

  sstring a(buf);
  return a;
}

int TNote::objectSell(TBeing *ch, TMonster *keeper)
{
  keeper->doTell(ch->getName(), "I'm sorry, I don't buy back notes.");
  return TRUE;
}

void TNote::junkMe(TBeing *ch)
{
  char buf[256];
  int tmpnum;

  if (action_description) {
    strcpy(buf, getName());
    if (sscanf(buf, "a small ticket marked number %d", &tmpnum) != 1) {
      // some bizarre descript.  possibly mail or a board-note
    } else {
      sprintf(buf, "mobdata/repairs/%d/%d", getRepairman(), tmpnum);
      int rc = unlink(buf);
      if (rc != 0) {
        vlogf(LOG_BUG, fmt("%s junking apparent ticket (#%d) [%s] that failed to unlink: err=%d, rc=%d") % 
            ch->getName() % tmpnum % buf % errno % rc);
      } else {
        // successfully unlinked the repair
        // we should update the count
        int objv = getObjV();
        int objr = real_object(objv);
        if (objr >= 0 && objr < (int) obj_index.size()){
          obj_index[objr].addToNumber(-1);
        }
      }
    }
  }
}

