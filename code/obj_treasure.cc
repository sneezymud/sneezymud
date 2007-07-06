//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// treasure.cc

#include "stdsneezy.h"
#include "obj_treasure.h"
#include "database.h"
#include "obj_base_container.h"


TTreasure::TTreasure() :
  TObj()
{
  setSerialNumber(0);
}

TTreasure::TTreasure(const TTreasure &a) :
  TObj(a),
  serial_number(a.serial_number)
{
}

TTreasure & TTreasure::operator=(const TTreasure &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  serial_number = a.serial_number;
  return *this;
}

TTreasure::~TTreasure()
{
}

void TTreasure::assignFourValues(int x1, int, int, int)
{
  // not sure how else to sneak in a value for a particular item
  // so i'm using an accessor specifically for the immortal exchange coin serial #s here
  setSerialNumber(x1);
}

void TTreasure::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  // not sure how else to steal in a value for a particular item
  // so i'm using an accessor specifically for the immortal exchange coin serial #s here
  *x1 = getSerialNumber();
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TTreasure::statObjInfo() const
{
  sstring a("");
  if (objVnum() == OBJ_IMMORTAL_EXCHANGE_COIN) {
    if (!getSerialNumber()) {
      a = "\n\r<R>Invalid Immortal Exchange coin: no serial number.<1>\n\r";
    } else {
      TDatabase db(DB_SNEEZY);
      db.query("select *, (select name from player where id = c1.created_by) as name_created_by, (select name from player where id = c1.created_for) as name_created_for, (select name from player where id = c1.redeemed_by) as name_redeemed_by, (select name from player where id = c1.redeemed_for) as name_redeemed_for from immortal_exchange_coin c1 where c1.k_coin = %i", getSerialNumber());
      if (db.fetchRow()) {
        a = fmt("\n\rImmortal Exchange coin serial number: %i\n\r") % getSerialNumber();
        a += fmt("Created by %s on %s.\n\r") % db["name_created_by"].cap() % db["date_created"];
        a += fmt("Created for %s.\n\r") % db["name_created_for"].cap();
        if (db["name_redeemed_by"].length() > 0)
          a += fmt("Redeemed by %s on %s.\n\r") % db["name_redeemed_by"].cap() % db["date_redeemed"];
        if (db["name_redeemed_for"].length() > 0)
          a += fmt("Redeemed for %s.\n\r") % db["name_redeemed_for"].cap();
        // add a warning...
        if (db["date_redeemed"].length() > 0)
          a += "<R>Note: Reedemed coins should no longer be in play.\n\r      This is possibly a duplicated object.<1>\n\r";
      } else {
        a = fmt("\n\r<R>Invalid Immortal Exchange coin: unregistered serial number %i.<1>\n\r") % getSerialNumber();
      }
    }
  }
  return a;
}

void TTreasure::lowCheck()
{
  if (objVnum() == OBJ_IMMORTAL_EXCHANGE_COIN) {
    if (getSerialNumber()) {
      TDatabase db(DB_SNEEZY);
      db.query("select k_coin from immortal_exchange_coin where k_coin = %i and date_redeemed is not null", getSerialNumber());
      if (db.rowCount() > 0) {
        sstring buffer;
        if (roomp) {
          buffer = fmt(" in room %i") % roomp->in_room;
        } else if (equippedBy) {
          buffer = fmt(" on %s") % equippedBy->getName();
        } else if (parent && dynamic_cast<TBeing *>(parent)) {
          buffer = fmt(" on %s") % parent->getName();
        } else if (parent && dynamic_cast<TBaseContainer *>(parent) && parent->roomp) {
          buffer = fmt(" in %s in room %i") % parent->getName() % parent->roomp->in_room;
        } else if (parent && parent->parent && dynamic_cast<TBeing *>(parent->parent)) {
          buffer = fmt(" in %s on %s") % parent->getName() % parent->parent->getName();
        } else if (parent && parent->parent && parent->parent->roomp) {
          buffer = fmt(" in %s on %s in %i") % parent->getName() % parent->parent->getName() % parent->parent->roomp->in_room;
        } else {
          buffer = "";
        }
        vlogf(LOG_LOW, fmt("Calling all godly wrath: We've got a counterfeit Immortal Exchange coin%s, serial #%s.") % buffer % getSerialNumber());
      }
    }
  }
}
